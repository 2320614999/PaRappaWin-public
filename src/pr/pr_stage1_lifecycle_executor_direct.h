#pragma once

#include "pr_stage1_lifecycle_direct.h"
#include "pr_stage1_loader_bootstrap_plan_direct.h"
#include "pr_stage1_loader_cd_hal.h"
#include "pr_stage1_loader_producer_adapter.h"
#include "pr_stage1_loader_spu_hal.h"
#include "pr_stage1_movie_segment_direct.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_stage_status_bank_direct.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace PrStage1LoaderDirect {
struct RunnerState;
struct ActionFeedback;
struct CdSeamResult;
struct SpuSeamResult;
}

namespace PrMovieSegmentDirect {
struct StreamClockProducerCarrier800493F4;
}

namespace PrStage1LowerCdProducerDirect {
struct LowerCdProducerFacts;
}

namespace PrStage1LoaderProducerAdapter {
struct ProducerRuntime;
struct MemoryRuntimeStepResult;
struct LiveHalFeedbackBuildResult;
struct GpuTimBytesLiveInput;
struct CdLookupLiveInput;
struct RecordDispatchLiveInput;
struct SpuHalResultLiveInput;
}

struct PrStage1XaCdDirectState;
struct PrStage1XaCdDirectStreamClockProbe800493F4;

