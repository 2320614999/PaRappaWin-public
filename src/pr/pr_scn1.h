#pragma once

#include "pr_stage1_common_lyrics_geometry_direct.h"
#include "pr_stage_scene_submit_constants.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

struct PrGameContext;
struct PrStage1ResolvedTextEvent;

namespace PrStage1LiveHud {
    struct Stage1ResolvedHudTextRouteState;
    struct Stage1ResolvedHudNumericStatusRouteState;
}

namespace PrMovieSegmentDirect {
    struct MovieSegmentRowInitFeedback8001A324;
    struct OverlayTransferSeekFeedback8001A89C;
    struct OverlayTransferReadFeedback8001A818;
}

namespace PrStage1LifecycleExecutorDirect {
    struct HostBlockSnapshot801C81EC;
}

struct PrStage1CommonLyricsSnapshot {
    bool valid = false;
    // Read-only runtime/debug view: true once the current Scene1 common-lyric
    // window has selected the fixed desc/lang pair and the direct runtime is
    // live, even if no text is currently active.
    bool producerActive = false;
    uint8_t mode = 0xFF;
    uint32_t queryFrame = 0;
    uint32_t queryFrame60 = 0;
    uint32_t eventFrame = 0;
    uint32_t eventCursor = 0;
    uint16_t durationFrames = 0;
    int16_t textIndex = 0;
    const char* text = nullptr;
    PrStage1CommonLyricsTextGeometrySub8001B954 textGeometrySub8001B954{};
    PrStage1CommonLyricsFrameSub8001C864 frameSub8001C864{};
};

// Read-only steady-gameplay overlay script-text direct runtime for the Stage1
// `sub_801C8604` path. Keep this internal-only and expose only the
// narrow runtime/debug fields needed for regression coverage.
struct PrStage1OverlayScriptTextSnapshot {
    bool producerActive = false;
    bool directScriptBoxPermit4E = false;
    uint8_t selectedLanguageIndex = 0;
    const char* activeTextPtr = nullptr;
    const char* activeTextMirrorPtr = nullptr;
    // Debug-only source id; display consumers must use the resolved sink.
    uint8_t activeTextId = 0;
    uint16_t activeTimeoutFramesRemaining = 0;
};

struct PrStage1RuntimePairSlotSnapshot {
    bool valid = false;
    uint8_t pairIndex = 0;
    uint16_t datHandle = 0;
    uint16_t vdfHandle = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
};

struct PrStage1RuntimeCameraSlotSnapshot {
    bool valid = false;
    int16_t bezId = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
};

struct PrStage1RuntimePerformanceTodSlotSnapshot {
    bool valid = false;
    uint8_t handle = 0;
    uint8_t sourceStreamId = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
};

struct PrStage1RuntimeSceneSlotSnapshot {
    bool valid = false;
    uint8_t selectedRow = 0;
    uint8_t selectedStream = 0;
    bool sharedTodListEnabled = false;
    uint32_t streamStartFrame = 0;
    uint32_t streamWindowFrames = 0;
    std::array<uint8_t, PrStageSceneSubmitConstants::kDynamicRowSlotCount>
        rowHandles{};
};

struct PrStage1RuntimeDescBankSnapshot {
    bool valid = false;
    uint8_t selector = 0;
    uint32_t startFrame = 0;
    std::array<bool, PrStageSceneSubmitConstants::kDescBankGroupCount>
        activeGroups{};
    std::array<uint32_t, PrStageSceneSubmitConstants::kDescBankGroupCount>
        groupStartFrames{};
    int32_t acceptedTailTimerFrames9443C = 0;
    uint32_t acceptedTailTimerStartFrame9443C = 0;
    uint32_t acceptedTailTimerSourceEventPsxAddr = 0;
};

struct PrStage1RuntimeTamaSelectorSnapshot {
    bool valid = false;
    uint8_t selector = 0;
    uint32_t startFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
};

static constexpr size_t kPrStage1RuntimeCompactRailMaxRows80024744 = 2;
static constexpr size_t kPrStage1RuntimeCompactRailBodySlotCount80024744 = 18;

