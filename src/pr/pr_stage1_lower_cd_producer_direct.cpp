#include "pr_stage1_lower_cd_producer_direct.h"

namespace PrStage1LowerCdProducerDirect {
namespace {

constexpr uint8_t kCommandParamCount8005733C[32] = {
    0, 0, 3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 1, 0,
    0, 0, 1, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr uint8_t kCommandResetReadyByte8005723C[64] = {
    0, 0, 0, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

constexpr uint32_t kCdEventStatusUpdateTable800572BC[64] = {
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
    0u, 0u, 1u, 1u, 1u, 1u, 1u, 1u,
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
    0u, 0u, 3u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u, 0u, 2u, 1u, 0u,
    0u, 0u, 1u, 0u, 1u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
};

constexpr uint32_t kCdEventStatus3Table800571BC[64] = {
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 1u,
    1u, 1u, 1u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 1u, 0u, 0u, 1u, 1u, 0u,
    0u, 0u, 1u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 1u, 1u, 1u, 1u, 0u,
    0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u, 0u, 1u, 1u, 0u,
    0u, 0u, 0u, 1u, 0u, 0u, 0u, 0u,
};

PrStage1LoaderDirect::CdSeamResult MakeBaseCdSeam(
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request,
    const PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution&
        overlayTransferAttempt) {
    PrStage1LoaderDirect::CdSeamResult cd{};
    cd.present = true;
    cd.lowerRequest = request;
    cd.overlayTransferAttempt = overlayTransferAttempt;
    cd.feedback.kind = request.actionKind;
    cd.feedback.handled = true;
    return cd;
}

bool IsRequestKind(
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request,
    PrStage1LoaderCdHal::ActionKind kind) {
    return request.known && request.actionKind == kind;
}

bool SameBcdMsf80036974(const PrStage1LoaderCdHal::BcdMsf80036974& lhs,
                        const PrStage1LoaderCdHal::BcdMsf80036974& rhs) {
    return lhs.minute == rhs.minute &&
           lhs.second == rhs.second &&
           lhs.frame == rhs.frame;
}

bool IsSeekSyncRequest8001A89C367A4(
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request) {
    return IsRequestKind(request,
                         PrStage1LoaderCdHal::ActionKind::SeekSync800367A4) &&
           request.callerFunction == PrStage1LoaderCdHal::kFn8001A89C &&
           request.directHelperFunction == PrStage1LoaderCdHal::kFn80036974 &&
           request.lowerFunction == PrStage1LoaderCdHal::kFn800367A4 &&
           request.seekRequestKnown &&
           request.seekMsfTargetPtrKnown &&
           request.seekLbaKnown &&
           request.seekMsfTargetKnown &&
           request.seekArg0 == PrStage1LoaderCdHal::kSeek8001A89CSyncMode &&
           request.seekArg1 ==
               static_cast<int32_t>(request.seekMsfTargetPtr) &&
           request.seekArg2 == PrStage1LoaderCdHal::kSeek8001A89CSyncArg2;
}

bool IsSeekSyncHalRequest8001A89C367A4(
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request) {
    return IsRequestKind(request,
                         PrStage1LoaderCdHal::ActionKind::SeekSync800367A4) &&
           request.callerFunction == PrStage1LoaderCdHal::kFn8001A89C &&
           request.directHelperFunction == PrStage1LoaderCdHal::kFn80036974 &&
           request.lowerFunction == PrStage1LoaderCdHal::kFn800367A4 &&
           request.finalFunction == 0u &&
           request.seekRequestKnown &&
           request.seekArg0 == PrStage1LoaderCdHal::kSeek8001A89CSyncMode &&
           (!request.seekMsfTargetPtrKnown ||
            request.seekArg1 ==
                static_cast<int32_t>(request.seekMsfTargetPtr)) &&
           request.seekArg2 == PrStage1LoaderCdHal::kSeek8001A89CSyncArg2;
}

bool IsReadStartRequest8001A81838FC0(
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request) {
    return IsRequestKind(request,
                         PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0) &&
           request.callerFunction == PrStage1LoaderCdHal::kFn8001A818 &&
           request.lowerFunction == PrStage1LoaderCdHal::kFn80038FC0 &&
           request.finalFunction == PrStage1LoaderCdHal::kFn800390C8 &&
           request.readStartRequestKnown &&
           request.readStartSectorCountKnown &&
           request.readStartModeFlagKnown &&
           request.readStartArg0 == request.readStartSectorCount &&
           request.readStartArg1 ==
               static_cast<int32_t>(request.readStartDstPtr) &&
           request.readSyncRequestKnown &&
           request.readSyncFunction == PrStage1LoaderCdHal::kFn800390C8 &&
           request.readSyncArg0 ==
               PrStage1LoaderCdHal::kRead8001A818SyncArg0 &&
           request.readSyncArg1 ==
               PrStage1LoaderCdHal::kRead8001A818SyncArg1;
}

bool ReadStartSetupMatchesRequest80038FC0(
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request,
    const ReadStartSetupResult80038FC0& setup) {
    if (setup.sectorCount80057410 != request.readStartSectorCount ||
        setup.dst80057414 !=
            static_cast<int32_t>(request.readStartDstPtr) ||
        setup.modeWord8005741C !=
            (static_cast<uint32_t>(request.readStartModeFlag) | 0x20u)) {
        return false;
    }
    return true;
}

bool ReadPumpGlobalsMatchSetup80038DE8(
    const ReadPumpResult80038DE8& pump,
    const ReadStartSetupResult80038FC0& setup) {
    if (!pump.globalsKnown) {
        return true;
    }
    return pump.activeDst80057418 == setup.dst80057414 &&
           pump.activeSectorCount80057410 == setup.sectorCount80057410 &&
           pump.remaining80057424 == setup.sectorCount80057410 &&
           pump.startClock8005742C == setup.startClock8005742C;
}

bool IsValidRawCallbackPumpSequence(
    bool sequenceKnown,
    uint32_t pumpCount,
    const RawCdRegTransactionResult80036AF8* transactions,
    const int32_t* psxReturns) {
    if (!sequenceKnown || transactions == nullptr || psxReturns == nullptr ||
        pumpCount == 0u || pumpCount > 16u) {
        return false;
    }

    for (uint32_t i = 0u; i < pumpCount; ++i) {
        if (!transactions[i].produced || transactions[i].incomplete) {
            return false;
        }
        if (!transactions[i].psxReturnKnown ||
            transactions[i].psxReturn != psxReturns[i]) {
            return false;
        }
        if (i + 1u < pumpCount && psxReturns[i] == 0) {
            return false;
        }
    }

    return psxReturns[pumpCount - 1u] == 0;
}

bool IsValidRawCdSyncPumpSequence80037070(
    bool sequenceKnown,
    uint32_t pumpCount,
    const RawCdRegTransactionResult80036AF8* transactions,
    const int32_t* psxReturns) {
    if (!sequenceKnown || transactions == nullptr || psxReturns == nullptr ||
        pumpCount == 0u || pumpCount > 16u) {
        return false;
    }

    for (uint32_t i = 0u; i < pumpCount; ++i) {
        if (!transactions[i].produced || transactions[i].incomplete) {
            return false;
        }
        if (!transactions[i].psxReturnKnown ||
            transactions[i].psxReturn != psxReturns[i]) {
            return false;
        }
        if (i + 1u < pumpCount && psxReturns[i] == 0) {
            return false;
        }
    }

    return true;
}

bool TryReadCdEventTable80036AF8(const uint32_t* table,
                                 uint32_t tableCount,
                                 uint8_t command,
                                 uint32_t* value) {
    if (command >= tableCount) {
        return false;
    }
    if (value != nullptr) {
        *value = table[command];
    }
    return true;
}

void CopyRawCdPaddedResponse80036AF8(uint8_t (&dst)[8],
                                     const uint8_t (&src)[8]) {
    for (uint32_t i = 0u; i < 8u; ++i) {
        dst[i] = src[i];
    }
}

bool SameRawCdResponse80036AF8(const uint8_t (&lhs)[8],
                               const uint8_t (&rhs)[8]) {
    for (uint32_t i = 0u; i < 8u; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

struct RawCdSyncBankFacts80036AF8 {
    bool byte800573D4Known = false;
    uint8_t byte800573D4 = 0;
    bool response882F8Known = false;
    uint8_t response882F8[8]{};
};

RawCdSyncBankFacts80036AF8 ResolveRawCdSyncBankFacts80036AF8(
    uint32_t transactionCount,
    const RawCdRegTransactionResult80036AF8* transactions) {
    RawCdSyncBankFacts80036AF8 out{};
    if (transactions == nullptr) {
        return out;
    }

    for (uint32_t i = 0u; i < transactionCount; ++i) {
        const RawCdRegTransactionResult80036AF8& transaction =
            transactions[i];
        if (!transaction.produced || transaction.incomplete) {
            continue;
        }
        if (transaction.byte800573D4Known) {
            out.byte800573D4Known = true;
            out.byte800573D4 = transaction.byte800573D4;
        }
        if (transaction.response882F8Known) {
            out.response882F8Known = true;
            CopyRawCdPaddedResponse80036AF8(out.response882F8,
                                            transaction.response882F8);
        }
    }
    return out;
}

struct RawCdReadyBankFacts80036AF8 {
    bool byte800573D6Known = false;
    uint8_t byte800573D6 = 0;
    bool response88308Known = false;
    uint8_t response88308[8]{};
    bool byte800573D5Known = false;
    uint8_t byte800573D5 = 0;
    bool response88300Known = false;
    uint8_t response88300[8]{};
};

RawCdReadyBankFacts80036AF8 ResolveRawCdReadyBankFacts80036AF8(
    uint32_t transactionCount,
    const RawCdRegTransactionResult80036AF8* transactions) {
    RawCdReadyBankFacts80036AF8 out{};
    if (transactions == nullptr) {
        return out;
    }

    for (uint32_t i = 0u; i < transactionCount; ++i) {
        const RawCdRegTransactionResult80036AF8& transaction =
            transactions[i];
        if (!transaction.produced || transaction.incomplete) {
            continue;
        }
        if (transaction.byte800573D6Known) {
            out.byte800573D6Known = true;
            out.byte800573D6 = transaction.byte800573D6;
        }
        if (transaction.response88308Known) {
            out.response88308Known = true;
            CopyRawCdPaddedResponse80036AF8(out.response88308,
                                            transaction.response88308);
        }
        if (transaction.byte800573D5Known) {
            out.byte800573D5Known = true;
            out.byte800573D5 = transaction.byte800573D5;
        }
        if (transaction.response88300Known) {
            out.response88300Known = true;
            CopyRawCdPaddedResponse80036AF8(out.response88300,
                                            transaction.response88300);
        }
    }
    return out;
}

} // namespace

CdRegisterPointerSnapshotResult80036AF8
BuildCdRegisterPointerSnapshotResult80036AF8(
    const CdRegisterPointerSnapshot80036AF8& input) {
    CdRegisterPointerSnapshotResult80036AF8 out{};
    if (!input.cdReg0PtrKnown ||
        !input.cdReg1PtrKnown ||
        !input.cdReg2PtrKnown ||
        !input.cdReg3PtrKnown ||
        !input.selectorWriteKnown) {
        out.incomplete = true;
        return out;
    }

    out.pointersMatched =
        input.cdReg0Ptr == kCdReg0Ptr80036AF8 &&
        input.cdReg1Ptr == kCdReg1Ptr80036AF8 &&
        input.cdReg2Ptr == kCdReg2Ptr80036AF8 &&
        input.cdReg3Ptr == kCdReg3Ptr80036AF8;
    out.selectorWriteMatched = input.selectorWriteValue == 1u;
    if (!out.pointersMatched || !out.selectorWriteMatched) {
        out.incomplete = true;
        return out;
    }

    out.produced = true;
    return out;
}

RawCdRegTransactionResult80036AF8 BuildRawCdRegTransactionResult80036AF8(
    const RawCdRegTransactionInput80036AF8& input) {
    RawCdRegTransactionResult80036AF8 out{};
    out.registerPointers =
        BuildCdRegisterPointerSnapshotResult80036AF8(
            input.registerPointers);
    if (!input.cdReg3InitialInterruptKnown ||
        !out.registerPointers.produced ||
        out.registerPointers.incomplete) {
        out.incomplete = true;
        return out;
    }

    out.cdReg3InitialInterrupt = input.cdReg3InitialInterrupt & 7u;
    if (out.cdReg3InitialInterrupt == 0u) {
        out.earlyReturnNoInterrupt = true;
        out.psxReturnKnown = true;
        out.psxReturn = 0;
        out.produced = true;
        return out;
    }

    if (!input.cdReg3StableInterruptKnown ||
        !input.cdReg0StatusKnown ||
        !input.cdReg0FifoStatusSamplesKnown ||
        !input.resultByteCountKnown ||
        !input.ackWritesKnown ||
        !input.priorFacts.dword80057108Known ||
        !input.priorFacts.dword80057110Known ||
        !input.priorFacts.byte80057119Known) {
        out.incomplete = true;
        return out;
    }

    if (input.cdReg0FifoStatusSampleCount == 0u ||
        input.cdReg0FifoStatusSampleCount > 9u ||
        input.cdReg0FifoStatusSamples[0] != input.cdReg0Status) {
        out.incomplete = true;
        return out;
    }

    if (input.resultByteCount > 8u ||
        (input.resultByteCount != 0u && !input.resultBytesKnown)) {
        out.incomplete = true;
        return out;
    }

    out.cdReg3StableInterrupt = input.cdReg3StableInterrupt & 7u;
    out.cdReg0StatusKnown = true;
    out.cdReg0Status = input.cdReg0Status;
    out.cdReg0FifoStatusSamplesKnown = true;
    out.cdReg0FifoStatusSampleCount =
        input.cdReg0FifoStatusSampleCount;
    for (uint32_t i = 0u; i < input.cdReg0FifoStatusSampleCount; ++i) {
        out.cdReg0FifoStatusSamples[i] =
            input.cdReg0FifoStatusSamples[i];
    }

    out.fifoDrainRequired = (input.cdReg0Status & 0x20u) != 0u;
    if (out.fifoDrainRequired) {
        if (!input.fifoDrainKnown || !input.fifoDrained ||
            input.fifoDrainCount > 8u ||
            input.cdReg0FifoStatusSampleCount < input.fifoDrainCount) {
            out.incomplete = true;
            return out;
        }
        for (uint32_t i = 0u; i < input.fifoDrainCount; ++i) {
            if ((input.cdReg0FifoStatusSamples[i] & 0x20u) == 0u) {
                out.incomplete = true;
                return out;
            }
        }
        if (input.fifoDrainCount < 8u) {
            if (input.cdReg0FifoStatusSampleCount <
                    input.fifoDrainCount + 1u ||
                (input.cdReg0FifoStatusSamples[input.fifoDrainCount] &
                    0x20u) != 0u) {
                out.incomplete = true;
                return out;
            }
            out.fifoStoppedOnNotReady = true;
        } else {
            out.fifoStoppedAtLimit = true;
        }
        out.fifoDrained = true;
        out.fifoDrainCount = input.fifoDrainCount;
    } else {
        if ((input.cdReg0FifoStatusSamples[0] & 0x20u) != 0u ||
            (input.fifoDrainKnown && input.fifoDrainCount != 0u)) {
            out.incomplete = true;
            return out;
        }
        out.fifoStoppedOnNotReady = true;
    }

    uint8_t paddedResultBytes[8]{};
    for (uint32_t i = 0u; i < input.resultByteCount; ++i) {
        paddedResultBytes[i] = input.resultBytes[i];
    }

    out.resultBytesKnown =
        input.resultBytesKnown || input.resultByteCount == 0u;
    out.resultByteCount = input.resultByteCount;
    for (uint32_t i = 0u; i < input.resultByteCount; ++i) {
        out.resultBytes[i] = input.resultBytes[i];
    }

    out.ackWritesMatched80036AF8 =
        input.ackCdReg0 == 1u &&
        input.ackCdReg3 == 7u &&
        input.ackCdReg2 == 7u;
    if (!out.ackWritesMatched80036AF8) {
        out.incomplete = true;
        return out;
    }

    if (out.cdReg3StableInterrupt == 1u) {
        if (!input.case1ClearWritesKnown ||
            input.case1ClearCdReg0 != 0u ||
            input.case1ClearCdReg3 != 0u) {
            out.incomplete = true;
            return out;
        }
        out.case1ClearMatched80036AF8 = true;
    }

    const uint8_t command = input.priorFacts.byte80057119;
    bool skipStatusGlobals = false;
    if (out.cdReg3StableInterrupt == 3u) {
        uint32_t statusUpdate = 0u;
        if (!TryReadCdEventTable80036AF8(
                kCdEventStatusUpdateTable800572BC,
                sizeof(kCdEventStatusUpdateTable800572BC) /
                    sizeof(kCdEventStatusUpdateTable800572BC[0]),
                command,
                &statusUpdate)) {
            out.incomplete = true;
            return out;
        }
        skipStatusGlobals = statusUpdate == 0u;
    }

    if (!skipStatusGlobals) {
        if ((paddedResultBytes[0] & 0x10u) != 0u &&
            (input.priorFacts.dword80057108 & 0x10u) == 0u) {
            out.dword80057110WrittenKnown = true;
            out.dword80057110Written =
                input.priorFacts.dword80057110 + 1u;
            out.dword80057110Incremented = true;
        }
        out.dword80057108WrittenKnown = true;
        out.dword80057108Written = paddedResultBytes[0];
        out.dword8005710CWrittenKnown = true;
        out.dword8005710CWritten = paddedResultBytes[1];
    }

    const uint8_t syncMask =
        skipStatusGlobals ? 0u : (paddedResultBytes[0] & 0x1Du);
    switch (out.cdReg3StableInterrupt) {
    case 1: {
        uint8_t status = 1u;
        if (syncMask != 0u) {
            if (out.resultByteCount == 1u) {
                status = 1u;
            } else {
                status = 5u;
            }
        }
        out.byte800573D5Known = true;
        out.byte800573D5 = status;
        out.response88300Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response88300,
                                        paddedResultBytes);
        out.psxReturnKnown = true;
        out.psxReturn = 4;
        break;
    }
    case 2: {
        out.byte800573D4Known = true;
        out.byte800573D4 = syncMask != 0u ? 5u : 2u;
        out.response882F8Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response882F8,
                                        paddedResultBytes);
        out.psxReturnKnown = true;
        out.psxReturn = 2;
        break;
    }
    case 3: {
        out.byte800573D4Known = true;
        if (syncMask != 0u) {
            out.byte800573D4 = 5u;
            out.psxReturn = 2;
        } else {
            uint32_t status3 = 0u;
            if (!TryReadCdEventTable80036AF8(
                    kCdEventStatus3Table800571BC,
                    sizeof(kCdEventStatus3Table800571BC) /
                        sizeof(kCdEventStatus3Table800571BC[0]),
                    command,
                    &status3)) {
                out.incomplete = true;
                return out;
            }
            out.byte800573D4 = status3 != 0u ? 3u : 2u;
            out.psxReturn = status3 != 0u ? 1 : 2;
        }
        out.response882F8Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response882F8,
                                        paddedResultBytes);
        out.psxReturnKnown = true;
        break;
    }
    case 4:
        out.byte800573D6Known = true;
        out.byte800573D6 = 4u;
        out.byte800573D5Known = true;
        out.byte800573D5 = out.byte800573D6;
        out.response88308Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response88308,
                                        paddedResultBytes);
        out.response88300Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response88300,
                                        paddedResultBytes);
        out.psxReturnKnown = true;
        out.psxReturn = 4;
        break;
    case 5:
        out.byte800573D5Known = true;
        out.byte800573D5 = 5u;
        out.byte800573D4Known = true;
        out.byte800573D4 = out.byte800573D5;
        out.response882F8Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response882F8,
                                        paddedResultBytes);
        out.response88300Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response88300,
                                        paddedResultBytes);
        out.psxReturnKnown = true;
        out.psxReturn = 6;
        break;
    default:
        out.unknownInterrupt = true;
        out.psxReturnKnown = true;
        out.psxReturn = 0;
        break;
    }

    out.priorFacts = input.priorFacts;
    out.produced = true;
    return out;
}

