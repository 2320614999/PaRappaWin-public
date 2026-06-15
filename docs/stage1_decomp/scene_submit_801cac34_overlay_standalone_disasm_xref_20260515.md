# Scene Submit 801CAC34 Overlay Standalone Disasm/Xref - 2026-05-15

Authority boundary: this supplement uses the existing IDA Hex-Rays pseudo C
dump from `S1/COMOD1.BIN.i64` as semantic authority and adds a raw
COMOD1 disassembly/xref table for the missing standalone evidence window.
It does not authorize renderer/backend deletion or Win-side packet defaults.

## Source

- Binary: `E:\game\PSgame\parappa the rapper\S1\COMOD1.BIN`
- Existing IDA pseudo C: `docs/stage1_decomp/scene_submit_draw_exports_20260512/decompile_801cac34.txt`
- Existing manifest: `docs/stage1_decomp/scene_submit_draw_exports_20260512/comod1_manifest.json`
- Overlay base: `0x801C3870`
- Target: `0x801CAC34` (`PrStageRunner_DrawBackground`), file offset `0x73C4`
- Disassembled range: `0x801CAC34..0x801CB190`

## Xrefs To 801CAC34

- `801CC1DC` file+`0x896C` `jal` -> `801CAC34`

## Direct Calls From 801CAC34

- `801CAC90` -> `8001B084` (sub_8001B084 / range submit -> 800428B0)
- `801CACC4` -> `8001B084` (sub_8001B084 / range submit -> 800428B0)
- `801CACF0` -> `8001B084` (sub_8001B084 / range submit -> 800428B0)
- `801CAD2C` -> `8001B084` (sub_8001B084 / range submit -> 800428B0)
- `801CAD74` -> `8001B084` (sub_8001B084 / range submit -> 800428B0)
- `801CAD9C` -> `8001B084` (sub_8001B084 / range submit -> 800428B0)
- `801CAE0C` -> `8001B084` (sub_8001B084 / range submit -> 800428B0)
- `801CAE40` -> `8001B084` (sub_8001B084 / range submit -> 800428B0)
- `801CAE94` -> `8003A3DC` (sub_8003A3DC)
- `801CAEE4` -> `8003B70C` (sub_8003B70C)
- `801CAF50` -> `801CA634`
- `801CAFA8` -> `8001B590` (sub_8001B590 / FastSprite submit wrapper)
- `801CAFF0` -> `8001B590` (sub_8001B590 / FastSprite submit wrapper)
- `801CB060` -> `8001B590` (sub_8001B590 / FastSprite submit wrapper)
- `801CB0B8` -> `8001B590` (sub_8001B590 / FastSprite submit wrapper)
- `801CB118` -> `8001B590` (sub_8001B590 / FastSprite submit wrapper)
- `801CB15C` -> `8001B590` (sub_8001B590 / FastSprite submit wrapper)

## Branches Within Function

- `801CAC5C` `bnez $v0, 0x801cac98`
- `801CAD40` `beqz $v0, 0x801cadd0`
- `801CADC8` `j 0x801cae40`
- `801CADDC` `bnez $v0, 0x801cae14`
- `801CAEC4` `bnez $v1, 0x801caedc`
- `801CAED4` `beqz $v0, 0x801caef4`
- `801CAFFC` `bnez $v0, 0x801caf64`
- `801CB06C` `bnez $v0, 0x801cb018`
- `801CB0CC` `beqz $v0, 0x801cb164`

## Entry Raw Bytes

```text
1E 80 02 3C 44 B3 42 8C A0 FF BD 27 5C 00 BF AF 58 00 B6 AF 54 00 B5 AF 50 00 B4 AF 4C 00 B3 AF 48 00 B2 AF 44 00 B1 AF 0E 00 40 14 40 00 B0 AF 1E 80 04 3C F0 98 84 24 01 00 05 34 07 80 02 3C
```

## Epilogue Raw Bytes

