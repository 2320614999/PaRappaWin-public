#include "pr_stage1_xa_cd_direct.h"

#include <algorithm>
#include <cstring>

namespace {

constexpr uint8_t kCdlSetloc = 2u;
constexpr uint8_t kCdlSetfilter = 13u;
constexpr uint8_t kCdlSetmode = 14u;
constexpr uint8_t kCdlGetlocP = 16u;
constexpr uint8_t kCdlReadS = 27u;
constexpr uint8_t kCdlPostReadStatus = 1u;
constexpr uint8_t kStageXaFile = 1u;
constexpr uint16_t kStageXaModeWord = 0x0048u;
constexpr uint16_t kMovieStreamModeWord = 0x01C8u;
constexpr uint32_t kStageXaPumpQuantum = 4u;
constexpr uint32_t kMovieStreamPumpQuantum = 16u;
constexpr uint16_t kRingHeaderMagic = 0x0160u;
constexpr uint16_t kRingStatusEmpty = 0u;
constexpr uint16_t kRingStatusWrap = 1u;
constexpr uint16_t kRingStatusReady = 2u;
constexpr uint16_t kRingStatusInFlight = 3u;
constexpr uint16_t kRingStatusInUse = 4u;

static uint16_t ReadU16LE(const uint8_t* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t ReadU32LE(const uint8_t* p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void WriteU16LE(uint8_t* p, uint16_t value) {
    p[0] = (uint8_t)(value & 0x00FFu);
    p[1] = (uint8_t)((value >> 8) & 0x00FFu);
}

static void WriteU32LE(uint8_t* p, uint32_t value) {
    p[0] = (uint8_t)(value & 0x000000FFu);
    p[1] = (uint8_t)((value >> 8) & 0x000000FFu);
    p[2] = (uint8_t)((value >> 16) & 0x000000FFu);
    p[3] = (uint8_t)((value >> 24) & 0x000000FFu);
}

static uint16_t SlotStatus(const PrStage1XaCdDirectRingSlot& slot) {
    return ReadU16LE(slot.header.data());
}

static void SetSlotStatus(PrStage1XaCdDirectRingSlot& slot, uint16_t status) {
    WriteU16LE(slot.header.data(), status);
}

static uint16_t SlotModeWord(const PrStage1XaCdDirectRingSlot& slot) {
    return ReadU16LE(slot.header.data() + 2);
}

static uint16_t SlotPartIndex(const PrStage1XaCdDirectRingSlot& slot) {
    return ReadU16LE(slot.header.data() + 4);
}

static uint16_t SlotPartCount(const PrStage1XaCdDirectRingSlot& slot) {
    return ReadU16LE(slot.header.data() + 6);
}

static uint16_t SlotFrameIndex(const PrStage1XaCdDirectRingSlot& slot) {
    return ReadU16LE(slot.header.data() + 8);
}

static uint32_t SlotCdResultStatus(const PrStage1XaCdDirectRingSlot& slot) {
    return ReadU32LE(slot.header.data() + 28);
}

static bool ValidSlotIndex(const PrStage1XaCdDirectState& state, uint32_t index) {
    return index < state.ringSlots.size();
}

static uint32_t DecodeBcdByte(uint32_t v) {
    return ((v >> 4) & 0x0Fu) * 10u + (v & 0x0Fu);
}

static uint32_t CdlPosBcdToLba(uint32_t posBcd) {
    const uint32_t minute = DecodeBcdByte(posBcd & 0xFFu);
    const uint32_t second = DecodeBcdByte((posBcd >> 8) & 0xFFu);
    const uint32_t sector = DecodeBcdByte((posBcd >> 16) & 0xFFu);
    const uint32_t absolute = 75u * (60u * minute + second) + sector;
    return (absolute >= 150u) ? (absolute - 150u) : 0u;
}

static uint32_t IssueCdCommand(PrStage1XaCdDirectState& state,
                               uint8_t command) {
    state.lastCdCommand = command;
    ++state.commandSerial;
    return state.commandSerial;
}

static uint32_t IssueCallbackEvent(PrStage1XaCdDirectState& state) {
    ++state.callbackSerial;
    return state.callbackSerial;
}

static bool IsAcceptedLowerCdSeamKind(
    PrStage1LoaderCdHal::ActionKind kind) {
    switch (kind) {
    case PrStage1LoaderCdHal::ActionKind::SeekSync800367A4:
    case PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0:
    case PrStage1LoaderCdHal::ActionKind::ReadSync800390C8:
        return true;
    default:
        return false;
    }
}

static void ApplySub800391AC(PrStage1XaCdDirectState& state,
                             uint16_t modeWord) {
    state.lastModeWord391AC = modeWord;
    state.lastSetModeByte391AC = (uint8_t)(modeWord & 0x00FFu);
    state.setmode14Serial = IssueCdCommand(state, kCdlSetmode);
    if ((modeWord & 0x0100u) != 0u) {
        state.dword_8008ECDC = (modeWord & 0x0020u) == 0u;
        state.callback80039318Installed = true;
        state.callback80039240Installed = true;
    }
    state.readS27Serial = IssueCdCommand(state, kCdlReadS);
}

static void ClearRingSlots(PrStage1XaCdDirectState& state,
                           uint32_t start,
                           uint32_t count) {
    if (state.ringSlots.empty()) {
        return;
    }
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t index = start + i;
        if (!ValidSlotIndex(state, index)) {
            break;
        }
        SetSlotStatus(state.ringSlots[index], kRingStatusEmpty);
    }
}

static void RecordProducerStatus(PrStage1XaCdDirectState& state,
                                 uint32_t statusCode) {
    state.producerRingAvailable = !state.ringSlots.empty();
    state.producerStatusKnown = true;
    state.lastProducerStatusCode = statusCode;
    state.dword_80057504 = statusCode;
}

static void ApplyStreamClockFeedback800493F4(
    PrStage1XaCdDirectState& state,
    const PrMovieSegmentDirect::StreamClockProducerFeedback800493F4& feedback,
    uint32_t sourceFunction) {
    if (!feedback.known || !feedback.byte800493F4Known) {
        return;
    }
    state.byte_800493F4Known = true;
    state.byte_800493F4 = feedback.byte800493F4;
    state.byte800493F4ProducerFunction = sourceFunction;
}

static void ApplyCdCallbackEvent80036AF8(
    PrStage1XaCdDirectState& state,
    const PrMovieSegmentDirect::CdCallbackEventResult80036AF8& result) {
    if (!result.called) {
        return;
    }
    state.cdLowerEventPsxReturn80036AF8Known = result.eventKnown;
    state.cdLowerEventPsxReturn80036AF8 = result.psxReturn;
    ++state.cdLowerEvent80036AF8Serial;
    if (result.dword80057108Known) {
        state.dword_80057108Known = true;
        state.dword_80057108 = result.dword80057108;
    }
    if (result.dword8005710CKnown) {
        state.dword_8005710CKnown = true;
        state.dword_8005710C = result.dword8005710C;
    }
    if (result.dword80057110Known) {
        state.dword_80057110Known = true;
        state.dword_80057110 = result.dword80057110;
    }
    if (result.byte800573D4Known) {
        state.byte_800573D4Known = true;
        state.byte_800573D4 = result.byte800573D4;
    }
    if (result.byte800573D5Known) {
        state.byte_800573D5Known = true;
        state.byte_800573D5 = result.byte800573D5;
    }
    if (result.byte800573D6Known) {
        state.byte_800573D6Known = true;
        state.byte_800573D6 = result.byte800573D6;
    }
    if (result.response882F8Known) {
        state.response_800882F8Known = true;
        state.response_800882F8 = result.response882F8;
    }
    if (result.syncFeedback.syncResultKnown) {
        state.cdSyncExplicitStatusKnown = true;
        state.cdSyncExplicitStatus =
            static_cast<uint8_t>(result.syncFeedback.syncResult);
    }
    if (result.syncFeedback.responseBytesKnown) {
        state.cdSyncExplicitResponseBytesKnown = true;
        state.cdSyncExplicitResponseBytes = {};
        const uint32_t count = (std::min)(
            result.syncFeedback.responseByteCount,
            static_cast<uint32_t>(state.cdSyncExplicitResponseBytes.size()));
        for (uint32_t i = 0u; i < count; ++i) {
            state.cdSyncExplicitResponseBytes[i] =
                result.syncFeedback.responseBytes[i];
        }
    }
    if (result.response88300Known) {
        state.response_80088300Known = true;
        state.response_80088300 = result.response88300;
    }
    if (result.response88308Known) {
        state.response_80088308Known = true;
        state.response_80088308 = result.response88308;
    }
    if (result.syncFeedback.known) {
        state.cdLowerFeedback80036AF8Known = true;
        state.cdLowerFeedback80036AF8 = result.syncFeedback;
        state.cdLowerFeedback80036AF8FromGetlocP = false;
    }
}

static bool TryBuildSub800364D0FeedbackFromExplicitStatus(
    const PrStage1XaCdDirectState& state,
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070& feedback) {
    if (!state.byte_80057119Known ||
        state.byte_80057119 != kCdlGetlocP ||
        !state.cdSyncExplicitStatusKnown ||
        !state.cdSyncExplicitResponseBytesKnown) {
        return false;
    }

    const uint8_t status = state.cdSyncExplicitStatus;
    if (status != 2u && status != 5u) {
        return false;
    }

    feedback.known = true;
    feedback.timedOut = false;
    feedback.syncResultKnown = true;
    feedback.syncResult = status;
    feedback.responseBytesKnown = true;
    feedback.responseByteCount =
        static_cast<uint32_t>(state.cdSyncExplicitResponseBytes.size());
    for (uint32_t i = 0u; i < state.cdSyncExplicitResponseBytes.size(); ++i) {
        feedback.responseBytes[i] = state.cdSyncExplicitResponseBytes[i];
    }
    return true;
}

static bool TryBuildStageRecordTickCommandFeedback8001A4D0(
    const PrStage1XaCdDirectStartInput& input,
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070& feedback) {
    feedback = {};
    if (!input.cdCommandCompletionKnown ||
        !input.cdCommandSyncResultKnown) {
        return false;
    }

    feedback.known = true;
    feedback.timedOut = input.cdCommandTimedOut;
    feedback.syncResultKnown = true;
    feedback.syncResult = input.cdCommandSyncResult;
    return true;
}

static void DispatchCdCallbacksFromSub80037070(
    PrStage1XaCdDirectState& state) {
    if (!state.cdLowerEventPsxReturn80036AF8Known ||
        state.cdLowerEvent80036AF8Serial == 0u ||
        state.cdLowerEvent80036AF8DispatchedSerial ==
            state.cdLowerEvent80036AF8Serial) {
        return;
    }
    if (!state.cdCallbackPending80035898Known) {
        ++state.cdCallbackPending80035898GapCount;
        return;
    } else if (!state.cdCallbackPending80035898) {
        return;
    }

    state.cdLowerEvent80036AF8DispatchedSerial =
        state.cdLowerEvent80036AF8Serial;

    const uint32_t eventMask =
        static_cast<uint32_t>(state.cdLowerEventPsxReturn80036AF8);
    if ((eventMask & 4u) != 0u) {
        ++state.cdAsyncCallback80037070GapCount;
    }
    if ((eventMask & 2u) == 0u) {
        return;
    }
    if (!state.dword_800570F8Known || state.dword_800570F8 == 0u) {
        ++state.cdSyncCallback80037070GapCount;
        return;
    }
    if (state.dword_800570F8 !=
        PrMovieSegmentDirect::kSub8001A210StreamClockCallback) {
        ++state.cdSyncCallback80037070GapCount;
        return;
    }

    PrMovieSegmentDirect::StreamClockCallbackInput8001A210 directInput{};
    directInput.a1 = state.byte_800573D4Known ? state.byte_800573D4 : 0u;
    if (state.response_800882F8Known) {
        directInput.resultBytes = state.response_800882F8.data();
        directInput.resultSize =
            static_cast<uint32_t>(state.response_800882F8.size());
    }

    const PrMovieSegmentDirect::StreamClockCallbackResult8001A210 result =
        PrMovieSegmentDirect::PsxCall8001A210_StreamClockCallback(
            directInput);
    ApplyStreamClockFeedback800493F4(
        state,
        result.feedback,
        result.sourceFunction);
    ++state.cdSyncCallback8001A210DispatchCount;
}

} // namespace

void PrStage1XaCdDirectReset(PrStage1XaCdDirectState& state) {
    state = PrStage1XaCdDirectState{};
}

PrStage1XaCdDirectStartResult PrStage1XaCdDirectStartStageStream(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectStartInput& input) {
    PrStage1XaCdDirectStartResult out{};
    if (!input.segPresent) {
        PrStage1XaCdDirectReset(state);
        return out;
    }

    state.streamStarted = true;
    PrStage1XaCdDirectClearRing(state, 32u);
    state.dword_800965A4 = 1u;
    state.dword_80091724 = 1u;
    state.dword_8009659C = 0xFFFFFFFFu;
    state.dword_8008ECD8 = false;
    state.dword_800917E0 = 0u;
    state.dword_80091720 = 0u;
    state.dword_800493EC = input.cdlFilePosBcd;
    state.dword_800493FC = CdlPosBcdToLba(input.cdlFilePosBcd);
    state.setloc2Serial = IssueCdCommand(state, kCdlSetloc);

    state.byte_8004940C = kStageXaFile;
    state.byte_8004940D = input.initialChannel;
    state.word_8004940E = 0u;
    state.setfilter13Serial = IssueCdCommand(state, kCdlSetfilter);

    state.dword_80049424 =
        input.mode1Streaming ? kMovieStreamPumpQuantum : kStageXaPumpQuantum;
    ApplySub800391AC(
        state,
        input.mode1Streaming ? kMovieStreamModeWord : kStageXaModeWord);

    out.started = true;
    out.file = state.byte_8004940C;
    out.channel = state.byte_8004940D;
    out.modeWord391AC = state.lastModeWord391AC;
    out.sectorLba = state.dword_800493FC;
    return out;
}

PrStage1XaCdDirectStageRecordTickResult8001A4D0
PrStage1XaCdDirectApplySub8001A4D0StageRecordTick(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectStartInput& input) {
    PrStage1XaCdDirectStageRecordTickResult8001A4D0 out{};
    out.called = true;
    if (!input.segPresent) {
        PrStage1XaCdDirectReset(state);
        out.resultKnown = true;
        out.psxReturn = 0;
        return out;
    }

    if (!state.streamStarted) {
        out.start = PrStage1XaCdDirectStartStageStream(state, input);
        out.started = out.start.started;
    }
    if (!state.streamStarted || state.readS27Serial == 0u) {
        out.waitingForHalFacts = true;
        return out;
    }

    PrStage1XaCdDirectCdSyncFromLowerStateInput800364D0 syncInput{};
    syncInput.a0WaitMode = 0;
    syncInput.a1OutputBufferPtrNonNull = false;
    syncInput.allowGetlocPFeedback = true;
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070 commandFeedback{};
    if (TryBuildStageRecordTickCommandFeedback8001A4D0(
            input,
            commandFeedback)) {
        PrStage1XaCdDirectCdSyncInput80037070 directInput{};
        directInput.a0WaitMode = syncInput.a0WaitMode;
        directInput.a1OutputBufferPtrNonNull =
            syncInput.a1OutputBufferPtrNonNull;
        directInput.feedback = commandFeedback;
        out.finalCdSync800364D0 =
            PrStage1XaCdDirectApplySub800364D0CdSync(state, directInput);
        DispatchCdCallbacksFromSub80037070(state);
    } else {
        out.finalCdSync800364D0 =
            PrStage1XaCdDirectApplySub800364D0CdSyncFromLowerState(
                state,
                syncInput);
    }
    if (!out.finalCdSync800364D0.syncResultKnown) {
        out.waitingForHalFacts = true;
        out.gapMissingCdSyncFeedback =
            out.finalCdSync800364D0.gapMissingCdSyncFeedback;
        return out;
    }
    if (out.finalCdSync800364D0.psxReturn != 2) {
        out.waitingForHalFacts = true;
        return out;
    }

    state.dword_80049410 = 1u;
    state.dword_80049420 = 0u - state.dword_80049424;
    state.dword_800570F8Known = true;
    state.dword_800570F8 = 0u;
    state.byte_800573D4Known = true;
    state.byte_800573D4 = 0u;
    state.byte_80057119Known = true;
    state.byte_80057119 = kCdlPostReadStatus;
    state.byte80057119ProducerFunction =
        PrMovieSegmentDirect::kSub800375BCCdCommand;
    state.cdCommandArgs800375BCKnown = true;
    state.cdCommandArgCount800375BC = 0u;
    state.cdCommandArgs800375BC = {};
    state.cdCommandA3_800375BC = 0;
    state.cdCommandSkipWait800375BC = true;
    state.command1Serial = IssueCdCommand(state, kCdlPostReadStatus);
    out.finalCommandWrapper80036678Known = true;
    out.finalCommandWrapper80036678Succeeded = true;
    out.resultKnown = true;
    out.psxReturn = 0;
    return out;
}

PrStage1XaCdDirectSetFilter13Request PrStage1XaCdDirectApplySub8001A654(
    PrStage1XaCdDirectState& state,
    uint8_t a1) {
    PrStage1XaCdDirectSetFilter13Request out{};
    state.byte_8004940D = a1;
    state.setfilter13Serial = IssueCdCommand(state, kCdlSetfilter);

    out.requestSetFilter = true;
    out.file = state.byte_8004940C;
    out.channel = state.byte_8004940D;
    out.command = kCdlSetfilter;
    out.commandSerial = state.commandSerial;
    return out;
}

PrStage1XaCdDirectBgmVolumeRequest8001A4A4
PrStage1XaCdDirectApplySub8001A478(int16_t a1) {
    PrStage1XaCdDirectBgmVolumeRequest8001A4A4 out{};
    out.requestSetVolume = true;
    out.left = a1;
    out.right = a1;
    out.normalizedVolume =
        (std::min)(1.0f, (std::max)(0.0f, (float)a1 / 127.0f));
    return out;
}

PrStage1XaCdDirectBgmVolumeRequest8001A4A4
PrStage1XaCdDirectApplySub8001A4A4(int a1) {
    return PrStage1XaCdDirectApplySub8001A478(
        static_cast<int16_t>(a1 == 1 ? 0 : 0x7F));
}

PrStage1XaCdDirectCallbackResult PrStage1XaCdDirectApplySub80039240PumpCallback(
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectCallbackResult out{};
    out.invoked = true;
    state.lastPumpCallback39240Serial = IssueCallbackEvent(state);
    out.callbackSerial = state.lastPumpCallback39240Serial;
    out.callbackCount = ++state.callback80039240Count;

    if (state.producerRingAvailable) {
        out.statusKnown = state.producerStatusKnown;
        out.statusCode = state.lastProducerStatusCode;
    }
    return out;
}

PrStage1XaCdDirectCallbackResult PrStage1XaCdDirectApplySub80039318DmaCallback(
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectCallbackResult out{};
    out.invoked = true;
    state.lastDmaCallback39318Serial = IssueCallbackEvent(state);
    out.callbackSerial = state.lastDmaCallback39318Serial;
    out.callbackCount = ++state.callback80039318Count;

    if (ValidSlotIndex(state, state.dword_80095C54)) {
        PrStage1XaCdDirectRingSlot& slot = state.ringSlots[state.dword_80095C54];
        SetSlotStatus(slot, kRingStatusReady);
        state.dword_8008A720 = SlotCdResultStatus(slot);
        state.dword_8008A724 = SlotFrameIndex(slot);
    }
    state.dword_80095C54 = state.dword_80095C50;
    state.dword_80092858 = false;
    out.statusKnown = true;
    out.statusCode = state.lastProducerStatusCode;
    return out;
}

PrMovieSegmentDirect::StreamClockCallbackResult8001A210
PrStage1XaCdDirectApplySub8001A210ClockCallback(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectClockCallbackInput8001A210& input) {
    PrMovieSegmentDirect::StreamClockCallbackInput8001A210 directInput{};
    directInput.a1 = input.a1;
    directInput.resultBytes = input.resultBytes;
    directInput.resultSize = input.resultSize;

    const PrMovieSegmentDirect::StreamClockCallbackResult8001A210 result =
        PrMovieSegmentDirect::PsxCall8001A210_StreamClockCallback(
            directInput);
    ApplyStreamClockFeedback800493F4(
        state,
        result.feedback,
        result.sourceFunction);
    return result;
}

PrStage1XaCdDirectCallbackRegisterResult80036510
PrStage1XaCdDirectApplySub80036510SetCdCallback(
    PrStage1XaCdDirectState& state,
    uint32_t callbackAddr,
    uint32_t sourceFunction) {
    PrStage1XaCdDirectCallbackRegisterResult80036510 out{};
    out.called = true;
    out.sourceFunction = sourceFunction;
    out.psxReturn = 0;
    out.dword800570F8Known = true;
    out.dword800570F8 = callbackAddr;
    out.streamClockCallback8001A210Registered =
        callbackAddr == PrMovieSegmentDirect::kSub8001A210StreamClockCallback;
    out.callbackCleared = callbackAddr == 0u;

    state.dword_800570F8Known = true;
    state.dword_800570F8 = callbackAddr;
    state.streamClockCallback8001A210Registered =
        out.streamClockCallback8001A210Registered;
    state.callbackRegisterSourceFunction = sourceFunction;
    return out;
}

PrStage1XaCdDirectCallbackRegisterResult8001A258
PrStage1XaCdDirectApplySub8001A258StreamClockCallbackRegister(
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectCallbackRegisterResult8001A258 out{};
    out.called = true;
    out.setCallback = PrStage1XaCdDirectApplySub80036510SetCdCallback(
        state,
        PrMovieSegmentDirect::kSub8001A210StreamClockCallback,
        out.sourceFunction);
    out.psxReturn = out.setCallback.psxReturn;
    return out;
}

PrStage1XaCdDirectClearCallbackResult8001A694
PrStage1XaCdDirectApplySub8001A694ClearCdCallback(
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectClearCallbackResult8001A694 out{};
    out.called = true;
    out.waitForCdSync800367A4 = true;
    out.setCallback = PrStage1XaCdDirectApplySub80036510SetCdCallback(
        state,
        0u,
        out.sourceFunction);
    out.psxReturn = out.setCallback.psxReturn;
    return out;
}

PrMovieSegmentDirect::StreamClockPollResult8001A3C8
PrStage1XaCdDirectApplySub8001A3C8ClockPoll(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectClockPollInput8001A3C8& input) {
    PrMovieSegmentDirect::StreamClockPollInput8001A3C8 directInput{};
    directInput.sub800364D0Known = input.sub800364D0Known;
    directInput.sub800364D0Result = input.sub800364D0Result;
    directInput.syncBytesKnown = input.syncBytesKnown;
    directInput.syncBytes = input.syncBytes;
    directInput.sub800363A4Known = input.sub800363A4Known;
    directInput.sub800363A4Result = input.sub800363A4Result;

    const PrMovieSegmentDirect::StreamClockPollResult8001A3C8 result =
        PrMovieSegmentDirect::PsxCall8001A3C8_StreamClockPoll(directInput);
    ApplyStreamClockFeedback800493F4(
        state,
        result.feedback,
        result.sourceFunction);
    if (result.dword80049428Known) {
        state.dword_80049428Known = true;
        state.dword_80049428 = result.dword80049428;
    }
    return result;
}

PrMovieSegmentDirect::StreamClockResetResult8001A724
PrStage1XaCdDirectApplySub8001A724ClockReset(
    PrStage1XaCdDirectState& state,
    int32_t a1) {
    const PrMovieSegmentDirect::StreamClockResetResult8001A724 result =
        PrMovieSegmentDirect::PsxCall8001A724_ResetStreamClock(a1);
    if (result.dword80049404Known) {
        state.dword_80049404Known = true;
        state.dword_80049404 = result.dword80049404;
    }
    if (result.dword80049408Known) {
        state.dword_80049408Known = true;
        state.dword_80049408 = result.dword80049408;
    }
    return result;
}

PrMovieSegmentDirect::CdReadyStatusResult800363A4
PrStage1XaCdDirectApplySub800363A4CdReadyStatus(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectCdReadyStatusInput800363A4& input) {
    PrMovieSegmentDirect::CdReadyStatusFeedback800363A4 feedback{};
    feedback.known = input.byte80057119Known;
    feedback.byte80057119 = input.byte80057119;

    const PrMovieSegmentDirect::CdReadyStatusResult800363A4 result =
        PrMovieSegmentDirect::PsxCall800363A4_ReadCdReadyStatus(feedback);
    if (result.resultKnown) {
        state.byte_80057119Known = true;
        state.byte_80057119 = static_cast<uint8_t>(result.psxReturn);
        if (result.psxReturn != kCdlGetlocP) {
            state.cdSyncExplicitStatusKnown = false;
            state.cdSyncExplicitStatus = 0u;
            state.cdSyncExplicitResponseBytesKnown = false;
            state.cdSyncExplicitResponseBytes = {};
        }
    }
    return result;
}

PrMovieSegmentDirect::CdSyncResult80037070
PrStage1XaCdDirectApplySub800364D0CdSync(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectCdSyncInput80037070& input) {
    const PrMovieSegmentDirect::CdSyncResult80037070 result =
        PrMovieSegmentDirect::PsxCall800364D0_CdSyncWrapper(
            input.a0WaitMode,
            input.a1OutputBufferPtrNonNull,
            input.feedback);
    if (result.syncResultKnown) {
        state.cdSync80037070Known = true;
        state.cdSync80037070 = result;
    }
    return result;
}

PrMovieSegmentDirect::CdSyncResult80037070
PrStage1XaCdDirectApplySub800364D0CdSyncFromLowerState(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectCdSyncFromLowerStateInput800364D0& input) {
    PrStage1XaCdDirectCdSyncInput80037070 directInput{};
    directInput.a0WaitMode = input.a0WaitMode;
    directInput.a1OutputBufferPtrNonNull = input.a1OutputBufferPtrNonNull;
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070 explicitFeedback{};
    if (input.allowGetlocPFeedback &&
        TryBuildSub800364D0FeedbackFromExplicitStatus(
            state,
            explicitFeedback)) {
        directInput.feedback = explicitFeedback;
        state.cdLowerFeedback80036AF8Known = true;
        state.cdLowerFeedback80036AF8 = explicitFeedback;
        state.cdLowerFeedback80036AF8FromGetlocP = true;
    } else if (state.cdLowerFeedback80036AF8Known &&
               (input.allowGetlocPFeedback ||
                !state.cdLowerFeedback80036AF8FromGetlocP)) {
        directInput.feedback = state.cdLowerFeedback80036AF8;
    }
    PrMovieSegmentDirect::CdSyncResult80037070 result =
        PrStage1XaCdDirectApplySub800364D0CdSync(state, directInput);
    DispatchCdCallbacksFromSub80037070(state);
    return result;
}

PrMovieSegmentDirect::CdCallbackEventResult80036AF8
PrStage1XaCdDirectApplySub80036AF8CdLowerEvent(
    PrStage1XaCdDirectState& state,
    const PrMovieSegmentDirect::CdCallbackEventInput80036AF8& input) {
    PrMovieSegmentDirect::CdCallbackEventInput80036AF8 eventInput = input;
    if (!eventInput.commandKnown && state.byte_80057119Known) {
        eventInput.commandKnown = true;
        eventInput.command = state.byte_80057119;
    }
    if (!eventInput.priorDword80057108Known &&
        state.dword_80057108Known) {
        eventInput.priorDword80057108Known = true;
        eventInput.priorDword80057108 = state.dword_80057108;
    }
    if (!eventInput.priorDword80057110Known &&
        state.dword_80057110Known) {
        eventInput.priorDword80057110Known = true;
        eventInput.priorDword80057110 = state.dword_80057110;
    }
    if (!eventInput.priorSyncMaskKnown && state.dword_80057108Known) {
        eventInput.priorSyncMaskKnown = true;
        eventInput.priorSyncMask =
            static_cast<uint8_t>(state.dword_80057108 & 0xFFu);
    }
    const PrMovieSegmentDirect::CdCallbackEventResult80036AF8 result =
        PrMovieSegmentDirect::PsxCall80036AF8_BuildCdLowerEvent(eventInput);
    ApplyCdCallbackEvent80036AF8(state, result);
    return result;
}

PrMovieSegmentDirect::CheckCallbackResult80035898
PrStage1XaCdDirectApplySub80035898CheckCallback(
    PrStage1XaCdDirectState& state,
    const PrMovieSegmentDirect::CheckCallbackInput80035898& input) {
    const PrMovieSegmentDirect::CheckCallbackResult80035898 result =
        PrMovieSegmentDirect::PsxCall80035898_CheckCallback(input);
    if (result.pendingKnown) {
        state.word_80055F7AKnown = true;
        state.word_80055F7A = result.word80055F7A;
        state.cdCallbackPending80035898Known = true;
        state.cdCallbackPending80035898 = result.pending;
    } else if (result.gapMissingWord80055F7A) {
        ++state.cdCallbackPending80035898GapCount;
    }
    return result;
}

PrMovieSegmentDirect::CdCallbackPendingProducerResult800359B8
PrStage1XaCdDirectApplySub800359B8CdCallbackPendingProducer(
    PrStage1XaCdDirectState& state,
    const PrMovieSegmentDirect::CdCallbackPendingProducerInput800359B8& input) {
    const PrMovieSegmentDirect::CdCallbackPendingProducerResult800359B8 result =
        PrMovieSegmentDirect::PsxCall800359B8_CdCallbackPendingProducer(input);
    if (result.pendingKnown) {
        if (input.word80055F78Known) {
            state.word_80055F78Known = true;
            state.word_80055F78 = input.word80055F78;
        }
        if (input.word80055FA8Known) {
            state.word_80055FA8Known = true;
            state.word_80055FA8 = input.word80055FA8;
        }
        if (input.interruptStatusKnown) {
            state.cdCallbackPending800359B8InterruptStatusKnown = true;
            state.cdCallbackPending800359B8InterruptStatus =
                input.interruptStatus;
        }
        if (input.interruptMaskKnown) {
            state.cdCallbackPending800359B8InterruptMaskKnown = true;
            state.cdCallbackPending800359B8InterruptMask = input.interruptMask;
        }
        if (input.watchdogKnown) {
            state.dword_80057010Known = true;
            state.dword_80057010 = input.dword80057010;
        }
        if (result.watchdogWritten) {
            state.dword_80057010Known = true;
            state.dword_80057010 = result.dword80057010Written;
        }
        state.word_80055F7AKnown = true;
        state.word_80055F7A = result.word80055F7A;
        state.cdCallbackPending80035898Known = true;
        state.cdCallbackPending80035898 = result.pending;
        state.cdCallbackPending800359B8WriteCount +=
            (result.setPendingWrite ? 1u : 0u) +
            (result.clearPendingWrite ? 1u : 0u);
        state.cdCallbackPending800359B8AckCount = result.interruptAckCount;
        for (uint32_t i = 0u;
             i < result.interruptAckCount &&
             i < state.cdCallbackPending800359B8AckBitIndex.size();
             ++i) {
            state.cdCallbackPending800359B8AckBitIndex[i] =
                result.interruptAckBitIndex[i];
        }
        state.cdCallbackPending800359B8CallbackDispatchCount =
            result.callbackDispatchCount;
        for (uint32_t i = 0u;
             i < result.callbackDispatchCount &&
             i < state.cdCallbackPending800359B8CallbackDispatchIndex.size();
             ++i) {
            state.cdCallbackPending800359B8CallbackDispatchIndex[i] =
                result.callbackDispatchIndex[i];
        }
    } else if (result.gapMissingWriteAddress ||
               result.gapUnknownWriteAddress) {
        ++state.cdCallbackPending800359B8GapCount;
    }
    return result;
}

PrStage1XaCdDirectLowerCdProducerResult
PrStage1XaCdDirectApplyLowerCdProducerSnapshot(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectLowerCdProducerSnapshot& snapshot) {
    PrStage1XaCdDirectLowerCdProducerResult out{};
    out.called = true;

    const bool hasCallbackSnapshot =
        snapshot.pendingProducer800359B8Known ||
        snapshot.lowerEvent80036AF8Known ||
        snapshot.lowerEventRegisters80036AF8Known;
    if (snapshot.pendingProducer800359B8Known) {
        out.pendingProducer800359B8 =
            PrStage1XaCdDirectApplySub800359B8CdCallbackPendingProducer(
                state,
                snapshot.pendingProducer800359B8);
        out.pendingProducer800359B8Applied =
            out.pendingProducer800359B8.called;
    }

    if (hasCallbackSnapshot) {
        PrMovieSegmentDirect::CheckCallbackInput80035898 checkInput{};
        checkInput.word80055F7AKnown = state.word_80055F7AKnown;
        checkInput.word80055F7A = state.word_80055F7A;
        out.checkCallback80035898 =
            PrStage1XaCdDirectApplySub80035898CheckCallback(state, checkInput);
        out.checkCallback80035898Applied = out.checkCallback80035898.called;
    }

    if (out.checkCallback80035898.pendingKnown &&
        out.checkCallback80035898.pending &&
        snapshot.lowerEventRegisters80036AF8Known) {
        PrMovieSegmentDirect::CdCallbackEventRegisterInput80036AF8
            registerInput = snapshot.lowerEventRegisters80036AF8;
        if (!registerInput.commandKnown && state.byte_80057119Known) {
            registerInput.commandKnown = true;
            registerInput.command = state.byte_80057119;
        }
        if (!registerInput.priorDword80057108Known &&
            state.dword_80057108Known) {
            registerInput.priorDword80057108Known = true;
            registerInput.priorDword80057108 = state.dword_80057108;
        }
        if (!registerInput.priorDword80057110Known &&
            state.dword_80057110Known) {
            registerInput.priorDword80057110Known = true;
            registerInput.priorDword80057110 = state.dword_80057110;
        }
        if (!registerInput.priorSyncMaskKnown &&
            state.dword_80057108Known) {
            registerInput.priorSyncMaskKnown = true;
            registerInput.priorSyncMask =
                static_cast<uint8_t>(state.dword_80057108 & 0xFFu);
        }
        out.lowerEventRegisters80036AF8 =
            PrMovieSegmentDirect::BuildCdCallbackEventInput80036AF8FromCdRegs(
                registerInput);
        out.lowerEventRegisters80036AF8Applied =
            out.lowerEventRegisters80036AF8.called;
        if (out.lowerEventRegisters80036AF8.builtEventInput) {
            out.lowerEvent80036AF8 =
                PrStage1XaCdDirectApplySub80036AF8CdLowerEvent(
                    state,
                    out.lowerEventRegisters80036AF8.eventInput);
            out.lowerEvent80036AF8Applied =
                out.lowerEvent80036AF8.called;
        }
    } else if (out.checkCallback80035898.pendingKnown &&
               out.checkCallback80035898.pending &&
               snapshot.lowerEvent80036AF8Known) {
        out.lowerEvent80036AF8 =
            PrStage1XaCdDirectApplySub80036AF8CdLowerEvent(
                state,
                snapshot.lowerEvent80036AF8);
        out.lowerEvent80036AF8Applied = out.lowerEvent80036AF8.called;
    }

    if (snapshot.cdSyncFeedback80037070Known) {
        state.cdLowerFeedback80036AF8Known = true;
        state.cdLowerFeedback80036AF8 =
            snapshot.cdSyncFeedback80037070;
        state.cdLowerFeedback80036AF8FromGetlocP = false;
        out.cdSyncFeedback80037070Applied = true;
    }
    out.readyForCdSync80037070 = state.cdLowerFeedback80036AF8Known;
    if (snapshot.cdSeamResultKnown) {
        out.cdSeamResult = snapshot.cdSeamResult;
        const PrStage1LoaderCdHal::Feedback& feedback =
            snapshot.cdSeamResult.feedback;
        if (snapshot.cdSeamResult.present && feedback.handled &&
            IsAcceptedLowerCdSeamKind(feedback.kind)) {
            out.cdSeamResultAccepted = true;
        } else {
            out.cdSeamResultRejected = true;
        }
    }
    return out;
}

PrStage1XaCdDirectLowerCdSnapshotBridgeResult
PrStage1XaCdDirectBuildLowerCdProducerSnapshot(
    const PrStage1XaCdDirectLowerCdSnapshotBridgeInput& input) {
    PrStage1XaCdDirectLowerCdSnapshotBridgeResult out{};
    const bool hasInput =
        input.interruptSnapshot800359B8Known ||
        input.rawEvent80036AF8Known ||
        input.cdSyncCoreFacts80037070Known ||
        input.cdSyncLoopFacts80037070Known ||
        input.cdSyncFeedback80037070Known ||
        input.lowerCdFactsKnown;
    if (!hasInput) {
        out.incomplete = true;
        return out;
    }
    const uint32_t cdSyncInputCount =
        (input.cdSyncCoreFacts80037070Known ? 1u : 0u) +
        (input.cdSyncLoopFacts80037070Known ? 1u : 0u) +
        (input.cdSyncFeedback80037070Known ? 1u : 0u);
    if (cdSyncInputCount > 1u) {
        out.incomplete = true;
        return out;
    }

    if (input.interruptSnapshot800359B8Known) {
        const PrStage1LowerCdProducerDirect::
            CdCallbackPendingBridgeResult800359B8 pending =
                PrStage1LowerCdProducerDirect::
                    BuildCdCallbackPendingBridgeInput800359B8(
                        input.interruptSnapshot800359B8);
        if (!pending.produced || pending.incomplete) {
            out.incomplete = true;
            return out;
        }
        out.snapshot.pendingProducer800359B8Known = true;
        out.snapshot.pendingProducer800359B8 = pending.input;
        out.pendingProducer800359B8Bridged = true;
    }

    if (input.rawEvent80036AF8Known) {
        const PrStage1LowerCdProducerDirect::
            CdCallbackEventRegisterBridgeResult80036AF8 event =
                PrStage1LowerCdProducerDirect::
                    BuildCdCallbackEventRegisterBridgeInput80036AF8(
                        input.rawEvent80036AF8);
        if (!event.produced || event.incomplete) {
            out.incomplete = true;
            return out;
        }
        if (event.earlyReturnNoInterrupt) {
            out.lowerEventEarlyReturnNoInterrupt = true;
        } else {
            out.snapshot.lowerEventRegisters80036AF8Known = true;
            out.snapshot.lowerEventRegisters80036AF8 = event.input;
            out.lowerEventRegisters80036AF8Bridged = true;
        }
    }

    if (input.cdSyncCoreFacts80037070Known) {
        const PrStage1LowerCdProducerDirect::
            CdSyncLowerFeedbackResult80037070 sync =
                PrStage1LowerCdProducerDirect::
                    BuildCdSyncLowerFeedback80037070FromCoreFacts(
                        input.cdSyncCoreFacts80037070);
        if (!sync.produced || sync.incomplete) {
            out.incomplete = true;
            return out;
        }
        out.snapshot.cdSyncFeedback80037070Known = true;
        out.snapshot.cdSyncFeedback80037070 = sync.feedback;
        out.cdSyncCoreFacts80037070Bridged = true;
    }

    if (input.cdSyncLoopFacts80037070Known) {
        const PrStage1LowerCdProducerDirect::CdSyncLoopFactsResult80037070
            loop =
                PrStage1LowerCdProducerDirect::BuildCdSyncLoopFacts80037070(
                    input.cdSyncLoopFacts80037070);
        if (!loop.produced || loop.incomplete || !loop.coreFactsKnown) {
            out.incomplete = true;
            return out;
        }

        const PrStage1LowerCdProducerDirect::
            CdSyncLowerFeedbackResult80037070 sync =
                PrStage1LowerCdProducerDirect::
                    BuildCdSyncLowerFeedback80037070FromCoreFacts(
                        loop.coreFacts);
        if (!sync.produced || sync.incomplete) {
            out.incomplete = true;
            return out;
        }
        out.snapshot.cdSyncFeedback80037070Known = true;
        out.snapshot.cdSyncFeedback80037070 = sync.feedback;
        out.cdSyncLoopFacts80037070Bridged = true;
    }

    if (input.cdSyncFeedback80037070Known) {
        out.snapshot.cdSyncFeedback80037070Known = true;
        out.snapshot.cdSyncFeedback80037070 = input.cdSyncFeedback80037070;
        out.cdSyncFeedback80037070Bridged = true;
    }

    if (input.lowerCdFactsKnown) {
        const PrStage1LowerCdProducerDirect::Result seam =
            PrStage1LowerCdProducerDirect::BuildLowerCdSeamFromFacts(
                input.lowerCdFacts);
        if (!seam.produced || seam.incomplete) {
            out.incomplete = true;
            return out;
        }
        out.snapshot.cdSeamResultKnown = true;
        out.snapshot.cdSeamResult = seam.cd;
        out.lowerCdFactsBridged = true;
    }

    out.produced = true;
    return out;
}

PrStage1XaCdDirectCommandResult800375BC
PrStage1XaCdDirectApplySub800375BCCommand(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectCommandInput800375BC& input) {
    PrStage1XaCdDirectCommandResult800375BC out{};
    out.called = true;
    out.preSyncCalled = true;
    out.preSyncResult =
        PrMovieSegmentDirect::PsxCall80037070_CdSync(
            0,
            false,
            input.preSyncFeedback);
    if (out.preSyncResult.syncResultKnown) {
        state.cdSync80037070Known = true;
        state.cdSync80037070 = out.preSyncResult;
    }

    state.cdSyncExplicitStatusKnown = false;
    state.cdSyncExplicitStatus = 0u;
    state.cdSyncExplicitResponseBytesKnown = false;
    state.cdSyncExplicitResponseBytes = {};
    state.byte_800573D4Known = true;
    state.byte_800573D4 = 0;
    state.byte_80057119Known = true;
    state.byte_80057119 = input.command;
    state.byte80057119ProducerFunction =
        PrMovieSegmentDirect::kSub800375BCCdCommand;
    state.cdCommandArgs800375BCKnown = input.argsKnown;
    state.cdCommandArgCount800375BC =
        input.argCount < state.cdCommandArgs800375BC.size()
            ? input.argCount
            : static_cast<uint32_t>(state.cdCommandArgs800375BC.size());
    state.cdCommandArgs800375BC = input.args;
    state.cdCommandA3_800375BC = input.a3;
    state.cdCommandSkipWait800375BC = input.skipWait;
    (void)IssueCdCommand(state, input.command);

    out.byte80057119Known = true;
    out.byte80057119 = input.command;
    out.psxReturn = 0;
    out.waitLoopRequested = !input.skipWait;
    if (!input.skipWait) {
        out.checkCallbackResult =
            PrStage1XaCdDirectApplySub80035898CheckCallback(
                state,
                input.checkCallback);
        if (out.checkCallbackResult.pendingKnown &&
            out.checkCallbackResult.pending) {
            out.callbackResult =
                PrStage1XaCdDirectApplySub80036AF8CdLowerEvent(
                    state,
                    input.callbackEvent);
        }
    }
    return out;
}

PrStage1XaCdDirectInitResult8001A280
PrStage1XaCdDirectApplySub8001A280WorkBaseCommand(
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectInitResult8001A280 out{};
    out.called = true;
    out.dword80049428Known = state.dword_80049428Known;
    out.dword80049428 = state.dword_80049428;
    if (!state.dword_80049428Known) {
        out.gapMissingDword80049428 = true;
        return out;
    }
    if (state.dword_80049428 != 0) {
        out.skippedNonZeroWorkBase = true;
        return out;
    }

    PrStage1XaCdDirectCommandInput800375BC command{};
    command.command = 0x10u;
    command.argsKnown = true;
    command.argCount = 0u;
    command.args = {};
    command.a3 = 0;
    command.skipWait = true;
    state.dword_800570F8Known = true;
    state.dword_800570F8 = 0u;
    out.commandResult =
        PrStage1XaCdDirectApplySub800375BCCommand(state, command);
    out.commandIssued = out.commandResult.called;
    return out;
}

PrMovieSegmentDirect::StreamClockPollResult8001A3C8
PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectCdSyncFromLowerStateInput800364D0 syncInput{};
    syncInput.a0WaitMode = 1;
    syncInput.a1OutputBufferPtrNonNull = true;
    syncInput.allowGetlocPFeedback = true;
    const PrMovieSegmentDirect::CdSyncResult80037070 sync =
        PrStage1XaCdDirectApplySub800364D0CdSyncFromLowerState(
            state,
            syncInput);

