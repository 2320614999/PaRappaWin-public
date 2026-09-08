#include "pr/pr_stage1_xa_cd_direct.h"
#include "pr/pr_stage1_lower_cd_producer_direct.h"

#include <cstdio>

namespace {

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,    \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

PrStage1XaCdDirectHalGetlocPFactsInput ValidGetlocFacts() {
    PrStage1XaCdDirectHalGetlocPFactsInput input{};
    input.source = PrStage1XaCdDirectHalGetlocPSource::CurrentPhysicalClock;
    input.sectorIndexKnown = true;
    input.sectorIndex = 1514u;
    input.cdGetlocPResponseKnown = true;
    input.cdGetlocPResponse = {0x39u, 0x87u, 0x17u, 0u, 0u, 0u, 0u, 0u};
    input.cdDataReadyInterruptKnown = true;
    input.cdDataReadyInterrupt = 2u;
    return input;
}

PrStage1LowerCdProducerDirect::CdInterruptSnapshotInput800359B8
ValidPendingInterruptSnapshot() {
    PrStage1LowerCdProducerDirect::CdInterruptSnapshotInput800359B8 input{};
    input.iStatPtrKnown = true;
    input.iMaskPtrKnown = true;
    input.iStatKnown = true;
    input.iStat = 0x0004u;
    input.iMaskKnown = true;
    input.iMask = 0xFFFFu;
    input.word80055FA8Known = true;
    input.word80055FA8 = 0xFFFFu;
    input.word80055F78Known = true;
    input.word80055F78 = 1u;
    input.word80055F7ABeforeKnown = true;
    input.word80055F7ABefore = 0u;
    input.word80055F7ASetWriteKnown = true;
    input.word80055F7ASetWrite = 1u;
    input.word80055F7AClearWriteKnown = true;
    input.word80055F7AClearWrite = 0u;
    input.watchdogKnown = true;
    input.dword80057010 = 0u;
    input.pendingSampleSequenceKnown = true;
    input.pendingSampleCount = 2u;
    input.iStatSamples[0] = 0x0004u;
    input.iMaskSamples[0] = 0xFFFFu;
    input.word80055FA8Samples[0] = 0xFFFFu;
    input.iStatSamples[1] = 0x0000u;
    input.iMaskSamples[1] = 0xFFFFu;
    input.word80055FA8Samples[1] = 0xFFFFu;
    input.callbackTableKnown = true;
    input.callbackTableBaseKnown = true;
    input.callbackPresent[2] = true;
    input.callbackAddressKnown[2] = true;
    input.callbackAddress[2] = 0x80039240u;
    return input;
}

PrStage1LowerCdProducerDirect::RawCdRegTransactionResult80036AF8
ValidRawEventTransaction() {
    PrStage1LowerCdProducerDirect::RawCdRegTransactionInput80036AF8 input{};
    input.registerPointers.cdReg0PtrKnown = true;
    input.registerPointers.cdReg1PtrKnown = true;
    input.registerPointers.cdReg2PtrKnown = true;
    input.registerPointers.cdReg3PtrKnown = true;
    input.registerPointers.selectorWriteKnown = true;
    input.registerPointers.selectorWriteValue = 1u;
    input.cdReg3InitialInterruptKnown = true;
    input.cdReg3InitialInterrupt = 2u;
    input.cdReg3StableInterruptKnown = true;
    input.cdReg3StableInterrupt = 2u;
    input.cdReg0StatusKnown = true;
    input.cdReg0Status = 0u;
    input.cdReg0FifoStatusSamplesKnown = true;
    input.cdReg0FifoStatusSampleCount = 1u;
    input.cdReg0FifoStatusSamples[0] = 0u;
    input.resultByteCountKnown = true;
    input.resultByteCount = 2u;
    input.resultBytesKnown = true;
    input.resultBytes[0] = 0x10u;
    input.resultBytes[1] = 0x02u;
    input.ackWritesKnown = true;
    input.priorFacts.dword80057108Known = true;
    input.priorFacts.dword80057108 = 0u;
    input.priorFacts.dword80057110Known = true;
    input.priorFacts.dword80057110 = 0u;
    input.priorFacts.byte80057119Known = true;
    input.priorFacts.byte80057119 = 0x10u;
    return PrStage1LowerCdProducerDirect::BuildRawCdRegTransactionResult80036AF8(
        input);
}

void FillValidRawEventTypedState(PrStage1XaCdDirectState& state) {
    state.rawEvent80036AF8InitialInterruptKnown = true;
    state.rawEvent80036AF8InitialInterrupt = 2u;
    state.rawEvent80036AF8StableInterruptKnown = true;
    state.rawEvent80036AF8StableInterrupt = 2u;
    state.rawEvent80036AF8CdReg0StatusKnown = true;
    state.rawEvent80036AF8CdReg0Status = 0u;
    state.rawEvent80036AF8FifoStatusSamplesKnown = true;
    state.rawEvent80036AF8FifoStatusSampleCount = 1u;
    state.rawEvent80036AF8FifoStatusSamples[0] = 0u;
    state.rawEvent80036AF8ResultByteCountKnown = true;
    state.rawEvent80036AF8ResultByteCount = 2u;
    state.rawEvent80036AF8ResultBytesKnown = true;
    state.rawEvent80036AF8ResultBytes[0] = 0x10u;
    state.rawEvent80036AF8ResultBytes[1] = 0x02u;
    state.rawEvent80036AF8AckWritesKnown = true;
    state.dword_80057108Known = true;
    state.dword_80057108 = 0u;
    state.dword_80057110Known = true;
    state.dword_80057110 = 0u;
    state.byte_80057119Known = true;
    state.byte_80057119 = 0x10u;
}

void TestGetlocFactsDoNotSynthesizePendingProducer() {
    PrStage1XaCdDirectState state{};

    const PrStage1XaCdDirectHalGetlocPFactsResult result =
        PrStage1XaCdDirectApplyHalGetlocPFacts(state, ValidGetlocFacts());

    CHECK(result.called);
    CHECK(result.applied);
    CHECK(!result.incomplete);
    CHECK(state.halGetlocPFactsApplyCount == 1u);
    CHECK(state.halGetlocLowerBridgeCount == 1u);
    CHECK(state.halGetlocLowerBridgeCdSyncCoreCount == 1u);
    CHECK(state.halGetlocLowerBridgePendingProducerCount == 0u);
    CHECK(state.halGetlocLowerBridgeCallbackEventCount == 0u);
    CHECK(state.lowerCdSnapshotApplyCount == 1u);
    CHECK(state.lowerCdSnapshotPendingProducerCount == 0u);
    CHECK(state.lowerCdSnapshotCallbackEventCount == 0u);
    CHECK(state.lowerCdSnapshotSyncFeedbackCount == 1u);
    CHECK(state.cdLowerFeedback80036AF8Known);
    CHECK(state.cdLowerFeedback80036AF8FromGetlocP);
    CHECK(state.cdCallbackPending800359B8WriteCount == 0u);
    CHECK(state.cdCallbackPending800359B8AckCount == 0u);
    CHECK(state.cdCallbackPending800359B8CallbackDispatchCount == 0u);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.cdLowerEventPsxReturn80036AF8Known);
    CHECK(!state.dword_80057108Known);
}

