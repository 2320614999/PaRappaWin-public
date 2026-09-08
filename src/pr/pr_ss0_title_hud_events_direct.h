#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0TitleHudEventsDirect {

static constexpr uint32_t kFn801C4894 = 0x801C4894u;
static constexpr uint32_t kFn801C47EC = 0x801C47ECu;
static constexpr uint32_t kFn801C4F68 = 0x801C4F68u;
static constexpr uint32_t kFn801C4FA0 = 0x801C4FA0u;
static constexpr uint32_t kFn801C5094 = 0x801C5094u;
static constexpr uint32_t kFn801C5190 = 0x801C5190u;
static constexpr uint32_t kFn801C5538 = 0x801C5538u;
static constexpr uint32_t kFn801C57E0 = 0x801C57E0u;
static constexpr uint32_t kFn801C5854 = 0x801C5854u;
static constexpr uint32_t kFn801C5AB4 = 0x801C5AB4u;
static constexpr uint32_t kFn801C6410 = 0x801C6410u;
static constexpr uint32_t kFn801C689C = 0x801C689Cu;
static constexpr uint32_t kFn80024E98 = 0x80024E98u;
static constexpr uint32_t kFn80024FD0 = 0x80024FD0u;
static constexpr uint32_t kFn80027664 = 0x80027664u;
static constexpr uint32_t kFn8001A694 = 0x8001A694u;
static constexpr uint32_t kFn80040370 = 0x80040370u;
static constexpr uint32_t kFn80035560 = 0x80035560u;
static constexpr uint32_t kFn80035510 = 0x80035510u;
static constexpr uint32_t kFn8001E3E4 = 0x8001E3E4u;
static constexpr uint32_t kFn80020488 = 0x80020488u;
static constexpr uint32_t kFn80025C8C = 0x80025C8Cu;
static constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
static constexpr uint32_t kFn80026ECC = 0x80026ECCu;
static constexpr uint32_t kFn80034240 = 0x80034240u;

static constexpr uint32_t kCallsite801C4C9C = 0x801C4C9Cu;
static constexpr uint32_t kCallsite801C4D14 = 0x801C4D14u;

static constexpr uint32_t kEventStreamDescriptor801C6E50 = 0x801C6E50u;
static constexpr uint32_t kEventStreamRecords801C6DD4 = 0x801C6DD4u;
static constexpr uint32_t kEventStreamRecordCount = 7u;
static constexpr uint32_t kEventStreamRecordBytes = 16u;
static constexpr uint32_t kComod0MappedBase801C3870 = 0x801C3870u;
static constexpr uint32_t kResourcePairTable801C6C14 = 0x801C6C14u;
static constexpr uint32_t kResourcePairCount801C6C14 = 78u;
static constexpr uint32_t kHudTimelineDescriptor801C6D4C = 0x801C6D4Cu;
static constexpr uint32_t kHudTimelineDescriptorCount = 7u;
static constexpr uint32_t kHudTimelineDescriptorBytes = 12u;
static constexpr uint32_t kHudTimelineRecordBytes = 12u;
static constexpr uint32_t kHudTimelineMaxTimIds = 4u;
static constexpr uint32_t kSelectorTable801C6F94 = 0x801C6F94u;
static constexpr uint32_t kSelectorRecordCount = 4u;
static constexpr uint32_t kSelectorRecordBytes = 8u;
static constexpr uint32_t kEventBuilderCount801C6F84 = 0x801C6F84u;

