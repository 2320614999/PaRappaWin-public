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
constexpr uint32_t kSub80039240PumpCallback = 0x80039240u;
constexpr uint32_t kSub800391ACSetCdMode = 0x800391ACu;
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
constexpr uint32_t kCallbackSlotsObservationPsxAddress800570F8 = 0x800570F8u;
constexpr uint32_t kCallbackSlotsObservationByteSize800570F8FC = 8u;
constexpr uint32_t kInterruptSnapshotCallbackStatePsxAddress80055F78 =
    0x80055F78u;
constexpr uint32_t kInterruptSnapshotCallbackStateByteSize80055F78 = 0x34u;
constexpr uint32_t kInterruptSnapshotRegsPsxAddress1F801070 = 0x1F801070u;
constexpr uint32_t kInterruptSnapshotRegsByteSize1F801070 = 8u;
constexpr uint32_t kInterruptSnapshotWatchdogPsxAddress80057010 = 0x80057010u;
constexpr uint32_t kInterruptSnapshotWatchdogByteSize80057010 = 4u;
constexpr uint32_t kStatusByteObservationPsxAddress800573D4 = 0x800573D4u;
constexpr uint32_t kStatusByteObservationByteSize800573D4 = 1u;
constexpr uint32_t kStatusFlagsObservationPsxAddress80057108 = 0x80057108u;
constexpr uint32_t kStatusFlagsObservationByteSize80057108 = 4u;
constexpr uint32_t kStatusCounterObservationPsxAddress80057110 = 0x80057110u;
constexpr uint32_t kStatusCounterObservationByteSize80057110 = 4u;
constexpr uint32_t kCallbackPendingObservationPsxAddress80055F7A =
    0x80055F7Au;
constexpr uint32_t kCallbackPendingObservationByteSize80055F7A = 2u;
constexpr uint32_t kCommandWaitLoopDeadlinePsxAddress80088310 =
    0x80088310u;
constexpr uint32_t kCommandWaitLoopSpinPsxAddress80088314 = 0x80088314u;
constexpr uint32_t kCommandWaitLoopRuntimeSourceByteSize800375BC = 4u;
constexpr uint32_t kCommandWaitLoopSpinLimit800375BC = 0x003C0000u;

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
        PrStage1XaCdDirectApplySub80036528SetCdReadyCallback(
            state,
            kSub80039240PumpCallback,
            kSub800391ACSetCdMode);
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

static void PublishStatusByteRuntimeObservation800573D4(
    PrStage1XaCdDirectState& state,
    uint8_t value,
    uint32_t pc) {
    PrStage1XaCdDirectStatusByteObservation800573D4 observation{};
    observation.source =
        PrStage1XaCdDirectStatusByteObservationSource::
            RuntimePsxMemoryObservation;
    observation.psxAddress = kStatusByteObservationPsxAddress800573D4;
    observation.byteSize = kStatusByteObservationByteSize800573D4;
    observation.valueKnown = true;
    observation.value = value;
    observation.pcKnown = true;
    observation.pc = pc;
    PrStage1XaCdDirectApplyStatusByteRuntimeObservation800573D4(observation,
                                                               state);
}

static void PublishStatusFlagsRuntimeObservation80057108(
    PrStage1XaCdDirectState& state,
    uint32_t value,
    uint32_t pc) {
    PrStage1XaCdDirectStatusFlagsObservation80057108 observation{};
    observation.source =
        PrStage1XaCdDirectStatusFlagsObservationSource80057108::
            RuntimePsxMemoryObservation;
    observation.psxAddress = kStatusFlagsObservationPsxAddress80057108;
    observation.byteSize = kStatusFlagsObservationByteSize80057108;
    observation.valueKnown = true;
    observation.value = value;
    observation.pcKnown = true;
    observation.pc = pc;
    PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(observation,
                                                                 state);
}

static void PublishStatusCounterRuntimeObservation80057110(
    PrStage1XaCdDirectState& state,
    uint32_t value,
    uint32_t pc) {
    PrStage1XaCdDirectStatusCounterObservation80057110 observation{};
    observation.source =
        PrStage1XaCdDirectStatusCounterObservationSource80057110::
            RuntimePsxMemoryObservation;
    observation.psxAddress = kStatusCounterObservationPsxAddress80057110;
    observation.byteSize = kStatusCounterObservationByteSize80057110;
    observation.valueKnown = true;
    observation.value = value;
    observation.pcKnown = true;
    observation.pc = pc;
    PrStage1XaCdDirectApplyStatusCounterRuntimeObservation80057110(
        observation,
        state);
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
        PublishStatusFlagsRuntimeObservation80057108(
            state,
            result.dword80057108,
            result.sourceFunction);
    }
    if (result.dword8005710CKnown) {
        state.dword_8005710CKnown = true;
        state.dword_8005710C = result.dword8005710C;
    }
    if (result.dword80057110Known) {
        PublishStatusCounterRuntimeObservation80057110(
            state,
            result.dword80057110,
            result.sourceFunction);
    }
    if (result.byte800573D4Known) {
        PublishStatusByteRuntimeObservation800573D4(
            state,
            result.byte800573D4,
            result.sourceFunction);
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

static bool AreCallbackStateTableSlotsKnown80055F7C(
    const PrStage1XaCdDirectState& state);

bool PrStage1XaCdDirectReadKnownStateRuntimePsxMemory(
    void* userData,
    uint32_t psxAddress,
    uint32_t byteSize,
    uint8_t* outBytes,
    size_t outSize) {
    if (userData == nullptr || outBytes == nullptr || outSize < byteSize) {
        return false;
    }
    const auto* state = static_cast<const PrStage1XaCdDirectState*>(userData);
    if (psxAddress == 0x80049414u && byteSize == state->response_80049414.size()) {
        if (!state->response_80049414Known) return false;
        std::copy(state->response_80049414.begin(), state->response_80049414.end(), outBytes);
        return true;
    }
    if (psxAddress == 0x80057108u && byteSize == 4u) {
        if (!state->dword_80057108Known) {
            return false;
        }
        WriteU32LE(outBytes, state->dword_80057108);
        return true;
    }
    if (psxAddress == 0x80057110u && byteSize == 4u) {
        if (!state->dword_80057110Known) {
            return false;
        }
        WriteU32LE(outBytes, state->dword_80057110);
        return true;
    }
    if (psxAddress == 0x80057119u && byteSize == 1u) {
        if (!state->byte_80057119Known) {
            return false;
        }
        outBytes[0] = state->byte_80057119;
        return true;
    }
    if (psxAddress == 0x80055F7Au && byteSize == 2u) {
        if (!state->word_80055F7AKnown) {
            return false;
        }
        WriteU16LE(outBytes, state->word_80055F7A);
        return true;
    }
    if (psxAddress == 0x80055F78u && byteSize == 0x34u) {
        if (!state->word_80055F78Known ||
            !state->word_80055F7AKnown ||
            !state->word_80055FA8Known ||
            !state->callbackStateTable80055F7CKnown ||
            !AreCallbackStateTableSlotsKnown80055F7C(*state)) {
            return false;
        }
        std::memset(outBytes, 0, byteSize);
        WriteU16LE(outBytes, state->word_80055F78);
        WriteU16LE(outBytes + sizeof(uint16_t), state->word_80055F7A);
        for (uint32_t i = 0u;
             i < PrMovieSegmentDirect::
                     kCdCallbackPendingProducerCallbackCount800359B8;
             ++i) {
            WriteU32LE(outBytes + 4u + i * sizeof(uint32_t),
                       state->callbackStateTable80055F7CAddress[i]);
        }
        WriteU16LE(outBytes + 0x30u, state->word_80055FA8);
        return true;
    }
    if (psxAddress == 0x80057010u && byteSize == 4u) {
        if (!state->dword_80057010Known) {
            return false;
        }
        WriteU32LE(outBytes, state->dword_80057010);
        return true;
    }
    if (psxAddress == 0x80088310u && byteSize == 4u) {
        if (!state->cdCommandTimeoutDeadline80088310Known) {
            return false;
        }
        WriteU32LE(
            outBytes,
            static_cast<uint32_t>(
                state->cdCommandTimeoutDeadline80088310));
        return true;
    }
    if (psxAddress == 0x80088314u && byteSize == 4u) {
        if (!state->cdCommandTimeoutSpin80088314Known) {
            return false;
        }
        WriteU32LE(outBytes, state->cdCommandTimeoutSpin80088314);
        return true;
    }
    return false;
}

bool PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory(
    void* userData,
    uint32_t psxAddress,
    uint32_t byteSize,
    uint8_t* outBytes,
    size_t outSize) {
    if (userData == nullptr || outBytes == nullptr || outSize < byteSize ||
        byteSize != 1u) {
        return false;
    }
    const auto* snapshot =
        static_cast<const PrStage1XaCdDirectCdMmioSnapshotRuntimeSource*>(
            userData);
    if (psxAddress == PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8) {
        if (!snapshot->cdReg3InitialKnown) {
            return false;
        }
        outBytes[0] = snapshot->cdReg3Initial;
        return true;
    }
    if (psxAddress == PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8) {
        if (!snapshot->cdReg0StatusKnown) {
            return false;
        }
        outBytes[0] = snapshot->cdReg0Status;
        return true;
    }
    return false;
}

PrStage1XaCdDirectCdMmioSnapshotRuntimeSourceAudit
PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectCdMmioSnapshotRuntimeSourceAudit out{};
    out.sourceInstalled = provider.cdMmioSourceInstalled;
    out.readFnInstalled = provider.cdMmioRead != nullptr;
    out.userDataInstalled = provider.cdMmioUserData != nullptr;
    if (!provider.cdMmioSourceInstalled ||
        provider.cdMmioRead !=
            PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory ||
        provider.cdMmioUserData == nullptr) {
        return out;
    }

    const auto* snapshot =
        static_cast<const PrStage1XaCdDirectCdMmioSnapshotRuntimeSource*>(
            provider.cdMmioUserData);
    out.snapshotSource = true;
    out.producerIngressInstalled = snapshot->producerIngressInstalled;
    out.providerSubmitAvailable = snapshot->producerIngressInstalled;
    out.producerObservationCallCount =
        snapshot->producerObservationCallCount;
    out.cdReg3InitialKnown = snapshot->cdReg3InitialKnown;
    out.cdReg3Initial = snapshot->cdReg3Initial;
    out.cdReg0StatusKnown = snapshot->cdReg0StatusKnown;
    out.cdReg0Status = snapshot->cdReg0Status;
    out.canFeedCdReg3Initial = snapshot->cdReg3InitialKnown;
    out.canFeedCdReg0Status = snapshot->cdReg0StatusKnown;
    out.samplePairAvailable =
        snapshot->cdReg3InitialKnown && snapshot->cdReg0StatusKnown;
    out.observationAcceptedCount = snapshot->observationAcceptedCount;
    out.observationRejectedCount = snapshot->observationRejectedCount;
    out.lastRejectReason = snapshot->lastRejectReason;
    return out;
}

void PrStage1XaCdDirectReset(PrStage1XaCdDirectState& state) {
    state = PrStage1XaCdDirectState{};
}