CdInterruptSnapshotResult800359B8 BuildCdInterruptSnapshotResult800359B8(
    const CdInterruptSnapshotInput800359B8& input) {
    CdInterruptSnapshotResult800359B8 out{};
    if (!input.iStatPtrKnown ||
        !input.iMaskPtrKnown ||
        !input.iStatKnown ||
        !input.iMaskKnown ||
        !input.word80055FA8Known ||
        !input.word80055F78Known ||
        !input.word80055F7ABeforeKnown ||
        !input.word80055F7ASetWriteKnown ||
        !input.word80055F7AClearWriteKnown ||
        !input.watchdogKnown ||
        !input.pendingSampleSequenceKnown ||
        !input.callbackTableKnown ||
        !input.callbackTableBaseKnown) {
        out.incomplete = true;
        return out;
    }

    if (input.pendingSampleCount == 0u ||
        input.pendingSampleCount > kCdInterruptSnapshotMaxSamples800359B8 ||
        input.iStatSamples[0] != input.iStat ||
        input.iMaskSamples[0] != input.iMask ||
        input.word80055FA8Samples[0] != input.word80055FA8) {
        out.incomplete = true;
        return out;
    }

    out.pointerSnapshotMatched =
        input.iStatPtr == kIStatPtr800359B8 &&
        input.iMaskPtr == kIMaskPtr800359B8 &&
        input.callbackTableBase == kCdCallbackTableBase800359B8;
    out.word80055F7ASetWriteMatched = input.word80055F7ASetWrite == 1u;
    out.word80055F7AClearWriteMatched = input.word80055F7AClearWrite == 0u;
    if (!out.pointerSnapshotMatched ||
        !out.word80055F7ASetWriteMatched ||
        !out.word80055F7AClearWriteMatched) {
        out.incomplete = true;
        return out;
    }

    out.pendingMaskKnown = true;
    out.pendingMask = static_cast<uint16_t>(
        input.iStat & input.iMask & input.word80055FA8);
    out.returnFromExceptionBeforePending = input.word80055F78 == 0u;

    bool sawTerminalNoPending = false;
    uint16_t lastIStat = input.iStat;
    uint16_t lastIMask = input.iMask;
    for (uint32_t sample = 0u; sample < input.pendingSampleCount; ++sample) {
        lastIStat = input.iStatSamples[sample];
        lastIMask = input.iMaskSamples[sample];
        const uint16_t enabledMask = static_cast<uint16_t>(
            input.iMaskSamples[sample] &
            input.word80055FA8Samples[sample]);
        const uint16_t pendingMask =
            static_cast<uint16_t>(lastIStat & enabledMask);
        out.pendingMaskSamples[sample] = pendingMask;
        out.pendingSampleCount = sample + 1u;
        if (pendingMask == 0u) {
            sawTerminalNoPending = true;
            break;
        }
        if (out.ackCount >= kCdCallbackCount800359B8) {
            out.incomplete = true;
            return out;
        }

        uint8_t bitIndex = 0u;
        while (bitIndex < kCdCallbackCount800359B8 &&
               (pendingMask & (1u << bitIndex)) == 0u) {
            ++bitIndex;
        }
        if (bitIndex >= kCdCallbackCount800359B8) {
            break;
        }

        const uint16_t bitMask =
            static_cast<uint16_t>(1u << bitIndex);
        out.ackBitIndex[out.ackCount] = bitIndex;
        out.ackWriteValue[out.ackCount] =
            static_cast<uint16_t>(~bitMask);
        ++out.ackCount;

        if (input.callbackPresent[bitIndex]) {
            if (!input.callbackAddressKnown[bitIndex]) {
                out.incomplete = true;
                return out;
            }
            out.dispatchBitIndex[out.dispatchCount] = bitIndex;
            out.dispatchedCallbackAddress[out.dispatchCount] =
                input.callbackAddress[bitIndex];
            ++out.dispatchCount;
        }
    }
    if (!sawTerminalNoPending) {
        out.incomplete = true;
        return out;
    }

    out.callbackTableComplete = true;
    for (uint32_t i = 0u; i < kCdCallbackCount800359B8; ++i) {
        if (input.callbackPresent[i] &&
            !input.callbackAddressKnown[i]) {
            out.callbackTableComplete = false;
        }
    }

    out.watchdogKnown = true;
    out.dword80057010 = input.dword80057010;
    out.watchdogWritten = true;
    if ((lastIStat & lastIMask) != 0u) {
        out.dword80057010Written = input.dword80057010 + 1u;
        out.watchdogTimeout = out.dword80057010Written >= 0x801u;
        out.interruptStatusClearedOnTimeout = out.watchdogTimeout;
    } else {
        out.dword80057010Written = 0u;
    }

    out.produced = true;
    return out;
}