struct PrStage1RuntimeCompactRailRow80024744 {
    int16_t teacherRaw8C = -1;
    int16_t studentRaw9E = -1;
    uint32_t bodyStreamPtr94 = 0;
    std::array<int8_t, kPrStage1RuntimeCompactRailBodySlotCount80024744>
        bodyStreamBytes94{};
};

struct PrStage1RuntimeCompactRailSnapshot80024744 {
    bool valid = false;
    int16_t painterGate7A = 0;
    int16_t rowCount8A = 0;
    std::array<
        PrStage1RuntimeCompactRailRow80024744,
        kPrStage1RuntimeCompactRailMaxRows80024744>
        rows{};
};

struct PrStage1RuntimeCompactProducerSnapshot801C9094 {
    bool active = false;
    uint32_t sourceEventPsxAddr = 0;
    uint32_t sourceFrame = 0;
    uint8_t teacherRecordIndex = 0;
    uint8_t studentRecordIndex = 0;
    int16_t teacherMode8A = 0;
    int16_t teacherCursor8C = -1;
    int16_t teacherCursor8E = -1;
    int16_t teacherState90 = 0;
    int16_t studentCursor9E = -1;
    int16_t studentCursorA0 = -1;
    int16_t studentStateA2 = 0;
};

struct PrStage1RuntimeSceneSubmitSnapshot {
    bool uiRenderBlock4CC4Valid = false;
    int16_t uiRenderBlock4CC4 = 0;
    uint32_t uiRenderBlock4CC4Frame = 0;
    bool directScriptBoxGate54Known = false;
    uint16_t directScriptBoxGate54 = 0;
    bool directScriptBoxArg68Known = false;
    int16_t directScriptBoxArg68 = 0;
    bool directNumericStatusGate64Known = false;
    uint16_t directNumericStatusGate64 = 0;
    uint32_t ctxFlagsFull = 0;
    bool ctxFlagTickAdvanceKnown801C9094 = false;
    bool ctxFlagTickAdvance801C9094 = false;
    uint32_t ctxFlagsPulse801CBFDC = 0;
    uint16_t ctxFlags40_2000_4000 = 0;
    uint16_t ctx72 = 0;
    uint16_t ctx11C = 0;
    uint16_t ctx120 = 0;
    bool flag2000DynamicRowReload801CBFDC = false;
    bool sceneInitPulse801CB6BC = false;
    bool compactWobbleResetPulse80024744 = false;
    bool compactTeacherResetPulse80024308 = false;
    bool compactStudentResetPulse80024390 = false;
    PrStage1RuntimeCompactProducerSnapshot801C9094 compactProducer801C9094{};
    PrStage1RuntimeCompactRailSnapshot80024744 compactRail80024744{};
};

struct PrStage1RuntimeScriptCtxMirrorSnapshot {
    bool valid = false;
    uint32_t sourceEventPsxAddr = 0;
    uint32_t dueFrame = 0;
    uint32_t ctxFlagsSet = 0;
    uint16_t paDatCtxOffset = 0;
    uint16_t paVdfCtxOffset = 0;
    uint16_t onDatCtxOffset = 0;
    uint16_t onVdfCtxOffset = 0;
    uint16_t performanceTodCtxOffset = 0;
    uint16_t row0TodCtxOffset = 0;
    uint16_t tamaSelectorCtxOffset = 0;
    uint16_t cameraBezCtxOffset = 0;
};

struct PrStage1RuntimeHudTimUploadSnapshot {
    bool valid = false;
    uint8_t channel = 0;
    uint8_t slotId = 0;
    uint32_t eventIndex = 0;
    uint32_t dueFrame = 0;
    uint32_t sourceEventPsxAddr = 0;
    std::array<uint16_t, 4> timIds{};
};

struct PrStage1RuntimeEvent801C9094EventSnapshot {
    bool valid = false;
    uint8_t streamId = 0;
    uint32_t eventIndex = 0;
    uint32_t dueFrame = 0;
    uint32_t psxAddr = 0;
};

