# Stage1 fail terminal RenderFrame(7) evidence

Generated from local IDA MCP against `S1/COMOD1.BIN.i64` on 2026-05-25.

## Export map

| Requested | IDA function | File | Notes |
| --- | --- | --- | --- |
| `0x801C74A0` | `0x801C73D0` | `decompile_801c74a0.txt` | `PrStrPlayer_Poll`; not the Stage runner render body. |
| `0x801C74E4` | `0x801C73D0` | `decompile_801c74e4.txt` | Same function as above. |
| `0x801C7560` | `0x801C7560` | `decompile_801c7560.txt` | `PrStageRunner_UpdateTimecode`. |
| `0x801C7638` | `0x801C7560` | `decompile_801c7638.txt` | Same function as above. |
| `0x801C7A60` | `0x801C7A60` | `decompile_801c7a60.txt` | Full Stage1 runner loop and fail cleanup tail. |
| `0x801CBFDC` | `0x801CBFDC` | `decompile_801cbfdc.txt` | `PrStageRunner_RenderFrame`. |
| `0x801CB190` | `0x801CB190` | `decompile_801cb190.txt` | `PrStageRunner_RenderUI`. |

## PSX cleanup tail facts

`801C7A60` `LABEL_68` clears three ctx words before the four cleanup render
passes:

```c
*((_WORD *)a2 + 50) = 0; // ctx+0x64
*((_WORD *)a2 + 42) = 0; // ctx+0x54
*((_WORD *)a2 + 61) = 0; // ctx+0x7A
do {
  PrStageRunner_RenderFrame(a2, 7);
  MEMORY[0x80035560](2);
  PrStageRunner_PresentFrame();
  --v11;
} while (v11 > 0);
```

The exported `801CBFDC` confirms cleanup still calls the normal render body
with mode `7`, not a blank/no-render path. The important cleared fields are
consumed later by the same render body:

- `ctx+0x54` gates `80023E10` script-box/text call:
  `if (*((_WORD *)a1 + 42) && *((_WORD *)a1 + 52)) MEMORY[0x80023E10](a1[67]);`
- `ctx+0x64` gates `8001E2E4` numeric/status output:
  `if (*((_WORD *)a1 + 50)) MEMORY[0x8001E2E4](a1, *((__int16 *)a1 + 52));`
- `ctx+0x7A` is the compact rail painter gate used by `80024744`; existing
  static evidence has `80024744` reading `ctx+0x7A`, and Win direct names it
  `compactPainterGate7A`.

`801CBFDC` then calls:

```c
MEMORY[0x80024744](a1);
return PrStageRunner_RenderUI();
```

So the PSX behavior is not "skip Stage1 render" and not "draw base-only"; it is
"run RenderFrame(7) with ctx+0x64/0x54/0x7A already zeroed".

## Current Win gap to audit

The direct helper already produces cleanup facts:

- `PrStageRunnerDirectResolveMainLoopTail7A60(...)` sets
  `clearWord50`, `clearWord42`, `clearWord61`, and `cleanupDrainFrameCount=4`.
- These flags are copied into
  `Stage1NumericRuntimeState::RunnerMainLoopTail7A60Runtime`.

But the exported cleanup clear does not appear to be carried into the render
snapshot consumed by `BuildStage1PsxRenderCtx801CBFDC(...)`:

- `tailGate54` is built from `runtime.steadyCtx801C7A60.word54_801C3694` or
  init fallback.
- `tailGate64` exists in `PsxPrStageRunnerCtx801CBFDC`, but current builder
  does not populate it from a `ctx+0x64` carrier.
- `compactPainterGate7A` is copied from
  `runtimeSlots.sceneSubmit.compactRail80024744.painterGate7A`, which is
  ultimately the active compact rail snapshot, not the cleanup-cleared
  `ctx+0x7A=0` fact.

This matches the latest user observation after `665b4fa4`: black flash is gone,
but during fail branch the failure animation/gameplay continues and the rail HUD
continues. A likely concrete mismatch is that the Win cleanup drain skips
`runner.Update(ctx)`, but still draws the old `801CBFDC` snapshot where
`ctx+0x7A` remains nonzero.

## Constraints for the next patch

Do not replace the cleanup render route with a base-only draw path. PSX still
calls `RenderFrame(7)` and then `RenderUI`.

The safe direction, if confirmed, is to carry the already-known direct tail
clear facts into the same render ctx used by `801CBFDC` during the four cleanup
passes:

- `clearWord42` -> force `ctx+0x54` / script-box gate to zero.
- `clearWord50` -> force `ctx+0x64` / numeric-status gate to zero, if/when that
  carrier is wired.
- `clearWord61` -> force `ctx+0x7A` / compact rail painter gate to zero.

The patch should stay in direct-port state/carrying code. It should not add
renderer/UI fallback suppression, and should not freeze runner/XA/audio by
visual guess.
