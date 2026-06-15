#include "pr_stage1_lifecycle_executor_direct.h"

#include "pr_stage1_bootstrap_cd_request_direct.h"
#include "pr_stage1_loader_producer_adapter.h"
#include "pr_stage1_xa_cd_direct.h"

#include <memory>
#include <system_error>

namespace PrStage1LifecycleExecutorDirect {

struct LoaderOwnerState801C81EC {
    LoaderOwnerState801C81EC();

    PrStage1LoaderDirect::RunnerState runnerState;
    PrStage1LoaderProducerAdapter::ProducerRuntime producerRuntime;
};

namespace {

constexpr uint32_t kFn800166AC = 0x800166ACu;
constexpr uint32_t kFn8001635C = 0x8001635Cu;
constexpr uint32_t kFn8001628C = 0x8001628Cu;
constexpr uint32_t kFn80019148 = 0x80019148u;
constexpr uint32_t kAddrSavePayloadBank80092F10 = 0x80092F10u;

bool TryResolveStage1MovieSegmentIso9660BinPath801C81EC(
    const std::filesystem::path& dataRoot,
    std::filesystem::path& outPath) {
    outPath.clear();
    std::error_code ec;
    std::filesystem::path p = dataRoot;
    for (int i = 0; i < 8 && !p.empty(); ++i) {
        const std::filesystem::path candidate =
            p / "PaRappa the Rapper.bin";
        if (std::filesystem::exists(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const std::filesystem::path parent = p.parent_path();
        if (parent == p) {
            break;
        }
        p = parent;
    }

    p = std::filesystem::current_path(ec);
    for (int i = 0; i < 8 && !p.empty(); ++i) {
        const std::filesystem::path candidate =
            p / "PaRappa the Rapper.bin";
        if (std::filesystem::exists(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const std::filesystem::path parent = p.parent_path();
        if (parent == p) {
            break;
        }
        p = parent;
    }
    return false;
}

LoaderOwnerState801C81EC& EnsureLoaderOwnerState801C81EC(
    State801C81EC& state) {
    if (!state.loaderOwner) {
        state.loaderOwner = std::make_unique<LoaderOwnerState801C81EC>();
    }
    return *state.loaderOwner;
}

const LoaderOwnerState801C81EC& EnsureLoaderOwnerState801C81EC(
    const State801C81EC& state) {
    return *state.loaderOwner;
}

bool IsBootstrap15590LoaderTerminal801C81EC(
    const PrStage1LoaderDirect::RunnerState& runnerState) {
    return PrStage1LoaderDirect::IsTerminalStatus(runnerState.status);
}

void SyncBootstrap15590LoaderDirectFlags801C81EC(
    State801C81EC& state,
    const PrStage1LoaderDirect::RunnerState& runnerState) {
    state.bootstrap15590LoaderDirectWaitingExternal =
        PrStage1LoaderDirect::IsWaitingForFeedback(runnerState);
    state.bootstrap15590LoaderDirectCompleted =
        runnerState.status == PrStage1LoaderDirect::RunnerStatus::Completed;
    state.bootstrap15590LoaderDirectFailed =
        runnerState.status == PrStage1LoaderDirect::RunnerStatus::Failed;
}

void ResetBootstrap15590LoaderDirectRuntime801C81EC(State801C81EC& state) {
    LoaderOwnerState801C81EC& owner = EnsureLoaderOwnerState801C81EC(state);
    PrStage1LoaderDirect::Reset(owner.runnerState);
    PrStage1LoaderProducerAdapter::ResetProducerRuntime(owner.producerRuntime);
    state.bootstrap15590LoaderDirectBeginAttempted = false;
    state.bootstrap15590LoaderDirectBegun = false;
    state.bootstrap15590LoaderDirectBeginSucceeded = false;
    state.bootstrap15590LoaderDirectBeginFailed = false;
    state.bootstrap15590LoaderDirectPumpStarted = false;
    state.bootstrap15590LoaderDirectWaitingExternal = false;
    state.bootstrap15590LoaderDirectCompleted = false;
    state.bootstrap15590LoaderDirectFailed = false;
    state.bootstrap15590LoaderDirectLastPumpActionCount = 0;
    state.bootstrap15590LoaderSpuHalState = PrStage1LoaderSpuHal::State{};
    PrStage1MovieSegmentDirect::ResetStage1MovieSegmentCdLookupRuntime801C4780(
        state.bootstrap15590MovieSegmentCdLookupRuntime);
    state.bootstrap15590CdLookupLowerProducerRuntime =
        Bootstrap15590CdLookupLowerProducerRuntime801C81EC{};
    state.bootstrap15590CdLowerProducerRuntime =
        Bootstrap15590CdLowerProducerRuntime801C81EC{};
    state.bootstrap15590CdLowerLivePayloadBytes.clear();
    state.bootstrap15590CdLowerFileBaseLbaKnown = false;
    state.bootstrap15590CdLowerFileBaseLba = 0;
    state.bootstrap15590CdLowerLastSeekLbaKnown = false;
    state.bootstrap15590CdLowerLastSeekLba = 0;
}

void RecordBootstrap15590MovieSegmentCdLookupSeam801C81EC(
    State801C81EC& state,
    const PrStage1LoaderDirect::CdSeamResult& cd) {
    (void)PrStage1MovieSegmentDirect::
        RecordStage1MovieSegmentCdLookupSeam801C4780(
            state.bootstrap15590MovieSegmentCdLookupRuntime,
            PrStage1MovieSegmentDirect::
                kStage1SceneEntryMovieSegmentSceneIndex801C4780,
            cd);
}

bool IsBootstrap15590ExplicitMovieSegmentCdLookupSeam801C81EC(
    const PrStage1LoaderDirect::CdSeamResult& cd) {
    return cd.present && cd.lookup800381F8Known &&
           cd.probe8001A2B0Known && cd.probe8001A2B0.known &&
           cd.probe8001A2B0.complete;
}

void RecordBootstrap15590MovieSegmentCdLookupTypedFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::TypedActionFeedback& feedback) {
    if (!feedback.valid || !feedback.cdFeedback ||
        !feedback.feedback.hasCdResult ||
        !IsBootstrap15590ExplicitMovieSegmentCdLookupSeam801C81EC(
            feedback.feedback.cdResult)) {
        return;
    }
    RecordBootstrap15590MovieSegmentCdLookupSeam801C81EC(
        state,
        feedback.feedback.cdResult);
}

PrStage1LoaderDirect::CdSeamResult
BuildBootstrap15590CdLookupSeamWithExplicitClock801C81EC(
    const PrStage1LoaderProducerAdapter::CdLookupLiveInput& input,
    const PrStage1XaCdDirectState* explicitXaCdState,
    const PrStage1XaCdDirectStreamClockProbe800493F4*
        explicitStreamClockProbe,
    const PrMovieSegmentDirect::StreamClockProducerCarrier800493F4*
        explicitStreamClockCarrier) {
    PrStage1LoaderDirect::CdSeamResult cd =
        PrStage1LoaderProducerAdapter::BuildCdLookupLiveSeamResult(input);
    if (explicitStreamClockCarrier) {
        return PrStage1LoaderDirect::
            AttachStreamClockProducerCarrierToCdSeamResult800493F4(
                cd,
                *explicitStreamClockCarrier);
    }
    if (explicitStreamClockProbe) {
        return PrStage1LoaderDirect::
            AttachStreamClockProducerCarrierToCdSeamResult800493F4(
                cd,
                explicitStreamClockProbe->carrier);
    }
    if (explicitXaCdState) {
        const PrStage1XaCdDirectStreamClockProbe800493F4 probe =
            PrStage1XaCdDirectProbeStreamClockProducer800493F4(
                *explicitXaCdState);
        return PrStage1LoaderDirect::
            AttachStreamClockProducerCarrierToCdSeamResult800493F4(
                cd,
                probe.carrier);
    }
    return cd;
}

void FillBootstrap15590LoaderSnapshot801C81EC(
    const State801C81EC& state,
    HostBlockSnapshot801C81EC& out) {
    out.bootstrap15590LoaderDirectBeginAttempted =
        state.bootstrap15590LoaderDirectBeginAttempted;
    out.bootstrap15590LoaderDirectBegun =
        state.bootstrap15590LoaderDirectBegun;
    out.bootstrap15590LoaderDirectBeginSucceeded =
        state.bootstrap15590LoaderDirectBeginSucceeded;
    out.bootstrap15590LoaderDirectBeginFailed =
        state.bootstrap15590LoaderDirectBeginFailed;
    out.bootstrap15590LoaderDirectPumpStarted =
        state.bootstrap15590LoaderDirectPumpStarted;
    out.bootstrap15590LoaderDirectWaitingExternal =
        state.bootstrap15590LoaderDirectWaitingExternal;
    out.bootstrap15590LoaderDirectCompleted =
        state.bootstrap15590LoaderDirectCompleted;
    out.bootstrap15590LoaderDirectFailed =
        state.bootstrap15590LoaderDirectFailed;
    out.bootstrap15590LoaderDirectLastPumpActionCount =
        state.bootstrap15590LoaderDirectLastPumpActionCount;
    if (!state.loaderOwner) {
        return;
    }
    const PrStage1LoaderDirect::RunnerState& runnerState =
        state.loaderOwner->runnerState;
    out.bootstrap15590LoaderDirectActionCount = runnerState.actions.size();
    out.bootstrap15590LoaderDirectNextActionIndex =
        runnerState.nextActionIndex;
    out.bootstrap15590LoaderDirectWaitingForFeedback =
        PrStage1LoaderDirect::IsWaitingForFeedback(runnerState);
    out.bootstrap15590CdLowerRequestPending =
        state.bootstrap15590CdLowerProducerRuntime.requestPending;
    out.bootstrap15590CdLower.keyKnown =
        state.bootstrap15590CdLowerProducerRuntime.keyKnown;
    out.bootstrap15590CdLower.requestPending =
        state.bootstrap15590CdLowerProducerRuntime.requestPending;
    out.bootstrap15590CdLower.status =
        state.bootstrap15590CdLowerProducerRuntime.status;
    out.bootstrap15590CdLower.psxOrder =
        state.bootstrap15590CdLowerProducerRuntime.psxOrder;
    out.bootstrap15590CdLower.stepKind =
        state.bootstrap15590CdLowerProducerRuntime.stepKind;
    out.bootstrap15590CdLower.cdActionKind =
        state.bootstrap15590CdLowerProducerRuntime.cdActionKind;
    out.bootstrap15590CdLower.recordIndex =
        state.bootstrap15590CdLowerProducerRuntime.recordIndex;
    out.bootstrap15590CdLower.recordType =
        state.bootstrap15590CdLowerProducerRuntime.recordType;
    out.bootstrap15590CdLower.attemptIndex =
        state.bootstrap15590CdLowerProducerRuntime.attemptIndex;
    out.bootstrap15590CdLower.lowerFunction =
        state.bootstrap15590CdLowerProducerRuntime.lowerFunction;
    out.bootstrap15590CdLower.finalFunction =
        state.bootstrap15590CdLowerProducerRuntime.finalFunction;
    out.bootstrap15590CdLower.lowerRequest =
        state.bootstrap15590CdLowerProducerRuntime.lowerRequest;
    out.bootstrap15590CdLower.payloadBytesRequired =
        state.bootstrap15590CdLowerProducerRuntime.payloadBytesRequired;
    out.bootstrap15590CdLower.seekLbaKnown =
        state.bootstrap15590CdLowerProducerRuntime.seekLbaKnown;
    out.bootstrap15590CdLower.seekLba =
        state.bootstrap15590CdLowerProducerRuntime.seekLba;
    out.bootstrap15590CdLower.readDstPtrKnown =
        state.bootstrap15590CdLowerProducerRuntime.readDstPtrKnown;
    out.bootstrap15590CdLower.readDstPtr =
        state.bootstrap15590CdLowerProducerRuntime.readDstPtr;
    out.bootstrap15590CdLower.readSectorCountKnown =
        state.bootstrap15590CdLowerProducerRuntime.readSectorCountKnown;
    out.bootstrap15590CdLower.readSectorCount =
        state.bootstrap15590CdLowerProducerRuntime.readSectorCount;
    out.bootstrap15590CdLower.readStartHalProgressAccepted =
        state.bootstrap15590CdLowerProducerRuntime.
            readStartHalProgressAccepted;
    out.bootstrap15590CdLower.readStartHalProgressReadS27Serial =
        state.bootstrap15590CdLowerProducerRuntime.
            readStartHalProgressReadS27Serial;
    out.bootstrap15590CdLower.finalReadyHalFactsRequired =
        state.bootstrap15590CdLowerProducerRuntime.
            finalReadyHalFactsRequired;
    out.bootstrap15590CdLower.finalReadyHalFactsReadS27Serial =
        state.bootstrap15590CdLowerProducerRuntime.
            finalReadyHalFactsReadS27Serial;
    out.bootstrap15590CdLower.cdSyncLoopFactsRequired80037070 =
        state.bootstrap15590CdLowerProducerRuntime.
            cdSyncLoopFactsRequired80037070;
    out.bootstrap15590CdLower.cdSyncLoopFunction80037070 =
        state.bootstrap15590CdLowerProducerRuntime.
            cdSyncLoopFunction80037070;
    out.bootstrap15590CdLower.cdSyncLoopA0WaitModeKnown80037070 =
        state.bootstrap15590CdLowerProducerRuntime.
            cdSyncLoopA0WaitModeKnown80037070;
    out.bootstrap15590CdLower.cdSyncLoopA0WaitMode80037070 =
        state.bootstrap15590CdLowerProducerRuntime.
            cdSyncLoopA0WaitMode80037070;
    out.bootstrap15590CdLower.lastRejectKnown =
        state.bootstrap15590CdLowerProducerRuntime.lastRejectKnown;
    out.bootstrap15590CdLower.lastRejectReason =
        state.bootstrap15590CdLowerProducerRuntime.lastRejectReason;
    out.bootstrap15590CdLower.lastRejectStatus =
        state.bootstrap15590CdLowerProducerRuntime.lastRejectStatus;
    out.bootstrap15590CdLower.lastRejectExpected =
        state.bootstrap15590CdLowerProducerRuntime.lastRejectExpected;
    out.bootstrap15590CdLower.lastRejectActual =
        state.bootstrap15590CdLowerProducerRuntime.lastRejectActual;
    out.saveUi19148LowerFeedbackRequests =
        state.saveUi19148LowerFeedbackRequests;
}

bool BuildBootstrap15590PadComFeedback801C81EC(
    const PrStage1LoaderDirect::ProducerStep& waitingStep,
    PrStage1LoaderSpuHal::State& candidateSpuState,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& out) {
    if (waitingStep.category !=
        PrStage1LoaderDirect::ProducerCategory::PadCom) {
        out = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
        return false;
    }

    PrStage1LoaderSpuHal::ApplyPadStartComAudioGlobalResetContract(
        candidateSpuState);
    return PrStage1LoaderProducerAdapter::BuildPadComTypedFeedback(
        waitingStep,
        true,
        true,
        0,
        out);
}

bool BuildBootstrap15590VabCloseFeedback801C81EC(
    const PrStage1LoaderDirect::ProducerStep& waitingStep,
    PrStage1LoaderSpuHal::State& candidateSpuState,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& out) {
    if (waitingStep.category !=
            PrStage1LoaderDirect::ProducerCategory::Spu ||
        waitingStep.action.kind !=
            PrStage1LoaderDirect::ActionKind::VabClose27120) {
        out = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
        return false;
    }

    PrStage1LoaderDirect::SpuSeamResult spu{};
    spu.present = true;
    spu.actionKind = waitingStep.action.spu.actionKind;
    (void)PrStage1LoaderSpuHal::Apply80027120(
        candidateSpuState,
        &spu.actionList);
    spu.actionListKnown = true;
    return PrStage1LoaderProducerAdapter::BuildSpuTypedFeedback(
        waitingStep,
        spu,
        true,
        out);
}

bool BuildBootstrap15590LocalHalFeedback801C81EC(
    const PrStage1LoaderDirect::ProducerStep& waitingStep,
    PrStage1LoaderSpuHal::State& candidateSpuState,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& out) {
    if (BuildBootstrap15590PadComFeedback801C81EC(
            waitingStep,
            candidateSpuState,
            out)) {
        return true;
    }
    return BuildBootstrap15590VabCloseFeedback801C81EC(
        waitingStep,
        candidateSpuState,
        out);
}

constexpr uint32_t kFn800201AC = 0x800201ACu;
constexpr uint32_t kFn80020110 = 0x80020110u;
constexpr uint32_t kFn80020090 = 0x80020090u;
constexpr uint32_t kFn80020008 = 0x80020008u;
constexpr uint32_t kFn80015590 = 0x80015590u;
constexpr uint32_t kFn80015408 = 0x80015408u;
constexpr uint32_t kFn8001545C = 0x8001545Cu;
constexpr uint32_t kFn8001537C = 0x8001537Cu;
constexpr uint32_t kFn8001AC18 = 0x8001AC18u;
constexpr uint32_t kFn80026ECC = 0x80026ECCu;
constexpr uint32_t kFn80025A00 = 0x80025A00u;
constexpr uint32_t kFn80025A34 = 0x80025A34u;
constexpr uint32_t kFn8001A2B0 = 0x8001A2B0u;
constexpr uint32_t kFn8001A324 = 0x8001A324u;
constexpr uint32_t kFn80036A78 = 0x80036A78u;
constexpr uint32_t kFn800381F8 = 0x800381F8u;
constexpr uint32_t kFn8001A8F0 = 0x8001A8F0u;
constexpr uint32_t kFn8001A89C = 0x8001A89Cu;
constexpr uint32_t kFn8001A818 = 0x8001A818u;
constexpr uint32_t kFn80025B28 = 0x80025B28u;
constexpr uint32_t kFn80025A70 = 0x80025A70u;
constexpr uint32_t kFn80025AF8 = 0x80025AF8u;
constexpr uint32_t kFn8001AE7C = 0x8001AE7Cu;
constexpr uint32_t kFn80025BFC = 0x80025BFCu;
constexpr uint32_t kFn80025BBC = 0x80025BBCu;
constexpr uint32_t kFn80027120 = 0x80027120u;
constexpr uint32_t kFn80026E4C = 0x80026E4Cu;
constexpr uint32_t kFn80027078 = 0x80027078u;
constexpr uint32_t kFn800270D4 = 0x800270D4u;
constexpr uint32_t kFn800270FC = 0x800270FCu;
constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
constexpr uint32_t kFn80036974 = 0x80036974u;
constexpr uint32_t kFn800367A4 = 0x800367A4u;
constexpr uint32_t kFn80038FC0 = 0x80038FC0u;
constexpr uint32_t kFn80035560 = 0x80035560u;
constexpr uint32_t kFn800390C8 = 0x800390C8u;
constexpr uint32_t kFn800364F0 = 0x800364F0u;
constexpr uint32_t kFn80040EAC = 0x80040EACu;
constexpr uint32_t kFn80044D64 = 0x80044D64u;
constexpr uint32_t kFn80044B3C = 0x80044B3Cu;
constexpr uint32_t kFn800468E0 = 0x800468E0u;
constexpr uint32_t kFn80046FFC = 0x80046FFCu;
constexpr uint32_t kFn8002DF80 = 0x8002DF80u;
constexpr uint32_t kFn8002E3D8 = 0x8002E3D8u;
constexpr uint32_t kFn8002EB80 = 0x8002EB80u;
constexpr uint32_t kFn8002EEFC = 0x8002EEFCu;
constexpr uint32_t kFn800351B8 = 0x800351B8u;
constexpr uint32_t kFn80034240 = 0x80034240u;
constexpr uint32_t kFn8002EFF4 = 0x8002EFF4u;
constexpr uint32_t kFn8002DA78 = 0x8002DA78u;
constexpr uint32_t kFn8002B130 = 0x8002B130u;
constexpr uint32_t kFn8002A6AC = 0x8002A6ACu;
constexpr uint32_t kFn8002AA90 = 0x8002AA90u;
constexpr uint32_t kFn8002AB24 = 0x8002AB24u;
constexpr uint32_t kFn8001EA74 = 0x8001EA74u;
constexpr uint32_t kFn8004019C = 0x8004019Cu;
constexpr uint32_t kFn80040F90 = 0x80040F90u;
constexpr uint32_t kFn80040CC8 = 0x80040CC8u;
constexpr uint32_t kFn8001EF40 = 0x8001EF40u;
constexpr uint32_t kFn8001F524 = 0x8001F524u;
constexpr uint32_t kFn8001FCBC = 0x8001FCBCu;
constexpr uint32_t kFn80022CBC = 0x80022CBCu;
constexpr uint32_t kFn8001D74C = 0x8001D74Cu;
constexpr uint32_t kFn8001FC40 = 0x8001FC40u;
constexpr uint32_t kFn80021E60 = 0x80021E60u;
constexpr uint32_t kFn8001FDC0 = 0x8001FDC0u;
constexpr uint32_t kFn80020308 = 0x80020308u;
constexpr uint32_t kFn80020248 = 0x80020248u;
constexpr uint32_t kFn8001C4EC = 0x8001C4ECu;
constexpr uint32_t kFn8001C550 = 0x8001C550u;
constexpr uint32_t kWord800916E0 = 0x800916E0u;
constexpr uint32_t kWord800916DC = 0x800916DCu;
constexpr uint32_t kDword8006ECD4 = 0x8006ECD4u;
constexpr uint32_t kDword80094410 = 0x80094410u;
constexpr uint32_t kWord800943A8 = 0x800943A8u;
constexpr uint32_t kWord800943AA = 0x800943AAu;
constexpr uint32_t kWord800943AC = 0x800943ACu;
constexpr uint32_t kDword800943B4 = 0x800943B4u;
constexpr uint32_t kDword8006ED50 = 0x8006ED50u;
constexpr uint32_t kDword80091858 = 0x80091858u;
constexpr uint32_t kDword8009185C = 0x8009185Cu;
constexpr uint32_t kTransitionWork801C3640 = 0x801C3640u;
constexpr uint32_t kBootstrapZcompoLoaderOffset = 0x12Cu;
constexpr uint32_t kLoaderResetClearEnd80092854 = 0x80092854u;
constexpr uint32_t kPatternLiveGrid80087330 = 0x80087330u;
constexpr uint32_t kPatternLiveGridLastColumn8008736C = 0x8008736Cu;
constexpr uint32_t kPatternLiveGridEnd8008766C = 0x8008766Cu;
constexpr uint32_t kPatternTableMode0_80050730 = 0x80050730u;
constexpr uint32_t kPatternTableMode1_80050790 = 0x80050790u;
constexpr uint32_t kPatternTableMode2_800507F0 = 0x800507F0u;
constexpr uint32_t kPatternTableMode3_80050850 = 0x80050850u;
constexpr uint32_t kPatternFrameLimitTable800508B4 = 0x800508B4u;
constexpr uint32_t kPatternHighlightTextureTable800508B0 = 0x800508B0u;
constexpr uint32_t kPatternTileIndexTable80050420 = 0x80050420u;
constexpr uint32_t kPatternTileTextureTable80050720 = 0x80050720u;
constexpr int32_t kSub80020110Delay27194Frames = 30;
constexpr int16_t kSub80015408BootstrapCurtainMode = 3;
constexpr int32_t kSub80015408CallbackFlagValue = 0;
constexpr int32_t kSub8001537CCallbackFlagValue = 1;
constexpr int32_t kSub8001537CCallbackReturnValue = 1;
constexpr int32_t kSub8001537CDrawArg0 = 1;
constexpr int32_t kSub8001537CDrawArg1 = 0;
constexpr int32_t kSub8001AC18BootstrapLoaderMode = 1;
constexpr uint8_t kSub8001AC18MaxAttempts = 4;
constexpr int32_t kSub8001AC18FirstAttemptParserFlag = 1;
constexpr int32_t kSub8001AC18RetryParserFlag = 0;
constexpr uint8_t kSub8001AC18CopiedDescriptorFirstWord = 4;
constexpr uint8_t kSub8001AC18CopiedDescriptorWordCount = 6;
constexpr uint16_t kSub80025A34ClearDwordCount = 1024;
constexpr int8_t kSub80025A34ClearDirectionDwords = -1;
constexpr uint8_t kSub8001A324LoadedFlagWord = 3;
constexpr uint8_t kSub8001A324DescriptorPresentWord = 0;
constexpr uint8_t kSub8001A324DescriptorPayloadFirstWord = 4;
constexpr uint8_t kSub8001A324SizeBytesWord = 5;
constexpr uint8_t kSub8001A324StartSectorWord = 10;
constexpr uint8_t kSub8001A324EndSectorWord = 11;
constexpr uint8_t kSub8001A324SectorShift = 11;
constexpr int32_t kSub8001A324AlreadyLoadedOrMissingReturn = 0;
constexpr int32_t kSub8001A324ProbeFailedReturn = -1;
constexpr int32_t kSub8001A324OpenedReturn = 0;
constexpr uint8_t kSub8001A2B0RetryCount = 4;
constexpr uint32_t kSub8001A8F0TempAllocBytes = 0x2000u;
constexpr uint8_t kSub8001A8F0HeaderReadBytes = 4;
constexpr int32_t kSub8001A8F0InitialSeekOffset = 0;
constexpr int32_t kSub8001A8F0HeaderSeekOffset = 4;
constexpr int32_t kSub8001A8F0RecordTypeEnd = -1;
constexpr int32_t kSub8001A8F0RecordTypeRegisterBlocks = 1;
constexpr int32_t kSub8001A8F0RecordTypePadSpu = 2;
constexpr int32_t kSub8001A8F0RecordTypeSplitBlocks = 3;
constexpr uint16_t kSub8001A8F0UnknownTypeErrorGpOffset = 744;
constexpr uint8_t kSub8001A8F0RecordCountWord = 1;
constexpr uint8_t kSub8001A8F0PayloadSectorCountWord = 2;
constexpr uint8_t kSub8001A8F0RecordPayloadFirstWord = 4;
constexpr uint8_t kSub8001A8F0RecordStrideWords = 5;
constexpr uint8_t kSub8001A8F0Type2PayloadBytesWord = 4;
constexpr uint8_t kSub8001A8F0Type2SpuBytesWord = 9;
constexpr uint8_t kSub8001A8F0SectorToBytesShift = 11;
constexpr uint8_t kSub8001A8F0Type2SuccessFreeCount = 1;
constexpr uint8_t kSub8001A8F0Type2FailureFreeCount = 2;
constexpr uint8_t kSub8001A8F0Type1FreeCount = 1;
constexpr int32_t kSub8001A8F0SuccessReturn = 1;
constexpr int32_t kSub8001A8F0FailureReturn = 0;
constexpr int32_t kSub8001A89CSyncMode = 2;
constexpr int32_t kSub8001A89CSyncArg2 = 0;
constexpr int32_t kSub80036974LbaToMsfBiasSectors = 150;
constexpr uint8_t kSub80025AllocAlignmentBytes = 8;
constexpr int32_t kSub80025AllocPositiveRoundAdd = 7;
constexpr int32_t kSub80025AllocNegativeRoundAdd = 14;
constexpr uint32_t kSub80025A70MaxRequestBytes = 0x12CFFFu;
constexpr uint16_t kSub80025A70MaxStackDepth = 1024;
constexpr uint16_t kLoaderAllocStackDepthGpOffset = 324;
constexpr uint16_t kLoaderAllocLowWaterGpOffset = 320;
constexpr uint16_t kLoaderAllocHeapEndGpOffset = 900;
constexpr uint16_t kLoaderAllocHeapCursorGpOffset = 904;
constexpr uint16_t kSub80025BFCFirstIndex = 1;
constexpr int32_t kSub80025BFCMissingReturn = 0;
constexpr int32_t kSub80025BBCSuccessReturn = 1;
constexpr int32_t kSub80025BBCOverflowReturn = 0;
constexpr uint8_t kSub8001A818StreamedModeFlag = 0x80;
constexpr int32_t kSub8001A818ResetInputArg = 3;
constexpr int32_t kSub8001A818SyncArg0 = 1;
constexpr int32_t kSub8001A818SyncArg1 = 0;
constexpr int32_t kSub8001A818SuccessSyncResult = 0;
constexpr int32_t kSub80038FC0SectorSize = 512;
constexpr int32_t kSub80038FC0TransferUnit = 32;
constexpr int32_t kSub800390C8TimeoutVblanks = 1200;
constexpr int32_t kSub800390C8PumpAfterVblanks = 60;
constexpr uint8_t kSub8001AE7CTimInfoPayloadOffset = 4;
constexpr uint8_t kSub8001AE7CClutFlagShift = 3;
constexpr uint8_t kSub8001AE7CClutFlagMask = 1;
constexpr int32_t kSub8001AE7CDrawSyncArg = 0;
constexpr int32_t kSub80027120ClosedVabSentinel = -1;
constexpr int32_t kSub80027078OpenArg1 = -1;
constexpr int32_t kSub800270FCEnableArg = 1;
constexpr uint8_t kSub80026EF8CuePitchAdd = 24;
constexpr uint8_t kSub80026EF8CuePitchByteOffset = 2;
constexpr uint8_t kSub80026EF8CueVolumeByteOffset = 3;
constexpr int32_t kSub80026FA4ResetArg = 0;
constexpr int32_t kPadStartComInit4096Arg = 4096;
constexpr int32_t kPadStartComInit90Arg = 90;
constexpr int32_t kPadStartComCurrentVabIdInit = 0;
constexpr int32_t kPadStartComClosedSentinel = -1;
constexpr int32_t kPadStartComFlushBusyInit = 0;
constexpr int32_t kPadStartComReturnValue = -1;
constexpr uint16_t kSub8001EA74CurrentDisplayIndexGpOffset = 872;
constexpr uint16_t kSub8001EA74DrawActiveFlagGpOffset = 792;
constexpr uint8_t kSub8001EF40GridColumns = 16;
constexpr uint8_t kSub8001EF40GridRows = 12;
constexpr uint8_t kSub8001EF40TileWidth = 20;
constexpr uint8_t kSub8001EF40TileHeight = 20;
constexpr uint8_t kSub8001EF40AdvanceEveryFrames = 3;
constexpr uint8_t kSub8001EF40BitsPerPatternWord = 32;
constexpr uint16_t kSub8001EF40BitCursorGpOffset = 49;
constexpr uint16_t kSub8001EF40WordCursorGpOffset = 50;
constexpr uint16_t kSub8001EF40FrameCounterGpOffset = 51;

LoaderReset25A34DirectPlan801C81EC BuildLoaderReset25A34DirectPlan801C81EC() {
    LoaderReset25A34DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn80025A34;
    out.clearEndAddress = kLoaderResetClearEnd80092854;
    out.clearDwordCount = kSub80025A34ClearDwordCount;
    out.clearDirectionDwords = kSub80025A34ClearDirectionDwords;
    out.tailFunction = kFn80025A00;
    return out;
}

LoaderOpenFile1A324DirectPlan801C81EC
BuildLoaderOpenFile1A324DirectPlan801C81EC() {
    LoaderOpenFile1A324DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn8001A324;
    out.loadedFlagWord = kSub8001A324LoadedFlagWord;
    out.descriptorPresentWord = kSub8001A324DescriptorPresentWord;
    out.descriptorPayloadFirstWord =
        kSub8001A324DescriptorPayloadFirstWord;
    out.sizeBytesWord = kSub8001A324SizeBytesWord;
    out.startSectorWord = kSub8001A324StartSectorWord;
    out.endSectorWord = kSub8001A324EndSectorWord;
    out.sectorShift = kSub8001A324SectorShift;
    out.probeFunction = kFn8001A2B0;
    out.probeLowerLookupFunction = kFn800381F8;
    out.probeRetryCount = kSub8001A2B0RetryCount;
    out.resolveStartSectorFunction = kFn80036A78;
    out.alreadyLoadedOrMissingReturn =
        kSub8001A324AlreadyLoadedOrMissingReturn;
    out.probeFailedReturn = kSub8001A324ProbeFailedReturn;
    out.openedReturn = kSub8001A324OpenedReturn;
    out.probeReturnsOriginalPointer = true;
    out.probeZeroMeansFailure = true;
    return out;
}

LoaderSeek1A89CDirectPlan801C81EC BuildLoaderSeek1A89CDirectPlan801C81EC() {
    LoaderSeek1A89CDirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn8001A89C;
    out.resolveStartSectorFunction = kFn80036A78;
    out.seekSectorFunction = kFn80036974;
    out.syncFunction = kFn800367A4;
    out.syncMode = kSub8001A89CSyncMode;
    out.syncArg2 = kSub8001A89CSyncArg2;
    out.lbaToMsfBiasSectors = kSub80036974LbaToMsfBiasSectors;
    out.addsOffsetToResolvedSector = true;
    out.seekConvertsLbaToBcdMsf = true;
    out.syncFunctionIsCdWrapperBoundary = true;
    return out;
}

LoaderRead1A818DirectPlan801C81EC BuildLoaderRead1A818DirectPlan801C81EC() {
    LoaderRead1A818DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn8001A818;
    out.readStartFunction = kFn80038FC0;
    out.resetInputFunction = kFn80035560;
    out.readSyncFunction = kFn800390C8;
    out.resetInputArg = kSub8001A818ResetInputArg;
    out.syncArg0 = kSub8001A818SyncArg0;
    out.syncArg1 = kSub8001A818SyncArg1;
    out.streamedModeFlag = kSub8001A818StreamedModeFlag;
    out.successSyncResult = kSub8001A818SuccessSyncResult;
    out.readStartSectorSize = kSub80038FC0SectorSize;
    out.readStartTransferUnit = kSub80038FC0TransferUnit;
    out.readSyncTimeoutVblanks = kSub800390C8TimeoutVblanks;
    out.readSyncPumpAfterVblanks = kSub800390C8PumpAfterVblanks;
    out.readSyncFinalFunction = kFn800364F0;
    out.readStartFunctionIsCdHalWrapper = true;
    out.resetInputFunctionIsVblankHal = true;
    out.readSyncFunctionIsCdHalWrapper = true;
    return out;
}

LoaderTempAlloc25B28DirectPlan801C81EC
BuildLoaderTempAlloc25B28DirectPlan801C81EC() {
    LoaderTempAlloc25B28DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn80025B28;
    out.alignmentBytes = kSub80025AllocAlignmentBytes;
    out.positiveRoundAdd = kSub80025AllocPositiveRoundAdd;
    out.negativeRoundAdd = kSub80025AllocNegativeRoundAdd;
    out.topDownFromHeapEnd = true;
    out.updatesLowWater = true;
    out.lowWaterGpOffset = kLoaderAllocLowWaterGpOffset;
    out.heapEndGpOffset = kLoaderAllocHeapEndGpOffset;
    out.heapCursorGpOffset = kLoaderAllocHeapCursorGpOffset;
    return out;
}

LoaderStackAlloc25A70DirectPlan801C81EC
BuildLoaderStackAlloc25A70DirectPlan801C81EC() {
    LoaderStackAlloc25A70DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn80025A70;
    out.stackTableAddress = kDword80091858;
    out.alignmentBytes = kSub80025AllocAlignmentBytes;
    out.positiveRoundAdd = kSub80025AllocPositiveRoundAdd;
    out.negativeRoundAdd = kSub80025AllocNegativeRoundAdd;
    out.maxRequestBytes = kSub80025A70MaxRequestBytes;
    out.maxStackDepth = kSub80025A70MaxStackDepth;
    out.stackDepthGpOffset = kLoaderAllocStackDepthGpOffset;
    out.heapEndGpOffset = kLoaderAllocHeapEndGpOffset;
    out.heapCursorGpOffset = kLoaderAllocHeapCursorGpOffset;
    out.storesOldCursorAtDepthPlusOne = true;
    return out;
}

LoaderStackFree25AF8DirectPlan801C81EC
BuildLoaderStackFree25AF8DirectPlan801C81EC() {
    LoaderStackFree25AF8DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn80025AF8;
    out.stackTableAddress = kDword80091858;
    out.stackDepthGpOffset = kLoaderAllocStackDepthGpOffset;
    out.heapCursorGpOffset = kLoaderAllocHeapCursorGpOffset;
    out.clearsCurrentStackEntry = true;
    out.stackDepthDelta = -1;
    return out;
}

LoaderStackFind25BFCDirectPlan801C81EC
BuildLoaderStackFind25BFCDirectPlan801C81EC() {
    LoaderStackFind25BFCDirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn80025BFC;
    out.stackTableFirstEntryAddress = kDword8009185C;
    out.stackDepthGpOffset = kLoaderAllocStackDepthGpOffset;
    out.firstIndex = kSub80025BFCFirstIndex;
    out.missingReturn = kSub80025BFCMissingReturn;
    return out;
}

LoaderStackSplit25BBCDirectPlan801C81EC
BuildLoaderStackSplit25BBCDirectPlan801C81EC() {
    LoaderStackSplit25BBCDirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn80025BBC;
    out.stackTableAddress = kDword80091858;
    out.stackDepthGpOffset = kLoaderAllocStackDepthGpOffset;
    out.heapEndGpOffset = kLoaderAllocHeapEndGpOffset;
    out.heapCursorGpOffset = kLoaderAllocHeapCursorGpOffset;
    out.storesBlockAtRequestedIndex = true;
    out.updatesDepthToRequestedIndex = true;
    out.successReturn = kSub80025BBCSuccessReturn;
    out.overflowReturn = kSub80025BBCOverflowReturn;
    return out;
}

LoaderTimUpload1AE7CDirectPlan801C81EC
BuildLoaderTimUpload1AE7CDirectPlan801C81EC() {
    LoaderTimUpload1AE7CDirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn8001AE7C;
    out.timInfoFunction = kFn80040EAC;
    out.uploadRectFunction = kFn80044D64;
    out.drawSyncFunction = kFn80044B3C;
    out.lowerUploadCommandFunction = kFn800468E0;
    out.lowerDrawSyncFunction = kFn80046FFC;
    out.timInfoPayloadOffset = kSub8001AE7CTimInfoPayloadOffset;
    out.clutFlagShift = kSub8001AE7CClutFlagShift;
    out.clutFlagMask = kSub8001AE7CClutFlagMask;
    out.drawSyncArg = kSub8001AE7CDrawSyncArg;
    out.uploadsPixelRectFirst = true;
    out.drawsSyncAfterPixelUpload = true;
    out.uploadsClutWhenFlagSet = true;
    out.drawsSyncAfterClutUpload = true;
    out.uploadFunctionIsGpuHalBoundary = true;
    out.drawSyncFunctionIsGpuHalBoundary = true;
    return out;
}

LoaderSpuVabDirectPlan801C81EC BuildLoaderSpuVabDirectPlan801C81EC() {
    LoaderSpuVabDirectPlan801C81EC out{};
    out.valid = true;
    out.closeFunction = kFn80027120;
    out.openFunction = kFn80027078;
    out.transferFunction = kFn800270D4;
    out.enableFunction = kFn800270FC;
    out.lowLevelCloseFunction = kFn8002DF80;
    out.lowLevelOpenFunction = kFn8002E3D8;
    out.lowLevelTransferFunction = kFn8002EB80;
    out.lowLevelEnableFunction = kFn8002EEFC;
    out.currentVabIdAddress = kWord800943A8;
    out.closedVabSentinel = kSub80027120ClosedVabSentinel;
    out.openArg1 = kSub80027078OpenArg1;
    out.enableArg = kSub800270FCEnableArg;
    out.closeOnlyWhenCurrentVabOpen = true;
    out.transferOnlyWhenOpenSucceeds = true;
    out.closeWritesClosedSentinel = true;
    out.openStoresReturnedVabId = true;
    out.openSuccessIsNonNegativeVabId = true;
    out.enableArgIsBoolean = true;
    return out;
}

LoaderRetrySfxDirectPlan801C81EC BuildLoaderRetrySfxDirectPlan801C81EC() {
    LoaderRetrySfxDirectPlan801C81EC out{};
    out.valid = true;
    out.resetFunction = kFn80026FA4;
    out.cueFunction = kFn80026EF8;
    out.flushFunction = kFn80026ECC;
    out.lowLevelResetFunction = kFn800351B8;
    out.lowLevelCueFunction = kFn80034240;
    out.lowLevelFlushFunction = kFn8002EFF4;
    out.currentVabIdAddress = kWord800943A8;
    out.currentVoiceAddress = kWord800943AC;
    out.flushBusyFlagAddress = kDword800943B4;
    out.cueTableAddress = kDword80094410;
    out.cuePitchAdd = kSub80026EF8CuePitchAdd;
    out.cuePitchByteOffset = kSub80026EF8CuePitchByteOffset;
    out.cueVolumeByteOffset = kSub80026EF8CueVolumeByteOffset;
    out.resetArg = kSub80026FA4ResetArg;
    out.writesCurrentVoice = true;
    out.flushOnlyWhenBusyFlagClear = true;
    out.resetIgnoresCallerArg = true;
    out.cueMutatesPitchByte = true;
    out.cueUsesCurrentVabId = true;
    out.lowerFunctionsAreSfxHalBoundaries = true;
    return out;
}

LoaderPadStartComDirectPlan801C81EC
BuildLoaderPadStartComDirectPlan801C81EC() {
    LoaderPadStartComDirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn80026E4C;
    out.callsSsInitHot = true;
    out.lowerInit4096Function = kFn8002DA78;
    out.lowerInitB130Function = kFn8002B130;
    out.lowerInitA6ACFunction = kFn8002A6AC;
    out.lowerInitAA90Function = kFn8002AA90;
    out.lowerInitAB24Function = kFn8002AB24;
    out.lowerInit4096Arg = kPadStartComInit4096Arg;
    out.lowerInitA6ACArg0 = kPadStartComInit90Arg;
    out.lowerInitA6ACArg1 = kPadStartComInit90Arg;
    out.currentVabIdAddress = kWord800943A8;
    out.currentVabSlotAddress = kWord800943AA;
    out.currentVoiceAddress = kWord800943AC;
    out.flushBusyFlagAddress = kDword800943B4;
    out.currentVabIdInit = kPadStartComCurrentVabIdInit;
    out.currentVabSlotInit = kPadStartComClosedSentinel;
    out.currentVoiceInit = kPadStartComClosedSentinel;
    out.flushBusyFlagInit = kPadStartComFlushBusyInit;
    out.returnValue = kPadStartComReturnValue;
    out.lowerCallsAreInitHalBoundaries = true;
    return out;
}

LoaderParse1A8F0DirectPlan801C81EC
BuildLoaderParse1A8F0DirectPlan801C81EC() {
    LoaderParse1A8F0DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn8001A8F0;
    out.seekFunction = kFn8001A89C;
    out.readFunction = kFn8001A818;
    out.tempAllocFunction = kFn80025B28;
    out.payloadAllocFunction = kFn80025A70;
    out.freeFunction = kFn80025AF8;
    out.tempAllocBytes = kSub8001A8F0TempAllocBytes;
    out.headerReadBytes = kSub8001A8F0HeaderReadBytes;
    out.initialSeekOffset = kSub8001A8F0InitialSeekOffset;
    out.headerSeekOffset = kSub8001A8F0HeaderSeekOffset;
    out.recordTypeEnd = kSub8001A8F0RecordTypeEnd;
    out.recordTypeRegisterBlocks = kSub8001A8F0RecordTypeRegisterBlocks;
    out.recordTypePadSpu = kSub8001A8F0RecordTypePadSpu;
    out.recordTypeSplitBlocks = kSub8001A8F0RecordTypeSplitBlocks;
    out.unknownTypeErrorGpOffset = kSub8001A8F0UnknownTypeErrorGpOffset;
    out.recordCountWord = kSub8001A8F0RecordCountWord;
    out.payloadSectorCountWord = kSub8001A8F0PayloadSectorCountWord;
    out.recordPayloadFirstWord = kSub8001A8F0RecordPayloadFirstWord;
    out.recordStrideWords = kSub8001A8F0RecordStrideWords;
    out.type2PayloadBytesWord = kSub8001A8F0Type2PayloadBytesWord;
    out.type2SpuBytesWord = kSub8001A8F0Type2SpuBytesWord;
    out.type1AllocSectorCountWord = kSub8001A8F0PayloadSectorCountWord;
    out.type1SeekSectorCountWord = kSub8001A8F0PayloadSectorCountWord;
    out.type3AllocSectorCountWord = kSub8001A8F0PayloadSectorCountWord;
    out.type3SeekSectorCountWord = kSub8001A8F0PayloadSectorCountWord;
    out.sectorToBytesShift = kSub8001A8F0SectorToBytesShift;
    out.type2SuccessFreeCount = kSub8001A8F0Type2SuccessFreeCount;
    out.type2FailureFreeCount = kSub8001A8F0Type2FailureFreeCount;
    out.type1FreeCount = kSub8001A8F0Type1FreeCount;
    out.successReturn = kSub8001A8F0SuccessReturn;
    out.failureReturn = kSub8001A8F0FailureReturn;
    out.type2PadStartComGap = true;
    out.type2RetrySfxOnlyWhenMode1 = true;
    out.type3RequiresFoundStackBase = true;
    out.type3SplitIndexStartsAtFoundBase = true;
    out.type1RegisterHelper = kFn8001AE7C;
    out.type3FindHeaderHelper = kFn80025BFC;
    out.type3SplitBlockHelper = kFn80025BBC;
    out.type2ComInitFunction = kFn80027120;
    out.type2PadStartComFunction = kFn80026E4C;
    out.type2SpuProbeFunction = kFn80027078;
    out.type2SpuLoadFunction = kFn800270D4;
    out.type2SpuEnableFunction = kFn800270FC;
    out.retryAudioResetFunction = kFn80026FA4;
    out.retrySfxFunction = kFn80026EF8;
    out.retryAudioFlushFunction = kFn80026ECC;
    out.retrySfxCueTableAddress = kDword80094410;
    out.seekPlan1A89C = BuildLoaderSeek1A89CDirectPlan801C81EC();
    out.readPlan1A818 = BuildLoaderRead1A818DirectPlan801C81EC();
    out.tempAllocPlan25B28 = BuildLoaderTempAlloc25B28DirectPlan801C81EC();
    out.stackAllocPlan25A70 = BuildLoaderStackAlloc25A70DirectPlan801C81EC();
    out.stackFreePlan25AF8 = BuildLoaderStackFree25AF8DirectPlan801C81EC();
    out.stackFindPlan25BFC = BuildLoaderStackFind25BFCDirectPlan801C81EC();
    out.stackSplitPlan25BBC = BuildLoaderStackSplit25BBCDirectPlan801C81EC();
    out.timUploadPlan1AE7C = BuildLoaderTimUpload1AE7CDirectPlan801C81EC();
    out.spuVabPlan = BuildLoaderSpuVabDirectPlan801C81EC();
    out.retrySfxPlan = BuildLoaderRetrySfxDirectPlan801C81EC();
    out.padStartComPlan = BuildLoaderPadStartComDirectPlan801C81EC();
    return out;
}

LoaderDraw1EA74DirectPlan801C81EC BuildLoaderDraw1EA74DirectPlan801C81EC() {
    LoaderDraw1EA74DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn8001EA74;
    out.displayIndexFunction = kFn8004019C;
    out.displayTableAddress = kDword8006ED50;
    out.setDrawEnvFunction = kFn80040F90;
    out.drawBaseFunction = kFn80040CC8;
    out.defaultPatternFunction = kFn8001EF40;
    out.mode1Function = kFn8001F524;
    out.mode2Function = kFn8001FCBC;
    out.mode3SetupFunction = kFn80022CBC;
    out.mode3Or4TextFunction = kFn8001D74C;
    out.mode3Or4DrawFunction = kFn8001FC40;
    out.mode4SetupFunction = kFn80021E60;
    out.mode2To4FlushFunction = kFn8001FDC0;
    out.mode5Function = kFn80020308;
    out.mode6Function = kFn80020248;
    out.subtitleFlagAddress = kWord800916DC;
    out.patternModeAddress = kWord800916E0;
    out.currentDisplayIndexGpOffset =
        kSub8001EA74CurrentDisplayIndexGpOffset;
    out.drawActiveFlagGpOffset = kSub8001EA74DrawActiveFlagGpOffset;
    return out;
}

LoaderPattern1EF40DirectPlan801C81EC
BuildLoaderPattern1EF40DirectPlan801C81EC() {
    LoaderPattern1EF40DirectPlan801C81EC out{};
    out.valid = true;
    out.function = kFn8001EF40;
    out.gridColumns = kSub8001EF40GridColumns;
    out.gridRows = kSub8001EF40GridRows;
    out.tileWidth = kSub8001EF40TileWidth;
    out.tileHeight = kSub8001EF40TileHeight;
    out.advanceEveryFrames = kSub8001EF40AdvanceEveryFrames;
    out.bitsPerPatternWord = kSub8001EF40BitsPerPatternWord;
    out.liveGridAddress = kPatternLiveGrid80087330;
    out.liveGridLastColumnAddress = kPatternLiveGridLastColumn8008736C;
    out.liveGridEndAddress = kPatternLiveGridEnd8008766C;
    out.tableMode0Address = kPatternTableMode0_80050730;
    out.tableMode1Address = kPatternTableMode1_80050790;
    out.tableMode2Address = kPatternTableMode2_800507F0;
    out.tableMode3Address = kPatternTableMode3_80050850;
    out.frameLimitTableAddress = kPatternFrameLimitTable800508B4;
    out.highlightTextureTableAddress = kPatternHighlightTextureTable800508B0;
    out.tileIndexTableAddress = kPatternTileIndexTable80050420;
    out.tileTextureTableAddress = kPatternTileTextureTable80050720;
    out.drawHighlightFunction = kFn8001C4EC;
    out.drawTileFunction = kFn8001C550;
    out.bitCursorGpOffset = kSub8001EF40BitCursorGpOffset;
    out.wordCursorGpOffset = kSub8001EF40WordCursorGpOffset;
    out.frameCounterGpOffset = kSub8001EF40FrameCounterGpOffset;
    return out;
}

bool IsTransitionAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    return action.kind == DirectActionKind::Transition201AC ||
           action.kind == DirectActionKind::Transition20110;
}

bool HasDirectTransitionPayload801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    if (action.transitionWorkPtr != kTransitionWork801C3640 ||
        action.transitionA1_801C3640 != kTransitionWork801C3640) {
        return false;
    }

    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    if (action.kind == DirectActionKind::Transition201AC) {
        return action.psxFunctionKnown &&
               action.psxFunction == kFn800201AC &&
               action.transitionFinishFunction == kFn80020090 &&
               !action.transitionHasExtraDelay27194 &&
               action.transitionExtraDelayFrames27194 == 0;
    }
    if (action.kind == DirectActionKind::Transition20110) {
        return action.psxFunctionKnown &&
               action.psxFunction == kFn80020110 &&
               action.transitionFinishFunction == kFn80020008 &&
               action.transitionHasExtraDelay27194 &&
               action.transitionExtraDelayFrames27194 ==
                   kSub80020110Delay27194Frames;
    }
    return false;
}

bool IsExactPsxActionFunction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action,
    uint32_t expectedFunction) {
    return action.psxFunctionKnown && action.psxFunction == expectedFunction;
}

} // namespace

static bool RunBootstrap15590LoaderCdLookupLiveProducerInternal801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::CdLookupLiveInput& input,
    bool success,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out,
    const PrStage1XaCdDirectState* explicitXaCdState,
    const PrStage1XaCdDirectStreamClockProbe800493F4*
        explicitStreamClockProbe,
    const PrMovieSegmentDirect::StreamClockProducerCarrier800493F4*
        explicitStreamClockCarrier);

