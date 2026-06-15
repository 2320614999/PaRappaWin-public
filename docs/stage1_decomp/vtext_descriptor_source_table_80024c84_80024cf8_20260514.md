# Stage1 vtext descriptor source table: 80024C84/80024CF8 (2026-05-14)

Authority:

- New narrow IDA export:
  `docs/stage1_decomp/ida_export_vtext_descriptor_callers_80024c84_80024cf8_20260514.md`
- New narrow IDA data-source export:
  `docs/stage1_decomp/ida_export_vtext_descriptor_data_sources_20260514.md`
- Scene0/Scene1 reconcile correction:
  `docs/stage1_decomp/vtext_descriptor_scene0_scene1_reconcile_20260514.md`
- Existing asymmetry note:
  `docs/stage1_decomp/text264_text268_event_stream_asymmetry_20260512.md`
- Win direct mapping:
  `src/pr/pr_stage1_vtext_direct.*`,
  `src/pr/pr_stage1_movie_text_direct.*`,
  `src/pr/pr_stage1_movie_text_outer_loop_direct.*`

## Direct xrefs

| Target | Direct caller | Call site | Argument source | Boundary |
|---|---:|---:|---|---|
| `80024C84` | `801C455C` | `801C45CC` | `a3 < 4 ? dword_800943CC + 28 * a3 : 0` | Stage1 movie/common-lyrics descriptor select. |
| `80024C84` | `801C4894` | `801C4944` | `0` | Reset-only pre-roll/menu branch; not a non-zero descriptor producer. |
| `80024CF8` | `801C455C` | `801C46A8` | `ctx` | Per-frame vtext event advance after input poll. |

Current IDA has no additional direct code xrefs to `80024C84` or `80024CF8`
in this target database. The `801C455C` path is the only current non-zero
descriptor caller for this closure, but its concrete descriptor base must be
taken from the active scene fn0 globals. The `801C5B14 -> 801C6BF8` export is
Scene0/COMOD0 evidence; Stage1/Scene1 uses `801CA3BC -> 801CE080`.

## 80024C84 descriptor field map

`80024C84(desc)` first clears secondary active text state:
`word_8008ECFA = 0`, `dword_8008ECE4 = 0`, `gp+0x37C = 0`, and
`gp+0x320 = 0`.

When `desc != 0`, it maps the 28-byte `PrVTextEventTableDesc` fields into the
main-exe vtext globals:

| PSX source | PSX sink | Meaning | Win field |
|---|---|---|---|
| `desc[0]` | `gp+0x364` / `v1[217]` | first text table pointer | `selectedFirstTextTableAddr` |
| `desc[5]` | `gp+0x370` / `v1[220]` | event entry table pointer | `selectedEntriesAddr` |
| `desc[6]` | `gp+0x36C` / `v1[219]` | entry count | `selectedEntryCount` |
| `desc[word_800916D8]` | `gp+0x374` / `v1[221]` | language-selected text table pointer | `selectedTextTableAddr` |
| reset | `gp+0x37C` / `v1[223]` | event cursor | `eventCursor = 0` |

Win currently represents the selected Stage1 descriptors with:

| Mode | Win constant | Expected entries | Role |
|---:|---:|---:|---|
| `0` | `0x801CE080` | `41` | MOVIE1/common-lyrics main table. |
| `1` | `0x801CE09C` | `6` | Short post/alternate vtext table. |
| `2` | `0x801CE0B8` | `6` | Short post/alternate vtext table. |

The Win loader reads those descriptors from COMOD1 through
`LoadFromComodSub80024C84(...)`, builds `PsxVTextDescriptorState`, then
projects it into `PrStage1VTextDirectDescInput`.

## dword_800943CC data source and scene ownership

The only current direct writer found for `dword_800943CC` is
`801C5B14 @ 801C5B60`, which writes `&off_801C6BF8`. `801C455C @ 801C4594`
then reads `dword_800943CC` before the `dword_800943CC + 28 * mode`
descriptor selection.

That writer belongs to scene index 0, not Stage1. The Scene1 / COMOD1 fn0 is
`801CA3BC`, and the COMOD1-specific export shows it writes
`dword_800943CC = &off_801CE080`. Therefore `801C6BF8` must not be used as
Stage1 common-lyrics authority.

The current IDB bytes at that writer-owned table give:

| Slot | Address | desc[0] | desc[1] | desc[2] | desc[3] | desc[4] | desc[5] entries | desc[6] count | Interpretation |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `0` | `0x801C6BF8` | `0x801C69C8` | `0x801C6A08` | `0x801C6A48` | `0x801C6A88` | `0x801C6AC8` | `0x801C6B08` | `0x0F` | Valid descriptor-shaped source for `mode=0`. |
| `1` | `0x801C6C14` | `0x00000000` | `0x00090012` | `0x000B0014` | `0x000C0015` | `0x000D0016` | `0x000E0017` | `0x000F0018` | Does not look like a 28-byte descriptor table head; treat as not closed. |
| `2` | `0x801C6C30` | `0x00100019` | `0x0011001A` | `0x00000000` | `0x0000001D` | `0x00000000` | `0x00000018` | `0x0000001E` | Does not look like a 28-byte descriptor table head; treat as not closed. |
| `3` | `0x801C6C4C` | `0x00000000` | `0x00000030` | `0x0000001C` | `0x00000000` | `0x00000048` | `0x0000001D` | `0x00000000` | Does not look like a 28-byte descriptor table head; treat as not closed. |

The Win COMOD1 constants `0x801CE080/0x801CE09C/0x801CE0B8` are not older
fallbacks here; they are the Scene1 authority derived from `801CA3BC`.
`801C6BF8` is valid Scene0 descriptor data and should only be used by a
Scene0/COMOD0 path.

## 80024CF8 advance field map

`80024CF8(ctx)` has two timer lanes, but only the secondary lane has a
non-zero event-stream producer in the exported closure.

| PSX source/sink | Behavior | Win field |
|---|---|---|
| `word_8008ECF8`, `dword_8008ECE0`, `ctx+0x108` | Primary lane only decrements and clears; no non-zero producer in this closure. | No live text264 producer should be inferred. |
| `word_8008ECFA`, `dword_8008ECE4`, `ctx+0x10C` | Secondary lane decrements, clears, and receives event-stream text. | `activeDurationFramesRemaining`, `activeTextPsxAddr`, `activeTextPtr`, `text268Mirror*`. |
| `gp+0x37C < gp+0x36C` | Cursor bounds check. | `eventCursor < track->lines.size()`. |
| `gp+0x370 + 16 * cursor` | Current 16-byte event row. | `PrScn0::MovieSubtitleLine`. |
| row timecode vs `ctx+4/6/7` | Advance when event timecode is due. | `PrStage1VTextDirectPackFrame30AsPsxTimecodeKey(...)`. |
| `row[3 + word_800916D8]` | Language text index. | `line.textIndex[languageIndex]`. |
| `*(gp+0x374 + 4 * textIndex)` | Selected text pointer. | `lineMeta.textAddrs[languageIndex]` / decoded string pointer. |

## Current Win chain

The current Win direct chain for this closure is:

`PrScn1::SyncStage1Scene1FrameDriverTextProducers`
-> `TickStage1MovieTextOuterLoopFromHostFacts801C455C`
-> `TickStage1MovieTextOuterLoopFromSceneFrame801C455C`
-> `StepMovieTextOuterLoopSub801C455C`
-> `AdvanceCommonLyricsSub80024CF8`
-> `PrStage1VTextDirectAdvanceSub80024CF8`

`BeginPlayAndWaitSub801C77C0(...)` performs the `80024C84` descriptor select
for known modes, and `StepMovieTextOuterLoopSub801C455C(...)` records the
matching `SelectDescSub80024C84` action during warmup.

## Remaining code-facing gaps

- `resetScratch200` is still reset-only in Win; no non-reset PSX source was
  identified in this narrow closure.
- `memory164`, `word800916D8`, `sub8001A750Result`, `dword801C9554EqualsOne`,
  and workbase/result fallback facts remain outer-loop input-source gaps when
  not supplied by the scene/lifecycle host facts.
- `dword_800943CC` is scene-owned. Scene0 writer `801C5B14` supplies
  `0x801C6BF8`; Scene1 writer `801CA3BC` supplies `0x801CE080`. The remaining
  gap is not a replacement decision; it is whether the direct path should be
  parameterized by active scene globals instead of Stage1 constants.
- These gaps do not justify a new typed carrier by themselves. If runtime
  verification later fails on this path, the next step is to provide the real
  facts source for those fields or widen the direct-port unit around the
  relevant caller, not to infer success from rendered text or COMOD readability.

## Boundary decision

- `text268` is the active vtext event-stream sink for this closure.
- `text264` remains clear/decrement-only in `80024C84/80024CF8/80024E98` and
  must not be treated as a missing non-zero producer inside this unit.
- `801C4894 -> 80024C84(0)` is reset-only and should not be used as descriptor
  source authority for Stage1 common lyrics.
