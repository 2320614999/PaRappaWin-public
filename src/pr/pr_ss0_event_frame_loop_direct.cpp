#include "pr_ss0_event_frame_loop_direct.h"

namespace PrSS0EventFrameLoopDirect {
namespace {

bool HasExactEvent6FrameCommandOrder80026B94(
    const Event6FrameTransaction80026B94& state)
{
    return state.commandCount == kEvent6FrameCommandCount80026B94 &&
           state.commands[0].kind ==
               Event6FrameCommandKind80026B94::Tick80025E48 &&
           state.commands[0].psxFunction == kFn80025E48 &&
           static_cast<uint32_t>(state.commands[0].arg0) ==
               kEventTable6_80054578 &&
           state.commands[0].arg1Known &&
           state.commands[0].arg1 == kCtxEvent6HiScoreTable80049278 &&
           state.commands[1].kind ==
               Event6FrameCommandKind80026B94::DrawRoute8001E750 &&
           state.commands[1].psxFunction == kFn8001E750 &&
           state.commands[1].arg0 == 6 &&
           state.commands[1].arg1Known &&
           state.commands[1].arg1 == kCtxEvent6HiScoreTable80049278 &&
           state.commands[2].kind ==
               Event6FrameCommandKind80026B94::PrepareDrawWork8001D74C &&
           state.commands[2].psxFunction == kFn8001D74C &&
           state.commands[2].arg0 == 3 &&
           !state.commands[2].arg1Known &&
           state.commands[3].kind ==
               Event6FrameCommandKind80026B94::DrawPage80021594 &&
           state.commands[3].psxFunction == kFn80021594 &&
           static_cast<uint32_t>(state.commands[3].arg0) ==
               kCtxEvent6HiScoreTable80049278 &&
           state.commands[4].kind ==
               Event6FrameCommandKind80026B94::WaitFrame80035560 &&
           state.commands[4].psxFunction == kFn80035560 &&
           state.commands[4].arg0 == 0 &&
           state.commands[5].kind ==
               Event6FrameCommandKind80026B94::EndFrame8001EA00 &&
           state.commands[5].psxFunction == kFn8001EA00 &&
           state.commands[5].arg0 == 6 &&
           state.commands[6].kind ==
               Event6FrameCommandKind80026B94::TextFlush800436F0 &&
           state.commands[6].psxFunction == kFn800436F0 &&
           state.commands[6].arg0 == -1;
}

constexpr EventTableSpec kEventTables[] = {
    {2,
     kEventTable2_8005453C,
     kFn800267F8,
     kFn80025F6C,
     kFn80026170,
     0,
     kCtxEvent2_80087B78,
     false,
     EventFrameOwner::DispatcherLoop,
     "event=2 stage select",
     "main menu stage-select result; byref out scene accepted only on result 1"},
    {3,
     kEventTable3_80054550,
     kFn80026794,
     kFn800264AC,
     kFn80026720,
     0,
     kCtxEvent3_800544F8,
     false,
     EventFrameOwner::DispatcherLoop,
     "event=3 main directory",
     "blocking main directory after title MENU; returns result 1/2/3/4/6/7/8"},
    {4,
     kEventTable4_80054564,
     kFn800267C8,
     kFn80025F0C,
     kFn80025E6C,
     0,
     kCtxEvent4_8006ED74,
     false,
     EventFrameOwner::ModalPrompt,
     "event=4 modal prompt",
     "blocking confirm/cancel prompt; Cross result 1, Circle result 2"},
    {6,
     kEventTable6_80054578,
     kFn800267E4,
     kFn80025E0C,
     kFn80025E48,
     0,
     kCtxEvent6Default_8005424C,
     true,
     EventFrameOwner::DispatcherLoop,
     "event=6 confirm token",
     "event=3 hi-score branch passes arg as ctx and waits for Cross"},
    {10,
     kEventTable10_8005458C,
     kFn800267F0,
     0,
     0,
     0,
     0,
     false,
     EventFrameOwner::Placeholder,
     "event=10 placeholder",
     "handle is null and timeout is zero; not a runnable dispatcher case"},
    {17,
     kEventTable17_800545A0,
     kFn800268E4,
     kFn80026910,
     kFn80026B54,
     0x4B0u,
     kCtxEvent17_8005451C,
     false,
     EventFrameOwner::DispatcherLoop,
     "event=17 options/language",
     "options/language loop; writes language/subtitle through word_800916D0-relative globals"},
};

constexpr DrawRouteSpec kDrawRoutes[] = {
    {2,
     kFn80020568,
     -1,
     3,
     EventFrameOwner::DispatcherLoop,
     false,
     "event=2 stage select draw",
     "owned by 80026B94 event=2, not a standalone menu state"},
    {3,
     kFn80021E60,
     -1,
     3,
     EventFrameOwner::DispatcherLoop,
     false,
     "event=3 main directory draw",
     "owned by 80026B94 event=3 main directory loop"},
    {4,
     kFn800203D4,
     -1,
     -1,
     EventFrameOwner::ModalPrompt,
     false,
     "event=4 prompt draw",
     "prompt draw branch under 8001E750 event=4"},
    {5,
     kFn80020BE4,
     -1,
     -1,
     EventFrameOwner::DrawOnly,
     true,
     "event=5 card info draw",
     "draw route only; loop owner remains lower card state machine"},
    {6,
     kFn80021594,
     -1,
     -1,
     EventFrameOwner::DispatcherLoop,
     false,
     "event=6 confirm token draw",
     "owned by 80026B94 event=6 with arg-provided ctx"},
    {7,
     kFn80020F94,
     -1,
     3,
     EventFrameOwner::DrawOnly,
     true,
     "event=7 save card grid draw",
     "draw id only; real loop owner is 80018FB0, not 80026B94"},
    {8,
     kFn80020F94,
     -1,
     3,
     EventFrameOwner::DrawOnly,
     true,
     "event=8 load card grid draw",
     "draw id only; real loop owner is 80018FB0, not 80026B94"},
    {9,
     kFn80020F94,
     -1,
     3,
     EventFrameOwner::DrawOnly,
     true,
     "event=9 replay card grid draw",
     "draw id only; real loop owner is 80018FB0, not 80026B94"},
    {11,
     kFn80022CBC,
     4,
     -1,
     EventFrameOwner::DrawOnly,
     true,
     "event=11 card prompt draw type 4",
     "draw route from card/save loop; not a dispatcher table id"},
    {12,
     kFn80022CBC,
     1,
     -1,
     EventFrameOwner::DrawOnly,
     true,
     "event=12 card prompt draw type 1",
     "draw route from card/save loop; not a dispatcher table id"},
    {13,
     kFn80022CBC,
     2,
     -1,
     EventFrameOwner::DrawOnly,
     true,
     "event=13 card prompt draw type 2",
     "draw route from card/save loop; not a dispatcher table id"},
    {14,
     kFn80022CBC,
     3,
     -1,
     EventFrameOwner::DrawOnly,
     true,
     "event=14 card prompt draw type 3",
     "draw route from card/save loop; not a dispatcher table id"},
    {15,
     kFn80022CBC,
     5,
     -1,
     EventFrameOwner::DrawOnly,
     true,
     "event=15 card prompt draw type 5",
     "draw route from card/save loop; not a dispatcher table id"},
    {16,
     kFn80023618,
     -1,
     3,
     EventFrameOwner::PracticeSelfLoop,
     true,
     "event=16 practice draw",
     "draw id used by 8002776C self-loop, not by 80026B94"},
    {17,
     kFn80021910,
     -1,
     4,
     EventFrameOwner::DispatcherLoop,
     false,
     "event=17 options/language draw",
     "owned by 80026B94 event=17 options/language loop"},
    {18,
     kFn80022CBC,
     6,
     -1,
     EventFrameOwner::DrawOnly,
     true,
     "event=18 card prompt draw type 6",
     "draw route from card/save loop; not a dispatcher table id"},
    {19,
     kFn80022CBC,
     7,
     -1,
     EventFrameOwner::DrawOnly,
     true,
     "event=19 card prompt draw type 7",
     "draw route from card/save loop; not a dispatcher table id"},
};

bool Append(EventFramePlan& plan, const EventFrameAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

void AppendAction(EventFramePlan& plan,
                  EventFrameActionKind kind,
                  uint32_t psxFunction,
                  int32_t eventId = 0,
                  uint32_t tableAddress = 0,
                  uint32_t ctxAddress = 0,
                  uint32_t argAddress = 0,
                  uint32_t auxAddress = 0,
                  int32_t arg0 = 0,
                  int32_t arg1 = 0,
                  int32_t arg2 = 0,
                  int32_t arg3 = 0,
                  bool conditional = false)
{
    EventFrameAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.eventId = eventId;
    action.tableAddress = tableAddress;
    action.ctxAddress = ctxAddress;
    action.argAddress = argAddress;
    action.auxAddress = auxAddress;
    action.args[0] = arg0;
    action.args[1] = arg1;
    action.args[2] = arg2;
    action.args[3] = arg3;
    action.conditional = conditional;
    (void)Append(plan, action);
}

EventFramePlan MakePlan(const char* name,
                         EventFrameOwner owner,
                         int32_t eventId)
{
    EventFramePlan plan{};
    plan.name = name;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    plan.owner = owner;
    plan.eventId = eventId;
    return plan;
}

const char* DispatcherPlanName(int32_t eventId)
{
    switch (eventId) {
    case 2:
        return "Dispatcher80026B94_Event2StageSelect";
    case 3:
        return "Dispatcher80026B94_Event3MainDirectory";
    case 4:
        return "Dispatcher80026B94_Event4ModalPrompt";
    case 6:
        return "Dispatcher80026B94_Event6ConfirmToken";
    case 10:
        return "Dispatcher80026B94_Event10Placeholder";
    case 17:
        return "Dispatcher80026B94_Event17OptionsLanguage";
    }
    return "Dispatcher80026B94_UnknownEvent";
}

uint32_t ResolveCtx(const EventTableSpec& spec,
                    bool argKnown,
                    uint32_t argAddress)
{
    if (spec.ctxOverriddenByArg) {
        return argKnown ? argAddress : 0;
    }
    return spec.defaultCtx;
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

Event4DrawPreambleTransaction8001E750
BuildEvent4DrawPreambleTransaction8001E750(
    const Event4DrawPreambleInput8001E750& input)
{
    Event4DrawPreambleTransaction8001E750 out{};
    if (!input.drawBufferSlotKnown || !input.packetAllocatorValueKnown ||
        !input.workListAddressKnown) {
        return out;
    }
    out.sourceKnown = true;
    out.drawBufferSlot = input.drawBufferSlot;
    out.packetAllocatorValue = input.packetAllocatorValue;
    out.workListAddress = input.workListAddress;
    if (input.drawBufferSlot > 1u) {
        out.status = Event4DrawPreambleStatus8001E750::DrawBufferSourceLimit;
        return out;
    }
    if (input.packetAllocatorValue == 0u) {
        out.status =
            Event4DrawPreambleStatus8001E750::PacketAllocatorSourceLimit;
        return out;
    }
    if (input.workListAddress !=
        kEvent4DrawWorkListBase80087288 +
            input.drawBufferSlot * kEvent4DrawWorkListStride8001E750) {
        out.status = Event4DrawPreambleStatus8001E750::WorkListBindingLimit;
        return out;
    }
    out.status = Event4DrawPreambleStatus8001E750::Ready;
    out.accepted = true;
    return out;
}

Event4DrawBodyTransaction8001E750 BuildEvent4DrawBodyTransaction8001E750(
    const Event4DrawBodyInput8001E750& input)
{
    Event4DrawBodyTransaction8001E750 out{};
    if (!input.wrapperStateKnown) {
        return out;
    }

    out.wrapperStateBefore = input.wrapperState;
    out.preamble =
        BuildEvent4DrawPreambleTransaction8001E750(input.preamble);
    if (!out.preamble.accepted) {
        out.status = Event4DrawBodyStatus8001E750::PreambleSourceLimit;
        return out;
    }
    switch (input.wrapperState) {
    case 0u:
        if (!input.prompt.requestBound || !input.prompt.choiceSourceKnown) {
            out.status = Event4DrawBodyStatus8001E750::InvalidPromptChoice;
            return out;
        }
        out.promptDrawList =
            PrSS0Event4PromptRenderDirect::
                BuildEvent4PromptDrawList800203D4(input.prompt);
        if (!out.promptDrawList.accepted ||
            !out.promptDrawList.complete) {
            out.status = Event4DrawBodyStatus8001E750::PromptOwnerLimit;
            return out;
        }
        out.promptDraw = true;
        out.promptChoice = input.prompt.ctx0;
        out.wrapperStateAfter = 0u;
        out.status = Event4DrawBodyStatus8001E750::PromptReady;
        out.accepted = true;
        return out;

    case 1u:
        if (!input.workListSlotKnown || !input.workListAddressKnown ||
            input.workListSlot > 1u ||
            input.workListAddress !=
                kEvent4DrawWorkListBase80087288 +
                    input.workListSlot * kEvent4DrawWorkListStride8001E750 ||
            input.workListSlot != out.preamble.drawBufferSlot ||
            input.workListAddress != out.preamble.workListAddress) {
            out.status = Event4DrawBodyStatus8001E750::WorkListBindingLimit;
            return out;
        }
        out.backdropFill = true;
        out.workListSlot = input.workListSlot;
        out.workListAddress = input.workListAddress;
        out.wrapperStateAfter = 2u;
        out.status = Event4DrawBodyStatus8001E750::BackdropReady;
        out.accepted = true;
        return out;

    default:
        if (!input.moveImageSlotKnown || input.moveImageSlot > 1u) {
            out.status = Event4DrawBodyStatus8001E750::MoveImageSourceLimit;
            return out;
        }
        out.moveImage = true;
        out.moveImageMode = 0u;
        out.moveImageTransaction.accepted = true;
        out.moveImageTransaction.sourceKnown = true;
        out.moveImageTransaction.drawBufferSlot = input.moveImageSlot;
        out.moveImageTransaction.rectWidth = 320;
        out.moveImageTransaction.rectHeight = 240;
        out.moveImageTransaction.rectY =
            input.moveImageSlot == 0u ? 0 : 240;
        out.moveImageTransaction.destY =
            input.moveImageSlot == 0u ? 240u : 0u;
        out.moveImageTransaction.rectGlobalValue =
            static_cast<uint32_t>(static_cast<uint16_t>(
                out.moveImageTransaction.rectX)) |
            (static_cast<uint32_t>(static_cast<uint16_t>(
                 out.moveImageTransaction.rectY))
             << 16u);
        out.moveImageTransaction.destGlobalValue =
            static_cast<uint32_t>(out.moveImageTransaction.destX) |
            (static_cast<uint32_t>(out.moveImageTransaction.destY) << 16u);
        out.moveImageTransaction.sizeGlobalValue =
            static_cast<uint32_t>(static_cast<uint16_t>(
                out.moveImageTransaction.rectWidth)) |
            (static_cast<uint32_t>(static_cast<uint16_t>(
                 out.moveImageTransaction.rectHeight))
             << 16u);
        out.wrapperStateAfter = 0u;
        out.status = Event4DrawBodyStatus8001E750::MoveImageReady;
        out.accepted = true;
        return out;
    }
}

Event4ModalInitTransaction800267C8 BuildEvent4ModalInitSoftware800267C8(
    const Event4ModalInitInput800267C8& input)
{
    Event4ModalInitTransaction800267C8 out{};
    if (!input.outputPointerKnown || input.outputPointer == 0u) {
        return out;
    }
    // 800267C8 resets the modal cadence and publishes -1 through *(a1+16).
    out.accepted = true;
    out.outputWrite = true;
    out.outputPointer = input.outputPointer;
    out.cueFrameBefore = 0u;
    out.cueFrameAfter = 0u;
    out.remainingBefore = 0u;
    return out;
}

Event4ModalInputTransaction80025F0C ExecuteEvent4ModalInputSoftware80025F0C(
    uint32_t inputMask,
    const Event4ModalInputSinks80025F0C& sinks)
{
    Event4ModalInputTransaction80025F0C out{};
    out.inputMask = inputMask;

    // 80025F0C ignores every pad value except exact 0x40/0x20.  Do not
    // convert a held or combined mask into a modal decision here.
    if (inputMask != 0x40u && inputMask != 0x20u) {
        out.status = Event4ModalInputStatus80025F0C::IgnoredInput;
        out.accepted = true;
        return out;
    }

    out.sfxInputMask = inputMask == 0x40u ? 0x20u : 0x40u;
    if (sinks.playSfx80025C8C == nullptr) {
        out.status = Event4ModalInputStatus80025F0C::AudioSinkMissing;
        return out;
    }
    if (!sinks.playSfx80025C8C(out.sfxInputMask, sinks.userData)) {
        out.status = Event4ModalInputStatus80025F0C::AudioSinkFailed;
        return out;
    }

    out.outputWrite = true;
    out.outputValue = inputMask == 0x40u ? 0 : 1;
    out.result = inputMask == 0x40u ? 1 : 2;
    out.sfxExecuted = true;
    out.status = inputMask == 0x40u
                     ? Event4ModalInputStatus80025F0C::CrossAccepted
                     : Event4ModalInputStatus80025F0C::CircleAccepted;
    out.accepted = true;
    return out;
}

Event4ModalTickTransaction80025E6C ExecuteEvent4ModalTickSoftware80025E6C(
    const Event4ModalTickInput80025E6C& input,
    const Event4ModalTickSinks80025E6C& sinks)
{
    Event4ModalTickTransaction80025E6C out{};
    if (!input.remainingKnown || !input.cueFrameKnown) {
        return out;
    }
    if (input.cueFrame > kEvent4ModalCueFramePeriod80025E6C) {
        return out;
    }

    out.cueFrameBefore = input.cueFrame;
    out.cueFrameAfter = input.cueFrame ==
                                kEvent4ModalCueFramePeriod80025E6C
                            ? 0u
                            : input.cueFrame + 1u;
    out.remainingBefore = input.remaining;
    out.remainingAfter = input.remaining;

    // 80025E6C returns immediately while the modal countdown is exhausted;
    // the cue-frame word is deliberately left untouched on that path.
    if (input.remaining == 0u) {
        out.tickReturn = input.remaining;
        out.cueFrameAfter = input.cueFrame;
        out.status = Event4ModalTickStatus80025E6C::Idle;
        out.accepted = true;
        return out;
    }

    // The original returns the incremented frame value, even when frame 72
    // is immediately wrapped to zero in the stored word.
    out.tickReturn = input.cueFrame + 1u;

    const bool secondCue =
        input.cueFrame == kEvent4ModalCueFrameOffset80025E6C;
    const bool firstCue = input.cueFrame == 0u;
    if (!firstCue && !secondCue) {
        out.status = Event4ModalTickStatus80025E6C::Advanced;
        out.accepted = true;
        return out;
    }

    if (!input.cueGlobalAddressKnown ||
        input.cueGlobalAddress != kCueEvent4_8009441C) {
        out.status = Event4ModalTickStatus80025E6C::CueBindingMissing;
        return out;
    }
    if (sinks.playCue80026EF8 == nullptr || sinks.flush80026ECC == nullptr) {
        out.status = Event4ModalTickStatus80025E6C::CueSinkMissing;
        return out;
    }

    out.cueIssued = true;
    out.cueAddress = input.cueGlobalAddress +
                     (secondCue ? kEvent4ModalSecondCueByteOffset80025E6C
                                : 0u);
    if (!sinks.playCue80026EF8(out.cueAddress, sinks.userData)) {
        out.status = Event4ModalTickStatus80025E6C::CueSinkFailed;
        return out;
    }
    out.cueExecuted = true;
    if (!sinks.flush80026ECC(sinks.userData)) {
        out.status = Event4ModalTickStatus80025E6C::CueSinkFailed;
        return out;
    }
    out.flushExecuted = true;
    if (secondCue) {
        --out.remainingAfter;
    }
    out.status = Event4ModalTickStatus80025E6C::Advanced;
    out.accepted = true;
    return out;
}

namespace {

bool HasExactEvent4FrameCommandOrder80026B94(
    const Event4FrameTransaction80026B94& state)
{
    return state.commandCount == kEvent4FrameCommandCount80026B94 &&
           state.commands[0].kind ==
               Event4FrameCommandKind80026B94::Tick80025E6C &&
           state.commands[0].psxFunction == kFn80025E6C &&
           !state.commands[0].arg1Known &&
           state.commands[1].kind ==
               Event4FrameCommandKind80026B94::DrawRoute8001E750 &&
           state.commands[1].psxFunction == kFn8001E750 &&
           state.commands[1].arg0 == 4 && state.commands[1].arg1Known &&
           state.commands[1].arg1 == kCtxEvent4_8006ED74 &&
           state.commands[2].kind ==
               Event4FrameCommandKind80026B94::WaitFrame80035560 &&
           state.commands[2].psxFunction == kFn80035560 &&
           state.commands[2].arg0 == 0 && !state.commands[2].arg1Known &&
           state.commands[3].kind ==
               Event4FrameCommandKind80026B94::EndFrame8001EA00 &&
           state.commands[3].psxFunction == kFn8001EA00 &&
           state.commands[3].arg0 == 4 && !state.commands[3].arg1Known &&
           state.commands[4].kind ==
               Event4FrameCommandKind80026B94::TextFlush800436F0 &&
           state.commands[4].psxFunction == kFn800436F0 &&
           state.commands[4].arg0 == -1 && !state.commands[4].arg1Known;
}

bool IsValidEvent4FrameClass80026B94(
    const Event4FrameTransactionBegin80026B94& begin)
{
    switch (begin.frameClass) {
    case Event4FrameClass80026B94::OpenInputLoop:
        return !begin.inputClosed && begin.tailFramesRemainingBefore == -1 &&
               begin.tailFramesRemainingAfter == -1;
    case Event4FrameClass80026B94::ResultAction:
        return !begin.inputClosed &&
               begin.tailFramesRemainingBefore ==
                   kDispatcherResultTailFrames80026B94 &&
               begin.tailFramesRemainingAfter ==
                   kDispatcherResultTailFrames80026B94;
    case Event4FrameClass80026B94::ClosedInputTail:
        return begin.inputClosed && begin.tailFramesRemainingBefore >= 1 &&
               begin.tailFramesRemainingBefore <=
                   kDispatcherResultTailFrames80026B94 &&
               begin.tailFramesRemainingAfter ==
                   begin.tailFramesRemainingBefore - 1;
    default:
        return false;
    }
}

} // namespace

void ResetEvent4FrameTransaction80026B94(
    Event4FrameTransaction80026B94& state)
{
    state = Event4FrameTransaction80026B94{};
}

bool CanAdvanceEvent4FrameLogic80026B94(
    const Event4FrameTransaction80026B94& state)
{
    return !state.active ||
           (state.directDrawCompleteWithinLimits &&
            state.hostFrameBoundaryExecuted);
}

bool TryStepEvent4DispatcherTail80026B94(
    const Event4FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep)
{
    outStep = DispatcherTailStep80026B94{};
    if (!CanAdvanceEvent4FrameLogic80026B94(transaction)) {
        return false;
    }
    outStep = StepDispatcherTail80026B94(tail);
    return true;
}

bool BeginEvent4FrameTransaction80026B94(
    Event4FrameTransaction80026B94& state,
    const Event4FrameTransactionBegin80026B94& begin)
{
    if ((state.active && !CanAdvanceEvent4FrameLogic80026B94(state)) ||
        !begin.requestBound || begin.ctxAddress != kCtxEvent4_8006ED74 ||
        !IsValidEvent4FrameClass80026B94(begin)) {
        return false;
    }

    ResetEvent4FrameTransaction80026B94(state);
    state.active = true;
    state.requestBound = true;
    state.logicFrame = begin.logicFrame;
    state.ctxAddress = begin.ctxAddress;
    state.frameClass = begin.frameClass;
    state.inputClosed = begin.inputClosed;
    state.tailFramesRemainingBefore = begin.tailFramesRemainingBefore;
    state.tailFramesRemainingAfter = begin.tailFramesRemainingAfter;
    state.commandCount = kEvent4FrameCommandCount80026B94;
    state.commands[0] = {
        Event4FrameCommandKind80026B94::Tick80025E6C,
        kFn80025E6C,
        0,
        0u,
        false};
    state.commands[1] = {
        Event4FrameCommandKind80026B94::DrawRoute8001E750,
        kFn8001E750,
        4,
        begin.ctxAddress,
        true};
    state.commands[2] = {
        Event4FrameCommandKind80026B94::WaitFrame80035560,
        kFn80035560,
        0,
        0u,
        false};
    state.commands[3] = {
        Event4FrameCommandKind80026B94::EndFrame8001EA00,
        kFn8001EA00,
        4,
        0u,
        false};
    state.commands[4] = {
        Event4FrameCommandKind80026B94::TextFlush800436F0,
        kFn800436F0,
        -1,
        0u,
        false};
    return true;
}

bool CommitEvent4FrameTick80025E6C(
    Event4FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t ctxAddress)
{
    if (!state.active || state.directDrawCompleteWithinLimits ||
        state.tickCommitted || state.logicFrame != logicFrame ||
        ctxAddress != kCtxEvent4_8006ED74 || state.ctxAddress != ctxAddress ||
        !HasExactEvent4FrameCommandOrder80026B94(state)) {
        return false;
    }
    state.tickCommitted = true;
    return true;
}

bool CanSubmitEvent4FrameDraw8001E750(
    const Event4FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!state.active || !state.requestBound || !state.tickCommitted ||
        state.ctxAddress != kCtxEvent4_8006ED74 ||
        !HasExactEvent4FrameCommandOrder80026B94(state)) {
        return false;
    }
    if (!state.directDrawCompleteWithinLimits) {
        return static_cast<int32_t>(presentationFrame - state.logicFrame) >= 0;
    }
    return renderOnlyPresentation &&
           presentationFrame == state.firstHostPresentationFrame;
}

bool CommitEvent4FrameDrawAndBindFormalTail80026B94(
    Event4FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!CanSubmitEvent4FrameDraw8001E750(
            state, presentationFrame, renderOnlyPresentation)) {
        return false;
    }
    if (state.directDrawCompleteWithinLimits) {
        ++state.hostPresentationSubmitCount;
        return true;
    }
    state.directDrawSubmitted = true;
    state.formalTailOrderBound = true;
    state.hostFrameBoundaryExecuted = false;
    state.exactPsxHalParity = false;
    state.directDrawCompleteWithinLimits = true;
    state.firstHostPresentationFrame = presentationFrame;
    state.hostPresentationSubmitCount = 1u;
    return true;
}

bool CommitEvent4FrameHostBoundary80026B94(
    Event4FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted)
{
    if (!state.active || !state.directDrawSubmitted ||
        !state.formalTailOrderBound || state.hostFrameBoundaryExecuted) {
        return false;
    }
    state.hostWaitExecuted = hostWaitExecuted;
    state.hostEndExecuted = hostEndExecuted;
    state.hostTextFlushExecuted = hostTextFlushExecuted;
    state.hostFrameBoundaryExecuted = hostWaitExecuted && hostEndExecuted &&
                                      hostTextFlushExecuted;
    state.exactPsxHalParity = false;
    return state.hostFrameBoundaryExecuted;
}

void ResetDispatcherTail80026B94(DispatcherTailState80026B94& state)
{
    state = DispatcherTailState80026B94{};
}

bool ArmDispatcherTail80026B94(DispatcherTailState80026B94& state,
                               int32_t result)
{
    if (state.active || result == 0) {
        return false;
    }
    state.active = true;
    state.latchedResult = result;
    state.framesRemaining = kDispatcherResultTailFrames80026B94;
    return true;
}

DispatcherTailStep80026B94 StepDispatcherTail80026B94(
    DispatcherTailState80026B94& state)
{
    DispatcherTailStep80026B94 step{};
    if (!state.active) {
        return step;
    }

    step.inputClosed = true;
    step.framesRemainingBefore = state.framesRemaining;
    if (state.framesRemaining > 0) {
        --state.framesRemaining;
        step.kind = DispatcherTailStepKind80026B94::HoldFrame;
        step.framesRemainingAfter = state.framesRemaining;
        return step;
    }

    step.kind = DispatcherTailStepKind80026B94::ReleaseResult;
    step.releasedResult = state.latchedResult;
    ResetDispatcherTail80026B94(state);
    return step;
}

void ResetEvent6FrameTransaction80026B94(
    Event6FrameTransaction80026B94& state)
{
    state = Event6FrameTransaction80026B94{};
}

bool CanAdvanceEvent6FrameLogic80026B94(
    const Event6FrameTransaction80026B94& state)
{
    return !state.active ||
           (state.directDrawCompleteWithinLimits &&
            state.hostFrameBoundaryExecuted);
}

bool TryStepEvent6DispatcherTail80026B94(
    const Event6FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep)
{
    outStep = DispatcherTailStep80026B94{};
    if (!CanAdvanceEvent6FrameLogic80026B94(transaction)) {
        return false;
    }
    outStep = StepDispatcherTail80026B94(tail);
    return true;
}

bool BeginEvent6FrameTransaction80026B94(
    Event6FrameTransaction80026B94& state,
    const Event6FrameTransactionBegin80026B94& begin)
{
    // A direct-draw-complete transaction may be replaced by the following
    // logic frame. An uncommitted one stays pending until its original draw
    // succeeds; this does not claim the remaining PSX HAL effects complete.
    if ((state.active && !CanAdvanceEvent6FrameLogic80026B94(state)) ||
        !begin.requestBound ||
        begin.ctxAddress != kCtxEvent6HiScoreTable80049278) {
        return false;
    }

    switch (begin.frameClass) {
    case Event6FrameClass80026B94::OpenInputLoop:
        if (begin.inputClosed || begin.tailFramesRemainingBefore != -1 ||
            begin.tailFramesRemainingAfter != -1) {
            return false;
        }
        break;
    case Event6FrameClass80026B94::ResultAction:
        if (begin.inputClosed ||
            begin.tailFramesRemainingBefore !=
                kDispatcherResultTailFrames80026B94 ||
            begin.tailFramesRemainingAfter !=
                kDispatcherResultTailFrames80026B94) {
            return false;
        }
        break;
    case Event6FrameClass80026B94::ClosedInputTail:
        if (!begin.inputClosed || begin.tailFramesRemainingBefore < 1 ||
            begin.tailFramesRemainingBefore >
                kDispatcherResultTailFrames80026B94 ||
            begin.tailFramesRemainingAfter !=
                begin.tailFramesRemainingBefore - 1) {
            return false;
        }
        break;
    default:
        return false;
    }

    ResetEvent6FrameTransaction80026B94(state);
    state.active = true;
    state.requestBound = true;
    state.logicFrame = begin.logicFrame;
    state.ctxAddress = begin.ctxAddress;
    state.frameClass = begin.frameClass;
    state.inputClosed = begin.inputClosed;
    state.tailFramesRemainingBefore = begin.tailFramesRemainingBefore;
    state.tailFramesRemainingAfter = begin.tailFramesRemainingAfter;
    state.commandCount = kEvent6FrameCommandCount80026B94;
    state.commands[0] = {
        Event6FrameCommandKind80026B94::Tick80025E48,
        kFn80025E48,
        static_cast<int32_t>(kEventTable6_80054578),
        begin.ctxAddress,
        true};
    state.commands[1] = {
        Event6FrameCommandKind80026B94::DrawRoute8001E750,
        kFn8001E750,
        6,
        begin.ctxAddress,
        true};
    state.commands[2] = {
        Event6FrameCommandKind80026B94::PrepareDrawWork8001D74C,
        kFn8001D74C,
        3,
        0u,
        false};
    state.commands[3] = {
        Event6FrameCommandKind80026B94::DrawPage80021594,
        kFn80021594,
        static_cast<int32_t>(begin.ctxAddress),
        0u};
    state.commands[4] = {
        Event6FrameCommandKind80026B94::WaitFrame80035560,
        kFn80035560,
        0,
        0u};
    state.commands[5] = {
        Event6FrameCommandKind80026B94::EndFrame8001EA00,
        kFn8001EA00,
        6,
        0u};
    state.commands[6] = {
        Event6FrameCommandKind80026B94::TextFlush800436F0,
        kFn800436F0,
        -1,
        0u};
    return true;
}

bool CommitEvent6FrameTick80025E48(
    Event6FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t tableAddress,
    uint32_t ctxAddress)
{
    if (!state.active || state.directDrawCompleteWithinLimits ||
        state.tickCommitted ||
        state.logicFrame != logicFrame ||
        tableAddress != kEventTable6_80054578 ||
        ctxAddress != kCtxEvent6HiScoreTable80049278 ||
        state.ctxAddress != ctxAddress ||
        state.commandCount != kEvent6FrameCommandCount80026B94 ||
        state.commands[0].kind !=
            Event6FrameCommandKind80026B94::Tick80025E48 ||
        state.commands[0].psxFunction != kFn80025E48) {
        return false;
    }
    state.tickCommitted = true;
    return true;
}

bool CanSubmitEvent6FrameDraw8001E750(
    const Event6FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!state.active || !state.requestBound || !state.tickCommitted ||
        state.ctxAddress != kCtxEvent6HiScoreTable80049278 ||
        !HasExactEvent6FrameCommandOrder80026B94(state)) {
        return false;
    }
    if (!state.directDrawCompleteWithinLimits) {
        // A missing renderer or a failed resource preflight must keep the
        // translated logic frame pending.  A later host frame may retry the
        // same transaction, but no later Event6 logic is allowed to advance.
        return static_cast<int32_t>(presentationFrame - state.logicFrame) >= 0;
    }
    // A direct-draw-complete translated frame may be presented again only by
    // the 60 Hz render-only half-step paired with its first successful host
    // presentation.  That host frame can be later than logicFrame after a
    // fail-closed resource retry.
    return renderOnlyPresentation &&
           presentationFrame == state.firstHostPresentationFrame;
}

bool CommitEvent6FrameDrawAndBindFormalTail80026B94(
    Event6FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!CanSubmitEvent6FrameDraw8001E750(
            state, presentationFrame, renderOnlyPresentation)) {
        return false;
    }
    if (state.directDrawCompleteWithinLimits) {
        ++state.hostPresentationSubmitCount;
        return true;
    }
    state.directDrawSubmitted = true;
    // The page draw is complete within SS0's current limits. The formal
    // wait/end/text calls execute through a separate SS0 host boundary before
    // logic or tail accounting can advance.
    state.formalTailOrderBound = true;
    state.hostWaitExecuted = false;
    state.hostEndExecuted = false;
    state.hostTextFlushExecuted = false;
    state.hostFrameBoundaryExecuted = false;
    state.exactPsxHalParity = false;
    state.directDrawCompleteWithinLimits = true;
    state.firstHostPresentationFrame = presentationFrame;
    state.hostPresentationSubmitCount = 1u;
    return true;
}

