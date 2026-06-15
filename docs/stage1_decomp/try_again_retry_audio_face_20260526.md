# Stage1 Try Again retry audio / face handoff - 2026-05-26

## Scope

- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Runtime baseline: `79870276`
- Purpose: prepare the next ChatGPT web Pro audit with local-only IDA evidence
  and current Win split points. No runtime code was changed for this note.

## Latest user-visible regressions

1. After Try Again YES retry, the character expression in the transition frozen
   frame keeps the failed-frame expression instead of resetting.
2. After retry, Stage1 CD music is missing; rating switches do not restore it.
3. On first play, opening CD music is missing; only a later rating switch brings
   background music back.

Previous fixes accepted by the user before these symptoms:

- Try Again retry/transition restart no longer jumps back to the old fail point.
- Try Again no longer repeatedly interrupts the restarted game.
- Try Again black flash was accepted as fixed before the new audio/face report.
- Event4 Try Again SFX were restored in `79870276`.

## Current Win split points to audit

`src/pr/pr_scenes.cpp` currently suppresses the StageRecord/XA adapter while the
intro transition is active:

```cpp
static bool AdvanceStage1Scene1XaFrameDriverAdapter(
    PrGameContext& ctx,
    const std::filesystem::path& stageRuntimePath,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window) {
    if (window.introTransitionActive) {
        return false;
    }

    if (s_stage1RetryStageRecordRestartPending801C81EC) {
        ResetStage1RetryStageRecordRestart801C81EC();
        if (s_xaStarted || ctx.stage1XaCdDirect.streamStarted) {
            if (ctx.xa1Player) {
                ctx.xa1Player->Stop();
            }
            PrStage1XaCdDirectReset(ctx.stage1XaCdDirect);
            s_xaStarted = false;
        }
    }

    if (!s_xaStarted && ctx.xa1Player && !stageRuntimePath.empty()) {
        const bool ok = ctx.xa1Player->Play(stageRuntimePath);
        ...
        if (ok) {
            ...
            ctx.xa1Player->Update();
            PumpStage1XaCdDirectRingPackets(ctx);
            ...
            (void)PrStage1XaCdDirectApplySub8001A4D0StageRecordTick(...);
            GetStageRunner().SyncXaStartBaseline(ctx);
            return tickResult.resultKnown && tickResult.psxReturn == 0;
        }
    }

    if (ctx.xa1Player && ctx.xa1Player->IsPlaying()) {
        ctx.xa1Player->Update();
        PumpStage1XaCdDirectRingPackets(ctx);
        ...
        return tickResult.resultKnown && tickResult.psxReturn == 0;
    }
    ...
}
```

The runner adapter also suppresses runner update during the same intro
transition window, then resyncs the XA baseline on the first post-transition
frame:

```cpp
static LifecycleHost801C81EC::StageRunnerHostResult801C81EC
RunStage1Scene1RunnerFrameDriverAdapter(
    PrGameContext& ctx,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window) {
    LifecycleHost801C81EC::StageRunnerHostResult801C81EC out{};
    if (window.introTransitionActive) {
        return out;
    }

    PrStageRunner& runner = GetStageRunner();
    ctx.stageRunning = true;
    if (runner.GetState() == StageRunnerState::Idle ||
        runner.GetSceneId() != 1) {
        ResetStage1FormalLifecycleRuntime();
        ResetStage1TerminalFormalLifecycleSnapshot();
        ResetStage1TerminalCleanupDrain801C7A60();
        PrScn1::ApplyStage1RunnerDirectInitLifecycle(ctx);
        runner.Init(1, ctx);
        MarkStage1IntroTransitionPrerollPending801C7A60();
    }
    if (ArmStage1IntroTransitionPreroll801C7A60()) {
        return out;
    }

    if (window.resetIntroConsumer) {
        runner.SyncXaStartBaseline(ctx);
    }
    ...
    (void)runner.Update(ctx);
}
```

The intro transition arming path resets only the draw backend runtime:

```cpp
static bool ArmStage1IntroTransitionPreroll801C7A60() {
    if (s_stage1IntroTransitionFrame >= 0 || s_stage1IntroTransitionActive) {
        return false;
    }

    s_stage1IntroTransitionActive = true;
    s_stage1IntroTransitionRenderActive = true;
    s_stage1IntroTransitionConsumerResetPending = false;
    PrStage1Scene1DrawBackend::ResetGameplaySubmitRuntime();
    return true;
}
```

