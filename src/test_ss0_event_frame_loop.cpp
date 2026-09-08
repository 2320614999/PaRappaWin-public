#include "pr/pr_ss0_event_frame_loop_direct.h"

#include <cstdio>
#include <initializer_list>

using namespace PrSS0EventFrameLoopDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

const EventFrameAction* FindAction(const EventFramePlan& plan,
                                   EventFrameActionKind kind,
                                   uint32_t start = 0) {
    for (uint32_t i = start; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            return &plan.actions[i];
        }
    }
    return nullptr;
}

uint32_t CountActions(const EventFramePlan& plan,
                      EventFrameActionKind kind) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            ++count;
        }
    }
    return count;
}

uint32_t IndexOfAction(const EventFramePlan& plan,
                       EventFrameActionKind kind,
                       uint32_t start = 0) {
    for (uint32_t i = start; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            return i;
        }
    }
    return plan.count;
}

struct Event4ModalAudioProbe {
    bool sfxResult = true;
    bool cueResult = true;
    bool flushResult = true;
    uint32_t sfxCalls = 0u;
    uint32_t cueCalls = 0u;
    uint32_t flushCalls = 0u;
    uint32_t lastSfxMask = 0u;
    uint32_t lastCueAddress = 0u;
};

bool ProbeEvent4ModalSfx(uint32_t inputMask, void* userData) {
    auto* probe = static_cast<Event4ModalAudioProbe*>(userData);
    ++probe->sfxCalls;
    probe->lastSfxMask = inputMask;
    return probe->sfxResult;
}

bool ProbeEvent4ModalCue(uint32_t cueAddress, void* userData) {
    auto* probe = static_cast<Event4ModalAudioProbe*>(userData);
    ++probe->cueCalls;
    probe->lastCueAddress = cueAddress;
    return probe->cueResult;
}

bool ProbeEvent4ModalFlush(void* userData) {
    auto* probe = static_cast<Event4ModalAudioProbe*>(userData);
    ++probe->flushCalls;
    return probe->flushResult;
}

void CheckOrder(const EventFramePlan& plan,
                const EventFrameActionKind* kinds,
                uint32_t count) {
    uint32_t start = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t index = IndexOfAction(plan, kinds[i], start);
        CHECK(index < plan.count);
        start = index + 1;
    }
}

void TestRuntimeGateFalse() {
    CHECK(!RuntimeCutoverAllowed());
    const EventFramePlan plan = BuildDispatcher80026B94Plan(3);
    CHECK(!plan.runtimeCutoverAllowed);
    CHECK(plan.blockedByP0Gap);
}

void TestSpecTablesAndDrawRoutes() {
    CHECK(KnownEventTableSpecCount() == 6);
    CHECK(KnownDrawRouteSpecCount() == 17);

    const EventTableSpec* mainDirectory = FindEventTableSpec(3);
    CHECK(mainDirectory != nullptr);
    CHECK(mainDirectory->tableAddress == kEventTable3_80054550);
    CHECK(mainDirectory->init == kFn80026794);
    CHECK(mainDirectory->handle == kFn800264AC);
    CHECK(mainDirectory->tick == kFn80026720);
    CHECK(mainDirectory->defaultCtx == kCtxEvent3_800544F8);
    CHECK(mainDirectory->owner == EventFrameOwner::DispatcherLoop);

    const EventTableSpec* event6 = FindEventTableSpec(6);
    CHECK(event6 != nullptr);
    CHECK(event6->ctxOverriddenByArg);
    CHECK(event6->defaultCtx == kCtxEvent6Default_8005424C);

    const EventTableSpec* event17 = FindEventTableSpec(17);
    CHECK(event17 != nullptr);
    CHECK(event17->tableAddress == kEventTable17_800545A0);
    CHECK(event17->init == kFn800268E4);
    CHECK(event17->handle == kFn80026910);
    CHECK(event17->tick == kFn80026B54);
    CHECK(event17->timeout ==
          static_cast<uint32_t>(kEvent17InitialInputTimeoutFrames80026B94));
    CHECK(event17->defaultCtx == kCtxEvent17_8005451C);

    const EventTableSpec* placeholder = FindEventTableSpec(10);
    CHECK(placeholder != nullptr);
    CHECK(placeholder->handle == 0);
    CHECK(placeholder->tick == 0);
    CHECK(placeholder->owner == EventFrameOwner::Placeholder);

    const DrawRouteSpec* practice = FindDrawRouteSpec(16);
    CHECK(practice != nullptr);
    CHECK(practice->callee == kFn80023618);
    CHECK(practice->workSlot == 3);
    CHECK(practice->drawOnly);
    CHECK(practice->owner == EventFrameOwner::PracticeSelfLoop);

    const DrawRouteSpec* saveGrid = FindDrawRouteSpec(7);
    CHECK(saveGrid != nullptr);
    CHECK(saveGrid->callee == kFn80020F94);
    CHECK(saveGrid->drawOnly);
    CHECK(saveGrid->owner == EventFrameOwner::DrawOnly);
}

void TestEventTableSpecsMatchCurrentIdaDispatcher() {
    struct ExpectedEventTable {
        int32_t eventId;
        uint32_t tableAddress;
        uint32_t init;
        uint32_t handle;
        uint32_t tick;
        uint32_t timeout;
        uint32_t defaultCtx;
        bool ctxOverriddenByArg;
        EventFrameOwner owner;
    };

    const ExpectedEventTable expected[] = {
        {2, kEventTable2_8005453C, kFn800267F8, kFn80025F6C,
         kFn80026170, 0u, kCtxEvent2_80087B78, false,
         EventFrameOwner::DispatcherLoop},
        {3, kEventTable3_80054550, kFn80026794, kFn800264AC,
         kFn80026720, 0u, kCtxEvent3_800544F8, false,
         EventFrameOwner::DispatcherLoop},
        {4, kEventTable4_80054564, kFn800267C8, kFn80025F0C,
         kFn80025E6C, 0u, kCtxEvent4_8006ED74, false,
         EventFrameOwner::ModalPrompt},
        {6, kEventTable6_80054578, kFn800267E4, kFn80025E0C,
         kFn80025E48, 0u, kCtxEvent6Default_8005424C, true,
         EventFrameOwner::DispatcherLoop},
        {10, kEventTable10_8005458C, kFn800267F0, 0u, 0u, 0u, 0u, false,
         EventFrameOwner::Placeholder},
        {17, kEventTable17_800545A0, kFn800268E4, kFn80026910,
         kFn80026B54,
         static_cast<uint32_t>(kEvent17InitialInputTimeoutFrames80026B94),
         kCtxEvent17_8005451C, false, EventFrameOwner::DispatcherLoop},
    };

    CHECK(KnownEventTableSpecCount() ==
          sizeof(expected) / sizeof(expected[0]));
    for (const ExpectedEventTable& row : expected) {
        const EventTableSpec* actual = FindEventTableSpec(row.eventId);
        CHECK(actual != nullptr);
        if (actual == nullptr) {
            continue;
        }
        CHECK(actual->tableAddress == row.tableAddress);
        CHECK(actual->init == row.init);
        CHECK(actual->handle == row.handle);
        CHECK(actual->tick == row.tick);
        CHECK(actual->timeout == row.timeout);
        CHECK(actual->defaultCtx == row.defaultCtx);
        CHECK(actual->ctxOverriddenByArg == row.ctxOverriddenByArg);
        CHECK(actual->owner == row.owner);
    }
}

void TestDispatcherMainDirectoryTail() {
    const EventFramePlan plan = BuildDispatcher80026B94Plan(3);
    CHECK(plan.owner == EventFrameOwner::DispatcherLoop);
    CHECK(plan.eventId == 3);
    CHECK(!plan.truncated);

    const EventFrameActionKind expected[] = {
        EventFrameActionKind::SelectTable,
        EventFrameActionKind::CallInit,
        EventFrameActionKind::PreTransition80020110,
        EventFrameActionKind::WaitPadRelease80035510,
        EventFrameActionKind::ResetLastPad,
        EventFrameActionKind::ReadPadChange80035510,
        EventFrameActionKind::GatePadChangeSource80035510,
        EventFrameActionKind::CallHandle,
        EventFrameActionKind::CallTick,
        EventFrameActionKind::DrawRoute8001E750,
        EventFrameActionKind::WaitFrame80035560,
        EventFrameActionKind::EndFrame8001EA00,
        EventFrameActionKind::TextFlush800436F0,
        EventFrameActionKind::TailFrames60,
        EventFrameActionKind::ReturnResult,
    };
    CheckOrder(plan, expected, sizeof(expected) / sizeof(expected[0]));

    CHECK(plan.actions[0].psxFunction == kFn80026B94);
    CHECK(plan.actions[0].tableAddress == kEventTable3_80054550);
    CHECK(plan.actions[0].ctxAddress == kCtxEvent3_800544F8);

    const EventFrameAction* preTransition =
        FindAction(plan, EventFrameActionKind::PreTransition80020110);
    CHECK(preTransition != nullptr);
    CHECK(preTransition->psxFunction == kFn80020110);
    CHECK(preTransition->args[1] == 4);
    CHECK(preTransition->args[2] == 2);
    CHECK(preTransition->args[3] == 1);

    const EventFrameAction* padGate =
        FindAction(plan, EventFrameActionKind::GatePadChangeSource80035510);
    CHECK(padGate != nullptr);
    CHECK(padGate->auxAddress == kPadLatchDword800882F0);
    CHECK(padGate->conditional);

    const EventFrameAction* tail =
        FindAction(plan, EventFrameActionKind::TailFrames60);
    CHECK(tail != nullptr);
    CHECK(tail->args[0] == 60);
}

void TestDispatcherResultTailRuntime() {
    DispatcherTailState80026B94 state{};
    DispatcherTailStep80026B94 step = StepDispatcherTail80026B94(state);
    CHECK(step.kind == DispatcherTailStepKind80026B94::Inactive);
    CHECK(!step.inputClosed);
    CHECK(!state.active);

    CHECK(!ArmDispatcherTail80026B94(state, 0));
    CHECK(ArmDispatcherTail80026B94(state, 4));
    CHECK(state.active);
    CHECK(state.latchedResult == 4);
    CHECK(state.framesRemaining == kDispatcherResultTailFrames80026B94);
    CHECK(!ArmDispatcherTail80026B94(state, 7));
    CHECK(state.latchedResult == 4);

    int stepCount = 0;
    for (int expectedBefore = kDispatcherResultTailFrames80026B94;
         expectedBefore > 0;
         --expectedBefore) {
        step = StepDispatcherTail80026B94(state);
        ++stepCount;
        CHECK(step.kind == DispatcherTailStepKind80026B94::HoldFrame);
        CHECK(step.inputClosed);
        CHECK(step.releasedResult == 0);
        CHECK(step.framesRemainingBefore == expectedBefore);
        CHECK(step.framesRemainingAfter == expectedBefore - 1);
        CHECK(state.active);
    }
    CHECK(state.framesRemaining == 0);

    step = StepDispatcherTail80026B94(state);
    ++stepCount;
    CHECK(step.kind == DispatcherTailStepKind80026B94::ReleaseResult);
    CHECK(step.inputClosed);
    CHECK(step.releasedResult == 4);
    CHECK(step.framesRemainingBefore == 0);
    CHECK(step.framesRemainingAfter == 0);
    CHECK(stepCount == kDispatcherResultTailFrames80026B94 + 1);
    CHECK(!state.active);
    CHECK(state.latchedResult == 0);
    CHECK(state.framesRemaining == 0);

    step = StepDispatcherTail80026B94(state);
    CHECK(step.kind == DispatcherTailStepKind80026B94::Inactive);
    CHECK(step.releasedResult == 0);

    CHECK(ArmDispatcherTail80026B94(state, 8));
    (void)StepDispatcherTail80026B94(state);
    ResetDispatcherTail80026B94(state);
    CHECK(!state.active);
    CHECK(state.latchedResult == 0);
    CHECK(state.framesRemaining == 0);
}