PrStage1XaCdDirectCallbackSlotsObservationResult
PrStage1XaCdDirectApplyCallbackSlotsRuntimeObservation800570F8FC(
    const PrStage1XaCdDirectCallbackSlotsObservation800570F8FC& observation,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectCallbackSlotsObservationResult out{};
    if (observation.source !=
        PrStage1XaCdDirectCallbackSlotsObservationSource::
            RuntimePsxMemoryObservation) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
                NonRuntimePsxMemoryObservation;
        return out;
    }
    if (observation.psxAddress !=
        kCallbackSlotsObservationPsxAddress800570F8) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
                WrongPsxAddress;
        return out;
    }
    if (observation.byteSize !=
        kCallbackSlotsObservationByteSize800570F8FC) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
                WrongByteSize;
        return out;
    }
    if (!observation.valueKnown) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
                UnknownValue;
        return out;
    }
    if (!observation.bytes ||
        observation.bytesSize <
            kCallbackSlotsObservationByteSize800570F8FC) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackSlotsObservationRejectReason::
                MissingBytes;
        return out;
    }

    const uint32_t syncCallback = ReadU32LE(observation.bytes);
    const uint32_t readyCallback =
        ReadU32LE(observation.bytes + sizeof(uint32_t));
    state.dword_800570F8Known = true;
    state.dword_800570F8 = syncCallback;
    state.streamClockCallback8001A210Registered =
        syncCallback == PrMovieSegmentDirect::kSub8001A210StreamClockCallback;
    state.dword_800570FCKnown = true;
    state.dword_800570FC = readyCallback;
    state.callback80039240Installed =
        readyCallback == kSub80039240PumpCallback;
    out.accepted = true;
    out.rejectReason =
        PrStage1XaCdDirectCallbackSlotsObservationRejectReason::None;
    return out;
}

PrStage1XaCdDirectCallbackSlotsObservationResult
PrStage1XaCdDirectPublishCallbackSlotsDiscFullbootStartupObservation800570F8FC(
    PrStage1XaCdDirectState& state) {
    const std::array<uint8_t,
                     kCallbackSlotsObservationByteSize800570F8FC>
        zeroBytes{};
    PrStage1XaCdDirectCallbackSlotsObservation800570F8FC observation{};
    observation.source =
        PrStage1XaCdDirectCallbackSlotsObservationSource::
            RuntimePsxMemoryObservation;
    observation.psxAddress = kCallbackSlotsObservationPsxAddress800570F8;
    observation.byteSize = kCallbackSlotsObservationByteSize800570F8FC;
    observation.valueKnown = true;
    observation.bytes = zeroBytes.data();
    observation.bytesSize = zeroBytes.size();
    observation.frameKnown = true;
    observation.frame = 3600u;
    observation.pcKnown = true;
    observation.pc = 0x800356D0u;
    return PrStage1XaCdDirectApplyCallbackSlotsRuntimeObservation800570F8FC(
        observation,
        state);
}

static bool ValidateInterruptSnapshotObservationWindow800359B8(
    const PrStage1XaCdDirectInterruptSnapshotObservation800359B8& observation,
    uint32_t expectedPsxAddress,
    uint32_t expectedByteSize,
    PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8&
        rejectReason) {
    if (observation.source !=
        PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8::
            RuntimePsxMemoryObservation) {
        rejectReason =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                NonRuntimePsxMemoryObservation;
        return false;
    }
    if (observation.psxAddress != expectedPsxAddress) {
        rejectReason =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                WrongPsxAddress;
        return false;
    }
    if (observation.byteSize != expectedByteSize) {
        rejectReason =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                WrongByteSize;
        return false;
    }
    if (!observation.valueKnown) {
        rejectReason =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                UnknownValue;
        return false;
    }
    if (!observation.bytes || observation.bytesSize < expectedByteSize) {
        rejectReason =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                MissingBytes;
        return false;
    }
    rejectReason =
        PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
            None;
    return true;
}

static bool IsCompleteInterruptSnapshotObservationWindow800359B8(
    const PrStage1XaCdDirectInterruptSnapshotObservation800359B8& observation,
    uint32_t expectedPsxAddress,
    uint32_t expectedByteSize) {
    PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8
        reject =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                None;
    return ValidateInterruptSnapshotObservationWindow800359B8(
        observation,
        expectedPsxAddress,
        expectedByteSize,
        reject);
}

static bool AreCallbackStateTableSlotsKnown80055F7C(
    const PrStage1XaCdDirectState& state) {
    for (bool known : state.callbackStateTable80055F7CAddressKnown) {
        if (!known) {
            return false;
        }
    }
    return true;
}

PrStage1XaCdDirectInterruptSnapshotObservationResult800359B8
PrStage1XaCdDirectApplyInterruptSnapshotRuntimeObservation800359B8(
    const PrStage1XaCdDirectInterruptSnapshotObservationBundle800359B8&
        observation,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectInterruptSnapshotObservationResult800359B8 out{};
    PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8
        reject =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                None;
    state.interruptSnapshot800359B8LastCallbackStateKnown =
        IsCompleteInterruptSnapshotObservationWindow800359B8(
            observation.callbackState80055F78,
            kInterruptSnapshotCallbackStatePsxAddress80055F78,
            kInterruptSnapshotCallbackStateByteSize80055F78);
    state.interruptSnapshot800359B8LastInitialRegsKnown =
        IsCompleteInterruptSnapshotObservationWindow800359B8(
            observation.initialInterruptRegs1F801070,
            kInterruptSnapshotRegsPsxAddress1F801070,
            kInterruptSnapshotRegsByteSize1F801070);
    state.interruptSnapshot800359B8LastTerminalRegsKnown =
        IsCompleteInterruptSnapshotObservationWindow800359B8(
            observation.terminalInterruptRegs1F801070,
            kInterruptSnapshotRegsPsxAddress1F801070,
            kInterruptSnapshotRegsByteSize1F801070);
    state.interruptSnapshot800359B8LastWatchdogKnown =
        IsCompleteInterruptSnapshotObservationWindow800359B8(
            observation.watchdog80057010,
            kInterruptSnapshotWatchdogPsxAddress80057010,
            kInterruptSnapshotWatchdogByteSize80057010);
    state.interruptSnapshot800359B8LastBundleKnown =
        state.interruptSnapshot800359B8LastCallbackStateKnown &&
        state.interruptSnapshot800359B8LastInitialRegsKnown &&
        state.interruptSnapshot800359B8LastTerminalRegsKnown &&
        state.interruptSnapshot800359B8LastWatchdogKnown;
    if (!ValidateInterruptSnapshotObservationWindow800359B8(
            observation.callbackState80055F78,
            kInterruptSnapshotCallbackStatePsxAddress80055F78,
            kInterruptSnapshotCallbackStateByteSize80055F78,
            reject) ||
        !ValidateInterruptSnapshotObservationWindow800359B8(
            observation.initialInterruptRegs1F801070,
            kInterruptSnapshotRegsPsxAddress1F801070,
            kInterruptSnapshotRegsByteSize1F801070,
            reject) ||
        !ValidateInterruptSnapshotObservationWindow800359B8(
            observation.terminalInterruptRegs1F801070,
            kInterruptSnapshotRegsPsxAddress1F801070,
            kInterruptSnapshotRegsByteSize1F801070,
            reject) ||
        !ValidateInterruptSnapshotObservationWindow800359B8(
            observation.watchdog80057010,
            kInterruptSnapshotWatchdogPsxAddress80057010,
            kInterruptSnapshotWatchdogByteSize80057010,
            reject)) {
        out.rejectReason = reject;
        ++state.interruptSnapshot800359B8ObservationRejectedCount;
        state.interruptSnapshot800359B8ObservationLastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }

    const uint8_t* callbackState = observation.callbackState80055F78.bytes;
    const uint8_t* initialRegs = observation.initialInterruptRegs1F801070.bytes;
    const uint8_t* terminalRegs =
        observation.terminalInterruptRegs1F801070.bytes;

    PrStage1LowerCdProducerDirect::CdInterruptSnapshotInput800359B8 input{};
    input.iStatPtrKnown = true;
    input.iMaskPtrKnown = true;
    input.iStatKnown = true;
    input.iStat = ReadU16LE(initialRegs);
    input.iMaskKnown = true;
    input.iMask = ReadU16LE(initialRegs + sizeof(uint32_t));
    input.word80055F78Known = true;
    input.word80055F78 = ReadU16LE(callbackState);
    input.word80055F7ABeforeKnown = true;
    input.word80055F7ABefore = ReadU16LE(callbackState + sizeof(uint16_t));
    input.word80055F7ASetWriteKnown = true;
    input.word80055F7ASetWrite = 1u;
    input.word80055F7AClearWriteKnown = true;
    input.word80055F7AClearWrite = 0u;
    input.word80055FA8Known = true;
    input.word80055FA8 = ReadU16LE(callbackState + 0x30u);
    input.watchdogKnown = true;
    input.dword80057010 = ReadU32LE(observation.watchdog80057010.bytes);
    input.pendingSampleSequenceKnown = true;
    input.pendingSampleCount = 2u;
    input.iStatSamples[0] = input.iStat;
    input.iMaskSamples[0] = input.iMask;
    input.word80055FA8Samples[0] = input.word80055FA8;
    input.iStatSamples[1] = ReadU16LE(terminalRegs);
    input.iMaskSamples[1] = ReadU16LE(terminalRegs + sizeof(uint32_t));
    input.word80055FA8Samples[1] = input.word80055FA8;
    input.callbackTableKnown = true;
    input.callbackTableBaseKnown = true;
    input.callbackTableBase =
        PrStage1LowerCdProducerDirect::kCdCallbackTableBase800359B8;
    for (uint32_t i = 0u;
         i < PrStage1LowerCdProducerDirect::kCdCallbackCount800359B8;
         ++i) {
        const uint32_t address =
            ReadU32LE(callbackState + 4u + i * sizeof(uint32_t));
        input.callbackAddressKnown[i] = true;
        input.callbackAddress[i] = address;
        input.callbackPresent[i] = address != 0u;
    }

    PrStage1XaCdDirectLowerCdSnapshotBridgeInput bridgeInput{};
    bridgeInput.interruptSnapshot800359B8Known = true;
    bridgeInput.interruptSnapshot800359B8 = input;
    out.bridge = PrStage1XaCdDirectBuildLowerCdProducerSnapshot(bridgeInput);
    if (!out.bridge.produced || out.bridge.incomplete ||
        !out.bridge.pendingProducer800359B8Bridged) {
        out.rejectReason =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                IncompleteSnapshot;
        ++state.interruptSnapshot800359B8ObservationRejectedCount;
        state.interruptSnapshot800359B8ObservationLastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    PrStage1XaCdDirectApplyLowerCdProducerSnapshot(state,
                                                  out.bridge.snapshot);
    ++state.interruptSnapshot800359B8ObservationAcceptedCount;
    state.interruptSnapshot800359B8ObservationLastReject =
        static_cast<uint8_t>(
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                None);
    out.accepted = true;
    out.rejectReason =
        PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
            None;
    return out;
}

PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8
PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(
    const PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8&
        window) {
    PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8 out{};
    out.sourceAvailable = window.providerInstalled;
    out.bundleKnown = window.bundleReadable;
    if (!out.sourceAvailable || !out.bundleKnown) {
        return out;
    }

    out.bundle.callbackState80055F78.source =
        PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8::
            RuntimePsxMemoryObservation;
    out.bundle.callbackState80055F78.psxAddress =
        kInterruptSnapshotCallbackStatePsxAddress80055F78;
    out.bundle.callbackState80055F78.byteSize =
        kInterruptSnapshotCallbackStateByteSize80055F78;
    out.bundle.callbackState80055F78.valueKnown = true;
    out.bundle.callbackState80055F78.bytes =
        window.callbackState80055F78Bytes.data();
    out.bundle.callbackState80055F78.bytesSize =
        window.callbackState80055F78Bytes.size();
    out.bundle.callbackState80055F78.frameKnown = window.frameKnown;
    out.bundle.callbackState80055F78.frame = window.frame;
    out.bundle.callbackState80055F78.pcKnown = window.pcKnown;
    out.bundle.callbackState80055F78.pc = window.pc;

    out.bundle.initialInterruptRegs1F801070.source =
        PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8::
            RuntimePsxMemoryObservation;
    out.bundle.initialInterruptRegs1F801070.psxAddress =
        kInterruptSnapshotRegsPsxAddress1F801070;
    out.bundle.initialInterruptRegs1F801070.byteSize =
        kInterruptSnapshotRegsByteSize1F801070;
    out.bundle.initialInterruptRegs1F801070.valueKnown = true;
    out.bundle.initialInterruptRegs1F801070.bytes =
        window.initialInterruptRegs1F801070Bytes.data();
    out.bundle.initialInterruptRegs1F801070.bytesSize =
        window.initialInterruptRegs1F801070Bytes.size();
    out.bundle.initialInterruptRegs1F801070.frameKnown = window.frameKnown;
    out.bundle.initialInterruptRegs1F801070.frame = window.frame;
    out.bundle.initialInterruptRegs1F801070.pcKnown = window.pcKnown;
    out.bundle.initialInterruptRegs1F801070.pc = window.pc;

    out.bundle.terminalInterruptRegs1F801070.source =
        PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8::
            RuntimePsxMemoryObservation;
    out.bundle.terminalInterruptRegs1F801070.psxAddress =
        kInterruptSnapshotRegsPsxAddress1F801070;
    out.bundle.terminalInterruptRegs1F801070.byteSize =
        kInterruptSnapshotRegsByteSize1F801070;
    out.bundle.terminalInterruptRegs1F801070.valueKnown = true;
    out.bundle.terminalInterruptRegs1F801070.bytes =
        window.terminalInterruptRegs1F801070Bytes.data();
    out.bundle.terminalInterruptRegs1F801070.bytesSize =
        window.terminalInterruptRegs1F801070Bytes.size();
    out.bundle.terminalInterruptRegs1F801070.frameKnown = window.frameKnown;
    out.bundle.terminalInterruptRegs1F801070.frame = window.frame;
    out.bundle.terminalInterruptRegs1F801070.pcKnown = window.pcKnown;
    out.bundle.terminalInterruptRegs1F801070.pc = window.pc;

    out.bundle.watchdog80057010.source =
        PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8::
            RuntimePsxMemoryObservation;
    out.bundle.watchdog80057010.psxAddress =
        kInterruptSnapshotWatchdogPsxAddress80057010;
    out.bundle.watchdog80057010.byteSize =
        kInterruptSnapshotWatchdogByteSize80057010;
    out.bundle.watchdog80057010.valueKnown = true;
    out.bundle.watchdog80057010.bytes =
        window.watchdog80057010Bytes.data();
    out.bundle.watchdog80057010.bytesSize =
        window.watchdog80057010Bytes.size();
    out.bundle.watchdog80057010.frameKnown = window.frameKnown;
    out.bundle.watchdog80057010.frame = window.frame;
    out.bundle.watchdog80057010.pcKnown = window.pcKnown;
    out.bundle.watchdog80057010.pc = window.pc;
    return out;
}

PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8
PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8 window{};
    window.providerInstalled = provider.installed && provider.read != nullptr;
    window.frameKnown = provider.frameKnown;
    window.frame = provider.frame;
    window.pcKnown = provider.pcKnown;
    window.pc = provider.pc;
    if (!window.providerInstalled) {
        return window;
    }

    window.callbackStateReadAttempted = true;
    window.callbackStateReadable =
        provider.read(provider.userData,
                      kInterruptSnapshotCallbackStatePsxAddress80055F78,
                      kInterruptSnapshotCallbackStateByteSize80055F78,
                      window.callbackState80055F78Bytes.data(),
                      window.callbackState80055F78Bytes.size());
    window.initialRegsReadAttempted = true;
    window.initialRegsReadable =
        provider.read(provider.userData,
                      kInterruptSnapshotRegsPsxAddress1F801070,
                      kInterruptSnapshotRegsByteSize1F801070,
                      window.initialInterruptRegs1F801070Bytes.data(),
                      window.initialInterruptRegs1F801070Bytes.size());
    window.terminalRegsReadAttempted = true;
    window.terminalRegsReadable =
        provider.read(provider.userData,
                      kInterruptSnapshotRegsPsxAddress1F801070,
                      kInterruptSnapshotRegsByteSize1F801070,
                      window.terminalInterruptRegs1F801070Bytes.data(),
                      window.terminalInterruptRegs1F801070Bytes.size());
    window.watchdogReadAttempted = true;
    window.watchdogReadable =
        provider.read(provider.userData,
                      kInterruptSnapshotWatchdogPsxAddress80057010,
                      kInterruptSnapshotWatchdogByteSize80057010,
                      window.watchdog80057010Bytes.data(),
                      window.watchdog80057010Bytes.size());
    window.bundleReadable = window.callbackStateReadable &&
                            window.initialRegsReadable &&
                            window.terminalRegsReadable &&
                            window.watchdogReadable;
    return window;
}