LoaderOwnerState801C81EC::LoaderOwnerState801C81EC() {
    PrStage1LoaderDirect::Reset(runnerState);
    PrStage1LoaderProducerAdapter::ResetProducerRuntime(producerRuntime);
}

State801C81EC::State801C81EC()
    : loaderOwner(std::make_unique<LoaderOwnerState801C81EC>()) {}

State801C81EC::~State801C81EC() = default;

State801C81EC::State801C81EC(State801C81EC&& other) noexcept = default;

State801C81EC& State801C81EC::operator=(
    State801C81EC&& other) noexcept = default;

void Reset801C81EC(State801C81EC& state) {
    state = State801C81EC{};
}

void ResetLoaderOwnerState801C81EC(State801C81EC& state) {
    ResetBootstrap15590LoaderDirectRuntime801C81EC(state);
}

PrStage1LoaderDirect::RunnerState& GetLoaderRunnerState801C81EC(
    State801C81EC& state) {
    return EnsureLoaderOwnerState801C81EC(state).runnerState;
}

const PrStage1LoaderDirect::RunnerState& GetLoaderRunnerState801C81EC(
    const State801C81EC& state) {
    return EnsureLoaderOwnerState801C81EC(state).runnerState;
}

PrStage1LoaderProducerAdapter::ProducerRuntime&
GetLoaderProducerRuntime801C81EC(State801C81EC& state) {
    return EnsureLoaderOwnerState801C81EC(state).producerRuntime;
}

