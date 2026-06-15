# Stage1 v3 GOOD row / clear-tail boundary (2026-05-17)

Scope: narrow recorder facts for the user v3 full Stage1 GOOD clear path. This
does not authorize renderer, lifecycle, marker, no-fail, or clear-tail
synthetic success patches.

Recording:

```text
E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec
```

## Row fact

`ctx+0x4E` / `word_801C368E` is the right-rank active row. In this v3 GOOD
clear path, it stays `1` for the whole recording:

```text
changes 0x801C368E 2 0 14385 -> 0 found
read 0x801C368E[2]:
  frame 5399 = 1
  frame 5513 = 1
  frame 5775 = 1
  frame 6035 = 1
  frame 6297 = 1
  frame 6557 = 1
  frame 6819 = 1
  frame 7079 = 1
  frame 7341 = 1
  frame 8255 = 1
  frame 8515 = 1
  frame 8775 = 1
  frame 9000 = 1
```

Nearby byte checks `0x801C368C..0x801C3690` also showed no changes in the full
recording. The expected GOOD clear-tail condition therefore is not "row3 later
recovers"; it is "row remains GOOD (`1`) while scorer fields update."

## Scorer facts

The scorer/right-rank helper state still updates across the song:

| Frame | `80091816` | `80091818` | `8008ED36` | `8008ED38` |
|---:|---|---|---|---|
| `5513` | `40 -> 52` | `40 -> 52` | `2 -> 1` | `0 -> 1` |
| `5775` | `52 -> 43` | `52 -> 43` | `1 -> 2` | `1 -> 0` |
| `6035` | `43 -> 46` | `43 -> 46` | `2 -> 1` | `0 -> 1` |
| `6297` | `46 -> 61` | `46 -> 61` | `1 -> 2` | `1 -> 0` |
| `6557` | `61 -> 79` | `61 -> 79` | `2 -> 1` | `0 -> 1` |
| `6819` | `79 -> 84` | `79 -> 84` | `1 -> 2` | `1 -> 0` |
| `7079` | `84 -> 82` | `84 -> 82` | `2 -> 0` | `0 -> 1` |
| `7341` | `82 -> 88` | `82 -> 88` | `0 -> 2` | `1 -> 0` |
| `8255` | `88 -> 106` | `88 -> 106` | `2 -> 1` | `0 -> 1` |
| `8515` | `106 -> 103` | `106 -> 103` | `1 -> 2` | `1 -> 0` |
| `8775` | `103 -> 109` | `103 -> 109` | `2 -> 1` | `0 -> 1` |

Narrow `writers f-1..f+1` resolves the PC owners:

- `80091816`: `8001528C`;
- `80091818`: `80014D44`;
- `8008ED36`: `80025434`, `80025494`, or `80024FA0`;
- `8008ED38`: `80025440` or `8002549C`.

## Boundary conclusion

The direct clear-tail gate must stay strict: `rightRankActiveRow > 1` blocks the
terminal tail. The v3 PSX GOOD path reaches the terminal path with
`rightRankActiveRow == 1`; it does not require relaxing `801C9094` clear-tail
logic.

If the Win runtime still reports row `3` near clear-tail, the next executable
work is to compare the Win row-publish inputs against this PSX v3 row-constant
path:

```text
80035510 live input -> 80014614 accepted records -> 80014D58/80014D28/80024F8C
-> bucket30 row publish -> rightRankActiveRow
```

Do not patch lifecycle/marker/renderer to force clear, and do not keep pursuing
a PSX row3 recovery path for this v3 GOOD recording.

## Win full-song comparison boundary

Latest full-song runtime artifacts do not contradict the v3 recorder row fact:

```text
20260517_003901:
  query 565: row 1 -> 2, v22 0, writeback24 0
  query 696: row 2 -> 3, v22 0, writeback24 0
  v22=1 samples: 0
  positive-delta samples: 0

20260516_224538:
  query 565: row 1 -> 2, v22 0, writeback24 0
  query 696: row 2 -> 3, v22 0, writeback24 0
  later positive deltas occur after row is already 3

20260516_203744:
  query 565: row 1 -> 2, v22 0, writeback24 0
  query 696: row 2 -> 3, v22 0, writeback24 0
  later positive deltas occur after row is already 3
```

Those artifacts are fixed full-song driver runs, already classified as
Circle-biased. They are not valid authority for changing the v3 GOOD row
constant path. The remaining runtime work is to drive or observe the actual
playable input path that can supply the PSX `0x10/class1` first beat followed
by `0x20/class2`, then evaluate clear-tail on that path.

## Current clear-tail code boundary

Static recheck of the current runtime path:

- `PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(...)`
  returns false when `rightRankActiveRow > 1`.
- When the terminal-tail pulse is present, `801C9094` arms it only if:
  - `clearTerminalTailArmed == false`;
  - `activeDispatchStream == 0`;
  - `pendingRatingBranchSeq == consumedRatingBranchSeq`.
- `PrimeStage1FormalLifecycleRuntime(...)` now keeps `clearGate` strictly on
  the late-branch `flag100` source stream 2/3. `clearTerminalTailGate` only
  reflects an armed-but-not-yet-completed terminal tail and does not use Win
  duration / runner progress as clear authority.
- A follow-up recorder check on frame `11407` confirms the post-clear status
  parameter direction for the GOOD row:
  - `read 11407 0x801C368E 2` = `0x0001`;
  - `call_trace 11407` includes `8001635C` from `801C8494`;
  - `writers 0x80092F1D 1 11406 11408` reports
    `PC=0x80016408: 0x01 -> 0x02`;
  - `read 11406/11407 0x80092F1D 4` changes from `0x00000001` to
    `0x00000102`.
  This matches the current direct path where row `1` yields the GOOD
  status-promotion target `2`; it is not a reason to invert clear-tail movie or
  save-status parameters.

That matches the v3 boundary: row `1` is allowed to reach the terminal-tail
trigger, row `2/3` is not. Therefore the known full-song block is still the
input/source row path, not a reason to relax clear-tail, lifecycle, marker, or
renderer logic.

## Playable input path boundary

Current runtime source check:

- `PrPad::Poll(...)` reads configured keyboard bindings every render step, and
  `PrPad::CommitLogicFrame()` latches them once per 30 Hz logic tick.
- Stage1 numeric runtime reads playable input through:

```text
PrPad::GetState(0).held
-> BuildStage1NumericRuntimeInputMask(...)
-> NormalizeLocalPrPadMaskToReturnedMask80035510(...)
-> PsxReadPadMask80035510(...)
-> CaptureStage1SteadyInput7A60(...)
-> PrStageRunnerDirectResolveInputDispatch7A60(...)
```

- In the non-replay path, `PrStageRunnerDirectResolveInputDispatch7A60(...)`
  emits `80035510(1)`, writes the returned mask to `ctx+0x18`, calls
  `80024B54` for non-zero input, and writes the resulting class to `ctx+0x20`.
- `ctx.debugPadInput` is only added as explicit debug-server injection, plus a
  Scene0 keyboard fallback. It is not the normal Stage1 playable input source.
- No PaRappaWin/DuckStation process or debug server was running during this
  recheck, so this is a static runtime-source boundary only, not a live clear
  proof.

Boundary: the actual playable path can carry `0x10/class1` and `0x20/class2`
without changing scorer code. The fixed full-song debug driver remains a
separate Circle-biased source and must not be treated as the playable path.
