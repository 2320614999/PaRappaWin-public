#include "pr/pr_stage1_bootstrap_cd_request_direct.h"

#include <cstdlib>
#include <iostream>

namespace {

using namespace PrStage1LifecycleExecutorDirect;
namespace CdHal = PrStage1LoaderCdHal;
namespace Loader = PrStage1LoaderDirect;

void Check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "failed: " << message << "\n";
        std::exit(1);
    }
}

Loader::ProducerStep ReadStartStep(uint32_t psxOrder,
                                   uint16_t recordIndex,
                                   uint8_t attemptIndex) {
    Loader::ProducerStep step{};
    step.valid = true;
    step.stepKind = Loader::ProducerStepKind::CdBoundary;
    step.category = Loader::ProducerCategory::Cd;
    step.psxOrder = psxOrder;
    step.recordIndex = recordIndex;
    step.recordType = Loader::LoaderRecordType::Type1Tim;
    step.cdActionKind = CdHal::ActionKind::ReadStart80038FC0;
    step.action.kind = Loader::ActionKind::ReadHeader1A818;
    step.action.boundary = Loader::ActionBoundary::CdHalBoundary;
    step.action.psxOrder = psxOrder;
    step.action.recordIndex = recordIndex;
    step.action.recordType = Loader::LoaderRecordType::Type1Tim;
    step.action.attemptIndex = attemptIndex;
    step.action.cd.actionKind = CdHal::ActionKind::ReadStart80038FC0;
    step.action.cd.callerFunction = CdHal::kFn8001A818;
    step.action.cd.lowerFunction = CdHal::kFn80038FC0;
    step.action.cd.finalFunction = CdHal::kFn800390C8;
    step.action.cd.dstPtr = 0x80123400u;
    step.action.cd.sectorCount = 2;
    return step;
}

Bootstrap15590CdLowerHostRequest801C81EC ReadStartRequest() {
    Bootstrap15590CdLowerHostRequest801C81EC request{};
    request.valid = true;
    request.waitingForCd = true;
    request.status = Bootstrap15590CdLowerAttemptStatus801C81EC::RequestReady;
    request.stepKind = Loader::ProducerStepKind::CdBoundary;
    request.cdActionKind = CdHal::ActionKind::ReadStart80038FC0;
    request.payloadBytesRequired = false;
    request.readDstPtrKnown = true;
    request.readDstPtr = 0x80123400u;
    request.readSectorCountKnown = true;
    request.readSectorCount = 2;
    request.lowerRequest.known = true;
    request.lowerRequest.actionKind = CdHal::ActionKind::ReadStart80038FC0;
    request.lowerRequest.callerFunction = CdHal::kFn8001A818;
    request.lowerRequest.lowerFunction = CdHal::kFn80038FC0;
    request.lowerRequest.finalFunction = CdHal::kFn800390C8;
    request.lowerRequest.readStartRequestKnown = true;
    request.lowerRequest.readStartDstPtrKnown = true;
    request.lowerRequest.readStartDstPtr = request.readDstPtr;
    request.lowerRequest.readStartSectorCountKnown = true;
    request.lowerRequest.readStartSectorCount = request.readSectorCount;
    request.lowerRequest.readSyncRequestKnown = true;
    request.lowerRequest.readSyncFunction = CdHal::kFn800390C8;
    request.lowerRequest.readSyncArg0 = CdHal::kRead8001A818SyncArg0;
    request.lowerRequest.readSyncArg1 = CdHal::kRead8001A818SyncArg1;
    return request;
}

CdHal::LowerActionRequestMetadata ReadSyncRequest() {
    CdHal::LowerActionRequestMetadata request{};
    request.known = true;
    request.actionKind = CdHal::ActionKind::ReadSync800390C8;
    request.callerFunction = CdHal::kFn8001A818;
    request.lowerFunction = CdHal::kFn800390C8;
    request.finalFunction = CdHal::kFn800364F0;
    request.readSyncRequestKnown = true;
    request.readSyncFunction = CdHal::kFn800390C8;
    request.readSyncArg0 = CdHal::kRead8001A818SyncArg0;
    request.readSyncArg1 = CdHal::kRead8001A818SyncArg1;
    return request;
}