## PSX authority already in repo

`docs/stage1_decomp/801c81ec_lifecycle_callsite_table_20260514.md` records the
Stage1 loop authority:

```text
Loop: 8001A4D0(sceneEntry+0x9C,0),
      801C7A60(sceneEntry+0x9C,0x801C3640,sceneId)
```

`docs/stage1_decomp/801C79E4_sub_801C79E4.c` shows the runner-entry reset
sequence:

```c
int __fastcall sub_801C79E4(int a1, int a2)
{
  dword_801D303C = 0;
  MEMORY[0x80026FA4]();
  PrStageRunner_InitUI();
  MEMORY[0x8001A478](*(__int16 *)(a1 + 6));
  MEMORY[0x8001A654](*(__int16 *)(a1 + 4));
  MEMORY[0x80014344]();
  MEMORY[0x80024E98]();
  MEMORY[0x80024FC0](a2);
  return PrStageRunner_ResetState();
}
```

`docs/stage1_decomp/801C7A60_sub_801C7A60.c` shows the steady/fail loop calls:

```c
if ( (*a2 & 0x200) != 0 )
{
  MEMORY[0x8001A654](*(_DWORD *)(12 * *((__int16 *)a2 + 39) - 2147134436));
}
else if ( (*a2 & 0x40) != 0 )
{
  MEMORY[0x80026EF8](MEMORY[0x800943EC]);
  MEMORY[0x8001A4A4](1);
}
else if ( (*a2 & 0x100) != 0 )
{
  v29 = 0;
}
...
MEMORY[0x80015350](a2, 2 * *(_DWORD *)(MEMORY[0x8006EDB8] + 352));
MEMORY[0x80035560](2);
PrStageRunner_PresentFrame(a2);
...
MEMORY[0x8001B120](1);
MEMORY[0x80026FA4]();
MEMORY[0x800357D4](0);
MEMORY[0x8001A694]();
```

## Local IDA recheck for this handoff

`8001A4D0`:

```c
int __fastcall sub_8001A4D0(int a1, int a2)
{
  int result; // $v0
  __int16 v5; // $v0
  int v6; // $s0
  int v7; // $v0

  result = *(_DWORD *)a1;
  if ( *(_DWORD *)a1 )
  {
    dword_800493EC = *(_DWORD *)(a1 + 16);
    dword_800493FC = sub_80036A78((unsigned __int8 *)&dword_800493EC);
    sub_8001A478(*(__int16 *)(a1 + 6));
    do
    {
      do
      {
        while ( !sub_800367A4(2, &dword_800493EC, dword_80049414) )
          ;
      }
      while ( sub_800364D0(0, 0) != 2 );
      byte_8004940C = 1;
      v5 = *(_WORD *)(a1 + 4);
      word_8004940E = 0;
      byte_8004940D = v5;
      do
      {
        while ( !sub_800367A4(13, &byte_8004940C, dword_80049414) )
          ;
      }
      while ( sub_800364D0(0, 0) != 2 );
      v6 = 72;
      if ( a2 == 1 )
      {
        v6 = 456;
        v7 = 16;
      }
      else
      {
        v7 = 4;
      }
      dword_80049424 = v7;
      while ( !sub_800391AC(v6) )
        ;
      sub_80035560(3);
    }
    while ( sub_800364D0(0, 0) != 2 );
    dword_80049410 = 1;
    dword_80049420 = 0;
    return sub_80036678(1, 0);
  }
  return result;
}
```

`8001A478`:

```c
int __fastcall sub_8001A478(__int16 a1)
{
  return sub_8002AB24(0, a1, a1);
}
```

`8001A654`:

```c
int __fastcall sub_8001A654(char a1)
{
  byte_8004940D = a1;
  return sub_80036678(13, &byte_8004940C);
}
```

`8001A4A4`:

```c
int __fastcall sub_8001A4A4(int a1)
{
  return sub_8001A478(a1 != 1 ? 0x7F : 0);
}
```

`80026EF8`:

```c
int __fastcall sub_80026EF8(unsigned __int8 *a1)
{
  int v1; // $a1
  int v2; // $a2
  unsigned __int8 v3; // $a3
  int result; // $v0

  v1 = *a1;
  v2 = a1[1];
  v3 = a1[1] + 24;
  a1[2] = v3;
  result = sub_80034240(word_800943A8, v1, v2, v3, 0, a1[3], a1[3]);
  word_800943AC = result;
  return result;
}
```

`80026FA4`:

```c
int sub_80026FA4()
{
  return sub_800351B8(0);
}
```

`80015350`:

```c
int __fastcall sub_80015350(int a1, int a2)
{
  int result; // $v0

  result = word_80091824;
  if ( word_80091824 > 0 )
  {
    result = word_80091824 - a2;
    word_80091824 -= a2;
  }
  return result;
}
```

`80014344` reset:

```c
int sub_80014344()
{
  int v0; // $gp
  int v1; // $s1
  _BYTE *v2; // $s0
  int result; // $v0
  int v4; // $gp

  v1 = 0;
  v2 = &unk_80092910;
  *(_DWORD *)(v0 + 28) = &unk_80092910;
  do
  {
    sub_80025C44(v2, 384);
    ++v1;
    v2 += 384;
  }
  while ( v1 < 4 );
  sub_80025C44(&dword_80091800, 44);
  word_8009181A = 0;
  word_8009181C = 0;
  word_80091816 = 0;
  word_80091818 = 0;
  sub_80014C1C();
  result = sub_8001448C();
  *(_DWORD *)(v4 + 32) = 0;
  dword_80091800 = 0;
  return result;
}
```

`80024E98` event/runtime reset:

```c
int sub_80024E98()
{
  int v0; // $gp

  sub_80025C44(&dword_8008ECE0, 244);
  dword_8008ED20 = 0;
  dword_8008ED00 = 0;
  g_PrStageEventStreamFlag = 1;
  g_PrStageEventStreamId = 0;
  *(_DWORD *)(v0 + 800) = 0;
  sub_80025C44(&unk_801C3640, 488);
  word_801C368E = 1;
  word_801C3690 = 0;
  word_801C3692 = 0;
  word_801C36AA = 1;
  word_801C36BA = 1;
  word_801C36CA = 0;
  word_801C36D0 = 0;
  word_801C36E2 = 0;
  dword_801C36D4 = dword_800943D0 + 4;
  dword_801C36D8 = dword_800943D0 + 24;
  word_801C36A8 = word_800916DC;
  word_801C3694 = word_800916DC;
  return sub_80024F8C(&unk_801C3640);
}
```

## Candidate questions for web Pro

Please audit only the following narrow questions:

1. For missing CD music on first play and after retry: is the Win split bug that
   `AdvanceStage1Scene1XaFrameDriverAdapter` returns immediately while
   `window.introTransitionActive`, so a started `ctx.xa1Player` is not updated
   and `PumpStage1XaCdDirectRingPackets(ctx)` is not run during the 76-frame
   transition/preroll window? If yes, output the smallest direct-port diff that
   keeps host XA pumping during transition without advancing StageRunner/gameplay
   authority or writing a generic clock/audio freeze patch.
2. For retry transition frozen-frame expression: does PSX `801C79E4` /
   `80014344` / `80024E98` imply a reset that Win currently applies to numeric
   runtime and event slots but not to the renderer/TIM face payload carrier
   before the transition freeze is rendered? If yes, output the smallest
   direct-port diff that resets/reseeds only the relevant direct face/TIM
   carrier at the same entry boundary, without renderer/UI symptom fallback.
3. If either question cannot be proven from the pasted and pushed evidence,
   output `no safe patch` for that question and list the exact missing IDA
   function or Win carrier evidence.

Hard constraints:

- IDA Hex-Rays pseudo-C / disasm is runtime authority.
- Do not use renderer/UI/shell symptom patches.
- Do not write clock/audio freeze guesses.
- Do not touch sentinel/matcher/observer/harness.
- If a safe patch is justified, output a `git apply --index` compatible unified
  diff only for runtime/doc files needed by the fix.
- If not justified, output `no safe patch` and the missing evidence.

## Web Pro environment limits to remember

- You cannot access local files, unpushed changes, IDA session, emulator,
  Visual Studio, or Codex terminal output.
- You cannot modify code, commit, push, run build/test, clone repos, or use
  Python to access the network.
- Work only from pasted content and the pushed branch/commit.
