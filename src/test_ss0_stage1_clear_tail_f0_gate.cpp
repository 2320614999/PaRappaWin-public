#include "pr/pr_stage1_lifecycle_direct.h"
#include "pr/pr_stage1_lifecycle_executor_direct.h"

#include <cstdio>

using namespace PrStage1LifecycleDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

uint32_t CountActions(const StepResult801C81EC& result,
                      ActionKind801C81EC kind) {
    uint32_t count = 0;
    for (const Action801C81EC& action : result.actions) {
        if (action.kind == kind) {
            ++count;
        }
    }
    return count;
}

Runtime801C81EC MakeClearTailRuntime() {
    Runtime801C81EC runtime{};
    runtime.phase = Phase801C81EC::ClearTailMovieRequested;
    runtime.clearTailStageStatusKnown = true;
    runtime.clearTailStageStatus166AC = 2;
    return runtime;
}

void TestStageRunnerPendingDoesNotReenterStageRecordInit() {
    Runtime801C81EC runtime{};
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input{};
    input.sceneId = 1;
    input.word800916D0 = 2; // Native entry bypasses MOVIE1.
    auto step = Step801C81EC(runtime, sceneEntry, input);
    CHECK(CountActions(step, ActionKind801C81EC::StageRecordTick1A4D0) == 1);
    CHECK(CountActions(step, ActionKind801C81EC::StageRunnerRun7A60) == 1);
    CHECK(runtime.phase == Phase801C81EC::StageRunRequested);
    input.word800916D0 = 0;
    for (int frame = 0; frame < 300; ++frame) {
        // A yielded host frame is not a returned PSX function result.
        input.stageResultKnown = false;
        step = Step801C81EC(runtime, sceneEntry, input);
        CHECK(CountActions(step, ActionKind801C81EC::StageRecordTick1A4D0) == 0);
        CHECK(CountActions(step, ActionKind801C81EC::StageRunnerRun7A60) == 1);
        CHECK(CountActions(step, ActionKind801C81EC::QueryAbort26B94) == 0);
        CHECK(runtime.phase == Phase801C81EC::StageRunRequested);
    }
    input.stageResultKnown = true;
    input.stageResult801C7A60 = 2;
    step = Step801C81EC(runtime, sceneEntry, input);
    CHECK(CountActions(step, ActionKind801C81EC::QueryAbort26B94) == 1);
    CHECK(CountActions(step, ActionKind801C81EC::StageRecordTick1A4D0) == 0);
    CHECK(runtime.phase == Phase801C81EC::AbortPollRequested);
    input.stageResultKnown = false;
    input.abortPollResultKnown = true;
    input.abortPollResult26B94 = 1; // Observed native Cross retry; Circle exit is2.
    step = Step801C81EC(runtime, sceneEntry, input);
    CHECK(CountActions(step, ActionKind801C81EC::ResetHoldTiles1EF14) == 1);
    // Native event4 retry falls through to LABEL_5 in this same step.
    CHECK(CountActions(step, ActionKind801C81EC::StageRecordTick1A4D0) == 1);
    CHECK(CountActions(step, ActionKind801C81EC::StageRunnerRun7A60) == 1);
    CHECK(runtime.phase == Phase801C81EC::StageRunRequested);
    input.abortPollResultKnown = false;
    step = Step801C81EC(runtime, sceneEntry, input);
    CHECK(CountActions(step, ActionKind801C81EC::StageRecordTick1A4D0) == 0);
    CHECK(CountActions(step, ActionKind801C81EC::StageRunnerRun7A60) == 1);
    CHECK(runtime.phase == Phase801C81EC::StageRunRequested);
    input.stageResultKnown = true;
    input.stageResult801C7A60 = 1;
    step = Step801C81EC(runtime, sceneEntry, input);
    CHECK(CountActions(step, ActionKind801C81EC::StageRecordTick1A4D0) == 0);
    CHECK(CountActions(step, ActionKind801C81EC::QueryStageStatus166AC) == 1);
    CHECK(runtime.phase == Phase801C81EC::ClearTailStatusRequested);
}

FrameInput801C81EC MakeClearTailInput() {
    FrameInput801C81EC input{};
    input.sceneId = 1;
    input.word800916D0 = 0;
    input.word800916DA = 0;
    input.word80091816 = 1234;
    input.clearTailPlayAndWaitResultKnown = true;
    input.clearTailPlayAndWaitResult = 0;
    return input;
}

