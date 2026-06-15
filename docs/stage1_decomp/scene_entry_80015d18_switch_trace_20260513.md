# `80015D18` switch-trace executor cutover

## Evidence

- Current-session IDA export:
  `docs/stage1_decomp/ida_export_80015d18_request_trace_current_20260513.txt`.
- Headless SCUS export attempts timed out at 120s and 90s without a file; the
  landed artifact was generated from the active IDA session with lightweight
  xref/disassembly/table extraction and no Hex-Rays pass.
- Existing Hex-Rays source:
  `docs/stage1_decomp/ida_export_stageclear_upstream_closure_20260511.txt`.

## `80015D18` call order

| PSX callsite | Request / call | Current direct boundary |
| --- | --- | --- |
| `80015D9C` | `8001EF14` | `ExecuteMainSceneHostTrace80015D18(...)` calls `PrTransition::ResetHoldOverlayState1EF14()` |
| `80015DA8` | `80015788(v1)` | `ExecuteMainSceneSwitchTrace80015D18(...)` now consumes the `Call80015788` trace request and calls existing `RunGenericSwitch80015788(...)` |
| `80015DC0`, `80015E5C` | `80025A34` | scene-callback executor applies `80025A34 -> 80025A00` loader-memory reset |
| `80015E10` | `8001A324(row0)` | scene-callback executor calls `PsxCall8001A324_InitSegmentRecord(...)` |
| `80015E54` | `800154B0(row0,0)` | scene-callback executor calls `PsxCall800154B0_Row0OverlayTransfer(...)`; lower-CD feedback remains typed facts only |
| `80015E90/80015EFC/80015F30` | callback table `fn0/fn1/fn2` | executor checks PSX function targets against `PrSceneDef::psxFn0/1/2`; `fn0/fn1` are gated by executor-owned `MainSceneInitGateState80015D18` |
| `80015F94` | `80015CC4` | `ExecuteMainSceneHostTrace80015D18(...)` forwards to `PrStage1SaveUiDirect::Sub80015CC4()` |
| `80015FD4` | `8001E34C` | `ExecuteMainSceneHostTrace80015D18(...)` records `8001E34C -> 8001E33C(80080CF8,80083FC0)` draw-buffer globals |

## Cutover

`pr_main.cpp` no longer calls `RunGenericSwitch80015788(...)` directly for the
scene-switch branch. It calls
`PrSceneEntryExecutorDirect::ExecuteMainSceneSwitchTrace80015D18(...)` with the
`BeginMainSceneLoopIteration80015D18(...)` trace, so the `80015788` execution is
now routed through the same scene-entry executor that owns the rest of the
`80015D18` request trace.

The new executor wrapper still uses the existing `RunGenericSwitch80015788(...)`
implementation. It only adds request validation and preserves pending
`80015788` host-dispatcher progress when a later frame has no fresh
`Call80015788` request in the trace.

`pr_main.cpp` also no longer owns the scene callback init latch directly.
`MainSceneInitGateState80015D18` now lives in the executor boundary; callers
reset it on startup/scene switch and call
`ExecuteMainSceneInitCallbacksOnce80015D18(...)`. This keeps the Win timing
gate out of `PrSceneEntryDirect::MainSceneState80015D18`, which remains a PSX
state mirror.

## Still Open

- Do not delete the executor init gate: it still preserves the `fn0/fn1`
  once-per-scene timing while `fn2` remains per-frame.
- Do not widen status-bank host bridge to `80015CC4` or `800169E0`; those remain
  SaveUi/payload-bank wrapper boundaries.
- Do not synthesize lower-CD success for `800154B0 -> 8001ACF8`; the row0 seam
  still needs real handled `CdSeamResult` facts.