const PrStage1LoaderProducerAdapter::ProducerRuntime&
GetLoaderProducerRuntime801C81EC(const State801C81EC& state) {
    return EnsureLoaderOwnerState801C81EC(state).producerRuntime;
}

static bool HasHostBlock801C81EC(const State801C81EC& state) {
    const bool clearTailBlocking =
        state.clearTailMovieBlockActive &&
        (!state.clearTailMoviePlayAndWaitPending ||
         state.pendingActions.empty());
    return state.movie1BlockActive ||
           state.abortPollBlockActive ||
           clearTailBlocking ||
           state.bootstrap15590Active ||
           state.saveUi19148StartPending ||
           state.saveUi19148Active;
}

static bool HasBlockingLifecycleWork801C81EC(const State801C81EC& state) {
    return HasHostBlock801C81EC(state) || !state.pendingActions.empty();
}

static bool HasSaveUi19148HostBlock801C81EC(const State801C81EC& state) {
    return state.saveUi19148StartPending || state.saveUi19148Active;
}

HostBlockSnapshot801C81EC GetHostBlockSnapshot801C81EC(
    const State801C81EC& state) {
    HostBlockSnapshot801C81EC out{};
    if (state.movie1BlockActive) {
        out.kind = HostBlockKind801C81EC::Movie1;
        out.active = true;
        out.pathResolved = state.movie1PathResolved;
        out.path = state.movie1Path;
        return out;
    }
    if (state.abortPollBlockActive) {
        out.kind = HostBlockKind801C81EC::AbortPoll;
        out.active = true;
        return out;
    }
    if (state.clearTailMovieBlockActive) {
        out.kind = HostBlockKind801C81EC::ClearTailMovie;
        out.active = true;
        out.waitingForPendingActions =
            state.clearTailMoviePlayAndWaitPending &&
            !state.pendingActions.empty();
        out.pathResolved = state.clearTailMoviePathResolved;
        out.path = state.clearTailMoviePath;
        return out;
    }
    if (state.bootstrap15590Active) {
        out.kind = HostBlockKind801C81EC::Bootstrap15590;
        out.active = true;
        out.pathResolved = state.bootstrap15590PathResolved;
        out.curtainStarted = state.bootstrap15590CurtainStarted;
        out.sceneLoaderSlotKnown = state.bootstrap15590SceneLoaderSlotKnown;
        out.sceneLoaderSlotPresent = state.bootstrap15590SceneLoaderSlotPresent;
        out.sceneLoaderSlot = state.bootstrap15590SceneLoaderSlot;
        out.bootstrapZcompoLoader = state.bootstrap15590ZcompoLoader;
        out.bootstrap15590DirectPlanKnown =
            state.bootstrap15590DirectPlanKnown;
        out.bootstrap15590DirectPlan = state.bootstrap15590DirectPlan;
        FillBootstrap15590LoaderSnapshot801C81EC(state, out);
        out.path = state.bootstrap15590Path;
        return out;
    }
    if (state.saveUi19148StartPending || state.saveUi19148Active) {
        out.kind = HostBlockKind801C81EC::SaveUi19148;
        out.active = state.saveUi19148Active;
        out.startPending = state.saveUi19148StartPending;
        out.saveUi19148LowerFeedbackRequests =
            state.saveUi19148LowerFeedbackRequests;
        out.saveUi19148Seed80092F10Known =
            state.saveUi19148Seed80092F10Known;
        out.saveUi19148Seed80092F10Address =
            state.saveUi19148Seed80092F10Address;
    }
    return out;
}

PendingActionDrainPlan801C81EC BuildPendingActionDrainPlan801C81EC(
    const HostBlockSnapshot801C81EC& block) {
    PendingActionDrainPlan801C81EC out{};
    const bool clearTailPendingActionsAllowed =
        block.kind == HostBlockKind801C81EC::ClearTailMovie &&
        block.waitingForPendingActions;
    if (!clearTailPendingActionsAllowed &&
        (block.active || block.startPending)) {
        return out;
    }
    out.shouldPopAction = true;
    return out;
}

HostBlockKind801C81EC GetActionHostBlockKind801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    using DirectStrBlockKind = PrStage1LifecycleDirect::StrBlockKind801C81EC;
    if (action.kind == DirectActionKind::StrPlayAndWait &&
        action.strBlockKind == DirectStrBlockKind::InitialMovie1) {
        return HostBlockKind801C81EC::Movie1;
    }
    if (action.kind == DirectActionKind::StrInit &&
        action.strBlockKind == DirectStrBlockKind::ClearTailMovie) {
        return HostBlockKind801C81EC::ClearTailMovie;
    }
    if (action.kind == DirectActionKind::StrPlayAndWait &&
        action.strBlockKind == DirectStrBlockKind::ClearTailMovie) {
        return HostBlockKind801C81EC::ClearTailMovie;
    }
    if (action.kind == DirectActionKind::QueryAbort26B94) {
        return HostBlockKind801C81EC::AbortPoll;
    }
    if (action.kind == DirectActionKind::Bootstrap15590) {
        return HostBlockKind801C81EC::Bootstrap15590;
    }
    if (action.kind == DirectActionKind::SaveUi19148) {
        return HostBlockKind801C81EC::SaveUi19148;
    }
    return HostBlockKind801C81EC::None;
}

HostBlockStart801C81EC BuildAbortPollHostBlockStart801C81EC() {
    HostBlockStart801C81EC start{};
    start.kind = HostBlockKind801C81EC::AbortPoll;
    start.active = true;
    return start;
}

HostBlockFeedback801C81EC BuildAbortPollCompletedHostBlockFeedback801C81EC(
    int32_t result) {
    HostBlockFeedback801C81EC feedback{};
    feedback.kind = HostBlockKind801C81EC::AbortPoll;
    feedback.completed = true;
    feedback.result = result;
    return feedback;
}

AbortPollHostTickPlan801C81EC BuildAbortPollHostTickPlan801C81EC(
    const AbortPollHostTickInput801C81EC& input) {
    AbortPollHostTickPlan801C81EC plan{};
    if (input.block.kind != HostBlockKind801C81EC::AbortPoll ||
        !input.block.active) {
        return plan;
    }

    plan.active = true;
    if (input.dispatcherRunning) {
        plan.waiting = true;
        return plan;
    }

    if (input.dispatcherResult >= 0) {
        plan.shouldComplete = true;
        plan.result = input.dispatcherResult;
        return plan;
    }

    if (!input.dispatcherStartAttempted) {
        plan.shouldStartDispatcher = true;
        return plan;
    }

    if (!input.dispatcherStartSucceeded) {
        plan.waiting = true;
        return plan;
    }

    plan.waiting = true;
    return plan;
}

HostBlockStart801C81EC BuildClearTailMovieHostBlockStart801C81EC(
    bool pathResolved,
    const std::filesystem::path& path) {
    HostBlockStart801C81EC start{};
    start.kind = HostBlockKind801C81EC::ClearTailMovie;
    start.active = true;
    start.pathResolved = pathResolved;
    start.path = path;
    return start;
}

HostBlockFeedback801C81EC
BuildClearTailMovieCompletedHostBlockFeedback801C81EC(int32_t result) {
    HostBlockFeedback801C81EC feedback{};
    feedback.kind = HostBlockKind801C81EC::ClearTailMovie;
    feedback.completed = true;
    feedback.result = result;
    return feedback;
}

ClearTailMovieHostTickPlan801C81EC BuildClearTailMovieHostTickPlan801C81EC(
    const ClearTailMovieHostTickInput801C81EC& input) {
    ClearTailMovieHostTickPlan801C81EC plan{};
    if (input.block.kind != HostBlockKind801C81EC::ClearTailMovie ||
        !input.block.active) {
        return plan;
    }

    plan.active = true;
    plan.waitingForPendingActions = input.block.waitingForPendingActions;
    plan.pathResolved = input.block.pathResolved;
    plan.path = input.block.path;
    if (plan.waitingForPendingActions) {
        return plan;
    }

    if (!input.strPlayerReady ||
        !input.block.pathResolved ||
        !input.movieFileExists) {
        return plan;
    }

    plan.shouldPlay = !input.strPlayerPlaying && !input.visualActive;
    plan.shouldUpdate = true;
    plan.skipAllowed = input.debugSkipRequested;
    return plan;
}

HostBlockStart801C81EC BuildSaveUi19148HostBlockStart801C81EC(
    bool started,
    uint32_t seed80092F10Address) {
    HostBlockStart801C81EC start{};
    start.kind = HostBlockKind801C81EC::SaveUi19148;
    start.active = started;
    start.startDeferred = !started;
    start.saveUi19148Seed80092F10Known = true;
    start.saveUi19148Seed80092F10Address = seed80092F10Address;
    return start;
}

HostBlockFeedback801C81EC BuildSaveUi19148CompletedHostBlockFeedback801C81EC(
    int32_t saveResult) {
    HostBlockFeedback801C81EC feedback{};
    feedback.kind = HostBlockKind801C81EC::SaveUi19148;
    feedback.completed = true;
    feedback.result = saveResult;
    return feedback;
}

