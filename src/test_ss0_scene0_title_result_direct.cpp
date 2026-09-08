#include "pr/pr_ss0_transition_direct.h"

#include <cstdint>
#include <cstdio>
#include <limits>

namespace {

int g_failedChecks = 0;

#define CHECK(expr)                                                       \
    do {                                                                  \
        if (!(expr)) {                                                    \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__, \
                        #expr);                                           \
            ++g_failedChecks;                                             \
        }                                                                 \
    } while (0)

PrSS0TransitionDirect::Scene0TitleResultResolveInput801C4DC4 MakeInput(
    int32_t selectorResult,
    bool currentSceneKnown,
    int32_t currentScene,
    bool previousSceneKnown,
    int32_t previousScene,
    bool drawsKnown,
    const int32_t* draws,
    std::size_t drawCount)
{
    PrSS0TransitionDirect::Scene0TitleResultResolveInput801C4DC4 input{};
    input.selectorResultKnown = true;
    input.selectorResult = selectorResult;
    input.currentSceneKnown = currentSceneKnown;
    input.currentScene = currentScene;
    input.previousSceneKnown = previousSceneKnown;
    input.previousScene = previousScene;
    input.injectedRandDrawsKnown = drawsKnown;
    input.injectedRandDraws = draws;
    input.injectedRandDrawCount = drawCount;
    return input;
}

void CheckValidTitleResults()
{
    using namespace PrSS0TransitionDirect;

    const int32_t drawZero[] = {0};
    const int32_t drawFive[] = {5};
    const int32_t retryDraws[] = {3, 9, 4};

    struct ValidCase {
        Scene0TitleResultResolveInput801C4DC4 input{};
        uint16_t word800916D0 = 0;
        bool publishD0BeforeDraws = false;
        bool cue94410Required = false;
        bool flush26ECCRequired = false;
        int32_t returnScene = 0;
        std::size_t drawCount = 0u;
    };
    const ValidCase cases[] = {
        {MakeInput(
             static_cast<int32_t>(kTitleSelectorResultMenu801C4DC4),
             false,
             0,
             false,
             0,
             false,
             nullptr,
             0u),
         0u,
         false,
         false,
         false,
         -1,
         0u},
        {MakeInput(
             static_cast<int32_t>(kTitleSelectorResultStart801C4DC4),
             true,
             0,
             false,
             0,
             false,
             nullptr,
             0u),
         0u,
         false,
         true,
         true,
         1,
         0u},
        {MakeInput(
             static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
             false,
             0,
             true,
             6,
             true,
             drawZero,
             1u),
         1u,
         true,
         false,
         false,
         1,
         1u},
        {MakeInput(
             static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
             false,
             0,
             true,
             2,
             true,
             drawFive,
             1u),
         1u,
         true,
         false,
         false,
         6,
         1u},
        {MakeInput(
             static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
             false,
             0,
             true,
             4,
             true,
             retryDraws,
             3u),
         1u,
         true,
         false,
         false,
         5,
         3u},
    };

    for (const auto& testCase : cases) {
        const auto result = ResolveScene0TitleResult801C4DC4(testCase.input);
        CHECK(result.status ==
              Scene0TitleResultResolveStatus801C4DC4::Accepted);
        CHECK(result.accepted);
        CHECK(result.word800916D0Known);
        CHECK(result.word800916D0 == testCase.word800916D0);
        CHECK(result.publishWord800916D0BeforeDrawConsumption ==
              testCase.publishD0BeforeDraws);
        CHECK(result.cue94410Required == testCase.cue94410Required);
        CHECK(result.flush26ECCRequired == testCase.flush26ECCRequired);
        CHECK(result.returnSceneKnown);
        CHECK(result.returnScene == testCase.returnScene);
        CHECK(result.drawCount == testCase.drawCount);
    }
}

void CheckRejectedTitleResults()
{
    using namespace PrSS0TransitionDirect;

    const int32_t oneDraw[] = {0};
    const int32_t exhaustedDraws[] = {3, 9};
    const int32_t invalidDraws[] = {3, -1};

    auto missingSelector = MakeInput(1, false, 0, false, 0, false,
                                     nullptr, 0u);
    missingSelector.selectorResultKnown = false;
    const auto invalidSelector = MakeInput(0, false, 0, false, 0, false,
                                           nullptr, 0u);
    const auto missingCurrent = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultStart801C4DC4),
        false, 0, false, 0, false, nullptr, 0u);
    const auto negativeCurrent = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultStart801C4DC4),
        true, -1, false, 0, false, nullptr, 0u);
    const auto overflowingCurrent = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultStart801C4DC4),
        true, std::numeric_limits<int32_t>::max(), false, 0, false,
        nullptr, 0u);
    const auto missingPrevious = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
        false, 0, false, 0, true, oneDraw, 1u);
    const auto missingDrawKnowledge = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
        false, 0, true, 4, false, oneDraw, 1u);
    const auto missingDrawSource = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
        false, 0, true, 4, true, nullptr, 1u);
    const auto emptyDrawSource = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
        false, 0, true, 4, true, nullptr, 0u);
    const auto exhaustedDrawSource = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
        false, 0, true, 4, true, exhaustedDraws, 2u);
    const auto invalidDrawSource = MakeInput(
        static_cast<int32_t>(kTitleSelectorResultRandom801C4DC4),
        false, 0, true, 4, true, invalidDraws, 2u);

    struct RejectedCase {
        Scene0TitleResultResolveInput801C4DC4 input{};
        Scene0TitleResultResolveStatus801C4DC4 status =
            Scene0TitleResultResolveStatus801C4DC4::SourceUnknown;
        bool attractD0Exposed = false;
        std::size_t drawCount = 0u;
    };
    const RejectedCase cases[] = {
        {missingSelector,
         Scene0TitleResultResolveStatus801C4DC4::SourceUnknown,
         false,
         0u},
        {invalidSelector,
         Scene0TitleResultResolveStatus801C4DC4::InvalidInput,
         false,
         0u},
        {missingCurrent,
         Scene0TitleResultResolveStatus801C4DC4::SourceUnknown,
         false,
         0u},
        {negativeCurrent,
         Scene0TitleResultResolveStatus801C4DC4::InvalidInput,
         false,
         0u},
        {overflowingCurrent,
         Scene0TitleResultResolveStatus801C4DC4::InvalidInput,
         false,
         0u},
        {missingPrevious,
         Scene0TitleResultResolveStatus801C4DC4::SourceUnknown,
         true,
         0u},
        {missingDrawKnowledge,
         Scene0TitleResultResolveStatus801C4DC4::SourceUnknown,
         true,
         0u},
        {missingDrawSource,
         Scene0TitleResultResolveStatus801C4DC4::DrawSourceMissing,
         true,
         0u},
        {emptyDrawSource,
         Scene0TitleResultResolveStatus801C4DC4::DrawsExhausted,
         true,
         0u},
        {exhaustedDrawSource,
         Scene0TitleResultResolveStatus801C4DC4::DrawsExhausted,
         true,
         2u},
        {invalidDrawSource,
         Scene0TitleResultResolveStatus801C4DC4::InvalidDraw,
         true,
         2u},
    };

    for (const auto& testCase : cases) {
        const auto result = ResolveScene0TitleResult801C4DC4(testCase.input);
        CHECK(result.status == testCase.status);
        CHECK(!result.accepted);
        CHECK(result.word800916D0Known == testCase.attractD0Exposed);
        CHECK(result.word800916D0 ==
              (testCase.attractD0Exposed ? 1u : 0u));
        CHECK(result.publishWord800916D0BeforeDrawConsumption ==
              testCase.attractD0Exposed);
        CHECK(!result.cue94410Required);
        CHECK(!result.flush26ECCRequired);
        CHECK(!result.returnSceneKnown);
        CHECK(result.drawCount == testCase.drawCount);
    }
}

