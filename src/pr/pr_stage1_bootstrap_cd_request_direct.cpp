#include "pr_stage1_bootstrap_cd_request_direct.h"

namespace PrStage1LifecycleExecutorDirect {

bool IsSameBootstrap15590CdLowerRuntimeKey801C81EC(
    const Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LoaderDirect::ProducerStep& step) {
    return runtime.keyKnown &&
           runtime.psxOrder == step.psxOrder &&
           runtime.stepKind == step.stepKind &&
           runtime.cdActionKind == step.action.cd.actionKind &&
           runtime.recordIndex == step.recordIndex &&
           runtime.attemptIndex == step.action.attemptIndex &&
           runtime.lowerFunction == step.action.cd.lowerFunction &&
           runtime.finalFunction == step.action.cd.finalFunction;
}

bool TryCarryBootstrap15590CdLowerSameKeyPendingRequest801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LoaderDirect::ProducerStep& step,
    Bootstrap15590CdLowerHostRequest801C81EC& out) {
    if (!runtime.requestPending ||
        !IsSameBootstrap15590CdLowerRuntimeKey801C81EC(runtime, step)) {
        return false;
    }

    out.readStartHalProgressAccepted =
        runtime.readStartHalProgressAccepted;
    out.readStartHalProgressReadS27Serial =
        runtime.readStartHalProgressReadS27Serial;
    runtime.status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::RequestAlreadyPending;
    out.status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::RequestAlreadyPending;
    return true;
}

void MarkBootstrap15590CdLowerRequestPending801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LoaderDirect::ProducerStep& step,
    const Bootstrap15590CdLowerHostRequest801C81EC& request) {
    runtime.keyKnown = true;
    runtime.requestPending = true;
    runtime.status = Bootstrap15590CdLowerAttemptStatus801C81EC::RequestReady;
    runtime.psxOrder = step.psxOrder;
    runtime.stepKind = step.stepKind;
    runtime.cdActionKind = step.action.cd.actionKind;
    runtime.recordIndex = step.recordIndex;
    runtime.recordType = step.recordType;
    runtime.attemptIndex = step.action.attemptIndex;
    runtime.lowerFunction = step.action.cd.lowerFunction;
    runtime.finalFunction = step.action.cd.finalFunction;
    runtime.lowerRequest = request.lowerRequest;
    runtime.payloadBytesRequired = request.payloadBytesRequired;
    runtime.seekLbaKnown = request.seekLbaKnown;
    runtime.seekLba = request.seekLba;
    runtime.readDstPtrKnown = request.readDstPtrKnown;
    runtime.readDstPtr = request.readDstPtr;
    runtime.readSectorCountKnown = request.readSectorCountKnown;
    runtime.readSectorCount = request.readSectorCount;
    runtime.readStartHalProgressAccepted = false;
    runtime.readStartHalProgressReadS27Serial = 0;
    runtime.modeCmd800375BCDirectOwnerCommitted = false;
    runtime.modeCmd800375BCDirectOwnerReadS27Serial = 0;
    runtime.modeCmd800375BCDirectOwnerCommand = 0u;
    runtime.modeCmd800375BCDirectOwnerArg0 = 0u;
    runtime.finalReadyHalFactsRequired = false;
    runtime.finalReadyHalFactsReadS27Serial = 0;
    runtime.cdSyncLoopFactsRequired80037070 =
        request.cdSyncLoopFactsRequired80037070;
    runtime.cdSyncLoopFunction80037070 =
        request.cdSyncLoopFunction80037070;
    runtime.cdSyncLoopA0WaitModeKnown80037070 =
        request.cdSyncLoopA0WaitModeKnown80037070;
    runtime.cdSyncLoopA0WaitMode80037070 =
        request.cdSyncLoopA0WaitMode80037070;
    runtime.lastRejectKnown = false;
    runtime.lastRejectReason =
        Bootstrap15590CdLowerRejectReason801C81EC::None;
    runtime.lastRejectStatus =
        Bootstrap15590CdLowerAttemptStatus801C81EC::None;
    runtime.lastRejectExpected =
        PrStage1LoaderCdHal::LowerActionRequestMetadata{};
    runtime.lastRejectActual =
        PrStage1LoaderCdHal::LowerActionRequestMetadata{};
    runtime.lastFactsAttempted = false;
    runtime.lastFactsActionKind = PrStage1LoaderCdHal::ActionKind::None;
    runtime.lastFactsReadStartHalFactsKnown = false;
    runtime.lastFactsReadStartSetupProduced = false;
    runtime.lastFactsReadStartSetupIncomplete = false;
    runtime.lastFactsReadPumpProduced = false;
    runtime.lastFactsReadPumpIncomplete = false;
    runtime.lastFactsPayloadBytesKnown = false;
    runtime.lastFactsBridgeProduced = false;
    runtime.lastFactsBridgeIncomplete = false;
    runtime.lastFactsLowerCdFactsBridged = false;
    runtime.lastFactsXaCdSeamKnown = false;
    runtime.lastFactsXaCdAccepted = false;
    runtime.lastFactsRejectReason =
        Bootstrap15590CdLowerRejectReason801C81EC::None;
}

