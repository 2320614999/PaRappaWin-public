# Stage1 runtime accepted writer source/timing window - 2026-05-16

Scope: Stage1 main runtime row/writeback -> clear-tail blocker. This note only
uses the existing runtime artifact and PSX recorder facts to narrow the next
code target. It does not authorize renderer, marker, lifecycle clear,
F5/no-fail, stage status, row/v22 forcing, or full-song harness input patches.

Runtime artifact:

`artifacts/stage1_rail_probe_runtime/20260516_175450_stage1_rail_full_song_runtime/summary.txt`

Samples source:

`artifacts/stage1_rail_probe_runtime/20260516_175450_stage1_rail_full_song_runtime/samples.jsonl`

PSX recorder authority:

- `docs/stage1_decomp/stage1_row3_accepted_input_window_20260516.md`
- `docs/stage1_decomp/stage1_row3_page12_slot10_writer_20260516.md`

Additional recorder export:

```text
python .\tools\模拟器内存回放器.py "<recording>" export_windows 3948 3965 artifacts\psx_row3_ctx52_replay_window_3948_3965.csv --format csv --step 1 --window ctx0@0x801C3640:112 --window replayCounts@0x800901BC:16 --window replayTicks@0x8008EEF8:64 --window replayMasks@0x8008EEFC:64
```

Output:

`artifacts/psx_row3_ctx52_replay_window_3948_3965.csv`

## Runtime accepted writer samples

The existing runtime artifact shows the direct accepted writer obeying the
translated `80014614` split rule:

| query | runtime tick | accepted tick | write page | record slot | rem24 | recorded result |
|---:|---:|---:|---:|---:|---:|---|
| `1586` | `4749` | `4733` | `12` | `5` | `13` | recorded |
| `1616` | `4837` | `4821` | `12` | `9` | `5` | recorded |
| `1646` | `4924` | `4909` | `12` | `12` | `21` | not recorded |

At `query=1616`, the runtime state already has:

- `lastWriteAvailable=1`
- `lastWritePage38=12`
- `lastWriteSlot24=9`
- `lastWriteMask=32`
- `acceptedTick96Known=1`
- `acceptedTick96=4821`

At `query=1646`, the candidate accepted tick is `4909`, but
`(4909 + 8) % 384 = 309`, so `recordSlot24=12` and `recordRem24=21`.
With `halfWindow34=8`, PSX `80014614` only records when
`recordRem24 <= 16`; this candidate correctly stays penalty-side and does not
fill page12 slot10.

## PSX target contrast

The PSX recorder target has a fresh non-replay input at frame `3961`:

- `ctx+0x0C = 4846`
- `ctx+0x10 = 4846`
- `ctx+0x18 = 0x20`
- `ctx+0x20 = 2`
- `ctx+0x34 = 8`
- `ctx+0x38 = 13`
- `ctx+0x52 = 0`
- `dword_800901BC = 12`
- `dword_800901C0 = 12`
- page12 slot10 is written with `0x20/2/1`

Writer confirmation for the same frame:

| Frame | Address | PC | Before | After | Meaning |
|---:|---|---|---:|---:|---|
| `3953` | `0x801C3658` | `0x801C7F44` | `0x20` | `0` | non-replay no-input clear |
| `3961` | `0x801C3658` | `0x801C7F24` | `0` | `0x20` | `80035510(1)` live poll accepted input |
| `3961` | `0x8008EF54` | `0x80014954` | `0` | `0x20` | `80014614` replay log class mask slot 11 |
| `3961` | `0x8008EF50` | `0x80014974` | `0` | `4846` | `80014614` replay log tick slot 11 |
| `3961` | `0x800901C0` | `0x8001498C` | `11` | `12` | `80014614` replay write count |
| `3961` | `0x800901BC` | `0x80014994` | `11` | `12` | `80014614` replay published count |

For that target tick, `80014614` computes:

- `(4846 + 8) % 384 = 246`
- `recordSlot24 = 10`
- `recordRem24 = 6`
- `6 <= 16`, so it records page12 slot10.

## Conclusion

The next safe code target is not `80014614` page/slot/split, row transition,
`v22`, renderer, marker, lifecycle clear, or stage status. The translated
writer is rejecting the later `4909` candidate for the same reason PSX would.
The target frame also has `ctx+0x52=0`, so the gap is not the replay-mode
fallback branch that substitutes `0x0800` while waiting for a replay slot.
The target frame's replay log writes are the normal `80014614` append
side-effect after live input is accepted; the replay buffer is a consequence,
not the source authority for this frame.

The remaining gap is the runtime accepted input source/timing that should
deliver a fresh non-replay `0x20` input around `ctx+0x0C=4846`, so the direct
path can naturally create page12 slot10 before the row3 first-beat classifier.
Do not fix this by editing full-song harness input semantics or by injecting
synthetic row/writeback success.

## Runtime seam patch

IDA current-session decompile for `80035510` confirms the function body is the
pad HAL boundary:

```c
int sub_80035510()
{
  PAD_dr();
  return ~dword_800882F0;
}
```

`src/pr/pr_stage1_scorer_host.cpp` now routes Stage1 numeric runtime input
through the existing `PrPsxPadDirect` seam:

- follow-up recorder facts show physical `RIGHT=0x2000` edges at frames
  `3947/3960/3975/3992`, while live memory at the corresponding accepted
  poll window has `dword_800882F0=0xFFFFFFDF`, so `80035510` returns `0x20`
  and `ctx+0x18=0x20`;
- there are no physical `0x20` input edges in that replay window, so `0x20`
  is the `80035510` returned action mask, not a standard PSX raw held input;
- local `PrPad` held state keeps the existing Stage1 alias handling and feeds
  `PsxReadPadMask80035510` as returned action-mask facts;
- `ctx.debugPadInput` remains the debug/server host PSX pad mask for the rest
  of the runtime, so the Stage1 `80035510` seam normalizes it at the HAL
  boundary with
  `NormalizeDebugServerPsxPadMaskToReturnedMask80035510`: already-returned
  low/control bits are preserved, and the PSX high directional quartet
  `0x1000/0x2000/0x4000/0x8000` is folded to `0x10/0x20/0x40/0x80`;
- `PrPsxPadDirect` now models only `PAD_dr(); return ~dword_800882F0`, and
  the zero-use raw held-mask mapper was removed.

The `801C7A60` bit0/bit1 remap remains in
`PrStageRunnerDirectNormalizeInputMask7A60`; the input builder does not
duplicate that stage-local transform. This patch does not change full-song
harness input semantics and does not add observer/sentinel/debug-only fields.
The full disassembly at `801C7F74/801C7F80` also confirms the `80014614`
call remains gated by a `ctx+0x18 & 0x9FF` mask-edge check against
`dword_801CCBB8`; stale pseudocode that omits this edge gate is not the
authority for the runtime seam.

The Scene1 movie/text outer-loop host facts now use the same `80035510` HAL
boundary: local `PrPad` held state and explicit `ctx.debugPadInput` are
combined only after `ctx.debugPadInput` is normalized at the HAL boundary, then
passed through `PsxReadPadMask80035510`. The normalization preserves
`0x0100/0x0800` so the movie/text confirm/abort exits still see PSX control
masks.

## Follow-up integration contrast

After the `80035510` HAL seam was aligned, the existing
`stage1-rail-full-song-runtime` integration entry still stopped at
`unexpected_fail_gate` in:

`artifacts/stage1_rail_probe_runtime/20260516_185049_stage1_rail_full_song_runtime/summary.txt`

The useful signal is not the marker failure. The numeric runtime reached row3
and page12/page13 windows, but the scorer owner still had no positive
`91816-91818` delta:

- near `tick96=4849`: `acceptedCount=3`, `unionMask=0x20`,
  `requiredMask=0x10`, `commitTerm=-2`, `scoreWriteback=0`;
- near `tick96=4962`: still `acceptedCount=3`, so the row3 phase remains
  `phase1LatchArmed=1` and `v22=2`.

PSX replay truth from the same recording window is different:

| Frame | `ctx+0x0C` | `ctx+0x18` | `ctx+0x38` | `91810` | `91816` | `91818` | `8ED36` | `8ED38` |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `3961` | `4846` | `0x20` | `13` | `2` | `9` | `9` | `1` | `1` |
| `3993` | `4940` | `0x20` | `13` | `4` | `9` | `9` | `1` | `1` |
| `3995` | `4947` | `0x20` | `13` | `4` | `18` | `18` | `2` | `0` |

So frame `3961` is an accepted writer fact, not the positive score delta
fact. The row3 phase closes at PSX frame `3995`, after the fourth `0x20`
accepted input in the same page. The current Win integration artifact only has
three accepted samples in that window, so it cannot be used to accuse
`80014D58/80024FD0` scorer math yet. Do not patch scorer or marker from this
marker failure; the next actionable check is the real main-runtime input
source/timing that should supply the fourth accepted `0x20` in this window.

## Follow-up after returned-mask normalization

After `ctx.debugPadInput` was normalized from external PSX pad mask into the
`80035510` returned-mask boundary, the existing full-song integration entry was
rerun:

`artifacts/stage1_rail_probe_runtime/20260516_193809_stage1_rail_full_song_runtime/summary.txt`

Useful signal:

- the run no longer stopped at the earlier `unexpected_fail_gate`; it reached
  `max_query_frame=7211` and ended by timeout / stale rail marker
  expectations;
- no `liveFailGate` was observed;
- `liveClearTerminalTailGate=True`, but `liveClearGate=False` and
  `terminalValid=False`;
- the row3 window now naturally produces fourth/fifth `0x20` accepted entries:
  `acceptedTickSeedQuery=4752`, `accepted=4`, `count91810=4`, then
  `acceptedTickSeedQuery=4782`, `accepted=5`, `count91810=5`;
- the later page window still has accepted `0x20` samples
  (`4911/4950/4980`), but only reaches `accepted=3`;
- `rowWritebackV22OneSamples=0`, and `resolutionV22` remains `2` in the row3
  samples.

Conclusion: the input source/timing gap that motivated the returned-mask seam
is closed enough for main-runtime integration. The next gap is not another
`80035510`/pad source chase; it is the bucket30/scorer owner classifier path
that keeps publishing `v22=2` instead of the PSX frame-3995 `v22=1` authority.