void CheckSub80027194Cadence()
{
    using namespace PrSS0TransitionDirect;

    CHECK(kSub80027194CounterAddress == 0x8006EC20u);
    CHECK(kSub80027194CuePayloadAddress == 0x8006EC18u);
    CHECK(kSub80027194CueProgram == 0u);
    CHECK(kSub80027194CueNote == 12u);
    CHECK(kSub80027194CueKey == 0x24u);
    CHECK(kSub80027194CueVolume == 0x50u);

    Sub80027194CadenceState state{};
    const int32_t expectedCounter[] = {1, 2, 1, 2, 1};
    const bool expectedCue[] = {false, false, true, false, true};
    for (std::size_t index = 0u; index < 5u; ++index) {
        const int32_t previousCounter = state.counter;
        const auto step = AdvanceSub80027194Cadence(state);
        CHECK(step.known);
        CHECK(step.previousCounter == previousCounter);
        CHECK(step.cueRequired == expectedCue[index]);
        CHECK(step.flushRequired == expectedCue[index]);
        CHECK(step.nextCounter == expectedCounter[index]);
        CHECK(state.counter == expectedCounter[index]);
    }
}

void CheckMainMenuEntryMode4Transition()
{
    using namespace PrSS0TransitionDirect;

    CHECK(HasKnownModeBodyTicks8001EA74(4, false));
    CHECK(KnownModeBodyTicks8001EA74(4, false) == 24);

    const TransitionPlan modePlan = BuildMode8001EA74Plan(4, false);
    CHECK(modePlan.count == 6u);
    CHECK(modePlan.actions[2].kind == TransitionActionKind::Call80021E60);
    CHECK(modePlan.actions[2].psxFunction == kFn80021E60);
    CHECK(modePlan.actions[2].arg0 == 0u);
    CHECK(modePlan.actions[3].kind == TransitionActionKind::Call8001D74C);
    CHECK(modePlan.actions[3].psxFunction == kFn8001D74C);
    CHECK(modePlan.actions[3].arg0 == 5u);
    CHECK(modePlan.actions[4].kind == TransitionActionKind::Call8001FC40);
    CHECK(modePlan.actions[4].psxFunction == kFn8001FC40);
    CHECK(modePlan.actions[4].arg0 == 4u);
    CHECK(modePlan.actions[4].arg1 == 8u);
    CHECK(modePlan.actions[5].kind == TransitionActionKind::Call8001FDC0);
    CHECK(modePlan.actions[5].psxFunction == kFn8001FDC0);
    CHECK(modePlan.actions[5].arg0 == 0u);

    SlowTransitionRuntime80020110 runtime{};
    CHECK(!BeginSlowTransitionRuntime80020110(
        runtime, kScene0WorkAddress, 4, 2, 1));
    CHECK(!BeginSlowTransitionRuntime80020110(runtime, 0u, 4, 1, 2));
    CHECK(!BeginSlowTransitionRuntime80020110(runtime, 0u, 2, 1, 2));
    CHECK(BeginSlowTransitionRuntime80020110(runtime, 0u, 4, 2, 1));
    CHECK(runtime.sceneFrame ==
          kScene0MainMenuEntryTransitionRelativeTickOrigin);

    Sub80027194CadenceState cadence{2};
    uint32_t cadenceCalls = 0u;
    uint32_t cueCalls = 0u;

    for (uint32_t hostTick = 1u; hostTick <= 56u; ++hostTick) {
        const SlowTransitionTickResult80020110 tick =
            TickSlowTransitionRuntime80020110(runtime);
        const bool tail = hostTick > 48u;
        const uint32_t iteration = tail
            ? (hostTick - 49u) / 2u
            : (hostTick - 1u) / 2u;
        const uint32_t expectedActiveCount = tail
            ? 0u
            : static_cast<uint32_t>(kSlowTransitionGridCells8001FDC0) -
                  (iteration + 1u) * 8u;
        CHECK(tick.accepted);
        CHECK(tick.visualFrame.known);
        CHECK(tick.visualFrame.ctxAddress == 0u);
        CHECK(tick.visualFrame.mode == 4);
        CHECK(tick.visualFrame.preFfd4Arg == 2);
        CHECK(tick.visualFrame.postFfd4Arg == 1);
        CHECK(tick.visualFrame.tail == tail);
        CHECK(tick.visualFrame.iteration == iteration);
        CHECK(tick.visualFrame.activeCount == expectedActiveCount);
        CHECK(tick.visualFrame.sourceFunction == kFn8001FDC0);
        uint32_t gridActiveCount = 0u;
        for (uint8_t cell : tick.visualFrame.grid) {
            CHECK(cell <= 1u);
            gridActiveCount += cell;
        }
        CHECK(gridActiveCount == expectedActiveCount);
        const SlowTransitionFramePlan8001FDC0 framePlan =
            BuildSlowTransitionFramePlan8001FDC0(tick.visualFrame);
        CHECK(framePlan.known);
        CHECK(framePlan.commandCount == expectedActiveCount);
        CHECK(tick.sub80027194CallRequired == ((hostTick & 1u) != 0u));
        if (tick.sub80027194CallRequired) {
            const auto cadenceStep = AdvanceSub80027194Cadence(cadence);
            CHECK(cadenceStep.known);
            ++cadenceCalls;
            if (cadenceStep.cueRequired) {
                CHECK(cadenceStep.flushRequired);
                ++cueCalls;
            }
        }
        CHECK(tick.presentRequired == ((hostTick & 1u) == 0u));
    }
    CHECK(!runtime.active);
    CHECK(runtime.phase == SlowTransitionRuntimePhase80020110::Complete);
    CHECK(runtime.loopIterationsCompleted == 24u);
    CHECK(runtime.tailIterationsCompleted == 4u);
    CHECK(runtime.sceneFrame ==
          kScene0MainMenuEntryTransitionRelativeTickOrigin + 56u);
    CHECK(cadenceCalls == 28u);
    CHECK(cueCalls == 14u);
    CHECK(cadence.counter == 2);
}

