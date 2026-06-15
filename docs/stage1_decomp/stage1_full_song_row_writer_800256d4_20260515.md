# Stage1 full-song row writer 800256D4

Date: 2026-05-15

Scope: Stage1 full-song terminal / flag100 only. This file does not change
lower-CD, Rail/GTE, case17, RGB, MenuHelp/StageClear, or boot seams.

## Recorder hit

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Window:

`0..13620`, with focused checks on `12000..13620` and `4240..4260`.

Facts:

- `changes 0x801C368E 2 0 13620`:
  - frame `3473`: `1 -> 2`
  - frame `3735`: `2 -> 3`
  - frame `3995`: `3 -> 2`
  - frame `4255`: `2 -> 1`
- `writers 0x801C368E 2 4240 4260`:
  - frame `4255`, writer `PC=0x800256D4`, value `0x0002 -> 0x0001`
- Tail reads:
  - frame `12000`, `0x801C368E[2] = 1`
  - frame `13620`, `0x801C368E[2] = 1`
  - frame `13620`, `0x801C3640[4] = 0`
  - frame `13620`, `0x801C3694[2] = 1`
- Focused `changes` for `0x801C368E/0x801C3640/0x801C3694/0x801C36B6`
  in `12000..13620` returned no changes, so the save/terminal tail window
  is stable after the earlier row writeback.

## Phase-cache snapshot

Focused reads around the four PSX row-write frames:

| frame | row `801C368E` | anim `801C37CE` | `8008ED36` | `8008ED38` | `8008ED00` | tick `801C364C` |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 3472 | 1 | 0 | 0 | 1 | 0 | 3407 |
| 3473 | 2 | 1 | 2 | 0 | 6 | 3409 |
| 3474 | 2 | 1 | 2 | 0 | 6 | 3414 |
| 3734 | 2 | 1 | 0 | 1 | 0 | 4179 |
| 3735 | 3 | 2 | 2 | 0 | 6 | 4182 |
| 3736 | 3 | 2 | 2 | 0 | 6 | 4184 |
| 3994 | 3 | 2 | 1 | 1 | 0 | 4944 |
| 3995 | 2 | 3 | 2 | 0 | 6 | 4947 |
| 3996 | 2 | 3 | 2 | 0 | 6 | 4949 |
| 4254 | 2 | 3 | 1 | 1 | 0 | 5707 |
| 4255 | 1 | 4 | 2 | 0 | 6 | 5712 |
| 4256 | 1 | 4 | 2 | 0 | 6 | 5714 |

Focused writers in `4240..4260`:

- `0x801C37CE[2]`: frame `4255`, `PC=0x8002570C`, `3 -> 4`.
- `0x8008ED36[2]`: frame `4255`, `PC=0x80024FA0`, `1 -> 2`.
- `0x8008ED38[2]`: frame `4255`, `PC=0x8002549C`, `1 -> 0`.
- `0x8008ED00[4]`: frame `4255`, `PC=0x80025670`, `0 -> 6`.

This closes the local authority for the last `2 -> 1` row write: frame
`4255` is the normal `sub_80024FD0` row-write branch with
`row=2 && v22=1`, not the later delayed-tail forced-good branch at
`0x800258A4`. The immediately preceding PSX state is
`ED36=1, ED38=1, ED00=0`; the row write then resets through `24F8C` and
sets `ED00=6`.

## IDA narrow export

New PC hit `0x800256D4` belongs to Hex-Rays function:

`sub_80024FD0(int a1)` at `0x80024FD0`.

Relevant Hex-Rays excerpt around the hit:

```c
v31 = *(__int16 *)(a1 + 78);
if ( v31 == 1 )
{
  if ( v22 == 1 )
  {
    *(_WORD *)(a1 + 78) = 0;
    *(_WORD *)(a1 + 398) = 5;
    *(_WORD *)(a1 + 122) = 0;
  }
  else
  {
    *(_WORD *)(a1 + 78) = 2;
    *(_WORD *)(a1 + 398) = 1;
    *(_WORD *)(a1 + 116) = *(_WORD *)(a1 + 116) + 1;
  }
}
else if ( v31 == 2 )
{
  if ( v22 == 1 )
  {
    *(_WORD *)(a1 + 78) = 1;   // 0x800256D4 writer
    v33 = 4;
  }
  else
  {
    *(_WORD *)(a1 + 78) = 3;
    v33 = 2;
  }
}
```