void TestEvent17InitialInputTimeoutRuntime() {
    DispatcherPreLoopPadReleaseState80026B94 releaseGate{};
    CHECK(releaseGate.waitingForRelease);
    for (const uint32_t heldMask : {
             0x0001u, 0x0002u, 0x0004u, 0x0008u,
             0x0010u, 0x0020u, 0x0040u, 0x0080u,
             0x0100u, 0x0800u,
             0x1000u, 0x2000u, 0x4000u, 0x8000u}) {
        const auto waiting = StepDispatcherPreLoopPadRelease80026B94(
            releaseGate, heldMask);
        CHECK(waiting.kind ==
              DispatcherPreLoopPadReleaseStepKind80026B94::
                  WaitingForRelease);
        CHECK(waiting.inputClosed);
        CHECK(releaseGate.waitingForRelease);
    }
    auto release =
        StepDispatcherPreLoopPadRelease80026B94(releaseGate, 0u);
    CHECK(release.kind ==
          DispatcherPreLoopPadReleaseStepKind80026B94::ReleasedEnterLoop);
    CHECK(!release.inputClosed);
    CHECK(!releaseGate.waitingForRelease);
    release = StepDispatcherPreLoopPadRelease80026B94(releaseGate, 0x0800u);
    CHECK(release.kind ==
          DispatcherPreLoopPadReleaseStepKind80026B94::LoopAlreadyActive);
    CHECK(!release.inputClosed);
    ResetDispatcherPreLoopPadRelease80026B94(releaseGate);
    CHECK(releaseGate.waitingForRelease);

    Event17InitialInputTimeoutState80026B94 state{};
    CHECK(state.initialInputPending);
    CHECK(!state.resultIssued);
    CHECK(state.framesRemaining ==
          kEvent17InitialInputTimeoutFrames80026B94);

    Event17InitialInputTimeoutStep80026B94 step{};
    for (int frame = 0;
         frame < kEvent17InitialInputTimeoutFrames80026B94 - 1;
         ++frame) {
        step = StepEvent17InitialInputTimeout80026B94(state, 0u);
        CHECK(step.kind == Event17InitialInputTimeoutStepKind80026B94::
                               CountedNoInputFrame);
        CHECK(step.result == 0);
        CHECK(step.framesRemainingBefore ==
              kEvent17InitialInputTimeoutFrames80026B94 - frame);
        CHECK(step.framesRemainingAfter ==
              kEvent17InitialInputTimeoutFrames80026B94 - frame - 1);
        CHECK(state.initialInputPending);
        CHECK(!state.resultIssued);
    }
    CHECK(state.framesRemaining == 1);

    step = StepEvent17InitialInputTimeout80026B94(state, 0u);
    CHECK(step.kind ==
          Event17InitialInputTimeoutStepKind80026B94::TimeoutResult);
    CHECK(step.result == kEvent17TimeoutResult80026B94);
    CHECK(step.framesRemainingBefore == 1);
    CHECK(step.framesRemainingAfter == 0);
    CHECK(!state.initialInputPending);
    CHECK(state.resultIssued);

    step = StepEvent17InitialInputTimeout80026B94(state, 0u);
    CHECK(step.kind == Event17InitialInputTimeoutStepKind80026B94::Inactive);
    CHECK(step.result == 0);

    ResetEvent17InitialInputTimeout80026B94(state);
    step = StepEvent17InitialInputTimeout80026B94(state, 0x0800u);
    CHECK(step.kind ==
          Event17InitialInputTimeoutStepKind80026B94::DisabledByInput);
    CHECK(!state.initialInputPending);
    CHECK(!state.resultIssued);
    CHECK(state.framesRemaining ==
          kEvent17InitialInputTimeoutFrames80026B94);
    for (int frame = 0;
         frame < kEvent17InitialInputTimeoutFrames80026B94 + 60;
         ++frame) {
        step = StepEvent17InitialInputTimeout80026B94(state, 0u);
        CHECK(step.kind ==
              Event17InitialInputTimeoutStepKind80026B94::Inactive);
        CHECK(step.result == 0);
    }

    ResetEvent17InitialInputTimeout80026B94(state);
    for (int frame = 0;
         frame < kEvent17InitialInputTimeoutFrames80026B94 - 1;
         ++frame) {
        (void)StepEvent17InitialInputTimeout80026B94(state, 0u);
    }
    CHECK(state.framesRemaining == 1);
    // Any nonzero input disables the latch before handler semantics, including
    // an otherwise unsupported input or one rejected by Event17 cooldown.
    step = StepEvent17InitialInputTimeout80026B94(state, 0x2000u);
    CHECK(step.kind ==
          Event17InitialInputTimeoutStepKind80026B94::DisabledByInput);
    CHECK(state.framesRemaining == 1);
    step = StepEvent17InitialInputTimeout80026B94(state, 0u);
    CHECK(step.kind == Event17InitialInputTimeoutStepKind80026B94::Inactive);
    CHECK(step.result == 0);

    const int results[] = {1, kEvent17TimeoutResult80026B94};
    for (const int result : results) {
        DispatcherTailState80026B94 tail{};
        CHECK(ArmDispatcherTail80026B94(tail, result));
        // Arming occurs on the handler/timeout action frame. It must not spend
        // one of the following 60 closed-input tail frames.
        CHECK(tail.framesRemaining == kDispatcherResultTailFrames80026B94);
        for (int frame = 0; frame < kDispatcherResultTailFrames80026B94;
             ++frame) {
            const auto held = StepDispatcherTail80026B94(tail);
            CHECK(held.kind == DispatcherTailStepKind80026B94::HoldFrame);
            CHECK(held.inputClosed);
        }
        const auto release = StepDispatcherTail80026B94(tail);
        CHECK(release.kind ==
              DispatcherTailStepKind80026B94::ReleaseResult);
        CHECK(release.releasedResult == result);
        CHECK(!tail.active);
    }
}