namespace PrStage1LifecycleExecutorDirect {

struct LoaderOwnerState801C81EC;

struct FrameHostInput801C81EC {
    uint8_t sceneId = 1;
    uint16_t word800916D0 = 0;
    uint16_t word800916DA = 0;
    bool word800916F0Known = false;
    uint16_t word800916F0 = 0;
    uint16_t word80091816 = 0;
    uint32_t dword801D3040 = 0;
    bool byte801C368E = false;
};

struct StatusBankDirectMemoryFeedback801C81EC {
    bool requestValid = false;
    PrStageStatusBankActionKind requestKind = PrStageStatusBankActionKind::None;
    uint32_t psxFunction = 0;
    bool psxFunctionMatched = false;
    bool requestHandled = false;
    bool status166ACKnown = false;
    int32_t status166AC = 0;
    int32_t status166ACSceneId = 0;
    bool status166ACMapped = false;
    int32_t status166ACSlotIndex = -1;
    bool status166ACStatusBankKnown = false;
    bool status166ACHelperGap = false;
    bool payloadKnown = false;
    bool payloadOk = false;
    bool payloadHelperGap = false;
    int32_t payloadResult = 0;
    uint32_t payloadLastFaultAddress = 0;
    bool payloadPrefixKnown80092F10 = false;
    bool payloadPrefixStatusBankKnown80092F1D = false;
    uint32_t payloadLastWriterFunction = 0;
    bool payloadWrote8001635C = false;
};

enum class StatusBankDirectMemoryGapKind801C81EC : uint8_t {
    None,
    QueryStageStatus166AC,
    SaveStatus1635C,
    UnlockNextStage1628C,
};

struct StatusBankDirectMemoryGap801C81EC {
    bool shouldLog = false;
    StatusBankDirectMemoryGapKind801C81EC kind =
        StatusBankDirectMemoryGapKind801C81EC::None;
    bool requestValid = false;
    uint32_t psxFunction = 0;
    bool psxFunctionMatched = false;
    int32_t sceneId = 0;
    int32_t arg0 = 0;
    int32_t arg1 = 0;
    bool status166ACMapped = false;
    int32_t status166ACSlotIndex = -1;
    bool status166ACStatusBankKnown = false;
    bool helperGap = false;
    uint32_t lastFaultAddress = 0;
    bool payloadKnown = false;
    bool payloadPrefixKnown80092F10 = false;
    bool payloadPrefixStatusBankKnown80092F1D = false;
    uint32_t payloadLastWriterFunction = 0;
    bool payloadWrote8001635C = false;
};

struct SaveStatus1635CReplayBackupHostRequest801C81EC {
    bool valid = false;
    uint32_t psxOrder = 0;
    bool psxFunctionKnown = false;
    uint32_t psxFunction = 0;
    bool prevGrade92F40Known = false;
    uint32_t prevGrade92F40 = 0;
};

struct SaveStatus1635CReplayBackupHostFeedback801C81EC {
    bool requestValid = false;
    bool psxFunctionKnown = false;
    uint32_t psxFunction = 0;
    bool psxFunctionMatched = false;
    bool prevGrade92F40Known = false;
    bool hostCallbackKnown = false;
    bool replayMirrorKnown8008EEF8 = false;
    bool replayMirrorProducerKnown8008EEF8 = false;
    uint32_t replayMirrorProducerFunction = 0;
    bool replayMirrorByteCountKnown8008EEF8 = false;
    uint32_t replayMirrorKnownByteCount8008EEF8 = 0;
    uint32_t replayPublishedCount901BC = 0;
    uint32_t replayWriteCount901C0 = 0;
    bool captureAttempted = false;
    bool backupValid = false;
    bool backupPrevGrade92F40Known = false;
    uint32_t backupPrevGrade92F40 = 0;
    uint32_t backupPublishedCount901BC = 0;
    bool requestHandled = false;
};

struct SaveStatus1635CReplayBackupHostGap801C81EC {
    bool shouldLog = false;
    uint32_t psxOrder = 0;
    bool requestValid = false;
    bool psxFunctionKnown = false;
    uint32_t psxFunction = 0;
    bool psxFunctionMatched = false;
    bool prevGrade92F40Known = false;
    bool hostCallbackKnown = false;
    bool replayMirrorKnown8008EEF8 = false;
    bool replayMirrorProducerKnown8008EEF8 = false;
    uint32_t replayMirrorProducerFunction = 0;
    bool replayMirrorByteCountKnown8008EEF8 = false;
    uint32_t replayMirrorKnownByteCount8008EEF8 = 0;
    uint32_t replayPublishedCount901BC = 0;
    uint32_t replayWriteCount901C0 = 0;
    bool captureAttempted = false;
    bool backupValid = false;
    bool backupPrevGrade92F40Known = false;
    uint32_t backupPrevGrade92F40 = 0;
    uint32_t backupPublishedCount901BC = 0;
    bool requestHandled = false;
};

using LoaderReset25A34DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderReset25A34DirectPlan801C81EC;
using LoaderOpenFile1A324DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderOpenFile1A324DirectPlan801C81EC;
using LoaderSeek1A89CDirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderSeek1A89CDirectPlan801C81EC;
using LoaderRead1A818DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderRead1A818DirectPlan801C81EC;
using LoaderTempAlloc25B28DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderTempAlloc25B28DirectPlan801C81EC;
using LoaderStackAlloc25A70DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderStackAlloc25A70DirectPlan801C81EC;
using LoaderStackFree25AF8DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderStackFree25AF8DirectPlan801C81EC;
using LoaderStackFind25BFCDirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderStackFind25BFCDirectPlan801C81EC;
using LoaderStackSplit25BBCDirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderStackSplit25BBCDirectPlan801C81EC;
using LoaderTimUpload1AE7CDirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderTimUpload1AE7CDirectPlan801C81EC;
using LoaderSpuVabDirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderSpuVabDirectPlan801C81EC;
using LoaderRetrySfxDirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderRetrySfxDirectPlan801C81EC;
using LoaderPadStartComDirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderPadStartComDirectPlan801C81EC;
using LoaderParse1A8F0DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderParse1A8F0DirectPlan801C81EC;
using LoaderDraw1EA74DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderDraw1EA74DirectPlan801C81EC;
using LoaderPattern1EF40DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::LoaderPattern1EF40DirectPlan801C81EC;
using Bootstrap15590DirectPlan801C81EC =
    PrStage1LoaderBootstrapPlanDirect::Bootstrap15590DirectPlan801C81EC;

struct Bootstrap15590CdLookupLowerProducerRuntime801C81EC {
    bool keyKnown = false;
    uint32_t sceneIndex = 0;
    uint32_t psxOrder = 0;
    uint32_t probeFunction = 0;
    uint32_t lookupFunction = 0;
    uint32_t cdlFilePtr = 0;
    uint32_t pathPtr = 0;
    const char* psxPath = nullptr;
    PrStage1LoaderCdHal::Probe8001A2B0State probe{};
    bool lastLookupFeedbackKnown = false;
    PrStage1LoaderCdHal::LookupFeedback800381F8 lastLookupFeedback{};
};

enum class Bootstrap15590CdLowerAttemptStatus801C81EC : uint8_t {
    None,
    NotApplicable,
    RequestReady,
    RequestAlreadyPending,
    FeedbackApplied,
    FeedbackRejected,
};

enum class Bootstrap15590CdLowerRejectReason801C81EC : uint8_t {
    None,
    NoPendingRequest,
    RequestInvalid,
    LookupRequest,
    ActionKindMismatch,
    MetadataMismatch,
    AttemptProvenanceMissing,
    AttemptProvenanceMismatch,
    RequestNotReady,
    FinalReadyRequirementMissing,
    BridgeIncomplete,
    XaCdRejected,
    AdapterNotReady,
    ApplyRejected,
};

struct Bootstrap15590CdLowerProducerRuntime801C81EC {
    bool keyKnown = false;
    bool requestPending = false;
    Bootstrap15590CdLowerAttemptStatus801C81EC status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::None;
    uint32_t psxOrder = 0;
    PrStage1LoaderDirect::ProducerStepKind stepKind =
        PrStage1LoaderDirect::ProducerStepKind::None;
    PrStage1LoaderCdHal::ActionKind cdActionKind =
        PrStage1LoaderCdHal::ActionKind::None;
    uint16_t recordIndex = 0;
    PrStage1LoaderDirect::LoaderRecordType recordType =
        PrStage1LoaderDirect::LoaderRecordType::Unknown;
    uint8_t attemptIndex = 0;
    uint32_t lowerFunction = 0;
    uint32_t finalFunction = 0;
    PrStage1LoaderCdHal::LowerActionRequestMetadata lowerRequest{};
    bool payloadBytesRequired = false;
    bool seekLbaKnown = false;
    int32_t seekLba = 0;
    bool readDstPtrKnown = false;
    uint32_t readDstPtr = 0;
    bool readSectorCountKnown = false;
    int32_t readSectorCount = 0;
    bool readStartHalProgressAccepted = false;
    uint32_t readStartHalProgressReadS27Serial = 0;
    bool finalReadyHalFactsRequired = false;
    uint32_t finalReadyHalFactsReadS27Serial = 0;
    bool cdSyncLoopFactsRequired80037070 = false;
    uint32_t cdSyncLoopFunction80037070 = 0;
    bool cdSyncLoopA0WaitModeKnown80037070 = false;
    int32_t cdSyncLoopA0WaitMode80037070 = 0;
    bool lastRejectKnown = false;
    Bootstrap15590CdLowerRejectReason801C81EC lastRejectReason =
        Bootstrap15590CdLowerRejectReason801C81EC::None;
    Bootstrap15590CdLowerAttemptStatus801C81EC lastRejectStatus =
        Bootstrap15590CdLowerAttemptStatus801C81EC::None;
    PrStage1LoaderCdHal::LowerActionRequestMetadata lastRejectExpected{};
    PrStage1LoaderCdHal::LowerActionRequestMetadata lastRejectActual{};
};

struct Bootstrap15590CdLowerObservableState801C81EC {
    bool keyKnown = false;
    bool requestPending = false;
    Bootstrap15590CdLowerAttemptStatus801C81EC status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::None;
    uint32_t psxOrder = 0;
    PrStage1LoaderDirect::ProducerStepKind stepKind =
        PrStage1LoaderDirect::ProducerStepKind::None;
    PrStage1LoaderCdHal::ActionKind cdActionKind =
        PrStage1LoaderCdHal::ActionKind::None;
    uint16_t recordIndex = 0;
    PrStage1LoaderDirect::LoaderRecordType recordType =
        PrStage1LoaderDirect::LoaderRecordType::Unknown;
    uint8_t attemptIndex = 0;
    uint32_t lowerFunction = 0;
    uint32_t finalFunction = 0;
    PrStage1LoaderCdHal::LowerActionRequestMetadata lowerRequest{};
    bool payloadBytesRequired = false;
    bool seekLbaKnown = false;
    int32_t seekLba = 0;
    bool readDstPtrKnown = false;
    uint32_t readDstPtr = 0;
    bool readSectorCountKnown = false;
    int32_t readSectorCount = 0;
    bool readStartHalProgressAccepted = false;
    uint32_t readStartHalProgressReadS27Serial = 0;
    bool finalReadyHalFactsRequired = false;
    uint32_t finalReadyHalFactsReadS27Serial = 0;
    bool cdSyncLoopFactsRequired80037070 = false;
    uint32_t cdSyncLoopFunction80037070 = 0;
    bool cdSyncLoopA0WaitModeKnown80037070 = false;
    int32_t cdSyncLoopA0WaitMode80037070 = 0;
    bool lastRejectKnown = false;
    Bootstrap15590CdLowerRejectReason801C81EC lastRejectReason =
        Bootstrap15590CdLowerRejectReason801C81EC::None;
    Bootstrap15590CdLowerAttemptStatus801C81EC lastRejectStatus =
        Bootstrap15590CdLowerAttemptStatus801C81EC::None;
    PrStage1LoaderCdHal::LowerActionRequestMetadata lastRejectExpected{};
    PrStage1LoaderCdHal::LowerActionRequestMetadata lastRejectActual{};
};

struct LifecycleStatusWrites801C81EC {
    bool write800916D0 = false;
    uint16_t word800916D0 = 0;
    bool write800916DA = false;
    uint16_t word800916DA = 0;
    bool write800916E0 = false;
    uint16_t word800916E0 = 0;
};

struct State801C81EC {
    State801C81EC();
    ~State801C81EC();
    State801C81EC(State801C81EC&& other) noexcept;
    State801C81EC& operator=(State801C81EC&& other) noexcept;
    State801C81EC(const State801C81EC&) = delete;
    State801C81EC& operator=(const State801C81EC&) = delete;

