#include "pr/pr_ss0_practice_lifecycle_direct.h"

#include <cstdio>
#include <limits>

using namespace PrSS0PracticeLifecycleDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

const PracticeAction* FindAction(const PracticePlan& plan,
                                 PracticeActionKind kind,
                                 uint32_t start = 0) {
    for (uint32_t i = start; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            return &plan.actions[i];
        }
    }
    return nullptr;
}

uint32_t CountActions(const PracticePlan& plan, PracticeActionKind kind) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            ++count;
        }
    }
    return count;
}

uint32_t IndexOfAction(const PracticePlan& plan,
                       PracticeActionKind kind,
                       uint32_t start = 0) {
    for (uint32_t i = start; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            return i;
        }
    }
    return plan.count;
}

void CheckOrder(const PracticePlan& plan,
                const PracticeActionKind* kinds,
                uint32_t count) {
    uint32_t start = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t index = IndexOfAction(plan, kinds[i], start);
        CHECK(index < plan.count);
        start = index + 1;
    }
}

const PracticeAction* FindRoundAction(const PracticePlan& plan,
                                      PracticeActionKind kind,
                                      int32_t round,
                                      uint32_t start = 0) {
    for (uint32_t i = start; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind &&
            plan.actions[i].round == round) {
            return &plan.actions[i];
        }
    }
    return nullptr;
}

void TestRuntimeGateFalse() {
    CHECK(!RuntimeCutoverAllowed());
    const PracticePlan plan = BuildPracticeLoop8002776CPlan();
    CHECK(!plan.runtimeCutoverAllowed);
    CHECK(plan.blockedByGap);
}

void TestCueAndRec44Specs() {
    CHECK(KnownPracticeCueSpecCount() == 15);
    CHECK(KnownPracticeRec44SpecCount() == kPracticeRoundCount);

    const PracticeCueSpec* exit8 = FindPracticeCueSpec(PracticeCueKind::VoiceExit8);
    CHECK(exit8 != nullptr);
    CHECK(exit8->address == kCuePracticeExitKind8_8006ECA0);
    CHECK(exit8->frames == 95);
    CHECK(exit8->padStopKind == 8);

    const PracticeRec44Spec& round3 = KnownPracticeRec44SpecAt(3);
    CHECK(round3.recordAddress ==
          kPracticeRec44RecordBase800554D8 + 0x84u);
    CHECK(round3.streamAddress ==
          kPracticeRec44StreamBase800554DC + 0x84u);
    CHECK(round3.pulseSlot == 14);
    CHECK(round3.sentinelSlot == 16);
}

void TestRec44ExactStreamsAndFailClosed() {
    constexpr int32_t pulseSlots[kPracticeRoundCount] = {2, 6, 10, 14};
    for (int32_t round = 0;
         round < static_cast<int32_t>(kPracticeRoundCount);
         ++round) {
        for (int32_t slot = 0;
             slot < static_cast<int32_t>(kPracticeRec44StreamByteCount);
             ++slot) {
            const PracticeRec44StreamValue value =
                ReadPracticeRec44StreamValue(round, slot);
            const int8_t expected =
                slot == pulseSlots[round] ? 1 : (slot == 16 ? -1 : 0);
            CHECK(value.sourceKnown);
            CHECK(value.sourceAddress ==
                  kPracticeRec44StreamBase800554DC +
                      static_cast<uint32_t>(round) * kPracticeRec44Stride +
                      static_cast<uint32_t>(slot));
            CHECK(value.value == expected);
        }
    }

    CHECK(!ReadPracticeRec44StreamValue(-1, 0).sourceKnown);
    CHECK(!ReadPracticeRec44StreamValue(
               static_cast<int32_t>(kPracticeRoundCount), 0)
               .sourceKnown);
    CHECK(!ReadPracticeRec44StreamValue(0, -1).sourceKnown);
    CHECK(!ReadPracticeRec44StreamValue(
               0, static_cast<int32_t>(kPracticeRec44StreamByteCount))
               .sourceKnown);
}