void CheckCommonClearTailStatusActions(const StepResult801C81EC& result) {
    CHECK(CountActions(result, ActionKind801C81EC::SaveStatus1635C) == 1);
    CHECK(CountActions(result, ActionKind801C81EC::UnlockNextStage1628C) == 1);
    CHECK(CountActions(result, ActionKind801C81EC::SfxCue26EF8) == 1);
    CHECK(CountActions(result, ActionKind801C81EC::AudioFlush26ECC) == 1);
    CHECK(CountActions(result, ActionKind801C81EC::ResetHoldTiles1EF14) == 2);
}

void CheckNoPostClearSaveActions(const StepResult801C81EC& result) {
    CHECK(CountActions(result, ActionKind801C81EC::Bootstrap15590) == 0);
    CHECK(CountActions(result, ActionKind801C81EC::SaveUi19148) == 0);
}

void TestUnknownF0BlocksClearTailBootstrapAndSaveUi() {
    Runtime801C81EC runtime = MakeClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input = MakeClearTailInput();
    input.word800916F0Known = false;
    input.word800916F0 = 0;

    const StepResult801C81EC result =
        Step801C81EC(runtime, sceneEntry, input);

    CheckCommonClearTailStatusActions(result);
    CheckNoPostClearSaveActions(result);
    CHECK(result.blockedByUnknownWord800916F0);
    CHECK(result.sceneResultKnown);
    CHECK(result.sceneResult == 2);
    CHECK(!PrStage1LifecycleExecutorDirect::
               IsStepSceneResultReleaseReady801C81EC(result));
    CHECK(runtime.phase == Phase801C81EC::ClearTailMovieRequested);
    CHECK(runtime.clearTailStageStatusKnown);
    CHECK(!runtime.clearTailWaitingForWord800916F0);
    CHECK(!runtime.clearTailPreWord800916F0ActionsApplied);
}

void TestPostMovieSceneResultLimitsPreserveTheExactF0Gate() {
    const SceneEntry801C7284 sceneEntry{};

    Runtime801C81EC daOneNextRuntime = MakeClearTailRuntime();
    FrameInput801C81EC daOneNextInput = MakeClearTailInput();
    daOneNextInput.word800916DA = 1;
    daOneNextInput.sceneId = 2;
    daOneNextInput.word800916F0Known = false;
    const StepResult801C81EC daOneNext =
        Step801C81EC(daOneNextRuntime, sceneEntry, daOneNextInput);
    CHECK(!daOneNext.blockedByUnknownWord800916F0);
    CHECK(daOneNext.sceneResultKnown);
    CHECK(daOneNext.sceneResult == 3);
    CHECK(PrStage1LifecycleExecutorDirect::
               IsStepSceneResultReleaseReady801C81EC(daOneNext));
    CheckNoPostClearSaveActions(daOneNext);

    Runtime801C81EC daOneTerminalRuntime = MakeClearTailRuntime();
    FrameInput801C81EC daOneTerminalInput = MakeClearTailInput();
    daOneTerminalInput.word800916DA = 1;
    daOneTerminalInput.sceneId = 3;
    daOneTerminalInput.word800916F0Known = false;
    const StepResult801C81EC daOneTerminal =
        Step801C81EC(
            daOneTerminalRuntime,
            sceneEntry,
            daOneTerminalInput);
    CHECK(!daOneTerminal.blockedByUnknownWord800916F0);
    CHECK(daOneTerminal.sceneResultKnown);
    CHECK(daOneTerminal.sceneResult == 0);
    CHECK(PrStage1LifecycleExecutorDirect::
               IsStepSceneResultReleaseReady801C81EC(daOneTerminal));
    CheckNoPostClearSaveActions(daOneTerminal);

    Runtime801C81EC nextRuntime = MakeClearTailRuntime();
    FrameInput801C81EC nextInput = MakeClearTailInput();
    nextInput.sceneId = 5;
    nextInput.word800916F0Known = false;
    const StepResult801C81EC next =
        Step801C81EC(nextRuntime, sceneEntry, nextInput);
    CHECK(next.blockedByUnknownWord800916F0);
    CHECK(next.sceneResultKnown);
    CHECK(next.sceneResult == 6);
    CHECK(!PrStage1LifecycleExecutorDirect::
               IsStepSceneResultReleaseReady801C81EC(next));
    CheckNoPostClearSaveActions(next);

    Runtime801C81EC terminalRuntime = MakeClearTailRuntime();
    FrameInput801C81EC terminalInput = MakeClearTailInput();
    terminalInput.sceneId = 6;
    terminalInput.word800916F0Known = false;
    const StepResult801C81EC terminal =
        Step801C81EC(terminalRuntime, sceneEntry, terminalInput);
    CHECK(terminal.blockedByUnknownWord800916F0);
    CHECK(terminal.sceneResultKnown);
    CHECK(terminal.sceneResult == 0);
    CHECK(!PrStage1LifecycleExecutorDirect::
               IsStepSceneResultReleaseReady801C81EC(terminal));
    CheckNoPostClearSaveActions(terminal);
}