    std::unique_ptr<LoaderOwnerState801C81EC> loaderOwner;

    bool movie1BlockActive = false;
    bool movie1PathResolved = false;
    std::filesystem::path movie1Path;
    bool movie1ResultKnown = false;
    int32_t movie1Result = 0;

    bool stageResultKnown = false;
    int32_t stageResult801C7A60 = 0;

    bool abortPollBlockActive = false;
    bool abortPollResultKnown = false;
    int32_t abortPollResult26B94 = 0;

    bool stageStatus166ACKnown = false;
    int32_t stageStatus166AC = 0;

    bool clearTailMovieBlockActive = false;
    bool clearTailMoviePlayAndWaitPending = false;
    bool clearTailMoviePathResolved = false;
    std::filesystem::path clearTailMoviePath;
    bool clearTailMoviePlayAndWaitResultKnown = false;
    int32_t clearTailMoviePlayAndWaitResult = 0;
    bool clearTailMovieVisualActive = false;

    bool bootstrap15590Active = false;
    bool bootstrap15590CurtainStarted = false;
    bool bootstrap15590PathResolved = false;
    bool bootstrap15590SceneLoaderSlotKnown = false;
    bool bootstrap15590SceneLoaderSlotPresent = false;
    uint8_t bootstrap15590SceneLoaderSlot = 0;
    bool bootstrap15590ZcompoLoader = false;
    std::filesystem::path bootstrap15590Path;
    bool bootstrap15590DirectPlanKnown = false;
    Bootstrap15590DirectPlan801C81EC bootstrap15590DirectPlan;
    bool bootstrap15590LoaderDirectBeginAttempted = false;
    bool bootstrap15590LoaderDirectBegun = false;
    bool bootstrap15590LoaderDirectBeginSucceeded = false;
    bool bootstrap15590LoaderDirectBeginFailed = false;
    bool bootstrap15590LoaderDirectPumpStarted = false;
    bool bootstrap15590LoaderDirectWaitingExternal = false;
    bool bootstrap15590LoaderDirectCompleted = false;
    bool bootstrap15590LoaderDirectFailed = false;
    std::size_t bootstrap15590LoaderDirectLastPumpActionCount = 0;
    PrStage1LoaderSpuHal::State bootstrap15590LoaderSpuHalState;
    PrStage1MovieSegmentDirect::Stage1MovieSegmentCdLookupRuntime801C4780
        bootstrap15590MovieSegmentCdLookupRuntime{};
    Bootstrap15590CdLookupLowerProducerRuntime801C81EC
        bootstrap15590CdLookupLowerProducerRuntime{};
    Bootstrap15590CdLowerProducerRuntime801C81EC
        bootstrap15590CdLowerProducerRuntime{};
    std::vector<uint8_t> bootstrap15590CdLowerLivePayloadBytes{};
    bool bootstrap15590CdLowerFileBaseLbaKnown = false;
    int32_t bootstrap15590CdLowerFileBaseLba = 0;
    bool bootstrap15590CdLowerLastSeekLbaKnown = false;
    int32_t bootstrap15590CdLowerLastSeekLba = 0;

    bool saveUi19148StartPending = false;
    bool saveUi19148Active = false;
    bool saveUi19148Seed80092F10Known = false;
    uint32_t saveUi19148Seed80092F10Address = 0;
    PrStage1SaveUi19148LowerFeedbackRequestList
        saveUi19148LowerFeedbackRequests{};

    bool deferredSceneResultKnown = false;
    int32_t deferredSceneResult = 0;

    bool pendingStatusWritesKnown = false;
    LifecycleStatusWrites801C81EC pendingStatusWrites{};

    bool transitionActionStateKnown = false;
    PrStage1LifecycleDirect::Action801C81EC transitionActionState;

