# Stage1 retry audio / face follow-up 2 - 2026-05-26

## Current baseline

- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Current pushed HEAD before this note: `ab70ed80`
- Patch under test: `ab70ed80 fix: apply Stage1 runner entry BGM volume`
- Full build for `ab70ed80` succeeded locally.

## User test result

User reported:

```text
修复均未命中
```

Interpretation for the next audit:

- `ab70ed80` did not fix first-play Stage1 CD music missing.
- `ab70ed80` did not fix Try Again YES retry CD music missing.
- `ab70ed80` did not fix retry transition frozen-frame expression retaining the
  failed expression.
- The previous negative boundary still applies: do not revive
  `9a9a63ec` / intro-transition XA pump. That was already proven to make the
  gameplay/process advance behind a static frozen frame.

## What `ab70ed80` changed

`ab70ed80` applied the Pro-suggested `801C79E4 -> 8001A478(*(stageRecord+6))`
entry carrier:

- added generic `PrStage1XaCdDirectApplySub8001A478(int16_t)`;
- kept `8001A4A4(a1)` as a wrapper over `8001A478(a1 == 1 ? 0 : 0x7F)`;
- passed `stageRecord+4/+6` from `sceneEntry+0x9C` loader
  `opaqueA1Plus04` into `PrStageRunnerDirectApplySub801C79E4`;
- applied `8001A478` volume before `8001A654` filter in
  `ApplyStage1RunnerDirectInitLifecycle`.

This may still be a direct-port correctness fix, but it is not sufficient for
the observed missing music symptoms. The next Pro audit must explicitly decide:

1. whether `ab70ed80` should be kept as correct-but-insufficient;
2. whether it should be reverted in the next diff because it is the wrong
   carrier for this path;
3. what exact next mismatch explains both first-play and retry missing CD music.

## Active unresolved symptoms

1. First-play Stage1 opening CD music is still missing; only later rating switch
   can bring background music back.
2. Try Again YES retry CD music is still missing.
3. Try Again YES retry transition frozen frame still keeps failed expression.

## Request for Pro audit

Work only from pushed branch `codex/stage1-gpt55-pro-handoff-20260524`.

Environment limits:

- You cannot access my local files, unpushed changes, IDA session, emulator,
  debugger, terminal, or build output.
- You cannot modify files, commit, push, run build/test, clone repos, or use
  Python to access the network.
- Work only from pasted content and the pushed branch/commit I specify.
- If a safe patch is justified, output a `git apply --index` compatible unified
  diff.
- If not justified, output `no safe patch`, then list the exact missing evidence
  or IDA addresses needed. Codex can export local IDA evidence and push it.

Forbidden directions:

- Do not re-propose intro-transition XA/player/ring-packet pumping.
- Do not advance StageRunner/gameplay timecode/direct XA clock behind frozen
  transition frames.
- Do not add renderer/UI/shell/atlas symptom patches.
- Do not touch observer/matcher/sentinel/harness.

Audit priorities:

1. Re-evaluate CD music missing after `ab70ed80` missed visually. If
   `8001A478` is correct but insufficient, find the next direct mismatch in
   `8001A4D0`, lower-CD callback/state, `xa1Player->Play/SetFilter/SetVolume`
   ordering, `8001A3C8/8001A750`, or shared audio reset/flush barriers.
2. Decide whether the next safe diff should keep or revert `ab70ed80`.
3. For face/expression frozen-frame reset, either provide a proven direct diff
   or request exact IDA exports.
