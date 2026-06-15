# Movie outer-loop direct carrier gap - 2026-05-10

Scope: code edit only in `pr_stage1_movie_text_outer_loop_direct.*`; no build,
no test.

## Change

`801C4350` now treats the movie segment window fields as direct-only:

- `segmentTimeBaseA1Plus40` (`row+0x28`)
- `segmentEndA1Plus44` (`row+0x2C`)
- `segmentEndBiasA1Plus8` (`row+0x08`)

Those fields only close `8001A7A4` / `8001A7F8` when
`MovieTextOuterLoopInputSub801C455C::segmentWindowFieldsFromDirectCarrier` is
set by one of the direct bridges:

- `BuildMovieTextOuterLoopInputFromSegment801C455C(...)`
- `BuildMovieTextOuterLoopInputFromScan801C455C(...)`
- `BuildMovieTextOuterLoopScanInputCarrier801C455C(...)`
- `BuildMovieTextOuterLoopScanInputCarrierFromSceneEntryTable801C455C(...)`
- `BuildStage1MovieTextOuterLoopScanInputCarrierFromStaticSceneEntry801C455C(...)`

If a caller still fills the same values as loose host facts, the outer-loop
runtime records `GapHostDerivedSegmentWindowFields` and keeps the movie-step
result dependent on lower direct feedback (`sub8001A7A4/sub8001A7F8`) or an
explicit `sub801C448C` result.

The new scan-input carrier is the narrow cutover seam:

```text
scene-entry table
  -> 801C4780 seven-row scan
  -> 801C4DC4 row2 selection (sceneEntry+0x6C)
  -> 801C455C outer-loop input
```

It records explicit gaps instead of filling them from the host:

- `gapMissingScanResult`
- `gapMissingSelectedMovieRow`
- `gapMissingSegmentWindowFields`
- `gapMissingCdLookupFeedback`

`directSegmentWindowFieldsComplete` is true only when selected row2 carries all
three direct fields needed by `801C4350`: `+0x28`, `+0x2C`, and `+0x08`.

The scan result now also carries CD lookup feedback coverage:

- `feedbackAppliedCount` and `feedbackAppliedMask[7]`
- `rowNeedsCdLookupCount` and `rowNeedsCdLookupMask[7]`
- `rowCdLookupReadyCount` and `rowCdLookupReadyMask[7]`
- `rowMissingCdLookupFeedbackCount` and
  `rowMissingCdLookupFeedbackMask[7]`
- `rowCdlFileNameReadyCount` and `rowCdlFileNameReadyMask[7]`
- `rowMissingCdlFileNameFeedbackCount` and
  `rowMissingCdlFileNameFeedbackMask[7]`

The outer-loop scan carrier mirrors the row2 facts as
`selectedMovieRowNeedsCdLookupFeedback`,
`selectedMovieRowCdLookupReady`, and
`selectedMovieRowMissingCdLookupFeedback`, plus the name-specific
`selectedMovieRowCdlFileNameReady` and
`selectedMovieRowMissingCdlFileNameFeedback`. This is only live lookup feedback
visibility: it does not synthesize `CdlFILE.pos/size/name` and does not infer
segment windows from host STR playback. Stage1 static scene-entry rows preserve
the CdlFILE reserved area as unknown until explicit live lookup feedback arrives.

The carrier now also exposes the clock producer seam:

- `byte800493F4ClockProducerKnown`
- `gapMissingByte800493F4ClockProducer`

`StepMovieTextOuterLoopSub801C455C(...)` records
`GapByte800493F4ClockProducer` when live `byte_800493F4` bytes are not provided.
This is a gap expression only; it does not accept an already-computed
`sub80036A78` LBA and does not derive the stream clock from Win playback time,
current video frame, or STR completion.

## Reason

The PSX authority for these fields is the row chain:

```text
sceneEntry rows -> 801C4780 -> 8001A324 -> sceneEntry+0x6C -> 801C455C
```

They must not be sourced from Win STR duration, video-finished state, subtitle
window state, or frame counters. This preserves the direct-port boundary while
the remaining CD lookup feedback and clock producer are still being closed.

## Remaining gaps

- Runtime scene-entry table cutover into the scene shell remains open; this
  edit only exposes the direct carrier and Stage1 static-table producer helper.
- `8001A2B0/800381F8` CD lookup feedback owner for live
  `CdlFILE.pos/size/name`;
  current code can now report missing coverage per scan row, but still depends
  on explicit feedback from the lower HAL.
- `byte_800493F4` stream clock producer for `8001A7A4`; current code only
  exposes the missing-producer gap unless the caller supplies live bytes.
  Supplying an explicit `80036A78` result is intentionally not a valid producer
  boundary.
- Runtime caller cutover from host-derived loose fields to scan-result ->
  `BuildMovieTextOuterLoopScanInputCarrier801C455C(...)`.
