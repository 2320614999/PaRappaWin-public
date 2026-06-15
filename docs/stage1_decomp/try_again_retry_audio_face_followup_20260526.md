# Stage1 retry audio / face follow-up - 2026-05-26

## Current baseline

- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Runtime code baseline: `fc1c1db7`
- `fc1c1db7` is a revert of `9a9a63ec fix: pump Stage1 XA during intro transition`.
- This note is committed on top of that runtime baseline for the next Pro audit.
- Full build after the revert succeeded locally and produced
  `E:\game\PSgame\parappa the rapper\PaRappaWin.exe`.
- No runtime code is changed by this note.

## Negative evidence from the reverted patch

`9a9a63ec` came from the previous Pro handoff and tried to service Stage1 XA
during the intro transition / frozen-frame window. User testing rejected it:

```text
This patch must not stay. None of the reported issues were fixed, and it made
the frozen frame stay visually static while the game process had already started
running early.
```

Treat this as a hard boundary:

- Do not re-propose pumping XA/player/ring packets while
  `window.introTransitionActive` is true.
- Do not advance `StageRunner`, gameplay timecode, direct XA clock, or any
  gameplay-owned time source behind the transition frozen frame.
- The missing CD music problem is not solved by letting the transition window
  run the stage clock early. Look for missing start/filter/volume/callback state
  or reset/flush ordering instead.

## Current user-visible issues

1. After Try Again YES retry, the transition frozen-frame character expression
   keeps the failed-frame expression instead of resetting.
2. After retry, Stage1 CD music is missing; rating switches do not restore it.
3. On first play, opening Stage1 CD music is missing; only a later rating switch
   brings background music back.

Accepted state before this follow-up:

- Try Again YES retry no longer jumps back to the old fail point.
- Try Again no longer repeatedly interrupts the restarted game.
- Try Again black flash was fixed, then stayed fixed after the bad XA-pump
  patch was reverted.
- Event4 Try Again SFX were restored in `79870276`.

## Current Win split points

`src/pr/pr_scenes.cpp` currently blocks the StageRecord/XA adapter during the
intro transition:

```cpp
static bool AdvanceStage1Scene1XaFrameDriverAdapter(
    PrGameContext& ctx,
    const std::filesystem::path& stageRuntimePath,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window) {
    if (window.introTransitionActive) {
        return false;
    }
    ...
}
```

The StageRunner adapter also blocks runner update during the same window:

```cpp
static LifecycleHost801C81EC::StageRunnerHostResult801C81EC
RunStage1Scene1RunnerFrameDriverAdapter(
    PrGameContext& ctx,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window) {
    LifecycleHost801C81EC::StageRunnerHostResult801C81EC out{};
    if (window.introTransitionActive) {
        return out;
    }
    ...
    if (window.resetIntroConsumer) {
        runner.SyncXaStartBaseline(ctx);
    }
    ...
}
```

The bad reverted patch added an intro-transition XA pump in these blocked paths.
That direction is now disallowed unless new IDA evidence proves a narrower
non-clock side effect.

## Face / expression carrier evidence

Existing repo exports show the Stage1 face/HUD callback chain.

`docs/stage1_decomp/80014614_sub_80014614.c`:

```c
sub_80026FC4(v20);
if ( *(_DWORD *)(v20 + 8) )
  dword_80094430(a1);
result = -9;
```

`docs/stage1_decomp/stage1_action_submit_20260524/decompile_801ca0e0.txt`:

```c
*(_DWORD *)a1 |= 0x10u;
*(_DWORD *)(a1 + 220) = *(_DWORD *)(4 * *((__int16 *)&unk_801D2DD0 + 5 * a2) - 2146887592);
*(_DWORD *)(a1 + 232) = *(_DWORD *)(4 * *((__int16 *)&unk_801D2DD2 + 5 * a2) - 2146887592);
*(_WORD *)(a1 + 284) = *((_WORD *)&unk_801D2DD6 + 5 * a2);
result = (char *)*((unsigned __int16 *)&unk_801D2DD4 + 5 * a2);
*(_WORD *)(a1 + 288) = (_WORD)result;
word_8008ECFC = (__int16)result;
...
g_PrHudOverlaySlots[0].id = *v3;
g_PrHudOverlaySlots[0].descPtr = result;
g_PrHudOverlaySlots[0].baseFrame = v4;
*((_DWORD *)result + 2) = 0;
```