void TestPendingProducerRequiresExplicitInterruptSnapshot() {
    PrStage1XaCdDirectLowerCdSnapshotBridgeInput bridge{};
    bridge.interruptSnapshot800359B8Known = true;
    bridge.interruptSnapshot800359B8 = ValidPendingInterruptSnapshot();

    const PrStage1XaCdDirectLowerCdSnapshotBridgeResult snapshot =
        PrStage1XaCdDirectBuildLowerCdProducerSnapshot(bridge);

    CHECK(snapshot.produced);
    CHECK(!snapshot.incomplete);
    CHECK(snapshot.pendingProducer800359B8Bridged);
    CHECK(!snapshot.cdSyncCoreFacts80037070Bridged);
    CHECK(snapshot.snapshot.pendingProducer800359B8Known);
    CHECK(!snapshot.snapshot.cdSyncFeedback80037070Known);

    PrStage1XaCdDirectState state{};
    const PrStage1XaCdDirectLowerCdProducerResult applied =
        PrStage1XaCdDirectApplyLowerCdProducerSnapshot(state, snapshot.snapshot);

    CHECK(applied.called);
    CHECK(applied.pendingProducer800359B8Applied);
    CHECK(state.lowerCdSnapshotPendingProducerCount == 1u);
    CHECK(state.lowerCdSnapshotSyncFeedbackCount == 0u);
    CHECK(state.cdCallbackPending800359B8WriteCount == 2u);
    CHECK(state.cdCallbackPending800359B8AckCount == 1u);
    CHECK(state.cdCallbackPending800359B8CallbackDispatchCount == 1u);
    CHECK(state.cdCallbackPending800359B8InterruptStatusKnown);
    CHECK(state.cdCallbackPending800359B8InterruptStatus == 0x0004u);
    CHECK(state.cdCallbackPending800359B8InterruptMaskKnown);
    CHECK(state.cdCallbackPending800359B8InterruptMask == 0xFFFFu);
    CHECK(state.word_80055F7AKnown);
    CHECK(state.word_80055F7A == 0u);
}

void TestRawEventRuntimeSourceFailsClosedBeforeBridge() {
    PrStage1XaCdDirectState state{};

    PrStage1XaCdDirectRawEventRuntimeSource80036AF8 missingSource{};
    const PrStage1XaCdDirectRawEventRuntimeSourceResult80036AF8
        missingResult =
            PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(
                missingSource,
                state);
    CHECK(!missingResult.publishAttempted);
    CHECK(!missingResult.cdMmioSubmitAttempted);
    CHECK(!missingResult.cdMmioSubmitAccepted);
    CHECK(missingResult.blocker ==
          PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::
              SourceUnavailable);
    CHECK(state.lowerCdSnapshotApplyCount == 0u);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.dword_80057108Known);

    PrStage1XaCdDirectRawEventRuntimeSource80036AF8 unknownTransaction{};
    unknownTransaction.sourceAvailable = true;
    const PrStage1XaCdDirectRawEventRuntimeSourceResult80036AF8
        unknownResult =
            PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(
                unknownTransaction,
                state);
    CHECK(!unknownResult.publishAttempted);
    CHECK(!unknownResult.cdMmioSubmitAttempted);
    CHECK(!unknownResult.cdMmioSubmitAccepted);
    CHECK(unknownResult.blocker ==
          PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::
              TransactionUnknown);
    CHECK(state.lowerCdSnapshotApplyCount == 0u);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.dword_80057108Known);
}