SaveUi19148HostStartPlan801C81EC BuildSaveUi19148HostStartPlan801C81EC(
    const SaveUi19148HostStartInput801C81EC& input) {
    SaveUi19148HostStartPlan801C81EC plan{};
    if (input.block.kind == HostBlockKind801C81EC::SaveUi19148 &&
        input.block.active) {
        plan.alreadyActive = true;
        return plan;
    }
    if (input.block.kind == HostBlockKind801C81EC::SaveUi19148 &&
        input.block.startPending) {
        if (!input.block.saveUi19148Seed80092F10Known ||
            input.block.saveUi19148Seed80092F10Address !=
                kAddrSavePayloadBank80092F10) {
            return plan;
        }
        plan.actionValid = true;
        plan.seed80092F10Known = true;
        plan.seed80092F10Address =
            input.block.saveUi19148Seed80092F10Address;
        if (!input.startAttempted) {
            plan.shouldStart = true;
            return plan;
        }
        plan.shouldBeginHostBlock = true;
        plan.started = input.startSucceeded;
        return plan;
    }
    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    plan.actionValid =
        input.action.kind == DirectActionKind::SaveUi19148 &&
        IsExactPsxActionFunction801C81EC(input.action, kFn80019148) &&
        input.action.arg0 == static_cast<int32_t>(kAddrSavePayloadBank80092F10) &&
        input.action.rawArg0 == kAddrSavePayloadBank80092F10;
    if (!plan.actionValid) {
        return plan;
    }
    plan.seed80092F10Known = true;
    plan.seed80092F10Address = input.action.rawArg0;
    if (!input.startAttempted) {
        plan.shouldStart = true;
        return plan;
    }
    plan.shouldBeginHostBlock = true;
    plan.started = input.startSucceeded;
    return plan;
}

SaveUi19148HostTickPlan801C81EC BuildSaveUi19148HostTickPlan801C81EC(
    const SaveUi19148HostTickInput801C81EC& input) {
    SaveUi19148HostTickPlan801C81EC plan{};
    if (input.block.kind == HostBlockKind801C81EC::SaveUi19148 &&
        input.block.startPending) {
        plan.active = true;
        plan.shouldStart = true;
        return plan;
    }
    if (input.block.kind != HostBlockKind801C81EC::SaveUi19148 ||
        !input.block.active) {
        return plan;
    }

    plan.active = true;
    if (!input.tickAttempted) {
        plan.shouldTick = true;
        return plan;
    }
    if (input.done) {
        plan.shouldComplete = true;
        plan.saveResult = input.saveResult;
    }
    return plan;
}

TransitionStartPlan801C81EC BuildTransitionStartPlan801C81EC(
    const TransitionStartInput801C81EC& input) {
    TransitionStartPlan801C81EC plan{};
    const PrStage1LifecycleDirect::Action801C81EC& action = input.action;
    plan.activeAlready = input.transitionActive;
    plan.psxFunction = action.psxFunction;
    plan.finishFunction = action.transitionFinishFunction;
    plan.modeA2 = action.transitionModeA2;
    plan.preFfd4ArgA3 = action.transitionPreFfd4ArgA3;
    plan.postFfd4ArgA4 = action.transitionPostFfd4ArgA4;
    plan.hasExtraDelay27194 = action.transitionHasExtraDelay27194;
    plan.extraDelayFrames27194 = action.transitionExtraDelayFrames27194;
    if (input.transitionActive) {
        return plan;
    }
    if (!HasDirectTransitionPayload801C81EC(action)) {
        return plan;
    }

    plan.validPayload = true;
    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    if (action.kind == DirectActionKind::Transition201AC &&
        action.transitionFinishFunction == kFn80020090 &&
        action.transitionModeA2 == 6 &&
        action.transitionPreFfd4ArgA3 == 2 &&
        action.transitionPostFfd4ArgA4 == 1) {
        plan.shouldStart = true;
        plan.preset = TransitionStartPreset801C81EC::ClearTailResultMovieMode6;
        return plan;
    }
    if (action.kind == DirectActionKind::Transition201AC &&
        action.transitionFinishFunction == kFn80020090 &&
        action.transitionModeA2 == 5 &&
        action.transitionPreFfd4ArgA3 == 1 &&
        action.transitionPostFfd4ArgA4 == 2) {
        plan.shouldStart = true;
        plan.preset = TransitionStartPreset801C81EC::ClearTailResultMovieMode5;
        return plan;
    }
    if (action.kind == DirectActionKind::Transition20110 &&
        action.transitionFinishFunction == kFn80020008 &&
        action.transitionHasExtraDelay27194 &&
        action.transitionExtraDelayFrames27194 ==
            kSub80020110Delay27194Frames &&
        action.transitionModeA2 == 1 &&
        action.transitionPreFfd4ArgA3 == 2 &&
        action.transitionPostFfd4ArgA4 == 1) {
        plan.shouldStart = true;
        plan.preset = TransitionStartPreset801C81EC::StageLoopEntry20110Mode1;
        return plan;
    }
    if (action.kind == DirectActionKind::Transition20110 &&
        action.transitionFinishFunction == kFn80020008 &&
        action.transitionHasExtraDelay27194 &&
        action.transitionExtraDelayFrames27194 ==
            kSub80020110Delay27194Frames &&
        action.transitionModeA2 == 2 &&
        action.transitionPreFfd4ArgA3 == 1 &&
        action.transitionPostFfd4ArgA4 == 2) {
        plan.shouldStart = true;
        plan.preset = TransitionStartPreset801C81EC::ClearTailPrelude20110Mode2;
    }
    return plan;
}

HostBlockStart801C81EC BuildMovie1HostBlockStart801C81EC(
    bool pathResolved,
    const std::filesystem::path& path) {
    HostBlockStart801C81EC start{};
    start.kind = HostBlockKind801C81EC::Movie1;
    start.active = true;
    start.pathResolved = pathResolved;
    start.path = path;
    return start;
}

HostBlockFeedback801C81EC BuildMovie1CompletedHostBlockFeedback801C81EC(
    int32_t result) {
    HostBlockFeedback801C81EC feedback{};
    feedback.kind = HostBlockKind801C81EC::Movie1;
    feedback.completed = true;
    feedback.result = result;
    return feedback;
}

Bootstrap15590DirectPlan801C81EC BuildBootstrap15590DirectPlan801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    Bootstrap15590DirectPlan801C81EC plan{};
    plan.wrapperFunction = kFn80015590;
    plan.sceneId = action.sceneId;
    plan.rawSceneArg = action.rawArg0;
    plan.startCurtainFunction = kFn80015408;
    plan.curtainMode15408 = kSub80015408BootstrapCurtainMode;
    plan.curtainModeAddress15408 = kWord800916E0;
    plan.curtainCallbackFlagAddress = kDword8006ECD4;
    plan.curtainStartFlagValue15408 = kSub80015408CallbackFlagValue;
    plan.callbackFunction = kFn8001537C;
    plan.callbackDrawFunction1537C = kFn8001EA74;
    plan.callbackDrawArg0_1537C = kSub8001537CDrawArg0;
    plan.callbackDrawArg1_1537C = kSub8001537CDrawArg1;
    plan.callbackFlushFunction1537C = kFn80026ECC;
    plan.callbackFlagValue1537C = kSub8001537CCallbackFlagValue;
    plan.callbackReturnValue1537C = kSub8001537CCallbackReturnValue;
    plan.loaderFunction = kFn8001AC18;
    plan.loaderOffsetFromSceneEntry = kBootstrapZcompoLoaderOffset;
    plan.loaderMode1AC18 = kSub8001AC18BootstrapLoaderMode;
    plan.loaderMaxAttempts1AC18 = kSub8001AC18MaxAttempts;
    plan.loaderFirstAttemptParserFlag1AC18 =
        kSub8001AC18FirstAttemptParserFlag;
    plan.loaderRetryParserFlag1AC18 = kSub8001AC18RetryParserFlag;
    plan.loaderCopiedDescriptorFirstWord1AC18 =
        kSub8001AC18CopiedDescriptorFirstWord;
    plan.loaderCopiedDescriptorWordCount1AC18 =
        kSub8001AC18CopiedDescriptorWordCount;
    plan.resetDiscFunction = kFn80025A34;
    plan.openFileFunction = kFn8001A324;
    plan.parseIntFunction = kFn8001A8F0;
    plan.stopCurtainFunction = kFn8001545C;
    plan.sceneLoaderSlotKnown = action.sceneLoaderSlotKnown;
    plan.sceneLoaderSlotPresent = action.sceneLoaderSlotPresent;
    plan.sceneLoaderSlot = action.sceneLoaderSlot;
    plan.sceneLoaderRecordKnown = action.sceneLoaderRecordKnown;
    plan.sceneLoaderRowIndex = action.sceneLoaderRowIndex;
    plan.sceneLoaderPsxAddrKnown = action.sceneLoaderPsxAddrKnown;
    plan.sceneLoaderPsxAddr = action.sceneLoaderPsxAddr;
    plan.sceneLoaderPathPtrKnown =
        action.sceneLoaderMovieSegmentRecord.pathPtrA1Plus00Known;
    plan.sceneLoaderPathPtr =
        action.sceneLoaderMovieSegmentRecord.pathPtrA1Plus00;
    plan.bootstrapZcompoLoader = action.bootstrapZcompoLoader;
    plan.resetPlan25A34 = BuildLoaderReset25A34DirectPlan801C81EC();
    plan.openFilePlan1A324 = BuildLoaderOpenFile1A324DirectPlan801C81EC();
    plan.parsePlan1A8F0 = BuildLoaderParse1A8F0DirectPlan801C81EC();
    plan.callbackDrawPlan1EA74 = BuildLoaderDraw1EA74DirectPlan801C81EC();
    plan.patternDrawPlan1EF40 =
        BuildLoaderPattern1EF40DirectPlan801C81EC();

    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    plan.valid =
        action.kind == DirectActionKind::Bootstrap15590 &&
        action.psxFunctionKnown &&
        action.psxFunction == kFn80015590 &&
        action.rawArg0 == action.sceneId &&
        action.bootstrapZcompoLoader &&
        action.loaderOffsetFromSceneEntry == kBootstrapZcompoLoaderOffset;
    return plan;
}

HostBlockStart801C81EC BuildBootstrap15590HostBlockStart801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const Bootstrap15590HostStartFacts801C81EC& facts) {
    HostBlockStart801C81EC start{};
    start.kind = HostBlockKind801C81EC::Bootstrap15590;
    start.active = true;
    start.curtainStarted = facts.curtainStarted;
    start.sceneLoaderSlotKnown = action.sceneLoaderSlotKnown;
    start.sceneLoaderSlotPresent = action.sceneLoaderSlotPresent;
    start.sceneLoaderSlot = action.sceneLoaderSlot;
    start.bootstrapZcompoLoader = action.bootstrapZcompoLoader;
    start.bootstrap15590DirectPlan =
        BuildBootstrap15590DirectPlan801C81EC(action);
    start.bootstrap15590DirectPlanKnown =
        start.bootstrap15590DirectPlan.valid;
    return start;
}

Bootstrap15590HostStartPlan801C81EC BuildBootstrap15590HostStartPlan801C81EC(
    const Bootstrap15590HostStartInput801C81EC& input) {
    Bootstrap15590HostStartPlan801C81EC plan{};
    if (input.block.kind == HostBlockKind801C81EC::Bootstrap15590 &&
        input.block.active) {
        plan.alreadyActive = true;
        return plan;
    }

    const Bootstrap15590DirectPlan801C81EC directPlan =
        BuildBootstrap15590DirectPlan801C81EC(input.action);
    if (!directPlan.valid) {
        return plan;
    }

    if (!input.curtainStartAttempted) {
        plan.shouldStartCurtain = true;
        return plan;
    }
    if (!input.pathResolveAttempted) {
        plan.shouldResolvePath = true;
        return plan;
    }
    if (!input.curtainStarted || !input.pathResolved) {
        return plan;
    }

    Bootstrap15590HostStartFacts801C81EC facts{};
    facts.curtainStarted = input.curtainStarted;
    plan.start = BuildBootstrap15590HostBlockStart801C81EC(
        input.action,
        facts);
    plan.start.pathResolved = input.pathResolved;
    plan.start.path = input.path;
    plan.shouldBeginHostBlock = true;
    return plan;
}

bool IsBootstrap15590DirectAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    return BuildBootstrap15590DirectPlan801C81EC(action).valid;
}

bool TryGetBootstrap15590DirectPlan801C81EC(
    const State801C81EC& state,
    Bootstrap15590DirectPlan801C81EC& out) {
    if (!state.bootstrap15590DirectPlanKnown) {
        return false;
    }
    out = state.bootstrap15590DirectPlan;
    return true;
}

void ClearBootstrap15590DirectPlan801C81EC(State801C81EC& state) {
    state.bootstrap15590DirectPlanKnown = false;
    state.bootstrap15590DirectPlan = Bootstrap15590DirectPlan801C81EC{};
}

Bootstrap15590LoaderBeginResult801C81EC
BeginBootstrap15590LoaderDirect801C81EC(State801C81EC& state) {
    if (!state.bootstrap15590DirectPlanKnown) {
        Bootstrap15590LoaderBeginResult801C81EC out{};
        out.active = state.bootstrap15590Active;
        return out;
    }
    return BeginBootstrap15590LoaderDirect801C81EC(
        state,
        state.bootstrap15590DirectPlan);
}

Bootstrap15590LoaderBeginResult801C81EC
BeginBootstrap15590LoaderDirect801C81EC(
    State801C81EC& state,
    const Bootstrap15590DirectPlan801C81EC& plan) {
    Bootstrap15590LoaderBeginResult801C81EC out{};
    out.active = state.bootstrap15590Active;
    out.planKnown = plan.valid;
    out.attempted = state.bootstrap15590Active && plan.valid;

    LoaderOwnerState801C81EC& owner = EnsureLoaderOwnerState801C81EC(state);
    out.alreadyBegun = state.bootstrap15590LoaderDirectBegun &&
                       !IsBootstrap15590LoaderTerminal801C81EC(
                           owner.runnerState);
    if (out.alreadyBegun) {
        out.begun = true;
        out.beginSucceeded = state.bootstrap15590LoaderDirectBeginSucceeded;
        out.beginFailed = state.bootstrap15590LoaderDirectBeginFailed;
        out.actionCount = owner.runnerState.actions.size();
        out.nextActionIndex = owner.runnerState.nextActionIndex;
        return out;
    }

    if (!out.attempted) {
        return out;
    }

    state.bootstrap15590DirectPlanKnown = true;
    state.bootstrap15590DirectPlan = plan;
    state.bootstrap15590LoaderDirectBeginAttempted = true;
    state.bootstrap15590LoaderDirectPumpStarted = false;
    state.bootstrap15590LoaderDirectWaitingExternal = false;
    state.bootstrap15590LoaderDirectCompleted = false;
    state.bootstrap15590LoaderDirectFailed = false;
    state.bootstrap15590LoaderDirectLastPumpActionCount = 0;
    state.bootstrap15590CdLookupLowerProducerRuntime =
        Bootstrap15590CdLookupLowerProducerRuntime801C81EC{};
    state.bootstrap15590CdLowerProducerRuntime =
        Bootstrap15590CdLowerProducerRuntime801C81EC{};
    state.bootstrap15590CdLowerLivePayloadBytes.clear();
    state.bootstrap15590CdLowerFileBaseLbaKnown = false;
    state.bootstrap15590CdLowerFileBaseLba = 0;
    state.bootstrap15590CdLowerLastSeekLbaKnown = false;
    state.bootstrap15590CdLowerLastSeekLba = 0;
    PrStage1LoaderProducerAdapter::ResetProducerRuntime(
        owner.producerRuntime);

    out.begun = PrStage1LoaderDirect::Begin(owner.runnerState, plan);
    state.bootstrap15590LoaderDirectBegun = out.begun;
    state.bootstrap15590LoaderDirectBeginSucceeded = out.begun;
    state.bootstrap15590LoaderDirectBeginFailed = !out.begun;
    out.beginSucceeded = state.bootstrap15590LoaderDirectBeginSucceeded;
    out.beginFailed = state.bootstrap15590LoaderDirectBeginFailed;
    out.actionCount = owner.runnerState.actions.size();
    out.nextActionIndex = owner.runnerState.nextActionIndex;
    SyncBootstrap15590LoaderDirectFlags801C81EC(state, owner.runnerState);
    return out;
}

Bootstrap15590LoaderPumpResult801C81EC
PumpBootstrap15590LoaderDirect801C81EC(State801C81EC& state,
                                      std::size_t maxActionCount) {
    Bootstrap15590LoaderPumpResult801C81EC out{};
    out.active = state.bootstrap15590Active;
    out.planKnown = state.bootstrap15590DirectPlanKnown &&
                    state.bootstrap15590DirectPlan.valid;
    if (!state.bootstrap15590Active) {
        return out;
    }

    if (!state.bootstrap15590LoaderDirectBegun) {
        const Bootstrap15590LoaderBeginResult801C81EC begin =
            BeginBootstrap15590LoaderDirect801C81EC(state);
        out.beginAttempted = begin.attempted;
        out.runnerBegun = begin.begun;
        out.failed = begin.beginFailed;
        if (!begin.begun) {
            return out;
        }
    }

    LoaderOwnerState801C81EC& owner = EnsureLoaderOwnerState801C81EC(state);
    PrStage1LoaderDirect::RunnerState& runnerState = owner.runnerState;
    PrStage1LoaderProducerAdapter::ProducerRuntime& producerRuntime =
        owner.producerRuntime;
    out.runnerBegun = true;
    state.bootstrap15590LoaderDirectPumpStarted = true;
    state.bootstrap15590LoaderDirectLastPumpActionCount = 0;

    while (out.poppedActionCount < maxActionCount) {
        if (IsBootstrap15590LoaderTerminal801C81EC(runnerState)) {
            break;
        }

        if (PrStage1LoaderDirect::IsWaitingForFeedback(runnerState)) {
            out.waitingForFeedback = true;
            PrStage1LoaderDirect::ProducerStep waitingStep{};
            if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
                    runnerState,
                    waitingStep)) {
                break;
            }
            out.waitingStepKnown = waitingStep.valid;
            const bool memoryStep =
                waitingStep.category ==
                PrStage1LoaderProducerAdapter::ProducerCategory::Memory;
            const bool gpuPayloadStep =
                waitingStep.category ==
                PrStage1LoaderProducerAdapter::ProducerCategory::Gpu;
            const bool localHalStep =
                waitingStep.category ==
                    PrStage1LoaderProducerAdapter::ProducerCategory::PadCom ||
                waitingStep.category ==
                    PrStage1LoaderProducerAdapter::ProducerCategory::Spu;
            out.externalProducerRequired =
                waitingStep.externalProducerRequired && !memoryStep &&
                !gpuPayloadStep && !localHalStep;
            out.helperGap = waitingStep.helperGap;

            if (waitingStep.helperGap) {
                break;
            }

            bool appliedLocalFeedback = false;
            if (memoryStep) {
                PrStage1LoaderProducerAdapter::MemoryRuntimeStepResult
                    memoryResult{};
                out.memoryPumpAttempted = true;
                if (!PrStage1LoaderProducerAdapter::RunMemoryProducerForWaitingStep(
                        runnerState,
                        producerRuntime,
                        true,
                        &memoryResult)) {
                    out.memoryPumpProduced = memoryResult.produced;
                    out.memoryPumpApplied = memoryResult.applied;
                    out.helperGap = out.helperGap || memoryResult.helperGap;
                    break;
                }
                out.memoryPumpProduced = out.memoryPumpProduced ||
                                         memoryResult.produced;
                out.memoryPumpApplied = out.memoryPumpApplied ||
                                        memoryResult.applied;
                appliedLocalFeedback = memoryResult.applied;
            } else if (gpuPayloadStep) {
                PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
                PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult
                    buildResult{};
                out.gpuLivePumpAttempted = true;
                const bool built =
                    PrStage1LoaderProducerAdapter::
                        BuildGpuTimBytesLiveTypedFeedbackFromResolvedPayload(
                            runnerState,
                            waitingStep,
                            feedback,
                            &buildResult);
                if (!built) {
                    out.gpuLivePumpProduced = buildResult.produced;
                    out.gpuLivePumpIncomplete =
                        buildResult.incompleteFeedback;
                    out.externalProducerRequired = true;
                    break;
                }
                out.gpuLivePumpProduced = true;
                appliedLocalFeedback =
                    PrStage1LoaderProducerAdapter::ApplyTypedFeedback(
                        runnerState,
                        feedback);
                out.gpuLivePumpApplied =
                    out.gpuLivePumpApplied || appliedLocalFeedback;
            } else if (localHalStep) {
                PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
                PrStage1LoaderSpuHal::State candidateSpuState =
                    state.bootstrap15590LoaderSpuHalState;
                out.localHalPumpAttempted = true;
                const bool built = BuildBootstrap15590LocalHalFeedback801C81EC(
                    waitingStep,
                    candidateSpuState,
                    feedback);
                if (!built) {
                    out.externalProducerRequired = true;
                    break;
                }
                out.localHalPumpProduced = true;
                appliedLocalFeedback =
                    PrStage1LoaderProducerAdapter::ApplyTypedFeedback(
                        runnerState,
                        feedback);
                out.localHalPumpApplied =
                    out.localHalPumpApplied || appliedLocalFeedback;
                if (appliedLocalFeedback) {
                    state.bootstrap15590LoaderSpuHalState =
                        candidateSpuState;
                }
            }

            if (!appliedLocalFeedback) {
                break;
            }
            continue;
        }

        PrStage1LoaderDirect::Action action{};
        if (!PrStage1LoaderDirect::PopNextAction(runnerState, action)) {
            break;
        }
        out.poppedAction = true;
        ++out.poppedActionCount;
        state.bootstrap15590LoaderDirectLastPumpActionCount =
            out.poppedActionCount;
    }

    out.reachedStepLimit =
        maxActionCount != 0u && out.poppedActionCount >= maxActionCount &&
        !IsBootstrap15590LoaderTerminal801C81EC(runnerState) &&
        !PrStage1LoaderDirect::IsWaitingForFeedback(runnerState);
    out.waitingForFeedback =
        out.waitingForFeedback ||
        PrStage1LoaderDirect::IsWaitingForFeedback(runnerState);
    out.runnerTerminal = IsBootstrap15590LoaderTerminal801C81EC(runnerState);
    out.completed =
        runnerState.status == PrStage1LoaderDirect::RunnerStatus::Completed;
    out.failed =
        runnerState.status == PrStage1LoaderDirect::RunnerStatus::Failed;
    SyncBootstrap15590LoaderDirectFlags801C81EC(state, runnerState);
    return out;
}