static constexpr uint32_t kEventStreamBaseFrame801C9558 = 0x801C9558u;
static constexpr uint32_t kSelectorBank801C955C = 0x801C955Cu;
static constexpr uint32_t kSelectorLastCursor801C9560 = 0x801C9560u;
static constexpr uint32_t kHudTimelineEnable801C9564 = 0x801C9564u;
static constexpr uint32_t kHudTimelineCountdown801C9568 = 0x801C9568u;
static constexpr uint32_t kHudCooldownWord8008ECFC = 0x8008ECFCu;
static constexpr uint32_t kCtxFrameOffset0C = 0x0Cu;
static constexpr uint32_t kCtxRuntimeTimListOffsetAC = 0xACu;
static constexpr uint32_t kCtxRuntimeTimListStride = 4u;
static constexpr uint32_t kCtxRuntimeTimListObservedChannel = 0u;
static constexpr uint32_t kCtxSelectorResourceOffsetDC = 0xDCu;
static constexpr uint32_t kCtxSelectorResourceOffsetE8 = 0xE8u;
static constexpr uint32_t kCtxFlagEventStreamDone = 0x100u;
static constexpr uint32_t kCtxFlagRuntimeTimList = 0x8000u;
static constexpr uint32_t kCtxFlagSelectorMime = 0x10000u;
static constexpr uint32_t kCtxFlagHudStart = 0x40000u;
static constexpr uint32_t kCtxFlagShortcutStart = 0x4000000u;
static constexpr uint32_t kCtxFlagShortcutReady = 0x8000000u;
static constexpr uint32_t kTitleShortcutFirstTickCtxFlags801C5AB4 =
    kCtxFlagShortcutStart | kCtxFlagShortcutReady | kCtxFlagSelectorMime;
static constexpr uint8_t kTitleShortcutTodResourceF4Index801C5AB4 = 4u;
static constexpr uint32_t kShortcutHoldWaitFrames = 15u;
static constexpr uint32_t kShortcutFrameWait = 2u;
static constexpr uint8_t kTitleSelectorInitialBank801C57E0 = 1u;
static constexpr uint8_t kTitleSelectorInitialHudSlot801C57E0 = 6u;
static constexpr uint8_t kTitleSelectorInitialLastCursor801C57E0 = 0u;
static constexpr int32_t kTitleSelectorInitialCountdown801C57E0 = 6;
static constexpr uint32_t kCueDword80094410 = 0x80094410u;
static constexpr uint32_t kTitleNormalSelectorEntryFrame801C4894 = 370u;
static constexpr uint32_t kTitleAttractTimeoutDefaultLoad801C48BC =
    0x801C48BCu;
static constexpr uint32_t kTitleAttractTimeoutSourceRead801C48C0 =
    0x801C48C0u;
static constexpr uint32_t kTitleAttractTimeoutBranch801C48DC =
    0x801C48DCu;
static constexpr uint32_t kTitleAttractTimeoutShortLoad801C48E4 =
    0x801C48E4u;
static constexpr uint32_t kTitleAttractTimeoutWord800916FC = 0x800916FCu;
static constexpr uint32_t kTitleAttractTimeoutDefault801C4894 = 0x708u;
static constexpr uint32_t kTitleAttractTimeoutShort801C4894 = 0x1C2u;
static constexpr uint32_t kTitleSelectorInputConfirmMask801C47EC = 0x40u;
static constexpr uint32_t kTitleSelectorInputMoveMask801C47EC = 0xA000u;
static constexpr uint32_t kTitleSelectorInputLeftMask801C47EC = 0x8000u;
static constexpr uint32_t kTitleSelectorInputRightMask801C47EC = 0x2000u;
static constexpr uint32_t kTitleSelectorInputConfirmCue801C47EC = 0x20u;
static constexpr uint32_t kTitleSelectorInputMoveCue801C47EC = 0x1000u;
static constexpr uint32_t kTitleSelectorResultNone801C47EC = 0u;
static constexpr uint32_t kTitleSelectorResultMenu801C47EC = 1u;
static constexpr uint32_t kTitleSelectorResultStart801C47EC = 2u;
static constexpr uint32_t kTitleSelectorExitWaitInitialCounter801C4894 = 15u;
static constexpr uint32_t kTitleSelectorExitWaitFrameWait801C4894 = 2u;
static constexpr std::size_t kTitleSelectorPromptSpriteCount80020488 = 6u;

enum class TitleHudTableKind : uint8_t {
    Unknown = 0,
    EventStream,
    ResourcePairs,
    HudTimelines,
    Selector,
    EventBuilderCount,
};

