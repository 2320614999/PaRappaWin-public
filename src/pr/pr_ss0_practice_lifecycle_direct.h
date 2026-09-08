#pragma once

#include <cstdint>

namespace PrSS0PracticeLifecycleDirect {

static constexpr uint32_t kFn8002776C = 0x8002776Cu;
static constexpr uint32_t kFn80015618 = 0x80015618u;
static constexpr uint32_t kFn80025A34 = 0x80025A34u;
static constexpr uint32_t kFn8001AC18 = 0x8001AC18u;
static constexpr uint32_t kFn8001E750 = 0x8001E750u;
static constexpr uint32_t kFn80023618 = 0x80023618u;
static constexpr uint32_t kFn80035510 = 0x80035510u;
static constexpr uint32_t kFn80035560 = 0x80035560u;
static constexpr uint32_t kFn8001EA00 = 0x8001EA00u;
static constexpr uint32_t kFn80024E98 = 0x80024E98u;
static constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
static constexpr uint32_t kFn80026ECC = 0x80026ECCu;
static constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
static constexpr uint32_t kFn80025C8C = 0x80025C8Cu;
static constexpr uint32_t kFn800276EC = 0x800276ECu;
static constexpr uint32_t kFn80015590 = 0x80015590u;

static constexpr uint32_t kScene0WorkAddress = 0x801C3640u;
static constexpr uint32_t kYCompoDescriptor800546EC = 0x800546ECu;
static constexpr uint32_t kCuePracticeLoop8006EC90 = 0x8006EC90u;
static constexpr uint32_t kCuePracticeBeat8006EC98 = 0x8006EC98u;
static constexpr uint32_t kCuePracticeExitKind8_8006ECA0 = 0x8006ECA0u;
static constexpr uint32_t kCuePracticeExitKind7_8006ECA8 = 0x8006ECA8u;
static constexpr uint32_t kCuePracticeRoundPromptTable80055494 = 0x80055494u;
static constexpr uint32_t kCuePracticeVoiceTable800554AC = 0x800554ACu;
static constexpr uint32_t kPracticeRoundTargetTable80055484 = 0x80055484u;
static constexpr uint32_t kPracticeRoundKindTable8006ECB0 = 0x8006ECB0u;
static constexpr uint32_t kPracticeRec44RecordBase800554D8 = 0x800554D8u;
static constexpr uint32_t kPracticeRec44StreamBase800554DC = 0x800554DCu;
static constexpr uint32_t kPracticeRec44Stride = 0x2Cu;
static constexpr uint32_t kPracticeRec44StreamByteCount = 40u;
static constexpr uint32_t kPracticeRoundCount = 4u;
static constexpr uint32_t kPracticeMainWindowFrames = 150u;
static constexpr uint32_t kPracticeBeatPeriodFrames = 75u;
static constexpr uint32_t kPracticeExitConfirmFrames = 15u;

enum class PracticeActionKind : uint8_t {
    None = 0,
    CallLoadYCompo80015618,
    ClearResourceTable80025A34,
    LoadInt8001AC18,
    TimUpload,
    VabLoadPractice,
    SetVSyncPadStart,
    GatePracticeLoopCtxSource8002776C,
    InitCtx,
    GatePadStopCueSource800276EC,
    PadStopCom800276EC,
    PlayCue80026EF8,
    Flush80026ECC,
    ResetEventText80024E98,
    PollInput80035510,
    DebounceInputGp844,
    GatePracticeRoundRec44Source8002776C,
    JudgeTriangle,
    FastForwardCross,
    AdvanceRec44,
    DrawEvent16_8001E750,
    DrawPractice80023618,
    WaitFrame80035560_2,
    EndFrame8001EA00_0,
    ExitConfirmLoop,
    RestorePrevScene80015590,
    Gap,
};

enum class PracticeCueKind : uint8_t {
    Unknown = 0,
    LoopStart,
    Beat,
    RoundPrompt0,
    RoundPrompt1,
    RoundPrompt2,
    RoundPrompt3,
    VoiceOnBeat,
    VoiceTooQuick,
    VoiceTooSlow,
    VoiceIntro0,
    VoiceIntro1,
    VoiceIntro2,
    VoiceIntro3,
    VoiceExit7,
    VoiceExit8,
};

enum class PracticeCtxFieldKind : uint8_t {
    Flags = 0,
    OverlayKind,
    ExitSelected,
    Blink,
    SeqMode,
    SeqCurA,
    SeqEnableA,
    SeqStreamA,
    SeqCurB,
    SeqEnableB,
    SeqStreamB,
};

struct PracticeCueSpec {
    PracticeCueKind kind = PracticeCueKind::Unknown;
    uint32_t address = 0;
    uint8_t bytes[4]{};
    int32_t frames = 0;
    int32_t padStopKind = -1;
    int32_t round = -1;
    const char* name = nullptr;
};

struct PracticeCtxFieldSpec {
    PracticeCtxFieldKind kind = PracticeCtxFieldKind::Flags;
    uint32_t offset = 0;
    uint32_t byteCount = 0;
    const char* name = nullptr;
    const char* producer = nullptr;
    const char* consumer = nullptr;
};

struct PracticeRec44Spec {
    int32_t round = 0;
    uint32_t recordAddress = 0;
    uint32_t streamAddress = 0;
    int16_t head0 = 0;
    uint16_t head1 = 0;
    int32_t pulseSlot = -1;
    int32_t sentinelSlot = -1;
};

struct PracticeRec44StreamValue {
    bool sourceKnown = false;
    uint32_t sourceAddress = 0;
    int8_t value = 0;
};

struct PracticeAction {
    PracticeActionKind kind = PracticeActionKind::None;
    uint32_t psxFunction = 0;
    uint32_t address = 0;
    int32_t round = -1;
    int32_t frame = -1;
    int32_t args[4]{};
    PracticeCueKind cue = PracticeCueKind::Unknown;
    bool conditional = false;
};

struct PracticePlan {
    const char* name = nullptr;
    bool runtimeCutoverAllowed = false;
    bool blockedByGap = true;
    PracticeAction actions[160]{};
    uint32_t count = 0;
    bool truncated = false;
};

enum class PracticeExitConfirmResult8002776C : uint8_t {
    Pending = 0,
    RetryPractice,
    RestorePreviousScene,
};

struct PracticeExitConfirmRuntime8002776C {
    bool initialized = false;
    bool choiceArmed = false;
    bool retrySelected = false;
    int32_t framesRemaining = 0;
    int32_t exitSelection48 = 0;
    int16_t exitBlink4C = 0;
    uint32_t blinkCounter = 0;
    PracticeExitConfirmResult8002776C result =
        PracticeExitConfirmResult8002776C::Pending;
};

struct PracticeExitConfirmInput8002776C {
    bool edgeKnown = false;
    uint32_t edge = 0;
};

bool RuntimeCutoverAllowed();

uint32_t KnownPracticeCueSpecCount();
const PracticeCueSpec& KnownPracticeCueSpecAt(uint32_t index);
const PracticeCueSpec* FindPracticeCueSpec(PracticeCueKind cue);

uint32_t KnownPracticeCtxFieldCount();
const PracticeCtxFieldSpec& KnownPracticeCtxFieldAt(uint32_t index);

uint32_t KnownPracticeRec44SpecCount();
const PracticeRec44Spec& KnownPracticeRec44SpecAt(uint32_t index);
PracticeRec44StreamValue ReadPracticeRec44StreamValue(
    int32_t round,
    int32_t slot);

PracticePlan BuildPracticeResourceLoad80015618Plan();
PracticePlan BuildPracticeRound8002776CPlan(int32_t round);
PracticePlan BuildPracticePadStop800276ECPlan(PracticeCueKind cue);
PracticePlan BuildPracticeDrawFramePlan(uint32_t menuCtx = kScene0WorkAddress);
PracticePlan BuildPracticeLoop8002776CPlan(
    uint32_t menuCtx = kScene0WorkAddress,
    int32_t prevScene = 0);
bool BeginPracticeExitConfirm8002776C(
    PracticeExitConfirmRuntime8002776C& runtime,
    int16_t initialBlink4C);
bool TickPracticeExitConfirm8002776C(
    PracticeExitConfirmRuntime8002776C& runtime,
    const PracticeExitConfirmInput8002776C& input);

const char* PracticeActionKindName(PracticeActionKind kind);
const char* PracticeCueKindName(PracticeCueKind kind);
const char* PracticeCtxFieldKindName(PracticeCtxFieldKind kind);

} // namespace PrSS0PracticeLifecycleDirect