```text
21 10 43 00 80 10 02 00 21 10 55 00 64 6D 00 0C 18 00 A2 AF 5C 00 BF 8F 58 00 B6 8F 54 00 B5 8F 50 00 B4 8F 4C 00 B3 8F 48 00 B2 8F 44 00 B1 8F 40 00 B0 8F 60 00 BD 27 08 00 E0 03 00 00 00 00
```

## Full Function Disassembly

```asm
801CAC34: 1E 80 02 3C     lui      $v0, 0x801e
801CAC38: 44 B3 42 8C     lw       $v0, -0x4cbc($v0)
801CAC3C: A0 FF BD 27     addiu    $sp, $sp, -0x60
801CAC40: 5C 00 BF AF     sw       $ra, 0x5c($sp)
801CAC44: 58 00 B6 AF     sw       $s6, 0x58($sp)
801CAC48: 54 00 B5 AF     sw       $s5, 0x54($sp)
801CAC4C: 50 00 B4 AF     sw       $s4, 0x50($sp)
801CAC50: 4C 00 B3 AF     sw       $s3, 0x4c($sp)
801CAC54: 48 00 B2 AF     sw       $s2, 0x48($sp)
801CAC58: 44 00 B1 AF     sw       $s1, 0x44($sp)
801CAC5C: 0E 00 40 14     bnez     $v0, 0x801cac98
801CAC60: 40 00 B0 AF     sw       $s0, 0x40($sp)
801CAC64: 1E 80 04 3C     lui      $a0, 0x801e
801CAC68: F0 98 84 24     addiu    $a0, $a0, -0x6710
801CAC6C: 01 00 05 34     ori      $a1, $zero, 1
801CAC70: 07 80 02 3C     lui      $v0, 0x8007
801CAC74: A8 ED 42 8C     lw       $v0, -0x1258($v0)
801CAC78: 0A 00 07 34     ori      $a3, $zero, 0xa
801CAC7C: 80 30 02 00     sll      $a2, $v0, 2
801CAC80: 21 30 C2 00     addu     $a2, $a2, $v0
801CAC84: 80 30 06 00     sll      $a2, $a2, 2
801CAC88: 1D 80 02 3C     lui      $v0, 0x801d
801CAC8C: F4 5A 42 24     addiu    $v0, $v0, 0x5af4
801CAC90: 21 6C 00 0C     jal      0x8001b084
801CAC94: 21 30 C2 00     addu     $a2, $a2, $v0
801CAC98: 1E 80 04 3C     lui      $a0, 0x801e
801CAC9C: D8 B2 84 24     addiu    $a0, $a0, -0x4d28
801CACA0: 01 00 05 34     ori      $a1, $zero, 1
801CACA4: 0A 00 07 34     ori      $a3, $zero, 0xa
801CACA8: 07 80 02 3C     lui      $v0, 0x8007
801CACAC: A8 ED 42 8C     lw       $v0, -0x1258($v0)
801CACB0: 1D 80 10 3C     lui      $s0, 0x801d
801CACB4: F4 5A 10 26     addiu    $s0, $s0, 0x5af4
801CACB8: 80 30 02 00     sll      $a2, $v0, 2
801CACBC: 21 30 C2 00     addu     $a2, $a2, $v0
801CACC0: 80 30 06 00     sll      $a2, $a2, 2
801CACC4: 21 6C 00 0C     jal      0x8001b084
801CACC8: 21 30 D0 00     addu     $a2, $a2, $s0
801CACCC: 1E 80 04 3C     lui      $a0, 0x801e
801CACD0: 38 80 84 24     addiu    $a0, $a0, -0x7fc8
801CACD4: 01 00 05 34     ori      $a1, $zero, 1
801CACD8: 07 80 02 3C     lui      $v0, 0x8007
801CACDC: A8 ED 42 8C     lw       $v0, -0x1258($v0)
801CACE0: 0A 00 07 34     ori      $a3, $zero, 0xa
801CACE4: 80 30 02 00     sll      $a2, $v0, 2
801CACE8: 21 30 C2 00     addu     $a2, $a2, $v0
801CACEC: 80 30 06 00     sll      $a2, $a2, 2
801CACF0: 21 6C 00 0C     jal      0x8001b084
801CACF4: 21 30 D0 00     addu     $a2, $a2, $s0
801CACF8: 01 00 05 34     ori      $a1, $zero, 1
801CACFC: 0A 00 07 34     ori      $a3, $zero, 0xa
801CAD00: 1E 80 02 3C     lui      $v0, 0x801e
801CAD04: D0 98 42 24     addiu    $v0, $v0, -0x6730
801CAD08: 1D 80 04 3C     lui      $a0, 0x801d
801CAD0C: 14 30 84 8C     lw       $a0, 0x3014($a0)
801CAD10: 07 80 03 3C     lui      $v1, 0x8007
801CAD14: A8 ED 63 8C     lw       $v1, -0x1258($v1)
801CAD18: 00 21 04 00     sll      $a0, $a0, 4
801CAD1C: 21 20 82 00     addu     $a0, $a0, $v0
801CAD20: 80 30 03 00     sll      $a2, $v1, 2
801CAD24: 21 30 C3 00     addu     $a2, $a2, $v1
801CAD28: 80 30 06 00     sll      $a2, $a2, 2
801CAD2C: 21 6C 00 0C     jal      0x8001b084
801CAD30: 21 30 D0 00     addu     $a2, $a2, $s0
801CAD34: 1E 80 02 3C     lui      $v0, 0x801e
801CAD38: 40 B3 42 8C     lw       $v0, -0x4cc0($v0)
801CAD3C: 00 00 00 00     nop      
801CAD40: 23 00 40 10     beqz     $v0, 0x801cadd0
801CAD44: 01 00 05 34     ori      $a1, $zero, 1
801CAD48: 1D 80 10 3C     lui      $s0, 0x801d
801CAD4C: 68 7F 10 26     addiu    $s0, $s0, 0x7f68
801CAD50: 21 20 00 02     move     $a0, $s0
801CAD54: 0A 00 07 34     ori      $a3, $zero, 0xa
801CAD58: 07 80 02 3C     lui      $v0, 0x8007
801CAD5C: A8 ED 42 8C     lw       $v0, -0x1258($v0)
801CAD60: 1D 80 11 3C     lui      $s1, 0x801d
801CAD64: 44 32 31 26     addiu    $s1, $s1, 0x3244
801CAD68: 80 30 02 00     sll      $a2, $v0, 2
801CAD6C: 21 30 C2 00     addu     $a2, $a2, $v0
801CAD70: 80 30 06 00     sll      $a2, $a2, 2
801CAD74: 21 6C 00 0C     jal      0x8001b084
801CAD78: 21 30 D1 00     addu     $a2, $a2, $s1
801CAD7C: 90 00 04 26     addiu    $a0, $s0, 0x90
801CAD80: 03 00 05 34     ori      $a1, $zero, 3
801CAD84: 07 80 02 3C     lui      $v0, 0x8007
801CAD88: A8 ED 42 8C     lw       $v0, -0x1258($v0)
801CAD8C: 0A 00 07 34     ori      $a3, $zero, 0xa
801CAD90: 80 30 02 00     sll      $a2, $v0, 2
801CAD94: 21 30 C2 00     addu     $a2, $a2, $v0
801CAD98: 80 30 06 00     sll      $a2, $a2, 2
801CAD9C: 21 6C 00 0C     jal      0x8001b084
801CADA0: 21 30 D1 00     addu     $a2, $a2, $s1
801CADA4: C0 00 04 26     addiu    $a0, $s0, 0xc0
801CADA8: 01 00 05 34     ori      $a1, $zero, 1
801CADAC: 07 80 02 3C     lui      $v0, 0x8007
801CADB0: A8 ED 42 8C     lw       $v0, -0x1258($v0)
801CADB4: 08 00 07 34     ori      $a3, $zero, 8
801CADB8: 80 30 02 00     sll      $a2, $v0, 2
801CADBC: 21 30 C2 00     addu     $a2, $a2, $v0
801CADC0: 1D 80 02 3C     lui      $v0, 0x801d
801CADC4: 6C 52 42 24     addiu    $v0, $v0, 0x526c
801CADC8: 90 2B 07 08     j        0x801cae40
801CADCC: 80 30 06 00     sll      $a2, $a2, 2
801CADD0: 1E 80 02 3C     lui      $v0, 0x801e
801CADD4: 3C B3 42 8C     lw       $v0, -0x4cc4($v0)
801CADD8: 00 00 00 00     nop      
801CADDC: 0D 00 40 14     bnez     $v0, 0x801cae14
801CADE0: 08 00 07 34     ori      $a3, $zero, 8
801CADE4: 1E 80 04 3C     lui      $a0, 0x801e
801CADE8: 28 80 84 24     addiu    $a0, $a0, -0x7fd8
801CADEC: 07 80 02 3C     lui      $v0, 0x8007
801CADF0: A8 ED 42 8C     lw       $v0, -0x1258($v0)
801CADF4: 00 00 00 00     nop      
801CADF8: 80 30 02 00     sll      $a2, $v0, 2
801CADFC: 21 30 C2 00     addu     $a2, $a2, $v0
801CAE00: 80 30 06 00     sll      $a2, $a2, 2
801CAE04: 1D 80 02 3C     lui      $v0, 0x801d
801CAE08: 6C 52 42 24     addiu    $v0, $v0, 0x526c
801CAE0C: 21 6C 00 0C     jal      0x8001b084
801CAE10: 21 30 C2 00     addu     $a2, $a2, $v0
801CAE14: 1D 80 04 3C     lui      $a0, 0x801d
801CAE18: 68 7F 84 24     addiu    $a0, $a0, 0x7f68
801CAE1C: 0C 00 05 34     ori      $a1, $zero, 0xc
801CAE20: 07 80 02 3C     lui      $v0, 0x8007
801CAE24: A8 ED 42 8C     lw       $v0, -0x1258($v0)
801CAE28: 0A 00 07 34     ori      $a3, $zero, 0xa
801CAE2C: 80 30 02 00     sll      $a2, $v0, 2
801CAE30: 21 30 C2 00     addu     $a2, $a2, $v0
801CAE34: 80 30 06 00     sll      $a2, $a2, 2
801CAE38: 1D 80 02 3C     lui      $v0, 0x801d
801CAE3C: 44 32 42 24     addiu    $v0, $v0, 0x3244
801CAE40: 21 6C 00 0C     jal      0x8001b084
801CAE44: 21 30 C2 00     addu     $a2, $a2, $v0
801CAE48: 20 00 A4 27     addiu    $a0, $sp, 0x20
801CAE4C: 30 00 A5 27     addiu    $a1, $sp, 0x30
801CAE50: 09 80 02 3C     lui      $v0, 0x8009
801CAE54: 6C 5C 42 8C     lw       $v0, 0x5c6c($v0)
801CAE58: 09 80 03 3C     lui      $v1, 0x8009
801CAE5C: 60 5C 63 8C     lw       $v1, 0x5c60($v1)
801CAE60: 09 80 06 3C     lui      $a2, 0x8009
801CAE64: 64 5C C6 8C     lw       $a2, 0x5c64($a2)
801CAE68: 09 80 07 3C     lui      $a3, 0x8009
801CAE6C: 68 5C E7 8C     lw       $a3, 0x5c68($a3)
801CAE70: 23 10 43 00     subu     $v0, $v0, $v1
801CAE74: 20 00 A2 AF     sw       $v0, 0x20($sp)
801CAE78: 09 80 02 3C     lui      $v0, 0x8009
801CAE7C: 70 5C 42 8C     lw       $v0, 0x5c70($v0)
801CAE80: 09 80 03 3C     lui      $v1, 0x8009
801CAE84: 74 5C 63 8C     lw       $v1, 0x5c74($v1)
801CAE88: 23 10 46 00     subu     $v0, $v0, $a2
801CAE8C: 23 18 67 00     subu     $v1, $v1, $a3
801CAE90: 24 00 A2 AF     sw       $v0, 0x24($sp)
801CAE94: F7 E8 00 0C     jal      0x8003a3dc
801CAE98: 28 00 A3 AF     sw       $v1, 0x28($sp)
801CAE9C: 34 00 A3 8F     lw       $v1, 0x34($sp)
801CAEA0: 00 00 00 00     nop      
801CAEA4: C0 10 03 00     sll      $v0, $v1, 3
801CAEA8: 23 10 43 00     subu     $v0, $v0, $v1
801CAEAC: C0 10 02 00     sll      $v0, $v0, 3
801CAEB0: 23 10 43 00     subu     $v0, $v0, $v1
801CAEB4: C0 10 02 00     sll      $v0, $v0, 3
801CAEB8: 23 10 02 00     negu     $v0, $v0
801CAEBC: 30 00 A3 8F     lw       $v1, 0x30($sp)
801CAEC0: 00 00 00 00     nop      
801CAEC4: 05 00 60 14     bnez     $v1, 0x801caedc
801CAEC8: 03 A3 02 00     sra      $s4, $v0, 0xc
801CAECC: 38 00 A2 8F     lw       $v0, 0x38($sp)
801CAED0: 00 00 00 00     nop      
801CAED4: 07 00 40 10     beqz     $v0, 0x801caef4
801CAED8: 21 98 00 00     move     $s3, $zero
801CAEDC: 38 00 A4 8F     lw       $a0, 0x38($sp)
801CAEE0: 30 00 A5 8F     lw       $a1, 0x30($sp)
801CAEE4: C3 ED 00 0C     jal      0x8003b70c
801CAEE8: 00 00 00 00     nop      
801CAEEC: 43 10 02 00     sra      $v0, $v0, 1
801CAEF0: 00 FE 53 24     addiu    $s3, $v0, -0x200
801CAEF4: 90 00 08 3C     lui      $t0, 0x90
801CAEF8: 00 18 08 35     ori      $t0, $t0, 0x1800
801CAEFC: E8 00 03 3C     lui      $v1, 0xe8
801CAF00: 3F 80 63 34     ori      $v1, $v1, 0x803f
801CAF04: 21 20 00 00     move     $a0, $zero
801CAF08: 21 28 00 00     move     $a1, $zero
801CAF0C: 40 01 06 34     ori      $a2, $zero, 0x140
801CAF10: 91 00 87 26     addiu    $a3, $s4, 0x91
801CAF14: 80 FD 12 24     addiu    $s2, $zero, -0x280
801CAF18: 21 88 00 00     move     $s1, $zero
801CAF1C: 1D 80 02 3C     lui      $v0, 0x801d
801CAF20: 1C 7B 42 24     addiu    $v0, $v0, 0x7b1c
801CAF24: 21 A8 40 00     move     $s5, $v0
801CAF28: 14 00 A3 AF     sw       $v1, 0x14($sp)
801CAF2C: 07 80 03 3C     lui      $v1, 0x8007
801CAF30: A8 ED 63 8C     lw       $v1, -0x1258($v1)
801CAF34: 01 00 02 34     ori      $v0, $zero, 1
801CAF38: 10 00 A8 AF     sw       $t0, 0x10($sp)
801CAF3C: 18 00 A2 AF     sw       $v0, 0x18($sp)
801CAF40: 80 10 03 00     sll      $v0, $v1, 2
801CAF44: 21 10 43 00     addu     $v0, $v0, $v1
801CAF48: 80 10 02 00     sll      $v0, $v0, 2
801CAF4C: 21 10 55 00     addu     $v0, $v0, $s5
801CAF50: 8D 29 07 0C     jal      0x801ca634
801CAF54: 1C 00 A2 AF     sw       $v0, 0x1c($sp)
801CAF58: 23 00 82 26     addiu    $v0, $s4, 0x23
801CAF5C: 00 B4 02 00     sll      $s6, $v0, 0x10
801CAF60: 21 80 72 02     addu     $s0, $s3, $s2
801CAF64: F0 00 04 26     addiu    $a0, $s0, 0xf0
801CAF68: 00 24 04 00     sll      $a0, $a0, 0x10
801CAF6C: 03 24 04 00     sra      $a0, $a0, 0x10
801CAF70: 03 2C 16 00     sra      $a1, $s6, 0x10
801CAF74: 1D 80 06 3C     lui      $a2, 0x801d
801CAF78: 90 2F C6 24     addiu    $a2, $a2, 0x2f90
801CAF7C: 21 38 00 00     move     $a3, $zero
801CAF80: 01 00 31 26     addiu    $s1, $s1, 1
801CAF84: 07 80 03 3C     lui      $v1, 0x8007
801CAF88: A8 ED 63 8C     lw       $v1, -0x1258($v1)
801CAF8C: 40 01 52 26     addiu    $s2, $s2, 0x140
801CAF90: 10 00 A0 AF     sw       $zero, 0x10($sp)
801CAF94: 14 00 A0 AF     sw       $zero, 0x14($sp)
801CAF98: 80 10 03 00     sll      $v0, $v1, 2
801CAF9C: 21 10 43 00     addu     $v0, $v0, $v1
801CAFA0: 80 10 02 00     sll      $v0, $v0, 2
801CAFA4: 21 10 55 00     addu     $v0, $v0, $s5
801CAFA8: 64 6D 00 0C     jal      0x8001b590
801CAFAC: 18 00 A2 AF     sw       $v0, 0x18($sp)
801CAFB0: E2 FF 10 26     addiu    $s0, $s0, -0x1e
801CAFB4: 00 84 10 00     sll      $s0, $s0, 0x10
801CAFB8: 03 24 10 00     sra      $a0, $s0, 0x10
801CAFBC: 00 2C 14 00     sll      $a1, $s4, 0x10
801CAFC0: 03 2C 05 00     sra      $a1, $a1, 0x10
801CAFC4: 1D 80 06 3C     lui      $a2, 0x801d
801CAFC8: 90 2F C6 24     addiu    $a2, $a2, 0x2f90
801CAFCC: 07 80 03 3C     lui      $v1, 0x8007
801CAFD0: A8 ED 63 8C     lw       $v1, -0x1258($v1)
801CAFD4: 21 38 00 00     move     $a3, $zero
801CAFD8: 10 00 A0 AF     sw       $zero, 0x10($sp)
801CAFDC: 14 00 A0 AF     sw       $zero, 0x14($sp)
801CAFE0: 80 10 03 00     sll      $v0, $v1, 2
801CAFE4: 21 10 43 00     addu     $v0, $v0, $v1
801CAFE8: 80 10 02 00     sll      $v0, $v0, 2
801CAFEC: 21 10 55 00     addu     $v0, $v0, $s5
801CAFF0: 64 6D 00 0C     jal      0x8001b590
801CAFF4: 18 00 A2 AF     sw       $v0, 0x18($sp)
801CAFF8: 05 00 22 2A     slti     $v0, $s1, 5
801CAFFC: D9 FF 40 14     bnez     $v0, 0x801caf64
801CB000: 21 80 72 02     addu     $s0, $s3, $s2
801CB004: 3C FD 12 24     addiu    $s2, $zero, -0x2c4
801CB008: 21 88 00 00     move     $s1, $zero
801CB00C: 67 00 82 26     addiu    $v0, $s4, 0x67
801CB010: 00 84 02 00     sll      $s0, $v0, 0x10
801CB014: 21 20 72 02     addu     $a0, $s3, $s2
801CB018: 00 24 04 00     sll      $a0, $a0, 0x10
801CB01C: 03 24 04 00     sra      $a0, $a0, 0x10
801CB020: 03 2C 10 00     sra      $a1, $s0, 0x10
801CB024: 1D 80 06 3C     lui      $a2, 0x801d
801CB028: B0 2F C6 24     addiu    $a2, $a2, 0x2fb0
801CB02C: 21 38 00 00     move     $a3, $zero
801CB030: 01 00 31 26     addiu    $s1, $s1, 1
801CB034: 00 01 52 26     addiu    $s2, $s2, 0x100
801CB038: 07 80 03 3C     lui      $v1, 0x8007
801CB03C: A8 ED 63 8C     lw       $v1, -0x1258($v1)
801CB040: 1D 80 15 3C     lui      $s5, 0x801d
801CB044: 1C 7B B5 26     addiu    $s5, $s5, 0x7b1c
801CB048: 10 00 A0 AF     sw       $zero, 0x10($sp)
801CB04C: 14 00 A0 AF     sw       $zero, 0x14($sp)
801CB050: 80 10 03 00     sll      $v0, $v1, 2
801CB054: 21 10 43 00     addu     $v0, $v0, $v1
801CB058: 80 10 02 00     sll      $v0, $v0, 2
801CB05C: 21 10 55 00     addu     $v0, $v0, $s5
801CB060: 64 6D 00 0C     jal      0x8001b590
801CB064: 18 00 A2 AF     sw       $v0, 0x18($sp)
801CB068: 07 00 22 2A     slti     $v0, $s1, 7
801CB06C: EA FF 40 14     bnez     $v0, 0x801cb018
801CB070: 21 20 72 02     addu     $a0, $s3, $s2
801CB074: 6E 00 64 26     addiu    $a0, $s3, 0x6e
801CB078: 00 24 04 00     sll      $a0, $a0, 0x10
801CB07C: 03 24 04 00     sra      $a0, $a0, 0x10
801CB080: 0A 00 85 26     addiu    $a1, $s4, 0xa
801CB084: 00 2C 05 00     sll      $a1, $a1, 0x10
801CB088: 03 2C 05 00     sra      $a1, $a1, 0x10
801CB08C: 1D 80 06 3C     lui      $a2, 0x801d
801CB090: A0 2F C6 24     addiu    $a2, $a2, 0x2fa0
801CB094: 07 80 03 3C     lui      $v1, 0x8007
801CB098: A8 ED 63 8C     lw       $v1, -0x1258($v1)
801CB09C: 21 38 00 00     move     $a3, $zero
801CB0A0: 10 00 A0 AF     sw       $zero, 0x10($sp)
801CB0A4: 14 00 A0 AF     sw       $zero, 0x14($sp)
801CB0A8: 80 10 03 00     sll      $v0, $v1, 2
801CB0AC: 21 10 43 00     addu     $v0, $v0, $v1
801CB0B0: 80 10 02 00     sll      $v0, $v0, 2
801CB0B4: 21 10 55 00     addu     $v0, $v0, $s5
801CB0B8: 64 6D 00 0C     jal      0x8001b590
801CB0BC: 18 00 A2 AF     sw       $v0, 0x18($sp)
801CB0C0: 1E 80 02 3C     lui      $v0, 0x801e
801CB0C4: 44 B3 42 8C     lw       $v0, -0x4cbc($v0)
801CB0C8: 00 00 00 00     nop      
801CB0CC: 25 00 40 10     beqz     $v0, 0x801cb164
801CB0D0: 28 00 70 26     addiu    $s0, $s3, 0x28
801CB0D4: 00 84 10 00     sll      $s0, $s0, 0x10
801CB0D8: 03 84 10 00     sra      $s0, $s0, 0x10
801CB0DC: 21 20 00 02     move     $a0, $s0
801CB0E0: FE FF 85 26     addiu    $a1, $s4, -2
801CB0E4: 00 2C 05 00     sll      $a1, $a1, 0x10
801CB0E8: 03 2C 05 00     sra      $a1, $a1, 0x10
801CB0EC: 1D 80 06 3C     lui      $a2, 0x801d
801CB0F0: 70 2F C6 24     addiu    $a2, $a2, 0x2f70
801CB0F4: 07 80 03 3C     lui      $v1, 0x8007
801CB0F8: A8 ED 63 8C     lw       $v1, -0x1258($v1)
801CB0FC: 21 38 00 00     move     $a3, $zero
801CB100: 10 00 A0 AF     sw       $zero, 0x10($sp)
801CB104: 14 00 A0 AF     sw       $zero, 0x14($sp)
801CB108: 80 10 03 00     sll      $v0, $v1, 2
801CB10C: 21 10 43 00     addu     $v0, $v0, $v1
801CB110: 80 10 02 00     sll      $v0, $v0, 2
801CB114: 21 10 55 00     addu     $v0, $v0, $s5
801CB118: 64 6D 00 0C     jal      0x8001b590
801CB11C: 18 00 A2 AF     sw       $v0, 0x18($sp)
801CB120: 21 20 00 02     move     $a0, $s0
801CB124: 5D 00 85 26     addiu    $a1, $s4, 0x5d
801CB128: 00 2C 05 00     sll      $a1, $a1, 0x10
801CB12C: 03 2C 05 00     sra      $a1, $a1, 0x10
801CB130: 1D 80 06 3C     lui      $a2, 0x801d
801CB134: 80 2F C6 24     addiu    $a2, $a2, 0x2f80
801CB138: 07 80 03 3C     lui      $v1, 0x8007
801CB13C: A8 ED 63 8C     lw       $v1, -0x1258($v1)
801CB140: 21 38 00 00     move     $a3, $zero
801CB144: 10 00 A0 AF     sw       $zero, 0x10($sp)
801CB148: 14 00 A0 AF     sw       $zero, 0x14($sp)
801CB14C: 80 10 03 00     sll      $v0, $v1, 2
801CB150: 21 10 43 00     addu     $v0, $v0, $v1
801CB154: 80 10 02 00     sll      $v0, $v0, 2
801CB158: 21 10 55 00     addu     $v0, $v0, $s5
801CB15C: 64 6D 00 0C     jal      0x8001b590
801CB160: 18 00 A2 AF     sw       $v0, 0x18($sp)
801CB164: 5C 00 BF 8F     lw       $ra, 0x5c($sp)
801CB168: 58 00 B6 8F     lw       $s6, 0x58($sp)
801CB16C: 54 00 B5 8F     lw       $s5, 0x54($sp)
801CB170: 50 00 B4 8F     lw       $s4, 0x50($sp)
801CB174: 4C 00 B3 8F     lw       $s3, 0x4c($sp)
801CB178: 48 00 B2 8F     lw       $s2, 0x48($sp)
801CB17C: 44 00 B1 8F     lw       $s1, 0x44($sp)
801CB180: 40 00 B0 8F     lw       $s0, 0x40($sp)
801CB184: 60 00 BD 27     addiu    $sp, $sp, 0x60
801CB188: 08 00 E0 03     jr       $ra
801CB18C: 00 00 00 00     nop      
```

## Call Chain Boundary

- `801CAC34` submits Stage1 backdrop TMD ranges through `8001B084`; the existing main-SCUS export then carries `8001B084 -> 800428B0`.
- `801CAC34` also emits backdrop FastSprite calls through `8001B590` and calls the gradient helper before those sprite loops.
- This closes the standalone xref/disasm supplement for `801CAC34`; packet side effects below `800428B0` remain a separate direct-port gap.