PrStage1XaCdDirectInterruptSnapshotRuntimeSourceResult800359B8
PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
    const PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8& source,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectInterruptSnapshotRuntimeSourceResult800359B8 out{};
    out.sourceAvailable = source.sourceAvailable;
    out.bundleKnown = source.bundleKnown;
    if (!source.sourceAvailable || !source.bundleKnown) {
        return out;
    }
    out.publishAttempted = true;
    out.observation =
        PrStage1XaCdDirectApplyInterruptSnapshotRuntimeObservation800359B8(
            source.bundle,
            state);
    return out;
}

PrStage1XaCdDirectInterruptSnapshotTypedSourceAudit800359B8
PrStage1XaCdDirectAuditInterruptSnapshotTypedSource800359B8(
    const PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectInterruptSnapshotTypedSourceAudit800359B8 out{};
    out.inspected = true;
    out.callbackStatePsxAddress =
        kInterruptSnapshotCallbackStatePsxAddress80055F78;
    out.callbackStateByteSize =
        kInterruptSnapshotCallbackStateByteSize80055F78;
    out.callbackSlotBasePsxAddress = kCallbackSlotsObservationPsxAddress800570F8;
    out.callbackSlotByteSize = kCallbackSlotsObservationByteSize800570F8FC;
    out.callbackSlotsKnown =
        state.dword_800570F8Known && state.dword_800570FCKnown;
    out.callbackSlotsAreCallbackStateTable = false;
    out.callbackStateWordsKnown =
        state.word_80055F78Known && state.word_80055F7AKnown &&
        state.word_80055FA8Known;
    out.callbackStateTableKnown = out.callbackStateWordsKnown &&
                                  state.callbackStateTable80055F7CKnown &&
                                  AreCallbackStateTableSlotsKnown80055F7C(
                                      state);
    out.initialInterruptRegsKnown =
        state.cdCallbackPending800359B8InterruptStatusKnown &&
        state.cdCallbackPending800359B8InterruptMaskKnown;
    out.terminalInterruptRegsKnown = false;
    out.watchdogKnown = state.dword_80057010Known;
    if (!out.callbackStateTableKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapCallbackStateTable800359B8;
    }
    if (!out.initialInterruptRegsKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapInitialRegs800359B8;
    }
    if (!out.terminalInterruptRegsKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapTerminalRegs800359B8;
    }
    if (!out.watchdogKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapWatchdog800359B8;
    }
    out.typedCanReconstructBundle = out.missingMask == 0u;
    out.canFeedRuntimeObservation = false;
    return out;
}