void TestSameKeyPendingRequestCarriesAcceptedReadStartProgress() {
    Bootstrap15590CdLowerProducerRuntime801C81EC runtime{};
    const Loader::ProducerStep step = ReadStartStep(0x15590u, 4, 1);
    const Bootstrap15590CdLowerHostRequest801C81EC request =
        ReadStartRequest();

    MarkBootstrap15590CdLowerRequestPending801C81EC(runtime, step, request);
    runtime.readStartHalProgressAccepted = true;
    runtime.readStartHalProgressReadS27Serial = 0x44u;

    Bootstrap15590CdLowerHostRequest801C81EC out = request;
    const bool carried =
        TryCarryBootstrap15590CdLowerSameKeyPendingRequest801C81EC(
            runtime,
            step,
            out);

    Check(carried, "same-key pending request should carry progress");
    Check(out.status ==
              Bootstrap15590CdLowerAttemptStatus801C81EC::RequestAlreadyPending,
          "same-key carry should surface RequestAlreadyPending");
    Check(out.readStartHalProgressAccepted,
          "same-key carry should preserve accepted progress bit");
    Check(out.readStartHalProgressReadS27Serial == 0x44u,
          "same-key carry should preserve accepted progress serial");
    Check(runtime.status ==
              Bootstrap15590CdLowerAttemptStatus801C81EC::RequestAlreadyPending,
          "same-key carry should update runtime pending status");
}

void TestFreshPendingRequestResetsStaleReadStartProgress() {
    Bootstrap15590CdLowerProducerRuntime801C81EC runtime{};
    const Loader::ProducerStep firstStep = ReadStartStep(0x15590u, 4, 1);
    const Loader::ProducerStep nextStep = ReadStartStep(0x15591u, 4, 1);
    const Bootstrap15590CdLowerHostRequest801C81EC request =
        ReadStartRequest();

    MarkBootstrap15590CdLowerRequestPending801C81EC(runtime, firstStep, request);
    runtime.readStartHalProgressAccepted = true;
    runtime.readStartHalProgressReadS27Serial = 0x55u;

    Bootstrap15590CdLowerHostRequest801C81EC out = request;
    const bool staleCarried =
        TryCarryBootstrap15590CdLowerSameKeyPendingRequest801C81EC(
            runtime,
            nextStep,
            out);
    Check(!staleCarried, "different-key pending request must not carry stale progress");
    Check(!out.readStartHalProgressAccepted,
          "different-key carry failure must leave out progress false");
    Check(out.readStartHalProgressReadS27Serial == 0,
          "different-key carry failure must leave out serial zero");

    MarkBootstrap15590CdLowerRequestPending801C81EC(runtime, nextStep, request);
    Check(runtime.status ==
              Bootstrap15590CdLowerAttemptStatus801C81EC::RequestReady,
          "fresh pending request should become RequestReady");
    Check(!runtime.readStartHalProgressAccepted,
          "fresh pending request must reset stale accepted progress");
    Check(runtime.readStartHalProgressReadS27Serial == 0,
          "fresh pending request must reset stale progress serial");
    Check(runtime.psxOrder == nextStep.psxOrder,
          "fresh pending request should update runtime key");
}

void TestReadSyncCompletionRequiresAcceptedReadStartProgress() {
    Bootstrap15590CdLowerHostRequest801C81EC pending = ReadStartRequest();
    pending.status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::RequestAlreadyPending;
    pending.readStartHalProgressAccepted = true;
    pending.readStartHalProgressReadS27Serial = 0x66u;

    const CdHal::LowerActionRequestMetadata readSync = ReadSyncRequest();
    Check(IsReadSyncCompletionForPendingReadStart801C81EC(pending, readSync),
          "accepted read-start progress should authorize exact ReadSync completion shape");

    Bootstrap15590CdLowerHostRequest801C81EC notAccepted = pending;
    notAccepted.readStartHalProgressAccepted = false;
    Check(!IsReadSyncCompletionForPendingReadStart801C81EC(notAccepted,
                                                           readSync),
          "ReadSync completion must reject missing accepted read-start progress");

    Bootstrap15590CdLowerHostRequest801C81EC zeroSerial = pending;
    zeroSerial.readStartHalProgressReadS27Serial = 0;
    Check(!IsReadSyncCompletionForPendingReadStart801C81EC(zeroSerial,
                                                           readSync),
          "ReadSync completion must reject zero read-start progress serial");

    CdHal::LowerActionRequestMetadata wrongSyncArg = readSync;
    wrongSyncArg.readSyncArg1 = 1;
    Check(!IsReadSyncCompletionForPendingReadStart801C81EC(pending,
                                                           wrongSyncArg),
          "ReadSync completion must reject mismatched sync args");

    CdHal::LowerActionRequestMetadata wrongFinalFunction = readSync;
    wrongFinalFunction.finalFunction = CdHal::kFn800390C8;
    Check(!IsReadSyncCompletionForPendingReadStart801C81EC(
              pending,
              wrongFinalFunction),
          "ReadSync completion must reject non-final-ready function chain");
}

