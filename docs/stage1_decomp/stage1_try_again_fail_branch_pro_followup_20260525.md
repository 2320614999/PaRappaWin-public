# Stage1 Try Again / fail-tail Pro follow-up 2026-05-25

## Purpose

This is the narrow follow-up for web GPT Pro after commit `67f54a6a`.
The previous Pro pass returned `no safe patch`, but the local Codex follow-up
found a more concrete candidate seam: current Win code still feeds unknown
`8001A7F8/8001A3B8` gate facts into the `801C7A60` main-loop tail even though
some direct carriers already exist.

Do not treat this file as approval to patch. Use it to audit whether the carrier
mapping is now evidence-complete.

## Branch

- Repository: `https://github.com/2320614999/PaRappaWin`
- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Start from the latest pushed HEAD after this follow-up doc is committed.

## Current user-visible problems

- Try Again appears.
- Transparent gray-black Try Again backdrop covers 1920x1080 widescreen.
- Compact rail/HUD no longer leaks into Try Again underlay.
- Still wrong:
  - black flash just before Try Again;
  - fail branch gameplay time / action trigger / animation / music-SFX linkage.

## New local finding to audit

The current code still calls:

```cpp
PrStageRunnerDirectResolveMainLoopTail7A60(
    PrStageRunnerDirectMainLoopTail7A60Input{
        postFrame.continueMainLoop,
        sceneEntryField352FallbackTickAdvance,
        false,
        false,
        false,
        false,
    });
```

Current location:

- `src/pr/pr_stage1_scorer_host.cpp:4657`
- `src/pr/pr_stage1_scorer_host.cpp:4663-4669`

This means:

- `continuationGate1A7F8Known = false`
- `continuationGate1A7F8ReturnsOne = false`
- `frameExit1A3B8Known = false`
- `frameExit1A3B8ReturnsOne = false`

But current code already has some candidate fact carriers:

- `src/pr/pr_stage1_xa_cd_direct.h:115-120`
  - `dword_80049404Known/value`
  - `dword_80049428Known/value`
- `src/pr/pr_stage1_movie_text_outer_loop_direct.h:124-133`
  - `dword80049404Known/value`
  - `segmentEndA1Plus44Known/value`
  - `segmentEndBiasA1Plus8Known/value`
  - `sub8001A7F8Known/result`
- `src/pr/pr_stage1_movie_text_outer_loop_direct.cpp:302-324`
  - already computes `segmentEnd + segmentEndBias <= watchdogValue`
  - already emits `8001A7F8` known/gap action state
- `src/pr/pr_scenes.cpp:3393-3403`
  - calls `TickStage1MovieTextOuterLoopFromHostFacts801C455C(...)`
  - receives `outerLoopStep`, but current code does not write
    `outerLoopStep.sub8001A7F8Known/result` back to `ctx.stage1XaCdDirect`.
- `src/pr/pr_stage1_scorer_host.inl:478-481`
  - calls `ResolveStage1RunnerMainLoopTail7A60(...)` without gate facts.

Historical candidate commits that should be audited at hunk level, not blindly
cherry-picked:

- `4259a8a2 fix: wire Stage1 tail A3B8 from xa/cd state`
  - mapped `8001A3B8()` from `ctx.stage1XaCdDirect.dword_80049428Known/value`
  - prior note said `8001A7F8` remained a gap.
- `7b0c1cb2 fix: wire Stage1 A7F8 same-frame carrier`
  - added `sub8001A7F8Known/result` to `PrStage1XaCdDirectState`
  - copied `outerLoopStep.sub8001A7F8Known/result` into
    `ctx.stage1XaCdDirect`
  - passed both `8001A7F8` and `8001A3B8` facts into
    `ResolveStage1RunnerMainLoopTail7A60(...)`.

These commits were later isolated with other overnight changes. Re-apply only
if the current `67f54a6a+` evidence proves the hunk is still correct.

## PSX authority

`docs/stage1_decomp/801C7A60_sub_801C7A60.c:221-225`:

```c
MEMORY[0x80015350](a2, 2 * *(_DWORD *)(MEMORY[0x8006EDB8] + 352));
MEMORY[0x80035560](2);
PrStageRunner_PresentFrame(a2);
v32 = MEMORY[0x8001A7F8](a1) != 1;
if ( MEMORY[0x8001A3B8]() == 1 )
```

New IDA export files:

- `docs/stage1_decomp/fail_terminal_cleanup_gate_20260525/ida_export_fail_terminal_cleanup_gate_20260525.md`
- `docs/stage1_decomp/fail_terminal_cleanup_gate_20260525/analysis_fail_terminal_cleanup_gate_20260525.md`

Key facts:

- `8001A7F8(a1)` depends on `a1+0x08`, `a1+0x2C`, and `dword_80049404`.
- `8001A3B8()` returns `dword_80049428` in disassembly; Hex-Rays collapses it
  to `return 0` only because of static initial value.
- `80015350` only decrements `word_80091824`; do not infer runner/audio freeze
  from it.

## What Pro should answer

Audit only this narrow question first:

1. Is the `4259a8a2` hunk still a safe direct-port patch on current HEAD?
2. Is the `7b0c1cb2` hunk still a safe direct-port patch on current HEAD?
3. If safe, output one `git apply --index` compatible unified diff that only
   wires the proven gate facts and updates docs.
4. If not safe, output `no safe patch` and the exact missing carrier or field.

Do not:

- add renderer/UI/shell symptom patches;
- delete `DrawGameplaySubmitFrozenRuntimeBaseOnly(ctx)`;
- add generic runner/XA/audio freeze;
- add observer/harness/sentinel changes;
- touch unrelated Stage1 visual work.

## Secondary MoveImage question

The MoveImage carrier gap is still separate. Current known mismatch:

- `src/pr/pr_psx_event_frame_direct.cpp:1565-1580` ignores `destX/destY` and
  only preserves DMA dispatch shape.
- `src/pr/pr_ui_overlay.cpp:693-704` still uses
  `DrawGameplaySubmitFrozenRuntimeBaseOnly(ctx)` as a temporary source-frame
  stand-in.

Do not patch this unless you can identify a real direct framebuffer/page pixel
carrier. The gate-fact seam above is the higher-probability narrow patch.