    PrMovieSegmentDirect::CdReadyStatusResult800363A4 ready{};
    ready.called = true;
    ready.sourceFunction = PrMovieSegmentDirect::kSub800363A4CdReadyStatus;
    ready.resultKnown = state.byte_80057119Known;
    ready.psxReturn = state.byte_80057119;
    ready.gapMissingByte80057119 = !state.byte_80057119Known;

    const PrMovieSegmentDirect::StreamClockPollInput8001A3C8 input =
        PrMovieSegmentDirect::BuildStreamClockPollInput8001A3C8FromCdSync(
            sync,
            ready);

    PrStage1XaCdDirectClockPollInput8001A3C8 directInput{};
    directInput.sub800364D0Known = input.sub800364D0Known;
    directInput.sub800364D0Result = input.sub800364D0Result;
    directInput.syncBytesKnown = input.syncBytesKnown;
    directInput.syncBytes = input.syncBytes;
    directInput.sub800363A4Known = input.sub800363A4Known;
    directInput.sub800363A4Result = input.sub800363A4Result;
    return PrStage1XaCdDirectApplySub8001A3C8ClockPoll(
        state,
        directInput);
}

PrMovieSegmentDirect::StreamStatusPollResult8001A750
PrStage1XaCdDirectApplySub8001A750StatusPollFromLowerState(
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectCdSyncFromLowerStateInput800364D0 syncInput{};
    syncInput.a0WaitMode = 1;
    syncInput.a1OutputBufferPtrNonNull = true;
    syncInput.allowGetlocPFeedback = false;
    const PrMovieSegmentDirect::CdSyncResult80037070 sync =
        PrStage1XaCdDirectApplySub800364D0CdSyncFromLowerState(
            state,
            syncInput);

    PrMovieSegmentDirect::StreamStatusPollInput8001A750 input =
        PrMovieSegmentDirect::BuildStreamStatusPollInput8001A750FromCdSync(
            sync);
    if (input.sub800364D0Known &&
        input.sub800364D0Result == 2 &&
        input.statusBytesKnown &&
        (input.statusBytes[0] & 0x20u) == 0u) {
        // 8001A750 calls 80036678(1,0). IDA shows that exact path skips
        // pre-Setloc and issues 800375BC(1,0,0,1), a no-arg skip-wait
        // command whose return is ignored by 8001A750.
        state.dword_800570F8Known = true;
        state.dword_800570F8 = 0u;
        state.byte_800573D4Known = true;
        state.byte_800573D4 = 0u;
        state.byte_80057119Known = true;
        state.byte_80057119 = kCdlPostReadStatus;
        state.byte80057119ProducerFunction =
            PrMovieSegmentDirect::kSub800375BCCdCommand;
        state.cdCommandArgs800375BCKnown = true;
        state.cdCommandArgCount800375BC = 0u;
        state.cdCommandArgs800375BC = {};
        state.cdCommandA3_800375BC = 0;
        state.cdCommandSkipWait800375BC = true;
        state.command1Serial =
            IssueCdCommand(state, kCdlPostReadStatus);

        input.commandWrapper80036678Known = true;
        input.commandWrapper80036678Succeeded = true;
    }
    return PrMovieSegmentDirect::PsxCall8001A750_StreamStatusPoll(input);
}