PrStage1XaCdDirectStatusByteObservationResult
PrStage1XaCdDirectApplyStatusByteRuntimeObservation800573D4(
    const PrStage1XaCdDirectStatusByteObservation800573D4& observation,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectStatusByteObservationResult out{};
    if (observation.source !=
        PrStage1XaCdDirectStatusByteObservationSource::
            RuntimePsxMemoryObservation) {
        out.rejectReason =
            PrStage1XaCdDirectStatusByteObservationRejectReason::
                NonRuntimePsxMemoryObservation;
        ++state.statusByte800573D4ObservationRejectedCount;
        state.statusByte800573D4LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.psxAddress !=
        kStatusByteObservationPsxAddress800573D4) {
        out.rejectReason =
            PrStage1XaCdDirectStatusByteObservationRejectReason::
                WrongPsxAddress;
        ++state.statusByte800573D4ObservationRejectedCount;
        state.statusByte800573D4LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.byteSize !=
        kStatusByteObservationByteSize800573D4) {
        out.rejectReason =
            PrStage1XaCdDirectStatusByteObservationRejectReason::
                WrongByteSize;
        ++state.statusByte800573D4ObservationRejectedCount;
        state.statusByte800573D4LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (!observation.valueKnown) {
        out.rejectReason =
            PrStage1XaCdDirectStatusByteObservationRejectReason::UnknownValue;
        ++state.statusByte800573D4ObservationRejectedCount;
        state.statusByte800573D4LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }

    state.byte_800573D4Known = true;
    state.byte_800573D4 = observation.value;
    state.statusRead80036384Known = true;
    state.statusRead80036384 = observation.value;
    ++state.statusByte800573D4ObservationAcceptedCount;
    state.statusByte800573D4LastReject =
        static_cast<uint8_t>(
            PrStage1XaCdDirectStatusByteObservationRejectReason::None);
    out.accepted = true;
    out.rejectReason =
        PrStage1XaCdDirectStatusByteObservationRejectReason::None;
    return out;
}

PrStage1XaCdDirectStatusFlagsObservationResult80057108
PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(
    const PrStage1XaCdDirectStatusFlagsObservation80057108& observation,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectStatusFlagsObservationResult80057108 out{};
    if (observation.source !=
        PrStage1XaCdDirectStatusFlagsObservationSource80057108::
            RuntimePsxMemoryObservation) {
        out.rejectReason =
            PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
                NonRuntimePsxMemoryObservation;
        ++state.statusFlags80057108ObservationRejectedCount;
        state.statusFlags80057108LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.psxAddress !=
        kStatusFlagsObservationPsxAddress80057108) {
        out.rejectReason =
            PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
                WrongPsxAddress;
        ++state.statusFlags80057108ObservationRejectedCount;
        state.statusFlags80057108LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.byteSize != kStatusFlagsObservationByteSize80057108) {
        out.rejectReason =
            PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
                WrongByteSize;
        ++state.statusFlags80057108ObservationRejectedCount;
        state.statusFlags80057108LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (!observation.valueKnown) {
        out.rejectReason =
            PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
                UnknownValue;
        ++state.statusFlags80057108ObservationRejectedCount;
        state.statusFlags80057108LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }

    state.dword_80057108Known = true;
    state.dword_80057108 = observation.value;
    state.statusFlags80057108ObservationPcKnown = observation.pcKnown;
    state.statusFlags80057108ObservationPc =
        observation.pcKnown ? observation.pc : 0u;
    ++state.statusFlags80057108ObservationAcceptedCount;
    state.statusFlags80057108LastReject =
        static_cast<uint8_t>(
            PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::
                None);
    out.accepted = true;
    out.rejectReason =
        PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::None;
    return out;
}

PrStage1XaCdDirectStatusCounterObservationResult80057110
PrStage1XaCdDirectApplyStatusCounterRuntimeObservation80057110(
    const PrStage1XaCdDirectStatusCounterObservation80057110& observation,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectStatusCounterObservationResult80057110 out{};
    if (observation.source !=
        PrStage1XaCdDirectStatusCounterObservationSource80057110::
            RuntimePsxMemoryObservation) {
        out.rejectReason =
            PrStage1XaCdDirectStatusCounterObservationRejectReason80057110::
                NonRuntimePsxMemoryObservation;
        ++state.statusCounter80057110ObservationRejectedCount;
        state.statusCounter80057110LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.psxAddress !=
        kStatusCounterObservationPsxAddress80057110) {
        out.rejectReason =
            PrStage1XaCdDirectStatusCounterObservationRejectReason80057110::
                WrongPsxAddress;
        ++state.statusCounter80057110ObservationRejectedCount;
        state.statusCounter80057110LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.byteSize != kStatusCounterObservationByteSize80057110) {
        out.rejectReason =
            PrStage1XaCdDirectStatusCounterObservationRejectReason80057110::
                WrongByteSize;
        ++state.statusCounter80057110ObservationRejectedCount;
        state.statusCounter80057110LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (!observation.valueKnown) {
        out.rejectReason =
            PrStage1XaCdDirectStatusCounterObservationRejectReason80057110::
                UnknownValue;
        ++state.statusCounter80057110ObservationRejectedCount;
        state.statusCounter80057110LastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }

    state.dword_80057110Known = true;
    state.dword_80057110 = observation.value;
    state.statusCounter80057110ObservationPcKnown = observation.pcKnown;
    state.statusCounter80057110ObservationPc =
        observation.pcKnown ? observation.pc : 0u;
    ++state.statusCounter80057110ObservationAcceptedCount;
    state.statusCounter80057110LastReject =
        static_cast<uint8_t>(
            PrStage1XaCdDirectStatusCounterObservationRejectReason80057110::
                None);
    out.accepted = true;
    out.rejectReason =
        PrStage1XaCdDirectStatusCounterObservationRejectReason80057110::None;
    return out;
}

PrStage1XaCdDirectCallbackPendingObservationResult80055F7A
PrStage1XaCdDirectApplyCallbackPendingRuntimeObservation80055F7A(
    const PrStage1XaCdDirectCallbackPendingObservation80055F7A& observation,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectCallbackPendingObservationResult80055F7A out{};
    if (observation.source !=
        PrStage1XaCdDirectCallbackPendingObservationSource80055F7A::
            RuntimePsxMemoryObservation) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
                NonRuntimePsxMemoryObservation;
        ++state.callbackPendingWord80055F7AObservationRejectedCount;
        state.callbackPendingWord80055F7ALastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.psxAddress !=
        kCallbackPendingObservationPsxAddress80055F7A) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
                WrongPsxAddress;
        ++state.callbackPendingWord80055F7AObservationRejectedCount;
        state.callbackPendingWord80055F7ALastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.byteSize !=
        kCallbackPendingObservationByteSize80055F7A) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
                WrongByteSize;
        ++state.callbackPendingWord80055F7AObservationRejectedCount;
        state.callbackPendingWord80055F7ALastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (!observation.valueKnown) {
        out.rejectReason =
            PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
                UnknownValue;
        ++state.callbackPendingWord80055F7AObservationRejectedCount;
        state.callbackPendingWord80055F7ALastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }

    state.word_80055F7AKnown = true;
    state.word_80055F7A = observation.value;
    state.cdCallbackPending80035898Known = true;
    state.cdCallbackPending80035898 = observation.value != 0u;
    state.callbackPendingWord80055F7AObservationPcKnown =
        observation.pcKnown;
    state.callbackPendingWord80055F7AObservationPc =
        observation.pcKnown ? observation.pc : 0u;
    ++state.callbackPendingWord80055F7AObservationAcceptedCount;
    state.callbackPendingWord80055F7ALastReject =
        static_cast<uint8_t>(
            PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
                None);
    out.accepted = true;
    out.rejectReason =
        PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
            None;
    return out;
}

PrStage1XaCdDirectRawEventInitialInterruptObservationResult80036AF8
PrStage1XaCdDirectApplyRawEventInitialInterruptRuntimeObservation80036AF8(
    const PrStage1XaCdDirectRawEventInitialInterruptObservation80036AF8&
        observation,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectRawEventInitialInterruptObservationResult80036AF8 out{};
    if (observation.source !=
        PrStage1XaCdDirectRawEventInitialInterruptObservationSource80036AF8::
            RuntimePsxMemoryObservation) {
        out.rejectReason =
            PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8::
                NonRuntimePsxMemoryObservation;
        ++state.rawEvent80036AF8InitialInterruptObservationRejectedCount;
        state.rawEvent80036AF8InitialInterruptObservationLastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.psxAddress !=
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8) {
        out.rejectReason =
            PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8::
                WrongPsxAddress;
        ++state.rawEvent80036AF8InitialInterruptObservationRejectedCount;
        state.rawEvent80036AF8InitialInterruptObservationLastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (observation.byteSize != 1u) {
        out.rejectReason =
            PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8::
                WrongByteSize;
        ++state.rawEvent80036AF8InitialInterruptObservationRejectedCount;
        state.rawEvent80036AF8InitialInterruptObservationLastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }
    if (!observation.valueKnown) {
        out.rejectReason =
            PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8::
                UnknownValue;
        ++state.rawEvent80036AF8InitialInterruptObservationRejectedCount;
        state.rawEvent80036AF8InitialInterruptObservationLastReject =
            static_cast<uint8_t>(out.rejectReason);
        return out;
    }

    state.rawEvent80036AF8InitialInterruptKnown = true;
    state.rawEvent80036AF8InitialInterrupt = observation.value;
    ++state.rawEvent80036AF8InitialInterruptObservationAcceptedCount;
    state.rawEvent80036AF8InitialInterruptObservationLastReject =
        static_cast<uint8_t>(
            PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8::
                None);
    out.accepted = true;
    out.rejectReason =
        PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8::
            None;
    return out;
}

PrStage1XaCdDirectCdMmioSnapshotObservationResult
PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeObservation(
    const PrStage1XaCdDirectCdMmioSnapshotObservation& observation,
    PrStage1XaCdDirectCdMmioSnapshotRuntimeSource& snapshot) {
    PrStage1XaCdDirectCdMmioSnapshotObservationResult out{};
    ++snapshot.producerObservationCallCount;
    const auto reject = [&](PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason
                                reason) {
        ++snapshot.observationRejectedCount;
        snapshot.lastRejectReason = reason;
        out.rejectReason = reason;
        return out;
    };
    if (observation.source !=
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer) {
        return reject(PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                          NonRuntimeCdMmioSampleProducer);
    }
    if (observation.psxAddress !=
            PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8 &&
        observation.psxAddress !=
            PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8) {
        return reject(PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                          UnsupportedPsxAddress);
    }
    if (observation.byteSize != 1u) {
        return reject(PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                          WrongByteSize);
    }
    if (!observation.valueKnown) {
        return reject(
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::UnknownValue);
    }

    if (observation.psxAddress ==
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8) {
        snapshot.cdReg3InitialKnown = true;
        snapshot.cdReg3Initial = observation.value;
    } else {
        snapshot.cdReg0StatusKnown = true;
        snapshot.cdReg0Status = observation.value;
    }
    out.accepted = true;
    out.rejectReason =
        PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
    ++snapshot.observationAcceptedCount;
    snapshot.lastRejectReason =
        PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
    return out;
}

PrStage1XaCdDirectCdMmioSnapshotObservationResult
PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
    const PrStage1XaCdDirectCdMmioSnapshotObservation& observation,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectCdMmioSnapshotObservationResult out{};
    if (!provider.cdMmioSourceInstalled ||
        provider.cdMmioRead !=
            PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory ||
        provider.cdMmioUserData == nullptr) {
        out.rejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                RuntimeProviderUnavailable;
        return out;
    }

    auto* snapshot =
        static_cast<PrStage1XaCdDirectCdMmioSnapshotRuntimeSource*>(
            provider.cdMmioUserData);
    if (!snapshot->producerIngressInstalled) {
        out.rejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                RuntimeProviderUnavailable;
        return out;
    }
    return PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeObservation(
        observation,
        *snapshot);
}

PrStage1XaCdDirectCdMmioRuntimeProducerResult
PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSample(
    const PrStage1XaCdDirectCdMmioRuntimeProducerSample& sample,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectCdMmioRuntimeProducerResult out{};
    out.attempted = true;
    out.sourceAvailable = sample.sourceAvailable;
    out.valueKnown = sample.valueKnown;
    if (!sample.sourceAvailable) {
        return out;
    }
    if (sample.psxAddress !=
            PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8 &&
        sample.psxAddress !=
            PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8) {
        out.rejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                UnsupportedPsxAddress;
        return out;
    }
    if (sample.byteSize != 1u) {
        out.rejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                WrongByteSize;
        return out;
    }
    if (!sample.valueKnown) {
        out.rejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                UnknownValue;
        return out;
    }
    out.exactEnvelope = true;

    PrStage1XaCdDirectCdMmioSnapshotObservation observation{};
    observation.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer;
    observation.psxAddress = sample.psxAddress;
    observation.byteSize = sample.byteSize;
    observation.valueKnown = sample.valueKnown;
    observation.value = sample.value;
    observation.frameKnown = sample.frameKnown;
    observation.frame = sample.frame;
    observation.pcKnown = sample.pcKnown;
    observation.pc = sample.pc;

    out.publishAttempted = true;
    const PrStage1XaCdDirectCdMmioSnapshotObservationResult result =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            observation,
            provider);
    out.accepted = result.accepted;
    out.rejectReason = result.rejectReason;
    return out;
}