enum class TitleHudActionKind : uint8_t {
    None = 0,
    ClearEventStreamCursor801C4FA0,
    ResetRelativeEventCursor801C4F68,
    StoreEventBaseFrame801C9558,
    GateEventStreamTimeSource801C5538,
    Call801C5538Tick,
    MatchEventRecord,
    ApplyEventRecord801C5190,
    SetCtxFlagsFromEvent,
    BindResourcePairIndex,
    SetHudSlot,
    SetHudSlotBaseFrame,
    ResetHudTimelineCursor,
    GateHudTimelineChannelSource801C5094,
    PollHudTimeline801C5094,
    ClearHudTimelineSlot,
    WriteRuntimeTimList,
    StartHudOverlay801C57E0,
    GateSelectorStateSource801C5854,
    SetSelectorBank,
    SetSelectorLastCursor,
    SetSelectorCooldown,
    UpdateSelectorResources,
    StartAndApply801C5AB4,
    GateTitleNormalSelectorEntrySource801C4894,
    Call8001A694SelectorEntryWaitCleanup,
    LatchTitleSelectorStateV8,
    Call80026EF8SelectorEntryCue,
    Nested80034240SelectorEntryVoice,
    Call80026ECCSelectorEntryFlush,
    GateTitleSelectorInputHelperSource801C47EC,
    Call80025C8CSelectorInputCue,
    Call80026EF8SelectorInputCue,
    Nested80034240SelectorInputVoice,
    Call80026ECCSelectorInputFlush,
    ToggleTitleSelectorCursor801C47EC,
    ReturnTitleSelectorConfirm801C47EC,
    GateTitleSelectorExitWaitSource801C4894,
    DecrementTitleSelectorExitWaitCounter,
    Call801C6410SelectorExitWaitRender,
    ClearTitleCtxFlagsForExitWait,
    Call8001E3E4SelectorExitWaitTail,
    Call80035560SelectorExitWaitWait,
    Call801C689CSelectorExitWaitPresent,
    GateTitleEarlyInputSource801C4894,
    Call80027664EarlyInputStopReset,
    Call8001A694EarlyInputWaitCleanup,
    Call80040370EarlyInputFlip,
    SetTitleShortcutStartFlag,
    Call801C6410ShortcutRender,
    Call80035560ShortcutWait,
    Call801C689CShortcutPresent,
    Call80035560ShortcutHoldLoop,
    SetTitleShortcutReadyFlag,
    WaitTitleShortcutInputRelease,
    Call80026EF8ShortcutCue,
    Call80026ECCShortcutFlush,
    Gap,
};

struct TitleHudEventRecord {
    uint32_t frame = 0;
    uint32_t flags = 0;
    uint8_t bytes[8]{};
};

// Runtime source state for the COMOD0 tables consumed by 801C5190,
// 801C5094, 801C57E0 and 801C5854.  The translated implementation keeps a
// built-in copy only for unit tests; live SS0 must first import these bytes
// from the current COMOD0 payload.
struct TitleHudComodSourceState801C6Dxx {
    bool loaded = false;
    bool descriptorKnown = false;
    bool eventStreamKnown = false;
    bool resourcePairsKnown = false;
    bool timelineKnown = false;
    bool selectorKnown = false;
    uint32_t mappedBase = 0u;
    uint32_t eventStreamAddress = 0u;
    uint32_t eventStreamCount = 0u;
    uint32_t resourcePairCount = 0u;
    uint32_t timelineDescriptorCount = 0u;
    uint32_t selectorCount = 0u;
};

struct TitleHudTimelineDescriptor {
    uint32_t eventsPtr = 0;
    uint32_t count = 0;
    uint32_t cursorInitial = 0;
};

struct TitleHudTimelineRecord801C5094 {
    uint32_t deltaFrame = 0;
    int16_t timIds[kHudTimelineMaxTimIds]{};
};

struct TitleHudSelectorRecord {
    int16_t idA = 0;
    int16_t idB = 0;
    uint16_t ticks = 0;
    int16_t hudSlotId = 0;
};

struct TitleHudAction {
    TitleHudActionKind kind = TitleHudActionKind::None;
    uint32_t psxFunction = 0;
    TitleHudTableKind table = TitleHudTableKind::Unknown;
    uint32_t index = 0;
    uint32_t arg0 = 0;
    uint32_t arg1 = 0;
    uint32_t arg2 = 0;
    uint32_t arg3 = 0;
    bool conditional = false;
};