void TestRawEventTypedSourceAuditDoesNotSynthesizeTransaction() {
    PrStage1XaCdDirectState state{};
    state.byte_80057119Known = true;
    state.byte_80057119 = 0x0Eu;

    const PrStage1XaCdDirectRawEventTypedSourceAudit80036AF8 audit =
        PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(state);

    CHECK(audit.inspected);
    CHECK(audit.registerPointersKnown);
    CHECK(!audit.initialInterruptKnown);
    CHECK(!audit.stableInterruptKnown);
    CHECK(!audit.cdReg0StatusKnown);
    CHECK(!audit.fifoStatusSamplesKnown);
    CHECK(!audit.resultByteCountKnown);
    CHECK(!audit.resultBytesKnown);
    CHECK(!audit.ackWritesKnown);
    CHECK(!audit.priorDword80057108Known);
    CHECK(!audit.priorDword80057110Known);
    CHECK(audit.priorByte80057119Known);
    CHECK(audit.missingMask == 1022u);
    CHECK(audit.firstMissing ==
          PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
              InitialInterrupt);
    CHECK(!audit.typedCanReconstructTransaction);
    CHECK(!audit.canFeedRuntimeSource);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.dword_80057108Known);

    const auto source =
        PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(state);
    CHECK(source.sourceAvailable);
    CHECK(!source.transactionKnown);
    const auto result =
        PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(source, state);
    CHECK(!result.publishAttempted);
    CHECK(!result.cdMmioSubmitAttempted);
    CHECK(!result.cdMmioSubmitAccepted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::
              TransactionUnknown);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.dword_80057108Known);
}

void TestRawEventTypedSourceAdvancesOnlyAfterInitialInterruptSample() {
    PrStage1XaCdDirectState state{};
    state.byte_80057119Known = true;
    state.byte_80057119 = 0x0Eu;
    state.rawEvent80036AF8InitialInterruptKnown = true;
    state.rawEvent80036AF8InitialInterrupt = 1u;

    const auto audit =
        PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(state);

    CHECK(audit.initialInterruptKnown);
    CHECK(!audit.stableInterruptKnown);
    CHECK(audit.missingMask == 1020u);
    CHECK(audit.firstMissing ==
          PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
              StableInterrupt);
    CHECK(!audit.typedCanReconstructTransaction);
    CHECK(!audit.canFeedRuntimeSource);

    const auto source =
        PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(state);
    CHECK(source.sourceAvailable);
    CHECK(!source.transactionKnown);
}

void TestRawEventInitialInterruptRuntimeSourcePublishesOnlyExactSample() {
    PrStage1XaCdDirectState state{};
    state.byte_80057119Known = true;
    state.byte_80057119 = 0x10u;

    PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8 missingWindow{};
    missingWindow.providerInstalled = true;
    missingWindow.cdReg3InitialPsxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    missingWindow.cdReg3InitialByteSize = 1u;
    missingWindow.cdReg3InitialReadAttempted = true;
    const auto missingSource =
        PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
            missingWindow);
    const auto missingResult =
        PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
            missingSource,
            state);
    CHECK(!missingResult.publishAttempted);
    CHECK(missingResult.blocker ==
          PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8::
              SourceUnavailable);
    CHECK(!state.rawEvent80036AF8InitialInterruptKnown);
    CHECK(state.rawEvent80036AF8InitialInterruptObservationAcceptedCount == 0u);
    CHECK(state.rawEvent80036AF8InitialInterruptObservationRejectedCount == 0u);

    PrStage1XaCdDirectRawEventInitialInterruptObservation80036AF8 wrong{};
    wrong.source =
        PrStage1XaCdDirectRawEventInitialInterruptObservationSource80036AF8::
            RuntimePsxMemoryObservation;
    wrong.psxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8 + 1u;
    wrong.byteSize = 1u;
    wrong.valueKnown = true;
    wrong.value = 1u;
    const auto wrongResult =
        PrStage1XaCdDirectApplyRawEventInitialInterruptRuntimeObservation80036AF8(
            wrong,
            state);
    CHECK(!wrongResult.accepted);
    CHECK(wrongResult.rejectReason ==
          PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8::
              WrongPsxAddress);
    CHECK(!state.rawEvent80036AF8InitialInterruptKnown);
    CHECK(state.rawEvent80036AF8InitialInterruptObservationAcceptedCount == 0u);
    CHECK(state.rawEvent80036AF8InitialInterruptObservationRejectedCount == 1u);

    PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8 readableWindow{};
    readableWindow.providerInstalled = true;
    readableWindow.cdReg3InitialReadAttempted = true;
    readableWindow.cdReg3InitialReadable = true;
    readableWindow.cdReg3InitialPsxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    readableWindow.cdReg3InitialByteSize = 1u;
    readableWindow.cdReg3InitialValueKnown = true;
    readableWindow.cdReg3InitialValue = 1u;
    readableWindow.cdReg0StatusReadAttempted = true;
    readableWindow.cdReg0StatusReadable = true;
    readableWindow.cdReg0StatusPsxAddress =
        PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8;
    readableWindow.cdReg0StatusByteSize = 1u;
    readableWindow.cdReg0StatusValueKnown = true;
    readableWindow.cdReg0StatusValue = 0u;
    const auto readableSource =
        PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
            readableWindow);
    const auto readableResult =
        PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
            readableSource,
            state);
    CHECK(readableResult.publishAttempted);
    CHECK(readableResult.observation.accepted);
    CHECK(readableResult.blocker ==
          PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8::
              None);
    CHECK(state.rawEvent80036AF8InitialInterruptKnown);
    CHECK(state.rawEvent80036AF8InitialInterrupt == 1u);
    CHECK(state.rawEvent80036AF8InitialInterruptObservationAcceptedCount == 1u);
    CHECK(state.rawEvent80036AF8InitialInterruptObservationRejectedCount == 1u);

    const auto audit =
        PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(state);
    CHECK(audit.initialInterruptKnown);
    CHECK(!audit.stableInterruptKnown);
    CHECK(audit.missingMask == 1020u);
    CHECK(audit.firstMissing ==
          PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
              StableInterrupt);
    CHECK(!audit.typedCanReconstructTransaction);
    CHECK(!audit.canFeedRuntimeSource);

    const auto fullSource =
        PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(state);
    CHECK(fullSource.sourceAvailable);
    CHECK(!fullSource.transactionKnown);
    const auto fullResult =
        PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(fullSource,
                                                              state);
    CHECK(!fullResult.publishAttempted);
    CHECK(!fullResult.cdMmioSubmitAttempted);
    CHECK(!fullResult.cdMmioSubmitAccepted);
    CHECK(state.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!state.dword_80057108Known);
}