CdCallbackPendingBridgeResult800359B8
BuildCdCallbackPendingBridgeInput800359B8(
    const CdInterruptSnapshotInput800359B8& input) {
    CdCallbackPendingBridgeResult800359B8 out{};
    const CdInterruptSnapshotResult800359B8 snapshot =
        BuildCdInterruptSnapshotResult800359B8(input);
    if (!snapshot.produced || snapshot.incomplete) {
        out.incomplete = true;
        return out;
    }

    out.input.writeAddressKnown = true;
    out.input.writeAddress = PrMovieSegmentDirect::kSub800359B8SetWord80055F7A;
    out.input.word80055F78Known = input.word80055F78Known;
    out.input.word80055F78 = input.word80055F78;
    out.input.word80055FA8Known = input.word80055FA8Known;
    out.input.word80055FA8 = input.word80055FA8;
    out.input.interruptStatusKnown = input.iStatKnown;
    out.input.interruptStatus = input.iStat;
    out.input.interruptMaskKnown = input.iMaskKnown;
    out.input.interruptMask = input.iMask;
    out.input.watchdogKnown = input.watchdogKnown;
    out.input.dword80057010 = input.dword80057010;
    out.input.callbackTableKnown = input.callbackTableKnown;
    for (uint32_t i = 0u;
         i < PrMovieSegmentDirect::
                 kCdCallbackPendingProducerCallbackCount800359B8 &&
         i < kCdCallbackCount800359B8;
         ++i) {
        out.input.callbackPresent[i] = input.callbackPresent[i];
    }
    out.produced = true;
    return out;
}

CdCallbackEventRegisterBridgeResult80036AF8
BuildCdCallbackEventRegisterBridgeInput80036AF8(
    const RawCdRegTransactionResult80036AF8& input) {
    CdCallbackEventRegisterBridgeResult80036AF8 out{};
    if (!input.produced || input.incomplete) {
        out.incomplete = true;
        return out;
    }
    if (input.earlyReturnNoInterrupt) {
        out.earlyReturnNoInterrupt = true;
        out.produced = true;
        return out;
    }
    if (!input.resultBytesKnown ||
        !input.priorFacts.dword80057108Known ||
        !input.priorFacts.dword80057110Known ||
        !input.priorFacts.byte80057119Known) {
        out.incomplete = true;
        return out;
    }

    out.input.cdReg3InitialKnown = true;
    out.input.cdReg3Initial = input.cdReg3InitialInterrupt;
    out.input.cdReg3StableKnown = true;
    out.input.cdReg3Stable = input.cdReg3StableInterrupt;
    out.input.resultFifoKnown = true;
    out.input.resultFifoByteCount = input.resultByteCount;
    for (uint32_t i = 0u;
         i < input.resultByteCount &&
         i < out.input.resultFifoBytes.size();
         ++i) {
        out.input.resultFifoBytes[i] = input.resultBytes[i];
    }
    out.input.commandKnown = true;
    out.input.command = input.priorFacts.byte80057119;
    out.input.priorDword80057108Known = true;
    out.input.priorDword80057108 = input.priorFacts.dword80057108;
    out.input.priorDword80057110Known = true;
    out.input.priorDword80057110 = input.priorFacts.dword80057110;
    out.input.priorSyncMaskKnown = true;
    out.input.priorSyncMask =
        static_cast<uint8_t>(input.priorFacts.dword80057108 & 0xFFu);
    out.produced = true;
    return out;
}