## Win runtime contrast

Current rebuilt runtime artifact:

`artifacts/stage1_rail_probe_runtime/20260515_182035_stage1_rail_full_song_runtime/summary.txt`

Runtime facts at the timeout tail:

- `max_query_frame=4806`
- `full_song.liveClearTerminalTailGate=True`
- `full_song.liveClearGate=False`
- `full_song.liveFailGate=False`
- `full_song.liveClearTailPulse=False`
- `full_song.livePendingRatingBranchSeq=6`
- `full_song.liveConsumedRatingBranchSeq=6`
- `full_song.liveClearTailArmed=False`
- `full_song.liveClearTailDispatchActive=False`
- `full_song.liveActiveDispatchStartScriptFrame=0`
- `full_song.liveActiveDispatchTerminalEndLocalFrame=0`
- `terminalValid=False`

Interpretation:

- The current Win runtime does not reach a clear-tail pulse into
  `801C9094 Advance`; this is not an active-dispatch or pending-seq block.
- The PSX recording remains row `1` in the late tail/save window, while the
  Win full-song runtime tail currently reports formal lifecycle row `3`.
- Do not authorize clear from `sceneExitReason=3`, F5/no-fail, renderer,
  stage status, payload, or default values. The next direct-port work should
  reconcile the runtime source for `sub_80014D28 -> ED36/ED38 -> v22=1`
  after the row3/row2 tail samples, then let the existing `sub_80024FD0`
  row-write path publish the PSX row sequence.

## `sub_80014D28` second-beat facts

Follow-up recorder/xref pass focused only on the two downward row-write
windows. Both windows execute the second `sub_80014D28` sample; PSX does not
skip phase1 when `ED38` is armed.

| frame | row write | call/return | input | accumulator before `14D28` | `14D28` baseline write | later snapshots | scorer counters |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `3995` | `3 -> 2` at `80025704` | `800253A4 -> 80014D28`, returns `800253AC` | `0x2000` | `8001528C` writes `80091816: 9 -> 18` | `80014D28` writes `80091818: 9 -> 18` | `800144A8/800144B0` write `8009181A/1C: 0 -> 18` | `91810=4, 91812=4, 91814=0` |
| `4255` | `2 -> 1` at `800256D4` | `800253A4 -> 80014D28`, returns `800253AC` | `0` | `8001528C` writes `80091816: 27 -> 32` | `80014D28` writes `80091818: 27 -> 32` | `800144A8/800144B0` write `8009181A/1C: 18 -> 32` | `91810=3, 91812=2, 91814=1` |

Additional same-window facts:

- `ED08=0x0000000E` at `3472/3473/3734/3735/3993/3994/3995/4253/4254/4255`;
  both the phase path and row-write path are enabled by descriptor flags.
- The downward row writes happen after `14D58` has already produced positive
  `91816` growth in that same frame, so `sub_80014D28` returns true by the
  direct `91816 - old(91818) > 0` rule.
- The Win gap is therefore not "skip the armed second sample"; it is whether
  the Win runtime produces the same `14D58 -> 91816` growth before
  `14D28`, then publishes `ED36/ED38 -> v22=1` through `sub_80024FD0`.

Runtime observer follow-up:

- `stage1ovl handoff` and `stage1ovl handoffhistory` now expose the existing
  direct-port state for `91816/91818/9181A/9181C/9181E/ED08/ED36/ED38` plus
  the parser-compatible `bucket30Owner*` field names.
- `--stage1-rail-full-song-runtime` now summarizes the parsed `stage1ovl
  handoff` numeric fields under `full_song.row_writeback`: sample counts for
  armed `ED38`, positive `91816-91818` delta, `v22=1`, and row-write samples,
  plus the last matching record for each category.
