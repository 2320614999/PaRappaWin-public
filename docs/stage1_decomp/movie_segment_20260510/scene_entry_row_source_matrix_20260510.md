# Scene entry row source matrix for `801C4780`

Scope: MOVSEG-row-source static audit only. No build, no test, no emulator, no
`src` edits.

This note answers one narrow question: whether the current Win side already has
authoritative carriers for the seven `sceneEntry + 0x0C + i * 0x30` rows scanned
by PSX `801C4780`, and whether those carriers can be connected directly to
`MaterializeSceneEntryMovieSegments801C4780(...)`.

## Evidence read

- `docs/项目规则.md`: direct-port authority must come from PSX control flow,
  tables, loader records, or explicit HAL feedback; do not infer row fields from
  Win shell state.
- `movie_segment_next_cutover_plan_20260510.md`: planned boundary is
  `sceneEntry loader rows -> 7 x MovieSegmentRecord48 -> 801C4780 scan ->
  sceneEntry+0x6C -> 801C455C`.
- `movie_segment_48_record_fields_20260510.md`: row layout is 0x30 bytes and
  `801C4780` scans seven rows.
- `cd_lookup_feedback_bridge_20260510.md`: `CdlFILE.pos/size` may enter only as
  explicit `8001A2B0/800381F8` feedback.
- `src/pr/pr_stage1_movie_segment_direct.h/.cpp`: current direct carriers and
  helpers.
- `src/pr/pr_stage1_loader_direct.h/.cpp` and
  `src/pr/pr_stage1_loader_cd_hal.h/.cpp`: loader/CD feedback carriers.
- `src/pr/pr_stage1_overlay_parser.h/.cpp`: current COMOD/COMPO parser output.
- `src/pr/pr_scenes.cpp`: current scene/movie/text/STR adapters.

## Static conclusion

Win currently has a partial carrier layer, not a true row source.

`PrStage1MovieSegmentDirect` can hold seven `MovieSegmentRecord48` rows, assign
their PSX row addresses, accept explicit `CdlFILE.pos/size` feedback, scan all
seven rows, select `sceneEntry+0x6C`, and bridge selected row fields into the
existing `801C455C` input. However, the current Win runtime does not yet
materialize the real seven row table from scene-entry data. The current
`PrStage1OverlayParser` does not parse this row table, and `pr_scenes.cpp`
does not call the movie-segment scan path for MOVIE1.

The existing lifecycle scene-entry carrier is only a slot-presence/offset
carrier. `SceneEntry801C7284::loaders[i]` records `present`, `slotIndex`, and
`psxOffsetFromSceneEntry = 12 + 48 * i`; it does not carry row `pathPtr`,
`+0x08`, initial `+0x0C`, `CdlFILE`, or row bytes.

## Owner matrix

