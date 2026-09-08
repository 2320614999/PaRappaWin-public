#include "pr_event.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "audio_engine.h"
#include "logger.h"
#include "pr_game_context.h"
#include "pr_memcard_backend.h"
#include "pr_pad.h"
#include "pr_psx_event_frame_direct.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_sfx.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "pr_transition.h"
#include "resource_manager.h"
#include "scene_event_parser.h"

namespace {

constexpr uint16_t PAD_CROSS    = 0x0040;
constexpr uint16_t PAD_CIRCLE   = 0x0020;
constexpr uint16_t PAD_SQUARE   = 0x0080;
constexpr uint16_t PAD_TRIANGLE = 0x0010;
constexpr uint16_t PAD_RIGHT    = 0x2000;
constexpr uint16_t PAD_DOWN     = 0x4000;
constexpr uint16_t PAD_LEFT     = 0x8000;
constexpr uint16_t PAD_UP       = 0x1000;

uint16_t PickSinglePadInput(uint16_t mask) {
    if (mask & PAD_CROSS) return PAD_CROSS;
    if (mask & PAD_CIRCLE) return PAD_CIRCLE;
    if (mask & PAD_SQUARE) return PAD_SQUARE;
    if (mask & PAD_TRIANGLE) return PAD_TRIANGLE;
    if (mask & PAD_UP) return PAD_UP;
    if (mask & PAD_DOWN) return PAD_DOWN;
    if (mask & PAD_LEFT) return PAD_LEFT;
    if (mask & PAD_RIGHT) return PAD_RIGHT;
    return 0;
}

uint16_t PickStrictSinglePadInput(uint16_t pressed) {
    switch (pressed) {
        case PAD_CROSS:
        case PAD_CIRCLE:
        case PAD_SQUARE:
        case PAD_TRIANGLE:
        case PAD_UP:
        case PAD_DOWN:
        case PAD_LEFT:
        case PAD_RIGHT:
            return pressed;
        default:
            return 0;
    }
}

// S0_OLD_STATE: legacy menu dispatcher runtime used by Scene0/S0 shell.
// SS0 must rebuild these event states from PSX evidence, not call this state.
PrDispatcherState s_dispState = PrDispatcherState::Idle;
PrEventId         s_dispEventId = 0;
int               s_dispResult = 0;
int               s_dispTimeout = 0;
int               s_dispTimeoutInit = 0;
int               s_dispFrameCountdown = 0;
uint16_t          s_dispLastInput = 0;
bool              s_dispWaitingInput = true;
bool              s_dispNoInputYet = true;
void*             s_dispArgPtr = nullptr;
PrGameContext*    s_dispGameCtxPtr = nullptr;
PrEventDispatcherContext s_dispCtx;
static int16_t*   s_ev3RecordsModePtr = nullptr; // -> ctx.transitionStateDA (word_800916DA)
static bool       s_stageSelectStatusInit = false;
static int16_t    s_stageSelectStatus[8] = {};
static bool       s_stageSelectForceEnableAllF0 = false; // word_800916F0 consumer only; writer unresolved
static int32_t    s_event4CueRepeatCount80025E6C = 0; // dword_8006ED6C (gp+0x32C)
static int32_t    s_event4CueFrame80025E6C = 0;       // dword_8006ED68 (gp+0x328)

constexpr size_t kStageStatusPayloadBase = 0x0C; // byte_80092F10 + 0x0C, statuses read from [1..7]
constexpr size_t kStageScorePayloadCount = 6;
constexpr size_t kStageLastSavedSlotPayloadOffset = 0x2C; // dword_80092F3C

void RecomputeBonusStatus();
void SyncStageSelectStatusToPayload();
void EnsureStageSelectStatusInitialized();
void RefreshStageSelectDispatcherContext();
void PopulateStageSelectDispatcherContext267F8();

static int ResolveStageSelectSceneIdFromSavedSlot161A8(int savedSlot) {
    // Match the current PSX `dword_80048DD8` table at the direct-port boundary.
    static constexpr int kSceneBySavedSlot[(int)kStageScorePayloadCount] = {
        1, 2, 3, 4, 5, 6,
    };
    if (savedSlot < 0 || savedSlot >= (int)kStageScorePayloadCount) {
        return -1;
    }
    return kSceneBySavedSlot[savedSlot];
}

static uint32_t ReadPayloadU32(const uint8_t* payload, size_t payloadSize, size_t offset) {
    if (!payload || offset + sizeof(uint32_t) > payloadSize) {
        return 0u;
    }
    return (uint32_t)payload[offset + 0] |
           ((uint32_t)payload[offset + 1] << 8) |
           ((uint32_t)payload[offset + 2] << 16) |
           ((uint32_t)payload[offset + 3] << 24);
}

static bool AreAllPrimaryStagesStatus3() {
    for (int stage = 1; stage <= 6; ++stage) {
        if (s_stageSelectStatus[stage] != 3) {
            return false;
        }
    }
    return true;
}

static void RefreshStageSelectAuthorityMirrorsAndPayload() {
    RecomputeBonusStatus();
    SyncStageSelectStatusToPayload();
    if (s_dispEventId == 2 && s_dispState == PrDispatcherState::Running) {
        RefreshStageSelectDispatcherContext();
    }
}

static void ReloadStageSelectAuthorityFromPayload164B4() {
    s_stageSelectStatusInit = false;
    EnsureStageSelectStatusInitialized();
    if (s_dispEventId == 2 && s_dispState == PrDispatcherState::Running) {
        RefreshStageSelectDispatcherContext();
    }
}

void RecomputeBonusStatus() {
    int clearCount = 0;
    for (int stage = 1; stage <= 6; ++stage) {
        if (s_stageSelectStatus[stage] >= 3) {
            ++clearCount;
        }
    }
    s_stageSelectStatus[7] = (clearCount >= 6) ? 1 : 0;
}

void SyncStageSelectStatusToPayload() {
    uint8_t* payload = PrMemCardBackend::MutablePayload();
    const size_t payloadSize = PrMemCardBackend::PayloadSize();
    if (!payload || payloadSize <= (kStageStatusPayloadBase + 7)) {
        return;
    }

    payload[kStageStatusPayloadBase + 0] = 0;
    for (int i = 1; i <= 7; ++i) {
        const int v = std::clamp((int)s_stageSelectStatus[i], 0, 3);
        payload[kStageStatusPayloadBase + (size_t)i] = (uint8_t)v;
    }
}

void SyncDebugStageSelectStatusToDirectBank() {
    for (int stage = 1; stage <= 6; ++stage) {
        const int status = std::clamp((int)s_stageSelectStatus[stage], 0, 3);
        if (status >= 1) {
            (void)PrStage1SaveUiDirect::Sub800167A8(stage, status >= 2 ? 1 : 0);
        }
    }
}

void ResetStageSelectStatusToBase() {
    for (int i = 0; i < 8; ++i) {
        s_stageSelectStatus[i] = 0;
    }
    s_stageSelectStatus[1] = 1;
    RecomputeBonusStatus();
    SyncStageSelectStatusToPayload();
}

void EnsureStageSelectStatusInitialized() {
    if (s_stageSelectStatusInit) {
        return;
    }
    s_stageSelectStatusInit = true;

    const uint8_t* payload = PrMemCardBackend::CurrentPayload();
    const size_t payloadSize = PrMemCardBackend::PayloadSize();
    bool any = false;
    if (payload && payloadSize > (kStageStatusPayloadBase + 7)) {
        for (int i = 1; i <= 6; ++i) {
            const int v = std::clamp((int)payload[kStageStatusPayloadBase + (size_t)i], 0, 3);
            s_stageSelectStatus[i] = (int16_t)v;
            any |= (v != 0);
        }
    }

    if (!any) {
        ResetStageSelectStatusToBase();
        return;
    }

    RecomputeBonusStatus();
    SyncStageSelectStatusToPayload();
}

void RefreshStageSelectDispatcherContext() {
    EnsureStageSelectStatusInitialized();
    PopulateStageSelectDispatcherContext267F8();

    if (s_dispCtx.menuIndex < 1 || s_dispCtx.menuIndex > 8 || s_dispCtx.selections[s_dispCtx.menuIndex] == 0) {
        s_dispCtx.menuIndex = 8;
        for (int i = 1; i <= 8; ++i) {
            if (s_dispCtx.selections[i] != 0) {
                s_dispCtx.menuIndex = (int16_t)i;
                break;
            }
        }
    }
}

void PopulateStageSelectDispatcherContext267F8() {
    // Direct port of the confirmed `sub_800267F8` consumer shape:
    // - fixed `count = 8`
    // - `word_800916F0 == 1` force-enable branch wins first
    // - otherwise `word_800916DA == 1` records-only branch
    // - default path mirrors the stage-status table + bonus + cancel
    s_dispCtx.menuCount = 8;
    s_dispCtx.selections[0] = 0;

    if (s_stageSelectForceEnableAllF0) {
        for (int i = 0; i <= 8; ++i) {
            s_dispCtx.selections[i] = 1;
        }
        // Keep the currently known oddball value writes literal until the
        // `word_800916F0` writer/source is recovered.
        s_dispCtx.selections[5] = 2;
        s_dispCtx.selections[6] = 3;
        return;
    }

    if (s_ev3RecordsModePtr && *s_ev3RecordsModePtr == 1) {
        s_dispCtx.selections[1] = 1;
        for (int i = 2; i <= 7; ++i) {
            s_dispCtx.selections[i] = 0;
        }
        s_dispCtx.selections[8] = 1;
        return;
    }

    for (int i = 1; i <= 7; ++i) {
        s_dispCtx.selections[i] = s_stageSelectStatus[i];
    }
    s_dispCtx.selections[8] = 1; // CANCEL / EXIT
}

struct Ev6Ctx {
    int32_t blink;
    int32_t done_flag;
};
static Ev6Ctx s_ev6Ctx;

struct Ev5Ctx {
    int32_t unused;
};
static Ev5Ctx s_ev5Ctx;

struct Ev16Ctx {
    int32_t frame;
    int32_t tick;
    int32_t round;
    int32_t frameInRound;
    int32_t mode; // 0=LISTEN, 1=PRACTICE
    int32_t hits;
    int32_t hitGood;
    int32_t hitEarly;
    int32_t hitLate;
    int32_t flags00;
    int32_t state1C;
    int32_t blink4C;
    int32_t exit48;
    int32_t padStopFrames;

    int32_t seqMode8A;
    int32_t seqCurA8C;
    int32_t seqEnA90;
    int32_t seqCurB9E;
    int32_t seqEnBA2;