- This is observer-only. It does not change scorer behavior, row authority,
  lifecycle clear/fail gates, renderer, input, or no-fail handling.

Direct-port follow-up:

- `BuildStage1DirectPortGlobalsAdapter(...)` now feeds `14D58`'s
  `currentPageOrdinal1Based` from the same-frame `ctx+56` mirror
  (`descriptorCadence.pageOrdinal56`) before falling back to the page-record
  mirror. This matches the PSX `sub_80014D58` use of `*(a1 + 56)` for the
  lookback page and avoids treating stale page-record state as the formula
  reader authority.
- The bucket30 owner direct slice now captures its own observer package:
  busy/scorer-window gates, owner-kernel entry, phase1 sample, formal
  writeback, tie-breaker, resolver gates, `v22`, good-to-cool commit, and
  after-produce `91818/9181A/9181C/ED36/ED38`. These are observable facts from
  the direct slice, not new success guards.
- `14D58` unknown-row/unknown-page paths were not converted into synthetic
  success. If a future full-song run still shows no positive `91816` growth
  after the `ctx+56` fix, the next evidence window is the direct commit
  package (`commitTermKnown`, score writeback, reader page) rather than
  lifecycle or renderer patching.

Zero-accepted direct fix:

- PSX `sub_80014D58` first checks `word_80091810`; when it is zero, it jumps
  straight to `LABEL_71` with `v2=-1` and still writes
  `word_80091816 += -1 + word_80091822`. This path does not read
  `*(a1+64)` / descriptor row.
- Win direct previously returned early on `!row.valid` before the zero-accepted
  branch, which could suppress the PSX writeback when descriptor row facts were
  not available. `PrStage1ScorerDirectCommit14D58` now handles
  `word91810AcceptedCount == 0` before the row-valid guard.
- The bucket30 owner handoff now also exposes the direct commit package
  (`bucket30Gameplay*` / `g*` aliases): current branch, accepted count,
  lookback count, overflow, branch count, pair bonus, spill penalty, additive,
  commit term, score writeback and clamp. These are diagnostics for the direct
  commit path, not lifecycle/renderer authority.

Runtime coverage note:

- `--stage1-rail-full-song-runtime` now writes explicit row/writeback coverage
  fields: `coverage_query_frame=4255`, `max_query`,
  `reached_query_frame`, `last_row_write_max_query`, and per-subset reached
  flags for armed, positive-delta, `v22=1`, and row-write samples.
- Artifact
  `artifacts/stage1_rail_probe_runtime/20260515_192227_stage1_rail_full_song_runtime/summary.txt`
  did not cover the target tail: `stop_reason=unexpected_fail_gate`,
  `max_query_frame=1359`, `rowWritebackReachedQueryFrame=False`, and
  `rowWritebackLastRowWriteMaxQuery=832`. It is therefore only a harness/input
  coverage result, not evidence that the `4255` tail scorer path is still
  wrong.

Runtime coverage follow-up:

- Artifact
  `artifacts/stage1_rail_probe_runtime/20260515_193131_stage1_rail_full_song_runtime/summary.txt`
  reached past the PSX target frame: `max_query_frame=4806`,
  `rowWritebackCoverageQueryFrame=4255`,
  `rowWritebackReachedQueryFrame=True`, and `liveFailGate=False`.
- This run changes the active evidence window. The blocker is no longer
  "harness stopped before 4255"; it is that the Win direct runtime still never
  produces the PSX 4255 intermediate conditions:
  `rowWritebackPositiveDeltaSamples=0`,
  `rowWritebackV22OneSamples=0`,
  `rowWritebackLastRowWriteMaxQuery=701`, and the final row/writeback sample
  remains `rankRow=3`, `resolutionV22=2`,
  `bucket30OwnerDescriptorFlagWord=0`, `descriptorED08=0`.
- Next evidence window: compare the Win q3905..4806 handoff/history fields
  against the PSX `3995/4255` windows for descriptor flag/page source,
  `14D58` growth, `14D28` phase sample, resolver gate, and row-write. Do not
  patch lifecycle/renderer/no-fail/terminal gates to force row1.

