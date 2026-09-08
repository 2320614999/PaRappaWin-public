#include "pr_ss0_scene0_runtime_direct.h"

#include "d3d11_renderer.h"
#include "logger.h"
#include "pr_game_context.h"
#include "pr_movie_segment_direct.h"
#include "pr_pad.h"
#include "pr_psx_event_frame_direct.h"
#include "pr_psx_graph_owner_direct.h"
#include "pr_psx_text_glyph_metrics_direct.h"
#include "pr_psx_pad_direct.h"
#include "pr_psx_sprite_template_render.h"
#include "pr_scene_bootstrap_direct.h"
#include "pr_scene_entry_card_feedback_direct.h"
#include "pr_scene_entry_direct.h"
#include "pr_scene_entry_executor_direct.h"
#include "pr_sfx.h"
#include "pr_stage1_movie_text_direct.h"
#include "pr_ss0_direct.h"
#include "pr_ss0_resource_audio_direct.h"
#include "pr_ss0_card_memcard_handoff_direct.h"
#include "pr_ss0_card_image_storage_direct.h"
#include "pr_ss0_card_io_banner_render_direct.h"
#include "pr_ss0_directory_dispatcher_direct.h"
#include "pr_ss0_directory_pages_render_direct.h"
#include "pr_ss0_event_frame_loop_direct.h"
#include "pr_ss0_event_text_direct.h"
#include "pr_ss0_practice_lifecycle_direct.h"
#include "pr_ss0_event_backdrop_render_direct.h"
#include "pr_ss0_hiscore_render_direct.h"
#include "pr_ss0_scene0_int_gpu_direct.h"
#include "pr_ss0_scene0_int_renderer_direct.h"
#include "pr_ss0_scene0_int_load_direct.h"
#include "pr_ss0_scene0_int_side_effect_direct.h"
#include "pr_ss0_scene0_int_spu_direct.h"
#include "pr_ss0_scene0_resource_ingress_direct.h"
#include "pr_ss0_scene0_global_bindings_direct.h"
#include "pr_ss0_scene0_shared_event_predispatch_direct.h"
#include "pr_ss0_stage_progress_bank_direct.h"
#include "pr_ss0_state16_runtime_one_shot_direct.h"
#include "pr_ss0_str_lifecycle_direct.h"
#include "pr_ss0_mdec_output_direct.h"
#include "pr_ss0_title_entry_prefix_direct.h"
#include "pr_ss0_title_initial_clock_direct.h"
#include "pr_ss0_title_packet_render_direct.h"
#include "pr_ss0_title_primitive_group_direct.h"
#include "pr_ss0_transition_direct.h"
#include "pr_ss0_title_draw_backend.h"
#include "pr_ui_overlay.h"
#include "pr_ss0_title_hud_events_direct.h"
#include "pr_ss0_title_tmd_backend.h"
#include "pr_vram_atlas.h"
#include "pr_stage1_save_card_hal_direct.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_stage_runner.h"
#include "pr_stage_scene_submit_backend.h"
#include "pr_stage1_lifecycle_executor_direct.h"
#include "pr_stage1_lifecycle_host_adapter_801c81ec.h"
#include "pr_transition.h"
#include "resource_manager.h"
#include "audio_engine.h"
#include "str_player.h"
#include "xa_decoder.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace PrSS0Scene0RuntimeDirect {
namespace {

namespace CardHandoff = PrSS0CardMemcardHandoffDirect;

static const int kAutoMenuFrame = 370;
static const int kPracticeRoundCount = 4;
static const int kPracticeRoundFrames = 150;
static const int kPracticeBeatFrames = 75;
static const int kPracticePreviewStartFrame = 112;
static const int kPracticePreviewEndFrame = 149;
static const int kPracticeFastForwardFrame = 135;
static constexpr uint32_t kScene0Movie0VtextDesc801C6BF8 = 0x801C6BF8u;
static constexpr uint32_t kScene0Movie0VtextEntryCount = 15u;
static bool s_word800916F0DiscFullbootStartupPublisherDisabledForProbe = false;
// Keep the historical disc/fullboot diagnostic from manufacturing a memory
// observation. Native execution now owns a separate translated BSS word; its
// startup zero never passes through this external-observation publisher.
static constexpr bool kRequirePsvMemoryReplayForWord800916F0 = true;
static constexpr uint32_t kProcessStartupZeroBegin80028590 = 0x8006ECB8u;
static constexpr uint32_t kProcessStartupZeroEnd80028590 = 0x801C3870u;
static constexpr uint32_t kWord800916F6PsxAddress = 0x800916F6u;
static constexpr uint32_t kWord800916FCPsxAddress = 0x800916FCu;

struct Word800916F6RuntimeState {
    bool known = false;
    uint16_t value = 0u;
    Word800916F6AuthoritySource source =
        Word800916F6AuthoritySource::Unknown;
};

struct Word800916FCRuntimeState {
    bool known = false;
    uint16_t value = 0u;
    Word800916FCAuthoritySource source =
        Word800916FCAuthoritySource::Unknown;
};

static Word800916F6RuntimeState s_word800916F6;
static Word800916FCRuntimeState s_word800916FC;
static const int kPracticeTargetOffsets[kPracticeRoundCount] = {0, 19, 37, 56};
static const int kPracticeRoundIntroFrames[kPracticeRoundCount] = {80, 46, 42, 49};
static constexpr int kPracticeRec44CursorLimit = 19;
// 80024418 initializes only the PSX sprite attribute/geometry and then lets
// the source CLUT provide the note colour.  The direct host path represents
// that neutral texture colour with the PSX 0x80-per-channel modulation; using
// the event id (16) here incorrectly zeroed the green/blue channels.
static constexpr uint8_t kPracticeIconNeutralRgb80024418 = 0x80u;
// 8003FA20 inserts same-priority sprites at the head of the PSX OT bucket,
// so later pseudo-C calls are drawn first and earlier calls remain on top.
// Keep explicit host orders for every Practice producer; otherwise a high
// header order would make subsequent auto-assigned orders wrap over it.
static constexpr uint64_t kPracticeOtOrderTail = 1000u;
static constexpr uint64_t kPracticeOtOrderSlices = 2000u;
static constexpr uint64_t kPracticeOtOrderPortraits = 3000u;
static constexpr uint64_t kPracticeOtOrderLeading = 4000u;
static constexpr uint64_t kPracticeOtOrderGuide = 5000u;
static constexpr uint64_t kPracticeOtOrderIcons = 6000u;
static constexpr uint8_t kPracticeEventId8001E750 = 16u;
static const int kPracticeResultPadStopFrames = 75;
static const int kPracticeCompletePromptFrames = 88;
static const int kPracticeExitPromptFrames = 95;
static const int kPracticeCompletePromptJudge = 7;
static const int kPracticeExitPromptJudge = 8;
static const int kPracticeExitConfirmFrames = static_cast<int>(
    PrSS0PracticeLifecycleDirect::kPracticeExitConfirmFrames);
static constexpr std::size_t kStageStatusPayloadBase = 0x0Cu;
static constexpr std::size_t kDirectCardImageBytes8007A318 = 128u * 1024u;
static constexpr std::size_t kSaveUiDirectoryRawRowBytes8007A318 = 40u;
static constexpr std::size_t kSaveUiDirectoryRawNameBytes8007A318 = 20u;
static constexpr std::size_t kSaveUiDirectoryRawBankBytes8007A318 =
    15u * kSaveUiDirectoryRawRowBytes8007A318;
static constexpr char kCardFilenamePrefix80019D7C[] = "BASCUS-94183";
static constexpr std::size_t kCardFilenamePrefixBytes80019D7C =
    sizeof(kCardFilenamePrefix80019D7C) - 1u;

enum class SS0DirectTypedCardReadLane : uint8_t {
    State16LoadPayload80019D7C,
    Case17HiScorePayload80019D7C,
};

struct SS0DirectTypedCardReadAuthority {
    bool producerWired800173A8_80016EB8_800179B4 = false;
    PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4
        carrierSource =
            PrStage1SaveCardHalDirect::
                CardReadTypedCarrierSource800179B4::Unknown;
    bool state16LoadPayloadLaneKnown = false;
    bool case17HiScorePayloadLaneKnown = false;
    bool state16SelectedBlockKnown = false;
    int state16SelectedBlock = -1;
    bool typedReadSuccessKnown800179B4 = false;
    bool payloadBytesKnown8007ADE8 = false;
    bool incomplete800179B4 = false;
};

struct SS0DirectTypedPayloadCommitSeam {
    bool known = false;
    int blockIndex = -1;
    std::size_t byteCount = 0;
    bool runtimeLowerCardProducerKnown = false;
    bool typedReadSuccessKnown800179B4 = false;
    bool payloadBytesKnown8007ADE8 = false;
    uint32_t payloadAddress8007ADE8 =
        PrStagePayloadBankDirect::kTypedPayloadSourceAddress8007ADE8;
    PrStagePayloadBankDirect::LoadSavePayloadAuthority800164B4
        payloadAuthority800164B4{};
    std::array<uint8_t, PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4>
        bytes{};
};

struct SS0DirectDebugReplayLoadResult {
    bool payloadCommitted = false;
    int replayScene = -1;
    bool rawSavedSlotKnown = false;
    uint32_t rawSavedSlot = 0;
};

enum class SS0DirectPhase : uint8_t {
    TitleIntroTransition,
    TitleMovie0TFinalReady,
    TitleSelector,
    TitleExitWait,
    TitleExitTransition,
    MainMenuEntryTransition,
    MainMenu,
    Options,
    StageSelect,
    Practice,
    HiScore,
    ReplayCard,
    LoadCard,
    WaitTransition,
    OpeningMovie0,
    OpeningMovie0PreTransition,
    OpeningMovie0PostTransition,
    OpeningMovie0InitialTransition,
    MainMenuResourceLoading,
};

// The translated transition bodies keep the original PSX call/return cadence.
// A host-facing Loading page must nevertheless remain observable for at least
// one second (30 Scene0 logic ticks) before its completed tail is released.
// This hold is deliberately outside the pseudo-C runtime counters so it does
// not change 80020110/800201AC iteration thresholds or video/subtitle clocks.
enum class SS0DirectLoadingHoldKind : uint8_t {
    None = 0,
    OpeningMovie0Initial,
    OpeningMovie0Pre,
    OpeningMovie0Post,
    TitleIntro,
    TitleExit,
    MainMenuEntry,
    SceneHandoff,
    MainMenuReload,
};

enum class SS0DirectPracticePhase : uint8_t {
    Idle,
    RoundIntro,
    Preview,
    Judge,
    ResultPadStop,
    CompletePrompt,
    ExitPrompt,
    ExitConfirm,
};

enum class SS0DirectTitleEarlyInputShortcutPhase : uint8_t {
    None = 0,
    Waiting15WaitCalls,
    AwaitingPixelSubmit,
    WaitingPadRelease,
    Blocked,
};

enum class SS0DirectLoadReplayCompletionKind80019D7C : uint8_t {
    None = 0,
    LoadReturnMainMenu,
    ReplayScene,
};

struct SS0DirectRuntimeState {
    bool initialized = false;
    PrSceneBootstrapDirect::BootstrapGraphRuntime8001C470
        bootstrapGraphRuntime8001C470{};
    PrSceneBootstrapDirect::ResetGraphState800446A0
        resetGraph800446A0{};
    PrSceneBootWorkListDirect::WorkListDrawBufferInit8001E6D0
        startupWorkList8001E6D0{};
    PrPsxPadDirect::PadInitState800354C0
        padInit800354C0{};
    bool word800916D2Known = false;
    uint16_t word800916D2 = 0u;
    PrSS0Scene0GlobalBindingsDirect::State801C5B14
        scene0GlobalBindings{};
    PrSS0Scene0GlobalBindingsDirect::InitTransaction801C4260
        scene0Init801C4260{};
    bool scene0SubtitleDataLoaded = false;
    bool scene0SubtitleDataAvailable = false;
    bool scene0SubtitleRenderLogged = false;
    bool scene0SubtitlePacketSubmitLogged = false;
    uint32_t scene0SubtitlePacketSubmitCount = 0u;
    uint32_t scene0SubtitlePacketLastTextPsxAddr = 0u;
    uint32_t scene0SubtitleEventCount = 0u;
    PrStage1MovieTextDirect::Movie1TextRuntime scene0Movie0Vtext{};
    // COMOD0 80024C84/80024CF8 owns the opening movie text cursor and
    // duration countdown.  Keep this separate from the Stage1 common-lyrics
    // runtime embedded in Movie1TextRuntime; borrowing that state would mix
    // Scene0 and Scene1 text ownership.
    PrStage1VTextDirectRuntime scene0SubtitleVtextRuntime{};
    PrSS0ResourceAudioDirect::StartupAudioBindings80016C8C
        bootStartupAudioBindings80016C8C{};
    bool bootStartupAudioBindingsCommitted80016C8C = false;
    PrSfx::SharedAudioResetBarrierResult26FA4
        bootStartupAudioReset80026FA4{};
    bool bootStartupAudioResetCommitted80026FA4 = false;
    PrSS0Scene0ResourceIngressDirect::StartupPreloadTransaction80016B84
        bootStartupPreloadIngress80016B84{};
    PrSS0Scene0IntLoadDirect::Transaction8001AC18
        bootStartupCommonIntLoad80016B84{};
    PrSS0Scene0IntLoadDirect::Transaction8001AC18
        bootStartupZCompoIntLoad80016B84{};
    PrSfx::Scene0VabCloseResult80027120
        bootStartupVabClose80027120{};
    bool bootStartupVabCloseCommitted80027120 = false;
    bool bootStartupDiscIntPrepared80016B84 = false;
    bool bootStartupPreparedBeforeFirstLogo80016B84 = false;
    uint32_t bootStartupPrepareFrame80016B84 = 0u;
    uint32_t bootStartupCommonAttemptCount8001AC18 = 0u;
    uint32_t bootStartupZCompoAttemptCount8001AC18 = 0u;
    PrSS0Scene0ResourceIngressDirect::Transaction801C4780
        scene0ResourceIngress801C4780{};
    PrSS0Scene0IntLoadDirect::Transaction8001AC18
        startupCommonIntLoad80016B84{};
    PrSS0Scene0IntLoadDirect::Transaction8001AC18
        startupZCompoIntLoad80016B84{};
    PrSS0Scene0IntLoadDirect::Transaction8001AC18
        scene0IntLoad8001AC18{};
    PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0
        scene0IntSideEffect8001A8F0{};
    bool scene0IntSideEffectGateCommitted = false;
    PrSS0Scene0IntSpuDirect::Transaction8001A8F0
        scene0IntSpu8001A8F0{};
    bool scene0IntSpuGateCommitted = false;
    PrSS0Scene0IntGpuDirect::Transaction8001AE7C
        scene0IntGpu8001AE7C{};
    bool scene0IntGpuGateCommitted = false;
    PrSS0Scene0IntRendererDirect::Transaction8001AE7C
        scene0IntRenderer8001AE7C{};
    bool scene0IntRendererGateCommitted = false;
    PrSS0Scene0IntLoadDirect::Transaction8001AC18
        practiceYCompoIntLoad80015618{};
    PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0
        practiceYCompoIntSideEffect8001A8F0{};
    bool practiceYCompoLoaderMemoryGateCommitted = false;
    PrSS0Scene0IntSpuDirect::PadStartComSource80026E4C
        practiceYCompoPadStartComSource{};
    PrSS0Scene0IntSpuDirect::Transaction8001A8F0
        practiceYCompoIntSpu8001A8F0{};
    PrSS0Scene0IntGpuDirect::Transaction8001AE7C
        practiceYCompoIntGpu8001AE7C{};
    PrSS0Scene0IntRendererDirect::Transaction8001AE7C
        practiceYCompoIntRenderer8001AE7C{};
    bool practiceYCompoResourcesCommitted = false;
    bool strStarted = false;
    // MOVIE0/MOVIE0T carry one MDEC picture on roughly every other 30 Hz
    // host logic tick (the source STRs decode at ~15 fps).  Keep this gate
    // in the direct owner so the optional 60 Hz render cadence cannot make
    // the movie run at 2x speed.  The title model/event loop itself remains
    // 30 Hz; this gate is only for the MDEC picture source.
    uint8_t strVideoFrameCadenceTick = 1u;
    bool strVideoCadenceLogged = false;
    uint32_t strTitleAudioTargetCursor = 0u;
    // 80024CF8 reads the prior 801C4350 mm:ss:ff fields.  Preserve that
    // 30 fps query frame independently from the 15 fps MDEC picture cadence.
    uint32_t strSubtitleQueryFrame30 = 0u;
    uint32_t strSubtitleClockFrame30 = 0u;
    PrSS0StrLifecycleDirect::StrDecoderAllocationState80027288
        strDecoderAllocation80027288{};
    PrSS0StrLifecycleDirect::StrDecoderMemoryRuntime80027288
        strDecoderMemoryRuntime80027288{};
    PrSS0StrLifecycleDirect::StrStreamStartState800274D4
        strStreamStart800274D4{};
    PrSS0StrLifecycleDirect::StrLowerCdStartRuntime8001A4D0
        strLowerCdStart8001A4D0{};
    PrSS0StrLifecycleDirect::StrXaAudioDiscRuntime8001A4D0
        strXaAudioDisc8001A4D0{};
    XaDecoder strXaAudioDecoder{};
    int strXaAudioVoice = -1;
    uint64_t strXaAudioVoiceGeneration = 0u;
    uint32_t strXaAudioSampleRate = 0u;
    uint32_t strXaAudioChannels = 0u;
    uint8_t strXaAudioCoding = 0u;
    uint32_t strXaAudioDecodedSectors = 0u;
    uint64_t strXaAudioQueuedSamples = 0u;
    float strXaAudioOutputVolume = 1.0f;
    bool strXaAudioOutputAdapterKnown = false;
    bool strXaAudioOutputAdapterActive = false;
    PrSS0StrLifecycleDirect::StrDecodeAdvanceState800273A4
        strDecodeAdvance800273A4{};
    PrSS0StrLifecycleDirect::StrVlcDecodeReleaseResult800273A4
        strVlcDecodeRelease800273A4{};
    PrSS0StrLifecycleDirect::StrMdecCommandOutputSubmission800273A4
        strMdecCommandOutput800273A4{};
    PrSS0MdecOutputDirect::Mdec15bppResult
        strMdecOutputCurrentScus{};
    PrSS0StrLifecycleDirect::StrMdecDmaCompletionState80027220
        strMdecCompletion80027220{};
    bool strMdecCompletionPending = false;
    uint32_t strDirectMdecFramesDecoded = 0;
    uint32_t strDirectMdecLastFrameNumber = 0;
    bool strDirectMdecStreamExhausted = false;
    PrSS0StrLifecycleDirect::StrDecodeGateState80027528
        strDecodeGate80027528{};
    PrSS0StrLifecycleDirect::StrStripUploadState8002756C
        strStripUpload8002756C{};
    PrSS0StrLifecycleDirect::StrStripUploadExecution8002756C
        strStripUploadExecution8002756C{};
    PrSS0StrLifecycleDirect::StrVramPageRuntime8001B120
        strVramPageRuntime8001B120 =
            PrSS0StrLifecycleDirect::InitializeStrVramPageRuntime8001B120();
    PrSS0StrLifecycleDirect::StrVramPagePublishResult8002756C
        strVramPagePublish8002756C{};
    PrSS0StrLifecycleDirect::StrDisplayMoveResult8001B120
        strDisplayMove8001B120{};
    PrSS0StrLifecycleDirect::StrOuterPostWaitResult801C4DC4
        strOuterPostWait801C4DC4{};
    PrSS0TitleTmdBackend::TitleMovie0TState0GraphFlipResult801C4B8C
        openingMovie0GraphFlip8001ED74{};
    PrSS0TitleTmdBackend::OpeningMovie0WorkListFlushResult8001ED3C
        openingMovie0WorkListFlush8001ED3C{};
    bool scene0Gp368WorkSlot8006EDA8Known = true;
    uint8_t scene0Gp368WorkSlot8006EDA8 = 0u;
    bool scene0Gp368LoadedZeroAuthority = true;
    ID3D11ShaderResourceView* strDirectMdecTexture = nullptr;
    D3D11Renderer* strDirectMdecTextureRenderer = nullptr;
    bool strDirectMdecTextureReady = false;
    uint32_t strDirectMdecTextureFrameNumber = 0u;
    uint32_t strDirectMdecTextureUploadCount = 0u;
    PrSS0StrLifecycleDirect::StrClockPollState801C4350
        strClockPoll801C4350{};
    PrSS0StrLifecycleDirect::StrLowerCdClockRuntime801C4350
        strLowerCdClock801C4350{};
    PrSS0StrLifecycleDirect::StrWorkBaseRuntime80049428
        strWorkBase80049428{};
    PrSS0StrLifecycleDirect::StrPlayerTick801C448C
        strPlayerTick801C448C{};
    PrSS0StrLifecycleDirect::StrPostUploadDecision801C455C
        strPostUploadDecision801C455C{};
    PrSS0StrLifecycleDirect::StrCdSyncCallbackRuntime800570F8
        strCdSyncCallback800570F8 =
            PrSS0StrLifecycleDirect::
                InitializeStrCdSyncCallbackRuntime800570F8();
    PrSS0StrLifecycleDirect::StrWaitCleanupResult8001A694
        strWaitCleanup8001A694{};
    PrSS0StrLifecycleDirect::StrCleanupTailPrefixResult801C455C
        strCleanupTailPrefix801C455C{};
    PrSS0StrLifecycleDirect::StrRootCallbackRuntime80055F78
        strRootCallback80055F78 =
            PrSS0StrLifecycleDirect::
                InitializeStrRootCallbackRuntime80055F78();
    PrSS0StrLifecycleDirect::StrRootCallbackResetResult800358DC
        strRootCallbackReset800358DC{};
    PrSS0StrLifecycleDirect::StrStopCallbackResult80035838
        strStopCallback80035838{};
    PrSS0StrLifecycleDirect::StrStopResetState80027664
        strStopReset80027664{};
    PrSS0TransitionDirect::SlowTransitionRuntime80020110
        openingMovie0InitialTransition{};
    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110
        openingMovie0InitialTransitionVisual{};
    bool openingMovie0InitialTransitionPresentBlockedLogged = false;
    PrSS0TransitionDirect::FastTransitionRuntime800201AC
        openingMovie0Transition{};
    PrSS0TransitionDirect::FastTransitionVisualFrame800201AC
        openingMovie0TransitionVisual{};
    bool openingMovie0TransitionPresentBlockedLogged = false;
    bool openingMovie0FrozenFrameHeld = false;
    PrSS0TransitionDirect::SlowTransitionRuntime80020110
        titleIntroTransition{};
    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110
        titleIntroTransitionVisual{};
    bool titleIntroTransitionPresentBlockedLogged = false;
    uint16_t titleMovie0TFinalReadyPollsRemaining = 0u;
    uint8_t titleMovie0TFinalReadyInterPollTicksRemaining = 0u;
    // COMOD0 801C4894's 801C4968 pre-loop keeps the title packet lane
    // visible while 8001A750 is not ready.  This is separate from
    // strStarted: the STR may already be decoding, but MOVIE0T must not be
    // exposed until the pre-loop exits.
    bool titleMovie0TFinalReadyPreloopActive = false;
    bool titleMovie0TFinalReadyBlockedLogged = false;
    bool titleMovie0TState0GraphFlipBlockedLogged = false;
    bool menuUnsupportedLogged = false;
    SS0DirectPhase phase = SS0DirectPhase::TitleSelector;
    SS0DirectPhase transitionReturnPhase = SS0DirectPhase::TitleSelector;
    int titleCursor = 0;
    uint16_t titleSelectorPreviousInputPsx = 0;
    // COMOD0 801C4894 keeps a separate v9 copy of the complete
    // 80035510 mask while MOVIE0T is in title-loop states 0/1.  The
    // selector edge state above is intentionally not reused here: it is
    // only the v8=2 cursor edge and would drop Start/Select/shoulder bits
    // from the original early-input comparison.
    uint32_t titleMovie0TPreviousInputPsx = 0u;
    int titleFrameCounter = 0;
    int32_t titleLoopTickV10 = -17;
    uint8_t titleLoopStateV8 = 0;
    // COMOD0 stops the MDEC/CD stream at the v8=0 -> v8=1 boundary, but the
    // STR XA tail is already part of the same playback and must not be cut
    // with the last visible video frame.  Keep this host-side lifetime bit
    // separate from strStarted (which owns picture visibility).
    bool titleMovie0TAudioTailPending = false;
    bool titleMovie0TAudioTailReleaseLogged = false;
    PrSS0TitleHudEventsDirect::TitleNaturalLoopCadenceState801C4894
        titleNaturalLoopCadence{};
    PrSS0TitleInitialClockDirect::Result801C49C0 titleInitialClock{};
    bool titleInitialCtxTick96Known = false;
    uint32_t titleInitialCtxTick96 = 0;
    bool titleCtxTick96Known = false;
    uint32_t titleCtxTick96 = 0;
    bool titleCtxBeatKnown = false;
    uint8_t titleCtxBeat = 0;
    PrSS0TitleHudEventsDirect::TitleSharedEventBucketState80024FD0
        titleSharedEventBucket{};
    PrSS0Scene0SharedEventPredispatchDirect::ContextState80024FD0
        titleSharedEventPredispatchContext{};
    bool titleSharedEventPredispatchLogged = false;
    PrSS0Scene0SharedEventPredispatchDirect::StaticDispatchState80024FD0
        titleSharedEventDispatch{};
    bool titleSharedEventDispatchLogged = false;
    PrSS0TitleHudEventsDirect::TitleAbsoluteEventStreamState801C5538
        titleAbsoluteEventStream{};
    PrSS0TitleHudEventsDirect::TitleEventRuntimeState801C5190
        titleEventRuntimeState{};
    PrSS0TitlePacketRenderDirect::HostVisibleTitleFrameCache801C689C
        titleHostVisibleFrameCache{};
    PrSS0TitleHudEventsDirect::TitleSelectorHudRuntimeState801C5854
        titleSelectorHudRuntimeState{};
    PrSS0TitleHudEventsDirect::TitleSelectorResourceUpdate801C5854
        titleSelectorResourceUpdate{};
    bool titleEventPending = false;
    PrSS0TitleHudEventsDirect::TitleAbsoluteEventTickResult801C5538
        titlePendingEvent{};
    PrSS0TitleEntryPrefixDirect::Transaction801C4894
        titleEntryPrefixTransaction{};
    SS0DirectTitleEarlyInputShortcutPhase titleEarlyInputShortcutPhase =
        SS0DirectTitleEarlyInputShortcutPhase::None;
    uint8_t titleEarlyInputShortcutCompletedWaitCalls = 0u;
    uint8_t titleEarlyInputShortcutFrameWaitRemaining = 0u;
    bool titleEarlyInputPixelSubmitComplete = false;
    bool titleEarlyInputPixelSubmitStatusLogged = false;
    bool titleMimeInit801C609CReady = false;
    bool titleAttractTimeoutKnown = false;
    int titleAttractTimeoutLimit = 0;
    int attractTimer = 0;
    bool titleAttractTimeoutGapLogged = false;
    int titlePendingSelectorResult =
        PrSS0TitleHudEventsDirect::kTitleSelectorResultNone801C47EC;
    PrSceneId titlePendingSourceScene = PrSceneId::Scene0;
    int titlePendingTargetScene = -1;
    bool titlePendingMenu = false;
    // 801C4DC4 resets the shared driver only after 801C4894 has returned
    // (the 15-frame selector-exit tail is part of that call).  Keep the
    // reset latched so the confirmation cue can finish before the reset.
    bool titleExitAudioResetCommitted80026FA4 = false;
    bool titleSelectorConfirmationPresentPending = false;
    bool titleExitWaitPrepareBlockedLogged = false;
    PrSS0TitleHudEventsDirect::TitleSelectorExitWaitPresentGateState801C4D58
        titleExitWaitPresentGate{};
    PrSS0TransitionDirect::SlowTransitionRuntime80020110
        titleExitTransition{};
    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110
        titleExitTransitionVisual{};
    bool titleExitTransitionPresentBlockedLogged = false;
    PrSS0TransitionDirect::SlowTransitionRuntime80020110
        mainMenuEntryTransition{};
    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110
        mainMenuEntryTransitionVisual{};
    bool mainMenuEntryTransitionPresentBlockedLogged = false;
    // External Scene0 -> SceneN handoff is still carried through the main
    // scene switch owner, but its visual/timing state belongs to SS0.  Keep
    // this separate from the legacy PrTransition curtain so a direct handoff
    // cannot accidentally borrow the Windows S0 shell cadence or tiles.
    PrSS0TransitionDirect::SlowTransitionRuntime80020110
        sceneHandoffTransition80020110{};
    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110
        sceneHandoffTransitionVisual80020110{};
    bool sceneHandoffTransitionPresentBlockedLogged80020110 = false;
    bool sceneHandoffRuntimeActive = false;
    bool sceneHandoffTargetPending = false;
    int sceneHandoffTargetScene = -1;
    SS0DirectLoadingHoldKind loadingHoldKind =
        SS0DirectLoadingHoldKind::None;
    SS0DirectLoadingHoldKind loadingScreenKind =
        SS0DirectLoadingHoldKind::None;
    uint32_t loadingScreenStartFrame = 0u;
    uint32_t loadingHoldStartFrame = 0u;
    uint32_t loadingHoldUntilFrame = 0u;
    bool loadingHoldLogged = false;
    PrSS0TransitionDirect::LoadingPatternRuntime8001EF40
        loadingPatternRuntime8001EF40{};
    PrSS0TransitionDirect::LoadingPatternFrame8001EF40
        loadingPatternFrame8001EF40{};
    // Word_800916E0 is written by 80015408/80015660.  Scene0's common
    // 80015590 path writes 3; keep that source value instead of hard-coding
    // one visual style for every later loading page.
    int16_t loadingPatternMode80015408 =
        PrSS0TransitionDirect::kLoadingPatternMode80015408;
    bool loadingPatternRenderBlockedLogged8001EF40 = false;
    uint32_t loadingPatternSubmittedHighlightCount8001EF40 = 0u;
    uint64_t loadingPatternSubmittedGridFnv1a8001EF40 = 0u;
    uint32_t loadingPatternSubmittedFrame8001EF40 = 0u;
    PrSS0TransitionDirect::Scene0TitleResultResolveResult801C4DC4
        titleExitLoadingResult801C4DC4{};
    int mainMenuIndex = 3;
    int mainMenuRecordsMode = 0;
    int mainMenuSubChoice = -1;
    int blinkCounter800916E4 = 0;
    int mainMenuBlink = 0;
    int optionsBlink = 0;
    int stageSelectBlink = 0;
    PrSS0DirectoryDispatcherDirect::MainMenuState800264AC mainMenuState{};
    PrSS0EventFrameLoopDirect::DispatcherTailState80026B94
        mainMenuDispatcherTail80026B94{};
    PrSS0EventFrameLoopDirect::Event3FrameTransaction80026B94
        mainMenuFrameTransaction80026B94{};
    // Independent SS0 model of the shared 8001E750/8001EA00 graph owner.
    // The existing Windows S0 shell never supplies this state.
    PrPsxEventFrameDirect::EventFrameState8001E750
        mainMenuFrameHostState8001E750{};
    bool mainMenuFrameEndFrameBindingKnown8001EA00 = false;
    uint32_t mainMenuFrameEndFrameWorkListSlot8001EA00 = 0u;
    bool mainMenuFrameEndFrameBindingBlockedLogged8001EA00 = false;
    PrSS0EventFrameLoopDirect::DispatcherTailState80026B94
        mainMenuFrameTailCandidate80026B94{};
    bool mainMenuFrameTailCandidateKnown80026B94 = false;
    int32_t optionsWord800916D8 = 0;
    int32_t optionsWord800916DC = 1;
    PrSS0DirectoryDispatcherDirect::OptionsState80026910 optionsState{};
    PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseState80026B94
        optionsPreLoopPadRelease80026B94{};
    bool optionsPreLoopPadReleaseBlockedLogged = false;
    // 80015788 starts a fresh event3 loop only after the title/menu carrier
    // button has been released.  Keep this owner separate from event17/2 so
    // the second frame of a title-confirm pulse cannot be reinterpreted as a
    // new STAGE/OPTION action when the menu becomes visible.
    PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseState80026B94
        mainMenuPreLoopPadRelease80026B94{};
    bool mainMenuPreLoopPadReleaseBlockedLogged = false;
    PrSS0EventFrameLoopDirect::Event17InitialInputTimeoutState80026B94
        optionsInitialInputTimeout80026B94{};
    PrSS0EventFrameLoopDirect::DispatcherTailState80026B94
        optionsDispatcherTail80026B94{};
    PrSS0EventFrameLoopDirect::Event17FrameTransaction80026B94
        optionsFrameTransaction80026B94{};
    // SS0 owns this direct EventFrame HAL model for Event17. It is not the
    // legacy S0 shell's frame state and is reset whenever Options is entered.
    PrPsxEventFrameDirect::EventFrameState8001E750
        optionsFrameHostState8001E750{};
    bool optionsFrameEndFrameBindingKnown8001EA00 = false;
    uint32_t optionsFrameEndFrameWorkListSlot8001EA00 = 0u;
    bool optionsFrameEndFrameBindingBlockedLogged8001EA00 = false;
    PrSS0EventFrameLoopDirect::DispatcherTailState80026B94
        optionsFrameTailCandidate80026B94{};
    bool optionsFrameTailCandidateKnown80026B94 = false;
    PrSS0EventFrameLoopDirect::DispatcherPadChangeState80026744
        optionsPadChange80026744{};
    PrSS0EventTextDirect::EventTextRuntimeState800436F0
        eventTextRuntime800436F0{};
    PrSS0EventTextDirect::EventTextEmbeddedFont8005CCE4
        eventTextEmbeddedFont8005CCE4{};
    PrSS0EventTextDirect::EventTextLoadImageResult80044D64
        eventTextFontClutUpload80044D64{};
    PrSS0EventTextDirect::EventTextLoadImageResult80044D64
        eventTextFontImageUpload80044D64{};
    PrSS0EventTextDirect::EventTextDispatchResult800468E0
        eventTextFontClutDispatch800468E0{};
    PrSS0EventTextDirect::EventTextDispatchResult800468E0
        eventTextFontImageDispatch800468E0{};
    PrSS0EventTextDirect::EventTextUploadResult800462C4
        eventTextFontClutUpload800462C4{};
    PrSS0EventTextDirect::EventTextUploadResult800462C4
        eventTextFontImageUpload800462C4{};
    bool eventTextFontLoadImagesKnown80044D64 = false;
    bool eventTextFontDispatchesKnown800468E0 = false;
    bool eventTextFontUploadsKnown800462C4 = false;
    PrSS0EventTextDirect::EventTextSubmitContext800450A0
        eventTextSubmitContext800450A0{};
    ID3D11ShaderResourceView* eventTextFontTexture = nullptr;
    D3D11Renderer* eventTextFontTextureRenderer = nullptr;
    bool eventTextResourceKnown = false;
    bool eventTextBootExecuted = false;
    bool eventTextResourceLogEmitted = false;
    // 80015788 waits for the complete pad release after Event17 before
    // entering Event3. This gate is SS0-owned and is not the old S0 shell.
    PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseState80026B94
        optionsResultReleaseGate80015788{};
    bool optionsResultReleaseBlockedLogged80015788 = false;
    bool optionsResultPending80015788 = false;
    int optionsResultPendingValue80015788 = 0;
    PrSS0DirectoryDispatcherDirect::StageSelectState80025F6C stageSelectState{};
    // 80020568 publishes the Stage Select page before 80026B94's first
    // Event2 transaction.  Keep that page latched for presentation during
    // the entry boundary; the PSX framebuffer remains the visible source
    // while the dispatcher is still in its pre-loop release gate.
    bool stageSelectEntryVisualPending80020568 = false;
    bool stageSelectWord800916F0Known = false;
    int32_t stageSelectWord800916F0 = 0;
    int32_t stageSelectWord800916DA = 0;
    bool stageSelectRenderSourceMismatchLogged = false;
    PrSS0EventFrameLoopDirect::DispatcherTailState80026B94
        stageSelectDispatcherTail80026B94{};
    PrSS0EventFrameLoopDirect::Event2FrameTransaction80026B94
        stageSelectFrameTransaction80026B94{};
    // Independent SS0 owner state for Event2's 8001E750/8001EA00 frame
    // boundary. The existing Windows S0 shell never supplies this state.
    PrPsxEventFrameDirect::EventFrameState8001E750
        stageSelectFrameHostState8001E750{};
    bool stageSelectFrameEndFrameBindingKnown8001EA00 = false;
    uint32_t stageSelectFrameEndFrameWorkListSlot8001EA00 = 0u;
    bool stageSelectFrameEndFrameBindingBlockedLogged8001EA00 = false;
    PrSS0EventFrameLoopDirect::DispatcherTailState80026B94
        stageSelectFrameTailCandidate80026B94{};
    bool stageSelectFrameTailCandidateKnown80026B94 = false;
    PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseState80026B94
        stageSelectPreLoopPadRelease80026B94{};
    PrSS0EventFrameLoopDirect::DispatcherPadChangeState80026744
        stageSelectPadChange80026744{};
    bool stageSelectPreLoopPadReleaseBlockedLogged = false;
    int cardCursor = 0;
    int cardEntryCount = 0;
    int cardBlockIndex[15]{};
    char cardTitle[15][32]{};
    bool cardDirectoryAuthorityKnown = false;
    CardHandoff::CardMode800191E4 cardDirectoryMode =
        CardHandoff::CardMode800191E4::Unknown;
    CardHandoff::LoadReplayDirectoryTypedCarrier80019D7C
        cardDirectoryCarrier80019D7C{};
    CardHandoff::LoadReplaySelectedRowIdentity800181D0
        cardSelectedRowIdentity800181D0{};
    uint32_t cardSelectedRowDirectoryGeneration = 0;
    uint32_t cardDirectoryGeneration = 0;
    PrSS0DirectoryPagesRenderDirect::CardGridState80020F94
        cardGridDrawState80020F94{};
    CardHandoff::LoadReplayListInputRuntime800181D0
        cardListInputRuntime800181D0{};
    CardHandoff::CardDriverVisualRuntime80018FB0
        cardDriverVisualRuntime80018FB0{};
    CardHandoff::LoadReplayState16CardIoResultCarrier80017594
        cardState16IoResultCarrier80017594{};
    PrPsxEventFrameDirect::EventFrameState8001E750
        cardPromptEventFrame8001E750{};
    SS0DirectLoadReplayCompletionKind80019D7C
        cardCompletionKind80019D7C =
            SS0DirectLoadReplayCompletionKind80019D7C::None;
    int cardCompletionReplayScene80019D7C = -1;
    bool cardCompletionRollbackPrefixKnown80092F10 = false;
    PrStage1SaveStatusPrefix80092F10
        cardCompletionRollbackPrefix80092F10{};
    char cardMessage[96]{};
    bool replayPayloadBackupPreflightKnown = false;
    PrSS0State16RuntimeOneShotDirect::State16RuntimeTypedFactsOneShot
        state16TypedFactsOneShot{};
    bool state16TypedFactsDirectoryBindingKnown = false;
    uint32_t state16TypedFactsDirectoryGeneration = 0;
    CardHandoff::CardMode800191E4 state16TypedFactsDirectoryMode =
        CardHandoff::CardMode800191E4::Unknown;
    int state16TypedFactsDirectoryBlock = -1;
    char state16TypedFactsDirectoryTitle[32]{};
    bool debugReplayRawSelectorArmed = false;
    int debugReplayRawSelectorBlock = -1;
    uint32_t debugReplayRawSelectorSavedSlot = 0;
    bool debugHiScoreEvent6EntryFixtureArmed = false;
    int hiScoreBlink = 0;
    PrSS0DirectoryDispatcherDirect::HiScoreEvent6State800267E4
        hiScoreEvent6State800267E4{};
    PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseState80026B94
        hiScorePreLoopPadRelease80026B94{};
    bool hiScorePreLoopPadReleaseBlockedLogged = false;
    PrSS0EventFrameLoopDirect::DispatcherPadChangeState80026744
        hiScorePadChange80026744{};
    PrSS0EventFrameLoopDirect::DispatcherTailState80026B94
        hiScoreDispatcherTail80026B94{};
    PrSS0EventFrameLoopDirect::Event6FrameTransaction80026B94
        hiScoreFrameTransaction80026B94{};
    // Independent SS0 owner state for Event6's 8001E750/8001EA00 boundary.
    // The existing Windows S0 shell never supplies this state.
    PrPsxEventFrameDirect::EventFrameState8001E750
        hiScoreFrameHostState8001E750{};
    bool hiScoreFrameEndFrameBindingKnown8001EA00 = false;
    uint32_t hiScoreFrameEndFrameWorkListSlot8001EA00 = 0u;
    bool hiScoreFrameEndFrameBindingBlockedLogged8001EA00 = false;
    bool hiScoreOuterPadReleaseActive80015788 = false;
    PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseState80026B94
        hiScoreOuterPadRelease80015788{};
    bool hiScoreOuterPadReleaseBlockedLogged = false;
    bool hiScoreEvent6TableCarrierKnown = false;
    uint32_t hiScoreEvent6TablePsxAddress = 0;
    std::size_t hiScoreEvent6TableByteCount = 0;
    std::array<uint8_t, PrSceneEntryDirect::kHiScoreTableSize80019284>
        hiScoreEvent6TableBytes{};
    int practiceFrame = 0;
    // 8002776C advances the Rec44 producers at the event16 logic cadence
    // (30 Hz).  Keep the last logic frame explicitly so the 60 Hz render-only
    // pass cannot mutate the same portrait/wobble state a second time.
    int practiceProducerLastLogicFrame = -1;
    SS0DirectPracticePhase practicePhase = SS0DirectPracticePhase::Idle;
    int practiceRound = 0;
    int practiceRoundFrame = 0;
    int practicePadStopFrames = 0;
    int practicePadStopKind = -1;
    uint32_t practiceFlags00 = 0u;
    int practiceOverlayState1C = 0;
    int practiceExitConfirmFrames = 0;
    int practiceExitSelection48 = 0;
    int practiceExitBlink4C = 0;
    PrSS0PracticeLifecycleDirect::PracticeExitConfirmRuntime8002776C
        practiceExitConfirmRuntime{};
    int practiceTick = 0;
    int practicePendingJudgeKind = -1;
    int practicePendingRound = -1;
    int practiceSeqCurA = -1;
    int practiceSeqCurB = -1;
    int practiceSeqEnA = 0;
    int practiceSeqEnB = 0;
    PrSS0DirectoryPagesRenderDirect::PracticeWobbleBank80023F20
        practiceWobbleBank{};
    PrSS0DirectoryPagesRenderDirect::PracticeGuideDrawList80023518
        practiceGuideDrawList{};
    PrSS0DirectoryPagesRenderDirect::PracticeSliceDrawList8001C604
        practiceSliceDrawList{};
    PrSS0DirectoryPagesRenderDirect::PracticePostSliceTailDrawList80023618
        practicePostSliceTailDrawList{};
    PrSS0DirectoryPagesRenderDirect::PracticeLeadingOverlayDrawList80023618
        practiceLeadingOverlayDrawList{};
    bool practiceIconRgbKnown = false;
    uint8_t practiceIconR = 0;
    uint8_t practiceIconG = 0;
    uint8_t practiceIconB = 0;
    bool practiceIconStreamKnown = false;
    std::array<
        PrSS0DirectoryPagesRenderDirect::PracticeIconSubmit80024418,
        18> practiceIconCandidates{};
    PrSS0DirectoryPagesRenderDirect::PracticePortraitBank80024600
        practicePortraitBank{};
    PrSS0DirectoryPagesRenderDirect::PracticePortraitSubmit80024600
        practicePortraitTeacher{};
    PrSS0DirectoryPagesRenderDirect::PracticePortraitSubmit80024600
        practicePortraitStudent{};
    int practiceScore = 0;
    int practiceCombo = 0;
    int practiceHits = 0;
    int practiceMisses = 0;
    int practiceLastJudge = 0;
    int practiceLastDelta = 0;
    int practiceJudgeFlash = 0;
    bool practiceTargetResolved = false;
    bool practiceComplete = false;
    int16_t word800916EE = -1;
    uint16_t lastDebugPad = 0;
};

static SS0DirectRuntimeState s_ss0Direct;

struct ResidentDirectoryRuntime80015788 {
    int previousScene = 0;
    bool resourcesReady = false;
    bool entryQueued = false;
    int result = -1;
    PrStage1LifecycleExecutorDirect::State801C81EC loader{};
    PrPsxEventFrameDirect::EventFrameState8001E750 page{};
};
static std::unique_ptr<ResidentDirectoryRuntime80015788> s_residentDirectory;

static void SS0DirectBeginLoadingScreen(
    SS0DirectLoadingHoldKind kind,
    uint32_t startFrame);
static bool SS0DirectBeginLoadingPattern8001EF40(
    SS0DirectLoadingHoldKind kind);
static bool SS0DirectLoadingMinimumHoldActive(
    SS0DirectLoadingHoldKind kind);
static bool SS0DirectLoadingMinimumHoldReady(
    SS0DirectLoadingHoldKind kind,
    uint32_t frame);
static bool SS0DirectFinishLoadingScreenOrHold(
    SS0DirectLoadingHoldKind kind,
    uint32_t frame);
static void SS0DirectClearLoadingMinimumHold();

static bool SS0DirectTryAdvanceTitleVisualState801C5EF0();
static void SS0DirectAdvanceTitleVisualState801C5EF0();
static bool SS0DirectExecutePreparedTitleVisualState801C5EF0();

static void SS0DirectClearStageSelectState();
static void SS0DirectClearPracticeState();

static void SS0DirectResetMainMenuFrameHostState8001E750()
{
    s_ss0Direct.mainMenuFrameHostState8001E750 = {};
    s_ss0Direct.mainMenuFrameEndFrameBindingKnown8001EA00 = false;
    s_ss0Direct.mainMenuFrameEndFrameWorkListSlot8001EA00 = 0u;
    s_ss0Direct.mainMenuFrameEndFrameBindingBlockedLogged8001EA00 = false;
}

static void SS0DirectResetStageSelectFrameHostState8001E750()
{
    s_ss0Direct.stageSelectFrameHostState8001E750 = {};
    s_ss0Direct.stageSelectFrameEndFrameBindingKnown8001EA00 = false;
    s_ss0Direct.stageSelectFrameEndFrameWorkListSlot8001EA00 = 0u;
    s_ss0Direct.stageSelectFrameEndFrameBindingBlockedLogged8001EA00 = false;
}

static PrSS0DirectoryDispatcherDirect::OptionsState80026910
SS0DirectInitOptionsStateFromDirectCarrier800268E4()
{
    return PrSS0DirectoryDispatcherDirect::InitOptions800268E4(
        s_ss0Direct.optionsWord800916D8,
        s_ss0Direct.optionsWord800916DC);
}

static void SS0DirectClearOptionsPageStatePreserveCarrier()
{
    s_ss0Direct.optionsState =
        SS0DirectInitOptionsStateFromDirectCarrier800268E4();
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.optionsPreLoopPadRelease80026B94);
    s_ss0Direct.optionsPreLoopPadReleaseBlockedLogged = false;
    PrSS0EventFrameLoopDirect::ResetEvent17InitialInputTimeout80026B94(
        s_ss0Direct.optionsInitialInputTimeout80026B94);
    PrSS0EventFrameLoopDirect::ResetDispatcherTail80026B94(
        s_ss0Direct.optionsDispatcherTail80026B94);
    PrSS0EventFrameLoopDirect::ResetEvent17FrameTransaction80026B94(
        s_ss0Direct.optionsFrameTransaction80026B94);
    s_ss0Direct.optionsFrameHostState8001E750 = {};
    s_ss0Direct.optionsFrameEndFrameBindingKnown8001EA00 = false;
    s_ss0Direct.optionsFrameEndFrameWorkListSlot8001EA00 = 0u;
    s_ss0Direct.optionsFrameEndFrameBindingBlockedLogged8001EA00 = false;
    s_ss0Direct.optionsFrameTailCandidate80026B94 = {};
    s_ss0Direct.optionsFrameTailCandidateKnown80026B94 = false;
    PrSS0EventFrameLoopDirect::ResetDispatcherPadChange80026744(
        s_ss0Direct.optionsPadChange80026744);
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.optionsResultReleaseGate80015788);
    s_ss0Direct.optionsResultReleaseBlockedLogged80015788 = false;
    s_ss0Direct.optionsResultPending80015788 = false;
    s_ss0Direct.optionsResultPendingValue80015788 = 0;
}

static void SS0DirectCalcPs1Viewport(const D3D11Renderer* renderer, float& outX, float& outY, float& outScale) {
    outX = 0.0f;
    outY = 0.0f;
    outScale = 1.0f;
    if (!renderer) {
        return;
    }

    const float winW = static_cast<float>(renderer->GetWidth());
    const float winH = static_cast<float>(renderer->GetHeight());
    const float baseW = 320.0f;
    const float baseH = 240.0f;
    const float fitX = (baseW > 0.0f) ? (winW / baseW) : 1.0f;
    const float fitY = (baseH > 0.0f) ? (winH / baseH) : 1.0f;
    const float scale = (std::min)(fitX, fitY);
    outScale = scale;
    outX = (winW - baseW * scale) * 0.5f;
    outY = (winH - baseH * scale) * 0.5f;
}

static bool SS0DirectFindOriginalExecutable8005CCE4(
    const PrGameContext& ctx,
    std::filesystem::path& outPath) {
    outPath.clear();
    std::array<std::filesystem::path, 3> candidates{};
    candidates[0] = ctx.dataRoot / "SCUS_941.83";
    candidates[1] = ctx.dataRoot.parent_path() / "SCUS_941.83";
    candidates[2] = std::filesystem::current_path() / "SCUS_941.83";
    for (const auto& candidate : candidates) {
        if (!candidate.empty() && std::filesystem::is_regular_file(candidate)) {
            outPath = candidate;
            return true;
        }
    }
    return false;
}

static bool SS0DirectLoadEmbeddedFont8005CCE4(
    const PrGameContext& ctx) {
    if (s_ss0Direct.eventTextResourceKnown) {
        return true;
    }

    std::filesystem::path originalPath;
    if (!SS0DirectFindOriginalExecutable8005CCE4(ctx, originalPath)) {
        if (!s_ss0Direct.eventTextResourceLogEmitted) {
            s_ss0Direct.eventTextResourceLogEmitted = true;
            Log::Printf(
                "SS0 direct runtime: EventText 8005CCE4 original executable not found dataRoot=%s",
                ctx.dataRoot.u8string().c_str());
        }
        return false;
    }

    std::ifstream input(originalPath, std::ios::binary | std::ios::ate);
    if (!input) {
        return false;
    }
    const std::streamoff end = input.tellg();
    if (end <= 0) {
        return false;
    }
    std::vector<uint8_t> bytes(static_cast<size_t>(end));
    input.seekg(0, std::ios::beg);
    if (!input.read(reinterpret_cast<char*>(bytes.data()), end)) {
        return false;
    }

    constexpr size_t kFontFileOffset =
        PrSS0EventTextDirect::kEmbeddedFontFilePayloadOffset +
        (PrSS0EventTextDirect::kEmbeddedFontSourceAddress8005CCE4 -
         PrSS0EventTextDirect::kEmbeddedFontTextLoadAddress);
    if (!PrSS0EventTextDirect::DecodeEmbeddedFont8005CCE4(
            bytes.data(), bytes.size(), kFontFileOffset,
            s_ss0Direct.eventTextEmbeddedFont8005CCE4)) {
        if (!s_ss0Direct.eventTextResourceLogEmitted) {
            s_ss0Direct.eventTextResourceLogEmitted = true;
            Log::Printf(
                "SS0 direct runtime: EventText 8005CCE4 decode rejected source=%s",
                originalPath.u8string().c_str());
        }
        return false;
    }

    using namespace PrSS0EventTextDirect;
    EventTextLoadImageRequest80044D64 clutUpload{};
    clutUpload.rect = {960, 384, 16, 1};
    clutUpload.pixelBytes =
        s_ss0Direct.eventTextEmbeddedFont8005CCE4.clutBytes.data();
    clutUpload.pixelByteCount =
        s_ss0Direct.eventTextEmbeddedFont8005CCE4.clutBytes.size();
    clutUpload.pixelAddress = kEmbeddedFontSourceAddress8005CCE4;
    clutUpload.pixelAddressKnown = true;

    EventTextLoadImageRequest80044D64 imageUpload{};
    imageUpload.rect = {960, 256, 32, 32};
    imageUpload.pixelBytes =
        s_ss0Direct.eventTextEmbeddedFont8005CCE4.imageBytes.data();
    imageUpload.pixelByteCount =
        s_ss0Direct.eventTextEmbeddedFont8005CCE4.imageBytes.size();
    imageUpload.pixelAddress = kEmbeddedFontImageAddress8005CEE4;
    imageUpload.pixelAddressKnown = true;

    if (!ExecuteLoadImageSoftware80044D64(
            s_ss0Direct.eventTextRuntime800436F0,
            clutUpload,
            s_ss0Direct.eventTextFontClutUpload80044D64) ||
        !ExecuteLoadImageSoftware80044D64(
            s_ss0Direct.eventTextRuntime800436F0,
            imageUpload,
            s_ss0Direct.eventTextFontImageUpload80044D64)) {
        if (!s_ss0Direct.eventTextResourceLogEmitted) {
            s_ss0Direct.eventTextResourceLogEmitted = true;
            Log::Printf(
                "SS0 direct runtime: EventText 80044D64 font upload binding rejected");
        }
        return false;
    }
    std::array<uint8_t, sizeof(EventTextPsxRect80044D64)> clutRectBytes{};
    std::array<uint8_t, sizeof(EventTextPsxRect80044D64)> imageRectBytes{};
    std::memcpy(clutRectBytes.data(), &clutUpload.rect, clutRectBytes.size());
    std::memcpy(imageRectBytes.data(), &imageUpload.rect, imageRectBytes.size());
    EventTextDispatchRequest800468E0 clutDispatch{};
    clutDispatch.callbackFunction = kFn800462C4;
    clutDispatch.headBytes = clutRectBytes.data();
    clutDispatch.headByteCount = clutRectBytes.size();
    clutDispatch.copyByteCount = 8u;
    clutDispatch.callbackArgument = clutUpload.pixelAddress;
    clutDispatch.callbackArgumentKnown = clutUpload.pixelAddressKnown;
    EventTextDispatchRequest800468E0 imageDispatch = clutDispatch;
    imageDispatch.headBytes = imageRectBytes.data();
    imageDispatch.callbackArgument = imageUpload.pixelAddress;
    imageDispatch.callbackArgumentKnown = imageUpload.pixelAddressKnown;
    if (!ExecuteTextDispatchSoftware800468E0(
            clutDispatch, s_ss0Direct.eventTextFontClutDispatch800468E0) ||
        !ExecuteTextDispatchSoftware800468E0(
            imageDispatch, s_ss0Direct.eventTextFontImageDispatch800468E0)) {
        if (!s_ss0Direct.eventTextResourceLogEmitted) {
            s_ss0Direct.eventTextResourceLogEmitted = true;
            Log::Printf(
                "SS0 direct runtime: EventText 800468E0 font dispatch binding rejected");
        }
        return false;
    }
    s_ss0Direct.eventTextFontDispatchesKnown800468E0 = true;
    if (!ExecuteLoadImageUploadSoftware800462C4(
            clutUpload, s_ss0Direct.eventTextFontClutUpload800462C4) ||
        !ExecuteLoadImageUploadSoftware800462C4(
            imageUpload, s_ss0Direct.eventTextFontImageUpload800462C4)) {
        if (!s_ss0Direct.eventTextResourceLogEmitted) {
            s_ss0Direct.eventTextResourceLogEmitted = true;
            Log::Printf(
                "SS0 direct runtime: EventText 800462C4 font upload binding rejected");
        }
        return false;
    }
    s_ss0Direct.eventTextFontUploadsKnown800462C4 = true;
    s_ss0Direct.eventTextFontLoadImagesKnown80044D64 = true;
    s_ss0Direct.eventTextResourceKnown = true;
    Log::Printf(
        "SS0 direct runtime: EventText 8005CCE4 source accepted path=%s fileOffset=%X clutRect=(%d,%d,%d,%d) imageRect=(%d,%d,%d,%d) clutWords=%u/%u imageWords=%u dmaBlocks=%u exactPsxHal=0",
        originalPath.u8string().c_str(),
        static_cast<unsigned>(kFontFileOffset),
        clutUpload.rect.x,
        clutUpload.rect.y,
        clutUpload.rect.w,
        clutUpload.rect.h,
        imageUpload.rect.x,
        imageUpload.rect.y,
        imageUpload.rect.w,
        imageUpload.rect.h,
        s_ss0Direct.eventTextFontClutUpload800462C4.transferWords,
        s_ss0Direct.eventTextFontClutUpload800462C4.cpuTailWords,
        s_ss0Direct.eventTextFontImageUpload800462C4.transferWords,
        s_ss0Direct.eventTextFontImageUpload800462C4.dmaBlockCount);
    return true;
}

static bool SS0DirectEnsureEventTextFontTexture(PrGameContext& ctx) {
    if (!ctx.renderer || !s_ss0Direct.eventTextResourceKnown) {
        return false;
    }
    if (s_ss0Direct.eventTextFontTexture != nullptr &&
        s_ss0Direct.eventTextFontTextureRenderer == ctx.renderer) {
        return true;
    }
    if (s_ss0Direct.eventTextFontTexture != nullptr &&
        s_ss0Direct.eventTextFontTextureRenderer != nullptr) {
        s_ss0Direct.eventTextFontTextureRenderer->DestroyTexture(
            s_ss0Direct.eventTextFontTexture);
        s_ss0Direct.eventTextFontTexture = nullptr;
        s_ss0Direct.eventTextFontTextureRenderer = nullptr;
    }
    s_ss0Direct.eventTextFontTexture = ctx.renderer->CreateTexture(
        s_ss0Direct.eventTextEmbeddedFont8005CCE4.rgba.data(),
        static_cast<int>(PrSS0EventTextDirect::kEmbeddedFontWidth),
        static_cast<int>(PrSS0EventTextDirect::kEmbeddedFontHeight));
    if (s_ss0Direct.eventTextFontTexture == nullptr) {
        return false;
    }
    s_ss0Direct.eventTextFontTextureRenderer = ctx.renderer;
    return true;
}

static int32_t SS0DirectEventTextDisplayEnableSink80044AA0(
    int32_t enabled,
    void*) {
    // The software core records the original GP1 display-enable call order;
    // Windows swap-chain state is owned by the existing host frame loop.
    (void)enabled;
    return 0;
}

static bool SS0DirectEventTextHostGlyphSink800450A0(
    const PrSS0EventTextDirect::EventTextHostGlyphDraw800450A0& draw,
    void* userData) {
    auto* runtime = static_cast<SS0DirectRuntimeState*>(userData);
    if (runtime == nullptr || runtime->eventTextFontTexture == nullptr ||
        runtime->eventTextFontTextureRenderer == nullptr) {
        return false;
    }
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(runtime->eventTextFontTextureRenderer,
                             vx, vy, vs);
    D3D11Renderer::SpriteCmd command{};
    command.texture = runtime->eventTextFontTexture;
    // FntOpen's verified -156/-120 origin is centered at the PSX 320x240
    // display origin, so map the packet back to the host top-left viewport.
    command.x = vx + (static_cast<float>(draw.x) + 160.0f) * vs;
    command.y = vy + (static_cast<float>(draw.y) + 120.0f) * vs;
    command.w = 8.0f * vs;
    command.h = 8.0f * vs;
    command.u0 = static_cast<float>(draw.u) /
                static_cast<float>(PrSS0EventTextDirect::kEmbeddedFontWidth);
    command.v0 = static_cast<float>(draw.v) /
                static_cast<float>(PrSS0EventTextDirect::kEmbeddedFontHeight);
    command.u1 = static_cast<float>(draw.u + 8u) /
                static_cast<float>(PrSS0EventTextDirect::kEmbeddedFontWidth);
    command.v1 = static_cast<float>(draw.v + 8u) /
                static_cast<float>(PrSS0EventTextDirect::kEmbeddedFontHeight);
    command.r = static_cast<float>(draw.r) / 255.0f;
    command.g = static_cast<float>(draw.g) / 255.0f;
    command.b = static_cast<float>(draw.b) / 255.0f;
    command.a = 1.0f;
    command.layer = 900;
    runtime->eventTextFontTextureRenderer->SubmitSprite(command);
    return true;
}

static bool SS0DirectEnsureEventTextRuntime(PrGameContext& ctx) {
    (void)SS0DirectLoadEmbeddedFont8005CCE4(ctx);
    const bool textureReady = SS0DirectEnsureEventTextFontTexture(ctx);

    auto& submitContext = s_ss0Direct.eventTextSubmitContext800450A0;
    submitContext.hostGlyphSink = SS0DirectEventTextHostGlyphSink800450A0;
    submitContext.hostGlyphSinkUserData = &s_ss0Direct;
    submitContext.hostGpuBoundaryKnown = textureReady;
    submitContext.hostRuntimeConsumerWired = textureReady;

    if (!s_ss0Direct.eventTextBootExecuted) {
        using namespace PrSS0EventTextDirect;
        EventTextFontBinding80043394 binding{};
        EventTextBootResult80027FAC boot{};
        if (ComputeFntLoadFontBinding80043394(960, 256, 0, 0u, binding) &&
            !s_ss0Direct.eventTextRuntime800436F0.textBankKnown) {
            s_ss0Direct.eventTextRuntime800436F0.textBankKnown = true;
            s_ss0Direct.eventTextRuntime800436F0.glyphBankKnown = true;
            if (ExecuteTextSystemBoot80027FACSoftwareCore(
                    s_ss0Direct.eventTextRuntime800436F0,
                    binding,
                    SS0DirectEventTextDisplayEnableSink80044AA0,
                    nullptr,
                    boot)) {
                s_ss0Direct.eventTextBootExecuted = true;
                Log::Printf(
                    "SS0 direct runtime: EventText 80027FAC software boot committed tpage=%04X clut=%04X fontTexture=%d",
                    binding.tpage,
                    binding.clut,
                    textureReady ? 1 : 0);
            }
        }
    }
    return s_ss0Direct.eventTextBootExecuted;
}

static bool SS0DirectFlushEventText800436F0(PrGameContext& ctx) {
    using namespace PrSS0EventTextDirect;
    if (!s_ss0Direct.eventTextBootExecuted ||
        !s_ss0Direct.eventTextSubmitContext800450A0.hostRuntimeConsumerWired ||
        !s_ss0Direct.eventTextRuntime800436F0.currentRecordKnown) {
        return false;
    }

    EventTextFlushExecutionResult800436F0 flush{};
    if (!ExecuteTextFlush800436F0(
            s_ss0Direct.eventTextRuntime800436F0,
            -1,
            SubmitTextOtagSoftware800450A0,
            &s_ss0Direct.eventTextSubmitContext800450A0,
            flush)) {
        Log::Printf(
            "SS0 direct runtime: EventText 800436F0 pending flush blocked status=%u frame=%u",
            static_cast<unsigned>(flush.status),
            static_cast<unsigned>(ctx.frame));
        return false;
    }
    const auto& submit = s_ss0Direct.eventTextSubmitContext800450A0.last;
    const bool drawn = submit.status == EventTextSubmitStatus800450A0::Executed &&
                       submit.hostGlyphsSubmitted &&
                       submit.hostGlyphCount != 0u;
    if (drawn) {
        Log::Printf(
            "SS0 direct runtime: EventText 800436F0 host glyph flush executed frame=%u glyphs=%u",
            static_cast<unsigned>(ctx.frame),
            static_cast<unsigned>(submit.hostGlyphCount));
    }
    // Empty text still executes the formal 800436F0 transaction. Return the
    // transaction result, while the glyph log above remains draw-specific.
    return flush.status == EventTextFlushStatus800436F0::Executed;
}

static void SS0DirectResetEventTextRuntime(PrGameContext& ctx) {
    if (s_ss0Direct.eventTextFontTexture != nullptr &&
        s_ss0Direct.eventTextFontTextureRenderer != nullptr) {
        s_ss0Direct.eventTextFontTextureRenderer->DestroyTexture(
            s_ss0Direct.eventTextFontTexture);
    }
    s_ss0Direct.eventTextFontTexture = nullptr;
    s_ss0Direct.eventTextFontTextureRenderer = nullptr;
    s_ss0Direct.eventTextRuntime800436F0 = {};
    s_ss0Direct.eventTextEmbeddedFont8005CCE4 = {};
    s_ss0Direct.eventTextFontClutUpload80044D64 = {};
    s_ss0Direct.eventTextFontImageUpload80044D64 = {};
    s_ss0Direct.eventTextFontClutUpload800462C4 = {};
    s_ss0Direct.eventTextFontImageUpload800462C4 = {};
    s_ss0Direct.eventTextFontClutDispatch800468E0 = {};
    s_ss0Direct.eventTextFontImageDispatch800468E0 = {};
    s_ss0Direct.eventTextFontLoadImagesKnown80044D64 = false;
    s_ss0Direct.eventTextFontDispatchesKnown800468E0 = false;
    s_ss0Direct.eventTextFontUploadsKnown800462C4 = false;
    s_ss0Direct.eventTextSubmitContext800450A0 = {};
    s_ss0Direct.eventTextResourceKnown = false;
    s_ss0Direct.eventTextBootExecuted = false;
    s_ss0Direct.eventTextResourceLogEmitted = false;
    (void)SS0DirectEnsureEventTextRuntime(ctx);
}

static uint16_t MapScene0DebugPadToLocalPrPadMask(uint16_t psxPadMask) {
    uint16_t localMask = static_cast<uint16_t>(
        psxPadMask & ((uint16_t)PrPadButton::Triangle |
                      (uint16_t)PrPadButton::Circle |
                      (uint16_t)PrPadButton::Cross |
                      (uint16_t)PrPadButton::Square));
    if ((psxPadMask & 0x1000u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::Up);
    }
    if ((psxPadMask & 0x2000u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::Right);
    }
    if ((psxPadMask & 0x4000u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::Down);
    }
    if ((psxPadMask & 0x8000u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::Left);
    }
    if ((psxPadMask & 0x0001u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::L2);
    }
    if ((psxPadMask & 0x0002u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::R2);
    }
    if ((psxPadMask & 0x0004u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::L1);
    }
    if ((psxPadMask & 0x0008u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::R1);
    }
    if ((psxPadMask & 0x0800u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::Start);
    }
    if ((psxPadMask & 0x0100u) != 0u) {
        localMask = static_cast<uint16_t>(
            localMask | (uint16_t)PrPadButton::Select);
    }
    return localMask;
}

static uint32_t SS0DirectLocalPressedToPsxPadMask(uint16_t localPressed) {
    uint32_t psx = 0;
    if ((localPressed & static_cast<uint16_t>(PrPadButton::Triangle)) != 0u) {
        psx |= PrSS0DirectoryDispatcherDirect::kPadTriangle;
    }
    if ((localPressed & static_cast<uint16_t>(PrPadButton::Circle)) != 0u) {
        psx |= PrSS0DirectoryDispatcherDirect::kPadCircle;
    }
    if ((localPressed & static_cast<uint16_t>(PrPadButton::Cross)) != 0u) {
        psx |= PrSS0DirectoryDispatcherDirect::kPadCross;
    }
    if ((localPressed & static_cast<uint16_t>(PrPadButton::Square)) != 0u) {
        psx |= PrSS0DirectoryDispatcherDirect::kPadSquare;
    }
    if ((localPressed & static_cast<uint16_t>(PrPadButton::Up)) != 0u) {
        psx |= PrSS0DirectoryDispatcherDirect::kPadUp;
    }
    if ((localPressed & static_cast<uint16_t>(PrPadButton::Right)) != 0u) {
        psx |= PrSS0DirectoryDispatcherDirect::kPadRight;
    }
    if ((localPressed & static_cast<uint16_t>(PrPadButton::Down)) != 0u) {
        psx |= PrSS0DirectoryDispatcherDirect::kPadDown;
    }
    if ((localPressed & static_cast<uint16_t>(PrPadButton::Left)) != 0u) {
        psx |= PrSS0DirectoryDispatcherDirect::kPadLeft;
    }
    return psx;
}

static uint32_t SS0DirectFullLocalPadToPsxMask80026744(
    uint16_t localPadMask) {
    // 80026744 forwards the complete nonzero 80035510 mask after a change.
    // Event6 and Event17 both require the full 16-bit value because their
    // handlers compare exact masks rather than testing individual bits.
    uint32_t psx = SS0DirectLocalPressedToPsxPadMask(localPadMask);
    if ((localPadMask & static_cast<uint16_t>(PrPadButton::L2)) != 0u) {
        psx |= 0x0001u;
    }
    if ((localPadMask & static_cast<uint16_t>(PrPadButton::R2)) != 0u) {
        psx |= 0x0002u;
    }
    if ((localPadMask & static_cast<uint16_t>(PrPadButton::L1)) != 0u) {
        psx |= 0x0004u;
    }
    if ((localPadMask & static_cast<uint16_t>(PrPadButton::R1)) != 0u) {
        psx |= 0x0008u;
    }
    if ((localPadMask & static_cast<uint16_t>(PrPadButton::Select)) != 0u) {
        psx |= 0x0100u;
    }
    if ((localPadMask & static_cast<uint16_t>(PrPadButton::Start)) != 0u) {
        psx |= 0x0800u;
    }
    return psx;
}

static bool SS0DirectReadStageStatusFromDirectBank(uint8_t outStatus[7]) {
    if (!outStatus) {
        return false;
    }

    const PrStageClearStatusBankSnapshot snapshot =
        PrStage1SaveUiDirect::GetStageClearStatusBankSnapshot();
    if (!snapshot.statusBytesKnown80092F1D) {
        return false;
    }

    bool allPrimaryClear = true;
    for (int i = 0; i < 6; ++i) {
        const uint8_t status =
            static_cast<uint8_t>(std::clamp(static_cast<int>(snapshot.byte80092F1D[i]),
                                            0,
                                            3));
        outStatus[i] = status;
        allPrimaryClear = allPrimaryClear && status == 3u;
    }
    outStatus[6] = allPrimaryClear ? 1u : 0u;
    return true;
}

static const char* SS0DirectTypedCardReadLaneName(
    SS0DirectTypedCardReadLane lane) {
    switch (lane) {
    case SS0DirectTypedCardReadLane::State16LoadPayload80019D7C:
        return "state16-load-payload-80019D7C";
    case SS0DirectTypedCardReadLane::Case17HiScorePayload80019D7C:
        return "case17-hiscore-payload-80019D7C";
    }
    return "unknown";
}

static const char* SS0DirectTypedCarrierSourceName(
    PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4 source) {
    switch (source) {
    case PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
        RuntimeLowerCardProducer:
        return "runtime-lower-card-producer";
    case PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
        DebugSyntheticFixture:
        return "debug-synthetic-fixture";
    case PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
        Unknown:
        break;
    }
    return "unknown";
}

static bool SS0DirectTypedCardReadLaneKnown(
    const SS0DirectTypedCardReadAuthority& authority,
    SS0DirectTypedCardReadLane requiredLane) {
    switch (requiredLane) {
    case SS0DirectTypedCardReadLane::State16LoadPayload80019D7C:
        return authority.state16LoadPayloadLaneKnown;
    case SS0DirectTypedCardReadLane::Case17HiScorePayload80019D7C:
        return authority.case17HiScorePayloadLaneKnown;
    }
    return false;
}

static bool SS0DirectRequireTypedCardReadAuthority(
    const char* reason,
    SS0DirectTypedCardReadLane requiredLane,
    int expectedBlockIndex = -1,
    SS0DirectTypedPayloadCommitSeam* outPayloadSeam = nullptr) {
    if (outPayloadSeam) {
        *outPayloadSeam = {};
    }
    SS0DirectTypedCardReadAuthority typedCardReadAuthority{};
    PrStage1SaveCardHalDirect::CardReadFeedbackRequest800179B4
        state16ReadRequest{};
    bool state16ReadRequestKnown = false;
    PrStage1SaveCardHalDirect::State16CardReadTypedCarrier800179B4
        state16Carrier{};
    bool state16CarrierKnown = false;
    PrStage1SaveCardHalDirect::Case17CardReadTypedCarrier800179B4
        case17Carrier{};
    bool case17CarrierKnown = false;
    if (requiredLane ==
        SS0DirectTypedCardReadLane::State16LoadPayload80019D7C) {
        state16ReadRequest =
            PrStage1SaveCardHalDirect::
                MakeState16LoadPayloadReadRequest800179B4(16);
        state16ReadRequestKnown = true;
        state16CarrierKnown =
            PrStage1SaveCardHalDirect::
                GetState16CardReadTypedCarrier800179B4(&state16Carrier);
        if (state16CarrierKnown) {
            typedCardReadAuthority
                .producerWired800173A8_80016EB8_800179B4 =
                state16Carrier.producerWired800173A8_80016EB8_800179B4;
            typedCardReadAuthority.carrierSource =
                state16Carrier.source;
            typedCardReadAuthority.state16LoadPayloadLaneKnown =
                state16Carrier.state16LoadPayloadLaneKnown;
            typedCardReadAuthority.state16SelectedBlockKnown =
                state16Carrier.selectedBlockKnown;
            typedCardReadAuthority.state16SelectedBlock =
                state16Carrier.selectedBlockIndex;
            typedCardReadAuthority.typedReadSuccessKnown800179B4 =
                state16Carrier.typedReadSuccessKnown800179B4;
            typedCardReadAuthority.payloadBytesKnown8007ADE8 =
                state16Carrier.payloadBytesKnown8007ADE8;
            typedCardReadAuthority.incomplete800179B4 =
                state16Carrier.incomplete;
        }
    } else if (requiredLane ==
               SS0DirectTypedCardReadLane::Case17HiScorePayload80019D7C) {
        case17CarrierKnown =
            PrStage1SaveCardHalDirect::GetCase17CardReadTypedCarrier800179B4(
                &case17Carrier);
        if (case17CarrierKnown) {
            typedCardReadAuthority
                .producerWired800173A8_80016EB8_800179B4 =
                case17Carrier.producerWired800173A8_80016EB8_800179B4;
            typedCardReadAuthority.carrierSource = case17Carrier.source;
            typedCardReadAuthority.case17HiScorePayloadLaneKnown =
                case17Carrier.case17HiScorePayloadLaneKnown;
            typedCardReadAuthority.typedReadSuccessKnown800179B4 =
                case17Carrier.typedReadSuccessKnown800179B4;
            typedCardReadAuthority.payloadBytesKnown8007ADE8 =
                case17Carrier.payloadBytesKnown8007ADE8;
            typedCardReadAuthority.incomplete800179B4 =
                case17Carrier.incomplete;
        }
    }
    const bool requiredLaneKnown = SS0DirectTypedCardReadLaneKnown(
        typedCardReadAuthority,
        requiredLane);
    const bool requiredBlockMatches =
        requiredLane !=
            SS0DirectTypedCardReadLane::State16LoadPayload80019D7C ||
        (expectedBlockIndex >= 0 &&
         typedCardReadAuthority.state16SelectedBlockKnown &&
         typedCardReadAuthority.state16SelectedBlock == expectedBlockIndex);
    const bool sourceIsRuntimeLowerCardProducer =
        typedCardReadAuthority.carrierSource ==
        PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
            RuntimeLowerCardProducer;
    const bool sourceAllowed = sourceIsRuntimeLowerCardProducer;
    const bool typedLowerCardReadProducerReady =
        typedCardReadAuthority
            .producerWired800173A8_80016EB8_800179B4 &&
        requiredLaneKnown &&
        requiredBlockMatches &&
        sourceAllowed &&
        typedCardReadAuthority.typedReadSuccessKnown800179B4 &&
        typedCardReadAuthority.payloadBytesKnown8007ADE8 &&
        !typedCardReadAuthority.incomplete800179B4;
    if (!typedLowerCardReadProducerReady) {
        Log::Printf(
            "SS0 direct runtime: %s payload import blocked: typed 800173A8/80016EB8/800179B4 lower-card producer missing lane=%s source=%s sourceAllowed=%d producer=%d laneKnown=%d blockKnown=%d block=%d expectedBlock=%d readSuccess=%d payloadBytes=%d incomplete=%d state16ReadRequest=%d reqArg2Known=%d reqArg2=%d reqName=%08X reqTarget=%08X reqPayload=%08X reqBlocks=%d reqBytes=%zu reqCloseGp696=%d",
            reason ? reason : "memcard",
            SS0DirectTypedCardReadLaneName(requiredLane),
            SS0DirectTypedCarrierSourceName(
                typedCardReadAuthority.carrierSource),
            sourceAllowed ? 1 : 0,
            typedCardReadAuthority
                .producerWired800173A8_80016EB8_800179B4
                ? 1
                : 0,
            requiredLaneKnown ? 1 : 0,
            typedCardReadAuthority.state16SelectedBlockKnown ? 1 : 0,
            typedCardReadAuthority.state16SelectedBlock,
            expectedBlockIndex,
            typedCardReadAuthority.typedReadSuccessKnown800179B4
                ? 1
                : 0,
            typedCardReadAuthority.payloadBytesKnown8007ADE8
                ? 1
                : 0,
            typedCardReadAuthority.incomplete800179B4 ? 1 : 0,
            state16ReadRequestKnown ? 1 : 0,
            state16ReadRequest.arg2Known ? 1 : 0,
            state16ReadRequest.arg2,
            state16ReadRequest.nameAddress,
            state16ReadRequest.targetBufferAddress,
            state16ReadRequest.payloadAddress,
            state16ReadRequest.blockCount,
            state16ReadRequest.blockBytes,
            state16ReadRequest.closeGp696FactRequired ? 1 : 0);
        if (requiredLane ==
                SS0DirectTypedCardReadLane::State16LoadPayload80019D7C &&
            state16CarrierKnown) {
            PrStage1SaveCardHalDirect::
                ClearState16CardReadTypedCarrier800179B4();
        }
        if (requiredLane ==
                SS0DirectTypedCardReadLane::Case17HiScorePayload80019D7C &&
            case17CarrierKnown) {
            PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
        }
        return false;
    }

    if (requiredLane ==
            SS0DirectTypedCardReadLane::State16LoadPayload80019D7C &&
        outPayloadSeam) {
        const int selectedBlock = typedCardReadAuthority.state16SelectedBlock;
        if (state16CarrierKnown &&
            selectedBlock >= 0 &&
            selectedBlock < PrStage1SaveCardHalDirect::
                kReadAttemptCount800179B4) {
            outPayloadSeam->known = true;
            outPayloadSeam->blockIndex = selectedBlock;
            outPayloadSeam->byteCount =
                PrStagePayloadBankDirect::kByteCount80092F10;
            outPayloadSeam->runtimeLowerCardProducerKnown = true;
            outPayloadSeam->typedReadSuccessKnown800179B4 =
                typedCardReadAuthority.typedReadSuccessKnown800179B4;
            outPayloadSeam->payloadBytesKnown8007ADE8 =
                typedCardReadAuthority.payloadBytesKnown8007ADE8;
            outPayloadSeam->payloadAddress8007ADE8 =
                PrStagePayloadBankDirect::kTypedPayloadSourceAddress8007ADE8;
            outPayloadSeam->payloadAuthority800164B4 =
                state16Carrier.payloadAuthority800164B4;
            std::copy_n(
                state16Carrier.blockStorage[selectedBlock].begin() +
                    PrStage1SaveCardHalDirect::
                        kCardReadPayloadOffset8007ADE8,
                PrStagePayloadBankDirect::kByteCount80092F10,
                outPayloadSeam->bytes.begin());
        }
    }
    Log::Printf(
        "SS0 direct runtime: %s typed card read authority accepted lane=%s source=%s block=%d payloadSeam=%d bytes=%zu",
        reason ? reason : "memcard",
        SS0DirectTypedCardReadLaneName(requiredLane),
        SS0DirectTypedCarrierSourceName(
            typedCardReadAuthority.carrierSource),
        typedCardReadAuthority.state16SelectedBlock,
        outPayloadSeam && outPayloadSeam->known ? 1 : 0,
        outPayloadSeam ? outPayloadSeam->byteCount : 0u);
    if (requiredLane ==
            SS0DirectTypedCardReadLane::State16LoadPayload80019D7C &&
        state16CarrierKnown) {
        PrStage1SaveCardHalDirect::ClearState16CardReadTypedCarrier800179B4();
    }
    if (requiredLane ==
            SS0DirectTypedCardReadLane::Case17HiScorePayload80019D7C &&
        case17CarrierKnown) {
        PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
    }
    return true;
}

static void SS0DirectClearHiScoreEvent6ControlState() {
    s_ss0Direct.hiScoreEvent6State800267E4 = {};
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.hiScorePreLoopPadRelease80026B94);
    s_ss0Direct.hiScorePreLoopPadReleaseBlockedLogged = false;
    PrSS0EventFrameLoopDirect::ResetDispatcherPadChange80026744(
        s_ss0Direct.hiScorePadChange80026744);
    PrSS0EventFrameLoopDirect::ResetDispatcherTail80026B94(
        s_ss0Direct.hiScoreDispatcherTail80026B94);
    PrSS0EventFrameLoopDirect::ResetEvent6FrameTransaction80026B94(
        s_ss0Direct.hiScoreFrameTransaction80026B94);
    s_ss0Direct.hiScoreFrameHostState8001E750 = {};
    s_ss0Direct.hiScoreFrameEndFrameBindingKnown8001EA00 = false;
    s_ss0Direct.hiScoreFrameEndFrameWorkListSlot8001EA00 = 0u;
    s_ss0Direct.hiScoreFrameEndFrameBindingBlockedLogged8001EA00 = false;
    s_ss0Direct.hiScoreOuterPadReleaseActive80015788 = false;
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.hiScoreOuterPadRelease80015788);
    s_ss0Direct.hiScoreOuterPadReleaseBlockedLogged = false;
}

static void SS0DirectClearHiScoreEvent6TableCarrier() {
    s_ss0Direct.hiScoreEvent6TableCarrierKnown = false;
    s_ss0Direct.hiScoreEvent6TablePsxAddress = 0;
    s_ss0Direct.hiScoreEvent6TableByteCount = 0;
    s_ss0Direct.hiScoreEvent6TableBytes.fill(0);
}

static void SS0DirectClearHiScorePageState() {
    SS0DirectClearHiScoreEvent6ControlState();
    SS0DirectClearHiScoreEvent6TableCarrier();
}

static void SS0DirectWriteHiScoreEvent6CtxWord(std::size_t offset,
                                               int32_t value) {
    if (!s_ss0Direct.hiScoreEvent6TableCarrierKnown ||
        offset + sizeof(uint32_t) >
            s_ss0Direct.hiScoreEvent6TableByteCount) {
        return;
    }
    const uint32_t word = static_cast<uint32_t>(value);
    auto& bytes = s_ss0Direct.hiScoreEvent6TableBytes;
    bytes[offset + 0u] = static_cast<uint8_t>(word & 0xFFu);
    bytes[offset + 1u] = static_cast<uint8_t>((word >> 8u) & 0xFFu);
    bytes[offset + 2u] = static_cast<uint8_t>((word >> 16u) & 0xFFu);
    bytes[offset + 3u] = static_cast<uint8_t>((word >> 24u) & 0xFFu);
}

static void SS0DirectPublishHiScoreEvent6Ctx80049278() {
    const auto& state = s_ss0Direct.hiScoreEvent6State800267E4;
    if (!state.requestBound ||
        state.ctxAddress !=
            PrSS0DirectoryDispatcherDirect::kEvent6HiScoreCtx80049278) {
        return;
    }
    SS0DirectWriteHiScoreEvent6CtxWord(
        PrSS0HiScoreRenderDirect::kHiScoreExitIconStateOffset80021594,
        state.exitIconStateCtx00);
    SS0DirectWriteHiScoreEvent6CtxWord(
        PrSS0HiScoreRenderDirect::kHiScoreExitLabelStateOffset80021594,
        state.exitLabelStateCtx04);
}

static bool SS0DirectBuildHiScoreEvent6TableCarrier() {
    SS0DirectClearHiScoreEvent6TableCarrier();

    const PrStage1SaveStatusPrefix80092F10 statusPrefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    if (!statusPrefix.known || statusPrefix.helperGap ||
        !statusPrefix.statusBankKnown80092F1D) {
        Log::Printf(
            "SS0 direct runtime: HI-SCORE table event6 carrier blocked: "
            "statusPrefixKnown=%d helperGap=%d statusBankKnown=%d psxAddress=%08X byteCount=%u",
            statusPrefix.known ? 1 : 0,
            statusPrefix.helperGap ? 1 : 0,
            statusPrefix.statusBankKnown80092F1D ? 1 : 0,
            statusPrefix.psxAddress,
            statusPrefix.byteCount);
        return false;
    }

    PrStage1SaveCardHalDirect::Case17CardReadTypedCarrier800179B4
        case17Carrier{};
    if (!PrStage1SaveCardHalDirect::GetCase17CardReadTypedCarrier800179B4(
            &case17Carrier)) {
        Log::Printf(
            "SS0 direct runtime: HI-SCORE table payload import blocked: "
            "typed 800173A8/80016EB8/800179B4 lower-card producer missing "
            "lane=%s source=unknown sourceAllowed=0 producer=0 laneKnown=0 "
            "blockKnown=0 block=-1 expectedBlock=-1 readSuccess=0 payloadBytes=0 "
            "incomplete=0 state16ReadRequest=0 reqArg2Known=0 reqArg2=0 "
            "reqName=00000000 reqTarget=00000000 reqPayload=00000000 reqBlocks=0 reqBytes=0 reqCloseGp696=0",
            SS0DirectTypedCardReadLaneName(
                SS0DirectTypedCardReadLane::Case17HiScorePayload80019D7C));
        return false;
    }

    const bool sourceAllowed =
        case17Carrier.source ==
        PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
            RuntimeLowerCardProducer;
    const bool authorityReady =
        sourceAllowed &&
        case17Carrier.producerWired800173A8_80016EB8_800179B4 &&
        case17Carrier.case17LoopCompletionKnown80019D7C &&
        !case17Carrier.incomplete;
    if (!authorityReady) {
        Log::Printf(
            "SS0 direct runtime: HI-SCORE table payload import blocked: "
            "typed 800173A8/80016EB8/800179B4 lower-card producer missing "
            "lane=%s source=%s sourceAllowed=%d producer=%d completionKnown=%d laneKnown=%d "
            "blockKnown=0 block=-1 expectedBlock=-1 readSuccess=%d payloadBytes=%d "
            "incomplete=%d state16ReadRequest=0 reqArg2Known=0 reqArg2=0 "
            "reqName=00000000 reqTarget=00000000 reqPayload=00000000 reqBlocks=0 reqBytes=0 reqCloseGp696=0",
            SS0DirectTypedCardReadLaneName(
                SS0DirectTypedCardReadLane::Case17HiScorePayload80019D7C),
            SS0DirectTypedCarrierSourceName(case17Carrier.source),
            sourceAllowed ? 1 : 0,
            case17Carrier.producerWired800173A8_80016EB8_800179B4 ? 1 : 0,
            case17Carrier.case17LoopCompletionKnown80019D7C ? 1 : 0,
            case17Carrier.case17HiScorePayloadLaneKnown ? 1 : 0,
            case17Carrier.typedReadSuccessKnown800179B4 ? 1 : 0,
            case17Carrier.payloadBytesKnown8007ADE8 ? 1 : 0,
            case17Carrier.incomplete ? 1 : 0);
        PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
        return false;
    }

    PrSceneEntryCardFeedbackDirect::Case17CardReadHalFeedback80019D7C
        case17Hal{};
    PrSceneEntryCardFeedbackDirect::
        BuildCase17CardReadHalFeedbackFromSaveCardHal800179B4(
            case17Carrier.feedback,
            case17Carrier.hal,
            &case17Hal);

    PrSceneEntryCardFeedbackDirect::Case17To19414FeedbackBuildResult80015788
        bridge{};
    PrSceneEntryCardFeedbackDirect::BuildFeedback80019414FromCase17CardReadFacts(
        statusPrefix,
        true,
        17,
        case17Hal,
        &bridge);
    if (!bridge.completed || bridge.gap) {
        Log::Printf(
            "SS0 direct runtime: HI-SCORE table event6 carrier gap: "
            "completed=%d gap=%d missingHal=%d",
            bridge.completed ? 1 : 0,
            bridge.gap ? 1 : 0,
            bridge.missingHalFacts800179B4 ? 1 : 0);
        PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
        return false;
    }

    const PrSceneEntryDirect::Call80019414Result80015788 call19414 =
        PrSceneEntryDirect::PsxCall80019414_HiScoreEntry80015788(
            static_cast<int32_t>(statusPrefix.psxAddress),
            true,
            bridge.adapter.feedback);
    const PrSceneEntryDirect::Call800191E4Result80015788& call191E4 =
        call19414.call800191E4;
    const bool call191E4SideEffectsKnown =
        bridge.adapter.call800191E4ReturnUnresolvedCurrentIda &&
        call191E4.sourceFunction == 0x800191E4u &&
        call191E4.arg0Known &&
        static_cast<uint32_t>(call191E4.arg0) == statusPrefix.psxAddress &&
        call191E4.modeKnown && call191E4.mode == 3 &&
        call191E4.called80026784 &&
        call191E4.sub80026784ResultKnown &&
        static_cast<uint32_t>(call191E4.sub80026784Result) ==
            PrSceneEntryDirect::kMemcardStateTable800544F8 &&
        call191E4.copied36BytesTo8007CC50 &&
        call191E4.wroteGp716Zero &&
        call191E4.wroteGp732Mode &&
        call191E4.called80017524 &&
        call191E4.called80018FB0 &&
        call191E4.callback80018E10 == 0x80018E10u &&
        call191E4.callback80019D7C == 0x80019D7Cu &&
        call191E4.stateMachineArg3 == 20 &&
        call191E4.stateMachineArg4 == 3 &&
        call191E4.called80017574 &&
        call191E4.missingGp716AfterStateMachine &&
        !call191E4.gp716AfterStateMachineKnown &&
        !call191E4.wordA1Plus44Known &&
        !call191E4.resultKnown;
    const uint8_t* tableBytes = static_cast<const uint8_t*>(
        call19414.event6HostArgPtr);
    if (!call191E4SideEffectsKnown ||
        call19414.sourceFunction != 0x80019414u ||
        !call19414.resultKnown ||
        !call19414.called80019284 ||
        call19414.call80019284Function != 0x80019284u ||
        static_cast<uint32_t>(call19414.result) !=
            bridge.adapter.tablePsxAddress ||
        tableBytes == nullptr ||
        bridge.adapter.tableByteCount !=
            PrSceneEntryDirect::kHiScoreTableSize80019284) {
        Log::Printf(
            "SS0 direct runtime: HI-SCORE table event6 carrier gap: "
            "call191E4SideEffects=%d call191E4ReturnUnresolved=%d "
            "call191E4ResultKnown=%d call191E4Result=%d "
            "call19414ResultKnown=%d called80019284=%d tablePsx=%08X tableBytes=%zu",
            call191E4SideEffectsKnown ? 1 : 0,
            bridge.adapter.call800191E4ReturnUnresolvedCurrentIda ? 1 : 0,
            call191E4.resultKnown ? 1 : 0,
            call191E4.result,
            call19414.resultKnown ? 1 : 0,
            call19414.called80019284 ? 1 : 0,
            call19414.resultKnown
                ? static_cast<uint32_t>(call19414.result)
                : 0u,
            bridge.adapter.tableByteCount);
        PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
        return false;
    }

    s_ss0Direct.hiScoreEvent6TableCarrierKnown = true;
    s_ss0Direct.hiScoreEvent6TablePsxAddress =
        static_cast<uint32_t>(call19414.result);
    s_ss0Direct.hiScoreEvent6TableByteCount =
        bridge.adapter.tableByteCount;
    std::copy(tableBytes,
              tableBytes + bridge.adapter.tableByteCount,
              s_ss0Direct.hiScoreEvent6TableBytes.begin());
    PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
    Log::Printf(
        "SS0 direct runtime: HI-SCORE event6 table carrier ready "
        "call191E4=1 call191E4ResultKnown=0 gp716After=unknown "
        "gp720=1 call19414=1 "
        "tablePsx=%08X tableBytes=%zu",
        s_ss0Direct.hiScoreEvent6TablePsxAddress,
        s_ss0Direct.hiScoreEvent6TableByteCount);
    return true;
}

static bool SS0DirectBuildProbeOnlyHiScoreEvent6EntryTable80019284() {
    // Entry-only regression fixture. It does not stand in for the production
    // 80019D7C -> 800179B4 Case17 card-read authority used above.
    SS0DirectClearHiScoreEvent6TableCarrier();
    const PrStage1SaveStatusPrefix80092F10 statusPrefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    if (!statusPrefix.known || statusPrefix.helperGap ||
        !statusPrefix.statusBankKnown80092F1D ||
        statusPrefix.psxAddress !=
            PrStage1SaveStatusPrefix80092F10::kPsxAddress ||
        statusPrefix.byteCount !=
            PrStage1SaveStatusPrefix80092F10::kByteCount) {
        return false;
    }

    const PrSceneEntryDirect::HiScoreBankCarrier800164F8 emptyBank =
        PrSceneEntryDirect::PsxCall800168DC_ClearHiScoreBank80019D7C();
    const PrSceneEntryDirect::Call80019284InputMemory80015788 input =
        PrSceneEntryDirect::PsxBuild80019284InputMemoryFromStatusAndBank80015788(
            statusPrefix.bytes,
            statusPrefix.byteCount,
            emptyBank);
    if (!input.inputMemoryKnown) {
        return false;
    }

    const PrSceneEntryDirect::Call80019284Result80015788 built =
        PrSceneEntryDirect::PsxCall80019284_BuildHiScoreRecords80015788(
            static_cast<int32_t>(statusPrefix.psxAddress),
            true,
            input.a1Memory,
            sizeof(input.a1Memory),
            input.initialTableMemory,
            sizeof(input.initialTableMemory));
    if (!built.resultKnown ||
        static_cast<uint32_t>(built.result) !=
            PrSS0HiScoreRenderDirect::kHiScoreTableAddress80049278 ||
        !built.tableAsciiKnown || !built.tablePsxGlyphBytesKnown) {
        return false;
    }

    s_ss0Direct.hiScoreEvent6TableCarrierKnown = true;
    s_ss0Direct.hiScoreEvent6TablePsxAddress =
        static_cast<uint32_t>(built.result);
    s_ss0Direct.hiScoreEvent6TableByteCount = sizeof(built.tableBytes);
    std::copy(std::begin(built.tableBytes),
              std::end(built.tableBytes),
              s_ss0Direct.hiScoreEvent6TableBytes.begin());
    Log::Printf(
        "SS0 direct runtime: probe-only Event6 entry table ready nonAuthority=1 entryReferenceOnly=1 tablePsx=%08X tableBytes=%zu",
        s_ss0Direct.hiScoreEvent6TablePsxAddress,
        s_ss0Direct.hiScoreEvent6TableByteCount);
    return true;
}

static PrSS0HiScoreRenderDirect::HiScoreTableDrawList80021594
SS0DirectBuildHiScoreTableDrawList80021594() {
    namespace HiScore = PrSS0HiScoreRenderDirect;
    HiScore::HiScoreEmptyTableInput80021594 input{};
    input.requestBound80019284 =
        s_ss0Direct.hiScoreEvent6TableCarrierKnown &&
        s_ss0Direct.hiScoreEvent6TablePsxAddress ==
            HiScore::kHiScoreTableAddress80049278 &&
        s_ss0Direct.hiScoreEvent6TableByteCount ==
            HiScore::kHiScoreTableSize80019284;
    input.tableBytes80019284 =
        s_ss0Direct.hiScoreEvent6TableBytes.data();
    input.tableByteCount80019284 =
        s_ss0Direct.hiScoreEvent6TableByteCount;
    input.languageKnown = true;
    input.language = s_ss0Direct.optionsWord800916D8;
    input.translatedLiveExitIconStateKnown = true;
    input.translatedLiveExitIconState = s_ss0Direct.hiScoreBlink;

    const std::size_t labelOffset =
        HiScore::kHiScoreExitLabelStateOffset80021594;
    const auto& table = s_ss0Direct.hiScoreEvent6TableBytes;
    const uint32_t labelState =
        static_cast<uint32_t>(table[labelOffset]) |
        static_cast<uint32_t>(table[labelOffset + 1u]) << 8u |
        static_cast<uint32_t>(table[labelOffset + 2u]) << 16u |
        static_cast<uint32_t>(table[labelOffset + 3u]) << 24u;
    input.requestBoundExitLabelStateKnown = input.requestBound80019284;
    input.requestBoundExitLabelState = static_cast<int32_t>(labelState);
    return HiScore::BuildHiScoreTableDrawList80021594(input);
}

static bool SS0DirectBackupReplayPayload80015700(const char* reason) {
    PrStage1SaveStatusPrefix80092F10 prefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    if (!prefix.known ||
        prefix.helperGap ||
        prefix.psxAddress != PrStage1SaveStatusPrefix80092F10::kPsxAddress ||
        prefix.byteCount != PrStage1SaveStatusPrefix80092F10::kByteCount) {
        Log::Printf(
            "SS0 direct runtime: %s replay payload backup skipped: direct prefix shape invalid known=%d helperGap=%d addr=%08X bytes=%u",
            reason ? reason : "replay",
            prefix.known ? 1 : 0,
            prefix.helperGap ? 1 : 0,
            prefix.psxAddress,
            prefix.byteCount);
        return false;
    }
    if (!prefix.statusBankKnown80092F1D) {
        Log::Printf(
            "SS0 direct runtime: %s replay payload backup skipped: direct status bank unknown",
            reason ? reason : "replay");
        return false;
    }

    const PrStage1SaveStatusBackupResult80015700 backup =
        PrStage1SaveUiDirect::Sub80015700(
            PrStage1SaveStatusPrefix80092F10::kPsxAddress);
    const bool ok = backup.ok && backup.backupKnown &&
                    backup.backupStatusBankKnown80092F1D;
    Log::Printf(
        "SS0 direct runtime: %s replay payload backup 80015700 ok=%d backup=%d backupStatus=%d fault=%08X",
        reason ? reason : "replay",
        ok ? 1 : 0,
        backup.backupKnown ? 1 : 0,
        backup.backupStatusBankKnown80092F1D ? 1 : 0,
        backup.lastFaultAddress);
    return ok;
}

static bool SS0DirectCommitTypedPayload800164B4(
    const char* reason,
    const SS0DirectTypedPayloadCommitSeam& payload,
    PrStage1SavePayloadProducerResult* outResult = nullptr) {
    if (outResult) {
        *outResult = {};
    }
    if (!payload.known ||
        payload.byteCount != PrStagePayloadBankDirect::kByteCount80092F10) {
        Log::Printf(
            "SS0 direct runtime: %s typed payload commit blocked before 800164B4: seamKnown=%d block=%d bytes=%zu expected=%u",
            reason ? reason : "memcard",
            payload.known ? 1 : 0,
            payload.blockIndex,
            payload.byteCount,
            PrStagePayloadBankDirect::kByteCount80092F10);
        return false;
    }

    const PrStage1SavePayloadProducerResult result =
        PrStage1SaveUiDirect::CommitTypedPayload800164B4(
            PrStage1SaveCardHalDirect::kCardReadPayloadAddr8007ADE8,
            payload.bytes.data(),
            payload.byteCount,
            payload.payloadAuthority800164B4);
    const PrStage1SaveStatusPrefix80092F10 committedPrefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    const bool commitBackingProvenanceIs800164B4 =
        committedPrefix.replayPayloadBackingProvenance80092F5C ==
        PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
            TypedCardCopy800164B4;
    const bool commitAccepted =
        result.ok &&
        result.payloadKnown &&
        committedPrefix.statusBankKnown80092F1D &&
        commitBackingProvenanceIs800164B4;
    if (outResult) {
        *outResult = result;
    }
    Log::Printf(
        "SS0 direct runtime: %s typed payload commit via 800164B4 ok=%d block=%d bytes=%zu payloadKnown=%d statusKnown=%d backingProvenance=%u writer=%08X wrote164B4=%d helperGap=%d lastFault=%08X",
        reason ? reason : "memcard",
        commitAccepted ? 1 : 0,
        payload.blockIndex,
        payload.byteCount,
        result.payloadKnown ? 1 : 0,
        committedPrefix.statusBankKnown80092F1D ? 1 : 0,
        static_cast<unsigned>(
            committedPrefix.replayPayloadBackingProvenance80092F5C),
        committedPrefix.lastWriterFunction,
        committedPrefix.wrote800164B4 ? 1 : 0,
        result.helperGap ? 1 : 0,
        result.lastFaultAddress);
    return commitAccepted;
}

static bool SS0DirectCanRollbackTypedPayloadCommit80092F10(
    const PrStage1SaveStatusPrefix80092F10& prefix) {
    return prefix.known &&
           !prefix.helperGap &&
           prefix.psxAddress ==
               PrStage1SaveStatusPrefix80092F10::kPsxAddress &&
           prefix.byteCount ==
               PrStage1SaveStatusPrefix80092F10::kByteCount &&
           prefix.statusBankKnown80092F1D;
}

static bool SS0DirectRollbackTypedPayloadCommit80092F10(
    const char* reason,
    const PrStage1SaveStatusPrefix80092F10& prefix) {
    if (!SS0DirectCanRollbackTypedPayloadCommit80092F10(prefix)) {
        PrStage1SaveUiDirect::InvalidateSaveStatusPrefixAuthority80092F10(
            PrStage1SaveStatusPrefix80092F10::kPsxAddress);
        s_ss0Direct.replayPayloadBackupPreflightKnown = false;
        Log::Printf(
            "SS0 direct runtime: %s typed payload rollback fail-closed: prefixKnown=%d helperGap=%d statusKnown=%d addr=%08X bytes=%u",
            reason ? reason : "memcard",
            prefix.known ? 1 : 0,
            prefix.helperGap ? 1 : 0,
            prefix.statusBankKnown80092F1D ? 1 : 0,
            prefix.psxAddress,
            prefix.byteCount);
        return false;
    }
    const PrStage1SaveStatusBackupResult80015700 restore =
        PrStage1SaveUiDirect::Sub80015744(
            PrStage1SaveStatusPrefix80092F10::kPsxAddress);
    const PrStage1SaveStatusPrefix80092F10 restored =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    const bool bytesRestored =
        restored.known && restored.statusBankKnown80092F1D &&
        std::memcmp(restored.bytes, prefix.bytes, prefix.byteCount) == 0;
    const bool provenanceRestored =
        restored.replayPayloadBackingProvenance80092F5C ==
        prefix.replayPayloadBackingProvenance80092F5C;
    const bool ok = restore.ok && restore.restoreKnown && bytesRestored &&
                     provenanceRestored;
    if (!ok) {
        PrStage1SaveUiDirect::InvalidateSaveStatusPrefixAuthority80092F10(
            PrStage1SaveStatusPrefix80092F10::kPsxAddress);
        s_ss0Direct.replayPayloadBackupPreflightKnown = false;
    }
    Log::Printf(
        "SS0 direct runtime: %s typed payload rollback 80015744 ok=%d restore=%d bytes=%d provenance=%d sourceProvenance=%u restoredProvenance=%u writer=%08X",
        reason ? reason : "memcard",
        ok ? 1 : 0,
        restore.restoreKnown ? 1 : 0,
        bytesRestored ? 1 : 0,
        provenanceRestored ? 1 : 0,
        static_cast<unsigned>(
            prefix.replayPayloadBackingProvenance80092F5C),
        static_cast<unsigned>(
            restored.replayPayloadBackingProvenance80092F5C),
        prefix.lastWriterFunction);
    return ok;
}

static bool SS0DirectLoadReplaySelectorFromCommittedPayload80092F3C(
    const char* reason,
    SS0DirectDebugReplayLoadResult* outReplayLoad) {
    if (!outReplayLoad) {
        return false;
    }
    SS0DirectDebugReplayLoadResult candidate{};
    candidate.payloadCommitted = true;
    const PrStage1SaveStatusPrefix80092F10 committedPrefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    const bool committedPrefixIs800164B4 =
        committedPrefix.known &&
        committedPrefix.psxAddress ==
            PrStage1SaveStatusPrefix80092F10::kPsxAddress &&
        committedPrefix.byteCount ==
            PrStage1SaveStatusPrefix80092F10::kByteCount &&
        committedPrefix.statusBankKnown80092F1D &&
        committedPrefix.replayPayloadBackingProvenance80092F5C ==
            PrStagePayloadBankDirect::ReplayPayloadBackingProvenance80092F5C::
                TypedCardCopy800164B4;
    if (!committedPrefixIs800164B4) {
        Log::Printf(
            "SS0 direct runtime: %s replay selector blocked: committed 800164B4 authority missing known=%d statusKnown=%d backingProvenance=%u writer=%08X wrote164B4=%d psx=%08X bytes=%u",
            reason ? reason : "replay",
            committedPrefix.known ? 1 : 0,
            committedPrefix.statusBankKnown80092F1D ? 1 : 0,
            static_cast<unsigned>(
                committedPrefix.replayPayloadBackingProvenance80092F5C),
            committedPrefix.lastWriterFunction,
            committedPrefix.wrote800164B4 ? 1 : 0,
            committedPrefix.psxAddress,
            committedPrefix.byteCount);
        return false;
    }
    const PrStageClearStatusBankSnapshot snapshot =
        PrStage1SaveUiDirect::GetStageClearStatusBankSnapshot();
    if (!snapshot.lastSavedSlotKnown80092F3C) {
        Log::Printf(
            "SS0 direct runtime: %s replay selector blocked: direct 80092F3C unknown after payload commit",
            reason ? reason : "replay");
        return false;
    }

    candidate.rawSavedSlotKnown = true;
    candidate.rawSavedSlot = snapshot.dword80092F3C;
    int replayScene = -1;
    if (snapshot.dword80092F3C <= 0x7FFFFFFFu &&
        PrSS0StageProgressBankDirect::TryMapReplayScene800161A8(
            static_cast<int32_t>(snapshot.dword80092F3C),
            &replayScene)) {
        candidate.replayScene = replayScene;
    }
    *outReplayLoad = candidate;
    Log::Printf(
        "SS0 direct runtime: %s replay selector 80092F3C known=1 rawSel=%u replayScene=%d payloadCommitted=1",
        reason ? reason : "replay",
        outReplayLoad->rawSavedSlot,
        outReplayLoad->replayScene);
    return true;
}

static void SS0DirectRecomputeBonusStatus(uint8_t status[8]) {
    bool allPrimaryClear = true;
    for (int stage = 1; stage <= 6; ++stage) {
        status[stage] = static_cast<uint8_t>(
            std::clamp(static_cast<int>(status[stage]), 0, 3));
        allPrimaryClear = allPrimaryClear && status[stage] >= 3u;
    }
    status[7] = allPrimaryClear ? 1u : 0u;
}

static bool SS0DirectReadDirectBankStageStatuses(uint8_t status[8]) {
    if (!status) {
        return false;
    }

    const PrStageClearStatusBankSnapshot snapshot =
        PrStage1SaveUiDirect::GetStageClearStatusBankSnapshot();
    if (!snapshot.statusBytesKnown80092F1D) {
        Log::Printf(
            "SS0 direct runtime: debug status read blocked: direct status bank unknown");
        return false;
    }

    std::fill(status, status + 8, 0u);
    for (int stage = 1; stage <= 6; ++stage) {
        status[stage] = static_cast<uint8_t>(
            std::clamp(static_cast<int>(
                           snapshot.byte80092F1D[stage - 1]),
                       0,
                       3));
    }
    SS0DirectRecomputeBonusStatus(status);
    return true;
}

static bool SS0DirectPublishStageStatuses(uint8_t status[8],
                                          const char* reason) {
    if (!status) {
        return false;
    }
    const PrStage1SaveStatusPrefix80092F10 currentPrefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    if (!currentPrefix.known ||
        !currentPrefix.statusBankKnown80092F1D ||
        currentPrefix.psxAddress != PrStage1SaveStatusPrefix80092F10::kPsxAddress ||
        currentPrefix.byteCount != PrStage1SaveStatusPrefix80092F10::kByteCount) {
        Log::Printf(
            "SS0 direct runtime: %s debug status publish skipped direct-prefix known=%d statusKnown=%d addr=%08X bytes=%u",
            reason ? reason : "stage-status",
            currentPrefix.known ? 1 : 0,
            currentPrefix.statusBankKnown80092F1D ? 1 : 0,
            currentPrefix.psxAddress,
            currentPrefix.byteCount);
        return false;
    }

    SS0DirectRecomputeBonusStatus(status);
    std::array<uint8_t, PrStage1SaveStatusPrefix80092F10::kByteCount> scratch{};
    std::memcpy(scratch.data(),
                currentPrefix.bytes,
                PrStage1SaveStatusPrefix80092F10::kByteCount);
    scratch[kStageStatusPayloadBase] = 0u;
    for (int stage = 1; stage <= 7; ++stage) {
        scratch[kStageStatusPayloadBase + static_cast<std::size_t>(stage)] =
            status[stage];
    }

    PrStage1SaveStatusPrefix80092F10 seed{};
    seed.known = true;
    seed.statusBankKnown80092F1D = true;
    seed.psxAddress = PrStage1SaveStatusPrefix80092F10::kPsxAddress;
    seed.byteCount = PrStage1SaveStatusPrefix80092F10::kByteCount;
    std::memcpy(seed.bytes,
                scratch.data(),
                PrStage1SaveStatusPrefix80092F10::kByteCount);

    const bool imported =
        PrStage1SaveUiDirect::ImportSaveStatusPrefix80092F10(seed);
    Log::Printf(
        "SS0 direct runtime: %s debug status [%u,%u,%u,%u,%u,%u] bonus=%u import=%d",
        reason ? reason : "stage-status",
        static_cast<unsigned>(status[1]),
        static_cast<unsigned>(status[2]),
        static_cast<unsigned>(status[3]),
        static_cast<unsigned>(status[4]),
        static_cast<unsigned>(status[5]),
        static_cast<unsigned>(status[6]),
        static_cast<unsigned>(status[7]),
        imported ? 1 : 0);
    return imported;
}

static bool SS0DirectDebugStatusOwned(const PrGameContext& ctx,
                                      const char* reason) {
    const bool runtimeEnabled = PrSS0Direct::RuntimeCutoverAllowed();
    const bool phaseAllowed =
        s_ss0Direct.phase == SS0DirectPhase::TitleSelector;
    if (!runtimeEnabled ||
        !s_ss0Direct.initialized ||
        (ctx.currentScene != PrSceneId::Scene0 &&
         ctx.currentScene != PrSceneId::Scene1) ||
        !phaseAllowed) {
        Log::Printf(
            "SS0 direct runtime: %s debug status blocked scene=%u runtime=%d initialized=%d phase=%u phaseAllowed=%d",
            reason ? reason : "stage-status",
            static_cast<unsigned>(ctx.currentScene),
            runtimeEnabled ? 1 : 0,
            s_ss0Direct.initialized ? 1 : 0,
            static_cast<unsigned>(s_ss0Direct.phase),
            phaseAllowed ? 1 : 0);
        return false;
    }
    return true;
}

static void SS0DirectDestroyStrDirectTexture() {
    if (s_ss0Direct.strDirectMdecTexture != nullptr &&
        s_ss0Direct.strDirectMdecTextureRenderer != nullptr) {
        s_ss0Direct.strDirectMdecTextureRenderer->DestroyTexture(
            s_ss0Direct.strDirectMdecTexture);
    }
    s_ss0Direct.strDirectMdecTexture = nullptr;
    s_ss0Direct.strDirectMdecTextureRenderer = nullptr;
    s_ss0Direct.strDirectMdecTextureReady = false;
    s_ss0Direct.strDirectMdecTextureFrameNumber = 0u;
    s_ss0Direct.strDirectMdecTextureUploadCount = 0u;
    s_ss0Direct.strStripUploadExecution8002756C = {};
}

static void SS0DirectReleaseStrXaAudioVoice8001A4D0() {
    if (s_ss0Direct.strXaAudioVoice >= 0) {
        const int voice = s_ss0Direct.strXaAudioVoice;
        const uint64_t generation = s_ss0Direct.strXaAudioVoiceGeneration;
        const bool activeBefore =
            AudioEngine::Get().IsVoiceLeaseActive(voice, generation);
        const bool freed = AudioEngine::Get().FreeVoiceIfGeneration(
            s_ss0Direct.strXaAudioVoice,
            s_ss0Direct.strXaAudioVoiceGeneration);
        Log::Printf(
            "SS0 direct runtime: 8001A4D0 XA output release voice=%d generation=%llu activeBefore=%d freed=%d activeAfter=%d",
            voice,
            static_cast<unsigned long long>(generation),
            activeBefore ? 1 : 0,
            freed ? 1 : 0,
            AudioEngine::Get().IsVoiceLeaseActive(voice, generation) ? 1 : 0);
    }
    s_ss0Direct.strXaAudioVoice = -1;
    s_ss0Direct.strXaAudioVoiceGeneration = 0u;
    s_ss0Direct.strXaAudioSampleRate = 0u;
    s_ss0Direct.strXaAudioChannels = 0u;
    s_ss0Direct.strXaAudioOutputAdapterActive = false;
}

static void SS0DirectStopStrXaAudioOutputAdapter8001A4D0() {
    SS0DirectReleaseStrXaAudioVoice8001A4D0();
    s_ss0Direct.strXaAudioDecoder.Reset();
    s_ss0Direct.strXaAudioDisc8001A4D0 = {};
    s_ss0Direct.strXaAudioCoding = 0u;
    s_ss0Direct.strXaAudioDecodedSectors = 0u;
    s_ss0Direct.strXaAudioQueuedSamples = 0u;
    s_ss0Direct.strXaAudioOutputVolume = 1.0f;
    s_ss0Direct.strXaAudioOutputAdapterKnown = false;
}

static void SS0DirectStopLegacyHostStrPlayer(
    PrGameContext& ctx,
    const char* owner) {
    if (ctx.strPlayer == nullptr) {
        return;
    }
    const bool hostStrActive =
        ctx.strPlayer->IsPlaying() || ctx.strPlayer->IsPaused() ||
        ctx.strPlayer->IsFinished() || ctx.strPlayer->HasAudio();
    if (!hostStrActive) {
        return;
    }
    // SS0's 8001A4D0/80027664 owners are direct original-disc state.  A
    // stale process-level StrPlayer is a separate Windows audio owner and
    // would otherwise continue emitting its queued XA samples after a PSX
    // pad skip.  Stop it at the direct boundary before it can be mixed with
    // the translated title stream.
    ctx.strPlayer->Stop();
    Log::Printf(
        "SS0 direct runtime: %s legacy host STR player stopped at direct boundary",
        owner ? owner : "Scene0");
}

static bool SS0DirectPumpStrXaAudioOutputAdapter8001A4D0(
    uint32_t targetRawSectorCursor,
    const char* owner,
    bool allowQueueGrowth = false) {
    auto& discRuntime = s_ss0Direct.strXaAudioDisc8001A4D0;
    if (!discRuntime.initialized ||
        !discRuntime.currentScusSemanticAuthority ||
        !discRuntime.currentComod0CallerAuthority ||
        !discRuntime.discImagePayloadAuthority ||
        !discRuntime.directXaSectorSelectionAuthority ||
        discRuntime.psxCdXaDecodeAuthority ||
        discRuntime.psxSpuHardwareAuthority ||
        discRuntime.hostExtractedFileAuthority ||
        discRuntime.replayValueAuthority ||
        discRuntime.oldWinS0Authority ||
        discRuntime.stage2PlusAuthority ||
        discRuntime.comod2Authority) {
        return false;
    }

    // Normal playback keeps the same short queue governor used by the
    // translated 8001A4D0 consumer.  At the exact 801C4894 video-finished
    // boundary the original CD stop can no longer feed future sectors, so
    // prime the already-selected XA tail in one bounded pass before stopping
    // the stream.  This is still the original-disc sector path; it is not a
    // host-extracted STR or a synthetic tone.
    static constexpr uint32_t kMaxAudioSectorsPerTick = 32u;
    const uint32_t maxAudioSectors =
        allowQueueGrowth ? (std::numeric_limits<uint32_t>::max)()
                         : kMaxAudioSectorsPerTick;
    uint32_t sectorsVisited = 0u;
    while (discRuntime.rawSectorCursor < targetRawSectorCursor &&
           sectorsVisited < maxAudioSectors) {
        if (s_ss0Direct.strXaAudioVoice >= 0 &&
            AudioEngine::Get().IsVoiceLeaseActive(
                s_ss0Direct.strXaAudioVoice,
                s_ss0Direct.strXaAudioVoiceGeneration)) {
            const size_t queued = AudioEngine::Get().GetVoiceQueuedSamples(
                s_ss0Direct.strXaAudioVoice);
            const size_t halfSecondSamples =
                static_cast<size_t>(s_ss0Direct.strXaAudioSampleRate) *
                static_cast<size_t>(s_ss0Direct.strXaAudioChannels) / 2u;
            if (!allowQueueGrowth && halfSecondSamples != 0u &&
                queued >= halfSecondSamples) {
                break;
            }
        }

        const auto sector =
            PrSS0StrLifecycleDirect::
                ReadNextStrXaAudioSectorFromDisc8001A4D0(
                    s_ss0Direct.strLowerCdStart8001A4D0,
                    targetRawSectorCursor,
                    discRuntime);
        if (!sector.known ||
            !sector.currentScusSemanticAuthority ||
            !sector.currentComod0CallerAuthority ||
            !sector.discImagePayloadAuthority ||
            !sector.directXaSectorSelectionAuthority ||
            sector.psxCdXaDecodeAuthority ||
            sector.psxSpuHardwareAuthority ||
            sector.hostExtractedFileAuthority ||
            sector.replayValueAuthority ||
            sector.oldWinS0Authority ||
            sector.stage2PlusAuthority ||
            sector.comod2Authority) {
            return false;
        }
        ++sectorsVisited;
        if (!sector.available) {
            break;
        }

        if (s_ss0Direct.strXaAudioDecodedSectors != 0u &&
            sector.coding != s_ss0Direct.strXaAudioCoding) {
            SS0DirectReleaseStrXaAudioVoice8001A4D0();
            s_ss0Direct.strXaAudioDecoder.Reset();
        }
        s_ss0Direct.strXaAudioCoding = sector.coding;
        if (!s_ss0Direct.strXaAudioDecoder.DecodeSector(
                sector.payload.data(),
                sector.payload.size(),
                sector.coding)) {
            s_ss0Direct.strXaAudioOutputAdapterKnown = true;
            Log::Printf(
                "SS0 direct runtime: %s original-disc XA output decode rejected cursor=%u coding=%02X; direct sector selection remains authoritative",
                owner ? owner : "Scene0",
                sector.matchedRawSectorCursor,
                static_cast<unsigned>(sector.coding));
            continue;
        }

        const auto& samples = s_ss0Direct.strXaAudioDecoder.GetSamples();
        const uint32_t sampleRate =
            s_ss0Direct.strXaAudioDecoder.GetSampleRate();
        const uint32_t channels =
            s_ss0Direct.strXaAudioDecoder.IsStereo() ? 2u : 1u;
        if (s_ss0Direct.strXaAudioVoice < 0 ||
            !AudioEngine::Get().IsVoiceLeaseActive(
                s_ss0Direct.strXaAudioVoice,
                s_ss0Direct.strXaAudioVoiceGeneration)) {
            SS0DirectReleaseStrXaAudioVoice8001A4D0();
            if (AudioEngine::Get().IsRunning()) {
                s_ss0Direct.strXaAudioVoice =
                    AudioEngine::Get().AllocVoice(
                        static_cast<int>(channels),
                        sampleRate,
                        s_ss0Direct.strXaAudioOutputVolume);
                if (s_ss0Direct.strXaAudioVoice >= 0) {
                    s_ss0Direct.strXaAudioVoiceGeneration =
                        AudioEngine::Get().GetVoiceGeneration(
                            s_ss0Direct.strXaAudioVoice);
                    Log::Printf(
                        "SS0 direct runtime: %s original-disc XA output adapter voice=%d generation=%llu rate=%u channels=%u coding=%02X hostExtractedFileAuthority=0 psxCdXaDecodeAuthority=0 psxSpuHardwareAuthority=0",
                        owner ? owner : "Scene0",
                        s_ss0Direct.strXaAudioVoice,
                        static_cast<unsigned long long>(
                            s_ss0Direct.strXaAudioVoiceGeneration),
                        sampleRate,
                        channels,
                        static_cast<unsigned>(sector.coding));
                }
            }
        }
        s_ss0Direct.strXaAudioSampleRate = sampleRate;
        s_ss0Direct.strXaAudioChannels = channels;
        if (s_ss0Direct.strXaAudioVoice >= 0 &&
            AudioEngine::Get().IsVoiceLeaseActive(
                s_ss0Direct.strXaAudioVoice,
                s_ss0Direct.strXaAudioVoiceGeneration)) {
            AudioEngine::Get().QueueSamples(
                s_ss0Direct.strXaAudioVoice,
                samples.data(),
                samples.size());
            s_ss0Direct.strXaAudioQueuedSamples += samples.size();
            s_ss0Direct.strXaAudioOutputAdapterActive = true;
        }
        ++s_ss0Direct.strXaAudioDecodedSectors;
        s_ss0Direct.strXaAudioOutputAdapterKnown = true;
    }
    return true;
}

static bool SS0DirectExecuteStrStopReset80027664(
    const char* owner,
    bool preserveXaAudio = false) {
    const PrSS0StrLifecycleDirect::StrStopResetState80027664 stopReset =
        PrSS0StrLifecycleDirect::ExecuteStrStopReset80027664(
            s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (stopReset.runtimeWasActive ||
        !s_ss0Direct.strStopReset80027664.executionAccepted) {
        s_ss0Direct.strStopReset80027664 = stopReset;
    }
    if (!stopReset.known ||
        (stopReset.runtimeWasActive &&
         (!stopReset.executionAccepted ||
          !stopReset.dmaCallbackClearExecuted ||
          !stopReset.cdStopExecuted ||
          !stopReset.memoryResetExecuted ||
          !stopReset.sectorRingReset ||
          !stopReset.directDmaCallbackStateAuthority ||
          !stopReset.directCdStateAuthority ||
          !stopReset.directMemoryStackAuthority ||
          stopReset.psxPointerAuthority ||
          stopReset.psxHardwareMmioAuthority ||
          stopReset.hardwareCallbackTimingAuthority ||
          stopReset.hostProjection ||
          stopReset.replayValueAuthority ||
          stopReset.oldWinS0Authority ||
          stopReset.stage2PlusAuthority ||
          stopReset.comod2Authority))) {
        Log::Printf(
            "SS0 direct runtime: %s 80027664 direct stop/reset blocked active=%u known=%u accepted=%u",
            owner ? owner : "Scene0",
            stopReset.runtimeWasActive ? 1u : 0u,
            stopReset.known ? 1u : 0u,
            stopReset.executionAccepted ? 1u : 0u);
        return false;
    }
    Log::Printf(
        "SS0 direct runtime: %s 80027664 stop/reset known=%u active=%u inactiveSkip=%u callback=%08X arg=%d channel=%u stopCd=%08X critical=%u dataClear=%u readyClear=%u reg0Clear=%u reg3Clear=%u memoryReset=%08Xx%u executed=%u released=%llu ringReset=%u directDmaCallbackStateAuthority=%u directCdStateAuthority=%u directMemoryStackAuthority=%u psxPointerAuthority=0 psxHardwareMmioAuthority=0 hardwareCallbackTimingAuthority=0 hostProjection=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "Scene0",
        stopReset.known ? 1u : 0u,
        stopReset.runtimeWasActive ? 1u : 0u,
        stopReset.executionSkippedInactive ? 1u : 0u,
        PrSS0StrLifecycleDirect::kFn80047658,
        stopReset.clearDmaCallbackArgument,
        stopReset.clearDmaCallbackChannel,
        PrSS0StrLifecycleDirect::kFn800392C0,
        stopReset.enterCriticalSectionExecuted ? 1u : 0u,
        stopReset.cdDataCallbackClearExecuted80036930 ? 1u : 0u,
        stopReset.cdReadyCallbackClearExecuted80036528 ? 1u : 0u,
        stopReset.cdromReg0ClearExecuted ? 1u : 0u,
        stopReset.cdromReg3ClearExecuted ? 1u : 0u,
        PrSS0StrLifecycleDirect::kFn80025AF8,
        stopReset.memoryResetCallCount80025AF8,
        stopReset.memoryResetCountExecuted,
        static_cast<unsigned long long>(
            stopReset.memoryBytesReleased),
        stopReset.sectorRingReset ? 1u : 0u,
        stopReset.directDmaCallbackStateAuthority ? 1u : 0u,
        stopReset.directCdStateAuthority ? 1u : 0u,
        stopReset.directMemoryStackAuthority ? 1u : 0u);
    if (!preserveXaAudio) {
        SS0DirectStopStrXaAudioOutputAdapter8001A4D0();
    }
    if (stopReset.runtimeWasActive) {
        s_ss0Direct.strDecoderAllocation80027288 = {};
        s_ss0Direct.strStreamStart800274D4 = {};
        s_ss0Direct.strDecodeAdvance800273A4 = {};
        s_ss0Direct.strVlcDecodeRelease800273A4 = {};
        s_ss0Direct.strMdecCommandOutput800273A4 = {};
        s_ss0Direct.strMdecOutputCurrentScus = {};
        s_ss0Direct.strMdecCompletion80027220 = {};
        s_ss0Direct.strMdecCompletionPending = false;
        s_ss0Direct.strDirectMdecFramesDecoded = 0u;
        s_ss0Direct.strDirectMdecLastFrameNumber = 0u;
        s_ss0Direct.strDirectMdecStreamExhausted = false;
        s_ss0Direct.strDecodeGate80027528 = {};
        s_ss0Direct.strStripUpload8002756C = {};
    }
    return true;
}

static bool SS0DirectExecuteStrCleanupTail801C455C(
    const char* owner) {
    if (!SS0DirectExecuteStrStopReset80027664(owner)) {
        return false;
    }

    const auto prefix =
        PrSS0StrLifecycleDirect::ExecuteStrCleanupTailPrefix801C455C(
            s_ss0Direct.strDecoderMemoryRuntime80027288,
            s_ss0Direct.strCdSyncCallback800570F8,
            s_ss0Direct.strWorkBase80049428,
            PrSS0StrLifecycleDirect::kScene0WorkAddress);
    s_ss0Direct.strCleanupTailPrefix801C455C = prefix;
    if (!prefix.known || !prefix.executionAccepted ||
        !prefix.exactCallOrder ||
        !prefix.currentScusSemanticAuthority ||
        !prefix.currentComod0CallerAuthority ||
        !prefix.directSoftwareStateAuthority ||
        !prefix.stopReset80027664Observed ||
        !prefix.nextDisplayMovePending8001B120 ||
        prefix.psxSpuHardwareAuthority ||
        prefix.psxGpuDmaAuthority || prefix.hostProjection ||
        prefix.replayValueAuthority || prefix.oldWinS0Authority ||
        prefix.stage2PlusAuthority || prefix.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s cleanup prefix blocked volume=%u secondWait=%u nullsub=%u; boundary before 8001B120",
            owner ? owner : "801C455C",
            prefix.muteSerialVolume.known ? 1u : 0u,
            prefix.secondWaitCleanup.known ? 1u : 0u,
            prefix.contextNoOp.known ? 1u : 0u);
        return false;
    }

    const PrPsxGraphOwnerDirect::PsxGraphState* graph =
        PrSS0TitleTmdBackend::GetTitleGraphState();
    if (graph == nullptr || graph->word_80096590 > 1u ||
        !s_ss0Direct.strRootCallback80055F78.interruptMaskKnown) {
        Log::Printf(
            "SS0 direct runtime: %s 8001B120 binding blocked graph=%u lane=%u interruptMaskKnown=%u",
            owner ? owner : "801C455C",
            graph != nullptr ? 1u : 0u,
            graph != nullptr
                ? static_cast<unsigned>(graph->word_80096590)
                : 0u,
            s_ss0Direct.strRootCallback80055F78.interruptMaskKnown
                ? 1u : 0u);
        return false;
    }
    const auto displayMove =
        PrSS0StrLifecycleDirect::ExecuteStrDisplayMove8001B120(
            1,
            graph->word_80096590,
            true,
            s_ss0Direct.strRootCallback80055F78.interruptMask,
            s_ss0Direct.strVramPageRuntime8001B120);
    s_ss0Direct.strDisplayMove8001B120 = displayMove;
    if (!displayMove.known || !displayMove.executionAccepted ||
        !displayMove.exact8001B120Branch ||
        !displayMove.softwareVramMoveExecuted ||
        !displayMove.currentScusSemanticAuthority ||
        !displayMove.currentComod0CallerAuthority ||
        !displayMove.directVramPixelStateAuthority ||
        !displayMove.directGpuPacketStateAuthority ||
        !displayMove.translatedGpuDmaStateAuthority ||
        !displayMove.translatedGpuCompletionAuthority ||
        !displayMove.dma.softwarePacketStateCommitted ||
        !displayMove.dma.directDmaTransactionRecorded ||
        displayMove.psxGpuMmioAuthority ||
        displayMove.hardwareCompletionTimingAuthority ||
        displayMove.dma.hostMmioWritten ||
        displayMove.dma.hostGpuSubmitted ||
        displayMove.dma.exactPsxHalParity ||
        displayMove.hostProjection || displayMove.replayValueAuthority ||
        displayMove.oldWinS0Authority || displayMove.stage2PlusAuthority ||
        displayMove.comod2Authority) {
        const bool sourceKnown = displayMove.sourcePage <
                                     s_ss0Direct.strVramPageRuntime8001B120
                                         .pages.size()
                                     ? s_ss0Direct
                                           .strVramPageRuntime8001B120
                                           .pages[displayMove.sourcePage]
                                           .contentKnown
                                     : false;
        Log::Printf(
            "SS0 direct runtime: %s 8001B120 software VRAM move blocked sourcePage=%u destinationPage=%u sourceKnown=%u",
            owner ? owner : "801C455C",
            static_cast<unsigned>(displayMove.sourcePage),
            static_cast<unsigned>(displayMove.destinationPage),
            sourceKnown ? 1u : 0u);
        return false;
    }

    Log::Printf(
        "SS0 direct runtime: %s 80027664->8001A4A4(1)->8001A478->8002AB24 software volume=%u mask=%04X scaled=%04X/%04X -> second 8001A694 syncState=%u -> 80024CF0 no-op -> 8001B120(1) lane=%u page=%u->%u rectY=%d destY=%u pixels=%u rgbaFnv1a=%016llX packet=8005D7DC/80000000 -> translated 80044E2C/800468E0/80046840 GPU/DMA state committed; host GP1/DMA2 MMIO and hardware completion timing remain absent; translatedGpuDmaStateAuthority=1 translatedGpuCompletionAuthority=1 psxSpuHardwareAuthority=0 psxGpuMmioAuthority=0 hostProjection=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "801C455C",
        static_cast<unsigned>(prefix.muteSerialVolume.mappedVolume),
        static_cast<unsigned>(
            prefix.muteSerialVolume.attributeMask8002AB24),
        static_cast<unsigned>(
            prefix.muteSerialVolume.scaledLeftVolume8002AB24),
        static_cast<unsigned>(
            prefix.muteSerialVolume.scaledRightVolume8002AB24),
        static_cast<unsigned>(
            prefix.secondWaitCleanup.command8Pause.syncState800573D4),
        static_cast<unsigned>(displayMove.drawBufferSlot80096590),
        static_cast<unsigned>(displayMove.sourcePage),
        static_cast<unsigned>(displayMove.destinationPage),
        static_cast<int>(displayMove.sourceY),
        static_cast<unsigned>(displayMove.destinationY),
        displayMove.pixelCountCopied,
        static_cast<unsigned long long>(displayMove.destinationRgbaFnv1a64));
    return true;
}

static bool SS0DirectExecuteTitleWaitCleanup8001A694(
    const char* owner) {
    const auto cleanup =
        PrSS0StrLifecycleDirect::ExecuteStrWaitCleanup8001A694(
            s_ss0Direct.strDecoderMemoryRuntime80027288,
            s_ss0Direct.strCdSyncCallback800570F8,
            s_ss0Direct.strWorkBase80049428);
    s_ss0Direct.strWaitCleanup8001A694 = cleanup;
    if (!cleanup.known || !cleanup.called ||
        !cleanup.executionAccepted || !cleanup.exactCallOrder ||
        !cleanup.currentScusSemanticAuthority ||
        !cleanup.directLowerCdStateAuthority ||
        !cleanup.directCdCommandStateAuthority ||
        cleanup.hardwareCallbackTimingAuthority ||
        cleanup.hostProjection || cleanup.replayValueAuthority ||
        cleanup.oldWinS0Authority || cleanup.stage2PlusAuthority ||
        cleanup.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 8001A694 direct wait/callback cleanup blocked known=%u accepted=%u",
            owner ? owner : "801C4894",
            cleanup.known ? 1u : 0u,
            cleanup.executionAccepted ? 1u : 0u);
        return false;
    }
    // 801C4B74 (video skip), 801C4BDC (real-time animation skip), and
    // 801C4A60 (natural selector entry) all pause CD playback here.  The
    // preceding 80027664 only ends MDEC; its preserved XA tail belongs to
    // this CD playback and must stop at command 8, before the selector cue.
    // Applying only the translated command state left the host voice queued
    // throughout a state-1 skip, where 80027664 is deliberately not repeated.
    const bool audioTailPending = s_ss0Direct.titleMovie0TAudioTailPending;
    SS0DirectStopStrXaAudioOutputAdapter8001A4D0();
    s_ss0Direct.titleMovie0TAudioTailPending = false;
    s_ss0Direct.titleMovie0TAudioTailReleaseLogged = false;
    Log::Printf(
        "SS0 direct runtime: %s 8001A694 direct command-8 wait/callback cleanup committed titleState=%u xaTailWasPending=%u xaVoiceAfter=%d hostProjection=0 hostLifecycleAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "801C4894",
        static_cast<unsigned>(s_ss0Direct.titleLoopStateV8),
        audioTailPending ? 1u : 0u,
        s_ss0Direct.strXaAudioVoice);
    return true;
}

static bool SS0DirectExecuteOpeningMovie0OuterPostWait801C4E4C(
    const char* owner) {
    const PrPsxGraphOwnerDirect::PsxGraphState* graph =
        PrSS0TitleTmdBackend::GetTitleGraphState();
    if (graph == nullptr || graph->word_80096590 > 1u ||
        !s_ss0Direct.strRootCallback80055F78.interruptMaskKnown) {
        Log::Printf(
            "SS0 direct runtime: %s outer 801C4E4C post-wait binding blocked graph=%u lane=%u interruptMaskKnown=%u",
            owner ? owner : "801C4DC4",
            graph != nullptr ? 1u : 0u,
            graph != nullptr
                ? static_cast<unsigned>(graph->word_80096590)
                : 0u,
            s_ss0Direct.strRootCallback80055F78.interruptMaskKnown
                ? 1u : 0u);
        return false;
    }

    const auto postWait =
        PrSS0StrLifecycleDirect::ExecuteStrOuterPostWait801C4DC4(
            graph->word_80096590,
            true,
            s_ss0Direct.strRootCallback80055F78.interruptMask,
            s_ss0Direct.strVramPageRuntime8001B120);
    s_ss0Direct.strOuterPostWait801C4DC4 = postWait;
    if (!postWait.known || !postWait.called ||
        !postWait.exactCallerOrder ||
        !postWait.currentComod0SemanticAuthority ||
        !postWait.directSoftwareStateAuthority ||
        !postWait.playAndWaitReturn.known ||
        !postWait.playAndWaitReturn.activeCallerModeZero ||
        postWait.playAndWaitReturn.callerMode != 0 ||
        postWait.playAndWaitReturn.returnValue != 0 ||
        !postWait.secondDisplayMove.known ||
        !postWait.secondDisplayMove.executionAccepted ||
        !postWait.secondDisplayMove.softwareVramMoveExecuted ||
        !postWait.fastTransitionPending ||
        postWait.fastTransitionContext !=
            PrSS0TransitionDirect::kScene0WorkAddress ||
        postWait.fastTransitionMode != 5 ||
        postWait.fastTransitionPreArg != 1 ||
        postWait.fastTransitionPostArg != 2 ||
        !postWait.word800916D2WritePending ||
        postWait.word800916D2Value != 1u ||
        postWait.psxGpuMmioAuthority ||
        postWait.hardwareCompletionTimingAuthority ||
        postWait.hostProjection || postWait.replayValueAuthority ||
        postWait.oldWinS0Authority || postWait.stage2PlusAuthority ||
        postWait.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s outer 801C4E4C->801C4E54 post-wait sequence blocked returnKnown=%u return=%d moveKnown=%u transition=%u",
            owner ? owner : "801C4DC4",
            postWait.playAndWaitReturn.known ? 1u : 0u,
            postWait.playAndWaitReturn.returnValue,
            postWait.secondDisplayMove.known ? 1u : 0u,
            postWait.fastTransitionPending ? 1u : 0u);
        return false;
    }

    Log::Printf(
        "SS0 direct runtime: %s outer 801C4E4C PlayAndWait return=0 -> 801C4E54 second 8001B120(1) lane=%u page=%u->%u pixels=%u -> 801C4E70 800201AC(scene0,5,1,2) pending; actual GP1/DMA2 MMIO and asynchronous completion remain open; hostProjection=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "801C4DC4",
        static_cast<unsigned>(
            postWait.secondDisplayMove.drawBufferSlot80096590),
        static_cast<unsigned>(postWait.secondDisplayMove.sourcePage),
        static_cast<unsigned>(postWait.secondDisplayMove.destinationPage),
        postWait.secondDisplayMove.pixelCountCopied);
    return true;
}

static void SS0DirectApplyFastTransitionCue800271E4(
    PrSS0TransitionDirect::FastTransitionRuntime800201AC& runtime,
    uint32_t iteration,
    uint8_t cueIndex) {
    const uint32_t cueAddress =
        PrSS0TransitionDirect::kCueGlobal9441C +
        6u * static_cast<uint32_t>(cueIndex);
    const bool played =
        PrSfx::PlayScene0MovieTransitionCue800271E4(cueIndex);
    if (iteration == 0u) {
        runtime.initialCue800271E4Dispatched = true;
    }
    Log::Printf(
        "SS0 direct runtime: 800201AC mode=%d iteration=%u 800271E4 cue=%u source=%08X translated=%u before visual/80035560(2)",
        runtime.mode,
        iteration,
        static_cast<unsigned>(cueIndex),
        cueAddress,
        played ? 1u : 0u);
}

static void SS0DirectApplyFastTransitionInitialCue800271E4(
    PrSS0TransitionDirect::FastTransitionRuntime800201AC& runtime) {
    if (!runtime.active || runtime.initialCue800271E4Dispatched ||
        (runtime.mode != 5 && runtime.mode != 6)) {
        return;
    }
    SS0DirectApplyFastTransitionCue800271E4(
        runtime,
        0u,
        static_cast<uint8_t>(runtime.mode == 5 ? 0u : 1u));
}

static bool SS0DirectBeginOpeningMovie0PostWaitTransition801C4E70(
    PrGameContext& ctx,
    const char* owner) {
    if (!SS0DirectExecuteOpeningMovie0OuterPostWait801C4E4C(owner)) {
        return false;
    }
    // The original PlayAndWait cleanup has already stopped its CD stream.
    // SS0 keeps the direct MDEC texture and VRAM page used by the transition.
    (void)ctx;
    s_ss0Direct.openingMovie0FrozenFrameHeld = true;
    Log::Printf(
        "SS0 direct runtime: 801C4E70 freeze-state strStarted=%u textureReady=%u texture=%u textureFrame=%u decoded=%u visualKind=%u visualIteration=%u",
        s_ss0Direct.strStarted ? 1u : 0u,
        s_ss0Direct.strDirectMdecTextureReady ? 1u : 0u,
        s_ss0Direct.strDirectMdecTexture != nullptr ? 1u : 0u,
        s_ss0Direct.strDirectMdecTextureFrameNumber,
        s_ss0Direct.strDirectMdecFramesDecoded,
        static_cast<unsigned>(s_ss0Direct.openingMovie0TransitionVisual.kind),
        s_ss0Direct.openingMovie0TransitionVisual.iteration);
    // 801C4E74 is the 800201AC delay slot and unconditionally republishes
    // the Scene0 first-run latch as one before entering the callee.
    s_ss0Direct.word800916D2Known = true;
    s_ss0Direct.word800916D2 = 1u;
    // 800201AC enters its do/while body immediately.  The first post-MOVIE0
    // frame is therefore the mode-5 80020308 iteration-0/8001F230 frame;
    // do not carry the preceding mode-6 subtitle visual into this boundary.
    s_ss0Direct.openingMovie0TransitionVisual =
        PrSS0TransitionDirect::ResolveFastTransitionVisualFrame800201AC(
            5,
            s_ss0Direct.optionsWord800916DC != 0,
            false,
            0u);
    s_ss0Direct.openingMovie0TransitionPresentBlockedLogged = false;
    if (!PrSS0TransitionDirect::BeginFastTransitionRuntime800201AC(
            s_ss0Direct.openingMovie0Transition,
            PrSS0TransitionDirect::kScene0WorkAddress,
            5,
            1,
            2,
            s_ss0Direct.optionsWord800916DC != 0)) {
        return false;
    }
    // 800201AC immediately enters 8001EA74; 80020308(0) calls
    // 800271E4(0) before it builds the first visual and waits for VBlank.
    // Dispatch on the Begin tick so the seeded iteration-0 page cannot appear
    // one host logic tick before its original cue.
    SS0DirectApplyFastTransitionInitialCue800271E4(
        s_ss0Direct.openingMovie0Transition);
    s_ss0Direct.phase = SS0DirectPhase::OpeningMovie0PostTransition;
    SS0DirectBeginLoadingScreen(
        SS0DirectLoadingHoldKind::OpeningMovie0Post, ctx.frame);
    return true;
}

static void SS0DirectStopTitleStrHostOnly(PrGameContext& ctx) {
    (void)ctx;
    s_ss0Direct.strStarted = false;
    // 80024C84/80024CF8 text state is scoped to one STR playback.  Clear the
    // translated cursor on every stop so a title re-entry cannot briefly
    // present the previous opening-movie line.
    PrStage1VTextDirectResetSub80024C84(
        s_ss0Direct.scene0SubtitleVtextRuntime);
    s_ss0Direct.strVideoFrameCadenceTick = 1u;
    s_ss0Direct.strVideoCadenceLogged = false;
    s_ss0Direct.strTitleAudioTargetCursor = 0u;
    s_ss0Direct.strSubtitleQueryFrame30 = 0u;
    s_ss0Direct.strSubtitleClockFrame30 = 0u;
    s_ss0Direct.strLowerCdStart8001A4D0 = {};
    s_ss0Direct.strLowerCdClock801C4350 = {};
    s_ss0Direct.openingMovie0FrozenFrameHeld = false;
    s_ss0Direct.openingMovie0GraphFlip8001ED74 = {};
    s_ss0Direct.openingMovie0WorkListFlush8001ED3C = {};
    SS0DirectDestroyStrDirectTexture();
}

static bool SS0DirectPrimeTitleMovie0TAudioTail8001A4D0(
    const char* owner) {
    const auto& lowerCdStart = s_ss0Direct.strLowerCdStart8001A4D0;
    if (!lowerCdStart.initialized ||
        lowerCdStart.logicalSectorCount == 0u ||
        !s_ss0Direct.strXaAudioDisc8001A4D0.initialized) {
        return false;
    }

    // The direct adapter normally stays half a second ahead of the CD clock.
    // 801C4894 then calls 80027664 exactly when the visible MOVIE0T picture
    // ends; finish feeding this already-bound original-disc XA track before
    // that CD stop so the tail can drain naturally from the audio voice.
    if (!SS0DirectPumpStrXaAudioOutputAdapter8001A4D0(
            lowerCdStart.logicalSectorCount,
            owner,
            true)) {
        return false;
    }
    const auto& disc = s_ss0Direct.strXaAudioDisc8001A4D0;
    const bool complete =
        disc.rawSectorCursor >= lowerCdStart.logicalSectorCount;
    if (complete) {
        s_ss0Direct.titleMovie0TAudioTailPending =
            s_ss0Direct.strXaAudioVoice >= 0 &&
            s_ss0Direct.strXaAudioOutputAdapterActive;
        s_ss0Direct.titleMovie0TAudioTailReleaseLogged = false;
        Log::Printf(
            "SS0 direct runtime: %s MOVIE0T video-finished XA tail primed rawCursor=%u/%u audioSectors=%u decoded=%u queuedSamples=%llu voice=%d preserveAudio=1 directDiscAuthority=1",
            owner ? owner : "801C4894",
            disc.rawSectorCursor,
            lowerCdStart.logicalSectorCount,
            disc.filteredAudioSectors,
            s_ss0Direct.strXaAudioDecodedSectors,
            static_cast<unsigned long long>(
                s_ss0Direct.strXaAudioQueuedSamples),
            s_ss0Direct.strXaAudioVoice);
    }
    return complete;
}

static bool SS0DirectStopTitleVideoKeepXaAudio(PrGameContext& ctx) {
    // Keep the exact 80027664 translated stop/reset order, but do not tear
    // down the XA output voice that was fully primed above.  The normal
    // StopTitleStr path remains the hard stop for input skips, errors, and
    // scene transitions.
    if (!SS0DirectExecuteStrStopReset80027664(
            "801C4894 title video-finished", true)) {
        return false;
    }
    (void)ctx;
    s_ss0Direct.strStarted = false;
    PrStage1VTextDirectResetSub80024C84(
        s_ss0Direct.scene0SubtitleVtextRuntime);
    s_ss0Direct.strVideoFrameCadenceTick = 1u;
    s_ss0Direct.strVideoCadenceLogged = false;
    s_ss0Direct.strSubtitleQueryFrame30 = 0u;
    s_ss0Direct.strSubtitleClockFrame30 = 0u;
    s_ss0Direct.strLowerCdClock801C4350 = {};
    s_ss0Direct.openingMovie0FrozenFrameHeld = false;
    s_ss0Direct.openingMovie0GraphFlip8001ED74 = {};
    s_ss0Direct.openingMovie0WorkListFlush8001ED3C = {};
    SS0DirectDestroyStrDirectTexture();
    return true;
}

static void SS0DirectReleaseTitleMovie0TAudioTailIfDrained() {
    if (!s_ss0Direct.titleMovie0TAudioTailPending) {
        return;
    }
    const auto& lowerCdStart = s_ss0Direct.strLowerCdStart8001A4D0;
    const auto& disc = s_ss0Direct.strXaAudioDisc8001A4D0;
    if (!lowerCdStart.initialized ||
        disc.rawSectorCursor < lowerCdStart.logicalSectorCount) {
        return;
    }
    const int voice = s_ss0Direct.strXaAudioVoice;
    if (voice >= 0 && AudioEngine::Get().IsVoiceLeaseActive(
                          voice,
                          s_ss0Direct.strXaAudioVoiceGeneration) &&
        AudioEngine::Get().GetVoiceQueuedSamples(voice) != 0u) {
        return;
    }
    if (!s_ss0Direct.titleMovie0TAudioTailReleaseLogged) {
        s_ss0Direct.titleMovie0TAudioTailReleaseLogged = true;
        Log::Printf(
            "SS0 direct runtime: MOVIE0T XA audio tail drained voice=%d rawCursor=%u/%u; releasing output adapter after video-finished tail",
            voice,
            disc.rawSectorCursor,
            lowerCdStart.logicalSectorCount);
    }
    SS0DirectStopStrXaAudioOutputAdapter8001A4D0();
    s_ss0Direct.titleMovie0TAudioTailPending = false;
}

static void SS0DirectStopTitleStr(PrGameContext& ctx) {
    (void)SS0DirectExecuteStrStopReset80027664("Scene0 stop");
    // A naturally-finished MOVIE0 keeps its XA tail alive briefly so the
    // audio can drain after the STR runtime becomes inactive.  An explicit
    // title skip/re-entry is a hard stop, however: release that preserved
    // voice here instead of only clearing the bookkeeping flag.
    SS0DirectStopStrXaAudioOutputAdapter8001A4D0();
    SS0DirectStopLegacyHostStrPlayer(ctx, "Scene0 stop");
    SS0DirectStopTitleStrHostOnly(ctx);
    s_ss0Direct.titleMovie0TAudioTailPending = false;
    s_ss0Direct.titleMovie0TAudioTailReleaseLogged = false;
}

static bool SS0DirectStartScene0Str(PrGameContext& ctx,
                                    const char* owner,
                                    uint32_t sceneEntryRowIndex,
                                        PrSS0StrLifecycleDirect::StrMovieKind
                                        movieKind) {
    // The direct start is the 801C44E0/801C4894 ownership boundary.  Ensure
    // no legacy StrPlayer (and its independent audio voice) survives into
    // the original-disc MDEC/XA path.
    SS0DirectStopLegacyHostStrPlayer(ctx, owner);
    SS0DirectStopStrXaAudioOutputAdapter8001A4D0();
    s_ss0Direct.titleMovie0TAudioTailPending = false;
    s_ss0Direct.titleMovie0TAudioTailReleaseLogged = false;
    s_ss0Direct.strVideoFrameCadenceTick = 1u;
    s_ss0Direct.strVideoCadenceLogged = false;
    s_ss0Direct.strTitleAudioTargetCursor = 0u;
    s_ss0Direct.strSubtitleQueryFrame30 = 0u;
    s_ss0Direct.strSubtitleClockFrame30 = 0u;
    s_ss0Direct.strLowerCdClock801C4350 = {};
    s_ss0Direct.strClockPoll801C4350 = {};
    s_ss0Direct.strLowerCdStart8001A4D0 = {};
    s_ss0Direct.strVlcDecodeRelease800273A4 = {};
    s_ss0Direct.strMdecCommandOutput800273A4 = {};
    s_ss0Direct.strMdecOutputCurrentScus = {};
    s_ss0Direct.strMdecCompletion80027220 = {};
    s_ss0Direct.strMdecCompletionPending = false;
    s_ss0Direct.strDirectMdecFramesDecoded = 0u;
    s_ss0Direct.strDirectMdecLastFrameNumber = 0u;
    s_ss0Direct.strDirectMdecStreamExhausted = false;
    s_ss0Direct.strVramPageRuntime8001B120 =
        PrSS0StrLifecycleDirect::InitializeStrVramPageRuntime8001B120();
    // 8001C470 calls 8001B1B0(0,0,0) before 8001C4DC4 starts MOVIE0.
    // Re-establish that semantic double-buffer clear after resetting the
    // per-STR page carrier; otherwise the first 8001B120(1) cleanup sees the
    // opposite display page as unknown and silently drops the PSX copy.
    const auto vramClear8001B1B0 =
        PrSS0StrLifecycleDirect::ClearStrVramPages8001B1B0(
            s_ss0Direct.strVramPageRuntime8001B120, 0, 0, 0);
    if (!vramClear8001B1B0.known ||
        !vramClear8001B1B0.executionAccepted ||
        !vramClear8001B1B0.completeDoubleBuffer ||
        !vramClear8001B1B0.directVramPixelStateAuthority ||
        vramClear8001B1B0.psxGpuMmioAuthority ||
        vramClear8001B1B0.hardwareCompletionTimingAuthority ||
        vramClear8001B1B0.hostProjection ||
        vramClear8001B1B0.replayValueAuthority ||
        vramClear8001B1B0.oldWinS0Authority ||
        vramClear8001B1B0.stage2PlusAuthority ||
        vramClear8001B1B0.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 8001B1B0 black double-buffer clear blocked",
            owner ? owner : "Scene0");
        return false;
    }
    Log::Printf(
        "SS0 direct runtime: %s 8001B1B0 black double-buffer clear committed width=%u height=%u pages=%u psxGpuMmioAuthority=0 hardwareCompletionTimingAuthority=0",
        owner ? owner : "Scene0",
        static_cast<unsigned>(vramClear8001B1B0.width),
        static_cast<unsigned>(vramClear8001B1B0.height),
        static_cast<unsigned>(
            s_ss0Direct.strVramPageRuntime8001B120.pages.size()));
    s_ss0Direct.strVramPagePublish8002756C = {};
    s_ss0Direct.strDisplayMove8001B120 = {};
    s_ss0Direct.strOuterPostWait801C4DC4 = {};
    s_ss0Direct.openingMovie0GraphFlip8001ED74 = {};
    s_ss0Direct.openingMovie0WorkListFlush8001ED3C = {};
    SS0DirectDestroyStrDirectTexture();
    const auto row =
        PrSceneEntryDirect::GetSceneEntryStaticRawRow(0u,
                                                      sceneEntryRowIndex);
    PrSS0StrLifecycleDirect::Scene0StrAudioRouteSource801C44E0 source{};
    source.rowKnown = row.known;
    source.opaque04Known = row.opaque04Known;
    source.opaque04 = row.opaque04;
    const auto audioRoute =
        PrSS0StrLifecycleDirect::BuildScene0StrAudioRoute801C44E0(source);
    if (!audioRoute.known || !audioRoute.currentScusSemanticAuthority ||
        !audioRoute.currentComod0CallerAuthority ||
        !audioRoute.volumeKnown || !audioRoute.directVolumeCommandAuthority ||
        audioRoute.psxSpuHardwareAuthority || audioRoute.hostProjection ||
        !audioRoute.filterKnown || !audioRoute.filterCommandSerialKnown ||
        !audioRoute.directFilterCommandAuthority ||
        audioRoute.replayValueAuthority || audioRoute.oldWinS0Authority ||
        audioRoute.stage2PlusAuthority || audioRoute.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s STR audio route blocked row=%u rowKnown=%d opaque04Known=%d",
            owner ? owner : "Scene0",
            sceneEntryRowIndex,
            row.known ? 1 : 0,
            row.opaque04Known ? 1 : 0);
        return false;
    }

    PrSS0StrLifecycleDirect::StrDecoderAllocationState80027288
        decoderState{};
    const int32_t movieKindValue = static_cast<int32_t>(movieKind);
    if (!PrSS0StrLifecycleDirect::TryInitializeStrDecoderState80027288(
            movieKindValue,
            s_ss0Direct.optionsWord800916DC != 0,
            decoderState) ||
        !decoderState.accepted ||
        !decoderState.currentScusSemanticAuthority ||
        !decoderState.currentComod0CallerAuthority ||
        !decoderState.comod1ComparisonOnly ||
        decoderState.hostProjection ||
        decoderState.psxPointerAuthority ||
        decoderState.replayValueAuthority ||
        decoderState.oldWinS0Authority ||
        decoderState.stage2PlusAuthority ||
        decoderState.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 80027288 decoder state blocked kind=%d",
            owner ? owner : "Scene0",
            movieKindValue);
        s_ss0Direct.strStarted = false;
        return false;
    }
    s_ss0Direct.strDecoderAllocation80027288 = decoderState;

    PrSS0StrLifecycleDirect::StrStreamStartState800274D4
        streamStartState{};
    if (!PrSS0StrLifecycleDirect::TryStartStrStreamState800274D4(
            decoderState,
            streamStartState) ||
        !streamStartState.accepted ||
        !streamStartState.currentScusSemanticAuthority ||
        !streamStartState.currentComod0CallerAuthority ||
        !streamStartState.comod1ComparisonOnly ||
        streamStartState.lowerCdStartAuthority ||
        streamStartState.initialDecodeAdvanceSemanticTranslated ||
        streamStartState.streamLibraryAuthority ||
        streamStartState.psxPointerAuthority ||
        streamStartState.hostProjection ||
        streamStartState.replayValueAuthority ||
        streamStartState.oldWinS0Authority ||
        streamStartState.stage2PlusAuthority ||
        streamStartState.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 800274D4 stream-start state blocked",
            owner ? owner : "Scene0");
        s_ss0Direct.strStarted = false;
        return false;
    }
    s_ss0Direct.strStreamStart800274D4 = streamStartState;

    const PrSS0StrLifecycleDirect::StrDecodeAdvanceInput800273A4
        decodeAdvanceInput{};
    PrSS0StrLifecycleDirect::StrDecodeAdvanceState800273A4
        decodeAdvanceState =
            PrSS0StrLifecycleDirect::BuildStrDecodeAdvanceState800273A4(
                streamStartState,
                decodeAdvanceInput);
    if (!decodeAdvanceState.planned ||
        decodeAdvanceState.executed ||
        !decodeAdvanceState.hasOpenInputGap ||
        !decodeAdvanceState.branchSemanticsTranslated ||
        decodeAdvanceState.runtimeInputBound ||
        decodeAdvanceState.sectorRingExecutionAuthority ||
        decodeAdvanceState.mdecExecutionAuthority ||
        decodeAdvanceState.psxPointerAuthority ||
        decodeAdvanceState.hostProjection ||
        decodeAdvanceState.replayValueAuthority ||
        decodeAdvanceState.oldWinS0Authority ||
        decodeAdvanceState.stage2PlusAuthority ||
        decodeAdvanceState.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 800273A4 decode-advance plan blocked",
            owner ? owner : "Scene0");
        s_ss0Direct.strStarted = false;
        return false;
    }
    s_ss0Direct.strDecodeAdvance800273A4 = decodeAdvanceState;

    PrSS0StrLifecycleDirect::StrDecodeGateState80027528
        decodeGateState =
            PrSS0StrLifecycleDirect::BuildStrDecodeGateState80027528(
                decodeAdvanceState.control);
    if (!decodeGateState.known ||
        !decodeGateState.branchSemanticsTranslated ||
        !decodeGateState.callDecodeAdvance800273A4 ||
        decodeGateState.decodeAdvanceExecuted ||
        decodeGateState.hostProjection ||
        decodeGateState.replayValueAuthority ||
        decodeGateState.oldWinS0Authority ||
        decodeGateState.stage2PlusAuthority ||
        decodeGateState.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 80027528 decode-gate plan blocked",
            owner ? owner : "Scene0");
        s_ss0Direct.strStarted = false;
        return false;
    }
    s_ss0Direct.strDecodeGate80027528 = decodeGateState;

    PrSS0StrLifecycleDirect::StrStripUploadState8002756C
        stripUploadState =
            PrSS0StrLifecycleDirect::BuildStrStripUploadState8002756C(
                decodeAdvanceState.control,
                decodeAdvanceState.sectorCounterAfter,
                false,
                0);
    if (!stripUploadState.known ||
        !stripUploadState.skippedForCounterWarmup ||
        stripUploadState.uploadCallsPlanned80044D64 ||
        stripUploadState.gpuUploadExecuted ||
        stripUploadState.psxImagePointerAuthority ||
        stripUploadState.hostProjection ||
        stripUploadState.replayValueAuthority ||
        stripUploadState.oldWinS0Authority ||
        stripUploadState.stage2PlusAuthority ||
        stripUploadState.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 8002756C strip-upload plan blocked",
            owner ? owner : "Scene0");
        s_ss0Direct.strStarted = false;
        return false;
    }
    s_ss0Direct.strStripUpload8002756C = stripUploadState;

    const auto& ingress = s_ss0Direct.scene0ResourceIngress801C4780;
    if (!PrSS0Scene0ResourceIngressDirect::
            IsExactAcceptedTransaction801C4780(ingress) ||
        !ingress.discImageDirectoryAuthority ||
        sceneEntryRowIndex >=
            PrMovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780) {
        Log::Printf(
            "SS0 direct runtime: %s 8001A4D0 lower-CD source blocked row=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex);
        s_ss0Direct.strStarted = false;
        return false;
    }
    const auto& segmentRow =
        ingress.scan801C4780.table.rows[sceneEntryRowIndex];
    const auto segmentFields =
        PrMovieSegmentDirect::BuildMovieStepSegmentFields801C4350(
            segmentRow);

    if (PrSS0StrLifecycleDirect::UsesStrLowerCdClockRuntime801C4350(
            movieKind)) {
        PrSS0StrLifecycleDirect::StrLowerCdClockSource801C4350
            lowerCdSource{};
        lowerCdSource.segmentTimeBaseKnown =
            segmentFields.segmentTimeBaseA1Plus40Known;
        lowerCdSource.segmentTimeBase =
            segmentFields.segmentTimeBaseA1Plus40;
        lowerCdSource.segmentEndKnown =
            segmentFields.segmentEndA1Plus44Known;
        lowerCdSource.segmentEnd =
            segmentFields.segmentEndA1Plus44;
        lowerCdSource.segmentEndBiasKnown =
            segmentFields.segmentEndBiasA1Plus8Known;
        lowerCdSource.segmentEndBias =
            segmentFields.segmentEndBiasA1Plus8;
        lowerCdSource.discImageDirectoryAuthority = true;
        if (!PrSS0StrLifecycleDirect::
                TryInitializeStrLowerCdClockRuntime801C4350(
                    lowerCdSource,
                    s_ss0Direct.strLowerCdClock801C4350)) {
            Log::Printf(
                "SS0 direct runtime: %s 801C4350 lower-CD init blocked row=%u baseKnown=%d endKnown=%d biasKnown=%d",
                owner ? owner : "Scene0",
                sceneEntryRowIndex,
                lowerCdSource.segmentTimeBaseKnown ? 1 : 0,
                lowerCdSource.segmentEndKnown ? 1 : 0,
                lowerCdSource.segmentEndBiasKnown ? 1 : 0);
            s_ss0Direct.strStarted = false;
            return false;
        }
        Log::Printf(
            "SS0 direct runtime: %s 801C4350 lower-CD bound row=%u base=%d end=%d bias=%d effectiveEnd=%d query=%08X threshold=%u scale=%u fps=%u fpm=%u deadlineLead=%u discImageDirectoryAuthority=1 runtimeInputBound=1 psxCdClockAuthority=1 hostClockAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
            owner ? owner : "Scene0",
            sceneEntryRowIndex,
            s_ss0Direct.strLowerCdClock801C4350.timeBaseLba,
            s_ss0Direct.strLowerCdClock801C4350.endLba,
            s_ss0Direct.strLowerCdClock801C4350.endBias,
            s_ss0Direct.strLowerCdClock801C4350.effectiveEndLba,
            PrSS0StrLifecycleDirect::kFn8001A7A4,
            PrSS0StrLifecycleDirect::kStrClockDiscontinuityThreshold,
            PrSS0StrLifecycleDirect::kStrClockSectorScale,
            PrSS0StrLifecycleDirect::kStrClockFramesPerSecond,
            PrSS0StrLifecycleDirect::kStrClockFramesPerMinute,
            PrSS0StrLifecycleDirect::kStrClockDeadlineLead);
    } else {
        Log::Printf(
            "SS0 direct runtime: %s lower-CD clock unbound by active owner kind=%d opening801C455C=0 title801C4894OwnTick=1 hostClockAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
            owner ? owner : "Scene0",
            movieKindValue);
    }

    PrSS0StrLifecycleDirect::StrLowerCdStartSource8001A4D0
        lowerCdStartSource{};
    lowerCdStartSource.segmentKnown = segmentRow.known;
    lowerCdStartSource.startLbaKnown =
        segmentFields.segmentTimeBaseA1Plus40Known;
    lowerCdStartSource.startLba =
        segmentFields.segmentTimeBaseA1Plus40;
    lowerCdStartSource.lengthBytesKnown =
        segmentRow.lengthSourceA1Plus20Known;
    lowerCdStartSource.lengthBytes =
        segmentRow.lengthSourceA1Plus20;
    lowerCdStartSource.filterKnown = audioRoute.filterKnown;
    lowerCdStartSource.filterFile = audioRoute.filterFile8004940C;
    lowerCdStartSource.filterChannel =
        audioRoute.filterChannel8004940D;
    lowerCdStartSource.discBinPath = ingress.discBinPath;
    lowerCdStartSource.discImageDirectoryAuthority =
        ingress.discImageDirectoryAuthority;
    PrSS0StrLifecycleDirect::StrLowerCdStartRuntime8001A4D0
        lowerCdStart{};
    if (!PrSS0StrLifecycleDirect::
            TryInitializeStrLowerCdStartRuntime8001A4D0(
                lowerCdStartSource,
                lowerCdStart) ||
        !lowerCdStart.initialized ||
        !lowerCdStart.currentScusSemanticAuthority ||
        !lowerCdStart.currentComod0CallerAuthority ||
        !lowerCdStart.discImageDirectoryAuthority ||
        !lowerCdStart.discImagePayloadAuthority ||
        !lowerCdStart.directPsxStatusAuthority ||
        lowerCdStart.hardwareCallbackAuthority ||
        lowerCdStart.hostProjection ||
        lowerCdStart.hostExtractedFileAuthority ||
        lowerCdStart.replayValueAuthority ||
        lowerCdStart.oldWinS0Authority ||
        lowerCdStart.stage2PlusAuthority ||
        lowerCdStart.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 8001A4D0 lower-CD start blocked row=%u lbaKnown=%d lengthKnown=%d filterKnown=%d",
            owner ? owner : "Scene0",
            sceneEntryRowIndex,
            lowerCdStartSource.startLbaKnown ? 1 : 0,
            lowerCdStartSource.lengthBytesKnown ? 1 : 0,
            lowerCdStartSource.filterKnown ? 1 : 0);
        s_ss0Direct.strStarted = false;
        return false;
    }
    s_ss0Direct.strLowerCdStart8001A4D0 = lowerCdStart;
    if (!PrSS0StrLifecycleDirect::
            TryInitializeStrXaAudioDiscRuntime8001A4D0(
                lowerCdStart,
                s_ss0Direct.strXaAudioDisc8001A4D0)) {
        Log::Printf(
            "SS0 direct runtime: %s original-disc XA selector init blocked row=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex);
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    s_ss0Direct.strXaAudioDecoder.Reset();
    s_ss0Direct.strXaAudioOutputVolume = audioRoute.normalizedVolume;
    s_ss0Direct.strXaAudioOutputAdapterKnown = true;
    if (!PrSS0StrLifecycleDirect::
            TryInitializeStrDecoderMemoryRuntime80027288(
                decoderState,
                s_ss0Direct.strDecoderMemoryRuntime80027288)) {
        Log::Printf(
            "SS0 direct runtime: %s 80027288 decoder memory execution blocked kind=%d",
            owner ? owner : "Scene0",
            movieKindValue);
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    s_ss0Direct.strTitleAudioTargetCursor =
        s_ss0Direct.strDecoderMemoryRuntime80027288.
            sectorRingRawSectorCursor;
    const auto rootCallbackReset800358DC =
        PrSS0StrLifecycleDirect::ExecuteStrRootCallbackReset800358DC(
            s_ss0Direct.strRootCallback80055F78);
    s_ss0Direct.strRootCallbackReset800358DC =
        rootCallbackReset800358DC;
    if (!rootCallbackReset800358DC.known ||
        !rootCallbackReset800358DC.executionAccepted ||
        !rootCallbackReset800358DC.currentScusSemanticAuthority ||
        !rootCallbackReset800358DC.directSoftwareStateAuthority ||
        rootCallbackReset800358DC.psxHardwareMmioAuthority ||
        rootCallbackReset800358DC.hardwareCallbackTimingAuthority ||
        rootCallbackReset800358DC.hostProjection ||
        rootCallbackReset800358DC.replayValueAuthority ||
        rootCallbackReset800358DC.oldWinS0Authority ||
        rootCallbackReset800358DC.stage2PlusAuthority ||
        rootCallbackReset800358DC.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 800473EC(0)->ResetCallback->800358DC root software reset blocked enabledBefore=%u; rollback before stream start",
            owner ? owner : "Scene0",
            rootCallbackReset800358DC.enabledBefore ? 1u : 0u);
        (void)SS0DirectExecuteStrStopReset80027664(
            "800358DC reset rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    Log::Printf(
        "SS0 direct runtime: %s 800473EC(0)->ResetCallback->800358DC root software reset accepted enabledBefore=%u noOp=%u return=%08X directSoftwareStateAuthority=1 psxHardwareMmioAuthority=0 hardwareCallbackTimingAuthority=0 hostProjection=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "Scene0",
        rootCallbackReset800358DC.enabledBefore ? 1u : 0u,
        rootCallbackReset800358DC.noOpAlreadyEnabled ? 1u : 0u,
        rootCallbackReset800358DC.returnValue);
    if (!PrSS0StrLifecycleDirect::
            PublishStrDecoderStreamingStart8001A4D0(
                lowerCdStart,
                s_ss0Direct.strDecoderMemoryRuntime80027288)) {
        Log::Printf(
            "SS0 direct runtime: %s 8001A4D0 decoder streaming state blocked row=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex);
        (void)SS0DirectExecuteStrStopReset80027664(
            "8001A4D0 start rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    if (!PrSS0StrLifecycleDirect::
            PublishStrLowerCdStartCommand8001A4D0(
                lowerCdStart,
                s_ss0Direct.strWorkBase80049428)) {
        Log::Printf(
            "SS0 direct runtime: %s 8001A4D0 lower-CD command latch blocked row=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex);
        (void)SS0DirectExecuteStrStopReset80027664(
            "8001A4D0 command rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }

    const PrSS0StrLifecycleDirect::StrSectorRingPrimeResult80039670
        sectorRingPrime =
            PrSS0StrLifecycleDirect::PrimeStrSectorRingFromDisc80039670(
                lowerCdStart,
                s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!sectorRingPrime.known ||
        !sectorRingPrime.framePublished ||
        !sectorRingPrime.directDiscSectorAuthority ||
        !sectorRingPrime.synchronousCallbackOrdering ||
        sectorRingPrime.hardwareCallbackTimingAuthority ||
        sectorRingPrime.psxPointerAuthority ||
        sectorRingPrime.hostProjection ||
        sectorRingPrime.replayValueAuthority ||
        sectorRingPrime.oldWinS0Authority ||
        sectorRingPrime.stage2PlusAuthority ||
        sectorRingPrime.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 80039670 sector-ring prime blocked row=%u scanned=%u filtered=%u rejected=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex,
            sectorRingPrime.rawSectorsScanned,
            sectorRingPrime.filteredVideoSectors,
            sectorRingPrime.rejectedSectors);
        (void)SS0DirectExecuteStrStopReset80027664(
            "80039670 sector-ring rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }

    const PrSS0StrLifecycleDirect::StrSectorRingAcquireResult8003958C
        sectorRingAcquire =
            PrSS0StrLifecycleDirect::AcquireStrSectorRingFrame8003958C(
                s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!sectorRingAcquire.known ||
        !sectorRingAcquire.available ||
        sectorRingAcquire.returnValue != 0 ||
        !sectorRingAcquire.payloadHandleKnown ||
        sectorRingAcquire.payloadHandle == 0u ||
        !sectorRingAcquire.sectorHeaderKnown ||
        !sectorRingAcquire.stateTransition2To4 ||
        !sectorRingAcquire.directSectorRingAuthority ||
        sectorRingAcquire.psxPointerAuthority ||
        sectorRingAcquire.hostProjection ||
        sectorRingAcquire.replayValueAuthority ||
        sectorRingAcquire.oldWinS0Authority ||
        sectorRingAcquire.stage2PlusAuthority ||
        sectorRingAcquire.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 8003958C sector-ring acquire blocked row=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex);
        (void)SS0DirectExecuteStrStopReset80027664(
            "8003958C sector-ring rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }

    PrSS0StrLifecycleDirect::StrDecodeAdvanceInput800273A4
        directDecodeAdvanceInput{};
    directDecodeAdvanceInput.acquireReturnKnown = true;
    directDecodeAdvanceInput.acquireReturn =
        sectorRingAcquire.returnValue;
    directDecodeAdvanceInput.payloadHandleKnown =
        sectorRingAcquire.payloadHandleKnown;
    directDecodeAdvanceInput.payloadHandle =
        sectorRingAcquire.payloadHandle;
    directDecodeAdvanceInput.sectorHeaderKnown =
        sectorRingAcquire.sectorHeaderKnown;
    directDecodeAdvanceInput.sectorWidth = sectorRingAcquire.width;
    directDecodeAdvanceInput.sectorHeight = sectorRingAcquire.height;
    directDecodeAdvanceInput.sectorCounterBefore =
        streamStartState.sectorCounterValue;
    directDecodeAdvanceInput.sectorRingExecutionAuthority = true;
    const PrSS0StrLifecycleDirect::StrVlcDecodeReleaseResult800273A4
        vlcDecodeRelease =
            PrSS0StrLifecycleDirect::
                ExecuteStrVlcDecodeAndRelease800273A4(
                    sectorRingAcquire.payloadHandle,
                    s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!vlcDecodeRelease.known ||
        !vlcDecodeRelease.executed ||
        !vlcDecodeRelease.vlc.known ||
        !vlcDecodeRelease.vlc.executed ||
        vlcDecodeRelease.vlc.returnValue != 0 ||
        !vlcDecodeRelease.vlc.frameComplete ||
        !vlcDecodeRelease.vlc.trailingFe00PaddingWritten ||
        vlcDecodeRelease.vlc.trailingFe00Halfwords != 65u ||
        !vlcDecodeRelease.vlc.exactCurrentIdaTableAuthority ||
        !vlcDecodeRelease.vlc.currentScusSemanticAuthority ||
        !vlcDecodeRelease.releaseFrameCalled80039490 ||
        !vlcDecodeRelease.release.known ||
        !vlcDecodeRelease.release.released ||
        vlcDecodeRelease.release.returnValue != 0 ||
        !vlcDecodeRelease.release.stateTransition4To0 ||
        !vlcDecodeRelease.exactCurrentIdaCallOrder ||
        !vlcDecodeRelease.currentScusSemanticAuthority ||
        !vlcDecodeRelease.directSectorRingAuthority ||
        vlcDecodeRelease.mdecHardwareExecutionAuthority ||
        vlcDecodeRelease.psxPointerAuthority ||
        vlcDecodeRelease.hostProjection ||
        vlcDecodeRelease.replayValueAuthority ||
        vlcDecodeRelease.oldWinS0Authority ||
        vlcDecodeRelease.stage2PlusAuthority ||
        vlcDecodeRelease.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s DecDCTvlc2/release blocked row=%u input=%u output=%u return=%d",
            owner ? owner : "Scene0",
            sceneEntryRowIndex,
            vlcDecodeRelease.inputBytes,
            vlcDecodeRelease.vlc.outputBytesWritten,
            vlcDecodeRelease.vlc.returnValue);
        (void)SS0DirectExecuteStrStopReset80027664(
            "DecDCTvlc2/release rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    s_ss0Direct.strVlcDecodeRelease800273A4 = vlcDecodeRelease;
    PrSS0StrLifecycleDirect::StrDecoderControlState80027288
        mdecControl = streamStartState.control;
    mdecControl.decodedWidth = sectorRingAcquire.width;
    mdecControl.decodedHeight = sectorRingAcquire.height;
    mdecControl.vlcDecodeResult =
        static_cast<uint32_t>(vlcDecodeRelease.vlc.returnValue);
    mdecControl.decodeInFlight = 1u;
    const PrSS0StrLifecycleDirect::StrMdecCommandOutputSubmission800273A4
        mdecCommandOutput =
            PrSS0StrLifecycleDirect::
                ExecuteStrMdecCommandOutputSubmission800273A4(
                    mdecControl,
                    vlcDecodeRelease,
                    s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!mdecCommandOutput.known || !mdecCommandOutput.executed ||
        mdecCommandOutput.controlArgument80047558 != 2u ||
        mdecCommandOutput.inputDmaWordCount80047778 !=
            vlcDecodeRelease.vlc.frameCodeCount ||
        !mdecCommandOutput.inputSyncCalled8004789C ||
        mdecCommandOutput.dmaPriorityOrMask != 0x88u ||
        mdecCommandOutput.inputMadrByteOffset != 4u ||
        mdecCommandOutput.inputChcr != 0x01000201u ||
        !mdecCommandOutput.mdecCommandPortWritten ||
        !mdecCommandOutput.outputSyncCalled80047934 ||
        mdecCommandOutput.outputChcr != 0x01000200u ||
        !mdecCommandOutput.outputReadyStored800273A4 ||
        !mdecCommandOutput.dmaCompletionCallbackPending80027220 ||
        !mdecCommandOutput.outputBufferUntouched ||
        !mdecCommandOutput.exactCurrentIdaCallOrder ||
        !mdecCommandOutput.currentScusSemanticAuthority ||
        !mdecCommandOutput.directCommandStateAuthority ||
        mdecCommandOutput.mdecHardwareExecutionAuthority ||
        mdecCommandOutput.hardwareCallbackTimingAuthority ||
        mdecCommandOutput.psxPointerAuthority ||
        mdecCommandOutput.psxHardwareMmioAuthority ||
        mdecCommandOutput.hostProjection ||
        mdecCommandOutput.replayValueAuthority ||
        mdecCommandOutput.oldWinS0Authority ||
        mdecCommandOutput.stage2PlusAuthority ||
        mdecCommandOutput.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s MDEC command/output submission blocked row=%u command=%08X inputBcr=%08X outputBcr=%08X",
            owner ? owner : "Scene0",
            sceneEntryRowIndex,
            mdecCommandOutput.vlcCommandWordAfter,
            mdecCommandOutput.inputBcr,
            mdecCommandOutput.outputBcr);
        (void)SS0DirectExecuteStrStopReset80027664(
            "MDEC command/output submission rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    s_ss0Direct.strMdecCommandOutput800273A4 = mdecCommandOutput;
    const PrSS0MdecOutputDirect::Mdec15bppResult mdecOutput =
        PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                allocations[2].data(),
            vlcDecodeRelease.vlc.outputBytesWritten,
            sectorRingAcquire.width,
            sectorRingAcquire.height,
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                allocations[3].data(),
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                allocations[3].size());
    if (!mdecOutput.known || !mdecOutput.executed ||
        mdecOutput.commandWord != mdecCommandOutput.vlcCommandWordAfter ||
        mdecOutput.inputWordCount !=
            mdecCommandOutput.inputDmaWordCount80047778 ||
        mdecOutput.width != sectorRingAcquire.width ||
        mdecOutput.height != sectorRingAcquire.height ||
        mdecOutput.outputBytesWritten !=
            mdecCommandOutput.outputDmaWordCount8004780C *
                sizeof(uint32_t) ||
        !mdecOutput.output15bpp || !mdecOutput.outputUnsigned ||
        !mdecOutput.outputBit15Set ||
        !mdecOutput.dma1VerticalMacroblockLayout ||
        !mdecOutput.currentScusQuantTableAuthority ||
        !mdecOutput.currentScusScaleTableAuthority ||
        !mdecOutput.currentIdaCommandAuthority ||
        !mdecOutput.documentedPsxMdecAlgorithmAuthority ||
        mdecOutput.mdecHardwareBitExactAuthority ||
        mdecOutput.dmaCompletionCallbackTimingAuthority ||
        mdecOutput.psxHardwareMmioAuthority ||
        mdecOutput.hostProjection || mdecOutput.replayValueAuthority ||
        mdecOutput.oldWinS0Authority || mdecOutput.stage2PlusAuthority ||
        mdecOutput.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s current-SCUS MDEC 15bpp output blocked row=%u command=%08X macroblocks=%u output=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex,
            mdecOutput.commandWord,
            mdecOutput.macroblocksDecoded,
            mdecOutput.outputBytesWritten);
        (void)SS0DirectExecuteStrStopReset80027664(
            "current-SCUS MDEC 15bpp output rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    s_ss0Direct.strMdecOutputCurrentScus = mdecOutput;
    Log::Printf(
        "SS0 direct runtime: %s current-SCUS MDEC 15bpp output row=%u macroblocks=%u halfwords=%u bytes=%u fnv1a=%016llX currentIdaTables=1 documentedAlgorithm=1 dma1VerticalLayout=1 hardwareBitExactAuthority=0 callbackPending=1 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "Scene0",
        sceneEntryRowIndex,
        mdecOutput.macroblocksDecoded,
        mdecOutput.inputHalfwordsConsumed,
        mdecOutput.outputBytesWritten,
        static_cast<unsigned long long>(mdecOutput.outputFnv1a64));
    directDecodeAdvanceInput.vlcDecodeResultKnown = true;
    directDecodeAdvanceInput.vlcDecodeResult =
        vlcDecodeRelease.vlc.returnValue;
    directDecodeAdvanceInput.mdecCommandOutputSubmissionKnown = true;
    directDecodeAdvanceInput.mdecCommandOutputSubmissionAuthority = true;
    decodeAdvanceState =
        PrSS0StrLifecycleDirect::BuildStrDecodeAdvanceState800273A4(
            streamStartState,
            directDecodeAdvanceInput);
    if (!decodeAdvanceState.planned ||
        !decodeAdvanceState.executed ||
        decodeAdvanceState.hasOpenInputGap ||
        decodeAdvanceState.branch !=
            PrSS0StrLifecycleDirect::
                StrDecodeAdvanceBranch800273A4::
                    MdecOutputSubmittedHardwareBoundary ||
        !decodeAdvanceState.branchSemanticsTranslated ||
        !decodeAdvanceState.runtimeInputBound ||
        !decodeAdvanceState.sectorRingExecutionAuthority ||
        !decodeAdvanceState.executedThroughAcquire8003958C ||
        !decodeAdvanceState.reachedDecDctVlc2Boundary ||
        decodeAdvanceState.blockedAtMdecDecDCTvlc2 ||
        !decodeAdvanceState.decDctVlc2Called ||
        !decodeAdvanceState.releaseFrameCalled80039490 ||
        !decodeAdvanceState.reachedMdecControlBoundary80047558 ||
        !decodeAdvanceState.blockedAtMdecHardwareExecution ||
        !decodeAdvanceState.blockedAtMdecHardwareOutput ||
        !decodeAdvanceState.hasOpenMdecHardwareGap ||
        !decodeAdvanceState.mdecControlCalled80047558 ||
        !decodeAdvanceState.outputTransferCalled ||
        !decodeAdvanceState.mdecCommandOutputSubmissionAuthority ||
        decodeAdvanceState.mdecExecutionAuthority ||
        decodeAdvanceState.psxPointerAuthority ||
        decodeAdvanceState.hostProjection ||
        decodeAdvanceState.replayValueAuthority ||
        decodeAdvanceState.oldWinS0Authority ||
        decodeAdvanceState.stage2PlusAuthority ||
        decodeAdvanceState.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 800273A4 direct sector input blocked row=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex);
        (void)SS0DirectExecuteStrStopReset80027664(
            "800273A4 sector-input rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    s_ss0Direct.strDecodeAdvance800273A4 = decodeAdvanceState;
    s_ss0Direct.strMdecCompletionPending = true;
    s_ss0Direct.strDirectMdecFramesDecoded = 1u;
    s_ss0Direct.strDirectMdecLastFrameNumber =
        sectorRingAcquire.frameNumber;
    s_ss0Direct.strDirectMdecStreamExhausted = false;
    decodeGateState =
        PrSS0StrLifecycleDirect::BuildStrDecodeGateState80027528(
            decodeAdvanceState.control);
    s_ss0Direct.strDecodeGate80027528 = decodeGateState;
    stripUploadState =
        PrSS0StrLifecycleDirect::BuildStrStripUploadState8002756C(
            decodeAdvanceState.control,
            decodeAdvanceState.sectorCounterAfter,
            false,
            0);
    if (!decodeGateState.known ||
        !decodeGateState.branchSemanticsTranslated ||
        decodeGateState.branch !=
            PrSS0StrLifecycleDirect::
                StrDecodeGateBranch80027528::DecodeBusy ||
        decodeGateState.callDecodeAdvance800273A4 ||
        decodeGateState.decodeAdvanceExecuted ||
        decodeGateState.hostProjection ||
        decodeGateState.replayValueAuthority ||
        decodeGateState.oldWinS0Authority ||
        decodeGateState.stage2PlusAuthority ||
        decodeGateState.comod2Authority ||
        !stripUploadState.known ||
        !stripUploadState.skippedForCounterWarmup ||
        stripUploadState.uploadCallsPlanned80044D64 ||
        stripUploadState.gpuUploadExecuted ||
        stripUploadState.psxImagePointerAuthority ||
        stripUploadState.hostProjection ||
        stripUploadState.replayValueAuthority ||
        stripUploadState.oldWinS0Authority ||
        stripUploadState.stage2PlusAuthority ||
        stripUploadState.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 80027528/8002756C post-acquire state blocked row=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex);
        (void)SS0DirectExecuteStrStopReset80027664(
            "80027528 post-acquire rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }
    s_ss0Direct.strStripUpload8002756C = stripUploadState;
    Log::Printf(
        "SS0 direct runtime: %s 8001A4D0 lower-CD start bound row=%u lba=%d bytes=%u sectors=%u filter=%u/%u submode=%02X coding=%02X setlocSync=2 setfilterSync=2 readSSync=2 wait=3 flags=1/-16 prime=80036678(1,0) byte80057119=%02X status8001A750=2/%02X currentScusSemanticAuthority=1 currentComod0CallerAuthority=1 discImagePayloadAuthority=1 directPsxStatusAuthority=1 hardwareCallbackAuthority=0 hostProjection=0 hostExtractedFileAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "Scene0",
        sceneEntryRowIndex,
        lowerCdStart.startLba,
        lowerCdStart.lengthBytes,
        lowerCdStart.logicalSectorCount,
        static_cast<unsigned>(lowerCdStart.filterFile),
        static_cast<unsigned>(lowerCdStart.filterChannel),
        static_cast<unsigned>(lowerCdStart.xaSubmode),
        static_cast<unsigned>(lowerCdStart.xaCoding),
        static_cast<unsigned>(
            s_ss0Direct.strWorkBase80049428.byte80057119),
        static_cast<unsigned>(lowerCdStart.status0));

    if (!SS0DirectPumpStrXaAudioOutputAdapter8001A4D0(
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                sectorRingRawSectorCursor,
            owner)) {
        Log::Printf(
            "SS0 direct runtime: %s original-disc XA initial pump blocked row=%u",
            owner ? owner : "Scene0",
            sceneEntryRowIndex);
        (void)SS0DirectExecuteStrStopReset80027664(
            "original-disc XA initial pump rollback");
        s_ss0Direct.strStarted = false;
        s_ss0Direct.strLowerCdStart8001A4D0 = {};
        s_ss0Direct.strLowerCdClock801C4350 = {};
        return false;
    }

    s_ss0Direct.strStarted = true;
    s_ss0Direct.openingMovie0FrozenFrameHeld = false;
    Log::Printf("SS0 direct runtime: %s STR started directMdecTextureOnly=1 extractedStrFileAuthority=0 row=%u volume=%d/%.3f filter=%u/%u command=%u decoderKind=%d alloc=%u/%u/%u/%u directMemoryBytes=%llu dst=%d,%d decoded=%ux%u ringSlots=%u ringSlotBytes=%u ringLifecycleAuthority=1 callback=%08X streamStart=%08X stSetStream=%d/%d/%d/%d/%d vlcSize=%d sectorCounter=%08X:%u decodeAdvance=%08X planned=%u executed=%u inputGap=%u decodeGate=%08X branch=%u callAdvance=%u stripUpload=%08X warmupSkip=%u uploadCount=%u hostProjection=0 hostLifecycleAuthority=0 directPsxStatusAuthority=1 lowerCdStartAuthority=1 sectorRingInputAuthority=1 decDctVlc2=%08X vlcInput=%u vlcOutput=%u release=%08X mdecCommand=%08X outputDma=%08X directCommandStateAuthority=1 mdecHardwareOutputAuthority=0 gpuUploadExecuted=0 streamLibraryAuthority=0 psxPointerAuthority=0 psxHardwareMmioAuthority=0 hardwareCallbackTimingAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
                owner ? owner : "Scene0",
                sceneEntryRowIndex,
                static_cast<int>(audioRoute.volume8001A478),
                static_cast<double>(audioRoute.normalizedVolume),
                static_cast<unsigned>(audioRoute.filterFile8004940C),
                static_cast<unsigned>(audioRoute.filterChannel8004940D),
                static_cast<unsigned>(audioRoute.filterCommand8001A654),
                movieKindValue,
                decoderState.allocationSizes[0],
                decoderState.allocationSizes[1],
                decoderState.allocationSizes[2],
                decoderState.allocationSizes[3],
                static_cast<unsigned long long>(
                    s_ss0Direct.strDecoderMemoryRuntime80027288.
                        allocationBytes),
                decoderState.control.dstX,
                decoderState.control.dstY,
                decoderState.control.decodedWidth,
                decoderState.control.decodedHeight,
                decoderState.sectorRingSlots8003624C,
                s_ss0Direct.strDecoderMemoryRuntime80027288.
                    sectorRingSlotBytes,
                decoderState.outputCallbackFunction80027220,
                PrSS0StrLifecycleDirect::kFn800274D4,
                streamStartState.stSetStreamArgs[0],
                streamStartState.stSetStreamArgs[1],
                streamStartState.stSetStreamArgs[2],
                streamStartState.stSetStreamArgs[3],
                streamStartState.stSetStreamArgs[4],
                streamStartState.decDctVlcSize2Argument,
                streamStartState.sectorCounterAddress800965B8,
                streamStartState.sectorCounterValue,
                PrSS0StrLifecycleDirect::kFn800273A4,
                decodeAdvanceState.planned ? 1u : 0u,
                decodeAdvanceState.executed ? 1u : 0u,
                decodeAdvanceState.hasOpenInputGap ? 1u : 0u,
                PrSS0StrLifecycleDirect::kFn80027528,
                static_cast<unsigned>(decodeGateState.branch),
                decodeGateState.callDecodeAdvance800273A4 ? 1u : 0u,
                PrSS0StrLifecycleDirect::kFn8002756C,
                stripUploadState.skippedForCounterWarmup ? 1u : 0u,
                stripUploadState.stripCount,
                0u,
                PrSS0StrLifecycleDirect::kFnDecDCTvlc2,
                vlcDecodeRelease.inputBytes,
                vlcDecodeRelease.vlc.outputBytesWritten,
                PrSS0StrLifecycleDirect::kFn80039490,
                PrSS0StrLifecycleDirect::kFn80047558,
                PrSS0StrLifecycleDirect::kFn8004780C);
    return true;
}

static bool SS0DirectPumpMdecPollPair80027528(const char* owner) {
    using namespace PrSS0StrLifecycleDirect;

    if (s_ss0Direct.strDirectMdecStreamExhausted) {
        return true;
    }
    if (!s_ss0Direct.strMdecCompletionPending) {
        Log::Printf(
            "SS0 direct runtime: %s 80027220 completion missing frames=%u lastFrame=%u",
            owner ? owner : "Scene0",
            s_ss0Direct.strDirectMdecFramesDecoded,
            s_ss0Direct.strDirectMdecLastFrameNumber);
        return false;
    }

    const StrMdecDmaCompletionState80027220 completion =
        ExecuteStrMdecDmaCompletion80027220(
            s_ss0Direct.strDecodeAdvance800273A4.control,
            s_ss0Direct.strMdecCommandOutput800273A4,
            s_ss0Direct.strMdecOutputCurrentScus,
            s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!completion.known || !completion.executed ||
        !completion.outputReadyCleared ||
        !completion.decodeInFlightPreserved ||
        !completion.exactCurrentIdaStoreAuthority ||
        !completion.directSynchronousOutputCompletionAuthority ||
        completion.hardwareCallbackTimingAuthority ||
        completion.psxPointerAuthority || completion.psxHardwareMmioAuthority ||
        completion.hostProjection || completion.replayValueAuthority ||
        completion.oldWinS0Authority || completion.stage2PlusAuthority ||
        completion.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 80027220 completion blocked frames=%u lastFrame=%u",
            owner ? owner : "Scene0",
            s_ss0Direct.strDirectMdecFramesDecoded,
            s_ss0Direct.strDirectMdecLastFrameNumber);
        return false;
    }
    s_ss0Direct.strMdecCompletion80027220 = completion;
    s_ss0Direct.strDecodeAdvance800273A4.control = completion.controlAfter;
    s_ss0Direct.strMdecCompletionPending = false;

    const StrDecodeGateState80027528 firstPoll =
        BuildStrDecodeGateState80027528(completion.controlAfter);
    if (!firstPoll.known || !firstPoll.branchSemanticsTranslated ||
        firstPoll.branch != StrDecodeGateBranch80027528::OutputNotReady ||
        !firstPoll.callDecodeAdvance800273A4 ||
        firstPoll.hostProjection || firstPoll.replayValueAuthority ||
        firstPoll.oldWinS0Authority || firstPoll.stage2PlusAuthority ||
        firstPoll.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s first 80027528 poll blocked outputReady=%u decodeInFlight=%u",
            owner ? owner : "Scene0",
            firstPoll.outputReady,
            firstPoll.decodeInFlight);
        return false;
    }

    const StrSectorRingPrimeResult80039670 prime =
        PrimeStrSectorRingFromDisc80039670(
            s_ss0Direct.strLowerCdStart8001A4D0,
            s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!prime.known || !prime.directDiscSectorAuthority ||
        !prime.synchronousCallbackOrdering ||
        prime.hardwareCallbackTimingAuthority || prime.psxPointerAuthority ||
        prime.hostProjection || prime.replayValueAuthority ||
        prime.oldWinS0Authority || prime.stage2PlusAuthority ||
        prime.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s ongoing 80039670 prime blocked cursor=%u/%u",
            owner ? owner : "Scene0",
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                sectorRingRawSectorCursor,
            s_ss0Direct.strLowerCdStart8001A4D0.logicalSectorCount);
        return false;
    }

    const StrSectorRingAcquireResult8003958C acquire =
        AcquireStrSectorRingFrame8003958C(
            s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!acquire.known || acquire.psxPointerAuthority ||
        acquire.hostProjection || acquire.replayValueAuthority ||
        acquire.oldWinS0Authority || acquire.stage2PlusAuthority ||
        acquire.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s ongoing 8003958C acquire blocked",
            owner ? owner : "Scene0");
        return false;
    }

    StrStreamStartState800274D4 stream =
        s_ss0Direct.strStreamStart800274D4;
    stream.control = completion.controlAfter;
    StrDecodeAdvanceInput800273A4 advanceInput{};
    advanceInput.acquireReturnKnown = true;
    advanceInput.acquireReturn = acquire.returnValue;
    advanceInput.sectorCounterBefore =
        s_ss0Direct.strDecodeAdvance800273A4.sectorCounterAfter;
    advanceInput.sectorRingExecutionAuthority = true;

    if (!acquire.available) {
        const StrDecodeAdvanceState800273A4 unavailableAdvance =
            BuildStrDecodeAdvanceState800273A4(stream, advanceInput);
        if (acquire.returnValue == 0 ||
            !unavailableAdvance.planned || !unavailableAdvance.executed ||
            unavailableAdvance.hasOpenInputGap ||
            unavailableAdvance.branch !=
                StrDecodeAdvanceBranch800273A4::AcquireUnavailable ||
            unavailableAdvance.control.outputReady != 0u ||
            unavailableAdvance.control.decodeInFlight != 0u ||
            !unavailableAdvance.runtimeInputBound ||
            !unavailableAdvance.sectorRingExecutionAuthority ||
            unavailableAdvance.hostProjection ||
            unavailableAdvance.replayValueAuthority ||
            unavailableAdvance.oldWinS0Authority ||
            unavailableAdvance.stage2PlusAuthority ||
            unavailableAdvance.comod2Authority) {
            Log::Printf(
                "SS0 direct runtime: %s ongoing 800273A4 empty acquire blocked return=%d",
                owner ? owner : "Scene0",
                acquire.returnValue);
            return false;
        }
        s_ss0Direct.strDecodeAdvance800273A4 = unavailableAdvance;
        s_ss0Direct.strDecodeGate80027528 =
            BuildStrDecodeGateState80027528(unavailableAdvance.control);
        s_ss0Direct.strDirectMdecStreamExhausted =
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                sectorRingRawSectorCursor >=
            s_ss0Direct.strLowerCdStart8001A4D0.logicalSectorCount;
        Log::Printf(
            "SS0 direct runtime: %s 80027220/80027528 poll pair no frame cursor=%u/%u exhausted=%u secondCallAdvance=%u hardwareCallbackTimingAuthority=0 hostProjection=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
            owner ? owner : "Scene0",
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                sectorRingRawSectorCursor,
            s_ss0Direct.strLowerCdStart8001A4D0.logicalSectorCount,
            s_ss0Direct.strDirectMdecStreamExhausted ? 1u : 0u,
            s_ss0Direct.strDecodeGate80027528.
                    callDecodeAdvance800273A4
                ? 1u
                : 0u);
        return true;
    }

    if (acquire.returnValue != 0 || !acquire.payloadHandleKnown ||
        acquire.payloadHandle == 0u || !acquire.sectorHeaderKnown ||
        !acquire.stateTransition2To4 ||
        !acquire.directSectorRingAuthority) {
        Log::Printf(
            "SS0 direct runtime: %s ongoing 8003958C acquired state blocked frame=%u",
            owner ? owner : "Scene0",
            acquire.frameNumber);
        return false;
    }
    advanceInput.payloadHandleKnown = acquire.payloadHandleKnown;
    advanceInput.payloadHandle = acquire.payloadHandle;
    advanceInput.sectorHeaderKnown = acquire.sectorHeaderKnown;
    advanceInput.sectorWidth = acquire.width;
    advanceInput.sectorHeight = acquire.height;

    const StrVlcDecodeReleaseResult800273A4 vlc =
        ExecuteStrVlcDecodeAndRelease800273A4(
            acquire.payloadHandle,
            s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!vlc.known || !vlc.executed || !vlc.vlc.known ||
        !vlc.vlc.executed || vlc.vlc.returnValue != 0 ||
        !vlc.vlc.frameComplete || !vlc.releaseFrameCalled80039490 ||
        !vlc.release.known || !vlc.release.released ||
        !vlc.exactCurrentIdaCallOrder ||
        !vlc.currentScusSemanticAuthority ||
        !vlc.directSectorRingAuthority ||
        vlc.mdecHardwareExecutionAuthority || vlc.psxPointerAuthority ||
        vlc.hostProjection || vlc.replayValueAuthority ||
        vlc.oldWinS0Authority || vlc.stage2PlusAuthority ||
        vlc.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s ongoing DecDCTvlc2/release blocked frame=%u return=%d",
            owner ? owner : "Scene0",
            acquire.frameNumber,
            vlc.vlc.returnValue);
        return false;
    }

    StrDecoderControlState80027288 mdecControl = completion.controlAfter;
    mdecControl.decodedWidth = acquire.width;
    mdecControl.decodedHeight = acquire.height;
    mdecControl.vlcDecodeResult =
        static_cast<uint32_t>(vlc.vlc.returnValue);
    mdecControl.decodeInFlight = 1u;
    const StrMdecCommandOutputSubmission800273A4 submission =
        ExecuteStrMdecCommandOutputSubmission800273A4(
            mdecControl,
            vlc,
            s_ss0Direct.strDecoderMemoryRuntime80027288);
    if (!submission.known || !submission.executed ||
        !submission.outputReadyStored800273A4 ||
        !submission.dmaCompletionCallbackPending80027220 ||
        !submission.exactCurrentIdaCallOrder ||
        !submission.currentScusSemanticAuthority ||
        !submission.directCommandStateAuthority ||
        submission.mdecHardwareExecutionAuthority ||
        submission.hardwareCallbackTimingAuthority ||
        submission.psxPointerAuthority || submission.psxHardwareMmioAuthority ||
        submission.hostProjection || submission.replayValueAuthority ||
        submission.oldWinS0Authority || submission.stage2PlusAuthority ||
        submission.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s ongoing MDEC submission blocked frame=%u",
            owner ? owner : "Scene0",
            acquire.frameNumber);
        return false;
    }

    const PrSS0MdecOutputDirect::Mdec15bppResult output =
        PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                allocations[2].data(),
            vlc.vlc.outputBytesWritten,
            acquire.width,
            acquire.height,
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                allocations[3].data(),
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                allocations[3].size());
    if (!output.known || !output.executed ||
        output.commandWord != submission.vlcCommandWordAfter ||
        output.inputWordCount != submission.inputDmaWordCount80047778 ||
        output.outputBytesWritten !=
            submission.outputDmaWordCount8004780C * sizeof(uint32_t) ||
        !output.output15bpp || !output.outputUnsigned ||
        !output.outputBit15Set || !output.dma1VerticalMacroblockLayout ||
        !output.currentScusQuantTableAuthority ||
        !output.currentScusScaleTableAuthority ||
        !output.currentIdaCommandAuthority ||
        !output.documentedPsxMdecAlgorithmAuthority ||
        output.mdecHardwareBitExactAuthority ||
        output.dmaCompletionCallbackTimingAuthority ||
        output.psxHardwareMmioAuthority || output.hostProjection ||
        output.replayValueAuthority || output.oldWinS0Authority ||
        output.stage2PlusAuthority || output.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s ongoing current-SCUS MDEC output blocked frame=%u",
            owner ? owner : "Scene0",
            acquire.frameNumber);
        return false;
    }

    advanceInput.vlcDecodeResultKnown = true;
    advanceInput.vlcDecodeResult = vlc.vlc.returnValue;
    advanceInput.mdecCommandOutputSubmissionKnown = true;
    advanceInput.mdecCommandOutputSubmissionAuthority = true;
    const StrDecodeAdvanceState800273A4 advance =
        BuildStrDecodeAdvanceState800273A4(stream, advanceInput);
    if (!advance.planned || !advance.executed || advance.hasOpenInputGap ||
        advance.branch != StrDecodeAdvanceBranch800273A4::
                              MdecOutputSubmittedHardwareBoundary ||
        !advance.runtimeInputBound ||
        !advance.sectorRingExecutionAuthority ||
        !advance.mdecCommandOutputSubmissionAuthority ||
        advance.control.outputReady != 1u ||
        advance.control.decodeInFlight != 1u ||
        advance.mdecExecutionAuthority || advance.psxPointerAuthority ||
        advance.hostProjection || advance.replayValueAuthority ||
        advance.oldWinS0Authority || advance.stage2PlusAuthority ||
        advance.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s ongoing 800273A4 advance blocked frame=%u",
            owner ? owner : "Scene0",
            acquire.frameNumber);
        return false;
    }
    const StrDecodeGateState80027528 secondPoll =
        BuildStrDecodeGateState80027528(advance.control);
    if (!secondPoll.known || !secondPoll.branchSemanticsTranslated ||
        secondPoll.branch != StrDecodeGateBranch80027528::DecodeBusy ||
        secondPoll.callDecodeAdvance800273A4 ||
        !secondPoll.returnValueKnown || secondPoll.returnValue != 1 ||
        secondPoll.hostProjection || secondPoll.replayValueAuthority ||
        secondPoll.oldWinS0Authority || secondPoll.stage2PlusAuthority ||
        secondPoll.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s second 80027528 poll blocked frame=%u",
            owner ? owner : "Scene0",
            acquire.frameNumber);
        return false;
    }

    s_ss0Direct.strVlcDecodeRelease800273A4 = vlc;
    s_ss0Direct.strMdecCommandOutput800273A4 = submission;
    s_ss0Direct.strMdecOutputCurrentScus = output;
    s_ss0Direct.strDecodeAdvance800273A4 = advance;
    s_ss0Direct.strDecodeGate80027528 = secondPoll;
    s_ss0Direct.strMdecCompletionPending = true;
    ++s_ss0Direct.strDirectMdecFramesDecoded;
    s_ss0Direct.strDirectMdecLastFrameNumber = acquire.frameNumber;
    Log::Printf(
        "SS0 direct runtime: %s 80027220/80027528 poll pair frame=%u frames=%u sectors=%u VLC=%u halfwords=%u output=%u fnv1a=%016llX firstCallAdvance=1 secondBusy=1 oneFramePerTwoLogicTicks=1 sourceFps~15 logicHz=30 hardwareCallbackTimingAuthority=0 gpuUploadExecuted=0 hostProjection=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "Scene0",
        acquire.frameNumber,
        s_ss0Direct.strDirectMdecFramesDecoded,
        static_cast<unsigned>(prime.sectorCount),
        vlc.vlc.outputBytesWritten,
        output.inputHalfwordsConsumed,
        output.outputBytesWritten,
        static_cast<unsigned long long>(output.outputFnv1a64));
    return true;
}

// The original STR payloads are ~15 fps while the translated game logic is
// intentionally stepped at 30 Hz.  A decode/upload is therefore due on one
// of every two logic ticks.  Render-only 60 Hz callbacks never call this
// helper, so they can only hold the most recently submitted PSX frame.
static bool SS0DirectVideoFrameDue(const char* owner) {
    const bool due = (s_ss0Direct.strVideoFrameCadenceTick & 1u) != 0u;
    s_ss0Direct.strVideoFrameCadenceTick = static_cast<uint8_t>(
        s_ss0Direct.strVideoFrameCadenceTick + 1u);
    if (!s_ss0Direct.strVideoCadenceLogged) {
        s_ss0Direct.strVideoCadenceLogged = true;
        Log::Printf(
            "SS0 direct runtime: %s STR video cadence gate sourceFps~15 logicHz=30 renderHz=60 decodeEveryTwoLogicTicks=1",
            owner ? owner : "Scene0");
    }
    return due;
}

static bool SS0DirectExecuteStrStripUpload8002756C(
    PrGameContext& ctx,
    const char* owner) {
    using namespace PrSS0StrLifecycleDirect;

    const uint32_t sectorCounter =
        s_ss0Direct.strDecodeAdvance800273A4.sectorCounterAfter;
    if (sectorCounter < 2u) {
        const StrStripUploadState8002756C warmup =
            BuildStrStripUploadState8002756C(
                s_ss0Direct.strDecodeAdvance800273A4.control,
                sectorCounter,
                false,
                0);
        if (!warmup.known || !warmup.skippedForCounterWarmup ||
            warmup.uploadCallsPlanned80044D64 ||
            warmup.gpuUploadExecuted || warmup.psxImagePointerAuthority ||
            warmup.hostProjection || warmup.replayValueAuthority ||
            warmup.oldWinS0Authority || warmup.stage2PlusAuthority ||
            warmup.comod2Authority) {
            return false;
        }
        s_ss0Direct.strStripUpload8002756C = warmup;
        return true;
    }

    const PrPsxGraphOwnerDirect::PsxGraphState* graph =
        PrSS0TitleTmdBackend::GetTitleGraphState();
    if (ctx.renderer == nullptr || graph == nullptr ||
        graph->word_80096590 > 1u) {
        Log::Printf(
            "SS0 direct runtime: %s 8002756C direct display-buffer binding blocked renderer=%u graph=%u lane=%u",
            owner ? owner : "Scene0",
            ctx.renderer != nullptr ? 1u : 0u,
            graph != nullptr ? 1u : 0u,
            graph != nullptr
                ? static_cast<unsigned>(graph->word_80096590)
                : 0u);
        return false;
    }
    const int32_t displayBuffer =
        static_cast<int32_t>(
            PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(*graph));
    const StrStripUploadState8002756C plan =
        BuildStrStripUploadState8002756C(
            s_ss0Direct.strDecodeAdvance800273A4.control,
            sectorCounter,
            true,
            displayBuffer);
    if (!plan.known || plan.skippedForCounterWarmup ||
        plan.skippedForZeroStrips || !plan.displayBufferResultKnown ||
        plan.displayBufferResult8004019C != displayBuffer ||
        plan.stripCount == 0u || plan.truncated ||
        !plan.uploadCallsPlanned80044D64 || plan.gpuUploadExecuted ||
        plan.psxImagePointerAuthority || plan.hostProjection ||
        plan.replayValueAuthority || plan.oldWinS0Authority ||
        plan.stage2PlusAuthority || plan.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 8002756C strip plan blocked counter=%u lane=%d strips=%u",
            owner ? owner : "Scene0",
            sectorCounter,
            displayBuffer,
            plan.stripCount);
        return false;
    }

    StrStripUploadExecution8002756C execution =
        ExecuteStrStripUpload8002756C(
            plan,
            s_ss0Direct.strDecoderMemoryRuntime80027288.
                allocations[3].data(),
            s_ss0Direct.strMdecOutputCurrentScus.outputBytesWritten);
    if (!execution.known || !execution.executed ||
        execution.commandsExecuted80044D64 != plan.stripCount ||
        execution.sourceBytesConsumed !=
            s_ss0Direct.strMdecOutputCurrentScus.outputBytesWritten ||
        execution.sourceFnv1a64 !=
            s_ss0Direct.strMdecOutputCurrentScus.outputFnv1a64 ||
        !execution.exactCurrentIdaStripOrder ||
        !execution.directMdecVerticalLayoutAuthority ||
        !execution.directRgbaAssemblyAuthority ||
        !execution.translatedGpuDmaStateAuthority ||
        !execution.translatedGpuCompletionAuthority ||
        execution.translatedGpuCommandCount != plan.stripCount ||
        !execution.hostTextureUploadPending ||
        execution.hostTextureUploadExecuted ||
        execution.psxGpuDmaExecutionAuthority ||
        execution.psxPointerAuthority || execution.replayValueAuthority ||
        execution.oldWinS0Authority || execution.stage2PlusAuthority ||
        execution.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 8002756C direct strip execution blocked frame=%u source=%u rgba=%u",
            owner ? owner : "Scene0",
            s_ss0Direct.strDirectMdecLastFrameNumber,
            execution.sourceBytesConsumed,
            execution.rgbaBytesWritten);
        return false;
    }

    const bool recreateTexture =
        s_ss0Direct.strDirectMdecTexture == nullptr ||
        s_ss0Direct.strDirectMdecTextureRenderer != ctx.renderer ||
        s_ss0Direct.strStripUploadExecution8002756C.width !=
            execution.width ||
        s_ss0Direct.strStripUploadExecution8002756C.height !=
            execution.height;
    if (recreateTexture) {
        SS0DirectDestroyStrDirectTexture();
        s_ss0Direct.strDirectMdecTexture = ctx.renderer->CreateTexture(
            execution.rgbaPixels.data(),
            static_cast<int>(execution.width),
            static_cast<int>(execution.height));
        s_ss0Direct.strDirectMdecTextureRenderer = ctx.renderer;
        if (s_ss0Direct.strDirectMdecTexture == nullptr) {
            Log::Printf(
                "SS0 direct runtime: %s direct MDEC texture creation failed frame=%u",
                owner ? owner : "Scene0",
                s_ss0Direct.strDirectMdecLastFrameNumber);
            return false;
        }
    } else if (!ctx.renderer->TryUpdateTexture(
                   s_ss0Direct.strDirectMdecTexture,
                   execution.rgbaPixels.data(),
                   static_cast<int>(execution.width),
                   static_cast<int>(execution.height))) {
        Log::Printf(
            "SS0 direct runtime: %s direct MDEC texture update failed frame=%u",
            owner ? owner : "Scene0",
            s_ss0Direct.strDirectMdecLastFrameNumber);
        return false;
    }

    execution.hostTextureUploadPending = false;
    execution.hostTextureUploadExecuted = true;
    const auto pagePublish =
        PublishStrVramPageUpload8002756C(
            s_ss0Direct.strVramPageRuntime8001B120,
            static_cast<uint16_t>(displayBuffer),
            execution);
    s_ss0Direct.strVramPagePublish8002756C = pagePublish;
    if (!pagePublish.known || !pagePublish.executionAccepted ||
        !pagePublish.exactFullPageShape ||
        !pagePublish.directVramPixelStateAuthority ||
        pagePublish.pixelCount != kStrVramPagePixelCount8001B120 ||
        pagePublish.sourcePixelCount != execution.rgbaPixels.size() ||
        pagePublish.sourceRgbaFnv1a64 != execution.rgbaFnv1a64 ||
        pagePublish.hostProjection || pagePublish.replayValueAuthority ||
        pagePublish.oldWinS0Authority || pagePublish.stage2PlusAuthority ||
        pagePublish.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: %s 8002756C direct VRAM page publish blocked lane=%d pixels=%u",
            owner ? owner : "Scene0",
            displayBuffer,
            pagePublish.pixelCount);
        return false;
    }
    s_ss0Direct.strStripUpload8002756C = plan;
    s_ss0Direct.strStripUploadExecution8002756C = std::move(execution);
    s_ss0Direct.strDirectMdecTextureReady = true;
    s_ss0Direct.strDirectMdecTextureFrameNumber =
        s_ss0Direct.strDirectMdecLastFrameNumber;
    ++s_ss0Direct.strDirectMdecTextureUploadCount;
    Log::Printf(
        "SS0 direct runtime: %s 8002756C translated GPU/DMA upload frame=%u counter=%u lane=%d dstY=%d strips=%u source=%u rgba=%u rgbaFnv1a=%016llX vramPagePixels=%u translatedGpuDmaStateAuthority=1 translatedGpuCompletionAuthority=1 hostTextureUploadExecuted=1 exactCurrentIdaStripOrder=1 psxGpuDmaExecutionAuthority=0 hostProjection=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        owner ? owner : "Scene0",
        s_ss0Direct.strDirectMdecTextureFrameNumber,
        sectorCounter,
        displayBuffer,
        plan.destinationY,
        plan.stripCount,
        s_ss0Direct.strStripUploadExecution8002756C.sourceBytesConsumed,
        s_ss0Direct.strStripUploadExecution8002756C.rgbaBytesWritten,
        static_cast<unsigned long long>(
            s_ss0Direct.strStripUploadExecution8002756C.rgbaFnv1a64),
        s_ss0Direct.strVramPagePublish8002756C.pixelCount);
    return true;
}

static CardHandoff::CardMode800191E4 SS0DirectCurrentCardMode800191E4() {
    if (s_ss0Direct.phase == SS0DirectPhase::LoadCard) {
        return CardHandoff::CardMode800191E4::Load;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::ReplayCard) {
        return CardHandoff::CardMode800191E4::Replay;
    }
    return CardHandoff::CardMode800191E4::Unknown;
}

static bool SS0DirectState16DirectoryTitleMatches(const char* lhs,
                                                  const char* rhs) {
    if (lhs == nullptr || rhs == nullptr) {
        return false;
    }
    if (std::memchr(
            lhs,
            '\0',
            sizeof(s_ss0Direct.state16TypedFactsDirectoryTitle)) == nullptr ||
        std::memchr(
            rhs,
            '\0',
            CardHandoff::kCardDirectoryNameMax80017900 + 1u) == nullptr) {
        return false;
    }
    return std::strcmp(lhs, rhs) == 0;
}

static const char* SS0DirectCurrentCardRowName8007A590() {
    const auto& directory = s_ss0Direct.cardDirectoryCarrier80019D7C;
    if (!s_ss0Direct.cardDirectoryAuthorityKnown ||
        !directory.entryCountKnown ||
        s_ss0Direct.cardCursor < 0 ||
        s_ss0Direct.cardCursor >= directory.entryCount ||
        s_ss0Direct.cardCursor >= 15) {
        return "";
    }
    const auto& row = directory.rows[s_ss0Direct.cardCursor];
    if (!row.rowNameKnown8007A590) {
        return "";
    }
    return row.rowName8007A590;
}

static void SS0DirectClearState16DirectoryBinding() {
    s_ss0Direct.state16TypedFactsDirectoryBindingKnown = false;
    s_ss0Direct.state16TypedFactsDirectoryGeneration = 0;
    s_ss0Direct.state16TypedFactsDirectoryMode =
        CardHandoff::CardMode800191E4::Unknown;
    s_ss0Direct.state16TypedFactsDirectoryBlock = -1;
    s_ss0Direct.state16TypedFactsDirectoryTitle[0] = '\0';
}

static void SS0DirectClearState16TypedPayloadAuthority() {
    PrSS0State16RuntimeOneShotDirect::
        ClearRuntimeState16CardReadTypedFactsOneShot(
            s_ss0Direct.state16TypedFactsOneShot);
    PrStage1SaveCardHalDirect::ClearState16CardReadTypedCarrier800179B4();
    SS0DirectClearState16DirectoryBinding();
}

static void SS0DirectClearLoadReplayCompletion80019D7C() {
    s_ss0Direct.cardCompletionKind80019D7C =
        SS0DirectLoadReplayCompletionKind80019D7C::None;
    s_ss0Direct.cardCompletionReplayScene80019D7C = -1;
    s_ss0Direct.cardCompletionRollbackPrefixKnown80092F10 = false;
    s_ss0Direct.cardCompletionRollbackPrefix80092F10 = {};
}

static void SS0DirectClearLoadReplayState16CardIoResult80017594() {
    s_ss0Direct.cardState16IoResultCarrier80017594 = {};
}

static void SS0DirectClearCardDirectoryAuthority() {
    s_ss0Direct.cardDirectoryAuthorityKnown = false;
    s_ss0Direct.cardDirectoryMode = CardHandoff::CardMode800191E4::Unknown;
    s_ss0Direct.cardDirectoryCarrier80019D7C = {};
    s_ss0Direct.cardSelectedRowIdentity800181D0 = {};
    s_ss0Direct.cardSelectedRowDirectoryGeneration = 0;
    s_ss0Direct.cardGridDrawState80020F94 = {};
    s_ss0Direct.cardListInputRuntime800181D0 = {};
    s_ss0Direct.cardDriverVisualRuntime80018FB0 = {};
    SS0DirectClearLoadReplayState16CardIoResult80017594();
    s_ss0Direct.cardPromptEventFrame8001E750 = {};
    SS0DirectClearLoadReplayCompletion80019D7C();
    ++s_ss0Direct.cardDirectoryGeneration;
    SS0DirectClearState16TypedPayloadAuthority();
}

static void SS0DirectClearTypedCardReadCarriers() {
    SS0DirectClearState16TypedPayloadAuthority();
    PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
}

static void SS0DirectClearReplayRawSelectorFixture() {
    s_ss0Direct.debugReplayRawSelectorArmed = false;
    s_ss0Direct.debugReplayRawSelectorBlock = -1;
    s_ss0Direct.debugReplayRawSelectorSavedSlot = 0;
}

static bool SS0DirectQueueRuntimeState16CardReadTypedFactsOneShot(
    const PrStage1SaveCardHalDirect::State16CardReadRuntimeTypedFacts800179B4&
        facts) {
    s_ss0Direct.state16TypedFactsOneShot = {};
    SS0DirectClearState16DirectoryBinding();
    const bool runtimeEnabled = PrSS0Direct::RuntimeCutoverAllowed();
    const bool phaseAllowed =
        s_ss0Direct.phase == SS0DirectPhase::LoadCard ||
        s_ss0Direct.phase == SS0DirectPhase::ReplayCard;
    if (!runtimeEnabled || !s_ss0Direct.initialized || !phaseAllowed) {
        Log::Printf(
            "SS0 direct runtime: state16 runtime typed facts queue rejected runtime=%d initialized=%d phase=%u phaseAllowed=%d",
            runtimeEnabled ? 1 : 0,
            s_ss0Direct.initialized ? 1 : 0,
            static_cast<unsigned>(s_ss0Direct.phase),
            phaseAllowed ? 1 : 0);
        return false;
    }

    if (s_ss0Direct.cardEntryCount <= 0 ||
        s_ss0Direct.cardEntryCount >
            PrStage1SaveCardHalDirect::kReadAttemptCount800179B4 ||
        s_ss0Direct.cardCursor < 0 ||
        s_ss0Direct.cardCursor >= s_ss0Direct.cardEntryCount ||
        s_ss0Direct.cardCursor >=
            PrStage1SaveCardHalDirect::kReadAttemptCount800179B4) {
        Log::Printf(
            "SS0 direct runtime: state16 runtime typed facts queue rejected cursor=%d entries=%d",
            s_ss0Direct.cardCursor,
            s_ss0Direct.cardEntryCount);
        return false;
    }

    const CardHandoff::CardMode800191E4 currentMode =
        SS0DirectCurrentCardMode800191E4();
    if (!s_ss0Direct.cardDirectoryAuthorityKnown ||
        s_ss0Direct.cardDirectoryMode != currentMode) {
        Log::Printf(
            "SS0 direct runtime: state16 runtime typed facts queue rejected directory authority known=%d mode=%s currentMode=%s generation=%u",
            s_ss0Direct.cardDirectoryAuthorityKnown ? 1 : 0,
            CardHandoff::CardMode800191E4Name(s_ss0Direct.cardDirectoryMode),
            CardHandoff::CardMode800191E4Name(currentMode),
            static_cast<unsigned>(s_ss0Direct.cardDirectoryGeneration));
        return false;
    }

    const int selectedBlock =
        s_ss0Direct.cardBlockIndex[s_ss0Direct.cardCursor];
    const char* selectedTitle = SS0DirectCurrentCardRowName8007A590();
    if (selectedBlock < 0 ||
        selectedBlock >=
            PrStage1SaveCardHalDirect::kReadAttemptCount800179B4 ||
        selectedTitle == nullptr ||
        selectedTitle[0] == '\0' ||
        !facts.selectedBlockKnown ||
        facts.selectedBlockIndex != selectedBlock ||
        !facts.selectedTitleKnown ||
        !SS0DirectState16DirectoryTitleMatches(facts.selectedTitle,
                                               selectedTitle) ||
        !facts.rowCountKnown ||
        facts.rowCount <= selectedBlock ||
        facts.rowCount >
            PrStage1SaveCardHalDirect::kReadAttemptCount800179B4) {
        Log::Printf(
            "SS0 direct runtime: state16 runtime typed facts queue rejected block=%d titleKnown=%d factKnown=%d factBlock=%d factTitleKnown=%d factTitle=%s rowCountKnown=%d rowCount=%d",
            selectedBlock,
            selectedTitle != nullptr && selectedTitle[0] != '\0' ? 1 : 0,
            facts.selectedBlockKnown ? 1 : 0,
            facts.selectedBlockIndex,
            facts.selectedTitleKnown ? 1 : 0,
            facts.selectedTitle,
            facts.rowCountKnown ? 1 : 0,
            facts.rowCount);
        return false;
    }

    if (!PrStage1SaveCardHalDirect::
            IsImportableState16RuntimeTypedFacts800179B4(
                facts,
                selectedBlock)) {
        Log::Printf(
            "SS0 direct runtime: state16 runtime typed facts queue rejected importable block=%d payloadKnown=%d ptr=%d bytes=%zu pollKnown=%d poll=%d returnKnown=%d return=%d",
            selectedBlock,
            facts.fullPayloadBytesKnown ? 1 : 0,
            facts.fullPayloadBytes != nullptr ? 1 : 0,
            facts.fullPayloadByteCount,
            facts.pollResultKnown ? 1 : 0,
            facts.pollResult80016EB8,
            facts.returnKnown ? 1 : 0,
            facts.psxReturn800179B4);
        return false;
    }

    if (!PrSS0State16RuntimeOneShotDirect::
            QueueRuntimeState16CardReadTypedFactsOneShot(
                s_ss0Direct.state16TypedFactsOneShot,
                facts)) {
        Log::Printf(
            "SS0 direct runtime: state16 runtime typed facts queue rejected payloadKnown=%d ptr=%d bytes=%zu",
            facts.fullPayloadBytesKnown ? 1 : 0,
            facts.fullPayloadBytes != nullptr ? 1 : 0,
            facts.fullPayloadByteCount);
        return false;
    }

    s_ss0Direct.state16TypedFactsDirectoryBindingKnown = true;
    s_ss0Direct.state16TypedFactsDirectoryGeneration =
        s_ss0Direct.cardDirectoryGeneration;
    s_ss0Direct.state16TypedFactsDirectoryMode =
        s_ss0Direct.cardDirectoryMode;
    s_ss0Direct.state16TypedFactsDirectoryBlock = selectedBlock;
    std::snprintf(s_ss0Direct.state16TypedFactsDirectoryTitle,
                  sizeof(s_ss0Direct.state16TypedFactsDirectoryTitle),
                  "%s",
                  selectedTitle);
    PrSS0State16RuntimeOneShotDirect::State16RuntimeTypedFactsOneShot&
        queued =
        s_ss0Direct.state16TypedFactsOneShot;
    Log::Printf(
        "SS0 direct runtime: state16 runtime typed facts queued blockKnown=%d block=%d rowCountKnown=%d rowCount=%d pollIterationsKnown=%d pollIterations=%d directoryGeneration=%u",
        queued.facts.selectedBlockKnown ? 1 : 0,
        queued.facts.selectedBlockIndex,
        queued.facts.rowCountKnown ? 1 : 0,
        queued.facts.rowCount,
        queued.facts.pollIterationCountKnown ? 1 : 0,
        queued.facts.pollIterationCount,
        static_cast<unsigned>(
            s_ss0Direct.state16TypedFactsDirectoryGeneration));
    return true;
}

static bool SS0DirectImportPendingState16TypedFactsBeforeConfirm(
    int blockIndex,
    const char* reason) {
    PrSS0State16RuntimeOneShotDirect::State16RuntimeTypedFactsOneShot&
        queued =
        s_ss0Direct.state16TypedFactsOneShot;
    if (!queued.pending) {
        return false;
    }
    const CardHandoff::CardMode800191E4 currentMode =
        SS0DirectCurrentCardMode800191E4();
    const char* currentTitle = SS0DirectCurrentCardRowName8007A590();
    if (!s_ss0Direct.state16TypedFactsDirectoryBindingKnown ||
        !s_ss0Direct.cardDirectoryAuthorityKnown ||
        s_ss0Direct.state16TypedFactsDirectoryGeneration !=
            s_ss0Direct.cardDirectoryGeneration ||
        s_ss0Direct.state16TypedFactsDirectoryMode != currentMode ||
        s_ss0Direct.state16TypedFactsDirectoryBlock != blockIndex ||
        !SS0DirectState16DirectoryTitleMatches(
            s_ss0Direct.state16TypedFactsDirectoryTitle,
            currentTitle)) {
        Log::Printf(
            "SS0 direct runtime: %s state16 runtime typed facts one-shot rejected directory binding known=%d authority=%d gen=%u currentGen=%u mode=%s currentMode=%s block=%d expectedBlock=%d",
            reason ? reason : "memcard",
            s_ss0Direct.state16TypedFactsDirectoryBindingKnown ? 1 : 0,
            s_ss0Direct.cardDirectoryAuthorityKnown ? 1 : 0,
            static_cast<unsigned>(
                s_ss0Direct.state16TypedFactsDirectoryGeneration),
            static_cast<unsigned>(s_ss0Direct.cardDirectoryGeneration),
            CardHandoff::CardMode800191E4Name(
                s_ss0Direct.state16TypedFactsDirectoryMode),
            CardHandoff::CardMode800191E4Name(currentMode),
            s_ss0Direct.state16TypedFactsDirectoryBlock,
            blockIndex);
        SS0DirectClearTypedCardReadCarriers();
        return false;
    }
    const bool published =
        PrSS0State16RuntimeOneShotDirect::
            ImportPendingState16TypedFactsBeforeConfirm(
                queued,
                blockIndex);
    SS0DirectClearState16DirectoryBinding();
    Log::Printf(
        "SS0 direct runtime: %s state16 runtime typed facts one-shot import block=%d published=%d",
        reason ? reason : "memcard",
        blockIndex,
        published ? 1 : 0);
    return published;
}

static void SS0DirectClearCardPageState(
    bool clearPendingDirectoryCarrier = true) {
    SS0DirectClearTypedCardReadCarriers();
    if (clearPendingDirectoryCarrier) {
        CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
    }
    SS0DirectClearCardDirectoryAuthority();
    s_ss0Direct.cardCursor = 0;
    s_ss0Direct.cardEntryCount = 0;
    std::fill(std::begin(s_ss0Direct.cardBlockIndex),
              std::end(s_ss0Direct.cardBlockIndex),
              -1);
    for (char (&title)[32] : s_ss0Direct.cardTitle) {
        title[0] = '\0';
    }
    s_ss0Direct.cardMessage[0] = '\0';
    s_ss0Direct.replayPayloadBackupPreflightKnown = false;
    SS0DirectClearReplayRawSelectorFixture();
}

static bool SS0DirectPhaseIsCardPage(SS0DirectPhase phase) {
    return phase == SS0DirectPhase::LoadCard ||
           phase == SS0DirectPhase::ReplayCard;
}

static void SS0DirectPublishOptionsCarrierContextMirrors80026910(PrGameContext& ctx) {
    ctx.languageIndex =
        static_cast<int16_t>(s_ss0Direct.optionsWord800916D8);
    ctx.subtitleFlag =
        static_cast<int16_t>(s_ss0Direct.optionsWord800916DC);
}

static void SS0DirectResetToTitle(PrGameContext& ctx) {
    const bool coldStart = !s_ss0Direct.initialized;

    // A Scene0 re-entry can follow a legacy-owned scene (for example after a
    // stage return).  Drop any stale Windows transition carrier at the SS0
    // boundary so it cannot tick or paint against the direct runtime.
    if (PrTransition::IsActive()) {
        PrTransition::Cancel();
    }

    if (coldStart) {
        // word_800916D2 is the process-lifetime Scene0 first-run latch. Keep
        // it outside later title resets so returning to Scene0 cannot repeat
        // the one-time 801C4DF8 transition.
        s_ss0Direct.word800916D2Known = true;
        s_ss0Direct.word800916D2 = 0u;
        PrPsxVSyncDirect::ResetProcessVSyncState80035E54();
        if (!PrSS0StrLifecycleDirect::
                TryInitializeStrWorkBaseRuntime80049428(
                    s_ss0Direct.strWorkBase80049428)) {
            Log::Printf(
                "SS0 direct runtime: cold-start 80049428 initialization blocked");
        }
    }

    ctx.stageRunning = false;
    ctx.transitionState = 0;
    ctx.transitionStateDA = 0;
    ctx.scn0PanelAnimActive = false;
    ctx.scn0PanelOffsetX = 0.0f;
    ctx.scn0PanelOffsetY = 0.0f;
    GetStageRunner().Reset();
    PrSfx::StopBgm();
    // 80016B84 already executes the startup 80026FA4 -> 800351B8(0) tail
    // before the first Logo on cold boot.  Do not replay it after the logos;
    // re-entry (where the pre-logo prefix is not run) still owns a fresh
    // reset at this boundary.
    if (!coldStart || !s_ss0Direct.bootStartupAudioResetCommitted80026FA4) {
        const auto reset = PrSfx::ApplySharedAudioResetBarrier26FA4();
        if (!reset.committed) {
            Log::Printf(
                "SS0 direct runtime: ResetToTitle 80026FA4 reset blocked coldStart=%d translated=%d known=%d",
                coldStart ? 1 : 0,
                reset.translated ? 1 : 0,
                reset.known ? 1 : 0);
        }
    }
    SS0DirectStopTitleStr(ctx);
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.openingMovie0InitialTransition);
    s_ss0Direct.openingMovie0InitialTransitionVisual =
        PrSS0TransitionDirect::SlowTransitionVisualFrame80020110{};
    s_ss0Direct.openingMovie0InitialTransitionPresentBlockedLogged = false;
    PrSS0TransitionDirect::ResetFastTransitionRuntime800201AC(
        s_ss0Direct.openingMovie0Transition);
    s_ss0Direct.openingMovie0TransitionVisual =
        PrSS0TransitionDirect::FastTransitionVisualFrame800201AC{};
    s_ss0Direct.openingMovie0TransitionPresentBlockedLogged = false;
    s_ss0Direct.openingMovie0FrozenFrameHeld = false;
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.titleIntroTransition);
    s_ss0Direct.titleIntroTransitionVisual =
        PrSS0TransitionDirect::SlowTransitionVisualFrame80020110{};
    s_ss0Direct.titleIntroTransitionPresentBlockedLogged = false;
    s_ss0Direct.titleMovie0TFinalReadyPollsRemaining = 0u;
    s_ss0Direct.titleMovie0TFinalReadyInterPollTicksRemaining = 0u;
    s_ss0Direct.titleMovie0TFinalReadyPreloopActive = false;
    s_ss0Direct.titleMovie0TFinalReadyBlockedLogged = false;
    s_ss0Direct.titleMovie0TState0GraphFlipBlockedLogged = false;

    s_ss0Direct.initialized = true;
    s_ss0Direct.phase = SS0DirectPhase::TitleSelector;
    s_ss0Direct.transitionReturnPhase = SS0DirectPhase::TitleSelector;
    s_ss0Direct.titleCursor = 0;
    s_ss0Direct.titleSelectorPreviousInputPsx = 0u;
    s_ss0Direct.titleMovie0TPreviousInputPsx = 0u;
    s_ss0Direct.titleFrameCounter = 0;
    s_ss0Direct.titleLoopTickV10 = -17;
    s_ss0Direct.titleLoopStateV8 = 0u;
    s_ss0Direct.titleNaturalLoopCadence = {};
    s_ss0Direct.titleInitialClock = {};
    s_ss0Direct.titleInitialCtxTick96Known = false;
    s_ss0Direct.titleInitialCtxTick96 = 0u;
    s_ss0Direct.titleCtxTick96Known = false;
    s_ss0Direct.titleCtxTick96 = 0u;
    s_ss0Direct.titleCtxBeatKnown = false;
    s_ss0Direct.titleCtxBeat = 0u;
    s_ss0Direct.titleSharedEventBucket = {};
    s_ss0Direct.titleSharedEventPredispatchContext = {};
    s_ss0Direct.titleSharedEventPredispatchLogged = false;
    s_ss0Direct.titleSharedEventDispatch = {};
    s_ss0Direct.titleSharedEventDispatchLogged = false;
    PrSS0TitleHudEventsDirect::ResetTitleAbsoluteEventStreamState80024E98(
        s_ss0Direct.titleAbsoluteEventStream);
    s_ss0Direct.titleEventRuntimeState = {};
    s_ss0Direct.titleSelectorHudRuntimeState = {};
    s_ss0Direct.titleSelectorResourceUpdate = {};
    s_ss0Direct.titleSelectorHudRuntimeState.resourceCooldownKnown = true;
    s_ss0Direct.titleSelectorHudRuntimeState.resourceCooldown = 0;
    s_ss0Direct.titleEventPending = false;
    s_ss0Direct.titlePendingEvent = {};
    s_ss0Direct.titleEntryPrefixTransaction = {};
    s_ss0Direct.titleEarlyInputShortcutPhase =
        SS0DirectTitleEarlyInputShortcutPhase::None;
    s_ss0Direct.titleEarlyInputShortcutCompletedWaitCalls = 0u;
    s_ss0Direct.titleEarlyInputShortcutFrameWaitRemaining = 0u;
    s_ss0Direct.titleEarlyInputPixelSubmitComplete = false;
    s_ss0Direct.titleEarlyInputPixelSubmitStatusLogged = false;
    PrSS0TitleTmdBackend::ResetRuntimeTitleState801C609C();
    // COMPO00 face TIMs are PSX runtime uploads into shared VRAM.  Returning
    // from a directory page must begin from the neutral title face instead
    // of inheriting whichever expression the previous title loop last wrote.
    // Keep this as the original resource path (handles 28/33/41), rather
    // than clearing the whole atlas and losing the static title textures.
    {
        PrSS0TitleHudEventsDirect::TitleHudTimRequest801C5094 neutralFace{};
        neutralFace.valid = true;
        neutralFace.slotId = 0u;
        neutralFace.recordIndex = 0u;
        neutralFace.timIdCount = 3u;
        neutralFace.timIds[0] = 28; // F_PAKU_0.TIM (neutral mouth)
        neutralFace.timIds[1] = 33; // F_PAMEL0.TIM (left eye)
        neutralFace.timIds[2] = 41; // F_PAMER0.TIM (right eye)
        const auto applied =
            PrSS0TitleTmdBackend::ApplyTitleHudTimRequest801C5094(
                ctx.resources, neutralFace);
        if (!applied.accepted) {
            Log::Printf(
                "SS0 direct runtime: title neutral face restore blocked status=%u",
                static_cast<unsigned>(applied.status));
        }
    }
    const auto& titleMimeInit801C609C =
        PrSS0TitleTmdBackend::GetTitleMimeInitState801C609C();
    s_ss0Direct.titleMimeInit801C609CReady =
        PrSS0TitleTmdBackend::IsExactTitleMimeInitState801C609C(
            titleMimeInit801C609C);
    if (s_ss0Direct.titleMimeInit801C609CReady) {
        Log::Printf(
            "SS0 direct runtime: title 801C609C MIME init committed 800139F8x4>8001EEE8>8001385Cx3>80013E40 channels=1/2/3 scratch=%u grid=%u hostProjection=0 oldWinS0Authority=0",
            static_cast<unsigned>(
                titleMimeInit801C609C.scratchZeroByteCount80013E40),
            static_cast<unsigned>(
                titleMimeInit801C609C.transitionGrid8001EEAC.size()));
    } else {
        Log::Printf(
            "SS0 direct runtime: title 801C609C MIME init blocked restore=%u channels=%u scratch=%u grid=%u",
            static_cast<unsigned>(
                titleMimeInit801C609C.baseRestoreCallCount800139F8),
            static_cast<unsigned>(
                titleMimeInit801C609C.channelInitCallCount8001385C),
            titleMimeInit801C609C.scratchClearCalled80013E40 ? 1u : 0u,
            titleMimeInit801C609C.transitionGridInitCalled8001EEE8 ? 1u
                                                                  : 0u);
    }
    PrSS0TitlePacketRenderDirect::ResetHostVisibleTitleFrameCache801C689C(
        s_ss0Direct.titleHostVisibleFrameCache);
    s_ss0Direct.titleAttractTimeoutKnown = false;
    s_ss0Direct.titleAttractTimeoutLimit = 0;
    s_ss0Direct.attractTimer = 0;
    s_ss0Direct.titleAttractTimeoutGapLogged = false;
    s_ss0Direct.titlePendingSelectorResult =
        PrSS0TitleHudEventsDirect::kTitleSelectorResultNone801C47EC;
    s_ss0Direct.titlePendingSourceScene = PrSceneId::Scene0;
    s_ss0Direct.titlePendingTargetScene = -1;
    s_ss0Direct.titlePendingMenu = false;
    s_ss0Direct.titleExitAudioResetCommitted80026FA4 = false;
    s_ss0Direct.titleExitLoadingResult801C4DC4 = {};
    s_ss0Direct.loadingPatternMode80015408 =
        PrSS0TransitionDirect::kLoadingPatternMode80015408;
    s_ss0Direct.titleSelectorConfirmationPresentPending = false;
    s_ss0Direct.titleExitWaitPrepareBlockedLogged = false;
    s_ss0Direct.titleExitWaitPresentGate = {};
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.titleExitTransition);
    s_ss0Direct.titleExitTransitionVisual =
        PrSS0TransitionDirect::SlowTransitionVisualFrame80020110{};
    s_ss0Direct.titleExitTransitionPresentBlockedLogged = false;
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.mainMenuEntryTransition);
    s_ss0Direct.mainMenuEntryTransitionVisual =
        PrSS0TransitionDirect::SlowTransitionVisualFrame80020110{};
    s_ss0Direct.mainMenuEntryTransitionPresentBlockedLogged = false;
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.sceneHandoffTransition80020110);
    s_ss0Direct.sceneHandoffTransitionVisual80020110 =
        PrSS0TransitionDirect::SlowTransitionVisualFrame80020110{};
    s_ss0Direct.sceneHandoffTransitionPresentBlockedLogged80020110 = false;
    s_ss0Direct.sceneHandoffRuntimeActive = false;
    s_ss0Direct.sceneHandoffTargetPending = false;
    s_ss0Direct.sceneHandoffTargetScene = -1;
    SS0DirectClearLoadingMinimumHold();
    s_ss0Direct.mainMenuIndex = 3;
    s_ss0Direct.mainMenuRecordsMode = 0;
    s_ss0Direct.mainMenuSubChoice = -1;
    s_ss0Direct.blinkCounter800916E4 = 0;
    s_ss0Direct.mainMenuBlink = 0;
    s_ss0Direct.optionsBlink = 0;
    s_ss0Direct.stageSelectBlink = 0;
    s_ss0Direct.mainMenuState =
        PrSS0DirectoryDispatcherDirect::InitMainMenuState80026794();
    s_ss0Direct.mainMenuState.cursor = 3;
    s_ss0Direct.mainMenuState.itemValue[1] = -1;
    s_ss0Direct.mainMenuState.itemValue[3] = -1;
    PrSS0EventFrameLoopDirect::ResetDispatcherTail80026B94(
        s_ss0Direct.mainMenuDispatcherTail80026B94);
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.mainMenuPreLoopPadRelease80026B94);
    s_ss0Direct.mainMenuPreLoopPadReleaseBlockedLogged = false;
    PrSS0EventFrameLoopDirect::ResetEvent3FrameTransaction80026B94(
        s_ss0Direct.mainMenuFrameTransaction80026B94);
    SS0DirectResetMainMenuFrameHostState8001E750();
    s_ss0Direct.mainMenuFrameTailCandidate80026B94 = {};
    s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94 = false;
    if (coldStart) {
        s_ss0Direct.optionsWord800916D8 = 0;
        s_ss0Direct.optionsWord800916DC = 1;
    }
    SS0DirectClearOptionsPageStatePreserveCarrier();
    SS0DirectPublishOptionsCarrierContextMirrors80026910(ctx);
    SS0DirectClearStageSelectState();
    SS0DirectClearCardPageState();
    s_ss0Direct.debugHiScoreEvent6EntryFixtureArmed = false;
    s_ss0Direct.hiScoreBlink = 0;
    SS0DirectClearHiScorePageState();
    SS0DirectClearPracticeState();
    // 80028590 zeroes the main RAM range once at process start.  A later
    // title re-entry must not erase a value already accepted from the live
    // runtime provider; the original 80015D18/title path has no F0 reset.
    if (!IsWord800916F0RuntimeObservationKnown()) {
        ClearWord800916F0RuntimeObservation();
        if (s_word800916F0DiscFullbootStartupPublisherDisabledForProbe ||
            kRequirePsvMemoryReplayForWord800916F0) {
            Log::Printf(
                "SS0 direct runtime: disc fullboot word_800916F0 startup observation skipped by probe-disable flag reason=psv-memory-replay-required");
        } else if (!PublishInitialWord800916F0FromDiscFullbootStartupSource()) {
            Log::Printf(
                "SS0 direct runtime: disc fullboot word_800916F0 startup observation rejected");
        }
    } else {
        Log::Printf(
            "SS0 direct runtime: preserved accepted word_800916F0 runtime observation across title reset value=%u",
            static_cast<unsigned>(GetWord800916F0()));
    }
    s_ss0Direct.menuUnsupportedLogged = false;
    s_ss0Direct.lastDebugPad = 0;
    Log::Printf("SS0 direct runtime: direct title selector entered");
}

static PrPadState SS0DirectReadPad(PrGameContext& ctx) {
    PrPadState pad = PrPad::GetState(0);
    const uint16_t dbgHeld = ctx.debugPadInput;
    const uint16_t dbgPressed =
        static_cast<uint16_t>(dbgHeld & static_cast<uint16_t>(~s_ss0Direct.lastDebugPad));
    s_ss0Direct.lastDebugPad = dbgHeld;
    pad.held = static_cast<uint16_t>(
        pad.held | MapScene0DebugPadToLocalPrPadMask(dbgHeld));
    pad.pressed = static_cast<uint16_t>(
        pad.pressed | MapScene0DebugPadToLocalPrPadMask(dbgPressed));
    return pad;
}

static bool SS0DirectPadPressed(const PrPadState& pad, PrPadButton button) {
    return (pad.pressed & static_cast<uint16_t>(button)) != 0;
}

static bool SS0DirectPadPressedAny(const PrPadState& pad, uint16_t mask) {
    return (pad.pressed & mask) != 0;
}

static bool SS0DirectPreflightSlowTransitionTextures80020110(
    PrGameContext& ctx,
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg);
static bool SS0DirectResolveMainDirectory80021E60(
    PrGameContext& ctx,
    const PrSS0DirectoryDispatcherDirect::MainMenuState800264AC&
        mainMenuState,
    float vx,
    float vy,
    float vs,
    bool contextPresent,
    uint16_t backdropPriority,
    bool submit);
static bool SS0DirectResolveHiScoreTable80021594(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    bool submit);
static auto SS0DirectResolveOptions80021910(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    bool submit) -> bool;
static auto SS0DirectResolveStageSelect80020568(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    bool submit) -> bool;
static bool SS0DirectPreflightFastTransitionTextures800201AC(
    PrGameContext& ctx,
    const PrSS0TransitionDirect::FastTransitionVisualFrame800201AC& visual);
static bool SS0DirectApplyOpeningMovie0TextWindow8001EC54(
    PrGameContext& ctx);
static bool SS0DirectTryCommitScene0Init801C4260(
    PrGameContext& ctx,
    bool forceResourceReload = false);
static bool SS0DirectStartScene0OuterEntry801C4DC4(PrGameContext& ctx);

static bool SS0DirectLoadScene0SubtitleData(PrGameContext& ctx) {
    if (s_ss0Direct.scene0SubtitleDataLoaded) {
        return s_ss0Direct.scene0SubtitleDataAvailable;
    }
    // Direct cutover deliberately clears ctx.currentComodBytes as legacy
    // overlay residue.  Re-open the immutable COMOD0 payload from the same
    // configured data root used by the direct resource ingress instead of
    // reviving that legacy ownership path.
    std::vector<uint8_t> comodBytes = ctx.currentComodBytes;
    std::filesystem::path sourcePath = ctx.currentComodPath;
    if (comodBytes.empty()) {
        sourcePath = ctx.dataRoot / "S0" / "COMOD0.BIN";
        std::ifstream input(sourcePath, std::ios::binary);
        if (input) {
            input.seekg(0, std::ios::end);
            const std::streamoff length = input.tellg();
            input.seekg(0, std::ios::beg);
            if (length > 0) {
                comodBytes.resize(static_cast<std::size_t>(length));
                input.read(reinterpret_cast<char*>(comodBytes.data()), length);
                if (!input) {
                    comodBytes.clear();
                }
            }
        }
    }
    if (comodBytes.empty()) {
        s_ss0Direct.scene0SubtitleDataLoaded = true;
        Log::Printf(
            "SS0 direct runtime: S0 subtitle source unavailable COMOD0 path='%s' bytes=0",
            sourcePath.u8string().c_str());
        PrStage1VTextDirectResetSub80024C84(
            s_ss0Direct.scene0SubtitleVtextRuntime);
        return false;
    }

    static constexpr PrStage1MovieTextDirect::PsxOverlayAddressResolver
        kScene0OverlayResolver[] = {{PrSS0Direct::kComod0MappedBase, 0u}};
    PrStage1MovieTextDirect::PsxMovieTextLoadSpec loadSpec{};
    loadSpec.overlayResolvers = kScene0OverlayResolver;
    loadSpec.overlayResolverCount =
        sizeof(kScene0OverlayResolver) / sizeof(kScene0OverlayResolver[0]);
    loadSpec.movieSubtitle = {
        kScene0Movie0VtextDesc801C6BF8,
        kScene0Movie0VtextEntryCount,
    };
    PrStage1MovieTextDirect::LoadFromComodSub80024C84(
        s_ss0Direct.scene0Movie0Vtext, comodBytes, loadSpec);
    const PrMovieSubtitles::MovieSubtitleTrack& track =
        PrStage1MovieTextDirect::GetMovieSubtitleTrack(
            s_ss0Direct.scene0Movie0Vtext);
    if (track.loaded && track.entryCount == kScene0Movie0VtextEntryCount &&
        track.lines.size() == kScene0Movie0VtextEntryCount) {
        s_ss0Direct.scene0SubtitleDataLoaded = true;
        s_ss0Direct.scene0SubtitleDataAvailable = true;
        s_ss0Direct.scene0SubtitleEventCount = track.entryCount;
        PrStage1VTextDirectResetSub80024C84(
            s_ss0Direct.scene0SubtitleVtextRuntime);
        const auto& first = track.lines.front();
        const auto& last = track.lines.back();
        Log::Printf(
            "SS0 direct runtime: S0 subtitle source committed 80024C84 vtext desc=%08X off=0x%X entries=%u firstFrame30=%u lastFrame30=%u legacySqevs=0",
            kScene0Movie0VtextDesc801C6BF8,
            static_cast<unsigned>(track.descOffset),
            static_cast<unsigned>(track.entryCount),
            static_cast<unsigned>(first.frame30),
            static_cast<unsigned>(last.frame30));
        return true;
    }

    s_ss0Direct.scene0SubtitleDataLoaded = true;
    s_ss0Direct.scene0SubtitleDataAvailable = false;
    PrStage1VTextDirectResetSub80024C84(
        s_ss0Direct.scene0SubtitleVtextRuntime);
    Log::Printf(
        "SS0 direct runtime: S0 subtitle source blocked COMOD0 801C6BF8 vtext unavailable bytes=%u legacySqevs=0",
        static_cast<unsigned>(comodBytes.size()));
    return false;
}

static PrStage1VTextDirectDescInput
SS0DirectBuildScene0SubtitleDescInput() {
    PrStage1VTextDirectDescInput out{};
    const auto& desc = s_ss0Direct.scene0Movie0Vtext.subtitleDesc;
    out.descAddr = desc.descAddr;
    out.textTableAddrs = desc.textTableAddrs;
    out.entriesAddr = desc.entriesAddr;
    out.entryCount = desc.entryCount;
    const auto& lineMeta = s_ss0Direct.scene0Movie0Vtext.subtitleLineMeta;
    out.lineMeta = lineMeta.empty() ? nullptr : lineMeta.data();
    out.lineMetaCount = static_cast<uint32_t>(lineMeta.size());
    return out;
}

static int SS0DirectFn0(PrGameContext& ctx) {
    Log::Printf("SS0 direct runtime: Fn0 init");
    s_ss0Direct.resetGraph800446A0 =
        PrSceneBootstrapDirect::PsxCall800446A0_ResetGraphRuntime(0);
    if (!s_ss0Direct.resetGraph800446A0.accepted ||
        !s_ss0Direct.resetGraph800446A0.softwareStateCommitted ||
        !s_ss0Direct.resetGraph800446A0.environmentZeroed ||
        s_ss0Direct.resetGraph800446A0.environmentBytes !=
            PrSceneBootstrapDirect::kResetGraphEnvironmentBytes800446A0 ||
        !s_ss0Direct.resetGraph800446A0.resetCallbackCalled ||
        !s_ss0Direct.resetGraph800446A0.gpuControlWordSubmitted ||
        s_ss0Direct.resetGraph800446A0.gpuCommandListAddress !=
            (PrSceneBootstrapDirect::
                 kResetGraphGpuCommandListAddress8005D6EC &
             0x00FFFFFFu) ||
        !s_ss0Direct.resetGraph800446A0.displayInitCalled ||
        s_ss0Direct.resetGraph800446A0.displayInitArg != 0 ||
        !s_ss0Direct.resetGraph800446A0.mainEnvironmentCleared ||
        s_ss0Direct.resetGraph800446A0.mainEnvironmentBytes !=
            PrSceneBootstrapDirect::
                kResetGraphMainEnvironmentBytes800446A0 ||
        !s_ss0Direct.resetGraph800446A0.tailEnvironmentCleared ||
        s_ss0Direct.resetGraph800446A0.tailEnvironmentBytes !=
            PrSceneBootstrapDirect::
                kResetGraphTailEnvironmentBytes800446A0 ||
        s_ss0Direct.resetGraph800446A0.hardwareResetHalAuthority ||
        s_ss0Direct.resetGraph800446A0.hostProjection ||
        s_ss0Direct.resetGraph800446A0.replayValueAuthority ||
        s_ss0Direct.resetGraph800446A0.oldWinS0Authority ||
        s_ss0Direct.resetGraph800446A0.stage2PlusAuthority ||
        s_ss0Direct.resetGraph800446A0.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: Fn0 800446A0 ResetGraph prefix blocked accepted=%d committed=%d env=%d reset=%d gpu=%d display=%d",
            s_ss0Direct.resetGraph800446A0.accepted ? 1 : 0,
            s_ss0Direct.resetGraph800446A0.softwareStateCommitted ? 1 : 0,
            s_ss0Direct.resetGraph800446A0.environmentZeroed ? 1 : 0,
            s_ss0Direct.resetGraph800446A0.resetCallbackCalled ? 1 : 0,
            s_ss0Direct.resetGraph800446A0.gpuControlWordSubmitted ? 1 : 0,
            s_ss0Direct.resetGraph800446A0.displayInitCalled ? 1 : 0);
        return 0;
    }
    Log::Printf(
        "SS0 direct runtime: Fn0 800446A0 ResetGraph prefix committed mode=%u env=%08X/%u gpuList24=%06X displayArg=%d displayReturnKnown=%d mainEnv=%08X/%u tailEnv=%08X/%u; hardwareResetHalAuthority=0 exactPsxHalParity=0",
        s_ss0Direct.resetGraph800446A0.modeLow3,
        s_ss0Direct.resetGraph800446A0.environmentAddress,
        s_ss0Direct.resetGraph800446A0.environmentBytes,
        s_ss0Direct.resetGraph800446A0.gpuCommandListAddress,
        s_ss0Direct.resetGraph800446A0.displayInitArg,
        s_ss0Direct.resetGraph800446A0.displayInitReturnKnown ? 1 : 0,
        s_ss0Direct.resetGraph800446A0.mainEnvironmentAddress,
        s_ss0Direct.resetGraph800446A0.mainEnvironmentBytes,
        s_ss0Direct.resetGraph800446A0.tailEnvironmentAddress,
        s_ss0Direct.resetGraph800446A0.tailEnvironmentBytes);
    s_ss0Direct.padInit800354C0 =
        PrPsxPadDirect::PsxCall800354C0_InitPadRuntime(0);
    if (!s_ss0Direct.padInit800354C0.accepted ||
        !s_ss0Direct.padInit800354C0.softwareStateCommitted ||
        !s_ss0Direct.padInit800354C0.resetCallbackCalled ||
        !s_ss0Direct.padInit800354C0.padInit2Called ||
        s_ss0Direct.padInit800354C0.padInit2Protocol !=
            PrPsxPadDirect::kPadInit2Protocol800354C0 ||
        s_ss0Direct.padInit800354C0.padInit2StatusAddress !=
            PrPsxPadDirect::kPadStatusGlobal800882F0 ||
        !s_ss0Direct.padInit800354C0.changeClearPadCalled ||
        s_ss0Direct.padInit800354C0.changeClearPadArg != 0 ||
        s_ss0Direct.padInit800354C0.hardwarePadHalAuthority ||
        s_ss0Direct.padInit800354C0.hostProjection ||
        s_ss0Direct.padInit800354C0.replayValueAuthority ||
        s_ss0Direct.padInit800354C0.oldWinS0Authority ||
        s_ss0Direct.padInit800354C0.stage2PlusAuthority ||
        s_ss0Direct.padInit800354C0.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: Fn0 800354C0 PAD init blocked accepted=%d committed=%d reset=%d padInit2=%d changeClear=%d",
            s_ss0Direct.padInit800354C0.accepted ? 1 : 0,
            s_ss0Direct.padInit800354C0.softwareStateCommitted ? 1 : 0,
            s_ss0Direct.padInit800354C0.resetCallbackCalled ? 1 : 0,
            s_ss0Direct.padInit800354C0.padInit2Called ? 1 : 0,
            s_ss0Direct.padInit800354C0.changeClearPadCalled ? 1 : 0);
        return 0;
    }
    Log::Printf(
        "SS0 direct runtime: Fn0 800354C0 PAD init committed modeAddr=%08X mode=%d statusAddr=%08X status=%d protocol=%08X; hardwarePadHalAuthority=0 exactPsxHalParity=0",
        s_ss0Direct.padInit800354C0.modeGlobalAddress,
        s_ss0Direct.padInit800354C0.modeGlobalValue,
        s_ss0Direct.padInit800354C0.statusGlobalAddress,
        s_ss0Direct.padInit800354C0.statusGlobalValue,
        s_ss0Direct.padInit800354C0.padInit2Protocol);
    s_ss0Direct.startupWorkList8001E6D0 =
        PrSceneBootWorkListDirect::
            PsxCall8001E6D0_InitWorkListsAndDrawBuffers();
    const auto& startupWorkList = s_ss0Direct.startupWorkList8001E6D0;
    if (!startupWorkList.known || !startupWorkList.workListsKnown ||
        startupWorkList.workLists.size() != 2u ||
        !startupWorkList.workLists[0].known ||
        !startupWorkList.workLists[1].known ||
        startupWorkList.workLists[0].descAddr !=
            PrSceneBootWorkListDirect::kWorkListBase8001E6D0 ||
        startupWorkList.workLists[1].descAddr !=
            PrSceneBootWorkListDirect::kWorkListBase8001E6D0 + 0x14u ||
        startupWorkList.workLists[0].order_00 != 4u ||
        startupWorkList.workLists[1].order_00 != 4u ||
        startupWorkList.workLists[0].headAddr_04 !=
            PrSceneBootWorkListDirect::kWorkListHeadBase8001E6D0 ||
        startupWorkList.workLists[1].headAddr_04 !=
            PrSceneBootWorkListDirect::kWorkListHeadBase8001E6D0 + 0x40u ||
        startupWorkList.workLists[0].lastAddr_08 != 0u ||
        startupWorkList.workLists[1].lastAddr_08 != 0u ||
        !startupWorkList.drawBuffers.known ||
        !startupWorkList.drawBuffers.arg0Known ||
        !startupWorkList.drawBuffers.arg1Known ||
        startupWorkList.drawBuffers.arg0 !=
            PrSceneDrawBufferDirect::kDrawBufferBase80080CF8 ||
        startupWorkList.drawBuffers.arg1 !=
            PrSceneDrawBufferDirect::kDrawBufferBase80083FC0 ||
        startupWorkList.drawBuffers.hostSideEffects) {
        Log::Printf(
            "SS0 direct runtime: Fn0 8001E6D0 independent work-list/draw-buffer bootstrap blocked known=%d lists=%d drawBuffers=%d",
            startupWorkList.known ? 1 : 0,
            startupWorkList.workListsKnown ? 1 : 0,
            startupWorkList.drawBuffers.known ? 1 : 0);
        return 0;
    }
    Log::Printf(
        "SS0 direct runtime: Fn0 8001E6D0 independent work-list bootstrap committed desc=%08X/%08X order=%u heads=%08X/%08X drawBuffers=%08X/%08X; mainPageOrder14Preserved=1 hostSideEffects=0",
        startupWorkList.workLists[0].descAddr,
        startupWorkList.workLists[1].descAddr,
        startupWorkList.workLists[0].order_00,
        startupWorkList.workLists[0].headAddr_04,
        startupWorkList.workLists[1].headAddr_04,
        startupWorkList.drawBuffers.arg0,
        startupWorkList.drawBuffers.arg1);
    s_ss0Direct.bootstrapGraphRuntime8001C470 =
        PrSceneBootstrapDirect::CommitBootstrapGraphRuntime8001C470(
            s_ss0Direct.resetGraph800446A0,
            s_ss0Direct.padInit800354C0,
            s_ss0Direct.startupWorkList8001E6D0,
            PrSS0TitleTmdBackend::GetTitleGraphState());
    const auto& bootstrapGraph =
        s_ss0Direct.bootstrapGraphRuntime8001C470;
    if (!bootstrapGraph.known || !bootstrapGraph.graphStateExact ||
        !bootstrapGraph.resetGraphExact || !bootstrapGraph.padInitExact ||
        !bootstrapGraph.workListExact ||
        !bootstrapGraph.initialClearOwnerBound ||
        !bootstrapGraph.softwareStateCommitted ||
        !bootstrapGraph.physicalHalGap ||
        bootstrapGraph.exactPsxHalParity ||
        bootstrapGraph.hostProjection ||
        bootstrapGraph.replayValueAuthority ||
        bootstrapGraph.oldWinS0Authority ||
        bootstrapGraph.stage2PlusAuthority ||
        bootstrapGraph.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: Fn0 8001ED94/8001C470 coupled bootstrap blocked known=%d graph=%d reset=%d pad=%d workList=%d clearOwner=%d committed=%d",
            bootstrapGraph.known ? 1 : 0,
            bootstrapGraph.graphStateExact ? 1 : 0,
            bootstrapGraph.resetGraphExact ? 1 : 0,
            bootstrapGraph.padInitExact ? 1 : 0,
            bootstrapGraph.workListExact ? 1 : 0,
            bootstrapGraph.initialClearOwnerBound ? 1 : 0,
            bootstrapGraph.softwareStateCommitted ? 1 : 0);
        return 0;
    }
    Log::Printf(
        "SS0 direct runtime: Fn0 8001ED94->8001C470->8001E6D0 coupled bootstrap committed graph=1 reset=1 pad=1 workList=1 clearOwner=8001B1B0 sourceOrder=8001C1E8>800446A0>800354C0>8003FB9C>80040AE4>80040B84>80040C74>8001B1B0>8001E6D0 physicalHalGap=1 exactPsxHalParity=0");
    const auto cardStorage =
        PrSS0CardImageStorageDirect::
            BindAndLoadPrimaryCardImage8007A318();
    Log::Printf(
        "SS0 direct runtime: Fn0 direct card storage backend=%d preserved=%d found=%d read=%d validated=%d imported=%d block=%d",
        cardStorage.backendBound ? 1 : 0,
        cardStorage.existingSinkPreserved ? 1 : 0,
        cardStorage.imageFileFound ? 1 : 0,
        cardStorage.imageRead ? 1 : 0,
        cardStorage.imageValidated ? 1 : 0,
        cardStorage.sinkImported ? 1 : 0,
        cardStorage.blockIndex);
    const auto scene0GlobalBindings =
        PrSS0Scene0GlobalBindingsDirect::BuildState801C5B14();
    if (!PrSS0Scene0GlobalBindingsDirect::IsExactState801C5B14(
            scene0GlobalBindings)) {
        Log::Printf(
            "SS0 direct runtime: Fn0 801C5B14 global binding blocked");
        return 0;
    }
    s_ss0Direct.scene0GlobalBindings = scene0GlobalBindings;
    Log::Printf(
        "SS0 direct runtime: Fn0 801C5B14 global binding committed C0=%08X C4=%08X C8=%u hostProjection=%u",
        scene0GlobalBindings.slot800943C0,
        scene0GlobalBindings.slot800943C4,
        static_cast<unsigned>(scene0GlobalBindings.slot800943C8),
        scene0GlobalBindings.hostProjection ? 1u : 0u);
    s_ss0Direct.scene0Init801C4260 = {};
    s_ss0Direct.scene0SubtitleDataLoaded = false;
    s_ss0Direct.scene0SubtitleDataAvailable = false;
    s_ss0Direct.scene0SubtitleRenderLogged = false;
    s_ss0Direct.scene0SubtitleEventCount = 0u;
    s_ss0Direct.scene0Movie0Vtext = {};
    PrStage1VTextDirectResetSub80024C84(
        s_ss0Direct.scene0SubtitleVtextRuntime);
    s_ss0Direct.scene0ResourceIngress801C4780 = {};
    s_ss0Direct.startupCommonIntLoad80016B84 = {};
    s_ss0Direct.startupZCompoIntLoad80016B84 = {};
    s_ss0Direct.scene0IntLoad8001AC18 = {};
    s_ss0Direct.scene0IntSideEffect8001A8F0 = {};
    s_ss0Direct.scene0IntSideEffectGateCommitted = false;
    s_ss0Direct.scene0IntSpu8001A8F0 = {};
    s_ss0Direct.scene0IntSpuGateCommitted = false;
    s_ss0Direct.scene0IntGpu8001AE7C = {};
    s_ss0Direct.scene0IntGpuGateCommitted = false;
    s_ss0Direct.scene0IntRenderer8001AE7C = {};
    s_ss0Direct.scene0IntRendererGateCommitted = false;
    s_ss0Direct.practiceYCompoIntLoad80015618 = {};
    s_ss0Direct.practiceYCompoIntSideEffect8001A8F0 = {};
    s_ss0Direct.practiceYCompoLoaderMemoryGateCommitted = false;
    s_ss0Direct.practiceYCompoPadStartComSource = {};
    s_ss0Direct.practiceYCompoIntSpu8001A8F0 = {};
    s_ss0Direct.practiceYCompoIntGpu8001AE7C = {};
    s_ss0Direct.practiceYCompoIntRenderer8001AE7C = {};
    s_ss0Direct.practiceYCompoResourcesCommitted = false;
    SS0DirectResetEventTextRuntime(ctx);
    SS0DirectResetToTitle(ctx);
    s_ss0Direct.phase = SS0DirectPhase::OpeningMovie0InitialTransition;
    return 0;
}

static bool SS0DirectResolveTitleAttractTimeoutPolicy801C48C0();
static bool SS0DirectRestartTitleAttractTimer801C4894();

static constexpr uint32_t kSs0LoadingMinimumLogicTicks = 30u;
static void SS0DirectClearLoadingMinimumHold();

static void SS0DirectBeginLoadingScreen(
    SS0DirectLoadingHoldKind kind,
    uint32_t startFrame) {
    if (kind == SS0DirectLoadingHoldKind::None) {
        return;
    }
    SS0DirectClearLoadingMinimumHold();
    s_ss0Direct.loadingScreenKind = kind;
    s_ss0Direct.loadingScreenStartFrame = startFrame;
    // Only the resource-load boundaries represented by 80015590/80015660
    // install 80015408->8001537C.  The mode-2 closing owners start the
    // callback only once the transition has completed and the page is
    // actually visible; starting them here would advance the mask before the
    // Loading page can be seen.
    if (kind == SS0DirectLoadingHoldKind::SceneHandoff ||
        kind == SS0DirectLoadingHoldKind::MainMenuReload) {
        (void)SS0DirectBeginLoadingPattern8001EF40(kind);
    }
    Log::Printf(
        "SS0 direct runtime: loading screen kind=%u started frame=%u minTicks=%u",
        static_cast<unsigned>(kind),
        startFrame,
        kSs0LoadingMinimumLogicTicks);
}

static void SS0DirectArmLoadingMinimumHold(
    SS0DirectLoadingHoldKind kind,
    uint32_t startFrame,
    uint32_t untilFrame) {
    if (kind == SS0DirectLoadingHoldKind::None) {
        return;
    }
    s_ss0Direct.loadingHoldKind = kind;
    s_ss0Direct.loadingHoldStartFrame = startFrame;
    s_ss0Direct.loadingHoldUntilFrame = untilFrame;
    s_ss0Direct.loadingHoldLogged = false;
}

static bool SS0DirectTickLoadingPatternForHostLogic8001EF40() {
    // The translated Scene0 dispatcher already supplies the observable
    // 8001537C callback cadence.  Advancing twice here makes the role-grid
    // mask visibly run at double speed, so consume exactly one callback body
    // per direct-runtime tick.
    const auto frame =
        PrSS0TransitionDirect::TickLoadingPatternRuntime8001EF40(
            s_ss0Direct.loadingPatternRuntime8001EF40);
    if (!frame.known) {
        return false;
    }
    s_ss0Direct.loadingPatternFrame8001EF40 = frame;
    // The second operation in 8001537C, after 8001EA74(1,0), services
    // the same SPU driver as the surrounding scene.  No separate song or
    // host-timed restart belongs to the Loading callback.
    PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
    return true;
}

static bool SS0DirectBeginLoadingPattern8001EF40(
    SS0DirectLoadingHoldKind kind) {
    if (kind == SS0DirectLoadingHoldKind::None) {
        Log::Printf(
            "SS0 direct runtime: 8001EF40 Loading pattern start blocked kind=none");
        return false;
    }
    std::array<uint8_t,
               PrSS0TransitionDirect::kLoadingPatternGridCells8001EF40>
        blankLoadingGrid{};
    if (!PrSS0TransitionDirect::BeginLoadingPatternRuntimeAfter8001FFD4(
            s_ss0Direct.loadingPatternRuntime8001EF40,
            s_ss0Direct.loadingPatternMode80015408,
            blankLoadingGrid.data(),
            PrSS0TransitionDirect::kLoadingPatternGridCells8001EF40)) {
        Log::Printf(
            "SS0 direct runtime: 80015408(%d)->8001537C->8001EF40 Loading pattern source rejected kind=%u",
            static_cast<int>(s_ss0Direct.loadingPatternMode80015408),
            static_cast<unsigned>(kind));
        return false;
    }
    // The source calls 8001537C on the same VSync cycle that first exposes
    // the callback-owned Loading page.  Publish that first zero-grid/one-bit
    // frame before the host renderer runs, so the boundary cannot show an
    // uninitialized white/black frame.
    if (!SS0DirectTickLoadingPatternForHostLogic8001EF40()) {
        PrSS0TransitionDirect::StopLoadingPatternRuntime8001EF40(
            s_ss0Direct.loadingPatternRuntime8001EF40);
        return false;
    }
    s_ss0Direct.loadingPatternRenderBlockedLogged8001EF40 = false;
    s_ss0Direct.loadingPatternSubmittedHighlightCount8001EF40 = 0u;
    s_ss0Direct.loadingPatternSubmittedGridFnv1a8001EF40 = 0u;
    s_ss0Direct.loadingPatternSubmittedFrame8001EF40 = 0u;
    Log::Printf(
        "SS0 direct runtime: Loading 80015408(%d)->8001537C->8001EA74(1,0)->8001EF40 started kind=%u style=%u color=%08X highlights=%u bit=%u word=%u counter=%u mutation=%u initialGrid=zeros",
        static_cast<int>(s_ss0Direct.loadingPatternMode80015408),
        static_cast<unsigned>(kind),
        static_cast<unsigned>(
            s_ss0Direct.loadingPatternFrame8001EF40.style),
        s_ss0Direct.loadingPatternFrame8001EF40.boxFillAttr8001B6C4,
        s_ss0Direct.loadingPatternFrame8001EF40.highlightCount,
        s_ss0Direct.loadingPatternFrame8001EF40.bitCursorGp49,
        s_ss0Direct.loadingPatternFrame8001EF40.wordCursorGp50,
        s_ss0Direct.loadingPatternFrame8001EF40.frameCounterGp51,
        s_ss0Direct.loadingPatternFrame8001EF40.mutationSerial);
    return true;
}

static bool SS0DirectLoadingMinimumHoldActive(
    SS0DirectLoadingHoldKind kind) {
    return s_ss0Direct.loadingHoldKind == kind &&
           kind != SS0DirectLoadingHoldKind::None;
}

static bool SS0DirectLoadingMinimumHoldReady(
    SS0DirectLoadingHoldKind kind,
    uint32_t frame) {
    if (!SS0DirectLoadingMinimumHoldActive(kind)) {
        return true;
    }
    if (frame < s_ss0Direct.loadingHoldUntilFrame) {
        // 8001EF40 is already being advanced by the translated 8001537C
        // callback in Fn2.  This floor is only a release barrier; do not tick
        // the pattern a second time from the hold path.
        if (!s_ss0Direct.loadingHoldLogged) {
            s_ss0Direct.loadingHoldLogged = true;
            Log::Printf(
                "SS0 direct runtime: loading hold kind=%u startFrame=%u untilFrame=%u remaining=%u",
                static_cast<unsigned>(kind),
                s_ss0Direct.loadingHoldStartFrame,
                s_ss0Direct.loadingHoldUntilFrame,
                s_ss0Direct.loadingHoldUntilFrame - frame);
        }
        return false;
    }
    Log::Printf(
        "SS0 direct runtime: loading hold kind=%u released frame=%u elapsed=%u minTicks=%u 8001EF40Mutation=%u mask=%016llX",
        static_cast<unsigned>(kind),
        frame,
        frame - s_ss0Direct.loadingHoldStartFrame,
        kSs0LoadingMinimumLogicTicks,
        s_ss0Direct.loadingPatternFrame8001EF40.mutationSerial,
        static_cast<unsigned long long>(
            s_ss0Direct.loadingPatternFrame8001EF40.liveGridFnv1a));
    PrSS0TransitionDirect::StopLoadingPatternRuntime8001EF40(
        s_ss0Direct.loadingPatternRuntime8001EF40);
    s_ss0Direct.loadingPatternFrame8001EF40 = {};
    s_ss0Direct.loadingPatternRenderBlockedLogged8001EF40 = false;
    s_ss0Direct.loadingPatternSubmittedHighlightCount8001EF40 = 0u;
    s_ss0Direct.loadingPatternSubmittedGridFnv1a8001EF40 = 0u;
    s_ss0Direct.loadingPatternSubmittedFrame8001EF40 = 0u;
    s_ss0Direct.loadingHoldKind = SS0DirectLoadingHoldKind::None;
    s_ss0Direct.loadingHoldStartFrame = 0u;
    s_ss0Direct.loadingHoldUntilFrame = 0u;
    s_ss0Direct.loadingHoldLogged = false;
    s_ss0Direct.loadingScreenKind = SS0DirectLoadingHoldKind::None;
    s_ss0Direct.loadingScreenStartFrame = 0u;
    return true;
}

// Called only after the pseudo-C transition runtime reports completion.  The
// translated body remains authoritative.  The 8001EF40 callback-owned grid
// was started at the 80015408 boundary; this function only decides whether
// the host-side Loading floor must delay release for observability.
static bool SS0DirectNeedsCompletedLoadingPageHold(
    SS0DirectLoadingHoldKind kind) {
    // Only the mode-2 closing owners keep a completed-page dwell.  Reveal
    // owners keep their translated cadence and do not inherit the extra
    // post-completion floor.
    return kind == SS0DirectLoadingHoldKind::OpeningMovie0Initial ||
           kind == SS0DirectLoadingHoldKind::TitleExit;
}

static bool SS0DirectFinishLoadingScreenOrHold(
    SS0DirectLoadingHoldKind kind,
    uint32_t frame) {
    if (SS0DirectLoadingMinimumHoldActive(kind)) {
        return SS0DirectLoadingMinimumHoldReady(kind, frame);
    }
    if (s_ss0Direct.loadingScreenKind != kind) {
        return true;
    }
    const uint32_t transitionStartFrame =
        s_ss0Direct.loadingScreenStartFrame;
    const uint32_t transitionElapsed =
        frame >= transitionStartFrame ? frame - transitionStartFrame : 0u;
    if (SS0DirectNeedsCompletedLoadingPageHold(kind)) {
        // The closing owners should only start the visible Loading callback
        // once the transition has completed.  Starting the pattern at
        // transition entry advances the mask before the page is actually on
        // screen.
        // Boot's 801C4DC4 close is followed by 800201AC, not 80015590 or
        // 80015660. Keep its requested dwell, but do not invent a text
        // callback there (the source never installs 80015408 on this path).
        if (kind == SS0DirectLoadingHoldKind::TitleExit &&
            !s_ss0Direct.loadingPatternRuntime8001EF40.active) {
            (void)SS0DirectBeginLoadingPattern8001EF40(kind);
        }
        const uint32_t untilFrame =
            frame + kSs0LoadingMinimumLogicTicks;
        SS0DirectArmLoadingMinimumHold(kind, frame, untilFrame);
        Log::Printf(
            "SS0 direct runtime: loading completed page kind=%u release-floor hold started frame=%u untilFrame=%u transitionStartFrame=%u transitionElapsed=%u minTicks=%u completedPageHold=1 finalFrameRetained=1",
            static_cast<unsigned>(kind),
            frame,
            untilFrame,
            transitionStartFrame,
            transitionElapsed,
            kSs0LoadingMinimumLogicTicks);
        return false;
    }
    // Reveal owners keep their translated cadence and release immediately
    // after the pseudo-C transition completes.
    Log::Printf(
        "SS0 direct runtime: loading screen kind=%u naturally completed frame=%u elapsed=%u minTicks=%u completedPageHold=0",
        static_cast<unsigned>(kind),
        frame,
        transitionElapsed,
        kSs0LoadingMinimumLogicTicks);
    PrSS0TransitionDirect::StopLoadingPatternRuntime8001EF40(
        s_ss0Direct.loadingPatternRuntime8001EF40);
    s_ss0Direct.loadingPatternFrame8001EF40 = {};
    s_ss0Direct.loadingPatternRenderBlockedLogged8001EF40 = false;
    s_ss0Direct.loadingPatternSubmittedHighlightCount8001EF40 = 0u;
    s_ss0Direct.loadingPatternSubmittedGridFnv1a8001EF40 = 0u;
    s_ss0Direct.loadingPatternSubmittedFrame8001EF40 = 0u;
    s_ss0Direct.loadingScreenKind = SS0DirectLoadingHoldKind::None;
    s_ss0Direct.loadingScreenStartFrame = 0u;
    return true;
}

static void SS0DirectClearLoadingMinimumHold() {
    PrSS0TransitionDirect::StopLoadingPatternRuntime8001EF40(
        s_ss0Direct.loadingPatternRuntime8001EF40);
    s_ss0Direct.loadingPatternFrame8001EF40 = {};
    s_ss0Direct.loadingPatternRenderBlockedLogged8001EF40 = false;
    s_ss0Direct.loadingPatternSubmittedHighlightCount8001EF40 = 0u;
    s_ss0Direct.loadingPatternSubmittedGridFnv1a8001EF40 = 0u;
    s_ss0Direct.loadingPatternSubmittedFrame8001EF40 = 0u;
    s_ss0Direct.loadingHoldKind = SS0DirectLoadingHoldKind::None;
    s_ss0Direct.loadingScreenKind = SS0DirectLoadingHoldKind::None;
    s_ss0Direct.loadingScreenStartFrame = 0u;
    s_ss0Direct.loadingHoldStartFrame = 0u;
    s_ss0Direct.loadingHoldUntilFrame = 0u;
    s_ss0Direct.loadingHoldLogged = false;
}

static bool SS0DirectStartScene0OuterEntry801C4DC4(PrGameContext& ctx) {
    PrSS0TransitionDirect::Scene0OuterEntryInput801C4DC4 input{};
    input.contextKnown = true;
    input.contextAddress = PrSS0TransitionDirect::kScene0WorkAddress;
    input.word800916D2Known = s_ss0Direct.word800916D2Known;
    input.word800916D2 = s_ss0Direct.word800916D2;
    const auto transaction =
        PrSS0TransitionDirect::BuildScene0OuterEntryTransaction801C4DC4(input);
    if (!transaction.accepted) {
        Log::Printf(
            "SS0 direct runtime: 801C4DC4 first-entry transaction blocked status=%u d2Known=%d d2=%u",
            static_cast<unsigned>(transaction.status),
            s_ss0Direct.word800916D2Known ? 1 : 0,
            static_cast<unsigned>(s_ss0Direct.word800916D2));
        return false;
    }

    if (transaction.initialSlowTransitionRequired80020110) {
        PrSS0TransitionDirect::SlowTransitionRuntime80020110 slowCandidate{};
        if (!SS0DirectPreflightSlowTransitionTextures80020110(
                ctx,
                transaction.slowCtxAddress80020110,
                transaction.slowMode80020110,
                transaction.slowPreFfd4Arg80020110,
                transaction.slowPostFfd4Arg80020110) ||
            !PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
                slowCandidate,
                transaction.slowCtxAddress80020110,
                transaction.slowMode80020110,
                transaction.slowPreFfd4Arg80020110,
                transaction.slowPostFfd4Arg80020110)) {
            Log::Printf(
                "SS0 direct runtime: 801C4DC4 initial 80020110 mode=2 all-clear blocked");
            return false;
        }

        s_ss0Direct.openingMovie0InitialTransition = slowCandidate;
        // 80020110 enters its do/while body immediately.  Publish the first
        // mode-2 grid visual on the same frame the runtime is started instead
        // of exposing one host frame with an empty/black Loading page.
        s_ss0Direct.openingMovie0InitialTransitionVisual =
            PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
                transaction.slowCtxAddress80020110,
                transaction.slowMode80020110,
                transaction.slowPreFfd4Arg80020110,
                transaction.slowPostFfd4Arg80020110,
                false,
                0u);
        s_ss0Direct.openingMovie0InitialTransitionPresentBlockedLogged = false;
        if (transaction.publishWord800916D2) {
            s_ss0Direct.word800916D2Known = true;
            s_ss0Direct.word800916D2 = transaction.nextWord800916D2;
        }
        s_ss0Direct.phase = SS0DirectPhase::OpeningMovie0InitialTransition;
        SS0DirectBeginLoadingScreen(
            SS0DirectLoadingHoldKind::OpeningMovie0Initial, ctx.frame);
        Log::Printf(
            "SS0 direct runtime: 801C4DC4 first-entry D2=%u -> 80020110 mode=2 started",
            static_cast<unsigned>(s_ss0Direct.word800916D2));
        return true;
    }

    PrSS0TransitionDirect::FastTransitionRuntime800201AC fastCandidate{};
    if (!PrSS0TransitionDirect::BeginFastTransitionRuntime800201AC(
            fastCandidate,
            transaction.fastCtxAddress800201AC,
            transaction.fastMode800201AC,
            transaction.fastPreFfd4Arg800201AC,
            transaction.fastPostFfd4Arg800201AC,
            s_ss0Direct.optionsWord800916DC != 0)) {
        Log::Printf(
            "SS0 direct runtime: 801C4DC4 800201AC mode=6 start blocked d2=%u",
            static_cast<unsigned>(s_ss0Direct.word800916D2));
        return false;
    }
    s_ss0Direct.openingMovie0Transition = fastCandidate;
    // Mode 6 enters 80020248(0) synchronously; its first operation is
    // 800271E4(1), before the first 8001F230 page and the VBlank wait.
    SS0DirectApplyFastTransitionInitialCue800271E4(
        s_ss0Direct.openingMovie0Transition);
    // 800201AC presents its first visual frame immediately after the
    // dispatcher starts the runtime.  Seed iteration 0 here instead of
    // leaving one host frame with an Unknown/black visual while the first
    // VBlank tick is still pending; this preserves the original frame
    // cadence and keeps the character-grid UI present from the first frame.
    s_ss0Direct.openingMovie0TransitionVisual =
        PrSS0TransitionDirect::ResolveFastTransitionVisualFrame800201AC(
            transaction.fastMode800201AC,
            s_ss0Direct.optionsWord800916DC != 0,
            false,
            0u);
    s_ss0Direct.openingMovie0TransitionPresentBlockedLogged = false;
    s_ss0Direct.phase = SS0DirectPhase::OpeningMovie0PreTransition;
    SS0DirectBeginLoadingScreen(
        SS0DirectLoadingHoldKind::OpeningMovie0Pre, ctx.frame);
    Log::Printf(
        "SS0 direct runtime: 801C4DC4 -> 800201AC mode=6 started d2=%u",
        static_cast<unsigned>(s_ss0Direct.word800916D2));
    return true;
}

static bool SS0DirectBuildTitleEntryPrefixTransaction801C4894(
    PrSS0TitleEntryPrefixDirect::Transaction801C4894& out) {
    PrSS0TitleEntryPrefixDirect::Source801C4894 source{};
    source.contextKnown = true;
    source.contextAddress =
        PrSS0TitleEntryPrefixDirect::kContextAddress801C3640;
    source.scene0TablePointerKnown = true;
    source.scene0TablePointer =
        PrSS0TitleEntryPrefixDirect::kScene0TablePointer801C6DA4;
    source.word800916DCKnown = true;
    source.word800916DC =
        static_cast<uint16_t>(s_ss0Direct.optionsWord800916DC);
    return PrSS0TitleEntryPrefixDirect::BuildTransaction801C4894(source, out);
}

static bool SS0DirectPrepareTitleEventProjectionReset80024E98(
    PrSS0Scene0SharedEventPredispatchDirect::StaticDispatchState80024FD0&
        resetDispatch) {
    resetDispatch = s_ss0Direct.titleSharedEventDispatch;
    return PrSS0Scene0SharedEventPredispatchDirect::
        ApplyStaticDispatchReset80024E98(resetDispatch);
}

static void SS0DirectCommitTitleEventProjectionReset80024E98(
    const PrSS0Scene0SharedEventPredispatchDirect::
        StaticDispatchState80024FD0& resetDispatch) {
    PrSS0TitleHudEventsDirect::ResetTitleSharedEventBucketState80024E98(
        s_ss0Direct.titleSharedEventBucket);
    PrSS0Scene0SharedEventPredispatchDirect::ResetContextState80024E98(
        s_ss0Direct.titleSharedEventPredispatchContext);
    s_ss0Direct.titleSharedEventPredispatchLogged = false;
    s_ss0Direct.titleSharedEventDispatch = resetDispatch;
    s_ss0Direct.titleSharedEventDispatchLogged = false;
    PrSS0TitleHudEventsDirect::ResetTitleEventRuntimeState80024E98(
        s_ss0Direct.titleEventRuntimeState);
    s_ss0Direct.titleSelectorResourceUpdate = {};
    s_ss0Direct.titleEventPending = false;
    s_ss0Direct.titlePendingEvent = {};
}

static bool SS0DirectTryCommitScene0Init801C4260(
    PrGameContext& ctx,
    bool forceResourceReload) {
    if (!forceResourceReload &&
        s_ss0Direct.bootStartupAudioBindingsCommitted80016C8C &&
        PrSS0ResourceAudioDirect::IsExactStartupAudioBindings80016C8C(
            s_ss0Direct.bootStartupAudioBindings80016C8C) &&
        s_ss0Direct.bootStartupDiscIntPrepared80016B84 &&
        s_ss0Direct.bootStartupPreparedBeforeFirstLogo80016B84 &&
        PrSS0Scene0ResourceIngressDirect::
            IsExactAcceptedStartupPreloadTransaction80016B84(
                s_ss0Direct.bootStartupPreloadIngress80016B84) &&
        PrSS0Scene0GlobalBindingsDirect::IsExactInitTransaction801C4260(
            s_ss0Direct.scene0Init801C4260) &&
        PrSS0Scene0ResourceIngressDirect::
            IsExactAcceptedTransaction801C4780(
                s_ss0Direct.scene0ResourceIngress801C4780) &&
        PrSS0Scene0IntLoadDirect::
            IsExactAcceptedStartupCommonTransaction80016B84(
                s_ss0Direct.startupCommonIntLoad80016B84) &&
        PrSS0Scene0IntLoadDirect::
            IsExactAcceptedZCompoTransaction80015590(
                s_ss0Direct.startupZCompoIntLoad80016B84) &&
        s_ss0Direct.bootStartupVabCloseCommitted80027120 &&
        s_ss0Direct.bootStartupVabClose80027120.committed &&
        s_ss0Direct.bootStartupVabClose80027120.function == 0x80027120u &&
        s_ss0Direct.bootStartupVabClose80027120.hostProjectionCleared &&
        !s_ss0Direct.bootStartupVabClose80027120.psxSpuRegisterAuthority &&
        !s_ss0Direct.bootStartupVabClose80027120.psxInterruptTimingAuthority &&
        PrSS0Scene0IntLoadDirect::IsExactAcceptedTransaction8001AC18(
            s_ss0Direct.scene0IntLoad8001AC18) &&
        s_ss0Direct.scene0IntSideEffectGateCommitted &&
        s_ss0Direct.scene0IntSpuGateCommitted &&
        s_ss0Direct.scene0IntGpuGateCommitted &&
        s_ss0Direct.scene0IntRendererGateCommitted) {
        (void)SS0DirectLoadScene0SubtitleData(ctx);
        return true;
    }

    const bool globalBindingsReady =
        PrSS0Scene0GlobalBindingsDirect::IsExactState801C5B14(
            s_ss0Direct.scene0GlobalBindings);
    const auto resourceState = PrSS0TitleTmdBackend::GetResourceState();
    const bool resetSourcesReady =
        ctx.mainSceneLoaderMemoryDirect != nullptr;
    const bool directCompoProjectionReady =
        ctx.resources != nullptr &&
        PrSS0TitleTmdBackend::IsResourceIngressReady(resourceState) &&
        ctx.resources->GetTextureCount() > 0u &&
        ctx.resources->GetMemCount() > 0u;
    PrSS0Scene0ResourceIngressDirect::Source801C4780 ingressSource{};
    ingressSource.sceneIndexKnown = true;
    ingressSource.sceneIndex = 0u;
    ingressSource.sceneEntryBaseKnown = true;
    ingressSource.sceneEntryBase = 0x8005474Cu;
    ingressSource.dataRoot = ctx.dataRoot;
    ingressSource.directCompoProjectionKnown = true;
    ingressSource.directCompoProjectionReady = directCompoProjectionReady;
    const auto resourceIngress =
        PrSS0Scene0ResourceIngressDirect::BuildTransaction801C4780(
            ingressSource);
    const bool resourceIngressReady =
        PrSS0Scene0ResourceIngressDirect::
            IsExactAcceptedTransaction801C4780(resourceIngress);
    const bool bootStartupDiscIntReady =
        s_ss0Direct.bootStartupAudioBindingsCommitted80016C8C &&
        PrSS0ResourceAudioDirect::IsExactStartupAudioBindings80016C8C(
            s_ss0Direct.bootStartupAudioBindings80016C8C) &&
        s_ss0Direct.bootStartupDiscIntPrepared80016B84 &&
        s_ss0Direct.bootStartupPreparedBeforeFirstLogo80016B84 &&
        PrSS0Scene0ResourceIngressDirect::
            IsExactAcceptedStartupPreloadTransaction80016B84(
                s_ss0Direct.bootStartupPreloadIngress80016B84) &&
        PrSS0Scene0IntLoadDirect::
            IsExactAcceptedStartupCommonTransaction80016B84(
                s_ss0Direct.bootStartupCommonIntLoad80016B84) &&
        PrSS0Scene0IntLoadDirect::
            IsExactAcceptedZCompoTransaction80015590(
                s_ss0Direct.bootStartupZCompoIntLoad80016B84) &&
        s_ss0Direct.bootStartupVabCloseCommitted80027120 &&
        s_ss0Direct.bootStartupVabClose80027120.committed &&
        s_ss0Direct.bootStartupVabClose80027120.function == 0x80027120u &&
        s_ss0Direct.bootStartupVabClose80027120.hostProjectionCleared &&
        !s_ss0Direct.bootStartupVabClose80027120.psxSpuRegisterAuthority &&
        !s_ss0Direct.bootStartupVabClose80027120.psxInterruptTimingAuthority &&
        resourceIngressReady && resourceIngress.discBinPathKnown &&
        resourceIngress.discBinPath ==
            s_ss0Direct.bootStartupPreloadIngress80016B84.discBinPath;
    PrMovieSegmentDirect::MovieSegmentRecord48 commonRow{};
    bool commonRowReady = false;
    if (bootStartupDiscIntReady) {
        commonRow =
            s_ss0Direct.bootStartupPreloadIngress80016B84.commonRow;
        commonRowReady = commonRow.known;
    } else {
        commonRowReady =
            resourceIngressReady &&
            PrSS0Scene0ResourceIngressDirect::BuildStartupCommonRow80016B84(
                resourceIngress, commonRow);
    }
    PrSS0Scene0IntLoadDirect::StartupCommonSource80016B84
        commonIntLoadSource{};
    commonIntLoadSource.ingressKnown = true;
    commonIntLoadSource.ingressAccepted = resourceIngressReady;
    commonIntLoadSource.discBinPathKnown =
        resourceIngress.discBinPathKnown;
    commonIntLoadSource.discBinPath = resourceIngress.discBinPath;
    commonIntLoadSource.commonRowKnown = commonRowReady;
    commonIntLoadSource.commonRow = commonRow;
    PrSS0Scene0IntLoadDirect::Transaction8001AC18 commonIntLoad{};
    if (bootStartupDiscIntReady) {
        commonIntLoad =
            s_ss0Direct.bootStartupCommonIntLoad80016B84;
    } else {
        commonIntLoad = PrSS0Scene0IntLoadDirect::
            BuildStartupCommonTransaction80016B84(commonIntLoadSource);
    }
    const bool commonIntLoadReady = PrSS0Scene0IntLoadDirect::
        IsExactAcceptedStartupCommonTransaction80016B84(commonIntLoad);
    PrSS0Scene0IntLoadDirect::ZCompoSource80015590 zCompoIntLoadSource{};
    zCompoIntLoadSource.ingressKnown = true;
    zCompoIntLoadSource.ingressAccepted = resourceIngressReady;
    zCompoIntLoadSource.discBinPathKnown =
        resourceIngress.discBinPathKnown;
    zCompoIntLoadSource.discBinPath = resourceIngress.discBinPath;
    if (bootStartupDiscIntReady) {
        zCompoIntLoadSource.zCompoRowKnown =
            s_ss0Direct.bootStartupPreloadIngress80016B84.zCompoRow.known;
        zCompoIntLoadSource.zCompoRow =
            s_ss0Direct.bootStartupPreloadIngress80016B84.zCompoRow;
    } else {
        zCompoIntLoadSource.zCompoRowKnown =
            resourceIngress.scan801C4780.table.rows[6].known;
        zCompoIntLoadSource.zCompoRow =
            resourceIngress.scan801C4780.table.rows[6];
    }
    PrSS0Scene0IntLoadDirect::Transaction8001AC18 zCompoIntLoad{};
    if (bootStartupDiscIntReady) {
        zCompoIntLoad =
            s_ss0Direct.bootStartupZCompoIntLoad80016B84;
    } else {
        zCompoIntLoad =
            PrSS0Scene0IntLoadDirect::BuildZCompoTransaction80015590(
                zCompoIntLoadSource);
    }
    const bool zCompoIntLoadReady = PrSS0Scene0IntLoadDirect::
        IsExactAcceptedZCompoTransaction80015590(zCompoIntLoad);
    PrSS0Scene0IntLoadDirect::Source8001AC18 intLoadSource{};
    intLoadSource.ingressKnown = true;
    intLoadSource.ingressAccepted = resourceIngressReady;
    intLoadSource.discBinPathKnown =
        resourceIngress.discBinPathKnown;
    intLoadSource.discBinPath = resourceIngress.discBinPath;
    intLoadSource.compoRowKnown =
        resourceIngress.scan801C4780.table.rows[1].known;
    intLoadSource.compoRow =
        resourceIngress.scan801C4780.table.rows[1];
    auto intLoad =
        PrSS0Scene0IntLoadDirect::BuildTransaction8001AC18(
            intLoadSource);
    const bool intLoadReady =
        PrSS0Scene0IntLoadDirect::IsExactAcceptedTransaction8001AC18(
            intLoad);
    PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0 intSideEffect{};
    if (intLoadReady && ctx.mainSceneLoaderMemoryDirect != nullptr) {
        auto loaderMemoryCandidate =
            std::make_unique<PrStage1LoaderMemoryDirectState>(
                *ctx.mainSceneLoaderMemoryDirect);
        if (PrSS0TitleTmdBackend::
                ApplyScene0LoaderResetPreservingTitlePacketArena80025A34(
                    *loaderMemoryCandidate)) {
            intSideEffect = PrSS0Scene0IntSideEffectDirect::
                BuildTransaction8001A8F0(
                    intLoad, std::move(loaderMemoryCandidate));
        }
    }
    const bool intSideEffectReady =
        PrSS0Scene0IntSideEffectDirect::
            IsExactAcceptedTransaction8001A8F0(
                intSideEffect, intLoad);
    PrSS0Scene0IntSpuDirect::PadStartComSource80026E4C
        padStartComSource{};
    padStartComSource.staticBodyKnown = true;
    padStartComSource.cuePointerKnown = globalBindingsReady;
    padStartComSource.cuePointer80094410 =
        s_ss0Direct.scene0GlobalBindings.slot80094410;
    auto intSpu = PrSS0Scene0IntSpuDirect::BuildTransaction8001A8F0(
        intLoad, intSideEffect, padStartComSource);
    const bool intSpuReady =
        PrSS0Scene0IntSpuDirect::IsExactAcceptedTransaction8001A8F0(
            intSpu, intLoad, intSideEffect, padStartComSource);
    PrSfx::Scene0IntVabCandidate8001A8F0 intSpuHostCandidate{};
    if (intSpuReady) {
        intSpuHostCandidate = PrSfx::BuildScene0IntVabCandidate8001A8F0(
            intSpu.vhBytes.data(),
            intSpu.vhBytes.size(),
            intSpu.vbBytes.data(),
            intSpu.vbBytes.size());
    }
    const bool intSpuHostCandidateReady =
        intSpuReady &&
        PrSfx::IsExactScene0IntVabCandidate8001A8F0(
            intSpuHostCandidate,
            intSpu.vhBytes.size(),
            intSpu.vhHash,
            intSpu.vbBytes.size(),
            intSpu.vbHash,
            intSpu.decoderToneSlotCount,
            intSpu.decoderEffectiveVagCount);
    auto intSpuCommitted = intSpu;
    const bool intSpuPadStartComGlobalWritesReady =
        intSpuReady &&
        PrSS0Scene0IntSpuDirect::
            CommitPadStartComGlobalWrites80026E4C(
                intSpuCommitted,
                intLoad,
                intSideEffect,
                padStartComSource) &&
        PrSS0Scene0IntSpuDirect::
            IsExactPadStartComGlobalWritesCommittedTransaction80026E4C(
                intSpuCommitted,
                intLoad,
                intSideEffect,
                padStartComSource);
    intSpuCommitted.hostVabCandidatePrepared =
        intSpuHostCandidateReady;
    const bool intSpuCommitReady =
        intSpuPadStartComGlobalWritesReady && intSpuHostCandidateReady &&
        PrSS0Scene0IntSpuDirect::CommitHostVabBank8001A8F0(
            intSpuCommitted) &&
        PrSS0Scene0IntSpuDirect::IsExactCommittedTransaction8001A8F0(
            intSpuCommitted,
            intLoad,
            intSideEffect,
            padStartComSource);
    auto intGpu = PrSS0Scene0IntGpuDirect::BuildTransaction8001AE7C(
        intLoad, intSideEffect);
    const bool intGpuReady =
        PrSS0Scene0IntGpuDirect::CommitPreparedTransaction8001AE7C(
            intGpu, intLoad, intSideEffect);
    auto intRenderer =
        PrSS0Scene0IntRendererDirect::BuildTransaction8001AE7C(
            intLoad, intSideEffect, intGpu);
    const bool intRendererReady =
        PrSS0Scene0IntRendererDirect::
            IsExactPreparedTransaction8001AE7C(
                intRenderer, intLoad, intSideEffect, intGpu);
    auto intRendererAtlasCandidate = PrSS0TitleTmdBackend::
        BuildScene0IntRendererAtlasCandidate8001AE7C(
            intRenderer, commonIntLoad, zCompoIntLoad, intLoad);
    const bool intRendererAtlasPartialVramReady =
        intRendererAtlasCandidate.prepared &&
        PrSS0TitleTmdBackend::ApplyRendererAtlasPartialVram8001AE7C(
            intRendererAtlasCandidate, intGpu);
    const bool intRendererAtlasReady = PrSS0TitleTmdBackend::
        IsExactScene0IntRendererAtlasCandidate8001AE7C(
            intRendererAtlasCandidate,
            intRenderer,
            commonIntLoad,
            zCompoIntLoad,
            intLoad);
    const bool intLoaderMemoryCommitReady =
        intRendererAtlasReady &&
        PrSS0Scene0IntSideEffectDirect::
            CommitLoaderMemoryAndHandleTable8001A8F0(
                intSideEffect, intLoad) &&
        PrSS0Scene0IntSideEffectDirect::
            IsExactLoaderMemoryCommittedTransaction8001A8F0(
                intSideEffect, intLoad);
    PrSS0Scene0IntSideEffectDirect::FinalResultSource8001AC18
        finalIntResultSource{};
    finalIntResultSource.staticEofReturnChainKnown = true;
    finalIntResultSource.eofSuccessKnown = true;
    finalIntResultSource.eofSuccess =
        intLoad.eofPresent && intLoad.eofHeaderEndsArchive;
    finalIntResultSource.loaderMemoryCommitted =
        intSideEffect.loaderMemoryCommitted;
    finalIntResultSource.memHandleTableCommitted =
        intSideEffect.memHandleTableCommitted;
    finalIntResultSource.timPartialVramCommitted =
        intGpuReady && intGpu.directVramCommitted &&
        intGpu.partialVramWordAuthority;
    finalIntResultSource.hostVabBankCommitted =
        intSpuCommitReady && intSpuCommitted.hostVabBankCommitted;
    const bool finalIntResultReady =
        intLoaderMemoryCommitReady &&
        PrSS0Scene0IntSideEffectDirect::CommitFinalResult8001AC18(
            intSideEffect, intLoad, finalIntResultSource) &&
        PrSS0Scene0IntSideEffectDirect::
            IsExactFinalResultTransaction8001AC18(
                intSideEffect, intLoad);

    PrSS0Scene0GlobalBindingsDirect::InitSource801C4260 source{};
    source.sceneIndexKnown = true;
    source.sceneIndex = 0u;
    source.sceneEntryBaseKnown = true;
    source.sceneEntryBase = 0x8005474Cu;
    source.sceneHeaderKnown = true;
    source.bpm100 = 9600;
    source.tickOffset = 96;
    source.extraTick = 0;
    source.resetSourcesKnown = true;
    source.reset80025A34Ready = resetSourcesReady;
    source.reset801C4FA0Ready = resetSourcesReady;
    source.reset80024E98Ready = resetSourcesReady;
    source.reset80014344Ready = resetSourcesReady;
    source.resourceIngress801C4780Known = true;
    source.resourceIngress801C4780Ready = resourceIngressReady;

    const auto transaction =
        PrSS0Scene0GlobalBindingsDirect::BuildInitTransaction801C4260(source);
    PrSS0Scene0SharedEventPredispatchDirect::StaticDispatchState80024FD0
        resetDispatch{};
    PrSS0Scene0SharedEventPredispatchDirect::
        ResetStaticDispatchState80014344To80024E98(resetDispatch);
    const bool resetDispatchReady =
        PrSS0Scene0SharedEventPredispatchDirect::
            ApplyStaticDispatchReset80024E98(resetDispatch);
    if (!PrSS0Scene0GlobalBindingsDirect::
            IsExactInitTransaction801C4260(transaction) ||
        !globalBindingsReady || !resetDispatchReady ||
        ctx.mainSceneLoaderMemoryDirect == nullptr ||
        !bootStartupDiscIntReady ||
        !commonIntLoadReady || !zCompoIntLoadReady || !intLoadReady ||
        !intSideEffectReady || !intSpuReady ||
        !intSpuHostCandidateReady || !intSpuCommitReady || !intGpuReady ||
        !intRendererReady || !intRendererAtlasPartialVramReady ||
        !intRendererAtlasReady || !intLoaderMemoryCommitReady ||
        !finalIntResultReady) {
        Log::Printf(
            "SS0 direct runtime: Fn1 801C4260 init blocked status=%u ingressStatus=%u commonStatus=%u zStatus=%u intStatus=%u sideEffectStatus=%u spuStatus=%u gpuStatus=%u rendererStatus=%u bindings=%d reset=%d rows=%d/%d resources=%d preLogoDiscInt=%d common=%d z=%d intPayload=%d intSideEffect=%d intSpu=%d padStartComGlobals=%d hostVabCandidate=%d intGpu=%d intRenderer=%d partialAtlas=%d atlasCandidate=%d loaderMemoryCommit=%d finalIntResult=%d",
            static_cast<unsigned>(transaction.status),
            static_cast<unsigned>(resourceIngress.status),
            static_cast<unsigned>(commonIntLoad.status),
            static_cast<unsigned>(zCompoIntLoad.status),
            static_cast<unsigned>(intLoad.status),
            static_cast<unsigned>(intSideEffect.status),
            static_cast<unsigned>(intSpu.status),
            static_cast<unsigned>(intGpu.status),
            static_cast<unsigned>(intRenderer.status),
            globalBindingsReady ? 1 : 0,
            resetSourcesReady ? 1 : 0,
            static_cast<int>(
                resourceIngress.scan801C4780.rowCdLookupReadyCount),
            static_cast<int>(
                resourceIngress.scan801C4780.rowNeedsCdLookupCount),
            resourceIngressReady ? 1 : 0,
            bootStartupDiscIntReady ? 1 : 0,
            commonIntLoadReady ? 1 : 0,
            zCompoIntLoadReady ? 1 : 0,
            intLoadReady ? 1 : 0,
            intSideEffectReady ? 1 : 0,
            intSpuReady ? 1 : 0,
            intSpuPadStartComGlobalWritesReady ? 1 : 0,
            intSpuHostCandidateReady ? 1 : 0,
            intGpuReady ? 1 : 0,
            intRendererReady ? 1 : 0,
            intRendererAtlasPartialVramReady ? 1 : 0,
            intRendererAtlasReady ? 1 : 0,
            intLoaderMemoryCommitReady ? 1 : 0,
            finalIntResultReady ? 1 : 0);
        return false;
    }

    // All potentially rejecting work is above. The remaining direct-state
    // publication mirrors 801C4260 as one no-failure host transaction.
    if (!PrSS0TitleTmdBackend::
            ApplyScene0LoaderResetPreservingTitlePacketArena80025A34(
            *ctx.mainSceneLoaderMemoryDirect)) {
        Log::Printf(
            "SS0 direct runtime: Fn1 801C4260 loader reset blocked packetArenaReservation=0");
        return false;
    }
    if (!PrSS0TitleTmdBackend::
            CommitScene0IntRendererAtlasCandidate8001AE7C(
                std::move(intRendererAtlasCandidate),
                intRenderer,
                commonIntLoad,
                zCompoIntLoad,
                intLoad)) {
        Log::Printf(
            "SS0 direct runtime: Fn1 801C4260 renderer atlas commit blocked");
        return false;
    }
    const auto scene0VabClose80027120 =
        PrSfx::ApplyScene0VabClose80027120();
    if (!scene0VabClose80027120.committed ||
        scene0VabClose80027120.function != 0x80027120u ||
        !scene0VabClose80027120.hostProjectionCleared ||
        scene0VabClose80027120.psxSpuRegisterAuthority ||
        scene0VabClose80027120.psxInterruptTimingAuthority) {
        Log::Printf(
            "SS0 direct runtime: Fn1 801C4260 80027120 close barrier blocked");
        return false;
    }
    PrSfx::CommitScene0IntVabCandidate8001A8F0(
        std::move(intSpuHostCandidate));
    *ctx.mainSceneLoaderMemoryDirect =
        *intSideEffect.loaderMemoryCandidate;
    if (ctx.resources == nullptr ||
        !PrSS0TitleTmdBackend::CommitCompo00HandleTable80091858(
            *ctx.mainSceneLoaderMemoryDirect, *ctx.resources)) {
        Log::Printf(
            "SS0 direct runtime: Fn1 801C4260 COMPO00 dword_80091858 handle table commit blocked");
        return false;
    }
    const auto committedTitleResources =
        PrSS0TitleTmdBackend::GetResourceState();
    Log::Printf(
        "SS0 direct runtime: COMPO00 dword_80091858 handle table committed known=%d committed=%d nonZero=%u generation=%u",
        committedTitleResources.compo00HandleTableKnown80091858 ? 1 : 0,
        committedTitleResources.compo00HandleTableCommitted80091858 ? 1 : 0,
        static_cast<unsigned>(
            committedTitleResources.compo00HandleTableNonZeroCount80091858),
        static_cast<unsigned>(
            committedTitleResources.compo00HandleTableGeneration80091858));
    PrSS0TitleHudEventsDirect::ResetTitleAbsoluteEventStreamState80024E98(
        s_ss0Direct.titleAbsoluteEventStream);
    SS0DirectCommitTitleEventProjectionReset80024E98(resetDispatch);
    s_ss0Direct.scene0ResourceIngress801C4780 = resourceIngress;
    s_ss0Direct.startupCommonIntLoad80016B84 = std::move(commonIntLoad);
    s_ss0Direct.startupZCompoIntLoad80016B84 = std::move(zCompoIntLoad);
    s_ss0Direct.scene0IntLoad8001AC18 = std::move(intLoad);
    s_ss0Direct.scene0IntSideEffect8001A8F0 =
        std::move(intSideEffect);
    s_ss0Direct.scene0IntSpu8001A8F0 = std::move(intSpuCommitted);
    s_ss0Direct.scene0IntGpu8001AE7C = std::move(intGpu);
    s_ss0Direct.scene0IntRenderer8001AE7C = std::move(intRenderer);
    s_ss0Direct.scene0Init801C4260 = transaction;
    s_ss0Direct.scene0IntSideEffectGateCommitted = true;
    s_ss0Direct.scene0IntSpuGateCommitted = true;
    s_ss0Direct.scene0IntGpuGateCommitted = true;
    s_ss0Direct.scene0IntRendererGateCommitted = true;
    Log::Printf(
        "SS0 direct runtime: Fn1 801C4260 committed calls=80025A34>801C4FA0>80024E98>80014344 timing=9216/3/96/16 ingress=801C4780 rows=%u/%u discDirectory=1 startupAtlas=COMMON53>ZCOMPO581>COMPO87 total721 intPayload=8001AC18/8001A8F0 blocks=%u entries=%u heapCandidate=8001A8F0 tim=%u vab=1/2 mem=2..%u cursor=%08X loaderMemory=1 memHandles=1 final8001AC18=1 finalResultAuthority=1 spu=80027120>PadStartCom>80027078>800270D4>800270FC padStartComGlobals=1 padStartComLower=0 hostVabBank=1 psxVabId=0 psxSpuRam=0 padStartCom=0 audible=0 gpu=8001AE7C actions=%u loadImage=%u knownWords=%u writes=%llu partialVram=1 fullVram=0 rendererAtlasCpu=1 rendererAtlasD3D=0 sideEffects=0 hostProjection=1",
        resourceIngress.scan801C4780.rowCdLookupReadyCount,
        resourceIngress.scan801C4780.rowNeedsCdLookupCount,
        s_ss0Direct.scene0IntLoad8001AC18.blockCount,
        s_ss0Direct.scene0IntLoad8001AC18.entryCount,
        static_cast<unsigned>(
            s_ss0Direct.scene0IntSideEffect8001A8F0.timUploadRequests.size()),
        static_cast<unsigned>(
            s_ss0Direct.scene0IntSideEffect8001A8F0.finalStackDepth),
        s_ss0Direct.scene0IntSideEffect8001A8F0.finalHeapCursor,
        static_cast<unsigned>(
            s_ss0Direct.scene0IntGpu8001AE7C.totalGpuActionCount),
        static_cast<unsigned>(
            s_ss0Direct.scene0IntGpu8001AE7C.totalLoadImageCount),
        static_cast<unsigned>(
            s_ss0Direct.scene0IntGpu8001AE7C.finalKnownWordCount),
        static_cast<unsigned long long>(
            s_ss0Direct.scene0IntGpu8001AE7C.totalWriteWordCount));
    Log::Printf(
        "SS0 direct runtime: Fn1 801C4260 reused pre-logo 80016C8C/80016B84 source startupAudioBindings=1 preLogoDiscInt=1 preLogoFrame=%u commonAttempts=%u commonVabClose=1 priorHostVab=%u zAttempts=%u commonTim=%u zTim=%u timVramSideEffectsAtPreLogo=0 vabSpuSideEffectsAtPreLogo=0 inputAudioTail80026FA4AtPreLogo=1 resetVoices=%u resetMask=%06X",
        static_cast<unsigned>(
            s_ss0Direct.bootStartupPrepareFrame80016B84),
        static_cast<unsigned>(
            s_ss0Direct.bootStartupCommonAttemptCount8001AC18),
        s_ss0Direct.bootStartupVabClose80027120.priorHostVabActive ? 1u : 0u,
        static_cast<unsigned>(
            s_ss0Direct.bootStartupZCompoAttemptCount8001AC18),
        static_cast<unsigned>(
            s_ss0Direct.startupCommonIntLoad80016B84.timEntryCount),
        static_cast<unsigned>(
            s_ss0Direct.startupZCompoIntLoad80016B84.timEntryCount),
        static_cast<unsigned>(
            s_ss0Direct.bootStartupAudioReset80026FA4.returnedVoiceCount),
        s_ss0Direct.bootStartupAudioReset80026FA4.resetVoiceMask);
    (void)SS0DirectLoadScene0SubtitleData(ctx);
    return true;
}

static void SS0DirectCommitTitleEntryPrefixTransaction801C4894(
    const PrSS0TitleEntryPrefixDirect::Transaction801C4894& transaction) {
    s_ss0Direct.titleInitialClock = {};
    s_ss0Direct.titleInitialCtxTick96Known = false;
    s_ss0Direct.titleInitialCtxTick96 = 0u;
    s_ss0Direct.titleCtxTick96Known = false;
    s_ss0Direct.titleCtxTick96 = 0u;
    s_ss0Direct.titleCtxBeatKnown = false;
    s_ss0Direct.titleCtxBeat = 0u;
    PrSS0Scene0SharedEventPredispatchDirect::StaticDispatchState80024FD0
        resetDispatch{};
    PrSS0Scene0SharedEventPredispatchDirect::
        ResetStaticDispatchState80014344To80024E98(resetDispatch);
    SS0DirectCommitTitleEventProjectionReset80024E98(resetDispatch);
    PrSS0TitleHudEventsDirect::ResetTitleAbsoluteEventStreamState80024E98(
        s_ss0Direct.titleAbsoluteEventStream);
    PrSS0TitlePacketRenderDirect::ResetHostVisibleTitleFrameCache801C689C(
        s_ss0Direct.titleHostVisibleFrameCache);
    s_ss0Direct.titleSelectorHudRuntimeState = {};
    s_ss0Direct.titleSelectorHudRuntimeState.resourceCooldownKnown = true;
    s_ss0Direct.titleSelectorHudRuntimeState.resourceCooldown = 0;
    s_ss0Direct.titleEntryPrefixTransaction = transaction;
    Log::Printf(
        "SS0 direct runtime: title entry prefix 80014344->80024E98->80024FC0->801C4FA0->80024C84 committed steps=%u ctx=801C3640 table=801C6DA4 ctx54=%d ctx68=%d hostProjection=%d psxMemoryBackingAuthority=%d",
        static_cast<unsigned>(transaction.stepCount),
        s_ss0Direct.optionsWord800916DC,
        s_ss0Direct.optionsWord800916DC,
        transaction.hostProjection ? 1 : 0,
        transaction.psxMemoryBackingAuthority ? 1 : 0);
}

static void SS0DirectEnterTitleMovie0T(PrGameContext& ctx) {
    SS0DirectResetToTitle(ctx);
    (void)SS0DirectResolveTitleAttractTimeoutPolicy801C48C0();
    s_ss0Direct.phase = SS0DirectPhase::TitleIntroTransition;

    if (!SS0DirectPreflightSlowTransitionTextures80020110(
            ctx,
            PrSS0TransitionDirect::kScene0WorkAddress,
            1,
            2,
            1) ||
        !SS0DirectStartScene0Str(
            ctx,
            "801C44E0 MOVIE0T",
            3u,
            PrSS0StrLifecycleDirect::StrMovieKind::TitleMovie0T)) {
        Log::Printf(
            "SS0 direct runtime: title intro 801C4958/80020110 start blocked");
        return;
    }

    PrSS0TitleEntryPrefixDirect::Transaction801C4894 prefixCandidate{};
    PrSS0TransitionDirect::SlowTransitionRuntime80020110 transitionCandidate{};
    if (!SS0DirectBuildTitleEntryPrefixTransaction801C4894(prefixCandidate) ||
        !prefixCandidate.accepted || !prefixCandidate.complete ||
        !prefixCandidate.hostProjection ||
        prefixCandidate.psxMemoryBackingAuthority ||
        prefixCandidate.stage1StateOrHeaderAuthority ||
        prefixCandidate.oldWinS0Authority ||
        prefixCandidate.stage2PlusAuthority ||
        prefixCandidate.replayValueAuthority ||
        prefixCandidate.hostFilesystemAuthority ||
        prefixCandidate.gameLiveProbeAuthority ||
        !PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
            transitionCandidate,
            PrSS0TransitionDirect::kScene0WorkAddress,
            1,
            2,
            1)) {
        SS0DirectStopTitleStr(ctx);
        Log::Printf(
            "SS0 direct runtime: title entry prefix/80020110 all-clear blocked subtitle=%d",
            s_ss0Direct.optionsWord800916DC);
        return;
    }

    SS0DirectCommitTitleEntryPrefixTransaction801C4894(prefixCandidate);
    s_ss0Direct.titleIntroTransition = transitionCandidate;
    // 801C4958 -> 80020110 also draws its first spiral cell immediately.
    s_ss0Direct.titleIntroTransitionVisual =
        PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
            PrSS0TransitionDirect::kScene0WorkAddress,
            1,
            2,
            1,
            false,
            0u);
    s_ss0Direct.titleIntroTransitionPresentBlockedLogged = false;
    s_ss0Direct.titleMovie0TFinalReadyPollsRemaining = 1800u;
    s_ss0Direct.titleMovie0TFinalReadyInterPollTicksRemaining = 0u;
    s_ss0Direct.titleMovie0TFinalReadyPreloopActive = false;
    s_ss0Direct.titleMovie0TFinalReadyBlockedLogged = false;
    SS0DirectBeginLoadingScreen(
        SS0DirectLoadingHoldKind::TitleIntro, ctx.frame);
    Log::Printf(
        "SS0 direct runtime: title intro 801C4958 -> 80020110 mode=1 started");
}

static void SS0DirectApplySub80027194CadenceBeforeWait(
    const PrSS0TransitionDirect::SlowTransitionTickResult80020110& tick) {
    if (!tick.sub80027194CallRequired) {
        return;
    }

    (void)PrSfx::ApplyMovieTransitionCueCadence80027194();
}

// Tick*TransitionRuntime models the two PSX VBlanks that make one grid
// iteration.  Scene0's game logic is already a 30 Hz host tick, so consume
// both VBlank steps inside this owner and publish exactly one visual grid
// iteration per logic tick.  The low-level helpers remain untouched for
// their unit tests and for callers that explicitly model individual VBlanks.
static PrSS0TransitionDirect::SlowTransitionTickResult80020110
SS0DirectTickSlowTransitionForHostLogic(
    PrSS0TransitionDirect::SlowTransitionRuntime80020110& runtime) {
    const auto runtimeBefore = runtime;
    const auto first = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(
        runtime);
    if (!first.accepted) {
        return first;
    }
    SS0DirectApplySub80027194CadenceBeforeWait(first);
    const auto second = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(
        runtime);
    if (!second.accepted) {
        runtime = runtimeBefore;
        return second;
    }
    static bool loggedHostCadence = false;
    if (!loggedHostCadence) {
        loggedHostCadence = true;
        Log::Printf(
            "SS0 direct runtime: transition cadence maps 2 PSX VBlanks to one 30Hz logic tick (one visual grid iteration per tick)");
    }
    return second;
}

static PrSS0TransitionDirect::FastTransitionTickResult800201AC
SS0DirectTickFastTransitionForHostLogic(
    PrSS0TransitionDirect::FastTransitionRuntime800201AC& runtime) {
    const auto runtimeBefore = runtime;
    const auto first = PrSS0TransitionDirect::TickFastTransitionRuntime800201AC(
        runtime);
    if (!first.accepted) {
        return first;
    }
    if (first.sub800271E4CallRequired) {
        SS0DirectApplyFastTransitionCue800271E4(
            runtime,
            first.visualFrame.iteration,
            first.sub800271E4CueIndex);
    }
    const auto second = PrSS0TransitionDirect::TickFastTransitionRuntime800201AC(
        runtime);
    if (!second.accepted) {
        runtime = runtimeBefore;
        return second;
    }
    return second;
}

static int SS0DirectTickOpeningMovie0InitialTransition80020110(
    PrGameContext& ctx) {
    if (!s_ss0Direct.openingMovie0InitialTransition.active &&
        s_ss0Direct.openingMovie0InitialTransition.phase ==
            PrSS0TransitionDirect::SlowTransitionRuntimePhase80020110::Idle) {
        (void)SS0DirectStartScene0OuterEntry801C4DC4(ctx);
        return 0;
    }

    bool heldComplete = false;
    PrSS0TransitionDirect::SlowTransitionTickResult80020110 tick{};
    if (SS0DirectLoadingMinimumHoldActive(
            SS0DirectLoadingHoldKind::OpeningMovie0Initial)) {
        if (!SS0DirectLoadingMinimumHoldReady(
                SS0DirectLoadingHoldKind::OpeningMovie0Initial, ctx.frame)) {
            return 0;
        }
        heldComplete = true;
    } else {
        const auto runtimeBefore = s_ss0Direct.openingMovie0InitialTransition;
        tick = SS0DirectTickSlowTransitionForHostLogic(
            s_ss0Direct.openingMovie0InitialTransition);
        if (!tick.accepted) {
            if (!s_ss0Direct.openingMovie0InitialTransitionPresentBlockedLogged) {
                s_ss0Direct.openingMovie0InitialTransitionPresentBlockedLogged =
                    true;
                Log::Printf(
                    "SS0 direct runtime: 801C4DC4 initial 80020110 blocked phase=%u",
                    static_cast<unsigned>(tick.phaseBefore));
            }
            return 0;
        }

        if (tick.presentRequired) {
            const auto present =
                PrSS0TitleTmdBackend::ApplySlowTransitionPresent8001EBF4(
                    tick.visualFrame);
            if (!present.committed) {
                s_ss0Direct.openingMovie0InitialTransition = runtimeBefore;
                if (!s_ss0Direct.openingMovie0InitialTransitionPresentBlockedLogged) {
                    s_ss0Direct.openingMovie0InitialTransitionPresentBlockedLogged =
                        true;
                    Log::Printf(
                        "SS0 direct runtime: 801C4DC4 initial 80020110 present blocked iteration=%u tail=%d",
                        tick.visualFrame.iteration,
                        tick.visualFrame.tail ? 1 : 0);
                }
                return 0;
            }
            s_ss0Direct.openingMovie0InitialTransitionPresentBlockedLogged = false;
        }
        if (tick.visualFrame.known) {
            s_ss0Direct.openingMovie0InitialTransitionVisual = tick.visualFrame;
        }
        if (!tick.complete) {
            return 0;
        }
    }

    if (!heldComplete && !SS0DirectFinishLoadingScreenOrHold(
            SS0DirectLoadingHoldKind::OpeningMovie0Initial,
            ctx.frame)) {
        return 0;
    }

    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.openingMovie0InitialTransition);
    s_ss0Direct.openingMovie0InitialTransitionVisual = {};
    s_ss0Direct.openingMovie0InitialTransitionPresentBlockedLogged = false;
    Log::Printf(
        "SS0 direct runtime: 801C4DC4 initial 80020110 mode=2 complete -> 800201AC mode=6");
    (void)SS0DirectStartScene0OuterEntry801C4DC4(ctx);
    return 0;
}

static int SS0DirectTickOpeningMovie0Transition(PrGameContext& ctx,
                                                bool beforeMovie) {
    const auto holdKind = beforeMovie
        ? SS0DirectLoadingHoldKind::OpeningMovie0Pre
        : SS0DirectLoadingHoldKind::OpeningMovie0Post;
    bool heldComplete = false;
    PrSS0TransitionDirect::FastTransitionTickResult800201AC tick{};
    if (SS0DirectLoadingMinimumHoldActive(holdKind)) {
        if (!SS0DirectLoadingMinimumHoldReady(holdKind, ctx.frame)) {
            return 0;
        }
        heldComplete = true;
        tick.complete = true;
        tick.loopIterationsCompleted =
            s_ss0Direct.openingMovie0Transition.loopIterationsCompleted;
        tick.tailIterationsCompleted =
            s_ss0Direct.openingMovie0Transition.tailIterationsCompleted;
    } else {
        const auto runtimeBefore = s_ss0Direct.openingMovie0Transition;
        tick = SS0DirectTickFastTransitionForHostLogic(
            s_ss0Direct.openingMovie0Transition);
        if (!tick.accepted) {
            Log::Printf(
                "SS0 direct runtime: 800201AC mode=%d transition blocked phase=%u",
                beforeMovie ? 6 : 5,
                static_cast<unsigned>(tick.phaseBefore));
            if (beforeMovie) {
                if (SS0DirectStartScene0Str(
                        ctx,
                        "801C44E0 MOVIE0",
                        2u,
                        PrSS0StrLifecycleDirect::StrMovieKind::OpeningMovie0)) {
                    s_ss0Direct.phase = SS0DirectPhase::OpeningMovie0;
                } else {
                    SS0DirectEnterTitleMovie0T(ctx);
                }
            } else {
                SS0DirectEnterTitleMovie0T(ctx);
            }
            return 0;
        }
        if (tick.loopIterationCompleted || tick.tailIterationCompleted) {
            if (!tick.visualFrame.known ||
                !SS0DirectPreflightFastTransitionTextures800201AC(
                    ctx, tick.visualFrame)) {
                s_ss0Direct.openingMovie0Transition = runtimeBefore;
                if (!s_ss0Direct.openingMovie0TransitionPresentBlockedLogged) {
                    s_ss0Direct.openingMovie0TransitionPresentBlockedLogged = true;
                    Log::Printf(
                        "SS0 direct runtime: 8001C864/8001CE30 resource preflight blocked mode=%d iteration=%u tail=%d",
                        beforeMovie ? 6 : 5,
                        tick.visualFrame.iteration,
                        tick.visualFrame.tail ? 1 : 0);
                }
                return 0;
            }
            const auto present =
                PrSS0TitleTmdBackend::ApplyFastTransitionPresent8001EBF4(
                    tick.visualFrame);
            if (!present.committed) {
                s_ss0Direct.openingMovie0Transition = runtimeBefore;
                if (!s_ss0Direct.openingMovie0TransitionPresentBlockedLogged) {
                    s_ss0Direct.openingMovie0TransitionPresentBlockedLogged = true;
                    Log::Printf(
                        "SS0 direct runtime: 8001EA74/8001EBF4 present blocked mode=%d iteration=%u tail=%d",
                        beforeMovie ? 6 : 5,
                        tick.visualFrame.iteration,
                        tick.visualFrame.tail ? 1 : 0);
                }
                return 0;
            }
            s_ss0Direct.openingMovie0TransitionPresentBlockedLogged = false;
        }
        if (tick.visualFrame.known) {
            s_ss0Direct.openingMovie0TransitionVisual = tick.visualFrame;
        }
        if (!tick.complete) {
            return 0;
        }
    }

    if (!heldComplete && !SS0DirectFinishLoadingScreenOrHold(
            holdKind, ctx.frame)) {
        return 0;
    }

    Log::Printf(
        "SS0 direct runtime: 800201AC mode=%d transition complete loop=%u tail=%u",
        beforeMovie ? 6 : 5,
        tick.loopIterationsCompleted,
        tick.tailIterationsCompleted);
    if (beforeMovie) {
        if (SS0DirectStartScene0Str(
                ctx,
                "801C44E0 MOVIE0",
                2u,
                PrSS0StrLifecycleDirect::StrMovieKind::OpeningMovie0)) {
            s_ss0Direct.phase = SS0DirectPhase::OpeningMovie0;
        } else {
            SS0DirectEnterTitleMovie0T(ctx);
        }
    } else {
        SS0DirectEnterTitleMovie0T(ctx);
    }
    return 0;
}

static bool SS0DirectApplyOpeningMovie0TextWindow8001EC54(
    PrGameContext& ctx) {
    (void)ctx;
    if (s_ss0Direct.optionsWord800916DC == 0 ||
        !s_ss0Direct.scene0SubtitleDataAvailable) {
        // 8001EC54 takes the 8001CE30 no-subtitle branch when the global
        // 800916DC flag is zero.  The existing transition renderer owns that
        // frame geometry, so there is no glyph packet to submit here.
        return true;
    }

    const auto& textRuntime = s_ss0Direct.scene0SubtitleVtextRuntime;
    if (textRuntime.activeTextPtr == nullptr ||
        textRuntime.activeDurationFramesRemaining == 0u) {
        return true;
    }

    PrPsxGraphOwnerDirect::PsxGraphState* graph =
        PrSS0TitleTmdBackend::GetTitleGraphStateMutable();
    if (graph == nullptr || !graph->mainPageWorkLists80087288Initialized ||
        graph->word_80096590 > 1u) {
        Log::Printf(
            "SS0 direct runtime: 8001EC54 text packet graph binding blocked graph=%d workLists=%d lane=%u",
            graph != nullptr ? 1 : 0,
            graph != nullptr && graph->mainPageWorkLists80087288Initialized
                ? 1
                : 0,
            graph != nullptr ? static_cast<unsigned>(graph->word_80096590)
                             : 0u);
        return false;
    }

    const uint8_t slot = static_cast<uint8_t>(
        PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(*graph) & 1u);
    const auto& page = graph->mainPageWorkLists80087288[slot];
    PrStage1MovieTextDirect::Movie1TextCurrentGp872WorkCarrier currentWork{};
    currentWork.usesCurrentGp872DrawBuffer = true;
    currentWork.workBasePsxAddr =
        PrStage1MovieTextDirect::kMovie1TextOtBufferBasePsxAddr;
    currentWork.workStrideBytes =
        PrStage1MovieTextDirect::kMovie1TextOtBufferStrideBytes;
    currentWork.gp872SlotKnown = true;
    currentWork.gp872Slot = slot;
    currentWork.workAddrKnown = true;
    currentWork.workAddr = page.workAddr;
    currentWork.drawOtagAddrKnown = true;
    currentWork.drawOtagAddr = page.workAddr + currentWork.workLastAddrOffset;

    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 submitRuntime{};
    const auto runtimeBuild =
        PrPsxGraphOwnerDirect::BuildMovie1TextMainRuntimeState8003FA20FromGraphOwner(
            *graph, submitRuntime);
    if (!runtimeBuild.seedFound || !runtimeBuild.built ||
        runtimeBuild.slot8004019C != slot ||
        runtimeBuild.packetAllocatorBase8006ED50 == 0u) {
        Log::Printf(
            "SS0 direct runtime: 8001EC54 text packet runtime build blocked slot=%u seed=%d built=%d allocator=%08X",
            static_cast<unsigned>(slot),
            runtimeBuild.seedFound ? 1 : 0,
            runtimeBuild.built ? 1 : 0,
            runtimeBuild.packetAllocatorBase8006ED50);
        return false;
    }

    PrStage1MovieTextDirect::Movie1TextDrawCommand command{};
    command.kind =
        PrStage1MovieTextDirect::Movie1TextDrawCommandKind::
            SubmitTextFastSpriteSequenceSub8001B954;
    command.psxFunctionAddr =
        PrStage1MovieTextDirect::kMovie1TextGlyphLoopFunctionSub8001B954;
    command.fastSpriteSourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
            Stage1MovieText;
    command.usesCurrentGp872DrawBuffer = true;
    command.otBufferBasePsxAddr =
        PrStage1MovieTextDirect::kMovie1TextOtBufferBasePsxAddr;
    command.otBufferStrideBytes =
        PrStage1MovieTextDirect::kMovie1TextOtBufferStrideBytes;
    command.x = 24u;
    command.y = 184u;
    command.z = 0u;
    command.scale = 480u;
    command.textPtr = textRuntime.activeTextPtr;
    command.textPsxAddr = textRuntime.text268MirrorPsxAddr;
    command.emitsGsSortFastSprite = true;
    command.centeredLineWidth =
        PrStage1MovieTextDirect::kMovie1TextGlyphCenteredBodyWidthSub8001B954;
    command.glyphWidthTablePsxAddr =
        PrPsxTextGlyphMetricsDirect::kGlyphMetricTableAddress8004945C;
    command.glyphSequenceBuiltSub8001B954 = true;

    const auto sequence =
        PrStage1MovieTextDirect::BuildTextFastSpriteSequenceSub8001B954(
            command,
            PrStage1MovieTextDirect::Movie1TextGlyphMetricTablesSub8001B954{},
            currentWork);
    if (!sequence.valid || sequence.lineCount == 0u ||
        !sequence.glyphMetricTablesKnown) {
        Log::Printf(
            "SS0 direct runtime: 8001EC54->8001DB00->8001B954 text packet sequence blocked valid=%d lines=%u metrics=%d text=%08X",
            sequence.valid ? 1 : 0,
            static_cast<unsigned>(sequence.lineCount),
            sequence.glyphMetricTablesKnown ? 1 : 0,
            textRuntime.text268MirrorPsxAddr);
        return false;
    }

    const auto apply =
        PrStage1MovieTextDirect::ApplyTextFastSpriteSequenceSub8001B954(
            sequence, submitRuntime);
    if (!apply.valid || apply.appliedSubmitCount == 0u) {
        Log::Printf(
            "SS0 direct runtime: 8001EC54->8001DB00->8001B954 glyph apply blocked valid=%d submits=%u text=%08X",
            apply.valid ? 1 : 0,
            static_cast<unsigned>(apply.appliedSubmitCount),
            textRuntime.text268MirrorPsxAddr);
        return false;
    }

    const auto commit =
        PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
            *graph, slot, submitRuntime);
    if (!commit.committed || !commit.packetWriteMirrorUpdated ||
        !commit.otSlotMirrorUpdated || !commit.allocatorUpdated) {
        Log::Printf(
            "SS0 direct runtime: 8001EC54 glyph packet commit blocked slot=%u submits=%u committed=%d packetMirror=%d otMirror=%d allocator=%d",
            static_cast<unsigned>(slot),
            static_cast<unsigned>(apply.appliedSubmitCount),
            commit.committed ? 1 : 0,
            commit.packetWriteMirrorUpdated ? 1 : 0,
            commit.otSlotMirrorUpdated ? 1 : 0,
            commit.allocatorUpdated ? 1 : 0);
        return false;
    }

    s_ss0Direct.scene0SubtitlePacketSubmitCount +=
        static_cast<uint32_t>(apply.appliedSubmitCount);
    if (!s_ss0Direct.scene0SubtitlePacketSubmitLogged ||
        s_ss0Direct.scene0SubtitlePacketLastTextPsxAddr !=
            textRuntime.text268MirrorPsxAddr) {
        s_ss0Direct.scene0SubtitlePacketSubmitLogged = true;
        s_ss0Direct.scene0SubtitlePacketLastTextPsxAddr =
            textRuntime.text268MirrorPsxAddr;
        Log::Printf(
            "SS0 direct runtime: 8001EC54->8001DB00->8001B954->8003FA20 glyph packets committed eventText=%08X slot=%u glyphs=%u totalGlyphs=%u origin=24,184 scale=480 body=264 priority=0 hostProjection=0 oldWinS0Authority=0",
            textRuntime.text268MirrorPsxAddr,
            static_cast<unsigned>(slot),
            static_cast<unsigned>(apply.appliedSubmitCount),
            static_cast<unsigned>(s_ss0Direct.scene0SubtitlePacketSubmitCount));
    }
    return true;
}

static int SS0DirectTickOpeningMovie0(PrGameContext& ctx) {
    if (!s_ss0Direct.strStarted) {
        SS0DirectEnterTitleMovie0T(ctx);
        return 0;
    }

    // COMOD0 801C4678..801C46A0 samples 80035510(1) before the subtitle,
    // MDEC and graph-flip work for this iteration.  Preserve its exact exit
    // set: Select (0x0100) returns one, while Start (0x0800) or Cross
    // (0x0040, via the 0x0840 mask) returns zero.  The caller ignores that
    // return for opening MOVIE0, but the distinction remains observable here
    // and must not be replaced with a generic "any button" shortcut.
    const PrPadState openingMoviePad = SS0DirectReadPad(ctx);
    const uint32_t openingMovieInput80035510 =
        SS0DirectFullLocalPadToPsxMask80026744(
            static_cast<uint16_t>(openingMoviePad.held |
                                  openingMoviePad.pressed));
    if (PrSS0StrLifecycleDirect::IsKnownExitInput80035510(
            openingMovieInput80035510)) {
        const int32_t movieReturn =
            PrSS0StrLifecycleDirect::KnownReturnValueForInput80035510(
                openingMovieInput80035510);
        Log::Printf(
            "SS0 direct runtime: 801C455C MOVIE0 80035510 pad exit mask=%04X return=%d before 80024CF8",
            static_cast<unsigned>(openingMovieInput80035510),
            movieReturn);
        if (!SS0DirectExecuteStrCleanupTail801C455C(
                "801C455C MOVIE0 pad exit")) {
            SS0DirectStopTitleStr(ctx);
            SS0DirectEnterTitleMovie0T(ctx);
            return 0;
        }
        if (!SS0DirectBeginOpeningMovie0PostWaitTransition801C4E70(
                ctx,
                "801C4DC4 pad return")) {
            SS0DirectStopTitleStr(ctx);
            SS0DirectEnterTitleMovie0T(ctx);
        }
        return 0;
    }

    // 801C455C calls 80024CF8 before 801C448C refreshes ctx+4/+6/+7.
    // Consume the prior 801C4350 frame here, then publish the newly sampled
    // frame below for the next outer-loop iteration.
    s_ss0Direct.strSubtitleQueryFrame30 =
        s_ss0Direct.strSubtitleClockFrame30;

    // 801C455C calls 80024CF8 against the previous CD timecode, before
    // 801C448C refreshes ctx+4/+6/+7.  Use the translated VText cursor and
    // countdown here rather than a render-time interval search; this keeps
    // the original one-event-per-logic-tick and expiry semantics intact at
    // both 30 Hz logic and 60 Hz render cadence.
    if (s_ss0Direct.scene0SubtitleDataAvailable) {
        const auto desc = SS0DirectBuildScene0SubtitleDescInput();
        const auto& track = PrStage1MovieTextDirect::GetMovieSubtitleTrack(
            s_ss0Direct.scene0Movie0Vtext);
        const uint32_t cursorBefore =
            s_ss0Direct.scene0SubtitleVtextRuntime.eventCursor;
        PrStage1VTextDirectAdvanceSub80024CF8(
            s_ss0Direct.scene0SubtitleVtextRuntime,
            &track,
            desc,
            PrStage1MovieTextDirect::ResolveLanguageIndex(ctx.languageIndex),
            s_ss0Direct.strSubtitleQueryFrame30);
        const auto& textRuntime = s_ss0Direct.scene0SubtitleVtextRuntime;
        if (textRuntime.eventCursor != cursorBefore &&
            textRuntime.activeTextPtr != nullptr) {
            Log::Printf(
                "SS0 direct runtime: S0 80024CF8 vtext event cursor=%u queryFrame30=%u eventFrame30=%u duration=%u text268=%08X",
                static_cast<unsigned>(textRuntime.eventCursor - 1u),
                static_cast<unsigned>(s_ss0Direct.strSubtitleQueryFrame30),
                static_cast<unsigned>(textRuntime.activeEventFrame),
                static_cast<unsigned>(textRuntime.activeDurationFramesRemaining),
                static_cast<unsigned>(textRuntime.text268MirrorPsxAddr));
        }
    }

    // COMOD0's 801C455C loop calls 8001EC54(a1, 7) after 80024CF8 and
    // before the MDEC/8001ED74/8001ED3C tail.  Keep that ordering in the
    // direct owner as well: the PSX text path clears the current OT lane,
    // seeds the 800901C8 allocator, and emits 8001B954 glyph packets into
    // the same graph work list used by the subsequent 8001ED3C submit.
    // Native late-subtitle drawing remains the host presentation fallback;
    // this packet transaction is the semantic source and makes the S0 path
    // observable in the graph state instead of silently skipping 8001EC54.
    (void)SS0DirectApplyOpeningMovie0TextWindow8001EC54(ctx);

    const bool advanceVideo = SS0DirectVideoFrameDue("801C455C MOVIE0");
    if (advanceVideo) {
        if (!SS0DirectPumpMdecPollPair80027528("801C455C MOVIE0")) {
            SS0DirectStopTitleStr(ctx);
            SS0DirectEnterTitleMovie0T(ctx);
            return 0;
        }
    }
    const auto directTick =
        PrSS0StrLifecycleDirect::StepStrPlayerTick801C448C(
            s_ss0Direct.strWorkBase80049428,
            s_ss0Direct.strLowerCdClock801C4350);
    s_ss0Direct.strPlayerTick801C448C = directTick;
    s_ss0Direct.strClockPoll801C4350 = directTick.clock.poll;
    if (!directTick.known || !directTick.clock.polled) {
        Log::Printf(
            "SS0 direct runtime: 801C448C MOVIE0 direct tick blocked poll=%u workBaseKnown=%d readyKnown=%d cleanupKnown=%d",
            s_ss0Direct.strLowerCdClock801C4350.pollCount,
            s_ss0Direct.strWorkBase80049428.dword80049428Known ? 1 : 0,
            s_ss0Direct.strWorkBase80049428.byte80057119Known ? 1 : 0,
            directTick.cleanup.known ? 1 : 0);
        SS0DirectStopTitleStr(ctx);
        SS0DirectEnterTitleMovie0T(ctx);
        return 0;
    }
    if (directTick.clock.poll.clockFieldsUpdated) {
        const auto& poll = directTick.clock.poll;
        s_ss0Direct.strSubtitleClockFrame30 =
            static_cast<uint32_t>(
                (std::max)(0, static_cast<int>(poll.minuteFieldOffset4))) *
                PrSS0StrLifecycleDirect::kStrClockFramesPerMinute +
            static_cast<uint32_t>(poll.secondFieldOffset6) *
                PrSS0StrLifecycleDirect::kStrClockFramesPerSecond +
            static_cast<uint32_t>(poll.frameFieldOffset7);
    }
    uint32_t xaTargetCursor =
        s_ss0Direct.strDecoderMemoryRuntime80027288.
            sectorRingRawSectorCursor;
    if (directTick.clock.currentLba >=
        s_ss0Direct.strLowerCdStart8001A4D0.startLba) {
        const uint64_t clockCursor =
            static_cast<uint64_t>(directTick.clock.currentLba -
                                  s_ss0Direct.strLowerCdStart8001A4D0.
                                      startLba) +
            1u;
        xaTargetCursor = (std::max)(
            xaTargetCursor,
            static_cast<uint32_t>((std::min<uint64_t>)(
                clockCursor,
                s_ss0Direct.strLowerCdStart8001A4D0.
                    logicalSectorCount)));
    }
    if (!SS0DirectPumpStrXaAudioOutputAdapter8001A4D0(
            xaTargetCursor,
            "801C455C MOVIE0")) {
        Log::Printf(
            "SS0 direct runtime: 801C455C MOVIE0 original-disc XA sector pump blocked");
        SS0DirectStopTitleStr(ctx);
        SS0DirectEnterTitleMovie0T(ctx);
        return 0;
    }
    const auto graphFlip8001ED74 =
        PrSS0TitleTmdBackend::ApplyOpeningMovie0GraphFlip8001ED74();
    s_ss0Direct.openingMovie0GraphFlip8001ED74 = graphFlip8001ED74;
    if (!graphFlip8001ED74.committed) {
        Log::Printf(
            "SS0 direct runtime: 801C455C MOVIE0 8001ED74 graph flip blocked");
        SS0DirectStopTitleStr(ctx);
        SS0DirectEnterTitleMovie0T(ctx);
        return 0;
    }
    if (advanceVideo &&
        !SS0DirectExecuteStrStripUpload8002756C(
            ctx, "801C455C MOVIE0")) {
            SS0DirectStopTitleStr(ctx);
            SS0DirectEnterTitleMovie0T(ctx);
            return 0;
    }
    const auto workListFlush8001ED3C =
        PrSS0TitleTmdBackend::ApplyOpeningMovie0WorkListFlush8001ED3C(
            s_ss0Direct.scene0Gp368WorkSlot8006EDA8,
            s_ss0Direct.openingMovie0WorkListFlush8001ED3C
                .callCount80040CA4);
    s_ss0Direct.openingMovie0WorkListFlush8001ED3C =
        workListFlush8001ED3C;
    if (!s_ss0Direct.scene0Gp368WorkSlot8006EDA8Known ||
        !s_ss0Direct.scene0Gp368LoadedZeroAuthority ||
        !workListFlush8001ED3C.committed) {
        Log::Printf(
            "SS0 direct runtime: 801C455C MOVIE0 8001ED3C work-list flush blocked slotKnown=%u submit=%u software=%u",
            workListFlush8001ED3C.cachedWorkSlotKnown8006EDA8 ? 1u : 0u,
            workListFlush8001ED3C.submitCalled80040CA4 ? 1u : 0u,
            workListFlush8001ED3C.softwareStateCommitted ? 1u : 0u);
        SS0DirectStopTitleStr(ctx);
        SS0DirectEnterTitleMovie0T(ctx);
        return 0;
    }
    const auto postUploadDecision =
        PrSS0StrLifecycleDirect::ResolveStrPostUploadDecision801C455C(
            s_ss0Direct.strWorkBase80049428,
            workListFlush8001ED3C.committed,
            directTick.clock.known,
            !directTick.clock.terminal);
    s_ss0Direct.strPostUploadDecision801C455C = postUploadDecision;
    if (!postUploadDecision.known ||
        !postUploadDecision.exactBranchOrder) {
        Log::Printf(
            "SS0 direct runtime: 801C455C MOVIE0 8001A3B8 post-upload decision blocked known=%u order=%u",
            postUploadDecision.known ? 1u : 0u,
            postUploadDecision.exactBranchOrder ? 1u : 0u);
        SS0DirectStopTitleStr(ctx);
        SS0DirectEnterTitleMovie0T(ctx);
        return 0;
    }
    if (postUploadDecision.specialWorkBaseExit) {
        const auto waitCleanup8001A694 =
            PrSS0StrLifecycleDirect::ExecuteStrWaitCleanup8001A694(
                s_ss0Direct.strDecoderMemoryRuntime80027288,
                s_ss0Direct.strCdSyncCallback800570F8,
                s_ss0Direct.strWorkBase80049428);
        s_ss0Direct.strWaitCleanup8001A694 = waitCleanup8001A694;
        if (!waitCleanup8001A694.known ||
            !waitCleanup8001A694.executionAccepted) {
            Log::Printf(
                "SS0 direct runtime: 801C455C MOVIE0 8001A694 direct command-8 cleanup blocked; fail-closed before 80035838");
            SS0DirectStopTitleStr(ctx);
            SS0DirectEnterTitleMovie0T(ctx);
            return 0;
        }
        const auto stopCallback80035838 =
            PrSS0StrLifecycleDirect::ExecuteStrStopCallback80035838(
                s_ss0Direct.strRootCallback80055F78);
        s_ss0Direct.strStopCallback80035838 = stopCallback80035838;
        if (!stopCallback80035838.known ||
            !stopCallback80035838.executionAccepted ||
            !stopCallback80035838.currentScusSemanticAuthority ||
            !stopCallback80035838.directSoftwareStateAuthority ||
            stopCallback80035838.psxHardwareMmioAuthority ||
            stopCallback80035838.hardwareCallbackTimingAuthority ||
            stopCallback80035838.hostProjection ||
            stopCallback80035838.replayValueAuthority ||
            stopCallback80035838.oldWinS0Authority ||
            stopCallback80035838.stage2PlusAuthority ||
            stopCallback80035838.comod2Authority) {
            Log::Printf(
                "SS0 direct runtime: 801C455C MOVIE0 80035838->80035CF4 root software stop blocked enabledBefore=%u; direct branch not promoted, applying existing safe cleanup adapter",
                stopCallback80035838.enabledBefore ? 1u : 0u);
            SS0DirectStopTitleStr(ctx);
            SS0DirectEnterTitleMovie0T(ctx);
            return 0;
        }
        if (!SS0DirectExecuteStrCleanupTail801C455C(
                "801C455C MOVIE0 special exit")) {
            SS0DirectStopTitleStr(ctx);
            SS0DirectEnterTitleMovie0T(ctx);
            return 0;
        }
        Log::Printf(
            "SS0 direct runtime: 801C455C MOVIE0 8001A694->80035838->80035CF4->80027664->8001A4A4->second 8001A694->80024CF0->8001B120(1) software VRAM move committed rootEnabledBefore=%u rootNoOp=%u savedIMask=%04X savedDICR=%08X return=%08X; actual 80046840 GP1/DMA2 MMIO and asynchronous completion remain open; psxHardwareMmioAuthority=0 hardwareCallbackTimingAuthority=0 psxSpuHardwareAuthority=0 psxGpuMmioAuthority=0 hostProjection=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
            stopCallback80035838.enabledBefore ? 1u : 0u,
            stopCallback80035838.noOpAlreadyDisabled ? 1u : 0u,
            static_cast<unsigned>(
                s_ss0Direct.strRootCallback80055F78.
                    savedInterruptMask80055FAA),
            s_ss0Direct.strRootCallback80055F78.
                savedDmaControl80055FAC,
            stopCallback80035838.returnValue);
        if (!SS0DirectBeginOpeningMovie0PostWaitTransition801C4E70(
                ctx,
                "801C4DC4 special return")) {
            SS0DirectStopTitleStr(ctx);
            SS0DirectEnterTitleMovie0T(ctx);
        }
        return 0;
    }
    Log::Printf(
        "SS0 direct runtime: 801C455C MOVIE0 8001ED74->80040370 and 8001ED3C->80040CA4 committed lane=%u->%u frame=%u->%u flushSlot=%u flushCount=%u hostPresentAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        static_cast<unsigned>(graphFlip8001ED74.drawSlotBefore),
        static_cast<unsigned>(graphFlip8001ED74.drawSlotAfter),
        graphFlip8001ED74.frameCounterBefore8009658C,
        graphFlip8001ED74.frameCounterAfter8009658C,
        static_cast<unsigned>(
            workListFlush8001ED3C.cachedWorkSlot8006EDA8),
        workListFlush8001ED3C.callCount80040CA4);
    if (postUploadDecision.continueLoop) {
        return 0;
    }

    Log::Printf(
        "SS0 direct runtime: 801C455C MOVIE0 ended directTerminal=%u debugSkip=%u currentLba=%d relative=%d deadline=%d endCheck=%u workBase=%d cleanupCommand=%02X discImageDirectoryAuthority=1 directLowerCdStateAuthority=1 hostClockAuthority=0 hostLifecycleAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        directTick.clock.terminal ? 1u : 0u,
        ctx.debugF1_StrSkip ? 1u : 0u,
        directTick.clock.currentLba,
        directTick.clock.relativeClock,
        directTick.clock.deadlineLba,
        directTick.clock.endCheckReturn ? 1u : 0u,
        directTick.dword80049428,
        static_cast<unsigned>(directTick.cleanup.command80036678));
    if (!SS0DirectExecuteStrCleanupTail801C455C(
            "801C455C MOVIE0 terminal")) {
        SS0DirectStopTitleStr(ctx);
        SS0DirectEnterTitleMovie0T(ctx);
        return 0;
    }
    if (!SS0DirectBeginOpeningMovie0PostWaitTransition801C4E70(
            ctx,
            "801C4DC4 terminal return")) {
        SS0DirectStopTitleStr(ctx);
        SS0DirectEnterTitleMovie0T(ctx);
    }
    return 0;
}

static bool SS0DirectEnterNaturalTitleLoop801C49AC() {
    if (!s_ss0Direct.titleMimeInit801C609CReady) {
        Log::Printf(
            "SS0 direct runtime: 801C49AC natural title blocked 801C609C MIME init incomplete");
        return false;
    }
    const auto cleanup =
        PrSS0StrLifecycleDirect::ApplyStrCleanup8001A280(
            s_ss0Direct.strWorkBase80049428);
    if (!cleanup.known || !cleanup.called ||
        !cleanup.dword80049428Known ||
        !cleanup.directCommandSinkAuthority ||
        cleanup.hardwareCallbackAuthority ||
        cleanup.hostProjection ||
        cleanup.replayValueAuthority ||
        cleanup.oldWinS0Authority ||
        cleanup.stage2PlusAuthority ||
        cleanup.comod2Authority) {
        Log::Printf(
            "SS0 direct runtime: 801C49AC 8001A280 cleanup blocked workBaseKnown=%d",
            cleanup.dword80049428Known ? 1 : 0);
        return false;
    }

    const auto source = PrSS0TitleInitialClockDirect::
        BuildKnownStaticScene0Source801C49C0();
    PrSS0TitleInitialClockDirect::Result801C49C0 initialClock{};
    if (!PrSS0TitleInitialClockDirect::
            ResolveStaticHeaderInitialClock801C49C0(source, initialClock) ||
        !initialClock.accepted || !initialClock.complete ||
        !initialClock.initialTick96Known || !initialClock.cursorKnown ||
        !initialClock.hostProjection ||
        !initialClock.staticScene0HeaderAuthority ||
        initialClock.psxMemoryBackingAuthority ||
        initialClock.stage1StateOrHeaderAuthority ||
        initialClock.oldWinS0Authority ||
        initialClock.stage2PlusAuthority ||
        initialClock.replayValueAuthority ||
        initialClock.hostFilesystemAuthority ||
        initialClock.gameLiveProbeAuthority ||
        !initialClock.cleanup8001A280SourceOpen ||
        initialClock.cleanup8001A280Authority ||
        !initialClock.ctx0CWriteIn80026FA4DelaySlot) {
        Log::Printf(
            "SS0 direct runtime: 801C49C0 static header initial clock blocked");
        return false;
    }

    PrSS0TitleInitialClockDirect::CleanupAuthorityInput8001A280
        cleanupBinding{};
    cleanupBinding.known = cleanup.known;
    cleanupBinding.called = cleanup.called;
    cleanupBinding.dword80049428Known =
        cleanup.dword80049428Known;
    cleanupBinding.dword80049428 = cleanup.dword80049428;
    cleanupBinding.skippedNonZeroWorkBase =
        cleanup.skippedNonZeroWorkBase;
    cleanupBinding.commandWrapperCalled80036678 =
        cleanup.commandWrapperCalled80036678;
    cleanupBinding.command80036678 = cleanup.command80036678;
    cleanupBinding.commandWrapperSucceeded80036678 =
        cleanup.commandWrapperSucceeded80036678;
    cleanupBinding.commandSinkCalled800375BC =
        cleanup.commandSinkCalled800375BC;
    cleanupBinding.commandSinkSkipWait800375BC =
        cleanup.commandSinkSkipWait800375BC;
    cleanupBinding.directCommandSinkAuthority =
        cleanup.directCommandSinkAuthority;
    cleanupBinding.hardwareCallbackAuthority =
        cleanup.hardwareCallbackAuthority;
    cleanupBinding.hostProjection = cleanup.hostProjection;
    cleanupBinding.replayValueAuthority =
        cleanup.replayValueAuthority;
    cleanupBinding.oldWinS0Authority = cleanup.oldWinS0Authority;
    cleanupBinding.stage2PlusAuthority = cleanup.stage2PlusAuthority;
    cleanupBinding.comod2Authority = cleanup.comod2Authority;
    if (!PrSS0TitleInitialClockDirect::
            BindCleanupAuthority8001A280(
                cleanupBinding,
                initialClock) ||
        !initialClock.cleanup8001A280Authority ||
        !initialClock.cleanupDword80049428Known ||
        !initialClock.cleanupCommandSinkAuthority800375BC) {
        Log::Printf(
            "SS0 direct runtime: 801C49AC 8001A280 cleanup binding blocked");
        return false;
    }

    s_ss0Direct.titleFrameCounter = 0;
    s_ss0Direct.titleLoopTickV10 = -17;
    s_ss0Direct.titleLoopStateV8 = 0u;
    s_ss0Direct.titleMovie0TPreviousInputPsx = 0u;
    s_ss0Direct.titleNaturalLoopCadence = {};
    s_ss0Direct.titleMovie0TState0GraphFlipBlockedLogged = false;
    s_ss0Direct.titleInitialClock = initialClock;
    s_ss0Direct.titleInitialCtxTick96Known = true;
    s_ss0Direct.titleInitialCtxTick96 = initialClock.initialTick96;
    s_ss0Direct.titleCtxTick96Known = true;
    s_ss0Direct.titleCtxTick96 = initialClock.initialTick96;
    PrSfx::ApplySharedAudioResetBarrier26FA4();
    s_ss0Direct.titleCursor = initialClock.cursor;
    s_ss0Direct.phase = SS0DirectPhase::TitleSelector;
    Log::Printf(
        "SS0 direct runtime: 801C49C0 natural title clock committed sceneBase=8005474C header08=%d header0A=%d ctx0C=%u cleanupWorkBase=%d cleanupCommand=%02X cleanup8001A280Authority=1 cleanup800375BCSinkAuthority=1 audioBarrier=80026FA4 hostProjection=1 psxMemoryBackingAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
        static_cast<int>(initialClock.headerField08),
        static_cast<int>(initialClock.headerField0A),
        static_cast<unsigned>(initialClock.initialTick96),
        initialClock.cleanupDword80049428,
        static_cast<unsigned>(
            initialClock.cleanupCommand80036678));
    return true;
}

static int SS0DirectTickTitleMovie0TFinalReady801C4968(PrGameContext& ctx) {
    if (!s_ss0Direct.strStarted) {
        s_ss0Direct.titleMovie0TFinalReadyPreloopActive = false;
        if (!s_ss0Direct.titleMovie0TFinalReadyBlockedLogged) {
            s_ss0Direct.titleMovie0TFinalReadyBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: title MOVIE0T final-ready blocked direct STR inactive");
        }
        return 0;
    }

    if (s_ss0Direct.titleMovie0TFinalReadyInterPollTicksRemaining > 0u) {
        --s_ss0Direct.titleMovie0TFinalReadyInterPollTicksRemaining;
        return 0;
    }

    const auto& lowerCdStart =
        s_ss0Direct.strLowerCdStart8001A4D0;
    if (!lowerCdStart.initialized ||
        !lowerCdStart.statusPollInputKnown8001A750 ||
        !lowerCdStart.directPsxStatusAuthority ||
        !lowerCdStart.currentScusSemanticAuthority ||
        !lowerCdStart.currentComod0CallerAuthority ||
        !lowerCdStart.discImagePayloadAuthority ||
        lowerCdStart.hardwareCallbackAuthority ||
        lowerCdStart.hostProjection ||
        lowerCdStart.hostExtractedFileAuthority ||
        lowerCdStart.replayValueAuthority ||
        lowerCdStart.oldWinS0Authority ||
        lowerCdStart.stage2PlusAuthority ||
        lowerCdStart.comod2Authority) {
        if (!s_ss0Direct.titleMovie0TFinalReadyBlockedLogged) {
            s_ss0Direct.titleMovie0TFinalReadyBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: title MOVIE0T final-ready direct lower-CD status unavailable");
        }
        return 0;
    }

    PrMovieSegmentDirect::StreamStatusPollInput8001A750 pollInput{};
    pollInput.sub800364D0Known = true;
    pollInput.sub800364D0Result =
        lowerCdStart.syncReturn800364D0;
    pollInput.statusBytesKnown = true;
    pollInput.statusBytes[0] = lowerCdStart.status0;
    pollInput.commandWrapper80036678Known =
        lowerCdStart.primeStatusCalled80036678;
    pollInput.commandWrapper80036678Succeeded =
        lowerCdStart.commandWrapper80036678Succeeded;
    const auto poll =
        PrMovieSegmentDirect::PsxCall8001A750_StreamStatusPoll(pollInput);
    if (!poll.resultKnown || poll.gapMissingSub800364D0Feedback ||
        poll.requestedCommandWrapper80036678 ||
        poll.gapMissingCommandWrapper80036678) {
        if (!s_ss0Direct.titleMovie0TFinalReadyBlockedLogged) {
            s_ss0Direct.titleMovie0TFinalReadyBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: title MOVIE0T final-ready poll blocked sync=%d statusKnown=%d command=%d",
                lowerCdStart.syncReturn800364D0,
                lowerCdStart.statusPollInputKnown8001A750 ? 1 : 0,
                poll.requestedCommandWrapper80036678 ? 1 : 0);
        }
        return 0;
    }

    s_ss0Direct.titleMovie0TFinalReadyBlockedLogged = false;
    if (poll.psxReturn == 1) {
        // 801C4894 leaves the 801C4968 pre-loop only after 8001A750 returns
        // one; the next iteration is the normal v8=0 title loop.  Do not
        // leave the host renderer in the packet-only pre-loop lane.
        s_ss0Direct.titleMovie0TFinalReadyPreloopActive = false;
        s_ss0Direct.titleMovie0TFinalReadyPollsRemaining = 0u;
        s_ss0Direct.titleMovie0TFinalReadyInterPollTicksRemaining = 0u;
        if (!SS0DirectEnterNaturalTitleLoop801C49AC()) {
            return 0;
        }
        Log::Printf(
            "SS0 direct runtime: 801C4968 MOVIE0T final-ready direct lower-CD ready sync=2 status=%02X directPsxStatusAuthority=1 discImagePayloadAuthority=1 hostProjection=0 hardwareCallbackAuthority=0 replayValueAuthority=0 oldWinS0Authority=0 stage2PlusAuthority=0 comod2Authority=0",
            static_cast<unsigned>(lowerCdStart.status0));
        return 0;
    }

    // COMOD0 801C4894 / 801C4968 executes this exact body for every
    // not-ready poll: 801C6410(ctx,0), 80035560(2), then 801C689C(ctx).
    // The host calls Fn2 once per render tick, so the existing one-tick
    // inter-poll hold represents the two-VBlank 80035560 wait.  Commit the
    // title packet frame here and keep MOVIE0T hidden until the poll exits;
    // otherwise the first decoded MDEC frame leaks through the pre-loop.
    s_ss0Direct.titleMovie0TFinalReadyPreloopActive = true;
    if (!SS0DirectTryAdvanceTitleVisualState801C5EF0()) {
        s_ss0Direct.titleMovie0TFinalReadyPreloopActive = false;
        if (!s_ss0Direct.titleMovie0TFinalReadyBlockedLogged) {
            s_ss0Direct.titleMovie0TFinalReadyBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: 801C4968 pre-loop 801C6410 prepare blocked");
        }
        return 0;
    }
    Log::Printf(
        "SS0 direct runtime: 801C4968 pre-loop 801C6410 prepared; render owns 80035560(2)->801C689C pollReturn=%d",
        poll.psxReturn);

    if (s_ss0Direct.titleMovie0TFinalReadyPollsRemaining > 0u) {
        --s_ss0Direct.titleMovie0TFinalReadyPollsRemaining;
    }
    if (s_ss0Direct.titleMovie0TFinalReadyPollsRemaining > 0u) {
        s_ss0Direct.titleMovie0TFinalReadyInterPollTicksRemaining = 1u;
        return 0;
    }

    Log::Printf(
        "SS0 direct runtime: 801C4968 MOVIE0T final-ready timeout after 1800 polls directPsxStatusAuthority=1 hostProjection=0");
    (void)SS0DirectEnterNaturalTitleLoop801C49AC();
    return 0;
}

static void SS0DirectAdvanceTitleMovie0T801C455C(PrGameContext& ctx) {
    if (!s_ss0Direct.strStarted || s_ss0Direct.titleLoopStateV8 != 0u) {
        return;
    }
    // 801C4894's outer loop runs at 30 Hz (80035560(2) = two PSX VBlanks).
    // MOVIE0T's MDEC source is only ~15 fps, so gate the picture/audio
    // progression here while leaving the 30 Hz title model/event loop free
    // to advance camera, MIME poses, and expression state every logic tick.
    if (!SS0DirectVideoFrameDue("801C455C MOVIE0T")) {
        return;
    }
    if (!SS0DirectPumpMdecPollPair80027528("801C455C MOVIE0T")) {
        SS0DirectStopTitleStr(ctx);
        return;
    }
    const uint32_t ringCursor =
        s_ss0Direct.strDecoderMemoryRuntime80027288.
            sectorRingRawSectorCursor;
    const uint32_t logicalSectorCount =
        s_ss0Direct.strLowerCdStart8001A4D0.logicalSectorCount;
    const uint32_t steppedAudioCursor = static_cast<uint32_t>(
        (std::min<uint64_t>)(
            static_cast<uint64_t>(s_ss0Direct.strTitleAudioTargetCursor) +
                PrSS0StrLifecycleDirect::kStrClockSectorScale,
            logicalSectorCount));
    s_ss0Direct.strTitleAudioTargetCursor =
        (std::max)(ringCursor, steppedAudioCursor);
    if (!SS0DirectPumpStrXaAudioOutputAdapter8001A4D0(
            s_ss0Direct.strTitleAudioTargetCursor,
            "801C455C MOVIE0T")) {
        Log::Printf(
            "SS0 direct runtime: 801C455C MOVIE0T original-disc XA sector pump blocked");
        SS0DirectStopTitleStr(ctx);
        return;
    }
    if (!SS0DirectExecuteStrStripUpload8002756C(
            ctx, "801C455C MOVIE0T")) {
        SS0DirectStopTitleStr(ctx);
    }
}

static int SS0DirectTickTitleIntroTransition80020110(PrGameContext& ctx) {
    bool heldComplete = false;
    PrSS0TransitionDirect::SlowTransitionTickResult80020110 tick{};
    if (SS0DirectLoadingMinimumHoldActive(
            SS0DirectLoadingHoldKind::TitleIntro)) {
        if (!SS0DirectLoadingMinimumHoldReady(
                SS0DirectLoadingHoldKind::TitleIntro, ctx.frame)) {
            return 0;
        }
        heldComplete = true;
    } else {
        const auto runtimeBefore = s_ss0Direct.titleIntroTransition;
        tick = SS0DirectTickSlowTransitionForHostLogic(
            s_ss0Direct.titleIntroTransition);
        if (!tick.accepted) {
            if (!s_ss0Direct.titleIntroTransitionPresentBlockedLogged) {
                s_ss0Direct.titleIntroTransitionPresentBlockedLogged = true;
                Log::Printf(
                    "SS0 direct runtime: title intro 80020110 blocked phase=%u",
                    static_cast<unsigned>(tick.phaseBefore));
            }
            return 0;
        }

        if (tick.presentRequired) {
            const auto present =
                PrSS0TitleTmdBackend::ApplySlowTransitionPresent8001EBF4(
                    tick.visualFrame);
            if (!present.committed) {
                s_ss0Direct.titleIntroTransition = runtimeBefore;
                if (!s_ss0Direct.titleIntroTransitionPresentBlockedLogged) {
                    s_ss0Direct.titleIntroTransitionPresentBlockedLogged = true;
                    Log::Printf(
                        "SS0 direct runtime: title intro 80020110 present blocked iteration=%u tail=%d",
                        tick.visualFrame.iteration,
                        tick.visualFrame.tail ? 1 : 0);
                }
                return 0;
            }
            s_ss0Direct.titleIntroTransitionPresentBlockedLogged = false;
        }
        if (tick.visualFrame.known) {
            s_ss0Direct.titleIntroTransitionVisual = tick.visualFrame;
        }
        if (!tick.complete) {
            return 0;
        }
    }

    if (!heldComplete && !SS0DirectFinishLoadingScreenOrHold(
            SS0DirectLoadingHoldKind::TitleIntro,
            ctx.frame)) {
        return 0;
    }

    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.titleIntroTransition);
    s_ss0Direct.titleIntroTransitionVisual =
        PrSS0TransitionDirect::SlowTransitionVisualFrame80020110{};
    s_ss0Direct.titleIntroTransitionPresentBlockedLogged = false;
    s_ss0Direct.phase = SS0DirectPhase::TitleMovie0TFinalReady;
    Log::Printf(
        "SS0 direct runtime: title intro 801C4958/80020110 complete -> MOVIE0T final-ready");
    return 0;
}

static void SS0DirectPublishMainMenuState800264AC(PrGameContext& ctx) {
    s_ss0Direct.mainMenuIndex = s_ss0Direct.mainMenuState.cursor;
    s_ss0Direct.mainMenuRecordsMode =
        s_ss0Direct.mainMenuState.word800916DA;
    const int cursor = s_ss0Direct.mainMenuState.cursor;
    s_ss0Direct.mainMenuSubChoice =
        (cursor >= 0 && cursor < 5)
            ? s_ss0Direct.mainMenuState.itemValue[cursor]
            : -1;
    ctx.transitionStateDA =
        static_cast<int16_t>(s_ss0Direct.mainMenuState.word800916DA);
    ctx.scn0HiliteCursor = s_ss0Direct.mainMenuIndex;
}

static void SS0DirectPublishOptionsState80026910(PrGameContext& ctx) {
    s_ss0Direct.optionsWord800916D8 = s_ss0Direct.optionsState.word800916D8;
    s_ss0Direct.optionsWord800916DC = s_ss0Direct.optionsState.word800916DC;
    SS0DirectPublishOptionsCarrierContextMirrors80026910(ctx);
    ctx.scn0HiliteCursor = s_ss0Direct.optionsState.cursor;
}

static void SS0DirectPublishStageSelectState80025F6C(PrGameContext& ctx) {
    ctx.scn0HiliteCursor = s_ss0Direct.stageSelectState.cursor;
}

static void SS0DirectClearStageSelectState() {
    s_ss0Direct.stageSelectState =
        PrSS0DirectoryDispatcherDirect::StageSelectState80025F6C{};
    s_ss0Direct.stageSelectEntryVisualPending80020568 = false;
    s_ss0Direct.stageSelectWord800916F0Known = false;
    s_ss0Direct.stageSelectWord800916F0 = 0;
    s_ss0Direct.stageSelectWord800916DA = 0;
    s_ss0Direct.stageSelectRenderSourceMismatchLogged = false;
    PrSS0EventFrameLoopDirect::ResetDispatcherTail80026B94(
        s_ss0Direct.stageSelectDispatcherTail80026B94);
    PrSS0EventFrameLoopDirect::ResetEvent2FrameTransaction80026B94(
        s_ss0Direct.stageSelectFrameTransaction80026B94);
    SS0DirectResetStageSelectFrameHostState8001E750();
    s_ss0Direct.stageSelectFrameTailCandidate80026B94 = {};
    s_ss0Direct.stageSelectFrameTailCandidateKnown80026B94 = false;
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.stageSelectPreLoopPadRelease80026B94);
    PrSS0EventFrameLoopDirect::ResetDispatcherPadChange80026744(
        s_ss0Direct.stageSelectPadChange80026744);
    s_ss0Direct.stageSelectPreLoopPadReleaseBlockedLogged = false;
}

static void SS0DirectClearTitleExitWaitState() {
    s_ss0Direct.titlePendingSelectorResult =
        PrSS0TitleHudEventsDirect::kTitleSelectorResultNone801C47EC;
    s_ss0Direct.titlePendingSourceScene = PrSceneId::Scene0;
    s_ss0Direct.titlePendingTargetScene = -1;
    s_ss0Direct.titlePendingMenu = false;
    s_ss0Direct.titleSelectorConfirmationPresentPending = false;
    s_ss0Direct.titleExitWaitPrepareBlockedLogged = false;
    s_ss0Direct.titleExitWaitPresentGate = {};
}

static void SS0DirectEnterMainMenu(PrGameContext& ctx) {
    ctx.stageRunning = false;
    ctx.scn0PanelAnimActive = false;
    ctx.scn0PanelOffsetX = 0.0f;
    ctx.scn0PanelOffsetY = 0.0f;
    s_ss0Direct.phase = SS0DirectPhase::MainMenu;
    s_ss0Direct.transitionReturnPhase = SS0DirectPhase::MainMenu;
    s_ss0Direct.mainMenuIndex = 3;
    s_ss0Direct.mainMenuRecordsMode = 0;
    s_ss0Direct.mainMenuSubChoice = -1;
    PrSS0EventFrameLoopDirect::ResetDispatcherTail80026B94(
        s_ss0Direct.mainMenuDispatcherTail80026B94);
    PrSS0EventFrameLoopDirect::ResetEvent3FrameTransaction80026B94(
        s_ss0Direct.mainMenuFrameTransaction80026B94);
    SS0DirectResetMainMenuFrameHostState8001E750();
    s_ss0Direct.mainMenuFrameTailCandidate80026B94 = {};
    s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94 = false;
    SS0DirectClearOptionsPageStatePreserveCarrier();
    SS0DirectClearStageSelectState();
    SS0DirectClearCardPageState();
    SS0DirectClearHiScorePageState();
    SS0DirectClearPracticeState();
    SS0DirectPublishMainMenuState800264AC(ctx);
    Log::Printf("SS0 direct runtime: MENU -> direct main menu");
    PrSfx::PlayBgm();
}

static bool SS0DirectBeginMainMenuEntryTransition80026C90(
    PrGameContext& ctx) {
    auto mainMenuStateCandidate =
        PrSS0DirectoryDispatcherDirect::InitMainMenuState80026794();
    mainMenuStateCandidate.cursor = 3;
    mainMenuStateCandidate.itemValue[1] = -1;
    mainMenuStateCandidate.itemValue[3] = -1;
    if (!ctx.renderer) {
        Log::Printf(
            "SS0 direct runtime: 80026C90 mode=4 blocked: renderer unavailable");
        return false;
    }
    const auto resourceState = PrSS0TitleTmdBackend::GetResourceState();
    if (!s_residentDirectory &&
        (resourceState.scene0IntRendererAtlasProfile8001AE7C !=
            PrSS0TitleTmdBackend::
                Scene0IntRendererAtlasProfile8001AE7C::
                    Scene0StartupComposite ||
        !resourceState.scene0IntRendererProjectionKnown8001AE7C ||
        !resourceState.
            scene0IntRendererSourcePartialVramAuthority8001AE7C ||
        !resourceState.scene0IntRendererCpuAtlasCommitted8001AE7C ||
        resourceState.scene0IntRendererProjectedTimCount8001AE7C !=
            PrSS0TitleTmdBackend::
                kScene0StartupCompositeTimCount80016B84)) {
        Log::Printf(
            "SS0 direct runtime: 80026C90 mode=4 blocked: startup composite atlas unavailable profile=%u tim=%u",
            static_cast<unsigned>(
                resourceState.scene0IntRendererAtlasProfile8001AE7C),
            static_cast<unsigned>(
                resourceState.
                    scene0IntRendererProjectedTimCount8001AE7C));
        return false;
    }
    if (!SS0DirectPreflightSlowTransitionTextures80020110(
            ctx, 0u, 4, 2, 1)) {
        Log::Printf(
            "SS0 direct runtime: 80026C90 mode=4 blocked: transition texture preflight");
        return false;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (!SS0DirectResolveMainDirectory80021E60(
            ctx, mainMenuStateCandidate, vx, vy, vs, false, 5u, false)) {
        Log::Printf(
            "SS0 direct runtime: 80026C90 mode=4 blocked: null-context directory preflight");
        return false;
    }

    PrSS0TransitionDirect::SlowTransitionRuntime80020110 runtimeCandidate{};
    if (!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
            runtimeCandidate, 0u, 4, 2, 1)) {
        Log::Printf(
            "SS0 direct runtime: 80026C90 mode=4 blocked: transition runtime begin");
        return false;
    }
    const auto visualCandidate =
        PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
            0u, 4, 2, 1, false, 0u);
    if (!visualCandidate.known || visualCandidate.activeCount != 184u) {
        Log::Printf(
            "SS0 direct runtime: 80026C90 mode=4 blocked: transition visual known=%d active=%u",
            visualCandidate.known ? 1 : 0,
            static_cast<unsigned>(visualCandidate.activeCount));
        return false;
    }

    s_ss0Direct.mainMenuState = mainMenuStateCandidate;
    PrSS0EventFrameLoopDirect::ResetEvent3FrameTransaction80026B94(
        s_ss0Direct.mainMenuFrameTransaction80026B94);
    SS0DirectResetMainMenuFrameHostState8001E750();
    s_ss0Direct.mainMenuFrameTailCandidate80026B94 = {};
    s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94 = false;
    s_ss0Direct.mainMenuEntryTransition = runtimeCandidate;
    s_ss0Direct.mainMenuEntryTransitionVisual = visualCandidate;
    s_ss0Direct.mainMenuEntryTransitionPresentBlockedLogged = false;
    Log::Printf(
        "SS0 direct runtime: 80026C90 -> 80020110 null-context mode=4 started");
    return true;
}

static bool SS0DirectQueueMainMenuEntryTransition80026C90(
    PrGameContext& ctx) {
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.mainMenuEntryTransition);
    s_ss0Direct.mainMenuEntryTransitionVisual = {};
    s_ss0Direct.mainMenuEntryTransitionPresentBlockedLogged = false;
    if (!SS0DirectBeginMainMenuEntryTransition80026C90(ctx)) {
        if (s_residentDirectory) {
            Log::Printf("SS0 resident 80015788: mode4 entry waiting for native directory projection");
            return false;
        }
        Log::Printf(
            "SS0 direct runtime: 80026C90 null-context mode=4 start blocked -> title fallback");
        SS0DirectResetToTitle(ctx);
        return false;
    }
    s_ss0Direct.phase = SS0DirectPhase::MainMenuEntryTransition;
    SS0DirectBeginLoadingScreen(
        SS0DirectLoadingHoldKind::MainMenuEntry, ctx.frame);
    return true;
}

static bool SS0DirectStartSceneTransition(PrGameContext& ctx,
                                          int targetScene,
                                          int transitionState) {
    int16_t mode = 0;
    if (!PrSS0TransitionDirect::ResolveSceneEntryLoadingMode801C7284(
            targetScene, mode) ||
        !SS0DirectPreflightSlowTransitionTextures80020110(
            ctx, PrSS0TransitionDirect::kScene0WorkAddress, 2, 1, 2)) {
        return false;
    }
    // 80015788 returns the scene id after the event dispatcher. There is
    // no mode-1 reveal here: 801C7284 resets 8001EF14 and installs the
    // full 192-tile 80015660 Loading page before the destination is ready.
    // Reuse only the translated static full-grid layout, not its animation.
    const auto fullGrid =
        PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
            PrSS0TransitionDirect::kScene0WorkAddress, 2, 1, 2, false, 23u);
    if (!fullGrid.known || fullGrid.activeCount != 192u) {
        return false;
    }
    const auto sourcePhase = s_ss0Direct.phase;
    ctx.transitionState = static_cast<int16_t>(transitionState);
    ctx.sceneExitReason = mode;
    s_ss0Direct.loadingPatternMode80015408 = mode;
    PrSS0TransitionDirect::ResetLoadingPatternState8001EF14(
        s_ss0Direct.loadingPatternRuntime8001EF40);
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.sceneHandoffTransition80020110);
    s_ss0Direct.sceneHandoffTransitionVisual80020110 = fullGrid;
    s_ss0Direct.sceneHandoffTransitionPresentBlockedLogged80020110 = false;
    s_ss0Direct.sceneHandoffRuntimeActive = true;
    s_ss0Direct.sceneHandoffTargetPending = false;
    s_ss0Direct.sceneHandoffTargetScene = targetScene;
    SS0DirectStopTitleStr(ctx);
    // 80015660 does not reset/key-off the menu's 94410 loop.
    if (SS0DirectPhaseIsCardPage(sourcePhase)) {
        SS0DirectClearCardPageState();
    }
    s_ss0Direct.phase = SS0DirectPhase::WaitTransition;
    s_ss0Direct.transitionReturnPhase = sourcePhase;
    SS0DirectBeginLoadingScreen(SS0DirectLoadingHoldKind::SceneHandoff, ctx.frame);
    SS0DirectArmLoadingMinimumHold(SS0DirectLoadingHoldKind::SceneHandoff,
                                  ctx.frame, ctx.frame + kSs0LoadingMinimumLogicTicks);
    Log::Printf(
        "SS0 direct runtime: scene transition target=%d state=%d Loading 801C7284/80015660 mode=%d bgm=%d minTicks=%u sourcePhase=%u",
        targetScene, transitionState, static_cast<int>(mode),
        PrSfx::IsBgmPlaying() ? 1 : 0, kSs0LoadingMinimumLogicTicks,
        static_cast<unsigned>(sourcePhase));
    return true;
}

static int SS0DirectTickSceneHandoffTransition80020110(PrGameContext& ctx) {
    if (!s_ss0Direct.sceneHandoffRuntimeActive ||
        !SS0DirectLoadingMinimumHoldReady(
            SS0DirectLoadingHoldKind::SceneHandoff, ctx.frame)) {
        return 0;
    }
    s_ss0Direct.sceneHandoffRuntimeActive = false;
    s_ss0Direct.sceneHandoffTargetPending = true;
    s_ss0Direct.sceneHandoffTransitionVisual80020110 = {};
    s_ss0Direct.sceneHandoffTransitionPresentBlockedLogged80020110 = false;
    return 0;
}

static PrSS0TransitionDirect::Scene0TitleResultResolveResult801C4DC4
SS0DirectResolveTitleSelectorResult(int selectorResult,
                                    PrSceneId scene) {
    PrSS0TransitionDirect::Scene0TitleResultResolveInput801C4DC4 input{};
    input.selectorResultKnown = true;
    input.selectorResult = selectorResult;
    input.currentSceneKnown = true;
    input.currentScene = static_cast<int>(scene);
    input.previousSceneKnown = true;
    input.previousScene = s_ss0Direct.word800916EE;
    input.injectedRandDrawsKnown =
        selectorResult == static_cast<int>(
                              PrSS0TransitionDirect::
                                  kTitleSelectorResultRandom801C4DC4);

    if (!input.injectedRandDrawsKnown) {
        return PrSS0TransitionDirect::ResolveScene0TitleResult801C4DC4(input);
    }

    int32_t injectedRandDraw = 0;
    input.injectedRandDraws = &injectedRandDraw;
    input.injectedRandDrawCount = 1u;
    for (;;) {
        injectedRandDraw = std::rand();
        const auto result =
            PrSS0TransitionDirect::ResolveScene0TitleResult801C4DC4(input);
        if (result.status !=
            PrSS0TransitionDirect::
                Scene0TitleResultResolveStatus801C4DC4::DrawsExhausted) {
            return result;
        }
    }
}

static int SS0DirectTitleTargetScene(
    const PrSS0TransitionDirect::
        Scene0TitleResultResolveResult801C4DC4& result) {
    if (result.accepted && result.returnSceneKnown &&
        result.returnScene >= 1 && result.returnScene <= 6) {
        return result.returnScene;
    }
    return -1;
}

static bool SS0DirectTitleHandoffTargetInRange(int targetScene) {
    return targetScene >= 1 && targetScene <= 6;
}

static int SS0DirectResolveTitleHandoffTarget(
    const PrSS0TransitionDirect::
        Scene0TitleResultResolveResult801C4DC4& result) {
    const int targetScene = SS0DirectTitleTargetScene(result);
    if (!SS0DirectTitleHandoffTargetInRange(targetScene)) {
        return -1;
    }
    return targetScene;
}

static void SS0DirectPlayTitleSelectorInputCue(uint16_t cueMask) {
    const bool directScene0VabVoice =
        PrSfx::PlayScene0UiCue80025C8CRaw(cueMask);
    Log::Printf(
        "SS0 direct runtime: title selector cue mask=%04X scene0VabVoice=%d",
        static_cast<unsigned>(cueMask),
        directScene0VabVoice ? 1 : 0);
}

static bool SS0DirectTitleSelectorResultKnown801C4DC4(int selectorResult) {
    return selectorResult ==
               PrSS0TitleHudEventsDirect::kTitleSelectorResultMenu801C47EC ||
           selectorResult ==
               PrSS0TitleHudEventsDirect::kTitleSelectorResultStart801C47EC ||
           selectorResult == static_cast<int>(
                                 PrSS0TransitionDirect::
                                 kTitleSelectorResultRandom801C4DC4);
}

static bool SS0DirectResolveTitleAttractTimeoutPolicy801C48C0() {
    PrSS0TitleHudEventsDirect::TitleAttractTimeoutPolicyInput801C48C0 input{};
    input.word800916FCKnown = s_word800916FC.known;
    input.word800916FC = s_word800916FC.value;
    const auto policy = PrSS0TitleHudEventsDirect::
        ResolveTitleAttractTimeoutPolicy801C48C0(input);
    if (!policy.accepted || !policy.timeoutKnown ||
        policy.timeoutFrames == 0u ||
        policy.timeoutFrames >
            static_cast<uint32_t>((std::numeric_limits<int>::max)())) {
        s_ss0Direct.titleAttractTimeoutKnown = false;
        s_ss0Direct.titleAttractTimeoutLimit = 0;
        s_ss0Direct.attractTimer = 0;
        if (!s_ss0Direct.titleAttractTimeoutGapLogged) {
            Log::Printf(
                "SS0 direct runtime: title attract timeout blocked word_800916FC source unknown");
            s_ss0Direct.titleAttractTimeoutGapLogged = true;
        }
        return false;
    }

    s_ss0Direct.titleAttractTimeoutKnown = true;
    s_ss0Direct.titleAttractTimeoutLimit =
        static_cast<int>(policy.timeoutFrames);
    s_ss0Direct.attractTimer = s_ss0Direct.titleAttractTimeoutLimit;
    s_ss0Direct.titleAttractTimeoutGapLogged = false;
    return true;
}

static bool SS0DirectRestartTitleAttractTimer801C4894() {
    if (!s_ss0Direct.titleAttractTimeoutKnown ||
        s_ss0Direct.titleAttractTimeoutLimit <= 0) {
        s_ss0Direct.attractTimer = 0;
        return false;
    }
    s_ss0Direct.attractTimer = s_ss0Direct.titleAttractTimeoutLimit;
    return true;
}

static int SS0DirectArmTitleExitWait(
    int selectorResult,
    PrSceneId sourceScene,
    int targetScene,
    bool menu) {
    if (!SS0DirectTitleSelectorResultKnown801C4DC4(selectorResult) ||
        sourceScene != PrSceneId::Scene0 ||
        menu !=
            (selectorResult ==
             PrSS0TitleHudEventsDirect::kTitleSelectorResultMenu801C47EC)) {
        Log::Printf(
            "SS0 direct runtime: title exit wait arm blocked selector=%d source=%d menu=%d",
            selectorResult,
            static_cast<int>(sourceScene),
            menu ? 1 : 0);
        return 0;
    }

    s_ss0Direct.titlePendingSelectorResult = selectorResult;
    s_ss0Direct.titlePendingSourceScene = sourceScene;
    s_ss0Direct.titlePendingTargetScene = targetScene;
    s_ss0Direct.titlePendingMenu = menu;
    // COMOD0 801C4DC4 calls 80026FA4 after 801C4894 returns.  This arm point
    // is still inside the translated 801C4894-equivalent 15-frame tail, so
    // defer the reset until SS0DirectTickTitleExitWait reaches transitionReady.
    s_ss0Direct.titleExitAudioResetCommitted80026FA4 = false;
    s_ss0Direct.titleLoopStateV8 = 3u;
    s_ss0Direct.titleSelectorConfirmationPresentPending = true;
    s_ss0Direct.titleExitWaitPrepareBlockedLogged = false;
    s_ss0Direct.titleExitWaitPresentGate = {};
    s_ss0Direct.titleExitWaitPresentGate.waitCounter =
        PrSS0TitleHudEventsDirect::
            kTitleSelectorExitWaitInitialCounter801C4894;
    s_ss0Direct.phase = SS0DirectPhase::TitleExitWait;
    Log::Printf(
        "SS0 direct runtime: title exit wait armed selector=%d source=%d menu=%d target=%d frames=%d",
        selectorResult,
        static_cast<int>(sourceScene),
        menu ? 1 : 0,
        targetScene,
        static_cast<int>(s_ss0Direct.titleExitWaitPresentGate.waitCounter));
    return 0;
}

static bool SS0DirectPreflightSlowTransitionTextures80020110(
    PrGameContext& ctx,
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg);

static bool SS0DirectBeginTitleExitTransition80020110(PrGameContext& ctx) {
    if (!SS0DirectPreflightSlowTransitionTextures80020110(
            ctx,
            PrSS0TransitionDirect::kScene0WorkAddress,
            2,
            1,
            2)) {
        return false;
    }
    PrSS0TransitionDirect::SlowTransitionRuntime80020110 candidate{};
    if (!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
            candidate,
            PrSS0TransitionDirect::kScene0WorkAddress,
            2,
            1,
            2)) {
        return false;
    }

    SS0DirectStopTitleStr(ctx);
    s_ss0Direct.titleExitTransition = candidate;
    s_ss0Direct.titleExitTransitionVisual =
        PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
            PrSS0TransitionDirect::kScene0WorkAddress,
            2,
            1,
            2,
            false,
            0u);
    s_ss0Direct.titleExitTransitionPresentBlockedLogged = false;
    s_ss0Direct.phase = SS0DirectPhase::TitleExitTransition;
    SS0DirectBeginLoadingScreen(
        SS0DirectLoadingHoldKind::TitleExit, ctx.frame);
    return true;
}

static bool SS0DirectBeginMainMenuFrameTransaction80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event3FrameClass80026B94 frameClass,
    bool inputClosed,
    int32_t tailFramesRemainingBefore,
    int32_t tailFramesRemainingAfter,
    const PrSS0EventFrameLoopDirect::DispatcherTailState80026B94& tailCandidate);

static int SS0DirectTickTitleExitWait(PrGameContext& ctx) {
    if (s_ss0Direct.titleSelectorConfirmationPresentPending) {
        return 0;
    }
    const auto gate = PrSS0TitleHudEventsDirect::
        TickTitleSelectorExitWaitPresentGate801C4D58(
            s_ss0Direct.titleExitWaitPresentGate);
    if (!gate.accepted) {
        Log::Printf(
            "SS0 direct runtime: title exit wait present gate blocked counter=%u frameWait=%u pending=%u ack=%u status=%u",
            s_ss0Direct.titleExitWaitPresentGate.waitCounter,
            s_ss0Direct.titleExitWaitPresentGate.frameWaitRemaining,
            s_ss0Direct.titleExitWaitPresentGate.presentPending ? 1u : 0u,
            s_ss0Direct.titleExitWaitPresentGate.presentAcknowledged ? 1u : 0u,
            static_cast<unsigned>(gate.status));
        return 0;
    }
    if (gate.preparePresent) {
        if (!SS0DirectTryAdvanceTitleVisualState801C5EF0()) {
            if (!s_ss0Direct.titleExitWaitPrepareBlockedLogged) {
                Log::Printf(
                    "SS0 direct runtime: title exit wait 801C6410 prepare blocked counter=%u",
                    s_ss0Direct.titleExitWaitPresentGate.waitCounter);
                s_ss0Direct.titleExitWaitPrepareBlockedLogged = true;
            }
            return 0;
        }
        s_ss0Direct.titleExitWaitPrepareBlockedLogged = false;
        s_ss0Direct.titleEventRuntimeState.ctxFlags = 0u;
    }
    s_ss0Direct.titleExitWaitPresentGate = gate.nextState;
    ++s_ss0Direct.titleFrameCounter;
    ctx.scn0HiliteCursor = s_ss0Direct.titleCursor;
    if (!gate.transitionReady) {
        return 0;
    }

    if (!s_ss0Direct.titleExitAudioResetCommitted80026FA4) {
        const auto reset = PrSfx::ApplySharedAudioResetBarrier26FA4();
        if (!reset.committed) {
            Log::Printf(
                "SS0 direct runtime: title exit 801C4DC4 post-801C4894 80026FA4 reset blocked");
            return 0;
        }
        s_ss0Direct.titleExitAudioResetCommitted80026FA4 = true;
    }

    const bool deferredAttractTarget =
        s_ss0Direct.titlePendingSelectorResult ==
        static_cast<int>(
            PrSS0TransitionDirect::kTitleSelectorResultRandom801C4DC4);
    const bool deterministicStartTargetMatches =
        s_ss0Direct.titlePendingSelectorResult !=
            PrSS0TitleHudEventsDirect::kTitleSelectorResultStart801C47EC ||
        s_ss0Direct.titlePendingTargetScene ==
            static_cast<int>(s_ss0Direct.titlePendingSourceScene) + 1;
    const bool pendingTargetMatches =
        s_ss0Direct.titlePendingMenu
            ? s_ss0Direct.titlePendingTargetScene == -1
            : (deferredAttractTarget
                   ? s_ss0Direct.titlePendingTargetScene == -1
                   : deterministicStartTargetMatches &&
                         SS0DirectTitleHandoffTargetInRange(
                             s_ss0Direct.titlePendingTargetScene));
    if (!pendingTargetMatches) {
        Log::Printf(
            "SS0 direct runtime: title exit wait target gap selector=%d source=%d target=%d",
            s_ss0Direct.titlePendingSelectorResult,
            static_cast<int>(s_ss0Direct.titlePendingSourceScene),
            s_ss0Direct.titlePendingTargetScene);
        SS0DirectClearTitleExitWaitState();
        s_ss0Direct.titleLoopStateV8 = 2u;
        s_ss0Direct.phase = SS0DirectPhase::TitleSelector;
        return 0;
    }
    if (!SS0DirectBeginTitleExitTransition80020110(ctx)) {
        Log::Printf(
            "SS0 direct runtime: title exit 80020110 start blocked menu=%d target=%d",
            s_ss0Direct.titlePendingMenu ? 1 : 0,
            s_ss0Direct.titlePendingTargetScene);
        return 0;
    }
    Log::Printf(
        "SS0 direct runtime: title exit wait complete -> direct 80020110 menu=%d target=%d",
        s_ss0Direct.titlePendingMenu ? 1 : 0,
        s_ss0Direct.titlePendingTargetScene);
    if (s_ss0Direct.titlePendingMenu) {
        Log::Printf("SS0 direct runtime: title exit wait complete -> MENU");
    } else {
        Log::Printf(
            "SS0 direct runtime: title exit wait complete -> Scene%d",
            s_ss0Direct.titlePendingTargetScene);
    }
    return 0;
}

static int SS0DirectTickTitleExitTransition80020110(PrGameContext& ctx) {
    bool heldComplete = false;
    PrSS0TransitionDirect::SlowTransitionTickResult80020110 tick{};
    if (SS0DirectLoadingMinimumHoldActive(
            SS0DirectLoadingHoldKind::TitleExit)) {
        if (!SS0DirectLoadingMinimumHoldReady(
                SS0DirectLoadingHoldKind::TitleExit, ctx.frame)) {
            return 0;
        }
        heldComplete = true;
    } else {
        const auto runtimeBefore = s_ss0Direct.titleExitTransition;
        tick = SS0DirectTickSlowTransitionForHostLogic(
            s_ss0Direct.titleExitTransition);
        if (!tick.accepted) {
            Log::Printf(
                "SS0 direct runtime: title exit 80020110 blocked phase=%u",
                static_cast<unsigned>(tick.phaseBefore));
            return 0;
        }

        if (tick.presentRequired) {
            const auto present =
                PrSS0TitleTmdBackend::ApplySlowTransitionPresent8001EBF4(
                    tick.visualFrame);
            if (!present.committed) {
                s_ss0Direct.titleExitTransition = runtimeBefore;
                if (!s_ss0Direct.titleExitTransitionPresentBlockedLogged) {
                    s_ss0Direct.titleExitTransitionPresentBlockedLogged = true;
                    Log::Printf(
                        "SS0 direct runtime: title exit 80020110 present blocked iteration=%u tail=%d",
                        tick.visualFrame.iteration,
                        tick.visualFrame.tail ? 1 : 0);
                }
                return 0;
            }
            s_ss0Direct.titleExitTransitionPresentBlockedLogged = false;
        }
        if (tick.visualFrame.known) {
            s_ss0Direct.titleExitTransitionVisual = tick.visualFrame;
        }
        if (!tick.complete) {
            return 0;
        }
    }

    const int selectorResult = s_ss0Direct.titlePendingSelectorResult;
    const PrSceneId sourceScene = s_ss0Direct.titlePendingSourceScene;
    int targetScene = s_ss0Direct.titlePendingTargetScene;
    const bool menu = s_ss0Direct.titlePendingMenu;
    PrSS0TransitionDirect::Scene0TitleResultResolveResult801C4DC4
        titleResult = s_ss0Direct.titleExitLoadingResult801C4DC4;
    if (!heldComplete) {
        // 801C4DC4 performs this reset/result dispatch BEFORE the destination
        // 80015590/80015660 callback. Resolve once, including the RNG branch;
        // the host's one-second visibility floor must not re-run it.
        PrTransition::ResetHoldOverlayState1EF14();
        PrSS0TransitionDirect::ResetLoadingPatternState8001EF14(
            s_ss0Direct.loadingPatternRuntime8001EF40);
        if (selectorResult == static_cast<int>(
                              PrSS0TransitionDirect::
                                  kTitleSelectorResultRandom801C4DC4)) {
            ctx.transitionState = 1;
            titleResult = SS0DirectResolveTitleSelectorResult(selectorResult, sourceScene);
            targetScene = SS0DirectResolveTitleHandoffTarget(titleResult);
        } else {
            titleResult = SS0DirectResolveTitleSelectorResult(selectorResult, sourceScene);
            if (menu) {
                // 80015D18 -> 80015788: reset, start 94410, flush, THEN
                // 80015590(3). Previously the loop started after Loading.
                PrSfx::ApplySharedAudioResetBarrier26FA4();
            }
            if (menu || titleResult.cue94410Required) {
                PrSfx::PlayScene0TransitionCue94410();
            }
            if (menu || titleResult.flush26ECCRequired) {
                PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
            }
            targetScene = menu ? -1 : SS0DirectResolveTitleHandoffTarget(titleResult);
            ctx.transitionState = static_cast<int16_t>(titleResult.word800916D0);
        }
        int16_t mode = PrSS0TransitionDirect::kLoadingPatternMode80015408;
        if (!menu && !PrSS0TransitionDirect::ResolveSceneEntryLoadingMode801C7284(
                         targetScene, mode)) {
            Log::Printf("SS0 direct runtime: Loading mode source rejected scene=%d", targetScene);
            return 0;
        }
        s_ss0Direct.loadingPatternMode80015408 = mode;
        ctx.sceneExitReason = mode;
        s_ss0Direct.titlePendingTargetScene = targetScene;
        s_ss0Direct.titleExitLoadingResult801C4DC4 = titleResult;
        Log::Printf("SS0 direct runtime: Loading owner prepared menu=%d scene=%d mode=%d bgm=%d source=%s",
                    menu ? 1 : 0, targetScene, static_cast<int>(mode),
                    PrSfx::IsBgmPlaying() ? 1 : 0,
                    menu ? "80015788/80015590" : "801C7284/80015660");
        if (!SS0DirectFinishLoadingScreenOrHold(
                SS0DirectLoadingHoldKind::TitleExit, ctx.frame)) {
            return 0;
        }
    }
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.titleExitTransition);
    s_ss0Direct.titleExitTransitionVisual =
        PrSS0TransitionDirect::SlowTransitionVisualFrame80020110{};
    s_ss0Direct.titleExitTransitionPresentBlockedLogged = false;
    SS0DirectClearTitleExitWaitState();

    if (menu) {
        if (SS0DirectQueueMainMenuEntryTransition80026C90(ctx)) {
            Log::Printf(
                "SS0 direct runtime: title exit mode=2 complete -> event3 mode=4 entry");
        } else {
            Log::Printf(
                "SS0 direct runtime: title exit mode=2 complete -> title fallback");
        }
        return titleResult.returnScene;
    }

    s_ss0Direct.phase = SS0DirectPhase::TitleSelector;
    Log::Printf(
        "SS0 direct runtime: title exit 80020110 complete -> Scene%d",
        targetScene);
    return targetScene;
}

static int SS0DirectTickEvent3EntryTransition80020110(
    PrGameContext& ctx) {
    if (!s_ss0Direct.mainMenuEntryTransition.active &&
        !SS0DirectLoadingMinimumHoldActive(
            SS0DirectLoadingHoldKind::MainMenuEntry)) {
        // The normal title->menu path queues this runtime through
        // SS0DirectQueueMainMenuEntryTransition80026C90, which arms the
        // one-second Loading floor.  Keep the direct retry/fallback path
        // equally observable: it can re-enter here with an idle runtime
        // after a transient preflight/present rejection.
        if (!SS0DirectBeginMainMenuEntryTransition80026C90(ctx)) {
            return 0;
        }
        SS0DirectBeginLoadingScreen(
            SS0DirectLoadingHoldKind::MainMenuEntry, ctx.frame);
    }

    bool heldComplete = false;
    PrSS0TransitionDirect::SlowTransitionTickResult80020110 tick{};
    // The one-second Loading floor must not freeze 80020110.  The original
    // role-grid rotation continues to advance while the host-visible floor
    // is active; only the completed tail is held until the floor expires.
    if (!s_ss0Direct.mainMenuEntryTransition.active) {
        if (!SS0DirectLoadingMinimumHoldReady(
                SS0DirectLoadingHoldKind::MainMenuEntry, ctx.frame)) {
            return 0;
        }
        heldComplete = true;
    } else {
        const auto runtimeBefore = s_ss0Direct.mainMenuEntryTransition;
        tick = SS0DirectTickSlowTransitionForHostLogic(
            s_ss0Direct.mainMenuEntryTransition);
        if (!tick.accepted) {
            Log::Printf(
                "SS0 direct runtime: event3 mode=4 80020110 blocked phase=%u",
                static_cast<unsigned>(tick.phaseBefore));
            return 0;
        }
        if (tick.presentRequired) {
            const auto present =
                PrSS0TitleTmdBackend::ApplySlowTransitionPresent8001EBF4(
                    tick.visualFrame);
            if (!present.committed) {
                s_ss0Direct.mainMenuEntryTransition = runtimeBefore;
                if (!s_ss0Direct.mainMenuEntryTransitionPresentBlockedLogged) {
                    s_ss0Direct.mainMenuEntryTransitionPresentBlockedLogged = true;
                    Log::Printf(
                        "SS0 direct runtime: event3 mode=4 present blocked iteration=%u tail=%d",
                        tick.visualFrame.iteration,
                        tick.visualFrame.tail ? 1 : 0);
                }
                return 0;
            }
            s_ss0Direct.mainMenuEntryTransitionPresentBlockedLogged = false;
        }
        if (tick.visualFrame.known) {
            s_ss0Direct.mainMenuEntryTransitionVisual = tick.visualFrame;
        }
        if (!tick.complete) {
            return 0;
        }
    }

    if (!heldComplete && !SS0DirectFinishLoadingScreenOrHold(
            SS0DirectLoadingHoldKind::MainMenuEntry,
            ctx.frame)) {
        return 0;
    }

    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(
        s_ss0Direct.mainMenuEntryTransition);
    s_ss0Direct.mainMenuEntryTransitionVisual = {};
    s_ss0Direct.mainMenuEntryTransitionPresentBlockedLogged = false;
    SS0DirectEnterMainMenu(ctx);
    // COMOD0 80026B94 enters its Event3 loop immediately after the mode-4
    // 80020110 tail.  Prime that first 8001E750/80021E60 directory frame in
    // the same logic turn; waiting for the next tick exposes the host clear
    // colour as a one-frame black flash on the menu boundary.
    const auto firstMenuTail = s_ss0Direct.mainMenuDispatcherTail80026B94;
    (void)SS0DirectBeginMainMenuFrameTransaction80026B94(
        ctx,
        PrSS0EventFrameLoopDirect::Event3FrameClass80026B94::OpenInputLoop,
        false,
        -1,
        -1,
        firstMenuTail);
    Log::Printf(
        "SS0 direct runtime: event3 mode=4 complete -> main menu input");
    return 0;
}

static bool SS0DirectEnterOptions(PrGameContext& ctx) {
    SS0DirectClearCardPageState();
    SS0DirectClearOptionsPageStatePreserveCarrier();
    if (!ctx.renderer) {
        Log::Printf(
            "SS0 direct runtime: main menu OPTION blocked before page preflight: renderer missing");
        return false;
    }
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (!SS0DirectResolveOptions80021910(ctx, vx, vy, vs, false)) {
        Log::Printf(
            "SS0 direct runtime: main menu OPTION blocked by full 8001D74C/80021910 resource preflight");
        return false;
    }
    s_ss0Direct.phase = SS0DirectPhase::Options;
    SS0DirectPublishOptionsState80026910(ctx);
    Log::Printf("SS0 direct runtime: main menu OPTION -> options");
    return true;
}

static bool SS0DirectEnterStageSelect(PrGameContext& ctx) {
    PrSS0DirectoryDispatcherDirect::StageSelectInitInput800267F8 input{};
    const bool directStatusKnown =
        SS0DirectReadStageStatusFromDirectBank(input.status80092F1DTo23);
    if (!directStatusKnown) {
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "STAGE STATUS GAP - O:BACK");
        Log::Printf(
            "SS0 direct runtime: main menu STAGE blocked: "
            "direct status bank unknown");
        return false;
    }
    input.word800916DA = s_ss0Direct.mainMenuState.word800916DA;
    uint16_t softwareWord800916F0 = 0u;
    const bool word800916F0Known =
        TryReadWord800916F0Software(softwareWord800916F0);
    if (!word800916F0Known) {
        Log::Printf(
            "SS0 direct runtime: main menu STAGE word_800916F0 source unknown; "
            "using direct status fallback da=%d statusSource=%s",
            input.word800916DA,
            "direct-bank");
    }
    input.word800916F0Known = word800916F0Known;
    char word800916F0LogValue[16] = "unknown";
    if (word800916F0Known) {
        input.word800916F0 = static_cast<int32_t>(softwareWord800916F0);
        std::snprintf(word800916F0LogValue,
                      sizeof(word800916F0LogValue),
                      "%d",
                      input.word800916F0);
    }
    SS0DirectClearCardPageState();
    SS0DirectPublishMainMenuState800264AC(ctx);
    SS0DirectClearStageSelectState();
    s_ss0Direct.stageSelectWord800916F0Known = word800916F0Known;
    s_ss0Direct.stageSelectWord800916F0 = input.word800916F0;
    s_ss0Direct.stageSelectWord800916DA = input.word800916DA;
    s_ss0Direct.stageSelectState =
        PrSS0DirectoryDispatcherDirect::InitStageSelect800267F8(input);
    s_ss0Direct.stageSelectBlink = 0;
    if (!ctx.renderer) {
        SS0DirectClearStageSelectState();
        Log::Printf(
            "SS0 direct runtime: main menu STAGE blocked before page preflight: renderer missing");
        return false;
    }
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (!SS0DirectResolveStageSelect80020568(ctx, vx, vy, vs, false)) {
        SS0DirectClearStageSelectState();
        Log::Printf(
            "SS0 direct runtime: main menu STAGE blocked by full 8001D74C/80020568 resource preflight");
        return false;
    }
    if (!word800916F0Known) {
        Log::Printf(
            "SS0 direct runtime: stage select cursor7 BONUS held closed: "
            "word_800916F0 source unknown");
    }
    s_ss0Direct.phase = SS0DirectPhase::StageSelect;
    s_ss0Direct.stageSelectEntryVisualPending80020568 = true;
    SS0DirectPublishStageSelectState80025F6C(ctx);
    Log::Printf(
        "SS0 direct runtime: main menu STAGE -> stage select cursor=%d f0Known=%d f0=%s da=%d statusSource=%s",
        s_ss0Direct.stageSelectState.cursor,
        input.word800916F0Known ? 1 : 0,
        word800916F0LogValue,
        input.word800916DA,
        "direct-bank");
    return true;
}

static bool SS0DirectCopyRawDirectoryRowName8007A318(
    char (&dst)[CardHandoff::kCardDirectoryNameMax80017900 + 1u],
    const uint8_t* row) {
    if (row == nullptr || row[0] == 0) {
        return false;
    }
    if (std::memcmp(row,
                    kCardFilenamePrefix80019D7C,
                    kCardFilenamePrefixBytes80019D7C) != 0) {
        return false;
    }

    std::memset(dst, 0, sizeof(dst));
    bool terminated = false;
    for (std::size_t i = 0;
         i < kSaveUiDirectoryRawNameBytes8007A318 && i + 1u < sizeof(dst);
         ++i) {
        dst[i] = static_cast<char>(row[i]);
        if (row[i] == 0) {
            terminated = true;
            break;
        }
    }
    if (!terminated &&
        kSaveUiDirectoryRawNameBytes8007A318 < sizeof(dst)) {
        dst[kSaveUiDirectoryRawNameBytes8007A318] = '\0';
        terminated = true;
    }
    return terminated && dst[0] != '\0';
}

static bool SS0DirectPublishLoadReplayDirectoryFromDirectCardImageSink(
    CardHandoff::CardMode800191E4 mode) {
    if (mode != CardHandoff::CardMode800191E4::Load &&
        mode != CardHandoff::CardMode800191E4::Replay) {
        return false;
    }

    const PrStage1SaveUiCardImagePersistenceView8007A318 sink =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    if (!sink.known || !sink.slotPolicyKnown || sink.blockIndex < 0 ||
        sink.blockIndex >= 15 || sink.bytes == nullptr ||
        sink.byteCount != kDirectCardImageBytes8007A318 ||
        sink.byteSize != kDirectCardImageBytes8007A318) {
        Log::Printf(
            "SS0 direct runtime: %s direct card-image directory rehydrate skipped sink=%d slot=%d block=%d bytes=%zu/%u",
            CardHandoff::CardMode800191E4Name(mode),
            sink.known ? 1 : 0,
            sink.slotPolicyKnown ? 1 : 0,
            sink.blockIndex,
            sink.byteCount,
            sink.byteSize);
        return false;
    }

    // The startup storage binder imports the durable image into the direct
    // card-image sink, but it deliberately does not mutate the Scene1 SaveUi
    // directory bank.  Rehydrate that bank here, at the same semantic
    // boundary as the original 80017594 directory read, before producing the
    // 80019D7C Load/Replay carrier.  Do not infer a directory from the slot
    // policy block index: a card image can contain several valid frames.
    const PrStage1SaveUiDirectCardLoadResult80017594 loaded =
        PrStage1SaveUiDirect::LoadSaveUiDirectCardImageDirectory80017594();
    if (!loaded.directoryFramesKnown || !loaded.directoryLoaded) {
        Log::Printf(
            "SS0 direct runtime: %s direct card-image directory rehydrate skipped directoryFrames=%d loaded=%d activeRows=%d",
            CardHandoff::CardMode800191E4Name(mode),
            loaded.directoryFramesKnown ? 1 : 0,
            loaded.directoryLoaded ? 1 : 0,
            loaded.activeRows);
        return false;
    }

    const PrStage1SaveUiDirectoryRawBankView8007A318 raw =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
    if (!raw.known || raw.bytes == nullptr ||
        raw.byteCount < kSaveUiDirectoryRawBankBytes8007A318 ||
        raw.byteSize < kSaveUiDirectoryRawBankBytes8007A318) {
        Log::Printf(
            "SS0 direct runtime: %s direct card-image directory rehydrate skipped rawBank=%d bytes=%zu/%u",
            CardHandoff::CardMode800191E4Name(mode),
            raw.known ? 1 : 0,
            raw.byteCount,
            raw.byteSize);
        return false;
    }

    CardHandoff::LoadReplayDirectoryScanFacts80019D7C facts{};
    facts.known = true;
    facts.mode = mode;
    facts.directoryRowsKnown80017B08 = true;
    facts.snapshotKnown80017B18 = true;
    facts.listRowsBuilt80019D7C = true;
    facts.entryCountKnown = true;
    facts.entryCount = 0;
    for (int32_t rowIndex = 0; rowIndex < 15; ++rowIndex) {
        const uint8_t* row =
            raw.bytes + static_cast<std::size_t>(rowIndex) *
                            kSaveUiDirectoryRawRowBytes8007A318;
        if (row[0] == 0u) {
            continue;
        }
        if (facts.entryCount >= 15) {
            Log::Printf(
                "SS0 direct runtime: %s direct card-image directory rehydrate rejected entryCount overflow",
                CardHandoff::CardMode800191E4Name(mode));
            return false;
        }
        char rowName[CardHandoff::kCardDirectoryNameMax80017900 + 1u]{};
        if (!SS0DirectCopyRawDirectoryRowName8007A318(rowName, row)) {
            // An active frame with a malformed game filename is not a free
            // slot.  Keep the original fail-closed rule instead of silently
            // dropping it and presenting an incomplete directory.
            Log::Printf(
                "SS0 direct runtime: %s direct card-image directory rehydrate rejected row=%d rowName8007A590=unknown",
                CardHandoff::CardMode800191E4Name(mode),
                rowIndex);
            return false;
        }
        const int32_t entryIndex = facts.entryCount++;
        facts.rows[entryIndex].blockIndexKnown = true;
        facts.rows[entryIndex].blockIndex = rowIndex;
        facts.rows[entryIndex].rowNameKnown8007A590 = true;
        std::snprintf(facts.rows[entryIndex].rowName8007A590,
                      sizeof(facts.rows[entryIndex].rowName8007A590),
                      "%s",
                      rowName);
        facts.rows[entryIndex].titleKnown = true;
        std::snprintf(facts.rows[entryIndex].title,
                      sizeof(facts.rows[entryIndex].title),
                      "%s",
                      rowName + kCardFilenamePrefixBytes80019D7C);
    }

    CardHandoff::LoadReplayDirectoryTypedCarrier80019D7C carrier{};
    if (!CardHandoff::BuildRuntimeLoadReplayDirectoryCarrierFromScanFacts80019D7C(
            facts,
            &carrier) ||
        !CardHandoff::PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(
            carrier)) {
        CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
        Log::Printf(
            "SS0 direct runtime: %s direct card-image directory rehydrate rejected block=%d",
            CardHandoff::CardMode800191E4Name(mode),
            sink.blockIndex);
        return false;
    }

    Log::Printf(
        "SS0 direct runtime: %s direct card-image directory rehydrated "
        "entries=%d firstBlock=%d source=direct-card-image "
        "durableKnown=%d durableCommitted=%d",
        CardHandoff::CardMode800191E4Name(mode),
        facts.entryCount,
        facts.entryCount > 0 ? facts.rows[0].blockIndex : -1,
        sink.durablePolicyKnown ? 1 : 0,
        sink.durableCommitted ? 1 : 0);
    return true;
}

static bool SS0DirectPublishSelectedState16FromDirectCardImageSink(
    int32_t selectedBlock) {
    PrStage1SaveUiCardImagePersistenceView8007A318 sink =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    if (!sink.known || !sink.slotPolicyKnown || sink.bytes == nullptr ||
        sink.byteCount != kDirectCardImageBytes8007A318 ||
        sink.byteSize != kDirectCardImageBytes8007A318 ||
        selectedBlock < 0 || selectedBlock >= 15) {
        return false;
    }

    // The HAL helper's blockIndex field is the physical directory frame being
    // read.  At Scene0 entry the persistence sink's blockIndex is only the
    // host slot policy (normally 0), so pass a value-normalized view for the
    // selected directory frame while retaining the same immutable image
    // bytes and durable provenance.
    sink.blockIndex = selectedBlock;
    const bool published =
        PrStage1SaveCardHalDirect::
            PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
                sink, selectedBlock);
    Log::Printf(
        "SS0 direct runtime: selected direct card-image state16 source block=%d published=%d",
        selectedBlock,
        published ? 1 : 0);
    return published;
}

static bool SS0DirectPublishCardGridDrawState80020F94(
    CardHandoff::CardMode800191E4 mode,
    const CardHandoff::LoadReplayDirectoryTypedCarrier80019D7C& directory) {
    namespace Directory = PrSS0DirectoryPagesRenderDirect;
    s_ss0Direct.cardGridDrawState80020F94 = {};
    s_ss0Direct.cardListInputRuntime800181D0 = {};
    s_ss0Direct.cardDriverVisualRuntime80018FB0 = {};
    if (!directory.entryCountKnown || directory.entryCount < 0 ||
        directory.entryCount >
            static_cast<int32_t>(Directory::kCardGridItemCapacity80020F94) ||
        (mode != CardHandoff::CardMode800191E4::Load &&
         mode != CardHandoff::CardMode800191E4::Replay)) {
        return false;
    }

    CardHandoff::CardDriverVisualRuntime80018FB0 driver{};
    if (!CardHandoff::InitLoadReplayCardDriverVisualRuntime80018FB0(
            mode, &driver)) {
        return false;
    }
    CardHandoff::LoadReplayListInputRuntime800181D0 listInput{};
    if (!CardHandoff::InitLoadReplayListInputRuntime800181D0(
            mode, directory.entryCount, &listInput)) {
        return false;
    }

    Directory::CardGridState80020F94 candidate{};
    candidate.requestBound = true;
    candidate.argAddress = Directory::kCardGridArgAddress80048E50;
    candidate.eventId = mode == CardHandoff::CardMode800191E4::Replay ? 9 : 8;
    candidate.language = s_ss0Direct.optionsWord800916D8;
    candidate.exitFrameState = driver.exitFrameStateArg0;
    candidate.exitBlinkState = driver.exitBlinkStateArg4;
    candidate.cardIoFlag = driver.cardIoFlagArg8;
    candidate.rows = 5;
    candidate.columns = 3;
    candidate.itemCount = static_cast<int16_t>(directory.entryCount);
    candidate.selected = static_cast<int16_t>(listInput.selected);
    for (int32_t i = 0; i < directory.entryCount; ++i) {
        candidate.enabled[static_cast<std::size_t>(i)] = 1;
        std::snprintf(
            candidate.slotText[static_cast<std::size_t>(i)].data(),
            candidate.slotText[static_cast<std::size_t>(i)].size(),
            "%s",
            directory.rows[i].title);
    }
    const Directory::CardGridDrawList80020F94 preflight =
        Directory::BuildCardGridDrawList80020F94(candidate);
    if (!preflight.sourceKnown || !preflight.accepted ||
        !preflight.complete || !preflight.runtimeSubmitAllowed ||
        !preflight.rawTextureOnly || preflight.truncated) {
        return false;
    }
    s_ss0Direct.cardDriverVisualRuntime80018FB0 = driver;
    s_ss0Direct.cardListInputRuntime800181D0 = listInput;
    s_ss0Direct.cardGridDrawState80020F94 = candidate;
    s_ss0Direct.cardCursor = listInput.selected;
    return true;
}

static bool SS0DirectRefreshCardEntries(
    CardHandoff::CardMode800191E4 mode,
    const char* emptyMessage,
    bool publishCardGridVisualAuthority) {
    SS0DirectClearTypedCardReadCarriers();
    SS0DirectClearCardDirectoryAuthority();
    SS0DirectClearReplayRawSelectorFixture();
    s_ss0Direct.cardEntryCount = 0;
    s_ss0Direct.cardCursor = 0;
    std::fill(std::begin(s_ss0Direct.cardBlockIndex),
              std::end(s_ss0Direct.cardBlockIndex),
              -1);
    for (int i = 0; i < 15; ++i) {
        s_ss0Direct.cardTitle[i][0] = '\0';
    }
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "%s",
                  emptyMessage ? emptyMessage
                               : "CARD DIRECTORY GAP - O:BACK");
    CardHandoff::LoadReplayDirectoryTypedCarrier80019D7C directory{};
    bool directoryFromProducer =
        CardHandoff::GetLoadReplayDirectoryTypedCarrier80019D7C(
            mode, &directory);
    if (!directoryFromProducer) {
        CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
        directoryFromProducer =
            SS0DirectPublishLoadReplayDirectoryFromDirectCardImageSink(mode) &&
            CardHandoff::GetLoadReplayDirectoryTypedCarrier80019D7C(
                mode, &directory);
        if (!directoryFromProducer) {
            // A card with no saved slots is still a valid directory page.  The
            // translated menu must expose LOAD/REPLAY and let the page show
            // its empty grid; only an actual row selection needs card
            // payload authority.  Keep this carrier explicitly typed so the
            // normal 80019D7C -> 80020F94 path remains in use.
            directory = {};
            directory.known = true;
            directory.source =
                CardHandoff::LoadReplayDirectoryTypedCarrierSource80019D7C::
                    RuntimeDirectoryProducer;
            directory.mode = mode;
            directory.entryCountKnown = true;
            directory.entryCount = 0;
            directory.producerWired80017B08_80017B18_80019D7C = true;
            Log::Printf(
                "SS0 direct runtime: %s directory source empty -> exposing typed empty page",
                CardHandoff::CardMode800191E4Name(mode));
        }
    }
    if (!directory.known ||
        directory.mode != mode ||
        !directory.entryCountKnown) {
            Log::Printf(
                "SS0 direct runtime: %s directory carrier missing after empty-page fallback",
                CardHandoff::CardMode800191E4Name(mode));
            return false;
    }
    CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
    if (!directory.entryCountKnown ||
        directory.entryCount < 0 ||
        directory.entryCount > 15) {
        Log::Printf(
            "SS0 direct runtime: %s directory carrier rejected: entryCountKnown=%d entryCount=%d",
            CardHandoff::CardMode800191E4Name(mode),
            directory.entryCountKnown ? 1 : 0,
            directory.entryCount);
        return false;
    }
    for (int i = 0; i < directory.entryCount; ++i) {
        if (!directory.rows[i].blockIndexKnown ||
            directory.rows[i].blockIndex < 0 ||
            directory.rows[i].blockIndex >= 15 ||
            !directory.rows[i].titleKnown) {
            Log::Printf(
                "SS0 direct runtime: %s directory carrier row rejected: row=%d blockKnown=%d block=%d titleKnown=%d",
                CardHandoff::CardMode800191E4Name(mode),
                i,
                directory.rows[i].blockIndexKnown ? 1 : 0,
                directory.rows[i].blockIndex,
                directory.rows[i].titleKnown ? 1 : 0);
            return false;
        }
    }
    for (int i = 0; i < directory.entryCount; ++i) {
        s_ss0Direct.cardBlockIndex[i] = directory.rows[i].blockIndex;
        std::snprintf(s_ss0Direct.cardTitle[i],
                      sizeof(s_ss0Direct.cardTitle[i]),
                      "%s",
                      directory.rows[i].title);
    }
    s_ss0Direct.cardEntryCount = directory.entryCount;
    s_ss0Direct.cardCursor = 0;
    s_ss0Direct.cardDirectoryAuthorityKnown = true;
    s_ss0Direct.cardDirectoryMode = mode;
    s_ss0Direct.cardDirectoryCarrier80019D7C = directory;
    const bool cardGridPublished = publishCardGridVisualAuthority &&
        SS0DirectPublishCardGridDrawState80020F94(mode, directory);
    if (publishCardGridVisualAuthority && !cardGridPublished) {
        Log::Printf(
            "SS0 direct runtime: %s directory accepted but 80048E50/80020F94 visual preflight rejected entries=%d",
            CardHandoff::CardMode800191E4Name(mode),
            directory.entryCount);
        return false;
    }
    s_ss0Direct.cardMessage[0] = '\0';
    Log::Printf(
        "SS0 direct runtime: %s directory carrier accepted entries=%d directoryGeneration=%u cardGrid80020F94=%d",
        CardHandoff::CardMode800191E4Name(mode),
        s_ss0Direct.cardEntryCount,
        static_cast<unsigned>(s_ss0Direct.cardDirectoryGeneration),
        cardGridPublished ? 1 : 0);
    return true;
}

static auto SS0DirectResolveCardGrid80020F94(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    int32_t eventId,
    bool submit) -> bool;

static void SS0DirectPracticeStartRoundIntro();

static void SS0DirectPracticeClearOverlayGate8002776C() {
    s_ss0Direct.practiceFlags00 = 0u;
}

static void SS0DirectPracticeSetPadStop(int frames, int kind) {
    s_ss0Direct.practicePadStopFrames = frames;
    s_ss0Direct.practicePadStopKind = frames > 0 ? kind : -1;
    if (frames > 0 && kind >= 0 && kind <= 8 && kind != 3) {
        s_ss0Direct.practiceFlags00 = 0x00400000u;
        s_ss0Direct.practiceOverlayState1C = kind;
    }
}

static void SS0DirectClearPracticeState() {
    s_ss0Direct.practiceFrame = 0;
    s_ss0Direct.practiceProducerLastLogicFrame = -1;
    s_ss0Direct.practicePhase = SS0DirectPracticePhase::Idle;
    s_ss0Direct.practiceRound = 0;
    s_ss0Direct.practiceRoundFrame = -1;
    s_ss0Direct.practiceFlags00 = 0u;
    s_ss0Direct.practiceOverlayState1C = 0;
    SS0DirectPracticeSetPadStop(0, -1);
    s_ss0Direct.practiceExitConfirmFrames = 0;
    s_ss0Direct.practiceExitSelection48 = 0;
    s_ss0Direct.practiceExitBlink4C = 0;
    s_ss0Direct.practiceExitConfirmRuntime = {};
    s_ss0Direct.practiceTick = 0;
    s_ss0Direct.practicePendingJudgeKind = -1;
    s_ss0Direct.practicePendingRound = -1;
    s_ss0Direct.practiceSeqCurA = -1;
    s_ss0Direct.practiceSeqCurB = -1;
    s_ss0Direct.practiceSeqEnA = 0;
    s_ss0Direct.practiceSeqEnB = 0;
    PrSS0DirectoryPagesRenderDirect::ResetPracticeWobbleBank80024308(
        s_ss0Direct.practiceWobbleBank);
    s_ss0Direct.practiceGuideDrawList = {};
    s_ss0Direct.practiceSliceDrawList = {};
    s_ss0Direct.practicePostSliceTailDrawList = {};
    s_ss0Direct.practiceLeadingOverlayDrawList = {};
    // 80023618 scans the note stream and calls 80024418 even during the
    // RoundIntro/Preview boundary, before either Rec44 lane has an active
    // producer.  The PSX textured sprite's neutral modulation is 0x80 per
    // channel; treating this as an unknown RGB word rejected every icon
    // candidate and made the complete 18-note row disappear.  Keep the
    // pure helper fail-closed for unbound callers, but seed the SS0 runtime
    // with the source-neutral value used by the actual 80024418 draw.
    s_ss0Direct.practiceIconRgbKnown = true;
    s_ss0Direct.practiceIconR = kPracticeIconNeutralRgb80024418;
    s_ss0Direct.practiceIconG = kPracticeIconNeutralRgb80024418;
    s_ss0Direct.practiceIconB = kPracticeIconNeutralRgb80024418;
    s_ss0Direct.practiceIconStreamKnown = false;
    s_ss0Direct.practiceIconCandidates = {};
    PrSS0DirectoryPagesRenderDirect::ResetPracticePortraitBank80024600(
        s_ss0Direct.practicePortraitBank);
    s_ss0Direct.practicePortraitTeacher = {};
    s_ss0Direct.practicePortraitStudent = {};
    s_ss0Direct.practiceScore = 0;
    s_ss0Direct.practiceCombo = 0;
    s_ss0Direct.practiceHits = 0;
    s_ss0Direct.practiceMisses = 0;
    s_ss0Direct.practiceLastJudge = 9;
    s_ss0Direct.practiceLastDelta = 0;
    s_ss0Direct.practiceJudgeFlash = 0;
    s_ss0Direct.practiceTargetResolved = false;
    s_ss0Direct.practiceComplete = false;
}

static void SS0DirectClearPracticeYCompoResources80015618() {
    s_ss0Direct.practiceYCompoIntLoad80015618 = {};
    s_ss0Direct.practiceYCompoIntSideEffect8001A8F0 = {};
    s_ss0Direct.practiceYCompoLoaderMemoryGateCommitted = false;
    s_ss0Direct.practiceYCompoPadStartComSource = {};
    s_ss0Direct.practiceYCompoIntSpu8001A8F0 = {};
    s_ss0Direct.practiceYCompoIntGpu8001AE7C = {};
    s_ss0Direct.practiceYCompoIntRenderer8001AE7C = {};
    s_ss0Direct.practiceYCompoResourcesCommitted = false;
}

static bool SS0DirectPracticeYCompoResourcesCommitted80015618() {
    const auto resourceState = PrSS0TitleTmdBackend::GetResourceState();
    return s_ss0Direct.practiceYCompoResourcesCommitted &&
        PrSS0Scene0IntLoadDirect::
            IsExactAcceptedYCompoTransaction80015618(
                s_ss0Direct.practiceYCompoIntLoad80015618) &&
        s_ss0Direct.practiceYCompoLoaderMemoryGateCommitted &&
        PrSS0Scene0IntSideEffectDirect::
            IsExactAcceptedYCompoTransaction8001A8F0(
                s_ss0Direct.practiceYCompoIntSideEffect8001A8F0,
                s_ss0Direct.practiceYCompoIntLoad80015618) &&
        PrSS0Scene0IntSpuDirect::
            IsExactCommittedYCompoTransaction8001A8F0(
                s_ss0Direct.practiceYCompoIntSpu8001A8F0,
                s_ss0Direct.practiceYCompoIntLoad80015618,
                s_ss0Direct.practiceYCompoIntSideEffect8001A8F0,
                s_ss0Direct.practiceYCompoPadStartComSource) &&
        PrSS0Scene0IntGpuDirect::
            IsExactCommittedYCompoTransaction8001AE7C(
                s_ss0Direct.practiceYCompoIntGpu8001AE7C,
                s_ss0Direct.practiceYCompoIntLoad80015618,
                s_ss0Direct.practiceYCompoIntSideEffect8001A8F0) &&
        PrSS0Scene0IntRendererDirect::
            IsExactCommittedYCompoTransaction8001AE7C(
                s_ss0Direct.practiceYCompoIntRenderer8001AE7C,
                s_ss0Direct.practiceYCompoIntLoad80015618,
                s_ss0Direct.practiceYCompoIntSideEffect8001A8F0,
                s_ss0Direct.practiceYCompoIntGpu8001AE7C) &&
        resourceState.scene0IntRendererAtlasProfile8001AE7C ==
            PrSS0TitleTmdBackend::
                Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo &&
        resourceState.scene0IntRendererCpuAtlasCommitted8001AE7C &&
        resourceState.scene0IntRendererD3DUploadCommitted8001AE7C &&
        resourceState.scene0IntRendererProjectedTpageCount8001AE7C > 0u &&
        resourceState.scene0IntRendererD3DFailedTpageCount8001AE7C == 0u &&
        resourceState.scene0IntRendererD3DReadyTpageCount8001AE7C ==
            resourceState.scene0IntRendererProjectedTpageCount8001AE7C;
}

static bool SS0DirectTryCommitPracticeYCompoResources80015618(
    PrGameContext& ctx) {
    if (SS0DirectPracticeYCompoResourcesCommitted80015618()) {
        PrSfx::StopBgm();
        return true;
    }

    const bool ingressReady =
        PrSS0Scene0ResourceIngressDirect::
            IsExactAcceptedTransaction801C4780(
                s_ss0Direct.scene0ResourceIngress801C4780);
    PrMovieSegmentDirect::MovieSegmentRecord48 yCompoRow{};
    const bool yCompoRowReady =
        ingressReady &&
        PrSS0Scene0ResourceIngressDirect::
            BuildPracticeYCompoRow80015618(
                s_ss0Direct.scene0ResourceIngress801C4780,
                yCompoRow);
    PrSS0Scene0IntLoadDirect::YCompoSource80015618 intLoadSource{};
    intLoadSource.ingressKnown = true;
    intLoadSource.ingressAccepted = ingressReady;
    intLoadSource.discBinPathKnown =
        s_ss0Direct.scene0ResourceIngress801C4780.discBinPathKnown;
    intLoadSource.discBinPath =
        s_ss0Direct.scene0ResourceIngress801C4780.discBinPath;
    intLoadSource.yCompoRowKnown = yCompoRowReady;
    intLoadSource.yCompoRow = yCompoRow;
    auto intLoad =
        PrSS0Scene0IntLoadDirect::BuildYCompoTransaction80015618(
            intLoadSource);
    const bool intLoadReady =
        PrSS0Scene0IntLoadDirect::
            IsExactAcceptedYCompoTransaction80015618(intLoad);

    PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0 intSideEffect{};
    if (intLoadReady && ctx.mainSceneLoaderMemoryDirect != nullptr) {
        auto loaderMemoryCandidate =
            std::make_unique<PrStage1LoaderMemoryDirectState>(
                *ctx.mainSceneLoaderMemoryDirect);
        if (PrSS0TitleTmdBackend::
                ApplyScene0LoaderResetPreservingTitlePacketArena80025A34(
                    *loaderMemoryCandidate)) {
            intSideEffect = PrSS0Scene0IntSideEffectDirect::
                BuildYCompoTransaction8001A8F0(
                    intLoad, std::move(loaderMemoryCandidate));
        }
    }
    const bool intSideEffectReady =
        PrSS0Scene0IntSideEffectDirect::
            IsExactAcceptedYCompoTransaction8001A8F0(
                intSideEffect, intLoad);

    PrSS0Scene0IntSpuDirect::PadStartComSource80026E4C
        padStartComSource{};
    padStartComSource.staticBodyKnown = true;
    padStartComSource.cuePointerKnown =
        PrSS0Scene0GlobalBindingsDirect::IsExactState801C5B14(
            s_ss0Direct.scene0GlobalBindings);
    padStartComSource.cuePointer80094410 =
        s_ss0Direct.scene0GlobalBindings.slot80094410;
    auto intSpu = PrSS0Scene0IntSpuDirect::BuildYCompoTransaction8001A8F0(
        intLoad, intSideEffect, padStartComSource);
    const bool intSpuReady =
        PrSS0Scene0IntSpuDirect::
            IsExactAcceptedYCompoTransaction8001A8F0(
                intSpu, intLoad, intSideEffect, padStartComSource);
    PrSfx::Scene0IntVabCandidate8001A8F0 intSpuHostCandidate{};
    if (intSpuReady) {
        intSpuHostCandidate = PrSfx::BuildScene0IntVabCandidate8001A8F0(
            intSpu.vhBytes.data(),
            intSpu.vhBytes.size(),
            intSpu.vbBytes.data(),
            intSpu.vbBytes.size());
    }
    const bool intSpuHostCandidateReady =
        intSpuReady &&
        PrSfx::IsExactScene0IntVabCandidate8001A8F0(
            intSpuHostCandidate,
            intSpu.vhBytes.size(),
            intSpu.vhHash,
            intSpu.vbBytes.size(),
            intSpu.vbHash,
            intSpu.decoderToneSlotCount,
            intSpu.decoderEffectiveVagCount);
    auto intSpuCommitted = intSpu;
    const bool intSpuCommitReady =
        intSpuHostCandidateReady &&
        PrSS0Scene0IntSpuDirect::
            CommitYCompoPadStartComGlobalWrites80026E4C(
                intSpuCommitted,
                intLoad,
                intSideEffect,
                padStartComSource) &&
        PrSS0Scene0IntSpuDirect::
            IsExactYCompoPadStartComGlobalWritesCommittedTransaction80026E4C(
                intSpuCommitted,
                intLoad,
                intSideEffect,
                padStartComSource);
    intSpuCommitted.hostVabCandidatePrepared = intSpuHostCandidateReady;
    const bool intSpuHostCommitReady =
        intSpuCommitReady &&
        PrSS0Scene0IntSpuDirect::CommitHostVabBank8001A8F0(
            intSpuCommitted) &&
        PrSS0Scene0IntSpuDirect::
            IsExactCommittedYCompoTransaction8001A8F0(
                intSpuCommitted,
                intLoad,
                intSideEffect,
                padStartComSource);

    auto intGpu = PrSS0Scene0IntGpuDirect::
        BuildYCompoTransaction8001AE7C(intLoad, intSideEffect);
    const bool intGpuReady =
        PrSS0Scene0IntGpuDirect::
            CommitPreparedYCompoTransaction8001AE7C(
                intGpu, intLoad, intSideEffect) &&
        PrSS0Scene0IntGpuDirect::
            IsExactCommittedYCompoTransaction8001AE7C(
                intGpu, intLoad, intSideEffect);
    auto intRenderer = PrSS0Scene0IntRendererDirect::
        BuildYCompoTransaction8001AE7C(
            intLoad, intSideEffect, intGpu);
    const bool intRendererReady =
        PrSS0Scene0IntRendererDirect::
            IsExactPreparedYCompoTransaction8001AE7C(
                intRenderer, intLoad, intSideEffect, intGpu);
    auto intRendererAtlasCandidate = PrSS0TitleTmdBackend::
        BuildPracticeYCompoRendererAtlasCandidate8001AE7C(
            intRenderer,
            intLoad,
            s_ss0Direct.startupCommonIntLoad80016B84,
            s_ss0Direct.startupZCompoIntLoad80016B84,
            s_ss0Direct.scene0IntLoad8001AC18);
    const bool intRendererAtlasPartialVramReady =
        intRendererAtlasCandidate.prepared &&
        PrSS0TitleTmdBackend::ApplyRendererAtlasPartialVram8001AE7C(
            intRendererAtlasCandidate, intGpu);
    const bool intRendererAtlasReady = PrSS0TitleTmdBackend::
        IsExactPracticeYCompoRendererAtlasCandidate8001AE7C(
            intRendererAtlasCandidate, intRenderer);
    const bool intRendererD3DReady =
        intRendererAtlasReady && ctx.renderer != nullptr &&
        PrSS0TitleTmdBackend::
            PreparePracticeYCompoRendererAtlasD3DCandidate8001AE7C(
                intRendererAtlasCandidate,
                intRenderer,
                ctx.renderer) &&
        PrSS0TitleTmdBackend::
            IsExactPracticeYCompoRendererAtlasD3DCandidate8001AE7C(
                intRendererAtlasCandidate,
                intRenderer);
    const bool intLoaderMemoryCommitReady =
        intRendererD3DReady &&
        PrSS0Scene0IntSideEffectDirect::
            CommitYCompoLoaderMemory8001A8F0(
                intSideEffect, intLoad) &&
        PrSS0Scene0IntSideEffectDirect::
            IsExactYCompoLoaderMemoryCommittedTransaction8001A8F0(
                intSideEffect, intLoad);
    if (intLoaderMemoryCommitReady) {
        intSideEffect.loaderMemoryCommitted = false;
    }
    const bool intSideEffectPublishSourceReady =
        PrSS0Scene0IntSideEffectDirect::
            IsExactAcceptedYCompoTransaction8001A8F0(
                intSideEffect, intLoad);

    if (!ingressReady || !yCompoRowReady || !intLoadReady ||
        !intSideEffectReady ||
        !intSpuReady || !intSpuHostCandidateReady ||
        !intSpuHostCommitReady || !intGpuReady || !intRendererReady ||
        !intRendererAtlasPartialVramReady || !intRendererAtlasReady ||
        !intRendererD3DReady ||
        !intLoaderMemoryCommitReady ||
        !intSideEffectPublishSourceReady ||
        intSideEffect.loaderMemoryCandidate == nullptr ||
        ctx.mainSceneLoaderMemoryDirect == nullptr) {
        Log::Printf(
            "SS0 direct runtime: Practice 80015618 YCOMPO commit blocked ingress=%d row=%d int=%d loader=%d spu=%d hostVab=%d gpu=%d renderer=%d partialAtlas=%d atlas=%d d3d=%d loaderCommit=%d",
            ingressReady ? 1 : 0,
            yCompoRowReady ? 1 : 0,
            intLoadReady ? 1 : 0,
            intSideEffectReady ? 1 : 0,
            intSpuReady ? 1 : 0,
            intSpuHostCommitReady ? 1 : 0,
            intGpuReady ? 1 : 0,
            intRendererReady ? 1 : 0,
            intRendererAtlasPartialVramReady ? 1 : 0,
            intRendererAtlasReady ? 1 : 0,
            intRendererD3DReady ? 1 : 0,
            intLoaderMemoryCommitReady ? 1 : 0);
        return false;
    }

    // All rejecting work is complete. These publications are local moves or
    // exact-shape commits and cannot expose a partial Practice phase.
    (void)PrSfx::PlayScene0ConfirmCue80025C8C();
    PrSfx::StopBgm();
    if (!PrSS0TitleTmdBackend::
            CommitPracticeYCompoRendererAtlasCandidate8001AE7C(
                std::move(intRendererAtlasCandidate),
                intRenderer,
                intLoad,
                intSideEffect,
                intGpu)) {
        Log::Printf(
            "SS0 direct runtime: Practice 80015618 YCOMPO atlas commit blocked");
        return false;
    }
    const auto practiceVabClose80027120 =
        PrSfx::ApplyScene0VabClose80027120();
    if (!practiceVabClose80027120.committed ||
        practiceVabClose80027120.function != 0x80027120u ||
        !practiceVabClose80027120.hostProjectionCleared ||
        practiceVabClose80027120.psxSpuRegisterAuthority ||
        practiceVabClose80027120.psxInterruptTimingAuthority) {
        Log::Printf(
            "SS0 direct runtime: Practice 80015618 80027120 close barrier blocked");
        return false;
    }
    PrSfx::CommitScene0IntVabCandidate8001A8F0(
        std::move(intSpuHostCandidate));
    *ctx.mainSceneLoaderMemoryDirect =
        *intSideEffect.loaderMemoryCandidate;
    s_ss0Direct.practiceYCompoIntLoad80015618 = std::move(intLoad);
    s_ss0Direct.practiceYCompoIntSideEffect8001A8F0 =
        std::move(intSideEffect);
    s_ss0Direct.practiceYCompoLoaderMemoryGateCommitted = true;
    s_ss0Direct.practiceYCompoPadStartComSource = padStartComSource;
    s_ss0Direct.practiceYCompoIntSpu8001A8F0 =
        std::move(intSpuCommitted);
    s_ss0Direct.practiceYCompoIntGpu8001AE7C = std::move(intGpu);
    s_ss0Direct.practiceYCompoIntRenderer8001AE7C =
        std::move(intRenderer);
    s_ss0Direct.practiceYCompoResourcesCommitted = true;
    Log::Printf(
        "SS0 direct runtime: Practice 80015618 YCOMPO committed entries=%u tim=%u loaderMemory=1 memHandles=0 partialVram=1 hostVabBank=1 rendererAtlasCpu=1 rendererAtlasD3D=1 lowerSpu=0 audible=0",
        s_ss0Direct.practiceYCompoIntLoad80015618.entryCount,
        static_cast<unsigned>(s_ss0Direct
            .practiceYCompoIntSideEffect8001A8F0.timUploadRequests.size()));
    return true;
}

static bool SS0DirectRestoreScene0ResourcesAfterPractice80015590(
    PrGameContext& ctx) {
    if (!s_ss0Direct.practiceYCompoResourcesCommitted) {
        return true;
    }
    if (!SS0DirectTryCommitScene0Init801C4260(ctx, true)) {
        Log::Printf(
            "SS0 direct runtime: Practice 80015590 previous Scene0 resource restore blocked");
        return false;
    }
    SS0DirectClearPracticeYCompoResources80015618();
    Log::Printf(
        "SS0 direct runtime: Practice 80015590 previous Scene0 resources restored via startup composite transaction");
    return true;
}

static bool SS0DirectEnterPractice(PrGameContext& ctx) {
    if (!SS0DirectTryCommitPracticeYCompoResources80015618(ctx)) {
        Log::Printf(
            "SS0 direct runtime: main menu PRACTICE blocked before YCOMPO resource publication");
        return false;
    }
    SS0DirectClearCardPageState();
    s_ss0Direct.phase = SS0DirectPhase::Practice;
    SS0DirectClearPracticeState();
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "LISTEN");
    Log::Printf("SS0 direct runtime: main menu PRACTICE -> direct practice page");
    SS0DirectPracticeStartRoundIntro();
    return true;
}

static bool SS0DirectPublishHiScoreCase17FromDirectCardImageSink() {
    const PrStage1SaveUiCardImagePersistenceView8007A318 sink =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    if (!sink.known || !sink.slotPolicyKnown || sink.blockIndex < 0 ||
        sink.blockIndex >= 15 || sink.bytes == nullptr ||
        sink.byteCount != kDirectCardImageBytes8007A318 ||
        sink.byteSize != kDirectCardImageBytes8007A318) {
        Log::Printf(
            "SS0 direct runtime: HI-SCORE direct card-image Case17 source unavailable sink=%d slot=%d block=%d bytes=%zu/%u",
            sink.known ? 1 : 0,
            sink.slotPolicyKnown ? 1 : 0,
            sink.blockIndex,
            sink.byteCount,
            sink.byteSize);
        return false;
    }

    const bool published =
        PrStage1SaveCardHalDirect::
            PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
                sink,
                sink.blockIndex);
    PrStage1SaveCardHalDirect::Case17CardReadTypedCarrier800179B4
        publishedCarrier{};
    const bool publishedCarrierKnown =
        published &&
        PrStage1SaveCardHalDirect::GetCase17CardReadTypedCarrier800179B4(
            &publishedCarrier);
    const int32_t compactEntryCount =
        publishedCarrierKnown && publishedCarrier.feedback.word8007ABE4Known
            ? publishedCarrier.feedback.word8007ABE4
            : -1;
    Log::Printf(
        "SS0 direct runtime: HI-SCORE direct card-image Case17 source "
        "published=%d entries=%d completionKnown=%d block=%d "
        "durableKnown=%d durableCommitted=%d",
        published ? 1 : 0,
        compactEntryCount,
        publishedCarrierKnown &&
                publishedCarrier.case17LoopCompletionKnown80019D7C
            ? 1
            : 0,
        sink.blockIndex,
        sink.durablePolicyKnown ? 1 : 0,
        sink.durableCommitted ? 1 : 0);
    return published;
}

static bool SS0DirectEnterHiScore(PrGameContext& ctx) {
    SS0DirectClearCardPageState();
    SS0DirectClearHiScorePageState();
    const bool probeOnlyEntryFixture =
        s_ss0Direct.debugHiScoreEvent6EntryFixtureArmed;
    s_ss0Direct.debugHiScoreEvent6EntryFixtureArmed = false;
    bool emptyScoreBaseline = false;
    if (!probeOnlyEntryFixture &&
        !SS0DirectPublishHiScoreCase17FromDirectCardImageSink()) {
        // With no durable card image, 80019414/80019284 still owns a valid
        // empty score table.  Build that zeroed table from the translated
        // status bank so the HI-SCORE page is enterable; actual saved scores
        // continue to come from Case17 when a card source is present.
        if (!SS0DirectBuildProbeOnlyHiScoreEvent6EntryTable80019284()) {
            Log::Printf(
                "SS0 direct runtime: HI-SCORE entry blocked: empty score table baseline unavailable");
            return false;
        }
        emptyScoreBaseline = true;
        Log::Printf(
            "SS0 direct runtime: HI-SCORE no durable card -> exposing translated empty score table");
    }
    const bool tableAuthorityReady =
        (probeOnlyEntryFixture || emptyScoreBaseline)
            ? true
            : SS0DirectBuildHiScoreEvent6TableCarrier();
    if (!tableAuthorityReady) {
        Log::Printf(
            probeOnlyEntryFixture
                ? "SS0 direct runtime: probe-only Event6 entry table gap nonAuthority=1"
                : "SS0 direct runtime: HI-SCORE table authority gap");
        PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
        SS0DirectClearHiScorePageState();
        return false;
    }

    PrSS0DirectoryDispatcherDirect::HiScoreEvent6InitInput800267E4
        event6Init{};
    event6Init.ctxKnown =
        s_ss0Direct.hiScoreEvent6TableCarrierKnown;
    event6Init.ctxAddress = s_ss0Direct.hiScoreEvent6TablePsxAddress;
    // 80019284 leaves ctx+0 untouched. Preserve the translated overlay-local
    // blink word across Event6 re-entry; 800267E4 clears only ctx+4.
    event6Init.exitIconStateCtx00 = s_ss0Direct.hiScoreBlink;
    s_ss0Direct.hiScoreEvent6State800267E4 =
        PrSS0DirectoryDispatcherDirect::InitHiScoreEvent6State800267E4(
            event6Init);
    if (!s_ss0Direct.hiScoreEvent6State800267E4.requestBound) {
        Log::Printf(
            "SS0 direct runtime: HI-SCORE event6 800267E4 ctx bind rejected known=%d addr=%08X expected=%08X",
            event6Init.ctxKnown ? 1 : 0,
            event6Init.ctxAddress,
            PrSS0DirectoryDispatcherDirect::kEvent6HiScoreCtx80049278);
        SS0DirectClearHiScorePageState();
        return false;
    }
    SS0DirectPublishHiScoreEvent6Ctx80049278();
    const auto drawList = SS0DirectBuildHiScoreTableDrawList80021594();
    if (!drawList.accepted || !drawList.complete ||
        !drawList.rawTextureOnly ||
        drawList.count < PrSS0HiScoreRenderDirect::
                             kHiScoreSpriteCommandCount80021594 ||
        drawList.count > PrSS0HiScoreRenderDirect::
                             kHiScoreSpriteCommandCapacity80021594 ||
        !ctx.renderer) {
        Log::Printf(
            "SS0 direct runtime: HI-SCORE entry blocked before page preflight tablePsx=%08X tableBytes=%zu sprites=%zu",
            s_ss0Direct.hiScoreEvent6TablePsxAddress,
            s_ss0Direct.hiScoreEvent6TableByteCount,
            drawList.count);
        SS0DirectClearHiScorePageState();
        return false;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (!SS0DirectResolveHiScoreTable80021594(
            ctx, vx, vy, vs, false)) {
        SS0DirectClearHiScorePageState();
        Log::Printf(
            "SS0 direct runtime: HI-SCORE entry blocked by full 8001D74C/80021594 resource preflight");
        return false;
    }

    s_ss0Direct.phase = SS0DirectPhase::HiScore;
    Log::Printf(
        "SS0 direct runtime: main menu HI-SCORE -> direct event6 records page ctx=%08X icon=%d label=%d tableBytes=%zu sprites=%zu nonEmptyCells=%zu glyphSprites=%zu",
        s_ss0Direct.hiScoreEvent6TablePsxAddress,
        s_ss0Direct.hiScoreEvent6State800267E4.exitIconStateCtx00,
        s_ss0Direct.hiScoreEvent6State800267E4.exitLabelStateCtx04,
        s_ss0Direct.hiScoreEvent6TableByteCount,
        drawList.count,
        drawList.nonEmptyCellCount,
        drawList.glyphSpriteCount);
    return true;
}

static bool SS0DirectEnterReplayCard(PrGameContext& ctx) {
    SS0DirectClearCardPageState(false);
    if (!SS0DirectRefreshCardEntries(
            CardHandoff::CardMode800191E4::Replay,
            "REPLAY DIRECTORY GAP - O:BACK",
            true) ||
        !ctx.renderer) {
        Log::Printf(
            "SS0 direct runtime: main menu REPLAY blocked before typed directory page preflight");
        SS0DirectClearCardPageState();
        return false;
    }
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (!SS0DirectResolveCardGrid80020F94(ctx, vx, vy, vs, 9, false)) {
        Log::Printf(
            "SS0 direct runtime: main menu REPLAY blocked by full 8001D74C/80020F94 resource preflight");
        SS0DirectClearCardPageState();
        return false;
    }
    // 80015700 is required only when a concrete replay row is committed.  A
    // fresh install with an empty card directory must still be able to open
    // the REPLAY page and show its empty grid.
    const bool backupKnown =
        SS0DirectBackupReplayPayload80015700("replay card page");
    s_ss0Direct.phase = SS0DirectPhase::ReplayCard;
    s_ss0Direct.replayPayloadBackupPreflightKnown = backupKnown;
    if (!backupKnown) {
        Log::Printf(
            "SS0 direct runtime: REPLAY page opened without 80015700 backup; row commit remains gated");
    }
    Log::Printf("SS0 direct runtime: main menu REPLAY -> direct card page entries=%d backup=%d",
                s_ss0Direct.cardEntryCount,
                s_ss0Direct.replayPayloadBackupPreflightKnown ? 1 : 0);
    return true;
}

static bool SS0DirectEnterLoadCard(PrGameContext& ctx) {
    SS0DirectClearCardPageState(false);
    if (!SS0DirectRefreshCardEntries(
            CardHandoff::CardMode800191E4::Load,
            "LOAD DIRECTORY GAP - O:BACK",
            true) ||
        !ctx.renderer) {
        Log::Printf(
            "SS0 direct runtime: main menu LOAD blocked before typed directory page preflight");
        SS0DirectClearCardPageState();
        return false;
    }
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (!SS0DirectResolveCardGrid80020F94(ctx, vx, vy, vs, 8, false)) {
        Log::Printf(
            "SS0 direct runtime: main menu LOAD blocked by full 8001D74C/80020F94 resource preflight");
        SS0DirectClearCardPageState();
        return false;
    }
    s_ss0Direct.phase = SS0DirectPhase::LoadCard;
    s_ss0Direct.replayPayloadBackupPreflightKnown = false;
    Log::Printf("SS0 direct runtime: main menu LOAD -> direct card page entries=%d",
                s_ss0Direct.cardEntryCount);
    return true;
}

static void SS0DirectPlayMainMenuInputCue800264AC(uint32_t cueCode80025C8C) {
    const bool directScene0VabVoice =
        PrSfx::PlayScene0UiCue80025C8CRaw(
            static_cast<uint16_t>(cueCode80025C8C));
    Log::Printf(
        "SS0 direct runtime: event3 800264AC cue=%04X scene0VabVoice=%d",
        static_cast<unsigned>(cueCode80025C8C),
        directScene0VabVoice ? 1 : 0);
}

static void SS0DirectPlayStageSelectInputCue80025F6C(
    uint32_t cueCode80025C8C) {
    const bool directScene0VabVoice =
        PrSfx::PlayScene0UiCue80025C8CRaw(
            static_cast<uint16_t>(cueCode80025C8C));
    Log::Printf(
        "SS0 direct runtime: event2 80025F6C cue=%04X scene0VabVoice=%d",
        static_cast<unsigned>(cueCode80025C8C),
        directScene0VabVoice ? 1 : 0);
}

static void SS0DirectPlayOptionsInputCue80026910(
    const PrGameContext& ctx,
    PrSS0DirectoryDispatcherDirect::OptionsCueKind80026910 cueKind,
    uint32_t cueCode80025C8C) {
    bool directScene0VabVoice = false;
    const char* owner = "none";
    switch (cueKind) {
    case PrSS0DirectoryDispatcherDirect::OptionsCueKind80026910::
        Input80025C8C:
        owner = "80025C8C";
        directScene0VabVoice = PrSfx::PlayScene0UiCue80025C8CRaw(
            static_cast<uint16_t>(cueCode80025C8C));
        break;
    case PrSS0DirectoryDispatcherDirect::OptionsCueKind80026910::
        Language80025DBC:
        owner = "80025DBC";
        directScene0VabVoice =
            PrSfx::PlayScene0OptionsLanguageCue80025DBCRaw();
        break;
    case PrSS0DirectoryDispatcherDirect::OptionsCueKind80026910::None:
    default:
        return;
    }
    Log::Printf(
        "SS0 direct runtime: event17 80026910 cue=%s code=%04X scene0VabVoice=%d frame=%u",
        owner,
        static_cast<unsigned>(cueCode80025C8C),
        directScene0VabVoice ? 1 : 0,
        static_cast<unsigned>(ctx.frame));
}

static void SS0DirectBeginHiScoreOuterPadRelease80015788(
    const char* reason) {
    PrSS0EventFrameLoopDirect::ResetEvent6FrameTransaction80026B94(
        s_ss0Direct.hiScoreFrameTransaction80026B94);
    s_ss0Direct.hiScoreOuterPadReleaseActive80015788 = true;
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.hiScoreOuterPadRelease80015788);
    s_ss0Direct.hiScoreOuterPadReleaseBlockedLogged = false;
    Log::Printf(
        "SS0 direct runtime: 80015788 post-HI-SCORE outer pad release armed reason=%s",
        reason ? reason : "unknown");
}

static int SS0DirectTickHiScoreOuterPadRelease80015788(
    PrGameContext& ctx) {
    if (!s_ss0Direct.hiScoreOuterPadReleaseActive80015788) {
        return 0;
    }

    const PrPadState pad = SS0DirectReadPad(ctx);
    const uint32_t currentPad =
        SS0DirectFullLocalPadToPsxMask80026744(
            static_cast<uint16_t>(pad.held | pad.pressed));
    const auto releaseStep =
        PrSS0EventFrameLoopDirect::StepDispatcherPreLoopPadRelease80026B94(
            s_ss0Direct.hiScoreOuterPadRelease80015788,
            currentPad);
    if (releaseStep.kind == PrSS0EventFrameLoopDirect::
                                DispatcherPreLoopPadReleaseStepKind80026B94::
                                    WaitingForRelease) {
        if (!s_ss0Direct.hiScoreOuterPadReleaseBlockedLogged) {
            s_ss0Direct.hiScoreOuterPadReleaseBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: 80015788 post-HI-SCORE waiting pad release pad=%04X frame=%u",
                static_cast<unsigned>(currentPad),
                static_cast<unsigned>(ctx.frame));
        }
        return 0;
    }

    s_ss0Direct.hiScoreOuterPadReleaseActive80015788 = false;
    s_ss0Direct.hiScoreOuterPadReleaseBlockedLogged = false;
    if (SS0DirectQueueMainMenuEntryTransition80026C90(ctx)) {
        Log::Printf(
            "SS0 direct runtime: 80015788 post-HI-SCORE pad released -> event3 entry frame=%u",
            static_cast<unsigned>(ctx.frame));
    } else {
        Log::Printf(
            "SS0 direct runtime: 80015788 post-HI-SCORE event3 entry blocked -> title fallback");
    }
    return 0;
}

static void SS0DirectConsumeMainMenuResult(PrGameContext& ctx, int result) {
    const auto consumed =
        PrSS0DirectoryDispatcherDirect::ConsumeMainMenuResult80015788(result);
    switch (consumed.action) {
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::ContinueLoop:
        break;
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::HiScoreRecordsPage:
        SS0DirectPublishMainMenuState800264AC(ctx);
        if (!SS0DirectEnterHiScore(ctx)) {
            SS0DirectBeginHiScoreOuterPadRelease80015788(
                "80019414 returned zero/direct authority gap");
            (void)SS0DirectTickHiScoreOuterPadRelease80015788(ctx);
        }
        break;
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::ReplayLoad:
        SS0DirectPublishMainMenuState800264AC(ctx);
        if (SS0DirectEnterReplayCard(ctx)) {
            Log::Printf(
                "SS0 direct runtime: main menu REPLAY -> direct card d0=%d",
                consumed.word800916D0);
        }
        break;
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::Practice:
        SS0DirectPublishMainMenuState800264AC(ctx);
        (void)SS0DirectEnterPractice(ctx);
        break;
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::StageSelect:
        (void)SS0DirectEnterStageSelect(ctx);
        break;
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::LoadCard:
        SS0DirectPublishMainMenuState800264AC(ctx);
        (void)SS0DirectEnterLoadCard(ctx);
        break;
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::ReturnScene0:
        if (s_residentDirectory) {
            s_residentDirectory->result = 0;
            Log::Printf("SS0 resident 80015788: EXIT returns scene=0");
            break;
        }
        SS0DirectEnterTitleMovie0T(ctx);
        Log::Printf("SS0 direct runtime: main menu EXIT -> title");
        break;
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::Options:
        SS0DirectPublishMainMenuState800264AC(ctx);
        (void)SS0DirectEnterOptions(ctx);
        break;
    case PrSS0DirectoryDispatcherDirect::MainLoopActionKind::Gap:
    default:
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "MAIN MENU GAP - O:BACK");
        Log::Printf(
            "SS0 direct runtime: main menu result=%d action=%s blocked=%d pending P0 gap",
            result,
            PrSS0DirectoryDispatcherDirect::MainLoopActionKindName(
                consumed.action),
            consumed.blockedByP0Gap ? 1 : 0);
        break;
    }
}

static const char* SS0DirectMainMenuFrameClassName80026B94(
    PrSS0EventFrameLoopDirect::Event3FrameClass80026B94 frameClass) {
    using FrameClass =
        PrSS0EventFrameLoopDirect::Event3FrameClass80026B94;
    switch (frameClass) {
    case FrameClass::OpenInputLoop:
        return "open-input";
    case FrameClass::ResultAction:
        return "result-action";
    case FrameClass::ClosedInputTail:
        return "closed-input-tail";
    default:
        return "unknown";
    }
}

static bool SS0DirectRunMainMenuTick80026720(PrGameContext& ctx) {
    // Current IDA binds 80026720 to the event-3 context 800544F8 and the
    // shared 800916E4 counter through 80025D70.
    const auto blink = PrSS0DirectoryDispatcherDirect::TickBlink80025D70(
        s_ss0Direct.blinkCounter800916E4,
        s_ss0Direct.mainMenuBlink);
    s_ss0Direct.blinkCounter800916E4 = blink.counter800916E4;
    s_ss0Direct.mainMenuBlink = blink.contextBlinkOnOff;
    SS0DirectPublishMainMenuState800264AC(ctx);
    return true;
}

static bool SS0DirectBeginMainMenuFrameTransaction80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event3FrameClass80026B94 frameClass,
    bool inputClosed,
    int32_t tailFramesRemainingBefore,
    int32_t tailFramesRemainingAfter,
    const PrSS0EventFrameLoopDirect::DispatcherTailState80026B94& tailCandidate) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    s_ss0Direct.mainMenuFrameTailCandidate80026B94 = {};
    s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94 = false;
    EventFrame::Event3FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = EventFrame::kCtxEvent3_800544F8;
    begin.logicFrame = ctx.frame;
    begin.frameClass = frameClass;
    begin.inputClosed = inputClosed;
    begin.tailFramesRemainingBefore = tailFramesRemainingBefore;
    begin.tailFramesRemainingAfter = tailFramesRemainingAfter;
    if (!EventFrame::BeginEvent3FrameTransaction80026B94(
            s_ss0Direct.mainMenuFrameTransaction80026B94, begin)) {
        Log::Printf(
            "SS0 direct runtime: event3 frame transaction begin rejected frame=%u class=%s inputClosed=%d tail=%d/%d",
            static_cast<unsigned>(ctx.frame),
            SS0DirectMainMenuFrameClassName80026B94(frameClass),
            inputClosed ? 1 : 0,
            tailFramesRemainingBefore,
            tailFramesRemainingAfter);
        EventFrame::ResetEvent3FrameTransaction80026B94(
            s_ss0Direct.mainMenuFrameTransaction80026B94);
        return false;
    }

    if (!SS0DirectRunMainMenuTick80026720(ctx) ||
        !EventFrame::CommitEvent3FrameTick80026720(
            s_ss0Direct.mainMenuFrameTransaction80026B94,
            ctx.frame,
            EventFrame::kEventTable3_80054550,
            EventFrame::kCtxEvent3_800544F8)) {
        Log::Printf(
            "SS0 direct runtime: event3 frame transaction tick rejected frame=%u class=%s",
            static_cast<unsigned>(ctx.frame),
            SS0DirectMainMenuFrameClassName80026B94(frameClass));
        EventFrame::ResetEvent3FrameTransaction80026B94(
            s_ss0Direct.mainMenuFrameTransaction80026B94);
        return false;
    }

    // Formal 8001E750 selects 8004019C, writes the same slot to gp+0x368,
    // sets the matching packet allocator, and clears that 80087288 work-list
    // before case 3 calls 8001D74C/80021E60.
    PrPsxEventFrameDirect::BeginDrawWrapper8001E750(
        s_ss0Direct.mainMenuFrameHostState8001E750, 3);
    s_ss0Direct.mainMenuFrameEndFrameWorkListSlot8001EA00 =
        s_ss0Direct.mainMenuFrameHostState8001E750.gp368WorkSlot & 1u;
    s_ss0Direct.mainMenuFrameEndFrameBindingKnown8001EA00 = true;
    s_ss0Direct.mainMenuFrameEndFrameBindingBlockedLogged8001EA00 = false;

    s_ss0Direct.mainMenuFrameTailCandidate80026B94 = tailCandidate;
    s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94 = true;
    return true;
}

static void SS0DirectCommitMainMenuFrameTail80026B94() {
    if (!s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94) {
        return;
    }
    s_ss0Direct.mainMenuDispatcherTail80026B94 =
        s_ss0Direct.mainMenuFrameTailCandidate80026B94;
    s_ss0Direct.mainMenuFrameTailCandidate80026B94 = {};
    s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94 = false;
}

static int SS0DirectTickMainMenu(PrGameContext& ctx) {
    if (s_ss0Direct.hiScoreOuterPadReleaseActive80015788) {
        return SS0DirectTickHiScoreOuterPadRelease80015788(ctx);
    }
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    auto& frameTransaction = s_ss0Direct.mainMenuFrameTransaction80026B94;
    if (!EventFrame::CanAdvanceEvent3FrameLogic80026B94(frameTransaction)) {
        if (!frameTransaction.logicAdvanceBlockedLogged) {
            frameTransaction.logicAdvanceBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: event3 pending frame blocks logicFrame=%u hostFrame=%u tickCommitted=%d drawSubmitted=%d",
                static_cast<unsigned>(frameTransaction.logicFrame),
                static_cast<unsigned>(ctx.frame),
                frameTransaction.tickCommitted ? 1 : 0,
                frameTransaction.directDrawSubmitted ? 1 : 0);
        }
        return 0;
    }

    auto tailCandidate = s_ss0Direct.mainMenuDispatcherTail80026B94;
    EventFrame::DispatcherTailStep80026B94 tailStep{};
    if (!EventFrame::TryStepEvent3DispatcherTail80026B94(
            frameTransaction, tailCandidate, tailStep)) {
        return 0;
    }
    if (tailStep.kind == PrSS0EventFrameLoopDirect::
                             DispatcherTailStepKind80026B94::ReleaseResult) {
        s_ss0Direct.mainMenuDispatcherTail80026B94 = tailCandidate;
        EventFrame::ResetEvent3FrameTransaction80026B94(frameTransaction);
        s_ss0Direct.mainMenuFrameTailCandidate80026B94 = {};
        s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94 = false;
        Log::Printf(
            "SS0 direct runtime: event3 80026B94 tail released result=%d",
            tailStep.releasedResult);
        SS0DirectConsumeMainMenuResult(ctx, tailStep.releasedResult);
        return 0;
    }

    PrSfx::PlayBgm();
    // 80015788's event3 loop has the same pre-loop release contract as the
    // event2/event17 directory loops.  Without it, the second frame of the
    // title-confirm pulse can land on the first visible MENU frame and be
    // decoded as a fresh STAGE/Cross action.  Keep drawing the menu while the
    // release is observed, but do not dispatch the held mask.
    const PrPadState preLoopPad = SS0DirectReadPad(ctx);
    const uint32_t preLoopPadMask =
        SS0DirectFullLocalPadToPsxMask80026744(
            static_cast<uint16_t>(preLoopPad.held | preLoopPad.pressed));
    const auto preLoopRelease =
        PrSS0EventFrameLoopDirect::StepDispatcherPreLoopPadRelease80026B94(
            s_ss0Direct.mainMenuPreLoopPadRelease80026B94,
            preLoopPadMask);
    if (preLoopRelease.kind ==
        PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseStepKind80026B94::
            WaitingForRelease) {
        if (!s_ss0Direct.mainMenuPreLoopPadReleaseBlockedLogged) {
            s_ss0Direct.mainMenuPreLoopPadReleaseBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: event3 pre-loop 80035510 waiting pad=%04X frame=%u",
                static_cast<unsigned>(preLoopPadMask),
                static_cast<unsigned>(ctx.frame));
        }
        (void)SS0DirectBeginMainMenuFrameTransaction80026B94(
            ctx,
            EventFrame::Event3FrameClass80026B94::ClosedInputTail,
            true,
            -1,
            -1,
            tailCandidate);
        return 0;
    }
    if (preLoopRelease.kind ==
        PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseStepKind80026B94::
            ReleasedEnterLoop) {
        s_ss0Direct.mainMenuPreLoopPadReleaseBlockedLogged = false;
        Log::Printf(
            "SS0 direct runtime: event3 pre-loop 80035510 released frame=%u",
            static_cast<unsigned>(ctx.frame));
    }
    if (tailStep.kind == PrSS0EventFrameLoopDirect::
                             DispatcherTailStepKind80026B94::HoldFrame) {
        (void)SS0DirectBeginMainMenuFrameTransaction80026B94(
            ctx,
            EventFrame::Event3FrameClass80026B94::ClosedInputTail,
            true,
            tailStep.framesRemainingBefore,
            tailStep.framesRemainingAfter,
            tailCandidate);
        return 0;
    }

    // Reuse the pre-loop sample.  Reading SS0DirectReadPad a second time in
    // the same host frame would update lastDebugPad and erase the pressed
    // edge that just passed the release gate.
    const PrPadState& pad = preLoopPad;
    const uint32_t psxPad =
        SS0DirectLocalPressedToPsxPadMask(pad.pressed);
    if (psxPad != 0u) {
        const int previousCursor = s_ss0Direct.mainMenuState.cursor;
        const int previousRecordsMode = s_ss0Direct.mainMenuState.word800916DA;
        const auto handled =
            PrSS0DirectoryDispatcherDirect::HandleMainMenu800264AC(
                s_ss0Direct.mainMenuState,
                psxPad);
        s_ss0Direct.mainMenuState = handled.state;
        SS0DirectPublishMainMenuState800264AC(ctx);
        if (handled.cueCode80025C8C != 0u) {
            SS0DirectPlayMainMenuInputCue800264AC(
                handled.cueCode80025C8C);
        }

        if (handled.result != 0) {
            if (!PrSS0EventFrameLoopDirect::ArmDispatcherTail80026B94(
                    tailCandidate,
                    handled.result)) {
                Log::Printf(
                    "SS0 direct runtime: event3 80026B94 tail arm rejected result=%d",
                    handled.result);
                return 0;
            }
            Log::Printf(
                "SS0 direct runtime: event3 80026B94 result=%d tail armed frames=%d",
                handled.result,
                PrSS0EventFrameLoopDirect::
                    kDispatcherResultTailFrames80026B94);
        }
        if (s_ss0Direct.mainMenuState.cursor != previousCursor) {
            Log::Printf("SS0 direct runtime: main menu cursor=%d",
                        s_ss0Direct.mainMenuState.cursor);
        } else if (s_ss0Direct.mainMenuState.word800916DA != previousRecordsMode) {
            Log::Printf("SS0 direct runtime: main menu RECORDS mode=%d",
                        s_ss0Direct.mainMenuState.word800916DA);
        }
    }

    const bool resultActionFrame = tailCandidate.active &&
        tailCandidate.framesRemaining ==
            EventFrame::kDispatcherResultTailFrames80026B94;
    const EventFrame::Event3FrameClass80026B94 frameClass =
        resultActionFrame
            ? EventFrame::Event3FrameClass80026B94::ResultAction
            : EventFrame::Event3FrameClass80026B94::OpenInputLoop;
    const int32_t tailFrameMarker = resultActionFrame
        ? EventFrame::kDispatcherResultTailFrames80026B94
        : -1;
    (void)SS0DirectBeginMainMenuFrameTransaction80026B94(
        ctx,
        frameClass,
        false,
        tailFrameMarker,
        tailFrameMarker,
        tailCandidate);

    return 0;
}

static const char* SS0DirectPracticeJudgeName(int judge) {
    switch (judge) {
    case 0: return "ON BEAT";
    case 1: return "TOO QUICK";
    case 2: return "TOO SLOW";
    case 7: return "DONE";
    case 8: return "EXIT";
    case 9: return "READY";
    default: break;
    }
    return "READY";
}

static void SS0DirectPracticeResetSequences() {
    s_ss0Direct.practiceSeqCurA = -1;
    s_ss0Direct.practiceSeqCurB = -1;
    s_ss0Direct.practiceSeqEnA = 0;
    s_ss0Direct.practiceSeqEnB = 0;
    s_ss0Direct.practiceProducerLastLogicFrame = -1;
    PrSS0DirectoryPagesRenderDirect::ResetPracticeWobbleBank80024308(
        s_ss0Direct.practiceWobbleBank);
    s_ss0Direct.practicePortraitTeacher = {};
    s_ss0Direct.practicePortraitStudent = {};
    s_ss0Direct.practiceSliceDrawList = {};
    s_ss0Direct.practicePostSliceTailDrawList = {};
    s_ss0Direct.practiceLeadingOverlayDrawList = {};
}

static void SS0DirectPracticeAdvanceSequenceFrame(int frameInRound) {
    if (frameInRound == 60) {
        s_ss0Direct.practiceSeqCurB = 0;
        s_ss0Direct.practiceSeqEnB = 1;
    } else if (frameInRound == kPracticeFastForwardFrame) {
        s_ss0Direct.practiceSeqCurA = 0;
        s_ss0Direct.practiceSeqEnA = 1;
    }

    if (frameInRound < 0 || (frameInRound % 5) != 0) {
        return;
    }

    const int round = s_ss0Direct.practiceRound & 3;
    // Pseudo-C reads the stream byte at the current cursor, stops on the
    // first -1 (or the hard cursor guard at 19), then advances by exactly one
    // slot.  The old pulse-slot +2 shortcut skipped source entries and made
    // the portrait/icon lanes diverge from the Rec44 data.
    const auto advanceLane = [round](int& cursor, int& enabled) {
        if (enabled == 0) {
            return false;
        }
        const auto value =
            PrSS0PracticeLifecycleDirect::ReadPracticeRec44StreamValue(
                round, cursor);
        if (cursor >= kPracticeRec44CursorLimit ||
            !value.sourceKnown || value.value == -1) {
            enabled = 0;
            cursor = -1;
            return true;
        }
        ++cursor;
        return false;
    };

    bool terminated = false;
    terminated = advanceLane(s_ss0Direct.practiceSeqCurA,
                             s_ss0Direct.practiceSeqEnA) || terminated;
    terminated = advanceLane(s_ss0Direct.practiceSeqCurB,
                             s_ss0Direct.practiceSeqEnB) || terminated;
    if (terminated) {
        // 80024308 clears the complete 36-entry wobble bank, not just the
        // lane that reached its sentinel.
        PrSS0DirectoryPagesRenderDirect::ResetPracticeWobbleBank80024308(
            s_ss0Direct.practiceWobbleBank);
    }
}

static void SS0DirectPracticeStartPreview() {
    s_ss0Direct.practicePhase = SS0DirectPracticePhase::Preview;
    s_ss0Direct.practiceRoundFrame = kPracticePreviewStartFrame - 1;
    SS0DirectPracticeClearOverlayGate8002776C();
    SS0DirectPracticeSetPadStop(0, -1);
    s_ss0Direct.practiceExitConfirmFrames = 0;
    s_ss0Direct.practicePendingJudgeKind = -1;
    s_ss0Direct.practicePendingRound = -1;
    s_ss0Direct.practiceTargetResolved = false;
    SS0DirectPracticeResetSequences();
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "PREVIEW");
    PrSfx::PlayPracticeLoopStart();
    Log::Printf("SS0 direct runtime: practice preview round=%d frame=%d",
                s_ss0Direct.practiceRound,
                s_ss0Direct.practiceRoundFrame);
}

static void SS0DirectPracticeStartJudge() {
    s_ss0Direct.practicePhase = SS0DirectPracticePhase::Judge;
    s_ss0Direct.practiceRoundFrame = -1;
    SS0DirectPracticeClearOverlayGate8002776C();
    SS0DirectPracticeSetPadStop(0, -1);
    s_ss0Direct.practiceExitConfirmFrames = 0;
    s_ss0Direct.practicePendingJudgeKind = -1;
    s_ss0Direct.practicePendingRound = -1;
    s_ss0Direct.practiceTargetResolved = false;
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "YOUR TURN");
    Log::Printf("SS0 direct runtime: practice judge start round=%d",
                s_ss0Direct.practiceRound);
}

static void SS0DirectPracticeStartRoundIntro() {
    if (s_ss0Direct.practicePendingRound >= 0) {
        s_ss0Direct.practiceRound = s_ss0Direct.practicePendingRound;
        s_ss0Direct.practicePendingRound = -1;
    }
    const int round = s_ss0Direct.practiceRound & 3;
    s_ss0Direct.practicePhase = SS0DirectPracticePhase::RoundIntro;
    s_ss0Direct.practiceRoundFrame = -1;
    if (round == 0) {
        SS0DirectPracticeClearOverlayGate8002776C();
    }
    SS0DirectPracticeSetPadStop(kPracticeRoundIntroFrames[round], 3 + round);
    s_ss0Direct.practiceExitConfirmFrames = 0;
    s_ss0Direct.practicePendingJudgeKind = -1;
    s_ss0Direct.practiceTargetResolved = false;
    s_ss0Direct.practiceLastJudge = 9;
    s_ss0Direct.practiceJudgeFlash = 0;
    SS0DirectPracticeResetSequences();
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "ROUND %d INTRO",
                  s_ss0Direct.practiceRound + 1);
    PrSfx::PlayPracticeRoundIntroVoiceCue(round);
    Log::Printf("SS0 direct runtime: practice intro round=%d frames=%d",
                s_ss0Direct.practiceRound,
                s_ss0Direct.practicePadStopFrames);
}

static void SS0DirectPracticeStartExitPrompt() {
    s_ss0Direct.practicePhase = SS0DirectPracticePhase::ExitPrompt;
    s_ss0Direct.practiceRoundFrame = -1;
    SS0DirectPracticeSetPadStop(kPracticeExitPromptFrames,
                                kPracticeExitPromptJudge);
    s_ss0Direct.practiceExitConfirmFrames = 0;
    s_ss0Direct.practicePendingJudgeKind = -1;
    s_ss0Direct.practicePendingRound = -1;
    s_ss0Direct.practiceLastJudge = kPracticeExitPromptJudge;
    s_ss0Direct.practiceJudgeFlash = kPracticeExitPromptFrames;
    s_ss0Direct.practiceTargetResolved = true;
    SS0DirectPracticeResetSequences();
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "X:EXIT  O:RETRY");
    PrSfx::PlayPracticeExitPromptVoiceCue();
    Log::Printf("SS0 direct runtime: practice exit prompt");
}

static void SS0DirectPracticeResolveJudge(int judgeKind, int roundBeforeResolve) {
    s_ss0Direct.practicePendingJudgeKind = -1;
    s_ss0Direct.practicePendingRound = -1;
    s_ss0Direct.practiceLastJudge = judgeKind;
    s_ss0Direct.practiceJudgeFlash = 60;
    s_ss0Direct.practiceTargetResolved = true;
    SS0DirectPracticeResetSequences();

    if (judgeKind == 0) {
        s_ss0Direct.practiceCombo++;
        s_ss0Direct.practiceHits++;
        s_ss0Direct.practiceScore += 300 + s_ss0Direct.practiceCombo * 25;
        const int nextRound = roundBeforeResolve + 1;
        if (nextRound >= kPracticeRoundCount) {
            s_ss0Direct.practiceComplete = true;
            s_ss0Direct.practicePhase = SS0DirectPracticePhase::CompletePrompt;
            SS0DirectPracticeSetPadStop(kPracticeCompletePromptFrames,
                                        kPracticeCompletePromptJudge);
            s_ss0Direct.practiceExitConfirmFrames = 0;
            s_ss0Direct.practiceLastJudge = kPracticeCompletePromptJudge;
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "SCORE %d  HITS %d/%d",
                          s_ss0Direct.practiceScore,
                          s_ss0Direct.practiceHits,
                          kPracticeRoundCount);
            PrSfx::PlayPracticeCompletePromptVoiceCue();
            Log::Printf("SS0 direct runtime: practice complete score=%d hits=%d misses=%d",
                        s_ss0Direct.practiceScore,
                        s_ss0Direct.practiceHits,
                        s_ss0Direct.practiceMisses);
            return;
        }
        s_ss0Direct.practicePendingRound = nextRound;
    } else {
        s_ss0Direct.practiceCombo = 0;
        s_ss0Direct.practiceMisses++;
        s_ss0Direct.practicePendingRound = roundBeforeResolve;
    }

    s_ss0Direct.practicePhase = SS0DirectPracticePhase::ResultPadStop;
    SS0DirectPracticeSetPadStop(kPracticeResultPadStopFrames, judgeKind);
    s_ss0Direct.practiceExitConfirmFrames = 0;
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "%s %+d",
                  SS0DirectPracticeJudgeName(judgeKind),
                  s_ss0Direct.practiceLastDelta);
    PrSfx::PlayPracticeResultVoiceCue(judgeKind);
    Log::Printf("SS0 direct runtime: practice resolve round=%d judge=%s delta=%d score=%d combo=%d",
                roundBeforeResolve,
                SS0DirectPracticeJudgeName(judgeKind),
                s_ss0Direct.practiceLastDelta,
                s_ss0Direct.practiceScore,
                s_ss0Direct.practiceCombo);
}

static void SS0DirectPracticeHandlePadStopEnd() {
    if (s_ss0Direct.practicePhase == SS0DirectPracticePhase::RoundIntro) {
        SS0DirectPracticeStartPreview();
        return;
    }
    if (s_ss0Direct.practicePhase == SS0DirectPracticePhase::ResultPadStop) {
        SS0DirectPracticeStartRoundIntro();
        return;
    }
    if (s_ss0Direct.practicePhase == SS0DirectPracticePhase::CompletePrompt) {
        SS0DirectPracticeStartExitPrompt();
        return;
    }
    if (s_ss0Direct.practicePhase == SS0DirectPracticePhase::ExitPrompt) {
        s_ss0Direct.practicePhase = SS0DirectPracticePhase::ExitConfirm;
        SS0DirectPracticeSetPadStop(0, -1);
        if (!PrSS0PracticeLifecycleDirect::
                BeginPracticeExitConfirm8002776C(
                    s_ss0Direct.practiceExitConfirmRuntime,
                    static_cast<int16_t>(s_ss0Direct.practiceExitBlink4C)) ||
            s_ss0Direct.practiceExitConfirmRuntime.framesRemaining !=
                kPracticeExitConfirmFrames) {
            s_ss0Direct.practicePhase = SS0DirectPracticePhase::Idle;
            s_ss0Direct.practiceExitConfirmFrames = 0;
            return;
        }
        s_ss0Direct.practiceExitConfirmFrames =
            s_ss0Direct.practiceExitConfirmRuntime.framesRemaining;
        s_ss0Direct.practiceExitSelection48 =
            s_ss0Direct.practiceExitConfirmRuntime.exitSelection48;
        s_ss0Direct.practiceExitBlink4C =
            s_ss0Direct.practiceExitConfirmRuntime.exitBlink4C;
        s_ss0Direct.practiceJudgeFlash = 0;
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "X:EXIT  O:RETRY");
        Log::Printf("SS0 direct runtime: practice exit confirm");
        return;
    }
}

static int SS0DirectTickPractice(PrGameContext& ctx) {
    s_ss0Direct.practiceFrame++;
    const PrPadState pad = SS0DirectReadPad(ctx);

    if (s_ss0Direct.practiceJudgeFlash > 0) {
        s_ss0Direct.practiceJudgeFlash--;
    }

    if (s_ss0Direct.practicePhase == SS0DirectPracticePhase::ExitConfirm) {
        PrSS0PracticeLifecycleDirect::PracticeExitConfirmInput8002776C
            input{};
        input.edgeKnown = true;
        const bool exitPressed =
            SS0DirectPadPressed(pad, PrPadButton::Cross);
        const bool retryPressed =
            SS0DirectPadPressed(pad, PrPadButton::Circle) ||
            SS0DirectPadPressed(pad, PrPadButton::Triangle);
        if (exitPressed) {
            input.edge = 0x40u;
        } else if (retryPressed) {
            input.edge = 0x20u;
        }

        if (!PrSS0PracticeLifecycleDirect::TickPracticeExitConfirm8002776C(
                s_ss0Direct.practiceExitConfirmRuntime, input)) {
            return 0;
        }
        s_ss0Direct.practiceExitConfirmFrames =
            s_ss0Direct.practiceExitConfirmRuntime.framesRemaining;
        s_ss0Direct.practiceExitSelection48 =
            s_ss0Direct.practiceExitConfirmRuntime.exitSelection48;
        s_ss0Direct.practiceExitBlink4C =
            s_ss0Direct.practiceExitConfirmRuntime.exitBlink4C;

        if (exitPressed) {
            (void)PrSfx::PlayScene0ConfirmCue80025C8C();
        } else if (retryPressed) {
            (void)PrSfx::PlayScene0CancelCue80025C8C();
        }

        const auto result = s_ss0Direct.practiceExitConfirmRuntime.result;
        if (result == PrSS0PracticeLifecycleDirect::
                          PracticeExitConfirmResult8002776C::RetryPractice) {
            if (SS0DirectEnterPractice(ctx)) {
                Log::Printf(
                    "SS0 direct runtime: practice retry from exit confirm");
            } else {
                SS0DirectPracticeStartExitPrompt();
                Log::Printf(
                    "SS0 direct runtime: practice retry resource gate blocked -> exit prompt");
            }
            return 0;
        }
        if (result == PrSS0PracticeLifecycleDirect::
                          PracticeExitConfirmResult8002776C::
                              RestorePreviousScene) {
            if (!SS0DirectRestoreScene0ResourcesAfterPractice80015590(ctx)) {
                SS0DirectPracticeStartExitPrompt();
                Log::Printf(
                    "SS0 direct runtime: practice exit confirm resource restore blocked -> exit prompt");
            } else {
                // 8002776C's tail restores the common segment via 80015590.
                // Keep its mode-3 Loading callback separate from the later
                // event3 mode-4 reveal.  80015590 itself starts no new sound.
                s_ss0Direct.loadingPatternMode80015408 =
                    PrSS0TransitionDirect::kLoadingPatternMode80015408;
                ctx.sceneExitReason = s_ss0Direct.loadingPatternMode80015408;
                s_ss0Direct.phase = SS0DirectPhase::MainMenuResourceLoading;
                SS0DirectBeginLoadingScreen(
                    SS0DirectLoadingHoldKind::MainMenuReload, ctx.frame);
                SS0DirectArmLoadingMinimumHold(
                    SS0DirectLoadingHoldKind::MainMenuReload, ctx.frame,
                    ctx.frame + kSs0LoadingMinimumLogicTicks);
                Log::Printf(
                    "SS0 direct runtime: practice exit confirm -> 80015590 Loading mode=3");
            }
            return 0;
        }
        return 0;
    }

    if (SS0DirectPadPressed(pad, PrPadButton::Circle) &&
        s_ss0Direct.practicePhase != SS0DirectPracticePhase::ExitPrompt &&
        s_ss0Direct.practicePhase != SS0DirectPracticePhase::CompletePrompt) {
        SS0DirectPracticeStartExitPrompt();
        return 0;
    }

    if (s_ss0Direct.practicePhase == SS0DirectPracticePhase::Judge &&
        SS0DirectPadPressed(pad, PrPadButton::Cross)) {
        s_ss0Direct.practiceRoundFrame = kPracticeFastForwardFrame - 1;
        SS0DirectPracticeStartExitPrompt();
        return 0;
    }

    if (s_ss0Direct.practicePadStopFrames > 0) {
        s_ss0Direct.practicePadStopFrames--;
        if (s_ss0Direct.practicePadStopFrames <= 0) {
            SS0DirectPracticeSetPadStop(0, -1);
            SS0DirectPracticeHandlePadStopEnd();
        }
        return 0;
    }

    if (s_ss0Direct.practicePhase == SS0DirectPracticePhase::Preview) {
        s_ss0Direct.practiceRoundFrame++;
        SS0DirectPracticeAdvanceSequenceFrame(s_ss0Direct.practiceRoundFrame);
        if (s_ss0Direct.practiceRoundFrame >= kPracticePreviewEndFrame) {
            SS0DirectPracticeStartJudge();
        }
        return 0;
    }

    if (s_ss0Direct.practicePhase != SS0DirectPracticePhase::Judge) {
        return 0;
    }

    s_ss0Direct.practiceRoundFrame++;
    const int round = s_ss0Direct.practiceRound & 3;
    const int targetOffset = kPracticeTargetOffsets[round];
    if (s_ss0Direct.practicePendingJudgeKind < 0 &&
        s_ss0Direct.practiceRoundFrame >= kPracticeRoundFrames) {
        s_ss0Direct.practiceRoundFrame = 0;
    }

    if (s_ss0Direct.practicePendingJudgeKind < 0 &&
        (s_ss0Direct.practiceRoundFrame % kPracticeBeatFrames) == 0) {
        s_ss0Direct.practiceTick++;
        PrSfx::PlayPracticeBeatCue();
    }

    if (s_ss0Direct.practicePendingJudgeKind < 0 &&
        s_ss0Direct.practiceRoundFrame == targetOffset) {
        PrSfx::PlayPracticeRoundPromptCue(round);
    }

    SS0DirectPracticeAdvanceSequenceFrame(s_ss0Direct.practiceRoundFrame);

    if (s_ss0Direct.practicePendingJudgeKind < 0 &&
        SS0DirectPadPressed(pad, PrPadButton::Triangle)) {
        if (s_ss0Direct.practiceRoundFrame < (kPracticeBeatFrames - 2)) {
            return 0;
        }

        int judgeKind = 0;
        if (s_ss0Direct.practiceRoundFrame < targetOffset + 73) {
            judgeKind = 1;
        } else if (s_ss0Direct.practiceRoundFrame > targetOffset + 77) {
            judgeKind = 2;
        }
        s_ss0Direct.practicePendingJudgeKind = judgeKind;
        s_ss0Direct.practicePendingRound = s_ss0Direct.practiceRound;
        s_ss0Direct.practiceLastJudge = judgeKind;
        s_ss0Direct.practiceLastDelta =
            s_ss0Direct.practiceRoundFrame -
            (targetOffset + kPracticeBeatFrames);
        s_ss0Direct.practiceJudgeFlash = 36;
        s_ss0Direct.practiceTargetResolved = true;
        PrSfx::PlayPracticeRoundPromptCue(round);
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "LOCKED %s %+d",
                      SS0DirectPracticeJudgeName(judgeKind),
                      s_ss0Direct.practiceLastDelta);
        Log::Printf("SS0 direct runtime: practice latch round=%d judge=%s frame=%d delta=%d",
                    s_ss0Direct.practiceRound,
                    SS0DirectPracticeJudgeName(judgeKind),
                    s_ss0Direct.practiceRoundFrame,
                    s_ss0Direct.practiceLastDelta);
    }

    if (s_ss0Direct.practicePendingJudgeKind >= 0 &&
        s_ss0Direct.practiceRoundFrame >= kPracticeRoundFrames - 1) {
        SS0DirectPracticeResolveJudge(s_ss0Direct.practicePendingJudgeKind,
                                      s_ss0Direct.practicePendingRound);
    }

    return 0;
}

static void SS0DirectPlayHiScoreEvent6Cue80025E0C(
    const PrGameContext& ctx,
    uint32_t cueCode80025C8C) {
    const bool directScene0VabVoice =
        PrSfx::PlayScene0UiCue80025C8CRaw(
            static_cast<uint16_t>(cueCode80025C8C));
    Log::Printf(
        "SS0 direct runtime: event6 80025E0C cue=%04X scene0VabVoice=%d frame=%u",
        static_cast<unsigned>(cueCode80025C8C),
        directScene0VabVoice ? 1 : 0,
        static_cast<unsigned>(ctx.frame));
}

static bool SS0DirectRunHiScoreEvent6Tick80025E48() {
    const auto tick =
        PrSS0DirectoryDispatcherDirect::TickHiScoreEvent6Blink80025E48(
            s_ss0Direct.hiScoreEvent6State800267E4,
            s_ss0Direct.blinkCounter800916E4);
    if (!tick.accepted) {
        Log::Printf(
            "SS0 direct runtime: event6 80025E48 rejected unbound ctx");
        return false;
    }
    s_ss0Direct.hiScoreEvent6State800267E4 = tick.state;
    s_ss0Direct.blinkCounter800916E4 = tick.blink.counter800916E4;
    s_ss0Direct.hiScoreBlink = tick.state.exitIconStateCtx00;
    SS0DirectPublishHiScoreEvent6Ctx80049278();
    return true;
}

static const char* SS0DirectEvent6FrameClassName80026B94(
    PrSS0EventFrameLoopDirect::Event6FrameClass80026B94 frameClass) {
    using FrameClass =
        PrSS0EventFrameLoopDirect::Event6FrameClass80026B94;
    switch (frameClass) {
    case FrameClass::OpenInputLoop:
        return "open-input";
    case FrameClass::ResultAction:
        return "result-action";
    case FrameClass::ClosedInputTail:
        return "closed-input-tail";
    default:
        return "unknown";
    }
}

static bool SS0DirectBeginHiScoreEvent6FrameTransaction80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event6FrameClass80026B94 frameClass,
    bool inputClosed,
    int32_t tailFramesRemainingBefore,
    int32_t tailFramesRemainingAfter) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    EventFrame::Event6FrameTransactionBegin80026B94 begin{};
    begin.requestBound =
        s_ss0Direct.hiScoreEvent6State800267E4.requestBound;
    begin.ctxAddress =
        s_ss0Direct.hiScoreEvent6State800267E4.ctxAddress;
    begin.logicFrame = ctx.frame;
    begin.frameClass = frameClass;
    begin.inputClosed = inputClosed;
    begin.tailFramesRemainingBefore = tailFramesRemainingBefore;
    begin.tailFramesRemainingAfter = tailFramesRemainingAfter;
    if (!EventFrame::BeginEvent6FrameTransaction80026B94(
            s_ss0Direct.hiScoreFrameTransaction80026B94, begin)) {
        Log::Printf(
            "SS0 direct runtime: event6 frame transaction begin rejected frame=%u class=%s inputClosed=%d tail=%d/%d",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent6FrameClassName80026B94(frameClass),
            inputClosed ? 1 : 0,
            tailFramesRemainingBefore,
            tailFramesRemainingAfter);
        return false;
    }
    if (!SS0DirectRunHiScoreEvent6Tick80025E48() ||
        !EventFrame::CommitEvent6FrameTick80025E48(
            s_ss0Direct.hiScoreFrameTransaction80026B94,
            ctx.frame,
            EventFrame::kEventTable6_80054578,
            EventFrame::kCtxEvent6HiScoreTable80049278)) {
        Log::Printf(
            "SS0 direct runtime: event6 frame transaction tick rejected frame=%u class=%s",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent6FrameClassName80026B94(frameClass));
        return false;
    }
    // Formal 8001E750 selects 8004019C, stores the same slot at gp+0x368,
    // and clears that slot's work-list before Event6's page draw. Keep the
    // owner binding independent from the legacy Windows S0 shell.
    PrPsxEventFrameDirect::BeginDrawWrapper8001E750(
        s_ss0Direct.hiScoreFrameHostState8001E750, 6);
    s_ss0Direct.hiScoreFrameEndFrameWorkListSlot8001EA00 =
        s_ss0Direct.hiScoreFrameHostState8001E750.gp368WorkSlot & 1u;
    s_ss0Direct.hiScoreFrameEndFrameBindingKnown8001EA00 = true;
    s_ss0Direct.hiScoreFrameEndFrameBindingBlockedLogged8001EA00 = false;
    return true;
}

static int SS0DirectTickHiScore(PrGameContext& ctx) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    auto& frameTransaction =
        s_ss0Direct.hiScoreFrameTransaction80026B94;
    if (s_ss0Direct.hiScoreOuterPadReleaseActive80015788) {
        if (!EventFrame::CanAdvanceEvent6FrameLogic80026B94(
                frameTransaction)) {
            if (!frameTransaction.logicAdvanceBlockedLogged) {
                frameTransaction.logicAdvanceBlockedLogged = true;
                Log::Printf(
                    "SS0 direct runtime: event6 pending frame blocks outer release logicFrame=%u hostFrame=%u",
                    static_cast<unsigned>(frameTransaction.logicFrame),
                    static_cast<unsigned>(ctx.frame));
            }
            return 0;
        }
        EventFrame::ResetEvent6FrameTransaction80026B94(frameTransaction);
        return SS0DirectTickHiScoreOuterPadRelease80015788(ctx);
    }

    if (!EventFrame::CanAdvanceEvent6FrameLogic80026B94(
            frameTransaction)) {
        if (!frameTransaction.logicAdvanceBlockedLogged) {
            frameTransaction.logicAdvanceBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: event6 pending frame blocks logic advance logicFrame=%u hostFrame=%u tickCommitted=%d drawSubmitted=%d",
                static_cast<unsigned>(frameTransaction.logicFrame),
                static_cast<unsigned>(ctx.frame),
                frameTransaction.tickCommitted ? 1 : 0,
                frameTransaction.directDrawSubmitted ? 1 : 0);
        }
        return 0;
    }

    auto tailCandidate = s_ss0Direct.hiScoreDispatcherTail80026B94;
    EventFrame::DispatcherTailStep80026B94 tailStep{};
    if (!EventFrame::TryStepEvent6DispatcherTail80026B94(
            frameTransaction, tailCandidate, tailStep)) {
        return 0;
    }
    if (tailStep.kind ==
        EventFrame::DispatcherTailStepKind80026B94::ReleaseResult) {
        s_ss0Direct.hiScoreDispatcherTail80026B94 = tailCandidate;
        // 80015788 intentionally ignores the Event6 return value, then waits
        // for full pad release before starting the next Event3 dispatcher.
        Log::Printf(
            "SS0 direct runtime: event6 80026B94 tail released ignoredResult=%d frame=%u icon=%d label=%d",
            tailStep.releasedResult,
            static_cast<unsigned>(ctx.frame),
            s_ss0Direct.hiScoreEvent6State800267E4.exitIconStateCtx00,
            s_ss0Direct.hiScoreEvent6State800267E4.exitLabelStateCtx04);
        SS0DirectBeginHiScoreOuterPadRelease80015788(
            "event6 return ignored");
        return SS0DirectTickHiScoreOuterPadRelease80015788(ctx);
    }

    PrSfx::PlayBgm();
    if (tailStep.kind ==
        EventFrame::DispatcherTailStepKind80026B94::HoldFrame) {
        if (SS0DirectBeginHiScoreEvent6FrameTransaction80026B94(
                ctx,
                EventFrame::Event6FrameClass80026B94::ClosedInputTail,
                true,
                tailStep.framesRemainingBefore,
                tailStep.framesRemainingAfter)) {
            // Decrement the 60-frame tail only after the translated tick is
            // bound to a retryable draw transaction.
            s_ss0Direct.hiScoreDispatcherTail80026B94 = tailCandidate;
        }
        return 0;
    }

    const PrPadState pad = SS0DirectReadPad(ctx);
    const uint32_t currentPad =
        SS0DirectFullLocalPadToPsxMask80026744(
            static_cast<uint16_t>(pad.held | pad.pressed));
    const auto preLoopRelease =
        PrSS0EventFrameLoopDirect::StepDispatcherPreLoopPadRelease80026B94(
            s_ss0Direct.hiScorePreLoopPadRelease80026B94,
            currentPad);
    if (preLoopRelease.kind == PrSS0EventFrameLoopDirect::
                                   DispatcherPreLoopPadReleaseStepKind80026B94::
                                       WaitingForRelease) {
        PrSS0EventFrameLoopDirect::ResetEvent6FrameTransaction80026B94(
            s_ss0Direct.hiScoreFrameTransaction80026B94);
        if (!s_ss0Direct.hiScorePreLoopPadReleaseBlockedLogged) {
            s_ss0Direct.hiScorePreLoopPadReleaseBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: event6 pre-loop 80035510 waiting pad=%04X frame=%u",
                static_cast<unsigned>(currentPad),
                static_cast<unsigned>(ctx.frame));
        }
        return 0;
    }
    if (preLoopRelease.kind == PrSS0EventFrameLoopDirect::
                                   DispatcherPreLoopPadReleaseStepKind80026B94::
                                       ReleasedEnterLoop) {
        PrSS0EventFrameLoopDirect::ResetDispatcherPadChange80026744(
            s_ss0Direct.hiScorePadChange80026744);
        Log::Printf(
            "SS0 direct runtime: event6 pre-loop 80035510 released frame=%u",
            static_cast<unsigned>(ctx.frame));
    }

    const auto padChange =
        PrSS0EventFrameLoopDirect::StepDispatcherPadChange80026744(
            s_ss0Direct.hiScorePadChange80026744,
            currentPad);
    const uint32_t padMask80026744 =
        padChange.changedPadMask80026744;
    bool resultActionFrame = false;
    bool resultTailArmFailed = false;
    if (padMask80026744 != 0u) {
        const auto handled =
            PrSS0DirectoryDispatcherDirect::
                HandleHiScoreEvent6Input80025E0C(
                    s_ss0Direct.hiScoreEvent6State800267E4,
                    padMask80026744);
        // 80025E0C writes ctx+4 before issuing the raw Scene0 cue.
        s_ss0Direct.hiScoreEvent6State800267E4 = handled.state;
        SS0DirectPublishHiScoreEvent6Ctx80049278();
        if (handled.cueCode80025C8C != 0u) {
            SS0DirectPlayHiScoreEvent6Cue80025E0C(
                ctx, handled.cueCode80025C8C);
        }
        if (handled.result != 0) {
            if (PrSS0EventFrameLoopDirect::ArmDispatcherTail80026B94(
                    s_ss0Direct.hiScoreDispatcherTail80026B94,
                    handled.result)) {
                resultActionFrame = true;
                Log::Printf(
                    "SS0 direct runtime: event6 80025E0C result=%d tail armed frames=%d frame=%u",
                    handled.result,
                    PrSS0EventFrameLoopDirect::
                        kDispatcherResultTailFrames80026B94,
                    static_cast<unsigned>(ctx.frame));
            } else {
                resultTailArmFailed = true;
                Log::Printf(
                    "SS0 direct runtime: event6 80026B94 tail arm rejected result=%d",
                    handled.result);
            }
        }
    }

    if (resultTailArmFailed) {
        PrSS0EventFrameLoopDirect::ResetEvent6FrameTransaction80026B94(
            s_ss0Direct.hiScoreFrameTransaction80026B94);
        return 0;
    }

    // 80026B94 invokes 80025E48 before 8001E750 on every active loop frame.
    // A result action arms 60 later closed-input frames without spending one
    // on the action frame itself.
    const auto frameClass =
        resultActionFrame
            ? PrSS0EventFrameLoopDirect::
                  Event6FrameClass80026B94::ResultAction
            : PrSS0EventFrameLoopDirect::
                  Event6FrameClass80026B94::OpenInputLoop;
    const int32_t tailFrameMarker =
        resultActionFrame
            ? PrSS0EventFrameLoopDirect::
                  kDispatcherResultTailFrames80026B94
            : -1;
    (void)SS0DirectBeginHiScoreEvent6FrameTransaction80026B94(
        ctx,
        frameClass,
        false,
        tailFrameMarker,
        tailFrameMarker);
    return 0;
}

static void SS0DirectSyncCardGridSelection80020F94() {
    auto& state = s_ss0Direct.cardGridDrawState80020F94;
    const auto& input = s_ss0Direct.cardListInputRuntime800181D0;
    const int32_t terminal =
        static_cast<int32_t>(state.rows) * state.columns;
    if (!state.requestBound ||
        state.argAddress !=
            PrSS0DirectoryPagesRenderDirect::kCardGridArgAddress80048E50 ||
        state.itemCount != s_ss0Direct.cardEntryCount ||
        !input.requestBound || input.entryCount != state.itemCount ||
        input.selected < 0 || input.selected > terminal ||
        (input.selected != terminal && input.selected >= state.itemCount)) {
        return;
    }
    state.selected = static_cast<int16_t>(input.selected);
    s_ss0Direct.cardCursor = input.selected;
}

static void SS0DirectClearSelectedRowIdentity800181D0() {
    s_ss0Direct.cardSelectedRowIdentity800181D0 = {};
    s_ss0Direct.cardSelectedRowDirectoryGeneration = 0;
}

static bool SS0DirectArmLoadReplayCompletion80019D7C(
    SS0DirectLoadReplayCompletionKind80019D7C kind,
    int replayScene,
    const PrStage1SaveStatusPrefix80092F10& rollbackPrefix) {
    if (kind == SS0DirectLoadReplayCompletionKind80019D7C::None ||
        (kind == SS0DirectLoadReplayCompletionKind80019D7C::ReplayScene &&
         (replayScene < 1 || replayScene > 6)) ||
        (kind ==
             SS0DirectLoadReplayCompletionKind80019D7C::LoadReturnMainMenu &&
         replayScene != -1) ||
        s_ss0Direct.cardCompletionKind80019D7C !=
            SS0DirectLoadReplayCompletionKind80019D7C::None ||
        !CardHandoff::BeginLoadReplayState16Completion80019D7C(
            &s_ss0Direct.cardDriverVisualRuntime80018FB0)) {
        return false;
    }

    SS0DirectClearLoadReplayState16CardIoResult80017594();
    s_ss0Direct.cardCompletionKind80019D7C = kind;
    s_ss0Direct.cardCompletionReplayScene80019D7C = replayScene;
    s_ss0Direct.cardCompletionRollbackPrefixKnown80092F10 = true;
    s_ss0Direct.cardCompletionRollbackPrefix80092F10 = rollbackPrefix;
    return true;
}

static void SS0DirectSyncCardGridVisualFromDriver80018FB0();

static bool SS0DirectResetLoadReplayVisualToList80018FB0() {
    const auto mode = s_ss0Direct.cardListInputRuntime800181D0.mode;
    SS0DirectClearLoadReplayState16CardIoResult80017594();
    if (!CardHandoff::InitLoadReplayCardDriverVisualRuntime80018FB0(
            mode, &s_ss0Direct.cardDriverVisualRuntime80018FB0)) {
        s_ss0Direct.cardGridDrawState80020F94 = {};
        s_ss0Direct.cardDriverVisualRuntime80018FB0 = {};
        return false;
    }
    SS0DirectSyncCardGridVisualFromDriver80018FB0();
    return true;
}

static bool SS0DirectSelectedRowIdentityMatches800181D0() {
    const auto& identity = s_ss0Direct.cardSelectedRowIdentity800181D0;
    const auto& directory = s_ss0Direct.cardDirectoryCarrier80019D7C;
    if (!identity.committed || !identity.gp716Known || identity.gp716 != 1 ||
        !identity.nameBuffer8007CBE8Known ||
        identity.nameBuffer8007CBE8[0] == '\0' ||
        !identity.blockIndexKnown ||
        s_ss0Direct.cardSelectedRowDirectoryGeneration !=
            s_ss0Direct.cardDirectoryGeneration ||
        !s_ss0Direct.cardDirectoryAuthorityKnown ||
        identity.mode != s_ss0Direct.cardDirectoryMode ||
        directory.mode != identity.mode ||
        !directory.entryCountKnown ||
        identity.selectedRow != s_ss0Direct.cardCursor ||
        identity.selectedRow < 0 ||
        identity.selectedRow >= directory.entryCount ||
        identity.selectedRow >= 15) {
        return false;
    }

    const auto& row = directory.rows[identity.selectedRow];
    return row.blockIndexKnown &&
           row.blockIndex == identity.blockIndex &&
           row.rowNameKnown8007A590 &&
           std::strcmp(row.rowName8007A590,
                       identity.nameBuffer8007CBE8) == 0;
}

static bool SS0DirectQueueRuntimeLoadReplayState16CardIoResult80017594(
    const CardHandoff::LoadReplayState16CardIoResultCarrier80017594&
        carrier) {
    SS0DirectClearLoadReplayState16CardIoResult80017594();
    const bool runtimeEnabled = PrSS0Direct::RuntimeCutoverAllowed();
    const bool phaseAllowed =
        s_ss0Direct.phase == SS0DirectPhase::LoadCard ||
        s_ss0Direct.phase == SS0DirectPhase::ReplayCard;
    const auto expectedMode =
        s_ss0Direct.phase == SS0DirectPhase::ReplayCard
            ? CardHandoff::CardMode800191E4::Replay
            : CardHandoff::CardMode800191E4::Load;
    const bool state16Ready =
        CardHandoff::IsLoadReplayState16AwaitingPayload80019D7C(
            s_ss0Direct.cardDriverVisualRuntime80018FB0);
    const bool identityMatches =
        SS0DirectSelectedRowIdentityMatches800181D0();
    const int expectedBlock = identityMatches
        ? s_ss0Direct.cardSelectedRowIdentity800181D0.blockIndex
        : -1;

    CardHandoff::LoadReplayState16CardIoResultCarrier80017594 normalized =
        carrier;
    normalized.source =
        CardHandoff::LoadReplayState16CardIoResultSource80017594::
            RuntimeCardEventProducer;
    normalized.producerWired80017594 = true;
    const bool accepted = runtimeEnabled && s_ss0Direct.initialized &&
        phaseAllowed && state16Ready && identityMatches &&
        CardHandoff::IsRequestBoundLoadReplayState16CardIoResult80017594(
            normalized, expectedMode, expectedBlock);
    if (!accepted) {
        Log::Printf(
            "SS0 direct runtime: LOAD/REPLAY state16 80017594 carrier rejected runtime=%d initialized=%d phaseAllowed=%d state16=%d identity=%d mode=%s block=%d known=%d requestBound=%d stateKnown=%d state=%d ioKnown=%d io=%d incomplete=%d",
            runtimeEnabled ? 1 : 0,
            s_ss0Direct.initialized ? 1 : 0,
            phaseAllowed ? 1 : 0,
            state16Ready ? 1 : 0,
            identityMatches ? 1 : 0,
            CardHandoff::CardMode800191E4Name(expectedMode),
            expectedBlock,
            carrier.known ? 1 : 0,
            carrier.requestBound ? 1 : 0,
            carrier.stateKnown ? 1 : 0,
            carrier.state,
            carrier.ioResultKnown80017594 ? 1 : 0,
            carrier.ioResult80017594,
            carrier.incomplete ? 1 : 0);
        return false;
    }

    s_ss0Direct.cardState16IoResultCarrier80017594 = normalized;
    Log::Printf(
        "SS0 direct runtime: %s state16 request-bound 80017594 result=3 queued block=%d source=runtime-card-event-producer",
        CardHandoff::CardMode800191E4Name(expectedMode),
        expectedBlock);
    return true;
}

static void SS0DirectSyncCardGridVisualFromDriver80018FB0() {
    auto& state = s_ss0Direct.cardGridDrawState80020F94;
    const auto& runtime = s_ss0Direct.cardDriverVisualRuntime80018FB0;
    state.exitFrameState = runtime.exitFrameStateArg0;
    state.exitBlinkState = runtime.exitBlinkStateArg4;
    state.cardIoFlag = runtime.cardIoFlagArg8;
}

static void SS0DirectTickCardGridVisual80018FB0() {
    auto& state = s_ss0Direct.cardGridDrawState80020F94;
    if (!state.requestBound) {
        return;
    }
    auto& runtime = s_ss0Direct.cardDriverVisualRuntime80018FB0;
    if (!CardHandoff::TickCardDriverVisualRuntime80018FB0(&runtime)) {
        state = {};
        runtime = {};
        return;
    }
    SS0DirectSyncCardGridVisualFromDriver80018FB0();
}

static bool SS0DirectTickCardGridExitPromptFlash80017E6C(
    PrGameContext& ctx) {
    auto& runtime = s_ss0Direct.cardDriverVisualRuntime80018FB0;
    if (runtime.phase !=
            CardHandoff::CardDriverVisualPhase80018FB0::
                SelectionFlash80017E6C &&
        runtime.phase !=
            CardHandoff::CardDriverVisualPhase80018FB0::
                TerminalFrame80018FB0 &&
        runtime.phase !=
            CardHandoff::CardDriverVisualPhase80018FB0::
                FinalFlash80017E6C) {
        return false;
    }
    const bool state16SuccessCompletion =
        runtime.gp720Known && runtime.gp720 == 1;
    const bool state5ErrorPromptCompletion =
        !runtime.gp720Known && runtime.gp720 == 0 &&
        runtime.eventId == CardHandoff::CardEventFrameId::InsertCardPrompt;
    const auto completionKind = s_ss0Direct.cardCompletionKind80019D7C;
    const int completionReplayScene =
        s_ss0Direct.cardCompletionReplayScene80019D7C;
    const bool rollbackPrefixKnown =
        s_ss0Direct.cardCompletionRollbackPrefixKnown80092F10;
    const PrStage1SaveStatusPrefix80092F10 rollbackPrefix =
        s_ss0Direct.cardCompletionRollbackPrefix80092F10;
    const auto tick =
        CardHandoff::TickLoadReplayExitPromptFlash80017E6C(&runtime);
    if (tick == CardHandoff::CardDriverExitTickResult80017E6C::Rejected) {
        if (state16SuccessCompletion) {
            if (rollbackPrefixKnown) {
                if (!SS0DirectRollbackTypedPayloadCommit80092F10(
                        "LOAD/REPLAY state16 completion flash",
                        rollbackPrefix)) {
                    Log::Printf(
                        "SS0 direct runtime: LOAD/REPLAY state16 completion flash rollback GAP -> payload authority invalidated");
                }
            } else {
                PrStage1SaveUiDirect::
                    InvalidateSaveStatusPrefixAuthority80092F10(
                        PrStage1SaveStatusPrefix80092F10::kPsxAddress);
                s_ss0Direct.replayPayloadBackupPreflightKnown = false;
                Log::Printf(
                    "SS0 direct runtime: LOAD/REPLAY state16 completion flash missing rollback prefix -> payload authority invalidated");
            }
        }
        SS0DirectClearSelectedRowIdentity800181D0();
        SS0DirectClearLoadReplayCompletion80019D7C();
        SS0DirectResetLoadReplayVisualToList80018FB0();
        Log::Printf(
            "SS0 direct runtime: LOAD/REPLAY state23/final 80017E6C flash rejected successCompletion=%d",
            state16SuccessCompletion ? 1 : 0);
        return true;
    }
    SS0DirectSyncCardGridVisualFromDriver80018FB0();
    if (tick !=
        CardHandoff::CardDriverExitTickResult80017E6C::ReturnScene0) {
        return true;
    }

    SS0DirectClearTypedCardReadCarriers();
    SS0DirectClearReplayRawSelectorFixture();
    SS0DirectClearSelectedRowIdentity800181D0();
    if (!state16SuccessCompletion) {
        SS0DirectClearLoadReplayCompletion80019D7C();
        if (SS0DirectQueueMainMenuEntryTransition80026C90(ctx)) {
            Log::Printf(
                "SS0 direct runtime: LOAD/REPLAY %s 80017E6C flash complete -> main menu",
                state5ErrorPromptCompletion ? "state5/event12 prompt" : "EXIT");
        } else {
            Log::Printf(
                "SS0 direct runtime: LOAD/REPLAY %s 80017E6C flash complete -> title fallback",
                state5ErrorPromptCompletion ? "state5/event12 prompt" : "EXIT");
        }
        return true;
    }

    if (!rollbackPrefixKnown ||
        completionKind ==
            SS0DirectLoadReplayCompletionKind80019D7C::None) {
        if (rollbackPrefixKnown) {
            if (!SS0DirectRollbackTypedPayloadCommit80092F10(
                    "LOAD/REPLAY state16 completion owner",
                    rollbackPrefix)) {
                Log::Printf(
                    "SS0 direct runtime: LOAD/REPLAY state16 completion owner rollback GAP -> payload authority invalidated");
            }
        } else {
            PrStage1SaveUiDirect::InvalidateSaveStatusPrefixAuthority80092F10(
                PrStage1SaveStatusPrefix80092F10::kPsxAddress);
            s_ss0Direct.replayPayloadBackupPreflightKnown = false;
            Log::Printf(
                "SS0 direct runtime: LOAD/REPLAY state16 completion owner missing rollback prefix -> payload authority invalidated");
        }
        SS0DirectClearLoadReplayCompletion80019D7C();
        SS0DirectResetLoadReplayVisualToList80018FB0();
        Log::Printf(
            "SS0 direct runtime: LOAD/REPLAY state16 completion blocked after final flash: pending owner missing");
        return true;
    }

    if (completionKind ==
        SS0DirectLoadReplayCompletionKind80019D7C::LoadReturnMainMenu) {
        SS0DirectClearLoadReplayCompletion80019D7C();
        if (SS0DirectQueueMainMenuEntryTransition80026C90(ctx)) {
            Log::Printf(
                "SS0 direct runtime: load card state16 gp720=1/state23/final flash complete -> direct main menu statusSource=direct-bank");
        } else {
            Log::Printf(
                "SS0 direct runtime: load card state16 gp720=1/state23/final flash complete -> title fallback statusSource=direct-bank");
        }
        return true;
    }

    SS0DirectClearLoadReplayCompletion80019D7C();
    if (completionKind ==
            SS0DirectLoadReplayCompletionKind80019D7C::ReplayScene &&
        SS0DirectStartSceneTransition(ctx, completionReplayScene, 2)) {
        Log::Printf(
            "SS0 direct runtime: replay card state16 gp720=1/state23/final flash complete -> Scene%d statusSource=direct-bank",
            completionReplayScene);
        return true;
    }

    const bool transitionRollbackOk =
        SS0DirectRollbackTypedPayloadCommit80092F10(
            "replay card completion transition", rollbackPrefix);
    if (!transitionRollbackOk) {
        Log::Printf(
            "SS0 direct runtime: replay completion transition rollback GAP -> payload authority invalidated");
    }
    SS0DirectResetLoadReplayVisualToList80018FB0();
    Log::Printf(
        "SS0 direct runtime: replay card state16 completion transition blocked scene=%d rollbackOk=%d",
        completionReplayScene,
        transitionRollbackOk ? 1 : 0);
    return true;
}

static bool SS0DirectTickCardGridEntrySelectionFlash80017E6C(
    bool* state16Ready) {
    if (state16Ready == nullptr) {
        return false;
    }
    *state16Ready = false;
    auto& runtime = s_ss0Direct.cardDriverVisualRuntime80018FB0;
    if (runtime.phase !=
        CardHandoff::CardDriverVisualPhase80018FB0::
            EntrySelectionFlash80017E6C) {
        return false;
    }

    const auto tick =
        CardHandoff::TickListEntrySelectionFlash80017E6C(&runtime);
    if (tick == CardHandoff::CardDriverEntryTickResult80017E6C::Rejected) {
        SS0DirectClearSelectedRowIdentity800181D0();
        const auto mode = s_ss0Direct.cardListInputRuntime800181D0.mode;
        if (!CardHandoff::InitLoadReplayCardDriverVisualRuntime80018FB0(
                mode, &runtime)) {
            s_ss0Direct.cardGridDrawState80020F94 = {};
            runtime = {};
        } else {
            SS0DirectSyncCardGridVisualFromDriver80018FB0();
        }
        Log::Printf(
            "SS0 direct runtime: LOAD/REPLAY entry 80017E6C flash rejected");
        return true;
    }
    SS0DirectSyncCardGridVisualFromDriver80018FB0();
    if (tick !=
        CardHandoff::CardDriverEntryTickResult80017E6C::EnterState16) {
        return true;
    }

    SS0DirectSyncCardGridVisualFromDriver80018FB0();
    *state16Ready = true;
    const auto mode = s_ss0Direct.cardListInputRuntime800181D0.mode;
    Log::Printf(
        "SS0 direct runtime: %s card entry 800181D0/80018E10 flash complete -> persistent state16 cursor=%d",
        mode == CardHandoff::CardMode800191E4::Replay ? "replay" : "load",
        s_ss0Direct.cardCursor);
    return true;
}

static int SS0DirectTickCardPage(PrGameContext& ctx) {
    const bool isReplay = s_ss0Direct.phase == SS0DirectPhase::ReplayCard;
    const auto cardMode = isReplay
        ? CardHandoff::CardMode800191E4::Replay
        : CardHandoff::CardMode800191E4::Load;
    bool state16Ready =
        CardHandoff::IsLoadReplayState16AwaitingPayload80019D7C(
            s_ss0Direct.cardDriverVisualRuntime80018FB0);
    if (!state16Ready &&
        SS0DirectTickCardGridEntrySelectionFlash80017E6C(&state16Ready) &&
        !state16Ready) {
        return 0;
    }
    if (!state16Ready &&
        SS0DirectTickCardGridExitPromptFlash80017E6C(ctx)) {
        return 0;
    }
    if (state16Ready &&
        s_ss0Direct.cardState16IoResultCarrier80017594.known) {
        const auto carrier =
            s_ss0Direct.cardState16IoResultCarrier80017594;
        SS0DirectClearLoadReplayState16CardIoResult80017594();
        const int selectedBlock =
            s_ss0Direct.cardSelectedRowIdentity800181D0.blockIndexKnown
                ? s_ss0Direct.cardSelectedRowIdentity800181D0.blockIndex
                : -1;
        if (CardHandoff::ApplyLoadReplayState16CardIoResult80019D7C(
                &s_ss0Direct.cardDriverVisualRuntime80018FB0,
                carrier,
                cardMode,
                selectedBlock)) {
            SS0DirectClearTypedCardReadCarriers();
            SS0DirectClearReplayRawSelectorFixture();
            SS0DirectClearSelectedRowIdentity800181D0();
            SS0DirectClearLoadReplayCompletion80019D7C();
            SS0DirectSyncCardGridVisualFromDriver80018FB0();
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "CARD I/O ERROR");
            Log::Printf(
                "SS0 direct runtime: %s state16 exact 80017594 result=3 -> state5/event12 prompt idle block=%d gp720Known=0",
                isReplay ? "replay" : "load",
                selectedBlock);
            return 0;
        }
        Log::Printf(
            "SS0 direct runtime: %s state16 80017594 carrier dropped after request mismatch block=%d",
            isReplay ? "replay" : "load",
            selectedBlock);
    }
    if (CardHandoff::IsLoadReplayState5PromptIdle800180D8(
            s_ss0Direct.cardDriverVisualRuntime80018FB0)) {
        SS0DirectTickCardGridVisual80018FB0();
        const PrPadState pad = SS0DirectReadPad(ctx);
        const int32_t inputMask = static_cast<int32_t>(
            SS0DirectLocalPressedToPsxPadMask(pad.pressed));
        if (CardHandoff::BeginLoadReplayState5PromptFlash80017E6C(
                &s_ss0Direct.cardDriverVisualRuntime80018FB0,
                inputMask)) {
            SS0DirectSyncCardGridVisualFromDriver80018FB0();
            (void)PrSfx::PlayScene0ConfirmCue80025C8C();
            Log::Printf(
                "SS0 direct runtime: %s state5/event12 Cross -> 80017E6C arg4=1 arg8=0 frames=20",
                isReplay ? "replay" : "load");
        }
        return 0;
    }
    if (!state16Ready) {
        SS0DirectTickCardGridVisual80018FB0();
        const PrPadState pad = SS0DirectReadPad(ctx);
        const int32_t inputMask = static_cast<int32_t>(
            SS0DirectLocalPressedToPsxPadMask(pad.pressed));
        const auto inputResult =
            CardHandoff::TickLoadReplayListInput800181D0(
                &s_ss0Direct.cardListInputRuntime800181D0,
                inputMask);
        if (!inputResult.accepted) {
            return 0;
        }
        if (inputResult.action ==
            CardHandoff::LoadReplayListInputAction800181D0::
                MoveSelection) {
            SS0DirectClearTypedCardReadCarriers();
            SS0DirectClearReplayRawSelectorFixture();
            SS0DirectClearSelectedRowIdentity800181D0();
            SS0DirectSyncCardGridSelection80020F94();
            (void)PrSfx::PlayScene0NavigateCue80025C8C();
            Log::Printf(
                "SS0 direct runtime: %s card 800181D0 cursor=%d input=%04X",
                isReplay ? "replay" : "load",
                s_ss0Direct.cardCursor,
                static_cast<unsigned>(inputMask));
            return 0;
        }
        if (inputResult.action ==
            CardHandoff::LoadReplayListInputAction800181D0::SelectExit) {
            SS0DirectClearTypedCardReadCarriers();
            SS0DirectClearReplayRawSelectorFixture();
            SS0DirectClearSelectedRowIdentity800181D0();
            if (CardHandoff::BeginLoadReplayExitPromptFlash80017E6C(
                    &s_ss0Direct.cardDriverVisualRuntime80018FB0)) {
                SS0DirectSyncCardGridVisualFromDriver80018FB0();
                (void)PrSfx::PlayScene0ConfirmCue80025C8C();
                Log::Printf(
                    "SS0 direct runtime: %s card EXIT 800181D0/80018E10 flash started result=2 frames=20 entries=%d",
                    isReplay ? "replay" : "load",
                    s_ss0Direct.cardEntryCount);
            } else {
                (void)PrSfx::PlayScene0CancelCue80025C8C();
                Log::Printf(
                    "SS0 direct runtime: %s card EXIT flash blocked entries=%d",
                    isReplay ? "replay" : "load",
                    s_ss0Direct.cardEntryCount);
            }
            return 0;
        }
        if (inputResult.action !=
            CardHandoff::LoadReplayListInputAction800181D0::SelectEntry) {
            return 0;
        }
        SS0DirectClearLoadReplayState16CardIoResult80017594();
        CardHandoff::LoadReplaySelectedRowIdentity800181D0
            selectedIdentity{};
        if (!CardHandoff::CommitLoadReplaySelectedRowIdentity800181D0(
                s_ss0Direct.cardDirectoryCarrier80019D7C,
                inputResult,
                &selectedIdentity)) {
            SS0DirectClearSelectedRowIdentity800181D0();
            (void)PrSfx::PlayScene0CancelCue80025C8C();
            Log::Printf(
                "SS0 direct runtime: %s card entry 800181D0 blocked before flash: exact 8007A590 row name unknown cursor=%d",
                isReplay ? "replay" : "load",
                s_ss0Direct.cardCursor);
            return 0;
        }
        if (!CardHandoff::BeginListEntrySelectionFlash80017E6C(
                &s_ss0Direct.cardDriverVisualRuntime80018FB0,
                inputResult)) {
            SS0DirectClearSelectedRowIdentity800181D0();
            (void)PrSfx::PlayScene0CancelCue80025C8C();
            Log::Printf(
                "SS0 direct runtime: %s card entry 800181D0/80018E10 flash blocked cursor=%d",
                isReplay ? "replay" : "load",
                s_ss0Direct.cardCursor);
            return 0;
        }
        s_ss0Direct.cardSelectedRowIdentity800181D0 = selectedIdentity;
        s_ss0Direct.cardSelectedRowDirectoryGeneration =
            s_ss0Direct.cardDirectoryGeneration;
        SS0DirectSyncCardGridVisualFromDriver80018FB0();
        (void)PrSfx::PlayScene0ConfirmCue80025C8C();
        Log::Printf(
            "SS0 direct runtime: %s card entry 800181D0/80018E10 flash started result=1 cursor=%d block=%d gp716=1 name8007CBE8Known=1 arg2=%d arg3=%d frames=20",
            isReplay ? "replay" : "load",
            s_ss0Direct.cardCursor,
            selectedIdentity.blockIndex,
            inputResult.promptArg2,
            inputResult.promptArg3);
        return 0;
    }

    if (s_ss0Direct.cardEntryCount > 15) {
        SS0DirectClearTypedCardReadCarriers();
        SS0DirectClearReplayRawSelectorFixture();
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "%s INDEX GAP - O:BACK",
                      isReplay ? "REPLAY" : "LOAD");
        Log::Printf(
            "SS0 direct runtime: %s card invalid entries=%d before block index",
            isReplay ? "replay" : "load",
            s_ss0Direct.cardEntryCount);
        return 0;
    }

    if (s_ss0Direct.cardCursor < 0 ||
        s_ss0Direct.cardCursor >= s_ss0Direct.cardEntryCount ||
        s_ss0Direct.cardCursor >= 15) {
        SS0DirectClearTypedCardReadCarriers();
        SS0DirectClearReplayRawSelectorFixture();
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "%s INDEX GAP - O:BACK",
                      isReplay ? "REPLAY" : "LOAD");
        Log::Printf(
            "SS0 direct runtime: %s card invalid cursor=%d entries=%d before block index",
            isReplay ? "replay" : "load",
            s_ss0Direct.cardCursor,
            s_ss0Direct.cardEntryCount);
        return 0;
    }

    if (!SS0DirectSelectedRowIdentityMatches800181D0()) {
        SS0DirectClearTypedCardReadCarriers();
        SS0DirectClearReplayRawSelectorFixture();
        SS0DirectClearSelectedRowIdentity800181D0();
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "%s NAME GAP - O:BACK",
                      isReplay ? "REPLAY" : "LOAD");
        Log::Printf(
            "SS0 direct runtime: %s card state16 blocked: request-bound 800181D0 gp716/8007CBE8 identity missing cursor=%d generation=%u",
            isReplay ? "replay" : "load",
            s_ss0Direct.cardCursor,
            static_cast<unsigned>(s_ss0Direct.cardDirectoryGeneration));
        return 0;
    }

    const int blockIndex = s_ss0Direct.cardBlockIndex[s_ss0Direct.cardCursor];
    if (blockIndex < 0 || blockIndex >= 15) {
        SS0DirectClearTypedCardReadCarriers();
        SS0DirectClearReplayRawSelectorFixture();
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "%s INDEX GAP - O:BACK",
                      isReplay ? "REPLAY" : "LOAD");
        Log::Printf(
            "SS0 direct runtime: %s card invalid block index=%d cursor=%d entries=%d",
            isReplay ? "replay" : "load",
            blockIndex,
            s_ss0Direct.cardCursor,
            s_ss0Direct.cardEntryCount);
        return 0;
    }

    if (isReplay && !s_ss0Direct.replayPayloadBackupPreflightKnown) {
        SS0DirectClearTypedCardReadCarriers();
        SS0DirectClearReplayRawSelectorFixture();
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "REPLAY BACKUP GAP - O:BACK");
        Log::Printf(
            "SS0 direct runtime: replay card block=%d blocked before load: 80015700 backup missing",
            blockIndex);
        return 0;
    }

    // Give an explicitly injected typed-facts fixture its original priority,
    // then fall back to the durable card image.  The directory page is
    // authoritative for the selected physical card frame; materialize the
    // matching state16 read carrier only after the original 800181D0 entry
    // flash has committed the row identity.  Using the startup slot-policy
    // block here would read the wrong save when a card contains more than one
    // entry.
    SS0DirectImportPendingState16TypedFactsBeforeConfirm(
        blockIndex,
        isReplay ? "replay card" : "load card");

    PrStage1SaveCardHalDirect::State16CardReadTypedCarrier800179B4
        selectedState16Carrier{};
    if (!PrStage1SaveCardHalDirect::GetState16CardReadTypedCarrier800179B4(
            &selectedState16Carrier) &&
        !SS0DirectPublishSelectedState16FromDirectCardImageSink(blockIndex)) {
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "%s AUTH PENDING",
                      isReplay ? "REPLAY" : "LOAD");
        Log::Printf(
            "SS0 direct runtime: %s card block=%d state16 direct card-image source unavailable",
            isReplay ? "replay" : "load",
            blockIndex);
        return 0;
    }

    if (!isReplay) {
        SS0DirectTypedPayloadCommitSeam loadPayload{};
        if (!SS0DirectRequireTypedCardReadAuthority(
                "load card",
                SS0DirectTypedCardReadLane::State16LoadPayload80019D7C,
                blockIndex,
                &loadPayload)) {
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "LOAD AUTH PENDING");
            Log::Printf(
                "SS0 direct runtime: load card block=%d state16 waiting: payload authority missing",
                blockIndex);
            return 0;
        }
        const PrStage1SaveStatusPrefix80092F10 loadPreCommitPrefix =
            PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
        if (!SS0DirectCanRollbackTypedPayloadCommit80092F10(
                loadPreCommitPrefix)) {
            (void)PrSfx::PlayScene0CancelCue80025C8C();
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "LOAD PREFIX GAP - O:BACK");
            Log::Printf(
                "SS0 direct runtime: load card block=%d blocked before payload commit: rollback prefix missing known=%d helperGap=%d statusKnown=%d addr=%08X bytes=%u",
                blockIndex,
                loadPreCommitPrefix.known ? 1 : 0,
                loadPreCommitPrefix.helperGap ? 1 : 0,
                loadPreCommitPrefix.statusBankKnown80092F1D ? 1 : 0,
                loadPreCommitPrefix.psxAddress,
                loadPreCommitPrefix.byteCount);
            return 0;
        }
        if (!SS0DirectBackupReplayPayload80015700("load card commit")) {
            (void)PrSfx::PlayScene0CancelCue80025C8C();
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "LOAD BACKUP GAP - O:BACK");
            Log::Printf(
                "SS0 direct runtime: load card block=%d blocked before payload commit: fresh 80015700 backup missing",
                blockIndex);
            return 0;
        }
        PrStage1SavePayloadProducerResult loadCommit{};
        if (!SS0DirectCommitTypedPayload800164B4("load card",
                                                 loadPayload,
                                                 &loadCommit)) {
            if (!SS0DirectRollbackTypedPayloadCommit80092F10(
                    "load card", loadPreCommitPrefix)) {
                Log::Printf(
                    "SS0 direct runtime: load card commit rollback GAP -> payload authority invalidated");
            }
            (void)PrSfx::PlayScene0CancelCue80025C8C();
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "LOAD COMMIT GAP - O:BACK");
            Log::Printf(
                "SS0 direct runtime: load card block=%d typed payload commit failed before cutover ok=%d payloadKnown=%d lastFault=%08X",
                blockIndex,
                loadCommit.ok ? 1 : 0,
                loadCommit.payloadKnown ? 1 : 0,
                loadCommit.lastFaultAddress);
            return 0;
        }
        if (!SS0DirectArmLoadReplayCompletion80019D7C(
                SS0DirectLoadReplayCompletionKind80019D7C::
                    LoadReturnMainMenu,
                -1,
                loadPreCommitPrefix)) {
            if (!SS0DirectRollbackTypedPayloadCommit80092F10(
                    "load card completion arm", loadPreCommitPrefix)) {
                Log::Printf(
                    "SS0 direct runtime: load completion arm rollback GAP -> payload authority invalidated");
            }
            (void)PrSfx::PlayScene0CancelCue80025C8C();
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "LOAD COMPLETE GAP");
            Log::Printf(
                "SS0 direct runtime: load card block=%d payload committed but gp720/state23 completion arm failed -> rollback",
                blockIndex);
            return 0;
        }
        SS0DirectSyncCardGridVisualFromDriver80018FB0();
        Log::Printf(
            "SS0 direct runtime: load card block=%d typed payload committed via 800164B4 seamBlock=%d bytes=%zu -> gp720=1/state23 terminal frame",
            blockIndex,
            loadPayload.blockIndex,
            loadPayload.byteCount);
        return 0;
    }

    const bool debugReplayRawSelectorFixture =
        s_ss0Direct.debugReplayRawSelectorArmed &&
        s_ss0Direct.debugReplayRawSelectorBlock == blockIndex;
    SS0DirectTypedPayloadCommitSeam replayPayload{};
    if (!SS0DirectRequireTypedCardReadAuthority(
            "replay card",
            SS0DirectTypedCardReadLane::State16LoadPayload80019D7C,
            blockIndex,
            &replayPayload)) {
        if (debugReplayRawSelectorFixture) {
            SS0DirectClearReplayRawSelectorFixture();
            Log::Printf(
                "SS0 direct runtime: debug replay raw selector blocked before map: payload authority missing");
        }
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "REPLAY AUTH PENDING");
        Log::Printf(
            "SS0 direct runtime: replay card block=%d state16 waiting: payload authority missing",
            blockIndex);
        return 0;
    }

    const PrStage1SaveStatusPrefix80092F10 replayPreCommitPrefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    if (!SS0DirectCanRollbackTypedPayloadCommit80092F10(
            replayPreCommitPrefix)) {
        if (debugReplayRawSelectorFixture) {
            SS0DirectClearReplayRawSelectorFixture();
        }
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "REPLAY PREFIX GAP - O:BACK");
        Log::Printf(
            "SS0 direct runtime: replay card block=%d blocked before payload commit: rollback prefix missing known=%d helperGap=%d statusKnown=%d addr=%08X bytes=%u",
            blockIndex,
            replayPreCommitPrefix.known ? 1 : 0,
            replayPreCommitPrefix.helperGap ? 1 : 0,
            replayPreCommitPrefix.statusBankKnown80092F1D ? 1 : 0,
            replayPreCommitPrefix.psxAddress,
            replayPreCommitPrefix.byteCount);
        return 0;
    }

    if (!SS0DirectBackupReplayPayload80015700("replay card commit")) {
        if (debugReplayRawSelectorFixture) {
            SS0DirectClearReplayRawSelectorFixture();
        }
        s_ss0Direct.replayPayloadBackupPreflightKnown = false;
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "REPLAY BACKUP GAP - O:BACK");
        Log::Printf(
            "SS0 direct runtime: replay card block=%d blocked before payload commit: fresh 80015700 backup missing",
            blockIndex);
        return 0;
    }
    s_ss0Direct.replayPayloadBackupPreflightKnown = true;

    PrStage1SavePayloadProducerResult replayCommit{};
    if (!SS0DirectCommitTypedPayload800164B4("replay card",
                                             replayPayload,
                                             &replayCommit)) {
        if (debugReplayRawSelectorFixture) {
            SS0DirectClearReplayRawSelectorFixture();
        }
        if (!SS0DirectRollbackTypedPayloadCommit80092F10(
                "replay card", replayPreCommitPrefix)) {
            Log::Printf(
                "SS0 direct runtime: replay card commit rollback GAP -> payload authority invalidated");
        }
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "REPLAY COMMIT GAP - O:BACK");
        Log::Printf(
            "SS0 direct runtime: replay card block=%d typed payload commit failed before selector ok=%d payloadKnown=%d lastFault=%08X",
            blockIndex,
            replayCommit.ok ? 1 : 0,
            replayCommit.payloadKnown ? 1 : 0,
            replayCommit.lastFaultAddress);
        return 0;
    }

    SS0DirectDebugReplayLoadResult replayLoad{};
    if (debugReplayRawSelectorFixture) {
        SS0DirectClearReplayRawSelectorFixture();
    }
    if (!SS0DirectLoadReplaySelectorFromCommittedPayload80092F3C(
            "replay card",
            &replayLoad)) {
        if (!SS0DirectRollbackTypedPayloadCommit80092F10(
                "replay card", replayPreCommitPrefix)) {
            Log::Printf(
                "SS0 direct runtime: replay selector rollback GAP -> payload authority invalidated");
        }
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "REPLAY SELECT GAP - O:BACK");
        Log::Printf(
            "SS0 direct runtime: replay card block=%d typed payload committed via 800164B4 seamBlock=%d bytes=%zu direct replay selector missing after payload commit",
            blockIndex,
            replayPayload.blockIndex,
            replayPayload.byteCount);
        return 0;
    }

    const int replayScene = replayLoad.replayScene;
    Log::Printf(
        "SS0 direct runtime: replay card block=%d loaded replayScene=%d committed=%d rawSelKnown=%d rawSel=%u",
                blockIndex,
                replayScene,
                replayLoad.payloadCommitted ? 1 : 0,
                replayLoad.rawSavedSlotKnown ? 1 : 0,
                replayLoad.rawSavedSlot);
    const bool replaySceneValid = replayScene >= 1 && replayScene <= 6;
    if (replaySceneValid && !replayLoad.payloadCommitted) {
        if (!SS0DirectRollbackTypedPayloadCommit80092F10(
                "replay card", replayPreCommitPrefix)) {
            Log::Printf(
                "SS0 direct runtime: replay commit ownership rollback GAP -> payload authority invalidated");
        }
        std::snprintf(s_ss0Direct.cardMessage,
                      sizeof(s_ss0Direct.cardMessage),
                      "REPLAY COMMIT GAP - O:BACK");
        (void)PrSfx::PlayScene0CancelCue80025C8C();
        Log::Printf(
            "SS0 direct runtime: replay card block=%d valid replayScene=%d without payload commit",
            blockIndex,
            replayScene);
        return 0;
    }
    if (replaySceneValid) {
        if (!SS0DirectArmLoadReplayCompletion80019D7C(
                SS0DirectLoadReplayCompletionKind80019D7C::ReplayScene,
                replayScene,
                replayPreCommitPrefix)) {
            if (!SS0DirectRollbackTypedPayloadCommit80092F10(
                    "replay card", replayPreCommitPrefix)) {
                Log::Printf(
                    "SS0 direct runtime: replay completion arm rollback GAP -> payload authority invalidated");
            }
            (void)PrSfx::PlayScene0CancelCue80025C8C();
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "REPLAY COMPLETE GAP");
            Log::Printf(
                "SS0 direct runtime: replay card block=%d valid replayScene=%d payload committed but gp720/state23 completion arm failed -> rollback",
                blockIndex,
                replayScene);
            return 0;
        }
        SS0DirectSyncCardGridVisualFromDriver80018FB0();
        Log::Printf(
            "SS0 direct runtime: replay card block=%d valid replayScene=%d -> gp720=1/state23 terminal frame statusSource=direct-bank",
            blockIndex,
            replayScene);
        return 0;
    }

    if (!SS0DirectRollbackTypedPayloadCommit80092F10(
            "replay card", replayPreCommitPrefix)) {
        Log::Printf(
            "SS0 direct runtime: invalid replay scene rollback GAP -> payload authority invalidated");
    }
    (void)PrSfx::PlayScene0CancelCue80025C8C();
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "NO REPLAY SCENE - O:BACK");
    Log::Printf(
        "SS0 direct runtime: replay card block=%d invalid replayScene=%d committed=%d rawSelKnown=%d rawSel=%u",
                blockIndex,
                replayScene,
                replayLoad.payloadCommitted ? 1 : 0,
                replayLoad.rawSavedSlotKnown ? 1 : 0,
                replayLoad.rawSavedSlot);
    return 0;
}

static void SS0DirectRunEvent17Tick80026B54(PrGameContext& ctx) {
    const auto tick = PrSS0DirectoryDispatcherDirect::TickOptions80026B54(
        s_ss0Direct.optionsState,
        s_ss0Direct.blinkCounter800916E4,
        s_ss0Direct.optionsBlink);
    s_ss0Direct.optionsState = tick.state;
    s_ss0Direct.blinkCounter800916E4 = tick.blink.counter800916E4;
    s_ss0Direct.optionsBlink = tick.blink.contextBlinkOnOff;
    SS0DirectPublishOptionsState80026910(ctx);
}

static bool SS0DirectArmOptionsResultTail80026B94(
    const PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::DispatcherTailState80026B94& tail,
    int result) {
    if (result != 1 &&
        result != PrSS0EventFrameLoopDirect::kEvent17TimeoutResult80026B94) {
        Log::Printf(
            "SS0 direct runtime: event17 80026B94 unexpected result=%d",
            result);
        return false;
    }
    if (!PrSS0EventFrameLoopDirect::ArmDispatcherTail80026B94(
            tail, result)) {
        Log::Printf(
            "SS0 direct runtime: event17 80026B94 tail arm rejected result=%d",
            result);
        return false;
    }
    Log::Printf(
        "SS0 direct runtime: event17 80026B94 result=%d tail armed frames=%d frame=%u",
        result,
        PrSS0EventFrameLoopDirect::kDispatcherResultTailFrames80026B94,
        static_cast<unsigned>(ctx.frame));
    return true;
}

static const char* SS0DirectEvent17FrameClassName80026B94(
    PrSS0EventFrameLoopDirect::Event17FrameClass80026B94 frameClass) {
    using FrameClass =
        PrSS0EventFrameLoopDirect::Event17FrameClass80026B94;
    switch (frameClass) {
    case FrameClass::OpenInputLoop:
        return "open-input";
    case FrameClass::ResultAction:
        return "result-action";
    case FrameClass::ClosedInputTail:
        return "closed-input-tail";
    default:
        return "unknown";
    }
}

static bool SS0DirectBeginOptionsFrameTransaction80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event17FrameClass80026B94 frameClass,
    bool inputClosed,
    int32_t tailFramesRemainingBefore,
    int32_t tailFramesRemainingAfter,
    const PrSS0EventFrameLoopDirect::DispatcherTailState80026B94& tailCandidate) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    s_ss0Direct.optionsFrameTailCandidate80026B94 = {};
    s_ss0Direct.optionsFrameTailCandidateKnown80026B94 = false;
    EventFrame::Event17FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = EventFrame::kCtxEvent17_8005451C;
    begin.logicFrame = ctx.frame;
    begin.frameClass = frameClass;
    begin.inputClosed = inputClosed;
    begin.tailFramesRemainingBefore = tailFramesRemainingBefore;
    begin.tailFramesRemainingAfter = tailFramesRemainingAfter;
    if (!EventFrame::BeginEvent17FrameTransaction80026B94(
            s_ss0Direct.optionsFrameTransaction80026B94, begin)) {
        Log::Printf(
            "SS0 direct runtime: event17 frame transaction begin rejected frame=%u class=%s inputClosed=%d tail=%d/%d",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent17FrameClassName80026B94(frameClass),
            inputClosed ? 1 : 0,
            tailFramesRemainingBefore,
            tailFramesRemainingAfter);
        EventFrame::ResetEvent17FrameTransaction80026B94(
            s_ss0Direct.optionsFrameTransaction80026B94);
        return false;
    }

    // 80026B94 calls the Event17 tick before the draw route on every active
    // frame. The direct 8001E750 owner preamble below binds the formal
    // 8001D74C(4) work-list slot before the draw; host wait/end/text remain
    // fail-closed until their own transaction steps are complete.
    SS0DirectRunEvent17Tick80026B54(ctx);
    if (!EventFrame::CommitEvent17FrameTick80026B54(
            s_ss0Direct.optionsFrameTransaction80026B94,
            ctx.frame,
            EventFrame::kEventTable17_800545A0,
            EventFrame::kCtxEvent17_8005451C)) {
        Log::Printf(
            "SS0 direct runtime: event17 frame transaction tick rejected frame=%u class=%s",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent17FrameClassName80026B94(frameClass));
        EventFrame::ResetEvent17FrameTransaction80026B94(
            s_ss0Direct.optionsFrameTransaction80026B94);
        return false;
    }

    // Formal 8001E750 selects the current draw buffer through 8004019C,
    // stores it in gp+0x368, then prepares that exact 80087288 work-list
    // before the case-17 renderer. Keep this owner binding in SS0's direct
    // frame state; it is deliberately independent of the legacy S0 shell.
    PrPsxEventFrameDirect::BeginDrawWrapper8001E750(
        s_ss0Direct.optionsFrameHostState8001E750, 17);
    s_ss0Direct.optionsFrameEndFrameWorkListSlot8001EA00 =
        s_ss0Direct.optionsFrameHostState8001E750.gp368WorkSlot & 1u;
    s_ss0Direct.optionsFrameEndFrameBindingKnown8001EA00 = true;
    s_ss0Direct.optionsFrameEndFrameBindingBlockedLogged8001EA00 = false;

    s_ss0Direct.optionsFrameTailCandidate80026B94 = tailCandidate;
    s_ss0Direct.optionsFrameTailCandidateKnown80026B94 = true;
    return true;
}

static void SS0DirectCommitOptionsFrameTail80026B94() {
    if (!s_ss0Direct.optionsFrameTailCandidateKnown80026B94) {
        return;
    }
    s_ss0Direct.optionsDispatcherTail80026B94 =
        s_ss0Direct.optionsFrameTailCandidate80026B94;
    s_ss0Direct.optionsFrameTailCandidate80026B94 = {};
    s_ss0Direct.optionsFrameTailCandidateKnown80026B94 = false;
}

static void SS0DirectConsumeOptionsResult80015788(
    PrGameContext& ctx,
    int result) {
    if (result != 1 &&
        result != PrSS0EventFrameLoopDirect::kEvent17TimeoutResult80026B94) {
        Log::Printf(
            "SS0 direct runtime: event17 80015788 ignored unexpected result=%d",
            result);
        return;
    }

    const bool queued = SS0DirectQueueMainMenuEntryTransition80026C90(ctx);
    if (result == 1) {
        if (queued) {
            Log::Printf(
                "SS0 direct runtime: options done lang=%d subtitle=%d",
                s_ss0Direct.optionsWord800916D8,
                s_ss0Direct.optionsWord800916DC);
        } else {
            Log::Printf(
                "SS0 direct runtime: options done blocked -> title fallback");
        }
        return;
    }

    Log::Printf(
        queued
            ? "SS0 direct runtime: options initial-input timeout -> main menu"
            : "SS0 direct runtime: options initial-input timeout blocked -> title fallback");
}

static bool SS0DirectTickOptionsResultRelease80015788(PrGameContext& ctx) {
    if (!s_ss0Direct.optionsResultPending80015788) {
        return false;
    }

    const PrPadState pad = SS0DirectReadPad(ctx);
    const uint32_t currentPad =
        SS0DirectFullLocalPadToPsxMask80026744(
            static_cast<uint16_t>(pad.held | pad.pressed));
    const auto releaseStep =
        PrSS0EventFrameLoopDirect::StepDispatcherPreLoopPadRelease80026B94(
            s_ss0Direct.optionsResultReleaseGate80015788,
            currentPad);
    if (releaseStep.kind == PrSS0EventFrameLoopDirect::
                                DispatcherPreLoopPadReleaseStepKind80026B94::
                                    WaitingForRelease) {
        if (!s_ss0Direct.optionsResultReleaseBlockedLogged80015788) {
            s_ss0Direct.optionsResultReleaseBlockedLogged80015788 = true;
            Log::Printf(
                "SS0 direct runtime: event17 80015788 waiting outer pad release pad=%04X frame=%u",
                static_cast<unsigned>(currentPad),
                static_cast<unsigned>(ctx.frame));
        }
        return true;
    }

    const int result = s_ss0Direct.optionsResultPendingValue80015788;
    s_ss0Direct.optionsResultPending80015788 = false;
    s_ss0Direct.optionsResultPendingValue80015788 = 0;
    s_ss0Direct.optionsResultReleaseBlockedLogged80015788 = false;
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.optionsResultReleaseGate80015788);
    Log::Printf(
        "SS0 direct runtime: event17 80015788 outer pad release complete frame=%u",
        static_cast<unsigned>(ctx.frame));
    SS0DirectConsumeOptionsResult80015788(ctx, result);
    return true;
}

static int SS0DirectTickOptions(PrGameContext& ctx) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    if (SS0DirectTickOptionsResultRelease80015788(ctx)) {
        return 0;
    }
    auto& frameTransaction = s_ss0Direct.optionsFrameTransaction80026B94;
    if (!EventFrame::CanAdvanceEvent17FrameLogic80026B94(frameTransaction)) {
        if (!frameTransaction.logicAdvanceBlockedLogged) {
            frameTransaction.logicAdvanceBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: event17 pending frame blocks logicFrame=%u hostFrame=%u tickCommitted=%d drawSubmitted=%d",
                static_cast<unsigned>(frameTransaction.logicFrame),
                static_cast<unsigned>(ctx.frame),
                frameTransaction.tickCommitted ? 1 : 0,
                frameTransaction.directDrawSubmitted ? 1 : 0);
        }
        return 0;
    }

    auto tailCandidate = s_ss0Direct.optionsDispatcherTail80026B94;
    EventFrame::DispatcherTailStep80026B94 tailStep{};
    if (!EventFrame::TryStepEvent17DispatcherTail80026B94(
            frameTransaction, tailCandidate, tailStep)) {
        return 0;
    }
    if (tailStep.kind == EventFrame::DispatcherTailStepKind80026B94::
                             ReleaseResult) {
        s_ss0Direct.optionsDispatcherTail80026B94 = tailCandidate;
        EventFrame::ResetEvent17FrameTransaction80026B94(frameTransaction);
        s_ss0Direct.optionsFrameTailCandidate80026B94 = {};
        s_ss0Direct.optionsFrameTailCandidateKnown80026B94 = false;
        Log::Printf(
            "SS0 direct runtime: event17 80026B94 tail released result=%d frame=%u",
            tailStep.releasedResult,
            static_cast<unsigned>(ctx.frame));
        s_ss0Direct.optionsResultPending80015788 = true;
        s_ss0Direct.optionsResultPendingValue80015788 =
            tailStep.releasedResult;
        s_ss0Direct.optionsResultReleaseBlockedLogged80015788 = false;
        EventFrame::ResetDispatcherPreLoopPadRelease80026B94(
            s_ss0Direct.optionsResultReleaseGate80015788);
        return 0;
    }

    PrSfx::PlayBgm();
    if (tailStep.kind == EventFrame::DispatcherTailStepKind80026B94::
                             HoldFrame) {
        (void)SS0DirectBeginOptionsFrameTransaction80026B94(
            ctx,
            EventFrame::Event17FrameClass80026B94::ClosedInputTail,
            true,
            tailStep.framesRemainingBefore,
            tailStep.framesRemainingAfter,
            tailCandidate);
        return 0;
    }

    const PrPadState pad = SS0DirectReadPad(ctx);
    const uint32_t currentEvent17Pad =
        SS0DirectFullLocalPadToPsxMask80026744(
            static_cast<uint16_t>(pad.held | pad.pressed));
    const auto releaseStep =
        PrSS0EventFrameLoopDirect::StepDispatcherPreLoopPadRelease80026B94(
            s_ss0Direct.optionsPreLoopPadRelease80026B94,
            currentEvent17Pad);
    if (releaseStep.kind == PrSS0EventFrameLoopDirect::
                                DispatcherPreLoopPadReleaseStepKind80026B94::
                                    WaitingForRelease) {
        EventFrame::ResetEvent17FrameTransaction80026B94(frameTransaction);
        s_ss0Direct.optionsFrameTailCandidate80026B94 = {};
        s_ss0Direct.optionsFrameTailCandidateKnown80026B94 = false;
        if (!s_ss0Direct.optionsPreLoopPadReleaseBlockedLogged) {
            s_ss0Direct.optionsPreLoopPadReleaseBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: event17 pre-loop 80035510 waiting pad=%04X frame=%u",
                static_cast<unsigned>(currentEvent17Pad),
                static_cast<unsigned>(ctx.frame));
        }
        return 0;
    }
    if (releaseStep.kind == PrSS0EventFrameLoopDirect::
                                DispatcherPreLoopPadReleaseStepKind80026B94::
                                    ReleasedEnterLoop) {
        EventFrame::ResetDispatcherPadChange80026744(
            s_ss0Direct.optionsPadChange80026744);
        Log::Printf(
            "SS0 direct runtime: event17 pre-loop 80035510 released frame=%u",
            static_cast<unsigned>(ctx.frame));
    }
    const auto padChange = EventFrame::StepDispatcherPadChange80026744(
        s_ss0Direct.optionsPadChange80026744,
        currentEvent17Pad);
    const uint32_t psxPad =
        padChange.changed ? padChange.changedPadMask80026744 : 0u;
    const int previousCursor = s_ss0Direct.optionsState.cursor;
    const int previousLang = s_ss0Direct.optionsState.word800916D8;
    const int previousSub = s_ss0Direct.optionsState.word800916DC;
    const int previousCooldown = s_ss0Direct.optionsState.cooldown;
    bool handledInput = false;
    int result = 0;

    const auto timeoutStep =
        PrSS0EventFrameLoopDirect::StepEvent17InitialInputTimeout80026B94(
            s_ss0Direct.optionsInitialInputTimeout80026B94,
            psxPad);
    if (timeoutStep.kind == PrSS0EventFrameLoopDirect::
                                Event17InitialInputTimeoutStepKind80026B94::
                                    DisabledByInput) {
        Log::Printf(
            "SS0 direct runtime: event17 initial-input timeout disabled pad=%04X remaining=%d frame=%u",
            static_cast<unsigned>(psxPad),
            timeoutStep.framesRemainingAfter,
            static_cast<unsigned>(ctx.frame));
    }

    if (psxPad != 0u) {
        handledInput = true;
        const auto handled =
            PrSS0DirectoryDispatcherDirect::HandleOptions80026910(
                s_ss0Direct.optionsState,
                psxPad);
        // 80026910 writes its ctx+0x1C language snapshot before 80025DBC.
        // Publish only that internal side effect before the cue, then commit
        // the remaining context/carrier writes after the cue.
        if (handled.languageSnapshotWrittenBeforeCue) {
            s_ss0Direct.optionsState.languageBeforeInputSnapshotCtx1C =
                handled.state.languageBeforeInputSnapshotCtx1C;
        }
        SS0DirectPlayOptionsInputCue80026910(
            ctx, handled.cueKind, handled.cueCode80025C8C);
        s_ss0Direct.optionsState = handled.state;
        result = handled.result;
    } else if (timeoutStep.kind == PrSS0EventFrameLoopDirect::
                                       Event17InitialInputTimeoutStepKind80026B94::
                                           TimeoutResult) {
        result = timeoutStep.result;
    }

    if (result != 0) {
        if (!SS0DirectArmOptionsResultTail80026B94(
                ctx, tailCandidate, result)) {
            EventFrame::ResetEvent17FrameTransaction80026B94(frameTransaction);
            s_ss0Direct.optionsFrameTailCandidate80026B94 = {};
            s_ss0Direct.optionsFrameTailCandidateKnown80026B94 = false;
            return 0;
        }
    }

    if (handledInput && previousCooldown > 0) {
        Log::Printf(
            "SS0 direct runtime: event17 input blocked by cooldown pad=%04X before=%d after=%d frame=%u",
            static_cast<unsigned>(psxPad),
            previousCooldown,
            s_ss0Direct.optionsState.cooldown,
            static_cast<unsigned>(ctx.frame));
    } else if (s_ss0Direct.optionsState.cursor != previousCursor) {
        Log::Printf(
            "SS0 direct runtime: options cursor=%d frame=%u",
            s_ss0Direct.optionsState.cursor,
            static_cast<unsigned>(ctx.frame));
    } else if (s_ss0Direct.optionsState.word800916D8 != previousLang ||
               s_ss0Direct.optionsState.word800916DC != previousSub) {
        Log::Printf(
            "SS0 direct runtime: options changed lang=%d subtitle=%d cooldown=%d frame=%u",
            // The handler has just committed the direct ctx.  The public
            // mirrors are published by the next 80026B54 tick, so logging
            // those mirrors here reported the previous language/subtitle
            // value (and hid the OFF focus transition).  Use the committed
            // Event17 state, exactly the fields consumed by 80021910.
            s_ss0Direct.optionsState.word800916D8,
            s_ss0Direct.optionsState.word800916DC,
               s_ss0Direct.optionsState.cooldown,
               static_cast<unsigned>(ctx.frame));
    }

    const bool resultActionFrame =
        tailCandidate.active &&
        tailCandidate.framesRemaining ==
            EventFrame::kDispatcherResultTailFrames80026B94;
    const EventFrame::Event17FrameClass80026B94 frameClass =
        resultActionFrame
            ? EventFrame::Event17FrameClass80026B94::ResultAction
            : EventFrame::Event17FrameClass80026B94::OpenInputLoop;
    const int32_t tailFrameMarker = resultActionFrame
        ? EventFrame::kDispatcherResultTailFrames80026B94
        : -1;
    (void)SS0DirectBeginOptionsFrameTransaction80026B94(
        ctx,
        frameClass,
        false,
        tailFrameMarker,
        tailFrameMarker,
        tailCandidate);
    return 0;
}

static void SS0DirectConsumeStageSelectResult(PrGameContext& ctx,
                                               int result) {
    if (result == 1) {
        const int targetScene = s_ss0Direct.stageSelectState.outScene;
        if (targetScene < 1 || targetScene > 6) {
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "STAGE TARGET GAP - O:BACK");
            Log::Printf(
                "SS0 direct runtime: stage select target gap cursor=%d target=%d",
                s_ss0Direct.stageSelectState.cursor,
                targetScene);
            return;
        }
        if (!SS0DirectStartSceneTransition(ctx, targetScene, 0)) {
            std::snprintf(s_ss0Direct.cardMessage,
                          sizeof(s_ss0Direct.cardMessage),
                          "TRANSITION BUSY - X:OK");
            Log::Printf(
                "SS0 direct runtime: stage select cursor=%d transition busy target=%d",
                s_ss0Direct.stageSelectState.cursor,
                targetScene);
            return;
        }
        Log::Printf("SS0 direct runtime: stage select cursor=%d -> Scene%d",
                    s_ss0Direct.stageSelectState.cursor,
                    targetScene);
        return;
    }
    if (result == 2) {
        if (SS0DirectQueueMainMenuEntryTransition80026C90(ctx)) {
            Log::Printf("SS0 direct runtime: stage select cancel -> main menu");
        } else {
            Log::Printf(
                "SS0 direct runtime: stage select cancel -> title fallback");
        }
        return;
    }
    Log::Printf(
        "SS0 direct runtime: event2 80026B94 unexpected released result=%d",
        result);
}

static void SS0DirectRunEvent2Tick80026170() {
    const auto blink = PrSS0DirectoryDispatcherDirect::TickBlink80025D70(
        s_ss0Direct.blinkCounter800916E4,
        s_ss0Direct.stageSelectBlink);
    s_ss0Direct.blinkCounter800916E4 = blink.counter800916E4;
    s_ss0Direct.stageSelectBlink = blink.contextBlinkOnOff;
}

static const char* SS0DirectEvent2FrameClassName80026B94(
    PrSS0EventFrameLoopDirect::Event2FrameClass80026B94 frameClass) {
    using FrameClass = PrSS0EventFrameLoopDirect::Event2FrameClass80026B94;
    switch (frameClass) {
    case FrameClass::OpenInputLoop:
        return "open-input";
    case FrameClass::ResultAction:
        return "result-action";
    case FrameClass::ClosedInputTail:
        return "closed-input-tail";
    default:
        return "unknown";
    }
}

static bool SS0DirectBeginStageSelectFrameTransaction80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event2FrameClass80026B94 frameClass,
    bool inputClosed,
    int32_t tailFramesRemainingBefore,
    int32_t tailFramesRemainingAfter,
    const PrSS0EventFrameLoopDirect::DispatcherTailState80026B94& tailCandidate) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    s_ss0Direct.stageSelectFrameTailCandidate80026B94 = {};
    s_ss0Direct.stageSelectFrameTailCandidateKnown80026B94 = false;
    EventFrame::Event2FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = EventFrame::kCtxEvent2_80087B78;
    begin.logicFrame = ctx.frame;
    begin.frameClass = frameClass;
    begin.word800916F6Known = s_word800916F6.known;
    begin.word800916F6 = s_word800916F6.value;
    begin.inputClosed = inputClosed;
    begin.tailFramesRemainingBefore = tailFramesRemainingBefore;
    begin.tailFramesRemainingAfter = tailFramesRemainingAfter;
    if (!EventFrame::BeginEvent2FrameTransaction80026B94(
            s_ss0Direct.stageSelectFrameTransaction80026B94, begin)) {
        Log::Printf(
            "SS0 direct runtime: event2 frame transaction begin rejected frame=%u class=%s inputClosed=%d tail=%d/%d",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent2FrameClassName80026B94(frameClass),
            inputClosed ? 1 : 0,
            tailFramesRemainingBefore,
            tailFramesRemainingAfter);
        EventFrame::ResetEvent2FrameTransaction80026B94(
            s_ss0Direct.stageSelectFrameTransaction80026B94);
        return false;
    }

    // 80026170 is the direct Event2 tick owner. It updates only the shared
    // blink carrier here; the formal host boundary follows the page draw.
    SS0DirectRunEvent2Tick80026170();
    if (!EventFrame::CommitEvent2FrameTick80026170(
            s_ss0Direct.stageSelectFrameTransaction80026B94,
            ctx.frame,
            EventFrame::kEventTable2_8005453C,
            EventFrame::kCtxEvent2_80087B78)) {
        Log::Printf(
            "SS0 direct runtime: event2 frame transaction tick rejected frame=%u class=%s",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent2FrameClassName80026B94(frameClass));
        EventFrame::ResetEvent2FrameTransaction80026B94(
            s_ss0Direct.stageSelectFrameTransaction80026B94);
        return false;
    }

    // Formal 8001E750 selects 8004019C, stores the same slot in gp+0x368,
    // and clears that slot's work-list before the Event2 page draw. Keep this
    // owner binding independent from the legacy Windows S0 shell.
    PrPsxEventFrameDirect::BeginDrawWrapper8001E750(
        s_ss0Direct.stageSelectFrameHostState8001E750, 2);
    s_ss0Direct.stageSelectFrameEndFrameWorkListSlot8001EA00 =
        s_ss0Direct.stageSelectFrameHostState8001E750.gp368WorkSlot & 1u;
    s_ss0Direct.stageSelectFrameEndFrameBindingKnown8001EA00 = true;
    s_ss0Direct.stageSelectFrameEndFrameBindingBlockedLogged8001EA00 = false;

    s_ss0Direct.stageSelectFrameTailCandidate80026B94 = tailCandidate;
    s_ss0Direct.stageSelectFrameTailCandidateKnown80026B94 = true;
    return true;
}

static void SS0DirectCommitStageSelectFrameTail80026B94() {
    if (!s_ss0Direct.stageSelectFrameTailCandidateKnown80026B94) {
        return;
    }
    s_ss0Direct.stageSelectDispatcherTail80026B94 =
        s_ss0Direct.stageSelectFrameTailCandidate80026B94;
    s_ss0Direct.stageSelectFrameTailCandidate80026B94 = {};
    s_ss0Direct.stageSelectFrameTailCandidateKnown80026B94 = false;
}

static int SS0DirectTickStageSelect(PrGameContext& ctx) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    auto& frameTransaction = s_ss0Direct.stageSelectFrameTransaction80026B94;
    if (!EventFrame::CanAdvanceEvent2FrameLogic80026B94(frameTransaction)) {
        if (!frameTransaction.logicAdvanceBlockedLogged) {
            frameTransaction.logicAdvanceBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: event2 pending frame blocks logicFrame=%u hostFrame=%u tickCommitted=%d drawSubmitted=%d",
                static_cast<unsigned>(frameTransaction.logicFrame),
                static_cast<unsigned>(ctx.frame),
                frameTransaction.tickCommitted ? 1 : 0,
                frameTransaction.directDrawSubmitted ? 1 : 0);
        }
        return 0;
    }

    auto tailCandidate = s_ss0Direct.stageSelectDispatcherTail80026B94;
    EventFrame::DispatcherTailStep80026B94 tailStep{};
    if (!EventFrame::TryStepEvent2DispatcherTail80026B94(
            frameTransaction, tailCandidate, tailStep)) {
        return 0;
    }
    if (tailStep.kind ==
        EventFrame::DispatcherTailStepKind80026B94::ReleaseResult) {
        s_ss0Direct.stageSelectDispatcherTail80026B94 = tailCandidate;
        EventFrame::ResetEvent2FrameTransaction80026B94(frameTransaction);
        s_ss0Direct.stageSelectFrameTailCandidate80026B94 = {};
        s_ss0Direct.stageSelectFrameTailCandidateKnown80026B94 = false;
        Log::Printf(
            "SS0 direct runtime: event2 80026B94 tail released result=%d outScene=%d frame=%u",
            tailStep.releasedResult,
            s_ss0Direct.stageSelectState.outScene,
            static_cast<unsigned>(ctx.frame));
        SS0DirectConsumeStageSelectResult(ctx, tailStep.releasedResult);
        return 0;
    }

    PrSfx::PlayBgm();
    if (tailStep.kind ==
        EventFrame::DispatcherTailStepKind80026B94::HoldFrame) {
        (void)SS0DirectBeginStageSelectFrameTransaction80026B94(
            ctx,
            EventFrame::Event2FrameClass80026B94::ClosedInputTail,
            true,
            tailStep.framesRemainingBefore,
            tailStep.framesRemainingAfter,
            tailCandidate);
        return 0;
    }

    const PrPadState pad = SS0DirectReadPad(ctx);
    const uint32_t currentPad80026744 =
        SS0DirectFullLocalPadToPsxMask80026744(
            static_cast<uint16_t>(pad.held | pad.pressed));
    const auto preLoopRelease =
        EventFrame::StepDispatcherPreLoopPadRelease80026B94(
            s_ss0Direct.stageSelectPreLoopPadRelease80026B94,
            currentPad80026744);
    if (preLoopRelease.kind ==
        EventFrame::DispatcherPreLoopPadReleaseStepKind80026B94::
            WaitingForRelease) {
        EventFrame::ResetEvent2FrameTransaction80026B94(frameTransaction);
        s_ss0Direct.stageSelectFrameTailCandidate80026B94 = {};
        s_ss0Direct.stageSelectFrameTailCandidateKnown80026B94 = false;
        if (!s_ss0Direct.stageSelectPreLoopPadReleaseBlockedLogged) {
            s_ss0Direct.stageSelectPreLoopPadReleaseBlockedLogged = true;
            Log::Printf(
                "SS0 direct runtime: event2 pre-loop 80035510 waiting pad=%04X frame=%u",
                static_cast<unsigned>(currentPad80026744),
                static_cast<unsigned>(ctx.frame));
        }
        return 0;
    }
    if (preLoopRelease.kind ==
        EventFrame::DispatcherPreLoopPadReleaseStepKind80026B94::
            ReleasedEnterLoop) {
        EventFrame::ResetDispatcherPadChange80026744(
            s_ss0Direct.stageSelectPadChange80026744);
        s_ss0Direct.stageSelectPreLoopPadReleaseBlockedLogged = false;
        Log::Printf(
            "SS0 direct runtime: event2 pre-loop 80035510 released frame=%u",
            static_cast<unsigned>(ctx.frame));
    }
    const auto padChange = EventFrame::StepDispatcherPadChange80026744(
        s_ss0Direct.stageSelectPadChange80026744,
        currentPad80026744);
    const uint32_t psxPad = padChange.changedPadMask80026744;
    if (psxPad != 0u) {
        const int previousCursor = s_ss0Direct.stageSelectState.cursor;
        const auto handled =
            PrSS0DirectoryDispatcherDirect::HandleStageSelect80025F6C(
                s_ss0Direct.stageSelectState,
                psxPad);
        if (handled.cueOrder == PrSS0DirectoryDispatcherDirect::
                                        StageSelectCueOrder80025F6C::
                                            BeforeStateCommit &&
            handled.cueCode80025C8C != 0u) {
            SS0DirectPlayStageSelectInputCue80025F6C(
                handled.cueCode80025C8C);
        }

        s_ss0Direct.stageSelectState = handled.state;
        SS0DirectPublishStageSelectState80025F6C(ctx);
        if (handled.cueOrder == PrSS0DirectoryDispatcherDirect::
                                        StageSelectCueOrder80025F6C::
                                            AfterStateCommit &&
            handled.cueCode80025C8C != 0u) {
            SS0DirectPlayStageSelectInputCue80025F6C(
                handled.cueCode80025C8C);
        }

        if (handled.result != 0) {
            if (!EventFrame::ArmDispatcherTail80026B94(
                    tailCandidate,
                    handled.result)) {
                Log::Printf(
                    "SS0 direct runtime: event2 80026B94 tail arm rejected result=%d",
                    handled.result);
            } else {
                Log::Printf(
                    "SS0 direct runtime: event2 80026B94 result=%d outScene=%d tail armed frames=%d frame=%u",
                    handled.result,
                    s_ss0Direct.stageSelectState.outScene,
                    PrSS0EventFrameLoopDirect::
                        kDispatcherResultTailFrames80026B94,
                    static_cast<unsigned>(ctx.frame));
            }
        } else if (s_ss0Direct.stageSelectState.cursor != previousCursor) {
            Log::Printf("SS0 direct runtime: stage select cursor=%d",
                        s_ss0Direct.stageSelectState.cursor);
        }
    }

    const bool resultActionFrame =
        tailCandidate.active &&
        tailCandidate.framesRemaining ==
            EventFrame::kDispatcherResultTailFrames80026B94;
    const EventFrame::Event2FrameClass80026B94 frameClass =
        resultActionFrame
            ? EventFrame::Event2FrameClass80026B94::ResultAction
            : EventFrame::Event2FrameClass80026B94::OpenInputLoop;
    const int32_t tailFrameMarker = resultActionFrame
        ? EventFrame::kDispatcherResultTailFrames80026B94
        : -1;
    (void)SS0DirectBeginStageSelectFrameTransaction80026B94(
        ctx,
        frameClass,
        false,
        tailFrameMarker,
        tailFrameMarker,
        tailCandidate);
    return 0;
}

static bool SS0DirectCommitPendingTitleEvent801C5190()
{
    if (!s_ss0Direct.titleEventPending) {
        return true;
    }

    const auto runtimeCommit =
        PrSS0TitleHudEventsDirect::BuildTitleEventRuntimeCommit801C5190(
            s_ss0Direct.titleEventRuntimeState,
            s_ss0Direct.titlePendingEvent.effect);
    if (!runtimeCommit.accepted) {
        return false;
    }
    if (!PrSS0TitleTmdBackend::CommitRuntimeTitleEventResources801C6410(
            s_ss0Direct.titlePendingEvent.effect)) {
        return false;
    }

    s_ss0Direct.titleEventRuntimeState = runtimeCommit.nextState;
    s_ss0Direct.titleAbsoluteEventStream =
        s_ss0Direct.titlePendingEvent.nextState;
    s_ss0Direct.titleEventPending = false;
    s_ss0Direct.titlePendingEvent = {};
    return true;
}

static bool SS0DirectAdvanceTitleHudTimeline801C5094(
    PrGameContext& ctx,
    const PrSS0TitleHudEventsDirect::TitleEventClock801C4894& clock)
{
    if (!s_ss0Direct.titleEventRuntimeState.hudSlot0Known) {
        return true;
    }
    if (!clock.known) {
        return false;
    }

    PrSS0TitleHudEventsDirect::TitleHudTimelineTickInput801C5094 input{};
    input.channelKnown = true;
    input.channel =
        PrSS0TitleHudEventsDirect::kCtxRuntimeTimListObservedChannel;
    input.currentTick96Known = true;
    input.currentTick96 = clock.tick96;
    const auto tick =
        PrSS0TitleHudEventsDirect::TickTitleHudTimeline801C5094(
            s_ss0Direct.titleEventRuntimeState, input);
    if (tick.status == PrSS0TitleHudEventsDirect::
                           TitleHudTimelineTickStatus801C5094::NotDue) {
        return true;
    }
    if (tick.status == PrSS0TitleHudEventsDirect::
                           TitleHudTimelineTickStatus801C5094::Complete) {
        s_ss0Direct.titleEventRuntimeState = tick.nextState;
        return true;
    }
    if (tick.status != PrSS0TitleHudEventsDirect::
                           TitleHudTimelineTickStatus801C5094::Applied) {
        return false;
    }

    const auto applied =
        PrSS0TitleTmdBackend::ApplyTitleHudTimRequest801C5094(
            ctx.resources, tick.request);
    if (!applied.accepted) {
        return false;
    }
    s_ss0Direct.titleEventRuntimeState = tick.nextState;
    return true;
}

static PrSS0TitleHudEventsDirect::TitleSelectorHudTickInput801C5854
SS0DirectBuildTitleSelectorHudTickInput801C5854(
    bool currentTick96Known,
    uint32_t currentTick96,
    bool beatKnown,
    uint8_t beat,
    uint8_t cursor)
{
    PrSS0TitleHudEventsDirect::TitleSelectorHudTickInput801C5854 input{};
    input.currentTick96Known = currentTick96Known;
    input.currentTick96 = currentTick96;
    input.beatKnown = beatKnown;
    input.beat = beat;
    input.cursorKnown = true;
    input.cursor = cursor;
    return input;
}

static auto SS0DirectPreflightTitleSelectorPrompt8001E3E4(
    PrGameContext& ctx,
    uint8_t cursor) -> bool;

static bool SS0DirectPrepareTitleSelectorHudTick801C5854(
    PrGameContext& ctx,
    const PrSS0TitleHudEventsDirect::TitleSelectorHudTickResult801C5854&
        tick,
    PrSS0TitleTmdBackend::PreparedRuntimeChannel2MimeBinding801C6410&
        preparedMime,
    uint8_t promptCursor)
{
    if (tick.status != PrSS0TitleHudEventsDirect::
                           TitleSelectorHudTickStatus801C5854::Applied) {
        return false;
    }
    if (tick.selectorMimeEffect.known !=
        tick.selectorResourceUpdate.known) {
        return false;
    }
    if (tick.selectorResourceUpdate.known) {
        const auto selector =
            PrSS0TitleHudEventsDirect::KnownSelectorRecordAt(
                tick.selectorIndex);
        if (tick.selectorResourceUpdate.selectorIndex !=
                tick.selectorIndex ||
            tick.selectorResourceUpdate.idA != selector.idA ||
            tick.selectorResourceUpdate.idB != selector.idB ||
            tick.selectorMimeEffect.eventIndex != tick.selectorIndex ||
            tick.selectorMimeEffect.channel2PairIndex !=
                PrSS0TitleHudEventsDirect::
                    KnownSelectorResourcePairIndexAt(tick.selectorIndex) ||
            tick.selectorResourceUpdate.targetOffsetA !=
                PrSS0TitleHudEventsDirect::kCtxSelectorResourceOffsetDC ||
            tick.selectorResourceUpdate.targetOffsetB !=
                PrSS0TitleHudEventsDirect::kCtxSelectorResourceOffsetE8 ||
            !tick.selectorResourceUpdate.appliedTick96Known ||
            tick.selectorResourceUpdate.appliedTick96 !=
                tick.selectorMimeEffect.appliedTick96) {
            return false;
        }
    }
    if (tick.selectorMimeEffect.known) {
        preparedMime = PrSS0TitleTmdBackend::
            PrepareRuntimeChannel2MimeBinding801C6410(
                tick.selectorMimeEffect);
        if (!preparedMime.accepted) {
            return false;
        }
    }
    if (tick.request.valid) {
        const auto applied =
            PrSS0TitleTmdBackend::ApplyTitleHudTimRequest801C5094(
                ctx.resources, tick.request);
        if (!applied.accepted) {
            return false;
        }
    }
    if (!SS0DirectPreflightTitleSelectorPrompt8001E3E4(
            ctx, promptCursor)) {
        return false;
    }
    return true;
}

static void SS0DirectPublishPreparedTitleSelectorHudTick801C5854(
    const PrSS0TitleHudEventsDirect::TitleSelectorHudTickResult801C5854& tick,
    const PrSS0TitleTmdBackend::PreparedRuntimeChannel2MimeBinding801C6410&
        preparedMime)
{
    if (tick.selectorMimeEffect.known) {
        PrSS0TitleTmdBackend::
            CommitPreparedRuntimeChannel2MimeBinding801C6410(preparedMime);
    }
    if (tick.selectorResourceUpdate.known) {
        s_ss0Direct.titleSelectorResourceUpdate =
            tick.selectorResourceUpdate;
    }
    s_ss0Direct.titleEventRuntimeState = tick.nextEventState;
    s_ss0Direct.titleSelectorHudRuntimeState = tick.nextSelectorState;
}

static bool SS0DirectExecutePreparedTitleVisualState801C5EF0()
{
    const bool earlyInputShortcut =
        PrSS0TitleTmdBackend::IsTitleEarlyInputShortcutReady801C6410();
    const auto state = PrSS0TitleTmdBackend::GetResourceState();
    const uint32_t shortcutLoopCount =
        state.titleEarlyInputShortcutLoopCount801CC676;
    bool complete = true;

    const auto camera =
        PrSS0TitleTmdBackend::AdvanceRuntimeTitleCamera80041D3C();
    complete = complete && camera.accepted;
    if (earlyInputShortcut && camera.sampledIndex != 299u) {
        complete = false;
    }
    const auto primaryPa =
        PrSS0TitleTmdBackend::AdvanceRuntimePrimaryPaMime800141D8();
    complete = complete && primaryPa.accepted;
    const auto tod = PrSS0TitleTmdBackend::AdvanceRuntimeTitleTod8001B000();
    complete = complete && tod.accepted;
    if (!PrSS0TitleTmdBackend::ComposeRuntimePaMatrix8001B084() ||
        !PrSS0TitleTmdBackend::ExecutePaMode25PrimitiveGroup8004274C()) {
        return false;
    }
    const auto channel2 =
        PrSS0TitleTmdBackend::AdvanceRuntimeChannel2Mime80013EA8();
    complete = complete && channel2.accepted;
    complete =
        PrSS0TitleTmdBackend::ComposeRuntimeTitleMatrix801C5E60() &&
        complete;
    complete =
        PrSS0TitleTmdBackend::ExecutePaKageMode25PrimitiveGroup8004274C() &&
        complete;
    if (!PrSS0TitleTmdBackend::CanExecuteLoHpPrimitiveGroups801C5EF0()) {
        return false;
    }
    const auto loMime =
        PrSS0TitleTmdBackend::AdvanceRuntimeChannel1Mime80013EA8();
    const bool loCursorAccepted = earlyInputShortcut
        ? loMime.cursorAfter >= shortcutLoopCount + 1u &&
              loMime.sampleFrame + 1u == loMime.cursorAfter
        // Natural title playback is also a monotonic LogoNew stream.  The
        // decoder clamps an exhausted stream to its last frame, so accept a
        // clamped sample whose cursor has advanced beyond that frame.
        : loMime.cursorAfter > 0u &&
              static_cast<uint32_t>(loMime.sampleFrame) + 1u <=
                  loMime.cursorAfter;
    if (!loMime.accepted || !loCursorAccepted ||
        !PrSS0TitleTmdBackend::ComposeRuntimeLoMatrix8001B084() ||
        !PrSS0TitleTmdBackend::ExecuteLoMode25PrimitiveGroup8004274C()) {
        return false;
    }
    if (!PrSS0TitleTmdBackend::CanExecuteHpPrimitiveGroup801C5EF0()) {
        return false;
    }
    const auto hpMime =
        PrSS0TitleTmdBackend::AdvanceRuntimeChannel3Mime80013EA8();
    const bool hpCursorAccepted = !earlyInputShortcut ||
        (hpMime.rawCursor >= shortcutLoopCount &&
         hpMime.sampleFrame == static_cast<uint16_t>(hpMime.rawCursor));
    if (!hpMime.accepted || !hpCursorAccepted ||
        !PrSS0TitleTmdBackend::ComposeRuntimeHpMatrix8001B084()) {
        return false;
    }
    if (!PrSS0TitleTmdBackend::ExecuteHpMode25PrimitiveGroup8004274C() ||
        !complete) {
        return false;
    }
    return true;
}

static bool SS0DirectTryAdvanceTitleVisualState801C5EF0()
{
    if (!PrSS0TitleTmdBackend::BeginTitlePacketFrame801C6410()) {
        return false;
    }
    if (!PrSS0TitleTmdBackend::ApplyTitleFrameFlags801C6410(
            s_ss0Direct.titleEventRuntimeState.ctxFlags)) {
        return false;
    }
    const auto resourceState = PrSS0TitleTmdBackend::GetResourceState();
    if (!resourceState.titleRenderActiveKnown801C9548) {
        return false;
    }
    if (resourceState.titleRenderActive801C9548 == 0u) {
        return true;
    }
    return SS0DirectExecutePreparedTitleVisualState801C5EF0();
}

static void SS0DirectAdvanceTitleVisualState801C5EF0()
{
    const auto resourceState = PrSS0TitleTmdBackend::GetResourceState();
    if ((s_ss0Direct.titleEventRuntimeState.ctxFlags & 0x00000020u) == 0u &&
        resourceState.titleRenderActiveKnown801C9548 &&
        resourceState.titleRenderActive801C9548 == 0u) {
        return;
    }
    (void)SS0DirectTryAdvanceTitleVisualState801C5EF0();
}

static void SS0DirectBlockTitleEarlyInputShortcut801C4894()
{
    s_ss0Direct.titleEarlyInputShortcutPhase =
        SS0DirectTitleEarlyInputShortcutPhase::Blocked;
    s_ss0Direct.titleEarlyInputShortcutCompletedWaitCalls = 0u;
    s_ss0Direct.titleEarlyInputShortcutFrameWaitRemaining = 0u;
    s_ss0Direct.titleEarlyInputPixelSubmitComplete = false;
    s_ss0Direct.titleEarlyInputPixelSubmitStatusLogged = false;
    s_ss0Direct.titleNaturalLoopCadence = {};
}

static int SS0DirectTickTitleEarlyInputShortcut801C4DC4(
    PrGameContext& ctx)
{
    const PrPadState pad = SS0DirectReadPad(ctx);
    s_ss0Direct.titleSelectorPreviousInputPsx = static_cast<uint16_t>(
        SS0DirectLocalPressedToPsxPadMask(pad.held));
    ctx.scn0HiliteCursor = s_ss0Direct.titleCursor;
    if (s_ss0Direct.titleEarlyInputShortcutPhase ==
        SS0DirectTitleEarlyInputShortcutPhase::Blocked) {
        return 0;
    }
    if (s_ss0Direct.titleEarlyInputShortcutPhase ==
        SS0DirectTitleEarlyInputShortcutPhase::AwaitingPixelSubmit) {
        if (!s_ss0Direct.titleEarlyInputPixelSubmitComplete) {
            return 0;
        }

        // COMOD0 801C4CB0..801C4C58 waits in 80035510 until the button
        // which skipped MOVIE0T has been released.  The old host path used
        // the held mask only as the next edge baseline, which let the
        // selector/BGM boundary run while the pad was still down.  Keep the
        // translated shortcut in an explicit release phase so no selector
        // input or BGM allocation can be observed before that PSX barrier.
        s_ss0Direct.titleEarlyInputShortcutPhase =
            SS0DirectTitleEarlyInputShortcutPhase::WaitingPadRelease;
        Log::Printf(
            "SS0 direct runtime: title early input pixels complete -> 80035510 release barrier");
        return 0;
    }

    if (s_ss0Direct.titleEarlyInputShortcutPhase ==
        SS0DirectTitleEarlyInputShortcutPhase::WaitingPadRelease) {
        const PrPadState releasePad = SS0DirectReadPad(ctx);
        const uint32_t releaseMask =
            SS0DirectFullLocalPadToPsxMask80026744(
                static_cast<uint16_t>(releasePad.held | releasePad.pressed));
        if (releaseMask != 0u) {
            return 0;
        }

        const auto state = PrSS0TitleTmdBackend::GetResourceState();
        s_ss0Direct.titleLoopTickV10 = kAutoMenuFrame + 1;
        s_ss0Direct.titleLoopStateV8 = 2u;
        PrSS0TitleHudEventsDirect::ResetTitleAbsoluteEventStreamState80024E98(
            s_ss0Direct.titleAbsoluteEventStream);
        // The PSX 801C4894 shortcut emits one final 801C6410 frame with the
        // 0x4000000/0x8000000 setup flags, then 80024FD0 clears ctx->flags
        // before the ordinary selector loop resumes.  The host translation
        // parks this branch while waiting for the D3D pixel-submit latch, so
        // perform that same boundary clear here; otherwise the first normal
        // selector frame replays shortcut flags through 801C6410 (which
        // correctly rejects them as a one-time setup) and the title model
        // disappears while TitleExitWait can never prepare its next frame.
        s_ss0Direct.titleEventRuntimeState.ctxFlags = 0u;
        s_ss0Direct.titleEventPending = false;
        s_ss0Direct.titlePendingEvent = {};
        s_ss0Direct.titleEarlyInputShortcutPhase =
            SS0DirectTitleEarlyInputShortcutPhase::None;
        s_ss0Direct.titleEarlyInputShortcutCompletedWaitCalls = 0u;
        s_ss0Direct.titleEarlyInputShortcutFrameWaitRemaining = 0u;
        s_ss0Direct.titleEarlyInputPixelSubmitComplete = false;
        s_ss0Direct.titleEarlyInputPixelSubmitStatusLogged = false;
        (void)SS0DirectRestartTitleAttractTimer801C4894();
        // COMOD0 801C4894 label_28 reaches the selector state through the
        // same audio tail as the natural v10>=370 path:
        // 80026EF8(dword_80094410) -> 80026ECC.  An active input skip enters
        // this shortcut directly and therefore must bind the looping tone-7
        // BGM here; otherwise the selector appears with silent background
        // audio even though the normal timeout path loops it correctly.
        PrSfx::PlayBgm();
        PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
        Log::Printf(
            "SS0 direct runtime: title early input pixels -> selector loop=%u v10=%d 94410 BGM loop/26ECC",
            static_cast<unsigned>(
                state.titleEarlyInputShortcutLoopCount801CC676),
            s_ss0Direct.titleLoopTickV10);
        return 0;
    }

    const auto hold =
        PrSS0TitleHudEventsDirect::TickTitleEarlyInputShortcutHold801C4894(
            s_ss0Direct.titleEarlyInputShortcutCompletedWaitCalls,
            s_ss0Direct.titleEarlyInputShortcutFrameWaitRemaining);
    if (!hold.accepted) {
        SS0DirectBlockTitleEarlyInputShortcut801C4894();
        Log::Printf(
            "SS0 direct runtime: title early input hold cadence blocked calls=%u frameWait=%u",
            s_ss0Direct.titleEarlyInputShortcutCompletedWaitCalls,
            s_ss0Direct.titleEarlyInputShortcutFrameWaitRemaining);
        return 0;
    }
    s_ss0Direct.titleEarlyInputShortcutCompletedWaitCalls =
        static_cast<uint8_t>(hold.nextCompletedWaitCalls);
    s_ss0Direct.titleEarlyInputShortcutFrameWaitRemaining =
        static_cast<uint8_t>(hold.nextFrameWaitRemaining);
    if (!hold.ready) {
        return 0;
    }

    const auto shortcutTransaction = PrSS0TitleHudEventsDirect::
        BuildShortcutTitleSelectorHudStartAndFirstTick801C5AB4();
    const auto& selectorStart = shortcutTransaction.firstTick;
    PrSS0Scene0SharedEventPredispatchDirect::StaticDispatchState80024FD0
        resetDispatch{};
    if (!shortcutTransaction.reset80024E98Applied ||
        !shortcutTransaction.callerReadyFlagApplied ||
        shortcutTransaction.firstTickCtxTick96 != 0u ||
        shortcutTransaction.firstTickBeatDerived != 0u ||
        shortcutTransaction.firstTickCursor != 0u ||
        selectorStart.status != PrSS0TitleHudEventsDirect::
                                    TitleSelectorHudTickStatus801C5854::
                                        Applied ||
        selectorStart.nextEventState.ctxFlags !=
            PrSS0TitleHudEventsDirect::
                kTitleShortcutFirstTickCtxFlags801C5AB4 ||
        !SS0DirectPrepareTitleEventProjectionReset80024E98(
            resetDispatch)) {
        SS0DirectBlockTitleEarlyInputShortcut801C4894();
        Log::Printf(
            "SS0 direct runtime: title early input 801C5AB4 reset/HUD source blocked status=%u reset=%u readyFlag=%u flags=%08X",
            static_cast<unsigned>(selectorStart.status),
            shortcutTransaction.reset80024E98Applied ? 1u : 0u,
            shortcutTransaction.callerReadyFlagApplied ? 1u : 0u,
            selectorStart.nextEventState.ctxFlags);
        return 0;
    }
    PrSS0TitleTmdBackend::PreparedRuntimeChannel2MimeBinding801C6410
        preparedSelectorMime{};
    if (!SS0DirectPrepareTitleSelectorHudTick801C5854(
            ctx,
            selectorStart,
            preparedSelectorMime,
            shortcutTransaction.firstTickCursor)) {
        SS0DirectBlockTitleEarlyInputShortcut801C4894();
        Log::Printf(
            "SS0 direct runtime: title early input 801C5AB4 HUD prepare blocked status=%u",
            static_cast<unsigned>(selectorStart.status));
        return 0;
    }
    const auto ready =
        PrSS0TitleTmdBackend::ApplyTitleEarlyInputShortcutReadyFrame801C6410();
    if (!ready.committed) {
        SS0DirectBlockTitleEarlyInputShortcut801C4894();
        Log::Printf(
            "SS0 direct runtime: title early input ready blocked status=%u",
            static_cast<unsigned>(ready.status));
        return 0;
    }
    SS0DirectCommitTitleEventProjectionReset80024E98(resetDispatch);
    s_ss0Direct.titleCtxTick96Known = true;
    s_ss0Direct.titleCtxTick96 = shortcutTransaction.firstTickCtxTick96;
    s_ss0Direct.titleCtxBeatKnown = true;
    s_ss0Direct.titleCtxBeat = shortcutTransaction.firstTickBeatDerived;
    SS0DirectPublishPreparedTitleSelectorHudTick801C5854(
        selectorStart, preparedSelectorMime);
    if (!SS0DirectExecutePreparedTitleVisualState801C5EF0()) {
        SS0DirectBlockTitleEarlyInputShortcut801C4894();
        Log::Printf(
            "SS0 direct runtime: title early input visual commit blocked");
        return 0;
    }

    s_ss0Direct.titleEarlyInputShortcutPhase =
        SS0DirectTitleEarlyInputShortcutPhase::AwaitingPixelSubmit;
    s_ss0Direct.titleEarlyInputPixelSubmitComplete = false;
    s_ss0Direct.titleEarlyInputPixelSubmitStatusLogged = false;
    Log::Printf(
        "SS0 direct runtime: title early input 801C5AB4 reset/tick0 ready packet awaiting pixels loop=%u flags=%08X",
        static_cast<unsigned>(ready.channel1LoopCount801CC676),
        selectorStart.nextEventState.ctxFlags);
    return 0;
}

static void SS0DirectQueueTitleAbsoluteEvent801C5538(
    const PrSS0TitleHudEventsDirect::TitleEventClock801C4894& clock)
{
    if (s_ss0Direct.titleEventPending || !clock.known) {
        return;
    }

    PrSS0TitleHudEventsDirect::TitleAbsoluteEventTickInput801C5538 input{};
    input.ctxTick96Known = true;
    input.ctxTick96 = clock.tick96;
    input.streamFlagKnown =
        s_ss0Direct.titleAbsoluteEventStream.streamFlagKnown;
    input.streamFlag = s_ss0Direct.titleAbsoluteEventStream.streamFlag;
    input.streamIdKnown =
        s_ss0Direct.titleAbsoluteEventStream.streamIdKnown;
    input.streamId = s_ss0Direct.titleAbsoluteEventStream.streamId;
    const auto tick =
        PrSS0TitleHudEventsDirect::TickTitleAbsoluteEventStream801C5538(
            s_ss0Direct.titleAbsoluteEventStream, input);
    if (tick.status == PrSS0TitleHudEventsDirect::
                           TitleAbsoluteEventTickStatus801C5538::Applied) {
        s_ss0Direct.titlePendingEvent = tick;
        s_ss0Direct.titleEventPending = true;
    }
}

static bool SS0DirectTickTitleSharedEventBucketPrefix80024FD0(
    PrGameContext& ctx,
    uint32_t callsite,
    const PrSS0TitleHudEventsDirect::TitleEventClock801C4894& clock) {
    PrSS0TitleHudEventsDirect::TitleSharedEventBucketInput80024FD0 input{};
    input.callsiteKnown = true;
    input.callsite = callsite;
    input.ctxTick96Known = clock.known;
    input.ctxTick96 = clock.tick96;
    const auto tick =
        PrSS0TitleHudEventsDirect::TickTitleSharedEventBucketPrefix80024FD0(
            s_ss0Direct.titleSharedEventBucket, input);
    if (!tick.accepted || (!tick.prefixApplied && !tick.earlyReturnEd20)) {
        Log::Printf(
            "SS0 direct runtime: title 80024FD0 bucket prefix blocked callsite=%08X tickKnown=%u status=%u",
            callsite,
            clock.known ? 1u : 0u,
            static_cast<unsigned>(tick.status));
        return false;
    }

    if (tick.sharedTailRequired) {
        PrSS0Scene0SharedEventPredispatchDirect::Input80024FD0
            predispatchInput{};
        predispatchInput.callsiteKnown = true;
        predispatchInput.callsite = callsite;
        predispatchInput.ctxTick96Known = clock.known;
        predispatchInput.ctxTick96 = clock.tick96;
        predispatchInput.word800916D0Known = true;
        predispatchInput.word800916D0 =
            static_cast<uint16_t>(ctx.transitionState);
        predispatchInput.prefixAppliedKnown = true;
        predispatchInput.prefixApplied = tick.prefixApplied;
        predispatchInput.bucketChangedKnown = true;
        predispatchInput.bucketChanged = tick.bucketChanged;
        predispatchInput.bucketKnown = true;
        predispatchInput.bucket = tick.nextState.dword8008ECF4;
        predispatchInput.ctxFlagsKnown = true;
        predispatchInput.ctxFlags = 0u;
        predispatchInput.globalBindings =
            s_ss0Direct.scene0GlobalBindings;
        const auto predispatch =
            PrSS0Scene0SharedEventPredispatchDirect::
                ApplyPredispatch800250E4To800251D4(
                    s_ss0Direct.titleSharedEventPredispatchContext,
                    predispatchInput);
        if (!predispatch.accepted || !predispatch.applied ||
            !predispatch.stoppedBeforeDispatch) {
            Log::Printf(
                "SS0 direct runtime: title 80024FD0 predispatch blocked callsite=%08X tick=%u bucket=%u D0=%d status=%u",
                callsite,
                static_cast<unsigned>(clock.tick96),
                static_cast<unsigned>(tick.nextState.dword8008ECF4),
                static_cast<int>(ctx.transitionState),
                static_cast<unsigned>(predispatch.status));
            return false;
        }

        PrSS0Scene0SharedEventPredispatchDirect::StaticDispatchInput80024FD0
            dispatchInput{};
        dispatchInput.callsiteKnown = true;
        dispatchInput.callsite = callsite;
        dispatchInput.ctxTick96Known = clock.known;
        dispatchInput.ctxTick96 = clock.tick96;
        dispatchInput.bucketKnown = true;
        dispatchInput.bucket = tick.nextState.dword8008ECF4;
        dispatchInput.predispatchAppliedKnown = true;
        dispatchInput.predispatchApplied = predispatch.applied;
        dispatchInput.predispatchStoppedBeforeDispatchKnown = true;
        dispatchInput.predispatchStoppedBeforeDispatch =
            predispatch.stoppedBeforeDispatch;
        dispatchInput.noRowDereferenceKnown = true;
        dispatchInput.noRowDereference =
            !predispatch.tableRowDereferenceRequired &&
            !predispatch.tableRowDereferenceAttempted;
        dispatchInput.ctxDword56Known =
            predispatch.nextContext.ctxDword56Known;
        dispatchInput.ctxDword56 = predispatch.nextContext.ctxDword56;
        dispatchInput.globalBindings = s_ss0Direct.scene0GlobalBindings;
        const auto dispatch =
            PrSS0Scene0SharedEventPredispatchDirect::
                ApplyStaticScene0Dispatch800251D8To800259B8(
                    s_ss0Direct.titleSharedEventDispatch,
                    dispatchInput);
        if (!dispatch.accepted || !dispatch.applied ||
            !dispatch.completedThrough800259B8) {
            Log::Printf(
                "SS0 direct runtime: title 80024FD0 dispatch blocked callsite=%08X tick=%u bucket=%u status=%u",
                callsite,
                static_cast<unsigned>(clock.tick96),
                static_cast<unsigned>(tick.nextState.dword8008ECF4),
                static_cast<unsigned>(dispatch.status));
            return false;
        }

        s_ss0Direct.titleSharedEventPredispatchContext =
            predispatch.nextContext;
        s_ss0Direct.titleSharedEventDispatch = dispatch.nextState;
        s_ss0Direct.titleEventRuntimeState.ctxFlags =
            predispatch.nextCtxFlags;
        if (!s_ss0Direct.titleSharedEventPredispatchLogged) {
            s_ss0Direct.titleSharedEventPredispatchLogged = true;
            Log::Printf(
                "SS0 direct runtime: title 80024FD0 predispatch committed callsite=%08X bucket=%u ctx56=%u flags=%08X stop=800251D4 next=800251D8",
                callsite,
                static_cast<unsigned>(tick.nextState.dword8008ECF4),
                static_cast<unsigned>(predispatch.ctxDword56V6),
                predispatch.nextCtxFlags);
        }
        if (!s_ss0Direct.titleSharedEventDispatchLogged) {
            s_ss0Direct.titleSharedEventDispatchLogged = true;
            Log::Printf(
                "SS0 direct runtime: title 80024FD0 static dispatch committed callsite=%08X bucket=%u key=%u end=800259B8 page=%08X noRow=1",
                callsite,
                static_cast<unsigned>(tick.nextState.dword8008ECF4),
                static_cast<unsigned>(dispatch.key),
                dispatch.ringPagePsxAddress);
        }
    }
    s_ss0Direct.titleSharedEventBucket = tick.nextState;
    return true;
}

static int SS0DirectFn2(PrGameContext& ctx) {
    if (!s_ss0Direct.initialized) {
        SS0DirectResetToTitle(ctx);
    }

    // Loading mode is written at its exact 80015408 caller, not polled from
    // a stale scene-exit carrier (and mode 1 is a valid Hold-on selection).

    // 80015408 registers 8001537C as a VSync callback.  The direct main loop
    // exposes one equivalent callback slot per logic tick; keep this outside
    // transition/page logic so all loading modes advance the same way and a
    // completed transition cannot tick the pattern twice.
    if (s_ss0Direct.loadingPatternRuntime8001EF40.active &&
        !SS0DirectTickLoadingPatternForHostLogic8001EF40()) {
        Log::Printf(
            "SS0 direct runtime: 8001537C->8001EF40 Loading callback blocked frame=%u",
            static_cast<unsigned>(ctx.frame));
    }

    // A title MOVIE0T stream may have crossed its visible-video boundary
    // while the original XA tail is still draining.  Reconcile that voice
    // before any page/scene transition can allocate or reset shared voices.
    SS0DirectReleaseTitleMovie0TAudioTailIfDrained();

    ctx.stageRunning = false;
    if (!s_residentDirectory &&
        (!PrSS0Scene0GlobalBindingsDirect::IsExactInitTransaction801C4260(
            s_ss0Direct.scene0Init801C4260) ||
        !PrSS0Scene0ResourceIngressDirect::
            IsExactAcceptedTransaction801C4780(
                s_ss0Direct.scene0ResourceIngress801C4780) ||
        !PrSS0Scene0IntLoadDirect::
            IsExactAcceptedStartupCommonTransaction80016B84(
                s_ss0Direct.startupCommonIntLoad80016B84) ||
        !PrSS0Scene0IntLoadDirect::
            IsExactAcceptedZCompoTransaction80015590(
                s_ss0Direct.startupZCompoIntLoad80016B84) ||
        !PrSS0Scene0IntLoadDirect::IsExactAcceptedTransaction8001AC18(
            s_ss0Direct.scene0IntLoad8001AC18) ||
        !s_ss0Direct.scene0IntSideEffectGateCommitted ||
        !s_ss0Direct.scene0IntSpuGateCommitted ||
        !s_ss0Direct.scene0IntGpuGateCommitted ||
        !s_ss0Direct.scene0IntRendererGateCommitted)) {
        if (!SS0DirectTryCommitScene0Init801C4260(ctx)) {
            return 0;
        }
        (void)SS0DirectStartScene0OuterEntry801C4DC4(ctx);
    }
    if (s_ss0Direct.phase ==
        SS0DirectPhase::OpeningMovie0InitialTransition) {
        return SS0DirectTickOpeningMovie0InitialTransition80020110(ctx);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleIntroTransition) {
        return SS0DirectTickTitleIntroTransition80020110(ctx);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleMovie0TFinalReady) {
        return SS0DirectTickTitleMovie0TFinalReady801C4968(ctx);
    }
    if (s_ss0Direct.phase ==
        SS0DirectPhase::OpeningMovie0PreTransition) {
        return SS0DirectTickOpeningMovie0Transition(ctx, true);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::OpeningMovie0) {
        return SS0DirectTickOpeningMovie0(ctx);
    }
    if (s_ss0Direct.phase ==
        SS0DirectPhase::OpeningMovie0PostTransition) {
        return SS0DirectTickOpeningMovie0Transition(ctx, false);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleExitTransition) {
        return SS0DirectTickTitleExitTransition80020110(ctx);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::MainMenuResourceLoading) {
        if (SS0DirectLoadingMinimumHoldReady(
                SS0DirectLoadingHoldKind::MainMenuReload, ctx.frame)) {
            (void)SS0DirectQueueMainMenuEntryTransition80026C90(ctx);
        }
        return 0;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::MainMenuEntryTransition) {
        return SS0DirectTickEvent3EntryTransition80020110(ctx);
    }
    if (s_ss0Direct.sceneHandoffRuntimeActive &&
        s_ss0Direct.phase != SS0DirectPhase::WaitTransition) {
        return 0;
    }

    if (s_ss0Direct.phase == SS0DirectPhase::WaitTransition) {
        // Keep the PSX mode-1 visual/timing runtime advancing while the main
        // scene owner holds the target-scene carrier.  The carrier and its
        // completion latch are SS0-owned; no legacy PrTransition state is
        // involved in this direct scene handoff.
        (void)SS0DirectTickSceneHandoffTransition80020110(ctx);
        if (s_ss0Direct.sceneHandoffTargetPending) {
            // PrMain consumes the target and switches scenes in this same
            // frame.  Keep the SS0 phase intact until that outer switch so
            // no title/menu fallback can paint a one-frame shell residue.
        } else if (!s_ss0Direct.sceneHandoffRuntimeActive) {
            if (s_ss0Direct.transitionReturnPhase == SS0DirectPhase::MainMenu) {
                (void)SS0DirectQueueMainMenuEntryTransition80026C90(ctx);
            } else {
                SS0DirectResetToTitle(ctx);
            }
        }
        return 0;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleExitWait) {
        return SS0DirectTickTitleExitWait(ctx);
    }

    if (s_ss0Direct.phase == SS0DirectPhase::MainMenu) {
        return SS0DirectTickMainMenu(ctx);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::Options) {
        return SS0DirectTickOptions(ctx);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::StageSelect) {
        return SS0DirectTickStageSelect(ctx);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::Practice) {
        return SS0DirectTickPractice(ctx);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::HiScore) {
        return SS0DirectTickHiScore(ctx);
    }
    if (s_ss0Direct.phase == SS0DirectPhase::ReplayCard ||
        s_ss0Direct.phase == SS0DirectPhase::LoadCard) {
        return SS0DirectTickCardPage(ctx);
    }
    if (s_ss0Direct.titleEarlyInputShortcutPhase !=
        SS0DirectTitleEarlyInputShortcutPhase::None) {
        return SS0DirectTickTitleEarlyInputShortcut801C4DC4(ctx);
    }

    const auto titleNaturalLoopCadence =
        PrSS0TitleHudEventsDirect::TickTitleNaturalLoopCadence801C4894(
            s_ss0Direct.titleNaturalLoopCadence,
            false);
    if (!titleNaturalLoopCadence.processIteration) {
        return 0;
    }

    if (!SS0DirectCommitPendingTitleEvent801C5190()) {
        s_ss0Direct.titleNaturalLoopCadence.holdTicksRemaining = 0u;
        return 0;
    }

    const int32_t currentTitleLoopTickV10 =
        s_ss0Direct.titleLoopTickV10;
    const auto titleClock =
        PrSS0TitleHudEventsDirect::ComputeTitleEventClock801C4894(
            currentTitleLoopTickV10,
            s_ss0Direct.titleInitialCtxTick96Known,
            s_ss0Direct.titleInitialCtxTick96);
    s_ss0Direct.titleCtxTick96Known = titleClock.known;
    s_ss0Direct.titleCtxTick96 = titleClock.tick96;
    s_ss0Direct.titleCtxBeatKnown = titleClock.beatKnown;
    s_ss0Direct.titleCtxBeat = titleClock.beat;
    bool titleSelectorStarting = false;
    PrSS0TitleHudEventsDirect::TitleSelectorHudStartResult801C57E0
        titleSelectorStart{};
    if (s_ss0Direct.titleLoopStateV8 == 0u &&
        currentTitleLoopTickV10 >= 50) {
        if (!SS0DirectPrimeTitleMovie0TAudioTail8001A4D0(
                "801C4894 title v8=0->1") ||
            !SS0DirectStopTitleVideoKeepXaAudio(ctx)) {
            Log::Printf(
                "SS0 direct runtime: 801C4894 title v8=0->1 video stop/audio-tail boundary blocked");
            SS0DirectStopTitleStr(ctx);
            return 0;
        }
        s_ss0Direct.titleLoopStateV8 = 1u;
    } else if (s_ss0Direct.titleLoopStateV8 == 1u &&
               currentTitleLoopTickV10 >= kAutoMenuFrame) {
        // Current COMOD0 801C4A60 executes 8001A694 before the cue/HUD
        // start sequence. This is direct command/callback state; no Host
        // player presence or result participates.
        if (!SS0DirectExecuteTitleWaitCleanup8001A694(
                "801C4A60 title natural entry")) {
            return 0;
        }
        titleSelectorStart =
            PrSS0TitleHudEventsDirect::StartTitleSelectorHud801C57E0(
                s_ss0Direct.titleEventRuntimeState,
                s_ss0Direct.titleSelectorHudRuntimeState,
                titleClock.known,
                titleClock.tick96);
        if (!titleSelectorStart.accepted) {
            Log::Printf(
                "SS0 direct runtime: title natural HUD start blocked tickKnown=%u",
                titleClock.known ? 1u : 0u);
            return 0;
        }

        titleSelectorStarting = true;
    }
    if (s_ss0Direct.titleLoopTickV10 <
        (std::numeric_limits<int32_t>::max)()) {
        ++s_ss0Direct.titleLoopTickV10;
    }
    ++s_ss0Direct.titleFrameCounter;

    const PrPadState pad = SS0DirectReadPad(ctx);
    // The original 801C4894 compares the complete mask returned by
    // 80035510 against its v9 previous-mask local while v8 is 0 or 1.
    // Keep this separate from the selector edge state: filtering this to
    // Cross/Left/Right makes a Start/Select/shoulder change invisible and
    // leaves MOVIE0T running past the PSX early-input shortcut boundary.
    const uint32_t titleMovie0TInputPsx =
        SS0DirectFullLocalPadToPsxMask80026744(pad.held);
    const uint32_t titleMovie0TPreviousInputPsx =
        s_ss0Direct.titleMovie0TPreviousInputPsx;
    const bool titleMovie0TInputEdge =
        titleMovie0TInputPsx != 0u &&
        titleMovie0TInputPsx != titleMovie0TPreviousInputPsx;
    const uint16_t titleSelectorEntryMask = static_cast<uint16_t>(
        static_cast<uint16_t>(PrPadButton::Cross) |
        static_cast<uint16_t>(PrPadButton::Left) |
        static_cast<uint16_t>(PrPadButton::Right));
    if (s_ss0Direct.titleLoopStateV8 < 2u && !titleSelectorStarting) {
        // The explicit selector edge is retained as a source-boundary marker
        // (the complete original condition is below), while the v9/v14
        // compare also admits every other nonzero mask change in v8=0/1.
        // if ((pad.pressed & titleSelectorEntryMask) != 0)
        if ((pad.pressed & titleSelectorEntryMask) != 0 ||
            titleMovie0TInputEdge) {
            const bool state0MovieCleanup =
                s_ss0Direct.titleLoopStateV8 == 0u;
            if (titleMovie0TInputEdge) {
                s_ss0Direct.titleMovie0TPreviousInputPsx =
                    titleMovie0TInputPsx;
                Log::Printf(
                    "SS0 direct runtime: title 801C4894 early-input mask change v9=%04X v14=%04X state=%u",
                    static_cast<unsigned>(titleMovie0TPreviousInputPsx),
                    static_cast<unsigned>(titleMovie0TInputPsx),
                    static_cast<unsigned>(s_ss0Direct.titleLoopStateV8));
            }
            s_ss0Direct.titleNaturalLoopCadence = {};
            if (state0MovieCleanup) {
                s_ss0Direct.titleLoopTickV10 = kAutoMenuFrame;
                SS0DirectStopTitleStr(ctx);
            }
            if (!SS0DirectExecuteTitleWaitCleanup8001A694(
                    "801C4B74 title early input")) {
                SS0DirectBlockTitleEarlyInputShortcut801C4894();
                Log::Printf(
                    "SS0 direct runtime: title early input 801C4B74 direct 8001A694 cleanup blocked state=%u",
                    static_cast<unsigned>(s_ss0Direct.titleLoopStateV8));
                return 0;
            }
            if (state0MovieCleanup) {
                const auto graphFlip =
                    PrSS0TitleTmdBackend::
                        ApplyTitleMovie0TEarlyInputGraphFlip801C4B7C();
                if (!graphFlip.committed) {
                    SS0DirectBlockTitleEarlyInputShortcut801C4894();
                    Log::Printf(
                        "SS0 direct runtime: title early input state0 801C4B7C graph flip blocked");
                    return 0;
                }
                s_ss0Direct.titleMovie0TState0GraphFlipBlockedLogged = false;
            }
            const auto setup =
                PrSS0TitleTmdBackend::
                    ApplyTitleEarlyInputShortcutSetupFrame801C6410();
            if (setup.committed) {
                s_ss0Direct.titleEarlyInputShortcutPhase =
                    SS0DirectTitleEarlyInputShortcutPhase::Waiting15WaitCalls;
                s_ss0Direct.titleEarlyInputShortcutCompletedWaitCalls = 0u;
                s_ss0Direct.titleEarlyInputShortcutFrameWaitRemaining =
                    static_cast<uint8_t>(PrSS0TitleHudEventsDirect::
                                             kShortcutFrameWait);
                Log::Printf(
                    "SS0 direct runtime: title early input setup loop=%u camera=%u",
                    static_cast<unsigned>(
                        setup.channel1LoopCount801CC676),
                    setup.cameraCursorAfter801CB59C);
            } else {
                SS0DirectBlockTitleEarlyInputShortcut801C4894();
                Log::Printf(
                    "SS0 direct runtime: title early input setup blocked status=%u",
                    static_cast<unsigned>(setup.status));
                return 0;
            }
        } else if (s_ss0Direct.titleLoopStateV8 == 0u) {
            const auto graphFlip =
                PrSS0TitleTmdBackend::
                    ApplyTitleMovie0TState0GraphFlip801C4B8C();
            if (!graphFlip.committed) {
                s_ss0Direct.titleLoopTickV10 = currentTitleLoopTickV10;
                if (s_ss0Direct.titleFrameCounter > 0) {
                    --s_ss0Direct.titleFrameCounter;
                }
                s_ss0Direct.titleNaturalLoopCadence.holdTicksRemaining = 0u;
                if (!s_ss0Direct.titleMovie0TState0GraphFlipBlockedLogged) {
                    s_ss0Direct.titleMovie0TState0GraphFlipBlockedLogged =
                        true;
                    Log::Printf(
                        "SS0 direct runtime: title MOVIE0T state0 801C4B8C graph flip blocked");
                }
                return 0;
            }
            s_ss0Direct.titleMovie0TState0GraphFlipBlockedLogged = false;
            SS0DirectAdvanceTitleMovie0T801C455C(ctx);
        } else if (s_ss0Direct.titleLoopStateV8 == 1u) {
            // COMOD0 801C4C90..801C4CA4 is ordered exactly as
            // PrScene0_Menu_Render -> ctx flags clear -> 80024FD0 ->
            // PrStageRunner_FrameUpdate.  The render call must consume the
            // event committed at the top of this loop before the clear; in
            // particular, clearing first drops the 0x20 title activation bit
            // and removes the original PA/PA_KAGE/LO/HP title scene.
            SS0DirectAdvanceTitleVisualState801C5EF0();
            s_ss0Direct.titleEventRuntimeState.ctxFlags = 0u;
            if (!SS0DirectTickTitleSharedEventBucketPrefix80024FD0(
                    ctx, 0x801C4C9Cu, titleClock)) {
                s_ss0Direct.titleLoopTickV10 = currentTitleLoopTickV10;
                if (s_ss0Direct.titleFrameCounter > 0) {
                    --s_ss0Direct.titleFrameCounter;
                }
                s_ss0Direct.titleNaturalLoopCadence.holdTicksRemaining = 0u;
                return 0;
            }
            (void)SS0DirectAdvanceTitleHudTimeline801C5094(
                ctx, titleClock);
            SS0DirectQueueTitleAbsoluteEvent801C5538(titleClock);
        }
        s_ss0Direct.titleSelectorPreviousInputPsx = static_cast<uint16_t>(
            SS0DirectLocalPressedToPsxPadMask(pad.held));
        s_ss0Direct.titleMovie0TPreviousInputPsx = titleMovie0TInputPsx;
        ctx.scn0HiliteCursor = s_ss0Direct.titleCursor;
        return 0;
    }

    // COMOD0 801C4CB4..801C4D24 renders the current title packet before it
    // clears ctx flags, draws the selector prompt, handles input, dispatches
    // 80024FD0 and updates the HUD.  Keep the selector path on that exact
    // order; the next title iteration consumes the newly published HUD/MIME
    // state.
    SS0DirectAdvanceTitleVisualState801C5EF0();
    const uint16_t titleSelectorCurrentLocalInput =
        static_cast<uint16_t>(pad.held | pad.pressed);
    // 80035510 supplies the complete controller word to 8001C4CE0/47EC.
    // Keep the action-family projection explicit for the exact Cross and
    // Left/Right handlers, then merge the remaining PSX bits back in so an
    // unsupported held button still changes v9 and resets the attract timer
    // exactly as the original loop does; ResolveTitleSelectorInput filters
    // those bits from direct actions.
    const uint32_t titleSelectorActionPsxMask =
        SS0DirectLocalPressedToPsxPadMask(titleSelectorCurrentLocalInput);
    const uint32_t titleSelectorFullPsxMask =
        SS0DirectFullLocalPadToPsxMask80026744(
            titleSelectorCurrentLocalInput);
    const uint16_t titleSelectorInputMask = static_cast<uint16_t>(
        titleSelectorActionPsxMask |
        (titleSelectorFullPsxMask & ~titleSelectorActionPsxMask));
    s_ss0Direct.titleEventRuntimeState.ctxFlags = 0u;
    const auto titleSelectorInputEdge =
        PrSS0TitleHudEventsDirect::ResolveTitleSelectorInputEdge801C4CE0(
            true,
            s_ss0Direct.titleSelectorPreviousInputPsx,
            true,
            titleSelectorInputMask,
            true,
            static_cast<uint8_t>(s_ss0Direct.titleCursor));
    if (!titleSelectorInputEdge.accepted) {
        Log::Printf(
            "SS0 direct runtime: title 801C4CE0/801C47EC input blocked previous=%04X current=%04X cursor=%d status=%u",
            s_ss0Direct.titleSelectorPreviousInputPsx,
            titleSelectorInputMask,
            s_ss0Direct.titleCursor,
            static_cast<unsigned>(titleSelectorInputEdge.action.status));
        return 0;
    }

    if (!SS0DirectTickTitleSharedEventBucketPrefix80024FD0(
            ctx, 0x801C4D14u, titleClock)) {
        s_ss0Direct.titleLoopTickV10 = currentTitleLoopTickV10;
        if (s_ss0Direct.titleFrameCounter > 0) {
            --s_ss0Direct.titleFrameCounter;
        }
        s_ss0Direct.titleNaturalLoopCadence.holdTicksRemaining = 0u;
        return 0;
    }

    const uint8_t titleSelectorCandidateCursor =
        titleSelectorInputEdge.action.cursorWrite
        ? titleSelectorInputEdge.action.nextCursor
        : static_cast<uint8_t>(s_ss0Direct.titleCursor);
    const auto selectorTickInput =
        SS0DirectBuildTitleSelectorHudTickInput801C5854(
            titleClock.known,
            titleClock.tick96,
            titleClock.beatKnown,
            titleClock.beat,
            titleSelectorCandidateCursor);
    const auto selectorTick =
        PrSS0TitleHudEventsDirect::TickTitleSelectorHud801C5854(
            titleSelectorStarting
                ? titleSelectorStart.nextEventState
                : s_ss0Direct.titleEventRuntimeState,
            titleSelectorStarting
                ? titleSelectorStart.nextSelectorState
                : s_ss0Direct.titleSelectorHudRuntimeState,
            selectorTickInput);
    PrSS0TitleTmdBackend::PreparedRuntimeChannel2MimeBinding801C6410
        preparedSelectorMime{};
    if (!SS0DirectPrepareTitleSelectorHudTick801C5854(
            ctx,
            selectorTick,
            preparedSelectorMime,
            titleSelectorCandidateCursor)) {
        Log::Printf(
            "SS0 direct runtime: title 801C4CE0/801C47EC HUD prepare blocked status=%u starting=%u cursor=%u",
            static_cast<unsigned>(selectorTick.status),
            titleSelectorStarting ? 1u : 0u,
            static_cast<unsigned>(titleSelectorCandidateCursor));
        return 0;
    }

    if (titleSelectorStarting) {
        s_ss0Direct.titleLoopStateV8 = 2u;
        (void)SS0DirectRestartTitleAttractTimer801C4894();
        // Pseudo-C 801C4894 starts the title selector with
        // 80026EF8(dword_80094410).  94410 is the title/menu BGM command,
        // not a one-shot transition effect: the PSX voice is left looping
        // until the selector/title exit path resets the shared driver.
        // PlayBgm() is the direct owner that binds tone 7, installs the VAG
        // loop region and keeps the voice observable for the whole title
        // selector.  The old one-shot helper allocated an untracked voice,
        // so the selector appeared silent and the FX tail was cut early.
        PrSfx::PlayBgm();
        PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
    }
    if (titleSelectorInputEdge.resetTimeout) {
        (void)SS0DirectRestartTitleAttractTimer801C4894();
    }
    if (titleSelectorInputEdge.action.playCue) {
        SS0DirectPlayTitleSelectorInputCue(
            titleSelectorInputEdge.action.cueMask);
    }
    if (titleSelectorInputEdge.action.cursorWrite) {
        s_ss0Direct.titleCursor = titleSelectorInputEdge.action.nextCursor;
    }
    s_ss0Direct.titleSelectorPreviousInputPsx =
        titleSelectorInputEdge.nextPreviousInput;
    SS0DirectPublishPreparedTitleSelectorHudTick801C5854(
        selectorTick, preparedSelectorMime);
    if (titleSelectorStarting) {
        Log::Printf(
            "SS0 direct runtime: title normal 8001A694 -> 94410 BGM loop/26ECC -> HUD start tick96=%u beat=%u slot=%u",
            titleClock.tick96,
            static_cast<unsigned>(titleClock.beat),
            static_cast<unsigned>(
                s_ss0Direct.titleEventRuntimeState.hudSlot0));
    }

    if (titleSelectorInputEdge.action.selectorResult !=
        PrSS0TitleHudEventsDirect::kTitleSelectorResultNone801C47EC) {
        const PrSceneId currentScene = ctx.currentScene;
        if (titleSelectorInputEdge.action.selectorResult ==
            PrSS0TitleHudEventsDirect::kTitleSelectorResultStart801C47EC) {
            const int targetScene = static_cast<int>(currentScene) + 1;
            if (currentScene != PrSceneId::Scene0 || targetScene != 1) {
                Log::Printf(
                    "SS0 direct runtime: START title source gap source=%d target=%d",
                    static_cast<int>(currentScene),
                    targetScene);
                return 0;
            }
            Log::Printf(
                "SS0 direct runtime: START selector accepted target=%d result deferred",
                targetScene);
            return SS0DirectArmTitleExitWait(
                titleSelectorInputEdge.action.selectorResult,
                currentScene,
                targetScene,
                false);
        }

        Log::Printf("SS0 direct runtime: title MENU selected result deferred");
        return SS0DirectArmTitleExitWait(
            titleSelectorInputEdge.action.selectorResult,
            currentScene,
            -1,
            true);
    }

    if (s_ss0Direct.titleAttractTimeoutKnown &&
        --s_ss0Direct.attractTimer <= 0) {
        Log::Printf(
            "SS0 direct runtime: attract timeout accepted random result deferred ee=%d",
            s_ss0Direct.word800916EE);
        return SS0DirectArmTitleExitWait(
            static_cast<int>(
                PrSS0TransitionDirect::kTitleSelectorResultRandom801C4DC4),
            ctx.currentScene,
            -1,
            false);
    }

    ctx.scn0HiliteCursor = s_ss0Direct.titleCursor;
    return 0;
}

static bool SS0DirectSubmitTitleTim(PrGameContext& ctx,
                                    const char* textureName,
                                    float vx,
                                    float vy,
                                    float vs,
                                    float psxX,
                                    float psxY,
                                    int layer) {
    if (!ctx.renderer || !ctx.resources || !textureName) {
        return false;
    }

    ID3D11ShaderResourceView* textureView =
        ctx.resources->GetTextureView(textureName);
    TextureResource* texture = ctx.resources->GetTexture(textureName);
    if (!textureView || !texture || texture->tim.width == 0 ||
        texture->tim.height == 0) {
        return false;
    }

    D3D11Renderer::SpriteCmd cmd{};
    cmd.texture = textureView;
    cmd.x = vx + psxX * vs;
    cmd.y = vy + psxY * vs;
    cmd.w = static_cast<float>(texture->tim.width) * vs;
    cmd.h = static_cast<float>(texture->tim.height) * vs;
    cmd.u0 = 0.0f;
    cmd.v0 = 0.0f;
    cmd.u1 = 1.0f;
    cmd.v1 = 1.0f;
    cmd.r = 1.0f;
    cmd.g = 1.0f;
    cmd.b = 1.0f;
    cmd.a = 1.0f;
    cmd.blend = D3D11Renderer::BlendMode::Alpha;
    cmd.layer = layer;
    cmd.order = 0;
    ctx.renderer->SubmitSprite(cmd);
    return true;
}

static bool SS0DirectTitleTimAvailable(PrGameContext& ctx,
                                       const char* textureName) {
    if (!ctx.resources || !textureName) {
        return false;
    }
    ID3D11ShaderResourceView* textureView =
        ctx.resources->GetTextureView(textureName);
    TextureResource* texture = ctx.resources->GetTexture(textureName);
    return textureView && texture && texture->tim.width != 0 &&
           texture->tim.height != 0;
}

static PrPsxSpriteTemplateRender::PsxSpriteTemplate
SS0DirectMakeTitleSelectorPromptTemplate80020488(
    const PrSS0TitleHudEventsDirect::TitleSelectorPromptTemplate80020488&
        source) {
    PrPsxSpriteTemplateRender::PsxSpriteTemplate out{};
    out.attr = source.attr;
    out.texX_hw = source.texX;
    out.texY_px = source.texY;
    out.w = source.width;
    out.h = source.height;
    out.clutX_px = source.clutX;
    out.clutY_px = source.clutY;
    return out;
}

using SS0DirectTitleSelectorPromptTemplates80020488 =
    std::array<PrPsxSpriteTemplateRender::PsxSpriteTemplate,
               PrSS0TitleHudEventsDirect::
                   kTitleSelectorPromptSpriteCount80020488>;

static bool SS0DirectResolveTitleSelectorPromptResources8001E3E4(
    PrGameContext& ctx,
    uint8_t cursor,
    PrSS0TitleHudEventsDirect::TitleSelectorPromptPlan80020488& plan,
    SS0DirectTitleSelectorPromptTemplates80020488& templates) {
    plan = PrSS0TitleHudEventsDirect::BuildTitleSelectorPrompt8001E3E4(
        true, static_cast<uint32_t>(cursor));
    if (!plan.accepted || !ctx.resources) {
        return false;
    }

    for (std::size_t i = 0; i < plan.sprites.size(); ++i) {
        const auto& sprite = plan.sprites[i];
        if (!sprite.active) {
            return false;
        }
        templates[i] = SS0DirectMakeTitleSelectorPromptTemplate80020488(
            sprite.spriteTemplate);
        const int bpp =
            PrPsxSpriteTemplateRender::PsxBppFromAttr(templates[i].attr);
        TextureResource* texture = ctx.resources->FindTextureByTimHeader(
            bpp,
            static_cast<int16_t>(templates[i].texX_hw),
            static_cast<int16_t>(templates[i].texY_px),
            static_cast<uint32_t>(templates[i].w),
            static_cast<uint32_t>(templates[i].h),
            static_cast<int16_t>(templates[i].clutX_px),
            static_cast<int16_t>(templates[i].clutY_px));
        if (!texture) {
            texture =
                PrPsxSpriteTemplateRender::FindLoadedTimTextureByTemplate(
                    ctx, templates[i]);
        }
        if (!texture || !ctx.resources->GetTextureView(*texture)) {
            return false;
        }
    }
    return true;
}

static bool SS0DirectPreflightTitleSelectorPrompt8001E3E4(
    PrGameContext& ctx,
    uint8_t cursor) {
    PrSS0TitleHudEventsDirect::TitleSelectorPromptPlan80020488 plan{};
    SS0DirectTitleSelectorPromptTemplates80020488 templates{};
    return SS0DirectResolveTitleSelectorPromptResources8001E3E4(
        ctx, cursor, plan, templates);
}

static bool SS0DirectRenderTitleSelectorPrompt8001E3E4(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs) {
    PrSS0TitleHudEventsDirect::TitleSelectorPromptPlan80020488 plan{};
    SS0DirectTitleSelectorPromptTemplates80020488 templates{};
    if (!SS0DirectResolveTitleSelectorPromptResources8001E3E4(
            ctx,
            static_cast<uint8_t>(s_ss0Direct.titleCursor),
            plan,
            templates)) {
        return false;
    }

    for (std::size_t i = 0; i < plan.sprites.size(); ++i) {
        const auto& sprite = plan.sprites[i];
        if (!PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateOrdered(
                ctx,
                vx,
                vy,
                vs,
                static_cast<float>(sprite.x),
                static_cast<float>(sprite.y),
                templates[i],
                1.0f,
                1.0f,
                1.0f,
                1.0f,
                500,
                static_cast<int>(i))) {
            return false;
        }
    }
    return true;
}

static bool SS0DirectRenderOpeningMovie0NativeRoleGrid(PrGameContext& ctx);
static bool SS0DirectRenderOpeningMovie0NativeFrame(PrGameContext& ctx);

static void SS0DirectRenderOpeningMovie0(PrGameContext& ctx,
                                         bool whiteBase = false) {
    if (!ctx.renderer) {
        return;
    }
    const float winW = static_cast<float>(ctx.renderer->GetWidth());
    const float winH = static_cast<float>(ctx.renderer->GetHeight());
    // The post-MOVIE0 mode-5 8001F230 outro is drawn over the previous
    // white subtitle/video page.  Keep that frozen-page base white while the
    // native edge tiles progressively cover the video and subbox; the
    // pre-video path still starts from the original black clear.
    const float base = whiteBase ? 1.0f : 0.0f;
    ctx.renderer->DrawRect(
        0.0f, 0.0f, winW, winH, base, base, base, 1.0f);

    // The original 80040420/MOVIE0 path keeps the all-192 role-grid page
    // behind the STR image and submits the native 8001CE30/8001C864 frame
    // after the MDEC quad.  This is deliberately composed from SS0's
    // direct frame plans; RenderMovie0FillTiles/RenderMovie0Frame are the
    // retired Win shell and must not own this phase.
    if (!SS0DirectRenderOpeningMovie0NativeRoleGrid(ctx)) {
        static bool loggedOpeningMovie0GridBlocked = false;
        if (!loggedOpeningMovie0GridBlocked) {
            loggedOpeningMovie0GridBlocked = true;
            Log::Printf(
                "SS0 direct runtime: MOVIE0 native role-grid backdrop blocked");
        }
    }

    if (!s_ss0Direct.strStarted) {
        (void)SS0DirectRenderOpeningMovie0NativeFrame(ctx);
        return;
    }

    const auto geometry =
        PrSS0StrLifecycleDirect::KnownDecodeGeometry80027288(
            static_cast<int32_t>(
                PrSS0StrLifecycleDirect::StrMovieKind::OpeningMovie0),
            s_ss0Direct.optionsWord800916DC != 0);
    if (!geometry.known) {
        return;
    }
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (s_ss0Direct.strDirectMdecTextureReady &&
        s_ss0Direct.strDirectMdecTexture != nullptr &&
        s_ss0Direct.strDirectMdecTextureRenderer == ctx.renderer) {
        // Keep the MDEC quad in the same ordered queue as the native PSX
        // sprites.  DrawSprite is immediate and would be covered by the
        // queued role-grid tiles before FlushSprites; layer 700 places the
        // movie between the grid (low) and the native frame (high).
        D3D11Renderer::SpriteCmd movie{};
        movie.texture = s_ss0Direct.strDirectMdecTexture;
        movie.x = vx + static_cast<float>(geometry.dstX) * vs;
        movie.y = vy + static_cast<float>(geometry.dstY) * vs;
        movie.w = static_cast<float>(geometry.width) * vs;
        movie.h = static_cast<float>(geometry.height) * vs;
        movie.layer = 700;
        ctx.renderer->SubmitSprite(movie);
    }
    // The direct MDEC texture is the only SS0 movie presentation source.
    // Keep the frame black until that texture becomes ready; extracted STR
    // files and the Windows StrPlayer are not part of the direct path.

    (void)SS0DirectRenderOpeningMovie0NativeFrame(ctx);
}

static PrPsxSpriteTemplateRender::PsxSpriteTemplate
SS0DirectMakeFastTransitionSpriteTemplate800201AC(
    const PrSS0TransitionDirect::FastTransitionSpriteTemplate800201AC&
        source) {
    PrPsxSpriteTemplateRender::PsxSpriteTemplate out{};
    out.attr = source.attr;
    out.texX_hw = source.texX;
    out.texY_px = source.texY;
    out.w = source.width;
    out.h = source.height;
    out.clutX_px = source.clutX;
    out.clutY_px = source.clutY;
    return out;
}

using SS0DirectTransitionTemplateArray8001B590 =
    std::array<PrPsxSpriteTemplateRender::PsxSpriteTemplate,
               PrSS0TransitionDirect::kFastTransitionMaxSpriteCommands800201AC>;

static bool SS0DirectResolveTransitionSpriteResources8001B590(
    PrGameContext& ctx,
    const PrSS0TransitionDirect::FastTransitionSpriteCommand800201AC* commands,
    std::size_t commandCount,
    SS0DirectTransitionTemplateArray8001B590& templates) {
    if (!commands || !ctx.renderer || !ctx.resources) {
        return false;
    }
    if (commandCount == 0u) {
        return true;
    }

    for (std::size_t i = 0; i < commandCount; ++i) {
        const auto& command = commands[i];
        if (!command.active || !command.rawTextureKnown ||
            !command.rawTexture || !command.semiTransparentKnown ||
            !command.abrKnown || command.abr != 1u || command.rgbKnown ||
            !command.spriteTemplate.known ||
            (command.priority != 0u && command.priority != 5u)) {
            static bool loggedTransitionResourceReject = false;
            if (!loggedTransitionResourceReject) {
                Log::Printf(
                    "SS0 direct runtime: transition resource preflight reject index=%u active=%d rawKnown=%d raw=%d semiKnown=%d abrKnown=%d abr=%u rgb=%d templateKnown=%d priority=%u source=0x%08X",
                    static_cast<unsigned>(i),
                    command.active ? 1 : 0,
                    command.rawTextureKnown ? 1 : 0,
                    command.rawTexture ? 1 : 0,
                    command.semiTransparentKnown ? 1 : 0,
                    command.abrKnown ? 1 : 0,
                    static_cast<unsigned>(command.abr),
                    command.rgbKnown ? 1 : 0,
                    command.spriteTemplate.known ? 1 : 0,
                    static_cast<unsigned>(command.priority),
                    static_cast<unsigned>(command.spriteTemplate.sourceAddress));
                loggedTransitionResourceReject = true;
            }
            return false;
        }
        templates[i] =
            SS0DirectMakeFastTransitionSpriteTemplate800201AC(
                command.spriteTemplate);
        TextureResource* texture =
            PrPsxSpriteTemplateRender::FindLoadedTimTextureByTemplate(
                ctx, templates[i]);
        // COMPO00 transition templates are commonly 20x20 subrects of a
        // shared PSX VRAM page rather than standalone TIM resources.  The
        // renderer has a native tpage/clut atlas path for those sprites; do
        // not reject the whole original transition merely because the
        // standalone-resource probe misses a subrect.
        if (!texture) {
            continue;
        }
        ID3D11ShaderResourceView* textureView = nullptr;
        if (command.semiTransparent) {
            textureView =
                ctx.resources->GetTexturePsxAbr1StpView(*texture);
        } else {
            if (!texture->srv && !texture->tim.rgba.empty()) {
                texture->srv = ctx.renderer->CreateTexture(
                    texture->tim.rgba.data(),
                    texture->tim.width,
                    texture->tim.height);
            }
            textureView = texture->srv;
        }
        // A texture without a host SRV can still be rendered from the PSX
        // VRAM atlas below; resource preflight is therefore only a fast path
        // and must not turn a valid original sprite plan into a black frame.
        (void)textureView;
    }
    return true;
}

static bool SS0DirectRenderTransitionSpriteCommands8001B590(
    PrGameContext& ctx,
    const PrSS0TransitionDirect::FastTransitionSpriteCommand800201AC* commands,
    std::size_t commandCount,
    int layerBias = 0) {
    SS0DirectTransitionTemplateArray8001B590 templates{};
    if (!SS0DirectResolveTransitionSpriteResources8001B590(
            ctx, commands, commandCount, templates)) {
        return false;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    for (std::size_t i = 0; i < commandCount; ++i) {
        const auto& command = commands[i];
        const int layer = layerBias +
                          (command.priority == 0u ? 784 : 779);
        // The PSX 8001FDC0/80021E60 paths share the same OT priority and are
        // ordered by their insertion into that bucket.  Transition plans
        // carry a local 1-based command index, but replaying that index on
        // the host collides with the directory's own explicit/automatic
        // orders and can put Menu sprites above the grid.  Let the renderer
        // assign one global monotonically increasing order here; submission
        // order remains the source-of-truth for both the slow mode-4 reveal
        // and the fast transition variants.
        const int hostOrder = 0;
        const bool submitted =
            command.semiTransparent
                ? PrPsxSpriteTemplateRender::
                      DrawPsxSpriteTemplateAbr1StpOrdered(
                          ctx,
                          vx,
                          vy,
                          vs,
                          static_cast<float>(command.x),
                          static_cast<float>(command.y),
                          templates[i],
                          1.0f,
                          1.0f,
                          1.0f,
                          1.0f,
                          layer,
                          hostOrder)
                : PrPsxSpriteTemplateRender::DrawPsxSpriteTemplateOrdered(
                      ctx,
                      vx,
                      vy,
                      vs,
                      static_cast<float>(command.x),
                      static_cast<float>(command.y),
                      templates[i],
                      1.0f,
                      1.0f,
                      1.0f,
                      1.0f,
                      layer,
                      hostOrder);
        if (!submitted) {
            Log::Printf(
                "SS0 direct runtime: transition sprite submit blocked index=%u semi=%d attr=0x%08X tex=(%u,%u) size=%ux%u clut=(%u,%u)",
                static_cast<unsigned>(i),
                command.semiTransparent ? 1 : 0,
                command.spriteTemplate.attr,
                static_cast<unsigned>(command.spriteTemplate.texX),
                static_cast<unsigned>(command.spriteTemplate.texY),
                static_cast<unsigned>(command.spriteTemplate.width),
                static_cast<unsigned>(command.spriteTemplate.height),
                static_cast<unsigned>(command.spriteTemplate.clutX),
                static_cast<unsigned>(command.spriteTemplate.clutY));
            return false;
        }
    }
    return true;
}

static bool SS0DirectPreflightSlowTransitionTextures80020110(
    PrGameContext& ctx,
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg) {
    const auto visual =
        PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
            ctxAddress,
            mode,
            preFfd4Arg,
            postFfd4Arg,
            mode == 2,
            0u);
    const auto plan =
        PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(visual);
    SS0DirectTransitionTemplateArray8001B590 templates{};
    return plan.known && !plan.truncated &&
           SS0DirectResolveTransitionSpriteResources8001B590(
               ctx, plan.commands, plan.commandCount, templates);
}

static bool SS0DirectPreflightFastTransitionTextures800201AC(
    PrGameContext& ctx,
    const PrSS0TransitionDirect::FastTransitionVisualFrame800201AC& visual) {
    const auto plan =
        PrSS0TransitionDirect::BuildFastTransitionFramePlan800201AC(visual);
    SS0DirectTransitionTemplateArray8001B590 templates{};
    return plan.known && !plan.truncated &&
           SS0DirectResolveTransitionSpriteResources8001B590(
               ctx, plan.commands, plan.commandCount, templates);
}

static bool SS0DirectRenderFastTransitionSprites800201AC(
    PrGameContext& ctx,
    const PrSS0TransitionDirect::FastTransitionVisualFrame800201AC& visual,
    int layerBias = 0) {
    const auto plan =
        PrSS0TransitionDirect::BuildFastTransitionFramePlan800201AC(visual);
    if (!plan.known || plan.truncated) {
        static bool loggedTransitionPlanReject = false;
        if (!loggedTransitionPlanReject) {
            Log::Printf(
                "SS0 direct runtime: transition frame plan rejected known=%d truncated=%d kind=%u count=%u",
                plan.known ? 1 : 0,
                plan.truncated ? 1 : 0,
                static_cast<unsigned>(plan.kind),
                static_cast<unsigned>(plan.commandCount));
            loggedTransitionPlanReject = true;
        }
    }
    return plan.known && !plan.truncated &&
           SS0DirectRenderTransitionSpriteCommands8001B590(
               ctx, plan.commands, plan.commandCount, layerBias);
}

static bool SS0DirectRenderSlowTransitionTiles8001F5248001FDC0(
    PrGameContext& ctx,
    const PrSS0TransitionDirect::SlowTransitionVisualFrame80020110& visual,
    int layerBias = 0) {
    const auto plan =
        PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(visual);
    return plan.known && !plan.truncated &&
           SS0DirectRenderTransitionSpriteCommands8001B590(
               ctx, plan.commands, plan.commandCount, layerBias);
}

static bool SS0DirectRenderCompletedLoadingPattern8001EF40(
    PrGameContext& ctx) {
    const auto& frame = s_ss0Direct.loadingPatternFrame8001EF40;
    if (!s_ss0Direct.loadingPatternRuntime8001EF40.active) {
        return true;
    }
    if (!ctx.renderer || !frame.known ||
        frame.sourceFunction != PrSS0TransitionDirect::kFn8001EF40 ||
        frame.drawHighlightFunction != PrSS0TransitionDirect::kFn8001C4EC ||
        frame.drawTileFunction != PrSS0TransitionDirect::kFn8001C550 ||
        frame.style >= 4u ||
        static_cast<uint8_t>(frame.boxFillAttr8001B6C4 >> 24u) != 0x40u ||
        (static_cast<uint8_t>(
             frame.boxFillGpuColorCode8003EE84 >> 24u) & 0xFCu) != 0x60u) {
        return false;
    }

    // 8001EF40 draws all 192 base tiles on every callback, independently of
    // the preceding transition's live mask.  A reveal mask underneath this
    // page used to expose menu pixels through the Loading letters.
    const auto fullGrid =
        PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
            PrSS0TransitionDirect::kScene0WorkAddress, 2, 1, 2, false, 23u);
    if (!fullGrid.known || fullGrid.activeCount != 192u ||
        !SS0DirectRenderSlowTransitionTiles8001F5248001FDC0(ctx, fullGrid)) {
        return false;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    const float tileW = 20.0f * vs;
    const float tileH = 20.0f * vs;
    // 8001EF40 passes dword_800508B0[style*2] to 8001C4EC/8001B6C4
    // as a GsBOXF attribute.  8001B6C4 extracts RGB from bits 16/8/0;
    // GsSortBoxFill maps attr bit 30 to GPU command 0x62 (semi-transparent).
    // Treating the high byte as literal alpha or a final GPU command makes
    // the Loading mask opaque and hides the role tiles, unlike the PSX frame.
    const uint32_t colorCode = frame.boxFillGpuColorCode8003EE84;
    const float r = static_cast<float>(colorCode & 0xFFu) / 255.0f;
    const float g = static_cast<float>(
                        (colorCode >> 8u) & 0xFFu) /
                    255.0f;
    const float b = static_cast<float>((colorCode >> 16u) & 0xFFu) /
                    255.0f;
    const float a = ((colorCode >> 24u) & 0x02u) != 0u ? 0.5f : 1.0f;

    uint32_t submitted = 0u;
    for (std::size_t row = 0u;
         row < PrSS0TransitionDirect::kLoadingPatternGridRows8001EF40;
         ++row) {
        for (std::size_t col = 0u;
             col < PrSS0TransitionDirect::kLoadingPatternGridColumns8001EF40;
             ++col) {
            const std::size_t index =
                row *
                    PrSS0TransitionDirect::kLoadingPatternGridColumns8001EF40 +
                col;
            if (frame.liveGrid[index] == 0u) {
                continue;
            }
            D3D11Renderer::SolidRectCmd command{};
            command.x = vx + static_cast<float>(col) * tileW;
            command.y = vy + static_cast<float>(row) * tileH;
            command.w = tileW;
            command.h = tileH;
            command.r = r;
            command.g = g;
            command.b = b;
            command.a = a;
            // 8001C4EC and 8001C550 both use priority zero.  Preserve that
            // bucket while ordering the colour tile after the base role tile,
            // matching PSX AddPrim's last-in-first-executed list semantics.
            command.layer = 784;
            command.order = (uint64_t{1} << 48u) |
                            static_cast<uint64_t>(index + 1u);
            ctx.renderer->SubmitSolidRect(command);
            ++submitted;
        }
    }
    if (submitted != frame.highlightCount) {
        return false;
    }
    s_ss0Direct.loadingPatternSubmittedHighlightCount8001EF40 = submitted;
    s_ss0Direct.loadingPatternSubmittedGridFnv1a8001EF40 =
        frame.liveGridFnv1a;
    s_ss0Direct.loadingPatternSubmittedFrame8001EF40 = ctx.frame;
    return true;
}

static bool SS0DirectRenderOpeningMovie0NativeRoleGrid(PrGameContext& ctx) {
    // MOVIE0's pre-video page is the completed mode-2 80020110 grid.  Using
    // the direct visual resolver at its final body iteration preserves the
    // original tile order/resource ownership without reviving the legacy
    // RenderMovie0FillTiles shell.
    const auto visual =
        PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
            PrSS0TransitionDirect::kScene0WorkAddress,
            2,
            1,
            2,
            false,
            23u);
    return visual.known && visual.activeCount == 192u &&
           SS0DirectRenderSlowTransitionTiles8001F5248001FDC0(
               ctx, visual, -300);
}

static bool SS0DirectRenderOpeningMovie0NativeFrame(PrGameContext& ctx) {
    // After the MDEC quad, the original 80040420 path calls either
    // 8001CE30 (no subtitle box) or 8001C864 (subtitle box).  Both frame
    // plans are already decoded from the PSX template table and share the
    // native S_FRM05..S_FRM14 atlas regions used by the scene.
    const auto visual =
        PrSS0TransitionDirect::ResolveFastTransitionVisualFrame800201AC(
            6,
            s_ss0Direct.optionsWord800916DC != 0,
            false,
            15u);
    return visual.known &&
           SS0DirectRenderFastTransitionSprites800201AC(ctx, visual, 300);
}

static void SS0DirectRenderOpeningMovie0Transition80040420(
    PrGameContext& ctx) {
    const auto& visual = s_ss0Direct.openingMovie0TransitionVisual;
    if (!ctx.renderer) {
        return;
    }
    const float winW = static_cast<float>(ctx.renderer->GetWidth());
    const float winH = static_cast<float>(ctx.renderer->GetHeight());
    const bool retainedMovieOutro =
        visual.known && s_ss0Direct.openingMovie0FrozenFrameHeld &&
        (visual.kind == PrSS0TransitionDirect::
                          FastTransitionFrameKind800201AC::
                              OutroNoSubboxFrame8001F230 ||
         visual.kind == PrSS0TransitionDirect::
                          FastTransitionFrameKind800201AC::
                              FinalNoVideoFrame8001FEB4);
    if (visual.known &&
        visual.kind == PrSS0TransitionDirect::
                           FastTransitionFrameKind800201AC::
                               OutroNoSubboxFrame8001F230 &&
        s_ss0Direct.openingMovie0FrozenFrameHeld) {
        SS0DirectRenderOpeningMovie0(ctx, true);
    } else if (retainedMovieOutro) {
        // With subtitle OFF, 80020308(0) returns zero and 8001EBF4 does not
        // issue a new 80040420 clear.  The preceding mode-6 page was already
        // white, so preserve that frozen-page base instead of reintroducing a
        // black host clear before the 8001FEB4 role-grid sprites.
        ctx.renderer->DrawRect(
            0.0f, 0.0f, winW, winH, 1.0f, 1.0f, 1.0f, 1.0f);
    } else {
        ctx.renderer->DrawRect(
            0.0f, 0.0f, winW, winH, 0.0f, 0.0f, 0.0f, 1.0f);
    }
    if (visual.known && visual.clearColorKnown) {
        ctx.renderer->DrawRect(
            0.0f,
            0.0f,
            winW,
            winH,
            static_cast<float>(visual.clearR) / 255.0f,
            static_cast<float>(visual.clearG) / 255.0f,
            static_cast<float>(visual.clearB) / 255.0f,
            1.0f);
    }
    // 8001F230 is the post-MOVIE0 wipe drawn into the same PSX ordering
    // table after the retained MDEC page and 8001C864 subtitle frame.  Keep
    // its host layer above that retained page; the default transition layer
    // (779/784) would otherwise leave the native subtitle frame on top and
    // make the gradual video/subbox disappearance invisible.
    const int transitionLayerBias =
        visual.known &&
                visual.kind == PrSS0TransitionDirect::
                                   FastTransitionFrameKind800201AC::
                                       OutroNoSubboxFrame8001F230 &&
                s_ss0Direct.openingMovie0FrozenFrameHeld
            ? 400
            : 0;
    if (!SS0DirectRenderFastTransitionSprites800201AC(
            ctx, visual, transitionLayerBias)) {
        Log::Printf(
            "SS0 direct runtime: 8001C864/8001CE30 render blocked after preflight kind=%u iteration=%u",
            static_cast<unsigned>(visual.kind),
            visual.iteration);
    }
}

static void SS0DirectRenderTitle(PrGameContext& ctx,
                                 bool applyPresentModel = true,
                                 bool forceWhiteBase = false) {
    if (!ctx.renderer) {
        return;
    }

    static bool s_loggedTitleRenderOwnership = false;
    if (!s_loggedTitleRenderOwnership) {
        s_loggedTitleRenderOwnership = true;
        Log::Printf("SS0 direct runtime: title render owned by direct runtime");
    }

    const float winW = static_cast<float>(ctx.renderer->GetWidth());
    const float winH = static_cast<float>(ctx.renderer->GetHeight());
    const bool titleMovie0TPreloop =
        s_ss0Direct.phase == SS0DirectPhase::TitleMovie0TFinalReady &&
        s_ss0Direct.titleMovie0TFinalReadyPreloopActive;
    const bool titleMovie0TVisible =
        s_ss0Direct.titleLoopStateV8 == 0u && s_ss0Direct.strStarted &&
        !titleMovie0TPreloop;
    const bool titleMovie0TFrameReady =
        titleMovie0TVisible && s_ss0Direct.strDirectMdecTextureReady &&
        s_ss0Direct.strDirectMdecTexture != nullptr &&
        s_ss0Direct.strDirectMdecTextureRenderer == ctx.renderer;
    // 801C4958's mode-1 title intro is a role-grid reveal over the retained
    // white page from the MOVIE0 post transition.  The MOVIE0T STR is already
    // started for its direct timing/audio owner, but it must not turn this
    // transition's page clear back to the old black host background.
    // 801C4958 returns from the mode-1 grid into 801C4968, which starts
    // MOVIE0T before the first decoded MDEC frame is available.  The PSX
    // keeps the completed white transition page on screen until that first
    // frame is ready; clearing the host target to black here exposes a
    // one-present black flash immediately before the title video.  Retain
    // that white page for the same boundary instead of inventing a black
    // frame that the translated 80020110 path never submitted.
    const float titleBase = forceWhiteBase
        ? 1.0f
        : (titleMovie0TVisible && titleMovie0TFrameReady ? 0.0f : 1.0f);
    ctx.renderer->DrawRect(
        0.0f, 0.0f, winW, winH,
        titleBase, titleBase, titleBase, 1.0f);

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);

    if (titleMovie0TVisible) {
        const auto geometry =
            PrSS0StrLifecycleDirect::KnownDecodeGeometry80027288(
                static_cast<int32_t>(
                    PrSS0StrLifecycleDirect::StrMovieKind::TitleMovie0T),
                s_ss0Direct.optionsWord800916DC != 0);
        if (geometry.known) {
            if (titleMovie0TFrameReady) {
                D3D11Renderer::SpriteCmd movie{};
                movie.texture = s_ss0Direct.strDirectMdecTexture;
                movie.x = vx + static_cast<float>(geometry.dstX) * vs;
                movie.y = vy + static_cast<float>(geometry.dstY) * vs;
                movie.w = static_cast<float>(geometry.width) * vs;
                movie.h = static_cast<float>(geometry.height) * vs;
                movie.u0 = 0.0f;
                movie.v0 = 0.0f;
                movie.u1 = 1.0f;
                movie.v1 = 1.0f;
                movie.r = 1.0f;
                movie.g = 1.0f;
                movie.b = 1.0f;
                movie.a = 1.0f;
                movie.blend = D3D11Renderer::BlendMode::Alpha;
                movie.layer = 700;
                movie.order = 0;
                ctx.renderer->SubmitSprite(movie);
            }
        }
        return;
    }

    const auto titleResourceState =
        PrSS0TitleTmdBackend::GetResourceState();
    const uint32_t titleResourceGeneration =
        titleResourceState.titleHudTimResourceGeneration801C5094;
    PrSS0TitlePacketRenderDirect::BuildResult801C689C preparedDecoded{};
    const PrSS0TitlePacketRenderDirect::BuildResult801C689C* visibleDecoded =
        nullptr;
    bool preparedFrameDecoded = false;
    const auto* titlePacketWork =
        PrSS0TitleTmdBackend::GetTitlePacketWork801C609C();
    if (titlePacketWork != nullptr &&
        titlePacketWork->framePrepared801C6410) {
        preparedDecoded = PrSS0TitlePacketRenderDirect::
            BuildCurrentMode25DrawCommands801C689C(*titlePacketWork);
        if (preparedDecoded.complete) {
            visibleDecoded = &preparedDecoded;
            preparedFrameDecoded = true;
        }
    } else {
        visibleDecoded = PrSS0TitlePacketRenderDirect::
            ResolveHostVisibleTitleFrameCache801C689C(
                s_ss0Direct.titleHostVisibleFrameCache,
                titleResourceGeneration);
    }

    // Keep the direct title path fail-closed, but expose the exact reason a
    // selector frame has no decoded OT mirror.  This is deliberately a
    // one-shot diagnostic: the primitive groups may be fully committed while
    // the present-model/cache handoff has already consumed the prepared lane.
    static bool s_loggedTitleVisibleFrameGap = false;
    if (visibleDecoded == nullptr && !s_loggedTitleVisibleFrameGap &&
        s_ss0Direct.titleLoopStateV8 >= 1u) {
        s_loggedTitleVisibleFrameGap = true;
        Log::Printf(
            "SS0 direct runtime: title visible frame gap state=%u prepared=%d framePrepared=%d renderActiveKnown=%d renderActive=%u cacheValid=%d cacheGeneration=%u resourceGeneration=%u",
            static_cast<unsigned>(s_ss0Direct.titleLoopStateV8),
            preparedFrameDecoded ? 1 : 0,
            titlePacketWork != nullptr &&
                    titlePacketWork->framePrepared801C6410
                ? 1
                : 0,
            titleResourceState.titleRenderActiveKnown801C9548 ? 1 : 0,
            static_cast<unsigned>(
                titleResourceState.titleRenderActive801C9548),
            s_ss0Direct.titleHostVisibleFrameCache.valid ? 1 : 0,
            static_cast<unsigned>(
                s_ss0Direct.titleHostVisibleFrameCache.resourceGeneration),
            static_cast<unsigned>(titleResourceGeneration));
    }

    PrSS0TitleDrawBackend::SubmitResult801C689C titlePackets{};
    if (visibleDecoded != nullptr) {
        titlePackets =
            PrSS0TitleDrawBackend::SubmitMode25DrawCommands801C689C(
                ctx, *visibleDecoded, vx, vy, vs, 400);
    }
    const auto* paGroup =
        PrSS0TitleTmdBackend::GetPaMode25PrimitiveGroup8004274C();
    const auto* paKageGroup =
        PrSS0TitleTmdBackend::GetPaKageMode25PrimitiveGroup8004274C();
    const auto* loGroup =
        PrSS0TitleTmdBackend::GetLoMode25PrimitiveGroup8004274C();
    const auto* hpGroup =
        PrSS0TitleTmdBackend::GetHpMode25PrimitiveGroup8004274C();
    const uint32_t titleGroupSignature =
        (loGroup ? loGroup->committedCount : 0u) |
        ((hpGroup ? hpGroup->committedCount : 0u) << 16);
    static uint32_t s_titleGroupSignature = UINT32_MAX;
    if (titleGroupSignature != s_titleGroupSignature) {
        s_titleGroupSignature = titleGroupSignature;
        Log::Printf(
            "SS0 direct runtime: title primitive groups state pa=%u/%u/%u/%u paKage=%u/%u/%u/%u lo=%u/%u/%u/%u hp=%u/%u/%u/%u",
            paGroup ? paGroup->requestedCount : 0u,
            paGroup ? paGroup->preparedCount : 0u,
            paGroup ? paGroup->committedCount : 0u,
            paGroup && paGroup->complete ? 1u : 0u,
            paKageGroup ? paKageGroup->requestedCount : 0u,
            paKageGroup ? paKageGroup->preparedCount : 0u,
            paKageGroup ? paKageGroup->committedCount : 0u,
            paKageGroup && paKageGroup->complete ? 1u : 0u,
            loGroup ? loGroup->requestedCount : 0u,
            loGroup ? loGroup->preparedCount : 0u,
            loGroup ? loGroup->committedCount : 0u,
            loGroup && loGroup->complete ? 1u : 0u,
            hpGroup ? hpGroup->requestedCount : 0u,
            hpGroup ? hpGroup->preparedCount : 0u,
            hpGroup ? hpGroup->committedCount : 0u,
            hpGroup && hpGroup->complete ? 1u : 0u);
    }
    static bool s_titlePacketSubmitLogged = false;
    if (!s_titlePacketSubmitLogged && titlePackets.submittedTriangles > 0u) {
        s_titlePacketSubmitLogged = true;
        Log::Printf(
            "SS0 direct runtime: title OT mirror submitted decoded=%u drawn=%u missingTexture=%u d3dAtlas=%d readyTpages=%u failedUploads=%u groups(pa=%u/%u/%u/%u,paKage=%u/%u/%u/%u,lo=%u/%u/%u/%u,hp=%u/%u/%u/%u)",
            titlePackets.decodedTriangles,
            titlePackets.submittedTriangles,
            titlePackets.missingTextures,
            titlePackets.directScene0AtlasUploadCommitted ? 1 : 0,
            titlePackets.readyTpages,
            titlePackets.failedTpageUploads,
            paGroup ? paGroup->requestedCount : 0u,
            paGroup ? paGroup->preparedCount : 0u,
            paGroup ? paGroup->committedCount : 0u,
            paGroup && paGroup->complete ? 1u : 0u,
            paKageGroup ? paKageGroup->requestedCount : 0u,
            paKageGroup ? paKageGroup->preparedCount : 0u,
            paKageGroup ? paKageGroup->committedCount : 0u,
            paKageGroup && paKageGroup->complete ? 1u : 0u,
            loGroup ? loGroup->requestedCount : 0u,
            loGroup ? loGroup->preparedCount : 0u,
            loGroup ? loGroup->committedCount : 0u,
            loGroup && loGroup->complete ? 1u : 0u,
            hpGroup ? hpGroup->requestedCount : 0u,
            hpGroup ? hpGroup->preparedCount : 0u,
            hpGroup ? hpGroup->committedCount : 0u,
            hpGroup && hpGroup->complete ? 1u : 0u);
    }

    const char* startTim =
        s_ss0Direct.titleCursor == 0 ? "TSTRT_11" : "TSTRT_01";
    const char* menuTim =
        s_ss0Direct.titleCursor == 1 ? "TMENU_11" : "TMENU_01";
    const bool selectorPhase = s_ss0Direct.titleLoopStateV8 >= 2u;
    const bool selectorTimsAvailable =
        selectorPhase && SS0DirectTitleTimAvailable(ctx, startTim) &&
        SS0DirectTitleTimAvailable(ctx, menuTim);
    const bool selectorPromptDrawn =
        selectorPhase && SS0DirectRenderTitleSelectorPrompt8001E3E4(
                             ctx, vx, vy, vs);
    const bool startTimDrawn =
        selectorPromptDrawn ||
        (selectorTimsAvailable && SS0DirectSubmitTitleTim(
            ctx, startTim, vx, vy, vs, 24.0f, 139.0f, 500));
    const bool menuTimDrawn =
        selectorPromptDrawn ||
        (selectorTimsAvailable && SS0DirectSubmitTitleTim(
            ctx, menuTim, vx, vy, vs, 231.0f, 139.0f, 500));
    const bool tmTimDrawn =
        selectorPromptDrawn ||
        (selectorPhase && SS0DirectSubmitTitleTim(
            ctx, "TM", vx, vy, vs, 278.0f, 78.0f, 500));

    static int s_loggedTitleTimIngress = -1;
    const int titleTimIngress = startTimDrawn && menuTimDrawn ? 1 : 0;
    if (s_loggedTitleTimIngress != titleTimIngress) {
        s_loggedTitleTimIngress = titleTimIngress;
        Log::Printf(
            "SS0 direct runtime: title COMPO00 TIM ingress start=%d menu=%d tm=%d prompt8001E3E4=%d",
            startTimDrawn ? 1 : 0,
            menuTimDrawn ? 1 : 0,
            tmTimDrawn ? 1 : 0,
            selectorPromptDrawn ? 1 : 0);
    }

    if (!applyPresentModel) {
        return;
    }

    const bool awaitingEarlyInputPixels =
        s_ss0Direct.titleEarlyInputShortcutPhase ==
        SS0DirectTitleEarlyInputShortcutPhase::AwaitingPixelSubmit;
    const bool titlePacketSubmissionComplete =
        preparedFrameDecoded && titlePackets.decodeComplete &&
        titlePackets.directScene0AtlasUploadCommitted &&
        titlePackets.texturePreflightComplete &&
        titlePackets.submittedTriangles == titlePackets.decodedTriangles &&
        titlePackets.missingTextures == 0u;
    const bool titlePacketPixelsSubmitted =
        titlePacketSubmissionComplete &&
        titlePackets.decodedTriangles > 0u;
    const bool titlePacketFrameSubmitted =
        titlePacketSubmissionComplete &&
        titleResourceState.titleRenderActiveKnown801C9548 &&
        (titleResourceState.titleRenderActive801C9548 != 0u
             ? titlePackets.decodedTriangles > 0u
             : titlePackets.decodedTriangles == 0u &&
                   titlePackets.submittedTriangles == 0u);
    const bool exitWaitPresentAttempt =
        s_ss0Direct.phase == SS0DirectPhase::TitleExitWait &&
        s_ss0Direct.titleExitWaitPresentGate.presentPending &&
        !s_ss0Direct.titleExitWaitPresentGate.presentAcknowledged;
    const bool selectorConfirmationPresentAttempt =
        s_ss0Direct.phase == SS0DirectPhase::TitleExitWait &&
        s_ss0Direct.titleSelectorConfirmationPresentPending;
    if (!applyPresentModel ||
        (s_ss0Direct.phase == SS0DirectPhase::TitleExitWait &&
         !exitWaitPresentAttempt && !selectorConfirmationPresentAttempt)) {
        return;
    }
    if ((exitWaitPresentAttempt || selectorConfirmationPresentAttempt) &&
        (!selectorPromptDrawn || !titlePacketFrameSubmitted)) {
        return;
    }
    PrSS0TitleTmdBackend::TitlePresentModelResult801C689C presentModel{};
    if (!awaitingEarlyInputPixels || titlePacketPixelsSubmitted) {
        presentModel =
            PrSS0TitleTmdBackend::ApplyTitlePresentModel801C689C();
    }
    const auto hostPresentProjection =
        PrSS0TitlePacketRenderDirect::BuildHostPresentProjection801C689C(
            presentModel.modelApplied,
            presentModel.extraFlipGraphStateAdvanced80040370,
            presentModel.fullHeightWhiteFill8001B1B0Requested);
    const bool preparedFrameHostSubmitComplete =
        preparedFrameDecoded && titlePackets.decodeComplete &&
        titlePackets.directScene0AtlasUploadCommitted &&
        titlePackets.texturePreflightComplete &&
        titlePackets.submittedTriangles == titlePackets.decodedTriangles &&
        titlePackets.missingTextures == 0u;
    if (preparedFrameHostSubmitComplete && hostPresentProjection.accepted) {
        (void)PrSS0TitlePacketRenderDirect::
            CommitHostVisibleTitleFrameCache801C689C(
                s_ss0Direct.titleHostVisibleFrameCache,
                preparedDecoded,
                hostPresentProjection,
                titleResourceGeneration);
    }
    if (selectorConfirmationPresentAttempt && selectorPromptDrawn &&
        titlePacketFrameSubmitted && presentModel.modelApplied &&
        hostPresentProjection.accepted &&
        hostPresentProjection.titleCommandsVisibleOnCurrentHostFrame) {
        s_ss0Direct.titleSelectorConfirmationPresentPending = false;
        Log::Printf(
            "SS0 direct runtime: title selector confirmation 801C689C presented");
    }
    if (exitWaitPresentAttempt) {
        const auto acknowledged = PrSS0TitleHudEventsDirect::
            AcknowledgeTitleSelectorExitWaitPresent801C4D74(
                s_ss0Direct.titleExitWaitPresentGate,
                selectorPromptDrawn,
                titlePacketFrameSubmitted,
                presentModel.modelApplied && hostPresentProjection.accepted &&
                    hostPresentProjection.
                        titleCommandsVisibleOnCurrentHostFrame);
        if (acknowledged.accepted) {
            s_ss0Direct.titleExitWaitPresentGate = acknowledged.nextState;
        }
    }
    if (awaitingEarlyInputPixels) {
        if (titlePacketPixelsSubmitted && hostPresentProjection.accepted &&
            hostPresentProjection.titleCommandsVisibleOnCurrentHostFrame) {
            s_ss0Direct.titleEarlyInputPixelSubmitComplete = true;
            if (!s_ss0Direct.titleEarlyInputPixelSubmitStatusLogged) {
                Log::Printf(
                    "SS0 direct runtime: title early input D3D submit complete decoded=%u drawn=%u",
                    titlePackets.decodedTriangles,
                    titlePackets.submittedTriangles);
                s_ss0Direct.titleEarlyInputPixelSubmitStatusLogged = true;
            }
        } else if (!s_ss0Direct.titleEarlyInputPixelSubmitStatusLogged) {
            s_ss0Direct.titleEarlyInputPixelSubmitStatusLogged = true;
            Log::Printf(
                "SS0 direct runtime: title early input D3D submit blocked decode=%d failure=%u decoded=%u atlasAttempt=%d atlasComplete=%d directAtlas=%d readyTpages=%u failedUploads=%u preflight=%d drawn=%u missingTexture=%u present=%d",
                titlePackets.decodeComplete ? 1 : 0,
                static_cast<unsigned>(titlePackets.decodeFailure),
                titlePackets.decodedTriangles,
                titlePackets.textureUploadAttempted ? 1 : 0,
                titlePackets.textureUploadComplete ? 1 : 0,
                titlePackets.directScene0AtlasUploadCommitted ? 1 : 0,
                titlePackets.readyTpages,
                titlePackets.failedTpageUploads,
                titlePackets.texturePreflightComplete ? 1 : 0,
                titlePackets.submittedTriangles,
                titlePackets.missingTextures,
                presentModel.modelApplied ? 1 : 0);
        }
    }
    static bool s_titlePresentModelLogged = false;
    if (!s_titlePresentModelLogged && presentModel.modelApplied) {
        s_titlePresentModelLogged = true;
        Log::Printf(
            "SS0 direct runtime: title present model applied lane=%u next=%u titleWork=%08X mainWork=%08X drawEnv80040060=%d envSlot=%u envCmd=%08X title80040CA4=%d main8001E3B0=%d titleCall=%u mainCall=%u",
            static_cast<unsigned>(presentModel.cachedDrawLane8006EDA8),
            static_cast<unsigned>(presentModel.nextDrawLane80096590),
            presentModel.titleWorkAddress801C9574,
            presentModel.mainPageWorkAddress80087288,
            presentModel.drawEnv80040060.known ? 1 : 0,
            static_cast<unsigned>(
                presentModel.drawEnv80040060.graphSlot80096590),
            presentModel.drawEnv80040060.commandAddress8008A730,
            presentModel.titleWorkSubmit80040CA4Executed ? 1 : 0,
            presentModel.mainPageWorkSubmit8001E3B0Executed ? 1 : 0,
            presentModel.titleWorkSubmitCallCount80040CA4,
            presentModel.mainPageWorkSubmitCallCount8001E3B0);
    }

}

struct SS0DirectRawSpriteSource8001B590 {
    bool known = false;
    uint32_t psxAddress = 0u;
    bool textureCoordinatesResolved = false;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint32_t callOrder = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
    uint16_t tpage = 0;
    uint8_t u = 0;
    uint8_t v = 0;
};

static uint16_t SS0DirectRawSpriteTpage8001B25C(
    const SS0DirectRawSpriteSource8001B590& sprite) {
    const uint16_t uvWord = static_cast<uint16_t>(4u * sprite.texX);
    const uint16_t helperA3 = static_cast<uint16_t>(
        (uvWord & 0xFF00u) >> 2u);
    const uint16_t helperA4 = static_cast<uint16_t>(sprite.texY & 0xFF00u);
    return static_cast<uint16_t>(
        0x20u |
        ((helperA4 & 0x0100u) >> 4u) |
        ((helperA3 & 0x03FFu) >> 6u) |
        (4u * (helperA4 & 0x0200u)));
}

static uint16_t SS0DirectRawSpriteClut8001B25C(
    const SS0DirectRawSpriteSource8001B590& sprite) {
    return static_cast<uint16_t>(
        ((static_cast<uint32_t>(sprite.clutY) & 0x01FFu) << 6u) |
        ((static_cast<uint32_t>(sprite.clutX) >> 4u) & 0x3Fu));
}

static int SS0DirectRawSpriteLayer8003FA20(uint16_t priority) {
    switch (priority) {
    case 0u:
        return 784;
    case 1u:
        return 782;
    case 2u:
        return 781;
    case 3u:
        return 780;
    default:
        return 779;
    }
}

// The original menu packet table contains a second set of overlapping W
// textures (MAIN_*W*/EXIT_*W*).  They are the native PSX plate outline and
// selection-halo images.  Their black interior is only correct behind the
// colored B/E/G/I/S payloads; submitting W on top exposes the destroyed
// black/magenta shell that appeared in the earlier host projection.
static bool SS0DirectMainDirectoryWTexture80021E60(uint32_t psxAddress) {
    switch (psxAddress) {
    case 0x80051010u:
    case 0x80051020u:
    case 0x80051150u:
    case 0x80051160u:
    case 0x80051290u:
    case 0x800512A0u:
    case 0x800514F0u:
    case 0x80051500u:
    case 0x80051630u:
    case 0x80051640u:
    case 0x80051770u:
    case 0x80051780u:
    case 0x800518B0u:
    case 0x800518C0u:
    case 0x80050AC0u:
    case 0x80050AD0u:
        return true;
    default:
        return false;
    }
}

// Main-directory TIMs are deliberately placed on overlapping PSX VRAM
// rectangles.  The original loader draws each TIM from its own upload before
// the next one can overwrite that rectangle; a single final atlas page loses
// that per-sprite upload history.  Resolve the exact TIM by its descriptor and
// materialize a small standalone host texture for the menu sprite.  This keeps
// the renderer's geometry/ordering path intact while preserving native
// transparency and palette selection.
static ID3D11ShaderResourceView*
SS0DirectResolveNativeMainMenuTim80021E60(
    PrGameContext& ctx,
    const SS0DirectRawSpriteSource8001B590& source) {
    if (!ctx.renderer || source.psxAddress == 0u ||
        source.width == 0u || source.height == 0u) {
        return nullptr;
    }

    // 8001C604 stage-number slices keep the 80051BF0 template's texX while
    // moving the actual glyph with a byte U offset.  Recover that upload's
    // halfword origin from the resolved tpage/U pair so STAG_2M..STAG_6M are
    // selected individually; looking up the unshifted 606 origin would miss
    // every slice after the first and fall back to the last atlas overwrite.
    uint16_t lookupOrgX = source.texX;
    uint16_t lookupOrgY = source.texY;
    if (source.textureCoordinatesResolved) {
        const int tpageBaseHw = static_cast<int>(source.tpage & 0x000Fu) * 64;
        const int tpageBaseY = ((source.tpage >> 4u) & 1u) != 0u ? 256 : 0;
        const int colorMode = static_cast<int>((source.tpage >> 7u) & 0x03u);
        const int pixelsPerWord = colorMode == 0 ? 4
            : (colorMode == 1 ? 2 : 1);
        const int localU = static_cast<int>(source.u);
        if (pixelsPerWord > 0 && (localU % pixelsPerWord) == 0) {
            const int resolvedOrgX =
                tpageBaseHw + localU / pixelsPerWord;
            if (resolvedOrgX >= 0 && resolvedOrgX < 1024) {
                lookupOrgX = static_cast<uint16_t>(resolvedOrgX);
                lookupOrgY = static_cast<uint16_t>(
                    tpageBaseY + static_cast<int>(source.v));
            }
        }
    }
    return PrSS0TitleTmdBackend::ResolveTitleStandaloneTimSRV801C689C(
        ctx.renderer, lookupOrgX, lookupOrgY, source.width, source.height,
        source.clutX, source.clutY);
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromBackdrop8001D74C(
    const PrSS0EventBackdropRenderDirect::
        EventBackdropSpriteCommand8001D74C& command) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = command.known && command.sprite.known;
    out.psxAddress = command.sprite.psxAddress;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.callOrder = command.callOrder;
    out.attr = command.sprite.attr;
    out.texX = command.sprite.texX;
    out.texY = command.sprite.texY;
    out.width = command.sprite.width;
    out.height = command.sprite.height;
    out.clutX = command.sprite.clutX;
    out.clutY = command.sprite.clutY;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromHiScore80021594(
    const PrSS0HiScoreRenderDirect::HiScoreSpriteCommand80021594& command) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = command.known && command.rawTexture;
    out.psxAddress = command.templateAddress;
    out.textureCoordinatesResolved = command.textureCoordinatesResolved;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.callOrder = command.callOrder;
    out.attr = command.descriptor.attr;
    out.texX = command.descriptor.texX;
    out.texY = command.descriptor.texY;
    out.width = command.descriptor.width;
    out.height = command.descriptor.height;
    out.clutX = command.descriptor.clutX;
    out.clutY = command.descriptor.clutY;
    out.tpage = command.tpage;
    out.u = command.u;
    out.v = command.v;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromMainDirectory80021E60(
    const PrSS0DirectoryPagesRenderDirect::
        MainDirectorySpriteCommand80021E60& command) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = command.known && command.sprite.known;
    out.psxAddress = command.sprite.psxAddress;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.callOrder = command.callOrder;
    out.attr = command.sprite.attr;
    out.texX = command.sprite.texX;
    out.texY = command.sprite.texY;
    out.width = command.sprite.width;
    out.height = command.sprite.height;
    out.clutX = command.sprite.clutX;
    out.clutY = command.sprite.clutY;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromOptions80021910(
    const PrSS0DirectoryPagesRenderDirect::
        OptionsSpriteCommand80021910& command) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = command.known && command.sprite.known;
    out.psxAddress = command.sprite.psxAddress;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.callOrder = command.callOrder;
    out.attr = command.sprite.attr;
    out.texX = command.sprite.texX;
    out.texY = command.sprite.texY;
    out.width = command.sprite.width;
    out.height = command.sprite.height;
    out.clutX = command.sprite.clutX;
    out.clutY = command.sprite.clutY;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromStageSelect80020568(
    const PrSS0DirectoryPagesRenderDirect::
        StageSelectSpriteCommand80020568& command) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = command.known && command.sprite.known;
    out.psxAddress = command.sprite.psxAddress;
    out.textureCoordinatesResolved = command.textureCoordinatesResolved;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.callOrder = command.callOrder;
    out.attr = command.sprite.attr;
    out.texX = command.sprite.texX;
    out.texY = command.sprite.texY;
    out.width = command.sprite.width;
    out.height = command.sprite.height;
    out.clutX = command.sprite.clutX;
    out.clutY = command.sprite.clutY;
    out.tpage = command.tpage;
    out.u = command.u;
    out.v = command.v;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromCardGrid80020F94(
    const PrSS0DirectoryPagesRenderDirect::CardGridDrawList80020F94& list,
    const PrSS0DirectoryPagesRenderDirect::CardGridSpriteCommand80020F94&
        command,
    bool pageRouteKnown) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = pageRouteKnown && list.sourceKnown && list.accepted &&
        list.rawTextureOnly && command.known && command.sprite.known;
    out.psxAddress = command.sprite.psxAddress;
    out.textureCoordinatesResolved = command.textureCoordinatesResolved;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.callOrder = command.callOrder;
    out.attr = command.sprite.attr;
    out.texX = command.sprite.texX;
    out.texY = command.sprite.texY;
    out.width = command.sprite.width;
    out.height = command.sprite.height;
    out.clutX = command.sprite.clutX;
    out.clutY = command.sprite.clutY;
    out.tpage = command.tpage;
    out.u = command.u;
    out.v = command.v;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromPracticeGuide80023518(
    const PrSS0DirectoryPagesRenderDirect::PracticeGuideDrawList80023518&
        list,
    const PrSS0DirectoryPagesRenderDirect::PracticeGuideSprite80023518&
        sprite) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = list.sourceKnown && list.accepted && list.complete &&
        list.runtimeSubmitAllowed && sprite.staticDescriptorKnown;
    out.psxAddress = sprite.templateAddress;
    out.x = sprite.x;
    out.y = sprite.y;
    out.priority = sprite.priority;
    out.callOrder = sprite.callOrder;
    out.attr = sprite.attr;
    out.texX = sprite.texX;
    out.texY = sprite.texY;
    out.width = sprite.width;
    out.height = sprite.height;
    out.clutX = sprite.clutX;
    out.clutY = sprite.clutY;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromPracticeSlice8001C604(
    const PrSS0DirectoryPagesRenderDirect::PracticeSliceDrawList8001C604&
        list,
    const PrSS0DirectoryPagesRenderDirect::PracticeSliceSprite8001C604&
        sprite) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = list.sourceKnown && list.accepted && list.complete &&
        list.runtimeSubmitAllowed && list.staticWriterSemanticsKnown &&
        list.localRgbKnown && !list.packetOtMutationPublished &&
        !list.replaySourceUsed && sprite.known &&
        sprite.staticDescriptorKnown && sprite.textureCoordinatesResolved &&
        sprite.localRgbKnown && sprite.localR == 16u &&
        sprite.localG == 0u && sprite.localB == 0u;
    out.textureCoordinatesResolved = sprite.textureCoordinatesResolved;
    out.x = sprite.x;
    out.y = sprite.y;
    out.priority = sprite.priority;
    out.callOrder = sprite.callOrder;
    out.attr = sprite.attr;
    out.texX = sprite.texX;
    out.texY = sprite.texY;
    out.width = sprite.width;
    out.height = sprite.height;
    out.clutX = sprite.clutX;
    out.clutY = sprite.clutY;
    out.tpage = sprite.tpage;
    out.u = sprite.u;
    out.v = sprite.v;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromPracticePostSliceTail80023618(
    const PrSS0DirectoryPagesRenderDirect::
        PracticePostSliceTailDrawList80023618& list,
    const PrSS0DirectoryPagesRenderDirect::
        PracticePostSliceTailSprite80023618& command) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = list.sourceKnown && list.accepted && list.complete &&
        ((list.runtimeSubmitAllowed && list.precedingSliceDrawPublished) ||
         (list.runtimeSubmitAllowed && list.alwaysVisibleFixedTail)) &&
        list.staticWriterSemanticsKnown && list.staticDescriptorSourcesKnown &&
        !list.packetOtMutationPublished &&
        !list.replaySourceUsed && command.known &&
        (command.psxFunction == PrSS0DirectoryPagesRenderDirect::kFn8001C550 ||
         command.psxFunction == PrSS0DirectoryPagesRenderDirect::kFn8001C5A8) &&
        command.sprite.known;
    out.psxAddress = command.sprite.psxAddress;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.callOrder = command.callOrder;
    out.attr = command.sprite.attr;
    out.texX = command.sprite.texX;
    out.texY = command.sprite.texY;
    out.width = command.sprite.width;
    out.height = command.sprite.height;
    out.clutX = command.sprite.clutX;
    out.clutY = command.sprite.clutY;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromPracticeLeadingOverlay80023618(
    const PrSS0DirectoryPagesRenderDirect::
        PracticeLeadingOverlayDrawList80023618& list,
    const PrSS0DirectoryPagesRenderDirect::
        PracticeLeadingOverlaySprite80023618& command) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = list.sourceKnown && list.accepted && list.complete &&
        list.runtimeSubmitAllowed && list.staticWriterSemanticsKnown &&
        list.staticDescriptorSourcesKnown &&
        !list.packetOtMutationPublished && !list.replaySourceUsed &&
        command.known &&
        (command.psxFunction == PrSS0DirectoryPagesRenderDirect::kFn8001C550 ||
         command.psxFunction == PrSS0DirectoryPagesRenderDirect::kFn8001C5A8) &&
        command.sprite.known;
    out.psxAddress = command.sprite.psxAddress;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.callOrder = command.callOrder;
    out.attr = command.sprite.attr;
    out.texX = command.sprite.texX;
    out.texY = command.sprite.texY;
    out.width = command.sprite.width;
    out.height = command.sprite.height;
    out.clutX = command.sprite.clutX;
    out.clutY = command.sprite.clutY;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromPracticePortrait80024600(
    const PrSS0DirectoryPagesRenderDirect::
        PracticePortraitSubmit80024600& submit,
    uint32_t callOrder) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = submit.sourceKnown && submit.accepted && submit.complete &&
        submit.drawRequested && submit.runtimeSubmitAllowed &&
        submit.staticDescriptorKnown;
    out.psxAddress = submit.templateAddress;
    out.x = submit.x;
    out.y = submit.y;
    out.priority = submit.priority;
    out.callOrder = callOrder;
    out.attr = submit.attr;
    out.texX = submit.texX;
    out.texY = submit.texY;
    out.width = submit.width;
    out.height = submit.height;
    out.clutX = submit.clutX;
    out.clutY = submit.clutY;
    return out;
}

static SS0DirectRawSpriteSource8001B590
SS0DirectRawSpriteFromCardIoBanner80020A3C(
    const PrSS0CardIoBannerRenderDirect::
        CardIoBannerSpriteCommand80020A3C& command) {
    SS0DirectRawSpriteSource8001B590 out{};
    out.known = command.known;
    out.textureCoordinatesResolved = true;
    out.x = command.x;
    out.y = command.y;
    out.priority = command.priority;
    out.attr = command.attr;
    out.width = command.width;
    out.height = command.height;
    out.clutX = command.clutX;
    out.clutY = command.clutY;
    out.tpage = command.tpage;
    out.u = command.u;
    out.v = command.v;
    return out;
}

static bool SS0DirectResolveRawSprite8001B590(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const SS0DirectRawSpriteSource8001B590& source,
    D3D11Renderer::SpriteCmd& command) {
    if (!source.known || source.width == 0u || source.height == 0u ||
        (source.attr & 0x40u) == 0u) {
        return false;
    }

    const uint16_t tpage = source.textureCoordinatesResolved
        ? source.tpage
        : SS0DirectRawSpriteTpage8001B25C(source);
    const uint16_t clut = SS0DirectRawSpriteClut8001B25C(source);
    const auto texture =
        PrSS0TitleTmdBackend::ResolveTitleTpageSRVExact801C689C(
            ctx.renderer, tpage, clut);
    const bool semiTransparent =
        ((source.attr >> 29u) & 0x02u) != 0u;
    const uint8_t abr = static_cast<uint8_t>(
        (source.attr >> 28u) & 0x03u);
    if (!texture.HasNativeCpuAtlasAuthority() ||
        !texture.tpageClutResolvable || !texture.clutStpKnown ||
        texture.srv == nullptr) {
        return false;
    }

    command.texture = texture.srv;
    command.blend = D3D11Renderer::BlendMode::Alpha;
    if (semiTransparent && texture.clutHasStpBits) {
        if (abr == 0u && texture.psxAbr0StpSrv != nullptr) {
            command.texture = texture.psxAbr0StpSrv;
        } else if (abr == 1u && texture.psxAbr1StpSrv != nullptr) {
            command.texture = texture.psxAbr1StpSrv;
            command.blend = D3D11Renderer::BlendMode::PsxAbr1Stp;
        } else {
            return false;
        }
    }

    // The original PSX scene submits each TIM upload while it is still the
    // current VRAM contents.  A final atlas page is only a fallback because
    // later uploads can overwrite an earlier page rectangle.  Use the exact
    // accepted startup TIM descriptor whenever one is available, for every
    // direct page (main/options/stage/card/practice/HI-SCORE), not just the
    // Event3 main-directory path.  Keep the PSX blend selection above: the
    // standalone view carries the same palette row while the command still
    // applies the descriptor's ABR/STP mode.
    if (const auto standalone =
            SS0DirectResolveNativeMainMenuTim80021E60(ctx, source);
        standalone != nullptr) {
        command.texture = standalone;
    }
    command.x = vx + static_cast<float>(source.x) * vs;
    command.y = vy + static_cast<float>(source.y) * vs;
    command.w = static_cast<float>(source.width) * vs;
    command.h = static_cast<float>(source.height) * vs;
    // Directory sprite descriptors carry absolute PSX VRAM coordinates.  The
    // atlas is one local 256x256 page, so remove the page's halfword/Y origin
    // before converting to texture UVs.  In particular, the menu text lives
    // at global Y=267+ on tpage 0x35 (baseY=256); sampling it as v=267 wraps
    // into the wrong page and produces the old shell-like black blocks.
    const int tpageBaseHw = static_cast<int>(tpage & 0x000Fu) * 64;
    const int tpageBaseY = ((tpage >> 4u) & 1u) != 0u ? 256 : 0;
    const int tpageColorMode = static_cast<int>((tpage >> 7u) & 0x03u);
    const int pixelsPerWord = tpageColorMode == 0
        ? 4
        : (tpageColorMode == 1 ? 2 : 1);
    const float u = static_cast<float>(source.textureCoordinatesResolved
        ? source.u
        : (static_cast<int>(source.texX) - tpageBaseHw) * pixelsPerWord);
    const float v = static_cast<float>(source.textureCoordinatesResolved
        ? source.v
        : (static_cast<int>(source.texY) - tpageBaseY));
    command.u0 = (u + 0.5f) / 256.0f;
    command.v0 = (v + 0.5f) / 256.0f;
    command.u1 = source.width <= 1u
        ? command.u0
        : (u + static_cast<float>(source.width) - 0.5f) / 256.0f;
    command.v1 = source.height <= 1u
        ? command.v0
        : (v + static_cast<float>(source.height) - 0.5f) / 256.0f;
    if (command.texture != texture.srv &&
        command.texture != texture.psxAbr0StpSrv &&
        command.texture != texture.psxAbr1StpSrv) {
        command.u0 = 0.0f;
        command.v0 = 0.0f;
        command.u1 = 1.0f;
        command.v1 = 1.0f;
    }
    command.r = 1.0f;
    command.g = 1.0f;
    command.b = 1.0f;
    command.a = 1.0f;
    command.layer = SS0DirectRawSpriteLayer8003FA20(source.priority);
    command.order = 0u;
    return true;
}

static bool SS0DirectSubmitEventBackdrop8001D74C(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs) {
    namespace Backdrop = PrSS0EventBackdropRenderDirect;
    const auto backdrop =
        Backdrop::BuildEventBackdropDrawList8001D74C(3u);
    if (!backdrop.sourceKnown || !backdrop.accepted || !backdrop.complete ||
        !backdrop.rawTextureOnly || backdrop.truncated ||
        backdrop.count != Backdrop::kEventBackdropSpriteCapacity8001D74C ||
        !ctx.renderer) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               Backdrop::kEventBackdropSpriteCapacity8001D74C>
        resolved{};
    for (std::size_t index = 0u; index < backdrop.count; ++index) {
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                SS0DirectRawSpriteFromBackdrop8001D74C(
                    backdrop.commands[index]),
                resolved[index])) {
            return false;
        }
    }
    for (std::size_t index = backdrop.count; index > 0u; --index) {
        ctx.renderer->SubmitSprite(resolved[index - 1u]);
    }
    return true;
}

static bool SS0DirectSubmitPracticeLeadingOverlay80023618(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs) {
    auto& list = s_ss0Direct.practiceLeadingOverlayDrawList;
    list.rendererAuthorityPublished = false;
    list.runtimeDrawPublished = false;
    if (!list.sourceKnown || !list.accepted || !list.complete ||
        !list.runtimeSubmitAllowed || list.truncated ||
        list.count > PrSS0DirectoryPagesRenderDirect::
                         kPracticeLeadingOverlaySpriteCapacity80023618) {
        return false;
    }
    if (list.count == 0u) {
        list.rendererAuthorityPublished = true;
        list.runtimeDrawPublished = true;
        return true;
    }
    if (!ctx.renderer) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               PrSS0DirectoryPagesRenderDirect::
                   kPracticeLeadingOverlaySpriteCapacity80023618>
        resolved{};
    for (std::size_t index = 0u; index < list.count; ++index) {
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                SS0DirectRawSpriteFromPracticeLeadingOverlay80023618(
                    list, list.sprites[index]),
                resolved[index])) {
            return false;
        }
        // PSX 8003FA20 inserts each same-priority command at the OT bucket
        // head.  The leading title/subtitle calls therefore render on top of
        // the later fixed-frame tail even though the host submits this list
        // before that tail.
        resolved[index].order = kPracticeOtOrderLeading +
            static_cast<uint64_t>(list.count - index);
    }

    for (std::size_t index = list.count; index > 0u; --index) {
        ctx.renderer->SubmitSprite(resolved[index - 1u]);
    }
    list.rendererAuthorityPublished = true;
    list.runtimeDrawPublished = true;
    return true;
}

static bool SS0DirectSubmitPracticeGuide80023518(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs) {
    auto& list = s_ss0Direct.practiceGuideDrawList;
    list.rendererAuthorityPublished = false;
    list.runtimeDrawPublished = false;
    if (!list.sourceKnown || !list.accepted || !list.complete ||
        !list.runtimeSubmitAllowed || list.truncated ||
        list.count != PrSS0DirectoryPagesRenderDirect::
                          kPracticeGuideSpriteCount80023518) {
        return false;
    }
    if (!ctx.renderer) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               PrSS0DirectoryPagesRenderDirect::
                   kPracticeGuideSpriteCount80023518>
        resolved{};
    for (std::size_t index = 0u; index < list.count; ++index) {
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                SS0DirectRawSpriteFromPracticeGuide80023518(
                    list, list.sprites[index]),
                resolved[index])) {
            return false;
        }
        resolved[index].order = kPracticeOtOrderGuide +
            static_cast<uint64_t>(list.count - index);
    }

    for (std::size_t index = list.count; index > 0u; --index) {
        ctx.renderer->SubmitSprite(resolved[index - 1u]);
    }
    list.rendererAuthorityPublished = true;
    list.runtimeDrawPublished = true;
    return true;
}

struct SS0DirectResolvedPracticeIcon80024418 {
    bool transformPath = false;
    D3D11Renderer::SpriteCmd sprite{};
    D3D11Renderer::TexturedTriCmd triangles{};
    PrSS0DirectoryPagesRenderDirect::PracticeIconSubmit80024418* request =
        nullptr;
};

static bool SS0DirectResolvePracticeIcon80024418(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    const PrSS0DirectoryPagesRenderDirect::
        PracticeIconDirectRenderPayload80024418& payload,
    SS0DirectResolvedPracticeIcon80024418& resolved) {
    static bool failureLogged = false;
    const auto reject = [&](const char* reason) {
        if (!failureLogged) {
            failureLogged = true;
            const auto* graph =
                PrSS0TitleTmdBackend::GetTitleGraphState();
            Log::Printf(
                "SS0 direct runtime: Practice 80024418 icon resolve rejected reason=%s payloadSource=%d renderPayload=%d fast=%d transform=%d tpage=%04X clut=%04X rgb=%u/%u/%u graph=%d mode=%u size=%d/%d drawEnv=%d offset=%d/%d center=%d/%d gte=%d/%u/%d,%d/%d,%d",
                reason ? reason : "unknown",
                payload.sourceKnown ? 1 : 0,
                payload.renderPayloadKnown ? 1 : 0,
                payload.fastPacketPath ? 1 : 0,
                payload.transformPacketPath ? 1 : 0,
                static_cast<unsigned>(payload.tpage),
                static_cast<unsigned>(payload.clut),
                static_cast<unsigned>(payload.r),
                static_cast<unsigned>(payload.g),
                static_cast<unsigned>(payload.b),
                graph != nullptr ? 1 : 0,
                graph != nullptr ? static_cast<unsigned>(graph->word_800965A0) : 0u,
                graph != nullptr ? static_cast<int>(graph->word_800928D4) : 0,
                graph != nullptr ? static_cast<int>(graph->word_800928D6) : 0,
                graph != nullptr && graph->drawOffset.setDrawEnvCalled ? 1 : 0,
                graph != nullptr ? static_cast<int>(graph->drawOffset.word_800917AA) : 0,
                graph != nullptr ? static_cast<int>(graph->drawOffset.word_800917AC) : 0,
                graph != nullptr ? static_cast<int>(graph->drawOffset.word_80091738) : 0,
                graph != nullptr ? static_cast<int>(graph->drawOffset.word_8009173A) : 0,
                graph != nullptr && graph->gte.geomScreenKnown ? 1 : 0,
                graph != nullptr ? static_cast<unsigned>(graph->gte.geomScreen) : 0u,
                graph != nullptr && graph->gte.geomOffsetKnown ? 1 : 0,
                graph != nullptr ? static_cast<int>(graph->gte.geomOffsetX) : 0,
                graph != nullptr ? static_cast<int>(graph->gte.geomOffsetY) : 0,
                graph != nullptr && graph->gte.depthCueKnown ? 1 : 0);
        }
        return false;
    };
    if (!ctx.renderer || !payload.renderPayloadKnown ||
        payload.packetOtMutationPublished || payload.replaySourceUsed) {
        return reject("payload-or-renderer");
    }
    const auto texture =
        PrSS0TitleTmdBackend::ResolveTitleTpageSRVExact801C689C(
            ctx.renderer, payload.tpage, payload.clut);
    if (!texture.HasNativeCpuAtlasAuthority() ||
        !texture.tpageClutResolvable || !texture.clutStpKnown ||
        texture.srv == nullptr) {
        return reject("texture-resolve");
    }

    ID3D11ShaderResourceView* srv = texture.srv;
    D3D11Renderer::BlendMode blend = D3D11Renderer::BlendMode::Alpha;
    const bool semiTransparent =
        ((payload.attr >> 29u) & 0x02u) != 0u;
    const uint8_t abr = static_cast<uint8_t>(
        (payload.attr >> 28u) & 0x03u);
    if (semiTransparent && texture.clutHasStpBits) {
        if (abr == 0u && texture.psxAbr0StpSrv != nullptr) {
            srv = texture.psxAbr0StpSrv;
        } else if (abr == 1u && texture.psxAbr1StpSrv != nullptr) {
            srv = texture.psxAbr1StpSrv;
            blend = D3D11Renderer::BlendMode::PsxAbr1Stp;
        } else {
            return reject("stp-blend");
        }
    }

    const auto color128 = [](uint8_t component) {
        return std::clamp(
            static_cast<float>(component) / 128.0f, 0.0f, 1.0f);
    };
    const float r = color128(payload.r);
    const float g = color128(payload.g);
    const float b = color128(payload.b);
    const int layer = SS0DirectRawSpriteLayer8003FA20(payload.priority);
    if (payload.fastPacketPath && !payload.transformPacketPath) {
        auto& command = resolved.sprite;
        command.texture = srv;
        command.blend = blend;
        const float drawOffsetX =
            static_cast<float>(payload.drawOffsetX) - 160.0f;
        const float drawOffsetY =
            static_cast<float>(payload.drawOffsetY) - 120.0f;
        command.x = vx +
            (static_cast<float>(payload.x[0]) - drawOffsetX) * vs;
        command.y = vy +
            (static_cast<float>(payload.y[0]) - drawOffsetY) * vs;
        command.w = static_cast<float>(payload.width) * vs;
        command.h = static_cast<float>(payload.height) * vs;
        const float u = static_cast<float>(payload.u[0]);
        const float v = static_cast<float>(payload.v[0]);
        command.u0 = (u + 0.5f) / 256.0f;
        command.v0 = (v + 0.5f) / 256.0f;
        command.u1 = (u + static_cast<float>(payload.width) - 0.5f) /
            256.0f;
        command.v1 = (v + static_cast<float>(payload.height) - 0.5f) /
            256.0f;
        command.r = r;
        command.g = g;
        command.b = b;
        command.a = 1.0f;
        command.layer = layer;
        command.order = 0u;
        resolved.transformPath = false;
        return true;
    }
    if (!payload.transformPacketPath || payload.fastPacketPath) {
        return reject("packet-path");
    }

    auto& command = resolved.triangles;
    command.texture = srv;
    command.vertexCount = 6;
    command.blend = blend;
    command.layer = layer;
    command.order = 0u;
    const float drawOffsetX =
        static_cast<float>(payload.drawOffsetX) - 160.0f;
    const float drawOffsetY =
        static_cast<float>(payload.drawOffsetY) - 120.0f;
    const auto vertex = [&](std::size_t index) {
        TexturedVertex out{};
        out.x = vx +
            (static_cast<float>(payload.x[index]) - drawOffsetX) * vs;
        out.y = vy +
            (static_cast<float>(payload.y[index]) - drawOffsetY) * vs;
        // 80024418 emits PSX byte UVs.  The D3D atlas stores one PSX texel
        // per host texel, so map the byte to the texel centre just like the
        // native sprite path.  Landing on the integer page boundary is
        // ambiguous for point sampling and, at high-resolution scales,
        // exposes the adjacent atlas texel as a one-pixel stitched border.
        PsxVramAtlas::UVtoNormalized(
            payload.u[index], payload.v[index], out.u, out.v);
        out.r = r;
        out.g = g;
        out.b = b;
        out.a = 1.0f;
        out.perspectiveW = 1.0f;
        return out;
    };
    // 8003F1B4 keeps the sprite's signed X scale in the transform matrix.
    // The PSX primitive is double-sided, while the host D3D rasterizer uses
    // its default back-face rule.  When rsin enters the negative half of the
    // 80023F20 flip sequence, the projected left/right vertices are swapped;
    // preserve the source texture orientation but reverse only the triangle
    // winding so that the mirrored note is not culled.  Positive/zero scale
    // retains the exact packet vertex order.
    const bool mirroredX = payload.x[1] < payload.x[0];
    command.vertices[0] = vertex(0u);
    command.vertices[1] = vertex(mirroredX ? 2u : 1u);
    command.vertices[2] = vertex(mirroredX ? 1u : 2u);
    command.vertices[3] = vertex(1u);
    command.vertices[4] = vertex(mirroredX ? 2u : 3u);
    command.vertices[5] = vertex(mirroredX ? 3u : 2u);
    resolved.transformPath = true;
    return true;
}

static bool SS0DirectSubmitPracticeIcons80024418(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs) {
    for (auto& candidate : s_ss0Direct.practiceIconCandidates) {
        candidate.rendererAuthorityPublished = false;
        candidate.runtimeDrawPublished = false;
    }
    if (!s_ss0Direct.practiceIconStreamKnown) {
        return false;
    }

    std::size_t drawCount = 0u;
    for (const auto& candidate : s_ss0Direct.practiceIconCandidates) {
        if (candidate.argumentsKnown && !candidate.accepted) {
            return false;
        }
        if (candidate.accepted) {
            ++drawCount;
        }
    }
    if (drawCount == 0u) {
        return true;
    }
    if (!ctx.renderer) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }
    const auto* graph = PrSS0TitleTmdBackend::GetTitleGraphState();
    if (graph == nullptr) {
        return false;
    }

    std::array<SS0DirectResolvedPracticeIcon80024418, 18> resolved{};
    std::size_t resolvedCount = 0u;
    for (auto& candidate : s_ss0Direct.practiceIconCandidates) {
        if (!candidate.accepted) {
            continue;
        }
        const auto payload = PrSS0DirectoryPagesRenderDirect::
            BuildPracticeIconDirectRenderPayload80024418(candidate, graph);
        auto& command = resolved[resolvedCount];
        if (!SS0DirectResolvePracticeIcon80024418(
                ctx, vx, vy, vs, payload, command)) {
            return false;
        }
        const uint64_t order = kPracticeOtOrderIcons +
            static_cast<uint64_t>(drawCount - resolvedCount);
        if (command.transformPath) {
            command.triangles.order = order;
        } else {
            command.sprite.order = order;
        }
        command.request = &candidate;
        ++resolvedCount;
    }
    if (resolvedCount != drawCount) {
        return false;
    }

    for (std::size_t index = resolvedCount; index > 0u; --index) {
        auto& command = resolved[index - 1u];
        if (command.transformPath) {
            ctx.renderer->SubmitTexturedTriangles(command.triangles);
        } else {
            ctx.renderer->SubmitSprite(command.sprite);
        }
        command.request->rendererAuthorityPublished = true;
        command.request->runtimeDrawPublished = true;
    }
    return true;
}

static bool SS0DirectSubmitPracticePortraits80024600(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs) {
    auto& teacher = s_ss0Direct.practicePortraitTeacher;
    auto& student = s_ss0Direct.practicePortraitStudent;
    teacher.rendererAuthorityPublished = false;
    teacher.runtimeDrawPublished = false;
    student.rendererAuthorityPublished = false;
    student.runtimeDrawPublished = false;

    std::array<PrSS0DirectoryPagesRenderDirect::
                   PracticePortraitSubmit80024600*, 2>
        requests{{&teacher, &student}};
    std::size_t drawCount = 0u;
    for (const auto* request : requests) {
        if (request->drawRequested) {
            ++drawCount;
        }
    }
    if (drawCount == 0u) {
        return true;
    }
    if (!ctx.renderer) {
        return false;
    }

    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd, 2> resolved{};
    std::array<PrSS0DirectoryPagesRenderDirect::
                   PracticePortraitSubmit80024600*, 2>
        resolvedRequests{};
    std::size_t resolvedCount = 0u;
    for (std::size_t index = 0u; index < requests.size(); ++index) {
        auto* request = requests[index];
        if (!request->drawRequested) {
            continue;
        }
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                SS0DirectRawSpriteFromPracticePortrait80024600(
                    *request, static_cast<uint32_t>(index)),
                resolved[resolvedCount])) {
            return false;
        }
        resolved[resolvedCount].order = kPracticeOtOrderPortraits +
            static_cast<uint64_t>(drawCount - resolvedCount);
        resolvedRequests[resolvedCount] = request;
        ++resolvedCount;
    }
    if (resolvedCount != drawCount) {
        return false;
    }

    for (std::size_t index = resolvedCount; index > 0u; --index) {
        ctx.renderer->SubmitSprite(resolved[index - 1u]);
        resolvedRequests[index - 1u]->rendererAuthorityPublished = true;
        resolvedRequests[index - 1u]->runtimeDrawPublished = true;
    }
    return true;
}

static bool SS0DirectSubmitPracticeSlices8001C604(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs) {
    auto& list = s_ss0Direct.practiceSliceDrawList;
    list.rendererAuthorityPublished = false;
    list.runtimeDrawPublished = false;
    if (!list.sourceKnown || !list.accepted || !list.complete ||
        !list.rawTextureOnly || !list.runtimeSubmitAllowed ||
        !list.precedingPortraitDrawProducerKnown ||
        !list.staticWriterSemanticsKnown || !list.localRgbKnown ||
        list.packetOtMutationPublished || list.replaySourceUsed ||
        list.truncated ||
        list.count != PrSS0DirectoryPagesRenderDirect::
                          kPracticeSliceSpriteCount8001C604) {
        return false;
    }
    if (!ctx.renderer) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               PrSS0DirectoryPagesRenderDirect::
                   kPracticeSliceSpriteCount8001C604>
        resolved{};
    for (std::size_t index = 0u; index < list.count; ++index) {
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                SS0DirectRawSpriteFromPracticeSlice8001C604(
                    list, list.sprites[index]),
                resolved[index])) {
            return false;
        }
        resolved[index].order = kPracticeOtOrderSlices +
            static_cast<uint64_t>(list.count - index);
    }

    for (std::size_t index = list.count; index > 0u; --index) {
        ctx.renderer->SubmitSprite(resolved[index - 1u]);
    }
    list.rendererAuthorityPublished = true;
    list.runtimeDrawPublished = true;
    return true;
}

static bool SS0DirectSubmitPracticePostSliceTail80023618(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs) {
    auto& list = s_ss0Direct.practicePostSliceTailDrawList;
    list.rendererAuthorityPublished = false;
    list.runtimeDrawPublished = false;
    if (!list.sourceKnown || !list.accepted || !list.complete ||
        !list.rawTextureOnly || !list.runtimeSubmitAllowed ||
        (!list.precedingSliceDrawPublished && !list.alwaysVisibleFixedTail) ||
        !list.languageAccepted || !list.exitSelectionAccepted ||
        !list.exitBlinkAccepted || !list.staticWriterSemanticsKnown ||
        !list.staticDescriptorSourcesKnown ||
        list.packetOtMutationPublished || list.replaySourceUsed ||
        list.truncated ||
        list.count != PrSS0DirectoryPagesRenderDirect::
                          kPracticePostSliceTailSpriteCount80023618) {
        return false;
    }
    if (!ctx.renderer) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               PrSS0DirectoryPagesRenderDirect::
                   kPracticePostSliceTailSpriteCount80023618>
        resolved{};
    for (std::size_t index = 0u; index < list.count; ++index) {
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                SS0DirectRawSpriteFromPracticePostSliceTail80023618(
                    list, list.sprites[index]),
                resolved[index])) {
            return false;
        }
        resolved[index].order = kPracticeOtOrderTail +
            static_cast<uint64_t>(list.count - index);
    }

    for (std::size_t index = list.count; index > 0u; --index) {
        ctx.renderer->SubmitSprite(resolved[index - 1u]);
    }
    list.rendererAuthorityPublished = true;
    list.runtimeDrawPublished = true;
    return true;
}

enum class SS0DirectResolvedPageCommandKind80020A3C : uint8_t {
    Sprite = 0,
    SolidRect,
};

struct SS0DirectResolvedPageCommand80020A3C {
    SS0DirectResolvedPageCommandKind80020A3C kind =
        SS0DirectResolvedPageCommandKind80020A3C::Sprite;
    uint32_t psxAddress = 0u;
    D3D11Renderer::SpriteCmd sprite{};
    D3D11Renderer::SolidRectCmd solidRect{};
};

static bool SS0DirectResolveCardIoBannerRect80020A3C(
    float vx,
    float vy,
    float vs,
    const PrSS0CardIoBannerRenderDirect::
        CardIoBannerSolidRectCommand80020A3C& source,
    D3D11Renderer::SolidRectCmd& command) {
    if (!source.known || source.width == 0u || source.height == 0u ||
        (source.attr & 0xFF000000u) != 0x40000000u) {
        return false;
    }
    command.x = vx + static_cast<float>(source.x) * vs;
    command.y = vy + static_cast<float>(source.y) * vs;
    command.w = static_cast<float>(source.width) * vs;
    command.h = static_cast<float>(source.height) * vs;
    command.r = static_cast<float>(source.r) / 255.0f;
    command.g = static_cast<float>(source.g) / 255.0f;
    command.b = static_cast<float>(source.b) / 255.0f;
    // 80020A3C receives the PSX box-fill attribute 0x400F0F0F.  Bit 30
    // selects the PSX average blend mode (ABR0), so the host equivalent is
    // a half-alpha fill over the menu rather than an opaque black rectangle.
    command.a = (source.attr & 0x40000000u) != 0u ? 0.5f : 1.0f;
    command.layer = SS0DirectRawSpriteLayer8003FA20(source.priority);
    command.order = 0u;
    return true;
}

static bool SS0DirectResolveMainDirectory80021E60(
    PrGameContext& ctx,
    const PrSS0DirectoryDispatcherDirect::MainMenuState800264AC&
        mainMenuState,
    float vx,
    float vy,
    float vs,
    bool contextPresent,
    uint16_t backdropPriority,
    bool submit) {
    namespace Directory = PrSS0DirectoryPagesRenderDirect;
    namespace Backdrop = PrSS0EventBackdropRenderDirect;
    namespace Banner = PrSS0CardIoBannerRenderDirect;
    Directory::MainDirectoryState80021E60 state{};
    state.contextPresent = contextPresent;
    state.language = s_ss0Direct.optionsWord800916D8;
    state.blinkOnOff = s_ss0Direct.mainMenuBlink;
    state.cursor = mainMenuState.cursor;
    for (std::size_t i = 0; i < std::size(state.itemValue); ++i) {
        state.itemValue[i] = mainMenuState.itemValue[i];
    }
    state.exitConfirmed = mainMenuState.doneFlag;

    const Directory::MainDirectoryDrawList80021E60 drawList =
        Directory::BuildMainDirectoryDrawList80021E60(state);
    const bool bannerRouteKnown =
        drawList.cardIoOverlay80020A3CRequired &&
        drawList.blockedByCardIoOverlay80020A3C &&
        drawList.cardIoOverlayInsertIndexKnown &&
        drawList.cardIoOverlayInsertIndex <= drawList.count;
    const uint32_t expectedDirectoryCount =
        Directory::kMainDirectoryBaseSpriteCount80021E60 +
        (drawList.choice2ReplayThenLoadFallthrough
             ? Directory::kMainDirectoryChoice2FallthroughSpriteCount80021E60
             : 0u);
    if (!drawList.accepted || !drawList.rawTextureOnly ||
        drawList.count != expectedDirectoryCount ||
        (!drawList.complete && !bannerRouteKnown)) {
        return false;
    }

    Banner::CardIoBannerDrawList80020A3C banner{};
    if (bannerRouteKnown) {
        banner = Banner::BuildMainDirectoryCardIoBannerDrawList80020A3C(
            state.language);
        if (!banner.complete || !banner.rawTextureSpritesOnly ||
            banner.packetRgbRequiredForVisibleOutput) {
            return false;
        }
    }

    const Backdrop::EventBackdropDrawList8001D74C backdrop =
        Backdrop::BuildEventBackdropDrawList8001D74C(backdropPriority);
    if (!backdrop.complete || !backdrop.rawTextureOnly ||
        backdrop.count != Backdrop::kEventBackdropSpriteCapacity8001D74C) {
        return false;
    }

    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               Backdrop::kEventBackdropSpriteCapacity8001D74C>
        resolvedBackdrop{};
    std::size_t resolvedBackdropCount = 0;
    for (std::size_t i = 0; i < backdrop.count; ++i) {
        const auto source =
            SS0DirectRawSpriteFromBackdrop8001D74C(backdrop.commands[i]);
        if (!SS0DirectResolveRawSprite8001B590(
                ctx, vx, vy, vs, source,
                resolvedBackdrop[resolvedBackdropCount])) {
            return false;
        }
        ++resolvedBackdropCount;
    }

    constexpr std::size_t kResolvedPageCommandCapacity =
        Directory::kMainDirectorySpriteCapacity80021E60 +
        Banner::kCardIoBannerCommandCapacity80020A3C;
    std::array<SS0DirectResolvedPageCommand80020A3C,
               kResolvedPageCommandCapacity>
        resolvedPage{};
    std::size_t resolvedPageCount = 0;
    const auto appendDirectory = [&](std::size_t index) -> bool {
        if (index >= drawList.count ||
            resolvedPageCount >= resolvedPage.size()) {
            return false;
        }
        const auto source = SS0DirectRawSpriteFromMainDirectory80021E60(
            drawList.commands[index]);
        auto& resolved = resolvedPage[resolvedPageCount];
        resolved.kind = SS0DirectResolvedPageCommandKind80020A3C::Sprite;
        resolved.psxAddress = source.psxAddress;
        if (!SS0DirectResolveRawSprite8001B590(
                ctx, vx, vy, vs, source, resolved.sprite)) {
            return false;
        }
        if (const auto nativeTim =
                SS0DirectResolveNativeMainMenuTim80021E60(ctx, source);
            nativeTim != nullptr) {
            resolved.sprite.texture = nativeTim;
            resolved.sprite.u0 = 0.0f;
            resolved.sprite.v0 = 0.0f;
            resolved.sprite.u1 = 1.0f;
            resolved.sprite.v1 = 1.0f;
        }
        ++resolvedPageCount;
        return true;
    };

    const std::size_t insertIndex = bannerRouteKnown
        ? drawList.cardIoOverlayInsertIndex
        : drawList.count;
    for (std::size_t i = 0; i < insertIndex; ++i) {
        if (!appendDirectory(i)) {
            return false;
        }
    }
    if (bannerRouteKnown) {
        for (std::size_t i = 0; i < banner.count; ++i) {
            if (resolvedPageCount >= resolvedPage.size()) {
                return false;
            }
            const auto& source = banner.commands[i];
            auto& resolved = resolvedPage[resolvedPageCount];
            if (!source.known) {
                return false;
            }
            if (source.kind == Banner::CardIoBannerCommandKind80020A3C::Sprite) {
                resolved.kind =
                    SS0DirectResolvedPageCommandKind80020A3C::Sprite;
                if (!SS0DirectResolveRawSprite8001B590(
                        ctx, vx, vy, vs,
                        SS0DirectRawSpriteFromCardIoBanner80020A3C(
                            source.sprite),
                        resolved.sprite)) {
                    return false;
                }
            } else {
                resolved.kind =
                    SS0DirectResolvedPageCommandKind80020A3C::SolidRect;
                if (!SS0DirectResolveCardIoBannerRect80020A3C(
                        vx, vy, vs, source.solidRect,
                        resolved.solidRect)) {
                    return false;
                }
            }
            ++resolvedPageCount;
        }
    }
    for (std::size_t i = insertIndex; i < drawList.count; ++i) {
        if (!appendDirectory(i)) {
            return false;
        }
    }

    const std::size_t expectedPageCount = drawList.count + banner.count;
    if (resolvedBackdropCount != backdrop.count ||
        resolvedPageCount != expectedPageCount) {
        return false;
    }
    if (!submit) {
        return true;
    }
    for (std::size_t i = backdrop.count; i > 0; --i) {
        ctx.renderer->SubmitSprite(resolvedBackdrop[i - 1u]);
    }
    for (std::size_t i = resolvedPageCount; i > 0; --i) {
        const auto& command = resolvedPage[i - 1u];
        if (command.kind ==
            SS0DirectResolvedPageCommandKind80020A3C::Sprite) {
            D3D11Renderer::SpriteCmd sprite = command.sprite;
            // W sprites are the native PSX plate outline/selection halo.  They
            // share the descriptor table with the colored B/E/G/I/S payloads,
            // but the original OT places the halo behind those payloads.  The
            // old reverse submission order used to put W on top, exposing its
            // black work-image interior as the destroyed shell.  Preserve W
            // geometry while forcing it one OT band behind the color plate.
            //
            // The language plate is the one exception in the captured 80021E60
            // order: MAIN_1W1/1W2 is submitted with priority=1 while the ev=3
            // backdrop is priority=3.  Applying the generic -4 shift moved
            // that outline below the backdrop, so the blue MAIN_1B plate was
            // visible but its native outer border disappeared.  Keep the
            // source priority for the two language W templates; it still
            // remains below the priority=0 language payload and above the
            // backdrop, exactly as the pseudo-C call order requires.
            if (SS0DirectMainDirectoryWTexture80021E60(
                    command.psxAddress)) {
                const bool languageW =
                    command.psxAddress == 0x80051010u ||
                    command.psxAddress == 0x80051020u;
                if (!languageW) {
                    sprite.layer -= 4;
                }
            }
            ctx.renderer->SubmitSprite(sprite);
        } else {
            ctx.renderer->SubmitSolidRect(command.solidRect);
        }
    }
    return true;
}

static bool SS0DirectResolveCardGrid80020F94(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    int32_t eventId,
    bool submit) {
    namespace Directory = PrSS0DirectoryPagesRenderDirect;
    namespace Backdrop = PrSS0EventBackdropRenderDirect;
    namespace Banner = PrSS0CardIoBannerRenderDirect;
    const Directory::CardGridState80020F94& state =
        s_ss0Direct.cardGridDrawState80020F94;
    if (!state.requestBound || state.eventId != eventId) {
        return false;
    }
    const Directory::CardGridDrawList80020F94 drawList =
        Directory::BuildCardGridDrawList80020F94(state);
    const bool bannerRouteKnown =
        drawList.cardIoOverlay80020A3CRequired &&
        drawList.blockedByCardIoOverlay80020A3C &&
        drawList.cardIoOverlayInsertIndexKnown &&
        drawList.cardIoOverlayInsertIndex <= drawList.count &&
        drawList.cardIoMessageType ==
            Banner::kRemoveCardMessageType80020A3C;
    if (!drawList.sourceKnown || !drawList.accepted ||
        (!drawList.complete && !bannerRouteKnown) ||
        (!drawList.runtimeSubmitAllowed && !bannerRouteKnown) ||
        !drawList.rawTextureOnly || drawList.truncated ||
        drawList.count == 0u || drawList.count > drawList.commands.size()) {
        return false;
    }

    Banner::CardIoBannerDrawList80020A3C banner{};
    if (bannerRouteKnown) {
        banner = Banner::BuildCardIoBannerDrawList80020A3C(
            drawList.cardIoMessageType, state.language);
        if (!banner.complete || !banner.rawTextureSpritesOnly ||
            banner.packetRgbRequiredForVisibleOutput) {
            return false;
        }
    }
    const Backdrop::EventBackdropDrawList8001D74C backdrop =
        Backdrop::BuildEventBackdropDrawList8001D74C(3u);
    if (!backdrop.complete || !backdrop.rawTextureOnly ||
        backdrop.count != Backdrop::kEventBackdropSpriteCapacity8001D74C) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               Backdrop::kEventBackdropSpriteCapacity8001D74C>
        resolvedBackdrop{};
    std::size_t resolvedBackdropCount = 0u;
    for (std::size_t i = 0u; i < backdrop.count; ++i) {
        const auto source =
            SS0DirectRawSpriteFromBackdrop8001D74C(backdrop.commands[i]);
        if (!SS0DirectResolveRawSprite8001B590(
                ctx, vx, vy, vs, source,
                resolvedBackdrop[resolvedBackdropCount])) {
            return false;
        }
        ++resolvedBackdropCount;
    }

    constexpr std::size_t kResolvedPageCommandCapacity =
        Directory::kCardGridSpriteCapacity80020F94 +
        Banner::kCardIoBannerCommandCapacity80020A3C;
    std::array<SS0DirectResolvedPageCommand80020A3C,
               kResolvedPageCommandCapacity>
        resolvedPage{};
    std::size_t resolvedPageCount = 0u;
    const bool pageRouteKnown = drawList.complete || bannerRouteKnown;
    const auto appendCardGrid = [&](std::size_t index) -> bool {
        if (index >= drawList.count ||
            resolvedPageCount >= resolvedPage.size()) {
            return false;
        }
        const auto source = SS0DirectRawSpriteFromCardGrid80020F94(
            drawList, drawList.commands[index], pageRouteKnown);
        auto& resolved = resolvedPage[resolvedPageCount];
        resolved.kind = SS0DirectResolvedPageCommandKind80020A3C::Sprite;
        if (!SS0DirectResolveRawSprite8001B590(
                ctx, vx, vy, vs, source, resolved.sprite)) {
            return false;
        }
        ++resolvedPageCount;
        return true;
    };

    const std::size_t insertIndex = bannerRouteKnown
        ? drawList.cardIoOverlayInsertIndex
        : drawList.count;
    for (std::size_t i = 0u; i < insertIndex; ++i) {
        if (!appendCardGrid(i)) {
            return false;
        }
    }
    if (bannerRouteKnown) {
        for (std::size_t i = 0u; i < banner.count; ++i) {
            if (resolvedPageCount >= resolvedPage.size()) {
                return false;
            }
            const auto& source = banner.commands[i];
            auto& resolved = resolvedPage[resolvedPageCount];
            if (!source.known) {
                return false;
            }
            if (source.kind == Banner::CardIoBannerCommandKind80020A3C::Sprite) {
                resolved.kind =
                    SS0DirectResolvedPageCommandKind80020A3C::Sprite;
                if (!SS0DirectResolveRawSprite8001B590(
                        ctx, vx, vy, vs,
                        SS0DirectRawSpriteFromCardIoBanner80020A3C(
                            source.sprite),
                        resolved.sprite)) {
                    return false;
                }
            } else {
                resolved.kind =
                    SS0DirectResolvedPageCommandKind80020A3C::SolidRect;
                if (!SS0DirectResolveCardIoBannerRect80020A3C(
                        vx, vy, vs, source.solidRect,
                        resolved.solidRect)) {
                    return false;
                }
            }
            ++resolvedPageCount;
        }
    }
    for (std::size_t i = insertIndex; i < drawList.count; ++i) {
        if (!appendCardGrid(i)) {
            return false;
        }
    }
    const std::size_t expectedPageCount = drawList.count + banner.count;
    if (resolvedBackdropCount != backdrop.count ||
        resolvedPageCount != expectedPageCount) {
        return false;
    }
    if (!submit) {
        return true;
    }
    for (std::size_t i = resolvedBackdropCount; i > 0u; --i) {
        ctx.renderer->SubmitSprite(resolvedBackdrop[i - 1u]);
    }
    for (std::size_t i = resolvedPageCount; i > 0u; --i) {
        const auto& command = resolvedPage[i - 1u];
        if (command.kind ==
            SS0DirectResolvedPageCommandKind80020A3C::Sprite) {
            ctx.renderer->SubmitSprite(command.sprite);
        } else {
            ctx.renderer->SubmitSolidRect(command.solidRect);
        }
    }
    return true;
}

static bool SS0DirectResolveOptions80021910(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    bool submit) {
    namespace Directory = PrSS0DirectoryPagesRenderDirect;
    namespace Backdrop = PrSS0EventBackdropRenderDirect;

    Directory::OptionsState80021910 state{};
    state.language = s_ss0Direct.optionsState.word800916D8;
    state.blinkOnOff = s_ss0Direct.optionsBlink;
    state.doneFlag = s_ss0Direct.optionsState.doneFlag;
    state.cursor = s_ss0Direct.optionsState.cursor;
    state.opt0Value = s_ss0Direct.optionsState.opt0Value;
    state.opt1Value = s_ss0Direct.optionsState.word801C36A6;
    state.opt2Value = s_ss0Direct.optionsState.opt2Value;

    const Directory::OptionsDrawList80021910 drawList =
        Directory::BuildOptionsDrawList80021910(state);
    if (!drawList.sourceKnown || !drawList.accepted || !drawList.complete ||
        !drawList.rawTextureOnly || drawList.truncated ||
        drawList.count != Directory::kOptionsSpriteCapacity80021910) {
        return false;
    }

    const Backdrop::EventBackdropDrawList8001D74C backdrop =
        Backdrop::BuildEventBackdropDrawList8001D74C(4u);
    if (!backdrop.complete || !backdrop.rawTextureOnly ||
        backdrop.count != Backdrop::kEventBackdropSpriteCapacity8001D74C) {
        return false;
    }

    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               Backdrop::kEventBackdropSpriteCapacity8001D74C>
        resolvedBackdrop{};
    std::size_t resolvedBackdropCount = 0u;
    for (std::size_t i = 0u; i < backdrop.count; ++i) {
        const auto source =
            SS0DirectRawSpriteFromBackdrop8001D74C(backdrop.commands[i]);
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                source,
                resolvedBackdrop[resolvedBackdropCount])) {
            return false;
        }
        ++resolvedBackdropCount;
    }

    std::array<D3D11Renderer::SpriteCmd,
               Directory::kOptionsSpriteCapacity80021910>
        resolvedPage{};
    std::size_t resolvedPageCount = 0u;
    for (std::size_t i = 0u; i < drawList.count; ++i) {
        const auto source =
            SS0DirectRawSpriteFromOptions80021910(drawList.commands[i]);
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                source,
                resolvedPage[resolvedPageCount])) {
            return false;
        }
        ++resolvedPageCount;
    }

    if (resolvedBackdropCount != backdrop.count ||
        resolvedPageCount != drawList.count) {
        return false;
    }
    if (!submit) {
        return true;
    }
    for (std::size_t i = resolvedBackdropCount; i > 0u; --i) {
        ctx.renderer->SubmitSprite(resolvedBackdrop[i - 1u]);
    }
    for (std::size_t i = resolvedPageCount; i > 0u; --i) {
        ctx.renderer->SubmitSprite(resolvedPage[i - 1u]);
    }
    return true;
}

static bool SS0DirectStageSelectRenderSourceConsistent80020568() {
    // 800267F8 materializes ctx+0x0E..ctx+0x1A before 80020568 reads that
    // context. The translated state must therefore carry the post-init words,
    // including the F0/DA rewrites, rather than the pre-init global bytes.
    for (int32_t index = 0; index < 6; ++index) {
        const bool expectedEnabled =
            s_ss0Direct.stageSelectState.rawStatus80092F1DTo23[index] != 0u;
        if (s_ss0Direct.stageSelectState.enabled[index + 1] !=
            expectedEnabled) {
            if (!s_ss0Direct.stageSelectRenderSourceMismatchLogged) {
                s_ss0Direct.stageSelectRenderSourceMismatchLogged = true;
                Log::Printf(
                    "SS0 direct runtime: event2 80020568 render/input source mismatch index=%d f0Known=%d f0=%d da=%d",
                    index,
                    s_ss0Direct.stageSelectWord800916F0Known ? 1 : 0,
                    s_ss0Direct.stageSelectWord800916F0,
                    s_ss0Direct.stageSelectWord800916DA);
            }
            return false;
        }
    }
    return true;
}

static bool SS0DirectResolveStageSelect80020568(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    bool submit) {
    namespace Directory = PrSS0DirectoryPagesRenderDirect;
    namespace Backdrop = PrSS0EventBackdropRenderDirect;

    if (!SS0DirectStageSelectRenderSourceConsistent80020568()) {
        return false;
    }

    Directory::StageSelectState80020568 state{};
    state.language = s_ss0Direct.optionsWord800916D8;
    state.blinkOnOff = s_ss0Direct.stageSelectBlink;
    state.doneFlag = s_ss0Direct.stageSelectState.doneFlag ? 1 : 0;
    state.cursor = s_ss0Direct.stageSelectState.cursor;
    for (uint32_t index = 0u; index < 7u; ++index) {
        state.rawStatus80092F1DTo23[index] =
            s_ss0Direct.stageSelectState.rawStatus80092F1DTo23[index];
    }

    const Directory::StageSelectDrawList80020568 drawList =
        Directory::BuildStageSelectDrawList80020568(state);
    const uint32_t expectedCount =
        Directory::kStageSelectBaseSpriteCount80020568 +
        (drawList.bonusOverlayPresent ? 2u : 0u);
    if (!drawList.sourceKnown || !drawList.accepted || !drawList.complete ||
        !drawList.rawTextureOnly || drawList.truncated ||
        drawList.count != expectedCount ||
        drawList.count > Directory::kStageSelectSpriteCapacity80020568) {
        return false;
    }

    const Backdrop::EventBackdropDrawList8001D74C backdrop =
        Backdrop::BuildEventBackdropDrawList8001D74C(3u);
    if (!backdrop.complete || !backdrop.rawTextureOnly ||
        backdrop.count != Backdrop::kEventBackdropSpriteCapacity8001D74C) {
        return false;
    }

    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               Backdrop::kEventBackdropSpriteCapacity8001D74C>
        resolvedBackdrop{};
    std::size_t resolvedBackdropCount = 0u;
    for (std::size_t index = 0u; index < backdrop.count; ++index) {
        const auto source =
            SS0DirectRawSpriteFromBackdrop8001D74C(backdrop.commands[index]);
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                source,
                resolvedBackdrop[resolvedBackdropCount])) {
            return false;
        }
        ++resolvedBackdropCount;
    }

    std::array<D3D11Renderer::SpriteCmd,
               Directory::kStageSelectSpriteCapacity80020568>
        resolvedPage{};
    std::size_t resolvedPageCount = 0u;
    for (std::size_t index = 0u; index < drawList.count; ++index) {
        const auto source = SS0DirectRawSpriteFromStageSelect80020568(
            drawList.commands[index]);
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                source,
                resolvedPage[resolvedPageCount])) {
            return false;
        }
        ++resolvedPageCount;
    }

    if (resolvedBackdropCount != backdrop.count ||
        resolvedPageCount != drawList.count) {
        return false;
    }
    if (!submit) {
        return true;
    }
    for (std::size_t index = resolvedBackdropCount; index > 0u; --index) {
        ctx.renderer->SubmitSprite(resolvedBackdrop[index - 1u]);
    }
    for (std::size_t index = resolvedPageCount; index > 0u; --index) {
        ctx.renderer->SubmitSprite(resolvedPage[index - 1u]);
    }
    return true;
}

static bool SS0DirectResolveHiScoreTable80021594(
    PrGameContext& ctx,
    float vx,
    float vy,
    float vs,
    bool submit) {
    namespace Backdrop = PrSS0EventBackdropRenderDirect;
    namespace HiScore = PrSS0HiScoreRenderDirect;

    const HiScore::HiScoreTableDrawList80021594 drawList =
        SS0DirectBuildHiScoreTableDrawList80021594();
    if (!drawList.accepted || !drawList.complete ||
        !drawList.rawTextureOnly ||
        drawList.count < HiScore::kHiScoreSpriteCommandCount80021594 ||
        drawList.count > HiScore::kHiScoreSpriteCommandCapacity80021594) {
        return false;
    }

    const Backdrop::EventBackdropDrawList8001D74C backdrop =
        Backdrop::BuildEventBackdropDrawList8001D74C(3u);
    if (!backdrop.complete || !backdrop.rawTextureOnly ||
        backdrop.count != Backdrop::kEventBackdropSpriteCapacity8001D74C) {
        return false;
    }

    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    std::array<D3D11Renderer::SpriteCmd,
               Backdrop::kEventBackdropSpriteCapacity8001D74C>
        resolvedBackdrop{};
    std::size_t resolvedBackdropCount = 0u;
    for (std::size_t i = 0u; i < backdrop.count; ++i) {
        const auto source =
            SS0DirectRawSpriteFromBackdrop8001D74C(backdrop.commands[i]);
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                source,
                resolvedBackdrop[resolvedBackdropCount])) {
            return false;
        }
        ++resolvedBackdropCount;
    }

    std::array<D3D11Renderer::SpriteCmd,
               HiScore::kHiScoreSpriteCommandCapacity80021594>
        resolvedPage{};
    std::size_t resolvedPageCount = 0u;
    for (std::size_t i = 0u; i < drawList.count; ++i) {
        const auto source =
            SS0DirectRawSpriteFromHiScore80021594(drawList.commands[i]);
        if (!SS0DirectResolveRawSprite8001B590(
                ctx,
                vx,
                vy,
                vs,
                source,
                resolvedPage[resolvedPageCount])) {
            return false;
        }
        ++resolvedPageCount;
    }

    if (resolvedBackdropCount != backdrop.count ||
        resolvedPageCount != drawList.count) {
        return false;
    }
    if (!submit) {
        return true;
    }
    for (std::size_t i = resolvedBackdropCount; i > 0u; --i) {
        ctx.renderer->SubmitSprite(resolvedBackdrop[i - 1u]);
    }
    for (std::size_t i = resolvedPageCount; i > 0u; --i) {
        ctx.renderer->SubmitSprite(resolvedPage[i - 1u]);
    }
    return true;
}

static bool SS0DirectRenderMainDirectory80021E60(PrGameContext& ctx,
                                                  float vx,
                                                  float vy,
                                                  float vs) {
    return SS0DirectResolveMainDirectory80021E60(
        ctx, s_ss0Direct.mainMenuState, vx, vy, vs, true, 3u, true);
}

static bool SS0DirectExecuteEvent3FrameBoundary80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event3FrameTransaction80026B94& transaction) {
    if (!transaction.active || !transaction.directDrawSubmitted ||
        !transaction.formalTailOrderBound) {
        return false;
    }

    auto& hostFrame = s_ss0Direct.mainMenuFrameHostState8001E750;
    bool hostWaitExecuted = transaction.hostWaitExecuted;
    if (!hostWaitExecuted) {
        auto waitFrame =
            PrPsxEventFrameDirect::PsxCall80035560_WaitFrameDetailed(
                hostFrame, PrPsxVSyncDirect::ProcessVSyncState80035560(), 0);
        if (!waitFrame.softwareWaitComplete) {
            waitFrame = PrPsxEventFrameDirect::
                PsxConsume80035560_WaitFrameHostVblankDetailed(
                    hostFrame,
                    PrPsxVSyncDirect::ProcessVSyncState80035560(),
                    1);
        }
        hostWaitExecuted =
            waitFrame.sourceKnown && waitFrame.softwareStateCommitted &&
            waitFrame.softwareWaitComplete &&
            waitFrame.mode ==
                PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
    }

    bool hostEndExecuted = transaction.hostEndExecuted;
    if (hostWaitExecuted && !hostEndExecuted &&
        s_ss0Direct.mainMenuFrameEndFrameBindingKnown8001EA00) {
        hostFrame.gp368WorkSlot =
            s_ss0Direct.mainMenuFrameEndFrameWorkListSlot8001EA00;
        const auto endFrame =
            PrPsxEventFrameDirect::PsxCall8001EA00_EndFrameDetailed(
                hostFrame, 3);
        // Initialization alone cannot prove flip, clear, and submit ran.
        hostEndExecuted = endFrame.completeWithinLimits;
    }

    bool hostTextFlushExecuted = transaction.hostTextFlushExecuted;
    if (hostWaitExecuted && hostEndExecuted && !hostTextFlushExecuted) {
        hostTextFlushExecuted = SS0DirectFlushEventText800436F0(ctx);
    }
    if (!hostEndExecuted &&
        !s_ss0Direct.mainMenuFrameEndFrameBindingBlockedLogged8001EA00) {
        s_ss0Direct.mainMenuFrameEndFrameBindingBlockedLogged8001EA00 = true;
        Log::Printf(
            "SS0 direct runtime: event3 host boundary blocked by missing formal work-list binding frame=%u",
            static_cast<unsigned>(ctx.frame));
    }
    return PrSS0EventFrameLoopDirect::CommitEvent3FrameHostBoundary80026B94(
        transaction,
        hostWaitExecuted,
        hostEndExecuted,
        hostTextFlushExecuted);
}

static void SS0DirectRenderMainMenu(PrGameContext& ctx) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    (void)SS0DirectEnsureEventTextRuntime(ctx);
    auto& transaction = s_ss0Direct.mainMenuFrameTransaction80026B94;
    if (transaction.directDrawCompleteWithinLimits &&
        !transaction.hostFrameBoundaryExecuted) {
        if (SS0DirectExecuteEvent3FrameBoundary80026B94(ctx, transaction)) {
            SS0DirectCommitMainMenuFrameTail80026B94();
            Log::Printf(
                "SS0 direct runtime: event3 deferred host boundary committed frame=%u hostFrame=%u hostWaitExecuted=1 hostEndExecuted=1 hostTextFlushExecuted=1 hostFrameBoundaryExecuted=1 exactHalParity=0",
                static_cast<unsigned>(transaction.logicFrame),
                static_cast<unsigned>(ctx.frame));
        }
        // 8001E750 has already produced the visible Event3 page, while the
        // translated 80035560/8001EA00/800436F0 boundary is allowed to
        // finish on this host frame.  Keep that same page on the queue while
        // the boundary is pending; returning with only main.cpp's clear
        // colour exposes a one-frame black/grey flash at page switches.
        if (ctx.renderer) {
            float vx = 0.0f;
            float vy = 0.0f;
            float vs = 1.0f;
            SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
            (void)SS0DirectRenderMainDirectory80021E60(ctx, vx, vy, vs);
        }
        return;
    }
    // Event3 is a logic-rate transaction but the host target is cleared at
    // every presentation.  Re-submit the completed directory on the
    // intervening render-only half-step without touching dispatcher state.
    if (transaction.active && transaction.directDrawCompleteWithinLimits &&
        ctx.renderOnlyFrame) {
        if (ctx.renderer) {
            float vx = 0.0f;
            float vy = 0.0f;
            float vs = 1.0f;
            SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
            (void)SS0DirectRenderMainDirectory80021E60(ctx, vx, vy, vs);
        }
        return;
    }
    if (!ctx.renderer ||
        !EventFrame::CanSubmitEvent3FrameDraw8001E750(
            transaction, ctx.frame, ctx.renderOnlyFrame)) {
        return;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    const bool firstTranslatedSubmit =
        !transaction.directDrawCompleteWithinLimits;
    if (!SS0DirectRenderMainDirectory80021E60(ctx, vx, vy, vs)) {
        Log::Printf(
            "SS0 direct runtime: event3 frame transaction draw rejected logicFrame=%u hostFrame=%u class=%s pending=1",
            static_cast<unsigned>(transaction.logicFrame),
            static_cast<unsigned>(ctx.frame),
            SS0DirectMainMenuFrameClassName80026B94(
                transaction.frameClass));
        return;
    }
    if ((firstTranslatedSubmit &&
         !s_ss0Direct.mainMenuFrameTailCandidateKnown80026B94) ||
        !EventFrame::CommitEvent3FrameDrawAndBindFormalTail80026B94(
            transaction, ctx.frame, ctx.renderOnlyFrame)) {
        Log::Printf(
            "SS0 direct runtime: event3 frame transaction commit rejected frame=%u class=%s renderOnly=%d",
            static_cast<unsigned>(ctx.frame),
            SS0DirectMainMenuFrameClassName80026B94(
                transaction.frameClass),
            ctx.renderOnlyFrame ? 1 : 0);
        return;
    }
    if (firstTranslatedSubmit) {
        const bool hostBoundaryExecuted =
            SS0DirectExecuteEvent3FrameBoundary80026B94(ctx, transaction);
        if (hostBoundaryExecuted) {
            SS0DirectCommitMainMenuFrameTail80026B94();
        }
        Log::Printf(
            "SS0 direct runtime: event3 frame draw submitted frame=%u hostFrame=%u class=%s inputClosed=%d tail=%d/%d order=80026720>8001E750{8001D74C(3,workSlot=%u),80021E60(800544F8)}>80035560(0)>8001EA00(3)>800436F0(-1) directDrawExecuted=1 formalTailOrderBound=1 hostWaitExecuted=%d hostEndExecuted=%d hostTextFlushExecuted=%d hostFrameBoundaryExecuted=%d exactHalParity=0",
            static_cast<unsigned>(transaction.logicFrame),
            static_cast<unsigned>(ctx.frame),
            SS0DirectMainMenuFrameClassName80026B94(
                transaction.frameClass),
            transaction.inputClosed ? 1 : 0,
            transaction.tailFramesRemainingBefore,
            transaction.tailFramesRemainingAfter,
            static_cast<unsigned>(
                s_ss0Direct.mainMenuFrameEndFrameWorkListSlot8001EA00),
            transaction.hostWaitExecuted ? 1 : 0,
            transaction.hostEndExecuted ? 1 : 0,
            transaction.hostTextFlushExecuted ? 1 : 0,
            hostBoundaryExecuted ? 1 : 0);
    }
}

static void SS0DirectRenderMainMenuTransitionBackdrop80021E60(
    PrGameContext& ctx) {
    // WaitTransition renders the already-published SS0 directory page under
    // the transition overlay. It is visual support only, not an Event3 frame
    // transaction and not a source of tail or semantic-frame credit.
    if (!ctx.renderer) {
        return;
    }
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    (void)SS0DirectRenderMainDirectory80021E60(ctx, vx, vy, vs);
}

static bool SS0DirectRenderOptionsVisualOnly(PrGameContext& ctx) {
    if (!ctx.renderer) {
        return false;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    return SS0DirectResolveOptions80021910(ctx, vx, vy, vs, true);
}

static bool SS0DirectExecuteEvent17FrameBoundary80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event17FrameTransaction80026B94& transaction) {
    if (!transaction.active || !transaction.directDrawSubmitted ||
        !transaction.formalTailOrderBound) {
        return false;
    }

    auto& hostFrame = s_ss0Direct.optionsFrameHostState8001E750;
    bool hostWaitExecuted = transaction.hostWaitExecuted;
    if (!hostWaitExecuted) {
        auto waitFrame =
            PrPsxEventFrameDirect::PsxCall80035560_WaitFrameDetailed(
                hostFrame, PrPsxVSyncDirect::ProcessVSyncState80035560(), 0);
        if (!waitFrame.softwareWaitComplete) {
            waitFrame = PrPsxEventFrameDirect::
                PsxConsume80035560_WaitFrameHostVblankDetailed(
                    hostFrame,
                    PrPsxVSyncDirect::ProcessVSyncState80035560(),
                    1);
        }
        hostWaitExecuted =
            waitFrame.sourceKnown && waitFrame.softwareStateCommitted &&
            waitFrame.softwareWaitComplete &&
            waitFrame.mode ==
                PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
    }

    bool hostEndExecuted = transaction.hostEndExecuted;
    if (hostWaitExecuted && !hostEndExecuted &&
        s_ss0Direct.optionsFrameEndFrameBindingKnown8001EA00) {
        hostFrame.gp368WorkSlot =
            s_ss0Direct.optionsFrameEndFrameWorkListSlot8001EA00;
        const auto endFrame =
            PrPsxEventFrameDirect::PsxCall8001EA00_EndFrameDetailed(
                hostFrame, 17);
        hostEndExecuted = endFrame.completeWithinLimits;
    }

    bool hostTextFlushExecuted = transaction.hostTextFlushExecuted;
    if (hostWaitExecuted && hostEndExecuted && !hostTextFlushExecuted) {
        hostTextFlushExecuted = SS0DirectFlushEventText800436F0(ctx);
    }
    if (!hostEndExecuted &&
        !s_ss0Direct.optionsFrameEndFrameBindingBlockedLogged8001EA00) {
        s_ss0Direct.optionsFrameEndFrameBindingBlockedLogged8001EA00 = true;
        Log::Printf(
            "SS0 direct runtime: event17 host boundary blocked by graph/work-list submit preflight frame=%u workSlot=%u",
            static_cast<unsigned>(ctx.frame),
            static_cast<unsigned>(s_ss0Direct.optionsFrameEndFrameWorkListSlot8001EA00));
    }
    return PrSS0EventFrameLoopDirect::CommitEvent17FrameHostBoundary80026B94(
        transaction,
        hostWaitExecuted,
        hostEndExecuted,
        hostTextFlushExecuted);
}

static void SS0DirectRenderOptions(PrGameContext& ctx) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    (void)SS0DirectEnsureEventTextRuntime(ctx);
    if (s_ss0Direct.optionsResultPending80015788) {
        // 80015788's outer release wait keeps the Options page visible, but
        // it is no longer an Event17 semantic frame transaction.
        (void)SS0DirectRenderOptionsVisualOnly(ctx);
        return;
    }
    auto& transaction = s_ss0Direct.optionsFrameTransaction80026B94;
    if (transaction.directDrawCompleteWithinLimits &&
        !transaction.hostFrameBoundaryExecuted) {
        if (SS0DirectExecuteEvent17FrameBoundary80026B94(ctx, transaction)) {
            SS0DirectCommitOptionsFrameTail80026B94();
            Log::Printf(
                "SS0 direct runtime: event17 deferred host boundary committed frame=%u hostFrame=%u hostWaitExecuted=1 hostEndExecuted=1 hostTextFlushExecuted=1 hostFrameBoundaryExecuted=1 exactHalParity=0",
                static_cast<unsigned>(transaction.logicFrame),
                static_cast<unsigned>(ctx.frame));
        }
        // Preserve the already submitted Event17 page through the formal
        // host-boundary frame.  The source page remains authoritative; this
        // is a visual re-submit only and does not advance the event loop.
        (void)SS0DirectRenderOptionsVisualOnly(ctx);
        return;
    }
    // The host clears the render target on every 60 Hz presentation, while
    // the PSX Event17 transaction advances only on its logic frame.  Once the
    // translated page has completed its semantic draw, keep submitting that
    // same authoritative page on intervening render-only frames; otherwise a
    // clear-colour frame leaks between two logic ticks.
    if (transaction.active && transaction.directDrawCompleteWithinLimits &&
        ctx.renderOnlyFrame) {
        (void)SS0DirectRenderOptionsVisualOnly(ctx);
        return;
    }
    if (!ctx.renderer ||
        !EventFrame::CanSubmitEvent17FrameDraw8001E750(
            transaction, ctx.frame, ctx.renderOnlyFrame)) {
        return;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    const bool firstTranslatedSubmit =
        !transaction.directDrawCompleteWithinLimits;
    if (!SS0DirectResolveOptions80021910(ctx, vx, vy, vs, true)) {
        Log::Printf(
            "SS0 direct runtime: event17 frame transaction draw rejected logicFrame=%u hostFrame=%u class=%s pending=1",
            static_cast<unsigned>(transaction.logicFrame),
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent17FrameClassName80026B94(transaction.frameClass));
        return;
    }
    if ((firstTranslatedSubmit &&
         !s_ss0Direct.optionsFrameTailCandidateKnown80026B94) ||
        !EventFrame::CommitEvent17FrameDrawAndBindFormalTail80026B94(
            transaction, ctx.frame, ctx.renderOnlyFrame)) {
        Log::Printf(
            "SS0 direct runtime: event17 frame transaction commit rejected frame=%u class=%s renderOnly=%d",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent17FrameClassName80026B94(transaction.frameClass),
            ctx.renderOnlyFrame ? 1 : 0);
        return;
    }
    if (firstTranslatedSubmit) {
        const bool hostBoundaryExecuted =
            SS0DirectExecuteEvent17FrameBoundary80026B94(ctx, transaction);
        if (hostBoundaryExecuted) {
            SS0DirectCommitOptionsFrameTail80026B94();
        }
        Log::Printf(
            "SS0 direct runtime: event17 frame draw submitted frame=%u hostFrame=%u class=%s inputClosed=%d tail=%d/%d order=80026B54>8001E750{8001D74C(4,workSlot=%u),80021910(8005451C)}>80035560(0)>8001EA00(17)>800436F0(-1) directDrawExecuted=1 formalTailOrderBound=1 hostWaitExecuted=%d hostEndExecuted=%d hostTextFlushExecuted=%d hostFrameBoundaryExecuted=%d exactHalParity=0",
            static_cast<unsigned>(transaction.logicFrame),
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent17FrameClassName80026B94(transaction.frameClass),
            transaction.inputClosed ? 1 : 0,
            transaction.tailFramesRemainingBefore,
            transaction.tailFramesRemainingAfter,
            static_cast<unsigned>(s_ss0Direct.optionsFrameEndFrameWorkListSlot8001EA00),
            transaction.hostWaitExecuted ? 1 : 0,
            transaction.hostEndExecuted ? 1 : 0,
            transaction.hostTextFlushExecuted ? 1 : 0,
            hostBoundaryExecuted ? 1 : 0);
    }
}

static bool SS0DirectExecuteEvent2FrameBoundary80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event2FrameTransaction80026B94& transaction) {
    if (!transaction.active || !transaction.directDrawSubmitted ||
        !transaction.formalTailOrderBound) {
        return false;
    }

    // Current formal IDA closes this process epoch at the 80028590 startup
    // zero. The Event2 StageClear branch is therefore known and not taken;
    // no text or status values are synthesized from the old S0 shell.
    if (!transaction.stageClearGateValueKnown800916F6 ||
        transaction.stageClearGateValue800916F6 != 0u ||
        transaction.stageClearBranchTaken80026D70) {
        return false;
    }

    auto& hostFrame = s_ss0Direct.stageSelectFrameHostState8001E750;
    bool hostWaitExecuted = transaction.hostWaitExecuted;
    if (!hostWaitExecuted) {
        auto waitFrame =
            PrPsxEventFrameDirect::PsxCall80035560_WaitFrameDetailed(
                hostFrame, PrPsxVSyncDirect::ProcessVSyncState80035560(), 0);
        if (!waitFrame.softwareWaitComplete) {
            waitFrame = PrPsxEventFrameDirect::
                PsxConsume80035560_WaitFrameHostVblankDetailed(
                    hostFrame,
                    PrPsxVSyncDirect::ProcessVSyncState80035560(),
                    1);
        }
        hostWaitExecuted =
            waitFrame.sourceKnown && waitFrame.softwareStateCommitted &&
            waitFrame.softwareWaitComplete &&
            waitFrame.mode ==
                PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
    }

    bool hostEndExecuted = transaction.hostEndExecuted;
    if (hostWaitExecuted && !hostEndExecuted &&
        s_ss0Direct.stageSelectFrameEndFrameBindingKnown8001EA00) {
        hostFrame.gp368WorkSlot =
            s_ss0Direct.stageSelectFrameEndFrameWorkListSlot8001EA00;
        const auto endFrame =
            PrPsxEventFrameDirect::PsxCall8001EA00_EndFrameDetailed(
                hostFrame, 2);
        hostEndExecuted = endFrame.completeWithinLimits;
    }

    bool hostTextFlushExecuted = transaction.hostTextFlushExecuted;
    if (hostWaitExecuted && hostEndExecuted && !hostTextFlushExecuted) {
        hostTextFlushExecuted = SS0DirectFlushEventText800436F0(ctx);
    }
    if (!hostEndExecuted &&
        !s_ss0Direct.stageSelectFrameEndFrameBindingBlockedLogged8001EA00) {
        s_ss0Direct.stageSelectFrameEndFrameBindingBlockedLogged8001EA00 = true;
        Log::Printf(
            "SS0 direct runtime: event2 host boundary blocked by missing formal work-list binding frame=%u",
            static_cast<unsigned>(ctx.frame));
    }
    return PrSS0EventFrameLoopDirect::CommitEvent2FrameHostBoundary80026B94(
        transaction,
        hostWaitExecuted,
        hostEndExecuted,
        hostTextFlushExecuted);
}

static void SS0DirectRenderStageSelect(PrGameContext& ctx) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    (void)SS0DirectEnsureEventTextRuntime(ctx);
    auto& transaction = s_ss0Direct.stageSelectFrameTransaction80026B94;

    // 80015788 changes the active page to Stage Select before it enters
    // 80026B94.  The first Event2 frame may still be held by the mandatory
    // 80035510 release gate, so the PSX framebuffer continues to show the
    // pre-dispatch page during that interval.  A host BeginFrame clear has
    // no such retention.  Re-submit the already-preflighted 80020568 page
    // while this entry latch is set; this is presentation only and does not
    // create an Event2 tick, tail, or input result.
    if (s_ss0Direct.stageSelectEntryVisualPending80020568 && ctx.renderer) {
        float entryVx = 0.0f;
        float entryVy = 0.0f;
        float entryVs = 1.0f;
        SS0DirectCalcPs1Viewport(ctx.renderer, entryVx, entryVy, entryVs);
        if (SS0DirectResolveStageSelect80020568(
                ctx, entryVx, entryVy, entryVs, true)) {
            s_ss0Direct.stageSelectEntryVisualPending80020568 = false;
            Log::Printf(
                "SS0 direct runtime: stage select 80020568 entry page retained before first Event2 present frame=%u transactionActive=%d",
                static_cast<unsigned>(ctx.frame),
                transaction.active ? 1 : 0);
        }
    }
    if (transaction.directDrawCompleteWithinLimits &&
        !transaction.hostFrameBoundaryExecuted) {
        if (SS0DirectExecuteEvent2FrameBoundary80026B94(ctx, transaction)) {
            SS0DirectCommitStageSelectFrameTail80026B94();
            Log::Printf(
                "SS0 direct runtime: event2 deferred host boundary committed frame=%u hostFrame=%u hostWaitExecuted=1 hostEndExecuted=1 hostTextFlushExecuted=1 hostFrameBoundaryExecuted=1 exactHalParity=0",
                static_cast<unsigned>(transaction.logicFrame),
                static_cast<unsigned>(ctx.frame));
        }
        // 80020568's page is still the visible source while the Event2
        // 80035560/8001EA00 tail closes.  Re-submit it on this boundary frame
        // so the host clear cannot leak through as a one-frame flash.
        if (ctx.renderer) {
            float vx = 0.0f;
            float vy = 0.0f;
            float vs = 1.0f;
            SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
            (void)SS0DirectResolveStageSelect80020568(
                ctx, vx, vy, vs, true);
        }
        return;
    }
    // Keep the completed Event2 page alive across the host's render-only
    // half-step.  This is presentation-only and does not grant another PSX
    // frame/tail credit.
    if (transaction.active && transaction.directDrawCompleteWithinLimits &&
        ctx.renderOnlyFrame) {
        if (ctx.renderer) {
            float vx = 0.0f;
            float vy = 0.0f;
            float vs = 1.0f;
            SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
            (void)SS0DirectResolveStageSelect80020568(
                ctx, vx, vy, vs, true);
        }
        return;
    }
    if (!ctx.renderer ||
        !EventFrame::CanSubmitEvent2FrameDraw8001E750(
            transaction, ctx.frame, ctx.renderOnlyFrame)) {
        return;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    const bool firstTranslatedSubmit =
        !transaction.directDrawCompleteWithinLimits;
    if (!SS0DirectResolveStageSelect80020568(ctx, vx, vy, vs, true)) {
        Log::Printf(
            "SS0 direct runtime: event2 frame transaction draw rejected logicFrame=%u hostFrame=%u class=%s pending=1",
            static_cast<unsigned>(transaction.logicFrame),
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent2FrameClassName80026B94(transaction.frameClass));
        return;
    }
    if ((firstTranslatedSubmit &&
         !s_ss0Direct.stageSelectFrameTailCandidateKnown80026B94) ||
        !EventFrame::CommitEvent2FrameDrawAndBindFormalTail80026B94(
            transaction, ctx.frame, ctx.renderOnlyFrame)) {
        Log::Printf(
            "SS0 direct runtime: event2 frame transaction commit rejected frame=%u class=%s renderOnly=%d",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent2FrameClassName80026B94(transaction.frameClass),
            ctx.renderOnlyFrame ? 1 : 0);
        return;
    }
    if (firstTranslatedSubmit) {
        const bool hostBoundaryExecuted =
            SS0DirectExecuteEvent2FrameBoundary80026B94(ctx, transaction);
        if (hostBoundaryExecuted) {
            SS0DirectCommitStageSelectFrameTail80026B94();
        }
        Log::Printf(
            "SS0 direct runtime: event2 frame draw submitted frame=%u hostFrame=%u class=%s inputClosed=%d tail=%d/%d order=80026170>8001E750{8001D74C(3,workSlot=%u),80020568(80087B78)}>StageClear(80043A14 gate=800916F6 known=1 value=0 taken=0)>80035560(0)>8001EA00(2)>800436F0(-1) directDrawExecuted=1 formalTailOrderBound=1 hostWaitExecuted=%d hostEndExecuted=%d hostTextFlushExecuted=%d hostFrameBoundaryExecuted=%d exactHalParity=0",
            static_cast<unsigned>(transaction.logicFrame),
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent2FrameClassName80026B94(transaction.frameClass),
            transaction.inputClosed ? 1 : 0,
            transaction.tailFramesRemainingBefore,
            transaction.tailFramesRemainingAfter,
            static_cast<unsigned>(
                s_ss0Direct.stageSelectFrameEndFrameWorkListSlot8001EA00),
            transaction.hostWaitExecuted ? 1 : 0,
            transaction.hostEndExecuted ? 1 : 0,
            transaction.hostTextFlushExecuted ? 1 : 0,
            hostBoundaryExecuted ? 1 : 0);
    }
}

static void SS0DirectRenderStageSelectTransitionBackdrop80020568(
    PrGameContext& ctx) {
    // WaitTransition may show the already-published StageSelect page, but it
    // is visual support only: it must not create Event2 logic/tail credit.
    if (!ctx.renderer) {
        return;
    }
    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    (void)SS0DirectResolveStageSelect80020568(ctx, vx, vy, vs, true);
}

static bool SS0DirectAdvancePracticeRec44Lane80023618(
    int32_t value,
    int32_t mode,
    PrSS0DirectoryPagesRenderDirect::PracticePortraitSubmit80024600& submit) {
    submit = {};
    if (value < 0) {
        return true;
    }
    if (!PrSS0DirectoryPagesRenderDirect::UpdatePracticeWobbleBank80023F20(
            s_ss0Direct.practiceWobbleBank, value)) {
        return false;
    }
    s_ss0Direct.practiceIconRgbKnown = true;
    s_ss0Direct.practiceIconR = kPracticeIconNeutralRgb80024418;
    s_ss0Direct.practiceIconG = kPracticeIconNeutralRgb80024418;
    s_ss0Direct.practiceIconB = kPracticeIconNeutralRgb80024418;
    if (!PrSS0DirectoryPagesRenderDirect::SelectPracticePortraitTemplate800246A8(
            s_ss0Direct.practicePortraitBank, mode)) {
        return false;
    }
    PrSS0DirectoryPagesRenderDirect::PracticePortraitState80024600 state{};
    state.argumentsKnown = true;
    state.row = 0;
    state.baseX = 16;
    state.baseY = 93;
    state.maxRepeatFrame = 3;
    state.value = value;
    submit = PrSS0DirectoryPagesRenderDirect::UpdatePracticePortraitBank80024600(
        s_ss0Direct.practicePortraitBank, state);
    return submit.accepted && submit.complete;
}

static bool SS0DirectBuildPracticeIconCandidates80024418() {
    s_ss0Direct.practiceIconCandidates = {};
    s_ss0Direct.practiceIconStreamKnown = false;
    const int32_t round = s_ss0Direct.practiceRound;
    if (round < 0 ||
        round >= static_cast<int32_t>(
                     PrSS0PracticeLifecycleDirect::kPracticeRoundCount)) {
        return false;
    }
    for (int32_t slot = 0; slot < 18; ++slot) {
        const auto value =
            PrSS0PracticeLifecycleDirect::ReadPracticeRec44StreamValue(
                round, slot);
        if (!value.sourceKnown) {
            return false;
        }
        if (value.value < 1 || value.value > 8) {
            continue;
        }
        const int16_t centerX = static_cast<int16_t>(41 + 14 * slot);
        auto& candidate = s_ss0Direct.practiceIconCandidates[slot];
        if (s_ss0Direct.practiceIconRgbKnown) {
            candidate = PrSS0DirectoryPagesRenderDirect::
                BuildPracticeIconSubmitFromWobbleBankAndRgb80024418(
                    s_ss0Direct.practiceWobbleBank,
                    centerX,
                    99,
                    static_cast<int16_t>(slot),
                    static_cast<int16_t>(value.value),
                    s_ss0Direct.practiceIconR,
                    s_ss0Direct.practiceIconG,
                    s_ss0Direct.practiceIconB);
        } else {
            candidate = PrSS0DirectoryPagesRenderDirect::
                BuildPracticeIconSubmitFromWobbleBank80024418(
                    s_ss0Direct.practiceWobbleBank,
                    centerX,
                    99,
                    static_cast<int16_t>(slot),
                    static_cast<int16_t>(value.value));
        }
    }
    s_ss0Direct.practiceIconStreamKnown = true;
    return true;
}

static bool SS0DirectBuildPracticeGuideCandidate80023518() {
    PrSS0DirectoryPagesRenderDirect::PracticeGuideState80023518 state{};
    state.progressKnown = true;
    state.progress = -1;
    s_ss0Direct.practiceGuideDrawList =
        PrSS0DirectoryPagesRenderDirect::
            BuildPracticeGuideDrawList80023518(state);
    const auto& list = s_ss0Direct.practiceGuideDrawList;
    return list.sourceKnown && list.accepted && list.complete &&
        list.runtimeSubmitAllowed && !list.truncated &&
        list.count == PrSS0DirectoryPagesRenderDirect::
                          kPracticeGuideSpriteCount80023518;
}

static bool SS0DirectAdvancePracticeRec44Producers80023618() {
    if (!SS0DirectAdvancePracticeRec44Lane80023618(
            s_ss0Direct.practiceSeqCurA,
            4,
            s_ss0Direct.practicePortraitTeacher)) {
        return false;
    }
    if (!SS0DirectAdvancePracticeRec44Lane80023618(
            s_ss0Direct.practiceSeqCurB,
            0,
            s_ss0Direct.practicePortraitStudent)) {
        return false;
    }
    return true;
}

static bool SS0DirectBuildPracticeSliceCandidate8001C604() {
    PrSS0DirectoryPagesRenderDirect::PracticeSliceProducerState8001C604
        state{};
    state.eventDispatchKnown = true;
    state.eventId = kPracticeEventId8001E750;
    state.precedingPortraitDrawProducerKnown =
        s_ss0Direct.practicePortraitTeacher.runtimeDrawPublished ||
        s_ss0Direct.practicePortraitStudent.runtimeDrawPublished;
    s_ss0Direct.practiceSliceDrawList =
        PrSS0DirectoryPagesRenderDirect::
            BuildPracticeSliceDrawList8001C604(state);
    const auto& list = s_ss0Direct.practiceSliceDrawList;
    return list.sourceKnown && list.accepted && list.complete &&
        list.runtimeSubmitAllowed && !list.truncated &&
        list.count == PrSS0DirectoryPagesRenderDirect::
                          kPracticeSliceSpriteCount8001C604;
}

static bool SS0DirectBuildPracticeLeadingOverlayCandidate80023618() {
    PrSS0DirectoryPagesRenderDirect::
        PracticeLeadingOverlayProducerState80023618 state{};
    state.eventDispatchKnown = true;
    state.eventId = kPracticeEventId8001E750;
    state.languageKnown = true;
    state.language = s_ss0Direct.optionsWord800916D8;
    state.flagsKnown = true;
    state.flags00 = s_ss0Direct.practiceFlags00;
    state.overlayStateKnown = true;
    state.overlayState1C = s_ss0Direct.practiceOverlayState1C;
    s_ss0Direct.practiceLeadingOverlayDrawList =
        PrSS0DirectoryPagesRenderDirect::
            BuildPracticeLeadingOverlayDrawList80023618(state);
    const auto& list = s_ss0Direct.practiceLeadingOverlayDrawList;
    return list.sourceKnown && list.accepted && list.complete &&
        list.runtimeSubmitAllowed && !list.truncated &&
        list.count <= PrSS0DirectoryPagesRenderDirect::
                          kPracticeLeadingOverlaySpriteCapacity80023618;
}

static bool SS0DirectBuildPracticePostSliceTailCandidate80023618() {
    PrSS0DirectoryPagesRenderDirect::
        PracticePostSliceTailProducerState80023618 state{};
    state.eventDispatchKnown = true;
    state.eventId = kPracticeEventId8001E750;
    state.precedingSliceDrawPublished =
        s_ss0Direct.practiceSliceDrawList.runtimeDrawPublished;
    state.languageKnown = true;
    state.language = s_ss0Direct.optionsWord800916D8;
    state.exitSelectionKnown = true;
    state.exitSelection48 = s_ss0Direct.practiceExitSelection48;
    state.exitBlinkKnown = true;
    state.exitBlink4C =
        static_cast<int16_t>(s_ss0Direct.practiceExitBlink4C);
    s_ss0Direct.practicePostSliceTailDrawList =
        PrSS0DirectoryPagesRenderDirect::
            BuildPracticePostSliceTailDrawList80023618(state);
    const auto& list = s_ss0Direct.practicePostSliceTailDrawList;
    return list.sourceKnown && list.accepted && list.complete &&
        list.runtimeSubmitAllowed && !list.truncated &&
        list.count == PrSS0DirectoryPagesRenderDirect::
                          kPracticePostSliceTailSpriteCount80023618;
}

static bool SS0DirectBuildPracticeAlwaysVisibleTailCandidate80023618() {
    PrSS0DirectoryPagesRenderDirect::
        PracticePostSliceTailProducerState80023618 state{};
    state.eventDispatchKnown = true;
    state.eventId = kPracticeEventId8001E750;
    state.precedingSliceDrawPublished =
        s_ss0Direct.practiceSliceDrawList.runtimeDrawPublished;
    state.languageKnown = true;
    state.language = s_ss0Direct.optionsWord800916D8;
    state.exitSelectionKnown = true;
    state.exitSelection48 = s_ss0Direct.practiceExitSelection48;
    state.exitBlinkKnown = true;
    state.exitBlink4C =
        static_cast<int16_t>(s_ss0Direct.practiceExitBlink4C);
    s_ss0Direct.practicePostSliceTailDrawList =
        PrSS0DirectoryPagesRenderDirect::
            BuildPracticeAlwaysVisibleTailDrawList80023618(state);
    const auto& list = s_ss0Direct.practicePostSliceTailDrawList;
    return list.sourceKnown && list.accepted && list.complete &&
        list.runtimeSubmitAllowed && list.alwaysVisibleFixedTail &&
        !list.truncated &&
        list.count == PrSS0DirectoryPagesRenderDirect::
                          kPracticePostSliceTailSpriteCount80023618;
}

static bool SS0DirectPracticeYCompoAtlasD3DReady801C689C(
    PrGameContext& ctx) {
    if (!ctx.renderer ||
        !SS0DirectPracticeYCompoResourcesCommitted80015618()) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    return upload.attempted && upload.complete &&
        upload.HasNativeCpuAtlasAuthority() &&
        upload.sourceAtlasProfile8001AE7C ==
            PrSS0TitleTmdBackend::
                Scene0IntRendererAtlasProfile8001AE7C::PracticeYCompo &&
        upload.tpageCount > 0u && upload.failedTpageCount == 0u &&
        upload.readyTpageCount == upload.tpageCount;
}

static void SS0DirectRenderPractice(PrGameContext& ctx) {
    static int lastRenderGate = -1;
    const auto logGate = [&](int gate, const char* detail) {
        if (lastRenderGate == gate) {
            return;
        }
        lastRenderGate = gate;
        Log::Printf("SS0 direct runtime: Practice render gate=%d %s",
                    gate,
                    detail ? detail : "");
    };
    if (!SS0DirectPracticeYCompoAtlasD3DReady801C689C(ctx)) {
        const auto resourceState = PrSS0TitleTmdBackend::GetResourceState();
        const auto upload = ctx.renderer
            ? PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(
                  ctx.renderer)
            : PrSS0TitleTmdBackend::TitleVramAtlasUploadResult801C689C{};
        Log::Printf(
            "SS0 direct runtime: Practice render gate=1 resources=%d renderer=%d upload=%d/%d profile=%d cpu=%d d3d=%d tpages=%u ready=%u failed=%u",
            SS0DirectPracticeYCompoResourcesCommitted80015618() ? 1 : 0,
            ctx.renderer ? 1 : 0,
            upload.attempted ? 1 : 0,
            upload.complete ? 1 : 0,
            static_cast<int>(resourceState.scene0IntRendererAtlasProfile8001AE7C),
            resourceState.scene0IntRendererCpuAtlasCommitted8001AE7C ? 1 : 0,
            resourceState.scene0IntRendererD3DUploadCommitted8001AE7C ? 1 : 0,
            static_cast<unsigned>(upload.tpageCount),
            static_cast<unsigned>(upload.readyTpageCount),
            static_cast<unsigned>(upload.failedTpageCount));
        return;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (!SS0DirectSubmitEventBackdrop8001D74C(ctx, vx, vy, vs)) {
        logGate(2, "backdrop submit rejected");
        return;
    }
    const bool leading = SS0DirectBuildPracticeLeadingOverlayCandidate80023618();
    // The PSX event loop calls 80023618 from the 30 Hz event16 tick.  At
    // 60 Hz the second pass is render-only; it must consume the already-built
    // portrait/wobble state instead of advancing Rec44 again.  Otherwise the
    // two passes race through the stream and visibly drop poses/expressions.
    bool producers = true;
    if (!ctx.renderOnlyFrame) {
        const int logicFrame = static_cast<int>(ctx.frame);
        if (s_ss0Direct.practiceProducerLastLogicFrame != logicFrame) {
            producers = SS0DirectAdvancePracticeRec44Producers80023618();
            if (producers) {
                s_ss0Direct.practiceProducerLastLogicFrame = logicFrame;
            }
        }
    }
    const bool guide = SS0DirectBuildPracticeGuideCandidate80023518();
    const bool icons = SS0DirectBuildPracticeIconCandidates80024418();
    if (!leading || !producers || !guide || !icons) {
        Log::Printf(
            "SS0 direct runtime: Practice render gate=3 leading=%d producers=%d guide=%d icons=%d leadCount=%u guideCount=%u iconStream=%d",
            leading ? 1 : 0,
            producers ? 1 : 0,
            guide ? 1 : 0,
            icons ? 1 : 0,
            static_cast<unsigned>(s_ss0Direct.practiceLeadingOverlayDrawList.count),
            static_cast<unsigned>(s_ss0Direct.practiceGuideDrawList.count),
            s_ss0Direct.practiceIconStreamKnown ? 1 : 0);
        return;
    }
    const bool leadingSubmitted =
        SS0DirectSubmitPracticeLeadingOverlay80023618(ctx, vx, vy, vs);
    const bool portraitsSubmitted =
        SS0DirectSubmitPracticePortraits80024600(ctx, vx, vy, vs);
    const bool guideSubmitted = SS0DirectSubmitPracticeGuide80023518(ctx, vx, vy, vs);
    const bool iconsSubmitted = SS0DirectSubmitPracticeIcons80024418(ctx, vx, vy, vs);
    if (!leadingSubmitted || !portraitsSubmitted || !guideSubmitted ||
        !iconsSubmitted) {
        Log::Printf(
            "SS0 direct runtime: Practice render gate=4 leadingSubmit=%d portraits=%d guideSubmit=%d iconsSubmit=%d",
            leadingSubmitted ? 1 : 0,
            portraitsSubmitted ? 1 : 0,
            guideSubmitted ? 1 : 0,
            iconsSubmitted ? 1 : 0);
    }
    bool slicesSubmitted = false;
    if (portraitsSubmitted &&
        SS0DirectBuildPracticeSliceCandidate8001C604()) {
        slicesSubmitted = SS0DirectSubmitPracticeSlices8001C604(
            ctx, vx, vy, vs);
    } else {
        s_ss0Direct.practiceSliceDrawList = {};
    }

    // The source 80023618 fixed tail is emitted every Practice frame, even
    // while the Rec44 lanes are still disabled during RoundIntro/Preview. Use
    // the legacy post-slice transaction when its preceding slice really
    // exists; otherwise use the same fixed/caption/EXIT records through the
    // always-visible source gate.
    if (slicesSubmitted &&
        SS0DirectBuildPracticePostSliceTailCandidate80023618() &&
        SS0DirectSubmitPracticePostSliceTail80023618(ctx, vx, vy, vs)) {
        return;
    }
    if (!SS0DirectBuildPracticeAlwaysVisibleTailCandidate80023618() ||
        !SS0DirectSubmitPracticePostSliceTail80023618(ctx, vx, vy, vs)) {
        s_ss0Direct.practicePostSliceTailDrawList = {};
    }
}

static bool SS0DirectExecuteEvent6FrameBoundary80026B94(
    PrGameContext& ctx,
    PrSS0EventFrameLoopDirect::Event6FrameTransaction80026B94& transaction) {
    if (!transaction.active || !transaction.directDrawSubmitted ||
        !transaction.formalTailOrderBound) {
        return false;
    }

    auto& hostFrame = s_ss0Direct.hiScoreFrameHostState8001E750;
    bool hostWaitExecuted = transaction.hostWaitExecuted;
    if (!hostWaitExecuted) {
        auto waitFrame =
            PrPsxEventFrameDirect::PsxCall80035560_WaitFrameDetailed(
                hostFrame, PrPsxVSyncDirect::ProcessVSyncState80035560(), 0);
        if (!waitFrame.softwareWaitComplete) {
            waitFrame = PrPsxEventFrameDirect::
                PsxConsume80035560_WaitFrameHostVblankDetailed(
                    hostFrame,
                    PrPsxVSyncDirect::ProcessVSyncState80035560(),
                    1);
        }
        hostWaitExecuted =
            waitFrame.sourceKnown && waitFrame.softwareStateCommitted &&
            waitFrame.softwareWaitComplete &&
            waitFrame.mode ==
                PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
    }

    bool hostEndExecuted = transaction.hostEndExecuted;
    if (hostWaitExecuted && !hostEndExecuted &&
        s_ss0Direct.hiScoreFrameEndFrameBindingKnown8001EA00) {
        hostFrame.gp368WorkSlot =
            s_ss0Direct.hiScoreFrameEndFrameWorkListSlot8001EA00;
        const auto endFrame =
            PrPsxEventFrameDirect::PsxCall8001EA00_EndFrameDetailed(
                hostFrame, 6);
        hostEndExecuted = endFrame.completeWithinLimits;
    }

    bool hostTextFlushExecuted = transaction.hostTextFlushExecuted;
    if (hostWaitExecuted && hostEndExecuted && !hostTextFlushExecuted) {
        hostTextFlushExecuted = SS0DirectFlushEventText800436F0(ctx);
    }
    if (!hostEndExecuted &&
        !s_ss0Direct.hiScoreFrameEndFrameBindingBlockedLogged8001EA00) {
        s_ss0Direct.hiScoreFrameEndFrameBindingBlockedLogged8001EA00 = true;
        Log::Printf(
            "SS0 direct runtime: event6 host boundary blocked by missing formal work-list binding frame=%u",
            static_cast<unsigned>(ctx.frame));
    }
    return PrSS0EventFrameLoopDirect::CommitEvent6FrameHostBoundary80026B94(
        transaction,
        hostWaitExecuted,
        hostEndExecuted,
        hostTextFlushExecuted);
}

static void SS0DirectRenderHiScore(PrGameContext& ctx) {
    namespace EventFrame = PrSS0EventFrameLoopDirect;
    (void)SS0DirectEnsureEventTextRuntime(ctx);
    auto& transaction = s_ss0Direct.hiScoreFrameTransaction80026B94;
    if (transaction.directDrawCompleteWithinLimits &&
        !transaction.hostFrameBoundaryExecuted) {
        if (SS0DirectExecuteEvent6FrameBoundary80026B94(ctx, transaction)) {
            Log::Printf(
                "SS0 direct runtime: event6 deferred host boundary committed frame=%u hostFrame=%u hostWaitExecuted=1 hostEndExecuted=1 hostTextFlushExecuted=1 hostFrameBoundaryExecuted=1 exactHalParity=0",
                static_cast<unsigned>(transaction.logicFrame),
                static_cast<unsigned>(ctx.frame));
        }
        // Event6 keeps the table page visible until its translated formal
        // boundary completes.  Redraw the same table on that host frame to
        // avoid exposing the process-level clear colour between pages.
        if (ctx.renderer) {
            float vx = 0.0f;
            float vy = 0.0f;
            float vs = 1.0f;
            SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
            (void)SS0DirectResolveHiScoreTable80021594(
                ctx, vx, vy, vs, true);
        }
        return;
    }
    // Event6's table remains the visible source until the translated tail is
    // consumed.  Re-submit it on every 60 Hz render-only frame so the host
    // clear cannot appear for a single frame.
    if (transaction.active && transaction.directDrawCompleteWithinLimits &&
        ctx.renderOnlyFrame) {
        if (ctx.renderer) {
            float vx = 0.0f;
            float vy = 0.0f;
            float vs = 1.0f;
            SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
            (void)SS0DirectResolveHiScoreTable80021594(
                ctx, vx, vy, vs, true);
        }
        return;
    }
    if (!ctx.renderer ||
        !EventFrame::CanSubmitEvent6FrameDraw8001E750(
            transaction, ctx.frame, ctx.renderOnlyFrame)) {
        return;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    const bool firstTranslatedSubmit =
        !transaction.directDrawCompleteWithinLimits;
    if (!SS0DirectResolveHiScoreTable80021594(
            ctx, vx, vy, vs, true)) {
        Log::Printf(
            "SS0 direct runtime: event6 frame transaction draw rejected logicFrame=%u hostFrame=%u class=%s pending=1",
            static_cast<unsigned>(transaction.logicFrame),
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent6FrameClassName80026B94(
                transaction.frameClass));
        return;
    }
    if (!EventFrame::CommitEvent6FrameDrawAndBindFormalTail80026B94(
            transaction, ctx.frame, ctx.renderOnlyFrame)) {
        Log::Printf(
            "SS0 direct runtime: event6 frame transaction commit rejected frame=%u class=%s renderOnly=%d",
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent6FrameClassName80026B94(
                transaction.frameClass),
            ctx.renderOnlyFrame ? 1 : 0);
        return;
    }
    if (firstTranslatedSubmit) {
        const bool hostBoundaryExecuted =
            SS0DirectExecuteEvent6FrameBoundary80026B94(ctx, transaction);
        Log::Printf(
            "SS0 direct runtime: event6 frame draw submitted frame=%u hostFrame=%u class=%s inputClosed=%d tail=%d/%d order=80025E48>8001E750{8001D74C(3,workSlot=%u),80021594(80049278)}>80035560(0)>8001EA00(6)>800436F0(-1) directDrawExecuted=1 formalTailOrderBound=1 hostWaitExecuted=%d hostEndExecuted=%d hostTextFlushExecuted=%d hostFrameBoundaryExecuted=%d exactHalParity=0",
            static_cast<unsigned>(transaction.logicFrame),
            static_cast<unsigned>(ctx.frame),
            SS0DirectEvent6FrameClassName80026B94(
                transaction.frameClass),
            transaction.inputClosed ? 1 : 0,
            transaction.tailFramesRemainingBefore,
            transaction.tailFramesRemainingAfter,
            static_cast<unsigned>(
                s_ss0Direct.hiScoreFrameEndFrameWorkListSlot8001EA00),
            transaction.hostWaitExecuted ? 1 : 0,
            transaction.hostEndExecuted ? 1 : 0,
            transaction.hostTextFlushExecuted ? 1 : 0,
            hostBoundaryExecuted ? 1 : 0);
    }
}

enum class SS0DirectEventFramePacketKind8001E750 : uint8_t {
    None = 0,
    FastSprite8003FA20,
    BoxFill8003EE84,
};

struct SS0DirectEventFramePacket8001E750 {
    SS0DirectEventFramePacketKind8001E750 kind =
        SS0DirectEventFramePacketKind8001E750::None;
    D3D11Renderer::SpriteCmd sprite{};
    D3D11Renderer::SolidRectCmd boxFill{};
};

static bool SS0DirectIsEventFrameFastSpriteSource8003FA20(
    PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20 source) {
    using Source =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20;
    return source == Source::Stage1EventFramePrompt ||
           source == Source::Stage1EventFrameBackdrop ||
           source == Source::Stage1EventFrameSaveUi;
}

static bool SS0DirectResolveEventFrameFastSprite8003FA20(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750& frame,
    const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write,
    uint32_t psxCallOrder,
    float vx,
    float vy,
    float vs,
    D3D11Renderer::SpriteCmd& out) {
    if (!ctx.renderer || !write.valid ||
        write.wordCount != PrPsxFastSpriteSubmitDirect::
                               kGsSortFastSpritePacketWordCount8003FA20 ||
        !write.provenance.active ||
        !SS0DirectIsEventFrameFastSpriteSource8003FA20(
            write.provenance.sourceKind)) {
        return false;
    }
    for (bool known : write.wordKnown) {
        if (!known) {
            return false;
        }
    }

    const uint8_t packetCommand = static_cast<uint8_t>(
        (write.words[2] >> 24u) & 0xFFu);
    if ((write.word2CommandKnown &&
         write.word2CommandCode != packetCommand) ||
        (packetCommand & 0x01u) == 0u) {
        return false;
    }
    const uint16_t width = static_cast<uint16_t>(
        write.words[5] & 0xFFFFu);
    const uint16_t height = static_cast<uint16_t>(
        (write.words[5] >> 16u) & 0xFFFFu);
    if (width == 0u || height == 0u) {
        return false;
    }

    const uint16_t tpage = static_cast<uint16_t>(
        write.words[1] & 0x01FFu);
    const uint16_t clut = static_cast<uint16_t>(
        (write.words[4] >> 16u) & 0xFFFFu);
    const auto texture =
        PrSS0TitleTmdBackend::ResolveTitleTpageSRVExact801C689C(
            ctx.renderer, tpage, clut);
    if (!texture.HasNativeCpuAtlasAuthority() ||
        !texture.tpageClutResolvable || !texture.clutStpKnown ||
        texture.srv == nullptr) {
        return false;
    }

    out.texture = texture.srv;
    out.blend = D3D11Renderer::BlendMode::Alpha;
    const bool semiTransparent = (packetCommand & 0x02u) != 0u;
    const uint8_t abr = static_cast<uint8_t>(
        (write.words[1] >> 5u) & 0x03u);
    if (semiTransparent && texture.clutHasStpBits) {
        if (abr == 0u && texture.psxAbr0StpSrv != nullptr) {
            out.texture = texture.psxAbr0StpSrv;
        } else if (abr == 1u && texture.psxAbr1StpSrv != nullptr) {
            out.texture = texture.psxAbr1StpSrv;
            out.blend = D3D11Renderer::BlendMode::PsxAbr1Stp;
        } else {
            return false;
        }
    }

    int16_t x = static_cast<int16_t>(write.words[3] & 0xFFFFu);
    int16_t y = static_cast<int16_t>(
        (write.words[3] >> 16u) & 0xFFFFu);
    if (frame.graph.drawOffset.setDrawEnvCalled) {
        x = static_cast<int16_t>(
            x + frame.graph.drawOffset.word_80091738);
        y = static_cast<int16_t>(
            y + frame.graph.drawOffset.word_8009173A);
    }
    const uint8_t u = static_cast<uint8_t>(
        write.words[4] & 0xFFu);
    const uint8_t v = static_cast<uint8_t>(
        (write.words[4] >> 8u) & 0xFFu);
    out.x = vx + static_cast<float>(x) * vs;
    out.y = vy + static_cast<float>(y) * vs;
    out.w = static_cast<float>(width) * vs;
    out.h = static_cast<float>(height) * vs;
    out.u0 = (static_cast<float>(u) + 0.5f) / 256.0f;
    out.v0 = (static_cast<float>(v) + 0.5f) / 256.0f;
    out.u1 = width <= 1u
        ? out.u0
        : (static_cast<float>(u) + static_cast<float>(width) - 0.5f) /
              256.0f;
    out.v1 = height <= 1u
        ? out.v0
        : (static_cast<float>(v) + static_cast<float>(height) - 0.5f) /
              256.0f;
    out.r = 1.0f;
    out.g = 1.0f;
    out.b = 1.0f;
    out.a = 1.0f;
    out.layer = SS0DirectRawSpriteLayer8003FA20(
        write.provenance.priority);
    out.order = (uint64_t{1} << 32u) |
                static_cast<uint64_t>(psxCallOrder + 1u);
    return true;
}

static bool SS0DirectResolveEventFrameBoxFill8003EE84(
    const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write,
    uint32_t psxCallOrder,
    float vx,
    float vy,
    float vs,
    D3D11Renderer::SolidRectCmd& out) {
    if (!write.valid || write.wordCount != 5u ||
        !write.provenance.active ||
        write.provenance.sourceFunction !=
            PrPsxEventFrameDirect::kFn8003EE84_GsSortBoxFill) {
        return false;
    }
    for (uint32_t i = 0u; i < 5u; ++i) {
        if (!write.wordKnown[i]) {
            return false;
        }
    }
    const uint16_t width = static_cast<uint16_t>(
        write.words[4] & 0xFFFFu);
    const uint16_t height = static_cast<uint16_t>(
        (write.words[4] >> 16u) & 0xFFFFu);
    if (width == 0u || height == 0u) {
        return false;
    }

    const int16_t x = static_cast<int16_t>(
        write.words[3] & 0xFFFFu);
    const int16_t y = static_cast<int16_t>(
        (write.words[3] >> 16u) & 0xFFFFu);
    const uint32_t colorCode = write.words[2];
    const uint8_t commandCode = static_cast<uint8_t>(
        (colorCode >> 24u) & 0xFFu);
    out.x = vx + static_cast<float>(x) * vs;
    out.y = vy + static_cast<float>(y) * vs;
    out.w = static_cast<float>(width) * vs;
    out.h = static_cast<float>(height) * vs;
    out.r = static_cast<float>(colorCode & 0xFFu) / 255.0f;
    out.g = static_cast<float>((colorCode >> 8u) & 0xFFu) / 255.0f;
    out.b = static_cast<float>((colorCode >> 16u) & 0xFFu) / 255.0f;
    out.a = (commandCode & 0x02u) != 0u ? 0.5f : 1.0f;
    out.layer = SS0DirectRawSpriteLayer8003FA20(
        write.provenance.priority);
    out.order = (uint64_t{1} << 32u) |
                static_cast<uint64_t>(psxCallOrder + 1u);
    return true;
}

static bool SS0DirectSubmitEventFramePackets8001E750(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750& frame) {
    using PacketWrite =
        PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20;
    constexpr std::size_t kCapacity =
        PrPsxFastSpriteSubmitDirect::
            kGsSortFastSpriteRuntimePacketWriteCapacity8003FA20;
    if (!ctx.renderer || !frame.initialized ||
        !frame.graph.mainPageWorkLists80087288Initialized) {
        return false;
    }
    const uint8_t pageIndex = static_cast<uint8_t>(
        frame.gp368WorkSlot & 1u);
    const auto& work =
        frame.graph.mainPageWorkLists80087288[pageIndex].work;
    if (!work.packetWriteMirrorKnown) {
        return false;
    }
    std::array<const PacketWrite*, kCapacity> writes{};
    std::size_t writeCount = 0u;
    for (const PacketWrite& write : work.packetWriteMirror) {
        if (!write.valid) {
            continue;
        }
        if (writeCount >= writes.size()) {
            return false;
        }
        writes[writeCount++] = &write;
    }
    if (writeCount == 0u) {
        return false;
    }
    const auto upload =
        PrSS0TitleTmdBackend::PrepareTitleVramAtlas801C689C(ctx.renderer);
    if (!upload.complete || !upload.HasNativeCpuAtlasAuthority()) {
        return false;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    std::array<SS0DirectEventFramePacket8001E750, kCapacity> resolved{};
    std::size_t resolvedCount = 0u;
    uint32_t psxCallOrder = 1u;
    for (std::size_t i = writeCount; i > 0u; --i, ++psxCallOrder) {
        const PacketWrite& write = *writes[i - 1u];
        auto& command = resolved[resolvedCount];
        if (write.wordCount == 5u &&
            write.provenance.sourceFunction ==
                PrPsxEventFrameDirect::kFn8003EE84_GsSortBoxFill) {
            command.kind =
                SS0DirectEventFramePacketKind8001E750::BoxFill8003EE84;
            if (!SS0DirectResolveEventFrameBoxFill8003EE84(
                    write, psxCallOrder, vx, vy, vs, command.boxFill)) {
                return false;
            }
        } else {
            command.kind =
                SS0DirectEventFramePacketKind8001E750::FastSprite8003FA20;
            if (!SS0DirectResolveEventFrameFastSprite8003FA20(
                    ctx,
                    frame,
                    write,
                    psxCallOrder,
                    vx,
                    vy,
                    vs,
                    command.sprite)) {
                return false;
            }
        }
        ++resolvedCount;
    }

    for (std::size_t i = 0u; i < resolvedCount; ++i) {
        const auto& command = resolved[i];
        if (command.kind ==
            SS0DirectEventFramePacketKind8001E750::FastSprite8003FA20) {
            ctx.renderer->SubmitSprite(command.sprite);
        } else if (command.kind ==
                   SS0DirectEventFramePacketKind8001E750::BoxFill8003EE84) {
            ctx.renderer->SubmitSolidRect(command.boxFill);
        } else {
            return false;
        }
    }
    return true;
}

static bool SS0DirectRenderLoadReplayInsertCardPrompt8001E750(
    PrGameContext& ctx) {
    const auto& runtime = s_ss0Direct.cardDriverVisualRuntime80018FB0;
    const bool state5Phase =
        runtime.state == 5 &&
        (runtime.phase ==
             CardHandoff::CardDriverVisualPhase80018FB0::
                 State5PromptIdle800180D8 ||
         runtime.phase ==
             CardHandoff::CardDriverVisualPhase80018FB0::
                 SelectionFlash80017E6C);
    const bool state23Phase =
        runtime.state == 23 &&
        (runtime.phase ==
             CardHandoff::CardDriverVisualPhase80018FB0::
                 TerminalFrame80018FB0 ||
         runtime.phase ==
             CardHandoff::CardDriverVisualPhase80018FB0::
                 FinalFlash80017E6C);
    if (!ctx.renderer || !runtime.known ||
        runtime.eventId != CardHandoff::CardEventFrameId::InsertCardPrompt ||
        (!state5Phase && !state23Phase) || runtime.gp720Known ||
        runtime.gp720 != 0 || runtime.exitFrameStateArg0 < 0 ||
        runtime.exitFrameStateArg0 > 1 || runtime.exitBlinkStateArg4 < -1 ||
        runtime.exitBlinkStateArg4 > 1 || runtime.cardIoFlagArg8 != 0 ||
        s_ss0Direct.optionsWord800916D8 < 0 ||
        s_ss0Direct.optionsWord800916D8 > 4) {
        return false;
    }

    auto& frame = s_ss0Direct.cardPromptEventFrame8001E750;
    frame = {};
    const bool frameCloseBlocked =
        PrPsxEventFrameDirect::PsxCall8001E750_SaveUiEventFrame(
            frame,
            static_cast<int32_t>(
                CardHandoff::CardEventFrameId::InsertCardPrompt),
            runtime.exitFrameStateArg0,
            runtime.exitBlinkStateArg4,
            runtime.cardIoFlagArg8,
            s_ss0Direct.optionsWord800916D8,
            nullptr);
    if (frameCloseBlocked) {
        frame = {};
        return false;
    }

    if (!SS0DirectSubmitEventFramePackets8001E750(ctx, frame)) {
        frame = {};
        return false;
    }
    return true;
}

static void SS0DirectRenderCardPage(PrGameContext& ctx, int32_t eventId) {
    if (!ctx.renderer || (eventId != 8 && eventId != 9)) {
        return;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (s_ss0Direct.cardDriverVisualRuntime80018FB0.eventId ==
        CardHandoff::CardEventFrameId::InsertCardPrompt) {
        if (SS0DirectRenderLoadReplayInsertCardPrompt8001E750(ctx)) {
            return;
        }
        (void)SS0DirectSubmitEventBackdrop8001D74C(ctx, vx, vy, vs);
        return;
    }
    if (SS0DirectResolveCardGrid80020F94(
            ctx, vx, vy, vs, eventId, true)) {
        return;
    }
    (void)SS0DirectSubmitEventBackdrop8001D74C(ctx, vx, vy, vs);
}

static void SS0DirectRenderSceneHandoffTransition80020110(
    PrGameContext& ctx) {
    const auto& visual = s_ss0Direct.sceneHandoffTransitionVisual80020110;
    if (!visual.known) {
        return;
    }
    if (!SS0DirectRenderSlowTransitionTiles8001F5248001FDC0(
            ctx, visual)) {
        if (!s_ss0Direct.sceneHandoffTransitionPresentBlockedLogged80020110) {
            s_ss0Direct.sceneHandoffTransitionPresentBlockedLogged80020110 = true;
            Log::Printf(
                "SS0 direct runtime: scene transition mode=1 render blocked iteration=%u tail=%d target=%d",
                visual.iteration,
                visual.tail ? 1 : 0,
                s_ss0Direct.sceneHandoffTargetScene);
        }
    }
}

static void SS0DirectRender(PrGameContext& ctx) {
    if (s_ss0Direct.phase == SS0DirectPhase::MainMenuResourceLoading) {
        if (SS0DirectRenderCompletedLoadingPattern8001EF40(ctx) && s_residentDirectory) {
            PrStage1LoadingDirect::RecordPresentation(
                s_residentDirectory->loader.bootstrap15590Loading, ctx.frame);
        }
        return;
    }
    if (s_ss0Direct.phase ==
        SS0DirectPhase::OpeningMovie0InitialTransition) {
        if (ctx.renderer) {
            ctx.renderer->DrawRect(
                0.0f,
                0.0f,
                static_cast<float>(ctx.renderer->GetWidth()),
                static_cast<float>(ctx.renderer->GetHeight()),
                0.0f,
                0.0f,
                0.0f,
                1.0f);
        }
        const bool tilesSubmitted =
            SS0DirectRenderSlowTransitionTiles8001F5248001FDC0(
                ctx, s_ss0Direct.openingMovie0InitialTransitionVisual);
        const bool patternVisible =
            s_ss0Direct.loadingPatternRuntime8001EF40.active ||
            SS0DirectLoadingMinimumHoldActive(
                SS0DirectLoadingHoldKind::OpeningMovie0Initial);
        const bool patternSubmitted = patternVisible
            ? SS0DirectRenderCompletedLoadingPattern8001EF40(ctx)
            : true;
        if ((!tilesSubmitted || (patternVisible && !patternSubmitted)) &&
            !s_ss0Direct.loadingPatternRenderBlockedLogged8001EF40) {
            s_ss0Direct.loadingPatternRenderBlockedLogged8001EF40 = true;
            Log::Printf(
                "SS0 direct runtime: initial Loading 8001EF40 render blocked tiles=%d pattern=%d",
                tilesSubmitted ? 1 : 0,
                patternSubmitted ? 1 : 0);
        }
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleIntroTransition) {
        // 801C4894 starts MOVIE0T before entering the synchronous
        // 80020110(mode=1) grid transition, but the title frame/event loop
        // (801C6410/801C689C) does not run until that transition returns.
        // Keep this page white and let the native spiral grid own the pixels;
        // presenting the started STR here leaks its black video frame through
        // the cleared cells and produces the mixed/black title-intro screen.
        if (ctx.renderer) {
            ctx.renderer->DrawRect(
                0.0f,
                0.0f,
                static_cast<float>(ctx.renderer->GetWidth()),
                static_cast<float>(ctx.renderer->GetHeight()),
                1.0f,
                1.0f,
                1.0f,
                1.0f);
        }
        (void)SS0DirectRenderSlowTransitionTiles8001F5248001FDC0(
            ctx, s_ss0Direct.titleIntroTransitionVisual);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleMovie0TFinalReady) {
        // During the 801C4968 pre-loop the pseudo-C submits a title packet
        // frame and immediately presents it after the two-VBlank wait.  Let
        // the direct renderer own that present model; once 8001A750 is ready,
        // EnterNaturalTitleLoop changes the phase and the normal MOVIE0T
        // branch below takes over.
        SS0DirectRenderTitle(
            ctx, s_ss0Direct.titleMovie0TFinalReadyPreloopActive);
        return;
    }
    if (s_ss0Direct.phase ==
            SS0DirectPhase::OpeningMovie0PreTransition ||
        s_ss0Direct.phase ==
        SS0DirectPhase::OpeningMovie0PostTransition) {
        SS0DirectRenderOpeningMovie0Transition80040420(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::OpeningMovie0) {
        SS0DirectRenderOpeningMovie0(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleExitTransition) {
        SS0DirectRenderTitle(ctx, false);
        (void)SS0DirectRenderSlowTransitionTiles8001F5248001FDC0(
            ctx, s_ss0Direct.titleExitTransitionVisual);
        // 801C4DC4's mode-2 close has filled the role grid before the
        // 80015788 -> 80015590 loading callback runs.  The completed-page
        // hold owns 8001EF40 here; advancing it in Fn2 without submitting
        // it left a static grid for the entire loading interval.  Its
        // inactive guard keeps the text out of the preceding transition.
        (void)SS0DirectRenderCompletedLoadingPattern8001EF40(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::MainMenuEntryTransition) {
        if (ctx.renderer &&
            s_ss0Direct.mainMenuEntryTransitionVisual.known) {
            float vx = 0.0f;
            float vy = 0.0f;
            float vs = 1.0f;
            SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
            // 8001E750's event3 preamble rebuilds the 80021E60 directory
            // before the mode-4 80020110 role-grid draw.  Both routes use
            // priority zero in the original OT.  Submit the directory first
            // and the active grid second so the grid's later OT links cover
            // the menu while the rotation is in progress; submitting them in
            // the opposite order makes the menu appear on top and hides the
            // transition.
            (void)SS0DirectResolveMainDirectory80021E60(
                ctx, s_ss0Direct.mainMenuState, vx, vy, vs, true, 3u,
                true);
            (void)SS0DirectRenderSlowTransitionTiles8001F5248001FDC0(
                ctx, s_ss0Direct.mainMenuEntryTransitionVisual);
        }
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::MainMenu) {
        SS0DirectRenderMainMenu(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::WaitTransition &&
        (s_ss0Direct.transitionReturnPhase == SS0DirectPhase::MainMenu ||
         s_ss0Direct.transitionReturnPhase == SS0DirectPhase::Practice ||
         s_ss0Direct.transitionReturnPhase == SS0DirectPhase::ReplayCard ||
         s_ss0Direct.transitionReturnPhase == SS0DirectPhase::LoadCard)) {
        SS0DirectRenderMainMenuTransitionBackdrop80021E60(ctx);
        SS0DirectRenderSceneHandoffTransition80020110(ctx);
        (void)SS0DirectRenderCompletedLoadingPattern8001EF40(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::Options) {
        SS0DirectRenderOptions(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::StageSelect) {
        SS0DirectRenderStageSelect(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::Practice) {
        SS0DirectRenderPractice(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::HiScore) {
        SS0DirectRenderHiScore(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::ReplayCard) {
        SS0DirectRenderCardPage(ctx, 9);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::LoadCard) {
        SS0DirectRenderCardPage(ctx, 8);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::WaitTransition &&
        s_ss0Direct.transitionReturnPhase == SS0DirectPhase::Options) {
        (void)SS0DirectRenderOptionsVisualOnly(ctx);
        (void)SS0DirectRenderCompletedLoadingPattern8001EF40(ctx);
        return;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::WaitTransition &&
        s_ss0Direct.transitionReturnPhase == SS0DirectPhase::StageSelect) {
        SS0DirectRenderStageSelectTransitionBackdrop80020568(ctx);
        SS0DirectRenderSceneHandoffTransition80020110(ctx);
        (void)SS0DirectRenderCompletedLoadingPattern8001EF40(ctx);
        return;
    }
    SS0DirectRenderTitle(ctx);
}

static uint32_t SS0DirectComputeSubtitleStageFrame() {
    return s_ss0Direct.strSubtitleQueryFrame30;
}

static bool SS0DirectRenderSubtitleText(PrGameContext& ctx,
                                        const char* text,
                                        float vx,
                                        float vy,
                                        float vs) {
    if (!ctx.renderer || !ctx.resources || ctx.subtitleFlag == 0 ||
        text == nullptr || *text == '\0') {
        return false;
    }

    static constexpr float kOriginX = 24.0f;
    static constexpr float kOriginY = 184.0f;
    static constexpr float kBodyWidth = 264.0f;
    static constexpr float kLineStep = 15.0f;
    const char* lineStart = text;
    std::size_t lineIndex = 0u;
    bool submitted = false;
    while (lineStart != nullptr) {
        const char* lineEnd = std::strchr(lineStart, '\n');
        const std::size_t length =
            (lineEnd != nullptr) ? static_cast<std::size_t>(lineEnd - lineStart)
                                 : std::strlen(lineStart);
        std::string line(lineStart, length);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            const float lineWidth =
                PrUiOverlay::MeasureSubtitleTextNative(1.0f, line.c_str());
            const float lineX =
                kOriginX + static_cast<float>(
                               static_cast<int>(kBodyWidth - lineWidth) / 2);
            const float lineY =
                kOriginY + kLineStep * static_cast<float>(lineIndex);
            submitted =
                PrUiOverlay::DrawSubtitleTextNative(
                    ctx,
                    vx + lineX * vs,
                    vy + lineY * vs,
                    vs,
                    line.c_str(),
                    1.0f,
                    1.0f,
                    1.0f,
                    1.0f) ||
                submitted;
        }
        if (lineEnd == nullptr) {
            break;
        }
        lineStart = lineEnd + 1;
        ++lineIndex;
    }
    return submitted;
}


} // namespace

bool RuntimeEnabled() {
    return PrSS0Direct::RuntimeCutoverAllowed();
}

const PrSS0Scene0IntLoadDirect::Transaction8001AC18*
GetSharedStartupCommonIntLoad80016B84() {
    const auto& common = s_ss0Direct.startupCommonIntLoad80016B84;
    return PrSS0Scene0IntLoadDirect::IsExactAcceptedStartupCommonTransaction80016B84(common)
        ? &common : nullptr;
}

bool PrepareStartupDiscIntBeforeBootLogo80016B84(PrGameContext& ctx) {
    if (!RuntimeEnabled()) {
        return false;
    }
    if (!s_ss0Direct.bootStartupAudioBindingsCommitted80016C8C) {
        const auto bindings =
            PrSS0ResourceAudioDirect::BuildStartupAudioBindings80016C8C();
        if (!PrSS0ResourceAudioDirect::
                IsExactStartupAudioBindings80016C8C(bindings)) {
            Log::Printf(
                "SS0 direct runtime: pre-logo 80016C8C startup audio bindings blocked");
            return false;
        }
        s_ss0Direct.bootStartupAudioBindings80016C8C = bindings;
        s_ss0Direct.bootStartupAudioBindingsCommitted80016C8C = true;
        Log::Printf(
            "SS0 direct runtime: pre-logo 80016C8C startup audio bindings committed slots=13 cue10=%08X cue1C=%08X return=%08X directSoftwareStateAuthority=1 physicalSpuAuthority=0",
            bindings.slot80094410,
            bindings.slot8009441C,
            bindings.returnValue);
    }
    if (s_ss0Direct.bootStartupDiscIntPrepared80016B84) {
        return s_ss0Direct.bootStartupAudioBindingsCommitted80016C8C &&
               PrSS0ResourceAudioDirect::
                   IsExactStartupAudioBindings80016C8C(
                       s_ss0Direct.bootStartupAudioBindings80016C8C) &&
               PrSS0Scene0ResourceIngressDirect::
                   IsExactAcceptedStartupPreloadTransaction80016B84(
                       s_ss0Direct.bootStartupPreloadIngress80016B84) &&
               PrSS0Scene0IntLoadDirect::
                   IsExactAcceptedStartupCommonTransaction80016B84(
                       s_ss0Direct.bootStartupCommonIntLoad80016B84) &&
               PrSS0Scene0IntLoadDirect::
                IsExactAcceptedZCompoTransaction80015590(
                        s_ss0Direct.bootStartupZCompoIntLoad80016B84) &&
               s_ss0Direct.bootStartupVabCloseCommitted80027120 &&
               s_ss0Direct.bootStartupVabClose80027120.committed &&
               s_ss0Direct.bootStartupVabClose80027120.function ==
                   0x80027120u &&
               s_ss0Direct.bootStartupVabClose80027120.hostProjectionCleared &&
               !s_ss0Direct.bootStartupVabClose80027120.psxSpuRegisterAuthority &&
               !s_ss0Direct.bootStartupVabClose80027120.psxInterruptTimingAuthority &&
               s_ss0Direct.bootStartupAudioResetCommitted80026FA4 &&
               s_ss0Direct.bootStartupAudioReset80026FA4.committed;
    }

    PrSS0Scene0ResourceIngressDirect::StartupPreloadSource80016B84 source{};
    source.dataRootKnown = !ctx.dataRoot.empty();
    source.dataRoot = ctx.dataRoot;
    auto ingress = PrSS0Scene0ResourceIngressDirect::
        BuildStartupPreloadTransaction80016B84(source);
    if (!PrSS0Scene0ResourceIngressDirect::
            IsExactAcceptedStartupPreloadTransaction80016B84(ingress)) {
        Log::Printf(
            "SS0 direct runtime: pre-logo 80016B84 disc ingress blocked status=%u frame=%u",
            static_cast<unsigned>(ingress.status),
            static_cast<unsigned>(ctx.frame));
        return false;
    }

    PrSS0Scene0IntLoadDirect::StartupCommonSource80016B84 commonSource{};
    commonSource.ingressKnown = true;
    commonSource.ingressAccepted = true;
    commonSource.discBinPathKnown = ingress.discBinPathKnown;
    commonSource.discBinPath = ingress.discBinPath;
    commonSource.commonRowKnown = ingress.commonRow.known;
    commonSource.commonRow = ingress.commonRow;
    PrSS0Scene0IntLoadDirect::Transaction8001AC18 common{};
    uint32_t commonAttempts = 0u;
    for (; commonAttempts < 4u; ++commonAttempts) {
        common = PrSS0Scene0IntLoadDirect::
            BuildStartupCommonTransaction80016B84(commonSource);
        if (PrSS0Scene0IntLoadDirect::
                IsExactAcceptedStartupCommonTransaction80016B84(common)) {
            ++commonAttempts;
            break;
        }
    }

    PrSS0Scene0IntLoadDirect::ZCompoSource80015590 zSource{};
    zSource.ingressKnown = true;
    zSource.ingressAccepted = true;
    zSource.discBinPathKnown = ingress.discBinPathKnown;
    zSource.discBinPath = ingress.discBinPath;
    zSource.zCompoRowKnown = ingress.zCompoRow.known;
    zSource.zCompoRow = ingress.zCompoRow;
    PrSS0Scene0IntLoadDirect::Transaction8001AC18 zCompo{};
    uint32_t zAttempts = 0u;
    for (; zAttempts < 4u; ++zAttempts) {
        zCompo = PrSS0Scene0IntLoadDirect::
            BuildZCompoTransaction80015590(zSource);
        if (PrSS0Scene0IntLoadDirect::
                IsExactAcceptedZCompoTransaction80015590(zCompo)) {
            ++zAttempts;
            break;
        }
    }

    if (!PrSS0Scene0IntLoadDirect::
            IsExactAcceptedStartupCommonTransaction80016B84(common) ||
        !PrSS0Scene0IntLoadDirect::
            IsExactAcceptedZCompoTransaction80015590(zCompo) ||
        commonAttempts == 0u || commonAttempts > 4u ||
        zAttempts == 0u || zAttempts > 4u) {
        Log::Printf(
            "SS0 direct runtime: pre-logo 80016B84 INT payload blocked commonStatus=%u commonAttempts=%u zStatus=%u zAttempts=%u",
            static_cast<unsigned>(common.status),
            static_cast<unsigned>(commonAttempts),
            static_cast<unsigned>(zCompo.status),
            static_cast<unsigned>(zAttempts));
        return false;
    }

    // SCUS 80016B84 closes the current VAB between the COMMON and ZCOMPO
    // 8001AC18 calls.  Keep that source boundary explicit even when the cold
    // startup has no open host VAB: the translated close is a committed
    // software no-op, while physical SPU/MMIO and interrupt timing remain
    // outside the Windows owner.
    const auto startupVabClose = PrSfx::ApplyScene0VabClose80027120();
    if (!startupVabClose.known || !startupVabClose.committed ||
        startupVabClose.function != 0x80027120u ||
        !startupVabClose.closeRequested ||
        !startupVabClose.hostProjectionCleared ||
        startupVabClose.psxSpuRegisterAuthority ||
        startupVabClose.psxInterruptTimingAuthority) {
        Log::Printf(
            "SS0 direct runtime: pre-logo 80016B84 COMMON->80027120->ZCOMPO close barrier blocked priorHostVab=%d committed=%d projectionCleared=%d",
            startupVabClose.priorHostVabActive ? 1 : 0,
            startupVabClose.committed ? 1 : 0,
            startupVabClose.hostProjectionCleared ? 1 : 0);
        return false;
    }

    // SCUS 80016B84 invokes 80026FA4 immediately after the ZCOMPO
    // 8001AC18 load and before either logo callback.  Execute the translated
    // 800351B8(0) state producer now and fail closed if the shared SS0 audio
    // state cannot accept the exact wrapper result.
    const auto audioReset = PrSfx::ApplySharedAudioResetBarrier26FA4();
    if (!audioReset.committed ||
        audioReset.wrapperFunction != 0x80026FA4u ||
        audioReset.resetFunction != 0x800351B8u ||
        audioReset.resetArg != 0 ||
        audioReset.psxSpuRegisterAuthority ||
        audioReset.psxInterruptTimingAuthority) {
        Log::Printf(
            "SS0 direct runtime: pre-logo 80026FA4->800351B8(0) blocked translated=%d known=%d committed=%d voiceCount=%u resetMask=%06X",
            audioReset.translated ? 1 : 0,
            audioReset.known ? 1 : 0,
            audioReset.committed ? 1 : 0,
            static_cast<unsigned>(audioReset.returnedVoiceCount),
            audioReset.resetVoiceMask);
        return false;
    }

    s_ss0Direct.bootStartupPreloadIngress80016B84 = std::move(ingress);
    s_ss0Direct.bootStartupCommonIntLoad80016B84 = std::move(common);
    s_ss0Direct.bootStartupZCompoIntLoad80016B84 = std::move(zCompo);
    s_ss0Direct.bootStartupVabClose80027120 = startupVabClose;
    s_ss0Direct.bootStartupVabCloseCommitted80027120 = true;
    s_ss0Direct.bootStartupAudioReset80026FA4 = audioReset;
    s_ss0Direct.bootStartupAudioResetCommitted80026FA4 = true;
    s_ss0Direct.bootStartupDiscIntPrepared80016B84 = true;
    s_ss0Direct.bootStartupPreparedBeforeFirstLogo80016B84 =
        ctx.frame == 0u && !s_ss0Direct.initialized;
    s_ss0Direct.bootStartupPrepareFrame80016B84 = ctx.frame;
    s_ss0Direct.bootStartupCommonAttemptCount8001AC18 = commonAttempts;
    s_ss0Direct.bootStartupZCompoAttemptCount8001AC18 = zAttempts;
    Log::Printf(
        "SS0 direct runtime: pre-logo 80016B84 disc/INT committed frame=%u beforeFirstLogo=%u calls=80025A34>8001AC18(COMMON)>80027120(close)>80025A34>8001AC18(ZCOMPO)>80026FA4>Logo1>Logo2>80025A34 commonAttempts=%u commonTim=%u commonVab=%u vabCloseKnown=%u vabClosePriorHost=%u zAttempts=%u zTim=%u zVab=%u discPayloadAuthority=1 startupAudioBindings80016C8C=1 timVramSideEffects=0 vabSpuSideEffects=0 inputAudioTail80026FA4=1 resetVoices=%u resetMask=%06X",
        static_cast<unsigned>(ctx.frame),
        s_ss0Direct.bootStartupPreparedBeforeFirstLogo80016B84 ? 1u : 0u,
        static_cast<unsigned>(commonAttempts),
        static_cast<unsigned>(
            s_ss0Direct.bootStartupCommonIntLoad80016B84.timEntryCount),
        static_cast<unsigned>(
            s_ss0Direct.bootStartupCommonIntLoad80016B84.vabEntryCount),
        s_ss0Direct.bootStartupVabCloseCommitted80027120 ? 1u : 0u,
        s_ss0Direct.bootStartupVabClose80027120.priorHostVabActive ? 1u : 0u,
        static_cast<unsigned>(zAttempts),
        static_cast<unsigned>(
            s_ss0Direct.bootStartupZCompoIntLoad80016B84.timEntryCount),
        static_cast<unsigned>(
            s_ss0Direct.bootStartupZCompoIntLoad80016B84.vabEntryCount),
        static_cast<unsigned>(audioReset.returnedVoiceCount),
        audioReset.resetVoiceMask);
    return s_ss0Direct.bootStartupPreparedBeforeFirstLogo80016B84;
}

void SetWord800916F0DiscFullbootStartupPublisherDisabledForProbe(
    bool disabled) {
    s_word800916F0DiscFullbootStartupPublisherDisabledForProbe = disabled;
}

int Fn0(PrGameContext& ctx) {
    return SS0DirectFn0(ctx);
}

void Fn1(PrGameContext& ctx) {
    if (SS0DirectTryCommitScene0Init801C4260(ctx)) {
        (void)SS0DirectStartScene0OuterEntry801C4DC4(ctx);
    }
}

int Fn2(PrGameContext& ctx) {
    return SS0DirectFn2(ctx);
}

void Main(PrGameContext& ctx) {
    (void)ctx;
}

void Render(PrGameContext& ctx) {
    SS0DirectRender(ctx);
}

void RenderLateSubtitles(PrGameContext& ctx) {
    if (!ctx.renderer || ctx.currentScene != PrSceneId::Scene0 ||
        s_ss0Direct.phase != SS0DirectPhase::OpeningMovie0 ||
        ctx.subtitleFlag == 0) {
        return;
    }
    if (!s_ss0Direct.scene0SubtitleDataLoaded) {
        (void)SS0DirectLoadScene0SubtitleData(ctx);
    }
    if (!s_ss0Direct.scene0SubtitleDataAvailable) {
        return;
    }

    const uint32_t stageFrame = SS0DirectComputeSubtitleStageFrame();
    const auto& textRuntime = s_ss0Direct.scene0SubtitleVtextRuntime;
    if (textRuntime.activeTextPtr == nullptr ||
        textRuntime.activeDurationFramesRemaining == 0u) {
        return;
    }

    const PrMovieSubtitles::MovieSubtitleTrack& track =
        PrStage1MovieTextDirect::GetMovieSubtitleTrack(
            s_ss0Direct.scene0Movie0Vtext);
    const uint32_t eventIndex = textRuntime.eventCursor > 0u
        ? textRuntime.eventCursor - 1u
        : 0u;
    const int language =
        textRuntime.selectedLanguageIndex < 5u
            ? static_cast<int>(textRuntime.selectedLanguageIndex)
            : 0;
    const PrMovieSubtitles::MovieSubtitleLine* subtitle =
        eventIndex < track.lines.size() ? &track.lines[eventIndex] : nullptr;
    if (subtitle == nullptr || subtitle->textIndex[language] <= 0) {
        return;
    }

    float vx = 0.0f;
    float vy = 0.0f;
    float vs = 1.0f;
    SS0DirectCalcPs1Viewport(ctx.renderer, vx, vy, vs);
    if (SS0DirectRenderSubtitleText(
            ctx, textRuntime.activeTextPtr, vx, vy, vs) &&
        !s_ss0Direct.scene0SubtitleRenderLogged) {
        s_ss0Direct.scene0SubtitleRenderLogged = true;
        Log::Printf(
            "SS0 direct runtime: S0 subtitle rendered from COMOD0 event=%u stageFrame=%u start=%u duration=%u language=%d renderer=PSX-glyph-atlas legacyActiveCursor=0",
            static_cast<unsigned>(eventIndex),
            static_cast<unsigned>(stageFrame),
            static_cast<unsigned>(subtitle->frame30),
            static_cast<unsigned>(subtitle->duration),
            language);
    }
}

bool BeginResidentDirectory80015788(PrGameContext& ctx, int previousScene) {
    namespace Executor = PrStage1LifecycleExecutorDirect;
    if (s_residentDirectory) return true;
    if (!s_ss0Direct.initialized || !ctx.renderer || !ctx.resources) return false;
    const auto action = Executor::BuildResidentDirectoryBootstrapAction80015788(previousScene);
    const auto plan = Executor::BuildBootstrap15590DirectPlan801C81EC(action);
    const auto path = PrSceneEntryDirect::IdentifySceneEntryPathPtr(plan.sceneLoaderPathPtr);
    auto& atlas = PrStageSceneSubmitBackend::GetNativeDirectoryAtlasProjection80015590();
    if (!plan.valid || !path.known || !path.relativeWinPath || atlas.GetLoadedCount() == 0) {
        Log::Printf(
            "SS0 resident 80015788: preflight rejected previousV1=%d "
            "planValid=%d pathKnown=%d relativePath=%d sceneLoader=%08X atlasLoaded=%llu",
            previousScene,
            plan.valid ? 1 : 0,
            path.known ? 1 : 0,
            path.relativeWinPath ? 1 : 0,
            action.sceneLoaderPsxAddr,
            static_cast<unsigned long long>(atlas.GetLoadedCount()));
        return false;
    }

    auto owner = std::make_unique<ResidentDirectoryRuntime80015788>();
    owner->previousScene = previousScene;
    PrPsxEventFrameDirect::ResetEventFrameState8003FB9C(owner->page, 320u, 240u);
    // 80015D18's 8001EF14, then 80015788's audio prefix and 80015590(3).
    // This is not Scene0 Fn0/Fn1: progress, card payload and Win options survive.
    PrSS0TransitionDirect::ResetLoadingPatternState8001EF14(
        owner->loader.bootstrap15590Loading.pattern);
    const auto audioReset = PrSfx::ApplySharedAudioResetBarrier26FA4();
    if (!audioReset.committed) return false;
    PrSfx::PlayScene0TransitionCue94410();
    PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
    Executor::Bootstrap15590HostStartFacts801C81EC facts{};
    facts.curtainStarted = PrStage1LoadingDirect::BeginAfterReset8001EF14(
        owner->loader.bootstrap15590Loading, 3);
    if (!facts.curtainStarted) return false;
    auto start = Executor::BuildBootstrap15590HostBlockStart801C81EC(action, facts);
    start.pathResolved = true;
    start.path = ctx.dataRoot / path.relativeWinPath;
    const auto started = Executor::BeginHostBlock801C81EC(owner->loader, start);
    if (!started.waitingForHostBlock || !owner->loader.bootstrap15590LoaderDirectBegun)
        return false;
    PrStage1LoadingDirect::Tick(owner->loader.bootstrap15590Loading, ctx.frame);
    s_ss0Direct.loadingPatternRuntime8001EF40 = owner->loader.bootstrap15590Loading.pattern;
    s_ss0Direct.loadingPatternFrame8001EF40 = owner->loader.bootstrap15590Loading.frame;
    SS0DirectClearLoadingMinimumHold();
    s_ss0Direct.loadingScreenKind = SS0DirectLoadingHoldKind::None;
    s_ss0Direct.loadingPatternMode80015408 = 3;
    s_ss0Direct.phase = SS0DirectPhase::MainMenuResourceLoading;
    s_ss0Direct.sceneHandoffRuntimeActive = false;
    s_ss0Direct.sceneHandoffTargetPending = false;
    s_ss0Direct.sceneHandoffTargetScene = -1;
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_ss0Direct.mainMenuPreLoopPadRelease80026B94);
    s_ss0Direct.mainMenuPreLoopPadReleaseBlockedLogged = false;
    ctx.stageRunning = false;
    ctx.sceneExitReason = 3;
    if (ctx.strPlayer && ctx.strPlayer->IsPlaying()) ctx.strPlayer->Stop();
    s_residentDirectory = std::move(owner);
    PrSS0TitleTmdBackend::BindResidentDirectoryRenderProjection80015788(
        &atlas, &s_residentDirectory->page.graph);
    PrPsxSpriteTemplateRender::BindResidentDirectoryAtlasProjection80015788(&atlas);
    Log::Printf("SS0 resident 80015788: begin currentScene=%u previousV1=%d row=%08X path=%s nativeLoader=1 scene0Fn1=0",
        static_cast<unsigned>(ctx.currentScene), previousScene,
        action.sceneLoaderPsxAddr, path.psxPath);
    return true;
}

bool IsResidentDirectoryActive80015788() {
    return s_residentDirectory != nullptr;
}

void TickResidentDirectory80015788(PrGameContext& ctx) {
    if (!s_residentDirectory) return;
    auto& owner = *s_residentDirectory;
    ctx.stageRunning = false;
    if (!owner.resourcesReady) {
        const auto completed = PrStage1LifecycleHostAdapter801C81EC::
            TickBootstrap15590Block(ctx, owner.loader);
        if (owner.loader.bootstrap15590Active) {
            s_ss0Direct.loadingPatternRuntime8001EF40 = owner.loader.bootstrap15590Loading.pattern;
            s_ss0Direct.loadingPatternFrame8001EF40 = owner.loader.bootstrap15590Loading.frame;
            return;
        }
        // Completion clears the executor's loader scratch/flags. Consume its
        // returned receipt, not fields deliberately reset by ClearBlock.
        if (!completed.producedImmediateInput) return;
        owner.resourcesReady = true;
        Log::Printf("SS0 resident 80015788: native 80015590 complete previousV1=%d", owner.previousScene);
    }
    if (!owner.entryQueued) {
        owner.entryQueued = SS0DirectQueueMainMenuEntryTransition80026C90(ctx);
        if (owner.entryQueued) {
            PrSS0TransitionDirect::StopLoadingPatternRuntime8001EF40(
                s_ss0Direct.loadingPatternRuntime8001EF40);
            s_ss0Direct.loadingPatternFrame8001EF40 = {};
        }
        return;
    }
    (void)SS0DirectFn2(ctx);
    const int target = ConsumeSceneHandoffTarget();
    if (target >= 0) owner.result = target;
}

int ConsumeResidentDirectoryResult80015788() {
    if (!s_residentDirectory || s_residentDirectory->result < 0) return -1;
    const int result = s_residentDirectory->result;
    PrSS0TitleTmdBackend::BindResidentDirectoryRenderProjection80015788(nullptr, nullptr);
    PrPsxSpriteTemplateRender::BindResidentDirectoryAtlasProjection80015788(nullptr);
    s_residentDirectory.reset();
    Log::Printf("SS0 resident 80015788: return target=%d", result);
    return result;
}

int ConsumeSceneHandoffTarget() {
    if (!s_ss0Direct.sceneHandoffTargetPending ||
        s_ss0Direct.sceneHandoffTargetScene < 0) {
        return -1;
    }
    const int target = s_ss0Direct.sceneHandoffTargetScene;
    s_ss0Direct.sceneHandoffTargetPending = false;
    s_ss0Direct.sceneHandoffTargetScene = -1;
    Log::Printf(
        "SS0 direct runtime: scene handoff target consumed by PrMain target=%d",
        target);
    return target;
}

bool IsSceneHandoffActive() {
    return s_ss0Direct.sceneHandoffRuntimeActive ||
           s_ss0Direct.sceneHandoffTargetPending;
}

int GetSceneHandoffTarget() {
    return IsSceneHandoffActive() ? s_ss0Direct.sceneHandoffTargetScene : -1;
}

bool QueueRuntimeState16CardReadTypedFactsOneShot(
    const PrStage1SaveCardHalDirect::State16CardReadRuntimeTypedFacts800179B4&
        facts) {
    return SS0DirectQueueRuntimeState16CardReadTypedFactsOneShot(facts);
}

bool QueueRuntimeLoadReplayState16CardIoResult80017594(
    const CardHandoff::LoadReplayState16CardIoResultCarrier80017594&
        carrier) {
    return SS0DirectQueueRuntimeLoadReplayState16CardIoResult80017594(
        carrier);
}

int GetPhaseDebug() {
    if (s_ss0Direct.phase ==
        SS0DirectPhase::OpeningMovie0InitialTransition) {
        return 94;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleIntroTransition) {
        return 96;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleMovie0TFinalReady) {
        return 95;
    }
    if (s_ss0Direct.phase ==
        SS0DirectPhase::OpeningMovie0PreTransition) {
        return 97;
    }
    if (s_ss0Direct.phase ==
        SS0DirectPhase::OpeningMovie0PostTransition) {
        return 98;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::OpeningMovie0) {
        return 99;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleSelector ||
        s_ss0Direct.phase == SS0DirectPhase::TitleExitWait) {
        return 100;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::TitleExitTransition) {
        return 101;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::MainMenuEntryTransition) {
        return 109;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::MainMenu) {
        return 102;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::Options) {
        return 103;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::StageSelect) {
        return 104;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::Practice) {
        return 105;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::HiScore) {
        return 108;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::ReplayCard) {
        return 106;
    }
    if (s_ss0Direct.phase == SS0DirectPhase::LoadCard) {
        return 107;
    }
    return 101;
}

void ApplyWord800916EEMainLoopWriteback80015D18(int32_t v2,
                                                uint16_t word800916D0) {
    if (word800916D0 != 1u) {
        return;
    }

    const int16_t next = static_cast<int16_t>(v2);
    if (s_ss0Direct.word800916EE != next) {
        Log::Printf(
            "SS0 direct runtime: 80015D18 wrote word_800916EE=%d from v2=%d",
            static_cast<int>(next),
            v2);
    }
    s_ss0Direct.word800916EE = next;
}

bool PublishInitialWord800916F0RuntimeObservation(
    const Word800916F0Observation& observation) {
    if (observation.source !=
        Word800916F0ObservationSource::RuntimePsxMemoryObservation) {
        Log::Printf(
            "SS0 direct runtime: initial word_800916F0 observation rejected source=%u",
            static_cast<unsigned>(observation.source));
        return ApplyWord800916F0RuntimeObservation(observation);
    }
    if (observation.psxAddress != 0x800916F0u ||
        observation.byteSize != 2u ||
        !observation.valueKnown) {
        Log::Printf(
            "SS0 direct runtime: initial word_800916F0 observation rejected address=0x%08X size=%u known=%d",
            observation.psxAddress,
            observation.byteSize,
            observation.valueKnown ? 1 : 0);
        return ApplyWord800916F0RuntimeObservation(observation);
    }

    return ApplyWord800916F0RuntimeObservation(observation);
}

bool InitializeWord800916F6FromProcessStartupZero80028590() {
    static constexpr uint32_t kCoveringStoreAddress =
        kWord800916F6PsxAddress & ~(sizeof(uint32_t) - 1u);
    static_assert((kCoveringStoreAddress & (sizeof(uint32_t) - 1u)) == 0u);
    static_assert(kProcessStartupZeroBegin80028590 <= kCoveringStoreAddress);
    static_assert(kWord800916F6PsxAddress + sizeof(uint16_t) <=
                  kCoveringStoreAddress + sizeof(uint32_t));
    static_assert(kCoveringStoreAddress + sizeof(uint32_t) <=
                  kProcessStartupZeroEnd80028590);

    if (s_word800916F6.known) {
        return s_word800916F6.source ==
                   Word800916F6AuthoritySource::ProcessStartupZero80028590 &&
               s_word800916F6.value == 0u;
    }
    if (s_word800916F6.source != Word800916F6AuthoritySource::Unknown) {
        return false;
    }

    s_word800916F6.known = true;
    s_word800916F6.value = 0u;
    s_word800916F6.source =
        Word800916F6AuthoritySource::ProcessStartupZero80028590;
    Log::Printf(
        "SS0 direct runtime: word_800916F6 initialized value=0 authority=current-IDA-80028590 coveringStore=800916F4 range=[8006ECB8,801C3870)");
    return true;
}

bool InitializeWord800916FCFromProcessStartupZero80028590() {
    static_assert(kProcessStartupZeroBegin80028590 <=
                  kWord800916FCPsxAddress);
    static_assert(kWord800916FCPsxAddress + sizeof(uint16_t) <=
                  kProcessStartupZeroEnd80028590);
    static_assert((kWord800916FCPsxAddress -
                   kProcessStartupZeroBegin80028590) %
                      sizeof(uint32_t) ==
                  0u);

    if (s_word800916FC.known) {
        return s_word800916FC.source ==
                   Word800916FCAuthoritySource::ProcessStartupZero80028590 &&
               s_word800916FC.value == 0u;
    }
    if (s_word800916FC.source != Word800916FCAuthoritySource::Unknown) {
        return false;
    }

    s_word800916FC.known = true;
    s_word800916FC.value = 0u;
    s_word800916FC.source =
        Word800916FCAuthoritySource::ProcessStartupZero80028590;
    Log::Printf(
        "SS0 direct runtime: word_800916FC initialized value=0 authority=current-IDA-80028590 range=[8006ECB8,801C3870)");
    return true;
}

bool DebugUnlockNextStage(const PrGameContext& ctx) {
    if (!SS0DirectDebugStatusOwned(ctx, "debug unlock next")) {
        return false;
    }

    uint8_t status[8]{};
    if (!SS0DirectReadDirectBankStageStatuses(status)) {
        return false;
    }

    bool changed = false;
    for (int stage = 1; stage <= 6; ++stage) {
        if (status[stage] == 0u) {
            status[stage] = 1u;
            changed = true;
            break;
        }
    }
    if (!changed) {
        Log::Printf("SS0 direct runtime: debug unlock next no locked stage");
        return true;
    }
    return SS0DirectPublishStageStatuses(status, "debug unlock next");
}

bool DebugFirstClearSelectableStages(const PrGameContext& ctx) {
    if (!SS0DirectDebugStatusOwned(ctx, "debug first clear")) {
        return false;
    }

    uint8_t status[8]{};
    if (!SS0DirectReadDirectBankStageStatuses(status)) {
        return false;
    }

    bool changed = false;
    int furthestUnlocked = 0;
    for (int stage = 1; stage <= 6; ++stage) {
        if (status[stage] == 0u) {
            continue;
        }
        furthestUnlocked = stage;
        if (status[stage] < 2u) {
            status[stage] = 2u;
            changed = true;
        }
    }
    if (furthestUnlocked >= 1 && furthestUnlocked < 6 &&
        status[furthestUnlocked + 1] == 0u) {
        status[furthestUnlocked + 1] = 1u;
        changed = true;
    }
    if (!changed) {
        Log::Printf("SS0 direct runtime: debug first clear no selectable change");
        return true;
    }
    return SS0DirectPublishStageStatuses(status, "debug first clear");
}

bool DebugSeedCardAuthGapEntries(int count, int firstBlockIndex) {
    if (!PrSS0Direct::RuntimeCutoverAllowed()) {
        Log::Printf("SS0 direct runtime: debug card seed rejected runtime disabled");
        return false;
    }
    if (!s_ss0Direct.initialized) {
        Log::Printf(
            "SS0 direct runtime: debug card seed rejected uninitialized");
        return false;
    }
    if (s_ss0Direct.phase != SS0DirectPhase::LoadCard &&
        s_ss0Direct.phase != SS0DirectPhase::ReplayCard) {
        Log::Printf(
            "SS0 direct runtime: debug card seed rejected phase=%u",
            static_cast<unsigned>(s_ss0Direct.phase));
        return false;
    }

    const int clampedCount = std::clamp(count, 1, 3);
    const int clampedFirstBlock = std::clamp(firstBlockIndex, 0, 14);
    SS0DirectClearTypedCardReadCarriers();
    CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
    SS0DirectClearCardDirectoryAuthority();
    SS0DirectClearReplayRawSelectorFixture();
    s_ss0Direct.cardEntryCount = clampedCount;
    s_ss0Direct.cardCursor = 0;
    std::fill(std::begin(s_ss0Direct.cardBlockIndex),
              std::end(s_ss0Direct.cardBlockIndex),
              -1);
    for (int i = 0; i < 15; ++i) {
        s_ss0Direct.cardTitle[i][0] = '\0';
    }
    for (int i = 0; i < clampedCount; ++i) {
        const int blockIndex = (clampedFirstBlock + i) % 15;
        s_ss0Direct.cardBlockIndex[i] = blockIndex;
        std::snprintf(s_ss0Direct.cardTitle[i],
                      sizeof(s_ss0Direct.cardTitle[i]),
                      "DEBUG AUTH GAP %02d",
                      blockIndex);
    }
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "DEBUG CARD AUTH GAP SEED - X:AUTH GAP O:BACK");
    Log::Printf(
        "SS0 direct runtime: debug card seed entries=%d firstBlock=%d phase=%u",
        clampedCount,
        clampedFirstBlock,
        static_cast<unsigned>(s_ss0Direct.phase));
    return true;
}

bool DebugPublishRuntimeDirectoryScanFacts(
    const CardHandoff::LoadReplayDirectoryScanFacts80019D7C& facts) {
    if (!PrSS0Direct::RuntimeCutoverAllowed()) {
        Log::Printf(
            "SS0 direct runtime: runtime directory scan facts rejected runtime disabled");
        return false;
    }
    if (!s_ss0Direct.initialized) {
        Log::Printf(
            "SS0 direct runtime: runtime directory scan facts rejected uninitialized");
        return false;
    }
    if (s_ss0Direct.phase != SS0DirectPhase::LoadCard &&
        s_ss0Direct.phase != SS0DirectPhase::ReplayCard) {
        Log::Printf(
            "SS0 direct runtime: runtime directory scan facts rejected phase=%u",
            static_cast<unsigned>(s_ss0Direct.phase));
        return false;
    }

    const CardHandoff::CardMode800191E4 phaseMode =
        s_ss0Direct.phase == SS0DirectPhase::ReplayCard
            ? CardHandoff::CardMode800191E4::Replay
            : CardHandoff::CardMode800191E4::Load;
    if (facts.mode != phaseMode) {
        SS0DirectClearTypedCardReadCarriers();
        CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
        Log::Printf(
            "SS0 direct runtime: runtime directory scan facts rejected mode=%d phaseMode=%d phase=%u",
            static_cast<int>(facts.mode),
            static_cast<int>(phaseMode),
            static_cast<unsigned>(s_ss0Direct.phase));
        return false;
    }

    SS0DirectClearTypedCardReadCarriers();
    CardHandoff::LoadReplayDirectoryTypedCarrier80019D7C carrier{};
    if (!CardHandoff::
            BuildRuntimeLoadReplayDirectoryCarrierFromScanFacts80019D7C(
                facts,
                &carrier)) {
        CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
        Log::Printf(
            "SS0 direct runtime: %s runtime directory scan facts rejected known=%d directoryRows=%d snapshot=%d listRows=%d entryCountKnown=%d entryCount=%d",
            CardHandoff::CardMode800191E4Name(phaseMode),
            facts.known ? 1 : 0,
            facts.directoryRowsKnown80017B08 ? 1 : 0,
            facts.snapshotKnown80017B18 ? 1 : 0,
            facts.listRowsBuilt80019D7C ? 1 : 0,
            facts.entryCountKnown ? 1 : 0,
            facts.entryCount);
        return false;
    }
    if (!CardHandoff::PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(
            carrier)) {
        CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
        Log::Printf(
            "SS0 direct runtime: %s runtime directory scan facts publish failed entries=%d",
            CardHandoff::CardMode800191E4Name(phaseMode),
            carrier.entryCount);
        return false;
    }

    const bool refreshed = SS0DirectRefreshCardEntries(
        phaseMode,
        phaseMode == CardHandoff::CardMode800191E4::Replay
            ? "REPLAY DIRECTORY GAP - O:BACK"
            : "LOAD DIRECTORY GAP - O:BACK",
        false);
    const bool accepted =
        refreshed &&
        s_ss0Direct.cardEntryCount == carrier.entryCount &&
        s_ss0Direct.cardCursor == 0 &&
        carrier.entryCount > 0 &&
        s_ss0Direct.cardBlockIndex[0] == carrier.rows[0].blockIndex;
    Log::Printf(
        "SS0 direct runtime: %s runtime directory scan facts publish entries=%d firstBlock=%d accepted=%d phase=%u",
        CardHandoff::CardMode800191E4Name(phaseMode),
        carrier.entryCount,
        carrier.entryCount > 0 ? carrier.rows[0].blockIndex : -1,
        accepted ? 1 : 0,
        static_cast<unsigned>(s_ss0Direct.phase));
    return accepted;
}

bool DebugSeedCardInvalidIndexEntry(int invalidBlockIndex) {
    if (!PrSS0Direct::RuntimeCutoverAllowed()) {
        Log::Printf(
            "SS0 direct runtime: debug card invalid-index seed rejected runtime disabled");
        return false;
    }
    if (!s_ss0Direct.initialized) {
        Log::Printf(
            "SS0 direct runtime: debug card invalid-index seed rejected uninitialized");
        return false;
    }
    if (s_ss0Direct.phase != SS0DirectPhase::LoadCard &&
        s_ss0Direct.phase != SS0DirectPhase::ReplayCard) {
        Log::Printf(
            "SS0 direct runtime: debug card invalid-index seed rejected phase=%u",
            static_cast<unsigned>(s_ss0Direct.phase));
        return false;
    }
    if (invalidBlockIndex >= 0 && invalidBlockIndex < 15) {
        Log::Printf(
            "SS0 direct runtime: debug card invalid-index seed rejected block=%d",
            invalidBlockIndex);
        return false;
    }

    SS0DirectClearTypedCardReadCarriers();
    CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
    SS0DirectClearCardDirectoryAuthority();
    SS0DirectClearReplayRawSelectorFixture();
    s_ss0Direct.cardEntryCount = 1;
    s_ss0Direct.cardCursor = 0;
    std::fill(std::begin(s_ss0Direct.cardBlockIndex),
              std::end(s_ss0Direct.cardBlockIndex),
              -1);
    for (int i = 0; i < 15; ++i) {
        s_ss0Direct.cardTitle[i][0] = '\0';
    }
    s_ss0Direct.cardBlockIndex[0] = invalidBlockIndex;
    std::snprintf(s_ss0Direct.cardTitle[0],
                  sizeof(s_ss0Direct.cardTitle[0]),
                  "DEBUG BAD INDEX");
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "DEBUG CARD INDEX GAP SEED - X:INDEX GAP O:BACK");
    Log::Printf(
        "SS0 direct runtime: debug card invalid-index seed block=%d phase=%u",
        invalidBlockIndex,
        static_cast<unsigned>(s_ss0Direct.phase));
    return true;
}

bool DebugPublishState16TypedPayloadCarrier(int byteSeed) {
    if (!PrSS0Direct::RuntimeCutoverAllowed()) {
        Log::Printf(
            "SS0 direct runtime: debug state16 typed payload rejected runtime disabled");
        return false;
    }
    if (!s_ss0Direct.initialized) {
        Log::Printf(
            "SS0 direct runtime: debug state16 typed payload rejected uninitialized");
        return false;
    }
    if (s_ss0Direct.phase != SS0DirectPhase::LoadCard &&
        s_ss0Direct.phase != SS0DirectPhase::ReplayCard) {
        Log::Printf(
            "SS0 direct runtime: debug state16 typed payload rejected phase=%u",
            static_cast<unsigned>(s_ss0Direct.phase));
        return false;
    }
    if (s_ss0Direct.cardEntryCount <= 0 ||
        s_ss0Direct.cardEntryCount > 15 ||
        s_ss0Direct.cardCursor < 0 ||
        s_ss0Direct.cardCursor >= s_ss0Direct.cardEntryCount ||
        s_ss0Direct.cardCursor >= 15) {
        SS0DirectClearTypedCardReadCarriers();
        Log::Printf(
            "SS0 direct runtime: debug state16 typed payload rejected cursor=%d entries=%d",
            s_ss0Direct.cardCursor,
            s_ss0Direct.cardEntryCount);
        return false;
    }

    const int selectedBlock =
        s_ss0Direct.cardBlockIndex[s_ss0Direct.cardCursor];
    if (selectedBlock < 0 || selectedBlock >=
            PrStage1SaveCardHalDirect::kReadAttemptCount800179B4) {
        SS0DirectClearTypedCardReadCarriers();
        Log::Printf(
            "SS0 direct runtime: debug state16 typed payload rejected block=%d cursor=%d entries=%d",
            selectedBlock,
            s_ss0Direct.cardCursor,
            s_ss0Direct.cardEntryCount);
        return false;
    }

    SS0DirectClearTypedCardReadCarriers();
    std::array<uint8_t,
               PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4>
        blockBytes{};
    const uint8_t seed = static_cast<uint8_t>(byteSeed & 0xFF);
    for (std::size_t i = 0; i < blockBytes.size(); ++i) {
        blockBytes[i] = static_cast<uint8_t>(seed + (i & 0xFFu));
    }

    PrStage1SaveCardHalDirect::CardReadFeedback800179B4 feedback{};
    feedback.feedbackKnown = true;
    feedback.word8007ABE4Known = true;
    feedback.word8007ABE4 = 1;
    for (int i = 0; i < PrStage1SaveCardHalDirect::kReadAttemptCount800179B4;
         ++i) {
        feedback.attempts[i].rowEnabledKnown = true;
        feedback.attempts[i].rowEnabled = false;
    }

    PrStage1SaveCardHalDirect::CardReadAttemptFeedback800179B4& attempt =
        feedback.attempts[selectedBlock];
    attempt.rowEnabled = true;
    attempt.rowNameKnown = true;
    std::snprintf(attempt.rowName,
                  sizeof(attempt.rowName),
                  "BISLPS-00000DEBUG");
    attempt.rowNameBuffer8007CBE8Known = true;
    attempt.cardSelectorKnown = true;
    attempt.cardSelectorGp128 = selectedBlock;
    attempt.cardSelectorGp124 = selectedBlock;
    attempt.pathBuilt800173A8 = true;
    attempt.pathOpenFlagsKnown800173A8 = true;
    attempt.pathOpenFlags800173A8 =
        PrStage1SaveCardHalDirect::kCardPathOpenFlags800173A8;
    attempt.openAttempted800173A8 = true;
    attempt.fdKnown800173A8 = true;
    attempt.fd800173A8 = 5;
    attempt.gp696FdWriteKnown800173A8 = true;
    attempt.gp696Fd800173A8 = 5;
    attempt.targetBufferKnown = true;
    attempt.targetBufferAddress =
        PrStage1SaveCardHalDirect::kCardReadBlockBufferAddr800179B4;
    attempt.readLengthKnown = true;
    attempt.readLength =
        PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4;
    attempt.payloadPointerKnown = true;
    attempt.payloadPointer =
        PrStage1SaveCardHalDirect::kCardReadPayloadAddr8007ADE8;
    attempt.payloadPassedTo800164F8 = true;
    attempt.blockCountKnown = true;
    attempt.blockCount =
        PrStage1SaveCardHalDirect::kCardReadBlockCount800179B4;
    attempt.clearEvents.called = true;
    attempt.clearEvents.eventHandlesKnown = true;
    for (int i = 0; i < 4; ++i) {
        attempt.clearEvents.eventHandles[i] =
            static_cast<uint32_t>(0x1000 + i);
        attempt.clearEvents.testEventResultsKnown[i] = true;
        attempt.clearEvents.testEventResults[i] = 0;
    }
    attempt.readSubmission.called = true;
    attempt.readSubmission.fdKnown = true;
    attempt.readSubmission.fd = 5;
    attempt.readSubmission.bufferAddressKnown = true;
    attempt.readSubmission.bufferAddress =
        PrStage1SaveCardHalDirect::kCardReadBlockBufferAddr800179B4;
    attempt.readSubmission.byteCountKnown = true;
    attempt.readSubmission.byteCount =
        PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4;
    attempt.poll.called = true;
    attempt.poll.eventHandlesKnown = true;
    for (int i = 0; i < 4; ++i) {
        attempt.poll.eventHandles[i] = static_cast<uint32_t>(0x2000 + i);
    }
    attempt.poll.resultKnown = true;
    attempt.poll.psxReturn = 1;
    attempt.poll.timedOutKnown = true;
    attempt.poll.timedOut = false;
    attempt.poll.hitEventIndexKnown = true;
    attempt.poll.hitEventIndex = 0;
    attempt.poll.pollIterationCountKnown = true;
    attempt.poll.pollIterationCount = 1;
    attempt.poll.waitCallCountKnown80035560 = true;
    attempt.poll.waitCallCount80035560 = 0;
    attempt.closeKnown800179B4 = true;
    attempt.closeFdKnown800179B4 = true;
    attempt.closeFd800179B4 = 5;
    attempt.blockBytesKnown = true;
    attempt.blockBytes = blockBytes.data();
    attempt.blockByteCount = blockBytes.size();

    (void)PrStage1SaveCardHalDirect::
        PublishDebugState16CardReadTypedCarrier800179B4ForBlock(
            feedback,
            selectedBlock);
    PrStage1SaveCardHalDirect::State16CardReadTypedCarrier800179B4
        carrier{};
    const bool published =
        PrStage1SaveCardHalDirect::GetState16CardReadTypedCarrier800179B4(
            &carrier) &&
        carrier.source ==
            PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
                DebugSyntheticFixture &&
        !carrier.producerWired800173A8_80016EB8_800179B4 &&
        carrier.state16LoadPayloadLaneKnown &&
        carrier.selectedBlockKnown &&
        carrier.selectedBlockIndex == selectedBlock &&
        carrier.typedReadSuccessKnown800179B4 &&
        carrier.payloadBytesKnown8007ADE8 &&
        !carrier.incomplete;
    Log::Printf(
        "SS0 direct runtime: debug state16 typed payload publish block=%d seed=%u ok=%d phase=%u entries=%d cursor=%d",
        selectedBlock,
        static_cast<unsigned>(seed),
        published ? 1 : 0,
        static_cast<unsigned>(s_ss0Direct.phase),
        s_ss0Direct.cardEntryCount,
        s_ss0Direct.cardCursor);
    return published;
}

bool DebugSeedReplayRawSelector(int savedSlot, int blockIndex) {
    if (!PrSS0Direct::RuntimeCutoverAllowed()) {
        Log::Printf(
            "SS0 direct runtime: debug replay raw selector rejected runtime disabled");
        return false;
    }
    if (!s_ss0Direct.initialized) {
        Log::Printf(
            "SS0 direct runtime: debug replay raw selector rejected uninitialized");
        return false;
    }
    if (s_ss0Direct.phase != SS0DirectPhase::ReplayCard) {
        Log::Printf(
            "SS0 direct runtime: debug replay raw selector rejected phase=%u",
            static_cast<unsigned>(s_ss0Direct.phase));
        return false;
    }
    if (savedSlot < 0) {
        Log::Printf(
            "SS0 direct runtime: debug replay raw selector rejected savedSlot=%d",
            savedSlot);
        return false;
    }

    const int clampedBlock = std::clamp(blockIndex, 0, 14);
    SS0DirectClearTypedCardReadCarriers();
    CardHandoff::ClearLoadReplayDirectoryTypedCarrier80019D7C();
    SS0DirectClearCardDirectoryAuthority();
    s_ss0Direct.cardEntryCount = 1;
    s_ss0Direct.cardCursor = 0;
    std::fill(std::begin(s_ss0Direct.cardBlockIndex),
              std::end(s_ss0Direct.cardBlockIndex),
              -1);
    for (int i = 0; i < 15; ++i) {
        s_ss0Direct.cardTitle[i][0] = '\0';
    }
    s_ss0Direct.cardBlockIndex[0] = clampedBlock;
    std::snprintf(s_ss0Direct.cardTitle[0],
                  sizeof(s_ss0Direct.cardTitle[0]),
                  "DEBUG RAW SEL %d",
                  savedSlot);
    std::snprintf(s_ss0Direct.cardMessage,
                  sizeof(s_ss0Direct.cardMessage),
                  "DEBUG REPLAY RAW SELECTOR - X:PROBE O:BACK");
    s_ss0Direct.debugReplayRawSelectorArmed = true;
    s_ss0Direct.debugReplayRawSelectorBlock = clampedBlock;
    s_ss0Direct.debugReplayRawSelectorSavedSlot =
        static_cast<uint32_t>(savedSlot);
    Log::Printf(
        "SS0 direct runtime: debug replay raw selector seed savedSlot=%d block=%d phase=%u",
        savedSlot,
        clampedBlock,
        static_cast<unsigned>(s_ss0Direct.phase));
    return true;
}

bool DebugArmHiScoreEvent6EntryFixture() {
    if (!PrSS0Direct::RuntimeCutoverAllowed() || !s_ss0Direct.initialized ||
        s_ss0Direct.phase != SS0DirectPhase::MainMenu ||
        s_ss0Direct.hiScoreOuterPadReleaseActive80015788) {
        Log::Printf(
            "SS0 direct runtime: probe-only Event6 entry fixture rejected nonAuthority=1 initialized=%d phase=%u outerRelease=%d",
            s_ss0Direct.initialized ? 1 : 0,
            static_cast<unsigned>(s_ss0Direct.phase),
            s_ss0Direct.hiScoreOuterPadReleaseActive80015788 ? 1 : 0);
        return false;
    }
    s_ss0Direct.debugHiScoreEvent6EntryFixtureArmed = true;
    Log::Printf(
        "SS0 direct runtime: probe-only Event6 entry fixture armed nonAuthority=1 entryReferenceOnly=1");
    return true;
}

bool DebugSeedAttractLastScene(int scene) {
    if (!PrSS0Direct::RuntimeCutoverAllowed()) {
        Log::Printf(
            "SS0 direct runtime: debug attract last-scene seed rejected runtime disabled");
        return false;
    }
    if (!s_ss0Direct.initialized ||
        s_ss0Direct.phase != SS0DirectPhase::TitleSelector ||
        s_ss0Direct.titleCursor != 0 ||
        s_ss0Direct.titleExitWaitPresentGate.waitCounter != 0u ||
        s_ss0Direct.titlePendingTargetScene != -1 ||
        s_ss0Direct.titlePendingMenu) {
        Log::Printf(
            "SS0 direct runtime: debug attract last-scene seed rejected phase=%u cursor=%d wait=%d target=%d menu=%d",
            static_cast<unsigned>(s_ss0Direct.phase),
            s_ss0Direct.titleCursor,
            static_cast<int>(s_ss0Direct.titleExitWaitPresentGate.waitCounter),
            s_ss0Direct.titlePendingTargetScene,
            s_ss0Direct.titlePendingMenu ? 1 : 0);
        return false;
    }
    if (scene < 1 || scene > 6) {
        Log::Printf(
            "SS0 direct runtime: debug attract last-scene seed rejected scene=%d",
            scene);
        return false;
    }

    s_ss0Direct.word800916EE = static_cast<int16_t>(scene);
    Log::Printf(
        "SS0 direct runtime: debug attract last-scene seed word_800916EE=%d",
        scene);
    return true;
}

bool DebugSeedTitleExitInvalidTarget(int targetScene) {
    if (!PrSS0Direct::RuntimeCutoverAllowed()) {
        Log::Printf(
            "SS0 direct runtime: debug title invalid target rejected runtime disabled");
        return false;
    }
    if (!s_ss0Direct.initialized ||
        s_ss0Direct.phase != SS0DirectPhase::TitleSelector ||
        s_ss0Direct.titleCursor != 0 ||
        s_ss0Direct.titleExitWaitPresentGate.waitCounter != 0u ||
        s_ss0Direct.titlePendingTargetScene != -1 ||
        s_ss0Direct.titlePendingMenu) {
        Log::Printf(
            "SS0 direct runtime: debug title invalid target rejected phase=%u cursor=%d wait=%d target=%d menu=%d",
            static_cast<unsigned>(s_ss0Direct.phase),
            s_ss0Direct.titleCursor,
            static_cast<int>(s_ss0Direct.titleExitWaitPresentGate.waitCounter),
            s_ss0Direct.titlePendingTargetScene,
            s_ss0Direct.titlePendingMenu ? 1 : 0);
        return false;
    }
    if (SS0DirectTitleHandoffTargetInRange(targetScene)) {
        Log::Printf(
            "SS0 direct runtime: debug title invalid target rejected target=%d",
            targetScene);
        return false;
    }

    Log::Printf(
        "SS0 direct runtime: debug title invalid target seed target=%d",
        targetScene);
    SS0DirectArmTitleExitWait(
        PrSS0TitleHudEventsDirect::kTitleSelectorResultStart801C47EC,
        PrSceneId::Scene0,
        targetScene,
        false);
    return true;
}

DebugSnapshot GetDebugSnapshot() {
    DebugSnapshot snapshot{};
    snapshot.phaseRaw = static_cast<int>(s_ss0Direct.phase);
    snapshot.phaseDebug = GetPhaseDebug();
    snapshot.transitionReturnPhaseRaw =
        static_cast<int>(s_ss0Direct.transitionReturnPhase);
    snapshot.directTitleLoopStateV8 =
        static_cast<int>(s_ss0Direct.titleLoopStateV8);
    snapshot.directTitleAttractTimeoutKnown =
        s_ss0Direct.titleAttractTimeoutKnown ? 1 : 0;
    snapshot.directTitleAttractTimeoutLimit =
        s_ss0Direct.titleAttractTimeoutLimit;
    snapshot.directTitleAttractTimer = s_ss0Direct.attractTimer;
    snapshot.loadingHoldKind =
        static_cast<int>(s_ss0Direct.loadingHoldKind);
    snapshot.loadingScreenKind =
        static_cast<int>(s_ss0Direct.loadingScreenKind);
    snapshot.loadingHoldStartFrame = s_ss0Direct.loadingHoldStartFrame;
    snapshot.loadingHoldUntilFrame = s_ss0Direct.loadingHoldUntilFrame;
    snapshot.loadingPatternActive8001EF40 =
        s_ss0Direct.loadingPatternRuntime8001EF40.active ? 1 : 0;
    snapshot.loadingPatternStyle8001EF40 =
        static_cast<int>(s_ss0Direct.loadingPatternFrame8001EF40.style);
    snapshot.loadingPatternHighlightCount8001EF40 =
        static_cast<int>(
            s_ss0Direct.loadingPatternFrame8001EF40.highlightCount);
    snapshot.loadingPatternSubmittedHighlightCount8001EF40 =
        static_cast<int>(
            s_ss0Direct.loadingPatternSubmittedHighlightCount8001EF40);
    snapshot.loadingPatternMutationSerial8001EF40 =
        s_ss0Direct.loadingPatternFrame8001EF40.mutationSerial;
    snapshot.loadingPatternCallbackCount8001537C =
        s_ss0Direct.loadingPatternFrame8001EF40.frameCounterGp51;
    snapshot.loadingPatternGridHashLow8001EF40 = static_cast<uint32_t>(
        s_ss0Direct.loadingPatternFrame8001EF40.liveGridFnv1a & 0xFFFFFFFFu);
    snapshot.loadingPatternSubmittedFrame8001EF40 =
        s_ss0Direct.loadingPatternSubmittedFrame8001EF40;

    switch (s_ss0Direct.phase) {
    case SS0DirectPhase::OpeningMovie0InitialTransition:
    case SS0DirectPhase::TitleIntroTransition:
    case SS0DirectPhase::TitleMovie0TFinalReady:
    case SS0DirectPhase::OpeningMovie0PreTransition:
    case SS0DirectPhase::OpeningMovie0PostTransition:
    case SS0DirectPhase::OpeningMovie0:
        break;
    case SS0DirectPhase::MainMenu:
        snapshot.directDispState = 1;
        snapshot.directDispEventId = 3;
        snapshot.directMenuIndex = s_ss0Direct.mainMenuState.cursor;
        snapshot.directMainMenuRecordsMode =
            s_ss0Direct.mainMenuState.word800916DA;
        break;
    case SS0DirectPhase::StageSelect:
        snapshot.directDispState = 1;
        snapshot.directDispEventId = 2;
        snapshot.directMenuIndex = s_ss0Direct.stageSelectState.cursor;
        for (int i = 0; i < 9; ++i) {
            if (s_ss0Direct.stageSelectState.enabled[i]) {
                snapshot.directStageSelectEnabledMask |= (1 << i);
            }
        }
        break;
    case SS0DirectPhase::Options:
        snapshot.directDispState = 1;
        snapshot.directDispEventId = 17;
        snapshot.directMenuIndex = s_ss0Direct.optionsState.cursor;
        snapshot.directOptionsLanguage = s_ss0Direct.optionsState.word800916D8;
        snapshot.directOptionsSubtitle = s_ss0Direct.optionsState.word800916DC;
        snapshot.directOptionsPreLoopReleasePending =
            s_ss0Direct.optionsPreLoopPadRelease80026B94.waitingForRelease
                ? 1
                : 0;
        snapshot.directOptionsCooldown = s_ss0Direct.optionsState.cooldown;
        snapshot.directOptionsTimeoutRemaining =
            s_ss0Direct.optionsInitialInputTimeout80026B94.framesRemaining;
        snapshot.directOptionsInitialInputPending =
            s_ss0Direct.optionsInitialInputTimeout80026B94.initialInputPending
                ? 1
                : 0;
        snapshot.directOptionsTailActive =
            s_ss0Direct.optionsDispatcherTail80026B94.active ? 1 : 0;
        snapshot.directOptionsTailFramesRemaining =
            s_ss0Direct.optionsDispatcherTail80026B94.framesRemaining;
        snapshot.directOptionsTailResult =
            s_ss0Direct.optionsDispatcherTail80026B94.latchedResult;
        break;
    case SS0DirectPhase::Practice:
        snapshot.directDispState = 1;
        snapshot.directDispEventId = 16;
        snapshot.directMenuIndex = s_ss0Direct.practiceRound;
        snapshot.directPracticePhase =
            static_cast<int>(s_ss0Direct.practicePhase);
        snapshot.directPracticeRound = s_ss0Direct.practiceRound;
        snapshot.directPracticeRoundFrame = s_ss0Direct.practiceRoundFrame;
        snapshot.directPracticePadStopFrames =
            s_ss0Direct.practicePadStopFrames;
        snapshot.directPracticePadStopKind =
            s_ss0Direct.practicePadStopKind;
        snapshot.directPracticeExitConfirmFrames =
            s_ss0Direct.practiceExitConfirmFrames;
        snapshot.directPracticeScore = s_ss0Direct.practiceScore;
        snapshot.directPracticeHits = s_ss0Direct.practiceHits;
        snapshot.directPracticeMisses = s_ss0Direct.practiceMisses;
        snapshot.directPracticeLastJudge = s_ss0Direct.practiceLastJudge;
        snapshot.directPracticeLastDelta = s_ss0Direct.practiceLastDelta;
        snapshot.directPracticeJudgeFlash = s_ss0Direct.practiceJudgeFlash;
        snapshot.directPracticeSeqCurA = s_ss0Direct.practiceSeqCurA;
        snapshot.directPracticeSeqCurB = s_ss0Direct.practiceSeqCurB;
        snapshot.directPracticeSeqEnA = s_ss0Direct.practiceSeqEnA;
        snapshot.directPracticeSeqEnB = s_ss0Direct.practiceSeqEnB;
        break;
    case SS0DirectPhase::ReplayCard:
        snapshot.directDispState = 1;
        snapshot.directDispEventId = 9;
        snapshot.directMenuIndex = s_ss0Direct.cardCursor;
        snapshot.directCardEntryCount = s_ss0Direct.cardEntryCount;
        snapshot.directCardSelectedBlock =
            (s_ss0Direct.cardEntryCount > 0 &&
             s_ss0Direct.cardCursor >= 0 &&
             s_ss0Direct.cardCursor < s_ss0Direct.cardEntryCount)
                ? s_ss0Direct.cardBlockIndex[s_ss0Direct.cardCursor]
                : -1;
        break;
    case SS0DirectPhase::LoadCard:
        snapshot.directDispState = 1;
        snapshot.directDispEventId = 8;
        snapshot.directMenuIndex = s_ss0Direct.cardCursor;
        snapshot.directCardEntryCount = s_ss0Direct.cardEntryCount;
        snapshot.directCardSelectedBlock =
            (s_ss0Direct.cardEntryCount > 0 &&
             s_ss0Direct.cardCursor >= 0 &&
             s_ss0Direct.cardCursor < s_ss0Direct.cardEntryCount)
                ? s_ss0Direct.cardBlockIndex[s_ss0Direct.cardCursor]
                : -1;
        break;
    case SS0DirectPhase::HiScore:
        snapshot.directDispState = 1;
        snapshot.directDispEventId = 6;
        snapshot.directMenuIndex = 0;
        snapshot.directHiScoreBlink = s_ss0Direct.hiScoreBlink;
        snapshot.directHiScoreExitLabelState =
            s_ss0Direct.hiScoreEvent6State800267E4.requestBound
                ? s_ss0Direct.hiScoreEvent6State800267E4
                      .exitLabelStateCtx04
                : -1;
        snapshot.directHiScorePreLoopReleasePending =
            s_ss0Direct.hiScorePreLoopPadRelease80026B94.waitingForRelease
                ? 1
                : 0;
        snapshot.directHiScoreTailActive =
            s_ss0Direct.hiScoreDispatcherTail80026B94.active ? 1 : 0;
        snapshot.directHiScoreTailFramesRemaining =
            s_ss0Direct.hiScoreDispatcherTail80026B94.framesRemaining;
        snapshot.directHiScoreTailResult =
            s_ss0Direct.hiScoreDispatcherTail80026B94.latchedResult;
        snapshot.directHiScoreOuterPadReleaseActive =
            s_ss0Direct.hiScoreOuterPadReleaseActive80015788 ? 1 : 0;
        snapshot.directHiScoreOuterPadReleasePending =
            s_ss0Direct.hiScoreOuterPadRelease80015788.waitingForRelease
                ? 1
                : 0;
        snapshot.directHiScoreEvent6TableKnown =
            s_ss0Direct.hiScoreEvent6TableCarrierKnown ? 1 : 0;
        snapshot.directHiScoreEvent6TablePsxAddress =
            s_ss0Direct.hiScoreEvent6TablePsxAddress;
        snapshot.directHiScoreEvent6TableByteCount =
            static_cast<int>(s_ss0Direct.hiScoreEvent6TableByteCount);
        break;
    case SS0DirectPhase::TitleSelector:
    case SS0DirectPhase::TitleExitWait:
    case SS0DirectPhase::TitleExitTransition:
        snapshot.directMenuIndex = s_ss0Direct.titleCursor;
        snapshot.directTitleExitWaitCounter = static_cast<int>(
            s_ss0Direct.titleExitWaitPresentGate.waitCounter);
        snapshot.directTitleExitTargetScene =
            s_ss0Direct.titlePendingTargetScene;
        snapshot.directTitleExitPendingMenu =
            s_ss0Direct.titlePendingMenu ? 1 : 0;
        break;
    case SS0DirectPhase::WaitTransition:
    default:
        break;
    }
    return snapshot;
}

} // namespace PrSS0Scene0RuntimeDirect
