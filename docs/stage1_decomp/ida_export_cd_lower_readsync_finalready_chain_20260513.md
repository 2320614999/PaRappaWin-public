# CD Lower ReadSync / FinalReady Call Chain

Scope: `BuildReadSyncSeam800390C8`, `BuildLowerCdSeamFromFacts`, `BuildReadyWaitResult800372F0`, `BuildCdSyncLoopFacts80037070`.

Existing export files:
- `docs/stage1_decomp/ida_export_cd_lower_read_callbacks_80038bc4_80038fc0_800390c8_800372f0_20260512.txt`
- `docs/stage1_decomp/ida_export_cd_lower_producer_followup_8001a818_80038de8_800364f0_20260513.txt`

## Call Chain / Xref Table

| Stage | Function | Current caller / callee / xref | Boundary note |
| --- | --- | --- | --- |
| Host request | `TryPumpBootstrap15590CdLowerExternalPlan` | `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp:201` -> `BuildBootstrap15590CdLowerProducerRequest801C81EC` at `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2167`, then `RunBootstrap15590CdLowerFacts801C81EC` at `src/pr/pr_stage1_lifecycle_executor_direct.cpp:2690` | Request assembly only. No CD authority here. |
| Producer request | `BuildBootstrap15590CdLowerProducerRequest801C81EC` | executor direct request builder; consumed by host adapter at `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp:213,268` | Typed request carrier only. |
| Facts dispatcher | `BuildLowerCdSeamFromFacts` | `src/pr/pr_stage1_lower_cd_producer_direct.cpp:1527` dispatches action kind into `BuildCdSyncLoopFacts80037070`, `BuildReadyWaitResult800372F0`, or `BuildReadSyncSeam800390C8` | Central dispatcher for the direct translation chain. |
| Seek facts | `BuildCdSyncLoopFacts80037070` | `src/pr/pr_stage1_lower_cd_producer_direct.cpp:546` called only from SeekSync branch at `:1547` | Facts gate is `cdSyncLoopFacts80037070Known` and `a0WaitMode == 0`. |
| Ready facts | `BuildReadyWaitResult800372F0` | `src/pr/pr_stage1_lower_cd_producer_direct.cpp:930` called from ReadSync branch at `:1609` | Result builder only. It is not a request authority source. |
| ReadSync seam | `BuildReadSyncSeam800390C8` | `src/pr/pr_stage1_lower_cd_producer_direct.cpp:1451` called from `BuildLowerCdSeamFromFacts` at `:1611` | Facts gate is `finalReadyInput800372F0Known`; retry pump and final-ready result are both required. |
| Host apply | `ApplyBootstrap15590CdLowerFacts801C81EC` | `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp:268` -> `RunBootstrap15590CdLowerFacts801C81EC` | Final bridge from host adapter to executor facts path. |

## Minimal Call Chain

`TryPumpBootstrap15590CdLowerExternalPlan`
-> `BuildBootstrap15590CdLowerProducerRequest801C81EC`
-> `RunBootstrap15590CdLowerFacts801C81EC`
-> `BuildLowerCdSeamFromFacts`
-> `BuildCdSyncLoopFacts80037070` / `BuildReadyWaitResult800372F0`
-> `BuildReadSyncSeam800390C8`

## Boundary Notes

- `finalReadyInput800372F0Known` is the hard gate for ReadSync final-ready facts.
- `BuildReadyWaitResult800372F0` is a result constructor only.
- `BuildReadSyncSeam800390C8` is the seam boundary; it must not be widened to raw side channels.