void TestCtxFieldSpecs() {
    CHECK(KnownPracticeCtxFieldCount() == 11);

    const PracticeCtxFieldSpec& flags = KnownPracticeCtxFieldAt(0);
    CHECK(flags.kind == PracticeCtxFieldKind::Flags);
    CHECK(flags.offset == 0x00u);
    CHECK(flags.byteCount == 4u);
    CHECK(flags.producer != nullptr);
    CHECK(flags.consumer != nullptr);

    const PracticeCtxFieldSpec& exitSelected = KnownPracticeCtxFieldAt(2);
    CHECK(exitSelected.kind == PracticeCtxFieldKind::ExitSelected);
    CHECK(exitSelected.offset == 0x48u);
    CHECK(exitSelected.byteCount == 4u);
    CHECK(exitSelected.producer != nullptr);
    CHECK(exitSelected.consumer != nullptr);

    const PracticeCtxFieldSpec& blink = KnownPracticeCtxFieldAt(3);
    CHECK(blink.kind == PracticeCtxFieldKind::Blink);
    CHECK(blink.offset == 0x4Cu);
    CHECK(blink.byteCount == 2u);

    const PracticeCtxFieldSpec& streamA = KnownPracticeCtxFieldAt(7);
    CHECK(streamA.kind == PracticeCtxFieldKind::SeqStreamA);
    CHECK(streamA.offset == 0x94u);
    CHECK(streamA.byteCount == 4u);

    const PracticeCtxFieldSpec& streamB = KnownPracticeCtxFieldAt(10);
    CHECK(streamB.kind == PracticeCtxFieldKind::SeqStreamB);
    CHECK(streamB.offset == 0xA4u);
    CHECK(streamB.byteCount == 4u);
}

void TestResourceLoadSequence() {
    const PracticePlan plan = BuildPracticeResourceLoad80015618Plan();
    CHECK(!plan.runtimeCutoverAllowed);
    CHECK(plan.blockedByGap);
    CHECK(plan.count == 6);

    const PracticeActionKind expected[] = {
        PracticeActionKind::CallLoadYCompo80015618,
        PracticeActionKind::ClearResourceTable80025A34,
        PracticeActionKind::LoadInt8001AC18,
        PracticeActionKind::TimUpload,
        PracticeActionKind::VabLoadPractice,
        PracticeActionKind::Gap,
    };
    CheckOrder(plan, expected, sizeof(expected) / sizeof(expected[0]));

    CHECK(plan.actions[0].psxFunction == kFn80015618);
    CHECK(plan.actions[0].address == kYCompoDescriptor800546EC);
    CHECK(plan.actions[2].psxFunction == kFn8001AC18);
    CHECK(plan.actions[2].args[0] == static_cast<int32_t>(kYCompoDescriptor800546EC));
    CHECK(plan.actions[2].args[1] == 1);
    CHECK(plan.actions[3].args[0] == 91);
    CHECK(plan.actions[4].args[0] == 2);
}

void TestPadStopCueGates() {
    const PracticePlan unknown = BuildPracticePadStop800276ECPlan(PracticeCueKind::Unknown);
    CHECK(unknown.count == 1);
    CHECK(unknown.actions[0].kind == PracticeActionKind::Gap);
    CHECK(unknown.actions[0].psxFunction == kFn800276EC);

    const PracticePlan exit7 = BuildPracticePadStop800276ECPlan(PracticeCueKind::VoiceExit7);
    CHECK(exit7.count == 4);
    const PracticeActionKind expected[] = {
        PracticeActionKind::GatePadStopCueSource800276EC,
        PracticeActionKind::PadStopCom800276EC,
        PracticeActionKind::PlayCue80026EF8,
        PracticeActionKind::Flush80026ECC,
    };
    CheckOrder(exit7, expected, sizeof(expected) / sizeof(expected[0]));
    CHECK(exit7.actions[0].address == kCuePracticeExitKind7_8006ECA8);
    CHECK(exit7.actions[0].args[1] == 88);
    CHECK(exit7.actions[0].args[2] == 7);
    CHECK(exit7.actions[1].args[0] == static_cast<int32_t>(kScene0WorkAddress));
    CHECK(exit7.actions[2].psxFunction == kFn80026EF8);
    CHECK(exit7.actions[3].psxFunction == kFn80026ECC);
}