void MarkBootstrap15590CdLowerFeedbackResolved801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LoaderProducerAdapter::TypedActionFeedback& feedback,
    bool applied) {
    if (!feedback.valid || !feedback.cdFeedback) {
        return;
    }
    runtime.requestPending = false;
    if (!applied) {
        runtime.status =
            Bootstrap15590CdLowerAttemptStatus801C81EC::FeedbackRejected;
        return;
    }

    const bool preserveFinalReadyCarrier =
        runtime.finalReadyHalFactsRequired &&
        runtime.finalReadyHalFactsReadS27Serial != 0u &&
        runtime.readStartHalProgressAccepted &&
        runtime.readStartHalProgressReadS27Serial ==
            runtime.finalReadyHalFactsReadS27Serial &&
        runtime.readStartHalCarrierKnown &&
        runtime.readStartHalCarrierReadS27Serial ==
            runtime.finalReadyHalFactsReadS27Serial;
    const bool preservedReadStartHalProgressAccepted =
        runtime.readStartHalProgressAccepted;
    const uint32_t preservedReadStartHalProgressReadS27Serial =
        runtime.readStartHalProgressReadS27Serial;
    const bool preservedReadStartHalCarrierKnown =
        runtime.readStartHalCarrierKnown;
    const uint32_t preservedReadStartHalCarrierReadS27Serial =
        runtime.readStartHalCarrierReadS27Serial;
    const auto preservedReadStartHalCarrierSetup =
        runtime.readStartHalCarrierSetup;
    const auto preservedReadStartHalCarrierPump =
        runtime.readStartHalCarrierPump;
    const bool preservedReadStartHalCarrierPayloadBytesKnown =
        runtime.readStartHalCarrierPayloadBytesKnown;
    const auto preservedReadStartHalCarrierPayloadBytes =
        runtime.readStartHalCarrierPayloadBytes;
    const bool preservedFinalReadyHalFactsRequired =
        runtime.finalReadyHalFactsRequired;
    const uint32_t preservedFinalReadyHalFactsReadS27Serial =
        runtime.finalReadyHalFactsReadS27Serial;

    runtime = Bootstrap15590CdLowerProducerRuntime801C81EC{};
    if (!preserveFinalReadyCarrier) {
        return;
    }
    runtime.readStartHalProgressAccepted =
        preservedReadStartHalProgressAccepted;
    runtime.readStartHalProgressReadS27Serial =
        preservedReadStartHalProgressReadS27Serial;
    runtime.readStartHalCarrierKnown = preservedReadStartHalCarrierKnown;
    runtime.readStartHalCarrierReadS27Serial =
        preservedReadStartHalCarrierReadS27Serial;
    runtime.readStartHalCarrierSetup = preservedReadStartHalCarrierSetup;
    runtime.readStartHalCarrierPump = preservedReadStartHalCarrierPump;
    runtime.readStartHalCarrierPayloadBytesKnown =
        preservedReadStartHalCarrierPayloadBytesKnown;
    runtime.readStartHalCarrierPayloadBytes =
        preservedReadStartHalCarrierPayloadBytes;
    runtime.finalReadyHalFactsRequired =
        preservedFinalReadyHalFactsRequired;
    runtime.finalReadyHalFactsReadS27Serial =
        preservedFinalReadyHalFactsReadS27Serial;
}

bool MarkBootstrap15590FinalReadyHalFactsRequired801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const Bootstrap15590CdLowerHostRequest801C81EC& request,
    uint32_t currentReadS27Serial) {
    if (!IsFinalReadyProgressGapForPendingReadStart801C81EC(
            request,
            currentReadS27Serial)) {
        return false;
    }

    runtime.finalReadyHalFactsRequired = true;
    runtime.finalReadyHalFactsReadS27Serial = currentReadS27Serial;
    return true;
}

