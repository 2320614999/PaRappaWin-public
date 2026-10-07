#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

namespace PrSS0TransitionDirect {

static constexpr uint32_t kFn80020110 = 0x80020110u;
static constexpr uint32_t kFn800201AC = 0x800201ACu;
static constexpr uint32_t kFn8001EA74 = 0x8001EA74u;
static constexpr uint32_t kFn8001FFD4 = 0x8001FFD4u;
static constexpr uint32_t kFn8001EBF4 = 0x8001EBF4u;
static constexpr uint32_t kFn8001F518 = 0x8001F518u;
static constexpr uint32_t kFn80020008 = 0x80020008u;
static constexpr uint32_t kFn80020090 = 0x80020090u;
static constexpr uint32_t kFn8001FCBC = 0x8001FCBCu;
static constexpr uint32_t kFn8001FC40 = 0x8001FC40u;
static constexpr uint32_t kFn8001F524 = 0x8001F524u;
static constexpr uint32_t kFn8001FDC0 = 0x8001FDC0u;
static constexpr uint32_t kFn8001EF40 = 0x8001EF40u;
static constexpr uint32_t kFn8001C4EC = 0x8001C4ECu;
static constexpr uint32_t kFn8001C550 = 0x8001C550u;
static constexpr uint32_t kFn80015408 = 0x80015408u;
static constexpr uint32_t kFn8001537C = 0x8001537Cu;
// 80015590 always loads the common/menu segment with mode 3.  This is not
// the default for 80015660: its caller supplies a scene-specific mode.
static constexpr int16_t kLoadingPatternMode80015408 = 3;
// COMOD1 801C7284 -> 80015660(a2, word_801CCBBC[a2], 1).
static constexpr std::array<int16_t, 8> kSceneEntryLoadingModes801CCBBC = {
    1, 1, 2, 3, 1, 2, 3, 4
};
static constexpr uint32_t kFn8001D74C = 0x8001D74Cu;
static constexpr uint32_t kFn80022CBC = 0x80022CBCu;
static constexpr uint32_t kFn80021E60 = 0x80021E60u;
static constexpr uint32_t kFn80020308 = 0x80020308u;
static constexpr uint32_t kFn80020248 = 0x80020248u;
static constexpr uint32_t kFn8001F230 = 0x8001F230u;
static constexpr uint32_t kFn8001FEB4 = 0x8001FEB4u;
static constexpr uint32_t kFn8001C864 = 0x8001C864u;
static constexpr uint32_t kFn8001CE30 = 0x8001CE30u;
static constexpr uint32_t kFn8004019C = 0x8004019Cu;
static constexpr uint32_t kFn80040370 = 0x80040370u;
static constexpr uint32_t kFn80040420 = 0x80040420u;
static constexpr uint32_t kFn80040CA4 = 0x80040CA4u;
static constexpr uint32_t kFn80040CC8 = 0x80040CC8u;
static constexpr uint32_t kFn80040F90 = 0x80040F90u;
static constexpr uint32_t kFn80027194 = 0x80027194u;
static constexpr uint32_t kFn800271E4 = 0x800271E4u;
static constexpr uint32_t kFn80035560 = 0x80035560u;
static constexpr uint32_t kFn801C4DC4 = 0x801C4DC4u;
static constexpr uint32_t kFn801C44E0 = 0x801C44E0u;
static constexpr uint32_t kFn801C455C = 0x801C455Cu;
static constexpr uint32_t kFn801C4894 = 0x801C4894u;
static constexpr uint32_t kFn8001B120 = 0x8001B120u;
static constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
static constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
static constexpr uint32_t kFn80026ECC = 0x80026ECCu;
static constexpr uint32_t kFn8001EF14 = 0x8001EF14u;
static constexpr uint32_t kSub80027194CounterAddress = 0x8006EC20u;
static constexpr uint32_t kSub80027194CuePayloadAddress = 0x8006EC18u;
static constexpr uint32_t kSub80027194CueProgram = 0u;
static constexpr uint32_t kSub80027194CueNote = 12u;
static constexpr uint32_t kSub80027194CueKey = 0x24u;
static constexpr uint32_t kSub80027194CueVolume = 0x50u;
static constexpr uint32_t kWord800916D0Address = 0x800916D0u;
static constexpr uint32_t kWord800916D2Address = 0x800916D2u;
static constexpr uint32_t kWord800916EEAddress = 0x800916EEu;
static constexpr uint32_t kCueGlobal94410 = 0x80094410u;
static constexpr uint32_t kCueGlobal9441C = 0x8009441Cu;
static constexpr uint32_t kScene0WorkAddress = 0x801C3640u;
static constexpr uint32_t kSceneEntryMovie0SegmentOffset = 0x6Cu;
static constexpr uint32_t kSceneEntryMovie0TSegmentOffset = 0x9Cu;
static constexpr uint32_t kScene0TitleLoopCallsite801C4E84 = 0x801C4E84u;
static constexpr uint32_t kScene0TitleLoopEntryFrame6810 = 6810u;
static constexpr uint32_t kScene0TitleIntroTransitionStartFrame6815 = 6815u;
static constexpr uint32_t kScene0TitleIntroTransitionTailFrame6863 = 6863u;
static constexpr uint32_t kScene0TitleIntroTransitionEndFrame6871 = 6871u;
static constexpr uint32_t kScene0TitleExitAudioResetCallsite801C4E8C = 0x801C4E8Cu;
static constexpr uint32_t kScene0TitleExitFadeCallsite801C4EA0 = 0x801C4EA0u;
static constexpr uint32_t kScene0TitleExitFadeTailCallsite80020188 = 0x80020188u;
static constexpr uint32_t kScene0TitleExitSceneTailCallsite801C4EA8 = 0x801C4EA8u;
static constexpr uint32_t kScene0TitleExitStartFrame7854 = 7854u;
static constexpr uint32_t kScene0TitleExitFadeTailFrame7902 = 7902u;
static constexpr uint32_t kScene0TitleExitSceneTailFrame7910 = 7910u;
static constexpr uint32_t kScene0MainMenuEntryTransitionFirstRecordedFrame8126 =
    8126u;
static constexpr uint32_t kScene0MainMenuEntryTransitionRelativeTickOrigin = 1u;
static constexpr uint32_t kScene0MainMenuEntryTransitionCallsite80026C90 =
    0x80026C90u;
static constexpr uint32_t kTitleSelectorResultMenu801C4DC4 = 1u;
static constexpr uint32_t kTitleSelectorResultStart801C4DC4 = 2u;
static constexpr uint32_t kTitleSelectorResultRandom801C4DC4 = 3u;
static constexpr uint32_t kScene0TitleResultCompareOneCallsite801C4EB0 = 0x801C4EB0u;
static constexpr uint32_t kScene0TitleResultMenuWriteCallsite801C4EBC = 0x801C4EBCu;
static constexpr uint32_t kScene0TitleResultMenuReturnCallsite801C4EC8 = 0x801C4EC8u;
static constexpr uint32_t kScene0TitleResultCompareRandomCallsite801C4ECC = 0x801C4ECCu;
static constexpr uint32_t kScene0TitleResultRandomWriteCallsite801C4ED4 = 0x801C4ED4u;
static constexpr uint32_t kScene0TitleResultRandCallsite801C4EE0 = 0x801C4EE0u;
static constexpr uint32_t kScene0TitleResultRandomModuloCallsite801C4EE8 = 0x801C4EE8u;
static constexpr uint32_t kScene0TitleResultLastRandomReadCallsite801C4F08 = 0x801C4F08u;
static constexpr uint32_t kScene0TitleResultAvoidLastBranchCallsite801C4F14 = 0x801C4F14u;
static constexpr uint32_t kScene0TitleResultDefaultCueLoadCallsite801C4F24 = 0x801C4F24u;
static constexpr uint32_t kScene0TitleResultDefaultCueCallsite801C4F2C = 0x801C4F2Cu;
static constexpr uint32_t kScene0TitleResultDefaultFlushCallsite801C4F34 = 0x801C4F34u;
static constexpr uint32_t kScene0TitleResultDefaultReturnCallsite801C4F3C = 0x801C4F3Cu;
static constexpr uint32_t kScene0TitleResultDefaultWriteCallsite801C4F40 = 0x801C4F40u;
static constexpr uint32_t kScene0TitleResultReturnJoinCallsite801C4F48 = 0x801C4F48u;

enum class TransitionPlanKind : uint8_t {
    Unknown = 0,
    Mode8001EA74,
    Slow80020110,
    Fast800201AC,
    Scene0Outer801C4DC4,
    Scene0TitleResult801C4DC4,
};

enum class FastTransitionRuntimePhase800201AC : uint8_t {
    Idle = 0,
    Loop8001EA74,
    Tail80020090,
    Complete,
};

static constexpr std::size_t kFastTransitionTileMaskCells8001EEAC = 192u;

struct FastTransitionRuntime800201AC {
    bool active = false;
    uint32_t ctxAddress = 0;
    int32_t mode = 0;
    int32_t preFfd4Arg = 0;
    int32_t postFfd4Arg = 0;
    bool word800916DC = false;
    FastTransitionRuntimePhase800201AC phase =
        FastTransitionRuntimePhase800201AC::Idle;
    uint32_t loopIterationsRequired = 0;
    uint32_t loopIterationsCompleted = 0;
    uint32_t tailIterationsCompleted = 0;
    uint32_t waitVblanksCompleted = 0;
    bool initialCue800271E4Dispatched = false;
    // 8001FFD4 resets gp+0xC4 and writes all 192 dword cells through
    // 8001EEAC both before the body and before the four-frame tail.
    bool tileMaskKnown8001EEAC = false;
    bool postFfd4Applied = false;
    uint32_t gp196 = 0;
    uint32_t tileMaskMutationSerial8001EEAC = 0;
    std::array<uint32_t, kFastTransitionTileMaskCells8001EEAC>
        tileMask8001EEAC{};
};

enum class FastTransitionFrameKind800201AC : uint8_t {
    Unknown = 0,
    NoSubtitleFrame8001CE30,
    SubtitleFrame8001C864,
    OutroNoSubboxFrame8001F230,
    FinalNoVideoFrame8001FEB4,
};

struct FastTransitionVisualFrame800201AC {
    bool known = false;
    bool tail = false;
    bool clearColorKnown = false;
    uint8_t clearR = 0;
    uint8_t clearG = 0;
    uint8_t clearB = 0;
    int32_t mode = 0;
    bool word800916DC = false;
    uint32_t iteration = 0;
    uint32_t sourceFunction = 0;
    FastTransitionFrameKind800201AC kind =
        FastTransitionFrameKind800201AC::Unknown;
};

struct FastTransitionSpriteTemplate800201AC {
    bool known = false;
    uint32_t sourceAddress = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

struct FastTransitionSpriteCommand800201AC {
    bool active = false;
    bool rawTextureKnown = false;
    bool rawTexture = false;
    bool semiTransparentKnown = false;
    bool semiTransparent = false;
    bool abrKnown = false;
    uint8_t abr = 0;
    bool rgbKnown = false;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t priority = 0;
    uint16_t order = 0;
    FastTransitionSpriteTemplate800201AC spriteTemplate{};
};

static constexpr std::size_t kFastTransitionMaxSpriteCommands800201AC = 192;

struct FastTransitionFramePlan800201AC {
    bool known = false;
    bool truncated = false;
    FastTransitionFrameKind800201AC kind =
        FastTransitionFrameKind800201AC::Unknown;
    std::size_t commandCount = 0;
    FastTransitionSpriteCommand800201AC
        commands[kFastTransitionMaxSpriteCommands800201AC]{};
};

enum class FastTransitionPresentActionKind8001EBF4 : uint8_t {
    Unknown = 0,
    GetDrawBuffer8004019C,
    SetPacketAllocator80040F90,
    ClearMainPageWork80040CC8,
    BuildVisualFrame8001EA74,
    FlipGraph80040370,
    ClearColor80040420,
    SubmitMainPageWork80040CA4,
};

struct FastTransitionGraphInput8001EA74 {
    bool known = false;
    // Resident 80015788 runs after 80015D18's 8001E34C, without COMOD0
    // 801C609C rebinding the two packet lanes to the title arena.
    bool residentMainPacketLanes8001E34C = false;
    uint16_t drawSlot8004019C = 0;
    uint32_t packetAllocator8006ED50 = 0;
    uint32_t mainPageWorkAddress80087288 = 0;
    uint32_t mainPageOtHeadAddress80088288 = 0;
    uint32_t mainPageWorkHeadAddress80040CC8 = 0;
    uint32_t mainPageWorkOrder = 0;
};

struct FastTransitionPresentAction8001EBF4 {
    FastTransitionPresentActionKind8001EBF4 kind =
        FastTransitionPresentActionKind8001EBF4::Unknown;
    uint32_t psxFunction = 0;
    uint32_t arg0 = 0;
    uint32_t arg1 = 0;
    uint32_t arg2 = 0;
};

static constexpr std::size_t kFastTransitionMaxPresentActions8001EBF4 = 7;

struct FastTransitionPresentPlan8001EBF4 {
    bool known = false;
    bool truncated = false;
    uint16_t drawSlotBefore = 0;
    uint16_t drawSlotAfter = 0;
    uint32_t packetAllocator8006ED50 = 0;
    uint32_t mainPageWorkAddress80087288 = 0;
    uint32_t mainPageOtHeadAddress80088288 = 0;
    bool clearColorRequested80040420 = false;
    bool mainPageWorkSubmitRequested80040CA4 = false;
    std::size_t actionCount = 0;
    FastTransitionPresentAction8001EBF4
        actions[kFastTransitionMaxPresentActions8001EBF4]{};
};

struct FastTransitionTickResult800201AC {
    bool accepted = false;
    bool loopIterationCompleted = false;
    bool tailIterationCompleted = false;
    bool complete = false;
    bool sub800271E4CallRequired = false;
    uint8_t sub800271E4CueIndex = 0;
    uint32_t sub800271E4CueAddress = 0;
    uint32_t sub800271E4SourceFunction = 0;
    bool tileMaskMutationApplied8001EEAC = false;
    uint32_t tileMaskMutationSerial8001EEAC = 0;
    uint32_t gp196 = 0;
    FastTransitionRuntimePhase800201AC phaseBefore =
        FastTransitionRuntimePhase800201AC::Idle;
    FastTransitionRuntimePhase800201AC phaseAfter =
        FastTransitionRuntimePhase800201AC::Idle;
    uint32_t loopIterationsCompleted = 0;
    uint32_t tailIterationsCompleted = 0;
    uint32_t waitVblanksCompleted = 0;
    FastTransitionVisualFrame800201AC visualFrame{};
};

enum class SlowTransitionRuntimePhase80020110 : uint8_t {
    Idle = 0,
    Loop8001EA74,
    Tail80020008,
    Complete,
};

static constexpr std::size_t kSlowTransitionGridRows8001FDC0 = 12;
static constexpr std::size_t kSlowTransitionGridColumns8001FDC0 = 16;
static constexpr std::size_t kSlowTransitionGridCells8001FDC0 =
    kSlowTransitionGridRows8001FDC0 *
    kSlowTransitionGridColumns8001FDC0;

struct SlowTransitionGridCoordinate8004EB80 {
    bool known = false;
    uint8_t row = 0;
    uint8_t column = 0;
};

struct SlowTransitionRuntime80020110 {
    bool active = false;
    uint32_t ctxAddress = 0;
    int32_t mode = 0;
    int32_t preFfd4Arg = 0;
    int32_t postFfd4Arg = 0;
    SlowTransitionRuntimePhase80020110 phase =
        SlowTransitionRuntimePhase80020110::Idle;
    uint32_t loopIterationsRequired = 0;
    uint32_t loopIterationsCompleted = 0;
    uint32_t tailIterationsCompleted = 0;
    uint32_t waitVblanksCompleted = 0;
    uint32_t hostTicksCompleted = 0;
    uint32_t sceneFrame = 0;