    const int8_t* streamA94;
    const int8_t* streamBA4;
};
static Ev16Ctx s_ev16Ctx;

static_assert(sizeof(Ev16Ctx) == sizeof(PrEv16Ctx), "Ev16Ctx/PrEv16Ctx size mismatch");

enum class Ev16Phase : uint8_t {
    Idle = 0,
    RoundIntro,
    Preview,
    Judge,
    ResultOverlay,
};

static int32_t s_ev16PostStage = 0;
static int32_t s_ev16ExitCountdown = 0;
static Ev16Phase s_ev16Phase = Ev16Phase::Idle;
static int32_t s_ev16PendingRound = -1;
static bool s_ev16JudgePending = false;
static bool s_ev16JudgeAbortToExit = false;
static int32_t s_ev16JudgePendingKind = -1;
static int32_t s_ev16JudgePendingRound = -1;
static int32_t s_ev16PendingVoiceKind = -1;
static int32_t s_ev16PendingVoiceDelay = 0;

static int8_t s_ev16Rec44Stream40[4][40] = {};
static int16_t s_ev16Rec44Head0[4] = {};
static uint16_t s_ev16Rec44Head1[4] = {};
static bool s_ev16Rec44Loaded = false;
static bool s_ev16Rec44Ok = false;
enum class Ev16Rec44Source : uint8_t { None = 0, Comod = 1, Resources = 2, Static = 3 };
static Ev16Rec44Source s_ev16Rec44Source = Ev16Rec44Source::None;
static std::filesystem::path s_ev16Rec44LoadedComodPath;
static uint32_t s_ev16Rec44LoadedResGen = 0;
static PrSceneId s_ev16Rec44LoadedScene = PrSceneId::Scene0;

static constexpr int16_t kEv16StaticRec44Head0[4] = {1, 1, 1, 1};
static constexpr uint16_t kEv16StaticRec44Head1[4] = {0, 0, 0, 0};
static constexpr int8_t kEv16StaticRec44Stream40[4][40] = {
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, -1, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

constexpr int kEv16FramesPerRound = 150;
constexpr int kEv16BeatFrames = 75;
constexpr int kEv16RoundCount = 4;
constexpr int kEv16PreviewStartFrame = 112;
constexpr int kEv16PreviewEndFrame = 149;
constexpr int kEv16FastForwardFrame = 135;
constexpr int kEv16TargetOffsets[4] = {0, 0x13, 0x25, 0x38};
constexpr int kEv16RoundIntroKinds[4] = {3, 4, 5, 6};
constexpr int kEv16RoundIntroFrames[4] = {80, 46, 42, 49};

static void ResetEv16SeqState() {
    s_ev16Ctx.seqMode8A = 0;
    s_ev16Ctx.seqCurA8C = -1;
    s_ev16Ctx.seqEnA90 = 0;
    s_ev16Ctx.seqCurB9E = -1;
    s_ev16Ctx.seqEnBA2 = 0;
}

static void RefreshEv16RoundStreams() {
    const int r = (int)s_ev16Ctx.round & 3;
    s_ev16Ctx.streamA94 = s_ev16Rec44Ok ? s_ev16Rec44Stream40[r] : nullptr;
    s_ev16Ctx.streamBA4 = s_ev16Rec44Ok ? s_ev16Rec44Stream40[r] : nullptr;
}

static void ResetEv16JudgePending() {
    s_ev16JudgePending = false;
    s_ev16JudgeAbortToExit = false;
    s_ev16JudgePendingKind = -1;
    s_ev16JudgePendingRound = -1;
}

static void ScheduleEv16VoiceKind(int kind, int delayFrames) {
    s_ev16PendingVoiceKind = kind;
    s_ev16PendingVoiceDelay = std::max(0, delayFrames);
}

static void StartEv16ExitConfirmIntro(const char* reason) {
    ResetEv16JudgePending();
    ResetEv16SeqState();
    RefreshEv16RoundStreams();
    s_ev16PostStage = 2;
    s_ev16Ctx.padStopFrames = 95;
    s_ev16Ctx.flags00 = 0x00400000;
    s_ev16Ctx.state1C = 8;
    s_ev16Ctx.blink4C = 0;
    s_ev16Ctx.exit48 = 0;
    s_ev16Ctx.frame = -1;
    s_ev16Ctx.frameInRound = -1;
    s_ev16Ctx.tick = 0;
    s_ev16Phase = Ev16Phase::ResultOverlay;
    ScheduleEv16VoiceKind(8, 4);
    Log::Printf("Practice ev=16 %s -> exitConfirm intro", reason ? reason : "judge end");
}

static void ResolveEv16JudgeResult(int kind, int roundBeforeResolve) {
    ResetEv16JudgePending();
    const int nextRound = (kind == 0) ? ((int)s_ev16Ctx.round + 1) : (int)s_ev16Ctx.round;
    s_ev16PendingRound = (kind == 0) ? nextRound : -1;

    ResetEv16SeqState();

    // PSX keeps the current round's symbol stream visible through the result overlay
    // and only commits the next round when the next intro actually starts.
    if (kind != 0) {
        RefreshEv16RoundStreams();
    }

    if (kind == 0 && nextRound >= kEv16RoundCount) {
        s_ev16PendingRound = -1;
        s_ev16PostStage = 1;
        s_ev16Ctx.padStopFrames = 88;
        s_ev16Ctx.flags00 = 0x00400000;
        s_ev16Ctx.state1C = 7;
        s_ev16Ctx.blink4C = 0;
        s_ev16Ctx.exit48 = 0;
        s_ev16Ctx.frame = -1;
        s_ev16Ctx.frameInRound = -1;
        s_ev16Ctx.tick = 0;
        s_ev16Phase = Ev16Phase::ResultOverlay;
        s_ev16PendingVoiceKind = -1;
        s_ev16PendingVoiceDelay = 0;
        PrSfx::PlayPracticeVoiceKind(7);
        Log::Printf("Practice ev=16 judge resolve: complete r0=%d kind=%d -> round=%d",
                    roundBeforeResolve,
                    kind,
                    (int)s_ev16Ctx.round);
        return;
    }

    PrSfx::PlayPracticeVoiceKind(kind);
    s_ev16Ctx.padStopFrames = 75;
    s_ev16Ctx.flags00 = 0x00400000;
    s_ev16Ctx.state1C = kind;
    s_ev16Ctx.blink4C = 0;
    s_ev16Ctx.exit48 = 0;
    s_ev16Ctx.frame = -1;
    s_ev16Ctx.frameInRound = -1;
    s_ev16Ctx.tick = 0;
    s_ev16Phase = Ev16Phase::ResultOverlay;
    s_ev16PendingVoiceKind = -1;
    s_ev16PendingVoiceDelay = 0;
    Log::Printf("Practice ev=16 judge resolve r0=%d kind=%d -> round=%d",
                roundBeforeResolve,
                kind,
                (int)s_ev16Ctx.round);
}

static void StartEv16RoundIntro() {
    if (s_ev16PendingRound >= 0) {
        s_ev16Ctx.round = s_ev16PendingRound;
        s_ev16PendingRound = -1;
    }
    const int r = (int)s_ev16Ctx.round & 3;
    ResetEv16JudgePending();
    RefreshEv16RoundStreams();
    ResetEv16SeqState();
    s_ev16Ctx.frame = -1;
    s_ev16Ctx.frameInRound = -1;
    s_ev16Ctx.tick = 0;
    s_ev16Ctx.flags00 = 0x00400000;
    s_ev16Ctx.state1C = kEv16RoundIntroKinds[r];
    s_ev16Ctx.blink4C = 0;
    s_ev16Ctx.exit48 = 0;
    s_ev16Ctx.padStopFrames = kEv16RoundIntroFrames[r];
    s_ev16Phase = Ev16Phase::RoundIntro;
    s_ev16PendingVoiceKind = -1;
    s_ev16PendingVoiceDelay = 0;
    PrSfx::PlayPracticeVoiceKind(s_ev16Ctx.state1C);
    Log::Printf("Practice ev=16 round intro start: round=%d kind=%d frames=%d",
                (int)s_ev16Ctx.round,
                (int)s_ev16Ctx.state1C,
                (int)s_ev16Ctx.padStopFrames);
}

static void StartEv16PreviewPhase() {
    RefreshEv16RoundStreams();
    ResetEv16SeqState();
    s_ev16Ctx.frame = kEv16PreviewStartFrame - 1;
    s_ev16Ctx.frameInRound = kEv16PreviewStartFrame - 1;
    s_ev16Ctx.tick = 0;
    s_ev16Ctx.flags00 = 0;
    s_ev16Ctx.state1C = 0;
    s_ev16Ctx.blink4C = 0;
    s_ev16Ctx.exit48 = 0;
    s_ev16Ctx.padStopFrames = 0;
    s_ev16Phase = Ev16Phase::Preview;
    s_ev16PendingVoiceKind = -1;
    s_ev16PendingVoiceDelay = 0;
    PrSfx::PlayPracticeHiTick();
    Log::Printf("Practice ev=16 preview start: round=%d frame=%d",
                (int)s_ev16Ctx.round,
                (int)s_ev16Ctx.frameInRound);
}

static void StartEv16JudgePhase() {
    ResetEv16JudgePending();
    s_ev16Ctx.frame = -1;
    s_ev16Ctx.frameInRound = -1;
    s_ev16Ctx.tick = 0;
    s_ev16Ctx.flags00 = 0;
    s_ev16Ctx.state1C = 0;
    s_ev16Ctx.blink4C = 0;
    s_ev16Ctx.exit48 = 0;
    s_ev16Ctx.padStopFrames = 0;
    s_ev16Phase = Ev16Phase::Judge;
    s_ev16PendingVoiceKind = -1;
    s_ev16PendingVoiceDelay = 0;
    Log::Printf("Practice ev=16 judge start: round=%d", (int)s_ev16Ctx.round);
}

static void AdvanceEv16SequenceFrame(int frameInRound) {
    s_ev16Ctx.flags00 = 0;

    const int r = (int)s_ev16Ctx.round & 3;
    if (frameInRound == 60) {
        s_ev16Ctx.flags00 = 0x00100000;
        s_ev16Ctx.seqCurB9E = 0;
        s_ev16Ctx.seqEnBA2 = 1;
        s_ev16Ctx.streamBA4 = s_ev16Rec44Ok ? s_ev16Rec44Stream40[r] : nullptr;
    } else if (frameInRound == 135) {
        s_ev16Ctx.flags00 = 0x00000800;
        const int16_t h0 = s_ev16Rec44Ok ? s_ev16Rec44Head0[r] : (int16_t)0;
        const uint16_t h1 = s_ev16Rec44Ok ? s_ev16Rec44Head1[r] : (uint16_t)0xFFFFu;
        s_ev16Ctx.seqMode8A = (int)h0;
        s_ev16Ctx.seqCurA8C = (h1 <= 18) ? (int)h1 : 0;
        s_ev16Ctx.seqEnA90 = 1;
        s_ev16Ctx.streamA94 = s_ev16Rec44Ok ? s_ev16Rec44Stream40[r] : nullptr;
    }

    if (frameInRound >= 0 && (frameInRound % 5) == 0) {
        if (s_ev16Ctx.seqEnA90 != 0 && s_ev16Ctx.seqCurA8C >= 0 && s_ev16Ctx.seqCurA8C < 19) {
            const int8_t v = s_ev16Ctx.streamA94 ? s_ev16Ctx.streamA94[s_ev16Ctx.seqCurA8C] : (int8_t)-1;
            if (v == -1 || s_ev16Ctx.seqCurA8C >= 19) {
                s_ev16Ctx.seqEnA90 = 0;
                s_ev16Ctx.seqCurA8C = -1;
            } else {
                s_ev16Ctx.seqCurA8C += 1;
            }
        }
        if (s_ev16Ctx.seqEnBA2 != 0 && s_ev16Ctx.seqCurB9E >= 0 && s_ev16Ctx.seqCurB9E < 19) {
            const int8_t v = s_ev16Ctx.streamBA4 ? s_ev16Ctx.streamBA4[s_ev16Ctx.seqCurB9E] : (int8_t)-1;
            if (v == -1 || s_ev16Ctx.seqCurB9E >= 19) {
                s_ev16Ctx.seqEnBA2 = 0;
                s_ev16Ctx.seqCurB9E = -1;
            } else {
                s_ev16Ctx.seqCurB9E += 1;
            }
        }
    }
}

static int16_t ReadS16LE(const uint8_t* p) {
    const uint16_t v = (uint16_t)p[0] | ((uint16_t)p[1] << 8);
    return (int16_t)v;
}

static uint16_t ReadU16LE(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static bool IsRec44Like(const uint8_t* rec44) {
    const int16_t head0 = ReadS16LE(rec44 + 0);
    const uint16_t head1 = ReadU16LE(rec44 + 2);
    if (head0 < -8 || head0 > 8) return false;
    if (head0 > 0) {
        if (head1 > 18) return false;
    } else {
        if (head1 != 0xFFFFu && head1 > 18) return false;
    }

    int nonZero = 0;
    int icons = 0;
    for (int i = 0; i < 19; i++) {
        const int8_t v = (int8_t)rec44[4 + i];
        if (v == -1) {
            return i != 0 && icons > 0;
        }
        if (v < 0 || v > 18) {
            return false;
        }
        if (v != 0) {
            ++nonZero;
        }
        if (v >= 1 && v <= 8) {
            ++icons;
        }
    }
    return nonZero > 0 && icons > 0;
}

static bool LoadEv16Rec44StreamsFromComod(const PrGameContext& ctx) {
    if (ctx.currentComodBytes.empty()) {
        return false;
    }

    const uint8_t* data = ctx.currentComodBytes.data();
    const size_t size = ctx.currentComodBytes.size();
    constexpr size_t kRec44Size = 44;
    constexpr size_t kNeedBytes = kRec44Size * 4;
    if (size < kNeedBytes) {
        return false;
    }

    size_t bestOff = (size_t)-1;
    for (size_t off = 0; off + kNeedBytes <= size; off += 1) {
        bool ok = true;
        for (int r = 0; r < 4; r++) {
            const uint8_t* rec = data + off + (size_t)r * kRec44Size;
            if (!IsRec44Like(rec)) {
                ok = false;
                break;
            }
        }
        if (ok) {
            bestOff = off;
            break;
        }
    }
    if (bestOff == (size_t)-1) {
        return false;
    }

    for (int r = 0; r < 4; r++) {
        const uint8_t* rec = data + bestOff + (size_t)r * kRec44Size;
        s_ev16Rec44Head0[r] = ReadS16LE(rec + 0);
        s_ev16Rec44Head1[r] = ReadU16LE(rec + 2);
        for (int k = 0; k < 40; k++) {
            s_ev16Rec44Stream40[r][k] = (int8_t)rec[4 + k];
        }
    }

    const uint32_t psxBase = SceneEventParser::FindBaseAddress(data, size);
    if (psxBase != 0) {
        Log::Printf("Practice: Rec44 from COMOD off=0x%X psx=0x%08X comod=%s", (unsigned)bestOff,
                    (unsigned)(psxBase + (uint32_t)bestOff), ctx.currentComodPath.u8string().c_str());
    } else {
        Log::Printf("Practice: Rec44 from COMOD off=0x%X comod=%s", (unsigned)bestOff,
                    ctx.currentComodPath.u8string().c_str());
    }
    return true;
}

static bool LoadEv16Rec44StreamsFromResources(const PrGameContext& ctx) {
    if (!ctx.resources) {
        return false;
    }

    const std::vector<std::string> names = ctx.resources->GetMemNames();
    constexpr size_t kRec44Size = 44;
    constexpr size_t kNeedBytes = kRec44Size * 4;

    auto scoreIcons18 = [&](const uint8_t* rec44, int* outEnd, int* outBad) -> int {
        int score = 0;
        int end = 19;
        int bad = 0;
        for (int i = 0; i < 18; i++) {
            const int8_t v = (int8_t)rec44[4 + i];
            if (v == -1) {
                end = i;
                break;
            }
            if (v >= 1 && v <= 8) {
                score += 1;
            } else if (v >= 9 && v <= 18) {
                bad += 1;
            }
        }
        if (outEnd) {
            *outEnd = end;
        }
        if (outBad) {
            *outBad = bad;
        }
        return score;
    };

    const std::string* bestName = nullptr;
    const std::vector<uint8_t>* bestBlob = nullptr;
    size_t bestOff = (size_t)-1;
    int bestScore = -1;

    for (const std::string& name : names) {
        const std::vector<uint8_t>* blob = ctx.resources->GetMem(name);
        if (!blob || blob->size() < kNeedBytes) {
            continue;
        }

        const uint8_t* data = blob->data();
        const size_t size = blob->size();

        size_t localBestOff = (size_t)-1;
        int localBestScore = -1;
        for (size_t off = 0; off + kNeedBytes <= size; off += 1) {
            bool ok = true;
            for (int r = 0; r < 4; r++) {
                const uint8_t* rec = data + off + (size_t)r * kRec44Size;
                if (!IsRec44Like(rec)) {
                    ok = false;
                    break;
                }
            }
            if (!ok) {
                continue;
            }

            int iconScore = 0;
            int badScore = 0;
            int ends[4] = {19, 19, 19, 19};
            for (int r = 0; r < 4; r++) {
                int bad = 0;
                iconScore += scoreIcons18(data + off + (size_t)r * kRec44Size, &ends[r], &bad);
                badScore += bad;
            }

            int endPenalty = 0;
            for (int r = 1; r < 4; r++) {
                endPenalty += (ends[r] == ends[0]) ? 0 : 5;
            }

            const int totalScore = iconScore * 10 - endPenalty - badScore * 2;
            if (totalScore > localBestScore) {
                localBestScore = totalScore;
                localBestOff = off;
            }
        }

        if (localBestOff == (size_t)-1) {
            continue;
        }

        const int totalScore = localBestScore;
        const size_t localOff = localBestOff;

        if (totalScore > bestScore) {
            bestScore = totalScore;
            bestName = &name;
            bestBlob = blob;
            bestOff = localBestOff;
        }
    }

    if (!bestName || !bestBlob || bestOff == (size_t)-1) {
        return false;
    }

    {
        const uint8_t* data = bestBlob->data();

        for (int r = 0; r < 4; r++) {
            const uint8_t* rec = data + bestOff + (size_t)r * kRec44Size;
            s_ev16Rec44Head0[r] = ReadS16LE(rec + 0);
            s_ev16Rec44Head1[r] = ReadU16LE(rec + 2);
            for (int k = 0; k < 40; k++) {
                s_ev16Rec44Stream40[r][k] = (int8_t)rec[4 + k];
            }
        }

        Log::Printf("Practice: Rec44 from MEM name=%s off=0x%X scene=%u score=%d",
                    bestName->c_str(),
                    (unsigned)bestOff,
                    (unsigned)ctx.currentScene,
                    bestScore);
        return true;
    }
}

static bool LoadEv16Rec44StreamsFromStatic() {
    if (s_ev16Rec44Loaded) {
        return s_ev16Rec44Ok;
    }
    s_ev16Rec44Loaded = true;
    s_ev16Rec44Ok = true;
    s_ev16Rec44Source = Ev16Rec44Source::Static;
    for (int i = 0; i < 4; i++) {
        s_ev16Rec44Head0[i] = kEv16StaticRec44Head0[i];
        s_ev16Rec44Head1[i] = kEv16StaticRec44Head1[i];
        for (int k = 0; k < 40; k++) {
            s_ev16Rec44Stream40[i][k] = kEv16StaticRec44Stream40[i][k];
        }
    }
    Log::Printf("Practice: Rec44 from static PSX table");
    return true;
}

static bool LoadEv16Rec44Streams(PrGameContext& ctx) {
    const uint32_t resGen = ctx.resources ? ctx.resources->GetGeneration() : 0u;
    const bool comodChanged = (!ctx.currentComodPath.empty() && ctx.currentComodPath != s_ev16Rec44LoadedComodPath);
    const bool resChanged = (resGen != s_ev16Rec44LoadedResGen) || (ctx.currentScene != s_ev16Rec44LoadedScene);
    if (comodChanged || resChanged) {
        s_ev16Rec44Loaded = false;
        s_ev16Rec44Ok = false;
        s_ev16Rec44Source = Ev16Rec44Source::None;
        s_ev16Rec44LoadedComodPath = ctx.currentComodPath;
        s_ev16Rec44LoadedResGen = resGen;
        s_ev16Rec44LoadedScene = ctx.currentScene;
    }

    if (s_ev16Rec44Loaded) {
        return s_ev16Rec44Ok;
    }

    if (ctx.currentScene == PrSceneId::Scene0) {
        return LoadEv16Rec44StreamsFromStatic();
    }

    s_ev16Rec44Loaded = true;
    s_ev16Rec44Ok = false;
    s_ev16Rec44Source = Ev16Rec44Source::None;

    if (LoadEv16Rec44StreamsFromResources(ctx)) {
        s_ev16Rec44Ok = true;
        s_ev16Rec44Source = Ev16Rec44Source::Resources;
        return true;
    }

    if (LoadEv16Rec44StreamsFromComod(ctx)) {
        s_ev16Rec44Ok = true;
        s_ev16Rec44Source = Ev16Rec44Source::Comod;
        return true;
    }

    s_ev16Rec44Loaded = false;
    return LoadEv16Rec44StreamsFromStatic();
}

int s_blinkBeat = 0;

void BlinkTick(int32_t* blink) {
    if (s_blinkBeat > 0) {
        ++s_blinkBeat;
        if (s_blinkBeat >= 20) {
            s_blinkBeat = 0;
        }
    } else {
        s_blinkBeat = 1;
        *blink = (*blink != 1) ? 1 : 0;
    }
}

static void EnsureUiSfxOut() {
    // AudioEngine is globally managed, nothing to do per-call
}

static void QueueTone(double freqHz, double seconds, double amp) {
    auto& engine = AudioEngine::Get();
    if (!engine.IsRunning()) return;
    if (seconds <= 0.0 || freqHz <= 0.0) return;

    const uint32_t sampleRate = engine.GetSampleRate();
    const size_t count = (size_t)std::max(1.0, seconds * (double)sampleRate);

    std::vector<int16_t> samples(count);
    constexpr double kPi = 3.14159265358979323846;
    const double w = 2.0 * kPi * freqHz;
    for (size_t i = 0; i < count; i++) {
        const double t = (double)i / (double)sampleRate;
        const double env = 1.0 - (double)i / (double)count;
        const double v = std::sin(w * t) * env;
        int s = (int)std::lround(v * amp);
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        samples[i] = (int16_t)s;
    }

    int voice = engine.AllocVoice(1, engine.GetSampleRate(), 1.0f, true);  // oneShot = auto-free
    if (voice >= 0) {
        engine.QueueSamples(voice, samples.data(), samples.size());
    }
}

uint16_t MapLocalPrPadMaskToDispatcherPad(uint16_t localMask) {
    uint16_t pad = static_cast<uint16_t>(
        localMask & ((uint16_t)PrPadButton::Triangle |
                     (uint16_t)PrPadButton::Circle |
                     (uint16_t)PrPadButton::Cross |
                     (uint16_t)PrPadButton::Square));
    if ((localMask & (uint16_t)PrPadButton::Up) != 0u) {
        pad = static_cast<uint16_t>(pad | PAD_UP);
    }
    if ((localMask & (uint16_t)PrPadButton::Right) != 0u) {
        pad = static_cast<uint16_t>(pad | PAD_RIGHT);
    }
    if ((localMask & (uint16_t)PrPadButton::Down) != 0u) {
        pad = static_cast<uint16_t>(pad | PAD_DOWN);
    }
    if ((localMask & (uint16_t)PrPadButton::Left) != 0u) {
        pad = static_cast<uint16_t>(pad | PAD_LEFT);
    }
    return pad;
}

void PlayUiSfx(int code) {
    switch (code) {
        case 0x10:
        case 0x1000:
        case 0x2000:
        case 0x4000:
        case 0x8000:
            PrSfx::PlayNavigate();
            return;
        case 0x20:
            PrSfx::PlayConfirm();
            return;
        case 0x40:
            PrSfx::PlayCancel();
            return;
        default:
            break;
    }

    double freq = 880.0;
    double sec = 0.04;
    double amp = 6000.0;

    switch (code) {
        case 0x20:
            freq = 740.0;
            sec = 0.05;
            break;
        case 0x40:
            freq = 1040.0;
            sec = 0.05;
            break;
        case 0x100:
            freq = 520.0;
            sec = 0.06;
            break;
        case 0x1000:
            freq = 660.0;
            sec = 0.03;
            amp = 5200.0;
            break;
        case 0x2000:
            freq = 660.0;
            sec = 0.03;
            amp = 5200.0;
            break;
        case 0x4000:
            freq = 620.0;
            sec = 0.03;
            amp = 5200.0;
            break;
        case 0x8000:
            freq = 620.0;
            sec = 0.03;
            amp = 5200.0;
            break;
        default:
            freq = 880.0;
            sec = 0.02;
            amp = 4500.0;
            break;
    }

    QueueTone(freq, sec, amp);
}

void PlayStage1Event4Cue80025E6C(uint8_t cueIndex) {
    PrSfx::PlayMovie1ShellCue9441CRaw(cueIndex);
    PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
}

void PlayStage1Event4InputCue80025C8C(uint16_t code) {
    PrSfx::PlayStage1UiCue80025C8CRaw(code);
    PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
}

void ResetStage1Event4CueTick800267C8() {
    s_event4CueRepeatCount80025E6C = 2;
    s_event4CueFrame80025E6C = 0;
}

void TickStage1Event4Cue80025E6C() {
    if (s_event4CueRepeatCount80025E6C <= 0) {
        return;
    }

    if (s_event4CueFrame80025E6C == 0) {
        PlayStage1Event4Cue80025E6C(0);
    } else if (s_event4CueFrame80025E6C == 36) {
        PlayStage1Event4Cue80025E6C(1);
        --s_event4CueRepeatCount80025E6C;
    }

    const int32_t nextFrame = s_event4CueFrame80025E6C + 1;
    s_event4CueFrame80025E6C =
        (s_event4CueFrame80025E6C == 72) ? 0 : nextFrame;
}

struct DispEntry;
typedef void (*DispInitFunc)(DispEntry* entry, PrEventId eventId, void* argPtr);
typedef int  (*DispHandleFunc)(uint16_t padMask, void* ctx, void* argPtr);
typedef void (*DispTickFunc)(DispEntry* entry);

struct DispEventDef {
    DispInitFunc   init;
    DispHandleFunc handle;
    DispTickFunc   tick;
    int            timeout;
    void*          default_ctx;
};

struct DispEntry {
    const DispEventDef* def;
    void* ctx;
    void* arg_ptr;
    int timeout;
};

static DispEntry s_dispEntry;
static PrPsxEventFrameDirect::EventFrameState8001E750
    s_event2FrameState8001E750;
static PrPsxEventFrameDirect::EventFrameState8001E750
    s_event4FrameState8001E750;

uint16_t MapDebugInputToPad(const PrGameContext& ctx) {
    // Real PrPad input (pressed = edge-triggered, for menu navigation)
    PrPadState padState = PrPad::GetState(0);
    uint16_t pad = MapLocalPrPadMaskToDispatcherPad(padState.pressed);

    // Debug: direct pad mask injection overrides
    if (ctx.debugPadInput != 0) {
        pad |= ctx.debugPadInput;
    }

    // Debug: debugGenericEvent mapping (legacy)
    int ev = ctx.debugGenericEvent;
    if (ev == 1) pad |= PAD_CROSS;
    if (ev == 2) pad |= PAD_CIRCLE;
    if (ev == 3) pad |= PAD_SQUARE;
    if (ev == 4) pad |= PAD_TRIANGLE;
    if (ev == 5) pad |= PAD_RIGHT;
    if (ev == 6) pad |= PAD_DOWN;
    if (ev == 7) pad |= PAD_LEFT;
    if (ev == 8) pad |= PAD_UP;
    return pad;
}

void DispatcherInit6Ex(DispEntry* entry, PrEventId eventId, void* argPtr) {
    entry->ctx = argPtr;
    Ev6Ctx* ev6 = (Ev6Ctx*)argPtr;
    ev6->blink = 0;
    ev6->done_flag = 0;
    s_dispFrameCountdown = 60;
    Log::Printf("Dispatcher ev=6 init");
}

int DispatcherHandle6Ex(uint16_t input, void* ctx, void* argPtr) {
    if (input == PAD_CROSS) {
        Ev6Ctx* ev6 = (Ev6Ctx*)ctx;
        ev6->done_flag = 1;
        PlayUiSfx(0x20);
        return 1; // confirmed -> proceed to play
    }
    if (input == PAD_CIRCLE || input == PAD_TRIANGLE) {
        PlayUiSfx(0x40);
        return 2; // cancelled -> back to main menu
    }
    return 0;
}

void DispatcherTick6Ex(DispEntry* entry) {
    Ev6Ctx* ev6 = (Ev6Ctx*)entry->ctx;
    BlinkTick(&ev6->blink);
}

void DispatcherInit11Ex(DispEntry* entry, PrEventId eventId, void* argPtr) {
    (void)eventId;
    (void)argPtr;
    entry->ctx = &s_dispCtx;
    s_dispCtx = PrEventDispatcherContext{};
    s_dispCtx.menuIndex = 0;
    s_dispCtx.menuCount = 2;
    s_dispCtx.selections[0] = 0;
    s_dispCtx.confirmFlag = 0;
    s_dispFrameCountdown = 20;
    Log::Printf("Dispatcher ev=11 init (Save? confirm)");
}

int DispatcherHandle11Ex(uint16_t input, void* ctx, void* argPtr) {
    (void)ctx;
    (void)argPtr;
    if (input == PAD_CROSS) {
        s_dispCtx.menuIndex = 0;
        s_dispCtx.selections[0] = 1;
        s_dispCtx.confirmFlag = 1;
        PlayUiSfx(0x20);
        return 1;
    }
    if (input == PAD_CIRCLE || input == PAD_TRIANGLE) {
        s_dispCtx.menuIndex = 1;
        s_dispCtx.selections[0] = 2;
        s_dispCtx.confirmFlag = 1;
        PlayUiSfx(0x40);
        return 2;
    }
    return 0;
}

void DispatcherTick11Ex(DispEntry* entry) {
    (void)entry;
    BlinkTick(&s_blinkBeat);
}

int32_t s_ev17Blink = 0;
int32_t s_ev2Blink = 0;

void DispatcherInit17Ex(DispEntry* entry, PrEventId eventId, void* argPtr) {
    s_dispCtx.menuIndex = 1;
    s_dispCtx.menuCount = 3;
    s_dispCtx.inputCooldown = 0;
    s_dispCtx.confirmFlag = 0;
    s_ev17Blink = 0;
    // PSX: opt0_value=(word_800916DC==0), opt1_value=word_800916D8
    if (s_dispGameCtxPtr) {
        s_dispCtx.selections[0] = (s_dispGameCtxPtr->subtitleFlag == 0) ? 1 : 0;
        s_dispCtx.selections[1] = s_dispGameCtxPtr->languageIndex;
    }
    s_dispFrameCountdown = 60;
    Log::Printf("Dispatcher ev=17 init lang=%d sub=%d timeout=%d",
                (int)s_dispCtx.selections[1], (int)s_dispCtx.selections[0], s_dispTimeout);
}

int DispatcherHandle17Ex(uint16_t input, void* ctx, void* argPtr) {
    if (s_dispCtx.inputCooldown > 0) return 0;
    int idx = s_dispCtx.menuIndex;
    switch (idx) {
        case 0:
            // PSX: Cross -> word_800916DC=1 (subtitle ON display), Circle -> =0
            if (input == PAD_CROSS) {
                s_dispCtx.selections[idx] = 0;
                if (s_dispGameCtxPtr) s_dispGameCtxPtr->subtitleFlag = 1;
                return 0;
            }
            if (input == PAD_CIRCLE) {
                s_dispCtx.selections[idx] = 1;
                if (s_dispGameCtxPtr) s_dispGameCtxPtr->subtitleFlag = 0;
                return 0;
            }
            break;
        case 1: {
            int sel = s_dispCtx.selections[1];
            if (input == PAD_RIGHT) {
                sel = (sel + 1) % 5;
                s_dispCtx.inputCooldown = 16;
            } else if (input == PAD_LEFT) {
                sel = (sel - 1 < 0) ? 4 : sel - 1;
                s_dispCtx.inputCooldown = 16;
            }
            s_dispCtx.selections[1] = (int16_t)sel;
            if (s_dispGameCtxPtr) s_dispGameCtxPtr->languageIndex = (int16_t)sel;
            break;
        }
        case 2:
            if (input == PAD_CROSS) {
                s_dispCtx.selections[idx] = 0;
                s_dispCtx.confirmFlag = 1;
                return 1;
            }
            break;
        default:
            break;
    }
    if (input == PAD_DOWN) {
        int cnt = s_dispCtx.menuCount;
        if (cnt > 0) s_dispCtx.menuIndex = (idx + 1) % cnt;
        return 0;
    }
    if (input == PAD_UP) {
        int cnt = s_dispCtx.menuCount;
        int newIdx = idx - 1;
        if (newIdx < 0) newIdx = cnt - 1;
        s_dispCtx.menuIndex = (int16_t)newIdx;
        return 0;
    }
    return 0;
}

void DispatcherTick17Ex(DispEntry* entry) {
    if (s_dispCtx.inputCooldown > 0) --s_dispCtx.inputCooldown;
    BlinkTick(&s_ev17Blink);
}

// S0_OLD_HANDOFF: legacy ev=2 stage-select dispatcher. SS0 owns selected
// scene/status handoff once cut over.
// ========== event=2: 选关菜单 ==========
void DispatcherInit2Ex(DispEntry* entry, PrEventId eventId, void* argPtr) {
    s_dispCtx.menuIndex = 1;
    s_dispCtx.confirmFlag = 0;
    s_dispCtx.outNextScene = -1;
    RefreshStageSelectDispatcherContext();
    PrPsxEventFrameDirect::ResetEventFrameState8003FB9C(
        s_event2FrameState8001E750,
        320u,
        240u);
    (void)PrPsxEventFrameDirect::PsxCall80027FAC_TextSystemBoot(
        s_event2FrameState8001E750);

    if (s_stageSelectForceEnableAllF0) {
        Log::Printf(
            "Dispatcher ev=2 init: F0 force-enable branch entries=[%d,%d,%d,%d,%d,%d,%d,%d]",
            (int)s_dispCtx.selections[1], (int)s_dispCtx.selections[2],
            (int)s_dispCtx.selections[3], (int)s_dispCtx.selections[4],
            (int)s_dispCtx.selections[5], (int)s_dispCtx.selections[6],
            (int)s_dispCtx.selections[7], (int)s_dispCtx.selections[8]);
    } else if (s_ev3RecordsModePtr && *s_ev3RecordsModePtr == 1) {
        Log::Printf("Dispatcher ev=2 init: RECORDS mode, only STAGE1+CANCEL");
    } else {
        Log::Printf(
            "Dispatcher ev=2 init: status=[%d,%d,%d,%d,%d,%d] bonus=%d",
            (int)s_dispCtx.selections[1], (int)s_dispCtx.selections[2], (int)s_dispCtx.selections[3],
            (int)s_dispCtx.selections[4], (int)s_dispCtx.selections[5], (int)s_dispCtx.selections[6],
            (int)s_dispCtx.selections[7]);
    }

    s_ev2Blink = 0;
    s_dispFrameCountdown = 60;
    Log::Printf("Dispatcher ev=2 init cursor=%d count=%d", (int)s_dispCtx.menuIndex, (int)s_dispCtx.menuCount);
}

int DispatcherHandle2Ex(uint16_t input, void* ctx, void* argPtr) {
    int16_t* outNextScenePtr = (int16_t*)argPtr;
    int cursor = s_dispCtx.menuIndex;

    auto moveForward = [&]() {
        const int count = 8;
        int next = cursor;
        for (int step = 0; step < count; ++step) {
            next = (next % count) + 1;
            if (s_dispCtx.selections[next] != 0) {
                s_dispCtx.menuIndex = (int16_t)next;
                break;
            }
        }
    };
    auto moveBackward = [&]() {
        const int count = 8;
        int next = cursor;
        for (int step = 0; step < count; ++step) {
            next = (next <= 1) ? count : (next - 1);
            if (s_dispCtx.selections[next] != 0) {
                s_dispCtx.menuIndex = (int16_t)next;
                break;
            }
        }
    };

    if (input == PAD_RIGHT || input == PAD_DOWN) {
        moveForward();
        return 0;
    }
    if (input == PAD_LEFT || input == PAD_UP) {
        moveBackward();
        return 0;
    }
    if (input == PAD_CROSS) {
        if (cursor >= 1 && cursor <= 7) {
            int scene = (cursor <= 6) ? cursor : 8;
            s_dispCtx.outNextScene = scene;
            if (outNextScenePtr) *outNextScenePtr = (int16_t)scene;
            if (cursor == 7) s_dispCtx.confirmFlag = 1;
            Log::Printf("Dispatcher ev=2 CROSS cursor=%d -> scene=%d", cursor, scene);
            return 1;
        }
        if (cursor == 8) {
            s_dispCtx.confirmFlag = 1;
            Log::Printf("Dispatcher ev=2 CROSS cursor=8 -> cancel");
            return 2;
        }
    }
    return 0;
}

void DispatcherTick2Ex(DispEntry* entry) {
    (void)entry;
    BlinkTick(&s_ev2Blink);

    PrPsxEventFrameDirect::StageSelectDrawInput80020568 stageSelectInput{};
    stageSelectInput.blinkFlag_00 = static_cast<int16_t>(s_ev2Blink);
    stageSelectInput.selected_04 = s_dispCtx.confirmFlag;
    stageSelectInput.cursor_08 = s_dispCtx.menuIndex;
    stageSelectInput.count_0A = s_dispCtx.menuCount;
    for (uint32_t i = 0; i < 6u; ++i) {
        stageSelectInput.status_0E[i] = s_dispCtx.selections[i + 1u];
    }
    stageSelectInput.bonusStatus_1A = s_dispCtx.selections[7];
    stageSelectInput.languageIndex_800916D8 =
        s_dispGameCtxPtr != nullptr
            ? static_cast<int16_t>(s_dispGameCtxPtr->languageIndex)
            : 0;

    PrPsxEventFrameDirect::PsxCall80026B94_EventFrameTail(
        s_event2FrameState8001E750,
        2,
        0,
        &stageSelectInput,
        nullptr);
    PrPsxEventFrameDirect::PsxConsume80035560_WaitFrameHostVblank(
        s_event2FrameState8001E750,
        1);
}

// ========== event=3: 主菜单 ==========
static int s_ev3Blink = 0;

void DispatcherInit3Ex(DispEntry* entry, PrEventId eventId, void* argPtr) {
    int16_t* transPtr = (int16_t*)argPtr;
    s_dispCtx.menuIndex = 3;
    s_dispCtx.menuCount = 5;
    for (int i = 0; i < 5; ++i) s_dispCtx.selections[i] = 0;
    s_dispCtx.selections[1] = -1;
    // PSX: per-item state words are not preselecting PRACTICE on entry.
    // Use -1 to represent "no sub-choice yet" for Button3.
    s_dispCtx.selections[3] = -1;
    s_dispCtx.confirmFlag = 0;
    s_ev3Blink = 0;
    s_ev3RecordsModePtr = nullptr;
    (void)transPtr;
    s_dispFrameCountdown = 60;
    Log::Printf("Dispatcher ev=3 init menuIndex=%d", (int)s_dispCtx.menuIndex);
}

int DispatcherHandle3Ex(uint16_t input, void* ctx, void* argPtr) {
    if (PrTransition::IsActive()) {
        return 0;
    }
    int idx = s_dispCtx.menuIndex;
    switch (idx) {
        case 0:
            if (input == PAD_CROSS) {
                s_dispCtx.selections[idx] = 1;
                return 8;
            }
            break;
        case 1:
            if (input == PAD_CROSS) {
                s_dispCtx.selections[idx] = 0;
                return 1;
            }
            break;
        case 2: // RECORDS
            if (input == PAD_CROSS) {
                s_dispCtx.selections[idx] = 0;
                if (s_ev3RecordsModePtr) *s_ev3RecordsModePtr = 0;
                return 0; // PSX: stays in dispatcher, doesn't exit
            }
            if (input == PAD_CIRCLE) {
                s_dispCtx.selections[idx] = 1;
                if (s_ev3RecordsModePtr) *s_ev3RecordsModePtr = 1;
                return 0;
            }
            break;
        case 3:
            if (input == PAD_SQUARE) {
                s_dispCtx.selections[idx] = 0;
                return 3;
            }
            if (input == PAD_CROSS) {
                s_dispCtx.selections[idx] = 1;
                return 4;
            }
            if (input == PAD_CIRCLE) {
                s_dispCtx.selections[idx] = 2;
                return 2;
            }
            if (input == PAD_TRIANGLE) {
                s_dispCtx.selections[idx] = 3;
                return 6;
            }
            break;
        case 4:
            if (input == PAD_CROSS) {
                s_dispCtx.selections[idx] = 0;
                s_dispCtx.confirmFlag = 1;
                return 7;
            }
            break;
        default:
            break;
    }
    if (input == PAD_RIGHT || input == PAD_DOWN) {
        int cnt = s_dispCtx.menuCount;
        if (cnt > 0) s_dispCtx.menuIndex = (idx + 1) % cnt;
        return 0;
    }
    if (input == PAD_LEFT || input == PAD_UP) {
        int cnt = s_dispCtx.menuCount;
        int newIdx = idx - 1;
        if (newIdx < 0) newIdx = cnt - 1;
        s_dispCtx.menuIndex = (int16_t)newIdx;
        return 0;
    }
    return 0;
}

void DispatcherTick3Ex(DispEntry* entry) {
    BlinkTick(&s_ev3Blink);
}

// ========== event=4: 暂停/退出菜单 ==========
void DispatcherInit4Ex(DispEntry* entry, PrEventId eventId, void* argPtr) {
    s_dispCtx.pauseChoice = -1;
    s_dispCtx.confirmFlag = 0;
    s_dispFrameCountdown = 60;
    ResetStage1Event4CueTick800267C8();
    PrPsxEventFrameDirect::ResetEventFrameState8003FB9C(
        s_event4FrameState8001E750,
        320u,
        240u);

    uint32_t stage1Event4SeedGp38C = 0u;
    if (s_dispGameCtxPtr != nullptr &&
        s_dispGameCtxPtr->currentScene == PrSceneId::Scene1 &&
        PrPsxEventFrameDirect::ConsumeStage1Event4Gp38CSeed8006EDCC(
            stage1Event4SeedGp38C)) {
        s_event4FrameState8001E750.gp38CEvent4State =
            stage1Event4SeedGp38C;
    }
    Log::Printf("Dispatcher ev=4 init");
}

int DispatcherHandle4Ex(uint16_t input, void* ctx, void* argPtr) {
    if (input == PAD_CROSS) {
        PlayStage1Event4InputCue80025C8C(0x20u);
        s_dispCtx.pauseChoice = 0;
        Log::Printf("Dispatcher ev=4 CROSS -> continue");
        return 1;
    }
    if (input == PAD_CIRCLE) {
        PlayStage1Event4InputCue80025C8C(0x40u);
        s_dispCtx.pauseChoice = 1;
        Log::Printf("Dispatcher ev=4 CIRCLE -> exit");
        return 2;
    }
    return 0;
}

void DispatcherTick4Ex(DispEntry* entry) {
    (void)entry;
    TickStage1Event4Cue80025E6C();
    const int32_t promptCtx0 =
        (s_dispCtx.pauseChoice == 0 || s_dispCtx.pauseChoice == 1)
            ? s_dispCtx.pauseChoice
            : -1;
    PrPsxEventFrameDirect::PsxCall80026B94_EventFrameTail(
        s_event4FrameState8001E750,
        4,
        promptCtx0,
        nullptr,
        nullptr);
    PrPsxEventFrameDirect::PsxConsume80035560_WaitFrameHostVblank(
        s_event4FrameState8001E750,
        1);
}

void DispatcherInit5Ex(DispEntry* entry, PrEventId eventId, void* argPtr) {
    s_dispCtx.menuIndex = 0;
    s_dispCtx.menuCount = 1;
    s_dispCtx.confirmFlag = 0;
    s_dispFrameCountdown = 60;
    Log::Printf("Dispatcher ev=5 init");
}

int DispatcherHandle5Ex(uint16_t input, void* ctx, void* argPtr) {
    if (input == PAD_CROSS) {
        PlayUiSfx(0x20);
        return 16;
    }
    if (input == PAD_CIRCLE || input == PAD_TRIANGLE) {
        PlayUiSfx(0x40);
        return 2;
    }
    return 0;
}

void DispatcherTick5Ex(DispEntry* entry) {
}

// S0_OLD_HANDOFF: legacy ev=7/8/9 memcard dispatcher. SS0 must carry
// slot UI, card HAL result, and replay scene resolution as one closure.
// ========== ev=7/8/9: Memory Card (SAVE/LOAD/REPLAY) ==========
static PrEvMemCardCtx s_evMemCardCtx;
static bool s_evMemCardArgBacked = false;

static bool LoadMemCardCtxFromPsxArg(const void* argPtr, PrEvMemCardCtx& out) {
    if (!argPtr) return false;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(argPtr);

    auto readS16 = [&](size_t off) -> int16_t {
        return (int16_t)(p[off] | (p[off + 1] << 8));
    };
    auto readU32 = [&](size_t off) -> uint32_t {
        return (uint32_t)p[off]
            | ((uint32_t)p[off + 1] << 8)
            | ((uint32_t)p[off + 2] << 16)
            | ((uint32_t)p[off + 3] << 24);
    };

    const int16_t rows = readS16(12);
    const int16_t cols = readS16(14);
    const int16_t itemCount = readS16(18);
    const int16_t selected = readS16(20);
    if (rows <= 0 || rows > 5 || cols <= 0 || cols > 3 || itemCount <= 0 || itemCount > 16) {
        return false;
    }

    out.rows = rows;
    out.cols = cols;
    out.itemCount = itemCount;
    out.selected = selected;
    out.exitFrameOn = readU32(0) ? 1 : 0;
    out.exitTextOn = readU32(4) ? 1 : 0;
    out.ioMessage = readU32(8) ? ((out.eventId == 7) ? 2 : 0) : -1;
    for (int i = 0; i < 15; ++i) {
        out.enabled[i] = readS16(22 + i * 2) ? 1 : 0;
        out.entryBlockIndex[i] = (int16_t)i;
        out.slotText[i][0] = '\0';
        const char* src = reinterpret_cast<const char*>(p + 120 + i * 32);
        if (src[0] != '\0') {
            strncpy_s(out.slotText[i], src, _TRUNCATE);
        }
    }
    out.exitSelected = (selected == rows * cols) ? 1 : 0;
    return true;
}

void DispatcherInitMemCardEx(DispEntry* entry, PrEventId eventId, void* argPtr) {
    s_evMemCardCtx = PrEvMemCardCtx{};
    s_evMemCardCtx.eventId = (int32_t)eventId;
    s_evMemCardCtx.rows = 5;
    s_evMemCardCtx.cols = 3;
    s_evMemCardCtx.itemCount = 15;   // grid item count; EXIT is rows*cols
    s_evMemCardCtx.selected = 15;    // PSX load-page baseline dump: EXIT selected
    s_evMemCardCtx.exitFrameOn = 0;
    s_evMemCardCtx.exitTextOn = 0;
    s_evMemCardCtx.ioMessage = -1;
    s_evMemCardCtx.exitSelected = 0;
    s_evMemCardCtx.replayResolvedSlot = -1;
    s_evMemCardCtx.replayResolvedScene = -1;
    const bool loadedFromArg = LoadMemCardCtxFromPsxArg(argPtr, s_evMemCardCtx);
    s_evMemCardArgBacked = loadedFromArg;
    if (!loadedFromArg) {
        PrMemCardBackend::SlotInfo slots[15] = {};
        const bool hasSlots = PrMemCardBackend::EnumerateSlots(slots);
        for (int i = 0; i < 15; ++i) {
            s_evMemCardCtx.enabled[i] = 0;
            s_evMemCardCtx.entryBlockIndex[i] = -1;
            s_evMemCardCtx.slotText[i][0] = '\0';
        }

        if ((int)eventId == 8) {
            PrMemCardBackend::EntryInfo entries[15] = {};
            const int count = PrMemCardBackend::EnumerateEntriesCompact(entries);
            if (count > 0) {
                s_evMemCardCtx.itemCount = (int16_t)count;
                s_evMemCardCtx.selected = 0;
                for (int i = 0; i < count; ++i) {
                    s_evMemCardCtx.enabled[i] = 1;
                    s_evMemCardCtx.entryBlockIndex[i] =
                        (int16_t)entries[i].blockIndex;
                    strncpy_s(s_evMemCardCtx.slotText[i], entries[i].title.c_str(), _TRUNCATE);
                }
            } else {
                // Empty-card baseline currently observed/used for LOAD fallback.
                s_evMemCardCtx.itemCount = 15;
                s_evMemCardCtx.selected = 15;
                for (int i = 0; i < 15; ++i) {
                    s_evMemCardCtx.enabled[i] = 1;
                }
            }
        } else {
            for (int i = 0; i < 15; ++i) {
                s_evMemCardCtx.enabled[i] = hasSlots ? (slots[i].occupied ? 1 : 0) : 1;
                s_evMemCardCtx.entryBlockIndex[i] = (int16_t)i;
                if (hasSlots && !slots[i].title.empty()) {
                    strncpy_s(s_evMemCardCtx.slotText[i], slots[i].title.c_str(), _TRUNCATE);
                }
            }
        }
    } else {
        Log::Printf("MemCard ev=%d loaded raw arg rows=%d cols=%d items=%d selected=%d io=%d",
                    (int)eventId,
                    (int)s_evMemCardCtx.rows,
                    (int)s_evMemCardCtx.cols,
                    (int)s_evMemCardCtx.itemCount,
                    (int)s_evMemCardCtx.selected,
                    (int)s_evMemCardCtx.ioMessage);
    }
    s_dispCtx.menuIndex = s_evMemCardCtx.selected;
    s_dispCtx.menuCount = 16; // fixed 3x5 grid + EXIT navigation space
    s_dispCtx.confirmFlag = 0;
    s_dispFrameCountdown = 60;
    const char* name = (eventId == 7) ? "SAVE" : (eventId == 8) ? "LOAD" : "REPLAY";
    const char* firstTitle = "";
    for (int i = 0; i < 15; ++i) {
        if (s_evMemCardCtx.slotText[i][0] != '\0') {
            firstTitle = s_evMemCardCtx.slotText[i];
            break;
        }
    }
    Log::Printf("Dispatcher ev=%d (%s) init rows=%d cols=%d items=%d selected=%d io=%d",
                (int)eventId, name,
                (int)s_evMemCardCtx.rows, (int)s_evMemCardCtx.cols,
                (int)s_evMemCardCtx.itemCount, (int)s_evMemCardCtx.selected,
                (int)s_evMemCardCtx.ioMessage);
    Log::Printf("MemCard ev=%d firstTitle='%s'", (int)eventId, firstTitle);
}

int DispatcherHandleMemCardEx(uint16_t input, void* ctx, void* argPtr) {
    auto& mc = s_evMemCardCtx;
    const int prevIdx = (int)s_dispCtx.menuIndex;
    const int idx = (int)s_dispCtx.menuIndex;
    const int kExitIdx = 15;
    const int kGridCols = 3;
    const int kGridSlots = std::clamp((int)mc.itemCount, 0, 15);

    if (input == PAD_UP) {
        if (idx == kExitIdx) {
            if (kGridSlots > 0) {
                s_dispCtx.menuIndex = (int16_t)(kGridSlots - 1);
                PlayUiSfx(0x10);
            }
        } else if (idx >= kGridCols && idx < kGridSlots) {
            s_dispCtx.menuIndex -= kGridCols;
            PlayUiSfx(0x10);
        }
    } else if (input == PAD_DOWN) {
        if (idx >= 0 && idx < kGridSlots) {
            if (idx + kGridCols < kGridSlots) {
                s_dispCtx.menuIndex += kGridCols;
                PlayUiSfx(0x10);
            } else {
                // Bottom visible row -> EXIT
                s_dispCtx.menuIndex = (int16_t)kExitIdx;
                PlayUiSfx(0x10);
            }
        }
    } else if (input == PAD_LEFT) {
        if (idx > 0 && idx < kGridSlots) {
            s_dispCtx.menuIndex--;
            PlayUiSfx(0x10);
        }
    } else if (input == PAD_RIGHT) {
        if (idx >= 0 && idx + 1 < kGridSlots) {
            s_dispCtx.menuIndex++;
            PlayUiSfx(0x10);
        }
    } else if (input == PAD_CROSS) {
        if (idx == kExitIdx) {
            // EXIT selected -> cancel
            mc.exitFrameOn = 1;
            mc.exitTextOn = 0;
            mc.exitSelected = 1;
            PlayUiSfx(0x40);
            return 2;
        }
        if (idx < 0 || idx >= kGridSlots) {
            return 0;
        }
        // Confirm slot selection
        mc.selected = (int16_t)idx;
        mc.exitFrameOn = 0;
        mc.exitTextOn = 0;
        mc.exitSelected = 0;
        PlayUiSfx(0x20);
        Log::Printf("MemCard ev=%d slot=%d confirmed", mc.eventId, idx);
        return 1; // result=1: confirmed
    } else if (input == PAD_CIRCLE || input == PAD_TRIANGLE) {
        mc.exitFrameOn = (idx == kExitIdx) ? 1 : 0;
        mc.exitTextOn = 0;
        mc.exitSelected = (idx == kExitIdx) ? 1 : 0;
        PlayUiSfx(0x40);
        return 2; // result=2: cancelled
    }

    if (idx >= kGridSlots && idx != kExitIdx) {
        if (kGridSlots > 0) {
            s_dispCtx.menuIndex = (int16_t)(kGridSlots - 1);
        } else {
            s_dispCtx.menuIndex = (int16_t)kExitIdx;
        }
    }
    mc.selected = (int16_t)s_dispCtx.menuIndex;
    mc.exitFrameOn = ((int)s_dispCtx.menuIndex == kExitIdx) ? 1 : 0;
    mc.exitTextOn = 0;
    mc.exitSelected = ((int)s_dispCtx.menuIndex == kExitIdx) ? 1 : 0;
    if ((int)s_dispCtx.menuIndex != prevIdx) {
        Log::Printf("MemCard ev=%d select=%d exit=%d",
                    mc.eventId, (int)s_dispCtx.menuIndex, mc.exitSelected);
    }
    return 0;
}

void DispatcherTickMemCardEx(DispEntry* entry) {
    (void)entry;
}

void DispatcherInit16Ex(DispEntry* entry, PrEventId eventId, void* argPtr) {
    s_ev16Ctx.frame = -1;
    s_ev16Ctx.tick = 0;
    s_ev16Ctx.round = 0;

    s_ev16PostStage = 0;
    s_ev16ExitCountdown = 0;
    s_ev16Phase = Ev16Phase::Idle;
    ResetEv16JudgePending();
    s_ev16Ctx.frameInRound = -1;
    s_ev16Ctx.mode = 1;
    s_ev16Ctx.hits = 0;
    s_ev16Ctx.hitGood = 0;
    s_ev16Ctx.hitEarly = 0;
    s_ev16Ctx.hitLate = 0;
    s_ev16Ctx.flags00 = 0;
    s_ev16Ctx.state1C = 0;
    s_ev16Ctx.blink4C = 0;
    s_ev16Ctx.exit48 = 0;
    s_ev16Ctx.padStopFrames = 0;

    ResetEv16SeqState();

    if (s_dispGameCtxPtr) {
        (void)LoadEv16Rec44Streams(*s_dispGameCtxPtr);
    }
    RefreshEv16RoundStreams();

    s_ev16Ctx.round = 0;

    PrSfx::StopBgm();

    if (s_dispGameCtxPtr) {
        const std::filesystem::path ycompoInt = s_dispGameCtxPtr->dataRoot / "S0" / "YCOMPO.INT";
        (void)PrSfx::InitPracticeVabFromInt(ycompoInt.u8string());
        if (s_dispGameCtxPtr->resources) {
            const bool ok = s_dispGameCtxPtr->resources->LoadIntArchiveTimOnly(ycompoInt.u8string(), "ycompo:");
            Log::Printf("Practice: LoadIntArchive(YCOMPO) ok=%d", ok ? 1 : 0);

            const int hasKtSi = s_dispGameCtxPtr->resources->GetTexture("ycompo:kt_si") ? 1 : 0;
            const int hasPaSi = s_dispGameCtxPtr->resources->GetTexture("ycompo:pa_si") ? 1 : 0;
            Log::Printf("Practice: ycompo portraits kt_si=%d pa_si=%d", hasKtSi, hasPaSi);

            const std::filesystem::path guiCompo = s_dispGameCtxPtr->dataRoot / "S1" / "COMPO01.INT";
            const bool okGui = s_dispGameCtxPtr->resources->LoadIntArchiveTimOnly(guiCompo.u8string(), "compo:");
            Log::Printf("Practice: LoadIntArchive(COMPO01) ok=%d", okGui ? 1 : 0);
        }
    }

    s_dispCtx.menuIndex = 0;
    s_dispCtx.menuCount = 2;
    s_dispCtx.confirmFlag = 0;
    s_dispFrameCountdown = 60;
    StartEv16RoundIntro();
    Log::Printf("Dispatcher ev=16 init -> auto start practice");
}

int DispatcherHandle16Ex(uint16_t input, void* ctx, void* argPtr) {
    (void)ctx;
    (void)argPtr;

    if (s_ev16PostStage == 3) {
        if (input == PAD_CIRCLE) {
            s_ev16PostStage = 0;
            s_ev16ExitCountdown = 0;
            s_ev16PendingRound = -1;
            s_ev16Ctx.round = 0;
            s_ev16Ctx.frame = -1;
            s_ev16Ctx.frameInRound = -1;
            s_ev16Ctx.tick = 0;
            s_ev16Ctx.exit48 = 0;
            s_ev16Ctx.blink4C = 0;
            s_ev16Ctx.flags00 = 0;
            s_ev16Ctx.state1C = 0;
            s_ev16Phase = Ev16Phase::Idle;
            ResetEv16JudgePending();
            RefreshEv16RoundStreams();
            ResetEv16SeqState();
            StartEv16RoundIntro();
            PlayUiSfx(0x20);
            Log::Printf("Practice ev=16 exitConfirm: Circle -> continue");
            return 0;
        }
        if (input == PAD_CROSS) {
            s_ev16Ctx.exit48 = 1;
            s_ev16Ctx.blink4C = 1;
            PlayUiSfx(0x20);
            Log::Printf("Practice ev=16 exitConfirm: Cross -> exit");
            return 1;
        }
        return 0;
    }

    if (s_ev16Ctx.padStopFrames > 0) {
        if (input == PAD_CROSS) {
            s_ev16Ctx.exit48 = 1;
            s_ev16Ctx.blink4C = 1;
            PlayUiSfx(0x20);
            if (s_ev16Ctx.state1C == 8 && (s_ev16Ctx.flags00 & 0x00400000) != 0) {
                return 1;
            }
        }
        return 0;
    }

    if (input == PAD_UP || input == PAD_DOWN) {
        const int16_t idx = s_dispCtx.menuIndex;
        s_dispCtx.menuIndex = (idx == 0) ? 1 : 0;
        Log::Printf("Practice ev=16 menu: %s -> menuIndex=%d mode=%d post=%d ex=%d st=%d fl=0x%08X",
                    (input == PAD_UP) ? "UP" : "DOWN",
                    (int)s_dispCtx.menuIndex,
                    (int)s_ev16Ctx.mode,
                    (int)s_ev16PostStage,
                    (int)s_ev16Ctx.exit48,
                    (int)s_ev16Ctx.state1C,
                    (unsigned)s_ev16Ctx.flags00);
        PlayUiSfx(0x10);
        return 0;
    }

    if (input == PAD_CROSS && s_ev16Ctx.mode == 0) {
        if (s_dispCtx.menuIndex == 0) {
            s_ev16Ctx.mode = 1;
            s_ev16Phase = Ev16Phase::Idle;
            ResetEv16JudgePending();
            s_ev16PendingRound = -1;
            s_ev16Ctx.round = 0;
            s_ev16Ctx.frame = -1;
            s_ev16Ctx.frameInRound = -1;
            s_ev16Ctx.tick = 0;
            s_ev16Ctx.hits = 0;
            s_ev16Ctx.hitGood = 0;
            s_ev16Ctx.hitEarly = 0;
            s_ev16Ctx.hitLate = 0;
            s_ev16Ctx.flags00 = 0;
            s_ev16Ctx.state1C = 0;
            s_ev16Ctx.blink4C = 0;
            s_ev16Ctx.exit48 = 0;
            s_ev16Ctx.padStopFrames = 0;

            RefreshEv16RoundStreams();
            ResetEv16SeqState();
            StartEv16RoundIntro();
            PlayUiSfx(0x20);
            Log::Printf("Practice ev=16 menu: Cross LISTEN -> mode=%d post=%d mi=%d mc=%d",
                        (int)s_ev16Ctx.mode,
                        (int)s_ev16PostStage,
                        (int)s_dispCtx.menuIndex,
                        (int)s_dispCtx.menuCount);
            return 0;
        }
        PlayUiSfx(0x20);
        StartEv16ExitConfirmIntro("menu exit");
        Log::Printf("Practice ev=16 menu: Cross EXIT -> exitConfirm post=%d mi=%d mc=%d",
                    (int)s_ev16PostStage,
                    (int)s_dispCtx.menuIndex,
                    (int)s_dispCtx.menuCount);
        return 0;
    }

    if (input == PAD_CROSS && s_ev16Ctx.mode != 0 && s_dispCtx.menuIndex != 0) {
        PlayUiSfx(0x20);
        StartEv16ExitConfirmIntro("menu active exit");
        Log::Printf("Practice ev=16 menu(active): Cross EXIT -> exitConfirm post=%d mi=%d mc=%d",
                    (int)s_ev16PostStage,
                    (int)s_dispCtx.menuIndex,
                    (int)s_dispCtx.menuCount);
        return 0;
    }

    if (s_ev16Ctx.mode != 0 && s_dispCtx.menuIndex == 0) {
        if (s_ev16Ctx.padStopFrames > 0) {
            return 0;
        }

        if (input == PAD_CROSS) {
            if (s_ev16Phase == Ev16Phase::Judge) {
                ResetEv16JudgePending();
                s_ev16JudgeAbortToExit = true;
                s_ev16Ctx.frameInRound = kEv16FastForwardFrame - 1;
                s_ev16Ctx.frame = kEv16FastForwardFrame - 1;
                PlayUiSfx(0x20);
                Log::Printf("Practice ev=16 Cross abort -> exitConfirm tail fr=%d", (int)s_ev16Ctx.frameInRound);
            }
            return 0;
        }

        if (input == PAD_TRIANGLE && s_ev16Phase == Ev16Phase::Judge) {
            if (s_ev16JudgePending) {
                return 0;
            }
            const int r = (int)s_ev16Ctx.round & 3;
            const int v37 = kEv16TargetOffsets[r];
            int v12 = s_ev16Ctx.frameInRound;
            if (v12 < 0) v12 = 0;

            const int nonJudgeEnd = kEv16BeatFrames - 2;
            if (v12 < nonJudgeEnd) {
                return 0;
            }

            int kind = -1;
            if (v12 < v37 + 73) kind = 1;
            else if (v12 > v37 + 77) kind = 2;
            else kind = 0;

            s_ev16JudgePending = true;
            s_ev16JudgePendingKind = kind;
            s_ev16JudgePendingRound = (int)s_ev16Ctx.round;

            PrSfx::PlayPracticePrompt(r);
            return 0;
        }
    }

    return 0;
}

void DispatcherTick16Ex(DispEntry* entry) {
    (void)entry;

    if (s_ev16PendingVoiceKind >= 0) {
        if (s_ev16PendingVoiceDelay > 0) {
            s_ev16PendingVoiceDelay -= 1;
        }
        if (s_ev16PendingVoiceDelay <= 0) {
            PrSfx::PlayPracticeVoiceKind((int)s_ev16PendingVoiceKind);
            s_ev16PendingVoiceKind = -1;
            s_ev16PendingVoiceDelay = 0;
        }
    }

    if (s_ev16Ctx.mode == 0) {
        return;
    }

    if (s_ev16Ctx.padStopFrames > 0) {
        s_ev16Ctx.padStopFrames -= 1;
        BlinkTick(&s_ev16Ctx.blink4C);
        if (s_ev16Ctx.padStopFrames <= 0) {
            s_ev16Ctx.padStopFrames = 0;
            s_ev16Ctx.flags00 = 0;
            s_ev16Ctx.blink4C = 0;
            s_ev16Ctx.exit48 = 0;

            if (s_ev16PostStage == 1) {
                s_ev16PostStage = 2;
                s_ev16Ctx.padStopFrames = 95;
                s_ev16Ctx.flags00 = 0x00400000;
                s_ev16Ctx.state1C = 8;
                s_ev16Ctx.blink4C = 0;
                PrSfx::PlayPracticeVoiceKind(8);
            } else if (s_ev16PostStage == 2) {
                s_ev16PostStage = 3;
                s_ev16ExitCountdown = 15;
                s_ev16Ctx.exit48 = 0;
                s_ev16Ctx.blink4C = 0;
                s_ev16Ctx.flags00 = 0x00400000;
                s_ev16Ctx.state1C = 8;
            } else if (s_ev16Phase == Ev16Phase::RoundIntro) {
                StartEv16PreviewPhase();
            } else if (s_ev16Phase == Ev16Phase::ResultOverlay) {
                StartEv16RoundIntro();
            }
        }
        return;
    }

    if (s_ev16PostStage == 3) {
        if (s_ev16ExitCountdown > 0) {
            s_ev16ExitCountdown -= 1;
            BlinkTick(&s_ev16Ctx.blink4C);
            if (s_ev16ExitCountdown <= 0) {
                s_ev16ExitCountdown = 15;
            }
        }

        // Keep exitConfirm overlay visible.
        s_ev16Ctx.flags00 = 0x00400000;
        s_ev16Ctx.state1C = 8;
        return;
    }

    if (s_dispCtx.menuIndex != 0) {
        return;
    }

    if (s_ev16Phase == Ev16Phase::Idle) {
        return;
    }

    ++s_ev16Ctx.frame;
    ++s_ev16Ctx.frameInRound;

    if (s_ev16Phase == Ev16Phase::Preview) {
        AdvanceEv16SequenceFrame((int)s_ev16Ctx.frameInRound);
        if (s_ev16Ctx.frameInRound >= kEv16PreviewEndFrame) {
            StartEv16JudgePhase();
        }
        return;
    }

    if (s_ev16Phase != Ev16Phase::Judge) {
        return;
    }

    if (s_ev16JudgeAbortToExit) {
        if (s_ev16Ctx.frameInRound >= (kEv16FramesPerRound - 1)) {
            StartEv16ExitConfirmIntro("judge abort");
        }
        return;
    }

    if (!s_ev16JudgePending && s_ev16Ctx.frameInRound >= kEv16FramesPerRound) {
        s_ev16Ctx.frameInRound = 0;
    }

    const int r = (int)s_ev16Ctx.round & 3;
    const int v37 = kEv16TargetOffsets[r];
    if (!s_ev16JudgePending && (s_ev16Ctx.frameInRound % kEv16BeatFrames) == 0) {
        ++s_ev16Ctx.tick;
        PrSfx::PlayPracticeBeat();
    }
    {
        if (!s_ev16JudgePending && s_ev16Ctx.frameInRound == v37) {
            PrSfx::PlayPracticePrompt(r);
        }
    }

    AdvanceEv16SequenceFrame((int)s_ev16Ctx.frameInRound);

    if (s_ev16JudgePending && s_ev16Ctx.frameInRound >= (kEv16FramesPerRound - 1)) {
        ResolveEv16JudgeResult((int)s_ev16JudgePendingKind, (int)s_ev16JudgePendingRound);
    }
}

// S0_OLD_BOUNDARY: legacy dispatcher event table for S0 menu flows.
// Treat entries as side-effect ledger seeds, not SS0 helper APIs.
// ========== Event Table ==========
const DispEventDef s_eventTable[] = {
    {nullptr, nullptr, nullptr, 0, nullptr},                                    // 0: unused
    {nullptr, nullptr, nullptr, 0, nullptr},                                    // 1: unused
    {DispatcherInit2Ex, DispatcherHandle2Ex, DispatcherTick2Ex, 0, &s_dispCtx}, // 2: 选关
    {DispatcherInit3Ex, DispatcherHandle3Ex, DispatcherTick3Ex, 0, &s_dispCtx}, // 3: 主菜单
    {DispatcherInit4Ex, DispatcherHandle4Ex, DispatcherTick4Ex, 0, &s_dispCtx}, // 4: 暂停
    {DispatcherInit5Ex, DispatcherHandle5Ex, DispatcherTick5Ex, 0, &s_ev5Ctx},  // 5
    {DispatcherInit6Ex, DispatcherHandle6Ex, DispatcherTick6Ex, 0, &s_ev6Ctx},  // 6: 确认提示
    {DispatcherInitMemCardEx, DispatcherHandleMemCardEx, DispatcherTickMemCardEx, 0, &s_evMemCardCtx}, // 7: SAVE
    {DispatcherInitMemCardEx, DispatcherHandleMemCardEx, DispatcherTickMemCardEx, 0, &s_evMemCardCtx}, // 8: LOAD
    {DispatcherInitMemCardEx, DispatcherHandleMemCardEx, DispatcherTickMemCardEx, 0, &s_evMemCardCtx}, // 9: REPLAY
    {nullptr, nullptr, nullptr, 0, nullptr},
    {DispatcherInit11Ex, DispatcherHandle11Ex, DispatcherTick11Ex, 0, &s_dispCtx}, // 11: SAVE confirm
    {nullptr, nullptr, nullptr, 0, nullptr},
    {nullptr, nullptr, nullptr, 0, nullptr},
    {nullptr, nullptr, nullptr, 0, nullptr},
    {nullptr, nullptr, nullptr, 0, nullptr},
    {DispatcherInit16Ex, DispatcherHandle16Ex, DispatcherTick16Ex, 0, &s_ev16Ctx},
    {DispatcherInit17Ex, DispatcherHandle17Ex, DispatcherTick17Ex, 1200, &s_dispCtx},
};
const int s_eventTableSize = sizeof(s_eventTable) / sizeof(s_eventTable[0]);

const DispEventDef* GetEventDef(PrEventId eventId) {
    if (eventId < (PrEventId)s_eventTableSize) return &s_eventTable[eventId];
    return nullptr;
}

}

void PrEvent::Init() {
    s_dispState = PrDispatcherState::Idle;
    s_dispEventId = 0;
    s_dispResult = 0;
    s_dispTimeout = 0;
    s_dispTimeoutInit = 0;
    s_dispFrameCountdown = 0;
    s_dispLastInput = 0;
    s_dispWaitingInput = true;
    s_dispNoInputYet = true;
    s_dispArgPtr = nullptr;
    s_dispGameCtxPtr = nullptr;
    s_dispCtx = PrEventDispatcherContext{};
    s_dispEntry = DispEntry{};
    s_blinkBeat = 0;
    s_ev3RecordsModePtr = nullptr;
    s_ev6Ctx = Ev6Ctx{};
    s_ev5Ctx = Ev5Ctx{};
    s_ev16Ctx = Ev16Ctx{};
    s_ev16PendingRound = -1;
    s_ev17Blink = 0;
    s_ev2Blink = 0;
    s_ev16PendingVoiceKind = -1;
    s_ev16PendingVoiceDelay = 0;
    s_stageSelectStatusInit = false;
    PrPsxEventFrameDirect::ResetStage1Event4Gp38CSeed8006EDCC();
    std::fill(std::begin(s_stageSelectStatus), std::end(s_stageSelectStatus), (int16_t)0);
}

void PrEvent::ClearDispatcherResidueForDirectCutover() {
    const bool hadResidue =
        s_dispState != PrDispatcherState::Idle ||
        s_dispEventId != 0;
    if (hadResidue) {
        Log::Printf(
            "PrEvent legacy dispatcher residue cleared for SS0 direct cutover ev=%u state=%u",
            static_cast<unsigned>(s_dispEventId),
            static_cast<unsigned>(s_dispState));
    }

    s_dispState = PrDispatcherState::Idle;
    s_dispEventId = 0;
    s_dispResult = 0;
    s_dispTimeout = 0;
    s_dispTimeoutInit = 0;
    s_dispFrameCountdown = 0;
    s_dispLastInput = 0;
    s_dispWaitingInput = true;
    s_dispNoInputYet = true;
    s_dispArgPtr = nullptr;
    s_dispGameCtxPtr = nullptr;
    s_dispCtx = PrEventDispatcherContext{};
    s_dispEntry = DispEntry{};
    s_blinkBeat = 0;
    s_ev3RecordsModePtr = nullptr;
    s_ev6Ctx = Ev6Ctx{};
    s_ev5Ctx = Ev5Ctx{};
    s_ev16Ctx = Ev16Ctx{};
    s_ev16PostStage = 0;
    s_ev16ExitCountdown = 0;
    s_ev16Phase = Ev16Phase::Idle;
    s_ev16PendingRound = -1;
    s_ev16JudgePending = false;
    s_ev16JudgeAbortToExit = false;
    s_ev16JudgePendingKind = -1;
    s_ev16JudgePendingRound = -1;
    s_ev16PendingVoiceKind = -1;
    s_ev16PendingVoiceDelay = 0;
    s_ev17Blink = 0;
    s_ev2Blink = 0;
    s_event4CueRepeatCount80025E6C = 0;
    s_event4CueFrame80025E6C = 0;
    s_event2FrameState8001E750 =
        PrPsxEventFrameDirect::EventFrameState8001E750{};
    s_event4FrameState8001E750 =
        PrPsxEventFrameDirect::EventFrameState8001E750{};
    s_evMemCardCtx = PrEvMemCardCtx{};
    s_evMemCardArgBacked = false;
}

void PrEvent::DebugUnlockNextStage() {
    EnsureStageSelectStatusInitialized();
    for (int stage = 1; stage <= 6; ++stage) {
        if (s_stageSelectStatus[stage] == 0) {
            s_stageSelectStatus[stage] = 1;
            break;
        }
    }
    RecomputeBonusStatus();
    SyncStageSelectStatusToPayload();
    SyncDebugStageSelectStatusToDirectBank();
    if (s_dispEventId == 2 && s_dispState == PrDispatcherState::Running) {
        RefreshStageSelectDispatcherContext();
    }
    Log::Printf(
        "DebugStageUnlockNext: [%d,%d,%d,%d,%d,%d] bonus=%d",
        (int)s_stageSelectStatus[1], (int)s_stageSelectStatus[2], (int)s_stageSelectStatus[3],
        (int)s_stageSelectStatus[4], (int)s_stageSelectStatus[5], (int)s_stageSelectStatus[6],
        (int)s_stageSelectStatus[7]);
}

void PrEvent::DebugFirstClearSelectableStages() {
    EnsureStageSelectStatusInitialized();
    int furthestUnlocked = 0;
    for (int stage = 1; stage <= 6; ++stage) {
        if (s_stageSelectStatus[stage] > 0) {
            furthestUnlocked = stage;
            if (s_stageSelectStatus[stage] < 2) {
                s_stageSelectStatus[stage] = 2;
            }
        }
    }
    if (furthestUnlocked >= 1 && furthestUnlocked < 6 && s_stageSelectStatus[furthestUnlocked + 1] == 0) {
        s_stageSelectStatus[furthestUnlocked + 1] = 1;
    }
    RecomputeBonusStatus();
    SyncStageSelectStatusToPayload();
    SyncDebugStageSelectStatusToDirectBank();
    if (s_dispEventId == 2 && s_dispState == PrDispatcherState::Running) {
        RefreshStageSelectDispatcherContext();
    }
    Log::Printf(
        "DebugStageFirstClear: [%d,%d,%d,%d,%d,%d] bonus=%d",
        (int)s_stageSelectStatus[1], (int)s_stageSelectStatus[2], (int)s_stageSelectStatus[3],
        (int)s_stageSelectStatus[4], (int)s_stageSelectStatus[5], (int)s_stageSelectStatus[6],
        (int)s_stageSelectStatus[7]);
}

int PrEvent::GetStageSelectStatus(int stage) {
    EnsureStageSelectStatusInitialized();
    if (stage < 1 || stage > 7) {
        return 0;
    }
    return (int)s_stageSelectStatus[stage];
}

static int GetStageSelectLastSavedSlot92F3C() {
    const uint8_t* payload = PrMemCardBackend::CurrentPayload();
    const size_t payloadSize = PrMemCardBackend::PayloadSize();
    const int savedSlot =
        (int)ReadPayloadU32(payload, payloadSize, kStageLastSavedSlotPayloadOffset);
    return (savedSlot >= 0 && savedSlot < (int)kStageScorePayloadCount) ? savedSlot
                                                                         : -1;
}

void PrEvent::Update(PrGameContext& ctx) {
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        (ctx.currentScene == PrSceneId::Scene0 ||
         ctx.currentScene == PrSceneId::Scene1)) {
        static bool s_loggedDirectSceneSkip = false;
        if (!s_loggedDirectSceneSkip) {
            Log::Printf(
                "PrEvent legacy update skipped in SS0 direct scene=%u frame=%u",
                static_cast<unsigned>(ctx.currentScene),
                ctx.frame);
            s_loggedDirectSceneSkip = true;
        }
        ClearDispatcherResidueForDirectCutover();
        return;
    }

    if (s_dispState != PrDispatcherState::Running) return;

    uint16_t curInput = MapDebugInputToPad(ctx);
    uint16_t changed = 0;
    if (s_dispEventId == 3) {
        // ev=3 should react to the rising edge of a single direction/button.
        // Using the full current mask here makes keyboard/debug fallback combos
        // easy to reject entirely, which freezes the main-menu cursor while
        // face buttons still work.
        const uint16_t pressed = (uint16_t)(curInput & (uint16_t)~s_dispLastInput);
        s_dispLastInput = curInput;
        changed = PickSinglePadInput(pressed);
    } else {
        const uint16_t pressed = (uint16_t)(curInput & (uint16_t)~s_dispLastInput);
        s_dispLastInput = curInput;
        changed = PickSinglePadInput(pressed);
    }

    int result = 0;
    if (s_dispWaitingInput && changed) {
        s_dispNoInputYet = false;
        if (s_dispEntry.def && s_dispEntry.def->handle) {
            result = s_dispEntry.def->handle(changed, s_dispEntry.ctx, s_dispArgPtr);
        }
        if (result != 0) {
            s_dispWaitingInput = false;
            s_dispResult = result;
        }
    }

    if (!s_dispWaitingInput) {
        --s_dispFrameCountdown;
        if (s_dispFrameCountdown <= 0) {
            s_dispState = PrDispatcherState::Done;
            Log::Printf("Dispatcher ev=%u done result=%d", s_dispEventId, s_dispResult);
            return;
        }
    }

    if (s_dispTimeoutInit > 0 && s_dispNoInputYet && s_dispWaitingInput) {
        --s_dispTimeout;
        if (s_dispTimeout <= 0) {
            // ev=3: attract timeout uses -3 to distinguish from Practice(result=3)
            s_dispResult = (s_dispEventId == 3) ? -3 : 3;
            s_dispWaitingInput = false;
            Log::Printf("Dispatcher ev=%u timeout (attract=%d)", s_dispEventId, (s_dispEventId == 3) ? 1 : 0);
        }
    }

    if (s_dispEntry.def && s_dispEntry.def->tick) {
        s_dispEntry.def->tick(&s_dispEntry);
    }
}

bool PrEvent::StartDispatcher(PrEventId eventId, int16_t* transitionStatePtr, PrGameContext& ctx) {
    return StartDispatcherEx(eventId, transitionStatePtr, ctx);
}

// S0_OLD_BOUNDARY: legacy S0 menu-event start point. SS0 should replace
// callers with its own dispatcher authority instead of calling this.
bool PrEvent::StartDispatcherEx(PrEventId eventId, void* argPtr, PrGameContext& ctx) {
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        (ctx.currentScene == PrSceneId::Scene0 ||
         ctx.currentScene == PrSceneId::Scene1)) {
        ClearDispatcherResidueForDirectCutover();
        Log::Printf(
            "PrEvent legacy dispatcher rejected in SS0 direct scene=%u ev=%u frame=%u",
            static_cast<unsigned>(ctx.currentScene),
            eventId,
            ctx.frame);
        return false;
    }