| Field / source edge | PSX owner | Win existing owner | Can direct-connect to `MaterializeSceneEntryMovieSegments801C4780`? | Gap if not direct-connectable |
|---|---|---|---|---|
| Seven-row table shape (`7 x 0x30`) | `801C4780` loop over `sceneEntry+0x0C+i*0x30` | Constants `kSceneEntryMovieSegmentCount801C4780`, `kMovieSegmentRecordSize8001A324`; lifecycle `SceneEntry801C7284::loaders[i].psxOffsetFromSceneEntry` | Partially: address/shape only | Shape is known, but row bytes are not sourced. |
| Current `sceneEntry` base (`*(dword_8006ED98+0x20)`) | PSX scene lifecycle/current scene entry | `SceneEntryMovieSegmentTable801C4780::sceneEntryBase`; `SceneEntry801C7284::sceneEntryPtr8006EDB8` exists but is initialized to `0` | No, except as caller-supplied metadata | Runtime does not expose the real current scene-entry base for this materializer. |
| Row PSX address | Derived from sceneEntry base and row index | `MaterializeSceneEntryMovieSegments801C4780` sets `row.psxAddr = sceneEntryBase + 0x0C + i*0x30` when base is known | Yes, if sceneEntry base is supplied | Only address is materialized; this does not prove row contents. |
| `row+0x00 pathPtr` | Static scene-entry loader row | `PrStage1LoaderDirect::CdSeamPayload::pathPtr` for loader actions; `PrStage1LoaderCdHal::Probe8001A2B0State::pathPtr` for probe actions | No | `MovieSegmentRecord48` has no `pathPtr` field, and no parser currently extracts the seven row path pointers. Existing pathPtr carriers are action/probe carriers, not scene-entry row storage. |
| `row+0x04` opaque metadata | Static scene-entry loader row | None in `MovieSegmentRecord48`; lifecycle only has slot metadata | No | No Win row-byte or opaque dword carrier exists for this field. |
| `row+0x08 endBias` | Static scene-entry loader row; consumed by `8001A7F8` | `MovieSegmentRecord48::endBiasA1Plus8Known/endBiasA1Plus8`; bridge into `MovieStepSegmentFields801C4350` and `801C455C` input exists | Only if a caller already provides the row value | Carrier exists, but no current table parser or lifecycle source fills it. Must not default from STR duration/frame state. |
| `row+0x0C loadedState` initial value | Static scene-entry row plus `8001A324` re-entry state | `MovieSegmentRecord48::loadedStateA1Plus0CKnown/loadedStateA1Plus0C`; `MovieSegmentRowInitFeedback8001A324` can also carry it | Only if supplied as row/feedback input | Carrier exists, but true initial table source is absent. Current `PsxCall8001A324_InitSegmentRecord` computes LBA/end fields but does not fully model the `pathPtr == 0`, already-loaded guard, or the PSX write of state `1`. |
| `row+0x10..0x13 CdlFILE.pos` | `8001A2B0 -> 800381F8` lookup feedback | `PrStage1LoaderCdHal::LookupFeedback800381F8`, `ProbeCompletionFeedback8001A2B0`, `MovieSegmentRowInitFeedback8001A324::cdlFilePos`; applied into `MovieSegmentRecord48::startMsf` | Not through `Materialize...` alone; yes through `PsxCall801C4780_ScanMovieSegmentsWithFeedback` if per-row feedback exists | Feedback carrier exists, but no per-row movie-segment probe is wired from real `pathPtr`, and no host HAL currently supplies authoritative per-row `CdlFILE.pos` here. |
| `row+0x14 CdlFILE.size` | `8001A2B0 -> 800381F8` lookup feedback | Same bridge as above; applied into `MovieSegmentRecord48::lengthSourceA1Plus20` | Not through `Materialize...` alone; yes through scan-with-feedback if per-row feedback exists | Feedback carrier exists, but producer is still open. |
| `row+0x18..0x27 CdlFILE.name[16]` | `8001A2B0 -> 800381F8` lookup feedback | None in `MovieSegmentRecord48` or movie-step bridge | No | Current movie-step consumers do not need it, but a faithful row table model does not carry it yet. |
| `row+0x28 startLba / timeBase` | `8001A324`: `80036A78(CdlFILE.pos)` | `MovieSegmentRecord48::timeBaseA1Plus40`; `PsxCall8001A324_InitSegmentRecord` writes it when `startMsfKnown` | Yes after explicit `CdlFILE.pos` feedback or prefilled row | Depends on the still-open `CdlFILE.pos` producer. |
| `row+0x2C endLba / segmentEnd` | `8001A324`: `startLba + (size >> 11)` | `MovieSegmentRecord48::endA1Plus44`; `PsxCall8001A324_InitSegmentRecord` writes it when start LBA and size are known | Yes after explicit `CdlFILE.pos/size` feedback or prefilled row | Depends on the still-open `CdlFILE.pos/size` producers. |
| `sceneEntry+0x6C` selected row | `801C4DC4` selects row index 2 after the table-wide scan | `PsxSelectMovieSegment801C4DC4FromScan`; `BuildMovieTextOuterLoopInputFromScan801C455C` can copy row fields to `801C455C` input | Yes, but only after a real scan result exists | `pr_scenes.cpp` does not currently build or pass a `MovieSegmentScanResult801C4780` into MOVIE1 runtime. |
| `80025A00()` action after scan | `801C4780` tail action | `MovieSegmentScanResult801C4780::reset80025A00Action` | Yes as an action flag | Side effect remains a lower-boundary action, not executed by this materializer. |
| `8001AC18(sceneEntry+0x3C, 0)` action after scan | `801C4780` tail COMPO/INT load | `MovieSegmentScanResult801C4780::loadCompo8001AC18Action/loadCompoArg0/loadCompoArg1` | Yes as an action flag if sceneEntry base is known | Loader completion and resource side effects are still outside this row materializer. |
| STR movie playback facts | Win platform/HAL adapter only | `BuildStage1Movie1HostFeedback`, `AdvanceStage1Movie1HostStrPlayer`, `ExecuteStage1Movie1HostActions` | No | These are not row sources. They must not be used to infer `pathPtr`, `+0x08`, `+0x0C`, `CdlFILE`, `+0x28`, or `+0x2C`. |