PrStage1XaCdDirectStreamClockProbe800493F4
PrStage1XaCdDirectProbeStreamClockProducer800493F4(
    const PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectStreamClockProbe800493F4 out{};
    out.inspected = true;
    out.setlocStartAnchorObserved = state.setloc2Serial != 0u;
    out.setloc2Serial = state.setloc2Serial;
    out.readS27Serial = state.readS27Serial;

    PrMovieSegmentDirect::StreamClockProducerFeedback800493F4 feedback{};
    if (state.byte_800493F4Known) {
        feedback.known = true;
        feedback.source =
            PrMovieSegmentDirect::StreamClockSource800493F4::Byte800493F4;
        feedback.producerAddr = state.byte800493F4ProducerFunction;
        feedback.byte800493F4Known = true;
        feedback.byte800493F4 = state.byte_800493F4;
        out.liveByte800493F4ProducerKnown = true;
    }
    out.carrier =
        PrMovieSegmentDirect::BuildStreamClockProducerCarrier800493F4(
            feedback);
    out.gapMissingStreamClock800493F4Producer =
        out.carrier.gapMissingByte800493F4ClockProducer;
    return out;
}

PrStage1XaCdDirectHalGetlocPFactsResult
PrStage1XaCdDirectApplyHalGetlocPFacts(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectHalGetlocPFactsInput& input) {
    PrStage1XaCdDirectHalGetlocPFactsResult out{};
    out.called = true;
    if (!input.cdGetlocPResponseKnown ||
        !input.cdDataReadyInterruptKnown ||
        input.cdDataReadyInterrupt != 2u) {
        out.incomplete = true;
        return out;
    }

    const std::array<uint8_t, 8> response = input.cdGetlocPResponse;

    state.cdSyncExplicitStatusKnown = true;
    state.cdSyncExplicitStatus = 2u;
    state.cdSyncExplicitResponseBytesKnown = true;
    state.cdSyncExplicitResponseBytes = response;
    ++state.halGetlocPFactsApplyCount;
    if (input.sectorIndexKnown) {
        state.lastHalGetlocPSectorIndexKnown = true;
        state.lastHalGetlocPSectorIndex = input.sectorIndex;
    }

    PrStage1XaCdDirectLowerCdSnapshotBridgeInput bridgeInput{};
    bridgeInput.cdSyncCoreFacts80037070Known = true;
    bridgeInput.cdSyncCoreFacts80037070.timedOutKnown = true;
    bridgeInput.cdSyncCoreFacts80037070.timedOut = false;
    bridgeInput.cdSyncCoreFacts80037070.syncResultKnown = true;
    bridgeInput.cdSyncCoreFacts80037070.syncResult = 2;
    bridgeInput.cdSyncCoreFacts80037070.responseBytesKnown = true;
    bridgeInput.cdSyncCoreFacts80037070.responseByteCount =
        static_cast<uint32_t>(
            sizeof(bridgeInput.cdSyncCoreFacts80037070.responseBytes));
    for (uint32_t i = 0u;
         i < bridgeInput.cdSyncCoreFacts80037070.responseByteCount;
         ++i) {
        bridgeInput.cdSyncCoreFacts80037070.responseBytes[i] =
            response[i];
    }
    const PrStage1XaCdDirectLowerCdSnapshotBridgeResult snapshot =
        PrStage1XaCdDirectBuildLowerCdProducerSnapshot(bridgeInput);
    if (!snapshot.produced || snapshot.incomplete) {
        out.incomplete = true;
        return out;
    }

    (void)PrStage1XaCdDirectApplyLowerCdProducerSnapshot(
        state,
        snapshot.snapshot);
    state.cdLowerFeedback80036AF8FromGetlocP = true;
    out.applied = true;
    return out;
}