CdSyncLowerFeedbackResult80037070
BuildCdSyncLowerFeedback80037070FromCoreFacts(
    const CdSyncLowerFeedbackInput80037070& input) {
    CdSyncLowerFeedbackResult80037070 out{};
    if (!input.timedOutKnown) {
        out.incomplete = true;
        return out;
    }

    out.feedback.known = true;
    if (input.timedOut) {
        out.timedOut = true;
        out.feedback.timedOut = true;
        out.feedback.syncResultKnown = true;
        out.feedback.syncResult = -1;
        out.produced = true;
        return out;
    }

    if (input.syncResultKnown) {
        if (input.rawEvent80036AF8Known ||
            (input.syncResult != 0 &&
             input.syncResult != 2 &&
             input.syncResult != 5)) {
            out.incomplete = true;
            return out;
        }
        if (input.syncResult == 2 || input.syncResult == 5) {
            if (!input.responseBytesKnown ||
                input.responseByteCount != sizeof(input.responseBytes)) {
                out.incomplete = true;
                return out;
            }
            out.feedback.responseBytesKnown = true;
            out.feedback.responseByteCount =
                input.responseByteCount > out.feedback.responseBytes.size()
                    ? static_cast<uint32_t>(
                          out.feedback.responseBytes.size())
                    : input.responseByteCount;
            for (uint32_t i = 0; i < out.feedback.responseByteCount; ++i) {
                out.feedback.responseBytes[i] = input.responseBytes[i];
            }
        }
        out.feedback.syncResultKnown = true;
        out.feedback.syncResult = input.syncResult;
        out.produced = true;
        return out;
    }

    out.incomplete = true;
    return out;
}

CdSyncLoopFactsResult80037070 BuildCdSyncLoopFacts80037070(
    const CdSyncLoopFactsInput80037070& input) {
    CdSyncLoopFactsResult80037070 out{};
    if (!input.timeoutKnown) {
        out.incomplete = true;
        return out;
    }

    out.coreFacts.timedOutKnown = true;
    out.coreFacts.timedOut = input.timedOut;
    out.timedOut = input.timedOut;
    if (input.timedOut) {
        out.coreFactsKnown = true;
        out.produced = true;
        return out;
    }

    if (!input.checkCallbackKnown) {
        out.incomplete = true;
        return out;
    }

    out.checkCallbackKnown = true;
    out.callbackPending = input.callbackPending;
    if (!input.callbackPending) {
        if (!input.a0WaitModeKnown || input.a0WaitMode == 0) {
            out.incomplete = true;
            return out;
        }
        out.coreFacts.syncResultKnown = true;
        out.coreFacts.syncResult = 0;
        out.noPendingReturnZero = true;
        out.coreFactsKnown = true;
        out.produced = true;
        return out;
    }

    out.callbackPumpRequired = true;
    if (!input.savedCdReg0SelectorKnown ||
        !input.selectorRestoredKnown ||
        !input.selectorRestored ||
        !input.callbackPumpDrained ||
        !IsValidRawCdSyncPumpSequence80037070(
            input.rawPumpSequenceKnown,
            input.rawPumpCount,
            input.rawPump,
            input.rawPumpPsxReturns)) {
        out.incomplete = true;
        return out;
    }

    const uint8_t savedSelector =
        static_cast<uint8_t>(input.savedCdReg0Selector & 3u);
    if ((input.restoredCdReg0Selector & 3u) != savedSelector) {
        out.incomplete = true;
        return out;
    }

    out.savedCdReg0SelectorKnown = true;
    out.savedCdReg0Selector = savedSelector;
    out.selectorRestored = true;
    out.restoredCdReg0Selector =
        static_cast<uint8_t>(input.restoredCdReg0Selector & 3u);
    out.callbackPumpDrained = true;

    const RawCdSyncBankFacts80036AF8 rawSyncFacts =
        ResolveRawCdSyncBankFacts80036AF8(input.rawPumpCount,
                                          input.rawPump);
    bool byte800573D4Known = false;
    uint8_t byte800573D4 = 0u;
    if (rawSyncFacts.byte800573D4Known) {
        if (input.byte800573D4Known &&
            input.byte800573D4 != rawSyncFacts.byte800573D4) {
            out.incomplete = true;
            return out;
        }
        byte800573D4Known = true;
        byte800573D4 = rawSyncFacts.byte800573D4;
    }

    bool response882F8Known = false;
    uint8_t response882F8[8]{};
    if (rawSyncFacts.response882F8Known) {
        if (input.response882F8Known &&
            !SameRawCdResponse80036AF8(input.response882F8,
                                       rawSyncFacts.response882F8)) {
            out.incomplete = true;
            return out;
        }
        response882F8Known = true;
        CopyRawCdPaddedResponse80036AF8(response882F8,
                                        rawSyncFacts.response882F8);
    }

    if (!byte800573D4Known) {
        out.incomplete = true;
        return out;
    }

    if (byte800573D4 != 2u && byte800573D4 != 5u) {
        if (!input.a0WaitModeKnown || input.a0WaitMode == 0) {
            out.incomplete = true;
            return out;
        }
        out.coreFacts.syncResultKnown = true;
        out.coreFacts.syncResult = 0;
        out.nonblockingReturnZero = true;
        out.coreFactsKnown = true;
        out.produced = true;
        return out;
    }

    if (!response882F8Known ||
        (input.response882F8Known &&
         input.response882F8ByteCount != sizeof(input.response882F8))) {
        out.incomplete = true;
        return out;
    }

    out.coreFacts.syncResultKnown = true;
    out.coreFacts.syncResult = byte800573D4;
    out.coreFacts.responseBytesKnown = true;
    out.coreFacts.responseByteCount =
        static_cast<uint32_t>(sizeof(out.coreFacts.responseBytes));
    for (uint32_t i = 0; i < out.coreFacts.responseByteCount; ++i) {
        out.coreFacts.responseBytes[i] = response882F8[i];
    }
    out.coreFactsKnown = true;
    out.produced = true;
    return out;
}

CommandAttr800375BC ResolveCommandAttr800375BC(uint8_t command) {
    CommandAttr800375BC out{};
    out.command = command;
    if (command >= 32u) {
        return out;
    }

    out.known = true;
    out.paramCount = kCommandParamCount8005733C[command];
    out.resetReadyByte573D5 =
        command < 64u && kCommandResetReadyByte8005723C[command] != 0u;
    out.mirrorsSetlocTo57114 = command == 2u;
    return out;
}

CommandInput800375BC BuildCommandInput800375BC(
    uint8_t command,
    bool argsKnown,
    uint32_t argCount,
    const uint8_t* args,
    bool preSyncResultKnown,
    int32_t preSyncPsxReturn,
    bool skipWait,
    bool clockKnown,
    int32_t clockNow,
    bool timeoutKnown,
    bool timedOut,
    bool checkCallbackKnown,
    bool callbackPending,
    bool savedCdReg0SelectorKnown,
    uint8_t savedCdReg0Selector,
    bool callbackPumpKnown,
    uint32_t callbackPumpCount,
    bool callbackPumpDrained,
    int32_t lastCallbackPumpReturn,
    bool rawCallbackTransactionSequenceKnown,
    const RawCdRegTransactionResult80036AF8* rawCallbackTransactions,
    const int32_t* rawCallbackPsxReturns,
    bool selectorRestored,
    bool waitLoopResultKnown,
    int32_t waitLoopPsxReturn) {
    CommandInput800375BC out{};
    out.command = command;
    out.argsKnown = argsKnown;
    out.argCount = argCount;
    out.args = args;
    const CommandAttr800375BC attr = ResolveCommandAttr800375BC(command);
    out.requiredArgCountKnown = attr.known;
    out.requiredArgCount = attr.paramCount;
    out.clearsByte800573D5 = attr.resetReadyByte573D5;
    out.preSyncResultKnown = preSyncResultKnown;
    out.preSyncPsxReturn = preSyncPsxReturn;
    out.skipWait = skipWait;
    out.clockKnown = clockKnown;
    out.clockNow = clockNow;
    out.timeoutKnown = timeoutKnown;
    out.timedOut = timedOut;
    out.checkCallbackKnown = checkCallbackKnown;
    out.callbackPending = callbackPending;
    out.savedCdReg0SelectorKnown = savedCdReg0SelectorKnown;
    out.savedCdReg0Selector = savedCdReg0Selector;
    out.callbackPumpKnown = callbackPumpKnown;
    out.callbackPumpCount = callbackPumpCount;
    out.callbackPumpDrained = callbackPumpDrained;
    out.lastCallbackPumpReturn = lastCallbackPumpReturn;
    out.rawCallbackTransactionSequenceKnown =
        rawCallbackTransactionSequenceKnown;
    if (rawCallbackTransactionSequenceKnown &&
        rawCallbackTransactions != nullptr &&
        rawCallbackPsxReturns != nullptr) {
        const uint32_t copyCount =
            callbackPumpCount < 16u ? callbackPumpCount : 16u;
        for (uint32_t i = 0u; i < copyCount; ++i) {
            out.rawCallbackTransactions[i] = rawCallbackTransactions[i];
            out.rawCallbackPsxReturns[i] = rawCallbackPsxReturns[i];
        }
    }
    out.selectorRestored = selectorRestored;
    out.waitLoopResultKnown = waitLoopResultKnown;
    out.waitLoopPsxReturn = waitLoopPsxReturn;
    return out;
}

