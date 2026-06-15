# scene entry glyph map closure: 80017FC4 / dword_800491C4 / 80019284

Scope: static-only整理 for `80017FC4` glyph mapping and its impact on
`80019284` high-score record output. No build, no runtime replay, no `src`
changes.

Authority used:

- `docs/项目规则.md`: IDA pseudocode/disassembly is the authority; replay cannot
  be used as implementation authority.
- Existing IDA export:
  `tools/auto_port/output/stage1_p0_save_ui_mcp_20260509/decompile_save_ui_helpers_20260509.txt`.
- Existing scene-entry export:
  `docs/stage1_decomp/scene_entry_800191e4_80019284_20260512.md`.
- Fresh current-IDB IDAPython probe for `80017FC4` disassembly and IDA database
  bytes only. This was used only to disambiguate `dword_800491C4` as a table
  address.

## 80017FC4 behavior

IDA pseudocode:

```c
_BYTE *__fastcall sub_80017FC4(_BYTE *a1, unsigned __int8 *a2)
{
  int v4; // $s0
  _BYTE *v5; // $s2
  unsigned __int8 *v6; // $s1
  bool v7; // dc
  _BYTE *result; // $v0
  int v9; // $v0

  v4 = 0;
  if ( a2 )
  {
    v5 = a1;
    v6 = a2;
    while ( 1 )
    {
      v7 = v4 >= strlen(a2);
      result = &a1[v4];
      if ( v7 )
        break;
      v9 = *v6++;
      ++v4;
      *v5++ = *((_BYTE *)dword_800491C4 + v9);
      if ( !a2 )
        goto LABEL_5;
    }
  }
  else
  {
LABEL_5:
    result = &a1[v4];
  }
  *result = 0;
  return result;
}
```

Disassembly points that matter:

```asm
80017FE8: beqz    $s3, loc_80018034      ; if src == NULL, skip copy
80017FF8: jal     strlen                  ; strlen(src) each loop
80018000: slt     $v0, $s0, $v0           ; offset < strlen(src)
8001800C: lbu     $v0, 0($s1)             ; source byte, unsigned
80018014: li      $at, dword_800491C4     ; table base address, not a pointer load
8001801C: addu    $at, $v0                ; table + source byte
80018020: lbu     $v0, 0($at)             ; mapped glyph byte
80018028: sb      $v0, 0($s2)             ; dest[offset] = mapped byte
80018038: sb      $zero, 0($v0)           ; terminator at dest[offset]
```

Input/output contract:

- `a1`: destination byte buffer.
- `a2`: source NUL-terminated byte string. `NULL` source is accepted and only
  writes `dest[0]=0`.
- Loop condition is `offset < strlen(source)`, so the source terminator is never
  mapped through the table.
- For each nonzero byte before the terminator:
  `dest[offset] = *(u8 *)(0x800491C4 + source[offset])`.
- After the copied range, `dest[offset] = 0`.
- Return value is `&dest[offset]`, i.e. pointer to the written terminator.
- Mapping is byte-to-byte. It is not a 2-byte Shift-JIS copy. The wider
  `0x80049124` region contains a preceding encoding-looking table, but the
  function uses the 256-byte byte map rooted at `0x800491C4`.

Conclusion: for implementation naming, treat this as an ASCII/source-byte to
PSX glyph-byte map. More precise name: `kAsciiToPsxGlyphByte800491C4`.

## 800491C4 table

Disassembly confirms `dword_800491C4` is used as an immediate table address.
Do not implement it as `ReadU32(0x800491C4)`; the first bytes at the address are
valid table entries.

Table address:

- Base: `0x800491C4`.
- Entry width: `u8`.
- Index: unsigned source byte `0..255`.
- Required full range for byte-exact implementation: `0x800491C4..0x800492C3`
  inclusive.

Current IDA data bytes for the required 256-byte map:

```text
800491C4: 90 82 91 82 92 82 93 82 94 82 95 82 96 82 97 82
800491D4: 98 82 99 82 9A 82 6F 81 62 81 70 81 50 81 40 81
800491E4: 20 21 22 23 24 7C 26 27 3C 3E 2A 2B 1E 2D 1F 2F
800491F4: 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
80049204: 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F
80049214: 50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 7E 5F
80049224: 60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F
80049234: 70 71 72 73 74 75 76 77 78 79 7A 2A 7C 7D 7E 7F
80049244: 00 00 00 00 00 00 00 00 00 00 00 00 E8 90 04 80
80049254: 04 00 0E 00 39 00 00 00 00 00 00 00 00 00 00 00
80049264: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
80049274: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
80049284: 06 00 03 00 13 00 00 00 00 00 00 00 00 00 00 00
80049294: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
800492A4: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
800492B4: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

Important caveat:

- The older raw-file byte slice
  `stage1_p0_save_ui_byte_exact_data_20260509/name_glyph_state_800490e8_80049324.bin`
  does not match the current IDA data view at `0x800491C4`. For this task, do
  not use that raw slice as the table source. Use current IDA database bytes or
  the existing `decompile_save_ui_helpers_20260509.txt` data block.

## Impact on 80019284

Existing IDA pseudocode for `80019284(a1)`:

```c
do
{
  v4 = 0;
  v5 = 16 * v1 + 20;
  v6 = &dword_80049278[4 * v1];
  v7 = v2 << 6;
  v8 = (const char *)(v12 + v3);
  do
  {
    v9 = *(_DWORD *)(v12 + v7 + 4888);
    if ( v9 > 0 )
    {
      sub_8004800C(v11, "%4d %-3.3s", v9, v8);
      sub_80017FC4((char *)dword_80049278 + v5, v11);
    }
    else
    {
      *((_BYTE *)v6 + 20) = 0;
    }
    v8 += 16;
    v7 += 16;
    ++v4;
    v5 += 16;
    v6 += 4;
    ++v1;
  }
  while ( v4 < 3 );
  ++v2;
  v3 += 64;
}
while ( v2 < 6 );
```

High-score output facts:

- Output base: `dword_80049278`.
- Records: 18 cells, laid out as 6 rows x 3 columns.
- Record stride: 16 bytes.
- First record offset from output base: 20 bytes.
- Source score offset:
  `a1 + 4888 + row * 64 + col * 16`, read as signed 32-bit little-endian.
- Source name offset:
  `a1 + 4876 + row * 64 + col * 16`.
- Positive score path formats a temporary ASCII/source-byte string with
  `"%4d %-3.3s"` and then calls `80017FC4(output + 20 + cell * 16, temp)`.
- Non-positive score path writes only `output[20 + cell * 16] = 0`; it does not
  clear the rest of the 16-byte record in this function.

Effect of the glyph map:

- `80019284` does not write formatted ASCII directly to the final record table.
  It writes PSX glyph bytes produced by `80017FC4`.
- For the score/name format's common bytes, many entries are identity:
  space `0x20 -> 0x20`, digits `0x30..0x39 -> 0x30..0x39`, uppercase
  `0x41..0x5A -> 0x41..0x5A`, lowercase `0x61..0x7A -> 0x61..0x7A`.
- Some printable bytes are not identity, e.g. `0x25 -> 0x7C`,
  `0x28 -> 0x3C`, `0x29 -> 0x3E`, `0x2C -> 0x1E`, `0x2E -> 0x1F`,
  `0x5E -> 0x7E`, `0x7B -> 0x2A`.
- Therefore an ASCII-carrier implementation is only conditionally equivalent
  for the limited bytes actually present. It should not mark the final table as
  PSX glyph bytes known unless it has applied the complete `0x800491C4` map.

## Main-thread implementation suggestions

Suggested constants:

```cpp
static constexpr uint32_t kFn80017FC4 = 0x80017FC4u;
static constexpr uint32_t kHiScoreOutputBase80049278 = 0x80049278u;
static constexpr uint32_t kAsciiToPsxGlyphByteBase800491C4 = 0x800491C4u;
static constexpr size_t kAsciiToPsxGlyphByteCount800491C4 = 256u;
```

Suggested data:

```cpp
static constexpr uint8_t kAsciiToPsxGlyphByte800491C4[256] = {
    // bytes from 0x800491C4..0x800492C3, exactly as above
};
```

Suggested helper:

```cpp
struct GlyphCopy80017FC4Result {
    bool known = false;
    size_t bytesWrittenBeforeTerminator = 0;
};

GlyphCopy80017FC4Result Apply80017FC4GlyphMap(
    uint8_t* dst,
    size_t dstCapacity,
    const uint8_t* nulTerminatedSource,
    const uint8_t glyphMap[256]);
```

Required helper semantics:

- If source is `nullptr`, write `dst[0]=0` when capacity allows and return known.
- Stop before source NUL; do not map `0x00` as a data byte.
- For every source byte before NUL, write `glyphMap[sourceByte]`.
- Write one terminator byte after the copied range.
- If the destination capacity cannot contain mapped bytes plus terminator, mark
  unknown/gap instead of truncating silently. `80019284` records have 16 bytes.

Suggested `Call80019284Result80015788` field updates:

- Keep `formattedAscii` or `formattedSourceBytes` as debug/source-carrier data.
- Add or use final per-cell glyph output bytes, e.g.
  `uint8_t glyphBytes[16]` plus `glyphBytesKnown`.
- Set `tablePsxGlyphBytesKnown = true` only after every positive-score cell has
  successfully run the table map and every non-positive cell has written its
  first zero byte according to the pseudocode.
- Remove or stop setting `missing80017FC4GlyphMap` once the 256-byte table is
  embedded/exported into `pr_scene_entry_direct`.

Minimal implementation path:

1. Embed `kAsciiToPsxGlyphByte800491C4[256]` in `pr_scene_entry_direct.cpp` or a
   shared static PSX data source.
2. Replace the current ASCII carrier copy in `PsxCall80019284_BuildHiScoreRecords80015788`
   with `Apply80017FC4GlyphMap(out.tableBytes + destOffset, 16, formattedAscii,
   kAsciiToPsxGlyphByte800491C4)`.
3. Preserve `formattedAscii` for inspection, but make `tableBytes` the mapped
   PSX glyph-byte output.
4. After the map is applied, `80019284` output can be promoted from
   `tableAsciiKnown` to `tablePsxGlyphBytesKnown`.