void PrStage1XaCdDirectClearRing(PrStage1XaCdDirectState& state,
                                 uint32_t slotCount) {
    state.dword_801C3868 = slotCount;
    state.ringSlots.clear();
    state.ringSlots.resize(slotCount);
    state.dword_80095C50 = 0u;
    state.dword_80095C54 = 0u;
    state.dword_80095C58 = 0u;
    state.dword_80092858 = false;
    state.dword_80091640 = 0u;
    state.word_8008ECD4 = 0u;
    state.dword_8008ECA4 = 0u;
    state.dword_800965A8Index = 0u;
    state.producerRingAvailable = slotCount != 0u;
    state.producerStatusKnown = false;
    state.lastProducerStatusCode = 0u;
}

bool PrStage1XaCdDirectIsRingPacketCandidate(
    const PrStage1XaCdDirectRingPacketInput& packet) {
    return packet.header32 != nullptr &&
           packet.headerSize >= sizeof(uint16_t) &&
           ReadU16LE(packet.header32) == kRingHeaderMagic;
}

PrStage1XaCdDirectRingPumpResult PrStage1XaCdDirectApplySub80039670Packet(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectRingPacketInput& packet) {
    PrStage1XaCdDirectRingPumpResult out{};
    out.writeIndex = state.dword_80095C50;
    out.frameStartIndex = state.dword_80095C54;
    out.expectedPart = state.word_8008ECD4;
    out.frameIndex = state.dword_8008ECA4;

    if (state.dword_80092858) {
        RecordProducerStatus(state, 1u);
        out.statusCode = state.dword_80057504;
        return out;
    }
    if (packet.header32 == nullptr || packet.payload2016 == nullptr ||
        packet.headerSize < 32u || packet.payloadSize < 2016u) {
        RecordProducerStatus(state, 3u);
        out.statusCode = state.dword_80057504;
        return out;
    }
    if (state.ringSlots.empty() || state.dword_801C3868 == 0u) {
        PrStage1XaCdDirectClearRing(state, 32u);
    }
    if (!ValidSlotIndex(state, state.dword_80095C50)) {
        state.dword_80095C50 = 0u;
    }

    PrStage1XaCdDirectRingSlot* slot = &state.ringSlots[state.dword_80095C50];
    if (SlotStatus(*slot) != kRingStatusEmpty) {
        RecordProducerStatus(state, 4u);
        out.statusCode = state.dword_80057504;
        return out;
    }

    std::copy_n(packet.header32, 32u, slot->header.begin());
    WriteU32LE(slot->header.data() + 28, 0u);

    if (state.dword_800965A4 == 1u && state.dword_80091724 != 0u) {
        if (state.dword_80091724 != SlotFrameIndex(*slot)) {
            SetSlotStatus(*slot, kRingStatusEmpty);
            out.consumed = true;
            out.statusCode = state.dword_80057504;
            return out;
        }
        state.dword_800965A4 = 0u;
    }

    const uint16_t magic = SlotStatus(*slot);
    const uint32_t streamId = (uint32_t)((SlotModeWord(*slot) >> 10) & 0x1Fu);
    if (magic != kRingHeaderMagic || streamId != state.dword_800917E0) {
        if (state.dword_80096594) {
            state.dword_80092908 = 0u;
        }
        SetSlotStatus(*slot, kRingStatusEmpty);
        RecordProducerStatus(state, 5u);
        out.consumed = true;
        out.statusCode = state.dword_80057504;
        return out;
    }

    const uint16_t partIndex = SlotPartIndex(*slot);
    const uint16_t partCount = SlotPartCount(*slot);
    const uint16_t frameIndex = SlotFrameIndex(*slot);
    if (state.word_8008ECD4 != partIndex ||
        (state.dword_8008ECA4 != 0u && state.dword_8008ECA4 != frameIndex)) {
        state.dword_8008ECA4 = 0u;
        state.word_8008ECD4 = 0u;
        const uint32_t flushCount =
            (state.dword_80095C50 >= state.dword_80095C54)
                ? (state.dword_80095C50 - state.dword_80095C54)
                : 0u;
        ClearRingSlots(state, state.dword_80095C54, flushCount);
        state.dword_80095C50 = state.dword_80095C54;
        SetSlotStatus(*slot, kRingStatusEmpty);
        RecordProducerStatus(state, 6u);
        out.consumed = true;
        out.statusCode = state.dword_80057504;
        return out;
    }

    if (partIndex == 0u) {
        state.word_8008ECD4 = 0u;
        state.dword_8008ECA4 = frameIndex;
        if (state.dword_8009659C != 0u &&
            (uint32_t)frameIndex >= state.dword_8009659C) {
            state.dword_8008ECA4 = 0u;
            state.word_8008ECD4 = 0u;
            const uint32_t flushCount =
                (state.dword_80095C50 >= state.dword_80095C54)
                    ? (state.dword_80095C50 - state.dword_80095C54)
                    : 0u;
            ClearRingSlots(state, state.dword_80095C54, flushCount);
            state.dword_80095C50 = state.dword_80095C54;
            SetSlotStatus(*slot, kRingStatusEmpty);
            state.dword_800965A4 = 1u;
            RecordProducerStatus(state, 7u);
            out.consumed = true;
            out.statusCode = state.dword_80057504;
            return out;
        }

        if (state.dword_801C3868 - state.dword_80095C50 - 1u < (uint32_t)partCount) {
            if (state.dword_8009659C == 0u) {
                SetSlotStatus(*slot, kRingStatusWrap);
                state.dword_800965A4 = 1u;
                RecordProducerStatus(state, 8u);
                out.consumed = true;
                out.statusCode = state.dword_80057504;
                return out;
            }
            if (!state.ringSlots.empty() && SlotStatus(state.ringSlots[0]) != kRingStatusEmpty) {
                SetSlotStatus(*slot, kRingStatusEmpty);
                RecordProducerStatus(state, 9u);
                out.consumed = true;
                out.statusCode = state.dword_80057504;
                return out;
            }

            SetSlotStatus(*slot, kRingStatusWrap);
            state.ringSlots[0].header = slot->header;
            state.dword_80095C50 = 0u;
            slot = &state.ringSlots[0];
        }
        state.dword_80095C54 = state.dword_80095C50;
    }

    RecordProducerStatus(state, 10u);
    ++state.word_8008ECD4;
    state.dword_800965A8Index = state.dword_80095C50;
    std::copy_n(packet.payload2016, 2016u, slot->payload.begin());

    const bool lastPart = (partCount != 0u && (uint16_t)(partCount - 1u) == partIndex);
    if (lastPart) {
        state.dword_80092858 = true;
    }
    SetSlotStatus(*slot, kRingStatusInFlight);
    ++state.dword_80095C50;

    out.consumed = true;
    out.accepted = true;
    out.statusCode = state.dword_80057504;
    out.writeIndex = state.dword_80095C50;
    out.frameStartIndex = state.dword_80095C54;
    out.expectedPart = state.word_8008ECD4;
    out.frameIndex = state.dword_8008ECA4;

    if (lastPart) {
        PrStage1XaCdDirectApplySub80039318DmaCallback(state);
        out.frameReady = true;
        out.writeIndex = state.dword_80095C50;
        out.frameStartIndex = state.dword_80095C54;
        out.expectedPart = state.word_8008ECD4;
        out.frameIndex = state.dword_8008ECA4;
    }
    return out;
}