void CheckCardSaveMode3Transition()
{
    using namespace PrSS0TransitionDirect;

    CHECK(HasKnownModeBodyTicks8001EA74(3, false));
    CHECK(KnownModeBodyTicks8001EA74(3, false) == 24);

    const TransitionPlan modePlan = BuildMode8001EA74Plan(3, false);
    CHECK(modePlan.count == 6u);
    CHECK(modePlan.actions[2].kind == TransitionActionKind::Call80022CBC);
    CHECK(modePlan.actions[2].psxFunction == kFn80022CBC);
    CHECK(modePlan.actions[2].arg0 == 4u);
    CHECK(modePlan.actions[3].kind == TransitionActionKind::Call8001D74C);
    CHECK(modePlan.actions[3].arg0 == 5u);
    CHECK(modePlan.actions[4].kind == TransitionActionKind::Call8001FC40);
    CHECK(modePlan.actions[4].arg0 == 4u);
    CHECK(modePlan.actions[4].arg1 == 8u);
    CHECK(modePlan.actions[5].kind == TransitionActionKind::Call8001FDC0);

    SlowTransitionRuntime80020110 runtime{};
    CHECK(BeginSlowTransitionRuntime80020110(runtime, 0u, 3, 2, 1));
    CHECK(runtime.active);
    CHECK(runtime.ctxAddress == 0u);
    CHECK(runtime.sceneFrame == kScene0MainMenuEntryTransitionRelativeTickOrigin);
    for (uint8_t cell : runtime.tileMask) {
        CHECK(cell == 1u);
    }

    for (uint32_t hostTick = 1u; hostTick <= 56u; ++hostTick) {
        const auto tick = TickSlowTransitionRuntime80020110(runtime);
        CHECK(tick.accepted);
        const bool tail = hostTick > 48u;
        const uint32_t iteration = tail
            ? (hostTick - 49u) / 2u
            : (hostTick - 1u) / 2u;
        const uint32_t expectedActive = tail
            ? 0u
            : static_cast<uint32_t>(kSlowTransitionGridCells8001FDC0) -
                  (iteration + 1u) * 8u;
        CHECK(tick.visualFrame.known);
        CHECK(tick.visualFrame.mode == 3);
        CHECK(tick.visualFrame.preFfd4Arg == 2);
        CHECK(tick.visualFrame.postFfd4Arg == 1);
        CHECK(tick.visualFrame.tail == tail);
        CHECK(tick.visualFrame.iteration == iteration);
        CHECK(tick.visualFrame.activeCount == expectedActive);
        CHECK(tick.visualFrame.sourceFunction == kFn8001FDC0);
        uint32_t activeCount = 0u;
        for (uint8_t cell : tick.visualFrame.grid) {
            CHECK(cell <= 1u);
            activeCount += cell;
        }
        CHECK(activeCount == expectedActive);
        CHECK(tick.presentRequired == ((hostTick & 1u) == 0u));
        CHECK(tick.tileMaskMutationApplied ==
              (!tail ? ((hostTick & 1u) != 0u) : hostTick == 49u));
    }
    CHECK(!runtime.active);
    CHECK(runtime.phase == SlowTransitionRuntimePhase80020110::Complete);
    CHECK(runtime.loopIterationsCompleted == 24u);
    CHECK(runtime.tailIterationsCompleted == 4u);
    CHECK(runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 0u);
    for (uint8_t cell : runtime.tileMask) {
        CHECK(cell == 0u);
    }
}