struct PrStage1RuntimeEvent801C9094Snapshot {
    bool valid = false;
    uint8_t consumedEventSource = 0;
    PrStage1RuntimeEvent801C9094EventSnapshot flagStreamEvent{};
    PrStage1RuntimeEvent801C9094EventSnapshot idStreamEvent{};
    PrStage1RuntimeEvent801C9094EventSnapshot consumedEvent{};
    PrStage1RuntimeEvent801C9094EventSnapshot compactEvent{};
};

struct PrStage1RuntimeSlotsSnapshot {
    bool valid = false;
    uint32_t queryFrame = 0;
    uint32_t queryFrame60 = 0;
    uint32_t scriptQueryFrame = 0;
    uint8_t mode = 0xFF;
    bool cameraPlaybackEnabled3034 = false;
    PrStage1RuntimePairSlotSnapshot onMotion{};
    PrStage1RuntimePairSlotSnapshot paMotion{};
    PrStage1RuntimeCameraSlotSnapshot camera{};
    PrStage1RuntimePerformanceTodSlotSnapshot performanceTod{};
    PrStage1RuntimePerformanceTodSlotSnapshot row0Tod{};
    PrStage1RuntimeSceneSlotSnapshot scene{};
    PrStage1RuntimeDescBankSnapshot descBank{};
    PrStage1RuntimeTamaSelectorSnapshot tamaSelector{};
    PrStage1RuntimeSceneSubmitSnapshot sceneSubmit{};
    PrStage1RuntimeScriptCtxMirrorSnapshot scriptCtxMirror{};
    PrStage1RuntimeEvent801C9094Snapshot event801C9094{};
    std::array<PrStage1RuntimeHudTimUploadSnapshot, 3> hudTimUploads{};
};

enum class PrStage1HdSubtitleSourceKind : uint8_t {
    None = 0,
    Movie1,
    CommonLyrics,
    OverlayScriptText,
};

struct PrStage1HdSubtitleRuntimeSource {
    bool valid = false;
    PrStage1HdSubtitleSourceKind kind = PrStage1HdSubtitleSourceKind::None;
    uint8_t mode = 0xFF;
    uint32_t queryFrame = 0;
    uint32_t eventFrame = 0;
    uint16_t durationFrames = 0;
    int16_t textIndex = 0;
    uint8_t textId = 0;
    uint32_t psxAddr = 0;
    const char* originalText = nullptr;
};

struct PrStage1DisplayGateSnapshot {
    bool valid = false;
    bool directScriptBoxEnabled = false;
    bool numericStatusHudEnabled = false;
    bool subtitleHighLayout = true;
    bool lessonVisible = false;
    uint8_t lessonSelector = 0;
    bool rightRankBlinkVisible = false;
    uint8_t rightRankBlinkTarget = 0;
    bool commonLyricVisible = false;
    bool commonLyricMuteGate = false;
};

