#include "pr_movie_segment_direct.h"

namespace PrMovieSegmentDirect {

namespace {

constexpr std::array<uint32_t, 64> kCdEventStatusUpdateTable800572BC = {
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
    0u, 0u, 1u, 1u, 1u, 1u, 1u, 1u,
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
    0u, 0u, 3u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u, 0u, 2u, 1u, 0u,
    0u, 0u, 1u, 0u, 1u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
};

constexpr std::array<uint32_t, 64> kCdEventStatus3Table800571BC = {
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 1u,
    1u, 1u, 1u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 1u, 0u, 0u, 1u, 1u, 0u,
    0u, 0u, 1u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 1u, 1u, 1u, 1u, 0u,
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u, 0u, 1u, 1u, 0u,
    0u, 0u, 0u, 1u, 0u, 0u, 0u, 0u,
};

bool TryReadCdEventTable80036AF8(
    const std::array<uint32_t, 64>& table,
    uint8_t command,
    uint32_t* value) {
    if (command >= table.size()) {
        return false;
    }
    if (value != nullptr) {
        *value = table[command];
    }
    return true;
}

} // namespace

uint32_t ComputeMovieSegmentRowOffset801C4780(uint32_t rowIndex) {
    return kSceneEntryMovieSegmentOffset801C4780 +
           rowIndex * kMovieSegmentRecordSize8001A324;
}

bool TryComputeMovieSegmentRowIndex801C4780(uint32_t rowOffset,
                                            uint32_t* rowIndex) {
    if (rowOffset < kSceneEntryMovieSegmentOffset801C4780) {
        return false;
    }

    const uint32_t delta =
        rowOffset - kSceneEntryMovieSegmentOffset801C4780;
    if ((delta % kMovieSegmentRecordSize8001A324) != 0u) {
        return false;
    }

    const uint32_t index = delta / kMovieSegmentRecordSize8001A324;
    if (index >= kSceneEntryMovieSegmentCount801C4780) {
        return false;
    }

    if (rowIndex != nullptr) {
        *rowIndex = index;
    }
    return true;
}

uint32_t ComputeMovieSegmentRowAddr801C4780(uint32_t sceneEntryBase,
                                            uint32_t rowIndex) {
    return sceneEntryBase + ComputeMovieSegmentRowOffset801C4780(rowIndex);
}

int32_t DecodeBcd80036A78(uint8_t value) {
    return 10 * static_cast<int32_t>(value >> 4) +
           static_cast<int32_t>(value & 0x0F);
}

MsfToLbaResult80036A78 PsxCall80036A78_MsfToLba(
    const MsfBcd80036A78& msf) {
    MsfToLbaResult80036A78 result{};
    result.known = true;
    result.lba =
        75 * (60 * DecodeBcd80036A78(msf.minute) +
              DecodeBcd80036A78(msf.second)) +
        DecodeBcd80036A78(msf.frame) - 150;
    return result;
}

CdReadyStatusResult800363A4 PsxCall800363A4_ReadCdReadyStatus(
    const CdReadyStatusFeedback800363A4& feedback) {
    CdReadyStatusResult800363A4 result{};
    result.called = true;
    if (!feedback.known) {
        result.gapMissingByte80057119 = true;
        return result;
    }

    result.resultKnown = true;
    result.psxReturn = feedback.byte80057119;
    return result;
}

CheckCallbackResult80035898 PsxCall80035898_CheckCallback(
    const CheckCallbackInput80035898& input) {
    CheckCallbackResult80035898 result{};
    result.called = true;
    if (!input.word80055F7AKnown) {
        result.gapMissingWord80055F7A = true;
        return result;
    }

    result.pendingKnown = true;
    result.pending = input.word80055F7A != 0u;
    result.word80055F7A = input.word80055F7A;
    return result;
}

CdCallbackPendingProducerResult800359B8
PsxCall800359B8_CdCallbackPendingProducer(
    const CdCallbackPendingProducerInput800359B8& input) {
    CdCallbackPendingProducerResult800359B8 result{};
    result.called = true;
    const bool fullInputRequested =
        input.word80055F78Known ||
        input.word80055FA8Known ||
        input.interruptStatusKnown ||
        input.interruptMaskKnown ||
        input.callbackTableKnown;
    if (fullInputRequested) {
        if (!input.word80055F78Known) {
            result.gapMissingWord80055F78 = true;
        }
        if (!input.word80055FA8Known) {
            result.gapMissingWord80055FA8 = true;
        }
        if (!input.interruptStatusKnown) {
            result.gapMissingInterruptStatus = true;
        }
        if (!input.interruptMaskKnown) {
            result.gapMissingInterruptMask = true;
        }
        if (!input.callbackTableKnown) {
            result.gapMissingCallbackTable = true;
        }
        if (result.gapMissingWord80055FA8 ||
            result.gapMissingInterruptStatus ||
            result.gapMissingInterruptMask) {
            return result;
        }

        result.fullProducerKnown = true;
        result.writeKnown = true;
        result.writeAddress = kSub800359B8SetWord80055F7A;
        result.setPendingWrite = true;
        result.word80055F7AKnown = true;
        result.word80055F7A = 1u;
        result.pendingKnown = true;
        result.pending = true;
        if (input.word80055F78Known && input.word80055F78 == 0u) {
            result.returnFromExceptionBeforePending = true;
        }

        uint16_t interruptStatus = input.interruptStatus;
        const uint16_t interruptMask = input.interruptMask;
        const uint16_t enabledMask =
            static_cast<uint16_t>(input.word80055FA8 & interruptMask);
        uint16_t pendingMask =
            static_cast<uint16_t>(interruptStatus & enabledMask);
        while (pendingMask != 0u &&
               result.interruptAckCount <
                   kCdCallbackPendingProducerCallbackCount800359B8) {
            uint8_t bitIndex = 0u;
            while (bitIndex <
                       kCdCallbackPendingProducerCallbackCount800359B8 &&
                   (pendingMask & (1u << bitIndex)) == 0u) {
                ++bitIndex;
            }
            if (bitIndex >=
                kCdCallbackPendingProducerCallbackCount800359B8) {
                break;
            }

            const uint16_t bitMask =
                static_cast<uint16_t>(1u << bitIndex);
            const uint32_t ackIndex = result.interruptAckCount++;
            result.interruptAckBitIndex[ackIndex] = bitIndex;
            result.interruptAckWriteValue[ackIndex] =
                static_cast<uint16_t>(~bitMask);
            result.interruptAckKnown = true;
            interruptStatus = static_cast<uint16_t>(interruptStatus & ~bitMask);

            if (input.callbackTableKnown && input.callbackPresent[bitIndex]) {
                const uint32_t callbackIndex = result.callbackDispatchCount++;
                result.callbackDispatchIndex[callbackIndex] = bitIndex;
            } else {
                result.gapMissingCallbackTable = true;
            }

            pendingMask = static_cast<uint16_t>(interruptStatus & enabledMask);
        }

        result.clearPendingWrite = true;
        result.writeAddress = kSub800359B8ClearWord80055F7A;
        result.word80055F7A = 0u;
        result.pending = false;
        result.returnFromExceptionAfterClear = true;
        if (input.watchdogKnown) {
            result.watchdogKnown = true;
            result.dword80057010 = input.dword80057010;
        }
        const bool hardwarePendingAfterCallbacks =
            (interruptStatus & input.interruptMask) != 0u;
        if (hardwarePendingAfterCallbacks) {
            const uint32_t nextWatchdog =
                input.watchdogKnown ? input.dword80057010 + 1u : 1u;
            result.watchdogWritten = true;
            result.dword80057010Written = nextWatchdog;
            result.watchdogTimeout = nextWatchdog >= 0x801u;
            result.interruptStatusClearedOnTimeout =
                result.watchdogTimeout;
            if (!input.watchdogKnown) {
                result.watchdogOverflowGap = true;
            }
        } else {
            result.watchdogWritten = true;
            result.dword80057010Written = 0u;
        }
        return result;
    }

    if (!input.writeAddressKnown) {
        result.gapMissingWriteAddress = true;
        return result;
    }

    result.writeKnown = true;
    result.writeAddress = input.writeAddress;
    switch (input.writeAddress) {
    case kSub800359B8SetWord80055F7A:
        result.setPendingWrite = true;
        result.word80055F7AKnown = true;
        result.word80055F7A = 1u;
        result.pendingKnown = true;
        result.pending = true;
        break;
    case kSub800359B8ClearWord80055F7A:
        result.clearPendingWrite = true;
        result.word80055F7AKnown = true;
        result.word80055F7A = 0u;
        result.pendingKnown = true;
        result.pending = false;
        break;
    default:
        result.gapUnknownWriteAddress = true;
        break;
    }
    return result;
}

namespace {

bool HasCdCallbackResponse80036AF8(
    const CdCallbackEventInput80036AF8& input) {
    return input.resultBytesKnown;
}

void CopyCdCallbackResponse80036AF8(std::array<uint8_t, 8>& dst,
                                    const CdCallbackEventInput80036AF8& input) {
    dst = {};
    if (!input.resultBytesKnown) {
        return;
    }
    const uint32_t count =
        input.resultByteCount < kCdCallbackResponseByteCount80036AF8
            ? input.resultByteCount
            : kCdCallbackResponseByteCount80036AF8;
    for (uint32_t i = 0u; i < count; ++i) {
        dst[i] = input.resultBytes[i];
    }
}

void SetCdCallbackSyncStatus80036AF8(
    CdCallbackEventResult80036AF8& result,
    uint8_t status,
    bool copyResponse,
    const CdCallbackEventInput80036AF8& input) {
    result.byte800573D4Known = true;
    result.byte800573D4 = status;
    if (copyResponse) {
        if (!HasCdCallbackResponse80036AF8(input)) {
            result.gapMissingResultBytes = true;
        } else {
            result.response882F8Known = true;
            CopyCdCallbackResponse80036AF8(result.response882F8, input);
        }
    }

    result.syncFeedback.known = true;
    result.syncFeedback.syncResultKnown = true;
    result.syncFeedback.syncResult = status;
    if (result.response882F8Known) {
        result.syncFeedback.responseBytesKnown = true;
        result.syncFeedback.responseByteCount =
            kCdCallbackResponseByteCount80036AF8;
        for (uint32_t i = 0; i < kCdCallbackResponseByteCount80036AF8; ++i) {
            result.syncFeedback.responseBytes[i] = result.response882F8[i];
        }
    }
}

void SetCdCallbackReadyStatus80036AF8(
    CdCallbackEventResult80036AF8& result,
    uint8_t status,
    bool copyResponse,
    const CdCallbackEventInput80036AF8& input) {
    result.byte800573D5Known = true;
    result.byte800573D5 = status;
    if (!copyResponse) {
        return;
    }
    if (!HasCdCallbackResponse80036AF8(input)) {
        result.gapMissingResultBytes = true;
        return;
    }
    result.response88300Known = true;
    CopyCdCallbackResponse80036AF8(result.response88300, input);
}

void SetCdCallbackAsyncStatus80036AF8(
    CdCallbackEventResult80036AF8& result,
    uint8_t status,
    bool copyResponse,
    const CdCallbackEventInput80036AF8& input) {
    result.byte800573D6Known = true;
    result.byte800573D6 = status;
    if (!copyResponse) {
        return;
    }
    if (!HasCdCallbackResponse80036AF8(input)) {
        result.gapMissingResultBytes = true;
        return;
    }
    result.response88308Known = true;
    CopyCdCallbackResponse80036AF8(result.response88308, input);
}

} // namespace

CdCallbackEventResult80036AF8 PsxCall80036AF8_BuildCdLowerEvent(
    const CdCallbackEventInput80036AF8& input) {
    CdCallbackEventResult80036AF8 result{};
    result.called = true;
    if (!input.interruptKnown) {
        result.gapMissingInterruptCode = true;
        return result;
    }

    const uint8_t interrupt = input.interruptCode & 7u;
    std::array<uint8_t, 8> paddedResult{};
    CopyCdCallbackResponse80036AF8(paddedResult, input);
    bool skipStatusGlobals = false;
    if (interrupt == 3u) {
        if (!input.commandKnown) {
            result.gapMissingCommand = true;
            return result;
        }
        uint32_t commandStatusUpdate = 0u;
        if (!TryReadCdEventTable80036AF8(
                kCdEventStatusUpdateTable800572BC,
                input.command,
                &commandStatusUpdate)) {
            result.gapMissingCommand = true;
            return result;
        }
        skipStatusGlobals = commandStatusUpdate == 0u;
    }

    if (input.resultBytesKnown) {
        result.resultBytesKnown = true;
        result.resultByteCount =
            input.resultByteCount < kCdCallbackResponseByteCount80036AF8
                ? input.resultByteCount
                : kCdCallbackResponseByteCount80036AF8;
        result.paddedResultBytes = paddedResult;
        if (!skipStatusGlobals) {
            if ((paddedResult[0] & 0x10u) != 0u) {
                if (!input.priorDword80057108Known) {
                    result.gapMissingPriorDword80057108 = true;
                } else if ((input.priorDword80057108 & 0x10u) == 0u) {
                    if (!input.priorDword80057110Known) {
                        result.gapMissingPriorDword80057110 = true;
                    } else {
                        result.dword80057110Known = true;
                        result.dword80057110 = input.priorDword80057110 + 1u;
                        result.dword80057110Incremented = true;
                    }
                }
            }
            result.dword80057108Known = true;
            result.dword80057108 = paddedResult[0];
            result.dword8005710CKnown = true;
            result.dword8005710C = paddedResult[1];
        }
    }
    const uint8_t syncMask = skipStatusGlobals
                                 ? 0u
                                 : (input.resultBytesKnown
                                        ? paddedResult[0]
                                        : input.priorSyncMask);
    const bool priorSyncActive =
        !skipStatusGlobals &&
        (input.resultBytesKnown || input.priorSyncMaskKnown) &&
        (syncMask & 0x1Du) != 0u;
    result.eventKnown = true;
    result.interruptCode = interrupt;

    switch (interrupt) {
    case 0:
        result.psxReturn = 0;
        return result;
    case 1: {
        uint8_t status = 1u;
        if (priorSyncActive && input.resultByteCount != 1u) {
            status = 5u;
        }
        SetCdCallbackReadyStatus80036AF8(result, status, true, input);
        result.cdReg0ClearAfterReady = true;
        result.cdReg3ClearAfterReady = true;
        result.psxReturn = 4;
        return result;
    }
    case 2: {
        const uint8_t status = priorSyncActive ? 5u : 2u;
        SetCdCallbackSyncStatus80036AF8(result, status, true, input);
        result.psxReturn = 2;
        return result;
    }
    case 3: {
        uint8_t status = 2u;
        int32_t psxReturn = 2;
        if (priorSyncActive) {
            status = 5u;
        } else {
            uint32_t commandStatus3 = 0u;
            if (!input.commandKnown ||
                !TryReadCdEventTable80036AF8(
                    kCdEventStatus3Table800571BC,
                    input.command,
                    &commandStatus3)) {
                result.gapMissingCommand = true;
                return result;
            }
            if (commandStatus3 != 0u) {
                status = 3u;
                psxReturn = 1;
            }
        }
        SetCdCallbackSyncStatus80036AF8(result, status, true, input);
        result.psxReturn = psxReturn;
        return result;
    }
    case 4:
        SetCdCallbackAsyncStatus80036AF8(result, 4u, true, input);
        SetCdCallbackReadyStatus80036AF8(result, 4u, true, input);
        result.psxReturn = 4;
        return result;
    case 5:
        SetCdCallbackReadyStatus80036AF8(result, 5u, true, input);
        SetCdCallbackSyncStatus80036AF8(result, 5u, true, input);
        result.psxReturn = 6;
        return result;
    default:
        result.eventKnown = false;
        return result;
    }
}

CdCallbackEventRegisterResult80036AF8
BuildCdCallbackEventInput80036AF8FromCdRegs(
    const CdCallbackEventRegisterInput80036AF8& input) {
    CdCallbackEventRegisterResult80036AF8 result{};
    result.called = true;
    result.wroteCdReg0Select1 = true;
    if (!input.cdReg3InitialKnown) {
        result.gapMissingCdReg3Initial = true;
        return result;
    }

    result.interruptKnown = true;
    result.initialInterruptCode = input.cdReg3Initial & 7u;
    if (result.initialInterruptCode == 0u) {
        result.builtEventInput = true;
        result.eventInput.interruptKnown = true;
        result.eventInput.interruptCode = 0u;
        return result;
    }

    if (!input.cdReg3StableKnown) {
        result.gapMissingCdReg3Stable = true;
        return result;
    }
    result.stableInterruptCode = input.cdReg3Stable & 7u;
    result.interruptChangedBeforeStable =
        result.stableInterruptCode != result.initialInterruptCode;

    if (!input.resultFifoKnown) {
        result.gapMissingResultFifo = true;
        return result;
    }

    result.resultFifoKnown = true;
    result.resultFifoByteCount =
        input.resultFifoByteCount < kCdCallbackResponseByteCount80036AF8
            ? input.resultFifoByteCount
            : kCdCallbackResponseByteCount80036AF8;
    for (uint32_t i = 0u; i < result.resultFifoByteCount; ++i) {
        result.paddedResultFifoBytes[i] = input.resultFifoBytes[i];
    }
    result.zeroPaddedResultFifo =
        result.resultFifoByteCount < kCdCallbackResponseByteCount80036AF8;

    result.ackCdReg0Select1 = true;
    result.ackCdReg3Write7 = true;
    result.ackCdReg2Write7 = true;
    result.builtEventInput = true;
    result.eventInput.interruptKnown = true;
    result.eventInput.interruptCode = result.stableInterruptCode;
    result.eventInput.resultBytesKnown = true;
    result.eventInput.resultByteCount = result.resultFifoByteCount;
    result.eventInput.resultBytes = result.paddedResultFifoBytes;
    result.eventInput.commandKnown = input.commandKnown;
    result.eventInput.command = input.command;
    result.eventInput.priorDword80057108Known =
        input.priorDword80057108Known;
    result.eventInput.priorDword80057108 = input.priorDword80057108;
    result.eventInput.priorDword80057110Known =
        input.priorDword80057110Known;
    result.eventInput.priorDword80057110 = input.priorDword80057110;
    result.eventInput.priorSyncMaskKnown = input.priorSyncMaskKnown;
    result.eventInput.priorSyncMask = input.priorSyncMask;
    return result;
}

CdSyncResult80037070 PsxCall80037070_CdSync(
    int32_t a0WaitMode,
    bool a1OutputBufferPtrNonNull,
    const CdSyncLowerFeedback80037070& feedback) {
    CdSyncResult80037070 result{};
    result.called = true;
    result.a0WaitMode = a0WaitMode;
    result.a1OutputBufferPtrNonNull = a1OutputBufferPtrNonNull;

    if (!feedback.known || !feedback.syncResultKnown) {
        result.gapMissingCdSyncFeedback = true;
        return result;
    }
    if (feedback.timedOut) {
        result.timeout = true;
        result.syncResultKnown = true;
        result.psxReturn = -1;
        return result;
    }

    result.syncResultKnown = true;
    result.psxReturn = feedback.syncResult;
    // PSX 80037070 args:
    // a0 = wait/poll mode, a1 = optional caller output buffer pointer.
    // Direct code keeps only a1 nullness; response bytes come from explicit
    // lower feedback, never from Win STR/XA/CD state.
    // PSX 80037070 copies the 8-byte response bank only for status 2/5 and
    // only when a1 points at an output buffer.
    if ((feedback.syncResult == 2 || feedback.syncResult == 5) &&
        a1OutputBufferPtrNonNull) {
        if (!feedback.responseBytesKnown || feedback.responseByteCount == 0u) {
            result.gapMissingCdSyncFeedback = true;
            return result;
        }
        const uint32_t byteCount =
            feedback.responseByteCount > result.outputBytes.size()
                ? static_cast<uint32_t>(result.outputBytes.size())
                : feedback.responseByteCount;
        result.outputBytesKnown = true;
        result.outputByteCount = byteCount;
        result.outputBytes = feedback.responseBytes;
        result.copiedOutputBytes = true;
    }
    return result;
}

CdSyncResult80037070 PsxCall800364D0_CdSyncWrapper(
    int32_t a0WaitMode,
    bool a1OutputBufferPtrNonNull,
    const CdSyncLowerFeedback80037070& feedback) {
    CdSyncResult80037070 result =
        PsxCall80037070_CdSync(
            a0WaitMode,
            a1OutputBufferPtrNonNull,
            feedback);
    result.sourceFunction = kSub800364D0CdSyncWrapper;
    return result;
}

StreamClockPollInput8001A3C8
BuildStreamClockPollInput8001A3C8FromCdSync(
    const CdSyncResult80037070& sync,
    const CdReadyStatusResult800363A4& ready) {
    StreamClockPollInput8001A3C8 input{};
    input.sub800364D0Known = sync.syncResultKnown;
    input.sub800364D0Result = sync.psxReturn;
    input.syncBytesKnown = sync.outputBytesKnown && sync.outputByteCount >= 3u;
    input.syncBytes = sync.outputBytes;
    input.sub800363A4Known = ready.resultKnown;
    input.sub800363A4Result = ready.psxReturn;
    return input;
}

StreamStatusPollInput8001A750
BuildStreamStatusPollInput8001A750FromCdSync(
    const CdSyncResult80037070& sync) {
    StreamStatusPollInput8001A750 input{};
    input.sub800364D0Known = sync.syncResultKnown;
    input.sub800364D0Result = sync.psxReturn;
    input.statusBytesKnown = sync.outputBytesKnown && sync.outputByteCount >= 1u;
    input.statusBytes = sync.outputBytes;
    return input;
}

StreamClockCallbackResult8001A210 PsxCall8001A210_StreamClockCallback(
    const StreamClockCallbackInput8001A210& input) {
    StreamClockCallbackResult8001A210 result{};
    result.called = true;
    result.psxReturn = 2;

    if (input.a1 != 2u) {
        return result;
    }
    if (input.resultBytes == nullptr || input.resultSize < 5u) {
        result.gapMissingCallbackResultBytes = true;
        return result;
    }

    result.psxReturn = input.resultBytes[4];
    result.acceptedByte800493F4 = true;
    result.feedback.known = true;
    result.feedback.source = StreamClockSource800493F4::Byte800493F4;
    result.feedback.producerAddr = result.sourceFunction;
    result.feedback.byte800493F4Known = true;
    result.feedback.byte800493F4.minute = input.resultBytes[2];
    result.feedback.byte800493F4.second = input.resultBytes[3];
    result.feedback.byte800493F4.frame = input.resultBytes[4];
    return result;
}

StreamClockPollResult8001A3C8 PsxCall8001A3C8_StreamClockPoll(
    const StreamClockPollInput8001A3C8& input) {
    StreamClockPollResult8001A3C8 result{};
    result.called = true;

    if (!input.sub800364D0Known || !input.syncBytesKnown) {
        result.gapMissingSub800364D0Feedback = true;
        return result;
    }

    if (input.sub800364D0Result == 5 &&
        (input.syncBytes[0] & 0x10u) != 0u) {
        result.dword80049428Known = true;
        result.dword80049428 = 1;
        result.psxReturn = 0;
        return result;
    }

    if (input.sub800364D0Result != 2) {
        result.psxReturn = 0;
        return result;
    }

    if (!input.sub800363A4Known) {
        result.gapMissingSub800363A4Feedback = true;
        result.psxReturn = 1;
        return result;
    }
    if (input.sub800363A4Result == 13) {
        result.psxReturn = 0;
        return result;
    }

    result.dword80049428Known = true;
    result.dword80049428 = 0;
    result.psxReturn = 1;
    result.acceptedByte800493F4 = true;
    result.feedback.known = true;
    result.feedback.source = StreamClockSource800493F4::Byte800493F4;
    result.feedback.producerAddr = result.sourceFunction;
    result.feedback.byte800493F4Known = true;
    result.feedback.byte800493F4.minute = input.syncBytes[0];
    result.feedback.byte800493F4.second = input.syncBytes[1];
    result.feedback.byte800493F4.frame = input.syncBytes[2];
    return result;
}

StreamStatusPollResult8001A750 PsxCall8001A750_StreamStatusPoll(
    const StreamStatusPollInput8001A750& input) {
    StreamStatusPollResult8001A750 result{};
    result.called = true;

    if (!input.sub800364D0Known) {
        result.gapMissingSub800364D0Feedback = true;
        return result;
    }

    if (input.sub800364D0Result != 2) {
        result.resultKnown = true;
        result.psxReturn = 0;
        return result;
    }

    if (!input.statusBytesKnown) {
        result.gapMissingSub800364D0Feedback = true;
        return result;
    }

    if ((input.statusBytes[0] & 0x20u) != 0u) {
        result.resultKnown = true;
        result.psxReturn = 1;
        return result;
    }

    result.requestedCommandWrapper80036678 = true;
    result.commandWrapper80036678Known =
        input.commandWrapper80036678Known;
    result.commandWrapper80036678Succeeded =
        input.commandWrapper80036678Succeeded;
    result.gapMissingCommandWrapper80036678 =
        !input.commandWrapper80036678Known;
    result.resultKnown = true;
    result.psxReturn = 0;
    return result;
}

StreamClockResetResult8001A724 PsxCall8001A724_ResetStreamClock(
    int32_t a1) {
    StreamClockResetResult8001A724 result{};
    result.called = true;
    result.psxReturn = 0;
    result.dword80049404Known = true;
    result.dword80049404 = 150;
    result.dword80049408Known = true;
    result.dword80049408 = -a1;
    return result;
}

StreamClockProducerCarrier800493F4
BuildStreamClockProducerCarrier800493F4(
    const StreamClockProducerFeedback800493F4& feedback) {
    StreamClockProducerCarrier800493F4 result{};
    result.called = true;
    result.feedback = feedback;

    if (!feedback.known) {
        result.gapMissingByte800493F4ClockProducer = true;
        return result;
    }

    if (feedback.byte800493F4Known) {
        result.acceptedSource = StreamClockSource800493F4::Byte800493F4;
        result.acceptedByte800493F4 = true;
        result.sub80036A78 =
            PsxCall80036A78_MsfToLba(feedback.byte800493F4);
        result.sub80036A78Computed = result.sub80036A78.known;
        result.clockKnown = result.sub80036A78.known;
        result.clockLba = result.sub80036A78.lba;
    }

    if (result.clockKnown) {
        result.watchdogKnown = true;
        result.dword80049404 = result.clockLba + 150;
    } else {
        result.gapMissingByte800493F4ClockProducer = true;
    }
    return result;
}

SegmentInitResult8001A324 PsxCall8001A324_InitSegmentRecord(
    const MovieSegmentRecord48& source) {
    SegmentInitResult8001A324 result{};
    result.record = source;

    if (source.psxAddr != 0u) {
        result.lookupRequestKnown = true;
        result.lookupRequestCdlFilePtr = source.psxAddr + 0x10u;
    }
    result.lookupRequestPathPtrKnown = source.pathPtrA1Plus00Known;
    result.lookupRequestPathPtr = source.pathPtrA1Plus00;

    if (source.loadedStateA1Plus0CKnown &&
        source.loadedStateA1Plus0C == 1) {
        result.result = 0;
        result.skippedAlreadyLoaded = true;
        return result;
    }

    if (source.pathPtrA1Plus00Known && source.pathPtrA1Plus00 == 0u) {
        result.result = 0;
        result.skippedPathPtrZero = true;
        return result;
    }

    // 8001A2B0 side effects are supplied by row feedback in this carrier.
    if (!source.startMsfKnown || !source.lengthSourceA1Plus20Known) {
        result.result = -1;
        return result;
    }

    const MsfToLbaResult80036A78 lba =
        PsxCall80036A78_MsfToLba(source.startMsf);
    result.record.timeBaseA1Plus40Known = lba.known;
    result.record.timeBaseA1Plus40 = lba.lba;
    result.timeBaseWrittenA1Plus40 = lba.known;

    result.record.loadedStateA1Plus0CKnown = true;
    result.record.loadedStateA1Plus0C = 1;
    result.loadedStateWrittenA1Plus0C = true;

    if (result.record.timeBaseA1Plus40Known) {
        result.record.endA1Plus44Known = true;
        result.record.endA1Plus44 =
            result.record.timeBaseA1Plus40 +
            static_cast<int32_t>(source.lengthSourceA1Plus20 >> 11);
        result.endWrittenA1Plus44 = true;
        result.cdLookupSucceeded = true;
        result.result = 0;
    }

    return result;
}

static uint32_t PackCdlFileNameWord8001ACF8(
    const std::array<uint8_t, 16>& name,
    uint32_t wordIndex) {
    const uint32_t base = wordIndex * 4u;
    return static_cast<uint32_t>(name[base + 0u]) |
           (static_cast<uint32_t>(name[base + 1u]) << 8) |
           (static_cast<uint32_t>(name[base + 2u]) << 16) |
           (static_cast<uint32_t>(name[base + 3u]) << 24);
}

static void CopyCdlFileWords8001ACF8(
    const MovieSegmentRecord48& row,
    OverlayTransferAttempt8001ACF8& attempt) {
    if (!row.startMsfKnown || !row.lengthSourceA1Plus20Known) {
        return;
    }

    attempt.copiedCdlFileWords = true;
    attempt.cdlFileWords[0] =
        static_cast<uint32_t>(row.startMsf.minute) |
        (static_cast<uint32_t>(row.startMsf.second) << 8) |
        (static_cast<uint32_t>(row.startMsf.frame) << 16);
    attempt.cdlFileWords[1] = row.lengthSourceA1Plus20;
    if (row.cdlFileNameA1Plus18Known) {
        attempt.cdlFileWords[2] =
            PackCdlFileNameWord8001ACF8(row.cdlFileNameA1Plus18, 0);
        attempt.cdlFileWords[3] =
            PackCdlFileNameWord8001ACF8(row.cdlFileNameA1Plus18, 1);
        attempt.cdlFileWords[4] =
            PackCdlFileNameWord8001ACF8(row.cdlFileNameA1Plus18, 2);
        attempt.cdlFileWords[5] =
            PackCdlFileNameWord8001ACF8(row.cdlFileNameA1Plus18, 3);
    }
}

OverlayTransferSeekFeedback8001A89C BuildOverlayTransferSeekFeedback8001A89C(
    const OverlayTransferSeekLowerFeedback8001A89C& lower) {
    OverlayTransferSeekFeedback8001A89C out{};
    out.known = lower.known;
    out.syncResultKnown = lower.syncResultKnown;
    out.syncResult = lower.syncResult;
    out.psxReturnKnown = lower.psxReturnKnown;
    out.psxReturn = lower.psxReturn;
    return out;
}

OverlayTransferAttemptProvenance8001ACF8
BuildOverlayTransferAttemptProvenance8001ACF8(
    bool rowAddrKnown,
    uint32_t rowAddr,
    bool attemptIndexKnown,
    uint32_t attemptIndex,
    bool destKnown,
    uint32_t dest,
    bool sectorCountKnown,
    uint32_t sectorCount) {
    OverlayTransferAttemptProvenance8001ACF8 out{};
    out.known = true;
    out.rowAddrKnown = rowAddrKnown;
    out.rowAddr = rowAddr;
    out.attemptIndexKnown = attemptIndexKnown;
    out.attemptIndex = attemptIndex;
    out.destKnown = destKnown;
    out.dest = dest;
    out.sectorCountKnown = sectorCountKnown;
    out.sectorCount = sectorCount;
    return out;
}

void AttachOverlayTransferAttemptProvenance8001ACF8(
    const OverlayTransferAttemptProvenance8001ACF8& provenance,
    OverlayTransferSeekFeedback8001A89C& feedback) {
    feedback.provenance = provenance;
}

void AttachOverlayTransferAttemptProvenance8001ACF8(
    const OverlayTransferAttemptProvenance8001ACF8& provenance,
    OverlayTransferReadFeedback8001A818& feedback) {
    feedback.provenance = provenance;
}

static bool OverlayTransferAttemptProvenanceMatches8001ACF8(
    const OverlayTransferAttemptProvenance8001ACF8& provenance,
    const MovieSegmentRecord48& row,
    uint32_t attemptIndex,
    bool destKnown,
    uint32_t dest,
    bool sectorCountKnown,
    uint32_t sectorCount) {
    if (!provenance.known) {
        return true;
    }
    if (provenance.wrapperFunction != kSub800154B0OverlayTransferWrapper ||
        provenance.transferFunction != kSub8001ACF8OverlayTransfer) {
        return false;
    }
    if (provenance.rowAddrKnown &&
        (!row.psxAddr || provenance.rowAddr != row.psxAddr)) {
        return false;
    }
    if (provenance.attemptIndexKnown &&
        provenance.attemptIndex != attemptIndex) {
        return false;
    }
    if (provenance.destKnown &&
        (!destKnown || provenance.dest != dest)) {
        return false;
    }
    if (provenance.sectorCountKnown &&
        (!sectorCountKnown || provenance.sectorCount != sectorCount)) {
        return false;
    }
    return true;
}

OverlayTransferSeekResult8001A89C PsxCall8001A89C_SeekForOverlayTransfer(
    const MovieSegmentRecord48& row,
    int32_t offset,
    bool offsetKnown,
    const OverlayTransferSeekFeedback8001A89C* feedback) {
    OverlayTransferSeekResult8001A89C result{};
    result.called = true;
    result.offsetKnown = offsetKnown;
    result.offset = offset;
    result.cdlFilePosKnown = row.startMsfKnown;
    result.cdlFilePos = row.startMsf;
    result.syncArg1CdlPtrKnown = row.psxAddr != 0u;
    result.syncArg1CdlPtr = row.psxAddr + 0x10u;

    if (!row.startMsfKnown) {
        result.gapMissingCdlFilePos = true;
        return result;
    }

    const MsfToLbaResult80036A78 lba =
        PsxCall80036A78_MsfToLba(row.startMsf);
    result.baseLbaKnown = lba.known;
    result.baseLba = lba.lba;
    result.targetLbaKnown = lba.known && offsetKnown;
    result.targetLba = lba.lba + offset;
    if (result.targetLbaKnown) {
        const int32_t biased = result.targetLba + 150;
        const int32_t totalSeconds = biased / 75;
        const auto toBcd = [](int32_t value) -> uint8_t {
            return static_cast<uint8_t>(((value / 10) << 4) |
                                        (value % 10));
        };
        result.targetMsfKnown = true;
        result.targetMsf.minute = toBcd(totalSeconds / 60);
        result.targetMsf.second = toBcd(totalSeconds % 60);
        result.targetMsf.frame = toBcd(biased % 75);
    }

    if (feedback == nullptr || !feedback->known) {
        result.gapMissingSeekFeedback = true;
        return result;
    }

    if (feedback->syncResultKnown) {
        result.syncResultKnown = true;
        result.syncResult = feedback->syncResult;
    }
    if (feedback->psxReturnKnown) {
        result.psxReturnKnown = true;
        result.psxReturn = feedback->psxReturn;
    } else if (feedback->syncResultKnown) {
        result.psxReturnKnown = true;
        result.psxReturn = feedback->syncResult != 0;
    } else {
        result.gapMissingSeekFeedback = true;
    }
    return result;
}

OverlayTransferReadFeedback8001A818 BuildOverlayTransferReadFeedback8001A818(
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown,
    int32_t firstAttemptFlag,
    bool firstAttemptFlagKnown,
    const OverlayTransferReadLowerFeedback8001A818& lower) {
    OverlayTransferReadFeedback8001A818 result{};
    result.known = lower.known;
    result.dstKnown = dstKnown;
    result.dst = dst;
    result.sectorCountKnown = sectorCountKnown;
    result.requestedSectorCount = sectorCount;
    result.firstAttemptFlagKnown = firstAttemptFlagKnown;
    result.firstAttemptFlag = firstAttemptFlag;
    if (firstAttemptFlagKnown) {
        result.modeFlagKnown = true;
        result.modeFlag =
            static_cast<uint8_t>((firstAttemptFlag == 1 ? 1 : 0) << 7);
    }
    result.readStartFunction = kSub80038FC0ReadStart;
    result.waitFunction = kSub80035560VBlankWait;
    result.waitArg = kOverlayTransferReadWaitArg8001A818;
    result.syncFunction = kSub800390C8ReadSync;
    result.syncArg0 = kOverlayTransferReadSyncArg0_8001A818;
    result.syncArg1 = kOverlayTransferReadSyncArg1_8001A818;

    if (!lower.known) {
        return result;
    }
    if (lower.readStartSucceededKnown) {
        result.readStartSucceeded = lower.readStartSucceeded;
    }
    if (lower.readStartRetryCountKnown) {
        result.readStartRetryCount = lower.readStartRetryCount;
    }
    if (lower.syncPollCountKnown) {
        result.syncPollCount = lower.syncPollCount;
    }
    if (lower.syncResultKnown) {
        result.syncResultKnown = true;
        result.syncResult = lower.syncResult;
        if (sectorCountKnown) {
            result.psxReturnKnown = true;
            result.psxReturn =
                lower.syncResult == kOverlayTransferReadSyncSuccess8001A818
                    ? static_cast<int32_t>(sectorCount)
                    : 0;
            result.readResultKnown = true;
            result.readSectorCount = result.psxReturn;
        }
    }
    return result;
}

OverlayTransferResult8001ACF8 PsxCall8001ACF8_OverlayTransfer(
    const MovieSegmentRecord48& row,
    const MovieSegmentRowInitFeedback8001A324& rowFeedback,
    const OverlayTransferSeekFeedback8001A89C* seekFeedback,
    uint32_t seekFeedbackCount,
    const OverlayTransferReadFeedback8001A818* readFeedback,
    uint32_t readFeedbackCount,
    uint32_t dest,
    bool destKnown) {
    OverlayTransferResult8001ACF8 result{};
    result.called = true;
    result.destKnown = destKnown;
    result.dest = dest;

    MovieSegmentRecord48 current = row;
    bool allKnownReadFeedbackFailed = true;
    bool sawKnownReadFeedback = false;

    for (uint32_t attemptIndex = 0;
         attemptIndex < kOverlayTransferMaxAttempts8001ACF8;
         ++attemptIndex) {
        OverlayTransferAttempt8001ACF8& attempt =
            result.attempts[attemptIndex];
        attempt.executed = true;
        attempt.attemptIndex = attemptIndex;
        attempt.firstAttemptFlag = attemptIndex == 0u ? 1 : 0;
        ++result.attemptCount;

        current = ApplyMovieSegmentRowFeedback8001A324(current, rowFeedback);
        attempt.rowInit = PsxCall8001A324_InitSegmentRecord(current);
        current = attempt.rowInit.record;
        result.finalRow = current;

        if (attempt.rowInit.result < 0) {
            attempt.gapMissingCdLookupFeedback = true;
            result.gapMissingCdLookupFeedback = true;
            allKnownReadFeedbackFailed = false;
            continue;
        }

        CopyCdlFileWords8001ACF8(current, attempt);
        if (current.lengthSourceA1Plus20Known) {
            attempt.sectorCountKnown = true;
            attempt.sectorCount =
                (current.lengthSourceA1Plus20 + 2047u) >> 11;
        }

        attempt.seekAction = current.startMsfKnown &&
                             current.lengthSourceA1Plus20Known;
        if (attempt.seekAction) {
            const OverlayTransferSeekFeedback8001A89C* seekCandidate =
                seekFeedback != nullptr && attemptIndex < seekFeedbackCount
                    ? &seekFeedback[attemptIndex]
                    : nullptr;
            const OverlayTransferSeekFeedback8001A89C* seekOne =
                seekCandidate != nullptr &&
                        OverlayTransferAttemptProvenanceMatches8001ACF8(
                            seekCandidate->provenance,
                            current,
                            attemptIndex,
                            destKnown,
                            dest,
                            attempt.sectorCountKnown,
                            attempt.sectorCount)
                    ? seekCandidate
                    : nullptr;
            attempt.seek8001A89C =
                PsxCall8001A89C_SeekForOverlayTransfer(
                    current,
                    0,
                    true,
                    seekOne);
            attempt.gapMissingSeekFeedback =
                attempt.seek8001A89C.gapMissingSeekFeedback;
            result.gapMissingSeekFeedback =
                result.gapMissingSeekFeedback ||
                attempt.gapMissingSeekFeedback;
        }
        attempt.destKnown = destKnown;
        attempt.dest = dest;
        attempt.readAction = attempt.sectorCountKnown && destKnown;
        if (!attempt.readAction) {
            attempt.gapMissingReadFeedback = true;
            result.gapMissingReadFeedback = true;
            allKnownReadFeedbackFailed = false;
            continue;
        }

        if (readFeedback != nullptr && attemptIndex < readFeedbackCount &&
            readFeedback[attemptIndex].known &&
            OverlayTransferAttemptProvenanceMatches8001ACF8(
                readFeedback[attemptIndex].provenance,
                current,
                attemptIndex,
                destKnown,
                dest,
                attempt.sectorCountKnown,
                attempt.sectorCount)) {
            attempt.readFeedbackKnown = true;
            const OverlayTransferReadFeedback8001A818& feedback =
                readFeedback[attemptIndex];
            const bool resultKnown =
                feedback.psxReturnKnown || feedback.readResultKnown;
            if (resultKnown) {
                attempt.readSectorCount =
                    feedback.psxReturnKnown
                        ? feedback.psxReturn
                        : feedback.readSectorCount;
                sawKnownReadFeedback = true;
                attempt.matchedExpectedSectorCount =
                    attempt.readSectorCount ==
                    static_cast<int32_t>(attempt.sectorCount);
                if (attempt.matchedExpectedSectorCount) {
                    result.resultKnown = true;
                    result.psxReturn = true;
                    break;
                }
                continue;
            }
        }

        attempt.gapMissingReadFeedback = true;
        result.gapMissingReadFeedback = true;
        allKnownReadFeedbackFailed = false;
    }

    if (!result.resultKnown && sawKnownReadFeedback &&
        allKnownReadFeedbackFailed) {
        result.resultKnown = true;
        result.psxReturn = false;
    }
    result.finalRow = current;
    return result;
}

OverlayTransferResult800154B0 PsxCall800154B0_Row0OverlayTransfer(
    const MovieSegmentRecord48& row,
    const MovieSegmentRowInitFeedback8001A324& rowFeedback,
    const OverlayTransferSeekFeedback8001A89C* seekFeedback,
    uint32_t seekFeedbackCount,
    const OverlayTransferReadFeedback8001A818* readFeedback,
    uint32_t readFeedbackCount) {
    OverlayTransferResult800154B0 result{};
    result.called = true;
    result.rowArgKnown = row.psxAddr != 0u;
    result.rowArg = row.psxAddr;
    result.ignoredArg1Known = true;
    result.ignoredArg1 = 0;
    result.destKnown = true;
    result.dest = kOverlayTransferDest801C3870;
    result.sub8001ACF8 = PsxCall8001ACF8_OverlayTransfer(
        row,
        rowFeedback,
        seekFeedback,
        seekFeedbackCount,
        readFeedback,
        readFeedbackCount,
        kOverlayTransferDest801C3870,
        true);
    return result;
}

MovieSegmentRecord48 BuildMovieSegmentRecordFromRawRow801C4780(
    const SceneEntryMovieSegmentRawRow801C4780& raw,
    uint32_t rowIndex) {
    MovieSegmentRecord48 out{};
    out.known = raw.known;
    out.tableIndex = rowIndex;
    out.pathPtrA1Plus00Known = raw.pathPtrA1Plus00Known;
    out.pathPtrA1Plus00 = raw.pathPtrA1Plus00;
    out.opaqueA1Plus04Known = raw.opaqueA1Plus04Known;
    out.opaqueA1Plus04 = raw.opaqueA1Plus04;
    out.endBiasA1Plus8Known = raw.endBiasA1Plus08Known;
    out.endBiasA1Plus8 = raw.endBiasA1Plus08;
    out.loadedStateA1Plus0CKnown = raw.loadedStateA1Plus0CKnown;
    out.loadedStateA1Plus0C = raw.loadedStateA1Plus0C;
    out.startMsfKnown = raw.cdlFilePosKnown;
    out.startMsf = raw.cdlFilePos;
    out.lengthSourceA1Plus20Known = raw.cdlFileSizeKnown;
    out.lengthSourceA1Plus20 = raw.cdlFileSize;
    out.cdlFileNameA1Plus18Known = raw.cdlFileNameKnown;
    out.cdlFileNameA1Plus18 = raw.cdlFileName;
    return out;
}

SceneEntryMovieSegmentTable801C4780 MaterializeSceneEntryMovieSegments801C4780(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    const MovieSegmentRecord48* rows,
    uint32_t rowCount) {
    SceneEntryMovieSegmentTable801C4780 result{};
    result.sceneEntryBaseKnown = sceneEntryBaseKnown;
    result.sceneEntryBase = sceneEntryBase;

    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        MovieSegmentRecord48& row = result.rows[index];
        if (rows != nullptr && index < rowCount) {
            row = rows[index];
        }
        row.tableIndex = index;
        if (sceneEntryBaseKnown) {
            row.psxAddr =
                ComputeMovieSegmentRowAddr801C4780(sceneEntryBase, index);
        }
    }