bool CommitEvent6FrameHostBoundary80026B94(
    Event6FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted)
{
    if (!state.active || !state.directDrawSubmitted ||
        !state.formalTailOrderBound || state.hostFrameBoundaryExecuted) {
        return false;
    }
    state.hostWaitExecuted = hostWaitExecuted;
    state.hostEndExecuted = hostEndExecuted;
    state.hostTextFlushExecuted = hostTextFlushExecuted;
    state.hostFrameBoundaryExecuted = hostWaitExecuted && hostEndExecuted &&
                                      hostTextFlushExecuted;
    state.exactPsxHalParity = false;
    return state.hostFrameBoundaryExecuted;
}

void ResetEvent6EndFrameOwnerPlan8001EA00(
    Event6EndFrameOwnerPlan8001EA00& plan)
{
    plan = Event6EndFrameOwnerPlan8001EA00{};
}

bool BuildEvent6EndFrameOwnerPlan8001EA00(
    const Event6EndFrameOwnerInput8001EA00& input,
    Event6EndFrameOwnerPlan8001EA00& outPlan)
{
    ResetEvent6EndFrameOwnerPlan8001EA00(outPlan);
    if (!input.requestBound || input.eventId != 6 ||
        !input.graphSlotBeforeKnown || input.graphSlotBefore > 1u ||
        !input.workListSlotKnown || input.workListSlot > 1u ||
        !input.workListAddressKnown) {
        return false;
    }

    const uint64_t expectedWorkListAddress =
        static_cast<uint64_t>(kEvent6EndFrameWorkListBase80087288) +
        static_cast<uint64_t>(input.workListSlot) *
            static_cast<uint64_t>(kEvent6EndFrameWorkListStride8001EA00);
    if (expectedWorkListAddress > 0xFFFFFFFFull ||
        input.workListAddress !=
            static_cast<uint32_t>(expectedWorkListAddress)) {
        return false;
    }

    outPlan.formalOrderBound = true;
    outPlan.graphSlotBefore = input.graphSlotBefore;
    outPlan.graphSlotAfter =
        static_cast<uint16_t>(input.graphSlotBefore ^ 1u);
    outPlan.workListSlot = input.workListSlot;
    outPlan.workListAddress = input.workListAddress;
    outPlan.actionCount = kEvent6EndFrameActionCount8001EA00;
    outPlan.actions[0] = {
        Event6EndFrameActionKind8001EA00::FlipGraph80040370,
        kFn80040370,
        0u,
        0u,
        0u};
    outPlan.actions[1] = {
        Event6EndFrameActionKind8001EA00::ClearColor80040420,
        kFn80040420,
        kEvent6EndFrameClearR80040420,
        kEvent6EndFrameClearG80040420,
        kEvent6EndFrameClearB80040420};
    outPlan.actions[2] = {
        Event6EndFrameActionKind8001EA00::SubmitWorkList80040CA4,
        kFn80040CA4,
        input.workListAddress,
        input.workListSlot,
        0u};

    // This planner records the formal tail but does not execute graph effects.
    // Current IDA now binds gp+872 to the pre-flip gp+0x368 draw slot; only the
    // translated event-frame owner may credit clear/submit execution.
    outPlan.graphModelReady =
        input.workListReady && input.graphModelPreflightKnown &&
        input.graphModelWorkListSlotMatches;
    outPlan.formalOnly = !outPlan.graphModelReady;
    return true;
}