bool DescribeBootstrap15590LoaderWaitingStep801C81EC(
    const State801C81EC& state,
    PrStage1LoaderDirect::ProducerStep& out) {
    out = PrStage1LoaderDirect::ProducerStep{};
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun) {
        return false;
    }
    const LoaderOwnerState801C81EC& owner =
        EnsureLoaderOwnerState801C81EC(state);
    return PrStage1LoaderDirect::DescribeWaitingProducerStep(
        owner.runnerState,
        out);
}

bool BuildBootstrap15590LoaderCallbackHostRequest801C81EC(
    const State801C81EC& state,
    Bootstrap15590CallbackHostRequest801C81EC& out) {
    namespace Loader = PrStage1LoaderDirect;
    out = Bootstrap15590CallbackHostRequest801C81EC{};

    Loader::ProducerStep waitingStep{};
    if (!DescribeBootstrap15590LoaderWaitingStep801C81EC(state, waitingStep) ||
        !waitingStep.valid ||
        waitingStep.category != Loader::ProducerCategory::Callback) {
        return false;
    }

    switch (waitingStep.stepKind) {
    case Loader::ProducerStepKind::Callback1537C:
        out.valid = true;
        out.waitingForCallback = true;
        out.transitionUpdateRequired = true;
        out.loadingCurtainCallbackRequired = true;
        return true;
    case Loader::ProducerStepKind::CallbackFlush26ECC:
    case Loader::ProducerStepKind::CallbackDraw1EA74:
        out.valid = true;
        out.waitingForCallback = true;
        out.loadingCurtainCallbackRequired = true;
        return true;
    default:
        return false;
    }
}

bool BuildBootstrap15590CdLookupLowerProducerRequest801C81EC(
    State801C81EC& state,
    Bootstrap15590CdLookupHostRequest801C81EC& out) {
    namespace Loader = PrStage1LoaderDirect;
    out = Bootstrap15590CdLookupHostRequest801C81EC{};

    Loader::ProducerStep waitingStep{};
    if (!DescribeBootstrap15590LoaderWaitingStep801C81EC(state, waitingStep) ||
        !waitingStep.valid ||
        waitingStep.category != Loader::ProducerCategory::Cd ||
        waitingStep.stepKind != Loader::ProducerStepKind::CdBoundary ||
        waitingStep.action.cd.actionKind !=
            PrStage1LoaderCdHal::ActionKind::Lookup800381F8) {
        return false;
    }

    const PrStage1MovieSegmentDirect::Stage1MovieSegmentIdentity801C4780
        identity =
            PrStage1MovieSegmentDirect::IdentifyStage1MovieSegmentPathPtr801C4780(
                waitingStep.action.cd.pathPtr);
    Bootstrap15590CdLookupLowerProducerRuntime801C81EC& runtime =
        state.bootstrap15590CdLookupLowerProducerRuntime;
    const uint32_t sceneIndex =
        PrStage1MovieSegmentDirect::
            kStage1SceneEntryMovieSegmentSceneIndex801C4780;
    const bool sameKey =
        runtime.keyKnown &&
        runtime.sceneIndex == sceneIndex &&
        runtime.psxOrder == waitingStep.psxOrder &&
        runtime.probeFunction == waitingStep.action.cd.callerFunction &&
        runtime.lookupFunction == waitingStep.action.cd.lowerFunction &&
        runtime.cdlFilePtr == waitingStep.action.cd.cdlFilePtr &&
        runtime.pathPtr == waitingStep.action.cd.pathPtr;
    if (!sameKey) {
        runtime = Bootstrap15590CdLookupLowerProducerRuntime801C81EC{};
        runtime.keyKnown = true;
        runtime.sceneIndex = sceneIndex;
        runtime.psxOrder = waitingStep.psxOrder;
        runtime.probeFunction = waitingStep.action.cd.callerFunction;
        runtime.lookupFunction = waitingStep.action.cd.lowerFunction;
        runtime.cdlFilePtr = waitingStep.action.cd.cdlFilePtr;
        runtime.pathPtr = waitingStep.action.cd.pathPtr;
        runtime.psxPath = identity.psxPath;
        runtime.probe = PrStage1LoaderCdHal::BeginProbe8001A2B0(
            runtime.cdlFilePtr,
            runtime.pathPtr);
    }

    out.valid = true;
    out.waitingForCdLookup = true;
    out.psxOrder = runtime.psxOrder;
    out.probeFunction = runtime.probeFunction;
    out.lookupFunction = runtime.lookupFunction;
    out.cdlFilePtr = runtime.probe.currentCdlFilePtr;
    out.pathPtr = runtime.pathPtr;
    out.psxPathKnown = identity.known && identity.psxPath != nullptr;
    out.psxPath = identity.psxPath;
    if (runtime.probe.complete) {
        out.probeCompleteWithoutHal = true;
        out.binNeeded = false;
        out.retryIndex = runtime.probe.nextRetryIndex;
        return true;
    }

    PrStage1LoaderCdHal::Action lookupAction{};
    if (!PrStage1LoaderCdHal::BuildNextLookupAction8001A2B0(
            runtime.probe,
            lookupAction)) {
        return false;
    }
    out.binNeeded = true;
    out.retryIndex = lookupAction.retryIndex;
    out.cdlFilePtr = lookupAction.cdlFilePtr;
    out.pathPtr = lookupAction.pathPtr;
    return true;
}

static bool ResolveBootstrap15590CdSeekLba801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderDirect::Action& action,
    int32_t& out) {
    if (action.cd.lba != 0) {
        out = action.cd.lba;
        return true;
    }
    int32_t relativeOffset = 0;
    bool relativeOffsetKnown = false;
    if (action.recordDataResolved && action.recordData.startSectorKnown &&
        action.recordData.startSector <= static_cast<uint32_t>(0x7FFFFFFF)) {
        relativeOffset = static_cast<int32_t>(action.recordData.startSector);
        relativeOffsetKnown = true;
    } else if (action.kind == PrStage1LoaderDirect::ActionKind::Seek1A89C &&
               action.recordType ==
                   PrStage1LoaderDirect::LoaderRecordType::Unknown) {
        relativeOffset = 0;
        relativeOffsetKnown = true;
    }
    if (relativeOffsetKnown &&
        state.bootstrap15590CdLowerFileBaseLbaKnown) {
        out = state.bootstrap15590CdLowerFileBaseLba + relativeOffset;
        return true;
    }
    return false;
}

static bool ResolveBootstrap15590CdReadDst801C81EC(
    const PrStage1LoaderDirect::Action& action,
    uint32_t& out) {
    if (action.cd.dstPtr != 0u) {
        out = action.cd.dstPtr;
        return true;
    }
    if (action.payloadResolved && action.resolvedPayload.psxAddressKnown) {
        out = action.resolvedPayload.psxAddress;
        return true;
    }
    return false;
}

static bool ResolveBootstrap15590CdReadSectorCount801C81EC(
    const PrStage1LoaderDirect::Action& action,
    int32_t& out) {
    if (action.cd.sectorCount != 0) {
        out = action.cd.sectorCount;
        return true;
    }
    if (action.payloadResolved && action.resolvedPayload.sectorCountKnown &&
        action.resolvedPayload.sectorCount <=
            static_cast<uint32_t>(0x7FFFFFFF)) {
        out = static_cast<int32_t>(action.resolvedPayload.sectorCount);
        return true;
    }
    if (action.recordDataResolved && action.recordData.sectorCountKnown &&
        action.recordData.sectorCount <= static_cast<uint32_t>(0x7FFFFFFF)) {
        out = static_cast<int32_t>(action.recordData.sectorCount);
        return true;
    }
    return false;
}

static PrStage1LoaderCdHal::LowerActionRequestMetadata
BuildBootstrap15590CdLowerRequestMetadata801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderDirect::Action& action) {
    PrStage1LoaderCdHal::LowerActionRequestMetadata out{};
    out.known = true;
    out.actionKind = action.cd.actionKind;
    out.callerFunction = action.cd.callerFunction;
    out.directHelperFunction = action.cd.directHelperFunction;
    out.lowerFunction = action.cd.lowerFunction;
    out.finalFunction = action.cd.finalFunction;

    switch (action.cd.actionKind) {
    case PrStage1LoaderCdHal::ActionKind::SeekSync800367A4: {
        out.seekRequestKnown = true;
        out.seekMsfTargetPtrKnown = action.cd.msfTargetPtr != 0u;
        out.seekMsfTargetPtr = action.cd.msfTargetPtr;
        int32_t lba = 0;
        if (ResolveBootstrap15590CdSeekLba801C81EC(state, action, lba)) {
            out.seekLbaKnown = true;
            out.seekLba = lba;
            out.seekMsfTargetKnown = true;
            out.seekMsfTarget =
                PrStage1LoaderCdHal::LbaToBcdMsf80036974(lba);
        }
        out.seekArg0 = action.cd.arg0;
        out.seekArg1 = static_cast<int32_t>(action.cd.msfTargetPtr);
        out.seekArg2 = action.cd.arg2;
        break;
    }
    case PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0: {
        out.readStartRequestKnown = true;
        uint32_t dst = 0;
        if (ResolveBootstrap15590CdReadDst801C81EC(action, dst)) {
            out.readStartDstPtrKnown = true;
            out.readStartDstPtr = dst;
            out.readStartArg1 = static_cast<int32_t>(dst);
        }
        int32_t sectorCount = 0;
        if (ResolveBootstrap15590CdReadSectorCount801C81EC(action,
                                                           sectorCount)) {
            out.readStartSectorCountKnown = true;
            out.readStartSectorCount = sectorCount;
            out.readStartArg0 = sectorCount;
        }
        out.readStartModeFlagKnown = true;
        out.readStartModeFlag = action.cd.modeFlag;
        out.readSyncRequestKnown = action.cd.finalFunction != 0u;
        out.readSyncFunction = action.cd.finalFunction;
        out.readSyncArg0 = action.cd.arg1;
        out.readSyncArg1 = action.cd.arg2;
        break;
    }
    case PrStage1LoaderCdHal::ActionKind::ReadSync800390C8:
        out.readSyncRequestKnown = true;
        out.readSyncFunction = action.cd.lowerFunction;
        out.readSyncArg0 = action.cd.arg0;
        out.readSyncArg1 = action.cd.arg1;
        break;
    default:
        break;
    }
    return out;
}

static bool BcdMsfMatches801C81EC(
    const PrStage1LoaderCdHal::BcdMsf80036974& a,
    const PrStage1LoaderCdHal::BcdMsf80036974& b) {
    return a.minute == b.minute && a.second == b.second &&
           a.frame == b.frame;
}

static bool LowerCdRequestMetadataMatches801C81EC(
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& expected,
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& actual) {
    namespace CdHal = PrStage1LoaderCdHal;
    if (!expected.known || !actual.known ||
        expected.actionKind != actual.actionKind ||
        expected.callerFunction != actual.callerFunction ||
        expected.directHelperFunction != actual.directHelperFunction ||
        expected.lowerFunction != actual.lowerFunction ||
        expected.finalFunction != actual.finalFunction) {
        return false;
    }

    switch (expected.actionKind) {
    case CdHal::ActionKind::SeekSync800367A4:
        if (expected.seekRequestKnown != actual.seekRequestKnown ||
            expected.seekArg0 != actual.seekArg0 ||
            expected.seekArg1 != actual.seekArg1 ||
            expected.seekArg2 != actual.seekArg2 ||
            expected.seekMsfTargetPtrKnown !=
                actual.seekMsfTargetPtrKnown ||
            expected.seekLbaKnown != actual.seekLbaKnown ||
            expected.seekMsfTargetKnown != actual.seekMsfTargetKnown) {
            return false;
        }
        if (expected.seekMsfTargetPtrKnown &&
            expected.seekMsfTargetPtr != actual.seekMsfTargetPtr) {
            return false;
        }
        if (expected.seekLbaKnown && expected.seekLba != actual.seekLba) {
            return false;
        }
        if (expected.seekMsfTargetKnown &&
            !BcdMsfMatches801C81EC(expected.seekMsfTarget,
                                   actual.seekMsfTarget)) {
            return false;
        }
        return true;
    case CdHal::ActionKind::ReadStart80038FC0:
        if (expected.readStartRequestKnown !=
                actual.readStartRequestKnown ||
            expected.readStartDstPtrKnown != actual.readStartDstPtrKnown ||
            expected.readStartSectorCountKnown !=
                actual.readStartSectorCountKnown ||
            expected.readStartModeFlagKnown !=
                actual.readStartModeFlagKnown ||
            expected.readStartArg0 != actual.readStartArg0 ||
            expected.readStartArg1 != actual.readStartArg1 ||
            expected.readSyncRequestKnown != actual.readSyncRequestKnown ||
            expected.readSyncFunction != actual.readSyncFunction ||
            expected.readSyncArg0 != actual.readSyncArg0 ||
            expected.readSyncArg1 != actual.readSyncArg1) {
            return false;
        }
        if (expected.readStartDstPtrKnown &&
            expected.readStartDstPtr != actual.readStartDstPtr) {
            return false;
        }
        if (expected.readStartSectorCountKnown &&
            expected.readStartSectorCount !=
                actual.readStartSectorCount) {
            return false;
        }
        if (expected.readStartModeFlagKnown &&
            expected.readStartModeFlag != actual.readStartModeFlag) {
            return false;
        }
        return true;
    case CdHal::ActionKind::ReadSync800390C8:
        return expected.readSyncRequestKnown ==
                   actual.readSyncRequestKnown &&
               expected.readSyncFunction == actual.readSyncFunction &&
               expected.readSyncArg0 == actual.readSyncArg0 &&
               expected.readSyncArg1 == actual.readSyncArg1;
    default:
        return true;
    }
}

static bool LowerCdAttemptProvenanceMatches801C81EC(
    const Bootstrap15590CdLowerHostRequest801C81EC& expected,
    const PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution&
        actual) {
    if (!actual.known || !actual.attemptIndexKnown) {
        return false;
    }
    if (actual.sourceFunction !=
            PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper ||
        actual.transferFunction !=
            PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer) {
        return false;
    }
    if (actual.attemptIndex != expected.attemptIndex) {
        return false;
    }
    if (expected.readDstPtrKnown &&
        (!actual.dstKnown || actual.dst != expected.readDstPtr)) {
        return false;
    }
    if (expected.readSectorCountKnown &&
        (!actual.sectorCountKnown ||
         actual.sectorCount !=
             static_cast<uint32_t>(expected.readSectorCount))) {
        return false;
    }
    return true;
}

static bool IsBootstrap15590CdProducerStep801C81EC(
    const PrStage1LoaderDirect::ProducerStep& step) {
    namespace Loader = PrStage1LoaderDirect;
    return step.category == Loader::ProducerCategory::Cd &&
           (step.stepKind == Loader::ProducerStepKind::CdBoundary ||
            step.stepKind == Loader::ProducerStepKind::CdReadPayload1A818);
}

static void RecordBootstrap15590CdLowerFeedbackRejected801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    Bootstrap15590CdLowerRejectReason801C81EC reason,
    Bootstrap15590CdLowerAttemptStatus801C81EC status,
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& expected,
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& actual) {
    runtime.status = Bootstrap15590CdLowerAttemptStatus801C81EC::
        FeedbackRejected;
    runtime.lastRejectKnown = true;
    runtime.lastRejectReason = reason;
    runtime.lastRejectStatus = status;
    runtime.lastRejectExpected = expected;
    runtime.lastRejectActual = actual;
}

static void MarkBootstrap15590CdLowerFeedbackResolved801C81EC(
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
    runtime.keyKnown = false;
    runtime = Bootstrap15590CdLowerProducerRuntime801C81EC{};
}

static void MarkBootstrap15590CdLowerFeedbackRejected801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LoaderDirect::CdSeamResult& cd,
    Bootstrap15590CdLowerRejectReason801C81EC reason,
    Bootstrap15590CdLowerAttemptStatus801C81EC status,
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& actual) {
    if (!cd.present || !cd.feedback.handled) {
        return;
    }
    runtime.requestPending = false;
    RecordBootstrap15590CdLowerFeedbackRejected801C81EC(
        runtime,
        reason,
        status,
        runtime.lowerRequest,
        actual);
}

bool BuildBootstrap15590CdLowerProducerRequest801C81EC(
    State801C81EC& state,
    Bootstrap15590CdLowerHostRequest801C81EC& out) {
    namespace Loader = PrStage1LoaderDirect;
    out = Bootstrap15590CdLowerHostRequest801C81EC{};

    Loader::ProducerStep waitingStep{};
    if (!DescribeBootstrap15590LoaderWaitingStep801C81EC(state, waitingStep) ||
        !waitingStep.valid ||
        !IsBootstrap15590CdProducerStep801C81EC(waitingStep)) {
        out.status =
            Bootstrap15590CdLowerAttemptStatus801C81EC::NotApplicable;
        return false;
    }

    out.valid = true;
    out.waitingForCd = true;
    out.stepKind = waitingStep.stepKind;
    out.cdActionKind = waitingStep.action.cd.actionKind;
    out.psxOrder = waitingStep.psxOrder;
    out.recordIndex = waitingStep.recordIndex;
    out.recordType = waitingStep.recordType;
    out.attemptIndex = waitingStep.action.attemptIndex;
    out.payloadBytesRequired = waitingStep.requiredFields.resolvedPayload;
    out.lowerRequest =
        BuildBootstrap15590CdLowerRequestMetadata801C81EC(
            state,
            waitingStep.action);

    if (out.cdActionKind ==
        PrStage1LoaderCdHal::ActionKind::Lookup800381F8) {
        out.status =
            Bootstrap15590CdLowerAttemptStatus801C81EC::NotApplicable;
        Bootstrap15590CdLookupHostRequest801C81EC lookup{};
        if (BuildBootstrap15590CdLookupLowerProducerRequest801C81EC(
                state,
                lookup)) {
            out.psxPathKnown = lookup.psxPathKnown;
            out.psxPath = lookup.psxPath;
            const auto identity =
                PrStage1MovieSegmentDirect::IdentifyStage1MovieSegmentPathPtr801C4780(
                    waitingStep.action.cd.pathPtr);
            out.relativeWinPathKnown =
                identity.known && identity.relativeWinPath != nullptr;
            out.relativeWinPath = identity.relativeWinPath;
        }
        return true;
    }

    if (out.lowerRequest.seekLbaKnown) {
        out.seekLbaKnown = true;
        out.seekLba = out.lowerRequest.seekLba;
    }
    if (out.cdActionKind ==
        PrStage1LoaderCdHal::ActionKind::SeekSync800367A4) {
        out.cdSyncLoopFactsRequired80037070 = true;
        out.cdSyncLoopFunction80037070 = 0x80037070u;
        out.cdSyncLoopA0WaitModeKnown80037070 = true;
        out.cdSyncLoopA0WaitMode80037070 = 0;
    }
    if (out.lowerRequest.readStartDstPtrKnown) {
        out.readDstPtrKnown = true;
        out.readDstPtr = out.lowerRequest.readStartDstPtr;
    }
    if (out.lowerRequest.readStartSectorCountKnown) {
        out.readSectorCountKnown = true;
        out.readSectorCount = out.lowerRequest.readStartSectorCount;
    }

    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime =
        state.bootstrap15590CdLowerProducerRuntime;
    if (TryCarryBootstrap15590CdLowerSameKeyPendingRequest801C81EC(
            runtime,
            waitingStep,
            out)) {
        return true;
    }

    MarkBootstrap15590CdLowerRequestPending801C81EC(runtime,
                                                   waitingStep,
                                                   out);
    out.status = Bootstrap15590CdLowerAttemptStatus801C81EC::RequestReady;
    return true;
}

