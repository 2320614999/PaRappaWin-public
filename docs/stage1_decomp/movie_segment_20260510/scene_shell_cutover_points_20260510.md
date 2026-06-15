# Stage1 scene shell cutover points

Scope: task B static analysis only. No build, no test, no `src` edits.

## Conclusion

Current Stage1 MOVIE1 / lifecycle / STR-row authority is still split:

- `801C7284/801C81EC` direct code emits PSX-shaped lifecycle actions and row offsets (`+0x6C`, `+0x9C`, `+0xCC`, `+0xFC`), but the current scene-entry carrier only knows slot presence and offsets.
- Actual STR file selection is still fed by the Win shell: initial MOVIE1 hardcodes `SS/MOVIE1.STR`; clear-tail/result STRs resolve `sceneLoaderSlot -> ctx.currentSceneDef -> file path`.
- `PrStage1MovieSegmentDirect::MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(...)` and the `801C4780` scan helpers already exist, but they are not called by `pr_scenes.cpp` or the current MOVIE1/lifecycle path.

Suggested one-line cutover boundary:

```text
PrScn1::Fn1 scene init: static scene-entry rows -> 801C7284 scene carrier -> 801C4780 scan result; PrScn1::Fn2/801C81EC consumes row selections and emits host actions, while pr_scenes.cpp only executes platform side effects.
```

Do not wire the materializer inside the MOVIE1 host STR adapter. That would keep the row source behind the old shell. The PSX order puts the row table scan in scene init / lifecycle setup, before `801C81EC` starts blocking STR playback.

## Current cutover points

| File / function | Lines | Static finding |
|---|---:|---|
| `src/pr/pr_scenes.cpp` / `TryResolveCurrentSceneLoaderSlotPath` | 739-763 | Win shell maps slot `0..6` to `ctx.currentSceneDef->{comod,compo,movie,xa,resultMovieA,resultMovieB,zcompo}` and builds a host filesystem path. This is the old row-to-path adapter. |
| `src/pr/pr_scenes.cpp` / `BuildStage1LifecycleInitSceneInput801C7284` | 3177-3195 | `801C7284` input is built from `ctx.currentSceneDef` path presence booleans only. It does not pass row bytes, `pathPtr`, `+0x04`, `+0x08`, `+0x0C`, or CdlFILE fields. |
| `src/pr/pr_stage1_lifecycle_direct.cpp` / `InitScene801C7284` | 396-421 | Direct init creates loader records with `present`, `slotIndex`, and `psxOffsetFromSceneEntry = 12 + 48*i`; `sceneEntryPtr8006EDB8` is still `0`. This is a skeleton, not the PSX scene-entry row table. |
| `src/pr/pr_scenes.cpp` / `PrScn1::Fn1` | 5705-5772 | Current scene init calls `InitScene801C7284(BuildStage1LifecycleInitSceneInput801C7284(ctx))`, resets MOVIE1/text/runtime adapters, and loads subtitle text from COMOD bytes. This is the correct place to connect static row materialization/scan. |
| `src/pr/pr_scenes.cpp` / `PrScn1::Fn2` | 5773-5774 | Runtime entry is `TickStage1Scene1_801C81EC_ActionExecutor(ctx)`. This is the current `801C81EC` shell cutover point. |
| `src/pr/pr_scenes.cpp` / `TickStage1Scene1_801C81EC_ActionExecutor` | 4874-4974 | Per frame, the shell ticks host blocks, builds `FrameInput801C81EC`, then calls `Step801C81EC(s_stage1LifecycleRuntime801C81EC, s_stage1LifecycleScene801C7284, input)`. This is where `801C81EC` should consume row-backed actions rather than Win path-derived slots. |

## Current MOVIE1 feed

| File / function | Lines | Static finding |
|---|---:|---|
| `src/pr/pr_stage1_lifecycle_direct.cpp` / `EmitInitialMovie1Request` | 110-144 | The direct lifecycle emits transition, `StrInit`, and `StrPlayAndWait` for loader offset `0x6C`, then marks the play action blocking. This matches row index 2 but only carries loader slot metadata. |
| `src/pr/pr_scenes.cpp` / `ApplyStage1LifecycleAction801C81EC` | 4736-4747 | `StrPlayAndWait + InitialMovie1` starts movie text play/wait and begins a `Movie1` host block. It does not pass a movie segment row or scan result. |
| `src/pr/pr_scenes.cpp` / `TickStage1InitialMovieBlock801C81EC` | 4108-4139 | The Movie1 host block advances through `AdvanceStage1Movie1DirectFromHostAdapter`. Completion feeds `BuildMovie1CompletedHostBlockFeedback801C81EC(0)` back into `801C81EC`. |
| `src/pr/pr_scenes.cpp` / `BuildStage1Movie1HostFeedback` | 3877-3893 | Host feedback hardcodes the existence check to `ctx.dataRoot / "SS" / "MOVIE1.STR"`. This is current MOVIE1 file authority. |
| `src/pr/pr_scenes.cpp` / `ExecuteStage1Movie1HostActions` | 3924-3945 | Host actions hardcode playback to `ctx.dataRoot / "SS" / "MOVIE1.STR"`. Direct `801C4780` row 2 is not involved. |
| `src/pr/pr_stage1_scene1_movie1_direct.h/.cpp` / `Movie1HostFeedback`, `AdvanceRuntimePure` | h:610-622, cpp:3119-3407 | MOVIE1 direct runtime consumes host booleans/frame/video-finished state and emits host actions. It has no segment-row input. |