    if (s_dispState == PrDispatcherState::Running) return false;

    const DispEventDef* def = GetEventDef(eventId);
    if (!def || !def->init) {
        return false;
    }

    s_dispState = PrDispatcherState::Running;
    s_dispEventId = eventId;
    s_dispResult = 0;
    s_dispLastInput = MapDebugInputToPad(ctx);
    s_dispWaitingInput = true;
    s_dispNoInputYet = true;
    s_dispCtx = PrEventDispatcherContext{};
    s_dispTimeout = def->timeout;
    s_dispTimeoutInit = def->timeout;

    s_dispEntry.def = def;
    s_dispEntry.ctx = def->default_ctx;
    s_dispEntry.timeout = def->timeout;

    if (eventId == 6 && argPtr == nullptr) {
        argPtr = &s_ev6Ctx;
    }
    s_dispArgPtr = argPtr;
    s_dispGameCtxPtr = &ctx;
    s_dispEntry.arg_ptr = argPtr;

    def->init(&s_dispEntry, eventId, argPtr);

    if (eventId == 3) {
        Log::Printf("PrEvent: ev=3 enter transition attempt (active=%d)", (int)PrTransition::IsActive());
        static const TransitionConfig kEv3EnterTrans = {0, 0, 24, 0};
        PrTransition::Start(-1, kEv3EnterTrans);
    }