// Read-only Stage1 formal lifecycle gate snapshot. Keep this at the derived
// clear/fail decision layer so Scene1 and debug consumers do not couple back
// to runner terminal states or extra raw internals.
struct PrStage1FormalLifecycleSnapshot {
    bool valid = false;
    uint32_t queryFrame = 0;
    bool clearGate = false;
    bool clearTerminalTailGate = false;
    bool failGate = false;
    uint8_t rightRankActiveRow = 0;
    uint32_t awfulHoldFrames = 0;
    uint32_t awfulHoldFramesRequired = 0;
    bool runnerExitLatch76Known = false;
    bool runnerExitLatch76Active = false;
    bool runnerExitLatch76SourceBucket0 = false;
    bool runnerExitLatch76SourceTailStream4 = false;
    bool runnerExitLatch76SourceLowLevelAbort = false;
    bool runnerExitLatch76Bucket0PulseDetailKnown = false;
    uint32_t runnerExitLatch76Bucket0PulseQueryFrame = 0;
    uint8_t runnerExitLatch76Bucket0PulseRightRankRow = 0;
    bool runnerExitLatch76Bucket0PulseCallWindowOpen = false;
    uint16_t runnerExitLatch76Bucket0PulseDescriptorFlags = 0;
    bool runnerExitGate78Known = false;
    bool runnerExitGate78Active = false;
    bool runnerExitGate78SourceEd1CHandoff = false;
    bool runnerExitGate78FrameUpdate9094Known = false;
    uint32_t runnerExitGate78FrameUpdate9094QueryFrame = 0;
    uint16_t runnerExitGate78FrameUpdate9094Ctx76 = 0;
    uint16_t runnerExitGate78FrameUpdate9094Ctx78 = 0;
    bool runnerExitGate78FrameUpdate9094TailFamilyActive = false;
    uint8_t runnerExitGate78FrameUpdate9094TailStream = 0;
    uint8_t runnerExitGate78FrameUpdate9094ActiveStream = 0;
    bool runnerExitGate78Ed1CProducerKnown = false;
    uint16_t runnerExitGate78Ed1CProducerEventStreamFlag = 0;
    bool runnerExitGate78Ed1CProducerFlagDescriptorValid = false;
    bool runnerExitGate78Ed1CProducerActiveFlagStreamValid = false;
    uint32_t runnerExitGate78Ed1CProducerActiveFlagStreamIndex = 0;
    uint32_t runnerExitGate78Ed1CProducerActiveFlagStreamCount = 0;
    bool runnerExitGate78Ed1CProducerDueKnown = false;
    int32_t runnerExitGate78Ed1CProducerDueFrame = 0;
    int32_t runnerExitGate78Ed1CProducerDueDelta = 0;
    uint32_t runnerExitGate78Ed1CProducerFlags04 = 0;
    bool runnerExitGate78Ed1CProducerFlag80 = false;
    bool runnerExitGate78Ed1CProducerConsumedFlagStreamEvent = false;
    bool runnerExitGate78Ed1CProducerProduced = false;
    bool runnerExitEventStreamFlagLastUpdateKnown = false;
    uint8_t runnerExitEventStreamFlagLastUpdateReason = 0;
    uint32_t runnerExitEventStreamFlagLastUpdateQueryFrame = 0;
    uint32_t runnerExitEventStreamFlagLastUpdateScriptFrame = 0;
    uint16_t runnerExitEventStreamFlagLastUpdatePrevious = 0;
    uint16_t runnerExitEventStreamFlagLastUpdateCurrent = 0;
    bool runnerExitEventStreamFlagLastChangeKnown = false;
    uint8_t runnerExitEventStreamFlagLastChangeReason = 0;
    uint32_t runnerExitEventStreamFlagLastChangeQueryFrame = 0;
    uint32_t runnerExitEventStreamFlagLastChangeScriptFrame = 0;
    uint16_t runnerExitEventStreamFlagLastChangePrevious = 0;
    uint16_t runnerExitEventStreamFlagLastChangeCurrent = 0;
    bool runnerExitFlag40Active = false;
    uint8_t runnerExitLateBranchSelectedStream = 0;
    uint8_t runnerExitLateBranchActiveDispatchStream = 0;
    bool runnerExitFlag100BlocksWaitActive = false;
    uint8_t runnerExitFlag100SourceStream = 0;
    uint32_t runnerExitLateBranchScriptFrame = 0;
    bool runnerExitClearTerminalTailPulseInput = false;
    bool runnerExitClearTerminalTailPulseArmed = false;
    bool runnerExitClearTerminalTailPulseBlockedAlreadyArmed = false;
    bool runnerExitClearTerminalTailPulseBlockedActiveDispatch = false;
    bool runnerExitClearTerminalTailPulseBlockedPendingMismatch = false;
    uint8_t runnerExitClearTerminalTailPulseStream = 0;
    bool runnerExitClearTerminalBranchTriggerAttempted = false;
    bool runnerExitClearTerminalBranchTriggerAccepted = false;
    uint32_t runnerExitClearTerminalBranchTriggerScriptFrame = 0;
    uint8_t runnerExitClearTerminalBranchTriggerRightRankRow = 0;
    uint8_t runnerExitClearTerminalBranchTriggerCurrentMode = 0;
    bool runnerExitClearTerminalBranchTriggerBlockedConsumed = false;
    bool runnerExitClearTerminalBranchTriggerBlockedArmed = false;
    bool runnerExitClearTerminalBranchTriggerBlockedFlagNotOne = false;
    bool runnerExitClearTerminalBranchTriggerBlockedRow = false;
    bool runnerExitClearTerminalBranchTriggerBlockedStreamMissing = false;
    bool runnerExitClearTerminalBranchTriggerBlockedCursorDone = false;
    bool runnerExitClearTerminalBranchTriggerBlockedEventNotDue = false;
    bool runnerExitClearTerminalBranchTriggerBlockedMissingFlag80 = false;
    uint16_t runnerExitClearTerminalBranchTriggerStreamFlag = 0;
    uint32_t runnerExitClearTerminalBranchTriggerAttemptCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerAcceptedCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerEligibleCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedFlagNotOneCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedRowCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedFlagAndRowCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedConsumedCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedArmedCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedStreamMissingCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedCursorDoneCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedEventNotDueCount = 0;
    uint32_t runnerExitClearTerminalBranchTriggerBlockedMissingFlag80Count = 0;
    bool runnerExitFirstClearTerminalBranchTriggerKnown = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerScriptFrame = 0;
    uint8_t runnerExitFirstClearTerminalBranchTriggerRightRankRow = 0;
    uint8_t runnerExitFirstClearTerminalBranchTriggerCurrentMode = 0;
    uint16_t runnerExitFirstClearTerminalBranchTriggerStreamFlag = 0;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleKnown = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleScriptFrame = 0;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleRightRankRow = 0;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleCurrentMode = 0;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleStream1Known = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Cursor = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Count = 0;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueKnown = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueFrame = 0;
    int32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1BaseFrame = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame = 0;
    int32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Flags04 = 0;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte29 = 0;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte30 = 0;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown = false;
    bool runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame = 0;
    int32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame = 0;
    int32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 = 0;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 = 0;
    uint8_t runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 = 0;
    uint32_t runnerExitClearTerminalBranchTriggerStream1Cursor = 0;
    uint32_t runnerExitClearTerminalBranchTriggerStream1Count = 0;
    bool runnerExitClearTerminalBranchTriggerStream1DueKnown = false;
    uint32_t runnerExitClearTerminalBranchTriggerStream1DueFrame = 0;
    int32_t runnerExitClearTerminalBranchTriggerStream1DueDelta = 0;
    uint32_t runnerExitClearTerminalBranchTriggerStream1BaseFrame = 0;
    uint32_t runnerExitClearTerminalBranchTriggerStream1AbsDueFrame = 0;
    int32_t runnerExitClearTerminalBranchTriggerStream1AbsDueDelta = 0;
    uint32_t runnerExitClearTerminalBranchTriggerStream1Flags04 = 0;
    uint8_t runnerExitClearTerminalBranchTriggerStream1Byte29 = 0;
    uint8_t runnerExitClearTerminalBranchTriggerStream1Byte30 = 0;
    bool runnerExitFirstClearTerminalBranchTriggerEventNotDueKnown = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueScriptFrame = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueCursor = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueFrame = 0;
    int32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueBaseFrame = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsFrame = 0;
    int32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerEventNotDueFlags04 = 0;
    bool runnerExitFirstClearTerminalBranchTriggerMissingFlag80Known = false;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80Cursor = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80Frame = 0;
    int32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80Delta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80BaseFrame = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsFrame = 0;
    int32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsDelta = 0;
    uint32_t runnerExitFirstClearTerminalBranchTriggerMissingFlag80Flags04 = 0;
    bool runnerExitClearTerminalTailServiceLatch = false;
    uint32_t runnerExitClearTerminalTailServiceScriptFrame = 0;
    uint8_t runnerExitClearTerminalTailServiceRightRankRow = 0;
    uint8_t runnerExitClearTerminalTailServiceCurrentMode = 0;
    uint8_t runnerExitClearTerminalTailServiceStream = 0;
    uint32_t runnerExitPendingRatingBranchSeq = 0;
    uint32_t runnerExitConsumedRatingBranchSeq = 0;
    bool runnerExitClearTerminalTailArmed = false;
    bool runnerExitClearTerminalTailDispatchActive = false;
    uint8_t runnerExitClearTerminalTailStream = 0;
    uint32_t runnerExitActiveDispatchStartScriptFrame = 0;
    uint32_t runnerExitActiveDispatchTerminalEndLocalFrame = 0;
    bool runnerExitActiveDispatchTerminalPulseEmitted = false;
    bool runnerTailRecordsModeDAActive = false;
    bool runnerTailRecordsModeDAEqualsOne = false;
    bool runnerTailStageStatusKnown = false;
    uint8_t runnerTailStageStatus166AC = 0;
    uint8_t runnerTailCleanupRenderPassBudget = 0;
    bool runnerTailFinalReturnKnown = false;
    int32_t runnerTailFinalReturn = -1;
    bool runnerTailFinalReturnWord59Known = false;
    bool runnerTailFinalReturnWord59IsOne = false;
    bool runnerTailFinalReturnWord60Known = false;
    bool runnerTailFinalReturnWord60IsOne = false;
    bool runnerTailFinalReturnCalls166AC = false;
    bool runnerTailFinalReturnRecordsModeReturnsOne = false;
    bool runnerTailFinalReturnStageStatusReturnsOne = false;
};

