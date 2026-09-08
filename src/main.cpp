#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "app_config.h"
#include "d3d11_renderer.h"
#include "boot_logo.h"
#include "int_loader.h"
#include "resource_manager.h"
#include "logger.h"
#include "pr/pr_game_context.h"
#include "pr/pr_main.h"
#include "pr/pr_event.h"
#include "pr/pr_ui_overlay.h"
#include "pr/pr_sfx.h"
#include "pr/pr_tmd_renderer.h"
#include "pr/pr_transition.h"
#include "pr/pr_stage_runner.h"
#include "pr/pr_stage1_compact_rail_80024744_direct.h"
#include "pr/pr_stage1_bootstrap_cd_request_direct.h"
#include "pr/pr_stage1_hd_subtitles.h"
#include "pr/pr_stage1_lifecycle_executor_direct.h"
#include "pr/pr_stage1_lifecycle_host_adapter_801c81ec.h"
#include "pr/pr_stage1_overlay_parser.h"
#include "pr/pr_stage1_save_ui_direct.h"
#include "pr/pr_stage1_save_ui_host_bridge_direct.h"
#include "pr/pr_stage1_xa_cd_direct.h"
#include "pr/pr_sqevs1.h"
#include "pr/pr_ss0_card_memcard_handoff_direct.h"
#include "pr/pr_ss0_scene0_runtime_direct.h"
#include "pr/pr_ss0_state16_runtime_envelope_import_direct.h"
#include "pr/pr_ss0_word800916f0_direct.h"
#include "pr/pr_pad.h"
#include "pr/pr_scn1.h"
#include "pr/pr_stage_scene_submit_backend.h"
#include "pr/pr_stage_scene_submit_debug.h"
#include "pr/pr_stage1_scorer_host_deps.h"
#include "pr/pr_vtext.h"
#include "debug_server.h"
#include "str_player.h"
#include "tim_data.h"
#include "tim_decoder.h"
#include "xa1_player.h"
#include "audio_engine.h"

#pragma comment(lib, "winmm.lib")

// Window settings
constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;
constexpr const wchar_t* WINDOW_TITLE = L"PaRappa the Rapper";

// 30 FPS timing (logic tick is always 30Hz for PSX accuracy)
constexpr double FRAME_TIME_MS = 1000.0 / 30.0;
constexpr double FRAME_TIME_60_MS = 1000.0 / 60.0;
static bool g_render60fps = false;  // 60fps rendering mode (config/cmdline)

// Game states
enum class GameState {
    BootLogo,
    StrPlayback,  // STR video playback
    Playing,
    Exit
};

// Global state
static D3D11Renderer g_renderer;
static ResourceManager g_resources;
static BootLogo g_bootLogo;
static StrPlayer g_strPlayer;
static Xa1Player g_xa1Player;
static GameState g_state = GameState::BootLogo;
static GameState g_stateAfterStr = GameState::Playing;  // State to transition to after STR playback
static bool g_running = true;
static bool g_upPressed = false;
static bool g_downPressed = false;
static bool g_confirmPressed = false;
static bool g_screenshotRequested = false;
static std::queue<std::filesystem::path> g_pendingShotPaths;
static int g_autoScreenshotFrames[] = {30, 60, 180};  // Auto-screenshot at these frames
static int g_autoScreenshotIndex = 0;
static bool g_prevTexPressed = false;
static bool g_nextTexPressed = false;
static bool g_nextScenePressed = false;
static bool g_genericSwitchPressed = false;
static int g_genericEvent = 0;
static int g_genericEventArg = 0;
static int g_frameNum = 0;
static bool g_autoNextSceneOnce = false;
static double g_lastFps = 0.0;
static int g_autoExitSeconds = 0;
static bool g_faceAutoShots = false;
static int g_scn0DanceEndDelayTicks = 21;
static bool g_ss0ProbeDisableF0StartupPublisher = false;

static int g_dbg_appState = 0;
static int g_dbg_xaPlaying = 0;
static int g_dbg_xaPlayedMs = 0;
static int g_dbg_stageFrame = 0;
static int g_dbg_stageRunning = 0;
static int g_dbg_strPlaying = 0;
static int g_dbg_strFrame = -1;
static int g_dbg_subtitleFlag = 0;
static int g_dbg_scn0Phase = -1;
static int g_dbg_ss0PhaseRaw = -1;
static int g_dbg_ss0PhaseDebug = -1;
static int g_dbg_ss0TransitionReturnPhaseRaw = -1;
static int g_dbg_ss0DirectDispState = 0;
static int g_dbg_ss0DirectDispEventId = 0;
static int g_dbg_ss0DirectMenuIndex = -1;
static int g_dbg_ss0DirectMainMenuRecordsMode = 0;
static int g_dbg_ss0DirectCardEntryCount = 0;
static int g_dbg_ss0DirectCardSelectedBlock = -1;
static int g_dbg_ss0DirectStageSelectEnabledMask = 0;
static int g_dbg_ss0DirectOptionsLanguage = -1;
static int g_dbg_ss0DirectOptionsSubtitle = -1;
static int g_dbg_ss0DirectOptionsPreLoopReleasePending = -1;
static int g_dbg_ss0DirectOptionsCooldown = -1;
static int g_dbg_ss0DirectOptionsTimeoutRemaining = -1;
static int g_dbg_ss0DirectOptionsInitialInputPending = -1;
static int g_dbg_ss0DirectOptionsTailActive = -1;
static int g_dbg_ss0DirectOptionsTailFramesRemaining = -1;
static int g_dbg_ss0DirectOptionsTailResult = -1;
static int g_dbg_ss0DirectPracticePhase = -1;
static int g_dbg_ss0DirectPracticeRound = -1;
static int g_dbg_ss0DirectPracticeRoundFrame = -1;
static int g_dbg_ss0DirectPracticePadStopFrames = 0;
static int g_dbg_ss0DirectPracticePadStopKind = -1;
static int g_dbg_ss0DirectPracticeExitConfirmFrames = 0;
static int g_dbg_ss0DirectPracticeScore = 0;
static int g_dbg_ss0DirectPracticeHits = 0;
static int g_dbg_ss0DirectPracticeMisses = 0;
static int g_dbg_ss0DirectPracticeLastJudge = 9;
static int g_dbg_ss0DirectPracticeLastDelta = 0;
static int g_dbg_ss0DirectPracticeJudgeFlash = 0;
static int g_dbg_ss0DirectPracticeSeqCurA = -1;
static int g_dbg_ss0DirectPracticeSeqCurB = -1;
static int g_dbg_ss0DirectPracticeSeqEnA = 0;
static int g_dbg_ss0DirectPracticeSeqEnB = 0;
static int g_dbg_ss0DirectHiScoreBlink = 0;
static int g_dbg_ss0DirectHiScoreExitLabelState = -1;
static int g_dbg_ss0DirectHiScorePreLoopReleasePending = -1;
static int g_dbg_ss0DirectHiScoreTailActive = -1;
static int g_dbg_ss0DirectHiScoreTailFramesRemaining = -1;
static int g_dbg_ss0DirectHiScoreTailResult = -1;
static int g_dbg_ss0DirectHiScoreOuterPadReleaseActive = -1;
static int g_dbg_ss0DirectHiScoreOuterPadReleasePending = -1;
static int g_dbg_ss0DirectHiScoreEvent6TableKnown = 0;
static int g_dbg_ss0DirectHiScoreEvent6TablePsxAddress = 0;
static int g_dbg_ss0DirectHiScoreEvent6TableByteCount = 0;
static int g_dbg_ss0DirectTitleExitWaitCounter = 0;
static int g_dbg_ss0DirectTitleExitTargetScene = -1;
static int g_dbg_ss0DirectTitleExitPendingMenu = 0;
static int g_dbg_ss0DirectTitleLoopStateV8 = 0;
static int g_dbg_ss0LoadingHoldKind = 0;
static int g_dbg_ss0LoadingScreenKind = 0;
static int g_dbg_ss0LoadingHoldStartFrame = 0;
static int g_dbg_ss0LoadingHoldUntilFrame = 0;
static int g_dbg_ss0LoadingPatternActive8001EF40 = 0;
static int g_dbg_ss0LoadingPatternStyle8001EF40 = 0;
static int g_dbg_ss0LoadingPatternHighlightCount8001EF40 = 0;
static int g_dbg_ss0LoadingPatternSubmittedHighlightCount8001EF40 = 0;
static int g_dbg_ss0LoadingPatternMutationSerial8001EF40 = 0;
static int g_dbg_ss0LoadingPatternCallbackCount8001537C = 0;
static int g_dbg_ss0LoadingPatternGridHashLow8001EF40 = 0;
static int g_dbg_ss0LoadingPatternSubmittedFrame8001EF40 = 0;

struct Stage1ScheduledPadProbe {
    bool enabled = false;
    bool fired = false;
    int targetScene = 1;
    int targetFrame = -1;
    int holdFrames = 0;
    int remainingFrames = 0;
    int firedFrame = -1;
    uint16_t mask = 0;
};

static Stage1ScheduledPadProbe g_stage1ScheduledPadProbe;
static std::vector<Stage1ScheduledPadProbe> g_stageScheduledPadProbes;

static void ClearStage1ScheduledPadProbe(const char* reason) {
    if (g_stage1ScheduledPadProbe.enabled ||
        g_stage1ScheduledPadProbe.fired ||
        g_stage1ScheduledPadProbe.remainingFrames != 0 ||
        !g_stageScheduledPadProbes.empty()) {
        Log::Printf(
            "Stage padat probe cleared reason=%s targetScene=%d targetFrame=%d fired=%d firedFrame=%d remaining=%d queued=%zu mask=0x%04X",
            reason ? reason : "unknown",
            g_stage1ScheduledPadProbe.targetScene,
            g_stage1ScheduledPadProbe.targetFrame,
            g_stage1ScheduledPadProbe.fired ? 1 : 0,
            g_stage1ScheduledPadProbe.firedFrame,
            g_stage1ScheduledPadProbe.remainingFrames,
            g_stageScheduledPadProbes.size(),
            (unsigned)g_stage1ScheduledPadProbe.mask);
    }
    g_stage1ScheduledPadProbe = {};
    g_stageScheduledPadProbes.clear();
}

static std::string DescribeStage1ScheduledPadProbe() {
    std::ostringstream out;
    out << "enabled=" << (g_stage1ScheduledPadProbe.enabled ? 1 : 0)
        << " fired=" << (g_stage1ScheduledPadProbe.fired ? 1 : 0)
        << " targetScene=" << g_stage1ScheduledPadProbe.targetScene
        << " targetFrame=" << g_stage1ScheduledPadProbe.targetFrame
        << " firedFrame=" << g_stage1ScheduledPadProbe.firedFrame
        << " remainingFrames=" << g_stage1ScheduledPadProbe.remainingFrames
        << " holdFrames=" << g_stage1ScheduledPadProbe.holdFrames
        << " queued=" << g_stageScheduledPadProbes.size()
        << " mask=" << (unsigned)g_stage1ScheduledPadProbe.mask;
    return out.str();
}
static int g_dbg_transActive = 0;
static int g_dbg_transPhase = 0;
static int g_dbg_transTotalFrame = 0;
static int g_dbg_transTargetScene = -1;
static int g_dbg_stage1OvlValid = 0;
static int g_dbg_word800916F0Known = 0;
static int g_dbg_word800916F0SoftwareKnown = 0;
static int g_dbg_word800916F0SoftwareValue = 0;
static int g_dbg_word800916F0SoftwareStartupCommitCount = 0;
static int g_dbg_word800916F0ObservationAcceptedCount = 0;
static int g_dbg_word800916F0RuntimeObservationAcceptedCount = 0;
static int g_dbg_word800916F0StartupObservationAcceptedCount = 0;
static int g_dbg_word800916F0ObservationRejectedCount = 0;
static int g_dbg_word800916F0ObservationLastRejectReason = 0;
static int g_dbg_word800916F0ObservationLastStartupSource = 0;
static int g_dbg_word800916F0ProviderIngressAttempted = 0;
static int g_dbg_word800916F0ProviderIngressReadable = 0;
static int g_dbg_word800916F0ProviderIngressPublishAttempted = 0;
static int g_dbg_word800916F0ProviderIngressPublished = 0;
static int g_dbg_word800916F0ProviderIngressKnownAfter = 0;
static int g_dbg_word800916F0ProviderIngressSourceMainGlobalAttempted = 0;
static int g_dbg_word800916F0ProviderIngressSourceMainGlobalReadable = 0;
static int g_dbg_word800916F0ProviderIngressSourceMainGlobalSelfBootstrapBlocked = 0;
static int g_dbg_word800916F0ProviderIngressSourceLoaderHeapAttempted = 0;
static int g_dbg_word800916F0ProviderIngressSourceLoaderHeapReadable = 0;
static int g_dbg_word800916F0ProviderIngressSourceKnownStateRelayAttempted = 0;
static int g_dbg_word800916F0ProviderIngressSourceKnownStateRelayReadable = 0;
static int g_dbg_word800916F0ProviderIngressSourceCdMmioSnapshotAttempted = 0;
static int g_dbg_word800916F0ProviderIngressSourceCdMmioSnapshotReadable = 0;
static int g_dbg_word800916F0ProviderIngressSourceExactCdAttempted = 0;
static int g_dbg_word800916F0ProviderIngressSourceExactCdReadable = 0;
static int g_dbg_word800916F0ProviderIngressAttemptCount = 0;
static int g_dbg_word800916F0ProviderIngressSourceMissingCount = 0;
static int g_dbg_word800916F0ProviderIngressPublishedCount = 0;
static int g_dbg_word800916F0MainGlobalBackingKnown = 0;
static int g_dbg_word800916F0MainGlobalBackingValue = 0;

namespace PrScn1 {
struct Stage1ClearTailMovieDebugSnapshot {
    int hostBlockKind = 0;
    int hostBlockActive = 0;
    int hostBlockWaitingForPendingActions = 0;
    int deferredSceneResultKnown = 0;
    int deferredSceneResult = 0;
    int pendingActionCount = 0;
    int pendingActionKind = 0;
    int pendingActionPsxOrder = 0;
    int pendingActionPsxFunctionKnown = 0;
    int pendingActionPsxFunction = 0;
    int pendingSaveStatus1635CKnown = 0;
    int pendingSaveStatus1635CIndex = 0;
    int pendingSaveStatus1635CPsxOrder = 0;
    int pendingSaveStatus1635CPsxFunctionKnown = 0;
    int pendingSaveStatus1635CPsxFunction = 0;
    int clearTailStageStatusKnown = 0;
    int clearTailWaitingForWord800916F0 = 0;
    int clearTailPreWord800916F0ActionsApplied = 0;
    int saveUi19148StartPending = 0;
    int saveUi19148Active = 0;
    int saveUi19148LowerFeedbackRequestCount = 0;
    int saveUi19148Seed80092F10Known = 0;
    int saveUi19148Seed80092F10Address = 0;
    int bootstrap15590LoaderDirectBeginAttempted = 0;
    int bootstrap15590LoaderDirectBegun = 0;
    int bootstrap15590LoaderDirectBeginSucceeded = 0;
    int bootstrap15590LoaderDirectBeginFailed = 0;
    int bootstrap15590LoaderDirectPumpStarted = 0;
    int bootstrap15590LoaderDirectWaitingExternal = 0;
    int bootstrap15590LoaderDirectCompleted = 0;
    int bootstrap15590LoaderDirectFailed = 0;
    int bootstrap15590LoaderDirectActionCount = 0;
    int bootstrap15590LoaderDirectNextActionIndex = 0;
    int bootstrap15590LoaderDirectLastPumpActionCount = 0;
    int bootstrap15590LoaderDirectWaitingForFeedback = 0, bootstrap15590LoaderDirectWaitingStepValid = 0, bootstrap15590LoaderDirectWaitingStepKind = 0, bootstrap15590LoaderDirectWaitingCategory = 0, bootstrap15590LoaderDirectWaitingActionKind = 0, bootstrap15590LoaderDirectWaitingPsxOrder = 0, bootstrap15590LoaderDirectWaitingPsxFunction = 0, bootstrap15590LoaderDirectWaitingDirectFunction = 0, bootstrap15590LoaderDirectWaitingLowerFunction = 0, bootstrap15590LoaderDirectWaitingCdActionKind = 0, bootstrap15590LoaderDirectWaitingRecordIndex = 0, bootstrap15590LoaderDirectWaitingRecordType = 0;
    int bootstrap15590CdLowerKeyKnown = 0;
    int bootstrap15590CdLowerRequestPending = 0;
    int bootstrap15590CdLowerStatus = 0;
    int bootstrap15590CdLowerPsxOrder = 0;
    int bootstrap15590CdLowerStepKind = 0;
    int bootstrap15590CdLowerCdActionKind = 0;
    int bootstrap15590CdLowerRecordIndex = 0;
    int bootstrap15590CdLowerRecordType = 0;
    int bootstrap15590CdLowerAttemptIndex = 0;
    int bootstrap15590CdLowerLowerFunction = 0;
    int bootstrap15590CdLowerFinalFunction = 0;
    int bootstrap15590CdLowerPayloadBytesRequired = 0;
    int bootstrap15590CdLowerSeekLbaKnown = 0;
    int bootstrap15590CdLowerSeekLba = 0;
    int bootstrap15590CdLowerReadDstPtrKnown = 0;
    int bootstrap15590CdLowerReadDstPtr = 0;
    int bootstrap15590CdLowerReadSectorCountKnown = 0;
    int bootstrap15590CdLowerReadSectorCount = 0;
    int bootstrap15590CdLowerReadStartHalProgressAccepted = 0;
    int bootstrap15590CdLowerReadStartHalProgressReadS27Serial = 0;
    int bootstrap15590CdLowerFinalReadyHalFactsRequired = 0;
    int bootstrap15590CdLowerFinalReadyHalFactsReadS27Serial = 0;
    int bootstrap15590CdLowerCdSyncLoopFactsRequired80037070 = 0;
    int bootstrap15590CdLowerCdSyncLoopFunction80037070 = 0;
    int bootstrap15590CdLowerCdSyncLoopA0WaitModeKnown80037070 = 0;
    int bootstrap15590CdLowerCdSyncLoopA0WaitMode80037070 = 0;
    int bootstrap15590CdLowerLastRejectKnown = 0;
    int bootstrap15590CdLowerLastRejectReason = 0;
    int bootstrap15590CdLowerLastRejectStatus = 0;
    int bootstrap15590CdLowerLastFactsAttempted = 0;
    int bootstrap15590CdLowerLastFactsActionKind = 0;
    int bootstrap15590CdLowerLastFactsReadStartHalFactsKnown = 0;
    int bootstrap15590CdLowerLastFactsReadStartSetupProduced = 0;
    int bootstrap15590CdLowerLastFactsReadStartSetupIncomplete = 0;
    int bootstrap15590CdLowerLastFactsReadStartSetupFirstMissing = 0;
    int bootstrap15590CdLowerLastFactsReadPumpProduced = 0;
    int bootstrap15590CdLowerLastFactsReadPumpIncomplete = 0;
    int bootstrap15590CdLowerLastFactsReadPumpFirstMissing = 0;
    int bootstrap15590CdLowerLastFactsPayloadBytesKnown = 0;
    int bootstrap15590CdLowerLastFactsBridgeProduced = 0;
    int bootstrap15590CdLowerLastFactsBridgeIncomplete = 0;
    int bootstrap15590CdLowerLastFactsLowerCdFactsBridged = 0;
    int bootstrap15590CdLowerLastFactsXaCdSeamKnown = 0;
    int bootstrap15590CdLowerLastFactsXaCdAccepted = 0;
    int bootstrap15590CdLowerLastFactsRejectReason = 0;
    int clearTailMovieBlockActive = 0;
    int clearTailMoviePlayAndWaitPending = 0;
    int clearTailMoviePlayAndWaitResultKnown = 0;
    int clearTailMoviePlayAndWaitResult = 0;
    int clearTailMovieVisualActive = 0;
    int moviePlayAndWaitCompletionPending = 0;
    int movieDrawableActive = 0;
    int transition20110Mode2Pending = 0;
    int transitionSub80020110Mode2Active = 0;
    int transitionSub800201ACActive = 0;
    int transitionSub800201ACCompleted = 0;
    int transitionSub800201ACPhase = 0;
    int transitionSub800201ACGp196 = 0;
    int transitionSub800201ACTailFrames = 0;
    int transitionSub800201ACTailIndex = 0;
    int movieStrFrameActive = 0;
    int movieOutroActive = 0;
    int movieOutroTailActive = 0;
    int movieSkipPreludeActive = 0;
    int movieOuterLoopPhase = 0;
    int movieOuterLoopComplete = 0;
    int movieOuterLoopExitReason = 0;
    int movieOuterLoopReturn = 0;
    int movieOuterLoopLastStepKnown = 0;
    int movieOuterLoopLastStepPhaseBefore = 0;
    int movieOuterLoopLastStepPhaseAfter = 0;
    int movieOuterLoopLastStepActionCount = 0;
    int movieOuterLoopLastStepMovieStepKnown = 0;
    int movieOuterLoopLastStepMovieStepResult = 0;
    int movieOuterLoopLastStepA7F8Computed = 0;
    int movieOuterLoopLastStepA7F8Known = 0;
    int movieOuterLoopLastStepA7F8Result = 0;
    int movieOuterLoopLastStepGapInputMask = 0;
    int movieOuterLoopLastStepGapDword9554 = 0;
    int movieOuterLoopLastStepGapGsWorkBase = 0;
    int movieOuterLoopLastStepGapByte493F4 = 0;
    int movieOuterLoopLastStepGapSub8001A750 = 0;
};

Stage1ClearTailMovieDebugSnapshot GetStage1ClearTailMovieDebugSnapshot();
}

static PrScn1::Stage1ClearTailMovieDebugSnapshot g_dbg_stage1ClearTailMovie{};

struct Stage1OvlActiveDebugSlot {
    int slot = 0;
    int event = -1;
    int frame = -1;
    int tim0 = 0;
    int tim1 = 0;
    int tim2 = 0;
    int tim3 = 0;

    void Clear() {
        slot = 0;
        event = -1;
        frame = -1;
        tim0 = 0;
        tim1 = 0;
        tim2 = 0;
        tim3 = 0;
    }

    void Set(const PrStage1ResolvedHudSlot& resolved) {
        slot = (int)resolved.slotId;
        event = resolved.valid ? (int)resolved.activeEventIndex : -1;
        frame = resolved.valid ? (int)resolved.activeFrame : -1;
        tim0 = resolved.valid ? (int)resolved.timIds[0] : 0;
        tim1 = resolved.valid ? (int)resolved.timIds[1] : 0;
        tim2 = resolved.valid ? (int)resolved.timIds[2] : 0;
        tim3 = resolved.valid ? (int)resolved.timIds[3] : 0;
    }
};

struct Stage1OvlActiveDebugSnapshot {
    int stream = 0;
    int event = -1;
    int queryFrame = -1;
    int dueFrame = -1;
    int textId = 0;
    Stage1OvlActiveDebugSlot slot0{};
    Stage1OvlActiveDebugSlot slot1Cool{};
    Stage1OvlActiveDebugSlot slot1Good{};
    Stage1OvlActiveDebugSlot slot1Bad{};
    Stage1OvlActiveDebugSlot slot1Awfl{};
    Stage1OvlActiveDebugSlot slot2{};

    void Clear() {
        stream = 0;
        event = -1;
        queryFrame = -1;
        dueFrame = -1;
        textId = 0;
        slot0.Clear();
        slot1Cool.Clear();
        slot1Good.Clear();
        slot1Bad.Clear();
        slot1Awfl.Clear();
        slot2.Clear();
    }

    void Set(const PrStage1ResolvedTextEvent& resolved, uint32_t latestQueryFrame) {
        stream = (int)resolved.streamId;
        event = (int)resolved.eventFrame;
        queryFrame = (int)latestQueryFrame;
        dueFrame = (int)resolved.dueFrame;
        textId = (int)resolved.textId;
        slot0.Set(resolved.slot0);
        slot1Cool.Set(resolved.slot1ByMode[0]);
        slot1Good.Set(resolved.slot1ByMode[1]);
        slot1Bad.Set(resolved.slot1ByMode[2]);
        slot1Awfl.Set(resolved.slot1ByMode[3]);
        slot2.Set(resolved.slot2);
    }
};

static Stage1OvlActiveDebugSnapshot g_dbg_stage1OvlActive{};

static std::filesystem::path GetExecutableDir();
static void AppendResolvedTextEventSummary(std::ostringstream& out,
                                           const PrStage1ResolvedTextEvent& resolved,
                                           bool includeQueryFrame,
                                           uint32_t queryFrame);
static bool TryResolveLatestStage1EventForDebug(const PrGameContext& prCtx,
                                                PrStage1OverlayData& scratch,
                                                const PrStage1OverlayData*& outData,
                                                PrStage1ResolvedTextEvent& outResolved,
                                                uint32_t& outQueryFrame);

static bool TryGetStage1OverlayDataForDebug(const PrGameContext& prCtx,
                                            PrStage1OverlayData& scratch,
                                            const PrStage1OverlayData*& outData) {
    if (prCtx.stage1OverlayData && prCtx.stage1OverlayData->valid) {
        outData = prCtx.stage1OverlayData.get();
        return true;
    }

    const std::filesystem::path exeDir = GetExecutableDir();
    const std::filesystem::path cwd = std::filesystem::current_path();
    const std::filesystem::path candidates[] = {
        prCtx.dataRoot,
        exeDir,
        cwd,
        cwd.parent_path(),
    };

    for (const auto& root : candidates) {
        if (root.empty()) continue;
        const std::filesystem::path comod = root / "S1" / "COMOD1.BIN";
        const std::filesystem::path compo = root / "S1" / "COMPO01.INT";
        if (!std::filesystem::exists(comod) || !std::filesystem::exists(compo)) {
            continue;
        }
        if (PrStage1OverlayParser::ParseFromFiles(comod.u8string(), compo.u8string(), scratch)) {
            outData = &scratch;
            return true;
        }
    }

    outData = nullptr;
    return false;
}

static std::string Stage1OverlaySummary(const PrGameContext& prCtx) {
    const PrStage1OverlayData* data = nullptr;
    PrStage1OverlayData scratch;
    data = nullptr;
    const bool ok = TryGetStage1OverlayDataForDebug(prCtx, scratch, data);
    if (!data || !data->valid) {
        (void)ok;
        return "ERR: stage1 overlay parser data not available";
    }

    std::ostringstream out;
    out << "valid=1"
        << " textTables=" << data->textTables.size()
        << " pairs=" << data->pairTable.size()
        << " streamRows=" << data->streamDescRows.size()
        << " hudSlots=" << data->hudSlotDescs.size()
        << " streams=" << data->streams.size();
    if (const auto* s1 = data->FindStream(1)) {
        out << " stream1.count=" << s1->count;
    }
    if (const auto* s8 = data->FindStream(8)) {
        out << " stream8.count=" << s8->count;
    }
    return out.str();
}

static void AppendResolvedHudSlotCompact(std::ostringstream& out,
                                         const PrStage1ResolvedHudSlot& slot,
                                         const char* label) {
    out << " " << label << "=" << (int)slot.slotId;
    if (!slot.valid) {
        out << ":<none>";
        return;
    }
    out << ":#" << slot.activeEventIndex << "@" << slot.activeFrame << "{";
    for (size_t i = 0; i < slot.timIds.size(); i++) {
        if (i) out << ",";
        const uint16_t handle = slot.timIds[i];
        out << handle;
        if (handle != 0 && slot.memNames[i] != nullptr) {
            out << ":" << *slot.memNames[i];
        }
    }
    out << "}";
}

static std::string Stage1OverlayResolveFrame(const PrGameContext& prCtx, int streamId, uint32_t frame) {
    PrStage1OverlayData scratch;
    const PrStage1OverlayData* data = nullptr;
    if (!TryGetStage1OverlayDataForDebug(prCtx, scratch, data) || !data) {
        return "ERR: stage1 overlay parser data not available";
    }

    if (streamId < 1 || streamId > 8) {
        return "ERR: stream not found";
    }
    const PrStage1EventStream* stream = data->FindStream((uint8_t)streamId);
    if (!stream) {
        return "ERR: stream not found";
    }
    const PrStage1ScriptEvent* ev = data->FindLatestEvent((uint8_t)streamId, frame);
    if (!ev) {
        return "ERR: no event at frame";
    }
    const uint8_t lang = (prCtx.languageIndex >= 0 && prCtx.languageIndex < 5) ? (uint8_t)prCtx.languageIndex : 0u;
    PrStage1ResolvedTextEvent resolved;
    data->ResolveTextEvent((uint8_t)streamId, *ev, lang, ev->frame, frame > ev->frame ? (frame - ev->frame) : 0u, resolved);

    std::ostringstream out;
    AppendResolvedTextEventSummary(out, resolved, true, frame);
    return out.str();
}

static void AppendStage1OverlayHudSlotInline(std::ostringstream& out,
                                             const PrStage1OverlayData& data,
                                             uint8_t slotId,
                                             const char* label) {
    out << "\n    " << label << "=" << (int)slotId;
    const PrStage1HudSlotDesc* slot = data.FindHudSlot(slotId);
    if (!slot) {
        out << " <none>";
        return;
    }

    out << " events=" << slot->eventCount;
    const PrStage1HudAnimEvent* ev = data.FindHudSlotInitialEvent(slotId);
    if (!ev) {
        out << " first=<empty>";
        return;
    }

    out << " firstTims={";
    for (size_t i = 0; i < ev->timIds.size(); i++) {
        if (i) out << ", ";
        const uint16_t handle = ev->timIds[i];
        out << handle;
        if (handle != 0) {
            if (const std::string* name = data.FindMemHandleName(handle)) {
                out << ":" << *name;
            }
        }
    }
    out << "}";
}

static void AppendStage1ResolvedHudSlotInline(std::ostringstream& out,
                                              const PrStage1ResolvedHudSlot& slot,
                                              const char* label) {
    out << "\n    " << label << "=" << (int)slot.slotId;
    if (!slot.valid) {
        out << " <none>";
        return;
    }
    out << " active=#" << slot.activeEventIndex << "@" << slot.activeFrame << " tims={";
    for (size_t i = 0; i < slot.timIds.size(); i++) {
        if (i) out << ", ";
        const uint16_t handle = slot.timIds[i];
        out << handle;
        if (handle != 0 && slot.memNames[i] != nullptr) {
            out << ":" << *slot.memNames[i];
        }
    }
    out << "}";
}

static void AppendResolvedTextEventSummary(std::ostringstream& out,
                                           const PrStage1ResolvedTextEvent& resolved,
                                           bool includeQueryFrame,
                                           uint32_t queryFrame) {
    out << "stream=" << (int)resolved.streamId;
    if (includeQueryFrame) {
        out << " queryFrame=" << queryFrame;
    }
    out << " event=0x" << std::hex << resolved.eventPsxAddr << std::dec
        << " eventFrame=" << resolved.eventFrame
        << " dueFrame=" << resolved.dueFrame
        << " textId=" << (int)resolved.textId;
    if (resolved.text && !resolved.text->empty()) {
        out << " text=" << *resolved.text;
    }
    AppendResolvedHudSlotCompact(out, resolved.slot0, "slot0");
    AppendResolvedHudSlotCompact(out, resolved.slot1ByMode[0], "slot1C");
    AppendResolvedHudSlotCompact(out, resolved.slot1ByMode[1], "slot1G");
    AppendResolvedHudSlotCompact(out, resolved.slot1ByMode[2], "slot1B");
    AppendResolvedHudSlotCompact(out, resolved.slot1ByMode[3], "slot1A");
    AppendResolvedHudSlotCompact(out, resolved.slot2, "slot2");
}

static bool TryResolveLatestStage1EventForDebug(const PrGameContext& prCtx,
                                                PrStage1OverlayData& scratch,
                                                const PrStage1OverlayData*& outData,
                                                PrStage1ResolvedTextEvent& outResolved,
                                                uint32_t& outQueryFrame) {
    outData = nullptr;
    outResolved = PrStage1ResolvedTextEvent{};
    outQueryFrame = 0;
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        (prCtx.currentScene == PrSceneId::Scene0 ||
         prCtx.currentScene == PrSceneId::Scene1)) {
        return false;
    }
    if (!TryGetStage1OverlayDataForDebug(prCtx, scratch, outData) || !outData) {
        return false;
    }
    return PrSqevs1::GetLatestResolvedSnapshot(outResolved, outQueryFrame) &&
           outResolved.valid;
}

static void ClearStage1OverlayResolvedDebugVars() {
    g_dbg_stage1OvlActive.Clear();
}

static void ApplyStage1OverlayResolvedDebugVars(
    const PrStage1ResolvedTextEvent& latestResolved,
    uint32_t queryFrame) {
    g_dbg_stage1OvlActive.Set(latestResolved, queryFrame);
}

struct StageSelectStatusDebugValue {
    bool known = false;
    int value = -1;
};

static StageSelectStatusDebugValue GetDirectStageSelectStatusForDebug(
    int sceneId) {
    const PrStageClearStatusBankSnapshot snapshot =
        PrStage1SaveUiDirect::GetStageClearStatusBankSnapshot();
    if (snapshot.statusBytesKnown80092F1D && sceneId >= 1 &&
        sceneId <= 6) {
        return {true,
                std::clamp(static_cast<int>(
                               snapshot.byte80092F1D[sceneId - 1]),
                           0,
                           3)};
    }
    return {};
}

static StageSelectStatusDebugValue GetLegacyStageSelectStatusForDebug(
    int sceneId) {
    return {true, std::clamp(PrEvent::GetStageSelectStatus(sceneId), 0, 3)};
}

static StageSelectStatusDebugValue GetStageSelectStatusForDebug(
    const PrGameContext& prCtx,
    int sceneId) {
    (void)prCtx;
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
        return GetDirectStageSelectStatusForDebug(sceneId);
    }
    return GetLegacyStageSelectStatusForDebug(sceneId);
}

static int Stage1RailBodyCount(
    const std::array<int8_t, kPrStage1RuntimeCompactRailBodySlotCount80024744>&
        body) {
    int count = 0;
    for (const int8_t v : body) {
        if (v != 0) {
            ++count;
        }
    }
    return count;
}

static std::string Stage1RailSnapshotDebugSurface(PrGameContext& prCtx) {
    PrStage1RuntimeSlotsSnapshot slots{};
    const bool slotsValid = PrScn1::GetStage1RuntimeSlotsSnapshot(prCtx, slots) &&
                            slots.valid;
    const auto& compact = slots.sceneSubmit.compactRail80024744;
    const auto& draw =
        PrStage1CompactRail80024744Direct::GetLastFrameDebug80024744();

    const int16_t slot0Raw =
        compact.valid ? compact.rows[0].teacherRaw8C
                      : (draw.valid ? draw.rows[0].teacherRaw8C : -1);
    const int16_t slot1Raw =
        compact.valid ? compact.rows[0].studentRaw9E
                      : (draw.valid ? draw.rows[0].studentRaw9E : -1);
    const bool slot0PortraitVisible =
        draw.valid && draw.rows[0].teacherPortrait.drawSubmitted;
    const bool slot1PortraitVisible =
        draw.valid && draw.rows[0].studentPortrait.drawSubmitted;
    const float slot0PortraitX =
        draw.valid ? draw.rows[0].teacherPortrait.x : -1.0f;
    const float slot0PortraitY =
        draw.valid ? draw.rows[0].teacherPortrait.y : -1.0f;
    const int slot0PortraitHold =
        draw.valid ? draw.rows[0].teacherPortrait.holdCount : -1;
    const float slot1PortraitX =
        draw.valid ? draw.rows[0].studentPortrait.x : -1.0f;
    const float slot1PortraitY =
        draw.valid ? draw.rows[0].studentPortrait.y : -1.0f;
    const int slot1PortraitHold =
        draw.valid ? draw.rows[0].studentPortrait.holdCount : -1;
    const int bodyCount =
        compact.valid ? Stage1RailBodyCount(compact.rows[0].bodyStreamBytes94)
                      : (draw.valid ? (int)draw.rows[0].bodyNoteCount : 0);
    const bool railRendered = draw.valid && draw.spriteCommandCount > 0;
    const bool railVisible =
        (compact.valid && compact.painterGate7A != 0) || railRendered;

    std::ostringstream out;
    out << "cursor.valid=" << (slotsValid ? 1 : 0)
        << " cursor.queryFrame=" << (slotsValid ? (int)slots.queryFrame : -1)
        << " cursor.queryFrame60="
        << (slotsValid ? (int)slots.queryFrame60 : -1);
    out << "\ncursor.producer.active=" << (slotsValid ? 1 : 0)
        << " cursor.producer.teacherSource=current-lane(2)"
        << " cursor.producer.teacherFormalShouldHoldPreviewOwnership=0"
        << " cursor.producer.teacherFormalCurrentLaneWaitingForStudentTailTargetWindow=0"
        << " cursor.producer.teacherFormalShouldPreferLiveSequenceAuthority=1"
        << " cursor.producer.studentLiveTimingSelectedSourceKind=0"
        << " cursor.producer.studentLiveTimingSelectedStartDueFrame="
        << (slotsValid ? (int)slots.scriptQueryFrame : -1)
        << " cursor.producer.studentCurrentLaneMaterializationAllowed=1"
        << " cursor.producer.localTeacherPreviewProbe.startDueFrame=-1"
        << " cursor.producer.localTeacherPreviewProbe.sourceDueFrame=-1";
    const auto& compactProducer =
        slots.sceneSubmit.compactProducer801C9094;
    out << "\nsceneSubmit.ctxFlagsFull=0x"
        << std::hex << slots.sceneSubmit.ctxFlagsFull << std::dec
        << " sceneSubmit.ctxFlagTickAdvanceKnown801C9094="
        << (slots.sceneSubmit.ctxFlagTickAdvanceKnown801C9094 ? 1 : 0)
        << " sceneSubmit.ctxFlagTickAdvance801C9094="
        << (slots.sceneSubmit.ctxFlagTickAdvance801C9094 ? 1 : 0);
    out << "\ncompactProducer801C9094.active="
        << (compactProducer.active ? 1 : 0)
        << " compactProducer801C9094.sourceEventPsxAddr=0x"
        << std::hex << compactProducer.sourceEventPsxAddr << std::dec
        << " compactProducer801C9094.sourceFrame="
        << (int)compactProducer.sourceFrame
        << " compactProducer801C9094.teacherRecordIndex="
        << (int)compactProducer.teacherRecordIndex
        << " compactProducer801C9094.studentRecordIndex="
        << (int)compactProducer.studentRecordIndex
        << " compactProducer801C9094.teacherMode8A="
        << (int)compactProducer.teacherMode8A
        << " compactProducer801C9094.teacherCursor8C="
        << (int)compactProducer.teacherCursor8C
        << " compactProducer801C9094.teacherCursor8E="
        << (int)compactProducer.teacherCursor8E
        << " compactProducer801C9094.teacherState90="
        << (int)compactProducer.teacherState90
        << " compactProducer801C9094.studentCursor9E="
        << (int)compactProducer.studentCursor9E
        << " compactProducer801C9094.studentCursorA0="
        << (int)compactProducer.studentCursorA0
        << " compactProducer801C9094.studentStateA2="
        << (int)compactProducer.studentStateA2;
    out << "\nteacher.effectiveCursor=" << (int)slot0Raw;
    out << "\nstudent.effectiveCursor=" << (int)slot1Raw;
    out << "\nnativeRailConsumer.slot0CompactValueRaw=" << (int)slot0Raw
        << " nativeRailConsumer.slot0CompactPhaseAvailable="
        << ((slot0Raw >= 0) ? 1 : 0)
        << " nativeRailConsumer.slot0CompactCurrentEnable="
        << ((slot0Raw >= 0) ? 1 : 0)
        << " nativeRailConsumer.slot0PortraitVisible="
        << (slot0PortraitVisible ? 1 : 0)
        << " nativeRailConsumer.slot0PortraitX=" << slot0PortraitX
        << " nativeRailConsumer.slot0PortraitY=" << slot0PortraitY
        << " nativeRailConsumer.slot0PortraitHold=" << slot0PortraitHold
        << " nativeRailConsumer.slot0EffectiveCursor=" << (int)slot0Raw;
    out << "\nnativeRailConsumer.slot1CompactValueRaw=" << (int)slot1Raw
        << " nativeRailConsumer.slot1CompactPhaseAvailable="
        << ((slot1Raw >= 0) ? 1 : 0)
        << " nativeRailConsumer.slot1CompactCurrentEnable="
        << ((slot1Raw >= 0) ? 1 : 0)
        << " nativeRailConsumer.slot1PortraitVisible="
        << (slot1PortraitVisible ? 1 : 0)
        << " nativeRailConsumer.slot1PortraitX=" << slot1PortraitX
        << " nativeRailConsumer.slot1PortraitY=" << slot1PortraitY
        << " nativeRailConsumer.slot1PortraitHold=" << slot1PortraitHold
        << " nativeRailConsumer.slot1EffectiveCursor=" << (int)slot1Raw;
    out << "\noverlayRender.nativeRailActivationContent="
        << ((slot0Raw >= 0 || slot1Raw >= 0 || bodyCount > 0) ? 1 : 0)
        << " overlayRender.nativeRailVisible=" << (railVisible ? 1 : 0)
        << " overlayRender.nativeRailRendered=" << (railRendered ? 1 : 0)
        << " overlayRender.nativeRailTeacherDraw="
        << ((draw.valid && draw.rows[0].drawEnabled && slot0Raw >= 0) ? 1 : 0)
        << " overlayRender.nativeRailTeacherPortraitDraw="
        << (slot0PortraitVisible ? 1 : 0)
        << " overlayRender.nativeRailStudentDraw="
        << ((draw.valid && draw.rows[0].drawEnabled && slot1Raw >= 0) ? 1 : 0)
        << " overlayRender.nativeRailStudentPortraitDraw="
        << (slot1PortraitVisible ? 1 : 0)
        << " overlayRender.nativeRailSymbolCount="
        << (draw.valid ? (int)draw.spriteCommandCount : bodyCount)
        << " overlayRender.nativeRailRenderedBodyCount=" << bodyCount;
    const PrStage1CompactRail80024744Direct::
        PsxCompactRailBodyNoteDebug80024744* firstRailNote = nullptr;
    int firstRailNoteRow = -1;
    int firstRailNoteSlot = -1;
    if (draw.valid) {
        for (std::size_t row = 0;
             row < draw.rows.size() && !firstRailNote;
             ++row) {
            const auto& rowDebug = draw.rows[row];
            for (std::size_t slot = 0;
                 slot < rowDebug.bodyNotes.size();
                 ++slot) {
                const auto& note = rowDebug.bodyNotes[slot];
                if (note.drawSubmitted) {
                    firstRailNote = &note;
                    firstRailNoteRow = static_cast<int>(row);
                    firstRailNoteSlot = static_cast<int>(slot);
                    break;
                }
            }
        }
    }
    if (firstRailNote) {
        const auto& submit =
            firstRailNote->submit8001C804.submitMetadata8003F1B4;
        const auto& submitDebug = firstRailNote->submit8001C804;
        const auto& packet = submit.packet;
        const auto& prelude = packet.transformPrelude;
        const auto& rt = prelude.rotTransPers4.rtOutput;
        const bool gtePacketAvailable = packet.written;
        const bool transformPacket =
            submit.packetPath ==
                PrPsxGsSpriteSubmitDirect::
                    GsSortSpritePacketPath8003F1B4::Transform &&
            prelude.evaluated &&
            !packet.transformFieldsGap;
        out << "\nrailGte8003F710.available=1"
            << " railGte8003F710.packetAvailable="
            << (gtePacketAvailable ? 1 : 0)
            << " railGte8003F710.row=" << firstRailNoteRow
            << " railGte8003F710.slot=" << firstRailNoteSlot
            << " railGte8003F710.localGsSpritePrefixKnown="
            << (submitDebug.localGsSpritePrefixKnown ? 1 : 0)
            << " railGte8003F710.localGsSpriteRgbKnown="
            << (submitDebug.localGsSpriteRgbKnown ? 1 : 0)
            << " railGte8003F710.localGsSpriteKnown="
            << (submitDebug.localGsSpriteKnown ? 1 : 0)
            << " railGte8003F710.submitPacketGap="
            << (submitDebug.submitPacketGap ? 1 : 0)
            << " railGte8003F710.packetPath="
            << static_cast<int>(submit.packetPath)
            << " railGte8003F710.packetWritten="
            << (packet.written ? 1 : 0)
            << " railGte8003F710.transformPrelude="
            << (prelude.evaluated ? 1 : 0)
            << " railGte8003F710.rotTransPers4="
            << (prelude.rotTransPers4.evaluated ? 1 : 0)
            << " railGte8003F710.inputsComplete="
            << (rt.inputsCompleteForGeometry ? 1 : 0)
            << " railGte8003F710.sxyWordsKnown="
            << (rt.sxyWordsKnown ? 1 : 0)
            << " railGte8003F710.flagAfterRtptKnown="
            << (rt.flagAfterRtptKnown ? 1 : 0)
            << " railGte8003F710.flagAfterRtpsKnown="
            << (rt.flagAfterRtpsKnown ? 1 : 0)
            << " railGte8003F710.flagOrKnown="
            << (rt.flagOrKnown ? 1 : 0)
            << " railGte8003F710.ir0Known="
            << (rt.ir0Known ? 1 : 0)
            << " railGte8003F710.returnValueKnown="
            << (rt.returnValueKnown ? 1 : 0)
            << " railGte8003F710.returnValue=" << rt.returnValue
            << " railGte8003F710.transformFieldsGap="
            << (packet.transformFieldsGap ? 1 : 0)
            << " railGte8003F710.word2Known="
            << (transformPacket ? 1 : 0)
            << " railGte8003F710.word4Known="
            << (transformPacket ? 1 : 0)
            << " railGte8003F710.word6Known="
            << (transformPacket ? 1 : 0)
            << " railGte8003F710.word8Known="
            << (transformPacket ? 1 : 0);
    } else {
        out << "\nrailGte8003F710.available=0";
    }
    out << "\nruntimeSlots.queryFrame="
        << (slotsValid ? (int)slots.queryFrame : -1)
        << " runtimeSlots.scriptQueryFrame="
        << (slotsValid ? (int)slots.scriptQueryFrame : -1)
        << " runtimeSlots.mode=" << (slotsValid ? (int)slots.mode : -1)
        << " runtimeSlots.onMotion.pairIndex="
        << (slots.onMotion.valid ? (int)slots.onMotion.pairIndex : -1)
        << " runtimeSlots.onMotion.startFrame="
        << (slots.onMotion.valid ? (int)slots.onMotion.startFrame : -1)
        << " runtimeSlots.onMotion.sourceEventPsxAddr=0x" << std::hex
        << (slots.onMotion.valid ? slots.onMotion.sourceEventPsxAddr : 0u)
        << std::dec
        << " runtimeSlots.scene.selectedStream="
        << (slots.scene.valid ? (int)slots.scene.selectedStream : -1)
        << " runtimeSlots.scene.selectedRow="
        << (slots.scene.valid ? (int)slots.scene.selectedRow : -1)
        << " runtimeSlots.scene.streamStartFrame="
        << (slots.scene.valid ? (int)slots.scene.streamStartFrame : -1)
        << " runtimeSlots.scene.streamWindowFrames="
        << (slots.scene.valid ? (int)slots.scene.streamWindowFrames : -1);
    const auto appendEvent801C9094 =
        [&out](const char* name,
               const PrStage1RuntimeEvent801C9094EventSnapshot& ev) {
            out << " event801C9094." << name << ".valid="
                << (ev.valid ? 1 : 0)
                << " event801C9094." << name << ".stream="
                << (int)ev.streamId
                << " event801C9094." << name << ".index="
                << (int)ev.eventIndex
                << " event801C9094." << name << ".due="
                << (int)ev.dueFrame
                << " event801C9094." << name << ".psx=0x"
                << std::hex << ev.psxAddr << std::dec;
        };
    out << "\nevent801C9094.valid="
        << (slots.event801C9094.valid ? 1 : 0)
        << " event801C9094.consumedSource="
        << (int)slots.event801C9094.consumedEventSource;
    appendEvent801C9094("flag", slots.event801C9094.flagStreamEvent);
    appendEvent801C9094("id", slots.event801C9094.idStreamEvent);
    appendEvent801C9094("consumed", slots.event801C9094.consumedEvent);
    appendEvent801C9094("compact", slots.event801C9094.compactEvent);
    out << "\nnumeric.rankRow=0 numeric.rankBlinkTarget=0 numeric.rankBlink=0"
        << " numeric.resolutionKnown=0 numeric.resolutionV22=0"
        << " numeric.rankWritebackCommitted=0"
        << " numeric.directScriptBoxEnabled="
        << (slots.sceneSubmit.directScriptBoxGate54Known &&
                    slots.sceneSubmit.directScriptBoxGate54 != 0
                ? 1
                : 0)
        << " numeric.stage1RunnerResult=-1 numeric.stage1RunnerState=-1"
        << " numeric.stage1RunnerResultIgnoredByLifecycle=0"
        << " numeric.runnerFrame=" << g_dbg_stageFrame
        << " numeric.audioMs=" << g_dbg_xaPlayedMs
        << " numeric.audioFrame30=" << g_dbg_stageFrame
        << " numeric.audioFrame60=" << (g_dbg_stageFrame * 2)
        << " numeric.audioSector75=0 numeric.tick96=0";
    return out.str();
}

static void AppendStage1Bootstrap15590CdLowerObserverSurface(
    std::ostringstream& out,
    bool blockKnown,
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC& block,
    uint32_t currentReadS27Serial) {
    const auto& cd = block.bootstrap15590CdLower;
    const bool finalReadyProgressShape =
        blockKnown &&
        PrStage1LifecycleExecutorDirect::
            IsFinalReadyProgressGapForPendingReadStart801C81EC(
                cd,
                currentReadS27Serial);
    const bool needFinalReadyFacts =
        blockKnown && cd.finalReadyHalFactsRequired &&
        cd.finalReadyHalFactsReadS27Serial == currentReadS27Serial;
    out << " bs15590CdK=" << (blockKnown ? 1 : 0)
        << " bs15590CdPend=" << (cd.requestPending ? 1 : 0)
        << " bs15590CdKeyK=" << (cd.keyKnown ? 1 : 0)
        << " bs15590CdStatus=" << static_cast<unsigned>(cd.status)
        << " bs15590CdKind=" << static_cast<unsigned>(cd.cdActionKind)
        << " bs15590CdLower=0x" << std::hex << cd.lowerFunction
        << " bs15590CdFinal=0x" << cd.finalFunction << std::dec
        << " bs15590CdProgress="
        << (cd.readStartHalProgressAccepted ? 1 : 0)
        << " bs15590CdProgressSerial="
        << cd.readStartHalProgressReadS27Serial
        << " bs15590CdReadS27Serial=" << currentReadS27Serial
        << " bs15590FinalReadyShape="
        << (finalReadyProgressShape ? 1 : 0)
        << " bs15590NeedFinalReadyFacts="
        << (needFinalReadyFacts ? 1 : 0)
        << " bs15590NeedFinalReadyFactsSerial="
        << cd.finalReadyHalFactsReadS27Serial
        << " bs15590FinalReadyFactsAuthorized=0";
}

static std::string Stage1OverlayDescribeAcceptedReplayPayloadGate(
    const PrScn1::Stage1AcceptedReplayPayloadRestoreGateRuntime& gate) {
    std::ostringstream out;
    out << " acceptedReplayRestorePayloadGateObserved="
        << (gate.observed ? 1 : 0)
        << " acceptedReplayRestorePayloadGateKnown="
        << (gate.livePayloadKnown ? 1 : 0)
        << " acceptedReplayRestorePayloadGateHelperGap="
        << (gate.helperGap ? 1 : 0)
        << " acceptedReplayRestorePayloadGateStatusBankKnown80092F1D="
        << (gate.statusBankKnown80092F1D ? 1 : 0)
        << " acceptedReplayRestorePayloadGateByteCount="
        << gate.byteCount
        << " acceptedReplayRestorePayloadGateByteCountExact="
        << (gate.byteCountExact ? 1 : 0)
        << " acceptedReplayRestorePayloadGateRecordsRequiredBytes="
        << gate.recordsRequiredBytes
        << " acceptedReplayRestorePayloadGateRecordsCovered="
        << (gate.recordsCovered ? 1 : 0)
        << " acceptedReplayRestorePayloadGateWriterKnown="
        << (gate.writerKnown ? 1 : 0)
        << " acceptedReplayRestorePayloadGateLastWriter=0x"
        << std::hex << gate.lastWriterFunction << std::dec
        << " acceptedReplayRestorePayloadGateWrote800164B4="
        << (gate.wrote800164B4 ? 1 : 0)
        << " acceptedReplayRestorePayloadGateWrote8001635C="
        << (gate.wrote8001635C ? 1 : 0)
        << " acceptedReplayRestorePayloadGatePublishedCountKnown="
        << (gate.publishedCountKnown ? 1 : 0)
        << " acceptedReplayRestorePayloadGatePublishedCount="
        << gate.publishedCount
        << " acceptedReplayRestorePayloadGatePublishedCountInRange="
        << (gate.publishedCountInRange ? 1 : 0)
        << " acceptedReplayRestorePayloadGateBackupValid="
        << (gate.backupValid ? 1 : 0)
        << " acceptedReplayRestorePayloadGateLastFault=0x"
        << std::hex << gate.lastFaultAddress << std::dec;
    return out.str();
}

static void AppendStage1XaCd359B8RuntimeSourceState(
    std::ostringstream& out,
    const PrStage1XaCdDirectInterruptSnapshotRuntimeSourceResult800359B8&
        sourceResult,
    const PrStage1XaCdDirectInterruptSnapshotTypedSourceAudit800359B8&
        typedSource) {
    out << " xaCd359B8RuntimeSourceAdapter=1"
        << " xaCd359B8RuntimeSourceAvailable="
        << (sourceResult.sourceAvailable ? 1 : 0)
        << " xaCd359B8RuntimeSourceBundleKnown="
        << (sourceResult.bundleKnown ? 1 : 0)
        << " xaCd359B8RuntimeSourcePublishAttempted="
        << (sourceResult.publishAttempted ? 1 : 0)
        << " xaCd359B8RuntimeSourceAccepted="
        << (sourceResult.observation.accepted ? 1 : 0)
        << " xaCd359B8RuntimeSourceReject="
        << static_cast<int>(sourceResult.observation.rejectReason)
        << " xaCd359B8TypedSourceAudit="
        << (typedSource.inspected ? 1 : 0)
        << " xaCd359B8TypedCallbackSlotsKnown="
        << (typedSource.callbackSlotsKnown ? 1 : 0)
        << " xaCd359B8TypedCallbackSlotsAreCallbackStateTable="
        << (typedSource.callbackSlotsAreCallbackStateTable ? 1 : 0)
        << " xaCd359B8TypedCallbackStateTableKnown="
        << (typedSource.callbackStateTableKnown ? 1 : 0)
        << " xaCd359B8TypedInterruptRegsInitialKnown="
        << (typedSource.initialInterruptRegsKnown ? 1 : 0)
        << " xaCd359B8TypedInterruptRegsTerminalKnown="
        << (typedSource.terminalInterruptRegsKnown ? 1 : 0)
        << " xaCd359B8TypedWatchdogKnown="
        << (typedSource.watchdogKnown ? 1 : 0)
        << " xaCd359B8TypedMissingMask=" << typedSource.missingMask
        << " xaCd359B8TypedCanReconstructBundle="
        << (typedSource.typedCanReconstructBundle ? 1 : 0)
        << " xaCd359B8CanFeedObservation="
        << (typedSource.canFeedRuntimeObservation ? 1 : 0)
        << " xaCd359B8ObservationFeedAttempted=0";
}

static PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108
Stage1ReadStatus57108RuntimeSourceWindow(const PrGameContext& prCtx) {
    return PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(
        prCtx.stage1RuntimePsxMemoryProvider);
}

static PrStage1XaCdDirectStatusCounterRuntimeSourceWindow80057110
Stage1ReadStatus57110RuntimeSourceWindow(const PrGameContext& prCtx) {
    return PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
        prCtx.stage1RuntimePsxMemoryProvider);
}

static PrStage1XaCdDirectInterruptSnapshotRuntimeSourceResult800359B8
Stage1PublishInterruptSnapshotRuntimeSource(
    const PrGameContext& prCtx,
    PrStage1XaCdDirectState& xaCd,
    PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8*
        windowOut) {
    const auto window =
        PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
            prCtx.stage1RuntimePsxMemoryProvider);
    if (windowOut != nullptr) {
        *windowOut = window;
    }
    const auto source =
        PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(window);
    return PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
        source,
        xaCd);
}

static const char* Stage1XaCd36AF8TypedFirstMissingName(
    PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8 firstMissing) {
    switch (firstMissing) {
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::None:
        return "none";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        RegisterPointers:
        return "registerPointers";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        InitialInterrupt:
        return "initialInterrupt";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        StableInterrupt:
        return "stableInterrupt";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        CdReg0Status:
        return "cdReg0Status";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        FifoStatusSamples:
        return "fifoStatusSamples";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        ResultByteCount:
        return "resultByteCount";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        ResultBytes:
        return "resultBytes";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        AckWrites:
        return "ackWrites";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        PriorDword80057108:
        return "prior57108";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        PriorDword80057110:
        return "prior57110";
    case PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::
        PriorByte80057119:
        return "prior57119";
    default:
        return "unknown";
    }
}

static void AppendStage1XaCd80036AF8EventSourceAudit(
    std::ostringstream& out,
    const PrStage1XaCdDirectState& xaCd,
    const PrStage1XaCdDirectCdMmioSnapshotRuntimeSourceAudit&
        cdMmioSnapshotAudit,
    const PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8&
        rawEventWindow,
    const PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceResult80036AF8&
        initialInterruptSource,
    const PrStage1XaCdDirectRawEventRuntimeSourceResult80036AF8&
        rawEventSource) {
    const auto typedSource =
        PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(xaCd);
    const auto cdReg3InitialReadPathFirstMissing =
        [&]() -> const char* {
        if (rawEventWindow.cdReg3InitialReadable) {
            return "none";
        }
        if (!cdMmioSnapshotAudit.sourceInstalled) {
            return "cdMmioSource";
        }
        if (!cdMmioSnapshotAudit.readFnInstalled) {
            return "cdMmioReadFn";
        }
        if (cdMmioSnapshotAudit.producerObservationCallCount == 0u) {
            return "cdMmioProducerCall";
        }
        return "cdMmioWindow";
    };
    out << " xaCd36AF8SourceAudit=1"
        << " xaCd36AF8RawEventSourceAdapter=1"
        << " xaCd36AF8CdReg3InitialSourceAdapter=1"
        << " xaCd36AF8CdReg3InitialProviderInstalled="
        << (rawEventWindow.providerInstalled ? 1 : 0)
        << " xaCd36AF8CdReg3InitialReadAttempted="
        << (rawEventWindow.cdReg3InitialReadAttempted ? 1 : 0)
        << " xaCd36AF8CdReg3InitialReadable="
        << (rawEventWindow.cdReg3InitialReadable ? 1 : 0)
        << " xaCd36AF8CdReg3InitialAddr=0x" << std::hex
        << rawEventWindow.cdReg3InitialPsxAddress << std::dec
        << " xaCd36AF8CdReg3InitialBytes="
        << rawEventWindow.cdReg3InitialByteSize
        << " xaCd36AF8CdReg3InitialValueKnown="
        << (rawEventWindow.cdReg3InitialValueKnown ? 1 : 0)
        << " xaCd36AF8CdReg3InitialValue="
        << static_cast<int>(rawEventWindow.cdReg3InitialValue)
        << " xaCd36AF8CdReg3InitialReadPathFirstMissing="
        << cdReg3InitialReadPathFirstMissing()
        << " xaCd36AF8CdReg3InitialExactCdFallbackRequiresAcceptedObservation=1"
        << " xaCd36AF8CdReg3InitialExactCdFallbackAcceptedCount="
        << xaCd.rawEvent80036AF8InitialInterruptObservationAcceptedCount
        << " xaCd36AF8CdReg3InitialExactCdFallbackCanCreate=0"
        << " xaCd36AF8CdReg0StatusSourceAdapter=1"
        << " xaCd36AF8CdReg0StatusReadAttempted="
        << (rawEventWindow.cdReg0StatusReadAttempted ? 1 : 0)
        << " xaCd36AF8CdReg0StatusReadable="
        << (rawEventWindow.cdReg0StatusReadable ? 1 : 0)
        << " xaCd36AF8CdReg0StatusAddr=0x" << std::hex
        << rawEventWindow.cdReg0StatusPsxAddress << std::dec
        << " xaCd36AF8CdReg0StatusBytes="
        << rawEventWindow.cdReg0StatusByteSize
        << " xaCd36AF8CdReg0StatusValueKnown="
        << (rawEventWindow.cdReg0StatusValueKnown ? 1 : 0)
        << " xaCd36AF8CdReg0StatusValue="
        << static_cast<int>(rawEventWindow.cdReg0StatusValue)
        << " xaCd36AF8CdReg3InitialRuntimeSourceAvailable="
        << (initialInterruptSource.sourceAvailable ? 1 : 0)
        << " xaCd36AF8CdReg3InitialRuntimeSourceValueKnown="
        << (initialInterruptSource.valueKnown ? 1 : 0)
        << " xaCd36AF8CdReg3InitialRuntimeSourcePublishAttempted="
        << (initialInterruptSource.publishAttempted ? 1 : 0)
        << " xaCd36AF8CdReg3InitialRuntimeSourceBlocker="
        << static_cast<int>(initialInterruptSource.blocker)
        << " xaCd36AF8CdReg3InitialRuntimeSourceAccepted="
        << (initialInterruptSource.observation.accepted ? 1 : 0)
        << " xaCd36AF8CdReg3InitialRuntimeSourceReject="
        << static_cast<int>(initialInterruptSource.observation.rejectReason)
        << " xaCd36AF8CdReg3InitialObsAccepted="
        << xaCd.rawEvent80036AF8InitialInterruptObservationAcceptedCount
        << " xaCd36AF8CdReg3InitialObsRejected="
        << xaCd.rawEvent80036AF8InitialInterruptObservationRejectedCount
        << " xaCd36AF8CdReg3InitialObsReject="
        << static_cast<int>(
               xaCd.rawEvent80036AF8InitialInterruptObservationLastReject)
        << " xaCd36AF8RawEventSourceAvailable="
        << (rawEventSource.sourceAvailable ? 1 : 0)
        << " xaCd36AF8RawEventSourceTransactionKnown="
        << (rawEventSource.transactionKnown ? 1 : 0)
        << " xaCd36AF8RawEventSourcePublishAttempted="
        << (rawEventSource.publishAttempted ? 1 : 0)
        << " xaCd36AF8RawEventCdMmioSubmitAttempted="
        << (rawEventSource.cdMmioSubmitAttempted ? 1 : 0)
        << " xaCd36AF8RawEventCdMmioSubmitAccepted="
        << (rawEventSource.cdMmioSubmitAccepted ? 1 : 0)
        << " xaCd36AF8RawEventSourceBlocker="
        << static_cast<int>(rawEventSource.blocker)
        << " xaCd36AF8RawEventSourceAccepted="
        << (rawEventSource.accepted ? 1 : 0)
        << " xaCd36AF8RawEventSourceEarlyReturn="
        << (rawEventSource.earlyReturnNoInterrupt ? 1 : 0)
        << " xaCd36AF8RawEventSourceLowerEventApplied="
        << (rawEventSource.lowerEventApplied ? 1 : 0)
        << " xaCd36AF8TypedSourceAudit="
        << (typedSource.inspected ? 1 : 0)
        << " xaCd36AF8TypedRegisterPointersKnown="
        << (typedSource.registerPointersKnown ? 1 : 0)
        << " xaCd36AF8TypedInitialInterruptKnown="
        << (typedSource.initialInterruptKnown ? 1 : 0)
        << " xaCd36AF8TypedStableInterruptKnown="
        << (typedSource.stableInterruptKnown ? 1 : 0)
        << " xaCd36AF8TypedCdReg0StatusKnown="
        << (typedSource.cdReg0StatusKnown ? 1 : 0)
        << " xaCd36AF8TypedFifoStatusSamplesKnown="
        << (typedSource.fifoStatusSamplesKnown ? 1 : 0)
        << " xaCd36AF8TypedResultByteCountKnown="
        << (typedSource.resultByteCountKnown ? 1 : 0)
        << " xaCd36AF8TypedResultBytesKnown="
        << (typedSource.resultBytesKnown ? 1 : 0)
        << " xaCd36AF8TypedAckWritesKnown="
        << (typedSource.ackWritesKnown ? 1 : 0)
        << " xaCd36AF8TypedPrior57108Known="
        << (typedSource.priorDword80057108Known ? 1 : 0)
        << " xaCd36AF8TypedPrior57110Known="
        << (typedSource.priorDword80057110Known ? 1 : 0)
        << " xaCd36AF8TypedPrior57119Known="
        << (typedSource.priorByte80057119Known ? 1 : 0)
        << " xaCd36AF8TypedPrior57119Value="
        << static_cast<int>(xaCd.byte_80057119)
        << " xaCd36AF8TypedPrior57119Producer=0x" << std::hex
        << xaCd.byte80057119ProducerFunction << std::dec
        << " xaCd36AF8LastCdCommand="
        << static_cast<int>(xaCd.lastCdCommand)
        << " xaCd36AF8CommandSerial=" << xaCd.commandSerial
        << " xaCd36AF8ReadS27Serial=" << xaCd.readS27Serial
        << " xaCd36AF8TypedMissingMask=" << typedSource.missingMask
        << " xaCd36AF8TypedFirstMissing="
        << Stage1XaCd36AF8TypedFirstMissingName(typedSource.firstMissing)
        << " xaCd36AF8TypedFirstMissingId="
        << static_cast<int>(typedSource.firstMissing)
        << " xaCd36AF8TypedCanReconstructTransaction="
        << (typedSource.typedCanReconstructTransaction ? 1 : 0)
        << " xaCd36AF8CanFeedRuntimeSource="
        << (typedSource.canFeedRuntimeSource ? 1 : 0)
        << " xaCd36AF8PendingProducerWrites="
        << xaCd.cdCallbackPending800359B8WriteCount
        << " xaCd36AF8PendingProducerGaps="
        << xaCd.cdCallbackPending800359B8GapCount
        << " xaCd36AF8PendingProducerSnapshotCount="
        << xaCd.lowerCdSnapshotPendingProducerCount
        << " xaCd36AF8CallbackEventSnapshotCount="
        << xaCd.lowerCdSnapshotCallbackEventCount
        << " xaCd36AF8GetlocBridgePendingCount="
        << xaCd.halGetlocLowerBridgePendingProducerCount
        << " xaCd36AF8GetlocBridgeEventCount="
        << xaCd.halGetlocLowerBridgeCallbackEventCount
        << " xaCd36AF8ReadyCallbackSlotKnown="
        << (xaCd.dword_800570FCKnown ? 1 : 0)
        << " xaCd36AF8ReadyCallbackSlot=0x" << std::hex
        << xaCd.dword_800570FC << std::dec
        << " xaCd36AF8ReadyCallbackWrites="
        << xaCd.readyCallbackRegisterWriteCount
        << " xaCd36AF8ReadyCallbackClears="
        << xaCd.readyCallbackRegisterClearCount
        << " xaCd36AF8ReadyCallbackInstall39240="
        << xaCd.readyCallbackRegisterInstall39240Count
        << " xaCd36AF8EventSerial="
        << xaCd.cdLowerEvent80036AF8Serial
        << " xaCd36AF8EventDispatchedSerial="
        << xaCd.cdLowerEvent80036AF8DispatchedSerial
        << " xaCd36AF8ReturnKnown="
        << (xaCd.cdLowerEventPsxReturn80036AF8Known ? 1 : 0)
        << " xaCd36AF8Status57108Known="
        << (xaCd.dword_80057108Known ? 1 : 0)
        << " xaCd36AF8Status57108ObsAccepted="
        << xaCd.statusFlags80057108ObservationAcceptedCount
        << " xaCd36AF8Status57108ObsRejected="
        << xaCd.statusFlags80057108ObservationRejectedCount
        << " xaCd36AF8BlockedBeforeEvent="
        << ((xaCd.cdCallbackPending800359B8WriteCount == 0u &&
             xaCd.lowerCdSnapshotPendingProducerCount == 0u &&
             xaCd.lowerCdSnapshotCallbackEventCount == 0u &&
             xaCd.cdLowerEvent80036AF8Serial == 0u)
                ? 1
                : 0);
}

static std::string Stage1OverlayDescribeSavePayloadBankRuntime(
    const PrStage1SavePayloadBankRuntimeSnapshot& bank) {
    std::ostringstream out;
    out << " savePayloadBankRuntimeKnown="
        << (bank.payloadKnown ? 1 : 0)
        << " savePayloadBankRuntimeStatusBankKnown80092F1D="
        << (bank.statusBankKnown80092F1D ? 1 : 0)
        << " savePayloadBankRuntimeHelperGap="
        << (bank.helperGap ? 1 : 0)
        << " savePayloadBankRuntimeSourceKnown="
        << (bank.savePayloadSourceKnown ? 1 : 0)
        << " savePayloadBankRuntimeReplayCandidateKnown8008EEF8="
        << (bank.replayMirrorCandidateKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplayCandidateProducerKnown8008EEF8="
        << (bank.replayMirrorCandidateProducerKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplayCandidateProducer=0x"
        << std::hex << bank.replayMirrorCandidateProducerFunction << std::dec
        << " savePayloadBankRuntimeReplayCandidateByteCountKnown8008EEF8="
        << (bank.replayMirrorCandidateByteCountKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplayCandidateKnownBytes8008EEF8="
        << bank.replayMirrorCandidateKnownByteCount8008EEF8
        << " savePayloadBankRuntimeReplayCandidateFullBackingKnown8008EEF8="
        << (bank.replayMirrorCandidateFullBackingKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplayCandidatePublishedCount901BC="
        << bank.replayMirrorCandidatePublishedCount901BC
        << " savePayloadBankRuntimeReplayCandidateWriteCount901C0="
        << bank.replayMirrorCandidateWriteCount901C0
        << " savePayloadBankRuntimeReplaySourceKnown8008EEF8="
        << (bank.replayMirrorSourceKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplaySourceShapeKnown8008EEF8="
        << (bank.replayMirrorSourceShapeKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplaySourceProducerKnown8008EEF8="
        << (bank.replayMirrorSourceProducerKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplaySourceProducer=0x"
        << std::hex << bank.replayMirrorSourceProducerFunction << std::dec
        << " savePayloadBankRuntimeReplaySourceByteCountKnown8008EEF8="
        << (bank.replayMirrorSourceByteCountKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplaySourceKnownBytes8008EEF8="
        << bank.replayMirrorSourceKnownByteCount8008EEF8
        << " savePayloadBankRuntimeReplaySourceFullBackingKnown8008EEF8="
        << (bank.replayMirrorSourceFullBackingKnown8008EEF8 ? 1 : 0)
        << " savePayloadBankRuntimeReplaySourcePublishedCount901BC="
        << bank.replayMirrorSourcePublishedCount901BC
        << " savePayloadBankRuntimeReplaySourceWriteCount901C0="
        << bank.replayMirrorSourceWriteCount901C0
        << " savePayloadBankRuntimeReplaySourceSetCount="
        << bank.replayMirrorSourceSetCount
        << " savePayloadBankRuntimeReplaySourceInvalidSetCount="
        << bank.replayMirrorSourceInvalidSetCount
        << " savePayloadBankRuntimeReplaySourceHydrateCount="
        << bank.replayMirrorSourceHydrateCount
        << " savePayloadBankRuntimeReplayAuthorityKnown8001635C="
        << (bank.replayMirrorAuthorityKnown8001635C ? 1 : 0)
        << " savePayloadBankRuntimeLastWriter=0x"
        << std::hex << bank.lastWriterFunction << std::dec
        << " savePayloadBankRuntimeWrote800164B4="
        << (bank.wrote800164B4 ? 1 : 0)
        << " savePayloadBankRuntimeWrote8001635C="
        << (bank.wrote8001635C ? 1 : 0)
        << " savePayloadBankRuntimeLastFault=0x"
        << std::hex << bank.lastFaultAddress << std::dec
        << " savePayloadBankRuntimeSub80015CC4Attempted="
        << (bank.sub80015CC4Attempted ? 1 : 0)
        << " savePayloadBankRuntimeSub80015CC4Ok="
        << (bank.sub80015CC4Ok ? 1 : 0)
        << " savePayloadBankRuntimeSub80015CC4Result="
        << bank.sub80015CC4Result
        << " savePayloadBankRuntimeSub8001635CAttempted="
        << (bank.sub8001635CAttempted ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635COk="
        << (bank.sub8001635COk ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CResult="
        << bank.sub8001635CResult
        << " savePayloadBankRuntimeSub8001635CPreflightPayloadKnown="
        << (bank.sub8001635CPreflightPayloadKnown ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CPreflightStatusBankKnown="
        << (bank.sub8001635CPreflightStatusBankKnown ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CPreflightMapped="
        << (bank.sub8001635CPreflightMapped ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CPreflightCarrierSourceKnown="
        << (bank.sub8001635CPreflightCarrierSourceKnown ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CPreflightCarrierSource=0x"
        << std::hex << bank.sub8001635CPreflightCarrierSource << std::dec
        << " savePayloadBankRuntimeSub8001635CPreflightMirrorSourceKnown="
        << (bank.sub8001635CPreflightMirrorSourceKnown ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CReplaySourceKnownAtEntry="
        << (bank.sub8001635CReplayMirrorSourceKnownAtEntry ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CReplaySourceShapeKnownAtEntry="
        << (bank.sub8001635CReplayMirrorSourceShapeKnownAtEntry ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CReplaySourceSetCountAtEntry="
        << bank.sub8001635CReplayMirrorSourceSetCountAtEntry
        << " savePayloadBankRuntimeSub8001635CReplaySourceInvalidSetCountAtEntry="
        << bank.sub8001635CReplayMirrorSourceInvalidSetCountAtEntry
        << " savePayloadBankRuntimeSub8001635CReplaySourceHydrateCountAtEntry="
        << bank.sub8001635CReplayMirrorSourceHydrateCountAtEntry
        << " savePayloadBankRuntimeSub8001635CScratchAuthorityKnown="
        << (bank.sub8001635CScratchAuthorityKnown ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CMirrorCopied="
        << (bank.sub8001635CMirrorCopied ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CAllClearQueried="
        << (bank.sub8001635CAllClearQueried ? 1 : 0)
        << " savePayloadBankRuntimeSub8001635CAllClearWritten="
        << (bank.sub8001635CAllClearWritten ? 1 : 0)
        << " savePayloadBankRuntimeSub800164B4Attempted="
        << (bank.sub800164B4Attempted ? 1 : 0)
        << " savePayloadBankRuntimeSub800164B4Ok="
        << (bank.sub800164B4Ok ? 1 : 0)
        << " savePayloadBankRuntimeSub800164B4Result="
        << bank.sub800164B4Result
        << " savePayloadBankRuntimeTyped800164B4Attempted="
        << (bank.typed800164B4Attempted ? 1 : 0)
        << " savePayloadBankRuntimeTyped800164B4Ok="
        << (bank.typed800164B4Ok ? 1 : 0)
        << " savePayloadBankRuntimeImport80092F10Attempted="
        << (bank.import80092F10Attempted ? 1 : 0)
        << " savePayloadBankRuntimeImport80092F10Ok="
        << (bank.import80092F10Ok ? 1 : 0)
        << " savePayloadBankRuntimeSeedColdBootAttempted="
        << (bank.seedColdBootAttempted ? 1 : 0)
        << " savePayloadBankRuntimeSeedColdBootOk="
        << (bank.seedColdBootOk ? 1 : 0)
        << " savePayloadBankRuntimeSeedColdBootResult="
        << bank.seedColdBootResult;
    return out.str();
}

static std::string Stage1OverlayDescribeBucket31Flag0200Producer(
    const PrScn1::Stage1NumericRuntimeState::Bucket31Flag0200ProducerRuntime&
        producer) {
    std::ostringstream out;
    out << " bucket31ProdK=" << (producer.known ? 1 : 0)
        << " bucket31ProdQ=" << producer.queryFrame
        << " bucket31ProdTick96=" << producer.tick96
        << " bucket31ProdPreEd00=" << producer.preEd00
        << " bucket31ProdAwait="
        << (producer.awaitBucket31AfterGoodToCool ? 1 : 0)
        << " bucket31ProdNarrow=" << (producer.narrowClearPending ? 1 : 0)
        << " bucket31ProdAdditive="
        << (producer.additiveClearPending ? 1 : 0)
        << " bucket31ProdPageClear="
        << (producer.runPageClear14BDC ? 1 : 0)
        << " bucket31ProdConsumer="
        << (producer.consumerPackageRan ? 1 : 0)
        << " bucket31ProdFlag0200="
        << (producer.ctxFlag0200Pulse ? 1 : 0)
        << " bucket31ProdLocalClear="
        << (producer.bucketLocalClearRan ? 1 : 0)
        << " bucket31ProdNarrowFired="
        << (producer.narrowClearFired ? 1 : 0)
        << " bucket31ProdAdditiveClear="
        << (producer.clearDeferredAdditiveBookkeeping ? 1 : 0)
        << " bucket31ProdFollowAction="
        << static_cast<int>(producer.followUpPhaseAction)
        << " bucket31ProdPlayCue="
        << (producer.playCompletionCue ? 1 : 0)
        << " bucket31ProdClearDelay="
        << (producer.clearDelayedCompletionPending ? 1 : 0)
        << " bucket31ProdSrcK=" << (producer.precursorKnown ? 1 : 0)
        << " bucket31ProdSrcQ=" << producer.precursorQueryFrame
        << " bucket31ProdSrcTick96=" << producer.precursorTick96
        << " bucket31ProdSrcActiveRow="
        << static_cast<int>(producer.precursorActiveRow)
        << " bucket31ProdSrcPreEd00=" << producer.precursorPreEd00
        << " bucket31ProdSrcPostEd00=" << producer.precursorPostEd00
        << " bucket31ProdSrcClearAction="
        << static_cast<int>(producer.precursorClearAction)
        << " bucket31ProdSrcDefer31="
        << (producer.precursorDeferBucket31 ? 1 : 0)
        << " bucket31ProdSrcMarkAdditive="
        << (producer.precursorMarkBucket31AdditiveClearPending ? 1 : 0)
        << " bucket31ProdSrcOwnerOpen="
        << (producer.precursorOwnerKernelOpen ? 1 : 0)
        << " bucket31ProdSrcResolverBit4="
        << (producer.precursorResolverGateBit4 ? 1 : 0)
        << " bucket31ProdSrcEd00Idle="
        << (producer.precursorResolutionGateEd00Idle ? 1 : 0)
        << " bucket31ProdSrcResolutionKnown="
        << (producer.precursorRowWriteResolutionKnown ? 1 : 0)
        << " bucket31ProdSrcResolutionV22="
        << static_cast<int>(producer.precursorRowWriteResolutionV22)
        << " bucket31ProdSrcRowWrite="
        << (producer.precursorRowWriteCommitted ? 1 : 0)
        << " bucket31ProdSrcGoodToCool="
        << (producer.precursorRowWriteGoodToCoolCommitted ? 1 : 0)
        << " bucket31ProdSrcAcceptedTail="
        << (producer.precursorAcceptedTailSurvived ? 1 : 0)
        << " bucket31ProdSrcWaitSecondBeat="
        << (producer.precursorWaitSecondBeatInsideBucket30 ? 1 : 0);
    return out.str();
}

static std::string Stage1OverlayDescribeScoreDisplayHandoff(PrGameContext& prCtx) {
    PrStage1RuntimeSlotsSnapshot slots{};
    const bool slotsValid = PrScn1::GetStage1RuntimeSlotsSnapshot(prCtx, slots) &&
                            slots.valid;
    const int query = slotsValid ? (int)slots.queryFrame : -1;
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    const auto& rightRank = numeric.rightRankState;
    const auto& accepted = numeric.acceptedProducer;
    const auto& acceptedProbe = numeric.acceptedProducerBoundaryProbe;
    const auto& acceptedCarrier = numeric.acceptedProducerCarrier;
    const auto& replayMirror = numeric.acceptedProducerReplayBuffer;
    const auto& lastWrite = numeric.acceptedProducerLastRecordedPageWrite;
    const auto& scorer = numeric.scorerPort;
    const auto& phase1Owner = numeric.rightRankPhase1Owner;
    const auto& observer24 = numeric.rightRank24F8CObserver;
    const auto& tieBreaker = numeric.rightRankTieBreakerObserver;
    const auto& helperShadow = numeric.rightRankHelperShadow;
    const auto& ownerObserver = numeric.bucket30OwnerObserver;
    const auto& requiredOccupiedSlot0 =
        numeric.rightRankFirstAcceptedCountClearGameplayRequiredOccupiedSlots[0];
    const auto& requiredOccupiedSlot1 =
        numeric.rightRankFirstAcceptedCountClearGameplayRequiredOccupiedSlots[1];
    const auto& sourceCell = numeric.sourceCellVoice;
    const auto& descriptorCadence = numeric.descriptorCadence;
    const auto& timecode = numeric.runnerTimecode801C7560;
    const auto& tailHost = numeric.runnerTailHost7A60;
    auto& xaCd = prCtx.stage1XaCdDirect;
    const auto xaCdClockProbe =
        PrStage1XaCdDirectProbeStreamClockProducer800493F4(xaCd);
    const auto status57108RuntimeWindow =
        Stage1ReadStatus57108RuntimeSourceWindow(prCtx);
    const auto status57108RuntimeSource =
        PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(
            status57108RuntimeWindow);
    const auto status57108RuntimeSourceResult =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(
            status57108RuntimeSource,
            xaCd);
    const auto rawEventRuntimeWindow =
        PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(
            prCtx.stage1RuntimePsxMemoryProvider);
    const auto rawEventInitialInterruptSource =
        PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
            rawEventRuntimeWindow);
    const auto rawEventInitialInterruptSourceResult =
        PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
            rawEventInitialInterruptSource,
            xaCd);
    const auto rawEventRuntimeSource =
        PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(xaCd);
    const auto rawEventRuntimeSourceResult =
        PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(
            rawEventRuntimeSource,
            xaCd,
            &prCtx.stage1RuntimePsxMemoryProvider);
    const auto xacd359b8RuntimeSourceResult =
        Stage1PublishInterruptSnapshotRuntimeSource(
            prCtx,
            xaCd,
            nullptr);
    const auto xacd359b8TypedSource =
        PrStage1XaCdDirectAuditInterruptSnapshotTypedSource800359B8(xaCd);
    const bool xaHostClockKnown =
        prCtx.xa1Player != nullptr && prCtx.xa1Player->IsPlaying();
    const bool xaPlayerSelectedFilterKnown =
        prCtx.xa1Player != nullptr && prCtx.xa1Player->GetSelectedFilterKnown();
    const bool xaPlayerSelectedCodingKnown =
        prCtx.xa1Player != nullptr && prCtx.xa1Player->GetSelectedCodingKnown();
    const uint32_t xaPlayerCurrentSector =
        prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetCurrentSectorIndex() : 0u;
    const uint32_t xaPlayerAcceptedPushCount =
        prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetAcceptedRawPushCount() : 0u;
    const size_t xaPlayerAcceptedQueueSize =
        prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetAcceptedRawSectorQueueSize() : 0u;
    const bool xaPlayerLastAcceptedSectorKnown =
        prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastAcceptedSectorKnown();
    const int xaHostClockRel75 =
        xaHostClockKnown ? prCtx.xa1Player->GetCdClockSectorIndex() : 0;
    const int xaHostAudibleRel75 =
        xaHostClockKnown ? prCtx.xa1Player->GetEstimatedAudibleSectorIndex() : 0;
    const int xaHostClockAbsLba =
        xaHostClockKnown
            ? (static_cast<int>(xaCd.dword_800493FC) + xaHostClockRel75)
            : 0;
    const int xaHostAudibleAbsLba =
        xaHostClockKnown
            ? (static_cast<int>(xaCd.dword_800493FC) + xaHostAudibleRel75)
            : 0;
    const uint16_t rightRankDescriptorFlagWord =
        PrScn1::ResolveStage1RightRankDescriptorFlagWord(numeric);
    const int currentPageOrdinal1Based =
        (descriptorCadence.pageOrdinal56Available &&
         descriptorCadence.pageOrdinal56 > 0u)
            ? static_cast<int>(
                  std::min(descriptorCadence.pageOrdinal56,
                           static_cast<size_t>(0xFFu)))
            : (numeric.pageRecordMirror.initialized
                   ? numeric.pageRecordMirror.currentPageOrdinal + 1
                   : 0);
    const auto& desc44Row = descriptorCadence.lookaheadDescriptor44Row;
    const bool desc44Available =
        descriptorCadence.lookaheadDescriptor44Available && desc44Row.available;
    const uint16_t desc44Substate50 = numeric.descriptorSubstate50;
    const bool desc44UseSubstateBranch = desc44Substate50 != 0u;
    const uint8_t desc44BranchSelectorByte0 =
        desc44Available
            ? (desc44UseSubstateBranch ? desc44Row.substate1SelectorByte0
                                       : desc44Row.defaultSelectorByte0)
            : 0u;
    const uint8_t desc44BranchSelectorByte1 =
        desc44Available
            ? (desc44UseSubstateBranch ? desc44Row.substate1SelectorByte1
                                       : desc44Row.defaultSelectorByte1)
            : 0u;
    const StageSelectStatusDebugValue stage1Status =
        GetStageSelectStatusForDebug(prCtx, 1);
    const bool stage1StatusKnown = stage1Status.known;
    const bool firstClearStatus =
        stage1StatusKnown && stage1Status.value >= 2;
    const int acceptedFormulaCount =
        static_cast<int>(accepted.acceptedContributionCount) +
        (numeric.additiveLane.value != 0 ? 1 : 0);
    const char* acceptedSplitName =
        acceptedProbe.split == 1u ? "recorded" :
        (acceptedProbe.split == 2u ? "penalty" : "none");
    PrStage1FormalLifecycleSnapshot liveLifecycle{};
    const bool liveValid =
        PrScn1::CopyStage1FormalLifecycleSnapshot(liveLifecycle);
    PrStage1FormalLifecycleSnapshot terminalLifecycle{};
    const bool terminalValid =
        PrScn1::CopyStage1TerminalFormalLifecycleSnapshot(terminalLifecycle);
    PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC hostBlock{};
    const bool hostBlockKnown =
        PrScn1::CopyStage1HostBlockSnapshot801C81EC(hostBlock);
    const PrStage1SavePayloadBankRuntimeSnapshot savePayloadBankRuntime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    const auto queryDeltaOrMissing = [](bool known, int lhs, int rhs) -> int {
        return known ? lhs - rhs : -1;
    };
    const bool liveRunnerClearKnown =
        liveValid && liveLifecycle.runnerExitEventStreamFlagLastRunnerClearKnown;
    const int liveRunnerClearQuery =
        liveRunnerClearKnown
            ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearQueryFrame
            : -1;
    const bool terminalRunnerClearKnown =
        terminalValid &&
        terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearKnown;
    const int terminalRunnerClearQuery =
        terminalRunnerClearKnown
            ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearQueryFrame
            : -1;

    std::ostringstream out;
    out << "numeric scene=" << static_cast<int>(prCtx.currentScene)
        << " stageRunning=" << (prCtx.stageRunning ? 1 : 0)
        << " sceneExitReason=" << prCtx.sceneExitReason
        << " available=" << (numeric.active ? 1 : 0)
        << " query=" << (numeric.active ? (int)numeric.queryFrame : query)
        << " stage1StatusKnown=" << (stage1StatusKnown ? 1 : 0)
        << " stage1Status=" << stage1Status.value
        << " goodToCoolGate=" << (firstClearStatus ? 1 : 0)
        << " proxyGate=" << (firstClearStatus ? 1 : 0)
        << " gateSource=latch"
        << " proxyDirect=0 score=" << numeric.scoreDisplayValue
        << " source=runtimeSlots aligned=1"
        << " formulaBacked=" << (numeric.scoreDisplayFormulaBacked ? 1 : 0)
        << " tick96=" << numeric.runnerTimecode801C7560.state.tick801C364C
        << " timecodeKnown=" << (timecode.known ? 1 : 0)
        << " tcBaseK=" << (timecode.xaSectorBaselineKnown ? 1 : 0)
        << " tcBase75=" << timecode.xaSectorBaseline75
        << " tcXaK=" << (timecode.xaReadValueA7A4Known ? 1 : 0)
        << " tcTimeBaseGap="
        << (timecode.gapMissingSceneEntryField196TimeBase ? 1 : 0)
        << " tcClockGap="
        << (timecode.gapMissingStreamClock800493F4 ? 1 : 0)
        << " tcPollA3C8="
        << (timecode.clockPoll8001A3C8Called ? 1 : 0)
        << " tcPollRet=" << timecode.clockPoll8001A3C8Return
        << " tcPollAccept="
        << (timecode.clockPoll8001A3C8AcceptedByte ? 1 : 0)
        << " tcPollGapD0="
        << (timecode.clockPoll8001A3C8Gap364D0 ? 1 : 0)
        << " tcPollGapA4="
        << (timecode.clockPoll8001A3C8Gap363A4 ? 1 : 0)
        << " tcCmdA280=" << (timecode.command8001A280Called ? 1 : 0)
        << " tcCmdA280Issued=" << (timecode.command8001A280Issued ? 1 : 0)
        << " tcCmdA280Skip="
        << (timecode.command8001A280SkippedNonZeroWorkBase ? 1 : 0)
        << " tcCmdA280Gap="
        << (timecode.command8001A280Gap49428 ? 1 : 0)
        << " tcCmdA280Base=" << timecode.command8001A280WorkBase
        << " xaCdReadS27Serial=" << xaCd.readS27Serial
        << " xaCdSetloc2Serial=" << xaCd.setloc2Serial
        << " xaCdD0FbK=" << (xaCd.cdLowerFeedback80036AF8Known ? 1 : 0)
        << " xaCdD0K=" << (xaCd.cdLowerFeedback80036AF8.known ? 1 : 0)
        << " xaCdD0SyncK="
        << (xaCd.cdLowerFeedback80036AF8.syncResultKnown ? 1 : 0)
        << " xaCdD0Ret=" << xaCd.cdLowerFeedback80036AF8.syncResult
        << " xaCdD0BytesK="
        << (xaCd.cdLowerFeedback80036AF8.responseBytesKnown ? 1 : 0)
        << " xaCdD0BytesN="
        << xaCd.cdLowerFeedback80036AF8.responseByteCount
        << " xaCdD0B0="
        << static_cast<int>(xaCd.cdLowerFeedback80036AF8.responseBytes[0])
        << " xaCdD0B1="
        << static_cast<int>(xaCd.cdLowerFeedback80036AF8.responseBytes[1])
        << " xaCdD0B2="
        << static_cast<int>(xaCd.cdLowerFeedback80036AF8.responseBytes[2])
        << " xaCdExpK=" << (xaCd.cdSyncExplicitStatusKnown ? 1 : 0)
        << " xaCdExpSt=" << static_cast<int>(xaCd.cdSyncExplicitStatus)
        << " xaCdExpBytesK="
        << (xaCd.cdSyncExplicitResponseBytesKnown ? 1 : 0)
        << " xaCdExpB0="
        << static_cast<int>(xaCd.cdSyncExplicitResponseBytes[0])
        << " xaCdExpB1="
        << static_cast<int>(xaCd.cdSyncExplicitResponseBytes[1])
        << " xaCdExpB2="
        << static_cast<int>(xaCd.cdSyncExplicitResponseBytes[2])
        << " xaCdRspF8K=" << (xaCd.response_800882F8Known ? 1 : 0)
        << " xaCdRspF8B0=" << static_cast<int>(xaCd.response_800882F8[0])
        << " xaCdRspF8B1=" << static_cast<int>(xaCd.response_800882F8[1])
        << " xaCdRspF8B2=" << static_cast<int>(xaCd.response_800882F8[2])
        << " xaCd5719K=" << (xaCd.byte_80057119Known ? 1 : 0)
        << " xaCd5719=" << static_cast<int>(xaCd.byte_80057119)
        << " xaCd57108K=" << (xaCd.dword_80057108Known ? 1 : 0)
        << " xaCd57108=" << xaCd.dword_80057108
        << " xaCd57108ObsAccepted="
        << xaCd.statusFlags80057108ObservationAcceptedCount
        << " xaCd57108ObsRejected="
        << xaCd.statusFlags80057108ObservationRejectedCount
        << " xaCd57108ObsReject="
        << static_cast<int>(xaCd.statusFlags80057108LastReject)
        << " xaCd57108RuntimeSource="
        << (status57108RuntimeSourceResult.sourceAvailable ? 1 : 0)
        << " xaCd57108RuntimeSourceAdapter=1"
        << " xaCd57108RuntimeSourceAvailable="
        << (status57108RuntimeSourceResult.sourceAvailable ? 1 : 0)
        << " xaCd57108RuntimeSourceValueKnown="
        << (status57108RuntimeSourceResult.valueKnown ? 1 : 0)
        << " xaCd57108RuntimeSourcePublishAttempted="
        << (status57108RuntimeSourceResult.publishAttempted ? 1 : 0)
        << " xaCd57108RuntimeSourceBlocker="
        << static_cast<int>(status57108RuntimeSourceResult.blocker)
        << " xaCd57108RuntimeSourceAccepted="
        << (status57108RuntimeSourceResult.observation.accepted ? 1 : 0)
        << " xaCd57108RuntimeSourceReject="
        << static_cast<int>(
               status57108RuntimeSourceResult.observation.rejectReason)
        << " xaCd57108Addr=0x" << std::hex << 0x80057108u << std::dec
        << " xaCd57108Bytes=4"
        << " xaCd57108Readable="
        << (status57108RuntimeWindow.windowReadable ? 1 : 0)
        << " xaCd57108CanFeedObservation="
        << ((status57108RuntimeSourceResult.sourceAvailable &&
             status57108RuntimeSourceResult.valueKnown)
                ? 1
                : 0)
        << " xaCd57108ObservationFeedAttempted=0";
    AppendStage1XaCd359B8RuntimeSourceState(
        out,
        xacd359b8RuntimeSourceResult,
        xacd359b8TypedSource);
    const auto cdMmioSnapshotAudit =
        PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(
            prCtx.stage1RuntimePsxMemoryProvider);
    AppendStage1XaCd80036AF8EventSourceAudit(
        out,
        xaCd,
        cdMmioSnapshotAudit,
        rawEventRuntimeWindow,
        rawEventInitialInterruptSourceResult,
        rawEventRuntimeSourceResult);
    out << " xaCdD4K=" << (xaCd.byte_800573D4Known ? 1 : 0)
        << " xaCdD4=" << static_cast<int>(xaCd.byte_800573D4)
        << " xaCdStatusObsK="
        << (xaCd.statusRead80036384Known ? 1 : 0)
        << " xaCdStatusObs=" << xaCd.statusRead80036384
        << " xaCdStatusObsAccepted="
        << xaCd.statusByte800573D4ObservationAcceptedCount
        << " xaCdStatusObsRejected="
        << xaCd.statusByte800573D4ObservationRejectedCount
        << " xaCdStatusObsReject="
        << static_cast<int>(xaCd.statusByte800573D4LastReject)
        << " xaCdF4K=" << (xaCd.byte_800493F4Known ? 1 : 0)
        << " xaCdF4M=" << static_cast<int>(xaCd.byte_800493F4.minute)
        << " xaCdF4S=" << static_cast<int>(xaCd.byte_800493F4.second)
        << " xaCdF4F=" << static_cast<int>(xaCd.byte_800493F4.frame)
        << " xaCdF4Producer=0x" << std::hex
        << xaCd.byte800493F4ProducerFunction << std::dec
        << " xaCdClockK="
        << (xaCdClockProbe.carrier.clockKnown ? 1 : 0)
        << " xaCdClockLba=" << xaCdClockProbe.carrier.clockLba
        << " xaCdClockGap="
        << (xaCdClockProbe.gapMissingStreamClock800493F4Producer ? 1 : 0)
        << " xaHalGetlocPCount=" << xaCd.halGetlocPFactsApplyCount
        << " xaCurrentPhysicalGetlocPProbeDisabled="
        << (PrScn1::IsStage1XaCurrentPhysicalGetlocPProbeDisabled() ? 1 : 0)
        << " xaHalGetlocPSource="
        << static_cast<int>(xaCd.lastHalGetlocPSource)
        << " xaHalGetlocPSectorK="
        << (xaCd.lastHalGetlocPSectorIndexKnown ? 1 : 0)
        << " xaHalGetlocPSector=" << xaCd.lastHalGetlocPSectorIndex
        << " xaHostClockK=" << (xaHostClockKnown ? 1 : 0)
        << " xaPlayerSelK=" << (xaPlayerSelectedFilterKnown ? 1 : 0)
        << " xaPlayerSelFile="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetSelectedFile()) : 0)
        << " xaPlayerSelCh="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetSelectedChannel()) : 0)
        << " xaPlayerSelCodingK=" << (xaPlayerSelectedCodingKnown ? 1 : 0)
        << " xaPlayerSelCoding="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetSelectedCoding()) : 0)
        << " xaPlayerSetFilterCount="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetSetFilterChangeCount() : 0u)
        << " xaPlayerLastSetFilterK="
        << (prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastSetFilterKnown() ? 1 : 0)
        << " xaPlayerLastSetFilterSector="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastSetFilterSectorIndex() : 0u)
        << " xaPlayerLastSetFilterFile="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastSetFilterFile()) : 0)
        << " xaPlayerLastSetFilterCh="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastSetFilterChannel()) : 0)
        << " xaTailReqK=" << (tailHost.known ? 1 : 0)
        << " xaTailReq=" << (tailHost.requestXaSetFilter13 ? 1 : 0)
        << " xaTailReqArg=" << static_cast<int>(tailHost.xaSetFilter13Arg)
        << " xaTailReqCount=" << tailHost.xaSetFilter13RequestCount
        << " xaTailLastReqK="
        << (tailHost.xaSetFilter13LastRequestKnown ? 1 : 0)
        << " xaTailLastReqQ=" << tailHost.xaSetFilter13LastRequestQueryFrame
        << " xaTailLastReqTick96="
        << tailHost.xaSetFilter13LastRequestTick96
        << " xaTailLastReqFlag0200="
        << (tailHost.xaSetFilter13LastRequestFlag0200Pulse ? 1 : 0)
        << " xaTailLastReqRow="
        << static_cast<int>(tailHost.xaSetFilter13LastRequestRow)
        << " xaTailLastReqArg="
        << static_cast<int>(tailHost.xaSetFilter13LastRequestArg)
        << " xaPlayerCurSector=" << xaPlayerCurrentSector
        << " xaPlayerRawReadCount="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRawReadCount() : 0u)
        << " xaPlayerLastRawReadK="
        << (prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastRawReadSectorKnown() ? 1 : 0)
        << " xaPlayerLastRawReadSector="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastRawReadSectorIndex() : 0u)
        << " xaPlayerAccPushCount=" << xaPlayerAcceptedPushCount
        << " xaPlayerAccPopCount="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetAcceptedRawPopCount() : 0u)
        << " xaPlayerAccQueue=" << xaPlayerAcceptedQueueSize
        << " xaPlayerLastPollK="
        << (prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastPolledSectorKnown() ? 1 : 0)
        << " xaPlayerLastPollSector="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastPolledSectorIndex() : 0u)
        << " xaPlayerLastAccK=" << (xaPlayerLastAcceptedSectorKnown ? 1 : 0)
        << " xaPlayerLastAccSector="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastAcceptedSectorIndex() : 0u)
        << " xaPlayerLastAccFile="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastAcceptedFile()) : 0)
        << " xaPlayerLastAccCh="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastAcceptedChannel()) : 0)
        << " xaPlayerLastAccCoding="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastAcceptedCoding()) : 0)
        << " xaPlayerLastRejectK="
        << (prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastFilterRejectKnown() ? 1 : 0)
        << " xaPlayerLastRejectSector="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastFilterRejectSectorIndex() : 0u)
        << " xaPlayerLastRejectFile="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastFilterRejectFile()) : 0)
        << " xaPlayerLastRejectCh="
        << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastFilterRejectChannel()) : 0)
        << " xaPlayerRecentRead0="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentRawReadSector(0) : 0u)
        << " xaPlayerRecentRead1="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentRawReadSector(1) : 0u)
        << " xaPlayerRecentRead2="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentRawReadSector(2) : 0u)
        << " xaPlayerRecentRead3="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentRawReadSector(3) : 0u)
        << " xaPlayerRecentAcc0="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentAcceptedSector(0) : 0u)
        << " xaPlayerRecentAcc1="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentAcceptedSector(1) : 0u)
        << " xaPlayerRecentAcc2="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentAcceptedSector(2) : 0u)
        << " xaPlayerRecentAcc3="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentAcceptedSector(3) : 0u)
        << " xaPlayerRecentPoll0="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentPolledSector(0) : 0u)
        << " xaPlayerRecentPoll1="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentPolledSector(1) : 0u)
        << " xaPlayerRecentPoll2="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentPolledSector(2) : 0u)
        << " xaPlayerRecentPoll3="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentPolledSector(3) : 0u)
        << " xaPlayerRecentReject0="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentFilterRejectSector(0) : 0u)
        << " xaPlayerRecentReject1="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentFilterRejectSector(1) : 0u)
        << " xaPlayerRecentReject2="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentFilterRejectSector(2) : 0u)
        << " xaPlayerRecentReject3="
        << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentFilterRejectSector(3) : 0u)
        << " xaHostClockRel75=" << xaHostClockRel75
        << " xaHostClockAbsLba=" << xaHostClockAbsLba
        << " xaHostAudibleRel75=" << xaHostAudibleRel75
        << " xaHostAudibleAbsLba=" << xaHostAudibleAbsLba
        << " xaCdA210Dispatch="
        << xaCd.cdSyncCallback8001A210DispatchCount
        << " xaCdSync37070Gap=" << xaCd.cdSyncCallback80037070GapCount
        << " xaCdAsync37070Gap=" << xaCd.cdAsyncCallback80037070GapCount
        << " xaCdEvtSerial=" << xaCd.cdLowerEvent80036AF8Serial
        << " xaCdEvtDispatched="
        << xaCd.cdLowerEvent80036AF8DispatchedSerial
        << " xaCdRet36AF8K="
        << (xaCd.cdLowerEventPsxReturn80036AF8Known ? 1 : 0)
        << " xaCdRet36AF8=" << xaCd.cdLowerEventPsxReturn80036AF8
        << " xaCdCb570F8K=" << (xaCd.dword_800570F8Known ? 1 : 0)
        << " xaCdCb570F8=0x" << std::hex << xaCd.dword_800570F8
        << " xaCdCb570FCK=" << std::dec
        << (xaCd.dword_800570FCKnown ? 1 : 0)
        << " xaCdCb570FC=0x" << std::hex << xaCd.dword_800570FC
        << std::dec
        << " xaCdCb570FCWrites="
        << xaCd.readyCallbackRegisterWriteCount
        << " xaCdCb570FCClears="
        << xaCd.readyCallbackRegisterClearCount
        << " xaCdCb570FCInstall39240="
        << xaCd.readyCallbackRegisterInstall39240Count
        << " xaCdCb570FCSource=0x" << std::hex
        << xaCd.readyCallbackRegisterSourceFunction
        << std::dec
        << " xaCdPend359B8Writes="
        << xaCd.cdCallbackPending800359B8WriteCount
        << " xaCdPend359B8Gaps="
        << xaCd.cdCallbackPending800359B8GapCount
        << " xaCd359B8ObsAccepted="
        << xaCd.interruptSnapshot800359B8ObservationAcceptedCount
        << " xaCd359B8ObsRejected="
        << xaCd.interruptSnapshot800359B8ObservationRejectedCount
        << " xaCd359B8ObsReject="
        << static_cast<int>(
               xaCd.interruptSnapshot800359B8ObservationLastReject)
        << " xaCd359B8ObsCbStateK="
        << (xaCd.interruptSnapshot800359B8LastCallbackStateKnown ? 1 : 0)
        << " xaCd359B8ObsInitialRegsK="
        << (xaCd.interruptSnapshot800359B8LastInitialRegsKnown ? 1 : 0)
        << " xaCd359B8ObsTerminalRegsK="
        << (xaCd.interruptSnapshot800359B8LastTerminalRegsKnown ? 1 : 0)
        << " xaCd359B8ObsWatchdogK="
        << (xaCd.interruptSnapshot800359B8LastWatchdogKnown ? 1 : 0)
        << " xaCd359B8ObsBundleK="
        << (xaCd.interruptSnapshot800359B8LastBundleKnown ? 1 : 0)
        << " xaCdPend359B8IntStatusK="
        << (xaCd.cdCallbackPending800359B8InterruptStatusKnown ? 1 : 0)
        << " xaCdPend359B8IntStatus="
        << xaCd.cdCallbackPending800359B8InterruptStatus
        << " xaCdPend359B8IntMaskK="
        << (xaCd.cdCallbackPending800359B8InterruptMaskKnown ? 1 : 0)
        << " xaCdPend359B8IntMask="
        << xaCd.cdCallbackPending800359B8InterruptMask
        << " xaCdPend359B8Word55F78K="
        << (xaCd.word_80055F78Known ? 1 : 0)
        << " xaCdPend359B8Word55F78="
        << xaCd.word_80055F78
        << " xaCdPend359B8Word55F7AK="
        << (xaCd.word_80055F7AKnown ? 1 : 0)
        << " xaCdPend359B8Word55F7A="
        << xaCd.word_80055F7A
        << " xaCdPend359B8Word55FA8K="
        << (xaCd.word_80055FA8Known ? 1 : 0)
        << " xaCdPend359B8Word55FA8="
        << xaCd.word_80055FA8
        << " xaCdPend359B8Watchdog57010K="
        << (xaCd.dword_80057010Known ? 1 : 0)
        << " xaCdPend359B8Watchdog57010="
        << xaCd.dword_80057010
        << " xaCdPend359B8Acks="
        << xaCd.cdCallbackPending800359B8AckCount
        << " xaCdPend359B8Dispatch="
        << xaCd.cdCallbackPending800359B8CallbackDispatchCount
        << " xaCdLowerSnapApply="
        << xaCd.lowerCdSnapshotApplyCount
        << " xaCdLowerSnapPending="
        << xaCd.lowerCdSnapshotPendingProducerCount
        << " xaCdLowerSnapEvent="
        << xaCd.lowerCdSnapshotCallbackEventCount
        << " xaCdLowerSnapSync="
        << xaCd.lowerCdSnapshotSyncFeedbackCount
        << " xaCdLowerSnapSeam="
        << xaCd.lowerCdSnapshotSeamResultCount
        << " xaCdGetlocBridge="
        << xaCd.halGetlocLowerBridgeCount
        << " xaCdGetlocBridgeCore="
        << xaCd.halGetlocLowerBridgeCdSyncCoreCount
        << " xaCdGetlocBridgePending="
        << xaCd.halGetlocLowerBridgePendingProducerCount
        << " xaCdGetlocBridgeEvent="
        << xaCd.halGetlocLowerBridgeCallbackEventCount;
    AppendStage1Bootstrap15590CdLowerObserverSurface(
        out,
        hostBlockKnown,
        hostBlock,
        xaCd.readS27Serial);
    out << " tcPrevXa=" << timecode.lastResult.previousXaReadValue801D303C
        << " tcXa=" << timecode.lastResult.xaSectorReadValueA7A4
        << " tcFallback="
        << (timecode.lastResult.usedFallbackField352Increment ? 1 : 0)
        << " tc348=" << timecode.lastResult.sceneEntryField348
        << " tc352="
        << timecode.lastResult.sceneEntryField352FallbackTickAdvance
        << " tc356=" << timecode.lastResult.sceneEntryField356TickOffset
        << " tcProduct=" << timecode.lastResult.xaProduct348ByReadValue32
        << " tcRound=" << timecode.lastResult.xaRoundedTickBeforeOffset
        << " tcCbRet=" << timecode.lastResult.callbackReturnValue
        << " intra384=" << numeric.acceptedProducerCarrier.phase384
        << " cadenceThreshold=0"
        << " cadenceMode=runtime"
        << " topLessonPageOrdinalAvailable="
        << (descriptorCadence.pageOrdinal56Available ? 1 : 0)
        << " topLessonPageOrdinal="
        << descriptorCadence.pageOrdinal56
        << " page56K="
        << (descriptorCadence.pageOrdinal56Available ? 1 : 0)
        << " page56=" << descriptorCadence.pageOrdinal56
        << " cadenceCursorAvailable="
        << (descriptorCadence.cadenceCursorAvailable ? 1 : 0)
        << " cadK="
        << (descriptorCadence.cadenceCursorAvailable ? 1 : 0)
        << " currentDescriptor40Available="
        << (descriptorCadence.currentDescriptor40Available ? 1 : 0)
        << " currentDescriptor40Index="
        << descriptorCadence.currentDescriptor40RowIndex
        << " desc40K="
        << (descriptorCadence.currentDescriptor40Available ? 1 : 0)
        << " desc40Idx="
        << descriptorCadence.currentDescriptor40RowIndex
        << " rightRankDescriptorFlagWord08Known="
        << (numeric.rightRankDescriptorFlagWord08Known ? 1 : 0)
        << " rightRankDescriptorFlagWord08="
        << numeric.rightRankDescriptorFlagWord08
        << " rr08K="
        << (numeric.rightRankDescriptorFlagWord08Known ? 1 : 0)
        << " rr08=" << numeric.rightRankDescriptorFlagWord08
        << " currentPageOrdinal1Based=" << currentPageOrdinal1Based
        << " curPage1=" << currentPageOrdinal1Based
        << " rankRow="
        << (int)rightRank.rightRankActiveRow
        << " rankBlinkTarget=" << (int)rightRank.rightRankBlinkTargetRow
        << " rankBlink=" << (rightRank.rightRankBlinkEnabled ? 1 : 0)
        << " resolutionKnown=" << (numeric.bucket30ResolutionKnown ? 1 : 0)
        << " resolutionV22=" << (int)numeric.bucket30ResolutionV22
        << " rankWritebackCommitted="
        << (numeric.bucket30RightRankWritebackCommitted ? 1 : 0)
        << " producerFormalWritebackKnown=" << (numeric.active ? 1 : 0)
        << " producerFormalWriteback24="
        << observer24.formalWritebackValue24
        << " cachedPhase1Classifier36="
        << (int)phase1Owner.cachedPhase1Classifier36
        << " phase1LatchArmed38="
        << (phase1Owner.phase1LatchArmed38 ? 1 : 0)
        << " bucket30OwnerBusyGateActive="
        << (ownerObserver.busyGateActive ? 1 : 0)
        << " bucket30OwnerProcessDescriptorFlagWord="
        << ownerObserver.processDescriptorFlagWord
        << " bucket30OwnerScorerCommitWindowOpen="
        << (ownerObserver.scorerCommitWindowOpen ? 1 : 0)
        << " bucket30OwnerNoInputCounterAdvanceRan="
        << (ownerObserver.noInputCounterAdvanceRan ? 1 : 0)
        << " bucket30OwnerNoInputCounterAcceptedCountInput="
        << ownerObserver.noInputCounterAcceptedCountInput
        << " bucket30OwnerNoInputCounterInput="
        << ownerObserver.noInputCounterInput
        << " bucket30OwnerNoInputCounterOutput="
        << ownerObserver.noInputCounterOutput
        << " bucket30OwnerNoInputCounterIncremented="
        << (ownerObserver.noInputCounterIncremented ? 1 : 0)
        << " bucket30OwnerKernelOpen="
        << (ownerObserver.kernelOpen ? 1 : 0)
        << " bucket30OwnerKernelEntered="
        << (ownerObserver.kernelEntered ? 1 : 0)
        << " bucket30OwnerDescriptorFlagWord="
        << ownerObserver.descriptorFlagWord
        << " bucket30OwnerActiveRow="
        << (int)ownerObserver.activeRow
        << " bucket30OwnerPrePhase1Classifier36="
        << (int)ownerObserver.prePhase1Classifier36
        << " bucket30OwnerPrePhase1LatchArmed38="
        << (ownerObserver.prePhase1LatchArmed38 ? 1 : 0)
        << " bucket30OwnerPhase1AdvanceCalled="
        << (ownerObserver.phase1AdvanceCalled ? 1 : 0)
        << " bucket30OwnerPhase1AdvanceSampledClassifier="
        << (int)ownerObserver.phase1AdvanceSampledClassifier
        << " bucket30OwnerPhase1AdvanceFirstBeat="
        << (ownerObserver.phase1AdvanceFirstBeat ? 1 : 0)
        << " bucket30GameplayCurrentBranchKnown="
        << (ownerObserver.gameplayCurrentBranchKnown ? 1 : 0)
        << " gBranchK="
        << (ownerObserver.gameplayCurrentBranchKnown ? 1 : 0)
        << " bucket30GameplayCurrentBranchSpecial="
        << (ownerObserver.gameplayCurrentBranchSpecial ? 1 : 0)
        << " gSpec="
        << (ownerObserver.gameplayCurrentBranchSpecial ? 1 : 0)
        << " bucket30GameplayAcceptedCountKnown="
        << (ownerObserver.gameplayAcceptedCountKnown ? 1 : 0)
        << " gAccK="
        << (ownerObserver.gameplayAcceptedCountKnown ? 1 : 0)
        << " bucket30GameplayAcceptedCount="
        << ownerObserver.gameplayAcceptedCount
        << " gAcc=" << ownerObserver.gameplayAcceptedCount
        << " bucket30GameplayLookbackPageCountKnown="
        << (ownerObserver.gameplayLookbackPageCountKnown ? 1 : 0)
        << " gLookK="
        << (ownerObserver.gameplayLookbackPageCountKnown ? 1 : 0)
        << " bucket30GameplayLookbackPageCount="
        << ownerObserver.gameplayLookbackPageCount
        << " gLook=" << ownerObserver.gameplayLookbackPageCount
        << " bucket30GameplayOverflowActiveKnown="
        << (ownerObserver.gameplayOverflowActiveKnown ? 1 : 0)
        << " gOvK="
        << (ownerObserver.gameplayOverflowActiveKnown ? 1 : 0)
        << " bucket30GameplayOverflowActive="
        << (ownerObserver.gameplayOverflowActive ? 1 : 0)
        << " gOv=" << (ownerObserver.gameplayOverflowActive ? 1 : 0)
        << " bucket30GameplayBranchCountKnown="
        << (ownerObserver.gameplayBranchCountKnown ? 1 : 0)
        << " gCountK="
        << (ownerObserver.gameplayBranchCountKnown ? 1 : 0)
        << " bucket30GameplayBranchCount="
        << ownerObserver.gameplayBranchCount
        << " gCount=" << ownerObserver.gameplayBranchCount
        << " bucket30GameplayDescriptorSubdeltaKnown="
        << (ownerObserver.gameplayDescriptorSubdeltaKnown ? 1 : 0)
        << " gSubK=" << (ownerObserver.gameplayDescriptorSubdeltaKnown ? 1 : 0)
        << " bucket30GameplayDescriptorSubdelta="
        << ownerObserver.gameplayDescriptorSubdelta
        << " gSub=" << ownerObserver.gameplayDescriptorSubdelta
        << " bucket30GameplayReaderPageOrdinalKnown="
        << (ownerObserver.gameplayReaderPageOrdinalKnown ? 1 : 0)
        << " gPageK=" << (ownerObserver.gameplayReaderPageOrdinalKnown ? 1 : 0)
        << " bucket30GameplayReaderPageOrdinal="
        << ownerObserver.gameplayReaderPageOrdinal
        << " gPage=" << ownerObserver.gameplayReaderPageOrdinal
        << " bucket30GameplayRequiredMaskKnown="
        << (ownerObserver.gameplayRequiredMaskKnown ? 1 : 0)
        << " gReqK=" << (ownerObserver.gameplayRequiredMaskKnown ? 1 : 0)
        << " bucket30GameplayRequiredMask="
        << ownerObserver.gameplayRequiredMask
        << " gReq=" << ownerObserver.gameplayRequiredMask
        << " bucket30GameplayUnionMaskKnown="
        << (ownerObserver.gameplayUnionMaskKnown ? 1 : 0)
        << " gUnionK=" << (ownerObserver.gameplayUnionMaskKnown ? 1 : 0)
        << " bucket30GameplayUnionMask="
        << ownerObserver.gameplayUnionMask
        << " gUnion=" << ownerObserver.gameplayUnionMask
        << " bucket30GameplayAnchorSlotIndexKnown="
        << (ownerObserver.gameplayAnchorSlotIndexKnown ? 1 : 0)
        << " gAnchorIdxK="
        << (ownerObserver.gameplayAnchorSlotIndexKnown ? 1 : 0)
        << " bucket30GameplayAnchorSlotIndex="
        << ownerObserver.gameplayAnchorSlotIndex
        << " gAnchorIdx=" << ownerObserver.gameplayAnchorSlotIndex
        << " bucket30GameplayRequiredClassTokenKnown="
        << (ownerObserver.gameplayRequiredClassTokenKnown ? 1 : 0)
        << " gReqClsK="
        << (ownerObserver.gameplayRequiredClassTokenKnown ? 1 : 0)
        << " bucket30GameplayRequiredClassToken="
        << ownerObserver.gameplayRequiredClassToken
        << " gReqCls=" << ownerObserver.gameplayRequiredClassToken
        << " bucket30GameplayAnchorSlotClassTokenKnown="
        << (ownerObserver.gameplayAnchorSlotClassTokenKnown ? 1 : 0)
        << " gAnchorClsK="
        << (ownerObserver.gameplayAnchorSlotClassTokenKnown ? 1 : 0)
        << " bucket30GameplayAnchorSlotClassToken="
        << ownerObserver.gameplayAnchorSlotClassToken
        << " gAnchorCls=" << ownerObserver.gameplayAnchorSlotClassToken
        << " bucket30GameplayAnchorSlotOccupiedKnown="
        << (ownerObserver.gameplayAnchorSlotOccupiedKnown ? 1 : 0)
        << " gOccK=" << (ownerObserver.gameplayAnchorSlotOccupiedKnown ? 1 : 0)
        << " bucket30GameplayAnchorSlotOccupied="
        << ownerObserver.gameplayAnchorSlotOccupied
        << " gOcc=" << ownerObserver.gameplayAnchorSlotOccupied
        << " bucket30GameplayAnchorClassMatchKnown="
        << (ownerObserver.gameplayAnchorClassMatchKnown ? 1 : 0)
        << " gAnchorMatchK="
        << (ownerObserver.gameplayAnchorClassMatchKnown ? 1 : 0)
        << " bucket30GameplayAnchorClassMatch="
        << ownerObserver.gameplayAnchorClassMatch
        << " gAnchorMatch=" << ownerObserver.gameplayAnchorClassMatch
        << " bucket30GameplayPairBonusKnown="
        << (ownerObserver.gameplayPairBonusKnown ? 1 : 0)
        << " gPairK="
        << (ownerObserver.gameplayPairBonusKnown ? 1 : 0)
        << " bucket30GameplayPairBonus="
        << ownerObserver.gameplayPairBonus
        << " gPair=" << ownerObserver.gameplayPairBonus
        << " bucket30GameplaySpillPenaltyKnown="
        << (ownerObserver.gameplaySpillPenaltyKnown ? 1 : 0)
        << " gSpillK="
        << (ownerObserver.gameplaySpillPenaltyKnown ? 1 : 0)
        << " bucket30GameplaySpillPenalty="
        << ownerObserver.gameplaySpillPenalty
        << " gSpill=" << ownerObserver.gameplaySpillPenalty
        << " bucket30GameplayAdditiveTermKnown="
        << (ownerObserver.gameplayAdditiveTermKnown ? 1 : 0)
        << " gAddK="
        << (ownerObserver.gameplayAdditiveTermKnown ? 1 : 0)
        << " bucket30GameplayAdditiveTerm="
        << ownerObserver.gameplayAdditiveTerm
        << " gAdd=" << ownerObserver.gameplayAdditiveTerm
        << " bucket30GameplayCommitTermKnown="
        << (ownerObserver.gameplayCommitTermKnown ? 1 : 0)
        << " gCommitK="
        << (ownerObserver.gameplayCommitTermKnown ? 1 : 0)
        << " bucket30GameplayCommitTerm="
        << ownerObserver.gameplayCommitTerm
        << " gCommit=" << ownerObserver.gameplayCommitTerm
        << " bucket30GameplayScoreWritebackKnown="
        << (ownerObserver.gameplayScoreWritebackKnown ? 1 : 0)
        << " gWbK="
        << (ownerObserver.gameplayScoreWritebackKnown ? 1 : 0)
        << " bucket30GameplayScoreWriteback="
        << ownerObserver.gameplayScoreWriteback
        << " gWb=" << ownerObserver.gameplayScoreWriteback
        << " bucket30GameplayClampActiveKnown="
        << (ownerObserver.gameplayClampActiveKnown ? 1 : 0)
        << " gClampK="
        << (ownerObserver.gameplayClampActiveKnown ? 1 : 0)
        << " bucket30GameplayClampActive="
        << (ownerObserver.gameplayClampActive ? 1 : 0)
        << " gClamp=" << (ownerObserver.gameplayClampActive ? 1 : 0)
        << " bucket30OwnerPhase1PrevBaseline18="
        << ownerObserver.phase1PrevBaseline18
        << " bucket30OwnerPhase1LiveAccumulator91816="
        << ownerObserver.phase1LiveAccumulator91816
        << " bucket30OwnerPhase1Delta91816MinusPrev18="
        << ownerObserver.phase1Delta91816MinusPrev18
        << " bucket30OwnerProducerGrowthBaseline18="
        << ownerObserver.producerGrowthBaseline18
        << " bucket30OwnerBlinkBaseline18="
        << ownerObserver.blinkBaseline18
        << " bucket30OwnerHelperSnapshot18="
        << ownerObserver.helperSnapshot18
        << " bucket30OwnerAfterAdvancePhase1Classifier36="
        << (int)ownerObserver.afterAdvancePhase1Classifier36
        << " bucket30OwnerAfterAdvancePhase1LatchArmed38="
        << (ownerObserver.afterAdvancePhase1LatchArmed38 ? 1 : 0)
        << " bucket30OwnerFormalWritebackKnown="
        << (ownerObserver.formalWritebackKnown ? 1 : 0)
        << " bucket30OwnerFormalWritebackValue="
        << ownerObserver.formalWritebackValue
        << " bucket30OwnerTieBreakerCalled="
        << (ownerObserver.tieBreakerCalled ? 1 : 0)
        << " bucket30OwnerTieBreakerResult14548="
        << ownerObserver.tieBreakerResult14548
        << " bucket30OwnerResolverGateBit4="
        << (ownerObserver.resolverGateBit4 ? 1 : 0)
        << " bucket30OwnerResolverGateEd00Idle="
        << (ownerObserver.resolverGateEd00Idle ? 1 : 0)
        << " bucket30OwnerResolutionCalled="
        << (ownerObserver.resolutionCalled ? 1 : 0)
        << " bucket30OwnerResolutionKnown="
        << (ownerObserver.resolutionKnown ? 1 : 0)
        << " bucket30OwnerResolutionInputPhase1Classifier36="
        << (int)ownerObserver.resolutionInputPhase1Classifier36
        << " ownResIn36="
        << (int)ownerObserver.resolutionInputPhase1Classifier36
        << " bucket30OwnerResolutionV22="
        << (int)ownerObserver.resolutionV22
        << " bucket30OwnerGoodToCoolCommitted="
        << (ownerObserver.goodToCoolCommitted ? 1 : 0)
        << " bucket30OwnerAfterProducePhase1Classifier36="
        << (int)ownerObserver.afterProducePhase1Classifier36
        << " bucket30OwnerAfterProducePhase1LatchArmed38="
        << (ownerObserver.afterProducePhase1LatchArmed38 ? 1 : 0)
        << " bucket30OwnerAfterProduceGrowthBaseline18="
        << ownerObserver.afterProduceGrowthBaseline18
        << " bucket30OwnerAfterProduceBlinkBaseline18="
        << ownerObserver.afterProduceBlinkBaseline18
        << " bucket30OwnerAfterProduceHelperSnapshot18="
        << ownerObserver.afterProduceHelperSnapshot18
        << " accumulator91816=" << scorer.accumulator91816
        << " baseline91818=" << phase1Owner.baselineValue18
        << " snapshot9181A=" << tieBreaker.snapshot1A
        << " snapshot9181C=" << helperShadow.snapshot1C
        << " carry9181E=" << helperShadow.tieCarryLatch1E
        << " descriptorED08=" << rightRankDescriptorFlagWord
        << " phaseCacheED36=" << (int)phase1Owner.cachedPhase1Classifier36
        << " phaseCounterED38="
        << (phase1Owner.phase1LatchArmed38 ? 1 : 0)
        << " lastRowWriteKnown="
        << (numeric.rightRankLastRowWriteKnown ? 1 : 0)
        << " lastRowWriteQuery=" << numeric.rightRankLastRowWriteQueryFrame
        << " lastRowWritePrevRow="
        << (int)numeric.rightRankLastRowWritePrevRow
        << " lastRowWriteResolvedRow="
        << (int)numeric.rightRankLastRowWriteResolvedRow
        << " lastRowWriteResolutionKnown="
        << (numeric.rightRankLastRowWriteResolutionKnown ? 1 : 0)
        << " lastRowWriteResolutionV22="
        << (int)numeric.rightRankLastRowWriteResolutionV22
        << " lastRowWriteCommitted="
        << (numeric.rightRankLastRowWriteCommitted ? 1 : 0)
        << " firstRow3WriteKnown="
        << (numeric.rightRankFirstRow3WriteKnown ? 1 : 0)
        << " firstRow3WriteQuery="
        << numeric.rightRankFirstRow3WriteQueryFrame
        << " firstRow3WritePrevRow="
        << (int)numeric.rightRankFirstRow3WritePrevRow
        << " firstRow3WriteResolvedRow="
        << (int)numeric.rightRankFirstRow3WriteResolvedRow
        << " firstRow3WriteResolutionKnown="
        << (numeric.rightRankFirstRow3WriteResolutionKnown ? 1 : 0)
        << " firstRow3WriteResolutionV22="
        << (int)numeric.rightRankFirstRow3WriteResolutionV22
        << " firstRow3WriteCommitted="
        << (numeric.rightRankFirstRow3WriteCommitted ? 1 : 0)
        << " firstRow3WriteOwnerActiveRow="
        << (int)numeric.rightRankFirstRow3WriteOwnerActiveRow
        << " firstRow3WriteResolutionInputPhase1Classifier36="
        << (int)numeric.rightRankFirstRow3WriteResolutionInputPhase1Classifier36
        << " firstRow3WritePhase1AdvanceFirstBeat="
        << (numeric.rightRankFirstRow3WritePhase1AdvanceFirstBeat ? 1 : 0)
        << " firstRow3WritePhase1Delta91816MinusPrev18="
        << numeric.rightRankFirstRow3WritePhase1Delta91816MinusPrev18
        << " firstRow3WriteFormalWritebackKnown="
        << (numeric.rightRankFirstRow3WriteFormalWritebackKnown ? 1 : 0)
        << " firstRow3WriteFormalWritebackValue="
        << numeric.rightRankFirstRow3WriteFormalWritebackValue
        << " firstRow3WriteTieBreakerCalled="
        << (numeric.rightRankFirstRow3WriteTieBreakerCalled ? 1 : 0)
        << " firstRow3WriteTieBreakerResult14548="
        << numeric.rightRankFirstRow3WriteTieBreakerResult14548
        << " firstRow3WriteResolverGateBit4="
        << (numeric.rightRankFirstRow3WriteResolverGateBit4 ? 1 : 0)
        << " firstRow3WriteResolverGateEd00Idle="
        << (numeric.rightRankFirstRow3WriteResolverGateEd00Idle ? 1 : 0)
        << " firstRow3WriteGoodToCoolCommitted="
        << (numeric.rightRankFirstRow3WriteGoodToCoolCommitted ? 1 : 0)
        << " ctx6AConsumerGate="
        << (numeric.rightRankBucketContext.ctx6AConsumerGate ? 1 : 0)
        << " ctx6AConsumerGateInitKnown="
        << (numeric.rightRankBucketContext.ctx6AConsumerGateInitKnown ? 1 : 0)
        << " ctx6AConsumerGateInitQuery="
        << numeric.rightRankBucketContext.ctx6AConsumerGateInitQueryFrame
        << " ctx6AConsumerGateInitValue="
        << numeric.rightRankBucketContext.ctx6AConsumerGateInitValue
        << " firstNoInputCounterProducerKnown="
        << (numeric.rightRankFirstNoInputCounterProducerKnown ? 1 : 0)
        << " firstNoInputCounterProducerQuery="
        << numeric.rightRankFirstNoInputCounterProducerQueryFrame
        << " firstNoInputCounterProducerAfterAcceptedClear="
        << (numeric.rightRankFirstNoInputCounterProducerAfterAcceptedClear ? 1 : 0)
        << " firstNoInputCounterProducerDeltaAfterAcceptedClear="
        << numeric.rightRankFirstNoInputCounterProducerDeltaAfterAcceptedClear
        << " firstNoInputCounterProducerDeltaAfterLastAcceptedCountClear="
        << numeric.rightRankFirstNoInputCounterProducerDeltaAfterLastAcceptedCountClear
        << " firstNoInputCounterProducerAcceptedSourceAfterLastClearKnown="
        << (numeric.rightRankFirstNoInputCounterProducerAcceptedSourceAfterLastClearKnown ? 1 : 0)
        << " firstNoInputCounterProducerAcceptedSourceAfterLastClear="
        << (numeric.rightRankFirstNoInputCounterProducerAcceptedSourceAfterLastClear ? 1 : 0)
        << " firstNoInputCounterProducerActiveRow="
        << (int)numeric.rightRankFirstNoInputCounterProducerActiveRow
        << " firstNoInputCounterProducerDescriptorFlags="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorFlags
        << " firstNoInputCounterProducerDescriptorLowBits03="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorLowBits03
        << " firstNoInputCounterProducerBusyGateActive="
        << (numeric.rightRankFirstNoInputCounterProducerBusyGateActive ? 1 : 0)
        << " firstNoInputCounterProducerScorerWindowOpen="
        << (numeric.rightRankFirstNoInputCounterProducerScorerWindowOpen ? 1 : 0)
        << " firstNoInputCounterProducerDescriptorSubstate50="
        << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorSubstate50
        << " firstNoInputCounterProducerDescriptorCadenceCursorAvailable="
        << (numeric.rightRankFirstNoInputCounterProducerDescriptorCadenceCursorAvailable ? 1 : 0)
        << " firstNoInputCounterProducerDescriptorCadenceCursorOrdinal1Based="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorCadenceCursorOrdinal1Based
        << " firstNoInputCounterProducerDescriptorPageOrdinal56Available="
        << (numeric.rightRankFirstNoInputCounterProducerDescriptorPageOrdinal56Available ? 1 : 0)
        << " firstNoInputCounterProducerDescriptorPageOrdinal56="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorPageOrdinal56
        << " firstNoInputCounterProducerDescriptorCurrentCommittedAvailable="
        << (numeric.rightRankFirstNoInputCounterProducerDescriptorCurrentCommittedAvailable ? 1 : 0)
        << " firstNoInputCounterProducerDescriptorCurrentCommittedRowIndex="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorCurrentCommittedRowIndex
        << " firstNoInputCounterProducerDescriptorCurrentDescriptor40Available="
        << (numeric.rightRankFirstNoInputCounterProducerDescriptorCurrentDescriptor40Available ? 1 : 0)
        << " firstNoInputCounterProducerDescriptorCurrentDescriptor40RowIndex="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorCurrentDescriptor40RowIndex
        << " firstNoInputCounterProducerDescriptorRowAvailable="
        << (numeric.rightRankFirstNoInputCounterProducerDescriptorRowAvailable ? 1 : 0)
        << " firstNoInputCounterProducerDescriptorLessonId="
        << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorLessonId
        << " firstNoInputCounterProducerDescriptorDefaultFlagWord="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorDefaultFlagWord
        << " firstNoInputCounterProducerDescriptorSubstateFlagWord="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorSubstateFlagWord
        << " firstNoInputCounterProducerDescriptorRequiredMask="
        << numeric.rightRankFirstNoInputCounterProducerDescriptorRequiredMask
        << " firstNoInputCounterProducerDescriptorAnchorSlotIndex="
        << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorAnchorSlotIndex
        << " firstNoInputCounterProducerDescriptorRequiredClassToken="
        << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorRequiredClassToken
        << " firstNoInputCounterProducerDescriptorDefaultSelector0="
        << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorDefaultSelector0
        << " firstNoInputCounterProducerDescriptorDefaultSelector1="
        << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorDefaultSelector1
        << " firstNoInputCounterProducerDescriptorSubstateSelector0="
        << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorSubstateSelector0
        << " firstNoInputCounterProducerDescriptorSubstateSelector1="
        << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorSubstateSelector1
        << " firstNoInputCounterProducerCurrentBucket="
        << (int)numeric.rightRankFirstNoInputCounterProducerCurrentBucket
        << " firstNoInputCounterProducerPreviousBucket="
        << (int)numeric.rightRankFirstNoInputCounterProducerPreviousBucket
        << " firstNoInputCounterProducerBucket30Advanced="
        << (numeric.rightRankFirstNoInputCounterProducerBucket30Advanced ? 1 : 0)
        << " firstNoInputCounterProducerBucketAdvanceCount="
        << numeric.rightRankFirstNoInputCounterProducerBucketAdvanceCount
        << " firstNoInputCounterProducerAcceptedCountInput="
        << numeric.rightRankFirstNoInputCounterProducerAcceptedCountInput
        << " firstNoInputCounterProducerSteadyInputKnown="
        << (numeric.rightRankFirstNoInputCounterProducerSteadyInputKnown ? 1 : 0)
        << " firstNoInputCounterProducerSteadyInputHeldMask="
        << numeric.rightRankFirstNoInputCounterProducerSteadyInputHeldMask
        << " firstNoInputCounterProducerSteadyInputWriteCtx18="
        << (numeric.rightRankFirstNoInputCounterProducerSteadyInputWriteCtx18 ? 1 : 0)
        << " firstNoInputCounterProducerSteadyInputCtx18Value="
        << numeric.rightRankFirstNoInputCounterProducerSteadyInputCtx18Value
        << " firstNoInputCounterProducerSteadyInputWriteCtx20="
        << (numeric.rightRankFirstNoInputCounterProducerSteadyInputWriteCtx20 ? 1 : 0)
        << " firstNoInputCounterProducerSteadyInputCtx20Value="
        << numeric.rightRankFirstNoInputCounterProducerSteadyInputCtx20Value
        << " firstNoInputCounterProducerLocalHoldMask80035510="
        << numeric.rightRankFirstNoInputCounterProducerLocalHoldMask80035510
        << " firstNoInputCounterProducerLocalConsumedHoldMask80035510="
        << numeric.rightRankFirstNoInputCounterProducerLocalConsumedHoldMask80035510
        << " firstNoInputCounterProducerLocalDebounceBypassed80035510="
        << (numeric.rightRankFirstNoInputCounterProducerLocalDebounceBypassed80035510 ? 1 : 0)
        << " firstNoInputCounterProducerLastSteadyInputNonZeroKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastSteadyInputNonZeroQuery="
        << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroQueryFrame
        << " firstNoInputCounterProducerLastSteadyInputNonZeroDeltaToProducer="
        << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroDeltaToProducer
        << " firstNoInputCounterProducerLastSteadyInputNonZeroDeltaAfterLastAcceptedCountClear="
        << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroDeltaAfterLastAcceptedCountClear
        << " firstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClearKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClearKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClear="
        << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClear ? 1 : 0)
        << " firstNoInputCounterProducerLastSteadyInputNonZeroReplayMode52="
        << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroReplayMode52 ? 1 : 0)
        << " firstNoInputCounterProducerLastSteadyInputNonZeroHeldMask="
        << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroHeldMask
        << " firstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx18="
        << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx18 ? 1 : 0)
        << " firstNoInputCounterProducerLastSteadyInputNonZeroCtx18Value="
        << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroCtx18Value
        << " firstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx20="
        << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx20 ? 1 : 0)
        << " firstNoInputCounterProducerLastSteadyInputNonZeroCtx20Value="
        << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroCtx20Value
        << " firstNoInputCounterProducerLastSteadyInputNonZeroRequiredMaskOverlap="
        << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroRequiredMaskOverlap
        << " firstNoInputCounterProducerReplayMirrorKnown="
        << (numeric.rightRankFirstNoInputCounterProducerReplayMirrorKnown ? 1 : 0)
        << " firstNoInputCounterProducerReplayProducerKnown="
        << (numeric.rightRankFirstNoInputCounterProducerReplayProducerKnown ? 1 : 0)
        << " firstNoInputCounterProducerReplayBytesKnown="
        << (numeric.rightRankFirstNoInputCounterProducerReplayBytesKnown ? 1 : 0)
        << " firstNoInputCounterProducerReplayRequiredBytes="
        << numeric.rightRankFirstNoInputCounterProducerReplayRequiredBytes
        << " firstNoInputCounterProducerReplayKnownBytes="
        << numeric.rightRankFirstNoInputCounterProducerReplayKnownBytes
        << " firstNoInputCounterProducerReplayMissingBytes="
        << numeric.rightRankFirstNoInputCounterProducerReplayMissingBytes
        << " firstNoInputCounterProducerReplayFullBytes="
        << (numeric.rightRankFirstNoInputCounterProducerReplayFullBytes ? 1 : 0)
        << " firstNoInputCounterProducerReplayFullBackingKnown8008EEF8="
        << (numeric.rightRankFirstNoInputCounterProducerReplayFullBackingKnown8008EEF8 ? 1 : 0)
        << " lastReplayAppendKnown="
        << (numeric.rightRankLastReplayAppendKnown ? 1 : 0)
        << " lastReplayAppendQuery="
        << numeric.rightRankLastReplayAppendQueryFrame
        << " lastReplayAppendDeltaToHandoff="
        << (numeric.rightRankLastReplayAppendKnown
                ? query - numeric.rightRankLastReplayAppendQueryFrame
                : -1)
        << " lastReplayAppendKnownBytes="
        << numeric.rightRankLastReplayAppendKnownBytes
        << " lastReplayAppendMissingBytes="
        << numeric.rightRankLastReplayAppendMissingBytes
        << " lastReplayAppendPublishedCount="
        << numeric.rightRankLastReplayAppendPublishedCount
        << " lastReplayAppendWriteCount="
        << numeric.rightRankLastReplayAppendWriteCount
        << " lastReplayAppendFullBytes="
        << (numeric.rightRankLastReplayAppendFullBytes ? 1 : 0)
        << " lastReplayAppendFullBackingKnown8008EEF8="
        << (numeric.rightRankLastReplayAppendFullBackingKnown8008EEF8 ? 1 : 0)
        << " lastAcceptedTailDecisionKnown="
        << (numeric.rightRankLastAcceptedTailDecisionKnown ? 1 : 0)
        << " lastAcceptedTailDecisionQuery="
        << numeric.rightRankLastAcceptedTailDecisionQueryFrame
        << " lastAcceptedTailDecisionDeltaToHandoff="
        << (numeric.rightRankLastAcceptedTailDecisionKnown
                ? query - numeric.rightRankLastAcceptedTailDecisionQueryFrame
                : -1)
        << " lastAcceptedTailDecisionCtxInput18="
        << numeric.rightRankLastAcceptedTailDecisionCtxInput18
        << " lastAcceptedTailDecisionAcceptedMask9FF="
        << numeric.rightRankLastAcceptedTailDecisionAcceptedMask9FF
        << " lastAcceptedTailDecisionGateOpen="
        << (numeric.rightRankLastAcceptedTailDecisionGateOpen ? 1 : 0)
        << " lastAcceptedTailDecisionMaskChanged="
        << (numeric.rightRankLastAcceptedTailDecisionMaskChanged ? 1 : 0)
        << " lastAcceptedTailDecisionCall14614="
        << (numeric.rightRankLastAcceptedTailDecisionCall14614 ? 1 : 0)
        << " lastAcceptedTailDecisionReplayMode52="
        << (numeric.rightRankLastAcceptedTailDecisionReplayMode52 ? 1 : 0)
        << " acceptedCarrierAvailable="
        << (acceptedCarrier.available ? 1 : 0)
        << " acceptedCarrierSourceKind="
        << (int)acceptedCarrier.controlWriterSourceKind
        << " acceptedCarrierRawControl18="
        << acceptedCarrier.rawControlSample18
        << " acceptedCarrierControlMask18="
        << acceptedCarrier.controlMask18
        << " acceptedCarrierClassToken20="
        << (int)acceptedCarrier.classToken20
        << " acceptedCarrierStreamFlagKnown="
        << (acceptedCarrier.eventStreamFlagKnown ? 1 : 0)
        << " acceptedCarrierStreamFlag="
        << (acceptedCarrier.eventStreamFlagActive ? 1 : 0)
        << " acceptedCarrierStreamIdKnown="
        << (acceptedCarrier.eventStreamIdRawKnown ? 1 : 0)
        << " acceptedCarrierStreamIdRaw="
        << (int)acceptedCarrier.eventStreamIdRaw
        << " acceptedCarrierBusyGateKnown="
        << (acceptedCarrier.busyGate24BF4Known ? 1 : 0)
        << " acceptedCarrierBusyGate="
        << (acceptedCarrier.busyGate24BF4Active ? 1 : 0)
        << " acceptedCarrierAcceptedGateKnown="
        << (acceptedCarrier.acceptedGateKnown ? 1 : 0)
        << " acceptedCarrierAcceptedGate="
        << (acceptedCarrier.acceptedGateActive ? 1 : 0)
        << " acceptedCarrierTick96Known="
        << (acceptedCarrier.acceptedTick96Known ? 1 : 0)
        << " acceptedCarrierTick96="
        << acceptedCarrier.acceptedTick96
        << " lastAcceptedTailCallKnown="
        << (numeric.rightRankLastAcceptedTailCallKnown ? 1 : 0)
        << " lastAcceptedTailCallQuery="
        << numeric.rightRankLastAcceptedTailCallQueryFrame
        << " lastAcceptedTailCallDeltaToHandoff="
        << (numeric.rightRankLastAcceptedTailCallKnown
                ? query - numeric.rightRankLastAcceptedTailCallQueryFrame
                : -1)
        << " lastAcceptedTailCallAcceptedMask9FF="
        << numeric.rightRankLastAcceptedTailCallAcceptedMask9FF
        << " lastAcceptedTailCallDirectRunCaptured="
        << (numeric.rightRankLastAcceptedTailCallDirectRunCaptured ? 1 : 0)
        << " lastAcceptedTailCallDirectRunResult="
        << numeric.rightRankLastAcceptedTailCallDirectRunResult
        << " lastAcceptedTailCallReplayAppend="
        << (numeric.rightRankLastAcceptedTailCallReplayAppend ? 1 : 0)
        << " lastAcceptedTailCallTimingTemplateKnown="
        << (numeric.rightRankLastAcceptedTailCallTimingTemplateKnown ? 1 : 0)
        << " lastAcceptedTailCallTimingTemplateSlot48="
        << (int)numeric.rightRankLastAcceptedTailCallTimingTemplateSlot48
        << " lastAcceptedTailCallTimingTemplateState="
        << (int)numeric.rightRankLastAcceptedTailCallTimingTemplateState
        << " lastAcceptedTailCallAcceptedTick96Known="
        << (numeric.rightRankLastAcceptedTailCallAcceptedTick96Known ? 1 : 0)
        << " lastAcceptedTailCallAcceptedTick96="
        << numeric.rightRankLastAcceptedTailCallAcceptedTick96
        << " lastAcceptedTailCallPhase384="
        << numeric.rightRankLastAcceptedTailCallPhase384
        << " lastAcceptedTailCallRecordSlot24="
        << (int)numeric.rightRankLastAcceptedTailCallRecordSlot24
        << " lastAcceptedTailCallRecordRemainder24="
        << (int)numeric.rightRankLastAcceptedTailCallRecordRemainder24
        << " lastAcceptedTailCallSourceCellValid="
        << (numeric.rightRankLastAcceptedTailCallSourceCellValid ? 1 : 0)
        << " firstNoInputCounterProducerLastReplayAppendKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastReplayAppendQuery="
        << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendQueryFrame
        << " firstNoInputCounterProducerLastReplayAppendDeltaAfterLastAcceptedCountClear="
        << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendDeltaAfterLastAcceptedCountClear
        << " firstNoInputCounterProducerLastReplayAppendAfterLastClearKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendAfterLastClearKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastReplayAppendAfterLastClear="
        << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendAfterLastClear ? 1 : 0)
        << " firstNoInputCounterProducerLastReplayAppendKnownBytes="
        << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendKnownBytes
        << " firstNoInputCounterProducerLastReplayAppendMissingBytes="
        << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendMissingBytes
        << " firstNoInputCounterProducerLastReplayAppendPublishedCount="
        << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendPublishedCount
        << " firstNoInputCounterProducerLastReplayAppendWriteCount="
        << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendWriteCount
        << " firstNoInputCounterProducerLastReplayAppendFullBytes="
        << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendFullBytes ? 1 : 0)
        << " firstNoInputCounterProducerLastReplayAppendFullBackingKnown8008EEF8="
        << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendFullBackingKnown8008EEF8 ? 1 : 0)
        << " acceptedReplayRestoreObserved="
        << (numeric.rightRankAcceptedReplayRestoreObserved ? 1 : 0)
        << " acceptedReplayRestorePayloadBackupValid="
        << (numeric.rightRankAcceptedReplayRestorePayloadBackupValid ? 1 : 0)
        << " acceptedReplayRestorePayloadFullBackingKnown8008EEF8="
        << (numeric.rightRankAcceptedReplayRestorePayloadFullBackingKnown8008EEF8 ? 1 : 0)
        << " acceptedReplayRestoreSidecarBackupValid="
        << (numeric.rightRankAcceptedReplayRestoreSidecarBackupValid ? 1 : 0)
        << " acceptedReplayRestoreSidecarFullBackingKnown8008EEF8="
        << (numeric.rightRankAcceptedReplayRestoreSidecarFullBackingKnown8008EEF8 ? 1 : 0)
        << " acceptedReplayRestoreSource1681C="
        << static_cast<int>(numeric.rightRankAcceptedReplayRestoreSource1681C)
        << " acceptedReplayRestoreRequested1681C="
        << (numeric.rightRankAcceptedReplayRestoreRequested1681C ? 1 : 0)
        << " acceptedReplayRestoreApplied1681C="
        << (numeric.rightRankAcceptedReplayRestoreApplied1681C ? 1 : 0)
        << " acceptedReplaySetupTransitionState="
        << numeric.rightRankAcceptedReplaySetupTransitionState
        << " acceptedReplayEventTableSeed801C8660Applied="
        << (numeric.rightRankAcceptedReplayEventTableSeed801C8660Applied ? 1 : 0)
        << " acceptedReplayRestorePostKnownBytes="
        << numeric.rightRankAcceptedReplayRestorePostKnownBytes
        << " acceptedReplayRestorePostFullBytes="
        << (numeric.rightRankAcceptedReplayRestorePostFullBytes ? 1 : 0)
        << " acceptedReplayRestorePostFullBackingKnown8008EEF8="
        << (numeric.rightRankAcceptedReplayRestorePostFullBackingKnown8008EEF8 ? 1 : 0)
        << Stage1OverlayDescribeAcceptedReplayPayloadGate(
               numeric.rightRankAcceptedReplayRestorePayloadGate)
        << Stage1OverlayDescribeSavePayloadBankRuntime(savePayloadBankRuntime)
        << " firstNoInputCounterProducerLastAcceptedTailDecisionKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailDecisionQuery="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionQueryFrame
        << " firstNoInputCounterProducerLastAcceptedTailDecisionCtxInput18="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionCtxInput18
        << " firstNoInputCounterProducerLastAcceptedTailDecisionPreviousInputMask801CCBB8="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionPreviousInputMask801CCBB8
        << " firstNoInputCounterProducerLastAcceptedTailDecisionAcceptedMask9FF="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionAcceptedMask9FF
        << " firstNoInputCounterProducerLastAcceptedTailDecisionGateOpen="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionGateOpen ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailDecisionMaskChanged="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionMaskChanged ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailDecisionCall14614="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionCall14614 ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailDecisionReplayMode52="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionReplayMode52 ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallQuery="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallQueryFrame
        << " firstNoInputCounterProducerLastAcceptedTailCallDeltaAfterLastAcceptedCountClear="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallDeltaAfterLastAcceptedCountClear
        << " firstNoInputCounterProducerLastAcceptedTailCallAfterLastClearKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAfterLastClearKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallAfterLastClear="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAfterLastClear ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallCtxInput18="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallCtxInput18
        << " firstNoInputCounterProducerLastAcceptedTailCallPreviousInputMask801CCBB8="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallPreviousInputMask801CCBB8
        << " firstNoInputCounterProducerLastAcceptedTailCallAcceptedMask9FF="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedMask9FF
        << " firstNoInputCounterProducerLastAcceptedTailCallDirectRunCaptured="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallDirectRunCaptured ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallDirectRunResult="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallDirectRunResult
        << " firstNoInputCounterProducerLastAcceptedTailCallReplayAppend="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallReplayAppend ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallSelectorResolved="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorResolved ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallSelectorByte0="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorByte0
        << " firstNoInputCounterProducerLastAcceptedTailCallSelectorByte1="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorByte1
        << " firstNoInputCounterProducerLastAcceptedTailCallTimingTemplateKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallTimingTemplateSlot48="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateSlot48
        << " firstNoInputCounterProducerLastAcceptedTailCallTimingTemplateState="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateState
        << " firstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96Known="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96Known ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96
        << " firstNoInputCounterProducerLastAcceptedTailCallHalfWindow34="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallHalfWindow34
        << " firstNoInputCounterProducerLastAcceptedTailCallPhase384="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallPhase384
        << " firstNoInputCounterProducerLastAcceptedTailCallRecordSlot24="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallRecordSlot24
        << " firstNoInputCounterProducerLastAcceptedTailCallRecordRemainder24="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallRecordRemainder24
        << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderValid="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderValid ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderAddr=0x"
        << std::hex
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderAddr
        << std::dec
        << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderBasePtr=0x"
        << std::hex
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderBasePtr
        << std::dec
        << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCount="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCount
        << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCursor="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCursor
        << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellValid="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellValid ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountNonZeroKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountNonZeroQuery="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroQueryFrame
        << " firstNoInputCounterProducerLastAcceptedCountNonZeroValue="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroValue
        << " firstNoInputCounterProducerLastAcceptedCountNonZeroMask="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroMask
        << " firstNoInputCounterProducerLastAcceptedCountClearKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearQuery="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearQueryFrame
        << " firstNoInputCounterProducerLastAcceptedCountClearSourceBucket="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearSourceBucket
        << " firstNoInputCounterProducerLastAcceptedCountClearPreCount="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreCount
        << " firstNoInputCounterProducerLastAcceptedCountClearPreMask="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreMask
        << " firstNoInputCounterProducerLastAcceptedCountClearMask9180C="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearMask9180C
        << " firstNoInputCounterProducerLastAcceptedCountClearAction="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearAction
        << " firstNoInputCounterProducerLastAcceptedCountClearPreBucket30Ed00="
        << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreBucket30Ed00
        << " firstNoInputCounterProducerLastAcceptedCountClearDirectConsumer94400="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumer94400 ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearDirectConsumerImmediateFollowUpClear="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumerImmediateFollowUpClear ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearDirectConsumerOwnerNoResolution94400="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumerOwnerNoResolution94400 ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearWaitSecondBeatBucket30="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearWaitSecondBeatBucket30 ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearAcceptedTailSurvived="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearAcceptedTailSurvived ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearOwnerKernelOpen="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearOwnerKernelOpen ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearResolverGateBit4="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearResolverGateBit4 ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearResolutionGateEd00Idle="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearResolutionGateEd00Idle ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearPhase1LatchArmed38="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearPhase1LatchArmed38 ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearFollowUpPhaseIsNone="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearFollowUpPhaseIsNone ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionV22="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionV22
        << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteCommitted="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteCommitted ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteGoodToCoolCommitted="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteGoodToCoolCommitted ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteDirectConsumerFallback94400="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteDirectConsumerFallback94400 ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearDirectSlotKnown="
        << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectSlotKnown ? 1 : 0)
        << " firstNoInputCounterProducerLastAcceptedCountClearDirectSlot="
        << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectSlot
        << " firstNoInputCounterProducerInput="
        << numeric.rightRankFirstNoInputCounterProducerInput
        << " firstNoInputCounterProducerOutput="
        << numeric.rightRankFirstNoInputCounterProducerOutput
        << " firstNoInputCounterProducerIncremented="
        << (numeric.rightRankFirstNoInputCounterProducerIncremented ? 1 : 0)
        << " firstNoInputCounterProducerSameSliceClearAction="
        << (int)numeric.rightRankFirstNoInputCounterProducerSameSliceClearAction
        << " firstNoInputCounterProducerSameSliceClearRan="
        << (numeric.rightRankFirstNoInputCounterProducerSameSliceClearRan ? 1 : 0)
        << " firstNoInputCounterProducerBeforeSameSliceClear="
        << (numeric.rightRankFirstNoInputCounterProducerBeforeSameSliceClear ? 1 : 0)
        << " firstNoInputCounterProducerCtx6AInitKnown="
        << (numeric.rightRankFirstNoInputCounterProducerCtx6AInitKnown ? 1 : 0)
        << " firstNoInputCounterProducerCtx6AInitQuery="
        << numeric.rightRankFirstNoInputCounterProducerCtx6AInitQueryFrame
        << " firstNoInputCounterProducerCtx6AInitValue="
        << numeric.rightRankFirstNoInputCounterProducerCtx6AInitValue
        << " firstAcceptedCountClearKnown="
        << (numeric.rightRankFirstAcceptedCountClearKnown ? 1 : 0)
        << " firstAcceptedCountClearQuery="
        << numeric.rightRankFirstAcceptedCountClearQueryFrame
        << " firstAcceptedCountClearSourceBucket="
        << (int)numeric.rightRankFirstAcceptedCountClearSourceBucket
        << " firstAcceptedCountClearCurrentBucket="
        << (int)numeric.rightRankFirstAcceptedCountClearCurrentBucket
        << " firstAcceptedCountClearPreviousBucket="
        << (int)numeric.rightRankFirstAcceptedCountClearPreviousBucket
        << " firstAcceptedCountClearBucket30Advanced="
        << (numeric.rightRankFirstAcceptedCountClearBucket30Advanced ? 1 : 0)
        << " firstAcceptedCountClearBucketAdvanceCount="
        << numeric.rightRankFirstAcceptedCountClearBucketAdvanceCount
        << " firstAcceptedCountClearPreCount="
        << numeric.rightRankFirstAcceptedCountClearPreCount
        << " firstAcceptedCountClearPostCount="
        << numeric.rightRankFirstAcceptedCountClearPostCount
        << " firstAcceptedCountClearPreMask="
        << numeric.rightRankFirstAcceptedCountClearPreMask
        << " firstAcceptedCountClearPostMask="
        << numeric.rightRankFirstAcceptedCountClearPostMask
        << " firstAcceptedCountClearMask9180C="
        << numeric.rightRankFirstAcceptedCountClearMask9180C
        << " firstAcceptedCountClearBucketLocalClearRan="
        << (numeric.rightRankFirstAcceptedCountClearBucketLocalClearRan ? 1 : 0)
        << " firstAcceptedCountClearNarrowFired="
        << (numeric.rightRankFirstAcceptedCountClearNarrowFired ? 1 : 0)
        << " firstAcceptedCountClearAction="
        << (int)numeric.rightRankFirstAcceptedCountClearAction
        << " firstAcceptedCountClearPreBucket30Ed00="
        << numeric.rightRankFirstAcceptedCountClearPreBucket30Ed00
        << " firstAcceptedCountClearDirectConsumer94400="
        << (numeric.rightRankFirstAcceptedCountClearDirectConsumer94400 ? 1 : 0)
        << " firstAcceptedCountClearDirectConsumerImmediateFollowUpClear="
        << (numeric.rightRankFirstAcceptedCountClearDirectConsumerImmediateFollowUpClear ? 1 : 0)
        << " firstAcceptedCountClearDirectConsumerOwnerNoResolution94400="
        << (numeric.rightRankFirstAcceptedCountClearDirectConsumerOwnerNoResolution94400 ? 1 : 0)
        << " firstAcceptedCountClearWaitSecondBeatBucket30="
        << (numeric.rightRankFirstAcceptedCountClearWaitSecondBeatBucket30 ? 1 : 0)
        << " firstAcceptedCountClearAcceptedTailSurvived="
        << (numeric.rightRankFirstAcceptedCountClearAcceptedTailSurvived ? 1 : 0)
        << " firstAcceptedCountClearOwnerKernelOpen="
        << (numeric.rightRankFirstAcceptedCountClearOwnerKernelOpen ? 1 : 0)
        << " firstAcceptedCountClearResolverGateBit4="
        << (numeric.rightRankFirstAcceptedCountClearResolverGateBit4 ? 1 : 0)
        << " firstAcceptedCountClearResolutionGateEd00Idle="
        << (numeric.rightRankFirstAcceptedCountClearResolutionGateEd00Idle ? 1 : 0)
        << " firstAcceptedCountClearPhase1LatchArmed38="
        << (numeric.rightRankFirstAcceptedCountClearPhase1LatchArmed38 ? 1 : 0)
        << " firstAcceptedCountClearFollowUpPhaseIsNone="
        << (numeric.rightRankFirstAcceptedCountClearFollowUpPhaseIsNone ? 1 : 0)
        << " firstAcceptedCountClearRowWriteResolutionKnown="
        << (numeric.rightRankFirstAcceptedCountClearRowWriteResolutionKnown ? 1 : 0)
        << " firstAcceptedCountClearRowWriteResolutionV22="
        << (int)numeric.rightRankFirstAcceptedCountClearRowWriteResolutionV22
        << " firstAcceptedCountClearRowWriteResolutionSkippedMissingResolverGateBit4="
        << (numeric.rightRankFirstAcceptedCountClearRowWriteResolutionSkippedMissingResolverGateBit4 ? 1 : 0)
        << " firstAcceptedCountClearRowWriteResolutionSkippedFollowUpActive="
        << (numeric.rightRankFirstAcceptedCountClearRowWriteResolutionSkippedFollowUpActive ? 1 : 0)
        << " firstAcceptedCountClearRowWriteCommitted="
        << (numeric.rightRankFirstAcceptedCountClearRowWriteCommitted ? 1 : 0)
        << " firstAcceptedCountClearRowWriteGoodToCoolCommitted="
        << (numeric.rightRankFirstAcceptedCountClearRowWriteGoodToCoolCommitted ? 1 : 0)
        << " firstAcceptedCountClearRowWriteDirectConsumerFallback94400="
        << (numeric.rightRankFirstAcceptedCountClearRowWriteDirectConsumerFallback94400 ? 1 : 0)
        << " firstAcceptedCountClearDirectSlotKnown="
        << (numeric.rightRankFirstAcceptedCountClearDirectSlotKnown ? 1 : 0)
        << " firstAcceptedCountClearDirectSlot="
        << (int)numeric.rightRankFirstAcceptedCountClearDirectSlot
        << " firstAcceptedCountClearDescriptorFlagKnown="
        << (numeric.rightRankFirstAcceptedCountClearDescriptorFlagKnown ? 1 : 0)
        << " firstAcceptedCountClearDescriptorFlagWord="
        << numeric.rightRankFirstAcceptedCountClearDescriptorFlagWord
        << " firstAcceptedCountClearDescriptorSubstate50="
        << (int)numeric.rightRankFirstAcceptedCountClearDescriptorSubstate50
        << " firstAcceptedCountClearDescriptorCadenceCursorAvailable="
        << (numeric.rightRankFirstAcceptedCountClearDescriptorCadenceCursorAvailable ? 1 : 0)
        << " firstAcceptedCountClearDescriptorCadenceCursorOrdinal1Based="
        << numeric.rightRankFirstAcceptedCountClearDescriptorCadenceCursorOrdinal1Based
        << " firstAcceptedCountClearDescriptorPageOrdinal56Available="
        << (numeric.rightRankFirstAcceptedCountClearDescriptorPageOrdinal56Available ? 1 : 0)
        << " firstAcceptedCountClearDescriptorPageOrdinal56="
        << numeric.rightRankFirstAcceptedCountClearDescriptorPageOrdinal56
        << " firstAcceptedCountClearDescriptorCurrentCommittedAvailable="
        << (numeric.rightRankFirstAcceptedCountClearDescriptorCurrentCommittedAvailable ? 1 : 0)
        << " firstAcceptedCountClearDescriptorCurrentCommittedRowIndex="
        << numeric.rightRankFirstAcceptedCountClearDescriptorCurrentCommittedRowIndex
        << " firstAcceptedCountClearDescriptorCurrentDescriptor40Available="
        << (numeric.rightRankFirstAcceptedCountClearDescriptorCurrentDescriptor40Available ? 1 : 0)
        << " firstAcceptedCountClearDescriptorCurrentDescriptor40RowIndex="
        << numeric.rightRankFirstAcceptedCountClearDescriptorCurrentDescriptor40RowIndex
        << " firstAcceptedCountClearDescriptorRowAvailable="
        << (numeric.rightRankFirstAcceptedCountClearDescriptorRowAvailable ? 1 : 0)
        << " firstAcceptedCountClearDescriptorLessonId="
        << (int)numeric.rightRankFirstAcceptedCountClearDescriptorLessonId
        << " firstAcceptedCountClearDescriptorDefaultSelector0="
        << (int)numeric.rightRankFirstAcceptedCountClearDescriptorDefaultSelector0
        << " firstAcceptedCountClearDescriptorDefaultSelector1="
        << (int)numeric.rightRankFirstAcceptedCountClearDescriptorDefaultSelector1
        << " firstAcceptedCountClearDescriptorDefaultFlagWord="
        << numeric.rightRankFirstAcceptedCountClearDescriptorDefaultFlagWord
        << " firstAcceptedCountClearDescriptorSubstateSelector0="
        << (int)numeric.rightRankFirstAcceptedCountClearDescriptorSubstateSelector0
        << " firstAcceptedCountClearDescriptorSubstateSelector1="
        << (int)numeric.rightRankFirstAcceptedCountClearDescriptorSubstateSelector1
        << " firstAcceptedCountClearDescriptorSubstateFlagWord="
        << numeric.rightRankFirstAcceptedCountClearDescriptorSubstateFlagWord
        << " firstAcceptedCountClearGameplayReaderPageOrdinalKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayReaderPageOrdinalKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayReaderPageOrdinal="
        << numeric.rightRankFirstAcceptedCountClearGameplayReaderPageOrdinal
        << " firstAcceptedCountClearGameplayRequiredMaskKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayRequiredMaskKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredMask="
        << numeric.rightRankFirstAcceptedCountClearGameplayRequiredMask
        << " firstAcceptedCountClearGameplayUnionMaskKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayUnionMaskKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayUnionMask="
        << numeric.rightRankFirstAcceptedCountClearGameplayUnionMask
        << " firstAcceptedCountClearGameplayAnchorSlotIndexKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotIndexKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorSlotIndex="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotIndex
        << " firstAcceptedCountClearGameplayRequiredClassTokenKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayRequiredClassTokenKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredClassToken="
        << numeric.rightRankFirstAcceptedCountClearGameplayRequiredClassToken
        << " firstAcceptedCountClearGameplayAnchorSlotClassTokenKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotClassTokenKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorSlotClassToken="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotClassToken
        << " firstAcceptedCountClearGameplayAnchorSlotOccupiedKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotOccupiedKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorSlotOccupied="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotOccupied
        << " firstAcceptedCountClearGameplayAnchorSlotAcceptedMaskKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotAcceptedMaskKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorSlotAcceptedMask="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotAcceptedMask
        << " firstAcceptedCountClearGameplayAnchorSlotPayloadKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotPayloadKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorSlotPayload="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotPayload
        << " firstAcceptedCountClearGameplayAnchorPageOccupiedSlotBitsKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageOccupiedSlotBitsKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorPageOccupiedSlotBits="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageOccupiedSlotBits
        << " firstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBitsKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBitsKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBits="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBits
        << " firstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBitsKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBitsKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBits="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBits
        << " firstAcceptedCountClearGameplayAnchorClassMatchKnown="
        << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorClassMatchKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayAnchorClassMatch="
        << numeric.rightRankFirstAcceptedCountClearGameplayAnchorClassMatch
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0Known="
        << (requiredOccupiedSlot0.slotKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0="
        << (int)requiredOccupiedSlot0.slotIndex
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WriterKnown="
        << (requiredOccupiedSlot0.writerKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0Producer=0x"
        << std::hex << requiredOccupiedSlot0.producerFunction << std::dec
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WriteQuery="
        << requiredOccupiedSlot0.writeQueryFrame
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOrdinal38="
        << requiredOccupiedSlot0.writePageOrdinal38
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0RecordSlot24="
        << (int)requiredOccupiedSlot0.recordSlot24
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0RecordRemainder24="
        << (int)requiredOccupiedSlot0.recordRemainder24
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerKnown="
        << (requiredOccupiedSlot0.writePageOwnerKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerOrdinal1Based="
        << requiredOccupiedSlot0.writePageOwnerOrdinal1Based
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerLastSetKnown="
        << (requiredOccupiedSlot0.writePageOwnerLastSetKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerLastSetQuery="
        << requiredOccupiedSlot0.writePageOwnerLastSetQueryFrame
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerLastSetTargetOrdinal1Based="
        << requiredOccupiedSlot0.writePageOwnerLastSetTargetOrdinal1Based
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerLastSetTick96Known="
        << (requiredOccupiedSlot0.writePageOwnerLastSetTick96Known ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerLastSetTick96="
        << requiredOccupiedSlot0.writePageOwnerLastSetTick96
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerLastSetPhase384="
        << requiredOccupiedSlot0.writePageOwnerLastSetPhase384
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0WritePageOwnerLastSetBucket31="
        << (int)requiredOccupiedSlot0.writePageOwnerLastSetBucket31
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0SameTickPageClearApplied14BDC="
        << (requiredOccupiedSlot0.sameTickPageClearApplied14BDC ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0SameTickPageClearTargetKnown14BDC="
        << (requiredOccupiedSlot0.sameTickPageClearTargetKnown14BDC ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0SameTickPageClearTargetOrdinal1Based14BDC="
        << requiredOccupiedSlot0.sameTickPageClearTargetOrdinal1Based14BDC
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0AcceptedTick96Known="
        << (requiredOccupiedSlot0.acceptedTick96Known ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0AcceptedTick96="
        << requiredOccupiedSlot0.acceptedTick96
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0Phase384="
        << requiredOccupiedSlot0.phase384
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0TimingSlot48="
        << (int)requiredOccupiedSlot0.timingTemplateSlot48
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0SourceCellCursor="
        << requiredOccupiedSlot0.sourceCellCursor
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0AcceptedMask="
        << requiredOccupiedSlot0.acceptedMask
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0ClassToken="
        << requiredOccupiedSlot0.pageCompanion
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0OccupiedCount="
        << requiredOccupiedSlot0.occupiedCount
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot0Payload="
        << requiredOccupiedSlot0.sourceCellPtr
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1Known="
        << (requiredOccupiedSlot1.slotKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1="
        << (int)requiredOccupiedSlot1.slotIndex
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WriterKnown="
        << (requiredOccupiedSlot1.writerKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1Producer=0x"
        << std::hex << requiredOccupiedSlot1.producerFunction << std::dec
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WriteQuery="
        << requiredOccupiedSlot1.writeQueryFrame
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOrdinal38="
        << requiredOccupiedSlot1.writePageOrdinal38
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1RecordSlot24="
        << (int)requiredOccupiedSlot1.recordSlot24
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1RecordRemainder24="
        << (int)requiredOccupiedSlot1.recordRemainder24
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerKnown="
        << (requiredOccupiedSlot1.writePageOwnerKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerOrdinal1Based="
        << requiredOccupiedSlot1.writePageOwnerOrdinal1Based
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerLastSetKnown="
        << (requiredOccupiedSlot1.writePageOwnerLastSetKnown ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerLastSetQuery="
        << requiredOccupiedSlot1.writePageOwnerLastSetQueryFrame
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerLastSetTargetOrdinal1Based="
        << requiredOccupiedSlot1.writePageOwnerLastSetTargetOrdinal1Based
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerLastSetTick96Known="
        << (requiredOccupiedSlot1.writePageOwnerLastSetTick96Known ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerLastSetTick96="
        << requiredOccupiedSlot1.writePageOwnerLastSetTick96
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerLastSetPhase384="
        << requiredOccupiedSlot1.writePageOwnerLastSetPhase384
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1WritePageOwnerLastSetBucket31="
        << (int)requiredOccupiedSlot1.writePageOwnerLastSetBucket31
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1SameTickPageClearApplied14BDC="
        << (requiredOccupiedSlot1.sameTickPageClearApplied14BDC ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1SameTickPageClearTargetKnown14BDC="
        << (requiredOccupiedSlot1.sameTickPageClearTargetKnown14BDC ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1SameTickPageClearTargetOrdinal1Based14BDC="
        << requiredOccupiedSlot1.sameTickPageClearTargetOrdinal1Based14BDC
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1AcceptedTick96Known="
        << (requiredOccupiedSlot1.acceptedTick96Known ? 1 : 0)
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1AcceptedTick96="
        << requiredOccupiedSlot1.acceptedTick96
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1Phase384="
        << requiredOccupiedSlot1.phase384
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1TimingSlot48="
        << (int)requiredOccupiedSlot1.timingTemplateSlot48
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1SourceCellCursor="
        << requiredOccupiedSlot1.sourceCellCursor
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1AcceptedMask="
        << requiredOccupiedSlot1.acceptedMask
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1ClassToken="
        << requiredOccupiedSlot1.pageCompanion
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1OccupiedCount="
        << requiredOccupiedSlot1.occupiedCount
        << " firstAcceptedCountClearGameplayRequiredOccupiedSlot1Payload="
        << requiredOccupiedSlot1.sourceCellPtr
        << " followUpPhaseId=" << (int)numeric.rightRankFollowUpPhase
        << " followUpPhase=" << (int)numeric.rightRankFollowUpPhase
        << " acceptedNarrowClearPending="
        << (numeric.acceptedProducerNarrowClearPending ? 1 : 0)
        << " acceptedNarrowClearFired="
        << (numeric.acceptedProducerNarrowClearFired ? 1 : 0)
        << " acceptedNarrowClearPhaseId="
        << (int)numeric.acceptedProducerNarrowClearPhase
        << Stage1OverlayDescribeBucket31Flag0200Producer(
               numeric.bucket31Flag0200Producer)
        << " replayMirrorKnown="
        << (replayMirror.replayMirrorKnown8008EEF8 ? 1 : 0)
        << " replayProducerKnown="
        << (replayMirror.replayMirrorProducerKnown8008EEF8 ? 1 : 0)
        << " replayProducer=0x" << std::hex
        << replayMirror.replayMirrorProducerFunction << std::dec
        << " replayBytesKnown="
        << (replayMirror.replayMirrorByteCountKnown8008EEF8 ? 1 : 0)
        << " replayKnownBytes="
        << replayMirror.replayMirrorKnownByteCount8008EEF8
        << " replayPublished=" << replayMirror.publishedCount901BC
        << " replayWriteCount=" << replayMirror.writeCount901C0
        << " replayFullBytes="
        << (replayMirror.replayMirrorKnown8008EEF8 &&
                    replayMirror.replayMirrorProducerKnown8008EEF8 &&
                    replayMirror.replayMirrorByteCountKnown8008EEF8 &&
                    replayMirror.replayMirrorKnownByteCount8008EEF8 >=
                        kPrStage1ScorerDirectReplayMirrorByteCount
                ? 1
                : 0)
        << " gate=" << (numeric.active ? 1 : 0)
        << " bucket=" << (int)numeric.bucketCadence.currentBucket
        << " prevBucket=" << (int)numeric.bucketCadence.previousBucket
        << " bucketChanged="
        << (numeric.bucketCadence.bucketChanged ? 1 : 0)
        << " bucket0=" << (numeric.bucketCadence.bucket0Advanced ? 1 : 0)
        << " bucket30=" << (numeric.bucketCadence.bucket30Advanced ? 1 : 0)
        << " bucket31=" << (numeric.bucketCadence.bucket31Advanced ? 1 : 0)
        << " bucketAdvanceCount="
        << numeric.bucketCadence.bucketAdvanceCount
        << " topLessonVisible="
        << (numeric.topLessonPairState.topLessonPairChangeVisible ? 1 : 0)
        << " topLessonLessonId="
        << (int)numeric.topLessonPairState.topLessonPairLessonId
        << " topLessonCurrentCommittedAvailable="
        << (descriptorCadence.currentCommittedAvailable ? 1 : 0)
        << " topLessonCurrentCommittedRow="
        << descriptorCadence.currentCommittedRowIndex
        << " topLessonCurrentCommittedLessonId="
        << (int)descriptorCadence.currentCommittedRow.lessonId
        << " topLessonNextLookaheadAvailable="
        << (descriptorCadence.nextLookaheadAvailable ? 1 : 0)
        << " topLessonNextLookaheadRow="
        << descriptorCadence.nextLookaheadRowIndex
        << " topLessonNextLookaheadLessonId="
        << (int)descriptorCadence.nextLookaheadRow.lessonId
        << " desc44Available=" << (desc44Available ? 1 : 0)
        << " desc44Row=" << descriptorCadence.nextLookaheadRowIndex
        << " desc44Lesson=" << (int)desc44Row.lessonId
        << " desc44Substate50=" << desc44Substate50
        << " desc44RequiredMask=" << desc44Row.requiredMask
        << " desc44DefaultSel0=" << (int)desc44Row.defaultSelectorByte0
        << " desc44DefaultSel1=" << (int)desc44Row.defaultSelectorByte1
        << " desc44SubstateSel0=" << (int)desc44Row.substate1SelectorByte0
        << " desc44SubstateSel1=" << (int)desc44Row.substate1SelectorByte1
        << " desc44BranchSel0=" << (int)desc44BranchSelectorByte0
        << " desc44BranchSel1=" << (int)desc44BranchSelectorByte1;
    out << "\nlifecycle available=" << (liveValid ? 1 : 0)
        << " query=" << (liveValid ? (int)liveLifecycle.queryFrame : query)
        << " clearGate=" << (liveValid && liveLifecycle.clearGate ? 1 : 0)
        << " failGate=" << (liveValid && liveLifecycle.failGate ? 1 : 0)
        << " rightRankActiveRow="
        << (liveValid ? (int)liveLifecycle.rightRankActiveRow : 0)
        << " awfulHoldFrames="
        << (liveValid ? liveLifecycle.awfulHoldFrames : 0)
        << " awfulHoldFramesRequired="
        << (liveValid ? liveLifecycle.awfulHoldFramesRequired : 0)
        << " liveValid=" << (liveValid ? 1 : 0)
        << " liveQuery=" << (liveValid ? (int)liveLifecycle.queryFrame : -1)
        << " liveClearGate=" << (liveValid && liveLifecycle.clearGate ? 1 : 0)
        << " liveClearTerminalTailGate="
        << (liveValid && liveLifecycle.clearTerminalTailGate ? 1 : 0)
        << " liveFailGate=" << (liveValid && liveLifecycle.failGate ? 1 : 0)
        << " liveRightRankActiveRow="
        << (liveValid ? (int)liveLifecycle.rightRankActiveRow : 0)
        << " liveAwfulHoldFrames="
        << (liveValid ? liveLifecycle.awfulHoldFrames : 0)
        << " liveAwfulHoldFramesRequired="
        << (liveValid ? liveLifecycle.awfulHoldFramesRequired : 0)
        << " liveRunnerExitLatch76Known="
        << (liveValid && liveLifecycle.runnerExitLatch76Known ? 1 : 0)
        << " liveRunnerExitLatch76="
        << (liveValid && liveLifecycle.runnerExitLatch76Active ? 1 : 0)
        << " liveRunnerExitLatch76SourceBucket0="
        << (liveValid && liveLifecycle.runnerExitLatch76SourceBucket0 ? 1 : 0)
        << " liveRunnerExitLatch76SourceTailStream4="
        << (liveValid && liveLifecycle.runnerExitLatch76SourceTailStream4 ? 1 : 0)
        << " liveRunnerExitLatch76SourceLowLevelAbort="
        << (liveValid && liveLifecycle.runnerExitLatch76SourceLowLevelAbort ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseDetailKnown="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseDetailKnown ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitLatch76Bucket0PulseQueryFrame : -1)
        << " liveRunnerExitLatch76Bucket0PulseRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitLatch76Bucket0PulseRightRankRow : 0)
        << " liveRunnerExitLatch76Bucket0PulseCallWindowOpen="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseCallWindowOpen ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseDescriptorFlags="
        << (liveValid ? (int)liveLifecycle.runnerExitLatch76Bucket0PulseDescriptorFlags : 0)
        << " liveRunnerExitLatch76Bucket0PulseBusyGateActive="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseBusyGateActive ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseDescriptorBit8ConsumeGate="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseDescriptorBit8ConsumeGate ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseDescriptorBit10ShortCircuitGate="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseDescriptorBit10ShortCircuitGate ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseShortCircuitedByBit10="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseShortCircuitedByBit10 ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseConsumeGateBit8="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseConsumeGateBit8 ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseCtx54PermitInput="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseCtx54PermitInput ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseCtx54PermitOutput="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseCtx54PermitOutput ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8Called="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Called ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8Result="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Result ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8Row3NoInputBranch="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Row3NoInputBranch ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8Row3TieCarryBranch="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Row3TieCarryBranch ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8Row0TieCarryBranch="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Row0TieCarryBranch ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8NonZeroRowBlocked="
        << (liveValid && liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8NonZeroRowBlocked ? 1 : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8NoInputCounterInput="
        << (liveValid ? (int)liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8NoInputCounterInput : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8TieCarryLatchInput="
        << (liveValid ? (int)liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8TieCarryLatchInput : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8Ctx6AConsumerGateInput="
        << (liveValid ? (int)liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Ctx6AConsumerGateInput : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8NoInputCounterOutput="
        << (liveValid ? (int)liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8NoInputCounterOutput : 0)
        << " liveRunnerExitLatch76Bucket0PulseReturnGate144B8TieCarryLatchOutput="
        << (liveValid ? (int)liveLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8TieCarryLatchOutput : 0)
        << " liveRunnerExitGate78Known="
        << (liveValid && liveLifecycle.runnerExitGate78Known ? 1 : 0)
        << " liveRunnerExitGate78="
        << (liveValid && liveLifecycle.runnerExitGate78Active ? 1 : 0)
        << " liveRunnerExitGate78SourceEd1CHandoff="
        << (liveValid && liveLifecycle.runnerExitGate78SourceEd1CHandoff ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094Known="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094Known ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094QueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094QueryFrame : -1)
        << " liveRunnerExitGate78FrameUpdate9094Ctx76="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094Ctx76 : 0)
        << " liveRunnerExitGate78FrameUpdate9094Ctx78="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094Ctx78 : 0)
        << " liveRunnerExitGate78FrameUpdate9094CtxFlags00InputKnown="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094CtxFlags00InputKnown ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094CtxFlags00Input="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094CtxFlags00Input : 0)
        << " liveRunnerExitGate78FrameUpdate9094CtxFlag40Input="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40Input ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094CtxFlag40SourceBucket0="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceBucket0 ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094CtxFlag40SourceQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceQueryFrame : -1)
        << " liveRunnerExitGate78FrameUpdate9094CtxFlag40SourceRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceRightRankRow : 0)
        << " liveRunnerExitGate78FrameUpdate9094CtxFlag40SourceCallWindowOpen="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceCallWindowOpen ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094CtxFlag40SourceDescriptorFlags="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceDescriptorFlags : 0)
        << " liveRunnerExitGate78FrameUpdate9094LastCtxFlag40InputKnown="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40InputKnown ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094LastCtxFlag40InputQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40InputQueryFrame : -1)
        << " liveRunnerExitGate78FrameUpdate9094LastCtxFlag40InputFlags00="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40InputFlags00 : 0)
        << " liveRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceBucket0="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceBucket0 ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceQueryFrame : -1)
        << " liveRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceRightRankRow : 0)
        << " liveRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceCallWindowOpen="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceCallWindowOpen ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceDescriptorFlags="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceDescriptorFlags : 0)
        << " liveRunnerExitGate78FrameUpdate9094TailFamilyActive="
        << (liveValid && liveLifecycle.runnerExitGate78FrameUpdate9094TailFamilyActive ? 1 : 0)
        << " liveRunnerExitGate78FrameUpdate9094TailStream="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094TailStream : 0)
        << " liveRunnerExitGate78FrameUpdate9094ActiveStream="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78FrameUpdate9094ActiveStream : 0)
        << " liveRunnerExitGate78Ed1CProducerKnown="
        << (liveValid && liveLifecycle.runnerExitGate78Ed1CProducerKnown ? 1 : 0)
        << " liveRunnerExitGate78Ed1CProducerEventStreamFlag="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78Ed1CProducerEventStreamFlag : 0)
        << " liveRunnerExitGate78Ed1CProducerFlagDescriptorValid="
        << (liveValid && liveLifecycle.runnerExitGate78Ed1CProducerFlagDescriptorValid ? 1 : 0)
        << " liveRunnerExitGate78Ed1CProducerActiveFlagStreamValid="
        << (liveValid && liveLifecycle.runnerExitGate78Ed1CProducerActiveFlagStreamValid ? 1 : 0)
        << " liveRunnerExitGate78Ed1CProducerActiveFlagStreamIndex="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78Ed1CProducerActiveFlagStreamIndex : 0)
        << " liveRunnerExitGate78Ed1CProducerActiveFlagStreamCount="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78Ed1CProducerActiveFlagStreamCount : 0)
        << " liveRunnerExitGate78Ed1CProducerDueKnown="
        << (liveValid && liveLifecycle.runnerExitGate78Ed1CProducerDueKnown ? 1 : 0)
        << " liveRunnerExitGate78Ed1CProducerDueFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78Ed1CProducerDueFrame : -1)
        << " liveRunnerExitGate78Ed1CProducerDueDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78Ed1CProducerDueDelta : 0)
        << " liveRunnerExitGate78Ed1CProducerFlags04="
        << (liveValid ? (int)liveLifecycle.runnerExitGate78Ed1CProducerFlags04 : 0)
        << " liveRunnerExitGate78Ed1CProducerFlag80="
        << (liveValid && liveLifecycle.runnerExitGate78Ed1CProducerFlag80 ? 1 : 0)
        << " liveRunnerExitGate78Ed1CProducerConsumedFlagStreamEvent="
        << (liveValid && liveLifecycle.runnerExitGate78Ed1CProducerConsumedFlagStreamEvent ? 1 : 0)
        << " liveRunnerExitGate78Ed1CProducerProduced="
        << (liveValid && liveLifecycle.runnerExitGate78Ed1CProducerProduced ? 1 : 0)
        << " liveRunnerExitEventStreamFlagLastUpdateKnown="
        << (liveValid && liveLifecycle.runnerExitEventStreamFlagLastUpdateKnown ? 1 : 0)
        << " liveRunnerExitEventStreamFlagLastUpdateReason="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastUpdateReason : 0)
        << " liveRunnerExitEventStreamFlagLastUpdateQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastUpdateQueryFrame : -1)
        << " liveRunnerExitEventStreamFlagLastUpdateScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastUpdateScriptFrame : -1)
        << " liveRunnerExitEventStreamFlagLastUpdatePrevious="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastUpdatePrevious : 0)
        << " liveRunnerExitEventStreamFlagLastUpdateCurrent="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastUpdateCurrent : 0)
        << " liveRunnerExitEventStreamFlagLastChangeKnown="
        << (liveValid && liveLifecycle.runnerExitEventStreamFlagLastChangeKnown ? 1 : 0)
        << " liveRunnerExitEventStreamFlagLastChangeReason="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastChangeReason : 0)
        << " liveRunnerExitEventStreamFlagLastChangeQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastChangeQueryFrame : -1)
        << " liveRunnerExitEventStreamFlagLastChangeScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastChangeScriptFrame : -1)
        << " liveRunnerExitEventStreamFlagLastChangePrevious="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastChangePrevious : 0)
        << " liveRunnerExitEventStreamFlagLastChangeCurrent="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastChangeCurrent : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearKnown="
        << (liveValid && liveLifecycle.runnerExitEventStreamFlagLastRunnerClearKnown ? 1 : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearReason="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearReason : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearQueryFrame : -1)
        << " liveRunnerExitEventStreamFlagLastRunnerClearScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearScriptFrame : -1)
        << " liveRunnerExitEventStreamFlagLastRunnerClearInputFlag="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearInputFlag : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearOutputFlag="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearOutputFlag : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearInputCtxFlags00="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearInputCtxFlags00 : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearOutputCtxFlags00="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearOutputCtxFlags00 : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearInputEd1C="
        << (liveValid && liveLifecycle.runnerExitEventStreamFlagLastRunnerClearInputEd1C ? 1 : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearOutputEd1C="
        << (liveValid && liveLifecycle.runnerExitEventStreamFlagLastRunnerClearOutputEd1C ? 1 : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearWord4E="
        << (liveValid ? (int)liveLifecycle.runnerExitEventStreamFlagLastRunnerClearWord4E : 0)
        << " liveRunnerExitEventStreamFlagLastRunnerClearDeltaToHandoff="
        << queryDeltaOrMissing(liveRunnerClearKnown,
                               (int)liveLifecycle.queryFrame,
                               liveRunnerClearQuery)
        << " liveRunnerExitEventStreamFlagLastRunnerClearDeltaAfterLastReplayAppend="
        << queryDeltaOrMissing(liveRunnerClearKnown &&
                                   numeric.rightRankLastReplayAppendKnown,
                               liveRunnerClearQuery,
                               numeric.rightRankLastReplayAppendQueryFrame)
        << " liveRunnerExitEventStreamFlagLastRunnerClearDeltaAfterLastAcceptedTailCall="
        << queryDeltaOrMissing(liveRunnerClearKnown &&
                                   numeric.rightRankLastAcceptedTailCallKnown,
                               liveRunnerClearQuery,
                               numeric.rightRankLastAcceptedTailCallQueryFrame)
        << " liveRunnerExitEventStreamFlagLastRunnerClearDeltaBeforeLastAcceptedTailDecision="
        << queryDeltaOrMissing(liveRunnerClearKnown &&
                                   numeric.rightRankLastAcceptedTailDecisionKnown,
                               numeric.rightRankLastAcceptedTailDecisionQueryFrame,
                               liveRunnerClearQuery)
        << " liveRunnerExitFlag40="
        << (liveValid && liveLifecycle.runnerExitFlag40Active ? 1 : 0)
        << " liveRunnerExitLateBranchSelectedStream="
        << (liveValid ? (int)liveLifecycle.runnerExitLateBranchSelectedStream : 0)
        << " liveRunnerExitLateBranchActiveDispatchStream="
        << (liveValid ? (int)liveLifecycle.runnerExitLateBranchActiveDispatchStream : 0)
        << " liveRunnerExitFlag100="
        << (liveValid && liveLifecycle.runnerExitFlag100BlocksWaitActive ? 1 : 0)
        << " liveRunnerExitFlag100SourceStream="
        << (liveValid ? (int)liveLifecycle.runnerExitFlag100SourceStream : 0)
        << " liveRunnerExitFirstFlag100PulseKnown="
        << (liveValid && liveLifecycle.runnerExitFirstFlag100PulseKnown ? 1 : 0)
        << " liveRunnerExitFirstFlag100PulseQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstFlag100PulseQueryFrame : 0)
        << " liveRunnerExitFirstFlag100PulseScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstFlag100PulseScriptFrame : 0)
        << " liveRunnerExitFirstFlag100PulseSourceStream="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstFlag100PulseSourceStream : 0)
        << " liveRunnerExitFirstFlag100PulseReason="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstFlag100PulseReason : 0)
        << " liveRunnerExit9094ScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitLateBranchScriptFrame : -1)
        << " liveRunnerExitClearTailPulse="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailPulseInput ? 1 : 0)
        << " liveRunnerExitClearTailPulseArmed="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailPulseArmed ? 1 : 0)
        << " liveRunnerExitClearTailPulseBlockAlreadyArmed="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailPulseBlockedAlreadyArmed ? 1 : 0)
        << " liveRunnerExitClearTailPulseBlockActiveDispatch="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailPulseBlockedActiveDispatch ? 1 : 0)
        << " liveRunnerExitClearTailPulseBlockPendingMismatch="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailPulseBlockedPendingMismatch ? 1 : 0)
        << " liveRunnerExitClearTailPulseStream="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailPulseStream : 0)
        << " liveRunnerExitClearTailLatchSetKnown="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailLatchSetKnown ? 1 : 0)
        << " liveRunnerExitClearTailLatchSetQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailLatchSetQueryFrame : -1)
        << " liveRunnerExitClearTailLatchSetScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailLatchSetScriptFrame : -1)
        << " liveRunnerExitClearTailLatchSetRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailLatchSetRightRankRow : 0)
        << " liveRunnerExitClearTailLatchSetCurrentMode="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailLatchSetCurrentMode : 0)
        << " liveRunnerExitClearTailLatchSetStream="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailLatchSetStream : 0)
        << " liveRunnerExitClearTailTriggerAttempted="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerAttempted ? 1 : 0)
        << " liveRunnerExitClearTailTriggerAccepted="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerAccepted ? 1 : 0)
        << " liveRunnerExitClearTailTriggerScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerScriptFrame : -1)
        << " liveRunnerExitClearTailTriggerRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerRightRankRow : 0)
        << " liveRunnerExitClearTailTriggerCurrentMode="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerCurrentMode : 0)
        << " liveRunnerExitClearTailTriggerBlockConsumed="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedConsumed ? 1 : 0)
        << " liveRunnerExitClearTailTriggerBlockArmed="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedArmed ? 1 : 0)
        << " liveRunnerExitClearTailTriggerBlockFlagNotOne="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedFlagNotOne ? 1 : 0)
        << " liveRunnerExitClearTailTriggerBlockRow="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedRow ? 1 : 0)
        << " liveRunnerExitClearTailTriggerBlockStreamMissing="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedStreamMissing ? 1 : 0)
        << " liveRunnerExitClearTailTriggerBlockCursorDone="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedCursorDone ? 1 : 0)
        << " liveRunnerExitClearTailTriggerBlockEventNotDue="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedEventNotDue ? 1 : 0)
        << " liveRunnerExitClearTailTriggerBlockMissingFlag80="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80 ? 1 : 0)
        << " liveRunnerExitClearTailTriggerStreamFlag="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStreamFlag : 0)
        << " liveRunnerExitClearTailTriggerAttemptCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAttemptCount : 0)
        << " liveRunnerExitClearTailTriggerAcceptedCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedCount : 0)
        << " liveRunnerExitClearTailTriggerEligibleCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerEligibleCount : 0)
        << " liveRunnerExitClearTailTriggerAcceptedKnown="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedKnown ? 1 : 0)
        << " liveRunnerExitClearTailTriggerAcceptedQueryFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedQueryFrame : -1)
        << " liveRunnerExitClearTailTriggerAcceptedScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedScriptFrame : -1)
        << " liveRunnerExitClearTailTriggerAcceptedRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedRightRankRow : 0)
        << " liveRunnerExitClearTailTriggerAcceptedCurrentMode="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedCurrentMode : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStreamFlag="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStreamFlag : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStream1Cursor="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1Cursor : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStream1Count="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1Count : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStream1DueKnown="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1DueKnown ? 1 : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStream1DueFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1DueFrame : -1)
        << " liveRunnerExitClearTailTriggerAcceptedStream1DueDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1DueDelta : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStream1BaseFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1BaseFrame : -1)
        << " liveRunnerExitClearTailTriggerAcceptedStream1AbsDueFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1AbsDueFrame : -1)
        << " liveRunnerExitClearTailTriggerAcceptedStream1AbsDueDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1AbsDueDelta : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStream1PsxAddr="
        << (liveValid ? liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1PsxAddr : 0u)
        << " liveRunnerExitClearTailTriggerAcceptedStream1Flags04="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1Flags04 : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStream1Byte29="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1Byte29 : 0)
        << " liveRunnerExitClearTailTriggerAcceptedStream1Byte30="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerAcceptedStream1Byte30 : 0)
        << " liveRunnerExitClearTailTriggerBlockFlagNotOneCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedFlagNotOneCount : 0)
        << " liveRunnerExitClearTailTriggerBlockRowCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedRowCount : 0)
        << " liveRunnerExitClearTailTriggerBlockFlagAndRowCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedFlagAndRowCount : 0)
        << " liveRunnerExitClearTailTriggerBlockConsumedCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedConsumedCount : 0)
        << " liveRunnerExitClearTailTriggerBlockArmedCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedArmedCount : 0)
        << " liveRunnerExitClearTailTriggerBlockStreamMissingCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedStreamMissingCount : 0)
        << " liveRunnerExitClearTailTriggerBlockCursorDoneCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedCursorDoneCount : 0)
        << " liveRunnerExitClearTailTriggerBlockEventNotDueCount="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedEventNotDueCount : 0)
        << " liveRunnerExitClearTailTriggerBlockMissingFlag80Count="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80Count : 0)
        << " liveRunnerExitFirstClearTailTriggerKnown="
        << (liveValid && liveLifecycle.runnerExitFirstClearTerminalBranchTriggerKnown ? 1 : 0)
        << " liveRunnerExitFirstClearTailTriggerScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerScriptFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerRightRankRow : 0)
        << " liveRunnerExitFirstClearTailTriggerCurrentMode="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerCurrentMode : 0)
        << " liveRunnerExitFirstClearTailTriggerStreamFlag="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerStreamFlag : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleKnown="
        << (liveValid && liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleKnown ? 1 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleScriptFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEligibleRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleRightRankRow : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleCurrentMode="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleCurrentMode : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1Known="
        << (liveValid && liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Known ? 1 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1Cursor="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Cursor : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1Count="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Count : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1DueKnown="
        << (liveValid && liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueKnown ? 1 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1DueFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1DueDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueDelta : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1BaseFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1BaseFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1AbsDueFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1AbsDueDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1Flags04="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Flags04 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1Byte29="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte29 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1Byte30="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte30 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80SearchKnown="
        << (liveValid && liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown ? 1 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Known="
        << (liveValid && liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known ? 1 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Cursor="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Frame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame : -1)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Delta="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80BaseFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80AbsFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80AbsDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Flags04="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Byte29="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 : 0)
        << " liveRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Byte30="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 : 0)
        << " liveRunnerExitClearTailTriggerStream1Cursor="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1Cursor : 0)
        << " liveRunnerExitClearTailTriggerStream1Count="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1Count : 0)
        << " liveRunnerExitClearTailTriggerStream1DueKnown="
        << (liveValid && liveLifecycle.runnerExitClearTerminalBranchTriggerStream1DueKnown ? 1 : 0)
        << " liveRunnerExitClearTailTriggerStream1DueFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1DueFrame : -1)
        << " liveRunnerExitClearTailTriggerStream1DueDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1DueDelta : 0)
        << " liveRunnerExitClearTailTriggerStream1BaseFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1BaseFrame : -1)
        << " liveRunnerExitClearTailTriggerStream1AbsDueFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1AbsDueFrame : -1)
        << " liveRunnerExitClearTailTriggerStream1AbsDueDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1AbsDueDelta : 0)
        << " liveRunnerExitClearTailTriggerStream1Flags04="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1Flags04 : 0)
        << " liveRunnerExitClearTailTriggerStream1Byte29="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1Byte29 : 0)
        << " liveRunnerExitClearTailTriggerStream1Byte30="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalBranchTriggerStream1Byte30 : 0)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueKnown="
        << (liveValid && liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueKnown ? 1 : 0)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueScriptFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueCursor="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueCursor : 0)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueDelta : 0)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueBaseFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueBaseFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueAbsFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueAbsDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueAbsDelta : 0)
        << " liveRunnerExitFirstClearTailTriggerEventNotDueFlags04="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerEventNotDueFlags04 : 0)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80Known="
        << (liveValid && liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Known ? 1 : 0)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80ScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80ScriptFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80Cursor="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Cursor : 0)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80Frame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Frame : -1)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80Delta="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Delta : 0)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80BaseFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80BaseFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80AbsFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsFrame : -1)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80AbsDelta="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80AbsDelta : 0)
        << " liveRunnerExitFirstClearTailTriggerMissingFlag80Flags04="
        << (liveValid ? (int)liveLifecycle.runnerExitFirstClearTerminalBranchTriggerMissingFlag80Flags04 : 0)
        << " liveRunnerExitClearTailServiceLatch="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailServiceLatch ? 1 : 0)
        << " liveRunnerExitClearTailServiceScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailServiceScriptFrame : -1)
        << " liveRunnerExitClearTailServiceRightRankRow="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailServiceRightRankRow : 0)
        << " liveRunnerExitClearTailServiceCurrentMode="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailServiceCurrentMode : 0)
        << " liveRunnerExitClearTailServiceStream="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailServiceStream : 0)
        << " liveRunnerExitPendingRatingBranchSeq="
        << (liveValid ? (int)liveLifecycle.runnerExitPendingRatingBranchSeq : 0)
        << " liveRunnerExitConsumedRatingBranchSeq="
        << (liveValid ? (int)liveLifecycle.runnerExitConsumedRatingBranchSeq : 0)
        << " liveRunnerExitClearTailArmed="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailArmed ? 1 : 0)
        << " liveRunnerExitClearTailDispatchActive="
        << (liveValid && liveLifecycle.runnerExitClearTerminalTailDispatchActive ? 1 : 0)
        << " liveRunnerExitClearTailStream="
        << (liveValid ? (int)liveLifecycle.runnerExitClearTerminalTailStream : 0)
        << " liveRunnerExitActiveDispatchStartScriptFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitActiveDispatchStartScriptFrame : 0)
        << " liveRunnerExitActiveDispatchTerminalEndLocalFrame="
        << (liveValid ? (int)liveLifecycle.runnerExitActiveDispatchTerminalEndLocalFrame : 0)
        << " liveRunnerExitActiveDispatchTerminalPulseEmitted="
        << (liveValid && liveLifecycle.runnerExitActiveDispatchTerminalPulseEmitted ? 1 : 0)
        << " liveRunnerTailRecordsModeDAActive="
        << (liveValid && liveLifecycle.runnerTailRecordsModeDAActive ? 1 : 0)
        << " liveRunnerTailRecordsModeDAEqualsOne="
        << (liveValid && liveLifecycle.runnerTailRecordsModeDAEqualsOne ? 1 : 0)
        << " liveRunnerTailStageStatusKnown="
        << (liveValid && liveLifecycle.runnerTailStageStatusKnown ? 1 : 0)
        << " liveRunnerTailStageStatus166AC="
        << (liveValid ? (int)liveLifecycle.runnerTailStageStatus166AC : 0)
        << " liveRunnerTailCleanupRenderPassBudget="
        << (liveValid ? (int)liveLifecycle.runnerTailCleanupRenderPassBudget : 0)
        << " liveRunnerTailFinalReturnKnown="
        << (liveValid && liveLifecycle.runnerTailFinalReturnKnown ? 1 : 0)
        << " liveRunnerTailFinalReturn="
        << (liveValid ? (int)liveLifecycle.runnerTailFinalReturn : -1)
        << " liveRunnerTailFinalReturnWord59Known="
        << (liveValid && liveLifecycle.runnerTailFinalReturnWord59Known ? 1 : 0)
        << " liveRunnerTailFinalReturnWord59IsOne="
        << (liveValid && liveLifecycle.runnerTailFinalReturnWord59IsOne ? 1 : 0)
        << " liveRunnerTailFinalReturnWord60Known="
        << (liveValid && liveLifecycle.runnerTailFinalReturnWord60Known ? 1 : 0)
        << " liveRunnerTailFinalReturnWord60IsOne="
        << (liveValid && liveLifecycle.runnerTailFinalReturnWord60IsOne ? 1 : 0)
        << " liveRunnerTailFinalReturnCalls166AC="
        << (liveValid && liveLifecycle.runnerTailFinalReturnCalls166AC ? 1 : 0)
        << " liveRunnerTailFinalReturnRecordsModeReturnsOne="
        << (liveValid && liveLifecycle.runnerTailFinalReturnRecordsModeReturnsOne ? 1 : 0)
        << " liveRunnerTailFinalReturnStageStatusReturnsOne="
        << (liveValid && liveLifecycle.runnerTailFinalReturnStageStatusReturnsOne ? 1 : 0)
        << " terminalValid=" << (terminalValid ? 1 : 0)
        << " terminalQuery="
        << (terminalValid ? (int)terminalLifecycle.queryFrame : -1)
        << " terminalClearGate="
        << (terminalValid && terminalLifecycle.clearGate ? 1 : 0)
        << " terminalClearTerminalTailGate="
        << (terminalValid && terminalLifecycle.clearTerminalTailGate ? 1 : 0)
        << " terminalFailGate="
        << (terminalValid && terminalLifecycle.failGate ? 1 : 0)
        << " terminalRightRankActiveRow="
        << (terminalValid ? (int)terminalLifecycle.rightRankActiveRow : 0)
        << " terminalAwfulHoldFrames="
        << (terminalValid ? terminalLifecycle.awfulHoldFrames : 0)
        << " terminalAwfulHoldFramesRequired="
        << (terminalValid ? terminalLifecycle.awfulHoldFramesRequired : 0)
        << " terminalRunnerExitLatch76Known="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Known ? 1 : 0)
        << " terminalRunnerExitLatch76="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Active ? 1 : 0)
        << " terminalRunnerExitLatch76SourceBucket0="
        << (terminalValid && terminalLifecycle.runnerExitLatch76SourceBucket0 ? 1 : 0)
        << " terminalRunnerExitLatch76SourceTailStream4="
        << (terminalValid && terminalLifecycle.runnerExitLatch76SourceTailStream4 ? 1 : 0)
        << " terminalRunnerExitLatch76SourceLowLevelAbort="
        << (terminalValid && terminalLifecycle.runnerExitLatch76SourceLowLevelAbort ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseDetailKnown="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseDetailKnown ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseQueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLatch76Bucket0PulseQueryFrame : -1)
        << " terminalRunnerExitLatch76Bucket0PulseRightRankRow="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLatch76Bucket0PulseRightRankRow : 0)
        << " terminalRunnerExitLatch76Bucket0PulseCallWindowOpen="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseCallWindowOpen ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseDescriptorFlags="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLatch76Bucket0PulseDescriptorFlags : 0)
        << " terminalRunnerExitLatch76Bucket0PulseBusyGateActive="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseBusyGateActive ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseDescriptorBit8ConsumeGate="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseDescriptorBit8ConsumeGate ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseDescriptorBit10ShortCircuitGate="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseDescriptorBit10ShortCircuitGate ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseShortCircuitedByBit10="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseShortCircuitedByBit10 ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseConsumeGateBit8="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseConsumeGateBit8 ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseCtx54PermitInput="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseCtx54PermitInput ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseCtx54PermitOutput="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseCtx54PermitOutput ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8Called="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Called ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8Result="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Result ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8Row3NoInputBranch="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Row3NoInputBranch ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8Row3TieCarryBranch="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Row3TieCarryBranch ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8Row0TieCarryBranch="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Row0TieCarryBranch ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8NonZeroRowBlocked="
        << (terminalValid && terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8NonZeroRowBlocked ? 1 : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8NoInputCounterInput="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8NoInputCounterInput : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8TieCarryLatchInput="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8TieCarryLatchInput : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8Ctx6AConsumerGateInput="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8Ctx6AConsumerGateInput : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8NoInputCounterOutput="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8NoInputCounterOutput : 0)
        << " terminalRunnerExitLatch76Bucket0PulseReturnGate144B8TieCarryLatchOutput="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLatch76Bucket0PulseReturnGate144B8TieCarryLatchOutput : 0)
        << " terminalRunnerExitGate78Known="
        << (terminalValid && terminalLifecycle.runnerExitGate78Known ? 1 : 0)
        << " terminalRunnerExitGate78="
        << (terminalValid && terminalLifecycle.runnerExitGate78Active ? 1 : 0)
        << " terminalRunnerExitGate78SourceEd1CHandoff="
        << (terminalValid && terminalLifecycle.runnerExitGate78SourceEd1CHandoff ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094Known="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094Known ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094QueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094QueryFrame : -1)
        << " terminalRunnerExitGate78FrameUpdate9094Ctx76="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094Ctx76 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094Ctx78="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094Ctx78 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094CtxFlags00InputKnown="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094CtxFlags00InputKnown ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094CtxFlags00Input="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094CtxFlags00Input : 0)
        << " terminalRunnerExitGate78FrameUpdate9094CtxFlag40Input="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40Input ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094CtxFlag40SourceBucket0="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceBucket0 ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094CtxFlag40SourceQueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceQueryFrame : -1)
        << " terminalRunnerExitGate78FrameUpdate9094CtxFlag40SourceRightRankRow="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceRightRankRow : 0)
        << " terminalRunnerExitGate78FrameUpdate9094CtxFlag40SourceCallWindowOpen="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceCallWindowOpen ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094CtxFlag40SourceDescriptorFlags="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094CtxFlag40SourceDescriptorFlags : 0)
        << " terminalRunnerExitGate78FrameUpdate9094LastCtxFlag40InputKnown="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40InputKnown ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094LastCtxFlag40InputQueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40InputQueryFrame : -1)
        << " terminalRunnerExitGate78FrameUpdate9094LastCtxFlag40InputFlags00="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40InputFlags00 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceBucket0="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceBucket0 ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceQueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceQueryFrame : -1)
        << " terminalRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceRightRankRow="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceRightRankRow : 0)
        << " terminalRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceCallWindowOpen="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceCallWindowOpen ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094LastCtxFlag40SourceDescriptorFlags="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094LastCtxFlag40SourceDescriptorFlags : 0)
        << " terminalRunnerExitGate78FrameUpdate9094TailFamilyActive="
        << (terminalValid && terminalLifecycle.runnerExitGate78FrameUpdate9094TailFamilyActive ? 1 : 0)
        << " terminalRunnerExitGate78FrameUpdate9094TailStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094TailStream : 0)
        << " terminalRunnerExitGate78FrameUpdate9094ActiveStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78FrameUpdate9094ActiveStream : 0)
        << " terminalRunnerExitGate78Ed1CProducerKnown="
        << (terminalValid && terminalLifecycle.runnerExitGate78Ed1CProducerKnown ? 1 : 0)
        << " terminalRunnerExitGate78Ed1CProducerEventStreamFlag="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78Ed1CProducerEventStreamFlag : 0)
        << " terminalRunnerExitGate78Ed1CProducerFlagDescriptorValid="
        << (terminalValid && terminalLifecycle.runnerExitGate78Ed1CProducerFlagDescriptorValid ? 1 : 0)
        << " terminalRunnerExitGate78Ed1CProducerActiveFlagStreamValid="
        << (terminalValid && terminalLifecycle.runnerExitGate78Ed1CProducerActiveFlagStreamValid ? 1 : 0)
        << " terminalRunnerExitGate78Ed1CProducerActiveFlagStreamIndex="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78Ed1CProducerActiveFlagStreamIndex : 0)
        << " terminalRunnerExitGate78Ed1CProducerActiveFlagStreamCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78Ed1CProducerActiveFlagStreamCount : 0)
        << " terminalRunnerExitGate78Ed1CProducerDueKnown="
        << (terminalValid && terminalLifecycle.runnerExitGate78Ed1CProducerDueKnown ? 1 : 0)
        << " terminalRunnerExitGate78Ed1CProducerDueFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78Ed1CProducerDueFrame : -1)
        << " terminalRunnerExitGate78Ed1CProducerDueDelta="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78Ed1CProducerDueDelta : 0)
        << " terminalRunnerExitGate78Ed1CProducerFlags04="
        << (terminalValid ? (int)terminalLifecycle.runnerExitGate78Ed1CProducerFlags04 : 0)
        << " terminalRunnerExitGate78Ed1CProducerFlag80="
        << (terminalValid && terminalLifecycle.runnerExitGate78Ed1CProducerFlag80 ? 1 : 0)
        << " terminalRunnerExitGate78Ed1CProducerConsumedFlagStreamEvent="
        << (terminalValid && terminalLifecycle.runnerExitGate78Ed1CProducerConsumedFlagStreamEvent ? 1 : 0)
        << " terminalRunnerExitGate78Ed1CProducerProduced="
        << (terminalValid && terminalLifecycle.runnerExitGate78Ed1CProducerProduced ? 1 : 0)
        << " terminalRunnerExitEventStreamFlagLastUpdateKnown="
        << (terminalValid && terminalLifecycle.runnerExitEventStreamFlagLastUpdateKnown ? 1 : 0)
        << " terminalRunnerExitEventStreamFlagLastUpdateReason="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastUpdateReason : 0)
        << " terminalRunnerExitEventStreamFlagLastUpdateQueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastUpdateQueryFrame : -1)
        << " terminalRunnerExitEventStreamFlagLastUpdateScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastUpdateScriptFrame : -1)
        << " terminalRunnerExitEventStreamFlagLastUpdatePrevious="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastUpdatePrevious : 0)
        << " terminalRunnerExitEventStreamFlagLastUpdateCurrent="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastUpdateCurrent : 0)
        << " terminalRunnerExitEventStreamFlagLastChangeKnown="
        << (terminalValid && terminalLifecycle.runnerExitEventStreamFlagLastChangeKnown ? 1 : 0)
        << " terminalRunnerExitEventStreamFlagLastChangeReason="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastChangeReason : 0)
        << " terminalRunnerExitEventStreamFlagLastChangeQueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastChangeQueryFrame : -1)
        << " terminalRunnerExitEventStreamFlagLastChangeScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastChangeScriptFrame : -1)
        << " terminalRunnerExitEventStreamFlagLastChangePrevious="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastChangePrevious : 0)
        << " terminalRunnerExitEventStreamFlagLastChangeCurrent="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastChangeCurrent : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearKnown="
        << (terminalValid && terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearKnown ? 1 : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearReason="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearReason : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearQueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearQueryFrame : -1)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearScriptFrame : -1)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearInputFlag="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearInputFlag : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearOutputFlag="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearOutputFlag : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearInputCtxFlags00="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearInputCtxFlags00 : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearOutputCtxFlags00="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearOutputCtxFlags00 : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearInputEd1C="
        << (terminalValid && terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearInputEd1C ? 1 : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearOutputEd1C="
        << (terminalValid && terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearOutputEd1C ? 1 : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearWord4E="
        << (terminalValid ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearWord4E : 0)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearDeltaToHandoff="
        << queryDeltaOrMissing(terminalRunnerClearKnown,
                               (int)terminalLifecycle.queryFrame,
                               terminalRunnerClearQuery)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearDeltaAfterLastReplayAppend="
        << queryDeltaOrMissing(terminalRunnerClearKnown &&
                                   numeric.rightRankLastReplayAppendKnown,
                               terminalRunnerClearQuery,
                               numeric.rightRankLastReplayAppendQueryFrame)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearDeltaAfterLastAcceptedTailCall="
        << queryDeltaOrMissing(terminalRunnerClearKnown &&
                                   numeric.rightRankLastAcceptedTailCallKnown,
                               terminalRunnerClearQuery,
                               numeric.rightRankLastAcceptedTailCallQueryFrame)
        << " terminalRunnerExitEventStreamFlagLastRunnerClearDeltaBeforeLastAcceptedTailDecision="
        << queryDeltaOrMissing(terminalRunnerClearKnown &&
                                   numeric.rightRankLastAcceptedTailDecisionKnown,
                               numeric.rightRankLastAcceptedTailDecisionQueryFrame,
                               terminalRunnerClearQuery)
        << " terminalRunnerExitFlag40="
        << (terminalValid && terminalLifecycle.runnerExitFlag40Active ? 1 : 0)
        << " terminalRunnerExitLateBranchSelectedStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLateBranchSelectedStream : 0)
        << " terminalRunnerExitLateBranchActiveDispatchStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLateBranchActiveDispatchStream : 0)
        << " terminalRunnerExitFlag100="
        << (terminalValid && terminalLifecycle.runnerExitFlag100BlocksWaitActive ? 1 : 0)
        << " terminalRunnerExitFlag100SourceStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFlag100SourceStream : 0)
        << " terminalRunnerExitFirstFlag100PulseKnown="
        << (terminalValid && terminalLifecycle.runnerExitFirstFlag100PulseKnown ? 1 : 0)
        << " terminalRunnerExitFirstFlag100PulseQueryFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstFlag100PulseQueryFrame : 0)
        << " terminalRunnerExitFirstFlag100PulseScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstFlag100PulseScriptFrame : 0)
        << " terminalRunnerExitFirstFlag100PulseSourceStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstFlag100PulseSourceStream : 0)
        << " terminalRunnerExitFirstFlag100PulseReason="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstFlag100PulseReason : 0)
        << " terminalRunnerExit9094ScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitLateBranchScriptFrame : -1)
        << " terminalRunnerExitClearTailPulse="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalTailPulseInput ? 1 : 0)
        << " terminalRunnerExitClearTailPulseArmed="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalTailPulseArmed ? 1 : 0)
        << " terminalRunnerExitClearTailPulseBlockAlreadyArmed="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalTailPulseBlockedAlreadyArmed ? 1 : 0)
        << " terminalRunnerExitClearTailPulseBlockActiveDispatch="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalTailPulseBlockedActiveDispatch ? 1 : 0)
        << " terminalRunnerExitClearTailPulseBlockPendingMismatch="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalTailPulseBlockedPendingMismatch ? 1 : 0)
        << " terminalRunnerExitClearTailPulseStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalTailPulseStream : 0)
        << " terminalRunnerExitClearTailTriggerAttempted="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerAttempted ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerAccepted="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerAccepted ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerScriptFrame : -1)
        << " terminalRunnerExitClearTailTriggerRightRankRow="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerRightRankRow : 0)
        << " terminalRunnerExitClearTailTriggerCurrentMode="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerCurrentMode : 0)
        << " terminalRunnerExitClearTailTriggerBlockConsumed="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedConsumed ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerBlockArmed="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedArmed ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerBlockFlagNotOne="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedFlagNotOne ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerBlockRow="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedRow ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerBlockStreamMissing="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedStreamMissing ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerBlockCursorDone="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedCursorDone ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerBlockEventNotDue="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedEventNotDue ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerBlockMissingFlag80="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80 ? 1 : 0)
        << " terminalRunnerExitClearTailTriggerStreamFlag="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerStreamFlag : 0)
        << " terminalRunnerExitClearTailTriggerAttemptCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerAttemptCount : 0)
        << " terminalRunnerExitClearTailTriggerAcceptedCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerAcceptedCount : 0)
        << " terminalRunnerExitClearTailTriggerEligibleCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerEligibleCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockFlagNotOneCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedFlagNotOneCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockRowCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedRowCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockFlagAndRowCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedFlagAndRowCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockConsumedCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedConsumedCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockArmedCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedArmedCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockStreamMissingCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedStreamMissingCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockCursorDoneCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedCursorDoneCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockEventNotDueCount="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedEventNotDueCount : 0)
        << " terminalRunnerExitClearTailTriggerBlockMissingFlag80Count="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalBranchTriggerBlockedMissingFlag80Count : 0)
        << " terminalRunnerExitFirstClearTailTriggerKnown="
        << (terminalValid && terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerKnown ? 1 : 0)
        << " terminalRunnerExitFirstClearTailTriggerScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerScriptFrame : -1)
        << " terminalRunnerExitFirstClearTailTriggerRightRankRow="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerRightRankRow : 0)
        << " terminalRunnerExitFirstClearTailTriggerCurrentMode="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerCurrentMode : 0)
        << " terminalRunnerExitFirstClearTailTriggerStreamFlag="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerStreamFlag : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleKnown="
        << (terminalValid && terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleKnown ? 1 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleScriptFrame : -1)
        << " terminalRunnerExitFirstClearTailTriggerEligibleRightRankRow="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleRightRankRow : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleCurrentMode="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleCurrentMode : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1Known="
        << (terminalValid && terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Known ? 1 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1Cursor="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Cursor : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1Count="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Count : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1DueKnown="
        << (terminalValid && terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueKnown ? 1 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1DueFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueFrame : -1)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1DueDelta="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1DueDelta : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1BaseFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1BaseFrame : -1)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1AbsDueFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueFrame : -1)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1AbsDueDelta="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1AbsDueDelta : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1Flags04="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Flags04 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1Byte29="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte29 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1Byte30="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1Byte30 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80SearchKnown="
        << (terminalValid && terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown ? 1 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Known="
        << (terminalValid && terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Known ? 1 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Cursor="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Frame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame : -1)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Delta="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80BaseFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80BaseFrame : -1)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80AbsFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsFrame : -1)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80AbsDelta="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80AbsDelta : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Flags04="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Flags04 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Byte29="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte29 : 0)
        << " terminalRunnerExitFirstClearTailTriggerEligibleStream1NextFlag80Byte30="
        << (terminalValid ? (int)terminalLifecycle.runnerExitFirstClearTerminalBranchTriggerEligibleStream1NextFlag80Byte30 : 0)
        << " terminalRunnerExitClearTailServiceLatch="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalTailServiceLatch ? 1 : 0)
        << " terminalRunnerExitClearTailServiceScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalTailServiceScriptFrame : -1)
        << " terminalRunnerExitClearTailServiceRightRankRow="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalTailServiceRightRankRow : 0)
        << " terminalRunnerExitClearTailServiceCurrentMode="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalTailServiceCurrentMode : 0)
        << " terminalRunnerExitClearTailServiceStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalTailServiceStream : 0)
        << " terminalRunnerExitPendingRatingBranchSeq="
        << (terminalValid ? (int)terminalLifecycle.runnerExitPendingRatingBranchSeq : 0)
        << " terminalRunnerExitConsumedRatingBranchSeq="
        << (terminalValid ? (int)terminalLifecycle.runnerExitConsumedRatingBranchSeq : 0)
        << " terminalRunnerExitClearTailArmed="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalTailArmed ? 1 : 0)
        << " terminalRunnerExitClearTailDispatchActive="
        << (terminalValid && terminalLifecycle.runnerExitClearTerminalTailDispatchActive ? 1 : 0)
        << " terminalRunnerExitClearTailStream="
        << (terminalValid ? (int)terminalLifecycle.runnerExitClearTerminalTailStream : 0)
        << " terminalRunnerExitActiveDispatchStartScriptFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitActiveDispatchStartScriptFrame : 0)
        << " terminalRunnerExitActiveDispatchTerminalEndLocalFrame="
        << (terminalValid ? (int)terminalLifecycle.runnerExitActiveDispatchTerminalEndLocalFrame : 0)
        << " terminalRunnerExitActiveDispatchTerminalPulseEmitted="
        << (terminalValid && terminalLifecycle.runnerExitActiveDispatchTerminalPulseEmitted ? 1 : 0)
        << " terminalRunnerTailRecordsModeDAActive="
        << (terminalValid && terminalLifecycle.runnerTailRecordsModeDAActive ? 1 : 0)
        << " terminalRunnerTailRecordsModeDAEqualsOne="
        << (terminalValid && terminalLifecycle.runnerTailRecordsModeDAEqualsOne ? 1 : 0)
        << " terminalRunnerTailStageStatusKnown="
        << (terminalValid && terminalLifecycle.runnerTailStageStatusKnown ? 1 : 0)
        << " terminalRunnerTailStageStatus166AC="
        << (terminalValid ? (int)terminalLifecycle.runnerTailStageStatus166AC : 0)
        << " terminalRunnerTailCleanupRenderPassBudget="
        << (terminalValid ? (int)terminalLifecycle.runnerTailCleanupRenderPassBudget : 0)
        << " terminalRunnerTailFinalReturnKnown="
        << (terminalValid && terminalLifecycle.runnerTailFinalReturnKnown ? 1 : 0)
        << " terminalRunnerTailFinalReturn="
        << (terminalValid ? (int)terminalLifecycle.runnerTailFinalReturn : -1)
        << " terminalRunnerTailFinalReturnWord59Known="
        << (terminalValid && terminalLifecycle.runnerTailFinalReturnWord59Known ? 1 : 0)
        << " terminalRunnerTailFinalReturnWord59IsOne="
        << (terminalValid && terminalLifecycle.runnerTailFinalReturnWord59IsOne ? 1 : 0)
        << " terminalRunnerTailFinalReturnWord60Known="
        << (terminalValid && terminalLifecycle.runnerTailFinalReturnWord60Known ? 1 : 0)
        << " terminalRunnerTailFinalReturnWord60IsOne="
        << (terminalValid && terminalLifecycle.runnerTailFinalReturnWord60IsOne ? 1 : 0)
        << " terminalRunnerTailFinalReturnCalls166AC="
        << (terminalValid && terminalLifecycle.runnerTailFinalReturnCalls166AC ? 1 : 0)
        << " terminalRunnerTailFinalReturnRecordsModeReturnsOne="
        << (terminalValid && terminalLifecycle.runnerTailFinalReturnRecordsModeReturnsOne ? 1 : 0)
        << " terminalRunnerTailFinalReturnStageStatusReturnsOne="
        << (terminalValid && terminalLifecycle.runnerTailFinalReturnStageStatusReturnsOne ? 1 : 0);
    out << "\nformula available=" << (numeric.active ? 1 : 0)
        << " query=" << (numeric.active ? (int)numeric.queryFrame : query)
        << " branchKnown=0 acceptedKnown=" << (numeric.active ? 1 : 0)
        << " accepted=" << acceptedFormulaCount
        << " acceptedContributionCountKnown=" << (numeric.active ? 1 : 0)
        << " acceptedContributionCount="
        << accepted.acceptedContributionCount
        << " recordedSplitKnown=" << (numeric.active ? 1 : 0)
        << " recordedSplit=" << accepted.recordedSplitCount
        << " penaltySplitKnown=" << (numeric.active ? 1 : 0)
        << " penaltySplit=" << accepted.penaltySideSplitCount
        << " aggregateAcceptedMaskKnown=" << (numeric.active ? 1 : 0)
        << " aggregateAcceptedMask=" << accepted.aggregateAcceptedMask
        << " add22Known=" << (numeric.active ? 1 : 0)
        << " add22=" << numeric.additiveLane.value;
    out << "\nobserver cueWindow="
        << (acceptedProbe.rawAcceptedMask != 0u ? 1 : 0)
        << " acceptedContribution=" << (acceptedProbe.materialized ? 1 : 0)
        << " rawPressedMask=" << acceptedProbe.rawPressedMask
        << " rawAcceptedMask=" << acceptedProbe.rawAcceptedMask
        << " controlWriterKind="
        << (int)acceptedCarrier.controlWriterSourceKind
        << " rawControl18=" << acceptedCarrier.rawControlSample18
        << " control18=" << acceptedCarrier.controlMask18
        << " class20=" << (int)acceptedCarrier.classToken20
        << " acceptedTickSeedQuery="
        << acceptedCarrier.acceptedTick96LastUpdateQueryFrame
        << " acceptedTickSeedKind="
        << (int)acceptedCarrier.acceptedTick96LastUpdateSourceKind
        << " acceptedTickSeedControl18="
        << acceptedCarrier.acceptedTick96LastUpdateControlMask18
        << " acceptedTickSeedClass20="
        << (int)acceptedCarrier.acceptedTick96LastUpdateClassToken20
        << " acceptedTickSeedPostCtx10="
        << (acceptedCarrier.acceptedTick96LastUpdateViaPostCtx10 ? 1 : 0)
        << " acceptedTickSeedCtxInput18="
        << acceptedCarrier.acceptedTick96LastUpdateCtxInput18
        << " acceptedTickSeedPrevMask="
        << acceptedCarrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8
        << " acceptedTickSeedMaskChanged="
        << (acceptedCarrier.acceptedTick96LastUpdateAcceptedMaskChanged ? 1 : 0)
        << " carrierCurrentTickProbeDisabled="
        << (acceptedCarrier.carrierCurrentTickProbeDisabled ? 1 : 0)
        << " carrierCurrentTickSeedAllowed="
        << (acceptedCarrier.carrierCurrentTickSeedAllowed ? 1 : 0)
        << " postCtxInput18="
        << numeric.runnerPostFrame7A60.ctxInput18
        << " postAcceptedMask9FF="
        << numeric.runnerPostFrame7A60.acceptedMask9FF
        << " postAcceptedGateOpen="
        << (numeric.runnerPostFrame7A60.acceptedGateOpen ? 1 : 0)
        << " postAcceptedMaskChanged="
        << (numeric.runnerPostFrame7A60.acceptedMaskChanged ? 1 : 0)
        << " postBackupCtx10="
        << (numeric.runnerPostFrame7A60.backupCtx10FromCtx0C ? 1 : 0)
        << " postCall14614="
        << (numeric.runnerPostFrame7A60.callAcceptedProducer14614 ? 1 : 0)
        << " initialPostKnown="
        << (acceptedProbe.initialPostKnown ? 1 : 0)
        << " initialPostCtxInput18="
        << acceptedProbe.initialPostCtxInput18
        << " initialPostPrevMask="
        << acceptedProbe.initialPostPreviousInputMask801CCBB8
        << " initialPostMask9FF="
        << acceptedProbe.initialPostAcceptedMask9FF
        << " initialPostGateOpen="
        << (acceptedProbe.initialPostAcceptedGateOpen ? 1 : 0)
        << " initialPostMaskChanged="
        << (acceptedProbe.initialPostAcceptedMaskChanged ? 1 : 0)
        << " initialPostBackupCtx10="
        << (acceptedProbe.initialPostBackupCtx10FromCtx0C ? 1 : 0)
        << " initialPostCall14614="
        << (acceptedProbe.initialPostCallAcceptedProducer14614 ? 1 : 0)
        << " frontDoorCurrentTickProbeDisabled="
        << (acceptedProbe.frontDoorCurrentTickProbeDisabled ? 1 : 0)
        << " frontDoorCurrentTickSeedAllowed="
        << (acceptedProbe.frontDoorCurrentTickSeedAllowed ? 1 : 0)
        << " direct14614Captured="
        << (acceptedProbe.directAcceptedRunCaptured ? 1 : 0)
        << " direct14614Result="
        << acceptedProbe.directAcceptedRunResultCode
        << " direct14614ReplayAppend="
        << (acceptedProbe.directAcceptedRunReplayAppendRan ? 1 : 0)
        << " direct14614WriteRan="
        << (acceptedProbe.directAcceptedRunWriteRan ? 1 : 0)
        << " direct14614WriteResult="
        << acceptedProbe.directAcceptedRunWriteResultCode
        << " direct14614Selector="
        << (acceptedProbe.directAcceptedRunSelectorResolved ? 1 : 0)
        << " direct14614TimingKnown="
        << (acceptedProbe.directAcceptedRunTimingTemplateKnown ? 1 : 0)
        << " direct14614TimingAddr=0x" << std::hex
        << acceptedProbe.directAcceptedRunTimingTemplateAddress << std::dec
        << " direct14614TimingSlot48="
        << (int)acceptedProbe.directAcceptedRunTimingTemplateSlot48
        << " direct14614TimingState="
        << (int)acceptedProbe.directAcceptedRunTimingTemplateState
        << " direct14614SourceCell="
        << (acceptedProbe.directAcceptedRunSourceCellValid ? 1 : 0)
        << " direct14614AcceptedTick96Known="
        << (acceptedProbe.directAcceptedRunAcceptedTick96Known ? 1 : 0)
        << " direct14614AcceptedTick96="
        << acceptedProbe.directAcceptedRunAcceptedTick96
        << " direct14614Phase384="
        << acceptedProbe.directAcceptedRunPhase384
        << " direct14614RecordSlot24="
        << (int)acceptedProbe.directAcceptedRunRecordSlot24
        << " direct14614RecordRem24="
        << (int)acceptedProbe.directAcceptedRunRecordRemainder24
        << " direct14614SourceCellCursor="
        << acceptedProbe.directAcceptedRunSourceCellCursor
        << " direct14614PageWriteApplied="
        << (acceptedProbe.directAcceptedRunPageWriteApplied ? 1 : 0)
        << " direct14614WritePage38="
        << acceptedProbe.directAcceptedRunWritePageOrdinal
        << " direct14614PageRecordSlot="
        << (int)acceptedProbe.directAcceptedRunPageRecordSlot
        << " direct14614DescSubstate50="
        << acceptedProbe.directInputDescriptorSubstate50
        << " direct14614RowValid="
        << (acceptedProbe.directInputLookaheadRowValid ? 1 : 0)
        << " direct14614RowLesson="
        << (int)acceptedProbe.directInputLookaheadLessonId
        << " direct14614DefaultSel0="
        << (int)acceptedProbe.directInputDefaultSelectorByte0
        << " direct14614DefaultSel1="
        << (int)acceptedProbe.directInputDefaultSelectorByte1
        << " direct14614SubstateSel0="
        << (int)acceptedProbe.directInputSubstate1SelectorByte0
        << " direct14614SubstateSel1="
        << (int)acceptedProbe.directInputSubstate1SelectorByte1
        << " direct14614BranchSel0="
        << (int)acceptedProbe.directInputBranchSelectorByte0
        << " direct14614BranchSel1="
        << (int)acceptedProbe.directInputBranchSelectorByte1
        << " steadyKnown=" << (numeric.steadyInput7A60.known ? 1 : 0)
        << " steadyReplay52=" << (numeric.steadyInput7A60.replayMode52 ? 1 : 0)
        << " steadyHeldMask=" << numeric.steadyInput7A60.heldMask
        << " steadyWriteCtx18=" << (numeric.steadyInput7A60.writeCtx18 ? 1 : 0)
        << " steadyCtx18=" << numeric.steadyInput7A60.ctx18Value
        << " steadyWriteCtx20=" << (numeric.steadyInput7A60.writeCtx20 ? 1 : 0)
        << " steadyCtx20Known=" << (numeric.steadyInput7A60.ctx20Known ? 1 : 0)
        << " steadyCtx20=" << numeric.steadyInput7A60.ctx20Value
        << " steadyWriteCtx10Current="
        << (numeric.steadyInput7A60.writeCtx10CurrentTick ? 1 : 0)
        << " steadyWriteCtx10Replay="
        << (numeric.steadyInput7A60.writeCtx10ReplayTick ? 1 : 0)
        << " steadyCtx10ReplayTick=" << numeric.steadyInput7A60.ctx10ReplayTick
        << " srcEvtAvail=" << (numeric.acceptedInputSourceEvent.available ? 1 : 0)
        << " srcEvtQ=" << numeric.acceptedInputSourceEvent.queryFrame
        << " srcEvtTick96=" << numeric.acceptedInputSourceEvent.tick96
        << " srcEvtHeldMask=" << numeric.acceptedInputSourceEvent.heldMask
        << " srcEvtCtx18=" << numeric.acceptedInputSourceEvent.ctx18Value
        << " srcEvtWriteCtx10Current="
        << (numeric.acceptedInputSourceEvent.writeCtx10CurrentTick ? 1 : 0)
        << " srcEvtPostGate="
        << (numeric.acceptedInputSourceEvent.postAcceptedGateOpen ? 1 : 0)
        << " srcEvtPostChanged="
        << (numeric.acceptedInputSourceEvent.postAcceptedMaskChanged ? 1 : 0)
        << " srcEvtPostBackup="
        << (numeric.acceptedInputSourceEvent.postBackupCtx10 ? 1 : 0)
        << " probeClass20=" << (int)acceptedProbe.classToken
        << " selectorByte0=" << (int)acceptedProbe.selectorByte0
        << " selectorByte1=" << (int)acceptedProbe.selectorByte1
        << " halfWindow34=" << (int)acceptedCarrier.halfWindow34
        << " substate50=" << (int)acceptedCarrier.substate50
        << " replayMode52=" << (numeric.ctx52ReplayMode7A60 ? 1 : 0)
        << " phase384=" << acceptedCarrier.phase384
        << " recordSlot24=" << (int)acceptedCarrier.recordSlot24
        << " recordRem24=" << (int)acceptedCarrier.recordRemainder24
        << " slot48=" << (int)acceptedCarrier.timingTemplateSlot48
        << " writePage38=" << acceptedCarrier.writePageOrdinal38
        << " lastWriteAvailable=" << (lastWrite.available ? 1 : 0)
        << " lastWritePage38=" << lastWrite.writePageOrdinal38
        << " lastWriteSlot24=" << (int)lastWrite.recordSlot24
        << " lastWriteMask=" << lastWrite.acceptedMask
        << " lastWriteComp=" << lastWrite.pageCompanion
        << " count91810=" << scorer.acceptedCount91810
        << " recorded91812=" << scorer.recordedHitCount91812
        << " penalty91814=" << scorer.penaltySideCount91814
        << " mask91808=" << scorer.aggregateAcceptedMask91808
        << " comp91824=" << scorer.recordCompanion91824
        << " clearMask9180C=" << scorer.lastClearedAcceptedMask9180C
        << " selectorAvailable="
        << (acceptedProbe.selectorAvailable ? 1 : 0)
        << " timingTemplateState="
        << (int)acceptedProbe.timingTemplateState
        << " sourceCellGate="
        << (acceptedProbe.sourceCellGateActive ? 1 : 0)
        << " materialized=" << (acceptedProbe.materialized ? 1 : 0)
        << " carryReplayed=" << (acceptedProbe.carryReplayed ? 1 : 0)
        << " streamFlagKnown="
        << (acceptedCarrier.eventStreamFlagKnown ? 1 : 0)
        << " streamFlag="
        << (acceptedCarrier.eventStreamFlagActive ? 1 : 0)
        << " streamIdKnown="
        << (acceptedCarrier.eventStreamIdRawKnown ? 1 : 0)
        << " streamIdRaw=" << (int)acceptedCarrier.eventStreamIdRaw
        << " busyGateKnown="
        << (acceptedCarrier.busyGate24BF4Known ? 1 : 0)
        << " busyGate="
        << (acceptedCarrier.busyGate24BF4Active ? 1 : 0)
        << " acceptedGateKnown="
        << (acceptedCarrier.acceptedGateKnown ? 1 : 0)
        << " acceptedGate="
        << (acceptedCarrier.acceptedGateActive ? 1 : 0)
        << " acceptedTick96Known="
        << (acceptedCarrier.acceptedTick96Known ? 1 : 0)
        << " acceptedTick96=" << acceptedCarrier.acceptedTick96
        << " acceptedSplitId=" << (int)acceptedProbe.split
        << " acceptedSplit=" << acceptedSplitName
        << " sourceCellCursor=" << acceptedProbe.sourceCellCursor
        << " voiceActive=" << (sourceCell.active ? 1 : 0)
        << " voiceProgram=" << (int)sourceCell.program
        << " voiceNote=" << (int)sourceCell.note
        << " voiceKey=" << (int)sourceCell.key
        << " voiceVolume=" << (int)sourceCell.volume
        << " voiceReplaceRestartPulse="
        << (sourceCell.replaceRestartPulse ? 1 : 0)
        << " runnerFrame=" << g_dbg_stageFrame
        << " audioFrame30=" << g_dbg_stageFrame
        << " audioFrame60=" << (g_dbg_stageFrame * 2)
        << " commonLyricsAvailable=0 commonLyricsSource=none";
    out << "\ncommonLyrics available=0 producerActive=0 source=none";
    out << "\nscriptText producerActive=0 source=none language=0 textId=0 timeout=0";
    return out.str();
}

static std::string Stage1OverlayDescribeXaCd359B8SourceAudit(PrGameContext& prCtx) {
    PrStage1RuntimeSlotsSnapshot slots{};
    const bool slotsValid = PrScn1::GetStage1RuntimeSlotsSnapshot(prCtx, slots) &&
                            slots.valid;
    const int query = slotsValid ? static_cast<int>(slots.queryFrame) : -1;
    auto& xaCd = prCtx.stage1XaCdDirect;
    const auto status57108Window =
        Stage1ReadStatus57108RuntimeSourceWindow(prCtx);
    const auto status57110Window =
        Stage1ReadStatus57110RuntimeSourceWindow(prCtx);
    const auto status57108SourceReadAudit =
        PrMain::AuditStage1RuntimePsxMemorySources(prCtx, 0x80057108u, 4u);
    const auto status57110SourceReadAudit =
        PrMain::AuditStage1RuntimePsxMemorySources(prCtx, 0x80057110u, 4u);
    const auto status57108Source =
        PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(
            status57108Window);
    const auto status57108SourceResult =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(
            status57108Source,
            xaCd);
    const auto status57110Source =
        PrStage1XaCdDirectBuildStatusCounterRuntimeSource80057110(
            status57110Window);
    const auto status57110SourceResult =
        PrStage1XaCdDirectPublishStatusCounterRuntimeSource80057110(
            status57110Source,
            xaCd);
    const auto rawEventWindow =
        PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(
            prCtx.stage1RuntimePsxMemoryProvider);
    const auto rawEventInitialInterruptSource =
        PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
            rawEventWindow);
    const auto rawEventInitialInterruptSourceResult =
        PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
            rawEventInitialInterruptSource,
            xaCd);
    const auto rawEventSource =
        PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(xaCd);
    const auto rawEventSourceResult =
        PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(
            rawEventSource,
            xaCd,
            &prCtx.stage1RuntimePsxMemoryProvider);
    PrStage1XaCdDirectCdMmioRuntimeProducerSample cdMmioProducerSample{};
    cdMmioProducerSample.frameKnown =
        prCtx.stage1RuntimePsxMemoryProvider.frameKnown;
    cdMmioProducerSample.frame = prCtx.stage1RuntimePsxMemoryProvider.frame;
    cdMmioProducerSample.pcKnown = true;
    cdMmioProducerSample.pc = 0x80036AF8u;
    const auto cdMmioProducerResult =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSample(
            cdMmioProducerSample,
            prCtx.stage1RuntimePsxMemoryProvider);
    const auto cdMmioSnapshotAudit =
        PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(
            prCtx.stage1RuntimePsxMemoryProvider);
    const auto cdMmioReadPathFirstMissing =
        [&](bool windowReadable) -> const char* {
        if (windowReadable) {
            return "none";
        }
        if (!prCtx.stage1RuntimePsxMemoryProvider.cdMmioSourceInstalled) {
            return "cdMmioSource";
        }
        if (prCtx.stage1RuntimePsxMemoryProvider.cdMmioRead == nullptr) {
            return "cdMmioReadFn";
        }
        if (cdMmioSnapshotAudit.producerObservationCallCount == 0u) {
            return "cdMmioProducerCall";
        }
        return "cdMmioWindow";
    };
    PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8
        xacd359b8RuntimeWindow{};
    const auto xacd359b8RuntimeSourceResult =
        Stage1PublishInterruptSnapshotRuntimeSource(
            prCtx,
            xaCd,
            &xacd359b8RuntimeWindow);
    const auto typedSource =
        PrStage1XaCdDirectAuditInterruptSnapshotTypedSource800359B8(xaCd);

    std::ostringstream out;
    out << "xacd359b8SourceAudit scene="
        << static_cast<int>(prCtx.currentScene)
        << " stageRunning=" << (prCtx.stageRunning ? 1 : 0)
        << " query=" << query
        << " debugChannel=stage1ovl"
        << " runtimePsxMemorySource=0"
        << " runtimePsxMemoryProviderInstalled="
        << ((status57108Window.providerInstalled ||
             status57110Window.providerInstalled ||
             xacd359b8RuntimeWindow.providerInstalled)
                ? 1
                : 0)
        << " runtimePsxMemoryProviderKind="
        << ((status57108Window.providerInstalled ||
             status57110Window.providerInstalled ||
             xacd359b8RuntimeWindow.providerInstalled)
                ? "stage1-loader-heap+xacd-known-state"
                : "none")
        << " runtimePsxMemoryLoaderHeapSourceInstalled="
        << (prCtx.stage1RuntimePsxMemoryProvider.loaderHeapSourceInstalled ? 1
                                                                           : 0)
        << " runtimePsxMemoryKnownStateRelayInstalled="
        << (prCtx.stage1RuntimePsxMemoryProvider.xaCdKnownStateRelayInstalled
                ? 1
                : 0)
        << " runtimePsxMemoryCdMmioSourceInstalled="
        << (prCtx.stage1RuntimePsxMemoryProvider.cdMmioSourceInstalled ? 1
                                                                       : 0)
        << " runtimePsxMemoryCdMmioReadFnInstalled="
        << (prCtx.stage1RuntimePsxMemoryProvider.cdMmioRead != nullptr ? 1
                                                                       : 0)
        << " runtimePsxMemoryCdMmioUserDataInstalled="
        << (prCtx.stage1RuntimePsxMemoryProvider.cdMmioUserData != nullptr
                ? 1
                : 0)
        << " runtimePsxMemoryCdMmioSnapshotSource="
        << (cdMmioSnapshotAudit.snapshotSource ? 1 : 0)
        << " runtimePsxMemoryCdMmioSnapshotIngressOnly=1"
        << " runtimePsxMemoryCdMmioProducerIngressInstalled="
        << (cdMmioSnapshotAudit.producerIngressInstalled ? 1 : 0)
        << " runtimePsxMemoryCdMmioProviderSubmitAvailable="
        << (cdMmioSnapshotAudit.providerSubmitAvailable ? 1 : 0)
        << " runtimePsxMemoryCdMmioProducerCallsiteAttempted="
        << (cdMmioProducerResult.attempted ? 1 : 0)
        << " runtimePsxMemoryCdMmioProducerCallsiteSourceAvailable="
        << (cdMmioProducerResult.sourceAvailable ? 1 : 0)
        << " runtimePsxMemoryCdMmioProducerCallsiteValueKnown="
        << (cdMmioProducerResult.valueKnown ? 1 : 0)
        << " runtimePsxMemoryCdMmioProducerCallsitePublishAttempted="
        << (cdMmioProducerResult.publishAttempted ? 1 : 0)
        << " runtimePsxMemoryCdMmioProducerCallsiteAccepted="
        << (cdMmioProducerResult.accepted ? 1 : 0)
        << " runtimePsxMemoryCdMmioProducerCallsiteReject="
        << static_cast<int>(cdMmioProducerResult.rejectReason)
        << " runtimePsxMemoryCdMmioProducerObservationCalls="
        << cdMmioSnapshotAudit.producerObservationCallCount
        << " runtimePsxMemoryCdMmioNaturalProducer=0"
        << " runtimePsxMemoryCdMmioNaturalAdapterAttemptCount="
        << xaCd.cdMmioNaturalAdapterAttemptCount
        << " runtimePsxMemoryCdMmioNaturalAdapterSourceAvailable=0"
        << " runtimePsxMemoryCdMmioNaturalAdapterSourceUnavailableCount="
        << xaCd.cdMmioNaturalAdapterSourceUnavailableCount
        << " runtimePsxMemoryCdMmioNaturalAdapterPublishAttemptCount="
        << xaCd.cdMmioNaturalAdapterPublishAttemptCount
        << " runtimePsxMemoryCdMmioNaturalAdapterAcceptedCount="
        << xaCd.cdMmioNaturalAdapterAcceptedCount
        << " runtimePsxMemoryCdMmioNaturalAdapterReject="
        << static_cast<int>(xaCd.cdMmioNaturalAdapterLastReject)
        << " runtimePsxMemoryCdMmioProducerGap=missingWindowsCdMmioSampleProducer"
        << " runtimePsxMemoryCdMmioCdReg3InitialKnown="
        << (cdMmioSnapshotAudit.cdReg3InitialKnown ? 1 : 0)
        << " runtimePsxMemoryCdMmioCdReg3Initial=0x" << std::hex
        << static_cast<unsigned>(cdMmioSnapshotAudit.cdReg3Initial) << std::dec
        << " runtimePsxMemoryCdMmioCdReg0StatusKnown="
        << (cdMmioSnapshotAudit.cdReg0StatusKnown ? 1 : 0)
        << " runtimePsxMemoryCdMmioCdReg0Status=0x" << std::hex
        << static_cast<unsigned>(cdMmioSnapshotAudit.cdReg0Status) << std::dec
        << " runtimePsxMemoryCdMmioCanFeedCdReg3Initial="
        << (cdMmioSnapshotAudit.canFeedCdReg3Initial ? 1 : 0)
        << " runtimePsxMemoryCdMmioCanFeedCdReg0Status="
        << (cdMmioSnapshotAudit.canFeedCdReg0Status ? 1 : 0)
        << " runtimePsxMemoryCdMmioSamplePairAvailable="
        << (cdMmioSnapshotAudit.samplePairAvailable ? 1 : 0)
        << " runtimePsxMemoryCdMmioObsAccepted="
        << cdMmioSnapshotAudit.observationAcceptedCount
        << " runtimePsxMemoryCdMmioObsRejected="
        << cdMmioSnapshotAudit.observationRejectedCount
        << " runtimePsxMemoryCdMmioObsReject="
        << static_cast<int>(cdMmioSnapshotAudit.lastRejectReason)
        << " runtimePsxMemoryExactCdSourceInstalled="
        << (prCtx.stage1RuntimePsxMemoryProvider.exactCdSourceInstalled ? 1
                                                                        : 0)
        << " runtimePsxMemoryExactCdSourceReadFnInstalled="
        << (prCtx.stage1RuntimePsxMemoryProvider.exactCdRead != nullptr ? 1
                                                                        : 0)
        << " runtimePsxMemoryExactCdSourceUserDataInstalled="
        << (prCtx.stage1RuntimePsxMemoryProvider.exactCdUserData != nullptr
                ? 1
                : 0)
        << " runtimePsxMemoryProviderCanReadExactWindows="
        << ((status57108Window.windowReadable &&
             xacd359b8RuntimeWindow.bundleReadable)
                ? 1
                : 0)
        << " cbStateAddr=0x" << std::hex << 0x80055F78u << std::dec
        << " cbStateBytes=52"
        << " cbStateReadAttempted="
        << (xacd359b8RuntimeWindow.callbackStateReadAttempted ? 1 : 0)
        << " cbStateReadable="
        << (xacd359b8RuntimeWindow.callbackStateReadable ? 1 : 0)
        << " initialRegsAddr=0x" << std::hex << 0x1F801070u << std::dec
        << " initialRegsBytes=8"
        << " initialRegsReadAttempted="
        << (xacd359b8RuntimeWindow.initialRegsReadAttempted ? 1 : 0)
        << " initialRegsReadable="
        << (xacd359b8RuntimeWindow.initialRegsReadable ? 1 : 0)
        << " terminalRegsAddr=0x" << std::hex << 0x1F801070u << std::dec
        << " terminalRegsBytes=8"
        << " terminalRegsReadAttempted="
        << (xacd359b8RuntimeWindow.terminalRegsReadAttempted ? 1 : 0)
        << " terminalRegsReadable="
        << (xacd359b8RuntimeWindow.terminalRegsReadable ? 1 : 0)
        << " watchdogAddr=0x" << std::hex << 0x80057010u << std::dec
        << " watchdogBytes=4"
        << " watchdogReadAttempted="
        << (xacd359b8RuntimeWindow.watchdogReadAttempted ? 1 : 0)
        << " watchdogReadable="
        << (xacd359b8RuntimeWindow.watchdogReadable ? 1 : 0)
        << " bundleReadable="
        << (xacd359b8RuntimeWindow.bundleReadable ? 1 : 0)
        << " runtimeSourceAdapter=1"
        << " runtimeSourceAvailable="
        << (xacd359b8RuntimeSourceResult.sourceAvailable ? 1 : 0)
        << " runtimeSourceBundleKnown="
        << (xacd359b8RuntimeSourceResult.bundleKnown ? 1 : 0)
        << " runtimeSourcePublishAttempted="
        << (xacd359b8RuntimeSourceResult.publishAttempted ? 1 : 0)
        << " status57108RuntimeSourceAdapter=1"
        << " status57108RuntimeSourceProviderInstalled="
        << (status57108Window.providerInstalled ? 1 : 0)
        << " status57108RuntimeSourceReadAttempted="
        << (status57108Window.readAttempted ? 1 : 0)
        << " status57108RuntimeSourceWindowReadable="
        << (status57108Window.windowReadable ? 1 : 0)
        << " status57108LoaderHeapReadAttempted="
        << (status57108SourceReadAudit.loaderHeapReadAttempted ? 1 : 0)
        << " status57108LoaderHeapReadable="
        << (status57108SourceReadAudit.loaderHeapReadable ? 1 : 0)
        << " status57108KnownStateRelayReadAttempted="
        << (status57108SourceReadAudit.knownStateRelayReadAttempted ? 1 : 0)
        << " status57108KnownStateRelayReadable="
        << (status57108SourceReadAudit.knownStateRelayReadable ? 1 : 0)
        << " status57108CdMmioSnapshotReadAttempted="
        << (status57108SourceReadAudit.cdMmioSnapshotReadAttempted ? 1 : 0)
        << " status57108CdMmioSnapshotReadable="
        << (status57108SourceReadAudit.cdMmioSnapshotReadable ? 1 : 0)
        << " status57108ExactCdReadAttempted="
        << (status57108SourceReadAudit.exactCdReadAttempted ? 1 : 0)
        << " status57108ExactCdReadable="
        << (status57108SourceReadAudit.exactCdReadable ? 1 : 0)
        << " status57108KnownStateRelayKnown="
        << (xaCd.dword_80057108Known ? 1 : 0)
        << " status57108KnownStateRelayCanFeed="
        << (xaCd.dword_80057108Known ? 1 : 0)
        << " status57108ExactCdFallbackSupportsWindow=0"
        << " status57108ReadPathFirstMissing="
        << cdMmioReadPathFirstMissing(status57108Window.windowReadable)
        << " status57108FirstPriorSourceKnown="
        << (status57108Window.windowReadable ? 1 : 0)
        << " status57108FirstPriorSource="
        << (status57108Window.windowReadable
                ? "runtimePsxMemoryObservation"
                : "missingRuntimePsxMemoryObservation")
        << " status57108RawEventRequiresPrior=1"
        << " status57108RawEventCanBootstrapPrior=0"
        << " status57108FirstProducerGap="
        << (status57108Window.windowReadable
                ? "none"
                : "exactStatus57108RuntimeSource")
        << " status57108HostCdBackendCanFeed=0"
        << " status57108XaPlayerCanFeed=0"
        << " status57108ExactCdFallbackCanFeed=0"
        << " cdReg3InitialHostCdBackendCanFeed=0"
        << " cdReg3InitialXaPlayerCanFeed=0"
        << " cdReg3InitialExactCdFallbackRequiresAcceptedObservation=1"
        << " cdReg3InitialExactCdFallbackCanCreate=0"
        << " status57108RuntimeSourceAvailable="
        << (status57108SourceResult.sourceAvailable ? 1 : 0)
        << " status57108RuntimeSourceValueKnown="
        << (status57108SourceResult.valueKnown ? 1 : 0)
        << " status57108RuntimeSourcePublishAttempted="
        << (status57108SourceResult.publishAttempted ? 1 : 0)
        << " status57108RuntimeSourceBlocker="
        << static_cast<int>(status57108SourceResult.blocker)
        << " status57108RuntimeSourceAccepted="
        << (status57108SourceResult.observation.accepted ? 1 : 0)
        << " status57108RuntimeSourceReject="
        << static_cast<int>(status57108SourceResult.observation.rejectReason)
        << " status57110RuntimeSourceAdapter=1"
        << " status57110RuntimeSourceProviderInstalled="
        << (status57110Window.providerInstalled ? 1 : 0)
        << " status57110RuntimeSourceReadAttempted="
        << (status57110Window.readAttempted ? 1 : 0)
        << " status57110RuntimeSourceWindowReadable="
        << (status57110Window.windowReadable ? 1 : 0)
        << " status57110LoaderHeapReadAttempted="
        << (status57110SourceReadAudit.loaderHeapReadAttempted ? 1 : 0)
        << " status57110LoaderHeapReadable="
        << (status57110SourceReadAudit.loaderHeapReadable ? 1 : 0)
        << " status57110KnownStateRelayReadAttempted="
        << (status57110SourceReadAudit.knownStateRelayReadAttempted ? 1 : 0)
        << " status57110KnownStateRelayReadable="
        << (status57110SourceReadAudit.knownStateRelayReadable ? 1 : 0)
        << " status57110CdMmioSnapshotReadAttempted="
        << (status57110SourceReadAudit.cdMmioSnapshotReadAttempted ? 1 : 0)
        << " status57110CdMmioSnapshotReadable="
        << (status57110SourceReadAudit.cdMmioSnapshotReadable ? 1 : 0)
        << " status57110ExactCdReadAttempted="
        << (status57110SourceReadAudit.exactCdReadAttempted ? 1 : 0)
        << " status57110ExactCdReadable="
        << (status57110SourceReadAudit.exactCdReadable ? 1 : 0)
        << " status57110KnownStateRelayKnown="
        << (xaCd.dword_80057110Known ? 1 : 0)
        << " status57110KnownStateRelayCanFeed="
        << (xaCd.dword_80057110Known ? 1 : 0)
        << " status57110ReadPathFirstMissing="
        << cdMmioReadPathFirstMissing(status57110Window.windowReadable)
        << " status57110FirstPriorSourceKnown="
        << (status57110Window.windowReadable ? 1 : 0)
        << " status57110FirstPriorSource="
        << (status57110Window.windowReadable
                ? "runtimePsxMemoryObservation"
                : "missingRuntimePsxMemoryObservation")
        << " status57110RawEventRequiresPrior=1"
        << " status57110RawEventCanBootstrapPrior=0"
        << " status57110FirstProducerGap="
        << (status57110Window.windowReadable
                ? "none"
                : "exactStatus57110RuntimeSource")
        << " status57110HostCdBackendCanFeed=0"
        << " status57110XaPlayerCanFeed=0"
        << " status57110ExactCdFallbackCanFeed=0"
        << " status57110RuntimeSourceAvailable="
        << (status57110SourceResult.sourceAvailable ? 1 : 0)
        << " status57110RuntimeSourceValueKnown="
        << (status57110SourceResult.valueKnown ? 1 : 0)
        << " status57110RuntimeSourceValue=0x" << std::hex
        << status57110Window.value << std::dec
        << " status57110RuntimeSourcePublishAttempted="
        << (status57110SourceResult.publishAttempted ? 1 : 0)
        << " status57110RuntimeSourceBlocker="
        << static_cast<int>(status57110SourceResult.blocker)
        << " status57110RuntimeSourceAccepted="
        << (status57110SourceResult.observation.accepted ? 1 : 0)
        << " status57110RuntimeSourceReject="
        << static_cast<int>(status57110SourceResult.observation.rejectReason)
        << " typedSourceAudit=" << (typedSource.inspected ? 1 : 0)
        << " typedCallbackStateBase=0x" << std::hex
        << typedSource.callbackStatePsxAddress << std::dec
        << " typedCallbackStateBytes=" << typedSource.callbackStateByteSize
        << " typedCallbackSlotBase=0x" << std::hex
        << typedSource.callbackSlotBasePsxAddress << std::dec
        << " typedCallbackSlotBytes=" << typedSource.callbackSlotByteSize
        << " typedCallbackSlotsKnown="
        << (typedSource.callbackSlotsKnown ? 1 : 0)
        << " typedCallbackSlotsAreCallbackStateTable="
        << (typedSource.callbackSlotsAreCallbackStateTable ? 1 : 0)
        << " typedCallbackSlot570F8K="
        << (xaCd.dword_800570F8Known ? 1 : 0)
        << " typedCallbackSlot570F8=0x" << std::hex << xaCd.dword_800570F8
        << std::dec
        << " typedCallbackSlot570FCK="
        << (xaCd.dword_800570FCKnown ? 1 : 0)
        << " typedCallbackSlot570FC=0x" << std::hex << xaCd.dword_800570FC
        << std::dec
        << " typedCallbackStateWordsKnown="
        << (typedSource.callbackStateWordsKnown ? 1 : 0)
        << " typedCallbackStateTableKnown="
        << (typedSource.callbackStateTableKnown ? 1 : 0)
        << " typedInterruptRegsInitialKnown="
        << (typedSource.initialInterruptRegsKnown ? 1 : 0)
        << " typedInterruptRegsTerminalKnown="
        << (typedSource.terminalInterruptRegsKnown ? 1 : 0)
        << " typedWord55F78K=" << (xaCd.word_80055F78Known ? 1 : 0)
        << " typedWord55F7AK=" << (xaCd.word_80055F7AKnown ? 1 : 0)
        << " typedWord55FA8K=" << (xaCd.word_80055FA8Known ? 1 : 0)
        << " typedWatchdog57010K="
        << (xaCd.dword_80057010Known ? 1 : 0)
        << " typedMissingMask=" << typedSource.missingMask
        << " typedCanReconstructBundle="
        << (typedSource.typedCanReconstructBundle ? 1 : 0)
        << " typedReconstructGap=callbackStateTable,initialRegs,terminalRegs,watchdog"
        << " canFeedObservation="
        << (typedSource.canFeedRuntimeObservation ? 1 : 0)
        << " observationFeedAttempted=0"
        << " xaCdGetlocBridge=" << xaCd.halGetlocLowerBridgeCount
        << " xaCdGetlocBridgeCore=" << xaCd.halGetlocLowerBridgeCdSyncCoreCount
        << " xaCdGetlocBridgePending="
        << xaCd.halGetlocLowerBridgePendingProducerCount
        << " xaCd36AF8HostXaRawSectorAudit=1"
        << " xaCd36AF8HostXaPlayerPresent="
        << (prCtx.xa1Player != nullptr ? 1 : 0)
        << " xaCd36AF8HostXaAcceptedRawPushCount="
        << (prCtx.xa1Player ? prCtx.xa1Player->GetAcceptedRawPushCount() : 0u)
        << " xaCd36AF8HostXaAcceptedRawPopCount="
        << (prCtx.xa1Player ? prCtx.xa1Player->GetAcceptedRawPopCount() : 0u)
        << " xaCd36AF8HostXaAcceptedRawQueueSize="
        << (prCtx.xa1Player
                ? static_cast<unsigned>(
                      prCtx.xa1Player->GetAcceptedRawSectorQueueSize())
                : 0u)
        << " xaCd36AF8HostXaLastAcceptedKnown="
        << (prCtx.xa1Player && prCtx.xa1Player->GetLastAcceptedSectorKnown()
                ? 1
                : 0)
        << " xaCd36AF8HostXaLastAcceptedSector="
        << (prCtx.xa1Player ? prCtx.xa1Player->GetLastAcceptedSectorIndex()
                            : 0u)
        << " xaCd36AF8HostXaLastAcceptedFile="
        << (prCtx.xa1Player
                ? static_cast<unsigned>(prCtx.xa1Player->GetLastAcceptedFile())
                : 0u)
        << " xaCd36AF8HostXaLastAcceptedChannel="
        << (prCtx.xa1Player
                ? static_cast<unsigned>(
                      prCtx.xa1Player->GetLastAcceptedChannel())
                : 0u)
        << " xaCd36AF8HostXaLastAcceptedCoding="
        << (prCtx.xa1Player
                ? static_cast<unsigned>(
                      prCtx.xa1Player->GetLastAcceptedCoding())
                : 0u)
        << " xaCd36AF8HostXaLastPolledKnown="
        << (prCtx.xa1Player && prCtx.xa1Player->GetLastPolledSectorKnown()
                ? 1
                : 0)
        << " xaCd36AF8HostXaLastPolledSector="
        << (prCtx.xa1Player ? prCtx.xa1Player->GetLastPolledSectorIndex()
                            : 0u)
        << " xaCd36AF8HostXaRawSectorCanFeedCdReg3Initial=0"
        << " xaCd36AF8HostXaRawSectorCanFeedCdReg0Status=0"
        << " xaCd36AF8HostXaRawSectorCanFeedRawTransaction=0"
        << " xaCd36AF8HostGetlocDataReadyCandidate=1"
        << " xaCd36AF8HostGetlocApplyCount="
        << xaCd.halGetlocPFactsApplyCount
        << " xaCd36AF8HostGetlocSource="
        << static_cast<int>(xaCd.lastHalGetlocPSource)
        << " xaCd36AF8HostGetlocSectorK="
        << (xaCd.lastHalGetlocPSectorIndexKnown ? 1 : 0)
        << " xaCd36AF8HostGetlocSector="
        << xaCd.lastHalGetlocPSectorIndex
        << " xaCd36AF8HostDataReadyInterruptKnown="
        << (xaCd.lastHalGetlocPDataReadyInterruptKnown ? 1 : 0)
        << " xaCd36AF8HostDataReadyInterrupt="
        << static_cast<int>(xaCd.lastHalGetlocPDataReadyInterrupt)
        << " xaCd36AF8HostGetlocCanFeedCdReg3Initial=0"
        << " xaCdPend359B8Writes="
        << xaCd.cdCallbackPending800359B8WriteCount
        << " xaCdPend359B8Gaps="
        << xaCd.cdCallbackPending800359B8GapCount
        << " xaCd359B8ObsAccepted="
        << xaCd.interruptSnapshot800359B8ObservationAcceptedCount
        << " xaCd359B8ObsRejected="
        << xaCd.interruptSnapshot800359B8ObservationRejectedCount
        << " xaCd359B8ObsCbStateK="
        << (xaCd.interruptSnapshot800359B8LastCallbackStateKnown ? 1 : 0)
        << " xaCd359B8ObsInitialRegsK="
        << (xaCd.interruptSnapshot800359B8LastInitialRegsKnown ? 1 : 0)
        << " xaCd359B8ObsTerminalRegsK="
        << (xaCd.interruptSnapshot800359B8LastTerminalRegsKnown ? 1 : 0)
        << " xaCd359B8ObsWatchdogK="
        << (xaCd.interruptSnapshot800359B8LastWatchdogKnown ? 1 : 0)
        << " xaCd359B8ObsBundleK="
        << (xaCd.interruptSnapshot800359B8LastBundleKnown ? 1 : 0);
    AppendStage1XaCd80036AF8EventSourceAudit(
        out,
        xaCd,
        cdMmioSnapshotAudit,
        rawEventWindow,
        rawEventInitialInterruptSourceResult,
        rawEventSourceResult);
    return out.str();
}

static std::string Stage1OverlayDescribeXaCdWindowReadAudit(
    PrGameContext& prCtx) {
    PrStage1RuntimeSlotsSnapshot slots{};
    const bool slotsValid = PrScn1::GetStage1RuntimeSlotsSnapshot(prCtx, slots) &&
                            slots.valid;
    const int query = slotsValid ? static_cast<int>(slots.queryFrame) : -1;
    const auto& provider = prCtx.stage1RuntimePsxMemoryProvider;
    const auto status57108SourceReadAudit =
        PrMain::AuditStage1RuntimePsxMemorySources(prCtx, 0x80057108u, 4u);
    const auto status57110SourceReadAudit =
        PrMain::AuditStage1RuntimePsxMemorySources(prCtx, 0x80057110u, 4u);
    const auto cdMmioSnapshotAudit =
        PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(provider);
    std::ostringstream out;
    out << "xacdWindowReadAudit scene=" << static_cast<int>(prCtx.currentScene)
        << " stageRunning=" << (prCtx.stageRunning ? 1 : 0)
        << " query=" << query
        << " debugChannel=stage1ovl"
        << " authority=provider-read-only"
        << " runtimeAuthority=0"
        << " publishAttempted=0"
        << " observationFeedAttempted=0"
        << " providerInstalled=" << (provider.installed ? 1 : 0)
        << " providerReadFnInstalled=" << (provider.read != nullptr ? 1 : 0)
        << " loaderHeapSourceInstalled="
        << (provider.loaderHeapSourceInstalled ? 1 : 0)
        << " knownStateRelayInstalled="
        << (provider.xaCdKnownStateRelayInstalled ? 1 : 0)
        << " cdMmioSourceInstalled="
        << (provider.cdMmioSourceInstalled ? 1 : 0)
        << " cdMmioReadFnInstalled="
        << (provider.cdMmioRead != nullptr ? 1 : 0)
        << " cdMmioUserDataInstalled="
        << (provider.cdMmioUserData != nullptr ? 1 : 0)
        << " cdMmioSnapshotSource="
        << (cdMmioSnapshotAudit.snapshotSource ? 1 : 0)
        << " cdMmioSnapshotIngressOnly=1"
        << " cdMmioProducerIngressInstalled="
        << (cdMmioSnapshotAudit.producerIngressInstalled ? 1 : 0)
        << " cdMmioProviderSubmitAvailable="
        << (cdMmioSnapshotAudit.providerSubmitAvailable ? 1 : 0)
        << " cdMmioProducerObservationCalls="
        << cdMmioSnapshotAudit.producerObservationCallCount
        << " cdMmioNaturalProducer=0"
        << " cdMmioProducerGap=missingWindowsCdMmioSampleProducer"
        << " cdMmioCdReg3InitialKnown="
        << (cdMmioSnapshotAudit.cdReg3InitialKnown ? 1 : 0)
        << " cdMmioCdReg3Initial=0x" << std::hex
        << static_cast<unsigned>(cdMmioSnapshotAudit.cdReg3Initial) << std::dec
        << " cdMmioCdReg0StatusKnown="
        << (cdMmioSnapshotAudit.cdReg0StatusKnown ? 1 : 0)
        << " cdMmioCdReg0Status=0x" << std::hex
        << static_cast<unsigned>(cdMmioSnapshotAudit.cdReg0Status) << std::dec
        << " cdMmioCanFeedCdReg3Initial="
        << (cdMmioSnapshotAudit.canFeedCdReg3Initial ? 1 : 0)
        << " cdMmioCanFeedCdReg0Status="
        << (cdMmioSnapshotAudit.canFeedCdReg0Status ? 1 : 0)
        << " cdMmioSamplePairAvailable="
        << (cdMmioSnapshotAudit.samplePairAvailable ? 1 : 0)
        << " cdMmioObsAccepted="
        << cdMmioSnapshotAudit.observationAcceptedCount
        << " cdMmioObsRejected="
        << cdMmioSnapshotAudit.observationRejectedCount
        << " cdMmioObsReject="
        << static_cast<int>(cdMmioSnapshotAudit.lastRejectReason)
        << " windowReadStatus57108LoaderHeapReadAttempted="
        << (status57108SourceReadAudit.loaderHeapReadAttempted ? 1 : 0)
        << " windowReadStatus57108LoaderHeapReadable="
        << (status57108SourceReadAudit.loaderHeapReadable ? 1 : 0)
        << " windowReadStatus57108KnownStateRelayReadAttempted="
        << (status57108SourceReadAudit.knownStateRelayReadAttempted ? 1 : 0)
        << " windowReadStatus57108KnownStateRelayReadable="
        << (status57108SourceReadAudit.knownStateRelayReadable ? 1 : 0)
        << " windowReadStatus57108CdMmioSnapshotReadAttempted="
        << (status57108SourceReadAudit.cdMmioSnapshotReadAttempted ? 1 : 0)
        << " windowReadStatus57108CdMmioSnapshotReadable="
        << (status57108SourceReadAudit.cdMmioSnapshotReadable ? 1 : 0)
        << " windowReadStatus57108ExactCdReadAttempted="
        << (status57108SourceReadAudit.exactCdReadAttempted ? 1 : 0)
        << " windowReadStatus57108ExactCdReadable="
        << (status57108SourceReadAudit.exactCdReadable ? 1 : 0)
        << " windowReadStatus57110LoaderHeapReadAttempted="
        << (status57110SourceReadAudit.loaderHeapReadAttempted ? 1 : 0)
        << " windowReadStatus57110LoaderHeapReadable="
        << (status57110SourceReadAudit.loaderHeapReadable ? 1 : 0)
        << " windowReadStatus57110KnownStateRelayReadAttempted="
        << (status57110SourceReadAudit.knownStateRelayReadAttempted ? 1 : 0)
        << " windowReadStatus57110KnownStateRelayReadable="
        << (status57110SourceReadAudit.knownStateRelayReadable ? 1 : 0)
        << " windowReadStatus57110CdMmioSnapshotReadAttempted="
        << (status57110SourceReadAudit.cdMmioSnapshotReadAttempted ? 1 : 0)
        << " windowReadStatus57110CdMmioSnapshotReadable="
        << (status57110SourceReadAudit.cdMmioSnapshotReadable ? 1 : 0)
        << " windowReadStatus57110ExactCdReadAttempted="
        << (status57110SourceReadAudit.exactCdReadAttempted ? 1 : 0)
        << " windowReadStatus57110ExactCdReadable="
        << (status57110SourceReadAudit.exactCdReadable ? 1 : 0)
        << " exactCdSourceInstalled="
        << (provider.exactCdSourceInstalled ? 1 : 0)
        << " exactCdReadFnInstalled="
        << (provider.exactCdRead != nullptr ? 1 : 0)
        << " exactCdUserDataInstalled="
        << (provider.exactCdUserData != nullptr ? 1 : 0)
        << " providerFrameKnown=" << (provider.frameKnown ? 1 : 0)
        << " providerFrame=" << provider.frame
        << " providerPcKnown=" << (provider.pcKnown ? 1 : 0)
        << " providerPc=0x" << std::hex << provider.pc << std::dec;
    const auto appendWindow = [&](const char* key,
                                  uint32_t psxAddress,
                                  uint32_t byteSize) {
        std::array<uint8_t, 0x40> bytes{};
        bool readable = false;
        if (provider.installed && provider.read != nullptr &&
            byteSize <= bytes.size()) {
            readable = provider.read(
                provider.userData,
                psxAddress,
                byteSize,
                bytes.data(),
                bytes.size());
        }
        uint32_t value = 0u;
        const uint32_t valueBytes = std::min<uint32_t>(byteSize, 4u);
        for (uint32_t i = 0; i < valueBytes; ++i) {
            value |= static_cast<uint32_t>(bytes[i]) << (8u * i);
        }
        out << " " << key << "Addr=0x" << std::hex << psxAddress << std::dec
            << " " << key << "Bytes=" << byteSize
            << " " << key << "ReadAttempted="
            << ((provider.installed && provider.read != nullptr &&
                 byteSize <= bytes.size())
                    ? 1
                    : 0)
            << " " << key << "Readable=" << (readable ? 1 : 0)
            << " " << key << "ValueKnown="
            << ((readable && byteSize <= 4u) ? 1 : 0);
        if (byteSize <= 4u) {
            out << " " << key << "Value=0x" << std::hex
                << (readable ? value : 0u) << std::dec;
        }
    };
    appendWindow("status57108", 0x80057108u, 4u);
    appendWindow("status57110", 0x80057110u, 4u);
    appendWindow("cdReg3Initial", 0x1F801803u, 1u);
    appendWindow("cdReg0Status", 0x1F801800u, 1u);
    appendWindow("cbState", 0x80055F78u, 0x34u);
    appendWindow("callbackPendingWord80055F7A", 0x80055F7Au, 2u);
    appendWindow("interruptRegs", 0x1F801070u, 8u);
    appendWindow("watchdog", 0x80057010u, 4u);
    appendWindow("timeoutState80088310", 0x80088310u, 4u);
    appendWindow("timeoutSpin80088314", 0x80088314u, 4u);
    return out.str();
}

static std::string Stage1OverlayDescribeScoreDisplayHandoffHistory(
    PrGameContext& prCtx,
    size_t maxCount) {
    (void)maxCount;
    PrStage1RuntimeSlotsSnapshot slots{};
    const bool slotsValid = PrScn1::GetStage1RuntimeSlotsSnapshot(prCtx, slots) &&
                            slots.valid;
    const int query = slotsValid ? (int)slots.queryFrame : -1;
    const PrScn1::Stage1NumericRuntimeState& numeric =
        PrScn1::s_stage1NumericRuntime;
    const auto& rightRank = numeric.rightRankState;
    const auto& accepted = numeric.acceptedProducer;
    const auto& acceptedProbe = numeric.acceptedProducerBoundaryProbe;
    const auto& acceptedCarrier = numeric.acceptedProducerCarrier;
    const auto& lastWrite = numeric.acceptedProducerLastRecordedPageWrite;
    const auto& scorer = numeric.scorerPort;
    const auto& phase1Owner = numeric.rightRankPhase1Owner;
    const auto& observer24 = numeric.rightRank24F8CObserver;
    const auto& tieBreaker = numeric.rightRankTieBreakerObserver;
    const auto& helperShadow = numeric.rightRankHelperShadow;
    const auto& ownerObserver = numeric.bucket30OwnerObserver;
    const auto& descriptorCadence = numeric.descriptorCadence;
        const auto& timecode = numeric.runnerTimecode801C7560;
        const auto& tailHost = numeric.runnerTailHost7A60;
        auto& xaCd = prCtx.stage1XaCdDirect;
    const auto xaCdClockProbe =
        PrStage1XaCdDirectProbeStreamClockProducer800493F4(xaCd);
    const auto status57108RuntimeWindow =
        Stage1ReadStatus57108RuntimeSourceWindow(prCtx);
    const auto status57108RuntimeSource =
        PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(
            status57108RuntimeWindow);
    const auto status57108RuntimeSourceResult =
        PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(
            status57108RuntimeSource,
            xaCd);
    const auto rawEventRuntimeWindow =
        PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(
            prCtx.stage1RuntimePsxMemoryProvider);
    const auto rawEventInitialInterruptSource =
        PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
            rawEventRuntimeWindow);
    const auto rawEventInitialInterruptSourceResult =
        PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
            rawEventInitialInterruptSource,
            xaCd);
    const auto rawEventRuntimeSource =
        PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(xaCd);
    const auto rawEventRuntimeSourceResult =
        PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(
            rawEventRuntimeSource,
            xaCd,
            &prCtx.stage1RuntimePsxMemoryProvider);
    const auto xacd359b8RuntimeSourceResult =
        Stage1PublishInterruptSnapshotRuntimeSource(
            prCtx,
            xaCd,
            nullptr);
    const auto xacd359b8TypedSource =
        PrStage1XaCdDirectAuditInterruptSnapshotTypedSource800359B8(xaCd);
    const bool xaHostClockKnown =
        prCtx.xa1Player != nullptr && prCtx.xa1Player->IsPlaying();
    const bool xaPlayerSelectedFilterKnown =
        prCtx.xa1Player != nullptr && prCtx.xa1Player->GetSelectedFilterKnown();
    const bool xaPlayerSelectedCodingKnown =
        prCtx.xa1Player != nullptr && prCtx.xa1Player->GetSelectedCodingKnown();
    const uint32_t xaPlayerCurrentSector =
        prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetCurrentSectorIndex() : 0u;
    const uint32_t xaPlayerAcceptedPushCount =
        prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetAcceptedRawPushCount() : 0u;
    const size_t xaPlayerAcceptedQueueSize =
        prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetAcceptedRawSectorQueueSize() : 0u;
    const bool xaPlayerLastAcceptedSectorKnown =
        prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastAcceptedSectorKnown();
    const int xaHostClockRel75 =
        xaHostClockKnown ? prCtx.xa1Player->GetCdClockSectorIndex() : 0;
    const int xaHostAudibleRel75 =
        xaHostClockKnown ? prCtx.xa1Player->GetEstimatedAudibleSectorIndex() : 0;
    const int xaHostClockAbsLba =
        xaHostClockKnown
            ? (static_cast<int>(xaCd.dword_800493FC) + xaHostClockRel75)
            : 0;
    const int xaHostAudibleAbsLba =
        xaHostClockKnown
            ? (static_cast<int>(xaCd.dword_800493FC) + xaHostAudibleRel75)
            : 0;
    const uint16_t rightRankDescriptorFlagWord =
        PrScn1::ResolveStage1RightRankDescriptorFlagWord(numeric);
    const int currentPageOrdinal1Based =
        (descriptorCadence.pageOrdinal56Available &&
         descriptorCadence.pageOrdinal56 > 0u)
            ? static_cast<int>(
                  std::min(descriptorCadence.pageOrdinal56,
                           static_cast<size_t>(0xFFu)))
            : (numeric.pageRecordMirror.initialized
                   ? numeric.pageRecordMirror.currentPageOrdinal + 1
                   : 0);
    const auto& desc44Row = descriptorCadence.lookaheadDescriptor44Row;
    const bool desc44Available =
        descriptorCadence.lookaheadDescriptor44Available && desc44Row.available;
    const uint16_t desc44Substate50 = numeric.descriptorSubstate50;
    const bool desc44UseSubstateBranch = desc44Substate50 != 0u;
    const uint8_t desc44BranchSelectorByte0 =
        desc44Available
            ? (desc44UseSubstateBranch ? desc44Row.substate1SelectorByte0
                                       : desc44Row.defaultSelectorByte0)
            : 0u;
    const uint8_t desc44BranchSelectorByte1 =
        desc44Available
            ? (desc44UseSubstateBranch ? desc44Row.substate1SelectorByte1
                                       : desc44Row.defaultSelectorByte1)
            : 0u;
    const StageSelectStatusDebugValue stage1Status =
        GetStageSelectStatusForDebug(prCtx, 1);
    const bool stage1StatusKnown = stage1Status.known;
    const bool firstClearStatus =
        stage1StatusKnown && stage1Status.value >= 2;
    const int acceptedFormulaCount =
        static_cast<int>(accepted.acceptedContributionCount) +
        (numeric.additiveLane.value != 0 ? 1 : 0);
    const char* acceptedSplitName =
        acceptedProbe.split == 1u ? "recorded" :
        (acceptedProbe.split == 2u ? "penalty" : "none");
    PrStage1FormalLifecycleSnapshot terminalLifecycle{};
    const bool terminalValid =
        PrScn1::CopyStage1TerminalFormalLifecycleSnapshot(terminalLifecycle);
    PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC hostBlock{};
    const bool hostBlockKnown =
        PrScn1::CopyStage1HostBlockSnapshot801C81EC(hostBlock);
    const PrStage1SavePayloadBankRuntimeSnapshot savePayloadBankRuntime =
        PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    const auto queryDeltaOrMissing = [](bool known, int lhs, int rhs) -> int {
        return known ? lhs - rhs : -1;
    };
    const bool terminalRunnerClearKnown =
        terminalValid &&
        terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearKnown;
    const int terminalRunnerClearQuery =
        terminalRunnerClearKnown
            ? (int)terminalLifecycle.runnerExitEventStreamFlagLastRunnerClearQueryFrame
            : -1;

    std::ostringstream out;
    out << "history count=" << (slotsValid ? 1 : 0);
    if (slotsValid) {
        out << "\nsample index=0 hostFrame=" << g_frameNum
            << " scene=1 stageRunning=1 query=" << query
            << " numericAvailable=" << (numeric.active ? 1 : 0)
            << " numericQuery=" << (numeric.active ? (int)numeric.queryFrame : query)
            << " stage1StatusKnown=" << (stage1StatusKnown ? 1 : 0)
            << " stage1Status=" << stage1Status.value
            << " goodToCoolGate=" << (firstClearStatus ? 1 : 0)
            << " proxyGate=" << (firstClearStatus ? 1 : 0)
            << " gateSource=latch proxyDirect=0"
            << " score=" << numeric.scoreDisplayValue
            << " source=runtimeSlots aligned=1"
            << " formulaBacked="
            << (numeric.scoreDisplayFormulaBacked ? 1 : 0)
            << " tick96=" << numeric.runnerTimecode801C7560.state.tick801C364C
            << " timecodeKnown=" << (timecode.known ? 1 : 0)
            << " tcBaseK=" << (timecode.xaSectorBaselineKnown ? 1 : 0)
            << " tcBase75=" << timecode.xaSectorBaseline75
            << " tcXaK=" << (timecode.xaReadValueA7A4Known ? 1 : 0)
            << " tcTimeBaseGap="
            << (timecode.gapMissingSceneEntryField196TimeBase ? 1 : 0)
            << " tcClockGap="
            << (timecode.gapMissingStreamClock800493F4 ? 1 : 0)
            << " tcPollA3C8="
            << (timecode.clockPoll8001A3C8Called ? 1 : 0)
            << " tcPollRet=" << timecode.clockPoll8001A3C8Return
            << " tcPollAccept="
            << (timecode.clockPoll8001A3C8AcceptedByte ? 1 : 0)
            << " tcPollGapD0="
            << (timecode.clockPoll8001A3C8Gap364D0 ? 1 : 0)
            << " tcPollGapA4="
            << (timecode.clockPoll8001A3C8Gap363A4 ? 1 : 0)
            << " tcCmdA280=" << (timecode.command8001A280Called ? 1 : 0)
            << " tcCmdA280Issued="
            << (timecode.command8001A280Issued ? 1 : 0)
            << " tcCmdA280Skip="
            << (timecode.command8001A280SkippedNonZeroWorkBase ? 1 : 0)
            << " tcCmdA280Gap="
            << (timecode.command8001A280Gap49428 ? 1 : 0)
            << " tcCmdA280Base=" << timecode.command8001A280WorkBase
            << " xaCdReadS27Serial=" << xaCd.readS27Serial
            << " xaCdSetloc2Serial=" << xaCd.setloc2Serial
            << " xaCdD0FbK="
            << (xaCd.cdLowerFeedback80036AF8Known ? 1 : 0)
            << " xaCdD0K=" << (xaCd.cdLowerFeedback80036AF8.known ? 1 : 0)
            << " xaCdD0SyncK="
            << (xaCd.cdLowerFeedback80036AF8.syncResultKnown ? 1 : 0)
            << " xaCdD0Ret=" << xaCd.cdLowerFeedback80036AF8.syncResult
            << " xaCdD0BytesK="
            << (xaCd.cdLowerFeedback80036AF8.responseBytesKnown ? 1 : 0)
            << " xaCdD0BytesN="
            << xaCd.cdLowerFeedback80036AF8.responseByteCount
            << " xaCdD0B0="
            << static_cast<int>(xaCd.cdLowerFeedback80036AF8.responseBytes[0])
            << " xaCdD0B1="
            << static_cast<int>(xaCd.cdLowerFeedback80036AF8.responseBytes[1])
            << " xaCdD0B2="
            << static_cast<int>(xaCd.cdLowerFeedback80036AF8.responseBytes[2])
            << " xaCdExpK=" << (xaCd.cdSyncExplicitStatusKnown ? 1 : 0)
            << " xaCdExpSt=" << static_cast<int>(xaCd.cdSyncExplicitStatus)
            << " xaCdExpBytesK="
            << (xaCd.cdSyncExplicitResponseBytesKnown ? 1 : 0)
            << " xaCdExpB0="
            << static_cast<int>(xaCd.cdSyncExplicitResponseBytes[0])
            << " xaCdExpB1="
            << static_cast<int>(xaCd.cdSyncExplicitResponseBytes[1])
            << " xaCdExpB2="
            << static_cast<int>(xaCd.cdSyncExplicitResponseBytes[2])
            << " xaCdRspF8K=" << (xaCd.response_800882F8Known ? 1 : 0)
            << " xaCdRspF8B0="
            << static_cast<int>(xaCd.response_800882F8[0])
            << " xaCdRspF8B1="
            << static_cast<int>(xaCd.response_800882F8[1])
            << " xaCdRspF8B2="
            << static_cast<int>(xaCd.response_800882F8[2])
            << " xaCd5719K=" << (xaCd.byte_80057119Known ? 1 : 0)
            << " xaCd5719=" << static_cast<int>(xaCd.byte_80057119)
            << " xaCd57108K=" << (xaCd.dword_80057108Known ? 1 : 0)
            << " xaCd57108=" << xaCd.dword_80057108
            << " xaCd57108ObsAccepted="
            << xaCd.statusFlags80057108ObservationAcceptedCount
            << " xaCd57108ObsRejected="
            << xaCd.statusFlags80057108ObservationRejectedCount
            << " xaCd57108ObsReject="
            << static_cast<int>(xaCd.statusFlags80057108LastReject)
            << " xaCd57108RuntimeSource="
            << (status57108RuntimeSourceResult.sourceAvailable ? 1 : 0)
            << " xaCd57108RuntimeSourceAdapter=1"
            << " xaCd57108RuntimeSourceAvailable="
            << (status57108RuntimeSourceResult.sourceAvailable ? 1 : 0)
            << " xaCd57108RuntimeSourceValueKnown="
            << (status57108RuntimeSourceResult.valueKnown ? 1 : 0)
            << " xaCd57108RuntimeSourcePublishAttempted="
            << (status57108RuntimeSourceResult.publishAttempted ? 1 : 0)
            << " xaCd57108RuntimeSourceBlocker="
            << static_cast<int>(status57108RuntimeSourceResult.blocker)
            << " xaCd57108RuntimeSourceAccepted="
            << (status57108RuntimeSourceResult.observation.accepted ? 1 : 0)
            << " xaCd57108RuntimeSourceReject="
            << static_cast<int>(
                   status57108RuntimeSourceResult.observation.rejectReason)
            << " xaCd57108Addr=0x" << std::hex << 0x80057108u << std::dec
            << " xaCd57108Bytes=4"
            << " xaCd57108Readable="
            << (status57108RuntimeWindow.windowReadable ? 1 : 0)
            << " xaCd57108CanFeedObservation="
            << ((status57108RuntimeSourceResult.sourceAvailable &&
                 status57108RuntimeSourceResult.valueKnown)
                    ? 1
                    : 0)
            << " xaCd57108ObservationFeedAttempted=0";
        AppendStage1XaCd359B8RuntimeSourceState(
            out,
            xacd359b8RuntimeSourceResult,
            xacd359b8TypedSource);
        const auto cdMmioSnapshotAudit =
            PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(
                prCtx.stage1RuntimePsxMemoryProvider);
        AppendStage1XaCd80036AF8EventSourceAudit(
            out,
            xaCd,
            cdMmioSnapshotAudit,
            rawEventRuntimeWindow,
            rawEventInitialInterruptSourceResult,
            rawEventRuntimeSourceResult);
        out << " xaCdD4K=" << (xaCd.byte_800573D4Known ? 1 : 0)
            << " xaCdD4=" << static_cast<int>(xaCd.byte_800573D4)
            << " xaCdStatusObsK="
            << (xaCd.statusRead80036384Known ? 1 : 0)
            << " xaCdStatusObs=" << xaCd.statusRead80036384
            << " xaCdStatusObsAccepted="
            << xaCd.statusByte800573D4ObservationAcceptedCount
            << " xaCdStatusObsRejected="
            << xaCd.statusByte800573D4ObservationRejectedCount
            << " xaCdStatusObsReject="
            << static_cast<int>(xaCd.statusByte800573D4LastReject)
            << " xaCdF4K=" << (xaCd.byte_800493F4Known ? 1 : 0)
            << " xaCdF4M=" << static_cast<int>(xaCd.byte_800493F4.minute)
            << " xaCdF4S=" << static_cast<int>(xaCd.byte_800493F4.second)
            << " xaCdF4F=" << static_cast<int>(xaCd.byte_800493F4.frame)
            << " xaCdF4Producer=0x" << std::hex
            << xaCd.byte800493F4ProducerFunction << std::dec
            << " xaCdClockK="
            << (xaCdClockProbe.carrier.clockKnown ? 1 : 0)
            << " xaCdClockLba=" << xaCdClockProbe.carrier.clockLba
            << " xaCdClockGap="
            << (xaCdClockProbe.gapMissingStreamClock800493F4Producer ? 1 : 0)
            << " xaHalGetlocPCount=" << xaCd.halGetlocPFactsApplyCount
            << " xaCurrentPhysicalGetlocPProbeDisabled="
            << (PrScn1::IsStage1XaCurrentPhysicalGetlocPProbeDisabled() ? 1 : 0)
            << " xaHalGetlocPSource="
            << static_cast<int>(xaCd.lastHalGetlocPSource)
            << " xaHalGetlocPSectorK="
            << (xaCd.lastHalGetlocPSectorIndexKnown ? 1 : 0)
            << " xaHalGetlocPSector=" << xaCd.lastHalGetlocPSectorIndex
            << " xaHostClockK=" << (xaHostClockKnown ? 1 : 0)
            << " xaPlayerSelK=" << (xaPlayerSelectedFilterKnown ? 1 : 0)
            << " xaPlayerSelFile="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetSelectedFile()) : 0)
            << " xaPlayerSelCh="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetSelectedChannel()) : 0)
            << " xaPlayerSelCodingK=" << (xaPlayerSelectedCodingKnown ? 1 : 0)
            << " xaPlayerSelCoding="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetSelectedCoding()) : 0)
            << " xaPlayerSetFilterCount="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetSetFilterChangeCount() : 0u)
            << " xaPlayerLastSetFilterK="
            << (prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastSetFilterKnown() ? 1 : 0)
            << " xaPlayerLastSetFilterSector="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastSetFilterSectorIndex() : 0u)
            << " xaPlayerLastSetFilterFile="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastSetFilterFile()) : 0)
            << " xaPlayerLastSetFilterCh="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastSetFilterChannel()) : 0)
            << " xaTailReqK=" << (tailHost.known ? 1 : 0)
            << " xaTailReq=" << (tailHost.requestXaSetFilter13 ? 1 : 0)
            << " xaTailReqArg=" << static_cast<int>(tailHost.xaSetFilter13Arg)
            << " xaTailReqCount=" << tailHost.xaSetFilter13RequestCount
            << " xaTailLastReqK="
            << (tailHost.xaSetFilter13LastRequestKnown ? 1 : 0)
            << " xaTailLastReqQ=" << tailHost.xaSetFilter13LastRequestQueryFrame
            << " xaTailLastReqTick96="
            << tailHost.xaSetFilter13LastRequestTick96
            << " xaTailLastReqFlag0200="
            << (tailHost.xaSetFilter13LastRequestFlag0200Pulse ? 1 : 0)
            << " xaTailLastReqRow="
            << static_cast<int>(tailHost.xaSetFilter13LastRequestRow)
            << " xaTailLastReqArg="
            << static_cast<int>(tailHost.xaSetFilter13LastRequestArg)
            << " xaPlayerCurSector=" << xaPlayerCurrentSector
            << " xaPlayerRawReadCount="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRawReadCount() : 0u)
            << " xaPlayerLastRawReadK="
            << (prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastRawReadSectorKnown() ? 1 : 0)
            << " xaPlayerLastRawReadSector="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastRawReadSectorIndex() : 0u)
            << " xaPlayerAccPushCount=" << xaPlayerAcceptedPushCount
            << " xaPlayerAccPopCount="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetAcceptedRawPopCount() : 0u)
            << " xaPlayerAccQueue=" << xaPlayerAcceptedQueueSize
            << " xaPlayerLastPollK="
            << (prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastPolledSectorKnown() ? 1 : 0)
            << " xaPlayerLastPollSector="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastPolledSectorIndex() : 0u)
            << " xaPlayerLastAccK=" << (xaPlayerLastAcceptedSectorKnown ? 1 : 0)
            << " xaPlayerLastAccSector="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastAcceptedSectorIndex() : 0u)
            << " xaPlayerLastAccFile="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastAcceptedFile()) : 0)
            << " xaPlayerLastAccCh="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastAcceptedChannel()) : 0)
            << " xaPlayerLastAccCoding="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastAcceptedCoding()) : 0)
            << " xaPlayerLastRejectK="
            << (prCtx.xa1Player != nullptr && prCtx.xa1Player->GetLastFilterRejectKnown() ? 1 : 0)
            << " xaPlayerLastRejectSector="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetLastFilterRejectSectorIndex() : 0u)
            << " xaPlayerLastRejectFile="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastFilterRejectFile()) : 0)
            << " xaPlayerLastRejectCh="
            << (prCtx.xa1Player != nullptr ? static_cast<int>(prCtx.xa1Player->GetLastFilterRejectChannel()) : 0)
            << " xaPlayerRecentRead0="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentRawReadSector(0) : 0u)
            << " xaPlayerRecentRead1="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentRawReadSector(1) : 0u)
            << " xaPlayerRecentRead2="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentRawReadSector(2) : 0u)
            << " xaPlayerRecentRead3="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentRawReadSector(3) : 0u)
            << " xaPlayerRecentAcc0="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentAcceptedSector(0) : 0u)
            << " xaPlayerRecentAcc1="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentAcceptedSector(1) : 0u)
            << " xaPlayerRecentAcc2="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentAcceptedSector(2) : 0u)
            << " xaPlayerRecentAcc3="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentAcceptedSector(3) : 0u)
            << " xaPlayerRecentPoll0="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentPolledSector(0) : 0u)
            << " xaPlayerRecentPoll1="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentPolledSector(1) : 0u)
            << " xaPlayerRecentPoll2="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentPolledSector(2) : 0u)
            << " xaPlayerRecentPoll3="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentPolledSector(3) : 0u)
            << " xaPlayerRecentReject0="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentFilterRejectSector(0) : 0u)
            << " xaPlayerRecentReject1="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentFilterRejectSector(1) : 0u)
            << " xaPlayerRecentReject2="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentFilterRejectSector(2) : 0u)
            << " xaPlayerRecentReject3="
            << (prCtx.xa1Player != nullptr ? prCtx.xa1Player->GetRecentFilterRejectSector(3) : 0u)
            << " xaHostClockRel75=" << xaHostClockRel75
            << " xaHostClockAbsLba=" << xaHostClockAbsLba
            << " xaHostAudibleRel75=" << xaHostAudibleRel75
            << " xaHostAudibleAbsLba=" << xaHostAudibleAbsLba
            << " xaCdA210Dispatch="
            << xaCd.cdSyncCallback8001A210DispatchCount
            << " xaCdSync37070Gap=" << xaCd.cdSyncCallback80037070GapCount
            << " xaCdAsync37070Gap=" << xaCd.cdAsyncCallback80037070GapCount
            << " xaCdEvtSerial=" << xaCd.cdLowerEvent80036AF8Serial
            << " xaCdEvtDispatched="
            << xaCd.cdLowerEvent80036AF8DispatchedSerial
            << " xaCdRet36AF8K="
            << (xaCd.cdLowerEventPsxReturn80036AF8Known ? 1 : 0)
            << " xaCdRet36AF8=" << xaCd.cdLowerEventPsxReturn80036AF8
            << " xaCdCb570F8K=" << (xaCd.dword_800570F8Known ? 1 : 0)
            << " xaCdCb570F8=0x" << std::hex << xaCd.dword_800570F8
            << " xaCdCb570FCK=" << std::dec
            << (xaCd.dword_800570FCKnown ? 1 : 0)
            << " xaCdCb570FC=0x" << std::hex << xaCd.dword_800570FC
            << std::dec
            << " xaCdCb570FCWrites="
            << xaCd.readyCallbackRegisterWriteCount
            << " xaCdCb570FCClears="
            << xaCd.readyCallbackRegisterClearCount
            << " xaCdCb570FCInstall39240="
            << xaCd.readyCallbackRegisterInstall39240Count
            << " xaCdCb570FCSource=0x" << std::hex
            << xaCd.readyCallbackRegisterSourceFunction
            << std::dec
            << " xaCdPend359B8Writes="
            << xaCd.cdCallbackPending800359B8WriteCount
            << " xaCdPend359B8Gaps="
            << xaCd.cdCallbackPending800359B8GapCount
            << " xaCd359B8ObsAccepted="
            << xaCd.interruptSnapshot800359B8ObservationAcceptedCount
            << " xaCd359B8ObsRejected="
            << xaCd.interruptSnapshot800359B8ObservationRejectedCount
            << " xaCd359B8ObsReject="
            << static_cast<int>(
                   xaCd.interruptSnapshot800359B8ObservationLastReject)
            << " xaCd359B8ObsCbStateK="
            << (xaCd.interruptSnapshot800359B8LastCallbackStateKnown ? 1 : 0)
            << " xaCd359B8ObsInitialRegsK="
            << (xaCd.interruptSnapshot800359B8LastInitialRegsKnown ? 1 : 0)
            << " xaCd359B8ObsTerminalRegsK="
            << (xaCd.interruptSnapshot800359B8LastTerminalRegsKnown ? 1 : 0)
            << " xaCd359B8ObsWatchdogK="
            << (xaCd.interruptSnapshot800359B8LastWatchdogKnown ? 1 : 0)
            << " xaCd359B8ObsBundleK="
            << (xaCd.interruptSnapshot800359B8LastBundleKnown ? 1 : 0)
            << " xaCdPend359B8IntStatusK="
            << (xaCd.cdCallbackPending800359B8InterruptStatusKnown ? 1 : 0)
            << " xaCdPend359B8IntStatus="
            << xaCd.cdCallbackPending800359B8InterruptStatus
            << " xaCdPend359B8IntMaskK="
            << (xaCd.cdCallbackPending800359B8InterruptMaskKnown ? 1 : 0)
            << " xaCdPend359B8IntMask="
            << xaCd.cdCallbackPending800359B8InterruptMask
            << " xaCdPend359B8Word55F78K="
            << (xaCd.word_80055F78Known ? 1 : 0)
            << " xaCdPend359B8Word55F78="
            << xaCd.word_80055F78
            << " xaCdPend359B8Word55F7AK="
            << (xaCd.word_80055F7AKnown ? 1 : 0)
            << " xaCdPend359B8Word55F7A="
            << xaCd.word_80055F7A
            << " xaCdPend359B8Word55FA8K="
            << (xaCd.word_80055FA8Known ? 1 : 0)
            << " xaCdPend359B8Word55FA8="
            << xaCd.word_80055FA8
            << " xaCdPend359B8Watchdog57010K="
            << (xaCd.dword_80057010Known ? 1 : 0)
            << " xaCdPend359B8Watchdog57010="
            << xaCd.dword_80057010
            << " xaCdPend359B8Acks="
            << xaCd.cdCallbackPending800359B8AckCount
            << " xaCdPend359B8Dispatch="
            << xaCd.cdCallbackPending800359B8CallbackDispatchCount
            << " xaCdLowerSnapApply="
            << xaCd.lowerCdSnapshotApplyCount
            << " xaCdLowerSnapPending="
            << xaCd.lowerCdSnapshotPendingProducerCount
            << " xaCdLowerSnapEvent="
            << xaCd.lowerCdSnapshotCallbackEventCount
            << " xaCdLowerSnapSync="
            << xaCd.lowerCdSnapshotSyncFeedbackCount
            << " xaCdLowerSnapSeam="
            << xaCd.lowerCdSnapshotSeamResultCount
            << " xaCdGetlocBridge="
            << xaCd.halGetlocLowerBridgeCount
            << " xaCdGetlocBridgeCore="
            << xaCd.halGetlocLowerBridgeCdSyncCoreCount
            << " xaCdGetlocBridgePending="
            << xaCd.halGetlocLowerBridgePendingProducerCount
            << " xaCdGetlocBridgeEvent="
            << xaCd.halGetlocLowerBridgeCallbackEventCount;
        AppendStage1Bootstrap15590CdLowerObserverSurface(
            out,
            hostBlockKnown,
            hostBlock,
            xaCd.readS27Serial);
        out << " tcPrevXa=" << timecode.lastResult.previousXaReadValue801D303C
            << " tcXa=" << timecode.lastResult.xaSectorReadValueA7A4
            << " tcFallback="
            << (timecode.lastResult.usedFallbackField352Increment ? 1 : 0)
            << " tc348=" << timecode.lastResult.sceneEntryField348
            << " tc352="
            << timecode.lastResult.sceneEntryField352FallbackTickAdvance
            << " tc356=" << timecode.lastResult.sceneEntryField356TickOffset
            << " tcProduct=" << timecode.lastResult.xaProduct348ByReadValue32
            << " tcRound=" << timecode.lastResult.xaRoundedTickBeforeOffset
            << " tcCbRet=" << timecode.lastResult.callbackReturnValue
            << " intra384=" << numeric.acceptedProducerCarrier.phase384
            << " cadenceThreshold=0 cadenceMode=runtime"
            << " topLessonPageOrdinalAvailable="
            << (descriptorCadence.pageOrdinal56Available ? 1 : 0)
            << " topLessonPageOrdinal="
            << descriptorCadence.pageOrdinal56
            << " page56K="
            << (descriptorCadence.pageOrdinal56Available ? 1 : 0)
            << " page56=" << descriptorCadence.pageOrdinal56
            << " cadenceCursorAvailable="
            << (descriptorCadence.cadenceCursorAvailable ? 1 : 0)
            << " cadK="
            << (descriptorCadence.cadenceCursorAvailable ? 1 : 0)
            << " currentDescriptor40Available="
            << (descriptorCadence.currentDescriptor40Available ? 1 : 0)
            << " currentDescriptor40Index="
            << descriptorCadence.currentDescriptor40RowIndex
            << " desc40K="
            << (descriptorCadence.currentDescriptor40Available ? 1 : 0)
            << " desc40Idx="
            << descriptorCadence.currentDescriptor40RowIndex
            << " rightRankDescriptorFlagWord08Known="
            << (numeric.rightRankDescriptorFlagWord08Known ? 1 : 0)
            << " rightRankDescriptorFlagWord08="
            << numeric.rightRankDescriptorFlagWord08
            << " rr08K="
            << (numeric.rightRankDescriptorFlagWord08Known ? 1 : 0)
            << " rr08=" << numeric.rightRankDescriptorFlagWord08
            << " currentPageOrdinal1Based=" << currentPageOrdinal1Based
            << " curPage1=" << currentPageOrdinal1Based
            << " rankRow=" << (int)rightRank.rightRankActiveRow
            << " rankBlinkTarget="
            << (int)rightRank.rightRankBlinkTargetRow
            << " rankBlink="
            << (rightRank.rightRankBlinkEnabled ? 1 : 0)
            << " resolutionKnown="
            << (numeric.bucket30ResolutionKnown ? 1 : 0)
            << " resolutionV22=" << (int)numeric.bucket30ResolutionV22
            << " rankWritebackCommitted="
            << (numeric.bucket30RightRankWritebackCommitted ? 1 : 0)
            << " producerFormalWritebackKnown=" << (numeric.active ? 1 : 0)
            << " producerFormalWriteback24="
            << observer24.formalWritebackValue24
            << " formal24K=" << (numeric.active ? 1 : 0)
            << " formal24=" << observer24.formalWritebackValue24
            << " cachedPhase1Classifier36="
            << (int)phase1Owner.cachedPhase1Classifier36
            << " phase1LatchArmed38="
            << (phase1Owner.phase1LatchArmed38 ? 1 : 0)
            << " bucket30OwnerBusyGateActive="
            << (ownerObserver.busyGateActive ? 1 : 0)
            << " bucket30OwnerProcessDescriptorFlagWord="
            << ownerObserver.processDescriptorFlagWord
            << " bucket30OwnerScorerCommitWindowOpen="
            << (ownerObserver.scorerCommitWindowOpen ? 1 : 0)
            << " bucket30OwnerNoInputCounterAdvanceRan="
            << (ownerObserver.noInputCounterAdvanceRan ? 1 : 0)
            << " bucket30OwnerNoInputCounterAcceptedCountInput="
            << ownerObserver.noInputCounterAcceptedCountInput
            << " bucket30OwnerNoInputCounterInput="
            << ownerObserver.noInputCounterInput
            << " bucket30OwnerNoInputCounterOutput="
            << ownerObserver.noInputCounterOutput
            << " bucket30OwnerNoInputCounterIncremented="
            << (ownerObserver.noInputCounterIncremented ? 1 : 0)
            << " bucket30OwnerKernelOpen="
            << (ownerObserver.kernelOpen ? 1 : 0)
            << " bucket30OwnerKernelEntered="
            << (ownerObserver.kernelEntered ? 1 : 0)
            << " bucket30OwnerDescriptorFlagWord="
            << ownerObserver.descriptorFlagWord
            << " bucket30OwnerActiveRow="
            << (int)ownerObserver.activeRow
            << " bucket30OwnerPrePhase1Classifier36="
            << (int)ownerObserver.prePhase1Classifier36
            << " bucket30OwnerPrePhase1LatchArmed38="
            << (ownerObserver.prePhase1LatchArmed38 ? 1 : 0)
            << " bucket30OwnerPhase1AdvanceCalled="
            << (ownerObserver.phase1AdvanceCalled ? 1 : 0)
            << " bucket30OwnerPhase1AdvanceSampledClassifier="
            << (int)ownerObserver.phase1AdvanceSampledClassifier
            << " bucket30OwnerPhase1AdvanceFirstBeat="
            << (ownerObserver.phase1AdvanceFirstBeat ? 1 : 0)
            << " bucket30GameplayCurrentBranchKnown="
            << (ownerObserver.gameplayCurrentBranchKnown ? 1 : 0)
            << " gBranchK="
            << (ownerObserver.gameplayCurrentBranchKnown ? 1 : 0)
            << " bucket30GameplayCurrentBranchSpecial="
            << (ownerObserver.gameplayCurrentBranchSpecial ? 1 : 0)
            << " gSpec="
            << (ownerObserver.gameplayCurrentBranchSpecial ? 1 : 0)
            << " bucket30GameplayAcceptedCountKnown="
            << (ownerObserver.gameplayAcceptedCountKnown ? 1 : 0)
            << " gAccK="
            << (ownerObserver.gameplayAcceptedCountKnown ? 1 : 0)
            << " bucket30GameplayAcceptedCount="
            << ownerObserver.gameplayAcceptedCount
            << " gAcc=" << ownerObserver.gameplayAcceptedCount
            << " bucket30GameplayLookbackPageCountKnown="
            << (ownerObserver.gameplayLookbackPageCountKnown ? 1 : 0)
            << " gLookK="
            << (ownerObserver.gameplayLookbackPageCountKnown ? 1 : 0)
            << " bucket30GameplayLookbackPageCount="
            << ownerObserver.gameplayLookbackPageCount
            << " gLook=" << ownerObserver.gameplayLookbackPageCount
            << " bucket30GameplayOverflowActiveKnown="
            << (ownerObserver.gameplayOverflowActiveKnown ? 1 : 0)
            << " gOvK="
            << (ownerObserver.gameplayOverflowActiveKnown ? 1 : 0)
            << " bucket30GameplayOverflowActive="
            << (ownerObserver.gameplayOverflowActive ? 1 : 0)
            << " gOv=" << (ownerObserver.gameplayOverflowActive ? 1 : 0)
            << " bucket30GameplayBranchCountKnown="
            << (ownerObserver.gameplayBranchCountKnown ? 1 : 0)
            << " gCountK="
            << (ownerObserver.gameplayBranchCountKnown ? 1 : 0)
            << " bucket30GameplayBranchCount="
            << ownerObserver.gameplayBranchCount
            << " gCount=" << ownerObserver.gameplayBranchCount
            << " bucket30GameplayDescriptorSubdeltaKnown="
            << (ownerObserver.gameplayDescriptorSubdeltaKnown ? 1 : 0)
            << " gSubK=" << (ownerObserver.gameplayDescriptorSubdeltaKnown ? 1 : 0)
            << " bucket30GameplayDescriptorSubdelta="
            << ownerObserver.gameplayDescriptorSubdelta
            << " gSub=" << ownerObserver.gameplayDescriptorSubdelta
            << " bucket30GameplayReaderPageOrdinalKnown="
            << (ownerObserver.gameplayReaderPageOrdinalKnown ? 1 : 0)
            << " gPageK=" << (ownerObserver.gameplayReaderPageOrdinalKnown ? 1 : 0)
            << " bucket30GameplayReaderPageOrdinal="
            << ownerObserver.gameplayReaderPageOrdinal
            << " gPage=" << ownerObserver.gameplayReaderPageOrdinal
            << " bucket30GameplayRequiredMaskKnown="
            << (ownerObserver.gameplayRequiredMaskKnown ? 1 : 0)
            << " gReqK=" << (ownerObserver.gameplayRequiredMaskKnown ? 1 : 0)
            << " bucket30GameplayRequiredMask="
            << ownerObserver.gameplayRequiredMask
            << " gReq=" << ownerObserver.gameplayRequiredMask
            << " bucket30GameplayUnionMaskKnown="
            << (ownerObserver.gameplayUnionMaskKnown ? 1 : 0)
            << " gUnionK=" << (ownerObserver.gameplayUnionMaskKnown ? 1 : 0)
            << " bucket30GameplayUnionMask="
            << ownerObserver.gameplayUnionMask
            << " gUnion=" << ownerObserver.gameplayUnionMask
            << " bucket30GameplayAnchorSlotIndexKnown="
            << (ownerObserver.gameplayAnchorSlotIndexKnown ? 1 : 0)
            << " gAnchorIdxK="
            << (ownerObserver.gameplayAnchorSlotIndexKnown ? 1 : 0)
            << " bucket30GameplayAnchorSlotIndex="
            << ownerObserver.gameplayAnchorSlotIndex
            << " gAnchorIdx=" << ownerObserver.gameplayAnchorSlotIndex
            << " bucket30GameplayRequiredClassTokenKnown="
            << (ownerObserver.gameplayRequiredClassTokenKnown ? 1 : 0)
            << " gReqClsK="
            << (ownerObserver.gameplayRequiredClassTokenKnown ? 1 : 0)
            << " bucket30GameplayRequiredClassToken="
            << ownerObserver.gameplayRequiredClassToken
            << " gReqCls=" << ownerObserver.gameplayRequiredClassToken
            << " bucket30GameplayAnchorSlotClassTokenKnown="
            << (ownerObserver.gameplayAnchorSlotClassTokenKnown ? 1 : 0)
            << " gAnchorClsK="
            << (ownerObserver.gameplayAnchorSlotClassTokenKnown ? 1 : 0)
            << " bucket30GameplayAnchorSlotClassToken="
            << ownerObserver.gameplayAnchorSlotClassToken
            << " gAnchorCls=" << ownerObserver.gameplayAnchorSlotClassToken
            << " bucket30GameplayAnchorSlotOccupiedKnown="
            << (ownerObserver.gameplayAnchorSlotOccupiedKnown ? 1 : 0)
            << " gOccK=" << (ownerObserver.gameplayAnchorSlotOccupiedKnown ? 1 : 0)
            << " bucket30GameplayAnchorSlotOccupied="
            << ownerObserver.gameplayAnchorSlotOccupied
            << " gOcc=" << ownerObserver.gameplayAnchorSlotOccupied
            << " bucket30GameplayAnchorClassMatchKnown="
            << (ownerObserver.gameplayAnchorClassMatchKnown ? 1 : 0)
            << " gAnchorMatchK="
            << (ownerObserver.gameplayAnchorClassMatchKnown ? 1 : 0)
            << " bucket30GameplayAnchorClassMatch="
            << ownerObserver.gameplayAnchorClassMatch
            << " gAnchorMatch=" << ownerObserver.gameplayAnchorClassMatch
            << " bucket30GameplayPairBonusKnown="
            << (ownerObserver.gameplayPairBonusKnown ? 1 : 0)
            << " gPairK="
            << (ownerObserver.gameplayPairBonusKnown ? 1 : 0)
            << " bucket30GameplayPairBonus="
            << ownerObserver.gameplayPairBonus
            << " gPair=" << ownerObserver.gameplayPairBonus
            << " bucket30GameplaySpillPenaltyKnown="
            << (ownerObserver.gameplaySpillPenaltyKnown ? 1 : 0)
            << " gSpillK="
            << (ownerObserver.gameplaySpillPenaltyKnown ? 1 : 0)
            << " bucket30GameplaySpillPenalty="
            << ownerObserver.gameplaySpillPenalty
            << " gSpill=" << ownerObserver.gameplaySpillPenalty
            << " bucket30GameplayAdditiveTermKnown="
            << (ownerObserver.gameplayAdditiveTermKnown ? 1 : 0)
            << " gAddK="
            << (ownerObserver.gameplayAdditiveTermKnown ? 1 : 0)
            << " bucket30GameplayAdditiveTerm="
            << ownerObserver.gameplayAdditiveTerm
            << " gAdd=" << ownerObserver.gameplayAdditiveTerm
            << " bucket30GameplayCommitTermKnown="
            << (ownerObserver.gameplayCommitTermKnown ? 1 : 0)
            << " gCommitK="
            << (ownerObserver.gameplayCommitTermKnown ? 1 : 0)
            << " bucket30GameplayCommitTerm="
            << ownerObserver.gameplayCommitTerm
            << " gCommit=" << ownerObserver.gameplayCommitTerm
            << " bucket30GameplayScoreWritebackKnown="
            << (ownerObserver.gameplayScoreWritebackKnown ? 1 : 0)
            << " gWbK="
            << (ownerObserver.gameplayScoreWritebackKnown ? 1 : 0)
            << " bucket30GameplayScoreWriteback="
            << ownerObserver.gameplayScoreWriteback
            << " gWb=" << ownerObserver.gameplayScoreWriteback
            << " bucket30GameplayClampActiveKnown="
            << (ownerObserver.gameplayClampActiveKnown ? 1 : 0)
            << " gClampK="
            << (ownerObserver.gameplayClampActiveKnown ? 1 : 0)
            << " bucket30GameplayClampActive="
            << (ownerObserver.gameplayClampActive ? 1 : 0)
            << " gClamp=" << (ownerObserver.gameplayClampActive ? 1 : 0)
            << " bucket30OwnerPhase1PrevBaseline18="
            << ownerObserver.phase1PrevBaseline18
            << " bucket30OwnerPhase1LiveAccumulator91816="
            << ownerObserver.phase1LiveAccumulator91816
            << " bucket30OwnerPhase1Delta91816MinusPrev18="
            << ownerObserver.phase1Delta91816MinusPrev18
            << " bucket30OwnerProducerGrowthBaseline18="
            << ownerObserver.producerGrowthBaseline18
            << " bucket30OwnerBlinkBaseline18="
            << ownerObserver.blinkBaseline18
            << " bucket30OwnerHelperSnapshot18="
            << ownerObserver.helperSnapshot18
            << " bucket30OwnerAfterAdvancePhase1Classifier36="
            << (int)ownerObserver.afterAdvancePhase1Classifier36
            << " bucket30OwnerAfterAdvancePhase1LatchArmed38="
            << (ownerObserver.afterAdvancePhase1LatchArmed38 ? 1 : 0)
            << " bucket30OwnerFormalWritebackKnown="
            << (ownerObserver.formalWritebackKnown ? 1 : 0)
            << " bucket30OwnerFormalWritebackValue="
            << ownerObserver.formalWritebackValue
            << " bucket30OwnerTieBreakerCalled="
            << (ownerObserver.tieBreakerCalled ? 1 : 0)
            << " bucket30OwnerTieBreakerResult14548="
            << ownerObserver.tieBreakerResult14548
            << " bucket30OwnerResolverGateBit4="
            << (ownerObserver.resolverGateBit4 ? 1 : 0)
            << " bucket30OwnerResolverGateEd00Idle="
            << (ownerObserver.resolverGateEd00Idle ? 1 : 0)
            << " bucket30OwnerResolutionCalled="
            << (ownerObserver.resolutionCalled ? 1 : 0)
            << " bucket30OwnerResolutionKnown="
            << (ownerObserver.resolutionKnown ? 1 : 0)
            << " bucket30OwnerResolutionInputPhase1Classifier36="
            << (int)ownerObserver.resolutionInputPhase1Classifier36
            << " ownResIn36="
            << (int)ownerObserver.resolutionInputPhase1Classifier36
            << " bucket30OwnerResolutionV22="
            << (int)ownerObserver.resolutionV22
            << " bucket30OwnerGoodToCoolCommitted="
            << (ownerObserver.goodToCoolCommitted ? 1 : 0)
            << " bucket30OwnerAfterProducePhase1Classifier36="
            << (int)ownerObserver.afterProducePhase1Classifier36
            << " bucket30OwnerAfterProducePhase1LatchArmed38="
            << (ownerObserver.afterProducePhase1LatchArmed38 ? 1 : 0)
            << " bucket30OwnerAfterProduceGrowthBaseline18="
            << ownerObserver.afterProduceGrowthBaseline18
            << " bucket30OwnerAfterProduceBlinkBaseline18="
            << ownerObserver.afterProduceBlinkBaseline18
            << " bucket30OwnerAfterProduceHelperSnapshot18="
            << ownerObserver.afterProduceHelperSnapshot18
            << " accumulator91816=" << scorer.accumulator91816
            << " baseline91818=" << phase1Owner.baselineValue18
            << " snapshot9181A=" << tieBreaker.snapshot1A
            << " snapshot9181C=" << helperShadow.snapshot1C
            << " carry9181E=" << helperShadow.tieCarryLatch1E
            << " descriptorED08=" << rightRankDescriptorFlagWord
            << " phaseCacheED36="
            << (int)phase1Owner.cachedPhase1Classifier36
            << " phaseCounterED38="
            << (phase1Owner.phase1LatchArmed38 ? 1 : 0)
            << " lastRowWriteKnown="
            << (numeric.rightRankLastRowWriteKnown ? 1 : 0)
            << " lastRowWriteQuery="
            << numeric.rightRankLastRowWriteQueryFrame
            << " lastRowWritePrevRow="
            << (int)numeric.rightRankLastRowWritePrevRow
            << " lastRowWriteResolvedRow="
            << (int)numeric.rightRankLastRowWriteResolvedRow
            << " lastRowWriteResolutionKnown="
            << (numeric.rightRankLastRowWriteResolutionKnown ? 1 : 0)
            << " lastRowWriteResolutionV22="
            << (int)numeric.rightRankLastRowWriteResolutionV22
            << " lastRowWriteCommitted="
            << (numeric.rightRankLastRowWriteCommitted ? 1 : 0)
            << " firstRow3WriteKnown="
            << (numeric.rightRankFirstRow3WriteKnown ? 1 : 0)
            << " firstRow3WriteQuery="
            << numeric.rightRankFirstRow3WriteQueryFrame
            << " firstRow3WritePrevRow="
            << (int)numeric.rightRankFirstRow3WritePrevRow
            << " firstRow3WriteResolvedRow="
            << (int)numeric.rightRankFirstRow3WriteResolvedRow
            << " firstRow3WriteResolutionKnown="
            << (numeric.rightRankFirstRow3WriteResolutionKnown ? 1 : 0)
            << " firstRow3WriteResolutionV22="
            << (int)numeric.rightRankFirstRow3WriteResolutionV22
            << " firstRow3WriteCommitted="
            << (numeric.rightRankFirstRow3WriteCommitted ? 1 : 0)
            << " firstRow3WriteOwnerActiveRow="
            << (int)numeric.rightRankFirstRow3WriteOwnerActiveRow
            << " firstRow3WriteResolutionInputPhase1Classifier36="
            << (int)numeric.rightRankFirstRow3WriteResolutionInputPhase1Classifier36
            << " firstRow3WritePhase1AdvanceFirstBeat="
            << (numeric.rightRankFirstRow3WritePhase1AdvanceFirstBeat ? 1 : 0)
            << " firstRow3WritePhase1Delta91816MinusPrev18="
            << numeric.rightRankFirstRow3WritePhase1Delta91816MinusPrev18
            << " firstRow3WriteFormalWritebackKnown="
            << (numeric.rightRankFirstRow3WriteFormalWritebackKnown ? 1 : 0)
            << " firstRow3WriteFormalWritebackValue="
            << numeric.rightRankFirstRow3WriteFormalWritebackValue
            << " firstRow3WriteTieBreakerCalled="
            << (numeric.rightRankFirstRow3WriteTieBreakerCalled ? 1 : 0)
            << " firstRow3WriteTieBreakerResult14548="
            << numeric.rightRankFirstRow3WriteTieBreakerResult14548
            << " firstRow3WriteResolverGateBit4="
            << (numeric.rightRankFirstRow3WriteResolverGateBit4 ? 1 : 0)
            << " firstRow3WriteResolverGateEd00Idle="
            << (numeric.rightRankFirstRow3WriteResolverGateEd00Idle ? 1 : 0)
            << " firstRow3WriteGoodToCoolCommitted="
            << (numeric.rightRankFirstRow3WriteGoodToCoolCommitted ? 1 : 0)
            << " ctx6AConsumerGate="
            << (numeric.rightRankBucketContext.ctx6AConsumerGate ? 1 : 0)
            << " ctx6AConsumerGateInitKnown="
            << (numeric.rightRankBucketContext.ctx6AConsumerGateInitKnown ? 1 : 0)
            << " ctx6AConsumerGateInitQuery="
            << numeric.rightRankBucketContext.ctx6AConsumerGateInitQueryFrame
            << " ctx6AConsumerGateInitValue="
            << numeric.rightRankBucketContext.ctx6AConsumerGateInitValue
            << " firstNoInputCounterProducerKnown="
            << (numeric.rightRankFirstNoInputCounterProducerKnown ? 1 : 0)
            << " firstNoInputCounterProducerQuery="
            << numeric.rightRankFirstNoInputCounterProducerQueryFrame
            << " firstNoInputCounterProducerAfterAcceptedClear="
            << (numeric.rightRankFirstNoInputCounterProducerAfterAcceptedClear ? 1 : 0)
            << " firstNoInputCounterProducerDeltaAfterAcceptedClear="
            << numeric.rightRankFirstNoInputCounterProducerDeltaAfterAcceptedClear
            << " firstNoInputCounterProducerDeltaAfterLastAcceptedCountClear="
            << numeric.rightRankFirstNoInputCounterProducerDeltaAfterLastAcceptedCountClear
            << " firstNoInputCounterProducerAcceptedSourceAfterLastClearKnown="
            << (numeric.rightRankFirstNoInputCounterProducerAcceptedSourceAfterLastClearKnown ? 1 : 0)
            << " firstNoInputCounterProducerAcceptedSourceAfterLastClear="
            << (numeric.rightRankFirstNoInputCounterProducerAcceptedSourceAfterLastClear ? 1 : 0)
            << " firstNoInputCounterProducerActiveRow="
            << (int)numeric.rightRankFirstNoInputCounterProducerActiveRow
            << " firstNoInputCounterProducerDescriptorFlags="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorFlags
            << " firstNoInputCounterProducerDescriptorLowBits03="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorLowBits03
            << " firstNoInputCounterProducerBusyGateActive="
            << (numeric.rightRankFirstNoInputCounterProducerBusyGateActive ? 1 : 0)
            << " firstNoInputCounterProducerScorerWindowOpen="
            << (numeric.rightRankFirstNoInputCounterProducerScorerWindowOpen ? 1 : 0)
            << " firstNoInputCounterProducerDescriptorSubstate50="
            << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorSubstate50
            << " firstNoInputCounterProducerDescriptorCadenceCursorAvailable="
            << (numeric.rightRankFirstNoInputCounterProducerDescriptorCadenceCursorAvailable ? 1 : 0)
            << " firstNoInputCounterProducerDescriptorCadenceCursorOrdinal1Based="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorCadenceCursorOrdinal1Based
            << " firstNoInputCounterProducerDescriptorPageOrdinal56Available="
            << (numeric.rightRankFirstNoInputCounterProducerDescriptorPageOrdinal56Available ? 1 : 0)
            << " firstNoInputCounterProducerDescriptorPageOrdinal56="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorPageOrdinal56
            << " firstNoInputCounterProducerDescriptorCurrentCommittedAvailable="
            << (numeric.rightRankFirstNoInputCounterProducerDescriptorCurrentCommittedAvailable ? 1 : 0)
            << " firstNoInputCounterProducerDescriptorCurrentCommittedRowIndex="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorCurrentCommittedRowIndex
            << " firstNoInputCounterProducerDescriptorCurrentDescriptor40Available="
            << (numeric.rightRankFirstNoInputCounterProducerDescriptorCurrentDescriptor40Available ? 1 : 0)
            << " firstNoInputCounterProducerDescriptorCurrentDescriptor40RowIndex="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorCurrentDescriptor40RowIndex
            << " firstNoInputCounterProducerDescriptorRowAvailable="
            << (numeric.rightRankFirstNoInputCounterProducerDescriptorRowAvailable ? 1 : 0)
            << " firstNoInputCounterProducerDescriptorLessonId="
            << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorLessonId
            << " firstNoInputCounterProducerDescriptorDefaultFlagWord="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorDefaultFlagWord
            << " firstNoInputCounterProducerDescriptorSubstateFlagWord="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorSubstateFlagWord
            << " firstNoInputCounterProducerDescriptorRequiredMask="
            << numeric.rightRankFirstNoInputCounterProducerDescriptorRequiredMask
            << " firstNoInputCounterProducerDescriptorAnchorSlotIndex="
            << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorAnchorSlotIndex
            << " firstNoInputCounterProducerDescriptorRequiredClassToken="
            << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorRequiredClassToken
            << " firstNoInputCounterProducerDescriptorDefaultSelector0="
            << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorDefaultSelector0
            << " firstNoInputCounterProducerDescriptorDefaultSelector1="
            << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorDefaultSelector1
            << " firstNoInputCounterProducerDescriptorSubstateSelector0="
            << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorSubstateSelector0
            << " firstNoInputCounterProducerDescriptorSubstateSelector1="
            << (int)numeric.rightRankFirstNoInputCounterProducerDescriptorSubstateSelector1
            << " firstNoInputCounterProducerCurrentBucket="
            << (int)numeric.rightRankFirstNoInputCounterProducerCurrentBucket
            << " firstNoInputCounterProducerPreviousBucket="
            << (int)numeric.rightRankFirstNoInputCounterProducerPreviousBucket
            << " firstNoInputCounterProducerBucket30Advanced="
            << (numeric.rightRankFirstNoInputCounterProducerBucket30Advanced ? 1 : 0)
            << " firstNoInputCounterProducerBucketAdvanceCount="
            << numeric.rightRankFirstNoInputCounterProducerBucketAdvanceCount
            << " firstNoInputCounterProducerAcceptedCountInput="
            << numeric.rightRankFirstNoInputCounterProducerAcceptedCountInput
            << " firstNoInputCounterProducerSteadyInputKnown="
            << (numeric.rightRankFirstNoInputCounterProducerSteadyInputKnown ? 1 : 0)
            << " firstNoInputCounterProducerSteadyInputHeldMask="
            << numeric.rightRankFirstNoInputCounterProducerSteadyInputHeldMask
            << " firstNoInputCounterProducerSteadyInputWriteCtx18="
            << (numeric.rightRankFirstNoInputCounterProducerSteadyInputWriteCtx18 ? 1 : 0)
            << " firstNoInputCounterProducerSteadyInputCtx18Value="
            << numeric.rightRankFirstNoInputCounterProducerSteadyInputCtx18Value
            << " firstNoInputCounterProducerSteadyInputWriteCtx20="
            << (numeric.rightRankFirstNoInputCounterProducerSteadyInputWriteCtx20 ? 1 : 0)
            << " firstNoInputCounterProducerSteadyInputCtx20Value="
            << numeric.rightRankFirstNoInputCounterProducerSteadyInputCtx20Value
            << " firstNoInputCounterProducerLocalHoldMask80035510="
            << numeric.rightRankFirstNoInputCounterProducerLocalHoldMask80035510
            << " firstNoInputCounterProducerLocalConsumedHoldMask80035510="
            << numeric.rightRankFirstNoInputCounterProducerLocalConsumedHoldMask80035510
            << " firstNoInputCounterProducerLocalDebounceBypassed80035510="
            << (numeric.rightRankFirstNoInputCounterProducerLocalDebounceBypassed80035510 ? 1 : 0)
            << " firstNoInputCounterProducerLastSteadyInputNonZeroKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastSteadyInputNonZeroQuery="
            << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroQueryFrame
            << " firstNoInputCounterProducerLastSteadyInputNonZeroDeltaToProducer="
            << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroDeltaToProducer
            << " firstNoInputCounterProducerLastSteadyInputNonZeroDeltaAfterLastAcceptedCountClear="
            << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroDeltaAfterLastAcceptedCountClear
            << " firstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClearKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClearKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClear="
            << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroAfterLastClear ? 1 : 0)
            << " firstNoInputCounterProducerLastSteadyInputNonZeroReplayMode52="
            << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroReplayMode52 ? 1 : 0)
            << " firstNoInputCounterProducerLastSteadyInputNonZeroHeldMask="
            << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroHeldMask
            << " firstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx18="
            << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx18 ? 1 : 0)
            << " firstNoInputCounterProducerLastSteadyInputNonZeroCtx18Value="
            << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroCtx18Value
            << " firstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx20="
            << (numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroWriteCtx20 ? 1 : 0)
            << " firstNoInputCounterProducerLastSteadyInputNonZeroCtx20Value="
            << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroCtx20Value
            << " firstNoInputCounterProducerLastSteadyInputNonZeroRequiredMaskOverlap="
            << numeric.rightRankFirstNoInputCounterProducerLastSteadyInputNonZeroRequiredMaskOverlap
            << " firstNoInputCounterProducerReplayMirrorKnown="
            << (numeric.rightRankFirstNoInputCounterProducerReplayMirrorKnown ? 1 : 0)
            << " firstNoInputCounterProducerReplayProducerKnown="
            << (numeric.rightRankFirstNoInputCounterProducerReplayProducerKnown ? 1 : 0)
            << " firstNoInputCounterProducerReplayBytesKnown="
            << (numeric.rightRankFirstNoInputCounterProducerReplayBytesKnown ? 1 : 0)
            << " firstNoInputCounterProducerReplayRequiredBytes="
            << numeric.rightRankFirstNoInputCounterProducerReplayRequiredBytes
            << " firstNoInputCounterProducerReplayKnownBytes="
            << numeric.rightRankFirstNoInputCounterProducerReplayKnownBytes
            << " firstNoInputCounterProducerReplayMissingBytes="
            << numeric.rightRankFirstNoInputCounterProducerReplayMissingBytes
            << " firstNoInputCounterProducerReplayFullBytes="
            << (numeric.rightRankFirstNoInputCounterProducerReplayFullBytes ? 1 : 0)
            << " firstNoInputCounterProducerReplayFullBackingKnown8008EEF8="
            << (numeric.rightRankFirstNoInputCounterProducerReplayFullBackingKnown8008EEF8 ? 1 : 0)
            << " lastReplayAppendKnown="
            << (numeric.rightRankLastReplayAppendKnown ? 1 : 0)
            << " lastReplayAppendQuery="
            << numeric.rightRankLastReplayAppendQueryFrame
            << " lastReplayAppendDeltaToHandoff="
            << (numeric.rightRankLastReplayAppendKnown
                    ? query - numeric.rightRankLastReplayAppendQueryFrame
                    : -1)
            << " lastReplayAppendKnownBytes="
            << numeric.rightRankLastReplayAppendKnownBytes
            << " lastReplayAppendMissingBytes="
            << numeric.rightRankLastReplayAppendMissingBytes
            << " lastReplayAppendPublishedCount="
            << numeric.rightRankLastReplayAppendPublishedCount
            << " lastReplayAppendWriteCount="
            << numeric.rightRankLastReplayAppendWriteCount
            << " lastReplayAppendFullBytes="
            << (numeric.rightRankLastReplayAppendFullBytes ? 1 : 0)
            << " lastReplayAppendFullBackingKnown8008EEF8="
            << (numeric.rightRankLastReplayAppendFullBackingKnown8008EEF8 ? 1 : 0)
            << " lastAcceptedTailDecisionKnown="
            << (numeric.rightRankLastAcceptedTailDecisionKnown ? 1 : 0)
            << " lastAcceptedTailDecisionQuery="
            << numeric.rightRankLastAcceptedTailDecisionQueryFrame
            << " lastAcceptedTailDecisionDeltaToHandoff="
            << (numeric.rightRankLastAcceptedTailDecisionKnown
                    ? query - numeric.rightRankLastAcceptedTailDecisionQueryFrame
                    : -1)
            << " lastAcceptedTailDecisionCtxInput18="
            << numeric.rightRankLastAcceptedTailDecisionCtxInput18
            << " lastAcceptedTailDecisionAcceptedMask9FF="
            << numeric.rightRankLastAcceptedTailDecisionAcceptedMask9FF
            << " lastAcceptedTailDecisionGateOpen="
            << (numeric.rightRankLastAcceptedTailDecisionGateOpen ? 1 : 0)
            << " lastAcceptedTailDecisionMaskChanged="
            << (numeric.rightRankLastAcceptedTailDecisionMaskChanged ? 1 : 0)
            << " lastAcceptedTailDecisionCall14614="
            << (numeric.rightRankLastAcceptedTailDecisionCall14614 ? 1 : 0)
            << " lastAcceptedTailDecisionReplayMode52="
            << (numeric.rightRankLastAcceptedTailDecisionReplayMode52 ? 1 : 0)
            << " acceptedCarrierAvailable="
            << (acceptedCarrier.available ? 1 : 0)
            << " acceptedCarrierSourceKind="
            << (int)acceptedCarrier.controlWriterSourceKind
            << " acceptedCarrierRawControl18="
            << acceptedCarrier.rawControlSample18
            << " acceptedCarrierControlMask18="
            << acceptedCarrier.controlMask18
            << " acceptedCarrierClassToken20="
            << (int)acceptedCarrier.classToken20
            << " acceptedCarrierStreamFlagKnown="
            << (acceptedCarrier.eventStreamFlagKnown ? 1 : 0)
            << " acceptedCarrierStreamFlag="
            << (acceptedCarrier.eventStreamFlagActive ? 1 : 0)
            << " acceptedCarrierStreamIdKnown="
            << (acceptedCarrier.eventStreamIdRawKnown ? 1 : 0)
            << " acceptedCarrierStreamIdRaw="
            << (int)acceptedCarrier.eventStreamIdRaw
            << " acceptedCarrierBusyGateKnown="
            << (acceptedCarrier.busyGate24BF4Known ? 1 : 0)
            << " acceptedCarrierBusyGate="
            << (acceptedCarrier.busyGate24BF4Active ? 1 : 0)
            << " acceptedCarrierAcceptedGateKnown="
            << (acceptedCarrier.acceptedGateKnown ? 1 : 0)
            << " acceptedCarrierAcceptedGate="
            << (acceptedCarrier.acceptedGateActive ? 1 : 0)
            << " acceptedCarrierTick96Known="
            << (acceptedCarrier.acceptedTick96Known ? 1 : 0)
            << " acceptedCarrierTick96="
            << acceptedCarrier.acceptedTick96
            << " lastAcceptedTailCallKnown="
            << (numeric.rightRankLastAcceptedTailCallKnown ? 1 : 0)
            << " lastAcceptedTailCallQuery="
            << numeric.rightRankLastAcceptedTailCallQueryFrame
            << " lastAcceptedTailCallDeltaToHandoff="
            << (numeric.rightRankLastAcceptedTailCallKnown
                    ? query - numeric.rightRankLastAcceptedTailCallQueryFrame
                    : -1)
            << " lastAcceptedTailCallAcceptedMask9FF="
            << numeric.rightRankLastAcceptedTailCallAcceptedMask9FF
            << " lastAcceptedTailCallDirectRunCaptured="
            << (numeric.rightRankLastAcceptedTailCallDirectRunCaptured ? 1 : 0)
            << " lastAcceptedTailCallDirectRunResult="
            << numeric.rightRankLastAcceptedTailCallDirectRunResult
            << " lastAcceptedTailCallReplayAppend="
            << (numeric.rightRankLastAcceptedTailCallReplayAppend ? 1 : 0)
            << " lastAcceptedTailCallTimingTemplateKnown="
            << (numeric.rightRankLastAcceptedTailCallTimingTemplateKnown ? 1 : 0)
            << " lastAcceptedTailCallTimingTemplateSlot48="
            << (int)numeric.rightRankLastAcceptedTailCallTimingTemplateSlot48
            << " lastAcceptedTailCallTimingTemplateState="
            << (int)numeric.rightRankLastAcceptedTailCallTimingTemplateState
            << " lastAcceptedTailCallAcceptedTick96Known="
            << (numeric.rightRankLastAcceptedTailCallAcceptedTick96Known ? 1 : 0)
            << " lastAcceptedTailCallAcceptedTick96="
            << numeric.rightRankLastAcceptedTailCallAcceptedTick96
            << " lastAcceptedTailCallPhase384="
            << numeric.rightRankLastAcceptedTailCallPhase384
            << " lastAcceptedTailCallRecordSlot24="
            << (int)numeric.rightRankLastAcceptedTailCallRecordSlot24
            << " lastAcceptedTailCallRecordRemainder24="
            << (int)numeric.rightRankLastAcceptedTailCallRecordRemainder24
            << " lastAcceptedTailCallSourceCellValid="
            << (numeric.rightRankLastAcceptedTailCallSourceCellValid ? 1 : 0)
            << " firstNoInputCounterProducerLastReplayAppendKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastReplayAppendQuery="
            << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendQueryFrame
            << " firstNoInputCounterProducerLastReplayAppendDeltaAfterLastAcceptedCountClear="
            << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendDeltaAfterLastAcceptedCountClear
            << " firstNoInputCounterProducerLastReplayAppendAfterLastClearKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendAfterLastClearKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastReplayAppendAfterLastClear="
            << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendAfterLastClear ? 1 : 0)
            << " firstNoInputCounterProducerLastReplayAppendKnownBytes="
            << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendKnownBytes
            << " firstNoInputCounterProducerLastReplayAppendMissingBytes="
            << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendMissingBytes
            << " firstNoInputCounterProducerLastReplayAppendPublishedCount="
            << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendPublishedCount
            << " firstNoInputCounterProducerLastReplayAppendWriteCount="
            << numeric.rightRankFirstNoInputCounterProducerLastReplayAppendWriteCount
            << " firstNoInputCounterProducerLastReplayAppendFullBytes="
            << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendFullBytes ? 1 : 0)
            << " firstNoInputCounterProducerLastReplayAppendFullBackingKnown8008EEF8="
            << (numeric.rightRankFirstNoInputCounterProducerLastReplayAppendFullBackingKnown8008EEF8 ? 1 : 0)
            << " acceptedReplayRestoreObserved="
            << (numeric.rightRankAcceptedReplayRestoreObserved ? 1 : 0)
            << " acceptedReplayRestorePayloadBackupValid="
            << (numeric.rightRankAcceptedReplayRestorePayloadBackupValid ? 1 : 0)
            << " acceptedReplayRestorePayloadFullBackingKnown8008EEF8="
            << (numeric.rightRankAcceptedReplayRestorePayloadFullBackingKnown8008EEF8 ? 1 : 0)
            << " acceptedReplayRestoreSidecarBackupValid="
            << (numeric.rightRankAcceptedReplayRestoreSidecarBackupValid ? 1 : 0)
            << " acceptedReplayRestoreSidecarFullBackingKnown8008EEF8="
            << (numeric.rightRankAcceptedReplayRestoreSidecarFullBackingKnown8008EEF8 ? 1 : 0)
            << " acceptedReplayRestoreSource1681C="
            << static_cast<int>(numeric.rightRankAcceptedReplayRestoreSource1681C)
            << " acceptedReplayRestoreRequested1681C="
            << (numeric.rightRankAcceptedReplayRestoreRequested1681C ? 1 : 0)
            << " acceptedReplayRestoreApplied1681C="
            << (numeric.rightRankAcceptedReplayRestoreApplied1681C ? 1 : 0)
            << " acceptedReplaySetupTransitionState="
            << numeric.rightRankAcceptedReplaySetupTransitionState
            << " acceptedReplayEventTableSeed801C8660Applied="
            << (numeric.rightRankAcceptedReplayEventTableSeed801C8660Applied ? 1 : 0)
            << " acceptedReplayRestorePostKnownBytes="
            << numeric.rightRankAcceptedReplayRestorePostKnownBytes
            << " acceptedReplayRestorePostFullBytes="
            << (numeric.rightRankAcceptedReplayRestorePostFullBytes ? 1 : 0)
            << " acceptedReplayRestorePostFullBackingKnown8008EEF8="
            << (numeric.rightRankAcceptedReplayRestorePostFullBackingKnown8008EEF8 ? 1 : 0)
            << Stage1OverlayDescribeAcceptedReplayPayloadGate(
                   numeric.rightRankAcceptedReplayRestorePayloadGate)
            << Stage1OverlayDescribeSavePayloadBankRuntime(
                   savePayloadBankRuntime)
            << " firstNoInputCounterProducerLastAcceptedTailDecisionKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailDecisionQuery="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionQueryFrame
            << " firstNoInputCounterProducerLastAcceptedTailDecisionCtxInput18="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionCtxInput18
            << " firstNoInputCounterProducerLastAcceptedTailDecisionPreviousInputMask801CCBB8="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionPreviousInputMask801CCBB8
            << " firstNoInputCounterProducerLastAcceptedTailDecisionAcceptedMask9FF="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionAcceptedMask9FF
            << " firstNoInputCounterProducerLastAcceptedTailDecisionGateOpen="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionGateOpen ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailDecisionMaskChanged="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionMaskChanged ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailDecisionCall14614="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionCall14614 ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailDecisionReplayMode52="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailDecisionReplayMode52 ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallQuery="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallQueryFrame
            << " firstNoInputCounterProducerLastAcceptedTailCallDeltaAfterLastAcceptedCountClear="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallDeltaAfterLastAcceptedCountClear
            << " firstNoInputCounterProducerLastAcceptedTailCallAfterLastClearKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAfterLastClearKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallAfterLastClear="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAfterLastClear ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallCtxInput18="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallCtxInput18
            << " firstNoInputCounterProducerLastAcceptedTailCallPreviousInputMask801CCBB8="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallPreviousInputMask801CCBB8
            << " firstNoInputCounterProducerLastAcceptedTailCallAcceptedMask9FF="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedMask9FF
            << " firstNoInputCounterProducerLastAcceptedTailCallDirectRunCaptured="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallDirectRunCaptured ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallDirectRunResult="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallDirectRunResult
            << " firstNoInputCounterProducerLastAcceptedTailCallReplayAppend="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallReplayAppend ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallSelectorResolved="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorResolved ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallSelectorByte0="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorByte0
            << " firstNoInputCounterProducerLastAcceptedTailCallSelectorByte1="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSelectorByte1
            << " firstNoInputCounterProducerLastAcceptedTailCallTimingTemplateKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallTimingTemplateSlot48="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateSlot48
            << " firstNoInputCounterProducerLastAcceptedTailCallTimingTemplateState="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallTimingTemplateState
            << " firstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96Known="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96Known ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallAcceptedTick96
            << " firstNoInputCounterProducerLastAcceptedTailCallHalfWindow34="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallHalfWindow34
            << " firstNoInputCounterProducerLastAcceptedTailCallPhase384="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallPhase384
            << " firstNoInputCounterProducerLastAcceptedTailCallRecordSlot24="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallRecordSlot24
            << " firstNoInputCounterProducerLastAcceptedTailCallRecordRemainder24="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallRecordRemainder24
            << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderValid="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderValid ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderAddr=0x"
            << std::hex
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderAddr
            << std::dec
            << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderBasePtr=0x"
            << std::hex
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderBasePtr
            << std::dec
            << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCount="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCount
            << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCursor="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellHeaderCursor
            << " firstNoInputCounterProducerLastAcceptedTailCallSourceCellValid="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedTailCallSourceCellValid ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountNonZeroKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountNonZeroQuery="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroQueryFrame
            << " firstNoInputCounterProducerLastAcceptedCountNonZeroValue="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroValue
            << " firstNoInputCounterProducerLastAcceptedCountNonZeroMask="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountNonZeroMask
            << " firstNoInputCounterProducerLastAcceptedCountClearKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearQuery="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearQueryFrame
            << " firstNoInputCounterProducerLastAcceptedCountClearSourceBucket="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearSourceBucket
            << " firstNoInputCounterProducerLastAcceptedCountClearPreCount="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreCount
            << " firstNoInputCounterProducerLastAcceptedCountClearPreMask="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreMask
            << " firstNoInputCounterProducerLastAcceptedCountClearMask9180C="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearMask9180C
            << " firstNoInputCounterProducerLastAcceptedCountClearAction="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearAction
            << " firstNoInputCounterProducerLastAcceptedCountClearPreBucket30Ed00="
            << numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearPreBucket30Ed00
            << " firstNoInputCounterProducerLastAcceptedCountClearDirectConsumer94400="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumer94400 ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearDirectConsumerImmediateFollowUpClear="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumerImmediateFollowUpClear ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearDirectConsumerOwnerNoResolution94400="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectConsumerOwnerNoResolution94400 ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearWaitSecondBeatBucket30="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearWaitSecondBeatBucket30 ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearAcceptedTailSurvived="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearAcceptedTailSurvived ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearOwnerKernelOpen="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearOwnerKernelOpen ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearResolverGateBit4="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearResolverGateBit4 ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearResolutionGateEd00Idle="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearResolutionGateEd00Idle ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearPhase1LatchArmed38="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearPhase1LatchArmed38 ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearFollowUpPhaseIsNone="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearFollowUpPhaseIsNone ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionV22="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteResolutionV22
            << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteCommitted="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteCommitted ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteGoodToCoolCommitted="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteGoodToCoolCommitted ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearRowWriteDirectConsumerFallback94400="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearRowWriteDirectConsumerFallback94400 ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearDirectSlotKnown="
            << (numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectSlotKnown ? 1 : 0)
            << " firstNoInputCounterProducerLastAcceptedCountClearDirectSlot="
            << (int)numeric.rightRankFirstNoInputCounterProducerLastAcceptedCountClearDirectSlot
            << " firstNoInputCounterProducerInput="
            << numeric.rightRankFirstNoInputCounterProducerInput
            << " firstNoInputCounterProducerOutput="
            << numeric.rightRankFirstNoInputCounterProducerOutput
            << " firstNoInputCounterProducerIncremented="
            << (numeric.rightRankFirstNoInputCounterProducerIncremented ? 1 : 0)
            << " firstNoInputCounterProducerSameSliceClearAction="
            << (int)numeric.rightRankFirstNoInputCounterProducerSameSliceClearAction
            << " firstNoInputCounterProducerSameSliceClearRan="
            << (numeric.rightRankFirstNoInputCounterProducerSameSliceClearRan ? 1 : 0)
            << " firstNoInputCounterProducerBeforeSameSliceClear="
            << (numeric.rightRankFirstNoInputCounterProducerBeforeSameSliceClear ? 1 : 0)
            << " firstNoInputCounterProducerCtx6AInitKnown="
            << (numeric.rightRankFirstNoInputCounterProducerCtx6AInitKnown ? 1 : 0)
            << " firstNoInputCounterProducerCtx6AInitQuery="
            << numeric.rightRankFirstNoInputCounterProducerCtx6AInitQueryFrame
            << " firstNoInputCounterProducerCtx6AInitValue="
            << numeric.rightRankFirstNoInputCounterProducerCtx6AInitValue
            << " firstAcceptedCountClearKnown="
            << (numeric.rightRankFirstAcceptedCountClearKnown ? 1 : 0)
            << " firstAcceptedCountClearQuery="
            << numeric.rightRankFirstAcceptedCountClearQueryFrame
            << " firstAcceptedCountClearSourceBucket="
            << (int)numeric.rightRankFirstAcceptedCountClearSourceBucket
            << " firstAcceptedCountClearCurrentBucket="
            << (int)numeric.rightRankFirstAcceptedCountClearCurrentBucket
            << " firstAcceptedCountClearPreviousBucket="
            << (int)numeric.rightRankFirstAcceptedCountClearPreviousBucket
            << " firstAcceptedCountClearBucket30Advanced="
            << (numeric.rightRankFirstAcceptedCountClearBucket30Advanced ? 1 : 0)
            << " firstAcceptedCountClearBucketAdvanceCount="
            << numeric.rightRankFirstAcceptedCountClearBucketAdvanceCount
            << " firstAcceptedCountClearPreCount="
            << numeric.rightRankFirstAcceptedCountClearPreCount
            << " firstAcceptedCountClearPostCount="
            << numeric.rightRankFirstAcceptedCountClearPostCount
            << " firstAcceptedCountClearPreMask="
            << numeric.rightRankFirstAcceptedCountClearPreMask
            << " firstAcceptedCountClearPostMask="
            << numeric.rightRankFirstAcceptedCountClearPostMask
            << " firstAcceptedCountClearMask9180C="
            << numeric.rightRankFirstAcceptedCountClearMask9180C
            << " firstAcceptedCountClearBucketLocalClearRan="
            << (numeric.rightRankFirstAcceptedCountClearBucketLocalClearRan ? 1 : 0)
            << " firstAcceptedCountClearNarrowFired="
            << (numeric.rightRankFirstAcceptedCountClearNarrowFired ? 1 : 0)
            << " firstAcceptedCountClearAction="
            << (int)numeric.rightRankFirstAcceptedCountClearAction
            << " firstAcceptedCountClearPreBucket30Ed00="
            << numeric.rightRankFirstAcceptedCountClearPreBucket30Ed00
            << " firstAcceptedCountClearDirectConsumer94400="
            << (numeric.rightRankFirstAcceptedCountClearDirectConsumer94400 ? 1 : 0)
            << " firstAcceptedCountClearDirectConsumerImmediateFollowUpClear="
            << (numeric.rightRankFirstAcceptedCountClearDirectConsumerImmediateFollowUpClear ? 1 : 0)
            << " firstAcceptedCountClearDirectConsumerOwnerNoResolution94400="
            << (numeric.rightRankFirstAcceptedCountClearDirectConsumerOwnerNoResolution94400 ? 1 : 0)
            << " firstAcceptedCountClearWaitSecondBeatBucket30="
            << (numeric.rightRankFirstAcceptedCountClearWaitSecondBeatBucket30 ? 1 : 0)
            << " firstAcceptedCountClearAcceptedTailSurvived="
            << (numeric.rightRankFirstAcceptedCountClearAcceptedTailSurvived ? 1 : 0)
            << " firstAcceptedCountClearOwnerKernelOpen="
            << (numeric.rightRankFirstAcceptedCountClearOwnerKernelOpen ? 1 : 0)
            << " firstAcceptedCountClearResolverGateBit4="
            << (numeric.rightRankFirstAcceptedCountClearResolverGateBit4 ? 1 : 0)
            << " firstAcceptedCountClearResolutionGateEd00Idle="
            << (numeric.rightRankFirstAcceptedCountClearResolutionGateEd00Idle ? 1 : 0)
            << " firstAcceptedCountClearPhase1LatchArmed38="
            << (numeric.rightRankFirstAcceptedCountClearPhase1LatchArmed38 ? 1 : 0)
            << " firstAcceptedCountClearFollowUpPhaseIsNone="
            << (numeric.rightRankFirstAcceptedCountClearFollowUpPhaseIsNone ? 1 : 0)
            << " firstAcceptedCountClearRowWriteResolutionKnown="
            << (numeric.rightRankFirstAcceptedCountClearRowWriteResolutionKnown ? 1 : 0)
            << " firstAcceptedCountClearRowWriteResolutionV22="
            << (int)numeric.rightRankFirstAcceptedCountClearRowWriteResolutionV22
            << " firstAcceptedCountClearRowWriteResolutionSkippedMissingResolverGateBit4="
            << (numeric.rightRankFirstAcceptedCountClearRowWriteResolutionSkippedMissingResolverGateBit4 ? 1 : 0)
            << " firstAcceptedCountClearRowWriteResolutionSkippedFollowUpActive="
            << (numeric.rightRankFirstAcceptedCountClearRowWriteResolutionSkippedFollowUpActive ? 1 : 0)
            << " firstAcceptedCountClearRowWriteCommitted="
            << (numeric.rightRankFirstAcceptedCountClearRowWriteCommitted ? 1 : 0)
            << " firstAcceptedCountClearRowWriteGoodToCoolCommitted="
            << (numeric.rightRankFirstAcceptedCountClearRowWriteGoodToCoolCommitted ? 1 : 0)
            << " firstAcceptedCountClearRowWriteDirectConsumerFallback94400="
            << (numeric.rightRankFirstAcceptedCountClearRowWriteDirectConsumerFallback94400 ? 1 : 0)
            << " firstAcceptedCountClearDirectSlotKnown="
            << (numeric.rightRankFirstAcceptedCountClearDirectSlotKnown ? 1 : 0)
            << " firstAcceptedCountClearDirectSlot="
            << (int)numeric.rightRankFirstAcceptedCountClearDirectSlot
            << " firstAcceptedCountClearDescriptorFlagKnown="
            << (numeric.rightRankFirstAcceptedCountClearDescriptorFlagKnown ? 1 : 0)
            << " firstAcceptedCountClearDescriptorFlagWord="
            << numeric.rightRankFirstAcceptedCountClearDescriptorFlagWord
            << " firstAcceptedCountClearDescriptorSubstate50="
            << (int)numeric.rightRankFirstAcceptedCountClearDescriptorSubstate50
            << " firstAcceptedCountClearDescriptorCadenceCursorAvailable="
            << (numeric.rightRankFirstAcceptedCountClearDescriptorCadenceCursorAvailable ? 1 : 0)
            << " firstAcceptedCountClearDescriptorCadenceCursorOrdinal1Based="
            << numeric.rightRankFirstAcceptedCountClearDescriptorCadenceCursorOrdinal1Based
            << " firstAcceptedCountClearDescriptorPageOrdinal56Available="
            << (numeric.rightRankFirstAcceptedCountClearDescriptorPageOrdinal56Available ? 1 : 0)
            << " firstAcceptedCountClearDescriptorPageOrdinal56="
            << numeric.rightRankFirstAcceptedCountClearDescriptorPageOrdinal56
            << " firstAcceptedCountClearDescriptorCurrentCommittedAvailable="
            << (numeric.rightRankFirstAcceptedCountClearDescriptorCurrentCommittedAvailable ? 1 : 0)
            << " firstAcceptedCountClearDescriptorCurrentCommittedRowIndex="
            << numeric.rightRankFirstAcceptedCountClearDescriptorCurrentCommittedRowIndex
            << " firstAcceptedCountClearDescriptorCurrentDescriptor40Available="
            << (numeric.rightRankFirstAcceptedCountClearDescriptorCurrentDescriptor40Available ? 1 : 0)
            << " firstAcceptedCountClearDescriptorCurrentDescriptor40RowIndex="
            << numeric.rightRankFirstAcceptedCountClearDescriptorCurrentDescriptor40RowIndex
            << " firstAcceptedCountClearDescriptorRowAvailable="
            << (numeric.rightRankFirstAcceptedCountClearDescriptorRowAvailable ? 1 : 0)
            << " firstAcceptedCountClearDescriptorLessonId="
            << (int)numeric.rightRankFirstAcceptedCountClearDescriptorLessonId
            << " firstAcceptedCountClearDescriptorDefaultSelector0="
            << (int)numeric.rightRankFirstAcceptedCountClearDescriptorDefaultSelector0
            << " firstAcceptedCountClearDescriptorDefaultSelector1="
            << (int)numeric.rightRankFirstAcceptedCountClearDescriptorDefaultSelector1
            << " firstAcceptedCountClearDescriptorDefaultFlagWord="
            << numeric.rightRankFirstAcceptedCountClearDescriptorDefaultFlagWord
            << " firstAcceptedCountClearDescriptorSubstateSelector0="
            << (int)numeric.rightRankFirstAcceptedCountClearDescriptorSubstateSelector0
            << " firstAcceptedCountClearDescriptorSubstateSelector1="
            << (int)numeric.rightRankFirstAcceptedCountClearDescriptorSubstateSelector1
            << " firstAcceptedCountClearDescriptorSubstateFlagWord="
            << numeric.rightRankFirstAcceptedCountClearDescriptorSubstateFlagWord
            << " firstAcceptedCountClearGameplayReaderPageOrdinalKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayReaderPageOrdinalKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayReaderPageOrdinal="
            << numeric.rightRankFirstAcceptedCountClearGameplayReaderPageOrdinal
            << " firstAcceptedCountClearGameplayRequiredMaskKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayRequiredMaskKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayRequiredMask="
            << numeric.rightRankFirstAcceptedCountClearGameplayRequiredMask
            << " firstAcceptedCountClearGameplayUnionMaskKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayUnionMaskKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayUnionMask="
            << numeric.rightRankFirstAcceptedCountClearGameplayUnionMask
            << " firstAcceptedCountClearGameplayAnchorSlotIndexKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotIndexKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorSlotIndex="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotIndex
            << " firstAcceptedCountClearGameplayRequiredClassTokenKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayRequiredClassTokenKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayRequiredClassToken="
            << numeric.rightRankFirstAcceptedCountClearGameplayRequiredClassToken
            << " firstAcceptedCountClearGameplayAnchorSlotClassTokenKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotClassTokenKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorSlotClassToken="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotClassToken
            << " firstAcceptedCountClearGameplayAnchorSlotOccupiedKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotOccupiedKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorSlotOccupied="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotOccupied
            << " firstAcceptedCountClearGameplayAnchorSlotAcceptedMaskKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotAcceptedMaskKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorSlotAcceptedMask="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotAcceptedMask
            << " firstAcceptedCountClearGameplayAnchorSlotPayloadKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotPayloadKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorSlotPayload="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorSlotPayload
            << " firstAcceptedCountClearGameplayAnchorPageOccupiedSlotBitsKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageOccupiedSlotBitsKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorPageOccupiedSlotBits="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageOccupiedSlotBits
            << " firstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBitsKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBitsKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBits="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredMaskSlotBits
            << " firstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBitsKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBitsKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBits="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorPageRequiredOccupiedSlotBits
            << " firstAcceptedCountClearGameplayAnchorClassMatchKnown="
            << (numeric.rightRankFirstAcceptedCountClearGameplayAnchorClassMatchKnown ? 1 : 0)
            << " firstAcceptedCountClearGameplayAnchorClassMatch="
            << numeric.rightRankFirstAcceptedCountClearGameplayAnchorClassMatch
            << " followUpPhaseId=" << (int)numeric.rightRankFollowUpPhase
            << " followUpPhase=" << (int)numeric.rightRankFollowUpPhase
            << " acceptedNarrowClearPending="
            << (numeric.acceptedProducerNarrowClearPending ? 1 : 0)
            << " acceptedNarrowClearFired="
            << (numeric.acceptedProducerNarrowClearFired ? 1 : 0)
            << " acceptedNarrowClearPhaseId="
            << (int)numeric.acceptedProducerNarrowClearPhase
            << Stage1OverlayDescribeBucket31Flag0200Producer(
                   numeric.bucket31Flag0200Producer)
            << " gate=" << (numeric.active ? 1 : 0)
            << " bucket=" << (int)numeric.bucketCadence.currentBucket
            << " prevBucket=" << (int)numeric.bucketCadence.previousBucket
            << " bucketChanged="
            << (numeric.bucketCadence.bucketChanged ? 1 : 0)
            << " bucket0=" << (numeric.bucketCadence.bucket0Advanced ? 1 : 0)
            << " bucket30="
            << (numeric.bucketCadence.bucket30Advanced ? 1 : 0)
            << " bucket31="
            << (numeric.bucketCadence.bucket31Advanced ? 1 : 0)
            << " bucketAdvanceCount="
            << numeric.bucketCadence.bucketAdvanceCount
            << " topLessonVisible="
            << (numeric.topLessonPairState.topLessonPairChangeVisible ? 1 : 0)
            << " topLessonLessonId="
            << (int)numeric.topLessonPairState.topLessonPairLessonId
            << " topLessonCurrentCommittedAvailable="
            << (descriptorCadence.currentCommittedAvailable ? 1 : 0)
            << " topLessonCurrentCommittedRow="
            << descriptorCadence.currentCommittedRowIndex
            << " topLessonCurrentCommittedLessonId="
            << (int)descriptorCadence.currentCommittedRow.lessonId
            << " topLessonNextLookaheadAvailable="
            << (descriptorCadence.nextLookaheadAvailable ? 1 : 0)
            << " topLessonNextLookaheadRow="
            << descriptorCadence.nextLookaheadRowIndex
            << " topLessonNextLookaheadLessonId="
            << (int)descriptorCadence.nextLookaheadRow.lessonId
            << " desc44Available=" << (desc44Available ? 1 : 0)
            << " desc44Row=" << descriptorCadence.nextLookaheadRowIndex
            << " desc44Lesson=" << (int)desc44Row.lessonId
            << " desc44Substate50=" << desc44Substate50
            << " desc44RequiredMask=" << desc44Row.requiredMask
            << " desc44DefaultSel0=" << (int)desc44Row.defaultSelectorByte0
            << " desc44DefaultSel1=" << (int)desc44Row.defaultSelectorByte1
            << " desc44SubstateSel0=" << (int)desc44Row.substate1SelectorByte0
            << " desc44SubstateSel1=" << (int)desc44Row.substate1SelectorByte1
            << " desc44BranchSel0=" << (int)desc44BranchSelectorByte0
            << " desc44BranchSel1=" << (int)desc44BranchSelectorByte1
            << " formulaAvailable=" << (numeric.active ? 1 : 0)
            << " formulaQuery="
            << (numeric.active ? (int)numeric.queryFrame : query)
            << " branchKnown=0"
            << " acceptedKnown=" << (numeric.active ? 1 : 0)
            << " accepted=" << acceptedFormulaCount
            << " acceptedContributionCountKnown="
            << (numeric.active ? 1 : 0)
            << " acceptedContributionCount="
            << accepted.acceptedContributionCount
            << " recordedSplitKnown=" << (numeric.active ? 1 : 0)
            << " recordedSplit=" << accepted.recordedSplitCount
            << " penaltySplitKnown=" << (numeric.active ? 1 : 0)
            << " penaltySplit=" << accepted.penaltySideSplitCount
            << " aggregateAcceptedMaskKnown="
            << (numeric.active ? 1 : 0)
            << " aggregateAcceptedMask=" << accepted.aggregateAcceptedMask
            << " add22Known=" << (numeric.active ? 1 : 0)
            << " add22=" << numeric.additiveLane.value
            << " acceptedContribution="
            << (acceptedProbe.materialized ? 1 : 0)
            << " rawPressedMask=" << acceptedProbe.rawPressedMask
            << " rawAcceptedMask=" << acceptedProbe.rawAcceptedMask
            << " controlWriterKind="
            << (int)acceptedCarrier.controlWriterSourceKind
            << " rawControl18=" << acceptedCarrier.rawControlSample18
            << " control18=" << acceptedCarrier.controlMask18
            << " class20=" << (int)acceptedCarrier.classToken20
            << " acceptedTickSeedQuery="
            << acceptedCarrier.acceptedTick96LastUpdateQueryFrame
            << " acceptedTickSeedKind="
            << (int)acceptedCarrier.acceptedTick96LastUpdateSourceKind
            << " acceptedTickSeedControl18="
            << acceptedCarrier.acceptedTick96LastUpdateControlMask18
            << " acceptedTickSeedClass20="
            << (int)acceptedCarrier.acceptedTick96LastUpdateClassToken20
            << " acceptedTickSeedPostCtx10="
            << (acceptedCarrier.acceptedTick96LastUpdateViaPostCtx10 ? 1 : 0)
            << " acceptedTickSeedCtxInput18="
            << acceptedCarrier.acceptedTick96LastUpdateCtxInput18
            << " acceptedTickSeedPrevMask="
            << acceptedCarrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8
            << " acceptedTickSeedMaskChanged="
            << (acceptedCarrier.acceptedTick96LastUpdateAcceptedMaskChanged ? 1 : 0)
            << " carrierCurrentTickProbeDisabled="
            << (acceptedCarrier.carrierCurrentTickProbeDisabled ? 1 : 0)
            << " carrierCurrentTickSeedAllowed="
            << (acceptedCarrier.carrierCurrentTickSeedAllowed ? 1 : 0)
            << " postCtxInput18="
            << numeric.runnerPostFrame7A60.ctxInput18
            << " postAcceptedMask9FF="
            << numeric.runnerPostFrame7A60.acceptedMask9FF
            << " postAcceptedGateOpen="
            << (numeric.runnerPostFrame7A60.acceptedGateOpen ? 1 : 0)
            << " postAcceptedMaskChanged="
            << (numeric.runnerPostFrame7A60.acceptedMaskChanged ? 1 : 0)
            << " postBackupCtx10="
            << (numeric.runnerPostFrame7A60.backupCtx10FromCtx0C ? 1 : 0)
            << " postCall14614="
            << (numeric.runnerPostFrame7A60.callAcceptedProducer14614 ? 1 : 0)
            << " initialPostKnown="
            << (acceptedProbe.initialPostKnown ? 1 : 0)
            << " initialPostCtxInput18="
            << acceptedProbe.initialPostCtxInput18
            << " initialPostPrevMask="
            << acceptedProbe.initialPostPreviousInputMask801CCBB8
            << " initialPostMask9FF="
            << acceptedProbe.initialPostAcceptedMask9FF
            << " initialPostGateOpen="
            << (acceptedProbe.initialPostAcceptedGateOpen ? 1 : 0)
            << " initialPostMaskChanged="
            << (acceptedProbe.initialPostAcceptedMaskChanged ? 1 : 0)
            << " initialPostBackupCtx10="
            << (acceptedProbe.initialPostBackupCtx10FromCtx0C ? 1 : 0)
            << " initialPostCall14614="
            << (acceptedProbe.initialPostCallAcceptedProducer14614 ? 1 : 0)
            << " frontDoorCurrentTickProbeDisabled="
            << (acceptedProbe.frontDoorCurrentTickProbeDisabled ? 1 : 0)
            << " frontDoorCurrentTickSeedAllowed="
            << (acceptedProbe.frontDoorCurrentTickSeedAllowed ? 1 : 0)
            << " direct14614Captured="
            << (acceptedProbe.directAcceptedRunCaptured ? 1 : 0)
            << " direct14614Result="
            << acceptedProbe.directAcceptedRunResultCode
            << " direct14614ReplayAppend="
            << (acceptedProbe.directAcceptedRunReplayAppendRan ? 1 : 0)
            << " direct14614WriteRan="
            << (acceptedProbe.directAcceptedRunWriteRan ? 1 : 0)
            << " direct14614WriteResult="
            << acceptedProbe.directAcceptedRunWriteResultCode
            << " direct14614Selector="
            << (acceptedProbe.directAcceptedRunSelectorResolved ? 1 : 0)
            << " direct14614TimingKnown="
            << (acceptedProbe.directAcceptedRunTimingTemplateKnown ? 1 : 0)
            << " direct14614TimingAddr=0x" << std::hex
            << acceptedProbe.directAcceptedRunTimingTemplateAddress << std::dec
            << " direct14614TimingSlot48="
            << (int)acceptedProbe.directAcceptedRunTimingTemplateSlot48
            << " direct14614TimingState="
            << (int)acceptedProbe.directAcceptedRunTimingTemplateState
            << " direct14614SourceCell="
            << (acceptedProbe.directAcceptedRunSourceCellValid ? 1 : 0)
            << " direct14614AcceptedTick96Known="
            << (acceptedProbe.directAcceptedRunAcceptedTick96Known ? 1 : 0)
            << " direct14614AcceptedTick96="
            << acceptedProbe.directAcceptedRunAcceptedTick96
            << " direct14614Phase384="
            << acceptedProbe.directAcceptedRunPhase384
            << " direct14614RecordSlot24="
            << (int)acceptedProbe.directAcceptedRunRecordSlot24
            << " direct14614RecordRem24="
            << (int)acceptedProbe.directAcceptedRunRecordRemainder24
            << " direct14614SourceCellCursor="
            << acceptedProbe.directAcceptedRunSourceCellCursor
            << " direct14614PageWriteApplied="
            << (acceptedProbe.directAcceptedRunPageWriteApplied ? 1 : 0)
            << " direct14614WritePage38="
            << acceptedProbe.directAcceptedRunWritePageOrdinal
            << " direct14614PageRecordSlot="
            << (int)acceptedProbe.directAcceptedRunPageRecordSlot
            << " direct14614DescSubstate50="
            << acceptedProbe.directInputDescriptorSubstate50
            << " direct14614RowValid="
            << (acceptedProbe.directInputLookaheadRowValid ? 1 : 0)
            << " direct14614RowLesson="
            << (int)acceptedProbe.directInputLookaheadLessonId
            << " direct14614DefaultSel0="
            << (int)acceptedProbe.directInputDefaultSelectorByte0
            << " direct14614DefaultSel1="
            << (int)acceptedProbe.directInputDefaultSelectorByte1
            << " direct14614SubstateSel0="
            << (int)acceptedProbe.directInputSubstate1SelectorByte0
            << " direct14614SubstateSel1="
            << (int)acceptedProbe.directInputSubstate1SelectorByte1
            << " direct14614BranchSel0="
            << (int)acceptedProbe.directInputBranchSelectorByte0
            << " direct14614BranchSel1="
            << (int)acceptedProbe.directInputBranchSelectorByte1
            << " steadyKnown=" << (numeric.steadyInput7A60.known ? 1 : 0)
            << " steadyReplay52=" << (numeric.steadyInput7A60.replayMode52 ? 1 : 0)
            << " steadyHeldMask=" << numeric.steadyInput7A60.heldMask
            << " steadyWriteCtx18=" << (numeric.steadyInput7A60.writeCtx18 ? 1 : 0)
            << " steadyCtx18=" << numeric.steadyInput7A60.ctx18Value
            << " steadyWriteCtx20=" << (numeric.steadyInput7A60.writeCtx20 ? 1 : 0)
            << " steadyCtx20Known=" << (numeric.steadyInput7A60.ctx20Known ? 1 : 0)
            << " steadyCtx20=" << numeric.steadyInput7A60.ctx20Value
            << " steadyWriteCtx10Current="
            << (numeric.steadyInput7A60.writeCtx10CurrentTick ? 1 : 0)
            << " steadyWriteCtx10Replay="
            << (numeric.steadyInput7A60.writeCtx10ReplayTick ? 1 : 0)
            << " steadyCtx10ReplayTick=" << numeric.steadyInput7A60.ctx10ReplayTick
            << " srcEvtAvail=" << (numeric.acceptedInputSourceEvent.available ? 1 : 0)
            << " srcEvtQ=" << numeric.acceptedInputSourceEvent.queryFrame
            << " srcEvtTick96=" << numeric.acceptedInputSourceEvent.tick96
            << " srcEvtHeldMask=" << numeric.acceptedInputSourceEvent.heldMask
            << " srcEvtCtx18=" << numeric.acceptedInputSourceEvent.ctx18Value
            << " srcEvtWriteCtx10Current="
            << (numeric.acceptedInputSourceEvent.writeCtx10CurrentTick ? 1 : 0)
            << " srcEvtPostGate="
            << (numeric.acceptedInputSourceEvent.postAcceptedGateOpen ? 1 : 0)
            << " srcEvtPostChanged="
            << (numeric.acceptedInputSourceEvent.postAcceptedMaskChanged ? 1 : 0)
            << " srcEvtPostBackup="
            << (numeric.acceptedInputSourceEvent.postBackupCtx10 ? 1 : 0)
            << " probeClass20=" << (int)acceptedProbe.classToken
            << " selectorByte0=" << (int)acceptedProbe.selectorByte0
            << " selectorByte1=" << (int)acceptedProbe.selectorByte1
            << " halfWindow34=" << (int)acceptedCarrier.halfWindow34
            << " substate50=" << (int)acceptedCarrier.substate50
            << " replayMode52=" << (numeric.ctx52ReplayMode7A60 ? 1 : 0)
            << " phase384=" << acceptedCarrier.phase384
            << " recordSlot24=" << (int)acceptedCarrier.recordSlot24
            << " recordRem24=" << (int)acceptedCarrier.recordRemainder24
            << " slot48=" << (int)acceptedCarrier.timingTemplateSlot48
            << " writePage38=" << acceptedCarrier.writePageOrdinal38
            << " lastWriteAvailable=" << (lastWrite.available ? 1 : 0)
            << " lastWritePage38=" << lastWrite.writePageOrdinal38
            << " lastWriteSlot24=" << (int)lastWrite.recordSlot24
            << " lastWriteMask=" << lastWrite.acceptedMask
            << " lastWriteComp=" << lastWrite.pageCompanion
            << " count91810=" << scorer.acceptedCount91810
            << " recorded91812=" << scorer.recordedHitCount91812
            << " penalty91814=" << scorer.penaltySideCount91814
            << " mask91808=" << scorer.aggregateAcceptedMask91808
            << " comp91824=" << scorer.recordCompanion91824
            << " clearMask9180C=" << scorer.lastClearedAcceptedMask9180C
            << " selectorAvailable="
            << (acceptedProbe.selectorAvailable ? 1 : 0)
            << " timingTemplateState="
            << (int)acceptedProbe.timingTemplateState
            << " sourceCellGate="
            << (acceptedProbe.sourceCellGateActive ? 1 : 0)
            << " materialized=" << (acceptedProbe.materialized ? 1 : 0)
            << " carryReplayed=" << (acceptedProbe.carryReplayed ? 1 : 0)
            << " streamFlagKnown="
            << (acceptedCarrier.eventStreamFlagKnown ? 1 : 0)
            << " streamFlag="
            << (acceptedCarrier.eventStreamFlagActive ? 1 : 0)
            << " streamIdKnown="
            << (acceptedCarrier.eventStreamIdRawKnown ? 1 : 0)
            << " streamIdRaw=" << (int)acceptedCarrier.eventStreamIdRaw
            << " busyGateKnown="
            << (acceptedCarrier.busyGate24BF4Known ? 1 : 0)
            << " busyGate="
            << (acceptedCarrier.busyGate24BF4Active ? 1 : 0)
            << " acceptedGateKnown="
            << (acceptedCarrier.acceptedGateKnown ? 1 : 0)
            << " acceptedGate="
            << (acceptedCarrier.acceptedGateActive ? 1 : 0)
            << " acceptedTick96Known="
            << (acceptedCarrier.acceptedTick96Known ? 1 : 0)
            << " acceptedTick96=" << acceptedCarrier.acceptedTick96
            << " acceptedSplitId=" << (int)acceptedProbe.split
            << " acceptedSplit=" << acceptedSplitName
            << " clearGate=0 failGate=0 runnerFrame=" << g_dbg_stageFrame
            << " terminalValid=" << (terminalValid ? 1 : 0)
            << " terminalQuery="
            << (terminalValid ? (int)terminalLifecycle.queryFrame : -1)
            << " terminalClearGate="
            << (terminalValid && terminalLifecycle.clearGate ? 1 : 0)
            << " terminalFailGate="
            << (terminalValid && terminalLifecycle.failGate ? 1 : 0)
            << " terminalRunnerExitEventStreamFlagLastRunnerClearDeltaToHandoff="
            << queryDeltaOrMissing(terminalRunnerClearKnown,
                                   (int)terminalLifecycle.queryFrame,
                                   terminalRunnerClearQuery)
            << " terminalRunnerExitEventStreamFlagLastRunnerClearDeltaAfterLastReplayAppend="
            << queryDeltaOrMissing(terminalRunnerClearKnown &&
                                       numeric.rightRankLastReplayAppendKnown,
                                   terminalRunnerClearQuery,
                                   numeric.rightRankLastReplayAppendQueryFrame)
            << " terminalRunnerExitEventStreamFlagLastRunnerClearDeltaAfterLastAcceptedTailCall="
            << queryDeltaOrMissing(terminalRunnerClearKnown &&
                                       numeric.rightRankLastAcceptedTailCallKnown,
                                   terminalRunnerClearQuery,
                                   numeric.rightRankLastAcceptedTailCallQueryFrame)
            << " terminalRunnerExitEventStreamFlagLastRunnerClearDeltaBeforeLastAcceptedTailDecision="
            << queryDeltaOrMissing(
                   terminalRunnerClearKnown &&
                       numeric.rightRankLastAcceptedTailDecisionKnown,
                   numeric.rightRankLastAcceptedTailDecisionQueryFrame,
                   terminalRunnerClearQuery);
    }
    return out.str();
}

static std::string Stage1OverlayDescribeActiveHud(const PrGameContext& prCtx) {
    PrStage1OverlayData scratch;
    const PrStage1OverlayData* data = nullptr;
    PrStage1ResolvedTextEvent resolved;
    uint32_t queryFrame = 0;
    if (!TryResolveLatestStage1EventForDebug(prCtx,
                                             scratch,
                                             data,
                                             resolved,
                                             queryFrame) ||
        !data || !resolved.valid) {
        return "runtime=<none>";
    }

    std::ostringstream out;
    AppendResolvedTextEventSummary(out, resolved, true, queryFrame);
    return out.str();
}

static bool SelfTest_TimDecode() {
    TimImage sonyTim;
    if (!TimDecoder::Decode(tim_sony, sizeof(tim_sony), sonyTim)) {
        return false;
    }
    if (sonyTim.width == 0 || sonyTim.height == 0) {
        return false;
    }
    return true;
}

static bool SelfTest_IntLoad(const std::filesystem::path& intPath, int* outTim, int* outVab, int* outMem, size_t* outTotal) {
    if (outTim) *outTim = 0;
    if (outVab) *outVab = 0;
    if (outMem) *outMem = 0;
    if (outTotal) *outTotal = 0;

    IntArchive archive;
    if (!IntLoader::Load(intPath.u8string(), archive)) {
        return false;
    }
    if (outTotal) *outTotal = archive.entries.size();

    for (const auto& e : archive.entries) {
        if (e.type == IntBlockType::Tim) {
            if (outTim) (*outTim)++;
            continue;
        }
        if (e.type == IntBlockType::Vab) {
            if (outVab) (*outVab)++;
            continue;
        }
        if (e.type == IntBlockType::Mem) {
            if (outMem) (*outMem)++;
            continue;
        }
    }

    return !archive.entries.empty();
}

static int RunSelfTests(const std::filesystem::path& dataRoot) {
    bool ok = true;

    {
        const bool timOk = SelfTest_TimDecode();
        Log::Printf("SelfTest: TIM decode ok=%d", timOk ? 1 : 0);
        ok = ok && timOk;
    }

    {
        struct Item { const char* name; std::filesystem::path path; bool requireMem; };
        const Item items[] = {
            {"S1/COMPO01.INT", dataRoot / "S1" / "COMPO01.INT", true},
            {"S1/ZCOMPO.INT", dataRoot / "S1" / "ZCOMPO.INT", false},
        };

        for (const auto& it : items) {
            int tim = 0, vab = 0, mem = 0;
            size_t total = 0;
            const bool exists = std::filesystem::exists(it.path);
            const bool loaded = exists && SelfTest_IntLoad(it.path, &tim, &vab, &mem, &total);
            const bool memOk = (!it.requireMem) || (mem > 0);
            Log::Printf("SelfTest: INT %s exists=%d loaded=%d total=%llu tim=%d vab=%d mem=%d", it.name, exists ? 1 : 0, loaded ? 1 : 0, (unsigned long long)total, tim, vab, mem);
            ok = ok && loaded && memOk;
        }
    }

    Log::Printf("SelfTest: overall ok=%d", ok ? 1 : 0);
    return ok ? 0 : 1;
}

// Debug hotkeys for minimal runnable game
static bool g_f1Pressed = false;   // F1: STR skip (pad==256)
static bool g_f1SkipRemote = false;
static bool g_escExitPressed = false; // Esc held for ev4 exit (not immediate app exit)
static bool g_f5Pressed = false;   // F5: Stage clear
static bool g_f6Pressed = false;   // F6: Unlock next stage in ev=2 test flow
static bool g_f7Pressed = false;   // F7: Mark selectable stages as first-clear

// Keyboard to PSX pad mask mapping
static uint16_t g_keyboardPadMask = 0;
// PSX pad bit definitions
constexpr uint16_t KPAD_CROSS    = 0x0040;
constexpr uint16_t KPAD_CIRCLE   = 0x0020;
constexpr uint16_t KPAD_SQUARE   = 0x0080;
constexpr uint16_t KPAD_TRIANGLE = 0x0010;
constexpr uint16_t KPAD_RIGHT    = 0x2000;
constexpr uint16_t KPAD_DOWN     = 0x4000;
constexpr uint16_t KPAD_LEFT     = 0x8000;
constexpr uint16_t KPAD_UP       = 0x1000;
constexpr uint16_t KPAD_SELECT   = 0x0100;
constexpr uint16_t KPAD_START    = 0x0800;
static bool g_texViewEnabled = false;
static std::string g_playingTextureName;
static int g_playingTextureWidth = 0;
static int g_playingTextureHeight = 0;
static std::vector<std::string> g_playingTextureNames;
static int g_playingTextureIndex = -1;

static bool IsStrSkipPadRequested(const PrGameContext& ctx) {
    const PrPadState pad = PrPad::GetState(0);
    const uint16_t localCurrent =
        static_cast<uint16_t>(pad.pressed | pad.held);
    const bool localSkip =
        (localCurrent & ((uint16_t)PrPadButton::Select |
                         (uint16_t)PrPadButton::Start)) != 0u;
    const bool psxDebugSkip =
        (ctx.debugPadInput & (KPAD_SELECT | KPAD_START)) != 0u;
    return localSkip || psxDebugSkip;
}

static void RenderStage1PsxFrameDebugOverlay(PrGameContext& ctx) {
    if (!ctx.debugStage1ShowPsxFrame ||
        !ctx.renderer ||
        ctx.currentScene != PrSceneId::Scene1 ||
        !ctx.stageRunning) {
        return;
    }

    char text[32];
    std::snprintf(text, sizeof(text), "PSX %d", GetStageRunner().GetFrame());
    PrVText::DrawString(ctx, 9, 9, text, 0xC0000000u, 1.0f, false);
    PrVText::DrawString(ctx, 8, 8, text, 0xFFFFFFFFu, 1.0f, false);
}

static std::filesystem::path GetExecutableDir() {
    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return std::filesystem::current_path();
    return std::filesystem::path(path).parent_path();
}

static void EnableConsoleOutput() {
    if (GetConsoleWindow()) return;
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
    }
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
}

static bool IsValidDataRoot(const std::filesystem::path& root) {
    std::error_code ec;
    if (root.empty()) return false;
    const bool scene1CompoOk =
        std::filesystem::exists(root / "S1" / "COMPO01.INT", ec);
    ec.clear();
    const bool sceneTablesOk =
        std::filesystem::exists(root / "out" / "scene_tables.json", ec);
    return scene1CompoOk || sceneTablesOk;
}

static std::filesystem::path DetectDataRoot(const std::filesystem::path& exeDir) {
    std::filesystem::path p = exeDir;
    for (int i = 0; i < 6; i++) {
        if (IsValidDataRoot(p)) {
            return p;
        }
        if (!p.has_parent_path()) {
            break;
        }
        p = p.parent_path();
    }
    return exeDir;
}

static std::wstring MakeTimestampStem() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[64];
    swprintf(buf, 64, L"%04d%02d%02d_%02d%02d%02d_%03d",
             (int)st.wYear, (int)st.wMonth, (int)st.wDay,
             (int)st.wHour, (int)st.wMinute, (int)st.wSecond,
             (int)st.wMilliseconds);
    return std::wstring(buf);
}

static std::wstring Utf8ToWide(const std::string& s);
static std::wstring SanitizeFileStem(const std::wstring& s);

static std::filesystem::path MakeScreenshotFilePath() {
    std::filesystem::path dir = GetExecutableDir() / L"screenshots";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);

    std::wstring stem = MakeTimestampStem();
    std::filesystem::path path = dir / (stem + L".png");
    for (int i = 1; std::filesystem::exists(path, ec) && i < 1000; i++) {
        path = dir / (stem + L"_" + std::to_wstring(i) + L".png");
    }
    return path;
}

static std::filesystem::path MakeDebugShotFilePath(int id, const std::string& stemUtf8) {
    std::filesystem::path dir = GetExecutableDir() / L"screenshots";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);

    std::wstring stemW = SanitizeFileStem(Utf8ToWide(stemUtf8));
    wchar_t fileBuf[256];
    if (!stemW.empty()) {
        swprintf(fileBuf, 256, L"%ls__%04d.png", stemW.c_str(), id);
    } else {
        swprintf(fileBuf, 256, L"shot__%04d.png", id);
    }
    return dir / fileBuf;
}

static std::wstring SanitizeFileStem(const std::wstring& s) {
    std::wstring out;
    out.reserve(s.size());
    for (wchar_t c : s) {
        const bool ok = (c >= L'0' && c <= L'9') ||
                        (c >= L'a' && c <= L'z') ||
                        (c >= L'A' && c <= L'Z') ||
                        (c == L'_') || (c == L'-') || (c == L'+') || (c == L'.');
        out.push_back(ok ? c : L'_');
    }
    if (out.size() > 160) {
        out.resize(160);
    }
    return out;
}

static std::filesystem::path MakeFaceAutoShotFilePath(uint32_t frame, const std::string& tagUtf8) {
    std::filesystem::path dir = GetExecutableDir() / L"screenshots_face";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);

    std::wstring stem = MakeTimestampStem();
    std::wstring tagW = SanitizeFileStem(Utf8ToWide(tagUtf8));
    std::filesystem::path path = dir / (stem + L"_f" + std::to_wstring((unsigned)frame) + L"_" + tagW + L".png");
    for (int i = 1; std::filesystem::exists(path, ec) && i < 1000; i++) {
        path = dir / (stem + L"_f" + std::to_wstring((unsigned)frame) + L"_" + tagW + L"_" + std::to_wstring(i) + L".png");
    }
    return path;
}

static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";

    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    if (len <= 0) return L"";

    std::wstring w;
    w.resize((size_t)len);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), len);
    return w;
}

static int SceneIdToDisplayScene(PrSceneId id) {
    switch (id) {
        case PrSceneId::Scene0: return 0;
        case PrSceneId::Scene1: return 1;
        case PrSceneId::Scene2: return 2;
        case PrSceneId::Scene3: return 3;
        case PrSceneId::Scene5: return 5;
        case PrSceneId::Scene6: return 6;
        case PrSceneId::Scene7: return 7;
        case PrSceneId::Scene8: return 8;
        case PrSceneId::Scene9: return 9;
    }
    return 0;
}

static void UpdatePlayingWindowTitle(HWND hwnd, PrSceneId scene) {
    int displayScene = SceneIdToDisplayScene(scene);
    // Scene0's direct translation is the SS0 owner.  Keep the numeric scene
    // id intact for runtime/resource addressing, but expose the owner name in
    // the window title so a live run cannot be mistaken for the retired Win
    // S0 shell.  Other scenes retain their original S<N> labels.
    std::wstring sceneLabel;
    if (scene == PrSceneId::Scene0 &&
        PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
        sceneLabel = L"SS0";
    } else {
        sceneLabel = L"S" + std::to_wstring(displayScene);
    }
    std::wstring title = L"PaRappa - Playing " + sceneLabel;

    {
        std::wstringstream ss;
        ss.setf(std::ios::fixed);
        ss.precision(1);
        ss << g_lastFps;
        title += L" FPS " + ss.str();
    }

    if (!g_playingTextureName.empty() && !g_playingTextureNames.empty() && g_playingTextureIndex >= 0) {
        title += L" Tex "
            + std::to_wstring(g_playingTextureIndex + 1)
            + L"/" + std::to_wstring((int)g_playingTextureNames.size())
            + L" " + Utf8ToWide(g_playingTextureName)
            + L" (" + std::to_wstring(g_playingTextureWidth)
            + L"x" + std::to_wstring(g_playingTextureHeight)
            + L")";
    }

    SetWindowTextW(hwnd, title.c_str());
}

static std::filesystem::path WeakCanonicalDebugPath(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::path out = std::filesystem::weakly_canonical(path, ec);
    return ec ? path : out;
}

static std::filesystem::path ResolveStage1TexreplaceDirForDebug(const PrGameContext& ctx) {
    std::filesystem::path configured = ctx.stage1TextureReplacementDir;
    if (configured.empty()) {
        configured = "ex/image/texreplace";
    }
    if (configured.is_absolute()) {
        return WeakCanonicalDebugPath(configured);
    }

    std::vector<std::filesystem::path> bases;
    if (!ctx.dataRoot.empty()) {
        bases.push_back(ctx.dataRoot);
        if (ctx.dataRoot.has_parent_path()) {
            bases.push_back(ctx.dataRoot.parent_path());
        }
    }
    bases.push_back(std::filesystem::current_path());
    if (std::filesystem::current_path().has_parent_path()) {
        bases.push_back(std::filesystem::current_path().parent_path());
    }

    for (const std::filesystem::path& base : bases) {
        std::error_code ec;
        std::filesystem::path candidate = WeakCanonicalDebugPath(base / configured);
        if (std::filesystem::is_directory(candidate, ec)) {
            return candidate;
        }
    }
    return WeakCanonicalDebugPath(bases.empty() ? configured : bases.front() / configured);
}

static std::string TrimAscii(std::string value) {
    while (!value.empty() && std::isspace((unsigned char)value.front())) {
        value.erase(value.begin());
    }
    while (!value.empty() && std::isspace((unsigned char)value.back())) {
        value.pop_back();
    }
    return value;
}

static std::string ToLowerAscii(std::string value) {
    std::transform(value.begin(),
                   value.end(),
                   value.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return value;
}

static std::string TabSafe(std::string value) {
    for (char& ch : value) {
        if (ch == '\t' || ch == '\r' || ch == '\n') {
            ch = ' ';
        }
    }
    return value;
}

static std::string SanitizeFileStem(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (unsigned char ch : value) {
        const bool ok =
            (ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            ch == '_' || ch == '-' || ch == '.' || ch == '~';
        out.push_back(ok ? (char)ch : '_');
    }
    while (!out.empty() && (out.back() == '.' || out.back() == ' ')) {
        out.pop_back();
    }
    if (out.empty()) {
        out = "texture";
    }
    if (out.size() > 120u) {
        out.resize(120u);
    }
    return out;
}

static std::string DumpLoadedTexturesForDebug(PrGameContext& ctx,
                                             const std::string& args) {
    if (!ctx.resources || !ctx.renderer) {
        return "ERR: resources/renderer not ready";
    }

    const std::string filter = ToLowerAscii(TrimAscii(args));
    const std::filesystem::path texreplace =
        ResolveStage1TexreplaceDirForDebug(ctx);
    const std::filesystem::path dumpRoot =
        texreplace / "stage1_resource_dump";
    const std::filesystem::path sourceRoot = dumpRoot / "sources";
    std::error_code ec;
    std::filesystem::create_directories(sourceRoot, ec);
    if (ec) {
        return std::string("ERR: failed to create ") + sourceRoot.u8string();
    }

    const std::filesystem::path manifestPath = dumpRoot / "manifest.tsv";
    std::ofstream manifest(manifestPath, std::ios::binary | std::ios::trunc);
    if (!manifest.is_open()) {
        return std::string("ERR: failed to write ") + manifestPath.u8string();
    }
    manifest << "# stage1_texture_resource_dump_v1\n";
    manifest << "# id\tresource_key\toriginal_name\twidth\theight\trel_path\tdetail\n";

    const std::vector<std::string> names = ctx.resources->GetTextureNames();
    int dumped = 0;
    int skipped = 0;
    for (int i = 0; i < (int)names.size(); ++i) {
        const std::string& key = names[(size_t)i];
        if (!filter.empty() && ToLowerAscii(key).find(filter) == std::string::npos) {
            continue;
        }
        TextureResource* res = ctx.resources->GetTexture(key);
        if (!res || res->tim.rgba.empty() || res->tim.width == 0u ||
            res->tim.height == 0u) {
            ++skipped;
            continue;
        }

        char idBuf[32];
        std::snprintf(idBuf, sizeof(idBuf), "tex_%04d", dumped + 1);
        char whBuf[48];
        std::snprintf(whBuf,
                      sizeof(whBuf),
                      "_%ux%u",
                      (unsigned)res->tim.width,
                      (unsigned)res->tim.height);
        std::string stem = std::string(idBuf) + "_" + SanitizeFileStem(key);
        if (!res->name.empty() && ToLowerAscii(res->name) != ToLowerAscii(key)) {
            stem += "_" + SanitizeFileStem(res->name);
        }
        stem += whBuf;
        const std::filesystem::path outPath = sourceRoot / (stem + ".png");
        const bool ok = ctx.renderer->SaveRgbaPng(outPath.wstring(),
                                                  res->tim.rgba.data(),
                                                  (int)res->tim.width,
                                                  (int)res->tim.height);
        if (!ok) {
            ++skipped;
            continue;
        }

        const std::filesystem::path rel =
            std::filesystem::relative(outPath, texreplace, ec);
        const std::string relText =
            (ec ? outPath.filename() : rel).generic_u8string();
        manifest << idBuf << '\t'
                 << TabSafe(key) << '\t'
                 << TabSafe(res->name) << '\t'
                 << (unsigned)res->tim.width << '\t'
                 << (unsigned)res->tim.height << '\t'
                 << relText << '\t'
                 << "bpp=" << (unsigned)res->tim.bpp
                 << " org=" << (int)res->tim.orgX << "," << (int)res->tim.orgY
                 << " clut=" << (int)res->tim.clutX << "," << (int)res->tim.clutY
                 << '\n';
        ++dumped;
    }
    manifest.close();
    Log::Printf("Texture dump: dumped=%d skipped=%d out=%s",
                dumped,
                skipped,
                dumpRoot.u8string().c_str());

    std::ostringstream out;
    out << "OK: texdump dumped=" << dumped
        << " skipped=" << skipped
        << " textures=" << names.size()
        << " out=" << dumpRoot.u8string();
    return out.str();
}

static LONG WINAPI UnhandledExceptionFilterProc(EXCEPTION_POINTERS* ep) {
    if (ep && ep->ExceptionRecord) {
        Log::Printf("UNHANDLED EXCEPTION code=0x%08X addr=%p state=%d frame=%d",
                    (unsigned)ep->ExceptionRecord->ExceptionCode,
                    ep->ExceptionRecord->ExceptionAddress,
                    (int)g_state,
                    g_frameNum);
    } else {
        Log::Printf("UNHANDLED EXCEPTION (no record) state=%d frame=%d", (int)g_state, g_frameNum);
    }
    Log::Shutdown();
    return EXCEPTION_CONTINUE_SEARCH;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            g_running = false;
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN: {
            if (GetForegroundWindow() != hwnd) break;

            // Menu navigation
            if (wParam == VK_UP || wParam == 'W') { g_upPressed = true; g_keyboardPadMask |= KPAD_UP; }
            if (wParam == VK_DOWN || wParam == 'S') { g_downPressed = true; g_keyboardPadMask |= KPAD_DOWN; }
            if (wParam == VK_LEFT || wParam == 'A') { g_keyboardPadMask |= KPAD_LEFT; }
            if (wParam == VK_RIGHT || wParam == 'D') { g_keyboardPadMask |= KPAD_RIGHT; }
            if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == 'Z') { g_confirmPressed = true; g_keyboardPadMask |= KPAD_CROSS; }
            if (wParam == 'X') { g_keyboardPadMask |= KPAD_CIRCLE; }
            if (wParam == 'C') { g_keyboardPadMask |= KPAD_SQUARE; }
            if (wParam == 'V') { g_keyboardPadMask |= KPAD_TRIANGLE; }

            // F12 for screenshot
            if (wParam == VK_F12) g_screenshotRequested = true;

            if (wParam == VK_PRIOR) g_prevTexPressed = true;
            if (wParam == VK_NEXT) g_nextTexPressed = true;

            if (wParam == 'N') {
                if ((lParam & 0x40000000) == 0) {
                    g_nextScenePressed = true;
                }
            }

            if (wParam == 'G') {
                if ((lParam & 0x40000000) == 0) {
                    g_genericSwitchPressed = true;
                }
            }

            int ev = -1;
            if (wParam >= '0' && wParam <= '8') {
                ev = (int)(wParam - '0');
            } else if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD8) {
                ev = (int)(wParam - VK_NUMPAD0);
            }
            if (ev >= 0) {
                if ((lParam & 0x40000000) == 0) {
                    g_genericEvent = ev;
                    g_genericSwitchPressed = true;
                    Log::Printf("debugGenericEvent=%d", g_genericEvent);
                }
            }

            if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
                if ((lParam & 0x40000000) == 0) {
                    g_genericEventArg++;
                    Log::Printf("debugGenericEventArg=%d", g_genericEventArg);
                }
            }

            if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
                if ((lParam & 0x40000000) == 0) {
                    g_genericEventArg--;
                    Log::Printf("debugGenericEventArg=%d", g_genericEventArg);
                }
            }

            // F1: STR skip trigger
            if (wParam == VK_F1) {
                if ((lParam & 0x40000000) == 0) {
                    g_f1Pressed = true;
                    Log::Printf("Debug: F1 pressed (STR skip)");
                }
            }

            // F5: Stage clear trigger
            if (wParam == VK_F5) {
                if ((lParam & 0x40000000) == 0) {
                    g_f5Pressed = true;
                    Log::Printf("Debug: F5 pressed (Stage clear)");
                }
            }

            if (wParam == VK_F6) {
                if ((lParam & 0x40000000) == 0) {
                    g_f6Pressed = true;
                    Log::Printf("Debug: F6 pressed (Unlock next stage)");
                }
            }

            if (wParam == VK_F7) {
                if ((lParam & 0x40000000) == 0) {
                    g_f7Pressed = true;
                    Log::Printf("Debug: F7 pressed (First-clear selectable stages)");
                }
            }

            // ESC: In Playing state, trigger ev4 exit; otherwise exit app
            if (wParam == VK_ESCAPE) {
                if (g_state == GameState::Playing) {
                    if ((lParam & 0x40000000) == 0) {
                        g_escExitPressed = true;
                        Log::Printf("Debug: Esc pressed (ev4 exit)");
                    }
                } else {
                    g_running = false;
                }
            }
            return 0;
        }

        case WM_KEYUP:
            if (GetForegroundWindow() != hwnd) break;
            if (wParam == VK_UP || wParam == 'W') { g_upPressed = false; g_keyboardPadMask &= ~KPAD_UP; }
            if (wParam == VK_DOWN || wParam == 'S') { g_downPressed = false; g_keyboardPadMask &= ~KPAD_DOWN; }
            if (wParam == VK_LEFT || wParam == 'A') { g_keyboardPadMask &= ~KPAD_LEFT; }
            if (wParam == VK_RIGHT || wParam == 'D') { g_keyboardPadMask &= ~KPAD_RIGHT; }
            if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == 'Z') { g_confirmPressed = false; g_keyboardPadMask &= ~KPAD_CROSS; }
            if (wParam == 'X') { g_keyboardPadMask &= ~KPAD_CIRCLE; }
            if (wParam == 'C') { g_keyboardPadMask &= ~KPAD_SQUARE; }
            if (wParam == 'V') { g_keyboardPadMask &= ~KPAD_TRIANGLE; }
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static int g_dbg_stageTick96 = 0;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    Log::Init();
    Log::Printf("PaRappaWin start");
    SetUnhandledExceptionFilter(UnhandledExceptionFilterProc);

    try {
        const MMRESULT timerRes = timeBeginPeriod(1);
        Log::Printf("timeBeginPeriod(1)=%u", (unsigned)timerRes);

        const std::filesystem::path exeDir = GetExecutableDir();

        // Load or create config.ini from EXE directory
        static AppConfig g_config = AppConfig::LoadOrCreate(exeDir);
        Log::Printf("Config loaded from %s (master=%.2f bgm=%.2f sfx=%.2f subtitles=%d)",
                    AppConfig::GetConfigPath(exeDir).u8string().c_str(),
                    g_config.audio.master, g_config.audio.bgm, g_config.audio.sfx,
                    g_config.subtitlesEnabled ? 1 : 0);

        // Apply key bindings to input system
        PrPad::SetKeyBindings(g_config.keys);

        std::filesystem::path dataRoot = DetectDataRoot(exeDir);
        bool dataRootOverridden = false;
        bool selfTest = false;
        bool enableConsole = false;
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv) {
            for (int i = 1; i < argc; i++) {
                const wchar_t* a = argv[i];
                if (!a) continue;

                if (wcscmp(a, L"--console") == 0) {
                    enableConsole = true;
                    continue;
                }

                if (wcscmp(a, L"--selftest") == 0) {
                    selfTest = true;
                    continue;
                }

                if (wcscmp(a, L"--data-root") == 0 && i + 1 < argc) {
                    dataRoot = argv[i + 1];
                    dataRootOverridden = true;
                    i++;
                    continue;
                }

                if (wcsncmp(a, L"--data-root=", 12) == 0) {
                    dataRoot = a + 12;
                    dataRootOverridden = true;
                    continue;
                }

                if (wcscmp(a, L"--auto-exit-seconds") == 0 && i + 1 < argc) {
                    g_autoExitSeconds = _wtoi(argv[i + 1]);
                    if (g_autoExitSeconds < 0) g_autoExitSeconds = 0;
                    i++;
                    continue;
                }

                if (wcsncmp(a, L"--auto-exit-seconds=", 20) == 0) {
                    g_autoExitSeconds = _wtoi(a + 20);
                    if (g_autoExitSeconds < 0) g_autoExitSeconds = 0;
                    continue;
                }

                if (wcscmp(a, L"--render60") == 0) {
                    g_render60fps = true;
                    continue;
                }

                if (wcscmp(a, L"--face-auto-shots") == 0) {
                    g_faceAutoShots = true;
                    continue;
                }

                if (wcsncmp(a, L"--face-auto-shots=", 17) == 0) {
                    g_faceAutoShots = (_wtoi(a + 17) != 0);
                    continue;
                }

                if (wcscmp(a, L"--scn0-dance-end-delay-ticks") == 0 && i + 1 < argc) {
                    g_scn0DanceEndDelayTicks = _wtoi(argv[i + 1]);
                    if (g_scn0DanceEndDelayTicks < 0) g_scn0DanceEndDelayTicks = 0;
                    i++;
                    continue;
                }

                if (wcsncmp(a, L"--scn0-dance-end-delay-ticks=", 29) == 0) {
                    g_scn0DanceEndDelayTicks = _wtoi(a + 29);
                    if (g_scn0DanceEndDelayTicks < 0) g_scn0DanceEndDelayTicks = 0;
                    continue;
                }

                if (wcscmp(a, L"--ss0-probe-disable-f0-startup-publisher") == 0) {
                    g_ss0ProbeDisableF0StartupPublisher = true;
                    continue;
                }
            }
            LocalFree(argv);
        }

        PrSS0Scene0RuntimeDirect::
            SetWord800916F0DiscFullbootStartupPublisherDisabledForProbe(
                g_ss0ProbeDisableF0StartupPublisher);

        if (enableConsole) {
            EnableConsoleOutput();
            Log::Printf("Console enabled");
        }

        if (dataRootOverridden && !IsValidDataRoot(dataRoot)) {
            const std::filesystem::path fallback = DetectDataRoot(exeDir);
            if (IsValidDataRoot(fallback)) {
                Log::Printf("WARN: --data-root invalid, fallback to auto-detected: %s", fallback.u8string().c_str());
                dataRoot = fallback;
            }
        }

        // Apply render60fps from config (command-line overrides)
        if (!g_render60fps) {
            g_render60fps = g_config.render60fps;
        }
        Log::Printf("render60fps: %s", g_render60fps ? "ON" : "OFF");

        Log::Printf("data-root: %s", dataRoot.u8string().c_str());

        if (selfTest) {
            return RunSelfTests(dataRoot);
        }

        // Register window class
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = L"PaRappaWinClass";

        if (!RegisterClassExW(&wc)) {
            Log::Printf("ERROR: RegisterClassExW failed");
            MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK);
            return 1;
        }

    // Calculate window size for desired client area (from config)
    const int winW = g_config.windowWidth;
    const int winH = g_config.windowHeight;
    RECT rc = { 0, 0, winW, winH };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    // Create window
    HWND hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        Log::Printf("ERROR: CreateWindowExW failed");
        MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK);
        return 1;
    }

    // Initialize D3D11
    if (!g_renderer.Initialize(hwnd, winW, winH)) {
        Log::Printf("ERROR: Failed to initialize D3D11");
        MessageBoxW(nullptr, L"Failed to initialize D3D11", L"Error", MB_OK);
        return 1;
    }
    Log::Printf("D3D11 initialized (%dx%d)", winW, winH);

    // Initialize resource manager
    g_resources.SetRenderer(&g_renderer);

    PrGameContext prCtx;
    PrSceneTable prSceneTable;
    bool sceneTableOk = PrMain::InitSceneTable(prSceneTable, dataRoot);
    Log::Printf("InitSceneTable ok=%d", sceneTableOk ? 1 : 0);
    prCtx.renderer = &g_renderer;
    prCtx.resources = &g_resources;
    prCtx.strPlayer = &g_strPlayer;
    prCtx.xa1Player = &g_xa1Player;
    prCtx.dataRoot = dataRoot;
    prCtx.debugFaceAutoShots = g_faceAutoShots;
    prCtx.scn0DanceEndDelayTicks = g_scn0DanceEndDelayTicks;
    prCtx.stage1ParappaRailAssist = g_config.stage1ParappaRailAssist;
    prCtx.stage1RailMode = g_config.stage1RailMode;
    prCtx.stage1RailParappa2Darken = g_config.stage1RailParappa2Darken;
    prCtx.stage1RailParappa2CoreAlign =
        g_config.stage1RailParappa2CoreAlign;
    prCtx.stage1RailParappa2PopFrames =
        g_config.stage1RailParappa2PopFrames;
    prCtx.stage1RailParappa2PopScale =
        g_config.stage1RailParappa2PopScale;
    prCtx.stage1RailParappa2FlipFrames =
        g_config.stage1RailParappa2FlipFrames;
    prCtx.stage1RailParappa2GlowFadeFrames =
        g_config.stage1RailParappa2GlowFadeFrames;
    prCtx.stage1RailParappa2GlowAlpha =
        g_config.stage1RailParappa2GlowAlpha;
    prCtx.stage1RailParappa2GlowScale =
        g_config.stage1RailParappa2GlowScale;
    prCtx.stage1RailParappa2LeadSlots =
        g_config.stage1RailParappa2LeadSlots;
    prCtx.stage1RailParappa2TraceAlign =
        g_config.stage1RailParappa2TraceAlign;
    prCtx.stage1RailParappa2ScorerHud =
        g_config.stage1RailParappa2ScorerHud;
    prCtx.stage1RailParappa2CreativePrompt =
        g_config.stage1RailParappa2CreativePrompt;
    prCtx.stage1RailParappa2CreativePromptLanguage =
        g_config.stage1RailParappa2CreativePromptLanguage;
    prCtx.stage1RestoreCeilingLights =
        g_config.stage1RestoreCeilingLights;
    prCtx.stage1HdGeometryCleanup = g_config.stage1HdGeometryCleanup;
    prCtx.stage1TextureReplacements =
        g_config.stage1TextureReplacements;
    prCtx.stage1TextureReplacementDir =
        g_config.stage1TextureReplacementDir;
    prCtx.stage1HdSubtitles = g_config.stage1HdSubtitles;
    prCtx.stage1HdSubtitleLanguage = g_config.stage1HdSubtitleLanguage;
    prCtx.stage1HdSubtitleFile = g_config.stage1HdSubtitleFile;
    prCtx.stage1HdSubtitleFont = g_config.stage1HdSubtitleFont;
    prCtx.stage1HdSubtitleFontSizePsx =
        g_config.stage1HdSubtitleFontSizePsx;
    prCtx.stage1HdSubtitleY = g_config.stage1HdSubtitleY;
    prCtx.stage1HdSubtitleMovieY = g_config.stage1HdSubtitleMovieY;
    prCtx.stage1HdSubtitleGameplayY = g_config.stage1HdSubtitleGameplayY;
    prCtx.stage1HdSubtitleWidth = g_config.stage1HdSubtitleWidth;
    prCtx.stage1HdSubtitleDrawBox = g_config.stage1HdSubtitleDrawBox;
    prCtx.stage1HdSubtitleFillColor = g_config.stage1HdSubtitleFillColor;
    prCtx.stage1HdSubtitleOutlineColor =
        g_config.stage1HdSubtitleOutlineColor;
    prCtx.stage1HdSubtitleShadowColor = g_config.stage1HdSubtitleShadowColor;
    prCtx.stage1HdSubtitleOutlinePsx = g_config.stage1HdSubtitleOutlinePsx;
    prCtx.stage1HdSubtitleShadowOffsetXPsx =
        g_config.stage1HdSubtitleShadowOffsetXPsx;
    prCtx.stage1HdSubtitleShadowOffsetYPsx =
        g_config.stage1HdSubtitleShadowOffsetYPsx;
    prCtx.debugStage1TextureReplacementTrace =
        g_config.debugStage1TextureReplacementTrace;
    prCtx.debugStage1ShowPsxFrame = g_config.debugStage1ShowPsxFrame;

    prCtx.subtitleFlag = g_config.subtitlesEnabled ? 1 : 0;

    // Initialize boot logo sequence
    if (!g_bootLogo.Initialize(&g_renderer)) {
        Log::Printf("ERROR: Failed to initialize boot logo");
        MessageBoxW(nullptr, L"Failed to initialize boot logo", L"Error", MB_OK);
        return 1;
    }
    Log::Printf("BootLogo initialized");

    // Initialize STR player
    if (!g_strPlayer.Initialize(&g_renderer)) {
        Log::Printf("WARN: StrPlayer init failed");
    } else {
        Log::Printf("StrPlayer initialized");
    }

    // Initialize UI overlay
    PrUiOverlay::Init(&g_renderer);

    // Initialize audio engine (single WASAPI output for all voices)
    if (!AudioEngine::Get().Initialize(44100, 2)) {
        Log::Printf("WARN: AudioEngine init failed, audio disabled");
    }

    // Initialize UI sound effects
    PrSfx::Init();

    Log::Printf("PrSfx: using direct fallback tones; external UI VAB disabled");

    // SCUS 80015D18 executes 80016C8C -> 80016B84 before either startup
    // logo callback.  Resolve and read/parse the original-disc COMMON and
    // ZCOMPO INT payloads now, before the first BootLogo::Update/Render.
    // Scene0 consumes these exact typed transactions later; it must not
    // silently re-read them after the logos.  The translated 80026FA4 reset
    // tail is committed by this call; TIM/VAB upload and SPU transfer remain
    // separately gated inside SS0.
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        !PrSS0Scene0RuntimeDirect::
            PrepareStartupDiscIntBeforeBootLogo80016B84(prCtx)) {
        Log::Printf(
            "ERROR: SS0 pre-logo 80016B84 original-disc INT preload failed");
        MessageBoxW(
            hwnd,
            L"SS0 pre-logo original-disc preload failed",
            L"PaRappaWin startup error",
            MB_OK | MB_ICONERROR);
        return 1;
    }


    // Initialize debug server for remote testing
    if (!DebugServer::Init(19790)) {
        Log::Printf("WARN: DebugServer init failed, remote testing disabled");
    } else {
        // 注册调试变量，使其可通过 get/set 命令访问
        DebugServer::RegisterVar("confirm", &g_confirmPressed);
        DebugServer::RegisterVar("up", &g_upPressed);
        DebugServer::RegisterVar("down", &g_downPressed);
        const bool ss0DirectCutover =
            PrSS0Scene0RuntimeDirect::RuntimeEnabled();
        DebugServer::SetLegacyPrEventStatusKeysEnabled(!ss0DirectCutover);
        DebugServer::SetLegacyGenericInputEnabled(!ss0DirectCutover);
        DebugServer::SetLegacyDebugSceneSwitchEnabled(!ss0DirectCutover);
        if (ss0DirectCutover) {
            Log::Printf(
                "DebugServer direct SS0: legacy PrEvent debug names and generic input disabled; use ss0Direct mirrors");
            Log::Printf(
                "DebugServer direct SS0: legacy debug scene-switch disabled");
            DebugServer::RegisterCommand("ss0cardseed", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: debug card seed rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0cardseed requires Scene0";
                }
                std::istringstream iss(args);
                int count = 1;
                int firstBlock = 0;
                if (!(iss >> count)) {
                    count = 1;
                }
                if (!(iss >> firstBlock)) {
                    firstBlock = 0;
                }
                if (!PrSS0Scene0RuntimeDirect::DebugSeedCardAuthGapEntries(
                        count,
                        firstBlock)) {
                    return "ERR: ss0cardseed requires direct LOAD/REPLAY card page";
                }
                return "OK: ss0cardseed";
            });
            DebugServer::RegisterCommand("ss0hiscoreentryfixture", [&](const std::string& args) -> std::string {
                (void)args;
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    return "ERR: ss0hiscoreentryfixture requires Scene0";
                }
                if (!PrSS0Scene0RuntimeDirect::
                        DebugArmHiScoreEvent6EntryFixture()) {
                    return "ERR: ss0hiscoreentryfixture requires direct main menu";
                }
                return "OK: ss0hiscoreentryfixture non-authority entry-reference-only";
            });
            DebugServer::RegisterCommand("ss0runtimecarddir", [&](const std::string& args) -> std::string {
                (void)args;
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: runtime directory carrier rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0runtimecarddir requires Scene0";
                }
                Log::Printf(
                    "SS0 direct runtime: runtime directory carrier rejected retired use ss0dirscanfacts");
                return "ERR: ss0runtimecarddir retired; use ss0dirscanfacts";
            });
            DebugServer::RegisterCommand("ss0cardbadindex", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: debug card invalid-index seed rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0cardbadindex requires Scene0";
                }
                std::istringstream iss(args);
                int blockIndex = -1;
                if (!(iss >> blockIndex)) {
                    blockIndex = -1;
                }
                if (!PrSS0Scene0RuntimeDirect::DebugSeedCardInvalidIndexEntry(
                        blockIndex)) {
                    return "ERR: ss0cardbadindex requires direct LOAD/REPLAY card page and invalid block";
                }
                return "OK: ss0cardbadindex";
            });
            DebugServer::RegisterCommand("ss0cardtypedpayload", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: debug state16 typed payload rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0cardtypedpayload requires Scene0";
                }
                std::istringstream iss(args);
                int byteSeed = 0xC1;
                if (!(iss >> byteSeed)) {
                    byteSeed = 0xC1;
                }
                if (!PrSS0Scene0RuntimeDirect::
                        DebugPublishState16TypedPayloadCarrier(byteSeed)) {
                    return "ERR: ss0cardtypedpayload requires selected LOAD/REPLAY card row";
                }
                return "OK: ss0cardtypedpayload";
            });
            PrSS0State16RuntimeEnvelopeImportDirect::
                State16RuntimeEnvelopeStaging ss0State16EnvelopeStaging{};
            DebugServer::RegisterCommand("ss0state16env", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: live-GDB state16 envelope rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0state16env requires Scene0";
                }
                PrSS0State16RuntimeEnvelopeImportDirect::
                    State16RuntimeTypedFactsEnvelope envelope{};
                std::string error;
                if (!PrSS0State16RuntimeEnvelopeImportDirect::
                        ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(
                            args,
                            envelope,
                            error)) {
                    Log::Printf(
                        "SS0 direct runtime: live-GDB state16 envelope rejected parse=%s",
                        error.c_str());
                    return "ERR: ss0state16env " + error;
                }
                if (!PrSS0Scene0RuntimeDirect::
                        QueueRuntimeState16CardReadTypedFactsOneShot(
                            envelope.facts)) {
                    return "ERR: ss0state16env requires selected LOAD/REPLAY live-GDB row";
                }
                Log::Printf(
                    "SS0 direct runtime: live-GDB state16 envelope queued block=%d rowCount=%d payloadSha256=%s",
                    envelope.facts.selectedBlockIndex,
                    envelope.facts.rowCount,
                    envelope.fullPayloadBytesSha256.c_str());
                return "OK: ss0state16env";
            });
            DebugServer::SetCommandArgsLogRedacted("ss0state16env", true);
            DebugServer::RegisterCommand("ss0state16envbegin", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: staged live-GDB state16 envelope begin rejected scene=%d",
                        (int)prCtx.currentScene);
                    PrSS0State16RuntimeEnvelopeImportDirect::
                        ClearStagedValidatedLiveGdbState16RuntimeEnvelope(
                            ss0State16EnvelopeStaging);
                    return "ERR: ss0state16envbegin requires Scene0";
                }
                std::string error;
                if (!PrSS0State16RuntimeEnvelopeImportDirect::
                        BeginStagedValidatedLiveGdbState16RuntimeEnvelopeArgs(
                            ss0State16EnvelopeStaging,
                            args,
                            error)) {
                    Log::Printf(
                        "SS0 direct runtime: staged live-GDB state16 envelope begin rejected parse=%s",
                        error.c_str());
                    return "ERR: ss0state16envbegin " + error;
                }
                Log::Printf(
                    "SS0 direct runtime: staged live-GDB state16 envelope begin accepted");
                return "OK: ss0state16envbegin";
            });
            DebugServer::SetCommandArgsLogRedacted("ss0state16envbegin", true);
            DebugServer::RegisterCommand("ss0state16envchunk", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: staged live-GDB state16 envelope chunk rejected scene=%d",
                        (int)prCtx.currentScene);
                    PrSS0State16RuntimeEnvelopeImportDirect::
                        ClearStagedValidatedLiveGdbState16RuntimeEnvelope(
                            ss0State16EnvelopeStaging);
                    return "ERR: ss0state16envchunk requires Scene0";
                }
                std::string error;
                if (!PrSS0State16RuntimeEnvelopeImportDirect::
                        AppendStagedValidatedLiveGdbState16RuntimeEnvelopeChunk(
                            ss0State16EnvelopeStaging,
                            args,
                            error)) {
                    Log::Printf(
                        "SS0 direct runtime: staged live-GDB state16 envelope chunk rejected parse=%s",
                        error.c_str());
                    return "ERR: ss0state16envchunk " + error;
                }
                return "OK: ss0state16envchunk";
            });
            DebugServer::SetCommandArgsLogRedacted("ss0state16envchunk", true);
            DebugServer::RegisterCommand("ss0state16envcommit", [&](const std::string& args) -> std::string {
                (void)args;
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: staged live-GDB state16 envelope commit rejected scene=%d",
                        (int)prCtx.currentScene);
                    PrSS0State16RuntimeEnvelopeImportDirect::
                        ClearStagedValidatedLiveGdbState16RuntimeEnvelope(
                            ss0State16EnvelopeStaging);
                    return "ERR: ss0state16envcommit requires Scene0";
                }
                PrSS0State16RuntimeEnvelopeImportDirect::
                    State16RuntimeTypedFactsEnvelope envelope{};
                std::string error;
                if (!PrSS0State16RuntimeEnvelopeImportDirect::
                        CommitStagedValidatedLiveGdbState16RuntimeEnvelope(
                            ss0State16EnvelopeStaging,
                            envelope,
                            error)) {
                    Log::Printf(
                        "SS0 direct runtime: staged live-GDB state16 envelope commit rejected parse=%s",
                        error.c_str());
                    return "ERR: ss0state16envcommit " + error;
                }
                if (!PrSS0Scene0RuntimeDirect::
                        QueueRuntimeState16CardReadTypedFactsOneShot(
                            envelope.facts)) {
                    return "ERR: ss0state16envcommit requires selected LOAD/REPLAY live-GDB row";
                }
                Log::Printf(
                    "SS0 direct runtime: staged live-GDB state16 envelope queued block=%d rowCount=%d payloadSha256=%s",
                    envelope.facts.selectedBlockIndex,
                    envelope.facts.rowCount,
                    envelope.fullPayloadBytesSha256.c_str());
                return "OK: ss0state16envcommit";
            });
            DebugServer::RegisterCommand("ss0state16envcancel", [&](const std::string& args) -> std::string {
                (void)args;
                PrSS0State16RuntimeEnvelopeImportDirect::
                    ClearStagedValidatedLiveGdbState16RuntimeEnvelope(
                        ss0State16EnvelopeStaging);
                Log::Printf(
                    "SS0 direct runtime: staged live-GDB state16 envelope canceled");
                return "OK: ss0state16envcancel";
            });
            DebugServer::RegisterCommand("ss0replayrawsel", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: debug replay raw selector rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0replayrawsel requires Scene0";
                }
                std::istringstream iss(args);
                int savedSlot = 6;
                int blockIndex = 4;
                if (!(iss >> savedSlot)) {
                    savedSlot = 6;
                }
                if (!(iss >> blockIndex)) {
                    blockIndex = 4;
                }
                if (!PrSS0Scene0RuntimeDirect::DebugSeedReplayRawSelector(
                        savedSlot,
                        blockIndex)) {
                    return "ERR: ss0replayrawsel requires direct REPLAY card page and savedSlot >= 0";
                }
                return "OK: ss0replayrawsel";
            });
            DebugServer::RegisterCommand("ss0attractlast", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: debug attract last-scene seed rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0attractlast requires Scene0";
                }
                std::istringstream iss(args);
                int scene = -1;
                if (!(iss >> scene)) {
                    return "ERR: usage: ss0attractlast <scene1-6>";
                }
                if (!PrSS0Scene0RuntimeDirect::DebugSeedAttractLastScene(scene)) {
                    return "ERR: ss0attractlast requires direct title START idle and scene 1..6";
                }
                return "OK: ss0attractlast";
            });
            DebugServer::RegisterCommand("ss0titlebadtarget", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: debug title invalid target rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0titlebadtarget requires Scene0";
                }
                std::istringstream iss(args);
                int targetScene = 7;
                if (!(iss >> targetScene)) {
                    targetScene = 7;
                }
                if (!PrSS0Scene0RuntimeDirect::DebugSeedTitleExitInvalidTarget(
                        targetScene)) {
                    return "ERR: ss0titlebadtarget requires direct title START idle and invalid scene target";
                }
                return "OK: ss0titlebadtarget";
            });
            DebugServer::RegisterCommand("ss0legacytrans", [&](const std::string& args) -> std::string {
                (void)args;
                Log::Printf(
                    "DebugServer direct SS0: legacy transition fixture disabled");
                return "ERR: ss0legacytrans disabled in direct SS0";
            });
            DebugServer::RegisterCommand("ss0dirscanfacts", [&](const std::string& args) -> std::string {
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: runtime directory scan facts rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0dirscanfacts requires Scene0";
                }

                std::unordered_map<std::string, std::string> kv;
                std::istringstream iss(args);
                std::string token;
                while (iss >> token) {
                    const std::size_t eq = token.find('=');
                    if (eq == std::string::npos || eq == 0u ||
                        eq + 1u >= token.size()) {
                        return "ERR: ss0dirscanfacts bad key=value token";
                    }
                    if (!kv.emplace(token.substr(0, eq),
                                    token.substr(eq + 1u))
                             .second) {
                        return "ERR: ss0dirscanfacts duplicate key";
                    }
                }

                auto get = [&](const char* key) -> const std::string* {
                    const auto it = kv.find(key);
                    return it == kv.end() ? nullptr : &it->second;
                };
                auto getInt = [&](const char* key, int& out) -> bool {
                    const std::string* text = get(key);
                    if (text == nullptr) {
                        return false;
                    }
                    char* end = nullptr;
                    const long value = std::strtol(text->c_str(), &end, 0);
                    if (end == nullptr || *end != '\0') {
                        return false;
                    }
                    out = static_cast<int>(value);
                    return true;
                };
                const std::string* source = get("source");
                if (source == nullptr ||
                    *source !=
                        "duck_gdb_load_replay_directory_scan_capture.py") {
                    return "ERR: ss0dirscanfacts unexpected source";
                }
                const std::string* authority = get("authority");
                if (authority == nullptr ||
                    *authority !=
                        "validated_live_gdb_load_replay_directory_scan_facts") {
                    return "ERR: ss0dirscanfacts unexpected authority";
                }
                const std::string* modeText = get("mode");
                if (modeText == nullptr) {
                    return "ERR: ss0dirscanfacts missing mode";
                }

                PrSS0CardMemcardHandoffDirect::
                    LoadReplayDirectoryScanFacts80019D7C facts{};
                if (*modeText == "load") {
                    facts.mode = PrSS0CardMemcardHandoffDirect::
                        CardMode800191E4::Load;
                } else if (*modeText == "replay") {
                    facts.mode = PrSS0CardMemcardHandoffDirect::
                        CardMode800191E4::Replay;
                } else {
                    return "ERR: ss0dirscanfacts bad mode";
                }

                int value = 0;
                if (!getInt("known", value) || value != 1) {
                    return "ERR: ss0dirscanfacts requires known=1";
                }
                facts.known = true;
                if (!getInt("directoryRowsKnown80017B08", value) ||
                    value != 1) {
                    return "ERR: ss0dirscanfacts requires directoryRowsKnown80017B08=1";
                }
                facts.directoryRowsKnown80017B08 = true;
                if (!getInt("snapshotKnown80017B18", value) || value != 1) {
                    return "ERR: ss0dirscanfacts requires snapshotKnown80017B18=1";
                }
                facts.snapshotKnown80017B18 = true;
                if (!getInt("listRowsBuilt80019D7C", value) || value != 1) {
                    return "ERR: ss0dirscanfacts requires listRowsBuilt80019D7C=1";
                }
                facts.listRowsBuilt80019D7C = true;
                if (!getInt("entryCountKnown", value) || value != 1) {
                    return "ERR: ss0dirscanfacts requires entryCountKnown=1";
                }
                facts.entryCountKnown = true;
                if (!getInt("entryCount", facts.entryCount)) {
                    return "ERR: ss0dirscanfacts missing entryCount";
                }
                if (facts.entryCount < 0 ||
                    facts.entryCount >
                        static_cast<int>(
                            PrSS0CardMemcardHandoffDirect::
                                kSaveListRowCount80019458)) {
                    return "ERR: ss0dirscanfacts bad entryCount";
                }
                auto isAllowedDirectoryScanFactKey =
                    [&](const std::string& key) -> bool {
                    static const char* const kFixedKeys[] = {
                        "source",
                        "authority",
                        "mode",
                        "known",
                        "directoryRowsKnown80017B08",
                        "snapshotKnown80017B18",
                        "listRowsBuilt80019D7C",
                        "entryCountKnown",
                        "entryCount",
                    };
                    for (const char* fixedKey : kFixedKeys) {
                        if (key == fixedKey) {
                            return true;
                        }
                    }
                    for (int row = 0; row < facts.entryCount; ++row) {
                        const std::string rowPrefix =
                            "row" + std::to_string(row);
                        if (key == rowPrefix + "BlockKnown" ||
                            key == rowPrefix + "Block" ||
                            key == rowPrefix + "TitleKnown" ||
                            key == rowPrefix + "Title") {
                            return true;
                        }
                    }
                    return false;
                };
                for (const auto& item : kv) {
                    if (!isAllowedDirectoryScanFactKey(item.first)) {
                        return "ERR: ss0dirscanfacts unexpected key";
                    }
                }
                for (int row = 0; row < facts.entryCount; ++row) {
                    const std::string rowPrefix =
                        "row" + std::to_string(row);
                    const std::string blockKnownKey =
                        rowPrefix + "BlockKnown";
                    const std::string blockKey = rowPrefix + "Block";
                    const std::string titleKnownKey =
                        rowPrefix + "TitleKnown";
                    const std::string titleKey = rowPrefix + "Title";
                    if (!getInt(blockKnownKey.c_str(), value) ||
                        value != 1) {
                        return "ERR: ss0dirscanfacts missing row block known";
                    }
                    facts.rows[row].blockIndexKnown = true;
                    if (!getInt(blockKey.c_str(),
                                facts.rows[row].blockIndex)) {
                        return "ERR: ss0dirscanfacts missing row block";
                    }
                    if (!getInt(titleKnownKey.c_str(), value) ||
                        value != 1) {
                        return "ERR: ss0dirscanfacts missing row title known";
                    }
                    facts.rows[row].titleKnown = true;
                    const std::string* title = get(titleKey.c_str());
                    if (title == nullptr) {
                        return "ERR: ss0dirscanfacts missing row title";
                    }
                    std::snprintf(facts.rows[row].title,
                                  sizeof(facts.rows[row].title),
                                  "%s",
                                  title->c_str());
                }

                if (!PrSS0Scene0RuntimeDirect::
                        DebugPublishRuntimeDirectoryScanFacts(facts)) {
                    return "ERR: ss0dirscanfacts requires direct LOAD/REPLAY typed directory scan facts";
                }
                return "OK: ss0dirscanfacts";
            });
            DebugServer::RegisterCommand("ss0f0runtimefixture0", [&](const std::string& args) -> std::string {
                (void)args;
                if (prCtx.currentScene != PrSceneId::Scene0) {
                    Log::Printf(
                        "SS0 direct runtime: probe-only F0 runtime fixture rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0f0runtimefixture0 requires Scene0";
                }
                const auto snapshot =
                    PrSS0Scene0RuntimeDirect::GetDebugSnapshot();
                if (snapshot.phaseDebug != 100 ||
                    snapshot.directMenuIndex != 0) {
                    Log::Printf(
                        "SS0 direct runtime: probe-only F0 runtime fixture rejected phase=%d menu=%d",
                        snapshot.phaseDebug,
                        snapshot.directMenuIndex);
                    return "ERR: ss0f0runtimefixture0 requires direct title START idle";
                }
                PrSS0Scene0RuntimeDirect::Word800916F0Observation observation{};
                observation.source = PrSS0Scene0RuntimeDirect::
                    Word800916F0ObservationSource::RuntimePsxMemoryObservation;
                observation.psxAddress = 0x800916F0u;
                observation.byteSize = 2u;
                observation.valueKnown = true;
                observation.value = 0u;
                if (!PrSS0Scene0RuntimeDirect::
                        PublishInitialWord800916F0RuntimeObservation(
                            observation)) {
                    return "ERR: ss0f0runtimefixture0 rejected";
                }
                Log::Printf(
                    "SS0 direct runtime: probe-only F0 runtime fixture accepted value=0 nonAuthority=1");
                return "OK: ss0f0runtimefixture0 non-authority";
            });
            DebugServer::RegisterCommand("ss0f0runtimefixture0late", [&](const std::string& args) -> std::string {
                (void)args;
                if (prCtx.currentScene != PrSceneId::Scene1) {
                    Log::Printf(
                        "SS0 direct runtime: probe-only late F0 runtime fixture rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0f0runtimefixture0late requires Scene1";
                }
                if (g_dbg_stage1ClearTailMovie.clearTailStageStatusKnown != 1 ||
                    g_dbg_stage1ClearTailMovie.clearTailWaitingForWord800916F0 != 1 ||
                    g_dbg_stage1ClearTailMovie.clearTailPreWord800916F0ActionsApplied != 1) {
                    Log::Printf(
                        "SS0 direct runtime: probe-only late F0 runtime fixture rejected stageStatus=%d waiting=%d preActions=%d",
                        g_dbg_stage1ClearTailMovie.clearTailStageStatusKnown,
                        g_dbg_stage1ClearTailMovie.clearTailWaitingForWord800916F0,
                        g_dbg_stage1ClearTailMovie.clearTailPreWord800916F0ActionsApplied);
                    return "ERR: ss0f0runtimefixture0late requires clear-tail F0 wait";
                }
                PrSS0Scene0RuntimeDirect::Word800916F0Observation observation{};
                observation.source = PrSS0Scene0RuntimeDirect::
                    Word800916F0ObservationSource::RuntimePsxMemoryObservation;
                observation.psxAddress = 0x800916F0u;
                observation.byteSize = 2u;
                observation.valueKnown = true;
                observation.value = 0u;
                observation.frameKnown = true;
                observation.frame = static_cast<uint32_t>(g_frameNum);
                if (!PrSS0Scene0RuntimeDirect::
                        PublishInitialWord800916F0RuntimeObservation(
                            observation)) {
                    return "ERR: ss0f0runtimefixture0late rejected";
                }
                Log::Printf(
                    "SS0 direct runtime: probe-only late F0 runtime fixture accepted value=0 nonAuthority=1 clearTailWait=1 preActions=1");
                return "OK: ss0f0runtimefixture0late non-authority";
            });
            DebugServer::RegisterCommand("ss0saveuipoll4fixture", [&](const std::string& args) -> std::string {
                (void)args;
                if (prCtx.currentScene != PrSceneId::Scene1) {
                    Log::Printf(
                        "SS0 direct runtime: probe-only SaveUi poll4 fixture rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0saveuipoll4fixture requires Scene1";
                }
                if (g_dbg_stage1ClearTailMovie.saveUi19148Active != 1) {
                    Log::Printf(
                        "SS0 direct runtime: probe-only SaveUi poll4 fixture rejected active=%d",
                        g_dbg_stage1ClearTailMovie.saveUi19148Active);
                    return "ERR: ss0saveuipoll4fixture requires active SaveUi19148";
                }
                PrStage1LifecycleHostAdapter801C81EC::
                    ArmProbeOnlySaveUi19148FormatPoll4Feedback801C81EC();
                Log::Printf(
                    "SS0 direct runtime: probe-only SaveUi poll4 fixture armed nonAuthority=1");
                return "OK: ss0saveuipoll4fixture non-authority";
            });
            DebugServer::RegisterCommand("ss0saveuiformatterminalfixture", [&](const std::string& args) -> std::string {
                const bool poll3 = args == "poll3";
                const bool retry = args == "retry";
                if (!poll3 && !retry) {
                    return "ERR: ss0saveuiformatterminalfixture requires poll3|retry";
                }
                if (prCtx.currentScene != PrSceneId::Scene1) {
                    Log::Printf(
                        "SS0 direct runtime: probe-only SaveUi format terminal fixture rejected scene=%d",
                        (int)prCtx.currentScene);
                    return "ERR: ss0saveuiformatterminalfixture requires Scene1";
                }
                if (g_dbg_stage1ClearTailMovie.saveUi19148Active != 1) {
                    Log::Printf(
                        "SS0 direct runtime: probe-only SaveUi format terminal fixture rejected active=%d",
                        g_dbg_stage1ClearTailMovie.saveUi19148Active);
                    return "ERR: ss0saveuiformatterminalfixture requires active SaveUi19148";
                }
                PrStage1LifecycleHostAdapter801C81EC::
                    ArmProbeOnlySaveUi19148FormatTerminalFeedback801C81EC(
                        poll3 ? 3 : 2);
                Log::Printf(
                    "SS0 direct runtime: probe-only SaveUi format terminal fixture armed mode=%s nonAuthority=1",
                    poll3 ? "poll3" : "retry");
                return "OK: ss0saveuiformatterminalfixture non-authority";
            });
            DebugServer::RegisterCommand("ss0debugf6", [&](const std::string& args) -> std::string {
                (void)args;
                g_f6Pressed = true;
                Log::Printf(
                    "DebugServer direct SS0: ss0debugf6 requested");
                return "OK: ss0debugf6";
            });
            DebugServer::RegisterCommand("ss0debugf7", [&](const std::string& args) -> std::string {
                (void)args;
                g_f7Pressed = true;
                Log::Printf(
                    "DebugServer direct SS0: ss0debugf7 requested");
                return "OK: ss0debugf7";
            });
        } else {
            DebugServer::RegisterVar("genericEvent", &g_genericEvent);
            DebugServer::RegisterVar("genericEventArg", &g_genericEventArg);
            DebugServer::RegisterVar("genericSwitch", &g_genericSwitchPressed);
            DebugServer::RegisterVar("ev6Done", PrEvent::GetEv6DoneFlagPtr());
            DebugServer::RegisterVar("dispTimeout",
                                     PrEvent::GetDispTimeoutPtr());
            DebugServer::RegisterVar("dispFrameCountdown",
                                     PrEvent::GetDispFrameCountdownPtr());
            DebugServer::RegisterVar("dispLastInput",
                                     PrEvent::GetDispLastInputPtr());
            DebugServer::RegisterVar("dispMenuIndex",
                                     PrEvent::GetDispMenuIndexPtr());
            DebugServer::RegisterVar("dispEventId",
                                     PrEvent::GetDispEventIdPtr());
            if (PrEvent::GetEvMemCardCtxPtr()) {
                DebugServer::RegisterVar(
                    "memcardItemCount",
                    &PrEvent::GetEvMemCardCtxPtr()->itemCount);
                DebugServer::RegisterVar(
                    "memcardSelected",
                    &PrEvent::GetEvMemCardCtxPtr()->selected);
                DebugServer::RegisterVar(
                    "memcardExitFrameOn",
                    &PrEvent::GetEvMemCardCtxPtr()->exitFrameOn);
                DebugServer::RegisterVar(
                    "memcardExitTextOn",
                    &PrEvent::GetEvMemCardCtxPtr()->exitTextOn);
                DebugServer::RegisterVar(
                    "memcardIoMessage",
                    &PrEvent::GetEvMemCardCtxPtr()->ioMessage);
            }
        }
        DebugServer::RegisterVar("texView", &g_texViewEnabled);
        DebugServer::RegisterVar("showBgTex", &prCtx.debugShowBgTexture);
        DebugServer::RegisterVar("bgTexIndex", &prCtx.debugBgTexIndex);
        if (!ss0DirectCutover) {
            DebugServer::RegisterVar("scn0Mode", &prCtx.debugScn0Mode);
            DebugServer::RegisterVar("scn0BgIndex", &prCtx.debugScn0BgIndex);
            DebugServer::RegisterVar("scn0UiBase", &prCtx.debugScn0UiBaseIndex);
            DebugServer::RegisterVar("scn0HiliteCursor", &prCtx.scn0HiliteCursor);
        }
        DebugServer::RegisterReadOnlyVar("appState", &g_dbg_appState);
        DebugServer::RegisterReadOnlyVar("xaPlaying", &g_dbg_xaPlaying);
        DebugServer::RegisterReadOnlyVar("xaPlayedMs", &g_dbg_xaPlayedMs);
        DebugServer::RegisterReadOnlyVar("stageFrame", &g_dbg_stageFrame);
        DebugServer::RegisterReadOnlyVar("stageTick96", &g_dbg_stageTick96);
        DebugServer::RegisterReadOnlyVar("strPlaying", &g_dbg_strPlaying);
        DebugServer::RegisterReadOnlyVar("strFrame", &g_dbg_strFrame);
        DebugServer::RegisterReadOnlyVar("subtitleFlag", &g_dbg_subtitleFlag);
        DebugServer::RegisterReadOnlyVar("scn0Phase", &g_dbg_scn0Phase);
        DebugServer::RegisterReadOnlyVar("ss0PhaseRaw", &g_dbg_ss0PhaseRaw);
        DebugServer::RegisterReadOnlyVar("ss0PhaseDebug", &g_dbg_ss0PhaseDebug);
        DebugServer::RegisterReadOnlyVar("ss0TransitionReturnPhaseRaw", &g_dbg_ss0TransitionReturnPhaseRaw);
        DebugServer::RegisterReadOnlyVar("ss0DirectDispState", &g_dbg_ss0DirectDispState);
        DebugServer::RegisterReadOnlyVar("ss0DirectDispEventId", &g_dbg_ss0DirectDispEventId);
        DebugServer::RegisterReadOnlyVar("ss0DirectMenuIndex", &g_dbg_ss0DirectMenuIndex);
        DebugServer::RegisterReadOnlyVar("ss0DirectMainMenuRecordsMode", &g_dbg_ss0DirectMainMenuRecordsMode);
        DebugServer::RegisterReadOnlyVar("ss0DirectCardEntryCount", &g_dbg_ss0DirectCardEntryCount);
        DebugServer::RegisterReadOnlyVar("ss0DirectCardSelectedBlock", &g_dbg_ss0DirectCardSelectedBlock);
        DebugServer::RegisterReadOnlyVar("ss0DirectStageSelectEnabledMask", &g_dbg_ss0DirectStageSelectEnabledMask);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsLanguage", &g_dbg_ss0DirectOptionsLanguage);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsSubtitle", &g_dbg_ss0DirectOptionsSubtitle);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsPreLoopReleasePending", &g_dbg_ss0DirectOptionsPreLoopReleasePending);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsCooldown", &g_dbg_ss0DirectOptionsCooldown);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsTimeoutRemaining", &g_dbg_ss0DirectOptionsTimeoutRemaining);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsInitialInputPending", &g_dbg_ss0DirectOptionsInitialInputPending);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsTailActive", &g_dbg_ss0DirectOptionsTailActive);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsTailFramesRemaining", &g_dbg_ss0DirectOptionsTailFramesRemaining);
        DebugServer::RegisterReadOnlyVar("ss0DirectOptionsTailResult", &g_dbg_ss0DirectOptionsTailResult);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticePhase", &g_dbg_ss0DirectPracticePhase);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeRound", &g_dbg_ss0DirectPracticeRound);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeRoundFrame", &g_dbg_ss0DirectPracticeRoundFrame);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticePadStopFrames", &g_dbg_ss0DirectPracticePadStopFrames);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticePadStopKind", &g_dbg_ss0DirectPracticePadStopKind);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeExitConfirmFrames", &g_dbg_ss0DirectPracticeExitConfirmFrames);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeScore", &g_dbg_ss0DirectPracticeScore);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeHits", &g_dbg_ss0DirectPracticeHits);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeMisses", &g_dbg_ss0DirectPracticeMisses);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeLastJudge", &g_dbg_ss0DirectPracticeLastJudge);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeLastDelta", &g_dbg_ss0DirectPracticeLastDelta);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeJudgeFlash", &g_dbg_ss0DirectPracticeJudgeFlash);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeSeqCurA", &g_dbg_ss0DirectPracticeSeqCurA);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeSeqCurB", &g_dbg_ss0DirectPracticeSeqCurB);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeSeqEnA", &g_dbg_ss0DirectPracticeSeqEnA);
        DebugServer::RegisterReadOnlyVar("ss0DirectPracticeSeqEnB", &g_dbg_ss0DirectPracticeSeqEnB);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreBlink", &g_dbg_ss0DirectHiScoreBlink);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreExitLabelState", &g_dbg_ss0DirectHiScoreExitLabelState);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScorePreLoopReleasePending", &g_dbg_ss0DirectHiScorePreLoopReleasePending);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreTailActive", &g_dbg_ss0DirectHiScoreTailActive);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreTailFramesRemaining", &g_dbg_ss0DirectHiScoreTailFramesRemaining);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreTailResult", &g_dbg_ss0DirectHiScoreTailResult);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreOuterPadReleaseActive", &g_dbg_ss0DirectHiScoreOuterPadReleaseActive);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreOuterPadReleasePending", &g_dbg_ss0DirectHiScoreOuterPadReleasePending);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreEvent6TableKnown", &g_dbg_ss0DirectHiScoreEvent6TableKnown);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreEvent6TablePsxAddress", &g_dbg_ss0DirectHiScoreEvent6TablePsxAddress);
        DebugServer::RegisterReadOnlyVar("ss0DirectHiScoreEvent6TableByteCount", &g_dbg_ss0DirectHiScoreEvent6TableByteCount);
        DebugServer::RegisterReadOnlyVar("ss0DirectTitleExitWaitCounter", &g_dbg_ss0DirectTitleExitWaitCounter);
        DebugServer::RegisterReadOnlyVar("ss0DirectTitleExitTargetScene", &g_dbg_ss0DirectTitleExitTargetScene);
        DebugServer::RegisterReadOnlyVar("ss0DirectTitleExitPendingMenu", &g_dbg_ss0DirectTitleExitPendingMenu);
        DebugServer::RegisterReadOnlyVar("ss0DirectTitleLoopStateV8", &g_dbg_ss0DirectTitleLoopStateV8);
        DebugServer::RegisterReadOnlyVar("ss0LoadingHoldKind", &g_dbg_ss0LoadingHoldKind);
        DebugServer::RegisterReadOnlyVar("ss0LoadingScreenKind", &g_dbg_ss0LoadingScreenKind);
        DebugServer::RegisterReadOnlyVar("ss0LoadingHoldStartFrame", &g_dbg_ss0LoadingHoldStartFrame);
        DebugServer::RegisterReadOnlyVar("ss0LoadingHoldUntilFrame", &g_dbg_ss0LoadingHoldUntilFrame);
        DebugServer::RegisterReadOnlyVar("ss0LoadingPatternActive8001EF40", &g_dbg_ss0LoadingPatternActive8001EF40);
        DebugServer::RegisterReadOnlyVar("ss0LoadingPatternStyle8001EF40", &g_dbg_ss0LoadingPatternStyle8001EF40);
        DebugServer::RegisterReadOnlyVar("ss0LoadingPatternHighlightCount8001EF40", &g_dbg_ss0LoadingPatternHighlightCount8001EF40);
        DebugServer::RegisterReadOnlyVar("ss0LoadingPatternSubmittedHighlightCount8001EF40", &g_dbg_ss0LoadingPatternSubmittedHighlightCount8001EF40);
        DebugServer::RegisterReadOnlyVar("ss0LoadingPatternMutationSerial8001EF40", &g_dbg_ss0LoadingPatternMutationSerial8001EF40);
        DebugServer::RegisterReadOnlyVar("ss0LoadingPatternCallbackCount8001537C", &g_dbg_ss0LoadingPatternCallbackCount8001537C);
        DebugServer::RegisterReadOnlyVar("ss0LoadingPatternGridHashLow8001EF40", &g_dbg_ss0LoadingPatternGridHashLow8001EF40);
        DebugServer::RegisterReadOnlyVar("ss0LoadingPatternSubmittedFrame8001EF40", &g_dbg_ss0LoadingPatternSubmittedFrame8001EF40);
        DebugServer::RegisterReadOnlyVar("transActive", &g_dbg_transActive);
        DebugServer::RegisterReadOnlyVar("transPhase", &g_dbg_transPhase);
        DebugServer::RegisterReadOnlyVar("transTotalFrame", &g_dbg_transTotalFrame);
        DebugServer::RegisterReadOnlyVar("transTargetScene", &g_dbg_transTargetScene);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailHostBlockKind", &g_dbg_stage1ClearTailMovie.hostBlockKind);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailHostBlockActive", &g_dbg_stage1ClearTailMovie.hostBlockActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailHostBlockWaitingForPendingActions", &g_dbg_stage1ClearTailMovie.hostBlockWaitingForPendingActions);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailDeferredSceneResultKnown", &g_dbg_stage1ClearTailMovie.deferredSceneResultKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailDeferredSceneResult", &g_dbg_stage1ClearTailMovie.deferredSceneResult);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingActionCount", &g_dbg_stage1ClearTailMovie.pendingActionCount);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingActionKind", &g_dbg_stage1ClearTailMovie.pendingActionKind);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingActionPsxOrder", &g_dbg_stage1ClearTailMovie.pendingActionPsxOrder);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingActionPsxFunctionKnown", &g_dbg_stage1ClearTailMovie.pendingActionPsxFunctionKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingActionPsxFunction", &g_dbg_stage1ClearTailMovie.pendingActionPsxFunction);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingSaveStatus1635CKnown", &g_dbg_stage1ClearTailMovie.pendingSaveStatus1635CKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingSaveStatus1635CIndex", &g_dbg_stage1ClearTailMovie.pendingSaveStatus1635CIndex);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingSaveStatus1635CPsxOrder", &g_dbg_stage1ClearTailMovie.pendingSaveStatus1635CPsxOrder);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingSaveStatus1635CPsxFunctionKnown", &g_dbg_stage1ClearTailMovie.pendingSaveStatus1635CPsxFunctionKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPendingSaveStatus1635CPsxFunction", &g_dbg_stage1ClearTailMovie.pendingSaveStatus1635CPsxFunction);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailStageStatusKnown", &g_dbg_stage1ClearTailMovie.clearTailStageStatusKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailWaitingForWord800916F0", &g_dbg_stage1ClearTailMovie.clearTailWaitingForWord800916F0);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailPreWord800916F0ActionsApplied", &g_dbg_stage1ClearTailMovie.clearTailPreWord800916F0ActionsApplied);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailSaveUi19148StartPending", &g_dbg_stage1ClearTailMovie.saveUi19148StartPending);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailSaveUi19148Active", &g_dbg_stage1ClearTailMovie.saveUi19148Active);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailSaveUi19148LowerFeedbackRequestCount", &g_dbg_stage1ClearTailMovie.saveUi19148LowerFeedbackRequestCount);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailSaveUi19148Seed80092F10Known", &g_dbg_stage1ClearTailMovie.saveUi19148Seed80092F10Known);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailSaveUi19148Seed80092F10Address", &g_dbg_stage1ClearTailMovie.saveUi19148Seed80092F10Address);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectBeginAttempted", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectBeginAttempted);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectBegun", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectBegun);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectBeginSucceeded", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectBeginSucceeded);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectBeginFailed", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectBeginFailed);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectPumpStarted", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectPumpStarted);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingExternal", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingExternal);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectCompleted", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectCompleted);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectFailed", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectFailed);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectActionCount", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectActionCount);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectNextActionIndex", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectNextActionIndex);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectLastPumpActionCount", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectLastPumpActionCount);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingForFeedback", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingForFeedback);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingStepValid", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingStepValid);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingStepKind", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingStepKind);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingCategory", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingCategory);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingActionKind", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingActionKind);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingPsxOrder", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingPsxOrder);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingPsxFunction", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingPsxFunction);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingDirectFunction", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingDirectFunction);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingLowerFunction", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingLowerFunction);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingCdActionKind", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingCdActionKind);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingRecordIndex", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingRecordIndex);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590LoaderDirectWaitingRecordType", &g_dbg_stage1ClearTailMovie.bootstrap15590LoaderDirectWaitingRecordType);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerKeyKnown", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerKeyKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerRequestPending", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerRequestPending);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerStatus", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerStatus);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerPsxOrder", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerPsxOrder);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerStepKind", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerStepKind);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerCdActionKind", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerCdActionKind);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerRecordIndex", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerRecordIndex);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerRecordType", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerRecordType);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerAttemptIndex", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerAttemptIndex);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLowerFunction", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLowerFunction);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerFinalFunction", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerFinalFunction);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerPayloadBytesRequired", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerPayloadBytesRequired);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerSeekLbaKnown", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerSeekLbaKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerSeekLba", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerSeekLba);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerReadDstPtrKnown", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerReadDstPtrKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerReadDstPtr", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerReadDstPtr);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerReadSectorCountKnown", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerReadSectorCountKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerReadSectorCount", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerReadSectorCount);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerReadStartHalProgressAccepted", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerReadStartHalProgressAccepted);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerReadStartHalProgressReadS27Serial", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerReadStartHalProgressReadS27Serial);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerFinalReadyHalFactsRequired", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerFinalReadyHalFactsRequired);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerFinalReadyHalFactsReadS27Serial", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerFinalReadyHalFactsReadS27Serial);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerCdSyncLoopFactsRequired80037070", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerCdSyncLoopFactsRequired80037070);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerCdSyncLoopFunction80037070", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerCdSyncLoopFunction80037070);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerCdSyncLoopA0WaitModeKnown80037070", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerCdSyncLoopA0WaitModeKnown80037070);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerCdSyncLoopA0WaitMode80037070", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerCdSyncLoopA0WaitMode80037070);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastRejectKnown", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastRejectKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastRejectReason", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastRejectReason);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastRejectStatus", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastRejectStatus);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsAttempted", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsAttempted);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsActionKind", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsActionKind);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsReadStartHalFactsKnown", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsReadStartHalFactsKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsReadStartSetupProduced", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsReadStartSetupProduced);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsReadStartSetupIncomplete", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsReadStartSetupIncomplete);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsReadStartSetupFirstMissing", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsReadStartSetupFirstMissing);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsReadPumpProduced", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsReadPumpProduced);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsReadPumpIncomplete", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsReadPumpIncomplete);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsReadPumpFirstMissing", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsReadPumpFirstMissing);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsPayloadBytesKnown", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsPayloadBytesKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsBridgeProduced", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsBridgeProduced);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsBridgeIncomplete", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsBridgeIncomplete);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsLowerCdFactsBridged", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsLowerCdFactsBridged);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsXaCdSeamKnown", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsXaCdSeamKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsXaCdAccepted", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsXaCdAccepted);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailBootstrap15590CdLowerLastFactsRejectReason", &g_dbg_stage1ClearTailMovie.bootstrap15590CdLowerLastFactsRejectReason);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieBlockActive", &g_dbg_stage1ClearTailMovie.clearTailMovieBlockActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMoviePlayAndWaitPending", &g_dbg_stage1ClearTailMovie.clearTailMoviePlayAndWaitPending);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMoviePlayAndWaitResultKnown", &g_dbg_stage1ClearTailMovie.clearTailMoviePlayAndWaitResultKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMoviePlayAndWaitResult", &g_dbg_stage1ClearTailMovie.clearTailMoviePlayAndWaitResult);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieVisualActive", &g_dbg_stage1ClearTailMovie.clearTailMovieVisualActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieRuntimeCompletionPending", &g_dbg_stage1ClearTailMovie.moviePlayAndWaitCompletionPending);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieDrawableActive", &g_dbg_stage1ClearTailMovie.movieDrawableActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailTransition20110Mode2Pending", &g_dbg_stage1ClearTailMovie.transition20110Mode2Pending);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailTransitionSub80020110Mode2Active", &g_dbg_stage1ClearTailMovie.transitionSub80020110Mode2Active);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailTransitionSub800201ACActive", &g_dbg_stage1ClearTailMovie.transitionSub800201ACActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailTransitionSub800201ACCompleted", &g_dbg_stage1ClearTailMovie.transitionSub800201ACCompleted);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailTransitionSub800201ACPhase", &g_dbg_stage1ClearTailMovie.transitionSub800201ACPhase);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailTransitionSub800201ACGp196", &g_dbg_stage1ClearTailMovie.transitionSub800201ACGp196);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailTransitionSub800201ACTailFrames", &g_dbg_stage1ClearTailMovie.transitionSub800201ACTailFrames);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailTransitionSub800201ACTailIndex", &g_dbg_stage1ClearTailMovie.transitionSub800201ACTailIndex);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieStrFrameActive", &g_dbg_stage1ClearTailMovie.movieStrFrameActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOutroActive", &g_dbg_stage1ClearTailMovie.movieOutroActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOutroTailActive", &g_dbg_stage1ClearTailMovie.movieOutroTailActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieSkipPreludeActive", &g_dbg_stage1ClearTailMovie.movieSkipPreludeActive);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopPhase", &g_dbg_stage1ClearTailMovie.movieOuterLoopPhase);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopComplete", &g_dbg_stage1ClearTailMovie.movieOuterLoopComplete);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopExitReason", &g_dbg_stage1ClearTailMovie.movieOuterLoopExitReason);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopReturn", &g_dbg_stage1ClearTailMovie.movieOuterLoopReturn);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepKnown", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepPhaseBefore", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepPhaseBefore);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepPhaseAfter", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepPhaseAfter);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepActionCount", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepActionCount);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepMovieStepKnown", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepMovieStepKnown);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepMovieStepResult", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepMovieStepResult);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepA7F8Computed", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepA7F8Computed);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepA7F8Known", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepA7F8Known);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepA7F8Result", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepA7F8Result);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepGapInputMask", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepGapInputMask);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepGapDword9554", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepGapDword9554);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepGapGsWorkBase", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepGapGsWorkBase);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepGapByte493F4", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepGapByte493F4);
        DebugServer::RegisterReadOnlyVar("stage1ClearTailMovieOuterLoopLastStepGapSub8001A750", &g_dbg_stage1ClearTailMovie.movieOuterLoopLastStepGapSub8001A750);
        DebugServer::RegisterReadOnlyVar("word800916F0Known", &g_dbg_word800916F0Known);
        DebugServer::RegisterReadOnlyVar("word800916F0SoftwareKnown", &g_dbg_word800916F0SoftwareKnown);
        DebugServer::RegisterReadOnlyVar("word800916F0SoftwareValue", &g_dbg_word800916F0SoftwareValue);
        DebugServer::RegisterReadOnlyVar("word800916F0SoftwareStartupCommitCount", &g_dbg_word800916F0SoftwareStartupCommitCount);
        DebugServer::RegisterReadOnlyVar("word800916F0ObservationAcceptedCount", &g_dbg_word800916F0ObservationAcceptedCount);
        DebugServer::RegisterReadOnlyVar("word800916F0RuntimeObservationAcceptedCount", &g_dbg_word800916F0RuntimeObservationAcceptedCount);
        DebugServer::RegisterReadOnlyVar("word800916F0StartupObservationAcceptedCount", &g_dbg_word800916F0StartupObservationAcceptedCount);
        DebugServer::RegisterReadOnlyVar("word800916F0ObservationRejectedCount", &g_dbg_word800916F0ObservationRejectedCount);
        DebugServer::RegisterReadOnlyVar("word800916F0ObservationLastRejectReason", &g_dbg_word800916F0ObservationLastRejectReason);
        DebugServer::RegisterReadOnlyVar("word800916F0ObservationLastStartupSource", &g_dbg_word800916F0ObservationLastStartupSource);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressAttempted", &g_dbg_word800916F0ProviderIngressAttempted);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressReadable", &g_dbg_word800916F0ProviderIngressReadable);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressPublishAttempted", &g_dbg_word800916F0ProviderIngressPublishAttempted);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressPublished", &g_dbg_word800916F0ProviderIngressPublished);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressKnownAfter", &g_dbg_word800916F0ProviderIngressKnownAfter);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceMainGlobalAttempted", &g_dbg_word800916F0ProviderIngressSourceMainGlobalAttempted);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceMainGlobalReadable", &g_dbg_word800916F0ProviderIngressSourceMainGlobalReadable);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceMainGlobalSelfBootstrapBlocked", &g_dbg_word800916F0ProviderIngressSourceMainGlobalSelfBootstrapBlocked);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceLoaderHeapAttempted", &g_dbg_word800916F0ProviderIngressSourceLoaderHeapAttempted);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceLoaderHeapReadable", &g_dbg_word800916F0ProviderIngressSourceLoaderHeapReadable);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceKnownStateRelayAttempted", &g_dbg_word800916F0ProviderIngressSourceKnownStateRelayAttempted);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceKnownStateRelayReadable", &g_dbg_word800916F0ProviderIngressSourceKnownStateRelayReadable);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceCdMmioSnapshotAttempted", &g_dbg_word800916F0ProviderIngressSourceCdMmioSnapshotAttempted);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceCdMmioSnapshotReadable", &g_dbg_word800916F0ProviderIngressSourceCdMmioSnapshotReadable);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceExactCdAttempted", &g_dbg_word800916F0ProviderIngressSourceExactCdAttempted);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceExactCdReadable", &g_dbg_word800916F0ProviderIngressSourceExactCdReadable);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressAttemptCount", &g_dbg_word800916F0ProviderIngressAttemptCount);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressSourceMissingCount", &g_dbg_word800916F0ProviderIngressSourceMissingCount);
        DebugServer::RegisterReadOnlyVar("word800916F0ProviderIngressPublishedCount", &g_dbg_word800916F0ProviderIngressPublishedCount);
        DebugServer::RegisterReadOnlyVar("word800916F0MainGlobalBackingKnown", &g_dbg_word800916F0MainGlobalBackingKnown);
        DebugServer::RegisterReadOnlyVar("word800916F0MainGlobalBackingValue", &g_dbg_word800916F0MainGlobalBackingValue);
        DebugServer::RegisterReadOnlyVar("stage1OvlValid", &g_dbg_stage1OvlValid);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveStream", &g_dbg_stage1OvlActive.stream);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveEvent", &g_dbg_stage1OvlActive.event);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveQueryFrame", &g_dbg_stage1OvlActive.queryFrame);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveDueFrame", &g_dbg_stage1OvlActive.dueFrame);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveTextId", &g_dbg_stage1OvlActive.textId);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot0", &g_dbg_stage1OvlActive.slot0.slot);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1Cool", &g_dbg_stage1OvlActive.slot1Cool.slot);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1Good", &g_dbg_stage1OvlActive.slot1Good.slot);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1Bad", &g_dbg_stage1OvlActive.slot1Bad.slot);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1Awfl", &g_dbg_stage1OvlActive.slot1Awfl.slot);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot2", &g_dbg_stage1OvlActive.slot2.slot);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot0Event", &g_dbg_stage1OvlActive.slot0.event);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot0Frame", &g_dbg_stage1OvlActive.slot0.frame);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1CoolEvent", &g_dbg_stage1OvlActive.slot1Cool.event);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1CoolFrame", &g_dbg_stage1OvlActive.slot1Cool.frame);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1GoodEvent", &g_dbg_stage1OvlActive.slot1Good.event);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1GoodFrame", &g_dbg_stage1OvlActive.slot1Good.frame);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1BadEvent", &g_dbg_stage1OvlActive.slot1Bad.event);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1BadFrame", &g_dbg_stage1OvlActive.slot1Bad.frame);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1AwflEvent", &g_dbg_stage1OvlActive.slot1Awfl.event);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1AwflFrame", &g_dbg_stage1OvlActive.slot1Awfl.frame);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot2Event", &g_dbg_stage1OvlActive.slot2.event);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot2Frame", &g_dbg_stage1OvlActive.slot2.frame);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot0Tim0", &g_dbg_stage1OvlActive.slot0.tim0);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot0Tim1", &g_dbg_stage1OvlActive.slot0.tim1);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot0Tim2", &g_dbg_stage1OvlActive.slot0.tim2);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot0Tim3", &g_dbg_stage1OvlActive.slot0.tim3);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1CoolTim0", &g_dbg_stage1OvlActive.slot1Cool.tim0);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1CoolTim1", &g_dbg_stage1OvlActive.slot1Cool.tim1);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1CoolTim2", &g_dbg_stage1OvlActive.slot1Cool.tim2);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1CoolTim3", &g_dbg_stage1OvlActive.slot1Cool.tim3);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1GoodTim0", &g_dbg_stage1OvlActive.slot1Good.tim0);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1GoodTim1", &g_dbg_stage1OvlActive.slot1Good.tim1);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1GoodTim2", &g_dbg_stage1OvlActive.slot1Good.tim2);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1GoodTim3", &g_dbg_stage1OvlActive.slot1Good.tim3);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1BadTim0", &g_dbg_stage1OvlActive.slot1Bad.tim0);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1BadTim1", &g_dbg_stage1OvlActive.slot1Bad.tim1);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1BadTim2", &g_dbg_stage1OvlActive.slot1Bad.tim2);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1BadTim3", &g_dbg_stage1OvlActive.slot1Bad.tim3);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1AwflTim0", &g_dbg_stage1OvlActive.slot1Awfl.tim0);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1AwflTim1", &g_dbg_stage1OvlActive.slot1Awfl.tim1);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1AwflTim2", &g_dbg_stage1OvlActive.slot1Awfl.tim2);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot1AwflTim3", &g_dbg_stage1OvlActive.slot1Awfl.tim3);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot2Tim0", &g_dbg_stage1OvlActive.slot2.tim0);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot2Tim1", &g_dbg_stage1OvlActive.slot2.tim1);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot2Tim2", &g_dbg_stage1OvlActive.slot2.tim2);
        DebugServer::RegisterReadOnlyVar("stage1OvlActiveSlot2Tim3", &g_dbg_stage1OvlActive.slot2.tim3);
        if (ss0DirectCutover) {
            DebugServer::RegisterReadOnlyVar("stage1TextStream", &prCtx.debugStage1TextStream);
        } else {
            DebugServer::RegisterVar("stage1TextStream", &prCtx.debugStage1TextStream);
        }
        if (!ss0DirectCutover) {
            DebugServer::RegisterVar("stage1DirectBoot", &prCtx.debugStage1DirectBoot);
        }
        if (ss0DirectCutover) {
            DebugServer::RegisterReadOnlyVar("sceneExitReason", &prCtx.sceneExitReason);
        } else {
            DebugServer::RegisterVar("sceneExitReason", &prCtx.sceneExitReason);
        }
        // Debug hotkey flags
        if (ss0DirectCutover) {
            DebugServer::RegisterReadOnlyVar("f1StrSkip", &g_f1SkipRemote);
        } else {
            DebugServer::RegisterVar("f1StrSkip", &g_f1SkipRemote);
        }
        if (ss0DirectCutover) {
            DebugServer::RegisterReadOnlyVar("escEv4Exit", &g_escExitPressed);
        } else {
            DebugServer::RegisterVar("escEv4Exit", &g_escExitPressed);
        }
        if (ss0DirectCutover) {
            DebugServer::RegisterReadOnlyVar("f5StageClear", &g_f5Pressed);
        } else {
            DebugServer::RegisterVar("f5StageClear", &g_f5Pressed);
        }
        if (ss0DirectCutover) {
            DebugServer::RegisterReadOnlyVar("f6Ev2UnlockNext", &g_f6Pressed);
        } else {
            DebugServer::RegisterVar("f6Ev2UnlockNext", &g_f6Pressed);
        }
        if (ss0DirectCutover) {
            DebugServer::RegisterReadOnlyVar("f7Ev2FirstClear", &g_f7Pressed);
        } else {
            DebugServer::RegisterVar("f7Ev2FirstClear", &g_f7Pressed);
        }
        if (ss0DirectCutover) {
            DebugServer::RegisterReadOnlyVar("stageRunning", &g_dbg_stageRunning);
        } else {
            DebugServer::RegisterVar("stageRunning", &prCtx.stageRunning);
        }

        DebugServer::RegisterCommand("texfind", [&](const std::string& args) -> std::string {
            std::string a = args;
            while (!a.empty() && std::isspace((unsigned char)a.front())) a.erase(a.begin());
            while (!a.empty() && std::isspace((unsigned char)a.back())) a.pop_back();

            if (a.empty()) {
                return "ERR: usage: texfind <namepart> [limit]";
            }
            if (!prCtx.resources) {
                return "ERR: resources not ready";
            }

            std::string needle;
            int limit = 20;
            {
                size_t sp = a.find(' ');
                if (sp == std::string::npos) {
                    needle = a;
                } else {
                    needle = a.substr(0, sp);
                    std::string lim = a.substr(sp + 1);
                    while (!lim.empty() && std::isspace((unsigned char)lim.front())) lim.erase(lim.begin());
                    if (!lim.empty()) {
                        limit = std::max(1, std::min(200, std::atoi(lim.c_str())));
                    }
                }
            }

            std::transform(needle.begin(), needle.end(), needle.begin(), [](unsigned char c) { return (char)std::tolower(c); });

            const std::vector<std::string> names = prCtx.resources->GetTextureNames();
            std::string out;
            out.reserve(256);
            out += "OK: textures=" + std::to_string((int)names.size()) + " matches:";

            int matched = 0;
            for (int i = 0; i < (int)names.size(); i++) {
                std::string hay = names[(size_t)i];
                std::transform(hay.begin(), hay.end(), hay.begin(), [](unsigned char c) { return (char)std::tolower(c); });
                if (hay.find(needle) == std::string::npos) {
                    continue;
                }

                int w = 0;
                int h = 0;
                if (TextureResource* res = prCtx.resources->GetTexture(names[(size_t)i])) {
                    w = res->tim.width;
                    h = res->tim.height;
                }

                out += "\n";
                out += std::to_string(i);
                out += ": ";
                out += names[(size_t)i];
                if (w > 0 && h > 0) {
                    out += " (" + std::to_string(w) + "x" + std::to_string(h) + ")";
                }

                matched++;
                if (matched >= limit) {
                    break;
                }
            }

            if (matched == 0) {
                out += " (none)";
            }
            return out;
        });

        DebugServer::RegisterCommand("texdump", [&](const std::string& args) -> std::string {
            return DumpLoadedTexturesForDebug(prCtx, args);
        });

        DebugServer::RegisterCommand("tmddump", [&](const std::string& args) -> std::string {
            (void)args;
            const bool ss0DirectOwnedScene =
                PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
                (prCtx.currentScene == PrSceneId::Scene0 ||
                 prCtx.currentScene == PrSceneId::Scene1);
            if (ss0DirectOwnedScene) {
                std::string out =
                    "direct SS0: legacy PrTmdRenderer dump suppressed\n";
                out += PrStageSceneSubmitDebug::DumpStage1Stats();
                return out;
            }
            std::string out = PrTmdRenderer::DumpStats();
            out += PrStageSceneSubmitDebug::DumpStage1Stats();
            return out;
        });

        DebugServer::RegisterCommand("stage1ovl", [&](const std::string& args) -> std::string {
            std::istringstream iss(args);
            std::string sub;
            iss >> sub;
            const auto requireDirectScene1HandoffSurface = [&]() -> std::string {
                const bool directScene1Running =
                    ss0DirectCutover &&
                    prCtx.currentScene == PrSceneId::Scene1 &&
                    prCtx.stageRunning;
                if (!ss0DirectCutover || directScene1Running) {
                    return {};
                }
                Log::Printf(
                    "SS0 direct runtime: stage1ovl handoff surface rejected scene=%d stageRunning=%d",
                    (int)prCtx.currentScene,
                    prCtx.stageRunning ? 1 : 0);
                return "ERR: stage1ovl handoff requires direct Scene1 stageRunning";
            };
            if (sub.empty() || sub == "summary") {
                return Stage1OverlaySummary(prCtx);
            }
            if (sub == "rail") {
                std::string extra;
                if (iss >> extra) {
                    return "ERR: usage: stage1ovl rail";
                }
                return Stage1RailSnapshotDebugSurface(prCtx);
            }
            if (sub == "resolveframe") {
                int id = 0;
                uint32_t frame = 0;
                if (!(iss >> id >> frame)) {
                    return "ERR: usage: stage1ovl resolveframe <stream> <frame>";
                }
                return Stage1OverlayResolveFrame(prCtx, id, frame);
            }
            if (sub == "activehud") {
                return Stage1OverlayDescribeActiveHud(prCtx);
            }
            if (sub == "textstream") {
                int id = prCtx.debugStage1TextStream;
                if (!(iss >> id)) {
                    std::ostringstream out;
                    out << "textStream=" << prCtx.debugStage1TextStream
                        << " (0=auto/main stream1, 1..8=force parser text stream)";
                    return out.str();
                }
                if (id < 0 || id > 8) {
                    return "ERR: usage: stage1ovl textstream [0..8]";
                }
                const bool ss0DirectOwnedScene =
                    ss0DirectCutover &&
                    (prCtx.currentScene == PrSceneId::Scene0 ||
                     prCtx.currentScene == PrSceneId::Scene1);
                if (ss0DirectOwnedScene) {
                    return "ERR: stage1ovl textstream disabled in direct SS0";
                }
                prCtx.debugStage1TextStream = id;
                std::ostringstream out;
                out << "textStream=" << prCtx.debugStage1TextStream
                    << " (0=auto/main stream1, 1..8=force parser text stream)";
                return out.str();
            }
            if (sub == "handoff") {
                const std::string handoffGateError =
                    requireDirectScene1HandoffSurface();
                if (!handoffGateError.empty()) {
                    return handoffGateError;
                }
                return Stage1OverlayDescribeScoreDisplayHandoff(prCtx);
            }
            if (sub == "handoffhistory") {
                const std::string handoffGateError =
                    requireDirectScene1HandoffSurface();
                if (!handoffGateError.empty()) {
                    return handoffGateError;
                }
                int maxCount = 0;
                if (iss >> maxCount) {
                    return Stage1OverlayDescribeScoreDisplayHandoffHistory(
                        prCtx,
                        (maxCount > 0) ? (size_t)maxCount : 0u);
                }
                return Stage1OverlayDescribeScoreDisplayHandoffHistory(prCtx, 0u);
            }
            if (sub == "xacd359b8source") {
                const std::string handoffGateError =
                    requireDirectScene1HandoffSurface();
                if (!handoffGateError.empty()) {
                    return handoffGateError;
                }
                return Stage1OverlayDescribeXaCd359B8SourceAudit(prCtx);
            }
            if (sub == "xacdwindowread") {
                const std::string handoffGateError =
                    requireDirectScene1HandoffSurface();
                if (!handoffGateError.empty()) {
                    return handoffGateError;
                }
                return Stage1OverlayDescribeXaCdWindowReadAudit(prCtx);
            }
            if (sub == "frontdoorcurrenttick") {
                std::string mode;
                if (!(iss >> mode)) {
                    std::ostringstream out;
                    out << "frontDoorCurrentTickProbeDisabled="
                        << (PrScn1::IsStage1AcceptedProducerFrontDoorCurrentTickProbeDisabled()
                                ? 1
                                : 0)
                        << " (natural|off)";
                    return out.str();
                }
                if (mode == "natural" || mode == "on") {
                    PrScn1::SetStage1AcceptedProducerFrontDoorCurrentTickProbeDisabled(false);
                    return "OK: frontDoorCurrentTick=natural";
                }
                if (mode == "off") {
                    PrScn1::SetStage1AcceptedProducerFrontDoorCurrentTickProbeDisabled(true);
                    return "OK: frontDoorCurrentTick=off";
                }
                return "ERR: usage: stage1ovl frontdoorcurrenttick [natural|off]";
            }
            if (sub == "carriercurrenttick") {
                std::string mode;
                if (!(iss >> mode)) {
                    std::ostringstream out;
                    out << "carrierCurrentTickProbeDisabled="
                        << (PrScn1::IsStage1AcceptedProducerCarrierCurrentTickProbeDisabled()
                                ? 1
                                : 0)
                        << " (natural|off)";
                    return out.str();
                }
                if (mode == "natural" || mode == "on") {
                    PrScn1::SetStage1AcceptedProducerCarrierCurrentTickProbeDisabled(false);
                    return "OK: carrierCurrentTick=natural";
                }
                if (mode == "off") {
                    PrScn1::SetStage1AcceptedProducerCarrierCurrentTickProbeDisabled(true);
                    return "OK: carrierCurrentTick=off";
                }
                return "ERR: usage: stage1ovl carriercurrenttick [natural|off]";
            }
            if (sub == "currentgetlocp") {
                std::string mode;
                if (!(iss >> mode)) {
                    std::ostringstream out;
                    out << "currentPhysicalGetlocPProbeDisabled="
                        << (PrScn1::IsStage1XaCurrentPhysicalGetlocPProbeDisabled()
                                ? 1
                                : 0)
                        << " (natural|off)";
                    return out.str();
                }
                if (mode == "natural" || mode == "on") {
                    PrScn1::SetStage1XaCurrentPhysicalGetlocPProbeDisabled(false);
                    return "OK: currentPhysicalGetlocP=natural";
                }
                if (mode == "off") {
                    PrScn1::SetStage1XaCurrentPhysicalGetlocPProbeDisabled(true);
                    return "OK: currentPhysicalGetlocP=off";
                }
                return "ERR: usage: stage1ovl currentgetlocp [natural|off]";
            }
            if (sub == "padat") {
                std::string mode;
                if (!(iss >> mode) || mode == "status") {
                    return DescribeStage1ScheduledPadProbe();
                }
                if (mode == "clear") {
                    ClearStage1ScheduledPadProbe("command");
                    return "OK: padat=clear";
                }
                const auto parseUnsigned = [](const std::string& text,
                                              unsigned long* outValue) -> bool {
                    try {
                        size_t pos = 0;
                        const unsigned long value = std::stoul(text, &pos, 0);
                        if (pos != text.size()) {
                            return false;
                        }
                        *outValue = value;
                        return true;
                    } catch (...) {
                        return false;
                    }
                };
                unsigned long targetScene = 1;
                unsigned long targetFrame = 0;
                unsigned long mask = 0;
                if (mode == "scene") {
                    std::string sceneText;
                    std::string frameText;
                    if (!(iss >> sceneText) ||
                        !parseUnsigned(sceneText, &targetScene) ||
                        targetScene > 9ul ||
                        !(iss >> frameText) ||
                        !parseUnsigned(frameText, &targetFrame)) {
                        return "ERR: usage: stage1ovl padat [scene <sceneId>] <targetFrame> <mask> [holdFrames]|status|clear";
                    }
                } else if (!parseUnsigned(mode, &targetFrame)) {
                    return "ERR: usage: stage1ovl padat [scene <sceneId>] <targetFrame> <mask> [holdFrames]|status|clear";
                }
                if (targetScene == 1ul) {
                    const std::string handoffGateError =
                        requireDirectScene1HandoffSurface();
                    if (!handoffGateError.empty()) {
                        return handoffGateError;
                    }
                } else if (!PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
                    return "ERR: direct SS0 runtime required";
                }
                std::string maskText;
                if (!(iss >> maskText) || !parseUnsigned(maskText, &mask) || mask > 0xFFFFul) {
                    return "ERR: usage: stage1ovl padat [scene <sceneId>] <targetFrame> <mask> [holdFrames]|status|clear";
                }
                unsigned long holdFrames = 1;
                std::string holdText;
                if (iss >> holdText) {
                    if (!parseUnsigned(holdText, &holdFrames) ||
                        holdFrames == 0 ||
                        holdFrames > 30ul) {
                        return "ERR: usage: stage1ovl padat [scene <sceneId>] <targetFrame> <mask> [holdFrames]|status|clear";
                    }
                }
                std::string extra;
                if (iss >> extra || targetFrame > 1000000ul) {
                    return "ERR: usage: stage1ovl padat [scene <sceneId>] <targetFrame> <mask> [holdFrames]|status|clear";
                }
                Stage1ScheduledPadProbe armed{};
                armed.enabled = true;
                armed.fired = false;
                armed.targetScene = static_cast<int>(targetScene);
                armed.targetFrame = static_cast<int>(targetFrame);
                armed.holdFrames = static_cast<int>(holdFrames);
                armed.remainingFrames = 0;
                armed.firedFrame = -1;
                armed.mask = static_cast<uint16_t>(mask);
                g_stageScheduledPadProbes.push_back(armed);
                g_stage1ScheduledPadProbe = armed;
                Log::Printf(
                    "Stage padat probe armed targetScene=%d targetFrame=%d holdFrames=%d mask=0x%04X currentStageFrame=%d currentScene=%u",
                    g_stage1ScheduledPadProbe.targetScene,
                    g_stage1ScheduledPadProbe.targetFrame,
                    g_stage1ScheduledPadProbe.holdFrames,
                    (unsigned)g_stage1ScheduledPadProbe.mask,
                    g_dbg_stageFrame,
                    static_cast<unsigned>(prCtx.currentScene));
                return "OK: padat=armed " + DescribeStage1ScheduledPadProbe();
            }
            if (sub == "scene428b0" || sub == "sceneSubmit428B0") {
                std::string extra;
                if (iss >> extra) {
                    return "ERR: usage: stage1ovl scene428b0";
                }
                return PrStageSceneSubmitBackend::
                    DescribeStage1SceneSubmit428B0Debug();
            }
            return "ERR: usage: stage1ovl [summary|rail|resolveframe <stream> <frame>|activehud|textstream [0..8]|handoff|handoffhistory [count]|xacd359b8source|xacdwindowread|frontdoorcurrenttick [natural|off]|carriercurrenttick [natural|off]|currentgetlocp [natural|off]|padat <targetFrame> <mask> [holdFrames]|scene428b0]";
        });
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Main loop with fixed 30Hz logic tick + variable render rate
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    auto lastFpsTime = lastFrameTime;
    int fpsFrames = 0;
    const auto startTime = lastFrameTime;
    double logicAccum = 0.0;  // accumulator for 30Hz logic tick

    MSG msg = {};
    while (g_running) {
        if (g_autoExitSeconds > 0) {
            auto nowExit = std::chrono::high_resolution_clock::now();
            const double secs = std::chrono::duration<double>(nowExit - startTime).count();
            if (secs >= (double)g_autoExitSeconds) {
                Log::Printf("AutoExit: reached %.3f sec (limit=%d), quitting...", secs, g_autoExitSeconds);
                g_running = false;
                break;
            }
        }

        // Process messages
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (!g_running) break;

        // Frame timing
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - lastFrameTime).count();
        const double renderFrameMs = g_render60fps ? FRAME_TIME_60_MS : FRAME_TIME_MS;

        if (elapsed >= renderFrameMs) {
            int renderSteps = (int)(elapsed / renderFrameMs);
            if (renderSteps < 1) {
                renderSteps = 1;
            }
            const double steppedMs = renderFrameMs * (double)renderSteps;
            prCtx.hostPresentationVblank60 += static_cast<uint64_t>(renderSteps) *
                (g_render60fps ? 1u : 2u);
            lastFrameTime += std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(
                std::chrono::duration<double, std::milli>(steppedMs));
            fpsFrames++;

            // Logic tick accumulator: always 30Hz regardless of render rate
            logicAccum += steppedMs;
            bool doLogicTick = false;
            if (logicAccum >= FRAME_TIME_MS) {
                logicAccum -= FRAME_TIME_MS;
                // Prevent spiral-of-death: cap accumulated logic debt
                if (logicAccum > FRAME_TIME_MS * 2.0) {
                    logicAccum = 0.0;
                }
                doLogicTick = true;
                g_frameNum++;
            }
            {
                const double fpsElapsed = std::chrono::duration<double>(now - lastFpsTime).count();
                if (fpsElapsed >= 1.0) {
                    g_lastFps = (fpsElapsed > 0.0) ? ((double)fpsFrames / fpsElapsed) : 0.0;
                    fpsFrames = 0;
                    lastFpsTime = now;
                    if (g_state == GameState::Playing) {
                        UpdatePlayingWindowTitle(hwnd, prCtx.currentScene);
                    }
                }
            }

            // Poll real pad input every render step so 60Hz render mode does
            // not miss short edges between 30Hz logic frames. Logical
            // consumers still see a single latched state per logic tick.
            PrPad::Poll(GetForegroundWindow() == hwnd);

            // ===== Logic update: only on 30Hz tick =====
            if (doLogicTick) {

            // Commit any high-frequency pad edges to the current logic frame.
            PrPad::CommitLogicFrame();

            g_dbg_appState = (int)g_state;
            g_dbg_xaPlaying = g_xa1Player.IsPlaying() ? 1 : 0;
            g_dbg_xaPlayedMs = (int)(g_xa1Player.GetPlayedSeconds() * 1000.0 + 0.5);
            g_dbg_stageFrame = GetStageRunner().GetFrame();
            g_dbg_stageTick96 = GetStageRunner().GetTick96();
            g_dbg_stageRunning = prCtx.stageRunning ? 1 : 0;
            g_dbg_strPlaying = g_strPlayer.IsPlaying() ? 1 : 0;
            g_dbg_strFrame = g_dbg_strPlaying ? (int)g_strPlayer.GetCurrentFrame() : -1;
            g_dbg_subtitleFlag = prCtx.subtitleFlag;
            const bool directScene0HandoffActive =
                PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
                prCtx.currentScene == PrSceneId::Scene0 &&
                PrSS0Scene0RuntimeDirect::IsSceneHandoffActive();
            g_dbg_transActive = directScene0HandoffActive
                ? 1
                : (PrTransition::IsActive() ? 1 : 0);
            g_dbg_transPhase = directScene0HandoffActive
                ? static_cast<int>(TransitionPhase::Hold)
                : static_cast<int>(PrTransition::GetPhase());
            g_dbg_transTotalFrame = directScene0HandoffActive
                ? 0
                : PrTransition::GetTotalFrameCount();
            g_dbg_transTargetScene = directScene0HandoffActive
                ? PrSS0Scene0RuntimeDirect::GetSceneHandoffTarget()
                : PrTransition::GetTargetScene();
            g_dbg_stage1ClearTailMovie =
                PrScn1::GetStage1ClearTailMovieDebugSnapshot();
            {
                const auto word800916F0Snapshot =
                    PrSS0Scene0RuntimeDirect::
                        GetWord800916F0ObservationDebugSnapshot();
                g_dbg_word800916F0Known =
                    word800916F0Snapshot.known ? 1 : 0;
                const auto softwareWord800916F0 =
                    PrSS0Scene0RuntimeDirect::GetWord800916F0SoftwareState();
                g_dbg_word800916F0SoftwareKnown = softwareWord800916F0.initialized ? 1 : 0;
                g_dbg_word800916F0SoftwareValue = softwareWord800916F0.value;
                g_dbg_word800916F0SoftwareStartupCommitCount =
                    static_cast<int>(softwareWord800916F0.startupCommitCount);
                g_dbg_word800916F0ObservationAcceptedCount =
                    word800916F0Snapshot.acceptedCount;
                g_dbg_word800916F0RuntimeObservationAcceptedCount =
                    word800916F0Snapshot.runtimeObservationAcceptedCount;
                g_dbg_word800916F0StartupObservationAcceptedCount =
                    word800916F0Snapshot.startupObservationAcceptedCount;
                g_dbg_word800916F0ObservationRejectedCount =
                    word800916F0Snapshot.rejectedCount;
                g_dbg_word800916F0ObservationLastRejectReason =
                    static_cast<int>(
                        word800916F0Snapshot.lastRejectReason);
                g_dbg_word800916F0ObservationLastStartupSource =
                    static_cast<int>(
                        word800916F0Snapshot.lastStartupSource);
                const auto word800916F0ProviderIngress =
                    PrMain::GetWord800916F0ProviderIngressDebug();
                g_dbg_word800916F0ProviderIngressAttempted =
                    word800916F0ProviderIngress.attempted ? 1 : 0;
                g_dbg_word800916F0ProviderIngressReadable =
                    word800916F0ProviderIngress.readable ? 1 : 0;
                g_dbg_word800916F0ProviderIngressPublishAttempted =
                    word800916F0ProviderIngress.publishAttempted ? 1 : 0;
                g_dbg_word800916F0ProviderIngressPublished =
                    word800916F0ProviderIngress.published ? 1 : 0;
                g_dbg_word800916F0ProviderIngressKnownAfter =
                    word800916F0ProviderIngress.knownAfter ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceMainGlobalAttempted =
                    word800916F0ProviderIngress.sourceMainGlobalAttempted ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceMainGlobalReadable =
                    word800916F0ProviderIngress.sourceMainGlobalReadable ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceMainGlobalSelfBootstrapBlocked =
                    word800916F0ProviderIngress.sourceMainGlobalSelfBootstrapBlocked ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceLoaderHeapAttempted =
                    word800916F0ProviderIngress.sourceLoaderHeapAttempted ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceLoaderHeapReadable =
                    word800916F0ProviderIngress.sourceLoaderHeapReadable ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceKnownStateRelayAttempted =
                    word800916F0ProviderIngress.sourceKnownStateRelayAttempted ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceKnownStateRelayReadable =
                    word800916F0ProviderIngress.sourceKnownStateRelayReadable ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceCdMmioSnapshotAttempted =
                    word800916F0ProviderIngress.sourceCdMmioSnapshotAttempted ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceCdMmioSnapshotReadable =
                    word800916F0ProviderIngress.sourceCdMmioSnapshotReadable ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceExactCdAttempted =
                    word800916F0ProviderIngress.sourceExactCdAttempted ? 1 : 0;
                g_dbg_word800916F0ProviderIngressSourceExactCdReadable =
                    word800916F0ProviderIngress.sourceExactCdReadable ? 1 : 0;
                g_dbg_word800916F0ProviderIngressAttemptCount =
                    static_cast<int>(
                        word800916F0ProviderIngress.attemptCount);
                g_dbg_word800916F0ProviderIngressSourceMissingCount =
                    static_cast<int>(
                        word800916F0ProviderIngress.sourceMissingCount);
                g_dbg_word800916F0ProviderIngressPublishedCount =
                    static_cast<int>(
                        word800916F0ProviderIngress.publishedCount);
                g_dbg_word800916F0MainGlobalBackingKnown =
                    prCtx.word800916F0Known ? 1 : 0;
                g_dbg_word800916F0MainGlobalBackingValue =
                    static_cast<int>(prCtx.word800916F0);
            }
            if (prCtx.stage1OverlayData && prCtx.stage1OverlayData->valid) {
                g_dbg_stage1OvlValid = 1;
                PrStage1OverlayData scratch;
                const PrStage1OverlayData* resolvedData = nullptr;
                PrStage1ResolvedTextEvent latestResolved;
                uint32_t queryFrame = 0;
                if (TryResolveLatestStage1EventForDebug(prCtx,
                                                        scratch,
                                                        resolvedData,
                                                        latestResolved,
                                                        queryFrame) &&
                    latestResolved.valid) {
                    ApplyStage1OverlayResolvedDebugVars(latestResolved, queryFrame);
                } else {
                    ClearStage1OverlayResolvedDebugVars();
                }
            } else {
                g_dbg_stage1OvlValid = 0;
                ClearStage1OverlayResolvedDebugVars();
            }
            const bool ss0DirectRuntime =
                PrSS0Scene0RuntimeDirect::RuntimeEnabled();
            const bool ss0DirectOwnedCurrentScene =
                ss0DirectRuntime &&
                (prCtx.currentScene == PrSceneId::Scene0 ||
                 prCtx.currentScene == PrSceneId::Scene1);
            PrSS0Scene0RuntimeDirect::DebugSnapshot ss0Debug{};
            const bool ss0DebugSnapshotValid =
                ss0DirectOwnedCurrentScene && (prCtx.currentScene == PrSceneId::Scene0 ||
                    PrSS0Scene0RuntimeDirect::IsResidentDirectoryActive80015788());
            if (ss0DebugSnapshotValid) {
                ss0Debug = PrSS0Scene0RuntimeDirect::GetDebugSnapshot();
                g_dbg_scn0Phase = ss0Debug.phaseDebug;
                g_dbg_ss0PhaseRaw = ss0Debug.phaseRaw;
                g_dbg_ss0PhaseDebug = ss0Debug.phaseDebug;
                g_dbg_ss0TransitionReturnPhaseRaw =
                    ss0Debug.transitionReturnPhaseRaw;
                g_dbg_ss0DirectDispState = ss0Debug.directDispState;
                g_dbg_ss0DirectDispEventId = ss0Debug.directDispEventId;
                g_dbg_ss0DirectMenuIndex = ss0Debug.directMenuIndex;
                g_dbg_ss0DirectMainMenuRecordsMode =
                    ss0Debug.directMainMenuRecordsMode;
                g_dbg_ss0DirectCardEntryCount = ss0Debug.directCardEntryCount;
                g_dbg_ss0DirectCardSelectedBlock =
                    ss0Debug.directCardSelectedBlock;
                g_dbg_ss0DirectStageSelectEnabledMask =
                    ss0Debug.directStageSelectEnabledMask;
                g_dbg_ss0DirectOptionsLanguage =
                    ss0Debug.directOptionsLanguage;
                g_dbg_ss0DirectOptionsSubtitle =
                    ss0Debug.directOptionsSubtitle;
                g_dbg_ss0DirectOptionsPreLoopReleasePending =
                    ss0Debug.directOptionsPreLoopReleasePending;
                g_dbg_ss0DirectOptionsCooldown =
                    ss0Debug.directOptionsCooldown;
                g_dbg_ss0DirectOptionsTimeoutRemaining =
                    ss0Debug.directOptionsTimeoutRemaining;
                g_dbg_ss0DirectOptionsInitialInputPending =
                    ss0Debug.directOptionsInitialInputPending;
                g_dbg_ss0DirectOptionsTailActive =
                    ss0Debug.directOptionsTailActive;
                g_dbg_ss0DirectOptionsTailFramesRemaining =
                    ss0Debug.directOptionsTailFramesRemaining;
                g_dbg_ss0DirectOptionsTailResult =
                    ss0Debug.directOptionsTailResult;
                g_dbg_ss0DirectPracticePhase = ss0Debug.directPracticePhase;
                g_dbg_ss0DirectPracticeRound = ss0Debug.directPracticeRound;
                g_dbg_ss0DirectPracticeRoundFrame =
                    ss0Debug.directPracticeRoundFrame;
                g_dbg_ss0DirectPracticePadStopFrames =
                    ss0Debug.directPracticePadStopFrames;
                g_dbg_ss0DirectPracticePadStopKind =
                    ss0Debug.directPracticePadStopKind;
                g_dbg_ss0DirectPracticeExitConfirmFrames =
                    ss0Debug.directPracticeExitConfirmFrames;
                g_dbg_ss0DirectPracticeScore = ss0Debug.directPracticeScore;
                g_dbg_ss0DirectPracticeHits = ss0Debug.directPracticeHits;
                g_dbg_ss0DirectPracticeMisses = ss0Debug.directPracticeMisses;
                g_dbg_ss0DirectPracticeLastJudge =
                    ss0Debug.directPracticeLastJudge;
                g_dbg_ss0DirectPracticeLastDelta =
                    ss0Debug.directPracticeLastDelta;
                g_dbg_ss0DirectPracticeJudgeFlash =
                    ss0Debug.directPracticeJudgeFlash;
                g_dbg_ss0DirectPracticeSeqCurA =
                    ss0Debug.directPracticeSeqCurA;
                g_dbg_ss0DirectPracticeSeqCurB =
                    ss0Debug.directPracticeSeqCurB;
                g_dbg_ss0DirectPracticeSeqEnA =
                    ss0Debug.directPracticeSeqEnA;
                g_dbg_ss0DirectPracticeSeqEnB =
                    ss0Debug.directPracticeSeqEnB;
                g_dbg_ss0DirectHiScoreBlink = ss0Debug.directHiScoreBlink;
                g_dbg_ss0DirectHiScoreExitLabelState =
                    ss0Debug.directHiScoreExitLabelState;
                g_dbg_ss0DirectHiScorePreLoopReleasePending =
                    ss0Debug.directHiScorePreLoopReleasePending;
                g_dbg_ss0DirectHiScoreTailActive =
                    ss0Debug.directHiScoreTailActive;
                g_dbg_ss0DirectHiScoreTailFramesRemaining =
                    ss0Debug.directHiScoreTailFramesRemaining;
                g_dbg_ss0DirectHiScoreTailResult =
                    ss0Debug.directHiScoreTailResult;
                g_dbg_ss0DirectHiScoreOuterPadReleaseActive =
                    ss0Debug.directHiScoreOuterPadReleaseActive;
                g_dbg_ss0DirectHiScoreOuterPadReleasePending =
                    ss0Debug.directHiScoreOuterPadReleasePending;
                g_dbg_ss0DirectHiScoreEvent6TableKnown =
                    ss0Debug.directHiScoreEvent6TableKnown;
                g_dbg_ss0DirectHiScoreEvent6TablePsxAddress =
                    static_cast<int>(
                        ss0Debug.directHiScoreEvent6TablePsxAddress);
                g_dbg_ss0DirectHiScoreEvent6TableByteCount =
                    ss0Debug.directHiScoreEvent6TableByteCount;
                g_dbg_ss0DirectTitleExitWaitCounter =
                    ss0Debug.directTitleExitWaitCounter;
                g_dbg_ss0DirectTitleExitTargetScene =
                    ss0Debug.directTitleExitTargetScene;
                g_dbg_ss0DirectTitleExitPendingMenu =
                    ss0Debug.directTitleExitPendingMenu;
                g_dbg_ss0DirectTitleLoopStateV8 =
                    ss0Debug.directTitleLoopStateV8;
                g_dbg_ss0LoadingHoldKind = ss0Debug.loadingHoldKind;
                g_dbg_ss0LoadingScreenKind = ss0Debug.loadingScreenKind;
                g_dbg_ss0LoadingHoldStartFrame =
                    static_cast<int>(ss0Debug.loadingHoldStartFrame);
                g_dbg_ss0LoadingHoldUntilFrame =
                    static_cast<int>(ss0Debug.loadingHoldUntilFrame);
                g_dbg_ss0LoadingPatternActive8001EF40 =
                    ss0Debug.loadingPatternActive8001EF40;
                g_dbg_ss0LoadingPatternStyle8001EF40 =
                    ss0Debug.loadingPatternStyle8001EF40;
                g_dbg_ss0LoadingPatternHighlightCount8001EF40 =
                    ss0Debug.loadingPatternHighlightCount8001EF40;
                g_dbg_ss0LoadingPatternSubmittedHighlightCount8001EF40 =
                    ss0Debug.loadingPatternSubmittedHighlightCount8001EF40;
                g_dbg_ss0LoadingPatternMutationSerial8001EF40 =
                    static_cast<int>(
                        ss0Debug.loadingPatternMutationSerial8001EF40);
                g_dbg_ss0LoadingPatternCallbackCount8001537C =
                    static_cast<int>(
                        ss0Debug.loadingPatternCallbackCount8001537C);
                g_dbg_ss0LoadingPatternGridHashLow8001EF40 =
                    static_cast<int>(
                        ss0Debug.loadingPatternGridHashLow8001EF40);
                g_dbg_ss0LoadingPatternSubmittedFrame8001EF40 =
                    static_cast<int>(
                        ss0Debug.loadingPatternSubmittedFrame8001EF40);
            } else {
                g_dbg_scn0Phase = -1;
                g_dbg_ss0PhaseRaw = -1;
                g_dbg_ss0PhaseDebug = -1;
                g_dbg_ss0TransitionReturnPhaseRaw = -1;
                g_dbg_ss0DirectDispState = 0;
                g_dbg_ss0DirectDispEventId = 0;
                g_dbg_ss0DirectMenuIndex = -1;
                g_dbg_ss0DirectMainMenuRecordsMode = 0;
                g_dbg_ss0DirectCardEntryCount = 0;
                g_dbg_ss0DirectCardSelectedBlock = -1;
                g_dbg_ss0DirectStageSelectEnabledMask = 0;
                g_dbg_ss0DirectOptionsLanguage = -1;
                g_dbg_ss0DirectOptionsSubtitle = -1;
                g_dbg_ss0DirectOptionsPreLoopReleasePending = -1;
                g_dbg_ss0DirectOptionsCooldown = -1;
                g_dbg_ss0DirectOptionsTimeoutRemaining = -1;
                g_dbg_ss0DirectOptionsInitialInputPending = -1;
                g_dbg_ss0DirectOptionsTailActive = -1;
                g_dbg_ss0DirectOptionsTailFramesRemaining = -1;
                g_dbg_ss0DirectOptionsTailResult = -1;
                g_dbg_ss0DirectPracticePhase = -1;
                g_dbg_ss0DirectPracticeRound = -1;
                g_dbg_ss0DirectPracticeRoundFrame = -1;
                g_dbg_ss0DirectPracticePadStopFrames = 0;
                g_dbg_ss0DirectPracticePadStopKind = -1;
                g_dbg_ss0DirectPracticeExitConfirmFrames = 0;
                g_dbg_ss0DirectPracticeScore = 0;
                g_dbg_ss0DirectPracticeHits = 0;
                g_dbg_ss0DirectPracticeMisses = 0;
                g_dbg_ss0DirectPracticeLastJudge = 9;
                g_dbg_ss0DirectPracticeLastDelta = 0;
                g_dbg_ss0DirectPracticeJudgeFlash = 0;
                g_dbg_ss0DirectPracticeSeqCurA = -1;
                g_dbg_ss0DirectPracticeSeqCurB = -1;
                g_dbg_ss0DirectPracticeSeqEnA = 0;
                g_dbg_ss0DirectPracticeSeqEnB = 0;
                g_dbg_ss0DirectHiScoreBlink = 0;
                g_dbg_ss0DirectHiScoreExitLabelState = -1;
                g_dbg_ss0DirectHiScorePreLoopReleasePending = -1;
                g_dbg_ss0DirectHiScoreTailActive = -1;
                g_dbg_ss0DirectHiScoreTailFramesRemaining = -1;
                g_dbg_ss0DirectHiScoreTailResult = -1;
                g_dbg_ss0DirectHiScoreOuterPadReleaseActive = -1;
                g_dbg_ss0DirectHiScoreOuterPadReleasePending = -1;
                g_dbg_ss0DirectHiScoreEvent6TableKnown = 0;
                g_dbg_ss0DirectHiScoreEvent6TablePsxAddress = 0;
                g_dbg_ss0DirectHiScoreEvent6TableByteCount = 0;
                g_dbg_ss0DirectTitleExitWaitCounter = 0;
                g_dbg_ss0DirectTitleExitTargetScene = -1;
                g_dbg_ss0DirectTitleExitPendingMenu = 0;
                g_dbg_ss0DirectTitleLoopStateV8 = 0;
                g_dbg_ss0LoadingHoldKind = 0;
                g_dbg_ss0LoadingScreenKind = 0;
                g_dbg_ss0LoadingHoldStartFrame = 0;
                g_dbg_ss0LoadingHoldUntilFrame = 0;
                g_dbg_ss0LoadingPatternActive8001EF40 = 0;
                g_dbg_ss0LoadingPatternStyle8001EF40 = 0;
                g_dbg_ss0LoadingPatternHighlightCount8001EF40 = 0;
                g_dbg_ss0LoadingPatternSubmittedHighlightCount8001EF40 = 0;
                g_dbg_ss0LoadingPatternMutationSerial8001EF40 = 0;
                g_dbg_ss0LoadingPatternCallbackCount8001537C = 0;
                g_dbg_ss0LoadingPatternGridHashLow8001EF40 = 0;
                g_dbg_ss0LoadingPatternSubmittedFrame8001EF40 = 0;
            }

            // Update debug server state and consume injected inputs
            int debugMenuIndex = -1;
            int debugDispatcherRunning = 0;
            if (ss0DirectOwnedCurrentScene) {
                if (ss0DebugSnapshotValid) {
                    debugMenuIndex = ss0Debug.directMenuIndex;
                    debugDispatcherRunning =
                        ss0Debug.directDispState != 0 ? 1 : 0;
                }
            } else if (!ss0DirectRuntime && PrEvent::IsDispatcherRunning()) {
                debugMenuIndex = PrEvent::GetDispatcherContext().menuIndex;
                debugDispatcherRunning = 1;
            }
            DebugServer::SetGameState(g_frameNum, (int)prCtx.currentScene,
                debugMenuIndex,
                debugDispatcherRunning);
            // Extended state: gameState (0=BootLogo,1=Menu,2=Playing,3=Exit)
            int gameStateVal = 0;
            if (g_state == GameState::BootLogo) gameStateVal = 0;
            else if (g_state == GameState::StrPlayback) gameStateVal = 2;
            else if (g_state == GameState::Playing) gameStateVal = 2;
            else if (g_state == GameState::Exit) gameStateVal = 3;
            DebugServer::SetGameStateEx(g_frameNum, (int)prCtx.currentScene,
                debugMenuIndex,
                debugDispatcherRunning,
                gameStateVal);
            int debugHeldPadBoundaryScene = -1;
            int debugHeldPadBoundaryPhaseRaw = -1;
            auto clearDebugPadInputOnBoundary = [&]() {
                int clearedPendingPadCount = 0;
                int clearedPendingPadNonZeroCount = 0;
                if (DebugServer::ClearPendingPadInput(
                        &clearedPendingPadCount,
                        &clearedPendingPadNonZeroCount)) {
                    Log::Printf(
                        "DebugServer direct SS0: cleared pending pad input on boundary scene=%d phase=%d pending=%d pendingNonZero=%d",
                        debugHeldPadBoundaryScene,
                        debugHeldPadBoundaryPhaseRaw,
                        clearedPendingPadCount,
                        clearedPendingPadNonZeroCount);
                }
                if (DebugServer::ClearHeldPadInput()) {
                    Log::Printf(
                        "DebugServer direct SS0: cleared held pad input on boundary scene=%d phase=%d",
                        debugHeldPadBoundaryScene,
                        debugHeldPadBoundaryPhaseRaw);
                }
                ClearStage1ScheduledPadProbe("boundary");
            };
            {
                static bool s_debugHeldPadBoundaryInit = false;
                static bool s_lastHeldPadDirectOwned = false;
                static int s_lastHeldPadScene = -1;
                static int s_lastHeldPadPhaseRaw = -1;
                const int heldPadScene =
                    ss0DirectOwnedCurrentScene ? (int)prCtx.currentScene : -1;
                const int heldPadPhaseRaw =
                    ss0DebugSnapshotValid ? ss0Debug.phaseRaw : -1;
                if (!s_debugHeldPadBoundaryInit) {
                    s_debugHeldPadBoundaryInit = true;
                } else if (
                    s_lastHeldPadDirectOwned != ss0DirectOwnedCurrentScene ||
                    s_lastHeldPadScene != heldPadScene ||
                    s_lastHeldPadPhaseRaw != heldPadPhaseRaw) {
                    debugHeldPadBoundaryScene = heldPadScene;
                    debugHeldPadBoundaryPhaseRaw = heldPadPhaseRaw;
                    clearDebugPadInputOnBoundary();
                }
                s_lastHeldPadDirectOwned = ss0DirectOwnedCurrentScene;
                s_lastHeldPadScene = heldPadScene;
                s_lastHeldPadPhaseRaw = heldPadPhaseRaw;
                }
                DebugServer::Update();
                {
                char k;
                while (DebugServer::ConsumeKeyPress(k)) {
                    if (k == 'G' || k == 'g') g_genericSwitchPressed = true;
                    else if (k >= '0' && k <= '9') {
                        g_genericEvent = k - '0';
                        g_genericSwitchPressed = true;
                    }
                    else if (k == 'N' || k == 'n') g_nextScenePressed = true;
                    else if (k == 'W' || k == 'w') g_upPressed = true;
                    else if (k == 'S' || k == 's') g_downPressed = true;
                    else if (k == '\r' || k == ' ') g_confirmPressed = true;
                }
                int ev;
                if (DebugServer::ConsumeGenericEvent(ev)) {
                    g_genericEvent = ev;
                }
                if (DebugServer::ConsumeGenericSwitch()) {
                    g_genericSwitchPressed = true;
                }
                {
                    int shotId = 0;
                    std::string stem;
                    while (DebugServer::ConsumeScreenshotRequestInfo(shotId, stem)) {
                        g_pendingShotPaths.push(MakeDebugShotFilePath(shotId, stem));
                    }
                }
                int inputOnly;
                if (DebugServer::ConsumeInputOnly(inputOnly)) {
                    g_genericEvent = inputOnly;  // Set input without triggering switch
                }
                uint16_t padMask;
                if (DebugServer::ConsumePadInput(padMask)) {
                    // 直接设置 PSX pad mask 到 context
                    prCtx.debugPadInput = padMask;
                    if (padMask != 0u) {
                        Log::Printf(
                            "DebugServer direct SS0: consumed pad input frame=%d scene=%u pad=0x%04X",
                            g_frameNum,
                            static_cast<unsigned>(prCtx.currentScene),
                            static_cast<unsigned>(padMask));
                    }
                }
                if (!g_stageScheduledPadProbes.empty() && ss0DirectRuntime &&
                    prCtx.stageRunning) {
                    for (Stage1ScheduledPadProbe& probe :
                         g_stageScheduledPadProbes) {
                        if (!probe.enabled ||
                            static_cast<int>(prCtx.currentScene) != probe.targetScene) {
                            continue;
                        }
                        if (!probe.fired &&
                            g_dbg_stageFrame >= probe.targetFrame) {
                            probe.fired = true;
                            probe.firedFrame = g_dbg_stageFrame;
                            probe.remainingFrames = probe.holdFrames;
                            Log::Printf(
                                "Stage padat probe fired targetScene=%d targetFrame=%d firedFrame=%d holdFrames=%d mask=0x%04X",
                                probe.targetScene,
                                probe.targetFrame,
                                probe.firedFrame,
                                probe.holdFrames,
                                (unsigned)probe.mask);
                        }
                        if (probe.remainingFrames > 0) {
                            prCtx.debugPadInput = probe.mask;
                            --probe.remainingFrames;
                            if (probe.remainingFrames == 0) {
                                probe.enabled = false;
                                Log::Printf(
                                    "Stage padat probe completed targetScene=%d targetFrame=%d firedFrame=%d mask=0x%04X",
                                    probe.targetScene,
                                    probe.targetFrame,
                                    probe.firedFrame,
                                    (unsigned)probe.mask);
                            }
                        }
                    }
                    while (!g_stageScheduledPadProbes.empty() &&
                           !g_stageScheduledPadProbes.front().enabled) {
                        g_stageScheduledPadProbes.erase(
                            g_stageScheduledPadProbes.begin());
                    }
                    if (!g_stageScheduledPadProbes.empty()) {
                        g_stage1ScheduledPadProbe = g_stageScheduledPadProbes.front();
                    } else {
                        g_stage1ScheduledPadProbe = {};
                    }
                }
            }

            // Normal keyboard control now comes through per-render PrPad::Poll()
            // plus per-logic-frame PrPad::CommitLogicFrame().
            // Keep debugPadInput reserved for explicit debug-server pad injection only.

            // State machine update
            switch (g_state) {
                case GameState::BootLogo: {
                    const PrPadState bootPad = PrPad::GetState(0);
                    const uint16_t bootPadCurrent =
                        static_cast<uint16_t>(bootPad.pressed | bootPad.held);
                    // SCUS 80015B00 exits its skippable wait only for the
                    // PSX Start value 0x0800.  PrPad's local Start bit is
                    // translated to that value by the normal input bridge;
                    // F1 remains an explicit Win-only debug shortcut.
                    const bool skip =
                        (bootPadCurrent &
                         static_cast<uint16_t>(PrPadButton::Start)) != 0u ||
                        g_f1Pressed || g_f1SkipRemote;
                    bool complete = g_bootLogo.Update(skip);
                    if (g_frameNum == 1 || g_frameNum == 30 || g_frameNum == 60 || (g_frameNum % 60) == 0) {
                        Log::Printf("BootLogo tick frame=%d complete=%d", g_frameNum, complete ? 1 : 0);
                    }
                    g_f1Pressed = false;
                    g_f1SkipRemote = false;

                    if (complete) {
                        Log::Printf("BootLogo complete, shutting down...");
                        g_bootLogo.Shutdown();
                        Log::Printf("BootLogo shutdown complete.");

                        g_state = GameState::Playing;
                        UpdatePlayingWindowTitle(hwnd, prCtx.currentScene);
                    }
                    break;
                }

                case GameState::StrPlayback: {
                    const bool ss0DirectRuntime =
                        PrSS0Scene0RuntimeDirect::RuntimeEnabled();
                    const bool ss0DirectOwnedCurrentScene =
                        ss0DirectRuntime &&
                        (prCtx.currentScene == PrSceneId::Scene0 ||
                         prCtx.currentScene == PrSceneId::Scene1);
                    // A direct SS0 scene must never enter the process-level
                    // legacy STR state.  The old state owns both the Win
                    // STR presenter and its shell-sized frame, so even a
                    // single render-only tick here recreates the mixed-shell
                    // frame.  SS0's translated MDEC/lifecycle owner runs from
                    // Playing; collapse this stale state before touching the
                    // host player and let the next tick continue there.
                    if (ss0DirectOwnedCurrentScene) {
                        static bool s_loggedDirectStrPlaybackSuppressed = false;
                        if (!s_loggedDirectStrPlaybackSuppressed) {
                            s_loggedDirectStrPlaybackSuppressed = true;
                            Log::Printf(
                                "PrMain direct SS0: stale StrPlayback state suppressed scene=%d; host STR presenter stopped",
                                static_cast<int>(prCtx.currentScene));
                        }
                        g_f1Pressed = false;
                        g_f1SkipRemote = false;
                        g_strPlayer.Stop();
                        g_state = GameState::Playing;
                        UpdatePlayingWindowTitle(hwnd, prCtx.currentScene);
                        break;
                    }
                    const bool legacyF1StrSkipAllowed =
                        !ss0DirectOwnedCurrentScene;
                    // PSX: Start/Select skip STR; F1 remains a non-direct debug hotkey.
                    bool skipRequest =
                        (legacyF1StrSkipAllowed && (g_f1Pressed || g_f1SkipRemote)) ||
                        IsStrSkipPadRequested(prCtx);
                    g_f1Pressed = false;
                    g_f1SkipRemote = false;

                    StrPlayerResult result = g_strPlayer.Update(skipRequest);

                    if (result == StrPlayerResult::Finished || result == StrPlayerResult::Skipped) {
                        Log::Printf("STR playback ended: %s",
                                    result == StrPlayerResult::Skipped ? "skipped" : "finished");
                        g_strPlayer.Stop();
                        g_state = g_stateAfterStr;
                        UpdatePlayingWindowTitle(hwnd, prCtx.currentScene);
                    } else if (result == StrPlayerResult::Error) {
                        Log::Printf("STR playback error: %s", g_strPlayer.GetError().c_str());
                        g_strPlayer.Stop();
                        g_state = g_stateAfterStr;
                        UpdatePlayingWindowTitle(hwnd, prCtx.currentScene);
                    }
                    break;
                }

                case GameState::Playing: {
                    prCtx.frame = (uint32_t)g_frameNum;
                    const bool ss0DirectRuntime =
                        PrSS0Scene0RuntimeDirect::RuntimeEnabled();
                    const bool ss0DirectOwnedCurrentScene =
                        ss0DirectRuntime &&
                        (prCtx.currentScene == PrSceneId::Scene0 ||
                         prCtx.currentScene == PrSceneId::Scene1);
                    prCtx.debugNextScene =
                        (g_nextScenePressed || g_autoNextSceneOnce) &&
                        !ss0DirectOwnedCurrentScene;
                    prCtx.debugGenericSwitch =
                        g_genericSwitchPressed && !ss0DirectOwnedCurrentScene;
                    prCtx.debugGenericEvent =
                        ss0DirectOwnedCurrentScene ? 0 : g_genericEvent;
                    prCtx.debugGenericEventArg =
                        ss0DirectOwnedCurrentScene ? 0 : g_genericEventArg;
                    // Debug hotkeys for minimal runnable game
                    const bool strPlaying = (prCtx.strPlayer && prCtx.strPlayer->IsPlaying());
                    const bool legacyF1StrSkipAllowed =
                        !ss0DirectOwnedCurrentScene;
                    const bool f1RemoteDeliver =
                        legacyF1StrSkipAllowed && g_f1SkipRemote && strPlaying;
                    if (f1RemoteDeliver) {
                        g_f1SkipRemote = false;
                    }
                    // PSX: Start/Select skip STR; F1 remains a non-direct debug hotkey.
                    prCtx.debugF1_StrSkip =
                        (legacyF1StrSkipAllowed && (g_f1Pressed || f1RemoteDeliver)) ||
                        IsStrSkipPadRequested(prCtx);
                    prCtx.debugEsc_Ev4Exit =
                        g_escExitPressed && !ss0DirectOwnedCurrentScene;
                    prCtx.debugF5_StageClear =
                        g_f5Pressed && !ss0DirectOwnedCurrentScene;
                    if (g_f6Pressed) {
                        if (!ss0DirectRuntime) {
                            PrEvent::DebugUnlockNextStage();
                        } else if (ss0DirectOwnedCurrentScene) {
                            const bool directOk =
                                PrSS0Scene0RuntimeDirect::DebugUnlockNextStage(prCtx);
                            if (!directOk) {
                                Log::Printf(
                                    "DebugStageUnlockNext direct failed scene=%u",
                                    (unsigned)prCtx.currentScene);
                            }
                        } else {
                            Log::Printf(
                                "DebugStageUnlockNext ignored outside SS0 direct scene=%u",
                                (unsigned)prCtx.currentScene);
                        }
                    }
                    if (g_f7Pressed) {
                        if (!ss0DirectRuntime) {
                            PrEvent::DebugFirstClearSelectableStages();
                        } else if (ss0DirectOwnedCurrentScene) {
                            const bool directOk =
                                PrSS0Scene0RuntimeDirect::DebugFirstClearSelectableStages(prCtx);
                            if (!directOk) {
                                Log::Printf(
                                    "DebugStageFirstClear direct failed scene=%u",
                                    (unsigned)prCtx.currentScene);
                            }
                        } else {
                            Log::Printf(
                                "DebugStageFirstClear ignored outside SS0 direct scene=%u",
                                (unsigned)prCtx.currentScene);
                        }
                    }
                    if (prCtx.debugFaceAutoShots) {
                        prCtx.debugFaceAutoShotTag.clear();
                    }
                    if (prCtx.debugPadInput == 0 && prCtx.currentScene == PrSceneId::Scene0) {
                        if (PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
                            prCtx.debugPadInput = g_keyboardPadMask;
                        } else {
                            const int dispEventId = PrEvent::GetDispEventIdPtr() ? *PrEvent::GetDispEventIdPtr() : 0;
                            if (!PrEvent::IsDispatcherRunning() || dispEventId != 16) {
                                prCtx.debugPadInput = g_keyboardPadMask;
                            }
                        }
                    }

                    // debugPadInput 已在 ConsumePadInput / Scene0 keyboard fallback 时设置
                    g_nextScenePressed = false;
                    g_genericSwitchPressed = false;
                    g_f1Pressed = false;
                    g_escExitPressed = false;
                    g_f5Pressed = false;
                    g_f6Pressed = false;
                    g_f7Pressed = false;
                    g_autoNextSceneOnce = false;
                    PrMain::Run(prCtx, prSceneTable);

                    if (prCtx.requestQuit) {
                        g_running = false;
                    }
                    // 每帧结束后清零 pad input，确保下帧能检测变化
                    prCtx.debugPadInput = 0;

                    static PrSceneId lastScene = (PrSceneId)0xFF;
                    if (prCtx.currentScene != lastScene) {
                        lastScene = prCtx.currentScene;

                        g_playingTextureName.clear();
                        g_playingTextureWidth = 0;
                        g_playingTextureHeight = 0;
                        g_playingTextureNames = g_resources.GetTextureNames();
                        g_playingTextureIndex = -1;

                        UpdatePlayingWindowTitle(hwnd, prCtx.currentScene);
                    }

                    if (g_texViewEnabled && g_playingTextureName.empty() && !g_playingTextureNames.empty()) {
                        int bestArea = -1;
                        for (int i = 0; i < (int)g_playingTextureNames.size(); i++) {
                            const std::string& name = g_playingTextureNames[i];
                            TextureResource* res = g_resources.GetTexture(name);
                            if (!res) continue;
                            const int w = res->tim.width;
                            const int h = res->tim.height;
                            if (w <= 0 || h <= 0) continue;
                            const int area = w * h;
                            if (area > bestArea) {
                                bestArea = area;
                                g_playingTextureName = name;
                                g_playingTextureWidth = w;
                                g_playingTextureHeight = h;
                                g_playingTextureIndex = i;
                            }
                        }

                        if (!g_playingTextureName.empty()) {
                            Log::Printf("Playing texture: '%s' (%dx%d)",
                                        g_playingTextureName.c_str(),
                                        g_playingTextureWidth,
                                        g_playingTextureHeight);
                        } else {
                            Log::Printf("Playing texture: (none)");
                        }

                        UpdatePlayingWindowTitle(hwnd, prCtx.currentScene);
                    }

                    if (g_texViewEnabled && (g_prevTexPressed || g_nextTexPressed) && !g_playingTextureNames.empty()) {
                        const int count = (int)g_playingTextureNames.size();
                        if (g_playingTextureIndex < 0) {
                            g_playingTextureIndex = 0;
                        }
                        if (g_prevTexPressed) {
                            g_playingTextureIndex = (g_playingTextureIndex - 1 + count) % count;
                        }
                        if (g_nextTexPressed) {
                            g_playingTextureIndex = (g_playingTextureIndex + 1) % count;
                        }

                        g_playingTextureName = g_playingTextureNames[g_playingTextureIndex];
                        if (TextureResource* res = g_resources.GetTexture(g_playingTextureName)) {
                            g_playingTextureWidth = res->tim.width;
                            g_playingTextureHeight = res->tim.height;
                        } else {
                            g_playingTextureWidth = 0;
                            g_playingTextureHeight = 0;
                        }
                        Log::Printf("Playing texture: '%s' (%dx%d)",
                                    g_playingTextureName.c_str(),
                                    g_playingTextureWidth,
                                    g_playingTextureHeight);
                        UpdatePlayingWindowTitle(hwnd, prCtx.currentScene);
                    }

                    g_prevTexPressed = false;
                    g_nextTexPressed = false;
                    break;
                }

                case GameState::Exit:
                    g_running = false;
                    break;
            }

            } // end doLogicTick

            // Set render-only flag for UI animation gating
            prCtx.renderOnlyFrame = !doLogicTick;
            if (!g_render60fps || doLogicTick) {
                prCtx.renderSubFrame8 = 0;
            } else {
                prCtx.renderSubFrame8 = 128;
            }

            // Only advances an already-owned SaveUi action continuation.
            // No gameplay, pad dispatch, CD clock or card I/O on redraws.
            PrStage1SaveUiHostBridgeDirect::PumpSaveUiPresentation19148(prCtx);

            // Render based on state
            static bool s_loggedScene1PlayingRender = false;
            static bool s_loggedScene1StrPlaybackRender = false;
            switch (g_state) {
                case GameState::BootLogo:
                    g_renderer.BeginFrame(0.0f, 0.0f, 0.0f);
                    g_bootLogo.Render();
                    g_renderer.EndFrame();
                    break;

                case GameState::StrPlayback:
                    if (!s_loggedScene1StrPlaybackRender && prCtx.currentScene == PrSceneId::Scene1 && prCtx.strPlayer && prCtx.strPlayer->IsPlaying()) {
                        s_loggedScene1StrPlaybackRender = true;
                    }
                    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
                        (prCtx.currentScene == PrSceneId::Scene0 ||
                         prCtx.currentScene == PrSceneId::Scene1)) {
                        // Render-only frames can observe the stale enum before
                        // the logic tick above collapses it.  Keep the frame
                        // on the exact SS0 direct presentation path and never
                        // call StrPlayer::Render(), which is the former Win
                        // S0 shell owner.
                        g_renderer.BeginFrame(0.1f, 0.1f, 0.1f);
                        const PrSceneDef& sceneDef =
                            prSceneTable.Get(prCtx.currentScene);
                        if (PrSS0Scene0RuntimeDirect::IsResidentDirectoryActive80015788()) {
                            PrSS0Scene0RuntimeDirect::Render(prCtx);
                        } else if (sceneDef.render) {
                            sceneDef.render(prCtx);
                        }
                        g_renderer.FlushSprites();
                        if (!PrSS0Scene0RuntimeDirect::IsResidentDirectoryActive80015788())
                            PrUiOverlay::Render(prCtx);
                        if (prCtx.currentScene == PrSceneId::Scene0) {
                            PrSS0Scene0RuntimeDirect::RenderLateSubtitles(prCtx);
                        }
                        if (!PrSS0Scene0RuntimeDirect::IsResidentDirectoryActive80015788())
                            PrStage1HdSubtitles::Render(prCtx);
                        g_renderer.FlushSprites();
                        PrTransition::Render(prCtx);
                        RenderStage1PsxFrameDebugOverlay(prCtx);
                        g_renderer.EndFrame();
                        break;
                    }
                    g_renderer.BeginFrame(0.0f, 0.0f, 0.0f);
                    g_strPlayer.Render();
                    g_renderer.EndFrame();
                    break;

                case GameState::Playing: {
                    if (!s_loggedScene1PlayingRender && prCtx.currentScene == PrSceneId::Scene1 && prCtx.strPlayer && prCtx.strPlayer->IsPlaying()) {
                        s_loggedScene1PlayingRender = true;
                    }
                    g_renderer.BeginFrame(0.1f, 0.1f, 0.1f);

                    // 调用当前场景的 render 回调（方案A）
                    const PrSceneDef& sceneDef = prSceneTable.Get(prCtx.currentScene);
                    if (PrSS0Scene0RuntimeDirect::IsResidentDirectoryActive80015788()) {
                        PrSS0Scene0RuntimeDirect::Render(prCtx);
                    } else if (sceneDef.render) {
                        sceneDef.render(prCtx);
                    }

                    g_renderer.FlushSprites();

                    // The overlay owns its own direct-scene gate. Calling it
                    // here also exposes active Stage1 SaveUi event packets.
                    if (!PrSS0Scene0RuntimeDirect::IsResidentDirectoryActive80015788())
                        PrUiOverlay::Render(prCtx);

                    // Texture viewer overlay for debugging.
                    if (g_texViewEnabled && !g_playingTextureName.empty()) {
                        ID3D11ShaderResourceView* srv = g_resources.GetTextureView(g_playingTextureName);
                        if (srv && g_playingTextureWidth > 0 && g_playingTextureHeight > 0) {
                            const float texW = (float)g_playingTextureWidth;
                            const float texH = (float)g_playingTextureHeight;
                            const float winW = (float)g_renderer.GetWidth();
                            const float winH = (float)g_renderer.GetHeight();

                            const float fitX = winW / texW;
                            const float fitY = winH / texH;
                            const float scale = std::min(fitX, fitY);

                            const float w = texW * scale;
                            const float h = texH * scale;
                            const float x = (winW - w) * 0.5f;
                            const float y = (winH - h) * 0.5f;
                            D3D11Renderer::SpriteCmd cmd;
                            cmd.texture = srv;
                            cmd.x = x;
                            cmd.y = y;
                            cmd.w = w;
                            cmd.h = h;
                            cmd.u0 = 0.0f;
                            cmd.v0 = 0.0f;
                            cmd.u1 = 1.0f;
                            cmd.v1 = 1.0f;
                            cmd.r = 1.0f;
                            cmd.g = 1.0f;
                            cmd.b = 1.0f;
                            cmd.a = 1.0f;
                            cmd.blend = D3D11Renderer::BlendMode::Alpha;
                            cmd.layer = 900;
                            cmd.order = 0;
                            g_renderer.SubmitSprite(cmd);
                        }
                    }

                    if (prCtx.currentScene == PrSceneId::Scene0) {
                        PrSS0Scene0RuntimeDirect::RenderLateSubtitles(prCtx);
                    }
                    if (!PrSS0Scene0RuntimeDirect::IsResidentDirectoryActive80015788())
                        PrStage1HdSubtitles::Render(prCtx);

                    g_renderer.FlushSprites();

                    // 过渡层（Scene0→Scene1 等切场时的 fade 效果）
                    PrTransition::Render(prCtx);
                    RenderStage1PsxFrameDebugOverlay(prCtx);

                    g_renderer.EndFrame();
                    break;
                }

                default:
                    g_renderer.BeginFrame(0.1f, 0.1f, 0.1f);
                    g_renderer.EndFrame();
                    break;
            }

            // Auto-screenshot at specific frames
            if (g_autoScreenshotIndex < 3 && g_frameNum == g_autoScreenshotFrames[g_autoScreenshotIndex]) {
                std::filesystem::path filename = MakeScreenshotFilePath();
                Log::Printf("Auto screenshot frame=%d", g_frameNum);
                bool shotOk = g_renderer.SaveScreenshot(filename.wstring());
                Log::Printf("Auto screenshot ok=%d", shotOk ? 1 : 0);
                g_autoScreenshotIndex++;
            }

            // Handle manual screenshot request (F12)
            if (g_screenshotRequested) {
                g_screenshotRequested = false;
                std::filesystem::path filename = MakeScreenshotFilePath();
                Log::Printf("Manual screenshot frame=%d", g_frameNum);
                if (g_renderer.SaveScreenshot(filename.wstring())) {
                    SetWindowTextW(hwnd, L"PaRappa - Screenshot saved!");
                    Log::Printf("Manual screenshot ok=1");
                } else {
                    Log::Printf("Manual screenshot ok=0");
                }
            }

            while (!g_pendingShotPaths.empty()) {
                std::filesystem::path filename = g_pendingShotPaths.front();
                g_pendingShotPaths.pop();
                Log::Printf("Remote screenshot frame=%d file=%s", g_frameNum, filename.u8string().c_str());
                bool shotOk = g_renderer.SaveScreenshot(filename.wstring());
                Log::Printf("Remote screenshot ok=%d", shotOk ? 1 : 0);
            }

            if (prCtx.debugFaceAutoShots && prCtx.debugFaceAutoShotPending) {
                prCtx.debugFaceAutoShotPending = false;
                std::filesystem::path filename = MakeFaceAutoShotFilePath(prCtx.frame, prCtx.debugFaceAutoShotTag);
                Log::Printf("FaceAutoShot frame=%u tag=%s", (unsigned)prCtx.frame, prCtx.debugFaceAutoShotTag.c_str());
                bool shotOk = g_renderer.SaveScreenshot(filename.wstring());
                Log::Printf("FaceAutoShot ok=%d", shotOk ? 1 : 0);
                prCtx.debugFaceAutoShotTag.clear();
            }
        } else {
            Sleep(1);
        }
    }

    // Cleanup
    DebugServer::Shutdown();
    PrUiOverlay::Shutdown();
    PrStage1HdSubtitles::ClearCache();
    g_strPlayer.Shutdown();
    g_bootLogo.Shutdown();
    g_resources.Clear();
    g_renderer.Shutdown();
    DestroyWindow(hwnd);

    Log::Printf("PaRappaWin exit");

    timeEndPeriod(1);

    return 0;
    } catch (const std::exception& e) {
        Log::Printf("FATAL: std::exception: %s", e.what());
        Log::Shutdown();
        return 1;
    } catch (...) {
        Log::Printf("FATAL: unknown exception");
        Log::Shutdown();
        return 1;
    }
}