namespace {

bool ValidateRuntimeCdMmioProducerSampleEnvelope(
    const PrStage1XaCdDirectCdMmioRuntimeProducerSample& sample,
    uint32_t expectedPsxAddress,
    PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason& rejectReason) {
    rejectReason = PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
    if (!sample.sourceAvailable) {
        return false;
    }
    if (sample.psxAddress != expectedPsxAddress) {
        rejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                UnsupportedPsxAddress;
        return false;
    }
    if (sample.byteSize != 1u) {
        rejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                WrongByteSize;
        return false;
    }
    if (!sample.valueKnown) {
        rejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::UnknownValue;
        return false;
    }
    return true;
}

PrStage1XaCdDirectCdMmioSnapshotObservation
BuildRuntimeCdMmioSnapshotObservation(
    const PrStage1XaCdDirectCdMmioRuntimeProducerSample& sample) {
    PrStage1XaCdDirectCdMmioSnapshotObservation observation{};
    observation.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer;
    observation.psxAddress = sample.psxAddress;
    observation.byteSize = sample.byteSize;
    observation.valueKnown = sample.valueKnown;
    observation.value = sample.value;
    observation.frameKnown = sample.frameKnown;
    observation.frame = sample.frame;
    observation.pcKnown = sample.pcKnown;
    observation.pc = sample.pc;
    return observation;
}

} // namespace

PrStage1XaCdDirectCdMmioRuntimeProducerPairResult
PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSamplePair(
    const PrStage1XaCdDirectCdMmioRuntimeProducerSample& cdReg3Sample,
    const PrStage1XaCdDirectCdMmioRuntimeProducerSample& cdReg0Sample,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectCdMmioRuntimeProducerPairResult out{};
    out.attempted = true;
    out.sourceAvailable =
        cdReg3Sample.sourceAvailable && cdReg0Sample.sourceAvailable;

    out.cdReg3ExactEnvelope = ValidateRuntimeCdMmioProducerSampleEnvelope(
        cdReg3Sample,
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8,
        out.cdReg3RejectReason);
    if (!out.cdReg3ExactEnvelope) {
        return out;
    }

    out.cdReg0ExactEnvelope = ValidateRuntimeCdMmioProducerSampleEnvelope(
        cdReg0Sample,
        PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8,
        out.cdReg0RejectReason);
    if (!out.cdReg0ExactEnvelope) {
        return out;
    }

    out.publishAttempted = true;
    if (!provider.cdMmioSourceInstalled ||
        provider.cdMmioRead !=
            PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory ||
        provider.cdMmioUserData == nullptr) {
        out.cdReg3RejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                RuntimeProviderUnavailable;
        out.cdReg0RejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                RuntimeProviderUnavailable;
        return out;
    }
    const auto* snapshot =
        static_cast<const PrStage1XaCdDirectCdMmioSnapshotRuntimeSource*>(
            provider.cdMmioUserData);
    if (!snapshot->producerIngressInstalled) {
        out.cdReg3RejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                RuntimeProviderUnavailable;
        out.cdReg0RejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
                RuntimeProviderUnavailable;
        return out;
    }

    const auto cdReg3Result =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            BuildRuntimeCdMmioSnapshotObservation(cdReg3Sample),
            provider);
    out.cdReg3RejectReason = cdReg3Result.rejectReason;
    if (!cdReg3Result.accepted) {
        return out;
    }

    const auto cdReg0Result =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            BuildRuntimeCdMmioSnapshotObservation(cdReg0Sample),
            provider);
    out.cdReg0RejectReason = cdReg0Result.rejectReason;
    out.accepted = cdReg0Result.accepted;
    return out;
}

bool PrStage1XaCdDirectSubmitCdMmioSnapshotSamplesFromRawEvent80036AF8(
    const PrStage1LowerCdProducerDirect::RawCdRegTransactionResult80036AF8&
        transaction,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    if (!transaction.produced ||
        transaction.incomplete ||
        transaction.earlyReturnNoInterrupt ||
        !transaction.cdReg0StatusKnown) {
        return false;
    }

    PrStage1XaCdDirectCdMmioRuntimeProducerSample cdReg3{};
    cdReg3.sourceAvailable = true;
    cdReg3.psxAddress = PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    cdReg3.byteSize = 1u;
    cdReg3.valueKnown = true;
    cdReg3.value = transaction.cdReg3InitialInterrupt;

    PrStage1XaCdDirectCdMmioRuntimeProducerSample cdReg0{};
    cdReg0.sourceAvailable = true;
    cdReg0.psxAddress = PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8;
    cdReg0.byteSize = 1u;
    cdReg0.valueKnown = true;
    cdReg0.value = transaction.cdReg0Status;

    const auto pairResult =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSamplePair(cdReg3,
                                                               cdReg0,
                                                               provider);
    return pairResult.accepted;
}

PrStage1XaCdDirectStatusFlagsRuntimeSource80057108
PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(
    const PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108& window) {
    PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 out{};
    out.sourceAvailable =
        window.providerInstalled &&
        window.windowReadable &&
        window.psxAddress == kStatusFlagsObservationPsxAddress80057108 &&
        window.byteSize == kStatusFlagsObservationByteSize80057108;
    out.valueKnown = window.valueKnown;
    if (!out.sourceAvailable) {
        return out;
    }

    out.observation.source =
        PrStage1XaCdDirectStatusFlagsObservationSource80057108::
            RuntimePsxMemoryObservation;
    out.observation.psxAddress = window.psxAddress;
    out.observation.byteSize = window.byteSize;
    out.observation.valueKnown = window.valueKnown;
    out.observation.value = window.value;
    out.observation.frameKnown = window.frameKnown;
    out.observation.frame = window.frame;
    out.observation.pcKnown = window.pcKnown;
    out.observation.pc = window.pc;
    return out;
}

PrStage1XaCdDirectStatusCounterRuntimeSource80057110
PrStage1XaCdDirectBuildStatusCounterRuntimeSource80057110(
    const PrStage1XaCdDirectStatusCounterRuntimeSourceWindow80057110& window) {
    PrStage1XaCdDirectStatusCounterRuntimeSource80057110 out{};
    out.sourceAvailable =
        window.providerInstalled &&
        window.windowReadable &&
        window.psxAddress == kStatusCounterObservationPsxAddress80057110 &&
        window.byteSize == kStatusCounterObservationByteSize80057110;
    out.valueKnown = window.valueKnown;
    if (!out.sourceAvailable) {
        return out;
    }

    out.observation.source =
        PrStage1XaCdDirectStatusCounterObservationSource80057110::
            RuntimePsxMemoryObservation;
    out.observation.psxAddress = window.psxAddress;
    out.observation.byteSize = window.byteSize;
    out.observation.valueKnown = window.valueKnown;
    out.observation.value = window.value;
    out.observation.frameKnown = window.frameKnown;
    out.observation.frame = window.frame;
    out.observation.pcKnown = window.pcKnown;
    out.observation.pc = window.pc;
    return out;
}

PrStage1XaCdDirectCallbackPendingRuntimeSource80055F7A
PrStage1XaCdDirectBuildCallbackPendingRuntimeSource80055F7A(
    const PrStage1XaCdDirectCallbackPendingRuntimeSourceWindow80055F7A& window) {
    PrStage1XaCdDirectCallbackPendingRuntimeSource80055F7A out{};
    out.sourceAvailable =
        window.providerInstalled &&
        window.windowReadable &&
        window.psxAddress == kCallbackPendingObservationPsxAddress80055F7A &&
        window.byteSize == kCallbackPendingObservationByteSize80055F7A;
    out.valueKnown = window.valueKnown;
    if (!out.sourceAvailable) {
        return out;
    }

    out.observation.source =
        PrStage1XaCdDirectCallbackPendingObservationSource80055F7A::
            RuntimePsxMemoryObservation;
    out.observation.psxAddress = window.psxAddress;
    out.observation.byteSize = window.byteSize;
    out.observation.valueKnown = window.valueKnown;
    out.observation.value = window.value;
    out.observation.frameKnown = window.frameKnown;
    out.observation.frame = window.frame;
    out.observation.pcKnown = window.pcKnown;
    out.observation.pc = window.pc;
    return out;
}

PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC
PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(
    const PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC& window) {
    PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC out{};
    out.sourceAvailable =
        window.providerInstalled &&
        window.deadlineReadable &&
        window.spinReadable &&
        window.deadlinePsxAddress ==
            kCommandWaitLoopDeadlinePsxAddress80088310 &&
        window.deadlineByteSize ==
            kCommandWaitLoopRuntimeSourceByteSize800375BC &&
        window.spinPsxAddress == kCommandWaitLoopSpinPsxAddress80088314 &&
        window.spinByteSize == kCommandWaitLoopRuntimeSourceByteSize800375BC;
    out.resultKnown = window.resultKnown;
    out.psxReturn = window.psxReturn;
    out.frameKnown = window.frameKnown;
    out.frame = window.frame;
    out.pcKnown = window.pcKnown;
    out.pc = window.pc;
    return out;
}

PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108
PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108 window{};
    window.providerInstalled = provider.installed && provider.read != nullptr;
    window.psxAddress = kStatusFlagsObservationPsxAddress80057108;
    window.byteSize = kStatusFlagsObservationByteSize80057108;
    window.frameKnown = provider.frameKnown;
    window.frame = provider.frame;
    window.pcKnown = provider.pcKnown;
    window.pc = provider.pc;
    if (!window.providerInstalled) {
        return window;
    }

    uint8_t bytes[kStatusFlagsObservationByteSize80057108] = {};
    window.readAttempted = true;
    if (!provider.read(provider.userData,
                       window.psxAddress,
                       window.byteSize,
                       bytes,
                       sizeof(bytes))) {
        return window;
    }

    window.windowReadable = true;
    window.valueKnown = true;
    window.value = static_cast<uint32_t>(bytes[0]) |
                   (static_cast<uint32_t>(bytes[1]) << 8) |
                   (static_cast<uint32_t>(bytes[2]) << 16) |
                   (static_cast<uint32_t>(bytes[3]) << 24);
    return window;
}

PrStage1XaCdDirectStatusCounterRuntimeSourceWindow80057110
PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectStatusCounterRuntimeSourceWindow80057110 window{};
    window.providerInstalled = provider.installed && provider.read != nullptr;
    window.psxAddress = kStatusCounterObservationPsxAddress80057110;
    window.byteSize = kStatusCounterObservationByteSize80057110;
    window.frameKnown = provider.frameKnown;
    window.frame = provider.frame;
    window.pcKnown = provider.pcKnown;
    window.pc = provider.pc;
    if (!window.providerInstalled) {
        return window;
    }

    uint8_t bytes[kStatusCounterObservationByteSize80057110] = {};
    window.readAttempted = true;
    if (!provider.read(provider.userData,
                       window.psxAddress,
                       window.byteSize,
                       bytes,
                       sizeof(bytes))) {
        return window;
    }

    window.windowReadable = true;
    window.valueKnown = true;
    window.value = static_cast<uint32_t>(bytes[0]) |
                   (static_cast<uint32_t>(bytes[1]) << 8) |
                   (static_cast<uint32_t>(bytes[2]) << 16) |
                   (static_cast<uint32_t>(bytes[3]) << 24);
    return window;
}

