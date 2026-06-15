# Movie segment `801C4780` row scan to `801C455C` (2026-05-14)

Authority:
- `docs/stage1_decomp/ida_export_movie_segment_801c4780_row_scan_to_801c455c_20260514.md`
- Current active IDA database: `SCUS_941.83`

Scope:
- Stage1 movie segment source for `801C455C` / common-lyrics outer-loop.
- This document does not authorize Win file, ISO readability, STR playback, or
  render-derived facts as PSX success.

## Confirmed Call Order

`801C4260` seeds the current scene-entry cache:

- `801C42D0`: writes `dword_8006ED98+0x20 = sceneEntry`
- `801C431C`: writes `sceneEntry+0x164 = *(s16+8)+*(s16+10)`
- `801C4330`: calls `801C4780`

`801C4780` performs the table-wide scan:

- starts at `sceneEntry+0x0C`
- loops 7 rows with stride `0x30`
- calls `8001A324(row)` for each row
- calls `80025A00()`
- calls `8001AC18(sceneEntry+0x3C, 0)`

`801C4DC4` consumes the scanned table:

- calls `801C44E0(sceneEntry+0x6C, 0)`
- calls `801C455C(sceneEntry+0x6C, &unk_801C3640, 0)`
- later calls `801C4894(sceneEntry+0x9C, &unk_801C3640)`

`801C455C` consumes the same scene-entry cache:

- reads `dword_8006ED98+0x20`
- reads `sceneEntry+0x164`
- writes that value to `ctx+0x0C`

## Direct-Port Mapping

Existing direct code already has the correct shape:

- `MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(sceneIndex)`
  builds the seven static rows.
- `PsxCall801C4780_ScanMovieSegmentsWithFeedback(...)` loops all seven rows and
  exposes `80025A00` / `8001AC18(sceneEntry+0x3C,0)` as actions.
- `PsxSelectMovieSegment801C4DC4FromScan(...)` selects row index `2`
  (`sceneEntry+0x6C`).
- `BuildMovieTextOuterLoopInputFromScan801C455C(...)` transfers selected row
  `+0x28/+0x2C/+0x08` fields into `801C455C` movie-step input.

## Remaining Gaps

- `memory164` still is not wired into `MovieTextOuterLoopInputSub801C455C`.
  It now has a source boundary: `sceneEntry+0x164`, produced by `801C4260`.
  It should be carried with an explicit `memory164Known` flag rather than
  defaulting to `0`.
- Row `+0x28/+0x2C` still depends on `8001A324` mutation from real CD lookup
  feedback (`CdlFILE.pos/size`). The direct carrier must not infer these values
  from STR playback.
- `sub8001A750Result` and `GsGetWorkBase` remain CD/status facts
  (`800364D0`, `8001A3C8`) and are not solved by `801C4780`.
- `dword801C9554EqualsOne` remains sourced from `801C44E0`, not from
  `801C455C`.

## Next Runtime Step

The next code change should be a guard/source tightening pass, not another
carrier:

- add explicit known/gap fields for `memory164`, `sub8001A750Result`, and
  `dword801C9554EqualsOne`;
- feed `memory164` from the scene-entry cache source when available;
- mark unknown `sub8001A750Result` / `GsGetWorkBase` as gaps instead of treating
  default `0` as authoritative.
