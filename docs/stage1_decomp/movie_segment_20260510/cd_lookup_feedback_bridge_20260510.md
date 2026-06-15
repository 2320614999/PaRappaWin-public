# CD lookup feedback bridge for movie segment scan

Scope: P1-D1 static/code seam note. No build, no test, no emulator.

## Goal

Align the `8001A2B0` CD lookup HAL feedback seam with the movie segment
`801C4780` seven-row scan without turning Win filesystem or STR playback state
into PSX truth.

The bridge added in code is intentionally only a type conversion:

```text
8001A2B0 probe / 800381F8 lookup feedback
  -> ProbeCompletionFeedback8001A2B0
  -> BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(...)
  -> MovieSegmentRowInitFeedback8001A324
  -> PsxCall801C4780_ScanMovieSegmentsWithFeedback(...)
```

## Code boundary

- `PrStage1LoaderCdHal::LookupFeedback800381F8` carries the external lookup
  result plus optional `CdlFILE.pos` and `CdlFILE.size` written by the lookup
  HAL.
- `PrStage1LoaderCdHal::ProbeCompletionFeedback8001A2B0` exposes whether the
  four-try `8001A2B0` probe completed and whether it ended in lookup success or
  failure.
- `PrStage1MovieSegmentDirect::BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0`
  converts only known successful lookup fields into
  `MovieSegmentRowInitFeedback8001A324`.
- The bridge does not fabricate `CdlFILE.pos`, `CdlFILE.size`, `+0x08`
  end-bias, scene-entry base, or CD clock bytes.
- `MovieSegmentScanResult801C4780` records lookup-feedback coverage per scan:
  which rows received explicit feedback, which rows still need lookup feedback,
  which rows already have `CdlFILE.pos/size`, and which rows are still missing
  live feedback.
- The scan result also records `CdlFILE.name[16]` coverage separately from
  `pos/size`: `rowCdlFileNameReadyMask[7]`,
  `rowMissingCdlFileNameFeedbackMask[7]`, and
  `gapMissingCdlFileNameFeedback`. Name bytes are carried only when live
  lookup feedback supplies them; the Stage1 static zero-reserved CdlFILE area
  does not close this gap.

## Preserved scan semantics

`801C4780` remains table-wide:

- materialize `sceneEntry + 0x0C + i * 0x30` for `i=0..6`;
- optionally apply per-row `MovieSegmentRowInitFeedback8001A324`;
- run `PsxCall8001A324_InitSegmentRecord(...)` for each row;
- expose `80025A00()` and `8001AC18(sceneEntry+0x3C, 0)` as actions.

This is not a `seg2` manual-fill path. `sceneEntry+0x6C` is selected only after
the seven-row scan result exists.

`gapMissingCdLookupFeedback` is diagnostic carrier state, not fallback logic. A
row is treated as ready only when its record has explicit start MSF and size
facts before `8001A324` runs. `gapMissingCdlFileNameFeedback` is tracked
separately because current movie-step math can proceed without name bytes, but
a faithful CdlFILE payload still needs live `8001A2B0/800381F8` name feedback.
Host file existence, STR duration, video finished state, and frame counters do
not close either gap.

## Still-open HAL gaps

The bridge does not close these producers:

- real `CdlFILE.pos` and `CdlFILE.size` acquisition from the CD lookup host HAL;
- the live `CdlFILE.name[16]` producer, now visible as a separate carrier gap;
- static table producer for row `+0x08` end-bias;
- `byte_800493F4` / stream clock producer for `8001A7A4`;
- `80025A00()` lower side effects;
- `8001AC18(sceneEntry+0x3C, 0)` COMPO/INT load completion feedback;
- scene-entry base ownership from `dword_8006ED98+0x20`.

## Non-goals

- Do not treat successful host file existence as successful PSX `CdlFILE`
  lookup unless the host HAL returns explicit `pos/size`.
- Do not source row `+0x28` or `+0x2C` from STR playback duration, current video
  frame, or Win filesystem metadata.
- Do not collapse the `801C4780` scan into a one-row shortcut for
  `sceneEntry+0x6C`.
