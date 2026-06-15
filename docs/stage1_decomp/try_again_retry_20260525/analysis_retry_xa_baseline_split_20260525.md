# Stage1 Try Again retry XA baseline split audit - 2026-05-25

## Scope

- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Runtime baseline: `70c1bf89`
- Symptom under review: after Try Again YES, Stage1 retry appears not to reset
  fully and can jump forward toward the old fail point.
- Status: candidate mismatch for web Pro review; no runtime code changed in this
  note.

## Observed Win split timing

The current lifecycle action order emits StageRecord before the StageRunner:

```text
StageRecordTick1A4D0 -> StageRunnerRun7A60
```

On a retry, `AdvanceStage1Scene1XaFrameDriverAdapter` consumes
`s_stage1RetryStageRecordRestartPending801C81EC`, stops/resets the old XA/CD
carrier, starts the new Stage1 XA stream, pumps `ctx.xa1Player->Update()`, and
then calls `GetStageRunner().SyncXaStartBaseline(ctx)`.

At that point, the runner can still be idle because `StageRecordTick1A4D0`
executes before `StageRunnerRun7A60`. The baseline sync therefore has no effect
when the runner has not been initialized yet.

The same frame then enters `RunStage1Scene1RunnerFrameDriverAdapter`. A fresh
runner entry performs the direct lifecycle resets, calls `runner.Init(1, ctx)`,
and arms the intro transition preroll:

```cpp
if (runner.GetState() == PrStageRunnerState::Idle || runner.GetSceneId() != 1) {
    ResetStage1FormalLifecycleRuntime();
    ResetStage1TerminalFormalLifecycleSnapshot();
    ResetStage1TerminalCleanupDrain801C7A60();
    ApplyStage1RunnerDirectInitLifecycle(ctx);
    runner.Init(1, ctx);
    MarkStage1IntroTransitionPrerollPending801C7A60();
}
if (ArmStage1IntroTransitionPreroll801C7A60()) {
    return out;
}
```

`PrStageRunner::Init` samples the current XA baseline if the XA stream is already
playing:

```cpp
if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
    m_startAudioSeconds = ctx.xa1Player->GetPlayedSeconds();
    m_startAudioSector75 = ctx.xa1Player->GetCdClockSectorIndex();
}
```

The intro transition preroll then blocks gameplay runner updates for
`kStage1IntroTransitionMaxLife` frames. During that window, the runner does not
update, but the host XA stream was already started by the StageRecord path and
can continue advancing.

When gameplay resumes, the first `runner.Update(ctx)` computes the hybrid
timecode from the XA sector difference against the baseline captured before the
transition preroll:

```cpp
relativeSector75 = ctx.xa1Player->GetCdClockSectorIndex() - m_startAudioSector75;
if (relativeSector75 > m_lastAcceptedXaSector75) {
    m_tick96 = m_tickOffset + floor(relativeSector75 * tickPerMinute / 4500 + 0.5);
}
```

This can make the retry begin with a positive timecode jump instead of the
expected first gameplay frame after the transition.

## Candidate minimal direct-host correction

There is an existing one-frame carrier when the intro transition consumer is
reset:

```cpp
frameWindow.resetIntroConsumer = !active && intro.consumerResetPending;
intro.consumerResetPending = false;
```

Candidate for review only:

```cpp
if (window.resetIntroConsumer) {
    runner.SyncXaStartBaseline(ctx);
}
```

Placement under review: in `RunStage1Scene1RunnerFrameDriverAdapter`, after
`ArmStage1IntroTransitionPreroll801C7A60()` has returned false and before
`TryServiceStage1TerminalCleanupDrain801C7A60()` / `runner.Update(ctx)`.

The helper is already narrow:

```cpp
if (m_state != PrStageRunnerState::Playing || !ctx.xa1Player ||
    !ctx.xa1Player->IsPlaying()) {
    return;
}
if (m_frameCount != 0 || m_elapsedSeconds > 0.0) {
    return;
}
m_startAudioSeconds = ctx.xa1Player->GetPlayedSeconds();
m_startAudioSector75 = ctx.xa1Player->GetCdClockSectorIndex();
m_lastAcceptedXaSector75 = 0;
```

## Audit question

This candidate does not freeze audio or runner state and does not draw from the
renderer/UI layer. It compensates for a Win host split where StageRecord starts
XA before the runner is initialized and the intro transition then delays the
first gameplay update.

The web Pro audit should either:

- accept the candidate and return a `git apply --index` compatible diff, or
- reject it as insufficiently proven and list the missing IDA/function evidence.
