# Stage1 retry audio / face follow-up 3 - 2026-05-26

## Current baseline

- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Current pushed HEAD before this note: `1ab24cf5`
- Recent patches under test:
  - `ab70ed80 fix: apply Stage1 runner entry BGM volume`
  - `1ab24cf5 fix: feed Stage1 record channel into XA tick`
- Both patches had full local builds before user testing.

## User test result

User reported again:

```text
修复均未命中
```

Interpretation:

- `ab70ed80` did not fix first-play Stage1 CD music missing.
- `ab70ed80` did not fix Try Again YES retry CD music missing.
- `1ab24cf5` also did not fix first-play Stage1 CD music missing.
- `1ab24cf5` also did not fix Try Again YES retry CD music missing.
- Neither patch fixed retry transition frozen-frame expression retaining the
  failed expression.

Treat both as negative visual evidence. Do not continue proposing variants of:

- `8001A478` BGM volume carrier;
- `8001A4D0` `initialChannel` / `stageRecord+4` carrier;
- intro-transition XA/player/ring-packet pump (`9a9a63ec`), already proven
  harmful because gameplay/process advanced behind a static frozen frame.

## Current unresolved symptoms

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
- Work only from pushed branch/commit and pasted content.
- If a safe patch is justified, output a `git apply --index` compatible unified
  diff.
- If not justified, output `no safe patch`, then list the exact missing IDA
  addresses/evidence. Codex can export local IDA evidence and push it.

Required decision before any next patch:

1. Decide whether `ab70ed80` and/or `1ab24cf5` should stay as
   correct-but-insufficient direct-port fixes, or be reverted in the next diff.
2. Do not produce another patch on the already-failed `8001A478` or
   `initialChannel/stageRecord+4` seams unless it also explains why the previous
   exact implementations missed.
3. For CD music, shift to a different proven seam: likely lower-CD callback /
   voice/callback carrier rebuild after `80026FA4`, `8001A4D0` command-complete
   state, `8001A3C8/8001A750`, shared audio reset/flush barriers, or
   host `Xa1Player` start/playback state not matching the direct carrier.
4. For face/expression frozen frame, either provide a direct diff with proof or
   request exact IDA exports. The prior missing-evidence list was:
   - full PSX pseudo-C/disasm for the face/TIM upload owner in `801CBFDC`;
   - exact helper(s) that consume/clear face TIM upload commands;
   - xrefs to the queue/slots backing expression payload;
   - full PSX pseudo-C/disasm or xrefs for `801CA0E0`, `801CA264`,
     `word_8008ECFC`, `g_PrHudOverlaySlots[0]`, and ctx fields
     `+220/+232/+284/+288`;
   - Win carrier map for `ResetStage1FaceTimUploads801CBFDC()`,
     `ConsumeStage1FaceTimUploadCommands801CBFDC()`, and owned scene-submit
     face/TIM runtime fields.

Forbidden directions:

- No renderer/UI/shell/atlas symptom patches.
- No observer/matcher/sentinel/harness changes.
- No intro-transition XA/player/ring-packet pump.
- No advancing StageRunner/gameplay timecode/direct XA clock behind frozen
  transition frames.