## Existing Win carriers by layer

| Layer | Existing data | What it can safely provide | What it cannot provide |
|---|---|---|---|
| `PrStage1LifecycleDirect::SceneEntry801C7284` | Seven loader slots with `present`, `slotIndex`, and scene-relative offsets | Slot existence and row offset skeleton (`0x0C`, `0x3C`, `0x6C`, `0x9C`, `0xCC`, `0xFC`, `0x12C`) | Real row bytes, `pathPtr`, `+0x08`, initial `+0x0C`, `CdlFILE`, current scene-entry base |
| `PrStage1OverlayParser` | Text tables, pair table, stream desc rows, compact rail rec44, HUD config rows, HUD slots, event streams, mem handle names | COMOD/COMPO overlay data already parsed for text/HUD/script paths | The seven `sceneEntry` loader rows are not parsed here |
| `PrStage1MovieSegmentDirect::MovieSegmentRecord48` | Known flags for loaded state, start MSF, length source, end bias, start/end LBA | Canonical movie-row carrier for fields needed by `801C4350/8001A7A4/8001A7F8` | `pathPtr`, opaque `+0x04`, full `CdlFILE.name`, raw 0x30 row bytes |
| `PrStage1LoaderCdHal` | `LookupFeedback800381F8`, `ProbeCompletionFeedback8001A2B0`, four-try probe state | Explicit `CdlFILE.pos/size` feedback when a lower lookup has actually supplied it | Static row fields and path discovery |
| `PrStage1LoaderDirect` | Loader action skeleton and generic CD seam payloads | Existing loader/bootstrapping CD HAL action plumbing | Per-row movie-segment table materialization |
| `pr_scenes.cpp` MOVIE1 adapter | Host STR readiness, file existence, language, subtitle flags, movie frame, video-finished state, STR actions | Platform playback/action boundary | PSX row fields; it currently does not call the `801C4780` movie-segment materializer/scan path |
| `PrStage1MovieTextOuterLoopDirect` | `BuildMovieTextOuterLoopInputFromSegment801C455C` and `BuildMovieTextOuterLoopInputFromScan801C455C` | Direct bridge from selected row to existing `801C455C` segment fields | It is a consumer bridge only; it does not source or scan the seven rows |

## Direct-connect decision

`MaterializeSceneEntryMovieSegments801C4780(...)` can be connected only to a
caller that already has authoritative row values. Today, no such caller exists
in the checked Win code.

Safe direct-connect inputs today:

- caller-supplied `sceneEntryBase` to compute row PSX addresses;
- caller-supplied `MovieSegmentRecord48 rows[7]`, if another direct parser later
  extracts the real table values;
- explicit per-row `MovieSegmentRowInitFeedback8001A324` from
  `8001A2B0/800381F8`, but only through
  `PsxCall801C4780_ScanMovieSegmentsWithFeedback(...)`.

Not safe / not present:

- deriving row fields from `ctx.currentSceneDef->movie.path`, STR file
  existence, `StrPlayer::GetPlayedSecondsPrecise()`, `strPlayed`,
  `IsVideoFinished()`, subtitle windows, or Win frame counters;
- treating lifecycle `loaderPresent[2]` as `pathPtr`;
- treating row `+0x08` as zero without the static table saying zero;
- using `Materialize...` as if it parses row bytes. It currently copies caller
  rows and annotates addresses; it is not the row source.

## Required gap closure before runtime cutover

1. Add or identify a direct scene-entry row-table source that extracts all seven
   `0x30` rows, at minimum `pathPtr`, `+0x08`, and initial `+0x0C`.
2. Keep `CdlFILE.pos/size` as explicit `8001A2B0/800381F8` feedback; do not
   fabricate it from host STR playback.
3. Either extend `MovieSegmentRecord48` or add a separate row-source carrier for
   `pathPtr` and any raw fields needed to drive `8001A324` faithfully.
4. Wire `pr_scenes.cpp` only after the above source exists: materialize seven
   rows, scan all seven, then select row index 2 (`sceneEntry+0x6C`) for the
   existing `801C455C` bridge.
