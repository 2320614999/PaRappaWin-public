# Stage1 retry transition face preview audit - 2026-05-26

## Current user result

User tested pushed commit `39b31cb3`:

```text
1.修复命中
2.修复命中
3.修复未命中
```

Interpretation:

- First-play Stage1 CD music is fixed.
- Try Again YES retry CD music is fixed.
- Remaining bug: Try Again YES retry transition frozen frame keeps the failed
  expression instead of starting from the fresh Stage1 expression state.

Audio is no longer the target for this bug.

## Local IDA probe

COMOD1 IDB was found at:

```text
E:\game\PSgame\parappa the rapper\S1\COMOD1.BIN.i64
```

Probe output is in:

```text
docs/stage1_decomp/retry_face_reset_owner_20260526/ida_probe_comod1_segments_20260526.txt
```

It confirms the Stage1 overlay functions exist at the expected addresses:

```text
801CBFDC PrStageRunner_RenderFrame
801CA0E0 PrStageRunner_HudOverlay_Configure
801CA1EC PrStageRunner_HudOverlay_Start
801CA25C PrStageRunner_HudOverlay_NoOp
801CA264 PrStageRunner_HudOverlay_Update
801C9094 PrStageRunner_FrameUpdate
801C895C PrStageRunner_ApplyScriptEvent
```

The existing IDA export
`docs/stage1_decomp/ida_export_stage1_mime_action_chain_801cbfdc_801cb190_801ca0e0_20260517.txt`
already contains the relevant pseudo-C:

- `801CA0E0` sets `ctx|=0x10`, writes `ctx+220`, `ctx+232`, `ctx+284`,
  `ctx+288`, writes `word_8008ECFC`, and can update
  `g_PrHudOverlaySlots[0]`.
- `801CA264` can set selector/camera/follow-up state based on accepted tail
  conditions.
- `801CBFDC` consumes `ctx&0x10` through `800140E0(ctx+220, ctx+232, 0,
  ctx+284, ctx+288)` and consumes `ctx&0x8000` TIM upload lists through
  `8001ADEC(...)`.

## Narrow Win suspect

The retry transition preview path in `src/pr/pr_scenes.cpp` constructs a fresh
`previewRuntime`, passes no `801C9094` event-stream runtime, but still carries
follow-up facts from the live numeric runtime:

```cpp
PrStage1RuntimeSlotsDirectFollowUpFacts801CBFDC facts =
    BuildStage1RuntimeSlotsFollowUpFacts801CBFDC(s_stage1NumericRuntime);
if (facts.rightRankActiveRow == 0u) {
    facts.rightRankActiveRow = kStage1InitialRightRankActiveRow;
}

PrStage1RuntimeSlotsDirectRuntime previewRuntime{};
const uint32_t queryFrame = 0u;
const uint32_t scriptFrame = 0u;
const PrStage1RuntimeSlotsDirectAdvanceCarrier801C9094 carrier =
    PrStage1RuntimeSlotsDirectBuildAdvanceCarrier801C9094(
        queryFrame,
        scriptFrame,
        BuildStage1GlobalOptionsCarrier801C7A60(ctx),
        nullptr,
        facts);
```

`BuildStage1RuntimeSlotsFollowUpFacts801CBFDC(...)` copies more than the right
rank row. It also carries:

```cpp
facts.descBankSelectorPulse
facts.descBankCtxFlagsPulse801CBFDC
facts.cameraPulse9443C
facts.acceptedTailTimer9443CPulse
facts.hudOverlayConfigure801CA0E0Pulse
facts.hudOverlayConfigure801CA0E0Arg
facts.hudOverlayConfigure801CA0E0QueryFrame
facts.hudOverlayConfigure801CA0E0ScriptFrame
facts.compactPainterGate7A
facts.ctxFlagTickAdvanceKnown801C9094
facts.ctxFlagTickAdvance801C9094
```

This is suspicious for the remaining bug because retry transition preview is
fresh (`previewRuntime{}`, `queryFrame=0`, `scriptFrame=0`, no event-stream
runtime), but the face/HUD follow-up pulse can still come from the previous
failed gameplay numeric runtime. If that stale `hudOverlayConfigure801CA0E0`
fact is applied to the preview runtime, `801CA0E0 -> 801CBFDC -> 800140E0` can
re-upload the failed expression into the transition frozen frame.

## What should be proven before patching

Do not patch only because the visual symptom matches. Pro should answer one of:

1. Safe diff: prove transition preview should only carry the minimal fresh
   `rightRankActiveRow`/initial rank fact, and must not carry stale
   `hudOverlayConfigure801CA0E0*`, selector, camera, accepted-tail timer, or
   ctx tick facts from `s_stage1NumericRuntime`.
2. No safe patch: identify the missing PSX pseudo-C/Win carrier evidence needed
   to decide which follow-up fields are legal during `allowTransitionFreeze`
   preview.

Constraints:

- Do not change audio: `39b31cb3` already fixed the two CD music symptoms.
- Do not reintroduce intro-transition XA pump.
- Do not add renderer/UI/shell/atlas symptom patches.
- Do not add observer/matcher/sentinel/harness.