CommandResult800375BC BuildCommandResult800375BC(
    const CommandInput800375BC& input) {
    CommandResult800375BC out{};
    out.command = input.command;

    if (!input.requiredArgCountKnown) {
        out.incomplete = true;
        return out;
    }

    if (input.requiredArgCount != 0u &&
        (!input.argsKnown || input.args == nullptr ||
         input.argCount < input.requiredArgCount)) {
        out.produced = true;
        out.psxReturn = -2;
        return out;
    }

    if (!input.preSyncResultKnown) {
        out.incomplete = true;
        return out;
    }

    out.byte800573D4Known = true;
    out.byte800573D4 = 0;
    out.byte80057119Known = true;
    out.byte80057119 = input.command;
    out.preSyncResultKnown = true;
    out.preSyncPsxReturn = input.preSyncPsxReturn;

    if (input.clearsByte800573D5) {
        out.byte800573D5Known = true;
        out.byte800573D5 = 0;
    }

    if (input.command == 2u && input.argsKnown && input.args != nullptr &&
        input.argCount >= 4u) {
        out.setlocArgsKnown = true;
        for (uint32_t i = 0u; i < 4u; ++i) {
            out.setlocArgs[i] = input.args[i];
        }
    }

    out.cdRegisterWritesKnown = true;
    out.cdRegisterArgWriteCount = input.requiredArgCount;
    for (uint32_t i = 0u; i < input.requiredArgCount && i < 16u; ++i) {
        out.cdRegisterArgWrites[i] = input.args[i];
    }

    out.waitLoopRequested = !input.skipWait;
    if (input.skipWait) {
        out.psxReturn = 0;
        out.produced = true;
        return out;
    }

    if (!input.clockKnown || !input.timeoutKnown) {
        out.incomplete = true;
        return out;
    }

    out.timeoutDeadlineKnown = true;
    out.timeoutStartClock = input.clockNow;
    out.timeoutDeadlineClock = input.clockNow + 960;
    out.timeoutSpinLimit = 0x3C0000u;
    out.timedOut = input.timedOut;
    if (input.timedOut) {
        out.psxReturn = -1;
        out.produced = true;
        return out;
    }

    if (!input.checkCallbackKnown) {
        out.incomplete = true;
        return out;
    }

    out.checkCallbackKnown = true;
    out.callbackPending = input.callbackPending;
    if (input.callbackPending) {
        out.callbackPumpRequired = true;
        if (!input.savedCdReg0SelectorKnown ||
            !input.callbackPumpKnown ||
            !input.callbackPumpDrained ||
            input.lastCallbackPumpReturn != 0 ||
            !input.selectorRestored ||
            !IsValidRawCallbackPumpSequence(
                input.rawCallbackTransactionSequenceKnown,
                input.callbackPumpCount,
                input.rawCallbackTransactions,
                input.rawCallbackPsxReturns)) {
            out.incomplete = true;
            return out;
        }
        out.savedCdReg0SelectorKnown = true;
        out.savedCdReg0Selector = input.savedCdReg0Selector & 3u;
        out.callbackPumpKnown = true;
        out.callbackPumpCount = input.callbackPumpCount;
        out.callbackPumpDrained = input.callbackPumpDrained;
        out.lastCallbackPumpReturn = input.lastCallbackPumpReturn;
        out.selectorRestored = input.selectorRestored;
        out.rawCallbackTransactionSequenceKnown = true;
        out.rawCallbackTransactionCount = input.callbackPumpCount;
        for (uint32_t i = 0u; i < input.callbackPumpCount; ++i) {
            out.rawCallbackTransactions[i] = input.rawCallbackTransactions[i];
            out.rawCallbackPsxReturns[i] = input.rawCallbackPsxReturns[i];
        }
        out.rawCallbackTransactionKnown = true;
        out.rawCallbackTransaction =
            input.rawCallbackTransactions[input.callbackPumpCount - 1u];
    }

    if (!input.waitLoopResultKnown) {
        out.incomplete = true;
        return out;
    }

    out.psxReturn = input.waitLoopPsxReturn;
    out.produced = true;
    return out;
}

CommandWrapperResult80036540 BuildCommandWrapperResult(
    const CommandWrapperInput80036540& input,
    bool finalSkipWait) {
    CommandWrapperResult80036540 out{};
    out.finalSkipWait = finalSkipWait;
    if (!input.argsKnown || !input.status57108Known ||
        (input.argsPresent && !input.commandNeedsSetlocKnown)) {
        out.incomplete = true;
        return out;
    }

    for (uint32_t i = 0u; i < 4u; ++i) {
        ++out.attemptsUsed;
        out.dword800570F8Cleared = true;
        ++out.clearedCallbackCount;

        if (input.command != 1u && (input.status57108 & 0x10u) != 0u) {
            const CommandResult800375BC& restart =
                input.attempts[i].restartCommand1;
            if (!input.attempts[i].restartCommand1Known ||
                !restart.produced ||
                restart.incomplete) {
                out.incomplete = true;
                return out;
            }
            out.issuedCommand1 = true;
        }

        bool mayIssueCommand = !input.argsPresent || !input.commandNeedsSetloc;
        if (!mayIssueCommand) {
            const CommandResult800375BC& pre = input.attempts[i].preSetloc;
            if (!input.attempts[i].preSetlocKnown ||
                !pre.produced ||
                pre.incomplete) {
                out.incomplete = true;
                return out;
            }
            out.issuedSetloc = true;
            mayIssueCommand = pre.psxReturn == 0;
        }

        if (mayIssueCommand) {
            const CommandResult800375BC& command =
                input.attempts[i].command;
            if (!input.attempts[i].commandKnown ||
                !command.produced ||
                command.incomplete) {
                out.incomplete = true;
                return out;
            }
            if (command.psxReturn == 0) {
                out.psxReturn = 1;
                out.produced = true;
                return out;
            }
        }
    }

    out.dword800570F8Cleared = true;
    ++out.clearedCallbackCount;
    out.psxReturn = 0;
    out.produced = true;
    return out;
}

CommandWrapperResult80036540 BuildCommandWrapperResult80036540(
    const CommandWrapperInput80036540& input) {
    return BuildCommandWrapperResult(input, false);
}

CommandWrapperResult80036678 BuildCommandWrapperResult80036678(
    const CommandWrapperInput80036678& input) {
    return BuildCommandWrapperResult(input, true);
}

ReadyWaitReadyBankFacts800372F0 ResolveReadyWaitReadyBankFacts800372F0(
    const ReadyWaitInput800372F0& input) {
    ReadyWaitReadyBankFacts800372F0 out{};
    const bool rawFactsPresent =
        input.rawCallbackTransactionSequenceKnown ||
        input.rawCallbackTransactionCount != 0u ||
        input.rawCallbackTransactionKnown;
    const bool rawFactsAuthorized =
        input.callbackPending && input.callbackPumpDrainedKnown &&
        input.callbackPumpDrained &&
        IsValidRawCallbackPumpSequence(
            input.rawCallbackTransactionSequenceKnown,
            input.rawCallbackTransactionCount,
            input.rawCallbackTransactions,
            input.rawCallbackPsxReturns);
    if (rawFactsPresent && !rawFactsAuthorized) {
        out.incomplete = true;
        return out;
    }

    const RawCdReadyBankFacts80036AF8 rawReadyFacts =
        rawFactsAuthorized
            ? ResolveRawCdReadyBankFacts80036AF8(
                  input.rawCallbackTransactionCount,
                  input.rawCallbackTransactions)
            : RawCdReadyBankFacts80036AF8{};

    if (rawReadyFacts.byte800573D6Known) {
        if (input.byte800573D6Known &&
            input.byte800573D6 != rawReadyFacts.byte800573D6) {
            out.incomplete = true;
            return out;
        }
        out.byte800573D6Known = true;
        out.byte800573D6 = rawReadyFacts.byte800573D6;
    } else if (input.byte800573D6Known) {
        out.byte800573D6Known = true;
        out.byte800573D6 = input.byte800573D6;
    }

    if (rawReadyFacts.response88308Known) {
        if (input.response88308Known &&
            !SameRawCdResponse80036AF8(input.response88308,
                                       rawReadyFacts.response88308)) {
            out.incomplete = true;
            return out;
        }
        out.response88308Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response88308,
                                        rawReadyFacts.response88308);
    } else if (input.response88308Known) {
        out.response88308Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response88308,
                                        input.response88308);
    }

    if (rawReadyFacts.byte800573D5Known) {
        if (input.byte800573D5Known &&
            input.byte800573D5 != rawReadyFacts.byte800573D5) {
            out.incomplete = true;
            return out;
        }
        out.byte800573D5Known = true;
        out.byte800573D5 = rawReadyFacts.byte800573D5;
    } else if (input.byte800573D5Known) {
        out.byte800573D5Known = true;
        out.byte800573D5 = input.byte800573D5;
    }

    if (rawReadyFacts.response88300Known) {
        if (input.response88300Known &&
            !SameRawCdResponse80036AF8(input.response88300,
                                       rawReadyFacts.response88300)) {
            out.incomplete = true;
            return out;
        }
        out.response88300Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response88300,
                                        rawReadyFacts.response88300);
    } else if (input.response88300Known) {
        out.response88300Known = true;
        CopyRawCdPaddedResponse80036AF8(out.response88300,
                                        input.response88300);
    }

    return out;
}