namespace {

bool HasExactEvent2FrameCommandOrder80026B94(
    const Event2FrameTransaction80026B94& state)
{
    return state.commandCount == kEvent2FrameCommandCount80026B94 &&
           state.commands[0].kind ==
               Event2FrameCommandKind80026B94::Tick80026170 &&
           state.commands[0].psxFunction == kFn80026170 &&
           static_cast<uint32_t>(state.commands[0].arg0) ==
               kEventTable2_8005453C &&
           state.commands[0].arg1Known &&
           state.commands[0].arg1 == kCtxEvent2_80087B78 &&
           !state.commands[0].conditional &&
           state.commands[1].kind ==
               Event2FrameCommandKind80026B94::DrawRoute8001E750 &&
           state.commands[1].psxFunction == kFn8001E750 &&
           state.commands[1].arg0 == 2 &&
           state.commands[1].arg1Known &&
           state.commands[1].arg1 == kCtxEvent2_80087B78 &&
           !state.commands[1].conditional &&
           state.commands[2].kind ==
               Event2FrameCommandKind80026B94::PrepareDrawWork8001D74C &&
           state.commands[2].psxFunction == kFn8001D74C &&
           state.commands[2].arg0 == 3 &&
           !state.commands[2].arg1Known &&
           !state.commands[2].conditional &&
           state.commands[3].kind ==
               Event2FrameCommandKind80026B94::DrawPage80020568 &&
           state.commands[3].psxFunction == kFn80020568 &&
           static_cast<uint32_t>(state.commands[3].arg0) ==
               kCtxEvent2_80087B78 &&
           state.commands[3].arg1 == 0u &&
           !state.commands[3].arg1Known &&
           !state.commands[3].conditional &&
           state.commands[4].kind ==
               Event2FrameCommandKind80026B94::StageClearText80043A14 &&
           state.commands[4].psxFunction == kFn80043A14 &&
           state.commands[4].arg1 == 0x800916F6u &&
           state.commands[4].arg1Known &&
           state.commands[4].conditional &&
           state.commands[4].conditionalGateAddress == 0x800916F6u &&
           state.commands[4].conditionalGateValueKnown &&
           state.commands[4].conditionalGateValue == 0u &&
           !state.commands[4].conditionalTaken &&
           state.stageClearGateValueKnown800916F6 &&
           state.stageClearGateValue800916F6 == 0u &&
           !state.stageClearBranchTaken80026D70 &&
           state.commands[5].kind ==
               Event2FrameCommandKind80026B94::WaitFrame80035560 &&
           state.commands[5].psxFunction == kFn80035560 &&
           state.commands[5].arg0 == 0 &&
           !state.commands[5].conditional &&
           state.commands[6].kind ==
               Event2FrameCommandKind80026B94::EndFrame8001EA00 &&
           state.commands[6].psxFunction == kFn8001EA00 &&
           state.commands[6].arg0 == 2 &&
           !state.commands[6].conditional &&
           state.commands[7].kind ==
               Event2FrameCommandKind80026B94::TextFlush800436F0 &&
           state.commands[7].psxFunction == kFn800436F0 &&
           state.commands[7].arg0 == -1 &&
           !state.commands[7].conditional;
}

bool IsValidEvent2FrameClass80026B94(
    const Event2FrameTransactionBegin80026B94& begin)
{
    switch (begin.frameClass) {
    case Event2FrameClass80026B94::OpenInputLoop:
        return !begin.inputClosed &&
               begin.tailFramesRemainingBefore == -1 &&
               begin.tailFramesRemainingAfter == -1;
    case Event2FrameClass80026B94::ResultAction:
        return !begin.inputClosed &&
               begin.tailFramesRemainingBefore ==
                   kDispatcherResultTailFrames80026B94 &&
               begin.tailFramesRemainingAfter ==
                   kDispatcherResultTailFrames80026B94;
    case Event2FrameClass80026B94::ClosedInputTail:
        return begin.inputClosed && begin.tailFramesRemainingBefore >= 1 &&
               begin.tailFramesRemainingBefore <=
                   kDispatcherResultTailFrames80026B94 &&
               begin.tailFramesRemainingAfter ==
                   begin.tailFramesRemainingBefore - 1;
    default:
        return false;
    }
}

} // namespace