Runtime timecode-source follow-up:

- Recorder facts from
  `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`:
  - frame `3995`: `ctx+0x0C=4947`, `ctx+0x38=13`, `ED08=0x0E`
  - frame `4255`: `ctx+0x0C=5712`, `ctx+0x38=15`,
    `ctx+0x40/0x44=0x801CD4F4`, `ctx+0x50=0`, `ED08=0x0E`, row `1`
  - `dword_800943C4=0x801CD38C`, `dword_800943C8=0x42`
- This explains the Win artifact mismatch: the old Win run used lifecycle
  ticks around `23497/25046` at q3995/q4259, so descriptor cadence advanced to
  rows `62/65` instead of the PSX rows `13/15`. The table itself has the PSX
  flags; the dispatcher was reading it with the wrong tick source.
- Code follow-up: `RunStage1DirectFrame7A60(...)` now builds `directTiming`
  after `AdvanceStage1RunnerTimecode801C7560(...)` and uses
  `runnerTimecode801C7560.state.tick801C364C` for the downstream direct
  `24FD0` dispatcher, descriptor/page mirrors, `801C9094`, and `14614`.
  This matches the PSX `801C7A60` order and does not synthesize ED08 or row
  success.

Runtime bucket-source follow-up:

- Static follow-up found one remaining part of the same bug: `80024FD0` bucket
  selection was still computed before `801C7560`, using the host runner tick in
  `UpdateStage1NumericRuntimeState`, then passed into
  `RunStage1ScorerDispatcher24FD0`. That could still trigger or skip bucket0 /
  bucket30 owner windows on the wrong cadence even after the downstream
  `tick96` argument was fixed.
- Code follow-up: `RunStage1DirectFrame7A60(...)` now computes
  `directCurrentBucket` after `AdvanceStage1RunnerTimecode801C7560(...)` and
  after `directTiming.tick96` has been replaced with
  `runnerTimecode801C7560.state.tick801C364C`. The dispatcher now uses this
  direct bucket, and first activation no longer seeds `bucketCadence` from the
  raw host bucket.
- This still leaves runtime validation as the authority: the next full-song
  run must show the real `ED08/91816/91818/v22/row` chain. Do not infer success
  from row targets, terminal/lifecycle state, renderer state, or no-fail.

Runtime descriptor/timecode observer follow-up:

- Full-song runtime after the last build:
  `artifacts/stage1_rail_probe_runtime/20260515_201131_stage1_rail_full_song_runtime/summary.txt`.
  It reached `max_query_frame=4806` with `liveFailGate=False`, but still had
  `rowWritebackPositiveDeltaSamples=0` and `rowWritebackV22OneSamples=0`.
- The new descriptor observer showed Win still far ahead of PSX cadence:
  q3905 had `page56=60`, `currentDescriptor40Index=60`, `rr08=6`; q4806 had
  `page56=74`, `currentDescriptor40Index=65`, `rr08=0`. PSX recorder truth for
  the same row/writeback windows is frame 3995 `ctx+0x0C=4947`, `ctx+0x38=13`,
  `ED08=0x0E`; frame 4255 `ctx+0x0C=5712`, `ctx+0x38=15`, `ED08=0x0E`.
- The remaining gap is now `801C7560` timecode read-value source/rate. New
  narrow evidence is in
  `docs/stage1_decomp/stage1_timecode_801c7560_source_20260515.md`.
  Recorder facts confirm `0x801C364C` same-frame writer `0x801C7634` with
  values `3905:4681`, `3995:4947`, `4255:5712`, `4806:7333`; `ctx+0x38` and
  `ED08` are written by earlier `80024FD0` bucket-boundary PCs
  `0x80025108` and `0x80025194`.
- `src/main.cpp` and `test_dispatcher.py` now expose/parse existing
  `RunnerTimecode801C7560Runtime` fields as `tcBase/tcXa/tcFallback/tc348/
  tc352/tc356/tcRound/tcCbRet`, and summary prints
  `full_song.rowWritebackLastTimecode=...`. This is observer-only and does not
  change row/writeback behavior.