void CheckFixedTailCallbackBindings()
{
    using namespace PrSS0TransitionDirect;

    const TransitionPlan slow = BuildSlow80020110Plan(
        kScene0WorkAddress, 4, 2, 1, false);
    CHECK(slow.count >= 2u);
    if (slow.count >= 2u) {
        const TransitionAction& slowTail = slow.actions[slow.count - 2u];
        CHECK(slowTail.kind == TransitionActionKind::Call80020008Tail);
        CHECK(slowTail.psxFunction == kFn80020008);
        CHECK(slowTail.arg0 == kScene0WorkAddress);
        CHECK(slowTail.arg1 == 4u);
        CHECK(slowTail.arg2 == kFn8001EA74);
        CHECK(slowTail.arg3 == kFn8001EBF4);
        CHECK(slowTail.repeatCount == 4);
    }

    const TransitionPlan fast = BuildFast800201ACPlan(
        kScene0WorkAddress, 4, 2, 1, false);
    CHECK(fast.count != 0u);
    if (fast.count != 0u) {
        const TransitionAction& fastTail = fast.actions[fast.count - 1u];
        CHECK(fastTail.kind == TransitionActionKind::Call80020090Tail);
        CHECK(fastTail.psxFunction == kFn80020090);
        CHECK(fastTail.arg0 == kScene0WorkAddress);
        CHECK(fastTail.arg1 == 4u);
        CHECK(fastTail.arg2 == kFn8001EA74);
        CHECK(fastTail.arg3 == kFn8001EBF4);
        CHECK(fastTail.repeatCount == 4);
    }
}