void TestRoundPlanRec44PromptAndFailClosedRound() {
    const PracticePlan invalid = BuildPracticeRound8002776CPlan(4);
    CHECK(invalid.count == 1);
    CHECK(invalid.actions[0].kind == PracticeActionKind::Gap);
    CHECK(invalid.actions[0].psxFunction == kFn8002776C);

    const PracticePlan round2 = BuildPracticeRound8002776CPlan(2);
    CHECK(round2.count > 20);
    const PracticeActionKind expected[] = {
        PracticeActionKind::GatePracticeRoundRec44Source8002776C,
        PracticeActionKind::GatePadStopCueSource800276EC,
        PracticeActionKind::PadStopCom800276EC,
        PracticeActionKind::AdvanceRec44,
        PracticeActionKind::PlayCue80026EF8,
        PracticeActionKind::PlayCue80026EF8,
        PracticeActionKind::PlayCue80026EF8,
        PracticeActionKind::PollInput80035510,
        PracticeActionKind::DebounceInputGp844,
        PracticeActionKind::JudgeTriangle,
        PracticeActionKind::PlayCue80026EF8,
        PracticeActionKind::FastForwardCross,
    };
    CheckOrder(round2, expected, sizeof(expected) / sizeof(expected[0]));

    const PracticeAction* gate =
        FindAction(round2, PracticeActionKind::GatePracticeRoundRec44Source8002776C);
    CHECK(gate != nullptr);
    CHECK(gate->address == kPracticeRec44RecordBase800554D8 + 0x58u);
    CHECK(gate->args[0] == static_cast<int32_t>(
        kPracticeRec44StreamBase800554DC + 0x58u));
    CHECK(gate->args[1] == 10);
    CHECK(gate->args[2] == 16);

    const PracticeAction* judge = FindAction(round2, PracticeActionKind::JudgeTriangle);
    CHECK(judge != nullptr);
    CHECK(judge->address == kPracticeRoundTargetTable80055484);
    CHECK(judge->cue == PracticeCueKind::RoundPrompt2);
    CHECK(judge->args[1] == static_cast<int32_t>(kCuePracticeRoundPromptTable80055494 + 0x0Cu));

    const PracticeAction* fastForward =
        FindAction(round2, PracticeActionKind::FastForwardCross);
    CHECK(fastForward != nullptr);
    CHECK(fastForward->args[0] == 135);
    CHECK(fastForward->args[1] == 1);
}

void TestAllRoundCueRec44Mappings() {
    const PracticeCueKind introCues[] = {
        PracticeCueKind::VoiceIntro0,
        PracticeCueKind::VoiceIntro1,
        PracticeCueKind::VoiceIntro2,
        PracticeCueKind::VoiceIntro3,
    };
    const PracticeCueKind promptCues[] = {
        PracticeCueKind::RoundPrompt0,
        PracticeCueKind::RoundPrompt1,
        PracticeCueKind::RoundPrompt2,
        PracticeCueKind::RoundPrompt3,
    };
    const uint32_t recOffsets[] = {0x00u, 0x2Cu, 0x58u, 0x84u};
    const int32_t pulseSlots[] = {2, 6, 10, 14};

    for (uint32_t round = 0; round < kPracticeRoundCount; ++round) {
        const PracticePlan plan = BuildPracticeRound8002776CPlan(
            static_cast<int32_t>(round));
        const PracticeAction* gate = FindAction(
            plan,
            PracticeActionKind::GatePracticeRoundRec44Source8002776C);
        CHECK(gate != nullptr);
        CHECK(gate->round == static_cast<int32_t>(round));
        CHECK(gate->address ==
              kPracticeRec44RecordBase800554D8 + recOffsets[round]);
        CHECK(gate->args[0] ==
              static_cast<int32_t>(kPracticeRec44StreamBase800554DC +
                                   recOffsets[round]));
        CHECK(gate->args[1] == pulseSlots[round]);
        CHECK(gate->args[2] == 16);

        const PracticeAction* introGate = FindRoundAction(
            plan,
            PracticeActionKind::GatePadStopCueSource800276EC,
            static_cast<int32_t>(round));
        CHECK(introGate != nullptr);
        CHECK(introGate->cue == introCues[round]);

        const PracticeAction* promptJudge = FindAction(
            plan,
            PracticeActionKind::JudgeTriangle);
        CHECK(promptJudge != nullptr);
        CHECK(promptJudge->round == static_cast<int32_t>(round));
        CHECK(promptJudge->cue == promptCues[round]);
        CHECK(promptJudge->args[1] ==
              static_cast<int32_t>(kCuePracticeRoundPromptTable80055494 +
                                   round * 0x06u));

        const PracticeAction* fastForward = FindAction(
            plan,
            PracticeActionKind::FastForwardCross);
        CHECK(fastForward != nullptr);
        CHECK(fastForward->round == static_cast<int32_t>(round));
        CHECK(fastForward->args[0] == 135);
        CHECK(fastForward->args[1] == 1);
    }
}