void TestEasyStage1ClearSkipsNormalRecordsForEveryF0Observation() {
    // Native 801C81EC places score, unlock, bootstrap and SaveUI together
    // under !word_800916DA. EASY proceeds to Stage2 without touching them,
    // even if F0 has not been observed. This is decoder coverage, not a
    // claim that an actual controller-driven EASY clear has been played.
    const SceneEntry801C7284 sceneEntry{};
    for (int observation = 0; observation < 3; ++observation) {
        auto runtime = MakeClearTailRuntime();
        auto input = MakeClearTailInput();
        input.word800916DA = 1;
        input.word800916F0Known = observation != 0;
        input.word800916F0 = observation == 2 ? 1 : 0;
        const auto result = Step801C81EC(runtime, sceneEntry, input);
        CHECK(CountActions(result, ActionKind801C81EC::SaveStatus1635C) == 0);
        CHECK(CountActions(result, ActionKind801C81EC::UnlockNextStage1628C) == 0);
        CheckNoPostClearSaveActions(result);
        CHECK(!result.clearTailStatusProducerCalled8001635C);
        CHECK(!result.clearTailNextStageUnlockCalled8001628C);
        CHECK(!result.clearTailSaveMenuCalled);
        CHECK(!result.blockedByUnknownWord800916F0);
        CHECK(result.sceneResultKnown && result.sceneResult == 2);
        CHECK(PrStage1LifecycleExecutorDirect::
                  IsStepSceneResultReleaseReady801C81EC(result));
        CHECK(CountActions(result, ActionKind801C81EC::SfxCue26EF8) == 1);
        CHECK(CountActions(result, ActionKind801C81EC::AudioFlush26ECC) == 1);
        CHECK(CountActions(result, ActionKind801C81EC::ResetHoldTiles1EF14) == 2);
    }
}