    std::vector<PrStage1LifecycleDirect::Action801C81EC> pendingActions;
};

struct ActionApplyResult801C81EC {
    bool producedImmediateInput = false;
    bool waitingForHostBlock = false;
    bool retryBlockedAction = false;
};

enum class HostBlockKind801C81EC : uint8_t {
    None,
    Movie1,
    ClearTailMovie,
    AbortPoll,
    Bootstrap15590,
    SaveUi19148,
};

struct HostBlockSnapshot801C81EC {
    HostBlockKind801C81EC kind = HostBlockKind801C81EC::None;
    bool active = false;
    bool startPending = false;
    bool waitingForPendingActions = false;
    bool pathResolved = false;
    bool curtainStarted = false;
    std::filesystem::path path;
    bool sceneLoaderSlotKnown = false;
    bool sceneLoaderSlotPresent = false;
    uint8_t sceneLoaderSlot = 0;
    bool bootstrapZcompoLoader = false;
    bool bootstrap15590DirectPlanKnown = false;
    Bootstrap15590DirectPlan801C81EC bootstrap15590DirectPlan;
    bool bootstrap15590LoaderDirectBeginAttempted = false;
    bool bootstrap15590LoaderDirectBegun = false;
    bool bootstrap15590LoaderDirectBeginSucceeded = false;
    bool bootstrap15590LoaderDirectBeginFailed = false;
    bool bootstrap15590LoaderDirectPumpStarted = false;
    bool bootstrap15590LoaderDirectWaitingExternal = false;
    bool bootstrap15590LoaderDirectCompleted = false;
    bool bootstrap15590LoaderDirectFailed = false;
    std::size_t bootstrap15590LoaderDirectActionCount = 0;
    std::size_t bootstrap15590LoaderDirectNextActionIndex = 0;
    std::size_t bootstrap15590LoaderDirectLastPumpActionCount = 0;
    bool bootstrap15590LoaderDirectWaitingForFeedback = false;
    bool bootstrap15590CdLowerRequestPending = false;
    Bootstrap15590CdLowerObservableState801C81EC
        bootstrap15590CdLower{};
    PrStage1SaveUi19148LowerFeedbackRequestList
        saveUi19148LowerFeedbackRequests{};
    bool saveUi19148Seed80092F10Known = false;
    uint32_t saveUi19148Seed80092F10Address = 0;
};

struct PendingActionDrainPlan801C81EC {
    bool shouldPopAction = false;
};

struct HostBlockFeedback801C81EC {
    HostBlockKind801C81EC kind = HostBlockKind801C81EC::None;
    bool completed = false;
    int32_t result = 0;
    bool startDeferred = false;
    bool active = false;
    bool curtainStarted = false;
    bool pathResolved = false;
    std::filesystem::path path;
    bool sceneLoaderSlotKnown = false;
    bool sceneLoaderSlotPresent = false;
    uint8_t sceneLoaderSlot = 0;
    bool bootstrapZcompoLoader = false;
    bool bootstrap15590DirectPlanKnown = false;
    Bootstrap15590DirectPlan801C81EC bootstrap15590DirectPlan;
};

struct HostBlockStart801C81EC {
    HostBlockKind801C81EC kind = HostBlockKind801C81EC::None;
    bool active = true;
    bool startDeferred = false;
    bool curtainStarted = false;
    bool pathResolved = false;
    std::filesystem::path path;
    bool sceneLoaderSlotKnown = false;
    bool sceneLoaderSlotPresent = false;
    uint8_t sceneLoaderSlot = 0;
    bool bootstrapZcompoLoader = false;
    bool bootstrap15590DirectPlanKnown = false;
    Bootstrap15590DirectPlan801C81EC bootstrap15590DirectPlan;
    bool saveUi19148Seed80092F10Known = false;
    uint32_t saveUi19148Seed80092F10Address = 0;
};

struct Bootstrap15590HostStartFacts801C81EC {
    bool curtainStarted = false;
};

struct Bootstrap15590HostStartInput801C81EC {
    HostBlockSnapshot801C81EC block{};
    PrStage1LifecycleDirect::Action801C81EC action{};
    bool curtainStartAttempted = false;
    bool curtainStarted = false;
    bool pathResolveAttempted = false;
    bool pathResolved = false;
    std::filesystem::path path;
};

struct Bootstrap15590HostStartPlan801C81EC {
    bool alreadyActive = false;
    bool shouldStartCurtain = false;
    bool shouldResolvePath = false;
    bool shouldBeginHostBlock = false;
    int16_t sceneExitReason = 3;
    HostBlockStart801C81EC start{};
};

struct AbortPollHostTickInput801C81EC {
    HostBlockSnapshot801C81EC block{};
    bool dispatcherRunning = false;
    int32_t dispatcherResult = -1;
    bool dispatcherStartAttempted = false;
    bool dispatcherStartSucceeded = false;
};

struct AbortPollHostTickPlan801C81EC {
    bool active = false;
    bool waiting = false;
    bool shouldStartDispatcher = false;
    bool shouldComplete = false;
    int32_t result = 0;
};

struct ClearTailMovieHostTickInput801C81EC {
    HostBlockSnapshot801C81EC block{};
    bool strPlayerReady = false;
    bool movieFileExists = false;
    bool strPlayerPlaying = false;
    bool visualActive = false;
    bool debugSkipRequested = false;
};

struct ClearTailMovieHostTickPlan801C81EC {
    bool active = false;
    bool waitingForPendingActions = false;
    bool shouldPlay = false;
    bool shouldUpdate = false;
    bool skipAllowed = false;
    bool pathResolved = false;
    std::filesystem::path path;
};

struct SaveUi19148HostStartInput801C81EC {
    HostBlockSnapshot801C81EC block{};
    PrStage1LifecycleDirect::Action801C81EC action{};
    bool startAttempted = false;
    bool startSucceeded = false;
};

struct SaveUi19148HostStartPlan801C81EC {
    bool alreadyActive = false;
    bool actionValid = false;
    bool seed80092F10Known = false;
    uint32_t seed80092F10Address = 0;
    bool shouldStart = false;
    bool shouldBeginHostBlock = false;
    bool started = false;
};

struct SaveUi19148HostTickInput801C81EC {
    HostBlockSnapshot801C81EC block{};
    bool tickAttempted = false;
    bool done = false;
    int32_t saveResult = 0;
};

struct SaveUi19148HostTickPlan801C81EC {
    bool active = false;
    bool shouldStart = false;
    bool shouldTick = false;
    bool shouldComplete = false;
    int32_t saveResult = 0;
};

enum class TransitionStartPreset801C81EC : uint8_t {
    None = 0,
    ClearTailResultMovieMode6,
    ClearTailResultMovieMode5,
    StageLoopEntry20110Mode1,
    ClearTailPrelude20110Mode2,
};

struct TransitionStartInput801C81EC {
    bool transitionActive = false;
    PrStage1LifecycleDirect::Action801C81EC action{};
};

struct TransitionStartPlan801C81EC {
    bool activeAlready = false;
    bool validPayload = false;
    bool shouldStart = false;
    int targetScene = -1;
    TransitionStartPreset801C81EC preset =
        TransitionStartPreset801C81EC::None;
    uint32_t psxFunction = 0;
    uint32_t finishFunction = 0;
    int32_t modeA2 = 0;
    int32_t preFfd4ArgA3 = 0;
    int32_t postFfd4ArgA4 = 0;
    bool hasExtraDelay27194 = false;
    int32_t extraDelayFrames27194 = 0;
};

HostBlockStart801C81EC BuildAbortPollHostBlockStart801C81EC();
HostBlockFeedback801C81EC BuildAbortPollCompletedHostBlockFeedback801C81EC(
    int32_t result);
AbortPollHostTickPlan801C81EC BuildAbortPollHostTickPlan801C81EC(
    const AbortPollHostTickInput801C81EC& input);
HostBlockStart801C81EC BuildClearTailMovieHostBlockStart801C81EC(
    bool pathResolved,
    const std::filesystem::path& path);
HostBlockFeedback801C81EC BuildClearTailMovieCompletedHostBlockFeedback801C81EC(
    int32_t result);
ClearTailMovieHostTickPlan801C81EC BuildClearTailMovieHostTickPlan801C81EC(
    const ClearTailMovieHostTickInput801C81EC& input);
HostBlockStart801C81EC BuildSaveUi19148HostBlockStart801C81EC(
    bool started,
    uint32_t seed80092F10Address);
HostBlockFeedback801C81EC BuildSaveUi19148CompletedHostBlockFeedback801C81EC(
    int32_t saveResult);
SaveUi19148HostStartPlan801C81EC BuildSaveUi19148HostStartPlan801C81EC(
    const SaveUi19148HostStartInput801C81EC& input);
SaveUi19148HostTickPlan801C81EC BuildSaveUi19148HostTickPlan801C81EC(
    const SaveUi19148HostTickInput801C81EC& input);
TransitionStartPlan801C81EC BuildTransitionStartPlan801C81EC(
    const TransitionStartInput801C81EC& input);
HostBlockStart801C81EC BuildMovie1HostBlockStart801C81EC(
    bool pathResolved,
    const std::filesystem::path& path);
HostBlockFeedback801C81EC BuildMovie1CompletedHostBlockFeedback801C81EC(
    int32_t result);

enum class StrBlockKind801C81EC : uint8_t {
    Movie1,
    ClearTailMovie,
};

struct SceneResultDispatch801C81EC {
    bool sceneResultKnown = false;
    bool deferred = false;
    int32_t sceneResult = 0;
};

struct Bootstrap15590LoaderBeginResult801C81EC {
    bool active = false;
    bool planKnown = false;
    bool attempted = false;
    bool alreadyBegun = false;
    bool begun = false;
    bool beginSucceeded = false;
    bool beginFailed = false;
    std::size_t actionCount = 0;
    std::size_t nextActionIndex = 0;
};

struct Bootstrap15590LoaderPumpResult801C81EC {
    bool active = false;
    bool planKnown = false;
    bool beginAttempted = false;
    bool runnerBegun = false;
    bool runnerTerminal = false;
    bool completed = false;
    bool failed = false;
    bool poppedAction = false;
    std::size_t poppedActionCount = 0;
    bool waitingForFeedback = false;
    bool waitingStepKnown = false;
    bool memoryPumpAttempted = false;
    bool memoryPumpProduced = false;
    bool memoryPumpApplied = false;
    bool gpuLivePumpAttempted = false;
    bool gpuLivePumpProduced = false;
    bool gpuLivePumpApplied = false;
    bool gpuLivePumpIncomplete = false;
    bool localHalPumpAttempted = false;
    bool localHalPumpProduced = false;
    bool localHalPumpApplied = false;
    bool externalProducerRequired = false;
    bool helperGap = false;
    bool reachedStepLimit = false;
};

struct Bootstrap15590CallbackHostRequest801C81EC {
    bool valid = false;
    bool waitingForCallback = false;
    bool transitionUpdateRequired = false;
    bool loadingCurtainCallbackRequired = false;
};

struct Bootstrap15590CallbackHostInput801C81EC {
    bool transitionUpdateAttempted = false;
    int32_t transitionUpdateResult = 0;
    bool loadingCurtainCallbackKnown = false;
    bool loadingCurtainCallbackFired = false;
};

struct Bootstrap15590CdLookupHostRequest801C81EC {
    bool valid = false;
    bool waitingForCdLookup = false;
    bool binNeeded = false;
    bool probeCompleteWithoutHal = false;
    uint32_t psxOrder = 0;
    uint32_t probeFunction = 0;
    uint32_t lookupFunction = 0;
    uint8_t retryIndex = 0;
    uint32_t cdlFilePtr = 0;
    uint32_t pathPtr = 0;
    bool psxPathKnown = false;
    const char* psxPath = nullptr;
};

struct Bootstrap15590CdLowerHostRequest801C81EC {
    bool valid = false;
    bool waitingForCd = false;
    Bootstrap15590CdLowerAttemptStatus801C81EC status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::None;
    PrStage1LoaderDirect::ProducerStepKind stepKind =
        PrStage1LoaderDirect::ProducerStepKind::None;
    PrStage1LoaderCdHal::ActionKind cdActionKind =
        PrStage1LoaderCdHal::ActionKind::None;
    uint32_t psxOrder = 0;
    uint16_t recordIndex = 0;
    PrStage1LoaderDirect::LoaderRecordType recordType =
        PrStage1LoaderDirect::LoaderRecordType::Unknown;
    uint8_t attemptIndex = 0;
    PrStage1LoaderCdHal::LowerActionRequestMetadata lowerRequest{};