void TestKnownStateProviderDoesNotSynthesizeRawCdRegSamples() {
    PrStage1XaCdDirectState sourceState{};
    sourceState.dword_80057108Known = true;
    sourceState.dword_80057108 = 0x00000022u;
    sourceState.dword_80057110Known = true;
    sourceState.dword_80057110 = 0u;
    sourceState.byte_80057119Known = true;
    sourceState.byte_80057119 = 0x0Eu;
    sourceState.dword_80057010Known = true;
    sourceState.dword_80057010 = 0u;

    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadKnownStateRuntimePsxMemory;
    provider.userData = &sourceState;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x80036AF8u;

    const auto window =
        PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(provider);
    CHECK(window.providerInstalled);
    CHECK(window.cdReg3InitialReadAttempted);
    CHECK(window.cdReg0StatusReadAttempted);
    CHECK(window.cdReg3InitialPsxAddress ==
          PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8);
    CHECK(window.cdReg0StatusPsxAddress ==
          PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8);
    CHECK(!window.cdReg3InitialReadable);
    CHECK(!window.cdReg3InitialValueKnown);
    CHECK(!window.cdReg0StatusReadable);
    CHECK(!window.cdReg0StatusValueKnown);

    PrStage1XaCdDirectState targetState{};
    const auto initialSource =
        PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
            window);
    CHECK(!initialSource.sourceAvailable);
    CHECK(!initialSource.valueKnown);
    const auto initialResult =
        PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
            initialSource,
            targetState);
    CHECK(!initialResult.publishAttempted);
    CHECK(initialResult.blocker ==
          PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8::
              SourceUnavailable);
    CHECK(!targetState.rawEvent80036AF8InitialInterruptKnown);
    CHECK(targetState.rawEvent80036AF8InitialInterruptObservationAcceptedCount ==
          0u);
    CHECK(targetState.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!targetState.dword_80057108Known);

    const auto audit =
        PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(sourceState);
    CHECK(audit.priorDword80057108Known);
    CHECK(audit.priorDword80057110Known);
    CHECK(audit.priorByte80057119Known);
    CHECK(!audit.initialInterruptKnown);
    CHECK(!audit.cdReg0StatusKnown);
    CHECK(!audit.typedCanReconstructTransaction);
    CHECK(!audit.canFeedRuntimeSource);

    const auto rawSource =
        PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(sourceState);
    CHECK(rawSource.sourceAvailable);
    CHECK(!rawSource.transactionKnown);
    const auto rawResult =
        PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(rawSource,
                                                              targetState);
    CHECK(!rawResult.publishAttempted);
    CHECK(rawResult.blocker ==
          PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::
              TransactionUnknown);
    CHECK(targetState.cdLowerEvent80036AF8Serial == 0u);
    CHECK(!targetState.dword_80057108Known);
}