void ResetEvent2FrameTransaction80026B94(
    Event2FrameTransaction80026B94& state)
{
    state = Event2FrameTransaction80026B94{};
}

bool CanAdvanceEvent2FrameLogic80026B94(
    const Event2FrameTransaction80026B94& state)
{
    return !state.active ||
           (state.directDrawCompleteWithinLimits &&
            state.hostFrameBoundaryExecuted);
}

bool TryStepEvent2DispatcherTail80026B94(
    const Event2FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep)
{
    outStep = DispatcherTailStep80026B94{};
    if (!CanAdvanceEvent2FrameLogic80026B94(transaction)) {
        return false;
    }
    outStep = StepDispatcherTail80026B94(tail);
    return true;
}

bool BeginEvent2FrameTransaction80026B94(
    Event2FrameTransaction80026B94& state,
    const Event2FrameTransactionBegin80026B94& begin)
{
    // Keep an unfinished StageSelect page pending until its own draw succeeds;
    // replacing it would spend an input/tail frame without a presentation.
    if ((state.active && !CanAdvanceEvent2FrameLogic80026B94(state)) ||
        !begin.requestBound || begin.ctxAddress != kCtxEvent2_80087B78 ||
        !begin.word800916F6Known || begin.word800916F6 != 0u ||
        !IsValidEvent2FrameClass80026B94(begin)) {
        return false;
    }

    ResetEvent2FrameTransaction80026B94(state);
    state.active = true;
    state.requestBound = true;
    state.logicFrame = begin.logicFrame;
    state.ctxAddress = begin.ctxAddress;
    state.frameClass = begin.frameClass;
    state.inputClosed = begin.inputClosed;
    state.tailFramesRemainingBefore = begin.tailFramesRemainingBefore;
    state.tailFramesRemainingAfter = begin.tailFramesRemainingAfter;
    state.stageClearGateValueKnown800916F6 = begin.word800916F6Known;
    state.stageClearGateValue800916F6 = begin.word800916F6;
    state.stageClearBranchTaken80026D70 = begin.word800916F6 != 0u;
    state.commandCount = kEvent2FrameCommandCount80026B94;
    state.commands[0] = {
        Event2FrameCommandKind80026B94::Tick80026170,
        kFn80026170,
        static_cast<int32_t>(kEventTable2_8005453C),
        begin.ctxAddress,
        true,
        false};
    state.commands[1] = {
        Event2FrameCommandKind80026B94::DrawRoute8001E750,
        kFn8001E750,
        2,
        begin.ctxAddress,
        true,
        false};
    state.commands[2] = {
        Event2FrameCommandKind80026B94::PrepareDrawWork8001D74C,
        kFn8001D74C,
        3,
        0u,
        false,
        false};
    state.commands[3] = {
        Event2FrameCommandKind80026B94::DrawPage80020568,
        kFn80020568,
        static_cast<int32_t>(begin.ctxAddress),
        0u,
        false,
        false};
    state.commands[4] = {
        Event2FrameCommandKind80026B94::StageClearText80043A14,
        kFn80043A14,
        0,
        0x800916F6u,
        true,
        true};
    state.commands[4].conditionalGateAddress = 0x800916F6u;
    state.commands[4].conditionalGateValueKnown = begin.word800916F6Known;
    state.commands[4].conditionalGateValue = begin.word800916F6;
    state.commands[4].conditionalTaken = begin.word800916F6 != 0u;
    state.commands[5] = {
        Event2FrameCommandKind80026B94::WaitFrame80035560,
        kFn80035560,
        0,
        0u,
        false,
        false};
    state.commands[6] = {
        Event2FrameCommandKind80026B94::EndFrame8001EA00,
        kFn8001EA00,
        2,
        0u,
        false,
        false};
    state.commands[7] = {
        Event2FrameCommandKind80026B94::TextFlush800436F0,
        kFn800436F0,
        -1,
        0u,
        false,
        false};
    return true;
}

bool CommitEvent2FrameTick80026170(
    Event2FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t tableAddress,
    uint32_t ctxAddress)
{
    if (!state.active || state.directDrawCompleteWithinLimits ||
        state.tickCommitted || state.logicFrame != logicFrame ||
        tableAddress != kEventTable2_8005453C ||
        ctxAddress != kCtxEvent2_80087B78 || state.ctxAddress != ctxAddress ||
        state.commandCount != kEvent2FrameCommandCount80026B94 ||
        state.commands[0].kind !=
            Event2FrameCommandKind80026B94::Tick80026170 ||
        state.commands[0].psxFunction != kFn80026170) {
        return false;
    }
    state.tickCommitted = true;
    return true;
}

bool CanSubmitEvent2FrameDraw8001E750(
    const Event2FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!state.active || !state.requestBound || !state.tickCommitted ||
        state.ctxAddress != kCtxEvent2_80087B78 ||
        !HasExactEvent2FrameCommandOrder80026B94(state)) {
        return false;
    }
    if (!state.directDrawCompleteWithinLimits) {
        // A resource/render failure retries this same logic frame later; it
        // cannot advance the dispatcher tail or input until the page submits.
        return static_cast<int32_t>(presentationFrame - state.logicFrame) >= 0;
    }
    return renderOnlyPresentation &&
           presentationFrame == state.firstHostPresentationFrame;
}

