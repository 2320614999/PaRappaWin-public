# MOVSEG feedback bridge next dependency map - 2026-05-10

Scope: documentation only. No code edits, no build, no test.

## Field flow

```text
SCUS static scene table
  sceneEntry = 0x8005474C + sceneIndex * 0x16C
  row[i] = sceneEntry + 0x0C + i * 0x30, i=0..6
    |
    v
801C4780
  scans all seven rows and calls 8001A324(row)
    |
    v
8001A324
  skips when row+0x0C == 1 or row+0x00 == 0
  calls 8001A2B0(row+0x10)
    |
    v
8001A2B0
  retries up to 4 times, calling 800381F8(row+0x10, path/name input)
    |
    v
800381F8
  CD lookup HAL fills CdlFILE payload at row+0x10..0x27 on success
```

After `8001A2B0` succeeds, `8001A324` computes:

```text
row+0x28 = 80036A78(row+0x10)
row+0x0C = 1
row+0x2C = row+0x28 + (row+0x14 >> 11)
```

Then `801C4780` continues with `80025A00()` and
`8001AC18(sceneEntry+0x3C, 0)`. It does not source or fabricate the row table.

## Row field owner matrix

| Field | Owner | Notes |
|---|---|---|
| `row+0x00` | SCUS static scene-entry row | Path pointer / row identity. `8001A324` treats zero as disabled row. |
| `row+0x0C` | SCUS static initial value, then `8001A324` mutation | Initial rows export as `0`; successful lookup sets it to `1`. |
| `row+0x10` | `800381F8` runtime fill inside CdlFILE payload | First CD clock/name payload dword. Static export is zero-reserved storage. |
| `row+0x14` | `800381F8` runtime fill inside CdlFILE payload | Size dword used by `8001A324` as `(size >> 11)` for end LBA. |
| `row+0x18` | `800381F8` runtime fill inside CdlFILE payload | Remaining CdlFILE/name payload; consumer-owned, not STR-state-derived. |
| `row+0x28` | `8001A324` runtime mutation | Start LBA from `80036A78(row+0x10)`. Do not infer from Win playback state. |
| `row+0x2C` | `8001A324` runtime mutation | End LBA from `row+0x28 + (row+0x14 >> 11)`. Do not infer from STR duration/filesystem metadata. |

Related static fields not in the minimum matrix:

| Field | Owner | Notes |
|---|---|---|
| `row+0x04` | SCUS static scene-entry row metadata | Opaque per-row metadata such as `0x005A0001`. |
| `row+0x08` | SCUS static scene-entry row end bias | Static, scene/row-specific. Must not be defaulted from STR runtime state. |

## HAL boundary

Stage1 CD HAL is only a feedback adapter for the direct-port scan:

```text
PrStage1LoaderCdHal::LookupFeedback800381F8
  -> ProbeCompletionFeedback8001A2B0
  -> BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(...)
  -> MovieSegmentRowInitFeedback8001A324
```

The adapter may pass through explicit successful lookup facts, especially
`CdlFILE.pos` and `CdlFILE.size`. It must not invent missing `CdlFILE` bytes,
scene-entry rows, `row+0x08`, `row+0x28`, or `row+0x2C`.

Generic MOVSEG must not depend on Stage1 HAL. The portable authority boundary is:

```text
SCUS static scene table + PSX MOVSEG control flow
  -> generic MOVSEG direct-port core
  -> thin per-platform/per-stage lookup feedback adapter
```

So Stage1 HAL can feed `8001A2B0/800381F8` results into the generic core, but the
generic core cannot call back into Stage1-specific filesystem, STR playback, or
loader state to decide MOVSEG fields.

## Cutover gate for Stage1

Stage1 `8001A324/801C4780` can be cut over to `PrMovieSegmentDirect` only when
all of these are true:

1. `PrMovieSegmentDirect` materializes all seven rows from
   `sceneEntry+0x0C+i*0x30`, including static `+0x00/+0x04/+0x08/+0x0C` and
   zero-reserved CdlFILE storage.
2. The lookup feedback path can provide real PSX-equivalent `CdlFILE.pos` and
   `CdlFILE.size` for successful `800381F8` results, not just host file
   existence.
3. The direct-port core owns the `8001A324` mutations for `+0x0C/+0x28/+0x2C`
   and preserves the four-try `8001A2B0` success/failure semantics.
4. `801C4780` remains a table-wide seven-row scan and still exposes the
   post-scan actions `80025A00()` and `8001AC18(sceneEntry+0x3C, 0)` at the
   boundary.
5. Stage1-specific code is reduced to a thin adapter that supplies scene-entry
   base and lookup feedback; it contains no MOVSEG field logic, no row shortcut
   for `sceneEntry+0x6C`, and no STR playback-derived fallback fields.

Until those gates are closed, Stage1 may bridge feedback into MOVSEG for
documentation or migration, but it should not replace generic MOVSEG authority.