bool PrStage1XaCdDirectApplySub8003958CAcquireFrame(
    PrStage1XaCdDirectState& state,
    PrStage1XaCdDirectRingFrameView& out) {
    out = PrStage1XaCdDirectRingFrameView{};
    if (!ValidSlotIndex(state, state.dword_80095C58)) {
        return true;
    }

    PrStage1XaCdDirectRingSlot* slot = &state.ringSlots[state.dword_80095C58];
    if (SlotStatus(*slot) == kRingStatusWrap) {
        state.dword_80095C58 = 0u;
        if (state.dword_8009659C != 0u) {
            SetSlotStatus(*slot, kRingStatusEmpty);
        }
        if (!ValidSlotIndex(state, state.dword_80095C58)) {
            return true;
        }
        slot = &state.ringSlots[state.dword_80095C58];
    }

    if (SlotStatus(*slot) != kRingStatusReady) {
        return true;
    }

    SetSlotStatus(*slot, kRingStatusInUse);
    out.available = true;
    out.frameHandle = state.dword_80095C58;
    out.payload2016 = slot->payload.data();
    out.payloadSize = slot->payload.size();
    out.header32 = slot->header.data();
    out.headerSize = slot->header.size();
    out.partCount = SlotPartCount(*slot);
    out.frameIndex = SlotFrameIndex(*slot);
    return false;
}

bool PrStage1XaCdDirectApplySub80039490ReleaseFrame(
    PrStage1XaCdDirectState& state,
    uint32_t frameHandle) {
    if (!ValidSlotIndex(state, frameHandle)) {
        return true;
    }

    PrStage1XaCdDirectRingSlot& firstSlot = state.ringSlots[frameHandle];
    if (SlotStatus(firstSlot) != kRingStatusInUse) {
        return true;
    }

    const uint16_t partCount = SlotPartCount(firstSlot);
    uint32_t cleared = 0u;
    for (; cleared < (uint32_t)partCount; ++cleared) {
        const uint32_t index = frameHandle + cleared;
        if (!ValidSlotIndex(state, index)) {
            break;
        }
        SetSlotStatus(state.ringSlots[index], kRingStatusEmpty);
    }
    state.dword_80095C58 = frameHandle + cleared;
    return false;
}