void TestDrawTailAndLoopCtxGates() {
    const PracticePlan draw = BuildPracticeDrawFramePlan(kScene0WorkAddress);
    const PracticeActionKind drawExpected[] = {
        PracticeActionKind::ResetEventText80024E98,
        PracticeActionKind::DrawEvent16_8001E750,
        PracticeActionKind::DrawPractice80023618,
        PracticeActionKind::WaitFrame80035560_2,
        PracticeActionKind::EndFrame8001EA00_0,
    };
    CheckOrder(draw, drawExpected, sizeof(drawExpected) / sizeof(drawExpected[0]));
    CHECK(draw.actions[1].args[0] == 16);
    CHECK(draw.actions[1].args[1] == static_cast<int32_t>(kScene0WorkAddress));
    CHECK(draw.actions[3].args[0] == 2);
    CHECK(draw.actions[4].args[0] == 0);

    const PracticePlan loop = BuildPracticeLoop8002776CPlan(kScene0WorkAddress, 0);
    CHECK(CountActions(loop, PracticeActionKind::GatePracticeLoopCtxSource8002776C) == 1);
    CHECK(CountActions(loop, PracticeActionKind::DrawEvent16_8001E750) == kPracticeRoundCount);
    CHECK(CountActions(loop, PracticeActionKind::DrawPractice80023618) == kPracticeRoundCount);
    CHECK(CountActions(loop, PracticeActionKind::RestorePrevScene80015590) == 1);

    const PracticeAction* ctxGate =
        FindAction(loop, PracticeActionKind::GatePracticeLoopCtxSource8002776C);
    CHECK(ctxGate != nullptr);
    CHECK(ctxGate->address == kScene0WorkAddress);
    CHECK(ctxGate->args[0] == static_cast<int32_t>(kScene0WorkAddress));
    CHECK(ctxGate->args[1] == 0);
    CHECK(ctxGate->args[2] == static_cast<int32_t>(kScene0WorkAddress));
    CHECK(ctxGate->args[3] == 1);

    const PracticeAction* exitLoop = FindAction(loop, PracticeActionKind::ExitConfirmLoop);
    CHECK(exitLoop != nullptr);
    CHECK(exitLoop->address == kScene0WorkAddress);
    CHECK(exitLoop->frame == static_cast<int32_t>(kPracticeExitConfirmFrames));
    CHECK(exitLoop->args[0] == 0x20);
    CHECK(exitLoop->args[1] == 0x40);

    const PracticeAction* restore =
        FindAction(loop, PracticeActionKind::RestorePrevScene80015590);
    CHECK(restore != nullptr);
    CHECK(restore->psxFunction == kFn80015590);
    CHECK(restore->args[0] == 0);

    const PracticePlan wrongCtx = BuildPracticeLoop8002776CPlan(0x12345678u, 2);
    const PracticeAction* wrongGate =
        FindAction(wrongCtx, PracticeActionKind::GatePracticeLoopCtxSource8002776C);
    CHECK(wrongGate != nullptr);
    CHECK(wrongGate->address == 0);
    CHECK(wrongGate->args[0] == 0x12345678);
    CHECK(wrongGate->args[1] == 2);
    CHECK(wrongGate->args[2] == static_cast<int32_t>(kScene0WorkAddress));
    CHECK(wrongGate->args[3] == 0);
    CHECK(CountActions(wrongCtx, PracticeActionKind::Gap) >
          CountActions(loop, PracticeActionKind::Gap));
}

void TestExitConfirmRuntimeWaitsForChoiceAndRestoresAfter15Loops() {
    PracticeExitConfirmRuntime8002776C runtime{};
    CHECK(BeginPracticeExitConfirm8002776C(runtime, 0));
    CHECK(runtime.initialized);
    CHECK(!runtime.choiceArmed);
    CHECK(runtime.framesRemaining == 15);
    CHECK(runtime.exitSelection48 == 0);
    CHECK(runtime.exitBlink4C == 0);
    CHECK(runtime.blinkCounter == 0u);
    CHECK(runtime.result == PracticeExitConfirmResult8002776C::Pending);

    PracticeExitConfirmInput8002776C unknown{};
    CHECK(!TickPracticeExitConfirm8002776C(runtime, unknown));
    CHECK(runtime.framesRemaining == 15);
    CHECK(runtime.blinkCounter == 0u);

    PracticeExitConfirmInput8002776C idle{};
    idle.edgeKnown = true;
    for (int loop = 0; loop < 40; ++loop) {
        CHECK(TickPracticeExitConfirm8002776C(runtime, idle));
        CHECK(!runtime.choiceArmed);
        CHECK(runtime.framesRemaining == 15);
        CHECK(runtime.result == PracticeExitConfirmResult8002776C::Pending);
    }
    CHECK(runtime.exitBlink4C == 0);
    CHECK(runtime.blinkCounter == 0u);

    PracticeExitConfirmInput8002776C exit{};
    exit.edgeKnown = true;
    exit.edge = 0x40u;
    CHECK(TickPracticeExitConfirm8002776C(runtime, exit));
    CHECK(runtime.choiceArmed);
    CHECK(!runtime.retrySelected);
    CHECK(runtime.framesRemaining == 14);
    CHECK(runtime.exitSelection48 == 1);
    CHECK(runtime.result == PracticeExitConfirmResult8002776C::Pending);

    for (int loop = 0; loop < 13; ++loop) {
        CHECK(TickPracticeExitConfirm8002776C(runtime, idle));
        CHECK(runtime.result == PracticeExitConfirmResult8002776C::Pending);
    }
    CHECK(runtime.framesRemaining == 1);
    CHECK(TickPracticeExitConfirm8002776C(runtime, idle));
    CHECK(runtime.framesRemaining == 0);
    CHECK(runtime.result ==
          PracticeExitConfirmResult8002776C::RestorePreviousScene);
    CHECK(!TickPracticeExitConfirm8002776C(runtime, idle));
}