ReadyWaitResult800372F0 BuildReadyWaitResult800372F0(
    const ReadyWaitInput800372F0& input) {
    ReadyWaitResult800372F0 out{};
    if (!input.a0WaitModeKnown ||
        !input.a1OutputBufferPtrKnown ||
        !input.timeoutCheckKnown) {
        out.incomplete = true;
        return out;
    }

    out.a0WaitModeKnown = true;
    out.a0WaitMode = input.a0WaitMode;
    out.a1OutputBufferPtrKnown = true;
    out.a1OutputBufferPtrNonNull = input.a1OutputBufferPtrNonNull;
    out.timeoutChecked = true;
    out.timedOut = input.timedOut;
    if (input.timedOut) {
        out.psxReturn = -1;
        out.produced = true;
        return out;
    }

    if (!input.callbackCheckKnown) {
        out.incomplete = true;
        return out;
    }

    out.callbackCheckKnown = true;
    out.callbackPending = input.callbackPending;
    if (input.callbackPending) {
        out.callbackPumpRequired = true;
        if (!input.callbackPumpDrainedKnown || !input.callbackMaskKnown) {
            out.incomplete = true;
            return out;
        }
        out.callbackPumpDrained = input.callbackPumpDrained;
        if (!input.callbackPumpDrained) {
            out.incomplete = true;
            return out;
        }
        if (!IsValidRawCallbackPumpSequence(
                input.rawCallbackTransactionSequenceKnown,
                input.rawCallbackTransactionCount,
                input.rawCallbackTransactions,
                input.rawCallbackPsxReturns)) {
            out.incomplete = true;
            return out;
        }
        if (input.callbackPumpDrained &&
            input.rawCallbackTransactionCount == 0u) {
            out.incomplete = true;
            return out;
        }
        out.rawCallbackTransactionKnown = true;
        out.rawCallbackTransaction =
            input.rawCallbackTransactions[input.rawCallbackTransactionCount - 1u];
        out.rawCallbackTransactionSequenceKnown = true;
        out.rawCallbackTransactionCount = input.rawCallbackTransactionCount;
        for (uint32_t i = 0u; i < input.rawCallbackTransactionCount; ++i) {
            out.rawCallbackTransactions[i] = input.rawCallbackTransactions[i];
            out.rawCallbackPsxReturns[i] = input.rawCallbackPsxReturns[i];
        }
        out.readyCallbackDispatch =
            (input.callbackMask & 4u) != 0u && input.readyCallbackRegistered;
        out.syncCallbackDispatch =
            (input.callbackMask & 2u) != 0u && input.syncCallbackRegistered;
    }

    const ReadyWaitReadyBankFacts800372F0 readyBank =
        ResolveReadyWaitReadyBankFacts800372F0(input);
    if (readyBank.incomplete) {
        out.incomplete = true;
        return out;
    }

    if (!readyBank.byte800573D6Known || !readyBank.byte800573D5Known) {
        out.incomplete = true;
        return out;
    }

    if (readyBank.byte800573D6 != 0u) {
        if (input.a1OutputBufferPtrNonNull &&
            !readyBank.response88308Known) {
            out.incomplete = true;
            return out;
        }
        out.byte800573D6Consumed = true;
        out.psxReturn = readyBank.byte800573D6;
        if (input.a1OutputBufferPtrNonNull) {
            out.outputBufferWritten = true;
            for (uint32_t i = 0u; i < 8u; ++i) {
                out.outputBuffer[i] = readyBank.response88308[i];
            }
        }
        out.produced = true;
        return out;
    }

    if (readyBank.byte800573D5 != 0u) {
        if (input.a1OutputBufferPtrNonNull &&
            !readyBank.response88300Known) {
            out.incomplete = true;
            return out;
        }
        out.byte800573D5Consumed = true;
        out.psxReturn = readyBank.byte800573D5;
        if (input.a1OutputBufferPtrNonNull) {
            out.outputBufferWritten = true;
            for (uint32_t i = 0u; i < 8u; ++i) {
                out.outputBuffer[i] = readyBank.response88300[i];
            }
        }
        out.produced = true;
        return out;
    }

    if (input.a0WaitMode == 0) {
        out.incomplete = true;
        return out;
    }

    out.psxReturn = 0;
    out.produced = true;
    return out;
}

CallbackSwapResult80036510 BuildCallbackSwapResult80036510(
    const CallbackSwapInput80036510& input) {
    CallbackSwapResult80036510 out{};
    if (!input.oldCallbackKnown) {
        return out;
    }
    out.produced = true;
    out.oldCallback = input.oldCallback;
    out.newCallback = input.newCallback;
    return out;
}

CallbackSwapResult80036528 BuildCallbackSwapResult80036528(
    const CallbackSwapInput80036528& input) {
    return BuildCallbackSwapResult80036510(input);
}

StatusReadResult80036384 BuildStatusReadResult80036384(
    const StatusReadInput80036384& input) {
    StatusReadResult80036384 out{};
    if (!input.statusKnown) {
        out.incomplete = true;
        return out;
    }
    out.produced = true;
    out.status = input.status & 0xFFu;
    return out;
}

LocSectorResult80036A78 BuildLocSectorResult80036A78(
    const LocSectorInput80036A78& input) {
    LocSectorResult80036A78 out{};
    if (!input.locBytesKnown) {
        out.incomplete = true;
        return out;
    }

    const int32_t minute =
        10 * static_cast<int32_t>(input.locBytes[0] >> 4) +
        static_cast<int32_t>(input.locBytes[0] & 0x0Fu);
    const int32_t second =
        10 * static_cast<int32_t>(input.locBytes[1] >> 4) +
        static_cast<int32_t>(input.locBytes[1] & 0x0Fu);
    const int32_t frame =
        10 * static_cast<int32_t>(input.locBytes[2] >> 4) +
        static_cast<int32_t>(input.locBytes[2] & 0x0Fu);
    out.sector = 75 * (60 * minute + second) + frame - 150;
    out.produced = true;
    return out;
}

ReadPumpResult80038DE8 BuildReadPumpResult80038DE8(
    const ReadPumpInput80038DE8& input) {
    ReadPumpResult80038DE8 out{};
    if (!input.clearSyncCallback.produced ||
        !input.clearReadyCallback.produced ||
        !input.status.produced || input.status.incomplete ||
        !input.clockKnown) {
        out.incomplete = true;
        return out;
    }
    out.clearedSyncCallback80036510 = true;
    out.clearedReadyCallback80036528 = true;

    if ((input.status.status & 0x10u) != 0u) {
        if (!input.shellOpenCommandResultKnown ||
            !input.shellOpenCommandResult.produced ||
            input.shellOpenCommandResult.incomplete) {
            out.incomplete = true;
            return out;
        }
        out.shellOpen = true;
        out.shellOpenCommandIssued80036678_1 = true;
        out.psxReturn = -1;
        out.remaining80057424 = -1;
        out.startClock8005742C = input.clockNow;
        out.produced = true;
        return out;
    }

    if (input.retry) {
        out.retryIssued = true;
        if (!input.retryPauseResultKnown ||
            !input.retryPauseResult.produced ||
            input.retryPauseResult.incomplete) {
            out.incomplete = true;
            return out;
        }
        if (!input.retrySetlocResult.produced ||
            input.retrySetlocResult.incomplete) {
            out.incomplete = true;
            return out;
        }
        if (input.retrySetlocResult.psxReturn == 0) {
            out.psxReturn = -1;
            out.remaining80057424 = -1;
            out.produced = true;
            return out;
        }
    }

    if (!input.preReadSetupKnown) {
        out.incomplete = true;
        return out;
    }
    out.preReadSetupApplied80036430 = true;

    if (!input.modeKnown) {
        out.incomplete = true;
        return out;
    }

    out.modeCommandIssued =
        (static_cast<uint8_t>(input.modeWord8005741C) != 0u) ||
        input.retry;
    out.modeCommandArg0 = static_cast<uint8_t>(input.modeWord8005741C);
    if (out.modeCommandIssued) {
        if (!input.modeCommandResult.produced ||
            input.modeCommandResult.incomplete) {
            out.incomplete = true;
            return out;
        }
        if (input.modeCommandResult.psxReturn == 0) {
            out.psxReturn = -1;
            out.remaining80057424 = -1;
            out.produced = true;
            return out;
        }
    }

    if (!input.locSectorKnown ||
        !input.activeDstKnown ||
        !input.activeSectorCountKnown ||
        !input.startReadResultKnown ||
        !input.startReadResult.produced ||
        input.startReadResult.incomplete) {
        out.incomplete = true;
        return out;
    }

    out.callbackInstalled80038BC4 = true;
    out.globalsKnown = true;
    out.psxReturn = input.activeSectorCount80057410;
    out.remaining80057424 = input.activeSectorCount80057410;
    out.startClock8005742C = input.clockNow;
    out.lastPumpClock80057428 = input.clockNow;
    out.activeDst80057418 = input.activeDst80057414;
    out.activeSectorCount80057410 = input.activeSectorCount80057410;
    out.expectedSector80057430 = input.locSector80057430;
    out.produced = true;
    return out;
}