void TestSceneResultReleaseCoreBlocksAndDefersPrecisely() {
    const SceneEntry801C7284 sceneEntry{};

    Runtime801C81EC blockedRuntime = MakeClearTailRuntime();
    FrameInput801C81EC blockedInput = MakeClearTailInput();
    blockedInput.word800916F0Known = false;
    const StepResult801C81EC blocked =
        Step801C81EC(blockedRuntime, sceneEntry, blockedInput);
    CHECK(blocked.sceneResultKnown);
    CHECK(blocked.sceneResult == 2);
    CHECK(blocked.blockedByUnknownWord800916F0);

    bool deferredKnown = false;
    int32_t deferredResult = 31;
    const auto blockedImmediate = PrStage1LifecycleExecutorDirect::
        ResolveStepSceneResultReleaseCore801C81EC(
            deferredKnown,
            deferredResult,
            blocked,
            false,
            false);
    CHECK(!blockedImmediate.sceneResultKnown);
    CHECK(!blockedImmediate.deferred);
    CHECK(blockedImmediate.sceneResult == 0);
    CHECK(!deferredKnown);
    CHECK(deferredResult == 31);

    deferredKnown = true;
    deferredResult = 41;
    const auto blockedWaiting = PrStage1LifecycleExecutorDirect::
        ResolveStepSceneResultReleaseCore801C81EC(
            deferredKnown,
            deferredResult,
            blocked,
            true,
            false);
    CHECK(!blockedWaiting.sceneResultKnown);
    CHECK(!blockedWaiting.deferred);
    CHECK(deferredKnown);
    CHECK(deferredResult == 41);

    deferredKnown = false;
    deferredResult = 43;
    const auto blockedHostWork = PrStage1LifecycleExecutorDirect::
        ResolveStepSceneResultReleaseCore801C81EC(
            deferredKnown,
            deferredResult,
            blocked,
            false,
            true);
    CHECK(!blockedHostWork.sceneResultKnown);
    CHECK(!blockedHostWork.deferred);
    CHECK(!deferredKnown);
    CHECK(deferredResult == 43);

    Runtime801C81EC readyRuntime = MakeClearTailRuntime();
    FrameInput801C81EC readyInput = MakeClearTailInput();
    readyInput.word800916F0Known = true;
    readyInput.word800916F0 = 1;
    const StepResult801C81EC ready =
        Step801C81EC(readyRuntime, sceneEntry, readyInput);
    CHECK(PrStage1LifecycleExecutorDirect::
              IsStepSceneResultReleaseReady801C81EC(ready));

    deferredKnown = false;
    deferredResult = 53;
    const auto readyImmediate = PrStage1LifecycleExecutorDirect::
        ResolveStepSceneResultReleaseCore801C81EC(
            deferredKnown,
            deferredResult,
            ready,
            false,
            false);
    CHECK(readyImmediate.sceneResultKnown);
    CHECK(!readyImmediate.deferred);
    CHECK(readyImmediate.sceneResult == 2);
    CHECK(!deferredKnown);
    CHECK(deferredResult == 53);

    deferredKnown = false;
    deferredResult = 61;
    const auto readyWaiting = PrStage1LifecycleExecutorDirect::
        ResolveStepSceneResultReleaseCore801C81EC(
            deferredKnown,
            deferredResult,
            ready,
            true,
            false);
    CHECK(readyWaiting.sceneResultKnown);
    CHECK(readyWaiting.deferred);
    CHECK(readyWaiting.sceneResult == 2);
    CHECK(deferredKnown);
    CHECK(deferredResult == 2);

    deferredKnown = false;
    deferredResult = 71;
    const auto readyHostWork = PrStage1LifecycleExecutorDirect::
        ResolveStepSceneResultReleaseCore801C81EC(
            deferredKnown,
            deferredResult,
            ready,
            false,
            true);
    CHECK(readyHostWork.sceneResultKnown);
    CHECK(readyHostWork.deferred);
    CHECK(readyHostWork.sceneResult == 2);
    CHECK(deferredKnown);
    CHECK(deferredResult == 2);
}

void TestUnknownF0CommitWaitsWithoutRepeatingPreGateActions() {
    Runtime801C81EC runtime = MakeClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input = MakeClearTailInput();
    input.word800916F0Known = false;
    input.word800916F0 = 0;

    const StepResult801C81EC first =
        Step801C81EC(runtime, sceneEntry, input);
    CHECK(first.blockedByUnknownWord800916F0);
    CheckCommonClearTailStatusActions(first);
    MarkClearTailWord800916F0GatePreActionsApplied(runtime);
    CHECK(runtime.clearTailWaitingForWord800916F0);
    CHECK(runtime.clearTailPreWord800916F0ActionsApplied);

    const StepResult801C81EC wait =
        Step801C81EC(runtime, sceneEntry, input);
    CHECK(wait.blockedByUnknownWord800916F0);
    CHECK(wait.actions.empty());
    CHECK(!wait.sceneResultKnown);
    CHECK(runtime.clearTailStageStatusKnown);
    CHECK(runtime.clearTailWaitingForWord800916F0);
}

void TestLateKnownZeroResumesSaveGateWithoutRepeatingPreGateActions() {
    Runtime801C81EC runtime = MakeClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input = MakeClearTailInput();
    input.word800916F0Known = false;
    input.word800916F0 = 0;

    const StepResult801C81EC first =
        Step801C81EC(runtime, sceneEntry, input);
    CHECK(first.blockedByUnknownWord800916F0);
    MarkClearTailWord800916F0GatePreActionsApplied(runtime);

    input.word800916F0Known = true;
    const StepResult801C81EC resumed =
        Step801C81EC(runtime, sceneEntry, input);
    CHECK(!resumed.blockedByUnknownWord800916F0);
    CHECK(CountActions(resumed, ActionKind801C81EC::SaveStatus1635C) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::UnlockNextStage1628C) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::SfxCue26EF8) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::AudioFlush26ECC) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::Bootstrap15590) == 1);
    CHECK(CountActions(resumed, ActionKind801C81EC::SaveUi19148) == 1);
    CHECK(resumed.sceneResultKnown);
    CHECK(resumed.sceneResult == 2);
    CHECK(PrStage1LifecycleExecutorDirect::
              IsStepSceneResultReleaseReady801C81EC(resumed));
    CHECK(!runtime.clearTailStageStatusKnown);
    CHECK(!runtime.clearTailWaitingForWord800916F0);
    CHECK(!runtime.clearTailPreWord800916F0ActionsApplied);
}

