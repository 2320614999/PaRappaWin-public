#pragma once

#include <cstdint>

#include "pr_ss0_event4_prompt_render_direct.h"

namespace PrSS0EventFrameLoopDirect {

static constexpr uint32_t kFn80026B94 = 0x80026B94u;
static constexpr uint32_t kFn8001E750 = 0x8001E750u;
static constexpr uint32_t kFn8001D74C = 0x8001D74Cu;
static constexpr uint32_t kFn80035510 = 0x80035510u;
static constexpr uint32_t kFn80035560 = 0x80035560u;
static constexpr uint32_t kFn8001EA00 = 0x8001EA00u;
static constexpr uint32_t kFn800436F0 = 0x800436F0u;
static constexpr uint32_t kFn80040370 = 0x80040370u;
static constexpr uint32_t kFn80040420 = 0x80040420u;
static constexpr uint32_t kFn80040CA4 = 0x80040CA4u;
static constexpr uint32_t kFn80043A14 = 0x80043A14u;
static constexpr uint32_t kFn80020110 = 0x80020110u;
static constexpr uint32_t kFn80025C8C = 0x80025C8Cu;
static constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
static constexpr uint32_t kFn80026ECC = 0x80026ECCu;
static constexpr uint32_t kFn80025D70 = 0x80025D70u;
static constexpr uint32_t kFn8004019C = 0x8004019Cu;
static constexpr uint32_t kFn80040F90 = 0x80040F90u;
static constexpr uint32_t kFn80040CC8 = 0x80040CC8u;
static constexpr uint32_t kFn8003EE84 = 0x8003EE84u;
static constexpr uint32_t kFn80044E2C = 0x80044E2Cu;
static constexpr uint32_t kFn800468E0 = 0x800468E0u;
static constexpr uint32_t kFn80046840 = 0x80046840u;

static constexpr uint32_t kFn800267F8 = 0x800267F8u;
static constexpr uint32_t kFn80025F6C = 0x80025F6Cu;
static constexpr uint32_t kFn80026170 = 0x80026170u;
static constexpr uint32_t kFn80026794 = 0x80026794u;
static constexpr uint32_t kFn800264AC = 0x800264ACu;
static constexpr uint32_t kFn80026720 = 0x80026720u;
static constexpr uint32_t kFn800267C8 = 0x800267C8u;
static constexpr uint32_t kFn80025F0C = 0x80025F0Cu;
static constexpr uint32_t kFn80025E6C = 0x80025E6Cu;
static constexpr uint32_t kFn800267E4 = 0x800267E4u;
static constexpr uint32_t kFn80025E0C = 0x80025E0Cu;
static constexpr uint32_t kFn80025E48 = 0x80025E48u;
static constexpr uint32_t kFn800267F0 = 0x800267F0u;
static constexpr uint32_t kFn800268E4 = 0x800268E4u;
static constexpr uint32_t kFn80026910 = 0x80026910u;
static constexpr uint32_t kFn80026B54 = 0x80026B54u;

static constexpr uint32_t kFn800203D4 = 0x800203D4u;
static constexpr uint32_t kFn8001B6C4 = 0x8001B6C4u;
static constexpr uint32_t kFn8001B120 = 0x8001B120u;
static constexpr uint32_t kFn80020568 = 0x80020568u;
static constexpr uint32_t kFn80020BE4 = 0x80020BE4u;
static constexpr uint32_t kFn80020F94 = 0x80020F94u;
static constexpr uint32_t kFn80021594 = 0x80021594u;
static constexpr uint32_t kFn80021910 = 0x80021910u;
static constexpr uint32_t kFn80021E60 = 0x80021E60u;
static constexpr uint32_t kFn80022CBC = 0x80022CBCu;
static constexpr uint32_t kFn80023618 = 0x80023618u;
static constexpr uint32_t kFn80015590 = 0x80015590u;
static constexpr uint32_t kFn80015618 = 0x80015618u;
static constexpr uint32_t kFn8002776C = 0x8002776Cu;

static constexpr uint32_t kEventTable2_8005453C = 0x8005453Cu;
static constexpr uint32_t kEventTable3_80054550 = 0x80054550u;
static constexpr uint32_t kEventTable4_80054564 = 0x80054564u;
static constexpr uint32_t kEventTable6_80054578 = 0x80054578u;
static constexpr uint32_t kEventTable10_8005458C = 0x8005458Cu;
static constexpr uint32_t kEventTable17_800545A0 = 0x800545A0u;

static constexpr uint32_t kCtxEvent2_80087B78 = 0x80087B78u;
static constexpr uint32_t kCtxEvent3_800544F8 = 0x800544F8u;
static constexpr uint32_t kCtxEvent4_8006ED74 = 0x8006ED74u;
static constexpr uint32_t kCtxEvent6Default_8005424C = 0x8005424Cu;
static constexpr uint32_t kCtxEvent6HiScoreTable80049278 = 0x80049278u;
// Event4 resolves the shared draw work list in the 8001E750 preamble, but it
// owns a separate body transaction and therefore keeps its own source names.
static constexpr uint32_t kEvent4DrawWorkListBase80087288 = 0x80087288u;
static constexpr uint32_t kEvent4DrawWorkListStride8001E750 = 0x14u;
static constexpr uint32_t kEvent6EndFrameWorkListBase80087288 = 0x80087288u;
static constexpr uint32_t kEvent6EndFrameWorkListStride8001EA00 = 0x14u;
static constexpr uint8_t kEvent6EndFrameClearR80040420 = 0u;
static constexpr uint8_t kEvent6EndFrameClearG80040420 = 0u;
static constexpr uint8_t kEvent6EndFrameClearB80040420 = 70u;
static constexpr uint32_t kCtxEvent17_8005451C = 0x8005451Cu;
// Event17 has its own end-frame owner contract. These duplicate the formal
// PSX constants instead of sharing Event6 state or an old-S0 helper.
static constexpr uint32_t kEvent17EndFrameActionCount8001EA00 = 3u;
static constexpr uint32_t kEvent17EndFrameWorkListBase80087288 = 0x80087288u;
static constexpr uint32_t kEvent17EndFrameWorkListStride8001EA00 = 0x14u;
static constexpr uint8_t kEvent17EndFrameClearR80040420 = 0u;
static constexpr uint8_t kEvent17EndFrameClearG80040420 = 0u;
static constexpr uint8_t kEvent17EndFrameClearB80040420 = 70u;
static constexpr uint32_t kPadLatchDword800882F0 = 0x800882F0u;
static constexpr uint32_t kScene0WorkAddress = 0x801C3640u;
static constexpr uint32_t kCueEvent4_8009441C = 0x8009441Cu;
static constexpr uint32_t kGpEvent4WrapperStateOffset = 0x38Cu;
// Event4's modal owner has a separate two-word cadence state in the current
// IDB: gp+0x328 is the 0..72 cue frame and gp+0x32C is the remaining count.
static constexpr uint32_t kGpEvent4ModalCueFrameOffset = 0x328u;
static constexpr uint32_t kGpEvent4ModalRemainingOffset = 0x32Cu;
static constexpr uint32_t kEvent4ModalInitialRemaining = 2u;
static constexpr uint32_t kEvent4ModalCueFrameOffset80025E6C = 36u;
static constexpr uint32_t kEvent4ModalCueFramePeriod80025E6C = 72u;
static constexpr uint32_t kEvent4ModalSecondCueByteOffset80025E6C = 6u;
static constexpr uint32_t kEvent4FrameCommandCount80026B94 = 5u;
static constexpr int32_t kEvent4BackdropBoxFillX8001B6C4 = 0;
static constexpr int32_t kEvent4BackdropBoxFillY8001B6C4 = 0;
static constexpr int32_t kEvent4BackdropBoxFillW8001B6C4 = 320;
static constexpr int32_t kEvent4BackdropBoxFillH8001B6C4 = 240;
// Current-IDB 8001E750 passes 0x400F0F0F to 8001B6C4.  Keep this
// PSX primitive attribute separate from the unrelated card-banner fill.
static constexpr uint32_t kEvent4BackdropBoxFillAttr8001B6C4 = 0x400F0F0Fu;
static constexpr int32_t kEvent4BackdropBoxFillPriority8001B6C4 = 0;
static constexpr int32_t kEvent4MoveImageW8001B120 = 320;
static constexpr int32_t kEvent4MoveImageH8001B120 = 240;
static constexpr int32_t kDispatcherResultTailFrames80026B94 = 60;
static constexpr int32_t kEvent17InitialInputTimeoutFrames80026B94 = 0x4B0;
static constexpr int32_t kEvent17TimeoutResult80026B94 = 3;

enum class EventFrameOwner : uint8_t {
    Unknown = 0,
    DispatcherLoop,
    ModalPrompt,
    DrawOnly,
    PracticeSelfLoop,
    Placeholder,
};

enum class EventFrameActionKind : uint8_t {
    None = 0,
    SelectTable,
    CallInit,
    GateArgCtxFromArg,
    OverrideCtxFromArg,
    GateEvent6HiScoreArg80049278,
    PreTransition80020110,
    WaitPadRelease80035510,
    ResetLastPad,
    ReadPadChange80035510,
    GatePadChangeSource80035510,
    CallHandle,
    CallTick,
    TimeoutCountdown,
    PrepareDrawWork8001D74C,
    DrawRoute8001E750,
    WaitFrame80035560,
    EndFrame8001EA00,
    TextFlush800436F0,
    TailFrames60,
    CallInputSfx80025C8C,
    PlayCue80026EF8,
    Flush80026ECC,
    StageClearTextGap,
    GateEvent4DrawWrapperSource8001E750,
    Event4PromptDraw800203D4,
    Event4Backdrop8001B6C4,
    Event4BackdropBoxFillLocal8001B6C4,
    Event4BackdropGsSortBoxFill8003EE84,
    Event4MoveImage8001B120,
    Event4MoveImageSlotSource8004019C,
    Event4MoveImageRect8001B120,
    Event4MoveImageDispatch80044E2C,
    Event4FrameSubmitHalGap,
    GatePracticeSelfLoopSource8002776C,
    PracticeSelfLoop,
    PracticeReload80015590,
    CardDrawIdOnly,
    ReturnResult,
    Gap,
};

struct EventTableSpec {
    int32_t eventId = 0;
    uint32_t tableAddress = 0;
    uint32_t init = 0;
    uint32_t handle = 0;
    uint32_t tick = 0;
    uint32_t timeout = 0;
    uint32_t defaultCtx = 0;
    bool ctxOverriddenByArg = false;
    EventFrameOwner owner = EventFrameOwner::Unknown;
    const char* name = nullptr;
    const char* use = nullptr;
};

struct DrawRouteSpec {
    int32_t eventId = 0;
    uint32_t callee = 0;
    int32_t typeArg = -1;
    int32_t workSlot = -1;
    EventFrameOwner owner = EventFrameOwner::Unknown;
    bool drawOnly = false;
    const char* name = nullptr;
    const char* ownerRule = nullptr;
};

struct EventFrameAction {
    EventFrameActionKind kind = EventFrameActionKind::None;
    uint32_t psxFunction = 0;
    int32_t eventId = 0;
    uint32_t tableAddress = 0;
    uint32_t ctxAddress = 0;
    uint32_t argAddress = 0;
    uint32_t auxAddress = 0;
    int32_t args[4]{};
    bool conditional = false;
};

struct EventFramePlan {
    const char* name = nullptr;
    bool runtimeCutoverAllowed = false;
    bool blockedByP0Gap = true;
    EventFrameOwner owner = EventFrameOwner::Unknown;
    int32_t eventId = 0;
    EventFrameAction actions[64]{};
    uint32_t count = 0;
    bool truncated = false;
};

// Event4 is not a second dispatcher state machine.  This is only the body
// transaction selected by 8001E750 after its common work-list preamble.
enum class Event4DrawBodyStatus8001E750 : uint8_t {
    SourceUnknown = 0,
    InvalidPromptChoice,
    PromptOwnerLimit,
    PreambleSourceLimit,
    WorkListBindingLimit,
    MoveImageSourceLimit,
    PromptReady,
    BackdropReady,
    MoveImageReady,
};

// 8001E750 performs this common graph preamble before selecting the Event4
// body state.  The callee internals remain an explicit input boundary until
// their formal owners are closed; no dynamic replay values are substituted.
enum class Event4DrawPreambleStatus8001E750 : uint8_t {
    SourceUnknown = 0,
    DrawBufferSourceLimit,
    PacketAllocatorSourceLimit,
    WorkListBindingLimit,
    Ready,
};

struct Event4DrawPreambleInput8001E750 {
    bool drawBufferSlotKnown = false;
    uint32_t drawBufferSlot = 0u;
    bool packetAllocatorValueKnown = false;
    uint32_t packetAllocatorValue = 0u;
    bool workListAddressKnown = false;
    uint32_t workListAddress = 0u;
};

struct Event4DrawPreambleTransaction8001E750 {
    Event4DrawPreambleStatus8001E750 status =
        Event4DrawPreambleStatus8001E750::SourceUnknown;
    bool accepted = false;
    bool sourceKnown = false;
    uint32_t drawBufferSlot = 0u;
    uint32_t drawBufferSourceFunction = kFn8004019C;
    uint32_t packetAllocatorFunction = kFn80040F90;
    uint32_t packetAllocatorValue = 0u;
    uint32_t clearWorkListFunction = kFn80040CC8;
    int32_t clearX = 0;
    int32_t clearY = 0;
    uint32_t workListAddress = 0u;
    bool exactPsxHalParity = false;
};

struct Event4DrawBodyInput8001E750 {
    bool wrapperStateKnown = false;
    uint32_t wrapperState = 0u;
    PrSS0Event4PromptRenderDirect::Event4PromptInput800203D4 prompt{};
    bool workListSlotKnown = false;
    uint32_t workListSlot = 0u;
    bool workListAddressKnown = false;
    uint32_t workListAddress = 0u;
    bool moveImageSlotKnown = false;
    uint32_t moveImageSlot = 0u;
    Event4DrawPreambleInput8001E750 preamble{};
};

struct Event4MoveImageTransaction80044E2C {
    bool accepted = false;
    bool sourceKnown = false;
    uint32_t drawBufferSlot = 0u;
    int16_t rectX = 0;
    int16_t rectY = 0;
    int16_t rectWidth = 0;
    int16_t rectHeight = 0;
    uint16_t destX = 0u;
    uint16_t destY = 0u;
    uint32_t packetAddress = 0x8005D7DCu;
    uint32_t rectGlobalAddress = 0x8005D7E4u;
    uint32_t destGlobalAddress = 0x8005D7E8u;
    uint32_t sizeGlobalAddress = 0x8005D7ECu;
    uint32_t rectGlobalValue = 0u;
    uint32_t destGlobalValue = 0u;
    uint32_t sizeGlobalValue = 0u;
    uint32_t dispatchFunction = kFn80044E2C;
    uint32_t dmaCallbackFunction = 0x80046840u;
    uint32_t dispatchWidth = 20u;
    uint32_t dispatchMode = 0u;
    bool exactPsxHalParity = false;
};

struct Event4DrawBodyTransaction8001E750 {
    Event4DrawBodyStatus8001E750 status =
        Event4DrawBodyStatus8001E750::SourceUnknown;
    bool accepted = false;
    uint32_t wrapperStateBefore = 0u;
    uint32_t wrapperStateAfter = 0u;
    Event4DrawPreambleTransaction8001E750 preamble{};
    bool promptDraw = false;
    int32_t promptChoice = -1;
    PrSS0Event4PromptRenderDirect::Event4PromptDrawList800203D4
        promptDrawList{};
    bool backdropFill = false;
    int32_t backdropX = kEvent4BackdropBoxFillX8001B6C4;
    int32_t backdropY = kEvent4BackdropBoxFillY8001B6C4;
    int32_t backdropWidth = kEvent4BackdropBoxFillW8001B6C4;
    int32_t backdropHeight = kEvent4BackdropBoxFillH8001B6C4;
    uint32_t backdropAttr = kEvent4BackdropBoxFillAttr8001B6C4;
    int32_t backdropPriority = kEvent4BackdropBoxFillPriority8001B6C4;
    uint32_t workListSlot = 0u;
    uint32_t workListAddress = 0u;
    bool moveImage = false;
    uint32_t moveImageMode = 0u;
    Event4MoveImageTransaction80044E2C moveImageTransaction{};
    uint32_t moveImageSourceFunction = kFn8004019C;
    int32_t moveImageWidth = kEvent4MoveImageW8001B120;
    int32_t moveImageHeight = kEvent4MoveImageH8001B120;
    uint32_t moveImageDispatchFunction = kFn80044E2C;
    bool exactPsxHalParity = false;
};

enum class Event4ModalInputStatus80025F0C : uint8_t {
    SourceUnknown = 0,
    IgnoredInput,
    AudioSinkMissing,
    AudioSinkFailed,
    CrossAccepted,
    CircleAccepted,
};

struct Event4ModalInitInput800267C8 {
    bool outputPointerKnown = false;
    uint32_t outputPointer = 0u;
};

struct Event4ModalInitTransaction800267C8 {
    bool accepted = false;
    bool outputWrite = false;
    uint32_t outputPointer = 0u;
    int32_t outputValue = -1;
    uint32_t cueFrameBefore = 0u;
    uint32_t cueFrameAfter = 0u;
    uint32_t remainingBefore = 0u;
    uint32_t remainingAfter = kEvent4ModalInitialRemaining;
    int32_t result = -1;
    bool exactPsxHalParity = false;
};

using Event4ModalSfxSink80025C8C = bool (*)(uint32_t inputMask,
                                           void* userData);

struct Event4ModalInputSinks80025F0C {
    Event4ModalSfxSink80025C8C playSfx80025C8C = nullptr;
    void* userData = nullptr;
};

struct Event4ModalInputTransaction80025F0C {
    Event4ModalInputStatus80025F0C status =
        Event4ModalInputStatus80025F0C::SourceUnknown;
    bool accepted = false;
    bool outputWrite = false;
    int32_t outputValue = -1;
    int32_t result = 0;
    uint32_t inputMask = 0u;
    uint32_t sfxInputMask = 0u;
    uint32_t sfxFunction = kFn80025C8C;
    bool sfxExecuted = false;
    bool exactPsxHalParity = false;
};

enum class Event4ModalTickStatus80025E6C : uint8_t {
    SourceUnknown = 0,
    Idle,
    CueBindingMissing,
    CueSinkMissing,
    CueSinkFailed,
    Advanced,
};

using Event4ModalCueSink80026EF8 = bool (*)(uint32_t cueAddress,
                                           void* userData);
using Event4ModalFlushSink80026ECC = bool (*)(void* userData);

struct Event4ModalTickSinks80025E6C {
    Event4ModalCueSink80026EF8 playCue80026EF8 = nullptr;
    Event4ModalFlushSink80026ECC flush80026ECC = nullptr;
    void* userData = nullptr;
};

struct Event4ModalTickInput80025E6C {
    bool remainingKnown = false;
    uint32_t remaining = 0u;
    bool cueFrameKnown = false;
    uint32_t cueFrame = 0u;
    bool cueGlobalAddressKnown = false;
    uint32_t cueGlobalAddress = 0u;
};

struct Event4ModalTickTransaction80025E6C {
    Event4ModalTickStatus80025E6C status =
        Event4ModalTickStatus80025E6C::SourceUnknown;
    bool accepted = false;
    uint32_t tickReturn = 0u;
    bool cueIssued = false;
    uint32_t cueAddress = 0u;
    uint32_t cueFrameBefore = 0u;
    uint32_t cueFrameAfter = 0u;
    uint32_t remainingBefore = 0u;
    uint32_t remainingAfter = 0u;
    bool cueExecuted = false;
    bool flushExecuted = false;
    bool exactPsxHalParity = false;
};

enum class Event4FrameClass80026B94 : uint8_t {
    OpenInputLoop = 0,
    ResultAction,
    ClosedInputTail,
};

enum class Event4FrameCommandKind80026B94 : uint8_t {
    Tick80025E6C = 0,
    DrawRoute8001E750,
    WaitFrame80035560,
    EndFrame8001EA00,
    TextFlush800436F0,
};

struct Event4FrameCommand80026B94 {
    Event4FrameCommandKind80026B94 kind =
        Event4FrameCommandKind80026B94::Tick80025E6C;
    uint32_t psxFunction = 0u;
    int32_t arg0 = 0;
    uint32_t arg1 = 0u;
    bool arg1Known = false;
};

struct Event4FrameTransactionBegin80026B94 {
    bool requestBound = false;
    uint32_t ctxAddress = 0u;
    uint32_t logicFrame = 0u;
    Event4FrameClass80026B94 frameClass =
        Event4FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
};

struct Event4FrameTransaction80026B94 {
    bool active = false;
    bool requestBound = false;
    bool tickCommitted = false;
    bool directDrawSubmitted = false;
    bool formalTailOrderBound = false;
    bool hostWaitExecuted = false;
    bool hostEndExecuted = false;
    bool hostTextFlushExecuted = false;
    bool hostFrameBoundaryExecuted = false;
    bool exactPsxHalParity = false;
    bool directDrawCompleteWithinLimits = false;
    bool logicAdvanceBlockedLogged = false;
    uint32_t logicFrame = 0u;
    uint32_t firstHostPresentationFrame = 0u;
    uint32_t ctxAddress = 0u;
    Event4FrameClass80026B94 frameClass =
        Event4FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
    Event4FrameCommand80026B94
        commands[kEvent4FrameCommandCount80026B94]{};
    uint32_t commandCount = 0u;
    uint32_t hostPresentationSubmitCount = 0u;
};

enum class DispatcherTailStepKind80026B94 : uint8_t {
    Inactive = 0,
    HoldFrame,
    ReleaseResult,
};

struct DispatcherTailState80026B94 {
    bool active = false;
    int32_t latchedResult = 0;
    int32_t framesRemaining = 0;
};

struct DispatcherTailStep80026B94 {
    DispatcherTailStepKind80026B94 kind =
        DispatcherTailStepKind80026B94::Inactive;
    bool inputClosed = false;
    int32_t releasedResult = 0;
    int32_t framesRemainingBefore = 0;
    int32_t framesRemainingAfter = 0;
};

static constexpr uint32_t kEvent6FrameCommandCount80026B94 = 7u;

// Event2 owns the stage-select frame through the same dispatcher loop, but
// its 80026170 tick, 80020568 page, and conditional StageClear text producer
// are distinct from Event3/Event6. Keep a separate transaction type so no
// other event can borrow its context or tail semantics.
static constexpr uint32_t kEvent2FrameCommandCount80026B94 = 8u;

enum class Event2FrameClass80026B94 : uint8_t {
    OpenInputLoop = 0,
    ResultAction,
    ClosedInputTail,
};

enum class Event2FrameCommandKind80026B94 : uint8_t {
    Tick80026170 = 0,
    DrawRoute8001E750,
    PrepareDrawWork8001D74C,
    DrawPage80020568,
    StageClearText80043A14,
    WaitFrame80035560,
    EndFrame8001EA00,
    TextFlush800436F0,
};

struct Event2FrameCommand80026B94 {
    Event2FrameCommandKind80026B94 kind =
        Event2FrameCommandKind80026B94::Tick80026170;
    uint32_t psxFunction = 0u;
    int32_t arg0 = 0;
    uint32_t arg1 = 0u;
    bool arg1Known = false;
    bool conditional = false;
    uint32_t conditionalGateAddress = 0u;
    bool conditionalGateValueKnown = false;
    uint16_t conditionalGateValue = 0u;
    bool conditionalTaken = false;
};

struct Event2FrameTransactionBegin80026B94 {
    bool requestBound = false;
    uint32_t ctxAddress = 0u;
    uint32_t logicFrame = 0u;
    Event2FrameClass80026B94 frameClass =
        Event2FrameClass80026B94::OpenInputLoop;
    bool word800916F6Known = false;
    uint16_t word800916F6 = 0u;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
};

struct Event2FrameTransaction80026B94 {
    bool active = false;
    bool requestBound = false;
    bool tickCommitted = false;
    bool directDrawSubmitted = false;
    // These flags describe the SS0 translated page only. They do not credit
    // the PSX wait/end/text HAL or the legacy Windows S0 shell.
    bool formalTailOrderBound = false;
    bool hostWaitExecuted = false;
    bool hostEndExecuted = false;
    bool hostTextFlushExecuted = false;
    bool hostFrameBoundaryExecuted = false;
    bool exactPsxHalParity = false;
    bool directDrawCompleteWithinLimits = false;
    bool logicAdvanceBlockedLogged = false;
    bool stageClearGateValueKnown800916F6 = false;
    uint16_t stageClearGateValue800916F6 = 0u;
    bool stageClearBranchTaken80026D70 = false;
    uint32_t logicFrame = 0u;
    uint32_t firstHostPresentationFrame = 0u;
    uint32_t ctxAddress = 0u;
    Event2FrameClass80026B94 frameClass =
        Event2FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
    Event2FrameCommand80026B94
        commands[kEvent2FrameCommandCount80026B94]{};
    uint32_t commandCount = 0u;
    uint32_t hostPresentationSubmitCount = 0u;
};

enum class Event6FrameClass80026B94 : uint8_t {
    OpenInputLoop = 0,
    ResultAction,
    ClosedInputTail,
};

enum class Event6FrameCommandKind80026B94 : uint8_t {
    Tick80025E48 = 0,
    DrawRoute8001E750,
    PrepareDrawWork8001D74C,
    DrawPage80021594,
    WaitFrame80035560,
    EndFrame8001EA00,
    TextFlush800436F0,
};

struct Event6FrameCommand80026B94 {
    Event6FrameCommandKind80026B94 kind =
        Event6FrameCommandKind80026B94::Tick80025E48;
    uint32_t psxFunction = 0u;
    int32_t arg0 = 0;
    uint32_t arg1 = 0u;
    bool arg1Known = false;
};

struct Event6FrameTransactionBegin80026B94 {
    bool requestBound = false;
    uint32_t ctxAddress = 0u;
    uint32_t logicFrame = 0u;
    Event6FrameClass80026B94 frameClass =
        Event6FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
};

struct Event6FrameTransaction80026B94 {
    bool active = false;
    bool requestBound = false;
    bool tickCommitted = false;
    bool directDrawSubmitted = false;
    // These flags describe the independent SS0 host projection only. They do
    // not claim byte-for-byte PSX HAL parity or use the old Windows S0 shell.
    bool formalTailOrderBound = false;
    bool hostWaitExecuted = false;
    bool hostEndExecuted = false;
    bool hostTextFlushExecuted = false;
    bool hostFrameBoundaryExecuted = false;
    bool exactPsxHalParity = false;
    bool directDrawCompleteWithinLimits = false;
    bool logicAdvanceBlockedLogged = false;
    uint32_t logicFrame = 0u;
    uint32_t firstHostPresentationFrame = 0u;
    uint32_t ctxAddress = 0u;
    Event6FrameClass80026B94 frameClass =
        Event6FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
    Event6FrameCommand80026B94
        commands[kEvent6FrameCommandCount80026B94]{};
    uint32_t commandCount = 0u;
    uint32_t hostPresentationSubmitCount = 0u;
};

// Event 6's 8001EA00(6) tail is a separate SS0 owner. The current-IDB
// decompile proves the flip, clear, and work-list-submit order, but does not
// prove the Win mapping of gp+872. Keep that mapping request-bound instead of
// borrowing the generic old-S0 EndFrame helper.
static constexpr uint32_t kEvent6EndFrameActionCount8001EA00 = 3u;

enum class Event6EndFrameActionKind8001EA00 : uint8_t {
    FlipGraph80040370 = 0,
    ClearColor80040420,
    SubmitWorkList80040CA4,
};

struct Event6EndFrameAction8001EA00 {
    Event6EndFrameActionKind8001EA00 kind =
        Event6EndFrameActionKind8001EA00::FlipGraph80040370;
    uint32_t psxFunction = 0u;
    uint32_t arg0 = 0u;
    uint32_t arg1 = 0u;
    uint32_t arg2 = 0u;
};

struct Event6EndFrameOwnerInput8001EA00 {
    bool requestBound = false;
    int32_t eventId = 0;
    bool graphSlotBeforeKnown = false;
    uint16_t graphSlotBefore = 0u;
    bool workListSlotKnown = false;
    uint32_t workListSlot = 0u;
    bool workListAddressKnown = false;
    uint32_t workListAddress = 0u;
    bool workListReady = false;
    // This is intentionally separate from the formal IDA facts. It may be
    // set only after the translated graph owner proves the host slot mapping.
    bool graphModelPreflightKnown = false;
    bool graphModelWorkListSlotMatches = false;
};

struct Event6EndFrameOwnerPlan8001EA00 {
    bool formalOrderBound = false;
    bool graphModelReady = false;
    bool formalOnly = true;
    bool graphModelApplied = false;
    bool hostClearExecuted = false;
    bool hostSubmitExecuted = false;
    bool exactPsxHalParity = false;
    uint16_t graphSlotBefore = 0u;
    uint16_t graphSlotAfter = 0u;
    uint32_t workListSlot = 0u;
    uint32_t workListAddress = 0u;
    uint8_t clearR = kEvent6EndFrameClearR80040420;
    uint8_t clearG = kEvent6EndFrameClearG80040420;
    uint8_t clearB = kEvent6EndFrameClearB80040420;
    uint32_t actionCount = 0u;
    Event6EndFrameAction8001EA00
        actions[kEvent6EndFrameActionCount8001EA00]{};
};

// Event3 has the same dispatcher-frame ownership rule as Event6, but its
// state and draw context are different. Keep a separate transaction type so
// the main-directory route cannot accidentally borrow HI-SCORE semantics.
static constexpr uint32_t kEvent3FrameCommandCount80026B94 = 7u;

enum class Event3FrameClass80026B94 : uint8_t {
    OpenInputLoop = 0,
    ResultAction,
    ClosedInputTail,
};

enum class Event3FrameCommandKind80026B94 : uint8_t {
    Tick80026720 = 0,
    DrawRoute8001E750,
    PrepareDrawWork8001D74C,
    DrawPage80021E60,
    WaitFrame80035560,
    EndFrame8001EA00,
    TextFlush800436F0,
};

struct Event3FrameCommand80026B94 {
    Event3FrameCommandKind80026B94 kind =
        Event3FrameCommandKind80026B94::Tick80026720;
    uint32_t psxFunction = 0u;
    int32_t arg0 = 0;
    uint32_t arg1 = 0u;
    bool arg1Known = false;
};

struct Event3FrameTransactionBegin80026B94 {
    bool requestBound = false;
    uint32_t ctxAddress = 0u;
    uint32_t logicFrame = 0u;
    Event3FrameClass80026B94 frameClass =
        Event3FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
};

struct Event3FrameTransaction80026B94 {
    bool active = false;
    bool requestBound = false;
    bool tickCommitted = false;
    bool directDrawSubmitted = false;
    // These flags describe the translated SS0 projection only. They must not
    // be used to credit PSX HAL parity or the old Windows S0 shell.
    bool formalTailOrderBound = false;
    bool hostWaitExecuted = false;
    bool hostEndExecuted = false;
    bool hostTextFlushExecuted = false;
    bool hostFrameBoundaryExecuted = false;
    bool exactPsxHalParity = false;
    bool directDrawCompleteWithinLimits = false;
    bool logicAdvanceBlockedLogged = false;
    uint32_t logicFrame = 0u;
    uint32_t firstHostPresentationFrame = 0u;
    uint32_t ctxAddress = 0u;
    Event3FrameClass80026B94 frameClass =
        Event3FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
    Event3FrameCommand80026B94
        commands[kEvent3FrameCommandCount80026B94]{};
    uint32_t commandCount = 0u;
    uint32_t hostPresentationSubmitCount = 0u;
};

// Event17 owns the Options/language page through the same dispatcher entry,
// but its table, context, draw callee, and work slot are distinct. Keep this
// transaction independent from Event2/Event3/Event6 so no shell or other
// translated page can borrow its semantics or tail state.
static constexpr uint32_t kEvent17FrameCommandCount80026B94 = 7u;

enum class Event17FrameClass80026B94 : uint8_t {
    OpenInputLoop = 0,
    ResultAction,
    ClosedInputTail,
};

enum class Event17FrameCommandKind80026B94 : uint8_t {
    Tick80026B54 = 0,
    DrawRoute8001E750,
    PrepareDrawWork8001D74C,
    DrawPage80021910,
    WaitFrame80035560,
    EndFrame8001EA00,
    TextFlush800436F0,
};

struct Event17FrameCommand80026B94 {
    Event17FrameCommandKind80026B94 kind =
        Event17FrameCommandKind80026B94::Tick80026B54;
    uint32_t psxFunction = 0u;
    int32_t arg0 = 0;
    uint32_t arg1 = 0u;
    bool arg1Known = false;
};

struct Event17FrameTransactionBegin80026B94 {
    bool requestBound = false;
    uint32_t ctxAddress = 0u;
    uint32_t logicFrame = 0u;
    Event17FrameClass80026B94 frameClass =
        Event17FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
};

struct Event17FrameTransaction80026B94 {
    bool active = false;
    bool requestBound = false;
    bool tickCommitted = false;
    bool directDrawSubmitted = false;
    // These flags describe only the translated SS0 page projection. They do
    // not credit the PSX HAL or the existing Windows S0 shell.
    bool formalTailOrderBound = false;
    bool hostWaitExecuted = false;
    bool hostEndExecuted = false;
    bool hostTextFlushExecuted = false;
    bool hostFrameBoundaryExecuted = false;
    bool exactPsxHalParity = false;
    bool directDrawCompleteWithinLimits = false;
    bool logicAdvanceBlockedLogged = false;
    uint32_t logicFrame = 0u;
    uint32_t firstHostPresentationFrame = 0u;
    uint32_t ctxAddress = 0u;
    Event17FrameClass80026B94 frameClass =
        Event17FrameClass80026B94::OpenInputLoop;
    bool inputClosed = false;
    int32_t tailFramesRemainingBefore = -1;
    int32_t tailFramesRemainingAfter = -1;
    Event17FrameCommand80026B94
        commands[kEvent17FrameCommandCount80026B94]{};
    uint32_t commandCount = 0u;
    uint32_t hostPresentationSubmitCount = 0u;
};

// Event17's 8001EA00(17) owner is deliberately separate from Event6's
// same-address plan. Current IDA proves the three-call order, while the
// gp+872-to-Win graph lane remains a request-bound preflight input.
enum class Event17EndFrameActionKind8001EA00 : uint8_t {
    FlipGraph80040370 = 0,
    ClearColor80040420,
    SubmitWorkList80040CA4,
};

struct Event17EndFrameAction8001EA00 {
    Event17EndFrameActionKind8001EA00 kind =
        Event17EndFrameActionKind8001EA00::FlipGraph80040370;
    uint32_t psxFunction = 0u;
    uint32_t arg0 = 0u;
    uint32_t arg1 = 0u;
    uint32_t arg2 = 0u;
    bool arg0Known = false;
    bool arg1Known = false;
    bool arg2Known = false;
};

struct Event17EndFrameOwnerInput8001EA00 {
    bool requestBound = false;
    int32_t eventId = 0;
    bool graphSlotBeforeKnown = false;
    uint16_t graphSlotBefore = 0u;
    bool workListSlotKnown = false;
    uint32_t workListSlot = 0u;
    bool workListAddressKnown = false;
    uint32_t workListAddress = 0u;
    bool workListReady = false;
    // These fields are host-translation preflight only. They do not alter the
    // formal IDA action order and are never sourced from the old S0 shell.
    bool graphModelPreflightKnown = false;
    bool graphModelWorkListSlotMatches = false;
};

struct Event17EndFrameOwnerPlan8001EA00 {
    bool formalOrderBound = false;
    bool graphModelReady = false;
    bool formalOnly = true;
    bool graphModelApplied = false;
    bool hostClearExecuted = false;
    bool hostSubmitExecuted = false;
    bool exactPsxHalParity = false;
    uint16_t graphSlotBefore = 0u;
    uint16_t graphSlotAfter = 0u;
    uint32_t workListSlot = 0u;
    uint32_t workListAddress = 0u;
    uint8_t clearR = kEvent17EndFrameClearR80040420;
    uint8_t clearG = kEvent17EndFrameClearG80040420;
    uint8_t clearB = kEvent17EndFrameClearB80040420;
    uint32_t actionCount = 0u;
    Event17EndFrameAction8001EA00
        actions[kEvent17EndFrameActionCount8001EA00]{};
};

enum class DispatcherPreLoopPadReleaseStepKind80026B94 : uint8_t {
    WaitingForRelease = 0,
    ReleasedEnterLoop,
    LoopAlreadyActive,
};

struct DispatcherPreLoopPadReleaseState80026B94 {
    bool waitingForRelease = true;
};

struct DispatcherPreLoopPadReleaseStep80026B94 {
    DispatcherPreLoopPadReleaseStepKind80026B94 kind =
        DispatcherPreLoopPadReleaseStepKind80026B94::WaitingForRelease;
    bool inputClosed = true;
};

struct DispatcherPadChangeState80026744 {
    uint32_t previousPadMask80035510 = 0u;
};

struct DispatcherPadChangeStep80026744 {
    uint32_t currentPadMask80035510 = 0u;
    uint32_t previousPadMask80035510 = 0u;
    uint32_t changedPadMask80026744 = 0u;
    bool changed = false;
};

enum class Event17InitialInputTimeoutStepKind80026B94 : uint8_t {
    Inactive = 0,
    CountedNoInputFrame,
    DisabledByInput,
    TimeoutResult,
};

struct Event17InitialInputTimeoutState80026B94 {
    bool initialInputPending = true;
    bool resultIssued = false;
    int32_t framesRemaining = kEvent17InitialInputTimeoutFrames80026B94;
};

struct Event17InitialInputTimeoutStep80026B94 {
    Event17InitialInputTimeoutStepKind80026B94 kind =
        Event17InitialInputTimeoutStepKind80026B94::Inactive;
    int32_t framesRemainingBefore = 0;
    int32_t framesRemainingAfter = 0;
    int32_t result = 0;
};

bool RuntimeCutoverAllowed();

Event4DrawPreambleTransaction8001E750
BuildEvent4DrawPreambleTransaction8001E750(
    const Event4DrawPreambleInput8001E750& input);

Event4DrawBodyTransaction8001E750 BuildEvent4DrawBodyTransaction8001E750(
    const Event4DrawBodyInput8001E750& input);

Event4ModalInputTransaction80025F0C ExecuteEvent4ModalInputSoftware80025F0C(
    uint32_t inputMask,
    const Event4ModalInputSinks80025F0C& sinks);

Event4ModalInitTransaction800267C8 BuildEvent4ModalInitSoftware800267C8(
    const Event4ModalInitInput800267C8& input);

Event4ModalTickTransaction80025E6C ExecuteEvent4ModalTickSoftware80025E6C(
    const Event4ModalTickInput80025E6C& input,
    const Event4ModalTickSinks80025E6C& sinks);

struct DispatcherTailState80026B94;
struct DispatcherTailStep80026B94;

void ResetEvent4FrameTransaction80026B94(
    Event4FrameTransaction80026B94& state);
bool CanAdvanceEvent4FrameLogic80026B94(
    const Event4FrameTransaction80026B94& state);
bool TryStepEvent4DispatcherTail80026B94(
    const Event4FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep);
bool BeginEvent4FrameTransaction80026B94(
    Event4FrameTransaction80026B94& state,
    const Event4FrameTransactionBegin80026B94& begin);
bool CommitEvent4FrameTick80025E6C(
    Event4FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t ctxAddress);
bool CanSubmitEvent4FrameDraw8001E750(
    const Event4FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent4FrameDrawAndBindFormalTail80026B94(
    Event4FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent4FrameHostBoundary80026B94(
    Event4FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted);

void ResetDispatcherTail80026B94(DispatcherTailState80026B94& state);
bool ArmDispatcherTail80026B94(DispatcherTailState80026B94& state,
                               int32_t result);
DispatcherTailStep80026B94 StepDispatcherTail80026B94(
    DispatcherTailState80026B94& state);
void ResetEvent6FrameTransaction80026B94(
    Event6FrameTransaction80026B94& state);
bool CanAdvanceEvent6FrameLogic80026B94(
    const Event6FrameTransaction80026B94& state);
bool TryStepEvent6DispatcherTail80026B94(
    const Event6FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep);
bool BeginEvent6FrameTransaction80026B94(
    Event6FrameTransaction80026B94& state,
    const Event6FrameTransactionBegin80026B94& begin);
bool CommitEvent6FrameTick80025E48(
    Event6FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t tableAddress,
    uint32_t ctxAddress);
bool CanSubmitEvent6FrameDraw8001E750(
    const Event6FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent6FrameDrawAndBindFormalTail80026B94(
    Event6FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent6FrameHostBoundary80026B94(
    Event6FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted);
void ResetEvent6EndFrameOwnerPlan8001EA00(
    Event6EndFrameOwnerPlan8001EA00& plan);
bool BuildEvent6EndFrameOwnerPlan8001EA00(
    const Event6EndFrameOwnerInput8001EA00& input,
    Event6EndFrameOwnerPlan8001EA00& outPlan);
void ResetEvent2FrameTransaction80026B94(
    Event2FrameTransaction80026B94& state);
bool CanAdvanceEvent2FrameLogic80026B94(
    const Event2FrameTransaction80026B94& state);
bool TryStepEvent2DispatcherTail80026B94(
    const Event2FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep);
bool BeginEvent2FrameTransaction80026B94(
    Event2FrameTransaction80026B94& state,
    const Event2FrameTransactionBegin80026B94& begin);
bool CommitEvent2FrameTick80026170(
    Event2FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t tableAddress,
    uint32_t ctxAddress);
bool CanSubmitEvent2FrameDraw8001E750(
    const Event2FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent2FrameDrawAndBindFormalTail80026B94(
    Event2FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent2FrameHostBoundary80026B94(
    Event2FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted);
void ResetEvent3FrameTransaction80026B94(
    Event3FrameTransaction80026B94& state);
bool CanAdvanceEvent3FrameLogic80026B94(
    const Event3FrameTransaction80026B94& state);
bool TryStepEvent3DispatcherTail80026B94(
    const Event3FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep);
bool BeginEvent3FrameTransaction80026B94(
    Event3FrameTransaction80026B94& state,
    const Event3FrameTransactionBegin80026B94& begin);
bool CommitEvent3FrameTick80026720(
    Event3FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t tableAddress,
    uint32_t ctxAddress);
bool CanSubmitEvent3FrameDraw8001E750(
    const Event3FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent3FrameDrawAndBindFormalTail80026B94(
    Event3FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent3FrameHostBoundary80026B94(
    Event3FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted);
void ResetEvent17FrameTransaction80026B94(
    Event17FrameTransaction80026B94& state);
bool CanAdvanceEvent17FrameLogic80026B94(
    const Event17FrameTransaction80026B94& state);
bool TryStepEvent17DispatcherTail80026B94(
    const Event17FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep);
bool BeginEvent17FrameTransaction80026B94(
    Event17FrameTransaction80026B94& state,
    const Event17FrameTransactionBegin80026B94& begin);
bool CommitEvent17FrameTick80026B54(
    Event17FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t tableAddress,
    uint32_t ctxAddress);
bool CanSubmitEvent17FrameDraw8001E750(
    const Event17FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent17FrameDrawAndBindFormalTail80026B94(
    Event17FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation);
bool CommitEvent17FrameHostBoundary80026B94(
    Event17FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted);
void ResetEvent17EndFrameOwnerPlan8001EA00(
    Event17EndFrameOwnerPlan8001EA00& plan);
bool BuildEvent17EndFrameOwnerPlan8001EA00(
    const Event17EndFrameOwnerInput8001EA00& input,
    Event17EndFrameOwnerPlan8001EA00& outPlan);
void ResetDispatcherPreLoopPadRelease80026B94(
    DispatcherPreLoopPadReleaseState80026B94& state);
DispatcherPreLoopPadReleaseStep80026B94
StepDispatcherPreLoopPadRelease80026B94(
    DispatcherPreLoopPadReleaseState80026B94& state,
    uint32_t currentPadMask80035510);
void ResetDispatcherPadChange80026744(
    DispatcherPadChangeState80026744& state);
DispatcherPadChangeStep80026744 StepDispatcherPadChange80026744(
    DispatcherPadChangeState80026744& state,
    uint32_t currentPadMask80035510);
void ResetEvent17InitialInputTimeout80026B94(
    Event17InitialInputTimeoutState80026B94& state);
Event17InitialInputTimeoutStep80026B94
StepEvent17InitialInputTimeout80026B94(
    Event17InitialInputTimeoutState80026B94& state,
    uint32_t padMask);

uint32_t KnownEventTableSpecCount();
const EventTableSpec& KnownEventTableSpecAt(uint32_t index);
const EventTableSpec* FindEventTableSpec(int32_t eventId);

uint32_t KnownDrawRouteSpecCount();
const DrawRouteSpec& KnownDrawRouteSpecAt(uint32_t index);
const DrawRouteSpec* FindDrawRouteSpec(int32_t eventId);

EventFramePlan BuildDispatcher80026B94Plan(int32_t eventId,
                                            bool argKnown = false,
                                            uint32_t argAddress = 0);
EventFramePlan BuildDrawRoute8001E750Plan(int32_t eventId);
EventFramePlan BuildPracticeSelfLoop8002776CPlan(
    uint32_t menuCtx = kScene0WorkAddress,
    int32_t prevScene = 0);
EventFramePlan BuildCardDrawIds80020F94Plan();

const char* EventFrameOwnerName(EventFrameOwner owner);
const char* EventFrameActionKindName(EventFrameActionKind kind);

} // namespace PrSS0EventFrameLoopDirect
