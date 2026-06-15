# 801C81EC lifecycle callsite table (2026-05-14)

Scope: Stage1 lifecycle direct-port evidence only. This file closes the current
`801C81EC` evidence window and prevents another broad lifecycle re-export.

## Authority

| Source | Result | Use |
|---|---|---|
| `docs/stage1_decomp/ida_export_801c81ec_lifecycle_cfg_callsite_20260514.md` | Current SCUS IDB sees `801C81EC` as `loc_801C81EC`; no function, no blocks, only lifecycle table xref `80048D3C -> 801C81EC`. | Table/callsite evidence only. Do not infer function body from this IDB. |
| `docs/stage1_decomp/ida_export_801c81ec_forced_range_callsite_20260514.md` | Forced `add_func(801C81EC,801C858C)` succeeds structurally, but pseudocode is junk and key call/data refs are empty. | Negative evidence: current SCUS bytes are not lifecycle body authority. |
| `docs/stage1_decomp/comod_status_coupled_pseudocode_20260511/S1_COMOD1/decompile_801c81ec.txt` | Overlay IDB decompiles `PrScene1_RunMovie1_AndStage1`. | Primary pseudocode authority for direct-port/control-flow work. |
| `docs/stage1_decomp/movie_segment_20260510/lifecycle_next_exports_20260510.md` | Confirms current-SCUS `loc_*` caveat and overlay bounds `801C81EC-801C858C`. | Boundary/source ownership note. |

## PSX call order

| Order | PSX action | Current C++ carrier | Boundary note |
|---|---|---|---|
| 1 | `PrStrPlayer_ConfigMovieViewport(); 80026FA4()` | `EmitEntryPrologue` in `src/pr/pr_stage1_lifecycle_direct.cpp` | Host adapter still treats viewport config as no-op shell; do not delete prologue action. |
| 2 | If `800916D0` is not `1/2`: `800201AC(...,6,1,2)`, `PrStrPlayer_Init(sceneEntry+0x6C,0)`, `PrStrPlayer_PlayAndWait(sceneEntry+0x6C,...,0)` | `EmitInitialMovie1Request`, `Step801C81EC` | Row2 `MOVIE1.STR` comes from materialized MOVSEG row, not Win STR metadata. |
| 3 | If initial movie returns `1`: `80026FA4`, `800201AC(...,5,1,2)`, `8001EF14`, write `800916E0=3`, return `-1` | `Step801C81EC` writes `word800916E0=3` and result `-1` | Needs `800916E0` writer/reader xref matrix before thinning exit-reason shell further. |
| 4 | Loop: `8001A4D0(sceneEntry+0x9C,0)`, `801C7A60(sceneEntry+0x9C,0x801C3640,sceneId)` | `EmitStageLoopRequest`, `StageRunnerRun7A60` action | Stage runner is a blocking PSX call; current host split is a carrier, not exact same call stack. |
| 5 | If `800916D0==1`: clear D0, copy `dword_801D3040` to DA, transition, reset hold tiles, return `0` | `Step801C81EC` restore branch | `D0/DA` remain PSX status-bank state, not host lifecycle state. |
| 6 | If `800916D0==2`: `80015744(&80092F10)`, clear D0, transition, reset, write `800916E0=3`, copy DA, return `-1` | `RestoreTransitionPayload15744` action plus status writes | Preserve seeded `80092F10` boundary. |
| 7 | If stage result is not `1`: call `80026B94(4,0)`; if result is `2`, transition/reset, write `800916E0=2`, return `-1`; else `8001EF14` and loop | `QueryAbort26B94`, `BuildAbortPollHostBlockStart801C81EC` | Dispatcher feedback is the authority for return value; start failure or host state must not synthesize `0/2`. |
| 8 | On stage clear: `80020110`, `800166AC(sceneId)`, choose row4/row5 by `801C368E`, init/play clear-tail STR mode `1/2` | `EmitClearTailPreludeRequest`, `EmitClearTailMovieRequest` | Row4/row5 `XMOVIE1.STR` comes from MOVSEG row identity. |
| 9 | After clear-tail STR: `80026FA4`, `800201AC(...,5,1,2)`, `8001EF14`, `80026EF8(80094410)`, `80026ECC()` | `EmitClearTailPostMovieAndStatus` | These remain ordered actions; do not fold into one host success. |
| 10 | If `!800916DA`: `8001635C(sceneId,status,prev,80091816)`, maybe `8001628C(sceneId+1)`, maybe `80015590(sceneId)` and `80019148(&80092F10)` when `800916F0 != 1` | `PrStageStatusBankDirectClearProducer`, `SaveStatus1635C`, `UnlockNextStage1628C`, `Bootstrap15590`, `SaveUi19148` actions | Status-bank request mapping is direct; bootstrap/save-ui still have host-block carrier gaps. |
| 11 | Final `8001EF14`; return `sceneId+1` or `0` depending `800916DA` and stage bound | `FinalizeClearTailResult` via status-bank producer return | Return comes from PSX clear producer, not movie/file success. |

## Current thin-shell targets

These can be revisited only after preserving the call-order boundaries above:

- `TryResolveStage1LifecycleActionRowPath(...)`: row-path host resolution shell.
- `BuildMovie1HostFeedback(...)`, `AdvanceMovie1HostStrPlayer(...)`,
  `ExecuteMovie1HostActions(...)`: Movie1 host I/O shell around direct STR
  action state.
- `TryPumpBootstrap15590CdLookupIsoHal(...)`,
  `TryPumpBootstrap15590CdLowerExternalPlan(...)`: HAL fact source shells for
  the `80015590` loader block.
- Gap logging in `ApplyAction(...)`: can be thinned only when corresponding
  direct requests/feedback remain observable.

Do not delete the direct semantics in `Phase801C81EC`, `Runtime801C81EC`,
`ActionKind801C81EC`, `Action801C81EC`, the `Emit*` functions, `Step801C81EC`,
or the executor host-block state. Those are the current direct-port carrier for
the PSX blocking calls.

## Remaining evidence windows

1. `word_800916E0` xref matrix: consolidate writers in `801C81EC` and readers in
   event case10 before changing exit-reason wiring.
2. `801C81EC -> 80015590 -> lower helpers`: narrow dependency graph only for
   the still-thick `StartStage1Bootstrap15590Block801C81EC` shell. Do not
   repeat lower-CD final-ready authority work.
3. `801C81EC -> 80019148(&80092F10)`: callsite seed/input boundary for the
   save-ui block; preserve the existing `80092F10` prefix authority and do not
   substitute empty host payloads.

No runtime/source code was changed for this evidence window.