void TestEvent17FrameTransactionRuntime() {
    Event17FrameTransaction80026B94 state{};
    Event17FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = kCtxEvent17_8005451C;
    begin.logicFrame = 100u;
    begin.frameClass = Event17FrameClass80026B94::OpenInputLoop;

    Event17FrameTransactionBegin80026B94 invalid = begin;
    invalid.requestBound = false;
    CHECK(!BeginEvent17FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.ctxAddress = 0x12345678u;
    CHECK(!BeginEvent17FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.inputClosed = true;
    CHECK(!BeginEvent17FrameTransaction80026B94(state, invalid));

    CHECK(BeginEvent17FrameTransaction80026B94(state, begin));
    CHECK(state.active);
    CHECK(!state.directDrawCompleteWithinLimits);
    CHECK(state.commandCount == kEvent17FrameCommandCount80026B94);
    CHECK(state.commands[0].kind ==
          Event17FrameCommandKind80026B94::Tick80026B54);
    CHECK(state.commands[0].psxFunction == kFn80026B54);
    CHECK(static_cast<uint32_t>(state.commands[0].arg0) ==
          kEventTable17_800545A0);
    CHECK(state.commands[0].arg1Known);
    CHECK(state.commands[0].arg1 == kCtxEvent17_8005451C);
    CHECK(state.commands[1].kind ==
          Event17FrameCommandKind80026B94::DrawRoute8001E750);
    CHECK(state.commands[1].psxFunction == kFn8001E750);
    CHECK(state.commands[1].arg0 == 17);
    CHECK(state.commands[1].arg1Known);
    CHECK(state.commands[1].arg1 == kCtxEvent17_8005451C);
    CHECK(state.commands[2].kind ==
          Event17FrameCommandKind80026B94::PrepareDrawWork8001D74C);
    CHECK(state.commands[2].psxFunction == kFn8001D74C);
    CHECK(state.commands[2].arg0 == 4);
    CHECK(!state.commands[2].arg1Known);
    CHECK(state.commands[3].kind ==
          Event17FrameCommandKind80026B94::DrawPage80021910);
    CHECK(state.commands[3].psxFunction == kFn80021910);
    CHECK(static_cast<uint32_t>(state.commands[3].arg0) ==
          kCtxEvent17_8005451C);
    CHECK(state.commands[4].psxFunction == kFn80035560);
    CHECK(state.commands[4].arg0 == 0);
    CHECK(state.commands[5].psxFunction == kFn8001EA00);
    CHECK(state.commands[5].arg0 == 17);
    CHECK(state.commands[6].psxFunction == kFn800436F0);
    CHECK(state.commands[6].arg0 == -1);

    CHECK(!CanSubmitEvent17FrameDraw8001E750(state, 100u, false));
    CHECK(!CommitEvent17FrameDrawAndBindFormalTail80026B94(
        state, 100u, false));
    CHECK(!CommitEvent17FrameTick80026B54(
        state, 100u, kEventTable3_80054550, kCtxEvent17_8005451C));
    CHECK(CommitEvent17FrameTick80026B54(
        state, 100u, kEventTable17_800545A0, kCtxEvent17_8005451C));
    CHECK(!CommitEvent17FrameTick80026B54(
        state, 100u, kEventTable17_800545A0, kCtxEvent17_8005451C));
    CHECK(CanSubmitEvent17FrameDraw8001E750(state, 100u, false));
    CHECK(CommitEvent17FrameDrawAndBindFormalTail80026B94(
        state, 100u, false));
    CHECK(state.directDrawSubmitted);
    CHECK(state.formalTailOrderBound);
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(!state.hostWaitExecuted);
    CHECK(!state.hostEndExecuted);
    CHECK(!state.hostTextFlushExecuted);
    CHECK(!state.exactPsxHalParity);
    CHECK(!CommitEvent17FrameHostBoundary80026B94(state, true, true, false));
    CHECK(state.hostWaitExecuted);
    CHECK(state.hostEndExecuted);
    CHECK(!state.hostTextFlushExecuted);
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(CommitEvent17FrameHostBoundary80026B94(state, true, true, true));
    CHECK(state.hostWaitExecuted);
    CHECK(state.hostEndExecuted);
    CHECK(state.hostTextFlushExecuted);
    CHECK(state.hostFrameBoundaryExecuted);
    CHECK(!state.exactPsxHalParity);
    CHECK(!CommitEvent17FrameHostBoundary80026B94(state, true, true, true));
    CHECK(state.directDrawCompleteWithinLimits);
    CHECK(state.firstHostPresentationFrame == 100u);
    CHECK(state.hostPresentationSubmitCount == 1u);
    CHECK(!CanSubmitEvent17FrameDraw8001E750(state, 100u, false));
    CHECK(CanSubmitEvent17FrameDraw8001E750(state, 100u, true));
    CHECK(CommitEvent17FrameDrawAndBindFormalTail80026B94(
        state, 100u, true));
    CHECK(state.hostPresentationSubmitCount == 2u);

    Event17FrameTransactionBegin80026B94 action = begin;
    action.logicFrame = 101u;
    action.frameClass = Event17FrameClass80026B94::ResultAction;
    action.tailFramesRemainingBefore =
        kDispatcherResultTailFrames80026B94;
    action.tailFramesRemainingAfter = kDispatcherResultTailFrames80026B94;
    CHECK(BeginEvent17FrameTransaction80026B94(state, action));
    CHECK(state.frameClass == Event17FrameClass80026B94::ResultAction);
    CHECK(!state.inputClosed);
    CHECK(state.tailFramesRemainingBefore == 60);

    Event17FrameTransactionBegin80026B94 nextOpen = begin;
    nextOpen.logicFrame = 102u;
    CHECK(!BeginEvent17FrameTransaction80026B94(state, nextOpen));
    CHECK(CommitEvent17FrameTick80026B54(
        state, 101u, kEventTable17_800545A0, kCtxEvent17_8005451C));
    CHECK(CanSubmitEvent17FrameDraw8001E750(state, 103u, false));
    CHECK(CommitEvent17FrameDrawAndBindFormalTail80026B94(
        state, 103u, false));
    CHECK(state.logicFrame == 101u);
    CHECK(state.firstHostPresentationFrame == 103u);
    CHECK(!CanAdvanceEvent17FrameLogic80026B94(state));
    CHECK(CommitEvent17FrameHostBoundary80026B94(state, true, true, true));
    CHECK(CanAdvanceEvent17FrameLogic80026B94(state));
    CHECK(CanSubmitEvent17FrameDraw8001E750(state, 103u, true));
    CHECK(CommitEvent17FrameDrawAndBindFormalTail80026B94(
        state, 103u, true));
    CHECK(state.hostPresentationSubmitCount == 2u);
    CHECK(!CanSubmitEvent17FrameDraw8001E750(state, 104u, true));

    // An unfinished translated draw freezes both logic and the candidate tail;
    // a later host frame can complete the same request without advancing it.
    ResetEvent17FrameTransaction80026B94(state);
    DispatcherTailState80026B94 tail{};
    CHECK(ArmDispatcherTail80026B94(tail, 1));
    Event17FrameTransactionBegin80026B94 pending = action;
    pending.logicFrame = 200u;
    CHECK(BeginEvent17FrameTransaction80026B94(state, pending));
    CHECK(CommitEvent17FrameTick80026B54(
        state, 200u, kEventTable17_800545A0, kCtxEvent17_8005451C));
    CHECK(!CanAdvanceEvent17FrameLogic80026B94(state));
    DispatcherTailStep80026B94 step{};
    CHECK(!TryStepEvent17DispatcherTail80026B94(state, tail, step));
    CHECK(tail.framesRemaining == kDispatcherResultTailFrames80026B94);
    CHECK(CommitEvent17FrameDrawAndBindFormalTail80026B94(
        state, 202u, false));
    CHECK(!CanAdvanceEvent17FrameLogic80026B94(state));
    CHECK(!TryStepEvent17DispatcherTail80026B94(state, tail, step));
    CHECK(tail.framesRemaining == kDispatcherResultTailFrames80026B94);
    CHECK(CommitEvent17FrameHostBoundary80026B94(state, true, true, true));
    CHECK(CanAdvanceEvent17FrameLogic80026B94(state));
    CHECK(TryStepEvent17DispatcherTail80026B94(state, tail, step));
    CHECK(step.kind == DispatcherTailStepKind80026B94::HoldFrame);
    CHECK(step.framesRemainingBefore == 60);
    CHECK(step.framesRemainingAfter == 59);
    CHECK(tail.framesRemaining == 59);

    Event17FrameTransactionBegin80026B94 closed{};
    closed.requestBound = true;
    closed.ctxAddress = kCtxEvent17_8005451C;
    closed.logicFrame = 203u;
    closed.frameClass = Event17FrameClass80026B94::ClosedInputTail;
    closed.inputClosed = true;
    closed.tailFramesRemainingBefore = 59;
    closed.tailFramesRemainingAfter = 58;
    CHECK(BeginEvent17FrameTransaction80026B94(state, closed));
    CHECK(CommitEvent17FrameTick80026B54(
        state, 203u, kEventTable17_800545A0, kCtxEvent17_8005451C));
    CHECK(CommitEvent17FrameDrawAndBindFormalTail80026B94(
        state, 203u, false));
    CHECK(CommitEvent17FrameHostBoundary80026B94(state, true, true, true));
    ResetEvent17FrameTransaction80026B94(state);
    CHECK(!CanSubmitEvent17FrameDraw8001E750(state, 204u, false));

    Event17FrameTransactionBegin80026B94 badTail = closed;
    badTail.logicFrame = 204u;
    badTail.tailFramesRemainingBefore = 0;
    badTail.tailFramesRemainingAfter = 0;
    CHECK(!BeginEvent17FrameTransaction80026B94(state, badTail));
}

void TestEvent6ArgSourceGate() {
    const EventFramePlan known =
        BuildDispatcher80026B94Plan(6, true, kCtxEvent6HiScoreTable80049278);
    CHECK(known.owner == EventFrameOwner::DispatcherLoop);
    CHECK(CountActions(known, EventFrameActionKind::Gap) == 0);

    const EventFrameAction* argGate =
        FindAction(known, EventFrameActionKind::GateArgCtxFromArg);
    CHECK(argGate != nullptr);
    CHECK(argGate->ctxAddress == kCtxEvent6HiScoreTable80049278);
    CHECK(argGate->args[0] == 1);

    const EventFrameAction* hiScoreGate =
        FindAction(known, EventFrameActionKind::GateEvent6HiScoreArg80049278);
    CHECK(hiScoreGate != nullptr);
    CHECK(hiScoreGate->auxAddress == kCtxEvent6HiScoreTable80049278);
    CHECK(hiScoreGate->args[0] == 1);
    CHECK(hiScoreGate->args[1] == 1);

    const EventFramePlan unknown = BuildDispatcher80026B94Plan(6);
    CHECK(FindAction(unknown, EventFrameActionKind::GateArgCtxFromArg) != nullptr);
    CHECK(CountActions(unknown, EventFrameActionKind::Gap) >= 1);

    const EventFramePlan wrong =
        BuildDispatcher80026B94Plan(6, true, 0x12345678u);
    const EventFrameAction* wrongGate =
        FindAction(wrong, EventFrameActionKind::GateEvent6HiScoreArg80049278);
    CHECK(wrongGate != nullptr);
    CHECK(wrongGate->args[0] == 1);
    CHECK(wrongGate->args[1] == 0);
    CHECK(CountActions(wrong, EventFrameActionKind::Gap) >= 1);
}

void TestEvent6FrameTransactionRuntime() {
    Event6FrameTransaction80026B94 state{};
    Event6FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = kCtxEvent6HiScoreTable80049278;
    begin.logicFrame = 100u;
    begin.frameClass = Event6FrameClass80026B94::OpenInputLoop;

    Event6FrameTransactionBegin80026B94 invalid = begin;
    invalid.requestBound = false;
    CHECK(!BeginEvent6FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.ctxAddress = 0x12345678u;
    CHECK(!BeginEvent6FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.inputClosed = true;
    CHECK(!BeginEvent6FrameTransaction80026B94(state, invalid));

    CHECK(BeginEvent6FrameTransaction80026B94(state, begin));
    CHECK(state.active);
    CHECK(!state.directDrawCompleteWithinLimits);
    CHECK(state.commandCount == kEvent6FrameCommandCount80026B94);
    CHECK(state.commands[0].kind ==
          Event6FrameCommandKind80026B94::Tick80025E48);
    CHECK(state.commands[0].psxFunction == kFn80025E48);
    CHECK(static_cast<uint32_t>(state.commands[0].arg0) ==
          kEventTable6_80054578);
    CHECK(state.commands[0].arg1Known);
    CHECK(state.commands[1].kind ==
          Event6FrameCommandKind80026B94::DrawRoute8001E750);
    CHECK(state.commands[1].psxFunction == kFn8001E750);
    CHECK(state.commands[1].arg0 == 6);
    CHECK(state.commands[1].arg1Known);
    CHECK(state.commands[1].arg1 == kCtxEvent6HiScoreTable80049278);
    CHECK(state.commands[2].kind ==
          Event6FrameCommandKind80026B94::PrepareDrawWork8001D74C);
    CHECK(state.commands[2].psxFunction == kFn8001D74C);
    CHECK(state.commands[2].arg0 == 3);
    CHECK(!state.commands[2].arg1Known);
    CHECK(state.commands[3].kind ==
          Event6FrameCommandKind80026B94::DrawPage80021594);
    CHECK(state.commands[3].psxFunction == kFn80021594);
    CHECK(static_cast<uint32_t>(state.commands[3].arg0) ==
          kCtxEvent6HiScoreTable80049278);
    CHECK(state.commands[4].psxFunction == kFn80035560);
    CHECK(state.commands[4].arg0 == 0);
    CHECK(state.commands[5].psxFunction == kFn8001EA00);
    CHECK(state.commands[5].arg0 == 6);
    CHECK(state.commands[6].psxFunction == kFn800436F0);
    CHECK(state.commands[6].arg0 == -1);

    CHECK(!CanSubmitEvent6FrameDraw8001E750(state, 100u, false));
    CHECK(!CommitEvent6FrameDrawAndBindFormalTail80026B94(
        state, 100u, false));
    CHECK(!CommitEvent6FrameTick80025E48(
        state, 100u, kEventTable3_80054550,
        kCtxEvent6HiScoreTable80049278));
    CHECK(CommitEvent6FrameTick80025E48(
        state, 100u, kEventTable6_80054578,
        kCtxEvent6HiScoreTable80049278));
    CHECK(!CommitEvent6FrameTick80025E48(
        state, 100u, kEventTable6_80054578,
        kCtxEvent6HiScoreTable80049278));
    CHECK(CanSubmitEvent6FrameDraw8001E750(state, 100u, false));
    CHECK(CommitEvent6FrameDrawAndBindFormalTail80026B94(
        state, 100u, false));
    CHECK(state.directDrawSubmitted);
    CHECK(state.formalTailOrderBound);
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(!state.exactPsxHalParity);
    CHECK(state.directDrawCompleteWithinLimits);
    CHECK(state.firstHostPresentationFrame == 100u);
    CHECK(state.hostPresentationSubmitCount == 1u);
    CHECK(!CanAdvanceEvent6FrameLogic80026B94(state));
    CHECK(!CommitEvent6FrameHostBoundary80026B94(state, true, false, false));
    CHECK(state.hostWaitExecuted);
    CHECK(!state.hostEndExecuted);
    CHECK(!state.hostTextFlushExecuted);
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(CommitEvent6FrameHostBoundary80026B94(state, true, true, true));
    CHECK(state.hostWaitExecuted);
    CHECK(state.hostEndExecuted);
    CHECK(state.hostTextFlushExecuted);
    CHECK(state.hostFrameBoundaryExecuted);
    CHECK(CanAdvanceEvent6FrameLogic80026B94(state));
    CHECK(!CanSubmitEvent6FrameDraw8001E750(state, 100u, false));
    CHECK(CanSubmitEvent6FrameDraw8001E750(state, 100u, true));
    CHECK(CommitEvent6FrameDrawAndBindFormalTail80026B94(
        state, 100u, true));
    CHECK(state.hostPresentationSubmitCount == 2u);

    Event6FrameTransactionBegin80026B94 action = begin;
    action.logicFrame = 101u;
    action.frameClass = Event6FrameClass80026B94::ResultAction;
    action.tailFramesRemainingBefore =
        kDispatcherResultTailFrames80026B94;
    action.tailFramesRemainingAfter =
        kDispatcherResultTailFrames80026B94;
    CHECK(BeginEvent6FrameTransaction80026B94(state, action));
    CHECK(state.frameClass == Event6FrameClass80026B94::ResultAction);
    CHECK(!state.inputClosed);
    CHECK(state.tailFramesRemainingBefore == 60);
    CHECK(state.tailFramesRemainingAfter == 60);

    Event6FrameTransactionBegin80026B94 nextOpen = begin;
    nextOpen.logicFrame = 102u;
    // An unfinished prior frame cannot be overwritten silently.
    CHECK(!BeginEvent6FrameTransaction80026B94(state, nextOpen));
    // A missing presentation keeps the same translated logic transaction
    // retryable on a later host frame; it may not be overwritten.
    CHECK(CommitEvent6FrameTick80025E48(
        state, 101u, kEventTable6_80054578,
        kCtxEvent6HiScoreTable80049278));
    CHECK(CanSubmitEvent6FrameDraw8001E750(state, 103u, false));
    CHECK(CommitEvent6FrameDrawAndBindFormalTail80026B94(
        state, 103u, false));
    CHECK(state.directDrawCompleteWithinLimits);
    CHECK(state.logicFrame == 101u);
    CHECK(state.firstHostPresentationFrame == 103u);
    CHECK(CommitEvent6FrameHostBoundary80026B94(state, true, true, true));
    CHECK(CanAdvanceEvent6FrameLogic80026B94(state));
    CHECK(CanSubmitEvent6FrameDraw8001E750(state, 103u, true));
    CHECK(CommitEvent6FrameDrawAndBindFormalTail80026B94(
        state, 103u, true));
    CHECK(state.hostPresentationSubmitCount == 2u);
    CHECK(state.formalTailOrderBound);
    CHECK(!CanSubmitEvent6FrameDraw8001E750(state, 104u, true));
    ResetEvent6FrameTransaction80026B94(state);
    CHECK(!state.active);

    uint32_t frame = 200u;
    action.logicFrame = frame;
    CHECK(BeginEvent6FrameTransaction80026B94(state, action));
    CHECK(CommitEvent6FrameTick80025E48(
        state, frame, kEventTable6_80054578,
        kCtxEvent6HiScoreTable80049278));
    CHECK(CommitEvent6FrameDrawAndBindFormalTail80026B94(
        state, frame, false));
    CHECK(CommitEvent6FrameHostBoundary80026B94(state, true, true, true));
    int committedFrames = 1;
    for (int before = kDispatcherResultTailFrames80026B94;
         before > 0;
         --before) {
        Event6FrameTransactionBegin80026B94 tail{};
        tail.requestBound = true;
        tail.ctxAddress = kCtxEvent6HiScoreTable80049278;
        tail.logicFrame = ++frame;
        tail.frameClass = Event6FrameClass80026B94::ClosedInputTail;
        tail.inputClosed = true;
        tail.tailFramesRemainingBefore = before;
        tail.tailFramesRemainingAfter = before - 1;
        CHECK(BeginEvent6FrameTransaction80026B94(state, tail));
        CHECK(CommitEvent6FrameTick80025E48(
            state, frame, kEventTable6_80054578,
            kCtxEvent6HiScoreTable80049278));
        CHECK(CommitEvent6FrameDrawAndBindFormalTail80026B94(
            state, frame, false));
        CHECK(CommitEvent6FrameHostBoundary80026B94(
            state, true, true, true));
        ++committedFrames;
    }
    CHECK(committedFrames == kDispatcherResultTailFrames80026B94 + 1);

    // The following result-release update is outside the original do/while
    // frame body. Clearing the request therefore forbids another draw submit.
    ++frame;
    ResetEvent6FrameTransaction80026B94(state);
    CHECK(!CanSubmitEvent6FrameDraw8001E750(state, frame, false));

    Event6FrameTransactionBegin80026B94 badTail{};
    badTail.requestBound = true;
    badTail.ctxAddress = kCtxEvent6HiScoreTable80049278;
    badTail.logicFrame = frame;
    badTail.frameClass = Event6FrameClass80026B94::ClosedInputTail;
    badTail.inputClosed = true;
    badTail.tailFramesRemainingBefore = 0;
    badTail.tailFramesRemainingAfter = 0;
    CHECK(!BeginEvent6FrameTransaction80026B94(state, badTail));
}

void TestEvent6PendingFrameBlocksLogicAndTail() {
    Event6FrameTransaction80026B94 transaction{};
    DispatcherTailState80026B94 tail{};
    CHECK(ArmDispatcherTail80026B94(tail, 1));
    CHECK(tail.framesRemaining == kDispatcherResultTailFrames80026B94);

    Event6FrameTransactionBegin80026B94 action{};
    action.requestBound = true;
    action.ctxAddress = kCtxEvent6HiScoreTable80049278;
    action.logicFrame = 300u;
    action.frameClass = Event6FrameClass80026B94::ResultAction;
    action.tailFramesRemainingBefore = kDispatcherResultTailFrames80026B94;
    action.tailFramesRemainingAfter = kDispatcherResultTailFrames80026B94;
    CHECK(BeginEvent6FrameTransaction80026B94(transaction, action));
    CHECK(CommitEvent6FrameTick80025E48(
        transaction, action.logicFrame, kEventTable6_80054578,
        kCtxEvent6HiScoreTable80049278));
    CHECK(!CanAdvanceEvent6FrameLogic80026B94(transaction));

    DispatcherTailStep80026B94 step{};
    CHECK(!TryStepEvent6DispatcherTail80026B94(
        transaction, tail, step));
    CHECK(!TryStepEvent6DispatcherTail80026B94(
        transaction, tail, step));
    CHECK(tail.active);
    CHECK(tail.framesRemaining == kDispatcherResultTailFrames80026B94);
    CHECK(transaction.active);
    CHECK(transaction.tickCommitted);
    CHECK(!transaction.directDrawCompleteWithinLimits);
    CHECK(!transaction.directDrawSubmitted);
    CHECK(!transaction.formalTailOrderBound);
    CHECK(!transaction.hostFrameBoundaryExecuted);
    CHECK(!transaction.exactPsxHalParity);
    CHECK(transaction.hostPresentationSubmitCount == 0u);

    // A later host frame may complete the same semantic transaction.
    CHECK(CommitEvent6FrameDrawAndBindFormalTail80026B94(
        transaction, 302u, false));
    CHECK(CommitEvent6FrameHostBoundary80026B94(
        transaction, true, true, true));
    CHECK(CanAdvanceEvent6FrameLogic80026B94(transaction));
    CHECK(TryStepEvent6DispatcherTail80026B94(
        transaction, tail, step));
    CHECK(step.kind == DispatcherTailStepKind80026B94::HoldFrame);
    CHECK(step.framesRemainingBefore == 60);
    CHECK(step.framesRemainingAfter == 59);
    CHECK(tail.framesRemaining == 59);

    Event6FrameTransactionBegin80026B94 closed{};
    closed.requestBound = true;
    closed.ctxAddress = kCtxEvent6HiScoreTable80049278;
    closed.logicFrame = 303u;
    closed.frameClass = Event6FrameClass80026B94::ClosedInputTail;
    closed.inputClosed = true;
    closed.tailFramesRemainingBefore = 60;
    closed.tailFramesRemainingAfter = 59;
    CHECK(BeginEvent6FrameTransaction80026B94(transaction, closed));
    CHECK(CommitEvent6FrameTick80025E48(
        transaction, closed.logicFrame, kEventTable6_80054578,
        kCtxEvent6HiScoreTable80049278));
    CHECK(!TryStepEvent6DispatcherTail80026B94(
        transaction, tail, step));
    CHECK(tail.framesRemaining == 59);
}

void TestEvent6EndFrameOwnerPlan8001EA00() {
    Event6EndFrameOwnerInput8001EA00 input{};
    input.requestBound = true;
    input.eventId = 6;
    input.graphSlotBeforeKnown = true;
    input.graphSlotBefore = 0u;
    input.workListSlotKnown = true;
    input.workListSlot = 1u;
    input.workListAddressKnown = true;
    input.workListAddress =
        kEvent6EndFrameWorkListBase80087288 +
        kEvent6EndFrameWorkListStride8001EA00;

    Event6EndFrameOwnerPlan8001EA00 plan{};
    CHECK(BuildEvent6EndFrameOwnerPlan8001EA00(input, plan));
    CHECK(plan.formalOrderBound);
    CHECK(!plan.graphModelReady);
    CHECK(plan.formalOnly);
    CHECK(!plan.graphModelApplied);
    CHECK(!plan.hostClearExecuted);
    CHECK(!plan.hostSubmitExecuted);
    CHECK(!plan.exactPsxHalParity);
    CHECK(plan.graphSlotBefore == 0u);
    CHECK(plan.graphSlotAfter == 1u);
    CHECK(plan.workListSlot == 1u);
    CHECK(plan.workListAddress == 0x8008729Cu);
    CHECK(plan.clearR == 0u);
    CHECK(plan.clearG == 0u);
    CHECK(plan.clearB == 70u);
    CHECK(plan.actionCount == kEvent6EndFrameActionCount8001EA00);
    CHECK(plan.actions[0].kind ==
          Event6EndFrameActionKind8001EA00::FlipGraph80040370);
    CHECK(plan.actions[0].psxFunction == kFn80040370);
    CHECK(plan.actions[1].kind ==
          Event6EndFrameActionKind8001EA00::ClearColor80040420);
    CHECK(plan.actions[1].psxFunction == kFn80040420);
    CHECK(plan.actions[1].arg0 == 0u);
    CHECK(plan.actions[1].arg1 == 0u);
    CHECK(plan.actions[1].arg2 == 70u);
    CHECK(plan.actions[2].kind ==
          Event6EndFrameActionKind8001EA00::SubmitWorkList80040CA4);
    CHECK(plan.actions[2].psxFunction == kFn80040CA4);
    CHECK(plan.actions[2].arg0 == 0x8008729Cu);
    CHECK(plan.actions[2].arg1 == 1u);

    input.workListReady = true;
    input.graphModelPreflightKnown = true;
    input.graphModelWorkListSlotMatches = true;
    CHECK(BuildEvent6EndFrameOwnerPlan8001EA00(input, plan));
    CHECK(plan.graphModelReady);
    CHECK(!plan.formalOnly);
    CHECK(!plan.graphModelApplied);
    CHECK(!plan.hostClearExecuted);
    CHECK(!plan.hostSubmitExecuted);

    Event6EndFrameOwnerInput8001EA00 invalid = input;
    invalid.eventId = 3;
    CHECK(!BuildEvent6EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.workListAddress = 0x80087288u;
    CHECK(!BuildEvent6EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.workListSlot = 2u;
    CHECK(!BuildEvent6EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.graphSlotBeforeKnown = false;
    CHECK(!BuildEvent6EndFrameOwnerPlan8001EA00(invalid, plan));
}

void TestEvent17EndFrameOwnerPlan8001EA00() {
    Event17EndFrameOwnerInput8001EA00 input{};
    input.requestBound = true;
    input.eventId = 17;
    input.graphSlotBeforeKnown = true;
    input.graphSlotBefore = 1u;
    input.workListSlotKnown = true;
    input.workListSlot = 0u;
    input.workListAddressKnown = true;
    input.workListAddress = kEvent17EndFrameWorkListBase80087288;

    Event17EndFrameOwnerPlan8001EA00 plan{};
    CHECK(BuildEvent17EndFrameOwnerPlan8001EA00(input, plan));
    CHECK(plan.formalOrderBound);
    CHECK(!plan.graphModelReady);
    CHECK(plan.formalOnly);
    CHECK(!plan.graphModelApplied);
    CHECK(!plan.hostClearExecuted);
    CHECK(!plan.hostSubmitExecuted);
    CHECK(!plan.exactPsxHalParity);
    CHECK(plan.graphSlotBefore == 1u);
    CHECK(plan.graphSlotAfter == 0u);
    CHECK(plan.workListSlot == 0u);
    CHECK(plan.workListAddress == 0x80087288u);
    CHECK(plan.clearR == 0u);
    CHECK(plan.clearG == 0u);
    CHECK(plan.clearB == 70u);
    CHECK(plan.actionCount == kEvent17EndFrameActionCount8001EA00);
    CHECK(plan.actions[0].kind ==
          Event17EndFrameActionKind8001EA00::FlipGraph80040370);
    CHECK(plan.actions[0].psxFunction == kFn80040370);
    CHECK(!plan.actions[0].arg0Known);
    CHECK(!plan.actions[0].arg1Known);
    CHECK(!plan.actions[0].arg2Known);
    CHECK(plan.actions[1].kind ==
          Event17EndFrameActionKind8001EA00::ClearColor80040420);
    CHECK(plan.actions[1].psxFunction == kFn80040420);
    CHECK(plan.actions[1].arg0 == 0u);
    CHECK(plan.actions[1].arg1 == 0u);
    CHECK(plan.actions[1].arg2 == 70u);
    CHECK(plan.actions[1].arg0Known);
    CHECK(plan.actions[1].arg1Known);
    CHECK(plan.actions[1].arg2Known);
    CHECK(plan.actions[2].kind ==
          Event17EndFrameActionKind8001EA00::SubmitWorkList80040CA4);
    CHECK(plan.actions[2].psxFunction == kFn80040CA4);
    CHECK(plan.actions[2].arg0 == 0x80087288u);
    CHECK(plan.actions[2].arg1 == 0u);
    CHECK(plan.actions[2].arg0Known);
    CHECK(!plan.actions[2].arg1Known);
    CHECK(!plan.actions[2].arg2Known);

    Event17EndFrameOwnerInput8001EA00 slotOne = input;
    slotOne.workListSlot = 1u;
    slotOne.workListAddress =
        kEvent17EndFrameWorkListBase80087288 +
        kEvent17EndFrameWorkListStride8001EA00;
    CHECK(BuildEvent17EndFrameOwnerPlan8001EA00(slotOne, plan));
    CHECK(plan.workListSlot == 1u);
    CHECK(plan.workListAddress == 0x8008729Cu);
    CHECK(plan.actions[2].arg0 == 0x8008729Cu);
    CHECK(!plan.actions[2].arg1Known);

    input.workListReady = true;
    input.graphModelPreflightKnown = true;
    input.graphModelWorkListSlotMatches = true;
    CHECK(BuildEvent17EndFrameOwnerPlan8001EA00(input, plan));
    CHECK(plan.graphModelReady);
    CHECK(!plan.formalOnly);
    CHECK(!plan.graphModelApplied);
    CHECK(!plan.hostClearExecuted);
    CHECK(!plan.hostSubmitExecuted);

    Event17EndFrameOwnerInput8001EA00 preflightMissing = input;
    preflightMissing.workListReady = false;
    CHECK(BuildEvent17EndFrameOwnerPlan8001EA00(preflightMissing, plan));
    CHECK(!plan.graphModelReady);
    CHECK(plan.formalOnly);
    preflightMissing = input;
    preflightMissing.graphModelPreflightKnown = false;
    CHECK(BuildEvent17EndFrameOwnerPlan8001EA00(preflightMissing, plan));
    CHECK(!plan.graphModelReady);
    CHECK(plan.formalOnly);
    preflightMissing = input;
    preflightMissing.graphModelWorkListSlotMatches = false;
    CHECK(BuildEvent17EndFrameOwnerPlan8001EA00(preflightMissing, plan));
    CHECK(!plan.graphModelReady);
    CHECK(plan.formalOnly);

    Event17EndFrameOwnerInput8001EA00 invalid = input;
    invalid.eventId = 6;
    CHECK(!BuildEvent17EndFrameOwnerPlan8001EA00(invalid, plan));
    CHECK(!plan.formalOrderBound);
    CHECK(plan.formalOnly);
    CHECK(plan.actionCount == 0u);
    invalid = input;
    invalid.workListAddress = 0x8008729Cu;
    CHECK(!BuildEvent17EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.workListSlot = 2u;
    CHECK(!BuildEvent17EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.workListSlotKnown = false;
    CHECK(!BuildEvent17EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.graphSlotBefore = 2u;
    CHECK(!BuildEvent17EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.graphSlotBeforeKnown = false;
    CHECK(!BuildEvent17EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.workListAddressKnown = false;
    CHECK(!BuildEvent17EndFrameOwnerPlan8001EA00(invalid, plan));
    invalid = input;
    invalid.requestBound = false;
    CHECK(!BuildEvent17EndFrameOwnerPlan8001EA00(invalid, plan));
}

void TestEvent3FrameTransactionRuntime() {
    Event3FrameTransaction80026B94 state{};
    Event3FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = kCtxEvent3_800544F8;
    begin.logicFrame = 100u;
    begin.frameClass = Event3FrameClass80026B94::OpenInputLoop;

    Event3FrameTransactionBegin80026B94 invalid = begin;
    invalid.requestBound = false;
    CHECK(!BeginEvent3FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.ctxAddress = 0x12345678u;
    CHECK(!BeginEvent3FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.inputClosed = true;
    CHECK(!BeginEvent3FrameTransaction80026B94(state, invalid));

    CHECK(BeginEvent3FrameTransaction80026B94(state, begin));
    CHECK(state.active);
    CHECK(!state.directDrawCompleteWithinLimits);
    CHECK(state.commandCount == kEvent3FrameCommandCount80026B94);
    CHECK(state.commands[0].kind ==
          Event3FrameCommandKind80026B94::Tick80026720);
    CHECK(state.commands[0].psxFunction == kFn80026720);
    CHECK(static_cast<uint32_t>(state.commands[0].arg0) ==
          kEventTable3_80054550);
    CHECK(state.commands[0].arg1Known);
    CHECK(state.commands[0].arg1 == kCtxEvent3_800544F8);
    CHECK(state.commands[1].kind ==
          Event3FrameCommandKind80026B94::DrawRoute8001E750);
    CHECK(state.commands[1].psxFunction == kFn8001E750);
    CHECK(state.commands[1].arg0 == 3);
    CHECK(state.commands[1].arg1Known);
    CHECK(state.commands[1].arg1 == kCtxEvent3_800544F8);
    CHECK(state.commands[2].kind ==
          Event3FrameCommandKind80026B94::PrepareDrawWork8001D74C);
    CHECK(state.commands[2].psxFunction == kFn8001D74C);
    CHECK(state.commands[2].arg0 == 3);
    CHECK(!state.commands[2].arg1Known);
    CHECK(state.commands[3].kind ==
          Event3FrameCommandKind80026B94::DrawPage80021E60);
    CHECK(state.commands[3].psxFunction == kFn80021E60);
    CHECK(static_cast<uint32_t>(state.commands[3].arg0) ==
          kCtxEvent3_800544F8);
    CHECK(state.commands[4].kind ==
          Event3FrameCommandKind80026B94::WaitFrame80035560);
    CHECK(state.commands[4].psxFunction == kFn80035560);
    CHECK(state.commands[4].arg0 == 0);
    CHECK(state.commands[5].kind ==
          Event3FrameCommandKind80026B94::EndFrame8001EA00);
    CHECK(state.commands[5].psxFunction == kFn8001EA00);
    CHECK(state.commands[5].arg0 == 3);
    CHECK(state.commands[6].kind ==
          Event3FrameCommandKind80026B94::TextFlush800436F0);
    CHECK(state.commands[6].psxFunction == kFn800436F0);
    CHECK(state.commands[6].arg0 == -1);

    CHECK(!CanSubmitEvent3FrameDraw8001E750(state, 100u, false));
    CHECK(!CommitEvent3FrameDrawAndBindFormalTail80026B94(
        state, 100u, false));
    CHECK(!CommitEvent3FrameTick80026720(
        state, 100u, kEventTable6_80054578, kCtxEvent3_800544F8));
    CHECK(CommitEvent3FrameTick80026720(
        state, 100u, kEventTable3_80054550, kCtxEvent3_800544F8));
    CHECK(!CommitEvent3FrameTick80026720(
        state, 100u, kEventTable3_80054550, kCtxEvent3_800544F8));
    CHECK(CanSubmitEvent3FrameDraw8001E750(state, 100u, false));
    CHECK(CommitEvent3FrameDrawAndBindFormalTail80026B94(
        state, 100u, false));
    CHECK(state.directDrawSubmitted);
    CHECK(state.formalTailOrderBound);
    CHECK(!state.hostWaitExecuted);
    CHECK(!state.hostEndExecuted);
    CHECK(!state.hostTextFlushExecuted);
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(!state.exactPsxHalParity);
    CHECK(state.directDrawCompleteWithinLimits);
    CHECK(state.firstHostPresentationFrame == 100u);
    CHECK(state.hostPresentationSubmitCount == 1u);
    CHECK(!CanSubmitEvent3FrameDraw8001E750(state, 100u, false));
    CHECK(CanSubmitEvent3FrameDraw8001E750(state, 100u, true));
    CHECK(CommitEvent3FrameDrawAndBindFormalTail80026B94(
        state, 100u, true));
    CHECK(state.hostPresentationSubmitCount == 2u);
    CHECK(!CanSubmitEvent3FrameDraw8001E750(state, 101u, true));
    CHECK(!CanAdvanceEvent3FrameLogic80026B94(state));
    CHECK(!CommitEvent3FrameHostBoundary80026B94(
        state, true, false, false));
    CHECK(state.hostWaitExecuted);
    CHECK(!state.hostEndExecuted);
    CHECK(!state.hostTextFlushExecuted);
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(CommitEvent3FrameHostBoundary80026B94(
        state, true, true, true));
    CHECK(state.hostFrameBoundaryExecuted);
    CHECK(CanAdvanceEvent3FrameLogic80026B94(state));

    Event3FrameTransactionBegin80026B94 action = begin;
    action.logicFrame = 101u;
    action.frameClass = Event3FrameClass80026B94::ResultAction;
    action.tailFramesRemainingBefore =
        kDispatcherResultTailFrames80026B94;
    action.tailFramesRemainingAfter =
        kDispatcherResultTailFrames80026B94;
    CHECK(BeginEvent3FrameTransaction80026B94(state, action));
    CHECK(state.frameClass == Event3FrameClass80026B94::ResultAction);
    CHECK(!state.inputClosed);
    CHECK(state.tailFramesRemainingBefore == 60);
    CHECK(state.tailFramesRemainingAfter == 60);

    Event3FrameTransactionBegin80026B94 nextOpen = begin;
    nextOpen.logicFrame = 102u;
    CHECK(!BeginEvent3FrameTransaction80026B94(state, nextOpen));
    CHECK(CommitEvent3FrameTick80026720(
        state, 101u, kEventTable3_80054550, kCtxEvent3_800544F8));
    CHECK(CanSubmitEvent3FrameDraw8001E750(state, 103u, false));
    CHECK(CommitEvent3FrameDrawAndBindFormalTail80026B94(
        state, 103u, false));
    CHECK(state.firstHostPresentationFrame == 103u);
    CHECK(CanSubmitEvent3FrameDraw8001E750(state, 103u, true));
    CHECK(CommitEvent3FrameDrawAndBindFormalTail80026B94(
        state, 103u, true));
    CHECK(state.hostPresentationSubmitCount == 2u);
    CHECK(!CanAdvanceEvent3FrameLogic80026B94(state));
    CHECK(CommitEvent3FrameHostBoundary80026B94(
        state, true, true, true));

    uint32_t frame = 200u;
    DispatcherTailState80026B94 tail{};
    CHECK(ArmDispatcherTail80026B94(tail, 1));
    Event3FrameTransactionBegin80026B94 pending = action;
    pending.logicFrame = frame;
    CHECK(BeginEvent3FrameTransaction80026B94(state, pending));
    CHECK(CommitEvent3FrameTick80026720(
        state, frame, kEventTable3_80054550, kCtxEvent3_800544F8));
    CHECK(!CanAdvanceEvent3FrameLogic80026B94(state));
    DispatcherTailStep80026B94 step{};
    CHECK(!TryStepEvent3DispatcherTail80026B94(state, tail, step));
    CHECK(tail.framesRemaining == kDispatcherResultTailFrames80026B94);
    CHECK(CommitEvent3FrameDrawAndBindFormalTail80026B94(
        state, frame + 2u, false));
    CHECK(!CanAdvanceEvent3FrameLogic80026B94(state));
    CHECK(CommitEvent3FrameHostBoundary80026B94(
        state, true, true, true));
    CHECK(CanAdvanceEvent3FrameLogic80026B94(state));
    CHECK(TryStepEvent3DispatcherTail80026B94(state, tail, step));
    CHECK(step.kind == DispatcherTailStepKind80026B94::HoldFrame);
    CHECK(step.framesRemainingBefore == 60);
    CHECK(step.framesRemainingAfter == 59);
    CHECK(tail.framesRemaining == 59);

    int committedFrames = 1;
    for (int before = kDispatcherResultTailFrames80026B94;
         before > 0;
         --before) {
        Event3FrameTransactionBegin80026B94 closed{};
        closed.requestBound = true;
        closed.ctxAddress = kCtxEvent3_800544F8;
        closed.logicFrame = ++frame;
        closed.frameClass = Event3FrameClass80026B94::ClosedInputTail;
        closed.inputClosed = true;
        closed.tailFramesRemainingBefore = before;
        closed.tailFramesRemainingAfter = before - 1;
        CHECK(BeginEvent3FrameTransaction80026B94(state, closed));
        CHECK(CommitEvent3FrameTick80026720(
            state, closed.logicFrame, kEventTable3_80054550,
            kCtxEvent3_800544F8));
        CHECK(CommitEvent3FrameDrawAndBindFormalTail80026B94(
            state, closed.logicFrame, false));
        CHECK(CommitEvent3FrameHostBoundary80026B94(
            state, true, true, true));
        ++committedFrames;
    }
    CHECK(committedFrames == kDispatcherResultTailFrames80026B94 + 1);

    ++frame;
    ResetEvent3FrameTransaction80026B94(state);
    CHECK(!CanSubmitEvent3FrameDraw8001E750(state, frame, false));

    Event3FrameTransactionBegin80026B94 badTail{};
    badTail.requestBound = true;
    badTail.ctxAddress = kCtxEvent3_800544F8;
    badTail.logicFrame = frame;
    badTail.frameClass = Event3FrameClass80026B94::ClosedInputTail;
    badTail.inputClosed = true;
    badTail.tailFramesRemainingBefore = 0;
    badTail.tailFramesRemainingAfter = 0;
    CHECK(!BeginEvent3FrameTransaction80026B94(state, badTail));
}

void TestEvent2FrameTransactionRuntime() {
    Event2FrameTransaction80026B94 state{};
    Event2FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = kCtxEvent2_80087B78;
    begin.logicFrame = 100u;
    begin.frameClass = Event2FrameClass80026B94::OpenInputLoop;
    begin.word800916F6Known = true;
    begin.word800916F6 = 0u;

    Event2FrameTransactionBegin80026B94 invalid = begin;
    invalid.requestBound = false;
    CHECK(!BeginEvent2FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.ctxAddress = 0x12345678u;
    CHECK(!BeginEvent2FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.inputClosed = true;
    CHECK(!BeginEvent2FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.word800916F6Known = false;
    CHECK(!BeginEvent2FrameTransaction80026B94(state, invalid));
    invalid = begin;
    invalid.word800916F6 = 1u;
    CHECK(!BeginEvent2FrameTransaction80026B94(state, invalid));

    CHECK(BeginEvent2FrameTransaction80026B94(state, begin));
    CHECK(state.active);
    CHECK(!state.directDrawCompleteWithinLimits);
    CHECK(state.commandCount == kEvent2FrameCommandCount80026B94);
    CHECK(state.commands[0].kind ==
          Event2FrameCommandKind80026B94::Tick80026170);
    CHECK(state.commands[0].psxFunction == kFn80026170);
    CHECK(static_cast<uint32_t>(state.commands[0].arg0) ==
          kEventTable2_8005453C);
    CHECK(state.commands[0].arg1Known);
    CHECK(state.commands[0].arg1 == kCtxEvent2_80087B78);
    CHECK(state.commands[1].kind ==
          Event2FrameCommandKind80026B94::DrawRoute8001E750);
    CHECK(state.commands[1].arg0 == 2);
    CHECK(state.commands[1].arg1 == kCtxEvent2_80087B78);
    CHECK(state.commands[2].kind ==
          Event2FrameCommandKind80026B94::PrepareDrawWork8001D74C);
    CHECK(state.commands[2].arg0 == 3);
    CHECK(!state.commands[2].arg1Known);
    CHECK(state.commands[3].kind ==
          Event2FrameCommandKind80026B94::DrawPage80020568);
    CHECK(state.commands[3].psxFunction == kFn80020568);
    CHECK(static_cast<uint32_t>(state.commands[3].arg0) ==
          kCtxEvent2_80087B78);
    CHECK(!state.commands[3].arg1Known);
    CHECK(state.commands[4].kind ==
          Event2FrameCommandKind80026B94::StageClearText80043A14);
    CHECK(state.commands[4].psxFunction == kFn80043A14);
    CHECK(state.commands[4].arg1 == 0x800916F6u);
    CHECK(state.commands[4].arg1Known);
    CHECK(state.commands[4].conditional);
    CHECK(state.commands[4].conditionalGateAddress == 0x800916F6u);
    CHECK(state.commands[4].conditionalGateValueKnown);
    CHECK(state.commands[4].conditionalGateValue == 0u);
    CHECK(!state.commands[4].conditionalTaken);
    CHECK(state.stageClearGateValueKnown800916F6);
    CHECK(state.stageClearGateValue800916F6 == 0u);
    CHECK(!state.stageClearBranchTaken80026D70);
    CHECK(state.commands[5].kind ==
          Event2FrameCommandKind80026B94::WaitFrame80035560);
    CHECK(state.commands[6].kind ==
          Event2FrameCommandKind80026B94::EndFrame8001EA00);
    CHECK(state.commands[6].arg0 == 2);
    CHECK(state.commands[7].kind ==
          Event2FrameCommandKind80026B94::TextFlush800436F0);
    CHECK(state.commands[7].arg0 == -1);

    CHECK(!CanSubmitEvent2FrameDraw8001E750(state, 100u, false));
    CHECK(!CommitEvent2FrameDrawAndBindFormalTail80026B94(
        state, 100u, false));
    CHECK(!CommitEvent2FrameTick80026170(
        state, 100u, kEventTable3_80054550, kCtxEvent2_80087B78));
    CHECK(CommitEvent2FrameTick80026170(
        state, 100u, kEventTable2_8005453C, kCtxEvent2_80087B78));
    CHECK(!CommitEvent2FrameTick80026170(
        state, 100u, kEventTable2_8005453C, kCtxEvent2_80087B78));
    CHECK(CanSubmitEvent2FrameDraw8001E750(state, 100u, false));
    CHECK(CommitEvent2FrameDrawAndBindFormalTail80026B94(
        state, 100u, false));
    CHECK(state.directDrawSubmitted);
    CHECK(state.formalTailOrderBound);
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(!state.exactPsxHalParity);
    CHECK(state.directDrawCompleteWithinLimits);
    CHECK(state.firstHostPresentationFrame == 100u);
    CHECK(state.hostPresentationSubmitCount == 1u);
    CHECK(!CanAdvanceEvent2FrameLogic80026B94(state));
    CHECK(!CommitEvent2FrameHostBoundary80026B94(state, true, false, false));
    CHECK(state.hostWaitExecuted);
    CHECK(!state.hostEndExecuted);
    CHECK(!state.hostTextFlushExecuted);
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(CommitEvent2FrameHostBoundary80026B94(state, true, true, true));
    CHECK(state.hostWaitExecuted);
    CHECK(state.hostEndExecuted);
    CHECK(state.hostTextFlushExecuted);
    CHECK(state.hostFrameBoundaryExecuted);
    CHECK(CanAdvanceEvent2FrameLogic80026B94(state));
    CHECK(!CanSubmitEvent2FrameDraw8001E750(state, 100u, false));
    CHECK(CanSubmitEvent2FrameDraw8001E750(state, 100u, true));
    CHECK(CommitEvent2FrameDrawAndBindFormalTail80026B94(
        state, 100u, true));
    CHECK(state.hostPresentationSubmitCount == 2u);
    CHECK(!CanSubmitEvent2FrameDraw8001E750(state, 101u, true));

    DispatcherTailState80026B94 tail{};
    CHECK(ArmDispatcherTail80026B94(tail, 1));
    Event2FrameTransactionBegin80026B94 action = begin;
    action.logicFrame = 200u;
    action.frameClass = Event2FrameClass80026B94::ResultAction;
    action.tailFramesRemainingBefore =
        kDispatcherResultTailFrames80026B94;
    action.tailFramesRemainingAfter =
        kDispatcherResultTailFrames80026B94;
    CHECK(BeginEvent2FrameTransaction80026B94(state, action));
    CHECK(CommitEvent2FrameTick80026170(
        state, 200u, kEventTable2_8005453C, kCtxEvent2_80087B78));
    CHECK(!CanAdvanceEvent2FrameLogic80026B94(state));
    DispatcherTailStep80026B94 step{};
    CHECK(!TryStepEvent2DispatcherTail80026B94(state, tail, step));
    CHECK(tail.framesRemaining == kDispatcherResultTailFrames80026B94);
    CHECK(CommitEvent2FrameDrawAndBindFormalTail80026B94(
        state, 202u, false));
    CHECK(!CanAdvanceEvent2FrameLogic80026B94(state));
    CHECK(CommitEvent2FrameHostBoundary80026B94(state, true, true, true));
    CHECK(CanAdvanceEvent2FrameLogic80026B94(state));
    CHECK(TryStepEvent2DispatcherTail80026B94(state, tail, step));
    CHECK(step.kind == DispatcherTailStepKind80026B94::HoldFrame);
    CHECK(step.framesRemainingBefore == 60);
    CHECK(step.framesRemainingAfter == 59);
    CHECK(tail.framesRemaining == 59);

    uint32_t frame = 203u;
    int committedFrames = 1;
    for (int before = kDispatcherResultTailFrames80026B94;
         before > 0;
         --before) {
        Event2FrameTransactionBegin80026B94 closed{};
        closed.requestBound = true;
        closed.ctxAddress = kCtxEvent2_80087B78;
        closed.logicFrame = frame++;
        closed.frameClass = Event2FrameClass80026B94::ClosedInputTail;
        closed.word800916F6Known = true;
        closed.word800916F6 = 0u;
        closed.inputClosed = true;
        closed.tailFramesRemainingBefore = before;
        closed.tailFramesRemainingAfter = before - 1;
        CHECK(BeginEvent2FrameTransaction80026B94(state, closed));
        CHECK(CommitEvent2FrameTick80026170(
            state, closed.logicFrame, kEventTable2_8005453C,
            kCtxEvent2_80087B78));
        CHECK(CommitEvent2FrameDrawAndBindFormalTail80026B94(
            state, closed.logicFrame, false));
        CHECK(CommitEvent2FrameHostBoundary80026B94(
            state, true, true, true));
        ++committedFrames;
    }
    CHECK(committedFrames == kDispatcherResultTailFrames80026B94 + 1);

    CHECK(TryStepEvent2DispatcherTail80026B94(state, tail, step));
    CHECK(step.kind == DispatcherTailStepKind80026B94::HoldFrame);
    CHECK(tail.framesRemaining == 58);
    ResetDispatcherTail80026B94(tail);
    CHECK(ArmDispatcherTail80026B94(tail, 2));
    // A release update is allowed only after the translated draw transaction
    // is complete; no body draw is credited for this boundary.
    tail.framesRemaining = 0;
    CHECK(TryStepEvent2DispatcherTail80026B94(state, tail, step));
    CHECK(step.kind == DispatcherTailStepKind80026B94::ReleaseResult);
    CHECK(step.releasedResult == 2);

    ResetEvent2FrameTransaction80026B94(state);
    CHECK(!CanSubmitEvent2FrameDraw8001E750(state, frame, false));
    Event2FrameTransactionBegin80026B94 badTail{};
    badTail.requestBound = true;
    badTail.ctxAddress = kCtxEvent2_80087B78;
    badTail.logicFrame = frame;
    badTail.frameClass = Event2FrameClass80026B94::ClosedInputTail;
    badTail.word800916F6Known = true;
    badTail.word800916F6 = 0u;
    badTail.inputClosed = true;
    badTail.tailFramesRemainingBefore = 0;
    badTail.tailFramesRemainingAfter = 0;
    CHECK(!BeginEvent2FrameTransaction80026B94(state, badTail));
}

void TestDispatcherPadChange80026744Runtime() {
    DispatcherPadChangeState80026744 state{};

    auto step = StepDispatcherPadChange80026744(state, 0u);
    CHECK(!step.changed);
    CHECK(step.previousPadMask80035510 == 0u);
    CHECK(step.changedPadMask80026744 == 0u);

    step = StepDispatcherPadChange80026744(state, 0x60u);
    CHECK(step.changed);
    CHECK(step.previousPadMask80035510 == 0u);
    CHECK(step.currentPadMask80035510 == 0x60u);
    CHECK(step.changedPadMask80026744 == 0x60u);

    step = StepDispatcherPadChange80026744(state, 0x60u);
    CHECK(!step.changed);
    CHECK(step.previousPadMask80035510 == 0x60u);
    CHECK(step.changedPadMask80026744 == 0u);

    // Releasing Circle while Cross remains held is a new exact 0x40 value.
    // The original 80026744 therefore forwards Cross to the handler.
    step = StepDispatcherPadChange80026744(state, 0x40u);
    CHECK(step.changed);
    CHECK(step.previousPadMask80035510 == 0x60u);
    CHECK(step.changedPadMask80026744 == 0x40u);

    step = StepDispatcherPadChange80026744(state, 0u);
    CHECK(step.changed);
    CHECK(step.previousPadMask80035510 == 0x40u);
    CHECK(step.changedPadMask80026744 == 0u);

    ResetDispatcherPadChange80026744(state);
    CHECK(state.previousPadMask80035510 == 0u);
}

void TestDrawRoutePromptAndDrawOnlyOwners() {
    const EventFramePlan prompt = BuildDrawRoute8001E750Plan(4);
    CHECK(prompt.owner == EventFrameOwner::ModalPrompt);
    CHECK(!prompt.truncated);

    const EventFrameActionKind expected[] = {
        EventFrameActionKind::DrawRoute8001E750,
        EventFrameActionKind::GateEvent4DrawWrapperSource8001E750,
        EventFrameActionKind::Event4PromptDraw800203D4,
        EventFrameActionKind::Event4Backdrop8001B6C4,
        EventFrameActionKind::Event4BackdropBoxFillLocal8001B6C4,
        EventFrameActionKind::Event4BackdropGsSortBoxFill8003EE84,
        EventFrameActionKind::Event4MoveImage8001B120,
        EventFrameActionKind::Event4MoveImageSlotSource8004019C,
        EventFrameActionKind::Event4MoveImageRect8001B120,
        EventFrameActionKind::Event4MoveImageDispatch80044E2C,
        EventFrameActionKind::Event4FrameSubmitHalGap,
        EventFrameActionKind::Gap,
    };
    CheckOrder(prompt, expected, sizeof(expected) / sizeof(expected[0]));

    const EventFrameAction* box =
        FindAction(prompt, EventFrameActionKind::Event4BackdropBoxFillLocal8001B6C4);
    CHECK(box != nullptr);
    CHECK(box->args[2] == kEvent4BackdropBoxFillW8001B6C4);
    CHECK(box->args[3] == kEvent4BackdropBoxFillH8001B6C4);

    const EventFrameAction* submitGap =
        FindAction(prompt, EventFrameActionKind::Event4FrameSubmitHalGap);
    CHECK(submitGap != nullptr);
    CHECK(submitGap->psxFunction == kFn800468E0);
    CHECK(submitGap->auxAddress == kFn80046840);

    const EventFramePlan card = BuildDrawRoute8001E750Plan(7);
    CHECK(card.owner == EventFrameOwner::DrawOnly);
    CHECK(FindAction(card, EventFrameActionKind::CardDrawIdOnly) != nullptr);
    CHECK(FindAction(card, EventFrameActionKind::PracticeSelfLoop) == nullptr);

    const EventFramePlan practiceDraw = BuildDrawRoute8001E750Plan(16);
    CHECK(practiceDraw.owner == EventFrameOwner::PracticeSelfLoop);
    CHECK(FindAction(practiceDraw, EventFrameActionKind::PracticeSelfLoop) != nullptr);
    CHECK(FindAction(practiceDraw, EventFrameActionKind::CardDrawIdOnly) == nullptr);
}

void TestEvent4DrawBodySoftwareTransaction() {
    Event4DrawPreambleInput8001E750 preambleInput{};
    auto preamble =
        BuildEvent4DrawPreambleTransaction8001E750(preambleInput);
    CHECK(!preamble.accepted);
    preambleInput.drawBufferSlotKnown = true;
    preambleInput.drawBufferSlot = 0u;
    preambleInput.packetAllocatorValueKnown = true;
    preambleInput.packetAllocatorValue = 0x801A73B0u;
    preambleInput.workListAddressKnown = true;
    preambleInput.workListAddress = kEvent4DrawWorkListBase80087288;
    preamble = BuildEvent4DrawPreambleTransaction8001E750(preambleInput);
    CHECK(preamble.accepted);
    CHECK(preamble.status == Event4DrawPreambleStatus8001E750::Ready);
    CHECK(preamble.drawBufferSourceFunction == kFn8004019C);
    CHECK(preamble.packetAllocatorFunction == kFn80040F90);
    CHECK(preamble.clearWorkListFunction == kFn80040CC8);
    CHECK(preamble.clearX == 0);
    CHECK(preamble.clearY == 0);
    CHECK(!preamble.exactPsxHalParity);
    preambleInput.drawBufferSlot = 2u;
    preamble = BuildEvent4DrawPreambleTransaction8001E750(preambleInput);
    CHECK(!preamble.accepted);
    CHECK(preamble.status ==
          Event4DrawPreambleStatus8001E750::DrawBufferSourceLimit);
    preambleInput.drawBufferSlot = 0u;
    preambleInput.packetAllocatorValue = 0u;
    preamble = BuildEvent4DrawPreambleTransaction8001E750(preambleInput);
    CHECK(!preamble.accepted);
    CHECK(preamble.status ==
          Event4DrawPreambleStatus8001E750::PacketAllocatorSourceLimit);
    preambleInput.packetAllocatorValue = 0x801A73B0u;
    preambleInput.workListAddress += 4u;
    preamble = BuildEvent4DrawPreambleTransaction8001E750(preambleInput);
    CHECK(!preamble.accepted);
    CHECK(preamble.status ==
          Event4DrawPreambleStatus8001E750::WorkListBindingLimit);

    Event4DrawBodyInput8001E750 input{};
    auto tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(!tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::SourceUnknown);

    input.wrapperStateKnown = true;
    input.wrapperState = 0u;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(!tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::PreambleSourceLimit);
    input.preamble.drawBufferSlotKnown = true;
    input.preamble.drawBufferSlot = 0u;
    input.preamble.packetAllocatorValueKnown = true;
    input.preamble.packetAllocatorValue = 0x801A73B0u;
    input.preamble.workListAddressKnown = true;
    input.preamble.workListAddress = kEvent4DrawWorkListBase80087288;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(!tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::InvalidPromptChoice);

    input.prompt.requestBound = true;
    input.prompt.choiceSourceKnown = true;
    input.prompt.ctx0 = -1;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::PromptReady);
    CHECK(tx.preamble.accepted);
    CHECK(tx.preamble.drawBufferSlot == 0u);
    CHECK(tx.preamble.workListAddress ==
          kEvent4DrawWorkListBase80087288);
    CHECK(tx.promptDraw);
    CHECK(tx.promptDrawList.accepted);
    CHECK(tx.promptDrawList.complete);
    CHECK(tx.promptDrawList.rawTextureOnly);
    CHECK(tx.promptDrawList.visibleColorIndependentOfRgb);
    CHECK(tx.promptDrawList.rgbTailUnresolvedWithoutInput);
    CHECK(tx.promptDrawList.count == 3u);
    CHECK(!tx.backdropFill);
    CHECK(tx.wrapperStateAfter == 0u);
    CHECK(tx.promptChoice == -1);

    input.prompt.ctx0 = 2;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::PromptReady);
    CHECK(tx.promptChoice == 2);
    CHECK(tx.promptDrawList.sprites[1].templateAddress == 0x80050960u);
    CHECK(tx.promptDrawList.sprites[2].templateAddress == 0x80050970u);
    CHECK(!tx.promptDrawList.sprites[1].selected);
    CHECK(!tx.promptDrawList.sprites[2].selected);

    input.wrapperState = 1u;
    input.workListSlotKnown = true;
    input.workListAddressKnown = false;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(!tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::WorkListBindingLimit);

    input.workListAddressKnown = true;
    input.workListSlot = 1u;
    input.workListAddress =
        kEvent4DrawWorkListBase80087288 + kEvent4DrawWorkListStride8001E750;
    input.preamble.drawBufferSlot = 1u;
    input.preamble.packetAllocatorValue = 0x801B54B0u;
    input.preamble.workListAddress = input.workListAddress;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::BackdropReady);
    CHECK(tx.backdropFill);
    CHECK(tx.preamble.drawBufferSlot == 1u);
    CHECK(tx.preamble.packetAllocatorValue == 0x801B54B0u);
    CHECK(tx.backdropAttr == 0x400F0F0Fu);
    CHECK(tx.backdropWidth == 320);
    CHECK(tx.backdropHeight == 240);
    CHECK(tx.wrapperStateAfter == 2u);

    input.workListAddress += 4u;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(!tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::WorkListBindingLimit);

    input.wrapperState = 2u;
    input.preamble.drawBufferSlot = 0u;
    input.preamble.packetAllocatorValue = 0x801A73B0u;
    input.preamble.workListAddress = kEvent4DrawWorkListBase80087288;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(!tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::MoveImageSourceLimit);

    input.moveImageSlotKnown = true;
    input.moveImageSlot = 0u;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(tx.accepted);
    CHECK(tx.status == Event4DrawBodyStatus8001E750::MoveImageReady);
    CHECK(tx.moveImage);
    CHECK(tx.moveImageMode == 0u);
    CHECK(tx.moveImageSourceFunction == kFn8004019C);
    CHECK(tx.moveImageDispatchFunction == kFn80044E2C);
    CHECK(tx.moveImageWidth == 320);
    CHECK(tx.moveImageHeight == 240);
    CHECK(tx.moveImageTransaction.accepted);
    CHECK(tx.preamble.accepted);
    CHECK(tx.preamble.drawBufferSlot == 0u);
    CHECK(tx.moveImageTransaction.sourceKnown);
    CHECK(tx.moveImageTransaction.drawBufferSlot == 0u);
    CHECK(tx.moveImageTransaction.rectX == 0);
    CHECK(tx.moveImageTransaction.rectY == 0);
    CHECK(tx.moveImageTransaction.rectWidth == 320);
    CHECK(tx.moveImageTransaction.rectHeight == 240);
    CHECK(tx.moveImageTransaction.destX == 0u);
    CHECK(tx.moveImageTransaction.destY == 240u);
    CHECK(tx.moveImageTransaction.rectGlobalValue == 0u);
    CHECK(tx.moveImageTransaction.destGlobalValue == 0x00F00000u);
    CHECK(tx.moveImageTransaction.sizeGlobalValue == 0x00F00140u);
    CHECK(tx.moveImageTransaction.packetAddress == 0x8005D7DCu);
    CHECK(tx.moveImageTransaction.dmaCallbackFunction == 0x80046840u);
    CHECK(tx.moveImageTransaction.dispatchWidth == 20u);
    CHECK(!tx.exactPsxHalParity);

    input.moveImageSlot = 1u;
    input.preamble.drawBufferSlot = 1u;
    input.preamble.packetAllocatorValue = 0x801B54B0u;
    input.preamble.workListAddress =
        kEvent4DrawWorkListBase80087288 + kEvent4DrawWorkListStride8001E750;
    tx = BuildEvent4DrawBodyTransaction8001E750(input);
    CHECK(tx.accepted);
    CHECK(tx.preamble.drawBufferSlot == 1u);
    CHECK(tx.moveImageTransaction.rectY == 240);
    CHECK(tx.moveImageTransaction.destY == 0u);
    CHECK(tx.moveImageTransaction.rectGlobalValue == 0x00F00000u);
    CHECK(tx.moveImageTransaction.destGlobalValue == 0u);
}

void TestEvent4ModalOwnerSoftwareTransactions() {
    Event4ModalInitInput800267C8 initInput{};
    auto init = BuildEvent4ModalInitSoftware800267C8(initInput);
    CHECK(!init.accepted);
    initInput.outputPointerKnown = true;
    initInput.outputPointer = 0x80123456u;
    init = BuildEvent4ModalInitSoftware800267C8(initInput);
    CHECK(init.accepted);
    CHECK(init.outputWrite);
    CHECK(init.outputPointer == 0x80123456u);
    CHECK(init.outputValue == -1);
    CHECK(init.remainingAfter == kEvent4ModalInitialRemaining);
    CHECK(init.result == -1);

    Event4ModalAudioProbe probe{};
    Event4ModalInputSinks80025F0C inputSinks{ProbeEvent4ModalSfx, &probe};
    auto input = ExecuteEvent4ModalInputSoftware80025F0C(0x40u, inputSinks);
    CHECK(input.accepted);
    CHECK(input.status == Event4ModalInputStatus80025F0C::CrossAccepted);
    CHECK(input.outputValue == 0);
    CHECK(input.result == 1);
    CHECK(input.sfxInputMask == 0x20u);
    CHECK(probe.sfxCalls == 1u);

    input = ExecuteEvent4ModalInputSoftware80025F0C(0x20u, inputSinks);
    CHECK(input.accepted);
    CHECK(input.status == Event4ModalInputStatus80025F0C::CircleAccepted);
    CHECK(input.outputValue == 1);
    CHECK(input.result == 2);
    CHECK(input.sfxInputMask == 0x40u);
    CHECK(probe.sfxCalls == 2u);

    input = ExecuteEvent4ModalInputSoftware80025F0C(0x60u, inputSinks);
    CHECK(input.accepted);
    CHECK(input.status == Event4ModalInputStatus80025F0C::IgnoredInput);
    CHECK(!input.outputWrite);
    CHECK(probe.sfxCalls == 2u);

    Event4ModalTickSinks80025E6C tickSinks{
        ProbeEvent4ModalCue, ProbeEvent4ModalFlush, &probe};
    Event4ModalTickInput80025E6C tickInput{};
    tickInput.remainingKnown = true;
    tickInput.remaining = 2u;
    tickInput.cueFrameKnown = true;
    tickInput.cueFrame = 0u;
    tickInput.cueGlobalAddressKnown = true;
    tickInput.cueGlobalAddress = kCueEvent4_8009441C;
    auto tick = ExecuteEvent4ModalTickSoftware80025E6C(tickInput, tickSinks);
    CHECK(tick.accepted);
    CHECK(tick.status == Event4ModalTickStatus80025E6C::Advanced);
    CHECK(tick.tickReturn == 1u);
    CHECK(tick.cueIssued);
    CHECK(tick.cueAddress == kCueEvent4_8009441C);
    CHECK(tick.cueFrameAfter == 1u);
    CHECK(tick.remainingAfter == 2u);
    CHECK(probe.cueCalls == 1u);
    CHECK(probe.flushCalls == 1u);

    tickInput.cueFrame = kEvent4ModalCueFrameOffset80025E6C;
    tick = ExecuteEvent4ModalTickSoftware80025E6C(tickInput, tickSinks);
    CHECK(tick.accepted);
    CHECK(tick.cueAddress == kCueEvent4_8009441C +
                               kEvent4ModalSecondCueByteOffset80025E6C);
    CHECK(tick.remainingAfter == 1u);
    CHECK(tick.cueFrameAfter == 37u);
    CHECK(probe.cueCalls == 2u);
    CHECK(probe.flushCalls == 2u);

    tickInput.cueFrame = 72u;
    tick = ExecuteEvent4ModalTickSoftware80025E6C(tickInput, tickSinks);
    CHECK(tick.accepted);
    CHECK(!tick.cueIssued);
    CHECK(tick.cueFrameAfter == 0u);
    CHECK(tick.tickReturn == 73u);
    CHECK(tick.remainingAfter == 2u);

    tickInput.remaining = 0u;
    tickInput.cueFrame = 36u;
    tick = ExecuteEvent4ModalTickSoftware80025E6C(tickInput, tickSinks);
    CHECK(tick.accepted);
    CHECK(tick.status == Event4ModalTickStatus80025E6C::Idle);
    CHECK(!tick.cueIssued);
    CHECK(tick.cueFrameAfter == 36u);

    tickInput.remaining = 2u;
    tickInput.cueGlobalAddress = 0x80094420u;
    tick = ExecuteEvent4ModalTickSoftware80025E6C(tickInput, tickSinks);
    CHECK(!tick.accepted);
    CHECK(tick.status == Event4ModalTickStatus80025E6C::CueBindingMissing);

    tickInput.cueGlobalAddress = kCueEvent4_8009441C;
    Event4ModalTickSinks80025E6C noSinks{};
    tick = ExecuteEvent4ModalTickSoftware80025E6C(tickInput, noSinks);
    CHECK(!tick.accepted);
    CHECK(tick.status == Event4ModalTickStatus80025E6C::CueSinkMissing);
}

void TestEvent4ModalRuntimeWiringContract() {
    DispatcherPreLoopPadReleaseState80026B94 releaseGate{};
    DispatcherPadChangeState80026744 padChange{};
    DispatcherTailState80026B94 tail{};
    Event4ModalAudioProbe probe{};
    const Event4ModalInputSinks80025F0C sinks{
        ProbeEvent4ModalSfx,
        &probe};

    auto release = StepDispatcherPreLoopPadRelease80026B94(
        releaseGate,
        0x40u);
    CHECK(release.kind ==
          DispatcherPreLoopPadReleaseStepKind80026B94::WaitingForRelease);
    release = StepDispatcherPreLoopPadRelease80026B94(releaseGate, 0u);
    CHECK(release.kind ==
          DispatcherPreLoopPadReleaseStepKind80026B94::ReleasedEnterLoop);

    auto pad = StepDispatcherPadChange80026744(padChange, 0u);
    CHECK(!pad.changed);
    pad = StepDispatcherPadChange80026744(padChange, 0x60u);
    CHECK(pad.changed);
    auto input = ExecuteEvent4ModalInputSoftware80025F0C(
        pad.changedPadMask80026744,
        sinks);
    CHECK(input.accepted);
    CHECK(input.status == Event4ModalInputStatus80025F0C::IgnoredInput);
    CHECK(!input.outputWrite);
    CHECK(probe.sfxCalls == 0u);

    pad = StepDispatcherPadChange80026744(padChange, 0x60u);
    CHECK(!pad.changed);
    pad = StepDispatcherPadChange80026744(padChange, 0x40u);
    CHECK(pad.changed);
    input = ExecuteEvent4ModalInputSoftware80025F0C(
        pad.changedPadMask80026744,
        sinks);
    CHECK(input.accepted);
    CHECK(input.status == Event4ModalInputStatus80025F0C::CrossAccepted);
    CHECK(input.outputValue == 0);
    CHECK(input.result == 1);
    CHECK(probe.sfxCalls == 1u);
    CHECK(ArmDispatcherTail80026B94(tail, input.result));

    for (int i = 0; i < kDispatcherResultTailFrames80026B94; ++i) {
        const DispatcherTailStep80026B94 held =
            StepDispatcherTail80026B94(tail);
        CHECK(held.kind == DispatcherTailStepKind80026B94::HoldFrame);
        CHECK(held.inputClosed);
        CHECK(held.releasedResult == 0);
    }
    const DispatcherTailStep80026B94 released =
        StepDispatcherTail80026B94(tail);
    CHECK(released.kind == DispatcherTailStepKind80026B94::ReleaseResult);
    CHECK(released.releasedResult == 1);
}

void TestEvent4FrameTransaction80026B94() {
    Event4FrameTransaction80026B94 state{};
    Event4FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = kCtxEvent4_8006ED74;
    begin.logicFrame = 12u;
    CHECK(BeginEvent4FrameTransaction80026B94(state, begin));
    CHECK(state.active);
    CHECK(state.commandCount == kEvent4FrameCommandCount80026B94);
    CHECK(state.commands[0].kind ==
          Event4FrameCommandKind80026B94::Tick80025E6C);
    CHECK(state.commands[0].psxFunction == kFn80025E6C);
    CHECK(state.commands[1].kind ==
          Event4FrameCommandKind80026B94::DrawRoute8001E750);
    CHECK(state.commands[1].arg0 == 4);
    CHECK(state.commands[1].arg1Known);
    CHECK(state.commands[1].arg1 == kCtxEvent4_8006ED74);
    CHECK(state.commands[2].kind ==
          Event4FrameCommandKind80026B94::WaitFrame80035560);
    CHECK(state.commands[3].kind ==
          Event4FrameCommandKind80026B94::EndFrame8001EA00);
    CHECK(state.commands[3].arg0 == 4);
    CHECK(state.commands[4].kind ==
          Event4FrameCommandKind80026B94::TextFlush800436F0);
    CHECK(state.commands[4].arg0 == -1);

    CHECK(!CommitEvent4FrameTick80025E6C(
        state, begin.logicFrame, kCtxEvent3_800544F8));
    CHECK(CommitEvent4FrameTick80025E6C(
        state, begin.logicFrame, kCtxEvent4_8006ED74));
    CHECK(!CanAdvanceEvent4FrameLogic80026B94(state));
    CHECK(CanSubmitEvent4FrameDraw8001E750(state, 12u, false));
    CHECK(CommitEvent4FrameDrawAndBindFormalTail80026B94(
        state, 12u, false));
    CHECK(state.directDrawCompleteWithinLimits);
    CHECK(!CanAdvanceEvent4FrameLogic80026B94(state));
    CHECK(!BeginEvent4FrameTransaction80026B94(state, begin));

    CHECK(!CommitEvent4FrameHostBoundary80026B94(state, true, false, true));
    CHECK(!state.hostFrameBoundaryExecuted);
    CHECK(CommitEvent4FrameHostBoundary80026B94(state, true, true, true));
    CHECK(state.hostFrameBoundaryExecuted);
    CHECK(CanAdvanceEvent4FrameLogic80026B94(state));

    Event4FrameTransactionBegin80026B94 tailBegin{};
    tailBegin.requestBound = true;
    tailBegin.ctxAddress = kCtxEvent4_8006ED74;
    tailBegin.logicFrame = 13u;
    tailBegin.frameClass = Event4FrameClass80026B94::ClosedInputTail;
    tailBegin.inputClosed = true;
    tailBegin.tailFramesRemainingBefore = 2;
    tailBegin.tailFramesRemainingAfter = 1;
    ResetEvent4FrameTransaction80026B94(state);
    CHECK(BeginEvent4FrameTransaction80026B94(state, tailBegin));

    Event4FrameTransactionBegin80026B94 bad = begin;
    bad.ctxAddress = kCtxEvent3_800544F8;
    ResetEvent4FrameTransaction80026B94(state);
    CHECK(!BeginEvent4FrameTransaction80026B94(state, bad));
}

void TestEvent4FrameRuntimeSequence80026B94() {
    Event4FrameTransaction80026B94 transaction{};
    DispatcherTailState80026B94 tail{};
    CHECK(ArmDispatcherTail80026B94(tail, 2));

    uint32_t logicFrame = 100u;
    Event4FrameTransactionBegin80026B94 begin{};
    begin.requestBound = true;
    begin.ctxAddress = kCtxEvent4_8006ED74;
    begin.logicFrame = logicFrame;
    begin.frameClass = Event4FrameClass80026B94::ResultAction;
    begin.tailFramesRemainingBefore = kDispatcherResultTailFrames80026B94;
    begin.tailFramesRemainingAfter = kDispatcherResultTailFrames80026B94;
    CHECK(BeginEvent4FrameTransaction80026B94(transaction, begin));
    CHECK(CommitEvent4FrameTick80025E6C(
        transaction, logicFrame, kCtxEvent4_8006ED74));
    CHECK(CommitEvent4FrameDrawAndBindFormalTail80026B94(
        transaction, logicFrame, false));
    CHECK(CommitEvent4FrameHostBoundary80026B94(
        transaction, true, true, true));

    for (int32_t expectedBefore = kDispatcherResultTailFrames80026B94;
         expectedBefore > 0;
         --expectedBefore) {
        DispatcherTailStep80026B94 step{};
        CHECK(TryStepEvent4DispatcherTail80026B94(
            transaction, tail, step));
        CHECK(step.kind == DispatcherTailStepKind80026B94::HoldFrame);
        CHECK(step.inputClosed);
        CHECK(step.framesRemainingBefore == expectedBefore);
        CHECK(step.framesRemainingAfter == expectedBefore - 1);

        ++logicFrame;
        begin = {};
        begin.requestBound = true;
        begin.ctxAddress = kCtxEvent4_8006ED74;
        begin.logicFrame = logicFrame;
        begin.frameClass = Event4FrameClass80026B94::ClosedInputTail;
        begin.inputClosed = true;
        begin.tailFramesRemainingBefore = step.framesRemainingBefore;
        begin.tailFramesRemainingAfter = step.framesRemainingAfter;
        CHECK(BeginEvent4FrameTransaction80026B94(transaction, begin));
        CHECK(CommitEvent4FrameTick80025E6C(
            transaction, logicFrame, kCtxEvent4_8006ED74));
        CHECK(CommitEvent4FrameDrawAndBindFormalTail80026B94(
            transaction, logicFrame, false));
        CHECK(CommitEvent4FrameHostBoundary80026B94(
            transaction, true, true, true));
    }

    DispatcherTailStep80026B94 released{};
    CHECK(TryStepEvent4DispatcherTail80026B94(
        transaction, tail, released));
    CHECK(released.kind == DispatcherTailStepKind80026B94::ReleaseResult);
    CHECK(released.releasedResult == 2);
    CHECK(!tail.active);
    CHECK(CanAdvanceEvent4FrameLogic80026B94(transaction));
}

void TestPracticeSelfLoopAndFailClosedCtx() {
    const EventFramePlan loop =
        BuildPracticeSelfLoop8002776CPlan(kScene0WorkAddress, 2);
    CHECK(loop.owner == EventFrameOwner::PracticeSelfLoop);
    CHECK(loop.eventId == 16);

    const EventFrameActionKind expected[] = {
        EventFrameActionKind::GatePracticeSelfLoopSource8002776C,
        EventFrameActionKind::PracticeSelfLoop,
        EventFrameActionKind::DrawRoute8001E750,
        EventFrameActionKind::WaitFrame80035560,
        EventFrameActionKind::EndFrame8001EA00,
        EventFrameActionKind::PracticeReload80015590,
        EventFrameActionKind::Gap,
    };
    CheckOrder(loop, expected, sizeof(expected) / sizeof(expected[0]));

    const EventFrameAction* gate =
        FindAction(loop, EventFrameActionKind::GatePracticeSelfLoopSource8002776C);
    CHECK(gate != nullptr);
    CHECK(gate->ctxAddress == kScene0WorkAddress);
    CHECK(gate->args[0] == static_cast<int32_t>(kScene0WorkAddress));
    CHECK(gate->args[1] == 2);
    CHECK(gate->args[2] == static_cast<int32_t>(kScene0WorkAddress));
    CHECK(gate->args[3] == 1);

    const EventFrameAction* draw =
        FindAction(loop, EventFrameActionKind::DrawRoute8001E750);
    CHECK(draw != nullptr);
    CHECK(draw->auxAddress == kFn80023618);

    const EventFrameAction* wait =
        FindAction(loop, EventFrameActionKind::WaitFrame80035560);
    CHECK(wait != nullptr);
    CHECK(wait->args[0] == 2);

    const EventFrameAction* reload =
        FindAction(loop, EventFrameActionKind::PracticeReload80015590);
    CHECK(reload != nullptr);
    CHECK(reload->psxFunction == kFn80015590);
    CHECK(reload->args[0] == 2);

    const EventFramePlan wrong =
        BuildPracticeSelfLoop8002776CPlan(0x12345678u, 4);
    const EventFrameAction* wrongGate =
        FindAction(wrong, EventFrameActionKind::GatePracticeSelfLoopSource8002776C);
    CHECK(wrongGate != nullptr);
    CHECK(wrongGate->ctxAddress == 0);
    CHECK(wrongGate->args[0] == 0x12345678);
    CHECK(wrongGate->args[1] == 4);
    CHECK(wrongGate->args[2] == static_cast<int32_t>(kScene0WorkAddress));
    CHECK(wrongGate->args[3] == 0);
    CHECK(CountActions(wrong, EventFrameActionKind::Gap) >
          CountActions(loop, EventFrameActionKind::Gap));
}

void TestCardDrawIdsAndUnknownEventsFailClosed() {
    const EventFramePlan card = BuildCardDrawIds80020F94Plan();
    CHECK(card.owner == EventFrameOwner::DrawOnly);
    CHECK(CountActions(card, EventFrameActionKind::CardDrawIdOnly) == 3);
    CHECK(CountActions(card, EventFrameActionKind::DrawRoute8001E750) == 3);
    CHECK(CountActions(card, EventFrameActionKind::Gap) == 1);

    const EventFrameAction* firstCard =
        FindAction(card, EventFrameActionKind::CardDrawIdOnly);
    CHECK(firstCard != nullptr);
    CHECK(firstCard->eventId == 7);
    CHECK(firstCard->psxFunction == kFn80020F94);

    const EventFramePlan unknownDispatcher = BuildDispatcher80026B94Plan(99);
    CHECK(unknownDispatcher.owner == EventFrameOwner::Unknown);
    CHECK(unknownDispatcher.count == 1);
    CHECK(unknownDispatcher.actions[0].kind == EventFrameActionKind::Gap);
    CHECK(unknownDispatcher.actions[0].psxFunction == kFn80026B94);

    const EventFramePlan unknownDraw = BuildDrawRoute8001E750Plan(99);
    CHECK(unknownDraw.owner == EventFrameOwner::Unknown);
    CHECK(unknownDraw.count == 1);
    CHECK(unknownDraw.actions[0].kind == EventFrameActionKind::Gap);
    CHECK(unknownDraw.actions[0].psxFunction == kFn8001E750);
}

} // namespace

int main() {
    TestRuntimeGateFalse();
    TestSpecTablesAndDrawRoutes();
    TestEventTableSpecsMatchCurrentIdaDispatcher();
    TestDispatcherMainDirectoryTail();
    TestDispatcherResultTailRuntime();
    TestEvent17InitialInputTimeoutRuntime();
    TestEvent17FrameTransactionRuntime();
    TestEvent17EndFrameOwnerPlan8001EA00();
    TestEvent6ArgSourceGate();
    TestEvent6FrameTransactionRuntime();
    TestEvent6PendingFrameBlocksLogicAndTail();
    TestEvent6EndFrameOwnerPlan8001EA00();
    TestEvent3FrameTransactionRuntime();
    TestEvent2FrameTransactionRuntime();
    TestDispatcherPadChange80026744Runtime();
    TestDrawRoutePromptAndDrawOnlyOwners();
    TestEvent4DrawBodySoftwareTransaction();
    TestEvent4ModalOwnerSoftwareTransactions();
    TestEvent4ModalRuntimeWiringContract();
    TestEvent4FrameTransaction80026B94();
    TestEvent4FrameRuntimeSequence80026B94();
    TestPracticeSelfLoopAndFailClosedCtx();
    TestCardDrawIdsAndUnknownEventsFailClosed();

    if (g_failed != 0) {
        std::printf("test_ss0_event_frame_loop: failed checks=%d\n",
                    g_failed);
        return 1;
    }

    std::printf("test_ss0_event_frame_loop: ok\n");
    return 0;
}