    bool psxPathKnown = false;
    const char* psxPath = nullptr;
    bool relativeWinPathKnown = false;
    const char* relativeWinPath = nullptr;

    bool seekLbaKnown = false;
    int32_t seekLba = 0;
    bool readDstPtrKnown = false;
    uint32_t readDstPtr = 0;
    bool readSectorCountKnown = false;
    int32_t readSectorCount = 0;
    bool readStartHalProgressAccepted = false;
    uint32_t readStartHalProgressReadS27Serial = 0;
    bool payloadBytesRequired = false;
    bool cdSyncLoopFactsRequired80037070 = false;
    uint32_t cdSyncLoopFunction80037070 = 0;
    bool cdSyncLoopA0WaitModeKnown80037070 = false;
    int32_t cdSyncLoopA0WaitMode80037070 = 0;
};

struct Bootstrap15590CdLowerFactsApplyResult801C81EC {
    bool attempted = false;
    bool requestMatched = false;
    bool requestPendingBefore = false;
    bool requestPendingAfter = false;
    bool xaCdAccepted = false;
    bool feedbackReady = false;
    bool applied = false;
    Bootstrap15590CdLowerRejectReason801C81EC rejectReason =
        Bootstrap15590CdLowerRejectReason801C81EC::None;
    Bootstrap15590CdLowerAttemptStatus801C81EC status =
        Bootstrap15590CdLowerAttemptStatus801C81EC::None;
    PrStage1LoaderCdHal::LowerActionRequestMetadata expectedLowerRequest{};
    PrStage1LoaderCdHal::LowerActionRequestMetadata actualLowerRequest{};
    PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult build{};
};

struct Bootstrap15590PumpHostBlockResult801C81EC {
    ActionApplyResult801C81EC action{};
    HostBlockFeedback801C81EC feedback{};
    bool completed = false;
    bool failed = false;
    bool waiting = false;
    bool shouldApplyCompletedFeedback = false;
    bool waitingForFeedback = false;
    bool externalProducerRequired = false;
    bool helperGap = false;
    bool reachedStepLimit = false;
    std::size_t poppedActionCount = 0;
};

struct Bootstrap15590HostTickInput801C81EC {
    HostBlockSnapshot801C81EC block{};
    bool pumpKnown = false;
    Bootstrap15590LoaderPumpResult801C81EC pump{};
    uint8_t callbackPumpCount = 0;
    bool callbackAttempted = false;
    bool callbackApplied = false;
    uint8_t cdLookupPumpCount = 0;
    bool cdLookupAttempted = false;
    bool cdLookupApplied = false;
    uint8_t cdLowerPumpCount = 0;
    bool cdLowerAttempted = false;
    bool cdLowerApplied = false;
    uint8_t recordDispatchPumpCount = 0;
    bool recordDispatchAttempted = false;
    bool recordDispatchApplied = false;
};

enum class Bootstrap15590HostTickCommand801C81EC : uint8_t {
    None,
    Pump,
    TryCallback,
    TryCdLookup,
    TryCdLower,
    TryRecordDispatch,
    ResolvePumpHost,
};

struct Bootstrap15590HostTickPlan801C81EC {
    bool active = false;
    Bootstrap15590HostTickCommand801C81EC command =
        Bootstrap15590HostTickCommand801C81EC::None;
    Bootstrap15590PumpHostBlockResult801C81EC pumpHost{};
};

struct Bootstrap15590RecordDispatchProbe801C81EC {
    bool active = false;
    bool begun = false;
    bool loaderOwnerKnown = false;
    bool waitingStepKnown = false;
    bool waitingStepValid = false;
    PrStage1LoaderDirect::ProducerStepKind stepKind =
        PrStage1LoaderDirect::ProducerStepKind::None;
    uint16_t recordIndex = 0;
    PrStage1LoaderDirect::LoaderRecordType recordType =
        PrStage1LoaderDirect::LoaderRecordType::Unknown;
    bool currentPayloadKnown = false;
    bool currentPayloadValid = false;
    PrStage1LoaderDirect::LoaderRecordType currentPayloadRecordType =
        PrStage1LoaderDirect::LoaderRecordType::Unknown;
    bool currentPayloadLiveBytes = false;
    bool currentPayloadLiveDataKnown = false;
    bool currentPayloadLiveSizeKnown = false;
    std::size_t currentPayloadLiveSize = 0u;
    bool historyRecord0PayloadKnown = false;
    bool historyRecord0PayloadValid = false;
    PrStage1LoaderDirect::LoaderRecordType historyRecord0PayloadRecordType =
        PrStage1LoaderDirect::LoaderRecordType::Unknown;
    bool historyRecord0PayloadLiveBytes = false;
    bool historyRecord0PayloadLiveDataKnown = false;
    bool historyRecord0PayloadLiveSizeKnown = false;
    std::size_t historyRecord0PayloadLiveSize = 0u;
    bool recordDataBuilt = false;
};

void Reset801C81EC(State801C81EC& state);
void ResetLoaderOwnerState801C81EC(State801C81EC& state);
PrStage1LoaderDirect::RunnerState& GetLoaderRunnerState801C81EC(
    State801C81EC& state);
const PrStage1LoaderDirect::RunnerState& GetLoaderRunnerState801C81EC(
    const State801C81EC& state);
PrStage1LoaderProducerAdapter::ProducerRuntime&
GetLoaderProducerRuntime801C81EC(State801C81EC& state);
const PrStage1LoaderProducerAdapter::ProducerRuntime&
GetLoaderProducerRuntime801C81EC(const State801C81EC& state);

HostBlockSnapshot801C81EC GetHostBlockSnapshot801C81EC(
    const State801C81EC& state);
PendingActionDrainPlan801C81EC BuildPendingActionDrainPlan801C81EC(
    const HostBlockSnapshot801C81EC& block);
HostBlockKind801C81EC GetActionHostBlockKind801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action);
Bootstrap15590DirectPlan801C81EC BuildBootstrap15590DirectPlan801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action);
HostBlockStart801C81EC BuildBootstrap15590HostBlockStart801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const Bootstrap15590HostStartFacts801C81EC& facts);
Bootstrap15590HostStartPlan801C81EC BuildBootstrap15590HostStartPlan801C81EC(
    const Bootstrap15590HostStartInput801C81EC& input);