void TestDeferredStatusReceiptPreservesF0Continuation() {
    using namespace PrStage1LifecycleExecutorDirect;
    Runtime801C81EC runtime = MakeClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input = MakeClearTailInput();
    input.word800916F0Known = false;
    const auto first = Step801C81EC(runtime, sceneEntry, input);
    CHECK(first.blockedByUnknownWord800916F0);
    CheckCommonClearTailStatusActions(first);
    const auto receipt = BuildStatusWrites801C81EC(first);
    CHECK(receipt.completeClearTailPreWord800916F0Actions);
    CHECK(HasStatusWrites801C81EC(receipt));
    CHECK(!runtime.clearTailPreWord800916F0ActionsApplied);
    CommitClearTailStatusContinuation801C81EC(runtime, {});
    CHECK(!runtime.clearTailPreWord800916F0ActionsApplied);
    // The adapter commits this same receipt only after queued native actions
    // finish. No external observation or F0 value is manufactured.
    CommitClearTailStatusContinuation801C81EC(runtime, receipt);
    const auto wait = Step801C81EC(runtime, sceneEntry, input);
    CHECK(wait.blockedByUnknownWord800916F0);
    CHECK(wait.actions.empty());
    CHECK(runtime.clearTailPreWord800916F0ActionsApplied);
    input.word800916F0Known = true;
    input.word800916F0 = 0;
    const auto resumed = Step801C81EC(runtime, sceneEntry, input);
    CHECK(CountActions(resumed, ActionKind801C81EC::Transition201AC) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::SaveStatus1635C) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::UnlockNextStage1628C) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::SaveUi19148) == 1);
}

void TestLateKnownOneSkipsSaveGateWithoutRepeatingPreGateActions() {
    Runtime801C81EC runtime = MakeClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input = MakeClearTailInput();
    input.word800916F0Known = false;
    input.word800916F0 = 0;

    const StepResult801C81EC first =
        Step801C81EC(runtime, sceneEntry, input);
    CHECK(first.blockedByUnknownWord800916F0);
    MarkClearTailWord800916F0GatePreActionsApplied(runtime);

    input.word800916F0Known = true;
    input.word800916F0 = 1;
    const StepResult801C81EC resumed =
        Step801C81EC(runtime, sceneEntry, input);
    CHECK(!resumed.blockedByUnknownWord800916F0);
    CHECK(CountActions(resumed, ActionKind801C81EC::SaveStatus1635C) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::UnlockNextStage1628C) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::SfxCue26EF8) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::AudioFlush26ECC) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::Bootstrap15590) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::SaveUi19148) == 0);
    CHECK(resumed.sceneResultKnown);
    CHECK(resumed.sceneResult == 2);
    CHECK(PrStage1LifecycleExecutorDirect::
              IsStepSceneResultReleaseReady801C81EC(resumed));
    CHECK(!runtime.clearTailStageStatusKnown);
    CHECK(!runtime.clearTailWaitingForWord800916F0);
    CHECK(!runtime.clearTailPreWord800916F0ActionsApplied);
}

