# Stage1 fail terminal runner-stop Pro follow-up 2 2026-05-25

## Purpose

This follow-up starts after pushed commit `665b4fa4`.
The Pro-proposed cleanup-drain patch was applied, full-built locally, pushed,
and user-tested.

User visual result after `665b4fa4`:

- fixed: black flash before Try Again is still gone;
- still broken: fail branch remains abnormal;
- still broken: failure animation / gameplay continue running;
- still broken: compact rail still continues running during the fail path.

Therefore the previous runner-stop patch did not hit the actual visible
gameplay/rail continuation. Do not keep extending the same patch blindly. Audit
whether the active wrong path is a different lifecycle/render carrier than the
one `665b4fa4` drained.

## Branch

- Repository: `https://github.com/2320614999/PaRappaWin`
- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Current pushed commit: `665b4fa4`
- Previous relevant commits:
  - `1da66d74`: wired `8001A7F8/8001A3B8` return facts; user confirmed black
    flash disappeared, but gameplay/rail still continued.
  - `665b4fa4`: drained the formal lifecycle cleanup tail without re-entering
    normal `runner.Update(ctx)`; user confirmed gameplay/rail still continued.

## Constraints

Runtime authority remains IDA / pseudo-C. Do not output a renderer/UI/shell
symptom patch. Do not add observer/harness/sentinel code. Do not touch
MoveImage, Try Again underlay, compact rail rendering, or generic
runner/XA/audio freeze unless there is a direct PSX pseudo-C boundary proving
the change.

## Current applied shape to re-audit

`src/pr/pr_scenes.cpp` now has a cleanup-drain state:

```cpp
static bool s_stage1TerminalCleanupDrainActive801C7A60 = false;
static uint8_t s_stage1TerminalCleanupDrainFramesRemaining801C7A60 = 0u;
static int32_t s_stage1TerminalCleanupDrainResult801C7A60 = 0;
```

`RunStage1Scene1RunnerFrameDriverAdapter` now tries to service the drain before
calling normal gameplay update:

```cpp
PrStage1FormalLifecycleSnapshot lifecycleSnapshot{};
if (CopyStage1FormalLifecycleSnapshot(lifecycleSnapshot) &&
    TryServiceStage1TerminalCleanupDrain801C7A60(lifecycleSnapshot, out)) {
    return out;
}

(void)runner.Update(ctx);
```

It also tries again after `PrimeStage1FormalLifecycleRuntime(...)`:

```cpp
if (CopyStage1FormalLifecycleSnapshot(lifecycleSnapshot) &&
    TryServiceStage1TerminalCleanupDrain801C7A60(lifecycleSnapshot, out)) {
    return out;
}
if (lifecycleSnapshot.valid &&
    (lifecycleSnapshot.clearGate || lifecycleSnapshot.failGate)) {
    out.known = false;
    out.result = 0;
    return out;
}
```

The service function returns `known=false` while consuming the four-frame
cleanup drain, and returns the final PSX result only after the local drain
counter reaches zero.

## Newly suspicious carrier after user test

The failed visual result suggests the normal `runner.Update(ctx)` call was not
the only source of visible gameplay/rail continuation.

One possible mismatched carrier is `ctx.stageRunning` and the Scene1 render
route. `RunStage1Scene1RunnerFrameDriverAdapter` still sets:

```cpp
ctx.stageRunning = true;
```

The final `ctx.stageRunning = false` path only happens when
`StageRunnerRun7A60` returns a known nonzero result to the lifecycle host. During
blocked cleanup drain frames, the host action is still retried and
`ctx.stageRunning` can remain true.

Scene1 render routing then uses only that carrier:

```cpp
carrier.stageRunning = ctx.stageRunning;
routeInput.stageRunning = carrier.stageRunning;
...
case Scene1RenderRoute::GameplaySubmitDraw:
    PrStage1Scene1DrawBackend::DrawGameplaySubmitAndHud(ctx);
    return;
```

`DrawGameplaySubmitAndHud(ctx)` advances/draws the Stage1 submit runtime and HUD:

```cpp
SubmitAndDrawStage1Scene801CBFDC190(ctx, false);
PrStage1LiveHud::DrawStage1RuntimeHud(ctx);
```

This may explain why the user still sees gameplay/rail running even after
`runner.Update(ctx)` is skipped during cleanup drain. However, do not patch this
as a visual route hide unless the PSX lifecycle/render boundary proves it.

## What Pro should answer

Please re-audit the fail terminal runner-stop boundary at current HEAD
`665b4fa4`, not `1da66d74`.

Questions:

1. Did `665b4fa4` fix the wrong seam? Specifically, is the continuing
   gameplay/rail driven by Scene1 render/submit runtime while
   `StageRunnerRun7A60` is blocked, rather than by `runner.Update(ctx)` itself?
2. In PSX `801C7A60 -> LABEL_68`, while the four cleanup
   `RenderFrame(7) -> 80035560(2) -> PresentFrame` passes run, what should the
   Win carrier equivalent of `ctx.stageRunning` / Scene1 render route be?
3. Is it a proven mismatch that Win keeps `ctx.stageRunning == true` and routes
   to `GameplaySubmitDraw` while the direct cleanup tail is already active?
4. If safe, output one `git apply --index` compatible unified diff that fixes
   only this boundary and updates docs.
5. If not safe, output `no safe patch` with the exact missing PSX function,
   field, or Win carrier.

Do not:

- revert `1da66d74` or `665b4fa4` without a direct reason;
- add a generic runner/XA/audio freeze;
- hide compact rail directly;
- touch `DrawGameplaySubmitFrozenRuntimeBaseOnly(ctx)`;
- touch MoveImage / Try Again underlay;
- touch renderer/UI shell layers;
- add observer/harness/sentinel code.

The target remains: after terminal fail, normal gameplay and compact rail must
stop advancing because the Win lifecycle/render carrier matches PSX
`801C7A60`, not because a visual component was hidden.
