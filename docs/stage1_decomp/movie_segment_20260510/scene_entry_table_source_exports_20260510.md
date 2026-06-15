# Scene entry table source exports for MOVSEG rows

Scope: MOVSEG-source-export static audit only. No `src` edits, no build, no
test. Target is the source of `sceneEntry + 0x0C + i * 0x30` rows, not STR
playback-state inference.

## IDA exports written

- `ida_movseg_source_export_20260510.json`
  - `801C4780`, `801C4DC4`, `8001A324`, `8001A2B0`, `800381F8`
  - xrefs to `dword_8006ED98+0x20` / `0x8006EDB8`
- `ida_scene_entry_lifecycle_source_20260510.json`
  - `801C4260`, `80015D18`, `8001AC18`, lifecycle entry table around
    `0x80048D28`
- `ida_scene_entry_static_rows_20260510.json`
  - static scene-entry rows from `0x8005474C + sceneIndex * 0x16C`
- `ida_stage1_lifecycle_raw_7284_81ec_20260510.json`
  - raw current-IDB references for `801C7284` / `801C81EC`

## Source conclusion

The seven MOVSEG rows are SCUS static table data, not overlay/lifecycle
constructed data.

`80015D18` is the main scene loop. It computes the current scene-entry pointer
as:

```text
sceneEntry = 0x8005474C + sceneIndex * (91 dwords)
           = 0x8005474C + sceneIndex * 0x16C
```

Then it calls:

```text
8001A324(sceneEntry + 0x0C)          ; preload row 0 / COMOD
800154B0(sceneEntry + 0x0C, 0)
entryInit = *(0x80048D2C + sceneIndex * 0x0C)
entryMain = *(0x80048D30 + sceneIndex * 0x0C)
entryInit(sceneEntry, sceneIndex)
entryMain(sceneIndex)
```

For the `801C4260 -> 801C4780 -> 801C4DC4` lifecycle group, `801C4260` stores
that same caller-supplied `sceneEntry` into `dword_8006ED98+0x20`
(`0x8006EDB8`). `801C4780`, `801C455C`, `801C4894`, and `801C4DC4` read it;
they do not own the static row bytes.

## Lifecycle entry table

IDA export around `0x80048D28` shows triplets indexed by `sceneIndex`:

| sceneIndex | init/prologue | entry init | entry main | sceneEntry base |
|---:|---|---|---|---|
| 0 | `801C5B14` | `801C4260` | `801C4DC4` | `0x8005474C` |
| 1 | `801CA3BC` | `801C7284` | `801C81EC` | `0x800548B8` |
| 2 | `801C97EC` | `801C657C` | `801C74E4` | `0x80054A24` |
| 3 | `801C998C` | `801C6918` | `801C7880` | `0x80054B90` |
| 4 | `801CB348` | `801C7D20` | `801C8C88` | `0x80054CFC` |
| 5 | `801C9310` | `801C60B0` | `801C7030` | `0x80054E68` |
| 6 | `801C9EA0` | `801C6AC4` | `801C7A2C` | `0x80054FD4` |

Current IDA has `801C7284` / `801C81EC` as `loc_*`, not functions. Raw
disassembly at those addresses is not reliable in this IDB without retyping or
using the overlay-specific function boundary. That does not block the row table
source: the caller side (`80015D18`) and static scene table source are already
visible.

## Owner matrix

| Field / edge | PSX source owner | Export evidence | Current status |
|---|---|---|---|
| `sceneEntry` base | Main scene loop `80015D18` selects `0x8005474C + sceneIndex * 0x16C` | `80015D18` uses `off_8005474C` / `off_80054758` with `91 * sceneIndex` dword stride | SCUS static table base. |
| `dword_8006ED98+0x20` / `0x8006EDB8` | Current scene-entry pointer cache for the `801C4260` group | only write found: `801C42D0 sw $s0, dword_8006ED98+0x20`; reads in `801C455C`, `801C4780`, `801C4894`, `801C4DC4` | Cache/alias, not table storage. |
| Seven row shape | Static scene entry at `sceneEntry+0x0C`, row stride `0x30`, count `7` | `801C4780` loops `s0=0x0C`, calls `8001A324(sceneEntry+s0)`, increments `s0 += 0x30`, stops at `7` | Confirmed. |
| `row+0x00 pathPtr` | SCUS static scene-entry row | `ida_scene_entry_static_rows_20260510.json` resolves path strings for all scenes | Static table field. |
| `row+0x04` | SCUS static scene-entry row metadata | values include `0`, `0x005A0001`, `0x007F0001` depending on row | Static opaque field. |
| `row+0x08` | SCUS static scene-entry row end bias | values are scene/row specific, e.g. Stage1 row 2 is `0`, scene 2 rows 4/5 are `75`, scene 5 row 2 is `-1500`, scene 6 row 2 is `150` | Static table field; must not be defaulted from STR state. |
| `row+0x0C` initial loaded state | SCUS static scene-entry row, later mutated by `8001A324` | all exported initial rows are `0`; `8001A324` writes `1` after successful lookup | Static initial value plus loader mutation. |
| `row+0x10..0x27 CdlFILE` initial region | SCUS static row reserves CdlFILE slot; `8001A2B0 -> 800381F8` fills it | exported initial `pos/size/name` are zero for all rows; `800381F8` copies six dwords to `row+0x10` | Initial static zero area, runtime-filled by CD lookup. |
| `row+0x28 startLba` | `8001A324` runtime mutation from `80036A78(row+0x10)` | exported initial `0`; `8001A324` writes `row+0x28` | Loader-derived runtime field. |
| `row+0x2C endLba` | `8001A324` runtime mutation from `startLba + (size >> 11)` | exported initial `0`; `8001A324` writes `row+0x2C` | Loader-derived runtime field. |
| `801C4780` | Scanner/mutator for all seven rows | reads `dword_8006ED98+0x20`, calls `8001A324` for rows 0..6, then `80025A00`, then `8001AC18(sceneEntry+0x3C,0)` | It consumes the static table; it is not the source. |
| `801C4DC4` | Consumer for opening/title movie rows in scene 0 lifecycle | disasm passes `sceneEntry+0x6C` to `801C44E0/801C455C`, then `sceneEntry+0x9C` to `801C4894` | Consumer only. |