    return result;
}

SceneEntryMovieSegmentTable801C4780
MaterializeSceneEntryMovieSegmentsFromRawRows801C4780(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    const SceneEntryMovieSegmentRawRow801C4780* rows,
    uint32_t rowCount) {
    MovieSegmentRecord48 records[kSceneEntryMovieSegmentCount801C4780]{};
    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        if (rows != nullptr && index < rowCount) {
            records[index] =
                BuildMovieSegmentRecordFromRawRow801C4780(rows[index], index);
        }
    }
    return MaterializeSceneEntryMovieSegments801C4780(
        sceneEntryBase,
        sceneEntryBaseKnown,
        records,
        kSceneEntryMovieSegmentCount801C4780);
}

MovieSegmentRecord48 ApplyMovieSegmentRowFeedback8001A324(
    const MovieSegmentRecord48& row,
    const MovieSegmentRowInitFeedback8001A324& feedback) {
    MovieSegmentRecord48 result = row;
    if (!feedback.known) {
        return result;
    }

    result.known = true;
    if (feedback.loadedStateA1Plus0CKnown) {
        result.loadedStateA1Plus0CKnown = true;
        result.loadedStateA1Plus0C = feedback.loadedStateA1Plus0C;
    }
    if (feedback.cdlFilePosKnown) {
        result.startMsfKnown = true;
        result.startMsf = feedback.cdlFilePos;
    }
    if (feedback.cdlFileSizeKnown) {
        result.lengthSourceA1Plus20Known = true;
        result.lengthSourceA1Plus20 = feedback.cdlFileSize;
    }
    if (feedback.cdlFileNameKnown) {
        result.cdlFileNameA1Plus18Known = true;
        result.cdlFileNameA1Plus18 = feedback.cdlFileName;
    }
    return result;
}