bool IsBootstrap15590FinalReadyHalFactsRequirementCurrent801C81EC(
    const Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const Bootstrap15590CdLowerHostRequest801C81EC& request,
    uint32_t currentReadS27Serial) {
    return runtime.finalReadyHalFactsRequired &&
           runtime.finalReadyHalFactsReadS27Serial ==
               currentReadS27Serial &&
           IsFinalReadyProgressGapForPendingReadStart801C81EC(
               request,
               currentReadS27Serial);
}

bool IsReadSyncCompletionForPendingReadStart801C81EC(
    const Bootstrap15590CdLowerHostRequest801C81EC& expected,
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& actual) {
    namespace CdHal = PrStage1LoaderCdHal;
    const CdHal::LowerActionRequestMetadata& readStart =
        expected.lowerRequest;
    return expected.cdActionKind == CdHal::ActionKind::ReadStart80038FC0 &&
           expected.status ==
               Bootstrap15590CdLowerAttemptStatus801C81EC::
                   RequestAlreadyPending &&
           expected.readStartHalProgressAccepted &&
           expected.readStartHalProgressReadS27Serial != 0u &&
           readStart.known &&
           readStart.actionKind == CdHal::ActionKind::ReadStart80038FC0 &&
           readStart.callerFunction == CdHal::kFn8001A818 &&
           readStart.lowerFunction == CdHal::kFn80038FC0 &&
           readStart.finalFunction == CdHal::kFn800390C8 &&
           readStart.readSyncRequestKnown &&
           actual.known &&
           actual.actionKind == CdHal::ActionKind::ReadSync800390C8 &&
           actual.callerFunction == CdHal::kFn8001A818 &&
           actual.lowerFunction == CdHal::kFn800390C8 &&
           actual.finalFunction == CdHal::kFn800364F0 &&
           actual.readSyncRequestKnown &&
           actual.readSyncFunction == readStart.readSyncFunction &&
           actual.readSyncArg0 == readStart.readSyncArg0 &&
           actual.readSyncArg1 == readStart.readSyncArg1;
}

bool IsFinalReadyProgressGapForPendingReadStart801C81EC(
    const Bootstrap15590CdLowerHostRequest801C81EC& request,
    uint32_t currentReadS27Serial) {
    namespace CdHal = PrStage1LoaderCdHal;
    const CdHal::LowerActionRequestMetadata& readStart =
        request.lowerRequest;
    return request.cdActionKind == CdHal::ActionKind::ReadStart80038FC0 &&
           request.status ==
               Bootstrap15590CdLowerAttemptStatus801C81EC::
                   RequestAlreadyPending &&
           request.readStartHalProgressAccepted &&
           request.readStartHalProgressReadS27Serial != 0u &&
           request.readStartHalProgressReadS27Serial == currentReadS27Serial &&
           readStart.known &&
           readStart.actionKind == CdHal::ActionKind::ReadStart80038FC0 &&
           readStart.callerFunction == CdHal::kFn8001A818 &&
           readStart.lowerFunction == CdHal::kFn80038FC0 &&
           readStart.finalFunction == CdHal::kFn800390C8 &&
           readStart.readSyncRequestKnown &&
           readStart.readSyncFunction == CdHal::kFn800390C8 &&
           readStart.readSyncArg0 == CdHal::kRead8001A818SyncArg0 &&
           readStart.readSyncArg1 == CdHal::kRead8001A818SyncArg1;
}

bool IsFinalReadyProgressGapForPendingReadStart801C81EC(
    const Bootstrap15590CdLowerObservableState801C81EC& observable,
    uint32_t currentReadS27Serial) {
    Bootstrap15590CdLowerHostRequest801C81EC request{};
    request.cdActionKind = observable.cdActionKind;
    request.status = observable.status;
    request.lowerRequest = observable.lowerRequest;
    request.readStartHalProgressAccepted =
        observable.readStartHalProgressAccepted;
    request.readStartHalProgressReadS27Serial =
        observable.readStartHalProgressReadS27Serial;
    return IsFinalReadyProgressGapForPendingReadStart801C81EC(
        request,
        currentReadS27Serial);
}

} // namespace PrStage1LifecycleExecutorDirect
