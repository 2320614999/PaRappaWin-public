# Stage1 full-song runtime input-mode bias - 2026-05-16

Scope: classify the latest full-song runtime artifact so it is not used as
PSX scorer authority. This does not authorize changing full-song harness input
semantics, scorer math, `v22`, row tables, renderer, marker, lifecycle, or
page-clear routing.

Runtime artifact:

`artifacts/stage1_rail_probe_runtime/20260516_224538_stage1_rail_full_song_runtime/samples.jsonl`

Latest checked artifact with the same boundary:

`artifacts/stage1_rail_probe_runtime/20260517_003901_stage1_rail_full_song_runtime/summary.txt`

Relevant driver code:

- `test_dispatcher.py:9275` sets
  `input_mode = 'stage1_chart_anchor_commit_keepalive_circle_circle_9100'`.
- `test_dispatcher.py:13172..13196` resolves that mode as:
  - `hold_mask = PAD_CIRCLE`
  - `keep_alive_pattern = (PAD_CIRCLE,)`
  - `keep_alive_pulse_ms = 35`
- `test_dispatcher.py:9360..9364` applies that input mode each runtime loop by
  calling `_drive_stage1_runtime_input_mode(...)`.

Current source-path recheck (2026-05-17):

- `test_dispatcher.py:9275` still hardcodes
  `stage1_chart_anchor_commit_keepalive_circle_circle_9100` for
  `run_stage1_rail_full_song_runtime_test`.
- `test_dispatcher.py:13172..13196` still resolves this as a Circle
  keep-alive pattern, and `_drive_stage1_runtime_input_mode(...)` injects that
  through debug pad commands.
- The real Stage1 runtime input path is separate:
  `GetStage1NumericRuntimeHeldMask(ctx)` reads
  `PrPad::GetState(0).held`, combines explicit `ctx.debugPadInput`, normalizes
  both at the `80035510` HAL seam, and then calls
  `PrPsxPadDirect::PsxReadPadMask80035510(...)`.
- Therefore this full-song artifact describes the test driver's chosen input
  schedule, not a direct-core scorer or replay authority.

Artifact scan:

```text
acceptedTickSeedControl18 values in 20260516_224538:
{ '32': 610 }
```

`32` is `0x20`, the returned/action mask for Circle/class token `2`.

The later `20260517_003901` summary remains in the same class: it reaches
row-writeback sampling but still ends with `liveClearTerminalTailGate=True`,
`liveClearTailPulse=False`, `rowWritebackPositiveDeltaSamples=0`, and
`rowWritebackV22OneSamples=0`. That result must not be used to patch
`80014614/80014D58/80024F8C`; the driver never supplies the PSX first
`0x10/class1` beat in this mode.

Nested sample recheck (2026-05-17):

```text
20260517_003901:
  initial: query 17, row 1
  write:   query 565, prev 1 -> row 2, v22 0, writeback24 0
  write:   query 696, prev 2 -> row 3, v22 0, writeback24 0
  v22_one samples: 0
  positive_delta samples: 0

20260516_224538:
  initial: query 17, row 1
  write:   query 565, prev 1 -> row 2, v22 0, writeback24 0
  write:   query 696, prev 2 -> row 3, v22 0, writeback24 0
  later positive-delta samples occur only after row is already 3.

20260516_203744:
  initial: query 18, row 1
  write:   query 565, prev 1 -> row 2, v22 0, writeback24 0
  write:   query 696, prev 2 -> row 3, v22 0, writeback24 0
  later positive-delta samples occur only after row is already 3.

older 20260516_015157:
  initial: query 17, row 1
  write:   query 570, prev 1 -> row 2, v22 0, writeback24 0
  write:   query 831, prev 2 -> row 1, v22 1, writeback24 4
  write:   query 962, prev 1 -> row 2, v22 0, writeback24 0
  write:   query 1223, prev 2 -> row 3, v22 0, writeback24 0
```

The latest artifacts therefore do not show a direct scorer rule that turns the
v3 GOOD path into row 3. They show a non-PSX driver schedule that first drops
from GOOD to row 2, then resolves row 2 to row 3 through the existing `v22=0`
path. That is an input/source artifact, not authority for changing row tables,
`v22`, coverage, slot14, marker, lifecycle, renderer, or clear-tail.

## Conclusion

The latest full-song runtime artifact is Circle-biased by its existing driver
mode. Its row3 page history therefore cannot be used as PSX authority for the
first-beat `0x10/1` history proven by recorder frame `3861`.

Do not patch `80014D58`, `80014D28`, `80024F8C`, `v22`, coverage, slot14,
renderer, marker, lifecycle, or page-clear routing from this artifact. The
next runtime validation for row3 must use an existing input path that can
drive the PSX first-beat `0x10` / second-beat `0x20` sequence, or else use
recorder/live facts only.
