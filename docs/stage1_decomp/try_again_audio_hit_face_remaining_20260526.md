# Stage1 retry audio hit / face remaining - 2026-05-26

## Current baseline

- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Latest tested commit: `39b31cb3`
- Build before user test: full local `.\build.ps1` passed.

## User test result

```text
1.修复命中
2.修复命中
3.修复未命中
```

Interpretation:

1. First-play Stage1 CD music is restored.
2. Try Again YES retry CD music is restored.
3. Try Again YES retry transition frozen frame still keeps the failed
   expression.

## Decisions

- Keep `39b31cb3 fix: reset Stage1 XA voice after audio barrier`.
- Stop changing the audio line for this defect. Do not continue variants of:
  - `8001A478` volume carrier;
  - `8001A4D0` channel carrier;
  - intro-transition XA/player/ring-packet pump;
  - additional clock/audio freeze or pump during frozen transition frames.
- Remaining work is only the direct face/TIM reset line.

## Remaining evidence target

The next patch must prove a mismatch in the face/TIM path rather than infer from
the frozen-frame visual symptom. Known evidence targets:

- `801CA0E0` and `801CA264` expression/HUD slot writers;
- `word_8008ECFC`, `g_PrHudOverlaySlots[0]`, and ctx fields
  `+220/+232/+284/+288`;
- `801CBFDC -> 8001ADEC` TIM upload owner/consumer;
- Win carriers:
  - `ResetStage1FaceTimUploads801CBFDC()`;
  - `ConsumeStage1FaceTimUploadCommands801CBFDC()`;
  - owned scene-submit runtime face/TIM command queues.

No renderer/UI/shell/atlas symptom patch is allowed.