bool CommitEvent2FrameDrawAndBindFormalTail80026B94(
    Event2FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!CanSubmitEvent2FrameDraw8001E750(
            state, presentationFrame, renderOnlyPresentation)) {
        return false;
    }
    if (state.directDrawCompleteWithinLimits) {
        ++state.hostPresentationSubmitCount;
        return true;
    }
    state.directDrawSubmitted = true;
    // The page draw is the only completed SS0 effect here. The conditional
    // StageClear producer remains source-gated, and wait/end/text execute
    // through a separate SS0 host boundary before logic can advance.
    state.formalTailOrderBound = true;
    state.hostWaitExecuted = false;
    state.hostEndExecuted = false;
    state.hostTextFlushExecuted = false;
    state.hostFrameBoundaryExecuted = false;
    state.exactPsxHalParity = false;
    state.directDrawCompleteWithinLimits = true;
    state.firstHostPresentationFrame = presentationFrame;
    state.hostPresentationSubmitCount = 1u;
    return true;
}

bool CommitEvent2FrameHostBoundary80026B94(
    Event2FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted)
{
    if (!state.active || !state.directDrawSubmitted ||
        !state.formalTailOrderBound || state.hostFrameBoundaryExecuted) {
        return false;
    }
    state.hostWaitExecuted = hostWaitExecuted;
    state.hostEndExecuted = hostEndExecuted;
    state.hostTextFlushExecuted = hostTextFlushExecuted;
    state.hostFrameBoundaryExecuted = hostWaitExecuted && hostEndExecuted &&
                                      hostTextFlushExecuted;
    state.exactPsxHalParity = false;
    return state.hostFrameBoundaryExecuted;
}

namespace {

bool HasExactEvent3FrameCommandOrder80026B94(
    const Event3FrameTransaction80026B94& state)
{
    return state.commandCount == kEvent3FrameCommandCount80026B94 &&
           state.commands[0].kind ==
               Event3FrameCommandKind80026B94::Tick80026720 &&
           state.commands[0].psxFunction == kFn80026720 &&
           static_cast<uint32_t>(state.commands[0].arg0) ==
               kEventTable3_80054550 &&
           state.commands[0].arg1Known &&
           state.commands[0].arg1 == kCtxEvent3_800544F8 &&
           state.commands[1].kind ==
               Event3FrameCommandKind80026B94::DrawRoute8001E750 &&
           state.commands[1].psxFunction == kFn8001E750 &&
           state.commands[1].arg0 == 3 &&
           state.commands[1].arg1Known &&
           state.commands[1].arg1 == kCtxEvent3_800544F8 &&
           state.commands[2].kind ==
               Event3FrameCommandKind80026B94::PrepareDrawWork8001D74C &&
           state.commands[2].psxFunction == kFn8001D74C &&
           state.commands[2].arg0 == 3 &&
           !state.commands[2].arg1Known &&
           state.commands[3].kind ==
               Event3FrameCommandKind80026B94::DrawPage80021E60 &&
           state.commands[3].psxFunction == kFn80021E60 &&
           static_cast<uint32_t>(state.commands[3].arg0) ==
               kCtxEvent3_800544F8 &&
           state.commands[4].kind ==
               Event3FrameCommandKind80026B94::WaitFrame80035560 &&
           state.commands[4].psxFunction == kFn80035560 &&
           state.commands[4].arg0 == 0 &&
           state.commands[5].kind ==
               Event3FrameCommandKind80026B94::EndFrame8001EA00 &&
           state.commands[5].psxFunction == kFn8001EA00 &&
           state.commands[5].arg0 == 3 &&
           state.commands[6].kind ==
               Event3FrameCommandKind80026B94::TextFlush800436F0 &&
           state.commands[6].psxFunction == kFn800436F0 &&
           state.commands[6].arg0 == -1;
}

bool IsValidEvent3FrameClass80026B94(
    const Event3FrameTransactionBegin80026B94& begin)
{
    switch (begin.frameClass) {
    case Event3FrameClass80026B94::OpenInputLoop:
        return !begin.inputClosed &&
               begin.tailFramesRemainingBefore == -1 &&
               begin.tailFramesRemainingAfter == -1;
    case Event3FrameClass80026B94::ResultAction:
        return !begin.inputClosed &&
               begin.tailFramesRemainingBefore ==
                   kDispatcherResultTailFrames80026B94 &&
               begin.tailFramesRemainingAfter ==
                   kDispatcherResultTailFrames80026B94;
    case Event3FrameClass80026B94::ClosedInputTail:
        return begin.inputClosed && begin.tailFramesRemainingBefore >= 1 &&
               begin.tailFramesRemainingBefore <=
                   kDispatcherResultTailFrames80026B94 &&
               begin.tailFramesRemainingAfter ==
                   begin.tailFramesRemainingBefore - 1;
    default:
        return false;
    }
}

bool HasExactEvent17FrameCommandOrder80026B94(
    const Event17FrameTransaction80026B94& state)
{
    return state.commandCount == kEvent17FrameCommandCount80026B94 &&
           state.commands[0].kind ==
               Event17FrameCommandKind80026B94::Tick80026B54 &&
           state.commands[0].psxFunction == kFn80026B54 &&
           static_cast<uint32_t>(state.commands[0].arg0) ==
               kEventTable17_800545A0 &&
           state.commands[0].arg1Known &&
           state.commands[0].arg1 == kCtxEvent17_8005451C &&
           state.commands[1].kind ==
               Event17FrameCommandKind80026B94::DrawRoute8001E750 &&
           state.commands[1].psxFunction == kFn8001E750 &&
           state.commands[1].arg0 == 17 &&
           state.commands[1].arg1Known &&
           state.commands[1].arg1 == kCtxEvent17_8005451C &&
           state.commands[2].kind ==
               Event17FrameCommandKind80026B94::PrepareDrawWork8001D74C &&
           state.commands[2].psxFunction == kFn8001D74C &&
           state.commands[2].arg0 == 4 &&
           !state.commands[2].arg1Known &&
           state.commands[3].kind ==
               Event17FrameCommandKind80026B94::DrawPage80021910 &&
           state.commands[3].psxFunction == kFn80021910 &&
           static_cast<uint32_t>(state.commands[3].arg0) ==
               kCtxEvent17_8005451C &&
           state.commands[4].kind ==
               Event17FrameCommandKind80026B94::WaitFrame80035560 &&
           state.commands[4].psxFunction == kFn80035560 &&
           state.commands[4].arg0 == 0 &&
           state.commands[5].kind ==
               Event17FrameCommandKind80026B94::EndFrame8001EA00 &&
           state.commands[5].psxFunction == kFn8001EA00 &&
           state.commands[5].arg0 == 17 &&
           state.commands[6].kind ==
               Event17FrameCommandKind80026B94::TextFlush800436F0 &&
           state.commands[6].psxFunction == kFn800436F0 &&
           state.commands[6].arg0 == -1;
}

bool IsValidEvent17FrameClass80026B94(
    const Event17FrameTransactionBegin80026B94& begin)
{
    switch (begin.frameClass) {
    case Event17FrameClass80026B94::OpenInputLoop:
        return !begin.inputClosed &&
               begin.tailFramesRemainingBefore == -1 &&
               begin.tailFramesRemainingAfter == -1;
    case Event17FrameClass80026B94::ResultAction:
        return !begin.inputClosed &&
               begin.tailFramesRemainingBefore ==
                   kDispatcherResultTailFrames80026B94 &&
               begin.tailFramesRemainingAfter ==
                   kDispatcherResultTailFrames80026B94;
    case Event17FrameClass80026B94::ClosedInputTail:
        return begin.inputClosed && begin.tailFramesRemainingBefore >= 1 &&
               begin.tailFramesRemainingBefore <=
                   kDispatcherResultTailFrames80026B94 &&
               begin.tailFramesRemainingAfter ==
                   begin.tailFramesRemainingBefore - 1;
    default:
        return false;
    }
}

} // namespace

void ResetEvent3FrameTransaction80026B94(
    Event3FrameTransaction80026B94& state)
{
    state = Event3FrameTransaction80026B94{};
}

bool CanAdvanceEvent3FrameLogic80026B94(
    const Event3FrameTransaction80026B94& state)
{
    return !state.active ||
           (state.directDrawCompleteWithinLimits &&
            state.hostFrameBoundaryExecuted);
}

bool TryStepEvent3DispatcherTail80026B94(
    const Event3FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep)
{
    outStep = DispatcherTailStep80026B94{};
    if (!CanAdvanceEvent3FrameLogic80026B94(transaction)) {
        return false;
    }
    outStep = StepDispatcherTail80026B94(tail);
    return true;
}

bool BeginEvent3FrameTransaction80026B94(
    Event3FrameTransaction80026B94& state,
    const Event3FrameTransactionBegin80026B94& begin)
{
    // A completed translated frame may be replaced by the next logic frame;
    // draw-only or partially executed host boundaries remain pending.
    if ((state.active && !CanAdvanceEvent3FrameLogic80026B94(state)) ||
        !begin.requestBound || begin.ctxAddress != kCtxEvent3_800544F8 ||
        !IsValidEvent3FrameClass80026B94(begin)) {
        return false;
    }

    ResetEvent3FrameTransaction80026B94(state);
    state.active = true;
    state.requestBound = true;
    state.logicFrame = begin.logicFrame;
    state.ctxAddress = begin.ctxAddress;
    state.frameClass = begin.frameClass;
    state.inputClosed = begin.inputClosed;
    state.tailFramesRemainingBefore = begin.tailFramesRemainingBefore;
    state.tailFramesRemainingAfter = begin.tailFramesRemainingAfter;
    state.commandCount = kEvent3FrameCommandCount80026B94;
    state.commands[0] = {
        Event3FrameCommandKind80026B94::Tick80026720,
        kFn80026720,
        static_cast<int32_t>(kEventTable3_80054550),
        begin.ctxAddress,
        true};
    state.commands[1] = {
        Event3FrameCommandKind80026B94::DrawRoute8001E750,
        kFn8001E750,
        3,
        begin.ctxAddress,
        true};
    state.commands[2] = {
        Event3FrameCommandKind80026B94::PrepareDrawWork8001D74C,
        kFn8001D74C,
        3,
        0u,
        false};
    state.commands[3] = {
        Event3FrameCommandKind80026B94::DrawPage80021E60,
        kFn80021E60,
        static_cast<int32_t>(begin.ctxAddress),
        0u,
        true};
    state.commands[4] = {
        Event3FrameCommandKind80026B94::WaitFrame80035560,
        kFn80035560,
        0,
        0u,
        false};
    state.commands[5] = {
        Event3FrameCommandKind80026B94::EndFrame8001EA00,
        kFn8001EA00,
        3,
        0u,
        false};
    state.commands[6] = {
        Event3FrameCommandKind80026B94::TextFlush800436F0,
        kFn800436F0,
        -1,
        0u,
        false};
    return true;
}

bool CommitEvent3FrameTick80026720(
    Event3FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t tableAddress,
    uint32_t ctxAddress)
{
    if (!state.active || state.directDrawCompleteWithinLimits ||
        state.tickCommitted || state.logicFrame != logicFrame ||
        tableAddress != kEventTable3_80054550 ||
        ctxAddress != kCtxEvent3_800544F8 || state.ctxAddress != ctxAddress ||
        state.commandCount != kEvent3FrameCommandCount80026B94 ||
        state.commands[0].kind !=
            Event3FrameCommandKind80026B94::Tick80026720 ||
        state.commands[0].psxFunction != kFn80026720) {
        return false;
    }
    state.tickCommitted = true;
    return true;
}