    Log::Printf("Dispatcher started ev=%u frame=%u", eventId, ctx.frame);
    return true;
}

bool PrEvent::IsDispatcherRunning() {
    return s_dispState == PrDispatcherState::Running;
}

// S0_OLD_HANDOFF: legacy result collection. Memcard events perform card HAL
// save/load/replay resolution here, so this is not a pure adapter.
int PrEvent::ConsumeDispatcherResult() {
    if (s_dispState != PrDispatcherState::Done) return -1;
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
        if (s_dispGameCtxPtr == nullptr) {
            Log::Printf(
                "PrEvent legacy dispatcher result rejected in SS0 direct missing ctx ev=%u",
                s_dispEventId);
            ClearDispatcherResidueForDirectCutover();
            return -1;
        }
        if (s_dispGameCtxPtr->currentScene == PrSceneId::Scene0 ||
            s_dispGameCtxPtr->currentScene == PrSceneId::Scene1) {
            Log::Printf(
                "PrEvent legacy dispatcher result rejected in SS0 direct scene=%u ev=%u frame=%u",
                static_cast<unsigned>(s_dispGameCtxPtr->currentScene),
                s_dispEventId,
                s_dispGameCtxPtr->frame);
            ClearDispatcherResidueForDirectCutover();
            return -1;
        }
    }
    const PrEventId completedEventId = s_dispEventId;
    int r = s_dispResult;
    s_evMemCardCtx.replayResolvedSlot = -1;
    s_evMemCardCtx.replayResolvedScene = -1;
    if (r == 1 && (s_dispEventId == 7 || s_dispEventId == 8 || s_dispEventId == 9)) {
        const int selected = (int)s_evMemCardCtx.selected;
        const bool selectedValid =
            selected >= 0 && selected < 15 &&
            s_evMemCardCtx.entryBlockIndex[selected] >= 0;
        if (!selectedValid) {
            Log::Printf("PrEvent memcard ev=%u invalid selection=%d",
                        s_dispEventId,
                        selected);
            r = 2;
        } else {
            const int blockIndex = (int)s_evMemCardCtx.entryBlockIndex[selected];
            bool ok = false;
            if (s_dispEventId == 7) {
                ok = PrMemCardBackend::SaveEntry(blockIndex);
            } else {
                ok = PrMemCardBackend::LoadEntry(blockIndex);
                if (ok) {
                    ReloadStageSelectAuthorityFromPayload164B4();
                    if (s_dispEventId == 9) {
                        const int replaySlot = GetStageSelectLastSavedSlot92F3C();
                        const int replayScene =
                            ResolveStageSelectSceneIdFromSavedSlot161A8(replaySlot);
                        s_evMemCardCtx.replayResolvedSlot = replaySlot;
                        s_evMemCardCtx.replayResolvedScene = replayScene;
                        ok = replayScene >= 0;
                    }
                }
            }
            Log::Printf(
                "PrEvent memcard ev=%u confirm selected=%d block=%d ok=%d replaySlot=%d replayScene=%d",
                        s_dispEventId,
                        selected,
                        blockIndex,
                        ok ? 1 : 0,
                        s_evMemCardCtx.replayResolvedSlot,
                        s_evMemCardCtx.replayResolvedScene);
            if (!ok) {
                r = 2;
            }
        }
    }
    s_dispState = PrDispatcherState::Idle;
    s_dispEventId = 0;
    s_dispResult = 0;
    if (completedEventId == 2) {
        s_event2FrameState8001E750 =
            PrPsxEventFrameDirect::EventFrameState8001E750{};
    } else if (completedEventId == 4) {
        s_event4FrameState8001E750 =
            PrPsxEventFrameDirect::EventFrameState8001E750{};
    }
    return r;
}