void TestCdMmioSnapshotAdapterFeedsOnlyExactRawCdRegSamples() {
    PrStage1XaCdDirectCdMmioSnapshotRuntimeSource snapshot{};
    snapshot.producerIngressInstalled = true;
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    provider.userData = &snapshot;
    provider.cdMmioSourceInstalled = true;
    provider.cdMmioRead = PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    provider.cdMmioUserData = &snapshot;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x80036AF8u;

    auto window =
        PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(provider);
    CHECK(window.providerInstalled);
    CHECK(window.cdReg3InitialReadAttempted);
    CHECK(window.cdReg0StatusReadAttempted);
    CHECK(!window.cdReg3InitialReadable);
    CHECK(!window.cdReg0StatusReadable);
    auto cdMmioAudit =
        PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(cdMmioAudit.snapshotSource);
    CHECK(cdMmioAudit.producerIngressInstalled);
    CHECK(cdMmioAudit.producerObservationCallCount == 0u);
    CHECK(cdMmioAudit.observationAcceptedCount == 0u);
    CHECK(cdMmioAudit.observationRejectedCount == 0u);
    CHECK(cdMmioAudit.lastRejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);

    PrStage1XaCdDirectRuntimePsxMemoryProvider missingProvider{};
    PrStage1XaCdDirectCdMmioSnapshotObservation missingProviderObservation{};
    missingProviderObservation.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer;
    missingProviderObservation.psxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    missingProviderObservation.byteSize = 1u;
    missingProviderObservation.valueKnown = true;
    missingProviderObservation.value = 1u;
    auto cdMmioObservation =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            missingProviderObservation,
            missingProvider);
    CHECK(!cdMmioObservation.accepted);
    CHECK(cdMmioObservation.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              RuntimeProviderUnavailable);
    CHECK(snapshot.producerObservationCallCount == 0u);
    CHECK(snapshot.observationAcceptedCount == 0u);
    CHECK(snapshot.observationRejectedCount == 0u);

    PrStage1XaCdDirectCdMmioSnapshotObservation badSource{};
    badSource.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::Unknown;
    badSource.psxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    badSource.byteSize = 1u;
    badSource.valueKnown = true;
    badSource.value = 1u;
    cdMmioObservation =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            badSource,
            provider);
    CHECK(!cdMmioObservation.accepted);
    CHECK(cdMmioObservation.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              NonRuntimeCdMmioSampleProducer);
    CHECK(!snapshot.cdReg3InitialKnown);
    CHECK(!snapshot.cdReg0StatusKnown);
    CHECK(snapshot.producerObservationCallCount == 1u);
    CHECK(snapshot.observationAcceptedCount == 0u);
    CHECK(snapshot.observationRejectedCount == 1u);
    CHECK(snapshot.lastRejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              NonRuntimeCdMmioSampleProducer);

    PrStage1XaCdDirectCdMmioSnapshotObservation wrongAddress{};
    wrongAddress.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer;
    wrongAddress.psxAddress = 0x80057108u;
    wrongAddress.byteSize = 1u;
    wrongAddress.valueKnown = true;
    wrongAddress.value = 1u;
    cdMmioObservation =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            wrongAddress,
            provider);
    CHECK(!cdMmioObservation.accepted);
    CHECK(cdMmioObservation.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              UnsupportedPsxAddress);
    CHECK(!snapshot.cdReg3InitialKnown);
    CHECK(!snapshot.cdReg0StatusKnown);
    CHECK(snapshot.producerObservationCallCount == 2u);
    CHECK(snapshot.observationAcceptedCount == 0u);
    CHECK(snapshot.observationRejectedCount == 2u);
    CHECK(snapshot.lastRejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              UnsupportedPsxAddress);

    PrStage1XaCdDirectCdMmioSnapshotObservation wrongSize{};
    wrongSize.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer;
    wrongSize.psxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    wrongSize.byteSize = 4u;
    wrongSize.valueKnown = true;
    wrongSize.value = 1u;
    cdMmioObservation =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            wrongSize,
            provider);
    CHECK(!cdMmioObservation.accepted);
    CHECK(cdMmioObservation.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              WrongByteSize);
    CHECK(!snapshot.cdReg3InitialKnown);
    CHECK(!snapshot.cdReg0StatusKnown);
    CHECK(snapshot.producerObservationCallCount == 3u);
    CHECK(snapshot.observationAcceptedCount == 0u);
    CHECK(snapshot.observationRejectedCount == 3u);
    CHECK(snapshot.lastRejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              WrongByteSize);

    PrStage1XaCdDirectCdMmioSnapshotObservation unknownValue{};
    unknownValue.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer;
    unknownValue.psxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    unknownValue.byteSize = 1u;
    unknownValue.valueKnown = false;
    unknownValue.value = 1u;
    cdMmioObservation =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            unknownValue,
            provider);
    CHECK(!cdMmioObservation.accepted);
    CHECK(cdMmioObservation.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              UnknownValue);
    CHECK(!snapshot.cdReg3InitialKnown);
    CHECK(!snapshot.cdReg0StatusKnown);
    CHECK(snapshot.producerObservationCallCount == 4u);
    CHECK(snapshot.observationAcceptedCount == 0u);
    CHECK(snapshot.observationRejectedCount == 4u);
    CHECK(snapshot.lastRejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              UnknownValue);

    auto missingStatusTransaction = ValidRawEventTransaction();
    missingStatusTransaction.cdReg0StatusKnown = false;
    const bool missingStatusSubmitted =
        PrStage1XaCdDirectSubmitCdMmioSnapshotSamplesFromRawEvent80036AF8(
            missingStatusTransaction,
            provider);
    CHECK(!missingStatusSubmitted);
    CHECK(!snapshot.cdReg3InitialKnown);
    CHECK(!snapshot.cdReg0StatusKnown);
    CHECK(snapshot.producerObservationCallCount == 4u);
    CHECK(snapshot.observationAcceptedCount == 0u);
    CHECK(snapshot.observationRejectedCount == 4u);

    PrStage1XaCdDirectCdMmioSnapshotObservation cdReg3{};
    cdReg3.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer;
    cdReg3.psxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    cdReg3.byteSize = 1u;
    cdReg3.valueKnown = true;
    cdReg3.value = 1u;
    cdMmioObservation =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            cdReg3,
            provider);
    CHECK(cdMmioObservation.accepted);
    CHECK(cdMmioObservation.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);
    CHECK(snapshot.observationAcceptedCount == 1u);
    CHECK(snapshot.producerObservationCallCount == 5u);
    CHECK(snapshot.observationRejectedCount == 4u);
    CHECK(snapshot.lastRejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);

    PrStage1XaCdDirectCdMmioSnapshotObservation cdReg0{};
    cdReg0.source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::
            RuntimeCdMmioSampleProducer;
    cdReg0.psxAddress = PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8;
    cdReg0.byteSize = 1u;
    cdReg0.valueKnown = true;
    cdReg0.value = 0u;
    cdMmioObservation =
        PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
            cdReg0,
            provider);
    CHECK(cdMmioObservation.accepted);
    CHECK(cdMmioObservation.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);
    CHECK(snapshot.observationAcceptedCount == 2u);
    CHECK(snapshot.producerObservationCallCount == 6u);
    CHECK(snapshot.observationRejectedCount == 4u);
    CHECK(snapshot.lastRejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);

    window =
        PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(provider);
    CHECK(window.cdReg3InitialReadable);
    CHECK(window.cdReg3InitialValueKnown);
    CHECK(window.cdReg3InitialValue == 1u);
    CHECK(window.cdReg0StatusReadable);
    CHECK(window.cdReg0StatusValueKnown);
    CHECK(window.cdReg0StatusValue == 0u);
    CHECK(window.pcKnown);
    CHECK(window.pc == 0x80036AF8u);
    cdMmioAudit =
        PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(cdMmioAudit.producerObservationCallCount == 6u);
    CHECK(cdMmioAudit.observationAcceptedCount == 2u);
    CHECK(cdMmioAudit.observationRejectedCount == 4u);
    CHECK(cdMmioAudit.lastRejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);

    const auto status57108Window =
        PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(provider);
    CHECK(status57108Window.readAttempted);
    CHECK(!status57108Window.windowReadable);
    CHECK(!status57108Window.valueKnown);

    PrStage1XaCdDirectState targetState{};
    const auto initialSource =
        PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
            window);
    const auto initialResult =
        PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
            initialSource,
            targetState);
    CHECK(initialResult.publishAttempted);
    CHECK(initialResult.observation.accepted);
    CHECK(targetState.rawEvent80036AF8InitialInterruptKnown);
    CHECK(targetState.rawEvent80036AF8InitialInterrupt == 1u);
    CHECK(targetState.rawEvent80036AF8InitialInterruptObservationAcceptedCount ==
          1u);

    const auto audit =
        PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(targetState);
    CHECK(audit.initialInterruptKnown);
    CHECK(!audit.stableInterruptKnown);
    CHECK(!audit.typedCanReconstructTransaction);
    CHECK(!audit.canFeedRuntimeSource);
    CHECK(!targetState.dword_80057108Known);
    CHECK(targetState.cdLowerEvent80036AF8Serial == 0u);
}