bool CanSubmitEvent3FrameDraw8001E750(
    const Event3FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!state.active || !state.requestBound || !state.tickCommitted ||
        state.ctxAddress != kCtxEvent3_800544F8 ||
        !HasExactEvent3FrameCommandOrder80026B94(state)) {
        return false;
    }
    if (!state.directDrawCompleteWithinLimits) {
        return static_cast<int32_t>(presentationFrame - state.logicFrame) >= 0;
    }
    return renderOnlyPresentation &&
           presentationFrame == state.firstHostPresentationFrame;
}

bool CommitEvent3FrameDrawAndBindFormalTail80026B94(
    Event3FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!CanSubmitEvent3FrameDraw8001E750(
            state, presentationFrame, renderOnlyPresentation)) {
        return false;
    }
    if (state.directDrawCompleteWithinLimits) {
        ++state.hostPresentationSubmitCount;
        return true;
    }
    state.directDrawSubmitted = true;
    // The SS0 page projection has succeeded. Wait/end/text execute through a
    // separate SS0 host boundary before logic or tail accounting can advance.
    state.formalTailOrderBound = true;
    state.hostFrameBoundaryExecuted = false;
    state.exactPsxHalParity = false;
    state.directDrawCompleteWithinLimits = true;
    state.firstHostPresentationFrame = presentationFrame;
    state.hostPresentationSubmitCount = 1u;
    return true;
}

bool CommitEvent3FrameHostBoundary80026B94(
    Event3FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted)
{
    if (!state.active || !state.directDrawSubmitted ||
        !state.formalTailOrderBound || state.hostFrameBoundaryExecuted) {
        return false;
    }
    state.hostWaitExecuted = hostWaitExecuted;
    state.hostEndExecuted = hostEndExecuted;
    state.hostTextFlushExecuted = hostTextFlushExecuted;
    state.hostFrameBoundaryExecuted = hostWaitExecuted && hostEndExecuted &&
                                      hostTextFlushExecuted;
    state.exactPsxHalParity = false;
    return state.hostFrameBoundaryExecuted;
}

void ResetEvent17FrameTransaction80026B94(
    Event17FrameTransaction80026B94& state)
{
    state = Event17FrameTransaction80026B94{};
}

bool CanAdvanceEvent17FrameLogic80026B94(
    const Event17FrameTransaction80026B94& state)
{
    return !state.active ||
           (state.directDrawCompleteWithinLimits &&
            state.hostFrameBoundaryExecuted);
}

bool TryStepEvent17DispatcherTail80026B94(
    const Event17FrameTransaction80026B94& transaction,
    DispatcherTailState80026B94& tail,
    DispatcherTailStep80026B94& outStep)
{
    outStep = DispatcherTailStep80026B94{};
    if (!CanAdvanceEvent17FrameLogic80026B94(transaction)) {
        return false;
    }
    outStep = StepDispatcherTail80026B94(tail);
    return true;
}

bool BeginEvent17FrameTransaction80026B94(
    Event17FrameTransaction80026B94& state,
    const Event17FrameTransactionBegin80026B94& begin)
{
    // Keep an unfinished Options page pending until its own translated draw
    // succeeds. This prevents an input or tail frame from being spent while
    // the page is waiting for a retryable host presentation.
    if ((state.active && !CanAdvanceEvent17FrameLogic80026B94(state)) ||
        !begin.requestBound || begin.ctxAddress != kCtxEvent17_8005451C ||
        !IsValidEvent17FrameClass80026B94(begin)) {
        return false;
    }

    ResetEvent17FrameTransaction80026B94(state);
    state.active = true;
    state.requestBound = true;
    state.logicFrame = begin.logicFrame;
    state.ctxAddress = begin.ctxAddress;
    state.frameClass = begin.frameClass;
    state.inputClosed = begin.inputClosed;
    state.tailFramesRemainingBefore = begin.tailFramesRemainingBefore;
    state.tailFramesRemainingAfter = begin.tailFramesRemainingAfter;
    state.commandCount = kEvent17FrameCommandCount80026B94;
    state.commands[0] = {
        Event17FrameCommandKind80026B94::Tick80026B54,
        kFn80026B54,
        static_cast<int32_t>(kEventTable17_800545A0),
        begin.ctxAddress,
        true};
    state.commands[1] = {
        Event17FrameCommandKind80026B94::DrawRoute8001E750,
        kFn8001E750,
        17,
        begin.ctxAddress,
        true};
    state.commands[2] = {
        Event17FrameCommandKind80026B94::PrepareDrawWork8001D74C,
        kFn8001D74C,
        4,
        0u,
        false};
    state.commands[3] = {
        Event17FrameCommandKind80026B94::DrawPage80021910,
        kFn80021910,
        static_cast<int32_t>(begin.ctxAddress),
        0u,
        false};
    state.commands[4] = {
        Event17FrameCommandKind80026B94::WaitFrame80035560,
        kFn80035560,
        0,
        0u,
        false};
    state.commands[5] = {
        Event17FrameCommandKind80026B94::EndFrame8001EA00,
        kFn8001EA00,
        17,
        0u,
        false};
    state.commands[6] = {
        Event17FrameCommandKind80026B94::TextFlush800436F0,
        kFn800436F0,
        -1,
        0u,
        false};
    return true;
}

bool CommitEvent17FrameTick80026B54(
    Event17FrameTransaction80026B94& state,
    uint32_t logicFrame,
    uint32_t tableAddress,
    uint32_t ctxAddress)
{
    if (!state.active || state.directDrawCompleteWithinLimits ||
        state.tickCommitted || state.logicFrame != logicFrame ||
        tableAddress != kEventTable17_800545A0 ||
        ctxAddress != kCtxEvent17_8005451C || state.ctxAddress != ctxAddress ||
        state.commandCount != kEvent17FrameCommandCount80026B94 ||
        state.commands[0].kind !=
            Event17FrameCommandKind80026B94::Tick80026B54 ||
        state.commands[0].psxFunction != kFn80026B54) {
        return false;
    }
    state.tickCommitted = true;
    return true;
}

bool CanSubmitEvent17FrameDraw8001E750(
    const Event17FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!state.active || !state.requestBound || !state.tickCommitted ||
        state.ctxAddress != kCtxEvent17_8005451C ||
        !HasExactEvent17FrameCommandOrder80026B94(state)) {
        return false;
    }
    if (!state.directDrawCompleteWithinLimits) {
        return static_cast<int32_t>(presentationFrame - state.logicFrame) >= 0;
    }
    return renderOnlyPresentation &&
           presentationFrame == state.firstHostPresentationFrame;
}

bool CommitEvent17FrameDrawAndBindFormalTail80026B94(
    Event17FrameTransaction80026B94& state,
    uint32_t presentationFrame,
    bool renderOnlyPresentation)
{
    if (!CanSubmitEvent17FrameDraw8001E750(
            state, presentationFrame, renderOnlyPresentation)) {
        return false;
    }
    if (state.directDrawCompleteWithinLimits) {
        ++state.hostPresentationSubmitCount;
        return true;
    }
    state.directDrawSubmitted = true;
    // The translated Options page is complete within the current direct
    // rendering limits. The SS0-owned host boundary is committed separately
    // after the direct wait/end/text calls execute.
    state.formalTailOrderBound = true;
    state.hostFrameBoundaryExecuted = false;
    state.exactPsxHalParity = false;
    state.directDrawCompleteWithinLimits = true;
    state.firstHostPresentationFrame = presentationFrame;
    state.hostPresentationSubmitCount = 1u;
    return true;
}

bool CommitEvent17FrameHostBoundary80026B94(
    Event17FrameTransaction80026B94& state,
    bool hostWaitExecuted,
    bool hostEndExecuted,
    bool hostTextFlushExecuted)
{
    if (!state.active || !state.directDrawSubmitted ||
        !state.formalTailOrderBound || state.hostFrameBoundaryExecuted) {
        return false;
    }
    state.hostWaitExecuted = hostWaitExecuted;
    state.hostEndExecuted = hostEndExecuted;
    state.hostTextFlushExecuted = hostTextFlushExecuted;
    state.hostFrameBoundaryExecuted = hostWaitExecuted && hostEndExecuted &&
                                      hostTextFlushExecuted;
    // Host execution closes the translated boundary, but does not upgrade
    // the result to byte-for-byte PSX HAL parity.
    state.exactPsxHalParity = false;
    return state.hostFrameBoundaryExecuted;
}

void ResetEvent17EndFrameOwnerPlan8001EA00(
    Event17EndFrameOwnerPlan8001EA00& plan)
{
    plan = Event17EndFrameOwnerPlan8001EA00{};
}

bool BuildEvent17EndFrameOwnerPlan8001EA00(
    const Event17EndFrameOwnerInput8001EA00& input,
    Event17EndFrameOwnerPlan8001EA00& outPlan)
{
    ResetEvent17EndFrameOwnerPlan8001EA00(outPlan);
    if (!input.requestBound || input.eventId != 17 ||
        !input.graphSlotBeforeKnown || input.graphSlotBefore > 1u ||
        !input.workListSlotKnown || input.workListSlot > 1u ||
        !input.workListAddressKnown) {
        return false;
    }

    const uint64_t expectedWorkListAddress =
        static_cast<uint64_t>(kEvent17EndFrameWorkListBase80087288) +
        static_cast<uint64_t>(input.workListSlot) *
            static_cast<uint64_t>(kEvent17EndFrameWorkListStride8001EA00);
    if (expectedWorkListAddress > 0xFFFFFFFFull ||
        input.workListAddress !=
            static_cast<uint32_t>(expectedWorkListAddress)) {
        return false;
    }

    outPlan.formalOrderBound = true;
    outPlan.graphSlotBefore = input.graphSlotBefore;
    outPlan.graphSlotAfter =
        static_cast<uint16_t>(input.graphSlotBefore ^ 1u);
    outPlan.workListSlot = input.workListSlot;
    outPlan.workListAddress = input.workListAddress;
    outPlan.actionCount = kEvent17EndFrameActionCount8001EA00;
    outPlan.actions[0].kind =
        Event17EndFrameActionKind8001EA00::FlipGraph80040370;
    outPlan.actions[0].psxFunction = kFn80040370;

    outPlan.actions[1].kind =
        Event17EndFrameActionKind8001EA00::ClearColor80040420;
    outPlan.actions[1].psxFunction = kFn80040420;
    outPlan.actions[1].arg0 = kEvent17EndFrameClearR80040420;
    outPlan.actions[1].arg1 = kEvent17EndFrameClearG80040420;
    outPlan.actions[1].arg2 = kEvent17EndFrameClearB80040420;
    outPlan.actions[1].arg0Known = true;
    outPlan.actions[1].arg1Known = true;
    outPlan.actions[1].arg2Known = true;

    outPlan.actions[2].kind =
        Event17EndFrameActionKind8001EA00::SubmitWorkList80040CA4;
    outPlan.actions[2].psxFunction = kFn80040CA4;
    outPlan.actions[2].arg0 = input.workListAddress;
    outPlan.actions[2].arg0Known = true;
    outPlan.actions[2].arg1Known = false;
    outPlan.actions[2].arg2Known = false;

    // Event17 is outside the 8001EA00 clear exemptions (4, 1, 10), so the
    // formal clear action is bound above. Host execution still requires an
    // explicit translated graph-owner preflight and is intentionally absent;
    // the existing old S0 shell is not an owner or helper for this plan.
    outPlan.graphModelReady =
        input.workListReady && input.graphModelPreflightKnown &&
        input.graphModelWorkListSlotMatches;
    outPlan.formalOnly = !outPlan.graphModelReady;
    outPlan.graphModelApplied = false;
    outPlan.hostClearExecuted = false;
    outPlan.hostSubmitExecuted = false;
    outPlan.exactPsxHalParity = false;
    return true;
}

void ResetDispatcherPreLoopPadRelease80026B94(
    DispatcherPreLoopPadReleaseState80026B94& state)
{
    state = DispatcherPreLoopPadReleaseState80026B94{};
}