`src/pr/pr_stage1_runtime_slots_direct.cpp` current Win carrier:

```cpp
void ApplyStage1RuntimeHudOverlayConfigure801CA0E0(
    PrStage1RuntimeSlotsDirectRuntime& runtime,
    const PrStage1OverlayData& data,
    const PrStage1RuntimeSlotsDirectDescBankSelectorInput& input,
    uint32_t queryFrame,
    uint32_t scriptFrame) {
    if (!input.hudOverlayConfigure801CA0E0Valid) {
        return;
    }
    ...
    runtime.sceneSubmit.ctxFlagsFull |= 0x00000010u;
    runtime.sceneSubmit.ctxFlagsPulse801CBFDC |= 0x00000010u;
    runtime.sceneSubmit.ctx11C = static_cast<uint16_t>(row->ctx11CValue);
    runtime.sceneSubmit.ctx120 = static_cast<uint16_t>(row->timerFrames);
    runtime.word8008ECFCResource10000TimerFrames =
        static_cast<int32_t>(row->timerFrames);
    ...
}
```

`src/pr/pr_stage_scene_submit_backend.cpp` has a suspicious empty reset hook:

```cpp
static void ResetStage1FaceTimUploads801CBFDC() {}
```

The same file consumes face TIM upload commands every submit advance:

```cpp
ConsumeStage1FaceTimUploadCommands801CBFDC(
    ctx,
    PrStageSceneSubmitDirect::
        GetOwnedStage1SceneSubmitRuntime801CBFDC190());
```

Do not patch the empty reset just because it looks suspicious. First prove from
PSX pseudo-C which reset owner clears the expression/TIM carrier on Stage1
retry/entry.

## Stage1 entry / audio evidence already in repo

`docs/stage1_decomp/801C79E4_sub_801C79E4.c`:

```c
dword_801D303C = 0;
MEMORY[0x80026FA4]();
PrStageRunner_InitUI();
MEMORY[0x8001A478](*(__int16 *)(a1 + 6));
MEMORY[0x8001A654](*(__int16 *)(a1 + 4));
MEMORY[0x80014344]();
MEMORY[0x80024E98]();
MEMORY[0x80024FC0](a2);
return PrStageRunner_ResetState();
```

`docs/stage1_decomp/try_again_retry_audio_face_20260526.md` includes local IDA
rechecks for:

- `8001A4D0`
- `8001A478`
- `8001A654`
- `8001A4A4`
- `80026FA4`
- `8001A694`

The current suspected area for missing CD music should be start/filter/volume
or reset/flush ownership around those direct functions and their Win carriers,
not transition-time XA pumping.

## Request for Pro audit

Work only from pushed branch `codex/stage1-gpt55-pro-handoff-20260524`.
Runtime code must be interpreted relative to baseline `fc1c1db7`; later commits
on this branch may be documentation-only handoff notes.

Environment limits:

- You cannot access my local files, unpushed changes, IDA session, emulator,
  debugger, terminal, or build output.
- You cannot modify files, commit, push, run build/test, clone repos, or use
  Python to access the network.
- Work only from pasted content and the pushed branch/commit I specify.
- If a safe patch is justified, output a `git apply --index` compatible unified
  diff.
- If not justified, output `no safe patch`, then list the exact missing evidence
  or IDA addresses needed. Codex will export local IDA evidence and push it.

Audit priorities:

1. Face / expression reset after Try Again YES retry. Determine whether the
   failed-frame expression persists because a direct carrier is not reset, a
   reset is applied to the wrong carrier, or a `801CA0E0`/TIM command is missing
   on retry entry. Do not add renderer or atlas symptom patches.
2. CD music missing on first Stage1 start and after retry. The reverted patch
   proves the fix is not transition-time XA pumping. Find the direct mismatch in
   start/filter/volume/callback/reset ordering.
3. If more IDA evidence is needed, request exact function addresses first.

Forbidden patch classes:

- renderer/UI/shell visual fallback;
- observer/matcher/sentinel/harness changes;
- intro-transition XA/player/ring-packet pumping;
- generic clock/audio freeze;
- changes outside Stage1 direct-port seams unless required by a proven shared
  PSX helper mismatch.