Bootstrap15590HostTickPlan801C81EC BuildBootstrap15590HostTickPlan801C81EC(
    const Bootstrap15590HostTickInput801C81EC& input);
bool IsBootstrap15590DirectAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action);
bool TryGetBootstrap15590DirectPlan801C81EC(
    const State801C81EC& state,
    Bootstrap15590DirectPlan801C81EC& out);
void ClearBootstrap15590DirectPlan801C81EC(State801C81EC& state);
Bootstrap15590LoaderBeginResult801C81EC
BeginBootstrap15590LoaderDirect801C81EC(State801C81EC& state);
Bootstrap15590LoaderBeginResult801C81EC
BeginBootstrap15590LoaderDirect801C81EC(
    State801C81EC& state,
    const Bootstrap15590DirectPlan801C81EC& plan);
Bootstrap15590LoaderPumpResult801C81EC
PumpBootstrap15590LoaderDirect801C81EC(State801C81EC& state,
                                      std::size_t maxActionCount = 64);
bool DescribeBootstrap15590LoaderWaitingStep801C81EC(
    const State801C81EC& state,
    PrStage1LoaderDirect::ProducerStep& out);
bool BuildBootstrap15590LoaderCallbackHostRequest801C81EC(
    const State801C81EC& state,
    Bootstrap15590CallbackHostRequest801C81EC& out);