## Current lifecycle / STR row feed

| File / function | Lines | Static finding |
|---|---:|---|
| `src/pr/pr_stage1_lifecycle_direct.cpp` / `EmitStageLoopRequest` | 147-168 | Stage loop uses loader offset `0x9C` for `StageRecordTick1A4D0` / `StageRunnerRun7A60`. This is row index 3, but still only attached as a loader slot. |
| `src/pr/pr_stage1_lifecycle_direct.cpp` / `EmitClearTailMovieRequest` | 238-273 | Clear-tail chooses `0xCC` or `0xFC` from `byte801C368E`, sets play mode `1/2`, emits `StrInit`, transition, and `StrPlayAndWait`. These are row indexes 4/5, but the host path is resolved later by Win shell. |
| `src/pr/pr_stage1_lifecycle_executor_direct.cpp` / `GetActionHostBlockKind801C81EC` | 905-927 | `StrPlayAndWait + InitialMovie1` becomes `Movie1`; `StrInit + ClearTailMovie` becomes `ClearTailMovie`. Host block classification is action-kind based, not row-table based. |
| `src/pr/pr_scenes.cpp` / `StartStage1ClearTailMovieBlock801C81EC` | 4142-4166 | Clear-tail movie path is resolved through `TryResolveCurrentSceneLoaderSlotPath(ctx, action.sceneLoaderSlot, path)`, then stored in the executor host block. |
| `src/pr/pr_scenes.cpp` / `TickStage1ClearTailMovieBlock801C81EC` | 4169-4215 | The shell plays and updates the resolved filesystem path with `StrPlayer`. Completion feeds `BuildClearTailMovieCompletedHostBlockFeedback801C81EC()` back into lifecycle input. |
| `src/pr/pr_stage1_lifecycle_executor_direct.cpp` / host-block state | 1974-2055, 2200-2221, 2350-2401 | Executor state owns `movie1BlockActive`, `clearTailMoviePathResolved`, `clearTailMoviePath`, and completion flags. It is a blocking host adapter, not a row source. |

## Current old terminal scene adapter

| File / function | Lines | Static finding |
|---|---:|---|
| `src/pr/pr_stage1_scenes_direct.cpp` / `Scene1LifecycleState` | 8-46 | Separate clear-post-run mini FSM stores result movie slot/mode and terminal phases. This is an older scene shell layer beside `801C81EC`. |
| `src/pr/pr_stage1_scenes_direct.cpp` / `PrimeClearPostRun` | 153-181 | Copies `clearResultMovieLoaderSlot` and `clearResultMoviePlayMode` from terminal resolution into old adapter state. |
| `src/pr/pr_stage1_scenes_direct.cpp` / `TickClearPostRun` | 212-342 | Drives prelude transition, result shell, result movie playback request, post-movie shell, reset/SFX/save/finalize actions as a custom phase machine. |
| `src/pr/pr_scenes.cpp` / `PrimeStage1ClearPostRunAdapter` | 5138-5174 | Resolves result movie path using `TryResolveCurrentSceneLoaderSlotPath`, again from slot -> `currentSceneDef` rather than from row data. |
| `src/pr/pr_scenes.cpp` / `TickStage1ClearPostRunAdapter` | 5176-5250 | Executes old adapter actions, including `StrPlayer::Play(s_stage1ClearPostRunAdapter.resultMoviePath)`. This should disappear once `801C81EC` owns the terminal clear-tail lifecycle. |
| `src/pr/pr_scenes.cpp` / `ApplyStage1SceneLoopTerminalResolution` | 5613-5656 | Routes clear/fail terminal decisions through `PrStage1ScenesDirect` instead of directly through the `801C81EC` lifecycle executor. |

## Direct row code already present