void TestCdMmioRuntimeProducerCallsiteFailsClosedWithoutSource() {
    PrStage1XaCdDirectCdMmioSnapshotRuntimeSource snapshot{};
    snapshot.producerIngressInstalled = true;
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    provider.userData = &snapshot;
    provider.cdMmioSourceInstalled = true;
    provider.cdMmioRead = PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    provider.cdMmioUserData = &snapshot;
    provider.frameKnown = true;
    provider.frame = 4438u;

    PrStage1XaCdDirectCdMmioRuntimeProducerSample missingSample{};
    missingSample.frameKnown = true;
    missingSample.frame = 4438u;
    missingSample.pcKnown = true;
    missingSample.pc = 0x80036AF8u;
    const auto missing =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSample(missingSample,
                                                            provider);
    CHECK(missing.attempted);
    CHECK(!missing.sourceAvailable);
    CHECK(!missing.valueKnown);
    CHECK(!missing.exactEnvelope);
    CHECK(!missing.publishAttempted);
    CHECK(!missing.accepted);
    CHECK(missing.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);
    auto audit = PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(audit.producerObservationCallCount == 0u);
    CHECK(audit.observationAcceptedCount == 0u);
    CHECK(audit.observationRejectedCount == 0u);
    CHECK(!audit.samplePairAvailable);

    PrStage1XaCdDirectCdMmioRuntimeProducerSample wrongAddress{};
    wrongAddress.sourceAvailable = true;
    wrongAddress.psxAddress = 0x80057108u;
    wrongAddress.byteSize = 1u;
    wrongAddress.valueKnown = true;
    wrongAddress.value = 3u;
    wrongAddress.frameKnown = true;
    wrongAddress.frame = 4438u;
    wrongAddress.pcKnown = true;
    wrongAddress.pc = 0x80036AF8u;
    const auto wrongAddressResult =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSample(wrongAddress,
                                                            provider);
    CHECK(wrongAddressResult.attempted);
    CHECK(wrongAddressResult.sourceAvailable);
    CHECK(wrongAddressResult.valueKnown);
    CHECK(!wrongAddressResult.exactEnvelope);
    CHECK(!wrongAddressResult.publishAttempted);
    CHECK(!wrongAddressResult.accepted);
    CHECK(wrongAddressResult.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              UnsupportedPsxAddress);
    audit = PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(audit.producerObservationCallCount == 0u);
    CHECK(audit.observationAcceptedCount == 0u);
    CHECK(audit.observationRejectedCount == 0u);

    PrStage1XaCdDirectCdMmioRuntimeProducerSample wrongSize{};
    wrongSize.sourceAvailable = true;
    wrongSize.psxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    wrongSize.byteSize = 4u;
    wrongSize.valueKnown = true;
    wrongSize.value = 3u;
    const auto wrongSizeResult =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSample(wrongSize,
                                                            provider);
    CHECK(wrongSizeResult.attempted);
    CHECK(wrongSizeResult.sourceAvailable);
    CHECK(wrongSizeResult.valueKnown);
    CHECK(!wrongSizeResult.exactEnvelope);
    CHECK(!wrongSizeResult.publishAttempted);
    CHECK(!wrongSizeResult.accepted);
    CHECK(wrongSizeResult.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::WrongByteSize);
    audit = PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(audit.producerObservationCallCount == 0u);
    CHECK(audit.observationAcceptedCount == 0u);
    CHECK(audit.observationRejectedCount == 0u);

    PrStage1XaCdDirectCdMmioRuntimeProducerSample unknownValue{};
    unknownValue.sourceAvailable = true;
    unknownValue.psxAddress =
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    unknownValue.byteSize = 1u;
    unknownValue.valueKnown = false;
    unknownValue.value = 3u;
    const auto unknownValueResult =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSample(unknownValue,
                                                            provider);
    CHECK(unknownValueResult.attempted);
    CHECK(unknownValueResult.sourceAvailable);
    CHECK(!unknownValueResult.valueKnown);
    CHECK(!unknownValueResult.exactEnvelope);
    CHECK(!unknownValueResult.publishAttempted);
    CHECK(!unknownValueResult.accepted);
    CHECK(unknownValueResult.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::UnknownValue);
    audit = PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(audit.producerObservationCallCount == 0u);
    CHECK(audit.observationAcceptedCount == 0u);
    CHECK(audit.observationRejectedCount == 0u);

    PrStage1XaCdDirectCdMmioRuntimeProducerSample exactSample{};
    exactSample.sourceAvailable = true;
    exactSample.psxAddress = PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    exactSample.byteSize = 1u;
    exactSample.valueKnown = true;
    exactSample.value = 3u;
    exactSample.frameKnown = true;
    exactSample.frame = 4438u;
    exactSample.pcKnown = true;
    exactSample.pc = 0x80036AF8u;
    const auto exact =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSample(exactSample,
                                                            provider);
    CHECK(exact.attempted);
    CHECK(exact.sourceAvailable);
    CHECK(exact.valueKnown);
    CHECK(exact.exactEnvelope);
    CHECK(exact.publishAttempted);
    CHECK(exact.accepted);
    CHECK(exact.rejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);
    audit = PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(audit.producerObservationCallCount == 1u);
    CHECK(audit.observationAcceptedCount == 1u);
    CHECK(audit.observationRejectedCount == 0u);
    CHECK(audit.cdReg3InitialKnown);
    CHECK(audit.cdReg3Initial == 3u);
    CHECK(!audit.cdReg0StatusKnown);
    CHECK(!audit.samplePairAvailable);
}

