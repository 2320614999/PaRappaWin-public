# Stage1 fail terminal runner-stop Pro follow-up 2026-05-25

## Purpose

This follow-up starts after pushed commit `1da66d74`.
The previous gate-fact patch was built and tested locally. User visual result:

- fixed: black flash just before Try Again is gone;
- still broken: fail branch remains abnormal;
- still broken: failure animation / gameplay continue running;
- still broken: compact rail continues running during the fail/Try Again path.

Do not revert the `8001A7F8/8001A3B8` gate-fact wiring just because the
larger fail branch is still wrong. It appears to have fixed the black flash but
did not close the gameplay/rail stop boundary.

## Branch

- Repository: `https://github.com/2320614999/PaRappaWin`
- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Latest pushed commit before this follow-up: `1da66d74`
- Audit this file first.

## PSX authority

`docs/stage1_decomp/801C7A60_sub_801C7A60.c` shows that once the main loop
falls through to `LABEL_68`, PSX no longer continues the normal gameplay loop.
It only performs the fixed cleanup tail:

```c
LABEL_68:
  *((_WORD *)a2 + 50) = 0;
  *((_WORD *)a2 + 42) = 0;
  *((_WORD *)a2 + 61) = 0;
  v33 = a2;
  do
  {
    PrStageRunner_RenderFrame(v33, 7);
    MEMORY[0x80035560](2);
    PrStageRunner_PresentFrame(a2);
    --v11;
    v33 = a2;
  }
  while ( v11 > 0 );
  MEMORY[0x8001B120](1);
  MEMORY[0x80026FA4]();
  MEMORY[0x800357D4](0);
  MEMORY[0x8001A694]();
  result = 2;
  if ( *((_WORD *)a2 + 59) != 1 )
  {
    result = 3;
    if ( *((_WORD *)a2 + 60) == 1 )
    {
      v35 = MEMORY[0x800166AC](a3);
      if ( MEMORY[0x800916DA] == 1 )
        return 1;
      result = 2;
      if ( v35 < 4 )
        return 1;
    }
  }
  return result;
```

After `801C7A60` returns nonzero, `801C81EC` consumes the stage result and then
performs the event4 Try Again poll outside the gameplay loop.

## Current Win behavior to audit

### 1. StageRunnerRun7A60 retry still calls runner.Update

Current adapter:

- `src/pr/pr_scenes.cpp`
- function `RunStage1Scene1RunnerFrameDriverAdapter`

Relevant shape:

```cpp
(void)runner.Update(ctx);
out.known = true;
out.result = 0;
...
const bool directFinalReturnKnown =
    lifecycleSnapshotKnown &&
    lifecycleSnapshot.runnerTailFinalReturnKnown &&
    lifecycleSnapshot.runnerTailCleanupRenderPassBudget != 0u;
if (directFinalReturnKnown) {
    out.result = lifecycleSnapshot.runnerTailFinalReturn;
    return out;
}
if (lifecycleSnapshotKnown &&
    (lifecycleSnapshot.clearGate || lifecycleSnapshot.failGate)) {
    out.known = false;
    out.result = 0;
    return out;
}
return out;
```

Current host action handling:

- `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp`
- `ActionKind::StageRunnerRun7A60`

Relevant shape:

```cpp
const StageRunnerHostResult801C81EC runnerResult =
    host.runStageRunner != nullptr ? host.runStageRunner(ctx, window)
                                   : StageRunnerHostResult801C81EC{};
if (runnerResult.known) {
    if (runnerResult.result != 0) {
        ctx.stageRunning = false;
        PrStage1RuntimeSlotsDirectApplyStageLoopExit801C7A60(
            host.runtimeSlots);
        GetStageRunner().Reset();
    }
    PrStage1LifecycleExecutorDirect::SetStageResult801C7A60(
        host.executor,
        runnerResult.result);
    out =
        PrStage1LifecycleExecutorDirect::MakeImmediateInputResult801C81EC();
} else {
    out = PrStage1LifecycleExecutorDirect::
        MakeBlockedActionRetryResult801C81EC();
}
```

`MakeBlockedActionRetryResult801C81EC()` sets both `waitingForHostBlock` and
`retryBlockedAction`, so the same `StageRunnerRun7A60` action can be retried.
Because the adapter calls `runner.Update(ctx)` before checking fail/clear gate,
this may keep advancing gameplay and compact rail after PSX would already be in
the cleanup/fail terminal tail.

### 2. Tail cleanup trace is generated but may not be executed as the blocking tail

Current direct tail resolver:

- `src/pr/pr_stage_runner_direct.cpp`
- `PrStageRunnerDirectResolveMainLoopTail7A60`

It emits the PSX cleanup tail when `cleanupRequired` is true:

```cpp
out.cleanupDrainFrameCount = 4u;
for (uint8_t i = 0u; i < out.cleanupDrainFrameCount; ++i) {
    Call801C75D0RenderFrame(arg=7);
    Call80035560(arg=2);
    Call801C76CCPresentFrame();
}
out.finalCall1B120 = true;
out.finalCall26FA4 = true;
out.finalCall357D4Zero = true;
out.finalCall1A694 = true;
```

Current bridge into formal lifecycle:

- `src/pr/pr_scenes.cpp`
- `BuildStage1FormalLifecycleFrameInputs`

It passes:

```cpp
inputs.runnerTailDirectKnown = tail.known;
inputs.runnerTailCleanupRequiredKnown = tail.cleanupRequiredKnown;
inputs.runnerTailCleanupRequired = tail.cleanupRequired;
inputs.runnerTailCleanupDrainFrameCount = tail.cleanupDrainFrameCount;
```

Current formal observer:

- `src/pr/pr_stage1_scorer_host.cpp`
- `RefreshStage1FormalLifecycleExitResultObserverRuntimeImpl`

It sets:

```cpp
runtime.runnerTailCleanupRenderPassBudget =
    inputs.runnerTailDirectKnown &&
            inputs.runnerTailCleanupRequiredKnown &&
            inputs.runnerTailCleanupRequired
        ? inputs.runnerTailCleanupDrainFrameCount
        : 0u;
```

But the visible result says the fail/Try Again path still allows gameplay and
rail to continue. Audit whether the cleanup tail is only being observed as a
snapshot instead of being executed as the blocking terminal pass that replaces
normal `runner.Update`.

## What Pro should answer

Audit only the fail terminal runner-stop boundary:

1. Is it a proven mismatch that `RunStage1Scene1RunnerFrameDriverAdapter`
   retries `StageRunnerRun7A60` by calling `runner.Update(ctx)` again after
   `lifecycleSnapshot.failGate` or `clearGate` is already visible?
2. Is it a proven mismatch that the four-frame `LABEL_68` cleanup tail is
   represented only as a snapshot/budget instead of a blocking action that
   replaces normal gameplay update?
3. If safe, output one `git apply --index` compatible unified diff that fixes
   only this boundary and updates docs.
4. If not safe, output `no safe patch` with the exact missing PSX field or Win
   carrier.

Do not:

- revert `1da66d74`;
- add generic runner/XA/audio freeze;
- use renderer/UI/shell symptom patches;
- touch MoveImage / Try Again underlay;
- touch compact rail rendering directly;
- add observer/harness/sentinel changes.

The user-visible target is not to hide rail. The target is to stop the normal
gameplay loop at the PSX `801C7A60` fail terminal boundary so rail, gameplay
animation, and fail/Try Again sequencing stop advancing for the right reason.