bool RunBootstrap15590CdLookupLowerProducerFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LoaderCdHal::LookupFeedback800381F8& feedback,
    const PrStage1XaCdDirectState* xaCdState) {
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        return false;
    }

    Bootstrap15590CdLookupLowerProducerRuntime801C81EC& runtime =
        state.bootstrap15590CdLookupLowerProducerRuntime;
    if (!runtime.keyKnown) {
        return false;
    }

    PrStage1LoaderCdHal::LookupFeedback800381F8 lookup = feedback;
    if (!lookup.result.handled) {
        lookup.result.kind = PrStage1LoaderCdHal::ActionKind::Lookup800381F8;
        lookup.result.handled = true;
        lookup.result.success = false;
        lookup.result.psxReturn = 0;
        lookup.requestKnown = runtime.cdlFilePtr != 0u;
        lookup.requestCdlFilePtr = runtime.cdlFilePtr;
        lookup.requestPathPtrKnown = runtime.pathPtr != 0u;
        lookup.requestPathPtr = runtime.pathPtr;
    }

    if (!runtime.probe.complete) {
        PrStage1LoaderCdHal::ApplyLookupFeedback8001A2B0(
            runtime.probe,
            lookup);
        runtime.lastLookupFeedback = lookup;
        runtime.lastLookupFeedbackKnown = true;
    } else if (!runtime.lastLookupFeedbackKnown) {
        runtime.lastLookupFeedback = lookup;
        runtime.lastLookupFeedbackKnown = true;
    }

    if (!runtime.probe.complete) {
        return false;
    }

    const PrStage1LoaderCdHal::ProbeCompletionFeedback8001A2B0 completion =
        PrStage1LoaderCdHal::BuildProbeCompletionFeedback8001A2B0(
            runtime.probe,
            runtime.lastLookupFeedback);
    if (!completion.known) {
        return false;
    }

    PrStage1LoaderProducerAdapter::CdLookupLiveInput liveInput{};
    liveInput.feedback = runtime.lastLookupFeedback.result;
    liveInput.lookup800381F8 = runtime.lastLookupFeedback;
    liveInput.lookup800381F8Known = runtime.lastLookupFeedback.result.handled;
    liveInput.probe8001A2B0 = completion;
    liveInput.probe8001A2B0Known = completion.known;

    return RunBootstrap15590LoaderCdLookupLiveProducerInternal801C81EC(
        state,
        liveInput,
        completion.lookupSucceeded,
        true,
        nullptr,
        nullptr,
        xaCdState,
        nullptr,
        nullptr);
}

bool RunBootstrap15590LoaderCallbackHostFeedback801C81EC(
    State801C81EC& state,
    const Bootstrap15590CallbackHostInput801C81EC& input) {
    namespace Adapter = PrStage1LoaderProducerAdapter;
    namespace Loader = PrStage1LoaderDirect;

    Loader::ProducerStep waitingStep{};
    if (!DescribeBootstrap15590LoaderWaitingStep801C81EC(state, waitingStep) ||
        !waitingStep.valid ||
        waitingStep.category != Loader::ProducerCategory::Callback) {
        return false;
    }

    if (!input.loadingCurtainCallbackKnown ||
        !input.loadingCurtainCallbackFired) {
        return false;
    }

    Adapter::TypedActionFeedback feedback{};
    switch (waitingStep.stepKind) {
    case Loader::ProducerStepKind::Callback1537C: {
        if (!input.transitionUpdateAttempted) {
            return false;
        }
        Loader::Callback1537CFeedback callback{};
        callback.callbackInvoked = true;
        callback.flagValueKnown = true;
        callback.flagValue = waitingStep.action.callback.flagValue;
        callback.resultKnown = true;
        callback.result = waitingStep.action.callback.returnValue;
        if (!Adapter::BuildCallback1537CTypedFeedback(
                waitingStep,
                callback,
                feedback)) {
            return false;
        }
        break;
    }
    case Loader::ProducerStepKind::CallbackFlush26ECC: {
        Loader::CallbackFlush26ECCFeedback callback{};
        callback.audioFlushSubmitted = true;
        callback.resultKnown = true;
        callback.result = 0;
        if (!Adapter::BuildCallbackFlush26ECCTypedFeedback(
                waitingStep,
                callback,
                feedback)) {
            return false;
        }
        break;
    }
    case Loader::ProducerStepKind::CallbackDraw1EA74: {
        Loader::CallbackDraw1EA74Feedback callback{};
        callback.drawSubmitted = true;
        callback.argsKnown = true;
        callback.arg0 = waitingStep.action.callback.drawArg0;
        callback.arg1 = waitingStep.action.callback.drawArg1;
        callback.resultKnown = true;
        callback.result = 0;
        if (!Adapter::BuildCallbackDraw1EA74TypedFeedback(
                waitingStep,
                callback,
                feedback)) {
            return false;
        }
        break;
    }
    default:
        return false;
    }

    return ApplyBootstrap15590LoaderTypedFeedback801C81EC(
        state,
        feedback);
}

Bootstrap15590PumpHostBlockResult801C81EC
ResolveBootstrap15590PumpHostBlockResult801C81EC(
    const Bootstrap15590LoaderPumpResult801C81EC& pump) {
    Bootstrap15590PumpHostBlockResult801C81EC out{};
    out.poppedActionCount = pump.poppedActionCount;
    out.completed = pump.completed;
    out.failed = pump.failed;
    out.waitingForFeedback = pump.waitingForFeedback;
    out.externalProducerRequired = pump.externalProducerRequired;
    out.helperGap = pump.helperGap;
    out.reachedStepLimit = pump.reachedStepLimit;
    out.waiting =
        pump.waitingForFeedback ||
        pump.externalProducerRequired ||
        pump.helperGap ||
        pump.reachedStepLimit;
    out.shouldApplyCompletedFeedback =
        pump.completed &&
        !pump.failed &&
        !pump.waitingForFeedback &&
        !pump.externalProducerRequired &&
        !pump.helperGap &&
        !pump.reachedStepLimit;
    if (out.shouldApplyCompletedFeedback) {
        out.feedback.kind = HostBlockKind801C81EC::Bootstrap15590;
        out.feedback.completed = true;
    }
    out.action = MakeHostBlockResult801C81EC(true);
    return out;
}

Bootstrap15590HostTickPlan801C81EC BuildBootstrap15590HostTickPlan801C81EC(
    const Bootstrap15590HostTickInput801C81EC& input) {
    Bootstrap15590HostTickPlan801C81EC plan{};
    if (input.block.kind != HostBlockKind801C81EC::Bootstrap15590 ||
        !input.block.active) {
        return plan;
    }

    plan.active = true;
    if (!input.pumpKnown ||
        (input.callbackAttempted && input.callbackApplied) ||
        (input.cdLookupAttempted && input.cdLookupApplied) ||
        (input.cdLowerAttempted && input.cdLowerApplied) ||
        (input.recordDispatchAttempted && input.recordDispatchApplied)) {
        plan.command = Bootstrap15590HostTickCommand801C81EC::Pump;
        return plan;
    }

    constexpr uint8_t kMaxCallbackPumps = 3;
    constexpr uint8_t kMaxCdLookupPumps = 4;
    constexpr uint8_t kMaxCdLowerPumps = 2;
    constexpr uint8_t kMaxRecordDispatchPumps = 2;
    if (input.pump.waitingForFeedback &&
        input.cdLookupPumpCount == 0 &&
        !input.callbackAttempted &&
        input.callbackPumpCount < kMaxCallbackPumps) {
        plan.command = Bootstrap15590HostTickCommand801C81EC::TryCallback;
        return plan;
    }

    if (input.pump.waitingForFeedback &&
        !input.cdLookupApplied &&
        input.cdLookupPumpCount < kMaxCdLookupPumps) {
        plan.command = Bootstrap15590HostTickCommand801C81EC::TryCdLookup;
        return plan;
    }

    if (input.pump.waitingForFeedback &&
        !input.cdLowerApplied &&
        input.cdLowerPumpCount < kMaxCdLowerPumps) {
        plan.command = Bootstrap15590HostTickCommand801C81EC::TryCdLower;
        return plan;
    }

    if (input.pump.waitingForFeedback &&
        !input.recordDispatchApplied &&
        input.recordDispatchPumpCount < kMaxRecordDispatchPumps) {
        plan.command =
            Bootstrap15590HostTickCommand801C81EC::TryRecordDispatch;
        return plan;
    }

    plan.command = Bootstrap15590HostTickCommand801C81EC::ResolvePumpHost;
    plan.pumpHost = ResolveBootstrap15590PumpHostBlockResult801C81EC(
        input.pump);
    return plan;
}

ActionApplyResult801C81EC ApplyBootstrap15590PumpHostBlockResult801C81EC(
    State801C81EC& state,
    const Bootstrap15590PumpHostBlockResult801C81EC& pumpHost) {
    if (pumpHost.shouldApplyCompletedFeedback) {
        return ApplyHostBlockFeedback801C81EC(state, pumpHost.feedback);
    }
    return pumpHost.action;
}

bool RunBootstrap15590LoaderMemoryProducer801C81EC(
    State801C81EC& state,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::MemoryRuntimeStepResult* out) {
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::MemoryRuntimeStepResult{};
        }
        return false;
    }
    LoaderOwnerState801C81EC& owner = EnsureLoaderOwnerState801C81EC(state);
    const bool produced =
        PrStage1LoaderProducerAdapter::RunMemoryProducerForWaitingStep(
            owner.runnerState,
            owner.producerRuntime,
            applyFeedback,
            out);
    SyncBootstrap15590LoaderDirectFlags801C81EC(state, owner.runnerState);
    return produced;
}

bool BuildBootstrap15590LoaderRecordDispatchLiveInput801C81EC(
    const State801C81EC& state,
    PrStage1LoaderProducerAdapter::RecordDispatchLiveInput& out) {
    out = PrStage1LoaderProducerAdapter::RecordDispatchLiveInput{};
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        return false;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step) ||
        !step.valid ||
        step.stepKind !=
            PrStage1LoaderDirect::ProducerStepKind::RecordDispatch1A8F0) {
        return false;
    }

    PrStage1LoaderDirect::RecordData recordData{};
    if (!PrStage1LoaderDirect::TryBuildRecordDataFromDescriptorPayload(
            state.loaderOwner->runnerState,
            step.recordIndex,
            recordData)) {
        return false;
    }

    out.recordData = recordData;
    out.recordDataKnown = true;
    return true;
}

Bootstrap15590RecordDispatchProbe801C81EC
ProbeBootstrap15590LoaderRecordDispatch801C81EC(
    const State801C81EC& state) {
    Bootstrap15590RecordDispatchProbe801C81EC out{};
    out.active = state.bootstrap15590Active;
    out.begun = state.bootstrap15590LoaderDirectBegun;
    out.loaderOwnerKnown = state.loaderOwner != nullptr;
    if (!out.active || !out.begun || !state.loaderOwner) {
        return out;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    out.waitingStepKnown =
        PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step);
    out.waitingStepValid = step.valid;
    out.stepKind = step.stepKind;
    out.recordIndex = step.recordIndex;
    out.recordType = step.recordType;

    PrStage1LoaderDirect::ResolvedPayload payload{};
    if (PrStage1LoaderDirect::TryGetCurrentResolvedPayload(
            state.loaderOwner->runnerState,
            payload)) {
        out.currentPayloadKnown = true;
        out.currentPayloadValid = payload.valid;
        out.currentPayloadRecordType = payload.recordType;
        out.currentPayloadLiveBytes = payload.liveBytesPresent;
        out.currentPayloadLiveDataKnown = payload.liveBytesData != nullptr;
        out.currentPayloadLiveSizeKnown = payload.liveBytesSizeKnown;
        out.currentPayloadLiveSize = payload.liveBytesSize;
    }
    if (PrStage1LoaderDirect::TryGetResolvedPayloadForRecord(
            state.loaderOwner->runnerState,
            0,
            payload)) {
        out.historyRecord0PayloadKnown = true;
        out.historyRecord0PayloadValid = payload.valid;
        out.historyRecord0PayloadRecordType = payload.recordType;
        out.historyRecord0PayloadLiveBytes = payload.liveBytesPresent;
        out.historyRecord0PayloadLiveDataKnown =
            payload.liveBytesData != nullptr;
        out.historyRecord0PayloadLiveSizeKnown = payload.liveBytesSizeKnown;
        out.historyRecord0PayloadLiveSize = payload.liveBytesSize;
    }

    PrStage1LoaderDirect::RecordData recordData{};
    out.recordDataBuilt =
        PrStage1LoaderDirect::TryBuildRecordDataFromDescriptorPayload(
            state.loaderOwner->runnerState,
            step.recordIndex,
            recordData);
    return out;
}

bool BuildBootstrap15590LoaderRecordDispatchLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderProducerAdapter::RecordDispatchLiveInput& input,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
        }
        return false;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step)) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
            out->status =
                PrStage1LoaderProducerAdapter::AdapterStatus::NoWaitingStep;
        }
        return false;
    }

    return PrStage1LoaderProducerAdapter::BuildRecordDispatchLiveTypedFeedback(
        step,
        input,
        outFeedback,
        out);
}

bool RunBootstrap15590LoaderRecordDispatchLiveProducer801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::RecordDispatchLiveInput& input,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    if (outFeedback) {
        *outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    }
    if (out) {
        *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
    }
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        return false;
    }

    PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult buildResult{};
    if (!BuildBootstrap15590LoaderRecordDispatchLiveTypedFeedback801C81EC(
            state,
            input,
            feedback,
            &buildResult)) {
        if (outFeedback) {
            *outFeedback = feedback;
        }
        if (out) {
            *out = buildResult;
        }
        return false;
    }

    if (outFeedback) {
        *outFeedback = feedback;
    }
    if (out) {
        *out = buildResult;
    }
    if (!applyFeedback) {
        return true;
    }

    const bool applied = PrStage1LoaderProducerAdapter::ApplyTypedFeedback(
        state.loaderOwner->runnerState,
        feedback);
    SyncBootstrap15590LoaderDirectFlags801C81EC(
        state,
        state.loaderOwner->runnerState);
    return applied;
}

static bool BuildBootstrap15590LoaderCdLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderDirect::CdSeamResult& cd,
    bool success,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
        }
        return false;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step)) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
            out->status =
                PrStage1LoaderProducerAdapter::AdapterStatus::NoWaitingStep;
        }
        return false;
    }
    return PrStage1LoaderProducerAdapter::BuildCdLiveTypedFeedback(
        step,
        cd,
        success,
        outFeedback,
        out);
}

static bool RunBootstrap15590LoaderCdSeamLiveProducer801C81EC(
    State801C81EC& state,
    const PrStage1LoaderDirect::CdSeamResult& cd,
    bool success,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    if (outFeedback) {
        *outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    }
    if (out) {
        *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
    }
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        return false;
    }

    PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult buildResult{};
    if (!BuildBootstrap15590LoaderCdLiveTypedFeedback801C81EC(
            state,
            cd,
            success,
            feedback,
            &buildResult)) {
        if (outFeedback) {
            *outFeedback = feedback;
        }
        if (out) {
            *out = buildResult;
        }
        MarkBootstrap15590CdLowerFeedbackRejected801C81EC(
            state.bootstrap15590CdLowerProducerRuntime,
            cd,
            Bootstrap15590CdLowerRejectReason801C81EC::AdapterNotReady,
            Bootstrap15590CdLowerAttemptStatus801C81EC::FeedbackRejected,
            cd.lowerRequest);
        return false;
    }

    if (outFeedback) {
        *outFeedback = feedback;
    }
    if (out) {
        *out = buildResult;
    }
    if (!applyFeedback) {
        return true;
    }

    const bool applied = PrStage1LoaderProducerAdapter::ApplyTypedFeedback(
        state.loaderOwner->runnerState,
        feedback);
    MarkBootstrap15590CdLowerFeedbackResolved801C81EC(
        state.bootstrap15590CdLowerProducerRuntime,
        feedback,
        applied);
    if (applied) {
        RecordBootstrap15590MovieSegmentCdLookupTypedFeedback801C81EC(
            state,
            feedback);
    }
    SyncBootstrap15590LoaderDirectFlags801C81EC(
        state,
        state.loaderOwner->runnerState);
    return applied;
}

bool RunBootstrap15590CdLowerFacts801C81EC(
    State801C81EC& state,
    PrStage1XaCdDirectState& xaCdState,
    const PrStage1LowerCdProducerDirect::LowerCdProducerFacts& facts,
    Bootstrap15590CdLowerFactsApplyResult801C81EC* out,
    bool applyLoaderFeedback) {
    Bootstrap15590CdLowerFactsApplyResult801C81EC result{};
    result.attempted = true;
    result.requestPendingBefore =
        state.bootstrap15590CdLowerProducerRuntime.requestPending;
    result.actualLowerRequest = facts.request;
    if (out) {
        *out = result;
    }

    Bootstrap15590CdLowerHostRequest801C81EC request{};
    const auto reject =
        [&](Bootstrap15590CdLowerRejectReason801C81EC reason) {
            result.rejectReason = reason;
            result.status = request.status;
            result.expectedLowerRequest = request.lowerRequest;
            if (request.valid &&
                (state.bootstrap15590CdLowerProducerRuntime.requestPending ||
                 reason ==
                     Bootstrap15590CdLowerRejectReason801C81EC::
                         ApplyRejected)) {
                RecordBootstrap15590CdLowerFeedbackRejected801C81EC(
                    state.bootstrap15590CdLowerProducerRuntime,
                    reason,
                    request.status,
                    request.lowerRequest,
                    facts.request);
            }
            result.requestPendingAfter =
                state.bootstrap15590CdLowerProducerRuntime.requestPending;
            if (out) {
                *out = result;
            }
            return false;
        };

    if (!BuildBootstrap15590CdLowerProducerRequest801C81EC(state,
                                                           request)) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::NoPendingRequest);
    }
    result.status = request.status;
    result.expectedLowerRequest = request.lowerRequest;
    if (!request.valid) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::RequestInvalid);
    }
    if (request.cdActionKind ==
        PrStage1LoaderCdHal::ActionKind::Lookup800381F8) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::LookupRequest);
    }
    const bool readSyncCompletion =
        IsReadSyncCompletionForPendingReadStart801C81EC(request,
                                                        facts.request);
    if (!readSyncCompletion &&
        request.cdActionKind != facts.request.actionKind) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::ActionKindMismatch);
    }
    if (!readSyncCompletion &&
        !LowerCdRequestMetadataMatches801C81EC(request.lowerRequest,
                                               facts.request)) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::MetadataMismatch);
    }
    if (!facts.overlayTransferAttempt.known ||
        !facts.overlayTransferAttempt.attemptIndexKnown) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::
                AttemptProvenanceMissing);
    }
    if (!LowerCdAttemptProvenanceMatches801C81EC(
            request,
            facts.overlayTransferAttempt)) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::
                AttemptProvenanceMismatch);
    }
    if (request.status != Bootstrap15590CdLowerAttemptStatus801C81EC::
                              RequestReady &&
        request.status != Bootstrap15590CdLowerAttemptStatus801C81EC::
                              RequestAlreadyPending) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::RequestNotReady);
    }
    if (readSyncCompletion &&
        !IsBootstrap15590FinalReadyHalFactsRequirementCurrent801C81EC(
            state.bootstrap15590CdLowerProducerRuntime,
            request,
            xaCdState.readS27Serial)) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::
                FinalReadyRequirementMissing);
    }
    result.requestMatched = true;
    result.status = request.status;

    PrStage1XaCdDirectLowerCdSnapshotBridgeInput bridgeInput{};
    if (facts.cdSyncLoopFacts80037070Known &&
        facts.request.actionKind ==
            PrStage1LoaderCdHal::ActionKind::SeekSync800367A4) {
        bridgeInput.cdSyncLoopFacts80037070Known = true;
        bridgeInput.cdSyncLoopFacts80037070 =
            facts.cdSyncLoopFacts80037070;
    }
    bridgeInput.lowerCdFactsKnown = true;
    bridgeInput.lowerCdFacts = facts;
    const PrStage1XaCdDirectLowerCdSnapshotBridgeResult snapshot =
        PrStage1XaCdDirectBuildLowerCdProducerSnapshot(bridgeInput);
    if (!snapshot.produced || snapshot.incomplete) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::BridgeIncomplete);
    }

    const PrStage1XaCdDirectLowerCdProducerResult xaCd =
        PrStage1XaCdDirectApplyLowerCdProducerSnapshot(xaCdState,
                                                       snapshot.snapshot);
    result.xaCdAccepted = xaCd.cdSeamResultAccepted;
    if (!xaCd.cdSeamResultAccepted) {
        if (xaCd.cdSeamResult.present) {
            MarkBootstrap15590CdLowerFeedbackRejected801C81EC(
                state.bootstrap15590CdLowerProducerRuntime,
                xaCd.cdSeamResult,
                Bootstrap15590CdLowerRejectReason801C81EC::XaCdRejected,
                result.status,
                facts.request);
        }
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::XaCdRejected);
    }
    if (!applyLoaderFeedback) {
        if (facts.request.actionKind ==
            PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0) {
            const uint32_t progressSerial = xaCdState.readS27Serial;
            state.bootstrap15590CdLowerProducerRuntime.
                readStartHalProgressReadS27Serial = progressSerial;
            state.bootstrap15590CdLowerProducerRuntime.
                readStartHalProgressAccepted = progressSerial != 0u;
        }
        result.applied = true;
        result.requestPendingAfter =
            state.bootstrap15590CdLowerProducerRuntime.requestPending;
        if (out) {
            *out = result;
        }
        return true;
    }

    PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult build{};
    const bool applied = RunBootstrap15590LoaderCdSeamLiveProducer801C81EC(
        state,
        xaCd.cdSeamResult,
        xaCd.cdSeamResult.feedback.success,
        true,
        &feedback,
        &build);
    result.feedback = feedback;
    result.build = build;
    result.feedbackReady = build.status ==
        PrStage1LoaderProducerAdapter::AdapterStatus::Ready;
    result.applied = applied;
    if (!result.feedbackReady) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::AdapterNotReady);
    }
    if (!applied) {
        return reject(
            Bootstrap15590CdLowerRejectReason801C81EC::ApplyRejected);
    }
    result.status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::FeedbackApplied;
    result.requestPendingAfter =
        state.bootstrap15590CdLowerProducerRuntime.requestPending;
    if (out) {
        *out = result;
    }
    return applied;
}

