#include "pr_ss0_scene0_int_spu_direct.h"

#include "pr_stage1_loader_spu_hal.h"

#include <algorithm>
#include <cstddef>
#include <limits>

namespace PrSS0Scene0IntSpuDirect {
namespace {

constexpr uint32_t kVhHeaderBytes = 32u;
constexpr uint32_t kProgramAttributeBytes = 16u;
constexpr uint32_t kMaxPrograms = 128u;
constexpr uint32_t kTonesPerProgram = 16u;
constexpr uint32_t kToneAttributeBytes = 32u;
constexpr uint32_t kVagTableBytes = 256u * 2u;
constexpr uint32_t kFnSsInit8002AC20 = 0x8002AC20u;
constexpr uint32_t kFnSsSetTableSize8002DA78 = 0x8002DA78u;
constexpr uint32_t kFnSsStart8002B130 = 0x8002B130u;
constexpr uint32_t kFnSsSetTickMode8002A6AC = 0x8002A6ACu;
constexpr uint32_t kFnSsSetMVol8002AA90 = 0x8002AA90u;
constexpr uint32_t kFnSsSetSerialAttr8002AB24 = 0x8002AB24u;
constexpr uint32_t kSsMidiChannelProgramBaseOffset8002BFDC = 0x2Cu;

int16_t WrapSsMidiI16(int32_t value) {
    return static_cast<int16_t>(static_cast<uint16_t>(value));
}

uint16_t NormalizeSsMidiEnvelopeVolume(int32_t value) {
    const uint16_t wrapped = static_cast<uint16_t>(value);
    return wrapped < 0x80u ? wrapped : 0x7Fu;
}

bool IsValidSsMidiEnvelopeObservation(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiEnvelopeObservation8002B474& observation) {
    return IsExactSsMidiParserSurface8002BA9C(surface) &&
           observation.known && observation.sequenceIndex >= 0 &&
           observation.sequenceIndex <= 0xFF && observation.trackIndex >= 0 &&
           observation.trackIndex <= 0xFF;
}

void SeedSsMidiEnvelopeDispatch(
    const SsMidiEnvelopeObservation8002B474& observation,
    uint32_t lowerFunction,
    SsMidiEnvelopeDispatch8002B474& out) {
    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.wordTrackOffset40After = observation.wordTrackOffset40;
    out.dwordTrackOffset98After = observation.dwordTrackOffset98 - 1u;
    out.dwordTrackOffset90FlagsAfter = observation.dwordTrackOffset90Flags;
    out.wordTrackOffset74After = observation.wordTrackOffset74;
    out.wordTrackOffset76After = observation.wordTrackOffset76;
    out.wordTrackOffset78After = observation.wordTrackOffset78;
    out.wordTrackOffset7AAfter = observation.wordTrackOffset7A;
    out.packedSequenceTrack = static_cast<uint16_t>(
        static_cast<uint16_t>(observation.sequenceIndex) |
        (static_cast<uint16_t>(observation.trackIndex) << 8u));
    out.helperReadFunction = 0x80033928u;
    out.helperWriteFunction = 0x800337B4u;
    out.helperReadObserved = false;
    out.helperReadCallCount = 0u;
    out.helperWriteObserved = false;
    out.helperWriteCallCount = 0u;
    out.helperWriteLeftRequested = 0;
    out.helperWriteRightRequested = 0;
    out.helperWriteLeftApplied = 0u;
    out.helperWriteRightApplied = 0u;
    out.flag10Cleared = false;
    out.result = out.packedSequenceTrack;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    (void)lowerFunction;
}

void SeedSsMidiEnvelopeDispatch(
    const SsMidiEnvelopeObservation8002B474& observation,
    SsMidiEnvelopeDispatch8002B750& out) {
    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.wordTrackOffset40After = observation.wordTrackOffset40;
    out.dwordTrackOffset98After = observation.dwordTrackOffset98 - 1u;
    out.dwordTrackOffset90FlagsAfter = observation.dwordTrackOffset90Flags;
    out.wordTrackOffset74After = observation.wordTrackOffset74;
    out.wordTrackOffset76After = observation.wordTrackOffset76;
    out.wordTrackOffset78After = observation.wordTrackOffset78;
    out.wordTrackOffset7AAfter = observation.wordTrackOffset7A;
    out.packedSequenceTrack = static_cast<uint16_t>(
        static_cast<uint16_t>(observation.sequenceIndex) |
        (static_cast<uint16_t>(observation.trackIndex) << 8u));
    out.helperReadFunction = 0x80033928u;
    out.helperWriteFunction = 0x800337B4u;
    out.helperReadObserved = false;
    out.helperReadCallCount = 0u;
    out.helperWriteObserved = false;
    out.helperWriteCallCount = 0u;
    out.helperWriteLeftRequested = 0;
    out.helperWriteRightRequested = 0;
    out.helperWriteLeftApplied = 0u;
    out.helperWriteRightApplied = 0u;
    out.flag20Cleared = false;
    out.result = out.packedSequenceTrack;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
}

void ObserveSsMidiEnvelopeRead(
    uint16_t sourceLeft,
    uint16_t sourceRight,
    SsMidiEnvelopeDispatch8002B474& out) {
    out.helperReadObserved = true;
    ++out.helperReadCallCount;
    out.wordTrackOffset78After = sourceLeft;
    out.wordTrackOffset7AAfter = sourceRight;
}

void ObserveSsMidiEnvelopeRead(
    uint16_t sourceLeft,
    uint16_t sourceRight,
    SsMidiEnvelopeDispatch8002B750& out) {
    out.helperReadObserved = true;
    ++out.helperReadCallCount;
    out.wordTrackOffset78After = sourceLeft;
    out.wordTrackOffset7AAfter = sourceRight;
}

void ObserveSsMidiEnvelopeWrite(
    int32_t requestedLeft,
    int32_t requestedRight,
    SsMidiEnvelopeDispatch8002B474& out) {
    out.helperWriteObserved = true;
    ++out.helperWriteCallCount;
    out.helperWriteLeftRequested = WrapSsMidiI16(requestedLeft);
    out.helperWriteRightRequested = WrapSsMidiI16(requestedRight);
    out.helperWriteLeftApplied = NormalizeSsMidiEnvelopeVolume(requestedLeft);
    out.helperWriteRightApplied = NormalizeSsMidiEnvelopeVolume(requestedRight);
    out.wordTrackOffset74After = out.helperWriteLeftApplied;
    out.wordTrackOffset76After = out.helperWriteRightApplied;
}

void ObserveSsMidiEnvelopeWrite(
    int32_t requestedLeft,
    int32_t requestedRight,
    SsMidiEnvelopeDispatch8002B750& out) {
    out.helperWriteObserved = true;
    ++out.helperWriteCallCount;
    out.helperWriteLeftRequested = WrapSsMidiI16(requestedLeft);
    out.helperWriteRightRequested = WrapSsMidiI16(requestedRight);
    out.helperWriteLeftApplied = NormalizeSsMidiEnvelopeVolume(requestedLeft);
    out.helperWriteRightApplied = NormalizeSsMidiEnvelopeVolume(requestedRight);
    out.wordTrackOffset74After = out.helperWriteLeftApplied;
    out.wordTrackOffset76After = out.helperWriteRightApplied;
}

void FinishSsMidiEnvelopeRead(
    SsMidiEnvelopeDispatch8002B474& out) {
    ObserveSsMidiEnvelopeRead(
        out.wordTrackOffset74After,
        out.wordTrackOffset76After,
        out);
}

void FinishSsMidiEnvelopeRead(
    SsMidiEnvelopeDispatch8002B750& out) {
    ObserveSsMidiEnvelopeRead(
        out.wordTrackOffset74After,
        out.wordTrackOffset76After,
        out);
}

bool IsArchiveRange8001A8F0(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    uint64_t offset,
    uint64_t size) {
    return offset <= intLoad.archiveBytes.size() &&
           size <= intLoad.archiveBytes.size() - offset;
}

const PrSS0Scene0IntLoadDirect::BlockMetadata8001A8F0* FindVabBlock(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad) {
    const PrSS0Scene0IntLoadDirect::BlockMetadata8001A8F0* found = nullptr;
    for (const auto& block : intLoad.blocks) {
        if (block.type !=
            PrSS0Scene0IntLoadDirect::BlockType8001A8F0::Vab) {
            continue;
        }
        if (found != nullptr) {
            return nullptr;
        }
        found = &block;
    }
    return found;
}

const PrSS0Scene0IntLoadDirect::EntryMetadata8001A8F0* FindVabEntry(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    uint32_t blockIndex,
    uint32_t entryIndex) {
    for (const auto& entry : intLoad.entries) {
        if (entry.blockIndex == blockIndex &&
            entry.entryIndex == entryIndex &&
            entry.type ==
                PrSS0Scene0IntLoadDirect::BlockType8001A8F0::Vab) {
            return &entry;
        }
    }
    return nullptr;
}

uint16_t ReadU16Le(const std::vector<uint8_t>& bytes, std::size_t offset) {
    return static_cast<uint16_t>(
        static_cast<uint16_t>(bytes[offset]) |
        static_cast<uint16_t>(bytes[offset + 1u]) << 8u);
}

uint64_t HashBytes(const std::vector<uint8_t>& bytes) {
    uint64_t hash = 14695981039346656037ull;
    for (uint8_t value : bytes) {
        hash ^= value;
        hash *= 1099511628211ull;
    }
    return hash;
}

bool EqualActionRequest8001A8F0(
    const VabActionRequest8001A8F0& left,
    const VabActionRequest8001A8F0& right) {
    return left.order == right.order &&
           left.wrapperFunction == right.wrapperFunction &&
           left.lowerFunction == right.lowerFunction &&
           left.pointerArg == right.pointerArg &&
           left.scalarArg == right.scalarArg &&
           left.lowerCallConditional == right.lowerCallConditional &&
           left.lowerResultAuthority == right.lowerResultAuthority;
}

bool EqualPadStartComCall80026E4C(
    const PadStartComCall80026E4C& left,
    const PadStartComCall80026E4C& right) {
    return left.order == right.order &&
           left.lowerFunction == right.lowerFunction &&
           left.args == right.args && left.argCount == right.argCount &&
           left.lowerSideEffectCommitted == right.lowerSideEffectCommitted;
}

bool EqualSsSequenceCallbackSurface8002AC20(
    const SsSequenceCallbackSurface8002AC20& left,
    const SsSequenceCallbackSurface8002AC20& right) {
    return left.known == right.known &&
           left.completeWithinLimits == right.completeWithinLimits &&
           left.sourceFunction == right.sourceFunction &&
           left.initializerFunction == right.initializerFunction &&
           left.consumerFunction == right.consumerFunction &&
           left.consumerJalr == right.consumerJalr &&
           left.callbackTableBase == right.callbackTableBase &&
           left.rowCount == right.rowCount &&
           left.columnCount == right.columnCount &&
           left.callbackTargets == right.callbackTargets &&
           left.callbackTableZeroInitialized ==
               right.callbackTableZeroInitialized &&
           left.dword80095C4CTickRate ==
               right.dword80095C4CTickRate &&
           left.dword800928CCActiveSequenceMask ==
               right.dword800928CCActiveSequenceMask &&
           left.dword800917A4ReentryGuard ==
               right.dword800917A4ReentryGuard &&
           left.psxMemoryAuthority == right.psxMemoryAuthority &&
           left.interruptCallbackAuthority ==
               right.interruptCallbackAuthority &&
           left.dynamicReplayAuthority == right.dynamicReplayAuthority &&
           left.oldWinS0Authority == right.oldWinS0Authority &&
           left.stage2PlusAuthority == right.stage2PlusAuthority;
}

bool EqualSpuTransferCompletionSurface8002AC20(
    const SpuTransferCompletionSurface8002AC20& left,
    const SpuTransferCompletionSurface8002AC20& right) {
    return left.known == right.known &&
           left.completeWithinLimits == right.completeWithinLimits &&
           left.sourceFunction == right.sourceFunction &&
           left.lowerInitFunction == right.lowerInitFunction &&
           left.transferInitFunction == right.transferInitFunction &&
           left.installFunction == right.installFunction &&
           left.dmaCallbackWrapperFunction ==
               right.dmaCallbackWrapperFunction &&
           left.dmaChannel == right.dmaChannel &&
           left.interruptHandlerFunction ==
               right.interruptHandlerFunction &&
           left.callbackJalr == right.callbackJalr &&
           left.callbackSlotAddress == right.callbackSlotAddress &&
           left.callbackInitialValue == right.callbackInitialValue &&
           left.callbackArgument == right.callbackArgument &&
           left.fallbackEventClass == right.fallbackEventClass &&
           left.fallbackEventSpec == right.fallbackEventSpec &&
           left.openEventMode == right.openEventMode &&
           left.openEventArgument == right.openEventArgument &&
           left.installOnceGuardAddress ==
               right.installOnceGuardAddress &&
           left.installOnceGuardSetValue ==
               right.installOnceGuardSetValue &&
           left.eventHandleAddress == right.eventHandleAddress &&
           left.spuControlOffset == right.spuControlOffset &&
           left.spuControlClearMask == right.spuControlClearMask &&
           left.spuControlBusyMask == right.spuControlBusyMask &&
           left.busyPollThreshold == right.busyPollThreshold &&
           left.preInterruptDelayLoopCount ==
               right.preInterruptDelayLoopCount &&
           left.preInterruptDelayIterations ==
               right.preInterruptDelayIterations &&
           left.preInterruptDelaySeed == right.preInterruptDelaySeed &&
           left.preInterruptDelayMultiplier ==
               right.preInterruptDelayMultiplier &&
           left.preInterruptDelayGuardAddress ==
               right.preInterruptDelayGuardAddress &&
           left.preInterruptDelayRunsWhenGuardZero ==
               right.preInterruptDelayRunsWhenGuardZero &&
           left.callbackSlotZeroInitialized ==
               right.callbackSlotZeroInitialized &&
           left.fallbackEventOpenedAndEnabled ==
               right.fallbackEventOpenedAndEnabled &&
           left.reverbGuardFunction == right.reverbGuardFunction &&
           left.reverbGuardSavesClearsRestoresSlot ==
               right.reverbGuardSavesClearsRestoresSlot &&
           left.callbackWriterClosure == right.callbackWriterClosure &&
           left.psxMemoryAuthority == right.psxMemoryAuthority &&
           left.dmaCallbackAuthority == right.dmaCallbackAuthority &&
           left.dynamicReplayAuthority == right.dynamicReplayAuthority &&
           left.oldWinS0Authority == right.oldWinS0Authority &&
           left.stage2PlusAuthority == right.stage2PlusAuthority;
}

bool EqualSsTickFlagRoute8002B200(
    const SsTickFlagRoute8002B200& left,
    const SsTickFlagRoute8002B200& right) {
    return left.order == right.order &&
           left.triggerMask == right.triggerMask &&
           left.lowerFunction == right.lowerFunction &&
           left.requiresBitOneBranch == right.requiresBitOneBranch &&
           left.clearsTrackFlags == right.clearsTrackFlags;
}

bool EqualSsTickInterruptSurface8002B130(
    const SsTickInterruptSurface8002B130& left,
    const SsTickInterruptSurface8002B130& right) {
    if (left.known != right.known ||
        left.completeWithinLimits != right.completeWithinLimits ||
        left.sourceFunction != right.sourceFunction ||
        left.startFunction != right.startFunction ||
        left.installFunction != right.installFunction ||
        left.startModeArgument != right.startModeArgument ||
        left.interruptCallbackApi != right.interruptCallbackApi ||
        left.defaultHandlerSlotAddress !=
            right.defaultHandlerSlotAddress ||
        left.defaultHandlerFunction != right.defaultHandlerFunction ||
        left.defaultHandlerStaticInitialized !=
            right.defaultHandlerStaticInitialized ||
        left.priorHandlerSlotAddress != right.priorHandlerSlotAddress ||
        left.priorHandlerInitialValue != right.priorHandlerInitialValue ||
        left.priorHandlerWrapperFunction !=
            right.priorHandlerWrapperFunction ||
        left.priorHandlerJalr != right.priorHandlerJalr ||
        left.defaultHandlerJalrFromPriorWrapper !=
            right.defaultHandlerJalrFromPriorWrapper ||
        left.dividerWrapperFunction != right.dividerWrapperFunction ||
        left.dividerGuardSlotAddress !=
            right.dividerGuardSlotAddress ||
        left.dividerGuardInitialValue !=
            right.dividerGuardInitialValue ||
        left.dividerHandlerJalr != right.dividerHandlerJalr ||
        left.reentryGuardAddress != right.reentryGuardAddress ||
        left.reentryGuardInitialValue !=
            right.reentryGuardInitialValue ||
        left.activeSequenceMaskAddress !=
            right.activeSequenceMaskAddress ||
        left.activeSequenceMaskInitialValue !=
            right.activeSequenceMaskInitialValue ||
        left.sequenceCountAddress != right.sequenceCountAddress ||
        left.trackCountAddress != right.trackCountAddress ||
        left.sequenceStatePointerTableAddress !=
            right.sequenceStatePointerTableAddress ||
        left.trackStateStride != right.trackStateStride ||
        left.trackFlagsOffset != right.trackFlagsOffset ||
        left.preTickFunction != right.preTickFunction ||
        left.defaultHandlerWriterClosure !=
            right.defaultHandlerWriterClosure ||
        left.priorHandlerWriterClosure !=
            right.priorHandlerWriterClosure ||
        left.psxMemoryAuthority != right.psxMemoryAuthority ||
        left.interruptCallbackAuthority !=
            right.interruptCallbackAuthority ||
        left.dynamicReplayAuthority != right.dynamicReplayAuthority ||
        left.oldWinS0Authority != right.oldWinS0Authority ||
        left.stage2PlusAuthority != right.stage2PlusAuthority) {
        return false;
    }
    for (std::size_t index = 0u; index < left.flagRoutes.size();
         ++index) {
        if (!EqualSsTickFlagRoute8002B200(
                left.flagRoutes[index], right.flagRoutes[index])) {
            return false;
        }
    }
    return true;
}

bool EqualSsMidiStatusRoute8002BC40(
    const SsMidiStatusRoute8002BC40& left,
    const SsMidiStatusRoute8002BC40& right) {
    return left.statusClass == right.statusClass &&
           left.kind == right.kind &&
           left.lowerFunction == right.lowerFunction &&
           left.parserDecodesDeltaBeforeLower ==
               right.parserDecodesDeltaBeforeLower &&
           left.lowerOwnsAdditionalBytes ==
               right.lowerOwnsAdditionalBytes;
}

bool EqualSsMidiParserSurface8002BA9C(
    const SsMidiParserSurface8002BA9C& left,
    const SsMidiParserSurface8002BA9C& right) {
    if (left.known != right.known ||
        left.headerAndDeltaCompleteWithinLimits !=
            right.headerAndDeltaCompleteWithinLimits ||
        left.tickEntryFunction != right.tickEntryFunction ||
        left.schedulerFunction != right.schedulerFunction ||
        left.parserFunction != right.parserFunction ||
        left.deltaDecoderFunction != right.deltaDecoderFunction ||
        left.sequenceStatePointerTableAddress !=
            right.sequenceStatePointerTableAddress ||
        left.trackStateStride != right.trackStateStride ||
        left.cursorOffset != right.cursorOffset ||
        left.runningStatusOffset != right.runningStatusOffset ||
        left.channelOffset != right.channelOffset ||
        left.absoluteTimeOffset != right.absoluteTimeOffset ||
        left.deltaOffset != right.deltaOffset ||
        left.crossFileExplicitReferenceClosure !=
            right.crossFileExplicitReferenceClosure ||
        left.psxMemoryAuthority != right.psxMemoryAuthority ||
        left.lowerEventAuthority != right.lowerEventAuthority ||
        left.dynamicReplayAuthority != right.dynamicReplayAuthority ||
        left.oldWinS0Authority != right.oldWinS0Authority ||
        left.stage2PlusAuthority != right.stage2PlusAuthority) {
        return false;
    }
    for (std::size_t index = 0u; index < left.statusRoutes.size();
         ++index) {
        if (!EqualSsMidiStatusRoute8002BC40(
                left.statusRoutes[index], right.statusRoutes[index])) {
            return false;
        }
    }
    return true;
}

bool EqualSpuState(const PrStage1LoaderSpuHal::State& left,
                   const PrStage1LoaderSpuHal::State& right) {
    return left.word_800943A8 == right.word_800943A8 &&
           left.word_800943AA == right.word_800943AA &&
           left.word_800943AC == right.word_800943AC &&
           left.dword_800943B4 == right.dword_800943B4 &&
           left.dword_80094410 == right.dword_80094410;
}

bool EqualTransaction8001A8F0(
    const Transaction8001A8F0& left,
    const Transaction8001A8F0& right) {
    if (left.status != right.status || left.accepted != right.accepted ||
        left.complete != right.complete ||
        left.sourceIntLoadKnown != right.sourceIntLoadKnown ||
        left.sourceLoaderCandidateKnown !=
            right.sourceLoaderCandidateKnown ||
        left.sourceArchiveKind != right.sourceArchiveKind ||
        left.requestBound != right.requestBound ||
        left.originalDiscPayloadAuthority !=
            right.originalDiscPayloadAuthority ||
        left.vhArchiveDataOffset != right.vhArchiveDataOffset ||
        left.vhPsxAddress != right.vhPsxAddress ||
        left.vhName != right.vhName || left.vhBytes != right.vhBytes ||
        left.vhHash != right.vhHash ||
        left.vbArchiveDataOffset != right.vbArchiveDataOffset ||
        left.vbPsxAddress != right.vbPsxAddress ||
        left.vbName != right.vbName || left.vbBytes != right.vbBytes ||
        left.vbHash != right.vbHash ||
        left.decoderProgramCount != right.decoderProgramCount ||
        left.decoderDeclaredToneCount !=
            right.decoderDeclaredToneCount ||
        left.decoderVagCount != right.decoderVagCount ||
        left.decoderEffectiveVagCount !=
            right.decoderEffectiveVagCount ||
        left.decoderToneSlotCount != right.decoderToneSlotCount ||
        left.decoderVagTableOffset != right.decoderVagTableOffset ||
        left.decoderPreflightKnown != right.decoderPreflightKnown ||
        left.decoderPreflightReady != right.decoderPreflightReady ||
        left.actionSequenceKnown != right.actionSequenceKnown ||
        left.padStartComRequired != right.padStartComRequired ||
        left.padStartComSsSequenceCallbackSurfaceKnown !=
            right.padStartComSsSequenceCallbackSurfaceKnown ||
        !EqualSsSequenceCallbackSurface8002AC20(
            left.padStartComSsSequenceCallbackSurface,
            right.padStartComSsSequenceCallbackSurface) ||
        left.padStartComSpuTransferCompletionSurfaceKnown !=
            right.padStartComSpuTransferCompletionSurfaceKnown ||
        !EqualSpuTransferCompletionSurface8002AC20(
            left.padStartComSpuTransferCompletionSurface,
            right.padStartComSpuTransferCompletionSurface) ||
        left.padStartComSsTickInterruptSurfaceKnown !=
            right.padStartComSsTickInterruptSurfaceKnown ||
        !EqualSsTickInterruptSurface8002B130(
            left.padStartComSsTickInterruptSurface,
            right.padStartComSsTickInterruptSurface) ||
        left.padStartComStaticBodyKnown !=
            right.padStartComStaticBodyKnown ||
        left.padStartComGlobalWritesKnown !=
            right.padStartComGlobalWritesKnown ||
        !EqualSpuState(left.padStartComGlobalState,
                       right.padStartComGlobalState) ||
        left.padStartComCuePointerBefore !=
            right.padStartComCuePointerBefore ||
        left.padStartComCuePointerAfter !=
            right.padStartComCuePointerAfter ||
        left.padStartComCuePointerPreserved !=
            right.padStartComCuePointerPreserved ||
        left.padStartComGlobalWritesCommitted !=
            right.padStartComGlobalWritesCommitted ||
        left.padStartComLowerCallsCommitted !=
            right.padStartComLowerCallsCommitted ||
        left.hostVabCandidatePrepared !=
            right.hostVabCandidatePrepared ||
        left.hostVabBankCommitted != right.hostVabBankCommitted ||
        left.psxVabIdAuthority != right.psxVabIdAuthority ||
        left.psxSpuRamAuthority != right.psxSpuRamAuthority ||
        left.padStartComCommitted != right.padStartComCommitted ||
        left.audibleOutputAuthority != right.audibleOutputAuthority ||
        left.replayValueAuthority != right.replayValueAuthority ||
        left.hostFilesystemAuthority != right.hostFilesystemAuthority ||
        left.consumerReadAuthority != right.consumerReadAuthority ||
        left.oldWinS0Authority != right.oldWinS0Authority ||
        left.stage2PlusAuthority != right.stage2PlusAuthority) {
        return false;
    }
    for (std::size_t index = 0u; index < left.actionRequests.size();
         ++index) {
        if (!EqualActionRequest8001A8F0(left.actionRequests[index],
                                        right.actionRequests[index])) {
            return false;
        }
    }
    for (std::size_t index = 0u; index < left.padStartComCalls.size();
         ++index) {
        if (!EqualPadStartComCall80026E4C(
                left.padStartComCalls[index],
                right.padStartComCalls[index])) {
            return false;
        }
    }
    return true;
}

bool IsPadStartComGlobalWritesCommitReady80026E4C(
    const Transaction8001A8F0& transaction) {
    return transaction.padStartComStaticBodyKnown &&
           transaction.padStartComGlobalWritesKnown &&
           transaction.padStartComCuePointerPreserved &&
           !transaction.padStartComGlobalWritesCommitted &&
           !transaction.padStartComLowerCallsCommitted &&
           !transaction.padStartComCommitted;
}

void MarkPadStartComGlobalWritesCommitted80026E4C(
    Transaction8001A8F0& transaction) {
    transaction.padStartComGlobalWritesCommitted = true;
}

}  // namespace

SsSequenceCallbackSurface8002AC20
BuildSsSequenceCallbackSurface8002AC20() {
    SsSequenceCallbackSurface8002AC20 surface{};
    surface.known = true;
    surface.completeWithinLimits = true;
    surface.sourceFunction = kFnSsInit8002AC20;
    surface.initializerFunction = 0x8002ADE0u;
    surface.consumerFunction = 0x8002C808u;
    surface.consumerJalr = 0x8002C8FCu;
    surface.callbackTableBase = 0x80095D88u;
    surface.rowCount = kSsSequenceCallbackRowCount8002ADE0;
    surface.columnCount = kSsSequenceCallbackColumnCount8002ADE0;
    surface.callbackTargets.fill(0u);
    surface.callbackTableZeroInitialized = true;
    surface.dword80095C4CTickRate = 60;
    surface.dword800928CCActiveSequenceMask = 0u;
    surface.dword800917A4ReentryGuard = 0u;
    surface.psxMemoryAuthority = false;
    surface.interruptCallbackAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsSequenceCallbackSurface8002AC20(
    const SsSequenceCallbackSurface8002AC20& surface) {
    return EqualSsSequenceCallbackSurface8002AC20(
        surface, BuildSsSequenceCallbackSurface8002AC20());
}

bool TryResolveSsSequenceCallback8002C808(
    const SsSequenceCallbackSurface8002AC20& surface,
    int16_t sequenceIndex,
    int16_t trackIndex,
    uint32_t& outCallbackTarget) {
    outCallbackTarget = 0u;
    if (!IsExactSsSequenceCallbackSurface8002AC20(surface) ||
        sequenceIndex < 0 ||
        static_cast<uint32_t>(sequenceIndex) >= surface.rowCount ||
        trackIndex < 0 ||
        static_cast<uint32_t>(trackIndex) >= surface.columnCount) {
        return false;
    }
    const std::size_t flatIndex =
        static_cast<std::size_t>(sequenceIndex) * surface.columnCount +
        static_cast<std::size_t>(trackIndex);
    outCallbackTarget = surface.callbackTargets[flatIndex];
    return true;
}

SpuTransferCompletionSurface8002AC20
BuildSpuTransferCompletionSurface8002AC20() {
    SpuTransferCompletionSurface8002AC20 surface{};
    surface.known = true;
    surface.completeWithinLimits = true;
    surface.sourceFunction = kFnSsInit8002AC20;
    surface.lowerInitFunction = 0x8002AC70u;
    surface.transferInitFunction = 0x8002961Cu;
    surface.installFunction = 0x8002AD38u;
    surface.dmaCallbackWrapperFunction = 0x8002ADBCu;
    surface.dmaChannel = 4u;
    surface.interruptHandlerFunction = 0x80029E6Cu;
    surface.callbackJalr = 0x80029FE0u;
    surface.callbackSlotAddress = 0x800555FCu;
    surface.callbackInitialValue = 0u;
    surface.callbackArgument = 0xF0000000u;
    surface.fallbackEventClass = 0xF0000009u;
    surface.fallbackEventSpec = 0x20u;
    surface.openEventMode = 0x2000u;
    surface.openEventArgument = 0u;
    surface.installOnceGuardAddress = 0x80055A80u;
    surface.installOnceGuardSetValue = 1u;
    surface.eventHandleAddress = 0x80055678u;
    surface.spuControlOffset = 0x1AAu;
    surface.spuControlClearMask = 0xFFCFu;
    surface.spuControlBusyMask = 0x0030u;
    surface.busyPollThreshold =
        kSpuTransferBusyPollThreshold80029E6C;
    surface.preInterruptDelayLoopCount = 3u;
    surface.preInterruptDelayIterations = 240u;
    surface.preInterruptDelaySeed = 13u;
    surface.preInterruptDelayMultiplier = 3u;
    surface.preInterruptDelayGuardAddress = 0x80055614u;
    surface.preInterruptDelayRunsWhenGuardZero = true;
    surface.callbackSlotZeroInitialized = true;
    surface.fallbackEventOpenedAndEnabled = true;
    surface.reverbGuardFunction = 0x80030088u;
    surface.reverbGuardSavesClearsRestoresSlot = true;
    surface.callbackWriterClosure = false;
    surface.psxMemoryAuthority = false;
    surface.dmaCallbackAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSpuTransferCompletionSurface8002AC20(
    const SpuTransferCompletionSurface8002AC20& surface) {
    return EqualSpuTransferCompletionSurface8002AC20(
        surface, BuildSpuTransferCompletionSurface8002AC20());
}

bool TryBuildSpuTransferCompletionDispatch80029E6C(
    const SpuTransferCompletionSurface8002AC20& surface,
    uint32_t callbackTarget,
    SpuTransferCompletionDispatch80029E6C& out) {
    out = SpuTransferCompletionDispatch80029E6C{};
    if (!IsExactSpuTransferCompletionSurface8002AC20(surface)) {
        return false;
    }
    out.known = true;
    out.spuControlClearMask = surface.spuControlClearMask;
    out.spuControlBusyMask = surface.spuControlBusyMask;
    out.busyPollThreshold = surface.busyPollThreshold;
    out.callbackTargetAuthority = false;
    out.psxMemoryAuthority = false;
    out.dmaCallbackAuthority = false;
    if (callbackTarget != 0u) {
        out.kind =
            SpuTransferCompletionDispatchKind80029E6C::Callback;
        out.callbackTarget = callbackTarget;
        out.callbackArgument = surface.callbackArgument;
    } else {
        out.kind =
            SpuTransferCompletionDispatchKind80029E6C::DeliverEvent;
        out.eventClass = surface.fallbackEventClass;
        out.eventSpec = surface.fallbackEventSpec;
    }
    return true;
}

SsTickInterruptSurface8002B130
BuildSsTickInterruptSurface8002B130() {
    SsTickInterruptSurface8002B130 surface{};
    surface.known = true;
    surface.completeWithinLimits = true;
    surface.sourceFunction = 0x80026E4Cu;
    surface.startFunction = kFnSsStart8002B130;
    surface.installFunction = 0x8002AEC8u;
    surface.startModeArgument = 1;
    surface.interruptCallbackApi = 0x80035774u;
    surface.defaultHandlerSlotAddress = 0x80055DB4u;
    surface.defaultHandlerFunction = 0x8002B200u;
    surface.defaultHandlerStaticInitialized = true;
    surface.priorHandlerSlotAddress = 0x80055DB8u;
    surface.priorHandlerInitialValue = 0u;
    surface.priorHandlerWrapperFunction = 0x8002B170u;
    surface.priorHandlerJalr = 0x8002B184u;
    surface.defaultHandlerJalrFromPriorWrapper = 0x8002B198u;
    surface.dividerWrapperFunction = 0x8002B1B0u;
    surface.dividerGuardSlotAddress = 0x80055DC0u;
    surface.dividerGuardInitialValue = 0u;
    surface.dividerHandlerJalr = 0x8002B1E8u;
    surface.reentryGuardAddress = 0x800917A4u;
    surface.reentryGuardInitialValue = 0u;
    surface.activeSequenceMaskAddress = 0x800928CCu;
    surface.activeSequenceMaskInitialValue = 0u;
    surface.sequenceCountAddress = 0x80096588u;
    surface.trackCountAddress = 0x80096598u;
    surface.sequenceStatePointerTableAddress = 0x80095D08u;
    surface.trackStateStride = 0xACu;
    surface.trackFlagsOffset = 0x90u;
    surface.preTickFunction = 0x80032B00u;
    surface.flagRoutes[0] = {
        0u, 0x01u, 0x8002BA9Cu, false, false};
    surface.flagRoutes[1] = {
        1u, 0x10u, 0x8002B474u, true, false};
    surface.flagRoutes[2] = {
        2u, 0x20u, 0x8002B750u, true, false};
    surface.flagRoutes[3] = {
        3u, 0x40u, 0x8002DDA4u, true, false};
    surface.flagRoutes[4] = {
        4u, 0x80u, 0x8002DDA4u, true, false};
    surface.flagRoutes[5] = {
        5u, 0x02u, 0x8002B9FCu, false, false};
    surface.flagRoutes[6] = {
        6u, 0x08u, 0x8002BAC8u, false, false};
    surface.flagRoutes[7] = {
        7u, 0x04u, 0x8002DBE4u, false, true};
    surface.defaultHandlerWriterClosure = false;
    surface.priorHandlerWriterClosure = false;
    surface.psxMemoryAuthority = false;
    surface.interruptCallbackAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsTickInterruptSurface8002B130(
    const SsTickInterruptSurface8002B130& surface) {
    return EqualSsTickInterruptSurface8002B130(
        surface, BuildSsTickInterruptSurface8002B130());
}

bool TryBuildSsTickPriorWrapperDispatch8002B170(
    const SsTickInterruptSurface8002B130& surface,
    uint32_t priorHandlerTarget,
    SsTickInterruptDispatch8002B170& out) {
    out = SsTickInterruptDispatch8002B170{};
    if (!IsExactSsTickInterruptSurface8002B130(surface)) {
        return false;
    }
    out.known = true;
    out.callsPriorHandler = priorHandlerTarget != 0u;
    out.priorHandlerTarget = priorHandlerTarget;
    out.callsDefaultHandler = true;
    out.defaultHandlerTarget = surface.defaultHandlerFunction;
    out.dividerGuardKnown = false;
    out.priorHandlerTargetAuthority = false;
    out.defaultHandlerTargetAuthority = false;
    out.interruptCallbackAuthority = false;
    out.lowerCallsCommitted = false;
    return true;
}

bool TryBuildSsTickDividerDispatch8002B1B0(
    const SsTickInterruptSurface8002B130& surface,
    bool dividerGuardSet,
    SsTickInterruptDispatch8002B170& out) {
    out = SsTickInterruptDispatch8002B170{};
    if (!IsExactSsTickInterruptSurface8002B130(surface)) {
        return false;
    }
    out.known = true;
    out.callsPriorHandler = false;
    out.callsDefaultHandler = dividerGuardSet;
    out.defaultHandlerTarget =
        dividerGuardSet ? surface.defaultHandlerFunction : 0u;
    out.dividerGuardKnown = true;
    out.dividerGuardAfter = !dividerGuardSet;
    out.priorHandlerTargetAuthority = false;
    out.defaultHandlerTargetAuthority = false;
    out.interruptCallbackAuthority = false;
    out.lowerCallsCommitted = false;
    return true;
}

bool TryBuildSsTickTrackDispatch8002B200(
    const SsTickInterruptSurface8002B130& surface,
    const SsTickTrackObservation8002B200& observation,
    SsTickTrackDispatch8002B200& out) {
    out = SsTickTrackDispatch8002B200{};
    if (!IsExactSsTickInterruptSurface8002B130(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0) {
        return false;
    }
    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    bool bitOneBranchEntered = false;
    for (std::size_t index = 0u; index < surface.flagRoutes.size();
         ++index) {
        const auto& route = surface.flagRoutes[index];
        if (route.requiresBitOneBranch && !bitOneBranchEntered) {
            continue;
        }
        if ((observation.flagsAtChecks[index] & route.triggerMask) == 0u) {
            continue;
        }
        if (route.order == 0u) {
            bitOneBranchEntered = true;
            out.bitOneBranchEntered = true;
        }
        out.actions.push_back({
            static_cast<uint32_t>(out.actions.size()),
            route.order,
            route.triggerMask,
            route.lowerFunction,
            observation.sequenceIndex,
            observation.trackIndex,
            false,
        });
        if (route.clearsTrackFlags) {
            out.clearsTrackFlags = true;
        }
    }
    out.psxMemoryAuthority = false;
    out.interruptCallbackAuthority = false;
    out.lowerCallsCommitted = false;
    return true;
}

SsMidiParserSurface8002BA9C
BuildSsMidiParserSurface8002BA9C() {
    SsMidiParserSurface8002BA9C surface{};
    surface.known = true;
    surface.headerAndDeltaCompleteWithinLimits = true;
    surface.tickEntryFunction = 0x8002BA9Cu;
    surface.schedulerFunction = 0x8002BB30u;
    surface.parserFunction = 0x8002BC40u;
    surface.deltaDecoderFunction = 0x8002D7D0u;
    surface.sequenceStatePointerTableAddress = 0x80095D08u;
    surface.trackStateStride = 0xACu;
    surface.cursorOffset = 0x04u;
    surface.runningStatusOffset = 0x11u;
    surface.channelOffset = 0x12u;
    surface.absoluteTimeOffset = 0x80u;
    surface.deltaOffset = 0x88u;
    surface.statusRoutes[0] = {
        0x90u,
        SsMidiEventKind8002BC40::NoteOn,
        0x8002BEECu,
        true,
        false,
    };
    surface.statusRoutes[1] = {
        0xB0u,
        SsMidiEventKind8002BC40::ControlChange,
        0x8002C054u,
        false,
        true,
    };
    surface.statusRoutes[2] = {
        0xC0u,
        SsMidiEventKind8002BC40::ProgramChange,
        0x8002BFDCu,
        false,
        true,
    };
    surface.statusRoutes[3] = {
        0xE0u,
        SsMidiEventKind8002BC40::PitchBend,
        0x8002D3D0u,
        false,
        true,
    };
    surface.statusRoutes[4] = {
        0xF0u,
        SsMidiEventKind8002BC40::Meta,
        0x8002D47Cu,
        false,
        true,
    };
    surface.crossFileExplicitReferenceClosure = false;
    surface.psxMemoryAuthority = false;
    surface.lowerEventAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsMidiParserSurface8002BA9C(
    const SsMidiParserSurface8002BA9C& surface) {
    return EqualSsMidiParserSurface8002BA9C(
        surface, BuildSsMidiParserSurface8002BA9C());
}

bool TryDecodeSsMidiDelta8002D7D0(
    const std::vector<uint8_t>& streamBytes,
    std::size_t& cursor,
    uint32_t& absoluteTime,
    uint32_t& outDeltaTicks10) {
    outDeltaTicks10 = 0u;
    if (cursor >= streamBytes.size()) {
        return false;
    }
    std::size_t nextCursor = cursor;
    uint32_t encoded = streamBytes[nextCursor++];
    if (encoded == 0u) {
        cursor = nextCursor;
        return true;
    }
    if ((encoded & 0x80u) != 0u) {
        encoded &= 0x7Fu;
        uint8_t next = 0u;
        do {
            if (nextCursor >= streamBytes.size()) {
                return false;
            }
            next = streamBytes[nextCursor++];
            encoded =
                (encoded << 7u) + static_cast<uint32_t>(next & 0x7Fu);
        } while ((next & 0x80u) != 0u);
    }
    const uint32_t deltaTicks10 = encoded * 10u;
    cursor = nextCursor;
    absoluteTime += deltaTicks10;
    outDeltaTicks10 = deltaTicks10;
    return true;
}

bool TryBuildSsMidiEventDispatch8002BC40(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackObservation8002BC40& observation,
    SsMidiEventDispatch8002BC40& out) {
    out = SsMidiEventDispatch8002BC40{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0 ||
        observation.cursorBefore >= observation.streamBytes.size()) {
        return false;
    }

    std::size_t cursor = observation.cursorBefore;
    const uint8_t eventByte = observation.streamBytes[cursor++];
    const bool explicitStatus = (eventByte & 0x80u) != 0u;
    uint8_t statusClass = observation.runningStatusBefore;
    uint8_t runningStatusAfter = observation.runningStatusBefore;
    uint8_t channelAfter = observation.channelBefore;
    if (explicitStatus) {
        statusClass = eventByte & 0xF0u;
        channelAfter = eventByte & 0x0Fu;
        if (statusClass == 0x90u || statusClass == 0xB0u ||
            statusClass == 0xC0u || statusClass == 0xE0u) {
            runningStatusAfter = statusClass;
        } else if (statusClass == 0xF0u) {
            runningStatusAfter = 0xFFu;
        }
    } else if (statusClass == 0xFFu) {
        statusClass = 0xF0u;
    }

    const SsMidiStatusRoute8002BC40* route = nullptr;
    for (const auto& candidate : surface.statusRoutes) {
        if (candidate.statusClass == statusClass) {
            route = &candidate;
            break;
        }
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.explicitStatusByte = explicitStatus;
    out.eventByte = eventByte;
    out.statusClass = statusClass;
    out.runningStatusAfter = runningStatusAfter;
    out.channelAfter = channelAfter;
    out.cursorAfterParser = cursor;
    out.absoluteTimeAfter = observation.absoluteTimeBefore;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    if (route == nullptr) {
        return true;
    }

    out.kind = route->kind;
    out.lowerFunction = route->lowerFunction;
    out.lowerOwnsAdditionalBytes = route->lowerOwnsAdditionalBytes;
    const auto consumeByte = [&]() -> bool {
        if (cursor >= observation.streamBytes.size() ||
            out.parserDataByteCount >= out.parserDataBytes.size()) {
            return false;
        }
        out.parserDataBytes[out.parserDataByteCount++] =
            observation.streamBytes[cursor++];
        return true;
    };

    if (explicitStatus) {
        if (!consumeByte()) {
            out = SsMidiEventDispatch8002BC40{};
            return false;
        }
    } else {
        out.parserDataBytes[out.parserDataByteCount++] = eventByte;
    }
    if (route->kind == SsMidiEventKind8002BC40::NoteOn &&
        !consumeByte()) {
        out = SsMidiEventDispatch8002BC40{};
        return false;
    }
    if (route->parserDecodesDeltaBeforeLower) {
        uint32_t absoluteTime = observation.absoluteTimeBefore;
        uint32_t deltaTicks10 = 0u;
        if (!TryDecodeSsMidiDelta8002D7D0(
                observation.streamBytes,
                cursor,
                absoluteTime,
                deltaTicks10)) {
            out = SsMidiEventDispatch8002BC40{};
            return false;
        }
        out.deltaDecoded = true;
        out.deltaTicks10 = deltaTicks10;
        out.absoluteTimeAfter = absoluteTime;
    }
    out.cursorAfterParser = cursor;
    return true;
}

bool TryBuildSsMidiSchedulerDispatch8002BB30(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiSchedulerObservation8002BB30& observation,
    SsMidiSchedulerDispatch8002BB30& out) {
    out = SsMidiSchedulerDispatch8002BB30{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.wordTrackOffset6EBefore = observation.wordTrackOffset6E;
    out.wordTrackOffset6EAfter = observation.wordTrackOffset6E;
    out.wordTrackOffset70 = observation.wordTrackOffset70;
    out.dwordTrackOffset88Before = observation.dwordTrackOffset88;
    out.dwordTrackOffset88After = observation.dwordTrackOffset88;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;

    // 8002BB30 treats the dword and halfword fields as signed values in the
    // arithmetic/branches, while the stores retain the original low-width
    // representation.  Keep all intermediate results in 32-bit bit form.
    const int32_t currentTick = observation.wordTrackOffset70;
    int32_t nextDelta = static_cast<int32_t>(
        observation.dwordTrackOffset88);
    const auto AddSignedBits = [](int32_t left, int32_t right) {
        return static_cast<int32_t>(static_cast<uint32_t>(left) +
                                    static_cast<uint32_t>(right));
    };

    if (static_cast<int64_t>(nextDelta) -
            static_cast<int64_t>(currentTick) <=
        0) {
        // When the next event is due, 8002BB30 keeps invoking 8002BC40 while
        // it produces zero-delay events, then accumulates each nonzero delta
        // until the next event lies after the current tick.  A missing
        // bounded parser result is a hard input gap, never an invitation to
        // spin indefinitely in the host runtime.
        int32_t accumulated = nextDelta;
        std::size_t parserIndex = 0u;
        for (;;) {
            for (;;) {
                if (parserIndex >=
                    observation.parserDeltaAfterCalls.size()) {
                    out.known = false;
                    out.parserInputExhausted = true;
                    out.parserCallsBounded = false;
                    return false;
                }
                const uint32_t parserDelta =
                    observation.parserDeltaAfterCalls[parserIndex++];
                out.parserDeltaAfterCalls.push_back(parserDelta);
                ++out.parserCallCount;
                nextDelta = static_cast<int32_t>(parserDelta);
                if (nextDelta != 0) {
                    break;
                }
            }

            accumulated = AddSignedBits(accumulated, nextDelta);
            const int64_t remaining =
                static_cast<int64_t>(accumulated) -
                static_cast<int64_t>(currentTick);
            if (!(static_cast<int64_t>(accumulated) <
                  static_cast<int64_t>(currentTick))) {
                const int32_t result = static_cast<int32_t>(remaining);
                out.branch =
                    SsMidiSchedulerBranch8002BB30::DueEventParserLoop;
                out.dwordTrackOffset88After =
                    static_cast<uint32_t>(result);
                out.result = result;
                out.parserCallsBounded = true;
                return true;
            }
        }
    }

    int32_t countdown = observation.wordTrackOffset6E;
    if (countdown > 0) {
        --countdown;
        out.branch =
            SsMidiSchedulerBranch8002BB30::PositiveCountdownDecrement;
        out.wordTrackOffset6EAfter = static_cast<int16_t>(countdown);
        out.result = countdown;
        out.parserCallsBounded = true;
        return true;
    }

    // The second test is an unsigned halfword test in the original body.
    // Therefore a negative signed countdown takes this rebase path instead
    // of being normalized to zero.
    if (static_cast<uint16_t>(observation.wordTrackOffset6E) != 0u) {
        const int32_t rebased = AddSignedBits(
            nextDelta, -currentTick);
        out.branch =
            SsMidiSchedulerBranch8002BB30::NegativeCountdownRebase;
        out.dwordTrackOffset88After = static_cast<uint32_t>(rebased);
        out.result = countdown;
        out.parserCallsBounded = true;
        return true;
    }

    // A zero countdown arms it with the current +0x70 halfword and decrements
    // the pending delta once.  The halfword store is intentionally retained
    // as a low-16-bit value.
    const int32_t decremented = AddSignedBits(nextDelta, -1);
    out.branch =
        SsMidiSchedulerBranch8002BB30::CountdownArmAndDeltaDecrement;
    out.wordTrackOffset6EAfter = static_cast<int16_t>(
        static_cast<uint16_t>(currentTick));
    out.dwordTrackOffset88After = static_cast<uint32_t>(decremented);
    out.result = decremented;
    out.parserCallsBounded = true;
    return true;
}

bool TryBuildSsMidiProgramChangeDispatch8002BFDC(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiProgramChangeObservation8002BFDC& observation,
    SsMidiProgramChangeDispatch8002BFDC& out) {
    out = SsMidiProgramChangeDispatch8002BFDC{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0 ||
        observation.channel >= kSsMidiChannelCount8002BEEC) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.channel = observation.channel;
    out.program = observation.program;
    out.channelProgramsAfter = observation.channelProgramsBefore;
    out.channelProgramsAfter[observation.channel] = observation.program;
    out.programTableOffset =
        kSsMidiChannelProgramBaseOffset8002BFDC + observation.channel;
    out.lowerFunction = 0x8002BFDCu;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiProgramChangeDispatch8002BFDC{};
        return false;
    }

    out.deltaCursorAfter = cursor;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    return true;
}

bool TryBuildSsMidiControlChangeDispatch8002C054(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiControlChangeObservation8002C054& observation,
    SsMidiControlChangeDispatch8002C054& out) {
    out = SsMidiControlChangeDispatch8002C054{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0 ||
        observation.channel >= kSsMidiChannelCount8002BEEC) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.channel = observation.channel;
    out.controller = observation.controller;
    out.value = observation.value;
    out.vabIdAfter = observation.vabId;
    out.channelProgramsAfter = observation.channelPrograms;
    out.channelVolumesAfter = observation.channelVolumes;
    out.channelPansAfter = observation.channelPans;
    out.packedSequenceTrack =
        static_cast<uint16_t>(static_cast<uint8_t>(observation.sequenceIndex)) |
        static_cast<uint16_t>(static_cast<uint8_t>(observation.trackIndex))
            << 8u;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;

    // 8002C054 reads one controller value, then either handles a small set
    // of stateful controllers itself or delegates additional bytes to a
    // dedicated lower routine.  Cases that fall through LABEL_20 all decode
    // the next delta exactly once and publish it at +0x88.
    bool decodeDelta = true;
    switch (observation.controller) {
    case 0u:
        out.branch = SsMidiControlChangeBranch8002C054::VabIdStore;
        out.vabIdAfter = static_cast<int16_t>(
            static_cast<uint16_t>(observation.value));
        break;
    case 6u:
        out.branch =
            SsMidiControlChangeBranch8002C054::LowerOwnsAdditionalBytes;
        out.lowerFunction = 0x8002CB6Cu;
        out.lowerOwnsAdditionalBytes = true;
        decodeDelta = false;
        break;
    case 7u:
        out.branch = SsMidiControlChangeBranch8002C054::ChannelVolumeStore;
        out.channelVolumesAfter[observation.channel] = observation.value;
        out.callsGsVoiceUpdate80033D08 = true;
        out.gsVoiceUpdateFunction = 0x80033D08u;
        out.gsVoiceUpdateVabId = observation.vabId;
        out.gsVoiceUpdateProgram =
            observation.channelPrograms[observation.channel];
        out.gsVoiceUpdateVolume = observation.value;
        out.gsVoiceUpdatePan =
            observation.channelPans[observation.channel];
        break;
    case 10u:
        out.branch = SsMidiControlChangeBranch8002C054::ChannelPanStore;
        out.channelPansAfter[observation.channel] = observation.value;
        out.callsGsVoiceUpdate80033D08 = true;
        out.gsVoiceUpdateFunction = 0x80033D08u;
        out.gsVoiceUpdateVabId = observation.vabId;
        out.gsVoiceUpdateProgram =
            observation.channelPrograms[observation.channel];
        out.gsVoiceUpdateVolume =
            observation.channelVolumes[observation.channel];
        out.gsVoiceUpdatePan = observation.value;
        break;
    case 11u:
        out.branch =
            SsMidiControlChangeBranch8002C054::ProgramVolumeUpdate;
        out.callsSsVmSetProgVol = true;
        // SsVmSetProgVol is a library symbol in the SCUS database rather
        // than a game-local address; retain the call as a symbolic side
        // effect without inventing a host function pointer.
        out.ssVmSetProgVolFunction = 0u;
        out.callsGsVoiceUpdate80033D08 = true;
        out.gsVoiceUpdateFunction = 0x80033D08u;
        out.gsVoiceUpdateVabId = observation.vabId;
        out.gsVoiceUpdateProgram =
            observation.channelPrograms[observation.channel];
        out.gsVoiceUpdateVolume =
            observation.channelVolumes[observation.channel];
        out.gsVoiceUpdatePan =
            observation.channelPans[observation.channel];
        break;
    case 64u:
        out.branch =
            SsMidiControlChangeBranch8002C054::Threshold64Dispatch;
        out.callsThresholdFunction = true;
        out.thresholdFunction = observation.value >= 0x40u
                                    ? 0x8002EFE0u
                                    : 0x8002EFD0u;
        break;
    case 65u:
        out.branch =
            SsMidiControlChangeBranch8002C054::LowerOwnsAdditionalBytes;
        out.lowerFunction = 0x8002C5DCu;
        out.lowerOwnsAdditionalBytes = true;
        decodeDelta = false;
        break;
    case 91u:
        out.branch = SsMidiControlChangeBranch8002C054::Timer91Dispatch;
        out.callsTimerFunction = true;
        out.timerFunction = 0x800302A4u;
        out.timerArg0 = observation.value;
        out.timerArg1 = observation.value;
        break;
    case 98u:
        out.branch =
            SsMidiControlChangeBranch8002C054::LowerOwnsAdditionalBytes;
        out.lowerFunction = 0x8002C808u;
        out.lowerOwnsAdditionalBytes = true;
        decodeDelta = false;
        break;
    case 99u:
        out.branch =
            SsMidiControlChangeBranch8002C054::LowerOwnsAdditionalBytes;
        out.lowerFunction = 0x8002C938u;
        out.lowerOwnsAdditionalBytes = true;
        decodeDelta = false;
        break;
    case 100u:
        out.branch =
            SsMidiControlChangeBranch8002C054::LowerOwnsAdditionalBytes;
        out.lowerFunction = 0x8002CA7Cu;
        out.lowerOwnsAdditionalBytes = true;
        decodeDelta = false;
        break;
    case 101u:
        out.branch =
            SsMidiControlChangeBranch8002C054::LowerOwnsAdditionalBytes;
        out.lowerFunction = 0x8002CAF4u;
        out.lowerOwnsAdditionalBytes = true;
        decodeDelta = false;
        break;
    case 121u:
        out.branch =
            SsMidiControlChangeBranch8002C054::LowerOwnsAdditionalBytes;
        out.lowerFunction = 0x8002C740u;
        out.lowerOwnsAdditionalBytes = true;
        decodeDelta = false;
        break;
    default:
        out.branch = SsMidiControlChangeBranch8002C054::DefaultDeltaOnly;
        break;
    }

    if (!decodeDelta) {
        return true;
    }

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiControlChangeDispatch8002C054{};
        return false;
    }
    out.deltaDecoded = true;
    out.deltaCursorAfter = cursor;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    return true;
}

bool TryBuildSsMidiPitchBendDispatch8002D3D0(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiPitchBendObservation8002D3D0& observation,
    SsMidiPitchBendDispatch8002D3D0& out) {
    out = SsMidiPitchBendDispatch8002D3D0{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0 ||
        observation.channel >= kSsMidiChannelCount8002BEEC) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.channel = observation.channel;
    out.bendValue = observation.bendValue;
    out.packedSequenceTrack =
        static_cast<uint16_t>(static_cast<uint8_t>(observation.sequenceIndex)) |
        static_cast<uint16_t>(static_cast<uint8_t>(observation.trackIndex))
            << 8u;
    out.voiceUpdateVabId = observation.vabId;
    out.voiceUpdateProgram = observation.channelPrograms[observation.channel];
    out.voiceUpdateValue = observation.bendValue;
    out.voiceUpdateFunction = 0x80032A10u;
    out.lowerFunction = 0x8002D3D0u;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiPitchBendDispatch8002D3D0{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    return true;
}

bool TryBuildSsMidiMetaDispatch8002D47C(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiMetaObservation8002D47C& observation,
    SsMidiMetaDispatch8002D47C& out) {
    out = SsMidiMetaDispatch8002D47C{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.metaType = observation.metaType;
    out.wordTrackOffset72After = observation.wordTrackOffset72;
    out.wordTrackOffset6EAfter = static_cast<uint16_t>(
        observation.wordTrackOffset6E);
    out.wordTrackOffset70After = static_cast<uint16_t>(
        observation.wordTrackOffset70);
    out.dwordTrackOffset04After = observation.dwordTrackOffset04Cursor;
    out.dwordTrackOffset0CAfter = observation.dwordTrackOffset0C;
    out.dwordTrackOffset80After = observation.dwordTrackOffset80AbsoluteTime;
    out.dwordTrackOffset88After = observation.dwordTrackOffset88Delta;
    out.dwordTrackOffset90After = observation.dwordTrackOffset90Flags;
    out.byteTrackOffset27After = observation.byteTrackOffset27;
    out.byteTrackOffset2BAfter = observation.byteTrackOffset2B;
    out.lowerFunction = 0x8002D47Cu;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;

    if (observation.metaType == 0x2Fu) {
        const uint16_t incremented = static_cast<uint16_t>(
            observation.wordTrackOffset72 + 1u);
        out.wordTrackOffset72After = incremented;
        const int16_t incrementedSigned =
            static_cast<int16_t>(incremented);
        if (observation.wordTrackOffset70 != 0 &&
            incrementedSigned >= observation.wordTrackOffset70) {
            // End-of-track completion clears the active/loop flags, marks
            // the track finished, resets the cursor to the stream start,
            // and emits the same sequence-track cleanup twice.  The second
            // cleanup is unconditional in the original body.
            uint32_t flags = observation.dwordTrackOffset90Flags;
            flags &= ~1u;
            flags &= ~8u;
            flags &= ~2u;
            flags |= 0x200u;
            flags |= 4u;
            out.dwordTrackOffset90After = flags;
            out.dwordTrackOffset0CAfter =
                observation.dwordTrackOffset08StreamStart;
            out.byteTrackOffset2BAfter = 0u;
            out.callsSequenceTrackEnd = 2u;
            out.sequenceTrackEndFunction = 0x80033A34u;
            out.callsTrackEndCleanup =
                observation.byteTrackOffset3C != 0xFFu;
            out.trackEndCleanupFunction =
                out.callsTrackEndCleanup ? 0x8002D970u : 0u;
            out.trackEndCleanupArg = observation.byteTrackOffset3C;
            out.trackEndCleanupArg1 = observation.byteTrackOffset00;
            out.result = static_cast<uint32_t>(
                static_cast<int32_t>(observation.wordTrackOffset70));
            out.dwordTrackOffset88After = out.result;
            out.branch = SsMidiMetaBranch8002D47C::EndOfTrackComplete;
            return true;
        }

        // The non-terminal end marker rewinds to the stream start and
        // clears the absolute-time, end-marker and pending-delta fields.
        out.result = observation.dwordTrackOffset08StreamStart;
        out.dwordTrackOffset04After = out.result;
        out.dwordTrackOffset0CAfter = out.result;
        out.dwordTrackOffset80After = 0u;
        out.dwordTrackOffset88After = 0u;
        out.byteTrackOffset27After = 0u;
        out.branch = SsMidiMetaBranch8002D47C::EndOfTrackReset;
        return true;
    }

    if (observation.metaType != 0x51u) {
        // The original function returns 0x51 for unsupported meta ids and
        // leaves the track row untouched.
        out.branch = SsMidiMetaBranch8002D47C::UnsupportedResult81;
        out.result = 0x51u;
        return true;
    }

    const uint32_t tempo =
        (static_cast<uint32_t>(observation.tempoBytes[0]) << 16u) |
        (static_cast<uint32_t>(observation.tempoBytes[1]) << 8u) |
        static_cast<uint32_t>(observation.tempoBytes[2]);
    if (tempo == 0u || observation.tickRate80095C4C == 0u ||
        observation.wordTrackOffset74 == 0u) {
        out = SsMidiMetaDispatch8002D47C{};
        return false;
    }

    const uint32_t bpm = 60000000u / tempo;
    const uint64_t product =
        static_cast<uint64_t>(observation.wordTrackOffset74) * bpm;
    if (bpm == 0u || product == 0u) {
        out = SsMidiMetaDispatch8002D47C{};
        return false;
    }
    out.tempoMicrosecondsPerQuarter = tempo;
    out.tempoBpm = bpm;
    out.dwordTrackOffset8CAfter = bpm;

    const uint64_t tickRate = observation.tickRate80095C4C;
    const uint64_t sixtyTickRate = 60u * tickRate;
    const uint64_t tenProduct = 10u * product;
    if (tenProduct >= sixtyTickRate) {
        const uint64_t scaled = tenProduct / sixtyTickRate;
        const uint64_t remainder = tenProduct % sixtyTickRate;
        out.wordTrackOffset6EAfter = static_cast<uint16_t>(0xFFFFu);
        uint64_t current = scaled;
        if (2u * 15u * tickRate < remainder) {
            ++current;
        }
        out.wordTrackOffset70After = static_cast<uint16_t>(current);
    } else {
        const uint64_t countdown =
            (600u * tickRate) / product;
        if (countdown == 0u) {
            out = SsMidiMetaDispatch8002D47C{};
            return false;
        }
        out.wordTrackOffset6EAfter = static_cast<uint16_t>(countdown);
        out.wordTrackOffset70After = static_cast<uint16_t>(countdown);
    }

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiMetaDispatch8002D47C{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaDecoded = true;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    out.result = deltaTicks10;
    out.branch = SsMidiMetaBranch8002D47C::TempoUpdate;
    return true;
}

bool TryBuildSsMidiCallbackDispatch8002C808(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiCallbackObservation8002C808& observation,
    SsMidiCallbackDispatch8002C808& out) {
    out = SsMidiCallbackDispatch8002C808{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.callbackValue = observation.callbackValue;
    out.byteTrackOffset16After = observation.byteTrackOffset16;
    out.byteTrackOffset21After = observation.byteTrackOffset21;
    out.byteTrackOffset22After = observation.byteTrackOffset22;
    out.byteTrackOffset39After = observation.byteTrackOffset39;
    out.byteTrackOffset40After = observation.byteTrackOffset40;
    out.byteTrackOffset41After = observation.byteTrackOffset41;
    out.byteTrackOffset42After = observation.byteTrackOffset42;
    out.callbackTarget = observation.callbackTarget;
    out.callbackTargetPresent = observation.callbackTarget != 0u;
    out.lowerFunction = 0x8002C808u;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;

    // 8002C808 first arms a pending callback when +39 is one and +16 is
    // clear.  It then re-reads +22: modes other than 20/30 store the value at
    // +21 and increment the byte counter at +42; mode 40 additionally loads
    // and calls the callback-table target.  The translated owner records that
    // call edge but deliberately does not execute an unknown function pointer.
    const bool armPending = observation.byteTrackOffset39 == 1u &&
                            observation.byteTrackOffset16 == 0u;
    if (armPending) {
        out.byteTrackOffset40After = observation.callbackValue;
        out.byteTrackOffset16After = 1u;
    }

    const uint8_t mode = observation.byteTrackOffset22;
    if (mode != 30u && mode != 20u) {
        out.byteTrackOffset21After = observation.callbackValue;
        out.byteTrackOffset42After = static_cast<uint8_t>(
            static_cast<uint16_t>(observation.byteTrackOffset42) + 1u);
        out.callbackDispatchObserved = mode == 40u;
        out.branch = armPending
                         ? SsMidiCallbackBranch8002C808::ArmAndDecode
                         : (mode == 40u
                                ? SsMidiCallbackBranch8002C808::
                                      AccumulateAndCallback
                                : SsMidiCallbackBranch8002C808::
                                      AccumulateNoCallback);
    } else {
        out.branch = SsMidiCallbackBranch8002C808::Mode20Or30Decode;
    }

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiCallbackDispatch8002C808{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaDecoded = true;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    out.lowerCallCommitted = false;
    out.callbackCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return true;
}

bool TryBuildSsMidiCallbackGateDispatch8002C938(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiCallbackGateObservation8002C938& observation,
    SsMidiCallbackGateDispatch8002C938& out) {
    out = SsMidiCallbackGateDispatch8002C938{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.inputValue = observation.inputValue;
    out.dwordTrackOffset04After = observation.dwordTrackOffset04Cursor;
    out.dwordTrackOffset0CAfter = observation.dwordTrackOffset0CStoredCursor;
    out.byteTrackOffset16After = observation.byteTrackOffset16;
    out.byteTrackOffset22After = observation.byteTrackOffset22;
    out.byteTrackOffset39After = observation.byteTrackOffset39;
    out.byteTrackOffset40After = observation.byteTrackOffset40;
    out.byteTrackOffset42After = observation.byteTrackOffset42;
    out.lowerFunction = 0x8002C938u;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;

    // The incoming value is the controller-99 argument, not a MIDI delta.
    // Preserve the original byte stores and the special mode-30 pending
    // counter behavior before decoding the following delta.
    if (observation.inputValue == 20u) {
        out.branch = SsMidiCallbackGateBranch8002C938::Mode20Arm;
        out.byteTrackOffset22After = 20u;
        out.byteTrackOffset39After = 1u;
    } else if (observation.inputValue != 30u) {
        out.branch = SsMidiCallbackGateBranch8002C938::ModeOtherAccumulate;
        out.byteTrackOffset22After = observation.inputValue;
        out.byteTrackOffset42After = static_cast<uint8_t>(
            static_cast<uint16_t>(observation.byteTrackOffset42) + 1u);
    } else {
        out.byteTrackOffset22After = 30u;
        if (observation.byteTrackOffset40 == 0u) {
            out.branch = SsMidiCallbackGateBranch8002C938::Mode30ClearGate;
            out.byteTrackOffset16After = 0u;
        } else if (observation.byteTrackOffset40 >= 0x7Fu) {
            // The saturated branch decodes the delta but publishes zero as
            // the next scheduler value, then restores the saved cursor.
            out.branch =
                SsMidiCallbackGateBranch8002C938::Mode30FinishPending;
            out.dwordTrackOffset04After =
                observation.dwordTrackOffset0CStoredCursor;
        } else {
            out.branch =
                SsMidiCallbackGateBranch8002C938::Mode30RetainPending;
            out.byteTrackOffset40After = static_cast<uint8_t>(
                observation.byteTrackOffset40 - 1u);
            if (out.byteTrackOffset40After != 0u) {
                out.dwordTrackOffset04After =
                    observation.dwordTrackOffset0CStoredCursor;
            } else {
                out.byteTrackOffset16After = 0u;
            }
        }
    }

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiCallbackGateDispatch8002C938{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaDecoded = true;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    if (out.branch == SsMidiCallbackGateBranch8002C938::
            Mode30FinishPending) {
        out.dwordTrackOffset88After = 0u;
        out.result = 0u;
    } else {
        out.dwordTrackOffset88After = deltaTicks10;
        out.result = deltaTicks10;
    }
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return true;
}

bool TryBuildSsMidiTrackByteCounterDispatch8002CA7C(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackByteCounterObservation8002CA7C& observation,
    SsMidiTrackByteCounterDispatch8002CA7C& out) {
    out = SsMidiTrackByteCounterDispatch8002CA7C{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.inputValue = observation.inputValue;
    out.byteTrackOffset19After = observation.inputValue;
    out.byteTrackOffset41After = static_cast<uint8_t>(
        static_cast<uint16_t>(observation.byteTrackOffset41) + 1u);
    out.lowerFunction = 0x8002CA7Cu;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiTrackByteCounterDispatch8002CA7C{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaDecoded = true;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return true;
}

bool TryBuildSsMidiTrackByteCounterDispatch8002CAF4(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackByteCounterObservation8002CAF4& observation,
    SsMidiTrackByteCounterDispatch8002CAF4& out) {
    out = SsMidiTrackByteCounterDispatch8002CAF4{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.inputValue = observation.inputValue;
    out.byteTrackOffset20After = observation.inputValue;
    out.byteTrackOffset41After = static_cast<uint8_t>(
        static_cast<uint16_t>(observation.byteTrackOffset41) + 1u);
    out.lowerFunction = 0x8002CAF4u;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiTrackByteCounterDispatch8002CAF4{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaDecoded = true;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return true;
}

bool TryBuildSsMidiControllerResetDispatch8002C740(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiControllerResetObservation8002C740& observation,
    SsMidiControllerResetDispatch8002C740& out) {
    out = SsMidiControllerResetDispatch8002C740{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0 ||
        observation.channel >= kSsMidiChannelCount8002BEEC) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.channel = observation.channel;
    out.byteTrackOffset19After = 0u;
    out.byteTrackOffset20After = 0u;
    out.channelProgramScratchAfter = observation.channelProgramScratch;
    out.channelVolumeScratchAfter = observation.channelVolumeScratch;
    out.channelPanScratchAfter = observation.channelPanScratch;
    // 8002C740 indexes the per-channel scratch by the track's +18 byte.
    // The byte at +18 is itself preserved; the selected channel's program
    // marker is reset to its channel index, volume to 127 and pan to 64.
    out.selectedProgramIndex = observation.channel;
    out.selectedProgramScratchAfter = observation.channel;
    out.selectedVolumeAfter = 127u;
    out.selectedPanAfter = 64u;
    out.channelProgramScratchAfter[observation.channel] = observation.channel;
    out.channelVolumeScratchAfter[observation.channel] = 127u;
    out.channelPanScratchAfter[observation.channel] = 64u;
    out.callsTimerReset = true;
    out.timerResetFunction = 0x80030244u;
    out.callsThresholdReset = true;
    out.thresholdResetFunction = 0x8002EFD0u;
    out.lowerFunction = 0x8002C740u;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiControllerResetDispatch8002C740{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaDecoded = true;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return true;
}

bool TryBuildSsMidiController65Dispatch8002C5DC(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiController65Observation8002C5DC& observation,
    SsMidiController65Dispatch8002C5DC& out) {
    out = SsMidiController65Dispatch8002C5DC{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0 || observation.vabId < 0 ||
        observation.vabId >= 16u ||
        observation.toneCount > kTonesPerProgram) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.vabId = observation.vabId;
    out.program = observation.program;
    out.inputValue = observation.inputValue;
    out.toneCount = observation.toneCount;
    out.toneReadCallCount = observation.toneCount;
    out.toneWriteCallCount = observation.toneCount;
    out.toneReadFunction = 0x8002F200u;
    out.toneWriteFunction = 0x8003037Cu;
    if (observation.toneCount == 0u) {
        out.band = SsMidiController65Band8002C5DC::NoTones;
    } else if (observation.inputValue < 0x40u) {
        out.band = SsMidiController65Band8002C5DC::Below64;
    } else if (observation.inputValue < 0x80u) {
        out.band = SsMidiController65Band8002C5DC::Inclusive64To127;
    } else {
        out.band = SsMidiController65Band8002C5DC::Above127;
    }
    out.lowerFunction = 0x8002C5DCu;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiController65Dispatch8002C5DC{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaDecoded = true;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return true;
}

bool TryBuildSsMidiController6Dispatch8002CB6C(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiController6Observation8002CB6C& observation,
    SsMidiController6Dispatch8002CB6C& out) {
    out = SsMidiController6Dispatch8002CB6C{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.trackIndex < 0 || observation.vabId < 0 ||
        observation.vabId >= 16 ||
        observation.channel >= kSsMidiChannelCount8002BEEC ||
        observation.toneCount > kTonesPerProgram) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.vabId = observation.vabId;
    out.channel = observation.channel;
    out.program = observation.program;
    out.inputValue = observation.inputValue;
    out.toneCount = observation.toneCount;
    out.byteTrackOffset16After = observation.byteTrackOffset16;
    out.byteTrackOffset19After = observation.byteTrackOffset19;
    out.byteTrackOffset20After = observation.byteTrackOffset20;
    out.byteTrackOffset21After = observation.byteTrackOffset21;
    out.byteTrackOffset22After = observation.byteTrackOffset22;
    out.byteTrackOffset39After = observation.byteTrackOffset39;
    out.byteTrackOffset40After = observation.byteTrackOffset40;
    out.byteTrackOffset41After = observation.byteTrackOffset41;
    out.byteTrackOffset42After = observation.byteTrackOffset42;
    out.toneReadFunction = 0x8002F200u;
    out.toneWriteFunction = 0x8003037Cu;
    out.toneUpdateFunction = 0x8002D10Cu;
    out.lowerFunction = 0x8002CB6Cu;
    out.deltaDecoderFunction = surface.deltaDecoderFunction;

    // 8002CB6C has an early pending-callback arm.  When that arm is taken,
    // the original jumps straight to the common delta tail and does not run
    // any of the +41/+42 tone branches in this invocation.
    const bool armPending = observation.byteTrackOffset39 == 1u &&
                            observation.byteTrackOffset16 == 0u;
    if (armPending) {
        out.byteTrackOffset40After = observation.inputValue;
        out.byteTrackOffset16After = 1u;
        out.pendingCallbackArmed = true;
        out.branch = SsMidiController6Branch8002CB6C::ArmPending;
    } else if (observation.byteTrackOffset41 == 2u) {
        if (observation.byteTrackOffset19 == 0u &&
            observation.byteTrackOffset20 == 0u) {
            out.branch = SsMidiController6Branch8002CB6C::
                ToneRewriteBase;
            out.toneReadCallCount = observation.toneCount;
            out.toneWriteCallCount = observation.toneCount;
            out.baseToneRewriteValue = observation.inputValue & 0x7Fu;
        } else if (observation.byteTrackOffset19 == 1u &&
                   observation.byteTrackOffset20 == 0u) {
            out.branch = SsMidiController6Branch8002CB6C::
                ToneRewriteMode1;
            out.toneReadCallCount = observation.toneCount;
            out.toneWriteCallCount = observation.toneCount;
        } else if (observation.byteTrackOffset19 == 2u &&
                   observation.byteTrackOffset20 == 0u) {
            out.branch = SsMidiController6Branch8002CB6C::
                ToneRewriteMode2;
            out.toneReadCallCount = observation.toneCount;
            out.toneWriteCallCount = observation.toneCount;
        } else {
            out.branch = SsMidiController6Branch8002CB6C::
                TrackCounterReset;
        }
        out.byteTrackOffset41After = 0u;
        out.counter41Reset = true;
    } else if (observation.byteTrackOffset42 == 2u) {
        out.toneUpdateMode = observation.byteTrackOffset22;
        out.toneUpdateValue = observation.inputValue;
        if (observation.byteTrackOffset22 == 16u) {
            out.branch = SsMidiController6Branch8002CB6C::
                DynamicToneUpdateAll;
            out.toneUpdateCallCount = observation.toneCount;
        } else {
            out.branch = SsMidiController6Branch8002CB6C::
                DynamicToneUpdateSingle;
            // The original non-16 path invokes 8002D10C once even when the
            // reported program tone count is zero.
            out.toneUpdateCallCount = 1u;
        }
        out.byteTrackOffset42After = 0u;
        out.counter42Reset = true;
    } else {
        out.branch = SsMidiController6Branch8002CB6C::DeltaOnly;
    }

    std::size_t cursor = observation.deltaCursorBefore;
    uint32_t absoluteTime = observation.absoluteTimeBefore;
    uint32_t deltaTicks10 = 0u;
    if (!TryDecodeSsMidiDelta8002D7D0(
            observation.deltaStreamBytes,
            cursor,
            absoluteTime,
            deltaTicks10)) {
        out = SsMidiController6Dispatch8002CB6C{};
        return false;
    }
    out.deltaCursorAfter = cursor;
    out.deltaDecoded = true;
    out.deltaTicks10 = deltaTicks10;
    out.absoluteTimeAfter = absoluteTime;
    out.dwordTrackOffset88After = deltaTicks10;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return true;
}

bool TryBuildSsMidiEnvelopeDispatch8002B474(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiEnvelopeObservation8002B474& observation,
    SsMidiEnvelopeDispatch8002B474& out) {
    out = SsMidiEnvelopeDispatch8002B474{};
    if (!IsValidSsMidiEnvelopeObservation(surface, observation)) {
        return false;
    }
    SeedSsMidiEnvelopeDispatch(observation, 0x8002B474u, out);
    out.branch = SsMidiEnvelopeBranch8002B474::ZeroStepCopy;

    const int16_t step = observation.wordTrackOffset42;
    const uint32_t counterAfter = out.dwordTrackOffset98After;
    int16_t current = observation.wordTrackOffset40;
    if (step > 0) {
        const uint32_t remainder = counterAfter %
                                    static_cast<uint32_t>(step);
        if (remainder != 0u) {
            out.branch = SsMidiEnvelopeBranch8002B474::
                PositiveModuloSkip;
            out.result = remainder;
            FinishSsMidiEnvelopeRead(out);
            return true;
        }
        current = WrapSsMidiI16(static_cast<int32_t>(current) - 1);
        out.wordTrackOffset40After = current;
        if (current < 0) {
            out.branch = SsMidiEnvelopeBranch8002B474::PositiveReset;
            ObserveSsMidiEnvelopeWrite(127, 127, out);
            out.dwordTrackOffset90FlagsAfter &= ~0x10u;
            out.flag10Cleared = true;
        } else {
            ObserveSsMidiEnvelopeRead(
                observation.wordTrackOffset74,
                observation.wordTrackOffset76,
                out);
            if (current == 0) {
                out.branch = SsMidiEnvelopeBranch8002B474::PositiveReset;
            } else {
                out.branch = SsMidiEnvelopeBranch8002B474::PositiveRamp;
                ObserveSsMidiEnvelopeWrite(
                    static_cast<int32_t>(observation.wordTrackOffset74) + 1,
                    static_cast<int32_t>(observation.wordTrackOffset76) + 1,
                    out);
            }
        }
    } else if (step < 0) {
        current = WrapSsMidiI16(static_cast<int32_t>(current) + step);
        out.wordTrackOffset40After = current;
        if (current < 0) {
            out.branch = SsMidiEnvelopeBranch8002B474::NegativeReset;
            ObserveSsMidiEnvelopeWrite(127, 127, out);
            out.dwordTrackOffset90FlagsAfter &= ~0x10u;
            out.flag10Cleared = true;
        } else {
            out.branch = SsMidiEnvelopeBranch8002B474::NegativeRamp;
            ObserveSsMidiEnvelopeRead(
                observation.wordTrackOffset74,
                observation.wordTrackOffset76,
                out);
            const int32_t negativeStep = -static_cast<int32_t>(step);
            if (static_cast<uint32_t>(observation.wordTrackOffset74) -
                    static_cast<int32_t>(step) >=
                127u &&
                static_cast<uint32_t>(observation.wordTrackOffset76) -
                    static_cast<int32_t>(step) >=
                127u) {
                ObserveSsMidiEnvelopeWrite(127, 127, out);
            }
            const uint32_t elapsed =
                observation.dwordTrackOffset94 - counterAfter;
            const uint32_t threshold = static_cast<uint32_t>(
                static_cast<int32_t>(observation.wordTrackOffset3E));
            if (static_cast<uint64_t>(elapsed) *
                    static_cast<uint64_t>(negativeStep) <
                static_cast<uint64_t>(threshold)) {
                ObserveSsMidiEnvelopeWrite(
                    static_cast<int32_t>(observation.wordTrackOffset74) -
                        static_cast<int32_t>(step),
                    static_cast<int32_t>(observation.wordTrackOffset76) -
                        static_cast<int32_t>(step),
                    out);
            }
        }
    } else {
        out.wordTrackOffset40After = current;
        out.branch = SsMidiEnvelopeBranch8002B474::ZeroStepCopy;
    }

    if (step != 0 &&
        (counterAfter == 0u || out.wordTrackOffset40After == 0)) {
        out.dwordTrackOffset90FlagsAfter &= ~0x10u;
        out.flag10Cleared = true;
    }
    FinishSsMidiEnvelopeRead(out);
    return true;
}

bool TryBuildSsMidiEnvelopeDispatch8002B750(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiEnvelopeObservation8002B474& observation,
    SsMidiEnvelopeDispatch8002B750& out) {
    out = SsMidiEnvelopeDispatch8002B750{};
    if (!IsValidSsMidiEnvelopeObservation(surface, observation)) {
        return false;
    }
    SeedSsMidiEnvelopeDispatch(observation, out);

    const int16_t step = observation.wordTrackOffset42;
    const uint32_t counterAfter = out.dwordTrackOffset98After;
    int16_t current = observation.wordTrackOffset40;
    if (step <= 0) {
        current = WrapSsMidiI16(static_cast<int32_t>(current) + step);
        out.wordTrackOffset40After = current;
        if (current <= 0) {
            out.branch = SsMidiEnvelopeBranch8002B750::NonPositiveReset;
            out.dwordTrackOffset90FlagsAfter &= ~0x20u;
            out.flag20Cleared = true;
        } else {
            ObserveSsMidiEnvelopeRead(
                observation.wordTrackOffset74,
                observation.wordTrackOffset76,
                out);
            const uint32_t elapsed =
                observation.dwordTrackOffset94 - counterAfter;
            const uint32_t threshold = static_cast<uint32_t>(
                static_cast<int32_t>(observation.wordTrackOffset3E));
            const uint32_t negativeStep =
                static_cast<uint32_t>(-static_cast<int32_t>(step));
            if (static_cast<uint64_t>(threshold) >=
                    static_cast<uint64_t>(elapsed) * negativeStep &&
                negativeStep < observation.wordTrackOffset74) {
                out.branch = SsMidiEnvelopeBranch8002B750::
                    NonPositiveRamp;
                ObserveSsMidiEnvelopeWrite(
                    static_cast<int32_t>(observation.wordTrackOffset74) +
                        static_cast<int32_t>(step),
                    static_cast<int32_t>(observation.wordTrackOffset76) +
                        static_cast<int32_t>(step),
                    out);
            } else {
                out.branch = SsMidiEnvelopeBranch8002B750::
                    NonPositiveReset;
                ObserveSsMidiEnvelopeWrite(1, 1, out);
            }
        }
    } else {
        const uint32_t remainder = counterAfter %
                                    static_cast<uint32_t>(step);
        if (remainder != 0u) {
            out.branch = SsMidiEnvelopeBranch8002B750::
                PositiveModuloSkip;
            out.result = remainder;
            FinishSsMidiEnvelopeRead(out);
            return true;
        }
        current = WrapSsMidiI16(static_cast<int32_t>(current) - 1);
        out.wordTrackOffset40After = current;
        if (current <= 0) {
            out.branch = SsMidiEnvelopeBranch8002B750::PositiveReset;
            out.dwordTrackOffset90FlagsAfter &= ~0x20u;
            out.flag20Cleared = true;
        } else {
            out.branch = SsMidiEnvelopeBranch8002B750::PositiveRamp;
            ObserveSsMidiEnvelopeRead(
                observation.wordTrackOffset74,
                observation.wordTrackOffset76,
                out);
            if (observation.wordTrackOffset74 <= current ||
                observation.wordTrackOffset76 <= current ||
                observation.wordTrackOffset74 == 1u) {
                ObserveSsMidiEnvelopeWrite(1, 1, out);
            } else {
                ObserveSsMidiEnvelopeWrite(
                    static_cast<int32_t>(observation.wordTrackOffset74) - 1,
                    static_cast<int32_t>(observation.wordTrackOffset76) - 1,
                    out);
            }
        }
    }

    if (counterAfter == 0u || out.wordTrackOffset40After == 0) {
        out.dwordTrackOffset90FlagsAfter &= ~0x20u;
        out.flag20Cleared = true;
    }
    FinishSsMidiEnvelopeRead(out);
    return true;
}

bool TryBuildSsMidiTempoTickDispatch8002DDA4(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTempoTickObservation8002DDA4& observation,
    SsMidiTempoTickDispatch8002DDA4& out) {
    out = SsMidiTempoTickDispatch8002DDA4{};
    if (!IsExactSsMidiParserSurface8002BA9C(surface) ||
        !observation.known || observation.sequenceIndex < 0 ||
        observation.sequenceIndex > 0xFF || observation.trackIndex < 0 ||
        observation.trackIndex > 0xFF) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.dwordTrackOffsetA0After = observation.dwordTrackOffsetA0 - 1u;
    out.dwordTrackOffset8CAfter = observation.dwordTrackOffset8C;
    out.dwordTrackOffset90FlagsAfter = observation.dwordTrackOffset90Flags;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;

    const int16_t period = observation.wordTrackOffset44;
    uint32_t accumulator = observation.dwordTrackOffset8C;
    if (period > 0) {
        out.positiveModulo = out.dwordTrackOffsetA0After %
                             static_cast<uint32_t>(period);
        if (out.positiveModulo != 0u) {
            out.branch = SsMidiTempoTickBranch8002DDA4::
                PositiveModuloSkip;
            out.result = out.positiveModulo;
            return true;
        }
        const uint32_t next = accumulator - 1u;
        if (observation.dwordTrackOffsetA4 != accumulator) {
            accumulator = next;
        }
        out.branch = SsMidiTempoTickBranch8002DDA4::PositiveStepDown;
    } else if (observation.dwordTrackOffsetA4 >= accumulator) {
        if (observation.dwordTrackOffsetA4 == accumulator) {
            out.branch = SsMidiTempoTickBranch8002DDA4::NonPositiveHold;
        } else {
            const uint32_t next = accumulator -
                static_cast<uint32_t>(static_cast<int32_t>(period));
            accumulator = next;
            if (observation.dwordTrackOffsetA4 < accumulator) {
                accumulator = observation.dwordTrackOffsetA4;
                out.branch = SsMidiTempoTickBranch8002DDA4::
                    NonPositiveClamp;
            } else {
                out.branch = SsMidiTempoTickBranch8002DDA4::
                    NonPositiveRebase;
            }
        }
    } else {
        const uint32_t next = accumulator +
            static_cast<uint32_t>(static_cast<int32_t>(period));
        accumulator = next;
        if (accumulator < observation.dwordTrackOffsetA4) {
            accumulator = observation.dwordTrackOffsetA4;
            out.branch = SsMidiTempoTickBranch8002DDA4::NonPositiveClamp;
        } else {
            out.branch = SsMidiTempoTickBranch8002DDA4::NonPositiveRebase;
        }
    }
    out.dwordTrackOffset8CAfter = accumulator;

    out.denominator = static_cast<uint64_t>(60u) *
                      observation.tickRate80095C4C;
    if (out.denominator == 0u) {
        out = SsMidiTempoTickDispatch8002DDA4{};
        return false;
    }
    const int64_t signedTerm =
        static_cast<int64_t>(observation.wordTrackOffset74) * 10;
    const uint64_t unsignedTerm = static_cast<uint32_t>(signedTerm);
    out.numerator = unsignedTerm * accumulator;
    const uint64_t scaled = out.numerator / out.denominator;
    uint16_t scaledWord = static_cast<uint16_t>(scaled);
    if (scaledWord == 0u || scaledWord >= 0x8000u) {
        scaledWord = 1u;
    }
    out.wordTrackOffset70After = scaledWord;
    if (out.dwordTrackOffsetA0After == 0u ||
        accumulator == observation.dwordTrackOffsetA4) {
        out.dwordTrackOffset90FlagsAfter &= ~0xC0u;
        out.flags40And80Cleared = true;
    }
    out.result = 0u;
    out.divisorKnown = true;
    return true;
}

namespace {

bool IsValidSsMidiTrackRouteObservation(
    const SsMidiParserSurface8002BA9C& surface,
    int16_t sequenceIndex,
    int16_t trackIndex,
    bool known) {
    return IsExactSsMidiParserSurface8002BA9C(surface) && known &&
           sequenceIndex >= 0 && sequenceIndex <= 0xFF && trackIndex >= 0 &&
           trackIndex <= 0xFF;
}

void SeedSsMidiTrackFlagToggleDispatch(
    const SsMidiTrackFlagToggleObservation8002B9FC& observation,
    uint32_t clearMask,
    uint8_t byteAfter,
    SsMidiTrackFlagToggleDispatch8002B9FC& out) {
    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.byteTrackOffset2BAfter = byteAfter;
    out.dwordTrackOffset90FlagsAfter =
        observation.dwordTrackOffset90Flags & ~clearMask;
    out.packedSequenceTrack = static_cast<uint16_t>(
        static_cast<uint16_t>(observation.sequenceIndex) |
        (static_cast<uint16_t>(observation.trackIndex) << 8u));
    out.cleanupFunction = 0x80033A34u;
    out.cleanupObserved = true;
    out.result = out.dwordTrackOffset90FlagsAfter;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
}

}  // namespace

bool TryBuildSsMidiTrackFlagToggleDispatch8002B9FC(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackFlagToggleObservation8002B9FC& observation,
    SsMidiTrackFlagToggleDispatch8002B9FC& out) {
    out = SsMidiTrackFlagToggleDispatch8002B9FC{};
    if (!IsValidSsMidiTrackRouteObservation(
            surface,
            observation.sequenceIndex,
            observation.trackIndex,
            observation.known)) {
        return false;
    }
    SeedSsMidiTrackFlagToggleDispatch(observation, 0x02u, 0u, out);
    return true;
}

bool TryBuildSsMidiTrackFlagToggleDispatch8002BAC8(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackFlagToggleObservation8002B9FC& observation,
    SsMidiTrackFlagToggleDispatch8002B9FC& out) {
    out = SsMidiTrackFlagToggleDispatch8002B9FC{};
    if (!IsValidSsMidiTrackRouteObservation(
            surface,
            observation.sequenceIndex,
            observation.trackIndex,
            observation.known)) {
        return false;
    }
    SeedSsMidiTrackFlagToggleDispatch(observation, 0x08u, 1u, out);
    return true;
}

bool TryBuildSsMidiTrackResetDispatch8002DBE4(
    const SsMidiParserSurface8002BA9C& surface,
    const SsMidiTrackResetObservation8002DBE4& observation,
    SsMidiTrackResetDispatch8002DBE4& out) {
    out = SsMidiTrackResetDispatch8002DBE4{};
    if (!IsValidSsMidiTrackRouteObservation(
            surface,
            observation.sequenceIndex,
            observation.trackIndex,
            observation.known)) {
        return false;
    }

    out.known = true;
    out.sequenceIndex = observation.sequenceIndex;
    out.trackIndex = observation.trackIndex;
    out.dwordTrackOffset90FlagsAfter =
        (observation.dwordTrackOffset90Flags & ~0x0Eu) | 0x04u;
    out.dwordTrackOffset80After = 0u;
    out.dwordTrackOffset88After = observation.dwordTrackOffset7C;
    out.dwordTrackOffset8CAfter = observation.dwordTrackOffset84;
    out.dwordTrackOffset04After = observation.dwordTrackOffset08;
    out.dwordTrackOffset0CAfter = observation.dwordTrackOffset08;
    out.wordTrackOffset72After = 0u;
    out.wordTrackOffset70After = observation.wordTrackOffset72;
    out.byteTrackOffset16After = 0u;
    out.byteTrackOffset17After = 0u;
    out.byteTrackOffset18After = 0u;
    out.byteTrackOffset19After = 0u;
    out.byteTrackOffset20After = 0u;
    out.byteTrackOffset21After = 0u;
    out.byteTrackOffset22After = 0u;
    out.byteTrackOffset2BAfter = 0u;
    out.byteTrackOffset39After = 0u;
    out.byteTrackOffset40After = 0u;
    out.byteTrackOffset41After = 0u;
    out.byteTrackOffset42After = 0u;
    out.byteTrackOffset43After = 0u;
    for (std::size_t index = 0u;
         index < out.channelProgramMarkersAfter.size(); ++index) {
        out.channelProgramMarkersAfter[index] =
            static_cast<uint8_t>(index);
        out.channelPansAfter[index] = 64u;
        out.channelVolumesAfter[index] = 127u;
    }
    out.channelLoopCount =
        static_cast<uint32_t>(out.channelProgramMarkersAfter.size());
    out.wordTrackOffset78After = 127u;
    out.wordTrackOffset7AAfter = 127u;
    out.packedSequenceTrack = static_cast<uint16_t>(
        static_cast<uint16_t>(observation.sequenceIndex) |
        (static_cast<uint16_t>(observation.trackIndex) << 8u));
    out.cleanupFunction = 0x80033A34u;
    out.cleanupObserved = true;
    out.result = 127u;
    out.lowerCallCommitted = false;
    out.psxMemoryAuthority = false;
    out.lowerEventAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;
    return true;
}

SsMidiNoteSurface8002BEEC BuildSsMidiNoteSurface8002BEEC() {
    SsMidiNoteSurface8002BEEC surface{};
    surface.known = true;
    surface.noteDecisionCompleteWithinLimits = true;
    surface.hostToneLayerSelectionCompleteWithinLimits = true;
    surface.hostNoteOffOwnershipCompleteWithinLimits = true;
    surface.noteDecisionFunction = 0x8002BEECu;
    surface.noteOnFunction = 0x80032EACu;
    surface.noteOffFunction = 0x8003349Cu;
    surface.sequenceStatePointerTableAddress = 0x80095D08u;
    surface.trackStateStride = 0xACu;
    surface.channelOffset = 0x12u;
    surface.channelPanBaseOffset = 0x17u;
    surface.channelProgramBaseOffset = 0x2Cu;
    surface.vabIdOffset = 0x4Cu;
    surface.channelVolumeBaseOffset = 0x4Eu;
    surface.sequenceEnabledOffset = 0x74u;
    surface.sequenceVolumeLeftOffset = 0x74u;
    surface.sequenceVolumeRightOffset = 0x76u;
    surface.lastVelocityOffset = 0xA8u;
    surface.muteMaskOffset = 0xAAu;
    surface.programAttributeStride = 0x10u;
    surface.toneAttributeStride = 0x20u;
    surface.toneNoteMinOffset = 0x06u;
    surface.toneNoteMaxOffset = 0x07u;
    surface.maxTonesPerProgram = 16u;
    surface.crossFileExplicitReferenceScanCompleteWithinScope = true;
    surface.psxMemoryAuthority = false;
    surface.psxVoiceIdentityAuthority = false;
    surface.psxSpuTimingAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsMidiNoteSurface8002BEEC(
    const SsMidiNoteSurface8002BEEC& surface) {
    const SsMidiNoteSurface8002BEEC expected =
        BuildSsMidiNoteSurface8002BEEC();
    return surface.known == expected.known &&
           surface.noteDecisionCompleteWithinLimits ==
               expected.noteDecisionCompleteWithinLimits &&
           surface.hostToneLayerSelectionCompleteWithinLimits ==
               expected.hostToneLayerSelectionCompleteWithinLimits &&
           surface.hostNoteOffOwnershipCompleteWithinLimits ==
               expected.hostNoteOffOwnershipCompleteWithinLimits &&
           surface.noteDecisionFunction == expected.noteDecisionFunction &&
           surface.noteOnFunction == expected.noteOnFunction &&
           surface.noteOffFunction == expected.noteOffFunction &&
           surface.sequenceStatePointerTableAddress ==
               expected.sequenceStatePointerTableAddress &&
           surface.trackStateStride == expected.trackStateStride &&
           surface.channelOffset == expected.channelOffset &&
           surface.channelPanBaseOffset ==
               expected.channelPanBaseOffset &&
           surface.channelProgramBaseOffset ==
               expected.channelProgramBaseOffset &&
           surface.vabIdOffset == expected.vabIdOffset &&
           surface.channelVolumeBaseOffset ==
               expected.channelVolumeBaseOffset &&
           surface.sequenceEnabledOffset ==
               expected.sequenceEnabledOffset &&
           surface.sequenceVolumeLeftOffset ==
               expected.sequenceVolumeLeftOffset &&
           surface.sequenceVolumeRightOffset ==
               expected.sequenceVolumeRightOffset &&
           surface.lastVelocityOffset == expected.lastVelocityOffset &&
           surface.muteMaskOffset == expected.muteMaskOffset &&
           surface.programAttributeStride ==
               expected.programAttributeStride &&
           surface.toneAttributeStride == expected.toneAttributeStride &&
           surface.toneNoteMinOffset == expected.toneNoteMinOffset &&
           surface.toneNoteMaxOffset == expected.toneNoteMaxOffset &&
           surface.maxTonesPerProgram == expected.maxTonesPerProgram &&
           surface.crossFileExplicitReferenceScanCompleteWithinScope ==
               expected.crossFileExplicitReferenceScanCompleteWithinScope &&
           surface.psxMemoryAuthority == expected.psxMemoryAuthority &&
           surface.psxVoiceIdentityAuthority ==
               expected.psxVoiceIdentityAuthority &&
           surface.psxSpuTimingAuthority ==
               expected.psxSpuTimingAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority == expected.stage2PlusAuthority;
}

bool TryBuildSsMidiNoteDispatch8002BEEC(
    const SsMidiNoteSurface8002BEEC& surface,
    SsMidiNoteTrackState8002BEEC& state,
    uint8_t note,
    uint8_t velocity,
    SsMidiNoteDispatch8002BEEC& out) {
    out = SsMidiNoteDispatch8002BEEC{};
    if (!IsExactSsMidiNoteSurface8002BEEC(surface) || !state.known ||
        state.sequenceIndex < 0 || state.trackIndex < 0 ||
        state.channel >= kSsMidiChannelCount8002BEEC) {
        return false;
    }

    const uint8_t channel = state.channel;
    out.known = true;
    out.sequenceIndex = state.sequenceIndex;
    out.trackIndex = state.trackIndex;
    out.packedSequenceTrack =
        static_cast<uint16_t>(
            static_cast<uint8_t>(state.sequenceIndex)) |
        static_cast<uint16_t>(
            static_cast<uint8_t>(state.trackIndex))
            << 8u;
    out.channel = channel;
    out.vabId = state.vabId;
    out.program = state.channelProgram[channel];
    out.note = note;
    out.velocity = velocity;
    out.pan = state.channelPan[channel];
    out.channelVolume = state.channelVolume[channel];
    out.sequenceVolumeLeft = state.sequenceEnabled;
    out.sequenceVolumeRight = state.sequenceVolumeRight;
    out.lastVelocityAfter = state.lastVelocity;
    out.muted = ((state.muteMask >> channel) & 1u) != 0u;
    out.sequenceEnabled = state.sequenceEnabled != 0u;
    out.lowerCallCommitted = false;
    out.hostProjectionReady = false;
    out.psxMemoryAuthority = false;
    out.psxVoiceIdentityAuthority = false;
    out.psxSpuTimingAuthority = false;

    if (out.muted || !out.sequenceEnabled) {
        return true;
    }

    out.hostProjectionReady = true;
    if (velocity != 0u) {
        out.action = SsMidiNoteAction8002BEEC::NoteOn;
        out.lowerFunction = surface.noteOnFunction;
        out.scaledVelocity =
            static_cast<uint32_t>(velocity) *
            static_cast<uint32_t>(out.channelVolume) / 127u;
        state.lastVelocity = static_cast<int16_t>(velocity);
        out.lastVelocityUpdated = true;
        out.lastVelocityAfter = state.lastVelocity;
    } else {
        out.action = SsMidiNoteAction8002BEEC::NoteOff;
        out.lowerFunction = surface.noteOffFunction;
    }
    return true;
}

SsDriverFlushSurface80026ECC BuildSsDriverFlushSurface80026ECC() {
    SsDriverFlushSurface80026ECC surface{};
    surface.known = true;
    surface.wrapperAndReentryControlCompleteWithinLimits = true;
    surface.wrapperFunction = 0x80026ECCu;
    surface.wrapperBusyFlagAddress = 0x800943B4u;
    surface.lowerFlushFunction = 0x8002EFF4u;
    surface.reentryGuardAddress = 0x800917A4u;
    surface.driverCommitFunction = 0x80032B00u;
    surface.lowerFlushExplicitCodeXrefCount = 1u;
    surface.comod0DirectWrapperCallCount = 3u;
    surface.comod1DirectWrapperCallCount = 2u;
    surface.comod0PathEntersSequenceScheduler = false;
    surface.psxSpuRegisterAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsDriverFlushSurface80026ECC(
    const SsDriverFlushSurface80026ECC& surface) {
    const SsDriverFlushSurface80026ECC expected =
        BuildSsDriverFlushSurface80026ECC();
    return surface.known == expected.known &&
           surface.wrapperAndReentryControlCompleteWithinLimits ==
               expected.wrapperAndReentryControlCompleteWithinLimits &&
           surface.wrapperFunction == expected.wrapperFunction &&
           surface.wrapperBusyFlagAddress ==
               expected.wrapperBusyFlagAddress &&
           surface.lowerFlushFunction == expected.lowerFlushFunction &&
           surface.reentryGuardAddress == expected.reentryGuardAddress &&
           surface.driverCommitFunction == expected.driverCommitFunction &&
           surface.lowerFlushExplicitCodeXrefCount ==
               expected.lowerFlushExplicitCodeXrefCount &&
           surface.comod0DirectWrapperCallCount ==
               expected.comod0DirectWrapperCallCount &&
           surface.comod1DirectWrapperCallCount ==
               expected.comod1DirectWrapperCallCount &&
           surface.comod0PathEntersSequenceScheduler ==
               expected.comod0PathEntersSequenceScheduler &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority == expected.stage2PlusAuthority;
}

bool TryExecuteSsDriverFlush80026ECC(
    const SsDriverFlushSurface80026ECC& surface,
    SsDriverFlushState80026ECC& state,
    SsDriverCommitSink80032B00 driverCommit,
    void* userData,
    SsDriverFlushDispatch80026ECC& out) {
    out = SsDriverFlushDispatch80026ECC{};
    if (!IsExactSsDriverFlushSurface80026ECC(surface) ||
        driverCommit == nullptr) {
        return false;
    }

    out.known = true;
    out.hostProjection = true;
    out.psxSpuRegisterAuthority = false;
    if (state.wrapperBusyFlag800943B4 != 0) {
        out.wrapperSkippedByBusyFlag = true;
        out.result = state.wrapperBusyFlag800943B4;
        return true;
    }

    out.lowerFlushEntered = true;
    if (state.reentryGuard800917A4 == 1) {
        out.lowerSkippedByReentryGuard = true;
        out.result = state.reentryGuard800917A4;
        return true;
    }

    state.reentryGuard800917A4 = 1;
    out.reentryGuardSetBeforeCommit = true;
    out.result = driverCommit(userData);
    out.driverCommitExecuted = true;
    state.reentryGuard800917A4 = 0;
    out.reentryGuardClearedAfterCommit = true;
    return true;
}

SsDriverVoiceCompletionSurface80032B00
BuildSsDriverVoiceCompletionSurface80032B00() {
    SsDriverVoiceCompletionSurface80032B00 surface{};
    surface.known = true;
    surface.completionHistoryControlCompleteWithinLimits = true;
    surface.function = 0x80032B00u;
    surface.voiceCountAddress = 0x800928A0u;
    surface.completionRingIndexAddress = 0x80088220u;
    surface.completionRingBaseAddress = 0x80088224u;
    surface.completionScanDisableAddress = 0x8009290Cu;
    surface.spuVoiceBasePointerAddress = 0x80055DD8u;
    surface.voiceEnvelopeRegisterOffset = 0x0Cu;
    surface.voiceStateBaseAddress = 0x80087D5Bu;
    surface.voiceStateStride = 0x34u;
    surface.noiseMaskClearFunction = 0x800353E8u;
    surface.noiseMaskClearValue = 0x00FFFFFFu;
    surface.maxVoiceCount = 24u;
    surface.completionRingEntryCount = 16u;
    surface.stableCompletionEntryCount = 15u;
    surface.psxEnvelopeAuthority = false;
    surface.psxNoiseRegisterAuthority = false;
    surface.psxSpuRegisterAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsDriverVoiceCompletionSurface80032B00(
    const SsDriverVoiceCompletionSurface80032B00& surface) {
    const SsDriverVoiceCompletionSurface80032B00 expected =
        BuildSsDriverVoiceCompletionSurface80032B00();
    return surface.known == expected.known &&
           surface.completionHistoryControlCompleteWithinLimits ==
               expected.completionHistoryControlCompleteWithinLimits &&
           surface.function == expected.function &&
           surface.voiceCountAddress == expected.voiceCountAddress &&
           surface.completionRingIndexAddress ==
               expected.completionRingIndexAddress &&
           surface.completionRingBaseAddress ==
               expected.completionRingBaseAddress &&
           surface.completionScanDisableAddress ==
               expected.completionScanDisableAddress &&
           surface.spuVoiceBasePointerAddress ==
               expected.spuVoiceBasePointerAddress &&
           surface.voiceEnvelopeRegisterOffset ==
               expected.voiceEnvelopeRegisterOffset &&
           surface.voiceStateBaseAddress ==
               expected.voiceStateBaseAddress &&
           surface.voiceStateStride == expected.voiceStateStride &&
           surface.noiseMaskClearFunction ==
               expected.noiseMaskClearFunction &&
           surface.noiseMaskClearValue == expected.noiseMaskClearValue &&
           surface.maxVoiceCount == expected.maxVoiceCount &&
           surface.completionRingEntryCount ==
               expected.completionRingEntryCount &&
           surface.stableCompletionEntryCount ==
               expected.stableCompletionEntryCount &&
           surface.psxEnvelopeAuthority == expected.psxEnvelopeAuthority &&
           surface.psxNoiseRegisterAuthority ==
               expected.psxNoiseRegisterAuthority &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority == expected.stage2PlusAuthority;
}

bool TryExecuteSsDriverVoiceCompletion80032B00(
    const SsDriverVoiceCompletionSurface80032B00& surface,
    const SsDriverVoiceCompletionObservation80032B00& observation,
    SsDriverVoiceCompletionState80032B00& state,
    SsDriverVoiceCompletionDispatch80032B00& out) {
    out = SsDriverVoiceCompletionDispatch80032B00{};
    if (!IsExactSsDriverVoiceCompletionSurface80032B00(surface) ||
        observation.voiceCount > surface.maxVoiceCount) {
        return false;
    }

    out.known = true;
    out.hostEnvelopeProjection = true;
    out.psxEnvelopeAuthority = false;
    out.psxNoiseRegisterAuthority = false;
    out.psxSpuRegisterAuthority = false;
    out.completionRingIndexBefore = state.completionRingIndex;
    state.completionRingIndex =
        (state.completionRingIndex + 1u) & 0x0Fu;
    out.completionRingIndexAfter = state.completionRingIndex;

    uint32_t currentZeroEnvelopeMask = 0u;
    for (uint32_t voice = 0u; voice < observation.voiceCount; ++voice) {
        if (observation.envelopeObservations[voice] == 0u) {
            currentZeroEnvelopeMask |= 1u << voice;
        }
    }
    state.completionRing[state.completionRingIndex] =
        currentZeroEnvelopeMask;
    out.currentZeroEnvelopeMask = currentZeroEnvelopeMask;

    if (observation.completionScanDisabled) {
        out.completionScanSkipped = true;
        return true;
    }

    uint32_t stableZeroEnvelopeMask = 0xFFFFFFFFu;
    for (uint32_t index = 0u;
         index < surface.stableCompletionEntryCount;
         ++index) {
        stableZeroEnvelopeMask &= state.completionRing[index];
    }
    const uint32_t voiceMask =
        observation.voiceCount == 0u
            ? 0u
            : ((1u << observation.voiceCount) - 1u);
    stableZeroEnvelopeMask &= voiceMask;
    out.stableZeroEnvelopeMask = stableZeroEnvelopeMask;

    for (uint32_t voice = 0u; voice < observation.voiceCount; ++voice) {
        const uint32_t voiceBit = 1u << voice;
        if ((stableZeroEnvelopeMask & voiceBit) == 0u) {
            continue;
        }
        if (state.voiceStatus[voice] == 2u) {
            ++out.noiseMaskClearCallCount;
        }
        state.voiceStatus[voice] = 0u;
        out.clearedVoiceStatusMask |= voiceBit;
    }
    return true;
}

SsDriverVolumeRampSurface80032B00
BuildSsDriverVolumeRampSurface80032B00() {
    SsDriverVolumeRampSurface80032B00 surface{};
    surface.known = true;
    surface.twoRampFunctionsCompleteWithinLimits = true;
    surface.firstFunction = 0x80031A28u;
    surface.secondFunction = 0x80031F28u;
    surface.voiceStateBaseAddress = 0x80087D40u;
    surface.voiceStateStride = 0x34u;
    surface.firstActiveOffset = 0x1Cu;
    surface.firstStepOffset = 0x1Eu;
    surface.firstPeriodOffset = 0x20u;
    surface.firstCountdownOffset = 0x22u;
    surface.firstCurrentOffset = 0x24u;
    surface.firstTargetOffset = 0x26u;
    surface.secondActiveOffset = 0x28u;
    surface.secondStepOffset = 0x2Au;
    surface.secondPeriodOffset = 0x2Cu;
    surface.secondCountdownOffset = 0x2Eu;
    surface.secondCurrentOffset = 0x30u;
    surface.secondTargetOffset = 0x32u;
    surface.pendingVolumeLeftAddress = 0x80087BA8u;
    surface.pendingVolumeRightAddress = 0x80087BAAu;
    surface.dirtyFlagsAddress = 0x80087D28u;
    surface.sharedScratchFirstAddress = 0x800928DCu;
    surface.sharedScratchSecondAddress = 0x800928DDu;
    surface.psxVolumeAuthority = false;
    surface.psxSpuRegisterAuthority = false;
    return surface;
}

bool IsExactSsDriverVolumeRampSurface80032B00(
    const SsDriverVolumeRampSurface80032B00& surface) {
    const SsDriverVolumeRampSurface80032B00 expected =
        BuildSsDriverVolumeRampSurface80032B00();
    return surface.known == expected.known &&
           surface.twoRampFunctionsCompleteWithinLimits ==
               expected.twoRampFunctionsCompleteWithinLimits &&
           surface.firstFunction == expected.firstFunction &&
           surface.secondFunction == expected.secondFunction &&
           surface.voiceStateBaseAddress ==
               expected.voiceStateBaseAddress &&
           surface.voiceStateStride == expected.voiceStateStride &&
           surface.firstActiveOffset == expected.firstActiveOffset &&
           surface.firstStepOffset == expected.firstStepOffset &&
           surface.firstPeriodOffset == expected.firstPeriodOffset &&
           surface.firstCountdownOffset ==
               expected.firstCountdownOffset &&
           surface.firstCurrentOffset == expected.firstCurrentOffset &&
           surface.firstTargetOffset == expected.firstTargetOffset &&
           surface.secondActiveOffset == expected.secondActiveOffset &&
           surface.secondStepOffset == expected.secondStepOffset &&
           surface.secondPeriodOffset == expected.secondPeriodOffset &&
           surface.secondCountdownOffset ==
               expected.secondCountdownOffset &&
           surface.secondCurrentOffset == expected.secondCurrentOffset &&
           surface.secondTargetOffset == expected.secondTargetOffset &&
           surface.pendingVolumeLeftAddress ==
               expected.pendingVolumeLeftAddress &&
           surface.pendingVolumeRightAddress ==
               expected.pendingVolumeRightAddress &&
           surface.dirtyFlagsAddress == expected.dirtyFlagsAddress &&
           surface.sharedScratchFirstAddress ==
               expected.sharedScratchFirstAddress &&
           surface.sharedScratchSecondAddress ==
               expected.sharedScratchSecondAddress &&
           surface.psxVolumeAuthority == expected.psxVolumeAuthority &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority;
}

SsDriverVolumeRampSetupSurface80035110
BuildSsDriverVolumeRampSetupSurface80035110() {
    SsDriverVolumeRampSetupSurface80035110 surface{};
    surface.known = true;
    surface.setupFunctionCompleteWithinLimits = true;
    surface.wrapperFunction = 0x80035110u;
    surface.setupFunction = 0x80031878u;
    surface.voiceStateBaseAddress = 0x80087D40u;
    surface.voiceStateStride = 0x34u;
    surface.activeOffset = 0x1Cu;
    surface.stepOffset = 0x1Eu;
    surface.periodOffset = 0x20u;
    surface.countdownOffset = 0x22u;
    surface.currentOffset = 0x24u;
    surface.targetOffset = 0x26u;
    surface.maxVoiceCount = 24u;
    surface.psxMemoryAuthority = false;
    surface.psxSpuTimingAuthority = false;
    return surface;
}

bool IsExactSsDriverVolumeRampSetupSurface80035110(
    const SsDriverVolumeRampSetupSurface80035110& surface) {
    const SsDriverVolumeRampSetupSurface80035110 expected =
        BuildSsDriverVolumeRampSetupSurface80035110();
    return surface.known == expected.known &&
           surface.setupFunctionCompleteWithinLimits ==
               expected.setupFunctionCompleteWithinLimits &&
           surface.wrapperFunction == expected.wrapperFunction &&
           surface.setupFunction == expected.setupFunction &&
           surface.voiceStateBaseAddress ==
               expected.voiceStateBaseAddress &&
           surface.voiceStateStride == expected.voiceStateStride &&
           surface.activeOffset == expected.activeOffset &&
           surface.stepOffset == expected.stepOffset &&
           surface.periodOffset == expected.periodOffset &&
           surface.countdownOffset == expected.countdownOffset &&
           surface.currentOffset == expected.currentOffset &&
           surface.targetOffset == expected.targetOffset &&
           surface.maxVoiceCount == expected.maxVoiceCount &&
           surface.psxMemoryAuthority == expected.psxMemoryAuthority &&
           surface.psxSpuTimingAuthority == expected.psxSpuTimingAuthority;
}

bool TryExecuteSsDriverVolumeRampSetup80035110(
    const SsDriverVolumeRampSetupSurface80035110& surface,
    uint16_t voiceIndex,
    int16_t start,
    int16_t target,
    int16_t rate,
    SsDriverVoiceRegisterState80032B00& voice,
    SsDriverVolumeRampSetupDispatch80035110& out) {
    out = SsDriverVolumeRampSetupDispatch80035110{};
    if (!IsExactSsDriverVolumeRampSetupSurface80035110(surface)) {
        return false;
    }

    out.known = true;
    out.voiceIndex = voiceIndex;
    out.start = start;
    out.target = target;
    out.rate = rate;
    out.wrapperFunction = surface.wrapperFunction;
    out.setupFunction = surface.setupFunction;
    out.psxMemoryAuthority = false;
    out.psxSpuTimingAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;

    const auto publishRamp = [&]() {
        out.activeAfter = voice.firstRamp.active;
        out.stepAfter = voice.firstRamp.step;
        out.periodAfter = voice.firstRamp.period;
        out.countdownAfter = voice.firstRamp.countdown;
        out.currentAfter = voice.firstRamp.current;
        out.targetAfter = voice.firstRamp.target;
    };

    // 80035110's only wrapper guard is the 24-voice bound.  The original
    // wrapper returns -1 without touching the selected voice when it fails.
    if (voiceIndex >= surface.maxVoiceCount) {
        out.branch =
            SsDriverVolumeRampSetupBranch80035110::WrapperVoiceReject;
        out.wrapperVoiceRejected = true;
        publishRamp();
        return true;
    }

    SsDriverRampState80032B00& ramp = voice.firstRamp;
    if (start == target) {
        // 80031878 returns before writing even the active bit.  Preserve all
        // prior ramp fields instead of normalizing them on this no-op path.
        out.branch = SsDriverVolumeRampSetupBranch80035110::EqualNoOp;
        out.equalStartTargetNoOp = true;
        publishRamp();
        return true;
    }

    // The lower routine publishes active/current/target before it performs
    // either quotient.  This ordering matters for a malformed rate as well.
    ramp.active = 1;
    ramp.current = static_cast<uint16_t>(start);
    ramp.target = target;

    const int32_t difference =
        static_cast<int32_t>(start) - static_cast<int32_t>(target);
    const int32_t absoluteDifference =
        difference < 0 ? -difference : difference;
    const bool coarseStep =
        absoluteDifference >= static_cast<int32_t>(rate);
    if (coarseStep) {
        // The coarse branch divides (start-target) by the supplied rate and
        // writes period=0.  A zero rate reaches the original _break(7,0);
        // keep the already-published fields but do not invent a quotient.
        if (rate == 0) {
            out.branch =
                SsDriverVolumeRampSetupBranch80035110::DivisionGuarded;
            out.divisionGuarded = true;
            publishRamp();
            return true;
        }
        if (rate == -1 && difference ==
                              std::numeric_limits<int32_t>::min()) {
            out.branch =
                SsDriverVolumeRampSetupBranch80035110::DivisionGuarded;
            out.divisionGuarded = true;
            out.divisionOverflowGuarded = true;
            publishRamp();
            return true;
        }
        ramp.period = 0;
        ramp.step = static_cast<int16_t>(difference /
                                         static_cast<int32_t>(rate));
        // The PSX coarse path does not write D62; preserve its prior value.
        out.branch = SsDriverVolumeRampSetupBranch80035110::CoarseStep;
        out.lowerCallCommitted = true;
        publishRamp();
        return true;
    }

    // The fine branch uses step=1 and stores rate/(start-target) into both
    // period and countdown.  `difference != 0` is guaranteed by the no-op
    // return above, so this quotient has no zero denominator.
    if (difference == -1 &&
        static_cast<int32_t>(rate) ==
            std::numeric_limits<int32_t>::min()) {
        out.branch =
            SsDriverVolumeRampSetupBranch80035110::DivisionGuarded;
        out.divisionGuarded = true;
        out.divisionOverflowGuarded = true;
        publishRamp();
        return true;
    }
    const int32_t quotient =
        static_cast<int32_t>(rate) / difference;
    ramp.step = 1;
    ramp.period = static_cast<int16_t>(quotient);
    ramp.countdown = static_cast<uint16_t>(quotient);
    out.branch = SsDriverVolumeRampSetupBranch80035110::FineStep;
    out.lowerCallCommitted = true;
    publishRamp();
    return true;
}

SsDriverDirectVolumeWriteSurface80034E5C
BuildSsDriverDirectVolumeWriteSurface80034E5C() {
    SsDriverDirectVolumeWriteSurface80034E5C surface{};
    surface.known = true;
    surface.threeWriteFunctionsCompleteWithinLimits = true;
    surface.ownerIdentityFunction = 0x80034E5Cu;
    surface.directFunction = 0x80034F8Cu;
    surface.scaledFunction = 0x80035084u;
    surface.voiceStateBaseAddress = 0x80087D40u;
    surface.voiceStateStride = 0x34u;
    surface.sourceVabIdOffset = 0x16u;
    surface.programOffset = 0x12u;
    surface.noteOffset = 0x0Cu;
    surface.pendingVolumeLeftAddress = 0x80087BA8u;
    surface.pendingVolumeRightAddress = 0x80087BAAu;
    surface.dirtyFlagsAddress = 0x80087D28u;
    surface.directDirtyMask = 0x03u;
    surface.identityDirtyMask = 0x30u;
    surface.scaledFactor = 129u;
    surface.maxVoiceCount = 24u;
    surface.psxMemoryAuthority = false;
    surface.psxSpuRegisterAuthority = false;
    return surface;
}

bool IsExactSsDriverDirectVolumeWriteSurface80034E5C(
    const SsDriverDirectVolumeWriteSurface80034E5C& surface) {
    const SsDriverDirectVolumeWriteSurface80034E5C expected =
        BuildSsDriverDirectVolumeWriteSurface80034E5C();
    return surface.known == expected.known &&
           surface.threeWriteFunctionsCompleteWithinLimits ==
               expected.threeWriteFunctionsCompleteWithinLimits &&
           surface.ownerIdentityFunction == expected.ownerIdentityFunction &&
           surface.directFunction == expected.directFunction &&
           surface.scaledFunction == expected.scaledFunction &&
           surface.voiceStateBaseAddress ==
               expected.voiceStateBaseAddress &&
           surface.voiceStateStride == expected.voiceStateStride &&
           surface.sourceVabIdOffset == expected.sourceVabIdOffset &&
           surface.programOffset == expected.programOffset &&
           surface.noteOffset == expected.noteOffset &&
           surface.pendingVolumeLeftAddress ==
               expected.pendingVolumeLeftAddress &&
           surface.pendingVolumeRightAddress ==
               expected.pendingVolumeRightAddress &&
           surface.dirtyFlagsAddress == expected.dirtyFlagsAddress &&
           surface.directDirtyMask == expected.directDirtyMask &&
           surface.identityDirtyMask == expected.identityDirtyMask &&
           surface.scaledFactor == expected.scaledFactor &&
           surface.maxVoiceCount == expected.maxVoiceCount &&
           surface.psxMemoryAuthority == expected.psxMemoryAuthority &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority;
}

bool TryExecuteSsDriverDirectVolumeWrite80034E5C(
    const SsDriverDirectVolumeWriteSurface80034E5C& surface,
    SsDriverDirectVolumeWriteKind80034E5C kind,
    uint16_t voiceIndex,
    int16_t sourceVabId,
    int16_t program,
    int16_t note,
    int16_t left,
    int16_t right,
    const SsDriverVoiceOwnerState8003226C& owner,
    SsDriverVoiceRegisterState80032B00& voice,
    SsDriverDirectVolumeWriteDispatch80034E5C& out) {
    out = SsDriverDirectVolumeWriteDispatch80034E5C{};
    if (!IsExactSsDriverDirectVolumeWriteSurface80034E5C(surface)) {
        return false;
    }

    out.known = true;
    out.kind = kind;
    out.voiceIndex = voiceIndex;
    out.sourceVabId = sourceVabId;
    out.program = program;
    out.note = note;
    out.leftRequested = left;
    out.rightRequested = right;
    out.ownerIdentityFunction = surface.ownerIdentityFunction;
    out.directFunction = surface.directFunction;
    out.scaledFunction = surface.scaledFunction;
    out.scaledFactor = surface.scaledFactor;
    out.psxMemoryAuthority = false;
    out.psxSpuRegisterAuthority = false;
    out.oldWinS0Authority = false;
    out.stage2PlusAuthority = false;

    const auto publish = [&]() {
        out.pendingVolumeLeftAfter = voice.pendingVolumeLeft;
        out.pendingVolumeRightAfter = voice.pendingVolumeRight;
        out.dirtyFlagsAfter = voice.dirtyFlags;
    };
    if (voiceIndex >= surface.maxVoiceCount) {
        out.wrapperVoiceRejected = true;
        out.result = -1;
        out.function = kind ==
                SsDriverDirectVolumeWriteKind80034E5C::OwnerIdentity80034E5C
            ? surface.ownerIdentityFunction
            : kind == SsDriverDirectVolumeWriteKind80034E5C::Direct80034F8C
                ? surface.directFunction
                : surface.scaledFunction;
        publish();
        return true;
    }

    if (kind == SsDriverDirectVolumeWriteKind80034E5C::OwnerIdentity80034E5C) {
        out.function = surface.ownerIdentityFunction;
        // 80034E5C compares the three identity words before it calls the
        // pending-volume writer.  Compare source-width bits for VAB IDs so a
        // signed -1 owner remains distinguishable from an unknown value.
        out.ownerIdentityMatched =
            owner.sourceVabId80087D56 == static_cast<uint16_t>(sourceVabId) &&
            owner.program80087D52 == static_cast<uint16_t>(program) &&
            owner.note80087D4C == static_cast<uint16_t>(note);
        if (!out.ownerIdentityMatched) {
            out.result = -1;
            publish();
            return true;
        }
        voice.pendingVolumeLeft = static_cast<uint16_t>(left);
        voice.pendingVolumeRight = static_cast<uint16_t>(right);
        voice.dirtyFlags |= surface.identityDirtyMask;
    } else if (kind == SsDriverDirectVolumeWriteKind80034E5C::Direct80034F8C) {
        out.function = surface.directFunction;
        voice.pendingVolumeLeft = static_cast<uint16_t>(left);
        voice.pendingVolumeRight = static_cast<uint16_t>(right);
        voice.dirtyFlags |= surface.directDirtyMask;
    } else {
        out.function = surface.scaledFunction;
        out.scaledLeft = static_cast<int32_t>(left) *
                         static_cast<int32_t>(surface.scaledFactor);
        out.scaledRight = static_cast<int32_t>(right) *
                          static_cast<int32_t>(surface.scaledFactor);
        voice.pendingVolumeLeft = static_cast<uint16_t>(out.scaledLeft);
        voice.pendingVolumeRight = static_cast<uint16_t>(out.scaledRight);
        voice.dirtyFlags |= surface.directDirtyMask;
    }
    out.writeCommitted = true;
    out.result = 0;
    publish();
    return true;
}

static int32_t Signed32FromBits80032B00(uint32_t bits) {
    if (bits <= static_cast<uint32_t>(
                    std::numeric_limits<int32_t>::max())) {
        return static_cast<int32_t>(bits);
    }
    return -1 - static_cast<int32_t>(
                    std::numeric_limits<uint32_t>::max() - bits);
}

static int32_t MultiplyLowSigned80032B00(int32_t left, int32_t right) {
    const uint64_t product =
        static_cast<uint64_t>(static_cast<uint32_t>(left)) *
        static_cast<uint64_t>(static_cast<uint32_t>(right));
    return Signed32FromBits80032B00(
        static_cast<uint32_t>(product));
}

static uint32_t ScaleDriverVolumeBase80032B00(
    int32_t amplitude,
    const SsDriverVolumeMix80032B00& mix) {
    int32_t value = MultiplyLowSigned80032B00(
        amplitude,
        static_cast<int32_t>(mix.masterVolume) * 0x3FFF);
    value /= 0x3F01;
    value = MultiplyLowSigned80032B00(
        value, static_cast<int32_t>(mix.mix800928E2));
    value = MultiplyLowSigned80032B00(
        value, static_cast<int32_t>(mix.mix800928E5));
    return static_cast<uint32_t>(value) / 0x3F01u;
}

static uint32_t MultiplyThenLogicalShiftSix80032B00(
    uint32_t value,
    uint32_t factor) {
    const int32_t product = MultiplyLowSigned80032B00(
        Signed32FromBits80032B00(value),
        static_cast<int32_t>(factor));
    return static_cast<uint32_t>(product) >> 6u;
}

static uint32_t MultiplyU16ThenShiftSix80032B00(
    uint32_t value,
    uint32_t factor) {
    const int32_t product = MultiplyLowSigned80032B00(
        static_cast<int32_t>(static_cast<uint16_t>(value)),
        static_cast<int32_t>(factor));
    const uint32_t adjusted =
        product < 0
            ? static_cast<uint32_t>(product + 0x3F)
            : static_cast<uint32_t>(product);
    return adjusted >> 6u;
}

static void ApplyDriverPan80032B00(
    uint8_t pan,
    uint32_t& left,
    uint32_t& right,
    bool useFullWidthInput) {
    if (pan < 0x40u) {
        right = useFullWidthInput
                    ? MultiplyThenLogicalShiftSix80032B00(right, pan)
                    : MultiplyU16ThenShiftSix80032B00(right, pan);
    } else {
        left = useFullWidthInput
                   ? MultiplyThenLogicalShiftSix80032B00(
                         left, 127u - pan)
                   : MultiplyU16ThenShiftSix80032B00(
                         left, 127u - pan);
    }
}

bool TryExecuteSsDriverVolumeRamp80032B00(
    const SsDriverVolumeRampSurface80032B00& surface,
    SsDriverVolumeRampKind80032B00 kind,
    const SsDriverVolumeMix80032B00& mix,
    SsDriverVolumeScratch80032B00& scratch,
    SsDriverVoiceRegisterState80032B00& voice,
    SsDriverVolumeRampDispatch80032B00& out) {
    out = SsDriverVolumeRampDispatch80032B00{};
    if (!IsExactSsDriverVolumeRampSurface80032B00(surface)) {
        return false;
    }

    SsDriverRampState80032B00& ramp =
        kind == SsDriverVolumeRampKind80032B00::First80031A28
            ? voice.firstRamp
            : voice.secondRamp;
    out.known = true;
    out.kind = kind;
    out.psxVolumeAuthority = false;
    out.psxSpuRegisterAuthority = false;

    if (ramp.period != 0) {
        const uint16_t countdownBefore = ramp.countdown;
        ramp.countdown =
            static_cast<uint16_t>(countdownBefore - 1u);
        const int32_t shiftedCountdown =
            Signed32FromBits80032B00(
                static_cast<uint32_t>(countdownBefore) << 16u);
        if (shiftedCountdown > 0) {
            out.countdownSkippedUpdate = true;
            out.currentAfter = ramp.current;
            out.dirtyFlagsAfter = voice.dirtyFlags;
            return true;
        }
        ramp.countdown = static_cast<uint16_t>(ramp.period);
    }

    ramp.current = static_cast<uint16_t>(
        ramp.current + static_cast<uint16_t>(ramp.step));
    out.currentAdvanced = true;
    const int16_t currentSigned =
        static_cast<int16_t>(ramp.current);
    if ((ramp.step > 0 && currentSigned >= ramp.target) ||
        (ramp.step < 0 && currentSigned <= ramp.target)) {
        ramp.current = static_cast<uint16_t>(ramp.target);
        ramp.active = 0;
        out.targetReached = true;
    }
    out.currentAfter = ramp.current;

    int32_t amplitude = 0;
    uint8_t finalPan = 0u;
    if (kind == SsDriverVolumeRampKind80032B00::First80031A28) {
        scratch.firstCurrent800928DC =
            static_cast<uint8_t>(ramp.current);
        amplitude = static_cast<int16_t>(ramp.current);
        finalPan = scratch.secondCurrent800928DD;
    } else {
        scratch.secondCurrent800928DD =
            static_cast<uint8_t>(ramp.current);
        amplitude = scratch.firstCurrent800928DC;
        finalPan = static_cast<uint8_t>(ramp.current);
    }

    const uint32_t base =
        ScaleDriverVolumeBase80032B00(amplitude, mix);
    uint32_t left = base;
    uint32_t right = base;
    ApplyDriverPan80032B00(
        mix.pan800928E6, left, right, true);
    ApplyDriverPan80032B00(
        mix.pan800928E3, left, right, false);
    ApplyDriverPan80032B00(
        finalPan, left, right, false);
    if (mix.monoFlag80091728 == 1) {
        const uint16_t mono = std::max(
            static_cast<uint16_t>(left),
            static_cast<uint16_t>(right));
        left = mono;
        right = mono;
    }

    voice.pendingVolumeLeft = static_cast<uint16_t>(left);
    voice.pendingVolumeRight = static_cast<uint16_t>(right);
    voice.dirtyFlags |= 0x03u;
    out.pendingVolumeLeft = voice.pendingVolumeLeft;
    out.pendingVolumeRight = voice.pendingVolumeRight;
    out.dirtyFlagsAfter = voice.dirtyFlags;
    out.hostVolumeProjectionReady = true;
    return true;
}

SsDriverRegisterCommitSurface80032B00
BuildSsDriverRegisterCommitSurface80032B00() {
    SsDriverRegisterCommitSurface80032B00 surface{};
    surface.known = true;
    surface.dirtyRegisterAndMaskControlCompleteWithinLimits = true;
    surface.function = 0x80032B00u;
    surface.dirtyLoopStart = 0x80032D28u;
    surface.dirtyLoopEnd = 0x80032E18u;
    surface.voiceCount = 24u;
    surface.voiceRegisterStride = 0x10u;
    surface.volumeDirtyMask = 0x01u;
    surface.pitchDirtyMask = 0x04u;
    surface.startAddressDirtyMask = 0x08u;
    surface.adsrDirtyMask = 0x10u;
    surface.keyOffLowAddress = 0x801C386Cu;
    surface.keyOffHighAddress = 0x801C386Eu;
    surface.keyOnLowAddress = 0x8008EC98u;
    surface.keyOnHighAddress = 0x8008EC9Au;
    surface.reverbLowAddress = 0x8008EC9Cu;
    surface.reverbHighAddress = 0x8008EC9Eu;
    surface.spuKeyOffLowOffset = 0x18Cu;
    surface.spuKeyOffHighOffset = 0x18Eu;
    surface.spuKeyOnLowOffset = 0x188u;
    surface.spuKeyOnHighOffset = 0x18Au;
    surface.spuReverbLowOffset = 0x198u;
    surface.spuReverbHighOffset = 0x19Au;
    surface.psxSpuRegisterAuthority = false;
    return surface;
}

bool IsExactSsDriverRegisterCommitSurface80032B00(
    const SsDriverRegisterCommitSurface80032B00& surface) {
    const SsDriverRegisterCommitSurface80032B00 expected =
        BuildSsDriverRegisterCommitSurface80032B00();
    return surface.known == expected.known &&
           surface.dirtyRegisterAndMaskControlCompleteWithinLimits ==
               expected.dirtyRegisterAndMaskControlCompleteWithinLimits &&
           surface.function == expected.function &&
           surface.dirtyLoopStart == expected.dirtyLoopStart &&
           surface.dirtyLoopEnd == expected.dirtyLoopEnd &&
           surface.voiceCount == expected.voiceCount &&
           surface.voiceRegisterStride == expected.voiceRegisterStride &&
           surface.volumeDirtyMask == expected.volumeDirtyMask &&
           surface.pitchDirtyMask == expected.pitchDirtyMask &&
           surface.startAddressDirtyMask ==
               expected.startAddressDirtyMask &&
           surface.adsrDirtyMask == expected.adsrDirtyMask &&
           surface.keyOffLowAddress == expected.keyOffLowAddress &&
           surface.keyOffHighAddress == expected.keyOffHighAddress &&
           surface.keyOnLowAddress == expected.keyOnLowAddress &&
           surface.keyOnHighAddress == expected.keyOnHighAddress &&
           surface.reverbLowAddress == expected.reverbLowAddress &&
           surface.reverbHighAddress == expected.reverbHighAddress &&
           surface.spuKeyOffLowOffset == expected.spuKeyOffLowOffset &&
           surface.spuKeyOffHighOffset == expected.spuKeyOffHighOffset &&
           surface.spuKeyOnLowOffset == expected.spuKeyOnLowOffset &&
           surface.spuKeyOnHighOffset == expected.spuKeyOnHighOffset &&
           surface.spuReverbLowOffset == expected.spuReverbLowOffset &&
           surface.spuReverbHighOffset == expected.spuReverbHighOffset &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority;
}

bool TryExecuteSsDriverRegisterCommit80032B00(
    const SsDriverRegisterCommitSurface80032B00& surface,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverRegisterCommitDispatch80032B00& out) {
    out = SsDriverRegisterCommitDispatch80032B00{};
    if (!IsExactSsDriverRegisterCommitSurface80032B00(surface)) {
        return false;
    }

    out.known = true;
    out.hostVolumeProjectionReady = true;
    out.psxSpuRegisterAuthority = false;
    masks.keyOnLow &= static_cast<uint16_t>(~masks.keyOffLow);
    masks.keyOnHigh &= static_cast<uint16_t>(~masks.keyOffHigh);
    out.pendingKeyOnMaskedByKeyOff = true;

    for (uint32_t voiceIndex = 0u;
         voiceIndex < surface.voiceCount;
         ++voiceIndex) {
        SsDriverVoiceRegisterState80032B00& voice =
            voices[voiceIndex];
        SsDriverVoiceRegisterRequest80032B00& request =
            out.voiceRequests[voiceIndex];
        const uint8_t dirty = voice.dirtyFlags;
        if ((dirty & surface.volumeDirtyMask) != 0u) {
            request.writeVolume = true;
            request.volumeLeft = voice.pendingVolumeLeft;
            request.volumeRight = voice.pendingVolumeRight;
        }
        if ((dirty & surface.pitchDirtyMask) != 0u) {
            request.writePitch = true;
            request.pitch = voice.pendingPitch;
        }
        if ((dirty & surface.startAddressDirtyMask) != 0u) {
            request.writeStartAddress = true;
            request.startAddress = voice.pendingStartAddress;
        }
        if ((dirty & surface.adsrDirtyMask) != 0u) {
            request.writeAdsr = true;
            request.adsr1 = voice.pendingAdsr1;
            request.adsr2 = voice.pendingAdsr2;
        }
        if (dirty != 0u) {
            out.dirtyVoiceMask |= 1u << voiceIndex;
        }
        voice.dirtyFlags = 0u;
    }

    out.keyOffLow = masks.keyOffLow;
    out.keyOffHigh = masks.keyOffHigh;
    out.keyOnLow = masks.keyOnLow;
    out.keyOnHigh = masks.keyOnHigh;
    out.reverbLow = masks.reverbLow;
    out.reverbHigh = masks.reverbHigh;
    masks.keyOffLow = 0u;
    masks.keyOffHigh = 0u;
    masks.keyOnLow = 0u;
    masks.keyOnHigh = 0u;
    out.pendingKeyOnAndKeyOffCleared = true;
    out.reverbMasksRetained =
        masks.reverbLow == out.reverbLow &&
        masks.reverbHigh == out.reverbHigh;
    return true;
}

SsDriverInitializeSurface8003226C
BuildSsDriverInitializeSurface8003226C() {
    SsDriverInitializeSurface8003226C surface{};
    surface.known = true;
    surface.driverStateProducerCompleteWithinLimits = true;
    surface.function = 0x8003226Cu;
    surface.callerFunction = 0x8002ADE0u;
    surface.callerCall = 0x8002AE60u;
    surface.callerRequestedVoiceCount = 24u;
    surface.driverControlFunction = 0x8002EB44u;
    surface.driverControlCall = 0x8003227Cu;
    surface.driverControlAddress = 0x800555F8u;
    surface.spuInitMallocFunction = 0x80035394u;
    surface.spuInitMallocCall = 0x8003229Cu;
    surface.spuInitMallocRecordCount = 32u;
    surface.spuInitMallocTableAddress = 0x80088268u;
    surface.voiceCountAddress = 0x800928A0u;
    surface.voiceStateBaseAddress = 0x80087D40u;
    surface.voiceStateStride = 0x34u;
    surface.pendingVoiceRegisterBaseAddress = 0x80087BA8u;
    surface.pendingVoiceRegisterStride = 0x10u;
    surface.dirtyFlagsAddress = 0x80087D28u;
    surface.completionScratchBaseAddress = 0x800928F8u;
    surface.completionScratchByteCount = 16u;
    surface.lastVoiceIndexAddress = 0x800928F2u;
    surface.masterVolumeLeftAddress = 0x8008ECC8u;
    surface.masterVolumeRightAddress = 0x8008ECCAu;
    surface.completionScanDisableAddress = 0x8009290Cu;
    surface.monoFlagAddress = 0x80091728u;
    surface.driverScalarAddress = 0x800917A8u;
    surface.driverCommitFunction = 0x80032B00u;
    surface.driverCommitCall = 0x80032620u;
    surface.maxVoiceCount = 24u;
    surface.defaultPitch = 0x1000u;
    surface.defaultStartAddress = 0x0200u;
    surface.defaultAdsr1 = 0x80FFu;
    surface.defaultAdsr2 = 0x4000u;
    surface.defaultMasterVolume = 0x3FFFu;
    surface.defaultDriverScalar = 0x0080u;
    surface.psxSpuAllocatorAuthority = false;
    surface.psxSpuRegisterAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsDriverInitializeSurface8003226C(
    const SsDriverInitializeSurface8003226C& surface) {
    const SsDriverInitializeSurface8003226C expected =
        BuildSsDriverInitializeSurface8003226C();
    return surface.known == expected.known &&
           surface.driverStateProducerCompleteWithinLimits ==
               expected.driverStateProducerCompleteWithinLimits &&
           surface.function == expected.function &&
           surface.callerFunction == expected.callerFunction &&
           surface.callerCall == expected.callerCall &&
           surface.callerRequestedVoiceCount ==
               expected.callerRequestedVoiceCount &&
           surface.driverControlFunction ==
               expected.driverControlFunction &&
           surface.driverControlCall == expected.driverControlCall &&
           surface.driverControlAddress ==
               expected.driverControlAddress &&
           surface.spuInitMallocFunction ==
               expected.spuInitMallocFunction &&
           surface.spuInitMallocCall == expected.spuInitMallocCall &&
           surface.spuInitMallocRecordCount ==
               expected.spuInitMallocRecordCount &&
           surface.spuInitMallocTableAddress ==
               expected.spuInitMallocTableAddress &&
           surface.voiceCountAddress == expected.voiceCountAddress &&
           surface.voiceStateBaseAddress ==
               expected.voiceStateBaseAddress &&
           surface.voiceStateStride == expected.voiceStateStride &&
           surface.pendingVoiceRegisterBaseAddress ==
               expected.pendingVoiceRegisterBaseAddress &&
           surface.pendingVoiceRegisterStride ==
               expected.pendingVoiceRegisterStride &&
           surface.dirtyFlagsAddress == expected.dirtyFlagsAddress &&
           surface.completionScratchBaseAddress ==
               expected.completionScratchBaseAddress &&
           surface.completionScratchByteCount ==
               expected.completionScratchByteCount &&
           surface.lastVoiceIndexAddress ==
               expected.lastVoiceIndexAddress &&
           surface.masterVolumeLeftAddress ==
               expected.masterVolumeLeftAddress &&
           surface.masterVolumeRightAddress ==
               expected.masterVolumeRightAddress &&
           surface.completionScanDisableAddress ==
               expected.completionScanDisableAddress &&
           surface.monoFlagAddress == expected.monoFlagAddress &&
           surface.driverScalarAddress == expected.driverScalarAddress &&
           surface.driverCommitFunction == expected.driverCommitFunction &&
           surface.driverCommitCall == expected.driverCommitCall &&
           surface.maxVoiceCount == expected.maxVoiceCount &&
           surface.defaultPitch == expected.defaultPitch &&
           surface.defaultStartAddress == expected.defaultStartAddress &&
           surface.defaultAdsr1 == expected.defaultAdsr1 &&
           surface.defaultAdsr2 == expected.defaultAdsr2 &&
           surface.defaultMasterVolume ==
               expected.defaultMasterVolume &&
           surface.defaultDriverScalar == expected.defaultDriverScalar &&
           surface.psxSpuAllocatorAuthority ==
               expected.psxSpuAllocatorAuthority &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority == expected.stage2PlusAuthority;
}

bool TryInitializeSsDriverState8003226C(
    const SsDriverInitializeSurface8003226C& surface,
    uint8_t requestedVoiceCount,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverVolumeMix80032B00& mix,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverInitializeState8003226C& state,
    SsDriverCommitSink80032B00 driverCommit,
    void* userData,
    SsDriverInitializeDispatch8003226C& out) {
    out = SsDriverInitializeDispatch8003226C{};
    if (!IsExactSsDriverInitializeSurface8003226C(surface) ||
        driverCommit == nullptr) {
        return false;
    }

    out.known = true;
    out.requestedVoiceCount = requestedVoiceCount;
    out.hostProjection = true;
    out.psxSpuAllocatorAuthority = false;
    out.psxSpuRegisterAuthority = false;

    state.driverControl800555F8 = 1;
    out.driverControlExecuted = true;
    state.spuMallocRecordCount =
        surface.spuInitMallocRecordCount;
    state.spuMallocTableAddress =
        surface.spuInitMallocTableAddress;
    out.spuInitMallocProjected = true;
    state.word800928C8 = 0u;
    state.word80091688 = 0u;
    state.word801C35F0 = 0u;
    state.completionScratch800928F8.fill(0u);
    state.configuredVoiceCount800928A0 =
        static_cast<uint8_t>(std::min<uint32_t>(
            requestedVoiceCount, surface.maxVoiceCount));
    out.configuredVoiceCount =
        state.configuredVoiceCount800928A0;

    for (uint32_t voiceIndex = 0u;
         voiceIndex < surface.maxVoiceCount;
         ++voiceIndex) {
        SsDriverVoiceRegisterState80032B00& voice =
            voices[voiceIndex];
        voice.dirtyFlags = 0u;
        voice.pendingVolumeLeft = 0u;
        voice.pendingVolumeRight = 0u;
        voice.pendingPitch = 0u;
        voice.pendingStartAddress = 0u;
        voice.pendingAdsr1 = 0u;
        voice.pendingAdsr2 = 0u;
        out.clearedPendingRegisterVoiceMask |= 1u << voiceIndex;
        out.clearedDirtyVoiceMask |= 1u << voiceIndex;
    }

    for (uint32_t voiceIndex = 0u;
         voiceIndex < state.configuredVoiceCount800928A0;
         ++voiceIndex) {
        SsDriverVoiceOwnerState8003226C& owner =
            voiceOwners[voiceIndex];
        owner.field80087D40 = 0u;
        owner.field80087D42 = 24u;
        owner.field80087D44 = 0u;
        owner.field80087D46 = 0u;
        owner.velocity80087D48 = 0u;
        owner.pan80087D4A = 64u;
        owner.packedSequenceTrack80087D4E = -1;
        owner.toneTableProgram80087D50 = 0u;
        owner.program80087D52 = 0u;
        owner.toneIndex80087D54 = 255u;

        completionState.voiceStatus[voiceIndex] = 0u;
        SsDriverVoiceRegisterState80032B00& voice =
            voices[voiceIndex];
        voice.firstRamp = SsDriverRampState80032B00{};
        voice.secondRamp = SsDriverRampState80032B00{};
        voice.pendingPitch = surface.defaultPitch;
        voice.pendingStartAddress = surface.defaultStartAddress;
        voice.pendingAdsr1 = surface.defaultAdsr1;
        voice.pendingAdsr2 = surface.defaultAdsr2;
        state.lastVoiceIndex800928F2 =
            static_cast<uint16_t>(voiceIndex);
        out.initializedVoiceMask |= 1u << voiceIndex;
    }

    state.masterVolumeLeft8008ECC8 =
        surface.defaultMasterVolume;
    state.masterVolumeRight8008ECCA =
        surface.defaultMasterVolume;
    masks = SsDriverGlobalMaskState80032B00{};
    state.completionScanDisabled8009290C = false;
    mix.monoFlag80091728 = 0;
    state.driverScalar800917A8 = surface.defaultDriverScalar;

    out.driverCommitResult = driverCommit(userData);
    out.driverCommitExecuted = true;
    return true;
}

SsDriverResetSurface800351B8
BuildSsDriverResetSurface800351B8() {
    SsDriverResetSurface800351B8 surface{};
    surface.known = true;
    surface.resetStateProducerCompleteWithinLimits = true;
    surface.wrapperFunction = 0x80026FA4u;
    surface.wrapperCall = 0x80026FACu;
    surface.resetFunction = 0x800351B8u;
    surface.voiceCountAddress = 0x800928A0u;
    surface.voiceStateBaseAddress = 0x80087D40u;
    surface.voiceStateStride = 0x34u;
    surface.completionStatusOffset = 0x1Bu;
    surface.pendingVoiceRegisterBaseAddress = 0x80087BA8u;
    surface.pendingVoiceRegisterStride = 0x10u;
    surface.lastVoiceIndexAddress = 0x800928F2u;
    surface.keyOffLowAddress = 0x801C386Cu;
    surface.keyOffHighAddress = 0x801C386Eu;
    surface.keyOnLowAddress = 0x8008EC98u;
    surface.keyOnHighAddress = 0x8008EC9Au;
    surface.maxVoiceCount = 24u;
    surface.defaultOwnerKind = 24u;
    surface.defaultOwnerSentinel = 255u;
    surface.defaultPitch = 0x1000u;
    surface.defaultStartAddress = 0x0200u;
    surface.defaultAdsr1 = 0x80FFu;
    surface.defaultAdsr2 = 0x4000u;
    surface.callsDriverCommit = false;
    surface.psxSpuRegisterAuthority = false;
    surface.psxInterruptTimingAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsDriverResetSurface800351B8(
    const SsDriverResetSurface800351B8& surface) {
    const SsDriverResetSurface800351B8 expected =
        BuildSsDriverResetSurface800351B8();
    return surface.known == expected.known &&
           surface.resetStateProducerCompleteWithinLimits ==
               expected.resetStateProducerCompleteWithinLimits &&
           surface.wrapperFunction == expected.wrapperFunction &&
           surface.wrapperCall == expected.wrapperCall &&
           surface.resetFunction == expected.resetFunction &&
           surface.voiceCountAddress == expected.voiceCountAddress &&
           surface.voiceStateBaseAddress ==
               expected.voiceStateBaseAddress &&
           surface.voiceStateStride == expected.voiceStateStride &&
           surface.completionStatusOffset ==
               expected.completionStatusOffset &&
           surface.pendingVoiceRegisterBaseAddress ==
               expected.pendingVoiceRegisterBaseAddress &&
           surface.pendingVoiceRegisterStride ==
               expected.pendingVoiceRegisterStride &&
           surface.lastVoiceIndexAddress ==
               expected.lastVoiceIndexAddress &&
           surface.keyOffLowAddress == expected.keyOffLowAddress &&
           surface.keyOffHighAddress == expected.keyOffHighAddress &&
           surface.keyOnLowAddress == expected.keyOnLowAddress &&
           surface.keyOnHighAddress == expected.keyOnHighAddress &&
           surface.maxVoiceCount == expected.maxVoiceCount &&
           surface.defaultOwnerKind == expected.defaultOwnerKind &&
           surface.defaultOwnerSentinel ==
               expected.defaultOwnerSentinel &&
           surface.defaultPitch == expected.defaultPitch &&
           surface.defaultStartAddress == expected.defaultStartAddress &&
           surface.defaultAdsr1 == expected.defaultAdsr1 &&
           surface.defaultAdsr2 == expected.defaultAdsr2 &&
           surface.callsDriverCommit == expected.callsDriverCommit &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority &&
           surface.psxInterruptTimingAuthority ==
               expected.psxInterruptTimingAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority ==
               expected.stage2PlusAuthority;
}

bool TryExecuteSsDriverReset800351B8(
    const SsDriverResetSurface800351B8& surface,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverInitializeState8003226C& state,
    SsDriverResetDispatch800351B8& out) {
    out = SsDriverResetDispatch800351B8{};
    if (!IsExactSsDriverResetSurface800351B8(surface) ||
        state.configuredVoiceCount800928A0 > surface.maxVoiceCount) {
        return false;
    }

    out.known = true;
    out.returnedVoiceCount = state.configuredVoiceCount800928A0;
    out.psxSpuRegisterAuthority = false;
    out.psxInterruptTimingAuthority = false;
    if (state.configuredVoiceCount800928A0 == 0u) {
        out.zeroVoiceCountNoOp = true;
        out.keyOffLowAfter = masks.keyOffLow;
        out.keyOffHighAfter = masks.keyOffHigh;
        out.keyOnLowAfter = masks.keyOnLow;
        out.keyOnHighAfter = masks.keyOnHigh;
        return true;
    }

    const uint16_t reverbLowBefore = masks.reverbLow;
    const uint16_t reverbHighBefore = masks.reverbHigh;
    for (uint32_t voiceIndex = 0u;
         voiceIndex < state.configuredVoiceCount800928A0;
         ++voiceIndex) {
        SsDriverVoiceOwnerState8003226C& owner =
            voiceOwners[voiceIndex];

        // 800351B8 writes 0x00FF to +0 first, then clears +0 while
        // publishing the key-off bit for the same voice. Preserve the
        // final observable value without claiming the transient as a
        // separately schedulable host state.
        owner.field80087D40 = surface.defaultOwnerSentinel;
        owner.field80087D42 = surface.defaultOwnerKind;
        completionState.voiceStatus[voiceIndex] = 0u;
        owner.field80087D44 = 0u;
        owner.field80087D46 = 0u;
        owner.packedSequenceTrack80087D4E =
            static_cast<int16_t>(surface.defaultOwnerSentinel);
        owner.toneTableProgram80087D50 = 0u;
        owner.program80087D52 = 0u;
        owner.toneIndex80087D54 =
            surface.defaultOwnerSentinel;

        SsDriverVoiceRegisterState80032B00& voice =
            voices[voiceIndex];
        voice.pendingVolumeLeft = 0u;
        voice.pendingVolumeRight = 0u;
        voice.pendingPitch = surface.defaultPitch;
        voice.pendingStartAddress = surface.defaultStartAddress;
        voice.pendingAdsr1 = surface.defaultAdsr1;
        voice.pendingAdsr2 = surface.defaultAdsr2;

        state.lastVoiceIndex800928F2 =
            static_cast<uint16_t>(voiceIndex);
        const uint16_t bit =
            static_cast<uint16_t>(1u << (voiceIndex & 15u));
        if (voiceIndex < 16u) {
            masks.keyOffLow =
                static_cast<uint16_t>(masks.keyOffLow | bit);
            masks.keyOnLow =
                static_cast<uint16_t>(
                    masks.keyOnLow &
                    static_cast<uint16_t>(~masks.keyOffLow));
        } else {
            masks.keyOffHigh =
                static_cast<uint16_t>(masks.keyOffHigh | bit);
            masks.keyOnHigh =
                static_cast<uint16_t>(
                    masks.keyOnHigh &
                    static_cast<uint16_t>(~masks.keyOffHigh));
        }
        owner.field80087D44 = 0u;
        owner.field80087D40 = 0u;
        out.resetVoiceMask |= 1u << voiceIndex;
    }

    out.keyOffLowAfter = masks.keyOffLow;
    out.keyOffHighAfter = masks.keyOffHigh;
    out.keyOnLowAfter = masks.keyOnLow;
    out.keyOnHighAfter = masks.keyOnHigh;
    out.pendingRegistersReinitialized = true;
    out.dirtyFlagsPreserved = true;
    out.rampStatePreserved = true;
    out.untouchedOwnerFieldsPreserved = true;
    out.reverbMasksPreserved =
        masks.reverbLow == reverbLowBefore &&
        masks.reverbHigh == reverbHighBefore;
    out.driverCommitExecuted = false;
    out.hostVoiceResetProjectionReady = true;
    return true;
}

bool TryProjectSsDriverVoiceEnvelope80032B00(
    const SsDriverVoiceCompletionSurface80032B00& surface,
    const SsDriverVoiceCompletionObservation80032B00& observation,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners) {
    if (!IsExactSsDriverVoiceCompletionSurface80032B00(surface) ||
        observation.voiceCount > surface.maxVoiceCount) {
        return false;
    }

    for (uint32_t voice = 0u; voice < observation.voiceCount; ++voice) {
        voiceOwners[voice].field80087D46 =
            observation.envelopeObservations[voice];
    }
    return true;
}

SsSpuFirstAllocationSurface8002E87C
BuildSsSpuFirstAllocationSurface8002E87C() {
    SsSpuFirstAllocationSurface8002E87C surface{};
    surface.known = true;
    surface.currentScene0FirstAllocationCompleteWithinLimits = true;
    surface.scene0CallerFunction = 0x80027078u;
    surface.vabOpenWrapperFunction = 0x8002E3D8u;
    surface.vabLoaderFunction = 0x8002E474u;
    surface.spuMallocFunction = 0x8002E87Cu;
    surface.allocatorNormalizeFunction = 0x8002E0D8u;
    surface.spuInitMallocFunction = 0x80035394u;
    surface.recordTableAddress = 0x80088268u;
    surface.recordCount = 32u;
    surface.initialFreeAddress = 0x1010u;
    surface.spuRamEndAddress = 0x80000u;
    surface.initialFreeFlag = 0x40000000u;
    surface.addressMask = 0x0FFFFFFFu;
    surface.alignmentMask = 7u;
    surface.alignmentShift = 3u;
    surface.activeReserveBytes = 0u;
    surface.fullSpuAllocatorAuthority = false;
    surface.psxSpuRamTransferAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsSpuFirstAllocationSurface8002E87C(
    const SsSpuFirstAllocationSurface8002E87C& surface) {
    const SsSpuFirstAllocationSurface8002E87C expected =
        BuildSsSpuFirstAllocationSurface8002E87C();
    return surface.known == expected.known &&
           surface.currentScene0FirstAllocationCompleteWithinLimits ==
               expected.currentScene0FirstAllocationCompleteWithinLimits &&
           surface.scene0CallerFunction ==
               expected.scene0CallerFunction &&
           surface.vabOpenWrapperFunction ==
               expected.vabOpenWrapperFunction &&
           surface.vabLoaderFunction == expected.vabLoaderFunction &&
           surface.spuMallocFunction == expected.spuMallocFunction &&
           surface.allocatorNormalizeFunction ==
               expected.allocatorNormalizeFunction &&
           surface.spuInitMallocFunction ==
               expected.spuInitMallocFunction &&
           surface.recordTableAddress == expected.recordTableAddress &&
           surface.recordCount == expected.recordCount &&
           surface.initialFreeAddress == expected.initialFreeAddress &&
           surface.spuRamEndAddress == expected.spuRamEndAddress &&
           surface.initialFreeFlag == expected.initialFreeFlag &&
           surface.addressMask == expected.addressMask &&
           surface.alignmentMask == expected.alignmentMask &&
           surface.alignmentShift == expected.alignmentShift &&
           surface.activeReserveBytes == expected.activeReserveBytes &&
           surface.fullSpuAllocatorAuthority ==
               expected.fullSpuAllocatorAuthority &&
           surface.psxSpuRamTransferAuthority ==
               expected.psxSpuRamTransferAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority == expected.stage2PlusAuthority;
}

bool TryInitializeSsSpuAllocator80035394(
    const SsSpuFirstAllocationSurface8002E87C& surface,
    SsSpuFirstAllocationState8002E87C& state,
    SsSpuFirstAllocationDispatch8002E87C& out) {
    out = SsSpuFirstAllocationDispatch8002E87C{};
    if (!IsExactSsSpuFirstAllocationSurface8002E87C(surface)) {
        return false;
    }

    state = SsSpuFirstAllocationState8002E87C{};
    state.initialized = true;
    state.recordTableAddress = surface.recordTableAddress;
    state.recordCount = surface.recordCount;
    state.records[0].addressAndFlags =
        surface.initialFreeFlag | surface.initialFreeAddress;
    state.records[0].size =
        surface.spuRamEndAddress - surface.initialFreeAddress;

    out.known = true;
    out.initialized = true;
    out.remainingFreeAddress = surface.initialFreeAddress;
    out.remainingFreeBytes = state.records[0].size;
    out.fullSpuAllocatorAuthority = false;
    out.psxSpuRamTransferAuthority = false;
    return true;
}

bool TryAllocateFirstSsSpuBlock8002E87C(
    const SsSpuFirstAllocationSurface8002E87C& surface,
    uint32_t requestedBytes,
    SsSpuFirstAllocationState8002E87C& state,
    SsSpuFirstAllocationDispatch8002E87C& out) {
    out = SsSpuFirstAllocationDispatch8002E87C{};
    if (!IsExactSsSpuFirstAllocationSurface8002E87C(surface)) {
        return false;
    }

    out.known = true;
    out.initialized = state.initialized;
    out.allocationAttempted = true;
    out.requestedBytes = requestedBytes;
    out.fullSpuAllocatorAuthority = false;
    out.psxSpuRamTransferAuthority = false;
    if (!state.initialized || state.recordCount < 2u ||
        state.allocationCount != 0u || requestedBytes == 0u ||
        requestedBytes > UINT32_MAX - surface.alignmentMask) {
        return true;
    }

    out.alignedBytes =
        (requestedBytes + surface.alignmentMask) &
        ~surface.alignmentMask;
    SsSpuAllocationRecord8002E87C& freeRecord = state.records[0];
    const uint32_t freeAddress =
        freeRecord.addressAndFlags & surface.addressMask;
    if ((freeRecord.addressAndFlags & surface.initialFreeFlag) == 0u ||
        freeAddress != surface.initialFreeAddress ||
        freeRecord.size < out.alignedBytes +
            surface.activeReserveBytes) {
        return true;
    }

    const uint32_t remainingBytes =
        freeRecord.size - out.alignedBytes;
    freeRecord.addressAndFlags = freeAddress;
    freeRecord.size = out.alignedBytes;
    if (remainingBytes != 0u) {
        state.records[1].addressAndFlags =
            surface.initialFreeFlag |
            (freeAddress + out.alignedBytes);
        state.records[1].size = remainingBytes;
        state.highWaterRecordIndex = 1u;
    }
    state.allocationCount = 1u;

    out.allocationSucceeded = true;
    out.allocatedAddress = freeAddress;
    out.remainingFreeAddress = freeAddress + out.alignedBytes;
    out.remainingFreeBytes = remainingBytes;
    out.sampleStartProductionReady = true;
    return true;
}

SsVabBodyTransferSurface8002EB80
BuildSsVabBodyTransferSurface8002EB80() {
    SsVabBodyTransferSurface8002EB80 surface{};
    surface.known = true;
    surface.firstScene0VabTransferCompleteWithinLimits = true;
    surface.completionControlCompleteWithinLimits = true;
    surface.initializeFunction = 0x8003226Cu;
    surface.openFunction = 0x8002E474u;
    surface.transferWrapperFunction = 0x800270D4u;
    surface.transferFunction = 0x8002EB80u;
    surface.setTransferModeFunction = 0x8002ECDCu;
    surface.setTransferStartFunction = 0x8002ECA0u;
    surface.writeFunction = 0x8002EC40u;
    surface.transferReadyFunction = 0x8002EB44u;
    surface.completionWrapperFunction = 0x800270FCu;
    surface.completionFunction = 0x8002EEFCu;
    surface.testEventFunction = 0x8002EF28u;
    surface.vabStatusAddress = 0x800928F8u;
    surface.vabSpuBaseAddress = 0x801C35F8u;
    surface.vabTransferBytesAddress = 0x801C35B0u;
    surface.transferStartAddress = 0x800555C4u;
    surface.transferReadyAddress = 0x800555F8u;
    surface.transferModeAddress = 0x80055624u;
    surface.transferPathAddress = 0x800555E0u;
    surface.initializedVabSlotCount = 16u;
    surface.transferAcceptedStatus = 2u;
    surface.transferPendingStatus = 1u;
    surface.transferAlignmentBytes = 8u;
    surface.transferAddressShift = 3u;
    surface.maximumTransferBytes = 0x7F000u;
    surface.spuRamEndAddress = 0x80000u;
    surface.psxDmaAuthority = false;
    surface.psxEventAuthority = false;
    surface.psxSpuRamAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsVabBodyTransferSurface8002EB80(
    const SsVabBodyTransferSurface8002EB80& surface) {
    const SsVabBodyTransferSurface8002EB80 expected =
        BuildSsVabBodyTransferSurface8002EB80();
    return surface.known == expected.known &&
           surface.firstScene0VabTransferCompleteWithinLimits ==
               expected.firstScene0VabTransferCompleteWithinLimits &&
           surface.completionControlCompleteWithinLimits ==
               expected.completionControlCompleteWithinLimits &&
           surface.initializeFunction == expected.initializeFunction &&
           surface.openFunction == expected.openFunction &&
           surface.transferWrapperFunction ==
               expected.transferWrapperFunction &&
           surface.transferFunction == expected.transferFunction &&
           surface.setTransferModeFunction ==
               expected.setTransferModeFunction &&
           surface.setTransferStartFunction ==
               expected.setTransferStartFunction &&
           surface.writeFunction == expected.writeFunction &&
           surface.transferReadyFunction ==
               expected.transferReadyFunction &&
           surface.completionWrapperFunction ==
               expected.completionWrapperFunction &&
           surface.completionFunction == expected.completionFunction &&
           surface.testEventFunction == expected.testEventFunction &&
           surface.vabStatusAddress == expected.vabStatusAddress &&
           surface.vabSpuBaseAddress == expected.vabSpuBaseAddress &&
           surface.vabTransferBytesAddress ==
               expected.vabTransferBytesAddress &&
           surface.transferStartAddress == expected.transferStartAddress &&
           surface.transferReadyAddress == expected.transferReadyAddress &&
           surface.transferModeAddress == expected.transferModeAddress &&
           surface.transferPathAddress == expected.transferPathAddress &&
           surface.initializedVabSlotCount ==
               expected.initializedVabSlotCount &&
           surface.transferAcceptedStatus ==
               expected.transferAcceptedStatus &&
           surface.transferPendingStatus ==
               expected.transferPendingStatus &&
           surface.transferAlignmentBytes ==
               expected.transferAlignmentBytes &&
           surface.transferAddressShift ==
               expected.transferAddressShift &&
           surface.maximumTransferBytes ==
               expected.maximumTransferBytes &&
           surface.spuRamEndAddress == expected.spuRamEndAddress &&
           surface.psxDmaAuthority == expected.psxDmaAuthority &&
           surface.psxEventAuthority == expected.psxEventAuthority &&
           surface.psxSpuRamAuthority == expected.psxSpuRamAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority == expected.stage2PlusAuthority;
}

bool TryPrepareFirstSsVabBodyTransferState8002E474(
    const SsVabBodyTransferSurface8002EB80& surface,
    uint32_t allocatedSpuBase,
    uint32_t transferBytes,
    SsVabBodyTransferState8002EB80& state) {
    if (!IsExactSsVabBodyTransferSurface8002EB80(surface) ||
        allocatedSpuBase == 0u ||
        allocatedSpuBase >= surface.spuRamEndAddress ||
        transferBytes == 0u ||
        transferBytes >
            surface.spuRamEndAddress - allocatedSpuBase) {
        return false;
    }

    state = SsVabBodyTransferState8002EB80{};
    state.transferReady800555F8 = true;
    state.vabStatus800928F8[0] =
        static_cast<uint8_t>(surface.transferAcceptedStatus);
    state.vabSpuBase801C35F8[0] = allocatedSpuBase;
    state.vabTransferBytes801C35B0[0] = transferBytes;
    return true;
}

bool TryBeginSsVabBodyTransfer8002EB80(
    const SsVabBodyTransferSurface8002EB80& surface,
    uint16_t vabId,
    uint32_t sourceBytesAvailable,
    SsVabBodyTransferState8002EB80& state,
    SsVabBodyTransferDispatch8002EB80& out) {
    out = SsVabBodyTransferDispatch8002EB80{};
    if (!IsExactSsVabBodyTransferSurface8002EB80(surface)) {
        return false;
    }

    out.known = true;
    out.vabId = vabId;
    out.sourceBytesAvailable = sourceBytesAvailable;
    out.psxDmaCommitted = false;
    out.psxDmaAuthority = false;
    out.psxSpuRamAuthority = false;
    if (vabId >= state.vabStatus800928F8.size()) {
        state.transferReady800555F8 = true;
        return true;
    }

    out.statusBefore = state.vabStatus800928F8[vabId];
    out.statusAfter = out.statusBefore;
    if (out.statusBefore != surface.transferAcceptedStatus) {
        state.transferReady800555F8 = true;
        return true;
    }

    out.requestedTransferBytes =
        state.vabTransferBytes801C35B0[vabId];
    out.committedTransferBytes =
        std::min(out.requestedTransferBytes,
                 surface.maximumTransferBytes);
    out.spuBaseBeforeAlignment =
        state.vabSpuBase801C35F8[vabId];
    if (out.spuBaseBeforeAlignment >
        UINT32_MAX - (surface.transferAlignmentBytes - 1u)) {
        state.transferReady800555F8 = true;
        return true;
    }
    out.spuBaseAfterAlignment =
        (out.spuBaseBeforeAlignment +
         surface.transferAlignmentBytes - 1u) &
        ~(surface.transferAlignmentBytes - 1u);
    if (out.spuBaseAfterAlignment >= surface.spuRamEndAddress ||
        out.committedTransferBytes == 0u ||
        out.committedTransferBytes >
            surface.spuRamEndAddress - out.spuBaseAfterAlignment) {
        state.transferReady800555F8 = true;
        return true;
    }

    state.transferMode80055624 = 0u;
    state.alternateTransferPath800555E0 = false;
    state.transferStart800555C4 = static_cast<uint16_t>(
        out.spuBaseAfterAlignment >> surface.transferAddressShift);
    state.transferReady800555F8 = false;
    state.vabStatus800928F8[vabId] =
        static_cast<uint8_t>(surface.transferPendingStatus);

    out.accepted = true;
    out.result = static_cast<int32_t>(vabId);
    out.statusAfter = state.vabStatus800928F8[vabId];
    out.transferStartUnits = state.transferStart800555C4;
    out.sourceRangeAvailable =
        sourceBytesAvailable >= out.committedTransferBytes;
    out.hostPayloadProjectionReady = out.sourceRangeAvailable;
    return true;
}

bool TryCompleteSsVabBodyTransfer8002EEFC(
    const SsVabBodyTransferSurface8002EB80& surface,
    bool waitRequested,
    bool eventObserved,
    SsVabBodyTransferState8002EB80& state,
    SsVabTransferCompletionDispatch8002EF28& out) {
    out = SsVabTransferCompletionDispatch8002EF28{};
    if (!IsExactSsVabBodyTransferSurface8002EB80(surface)) {
        return false;
    }

    out.known = true;
    out.waitRequested = waitRequested;
    out.eventObserved = eventObserved;
    out.psxEventAuthority = false;
    out.psxDmaAuthority = false;
    if (state.transferMode80055624 == 1u ||
        state.transferReady800555F8) {
        out.result = 1;
        out.completionLatched = state.transferReady800555F8;
        return true;
    }

    out.testEventRequested = true;
    if (!eventObserved) {
        out.result = 0;
        out.wouldBlock = waitRequested;
        return true;
    }

    state.transferReady800555F8 = true;
    out.result = 1;
    out.completionLatched = true;
    return true;
}

SsCompactSfxSurface80034240 BuildSsCompactSfxSurface80034240() {
    SsCompactSfxSurface80034240 surface{};
    surface.known = true;
    surface.current80026EF8RouteCompleteWithinLimits = true;
    surface.wrapperFunction = 0x80026EF8u;
    surface.function = 0x80034240u;
    surface.validationFunction = 0x8002F13Cu;
    surface.allocatorFunction = 0x80030544u;
    surface.allocatorNoiseMaskClearFunction = 0x800353E8u;
    surface.allocatorNoiseMaskClearValue = 0x00FFFFFFu;
    surface.prepareFunction = 0x80030C90u;
    surface.noiseStartFunction = 0x80030EA4u;
    surface.pitchFunction = 0x800315C8u;
    surface.regularStartFunction = 0x800307ACu;
    surface.sourceVabIdAddress = 0x800943A8u;
    surface.resultVoiceAddress = 0x800943ACu;
    surface.reentryGuardAddress = 0x800917A4u;
    surface.programCountAddress = 0x800917A8u;
    surface.vabStatusAddress = 0x800928F8u;
    surface.programAttributeTableAddress = 0x800917D0u;
    surface.toneAttributeTableAddress = 0x800917DCu;
    surface.voiceStateBaseAddress = 0x80087D40u;
    surface.voiceStateStride = 0x34u;
    surface.cuePitchAdd = 0x18u;
    surface.acceptedVabSlotCount = 16u;
    surface.maximumProgramCount = 128u;
    surface.maximumToneSlotCount = 16u;
    surface.compactOwnerTag = 33u;
    surface.equalVolumeCurrentWrapperOnly = true;
    surface.fineAdjustZeroCurrentWrapperOnly = true;
    surface.psxVoiceIdentityAuthority = false;
    surface.psxNoiseRegisterAuthority = false;
    surface.psxSpuRegisterAuthority = false;
    surface.psxSpuTimingAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsCompactSfxSurface80034240(
    const SsCompactSfxSurface80034240& surface) {
    const SsCompactSfxSurface80034240 expected =
        BuildSsCompactSfxSurface80034240();
    return surface.known == expected.known &&
           surface.current80026EF8RouteCompleteWithinLimits ==
               expected.current80026EF8RouteCompleteWithinLimits &&
           surface.wrapperFunction == expected.wrapperFunction &&
           surface.function == expected.function &&
           surface.validationFunction == expected.validationFunction &&
           surface.allocatorFunction == expected.allocatorFunction &&
           surface.allocatorNoiseMaskClearFunction ==
               expected.allocatorNoiseMaskClearFunction &&
           surface.allocatorNoiseMaskClearValue ==
               expected.allocatorNoiseMaskClearValue &&
           surface.prepareFunction == expected.prepareFunction &&
           surface.noiseStartFunction == expected.noiseStartFunction &&
           surface.pitchFunction == expected.pitchFunction &&
           surface.regularStartFunction == expected.regularStartFunction &&
           surface.sourceVabIdAddress ==
               expected.sourceVabIdAddress &&
           surface.resultVoiceAddress == expected.resultVoiceAddress &&
           surface.reentryGuardAddress == expected.reentryGuardAddress &&
           surface.programCountAddress == expected.programCountAddress &&
           surface.vabStatusAddress == expected.vabStatusAddress &&
           surface.programAttributeTableAddress ==
               expected.programAttributeTableAddress &&
           surface.toneAttributeTableAddress ==
               expected.toneAttributeTableAddress &&
           surface.voiceStateBaseAddress ==
               expected.voiceStateBaseAddress &&
           surface.voiceStateStride == expected.voiceStateStride &&
           surface.cuePitchAdd == expected.cuePitchAdd &&
           surface.acceptedVabSlotCount ==
               expected.acceptedVabSlotCount &&
           surface.maximumProgramCount ==
               expected.maximumProgramCount &&
           surface.maximumToneSlotCount ==
               expected.maximumToneSlotCount &&
           surface.compactOwnerTag == expected.compactOwnerTag &&
           surface.equalVolumeCurrentWrapperOnly ==
               expected.equalVolumeCurrentWrapperOnly &&
           surface.fineAdjustZeroCurrentWrapperOnly ==
               expected.fineAdjustZeroCurrentWrapperOnly &&
           surface.psxVoiceIdentityAuthority ==
               expected.psxVoiceIdentityAuthority &&
           surface.psxNoiseRegisterAuthority ==
               expected.psxNoiseRegisterAuthority &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority &&
           surface.psxSpuTimingAuthority ==
               expected.psxSpuTimingAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority == expected.stage2PlusAuthority;
}

SsDriverVoiceAllocationSurface80032EAC
BuildSsDriverVoiceAllocationSurface80032EAC() {
    SsDriverVoiceAllocationSurface80032EAC surface{};
    surface.known = true;
    surface.allocationAndRegisterProducerCompleteWithinKnownInputs = true;
    surface.noteOffStateProducerCompleteWithinLimits = true;
    surface.function = 0x80032EACu;
    surface.validationFunction = 0x8002F13Cu;
    surface.allocatorFunction = 0x80030544u;
    surface.prepareFunction = 0x80030C90u;
    surface.noiseStartFunction = 0x80030EA4u;
    surface.pitchFunction = 0x80031510u;
    surface.regularStartFunction = 0x800307ACu;
    surface.noteOffFunction = 0x8003349Cu;
    surface.pitchTableAddress = 0x80055DDCu;
    surface.pitchTableEntryCount = 192u;
    surface.maxVoiceCount = 24u;
    surface.maxToneLayers = 16u;
    surface.voiceStateBaseAddress = 0x80087D40u;
    surface.voiceStateStride = 0x34u;
    surface.pendingVoiceRegisterBaseAddress = 0x80087BA8u;
    surface.pendingVoiceRegisterStride = 0x10u;
    surface.completionRingBaseAddress = 0x80088224u;
    surface.completionRingEntryCount = 16u;
    surface.psxSampleStartAddressAuthority = false;
    surface.psxNoiseRegisterAuthority = false;
    surface.psxSpuRegisterAuthority = false;
    surface.psxSpuTimingAuthority = false;
    surface.dynamicReplayAuthority = false;
    surface.oldWinS0Authority = false;
    surface.stage2PlusAuthority = false;
    return surface;
}

bool IsExactSsDriverVoiceAllocationSurface80032EAC(
    const SsDriverVoiceAllocationSurface80032EAC& surface) {
    const SsDriverVoiceAllocationSurface80032EAC expected =
        BuildSsDriverVoiceAllocationSurface80032EAC();
    return surface.known == expected.known &&
           surface.allocationAndRegisterProducerCompleteWithinKnownInputs ==
               expected.allocationAndRegisterProducerCompleteWithinKnownInputs &&
           surface.noteOffStateProducerCompleteWithinLimits ==
               expected.noteOffStateProducerCompleteWithinLimits &&
           surface.function == expected.function &&
           surface.validationFunction == expected.validationFunction &&
           surface.allocatorFunction == expected.allocatorFunction &&
           surface.prepareFunction == expected.prepareFunction &&
           surface.noiseStartFunction == expected.noiseStartFunction &&
           surface.pitchFunction == expected.pitchFunction &&
           surface.regularStartFunction == expected.regularStartFunction &&
           surface.noteOffFunction == expected.noteOffFunction &&
           surface.pitchTableAddress == expected.pitchTableAddress &&
           surface.pitchTableEntryCount ==
               expected.pitchTableEntryCount &&
           surface.maxVoiceCount == expected.maxVoiceCount &&
           surface.maxToneLayers == expected.maxToneLayers &&
           surface.voiceStateBaseAddress ==
               expected.voiceStateBaseAddress &&
           surface.voiceStateStride == expected.voiceStateStride &&
           surface.pendingVoiceRegisterBaseAddress ==
               expected.pendingVoiceRegisterBaseAddress &&
           surface.pendingVoiceRegisterStride ==
               expected.pendingVoiceRegisterStride &&
           surface.completionRingBaseAddress ==
               expected.completionRingBaseAddress &&
           surface.completionRingEntryCount ==
               expected.completionRingEntryCount &&
           surface.psxSampleStartAddressAuthority ==
               expected.psxSampleStartAddressAuthority &&
           surface.psxNoiseRegisterAuthority ==
               expected.psxNoiseRegisterAuthority &&
           surface.psxSpuRegisterAuthority ==
               expected.psxSpuRegisterAuthority &&
           surface.psxSpuTimingAuthority ==
               expected.psxSpuTimingAuthority &&
           surface.dynamicReplayAuthority ==
               expected.dynamicReplayAuthority &&
           surface.oldWinS0Authority == expected.oldWinS0Authority &&
           surface.stage2PlusAuthority == expected.stage2PlusAuthority;
}

static constexpr std::array<uint16_t, 192>
    kSsPitchTable80055DDC = {{
        4096u, 4110u, 4125u, 4140u, 4155u, 4170u, 4185u, 4200u,
        4216u, 4231u, 4246u, 4261u, 4277u, 4292u, 4308u, 4323u,
        4339u, 4355u, 4371u, 4386u, 4402u, 4418u, 4434u, 4450u,
        4466u, 4482u, 4499u, 4515u, 4531u, 4548u, 4564u, 4581u,
        4597u, 4614u, 4630u, 4647u, 4664u, 4681u, 4698u, 4715u,
        4732u, 4749u, 4766u, 4783u, 4801u, 4818u, 4835u, 4853u,
        4870u, 4888u, 4906u, 4924u, 4941u, 4959u, 4977u, 4995u,
        5013u, 5031u, 5050u, 5068u, 5086u, 5105u, 5123u, 5142u,
        5160u, 5179u, 5198u, 5216u, 5235u, 5254u, 5273u, 5292u,
        5311u, 5331u, 5350u, 5369u, 5389u, 5408u, 5428u, 5447u,
        5467u, 5487u, 5507u, 5527u, 5547u, 5567u, 5587u, 5607u,
        5627u, 5648u, 5668u, 5688u, 5709u, 5730u, 5750u, 5771u,
        5792u, 5813u, 5834u, 5855u, 5876u, 5898u, 5919u, 5940u,
        5962u, 5983u, 6005u, 6027u, 6049u, 6070u, 6092u, 6114u,
        6137u, 6159u, 6181u, 6203u, 6226u, 6248u, 6271u, 6294u,
        6316u, 6339u, 6362u, 6385u, 6408u, 6431u, 6455u, 6478u,
        6501u, 6525u, 6549u, 6572u, 6596u, 6620u, 6644u, 6668u,
        6692u, 6716u, 6741u, 6765u, 6789u, 6814u, 6839u, 6863u,
        6888u, 6913u, 6938u, 6963u, 6988u, 7014u, 7039u, 7064u,
        7090u, 7116u, 7141u, 7167u, 7193u, 7219u, 7245u, 7271u,
        7298u, 7324u, 7351u, 7377u, 7404u, 7431u, 7458u, 7485u,
        7512u, 7539u, 7566u, 7593u, 7621u, 7648u, 7676u, 7704u,
        7732u, 7760u, 7788u, 7816u, 7844u, 7873u, 7901u, 7930u,
        7958u, 7987u, 8016u, 8045u, 8074u, 8103u, 8133u, 8162u,
    }};

static uint16_t ComputeSsPitch80031510(
    uint8_t note,
    uint8_t centerNote,
    uint8_t centerFine) {
    const int32_t noteDistance =
        static_cast<int32_t>(note) + 60 -
        static_cast<int32_t>(centerNote);
    if (noteDistance < 0) {
        return 0u;
    }
    const uint32_t fineIndex = std::min<uint32_t>(
        static_cast<uint32_t>(centerFine) >> 3u, 15u);
    const int32_t octave = noteDistance / 12 - 5;
    const uint32_t semitone =
        static_cast<uint32_t>(noteDistance % 12);
    uint32_t pitch =
        kSsPitchTable80055DDC[semitone * 16u + fineIndex];
    if (octave > 0) {
        pitch <<= static_cast<uint32_t>(octave);
    } else if (octave < 0) {
        pitch >>= static_cast<uint32_t>(-octave);
    }
    return static_cast<uint16_t>(pitch);
}

static void ApplySsPan80032EAC(
    uint8_t pan,
    uint32_t& left,
    uint32_t& right) {
    if (pan >= 0x40u) {
        left = left * (127u - pan) / 0x3Fu;
    } else {
        right = right * pan / 0x3Fu;
    }
}

static void ComputeRegularSsVolumes800307AC(
    const SsDriverNoteOnRequest80032EAC& request,
    const SsDriverToneAttributes80032EAC& tone,
    uint16_t effectiveVelocity,
    bool mono,
    uint16_t& outLeft,
    uint16_t& outRight) {
    uint32_t volume =
        static_cast<uint32_t>(effectiveVelocity) * 0x3FFFu *
        request.vabMasterVolume / 16129u;
    volume =
        volume * request.programAttributes.volume * tone.volume /
        16129u;
    uint32_t left = volume;
    uint32_t right = volume;
    if (request.packedSequenceTrack != 33u) {
        left = volume * request.trackVolumeLeft / 0x7Fu;
        right = volume * request.trackVolumeRight / 0x7Fu;
    }
    ApplySsPan80032EAC(tone.pan, left, right);
    ApplySsPan80032EAC(
        request.programAttributes.pan, left, right);
    ApplySsPan80032EAC(request.channelPan, left, right);
    if (mono) {
        left = right = std::max(left, right);
    }
    outLeft = static_cast<uint16_t>(
        left * left / 0x3FFFu);
    outRight = static_cast<uint16_t>(
        right * right / 0x3FFFu);
}

static void ComputeNoiseSsVolumes80030EA4(
    const SsDriverNoteOnRequest80032EAC& request,
    const SsDriverToneAttributes80032EAC& tone,
    bool mono,
    uint16_t& outLeft,
    uint16_t& outRight) {
    const uint32_t leftWide =
        129u * request.trackVolumeLeft *
        request.programAttributes.volume;
    uint32_t left =
        leftWide / 0x7Fu * tone.volume / 0x7Fu;
    uint32_t right =
        129u * request.trackVolumeRight *
        request.programAttributes.volume / 0x7Fu *
        tone.volume / 0x7Fu;
    ApplySsPan80032EAC(tone.pan, left, right);
    ApplySsPan80032EAC(
        request.programAttributes.pan, left, right);
    ApplySsPan80032EAC(request.channelPan, left, right);
    if (mono) {
        left = right = std::max(left, right);
    }
    outLeft = static_cast<uint16_t>(left);
    outRight = static_cast<uint16_t>(right);
}

static uint8_t AllocateSsVoice80030544(
    uint8_t configuredVoiceCount,
    uint8_t newPriority,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    const SsDriverVoiceCompletionState80032B00& completionState) {
    uint8_t freeVoice = 99u;
    uint16_t bestField46 = 0xFFFFu;
    uint16_t bestAge = 0u;
    uint8_t candidate = 99u;
    uint8_t candidateCount = 0u;
    uint16_t priorityThreshold = newPriority;

    for (uint8_t voice = 0u;
         voice < configuredVoiceCount;
         ++voice) {
        const SsDriverVoiceOwnerState8003226C& owner =
            voiceOwners[voice];
        if (completionState.voiceStatus[voice] == 0u &&
            owner.field80087D46 == 0u) {
            freeVoice = voice;
            break;
        }
        if (owner.priority80087D58 < priorityThreshold) {
            priorityThreshold = owner.priority80087D58;
            candidate = voice;
            bestField46 = owner.field80087D46;
            bestAge = owner.field80087D42;
            candidateCount = 1u;
        } else if (owner.priority80087D58 == priorityThreshold) {
            ++candidateCount;
            if (owner.field80087D46 < bestField46 ||
                (owner.field80087D46 == bestField46 &&
                 owner.field80087D42 > bestAge)) {
                candidate = voice;
                bestField46 = owner.field80087D46;
                bestAge = owner.field80087D42;
            }
        }
    }

    if (freeVoice != 99u) {
        return freeVoice;
    }
    return candidateCount != 0u ? candidate : configuredVoiceCount;
}

static void SetSsVoiceMaskBit80032EAC(
    uint8_t voice,
    uint16_t& low,
    uint16_t& high) {
    if (voice < 16u) {
        low |= static_cast<uint16_t>(1u << voice);
    } else {
        high |= static_cast<uint16_t>(1u << (voice - 16u));
    }
}

static void ClearSsVoiceMaskBit80032EAC(
    uint8_t voice,
    uint16_t& low,
    uint16_t& high) {
    if (voice < 16u) {
        low &= static_cast<uint16_t>(~(1u << voice));
    } else {
        high &= static_cast<uint16_t>(~(1u << (voice - 16u)));
    }
}

bool TryExecuteSsCompactSfxCommand80026EF8(
    const SsCompactSfxSurface80034240& surface,
    const SsCompactSfxRequest80034240& request,
    SsDriverFlushState80026ECC& reentryState,
    SsDriverInitializeState8003226C& initializeState,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverVolumeMix80032B00& mix,
    SsDriverVolumeScratch80032B00& scratch,
    SsDriverGlobalMaskState80032B00& masks,
    SsCompactSfxDispatch80034240& out) {
    out = SsCompactSfxDispatch80034240{};
    if (!IsExactSsCompactSfxSurface80034240(surface)) {
        return false;
    }

    out.known = true;
    out.commandBefore = request.command;
    out.commandAfter = request.command;
    out.commandAfter[2] = static_cast<uint8_t>(
        out.commandAfter[1] + surface.cuePitchAdd);
    out.commandMutated80026EF8 = true;
    out.psxVoiceIdentityAuthority = false;
    out.psxNoiseRegisterAuthority = false;
    out.psxSpuRegisterAuthority = false;
    out.psxSpuTimingAuthority = false;
    if (reentryState.reentryGuard800917A4 == 1) {
        out.reentrySkipped = true;
        out.result = -1;
        return true;
    }

    reentryState.reentryGuard800917A4 = 1;
    out.reentryGuardSet = true;
    const auto clearReentryGuard = [&]() {
        reentryState.reentryGuard800917A4 = 0;
        out.reentryGuardCleared = true;
    };

    const uint8_t program = out.commandAfter[0];
    const uint8_t toneSlot = out.commandAfter[1];
    const uint8_t pitchKey = out.commandAfter[2];
    const uint8_t volume = out.commandAfter[3];
    const uint32_t expectedToneIndex =
        static_cast<uint32_t>(
            request.programAttributes.toneTableProgram) *
            surface.maximumToneSlotCount +
        toneSlot;
    out.sampleId = request.tone.sampleId;
    out.noiseSample = out.sampleId == 255u;
    out.sampleStartAddressKnown =
        request.tone.sampleStartAddressKnown;
    if (!request.bankReady || request.sourceVabId < 0 ||
        request.sourceVabId >=
            static_cast<int16_t>(surface.acceptedVabSlotCount) ||
        program >= surface.maximumProgramCount ||
        toneSlot >= surface.maximumToneSlotCount ||
        !request.programAttributes.valid || !request.tone.valid ||
        request.tone.toneSlot != toneSlot ||
        request.tone.toneIndex != expectedToneIndex ||
        request.tone.sampleId == 0u) {
        out.validationRejected = true;
        clearReentryGuard();
        return true;
    }

    mix.masterVolume = request.vabMasterVolume;
    mix.mix800928E2 = request.programAttributes.volume;
    mix.pan800928E3 = request.programAttributes.pan;
    mix.mix800928E5 = request.tone.volume;
    mix.pan800928E6 = request.tone.pan;
    scratch.firstCurrent800928DC = volume;
    scratch.secondCurrent800928DD = 0x40u;

    // 80034240 publishes its compact-command scratch before allocation and
    // before entering 80030C90/800307AC/80030EA4.  Keep the exact byte/word
    // values on the dispatch so later software owners can consume the same
    // transient state without relying on a host-only pointer.
    out.byte800928DA = pitchKey;
    out.byte800928DB = 0u;
    // Program byte +0 is the tone-count used by 80034240's bounded tone
    // scan; it is not the program mode byte at +3.
    out.byte800928D8 = request.programAttributes.toneCount;
    out.byte800928DF = request.programAttributes.toneTableProgram;
    out.byte800928E4 = toneSlot;
    out.byte800928E5 = request.tone.volume;
    out.byte800928E6 = request.tone.pan;
    out.byte800928E7 = request.tone.priority;
    out.byte800928E8 = request.tone.centerNote;
    out.byte800928E9 = request.tone.centerFine;
    out.byte800928EA = request.tone.noteMin;
    out.byte800928EB = request.tone.noteMax;
    out.byte800928EC = request.tone.mode;
    out.word800928F0 = request.tone.sampleId;
    out.transientScratchCommitted = true;

    const uint8_t configuredVoiceCount =
        initializeState.configuredVoiceCount800928A0;
    const uint8_t selectedVoice = AllocateSsVoice80030544(
        configuredVoiceCount,
        request.tone.priority,
        voiceOwners,
        completionState);
    out.selectedVoice = selectedVoice;
    if (selectedVoice >= configuredVoiceCount) {
        out.allocationFailed = true;
        clearReentryGuard();
        return true;
    }

    const uint8_t statusBefore =
        completionState.voiceStatus[selectedVoice];
    out.replacedActiveVoice =
        statusBefore != 0u ||
        voiceOwners[selectedVoice].field80087D46 != 0u;
    out.replacedNoiseVoice = statusBefore == 2u;
    if (out.replacedNoiseVoice) {
        // 80030544 clears the prior noise-mask owner before the selected
        // voice is republished as regular/noise state.  Retain the exact
        // lower request without claiming physical SPU-register authority.
        out.allocatorNoiseMaskClearCallCount = 1u;
        out.allocatorNoiseMaskClearValue =
            surface.allocatorNoiseMaskClearValue;
    }
    for (uint8_t voice = 0u;
         voice < configuredVoiceCount;
         ++voice) {
        voiceOwners[voice].field80087D42 =
            static_cast<uint16_t>(
                voiceOwners[voice].field80087D42 + 1u);
    }

    initializeState.lastVoiceIndex800928F2 = selectedVoice;
    // Exact 80030C90 scratch side effects precede its pending-register
    // writes.  Keep both values in the translated owner so later commit and
    // diagnostics can observe the same voice/tone selection without using
    // a host-only index.
    initializeState.word800928F4 = static_cast<uint16_t>(
        8u * static_cast<uint16_t>(selectedVoice));
    initializeState.word800928F6 = static_cast<uint16_t>(expectedToneIndex);
    out.word800928F4 = initializeState.word800928F4;
    out.word800928F6 = initializeState.word800928F6;
    SsDriverVoiceOwnerState8003226C& owner =
        voiceOwners[selectedVoice];
    owner.field80087D40 = request.tone.sampleId;
    owner.field80087D42 = 0u;
    owner.packedSequenceTrack80087D4E =
        static_cast<int16_t>(surface.compactOwnerTag);
    owner.toneTableProgram80087D50 =
        request.programAttributes.toneTableProgram;
    owner.program80087D52 = program;
    owner.toneIndex80087D54 = toneSlot;
    owner.sourceVabId80087D56 =
        static_cast<uint16_t>(request.sourceVabId);
    owner.priority80087D58 = request.tone.priority;
    owner.note80087D4C = pitchKey;
    owner.field80087D46 = 0x7FFFu;
    completionState.voiceStatus[selectedVoice] = 1u;
    for (uint32_t ring = 0u;
         ring < completionState.completionRing.size();
         ++ring) {
        completionState.completionRing[ring] &=
            ~(1u << selectedVoice);
    }

    SsDriverVoiceRegisterState80032B00& voiceState =
        voices[selectedVoice];
    voiceState.pendingStartAddress =
        request.tone.sampleStartAddressKnown
            ? request.tone.sampleStartAddress
            : 0u;
    voiceState.pendingAdsr1 = request.tone.adsr1;
    voiceState.pendingAdsr2 = static_cast<uint16_t>(
        request.tone.adsr2 + initializeState.word80091688);
    voiceState.dirtyFlags |= 0x38u;

    SsDriverNoteOnRequest80032EAC volumeRequest{};
    volumeRequest.packedSequenceTrack =
        static_cast<uint16_t>(surface.compactOwnerTag);
    volumeRequest.velocity = volume;
    volumeRequest.channelVolume = 0x7Fu;
    volumeRequest.channelPan = 0x40u;
    volumeRequest.trackVolumeLeft = volume;
    volumeRequest.trackVolumeRight = volume;
    volumeRequest.vabMasterVolume = request.vabMasterVolume;
    volumeRequest.programAttributes = request.programAttributes;

    uint16_t volumeLeft = 0u;
    uint16_t volumeRight = 0u;
    if (out.noiseSample) {
        ComputeNoiseSsVolumes80030EA4(
            volumeRequest,
            request.tone,
            mix.monoFlag80091728 == 1,
            volumeLeft,
            volumeRight);
        for (uint8_t voice = 0u;
             voice < configuredVoiceCount;
             ++voice) {
            completionState.voiceStatus[voice] &= 1u;
        }
        owner.field80087D44 = 10u;
        completionState.voiceStatus[selectedVoice] = 2u;
    } else {
        out.pitch = ComputeSsPitch80031510(
            pitchKey,
            request.tone.centerNote,
            request.tone.centerFine);
        ComputeRegularSsVolumes800307AC(
            volumeRequest,
            request.tone,
            volume,
            mix.monoFlag80091728 == 1,
            volumeLeft,
            volumeRight);
        voiceState.pendingPitch = out.pitch;
        voiceState.dirtyFlags |= 0x07u;
        owner.field80087D44 = out.pitch;
        completionState.voiceStatus[selectedVoice] = 1u;
        out.hostPlaybackReady =
            out.pitch != 0u &&
            request.tone.sampleStartAddressKnown;
    }
    voiceState.pendingVolumeLeft = volumeLeft;
    voiceState.pendingVolumeRight = volumeRight;
    voiceState.dirtyFlags |= 0x03u;
    out.pendingVolumeLeft = volumeLeft;
    out.pendingVolumeRight = volumeRight;
    out.dirtyFlagsAfter = voiceState.dirtyFlags;

    SetSsVoiceMaskBit80032EAC(
        selectedVoice, masks.keyOnLow, masks.keyOnHigh);
    // 800307AC/80030EA4 clear key-off with the complete accumulated key-on
    // mask, not only the newly selected voice bit.
    masks.keyOffLow &= static_cast<uint16_t>(~masks.keyOnLow);
    masks.keyOffHigh &= static_cast<uint16_t>(~masks.keyOnHigh);
    if ((request.tone.mode & 4u) != 0u) {
        SetSsVoiceMaskBit80032EAC(
            selectedVoice, masks.reverbLow, masks.reverbHigh);
    } else {
        ClearSsVoiceMaskBit80032EAC(
            selectedVoice, masks.reverbLow, masks.reverbHigh);
    }

    out.result = selectedVoice;
    clearReentryGuard();
    return true;
}

bool TryExecuteSsDriverNoteOn80032EAC(
    const SsDriverVoiceAllocationSurface80032EAC& surface,
    const SsDriverNoteOnRequest80032EAC& request,
    SsDriverInitializeState8003226C& initializeState,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    std::array<SsDriverVoiceRegisterState80032B00, 24>& voices,
    SsDriverVolumeMix80032B00& mix,
    SsDriverVolumeScratch80032B00& scratch,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverNoteOnDispatch80032EAC& out) {
    out = SsDriverNoteOnDispatch80032EAC{};
    if (!IsExactSsDriverVoiceAllocationSurface80032EAC(surface) ||
        request.matchedToneCount > surface.maxToneLayers) {
        return false;
    }

    out.known = true;
    out.configuredVoiceCount =
        initializeState.configuredVoiceCount800928A0;
    out.matchedToneCount = request.matchedToneCount;
    out.psxSampleStartAddressAuthority = false;
    out.psxNoiseRegisterAuthority = false;
    out.psxSpuRegisterAuthority = false;
    out.psxSpuTimingAuthority = false;
    if (!request.bankReady || request.sourceVabId < 0 ||
        request.sourceVabId >= 16 || request.program >= 128u ||
        request.velocity == 0u || !request.programAttributes.valid ||
        request.programAttributes.toneCount > surface.maxToneLayers ||
        request.matchedToneCount >
            request.programAttributes.toneCount) {
        out.validationRejected = true;
        out.result = -1;
        return true;
    }

    out.hostProjectionReady = true;
    out.result = 0;
    const uint16_t effectiveVelocity =
        request.packedSequenceTrack == 33u
            ? request.velocity
            : static_cast<uint16_t>(
                  static_cast<uint32_t>(request.velocity) *
                  request.channelVolume / 0x7Fu);

    for (uint8_t layerIndex = 0u;
         layerIndex < request.matchedToneCount;
         ++layerIndex) {
        const SsDriverToneAttributes80032EAC& tone =
            request.tones[layerIndex];
        SsDriverToneLayerDispatch80032EAC& layer =
            out.layers[layerIndex];
        layer.matched = tone.valid;
        layer.toneSlot = tone.toneSlot;
        layer.toneIndex = tone.toneIndex;
        layer.sampleId =
            static_cast<uint8_t>(tone.sampleId);
        layer.noiseSample = layer.sampleId == 255u;
        layer.effectiveVelocity = effectiveVelocity;
        layer.sampleStartAddressKnown =
            tone.sampleStartAddressKnown;
        if (!tone.valid || request.note < tone.noteMin ||
            request.note > tone.noteMax) {
            layer.allocationFailed = true;
            out.result = -1;
            continue;
        }

        const uint8_t selectedVoice = AllocateSsVoice80030544(
            out.configuredVoiceCount,
            tone.priority,
            voiceOwners,
            completionState);
        layer.selectedVoice = selectedVoice;
        if (selectedVoice >= out.configuredVoiceCount) {
            layer.allocationFailed = true;
            out.result = -1;
            continue;
        }

        const uint8_t statusBefore =
            completionState.voiceStatus[selectedVoice];
        layer.replacedActiveVoice =
            statusBefore != 0u ||
            voiceOwners[selectedVoice].field80087D46 != 0u;
        layer.replacedNoiseVoice = statusBefore == 2u;
        if (layer.replacedActiveVoice) {
            out.replacedVoiceMask |= 1u << selectedVoice;
        }
        if (layer.replacedNoiseVoice) {
            ++out.noiseMaskClearRequestCount;
        }

        for (uint8_t voice = 0u;
             voice < out.configuredVoiceCount;
             ++voice) {
            voiceOwners[voice].field80087D42 =
                static_cast<uint16_t>(
                    voiceOwners[voice].field80087D42 + 1u);
        }

        SsDriverVoiceOwnerState8003226C& owner =
            voiceOwners[selectedVoice];
        initializeState.lastVoiceIndex800928F2 = selectedVoice;
        // 80030C90 derives the pending-bank index from the selected voice
        // and the dense tone-table index for every 80032EAC layer.
        initializeState.word800928F4 = static_cast<uint16_t>(
            8u * static_cast<uint16_t>(selectedVoice));
        initializeState.word800928F6 = tone.toneIndex;
        owner.field80087D42 = 0u;
        owner.priority80087D58 = tone.priority;
        completionState.voiceStatus[selectedVoice] = 1u;
        owner.packedSequenceTrack80087D4E =
            static_cast<int16_t>(request.packedSequenceTrack);
        owner.sourceVabId80087D56 =
            static_cast<uint16_t>(request.sourceVabId);
        owner.toneTableProgram80087D50 =
            request.programAttributes.toneTableProgram;
        owner.program80087D52 = request.program;
        if (request.packedSequenceTrack != 33u) {
            owner.velocity80087D48 = request.velocity;
        }
        owner.pan80087D4A = request.channelPan;
        owner.toneIndex80087D54 = tone.toneSlot;
        owner.note80087D4C = request.note;
        owner.priority80087D58 = tone.priority;
        owner.field80087D40 = tone.sampleId;

        owner.field80087D46 = 0x7FFFu;
        for (uint32_t ring = 0u;
             ring < surface.completionRingEntryCount;
             ++ring) {
            completionState.completionRing[ring] &=
                ~(1u << selectedVoice);
        }
        SsDriverVoiceRegisterState80032B00& voiceState =
            voices[selectedVoice];
        voiceState.pendingStartAddress =
            tone.sampleStartAddressKnown
                ? tone.sampleStartAddress
                : 0u;
        voiceState.pendingAdsr1 = tone.adsr1;
        voiceState.pendingAdsr2 = static_cast<uint16_t>(
            tone.adsr2 + initializeState.word80091688);
        voiceState.dirtyFlags |= 0x38u;

        mix.masterVolume = request.vabMasterVolume;
        mix.mix800928E2 = request.programAttributes.volume;
        mix.pan800928E3 = request.programAttributes.pan;
        mix.mix800928E5 = tone.volume;
        mix.pan800928E6 = tone.pan;
        scratch.firstCurrent800928DC =
            static_cast<uint8_t>(effectiveVelocity);
        scratch.secondCurrent800928DD = request.channelPan;

        uint16_t volumeLeft = 0u;
        uint16_t volumeRight = 0u;
        if (layer.noiseSample) {
            ComputeNoiseSsVolumes80030EA4(
                request,
                tone,
                mix.monoFlag80091728 == 1,
                volumeLeft,
                volumeRight);
            for (uint8_t voice = 0u;
                 voice < out.configuredVoiceCount;
                 ++voice) {
                completionState.voiceStatus[voice] &= 1u;
            }
            owner.field80087D44 = 10u;
            completionState.voiceStatus[selectedVoice] = 2u;
        } else {
            layer.pitch = ComputeSsPitch80031510(
                request.note, tone.centerNote, tone.centerFine);
            ComputeRegularSsVolumes800307AC(
                request,
                tone,
                effectiveVelocity,
                mix.monoFlag80091728 == 1,
                volumeLeft,
                volumeRight);
            voiceState.pendingPitch = layer.pitch;
            voiceState.dirtyFlags |= 0x07u;
            owner.field80087D44 = layer.pitch;
            completionState.voiceStatus[selectedVoice] = 1u;
            layer.hostPlaybackReady = layer.pitch != 0u;
        }
        voiceState.pendingVolumeLeft = volumeLeft;
        voiceState.pendingVolumeRight = volumeRight;
        voiceState.dirtyFlags |= 0x03u;
        layer.pendingVolumeLeft = volumeLeft;
        layer.pendingVolumeRight = volumeRight;
        layer.dirtyFlagsAfter = voiceState.dirtyFlags;

        SetSsVoiceMaskBit80032EAC(
            selectedVoice, masks.keyOnLow, masks.keyOnHigh);
        masks.keyOffLow &= static_cast<uint16_t>(~masks.keyOnLow);
        masks.keyOffHigh &= static_cast<uint16_t>(~masks.keyOnHigh);
        if ((tone.mode & 4u) != 0u) {
            SetSsVoiceMaskBit80032EAC(
                selectedVoice, masks.reverbLow, masks.reverbHigh);
        } else {
            ClearSsVoiceMaskBit80032EAC(
                selectedVoice, masks.reverbLow, masks.reverbHigh);
        }

        ++out.allocatedToneCount;
        out.allocatedVoiceMask |= 1u << selectedVoice;
        out.result |= static_cast<int32_t>(1u << selectedVoice);
    }
    return true;
}

bool TryExecuteSsDriverNoteOff8003349C(
    const SsDriverVoiceAllocationSurface80032EAC& surface,
    uint16_t packedSequenceTrack,
    int16_t sourceVabId,
    uint8_t program,
    uint8_t note,
    SsDriverInitializeState8003226C& initializeState,
    std::array<SsDriverVoiceOwnerState8003226C, 24>& voiceOwners,
    SsDriverVoiceCompletionState80032B00& completionState,
    SsDriverGlobalMaskState80032B00& masks,
    SsDriverNoteOffDispatch8003349C& out) {
    out = SsDriverNoteOffDispatch8003349C{};
    if (!IsExactSsDriverVoiceAllocationSurface80032EAC(surface)) {
        return false;
    }

    out.known = true;
    out.hostProjectionReady = true;
    out.psxNoiseRegisterAuthority = false;
    out.psxSpuRegisterAuthority = false;
    const uint8_t configuredVoiceCount =
        initializeState.configuredVoiceCount800928A0;
    for (uint8_t voice = 0u;
         voice < configuredVoiceCount;
         ++voice) {
        SsDriverVoiceOwnerState8003226C& owner =
            voiceOwners[voice];
        if (owner.note80087D4C != note ||
            owner.program80087D52 != program ||
            owner.packedSequenceTrack80087D4E !=
                static_cast<int16_t>(packedSequenceTrack) ||
            owner.sourceVabId80087D56 !=
                static_cast<uint16_t>(sourceVabId)) {
            continue;
        }

        ++out.matchedVoiceCount;
        out.matchedVoiceMask |= 1u << voice;
        completionState.voiceStatus[voice] = 0u;
        owner.field80087D44 = 0u;
        if (owner.field80087D40 == 255u) {
            ++out.noiseRegisterClearRequestCount;
            continue;
        }

        initializeState.lastVoiceIndex800928F2 = voice;
        owner.field80087D40 = 0u;
        SetSsVoiceMaskBit80032EAC(
            voice, masks.keyOffLow, masks.keyOffHigh);
        ClearSsVoiceMaskBit80032EAC(
            voice, masks.keyOnLow, masks.keyOnHigh);
        out.regularKeyOffVoiceMask |= 1u << voice;
    }
    return true;
}

static Transaction8001A8F0 BuildArchiveTransaction8001A8F0(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource,
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 expectedArchiveKind) {
    Transaction8001A8F0 transaction{};
    transaction.sourceIntLoadKnown = true;
    transaction.sourceArchiveKind = intLoad.archiveKind;
    const bool exactIntLoad =
        expectedArchiveKind ==
                PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Compo00
            ? PrSS0Scene0IntLoadDirect::
                  IsExactAcceptedTransaction8001AC18(intLoad)
            : expectedArchiveKind ==
                      PrSS0Scene0IntLoadDirect::
                          ArchiveKind8001AC18::PracticeYCompo &&
                  PrSS0Scene0IntLoadDirect::
                      IsExactAcceptedYCompoTransaction80015618(intLoad);
    if (!exactIntLoad || intLoad.archiveKind != expectedArchiveKind) {
        transaction.status = Status8001A8F0::IntLoadRejected;
        return transaction;
    }
    transaction.sourceLoaderCandidateKnown = true;
    const bool exactLoaderCandidate =
        expectedArchiveKind ==
                PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Compo00
            ? PrSS0Scene0IntSideEffectDirect::
                  IsExactAcceptedTransaction8001A8F0(
                      loaderCandidate, intLoad)
            : PrSS0Scene0IntSideEffectDirect::
                  IsExactAcceptedYCompoTransaction8001A8F0(
                      loaderCandidate, intLoad);
    if (!exactLoaderCandidate) {
        transaction.status = Status8001A8F0::LoaderCandidateRejected;
        return transaction;
    }
    if (!padStartComSource.staticBodyKnown ||
        !padStartComSource.cuePointerKnown ||
        padStartComSource.cuePointer80094410 == 0u ||
        padStartComSource.replayValueAuthority) {
        transaction.status = Status8001A8F0::PadStartComSourceMalformed;
        return transaction;
    }

    const auto* vabBlock = FindVabBlock(intLoad);
    if (vabBlock == nullptr || vabBlock->files !=
            PrSS0Scene0IntLoadDirect::kExpectedVabEntries8001AC18) {
        transaction.status = Status8001A8F0::VabBlockMissing;
        return transaction;
    }
    const auto* vhEntry = FindVabEntry(intLoad, vabBlock->blockIndex, 0u);
    const auto* vbEntry = FindVabEntry(intLoad, vabBlock->blockIndex, 1u);
    if (vhEntry == nullptr || vbEntry == nullptr ||
        !loaderCandidate.vabRequest.known ||
        loaderCandidate.vabRequest.vhPsxAddress == 0u ||
        loaderCandidate.vabRequest.vbPsxAddress == 0u ||
        loaderCandidate.vabRequest.vhSize != vhEntry->size ||
        loaderCandidate.vabRequest.vhName != vhEntry->name ||
        loaderCandidate.vabRequest.vbSize != vbEntry->size ||
        loaderCandidate.vabRequest.vbName != vbEntry->name ||
        !loaderCandidate.vabRequest.closeRequested80027120 ||
        !loaderCandidate.vabRequest.openRequested80027078 ||
        !loaderCandidate.vabRequest.transferRequested800270D4 ||
        !loaderCandidate.vabRequest.enableRequested800270FC) {
        transaction.status = Status8001A8F0::VabRequestMalformed;
        return transaction;
    }
    if (!IsArchiveRange8001A8F0(
            intLoad, vhEntry->dataOffset, vhEntry->size) ||
        !IsArchiveRange8001A8F0(
            intLoad, vbEntry->dataOffset, vbEntry->size)) {
        transaction.status = Status8001A8F0::ArchiveRangeInvalid;
        return transaction;
    }

    transaction.requestBound = intLoad.requestBound;
    transaction.originalDiscPayloadAuthority =
        intLoad.discImagePayloadAuthority && intLoad.archiveBytesKnown;
    transaction.vhArchiveDataOffset = vhEntry->dataOffset;
    transaction.vhPsxAddress = loaderCandidate.vabRequest.vhPsxAddress;
    transaction.vhName = vhEntry->name;
    transaction.vhBytes.assign(
        intLoad.archiveBytes.begin() + vhEntry->dataOffset,
        intLoad.archiveBytes.begin() + vhEntry->dataOffset + vhEntry->size);
    transaction.vhHash = HashBytes(transaction.vhBytes);
    transaction.vbArchiveDataOffset = vbEntry->dataOffset;
    transaction.vbPsxAddress = loaderCandidate.vabRequest.vbPsxAddress;
    transaction.vbName = vbEntry->name;
    transaction.vbBytes.assign(
        intLoad.archiveBytes.begin() + vbEntry->dataOffset,
        intLoad.archiveBytes.begin() + vbEntry->dataOffset + vbEntry->size);
    transaction.vbHash = HashBytes(transaction.vbBytes);

    transaction.decoderPreflightKnown = true;
    if (!transaction.requestBound ||
        !transaction.originalDiscPayloadAuthority ||
        transaction.vhBytes.size() < kVhHeaderBytes ||
        transaction.vbBytes.empty() || transaction.vhBytes[0] != 0x70u ||
        transaction.vhBytes[1] != 0x42u ||
        transaction.vhBytes[2] != 0x41u ||
        transaction.vhBytes[3] != 0x56u) {
        transaction.status = Status8001A8F0::DecoderPreflightRejected;
        return transaction;
    }
    transaction.decoderProgramCount = std::min<uint16_t>(
        ReadU16Le(transaction.vhBytes, 18u),
        static_cast<uint16_t>(kMaxPrograms));
    transaction.decoderDeclaredToneCount =
        ReadU16Le(transaction.vhBytes, 20u);
    transaction.decoderVagCount = std::min<uint16_t>(
        ReadU16Le(transaction.vhBytes, 22u), 256u);
    transaction.decoderToneSlotCount =
        static_cast<uint32_t>(transaction.decoderProgramCount) *
        kTonesPerProgram;
    const uint64_t vagTableOffset =
        static_cast<uint64_t>(kVhHeaderBytes) +
        static_cast<uint64_t>(kMaxPrograms) * kProgramAttributeBytes +
        static_cast<uint64_t>(transaction.decoderToneSlotCount) *
            kToneAttributeBytes;
    if (vagTableOffset > std::numeric_limits<uint32_t>::max() ||
        vagTableOffset > transaction.vhBytes.size() ||
        kVagTableBytes > transaction.vhBytes.size() - vagTableOffset) {
        transaction.status = Status8001A8F0::DecoderPreflightRejected;
        return transaction;
    }
    transaction.decoderVagTableOffset =
        static_cast<uint32_t>(vagTableOffset);
    uint32_t effectiveVagCount = transaction.decoderVagCount;
    if (effectiveVagCount < 256u) {
        uint64_t summedVbBytes = 0u;
        for (uint32_t index = 0u; index < effectiveVagCount; ++index) {
            const uint16_t sizeUnits = ReadU16Le(
                transaction.vhBytes,
                transaction.decoderVagTableOffset + index * 2u);
            summedVbBytes += static_cast<uint64_t>(sizeUnits) * 8u;
        }
        while (effectiveVagCount < 256u) {
            const uint16_t sizeUnits = ReadU16Le(
                transaction.vhBytes,
                transaction.decoderVagTableOffset +
                    effectiveVagCount * 2u);
            if (sizeUnits == 0u) {
                break;
            }
            const uint64_t sizeBytes =
                static_cast<uint64_t>(sizeUnits) * 8u;
            if (summedVbBytes > transaction.vbBytes.size() ||
                sizeBytes > transaction.vbBytes.size() - summedVbBytes) {
                break;
            }
            summedVbBytes += sizeBytes;
            ++effectiveVagCount;
        }
    }
    transaction.decoderEffectiveVagCount =
        static_cast<uint16_t>(effectiveVagCount);
    transaction.decoderPreflightReady = true;

    transaction.actionRequests[0] = {
        0u,
        PrStage1LoaderSpuHal::kFn80027120,
        PrStage1LoaderSpuHal::kFnSsVabClose8002DF80,
        0u,
        0,
        true,
        false,
    };
    transaction.actionRequests[1] = {
        1u,
        PrStage1LoaderSpuHal::kFn80027078,
        PrStage1LoaderSpuHal::kFn8002E3D8,
        transaction.vhPsxAddress,
        PrStage1LoaderSpuHal::kOpenVabAnySlotArg80027078,
        false,
        false,
    };
    transaction.actionRequests[2] = {
        2u,
        PrStage1LoaderSpuHal::kFn800270D4,
        PrStage1LoaderSpuHal::kFn8002EB80,
        transaction.vbPsxAddress,
        0,
        false,
        false,
    };
    transaction.actionRequests[3] = {
        3u,
        PrStage1LoaderSpuHal::kFn800270FC,
        PrStage1LoaderSpuHal::kFn8002EEFC,
        0u,
        1,
        false,
        false,
    };
    for (auto& action : transaction.actionRequests) {
        action.lowerResultAuthority = false;
    }
    transaction.actionSequenceKnown = true;
    transaction.padStartComRequired = true;
    transaction.padStartComSpuTransferCompletionSurface =
        BuildSpuTransferCompletionSurface8002AC20();
    transaction.padStartComSpuTransferCompletionSurfaceKnown =
        IsExactSpuTransferCompletionSurface8002AC20(
            transaction.padStartComSpuTransferCompletionSurface);
    if (!transaction.padStartComSpuTransferCompletionSurfaceKnown) {
        transaction.status = Status8001A8F0::PadStartComSourceMalformed;
        return transaction;
    }
    transaction.padStartComSsTickInterruptSurface =
        BuildSsTickInterruptSurface8002B130();
    transaction.padStartComSsTickInterruptSurfaceKnown =
        IsExactSsTickInterruptSurface8002B130(
            transaction.padStartComSsTickInterruptSurface);
    if (!transaction.padStartComSsTickInterruptSurfaceKnown) {
        transaction.status = Status8001A8F0::PadStartComSourceMalformed;
        return transaction;
    }
    transaction.padStartComSsSequenceCallbackSurface =
        BuildSsSequenceCallbackSurface8002AC20();
    transaction.padStartComSsSequenceCallbackSurfaceKnown =
        IsExactSsSequenceCallbackSurface8002AC20(
            transaction.padStartComSsSequenceCallbackSurface);
    if (!transaction.padStartComSsSequenceCallbackSurfaceKnown) {
        transaction.status = Status8001A8F0::PadStartComSourceMalformed;
        return transaction;
    }
    transaction.padStartComCalls[0] = {
        0u, kFnSsInit8002AC20, {}, 0u, false};
    transaction.padStartComCalls[1] = {
        1u, kFnSsSetTableSize8002DA78, {0x1000, 0, 0}, 1u, false};
    transaction.padStartComCalls[2] = {
        2u, kFnSsStart8002B130, {}, 0u, false};
    transaction.padStartComCalls[3] = {
        3u, kFnSsSetTickMode8002A6AC, {0x5A, 0x5A, 0}, 2u, false};
    transaction.padStartComCalls[4] = {
        4u, kFnSsSetMVol8002AA90, {0, 0, 1}, 3u, false};
    transaction.padStartComCalls[5] = {
        5u, kFnSsSetSerialAttr8002AB24, {0, 0x7F, 0x7F}, 3u, false};
    transaction.padStartComStaticBodyKnown = true;
    transaction.padStartComCuePointerBefore =
        padStartComSource.cuePointer80094410;
    transaction.padStartComGlobalState.dword_80094410 =
        padStartComSource.cuePointer80094410;
    PrStage1LoaderSpuHal::ApplyPadStartComAudioGlobalResetContract(
        transaction.padStartComGlobalState);
    transaction.padStartComCuePointerAfter =
        transaction.padStartComGlobalState.dword_80094410;
    transaction.padStartComCuePointerPreserved =
        transaction.padStartComCuePointerBefore ==
        transaction.padStartComCuePointerAfter;
    if (transaction.padStartComGlobalState.word_800943A8 != 0 ||
        transaction.padStartComGlobalState.word_800943AA != -1 ||
        transaction.padStartComGlobalState.word_800943AC != -1 ||
        transaction.padStartComGlobalState.dword_800943B4 != 0 ||
        !transaction.padStartComCuePointerPreserved) {
        transaction.status = Status8001A8F0::PadStartComSourceMalformed;
        return transaction;
    }
    transaction.padStartComGlobalWritesKnown = true;
    transaction.padStartComGlobalWritesCommitted = false;
    transaction.padStartComLowerCallsCommitted = false;
    transaction.hostVabCandidatePrepared = false;
    transaction.hostVabBankCommitted = false;
    transaction.psxVabIdAuthority = false;
    transaction.psxSpuRamAuthority = false;
    transaction.padStartComCommitted = false;
    transaction.audibleOutputAuthority = false;
    transaction.replayValueAuthority = false;
    transaction.hostFilesystemAuthority = false;
    transaction.consumerReadAuthority = false;
    transaction.oldWinS0Authority = false;
    transaction.stage2PlusAuthority = false;
    transaction.status = Status8001A8F0::Accepted;
    transaction.accepted = true;
    transaction.complete = true;
    return transaction;
}

Transaction8001A8F0 BuildTransaction8001A8F0(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    return BuildArchiveTransaction8001A8F0(
        intLoad,
        loaderCandidate,
        padStartComSource,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Compo00);
}

Transaction8001A8F0 BuildYCompoTransaction8001A8F0(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    return BuildArchiveTransaction8001A8F0(
        intLoad,
        loaderCandidate,
        padStartComSource,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::PracticeYCompo);
}

bool IsExactAcceptedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    if (!transaction.accepted || !transaction.complete ||
        transaction.status != Status8001A8F0::Accepted ||
        transaction.hostVabCandidatePrepared ||
        transaction.hostVabBankCommitted) {
        return false;
    }
    const Transaction8001A8F0 expected =
        BuildTransaction8001A8F0(
            intLoad, loaderCandidate, padStartComSource);
    return expected.accepted &&
           EqualTransaction8001A8F0(transaction, expected);
}

bool IsExactAcceptedYCompoTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    if (!transaction.accepted || !transaction.complete ||
        transaction.status != Status8001A8F0::Accepted ||
        transaction.sourceArchiveKind !=
            PrSS0Scene0IntLoadDirect::
                ArchiveKind8001AC18::PracticeYCompo ||
        transaction.padStartComGlobalWritesCommitted ||
        transaction.padStartComLowerCallsCommitted ||
        transaction.hostVabCandidatePrepared ||
        transaction.hostVabBankCommitted ||
        transaction.psxVabIdAuthority || transaction.psxSpuRamAuthority ||
        transaction.padStartComCommitted || transaction.audibleOutputAuthority) {
        return false;
    }
    const Transaction8001A8F0 expected = BuildYCompoTransaction8001A8F0(
        intLoad, loaderCandidate, padStartComSource);
    return expected.accepted &&
           EqualTransaction8001A8F0(transaction, expected);
}

bool CommitYCompoPadStartComGlobalWrites80026E4C(
    Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    if (!IsExactAcceptedYCompoTransaction8001A8F0(
            transaction,
            intLoad,
            loaderCandidate,
            padStartComSource) ||
        !IsPadStartComGlobalWritesCommitReady80026E4C(transaction)) {
        return false;
    }
    MarkPadStartComGlobalWritesCommitted80026E4C(transaction);
    return true;
}

bool IsExactYCompoPadStartComGlobalWritesCommittedTransaction80026E4C(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    Transaction8001A8F0 expected = BuildYCompoTransaction8001A8F0(
        intLoad, loaderCandidate, padStartComSource);
    if (!expected.accepted ||
        !CommitYCompoPadStartComGlobalWrites80026E4C(
            expected,
            intLoad,
            loaderCandidate,
            padStartComSource)) {
        return false;
    }
    return EqualTransaction8001A8F0(transaction, expected);
}

bool CommitPadStartComGlobalWrites80026E4C(
    Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    if (!IsExactAcceptedTransaction8001A8F0(
            transaction,
            intLoad,
            loaderCandidate,
            padStartComSource) ||
        !IsPadStartComGlobalWritesCommitReady80026E4C(transaction)) {
        return false;
    }
    MarkPadStartComGlobalWritesCommitted80026E4C(transaction);
    return true;
}

bool IsExactPadStartComGlobalWritesCommittedTransaction80026E4C(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    Transaction8001A8F0 expected = BuildTransaction8001A8F0(
        intLoad, loaderCandidate, padStartComSource);
    if (!expected.accepted ||
        !CommitPadStartComGlobalWrites80026E4C(
            expected,
            intLoad,
            loaderCandidate,
            padStartComSource)) {
        return false;
    }
    return EqualTransaction8001A8F0(transaction, expected);
}

bool CommitHostVabBank8001A8F0(Transaction8001A8F0& transaction) {
    if (!transaction.accepted || !transaction.complete ||
        transaction.status != Status8001A8F0::Accepted ||
        !transaction.requestBound ||
        !transaction.originalDiscPayloadAuthority ||
        !transaction.decoderPreflightKnown ||
        !transaction.decoderPreflightReady ||
        !transaction.actionSequenceKnown ||
        !transaction.padStartComRequired ||
        !transaction.padStartComStaticBodyKnown ||
        !transaction.padStartComGlobalWritesKnown ||
        !transaction.padStartComCuePointerPreserved ||
        !transaction.padStartComGlobalWritesCommitted ||
        transaction.padStartComLowerCallsCommitted ||
        !transaction.hostVabCandidatePrepared ||
        transaction.hostVabBankCommitted || transaction.psxVabIdAuthority ||
        transaction.psxSpuRamAuthority ||
        transaction.padStartComCommitted ||
        transaction.audibleOutputAuthority ||
        transaction.replayValueAuthority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority || transaction.oldWinS0Authority ||
        transaction.stage2PlusAuthority) {
        return false;
    }
    transaction.hostVabBankCommitted = true;
    return true;
}

bool IsExactCommittedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    Transaction8001A8F0 expected = BuildTransaction8001A8F0(
        intLoad, loaderCandidate, padStartComSource);
    if (!expected.accepted) {
        return false;
    }
    if (!CommitPadStartComGlobalWrites80026E4C(
            expected,
            intLoad,
            loaderCandidate,
            padStartComSource)) {
        return false;
    }
    expected.hostVabCandidatePrepared = true;
    if (!CommitHostVabBank8001A8F0(expected)) {
        return false;
    }
    return EqualTransaction8001A8F0(transaction, expected);
}

bool IsExactCommittedYCompoTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PadStartComSource80026E4C& padStartComSource) {
    Transaction8001A8F0 expected = BuildYCompoTransaction8001A8F0(
        intLoad, loaderCandidate, padStartComSource);
    if (!expected.accepted ||
        !CommitYCompoPadStartComGlobalWrites80026E4C(
            expected,
            intLoad,
            loaderCandidate,
            padStartComSource)) {
        return false;
    }
    expected.hostVabCandidatePrepared = true;
    if (!CommitHostVabBank8001A8F0(expected)) {
        return false;
    }
    return EqualTransaction8001A8F0(transaction, expected);
}

}  // namespace PrSS0Scene0IntSpuDirect