DispatcherPreLoopPadReleaseStep80026B94
StepDispatcherPreLoopPadRelease80026B94(
    DispatcherPreLoopPadReleaseState80026B94& state,
    uint32_t currentPadMask80035510)
{
    DispatcherPreLoopPadReleaseStep80026B94 step{};
    if (!state.waitingForRelease) {
        step.kind = DispatcherPreLoopPadReleaseStepKind80026B94::
            LoopAlreadyActive;
        step.inputClosed = false;
        return step;
    }
    if (currentPadMask80035510 != 0u) {
        return step;
    }

    state.waitingForRelease = false;
    step.kind =
        DispatcherPreLoopPadReleaseStepKind80026B94::ReleasedEnterLoop;
    step.inputClosed = false;
    return step;
}

void ResetDispatcherPadChange80026744(
    DispatcherPadChangeState80026744& state)
{
    state = DispatcherPadChangeState80026744{};
}

DispatcherPadChangeStep80026744 StepDispatcherPadChange80026744(
    DispatcherPadChangeState80026744& state,
    uint32_t currentPadMask80035510)
{
    DispatcherPadChangeStep80026744 step{};
    step.currentPadMask80035510 = currentPadMask80035510;
    step.previousPadMask80035510 = state.previousPadMask80035510;
    if (currentPadMask80035510 == state.previousPadMask80035510) {
        return step;
    }

    state.previousPadMask80035510 = currentPadMask80035510;
    step.changed = true;
    // 80026744 returns the new 80035510 value after a change. A transition
    // to the released state therefore records the change but returns zero.
    step.changedPadMask80026744 = currentPadMask80035510;
    return step;
}

void ResetEvent17InitialInputTimeout80026B94(
    Event17InitialInputTimeoutState80026B94& state)
{
    state = Event17InitialInputTimeoutState80026B94{};
}

Event17InitialInputTimeoutStep80026B94
StepEvent17InitialInputTimeout80026B94(
    Event17InitialInputTimeoutState80026B94& state,
    uint32_t padMask)
{
    Event17InitialInputTimeoutStep80026B94 step{};
    step.framesRemainingBefore = state.framesRemaining;
    step.framesRemainingAfter = state.framesRemaining;
    if (!state.initialInputPending || state.resultIssued) {
        return step;
    }

    // In 80026B94 any nonzero 80026744 result clears the initial-input
    // timeout latch before the handler is called, even when the handler later
    // rejects the input (for example because Event17 cooldown is active).
    if (padMask != 0u) {
        state.initialInputPending = false;
        step.kind =
            Event17InitialInputTimeoutStepKind80026B94::DisabledByInput;
        return step;
    }

    if (state.framesRemaining > 0) {
        --state.framesRemaining;
    }
    step.framesRemainingAfter = state.framesRemaining;
    if (state.framesRemaining <= 0) {
        state.initialInputPending = false;
        state.resultIssued = true;
        step.kind = Event17InitialInputTimeoutStepKind80026B94::TimeoutResult;
        step.result = kEvent17TimeoutResult80026B94;
        return step;
    }

    step.kind =
        Event17InitialInputTimeoutStepKind80026B94::CountedNoInputFrame;
    return step;
}

uint32_t KnownEventTableSpecCount()
{
    return sizeof(kEventTables) / sizeof(kEventTables[0]);
}

const EventTableSpec& KnownEventTableSpecAt(uint32_t index)
{
    if (index >= KnownEventTableSpecCount()) {
        index = KnownEventTableSpecCount() - 1u;
    }
    return kEventTables[index];
}

const EventTableSpec* FindEventTableSpec(int32_t eventId)
{
    for (uint32_t i = 0; i < KnownEventTableSpecCount(); ++i) {
        if (kEventTables[i].eventId == eventId) {
            return &kEventTables[i];
        }
    }
    return nullptr;
}

uint32_t KnownDrawRouteSpecCount()
{
    return sizeof(kDrawRoutes) / sizeof(kDrawRoutes[0]);
}

const DrawRouteSpec& KnownDrawRouteSpecAt(uint32_t index)
{
    if (index >= KnownDrawRouteSpecCount()) {
        index = KnownDrawRouteSpecCount() - 1u;
    }
    return kDrawRoutes[index];
}

const DrawRouteSpec* FindDrawRouteSpec(int32_t eventId)
{
    for (uint32_t i = 0; i < KnownDrawRouteSpecCount(); ++i) {
        if (kDrawRoutes[i].eventId == eventId) {
            return &kDrawRoutes[i];
        }
    }
    return nullptr;
}

EventFramePlan BuildDispatcher80026B94Plan(int32_t eventId,
                                            bool argKnown,
                                            uint32_t argAddress)
{
    const EventTableSpec* spec = FindEventTableSpec(eventId);
    if (spec == nullptr) {
        EventFramePlan plan =
            MakePlan("Dispatcher80026B94_UnknownEvent",
                     EventFrameOwner::Unknown,
                     eventId);
        AppendAction(plan, EventFrameActionKind::Gap, kFn80026B94,
                     eventId);
        return plan;
    }

    EventFramePlan plan =
        MakePlan(DispatcherPlanName(eventId), spec->owner, eventId);
    const uint32_t ctxAddress = ResolveCtx(*spec, argKnown, argAddress);

    AppendAction(plan,
                 EventFrameActionKind::SelectTable,
                 kFn80026B94,
                 eventId,
                 spec->tableAddress,
                 spec->defaultCtx,
                 argKnown ? argAddress : 0);
    AppendAction(plan,
                 EventFrameActionKind::CallInit,
                 spec->init,
                 eventId,
                 spec->tableAddress,
                 spec->defaultCtx,
                 argKnown ? argAddress : 0);

    if (spec->ctxOverriddenByArg) {
        AppendAction(plan,
                     EventFrameActionKind::GateArgCtxFromArg,
                     spec->init,
                     eventId,
                     spec->tableAddress,
                     ctxAddress,
                     argKnown ? argAddress : 0,
                     spec->defaultCtx,
                     argKnown ? 1 : 0,
                     0,
                     0,
                     0,
                     true);
        AppendAction(plan,
                     EventFrameActionKind::OverrideCtxFromArg,
                     spec->init,
                     eventId,
                     spec->tableAddress,
                     ctxAddress,
                     argKnown ? argAddress : 0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     !argKnown);
        if (!argKnown) {
            AppendAction(plan,
                         EventFrameActionKind::Gap,
                         kFn80026B94,
                         eventId,
                         spec->tableAddress,
                         0,
                         0,
                         spec->defaultCtx);
        }
        if (eventId == 6) {
            const bool knownHiScoreTable =
                argKnown && argAddress == kCtxEvent6HiScoreTable80049278;
            AppendAction(plan,
                         EventFrameActionKind::GateEvent6HiScoreArg80049278,
                         spec->init,
                         eventId,
                         spec->tableAddress,
                         ctxAddress,
                         argKnown ? argAddress : 0,
                         kCtxEvent6HiScoreTable80049278,
                         argKnown ? 1 : 0,
                         knownHiScoreTable ? 1 : 0,
                         0,
                         0,
                         true);
            if (argKnown && !knownHiScoreTable) {
                AppendAction(plan,
                             EventFrameActionKind::Gap,
                             kFn80026B94,
                             eventId,
                             spec->tableAddress,
                             ctxAddress,
                             argAddress,
                             kCtxEvent6HiScoreTable80049278);
            }
        }
    }

    if (eventId == 3) {
        AppendAction(plan,
                     EventFrameActionKind::PreTransition80020110,
                     kFn80020110,
                     eventId,
                     spec->tableAddress,
                     ctxAddress,
                     argKnown ? argAddress : 0,
                     0,
                     0,
                     4,
                     2,
                     1);
    }

    if (eventId == 10) {
        AppendAction(plan,
                     EventFrameActionKind::Gap,
                     kFn80026B94,
                     eventId,
                     spec->tableAddress,
                     ctxAddress);
        return plan;
    }

    AppendAction(plan,
                 EventFrameActionKind::WaitPadRelease80035510,
                 kFn80035510,
                 eventId,
                 spec->tableAddress,
                 ctxAddress);
    AppendAction(plan,
                 EventFrameActionKind::ResetLastPad,
                 kFn80026B94,
                 eventId,
                 spec->tableAddress,
                 ctxAddress);
    AppendAction(plan,
                 EventFrameActionKind::ReadPadChange80035510,
                 kFn80035510,
                 eventId,
                 spec->tableAddress,
                 ctxAddress);
    AppendAction(plan,
                 EventFrameActionKind::GatePadChangeSource80035510,
                 kFn80035510,
                 eventId,
                 spec->tableAddress,
                 ctxAddress,
                 0,
                 kPadLatchDword800882F0,
                 0,
                 0,
                 0,
                 0,
                 true);
    AppendAction(plan,
                 EventFrameActionKind::CallHandle,
                 spec->handle,
                 eventId,
                 spec->tableAddress,
                 ctxAddress,
                 argKnown ? argAddress : 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 true);

    if (eventId == 6) {
        AppendAction(plan,
                     EventFrameActionKind::CallInputSfx80025C8C,
                     kFn80025C8C,
                     eventId,
                     spec->tableAddress,
                     ctxAddress,
                     argKnown ? argAddress : 0,
                     0,
                     0x20,
                     0,
                     0,
                     0,
                     true);
    }

    if (spec->timeout != 0) {
        AppendAction(plan,
                     EventFrameActionKind::TimeoutCountdown,
                     kFn80026B94,
                     eventId,
                     spec->tableAddress,
                     ctxAddress,
                     0,
                     0,
                     static_cast<int32_t>(spec->timeout),
                     3,
                     0,
                     0,
                     true);
    }

    if (spec->tick != 0) {
        AppendAction(plan,
                     EventFrameActionKind::CallTick,
                     spec->tick,
                     eventId,
                     spec->tableAddress,
                     ctxAddress);
    }

    if (eventId == 4) {
        AppendAction(plan,
                     EventFrameActionKind::PlayCue80026EF8,
                     kFn80026EF8,
                     eventId,
                     spec->tableAddress,
                     ctxAddress,
                     0,
                     kCueEvent4_8009441C,
                     0,
                     0,
                     0,
                     0,
                     true);
        AppendAction(plan,
                     EventFrameActionKind::Flush80026ECC,
                     kFn80026ECC,
                     eventId,
                     spec->tableAddress,
                     ctxAddress,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     true);
    }

    AppendAction(plan,
                 EventFrameActionKind::DrawRoute8001E750,
                 kFn8001E750,
                 eventId,
                 spec->tableAddress,
                 ctxAddress);

    if (eventId == 2) {
        AppendAction(plan,
                     EventFrameActionKind::StageClearTextGap,
                     kFn80026B94,
                     eventId,
                     spec->tableAddress,
                     ctxAddress,
                     0,
                     0x800916F6u,
                     0,
                     0,
                     0,
                     0,
                     true);
    }

    AppendAction(plan,
                 EventFrameActionKind::WaitFrame80035560,
                 kFn80035560,
                 eventId,
                 spec->tableAddress,
                 ctxAddress,
                 0,
                 0,
                 0);
    AppendAction(plan,
                 EventFrameActionKind::EndFrame8001EA00,
                 kFn8001EA00,
                 eventId,
                 spec->tableAddress,
                 ctxAddress);
    AppendAction(plan,
                 EventFrameActionKind::TextFlush800436F0,
                 kFn800436F0,
                 eventId,
                 spec->tableAddress,
                 ctxAddress,
                 0,
                 0,
                 -1);
    AppendAction(plan,
                 EventFrameActionKind::TailFrames60,
                 kFn80026B94,
                 eventId,
                 spec->tableAddress,
                 ctxAddress,
                 0,
                 0,
                 60);
    AppendAction(plan,
                 EventFrameActionKind::ReturnResult,
                 kFn80026B94,
                 eventId,
                 spec->tableAddress,
                 ctxAddress);
    return plan;
}