void TestRawEventRuntimeSourceBuildsFromTypedStateWhenComplete() {
    PrStage1XaCdDirectState state{};
    FillValidRawEventTypedState(state);

    const auto audit =
        PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(state);
    CHECK(audit.typedCanReconstructTransaction);
    CHECK(audit.canFeedRuntimeSource);
    CHECK(audit.firstMissing ==
          PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::None);

    const auto source =
        PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(state);
    CHECK(source.sourceAvailable);
    CHECK(source.transactionKnown);
    CHECK(source.transaction.produced);
    CHECK(!source.transaction.incomplete);
    CHECK(source.transaction.resultBytesKnown);
    CHECK(source.transaction.resultBytes[0] == 0x10u);

    state.word_80055F7AKnown = true;
    state.word_80055F7A = 1u;
    const auto result =
        PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(source, state);
    CHECK(result.publishAttempted);
    CHECK(!result.cdMmioSubmitAttempted);
    CHECK(!result.cdMmioSubmitAccepted);
    CHECK(result.accepted);
    CHECK(result.lowerEventApplied);
    CHECK(state.cdLowerEvent80036AF8Serial == 1u);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x10u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
}

void TestRawEventRuntimeSourceFeedsExplicitEventOnlyWhenComplete() {
    const auto transaction = ValidRawEventTransaction();
    CHECK(transaction.produced);
    CHECK(!transaction.incomplete);
    CHECK(transaction.resultBytesKnown);

    PrStage1XaCdDirectState state{};
    state.word_80055F7AKnown = true;
    state.word_80055F7A = 1u;

    PrStage1XaCdDirectCdMmioSnapshotRuntimeSource snapshot{};
    snapshot.producerIngressInstalled = true;
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    provider.userData = &snapshot;
    provider.cdMmioSourceInstalled = true;
    provider.cdMmioRead = PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    provider.cdMmioUserData = &snapshot;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x80036AF8u;

    PrStage1XaCdDirectRawEventRuntimeSource80036AF8 source{};
    source.sourceAvailable = true;
    source.transactionKnown = true;
    source.transaction = transaction;
    const PrStage1XaCdDirectRawEventRuntimeSourceResult80036AF8 result =
        PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(source,
                                                              state,
                                                              &provider);

    CHECK(result.publishAttempted);
    CHECK(result.cdMmioSubmitAttempted);
    CHECK(result.cdMmioSubmitAccepted);
    CHECK(result.accepted);
    CHECK(result.blocker ==
          PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::None);
    CHECK(result.bridge.produced);
    CHECK(result.bridge.lowerEventRegisters80036AF8Bridged);
    CHECK(result.applied.called);
    CHECK(result.applied.checkCallback80035898Applied);
    CHECK(result.applied.lowerEventRegisters80036AF8Applied);
    CHECK(result.lowerEventApplied);
    CHECK(state.lowerCdSnapshotApplyCount == 1u);
    CHECK(state.lowerCdSnapshotCallbackEventCount == 1u);
    CHECK(state.cdLowerEvent80036AF8Serial == 1u);
    CHECK(state.cdLowerEventPsxReturn80036AF8Known);
    CHECK(state.cdLowerEventPsxReturn80036AF8 == 2);
    CHECK(state.dword_80057108Known);
    CHECK(state.dword_80057108 == 0x10u);
    CHECK(state.statusFlags80057108ObservationAcceptedCount == 1u);
    CHECK(state.statusFlags80057108ObservationRejectedCount == 0u);

    const auto cdMmioAudit =
        PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(cdMmioAudit.producerObservationCallCount == 2u);
    CHECK(cdMmioAudit.observationAcceptedCount == 2u);
    CHECK(cdMmioAudit.observationRejectedCount == 0u);
    CHECK(cdMmioAudit.cdReg3InitialKnown);
    CHECK(cdMmioAudit.cdReg3Initial == 2u);
    CHECK(cdMmioAudit.cdReg0StatusKnown);
    CHECK(cdMmioAudit.cdReg0Status == 0u);
    CHECK(cdMmioAudit.samplePairAvailable);
    const auto cdMmioWindow =
        PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(provider);
    CHECK(cdMmioWindow.cdReg3InitialReadable);
    CHECK(cdMmioWindow.cdReg3InitialValue == 2u);
    CHECK(cdMmioWindow.cdReg0StatusReadable);
    CHECK(cdMmioWindow.cdReg0StatusValue == 0u);
}