## Stage1 row matrix (`sceneIndex=1`)

`sceneEntry base = 0x800548B8`. Rows start at `0x800548C4`.

| row | row addr | `+0x00 pathPtr` | path | `+0x04` | `+0x08` | `+0x0C` initial | CdlFILE initial | static vs runtime |
|---:|---|---|---|---:|---:|---:|---|---|
| 0 | `0x800548C4` | `0x80011784` | `\S1\COMOD1.BIN;1` | `0x00000000` | `0` | `0` | zeroed `pos/size/name` | SCUS static row; CdlFILE filled by lookup |
| 1 | `0x800548F4` | `0x80011770` | `\S1\COMPO01.INT;1` | `0x00000000` | `0` | `0` | zeroed `pos/size/name` | SCUS static row; CdlFILE filled by lookup |
| 2 | `0x80054924` | `0x8001175C` | `\SS\MOVIE1.STR;1` | `0x005A0001` | `0` | `0` | zeroed `pos/size/name` | SCUS static row; CdlFILE filled by lookup |
| 3 | `0x80054954` | `0x80011748` | `\S1\STAGE1.XA1;1` | `0x005A0001` | `0` | `0` | zeroed `pos/size/name` | SCUS static row; CdlFILE filled by lookup |
| 4 | `0x80054984` | `0x80011734` | `\S1\XMOVIE1.STR;1` | `0x005A0001` | `0` | `0` | zeroed `pos/size/name` | SCUS static row; CdlFILE filled by lookup |
| 5 | `0x800549B4` | `0x80011734` | `\S1\XMOVIE1.STR;1` | `0x005A0001` | `0` | `0` | zeroed `pos/size/name` | SCUS static row; CdlFILE filled by lookup |
| 6 | `0x800549E4` | `0x80011720` | `\S1\ZCOMPO.INT;1` | `0x005A0001` | `0` | `0` | zeroed `pos/size/name` | SCUS static row; CdlFILE filled by lookup |

## MOVIE1 consumer path

For the `801C4DC4` group, opening STR uses:

```text
sceneEntry = *(dword_8006ED98 + 0x20)
801C44E0(sceneEntry + 0x6C, 0)
801C455C(sceneEntry + 0x6C, &801C3640, 0)
```

`sceneEntry+0x6C` is row index 2:

```text
0x0C + 2 * 0x30 = 0x6C
```

For Stage1 static data, row index 2 is `\SS\MOVIE1.STR;1` at
`0x80054924`. Its static `+0x08` is `0`, static `+0x0C` is `0`, and its
initial CdlFILE area is zero until `8001A2B0/800381F8` fills it.

## Direct-port implication

`MaterializeSceneEntryMovieSegments801C4780(...)` should not infer row values
from Win STR state. The correct source boundary is:

```text
SCUS static scene table
  0x8005474C + sceneIndex * 0x16C
  rows: +0x0C + i*0x30
    -> pathPtr/+04/+08/+0C/initial CdlFILE zero area
8001A324
  -> fills CdlFILE via 8001A2B0/800381F8
  -> writes +0x0C = 1, +0x28, +0x2C
801C4780
  -> runs 8001A324 across seven rows
801C4DC4 / 801C455C
  -> consumes row 2 at sceneEntry+0x6C
```

So the row table is in the SCUS static table, while `801C4780` and lifecycle
entry functions only select, cache, initialize, and consume it.

## If more export is needed

The minimum next exports are:

1. Force/decode the correct function boundaries for the lifecycle table entries
   that IDA currently names `loc_801C7284` and `loc_801C81EC`, or use the
   overlay-specific IDB/Ghidra path that already recognizes them.
2. Export `800154B0`, because `80015D18` calls it immediately after preloading
   `sceneEntry+0x0C`; this may explain additional row-0/load-handle side
   effects.
3. Export a narrower `8001A8F0` summary if the `row+0x10..0x27` CdlFILE payload
   consumer needs to be connected past `8001AC18`.

None of those are required to answer the table-source question: the seven
`0x30` rows come from `0x8005474C + sceneIndex * 0x16C + 0x0C`, not from STR
runtime state or overlay/lifecycle construction.