MovieSegmentRowInitFeedback8001A324
BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(
    const CdLookupProbeCompletion8001A2B0& feedback,
    bool loadedStateA1Plus0CKnown,
    int32_t loadedStateA1Plus0C) {
    MovieSegmentRowInitFeedback8001A324 out{};
    if (!feedback.known || !feedback.complete) {
        return out;
    }

    out.known = true;
    out.explicitCdLookupFeedback = feedback.explicitCdLookupFeedback;
    out.lookupRequestKnown = feedback.requestKnown;
    out.lookupRequestCdlFilePtr = feedback.requestCdlFilePtr;
    out.lookupRequestPathPtrKnown = feedback.requestPathPtrKnown;
    out.lookupRequestPathPtr = feedback.requestPathPtr;
    out.lookupResultPtrKnown = true;
    out.lookupResultPtr = feedback.resultPtr;
    out.loadedStateA1Plus0CKnown = loadedStateA1Plus0CKnown;
    out.loadedStateA1Plus0C = loadedStateA1Plus0C;
    if (feedback.lookupSucceeded && feedback.explicitCdLookupFeedback) {
        out.cdlFilePosKnown = feedback.cdlFilePosKnown;
        out.cdlFilePos = feedback.cdlFilePos;
        out.cdlFileSizeKnown = feedback.cdlFileSizeKnown;
        out.cdlFileSize = feedback.cdlFileSize;
        out.cdlFileNameKnown = feedback.cdlFileNameKnown;
        out.cdlFileName = feedback.cdlFileName;
    }
    return out;
}