PrStage1XaCdDirectCdMmioRuntimeProducerSample RuntimeCdMmioSample(
    uint32_t psxAddress,
    uint8_t value) {
    PrStage1XaCdDirectCdMmioRuntimeProducerSample sample{};
    sample.sourceAvailable = true;
    sample.psxAddress = psxAddress;
    sample.byteSize = 1u;
    sample.valueKnown = true;
    sample.value = value;
    sample.frameKnown = true;
    sample.frame = 4438u;
    sample.pcKnown = true;
    sample.pc = 0x80036AF8u;
    return sample;
}

void TestCdMmioRuntimeProducerPairIsAtomic() {
    PrStage1XaCdDirectCdMmioSnapshotRuntimeSource snapshot{};
    snapshot.producerIngressInstalled = true;
    PrStage1XaCdDirectRuntimePsxMemoryProvider provider{};
    provider.installed = true;
    provider.read = PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    provider.userData = &snapshot;
    provider.cdMmioSourceInstalled = true;
    provider.cdMmioRead = PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    provider.cdMmioUserData = &snapshot;
    provider.frameKnown = true;
    provider.frame = 4438u;
    provider.pcKnown = true;
    provider.pc = 0x80036AF8u;

    auto cdReg3 = RuntimeCdMmioSample(
        PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8,
        2u);
    auto malformedCdReg0 = RuntimeCdMmioSample(
        PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8,
        0u);
    malformedCdReg0.byteSize = 4u;
    const auto rejected =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSamplePair(
            cdReg3,
            malformedCdReg0,
            provider);
    CHECK(rejected.attempted);
    CHECK(rejected.sourceAvailable);
    CHECK(rejected.cdReg3ExactEnvelope);
    CHECK(!rejected.cdReg0ExactEnvelope);
    CHECK(!rejected.publishAttempted);
    CHECK(!rejected.accepted);
    CHECK(rejected.cdReg0RejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::
              WrongByteSize);
    CHECK(!snapshot.cdReg3InitialKnown);
    CHECK(!snapshot.cdReg0StatusKnown);
    CHECK(snapshot.producerObservationCallCount == 0u);
    CHECK(snapshot.observationAcceptedCount == 0u);
    CHECK(snapshot.observationRejectedCount == 0u);

    auto cdReg0 = RuntimeCdMmioSample(
        PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8,
        0u);
    const auto accepted =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSamplePair(cdReg3,
                                                               cdReg0,
                                                               provider);
    CHECK(accepted.attempted);
    CHECK(accepted.sourceAvailable);
    CHECK(accepted.cdReg3ExactEnvelope);
    CHECK(accepted.cdReg0ExactEnvelope);
    CHECK(accepted.publishAttempted);
    CHECK(accepted.accepted);
    CHECK(accepted.cdReg3RejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);
    CHECK(accepted.cdReg0RejectReason ==
          PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None);
    CHECK(snapshot.producerObservationCallCount == 2u);
    CHECK(snapshot.observationAcceptedCount == 2u);
    CHECK(snapshot.observationRejectedCount == 0u);
    CHECK(snapshot.cdReg3InitialKnown);
    CHECK(snapshot.cdReg3Initial == 2u);
    CHECK(snapshot.cdReg0StatusKnown);
    CHECK(snapshot.cdReg0Status == 0u);

    const auto audit =
        PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    CHECK(audit.samplePairAvailable);
    CHECK(audit.canFeedCdReg3Initial);
    CHECK(audit.canFeedCdReg0Status);
}

} // namespace

int main() {
    TestGetlocFactsDoNotSynthesizePendingProducer();
    TestPendingProducerRequiresExplicitInterruptSnapshot();
    TestRawEventRuntimeSourceFailsClosedBeforeBridge();
    TestRawEventTypedSourceAuditDoesNotSynthesizeTransaction();
    TestRawEventTypedSourceAdvancesOnlyAfterInitialInterruptSample();
    TestRawEventInitialInterruptRuntimeSourcePublishesOnlyExactSample();
    TestKnownStateProviderDoesNotSynthesizeRawCdRegSamples();
    TestCdMmioSnapshotAdapterFeedsOnlyExactRawCdRegSamples();
    TestCdMmioRuntimeProducerPairIsAtomic();
    TestCdMmioRuntimeProducerCallsiteFailsClosedWithoutSource();
    TestRawEventRuntimeSourceBuildsFromTypedStateWhenComplete();
    TestRawEventRuntimeSourceFeedsExplicitEventOnlyWhenComplete();
    if (g_failedChecks != 0) {
        std::printf(
            "test_ss0_stage1_xacd_getloc_pending_boundary_contract: failed checks=%d\n",
            g_failedChecks);
        return 1;
    }
    std::printf(
        "test_ss0_stage1_xacd_getloc_pending_boundary_contract: ok\n");
    return 0;
}