void TestFinalReadyGapShapeRequiresCurrentReadStartProgressSerial() {
    Bootstrap15590CdLowerHostRequest801C81EC pending = ReadStartRequest();
    pending.status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::RequestAlreadyPending;
    pending.readStartHalProgressAccepted = true;
    pending.readStartHalProgressReadS27Serial = 0x77u;

    Check(IsFinalReadyProgressGapForPendingReadStart801C81EC(pending, 0x77u),
          "final-ready gap shape should accept current read-start progress serial");
    Check(!IsFinalReadyProgressGapForPendingReadStart801C81EC(pending, 0x78u),
          "final-ready gap shape must reject stale read-start progress serial");

    Bootstrap15590CdLowerHostRequest801C81EC notAccepted = pending;
    notAccepted.readStartHalProgressAccepted = false;
    Check(!IsFinalReadyProgressGapForPendingReadStart801C81EC(notAccepted,
                                                             0x77u),
          "final-ready gap shape must reject missing accepted read-start progress");

    Bootstrap15590CdLowerHostRequest801C81EC zeroSerial = pending;
    zeroSerial.readStartHalProgressReadS27Serial = 0;
    Check(!IsFinalReadyProgressGapForPendingReadStart801C81EC(zeroSerial,
                                                             0u),
          "final-ready gap shape must reject zero progress serial");

    Bootstrap15590CdLowerHostRequest801C81EC wrongSyncArg = pending;
    wrongSyncArg.lowerRequest.readSyncArg0 = 0;
    Check(!IsFinalReadyProgressGapForPendingReadStart801C81EC(wrongSyncArg,
                                                             0x77u),
          "final-ready gap shape must reject mismatched read-sync args");
}

void TestFinalReadyGapObservableShapeMirrorsHostRequest() {
    Bootstrap15590CdLowerHostRequest801C81EC pending = ReadStartRequest();
    pending.status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::RequestAlreadyPending;
    pending.readStartHalProgressAccepted = true;
    pending.readStartHalProgressReadS27Serial = 0x99u;

    Bootstrap15590CdLowerObservableState801C81EC observable{};
    observable.cdActionKind = pending.cdActionKind;
    observable.status = pending.status;
    observable.lowerRequest = pending.lowerRequest;
    observable.readStartHalProgressAccepted =
        pending.readStartHalProgressAccepted;
    observable.readStartHalProgressReadS27Serial =
        pending.readStartHalProgressReadS27Serial;

    Check(IsFinalReadyProgressGapForPendingReadStart801C81EC(observable,
                                                             0x99u),
          "observable final-ready gap shape should mirror host request");
    Check(!IsFinalReadyProgressGapForPendingReadStart801C81EC(observable,
                                                              0x98u),
          "observable final-ready gap shape must reject stale serial");

    observable.readStartHalProgressAccepted = false;
    Check(!IsFinalReadyProgressGapForPendingReadStart801C81EC(observable,
                                                              0x99u),
          "observable final-ready gap shape must reject missing accepted progress");
}

