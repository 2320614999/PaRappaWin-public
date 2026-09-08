#pragma once

#include "pr_stage1_lifecycle_direct.h"
#include "pr_stage1_lifecycle_executor_direct.h"
#include "pr_stage1_movie_text_direct.h"
#include "pr_stage1_movie_text_outer_loop_direct.h"
#include "pr_stage1_runtime_slots_direct.h"
#include "pr_stage1_scene1_frame_driver_direct.h"
#include "pr_stage1_scene1_movie1_direct.h"
#include "pr_stage1_scorer_host.h"
#include "pr_stage1_save_ui_host_bridge_direct.h"

#include <cstdint>
#include <filesystem>

struct PrGameContext;
struct PrStage1SaveUi19148LowerFeedback;

namespace PrPsxEventFrameDirect {
struct EventFrameState8001E750;
}

namespace PrStage1LifecycleHostAdapter801C81EC {

struct AbortPollEvent4OverlayState801C81EC {
    const PrPsxEventFrameDirect::EventFrameState8001E750* frameState =
        nullptr;
    int32_t selectionState = -1;
    bool ss0ModalOwnerInitialized = false;
    bool inputClosed = false;
    int32_t tailFramesRemaining = 0;
    uint32_t cueFrame80025E6C = 0u;
    uint32_t cueRemaining80025E6C = 0u;
    uint8_t lastInputStatus80025F0C = 0u;
    uint8_t lastTickStatus80025E6C = 0u;
    bool frameTransactionActive80026B94 = false;
    bool frameTransactionCompleteWithinLimits80026B94 = false;
    bool frameHostBoundaryExecuted80026B94 = false;
    uint8_t frameClass80026B94 = 0u;
    uint32_t frameLogicFrame80026B94 = 0u;
    bool inputCueSourceAccepted80025C8C = false;
    bool inputCueHostProjection80025C8C = false;
    int16_t inputCuePlayResult80034240 = -1;
    int32_t inputCueFlushResult80026ECC = 0;
    bool periodicCueSourceAccepted80026EF8 = false;
    bool periodicCueHostProjection80026EF8 = false;
    int16_t periodicCuePlayResult80034240 = -1;
    int32_t periodicCueFlushResult80026ECC = 0;
    bool exactPsxAudioHalParity80034240 = false;
};

PrStage1Scene1Movie1Direct::Movie1HostFeedback BuildMovie1HostFeedback(
    PrGameContext& ctx,
    bool lifecyclePathResolved,
    const std::filesystem::path& lifecyclePath,
    bool freezeSubbox);

uint32_t ResolveMovie1HostFrame30(PrGameContext& ctx);

void AdvanceMovie1HostStrPlayer(
    PrGameContext& ctx,
    PrStage1Scene1Movie1Direct::Movie1RuntimeState& runtime,
    PrStage1Scene1Movie1Direct::Movie1HostFeedback& host);

PrStage1Scene1Movie1Direct::Movie1HostActionFeedback
ExecuteMovie1HostActions(
    PrGameContext& ctx,
    bool lifecyclePathResolved,
    const std::filesystem::path& lifecyclePath,
    const PrStage1Scene1Movie1Direct::Movie1HostActionList& actions);

struct StageRunnerHostResult801C81EC {
    bool known = false;
    int32_t result = 0;
};

using TickHostBlockFn801C81EC =
    PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC (*)(
        PrGameContext& ctx);
using TickInitialMovie1Transition201ACFn801C81EC =
    PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC (*)(
        PrGameContext& ctx,
        const PrStage1LifecycleDirect::Action801C81EC& action);
using RunStageRunnerFn801C81EC = StageRunnerHostResult801C81EC (*)(
    PrGameContext& ctx,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window);
using StageRecordTickFn801C81EC = bool (*)(
    PrGameContext& ctx,
    const std::filesystem::path& stageRuntimePath,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window);
using CaptureReplayBackupFn801C81EC = void (*)(
    uint32_t prevGrade92F40,
    const PrScn1::Stage1NumericRuntimeState::AcceptedProducerReplayBufferRuntime&
        replay,
    PrScn1::Stage1AcceptedProducerReplayBackupRuntime& backup);
using BuildFrameHostInputFn801C81EC =
    PrStage1LifecycleExecutorDirect::FrameHostInput801C81EC (*)(
        PrGameContext& ctx);

struct ActionHostRefs801C81EC {
    PrStage1LifecycleExecutorDirect::State801C81EC& executor;
    PrStage1RuntimeSlotsDirectRuntime& runtimeSlots;
    PrScn1::Stage1NumericRuntimeState& numericRuntime;
    PrScn1::Stage1AcceptedProducerReplayBackupRuntime& acceptedReplayBackup;
    PrStage1MovieTextDirect::Movie1TextRuntime& movieText;
    PrStage1MovieTextOuterLoopDirect::MovieTextOuterLoopRuntimeSub801C455C&
        movieTextOuterLoop;
    PrStage1Scene1Movie1Direct::Movie1RuntimeState* movieRuntime = nullptr;
    uint8_t commonLyricsLanguageIndex = 0;
    TickHostBlockFn801C81EC tickMovie1Block = nullptr;
    TickInitialMovie1Transition201ACFn801C81EC
        tickInitialMovie1Transition201AC = nullptr;
    RunStageRunnerFn801C81EC runStageRunner = nullptr;
    CaptureReplayBackupFn801C81EC captureReplayBackup = nullptr;
    StageRecordTickFn801C81EC stageRecordTick = nullptr;
};

struct LifecycleStepPassResult801C81EC {
    PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
        aggregateApplyResult{};
    PrStage1LifecycleExecutorDirect::SceneResultDispatch801C81EC
        sceneDispatch{};
};

struct LifecycleFrameTransactionResult801C81EC {
    bool sceneResultKnown = false;
    int32_t sceneResult = 0;
};

bool ApplyBootstrap15590CdLowerFacts801C81EC(
    PrGameContext& ctx,
    ActionHostRefs801C81EC& host,
    const PrStage1LowerCdProducerDirect::LowerCdProducerFacts& facts,
    PrStage1LifecycleExecutorDirect::
        Bootstrap15590CdLowerFactsApplyResult801C81EC* out = nullptr);

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC ApplyAction(
    PrGameContext& ctx,
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window,
    ActionHostRefs801C81EC& host);

void DrainPendingActions801C81EC(
    PrGameContext& ctx,
    PrStage1LifecycleDirect::Runtime801C81EC& runtime,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window,
    ActionHostRefs801C81EC& host);

LifecycleStepPassResult801C81EC RunLifecycleStepPass801C81EC(
    PrGameContext& ctx,
    PrStage1LifecycleDirect::Runtime801C81EC& runtime,
    const PrStage1LifecycleDirect::SceneEntry801C7284& sceneEntry,
    const PrStage1LifecycleDirect::FrameInput801C81EC& input,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window,
    ActionHostRefs801C81EC& host);

LifecycleFrameTransactionResult801C81EC RunLifecycleFrameTransaction801C81EC(
    PrGameContext& ctx,
    PrStage1LifecycleDirect::Runtime801C81EC& runtime,
    const PrStage1LifecycleDirect::SceneEntry801C7284& sceneEntry,
    BuildFrameHostInputFn801C81EC buildFrameHostInput,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window,
    ActionHostRefs801C81EC& host);

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickClearTailMovieBlock(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    PrStage1MovieTextOuterLoopDirect::MovieTextOuterLoopRuntimeSub801C455C&
        movieTextOuterLoop,
    PrStage1MovieTextDirect::Movie1TextRuntime& movieText,
    PrStage1Scene1Movie1Direct::Movie1RuntimeState* movieRuntime);

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickAbortPollBlock(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state);

AbortPollEvent4OverlayState801C81EC GetActiveAbortPollEvent4OverlayState801C81EC(
    uint32_t frame);

void ResetAbortPollEvent4Overlay801C81EC();

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickSaveUi19148Block(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1SaveUi19148LowerFeedback* lowerFeedback = nullptr);

PrStage1SaveUiHostBridgeDirect::SaveUi19148HostTickAttempt
TickSaveUi19148Standalone801C81EC(
    PrGameContext& ctx,
    const PrStage1SaveUi19148LowerFeedbackRequestList& pendingRequests,
    PrStage1SaveUi19148LowerFeedbackRequestList* outNextRequests = nullptr);

void ArmProbeOnlySaveUi19148FormatPoll4Feedback801C81EC();
void ArmProbeOnlySaveUi19148FormatTerminalFeedback801C81EC(
    int32_t terminalPollResult80017008);

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickBootstrap15590Block(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state);

} // namespace PrStage1LifecycleHostAdapter801C81EC
