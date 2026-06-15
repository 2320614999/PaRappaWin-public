# MOVSEG loader side-effect exports

Scope: Task D static closure only. No `src` edits, no build, no test.

## Inputs and raw exports

- Rule file checked: `docs/项目规则.md`.
- Prior notes checked:
  - `lifecycle_next_exports_20260510.md`
  - `scene_entry_table_source_exports_20260510.md`
  - `scene_entry_row_source_matrix_20260510.md`
  - `cd_lookup_feedback_bridge_20260510.md`
- Current IDA MCP IDB confirmed by `get_metadata`:
  `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`
- New raw exports:
  - `ida_loader_side_effect_exports_20260510.json`
  - `ida_loader_side_effect_raw_20260510.txt`
  - `ida_loader_side_effect_helpers_20260510.json`
  - `ida_loader_side_effect_helpers_20260510.txt`

One first IDAPython batch failed before exporting because this IDA build did
not expose `ida_ida.inf_get_input_file_path`. The corrected batch used
`ida_nalt.get_input_file_path()` and completed.

## Closed conclusion

The MOVSEG loader side effects split into three distinct paths:

```text
80015D18 scene loop:
  row0 = 0x8005474C + sceneIndex*0x16C + 0x0C
  8001A324(row0)
  800154B0(row0, 0)        ; ABI passes a1=0, wrapper ignores it
    -> 8001ACF8(row0, dword_801C3870 / off_80010000)
  entry prologue/init/main from 0x80048D28 table

801C4780 scene-entry scan:
  for row 0..6:
    8001A324(sceneEntry + 0x0C + row*0x30)
  80025A00()
  8001AC18(sceneEntry + 0x3C, 0)
```

So row0 overlay loading and `sceneEntry+0x3C` completion feedback are not the
same side effect. Row0 is the COMOD overlay load edge in the main scene loop.
`sceneEntry+0x3C` is row1, the COMPO/INT load edge after the seven-row
`801C4780` scan.

## `80015D18` / row0 overlay load

`80015D18` computes row0 from `off_80054758`, i.e. `sceneEntry+0x0C`, using
the same `91 dwords == 0x16C` scene-entry stride already documented.

Key disassembly:

```text
80015E00  li      $v1, off_80054758
80015E08  addu    $v0, $v1
80015E0C  move    $a0, $v0
80015E10  jal     sub_8001A324

80015E40  li      $v1, off_80054758
80015E48  addu    $v0, $v1
80015E4C  move    $a0, $v0
80015E50  move    $a1, $zero
80015E54  jal     sub_800154B0
```

`800154B0` stores both incoming args, but reloads only arg0 and replaces arg1
with `off_80010000` before calling `8001ACF8`:

```c
int __fastcall sub_800154B0(int a1)
{
  return sub_8001ACF8(a1, dword_801C3870);
}
```

```text
800154C8  lw      $a0, 0x10+arg_0($fp)
800154CC  lw      $a1, off_80010000
800154D4  jal     sub_8001ACF8
```

`8001ACF8(row0, dest)` then re-runs `8001A324(row0)`, copies the row CdlFILE
area (`row+0x10..0x27`) to a local six-dword buffer, computes
`sectorCount = (size + 2047) >> 11`, seeks via `8001A89C(cdl, 0)`, and reads
with `8001A818(dest, sectorCount, firstAttemptFlag)`. It retries up to four
times and returns true only when the read count matches the expected sector
count.

Implication: `800154B0` is a row0 overlay transfer wrapper. It does not build
MOVSEG rows and does not provide the seven-row scan source. Its side effect is
loading/copying the row0 COMOD payload to the overlay destination.

## `801C4780` / seven-row scan and row1 feedback

`801C4780` reads the cached scene-entry pointer at `dword_8006ED98+0x20`
(`0x8006EDB8`), starts at offset `0x0C`, increments by `0x30`, and calls
`8001A324` seven times:

```text
801C4790  li      $s0, 0xC
801C4798  lw      $a0, dword_8006ED98+0x20
801C47A0  addiu   $s1, 1
801C47A4  jal     sub_8001A324
801C47A8  addu    $a0, $s0
801C47AC  slti    $v0, $s1, 7
801C47B0  bnez    $v0, loc_801C4798
801C47B4  addiu   $s0, 0x30
```

After the seven calls complete, it executes:

```text
801C47B8  jal     sub_80025A00
801C47C0  lw      $a0, dword_8006ED98+0x20
801C47C8  move    $a1, $zero
801C47CC  jal     sub_8001AC18
801C47D0  addiu   $a0, 0x3C
```

`sceneEntry+0x3C` is row1:

```text
0x0C + 1 * 0x30 = 0x3C
```

For Stage1 static rows, row1 is:

```text
0x800548F4  \S1\COMPO01.INT;1  +0x04=0  +0x08=0  +0x0C initial=0
```

`8001AC18(row1, 0)` also retries up to four times. Each attempt calls
`80025A34()`, then `8001A324(row1)`, copies the row CdlFILE area to a local
six-dword buffer, and calls:

```c
sub_8001A8F0(cdlFileCopy, firstAttemptFlag, 0)
```

It stores the helper result, breaks when the result is `1`, and returns that
result. `801C4780` itself does not branch on this return value, so the feedback
is consumed inside the loader helper loop rather than by `801C4780`'s caller.

## `8001A324` call order

The relevant `8001A324` calls occur in this order for the scene path:

| Order | Caller | Row/address expression | Purpose |
|---:|---|---|---|
| 1 | `80015D18` | `sceneEntry+0x0C` / row0 | Preload row0 CdlFILE/LBA metadata before overlay load. |
| 2 | `800154B0 -> 8001ACF8` | same row0 | Re-confirm row0 lookup before reading the COMOD overlay payload. |
| 3 | lifecycle init (`801C4260 -> 801C4780`, or Stage1 `801C7284`) | rows `0..6` | Materialize all seven scene-entry rows. |
| 4 | `801C4780 -> 8001AC18` | `sceneEntry+0x3C` / row1 | Re-confirm row1 lookup before COMPO/INT load completion path. |

`8001A324` itself owns only lookup/materialization mutation:

```text
if row+0x0C == 1: return 0
if row+0x00 pathPtr == 0: return 0
8001A2B0(row+0x10, pathPtr)
row+0x28 = 80036A78(row+0x10)
row+0x0C = 1
row+0x2C = row+0x28 + (row+0x14 >> 11)
```

This keeps the row-source boundary unchanged: row bytes still come from
`0x8005474C + sceneIndex*0x16C + 0x0C + row*0x30`; `8001A324` only fills
CdlFILE-derived runtime fields and the loaded flag.

## Direct-port implication

For the current Win-side MOVSEG work:

- Do not collapse row0 `800154B0/8001ACF8` and row1 `8001AC18` into one generic
  action. They load different scene-entry rows and occur at different points.
- `801C4780` must preserve the exact order: seven `8001A324` calls, then
  `80025A00`, then `8001AC18(sceneEntry+0x3C,0)`.
- `sceneEntry+0x3C` completion feedback should be modeled as a loader action or
  feedback edge for row1 COMPO/INT load, not as STR playback completion and not
  as the MOVIE1 row2 consumer.
- `800154B0` belongs to the main scene-loop row0 overlay load edge. It should
  not be used as a source of row table bytes.

The static side-effect boundary is therefore closed enough for the next
cutover design step: table bytes, CD lookup feedback, row materialization,
row0 overlay load, row1 COMPO/INT load, and row2 MOVIE1 consumer are separate
edges and should remain separate in Win.