ReadStartSetupResult80038FC0 BuildReadStartSetupResult80038FC0(
    const ReadStartSetupInput80038FC0& input) {
    ReadStartSetupResult80038FC0 out{};
    if (!input.sectorCountKnown ||
        !input.dstKnown ||
        !input.modeKnown ||
        !input.savedSyncCallback.produced ||
        !input.savedReadyCallback.produced ||
        !input.status.produced ||
        input.status.incomplete ||
        !input.clockKnown) {
        out.incomplete = true;
        return out;
    }

    switch (input.mode & 0x30u) {
    case 0x00u:
        out.sectorSize80057420 = 0x200;
        break;
    case 0x20u:
        out.sectorSize80057420 = 0x249;
        break;
    default:
        out.sectorSize80057420 = 0x246;
        break;
    }

    out.modeWord8005741C = input.mode | 0x20u;
    out.dst80057414 = input.dst;
    out.sectorCount80057410 = input.sectorCount;
    out.savedSyncCallback80057434 =
        input.savedSyncCallback.oldCallback;
    out.savedReadyCallback80057438 =
        input.savedReadyCallback.oldCallback;
    out.startClock8005742C = input.clockNow;

    out.preSeekRequested = (input.status.status & 0xE0u) != 0u;
    if (out.preSeekRequested) {
        if (!input.preSeekResult800367A4.produced ||
            input.preSeekResult800367A4.incomplete) {
            out.incomplete = true;
            return out;
        }
        out.preSeekApplied = true;
    }

    out.produced = true;
    return out;
}

ReadCallbackResult80038BC4 BuildReadCallbackResult80038BC4(
    const ReadCallbackInput80038BC4& input) {
    ReadCallbackResult80038BC4 out{};
    if (!input.callbackEventKnown ||
        !input.callbackResultPtrKnown ||
        !input.tickNowKnown ||
        !input.sectorSizeKnown ||
        !input.remainingKnown ||
        !input.activeDstKnown ||
        !input.expectedSectorKnown ||
        !input.startClockKnown) {
        out.incomplete = true;
        return out;
    }

    int32_t remaining = input.remaining80057424;
    int32_t dst = input.activeDst80057418;
    int32_t expectedSector = input.expectedSector80057430;

    if (input.callbackEvent == 1u) {
        if (remaining > 0) {
            if (input.sectorSize80057420 == 0x200) {
                if (!input.resultBytesKnown) {
                    out.incomplete = true;
                    return out;
                }
                LocSectorInput80036A78 loc{};
                loc.locBytesKnown = true;
                loc.locBytes[0] = input.resultBytes[0];
                loc.locBytes[1] = input.resultBytes[1];
                loc.locBytes[2] = input.resultBytes[2];
                const LocSectorResult80036A78 lba =
                    BuildLocSectorResult80036A78(loc);
                if (!lba.produced || lba.incomplete) {
                    out.incomplete = true;
                    return out;
                }
                out.msfCheckPerformed = true;
                out.msfLbaKnown = true;
                out.msfLba = lba.sector;
                if (lba.sector != expectedSector) {
                    out.sectorMismatch = true;
                    remaining = -1;
                }
            }

            if (remaining > 0) {
                out.copiedSectorPayload = true;
                dst += input.sectorSize80057420 << 2;
                --remaining;
                ++expectedSector;
            }
        }
    } else {
        remaining = -1;
    }

    out.retryRequested80038DE8 = remaining < 0;
    if (out.retryRequested80038DE8) {
        if (!input.retryPumpResult.produced ||
            input.retryPumpResult.incomplete ||
            !input.retryPumpResult.retryIssued ||
            !input.retryPumpResult.globalsKnown) {
            out.incomplete = true;
            return out;
        }
        out.retryPumpApplied = true;
        remaining = input.retryPumpResult.remaining80057424;
        dst = input.retryPumpResult.activeDst80057418;
        expectedSector = input.retryPumpResult.expectedSector80057430;
    }

    out.timeout1200 =
        input.tickNow > input.startClock8005742C +
                            PrStage1LoaderCdHal::
                                kReadSync800390C8TimeoutVblanks;
    if (out.timeout1200) {
        remaining = -1;
    }

    const bool terminal = remaining == 0 || out.timeout1200;
    if (terminal) {
        if (!input.savedSyncCallbackKnown ||
            !input.savedReadyCallbackKnown ||
            !input.pauseCommandResult.produced ||
            input.pauseCommandResult.incomplete) {
            out.incomplete = true;
            return out;
        }
        out.restoredCallbacks = true;
        out.restore36510Arg = input.savedSyncCallback80057434;
        out.restore36528Arg = input.savedReadyCallback80057438;
        out.pauseCommandIssued80036540_9 = true;
        if (input.userCallbackKnown && input.userCallback8005740C != 0u) {
            out.userCallbackInvoked = true;
            out.userCallbackArg0 = remaining == 0 ? 2u : 5u;
            out.userCallbackArg1 = input.callbackResultPtr;
        }
    }

    out.remainingAfter = remaining;
    out.dstAfter = dst;
    out.expectedSectorAfter = expectedSector;
    out.produced = true;
    return out;
}

Result BuildSeekSyncSeam800367A4(const SeekSyncInput800367A4& input) {
    Result out{};
    if (input.halFactsKnown) {
        if (!IsSeekSyncHalRequest8001A89C367A4(input.request)) {
            out.incomplete = true;
            return out;
        }
        out.cd = MakeBaseCdSeam(input.request, input.overlayTransferAttempt);
        const PrStage1LoaderCdHal::SeekSyncHalFacts800367A4& facts =
            input.halFacts;
        if (!facts.known ||
            !facts.commandAcceptedKnown ||
            !facts.seekCompletedKnown ||
            !facts.seekFailedKnown ||
            !facts.timeoutKnown) {
            out.incomplete = true;
            return out;
        }

        out.cd.feedback.psxReturn =
            facts.commandAccepted && facts.seekCompleted &&
                    !facts.seekFailed && !facts.timeout
                ? 1
                : 0;
        out.cd.feedback.success = out.cd.feedback.psxReturn != 0;
        out.produced = true;
        return out;
    }

    if (!IsSeekSyncRequest8001A89C367A4(input.request)) {
        out.incomplete = true;
        return out;
    }

    out.cd = MakeBaseCdSeam(input.request, input.overlayTransferAttempt);

    if (!input.commandResult.produced ||
        input.commandResult.incomplete ||
        (input.commandResult.setlocArgsKnown &&
         !SameBcdMsf80036974(
             input.request.seekMsfTarget,
             PrStage1LoaderCdHal::BcdMsf80036974{
                 input.commandResult.setlocArgs[0],
                 input.commandResult.setlocArgs[1],
                 input.commandResult.setlocArgs[2]}))) {
        out.incomplete = true;
        return out;
    }

    if (input.commandResult.psxReturn != 0) {
        out.cd.feedback.psxReturn = 0;
        out.cd.feedback.success = false;
        out.produced = true;
        return out;
    }

    if (!input.cdSyncResultKnown) {
        out.incomplete = true;
        return out;
    }

    out.cd.feedback.psxReturn =
        input.cdSyncPsxReturn == 2 ? 1 : 0;
    out.cd.feedback.success = out.cd.feedback.psxReturn != 0;
    out.produced = true;
    return out;
}

Result BuildReadStartSeam80038FC0(const ReadStartInput80038FC0& input) {
    Result out{};
    if (!IsReadStartRequest8001A81838FC0(input.request)) {
        out.incomplete = true;
        return out;
    }
    if (input.halFactsKnown) {
        const PrStage1LoaderCdHal::ReadStartHalFacts80038FC0& facts =
            input.halFacts;
        if (!facts.known ||
            !facts.commandAcceptedKnown ||
            !facts.readStartedKnown ||
            !facts.readFailedKnown ||
            !facts.timeoutKnown) {
            out.incomplete = true;
            return out;
        }

        out.cd = MakeBaseCdSeam(input.request, input.overlayTransferAttempt);
        out.cd.feedback.psxReturn =
            facts.commandAccepted && facts.readStarted &&
                    !facts.readFailed && !facts.timeout
                ? 1
                : 0;
        out.cd.feedback.success = out.cd.feedback.psxReturn != 0;
        if (facts.dstPtrKnown) {
            out.cd.dstPtrKnown = true;
            out.cd.dstPtr = facts.dstPtr;
        } else if (input.request.readStartDstPtrKnown) {
            out.cd.dstPtrKnown = true;
            out.cd.dstPtr = input.request.readStartDstPtr;
        }
        if (facts.sectorCountKnown) {
            out.cd.sectorCountKnown = true;
            out.cd.sectorCount = facts.sectorCount;
        } else if (input.request.readStartSectorCountKnown) {
            out.cd.sectorCountKnown = true;
            out.cd.sectorCount = input.request.readStartSectorCount;
        }
        out.produced = true;
        return out;
    }

    if (
        !input.setupResult.produced ||
        input.setupResult.incomplete ||
        !ReadStartSetupMatchesRequest80038FC0(input.request,
                                              input.setupResult) ||
        !input.readPumpResult.produced ||
        input.readPumpResult.incomplete ||
        !ReadPumpGlobalsMatchSetup80038DE8(input.readPumpResult,
                                           input.setupResult)) {
        out.incomplete = true;
        return out;
    }
    if (!input.overlayTransferAttempt.known ||
        !input.overlayTransferAttempt.attemptIndexKnown ||
        input.overlayTransferAttempt.sourceFunction !=
            PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper ||
        input.overlayTransferAttempt.transferFunction !=
            PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer) {
        out.incomplete = true;
        return out;
    }
    if (input.overlayTransferAttempt.dstKnown &&
        input.request.readStartDstPtrKnown &&
        input.overlayTransferAttempt.dst != input.request.readStartDstPtr) {
        out.incomplete = true;
        return out;
    }
    if (input.overlayTransferAttempt.sectorCountKnown &&
        input.request.readStartSectorCountKnown &&
        input.overlayTransferAttempt.sectorCount !=
            static_cast<uint32_t>(input.request.readStartSectorCount)) {
        out.incomplete = true;
        return out;
    }

    out.cd = MakeBaseCdSeam(input.request, input.overlayTransferAttempt);
    out.cd.feedback.psxReturn =
        input.readPumpResult.psxReturn > 0 ? 1 : 0;
    out.cd.feedback.success = out.cd.feedback.psxReturn != 0;
    if (input.request.readStartDstPtrKnown) {
        out.cd.dstPtrKnown = true;
        out.cd.dstPtr = input.request.readStartDstPtr;
    }
    if (input.request.readStartSectorCountKnown) {
        out.cd.sectorCountKnown = true;
        out.cd.sectorCount = input.request.readStartSectorCount;
    }
    if (input.payloadBytesKnown) {
        out.cd.livePayloadBytesKnown = true;
        out.cd.livePayloadData = input.payloadData;
        out.cd.livePayloadSize = input.payloadSize;
    }
    out.produced = true;
    return out;
}