void TestLateKnownZeroRequiresPreservedClearTailMovieResult() {
    Runtime801C81EC runtime = MakeClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input = MakeClearTailInput();
    input.word800916F0Known = false;
    input.word800916F0 = 0;

    const StepResult801C81EC first =
        Step801C81EC(runtime, sceneEntry, input);
    CHECK(first.blockedByUnknownWord800916F0);
    MarkClearTailWord800916F0GatePreActionsApplied(runtime);

    FrameInput801C81EC missingResult = input;
    missingResult.word800916F0Known = true;
    missingResult.clearTailPlayAndWaitResultKnown = false;
    const StepResult801C81EC missing =
        Step801C81EC(runtime, sceneEntry, missingResult);
    CHECK(!missing.blockedByUnknownWord800916F0);
    CHECK(missing.actions.empty());
    CHECK(!missing.sceneResultKnown);
    CHECK(runtime.clearTailStageStatusKnown);
    CHECK(runtime.clearTailWaitingForWord800916F0);
    CHECK(runtime.clearTailPreWord800916F0ActionsApplied);

    FrameInput801C81EC preserved = input;
    preserved.word800916F0Known = true;
    preserved.clearTailPlayAndWaitResultKnown = true;
    const StepResult801C81EC resumed =
        Step801C81EC(runtime, sceneEntry, preserved);
    CHECK(!resumed.blockedByUnknownWord800916F0);
    CHECK(CountActions(resumed, ActionKind801C81EC::SaveStatus1635C) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::UnlockNextStage1628C) == 0);
    CHECK(CountActions(resumed, ActionKind801C81EC::Bootstrap15590) == 1);
    CHECK(CountActions(resumed, ActionKind801C81EC::SaveUi19148) == 1);
    CHECK(resumed.sceneResultKnown);
    CHECK(resumed.sceneResult == 2);
}

void TestKnownZeroAllowsClearTailBootstrapAndSaveUi() {
    Runtime801C81EC runtime = MakeClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input = MakeClearTailInput();
    input.word800916F0Known = true;
    input.word800916F0 = 0;

    const StepResult801C81EC result =
        Step801C81EC(runtime, sceneEntry, input);

    CheckCommonClearTailStatusActions(result);
    CHECK(CountActions(result, ActionKind801C81EC::Bootstrap15590) == 1);
    CHECK(CountActions(result, ActionKind801C81EC::SaveUi19148) == 1);
    CHECK(!result.blockedByUnknownWord800916F0);
    CHECK(result.sceneResultKnown);
    CHECK(result.sceneResult == 2);
    CHECK(runtime.phase == Phase801C81EC::ClearTailMovieRequested);
    CHECK(!runtime.clearTailStageStatusKnown);
}

void TestKnownOneSkipsClearTailBootstrapAndSaveUiButReturns() {
    Runtime801C81EC runtime = MakeClearTailRuntime();
    const SceneEntry801C7284 sceneEntry{};
    FrameInput801C81EC input = MakeClearTailInput();
    input.word800916F0Known = true;
    input.word800916F0 = 1;

    const StepResult801C81EC result =
        Step801C81EC(runtime, sceneEntry, input);

    CheckCommonClearTailStatusActions(result);
    CheckNoPostClearSaveActions(result);
    CHECK(!result.blockedByUnknownWord800916F0);
    CHECK(result.sceneResultKnown);
    CHECK(result.sceneResult == 2);
    CHECK(runtime.phase == Phase801C81EC::ClearTailMovieRequested);
    CHECK(!runtime.clearTailStageStatusKnown);
}

} // namespace

int main() {
    TestStageRunnerPendingDoesNotReenterStageRecordInit();
    TestDeferredStatusReceiptPreservesF0Continuation();
    TestUnknownF0BlocksClearTailBootstrapAndSaveUi();
    TestPostMovieSceneResultLimitsPreserveTheExactF0Gate();
    TestEasyStage1ClearSkipsNormalRecordsForEveryF0Observation();
    TestSceneResultReleaseCoreBlocksAndDefersPrecisely();
    TestUnknownF0CommitWaitsWithoutRepeatingPreGateActions();
    TestLateKnownZeroResumesSaveGateWithoutRepeatingPreGateActions();
    TestLateKnownOneSkipsSaveGateWithoutRepeatingPreGateActions();
    TestLateKnownZeroRequiresPreservedClearTailMovieResult();
    TestKnownZeroAllowsClearTailBootstrapAndSaveUi();
    TestKnownOneSkipsClearTailBootstrapAndSaveUiButReturns();

    if (g_failed != 0) {
        std::printf("test_ss0_stage1_clear_tail_f0_gate: failed checks=%d\n",
                    g_failed);
        return 1;
    }
    std::printf("test_ss0_stage1_clear_tail_f0_gate: ok\n");
    return 0;
}