EventFramePlan BuildDrawRoute8001E750Plan(int32_t eventId)
{
    const DrawRouteSpec* route = FindDrawRouteSpec(eventId);
    if (route == nullptr) {
        EventFramePlan plan =
            MakePlan("DrawRoute8001E750_UnknownEvent",
                     EventFrameOwner::Unknown,
                     eventId);
        AppendAction(plan, EventFrameActionKind::Gap, kFn8001E750,
                     eventId);
        return plan;
    }

    EventFramePlan plan =
        MakePlan(route->name, route->owner, route->eventId);
    if (route->workSlot >= 0) {
        AppendAction(plan,
                     EventFrameActionKind::PrepareDrawWork8001D74C,
                     kFn8001D74C,
                     route->eventId,
                     0,
                     0,
                     0,
                     0,
                     route->workSlot);
    }
    AppendAction(plan,
                 EventFrameActionKind::DrawRoute8001E750,
                 kFn8001E750,
                 route->eventId,
                 0,
                 0,
                 0,
                 route->callee,
                 route->typeArg);

    if (route->eventId == 4) {
        AppendAction(plan,
                     EventFrameActionKind::GateEvent4DrawWrapperSource8001E750,
                     kFn8001E750,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kGpEvent4WrapperStateOffset,
                     0,
                     1,
                     2,
                     static_cast<int32_t>(kCueEvent4_8009441C),
                     true);
        AppendAction(plan,
                     EventFrameActionKind::Event4PromptDraw800203D4,
                     kFn800203D4,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kGpEvent4WrapperStateOffset,
                     0);
        AppendAction(plan,
                     EventFrameActionKind::Event4Backdrop8001B6C4,
                     kFn8001B6C4,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kGpEvent4WrapperStateOffset,
                     1);
        AppendAction(plan,
                     EventFrameActionKind::Event4BackdropBoxFillLocal8001B6C4,
                     kFn8001B6C4,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kGpEvent4WrapperStateOffset,
                     kEvent4BackdropBoxFillX8001B6C4,
                     kEvent4BackdropBoxFillY8001B6C4,
                     kEvent4BackdropBoxFillW8001B6C4,
                     kEvent4BackdropBoxFillH8001B6C4,
                     true);
        AppendAction(plan,
                     EventFrameActionKind::Event4BackdropGsSortBoxFill8003EE84,
                     kFn8003EE84,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kFn8001B6C4,
                     static_cast<int32_t>(kEvent4BackdropBoxFillAttr8001B6C4),
                     kEvent4BackdropBoxFillPriority8001B6C4,
                     5,
                     0,
                     true);
        AppendAction(plan,
                     EventFrameActionKind::Event4MoveImage8001B120,
                     kFn8001B120,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kGpEvent4WrapperStateOffset,
                     2,
                     0);
        AppendAction(plan,
                     EventFrameActionKind::Event4MoveImageSlotSource8004019C,
                     kFn8004019C,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kGpEvent4WrapperStateOffset,
                     0,
                     1,
                     0,
                     0,
                     true);
        AppendAction(plan,
                     EventFrameActionKind::Event4MoveImageRect8001B120,
                     kFn8001B120,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kGpEvent4WrapperStateOffset,
                     0,
                     kEvent4MoveImageW8001B120,
                     kEvent4MoveImageH8001B120,
                     1,
                     true);
        AppendAction(plan,
                     EventFrameActionKind::Event4MoveImageDispatch80044E2C,
                     kFn80044E2C,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kFn800468E0,
                     0,
                     0,
                     0xF0,
                     1,
                     true);
        AppendAction(plan,
                     EventFrameActionKind::Event4FrameSubmitHalGap,
                     kFn800468E0,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kFn80046840,
                     0,
                     0,
                     0,
                     0,
                     true);
        AppendAction(plan,
                     EventFrameActionKind::Gap,
                     kFn8001E750,
                     route->eventId,
                     0,
                     kCtxEvent4_8006ED74,
                     0,
                     kGpEvent4WrapperStateOffset);
    }

    if (route->drawOnly) {
        AppendAction(plan,
                     route->eventId == 16
                         ? EventFrameActionKind::PracticeSelfLoop
                         : EventFrameActionKind::CardDrawIdOnly,
                     route->callee,
                     route->eventId,
                     0,
                     0,
                     0,
                     0,
                     route->typeArg);
    }
    return plan;
}

EventFramePlan BuildPracticeSelfLoop8002776CPlan(uint32_t menuCtx,
                                                  int32_t prevScene)
{
    EventFramePlan plan =
        MakePlan("PracticeSelfLoop8002776C",
                 EventFrameOwner::PracticeSelfLoop,
                 16);
    const bool ctxSourceKnown = menuCtx == kScene0WorkAddress;

    AppendAction(plan,
                 EventFrameActionKind::GatePracticeSelfLoopSource8002776C,
                 kFn8002776C,
                 16,
                 0,
                 ctxSourceKnown ? menuCtx : 0u,
                 0,
                 0,
                 static_cast<int32_t>(menuCtx),
                 prevScene,
                 static_cast<int32_t>(kScene0WorkAddress),
                 ctxSourceKnown ? 1 : 0,
                 true);
    if (!ctxSourceKnown) {
        AppendAction(plan,
                     EventFrameActionKind::Gap,
                     kFn8002776C,
                     16,
                     0,
                     menuCtx,
                     0,
                     0,
                     prevScene,
                     static_cast<int32_t>(kScene0WorkAddress));
    }
    AppendAction(plan,
                 EventFrameActionKind::PracticeSelfLoop,
                 kFn8002776C,
                 16,
                 0,
                 menuCtx,
                 0,
                 0,
                 prevScene);
    AppendAction(plan,
                 EventFrameActionKind::DrawRoute8001E750,
                 kFn8001E750,
                 16,
                 0,
                 menuCtx,
                 0,
                 kFn80023618);
    AppendAction(plan,
                 EventFrameActionKind::WaitFrame80035560,
                 kFn80035560,
                 16,
                 0,
                 menuCtx,
                 0,
                 0,
                 2);
    AppendAction(plan,
                 EventFrameActionKind::EndFrame8001EA00,
                 kFn8001EA00,
                 16,
                 0,
                 menuCtx,
                 0,
                 0,
                 0);
    AppendAction(plan,
                 EventFrameActionKind::PracticeReload80015590,
                 kFn80015590,
                 16,
                 0,
                 menuCtx,
                 0,
                 0,
                 prevScene);
    AppendAction(plan,
                 EventFrameActionKind::Gap,
                 kFn8002776C,
                 16,
                 0,
                 menuCtx);
    return plan;
}

EventFramePlan BuildCardDrawIds80020F94Plan()
{
    EventFramePlan plan =
        MakePlan("CardDrawIds80020F94",
                 EventFrameOwner::DrawOnly,
                 0);
    for (int32_t eventId = 7; eventId <= 9; ++eventId) {
        AppendAction(plan,
                     EventFrameActionKind::CardDrawIdOnly,
                     kFn80020F94,
                     eventId);
        AppendAction(plan,
                     EventFrameActionKind::DrawRoute8001E750,
                     kFn8001E750,
                     eventId,
                     0,
                     0,
                     0,
                     kFn80020F94);
    }
    AppendAction(plan, EventFrameActionKind::Gap, kFn80020F94, 0);
    return plan;
}

const char* EventFrameOwnerName(EventFrameOwner owner)
{
    switch (owner) {
    case EventFrameOwner::Unknown:
        return "Unknown";
    case EventFrameOwner::DispatcherLoop:
        return "DispatcherLoop";
    case EventFrameOwner::ModalPrompt:
        return "ModalPrompt";
    case EventFrameOwner::DrawOnly:
        return "DrawOnly";
    case EventFrameOwner::PracticeSelfLoop:
        return "PracticeSelfLoop";
    case EventFrameOwner::Placeholder:
        return "Placeholder";
    }
    return "Unknown";
}

const char* EventFrameActionKindName(EventFrameActionKind kind)
{
    switch (kind) {
    case EventFrameActionKind::None:
        return "None";
    case EventFrameActionKind::SelectTable:
        return "SelectTable";
    case EventFrameActionKind::CallInit:
        return "CallInit";
    case EventFrameActionKind::GateArgCtxFromArg:
        return "GateArgCtxFromArg";
    case EventFrameActionKind::OverrideCtxFromArg:
        return "OverrideCtxFromArg";
    case EventFrameActionKind::GateEvent6HiScoreArg80049278:
        return "GateEvent6HiScoreArg80049278";
    case EventFrameActionKind::PreTransition80020110:
        return "PreTransition80020110";
    case EventFrameActionKind::WaitPadRelease80035510:
        return "WaitPadRelease80035510";
    case EventFrameActionKind::ResetLastPad:
        return "ResetLastPad";
    case EventFrameActionKind::ReadPadChange80035510:
        return "ReadPadChange80035510";
    case EventFrameActionKind::GatePadChangeSource80035510:
        return "GatePadChangeSource80035510";
    case EventFrameActionKind::CallHandle:
        return "CallHandle";
    case EventFrameActionKind::CallTick:
        return "CallTick";
    case EventFrameActionKind::TimeoutCountdown:
        return "TimeoutCountdown";
    case EventFrameActionKind::PrepareDrawWork8001D74C:
        return "PrepareDrawWork8001D74C";
    case EventFrameActionKind::DrawRoute8001E750:
        return "DrawRoute8001E750";
    case EventFrameActionKind::WaitFrame80035560:
        return "WaitFrame80035560";
    case EventFrameActionKind::EndFrame8001EA00:
        return "EndFrame8001EA00";
    case EventFrameActionKind::TextFlush800436F0:
        return "TextFlush800436F0";
    case EventFrameActionKind::TailFrames60:
        return "TailFrames60";
    case EventFrameActionKind::CallInputSfx80025C8C:
        return "CallInputSfx80025C8C";
    case EventFrameActionKind::PlayCue80026EF8:
        return "PlayCue80026EF8";
    case EventFrameActionKind::Flush80026ECC:
        return "Flush80026ECC";
    case EventFrameActionKind::StageClearTextGap:
        return "StageClearTextGap";
    case EventFrameActionKind::GateEvent4DrawWrapperSource8001E750:
        return "GateEvent4DrawWrapperSource8001E750";
    case EventFrameActionKind::Event4PromptDraw800203D4:
        return "Event4PromptDraw800203D4";
    case EventFrameActionKind::Event4Backdrop8001B6C4:
        return "Event4Backdrop8001B6C4";
    case EventFrameActionKind::Event4BackdropBoxFillLocal8001B6C4:
        return "Event4BackdropBoxFillLocal8001B6C4";
    case EventFrameActionKind::Event4BackdropGsSortBoxFill8003EE84:
        return "Event4BackdropGsSortBoxFill8003EE84";
    case EventFrameActionKind::Event4MoveImage8001B120:
        return "Event4MoveImage8001B120";
    case EventFrameActionKind::Event4MoveImageSlotSource8004019C:
        return "Event4MoveImageSlotSource8004019C";
    case EventFrameActionKind::Event4MoveImageRect8001B120:
        return "Event4MoveImageRect8001B120";
    case EventFrameActionKind::Event4MoveImageDispatch80044E2C:
        return "Event4MoveImageDispatch80044E2C";
    case EventFrameActionKind::Event4FrameSubmitHalGap:
        return "Event4FrameSubmitHalGap";
    case EventFrameActionKind::GatePracticeSelfLoopSource8002776C:
        return "GatePracticeSelfLoopSource8002776C";
    case EventFrameActionKind::PracticeSelfLoop:
        return "PracticeSelfLoop";
    case EventFrameActionKind::PracticeReload80015590:
        return "PracticeReload80015590";
    case EventFrameActionKind::CardDrawIdOnly:
        return "CardDrawIdOnly";
    case EventFrameActionKind::ReturnResult:
        return "ReturnResult";
    case EventFrameActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0EventFrameLoopDirect