int PrEvent::GetMemCardReplayResolvedSlot() {
    return s_evMemCardCtx.replayResolvedSlot;
}

int PrEvent::GetMemCardReplayResolvedScene() {
    return s_evMemCardCtx.replayResolvedScene;
}

PrEventDispatcherContext& PrEvent::GetDispatcherContext() {
    return s_dispCtx;
}

const PrPsxEventFrameDirect::EventFrameState8001E750*
PrEvent::GetActiveEventFrameState8001E750() {
    if (s_dispState != PrDispatcherState::Running) {
        return nullptr;
    }
    if (s_dispEventId == 2) {
        return &s_event2FrameState8001E750;
    }
    if (s_dispEventId == 4) {
        return &s_event4FrameState8001E750;
    }
    return nullptr;
}

int* PrEvent::GetEv6DoneFlagPtr() {
    return (int*)&s_ev6Ctx.done_flag;
}

int* PrEvent::GetDispTimeoutPtr() {
    return &s_dispTimeout;
}

int* PrEvent::GetDispFrameCountdownPtr() {
    return &s_dispFrameCountdown;
}

uint16_t* PrEvent::GetDispLastInputPtr() {
    return &s_dispLastInput;
}

int* PrEvent::GetEv17BlinkPtr() {
    return &s_ev17Blink;
}

int* PrEvent::GetEv2BlinkPtr() {
    return &s_ev2Blink;
}

int* PrEvent::GetEv3BlinkPtr() {
    return &s_ev3Blink;
}

int16_t* PrEvent::GetDispMenuIndexPtr() {
    return &s_dispCtx.menuIndex;
}

int* PrEvent::GetDispEventIdPtr() {
    return reinterpret_cast<int*>(&s_dispEventId);
}

void PrEvent::SetRecordsModePtr(int16_t* ptr) {
    s_ev3RecordsModePtr = ptr;
}

PrEv16Ctx* PrEvent::GetEv16CtxPtr() {
    return (PrEv16Ctx*)&s_ev16Ctx;
}

PrEvMemCardCtx* PrEvent::GetEvMemCardCtxPtr() {
    return &s_evMemCardCtx;
}

bool PrEvent::IsEvMemCardArgBacked() {
    return s_evMemCardArgBacked;
}