Result BuildReadSyncSeam800390C8(const ReadSyncInput800390C8& input) {
    Result out{};
    if (!IsRequestKind(input.request,
                       PrStage1LoaderCdHal::ActionKind::ReadSync800390C8) ||
        input.request.callerFunction != PrStage1LoaderCdHal::kFn8001A818 ||
        input.request.lowerFunction != PrStage1LoaderCdHal::kFn800390C8 ||
        input.request.finalFunction != PrStage1LoaderCdHal::kFn800364F0 ||
        !input.request.readSyncRequestKnown ||
        input.request.readSyncFunction != PrStage1LoaderCdHal::kFn800390C8 ||
        input.request.readSyncArg0 !=
            PrStage1LoaderCdHal::kRead8001A818SyncArg0 ||
        input.request.readSyncArg1 !=
            PrStage1LoaderCdHal::kRead8001A818SyncArg1 ||
        ((input.request.readSyncArg0 != 0) != input.wait) ||
        !input.clockKnown ||
        !input.startClockKnown ||
        !input.remainingKnown) {
        out.incomplete = true;
        return out;
    }

    out.cd = MakeBaseCdSeam(input.request, input.overlayTransferAttempt);
    int32_t psxReturn = -1;
    const bool timedOut =
        input.clockNow > input.startClock8005742C +
                             PrStage1LoaderCdHal::
                                 kReadSync800390C8TimeoutVblanks;
    if (!timedOut) {
        bool needsRetryPump = input.remaining80057424 < 0;
        if (!needsRetryPump) {
            if (!input.lastPumpClockKnown) {
                out.incomplete = true;
                return out;
            }
            needsRetryPump = input.clockNow > input.lastPumpClock80057428 + 60;
        }

        psxReturn = input.remaining80057424;
        if (needsRetryPump) {
            if (!input.retryPumpResult.produced ||
                input.retryPumpResult.incomplete ||
                !input.retryPumpResult.retryIssued ||
                !input.retryPumpResult.globalsKnown ||
                input.retryPumpResult.psxReturn !=
                    input.retryPumpResult.activeSectorCount80057410 ||
                input.retryPumpResult.remaining80057424 !=
                    input.retryPumpResult.activeSectorCount80057410) {
                out.incomplete = true;
                return out;
            }
            psxReturn = input.retryPumpResult.activeSectorCount80057410;
        }

        if (input.wait && psxReturn > 0) {
            out.incomplete = true;
            return out;
        }
    }

    if (!input.finalReadyResult.produced ||
        input.finalReadyResult.incomplete ||
        !input.finalReadyResult.a0WaitModeKnown ||
        input.finalReadyResult.a0WaitMode != 1 ||
        !input.finalReadyResult.a1OutputBufferPtrKnown ||
        input.finalReadyResult.a1OutputBufferPtrNonNull !=
            (input.request.readSyncArg1 != 0)) {
        out.incomplete = true;
        return out;
    }

    out.cd.feedback.psxReturn = psxReturn;
    out.cd.feedback.success = out.cd.feedback.psxReturn == 0;
    if (input.overlayTransferAttempt.dstKnown) {
        out.cd.dstPtrKnown = true;
        out.cd.dstPtr = input.overlayTransferAttempt.dst;
    }
    if (input.overlayTransferAttempt.sectorCountKnown) {
        out.cd.sectorCountKnown = true;
        out.cd.sectorCount =
            static_cast<int32_t>(input.overlayTransferAttempt.sectorCount);
    }
    if (input.payloadBytesKnown) {
        out.cd.livePayloadBytesKnown = true;
        out.cd.livePayloadData = input.payloadData;
        out.cd.livePayloadSize = input.payloadSize;
    }
    out.produced = true;
    return out;
}

Result BuildLowerCdSeamFromFacts(const LowerCdProducerFacts& input) {
    switch (input.request.actionKind) {
    case PrStage1LoaderCdHal::ActionKind::SeekSync800367A4: {
        if (input.seekSyncHalFacts800367A4Known &&
            (input.cdSyncLoopFacts80037070Known ||
             input.cdSyncResult80037070Known ||
             input.commandResult800375BC.produced)) {
            Result out{};
            out.incomplete = true;
            return out;
        }
        if (input.cdSyncLoopFacts80037070Known &&
            input.cdSyncResult80037070Known) {
            Result out{};
            out.incomplete = true;
            return out;
        }

        bool cdSyncResultKnown = input.cdSyncResult80037070Known;
        int32_t cdSyncPsxReturn = input.cdSyncPsxReturn80037070;
        if (input.cdSyncLoopFacts80037070Known) {
            if (!input.cdSyncLoopFacts80037070.a0WaitModeKnown ||
                input.cdSyncLoopFacts80037070.a0WaitMode != 0) {
                Result out{};
                out.incomplete = true;
                return out;
            }
            const CdSyncLoopFactsResult80037070 loop =
                BuildCdSyncLoopFacts80037070(
                    input.cdSyncLoopFacts80037070);
            if (!loop.produced || loop.incomplete || !loop.coreFactsKnown) {
                Result out{};
                out.incomplete = true;
                return out;
            }
            const CdSyncLowerFeedbackResult80037070 sync =
                BuildCdSyncLowerFeedback80037070FromCoreFacts(
                    loop.coreFacts);
            if (!sync.produced ||
                sync.incomplete ||
                !sync.feedback.known ||
                !sync.feedback.syncResultKnown) {
                Result out{};
                out.incomplete = true;
                return out;
            }
            cdSyncResultKnown = true;
            cdSyncPsxReturn = sync.feedback.syncResult;
        }

        SeekSyncInput800367A4 seam{};
        seam.request = input.request;
        seam.overlayTransferAttempt = input.overlayTransferAttempt;
        seam.commandResult = input.commandResult800375BC;
        seam.halFactsKnown = input.seekSyncHalFacts800367A4Known;
        seam.halFacts = input.seekSyncHalFacts800367A4;
        seam.cdSyncResultKnown = cdSyncResultKnown;
        seam.cdSyncPsxReturn = cdSyncPsxReturn;
        return BuildSeekSyncSeam800367A4(seam);
    }
    case PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0: {
        if (input.readStartHalFacts80038FC0Known &&
            (input.readStartSetup80038FC0.produced ||
             input.readPump80038DE8.produced ||
             input.payloadBytesKnown)) {
            Result out{};
            out.incomplete = true;
            return out;
        }
        ReadStartInput80038FC0 seam{};
        seam.request = input.request;
        seam.overlayTransferAttempt = input.overlayTransferAttempt;
        seam.halFactsKnown = input.readStartHalFacts80038FC0Known;
        seam.halFacts = input.readStartHalFacts80038FC0;
        seam.setupResult = input.readStartSetup80038FC0;
        seam.readPumpResult = input.readPump80038DE8;
        seam.payloadData = input.payloadData;
        seam.payloadSize = input.payloadSize;
        seam.payloadBytesKnown = input.payloadBytesKnown;
        return BuildReadStartSeam80038FC0(seam);
    }
    case PrStage1LoaderCdHal::ActionKind::ReadSync800390C8: {
        if (!input.finalReadyInput800372F0Known) {
            Result out{};
            out.incomplete = true;
            return out;
        }

        ReadSyncInput800390C8 seam{};
        seam.request = input.request;
        seam.overlayTransferAttempt = input.overlayTransferAttempt;
        seam.wait = input.readSyncWait;
        seam.clockKnown = input.clockKnown;
        seam.clockNow = input.clockNow;
        seam.startClockKnown = input.startClockKnown;
        seam.startClock8005742C = input.startClock8005742C;
        seam.lastPumpClockKnown = input.lastPumpClockKnown;
        seam.lastPumpClock80057428 = input.lastPumpClock80057428;
        seam.remainingKnown = input.remainingKnown;
        seam.remaining80057424 = input.remaining80057424;
        seam.retryPumpResult = input.retryPump80038DE8;
        seam.finalReadyResult =
            BuildReadyWaitResult800372F0(
                input.finalReadyInput800372F0);
        seam.payloadData = input.payloadData;
        seam.payloadSize = input.payloadSize;
        seam.payloadBytesKnown = input.payloadBytesKnown;
        return BuildReadSyncSeam800390C8(seam);
    }
    default:
        break;
    }

    Result out{};
    out.incomplete = true;
    return out;
}

} // namespace PrStage1LowerCdProducerDirect