PrStage1XaCdDirectCallbackPendingRuntimeSourceWindow80055F7A
PrStage1XaCdDirectReadCallbackPendingRuntimeSourceWindow80055F7A(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectCallbackPendingRuntimeSourceWindow80055F7A window{};
    window.providerInstalled = provider.installed && provider.read != nullptr;
    window.psxAddress = kCallbackPendingObservationPsxAddress80055F7A;
    window.byteSize = kCallbackPendingObservationByteSize80055F7A;
    window.frameKnown = provider.frameKnown;
    window.frame = provider.frame;
    window.pcKnown = provider.pcKnown;
    window.pc = provider.pc;
    if (!window.providerInstalled) {
        return window;
    }

    uint8_t bytes[kCallbackPendingObservationByteSize80055F7A] = {};
    window.readAttempted = true;
    if (!provider.read(provider.userData,
                       window.psxAddress,
                       window.byteSize,
                       bytes,
                       sizeof(bytes))) {
        return window;
    }

    window.windowReadable = true;
    window.valueKnown = true;
    window.value = static_cast<uint16_t>(bytes[0]) |
                   static_cast<uint16_t>(
                       static_cast<uint16_t>(bytes[1]) << 8);
    return window;
}

PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC
PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider,
    bool clockKnown,
    int32_t clockNow) {
    PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC window{};
    window.providerInstalled = provider.installed && provider.read != nullptr;
    window.deadlinePsxAddress = kCommandWaitLoopDeadlinePsxAddress80088310;
    window.deadlineByteSize = kCommandWaitLoopRuntimeSourceByteSize800375BC;
    window.spinPsxAddress = kCommandWaitLoopSpinPsxAddress80088314;
    window.spinByteSize = kCommandWaitLoopRuntimeSourceByteSize800375BC;
    window.clockKnown = clockKnown;
    window.clockNow = clockNow;
    window.frameKnown = provider.frameKnown;
    window.frame = provider.frame;
    window.pcKnown = provider.pcKnown;
    window.pc = provider.pc;
    if (!window.providerInstalled) {
        return window;
    }

    uint8_t deadlineBytes[kCommandWaitLoopRuntimeSourceByteSize800375BC] = {};
    window.deadlineReadAttempted = true;
    if (provider.read(provider.userData,
                      window.deadlinePsxAddress,
                      window.deadlineByteSize,
                      deadlineBytes,
                      sizeof(deadlineBytes))) {
        window.deadlineReadable = true;
        window.deadlineClock = ReadU32LE(deadlineBytes);
    }

    uint8_t spinBytes[kCommandWaitLoopRuntimeSourceByteSize800375BC] = {};
    window.spinReadAttempted = true;
    if (provider.read(provider.userData,
                      window.spinPsxAddress,
                      window.spinByteSize,
                      spinBytes,
                      sizeof(spinBytes))) {
        window.spinReadable = true;
        window.spinCount = ReadU32LE(spinBytes);
    }

    if (!window.deadlineReadable || !window.spinReadable || !clockKnown) {
        return window;
    }

    const bool deadlineExpired =
        static_cast<int32_t>(window.deadlineClock) < clockNow;
    const bool spinExceeded =
        window.spinCount > kCommandWaitLoopSpinLimit800375BC;
    if (!deadlineExpired && !spinExceeded) {
        return window;
    }

    window.resultKnown = true;
    window.psxReturn = -1;
    return window;
}

PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8
PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider) {
    PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8 window{};
    window.providerInstalled = provider.installed && provider.read != nullptr;
    window.cdReg3InitialPsxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    window.cdReg3InitialByteSize = 1u;
    window.cdReg0StatusPsxAddress =
        PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8;
    window.cdReg0StatusByteSize = 1u;
    window.frameKnown = provider.frameKnown;
    window.frame = provider.frame;
    window.pcKnown = provider.pcKnown;
    window.pc = provider.pc;
    if (!window.providerInstalled) {
        return window;
    }

    uint8_t cdReg3InitialByte = 0u;
    window.cdReg3InitialReadAttempted = true;
    if (provider.read(provider.userData,
                      window.cdReg3InitialPsxAddress,
                      window.cdReg3InitialByteSize,
                      &cdReg3InitialByte,
                      1u)) {
        window.cdReg3InitialReadable = true;
        window.cdReg3InitialValueKnown = true;
        window.cdReg3InitialValue = cdReg3InitialByte;
    }

    uint8_t cdReg0StatusByte = 0u;
    window.cdReg0StatusReadAttempted = true;
    if (provider.read(provider.userData,
                      window.cdReg0StatusPsxAddress,
                      window.cdReg0StatusByteSize,
                      &cdReg0StatusByte,
                      1u)) {
        window.cdReg0StatusReadable = true;
        window.cdReg0StatusValueKnown = true;
        window.cdReg0StatusValue = cdReg0StatusByte;
    }
    return window;
}

PrStage1XaCdDirectRawEventInitialInterruptRuntimeSource80036AF8
PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
    const PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8& window) {
    PrStage1XaCdDirectRawEventInitialInterruptRuntimeSource80036AF8 out{};
    out.sourceAvailable =
        window.providerInstalled &&
        window.cdReg3InitialReadable &&
        window.cdReg3InitialPsxAddress ==
            PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8 &&
        window.cdReg3InitialByteSize == 1u;
    out.valueKnown = window.cdReg3InitialValueKnown;
    if (!out.sourceAvailable) {
        return out;
    }

    out.observation.source =
        PrStage1XaCdDirectRawEventInitialInterruptObservationSource80036AF8::
            RuntimePsxMemoryObservation;
    out.observation.psxAddress = window.cdReg3InitialPsxAddress;
    out.observation.byteSize = window.cdReg3InitialByteSize;
    out.observation.valueKnown = window.cdReg3InitialValueKnown;
    out.observation.value = window.cdReg3InitialValue;
    out.observation.frameKnown = window.frameKnown;
    out.observation.frame = window.frame;
    out.observation.pcKnown = window.pcKnown;
    out.observation.pc = window.pc;
    return out;
}

PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108
PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(
    const PrStage1XaCdDirectStatusFlagsRuntimeSource80057108& source,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108 out{};
    out.sourceAvailable = source.sourceAvailable;
    out.valueKnown = source.valueKnown;
    if (!source.sourceAvailable) {
        out.blocker =
            PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
                SourceUnavailable;
        return out;
    }
    if (!source.valueKnown) {
        out.blocker =
            PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
                ValueUnknown;
        return out;
    }
    out.publishAttempted = true;
    out.observation =
        PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(
            source.observation,
            state);
    if (!out.observation.accepted) {
        out.blocker =
            PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::
                ObservationRejected;
    }
    return out;
}

PrStage1XaCdDirectStatusCounterRuntimeSourceResult80057110
PrStage1XaCdDirectPublishStatusCounterRuntimeSource80057110(
    const PrStage1XaCdDirectStatusCounterRuntimeSource80057110& source,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectStatusCounterRuntimeSourceResult80057110 out{};
    out.sourceAvailable = source.sourceAvailable;
    out.valueKnown = source.valueKnown;
    if (!source.sourceAvailable) {
        out.blocker =
            PrStage1XaCdDirectStatusCounterRuntimeSourceBlocker80057110::
                SourceUnavailable;
        return out;
    }
    if (!source.valueKnown) {
        out.blocker =
            PrStage1XaCdDirectStatusCounterRuntimeSourceBlocker80057110::
                ValueUnknown;
        return out;
    }
    out.publishAttempted = true;
    out.observation =
        PrStage1XaCdDirectApplyStatusCounterRuntimeObservation80057110(
            source.observation,
            state);
    if (!out.observation.accepted) {
        out.blocker =
            PrStage1XaCdDirectStatusCounterRuntimeSourceBlocker80057110::
                ObservationRejected;
    }
    return out;
}

PrStage1XaCdDirectCallbackPendingRuntimeSourceResult80055F7A
PrStage1XaCdDirectPublishCallbackPendingRuntimeSource80055F7A(
    const PrStage1XaCdDirectCallbackPendingRuntimeSource80055F7A& source,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectCallbackPendingRuntimeSourceResult80055F7A out{};
    out.sourceAvailable = source.sourceAvailable;
    out.valueKnown = source.valueKnown;
    if (!source.sourceAvailable) {
        out.blocker =
            PrStage1XaCdDirectCallbackPendingRuntimeSourceBlocker80055F7A::
                SourceUnavailable;
        return out;
    }
    if (!source.valueKnown) {
        out.blocker =
            PrStage1XaCdDirectCallbackPendingRuntimeSourceBlocker80055F7A::
                ValueUnknown;
        return out;
    }
    out.publishAttempted = true;
    out.observation =
        PrStage1XaCdDirectApplyCallbackPendingRuntimeObservation80055F7A(
            source.observation,
            state);
    if (!out.observation.accepted) {
        out.blocker =
            PrStage1XaCdDirectCallbackPendingRuntimeSourceBlocker80055F7A::
                ObservationRejected;
    }
    return out;
}

PrStage1XaCdDirectCommandWaitLoopRuntimeSourceResult800375BC
PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(
    const PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC& source) {
    PrStage1XaCdDirectCommandWaitLoopRuntimeSourceResult800375BC out{};
    out.sourceAvailable = source.sourceAvailable;
    out.resultKnown = source.resultKnown;
    out.psxReturn = source.psxReturn;
    if (!source.sourceAvailable) {
        out.blocker =
            PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC::
                SourceUnavailable;
        return out;
    }
    if (!source.resultKnown) {
        out.blocker =
            PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC::
                ResultUnknown;
        return out;
    }
    out.feedsCommand = true;
    return out;
}

PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceResult80036AF8
PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
    const PrStage1XaCdDirectRawEventInitialInterruptRuntimeSource80036AF8&
        source,
    PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceResult80036AF8 out{};
    out.sourceAvailable = source.sourceAvailable;
    out.valueKnown = source.valueKnown;
    if (!source.sourceAvailable) {
        out.blocker =
            PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8::
                SourceUnavailable;
        return out;
    }
    if (!source.valueKnown) {
        out.blocker =
            PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8::
                ValueUnknown;
        return out;
    }
    out.publishAttempted = true;
    out.observation =
        PrStage1XaCdDirectApplyRawEventInitialInterruptRuntimeObservation80036AF8(
            source.observation,
            state);
    if (!out.observation.accepted) {
        out.blocker =
            PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8::
                ObservationRejected;
        return out;
    }
    out.blocker =
        PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8::
            None;
    return out;
}

PrStage1XaCdDirectRawEventRuntimeSourceResult80036AF8
PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(
    const PrStage1XaCdDirectRawEventRuntimeSource80036AF8& source,
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider* provider) {
    PrStage1XaCdDirectRawEventRuntimeSourceResult80036AF8 out{};
    out.sourceAvailable = source.sourceAvailable;
    out.transactionKnown = source.transactionKnown;
    if (!source.sourceAvailable) {
        out.blocker =
            PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::
                SourceUnavailable;
        return out;
    }
    if (!source.transactionKnown) {
        out.blocker =
            PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::
                TransactionUnknown;
        return out;
    }

    if (provider != nullptr) {
        out.cdMmioSubmitAttempted = true;
        out.cdMmioSubmitAccepted =
            PrStage1XaCdDirectSubmitCdMmioSnapshotSamplesFromRawEvent80036AF8(
                source.transaction,
                *provider);
    }

    out.publishAttempted = true;
    PrStage1XaCdDirectLowerCdSnapshotBridgeInput bridgeInput{};
    bridgeInput.rawEvent80036AF8Known = true;
    bridgeInput.rawEvent80036AF8 = source.transaction;
    out.bridge = PrStage1XaCdDirectBuildLowerCdProducerSnapshot(bridgeInput);
    if (!out.bridge.produced || out.bridge.incomplete ||
        (!out.bridge.lowerEventRegisters80036AF8Bridged &&
         !out.bridge.lowerEventEarlyReturnNoInterrupt)) {
        out.blocker =
            PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::
                BridgeRejected;
        return out;
    }

    out.earlyReturnNoInterrupt = out.bridge.lowerEventEarlyReturnNoInterrupt;
    out.accepted = true;
    if (out.bridge.lowerEventRegisters80036AF8Bridged) {
        out.applied =
            PrStage1XaCdDirectApplyLowerCdProducerSnapshot(state,
                                                          out.bridge.snapshot);
        out.lowerEventApplied = out.applied.lowerEvent80036AF8Applied;
    }
    return out;
}