struct TitleHudPlan {
    bool runtimeCutoverAllowed = false;
    bool hasOpenP0Gap = true;
    TitleHudAction actions[96]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct TitleEventClock801C4894 {
    bool known = false;
    int32_t loopTickV10 = -17;
    uint32_t tick96 = 0;
    bool beatKnown = false;
    uint8_t beat = 0;
    uint8_t tickWithinBeat = 0;
};

struct TitleSharedEventBucketState80024FD0 {
    bool resetKnown = false;
    uint32_t dword8008ED20 = 0;
    uint32_t dword8008ECE8 = 0;
    uint32_t dword8008ECF0 = 0;
    uint32_t dword8008ECF4 = 0;
};

struct TitleSharedEventBucketInput80024FD0 {
    bool callsiteKnown = false;
    uint32_t callsite = 0;
    bool ctxTick96Known = false;
    uint32_t ctxTick96 = 0;
};

enum class TitleSharedEventBucketStatus80024FD0 : uint8_t {
    SourceUnknown = 0,
    UnsupportedCallsite,
    Disabled,
    BucketUnchanged,
    BucketAdvanced,
};

struct TitleSharedEventBucketResult80024FD0 {
    TitleSharedEventBucketStatus80024FD0 status =
        TitleSharedEventBucketStatus80024FD0::SourceUnknown;
    bool accepted = false;
    bool prefixApplied = false;
    bool earlyReturnEd20 = false;
    bool bucketChanged = false;
    bool sharedTailRequired = false;
    TitleSharedEventBucketState80024FD0 nextState{};
};

enum class TitleAttractTimeoutPolicyStatus801C48C0 : uint8_t {
    SourceUnknown = 0,
    Default1800,
    Short450,
};

struct TitleAttractTimeoutPolicyInput801C48C0 {
    bool word800916FCKnown = false;
    uint16_t word800916FC = 0;
};

struct TitleAttractTimeoutPolicyResult801C48C0 {
    TitleAttractTimeoutPolicyStatus801C48C0 status =
        TitleAttractTimeoutPolicyStatus801C48C0::SourceUnknown;
    bool accepted = false;
    bool timeoutKnown = false;
    uint32_t timeoutFrames = 0;
};

struct TitleNaturalLoopCadenceState801C4894 {
    uint8_t holdTicksRemaining = 0;
};

struct TitleNaturalLoopCadenceTick801C4894 {
    bool processIteration = false;
    bool holdTick = false;
};

struct TitleEventEffect801C5190 {
    bool known = false;
    uint32_t eventIndex = 0;
    uint32_t thresholdTick96 = 0;
    bool appliedTick96Known = false;
    uint32_t appliedTick96 = 0;
    uint32_t ctxFlagsSetMask = 0;
    bool eventStatus80SetOne = false;
    bool objectResource104Write = false;
    uint8_t objectResource104Index = 0;
    bool todResourceF4Write = false;
    uint8_t todResourceF4Index = 0;
    bool channel2PairWrite = false;
    uint8_t channel2PairIndex = 0;
    bool extraResourceFCWrite = false;
    uint8_t extraResourceFCIndex = 0;
    bool channel1PairWrite = false;
    uint8_t channel1PairIndex = 0;
    bool channel3PairWrite = false;
    uint8_t channel3PairIndex = 0;
    bool hudSlot0Write = false;
    uint8_t hudSlot0 = 0;
};

struct TitleHudTimelineChannel0State801C5094 {
    bool descriptorAvailable = false;
    bool cursorKnown = false;
    uint32_t cursor = 0;
    int16_t emittedTimIds[kHudTimelineMaxTimIds]{};
    uint32_t emittedTimIdCount = 0;
};

struct TitleEventRuntimeState801C5190 {
    uint32_t ctxFlags = 0;
    bool eventStatus80Known = false;
    bool eventStatus80 = false;
    bool objectResource104Known = false;
    uint8_t objectResource104Index = 0;
    bool todResourceF4Known = false;
    uint8_t todResourceF4Index = 0;
    bool channel2PairKnown = false;
    uint8_t channel2PairIndex = 0;
    bool extraResourceFCKnown = false;
    uint8_t extraResourceFCIndex = 0;
    bool channel1PairKnown = false;
    uint8_t channel1PairIndex = 0;
    bool channel3PairKnown = false;
    uint8_t channel3PairIndex = 0;
    bool hudSlot0Known = false;
    uint8_t hudSlot0 = 0;
    bool hudSlot0BaseTick96Known = false;
    uint32_t hudSlot0BaseTick96 = 0;
    TitleHudTimelineChannel0State801C5094 hudTimelineChannel0{};
};

struct TitleEventRuntimeCommitResult801C5190 {
    bool accepted = false;
    TitleEventRuntimeState801C5190 nextState{};
};

struct TitleHudTimelineTickInput801C5094 {
    bool channelKnown = false;
    uint32_t channel = 0;
    bool currentTick96Known = false;
    uint32_t currentTick96 = 0;
};

enum class TitleHudTimelineTickStatus801C5094 : uint8_t {
    SourceUnknown = 0,
    UnsupportedChannel,
    InvalidSlot,
    DescriptorUnavailable,
    InvalidCursor,
    ThresholdOverflow,
    NotDue,
    Applied,
    Complete,
};

struct TitleHudTimRequest801C5094 {
    bool valid = false;
    int16_t timIds[kHudTimelineMaxTimIds]{};
    uint32_t timIdCount = 0;
    uint8_t slotId = 0;
    uint32_t recordIndex = 0;
    uint32_t baseTick96 = 0;
    uint32_t appliedTick96 = 0;
};

struct TitleHudTimelineTickResult801C5094 {
    TitleHudTimelineTickStatus801C5094 status =
        TitleHudTimelineTickStatus801C5094::SourceUnknown;
    TitleEventRuntimeState801C5190 nextState{};
    TitleHudTimRequest801C5094 request{};
};

struct TitleSelectorHudRuntimeState801C5854 {
    bool known = false;
    uint8_t bank = 0;
    uint8_t lastCursor = 0;
    bool timelineEnabled = false;
    int32_t timelineCountdown = 0;
    bool resourceCooldownKnown = false;
    int32_t resourceCooldown = 0;
};

struct TitleSelectorHudStartResult801C57E0 {
    bool accepted = false;
    TitleSelectorHudRuntimeState801C5854 nextSelectorState{};
    TitleEventRuntimeState801C5190 nextEventState{};
};

struct TitleSelectorHudTickInput801C5854 {
    bool currentTick96Known = false;
    uint32_t currentTick96 = 0;
    bool beatKnown = false;
    uint8_t beat = 0;
    bool cursorKnown = false;
    uint8_t cursor = 0;
};

enum class TitleSelectorHudTickStatus801C5854 : uint8_t {
    SourceUnknown = 0,
    InvalidBeat,
    InvalidCursor,
    InvalidState,
    InvalidSelectorRecord,
    TimelineRejected,
    Applied,
};

struct TitleSelectorResourceUpdate801C5854 {
    bool known = false;
    uint32_t selectorIndex = 0;
    int16_t idA = 0;
    int16_t idB = 0;
    uint16_t targetOffsetA = 0;
    uint16_t targetOffsetB = 0;
    bool appliedTick96Known = false;
    uint32_t appliedTick96 = 0;
};

struct TitleSelectorHudTickResult801C5854 {
    TitleSelectorHudTickStatus801C5854 status =
        TitleSelectorHudTickStatus801C5854::SourceUnknown;
    TitleSelectorHudRuntimeState801C5854 nextSelectorState{};
    TitleEventRuntimeState801C5190 nextEventState{};
    TitleHudTimRequest801C5094 request{};
    TitleSelectorResourceUpdate801C5854 selectorResourceUpdate{};
    TitleEventEffect801C5190 selectorMimeEffect{};
    uint32_t selectorIndex = 0;
};

struct TitleShortcutHudResetTransaction801C5AB4 {
    bool reset80024E98Applied = false;
    uint32_t firstTickCtxTick96 = 0;
    uint8_t firstTickBeatDerived = 0;
    uint8_t firstTickCursor = 0;
    bool callerReadyFlagApplied = false;
    TitleSelectorHudTickResult801C5854 firstTick{};
};

enum class TitleSelectorInputStatus801C47EC : uint8_t {
    SourceUnknown = 0,
    InvalidCursor,
    Applied,
};

struct TitleSelectorInputResult801C47EC {
    TitleSelectorInputStatus801C47EC status =
        TitleSelectorInputStatus801C47EC::SourceUnknown;
    bool accepted = false;
    bool playCue = false;
    uint16_t cueMask = 0;
    bool cursorWrite = false;
    uint8_t nextCursor = 0;
    uint8_t selectorResult = kTitleSelectorResultNone801C47EC;
};

struct TitleSelectorInputEdgeResult801C4CE0 {
    bool accepted = false;
    bool inputChanged = false;
    bool resetTimeout = false;
    uint16_t nextPreviousInput = 0;
    TitleSelectorInputResult801C47EC action{};
};

enum class TitleSelectorExitWaitTickStatus801C4894 : uint8_t {
    InvalidState = 0,
    FrameWait,
    RenderAndWait,
    RenderAndComplete,
    Complete,
};

struct TitleSelectorExitWaitTickResult801C4894 {
    TitleSelectorExitWaitTickStatus801C4894 status =
        TitleSelectorExitWaitTickStatus801C4894::InvalidState;
    bool accepted = false;
    bool renderFrame = false;
    bool complete = false;
    uint32_t nextWaitCounter = 0;
    uint32_t nextFrameWaitRemaining = 0;
};

enum class TitleSelectorExitWaitPresentGateStatus801C4D58 : uint8_t {
    InvalidState = 0,
    FrameWait,
    PreparePresent,
    AwaitPresent,
    PresentCommitted,
    Complete,
};

struct TitleSelectorExitWaitPresentGateState801C4D58 {
    uint32_t waitCounter = 0;
    uint32_t frameWaitRemaining = 0;
    bool presentPending = false;
    bool presentAcknowledged = false;
    uint32_t pendingNextWaitCounter = 0;
    uint32_t pendingNextFrameWaitRemaining = 0;
    bool pendingComplete = false;
};

struct TitleSelectorExitWaitPresentGateResult801C4D58 {
    TitleSelectorExitWaitPresentGateStatus801C4D58 status =
        TitleSelectorExitWaitPresentGateStatus801C4D58::InvalidState;
    bool accepted = false;
    bool preparePresent = false;
    bool transitionReady = false;
    TitleSelectorExitWaitPresentGateState801C4D58 nextState{};
};

struct TitleSelectorExitWaitPresentAckResult801C4D74 {
    bool accepted = false;
    TitleSelectorExitWaitPresentGateState801C4D58 nextState{};
};

enum class TitleEarlyInputShortcutHoldTickStatus801C4894 : uint8_t {
    InvalidState = 0,
    Waiting,
    Ready,
};

struct TitleEarlyInputShortcutHoldTickResult801C4894 {
    TitleEarlyInputShortcutHoldTickStatus801C4894 status =
        TitleEarlyInputShortcutHoldTickStatus801C4894::InvalidState;
    bool accepted = false;
    bool ready = false;
    uint32_t nextCompletedWaitCalls = 0;
    uint32_t nextFrameWaitRemaining = 0;
};

struct TitleSelectorPromptTemplate80020488 {
    uint32_t sourceAddress = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

struct TitleSelectorPromptSprite80020488 {
    bool active = false;
    int16_t x = 0;
    int16_t y = 0;
    TitleSelectorPromptTemplate80020488 spriteTemplate{};
};

enum class TitleSelectorPromptPlanStatus80020488 : uint8_t {
    SourceUnknown = 0,
    UnsupportedSelector,
    Ready,
};

struct TitleSelectorPromptPlan80020488 {
    TitleSelectorPromptPlanStatus80020488 status =
        TitleSelectorPromptPlanStatus80020488::SourceUnknown;
    bool accepted = false;
    uint32_t selectorValue = 0;
    uint32_t wrapperFunction = kFn8001E3E4;
    uint32_t drawFunction = kFn80020488;
    std::array<TitleSelectorPromptSprite80020488,
               kTitleSelectorPromptSpriteCount80020488>
        sprites{};
};

struct TitleAbsoluteEventStreamState801C5538 {
    // 80024E98 owns the Scene0 title stream selectors globally.  Keep those
    // selectors in the translated stream carrier instead of re-injecting
    // literals at every 801C5538 call.
    bool resetKnown = false;
    bool streamFlagKnown = false;
    uint16_t streamFlag = 0;
    bool streamIdKnown = false;
    uint16_t streamId = 0;
    uint32_t cursor = 0;
};

struct TitleAbsoluteEventTickInput801C5538 {
    bool ctxTick96Known = false;
    uint32_t ctxTick96 = 0;
    bool streamFlagKnown = false;
    uint16_t streamFlag = 0;
    bool streamIdKnown = false;
    uint16_t streamId = 0;
};

enum class TitleAbsoluteEventTickStatus801C5538 : uint8_t {
    SourceUnknown = 0,
    UnsupportedMode,
    InvalidState,
    NotDue,
    Applied,
    Complete,
};

struct TitleAbsoluteEventTickResult801C5538 {
    TitleAbsoluteEventTickStatus801C5538 status =
        TitleAbsoluteEventTickStatus801C5538::SourceUnknown;
    TitleAbsoluteEventStreamState801C5538 nextState{};
    TitleEventEffect801C5190 effect{};
};

bool RuntimeCutoverAllowed();

TitleEventClock801C4894 ComputeTitleEventClock801C4894(
    int32_t loopTickV10,
    bool initialTick96Known,
    uint32_t initialTick96);
void ResetTitleSharedEventBucketState80024E98(
    TitleSharedEventBucketState80024FD0& state);
void ResetTitleEventRuntimeState80024E98(
    TitleEventRuntimeState801C5190& state);
void ResetTitleAbsoluteEventStreamState80024E98(
    TitleAbsoluteEventStreamState801C5538& state);
TitleSharedEventBucketResult80024FD0 TickTitleSharedEventBucketPrefix80024FD0(
    const TitleSharedEventBucketState80024FD0& state,
    const TitleSharedEventBucketInput80024FD0& input);
TitleAttractTimeoutPolicyResult801C48C0
ResolveTitleAttractTimeoutPolicy801C48C0(
    const TitleAttractTimeoutPolicyInput801C48C0& input);
TitleNaturalLoopCadenceTick801C4894 TickTitleNaturalLoopCadence801C4894(
    TitleNaturalLoopCadenceState801C4894& state,
    bool holdOneHostTick = true);
TitleEventEffect801C5190 DecodeTitleEventEffect801C5190(
    uint32_t eventIndex);
TitleEventRuntimeCommitResult801C5190 BuildTitleEventRuntimeCommit801C5190(
    const TitleEventRuntimeState801C5190& state,
    const TitleEventEffect801C5190& effect);
TitleHudTimelineTickResult801C5094 TickTitleHudTimeline801C5094(
    const TitleEventRuntimeState801C5190& state,
    const TitleHudTimelineTickInput801C5094& input);
TitleSelectorHudStartResult801C57E0 StartTitleSelectorHud801C57E0(
    const TitleEventRuntimeState801C5190& eventState,
    const TitleSelectorHudRuntimeState801C5854& selectorState,
    bool currentTick96Known,
    uint32_t currentTick96);
TitleSelectorHudTickResult801C5854 TickTitleSelectorHud801C5854(
    const TitleEventRuntimeState801C5190& eventState,
    const TitleSelectorHudRuntimeState801C5854& selectorState,
    const TitleSelectorHudTickInput801C5854& input);
TitleSelectorInputResult801C47EC ResolveTitleSelectorInput801C47EC(
    bool inputMaskKnown,
    uint16_t inputMask,
    bool cursorKnown,
    uint8_t cursor);
TitleSelectorInputEdgeResult801C4CE0 ResolveTitleSelectorInputEdge801C4CE0(
    bool previousInputKnown,
    uint16_t previousInput,
    bool currentInputKnown,
    uint16_t currentInput,
    bool cursorKnown,
    uint8_t cursor);
TitleShortcutHudResetTransaction801C5AB4
BuildShortcutTitleSelectorHudStartAndFirstTick801C5AB4();
TitleSelectorExitWaitTickResult801C4894
TickTitleSelectorExitWait801C4894(uint32_t waitCounter,
                                  uint32_t frameWaitRemaining);
TitleSelectorExitWaitPresentGateResult801C4D58
TickTitleSelectorExitWaitPresentGate801C4D58(
    const TitleSelectorExitWaitPresentGateState801C4D58& state);
TitleSelectorExitWaitPresentAckResult801C4D74
AcknowledgeTitleSelectorExitWaitPresent801C4D74(
    const TitleSelectorExitWaitPresentGateState801C4D58& state,
    bool selectorPromptDrawn,
    bool titlePacketFrameSubmitted,
    bool presentModelApplied);
TitleEarlyInputShortcutHoldTickResult801C4894
TickTitleEarlyInputShortcutHold801C4894(uint32_t completedWaitCalls,
                                        uint32_t frameWaitRemaining);
TitleSelectorPromptPlan80020488 BuildTitleSelectorPrompt8001E3E4(
    bool selectorValueKnown,
    uint32_t selectorValue);
TitleAbsoluteEventTickResult801C5538 TickTitleAbsoluteEventStream801C5538(
    const TitleAbsoluteEventStreamState801C5538& state,
    const TitleAbsoluteEventTickInput801C5538& input);

uint32_t KnownEventStreamRecordCount();
TitleHudEventRecord KnownEventStreamRecordAt(uint32_t index);
void ResetTitleHudComodSource801C6Dxx();
bool LoadTitleHudTablesFromComod0(
    const uint8_t* data,
    std::size_t size,
    uint32_t mappedBase = kComod0MappedBase801C3870);
const TitleHudComodSourceState801C6Dxx&
GetTitleHudComodSourceState801C6Dxx();
uint32_t KnownHudTimelineDescriptorCount();
TitleHudTimelineDescriptor KnownHudTimelineDescriptorAt(uint32_t index);
uint32_t KnownHudTimelineRecordCount(uint32_t slotId);
TitleHudTimelineRecord801C5094 KnownHudTimelineRecordAt(
    uint32_t slotId,
    uint32_t recordIndex);
uint32_t KnownSelectorRecordCount();
TitleHudSelectorRecord KnownSelectorRecordAt(uint32_t index);
uint8_t KnownSelectorResourcePairIndexAt(uint32_t index);

TitleHudPlan BuildEventStreamClear801C4FA0Plan();
TitleHudPlan BuildRelativeEventReset801C4F68Plan(uint32_t ctxFrame,
                                                 uint32_t id);
TitleHudPlan BuildApplyEventRecord801C5190Plan(uint32_t eventIndex);
TitleHudPlan BuildEventStreamTick801C5538Plan(uint32_t ctxFrame,
                                              uint32_t cursor,
                                              bool relativeMode,
                                              uint32_t baseFrame);
TitleHudPlan BuildHudTimelinePoll801C5094Plan(uint32_t channel,
                                              uint32_t slotId,
                                              uint32_t cursor,
                                              uint32_t ctxFrame,
                                              uint32_t baseFrame);
TitleHudPlan BuildSelectorHud801C5854Plan(uint32_t cursor,
                                          uint32_t bank,
                                          uint32_t cooldown);
TitleHudPlan BuildTitleSelectorStart801C57E0Plan(uint32_t ctxFrame,
                                                 uint32_t initialHudSlot);
TitleHudPlan BuildTitleNormalSelectorEntry801C4894Plan(
    uint32_t ctxFrame,
    uint32_t selectorTimeoutFrames,
    uint32_t initialHudSlot);
TitleHudPlan BuildTitleSelectorInputHelper801C47ECPlan(uint32_t inputMask,
                                                       uint32_t cursorBefore);
TitleHudPlan BuildTitleSelectorExitWait801C4894Plan(uint32_t selectorResult,
                                                    uint32_t waitCounter,
                                                    uint32_t ctxFrame);
TitleHudPlan BuildShortcutStartAndApply801C5AB4Plan(uint32_t ctxFrame,
                                                    uint32_t cursor);
TitleHudPlan BuildTitleEarlyInputShortcut801C4894Plan(uint32_t ctxFrame,
                                                      uint32_t cursor,
                                                      uint32_t stateV8,
                                                      uint32_t inputMask,
                                                      bool inputChanged);

const char* TitleHudTableKindName(TitleHudTableKind kind);
const char* TitleHudActionKindName(TitleHudActionKind kind);

} // namespace PrSS0TitleHudEventsDirect