MovieSegmentScanResult801C4780 PsxCall801C4780_ScanMovieSegments(
    const SceneEntryMovieSegmentTable801C4780& table) {
    return PsxCall801C4780_ScanMovieSegmentsWithFeedback(table, nullptr, 0);
}

MovieSegmentScanResult801C4780 PsxCall801C4780_ScanMovieSegmentsWithFeedback(
    const SceneEntryMovieSegmentTable801C4780& table,
    const MovieSegmentRowInitFeedback8001A324* feedback,
    uint32_t feedbackCount) {
    MovieSegmentScanResult801C4780 result{};
    result.called = true;
    result.table = table;
    result.feedbackCount = feedbackCount;

    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        if (feedback != nullptr && index < feedbackCount &&
            feedback[index].known) {
            result.feedbackAppliedMask[index] = true;
            ++result.feedbackAppliedCount;
            result.table.rows[index] =
                ApplyMovieSegmentRowFeedback8001A324(result.table.rows[index],
                                                     feedback[index]);
        }
        const MovieSegmentRecord48& rowBeforeInit = result.table.rows[index];
        const bool explicitFeedback =
            feedback != nullptr && index < feedbackCount &&
            feedback[index].known &&
            feedback[index].explicitCdLookupFeedback;
        const bool disabledPath =
            rowBeforeInit.pathPtrA1Plus00Known &&
            rowBeforeInit.pathPtrA1Plus00 == 0u;
        const bool alreadyLoaded =
            rowBeforeInit.loadedStateA1Plus0CKnown &&
            rowBeforeInit.loadedStateA1Plus0C == 1;
        const bool lookupReady =
            explicitFeedback && feedback[index].cdlFilePosKnown &&
            feedback[index].cdlFileSizeKnown;
        const bool cdlFileNameReady =
            explicitFeedback && feedback[index].cdlFileNameKnown;
        const bool needsLookup =
            rowBeforeInit.known && !disabledPath && !alreadyLoaded;
        const bool requestKnown =
            needsLookup && rowBeforeInit.psxAddr != 0u;
        const bool requestPathPtrKnown =
            requestKnown && rowBeforeInit.pathPtrA1Plus00Known;
        result.rowNeedsCdLookupMask[index] = needsLookup;
        if (needsLookup) {
            ++result.rowNeedsCdLookupCount;
        }
        result.rowCdLookupRequestKnownMask[index] = requestKnown;
        if (requestKnown) {
            result.rowCdLookupRequestCdlFilePtr[index] =
                rowBeforeInit.psxAddr + 0x10u;
            ++result.rowCdLookupRequestKnownCount;
        }
        result.rowCdLookupRequestPathPtrKnownMask[index] =
            requestPathPtrKnown;
        if (requestPathPtrKnown) {
            result.rowCdLookupRequestPathPtr[index] =
                rowBeforeInit.pathPtrA1Plus00;
            ++result.rowCdLookupRequestPathPtrKnownCount;
        }
        result.rowMissingCdLookupRequestMask[index] =
            needsLookup && !requestPathPtrKnown;
        if (result.rowMissingCdLookupRequestMask[index]) {
            ++result.rowMissingCdLookupRequestCount;
        }
        if (feedback != nullptr && index < feedbackCount &&
            feedback[index].known && feedback[index].lookupResultPtrKnown) {
            result.rowCdLookupResultPtrKnownMask[index] = true;
            result.rowCdLookupResultPtr[index] = feedback[index].lookupResultPtr;
            ++result.rowCdLookupResultPtrKnownCount;
        }
        result.rowCdLookupReadyMask[index] = lookupReady;
        if (lookupReady) {
            ++result.rowCdLookupReadyCount;
        }
        result.rowCdlFileNameReadyMask[index] = cdlFileNameReady;
        if (cdlFileNameReady) {
            ++result.rowCdlFileNameReadyCount;
        }
        result.rowMissingCdLookupFeedbackMask[index] =
            needsLookup && !lookupReady;
        if (result.rowMissingCdLookupFeedbackMask[index]) {
            ++result.rowMissingCdLookupFeedbackCount;
        }
        result.rowMissingCdlFileNameFeedbackMask[index] =
            needsLookup && !cdlFileNameReady;
        if (result.rowMissingCdlFileNameFeedbackMask[index]) {
            ++result.rowMissingCdlFileNameFeedbackCount;
        }
        MovieSegmentRecord48 rowForInit = result.table.rows[index];
        if (needsLookup && !lookupReady) {
            rowForInit.startMsfKnown = false;
            rowForInit.lengthSourceA1Plus20Known = false;
            rowForInit.cdlFileNameA1Plus18Known = false;
        }
        result.rowInit[index] =
            PsxCall8001A324_InitSegmentRecord(rowForInit);
        result.table.rows[index] = result.rowInit[index].record;
    }
    result.gapMissingCdLookupFeedback =
        result.rowMissingCdLookupFeedbackCount != 0u;
    result.gapMissingCdlFileNameFeedback =
        result.rowMissingCdlFileNameFeedbackCount != 0u;
    result.gapMissingCdLookupRequest =
        result.rowMissingCdLookupRequestCount != 0u;

    result.reset80025A00Action = true;
    result.loadCompo8001AC18Action = true;
    if (table.sceneEntryBaseKnown) {
        result.loadCompoArg0 =
            table.sceneEntryBase +
            ComputeMovieSegmentRowOffset801C4780(1u);
    }
    result.loadCompoArg1 = 0;
    return result;
}