PrStage1XaCdDirectRawEventRuntimeSource80036AF8
PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(
    const PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectRawEventRuntimeSource80036AF8 out{};
    out.sourceAvailable = true;

    PrStage1LowerCdProducerDirect::RawCdRegTransactionInput80036AF8 input{};
    input.registerPointers.cdReg0PtrKnown = true;
    input.registerPointers.cdReg1PtrKnown = true;
    input.registerPointers.cdReg2PtrKnown = true;
    input.registerPointers.cdReg3PtrKnown = true;
    input.registerPointers.selectorWriteKnown = true;
    input.registerPointers.selectorWriteValue = 1u;
    input.cdReg3InitialInterruptKnown =
        state.rawEvent80036AF8InitialInterruptKnown;
    input.cdReg3InitialInterrupt =
        state.rawEvent80036AF8InitialInterrupt;
    input.cdReg3StableInterruptKnown =
        state.rawEvent80036AF8StableInterruptKnown;
    input.cdReg3StableInterrupt =
        state.rawEvent80036AF8StableInterrupt;
    input.cdReg0StatusKnown = state.rawEvent80036AF8CdReg0StatusKnown;
    input.cdReg0Status = state.rawEvent80036AF8CdReg0Status;
    input.cdReg0FifoStatusSamplesKnown =
        state.rawEvent80036AF8FifoStatusSamplesKnown;
    input.cdReg0FifoStatusSampleCount =
        state.rawEvent80036AF8FifoStatusSampleCount;
    const uint32_t fifoCount =
        (std::min)(input.cdReg0FifoStatusSampleCount,
                   static_cast<uint32_t>(
                       state.rawEvent80036AF8FifoStatusSamples.size()));
    for (uint32_t i = 0u; i < fifoCount; ++i) {
        input.cdReg0FifoStatusSamples[i] =
            state.rawEvent80036AF8FifoStatusSamples[i];
    }
    input.fifoDrainKnown = state.rawEvent80036AF8FifoDrainKnown;
    input.fifoDrainCount = state.rawEvent80036AF8FifoDrainCount;
    input.fifoDrained = state.rawEvent80036AF8FifoDrained;
    input.resultByteCountKnown =
        state.rawEvent80036AF8ResultByteCountKnown;
    input.resultByteCount = state.rawEvent80036AF8ResultByteCount;
    input.resultBytesKnown = state.rawEvent80036AF8ResultBytesKnown;
    const uint32_t resultCount =
        (std::min)(input.resultByteCount,
                   static_cast<uint32_t>(
                       state.rawEvent80036AF8ResultBytes.size()));
    for (uint32_t i = 0u; i < resultCount; ++i) {
        input.resultBytes[i] = state.rawEvent80036AF8ResultBytes[i];
    }
    input.ackWritesKnown = state.rawEvent80036AF8AckWritesKnown;
    input.case1ClearWritesKnown =
        state.rawEvent80036AF8Case1ClearWritesKnown;
    input.case1ClearCdReg0 = state.rawEvent80036AF8Case1ClearCdReg0;
    input.case1ClearCdReg3 = state.rawEvent80036AF8Case1ClearCdReg3;
    input.priorFacts.dword80057108Known = state.dword_80057108Known;
    input.priorFacts.dword80057108 = state.dword_80057108;
    input.priorFacts.dword80057110Known = state.dword_80057110Known;
    input.priorFacts.dword80057110 = state.dword_80057110;
    input.priorFacts.byte80057119Known = state.byte_80057119Known;
    input.priorFacts.byte80057119 = state.byte_80057119;

    out.transaction =
        PrStage1LowerCdProducerDirect::BuildRawCdRegTransactionResult80036AF8(
            input);
    out.transactionKnown = out.transaction.produced &&
                           !out.transaction.incomplete;
    return out;
}

PrStage1XaCdDirectRawEventTypedSourceAudit80036AF8
PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(
    const PrStage1XaCdDirectState& state) {
    PrStage1XaCdDirectRawEventTypedSourceAudit80036AF8 out{};
    out.inspected = true;
    out.registerPointersKnown = true;
    out.initialInterruptKnown = state.rawEvent80036AF8InitialInterruptKnown;
    const bool earlyReturnNoInterrupt =
        out.initialInterruptKnown &&
        ((state.rawEvent80036AF8InitialInterrupt & 7u) == 0u);
    out.stableInterruptKnown =
        earlyReturnNoInterrupt ||
        state.rawEvent80036AF8StableInterruptKnown;
    out.cdReg0StatusKnown =
        earlyReturnNoInterrupt ||
        state.rawEvent80036AF8CdReg0StatusKnown;
    out.fifoStatusSamplesKnown =
        earlyReturnNoInterrupt ||
        state.rawEvent80036AF8FifoStatusSamplesKnown;
    out.resultByteCountKnown =
        earlyReturnNoInterrupt ||
        state.rawEvent80036AF8ResultByteCountKnown;
    out.resultBytesKnown =
        earlyReturnNoInterrupt ||
        state.rawEvent80036AF8ResultBytesKnown ||
        (state.rawEvent80036AF8ResultByteCountKnown &&
         state.rawEvent80036AF8ResultByteCount == 0u);
    out.ackWritesKnown =
        earlyReturnNoInterrupt ||
        state.rawEvent80036AF8AckWritesKnown;
    out.priorDword80057108Known = state.dword_80057108Known;
    out.priorDword80057110Known = state.dword_80057110Known;
    out.priorByte80057119Known = state.byte_80057119Known;
    if (earlyReturnNoInterrupt) {
        out.priorDword80057108Known = true;
        out.priorDword80057110Known = true;
        out.priorByte80057119Known = true;
    }

    if (!out.registerPointersKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapRegisterPointers80036AF8;
    }
    if (!out.initialInterruptKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapInitialInterrupt80036AF8;
    }
    if (!out.stableInterruptKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapStableInterrupt80036AF8;
    }
    if (!out.cdReg0StatusKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapCdReg0Status80036AF8;
    }
    if (!out.fifoStatusSamplesKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapFifoStatusSamples80036AF8;
    }
    if (!out.resultByteCountKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapResultByteCount80036AF8;
    }
    if (!out.resultBytesKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapResultBytes80036AF8;
    }
    if (!out.ackWritesKnown) {
        out.missingMask |=
            kPrStage1XaCdDirectTypedSourceGapAckWrites80036AF8;
    }
    if (!out.priorDword80057108Known) {
        out.missingMask |= kPrStage1XaCdDirectTypedSourceGapPriorDword80057108;
    }
    if (!out.priorDword80057110Known) {
        out.missingMask |= kPrStage1XaCdDirectTypedSourceGapPriorDword80057110;
    }
    if (!out.priorByte80057119Known) {
        out.missingMask |= kPrStage1XaCdDirectTypedSourceGapPriorByte80057119;
    }
    if (!out.registerPointersKnown) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                RegisterPointers;
    } else if (!out.initialInterruptKnown) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                InitialInterrupt;
    } else if (!out.stableInterruptKnown) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                StableInterrupt;
    } else if (!out.cdReg0StatusKnown) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                CdReg0Status;
    } else if (!out.fifoStatusSamplesKnown) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                FifoStatusSamples;
    } else if (!out.resultByteCountKnown) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                ResultByteCount;
    } else if (!out.resultBytesKnown) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                ResultBytes;
    } else if (!out.ackWritesKnown) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                AckWrites;
    } else if (!out.priorDword80057108Known) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                PriorDword80057108;
    } else if (!out.priorDword80057110Known) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                PriorDword80057110;
    } else if (!out.priorByte80057119Known) {
        out.firstMissing =
            PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
                PriorByte80057119;
    }
    const auto source = PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(
        state);
    out.typedCanReconstructTransaction =
        out.missingMask == 0u && source.transactionKnown;
    out.canFeedRuntimeSource = out.typedCanReconstructTransaction;
    return out;
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
    PublishStatusByteRuntimeObservation800573D4(
        state,
        0u,
        PrMovieSegmentDirect::kSub800375BCCdCommand);
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
    // lw old, sw new: Hex-Rays folds this mutable global to zero in SCUS.
    out.psxReturnKnown = state.dword_800570F8Known;
    out.psxReturn = static_cast<int32_t>(state.dword_800570F8);
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

PrStage1XaCdDirectReadyCallbackRegisterResult80036528
PrStage1XaCdDirectApplySub80036528SetCdReadyCallback(
    PrStage1XaCdDirectState& state,
    uint32_t callbackAddr,
    uint32_t sourceFunction) {
    PrStage1XaCdDirectReadyCallbackRegisterResult80036528 out{};
    out.called = true;
    out.sourceFunction = sourceFunction;
    out.psxReturnKnown = state.dword_800570FCKnown;
    out.psxReturn = static_cast<int32_t>(state.dword_800570FC);
    out.dword800570FCKnown = true;
    out.dword800570FC = callbackAddr;
    out.callback80039240Installed =
        callbackAddr == kSub80039240PumpCallback;
    out.callbackCleared = callbackAddr == 0u;

    state.dword_800570FCKnown = true;
    state.dword_800570FC = callbackAddr;
    state.callback80039240Installed = out.callback80039240Installed;
    state.readyCallbackRegisterSourceFunction = sourceFunction;
    ++state.readyCallbackRegisterWriteCount;
    if (out.callbackCleared) {
        ++state.readyCallbackRegisterClearCount;
    }
    if (out.callback80039240Installed) {
        ++state.readyCallbackRegisterInstall39240Count;
    }
    return out;
}