void TestExitConfirmNoncanonicalBlinkNormalizesLike80027F30() {
    constexpr int16_t kInitialValues[] = {
        (std::numeric_limits<int16_t>::min)(), -7, 0, 1, 2,
        (std::numeric_limits<int16_t>::max)(),
    };
    PracticeExitConfirmInput8002776C idle{};
    idle.edgeKnown = true;

    for (const int16_t initial : kInitialValues) {
        PracticeExitConfirmRuntime8002776C runtime{};
        CHECK(BeginPracticeExitConfirm8002776C(runtime, initial));
        CHECK(runtime.initialized);
        CHECK(runtime.exitBlink4C == initial);
        runtime.exitSelection48 = 2;
        CHECK(TickPracticeExitConfirm8002776C(runtime, idle));
        CHECK(runtime.exitSelection48 == 2);
        CHECK(runtime.exitBlink4C == (initial == 1 ? 0 : 1));
        CHECK(runtime.blinkCounter == 1u);
    }
}

void TestExitConfirmRuntimeRetryAndLateEdgeOverride() {
    PracticeExitConfirmRuntime8002776C runtime{};
    CHECK(BeginPracticeExitConfirm8002776C(runtime, 0));

    PracticeExitConfirmInput8002776C retry{};
    retry.edgeKnown = true;
    retry.edge = 0x20u;
    CHECK(TickPracticeExitConfirm8002776C(runtime, retry));
    CHECK(runtime.choiceArmed);
    CHECK(runtime.retrySelected);
    CHECK(runtime.framesRemaining == 14);
    CHECK(runtime.exitSelection48 == 0);

    PracticeExitConfirmInput8002776C exit{};
    exit.edgeKnown = true;
    exit.edge = 0x40u;
    CHECK(TickPracticeExitConfirm8002776C(runtime, exit));
    CHECK(!runtime.retrySelected);
    CHECK(runtime.framesRemaining == 13);
    CHECK(runtime.exitSelection48 == 1);

    CHECK(TickPracticeExitConfirm8002776C(runtime, retry));
    CHECK(runtime.retrySelected);
    CHECK(runtime.framesRemaining == 12);
    CHECK(runtime.exitSelection48 == 1);

    PracticeExitConfirmInput8002776C idle{};
    idle.edgeKnown = true;
    for (int loop = 0; loop < 12; ++loop) {
        CHECK(TickPracticeExitConfirm8002776C(runtime, idle));
    }
    CHECK(runtime.framesRemaining == 0);
    CHECK(runtime.result == PracticeExitConfirmResult8002776C::RetryPractice);
}

} // namespace

int main() {
    TestRuntimeGateFalse();
    TestCueAndRec44Specs();
    TestRec44ExactStreamsAndFailClosed();
    TestCtxFieldSpecs();
    TestResourceLoadSequence();
    TestPadStopCueGates();
    TestRoundPlanRec44PromptAndFailClosedRound();
    TestAllRoundCueRec44Mappings();
    TestDrawTailAndLoopCtxGates();
    TestExitConfirmRuntimeWaitsForChoiceAndRestoresAfter15Loops();
    TestExitConfirmNoncanonicalBlinkNormalizesLike80027F30();
    TestExitConfirmRuntimeRetryAndLateEdgeOverride();

    if (g_failed != 0) {
        std::printf("test_ss0_practice_lifecycle: failed checks=%d\n",
                    g_failed);
        return 1;
    }

    std::printf("test_ss0_practice_lifecycle: ok\n");
    return 0;
}
