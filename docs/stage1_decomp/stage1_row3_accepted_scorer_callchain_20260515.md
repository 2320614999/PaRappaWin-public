# Stage1 row3 accepted/scorer callchain evidence - 2026-05-15

Scope: narrow evidence window for the `--stage1-rail-full-song-runtime`
`rightRankActiveRow=3 -> stream4 -> failGate` result. This does not reopen
lower-CD, case17, Rail/GTE, SaveUi19148, renderer, or formal lifecycle
fail-gate semantics.

## Static chain

The direct-port producer chain is:

```text
heldMask / debugPadInput
  -> 801C7A60 writes ctx+0x18 and ctx+0x20
  -> 80024FD0 bucket dispatcher
  -> 801C9094 frame update
  -> gated 80014614 accepted producer
  -> next scorer bucket: 80014D58, 80014D28, 80014548
  -> 80024FD0 v22 / committed row write
  -> ctx+0x4E rightRankActiveRow
```

Important ordering point: `80014614 -> 80014D58` is a data dependency into a
later scorer bucket/window, not a same-frame direct call after `80014614`.

## PSX / Win mapping table

| Node | PSX evidence | Win direct-port evidence | Current note |
|---|---|---|---|
| `801C7A60` frame order | `docs/stage1_decomp/801C7A60_sub_801C7A60.c`: writes control fields, calls `80024FD0`, calls `801C9094`, then gates `80014614` | `src/pr/pr_stage1_scorer_host.inl`: Stage1 frame order comment and implementation keep `24FD0 -> 801C9094 -> 14614` | Order is aligned; do not patch formal fail-gate. |
| `ctx+0x18` | current-frame control mask | `rawControlSample18` / `controlMask18` carriers | Next gap is whether Win local held de-dup changes accepted growth. |
| `ctx+0x20` | `80024B54(ctx+0x18)` priority class token | direct classifier path stores `classToken20` | Class token is not a one-hot success validator. |
| `80014614` | accepted producer updates `word_80091810/12/14` and `dword_80091808` when all event/descriptor/timing gates pass | `PrStage1ScorerDirectAcceptedProducer14614` mirrors record/penalty split and accepted count | This is the first dynamic authority to compare for full-song row3. |
| `80014D58` | consumes accepted counters plus descriptor/page data, adds `word_80091822`, updates `word_80091816` | `PrStage1ScorerDirectCommit14D58` / writeback resolver | `word_80091812` is a record-write shadow count, not the direct formula input. |
| `80014D28` | updates `word_80091818`, returns positive growth | `PrStage1ScorerDirectPositiveGrowth14D28` | Non-COOL row phase1 cache uses this positive-growth result. |
| `80014548` | row3 tie-break/delta helper around `word_8009181C/1E` | `PrStage1ScorerDirectTieBreak14548` | Row3 can leave AWFUL only when the relevant delta path returns the committing value. |
| `80024FD0` | bucket30 resolves `v22` and commits `ctx+0x4E` only on writeback path | `RunBucket30RowWriteSlice` then host publish to `rightRankActiveRow` | No renderer or formal lifecycle authority here. |

## Recorder facts from existing psxrec

Recording:
`E:\game\PSgame\parappa the rapper\duckstation\stage1二周目_无cool_good评级通关3.psxrec`

Metadata:

| Field | Value |
|---|---|
| Format | PSXREC03 / v3 |
| Frames | 9176 |
| Write log | enabled |
| PC trace | enabled |
| Call trace | enabled |
| Game | PaRappa the Rapper / SCUS-94183 |
| Recorded | 2026-03-27T23:50:22 to 2026-03-27T23:52:57 |

Observed score/accepted writers:

| Frame | Address | PC | Before | After | Meaning |
|---:|---|---|---:|---:|---|
| 1287 | `0x80091816` | `0x8001528C` | `0` | `3` | `80014D58` writeback to score-like accumulator |
| 1287 | `0x80091810` | `0x80014C2C` | `1` | `0` | accepted counter reset/maintenance after scorer consumption |
| 1287 | `0x80091818` | `0x80014D44` | `0` | `3` | `80014D28` baseline update |
| 1287 | `0x8009181C` | `0x800144B0` | `0` | `3` | shared baseline snapshot update |

Observed `ctx+0x4E` (`0x801C368E`) row writes:

| Frame | PC | Before | After | Inferred `v22` branch |
|---:|---|---:|---:|---|
| 2853 | `0x800256A4` | `1` | `2` | old row `1`, `v22=2` |
| 3115 | `0x800256E4` | `2` | `3` | old row `2`, `v22=0` |
| 3375 | `0x80025704` | `3` | `2` | old row `3`, `v22=1` |
| 5203 | `0x800256D4` | `2` | `1` | old row `2`, `v22=1` |