PrStage1XaCdDirectCallbackStateTableSetResult80035BA0
PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
    PrStage1XaCdDirectState& state,
    uint32_t callbackIndex,
    bool callbackAddrKnown,
    uint32_t callbackAddr,
    uint32_t sourceFunction) {
    PrStage1XaCdDirectCallbackStateTableSetResult80035BA0 out{};
    out.called = true;
    out.sourceFunction = sourceFunction;
    out.callbackIndex = callbackIndex;
    out.callbackAddrKnown = callbackAddrKnown;
    out.callbackAddr = callbackAddr;

    if (callbackIndex >=
        PrMovieSegmentDirect::kCdCallbackPendingProducerCallbackCount800359B8) {
        out.gapInvalidCallbackIndex = true;
        ++state.callbackStateTable80035BA0GapCount;
        return out;
    }
    if (!callbackAddrKnown) {
        out.gapMissingCallbackAddr = true;
        ++state.callbackStateTable80035BA0GapCount;
        return out;
    }
    if (!state.word_80055F78Known) {
        out.gapMissingWord80055F78 = true;
        ++state.callbackStateTable80035BA0GapCount;
        return out;
    }
    if (state.word_80055F78 == 0u) {
        out.gateClosedWord80055F78 = true;
        ++state.callbackStateTable80035BA0GapCount;
        return out;
    }
    if (!state.word_80055FA8Known) {
        out.gapMissingWord80055FA8 = true;
        ++state.callbackStateTable80035BA0GapCount;
        return out;
    }

    if (state.callbackStateTable80055F7CAddressKnown[callbackIndex]) {
        out.oldCallbackKnown = true;
        out.oldCallback = state.callbackStateTable80055F7CAddress[callbackIndex];
        out.psxReturn = static_cast<int32_t>(out.oldCallback);
        out.noChange = out.oldCallback == callbackAddr;
    }

    state.callbackStateTable80055F7CAddressKnown[callbackIndex] = true;
    state.callbackStateTable80055F7CAddress[callbackIndex] = callbackAddr;
    if (callbackAddr != 0u) {
        state.word_80055FA8 = static_cast<uint16_t>(
            state.word_80055FA8 | (1u << callbackIndex));
    } else {
        state.word_80055FA8 = static_cast<uint16_t>(
            state.word_80055FA8 & ~(1u << callbackIndex));
    }
    state.word_80055FA8Known = true;
    state.callbackStateTable80055F7CKnown =
        AreCallbackStateTableSlotsKnown80055F7C(state);
    state.callbackStateTable80035BA0SourceFunction = sourceFunction;
    ++state.callbackStateTable80035BA0ApplyCount;

    out.applied = true;
    out.callbackStateTableKnown = state.callbackStateTable80055F7CKnown;
    out.word80055FA8Known = true;
    out.word80055FA8 = state.word_80055FA8;
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

PrStage1CdStopStep8001A694 PrStage1XaCdDirectAdvanceStop8001A694(
    PrStage1CdStopRuntime8001A694& runtime,
    PrStage1XaCdDirectState& state,
    const PrStage1CdStopFeedback8001A694& feedback) {
    using Phase = PrStage1CdStopPhase8001A694;
    PrStage1CdStopStep8001A694 out{};
    if (runtime.phase == Phase::Complete) {
        out.complete = true;
        out.feedbackRejected = feedback.known;
        return out;
    }
    const auto writeCallback = [&](uint32_t value) {
        // 367A4 uses direct sw, not 36510. Keep the shared callback mirror
        // coherent without erasing ready callbacks, CD status or ring state.
        state.dword_800570F8Known = true;
        state.dword_800570F8 = value;
        state.streamClockCallback8001A210Registered =
            value == PrMovieSegmentDirect::kSub8001A210StreamClockCallback;
        state.callbackRegisterSourceFunction = 0x800367A4u;
    };
    const auto issue = [&](Phase phase, uint32_t function,
                           std::array<uint32_t, 4> args) {
        runtime.phase = phase;
        runtime.request = {++runtime.requestSerial, function, args};
        out.requestIssued = true;
    };
    const auto newOuterCall = [&]() {
        if (!state.dword_800570F8Known) return false;
        runtime.savedCallback800570F8 = state.dword_800570F8;
        runtime.attempts800367A4 = 0;
        ++runtime.calls800367A4;
        runtime.phase = Phase::BeginAttempt;
        return true;
    };
    const bool waiting = runtime.phase == Phase::StatusCommand ||
        runtime.phase == Phase::StopCommand || runtime.phase == Phase::Sync;
    if (feedback.known) {
        if (!waiting || feedback.request.serial != runtime.request.serial ||
            feedback.request.function != runtime.request.function ||
            feedback.request.args != runtime.request.args) {
            out.feedbackRejected = true;
            return out;
        }
        out.feedbackConsumed = true;
        if (runtime.phase == Phase::StatusCommand) {
            // Recovery command1 return is ignored at80036828.
            writeCallback(runtime.savedCallback800570F8);
            issue(Phase::StopCommand, 0x800375BCu, {8u, 0u, 0x80049414u, 0u});
            return out;
        }
        if (runtime.phase == Phase::StopCommand) {
            if (feedback.result == 0) {
                issue(Phase::Sync, 0x80037070u, {0u, 0x80049414u, 0u, 0u});
                return out;
            }
            ++runtime.attempts800367A4;
            // s0 starts3: four failed sends return0 from367A4;1A694 retries.
            if (runtime.attempts800367A4 == 4u) {
                writeCallback(runtime.savedCallback800570F8);
                (void)newOuterCall(); // saved callback was just restored.
            } else {
                runtime.phase = Phase::BeginAttempt;
            }
        } else if (feedback.result == 2) {
            const auto cleared = PrStage1XaCdDirectApplySub80036510SetCdCallback(
                state, 0u, 0x8001A694u);
            runtime.returnKnown = cleared.psxReturnKnown;
            runtime.result = cleared.psxReturn;
            runtime.phase = Phase::Complete;
            out.complete = true;
            return out;
        } else {
            // Failed synchronous completion retries the outer call, unbounded.
            if (!newOuterCall()) {
                runtime.phase = Phase::Idle;
                out.waitingForSource = true;
                return out;
            }
        }
    } else if (waiting) {
        return out;
    }
    if (runtime.phase == Phase::Idle && !newOuterCall()) {
        out.waitingForSource = true;
        return out;
    }
    if (!state.dword_80057108Known) {
        out.waitingForSource = true;
        return out;
    }
    writeCallback(0u);
    if ((state.dword_80057108 & 0x10u) != 0u) {
        issue(Phase::StatusCommand, 0x800375BCu, {1u, 0u, 0u, 0u});
    } else {
        writeCallback(runtime.savedCallback800570F8);
        issue(Phase::StopCommand, 0x800375BCu, {8u, 0u, 0x80049414u, 0u});
    }
    return out;
}

void PrStage1XaCdDirectDispatchCommandCallbacks800375BC(PrStage1XaCdDirectState& state) {
    // 800378F8..80037920 uses the same 36AF8 result-mask callback dispatch
    // as 80037214..8003723C. The event serial prevents replay on a host poll.
    DispatchCdCallbacksFromSub80037070(state);
}

PrMovieSegmentDirect::CdSyncResult80037070 PrStage1XaCdDirectApplySub80037070(
    PrStage1XaCdDirectState& state, const PrStage1XaCdDirectCdSyncInput80037070& input) {
    DispatchCdCallbacksFromSub80037070(state);
    const auto result = PrMovieSegmentDirect::PsxCall80037070_CdSync(
        input.a0WaitMode, input.a1OutputBufferPtrNonNull, input.feedback);
    if (result.syncResultKnown) {
        state.cdSync80037070Known = true;
        state.cdSync80037070 = result;
        if (result.psxReturn == 2 || result.psxReturn == 5) {
            // 80037280 consumes status 2/5 by storing 2, while returning the
            // original status to the caller. Do not erase response banks.
            PublishStatusByteRuntimeObservation800573D4(state, 2u, 0x80037070u);
        }
    }
    return result;
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
    auto result = PrStage1XaCdDirectApplySub80037070(state, input);
    result.sourceFunction = PrMovieSegmentDirect::kSub800364D0CdSyncWrapper;
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
    ++state.lowerCdSnapshotApplyCount;

    const bool hasCallbackSnapshot =
        snapshot.pendingProducer800359B8Known ||
        snapshot.lowerEvent80036AF8Known ||
        snapshot.lowerEventRegisters80036AF8Known;
    if (snapshot.pendingProducer800359B8Known) {
        ++state.lowerCdSnapshotPendingProducerCount;
        out.pendingProducer800359B8 =
            PrStage1XaCdDirectApplySub800359B8CdCallbackPendingProducer(
                state,
                snapshot.pendingProducer800359B8);
        out.pendingProducer800359B8Applied =
            out.pendingProducer800359B8.called;
    }

    if (hasCallbackSnapshot) {
        ++state.lowerCdSnapshotCallbackEventCount;
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
        ++state.lowerCdSnapshotSyncFeedbackCount;
        state.cdLowerFeedback80036AF8Known = true;
        state.cdLowerFeedback80036AF8 =
            snapshot.cdSyncFeedback80037070;
        state.cdLowerFeedback80036AF8FromGetlocP = false;
        out.cdSyncFeedback80037070Applied = true;
    }
    out.readyForCdSync80037070 = state.cdLowerFeedback80036AF8Known;
    if (snapshot.cdSeamResultKnown) {
        ++state.lowerCdSnapshotSeamResultCount;
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
    PublishStatusByteRuntimeObservation800573D4(
        state,
        0u,
        PrMovieSegmentDirect::kSub800375BCCdCommand);
    // 800376C4 clears the command-completion byte573D4, not drive status
    // dword57108. Retain that status until the next actual36AF8 response.
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
    out.waitLoopRequested = !input.skipWait;
    if (input.skipWait) {
        out.psxReturnKnown = true;
        out.psxReturn = 0;
        return out;
    }

    if (input.clockKnown) {
        PrStage1XaCdDirectPrimeSub800375BCTimeoutState(state,
                                                       input.clockNow);
    }

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

    if (!input.waitLoopResultKnown) {
        out.incomplete = true;
        return out;
    }

    out.waitLoopResultKnown = true;
    out.waitLoopPsxReturn = input.waitLoopPsxReturn;
    out.psxReturnKnown = true;
    out.psxReturn = input.waitLoopPsxReturn;
    return out;
}

void PrStage1XaCdDirectPrimeSub800375BCTimeoutState(
    PrStage1XaCdDirectState& state,
    int32_t clockNow) {
    state.cdCommandTimeoutDeadline80088310Known = true;
    state.cdCommandTimeoutDeadline80088310 = clockNow + 960;
    state.cdCommandTimeoutSpin80088314Known = true;
    state.cdCommandTimeoutSpin80088314 = 0u;
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
        PublishStatusByteRuntimeObservation800573D4(
            state,
            0u,
            PrMovieSegmentDirect::kSub800375BCCdCommand);
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
    state.lastHalGetlocPSource = input.source;
    if (input.sectorIndexKnown) {
        state.lastHalGetlocPSectorIndexKnown = true;
        state.lastHalGetlocPSectorIndex = input.sectorIndex;
    }
    state.lastHalGetlocPDataReadyInterruptKnown = true;
    state.lastHalGetlocPDataReadyInterrupt = input.cdDataReadyInterrupt;

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
    ++state.halGetlocLowerBridgeCount;
    state.halGetlocLowerBridgeCdSyncCoreCount +=
        snapshot.cdSyncCoreFacts80037070Bridged ? 1u : 0u;
    state.halGetlocLowerBridgePendingProducerCount +=
        snapshot.pendingProducer800359B8Bridged ? 1u : 0u;
    state.halGetlocLowerBridgeCallbackEventCount +=
        (snapshot.lowerEventRegisters80036AF8Bridged ||
         snapshot.lowerEventEarlyReturnNoInterrupt)
            ? 1u
            : 0u;

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