static bool RunBootstrap15590LoaderCdLookupLiveProducerInternal801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::CdLookupLiveInput& input,
    bool success,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out,
    const PrStage1XaCdDirectState* explicitXaCdState,
    const PrStage1XaCdDirectStreamClockProbe800493F4*
        explicitStreamClockProbe,
    const PrMovieSegmentDirect::StreamClockProducerCarrier800493F4*
        explicitStreamClockCarrier) {
    if (outFeedback) {
        *outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    }
    if (out) {
        *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
    }
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        return false;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step)) {
        if (out) {
            out->status =
                PrStage1LoaderProducerAdapter::AdapterStatus::NoWaitingStep;
        }
        return false;
    }

    PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult buildResult{};
    const PrStage1LoaderDirect::CdSeamResult cd =
        BuildBootstrap15590CdLookupSeamWithExplicitClock801C81EC(
            input,
            explicitXaCdState,
            explicitStreamClockProbe,
            explicitStreamClockCarrier);
    if (!PrStage1LoaderProducerAdapter::BuildCdLiveTypedFeedback(
            step,
            cd,
            success,
            feedback,
            &buildResult)) {
        if (outFeedback) {
            *outFeedback = feedback;
        }
        if (out) {
            *out = buildResult;
        }
        return false;
    }

    if (outFeedback) {
        *outFeedback = feedback;
    }
    if (out) {
        *out = buildResult;
    }
    if (!applyFeedback) {
        return true;
    }

    const bool applied = PrStage1LoaderProducerAdapter::ApplyTypedFeedback(
        state.loaderOwner->runnerState,
        feedback);
    if (applied) {
        RecordBootstrap15590MovieSegmentCdLookupTypedFeedback801C81EC(
            state,
            feedback);
    }
    SyncBootstrap15590LoaderDirectFlags801C81EC(
        state,
        state.loaderOwner->runnerState);
    return applied;
}

bool BuildBootstrap15590MovieSegmentCdLookupCarrier801C81EC(
    State801C81EC& state,
    const std::filesystem::path& binPath,
    bool binPathKnown,
    const PrStage1XaCdDirectState* xaCdState,
    PrStage1MovieSegmentDirect::MovieSegmentCdLookupBatchFeedback801C4780&
        out) {
    const PrMovieSegmentDirect::StreamClockProducerCarrier800493F4*
        streamClockCarrier = nullptr;
    bool gapMissingStreamClockProducer = false;
    PrMovieSegmentDirect::StreamClockProducerCarrier800493F4 carrier{};
    if (xaCdState != nullptr) {
        const PrStage1XaCdDirectStreamClockProbe800493F4 clockProbe =
            PrStage1XaCdDirectProbeStreamClockProducer800493F4(*xaCdState);
        carrier = clockProbe.carrier;
        streamClockCarrier = &carrier;
        gapMissingStreamClockProducer =
            clockProbe.gapMissingStreamClock800493F4Producer;
    }
    return PrStage1MovieSegmentDirect::
        BuildStage1MovieSegmentCdLookupCarrier801C4780(
            state.bootstrap15590MovieSegmentCdLookupRuntime,
            PrStage1MovieSegmentDirect::
                kStage1SceneEntryMovieSegmentSceneIndex801C4780,
            binPath,
            binPathKnown,
            streamClockCarrier,
            gapMissingStreamClockProducer,
            out);
}

PrStageStatusBankDirectCallRequest BuildStatusBankDirectMemoryRequest801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    using ActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    PrStageStatusBankDirectCallRequest request{};
    request.psxFunction = action.psxFunction;
    switch (action.kind) {
    case ActionKind::QueryStageStatus166AC:
        if (!IsExactPsxActionFunction801C81EC(action, kFn800166AC) ||
            action.rawArg0 != action.sceneId) {
            break;
        }
        request.valid = true;
        request.kind = PrStageStatusBankActionKind::Call800166AC;
        request.arg0Known = true;
        request.arg0 = action.sceneId;
        break;
    case ActionKind::SaveStatus1635C:
        if (!IsExactPsxActionFunction801C81EC(action, kFn8001635C) ||
            action.rawArg0 != action.sceneId ||
            action.rawArg1 != static_cast<uint32_t>(action.arg0) ||
            action.rawArg2 != static_cast<uint32_t>(action.arg1) ||
            action.rawArg3 != static_cast<uint32_t>(action.arg2)) {
            break;
        }
        request.valid = true;
        request.kind = PrStageStatusBankActionKind::Call8001635C;
        request.arg0Known = true;
        request.arg0 = action.sceneId;
        request.arg1Known = true;
        request.arg1 = action.arg0;
        request.arg2Known = true;
        request.arg2 = action.arg1;
        request.arg3Known = true;
        request.arg3 = action.arg2;
        break;
    case ActionKind::UnlockNextStage1628C:
        if (!IsExactPsxActionFunction801C81EC(action, kFn8001628C) ||
            action.rawArg0 != action.sceneId) {
            break;
        }
        request.valid = true;
        request.kind = PrStageStatusBankActionKind::Call8001628C;
        request.arg0Known = true;
        request.arg0 = action.sceneId;
        break;
    case ActionKind::None:
    case ActionKind::ConfigMovieViewport:
    case ActionKind::AudioReset26FA4:
    case ActionKind::Transition201AC:
    case ActionKind::Transition20110:
    case ActionKind::StrInit:
    case ActionKind::StrPlayAndWait:
    case ActionKind::StageRecordTick1A4D0:
    case ActionKind::StageRunnerRun7A60:
    case ActionKind::ResetHoldTiles1EF14:
    case ActionKind::RestoreTransitionPayload15744:
    case ActionKind::QueryAbort26B94:
    case ActionKind::SfxCue26EF8:
    case ActionKind::AudioFlush26ECC:
    case ActionKind::Bootstrap15590:
    case ActionKind::SaveUi19148:
        break;
    }
    return request;
}

ActionApplyResult801C81EC ApplyStatusBankDirectMemoryFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const StatusBankDirectMemoryFeedback801C81EC& feedback) {
    using ActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    if (action.kind == ActionKind::QueryStageStatus166AC &&
        feedback.requestHandled && feedback.status166ACKnown) {
        SetStageStatus166AC(state, feedback.status166AC);
        return MakeImmediateInputResult801C81EC();
    }
    return ActionApplyResult801C81EC{};
}

StatusBankDirectMemoryGap801C81EC
BuildStatusBankDirectMemoryGap801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const StatusBankDirectMemoryFeedback801C81EC& feedback) {
    using ActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    StatusBankDirectMemoryGap801C81EC out{};
    const bool saveStatus1635CWriterPublished =
        action.kind == ActionKind::SaveStatus1635C &&
        feedback.payloadKnown &&
        feedback.payloadPrefixKnown80092F10 &&
        feedback.payloadPrefixStatusBankKnown80092F1D &&
        feedback.payloadLastWriterFunction == kFn8001635C &&
        feedback.payloadWrote8001635C;
    if (feedback.requestHandled &&
        (action.kind != ActionKind::SaveStatus1635C ||
         (feedback.payloadOk && saveStatus1635CWriterPublished))) {
        return out;
    }

    out.sceneId = action.sceneId;
    out.arg0 = action.arg0;
    out.arg1 = action.arg1;
    out.requestValid = feedback.requestValid;
    out.psxFunction = feedback.psxFunction;
    out.psxFunctionMatched = feedback.psxFunctionMatched;
    out.payloadKnown = feedback.payloadKnown;
    out.payloadPrefixKnown80092F10 = feedback.payloadPrefixKnown80092F10;
    out.payloadPrefixStatusBankKnown80092F1D =
        feedback.payloadPrefixStatusBankKnown80092F1D;
    out.payloadLastWriterFunction = feedback.payloadLastWriterFunction;
    out.payloadWrote8001635C = feedback.payloadWrote8001635C;
    out.helperGap = feedback.payloadHelperGap;
    out.lastFaultAddress = feedback.payloadLastFaultAddress;
    switch (action.kind) {
    case ActionKind::QueryStageStatus166AC:
        if (!feedback.requestValid ||
            feedback.requestKind ==
                PrStageStatusBankActionKind::Call800166AC) {
            out.kind =
                StatusBankDirectMemoryGapKind801C81EC::QueryStageStatus166AC;
            out.shouldLog = true;
            out.sceneId =
                feedback.requestValid ? feedback.status166ACSceneId
                                      : action.sceneId;
            out.status166ACMapped = feedback.status166ACMapped;
            out.status166ACSlotIndex = feedback.status166ACSlotIndex;
            out.status166ACStatusBankKnown =
                feedback.status166ACStatusBankKnown;
            out.helperGap = feedback.status166ACHelperGap;
        }
        break;
    case ActionKind::SaveStatus1635C:
        if (!feedback.requestValid ||
            feedback.requestKind ==
                PrStageStatusBankActionKind::Call8001635C) {
            out.kind =
                StatusBankDirectMemoryGapKind801C81EC::SaveStatus1635C;
            out.shouldLog = true;
        }
        break;
    case ActionKind::UnlockNextStage1628C:
        if (!feedback.requestValid ||
            feedback.requestKind ==
                PrStageStatusBankActionKind::Call8001628C) {
            out.kind =
                StatusBankDirectMemoryGapKind801C81EC::UnlockNextStage1628C;
            out.shouldLog = true;
        }
        break;
    default:
        break;
    }
    return out;
}

SaveStatus1635CReplayBackupHostRequest801C81EC
BuildSaveStatus1635CReplayBackupHostRequest801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    using ActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    SaveStatus1635CReplayBackupHostRequest801C81EC request{};
    if (action.kind != ActionKind::SaveStatus1635C ||
        !IsExactPsxActionFunction801C81EC(action, kFn8001635C) ||
        action.rawArg0 != action.sceneId ||
        action.rawArg1 != static_cast<uint32_t>(action.arg0) ||
        action.rawArg2 != static_cast<uint32_t>(action.arg1) ||
        action.rawArg3 != static_cast<uint32_t>(action.arg2)) {
        return request;
    }

    request.valid = true;
    request.psxOrder = action.psxOrder;
    request.psxFunctionKnown = action.psxFunctionKnown;
    request.psxFunction = action.psxFunction;
    request.prevGrade92F40Known = true;
    request.prevGrade92F40 = static_cast<uint32_t>(action.arg1);
    return request;
}

ActionApplyResult801C81EC ApplySaveStatus1635CReplayBackupHostFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const SaveStatus1635CReplayBackupHostFeedback801C81EC& feedback) {
    (void)state;
    if (action.kind !=
            PrStage1LifecycleDirect::ActionKind801C81EC::SaveStatus1635C ||
        !feedback.requestValid ||
        !feedback.psxFunctionMatched ||
        !feedback.captureAttempted ||
        !feedback.backupValid ||
        !feedback.backupPrevGrade92F40Known ||
        feedback.backupPrevGrade92F40 !=
            static_cast<uint32_t>(action.arg1) ||
        !feedback.requestHandled) {
        return ActionApplyResult801C81EC{};
    }
    return ActionApplyResult801C81EC{};
}

SaveStatus1635CReplayBackupHostGap801C81EC
BuildSaveStatus1635CReplayBackupHostGap801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const SaveStatus1635CReplayBackupHostFeedback801C81EC& feedback) {
    SaveStatus1635CReplayBackupHostGap801C81EC out{};
    if (action.kind !=
            PrStage1LifecycleDirect::ActionKind801C81EC::SaveStatus1635C ||
        feedback.requestHandled) {
        return out;
    }

    out.shouldLog = true;
    out.psxOrder = action.psxOrder;
    out.requestValid = feedback.requestValid;
    out.psxFunctionKnown = feedback.psxFunctionKnown;
    out.psxFunction = feedback.psxFunction;
    out.psxFunctionMatched = feedback.psxFunctionMatched;
    out.prevGrade92F40Known = feedback.prevGrade92F40Known;
    out.hostCallbackKnown = feedback.hostCallbackKnown;
    out.replayMirrorKnown8008EEF8 = feedback.replayMirrorKnown8008EEF8;
    out.replayMirrorProducerKnown8008EEF8 =
        feedback.replayMirrorProducerKnown8008EEF8;
    out.replayMirrorProducerFunction =
        feedback.replayMirrorProducerFunction;
    out.replayMirrorByteCountKnown8008EEF8 =
        feedback.replayMirrorByteCountKnown8008EEF8;
    out.replayMirrorKnownByteCount8008EEF8 =
        feedback.replayMirrorKnownByteCount8008EEF8;
    out.replayPublishedCount901BC = feedback.replayPublishedCount901BC;
    out.replayWriteCount901C0 = feedback.replayWriteCount901C0;
    out.captureAttempted = feedback.captureAttempted;
    out.backupValid = feedback.backupValid;
    out.backupPrevGrade92F40Known = feedback.backupPrevGrade92F40Known;
    out.backupPrevGrade92F40 = feedback.backupPrevGrade92F40;
    out.backupPublishedCount901BC = feedback.backupPublishedCount901BC;
    out.requestHandled = feedback.requestHandled;
    return out;
}

PrStage1LifecycleDirect::InitSceneInput801C7284
BuildStage1InitSceneInputWithMovieSegmentFeedback801C7284(
    State801C81EC& state,
    const std::filesystem::path& dataRoot,
    const PrStage1XaCdDirectState* xaCdState) {
    PrStage1LifecycleDirect::InitSceneInput801C7284 input{};
    input.sceneIndex =
        PrStage1MovieSegmentDirect::
            kStage1SceneEntryMovieSegmentSceneIndex801C4780;

    std::filesystem::path binPath;
    const bool binPathKnown =
        TryResolveStage1MovieSegmentIso9660BinPath801C81EC(dataRoot, binPath);
    input.movieSegmentCdLookupBatchFeedbackKnown =
        BuildBootstrap15590MovieSegmentCdLookupCarrier801C81EC(
            state,
            binPath,
            binPathKnown,
            xaCdState,
            input.movieSegmentCdLookupBatchFeedback);
    return input;
}

bool BuildBootstrap15590LoaderGpuTimBytesLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderProducerAdapter::GpuTimBytesLiveInput& timBytes,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
        }
        return false;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step)) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
            out->status =
                PrStage1LoaderProducerAdapter::AdapterStatus::NoWaitingStep;
        }
        return false;
    }
    return PrStage1LoaderProducerAdapter::BuildGpuTimBytesLiveTypedFeedback(
        step,
        timBytes,
        outFeedback,
        out);
}

bool RunBootstrap15590LoaderGpuTimBytesLiveProducer801C81EC(
    State801C81EC& state,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    if (outFeedback) {
        *outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    }
    if (out) {
        *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
    }
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        return false;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step)) {
        if (out) {
            out->status =
                PrStage1LoaderProducerAdapter::AdapterStatus::NoWaitingStep;
        }
        return false;
    }

    PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult buildResult{};
    if (!PrStage1LoaderProducerAdapter::
            BuildGpuTimBytesLiveTypedFeedbackFromResolvedPayload(
                state.loaderOwner->runnerState,
                step,
                feedback,
                &buildResult)) {
        if (outFeedback) {
            *outFeedback = feedback;
        }
        if (out) {
            *out = buildResult;
        }
        return false;
    }

    if (outFeedback) {
        *outFeedback = feedback;
    }
    if (out) {
        *out = buildResult;
    }
    if (!applyFeedback) {
        return true;
    }

    const bool applied = PrStage1LoaderProducerAdapter::ApplyTypedFeedback(
        state.loaderOwner->runnerState,
        feedback);
    SyncBootstrap15590LoaderDirectFlags801C81EC(
        state,
        state.loaderOwner->runnerState);
    return applied;
}

bool BuildBootstrap15590LoaderSpuHalResultLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderDirect::SpuSeamResult& spu,
    bool success,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
        }
        return false;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step)) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
            out->status =
                PrStage1LoaderProducerAdapter::AdapterStatus::NoWaitingStep;
        }
        return false;
    }
    return PrStage1LoaderProducerAdapter::BuildSpuHalResultLiveTypedFeedback(
        step,
        spu,
        success,
        outFeedback,
        out);
}

bool BuildBootstrap15590LoaderSpuHalLowerResultLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    PrStage1LoaderSpuHal::State& spuState,
    const PrStage1LoaderProducerAdapter::SpuHalResultLiveInput& input,
    bool success,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
        }
        return false;
    }

    PrStage1LoaderDirect::ProducerStep step{};
    if (!PrStage1LoaderDirect::DescribeWaitingProducerStep(
            state.loaderOwner->runnerState,
            step)) {
        if (out) {
            *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
            out->status =
                PrStage1LoaderProducerAdapter::AdapterStatus::NoWaitingStep;
        }
        return false;
    }

    return PrStage1LoaderProducerAdapter::
        BuildSpuHalResultLiveTypedFeedbackFromLowerResult(
            step,
            spuState,
            input,
            success,
            outFeedback,
            out);
}

bool RunBootstrap15590LoaderSpuHalLowerResultLiveProducer801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::SpuHalResultLiveInput& input,
    bool success,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out) {
    if (outFeedback) {
        *outFeedback = PrStage1LoaderProducerAdapter::TypedActionFeedback{};
    }
    if (out) {
        *out = PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult{};
    }
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun ||
        !state.loaderOwner) {
        return false;
    }

    PrStage1LoaderSpuHal::State candidateSpuState =
        state.bootstrap15590LoaderSpuHalState;
    PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult buildResult{};
    if (!BuildBootstrap15590LoaderSpuHalLowerResultLiveTypedFeedback801C81EC(
            state,
            candidateSpuState,
            input,
            success,
            feedback,
            &buildResult)) {
        if (outFeedback) {
            *outFeedback = feedback;
        }
        if (out) {
            *out = buildResult;
        }
        return false;
    }

    if (outFeedback) {
        *outFeedback = feedback;
    }
    if (out) {
        *out = buildResult;
    }
    if (!applyFeedback) {
        return true;
    }

    const bool applied = PrStage1LoaderProducerAdapter::ApplyTypedFeedback(
        state.loaderOwner->runnerState,
        feedback);
    if (applied) {
        state.bootstrap15590LoaderSpuHalState = candidateSpuState;
    }
    SyncBootstrap15590LoaderDirectFlags801C81EC(
        state,
        state.loaderOwner->runnerState);
    return applied;
}

bool ApplyBootstrap15590LoaderTypedFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::TypedActionFeedback& feedback) {
    if (!state.bootstrap15590Active || !state.bootstrap15590LoaderDirectBegun) {
        return false;
    }
    if (feedback.cdFeedback) {
        return false;
    }
    LoaderOwnerState801C81EC& owner = EnsureLoaderOwnerState801C81EC(state);
    const bool applied = PrStage1LoaderProducerAdapter::ApplyTypedFeedback(
        owner.runnerState,
        feedback);
    MarkBootstrap15590CdLowerFeedbackResolved801C81EC(
        state.bootstrap15590CdLowerProducerRuntime,
        feedback,
        applied);
    if (applied) {
        RecordBootstrap15590MovieSegmentCdLookupTypedFeedback801C81EC(
            state,
            feedback);
    }
    SyncBootstrap15590LoaderDirectFlags801C81EC(state, owner.runnerState);
    return applied;
}