SceneEntrySegmentSelection801C4DC4 PsxSelectMovieSegmentByOffset801C4DC4(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    uint32_t segmentOffset) {
    SceneEntrySegmentSelection801C4DC4 result{};
    result.known = sceneEntryBaseKnown;
    result.sceneEntryBase = sceneEntryBase;
    result.segmentOffset = segmentOffset;
    result.segmentAddr = sceneEntryBase + segmentOffset;
    result.known =
        result.known &&
        TryComputeMovieSegmentRowIndex801C4780(segmentOffset,
                                               &result.rowIndex);
    return result;
}

SceneEntrySegmentSelection801C4DC4 PsxSelectMovieSegmentByOffsetFromScan801C4DC4(
    const MovieSegmentScanResult801C4780& scan,
    uint32_t segmentOffset) {
    SceneEntrySegmentSelection801C4DC4 result =
        PsxSelectMovieSegmentByOffset801C4DC4(scan.table.sceneEntryBase,
                                              scan.table.sceneEntryBaseKnown,
                                              segmentOffset);
    if (!scan.called || result.rowIndex >= kSceneEntryMovieSegmentCount801C4780) {
        return result;
    }

    result.row = scan.table.rows[result.rowIndex];
    result.rowKnown = result.row.known;
    result.known = result.known && result.rowKnown;
    return result;
}

MovieStepSegmentFields801C4350 BuildMovieStepSegmentFields801C4350(
    const MovieSegmentRecord48& record) {
    MovieStepSegmentFields801C4350 result{};
    result.segmentTimeBaseA1Plus40Known = record.timeBaseA1Plus40Known;
    result.segmentTimeBaseA1Plus40 = record.timeBaseA1Plus40;
    result.segmentEndA1Plus44Known = record.endA1Plus44Known;
    result.segmentEndA1Plus44 = record.endA1Plus44;
    result.segmentEndBiasA1Plus8Known = record.endBiasA1Plus8Known;
    result.segmentEndBiasA1Plus8 = record.endBiasA1Plus8;
    return result;
}

}  // namespace PrMovieSegmentDirect