    // Software owner for the PSX 8001FFD4 -> 8001EEAC tile-mask state.  The
    // old host curtain kept this state implicit in its renderer; SS0 keeps it
    // alongside the translated transition so the 8001EA74 body operates on
    // the same 12x16 mask that 8001F524/8001FDC0 consume.
    bool tileMaskKnown = false;
    bool postFfd4Applied = false;
    uint32_t gp196 = 0;
    uint32_t tileMaskMutationSerial = 0;
    std::array<uint8_t, kSlowTransitionGridCells8001FDC0> tileMask{};
};

struct Sub80027194CadenceState {
    int32_t counter = 0;
};

struct Sub80027194CadenceStep {
    bool known = false;
    bool cueRequired = false;
    bool flushRequired = false;
    int32_t previousCounter = 0;
    int32_t nextCounter = 0;
};

struct SlowTransitionVisualFrame80020110 {
    bool known = false;
    bool tail = false;
    uint32_t ctxAddress = 0;
    int32_t mode = 0;
    int32_t preFfd4Arg = 0;
    int32_t postFfd4Arg = 0;
    uint32_t iteration = 0;
    uint32_t activeCount = 0;
    uint32_t sourceFunction = 0;
    uint8_t grid[kSlowTransitionGridCells8001FDC0]{};
};

struct SlowTransitionFramePlan8001FDC0 {
    bool known = false;
    bool truncated = false;
    bool tail = false;
    uint32_t iteration = 0;
    uint32_t activeCount = 0;
    std::size_t commandCount = 0;
    FastTransitionSpriteCommand800201AC
        commands[kSlowTransitionGridCells8001FDC0]{};
};

struct SlowTransitionTickResult80020110 {
    bool accepted = false;
    bool sub80027194CallRequired = false;
    bool presentRequired = false;
    bool loopIterationCompleted = false;
    bool tailIterationCompleted = false;
    bool complete = false;
    SlowTransitionRuntimePhase80020110 phaseBefore =
        SlowTransitionRuntimePhase80020110::Idle;
    SlowTransitionRuntimePhase80020110 phaseAfter =
        SlowTransitionRuntimePhase80020110::Idle;
    uint32_t loopIterationsCompleted = 0;
    uint32_t tailIterationsCompleted = 0;
    uint32_t waitVblanksCompleted = 0;
    uint32_t hostTicksCompleted = 0;
    uint32_t sceneFrame = 0;
    bool tileMaskMutationApplied = false;
    uint32_t tileMaskMutationSerial = 0;
    SlowTransitionVisualFrame80020110 visualFrame{};
};

// Direct software owner for SCUS 8001EF40's full-screen Loading pattern.
// The four pattern tables stream one bit into each row of the shared 12x16
// live grid every third callback.  8001EF40 always draws all 192 role tiles;
// cells set in liveGrid receive an opaque 8001C4EC colour tile afterwards.
static constexpr std::size_t kLoadingPatternGridRows8001EF40 = 12u;
static constexpr std::size_t kLoadingPatternGridColumns8001EF40 = 16u;
static constexpr std::size_t kLoadingPatternGridCells8001EF40 =
    kLoadingPatternGridRows8001EF40 *
    kLoadingPatternGridColumns8001EF40;

struct LoadingPatternRuntime8001EF40 {
    bool countersKnown = false;
    bool active = false;
    uint8_t style = 0u;
    uint32_t bitCursorGp49 = 0u;
    uint32_t wordCursorGp50 = 0u;
    uint32_t frameCounterGp51 = 0u;
    uint32_t mutationSerial = 0u;
    std::array<uint8_t, kLoadingPatternGridCells8001EF40> liveGrid{};
};

struct LoadingPatternFrame8001EF40 {
    bool known = false;
    bool mutationApplied = false;
    uint8_t style = 0u;
    uint32_t sourceFunction = 0u;
    uint32_t drawHighlightFunction = 0u;
    uint32_t drawTileFunction = 0u;
    uint32_t boxFillAttr8001B6C4 = 0u;
    uint32_t boxFillGpuColorCode8003EE84 = 0u;
    uint32_t bitCursorGp49 = 0u;
    uint32_t wordCursorGp50 = 0u;
    uint32_t frameCounterGp51 = 0u;
    uint32_t mutationSerial = 0u;
    uint32_t highlightCount = 0u;
    uint64_t liveGridFnv1a = 0u;
    std::array<uint8_t, kLoadingPatternGridCells8001EF40> liveGrid{};
};

enum class TransitionActionKind : uint8_t {
    None = 0,
    GateModeSource8001EA74,
    Call80020110,
    Call800201AC,
    Call8001EA74,
    GateTransitionCallSource80020110,
    GateTransitionCallSource800201AC,
    Call8001FFD4,
    Call8001EBF4,
    Call8001F518,
    Call80020008Tail,
    Call80020090Tail,
    Call8001FCBC,
    Call8001FC40,
    Call8001FDC0,
    Call8001D74C,
    Call80022CBC,
    Call80021E60,
    Call80020308,
    Call80020248,
    Call8001F230,
    Call8001FEB4,
    Call8001C864,
    Call8001CE30,
    Call80027194,
    Call80035560,
    Call801C44E0,
    Call801C455C,
    Call801C4894,
    Call801C4894TitleLoop801C4E84,
    Call8001B120,
    Call80026FA4,
    Call80026FA4TitleExit801C4E8C,
    GateScene0TitleExitFadeCallsite801C4EA0,
    Call80020110TitleExitFade,
    Call80020008TitleExitFadeTail,
    Call8001EF14,
    Call8001EF14TitleExitSceneTail801C4EA8,
    GateScene0OuterSource801C4DC4,
    GateScene0TitleResultSource801C4DC4,
    CompareTitleResultMenu801C4EB0,
    WriteWord800916D0TitleMenuZero801C4EBC,
    ReturnTitleMenuMinusOne801C4EC8,
    CompareTitleResultRandom801C4ECC,
    WriteWord800916D0TitleRandomOne801C4ED4,
    CallRandTitleRandomScene801C4EE0,
    GateTitleRandomModuloSixPlusOne801C4EE8,
    GateTitleRandomAvoidLastScene800916EE,
    ReturnTitleRandomScene801C4F48,
    GateTitleDefaultBranch801C4F24,
    Call80026EF8TitleDefaultCue801C4F2C,
    Call80026ECCTitleDefaultFlush801C4F34,
    ReturnTitleDefaultScenePlusOne801C4F3C,
    WriteWord800916D0TitleDefaultZero801C4F40,
    SetGpC4,
    SetGp792,
    Gap,
};

struct TransitionAction {
    TransitionActionKind kind = TransitionActionKind::None;
    uint32_t psxFunction = 0;
    uint32_t arg0 = 0;
    uint32_t arg1 = 0;
    uint32_t arg2 = 0;
    uint32_t arg3 = 0;
    int32_t repeatCount = 1;
    bool repeatKnown = true;
};

struct TransitionPlan {
    TransitionPlanKind kind = TransitionPlanKind::Unknown;
    int32_t mode = 0;
    bool word800916DC = false;
    bool runtimeCutoverAllowed = false;
    bool hasOpenP0Gap = true;
    TransitionAction actions[48]{};
    uint32_t count = 0;
    bool truncated = false;
};

enum class Scene0OuterEntryStatus801C4DC4 : uint8_t {
    SourceUnknown = 0,
    InvalidContext,
    AcceptedSkipInitialSlow,
    AcceptedInitialSlow,
};

struct Scene0OuterEntryInput801C4DC4 {
    bool contextKnown = false;
    uint32_t contextAddress = 0u;
    bool word800916D2Known = false;
    uint16_t word800916D2 = 0u;
};

struct Scene0OuterEntryTransaction801C4DC4 {
    Scene0OuterEntryStatus801C4DC4 status =
        Scene0OuterEntryStatus801C4DC4::SourceUnknown;
    bool accepted = false;
    bool publishWord800916D2 = false;
    uint16_t nextWord800916D2 = 0u;
    bool initialSlowTransitionRequired80020110 = false;
    uint32_t slowCtxAddress80020110 = 0u;
    int32_t slowMode80020110 = 0;
    int32_t slowPreFfd4Arg80020110 = 0;
    int32_t slowPostFfd4Arg80020110 = 0;
    uint32_t fastCtxAddress800201AC = 0u;
    int32_t fastMode800201AC = 0;
    int32_t fastPreFfd4Arg800201AC = 0;
    int32_t fastPostFfd4Arg800201AC = 0;
};

enum class Scene0TitleResultResolveStatus801C4DC4 : uint8_t {
    SourceUnknown = 0,
    InvalidInput,
    DrawSourceMissing,
    InvalidDraw,
    DrawsExhausted,
    Accepted,
};

struct Scene0TitleResultResolveInput801C4DC4 {
    bool selectorResultKnown = false;
    int32_t selectorResult = 0;
    bool currentSceneKnown = false;
    int32_t currentScene = 0;
    bool previousSceneKnown = false;
    int32_t previousScene = 0;
    bool injectedRandDrawsKnown = false;
    const int32_t* injectedRandDraws = nullptr;
    std::size_t injectedRandDrawCount = 0u;
};

struct Scene0TitleResultResolveResult801C4DC4 {
    Scene0TitleResultResolveStatus801C4DC4 status =
        Scene0TitleResultResolveStatus801C4DC4::SourceUnknown;
    bool accepted = false;
    bool word800916D0Known = false;
    uint16_t word800916D0 = 0;
    bool publishWord800916D0BeforeDrawConsumption = false;
    bool cue94410Required = false;
    bool flush26ECCRequired = false;
    bool returnSceneKnown = false;
    int32_t returnScene = 0;
    std::size_t drawCount = 0u;
};

bool RuntimeCutoverAllowed();
bool HasKnownModeBodyTicks8001EA74(int32_t mode, bool word800916DC);
int32_t KnownModeBodyTicks8001EA74(int32_t mode, bool word800916DC);

bool BeginFastTransitionRuntime800201AC(
    FastTransitionRuntime800201AC& runtime,
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg,
    bool word800916DC);
FastTransitionTickResult800201AC TickFastTransitionRuntime800201AC(
    FastTransitionRuntime800201AC& runtime);
void ResetFastTransitionRuntime800201AC(
    FastTransitionRuntime800201AC& runtime);
FastTransitionVisualFrame800201AC ResolveFastTransitionVisualFrame800201AC(
    int32_t mode,
    bool word800916DC,
    bool tail,
    uint32_t iteration);
FastTransitionSpriteTemplate800201AC
ResolveFastTransitionSpriteTemplate800201AC(uint32_t sourceAddress);
FastTransitionFramePlan800201AC BuildFastTransitionFramePlan800201AC(
    const FastTransitionVisualFrame800201AC& visualFrame);
FastTransitionPresentPlan8001EBF4 BuildFastTransitionPresentPlan8001EBF4(
    const FastTransitionVisualFrame800201AC& visualFrame,
    const FastTransitionGraphInput8001EA74& graphInput);

bool BeginSlowTransitionRuntime80020110(
    SlowTransitionRuntime80020110& runtime,
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg);
Sub80027194CadenceStep AdvanceSub80027194Cadence(
    Sub80027194CadenceState& state);
SlowTransitionTickResult80020110 TickSlowTransitionRuntime80020110(
    SlowTransitionRuntime80020110& runtime);
void ResetSlowTransitionRuntime80020110(
    SlowTransitionRuntime80020110& runtime);
SlowTransitionVisualFrame80020110
ResolveSlowTransitionVisualFrame80020110(
    uint32_t ctxAddress,
    int32_t mode,
    int32_t preFfd4Arg,
    int32_t postFfd4Arg,
    bool tail,
    uint32_t iteration);
SlowTransitionFramePlan8001FDC0 BuildSlowTransitionFramePlan8001FDC0(
    const SlowTransitionVisualFrame80020110& visualFrame);
FastTransitionPresentPlan8001EBF4
BuildSlowTransitionPresentPlan8001EBF4(
    const SlowTransitionVisualFrame80020110& visualFrame,
    const FastTransitionGraphInput8001EA74& graphInput);
SlowTransitionGridCoordinate8004EB80
ResolveSlowTransitionMode1SpiralCoordinate8004EB80(std::size_t orderIndex);

uint8_t ResolveLoadingPatternStyle8001EF40(int16_t word800916E0);
bool ResolveSceneEntryLoadingMode801C7284(int sceneIndex, int16_t& outMode);
void ResetLoadingPatternState8001EF14(LoadingPatternRuntime8001EF40& runtime);
bool BeginLoadingPatternRuntimeAfter8001FFD4(
    LoadingPatternRuntime8001EF40& runtime,
    int16_t word800916E0,
    const uint8_t* completedLiveGrid,
    std::size_t completedLiveGridCount);
LoadingPatternFrame8001EF40 TickLoadingPatternRuntime8001EF40(
    LoadingPatternRuntime8001EF40& runtime);
void StopLoadingPatternRuntime8001EF40(
    LoadingPatternRuntime8001EF40& runtime);

TransitionPlan BuildMode8001EA74Plan(int32_t mode, bool word800916DC);
TransitionPlan BuildSlow80020110Plan(uint32_t ctxAddress,
                                     int32_t mode,
                                     int32_t preFfd4Arg,
                                     int32_t postFfd4Arg,
                                     bool word800916DC);
TransitionPlan BuildFast800201ACPlan(uint32_t ctxAddress,
                                     int32_t mode,
                                     int32_t preFfd4Arg,
                                     int32_t postFfd4Arg,
                                     bool word800916DC);
Scene0OuterEntryTransaction801C4DC4
BuildScene0OuterEntryTransaction801C4DC4(
    const Scene0OuterEntryInput801C4DC4& input);
TransitionPlan BuildScene0Outer801C4DC4Plan(bool word800916D2WasZero);
TransitionPlan BuildScene0TitleResult801C4DC4Plan(int32_t selectorResult,
                                                  int32_t currentScene,
                                                  int32_t lastRandomScene);
Scene0TitleResultResolveResult801C4DC4
ResolveScene0TitleResult801C4DC4(
    const Scene0TitleResultResolveInput801C4DC4& input);

const char* TransitionPlanKindName(TransitionPlanKind kind);
const char* TransitionActionKindName(TransitionActionKind kind);

} // namespace PrSS0TransitionDirect