void TestFinalReadyHalFactsRequirementRecordsOnlyCurrentShape() {
    Loader::ProducerStep step = ReadStartStep(0x44u, 3u, 1u);
    Bootstrap15590CdLowerProducerRuntime801C81EC runtime{};
    Bootstrap15590CdLowerHostRequest801C81EC request = ReadStartRequest();
    MarkBootstrap15590CdLowerRequestPending801C81EC(runtime, step, request);
    runtime.readStartHalProgressAccepted = true;
    runtime.readStartHalProgressReadS27Serial = 0x55u;

    Bootstrap15590CdLowerHostRequest801C81EC pending = request;
    Check(TryCarryBootstrap15590CdLowerSameKeyPendingRequest801C81EC(
              runtime,
              step,
              pending),
          "same-key pending request should carry read-start progress before final-ready requirement");
    Check(MarkBootstrap15590FinalReadyHalFactsRequired801C81EC(runtime,
                                                              pending,
                                                              0x55u),
          "final-ready HAL facts requirement should record current progress shape");
    Check(runtime.finalReadyHalFactsRequired,
          "final-ready HAL facts requirement should set required state");
    Check(runtime.finalReadyHalFactsReadS27Serial == 0x55u,
          "final-ready HAL facts requirement should record current serial");

    Bootstrap15590CdLowerProducerRuntime801C81EC staleRuntime{};
    Bootstrap15590CdLowerHostRequest801C81EC stale = pending;
    Check(!MarkBootstrap15590FinalReadyHalFactsRequired801C81EC(staleRuntime,
                                                               stale,
                                                               0x56u),
          "final-ready HAL facts requirement must reject stale progress serial");
    Check(!staleRuntime.finalReadyHalFactsRequired,
          "stale final-ready requirement must not set required state");

    runtime.finalReadyHalFactsRequired = true;
    runtime.finalReadyHalFactsReadS27Serial = 0x55u;
    MarkBootstrap15590CdLowerRequestPending801C81EC(runtime, step, request);
    Check(!runtime.finalReadyHalFactsRequired,
          "fresh pending request must clear final-ready facts requirement");
    Check(runtime.finalReadyHalFactsReadS27Serial == 0u,
          "fresh pending request must clear final-ready facts serial");
}

void TestFinalReadyHalFactsRequirementMustBeCurrent() {
    Loader::ProducerStep step = ReadStartStep(0x45u, 4u, 1u);
    Bootstrap15590CdLowerProducerRuntime801C81EC runtime{};
    Bootstrap15590CdLowerHostRequest801C81EC request = ReadStartRequest();
    MarkBootstrap15590CdLowerRequestPending801C81EC(runtime, step, request);
    runtime.readStartHalProgressAccepted = true;
    runtime.readStartHalProgressReadS27Serial = 0x66u;

    Bootstrap15590CdLowerHostRequest801C81EC pending = request;
    Check(TryCarryBootstrap15590CdLowerSameKeyPendingRequest801C81EC(
              runtime,
              step,
              pending),
          "same-key pending request should carry progress before requirement-current check");
    Check(!IsBootstrap15590FinalReadyHalFactsRequirementCurrent801C81EC(
              runtime,
              pending,
              0x66u),
          "final-ready facts requirement current check must reject missing marker");
    Check(MarkBootstrap15590FinalReadyHalFactsRequired801C81EC(runtime,
                                                              pending,
                                                              0x66u),
          "final-ready facts requirement should record before current check");
    Check(IsBootstrap15590FinalReadyHalFactsRequirementCurrent801C81EC(
              runtime,
              pending,
              0x66u),
          "final-ready facts requirement current check should accept current marker");
    Check(!IsBootstrap15590FinalReadyHalFactsRequirementCurrent801C81EC(
              runtime,
              pending,
              0x67u),
          "final-ready facts requirement current check must reject stale serial");
}

} // namespace

int main() {
    TestSameKeyPendingRequestCarriesAcceptedReadStartProgress();
    TestFreshPendingRequestResetsStaleReadStartProgress();
    TestReadSyncCompletionRequiresAcceptedReadStartProgress();
    TestFinalReadyGapShapeRequiresCurrentReadStartProgressSerial();
    TestFinalReadyGapObservableShapeMirrorsHostRequest();
    TestFinalReadyHalFactsRequirementRecordsOnlyCurrentShape();
    TestFinalReadyHalFactsRequirementMustBeCurrent();
    std::cout << "bootstrap CD request tests passed\n";
    return 0;
}