void CheckLoadingVariantsAndReset()
{
    using namespace PrSS0TransitionDirect;
    const int16_t sceneModes[] = {1, 1, 2, 3, 1, 2, 3, 4};
    for (int scene = 0; scene < 8; ++scene) {
        int16_t mode = -1;
        CHECK(ResolveSceneEntryLoadingMode801C7284(scene, mode));
        CHECK(mode == sceneModes[scene]);
    }
    int16_t unchanged = 9;
    CHECK(!ResolveSceneEntryLoadingMode801C7284(-1, unchanged));
    CHECK(!ResolveSceneEntryLoadingMode801C7284(8, unchanged));
    CHECK(unchanged == 9);
    CHECK(ResolveLoadingPatternStyle8001EF40(0) == 0u);
    CHECK(ResolveLoadingPatternStyle8001EF40(5) == 0u);

    const uint32_t colors[] = {0x401C0B5Au, 0x40003F1Eu, 0x40AE2A02u, 0x4053005Du};
    std::array<uint8_t, kLoadingPatternGridCells8001EF40> blank{};
    uint64_t hashes[4]{};
    for (int16_t mode = 1; mode <= 4; ++mode) {
        LoadingPatternRuntime8001EF40 runtime{};
        CHECK(BeginLoadingPatternRuntimeAfter8001FFD4(
            runtime, mode, blank.data(), blank.size()));
        LoadingPatternFrame8001EF40 frame{};
        for (uint32_t tick = 0; tick < 192u; ++tick) {
            frame = TickLoadingPatternRuntime8001EF40(runtime);
            CHECK(frame.known);
            CHECK(frame.style == static_cast<uint8_t>(mode - 1));
            CHECK(frame.boxFillAttr8001B6C4 == colors[mode - 1]);
            CHECK(frame.mutationApplied == (tick % 3u == 0u));
        }
        CHECK(runtime.mutationSerial == 64u);
        CHECK(runtime.bitCursorGp49 == 32u);
        CHECK(runtime.wordCursorGp50 == 1u);
        hashes[mode - 1] = frame.liveGridFnv1a;
        StopLoadingPatternRuntime8001EF40(runtime);
        CHECK(!TickLoadingPatternRuntime8001EF40(runtime).known);
        // 8001FFD4 preserves the word/callback counters.
        CHECK(BeginLoadingPatternRuntimeAfter8001FFD4(
            runtime, mode, blank.data(), blank.size()));
        CHECK(runtime.wordCursorGp50 == 1u);
        CHECK(runtime.frameCounterGp51 == 192u);
        // But 8001EF14 (title exit/destination entry) resets all three.
        ResetLoadingPatternState8001EF14(runtime);
        CHECK(runtime.active);
        CHECK(runtime.bitCursorGp49 == 0u);
        CHECK(runtime.wordCursorGp50 == 0u);
        CHECK(runtime.frameCounterGp51 == 0u);
        for (uint8_t cell : runtime.liveGrid) CHECK(cell == 0u);
        CHECK(TickLoadingPatternRuntime8001EF40(runtime).mutationApplied);
    }
    // Modes 1/2/4 end with different dot patterns; mode 3 retains letters.
    CHECK(hashes[0] != hashes[1]);
    CHECK(hashes[1] != hashes[2]);
    CHECK(hashes[2] != hashes[3]);
}

} // namespace

int main()
{
    CHECK(!PrSS0TransitionDirect::RuntimeCutoverAllowed());
    CheckValidTitleResults();
    CheckRejectedTitleResults();
    CheckSub80027194Cadence();
    CheckMainMenuEntryMode4Transition();
    CheckCardSaveMode3Transition();
    CheckFixedTailCallbackBindings();
    CheckLoadingVariantsAndReset();

    if (g_failedChecks != 0) {
        std::printf(
            "test_ss0_scene0_title_result_direct: FAIL (%d checks)\n",
            g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_scene0_title_result_direct: PASS\n");
    return 0;
}