bool BuildBootstrap15590CdLookupLowerProducerRequest801C81EC(
    State801C81EC& state,
    Bootstrap15590CdLookupHostRequest801C81EC& out);
bool BuildBootstrap15590CdLowerProducerRequest801C81EC(
    State801C81EC& state,
    Bootstrap15590CdLowerHostRequest801C81EC& out);
bool RunBootstrap15590CdLookupLowerProducerFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LoaderCdHal::LookupFeedback800381F8& feedback,
    const PrStage1XaCdDirectState* xaCdState);
bool RunBootstrap15590LoaderCallbackHostFeedback801C81EC(
    State801C81EC& state,
    const Bootstrap15590CallbackHostInput801C81EC& input);
Bootstrap15590PumpHostBlockResult801C81EC
ResolveBootstrap15590PumpHostBlockResult801C81EC(
    const Bootstrap15590LoaderPumpResult801C81EC& pump);
ActionApplyResult801C81EC ApplyBootstrap15590PumpHostBlockResult801C81EC(
    State801C81EC& state,
    const Bootstrap15590PumpHostBlockResult801C81EC& pumpHost);
bool RunBootstrap15590LoaderMemoryProducer801C81EC(
    State801C81EC& state,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::MemoryRuntimeStepResult* out = nullptr);
bool BuildBootstrap15590LoaderRecordDispatchLiveInput801C81EC(
    const State801C81EC& state,
    PrStage1LoaderProducerAdapter::RecordDispatchLiveInput& out);
Bootstrap15590RecordDispatchProbe801C81EC
ProbeBootstrap15590LoaderRecordDispatch801C81EC(
    const State801C81EC& state);
bool BuildBootstrap15590LoaderRecordDispatchLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderProducerAdapter::RecordDispatchLiveInput& input,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out = nullptr);
bool RunBootstrap15590LoaderRecordDispatchLiveProducer801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::RecordDispatchLiveInput& input,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback = nullptr,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out = nullptr);
bool RunBootstrap15590CdLowerFacts801C81EC(
    State801C81EC& state,
    PrStage1XaCdDirectState& xaCdState,
    const PrStage1LowerCdProducerDirect::LowerCdProducerFacts& facts,
    Bootstrap15590CdLowerFactsApplyResult801C81EC* out = nullptr,
    bool applyLoaderFeedback = true);
bool BuildBootstrap15590MovieSegmentCdLookupCarrier801C81EC(
    State801C81EC& state,
    const std::filesystem::path& binPath,
    bool binPathKnown,
    const PrStage1XaCdDirectState* xaCdState,
    PrStage1MovieSegmentDirect::MovieSegmentCdLookupBatchFeedback801C4780&
        out);
PrStageStatusBankDirectCallRequest BuildStatusBankDirectMemoryRequest801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action);
ActionApplyResult801C81EC ApplyStatusBankDirectMemoryFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const StatusBankDirectMemoryFeedback801C81EC& feedback);
StatusBankDirectMemoryGap801C81EC
BuildStatusBankDirectMemoryGap801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const StatusBankDirectMemoryFeedback801C81EC& feedback);
SaveStatus1635CReplayBackupHostRequest801C81EC
BuildSaveStatus1635CReplayBackupHostRequest801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action);
ActionApplyResult801C81EC ApplySaveStatus1635CReplayBackupHostFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const SaveStatus1635CReplayBackupHostFeedback801C81EC& feedback);
SaveStatus1635CReplayBackupHostGap801C81EC
BuildSaveStatus1635CReplayBackupHostGap801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const SaveStatus1635CReplayBackupHostFeedback801C81EC& feedback);
PrStage1LifecycleDirect::InitSceneInput801C7284
BuildStage1InitSceneInputWithMovieSegmentFeedback801C7284(
    State801C81EC& state,
    const std::filesystem::path& dataRoot,
    const PrStage1XaCdDirectState* xaCdState);
bool BuildBootstrap15590LoaderGpuTimBytesLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderProducerAdapter::GpuTimBytesLiveInput& timBytes,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out = nullptr);
bool RunBootstrap15590LoaderGpuTimBytesLiveProducer801C81EC(
    State801C81EC& state,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback = nullptr,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out = nullptr);
bool BuildBootstrap15590LoaderSpuHalResultLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    const PrStage1LoaderDirect::SpuSeamResult& spu,
    bool success,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out = nullptr);