Full-range `writers 0x801C368E` timed out, but narrow windows around the
`changes` hits resolved the writer PCs above.

Narrow `f-2..f+2` PSXREC03 follow-up confirms each row-write frame has the
same same-frame call pattern:

```text
801C7F48 -> 80024FD0
80025320 -> 80014D58
800253A4 -> 80014D28
80025544 -> 80024F8C
800255B4 -> 80026EF8
```

The corresponding write facts are:

| Frame | Pre-row | Post-row | `80091816` | `80091818` | `8008ED38` | `8008ED36` | `8009181C` | Row writer |
|---:|---:|---:|---|---|---|---|---|---|
| 2853 | 1 | 2 | `8001528C:40->39` | `80014D44:40->39` | `8002549C:1->0` | `80024FA0:0->2` | `800144B0:41->39` | `800256A4:1->2` |
| 3115 | 2 | 3 | `8001528C:38->37` | `80014D44:38->37` | `8002549C:1->0` | `80024FA0:0->2` | `800144B0:39->37` | `800256E4:2->3` |
| 3375 | 3 | 2 | `8001528C:45->49` | `80014D44:45->49` | `8002549C:1->0` | `80024FA0:1->2` | `800144B0:37->49` | `80025704:3->2` |
| 5203 | 2 | 1 | `8001528C:96->102` | `80014D44:96->102` | `8002549C:1->0` | `80024FA0:1->2` | `800144B0:54->102` | `800256D4:2->1` |

`v22` is not a memory field, so it is inferred from the row writer PC plus
old/new `ctx+0x4E`, then interpreted through `80024FD0:800254E0..80025730`.
This evidence closes the PSX side of the bucket30 row-write window; code should
only change if a Win direct mapping diff is observed.

`ctx+0x18` / `ctx+0x20` at `0x801C3658` / `0x801C3660` showed no end-of-frame
change in frames 1260..1300, and frame 1287 reads both as `0`. This only says
the stored end-of-frame values are zero in that window; it does not disprove
transient same-frame control samples. Use `wlog` / live GDB if the exact
writer instant is needed.

## Current smallest gap

The direct-port check found a Win-local held de-duplication layer that had no
matching state in the PSX `801C7A60 -> 80014614` path:

```text
localAcceptedProducerConsumedHoldMask
  vs
801C7A60 every-frame 80035510(1) -> ctx+0x18/0x20 -> 80014614
```

The Win-local de-dup gate has been removed from
`src/pr/pr_stage1_scorer_host.inl`, and the unused state field was removed from
`src/pr/pr_stage1_scorer_host.h`. `80014614` now observes the same
`writerControlSample18` / `rawAcceptedMask` that the direct `801C7A60` carrier
produced, without an extra local physical held-consumed state between the PSX
writer and accepted producer tail.

Debug input boundary note: `pad` / `padseq` from the debug server are queued
single-frame samples, not a held-level source. `padseq` also appends a zero
after each non-zero mask. Therefore the old local held-consumed filter mainly
affected physical `PrPad::GetState(0).held` input. The full-song runtime harness
now drives `_drive_stage1_runtime_input_mode(..., input_mode='stage1_chart')`
inside its sampling loop after the no-fail guard succeeds, so the next valid
full-song run can be used to observe accepted growth and row writeback. If the
no-fail guard fails, the harness still stops as invalid and must not be treated
as runtime evidence.

Narrow follow-up addresses:

| Purpose | Address / field |
|---|---|
| Stage context tick | `0x801C364C` (`ctx+0x0C`) |
| Control sample | `0x801C3658` (`ctx+0x18`) |
| Class token | `0x801C3660` (`ctx+0x20`) |
| Right-rank row | `0x801C368E` (`ctx+0x4E`) |
| Accepted counters | `0x80091810/12/14`, `0x80091808` |
| Accumulator/baselines | `0x80091816/18/1C/1E/20/22` |
| Calls | `0x801C7F48`, `0x801C7F50`, `0x801C7FB8` |
| Row resolution | `0x8002550C..0x800256E0`, observed writers through `0x80025704` |

No conclusion in this file authorizes success from renderer state, formal
fail-gate edits, no-fail harness flags, payload bytes, Win files, ISO/STR/XA,
or default values.

## 2026-05-15 Win runtime observable follow-up

`stage1ovl handoff` / `stage1ovl handoffhistory` now expose the accepted/scorer
state needed to observe this chain from the live Win runtime:

| Field group | Source |
|---|---|
| Accepted candidate / materialization | `acceptedProducerBoundaryProbe` |
| Control/class/timing gate | `acceptedProducerCarrier` |
| Accepted counters and masks | `acceptedProducer` and `scorerPort` |
| `80091808/10/12/14/24/0C` mirrors | `scorerPort` |
| row writeback / bucket cadence | `rightRankState`, last-row-write fields, `bucketCadence` |
| narrow clear / follow-up phase | `acceptedProducerNarrowClear*`, `rightRankFollowUpPhase` |
| descriptor lookahead for source-gate probing | `descriptorCadence.currentCommitted*`, `descriptorCadence.nextLookahead*` |

The current `test_dispatcher.py --stage1-accepted-runtime` no longer depends on
a C++ ring history being present: the Python harness accumulates live
`stage1ovl handoff` samples during the 40s window and appends the current
`handoffhistory` tail sample. This is a verification-surface fix only; it does
not add runtime authority.

Latest runtime result after the observable fix:

```text
acceptedContributionCount=3
recordedSplit=2
penaltySplit=1
aggregateAcceptedMask=224
rankRow=3
bucket30=1
```

The remaining runtime coverage gap is now specific: the `stage1_chart` input
path did not observe a `raw accepted candidate rejected before
template2/selector gate`. Do not treat that as a scorer failure by itself; it is
a missing reject-window sample. Next evidence should identify a PSX
query/mask/window for that reject path, or split the accepted growth and reject
coverage into separate tests.

## 2026-05-15 PSX recorder reject-window facts

Recorder source:
`E:\game\PSgame\parappa the rapper\duckstation\stage1二周目_无cool_good评级通关3.psxrec`.

Narrow query: scan only frames whose `call_trace` contains target `0x80014614`,
then sample the stage1 accepted-producer context at `0x801C3640` and accepted
global writes in the same frame.

Result summary:

```text
call_frames=98
materialized=84
selector0-zero=14
```

The 14 reject-window frames all have `raw_mask = ctx+0x18 & 0x09FF != 0`,
`ctx.flags & 1 != 0`, and no write to `80091808/80091810/80091812/80091814`.
They reject before source/template materialization because the descriptor row
has `selectorByte0=0` and `selectorByte1=0`; this is a real PSX
selector-before-template reject window, not a renderer/no-fail/lower-CD issue.

| frame | ctx10 | ctx18/raw | class | desc | selector0/1 | slot48 | accepted writes | reason |
|---:|---:|---:|---:|---|---|---:|---|---|
| 489 | 1063 | `0060/0060` | 2 | `801CD3D4+6*0` | `0/0` | 6 | none | selector0-zero |
| 499 | 1091 | `00E0/00E0` | 2 | `801CD3D4+6*0` | `0/0` | 6 | none | selector0-zero |
| 507 | 1114 | `0080/0080` | 4 | `801CD3D4+6*0` | `0/0` | 7 | none | selector0-zero |
| 529 | 1180 | `0010/0010` | 1 | `801CD3EC+6*0` | `0/0` | 0 | none | selector0-zero |
| 535 | 1197 | `0030/0030` | 1 | `801CD3EC+6*0` | `0/0` | 1 | none | selector0-zero |
| 539 | 1208 | `00B0/00B0` | 1 | `801CD3EC+6*0` | `0/0` | 1 | none | selector0-zero |
| 549 | 1239 | `00A0/00A0` | 2 | `801CD3EC+6*0` | `0/0` | 1 | none | selector0-zero |
| 551 | 1244 | `0020/0020` | 2 | `801CD3EC+6*0` | `0/0` | 2 | none | selector0-zero |
| 609 | 1415 | `0008/0008` | 7 | `801CD3EC+6*0` | `0/0` | 5 | none | selector0-zero |
| 613 | 1427 | `000C/000C` | 5 | `801CD3EC+6*0` | `0/0` | 5 | none | selector0-zero |
| 617 | 1438 | `0004/0004` | 5 | `801CD3EC+6*0` | `0/0` | 6 | none | selector0-zero |
| 623 | 1457 | `0008/0008` | 7 | `801CD3EC+6*0` | `0/0` | 6 | none | selector0-zero |
| 627 | 1469 | `000C/000C` | 5 | `801CD3EC+6*0` | `0/0` | 6 | none | selector0-zero |
| 629 | 1473 | `0004/0004` | 5 | `801CD3EC+6*0` | `0/0` | 6 | none | selector0-zero |

Materialized control rows from the same scan prove the positive side of the
same `80014614` path:

| frame | ctx10 | ctx18/raw | class | selector0/1 | slot48 | template | accepted write PCs |
|---:|---:|---:|---:|---|---:|---:|---|
| 1111 | 2893 | `0010/0010` | 1 | `1/6` | 4 | 2 | `800147F0`, `80014A14`, `80014A34`, `80014A40` |
| 1239 | 3269 | `0020/0020` | 2 | `2/6` | 4 | 2 | `800147F0`, `800149F4`, `80014A34`, `80014A40` |
| 1367 | 3647 | `0040/0040` | 3 | `3/6` | 4 | 2 | `800147F0`, `800149F4`, `80014A34`, `80014A40` |
| 1495 | 4022 | `0080/0080` | 4 | `4/6` | 3 | 2 | `800147F0`, `80014A14`, `80014A34`, `80014A40` |

This recorder fact closes the missing PSX reject-window evidence for the
accepted-runtime harness. It does not authorize any synthetic Win success; the
Win runtime still has to either drive a matching reject-window sample or split
accepted growth and selector/template reject into separate checks.

## 2026-05-15 Verification split

The verification has now been split so the Win runtime chart window no longer
has to reproduce the PSX selector0-zero reject window in the same run that
checks accepted growth:

| Test | Scope |
|---|---|
| `python .\test_dispatcher.py --stage1-accepted-runtime` | live Win accepted/materialized growth, split, counter, and downstream consumer checks |
| `python .\test_dispatcher.py --stage1-accepted-selector-reject-recorder` | PSX recorder `80014614` selector-before-template reject frame set plus materialized split facts |
| `python .\test_dispatcher.py --stage1-accepted-split-recorder` | alias for the same recorder fact-set check |

`--stage1-accepted-selector-reject-recorder` loads the same PSXREC03 recording,
scans only `call_trace target=0x80014614`, and verifies the expected 14
selector0-zero frames plus materialized control rows and split facts:

```text
call_frames=98
selector0_zero_reject=14
materialized=84
frames=[489, 499, 507, 529, 535, 539, 549, 551, 609, 613, 617, 623, 627, 629]
split_frames={1111: penalty, 1239: recorded, 1367: recorded, 1495: penalty}
```

Materialized split facts verified by the recorder-only test:

| Frame | Split | Required side write | Shared writes |
|---:|---|---|---|
| 1111 | penalty | `80014A14 -> 80091814` | `80014A34 -> 80091810`, `80014A40 -> 80091808` |
| 1239 | recorded | `800149F4 -> 80091812` | `80014A34 -> 80091810`, `80014A40 -> 80091808` |
| 1367 | recorded | `800149F4 -> 80091812` | `80014A34 -> 80091810`, `80014A40 -> 80091808` |
| 1495 | penalty | `80014A14 -> 80091814` | `80014A34 -> 80091810`, `80014A40 -> 80091808` |

This is a test/harness split only. It does not add runtime authority and does
not derive success from Win files, renderer state, no-fail state, payload bytes,
ISO/STR/XA readability, or sampled payload values.

Follow-up split: `--stage1-accepted-runtime` no longer treats selector reject,
recorded-vs-penalty split coverage, source reject coverage, or bucket30
fallback consumer coverage as hard pass/fail gates for the accepted/materialized
growth test. Those are independent evidence windows. The main runtime test keeps
the hard checks on the accepted producer boundary itself: materialized samples
must carry a raw mask, accepted gate, selector, template state `2`,
source-cell gate, split id, and correct `80091808/10/12/14` updates for the
split that actually appears live; raw non-materialized samples must not change
accepted counters.

The live Win sampling window may miss the one-frame `observer.materialized`
sample while the downstream scorer counters have already committed the accepted
growth. To avoid treating that sampling gap as a runtime failure, the runtime
test also accepts a live aggregate-growth proof when all of these are true:
`acceptedContributionCount > 0`, the aggregate changed relative to the previous
handoff sample or it is the first committed accepted sample,
`recordedSplit + penaltySplit == acceptedContributionCount`,
`aggregateAcceptedMask != 0`, and `rankWritebackCommitted == 1`. This is still
a live runtime counter/row-write proof; it does not synthesize success from
renderer state, Win files, payload bytes, ISO/STR/XA readability, or recorder
sample values.

Latest run after this split:

```text
python .\test_dispatcher.py --stage1-accepted-runtime --stage1-runtime-retries 1
=> OK:
attempt=1
template_reject=recorder-selector0-zero[frames=14]
source_reject=split-out
recorded=recorder-split-fact
penalty=recorder-split-fact
accepted_growth=q=601 accepted=1 recorded=0 penalty=1 mask=0x0020
fallback=split-out
right_rank=downstream-missing
```

That closes the accepted-runtime harness as a split verification surface. If a
future live materialized sample appears, the stricter per-sample raw/gate/split/
counter checks still run. Further accepted/scorer work should only continue if a
new runtime diff appears in the actual direct chain, especially the existing
`80024FD0` bucket30 `v22 -> ctx+0x4E` row-write window
(`800253BC..80025704`).