// Read-only steady-gameplay numeric/status HUD mirror for the Stage1
// sub_8001E2E4/sub_8001DF24 path. Fields stay grouped by their confirmed
// direct-runtime producer origin so render routes do not infer presentation
// state from Win shell state.
struct PrStage1ScoreDisplaySnapshot {
    // bucket30 mirrors word_80091816 into ctx+0x30; keep this at the
    // cumulative display-mirror layer and avoid gameplay formula semantics.
    int32_t scoreDisplayValue = 0;
};

struct PrStage1RightRankSnapshot {
    // bucket30 writes the right-side active-row selector, blink-target
    // selector, and blink enable bit.
    uint8_t rightRankActiveRow = 0;
    uint8_t rightRankBlinkTargetRow = 0;
    bool rightRankBlinkEnabled = false;
};

struct PrStage1TopLessonPairSnapshot {
    // bucket0 writes the top LESSON pair change-triggered visible bit and
    // lesson selector.
    bool topLessonPairChangeVisible = false;
    uint8_t topLessonPairLessonId = 0;
};

struct PrStage1NumericStatusSnapshot {
    bool valid = false;
    PrStage1ScoreDisplaySnapshot scoreDisplayState{};
    PrStage1RightRankSnapshot rightRankState{};
    bool rightRankFlag0200Pulse = false;
    uint16_t rightRankTransitionAnim18E = 0;
    PrStage1TopLessonPairSnapshot topLessonPairState{};
    bool steadyGameplayGateActive = false;
    bool highLayoutMode = true;
};

