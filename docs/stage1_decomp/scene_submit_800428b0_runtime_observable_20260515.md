# Scene Submit `800428B0` runtime observable snapshot

Authority:

- Runtime command surface added in this workspace:
  `stage1ovl scene428b0`.
- Harness:
  `python .\test_dispatcher.py --stage1-scene428b0-runtime-snapshot --stage1-runtime-retries 1`.
- Runtime artifact:
  `artifacts/stage1_scene_submit_428b0_runtime/20260515_142524_stage1_scene428b0_runtime_snapshot/summary.txt`.

Observed runtime facts:

- The Stage1 draw path called the `800428B0` packet-command builder:
  `drawCalled=1`, `renderer=1`.
- The latest sampled frame exposed the packet command stream:
  `commands=1575`, `valid=1575`, `gaps=0`, `specialTable=0`.
- Last observed packet command:
  `handler=TNF4`, `primitiveCode=0x2D`, `primitiveFlag=0x01`,
  `flag4=0`, `rawKnown=1`, `rawIndex=20`, `rawOffset=708`,
  `rawSize=32`, `handlerSlot=54`, `handlerEntry=0x8008EEB0`,
  `cursorAdvanceHalfwords=16`.
- `lastSpecial <none>` in this runtime coverage.

Code boundary:

- `DrawStage1Tmd428B0(...)` now records a debug-only summary of the
  `PsxGpuPacketCommand428B0` stream that already feeds the backend.
- `stage1ovl scene428b0` only reports observable command/gap metadata. It does
  not alter rendering, handler resolution, GTE output, OT ordering, packet
  words, or primitive acceptance.
- The dispatcher harness only verifies that the runtime command stream is
  observable and stores samples; it does not treat a missing special-table hit
  as proof that `8008EED8/8008EEE4` are always unused.

Boundary / non-authority:

- This run did not hit the `0x20/0x30 + flag&4` special-table path.
- `specialTable=0` is only coverage for this run. It is not a writer/table-value
  proof for `8008EED8/8008EEE4`.
- Do not infer special handler targets from Win renderer output, TMD parser
  shape, static BSS zero, ISO/file readability, visual sampling, or default
  values.

Next authority requirement:

- If a future runtime sample reports `specialTable>0`, capture the matching
  `lastSpecial` line and then obtain true writer/live table facts for
  `8008EED8/8008EEE4`.
- Until then, the special-table path remains fail-closed:
  `handlerTargetKnown800428B0=false`, no render authorization.