bool BuildBootstrap15590LoaderSpuHalLowerResultLiveTypedFeedback801C81EC(
    const State801C81EC& state,
    PrStage1LoaderSpuHal::State& spuState,
    const PrStage1LoaderProducerAdapter::SpuHalResultLiveInput& input,
    bool success,
    PrStage1LoaderProducerAdapter::TypedActionFeedback& outFeedback,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out = nullptr);
bool RunBootstrap15590LoaderSpuHalLowerResultLiveProducer801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::SpuHalResultLiveInput& input,
    bool success,
    bool applyFeedback,
    PrStage1LoaderProducerAdapter::TypedActionFeedback* outFeedback = nullptr,
    PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult* out = nullptr);
bool ApplyBootstrap15590LoaderTypedFeedback801C81EC(
    State801C81EC& state,
    const PrStage1LoaderProducerAdapter::TypedActionFeedback& feedback);
ActionApplyResult801C81EC BeginHostBlock801C81EC(
    State801C81EC& state,
    const HostBlockStart801C81EC& start);
ActionApplyResult801C81EC ApplyHostBlockFeedback801C81EC(
    State801C81EC& state,
    const HostBlockFeedback801C81EC& feedback);

PrStage1LifecycleDirect::FrameInput801C81EC BuildFrameInput801C81EC(
    const State801C81EC& state,
    const FrameHostInput801C81EC& host);

void ConsumeFrameInputKnownFlags801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::FrameInput801C81EC& input);

void EnqueueActions801C81EC(
    State801C81EC& state,
    const std::vector<PrStage1LifecycleDirect::Action801C81EC>& actions,
    std::size_t firstActionIndex);

void EnqueueActionsAfterBlockedAction801C81EC(
    State801C81EC& state,
    const std::vector<PrStage1LifecycleDirect::Action801C81EC>& actions,
    std::size_t blockedActionIndex);

bool PopNextAction801C81EC(
    State801C81EC& state,
    PrStage1LifecycleDirect::Action801C81EC& out);
bool HasPendingActions801C81EC(const State801C81EC& state);

ActionApplyResult801C81EC MakeImmediateInputResult801C81EC();
ActionApplyResult801C81EC MakeHostBlockResult801C81EC(bool active);
ActionApplyResult801C81EC MakeBlockedActionRetryResult801C81EC();
void MergeActionApplyResult801C81EC(ActionApplyResult801C81EC& aggregate,
                                    const ActionApplyResult801C81EC& next);

void StartStrBlock801C81EC(State801C81EC& state,
                           StrBlockKind801C81EC kind);
void CompleteStrBlock801C81EC(State801C81EC& state,
                              StrBlockKind801C81EC kind,
                              int32_t result = 0);

void SetMovie1Result801C81EC(State801C81EC& state, int32_t result);

void SetStageResult801C7A60(State801C81EC& state, int32_t result);

void SetAbortPollBlockActive801C81EC(State801C81EC& state, bool active);
void SetAbortPollResult26B94(State801C81EC& state, int32_t result);

void SetStageStatus166AC(State801C81EC& state, int32_t status);

void StartClearTailMovieBlock801C81EC(State801C81EC& state,
                                      bool pathResolved,
                                      const std::filesystem::path& path);
void CompleteClearTailMovieBlock801C81EC(State801C81EC& state);
void SetClearTailMovieVisualActive801C81EC(State801C81EC& state, bool active);
bool IsClearTailMovieVisualActive801C81EC(const State801C81EC& state);
bool IsInitialMovie1PreTransition201ACPending801C81EC(
    const State801C81EC& state);
bool IsInitialMovie1PostTransition201ACPending801C81EC(
    const State801C81EC& state);
bool IsClearTailPreMovieTransition201ACPending801C81EC(
    const State801C81EC& state);
bool IsTransition20110Mode1Pending801C81EC(const State801C81EC& state);
bool IsTransition20110Mode2Pending801C81EC(const State801C81EC& state);
void ClearClearTailMovieVisualActive801C81EC(State801C81EC& state);

void SetBootstrap15590Block801C81EC(State801C81EC& state,
                                    bool active,
                                    bool curtainStarted,
                                    bool pathResolved,
                                    const std::filesystem::path& path);
void ClearBootstrap15590Block801C81EC(State801C81EC& state);

void SetSaveUi19148StartPending801C81EC(State801C81EC& state, bool pending);
void SetSaveUi19148Active801C81EC(State801C81EC& state, bool active);
void RecordSaveUi19148LowerFeedbackRequests801C81EC(
    State801C81EC& state,
    const PrStage1SaveUi19148LowerFeedbackRequestList& requests);

void SetDeferredSceneResult801C81EC(State801C81EC& state, int32_t result);
bool HasDeferredSceneResult801C81EC(const State801C81EC& state);
bool PopDeferredSceneResult801C81EC(State801C81EC& state, int32_t& out);
bool PopReadyDeferredSceneResult801C81EC(State801C81EC& state, int32_t& out);
LifecycleStatusWrites801C81EC BuildStatusWrites801C81EC(
    const PrStage1LifecycleDirect::StepResult801C81EC& step);
bool HasStatusWrites801C81EC(const LifecycleStatusWrites801C81EC& writes);
void SetPendingStatusWrites801C81EC(
    State801C81EC& state,
    const LifecycleStatusWrites801C81EC& writes);
bool PopReadyPendingStatusWrites801C81EC(
    State801C81EC& state,
    LifecycleStatusWrites801C81EC& out);
SceneResultDispatch801C81EC ResolveStepSceneResult801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::StepResult801C81EC& step,
    bool waitingForHostBlock);
void ClearDeferredSceneResult801C81EC(State801C81EC& state);

void RecordTransitionActionState801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action);
void RecordLifecycleActionState801C81EC(
    State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action);
bool TryGetTransitionActionState801C81EC(
    const State801C81EC& state,
    PrStage1LifecycleDirect::Action801C81EC& out);
void ClearTransitionActionState801C81EC(State801C81EC& state);

} // namespace PrStage1LifecycleExecutorDirect