namespace PrScn1 {
    int Fn0(PrGameContext& ctx);
    void Fn1(PrGameContext& ctx);
    int Fn2(PrGameContext& ctx);
    void Main(PrGameContext& ctx);
    void Render(PrGameContext& ctx);
    uint32_t GetStage1QueryFrame30(PrGameContext& ctx);
    uint32_t GetStage1QueryFrame60(PrGameContext& ctx);
    bool BuildScene1Row0OverlayTransferFeedback800154B0(
        PrGameContext& ctx,
        PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324&
            outRow0Feedback);
    bool GetStage1OverlayScriptTextRuntimeSnapshot(PrGameContext& ctx, PrStage1OverlayScriptTextSnapshot& out);
    bool GetStage1HdSubtitleRuntimeSource(PrGameContext& ctx, PrStage1HdSubtitleRuntimeSource& out);
    bool GetStage1RuntimeSlotsSnapshot(PrGameContext& ctx,
                                       PrStage1RuntimeSlotsSnapshot& out,
                                       bool allowTransitionFreeze = false,
                                       bool allowStoppedRuntimeFreeze = false);
    bool CopyStage1HostBlockSnapshot801C81EC(
        PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC& out);
    bool GetStage1HudRouteSnapshot(
        PrGameContext& ctx,
        PrStage1LiveHud::Stage1ResolvedHudTextRouteState& outTextRoute,
        PrStage1LiveHud::Stage1ResolvedHudNumericStatusRouteState& outNumericRoute);
    bool GetStage1CommonLyricsHudRouteSnapshot(
        PrGameContext& ctx,
        PrStage1LiveHud::Stage1ResolvedHudTextRouteState& outTextRoute);
}
