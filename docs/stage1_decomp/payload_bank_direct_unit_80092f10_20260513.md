# `80092F10` payload bank direct unit boundary

## Scope

This note records the first eight code cuts for the `80092F10` payload-bank unit.
The changes are intentionally below the live-writer boundary for
`word_800916F6`: it does not infer MenuHelp/StageClear TEXT state and does
not use Win files, replay samples, payload readability, ISO readability, or
STR/XA state as authority.

## Direct unit added

- Source: `src/pr/pr_stage_payload_bank_direct.h`
- Source: `src/pr/pr_stage_payload_bank_direct.cpp`
- Build list: `CMakeLists.txt`

The direct unit owns the payload-bank layout, pure helper facts, and the
mutable memory slices that model the PSX bank:

| PSX item | Direct-unit surface |
| --- | --- |
| `80092F10` base | `kBaseAddress80092F10` |
| `80092F10[4876]` payload bank | `MemoryState80092F10::savePayloadBank` |
| `80079008[4876]` backup bank | `MemoryState80092F10::saveStatusBackup` |
| `80092F1D` status bytes | `kStatusBaseAddress80092F1D` |
| `80092F24` score dwords | `kScoreBaseAddress80092F24` |
| `80092F3C/40/44/48/5C` carrier/mirror offsets | `kCarrier*`, `kMirrorDstAddress80092F5C` |
| `8008EEF8 -> 80092F5C` mirror source | `MemoryState80092F10::replayMirror`, `kMirrorBytes8001635C = 4800` |
| `8001615C` stage-to-slot map | `MapSaveStage8001615C(...)` |
| `800161A8` scene map | `MapScene800161A8(...)` |
| `800161F4` six-status completion query | `QueryAllStatuses800161F4(...)`, `AllStatusesClear800161F4(...)` |
| writer provenance / faults | `MarkPayloadWriter80092F10(...)`, `DirectMemoryPtr80092F10(...)` |
| `800166AC` status query | `QueryStatus800166AC(...)` |
| `8001628C` unlock/status writer | `EnsureProgress8001628C(...)` |
| `8001635C` update-save-payload bank mutation | `UpdateSavePayload8001635C(...)` |
| `80015CC4` init-save-payload clear body | `InitSavePayload80015CC4(...)` |
| `800164B4` load-save-payload bank write | `LoadSavePayload800164B4(...)` |
| `800167A8` leaf status writer | `WriteStatus800167A8(...)` |
| `800169E0` saved-score sync bank read | `SyncSavedScore800169E0(...)` |
| status/score/carrier snapshot | `SnapshotStatusBank80092F10(...)` |
| save-status prefix snapshot | `SnapshotSaveStatusPrefix80092F10(...)` |

## SaveUi adapter use

`PrStage1SaveUiDirect` now stores only a
`PrStagePayloadBankDirect::MemoryState80092F10 payloadBank` member for this
bank. The SaveUi direct-memory dispatcher delegates the three payload ranges
to `DirectMemoryPtr80092F10(...)`, while SaveUi/card/list/UI buffers stay in
the SaveUi module.

The SaveUi adapter consumes the constants/helpers instead of owning the map
tables, completion-loop body, or mutable payload-bank arrays locally:

- `Sub8001615CDirect(...)` calls `MapSaveStage8001615C(...)`.
- `Sub800161A8Direct(...)` calls `MapScene800161A8(...)`.
- `Sub800161F4(...)` is now a SaveUi wrapper around
  `QueryAllStatuses800161F4(...)`; the wrapper keeps the public return carrier
  and `helperGap` projection.
- `Sub8001635CDirect(...)` remains a SaveUi wrapper for action trace,
  `8001615C` trace projection, `dword800901BC` known/value attribution, and
  helper gaps. Its payload-bank mutation now calls
  `UpdateSavePayload8001635C(...)`; there is no remaining SaveUi-local
  `800161F4` read loop.
- `PrStage1SaveStatusPrefix80092F10` takes its address/byte-count constants
  from `PrStagePayloadBankDirect`; `GetSaveStatusPrefix80092F10(...)` is now a
  SaveUi wrapper around `SnapshotSaveStatusPrefix80092F10(...)` and only adds
  the SaveUi `helperGap` projection.
- `MarkSavePayloadWriter80092F10(...)` delegates to
  `MarkPayloadWriter80092F10(...)`.
- `Sub800166AC(...)`, `Sub8001628C(...)`, `Sub8001635C(...)`,
  `Sub80015CC4(...)`,
  `Sub800164B4(...)`, `Sub800167A8(...)`, `Sub800169E0(...)`, and
  `GetStageClearStatusBankSnapshot(...)` are now SaveUi wrappers around
  payload-bank direct APIs. The wrappers still project `psxFunction`,
  `helperGap`, action trace, source resolver state, and public return types
  for existing callers.

## Still not closed

- `80015CC4` init-save-payload clear body is direct-owned, but its public
  SaveUi wrapper still owns action trace, helper gaps, `savePayloadSourceKnown`,
  and the fixed initial `8001635C(1,1,1,0)` call.
- `8001635C` update-save-payload bank mutation is direct-owned, but its public
  SaveUi wrapper still owns action trace, helper gaps, `8001615C` trace
  projection, `dword800901BC` known/value source attribution, and
  runner/scorer execution point.
- `800164B4` load-save-payload bank copy body is direct-owned, but its public
  SaveUi wrapper still owns `srcAddress` validation, source resolver state,
  action trace, and live-gap projection.
- `GetSaveStatusPrefix80092F10(...)` still has a SaveUi public wrapper for
  `helperGap` and seed/import compatibility, but the prefix payload-bank facts
  are direct-owned by `SnapshotSaveStatusPrefix80092F10(...)`.
- `800161F4` has no lifecycle/status-bank host bridge request. It remains an
  internal SaveUi/payload-bank query API and must not be added to
  `801C81EC` request/feedback handling.
- `800169E0` saved-score bank read body is direct-owned, but its SaveUi
  wrapper, runner/scorer execution point, action trace, and live-gap projection
  remain outside the payload-bank unit. It must not be added to the
  `801C81EC` status-bank host bridge executable request list.
- `word_800916F6` remains consumer-only in the current SCUS evidence.
- `80026314` caller/context and `800167A8` live caller remain open; the
  `800167A8` leaf writer body being direct-owned does not close its caller.