ActionApplyResult801C81EC BeginHostBlock801C81EC(
    State801C81EC& state,
    const HostBlockStart801C81EC& start) {
    ActionApplyResult801C81EC out{};
    switch (start.kind) {
    case HostBlockKind801C81EC::None:
        break;
    case HostBlockKind801C81EC::Movie1:
        if (!start.pathResolved || start.path.empty()) {
            break;
        }
        state.movie1PathResolved = start.pathResolved;
        state.movie1Path = start.path;
        StartStrBlock801C81EC(state, StrBlockKind801C81EC::Movie1);
        out.waitingForHostBlock = state.movie1BlockActive;
        break;
    case HostBlockKind801C81EC::ClearTailMovie:
        if (!start.pathResolved || start.path.empty()) {
            break;
        }
        StartClearTailMovieBlock801C81EC(
            state,
            start.pathResolved,
            start.path);
        out.waitingForHostBlock = state.clearTailMovieBlockActive;
        break;
    case HostBlockKind801C81EC::AbortPoll:
        SetAbortPollBlockActive801C81EC(state, start.active);
        out.waitingForHostBlock = state.abortPollBlockActive;
        break;
    case HostBlockKind801C81EC::Bootstrap15590:
        SetBootstrap15590Block801C81EC(
            state,
            start.active,
            start.curtainStarted,
            start.pathResolved,
            start.path);
        state.bootstrap15590SceneLoaderSlotKnown = start.sceneLoaderSlotKnown;
        state.bootstrap15590SceneLoaderSlotPresent = start.sceneLoaderSlotPresent;
        state.bootstrap15590SceneLoaderSlot = start.sceneLoaderSlot;
        state.bootstrap15590ZcompoLoader = start.bootstrapZcompoLoader;
        if (start.bootstrap15590DirectPlanKnown) {
            state.bootstrap15590DirectPlanKnown = true;
            state.bootstrap15590DirectPlan = start.bootstrap15590DirectPlan;
        }
        if (state.bootstrap15590DirectPlanKnown) {
            (void)BeginBootstrap15590LoaderDirect801C81EC(state);
        }
        out.waitingForHostBlock = state.bootstrap15590Active;
        break;
    case HostBlockKind801C81EC::SaveUi19148:
        if (!start.saveUi19148Seed80092F10Known ||
            start.saveUi19148Seed80092F10Address !=
                kAddrSavePayloadBank80092F10) {
            break;
        }
        state.saveUi19148Seed80092F10Known =
            start.saveUi19148Seed80092F10Known;
        state.saveUi19148Seed80092F10Address =
            start.saveUi19148Seed80092F10Address;
        if (start.startDeferred) {
            SetSaveUi19148StartPending801C81EC(state, true);
        } else {
            SetSaveUi19148StartPending801C81EC(state, false);
            SetSaveUi19148Active801C81EC(state, start.active);
        }
        out.waitingForHostBlock = HasSaveUi19148HostBlock801C81EC(state);
        break;
    }
    return out;
}

ActionApplyResult801C81EC ApplyHostBlockFeedback801C81EC(
    State801C81EC& state,
    const HostBlockFeedback801C81EC& feedback) {
    ActionApplyResult801C81EC out{};
    switch (feedback.kind) {
    case HostBlockKind801C81EC::None:
        break;
    case HostBlockKind801C81EC::Movie1:
        if (feedback.completed) {
            CompleteStrBlock801C81EC(
                state,
                StrBlockKind801C81EC::Movie1,
                feedback.result);
            out.producedImmediateInput = true;
        }
        out.waitingForHostBlock = state.movie1BlockActive;
        break;
    case HostBlockKind801C81EC::ClearTailMovie:
        if (feedback.completed) {
            CompleteStrBlock801C81EC(
                state,
                StrBlockKind801C81EC::ClearTailMovie,
                feedback.result);
            out.producedImmediateInput = true;
        }
        out.waitingForHostBlock = state.clearTailMovieBlockActive;
        break;
    case HostBlockKind801C81EC::AbortPoll:
        if (feedback.completed) {
            SetAbortPollBlockActive801C81EC(state, false);
            SetAbortPollResult26B94(state, feedback.result);
            out.producedImmediateInput = true;
        } else if (feedback.active) {
            SetAbortPollBlockActive801C81EC(state, true);
        }
        out.waitingForHostBlock = state.abortPollBlockActive;
        break;
    case HostBlockKind801C81EC::Bootstrap15590:
        if (feedback.completed) {
            ClearBootstrap15590Block801C81EC(state);
            out.producedImmediateInput = true;
        }
        out.waitingForHostBlock = state.bootstrap15590Active;
        break;
    case HostBlockKind801C81EC::SaveUi19148:
        if (feedback.startDeferred) {
            SetSaveUi19148StartPending801C81EC(state, true);
        } else if (feedback.active) {
            SetSaveUi19148StartPending801C81EC(state, false);
            SetSaveUi19148Active801C81EC(state, true);
        }
        if (feedback.completed) {
            SetSaveUi19148Active801C81EC(state, false);
            state.saveUi19148Seed80092F10Known = false;
            state.saveUi19148Seed80092F10Address = 0;
            out.producedImmediateInput = true;
        }
        out.waitingForHostBlock = HasSaveUi19148HostBlock801C81EC(state);
        break;
    }
    return out;
}

PrStage1LifecycleDirect::FrameInput801C81EC BuildFrameInput801C81EC(
    const State801C81EC& state,
    const FrameHostInput801C81EC& host) {
    PrStage1LifecycleDirect::FrameInput801C81EC input{};
    input.sceneId = host.sceneId;
    input.word800916D0 = host.word800916D0;
    input.word800916DA = host.word800916DA;
    input.word800916F0Known = host.word800916F0Known;
    input.word800916F0 = host.word800916F0;
    input.word80091816 = host.word80091816;
    input.dword801D3040 = host.dword801D3040;
    input.byte801C368E = host.byte801C368E;

    input.movie1PlayAndWaitResultKnown = state.movie1ResultKnown;
    input.movie1PlayAndWaitResult = state.movie1Result;
    input.stageResultKnown = state.stageResultKnown;
    input.stageResult801C7A60 = state.stageResult801C7A60;
    input.abortPollResultKnown = state.abortPollResultKnown;
    input.abortPollResult26B94 = state.abortPollResult26B94;
    input.stageStatus166ACKnown = state.stageStatus166ACKnown;
    input.stageStatus166AC = state.stageStatus166AC;
    input.clearTailPlayAndWaitResultKnown =
        state.clearTailMoviePlayAndWaitResultKnown;
    input.clearTailPlayAndWaitResult = state.clearTailMoviePlayAndWaitResult;
    return input;
}

void ConsumeFrameInputKnownFlags801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::FrameInput801C81EC& input) {
    if (input.movie1PlayAndWaitResultKnown) {
        state.movie1ResultKnown = false;
    }
    if (input.stageResultKnown) {
        state.stageResultKnown = false;
    }
    if (input.abortPollResultKnown) {
        state.abortPollResultKnown = false;
    }
    if (input.stageStatus166ACKnown) {
        state.stageStatus166ACKnown = false;
    }
    if (input.clearTailPlayAndWaitResultKnown) {
        state.clearTailMoviePlayAndWaitResultKnown = false;
        state.clearTailMoviePlayAndWaitResult = 0;
    }
}

void EnqueueActions801C81EC(
    State801C81EC& state,
    const std::vector<PrStage1LifecycleDirect::Action801C81EC>& actions,
    std::size_t firstActionIndex) {
    for (std::size_t i = firstActionIndex; i < actions.size(); i++) {
        RecordLifecycleActionState801C81EC(state, actions[i]);
        state.pendingActions.push_back(actions[i]);
    }
}

void EnqueueActionsAfterBlockedAction801C81EC(
    State801C81EC& state,
    const std::vector<PrStage1LifecycleDirect::Action801C81EC>& actions,
    std::size_t blockedActionIndex) {
    EnqueueActions801C81EC(state, actions, blockedActionIndex + 1u);
}

bool PopNextAction801C81EC(
    State801C81EC& state,
    PrStage1LifecycleDirect::Action801C81EC& out) {
    if (state.pendingActions.empty()) {
        return false;
    }
    out = state.pendingActions.front();
    state.pendingActions.erase(state.pendingActions.begin());
    RecordLifecycleActionState801C81EC(state, out);
    if (out.kind ==
            PrStage1LifecycleDirect::ActionKind801C81EC::StrPlayAndWait &&
        out.strBlockKind ==
            PrStage1LifecycleDirect::StrBlockKind801C81EC::ClearTailMovie) {
        state.clearTailMoviePlayAndWaitPending = false;
    }
    return true;
}

bool HasPendingActions801C81EC(const State801C81EC& state) {
    return !state.pendingActions.empty();
}

ActionApplyResult801C81EC MakeImmediateInputResult801C81EC() {
    ActionApplyResult801C81EC out{};
    out.producedImmediateInput = true;
    return out;
}

ActionApplyResult801C81EC MakeHostBlockResult801C81EC(bool active) {
    ActionApplyResult801C81EC out{};
    out.waitingForHostBlock = active;
    return out;
}

ActionApplyResult801C81EC MakeBlockedActionRetryResult801C81EC() {
    ActionApplyResult801C81EC out{};
    out.waitingForHostBlock = true;
    out.retryBlockedAction = true;
    return out;
}

void MergeActionApplyResult801C81EC(ActionApplyResult801C81EC& aggregate,
                                    const ActionApplyResult801C81EC& next) {
    aggregate.producedImmediateInput =
        aggregate.producedImmediateInput || next.producedImmediateInput;
    aggregate.waitingForHostBlock =
        aggregate.waitingForHostBlock || next.waitingForHostBlock;
    aggregate.retryBlockedAction =
        aggregate.retryBlockedAction || next.retryBlockedAction;
}

void StartStrBlock801C81EC(State801C81EC& state,
                           StrBlockKind801C81EC kind) {
    if (kind == StrBlockKind801C81EC::Movie1) {
        state.movie1BlockActive = true;
        return;
    }
    state.clearTailMovieBlockActive = true;
    state.clearTailMoviePlayAndWaitPending = false;
    state.clearTailMoviePlayAndWaitResultKnown = false;
    state.clearTailMoviePlayAndWaitResult = 0;
}

void CompleteStrBlock801C81EC(State801C81EC& state,
                              StrBlockKind801C81EC kind,
                              int32_t result) {
    if (kind == StrBlockKind801C81EC::Movie1) {
        state.movie1BlockActive = false;
        state.movie1PathResolved = false;
        state.movie1Path.clear();
        SetMovie1Result801C81EC(state, result);
        return;
    }
    state.clearTailMovieBlockActive = false;
    state.clearTailMoviePlayAndWaitPending = false;
    ClearClearTailMovieVisualActive801C81EC(state);
    state.clearTailMoviePlayAndWaitResultKnown = true;
    state.clearTailMoviePlayAndWaitResult = result;
}

void SetMovie1Result801C81EC(State801C81EC& state, int32_t result) {
    state.movie1ResultKnown = true;
    state.movie1Result = result;
}

void SetStageResult801C7A60(State801C81EC& state, int32_t result) {
    state.stageResultKnown = true;
    state.stageResult801C7A60 = result;
}

void SetAbortPollBlockActive801C81EC(State801C81EC& state, bool active) {
    state.abortPollBlockActive = active;
}

void SetAbortPollResult26B94(State801C81EC& state, int32_t result) {
    state.abortPollResultKnown = true;
    state.abortPollResult26B94 = result;
}

void SetStageStatus166AC(State801C81EC& state, int32_t status) {
    state.stageStatus166ACKnown = true;
    state.stageStatus166AC = status;
}

void StartClearTailMovieBlock801C81EC(State801C81EC& state,
                                      bool pathResolved,
                                      const std::filesystem::path& path) {
    state.clearTailMoviePathResolved = pathResolved;
    state.clearTailMoviePath = path;
    StartStrBlock801C81EC(state, StrBlockKind801C81EC::ClearTailMovie);
    // Route pre-movie transition frames through Movie1 before STR playback starts.
    state.clearTailMovieVisualActive = true;
    state.clearTailMoviePlayAndWaitPending = true;
}

void CompleteClearTailMovieBlock801C81EC(State801C81EC& state) {
    CompleteStrBlock801C81EC(
        state,
        StrBlockKind801C81EC::ClearTailMovie,
        0);
}

void SetClearTailMovieVisualActive801C81EC(State801C81EC& state, bool active) {
    state.clearTailMovieVisualActive = active;
}

bool IsClearTailMovieVisualActive801C81EC(const State801C81EC& state) {
    return state.clearTailMovieVisualActive;
}

bool IsInitialMovie1PreTransition201ACPending801C81EC(
    const State801C81EC& state) {
    if (state.pendingActions.empty()) {
        return false;
    }

    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    using DirectStrBlockKind = PrStage1LifecycleDirect::StrBlockKind801C81EC;
    const PrStage1LifecycleDirect::Action801C81EC& action =
        state.pendingActions.front();
    return action.kind == DirectActionKind::Transition201AC &&
           action.strBlockKind == DirectStrBlockKind::InitialMovie1 &&
           HasDirectTransitionPayload801C81EC(action) &&
           action.transitionModeA2 == 6 &&
           action.transitionPreFfd4ArgA3 == 2 &&
           action.transitionPostFfd4ArgA4 == 1;
}

bool IsInitialMovie1PostTransition201ACPending801C81EC(
    const State801C81EC& state) {
    if (state.pendingActions.empty()) {
        return false;
    }

    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    using DirectStrBlockKind = PrStage1LifecycleDirect::StrBlockKind801C81EC;
    const PrStage1LifecycleDirect::Action801C81EC& action =
        state.pendingActions.front();
    return action.kind == DirectActionKind::Transition201AC &&
           action.strBlockKind == DirectStrBlockKind::InitialMovie1 &&
           HasDirectTransitionPayload801C81EC(action) &&
           action.transitionModeA2 == 5 &&
           action.transitionPreFfd4ArgA3 == 1 &&
           action.transitionPostFfd4ArgA4 == 2;
}

bool IsClearTailPreMovieTransition201ACPending801C81EC(
    const State801C81EC& state) {
    if (state.pendingActions.empty()) {
        return false;
    }

    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    using DirectStrBlockKind = PrStage1LifecycleDirect::StrBlockKind801C81EC;
    const PrStage1LifecycleDirect::Action801C81EC& action =
        state.pendingActions.front();
    return action.kind == DirectActionKind::Transition201AC &&
           action.strBlockKind == DirectStrBlockKind::ClearTailMovie &&
           HasDirectTransitionPayload801C81EC(action) &&
           action.transitionModeA2 == 6 &&
           action.transitionPreFfd4ArgA3 == 2 &&
           action.transitionPostFfd4ArgA4 == 1;
}

bool IsTransition20110Mode1Pending801C81EC(const State801C81EC& state) {
    if (state.pendingActions.empty()) {
        return false;
    }

    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    const PrStage1LifecycleDirect::Action801C81EC& action =
        state.pendingActions.front();
    return action.kind == DirectActionKind::Transition20110 &&
           HasDirectTransitionPayload801C81EC(action) &&
           action.transitionModeA2 == 1 &&
           action.transitionPreFfd4ArgA3 == 2 &&
           action.transitionPostFfd4ArgA4 == 1;
}

bool IsTransition20110Mode2Pending801C81EC(const State801C81EC& state) {
    if (state.pendingActions.empty()) {
        return false;
    }

    using DirectActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    const PrStage1LifecycleDirect::Action801C81EC& action =
        state.pendingActions.front();
    return action.kind == DirectActionKind::Transition20110 &&
           HasDirectTransitionPayload801C81EC(action) &&
           action.transitionModeA2 == 2 &&
           action.transitionPreFfd4ArgA3 == 1 &&
           action.transitionPostFfd4ArgA4 == 2;
}

void ClearClearTailMovieVisualActive801C81EC(State801C81EC& state) {
    state.clearTailMovieVisualActive = false;
}

void SetBootstrap15590Block801C81EC(State801C81EC& state,
                                    bool active,
                                    bool curtainStarted,
                                    bool pathResolved,
                                    const std::filesystem::path& path) {
    const bool startingNewBlock = active && !state.bootstrap15590Active;
    if (startingNewBlock) {
        ResetBootstrap15590LoaderDirectRuntime801C81EC(state);
    }
    state.bootstrap15590Active = active;
    state.bootstrap15590CurtainStarted = curtainStarted;
    state.bootstrap15590PathResolved = pathResolved;
    state.bootstrap15590Path = path;
    if (!active) {
        ResetBootstrap15590LoaderDirectRuntime801C81EC(state);
    }
}

void ClearBootstrap15590Block801C81EC(State801C81EC& state) {
    state.bootstrap15590Active = false;
    state.bootstrap15590CurtainStarted = false;
    state.bootstrap15590SceneLoaderSlotKnown = false;
    state.bootstrap15590SceneLoaderSlotPresent = false;
    state.bootstrap15590SceneLoaderSlot = 0;
    state.bootstrap15590ZcompoLoader = false;
    ResetBootstrap15590LoaderDirectRuntime801C81EC(state);
}

void SetSaveUi19148StartPending801C81EC(State801C81EC& state, bool pending) {
    state.saveUi19148StartPending = pending;
}

void SetSaveUi19148Active801C81EC(State801C81EC& state, bool active) {
    state.saveUi19148Active = active;
    if (!active) {
        state.saveUi19148LowerFeedbackRequests =
            PrStage1SaveUi19148LowerFeedbackRequestList{};
    }
}

void RecordSaveUi19148LowerFeedbackRequests801C81EC(
    State801C81EC& state,
    const PrStage1SaveUi19148LowerFeedbackRequestList& requests) {
    state.saveUi19148LowerFeedbackRequests = requests;
}

void SetDeferredSceneResult801C81EC(State801C81EC& state, int32_t result) {
    state.deferredSceneResultKnown = true;
    state.deferredSceneResult = result;
}

bool HasDeferredSceneResult801C81EC(const State801C81EC& state) {
    return state.deferredSceneResultKnown;
}

bool PopDeferredSceneResult801C81EC(State801C81EC& state, int32_t& out) {
    if (!state.deferredSceneResultKnown) {
        return false;
    }
    out = state.deferredSceneResult;
    ClearDeferredSceneResult801C81EC(state);
    return true;
}

bool PopReadyDeferredSceneResult801C81EC(State801C81EC& state, int32_t& out) {
    if (HasBlockingLifecycleWork801C81EC(state)) {
        return false;
    }
    return PopDeferredSceneResult801C81EC(state, out);
}

LifecycleStatusWrites801C81EC BuildStatusWrites801C81EC(
    const PrStage1LifecycleDirect::StepResult801C81EC& step) {
    LifecycleStatusWrites801C81EC out{};
    out.write800916D0 = step.write800916D0;
    out.word800916D0 = step.word800916D0;
    out.write800916DA = step.write800916DA;
    out.word800916DA = step.word800916DA;
    out.write800916E0 = step.write800916E0;
    out.word800916E0 = step.word800916E0;
    return out;
}

bool HasStatusWrites801C81EC(const LifecycleStatusWrites801C81EC& writes) {
    return writes.write800916D0 ||
           writes.write800916DA ||
           writes.write800916E0;
}

void SetPendingStatusWrites801C81EC(
    State801C81EC& state,
    const LifecycleStatusWrites801C81EC& writes) {
    if (!HasStatusWrites801C81EC(writes)) {
        return;
    }
    state.pendingStatusWritesKnown = true;
    state.pendingStatusWrites = writes;
}

bool PopReadyPendingStatusWrites801C81EC(
    State801C81EC& state,
    LifecycleStatusWrites801C81EC& out) {
    if (!state.pendingStatusWritesKnown ||
        HasBlockingLifecycleWork801C81EC(state)) {
        return false;
    }
    out = state.pendingStatusWrites;
    state.pendingStatusWritesKnown = false;
    state.pendingStatusWrites = LifecycleStatusWrites801C81EC{};
    return true;
}

SceneResultDispatch801C81EC ResolveStepSceneResult801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::StepResult801C81EC& step,
    bool waitingForHostBlock) {
    SceneResultDispatch801C81EC out{};
    if (!step.sceneResultKnown) {
        return out;
    }

    out.sceneResultKnown = true;
    out.sceneResult = step.sceneResult;
    if (waitingForHostBlock || HasBlockingLifecycleWork801C81EC(state)) {
        SetDeferredSceneResult801C81EC(state, step.sceneResult);
        out.deferred = true;
    }
    return out;
}

void ClearDeferredSceneResult801C81EC(State801C81EC& state) {
    state.deferredSceneResultKnown = false;
    state.deferredSceneResult = 0;
}

void RecordTransitionActionState801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    if (!IsTransitionAction801C81EC(action) ||
        !HasDirectTransitionPayload801C81EC(action)) {
        return;
    }
    state.transitionActionStateKnown = true;
    state.transitionActionState = action;
}

void RecordBootstrap15590ActionState801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    const Bootstrap15590DirectPlan801C81EC plan =
        BuildBootstrap15590DirectPlan801C81EC(action);
    if (!plan.valid) {
        return;
    }
    state.bootstrap15590DirectPlanKnown = true;
    state.bootstrap15590DirectPlan = plan;
}

void RecordLifecycleActionState801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    RecordTransitionActionState801C81EC(state, action);
    RecordBootstrap15590ActionState801C81EC(state, action);
}

bool TryGetTransitionActionState801C81EC(
    const State801C81EC& state,
    PrStage1LifecycleDirect::Action801C81EC& out) {
    if (!state.transitionActionStateKnown) {
        return false;
    }
    out = state.transitionActionState;
    return true;
}

void ClearTransitionActionState801C81EC(State801C81EC& state) {
    state.transitionActionStateKnown = false;
    state.transitionActionState = PrStage1LifecycleDirect::Action801C81EC{};
}

} // namespace PrStage1LifecycleExecutorDirect