| File / function | Lines | Static finding |
|---|---:|---|
| `src/pr/pr_stage1_movie_segment_direct.h` / constants and carriers | 10-14, 22-89, 104-130 | Defines 0x30 row size, seven-row count, Stage1 base `0x800548B8`, row carrier, table carrier, scan result, and selected segment fields. |
| `src/pr/pr_stage1_movie_segment_direct.cpp` / `MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780` | 151-176 | Static Stage1 table rows are encoded for COMOD1, COMPO01, MOVIE1, STAGE1.XA1, XMOVIE1 rows, and ZCOMPO. This is the right source for initial row bytes. |
| `src/pr/pr_stage1_movie_segment_direct.cpp` / `PsxCall801C4780_ScanMovieSegmentsWithFeedback` | 237-264 | Runs `8001A324` semantics over all seven rows and exposes `80025A00` / `8001AC18(sceneEntry+0x3C,0)` as actions. |
| `src/pr/pr_stage1_movie_segment_direct.cpp` / `PsxSelectMovieSegment801C4DC4FromScan` | 280-292 | Selects row index 2 (`sceneEntry+0x6C`) from a scan result. |
| `src/pr/pr_stage1_movie_text_outer_loop_direct.cpp` / `BuildMovieTextOuterLoopInputFromScan801C455C` | 510-522 | Can feed selected scan row fields into `801C455C`, but no current caller reaches it from `pr_scenes.cpp`. |

## Where to attach next

1. Attach `MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(1)` at `PrScn1::Fn1`, immediately around the current `InitScene801C7284(...)` call. This matches the exported `801C7284` behavior: cache scene entry, derive timing fields, run `8001A324` over rows 0..6, then proceed to later lifecycle/resource helpers.
2. Store the materialized/scanned table beside or inside `s_stage1LifecycleScene801C7284`. The important rule is that `801C81EC` receives row-backed selections, not just `loaderPresent/slotIndex`.
3. Feed `801C81EC` row selections by offset: `0x6C -> row 2 MOVIE1`, `0x9C -> row 3 stage runner/XA`, `0xCC/0xFC -> rows 4/5 clear-tail STR`. Host adapters may translate a known row/path to filesystem playback, but they must not infer row fields from playback state.
4. Keep `StrPlayer`, `PrTransition`, SFX, SQEVS, and save UI execution in `pr_scenes.cpp` as platform side effects. Move only authority for row identity, row timing fields, and lifecycle sequencing to the direct layer.

## Old shell code later removable

Remove only after the row-backed `801C7284 -> 801C81EC` path is the single authority:

- `BuildStage1LifecycleInitSceneInput801C7284` path-presence loader construction can be deleted or reduced to non-authoritative diagnostics; it should not be the source of row presence.
- `TryResolveCurrentSceneLoaderSlotPath` should no longer be used for Stage1 lifecycle rows. A thin host path adapter may remain only if it translates a direct row/path identity to a platform path.
- Initial MOVIE1 hardcoded path/existence in `BuildStage1Movie1HostFeedback` and `ExecuteStage1Movie1HostActions` should be replaced by row/action-provided playback identity.
- `PrStage1ScenesDirect` clear-post-run FSM (`PrimeClearPostRun`, `TickClearPostRun`, result-movie slot/mode state) can be deleted once `801C81EC` emits the terminal clear-tail/post-movie/status actions directly.
- `Stage1ClearPostRunAdapterState`, `PrimeStage1ClearPostRunAdapter`, and `TickStage1ClearPostRunAdapter` can be removed after their platform side effects are rehomed under the `801C81EC` executor action path.
- Duplicate result-movie path caches (`resultMoviePathResolved`, `resultMoviePath`, `resultMovieVisualActive`) should be removed once host block state is driven by row-backed lifecycle actions.

Do not delete these in the movie-segment cutover:

- `StrPlayer::Play/Update/Stop` execution, SQEVS calls, transition start/poll, SFX calls, and save UI calls. These are platform/HAL side effects.
- MOVIE1 draw-plan and subtitle rendering adapters. They consume runtime state; they are not row-table sources.
- CD lookup feedback boundaries. `CdlFILE.pos/size` still must enter as explicit `8001A2B0/800381F8` feedback, not from Win STR duration or frame counters.

## Static decision

The next direct connection should be scene-init first, not MOVIE1-host first:

```text
PrScn1::Fn1
  -> InitScene801C7284(sceneIndex=1, static scene-entry rows)
  -> MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(1)
  -> PsxCall801C4780_ScanMovieSegments[WithFeedback]
  -> persist row table / scan result

PrScn1::Fn2 / TickStage1Scene1_801C81EC_ActionExecutor
  -> Step801C81EC consumes row-backed offsets
  -> host blocks execute playback/transition/SFX only
```

This makes `pr_scenes.cpp` thinner in the intended direction: one scene init call, one per-frame lifecycle call, and platform action execution only.
