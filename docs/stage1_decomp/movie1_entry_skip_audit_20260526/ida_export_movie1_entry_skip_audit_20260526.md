[summary]
success: True
mode: exec
filename: <mcp-idapython-exec>
persist_globals: False
result_type: NoneType
result_repr: None

[stdout]
# IDA export: Stage1 Movie1 entry / skip transition audit

Generated: 2026-05-26T16:54:06.732354
Input IDB target: SCUS_941.83 current IDA session
Scope: Pro-requested evidence for Initial Movie1 pre-transition visibility and story Movie1 skip freeze/black frame. No runtime patch in this file.

## `801C4DC4` `sub_801C4DC4`
Bounds: `801C4DC4-801C4F68`

### Pseudocode
```c
int __fastcall sub_801C4DC4(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int (*a9)(void))
{
  int v10; // $a0
  int v11; // $s0
  int result; // $v0

  if ( !word_800916D2 )
  {
    word_800916D2 = 1;
    sub_80020110(&unk_801C3640, 2, 1, 2);
  }
  sub_800201AC(&unk_801C3640, 6, 1, 2);
  sub_801C44E0(108, 0);
  sub_801C455C(108, &unk_801C3640, 0);
  sub_8001B120(1);
  word_800916D2 = 1;
  sub_800201AC(&unk_801C3640, 5, 1, 2);
  v11 = sub_801C4894(156, (int)&unk_801C3640);
  sub_80026FA4(v10);
  sub_80020110(&unk_801C3640, 2, 1, 2);
  sub_8001EF14();
  if ( v11 == 1 )
  {
    word_800916D0 = 0;
    return -1;
  }
  else if ( v11 == 3 )
  {
    word_800916D0 = 1;
    do
      result = rand() % 6 + 1;
    while ( result == word_800916EE );
  }
  else
  {
    sub_80026EF8(dword_80094410);
    sub_80026ECC();
    result = a1 + 1;
    word_800916D0 = 0;
  }
  return result;
}

```

### Calls in function
- 801C4DF0: `bnez    $v0, loc_801C4E14` -> 801C4E14 loc_801C4E14
- 801C4E0C: `jal     sub_80020110` -> 80020110 sub_80020110
- 801C4E20: `jal     sub_800201AC` -> 800201AC sub_800201AC
- 801C4E34: `jal     sub_801C44E0` -> 801C44E0 sub_801C44E0
- 801C4E4C: `jal     sub_801C455C` -> 801C455C sub_801C455C
- 801C4E54: `jal     sub_8001B120` -> 8001B120 sub_8001B120
- 801C4E70: `jal     sub_800201AC` -> 800201AC sub_800201AC
- 801C4E84: `jal     sub_801C4894` -> 801C4894 sub_801C4894
- 801C4E8C: `jal     sub_80026FA4` -> 80026FA4 sub_80026FA4
- 801C4EA0: `jal     sub_80020110` -> 80020110 sub_80020110
- 801C4EA8: `jal     sub_8001EF14` -> 8001EF14 sub_8001EF14
- 801C4EB4: `bne     $s0, $v0, loc_801C4ECC` -> 801C4ECC loc_801C4ECC
- 801C4EC4: `j       loc_801C4F48` -> 801C4F48 loc_801C4F48
- 801C4ECC: `bne     $s0, $v0, loc_801C4F24` -> 801C4F24 loc_801C4F24
- 801C4EE0: `jal     rand` -> 80047FDC rand
- 801C4F14: `beq     $v0, $v1, loc_801C4EE0` -> 801C4EE0 loc_801C4EE0
- 801C4F1C: `j       loc_801C4F48` -> 801C4F48 loc_801C4F48
- 801C4F2C: `jal     sub_80026EF8` -> 80026EF8 sub_80026EF8
- 801C4F34: `jal     sub_80026ECC` -> 80026ECC sub_80026ECC

### Xrefs to
- 80048D30 -> 801C4DC4 type=1 func=<no func> line=`.word sub_801C4DC4`

### Xrefs from
- 801C4DC4 -> 801C4DC8 type=21 target=sub_801C4DC8 line=`addiu   $sp, -0x28`
- 801C4DC8 -> 801C4DCC type=21 target=sub_801C4DCC line=`sw      $s3, 0x10+var_sC($sp)`
- 801C4DCC -> 801C4DD0 type=21 target=sub_801C4DD0 line=`sw      $s0, 0x10+var_s0($sp)`
- 801C4DD0 -> 801C4DD8 type=21 target=sub_801C4DD8 line=`li      $s0, word_800916D2`
- 801C4DD8 -> 801C4DDC type=21 target=sub_801C4DDC line=`sw      $ra, 0x10+var_s10($sp)`
- 801C4DDC -> 801C4DE0 type=21 target=sub_801C4DE0 line=`sw      $s2, 0x10+var_s8($sp)`
- 801C4DE0 -> 801C4DE4 type=21 target=sub_801C4DE4 line=`sw      $s1, 0x10+var_s4($sp)`
- 801C4DE4 -> 801C4DE8 type=21 target=sub_801C4DE8 line=`lh      $v0, (word_800916D2 - 0x800916D2)($s0)`
- 801C4DE8 -> 801C4DF0 type=21 target=sub_801C4DF0 line=`li      $s1, unk_801C3640`
- 801C4DF0 -> 801C4DF4 type=21 target=sub_801C4DF4 line=`bnez    $v0, loc_801C4E14`
- 801C4DF0 -> 801C4E14 type=19 target=loc_801C4E14 line=`bnez    $v0, loc_801C4E14`
- 801C4DF4 -> 801C4DF8 type=21 target=sub_801C4DF8 line=`move    $s3, $a0`
- 801C4DF8 -> 801C4DFC type=21 target=sub_801C4DFC line=`li      $v0, 1`
- 801C4DFC -> 801C4E00 type=21 target=sub_801C4E00 line=`sh      $v0, (word_800916D2 - 0x800916D2)($s0)`
- 801C4E00 -> 801C4E04 type=21 target=sub_801C4E04 line=`move    $a0, $s1`
- 801C4E04 -> 801C4E08 type=21 target=sub_801C4E08 line=`li      $a1, 2`
- 801C4E08 -> 801C4E0C type=21 target=sub_801C4E0C line=`li      $a2, 1`
- 801C4E0C -> 801C4E10 type=21 target=sub_801C4E10 line=`jal     sub_80020110`
- 801C4E0C -> 80020110 type=17 target=sub_80020110 line=`jal     sub_80020110`
- 801C4E10 -> 801C4E14 type=21 target=loc_801C4E14 line=`li      $a3, 2`
- 801C4E14 -> 801C4E18 type=21 target=sub_801C4E18 line=`move    $a0, $s1`
- 801C4E18 -> 801C4E1C type=21 target=sub_801C4E1C line=`li      $a1, 6`
- 801C4E1C -> 801C4E20 type=21 target=sub_801C4E20 line=`li      $a2, 1`
- 801C4E20 -> 801C4E24 type=21 target=sub_801C4E24 line=`jal     sub_800201AC`
- 801C4E20 -> 800201AC type=17 target=sub_800201AC line=`jal     sub_800201AC`
- 801C4E24 -> 801C4E28 type=21 target=sub_801C4E28 line=`li      $a3, 2`
- 801C4E28 -> 801C4E30 type=21 target=sub_801C4E30 line=`lw      $a0, dword_8006ED98+0x20`
- 801C4E30 -> 801C4E34 type=21 target=sub_801C4E34 line=`move    $a1, $zero`
- 801C4E34 -> 801C4E38 type=21 target=sub_801C4E38 line=`jal     sub_801C44E0`
- 801C4E34 -> 801C44E0 type=17 target=sub_801C44E0 line=`jal     sub_801C44E0`
- 801C4E38 -> 801C4E3C type=21 target=sub_801C4E3C line=`addiu   $a0, 0x6C  # 'l'`
- 801C4E3C -> 801C4E40 type=21 target=sub_801C4E40 line=`move    $a1, $s1`
- 801C4E40 -> 801C4E48 type=21 target=sub_801C4E48 line=`lw      $a0, dword_8006ED98+0x20`
- 801C4E48 -> 801C4E4C type=21 target=sub_801C4E4C line=`move    $a2, $zero`
- 801C4E4C -> 801C4E50 type=21 target=sub_801C4E50 line=`jal     sub_801C455C`
- 801C4E4C -> 801C455C type=17 target=sub_801C455C line=`jal     sub_801C455C`
- 801C4E50 -> 801C4E54 type=21 target=sub_801C4E54 line=`addiu   $a0, 0x6C  # 'l'`
- 801C4E54 -> 801C4E58 type=21 target=sub_801C4E58 line=`jal     sub_8001B120`
- 801C4E54 -> 8001B120 type=17 target=sub_8001B120 line=`jal     sub_8001B120`
- 801C4E58 -> 801C4E5C type=21 target=sub_801C4E5C line=`li      $a0, 1`
- 801C4E5C -> 801C4E60 type=21 target=sub_801C4E60 line=`move    $a0, $s1`
- 801C4E60 -> 801C4E64 type=21 target=sub_801C4E64 line=`li      $a1, 5`
- 801C4E64 -> 801C4E68 type=21 target=sub_801C4E68 line=`li      $a2, 1`
- 801C4E68 -> 801C4E6C type=21 target=sub_801C4E6C line=`li      $a3, 2`
- 801C4E6C -> 801C4E70 type=21 target=sub_801C4E70 line=`li      $s2, 1`
- 801C4E70 -> 801C4E74 type=21 target=sub_801C4E74 line=`jal     sub_800201AC`
- 801C4E70 -> 800201AC type=17 target=sub_800201AC line=`jal     sub_800201AC`
- 801C4E74 -> 801C4E78 type=21 target=sub_801C4E78 line=`sh      $s2, (word_800916D2 - 0x800916D2)($s0)`
- 801C4E78 -> 801C4E80 type=21 target=sub_801C4E80 line=`lw      $a0, dword_8006ED98+0x20`
- 801C4E80 -> 801C4E84 type=21 target=sub_801C4E84 line=`move    $a1, $s1`
- 801C4E84 -> 801C4E88 type=21 target=sub_801C4E88 line=`jal     sub_801C4894`
- 801C4E84 -> 801C4894 type=17 target=sub_801C4894 line=`jal     sub_801C4894`
- 801C4E88 -> 801C4E8C type=21 target=sub_801C4E8C line=`addiu   $a0, 0x9C`
- 801C4E8C -> 801C4E90 type=21 target=sub_801C4E90 line=`jal     sub_80026FA4`
- 801C4E8C -> 80026FA4 type=17 target=sub_80026FA4 line=`jal     sub_80026FA4`
- 801C4E90 -> 801C4E94 type=21 target=sub_801C4E94 line=`move    $s0, $v0`
- 801C4E94 -> 801C4E98 type=21 target=sub_801C4E98 line=`move    $a0, $s1`
- 801C4E98 -> 801C4E9C type=21 target=sub_801C4E9C line=`li      $a1, 2`
- 801C4E9C -> 801C4EA0 type=21 target=sub_801C4EA0 line=`li      $a2, 1`
- 801C4EA0 -> 801C4EA4 type=21 target=sub_801C4EA4 line=`jal     sub_80020110`
- 801C4EA0 -> 80020110 type=17 target=sub_80020110 line=`jal     sub_80020110`
- 801C4EA4 -> 801C4EA8 type=21 target=sub_801C4EA8 line=`li      $a3, 2`
- 801C4EA8 -> 801C4EAC type=21 target=sub_801C4EAC line=`jal     sub_8001EF14`
- 801C4EA8 -> 8001EF14 type=17 target=sub_8001EF14 line=`jal     sub_8001EF14`
- 801C4EAC -> 801C4EB0 type=21 target=sub_801C4EB0 line=`nop`
- 801C4EB0 -> 801C4EB4 type=21 target=sub_801C4EB4 line=`li      $v0, 1`
- 801C4EB4 -> 801C4EB8 type=21 target=sub_801C4EB8 line=`bne     $s0, $v0, loc_801C4ECC`
- 801C4EB4 -> 801C4ECC type=19 target=loc_801C4ECC line=`bne     $s0, $v0, loc_801C4ECC`
- 801C4EB8 -> 801C4EBC type=21 target=sub_801C4EBC line=`li      $v0, 3`
- 801C4EBC -> 801C4EC4 type=21 target=sub_801C4EC4 line=`sh      $zero, word_800916D0`
- 801C4EC4 -> 801C4EC8 type=21 target=sub_801C4EC8 line=`j       loc_801C4F48`
- 801C4EC4 -> 801C4F48 type=19 target=loc_801C4F48 line=`j       loc_801C4F48`
- 801C4ECC -> 801C4ED0 type=21 target=sub_801C4ED0 line=`bne     $s0, $v0, loc_801C4F24`
- 801C4ECC -> 801C4F24 type=19 target=loc_801C4F24 line=`bne     $s0, $v0, loc_801C4F24`
- 801C4ED0 -> 801C4ED4 type=21 target=sub_801C4ED4 line=`lui     $s0, 0x2AAA`
- 801C4ED4 -> 801C4EDC type=21 target=sub_801C4EDC line=`sh      $s2, word_800916D0`
- 801C4EDC -> 801C4EE0 type=21 target=loc_801C4EE0 line=`li      $s0, 0x2AAAAAAB`
- 801C4EE0 -> 801C4EE4 type=21 target=sub_801C4EE4 line=`jal     rand`
- 801C4EE0 -> 80047FDC type=17 target=rand line=`jal     rand`
- 801C4EE4 -> 801C4EE8 type=21 target=sub_801C4EE8 line=`nop`
- 801C4EE8 -> 801C4EEC type=21 target=sub_801C4EEC line=`mult    $v0, $s0`
- 801C4EEC -> 801C4EF0 type=21 target=sub_801C4EF0 line=`sra     $a0, $v0, 31`
- 801C4EF0 -> 801C4EF4 type=21 target=sub_801C4EF4 line=`mfhi    $t0`
- 801C4EF4 -> 801C4EF8 type=21 target=sub_801C4EF8 line=`subu    $a0, $t0, $a0`
- 801C4EF8 -> 801C4EFC type=21 target=sub_801C4EFC line=`sll     $v1, $a0, 1`
- 801C4EFC -> 801C4F00 type=21 target=sub_801C4F00 line=`addu    $v1, $a0`
- 801C4F00 -> 801C4F04 type=21 target=sub_801C4F04 line=`sll     $v1, 1`
- 801C4F04 -> 801C4F08 type=21 target=sub_801C4F08 line=`subu    $v0, $v1`
- 801C4F08 -> 801C4F10 type=21 target=sub_801C4F10 line=`lh      $v1, word_800916EE`
- 801C4F10 -> 801C4F14 type=21 target=sub_801C4F14 line=`addiu   $v0, 1`
- 801C4F14 -> 801C4F18 type=21 target=sub_801C4F18 line=`beq     $v0, $v1, loc_801C4EE0`
- 801C4F14 -> 801C4EE0 type=19 target=loc_801C4EE0 line=`beq     $v0, $v1, loc_801C4EE0`
- 801C4F18 -> 801C4F1C type=21 target=sub_801C4F1C line=`nop`
- 801C4F1C -> 801C4F20 type=21 target=sub_801C4F20 line=`j       loc_801C4F48`
- 801C4F1C -> 801C4F48 type=19 target=loc_801C4F48 line=`j       loc_801C4F48`
- 801C4F24 -> 801C4F2C type=21 target=sub_801C4F2C line=`lw      $a0, dword_80094410`
- 801C4F2C -> 801C4F30 type=21 target=sub_801C4F30 line=`jal     sub_80026EF8`
- 801C4F2C -> 80026EF8 type=17 target=sub_80026EF8 line=`jal     sub_80026EF8`
- 801C4F30 -> 801C4F34 type=21 target=sub_801C4F34 line=`nop`
- 801C4F34 -> 801C4F38 type=21 target=sub_801C4F38 line=`jal     sub_80026ECC`
- 801C4F34 -> 80026ECC type=17 target=sub_80026ECC line=`jal     sub_80026ECC`
- 801C4F38 -> 801C4F3C type=21 target=sub_801C4F3C line=`nop`
- 801C4F3C -> 801C4F40 type=21 target=sub_801C4F40 line=`addiu   $v0, $s3, 1`
- 801C4F40 -> 801C4F48 type=21 target=loc_801C4F48 line=`sh      $zero, word_800916D0`
- 801C4F48 -> 801C4F4C type=21 target=sub_801C4F4C line=`lw      $ra, 0x10+var_s10($sp)`
- 801C4F4C -> 801C4F50 type=21 target=sub_801C4F50 line=`lw      $s3, 0x10+var_sC($sp)`
- 801C4F50 -> 801C4F54 type=21 target=sub_801C4F54 line=`lw      $s2, 0x10+var_s8($sp)`
- 801C4F54 -> 801C4F58 type=21 target=sub_801C4F58 line=`lw      $s1, 0x10+var_s4($sp)`
- 801C4F58 -> 801C4F5C type=21 target=sub_801C4F5C line=`lw      $s0, 0x10+var_s0($sp)`
- 801C4F5C -> 801C4F60 type=21 target=sub_801C4F60 line=`addiu   $sp, 0x28`
- 801C4F60 -> 801C4F64 type=21 target=sub_801C4F64 line=`jr      $ra`

### Full disasm
```asm
801C4DC4: addiu   $sp, -0x28                                               ; D8 FF BD 27
801C4DC8: sw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 AF
801C4DCC: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
801C4DD0: li      $s0, word_800916D2                                       ; 09 80 10 3C D2 16 10 26
801C4DD8: sw      $ra, 0x10+var_s10($sp)                                   ; 20 00 BF AF
801C4DDC: sw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 AF
801C4DE0: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
801C4DE4: lh      $v0, (word_800916D2 - 0x800916D2)($s0)                   ; 00 00 02 86
801C4DE8: li      $s1, unk_801C3640                                        ; 1C 80 11 3C 40 36 31 26
801C4DF0: bnez    $v0, loc_801C4E14                                        ; 08 00 40 14
801C4DF4: move    $s3, $a0                                                 ; 21 98 80 00
801C4DF8: li      $v0, 1                                                   ; 01 00 02 34
801C4DFC: sh      $v0, (word_800916D2 - 0x800916D2)($s0)                   ; 00 00 02 A6
801C4E00: move    $a0, $s1                                                 ; 21 20 20 02
801C4E04: li      $a1, 2                                                   ; 02 00 05 34
801C4E08: li      $a2, 1                                                   ; 01 00 06 34
801C4E0C: jal     sub_80020110                                             ; 44 80 00 0C
801C4E10: li      $a3, 2                                                   ; 02 00 07 34
801C4E14: move    $a0, $s1                                                 ; 21 20 20 02
801C4E18: li      $a1, 6                                                   ; 06 00 05 34
801C4E1C: li      $a2, 1                                                   ; 01 00 06 34
801C4E20: jal     sub_800201AC                                             ; 6B 80 00 0C
801C4E24: li      $a3, 2                                                   ; 02 00 07 34
801C4E28: lw      $a0, dword_8006ED98+0x20                                 ; 07 80 04 3C B8 ED 84 8C
801C4E30: move    $a1, $zero                                               ; 21 28 00 00
801C4E34: jal     sub_801C44E0                                             ; 38 11 07 0C
801C4E38: addiu   $a0, 0x6C  # 'l'                                         ; 6C 00 84 24
801C4E3C: move    $a1, $s1                                                 ; 21 28 20 02
801C4E40: lw      $a0, dword_8006ED98+0x20                                 ; 07 80 04 3C B8 ED 84 8C
801C4E48: move    $a2, $zero                                               ; 21 30 00 00
801C4E4C: jal     sub_801C455C                                             ; 57 11 07 0C
801C4E50: addiu   $a0, 0x6C  # 'l'                                         ; 6C 00 84 24
801C4E54: jal     sub_8001B120                                             ; 48 6C 00 0C
801C4E58: li      $a0, 1                                                   ; 01 00 04 34
801C4E5C: move    $a0, $s1                                                 ; 21 20 20 02
801C4E60: li      $a1, 5                                                   ; 05 00 05 34
801C4E64: li      $a2, 1                                                   ; 01 00 06 34
801C4E68: li      $a3, 2                                                   ; 02 00 07 34
801C4E6C: li      $s2, 1                                                   ; 01 00 12 34
801C4E70: jal     sub_800201AC                                             ; 6B 80 00 0C
801C4E74: sh      $s2, (word_800916D2 - 0x800916D2)($s0)                   ; 00 00 12 A6
801C4E78: lw      $a0, dword_8006ED98+0x20                                 ; 07 80 04 3C B8 ED 84 8C
801C4E80: move    $a1, $s1                                                 ; 21 28 20 02
801C4E84: jal     sub_801C4894                                             ; 25 12 07 0C
801C4E88: addiu   $a0, 0x9C                                                ; 9C 00 84 24
801C4E8C: jal     sub_80026FA4                                             ; E9 9B 00 0C
801C4E90: move    $s0, $v0                                                 ; 21 80 40 00
801C4E94: move    $a0, $s1                                                 ; 21 20 20 02
801C4E98: li      $a1, 2                                                   ; 02 00 05 34
801C4E9C: li      $a2, 1                                                   ; 01 00 06 34
801C4EA0: jal     sub_80020110                                             ; 44 80 00 0C
801C4EA4: li      $a3, 2                                                   ; 02 00 07 34
801C4EA8: jal     sub_8001EF14                                             ; C5 7B 00 0C
801C4EAC: nop                                                              ; 00 00 00 00
801C4EB0: li      $v0, 1                                                   ; 01 00 02 34
801C4EB4: bne     $s0, $v0, loc_801C4ECC                                   ; 05 00 02 16
801C4EB8: li      $v0, 3                                                   ; 03 00 02 34
801C4EBC: sh      $zero, word_800916D0                                     ; 09 80 01 3C D0 16 20 A4
801C4EC4: j       loc_801C4F48                                             ; D2 13 07 08
801C4EC8: li      $v0, 0xFFFFFFFF                                          ; FF FF 02 24
801C4ECC: bne     $s0, $v0, loc_801C4F24                                   ; 15 00 02 16
801C4ED0: lui     $s0, 0x2AAA                                              ; AA 2A 10 3C
801C4ED4: sh      $s2, word_800916D0                                       ; 09 80 01 3C D0 16 32 A4
801C4EDC: li      $s0, 0x2AAAAAAB                                          ; AB AA 10 36
801C4EE0: jal     rand                                                     ; F7 1F 01 0C
801C4EE4: nop                                                              ; 00 00 00 00
801C4EE8: mult    $v0, $s0                                                 ; 18 00 50 00
801C4EEC: sra     $a0, $v0, 31                                             ; C3 27 02 00
801C4EF0: mfhi    $t0                                                      ; 10 40 00 00
801C4EF4: subu    $a0, $t0, $a0                                            ; 23 20 04 01
801C4EF8: sll     $v1, $a0, 1                                              ; 40 18 04 00
801C4EFC: addu    $v1, $a0                                                 ; 21 18 64 00
801C4F00: sll     $v1, 1                                                   ; 40 18 03 00
801C4F04: subu    $v0, $v1                                                 ; 23 10 43 00
801C4F08: lh      $v1, word_800916EE                                       ; 09 80 03 3C EE 16 63 84
801C4F10: addiu   $v0, 1                                                   ; 01 00 42 24
801C4F14: beq     $v0, $v1, loc_801C4EE0                                   ; F2 FF 43 10
801C4F18: nop                                                              ; 00 00 00 00
801C4F1C: j       loc_801C4F48                                             ; D2 13 07 08
801C4F20: nop                                                              ; 00 00 00 00
801C4F24: lw      $a0, dword_80094410                                      ; 09 80 04 3C 10 44 84 8C
801C4F2C: jal     sub_80026EF8                                             ; BE 9B 00 0C
801C4F30: nop                                                              ; 00 00 00 00
801C4F34: jal     sub_80026ECC                                             ; B3 9B 00 0C
801C4F38: nop                                                              ; 00 00 00 00
801C4F3C: addiu   $v0, $s3, 1                                              ; 01 00 62 26
801C4F40: sh      $zero, word_800916D0                                     ; 09 80 01 3C D0 16 20 A4
801C4F48: lw      $ra, 0x10+var_s10($sp)                                   ; 20 00 BF 8F
801C4F4C: lw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 8F
801C4F50: lw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 8F
801C4F54: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
801C4F58: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
801C4F5C: addiu   $sp, 0x28                                                ; 28 00 BD 27
801C4F60: jr      $ra                                                      ; 08 00 E0 03
801C4F64: nop                                                              ; 00 00 00 00
```

## `801C44E0` `sub_801C44E0`
Bounds: `801C44E0-801C455C`

### Pseudocode
```c
int __fastcall sub_801C44E0(int a1, int a2)
{
  int v4; // $a0

  sub_80026FA4(a1);
  nullsub_8();
  sub_80024E98();
  v4 = *(__int16 *)(a1 + 6);
  dword_801C9554 = a2 != 1;
  sub_8001A478(v4);
  sub_80027288(a2);
  sub_8001A4D0(a1, 1);
  return sub_800274D4();
}

```

### Calls in function
- 801C44F4: `jal     sub_80026FA4` -> 80026FA4 sub_80026FA4
- 801C44FC: `jal     nullsub_8` -> 801C6094 nullsub_8
- 801C4504: `jal     sub_80024E98` -> 80024E98 sub_80024E98
- 801C4520: `jal     sub_8001A478` -> 8001A478 sub_8001A478
- 801C4528: `jal     sub_80027288` -> 80027288 sub_80027288
- 801C4534: `jal     sub_8001A4D0` -> 8001A4D0 sub_8001A4D0
- 801C453C: `jal     sub_800274D4` -> 800274D4 sub_800274D4

### Xrefs to
- 801C48F4 -> 801C44E0 type=17 func=sub_801C4894 line=`jal     sub_801C44E0`
- 801C4E34 -> 801C44E0 type=17 func=sub_801C4DC4 line=`jal     sub_801C44E0`

### Xrefs from
- 801C44E0 -> 801C44E4 type=21 target=sub_801C44E4 line=`addiu   $sp, -0x20`
- 801C44E4 -> 801C44E8 type=21 target=sub_801C44E8 line=`sw      $s1, 0x10+var_s4($sp)`
- 801C44E8 -> 801C44EC type=21 target=sub_801C44EC line=`move    $s1, $a0`
- 801C44EC -> 801C44F0 type=21 target=sub_801C44F0 line=`sw      $s0, 0x10+var_s0($sp)`
- 801C44F0 -> 801C44F4 type=21 target=sub_801C44F4 line=`sw      $ra, 0x10+var_s8($sp)`
- 801C44F4 -> 801C44F8 type=21 target=sub_801C44F8 line=`jal     sub_80026FA4`
- 801C44F4 -> 80026FA4 type=17 target=sub_80026FA4 line=`jal     sub_80026FA4`
- 801C44F8 -> 801C44FC type=21 target=sub_801C44FC line=`move    $s0, $a1`
- 801C44FC -> 801C4500 type=21 target=sub_801C4500 line=`jal     nullsub_8`
- 801C44FC -> 801C6094 type=17 target=nullsub_8 line=`jal     nullsub_8`
- 801C4500 -> 801C4504 type=21 target=sub_801C4504 line=`nop`
- 801C4504 -> 801C4508 type=21 target=sub_801C4508 line=`jal     sub_80024E98`
- 801C4504 -> 80024E98 type=17 target=sub_80024E98 line=`jal     sub_80024E98`
- 801C4508 -> 801C450C type=21 target=sub_801C450C line=`nop`
- 801C450C -> 801C4510 type=21 target=sub_801C4510 line=`xori    $v0, $s0, 1`
- 801C4510 -> 801C4514 type=21 target=sub_801C4514 line=`lh      $a0, 6($s1)`
- 801C4514 -> 801C4518 type=21 target=sub_801C4518 line=`sltu    $v0, $zero, $v0`
- 801C4518 -> 801C4520 type=21 target=sub_801C4520 line=`sw      $v0, dword_801C9554`
- 801C4520 -> 801C4524 type=21 target=sub_801C4524 line=`jal     sub_8001A478`
- 801C4520 -> 8001A478 type=17 target=sub_8001A478 line=`jal     sub_8001A478`
- 801C4524 -> 801C4528 type=21 target=sub_801C4528 line=`nop`
- 801C4528 -> 801C452C type=21 target=sub_801C452C line=`jal     sub_80027288`
- 801C4528 -> 80027288 type=17 target=sub_80027288 line=`jal     sub_80027288`
- 801C452C -> 801C4530 type=21 target=sub_801C4530 line=`move    $a0, $s0`
- 801C4530 -> 801C4534 type=21 target=sub_801C4534 line=`move    $a0, $s1`
- 801C4534 -> 801C4538 type=21 target=sub_801C4538 line=`jal     sub_8001A4D0`
- 801C4534 -> 8001A4D0 type=17 target=sub_8001A4D0 line=`jal     sub_8001A4D0`
- 801C4538 -> 801C453C type=21 target=sub_801C453C line=`li      $a1, 1`
- 801C453C -> 801C4540 type=21 target=sub_801C4540 line=`jal     sub_800274D4`
- 801C453C -> 800274D4 type=17 target=sub_800274D4 line=`jal     sub_800274D4`
- 801C4540 -> 801C4544 type=21 target=sub_801C4544 line=`nop`
- 801C4544 -> 801C4548 type=21 target=sub_801C4548 line=`lw      $ra, 0x10+var_s8($sp)`
- 801C4548 -> 801C454C type=21 target=sub_801C454C line=`lw      $s1, 0x10+var_s4($sp)`
- 801C454C -> 801C4550 type=21 target=sub_801C4550 line=`lw      $s0, 0x10+var_s0($sp)`
- 801C4550 -> 801C4554 type=21 target=sub_801C4554 line=`addiu   $sp, 0x20`
- 801C4554 -> 801C4558 type=21 target=sub_801C4558 line=`jr      $ra`

### Full disasm
```asm
801C44E0: addiu   $sp, -0x20                                               ; E0 FF BD 27
801C44E4: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
801C44E8: move    $s1, $a0                                                 ; 21 88 80 00
801C44EC: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
801C44F0: sw      $ra, 0x10+var_s8($sp)                                    ; 18 00 BF AF
801C44F4: jal     sub_80026FA4                                             ; E9 9B 00 0C
801C44F8: move    $s0, $a1                                                 ; 21 80 A0 00
801C44FC: jal     nullsub_8                                                ; 25 18 07 0C
801C4500: nop                                                              ; 00 00 00 00
801C4504: jal     sub_80024E98                                             ; A6 93 00 0C
801C4508: nop                                                              ; 00 00 00 00
801C450C: xori    $v0, $s0, 1                                              ; 01 00 02 3A
801C4510: lh      $a0, 6($s1)                                              ; 06 00 24 86
801C4514: sltu    $v0, $zero, $v0                                          ; 2B 10 02 00
801C4518: sw      $v0, dword_801C9554                                      ; 1D 80 01 3C 54 95 22 AC
801C4520: jal     sub_8001A478                                             ; 1E 69 00 0C
801C4524: nop                                                              ; 00 00 00 00
801C4528: jal     sub_80027288                                             ; A2 9C 00 0C
801C452C: move    $a0, $s0                                                 ; 21 20 00 02
801C4530: move    $a0, $s1                                                 ; 21 20 20 02
801C4534: jal     sub_8001A4D0                                             ; 34 69 00 0C
801C4538: li      $a1, 1                                                   ; 01 00 05 34
801C453C: jal     sub_800274D4                                             ; 35 9D 00 0C
801C4540: nop                                                              ; 00 00 00 00
801C4544: lw      $ra, 0x10+var_s8($sp)                                    ; 18 00 BF 8F
801C4548: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
801C454C: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
801C4550: addiu   $sp, 0x20                                                ; 20 00 BD 27
801C4554: jr      $ra                                                      ; 08 00 E0 03
801C4558: nop                                                              ; 00 00 00 00
```

## `801C455C` `sub_801C455C`
Bounds: `801C455C-801C4780`

### Pseudocode
```c
int __fastcall sub_801C455C(int a1, PrGameContext_Psx *a2, unsigned int a3)
{
  int *v5; // $a0
  int v6; // $s3
  int v7; // $s1
  __int16 v8; // $v0
  int v9; // $v1
  BOOL v10; // $s1

  if ( a3 >= 4 )
    v5 = 0;
  else
    v5 = (int *)(dword_800943CC + 28 * a3);
  v7 = 1800;
  sub_80024C84(v5);
  dword_801C954C = 0;
  do
  {
    if ( sub_8001A750() == 1 )
      break;
    if ( sub_8001EC54 )
      sub_8001EC54(a2, 7);
    sub_80035560(2);
    if ( sub_8001ED3C )
      sub_8001ED3C();
    --v7;
  }
  while ( v7 > 0 );
  v6 = 0;
  sub_8001A280();
  *(_DWORD *)&a2->_pad0[12] = MEMORY[0x164];
  *(_WORD *)&a2->_pad0[102] = word_800916D8;
  v8 = word_800916DC;
  *(_WORD *)&a2->_pad0[104] = word_800916DC;
  *(_WORD *)&a2->_pad0[84] = v8;
  while ( 1 )
  {
    *(_DWORD *)a2->_pad0 = 0;
    v9 = sub_80035510();
    if ( v9 )
    {
      if ( v9 == 256 )
      {
        v6 = 1;
        goto LABEL_24;
      }
      if ( v9 == 2048 || (v9 & 0x840) != 0 )
        goto LABEL_24;
    }
    sub_80024CF8(a2);
    sub_80027528();
    if ( dword_801C9554 == 1 )
      sub_8001EC54(a2, 7);
    sub_80027528();
    v10 = sub_801C448C(a1, (int)a2);
    sub_8001ED74();
    sub_8002756C();
    if ( dword_801C9554 == 1 )
      sub_8001ED3C();
    if ( GsGetWorkBase() == 1 )
      break;
    if ( !v10 )
      goto LABEL_24;
  }
  sub_8001A694();
  StopCallback();
LABEL_24:
  sub_80027664();
  sub_8001A4A4(1);
  sub_8001A694();
  nullsub_4(a2);
  sub_8001B120(1);
  return v6;
}

```

### Calls in function
- 801C4584: `beqz    $v0, loc_801C45C8` -> 801C45C8 loc_801C45C8
- 801C45A0: `j       loc_801C45CC` -> 801C45CC loc_801C45CC
- 801C45A8: `j       loc_801C472C` -> 801C472C loc_801C472C
- 801C45B0: `jal     sub_8001A694` -> 8001A694 sub_8001A694
- 801C45B8: `jal     StopCallback` -> 80035838 StopCallback
- 801C45C0: `j       loc_801C472C` -> 801C472C loc_801C472C
- 801C45CC: `jal     sub_80024C84` -> 80024C84 sub_80024C84
- 801C45F0: `jal     sub_8001A750` -> 8001A750 sub_8001A750
- 801C45F8: `beq     $v0, $s2, loc_801C4634` -> 801C4634 loc_801C4634
- 801C4600: `beqz    $s4, loc_801C4610` -> 801C4610 loc_801C4610
- 801C4608: `jal     sub_8001EC54` -> 8001EC54 sub_8001EC54
- 801C4610: `jal     sub_80035560` -> 80035560 sub_80035560
- 801C4618: `beqz    $s3, loc_801C4628` -> 801C4628 loc_801C4628
- 801C4620: `jal     sub_8001ED3C` -> 8001ED3C sub_8001ED3C
- 801C462C: `bgtz    $s1, loc_801C45F0` -> 801C45F0 loc_801C45F0
- 801C4634: `jal     sub_8001A280` -> 8001A280 sub_8001A280
- 801C467C: `jal     sub_80035510` -> 80035510 sub_80035510
- 801C4688: `beqz    $v1, loc_801C46A8` -> 801C46A8 loc_801C46A8
- 801C4690: `beq     $v1, $v0, loc_801C45A8` -> 801C45A8 loc_801C45A8
- 801C4698: `beq     $v1, $v0, loc_801C472C` -> 801C472C loc_801C472C
- 801C46A0: `bnez    $v0, loc_801C472C` -> 801C472C loc_801C472C
- 801C46A8: `jal     sub_80024CF8` -> 80024CF8 sub_80024CF8
- 801C46B0: `jal     sub_80027528` -> 80027528 sub_80027528
- 801C46C4: `bne     $v0, $s2, loc_801C46D4` -> 801C46D4 loc_801C46D4
- 801C46CC: `jal     sub_8001EC54` -> 8001EC54 sub_8001EC54
- 801C46D4: `jal     sub_80027528` -> 80027528 sub_80027528
- 801C46E0: `jal     sub_801C448C` -> 801C448C sub_801C448C
- 801C46E8: `jal     sub_8001ED74` -> 8001ED74 sub_8001ED74
- 801C46F0: `jal     sub_8002756C` -> 8002756C sub_8002756C
- 801C4704: `bne     $v0, $s2, loc_801C4714` -> 801C4714 loc_801C4714
- 801C470C: `jal     sub_8001ED3C` -> 8001ED3C sub_8001ED3C
- 801C4714: `jal     GsGetWorkBase` -> 8001A3B8 GsGetWorkBase
- 801C471C: `beq     $v0, $s2, loc_801C45B0` -> 801C45B0 loc_801C45B0
- 801C4724: `beq     $s1, $s2, loc_801C4678` -> 801C4678 loc_801C4678
- 801C472C: `jal     sub_80027664` -> 80027664 sub_80027664
- 801C4734: `jal     sub_8001A4A4` -> 8001A4A4 sub_8001A4A4
- 801C473C: `jal     sub_8001A694` -> 8001A694 sub_8001A694
- 801C4744: `jal     nullsub_4` -> 80024CF0 nullsub_4
- 801C474C: `jal     sub_8001B120` -> 8001B120 sub_8001B120

### Xrefs to
- 801C4E4C -> 801C455C type=17 func=sub_801C4DC4 line=`jal     sub_801C455C`

### Xrefs from
- 801C455C -> 801C4560 type=21 target=sub_801C4560 line=`addiu   $sp, -0x30`
- 801C4560 -> 801C4564 type=21 target=sub_801C4564 line=`sw      $s5, 0x10+var_s14($sp)`
- 801C4564 -> 801C4568 type=21 target=sub_801C4568 line=`move    $s5, $a0`
- 801C4568 -> 801C456C type=21 target=sub_801C456C line=`sw      $s0, 0x10+var_s0($sp)`
- 801C456C -> 801C4570 type=21 target=sub_801C4570 line=`move    $s0, $a1`
- 801C4570 -> 801C4574 type=21 target=sub_801C4574 line=`sltiu   $v0, $a2, 4`
- 801C4574 -> 801C4578 type=21 target=sub_801C4578 line=`sw      $ra, 0x10+var_s18($sp)`
- 801C4578 -> 801C457C type=21 target=sub_801C457C line=`sw      $s4, 0x10+var_s10($sp)`
- 801C457C -> 801C4580 type=21 target=sub_801C4580 line=`sw      $s3, 0x10+var_sC($sp)`
- 801C4580 -> 801C4584 type=21 target=sub_801C4584 line=`sw      $s2, 0x10+var_s8($sp)`
- 801C4584 -> 801C4588 type=21 target=sub_801C4588 line=`beqz    $v0, loc_801C45C8`
- 801C4584 -> 801C45C8 type=19 target=loc_801C45C8 line=`beqz    $v0, loc_801C45C8`
- 801C4588 -> 801C458C type=21 target=sub_801C458C line=`sw      $s1, 0x10+var_s4($sp)`
- 801C458C -> 801C4590 type=21 target=sub_801C4590 line=`sll     $a0, $a2, 3`
- 801C4590 -> 801C4594 type=21 target=sub_801C4594 line=`subu    $a0, $a2`
- 801C4594 -> 801C459C type=21 target=sub_801C459C line=`lw      $v0, dword_800943CC`
- 801C459C -> 801C45A0 type=21 target=sub_801C45A0 line=`sll     $a0, 2`
- 801C45A0 -> 801C45A4 type=21 target=sub_801C45A4 line=`j       loc_801C45CC`
- 801C45A0 -> 801C45CC type=19 target=loc_801C45CC line=`j       loc_801C45CC`
- 801C45A8 -> 801C45AC type=21 target=sub_801C45AC line=`j       loc_801C472C`
- 801C45A8 -> 801C472C type=19 target=loc_801C472C line=`j       loc_801C472C`
- 801C45B0 -> 801C45B4 type=21 target=sub_801C45B4 line=`jal     sub_8001A694`
- 801C45B0 -> 8001A694 type=17 target=sub_8001A694 line=`jal     sub_8001A694`
- 801C45B4 -> 801C45B8 type=21 target=sub_801C45B8 line=`nop`
- 801C45B8 -> 801C45BC type=21 target=sub_801C45BC line=`jal     StopCallback`
- 801C45B8 -> 80035838 type=17 target=StopCallback line=`jal     StopCallback`
- 801C45BC -> 801C45C0 type=21 target=sub_801C45C0 line=`nop`
- 801C45C0 -> 801C45C4 type=21 target=sub_801C45C4 line=`j       loc_801C472C`
- 801C45C0 -> 801C472C type=19 target=loc_801C472C line=`j       loc_801C472C`
- 801C45C8 -> 801C45CC type=21 target=loc_801C45CC line=`move    $a0, $zero`
- 801C45CC -> 801C45D0 type=21 target=sub_801C45D0 line=`jal     sub_80024C84`
- 801C45CC -> 80024C84 type=17 target=sub_80024C84 line=`jal     sub_80024C84`
- 801C45D0 -> 801C45D4 type=21 target=sub_801C45D4 line=`li      $s1, 0x708`
- 801C45D4 -> 801C45DC type=21 target=sub_801C45DC line=`li      $s4, sub_8001EC54`
- 801C45D4 -> 8001EC54 type=1 target=sub_8001EC54 line=`li      $s4, sub_8001EC54`
- 801C45DC -> 801C45E4 type=21 target=sub_801C45E4 line=`li      $s3, sub_8001ED3C`
- 801C45DC -> 8001ED3C type=1 target=sub_8001ED3C line=`li      $s3, sub_8001ED3C`
- 801C45E4 -> 801C45EC type=21 target=sub_801C45EC line=`sw      $zero, dword_801C954C`
- 801C45EC -> 801C45F0 type=21 target=loc_801C45F0 line=`li      $s2, 1`
- 801C45F0 -> 801C45F4 type=21 target=sub_801C45F4 line=`jal     sub_8001A750`
- 801C45F0 -> 8001A750 type=17 target=sub_8001A750 line=`jal     sub_8001A750`
- 801C45F4 -> 801C45F8 type=21 target=sub_801C45F8 line=`nop`
- 801C45F8 -> 801C45FC type=21 target=sub_801C45FC line=`beq     $v0, $s2, loc_801C4634`
- 801C45F8 -> 801C4634 type=19 target=loc_801C4634 line=`beq     $v0, $s2, loc_801C4634`
- 801C45FC -> 801C4600 type=21 target=sub_801C4600 line=`nop`
- 801C4600 -> 801C4604 type=21 target=sub_801C4604 line=`beqz    $s4, loc_801C4610`
- 801C4600 -> 801C4610 type=19 target=loc_801C4610 line=`beqz    $s4, loc_801C4610`
- 801C4604 -> 801C4608 type=21 target=sub_801C4608 line=`move    $a0, $s0`
- 801C4608 -> 801C460C type=21 target=sub_801C460C line=`jal     sub_8001EC54`
- 801C4608 -> 8001EC54 type=17 target=sub_8001EC54 line=`jal     sub_8001EC54`
- 801C460C -> 801C4610 type=21 target=loc_801C4610 line=`li      $a1, 7`
- 801C4610 -> 801C4614 type=21 target=sub_801C4614 line=`jal     sub_80035560`
- 801C4610 -> 80035560 type=17 target=sub_80035560 line=`jal     sub_80035560`
- 801C4614 -> 801C4618 type=21 target=sub_801C4618 line=`li      $a0, 2`
- 801C4618 -> 801C461C type=21 target=sub_801C461C line=`beqz    $s3, loc_801C4628`
- 801C4618 -> 801C4628 type=19 target=loc_801C4628 line=`beqz    $s3, loc_801C4628`
- 801C461C -> 801C4620 type=21 target=sub_801C4620 line=`nop`
- 801C4620 -> 801C4624 type=21 target=sub_801C4624 line=`jal     sub_8001ED3C`
- 801C4620 -> 8001ED3C type=17 target=sub_8001ED3C line=`jal     sub_8001ED3C`
- 801C4624 -> 801C4628 type=21 target=loc_801C4628 line=`move    $a0, $s0`
- 801C4628 -> 801C462C type=21 target=sub_801C462C line=`addiu   $s1, -1`
- 801C462C -> 801C4630 type=21 target=sub_801C4630 line=`bgtz    $s1, loc_801C45F0`
- 801C462C -> 801C45F0 type=19 target=loc_801C45F0 line=`bgtz    $s1, loc_801C45F0`
- 801C4630 -> 801C4634 type=21 target=loc_801C4634 line=`nop`
- 801C4634 -> 801C4638 type=21 target=sub_801C4638 line=`jal     sub_8001A280`
- 801C4634 -> 8001A280 type=17 target=sub_8001A280 line=`jal     sub_8001A280`
- 801C4638 -> 801C463C type=21 target=sub_801C463C line=`move    $s3, $zero`
- 801C463C -> 801C4644 type=21 target=sub_801C4644 line=`lw      $v0, dword_8006ED98+0x20`
- 801C4644 -> 801C4648 type=21 target=sub_801C4648 line=`nop`
- 801C4648 -> 801C464C type=21 target=sub_801C464C line=`lw      $v0, 0x164($v0)`
- 801C464C -> 801C4650 type=21 target=sub_801C4650 line=`nop`
- 801C4650 -> 801C4654 type=21 target=sub_801C4654 line=`sw      $v0, 0xC($s0)`
- 801C4654 -> 801C465C type=21 target=sub_801C465C line=`lhu     $v0, word_800916D8`
- 801C465C -> 801C4660 type=21 target=sub_801C4660 line=`nop`
- 801C4660 -> 801C4664 type=21 target=sub_801C4664 line=`sh      $v0, 0x66($s0)`
- 801C4664 -> 801C466C type=21 target=sub_801C466C line=`lhu     $v0, word_800916DC`
- 801C466C -> 801C4670 type=21 target=sub_801C4670 line=`li      $s2, 1`
- 801C4670 -> 801C4674 type=21 target=sub_801C4674 line=`sh      $v0, 0x68($s0)`
- 801C4674 -> 801C4678 type=21 target=loc_801C4678 line=`sh      $v0, 0x54($s0)`
- 801C4678 -> 801C467C type=21 target=sub_801C467C line=`sw      $zero, 0($s0)`
- 801C467C -> 801C4680 type=21 target=sub_801C4680 line=`jal     sub_80035510`
- 801C467C -> 80035510 type=17 target=sub_80035510 line=`jal     sub_80035510`
- 801C4680 -> 801C4684 type=21 target=sub_801C4684 line=`li      $a0, 1`
- 801C4684 -> 801C4688 type=21 target=sub_801C4688 line=`move    $v1, $v0`
- 801C4688 -> 801C468C type=21 target=sub_801C468C line=`beqz    $v1, loc_801C46A8`
- 801C4688 -> 801C46A8 type=19 target=loc_801C46A8 line=`beqz    $v1, loc_801C46A8`
- 801C468C -> 801C4690 type=21 target=sub_801C4690 line=`li      $v0, 0x100`
- 801C4690 -> 801C4694 type=21 target=sub_801C4694 line=`beq     $v1, $v0, loc_801C45A8`
- 801C4690 -> 801C45A8 type=19 target=loc_801C45A8 line=`beq     $v1, $v0, loc_801C45A8`
- 801C4694 -> 801C4698 type=21 target=sub_801C4698 line=`li      $v0, 0x800`
- 801C4698 -> 801C469C type=21 target=sub_801C469C line=`beq     $v1, $v0, loc_801C472C`
- 801C4698 -> 801C472C type=19 target=loc_801C472C line=`beq     $v1, $v0, loc_801C472C`
- 801C469C -> 801C46A0 type=21 target=sub_801C46A0 line=`andi    $v0, $v1, 0x840`
- 801C46A0 -> 801C46A4 type=21 target=sub_801C46A4 line=`bnez    $v0, loc_801C472C`
- 801C46A0 -> 801C472C type=19 target=loc_801C472C line=`bnez    $v0, loc_801C472C`
- 801C46A4 -> 801C46A8 type=21 target=loc_801C46A8 line=`nop`
- 801C46A8 -> 801C46AC type=21 target=sub_801C46AC line=`jal     sub_80024CF8`
- 801C46A8 -> 80024CF8 type=17 target=sub_80024CF8 line=`jal     sub_80024CF8`
- 801C46AC -> 801C46B0 type=21 target=sub_801C46B0 line=`move    $a0, $s0`
- 801C46B0 -> 801C46B4 type=21 target=sub_801C46B4 line=`jal     sub_80027528`
- 801C46B0 -> 80027528 type=17 target=sub_80027528 line=`jal     sub_80027528`
- 801C46B4 -> 801C46B8 type=21 target=sub_801C46B8 line=`nop`
- 801C46B8 -> 801C46C0 type=21 target=sub_801C46C0 line=`lw      $v0, dword_801C9554`
- 801C46C0 -> 801C46C4 type=21 target=sub_801C46C4 line=`nop`
- 801C46C4 -> 801C46C8 type=21 target=sub_801C46C8 line=`bne     $v0, $s2, loc_801C46D4`
- 801C46C4 -> 801C46D4 type=19 target=loc_801C46D4 line=`bne     $v0, $s2, loc_801C46D4`
- 801C46C8 -> 801C46CC type=21 target=sub_801C46CC line=`move    $a0, $s0`
- 801C46CC -> 801C46D0 type=21 target=sub_801C46D0 line=`jal     sub_8001EC54`
- 801C46CC -> 8001EC54 type=17 target=sub_8001EC54 line=`jal     sub_8001EC54`
- 801C46D0 -> 801C46D4 type=21 target=loc_801C46D4 line=`li      $a1, 7`
- 801C46D4 -> 801C46D8 type=21 target=sub_801C46D8 line=`jal     sub_80027528`
- 801C46D4 -> 80027528 type=17 target=sub_80027528 line=`jal     sub_80027528`
- 801C46D8 -> 801C46DC type=21 target=sub_801C46DC line=`nop`
- 801C46DC -> 801C46E0 type=21 target=sub_801C46E0 line=`move    $a0, $s5`
- 801C46E0 -> 801C46E4 type=21 target=sub_801C46E4 line=`jal     sub_801C448C`
- 801C46E0 -> 801C448C type=17 target=sub_801C448C line=`jal     sub_801C448C`
- 801C46E4 -> 801C46E8 type=21 target=sub_801C46E8 line=`move    $a1, $s0`
- 801C46E8 -> 801C46EC type=21 target=sub_801C46EC line=`jal     sub_8001ED74`
- 801C46E8 -> 8001ED74 type=17 target=sub_8001ED74 line=`jal     sub_8001ED74`
- 801C46EC -> 801C46F0 type=21 target=sub_801C46F0 line=`move    $s1, $v0`
- 801C46F0 -> 801C46F4 type=21 target=sub_801C46F4 line=`jal     sub_8002756C`
- 801C46F0 -> 8002756C type=17 target=sub_8002756C line=`jal     sub_8002756C`
- 801C46F4 -> 801C46F8 type=21 target=sub_801C46F8 line=`nop`
- 801C46F8 -> 801C4700 type=21 target=sub_801C4700 line=`lw      $v0, dword_801C9554`
- 801C4700 -> 801C4704 type=21 target=sub_801C4704 line=`nop`
- 801C4704 -> 801C4708 type=21 target=sub_801C4708 line=`bne     $v0, $s2, loc_801C4714`
- 801C4704 -> 801C4714 type=19 target=loc_801C4714 line=`bne     $v0, $s2, loc_801C4714`
- 801C4708 -> 801C470C type=21 target=sub_801C470C line=`nop`
- 801C470C -> 801C4710 type=21 target=sub_801C4710 line=`jal     sub_8001ED3C`
- 801C470C -> 8001ED3C type=17 target=sub_8001ED3C line=`jal     sub_8001ED3C`
- 801C4710 -> 801C4714 type=21 target=loc_801C4714 line=`move    $a0, $s0`
- 801C4714 -> 801C4718 type=21 target=sub_801C4718 line=`jal     GsGetWorkBase`
- 801C4714 -> 8001A3B8 type=17 target=GsGetWorkBase line=`jal     GsGetWorkBase`
- 801C4718 -> 801C471C type=21 target=sub_801C471C line=`nop`
- 801C471C -> 801C4720 type=21 target=sub_801C4720 line=`beq     $v0, $s2, loc_801C45B0`
- 801C471C -> 801C45B0 type=19 target=loc_801C45B0 line=`beq     $v0, $s2, loc_801C45B0`
- 801C4720 -> 801C4724 type=21 target=sub_801C4724 line=`nop`
- 801C4724 -> 801C4728 type=21 target=sub_801C4728 line=`beq     $s1, $s2, loc_801C4678`
- 801C4724 -> 801C4678 type=19 target=loc_801C4678 line=`beq     $s1, $s2, loc_801C4678`
- 801C4728 -> 801C472C type=21 target=loc_801C472C line=`nop`
- 801C472C -> 801C4730 type=21 target=sub_801C4730 line=`jal     sub_80027664`
- 801C472C -> 80027664 type=17 target=sub_80027664 line=`jal     sub_80027664`
- 801C4730 -> 801C4734 type=21 target=sub_801C4734 line=`nop`
- 801C4734 -> 801C4738 type=21 target=sub_801C4738 line=`jal     sub_8001A4A4`
- 801C4734 -> 8001A4A4 type=17 target=sub_8001A4A4 line=`jal     sub_8001A4A4`
- 801C4738 -> 801C473C type=21 target=sub_801C473C line=`li      $a0, 1`
- 801C473C -> 801C4740 type=21 target=sub_801C4740 line=`jal     sub_8001A694`
- 801C473C -> 8001A694 type=17 target=sub_8001A694 line=`jal     sub_8001A694`
- 801C4740 -> 801C4744 type=21 target=sub_801C4744 line=`nop`
- 801C4744 -> 801C4748 type=21 target=sub_801C4748 line=`jal     nullsub_4`
- 801C4744 -> 80024CF0 type=17 target=nullsub_4 line=`jal     nullsub_4`
- 801C4748 -> 801C474C type=21 target=sub_801C474C line=`move    $a0, $s0`
- 801C474C -> 801C4750 type=21 target=sub_801C4750 line=`jal     sub_8001B120`
- 801C474C -> 8001B120 type=17 target=sub_8001B120 line=`jal     sub_8001B120`
- 801C4750 -> 801C4754 type=21 target=sub_801C4754 line=`li      $a0, 1`
- 801C4754 -> 801C4758 type=21 target=sub_801C4758 line=`move    $v0, $s3`
- 801C4758 -> 801C475C type=21 target=sub_801C475C line=`lw      $ra, 0x10+var_s18($sp)`
- 801C475C -> 801C4760 type=21 target=sub_801C4760 line=`lw      $s5, 0x10+var_s14($sp)`
- 801C4760 -> 801C4764 type=21 target=sub_801C4764 line=`lw      $s4, 0x10+var_s10($sp)`
- 801C4764 -> 801C4768 type=21 target=sub_801C4768 line=`lw      $s3, 0x10+var_sC($sp)`
- 801C4768 -> 801C476C type=21 target=sub_801C476C line=`lw      $s2, 0x10+var_s8($sp)`
- 801C476C -> 801C4770 type=21 target=sub_801C4770 line=`lw      $s1, 0x10+var_s4($sp)`
- 801C4770 -> 801C4774 type=21 target=sub_801C4774 line=`lw      $s0, 0x10+var_s0($sp)`
- 801C4774 -> 801C4778 type=21 target=sub_801C4778 line=`addiu   $sp, 0x30`
- 801C4778 -> 801C477C type=21 target=sub_801C477C line=`jr      $ra`

### Full disasm
```asm
801C455C: addiu   $sp, -0x30                                               ; D0 FF BD 27
801C4560: sw      $s5, 0x10+var_s14($sp)                                   ; 24 00 B5 AF
801C4564: move    $s5, $a0                                                 ; 21 A8 80 00
801C4568: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
801C456C: move    $s0, $a1                                                 ; 21 80 A0 00
801C4570: sltiu   $v0, $a2, 4                                              ; 04 00 C2 2C
801C4574: sw      $ra, 0x10+var_s18($sp)                                   ; 28 00 BF AF
801C4578: sw      $s4, 0x10+var_s10($sp)                                   ; 20 00 B4 AF
801C457C: sw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 AF
801C4580: sw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 AF
801C4584: beqz    $v0, loc_801C45C8                                        ; 10 00 40 10
801C4588: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
801C458C: sll     $a0, $a2, 3                                              ; C0 20 06 00
801C4590: subu    $a0, $a2                                                 ; 23 20 86 00
801C4594: lw      $v0, dword_800943CC                                      ; 09 80 02 3C CC 43 42 8C
801C459C: sll     $a0, 2                                                   ; 80 20 04 00
801C45A0: j       loc_801C45CC                                             ; 73 11 07 08
801C45A4: addu    $a0, $v0, $a0                                            ; 21 20 44 00
801C45A8: j       loc_801C472C                                             ; CB 11 07 08
801C45AC: li      $s3, 1                                                   ; 01 00 13 34
801C45B0: jal     sub_8001A694                                             ; A5 69 00 0C
801C45B4: nop                                                              ; 00 00 00 00
801C45B8: jal     StopCallback                                             ; 0E D6 00 0C
801C45BC: nop                                                              ; 00 00 00 00
801C45C0: j       loc_801C472C                                             ; CB 11 07 08
801C45C4: nop                                                              ; 00 00 00 00
801C45C8: move    $a0, $zero                                               ; 21 20 00 00
801C45CC: jal     sub_80024C84                                             ; 21 93 00 0C
801C45D0: li      $s1, 0x708                                               ; 08 07 11 34
801C45D4: li      $s4, sub_8001EC54                                        ; 02 80 14 3C 54 EC 94 26
801C45DC: li      $s3, sub_8001ED3C                                        ; 02 80 13 3C 3C ED 73 26
801C45E4: sw      $zero, dword_801C954C                                    ; 1D 80 01 3C 4C 95 20 AC
801C45EC: li      $s2, 1                                                   ; 01 00 12 34
801C45F0: jal     sub_8001A750                                             ; D4 69 00 0C
801C45F4: nop                                                              ; 00 00 00 00
801C45F8: beq     $v0, $s2, loc_801C4634                                   ; 0E 00 52 10
801C45FC: nop                                                              ; 00 00 00 00
801C4600: beqz    $s4, loc_801C4610                                        ; 03 00 80 12
801C4604: move    $a0, $s0                                                 ; 21 20 00 02
801C4608: jal     sub_8001EC54                                             ; 15 7B 00 0C
801C460C: li      $a1, 7                                                   ; 07 00 05 34
801C4610: jal     sub_80035560                                             ; 58 D5 00 0C
801C4614: li      $a0, 2                                                   ; 02 00 04 34
801C4618: beqz    $s3, loc_801C4628                                        ; 03 00 60 12
801C461C: nop                                                              ; 00 00 00 00
801C4620: jal     sub_8001ED3C                                             ; 4F 7B 00 0C
801C4624: move    $a0, $s0                                                 ; 21 20 00 02
801C4628: addiu   $s1, -1                                                  ; FF FF 31 26
801C462C: bgtz    $s1, loc_801C45F0                                        ; F0 FF 20 1E
801C4630: nop                                                              ; 00 00 00 00
801C4634: jal     sub_8001A280                                             ; A0 68 00 0C
801C4638: move    $s3, $zero                                               ; 21 98 00 00
801C463C: lw      $v0, dword_8006ED98+0x20                                 ; 07 80 02 3C B8 ED 42 8C
801C4644: nop                                                              ; 00 00 00 00
801C4648: lw      $v0, 0x164($v0)                                          ; 64 01 42 8C
801C464C: nop                                                              ; 00 00 00 00
801C4650: sw      $v0, 0xC($s0)                                            ; 0C 00 02 AE
801C4654: lhu     $v0, word_800916D8                                       ; 09 80 02 3C D8 16 42 94
801C465C: nop                                                              ; 00 00 00 00
801C4660: sh      $v0, 0x66($s0)                                           ; 66 00 02 A6
801C4664: lhu     $v0, word_800916DC                                       ; 09 80 02 3C DC 16 42 94
801C466C: li      $s2, 1                                                   ; 01 00 12 34
801C4670: sh      $v0, 0x68($s0)                                           ; 68 00 02 A6
801C4674: sh      $v0, 0x54($s0)                                           ; 54 00 02 A6
801C4678: sw      $zero, 0($s0)                                            ; 00 00 00 AE
801C467C: jal     sub_80035510                                             ; 44 D5 00 0C
801C4680: li      $a0, 1                                                   ; 01 00 04 34
801C4684: move    $v1, $v0                                                 ; 21 18 40 00
801C4688: beqz    $v1, loc_801C46A8                                        ; 07 00 60 10
801C468C: li      $v0, 0x100                                               ; 00 01 02 34
801C4690: beq     $v1, $v0, loc_801C45A8                                   ; C5 FF 62 10
801C4694: li      $v0, 0x800                                               ; 00 08 02 34
801C4698: beq     $v1, $v0, loc_801C472C                                   ; 24 00 62 10
801C469C: andi    $v0, $v1, 0x840                                          ; 40 08 62 30
801C46A0: bnez    $v0, loc_801C472C                                        ; 22 00 40 14
801C46A4: nop                                                              ; 00 00 00 00
801C46A8: jal     sub_80024CF8                                             ; 3E 93 00 0C
801C46AC: move    $a0, $s0                                                 ; 21 20 00 02
801C46B0: jal     sub_80027528                                             ; 4A 9D 00 0C
801C46B4: nop                                                              ; 00 00 00 00
801C46B8: lw      $v0, dword_801C9554                                      ; 1D 80 02 3C 54 95 42 8C
801C46C0: nop                                                              ; 00 00 00 00
801C46C4: bne     $v0, $s2, loc_801C46D4                                   ; 03 00 52 14
801C46C8: move    $a0, $s0                                                 ; 21 20 00 02
801C46CC: jal     sub_8001EC54                                             ; 15 7B 00 0C
801C46D0: li      $a1, 7                                                   ; 07 00 05 34
801C46D4: jal     sub_80027528                                             ; 4A 9D 00 0C
801C46D8: nop                                                              ; 00 00 00 00
801C46DC: move    $a0, $s5                                                 ; 21 20 A0 02
801C46E0: jal     sub_801C448C                                             ; 23 11 07 0C
801C46E4: move    $a1, $s0                                                 ; 21 28 00 02
801C46E8: jal     sub_8001ED74                                             ; 5D 7B 00 0C
801C46EC: move    $s1, $v0                                                 ; 21 88 40 00
801C46F0: jal     sub_8002756C                                             ; 5B 9D 00 0C
801C46F4: nop                                                              ; 00 00 00 00
801C46F8: lw      $v0, dword_801C9554                                      ; 1D 80 02 3C 54 95 42 8C
801C4700: nop                                                              ; 00 00 00 00
801C4704: bne     $v0, $s2, loc_801C4714                                   ; 03 00 52 14
801C4708: nop                                                              ; 00 00 00 00
801C470C: jal     sub_8001ED3C                                             ; 4F 7B 00 0C
801C4710: move    $a0, $s0                                                 ; 21 20 00 02
801C4714: jal     GsGetWorkBase                                            ; EE 68 00 0C
801C4718: nop                                                              ; 00 00 00 00
801C471C: beq     $v0, $s2, loc_801C45B0                                   ; A4 FF 52 10
801C4720: nop                                                              ; 00 00 00 00
801C4724: beq     $s1, $s2, loc_801C4678                                   ; D4 FF 32 12
801C4728: nop                                                              ; 00 00 00 00
801C472C: jal     sub_80027664                                             ; 99 9D 00 0C
801C4730: nop                                                              ; 00 00 00 00
801C4734: jal     sub_8001A4A4                                             ; 29 69 00 0C
801C4738: li      $a0, 1                                                   ; 01 00 04 34
801C473C: jal     sub_8001A694                                             ; A5 69 00 0C
801C4740: nop                                                              ; 00 00 00 00
801C4744: jal     nullsub_4                                                ; 3C 93 00 0C
801C4748: move    $a0, $s0                                                 ; 21 20 00 02
801C474C: jal     sub_8001B120                                             ; 48 6C 00 0C
801C4750: li      $a0, 1                                                   ; 01 00 04 34
801C4754: move    $v0, $s3                                                 ; 21 10 60 02
801C4758: lw      $ra, 0x10+var_s18($sp)                                   ; 28 00 BF 8F
801C475C: lw      $s5, 0x10+var_s14($sp)                                   ; 24 00 B5 8F
801C4760: lw      $s4, 0x10+var_s10($sp)                                   ; 20 00 B4 8F
801C4764: lw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 8F
801C4768: lw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 8F
801C476C: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
801C4770: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
801C4774: addiu   $sp, 0x30                                                ; 30 00 BD 27
801C4778: jr      $ra                                                      ; 08 00 E0 03
801C477C: nop                                                              ; 00 00 00 00
```

## `800201AC` `sub_800201AC`
Bounds: `800201AC-80020248`

### Pseudocode
```c
int __fastcall sub_800201AC(int a1, int a2, int a3, int a4)
{
  int v7; // $a0
  int v8; // $v0
  int v9; // $gp

  sub_8001FFD4(a3);
  v7 = a1;
  do
  {
    sub_8001EA74(v7, a2);
    sub_80035560(2);
    sub_8001EBF4();
    v8 = sub_8001F518();
    v7 = a1;
  }
  while ( v8 );
  sub_8001FFD4(a4);
  *(_DWORD *)(v9 + 196) = 190;
  return sub_80020090(a1, a2, sub_8001EA74, sub_8001EBF4);
}

```

### Calls in function
- 800201CC: `jal     sub_8001FFD4` -> 8001FFD4 sub_8001FFD4
- 800201D8: `jal     sub_8001EA74` -> 8001EA74 sub_8001EA74
- 800201E0: `jal     sub_80035560` -> 80035560 sub_80035560
- 800201E8: `jal     sub_8001EBF4` -> 8001EBF4 sub_8001EBF4
- 800201F0: `jal     sub_8001F518` -> 8001F518 sub_8001F518
- 800201F8: `bnez    $v0, loc_800201D8` -> 800201D8 loc_800201D8
- 80020200: `jal     sub_8001FFD4` -> 8001FFD4 sub_8001FFD4
- 80020224: `jal     sub_80020090` -> 80020090 sub_80020090

### Xrefs to
- 801C4E20 -> 800201AC type=17 func=sub_801C4DC4 line=`jal     sub_800201AC`
- 801C4E70 -> 800201AC type=17 func=sub_801C4DC4 line=`jal     sub_800201AC`

### Xrefs from
- 800201AC -> 800201B0 type=21 target=sub_800201B0 line=`addiu   $sp, -0x20`
- 800201B0 -> 800201B4 type=21 target=sub_800201B4 line=`sw      $s0, 0x10+var_s0($sp)`
- 800201B4 -> 800201B8 type=21 target=sub_800201B8 line=`move    $s0, $a0`
- 800201B8 -> 800201BC type=21 target=sub_800201BC line=`sw      $s1, 0x10+var_s4($sp)`
- 800201BC -> 800201C0 type=21 target=sub_800201C0 line=`move    $s1, $a1`
- 800201C0 -> 800201C4 type=21 target=sub_800201C4 line=`move    $a0, $a2`
- 800201C4 -> 800201C8 type=21 target=sub_800201C8 line=`sw      $s2, 0x10+var_s8($sp)`
- 800201C8 -> 800201CC type=21 target=sub_800201CC line=`sw      $ra, 0x10+var_sC($sp)`
- 800201CC -> 800201D0 type=21 target=sub_800201D0 line=`jal     sub_8001FFD4`
- 800201CC -> 8001FFD4 type=17 target=sub_8001FFD4 line=`jal     sub_8001FFD4`
- 800201D0 -> 800201D4 type=21 target=sub_800201D4 line=`move    $s2, $a3`
- 800201D4 -> 800201D8 type=21 target=loc_800201D8 line=`move    $a0, $s0`
- 800201D8 -> 800201DC type=21 target=sub_800201DC line=`jal     sub_8001EA74`
- 800201D8 -> 8001EA74 type=17 target=sub_8001EA74 line=`jal     sub_8001EA74`
- 800201DC -> 800201E0 type=21 target=sub_800201E0 line=`move    $a1, $s1`
- 800201E0 -> 800201E4 type=21 target=sub_800201E4 line=`jal     sub_80035560`
- 800201E0 -> 80035560 type=17 target=sub_80035560 line=`jal     sub_80035560`
- 800201E4 -> 800201E8 type=21 target=sub_800201E8 line=`li      $a0, 2`
- 800201E8 -> 800201EC type=21 target=sub_800201EC line=`jal     sub_8001EBF4`
- 800201E8 -> 8001EBF4 type=17 target=sub_8001EBF4 line=`jal     sub_8001EBF4`
- 800201EC -> 800201F0 type=21 target=sub_800201F0 line=`move    $a0, $s0`
- 800201F0 -> 800201F4 type=21 target=sub_800201F4 line=`jal     sub_8001F518`
- 800201F0 -> 8001F518 type=17 target=sub_8001F518 line=`jal     sub_8001F518`
- 800201F4 -> 800201F8 type=21 target=sub_800201F8 line=`nop`
- 800201F8 -> 800201FC type=21 target=sub_800201FC line=`bnez    $v0, loc_800201D8`
- 800201F8 -> 800201D8 type=19 target=loc_800201D8 line=`bnez    $v0, loc_800201D8`
- 800201FC -> 80020200 type=21 target=sub_80020200 line=`move    $a0, $s0`
- 80020200 -> 80020204 type=21 target=sub_80020204 line=`jal     sub_8001FFD4`
- 80020200 -> 8001FFD4 type=17 target=sub_8001FFD4 line=`jal     sub_8001FFD4`
- 80020204 -> 80020208 type=21 target=sub_80020208 line=`move    $a0, $s2`
- 80020208 -> 8002020C type=21 target=sub_8002020C line=`li      $v0, 0xBE`
- 8002020C -> 80020210 type=21 target=sub_80020210 line=`sw      $v0, 0xC4($gp)`
- 80020210 -> 80020214 type=21 target=sub_80020214 line=`move    $a0, $s0`
- 80020214 -> 8002021C type=21 target=sub_8002021C line=`li      $a2, sub_8001EA74`
- 80020214 -> 8001EA74 type=1 target=sub_8001EA74 line=`li      $a2, sub_8001EA74`
- 8002021C -> 80020224 type=21 target=sub_80020224 line=`li      $a3, sub_8001EBF4`
- 8002021C -> 8001EBF4 type=1 target=sub_8001EBF4 line=`li      $a3, sub_8001EBF4`
- 80020224 -> 80020228 type=21 target=sub_80020228 line=`jal     sub_80020090`
- 80020224 -> 80020090 type=17 target=sub_80020090 line=`jal     sub_80020090`
- 80020228 -> 8002022C type=21 target=sub_8002022C line=`move    $a1, $s1`
- 8002022C -> 80020230 type=21 target=sub_80020230 line=`lw      $ra, 0x10+var_sC($sp)`
- 80020230 -> 80020234 type=21 target=sub_80020234 line=`lw      $s2, 0x10+var_s8($sp)`
- 80020234 -> 80020238 type=21 target=sub_80020238 line=`lw      $s1, 0x10+var_s4($sp)`
- 80020238 -> 8002023C type=21 target=sub_8002023C line=`lw      $s0, 0x10+var_s0($sp)`
- 8002023C -> 80020240 type=21 target=sub_80020240 line=`addiu   $sp, 0x20`
- 80020240 -> 80020244 type=21 target=sub_80020244 line=`jr      $ra`

### Full disasm
```asm
800201AC: addiu   $sp, -0x20                                               ; E0 FF BD 27
800201B0: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
800201B4: move    $s0, $a0                                                 ; 21 80 80 00
800201B8: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
800201BC: move    $s1, $a1                                                 ; 21 88 A0 00
800201C0: move    $a0, $a2                                                 ; 21 20 C0 00
800201C4: sw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 AF
800201C8: sw      $ra, 0x10+var_sC($sp)                                    ; 1C 00 BF AF
800201CC: jal     sub_8001FFD4                                             ; F5 7F 00 0C
800201D0: move    $s2, $a3                                                 ; 21 90 E0 00
800201D4: move    $a0, $s0                                                 ; 21 20 00 02
800201D8: jal     sub_8001EA74                                             ; 9D 7A 00 0C
800201DC: move    $a1, $s1                                                 ; 21 28 20 02
800201E0: jal     sub_80035560                                             ; 58 D5 00 0C
800201E4: li      $a0, 2                                                   ; 02 00 04 34
800201E8: jal     sub_8001EBF4                                             ; FD 7A 00 0C
800201EC: move    $a0, $s0                                                 ; 21 20 00 02
800201F0: jal     sub_8001F518                                             ; 46 7D 00 0C
800201F4: nop                                                              ; 00 00 00 00
800201F8: bnez    $v0, loc_800201D8                                        ; F7 FF 40 14
800201FC: move    $a0, $s0                                                 ; 21 20 00 02
80020200: jal     sub_8001FFD4                                             ; F5 7F 00 0C
80020204: move    $a0, $s2                                                 ; 21 20 40 02
80020208: li      $v0, 0xBE                                                ; BE 00 02 34
8002020C: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
80020210: move    $a0, $s0                                                 ; 21 20 00 02
80020214: li      $a2, sub_8001EA74                                        ; 02 80 06 3C 74 EA C6 24
8002021C: li      $a3, sub_8001EBF4                                        ; 02 80 07 3C F4 EB E7 24
80020224: jal     sub_80020090                                             ; 24 80 00 0C
80020228: move    $a1, $s1                                                 ; 21 28 20 02
8002022C: lw      $ra, 0x10+var_sC($sp)                                    ; 1C 00 BF 8F
80020230: lw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 8F
80020234: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
80020238: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
8002023C: addiu   $sp, 0x20                                                ; 20 00 BD 27
80020240: jr      $ra                                                      ; 08 00 E0 03
80020244: nop                                                              ; 00 00 00 00
```

## `8001EA74` `sub_8001EA74`
Bounds: `8001EA74-8001EBF4`

### Pseudocode
```c
int __fastcall sub_8001EA74(int a1, int a2)
{
  int v3; // $v0
  int v5; // $a0
  int v6; // $gp
  int v7; // $gp
  int result; // $v0
  int v9; // $gp
  int v10; // $gp
  int v11; // $gp
  int v12; // $gp

  v3 = sub_8004019C();
  v5 = dword_8006ED50[v3];
  *(_DWORD *)(v6 + 872) = v3;
  *(_DWORD *)(v6 + 792) = 1;
  sub_80040F90(v5);
  result = sub_80040CC8(0, 0, (int *)&unk_80087288 + 5 * *(_DWORD *)(v7 + 872));
  switch ( a2 )
  {
    case 1:
      return sub_8001F524(0, 8);
    case 2:
      *(_DWORD *)(v9 + 792) = 0;
      sub_8001FCBC(8, 4);
      goto LABEL_6;
    case 3:
      sub_80022CBC(4, a1);
      sub_8001D74C(5, *(_DWORD *)(v10 + 872));
      sub_8001FC40(4, 8);
      goto LABEL_6;
    case 4:
      sub_80021E60(0);
      sub_8001D74C(5, *(_DWORD *)(v11 + 872));
      sub_8001FC40(4, 8);
LABEL_6:
      result = sub_8001FDC0(0);
      break;
    case 5:
      result = sub_80020308(word_800916DC);
      *(_DWORD *)(v12 + 792) = result;
      break;
    case 6:
      result = sub_80020248(word_800916DC);
      break;
    default:
      if ( a1 )
        result = sub_8001EF40(word_800916E0, 0);
      break;
  }
  return result;
}

```

### Calls in function
- 8001EA88: `jal     sub_8004019C` -> 8004019C sub_8004019C
- 8001EAB0: `jal     sub_80040F90` -> 80040F90 sub_80040F90
- 8001EAD8: `jal     sub_80040CC8` -> 80040CC8 sub_80040CC8
- 8001EAE4: `beqz    $v0, def_8001EB00 # jumptable 8001EB00 default case` -> 8001EBC4 def_8001EB00
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB08 loc_8001EB08
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB1C loc_8001EB1C
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB48 loc_8001EB48
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB70 loc_8001EB70
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB90 loc_8001EB90
- 8001EB00: `jr      $v0 # switch jump` -> 8001EBA8 loc_8001EBA8
- 8001EB0C: `jal     sub_8001F524` -> 8001F524 sub_8001F524
- 8001EB14: `j       loc_8001EBDC` -> 8001EBDC loc_8001EBDC
- 8001EB20: `jal     sub_80022CBC` -> 80022CBC sub_80022CBC
- 8001EB2C: `jal     sub_8001D74C` -> 8001D74C sub_8001D74C
- 8001EB38: `jal     sub_8001FC40` -> 8001FC40 sub_8001FC40
- 8001EB40: `j       loc_8001EB80` -> 8001EB80 loc_8001EB80
- 8001EB48: `jal     sub_80021E60 # jumptable 8001EB00 case 4` -> 80021E60 sub_80021E60
- 8001EB54: `jal     sub_8001D74C` -> 8001D74C sub_8001D74C
- 8001EB60: `jal     sub_8001FC40` -> 8001FC40 sub_8001FC40
- 8001EB68: `j       loc_8001EB80` -> 8001EB80 loc_8001EB80
- 8001EB78: `jal     sub_8001FCBC` -> 8001FCBC sub_8001FCBC
- 8001EB80: `jal     sub_8001FDC0` -> 8001FDC0 sub_8001FDC0
- 8001EB88: `j       loc_8001EBDC` -> 8001EBDC loc_8001EBDC
- 8001EB98: `jal     sub_80020248` -> 80020248 sub_80020248
- 8001EBA0: `j       loc_8001EBDC` -> 8001EBDC loc_8001EBDC
- 8001EBB0: `jal     sub_80020308` -> 80020308 sub_80020308
- 8001EBBC: `j       loc_8001EBDC` -> 8001EBDC loc_8001EBDC
- 8001EBC4: `beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case` -> 8001EBDC loc_8001EBDC
- 8001EBD4: `jal     sub_8001EF40` -> 8001EF40 sub_8001EF40

### Xrefs to
- 800153A8 -> 8001EA74 type=17 func=sub_8001537C line=`jal     sub_8001EA74`
- 8002013C -> 8001EA74 type=17 func=sub_80020110 line=`jal     sub_8001EA74`
- 800201D8 -> 8001EA74 type=17 func=sub_800201AC line=`jal     sub_8001EA74`
- 80020178 -> 8001EA74 type=1 func=sub_80020110 line=`li      $a2, sub_8001EA74`
- 80020214 -> 8001EA74 type=1 func=sub_800201AC line=`li      $a2, sub_8001EA74`

### Xrefs from
- 8001EA74 -> 8001EA78 type=21 target=sub_8001EA78 line=`addiu   $sp, -0x20`
- 8001EA78 -> 8001EA7C type=21 target=sub_8001EA7C line=`sw      $s1, 0x10+var_s4($sp)`
- 8001EA7C -> 8001EA80 type=21 target=sub_8001EA80 line=`move    $s1, $a0`
- 8001EA80 -> 8001EA84 type=21 target=sub_8001EA84 line=`sw      $s0, 0x10+var_s0($sp)`
- 8001EA84 -> 8001EA88 type=21 target=sub_8001EA88 line=`sw      $ra, 0x10+var_s8($sp)`
- 8001EA88 -> 8001EA8C type=21 target=sub_8001EA8C line=`jal     sub_8004019C`
- 8001EA88 -> 8004019C type=17 target=sub_8004019C line=`jal     sub_8004019C`
- 8001EA8C -> 8001EA90 type=21 target=sub_8001EA90 line=`move    $s0, $a1`
- 8001EA90 -> 8001EA94 type=21 target=sub_8001EA94 line=`sll     $v1, $v0, 2`
- 8001EA94 -> 8001EA9C type=21 target=sub_8001EA9C line=`li      $at, dword_8006ED50`
- 8001EA9C -> 8001EAA0 type=21 target=sub_8001EAA0 line=`addu    $at, $v1`
- 8001EAA0 -> 8001EAA4 type=21 target=sub_8001EAA4 line=`lw      $a0, 0($at)`
- 8001EAA4 -> 8001EAA8 type=21 target=sub_8001EAA8 line=`sw      $v0, 0x368($gp)`
- 8001EAA8 -> 8001EAAC type=21 target=sub_8001EAAC line=`li      $v0, 1`
- 8001EAAC -> 8001EAB0 type=21 target=sub_8001EAB0 line=`sw      $v0, 0x318($gp)`
- 8001EAB0 -> 8001EAB4 type=21 target=sub_8001EAB4 line=`jal     sub_80040F90`
- 8001EAB0 -> 80040F90 type=17 target=sub_80040F90 line=`jal     sub_80040F90`
- 8001EAB4 -> 8001EAB8 type=21 target=sub_8001EAB8 line=`addiu   $s0, -1 # switch 6 cases`
- 8001EAB8 -> 8001EABC type=21 target=sub_8001EABC line=`move    $a0, $zero`
- 8001EABC -> 8001EAC0 type=21 target=sub_8001EAC0 line=`lw      $v0, 0x368($gp)`
- 8001EAC0 -> 8001EAC4 type=21 target=sub_8001EAC4 line=`move    $a1, $zero`
- 8001EAC4 -> 8001EAC8 type=21 target=sub_8001EAC8 line=`sll     $a2, $v0, 2`
- 8001EAC8 -> 8001EACC type=21 target=sub_8001EACC line=`addu    $a2, $v0`
- 8001EACC -> 8001EAD0 type=21 target=sub_8001EAD0 line=`sll     $a2, 2`
- 8001EAD0 -> 8001EAD8 type=21 target=sub_8001EAD8 line=`li      $v0, unk_80087288`
- 8001EAD8 -> 8001EADC type=21 target=sub_8001EADC line=`jal     sub_80040CC8`
- 8001EAD8 -> 80040CC8 type=17 target=sub_80040CC8 line=`jal     sub_80040CC8`
- 8001EADC -> 8001EAE0 type=21 target=sub_8001EAE0 line=`addu    $a2, $v0`
- 8001EAE0 -> 8001EAE4 type=21 target=sub_8001EAE4 line=`sltiu   $v0, $s0, 6`
- 8001EAE4 -> 8001EAE8 type=21 target=sub_8001EAE8 line=`beqz    $v0, def_8001EB00 # jumptable 8001EB00 default case`
- 8001EAE4 -> 8001EBC4 type=19 target=def_8001EB00 line=`beqz    $v0, def_8001EB00 # jumptable 8001EB00 default case`
- 8001EAE8 -> 8001EAEC type=21 target=sub_8001EAEC line=`sll     $v0, $s0, 2`
- 8001EAEC -> 8001EAF4 type=21 target=sub_8001EAF4 line=`li      $at, jpt_8001EB00`
- 8001EAF4 -> 8001EAF8 type=21 target=sub_8001EAF8 line=`addu    $at, $v0`
- 8001EAF8 -> 8001EAFC type=21 target=sub_8001EAFC line=`lw      $v0, 0($at)`
- 8001EAFC -> 8001EB00 type=21 target=sub_8001EB00 line=`nop`
- 8001EB00 -> 8001EB04 type=21 target=sub_8001EB04 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB08 type=19 target=loc_8001EB08 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB1C type=19 target=loc_8001EB1C line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB48 type=19 target=loc_8001EB48 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB70 type=19 target=loc_8001EB70 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB90 type=19 target=loc_8001EB90 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EBA8 type=19 target=loc_8001EBA8 line=`jr      $v0 # switch jump`
- 8001EB08 -> 8001EB0C type=21 target=sub_8001EB0C line=`move    $a0, $zero # jumptable 8001EB00 case 1`
- 8001EB0C -> 8001EB10 type=21 target=sub_8001EB10 line=`jal     sub_8001F524`
- 8001EB0C -> 8001F524 type=17 target=sub_8001F524 line=`jal     sub_8001F524`
- 8001EB10 -> 8001EB14 type=21 target=sub_8001EB14 line=`li      $a1, 8`
- 8001EB14 -> 8001EB18 type=21 target=sub_8001EB18 line=`j       loc_8001EBDC`
- 8001EB14 -> 8001EBDC type=19 target=loc_8001EBDC line=`j       loc_8001EBDC`
- 8001EB1C -> 8001EB20 type=21 target=sub_8001EB20 line=`li      $a0, 4 # jumptable 8001EB00 case 3`
- 8001EB20 -> 8001EB24 type=21 target=sub_8001EB24 line=`jal     sub_80022CBC`
- 8001EB20 -> 80022CBC type=17 target=sub_80022CBC line=`jal     sub_80022CBC`
- 8001EB24 -> 8001EB28 type=21 target=sub_8001EB28 line=`move    $a1, $s1`
- 8001EB28 -> 8001EB2C type=21 target=sub_8001EB2C line=`lw      $a1, 0x368($gp)`
- 8001EB2C -> 8001EB30 type=21 target=sub_8001EB30 line=`jal     sub_8001D74C`
- 8001EB2C -> 8001D74C type=17 target=sub_8001D74C line=`jal     sub_8001D74C`
- 8001EB30 -> 8001EB34 type=21 target=sub_8001EB34 line=`li      $a0, 5`
- 8001EB34 -> 8001EB38 type=21 target=sub_8001EB38 line=`li      $a0, 4`
- 8001EB38 -> 8001EB3C type=21 target=sub_8001EB3C line=`jal     sub_8001FC40`
- 8001EB38 -> 8001FC40 type=17 target=sub_8001FC40 line=`jal     sub_8001FC40`
- 8001EB3C -> 8001EB40 type=21 target=sub_8001EB40 line=`li      $a1, 8`
- 8001EB40 -> 8001EB44 type=21 target=sub_8001EB44 line=`j       loc_8001EB80`
- 8001EB40 -> 8001EB80 type=19 target=loc_8001EB80 line=`j       loc_8001EB80`
- 8001EB48 -> 8001EB4C type=21 target=sub_8001EB4C line=`jal     sub_80021E60 # jumptable 8001EB00 case 4`
- 8001EB48 -> 80021E60 type=17 target=sub_80021E60 line=`jal     sub_80021E60 # jumptable 8001EB00 case 4`
- 8001EB4C -> 8001EB50 type=21 target=sub_8001EB50 line=`move    $a0, $zero`
- 8001EB50 -> 8001EB54 type=21 target=sub_8001EB54 line=`lw      $a1, 0x368($gp)`
- 8001EB54 -> 8001EB58 type=21 target=sub_8001EB58 line=`jal     sub_8001D74C`
- 8001EB54 -> 8001D74C type=17 target=sub_8001D74C line=`jal     sub_8001D74C`
- 8001EB58 -> 8001EB5C type=21 target=sub_8001EB5C line=`li      $a0, 5`
- 8001EB5C -> 8001EB60 type=21 target=sub_8001EB60 line=`li      $a0, 4`
- 8001EB60 -> 8001EB64 type=21 target=sub_8001EB64 line=`jal     sub_8001FC40`
- 8001EB60 -> 8001FC40 type=17 target=sub_8001FC40 line=`jal     sub_8001FC40`
- 8001EB64 -> 8001EB68 type=21 target=sub_8001EB68 line=`li      $a1, 8`
- 8001EB68 -> 8001EB6C type=21 target=sub_8001EB6C line=`j       loc_8001EB80`
- 8001EB68 -> 8001EB80 type=19 target=loc_8001EB80 line=`j       loc_8001EB80`
- 8001EB70 -> 8001EB74 type=21 target=sub_8001EB74 line=`sw      $zero, 0x318($gp) # jumptable 8001EB00 case 2`
- 8001EB74 -> 8001EB78 type=21 target=sub_8001EB78 line=`li      $a0, 8`
- 8001EB78 -> 8001EB7C type=21 target=sub_8001EB7C line=`jal     sub_8001FCBC`
- 8001EB78 -> 8001FCBC type=17 target=sub_8001FCBC line=`jal     sub_8001FCBC`
- 8001EB7C -> 8001EB80 type=21 target=loc_8001EB80 line=`li      $a1, 4`
- 8001EB80 -> 8001EB84 type=21 target=sub_8001EB84 line=`jal     sub_8001FDC0`
- 8001EB80 -> 8001FDC0 type=17 target=sub_8001FDC0 line=`jal     sub_8001FDC0`
- 8001EB84 -> 8001EB88 type=21 target=sub_8001EB88 line=`move    $a0, $zero`
- 8001EB88 -> 8001EB8C type=21 target=sub_8001EB8C line=`j       loc_8001EBDC`
- 8001EB88 -> 8001EBDC type=19 target=loc_8001EBDC line=`j       loc_8001EBDC`
- 8001EB90 -> 8001EB98 type=21 target=sub_8001EB98 line=`lh      $a0, word_800916DC # jumptable 8001EB00 case 6`
- 8001EB98 -> 8001EB9C type=21 target=sub_8001EB9C line=`jal     sub_80020248`
- 8001EB98 -> 80020248 type=17 target=sub_80020248 line=`jal     sub_80020248`
- 8001EB9C -> 8001EBA0 type=21 target=sub_8001EBA0 line=`nop`
- 8001EBA0 -> 8001EBA4 type=21 target=sub_8001EBA4 line=`j       loc_8001EBDC`
- 8001EBA0 -> 8001EBDC type=19 target=loc_8001EBDC line=`j       loc_8001EBDC`
- 8001EBA8 -> 8001EBB0 type=21 target=sub_8001EBB0 line=`lh      $a0, word_800916DC # jumptable 8001EB00 case 5`
- 8001EBB0 -> 8001EBB4 type=21 target=sub_8001EBB4 line=`jal     sub_80020308`
- 8001EBB0 -> 80020308 type=17 target=sub_80020308 line=`jal     sub_80020308`
- 8001EBB4 -> 8001EBB8 type=21 target=sub_8001EBB8 line=`nop`
- 8001EBB8 -> 8001EBBC type=21 target=sub_8001EBBC line=`sw      $v0, 0x318($gp)`
- 8001EBBC -> 8001EBC0 type=21 target=sub_8001EBC0 line=`j       loc_8001EBDC`
- 8001EBBC -> 8001EBDC type=19 target=loc_8001EBDC line=`j       loc_8001EBDC`
- 8001EBC4 -> 8001EBC8 type=21 target=sub_8001EBC8 line=`beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case`
- 8001EBC4 -> 8001EBDC type=19 target=loc_8001EBDC line=`beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case`
- 8001EBC8 -> 8001EBCC type=21 target=sub_8001EBCC line=`nop`
- 8001EBCC -> 8001EBD4 type=21 target=sub_8001EBD4 line=`lh      $a0, word_800916E0`
- 8001EBD4 -> 8001EBD8 type=21 target=sub_8001EBD8 line=`jal     sub_8001EF40`
- 8001EBD4 -> 8001EF40 type=17 target=sub_8001EF40 line=`jal     sub_8001EF40`
- 8001EBD8 -> 8001EBDC type=21 target=loc_8001EBDC line=`move    $a1, $zero`
- 8001EBDC -> 8001EBE0 type=21 target=sub_8001EBE0 line=`lw      $ra, 0x10+var_s8($sp)`
- 8001EBE0 -> 8001EBE4 type=21 target=sub_8001EBE4 line=`lw      $s1, 0x10+var_s4($sp)`
- 8001EBE4 -> 8001EBE8 type=21 target=sub_8001EBE8 line=`lw      $s0, 0x10+var_s0($sp)`
- 8001EBE8 -> 8001EBEC type=21 target=sub_8001EBEC line=`addiu   $sp, 0x20`
- 8001EBEC -> 8001EBF0 type=21 target=sub_8001EBF0 line=`jr      $ra`

### Full disasm
```asm
8001EA74: addiu   $sp, -0x20                                               ; E0 FF BD 27
8001EA78: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
8001EA7C: move    $s1, $a0                                                 ; 21 88 80 00
8001EA80: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
8001EA84: sw      $ra, 0x10+var_s8($sp)                                    ; 18 00 BF AF
8001EA88: jal     sub_8004019C                                             ; 67 00 01 0C
8001EA8C: move    $s0, $a1                                                 ; 21 80 A0 00
8001EA90: sll     $v1, $v0, 2                                              ; 80 18 02 00
8001EA94: li      $at, dword_8006ED50                                      ; 07 80 01 3C 50 ED 21 24
8001EA9C: addu    $at, $v1                                                 ; 21 08 23 00
8001EAA0: lw      $a0, 0($at)                                              ; 00 00 24 8C
8001EAA4: sw      $v0, 0x368($gp)                                          ; 68 03 82 AF
8001EAA8: li      $v0, 1                                                   ; 01 00 02 34
8001EAAC: sw      $v0, 0x318($gp)                                          ; 18 03 82 AF
8001EAB0: jal     sub_80040F90                                             ; E4 03 01 0C
8001EAB4: addiu   $s0, -1 # switch 6 cases                                 ; FF FF 10 26
8001EAB8: move    $a0, $zero                                               ; 21 20 00 00
8001EABC: lw      $v0, 0x368($gp)                                          ; 68 03 82 8F
8001EAC0: move    $a1, $zero                                               ; 21 28 00 00
8001EAC4: sll     $a2, $v0, 2                                              ; 80 30 02 00
8001EAC8: addu    $a2, $v0                                                 ; 21 30 C2 00
8001EACC: sll     $a2, 2                                                   ; 80 30 06 00
8001EAD0: li      $v0, unk_80087288                                        ; 08 80 02 3C 88 72 42 24
8001EAD8: jal     sub_80040CC8                                             ; 32 03 01 0C
8001EADC: addu    $a2, $v0                                                 ; 21 30 C2 00
8001EAE0: sltiu   $v0, $s0, 6                                              ; 06 00 02 2E
8001EAE4: beqz    $v0, def_8001EB00 # jumptable 8001EB00 default case      ; 37 00 40 10
8001EAE8: sll     $v0, $s0, 2                                              ; 80 10 10 00
8001EAEC: li      $at, jpt_8001EB00                                        ; 01 80 01 3C 44 10 21 24
8001EAF4: addu    $at, $v0                                                 ; 21 08 22 00
8001EAF8: lw      $v0, 0($at)                                              ; 00 00 22 8C
8001EAFC: nop                                                              ; 00 00 00 00
8001EB00: jr      $v0 # switch jump                                        ; 08 00 40 00
8001EB04: nop                                                              ; 00 00 00 00
8001EB08: move    $a0, $zero # jumptable 8001EB00 case 1                   ; 21 20 00 00
8001EB0C: jal     sub_8001F524                                             ; 49 7D 00 0C
8001EB10: li      $a1, 8                                                   ; 08 00 05 34
8001EB14: j       loc_8001EBDC                                             ; F7 7A 00 08
8001EB18: nop                                                              ; 00 00 00 00
8001EB1C: li      $a0, 4 # jumptable 8001EB00 case 3                       ; 04 00 04 34
8001EB20: jal     sub_80022CBC                                             ; 2F 8B 00 0C
8001EB24: move    $a1, $s1                                                 ; 21 28 20 02
8001EB28: lw      $a1, 0x368($gp)                                          ; 68 03 85 8F
8001EB2C: jal     sub_8001D74C                                             ; D3 75 00 0C
8001EB30: li      $a0, 5                                                   ; 05 00 04 34
8001EB34: li      $a0, 4                                                   ; 04 00 04 34
8001EB38: jal     sub_8001FC40                                             ; 10 7F 00 0C
8001EB3C: li      $a1, 8                                                   ; 08 00 05 34
8001EB40: j       loc_8001EB80                                             ; E0 7A 00 08
8001EB44: nop                                                              ; 00 00 00 00
8001EB48: jal     sub_80021E60 # jumptable 8001EB00 case 4                 ; 98 87 00 0C
8001EB4C: move    $a0, $zero                                               ; 21 20 00 00
8001EB50: lw      $a1, 0x368($gp)                                          ; 68 03 85 8F
8001EB54: jal     sub_8001D74C                                             ; D3 75 00 0C
8001EB58: li      $a0, 5                                                   ; 05 00 04 34
8001EB5C: li      $a0, 4                                                   ; 04 00 04 34
8001EB60: jal     sub_8001FC40                                             ; 10 7F 00 0C
8001EB64: li      $a1, 8                                                   ; 08 00 05 34
8001EB68: j       loc_8001EB80                                             ; E0 7A 00 08
8001EB6C: nop                                                              ; 00 00 00 00
8001EB70: sw      $zero, 0x318($gp) # jumptable 8001EB00 case 2            ; 18 03 80 AF
8001EB74: li      $a0, 8                                                   ; 08 00 04 34
8001EB78: jal     sub_8001FCBC                                             ; 2F 7F 00 0C
8001EB7C: li      $a1, 4                                                   ; 04 00 05 34
8001EB80: jal     sub_8001FDC0                                             ; 70 7F 00 0C
8001EB84: move    $a0, $zero                                               ; 21 20 00 00
8001EB88: j       loc_8001EBDC                                             ; F7 7A 00 08
8001EB8C: nop                                                              ; 00 00 00 00
8001EB90: lh      $a0, word_800916DC # jumptable 8001EB00 case 6           ; 09 80 04 3C DC 16 84 84
8001EB98: jal     sub_80020248                                             ; 92 80 00 0C
8001EB9C: nop                                                              ; 00 00 00 00
8001EBA0: j       loc_8001EBDC                                             ; F7 7A 00 08
8001EBA4: nop                                                              ; 00 00 00 00
8001EBA8: lh      $a0, word_800916DC # jumptable 8001EB00 case 5           ; 09 80 04 3C DC 16 84 84
8001EBB0: jal     sub_80020308                                             ; C2 80 00 0C
8001EBB4: nop                                                              ; 00 00 00 00
8001EBB8: sw      $v0, 0x318($gp)                                          ; 18 03 82 AF
8001EBBC: j       loc_8001EBDC                                             ; F7 7A 00 08
8001EBC0: nop                                                              ; 00 00 00 00
8001EBC4: beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case      ; 05 00 20 12
8001EBC8: nop                                                              ; 00 00 00 00
8001EBCC: lh      $a0, word_800916E0                                       ; 09 80 04 3C E0 16 84 84
8001EBD4: jal     sub_8001EF40                                             ; D0 7B 00 0C
8001EBD8: move    $a1, $zero                                               ; 21 28 00 00
8001EBDC: lw      $ra, 0x10+var_s8($sp)                                    ; 18 00 BF 8F
8001EBE0: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
8001EBE4: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
8001EBE8: addiu   $sp, 0x20                                                ; 20 00 BD 27
8001EBEC: jr      $ra                                                      ; 08 00 E0 03
8001EBF0: nop                                                              ; 00 00 00 00
```

## `80020248` `sub_80020248`
Bounds: `80020248-80020308`

### Pseudocode
```c
int __fastcall sub_80020248(int a1)
{
  int v1; // $gp
  int v3; // $gp
  int v4; // $v0
  BOOL v5; // $v0
  int v6; // $v0
  bool v7; // dc
  int result; // $v0

  if ( !*(_DWORD *)(v1 + 196) )
    sub_800271E4(1);
  if ( a1 )
  {
    if ( *(_DWORD *)(v1 + 196) == 15 )
      sub_800271E4(0);
    if ( *(int *)(v1 + 196) >= 15 )
      sub_8001C864(5);
    else
      sub_8001F230(5);
    v6 = *(_DWORD *)(v3 + 196) + 1;
    *(_DWORD *)(v3 + 196) = v6;
    v5 = v6 < 31;
  }
  else
  {
    sub_8001CE30(5);
    v4 = *(_DWORD *)(v3 + 196) + 1;
    *(_DWORD *)(v3 + 196) = v4;
    v5 = v4 < 16;
  }
  v7 = v5;
  result = 192;
  if ( !v7 )
    *(_DWORD *)(v3 + 196) = 192;
  return result;
}

```

### Calls in function
- 80020258: `bnez    $v0, loc_80020268` -> 80020268 loc_80020268
- 80020260: `jal     sub_800271E4` -> 800271E4 sub_800271E4
- 80020268: `bnez    $s0, loc_80020290` -> 80020290 loc_80020290
- 80020270: `jal     sub_8001CE30` -> 8001CE30 sub_8001CE30
- 80020288: `j       loc_800202E8` -> 800202E8 loc_800202E8
- 80020298: `bne     $v1, $v0, loc_800202A8` -> 800202A8 loc_800202A8
- 800202A0: `jal     sub_800271E4` -> 800271E4 sub_800271E4
- 800202B4: `beqz    $v0, loc_800202CC` -> 800202CC loc_800202CC
- 800202BC: `jal     sub_8001F230` -> 8001F230 sub_8001F230
- 800202C4: `j       loc_800202D4` -> 800202D4 loc_800202D4
- 800202CC: `jal     sub_8001C864` -> 8001C864 sub_8001C864
- 800202E8: `bnez    $v0, loc_800202F4` -> 800202F4 loc_800202F4

### Xrefs to
- 8001EB98 -> 80020248 type=17 func=sub_8001EA74 line=`jal     sub_80020248`

### Xrefs from
- 80020248 -> 8002024C type=21 target=sub_8002024C line=`lw      $v0, 0xC4($gp)`
- 8002024C -> 80020250 type=21 target=sub_80020250 line=`addiu   $sp, -0x18`
- 80020250 -> 80020254 type=21 target=sub_80020254 line=`sw      $s0, 0x10+var_s0($sp)`
- 80020254 -> 80020258 type=21 target=sub_80020258 line=`move    $s0, $a0`
- 80020258 -> 8002025C type=21 target=sub_8002025C line=`bnez    $v0, loc_80020268`
- 80020258 -> 80020268 type=19 target=loc_80020268 line=`bnez    $v0, loc_80020268`
- 8002025C -> 80020260 type=21 target=sub_80020260 line=`sw      $ra, 0x10+var_s4($sp)`
- 80020260 -> 80020264 type=21 target=sub_80020264 line=`jal     sub_800271E4`
- 80020260 -> 800271E4 type=17 target=sub_800271E4 line=`jal     sub_800271E4`
- 80020264 -> 80020268 type=21 target=loc_80020268 line=`li      $a0, 1`
- 80020268 -> 8002026C type=21 target=sub_8002026C line=`bnez    $s0, loc_80020290`
- 80020268 -> 80020290 type=19 target=loc_80020290 line=`bnez    $s0, loc_80020290`
- 8002026C -> 80020270 type=21 target=sub_80020270 line=`nop`
- 80020270 -> 80020274 type=21 target=sub_80020274 line=`jal     sub_8001CE30`
- 80020270 -> 8001CE30 type=17 target=sub_8001CE30 line=`jal     sub_8001CE30`
- 80020274 -> 80020278 type=21 target=sub_80020278 line=`li      $a0, 5`
- 80020278 -> 8002027C type=21 target=sub_8002027C line=`lw      $v0, 0xC4($gp)`
- 8002027C -> 80020280 type=21 target=sub_80020280 line=`nop`
- 80020280 -> 80020284 type=21 target=sub_80020284 line=`addiu   $v0, 1`
- 80020284 -> 80020288 type=21 target=sub_80020288 line=`sw      $v0, 0xC4($gp)`
- 80020288 -> 8002028C type=21 target=sub_8002028C line=`j       loc_800202E8`
- 80020288 -> 800202E8 type=19 target=loc_800202E8 line=`j       loc_800202E8`
- 80020290 -> 80020294 type=21 target=sub_80020294 line=`lw      $v1, 0xC4($gp)`
- 80020294 -> 80020298 type=21 target=sub_80020298 line=`li      $v0, 0xF`
- 80020298 -> 8002029C type=21 target=sub_8002029C line=`bne     $v1, $v0, loc_800202A8`
- 80020298 -> 800202A8 type=19 target=loc_800202A8 line=`bne     $v1, $v0, loc_800202A8`
- 8002029C -> 800202A0 type=21 target=sub_800202A0 line=`nop`
- 800202A0 -> 800202A4 type=21 target=sub_800202A4 line=`jal     sub_800271E4`
- 800202A0 -> 800271E4 type=17 target=sub_800271E4 line=`jal     sub_800271E4`
- 800202A4 -> 800202A8 type=21 target=loc_800202A8 line=`move    $a0, $zero`
- 800202A8 -> 800202AC type=21 target=sub_800202AC line=`lw      $v0, 0xC4($gp)`
- 800202AC -> 800202B0 type=21 target=sub_800202B0 line=`nop`
- 800202B0 -> 800202B4 type=21 target=sub_800202B4 line=`slti    $v0, 0xF`
- 800202B4 -> 800202B8 type=21 target=sub_800202B8 line=`beqz    $v0, loc_800202CC`
- 800202B4 -> 800202CC type=19 target=loc_800202CC line=`beqz    $v0, loc_800202CC`
- 800202B8 -> 800202BC type=21 target=sub_800202BC line=`nop`
- 800202BC -> 800202C0 type=21 target=sub_800202C0 line=`jal     sub_8001F230`
- 800202BC -> 8001F230 type=17 target=sub_8001F230 line=`jal     sub_8001F230`
- 800202C0 -> 800202C4 type=21 target=sub_800202C4 line=`li      $a0, 5`
- 800202C4 -> 800202C8 type=21 target=sub_800202C8 line=`j       loc_800202D4`
- 800202C4 -> 800202D4 type=19 target=loc_800202D4 line=`j       loc_800202D4`
- 800202CC -> 800202D0 type=21 target=sub_800202D0 line=`jal     sub_8001C864`
- 800202CC -> 8001C864 type=17 target=sub_8001C864 line=`jal     sub_8001C864`
- 800202D0 -> 800202D4 type=21 target=loc_800202D4 line=`li      $a0, 5`
- 800202D4 -> 800202D8 type=21 target=sub_800202D8 line=`lw      $v0, 0xC4($gp)`
- 800202D8 -> 800202DC type=21 target=sub_800202DC line=`nop`
- 800202DC -> 800202E0 type=21 target=sub_800202E0 line=`addiu   $v0, 1`
- 800202E0 -> 800202E4 type=21 target=sub_800202E4 line=`sw      $v0, 0xC4($gp)`
- 800202E4 -> 800202E8 type=21 target=loc_800202E8 line=`slti    $v0, 0x1F`
- 800202E8 -> 800202EC type=21 target=sub_800202EC line=`bnez    $v0, loc_800202F4`
- 800202E8 -> 800202F4 type=19 target=loc_800202F4 line=`bnez    $v0, loc_800202F4`
- 800202EC -> 800202F0 type=21 target=sub_800202F0 line=`li      $v0, 0xC0`
- 800202F0 -> 800202F4 type=21 target=loc_800202F4 line=`sw      $v0, 0xC4($gp)`
- 800202F4 -> 800202F8 type=21 target=sub_800202F8 line=`lw      $ra, 0x10+var_s4($sp)`
- 800202F8 -> 800202FC type=21 target=sub_800202FC line=`lw      $s0, 0x10+var_s0($sp)`
- 800202FC -> 80020300 type=21 target=sub_80020300 line=`addiu   $sp, 0x18`
- 80020300 -> 80020304 type=21 target=sub_80020304 line=`jr      $ra`

### Full disasm
```asm
80020248: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002024C: addiu   $sp, -0x18                                               ; E8 FF BD 27
80020250: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
80020254: move    $s0, $a0                                                 ; 21 80 80 00
80020258: bnez    $v0, loc_80020268                                        ; 03 00 40 14
8002025C: sw      $ra, 0x10+var_s4($sp)                                    ; 14 00 BF AF
80020260: jal     sub_800271E4                                             ; 79 9C 00 0C
80020264: li      $a0, 1                                                   ; 01 00 04 34
80020268: bnez    $s0, loc_80020290                                        ; 09 00 00 16
8002026C: nop                                                              ; 00 00 00 00
80020270: jal     sub_8001CE30                                             ; 8C 73 00 0C
80020274: li      $a0, 5                                                   ; 05 00 04 34
80020278: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002027C: nop                                                              ; 00 00 00 00
80020280: addiu   $v0, 1                                                   ; 01 00 42 24
80020284: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
80020288: j       loc_800202E8                                             ; BA 80 00 08
8002028C: slti    $v0, 0x10                                                ; 10 00 42 28
80020290: lw      $v1, 0xC4($gp)                                           ; C4 00 83 8F
80020294: li      $v0, 0xF                                                 ; 0F 00 02 34
80020298: bne     $v1, $v0, loc_800202A8                                   ; 03 00 62 14
8002029C: nop                                                              ; 00 00 00 00
800202A0: jal     sub_800271E4                                             ; 79 9C 00 0C
800202A4: move    $a0, $zero                                               ; 21 20 00 00
800202A8: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
800202AC: nop                                                              ; 00 00 00 00
800202B0: slti    $v0, 0xF                                                 ; 0F 00 42 28
800202B4: beqz    $v0, loc_800202CC                                        ; 05 00 40 10
800202B8: nop                                                              ; 00 00 00 00
800202BC: jal     sub_8001F230                                             ; 8C 7C 00 0C
800202C0: li      $a0, 5                                                   ; 05 00 04 34
800202C4: j       loc_800202D4                                             ; B5 80 00 08
800202C8: nop                                                              ; 00 00 00 00
800202CC: jal     sub_8001C864                                             ; 19 72 00 0C
800202D0: li      $a0, 5                                                   ; 05 00 04 34
800202D4: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
800202D8: nop                                                              ; 00 00 00 00
800202DC: addiu   $v0, 1                                                   ; 01 00 42 24
800202E0: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
800202E4: slti    $v0, 0x1F                                                ; 1F 00 42 28
800202E8: bnez    $v0, loc_800202F4                                        ; 02 00 40 14
800202EC: li      $v0, 0xC0                                                ; C0 00 02 34
800202F0: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
800202F4: lw      $ra, 0x10+var_s4($sp)                                    ; 14 00 BF 8F
800202F8: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
800202FC: addiu   $sp, 0x18                                                ; 18 00 BD 27
80020300: jr      $ra                                                      ; 08 00 E0 03
80020304: nop                                                              ; 00 00 00 00
```

## `80020308` `sub_80020308`
Bounds: `80020308-800203D4`

### Pseudocode
```c
int __fastcall sub_80020308(int a1)
{
  int v1; // $gp
  int v3; // $gp
  int v4; // $s0
  int v5; // $v0
  BOOL v6; // $v0
  int v7; // $v0
  bool v8; // dc
  int result; // $v0

  if ( !*(_DWORD *)(v1 + 196) )
    sub_800271E4(0);
  if ( a1 )
  {
    if ( *(_DWORD *)(v1 + 196) == 15 )
      sub_800271E4(1);
    v4 = 0;
    if ( *(int *)(v1 + 196) >= 15 )
    {
      v4 = 1;
      sub_8001FEB4(0);
    }
    else
    {
      sub_8001F230(5);
    }
    v7 = *(_DWORD *)(v3 + 196) + 1;
    *(_DWORD *)(v3 + 196) = v7;
    v6 = v7 < 31;
  }
  else
  {
    sub_8001FEB4(0);
    v4 = 0;
    v5 = *(_DWORD *)(v3 + 196) + 1;
    *(_DWORD *)(v3 + 196) = v5;
    v6 = v5 < 16;
  }
  v8 = v6;
  result = v4;
  if ( !v8 )
  {
    *(_DWORD *)(v3 + 196) = 192;
    return v4;
  }
  return result;
}

```

### Calls in function
- 80020318: `bnez    $v0, loc_80020328` -> 80020328 loc_80020328
- 80020320: `jal     sub_800271E4` -> 800271E4 sub_800271E4
- 80020328: `bnez    $s0, loc_80020350` -> 80020350 loc_80020350
- 80020330: `jal     sub_8001FEB4` -> 8001FEB4 sub_8001FEB4
- 80020348: `j       loc_800203AC` -> 800203AC loc_800203AC
- 80020358: `bne     $v1, $v0, loc_80020368` -> 80020368 loc_80020368
- 80020360: `jal     sub_800271E4` -> 800271E4 sub_800271E4
- 80020374: `beqz    $v0, loc_8002038C` -> 8002038C loc_8002038C
- 8002037C: `jal     sub_8001F230` -> 8001F230 sub_8001F230
- 80020384: `j       loc_80020398` -> 80020398 loc_80020398
- 80020390: `jal     sub_8001FEB4` -> 8001FEB4 sub_8001FEB4
- 800203AC: `bnez    $v0, loc_800203C0` -> 800203C0 loc_800203C0

### Xrefs to
- 8001EBB0 -> 80020308 type=17 func=sub_8001EA74 line=`jal     sub_80020308`

### Xrefs from
- 80020308 -> 8002030C type=21 target=sub_8002030C line=`lw      $v0, 0xC4($gp)`
- 8002030C -> 80020310 type=21 target=sub_80020310 line=`addiu   $sp, -0x18`
- 80020310 -> 80020314 type=21 target=sub_80020314 line=`sw      $s0, 0x10+var_s0($sp)`
- 80020314 -> 80020318 type=21 target=sub_80020318 line=`move    $s0, $a0`
- 80020318 -> 8002031C type=21 target=sub_8002031C line=`bnez    $v0, loc_80020328`
- 80020318 -> 80020328 type=19 target=loc_80020328 line=`bnez    $v0, loc_80020328`
- 8002031C -> 80020320 type=21 target=sub_80020320 line=`sw      $ra, 0x10+var_s4($sp)`
- 80020320 -> 80020324 type=21 target=sub_80020324 line=`jal     sub_800271E4`
- 80020320 -> 800271E4 type=17 target=sub_800271E4 line=`jal     sub_800271E4`
- 80020324 -> 80020328 type=21 target=loc_80020328 line=`move    $a0, $zero`
- 80020328 -> 8002032C type=21 target=sub_8002032C line=`bnez    $s0, loc_80020350`
- 80020328 -> 80020350 type=19 target=loc_80020350 line=`bnez    $s0, loc_80020350`
- 8002032C -> 80020330 type=21 target=sub_80020330 line=`nop`
- 80020330 -> 80020334 type=21 target=sub_80020334 line=`jal     sub_8001FEB4`
- 80020330 -> 8001FEB4 type=17 target=sub_8001FEB4 line=`jal     sub_8001FEB4`
- 80020334 -> 80020338 type=21 target=sub_80020338 line=`move    $a0, $zero`
- 80020338 -> 8002033C type=21 target=sub_8002033C line=`lw      $v0, 0xC4($gp)`
- 8002033C -> 80020340 type=21 target=sub_80020340 line=`move    $s0, $zero`
- 80020340 -> 80020344 type=21 target=sub_80020344 line=`addiu   $v0, 1`
- 80020344 -> 80020348 type=21 target=sub_80020348 line=`sw      $v0, 0xC4($gp)`
- 80020348 -> 8002034C type=21 target=sub_8002034C line=`j       loc_800203AC`
- 80020348 -> 800203AC type=19 target=loc_800203AC line=`j       loc_800203AC`
- 80020350 -> 80020354 type=21 target=sub_80020354 line=`lw      $v1, 0xC4($gp)`
- 80020354 -> 80020358 type=21 target=sub_80020358 line=`li      $v0, 0xF`
- 80020358 -> 8002035C type=21 target=sub_8002035C line=`bne     $v1, $v0, loc_80020368`
- 80020358 -> 80020368 type=19 target=loc_80020368 line=`bne     $v1, $v0, loc_80020368`
- 8002035C -> 80020360 type=21 target=sub_80020360 line=`nop`
- 80020360 -> 80020364 type=21 target=sub_80020364 line=`jal     sub_800271E4`
- 80020360 -> 800271E4 type=17 target=sub_800271E4 line=`jal     sub_800271E4`
- 80020364 -> 80020368 type=21 target=loc_80020368 line=`li      $a0, 1`
- 80020368 -> 8002036C type=21 target=sub_8002036C line=`lw      $v0, 0xC4($gp)`
- 8002036C -> 80020370 type=21 target=sub_80020370 line=`nop`
- 80020370 -> 80020374 type=21 target=sub_80020374 line=`slti    $v0, 0xF`
- 80020374 -> 80020378 type=21 target=sub_80020378 line=`beqz    $v0, loc_8002038C`
- 80020374 -> 8002038C type=19 target=loc_8002038C line=`beqz    $v0, loc_8002038C`
- 80020378 -> 8002037C type=21 target=sub_8002037C line=`move    $s0, $zero`
- 8002037C -> 80020380 type=21 target=sub_80020380 line=`jal     sub_8001F230`
- 8002037C -> 8001F230 type=17 target=sub_8001F230 line=`jal     sub_8001F230`
- 80020380 -> 80020384 type=21 target=sub_80020384 line=`li      $a0, 5`
- 80020384 -> 80020388 type=21 target=sub_80020388 line=`j       loc_80020398`
- 80020384 -> 80020398 type=19 target=loc_80020398 line=`j       loc_80020398`
- 8002038C -> 80020390 type=21 target=sub_80020390 line=`li      $s0, 1`
- 80020390 -> 80020394 type=21 target=sub_80020394 line=`jal     sub_8001FEB4`
- 80020390 -> 8001FEB4 type=17 target=sub_8001FEB4 line=`jal     sub_8001FEB4`
- 80020394 -> 80020398 type=21 target=loc_80020398 line=`move    $a0, $zero`
- 80020398 -> 8002039C type=21 target=sub_8002039C line=`lw      $v0, 0xC4($gp)`
- 8002039C -> 800203A0 type=21 target=sub_800203A0 line=`nop`
- 800203A0 -> 800203A4 type=21 target=sub_800203A4 line=`addiu   $v0, 1`
- 800203A4 -> 800203A8 type=21 target=sub_800203A8 line=`sw      $v0, 0xC4($gp)`
- 800203A8 -> 800203AC type=21 target=loc_800203AC line=`slti    $v0, 0x1F`
- 800203AC -> 800203B0 type=21 target=sub_800203B0 line=`bnez    $v0, loc_800203C0`
- 800203AC -> 800203C0 type=19 target=loc_800203C0 line=`bnez    $v0, loc_800203C0`
- 800203B0 -> 800203B4 type=21 target=sub_800203B4 line=`move    $v0, $s0`
- 800203B4 -> 800203B8 type=21 target=sub_800203B8 line=`li      $v0, 0xC0`
- 800203B8 -> 800203BC type=21 target=sub_800203BC line=`sw      $v0, 0xC4($gp)`
- 800203BC -> 800203C0 type=21 target=loc_800203C0 line=`move    $v0, $s0`
- 800203C0 -> 800203C4 type=21 target=sub_800203C4 line=`lw      $ra, 0x10+var_s4($sp)`
- 800203C4 -> 800203C8 type=21 target=sub_800203C8 line=`lw      $s0, 0x10+var_s0($sp)`
- 800203C8 -> 800203CC type=21 target=sub_800203CC line=`addiu   $sp, 0x18`
- 800203CC -> 800203D0 type=21 target=sub_800203D0 line=`jr      $ra`

### Full disasm
```asm
80020308: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002030C: addiu   $sp, -0x18                                               ; E8 FF BD 27
80020310: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
80020314: move    $s0, $a0                                                 ; 21 80 80 00
80020318: bnez    $v0, loc_80020328                                        ; 03 00 40 14
8002031C: sw      $ra, 0x10+var_s4($sp)                                    ; 14 00 BF AF
80020320: jal     sub_800271E4                                             ; 79 9C 00 0C
80020324: move    $a0, $zero                                               ; 21 20 00 00
80020328: bnez    $s0, loc_80020350                                        ; 09 00 00 16
8002032C: nop                                                              ; 00 00 00 00
80020330: jal     sub_8001FEB4                                             ; AD 7F 00 0C
80020334: move    $a0, $zero                                               ; 21 20 00 00
80020338: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002033C: move    $s0, $zero                                               ; 21 80 00 00
80020340: addiu   $v0, 1                                                   ; 01 00 42 24
80020344: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
80020348: j       loc_800203AC                                             ; EB 80 00 08
8002034C: slti    $v0, 0x10                                                ; 10 00 42 28
80020350: lw      $v1, 0xC4($gp)                                           ; C4 00 83 8F
80020354: li      $v0, 0xF                                                 ; 0F 00 02 34
80020358: bne     $v1, $v0, loc_80020368                                   ; 03 00 62 14
8002035C: nop                                                              ; 00 00 00 00
80020360: jal     sub_800271E4                                             ; 79 9C 00 0C
80020364: li      $a0, 1                                                   ; 01 00 04 34
80020368: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002036C: nop                                                              ; 00 00 00 00
80020370: slti    $v0, 0xF                                                 ; 0F 00 42 28
80020374: beqz    $v0, loc_8002038C                                        ; 05 00 40 10
80020378: move    $s0, $zero                                               ; 21 80 00 00
8002037C: jal     sub_8001F230                                             ; 8C 7C 00 0C
80020380: li      $a0, 5                                                   ; 05 00 04 34
80020384: j       loc_80020398                                             ; E6 80 00 08
80020388: nop                                                              ; 00 00 00 00
8002038C: li      $s0, 1                                                   ; 01 00 10 34
80020390: jal     sub_8001FEB4                                             ; AD 7F 00 0C
80020394: move    $a0, $zero                                               ; 21 20 00 00
80020398: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002039C: nop                                                              ; 00 00 00 00
800203A0: addiu   $v0, 1                                                   ; 01 00 42 24
800203A4: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
800203A8: slti    $v0, 0x1F                                                ; 1F 00 42 28
800203AC: bnez    $v0, loc_800203C0                                        ; 04 00 40 14
800203B0: move    $v0, $s0                                                 ; 21 10 00 02
800203B4: li      $v0, 0xC0                                                ; C0 00 02 34
800203B8: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
800203BC: move    $v0, $s0                                                 ; 21 10 00 02
800203C0: lw      $ra, 0x10+var_s4($sp)                                    ; 14 00 BF 8F
800203C4: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
800203C8: addiu   $sp, 0x18                                                ; 18 00 BD 27
800203CC: jr      $ra                                                      ; 08 00 E0 03
800203D0: nop                                                              ; 00 00 00 00
```

## `8001F230` `sub_8001F230`
Bounds: `8001F230-8001F518`

### Pseudocode
```c
int __fastcall sub_8001F230(unsigned __int16 a1)
{
  int v2; // $s5
  int v3; // $s4
  int v4; // $s3
  int v5; // $s1
  int v6; // $s5
  int v7; // $s7
  int v8; // $s6
  int v9; // $s4
  int v10; // $s3

  sub_8001C550(20, 20, (int)dword_80050380, a1);
  sub_8001C550(280, 20, (int)dword_80050390, a1);
  sub_8001C550(40, 20, (int)dword_800503A0, a1);
  sub_8001C550(160, 20, (int)dword_800503B0, a1);
  sub_8001C550(40, 160, (int)dword_800503C0, a1);
  sub_8001C550(160, 160, (int)dword_800503D0, a1);
  v2 = 0;
  v3 = 1310720;
  v4 = 2621440;
  LOWORD(v5) = 40;
  do
  {
    sub_8001C550(v5, 0, (int)dword_800503E0, a1);
    sub_8001C550(SHIWORD(v3), 0, (int)dword_800503F0, a1);
    sub_8001C550(v5, 220, (int)dword_80050400, a1);
    sub_8001C550(SHIWORD(v3), 220, (int)dword_80050410, a1);
    v3 += 2621440;
    v4 += 2621440;
    ++v2;
    v5 = v4 >> 16;
  }
  while ( v2 < 7 );
  v6 = 0;
  v7 = 3932160;
  v8 = 2621440;
  v9 = 0;
  v10 = 1310720;
  do
  {
    sub_8001C550(0, SHIWORD(v10), (int)dword_80050400, a1);
    sub_8001C550(0, SHIWORD(v9), (int)dword_800503E0, a1);
    sub_8001C550(300, SHIWORD(v10), (int)dword_80050410, a1);
    sub_8001C550(300, SHIWORD(v9), (int)dword_800503F0, a1);
    sub_8001C550(SHIWORD(v8), 200, (int)dword_800503E0, a1);
    sub_8001C550(SHIWORD(v7), 200, (int)dword_800503F0, a1);
    sub_8001C550(SHIWORD(v8), 180, (int)dword_80050400, a1);
    sub_8001C550(SHIWORD(v7), 180, (int)dword_80050410, a1);
    v7 += 2621440;
    v8 += 2621440;
    v9 += 2621440;
    v10 += 2621440;
    ++v6;
  }
  while ( v6 < 6 );
  sub_8001C550(280, 200, (int)dword_800503E0, a1);
  sub_8001C550(20, 200, (int)dword_800503F0, a1);
  sub_8001C550(280, 180, (int)dword_80050400, a1);
  return sub_8001C550(20, 180, (int)dword_80050410, a1);
}

```

### Calls in function
- 8001F270: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F288: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F2A0: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F2B8: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F2D0: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F2E8: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F310: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F32C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F344: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F35C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F378: `bnez    $v0, loc_8001F300` -> 8001F300 loc_8001F300
- 8001F3B0: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F3C8: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F3E0: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F3F8: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F410: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F42C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F444: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F45C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F480: `bnez    $v0, loc_8001F3A0` -> 8001F3A0 loc_8001F3A0
- 8001F494: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F4AC: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F4C4: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F4DC: `jal     sub_8001C550` -> 8001C550 sub_8001C550

### Xrefs to
- 800202BC -> 8001F230 type=17 func=sub_80020248 line=`jal     sub_8001F230`
- 8002037C -> 8001F230 type=17 func=sub_80020308 line=`jal     sub_8001F230`

### Xrefs from
- 8001F230 -> 8001F234 type=21 target=sub_8001F234 line=`addiu   $sp, -0x38`
- 8001F234 -> 8001F238 type=21 target=sub_8001F238 line=`sw      $s2, 0x10+var_s8($sp)`
- 8001F238 -> 8001F23C type=21 target=sub_8001F23C line=`move    $s2, $a0`
- 8001F23C -> 8001F240 type=21 target=sub_8001F240 line=`li      $a0, 0x14`
- 8001F240 -> 8001F244 type=21 target=sub_8001F244 line=`li      $a1, 0x14`
- 8001F244 -> 8001F24C type=21 target=sub_8001F24C line=`li      $a2, dword_80050380`
- 8001F24C -> 8001F250 type=21 target=sub_8001F250 line=`move    $a3, $s2`
- 8001F250 -> 8001F254 type=21 target=sub_8001F254 line=`sw      $ra, 0x10+var_s24($sp)`
- 8001F254 -> 8001F258 type=21 target=sub_8001F258 line=`sw      $fp, 0x10+var_s20($sp)`
- 8001F258 -> 8001F25C type=21 target=sub_8001F25C line=`sw      $s7, 0x10+var_s1C($sp)`
- 8001F25C -> 8001F260 type=21 target=sub_8001F260 line=`sw      $s6, 0x10+var_s18($sp)`
- 8001F260 -> 8001F264 type=21 target=sub_8001F264 line=`sw      $s5, 0x10+var_s14($sp)`
- 8001F264 -> 8001F268 type=21 target=sub_8001F268 line=`sw      $s4, 0x10+var_s10($sp)`
- 8001F268 -> 8001F26C type=21 target=sub_8001F26C line=`sw      $s3, 0x10+var_sC($sp)`
- 8001F26C -> 8001F270 type=21 target=sub_8001F270 line=`sw      $s1, 0x10+var_s4($sp)`
- 8001F270 -> 8001F274 type=21 target=sub_8001F274 line=`jal     sub_8001C550`
- 8001F270 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F274 -> 8001F278 type=21 target=sub_8001F278 line=`sw      $s0, 0x10+var_s0($sp)`
- 8001F278 -> 8001F27C type=21 target=sub_8001F27C line=`li      $a0, 0x118`
- 8001F27C -> 8001F280 type=21 target=sub_8001F280 line=`li      $a1, 0x14`
- 8001F280 -> 8001F288 type=21 target=sub_8001F288 line=`li      $a2, dword_80050390`
- 8001F288 -> 8001F28C type=21 target=sub_8001F28C line=`jal     sub_8001C550`
- 8001F288 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F28C -> 8001F290 type=21 target=sub_8001F290 line=`move    $a3, $s2`
- 8001F290 -> 8001F294 type=21 target=sub_8001F294 line=`li      $a0, 0x28  # '('`
- 8001F294 -> 8001F298 type=21 target=sub_8001F298 line=`li      $a1, 0x14`
- 8001F298 -> 8001F2A0 type=21 target=sub_8001F2A0 line=`li      $a2, dword_800503A0`
- 8001F2A0 -> 8001F2A4 type=21 target=sub_8001F2A4 line=`jal     sub_8001C550`
- 8001F2A0 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F2A4 -> 8001F2A8 type=21 target=sub_8001F2A8 line=`move    $a3, $s2`
- 8001F2A8 -> 8001F2AC type=21 target=sub_8001F2AC line=`li      $a0, 0xA0`
- 8001F2AC -> 8001F2B0 type=21 target=sub_8001F2B0 line=`li      $a1, 0x14`
- 8001F2B0 -> 8001F2B8 type=21 target=sub_8001F2B8 line=`li      $a2, dword_800503B0`
- 8001F2B8 -> 8001F2BC type=21 target=sub_8001F2BC line=`jal     sub_8001C550`
- 8001F2B8 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F2BC -> 8001F2C0 type=21 target=sub_8001F2C0 line=`move    $a3, $s2`
- 8001F2C0 -> 8001F2C4 type=21 target=sub_8001F2C4 line=`li      $a0, 0x28  # '('`
- 8001F2C4 -> 8001F2C8 type=21 target=sub_8001F2C8 line=`li      $a1, 0xA0`
- 8001F2C8 -> 8001F2D0 type=21 target=sub_8001F2D0 line=`li      $a2, dword_800503C0`
- 8001F2D0 -> 8001F2D4 type=21 target=sub_8001F2D4 line=`jal     sub_8001C550`
- 8001F2D0 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F2D4 -> 8001F2D8 type=21 target=sub_8001F2D8 line=`move    $a3, $s2`
- 8001F2D8 -> 8001F2DC type=21 target=sub_8001F2DC line=`li      $a0, 0xA0`
- 8001F2DC -> 8001F2E0 type=21 target=sub_8001F2E0 line=`li      $a1, 0xA0`
- 8001F2E0 -> 8001F2E8 type=21 target=sub_8001F2E8 line=`li      $a2, dword_800503D0`
- 8001F2E8 -> 8001F2EC type=21 target=sub_8001F2EC line=`jal     sub_8001C550`
- 8001F2E8 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F2EC -> 8001F2F0 type=21 target=sub_8001F2F0 line=`move    $a3, $s2`
- 8001F2F0 -> 8001F2F4 type=21 target=sub_8001F2F4 line=`move    $s5, $zero`
- 8001F2F4 -> 8001F2F8 type=21 target=sub_8001F2F8 line=`lui     $s4, 0x14`
- 8001F2F8 -> 8001F2FC type=21 target=sub_8001F2FC line=`lui     $s3, 0x28  # '('`
- 8001F2FC -> 8001F300 type=21 target=loc_8001F300 line=`sra     $s1, $s3, 16`
- 8001F300 -> 8001F304 type=21 target=sub_8001F304 line=`move    $a0, $s1`
- 8001F304 -> 8001F308 type=21 target=sub_8001F308 line=`move    $a1, $zero`
- 8001F308 -> 8001F310 type=21 target=sub_8001F310 line=`li      $a2, dword_800503E0`
- 8001F310 -> 8001F314 type=21 target=sub_8001F314 line=`jal     sub_8001C550`
- 8001F310 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F314 -> 8001F318 type=21 target=sub_8001F318 line=`move    $a3, $s2`
- 8001F318 -> 8001F31C type=21 target=sub_8001F31C line=`sra     $s0, $s4, 16`
- 8001F31C -> 8001F320 type=21 target=sub_8001F320 line=`move    $a0, $s0`
- 8001F320 -> 8001F324 type=21 target=sub_8001F324 line=`move    $a1, $zero`
- 8001F324 -> 8001F32C type=21 target=sub_8001F32C line=`li      $a2, dword_800503F0`
- 8001F32C -> 8001F330 type=21 target=sub_8001F330 line=`jal     sub_8001C550`
- 8001F32C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F330 -> 8001F334 type=21 target=sub_8001F334 line=`move    $a3, $s2`
- 8001F334 -> 8001F338 type=21 target=sub_8001F338 line=`move    $a0, $s1`
- 8001F338 -> 8001F33C type=21 target=sub_8001F33C line=`li      $a1, 0xDC`
- 8001F33C -> 8001F344 type=21 target=sub_8001F344 line=`li      $a2, dword_80050400`
- 8001F344 -> 8001F348 type=21 target=sub_8001F348 line=`jal     sub_8001C550`
- 8001F344 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F348 -> 8001F34C type=21 target=sub_8001F34C line=`move    $a3, $s2`
- 8001F34C -> 8001F350 type=21 target=sub_8001F350 line=`move    $a0, $s0`
- 8001F350 -> 8001F354 type=21 target=sub_8001F354 line=`li      $a1, 0xDC`
- 8001F354 -> 8001F35C type=21 target=sub_8001F35C line=`li      $a2, dword_80050410`
- 8001F35C -> 8001F360 type=21 target=sub_8001F360 line=`jal     sub_8001C550`
- 8001F35C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F360 -> 8001F364 type=21 target=sub_8001F364 line=`move    $a3, $s2`
- 8001F364 -> 8001F368 type=21 target=sub_8001F368 line=`lui     $v0, 0x28  # '('`
- 8001F368 -> 8001F36C type=21 target=sub_8001F36C line=`addu    $s4, $v0`
- 8001F36C -> 8001F370 type=21 target=sub_8001F370 line=`addu    $s3, $v0`
- 8001F370 -> 8001F374 type=21 target=sub_8001F374 line=`addiu   $s5, 1`
- 8001F374 -> 8001F378 type=21 target=sub_8001F378 line=`slti    $v0, $s5, 7`
- 8001F378 -> 8001F37C type=21 target=sub_8001F37C line=`bnez    $v0, loc_8001F300`
- 8001F378 -> 8001F300 type=19 target=loc_8001F300 line=`bnez    $v0, loc_8001F300`
- 8001F37C -> 8001F380 type=21 target=sub_8001F380 line=`sra     $s1, $s3, 16`
- 8001F380 -> 8001F384 type=21 target=sub_8001F384 line=`move    $s5, $zero`
- 8001F384 -> 8001F38C type=21 target=sub_8001F38C line=`li      $fp, dword_800503E0`
- 8001F38C -> 8001F390 type=21 target=sub_8001F390 line=`lui     $s7, 0x3C  # '<'`
- 8001F390 -> 8001F394 type=21 target=sub_8001F394 line=`lui     $s6, 0x28  # '('`
- 8001F394 -> 8001F398 type=21 target=sub_8001F398 line=`move    $s4, $zero`
- 8001F398 -> 8001F39C type=21 target=sub_8001F39C line=`lui     $s3, 0x14`
- 8001F39C -> 8001F3A0 type=21 target=loc_8001F3A0 line=`move    $a0, $zero`
- 8001F3A0 -> 8001F3A4 type=21 target=sub_8001F3A4 line=`sra     $s1, $s3, 16`
- 8001F3A4 -> 8001F3A8 type=21 target=sub_8001F3A8 line=`move    $a1, $s1`
- 8001F3A8 -> 8001F3B0 type=21 target=sub_8001F3B0 line=`li      $a2, dword_80050400`
- 8001F3B0 -> 8001F3B4 type=21 target=sub_8001F3B4 line=`jal     sub_8001C550`
- 8001F3B0 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F3B4 -> 8001F3B8 type=21 target=sub_8001F3B8 line=`move    $a3, $s2`
- 8001F3B8 -> 8001F3BC type=21 target=sub_8001F3BC line=`move    $a0, $zero`
- 8001F3BC -> 8001F3C0 type=21 target=sub_8001F3C0 line=`sra     $s0, $s4, 16`
- 8001F3C0 -> 8001F3C4 type=21 target=sub_8001F3C4 line=`move    $a1, $s0`
- 8001F3C4 -> 8001F3C8 type=21 target=sub_8001F3C8 line=`move    $a2, $fp`
- 8001F3C8 -> 8001F3CC type=21 target=sub_8001F3CC line=`jal     sub_8001C550`
- 8001F3C8 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F3CC -> 8001F3D0 type=21 target=sub_8001F3D0 line=`move    $a3, $s2`
- 8001F3D0 -> 8001F3D4 type=21 target=sub_8001F3D4 line=`li      $a0, 0x12C`
- 8001F3D4 -> 8001F3D8 type=21 target=sub_8001F3D8 line=`move    $a1, $s1`
- 8001F3D8 -> 8001F3E0 type=21 target=sub_8001F3E0 line=`li      $a2, dword_80050410`
- 8001F3E0 -> 8001F3E4 type=21 target=sub_8001F3E4 line=`jal     sub_8001C550`
- 8001F3E0 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F3E4 -> 8001F3E8 type=21 target=sub_8001F3E8 line=`move    $a3, $s2`
- 8001F3E8 -> 8001F3EC type=21 target=sub_8001F3EC line=`li      $a0, 0x12C`
- 8001F3EC -> 8001F3F0 type=21 target=sub_8001F3F0 line=`move    $a1, $s0`
- 8001F3F0 -> 8001F3F8 type=21 target=sub_8001F3F8 line=`li      $a2, dword_800503F0`
- 8001F3F8 -> 8001F3FC type=21 target=sub_8001F3FC line=`jal     sub_8001C550`
- 8001F3F8 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F3FC -> 8001F400 type=21 target=sub_8001F400 line=`move    $a3, $s2`
- 8001F400 -> 8001F404 type=21 target=sub_8001F404 line=`sra     $s1, $s6, 16`
- 8001F404 -> 8001F408 type=21 target=sub_8001F408 line=`move    $a0, $s1`
- 8001F408 -> 8001F40C type=21 target=sub_8001F40C line=`li      $a1, 0xC8`
- 8001F40C -> 8001F410 type=21 target=sub_8001F410 line=`move    $a2, $fp`
- 8001F410 -> 8001F414 type=21 target=sub_8001F414 line=`jal     sub_8001C550`
- 8001F410 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F414 -> 8001F418 type=21 target=sub_8001F418 line=`move    $a3, $s2`
- 8001F418 -> 8001F41C type=21 target=sub_8001F41C line=`sra     $s0, $s7, 16`
- 8001F41C -> 8001F420 type=21 target=sub_8001F420 line=`move    $a0, $s0`
- 8001F420 -> 8001F424 type=21 target=sub_8001F424 line=`li      $a1, 0xC8`
- 8001F424 -> 8001F42C type=21 target=sub_8001F42C line=`li      $a2, dword_800503F0`
- 8001F42C -> 8001F430 type=21 target=sub_8001F430 line=`jal     sub_8001C550`
- 8001F42C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F430 -> 8001F434 type=21 target=sub_8001F434 line=`move    $a3, $s2`
- 8001F434 -> 8001F438 type=21 target=sub_8001F438 line=`move    $a0, $s1`
- 8001F438 -> 8001F43C type=21 target=sub_8001F43C line=`li      $a1, 0xB4`
- 8001F43C -> 8001F444 type=21 target=sub_8001F444 line=`li      $a2, dword_80050400`
- 8001F444 -> 8001F448 type=21 target=sub_8001F448 line=`jal     sub_8001C550`
- 8001F444 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F448 -> 8001F44C type=21 target=sub_8001F44C line=`move    $a3, $s2`
- 8001F44C -> 8001F450 type=21 target=sub_8001F450 line=`move    $a0, $s0`
- 8001F450 -> 8001F454 type=21 target=sub_8001F454 line=`li      $a1, 0xB4`
- 8001F454 -> 8001F45C type=21 target=sub_8001F45C line=`li      $a2, dword_80050410`
- 8001F45C -> 8001F460 type=21 target=sub_8001F460 line=`jal     sub_8001C550`
- 8001F45C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F460 -> 8001F464 type=21 target=sub_8001F464 line=`move    $a3, $s2`
- 8001F464 -> 8001F468 type=21 target=sub_8001F468 line=`lui     $v0, 0x28  # '('`
- 8001F468 -> 8001F46C type=21 target=sub_8001F46C line=`addu    $s7, $v0`
- 8001F46C -> 8001F470 type=21 target=sub_8001F470 line=`addu    $s6, $v0`
- 8001F470 -> 8001F474 type=21 target=sub_8001F474 line=`addu    $s4, $v0`
- 8001F474 -> 8001F478 type=21 target=sub_8001F478 line=`addu    $s3, $v0`
- 8001F478 -> 8001F47C type=21 target=sub_8001F47C line=`addiu   $s5, 1`
- 8001F47C -> 8001F480 type=21 target=sub_8001F480 line=`slti    $v0, $s5, 6`
- 8001F480 -> 8001F484 type=21 target=sub_8001F484 line=`bnez    $v0, loc_8001F3A0`
- 8001F480 -> 8001F3A0 type=19 target=loc_8001F3A0 line=`bnez    $v0, loc_8001F3A0`
- 8001F484 -> 8001F488 type=21 target=sub_8001F488 line=`move    $a0, $zero`
- 8001F488 -> 8001F48C type=21 target=sub_8001F48C line=`li      $a0, 0x118`
- 8001F48C -> 8001F490 type=21 target=sub_8001F490 line=`li      $a1, 0xC8`
- 8001F490 -> 8001F494 type=21 target=sub_8001F494 line=`move    $a2, $fp`
- 8001F494 -> 8001F498 type=21 target=sub_8001F498 line=`jal     sub_8001C550`
- 8001F494 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F498 -> 8001F49C type=21 target=sub_8001F49C line=`move    $a3, $s2`
- 8001F49C -> 8001F4A0 type=21 target=sub_8001F4A0 line=`li      $a0, 0x14`
- 8001F4A0 -> 8001F4A4 type=21 target=sub_8001F4A4 line=`li      $a1, 0xC8`
- 8001F4A4 -> 8001F4AC type=21 target=sub_8001F4AC line=`li      $a2, dword_800503F0`
- 8001F4AC -> 8001F4B0 type=21 target=sub_8001F4B0 line=`jal     sub_8001C550`
- 8001F4AC -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F4B0 -> 8001F4B4 type=21 target=sub_8001F4B4 line=`move    $a3, $s2`
- 8001F4B4 -> 8001F4B8 type=21 target=sub_8001F4B8 line=`li      $a0, 0x118`
- 8001F4B8 -> 8001F4BC type=21 target=sub_8001F4BC line=`li      $a1, 0xB4`
- 8001F4BC -> 8001F4C4 type=21 target=sub_8001F4C4 line=`li      $a2, dword_80050400`
- 8001F4C4 -> 8001F4C8 type=21 target=sub_8001F4C8 line=`jal     sub_8001C550`
- 8001F4C4 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F4C8 -> 8001F4CC type=21 target=sub_8001F4CC line=`move    $a3, $s2`
- 8001F4CC -> 8001F4D0 type=21 target=sub_8001F4D0 line=`li      $a0, 0x14`
- 8001F4D0 -> 8001F4D4 type=21 target=sub_8001F4D4 line=`li      $a1, 0xB4`
- 8001F4D4 -> 8001F4DC type=21 target=sub_8001F4DC line=`li      $a2, dword_80050410`
- 8001F4DC -> 8001F4E0 type=21 target=sub_8001F4E0 line=`jal     sub_8001C550`
- 8001F4DC -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F4E0 -> 8001F4E4 type=21 target=sub_8001F4E4 line=`move    $a3, $s2`
- 8001F4E4 -> 8001F4E8 type=21 target=sub_8001F4E8 line=`lw      $ra, 0x10+var_s24($sp)`
- 8001F4E8 -> 8001F4EC type=21 target=sub_8001F4EC line=`lw      $fp, 0x10+var_s20($sp)`
- 8001F4EC -> 8001F4F0 type=21 target=sub_8001F4F0 line=`lw      $s7, 0x10+var_s1C($sp)`
- 8001F4F0 -> 8001F4F4 type=21 target=sub_8001F4F4 line=`lw      $s6, 0x10+var_s18($sp)`
- 8001F4F4 -> 8001F4F8 type=21 target=sub_8001F4F8 line=`lw      $s5, 0x10+var_s14($sp)`
- 8001F4F8 -> 8001F4FC type=21 target=sub_8001F4FC line=`lw      $s4, 0x10+var_s10($sp)`
- 8001F4FC -> 8001F500 type=21 target=sub_8001F500 line=`lw      $s3, 0x10+var_sC($sp)`
- 8001F500 -> 8001F504 type=21 target=sub_8001F504 line=`lw      $s2, 0x10+var_s8($sp)`
- 8001F504 -> 8001F508 type=21 target=sub_8001F508 line=`lw      $s1, 0x10+var_s4($sp)`
- 8001F508 -> 8001F50C type=21 target=sub_8001F50C line=`lw      $s0, 0x10+var_s0($sp)`
- 8001F50C -> 8001F510 type=21 target=sub_8001F510 line=`addiu   $sp, 0x38`
- 8001F510 -> 8001F514 type=21 target=sub_8001F514 line=`jr      $ra`

### Full disasm
```asm
8001F230: addiu   $sp, -0x38                                               ; C8 FF BD 27
8001F234: sw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 AF
8001F238: move    $s2, $a0                                                 ; 21 90 80 00
8001F23C: li      $a0, 0x14                                                ; 14 00 04 34
8001F240: li      $a1, 0x14                                                ; 14 00 05 34
8001F244: li      $a2, dword_80050380                                      ; 05 80 06 3C 80 03 C6 24
8001F24C: move    $a3, $s2                                                 ; 21 38 40 02
8001F250: sw      $ra, 0x10+var_s24($sp)                                   ; 34 00 BF AF
8001F254: sw      $fp, 0x10+var_s20($sp)                                   ; 30 00 BE AF
8001F258: sw      $s7, 0x10+var_s1C($sp)                                   ; 2C 00 B7 AF
8001F25C: sw      $s6, 0x10+var_s18($sp)                                   ; 28 00 B6 AF
8001F260: sw      $s5, 0x10+var_s14($sp)                                   ; 24 00 B5 AF
8001F264: sw      $s4, 0x10+var_s10($sp)                                   ; 20 00 B4 AF
8001F268: sw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 AF
8001F26C: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
8001F270: jal     sub_8001C550                                             ; 54 71 00 0C
8001F274: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
8001F278: li      $a0, 0x118                                               ; 18 01 04 34
8001F27C: li      $a1, 0x14                                                ; 14 00 05 34
8001F280: li      $a2, dword_80050390                                      ; 05 80 06 3C 90 03 C6 24
8001F288: jal     sub_8001C550                                             ; 54 71 00 0C
8001F28C: move    $a3, $s2                                                 ; 21 38 40 02
8001F290: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001F294: li      $a1, 0x14                                                ; 14 00 05 34
8001F298: li      $a2, dword_800503A0                                      ; 05 80 06 3C A0 03 C6 24
8001F2A0: jal     sub_8001C550                                             ; 54 71 00 0C
8001F2A4: move    $a3, $s2                                                 ; 21 38 40 02
8001F2A8: li      $a0, 0xA0                                                ; A0 00 04 34
8001F2AC: li      $a1, 0x14                                                ; 14 00 05 34
8001F2B0: li      $a2, dword_800503B0                                      ; 05 80 06 3C B0 03 C6 24
8001F2B8: jal     sub_8001C550                                             ; 54 71 00 0C
8001F2BC: move    $a3, $s2                                                 ; 21 38 40 02
8001F2C0: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001F2C4: li      $a1, 0xA0                                                ; A0 00 05 34
8001F2C8: li      $a2, dword_800503C0                                      ; 05 80 06 3C C0 03 C6 24
8001F2D0: jal     sub_8001C550                                             ; 54 71 00 0C
8001F2D4: move    $a3, $s2                                                 ; 21 38 40 02
8001F2D8: li      $a0, 0xA0                                                ; A0 00 04 34
8001F2DC: li      $a1, 0xA0                                                ; A0 00 05 34
8001F2E0: li      $a2, dword_800503D0                                      ; 05 80 06 3C D0 03 C6 24
8001F2E8: jal     sub_8001C550                                             ; 54 71 00 0C
8001F2EC: move    $a3, $s2                                                 ; 21 38 40 02
8001F2F0: move    $s5, $zero                                               ; 21 A8 00 00
8001F2F4: lui     $s4, 0x14                                                ; 14 00 14 3C
8001F2F8: lui     $s3, 0x28  # '('                                         ; 28 00 13 3C
8001F2FC: sra     $s1, $s3, 16                                             ; 03 8C 13 00
8001F300: move    $a0, $s1                                                 ; 21 20 20 02
8001F304: move    $a1, $zero                                               ; 21 28 00 00
8001F308: li      $a2, dword_800503E0                                      ; 05 80 06 3C E0 03 C6 24
8001F310: jal     sub_8001C550                                             ; 54 71 00 0C
8001F314: move    $a3, $s2                                                 ; 21 38 40 02
8001F318: sra     $s0, $s4, 16                                             ; 03 84 14 00
8001F31C: move    $a0, $s0                                                 ; 21 20 00 02
8001F320: move    $a1, $zero                                               ; 21 28 00 00
8001F324: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001F32C: jal     sub_8001C550                                             ; 54 71 00 0C
8001F330: move    $a3, $s2                                                 ; 21 38 40 02
8001F334: move    $a0, $s1                                                 ; 21 20 20 02
8001F338: li      $a1, 0xDC                                                ; DC 00 05 34
8001F33C: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001F344: jal     sub_8001C550                                             ; 54 71 00 0C
8001F348: move    $a3, $s2                                                 ; 21 38 40 02
8001F34C: move    $a0, $s0                                                 ; 21 20 00 02
8001F350: li      $a1, 0xDC                                                ; DC 00 05 34
8001F354: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001F35C: jal     sub_8001C550                                             ; 54 71 00 0C
8001F360: move    $a3, $s2                                                 ; 21 38 40 02
8001F364: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001F368: addu    $s4, $v0                                                 ; 21 A0 82 02
8001F36C: addu    $s3, $v0                                                 ; 21 98 62 02
8001F370: addiu   $s5, 1                                                   ; 01 00 B5 26
8001F374: slti    $v0, $s5, 7                                              ; 07 00 A2 2A
8001F378: bnez    $v0, loc_8001F300                                        ; E1 FF 40 14
8001F37C: sra     $s1, $s3, 16                                             ; 03 8C 13 00
8001F380: move    $s5, $zero                                               ; 21 A8 00 00
8001F384: li      $fp, dword_800503E0                                      ; 05 80 1E 3C E0 03 DE 27
8001F38C: lui     $s7, 0x3C  # '<'                                         ; 3C 00 17 3C
8001F390: lui     $s6, 0x28  # '('                                         ; 28 00 16 3C
8001F394: move    $s4, $zero                                               ; 21 A0 00 00
8001F398: lui     $s3, 0x14                                                ; 14 00 13 3C
8001F39C: move    $a0, $zero                                               ; 21 20 00 00
8001F3A0: sra     $s1, $s3, 16                                             ; 03 8C 13 00
8001F3A4: move    $a1, $s1                                                 ; 21 28 20 02
8001F3A8: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001F3B0: jal     sub_8001C550                                             ; 54 71 00 0C
8001F3B4: move    $a3, $s2                                                 ; 21 38 40 02
8001F3B8: move    $a0, $zero                                               ; 21 20 00 00
8001F3BC: sra     $s0, $s4, 16                                             ; 03 84 14 00
8001F3C0: move    $a1, $s0                                                 ; 21 28 00 02
8001F3C4: move    $a2, $fp                                                 ; 21 30 C0 03
8001F3C8: jal     sub_8001C550                                             ; 54 71 00 0C
8001F3CC: move    $a3, $s2                                                 ; 21 38 40 02
8001F3D0: li      $a0, 0x12C                                               ; 2C 01 04 34
8001F3D4: move    $a1, $s1                                                 ; 21 28 20 02
8001F3D8: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001F3E0: jal     sub_8001C550                                             ; 54 71 00 0C
8001F3E4: move    $a3, $s2                                                 ; 21 38 40 02
8001F3E8: li      $a0, 0x12C                                               ; 2C 01 04 34
8001F3EC: move    $a1, $s0                                                 ; 21 28 00 02
8001F3F0: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001F3F8: jal     sub_8001C550                                             ; 54 71 00 0C
8001F3FC: move    $a3, $s2                                                 ; 21 38 40 02
8001F400: sra     $s1, $s6, 16                                             ; 03 8C 16 00
8001F404: move    $a0, $s1                                                 ; 21 20 20 02
8001F408: li      $a1, 0xC8                                                ; C8 00 05 34
8001F40C: move    $a2, $fp                                                 ; 21 30 C0 03
8001F410: jal     sub_8001C550                                             ; 54 71 00 0C
8001F414: move    $a3, $s2                                                 ; 21 38 40 02
8001F418: sra     $s0, $s7, 16                                             ; 03 84 17 00
8001F41C: move    $a0, $s0                                                 ; 21 20 00 02
8001F420: li      $a1, 0xC8                                                ; C8 00 05 34
8001F424: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001F42C: jal     sub_8001C550                                             ; 54 71 00 0C
8001F430: move    $a3, $s2                                                 ; 21 38 40 02
8001F434: move    $a0, $s1                                                 ; 21 20 20 02
8001F438: li      $a1, 0xB4                                                ; B4 00 05 34
8001F43C: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001F444: jal     sub_8001C550                                             ; 54 71 00 0C
8001F448: move    $a3, $s2                                                 ; 21 38 40 02
8001F44C: move    $a0, $s0                                                 ; 21 20 00 02
8001F450: li      $a1, 0xB4                                                ; B4 00 05 34
8001F454: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001F45C: jal     sub_8001C550                                             ; 54 71 00 0C
8001F460: move    $a3, $s2                                                 ; 21 38 40 02
8001F464: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001F468: addu    $s7, $v0                                                 ; 21 B8 E2 02
8001F46C: addu    $s6, $v0                                                 ; 21 B0 C2 02
8001F470: addu    $s4, $v0                                                 ; 21 A0 82 02
8001F474: addu    $s3, $v0                                                 ; 21 98 62 02
8001F478: addiu   $s5, 1                                                   ; 01 00 B5 26
8001F47C: slti    $v0, $s5, 6                                              ; 06 00 A2 2A
8001F480: bnez    $v0, loc_8001F3A0                                        ; C7 FF 40 14
8001F484: move    $a0, $zero                                               ; 21 20 00 00
8001F488: li      $a0, 0x118                                               ; 18 01 04 34
8001F48C: li      $a1, 0xC8                                                ; C8 00 05 34
8001F490: move    $a2, $fp                                                 ; 21 30 C0 03
8001F494: jal     sub_8001C550                                             ; 54 71 00 0C
8001F498: move    $a3, $s2                                                 ; 21 38 40 02
8001F49C: li      $a0, 0x14                                                ; 14 00 04 34
8001F4A0: li      $a1, 0xC8                                                ; C8 00 05 34
8001F4A4: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001F4AC: jal     sub_8001C550                                             ; 54 71 00 0C
8001F4B0: move    $a3, $s2                                                 ; 21 38 40 02
8001F4B4: li      $a0, 0x118                                               ; 18 01 04 34
8001F4B8: li      $a1, 0xB4                                                ; B4 00 05 34
8001F4BC: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001F4C4: jal     sub_8001C550                                             ; 54 71 00 0C
8001F4C8: move    $a3, $s2                                                 ; 21 38 40 02
8001F4CC: li      $a0, 0x14                                                ; 14 00 04 34
8001F4D0: li      $a1, 0xB4                                                ; B4 00 05 34
8001F4D4: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001F4DC: jal     sub_8001C550                                             ; 54 71 00 0C
8001F4E0: move    $a3, $s2                                                 ; 21 38 40 02
8001F4E4: lw      $ra, 0x10+var_s24($sp)                                   ; 34 00 BF 8F
8001F4E8: lw      $fp, 0x10+var_s20($sp)                                   ; 30 00 BE 8F
8001F4EC: lw      $s7, 0x10+var_s1C($sp)                                   ; 2C 00 B7 8F
8001F4F0: lw      $s6, 0x10+var_s18($sp)                                   ; 28 00 B6 8F
8001F4F4: lw      $s5, 0x10+var_s14($sp)                                   ; 24 00 B5 8F
8001F4F8: lw      $s4, 0x10+var_s10($sp)                                   ; 20 00 B4 8F
8001F4FC: lw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 8F
8001F500: lw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 8F
8001F504: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
8001F508: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
8001F50C: addiu   $sp, 0x38                                                ; 38 00 BD 27
8001F510: jr      $ra                                                      ; 08 00 E0 03
8001F514: nop                                                              ; 00 00 00 00
```

## `8001FEB4` `sub_8001FEB4`
Bounds: `8001FEB4-8001FFD4`

### Pseudocode
```c
BOOL __fastcall sub_8001FEB4(unsigned __int16 a1)
{
  int v2; // $fp
  int v3; // $s7
  int v4; // $s3
  int v5; // $s2
  int v6; // $s1
  BOOL result; // $v0
  int i; // [sp+10h] [-8h]

  v2 = 1310720;
  v3 = 0;
  for ( i = 0; i < 8; ++i )
  {
    v4 = 0;
    v5 = 1310720;
    v6 = 0;
    do
    {
      sub_8001C550(SHIWORD(v3), SHIWORD(v6), (int)dword_800503E0, a1);
      sub_8001C550(SHIWORD(v2), SHIWORD(v6), (int)dword_800503F0, a1);
      sub_8001C550(SHIWORD(v3), SHIWORD(v5), (int)dword_80050400, a1);
      sub_8001C550(SHIWORD(v2), SHIWORD(v5), (int)dword_80050410, a1);
      v5 += 2621440;
      ++v4;
      v6 += 2621440;
    }
    while ( v4 < 6 );
    v2 += 2621440;
    v3 += 2621440;
    result = i + 1 < 8;
  }
  return result;
}

```

### Calls in function
- 8001FF18: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001FF30: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001FF4C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001FF64: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001FF7C: `bnez    $v0, loc_8001FF04` -> 8001FF04 loc_8001FF04
- 8001FF98: `bnez    $v0, loc_8001FEF0` -> 8001FEF0 loc_8001FEF0

### Xrefs to
- 80020330 -> 8001FEB4 type=17 func=sub_80020308 line=`jal     sub_8001FEB4`
- 80020390 -> 8001FEB4 type=17 func=sub_80020308 line=`jal     sub_8001FEB4`

### Xrefs from
- 8001FEB4 -> 8001FEB8 type=21 target=sub_8001FEB8 line=`addiu   $sp, -0x40`
- 8001FEB8 -> 8001FEBC type=21 target=sub_8001FEBC line=`sw      $s4, 0x18+var_s10($sp)`
- 8001FEBC -> 8001FEC0 type=21 target=sub_8001FEC0 line=`move    $s4, $a0`
- 8001FEC0 -> 8001FEC4 type=21 target=sub_8001FEC4 line=`sw      $fp, 0x18+var_s20($sp)`
- 8001FEC4 -> 8001FEC8 type=21 target=sub_8001FEC8 line=`lui     $fp, 0x14`
- 8001FEC8 -> 8001FECC type=21 target=sub_8001FECC line=`sw      $s7, 0x18+var_s1C($sp)`
- 8001FECC -> 8001FED0 type=21 target=sub_8001FED0 line=`move    $s7, $zero`
- 8001FED0 -> 8001FED4 type=21 target=sub_8001FED4 line=`sw      $ra, 0x18+var_s24($sp)`
- 8001FED4 -> 8001FED8 type=21 target=sub_8001FED8 line=`sw      $s6, 0x18+var_s18($sp)`
- 8001FED8 -> 8001FEDC type=21 target=sub_8001FEDC line=`sw      $s5, 0x18+var_s14($sp)`
- 8001FEDC -> 8001FEE0 type=21 target=sub_8001FEE0 line=`sw      $s3, 0x18+var_sC($sp)`
- 8001FEE0 -> 8001FEE4 type=21 target=sub_8001FEE4 line=`sw      $s2, 0x18+var_s8($sp)`
- 8001FEE4 -> 8001FEE8 type=21 target=sub_8001FEE8 line=`sw      $s1, 0x18+var_s4($sp)`
- 8001FEE8 -> 8001FEEC type=21 target=sub_8001FEEC line=`sw      $s0, 0x18+var_s0($sp)`
- 8001FEEC -> 8001FEF0 type=21 target=loc_8001FEF0 line=`sw      $zero, 0x18+var_8($sp)`
- 8001FEF0 -> 8001FEF4 type=21 target=sub_8001FEF4 line=`move    $s3, $zero`
- 8001FEF4 -> 8001FEF8 type=21 target=sub_8001FEF8 line=`sra     $s6, $s7, 16`
- 8001FEF8 -> 8001FEFC type=21 target=sub_8001FEFC line=`sra     $s5, $fp, 16`
- 8001FEFC -> 8001FF00 type=21 target=sub_8001FF00 line=`lui     $s2, 0x14`
- 8001FF00 -> 8001FF04 type=21 target=loc_8001FF04 line=`move    $s1, $zero`
- 8001FF04 -> 8001FF08 type=21 target=sub_8001FF08 line=`move    $a0, $s6`
- 8001FF08 -> 8001FF0C type=21 target=sub_8001FF0C line=`sra     $s0, $s1, 16`
- 8001FF0C -> 8001FF10 type=21 target=sub_8001FF10 line=`move    $a1, $s0`
- 8001FF10 -> 8001FF18 type=21 target=sub_8001FF18 line=`li      $a2, dword_800503E0`
- 8001FF18 -> 8001FF1C type=21 target=sub_8001FF1C line=`jal     sub_8001C550`
- 8001FF18 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001FF1C -> 8001FF20 type=21 target=sub_8001FF20 line=`move    $a3, $s4`
- 8001FF20 -> 8001FF24 type=21 target=sub_8001FF24 line=`move    $a0, $s5`
- 8001FF24 -> 8001FF28 type=21 target=sub_8001FF28 line=`move    $a1, $s0`
- 8001FF28 -> 8001FF30 type=21 target=sub_8001FF30 line=`li      $a2, dword_800503F0`
- 8001FF30 -> 8001FF34 type=21 target=sub_8001FF34 line=`jal     sub_8001C550`
- 8001FF30 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001FF34 -> 8001FF38 type=21 target=sub_8001FF38 line=`move    $a3, $s4`
- 8001FF38 -> 8001FF3C type=21 target=sub_8001FF3C line=`move    $a0, $s6`
- 8001FF3C -> 8001FF40 type=21 target=sub_8001FF40 line=`sra     $s0, $s2, 16`
- 8001FF40 -> 8001FF44 type=21 target=sub_8001FF44 line=`move    $a1, $s0`
- 8001FF44 -> 8001FF4C type=21 target=sub_8001FF4C line=`li      $a2, dword_80050400`
- 8001FF4C -> 8001FF50 type=21 target=sub_8001FF50 line=`jal     sub_8001C550`
- 8001FF4C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001FF50 -> 8001FF54 type=21 target=sub_8001FF54 line=`move    $a3, $s4`
- 8001FF54 -> 8001FF58 type=21 target=sub_8001FF58 line=`move    $a0, $s5`
- 8001FF58 -> 8001FF5C type=21 target=sub_8001FF5C line=`move    $a1, $s0`
- 8001FF5C -> 8001FF64 type=21 target=sub_8001FF64 line=`li      $a2, dword_80050410`
- 8001FF64 -> 8001FF68 type=21 target=sub_8001FF68 line=`jal     sub_8001C550`
- 8001FF64 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001FF68 -> 8001FF6C type=21 target=sub_8001FF6C line=`move    $a3, $s4`
- 8001FF6C -> 8001FF70 type=21 target=sub_8001FF70 line=`lui     $v1, 0x28  # '('`
- 8001FF70 -> 8001FF74 type=21 target=sub_8001FF74 line=`addu    $s2, $v1`
- 8001FF74 -> 8001FF78 type=21 target=sub_8001FF78 line=`addiu   $s3, 1`
- 8001FF78 -> 8001FF7C type=21 target=sub_8001FF7C line=`slti    $v0, $s3, 6`
- 8001FF7C -> 8001FF80 type=21 target=sub_8001FF80 line=`bnez    $v0, loc_8001FF04`
- 8001FF7C -> 8001FF04 type=19 target=loc_8001FF04 line=`bnez    $v0, loc_8001FF04`
- 8001FF80 -> 8001FF84 type=21 target=sub_8001FF84 line=`addu    $s1, $v1`
- 8001FF84 -> 8001FF88 type=21 target=sub_8001FF88 line=`addu    $fp, $v1`
- 8001FF88 -> 8001FF8C type=21 target=sub_8001FF8C line=`lw      $t0, 0x18+var_8($sp)`
- 8001FF8C -> 8001FF90 type=21 target=sub_8001FF90 line=`addu    $s7, $v1`
- 8001FF90 -> 8001FF94 type=21 target=sub_8001FF94 line=`addiu   $t0, 1`
- 8001FF94 -> 8001FF98 type=21 target=sub_8001FF98 line=`slti    $v0, $t0, 8`
- 8001FF98 -> 8001FF9C type=21 target=sub_8001FF9C line=`bnez    $v0, loc_8001FEF0`
- 8001FF98 -> 8001FEF0 type=19 target=loc_8001FEF0 line=`bnez    $v0, loc_8001FEF0`
- 8001FF9C -> 8001FFA0 type=21 target=sub_8001FFA0 line=`sw      $t0, 0x18+var_8($sp)`
- 8001FFA0 -> 8001FFA4 type=21 target=sub_8001FFA4 line=`lw      $ra, 0x18+var_s24($sp)`
- 8001FFA4 -> 8001FFA8 type=21 target=sub_8001FFA8 line=`lw      $fp, 0x18+var_s20($sp)`
- 8001FFA8 -> 8001FFAC type=21 target=sub_8001FFAC line=`lw      $s7, 0x18+var_s1C($sp)`
- 8001FFAC -> 8001FFB0 type=21 target=sub_8001FFB0 line=`lw      $s6, 0x18+var_s18($sp)`
- 8001FFB0 -> 8001FFB4 type=21 target=sub_8001FFB4 line=`lw      $s5, 0x18+var_s14($sp)`
- 8001FFB4 -> 8001FFB8 type=21 target=sub_8001FFB8 line=`lw      $s4, 0x18+var_s10($sp)`
- 8001FFB8 -> 8001FFBC type=21 target=sub_8001FFBC line=`lw      $s3, 0x18+var_sC($sp)`
- 8001FFBC -> 8001FFC0 type=21 target=sub_8001FFC0 line=`lw      $s2, 0x18+var_s8($sp)`
- 8001FFC0 -> 8001FFC4 type=21 target=sub_8001FFC4 line=`lw      $s1, 0x18+var_s4($sp)`
- 8001FFC4 -> 8001FFC8 type=21 target=sub_8001FFC8 line=`lw      $s0, 0x18+var_s0($sp)`
- 8001FFC8 -> 8001FFCC type=21 target=sub_8001FFCC line=`addiu   $sp, 0x40`
- 8001FFCC -> 8001FFD0 type=21 target=sub_8001FFD0 line=`jr      $ra`

### Full disasm
```asm
8001FEB4: addiu   $sp, -0x40                                               ; C0 FF BD 27
8001FEB8: sw      $s4, 0x18+var_s10($sp)                                   ; 28 00 B4 AF
8001FEBC: move    $s4, $a0                                                 ; 21 A0 80 00
8001FEC0: sw      $fp, 0x18+var_s20($sp)                                   ; 38 00 BE AF
8001FEC4: lui     $fp, 0x14                                                ; 14 00 1E 3C
8001FEC8: sw      $s7, 0x18+var_s1C($sp)                                   ; 34 00 B7 AF
8001FECC: move    $s7, $zero                                               ; 21 B8 00 00
8001FED0: sw      $ra, 0x18+var_s24($sp)                                   ; 3C 00 BF AF
8001FED4: sw      $s6, 0x18+var_s18($sp)                                   ; 30 00 B6 AF
8001FED8: sw      $s5, 0x18+var_s14($sp)                                   ; 2C 00 B5 AF
8001FEDC: sw      $s3, 0x18+var_sC($sp)                                    ; 24 00 B3 AF
8001FEE0: sw      $s2, 0x18+var_s8($sp)                                    ; 20 00 B2 AF
8001FEE4: sw      $s1, 0x18+var_s4($sp)                                    ; 1C 00 B1 AF
8001FEE8: sw      $s0, 0x18+var_s0($sp)                                    ; 18 00 B0 AF
8001FEEC: sw      $zero, 0x18+var_8($sp)                                   ; 10 00 A0 AF
8001FEF0: move    $s3, $zero                                               ; 21 98 00 00
8001FEF4: sra     $s6, $s7, 16                                             ; 03 B4 17 00
8001FEF8: sra     $s5, $fp, 16                                             ; 03 AC 1E 00
8001FEFC: lui     $s2, 0x14                                                ; 14 00 12 3C
8001FF00: move    $s1, $zero                                               ; 21 88 00 00
8001FF04: move    $a0, $s6                                                 ; 21 20 C0 02
8001FF08: sra     $s0, $s1, 16                                             ; 03 84 11 00
8001FF0C: move    $a1, $s0                                                 ; 21 28 00 02
8001FF10: li      $a2, dword_800503E0                                      ; 05 80 06 3C E0 03 C6 24
8001FF18: jal     sub_8001C550                                             ; 54 71 00 0C
8001FF1C: move    $a3, $s4                                                 ; 21 38 80 02
8001FF20: move    $a0, $s5                                                 ; 21 20 A0 02
8001FF24: move    $a1, $s0                                                 ; 21 28 00 02
8001FF28: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001FF30: jal     sub_8001C550                                             ; 54 71 00 0C
8001FF34: move    $a3, $s4                                                 ; 21 38 80 02
8001FF38: move    $a0, $s6                                                 ; 21 20 C0 02
8001FF3C: sra     $s0, $s2, 16                                             ; 03 84 12 00
8001FF40: move    $a1, $s0                                                 ; 21 28 00 02
8001FF44: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001FF4C: jal     sub_8001C550                                             ; 54 71 00 0C
8001FF50: move    $a3, $s4                                                 ; 21 38 80 02
8001FF54: move    $a0, $s5                                                 ; 21 20 A0 02
8001FF58: move    $a1, $s0                                                 ; 21 28 00 02
8001FF5C: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001FF64: jal     sub_8001C550                                             ; 54 71 00 0C
8001FF68: move    $a3, $s4                                                 ; 21 38 80 02
8001FF6C: lui     $v1, 0x28  # '('                                         ; 28 00 03 3C
8001FF70: addu    $s2, $v1                                                 ; 21 90 43 02
8001FF74: addiu   $s3, 1                                                   ; 01 00 73 26
8001FF78: slti    $v0, $s3, 6                                              ; 06 00 62 2A
8001FF7C: bnez    $v0, loc_8001FF04                                        ; E1 FF 40 14
8001FF80: addu    $s1, $v1                                                 ; 21 88 23 02
8001FF84: addu    $fp, $v1                                                 ; 21 F0 C3 03
8001FF88: lw      $t0, 0x18+var_8($sp)                                     ; 10 00 A8 8F
8001FF8C: addu    $s7, $v1                                                 ; 21 B8 E3 02
8001FF90: addiu   $t0, 1                                                   ; 01 00 08 25
8001FF94: slti    $v0, $t0, 8                                              ; 08 00 02 29
8001FF98: bnez    $v0, loc_8001FEF0                                        ; D5 FF 40 14
8001FF9C: sw      $t0, 0x18+var_8($sp)                                     ; 10 00 A8 AF
8001FFA0: lw      $ra, 0x18+var_s24($sp)                                   ; 3C 00 BF 8F
8001FFA4: lw      $fp, 0x18+var_s20($sp)                                   ; 38 00 BE 8F
8001FFA8: lw      $s7, 0x18+var_s1C($sp)                                   ; 34 00 B7 8F
8001FFAC: lw      $s6, 0x18+var_s18($sp)                                   ; 30 00 B6 8F
8001FFB0: lw      $s5, 0x18+var_s14($sp)                                   ; 2C 00 B5 8F
8001FFB4: lw      $s4, 0x18+var_s10($sp)                                   ; 28 00 B4 8F
8001FFB8: lw      $s3, 0x18+var_sC($sp)                                    ; 24 00 B3 8F
8001FFBC: lw      $s2, 0x18+var_s8($sp)                                    ; 20 00 B2 8F
8001FFC0: lw      $s1, 0x18+var_s4($sp)                                    ; 1C 00 B1 8F
8001FFC4: lw      $s0, 0x18+var_s0($sp)                                    ; 18 00 B0 8F
8001FFC8: addiu   $sp, 0x40                                                ; 40 00 BD 27
8001FFCC: jr      $ra                                                      ; 08 00 E0 03
8001FFD0: nop                                                              ; 00 00 00 00
```

## `8001C864` `sub_8001C864`
Bounds: `8001C864-8001CE30`

### Pseudocode
```c
int __fastcall sub_8001C864(unsigned __int16 a1)
{
  int v1; // $gp
  int v3; // $s4
  int v4; // $s6
  int v5; // $s5
  int v6; // $gp
  int v7; // $gp
  int v8; // $gp
  int v9; // $gp
  int v10; // $s1
  int v11; // $gp
  int v12; // $gp
  int v13; // $gp
  int v14; // $s4
  int v15; // $fp
  int v16; // $s7
  int v17; // $s6
  int v18; // $s5
  __int16 v19; // $s0
  __int16 v20; // $a1
  int v21; // $gp
  int v22; // $gp
  int v23; // $gp
  int v24; // $gp
  int v25; // $gp
  int v26; // $gp
  int v27; // $gp
  int v28; // $gp
  int v29; // $gp
  int v30; // $gp
  int v31; // $gp
  int v32; // $gp

  v3 = 0;
  v4 = 1310720;
  v5 = 2621440;
  sub_8001B590(280, 200, (int)dword_8004E940, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v1 + 872));
  sub_8001B590(20, 200, (int)dword_8004E950, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v6 + 872));
  sub_8001B590(280, 180, (int)dword_8004E960, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v7 + 872));
  sub_8001B590(20, 180, (int)dword_8004E970, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v8 + 872));
  LOWORD(v10) = 40;
  do
  {
    ++v3;
    sub_8001B590(v10, 0, (int)dword_8004E900, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
    sub_8001B590(SHIWORD(v4), 0, (int)dword_8004E910, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v11 + 872));
    sub_8001B590(v10, 220, (int)dword_8004E920, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v12 + 872));
    sub_8001B590(SHIWORD(v4), 220, (int)dword_8004E930, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v13 + 872));
    v4 += 2621440;
    v5 += 2621440;
    v10 = v5 >> 16;
  }
  while ( v3 < 7 );
  v14 = 0;
  v15 = 3932160;
  v16 = 2621440;
  v17 = 1310720;
  v18 = 0;
  do
  {
    ++v14;
    sub_8001B590(0, SHIWORD(v17), (int)dword_8004E920, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
    v19 = 8 * v18;
    v20 = 8 * v18;
    v18 += 5;
    sub_8001B590(0, v20, (int)dword_8004E900, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v21 + 872));
    sub_8001B590(300, SHIWORD(v17), (int)dword_8004E930, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v22 + 872));
    sub_8001B590(300, v19, (int)dword_8004E910, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v23 + 872));
    sub_8001B590(SHIWORD(v16), 200, (int)dword_8004E860, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v24 + 872));
    sub_8001B590(SHIWORD(v15), 200, (int)dword_8004E870, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v25 + 872));
    sub_8001B590(SHIWORD(v16), 180, (int)dword_8004E880, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v26 + 872));
    sub_8001B590(SHIWORD(v15), 180, (int)dword_8004E890, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v27 + 872));
    v15 += 2621440;
    v16 += 2621440;
    v17 += 2621440;
  }
  while ( v14 < 6 );
  sub_8001B590(40, 160, (int)dword_8004E8E0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
  sub_8001B590(160, 160, (int)dword_8004E8F0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v28 + 872));
  sub_8001B590(40, 20, (int)dword_8004E8C0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v29 + 872));
  sub_8001B590(160, 20, (int)dword_8004E8D0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v30 + 872));
  sub_8001B590(20, 20, (int)dword_8004E8A0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v31 + 872));
  return sub_8001B590(280, 20, (int)dword_8004E8B0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v32 + 872));
}

```

### Calls in function
- 8001C8DC: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001C914: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001C94C: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001C984: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001C9C4: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CA00: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CA38: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CA70: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CA88: `bnez    $v0, loc_8001C990` -> 8001C990 loc_8001C990
- 8001CAE4: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CB28: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CB60: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CB98: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CBD4: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CC10: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CC48: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CC80: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CC9C: `bnez    $v0, loc_8001CAB0` -> 8001CAB0 loc_8001CAB0
- 8001CCDC: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CD14: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CD4C: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CD84: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CDBC: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CDF4: `jal     sub_8001B590` -> 8001B590 sub_8001B590

### Xrefs to
- 8001DB38 -> 8001C864 type=17 func=sub_8001DB00 line=`jal     sub_8001C864`
- 8001DB70 -> 8001C864 type=17 func=sub_8001DB58 line=`jal     sub_8001C864`
- 800202CC -> 8001C864 type=17 func=sub_80020248 line=`jal     sub_8001C864`

### Xrefs from
- 8001C864 -> 8001C868 type=21 target=sub_8001C868 line=`addiu   $sp, -0x48`
- 8001C868 -> 8001C86C type=21 target=sub_8001C86C line=`sw      $s2, 0x20+var_s8($sp)`
- 8001C86C -> 8001C870 type=21 target=sub_8001C870 line=`move    $s2, $a0`
- 8001C870 -> 8001C874 type=21 target=sub_8001C874 line=`li      $a0, 0x118`
- 8001C874 -> 8001C878 type=21 target=sub_8001C878 line=`li      $a1, 0xC8`
- 8001C878 -> 8001C880 type=21 target=sub_8001C880 line=`li      $a2, dword_8004E940`
- 8001C880 -> 8001C884 type=21 target=sub_8001C884 line=`move    $a3, $zero`
- 8001C884 -> 8001C888 type=21 target=sub_8001C888 line=`sw      $s4, 0x20+var_s10($sp)`
- 8001C888 -> 8001C88C type=21 target=sub_8001C88C line=`move    $s4, $zero`
- 8001C88C -> 8001C890 type=21 target=sub_8001C890 line=`lw      $v1, 0x368($gp)`
- 8001C890 -> 8001C898 type=21 target=sub_8001C898 line=`li      $v0, unk_80087288`
- 8001C898 -> 8001C89C type=21 target=sub_8001C89C line=`sw      $s3, 0x20+var_sC($sp)`
- 8001C89C -> 8001C8A0 type=21 target=sub_8001C8A0 line=`move    $s3, $v0`
- 8001C8A0 -> 8001C8A4 type=21 target=sub_8001C8A4 line=`sw      $s6, 0x20+var_s18($sp)`
- 8001C8A4 -> 8001C8A8 type=21 target=sub_8001C8A8 line=`lui     $s6, 0x14`
- 8001C8A8 -> 8001C8AC type=21 target=sub_8001C8AC line=`sw      $s5, 0x20+var_s14($sp)`
- 8001C8AC -> 8001C8B0 type=21 target=sub_8001C8B0 line=`lui     $s5, 0x28  # '('`
- 8001C8B0 -> 8001C8B4 type=21 target=sub_8001C8B4 line=`sw      $ra, 0x20+var_s24($sp)`
- 8001C8B4 -> 8001C8B8 type=21 target=sub_8001C8B8 line=`sw      $fp, 0x20+var_s20($sp)`
- 8001C8B8 -> 8001C8BC type=21 target=sub_8001C8BC line=`sw      $s7, 0x20+var_s1C($sp)`
- 8001C8BC -> 8001C8C0 type=21 target=sub_8001C8C0 line=`sw      $s1, 0x20+var_s4($sp)`
- 8001C8C0 -> 8001C8C4 type=21 target=sub_8001C8C4 line=`sw      $s0, 0x20+var_s0($sp)`
- 8001C8C4 -> 8001C8C8 type=21 target=sub_8001C8C8 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C8C8 -> 8001C8CC type=21 target=sub_8001C8CC line=`sw      $s2, 0x20+var_C($sp)`
- 8001C8CC -> 8001C8D0 type=21 target=sub_8001C8D0 line=`sll     $v0, $v1, 2`
- 8001C8D0 -> 8001C8D4 type=21 target=sub_8001C8D4 line=`addu    $v0, $v1`
- 8001C8D4 -> 8001C8D8 type=21 target=sub_8001C8D8 line=`sll     $v0, 2`
- 8001C8D8 -> 8001C8DC type=21 target=sub_8001C8DC line=`addu    $v0, $s3`
- 8001C8DC -> 8001C8E0 type=21 target=sub_8001C8E0 line=`jal     sub_8001B590`
- 8001C8DC -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C8E0 -> 8001C8E4 type=21 target=sub_8001C8E4 line=`sw      $v0, 0x20+var_8($sp)`
- 8001C8E4 -> 8001C8E8 type=21 target=sub_8001C8E8 line=`li      $a0, 0x14`
- 8001C8E8 -> 8001C8EC type=21 target=sub_8001C8EC line=`li      $a1, 0xC8`
- 8001C8EC -> 8001C8F4 type=21 target=sub_8001C8F4 line=`li      $a2, dword_8004E950`
- 8001C8F4 -> 8001C8F8 type=21 target=sub_8001C8F8 line=`lw      $v1, 0x368($gp)`
- 8001C8F8 -> 8001C8FC type=21 target=sub_8001C8FC line=`move    $a3, $zero`
- 8001C8FC -> 8001C900 type=21 target=sub_8001C900 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C900 -> 8001C904 type=21 target=sub_8001C904 line=`sw      $s2, 0x20+var_C($sp)`
- 8001C904 -> 8001C908 type=21 target=sub_8001C908 line=`sll     $v0, $v1, 2`
- 8001C908 -> 8001C90C type=21 target=sub_8001C90C line=`addu    $v0, $v1`
- 8001C90C -> 8001C910 type=21 target=sub_8001C910 line=`sll     $v0, 2`
- 8001C910 -> 8001C914 type=21 target=sub_8001C914 line=`addu    $v0, $s3`
- 8001C914 -> 8001C918 type=21 target=sub_8001C918 line=`jal     sub_8001B590`
- 8001C914 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C918 -> 8001C91C type=21 target=sub_8001C91C line=`sw      $v0, 0x20+var_8($sp)`
- 8001C91C -> 8001C920 type=21 target=sub_8001C920 line=`li      $a0, 0x118`
- 8001C920 -> 8001C924 type=21 target=sub_8001C924 line=`li      $a1, 0xB4`
- 8001C924 -> 8001C92C type=21 target=sub_8001C92C line=`li      $a2, dword_8004E960`
- 8001C92C -> 8001C930 type=21 target=sub_8001C930 line=`lw      $v1, 0x368($gp)`
- 8001C930 -> 8001C934 type=21 target=sub_8001C934 line=`move    $a3, $zero`
- 8001C934 -> 8001C938 type=21 target=sub_8001C938 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C938 -> 8001C93C type=21 target=sub_8001C93C line=`sw      $s2, 0x20+var_C($sp)`
- 8001C93C -> 8001C940 type=21 target=sub_8001C940 line=`sll     $v0, $v1, 2`
- 8001C940 -> 8001C944 type=21 target=sub_8001C944 line=`addu    $v0, $v1`
- 8001C944 -> 8001C948 type=21 target=sub_8001C948 line=`sll     $v0, 2`
- 8001C948 -> 8001C94C type=21 target=sub_8001C94C line=`addu    $v0, $s3`
- 8001C94C -> 8001C950 type=21 target=sub_8001C950 line=`jal     sub_8001B590`
- 8001C94C -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C950 -> 8001C954 type=21 target=sub_8001C954 line=`sw      $v0, 0x20+var_8($sp)`
- 8001C954 -> 8001C958 type=21 target=sub_8001C958 line=`li      $a0, 0x14`
- 8001C958 -> 8001C95C type=21 target=sub_8001C95C line=`li      $a1, 0xB4`
- 8001C95C -> 8001C964 type=21 target=sub_8001C964 line=`li      $a2, dword_8004E970`
- 8001C964 -> 8001C968 type=21 target=sub_8001C968 line=`lw      $v1, 0x368($gp)`
- 8001C968 -> 8001C96C type=21 target=sub_8001C96C line=`move    $a3, $zero`
- 8001C96C -> 8001C970 type=21 target=sub_8001C970 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C970 -> 8001C974 type=21 target=sub_8001C974 line=`sw      $s2, 0x20+var_C($sp)`
- 8001C974 -> 8001C978 type=21 target=sub_8001C978 line=`sll     $v0, $v1, 2`
- 8001C978 -> 8001C97C type=21 target=sub_8001C97C line=`addu    $v0, $v1`
- 8001C97C -> 8001C980 type=21 target=sub_8001C980 line=`sll     $v0, 2`
- 8001C980 -> 8001C984 type=21 target=sub_8001C984 line=`addu    $v0, $s3`
- 8001C984 -> 8001C988 type=21 target=sub_8001C988 line=`jal     sub_8001B590`
- 8001C984 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C988 -> 8001C98C type=21 target=sub_8001C98C line=`sw      $v0, 0x20+var_8($sp)`
- 8001C98C -> 8001C990 type=21 target=loc_8001C990 line=`sra     $s1, $s5, 16`
- 8001C990 -> 8001C994 type=21 target=sub_8001C994 line=`move    $a0, $s1`
- 8001C994 -> 8001C998 type=21 target=sub_8001C998 line=`move    $a1, $zero`
- 8001C998 -> 8001C9A0 type=21 target=sub_8001C9A0 line=`li      $a2, dword_8004E900`
- 8001C9A0 -> 8001C9A4 type=21 target=sub_8001C9A4 line=`move    $a3, $zero`
- 8001C9A4 -> 8001C9A8 type=21 target=sub_8001C9A8 line=`lw      $v1, 0x368($gp)`
- 8001C9A8 -> 8001C9AC type=21 target=sub_8001C9AC line=`addiu   $s4, 1`
- 8001C9AC -> 8001C9B0 type=21 target=sub_8001C9B0 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C9B0 -> 8001C9B4 type=21 target=sub_8001C9B4 line=`sw      $s2, 0x20+var_C($sp)`
- 8001C9B4 -> 8001C9B8 type=21 target=sub_8001C9B8 line=`sll     $v0, $v1, 2`
- 8001C9B8 -> 8001C9BC type=21 target=sub_8001C9BC line=`addu    $v0, $v1`
- 8001C9BC -> 8001C9C0 type=21 target=sub_8001C9C0 line=`sll     $v0, 2`
- 8001C9C0 -> 8001C9C4 type=21 target=sub_8001C9C4 line=`addu    $v0, $s3`
- 8001C9C4 -> 8001C9C8 type=21 target=sub_8001C9C8 line=`jal     sub_8001B590`
- 8001C9C4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C9C8 -> 8001C9CC type=21 target=sub_8001C9CC line=`sw      $v0, 0x20+var_8($sp)`
- 8001C9CC -> 8001C9D0 type=21 target=sub_8001C9D0 line=`sra     $s0, $s6, 16`
- 8001C9D0 -> 8001C9D4 type=21 target=sub_8001C9D4 line=`move    $a0, $s0`
- 8001C9D4 -> 8001C9D8 type=21 target=sub_8001C9D8 line=`move    $a1, $zero`
- 8001C9D8 -> 8001C9E0 type=21 target=sub_8001C9E0 line=`li      $a2, dword_8004E910`
- 8001C9E0 -> 8001C9E4 type=21 target=sub_8001C9E4 line=`lw      $v1, 0x368($gp)`
- 8001C9E4 -> 8001C9E8 type=21 target=sub_8001C9E8 line=`move    $a3, $zero`
- 8001C9E8 -> 8001C9EC type=21 target=sub_8001C9EC line=`sw      $zero, 0x20+var_10($sp)`
- 8001C9EC -> 8001C9F0 type=21 target=sub_8001C9F0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001C9F0 -> 8001C9F4 type=21 target=sub_8001C9F4 line=`sll     $v0, $v1, 2`
- 8001C9F4 -> 8001C9F8 type=21 target=sub_8001C9F8 line=`addu    $v0, $v1`
- 8001C9F8 -> 8001C9FC type=21 target=sub_8001C9FC line=`sll     $v0, 2`
- 8001C9FC -> 8001CA00 type=21 target=sub_8001CA00 line=`addu    $v0, $s3`
- 8001CA00 -> 8001CA04 type=21 target=sub_8001CA04 line=`jal     sub_8001B590`
- 8001CA00 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CA04 -> 8001CA08 type=21 target=sub_8001CA08 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CA08 -> 8001CA0C type=21 target=sub_8001CA0C line=`move    $a0, $s1`
- 8001CA0C -> 8001CA10 type=21 target=sub_8001CA10 line=`li      $a1, 0xDC`
- 8001CA10 -> 8001CA18 type=21 target=sub_8001CA18 line=`li      $a2, dword_8004E920`
- 8001CA18 -> 8001CA1C type=21 target=sub_8001CA1C line=`lw      $v1, 0x368($gp)`
- 8001CA1C -> 8001CA20 type=21 target=sub_8001CA20 line=`move    $a3, $zero`
- 8001CA20 -> 8001CA24 type=21 target=sub_8001CA24 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CA24 -> 8001CA28 type=21 target=sub_8001CA28 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CA28 -> 8001CA2C type=21 target=sub_8001CA2C line=`sll     $v0, $v1, 2`
- 8001CA2C -> 8001CA30 type=21 target=sub_8001CA30 line=`addu    $v0, $v1`
- 8001CA30 -> 8001CA34 type=21 target=sub_8001CA34 line=`sll     $v0, 2`
- 8001CA34 -> 8001CA38 type=21 target=sub_8001CA38 line=`addu    $v0, $s3`
- 8001CA38 -> 8001CA3C type=21 target=sub_8001CA3C line=`jal     sub_8001B590`
- 8001CA38 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CA3C -> 8001CA40 type=21 target=sub_8001CA40 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CA40 -> 8001CA44 type=21 target=sub_8001CA44 line=`move    $a0, $s0`
- 8001CA44 -> 8001CA48 type=21 target=sub_8001CA48 line=`li      $a1, 0xDC`
- 8001CA48 -> 8001CA50 type=21 target=sub_8001CA50 line=`li      $a2, dword_8004E930`
- 8001CA50 -> 8001CA54 type=21 target=sub_8001CA54 line=`lw      $v1, 0x368($gp)`
- 8001CA54 -> 8001CA58 type=21 target=sub_8001CA58 line=`move    $a3, $zero`
- 8001CA58 -> 8001CA5C type=21 target=sub_8001CA5C line=`sw      $zero, 0x20+var_10($sp)`
- 8001CA5C -> 8001CA60 type=21 target=sub_8001CA60 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CA60 -> 8001CA64 type=21 target=sub_8001CA64 line=`sll     $v0, $v1, 2`
- 8001CA64 -> 8001CA68 type=21 target=sub_8001CA68 line=`addu    $v0, $v1`
- 8001CA68 -> 8001CA6C type=21 target=sub_8001CA6C line=`sll     $v0, 2`
- 8001CA6C -> 8001CA70 type=21 target=sub_8001CA70 line=`addu    $v0, $s3`
- 8001CA70 -> 8001CA74 type=21 target=sub_8001CA74 line=`jal     sub_8001B590`
- 8001CA70 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CA74 -> 8001CA78 type=21 target=sub_8001CA78 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CA78 -> 8001CA7C type=21 target=sub_8001CA7C line=`lui     $v0, 0x28  # '('`
- 8001CA7C -> 8001CA80 type=21 target=sub_8001CA80 line=`addu    $s6, $v0`
- 8001CA80 -> 8001CA84 type=21 target=sub_8001CA84 line=`addu    $s5, $v0`
- 8001CA84 -> 8001CA88 type=21 target=sub_8001CA88 line=`slti    $v0, $s4, 7`
- 8001CA88 -> 8001CA8C type=21 target=sub_8001CA8C line=`bnez    $v0, loc_8001C990`
- 8001CA88 -> 8001C990 type=19 target=loc_8001C990 line=`bnez    $v0, loc_8001C990`
- 8001CA8C -> 8001CA90 type=21 target=sub_8001CA90 line=`sra     $s1, $s5, 16`
- 8001CA90 -> 8001CA94 type=21 target=sub_8001CA94 line=`move    $s4, $zero`
- 8001CA94 -> 8001CA9C type=21 target=sub_8001CA9C line=`li      $s3, unk_80087288`
- 8001CA9C -> 8001CAA0 type=21 target=sub_8001CAA0 line=`lui     $fp, 0x3C  # '<'`
- 8001CAA0 -> 8001CAA4 type=21 target=sub_8001CAA4 line=`lui     $s7, 0x28  # '('`
- 8001CAA4 -> 8001CAA8 type=21 target=sub_8001CAA8 line=`lui     $s6, 0x14`
- 8001CAA8 -> 8001CAAC type=21 target=sub_8001CAAC line=`move    $s5, $zero`
- 8001CAAC -> 8001CAB0 type=21 target=loc_8001CAB0 line=`move    $a0, $zero`
- 8001CAB0 -> 8001CAB4 type=21 target=sub_8001CAB4 line=`sra     $s1, $s6, 16`
- 8001CAB4 -> 8001CAB8 type=21 target=sub_8001CAB8 line=`move    $a1, $s1`
- 8001CAB8 -> 8001CAC0 type=21 target=sub_8001CAC0 line=`li      $a2, dword_8004E920`
- 8001CAC0 -> 8001CAC4 type=21 target=sub_8001CAC4 line=`move    $a3, $zero`
- 8001CAC4 -> 8001CAC8 type=21 target=sub_8001CAC8 line=`lw      $v1, 0x368($gp)`
- 8001CAC8 -> 8001CACC type=21 target=sub_8001CACC line=`addiu   $s4, 1`
- 8001CACC -> 8001CAD0 type=21 target=sub_8001CAD0 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CAD0 -> 8001CAD4 type=21 target=sub_8001CAD4 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CAD4 -> 8001CAD8 type=21 target=sub_8001CAD8 line=`sll     $v0, $v1, 2`
- 8001CAD8 -> 8001CADC type=21 target=sub_8001CADC line=`addu    $v0, $v1`
- 8001CADC -> 8001CAE0 type=21 target=sub_8001CAE0 line=`sll     $v0, 2`
- 8001CAE0 -> 8001CAE4 type=21 target=sub_8001CAE4 line=`addu    $v0, $s3`
- 8001CAE4 -> 8001CAE8 type=21 target=sub_8001CAE8 line=`jal     sub_8001B590`
- 8001CAE4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CAE8 -> 8001CAEC type=21 target=sub_8001CAEC line=`sw      $v0, 0x20+var_8($sp)`
- 8001CAEC -> 8001CAF0 type=21 target=sub_8001CAF0 line=`move    $a0, $zero`
- 8001CAF0 -> 8001CAF4 type=21 target=sub_8001CAF4 line=`sll     $s0, $s5, 19`
- 8001CAF4 -> 8001CAF8 type=21 target=sub_8001CAF8 line=`sra     $s0, 16`
- 8001CAF8 -> 8001CAFC type=21 target=sub_8001CAFC line=`move    $a1, $s0`
- 8001CAFC -> 8001CB04 type=21 target=sub_8001CB04 line=`li      $a2, dword_8004E900`
- 8001CB04 -> 8001CB08 type=21 target=sub_8001CB08 line=`move    $a3, $zero`
- 8001CB08 -> 8001CB0C type=21 target=sub_8001CB0C line=`lw      $v1, 0x368($gp)`
- 8001CB0C -> 8001CB10 type=21 target=sub_8001CB10 line=`addiu   $s5, 5`
- 8001CB10 -> 8001CB14 type=21 target=sub_8001CB14 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CB14 -> 8001CB18 type=21 target=sub_8001CB18 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CB18 -> 8001CB1C type=21 target=sub_8001CB1C line=`sll     $v0, $v1, 2`
- 8001CB1C -> 8001CB20 type=21 target=sub_8001CB20 line=`addu    $v0, $v1`
- 8001CB20 -> 8001CB24 type=21 target=sub_8001CB24 line=`sll     $v0, 2`
- 8001CB24 -> 8001CB28 type=21 target=sub_8001CB28 line=`addu    $v0, $s3`
- 8001CB28 -> 8001CB2C type=21 target=sub_8001CB2C line=`jal     sub_8001B590`
- 8001CB28 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CB2C -> 8001CB30 type=21 target=sub_8001CB30 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CB30 -> 8001CB34 type=21 target=sub_8001CB34 line=`li      $a0, 0x12C`
- 8001CB34 -> 8001CB38 type=21 target=sub_8001CB38 line=`move    $a1, $s1`
- 8001CB38 -> 8001CB40 type=21 target=sub_8001CB40 line=`li      $a2, dword_8004E930`
- 8001CB40 -> 8001CB44 type=21 target=sub_8001CB44 line=`lw      $v1, 0x368($gp)`
- 8001CB44 -> 8001CB48 type=21 target=sub_8001CB48 line=`move    $a3, $zero`
- 8001CB48 -> 8001CB4C type=21 target=sub_8001CB4C line=`sw      $zero, 0x20+var_10($sp)`
- 8001CB4C -> 8001CB50 type=21 target=sub_8001CB50 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CB50 -> 8001CB54 type=21 target=sub_8001CB54 line=`sll     $v0, $v1, 2`
- 8001CB54 -> 8001CB58 type=21 target=sub_8001CB58 line=`addu    $v0, $v1`
- 8001CB58 -> 8001CB5C type=21 target=sub_8001CB5C line=`sll     $v0, 2`
- 8001CB5C -> 8001CB60 type=21 target=sub_8001CB60 line=`addu    $v0, $s3`
- 8001CB60 -> 8001CB64 type=21 target=sub_8001CB64 line=`jal     sub_8001B590`
- 8001CB60 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CB64 -> 8001CB68 type=21 target=sub_8001CB68 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CB68 -> 8001CB6C type=21 target=sub_8001CB6C line=`li      $a0, 0x12C`
- 8001CB6C -> 8001CB70 type=21 target=sub_8001CB70 line=`move    $a1, $s0`
- 8001CB70 -> 8001CB78 type=21 target=sub_8001CB78 line=`li      $a2, dword_8004E910`
- 8001CB78 -> 8001CB7C type=21 target=sub_8001CB7C line=`lw      $v1, 0x368($gp)`
- 8001CB7C -> 8001CB80 type=21 target=sub_8001CB80 line=`move    $a3, $zero`
- 8001CB80 -> 8001CB84 type=21 target=sub_8001CB84 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CB84 -> 8001CB88 type=21 target=sub_8001CB88 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CB88 -> 8001CB8C type=21 target=sub_8001CB8C line=`sll     $v0, $v1, 2`
- 8001CB8C -> 8001CB90 type=21 target=sub_8001CB90 line=`addu    $v0, $v1`
- 8001CB90 -> 8001CB94 type=21 target=sub_8001CB94 line=`sll     $v0, 2`
- 8001CB94 -> 8001CB98 type=21 target=sub_8001CB98 line=`addu    $v0, $s3`
- 8001CB98 -> 8001CB9C type=21 target=sub_8001CB9C line=`jal     sub_8001B590`
- 8001CB98 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CB9C -> 8001CBA0 type=21 target=sub_8001CBA0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CBA0 -> 8001CBA4 type=21 target=sub_8001CBA4 line=`sra     $s1, $s7, 16`
- 8001CBA4 -> 8001CBA8 type=21 target=sub_8001CBA8 line=`move    $a0, $s1`
- 8001CBA8 -> 8001CBAC type=21 target=sub_8001CBAC line=`li      $a1, 0xC8`
- 8001CBAC -> 8001CBB4 type=21 target=sub_8001CBB4 line=`li      $a2, dword_8004E860`
- 8001CBB4 -> 8001CBB8 type=21 target=sub_8001CBB8 line=`lw      $v1, 0x368($gp)`
- 8001CBB8 -> 8001CBBC type=21 target=sub_8001CBBC line=`move    $a3, $zero`
- 8001CBBC -> 8001CBC0 type=21 target=sub_8001CBC0 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CBC0 -> 8001CBC4 type=21 target=sub_8001CBC4 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CBC4 -> 8001CBC8 type=21 target=sub_8001CBC8 line=`sll     $v0, $v1, 2`
- 8001CBC8 -> 8001CBCC type=21 target=sub_8001CBCC line=`addu    $v0, $v1`
- 8001CBCC -> 8001CBD0 type=21 target=sub_8001CBD0 line=`sll     $v0, 2`
- 8001CBD0 -> 8001CBD4 type=21 target=sub_8001CBD4 line=`addu    $v0, $s3`
- 8001CBD4 -> 8001CBD8 type=21 target=sub_8001CBD8 line=`jal     sub_8001B590`
- 8001CBD4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CBD8 -> 8001CBDC type=21 target=sub_8001CBDC line=`sw      $v0, 0x20+var_8($sp)`
- 8001CBDC -> 8001CBE0 type=21 target=sub_8001CBE0 line=`sra     $s0, $fp, 16`
- 8001CBE0 -> 8001CBE4 type=21 target=sub_8001CBE4 line=`move    $a0, $s0`
- 8001CBE4 -> 8001CBE8 type=21 target=sub_8001CBE8 line=`li      $a1, 0xC8`
- 8001CBE8 -> 8001CBF0 type=21 target=sub_8001CBF0 line=`li      $a2, dword_8004E870`
- 8001CBF0 -> 8001CBF4 type=21 target=sub_8001CBF4 line=`lw      $v1, 0x368($gp)`
- 8001CBF4 -> 8001CBF8 type=21 target=sub_8001CBF8 line=`move    $a3, $zero`
- 8001CBF8 -> 8001CBFC type=21 target=sub_8001CBFC line=`sw      $zero, 0x20+var_10($sp)`
- 8001CBFC -> 8001CC00 type=21 target=sub_8001CC00 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CC00 -> 8001CC04 type=21 target=sub_8001CC04 line=`sll     $v0, $v1, 2`
- 8001CC04 -> 8001CC08 type=21 target=sub_8001CC08 line=`addu    $v0, $v1`
- 8001CC08 -> 8001CC0C type=21 target=sub_8001CC0C line=`sll     $v0, 2`
- 8001CC0C -> 8001CC10 type=21 target=sub_8001CC10 line=`addu    $v0, $s3`
- 8001CC10 -> 8001CC14 type=21 target=sub_8001CC14 line=`jal     sub_8001B590`
- 8001CC10 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CC14 -> 8001CC18 type=21 target=sub_8001CC18 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CC18 -> 8001CC1C type=21 target=sub_8001CC1C line=`move    $a0, $s1`
- 8001CC1C -> 8001CC20 type=21 target=sub_8001CC20 line=`li      $a1, 0xB4`
- 8001CC20 -> 8001CC28 type=21 target=sub_8001CC28 line=`li      $a2, dword_8004E880`
- 8001CC28 -> 8001CC2C type=21 target=sub_8001CC2C line=`lw      $v1, 0x368($gp)`
- 8001CC2C -> 8001CC30 type=21 target=sub_8001CC30 line=`move    $a3, $zero`
- 8001CC30 -> 8001CC34 type=21 target=sub_8001CC34 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CC34 -> 8001CC38 type=21 target=sub_8001CC38 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CC38 -> 8001CC3C type=21 target=sub_8001CC3C line=`sll     $v0, $v1, 2`
- 8001CC3C -> 8001CC40 type=21 target=sub_8001CC40 line=`addu    $v0, $v1`
- 8001CC40 -> 8001CC44 type=21 target=sub_8001CC44 line=`sll     $v0, 2`
- 8001CC44 -> 8001CC48 type=21 target=sub_8001CC48 line=`addu    $v0, $s3`
- 8001CC48 -> 8001CC4C type=21 target=sub_8001CC4C line=`jal     sub_8001B590`
- 8001CC48 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CC4C -> 8001CC50 type=21 target=sub_8001CC50 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CC50 -> 8001CC54 type=21 target=sub_8001CC54 line=`move    $a0, $s0`
- 8001CC54 -> 8001CC58 type=21 target=sub_8001CC58 line=`li      $a1, 0xB4`
- 8001CC58 -> 8001CC60 type=21 target=sub_8001CC60 line=`li      $a2, dword_8004E890`
- 8001CC60 -> 8001CC64 type=21 target=sub_8001CC64 line=`lw      $v1, 0x368($gp)`
- 8001CC64 -> 8001CC68 type=21 target=sub_8001CC68 line=`move    $a3, $zero`
- 8001CC68 -> 8001CC6C type=21 target=sub_8001CC6C line=`sw      $zero, 0x20+var_10($sp)`
- 8001CC6C -> 8001CC70 type=21 target=sub_8001CC70 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CC70 -> 8001CC74 type=21 target=sub_8001CC74 line=`sll     $v0, $v1, 2`
- 8001CC74 -> 8001CC78 type=21 target=sub_8001CC78 line=`addu    $v0, $v1`
- 8001CC78 -> 8001CC7C type=21 target=sub_8001CC7C line=`sll     $v0, 2`
- 8001CC7C -> 8001CC80 type=21 target=sub_8001CC80 line=`addu    $v0, $s3`
- 8001CC80 -> 8001CC84 type=21 target=sub_8001CC84 line=`jal     sub_8001B590`
- 8001CC80 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CC84 -> 8001CC88 type=21 target=sub_8001CC88 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CC88 -> 8001CC8C type=21 target=sub_8001CC8C line=`lui     $v0, 0x28  # '('`
- 8001CC8C -> 8001CC90 type=21 target=sub_8001CC90 line=`addu    $fp, $v0`
- 8001CC90 -> 8001CC94 type=21 target=sub_8001CC94 line=`addu    $s7, $v0`
- 8001CC94 -> 8001CC98 type=21 target=sub_8001CC98 line=`addu    $s6, $v0`
- 8001CC98 -> 8001CC9C type=21 target=sub_8001CC9C line=`slti    $v0, $s4, 6`
- 8001CC9C -> 8001CCA0 type=21 target=sub_8001CCA0 line=`bnez    $v0, loc_8001CAB0`
- 8001CC9C -> 8001CAB0 type=19 target=loc_8001CAB0 line=`bnez    $v0, loc_8001CAB0`
- 8001CCA0 -> 8001CCA4 type=21 target=sub_8001CCA4 line=`move    $a0, $zero`
- 8001CCA4 -> 8001CCA8 type=21 target=sub_8001CCA8 line=`li      $a0, 0x28  # '('`
- 8001CCA8 -> 8001CCAC type=21 target=sub_8001CCAC line=`li      $a1, 0xA0`
- 8001CCAC -> 8001CCB4 type=21 target=sub_8001CCB4 line=`li      $a2, dword_8004E8E0`
- 8001CCB4 -> 8001CCB8 type=21 target=sub_8001CCB8 line=`move    $a3, $zero`
- 8001CCB8 -> 8001CCBC type=21 target=sub_8001CCBC line=`lw      $v1, 0x368($gp)`
- 8001CCBC -> 8001CCC4 type=21 target=sub_8001CCC4 line=`li      $s0, unk_80087288`
- 8001CCC4 -> 8001CCC8 type=21 target=sub_8001CCC8 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CCC8 -> 8001CCCC type=21 target=sub_8001CCCC line=`sw      $s2, 0x20+var_C($sp)`
- 8001CCCC -> 8001CCD0 type=21 target=sub_8001CCD0 line=`sll     $v0, $v1, 2`
- 8001CCD0 -> 8001CCD4 type=21 target=sub_8001CCD4 line=`addu    $v0, $v1`
- 8001CCD4 -> 8001CCD8 type=21 target=sub_8001CCD8 line=`sll     $v0, 2`
- 8001CCD8 -> 8001CCDC type=21 target=sub_8001CCDC line=`addu    $v0, $s0`
- 8001CCDC -> 8001CCE0 type=21 target=sub_8001CCE0 line=`jal     sub_8001B590`
- 8001CCDC -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CCE0 -> 8001CCE4 type=21 target=sub_8001CCE4 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CCE4 -> 8001CCE8 type=21 target=sub_8001CCE8 line=`li      $a0, 0xA0`
- 8001CCE8 -> 8001CCEC type=21 target=sub_8001CCEC line=`li      $a1, 0xA0`
- 8001CCEC -> 8001CCF4 type=21 target=sub_8001CCF4 line=`li      $a2, dword_8004E8F0`
- 8001CCF4 -> 8001CCF8 type=21 target=sub_8001CCF8 line=`lw      $v1, 0x368($gp)`
- 8001CCF8 -> 8001CCFC type=21 target=sub_8001CCFC line=`move    $a3, $zero`
- 8001CCFC -> 8001CD00 type=21 target=sub_8001CD00 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CD00 -> 8001CD04 type=21 target=sub_8001CD04 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CD04 -> 8001CD08 type=21 target=sub_8001CD08 line=`sll     $v0, $v1, 2`
- 8001CD08 -> 8001CD0C type=21 target=sub_8001CD0C line=`addu    $v0, $v1`
- 8001CD0C -> 8001CD10 type=21 target=sub_8001CD10 line=`sll     $v0, 2`
- 8001CD10 -> 8001CD14 type=21 target=sub_8001CD14 line=`addu    $v0, $s0`
- 8001CD14 -> 8001CD18 type=21 target=sub_8001CD18 line=`jal     sub_8001B590`
- 8001CD14 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CD18 -> 8001CD1C type=21 target=sub_8001CD1C line=`sw      $v0, 0x20+var_8($sp)`
- 8001CD1C -> 8001CD20 type=21 target=sub_8001CD20 line=`li      $a0, 0x28  # '('`
- 8001CD20 -> 8001CD24 type=21 target=sub_8001CD24 line=`li      $a1, 0x14`
- 8001CD24 -> 8001CD2C type=21 target=sub_8001CD2C line=`li      $a2, dword_8004E8C0`
- 8001CD2C -> 8001CD30 type=21 target=sub_8001CD30 line=`lw      $v1, 0x368($gp)`
- 8001CD30 -> 8001CD34 type=21 target=sub_8001CD34 line=`move    $a3, $zero`
- 8001CD34 -> 8001CD38 type=21 target=sub_8001CD38 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CD38 -> 8001CD3C type=21 target=sub_8001CD3C line=`sw      $s2, 0x20+var_C($sp)`
- 8001CD3C -> 8001CD40 type=21 target=sub_8001CD40 line=`sll     $v0, $v1, 2`
- 8001CD40 -> 8001CD44 type=21 target=sub_8001CD44 line=`addu    $v0, $v1`
- 8001CD44 -> 8001CD48 type=21 target=sub_8001CD48 line=`sll     $v0, 2`
- 8001CD48 -> 8001CD4C type=21 target=sub_8001CD4C line=`addu    $v0, $s0`
- 8001CD4C -> 8001CD50 type=21 target=sub_8001CD50 line=`jal     sub_8001B590`
- 8001CD4C -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CD50 -> 8001CD54 type=21 target=sub_8001CD54 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CD54 -> 8001CD58 type=21 target=sub_8001CD58 line=`li      $a0, 0xA0`
- 8001CD58 -> 8001CD5C type=21 target=sub_8001CD5C line=`li      $a1, 0x14`
- 8001CD5C -> 8001CD64 type=21 target=sub_8001CD64 line=`li      $a2, dword_8004E8D0`
- 8001CD64 -> 8001CD68 type=21 target=sub_8001CD68 line=`lw      $v1, 0x368($gp)`
- 8001CD68 -> 8001CD6C type=21 target=sub_8001CD6C line=`move    $a3, $zero`
- 8001CD6C -> 8001CD70 type=21 target=sub_8001CD70 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CD70 -> 8001CD74 type=21 target=sub_8001CD74 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CD74 -> 8001CD78 type=21 target=sub_8001CD78 line=`sll     $v0, $v1, 2`
- 8001CD78 -> 8001CD7C type=21 target=sub_8001CD7C line=`addu    $v0, $v1`
- 8001CD7C -> 8001CD80 type=21 target=sub_8001CD80 line=`sll     $v0, 2`
- 8001CD80 -> 8001CD84 type=21 target=sub_8001CD84 line=`addu    $v0, $s0`
- 8001CD84 -> 8001CD88 type=21 target=sub_8001CD88 line=`jal     sub_8001B590`
- 8001CD84 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CD88 -> 8001CD8C type=21 target=sub_8001CD8C line=`sw      $v0, 0x20+var_8($sp)`
- 8001CD8C -> 8001CD90 type=21 target=sub_8001CD90 line=`li      $a0, 0x14`
- 8001CD90 -> 8001CD94 type=21 target=sub_8001CD94 line=`li      $a1, 0x14`
- 8001CD94 -> 8001CD9C type=21 target=sub_8001CD9C line=`li      $a2, dword_8004E8A0`
- 8001CD9C -> 8001CDA0 type=21 target=sub_8001CDA0 line=`lw      $v1, 0x368($gp)`
- 8001CDA0 -> 8001CDA4 type=21 target=sub_8001CDA4 line=`move    $a3, $zero`
- 8001CDA4 -> 8001CDA8 type=21 target=sub_8001CDA8 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CDA8 -> 8001CDAC type=21 target=sub_8001CDAC line=`sw      $s2, 0x20+var_C($sp)`
- 8001CDAC -> 8001CDB0 type=21 target=sub_8001CDB0 line=`sll     $v0, $v1, 2`
- 8001CDB0 -> 8001CDB4 type=21 target=sub_8001CDB4 line=`addu    $v0, $v1`
- 8001CDB4 -> 8001CDB8 type=21 target=sub_8001CDB8 line=`sll     $v0, 2`
- 8001CDB8 -> 8001CDBC type=21 target=sub_8001CDBC line=`addu    $v0, $s0`
- 8001CDBC -> 8001CDC0 type=21 target=sub_8001CDC0 line=`jal     sub_8001B590`
- 8001CDBC -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CDC0 -> 8001CDC4 type=21 target=sub_8001CDC4 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CDC4 -> 8001CDC8 type=21 target=sub_8001CDC8 line=`li      $a0, 0x118`
- 8001CDC8 -> 8001CDCC type=21 target=sub_8001CDCC line=`li      $a1, 0x14`
- 8001CDCC -> 8001CDD4 type=21 target=sub_8001CDD4 line=`li      $a2, dword_8004E8B0`
- 8001CDD4 -> 8001CDD8 type=21 target=sub_8001CDD8 line=`lw      $v1, 0x368($gp)`
- 8001CDD8 -> 8001CDDC type=21 target=sub_8001CDDC line=`move    $a3, $zero`
- 8001CDDC -> 8001CDE0 type=21 target=sub_8001CDE0 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CDE0 -> 8001CDE4 type=21 target=sub_8001CDE4 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CDE4 -> 8001CDE8 type=21 target=sub_8001CDE8 line=`sll     $v0, $v1, 2`
- 8001CDE8 -> 8001CDEC type=21 target=sub_8001CDEC line=`addu    $v0, $v1`
- 8001CDEC -> 8001CDF0 type=21 target=sub_8001CDF0 line=`sll     $v0, 2`
- 8001CDF0 -> 8001CDF4 type=21 target=sub_8001CDF4 line=`addu    $v0, $s0`
- 8001CDF4 -> 8001CDF8 type=21 target=sub_8001CDF8 line=`jal     sub_8001B590`
- 8001CDF4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CDF8 -> 8001CDFC type=21 target=sub_8001CDFC line=`sw      $v0, 0x20+var_8($sp)`
- 8001CDFC -> 8001CE00 type=21 target=sub_8001CE00 line=`lw      $ra, 0x20+var_s24($sp)`
- 8001CE00 -> 8001CE04 type=21 target=sub_8001CE04 line=`lw      $fp, 0x20+var_s20($sp)`
- 8001CE04 -> 8001CE08 type=21 target=sub_8001CE08 line=`lw      $s7, 0x20+var_s1C($sp)`
- 8001CE08 -> 8001CE0C type=21 target=sub_8001CE0C line=`lw      $s6, 0x20+var_s18($sp)`
- 8001CE0C -> 8001CE10 type=21 target=sub_8001CE10 line=`lw      $s5, 0x20+var_s14($sp)`
- 8001CE10 -> 8001CE14 type=21 target=sub_8001CE14 line=`lw      $s4, 0x20+var_s10($sp)`
- 8001CE14 -> 8001CE18 type=21 target=sub_8001CE18 line=`lw      $s3, 0x20+var_sC($sp)`
- 8001CE18 -> 8001CE1C type=21 target=sub_8001CE1C line=`lw      $s2, 0x20+var_s8($sp)`
- 8001CE1C -> 8001CE20 type=21 target=sub_8001CE20 line=`lw      $s1, 0x20+var_s4($sp)`
- 8001CE20 -> 8001CE24 type=21 target=sub_8001CE24 line=`lw      $s0, 0x20+var_s0($sp)`
- 8001CE24 -> 8001CE28 type=21 target=sub_8001CE28 line=`addiu   $sp, 0x48`
- 8001CE28 -> 8001CE2C type=21 target=sub_8001CE2C line=`jr      $ra`

### Full disasm
```asm
8001C864: addiu   $sp, -0x48                                               ; B8 FF BD 27
8001C868: sw      $s2, 0x20+var_s8($sp)                                    ; 28 00 B2 AF
8001C86C: move    $s2, $a0                                                 ; 21 90 80 00
8001C870: li      $a0, 0x118                                               ; 18 01 04 34
8001C874: li      $a1, 0xC8                                                ; C8 00 05 34
8001C878: li      $a2, dword_8004E940                                      ; 05 80 06 3C 40 E9 C6 24
8001C880: move    $a3, $zero                                               ; 21 38 00 00
8001C884: sw      $s4, 0x20+var_s10($sp)                                   ; 30 00 B4 AF
8001C888: move    $s4, $zero                                               ; 21 A0 00 00
8001C88C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C890: li      $v0, unk_80087288                                        ; 08 80 02 3C 88 72 42 24
8001C898: sw      $s3, 0x20+var_sC($sp)                                    ; 2C 00 B3 AF
8001C89C: move    $s3, $v0                                                 ; 21 98 40 00
8001C8A0: sw      $s6, 0x20+var_s18($sp)                                   ; 38 00 B6 AF
8001C8A4: lui     $s6, 0x14                                                ; 14 00 16 3C
8001C8A8: sw      $s5, 0x20+var_s14($sp)                                   ; 34 00 B5 AF
8001C8AC: lui     $s5, 0x28  # '('                                         ; 28 00 15 3C
8001C8B0: sw      $ra, 0x20+var_s24($sp)                                   ; 44 00 BF AF
8001C8B4: sw      $fp, 0x20+var_s20($sp)                                   ; 40 00 BE AF
8001C8B8: sw      $s7, 0x20+var_s1C($sp)                                   ; 3C 00 B7 AF
8001C8BC: sw      $s1, 0x20+var_s4($sp)                                    ; 24 00 B1 AF
8001C8C0: sw      $s0, 0x20+var_s0($sp)                                    ; 20 00 B0 AF
8001C8C4: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C8C8: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C8CC: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C8D0: addu    $v0, $v1                                                 ; 21 10 43 00
8001C8D4: sll     $v0, 2                                                   ; 80 10 02 00
8001C8D8: addu    $v0, $s3                                                 ; 21 10 53 00
8001C8DC: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C8E0: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C8E4: li      $a0, 0x14                                                ; 14 00 04 34
8001C8E8: li      $a1, 0xC8                                                ; C8 00 05 34
8001C8EC: li      $a2, dword_8004E950                                      ; 05 80 06 3C 50 E9 C6 24
8001C8F4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C8F8: move    $a3, $zero                                               ; 21 38 00 00
8001C8FC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C900: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C904: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C908: addu    $v0, $v1                                                 ; 21 10 43 00
8001C90C: sll     $v0, 2                                                   ; 80 10 02 00
8001C910: addu    $v0, $s3                                                 ; 21 10 53 00
8001C914: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C918: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C91C: li      $a0, 0x118                                               ; 18 01 04 34
8001C920: li      $a1, 0xB4                                                ; B4 00 05 34
8001C924: li      $a2, dword_8004E960                                      ; 05 80 06 3C 60 E9 C6 24
8001C92C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C930: move    $a3, $zero                                               ; 21 38 00 00
8001C934: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C938: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C93C: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C940: addu    $v0, $v1                                                 ; 21 10 43 00
8001C944: sll     $v0, 2                                                   ; 80 10 02 00
8001C948: addu    $v0, $s3                                                 ; 21 10 53 00
8001C94C: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C950: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C954: li      $a0, 0x14                                                ; 14 00 04 34
8001C958: li      $a1, 0xB4                                                ; B4 00 05 34
8001C95C: li      $a2, dword_8004E970                                      ; 05 80 06 3C 70 E9 C6 24
8001C964: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C968: move    $a3, $zero                                               ; 21 38 00 00
8001C96C: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C970: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C974: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C978: addu    $v0, $v1                                                 ; 21 10 43 00
8001C97C: sll     $v0, 2                                                   ; 80 10 02 00
8001C980: addu    $v0, $s3                                                 ; 21 10 53 00
8001C984: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C988: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C98C: sra     $s1, $s5, 16                                             ; 03 8C 15 00
8001C990: move    $a0, $s1                                                 ; 21 20 20 02
8001C994: move    $a1, $zero                                               ; 21 28 00 00
8001C998: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001C9A0: move    $a3, $zero                                               ; 21 38 00 00
8001C9A4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C9A8: addiu   $s4, 1                                                   ; 01 00 94 26
8001C9AC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C9B0: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C9B4: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C9B8: addu    $v0, $v1                                                 ; 21 10 43 00
8001C9BC: sll     $v0, 2                                                   ; 80 10 02 00
8001C9C0: addu    $v0, $s3                                                 ; 21 10 53 00
8001C9C4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C9C8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C9CC: sra     $s0, $s6, 16                                             ; 03 84 16 00
8001C9D0: move    $a0, $s0                                                 ; 21 20 00 02
8001C9D4: move    $a1, $zero                                               ; 21 28 00 00
8001C9D8: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001C9E0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C9E4: move    $a3, $zero                                               ; 21 38 00 00
8001C9E8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C9EC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C9F0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C9F4: addu    $v0, $v1                                                 ; 21 10 43 00
8001C9F8: sll     $v0, 2                                                   ; 80 10 02 00
8001C9FC: addu    $v0, $s3                                                 ; 21 10 53 00
8001CA00: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CA04: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CA08: move    $a0, $s1                                                 ; 21 20 20 02
8001CA0C: li      $a1, 0xDC                                                ; DC 00 05 34
8001CA10: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001CA18: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CA1C: move    $a3, $zero                                               ; 21 38 00 00
8001CA20: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CA24: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CA28: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CA2C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CA30: sll     $v0, 2                                                   ; 80 10 02 00
8001CA34: addu    $v0, $s3                                                 ; 21 10 53 00
8001CA38: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CA3C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CA40: move    $a0, $s0                                                 ; 21 20 00 02
8001CA44: li      $a1, 0xDC                                                ; DC 00 05 34
8001CA48: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001CA50: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CA54: move    $a3, $zero                                               ; 21 38 00 00
8001CA58: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CA5C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CA60: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CA64: addu    $v0, $v1                                                 ; 21 10 43 00
8001CA68: sll     $v0, 2                                                   ; 80 10 02 00
8001CA6C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CA70: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CA74: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CA78: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001CA7C: addu    $s6, $v0                                                 ; 21 B0 C2 02
8001CA80: addu    $s5, $v0                                                 ; 21 A8 A2 02
8001CA84: slti    $v0, $s4, 7                                              ; 07 00 82 2A
8001CA88: bnez    $v0, loc_8001C990                                        ; C1 FF 40 14
8001CA8C: sra     $s1, $s5, 16                                             ; 03 8C 15 00
8001CA90: move    $s4, $zero                                               ; 21 A0 00 00
8001CA94: li      $s3, unk_80087288                                        ; 08 80 13 3C 88 72 73 26
8001CA9C: lui     $fp, 0x3C  # '<'                                         ; 3C 00 1E 3C
8001CAA0: lui     $s7, 0x28  # '('                                         ; 28 00 17 3C
8001CAA4: lui     $s6, 0x14                                                ; 14 00 16 3C
8001CAA8: move    $s5, $zero                                               ; 21 A8 00 00
8001CAAC: move    $a0, $zero                                               ; 21 20 00 00
8001CAB0: sra     $s1, $s6, 16                                             ; 03 8C 16 00
8001CAB4: move    $a1, $s1                                                 ; 21 28 20 02
8001CAB8: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001CAC0: move    $a3, $zero                                               ; 21 38 00 00
8001CAC4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CAC8: addiu   $s4, 1                                                   ; 01 00 94 26
8001CACC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CAD0: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CAD4: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CAD8: addu    $v0, $v1                                                 ; 21 10 43 00
8001CADC: sll     $v0, 2                                                   ; 80 10 02 00
8001CAE0: addu    $v0, $s3                                                 ; 21 10 53 00
8001CAE4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CAE8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CAEC: move    $a0, $zero                                               ; 21 20 00 00
8001CAF0: sll     $s0, $s5, 19                                             ; C0 84 15 00
8001CAF4: sra     $s0, 16                                                  ; 03 84 10 00
8001CAF8: move    $a1, $s0                                                 ; 21 28 00 02
8001CAFC: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001CB04: move    $a3, $zero                                               ; 21 38 00 00
8001CB08: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CB0C: addiu   $s5, 5                                                   ; 05 00 B5 26
8001CB10: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CB14: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CB18: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CB1C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CB20: sll     $v0, 2                                                   ; 80 10 02 00
8001CB24: addu    $v0, $s3                                                 ; 21 10 53 00
8001CB28: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CB2C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CB30: li      $a0, 0x12C                                               ; 2C 01 04 34
8001CB34: move    $a1, $s1                                                 ; 21 28 20 02
8001CB38: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001CB40: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CB44: move    $a3, $zero                                               ; 21 38 00 00
8001CB48: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CB4C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CB50: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CB54: addu    $v0, $v1                                                 ; 21 10 43 00
8001CB58: sll     $v0, 2                                                   ; 80 10 02 00
8001CB5C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CB60: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CB64: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CB68: li      $a0, 0x12C                                               ; 2C 01 04 34
8001CB6C: move    $a1, $s0                                                 ; 21 28 00 02
8001CB70: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001CB78: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CB7C: move    $a3, $zero                                               ; 21 38 00 00
8001CB80: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CB84: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CB88: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CB8C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CB90: sll     $v0, 2                                                   ; 80 10 02 00
8001CB94: addu    $v0, $s3                                                 ; 21 10 53 00
8001CB98: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CB9C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CBA0: sra     $s1, $s7, 16                                             ; 03 8C 17 00
8001CBA4: move    $a0, $s1                                                 ; 21 20 20 02
8001CBA8: li      $a1, 0xC8                                                ; C8 00 05 34
8001CBAC: li      $a2, dword_8004E860                                      ; 05 80 06 3C 60 E8 C6 24
8001CBB4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CBB8: move    $a3, $zero                                               ; 21 38 00 00
8001CBBC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CBC0: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CBC4: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CBC8: addu    $v0, $v1                                                 ; 21 10 43 00
8001CBCC: sll     $v0, 2                                                   ; 80 10 02 00
8001CBD0: addu    $v0, $s3                                                 ; 21 10 53 00
8001CBD4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CBD8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CBDC: sra     $s0, $fp, 16                                             ; 03 84 1E 00
8001CBE0: move    $a0, $s0                                                 ; 21 20 00 02
8001CBE4: li      $a1, 0xC8                                                ; C8 00 05 34
8001CBE8: li      $a2, dword_8004E870                                      ; 05 80 06 3C 70 E8 C6 24
8001CBF0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CBF4: move    $a3, $zero                                               ; 21 38 00 00
8001CBF8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CBFC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CC00: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CC04: addu    $v0, $v1                                                 ; 21 10 43 00
8001CC08: sll     $v0, 2                                                   ; 80 10 02 00
8001CC0C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CC10: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CC14: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CC18: move    $a0, $s1                                                 ; 21 20 20 02
8001CC1C: li      $a1, 0xB4                                                ; B4 00 05 34
8001CC20: li      $a2, dword_8004E880                                      ; 05 80 06 3C 80 E8 C6 24
8001CC28: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CC2C: move    $a3, $zero                                               ; 21 38 00 00
8001CC30: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CC34: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CC38: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CC3C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CC40: sll     $v0, 2                                                   ; 80 10 02 00
8001CC44: addu    $v0, $s3                                                 ; 21 10 53 00
8001CC48: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CC4C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CC50: move    $a0, $s0                                                 ; 21 20 00 02
8001CC54: li      $a1, 0xB4                                                ; B4 00 05 34
8001CC58: li      $a2, dword_8004E890                                      ; 05 80 06 3C 90 E8 C6 24
8001CC60: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CC64: move    $a3, $zero                                               ; 21 38 00 00
8001CC68: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CC6C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CC70: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CC74: addu    $v0, $v1                                                 ; 21 10 43 00
8001CC78: sll     $v0, 2                                                   ; 80 10 02 00
8001CC7C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CC80: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CC84: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CC88: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001CC8C: addu    $fp, $v0                                                 ; 21 F0 C2 03
8001CC90: addu    $s7, $v0                                                 ; 21 B8 E2 02
8001CC94: addu    $s6, $v0                                                 ; 21 B0 C2 02
8001CC98: slti    $v0, $s4, 6                                              ; 06 00 82 2A
8001CC9C: bnez    $v0, loc_8001CAB0                                        ; 84 FF 40 14
8001CCA0: move    $a0, $zero                                               ; 21 20 00 00
8001CCA4: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001CCA8: li      $a1, 0xA0                                                ; A0 00 05 34
8001CCAC: li      $a2, dword_8004E8E0                                      ; 05 80 06 3C E0 E8 C6 24
8001CCB4: move    $a3, $zero                                               ; 21 38 00 00
8001CCB8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CCBC: li      $s0, unk_80087288                                        ; 08 80 10 3C 88 72 10 26
8001CCC4: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CCC8: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CCCC: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CCD0: addu    $v0, $v1                                                 ; 21 10 43 00
8001CCD4: sll     $v0, 2                                                   ; 80 10 02 00
8001CCD8: addu    $v0, $s0                                                 ; 21 10 50 00
8001CCDC: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CCE0: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CCE4: li      $a0, 0xA0                                                ; A0 00 04 34
8001CCE8: li      $a1, 0xA0                                                ; A0 00 05 34
8001CCEC: li      $a2, dword_8004E8F0                                      ; 05 80 06 3C F0 E8 C6 24
8001CCF4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CCF8: move    $a3, $zero                                               ; 21 38 00 00
8001CCFC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CD00: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CD04: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CD08: addu    $v0, $v1                                                 ; 21 10 43 00
8001CD0C: sll     $v0, 2                                                   ; 80 10 02 00
8001CD10: addu    $v0, $s0                                                 ; 21 10 50 00
8001CD14: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CD18: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CD1C: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001CD20: li      $a1, 0x14                                                ; 14 00 05 34
8001CD24: li      $a2, dword_8004E8C0                                      ; 05 80 06 3C C0 E8 C6 24
8001CD2C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CD30: move    $a3, $zero                                               ; 21 38 00 00
8001CD34: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CD38: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CD3C: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CD40: addu    $v0, $v1                                                 ; 21 10 43 00
8001CD44: sll     $v0, 2                                                   ; 80 10 02 00
8001CD48: addu    $v0, $s0                                                 ; 21 10 50 00
8001CD4C: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CD50: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CD54: li      $a0, 0xA0                                                ; A0 00 04 34
8001CD58: li      $a1, 0x14                                                ; 14 00 05 34
8001CD5C: li      $a2, dword_8004E8D0                                      ; 05 80 06 3C D0 E8 C6 24
8001CD64: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CD68: move    $a3, $zero                                               ; 21 38 00 00
8001CD6C: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CD70: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CD74: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CD78: addu    $v0, $v1                                                 ; 21 10 43 00
8001CD7C: sll     $v0, 2                                                   ; 80 10 02 00
8001CD80: addu    $v0, $s0                                                 ; 21 10 50 00
8001CD84: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CD88: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CD8C: li      $a0, 0x14                                                ; 14 00 04 34
8001CD90: li      $a1, 0x14                                                ; 14 00 05 34
8001CD94: li      $a2, dword_8004E8A0                                      ; 05 80 06 3C A0 E8 C6 24
8001CD9C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CDA0: move    $a3, $zero                                               ; 21 38 00 00
8001CDA4: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CDA8: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CDAC: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CDB0: addu    $v0, $v1                                                 ; 21 10 43 00
8001CDB4: sll     $v0, 2                                                   ; 80 10 02 00
8001CDB8: addu    $v0, $s0                                                 ; 21 10 50 00
8001CDBC: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CDC0: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CDC4: li      $a0, 0x118                                               ; 18 01 04 34
8001CDC8: li      $a1, 0x14                                                ; 14 00 05 34
8001CDCC: li      $a2, dword_8004E8B0                                      ; 05 80 06 3C B0 E8 C6 24
8001CDD4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CDD8: move    $a3, $zero                                               ; 21 38 00 00
8001CDDC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CDE0: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CDE4: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CDE8: addu    $v0, $v1                                                 ; 21 10 43 00
8001CDEC: sll     $v0, 2                                                   ; 80 10 02 00
8001CDF0: addu    $v0, $s0                                                 ; 21 10 50 00
8001CDF4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CDF8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CDFC: lw      $ra, 0x20+var_s24($sp)                                   ; 44 00 BF 8F
8001CE00: lw      $fp, 0x20+var_s20($sp)                                   ; 40 00 BE 8F
8001CE04: lw      $s7, 0x20+var_s1C($sp)                                   ; 3C 00 B7 8F
8001CE08: lw      $s6, 0x20+var_s18($sp)                                   ; 38 00 B6 8F
8001CE0C: lw      $s5, 0x20+var_s14($sp)                                   ; 34 00 B5 8F
8001CE10: lw      $s4, 0x20+var_s10($sp)                                   ; 30 00 B4 8F
8001CE14: lw      $s3, 0x20+var_sC($sp)                                    ; 2C 00 B3 8F
8001CE18: lw      $s2, 0x20+var_s8($sp)                                    ; 28 00 B2 8F
8001CE1C: lw      $s1, 0x20+var_s4($sp)                                    ; 24 00 B1 8F
8001CE20: lw      $s0, 0x20+var_s0($sp)                                    ; 20 00 B0 8F
8001CE24: addiu   $sp, 0x48                                                ; 48 00 BD 27
8001CE28: jr      $ra                                                      ; 08 00 E0 03
8001CE2C: nop                                                              ; 00 00 00 00
```

## `8001CE30` `sub_8001CE30`
Bounds: `8001CE30-8001D74C`

### Pseudocode
```c
int __fastcall sub_8001CE30(int a1)
{
  int v1; // $gp
  int v3; // $s4
  int v4; // $s6
  int v5; // $s5
  int v6; // $s1
  int v7; // $gp
  int v8; // $gp
  int v9; // $gp
  int v10; // $gp
  int v11; // $gp
  int v12; // $gp
  int v13; // $gp
  int v14; // $s4
  int v15; // $s5
  int v16; // $s3
  int v17; // $gp
  int v18; // $gp
  int v19; // $gp
  int v20; // $gp
  int v21; // $gp
  int v22; // $gp
  int v23; // $gp
  int v24; // $gp
  int v25; // $gp
  int v26; // $gp
  int v27; // $gp
  int v28; // $gp
  int v29; // $gp
  int v30; // $gp
  int v31; // $gp
  int v32; // $gp
  int v33; // $gp
  int v34; // $gp
  int v35; // $gp
  int v36; // $gp
  int v37; // $gp
  int v38; // $gp
  int v39; // $gp
  int v40; // $gp
  int v41; // $gp
  int v42; // $gp
  int v43; // $gp
  int v44; // $gp

  v3 = 0;
  v4 = 1310720;
  v5 = 2621440;
  v6 = 40;
  do
  {
    ++v3;
    sub_8001B590(v6, 0, dword_8004E900, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v1 + 872));
    sub_8001B590(v4 >> 16, 0, dword_8004E910, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v7 + 872));
    sub_8001B590(v6, 220, dword_8004E920, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v8 + 872));
    sub_8001B590(v4 >> 16, 220, dword_8004E930, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
    sub_8001B590(v6, 20, dword_8004E920, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v10 + 872));
    sub_8001B590(v4 >> 16, 20, dword_8004E930, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v11 + 872));
    sub_8001B590(v6, 200, dword_8004E900, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v12 + 872));
    sub_8001B590(v4 >> 16, 200, dword_8004E910, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v13 + 872));
    v4 += 2621440;
    v5 += 2621440;
    v6 = v5 >> 16;
  }
  while ( v3 < 7 );
  v14 = 0;
  v15 = 0;
  v16 = 1310720;
  do
  {
    ++v14;
    sub_8001B590(0, v16 >> 16, dword_8004E920, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v1 + 872));
    sub_8001B590(0, v15 >> 16, dword_8004E900, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v17 + 872));
    sub_8001B590(300, v16 >> 16, dword_8004E930, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v18 + 872));
    sub_8001B590(300, v15 >> 16, dword_8004E910, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v19 + 872));
    v15 += 2621440;
    v16 += 2621440;
  }
  while ( v14 < 6 );
  sub_8001B590(40, 180, dword_8004EA60, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v1 + 872));
  sub_8001B590(60, 180, dword_8004EA70, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v20 + 872));
  sub_8001B590(80, 180, dword_8004EA80, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v21 + 872));
  sub_8001B590(100, 180, dword_8004EA90, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v22 + 872));
  sub_8001B590(120, 180, dword_8004EAA0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v23 + 872));
  sub_8001B590(140, 180, dword_8004EAB0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v24 + 872));
  sub_8001B590(160, 180, dword_8004EAC0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v25 + 872));
  sub_8001B590(180, 180, dword_8004EAD0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v26 + 872));
  sub_8001B590(200, 180, dword_8004EAE0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v27 + 872));
  sub_8001B590(220, 180, dword_8004EAF0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v28 + 872));
  sub_8001B590(240, 180, dword_8004EB00, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v29 + 872));
  sub_8001B590(260, 180, dword_8004EB10, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v30 + 872));
  sub_8001B590(280, 40, dword_8004E980, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v31 + 872));
  sub_8001B590(20, 40, dword_8004EA50, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v32 + 872));
  sub_8001B590(160, 40, dword_8004E9F0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v33 + 872));
  sub_8001B590(140, 40, dword_8004E9E0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v34 + 872));
  sub_8001B590(180, 40, dword_8004EA00, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v35 + 872));
  sub_8001B590(120, 40, dword_8004E9D0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v36 + 872));
  sub_8001B590(200, 40, dword_8004EA10, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v37 + 872));
  sub_8001B590(100, 40, dword_8004E9C0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v38 + 872));
  sub_8001B590(240, 40, dword_8004EA30, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v39 + 872));
  sub_8001B590(80, 40, dword_8004E9B0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v40 + 872));
  sub_8001B590(220, 40, dword_8004EA20, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v41 + 872));
  sub_8001B590(60, 40, dword_8004E9A0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v42 + 872));
  sub_8001B590(260, 40, dword_8004EA40, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v43 + 872));
  return sub_8001B590(40, 40, dword_8004E990, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v44 + 872));
}

```

### Calls in function
- 8001CEA4: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CEE0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CF18: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CF50: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CF88: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CFC0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CFF8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D030: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D048: `bnez    $v0, loc_8001CE70` -> 8001CE70 loc_8001CE70
- 8001D09C: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D0D8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D110: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D148: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D160: `bnez    $v0, loc_8001D068` -> 8001D068 loc_8001D068
- 8001D1A0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D1D8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D210: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D248: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D280: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D2B8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D2F0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D328: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D360: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D398: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D3D0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D408: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D440: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D478: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D4B0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D4E8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D520: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D558: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D590: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D5C8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D600: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D638: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D670: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D6A8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D6E0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D718: `jal     sub_8001B590` -> 8001B590 sub_8001B590

### Xrefs to
- 8001ED18 -> 8001CE30 type=17 func=sub_8001EC54 line=`jal     sub_8001CE30`
- 80020270 -> 8001CE30 type=17 func=sub_80020248 line=`jal     sub_8001CE30`

### Xrefs from
- 8001CE30 -> 8001CE34 type=21 target=sub_8001CE34 line=`addiu   $sp, -0x40`
- 8001CE34 -> 8001CE38 type=21 target=sub_8001CE38 line=`sw      $s2, 0x20+var_s8($sp)`
- 8001CE38 -> 8001CE3C type=21 target=sub_8001CE3C line=`move    $s2, $a0`
- 8001CE3C -> 8001CE40 type=21 target=sub_8001CE40 line=`sw      $s4, 0x20+var_s10($sp)`
- 8001CE40 -> 8001CE44 type=21 target=sub_8001CE44 line=`move    $s4, $zero`
- 8001CE44 -> 8001CE48 type=21 target=sub_8001CE48 line=`sw      $s3, 0x20+var_sC($sp)`
- 8001CE48 -> 8001CE50 type=21 target=sub_8001CE50 line=`li      $s3, unk_80087288`
- 8001CE50 -> 8001CE54 type=21 target=sub_8001CE54 line=`sw      $s6, 0x20+var_s18($sp)`
- 8001CE54 -> 8001CE58 type=21 target=sub_8001CE58 line=`lui     $s6, 0x14`
- 8001CE58 -> 8001CE5C type=21 target=sub_8001CE5C line=`sw      $s5, 0x20+var_s14($sp)`
- 8001CE5C -> 8001CE60 type=21 target=sub_8001CE60 line=`lui     $s5, 0x28  # '('`
- 8001CE60 -> 8001CE64 type=21 target=sub_8001CE64 line=`sw      $ra, 0x20+var_s1C($sp)`
- 8001CE64 -> 8001CE68 type=21 target=sub_8001CE68 line=`sw      $s1, 0x20+var_s4($sp)`
- 8001CE68 -> 8001CE6C type=21 target=sub_8001CE6C line=`sw      $s0, 0x20+var_s0($sp)`
- 8001CE6C -> 8001CE70 type=21 target=loc_8001CE70 line=`sra     $s1, $s5, 16`
- 8001CE70 -> 8001CE74 type=21 target=sub_8001CE74 line=`move    $a0, $s1`
- 8001CE74 -> 8001CE78 type=21 target=sub_8001CE78 line=`move    $a1, $zero`
- 8001CE78 -> 8001CE80 type=21 target=sub_8001CE80 line=`li      $a2, dword_8004E900`
- 8001CE80 -> 8001CE84 type=21 target=sub_8001CE84 line=`move    $a3, $zero`
- 8001CE84 -> 8001CE88 type=21 target=sub_8001CE88 line=`lw      $v1, 0x368($gp)`
- 8001CE88 -> 8001CE8C type=21 target=sub_8001CE8C line=`addiu   $s4, 1`
- 8001CE8C -> 8001CE90 type=21 target=sub_8001CE90 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CE90 -> 8001CE94 type=21 target=sub_8001CE94 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CE94 -> 8001CE98 type=21 target=sub_8001CE98 line=`sll     $v0, $v1, 2`
- 8001CE98 -> 8001CE9C type=21 target=sub_8001CE9C line=`addu    $v0, $v1`
- 8001CE9C -> 8001CEA0 type=21 target=sub_8001CEA0 line=`sll     $v0, 2`
- 8001CEA0 -> 8001CEA4 type=21 target=sub_8001CEA4 line=`addu    $v0, $s3`
- 8001CEA4 -> 8001CEA8 type=21 target=sub_8001CEA8 line=`jal     sub_8001B590`
- 8001CEA4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CEA8 -> 8001CEAC type=21 target=sub_8001CEAC line=`sw      $v0, 0x20+var_8($sp)`
- 8001CEAC -> 8001CEB0 type=21 target=sub_8001CEB0 line=`sra     $s0, $s6, 16`
- 8001CEB0 -> 8001CEB4 type=21 target=sub_8001CEB4 line=`move    $a0, $s0`
- 8001CEB4 -> 8001CEB8 type=21 target=sub_8001CEB8 line=`move    $a1, $zero`
- 8001CEB8 -> 8001CEC0 type=21 target=sub_8001CEC0 line=`li      $a2, dword_8004E910`
- 8001CEC0 -> 8001CEC4 type=21 target=sub_8001CEC4 line=`lw      $v1, 0x368($gp)`
- 8001CEC4 -> 8001CEC8 type=21 target=sub_8001CEC8 line=`move    $a3, $zero`
- 8001CEC8 -> 8001CECC type=21 target=sub_8001CECC line=`sw      $zero, 0x20+var_10($sp)`
- 8001CECC -> 8001CED0 type=21 target=sub_8001CED0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CED0 -> 8001CED4 type=21 target=sub_8001CED4 line=`sll     $v0, $v1, 2`
- 8001CED4 -> 8001CED8 type=21 target=sub_8001CED8 line=`addu    $v0, $v1`
- 8001CED8 -> 8001CEDC type=21 target=sub_8001CEDC line=`sll     $v0, 2`
- 8001CEDC -> 8001CEE0 type=21 target=sub_8001CEE0 line=`addu    $v0, $s3`
- 8001CEE0 -> 8001CEE4 type=21 target=sub_8001CEE4 line=`jal     sub_8001B590`
- 8001CEE0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CEE4 -> 8001CEE8 type=21 target=sub_8001CEE8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CEE8 -> 8001CEEC type=21 target=sub_8001CEEC line=`move    $a0, $s1`
- 8001CEEC -> 8001CEF0 type=21 target=sub_8001CEF0 line=`li      $a1, 0xDC`
- 8001CEF0 -> 8001CEF8 type=21 target=sub_8001CEF8 line=`li      $a2, dword_8004E920`
- 8001CEF8 -> 8001CEFC type=21 target=sub_8001CEFC line=`lw      $v1, 0x368($gp)`
- 8001CEFC -> 8001CF00 type=21 target=sub_8001CF00 line=`move    $a3, $zero`
- 8001CF00 -> 8001CF04 type=21 target=sub_8001CF04 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CF04 -> 8001CF08 type=21 target=sub_8001CF08 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CF08 -> 8001CF0C type=21 target=sub_8001CF0C line=`sll     $v0, $v1, 2`
- 8001CF0C -> 8001CF10 type=21 target=sub_8001CF10 line=`addu    $v0, $v1`
- 8001CF10 -> 8001CF14 type=21 target=sub_8001CF14 line=`sll     $v0, 2`
- 8001CF14 -> 8001CF18 type=21 target=sub_8001CF18 line=`addu    $v0, $s3`
- 8001CF18 -> 8001CF1C type=21 target=sub_8001CF1C line=`jal     sub_8001B590`
- 8001CF18 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CF1C -> 8001CF20 type=21 target=sub_8001CF20 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CF20 -> 8001CF24 type=21 target=sub_8001CF24 line=`move    $a0, $s0`
- 8001CF24 -> 8001CF28 type=21 target=sub_8001CF28 line=`li      $a1, 0xDC`
- 8001CF28 -> 8001CF30 type=21 target=sub_8001CF30 line=`li      $a2, dword_8004E930`
- 8001CF30 -> 8001CF34 type=21 target=sub_8001CF34 line=`lw      $v1, 0x368($gp)`
- 8001CF34 -> 8001CF38 type=21 target=sub_8001CF38 line=`move    $a3, $zero`
- 8001CF38 -> 8001CF3C type=21 target=sub_8001CF3C line=`sw      $zero, 0x20+var_10($sp)`
- 8001CF3C -> 8001CF40 type=21 target=sub_8001CF40 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CF40 -> 8001CF44 type=21 target=sub_8001CF44 line=`sll     $v0, $v1, 2`
- 8001CF44 -> 8001CF48 type=21 target=sub_8001CF48 line=`addu    $v0, $v1`
- 8001CF48 -> 8001CF4C type=21 target=sub_8001CF4C line=`sll     $v0, 2`
- 8001CF4C -> 8001CF50 type=21 target=sub_8001CF50 line=`addu    $v0, $s3`
- 8001CF50 -> 8001CF54 type=21 target=sub_8001CF54 line=`jal     sub_8001B590`
- 8001CF50 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CF54 -> 8001CF58 type=21 target=sub_8001CF58 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CF58 -> 8001CF5C type=21 target=sub_8001CF5C line=`move    $a0, $s1`
- 8001CF5C -> 8001CF60 type=21 target=sub_8001CF60 line=`li      $a1, 0x14`
- 8001CF60 -> 8001CF68 type=21 target=sub_8001CF68 line=`li      $a2, dword_8004E920`
- 8001CF68 -> 8001CF6C type=21 target=sub_8001CF6C line=`lw      $v1, 0x368($gp)`
- 8001CF6C -> 8001CF70 type=21 target=sub_8001CF70 line=`move    $a3, $zero`
- 8001CF70 -> 8001CF74 type=21 target=sub_8001CF74 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CF74 -> 8001CF78 type=21 target=sub_8001CF78 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CF78 -> 8001CF7C type=21 target=sub_8001CF7C line=`sll     $v0, $v1, 2`
- 8001CF7C -> 8001CF80 type=21 target=sub_8001CF80 line=`addu    $v0, $v1`
- 8001CF80 -> 8001CF84 type=21 target=sub_8001CF84 line=`sll     $v0, 2`
- 8001CF84 -> 8001CF88 type=21 target=sub_8001CF88 line=`addu    $v0, $s3`
- 8001CF88 -> 8001CF8C type=21 target=sub_8001CF8C line=`jal     sub_8001B590`
- 8001CF88 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CF8C -> 8001CF90 type=21 target=sub_8001CF90 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CF90 -> 8001CF94 type=21 target=sub_8001CF94 line=`move    $a0, $s0`
- 8001CF94 -> 8001CF98 type=21 target=sub_8001CF98 line=`li      $a1, 0x14`
- 8001CF98 -> 8001CFA0 type=21 target=sub_8001CFA0 line=`li      $a2, dword_8004E930`
- 8001CFA0 -> 8001CFA4 type=21 target=sub_8001CFA4 line=`lw      $v1, 0x368($gp)`
- 8001CFA4 -> 8001CFA8 type=21 target=sub_8001CFA8 line=`move    $a3, $zero`
- 8001CFA8 -> 8001CFAC type=21 target=sub_8001CFAC line=`sw      $zero, 0x20+var_10($sp)`
- 8001CFAC -> 8001CFB0 type=21 target=sub_8001CFB0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CFB0 -> 8001CFB4 type=21 target=sub_8001CFB4 line=`sll     $v0, $v1, 2`
- 8001CFB4 -> 8001CFB8 type=21 target=sub_8001CFB8 line=`addu    $v0, $v1`
- 8001CFB8 -> 8001CFBC type=21 target=sub_8001CFBC line=`sll     $v0, 2`
- 8001CFBC -> 8001CFC0 type=21 target=sub_8001CFC0 line=`addu    $v0, $s3`
- 8001CFC0 -> 8001CFC4 type=21 target=sub_8001CFC4 line=`jal     sub_8001B590`
- 8001CFC0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CFC4 -> 8001CFC8 type=21 target=sub_8001CFC8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CFC8 -> 8001CFCC type=21 target=sub_8001CFCC line=`move    $a0, $s1`
- 8001CFCC -> 8001CFD0 type=21 target=sub_8001CFD0 line=`li      $a1, 0xC8`
- 8001CFD0 -> 8001CFD8 type=21 target=sub_8001CFD8 line=`li      $a2, dword_8004E900`
- 8001CFD8 -> 8001CFDC type=21 target=sub_8001CFDC line=`lw      $v1, 0x368($gp)`
- 8001CFDC -> 8001CFE0 type=21 target=sub_8001CFE0 line=`move    $a3, $zero`
- 8001CFE0 -> 8001CFE4 type=21 target=sub_8001CFE4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CFE4 -> 8001CFE8 type=21 target=sub_8001CFE8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CFE8 -> 8001CFEC type=21 target=sub_8001CFEC line=`sll     $v0, $v1, 2`
- 8001CFEC -> 8001CFF0 type=21 target=sub_8001CFF0 line=`addu    $v0, $v1`
- 8001CFF0 -> 8001CFF4 type=21 target=sub_8001CFF4 line=`sll     $v0, 2`
- 8001CFF4 -> 8001CFF8 type=21 target=sub_8001CFF8 line=`addu    $v0, $s3`
- 8001CFF8 -> 8001CFFC type=21 target=sub_8001CFFC line=`jal     sub_8001B590`
- 8001CFF8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CFFC -> 8001D000 type=21 target=sub_8001D000 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D000 -> 8001D004 type=21 target=sub_8001D004 line=`move    $a0, $s0`
- 8001D004 -> 8001D008 type=21 target=sub_8001D008 line=`li      $a1, 0xC8`
- 8001D008 -> 8001D010 type=21 target=sub_8001D010 line=`li      $a2, dword_8004E910`
- 8001D010 -> 8001D014 type=21 target=sub_8001D014 line=`lw      $v1, 0x368($gp)`
- 8001D014 -> 8001D018 type=21 target=sub_8001D018 line=`move    $a3, $zero`
- 8001D018 -> 8001D01C type=21 target=sub_8001D01C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D01C -> 8001D020 type=21 target=sub_8001D020 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D020 -> 8001D024 type=21 target=sub_8001D024 line=`sll     $v0, $v1, 2`
- 8001D024 -> 8001D028 type=21 target=sub_8001D028 line=`addu    $v0, $v1`
- 8001D028 -> 8001D02C type=21 target=sub_8001D02C line=`sll     $v0, 2`
- 8001D02C -> 8001D030 type=21 target=sub_8001D030 line=`addu    $v0, $s3`
- 8001D030 -> 8001D034 type=21 target=sub_8001D034 line=`jal     sub_8001B590`
- 8001D030 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D034 -> 8001D038 type=21 target=sub_8001D038 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D038 -> 8001D03C type=21 target=sub_8001D03C line=`lui     $v0, 0x28  # '('`
- 8001D03C -> 8001D040 type=21 target=sub_8001D040 line=`addu    $s6, $v0`
- 8001D040 -> 8001D044 type=21 target=sub_8001D044 line=`addu    $s5, $v0`
- 8001D044 -> 8001D048 type=21 target=sub_8001D048 line=`slti    $v0, $s4, 7`
- 8001D048 -> 8001D04C type=21 target=sub_8001D04C line=`bnez    $v0, loc_8001CE70`
- 8001D048 -> 8001CE70 type=19 target=loc_8001CE70 line=`bnez    $v0, loc_8001CE70`
- 8001D04C -> 8001D050 type=21 target=sub_8001D050 line=`sra     $s1, $s5, 16`
- 8001D050 -> 8001D054 type=21 target=sub_8001D054 line=`move    $s4, $zero`
- 8001D054 -> 8001D05C type=21 target=sub_8001D05C line=`li      $s6, unk_80087288`
- 8001D05C -> 8001D060 type=21 target=sub_8001D060 line=`move    $s5, $zero`
- 8001D060 -> 8001D064 type=21 target=sub_8001D064 line=`lui     $s3, 0x14`
- 8001D064 -> 8001D068 type=21 target=loc_8001D068 line=`move    $a0, $zero`
- 8001D068 -> 8001D06C type=21 target=sub_8001D06C line=`sra     $s1, $s3, 16`
- 8001D06C -> 8001D070 type=21 target=sub_8001D070 line=`move    $a1, $s1`
- 8001D070 -> 8001D078 type=21 target=sub_8001D078 line=`li      $a2, dword_8004E920`
- 8001D078 -> 8001D07C type=21 target=sub_8001D07C line=`move    $a3, $zero`
- 8001D07C -> 8001D080 type=21 target=sub_8001D080 line=`lw      $v1, 0x368($gp)`
- 8001D080 -> 8001D084 type=21 target=sub_8001D084 line=`addiu   $s4, 1`
- 8001D084 -> 8001D088 type=21 target=sub_8001D088 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D088 -> 8001D08C type=21 target=sub_8001D08C line=`sw      $s2, 0x20+var_C($sp)`
- 8001D08C -> 8001D090 type=21 target=sub_8001D090 line=`sll     $v0, $v1, 2`
- 8001D090 -> 8001D094 type=21 target=sub_8001D094 line=`addu    $v0, $v1`
- 8001D094 -> 8001D098 type=21 target=sub_8001D098 line=`sll     $v0, 2`
- 8001D098 -> 8001D09C type=21 target=sub_8001D09C line=`addu    $v0, $s6`
- 8001D09C -> 8001D0A0 type=21 target=sub_8001D0A0 line=`jal     sub_8001B590`
- 8001D09C -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D0A0 -> 8001D0A4 type=21 target=sub_8001D0A4 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D0A4 -> 8001D0A8 type=21 target=sub_8001D0A8 line=`move    $a0, $zero`
- 8001D0A8 -> 8001D0AC type=21 target=sub_8001D0AC line=`sra     $s0, $s5, 16`
- 8001D0AC -> 8001D0B0 type=21 target=sub_8001D0B0 line=`move    $a1, $s0`
- 8001D0B0 -> 8001D0B8 type=21 target=sub_8001D0B8 line=`li      $a2, dword_8004E900`
- 8001D0B8 -> 8001D0BC type=21 target=sub_8001D0BC line=`lw      $v1, 0x368($gp)`
- 8001D0BC -> 8001D0C0 type=21 target=sub_8001D0C0 line=`move    $a3, $zero`
- 8001D0C0 -> 8001D0C4 type=21 target=sub_8001D0C4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D0C4 -> 8001D0C8 type=21 target=sub_8001D0C8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D0C8 -> 8001D0CC type=21 target=sub_8001D0CC line=`sll     $v0, $v1, 2`
- 8001D0CC -> 8001D0D0 type=21 target=sub_8001D0D0 line=`addu    $v0, $v1`
- 8001D0D0 -> 8001D0D4 type=21 target=sub_8001D0D4 line=`sll     $v0, 2`
- 8001D0D4 -> 8001D0D8 type=21 target=sub_8001D0D8 line=`addu    $v0, $s6`
- 8001D0D8 -> 8001D0DC type=21 target=sub_8001D0DC line=`jal     sub_8001B590`
- 8001D0D8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D0DC -> 8001D0E0 type=21 target=sub_8001D0E0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D0E0 -> 8001D0E4 type=21 target=sub_8001D0E4 line=`li      $a0, 0x12C`
- 8001D0E4 -> 8001D0E8 type=21 target=sub_8001D0E8 line=`move    $a1, $s1`
- 8001D0E8 -> 8001D0F0 type=21 target=sub_8001D0F0 line=`li      $a2, dword_8004E930`
- 8001D0F0 -> 8001D0F4 type=21 target=sub_8001D0F4 line=`lw      $v1, 0x368($gp)`
- 8001D0F4 -> 8001D0F8 type=21 target=sub_8001D0F8 line=`move    $a3, $zero`
- 8001D0F8 -> 8001D0FC type=21 target=sub_8001D0FC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D0FC -> 8001D100 type=21 target=sub_8001D100 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D100 -> 8001D104 type=21 target=sub_8001D104 line=`sll     $v0, $v1, 2`
- 8001D104 -> 8001D108 type=21 target=sub_8001D108 line=`addu    $v0, $v1`
- 8001D108 -> 8001D10C type=21 target=sub_8001D10C line=`sll     $v0, 2`
- 8001D10C -> 8001D110 type=21 target=sub_8001D110 line=`addu    $v0, $s6`
- 8001D110 -> 8001D114 type=21 target=sub_8001D114 line=`jal     sub_8001B590`
- 8001D110 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D114 -> 8001D118 type=21 target=sub_8001D118 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D118 -> 8001D11C type=21 target=sub_8001D11C line=`li      $a0, 0x12C`
- 8001D11C -> 8001D120 type=21 target=sub_8001D120 line=`move    $a1, $s0`
- 8001D120 -> 8001D128 type=21 target=sub_8001D128 line=`li      $a2, dword_8004E910`
- 8001D128 -> 8001D12C type=21 target=sub_8001D12C line=`lw      $v1, 0x368($gp)`
- 8001D12C -> 8001D130 type=21 target=sub_8001D130 line=`move    $a3, $zero`
- 8001D130 -> 8001D134 type=21 target=sub_8001D134 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D134 -> 8001D138 type=21 target=sub_8001D138 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D138 -> 8001D13C type=21 target=sub_8001D13C line=`sll     $v0, $v1, 2`
- 8001D13C -> 8001D140 type=21 target=sub_8001D140 line=`addu    $v0, $v1`
- 8001D140 -> 8001D144 type=21 target=sub_8001D144 line=`sll     $v0, 2`
- 8001D144 -> 8001D148 type=21 target=sub_8001D148 line=`addu    $v0, $s6`
- 8001D148 -> 8001D14C type=21 target=sub_8001D14C line=`jal     sub_8001B590`
- 8001D148 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D14C -> 8001D150 type=21 target=sub_8001D150 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D150 -> 8001D154 type=21 target=sub_8001D154 line=`lui     $v0, 0x28  # '('`
- 8001D154 -> 8001D158 type=21 target=sub_8001D158 line=`addu    $s5, $v0`
- 8001D158 -> 8001D15C type=21 target=sub_8001D15C line=`addu    $s3, $v0`
- 8001D15C -> 8001D160 type=21 target=sub_8001D160 line=`slti    $v0, $s4, 6`
- 8001D160 -> 8001D164 type=21 target=sub_8001D164 line=`bnez    $v0, loc_8001D068`
- 8001D160 -> 8001D068 type=19 target=loc_8001D068 line=`bnez    $v0, loc_8001D068`
- 8001D164 -> 8001D168 type=21 target=sub_8001D168 line=`move    $a0, $zero`
- 8001D168 -> 8001D16C type=21 target=sub_8001D16C line=`li      $a0, 0x28  # '('`
- 8001D16C -> 8001D170 type=21 target=sub_8001D170 line=`li      $a1, 0xB4`
- 8001D170 -> 8001D178 type=21 target=sub_8001D178 line=`li      $a2, dword_8004EA60`
- 8001D178 -> 8001D17C type=21 target=sub_8001D17C line=`move    $a3, $zero`
- 8001D17C -> 8001D180 type=21 target=sub_8001D180 line=`lw      $v1, 0x368($gp)`
- 8001D180 -> 8001D188 type=21 target=sub_8001D188 line=`li      $s0, unk_80087288`
- 8001D188 -> 8001D18C type=21 target=sub_8001D18C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D18C -> 8001D190 type=21 target=sub_8001D190 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D190 -> 8001D194 type=21 target=sub_8001D194 line=`sll     $v0, $v1, 2`
- 8001D194 -> 8001D198 type=21 target=sub_8001D198 line=`addu    $v0, $v1`
- 8001D198 -> 8001D19C type=21 target=sub_8001D19C line=`sll     $v0, 2`
- 8001D19C -> 8001D1A0 type=21 target=sub_8001D1A0 line=`addu    $v0, $s0`
- 8001D1A0 -> 8001D1A4 type=21 target=sub_8001D1A4 line=`jal     sub_8001B590`
- 8001D1A0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D1A4 -> 8001D1A8 type=21 target=sub_8001D1A8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D1A8 -> 8001D1AC type=21 target=sub_8001D1AC line=`li      $a0, 0x3C  # '<'`
- 8001D1AC -> 8001D1B0 type=21 target=sub_8001D1B0 line=`li      $a1, 0xB4`
- 8001D1B0 -> 8001D1B8 type=21 target=sub_8001D1B8 line=`li      $a2, dword_8004EA70`
- 8001D1B8 -> 8001D1BC type=21 target=sub_8001D1BC line=`lw      $v1, 0x368($gp)`
- 8001D1BC -> 8001D1C0 type=21 target=sub_8001D1C0 line=`move    $a3, $zero`
- 8001D1C0 -> 8001D1C4 type=21 target=sub_8001D1C4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D1C4 -> 8001D1C8 type=21 target=sub_8001D1C8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D1C8 -> 8001D1CC type=21 target=sub_8001D1CC line=`sll     $v0, $v1, 2`
- 8001D1CC -> 8001D1D0 type=21 target=sub_8001D1D0 line=`addu    $v0, $v1`
- 8001D1D0 -> 8001D1D4 type=21 target=sub_8001D1D4 line=`sll     $v0, 2`
- 8001D1D4 -> 8001D1D8 type=21 target=sub_8001D1D8 line=`addu    $v0, $s0`
- 8001D1D8 -> 8001D1DC type=21 target=sub_8001D1DC line=`jal     sub_8001B590`
- 8001D1D8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D1DC -> 8001D1E0 type=21 target=sub_8001D1E0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D1E0 -> 8001D1E4 type=21 target=sub_8001D1E4 line=`li      $a0, 0x50  # 'P'`
- 8001D1E4 -> 8001D1E8 type=21 target=sub_8001D1E8 line=`li      $a1, 0xB4`
- 8001D1E8 -> 8001D1F0 type=21 target=sub_8001D1F0 line=`li      $a2, dword_8004EA80`
- 8001D1F0 -> 8001D1F4 type=21 target=sub_8001D1F4 line=`lw      $v1, 0x368($gp)`
- 8001D1F4 -> 8001D1F8 type=21 target=sub_8001D1F8 line=`move    $a3, $zero`
- 8001D1F8 -> 8001D1FC type=21 target=sub_8001D1FC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D1FC -> 8001D200 type=21 target=sub_8001D200 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D200 -> 8001D204 type=21 target=sub_8001D204 line=`sll     $v0, $v1, 2`
- 8001D204 -> 8001D208 type=21 target=sub_8001D208 line=`addu    $v0, $v1`
- 8001D208 -> 8001D20C type=21 target=sub_8001D20C line=`sll     $v0, 2`
- 8001D20C -> 8001D210 type=21 target=sub_8001D210 line=`addu    $v0, $s0`
- 8001D210 -> 8001D214 type=21 target=sub_8001D214 line=`jal     sub_8001B590`
- 8001D210 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D214 -> 8001D218 type=21 target=sub_8001D218 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D218 -> 8001D21C type=21 target=sub_8001D21C line=`li      $a0, 0x64  # 'd'`
- 8001D21C -> 8001D220 type=21 target=sub_8001D220 line=`li      $a1, 0xB4`
- 8001D220 -> 8001D228 type=21 target=sub_8001D228 line=`li      $a2, dword_8004EA90`
- 8001D228 -> 8001D22C type=21 target=sub_8001D22C line=`lw      $v1, 0x368($gp)`
- 8001D22C -> 8001D230 type=21 target=sub_8001D230 line=`move    $a3, $zero`
- 8001D230 -> 8001D234 type=21 target=sub_8001D234 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D234 -> 8001D238 type=21 target=sub_8001D238 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D238 -> 8001D23C type=21 target=sub_8001D23C line=`sll     $v0, $v1, 2`
- 8001D23C -> 8001D240 type=21 target=sub_8001D240 line=`addu    $v0, $v1`
- 8001D240 -> 8001D244 type=21 target=sub_8001D244 line=`sll     $v0, 2`
- 8001D244 -> 8001D248 type=21 target=sub_8001D248 line=`addu    $v0, $s0`
- 8001D248 -> 8001D24C type=21 target=sub_8001D24C line=`jal     sub_8001B590`
- 8001D248 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D24C -> 8001D250 type=21 target=sub_8001D250 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D250 -> 8001D254 type=21 target=sub_8001D254 line=`li      $a0, 0x78  # 'x'`
- 8001D254 -> 8001D258 type=21 target=sub_8001D258 line=`li      $a1, 0xB4`
- 8001D258 -> 8001D260 type=21 target=sub_8001D260 line=`li      $a2, dword_8004EAA0`
- 8001D260 -> 8001D264 type=21 target=sub_8001D264 line=`lw      $v1, 0x368($gp)`
- 8001D264 -> 8001D268 type=21 target=sub_8001D268 line=`move    $a3, $zero`
- 8001D268 -> 8001D26C type=21 target=sub_8001D26C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D26C -> 8001D270 type=21 target=sub_8001D270 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D270 -> 8001D274 type=21 target=sub_8001D274 line=`sll     $v0, $v1, 2`
- 8001D274 -> 8001D278 type=21 target=sub_8001D278 line=`addu    $v0, $v1`
- 8001D278 -> 8001D27C type=21 target=sub_8001D27C line=`sll     $v0, 2`
- 8001D27C -> 8001D280 type=21 target=sub_8001D280 line=`addu    $v0, $s0`
- 8001D280 -> 8001D284 type=21 target=sub_8001D284 line=`jal     sub_8001B590`
- 8001D280 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D284 -> 8001D288 type=21 target=sub_8001D288 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D288 -> 8001D28C type=21 target=sub_8001D28C line=`li      $a0, 0x8C`
- 8001D28C -> 8001D290 type=21 target=sub_8001D290 line=`li      $a1, 0xB4`
- 8001D290 -> 8001D298 type=21 target=sub_8001D298 line=`li      $a2, dword_8004EAB0`
- 8001D298 -> 8001D29C type=21 target=sub_8001D29C line=`lw      $v1, 0x368($gp)`
- 8001D29C -> 8001D2A0 type=21 target=sub_8001D2A0 line=`move    $a3, $zero`
- 8001D2A0 -> 8001D2A4 type=21 target=sub_8001D2A4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D2A4 -> 8001D2A8 type=21 target=sub_8001D2A8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D2A8 -> 8001D2AC type=21 target=sub_8001D2AC line=`sll     $v0, $v1, 2`
- 8001D2AC -> 8001D2B0 type=21 target=sub_8001D2B0 line=`addu    $v0, $v1`
- 8001D2B0 -> 8001D2B4 type=21 target=sub_8001D2B4 line=`sll     $v0, 2`
- 8001D2B4 -> 8001D2B8 type=21 target=sub_8001D2B8 line=`addu    $v0, $s0`
- 8001D2B8 -> 8001D2BC type=21 target=sub_8001D2BC line=`jal     sub_8001B590`
- 8001D2B8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D2BC -> 8001D2C0 type=21 target=sub_8001D2C0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D2C0 -> 8001D2C4 type=21 target=sub_8001D2C4 line=`li      $a0, 0xA0`
- 8001D2C4 -> 8001D2C8 type=21 target=sub_8001D2C8 line=`li      $a1, 0xB4`
- 8001D2C8 -> 8001D2D0 type=21 target=sub_8001D2D0 line=`li      $a2, dword_8004EAC0`
- 8001D2D0 -> 8001D2D4 type=21 target=sub_8001D2D4 line=`lw      $v1, 0x368($gp)`
- 8001D2D4 -> 8001D2D8 type=21 target=sub_8001D2D8 line=`move    $a3, $zero`
- 8001D2D8 -> 8001D2DC type=21 target=sub_8001D2DC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D2DC -> 8001D2E0 type=21 target=sub_8001D2E0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D2E0 -> 8001D2E4 type=21 target=sub_8001D2E4 line=`sll     $v0, $v1, 2`
- 8001D2E4 -> 8001D2E8 type=21 target=sub_8001D2E8 line=`addu    $v0, $v1`
- 8001D2E8 -> 8001D2EC type=21 target=sub_8001D2EC line=`sll     $v0, 2`
- 8001D2EC -> 8001D2F0 type=21 target=sub_8001D2F0 line=`addu    $v0, $s0`
- 8001D2F0 -> 8001D2F4 type=21 target=sub_8001D2F4 line=`jal     sub_8001B590`
- 8001D2F0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D2F4 -> 8001D2F8 type=21 target=sub_8001D2F8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D2F8 -> 8001D2FC type=21 target=sub_8001D2FC line=`li      $a0, 0xB4`
- 8001D2FC -> 8001D300 type=21 target=sub_8001D300 line=`li      $a1, 0xB4`
- 8001D300 -> 8001D308 type=21 target=sub_8001D308 line=`li      $a2, dword_8004EAD0`
- 8001D308 -> 8001D30C type=21 target=sub_8001D30C line=`lw      $v1, 0x368($gp)`
- 8001D30C -> 8001D310 type=21 target=sub_8001D310 line=`move    $a3, $zero`
- 8001D310 -> 8001D314 type=21 target=sub_8001D314 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D314 -> 8001D318 type=21 target=sub_8001D318 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D318 -> 8001D31C type=21 target=sub_8001D31C line=`sll     $v0, $v1, 2`
- 8001D31C -> 8001D320 type=21 target=sub_8001D320 line=`addu    $v0, $v1`
- 8001D320 -> 8001D324 type=21 target=sub_8001D324 line=`sll     $v0, 2`
- 8001D324 -> 8001D328 type=21 target=sub_8001D328 line=`addu    $v0, $s0`
- 8001D328 -> 8001D32C type=21 target=sub_8001D32C line=`jal     sub_8001B590`
- 8001D328 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D32C -> 8001D330 type=21 target=sub_8001D330 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D330 -> 8001D334 type=21 target=sub_8001D334 line=`li      $a0, 0xC8`
- 8001D334 -> 8001D338 type=21 target=sub_8001D338 line=`li      $a1, 0xB4`
- 8001D338 -> 8001D340 type=21 target=sub_8001D340 line=`li      $a2, dword_8004EAE0`
- 8001D340 -> 8001D344 type=21 target=sub_8001D344 line=`lw      $v1, 0x368($gp)`
- 8001D344 -> 8001D348 type=21 target=sub_8001D348 line=`move    $a3, $zero`
- 8001D348 -> 8001D34C type=21 target=sub_8001D34C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D34C -> 8001D350 type=21 target=sub_8001D350 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D350 -> 8001D354 type=21 target=sub_8001D354 line=`sll     $v0, $v1, 2`
- 8001D354 -> 8001D358 type=21 target=sub_8001D358 line=`addu    $v0, $v1`
- 8001D358 -> 8001D35C type=21 target=sub_8001D35C line=`sll     $v0, 2`
- 8001D35C -> 8001D360 type=21 target=sub_8001D360 line=`addu    $v0, $s0`
- 8001D360 -> 8001D364 type=21 target=sub_8001D364 line=`jal     sub_8001B590`
- 8001D360 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D364 -> 8001D368 type=21 target=sub_8001D368 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D368 -> 8001D36C type=21 target=sub_8001D36C line=`li      $a0, 0xDC`
- 8001D36C -> 8001D370 type=21 target=sub_8001D370 line=`li      $a1, 0xB4`
- 8001D370 -> 8001D378 type=21 target=sub_8001D378 line=`li      $a2, dword_8004EAF0`
- 8001D378 -> 8001D37C type=21 target=sub_8001D37C line=`lw      $v1, 0x368($gp)`
- 8001D37C -> 8001D380 type=21 target=sub_8001D380 line=`move    $a3, $zero`
- 8001D380 -> 8001D384 type=21 target=sub_8001D384 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D384 -> 8001D388 type=21 target=sub_8001D388 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D388 -> 8001D38C type=21 target=sub_8001D38C line=`sll     $v0, $v1, 2`
- 8001D38C -> 8001D390 type=21 target=sub_8001D390 line=`addu    $v0, $v1`
- 8001D390 -> 8001D394 type=21 target=sub_8001D394 line=`sll     $v0, 2`
- 8001D394 -> 8001D398 type=21 target=sub_8001D398 line=`addu    $v0, $s0`
- 8001D398 -> 8001D39C type=21 target=sub_8001D39C line=`jal     sub_8001B590`
- 8001D398 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D39C -> 8001D3A0 type=21 target=sub_8001D3A0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D3A0 -> 8001D3A4 type=21 target=sub_8001D3A4 line=`li      $a0, 0xF0`
- 8001D3A4 -> 8001D3A8 type=21 target=sub_8001D3A8 line=`li      $a1, 0xB4`
- 8001D3A8 -> 8001D3B0 type=21 target=sub_8001D3B0 line=`li      $a2, dword_8004EB00`
- 8001D3B0 -> 8001D3B4 type=21 target=sub_8001D3B4 line=`lw      $v1, 0x368($gp)`
- 8001D3B4 -> 8001D3B8 type=21 target=sub_8001D3B8 line=`move    $a3, $zero`
- 8001D3B8 -> 8001D3BC type=21 target=sub_8001D3BC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D3BC -> 8001D3C0 type=21 target=sub_8001D3C0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D3C0 -> 8001D3C4 type=21 target=sub_8001D3C4 line=`sll     $v0, $v1, 2`
- 8001D3C4 -> 8001D3C8 type=21 target=sub_8001D3C8 line=`addu    $v0, $v1`
- 8001D3C8 -> 8001D3CC type=21 target=sub_8001D3CC line=`sll     $v0, 2`
- 8001D3CC -> 8001D3D0 type=21 target=sub_8001D3D0 line=`addu    $v0, $s0`
- 8001D3D0 -> 8001D3D4 type=21 target=sub_8001D3D4 line=`jal     sub_8001B590`
- 8001D3D0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D3D4 -> 8001D3D8 type=21 target=sub_8001D3D8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D3D8 -> 8001D3DC type=21 target=sub_8001D3DC line=`li      $a0, 0x104`
- 8001D3DC -> 8001D3E0 type=21 target=sub_8001D3E0 line=`li      $a1, 0xB4`
- 8001D3E0 -> 8001D3E8 type=21 target=sub_8001D3E8 line=`li      $a2, dword_8004EB10`
- 8001D3E8 -> 8001D3EC type=21 target=sub_8001D3EC line=`lw      $v1, 0x368($gp)`
- 8001D3EC -> 8001D3F0 type=21 target=sub_8001D3F0 line=`move    $a3, $zero`
- 8001D3F0 -> 8001D3F4 type=21 target=sub_8001D3F4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D3F4 -> 8001D3F8 type=21 target=sub_8001D3F8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D3F8 -> 8001D3FC type=21 target=sub_8001D3FC line=`sll     $v0, $v1, 2`
- 8001D3FC -> 8001D400 type=21 target=sub_8001D400 line=`addu    $v0, $v1`
- 8001D400 -> 8001D404 type=21 target=sub_8001D404 line=`sll     $v0, 2`
- 8001D404 -> 8001D408 type=21 target=sub_8001D408 line=`addu    $v0, $s0`
- 8001D408 -> 8001D40C type=21 target=sub_8001D40C line=`jal     sub_8001B590`
- 8001D408 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D40C -> 8001D410 type=21 target=sub_8001D410 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D410 -> 8001D414 type=21 target=sub_8001D414 line=`li      $a0, 0x118`
- 8001D414 -> 8001D418 type=21 target=sub_8001D418 line=`li      $a1, 0x28  # '('`
- 8001D418 -> 8001D420 type=21 target=sub_8001D420 line=`li      $a2, dword_8004E980`
- 8001D420 -> 8001D424 type=21 target=sub_8001D424 line=`lw      $v1, 0x368($gp)`
- 8001D424 -> 8001D428 type=21 target=sub_8001D428 line=`move    $a3, $zero`
- 8001D428 -> 8001D42C type=21 target=sub_8001D42C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D42C -> 8001D430 type=21 target=sub_8001D430 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D430 -> 8001D434 type=21 target=sub_8001D434 line=`sll     $v0, $v1, 2`
- 8001D434 -> 8001D438 type=21 target=sub_8001D438 line=`addu    $v0, $v1`
- 8001D438 -> 8001D43C type=21 target=sub_8001D43C line=`sll     $v0, 2`
- 8001D43C -> 8001D440 type=21 target=sub_8001D440 line=`addu    $v0, $s0`
- 8001D440 -> 8001D444 type=21 target=sub_8001D444 line=`jal     sub_8001B590`
- 8001D440 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D444 -> 8001D448 type=21 target=sub_8001D448 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D448 -> 8001D44C type=21 target=sub_8001D44C line=`li      $a0, 0x14`
- 8001D44C -> 8001D450 type=21 target=sub_8001D450 line=`li      $a1, 0x28  # '('`
- 8001D450 -> 8001D458 type=21 target=sub_8001D458 line=`li      $a2, dword_8004EA50`
- 8001D458 -> 8001D45C type=21 target=sub_8001D45C line=`lw      $v1, 0x368($gp)`
- 8001D45C -> 8001D460 type=21 target=sub_8001D460 line=`move    $a3, $zero`
- 8001D460 -> 8001D464 type=21 target=sub_8001D464 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D464 -> 8001D468 type=21 target=sub_8001D468 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D468 -> 8001D46C type=21 target=sub_8001D46C line=`sll     $v0, $v1, 2`
- 8001D46C -> 8001D470 type=21 target=sub_8001D470 line=`addu    $v0, $v1`
- 8001D470 -> 8001D474 type=21 target=sub_8001D474 line=`sll     $v0, 2`
- 8001D474 -> 8001D478 type=21 target=sub_8001D478 line=`addu    $v0, $s0`
- 8001D478 -> 8001D47C type=21 target=sub_8001D47C line=`jal     sub_8001B590`
- 8001D478 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D47C -> 8001D480 type=21 target=sub_8001D480 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D480 -> 8001D484 type=21 target=sub_8001D484 line=`li      $a0, 0xA0`
- 8001D484 -> 8001D488 type=21 target=sub_8001D488 line=`li      $a1, 0x28  # '('`
- 8001D488 -> 8001D490 type=21 target=sub_8001D490 line=`li      $a2, dword_8004E9F0`
- 8001D490 -> 8001D494 type=21 target=sub_8001D494 line=`lw      $v1, 0x368($gp)`
- 8001D494 -> 8001D498 type=21 target=sub_8001D498 line=`move    $a3, $zero`
- 8001D498 -> 8001D49C type=21 target=sub_8001D49C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D49C -> 8001D4A0 type=21 target=sub_8001D4A0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D4A0 -> 8001D4A4 type=21 target=sub_8001D4A4 line=`sll     $v0, $v1, 2`
- 8001D4A4 -> 8001D4A8 type=21 target=sub_8001D4A8 line=`addu    $v0, $v1`
- 8001D4A8 -> 8001D4AC type=21 target=sub_8001D4AC line=`sll     $v0, 2`
- 8001D4AC -> 8001D4B0 type=21 target=sub_8001D4B0 line=`addu    $v0, $s0`
- 8001D4B0 -> 8001D4B4 type=21 target=sub_8001D4B4 line=`jal     sub_8001B590`
- 8001D4B0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D4B4 -> 8001D4B8 type=21 target=sub_8001D4B8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D4B8 -> 8001D4BC type=21 target=sub_8001D4BC line=`li      $a0, 0x8C`
- 8001D4BC -> 8001D4C0 type=21 target=sub_8001D4C0 line=`li      $a1, 0x28  # '('`
- 8001D4C0 -> 8001D4C8 type=21 target=sub_8001D4C8 line=`li      $a2, dword_8004E9E0`
- 8001D4C8 -> 8001D4CC type=21 target=sub_8001D4CC line=`lw      $v1, 0x368($gp)`
- 8001D4CC -> 8001D4D0 type=21 target=sub_8001D4D0 line=`move    $a3, $zero`
- 8001D4D0 -> 8001D4D4 type=21 target=sub_8001D4D4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D4D4 -> 8001D4D8 type=21 target=sub_8001D4D8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D4D8 -> 8001D4DC type=21 target=sub_8001D4DC line=`sll     $v0, $v1, 2`
- 8001D4DC -> 8001D4E0 type=21 target=sub_8001D4E0 line=`addu    $v0, $v1`
- 8001D4E0 -> 8001D4E4 type=21 target=sub_8001D4E4 line=`sll     $v0, 2`
- 8001D4E4 -> 8001D4E8 type=21 target=sub_8001D4E8 line=`addu    $v0, $s0`
- 8001D4E8 -> 8001D4EC type=21 target=sub_8001D4EC line=`jal     sub_8001B590`
- 8001D4E8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D4EC -> 8001D4F0 type=21 target=sub_8001D4F0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D4F0 -> 8001D4F4 type=21 target=sub_8001D4F4 line=`li      $a0, 0xB4`
- 8001D4F4 -> 8001D4F8 type=21 target=sub_8001D4F8 line=`li      $a1, 0x28  # '('`
- 8001D4F8 -> 8001D500 type=21 target=sub_8001D500 line=`li      $a2, dword_8004EA00`
- 8001D500 -> 8001D504 type=21 target=sub_8001D504 line=`lw      $v1, 0x368($gp)`
- 8001D504 -> 8001D508 type=21 target=sub_8001D508 line=`move    $a3, $zero`
- 8001D508 -> 8001D50C type=21 target=sub_8001D50C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D50C -> 8001D510 type=21 target=sub_8001D510 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D510 -> 8001D514 type=21 target=sub_8001D514 line=`sll     $v0, $v1, 2`
- 8001D514 -> 8001D518 type=21 target=sub_8001D518 line=`addu    $v0, $v1`
- 8001D518 -> 8001D51C type=21 target=sub_8001D51C line=`sll     $v0, 2`
- 8001D51C -> 8001D520 type=21 target=sub_8001D520 line=`addu    $v0, $s0`
- 8001D520 -> 8001D524 type=21 target=sub_8001D524 line=`jal     sub_8001B590`
- 8001D520 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D524 -> 8001D528 type=21 target=sub_8001D528 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D528 -> 8001D52C type=21 target=sub_8001D52C line=`li      $a0, 0x78  # 'x'`
- 8001D52C -> 8001D530 type=21 target=sub_8001D530 line=`li      $a1, 0x28  # '('`
- 8001D530 -> 8001D538 type=21 target=sub_8001D538 line=`li      $a2, dword_8004E9D0`
- 8001D538 -> 8001D53C type=21 target=sub_8001D53C line=`lw      $v1, 0x368($gp)`
- 8001D53C -> 8001D540 type=21 target=sub_8001D540 line=`move    $a3, $zero`
- 8001D540 -> 8001D544 type=21 target=sub_8001D544 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D544 -> 8001D548 type=21 target=sub_8001D548 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D548 -> 8001D54C type=21 target=sub_8001D54C line=`sll     $v0, $v1, 2`
- 8001D54C -> 8001D550 type=21 target=sub_8001D550 line=`addu    $v0, $v1`
- 8001D550 -> 8001D554 type=21 target=sub_8001D554 line=`sll     $v0, 2`
- 8001D554 -> 8001D558 type=21 target=sub_8001D558 line=`addu    $v0, $s0`
- 8001D558 -> 8001D55C type=21 target=sub_8001D55C line=`jal     sub_8001B590`
- 8001D558 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D55C -> 8001D560 type=21 target=sub_8001D560 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D560 -> 8001D564 type=21 target=sub_8001D564 line=`li      $a0, 0xC8`
- 8001D564 -> 8001D568 type=21 target=sub_8001D568 line=`li      $a1, 0x28  # '('`
- 8001D568 -> 8001D570 type=21 target=sub_8001D570 line=`li      $a2, dword_8004EA10`
- 8001D570 -> 8001D574 type=21 target=sub_8001D574 line=`lw      $v1, 0x368($gp)`
- 8001D574 -> 8001D578 type=21 target=sub_8001D578 line=`move    $a3, $zero`
- 8001D578 -> 8001D57C type=21 target=sub_8001D57C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D57C -> 8001D580 type=21 target=sub_8001D580 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D580 -> 8001D584 type=21 target=sub_8001D584 line=`sll     $v0, $v1, 2`
- 8001D584 -> 8001D588 type=21 target=sub_8001D588 line=`addu    $v0, $v1`
- 8001D588 -> 8001D58C type=21 target=sub_8001D58C line=`sll     $v0, 2`
- 8001D58C -> 8001D590 type=21 target=sub_8001D590 line=`addu    $v0, $s0`
- 8001D590 -> 8001D594 type=21 target=sub_8001D594 line=`jal     sub_8001B590`
- 8001D590 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D594 -> 8001D598 type=21 target=sub_8001D598 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D598 -> 8001D59C type=21 target=sub_8001D59C line=`li      $a0, 0x64  # 'd'`
- 8001D59C -> 8001D5A0 type=21 target=sub_8001D5A0 line=`li      $a1, 0x28  # '('`
- 8001D5A0 -> 8001D5A8 type=21 target=sub_8001D5A8 line=`li      $a2, dword_8004E9C0`
- 8001D5A8 -> 8001D5AC type=21 target=sub_8001D5AC line=`lw      $v1, 0x368($gp)`
- 8001D5AC -> 8001D5B0 type=21 target=sub_8001D5B0 line=`move    $a3, $zero`
- 8001D5B0 -> 8001D5B4 type=21 target=sub_8001D5B4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D5B4 -> 8001D5B8 type=21 target=sub_8001D5B8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D5B8 -> 8001D5BC type=21 target=sub_8001D5BC line=`sll     $v0, $v1, 2`
- 8001D5BC -> 8001D5C0 type=21 target=sub_8001D5C0 line=`addu    $v0, $v1`
- 8001D5C0 -> 8001D5C4 type=21 target=sub_8001D5C4 line=`sll     $v0, 2`
- 8001D5C4 -> 8001D5C8 type=21 target=sub_8001D5C8 line=`addu    $v0, $s0`
- 8001D5C8 -> 8001D5CC type=21 target=sub_8001D5CC line=`jal     sub_8001B590`
- 8001D5C8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D5CC -> 8001D5D0 type=21 target=sub_8001D5D0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D5D0 -> 8001D5D4 type=21 target=sub_8001D5D4 line=`li      $a0, 0xF0`
- 8001D5D4 -> 8001D5D8 type=21 target=sub_8001D5D8 line=`li      $a1, 0x28  # '('`
- 8001D5D8 -> 8001D5E0 type=21 target=sub_8001D5E0 line=`li      $a2, dword_8004EA30`
- 8001D5E0 -> 8001D5E4 type=21 target=sub_8001D5E4 line=`lw      $v1, 0x368($gp)`
- 8001D5E4 -> 8001D5E8 type=21 target=sub_8001D5E8 line=`move    $a3, $zero`
- 8001D5E8 -> 8001D5EC type=21 target=sub_8001D5EC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D5EC -> 8001D5F0 type=21 target=sub_8001D5F0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D5F0 -> 8001D5F4 type=21 target=sub_8001D5F4 line=`sll     $v0, $v1, 2`
- 8001D5F4 -> 8001D5F8 type=21 target=sub_8001D5F8 line=`addu    $v0, $v1`
- 8001D5F8 -> 8001D5FC type=21 target=sub_8001D5FC line=`sll     $v0, 2`
- 8001D5FC -> 8001D600 type=21 target=sub_8001D600 line=`addu    $v0, $s0`
- 8001D600 -> 8001D604 type=21 target=sub_8001D604 line=`jal     sub_8001B590`
- 8001D600 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D604 -> 8001D608 type=21 target=sub_8001D608 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D608 -> 8001D60C type=21 target=sub_8001D60C line=`li      $a0, 0x50  # 'P'`
- 8001D60C -> 8001D610 type=21 target=sub_8001D610 line=`li      $a1, 0x28  # '('`
- 8001D610 -> 8001D618 type=21 target=sub_8001D618 line=`li      $a2, dword_8004E9B0`
- 8001D618 -> 8001D61C type=21 target=sub_8001D61C line=`lw      $v1, 0x368($gp)`
- 8001D61C -> 8001D620 type=21 target=sub_8001D620 line=`move    $a3, $zero`
- 8001D620 -> 8001D624 type=21 target=sub_8001D624 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D624 -> 8001D628 type=21 target=sub_8001D628 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D628 -> 8001D62C type=21 target=sub_8001D62C line=`sll     $v0, $v1, 2`
- 8001D62C -> 8001D630 type=21 target=sub_8001D630 line=`addu    $v0, $v1`
- 8001D630 -> 8001D634 type=21 target=sub_8001D634 line=`sll     $v0, 2`
- 8001D634 -> 8001D638 type=21 target=sub_8001D638 line=`addu    $v0, $s0`
- 8001D638 -> 8001D63C type=21 target=sub_8001D63C line=`jal     sub_8001B590`
- 8001D638 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D63C -> 8001D640 type=21 target=sub_8001D640 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D640 -> 8001D644 type=21 target=sub_8001D644 line=`li      $a0, 0xDC`
- 8001D644 -> 8001D648 type=21 target=sub_8001D648 line=`li      $a1, 0x28  # '('`
- 8001D648 -> 8001D650 type=21 target=sub_8001D650 line=`li      $a2, dword_8004EA20`
- 8001D650 -> 8001D654 type=21 target=sub_8001D654 line=`lw      $v1, 0x368($gp)`
- 8001D654 -> 8001D658 type=21 target=sub_8001D658 line=`move    $a3, $zero`
- 8001D658 -> 8001D65C type=21 target=sub_8001D65C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D65C -> 8001D660 type=21 target=sub_8001D660 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D660 -> 8001D664 type=21 target=sub_8001D664 line=`sll     $v0, $v1, 2`
- 8001D664 -> 8001D668 type=21 target=sub_8001D668 line=`addu    $v0, $v1`
- 8001D668 -> 8001D66C type=21 target=sub_8001D66C line=`sll     $v0, 2`
- 8001D66C -> 8001D670 type=21 target=sub_8001D670 line=`addu    $v0, $s0`
- 8001D670 -> 8001D674 type=21 target=sub_8001D674 line=`jal     sub_8001B590`
- 8001D670 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D674 -> 8001D678 type=21 target=sub_8001D678 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D678 -> 8001D67C type=21 target=sub_8001D67C line=`li      $a0, 0x3C  # '<'`
- 8001D67C -> 8001D680 type=21 target=sub_8001D680 line=`li      $a1, 0x28  # '('`
- 8001D680 -> 8001D688 type=21 target=sub_8001D688 line=`li      $a2, dword_8004E9A0`
- 8001D688 -> 8001D68C type=21 target=sub_8001D68C line=`lw      $v1, 0x368($gp)`
- 8001D68C -> 8001D690 type=21 target=sub_8001D690 line=`move    $a3, $zero`
- 8001D690 -> 8001D694 type=21 target=sub_8001D694 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D694 -> 8001D698 type=21 target=sub_8001D698 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D698 -> 8001D69C type=21 target=sub_8001D69C line=`sll     $v0, $v1, 2`
- 8001D69C -> 8001D6A0 type=21 target=sub_8001D6A0 line=`addu    $v0, $v1`
- 8001D6A0 -> 8001D6A4 type=21 target=sub_8001D6A4 line=`sll     $v0, 2`
- 8001D6A4 -> 8001D6A8 type=21 target=sub_8001D6A8 line=`addu    $v0, $s0`
- 8001D6A8 -> 8001D6AC type=21 target=sub_8001D6AC line=`jal     sub_8001B590`
- 8001D6A8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D6AC -> 8001D6B0 type=21 target=sub_8001D6B0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D6B0 -> 8001D6B4 type=21 target=sub_8001D6B4 line=`li      $a0, 0x104`
- 8001D6B4 -> 8001D6B8 type=21 target=sub_8001D6B8 line=`li      $a1, 0x28  # '('`
- 8001D6B8 -> 8001D6C0 type=21 target=sub_8001D6C0 line=`li      $a2, dword_8004EA40`
- 8001D6C0 -> 8001D6C4 type=21 target=sub_8001D6C4 line=`lw      $v1, 0x368($gp)`
- 8001D6C4 -> 8001D6C8 type=21 target=sub_8001D6C8 line=`move    $a3, $zero`
- 8001D6C8 -> 8001D6CC type=21 target=sub_8001D6CC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D6CC -> 8001D6D0 type=21 target=sub_8001D6D0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D6D0 -> 8001D6D4 type=21 target=sub_8001D6D4 line=`sll     $v0, $v1, 2`
- 8001D6D4 -> 8001D6D8 type=21 target=sub_8001D6D8 line=`addu    $v0, $v1`
- 8001D6D8 -> 8001D6DC type=21 target=sub_8001D6DC line=`sll     $v0, 2`
- 8001D6DC -> 8001D6E0 type=21 target=sub_8001D6E0 line=`addu    $v0, $s0`
- 8001D6E0 -> 8001D6E4 type=21 target=sub_8001D6E4 line=`jal     sub_8001B590`
- 8001D6E0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D6E4 -> 8001D6E8 type=21 target=sub_8001D6E8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D6E8 -> 8001D6EC type=21 target=sub_8001D6EC line=`li      $a0, 0x28  # '('`
- 8001D6EC -> 8001D6F0 type=21 target=sub_8001D6F0 line=`li      $a1, 0x28  # '('`
- 8001D6F0 -> 8001D6F8 type=21 target=sub_8001D6F8 line=`li      $a2, dword_8004E990`
- 8001D6F8 -> 8001D6FC type=21 target=sub_8001D6FC line=`lw      $v1, 0x368($gp)`
- 8001D6FC -> 8001D700 type=21 target=sub_8001D700 line=`move    $a3, $zero`
- 8001D700 -> 8001D704 type=21 target=sub_8001D704 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D704 -> 8001D708 type=21 target=sub_8001D708 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D708 -> 8001D70C type=21 target=sub_8001D70C line=`sll     $v0, $v1, 2`
- 8001D70C -> 8001D710 type=21 target=sub_8001D710 line=`addu    $v0, $v1`
- 8001D710 -> 8001D714 type=21 target=sub_8001D714 line=`sll     $v0, 2`
- 8001D714 -> 8001D718 type=21 target=sub_8001D718 line=`addu    $v0, $s0`
- 8001D718 -> 8001D71C type=21 target=sub_8001D71C line=`jal     sub_8001B590`
- 8001D718 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D71C -> 8001D720 type=21 target=sub_8001D720 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D720 -> 8001D724 type=21 target=sub_8001D724 line=`lw      $ra, 0x20+var_s1C($sp)`
- 8001D724 -> 8001D728 type=21 target=sub_8001D728 line=`lw      $s6, 0x20+var_s18($sp)`
- 8001D728 -> 8001D72C type=21 target=sub_8001D72C line=`lw      $s5, 0x20+var_s14($sp)`
- 8001D72C -> 8001D730 type=21 target=sub_8001D730 line=`lw      $s4, 0x20+var_s10($sp)`
- 8001D730 -> 8001D734 type=21 target=sub_8001D734 line=`lw      $s3, 0x20+var_sC($sp)`
- 8001D734 -> 8001D738 type=21 target=sub_8001D738 line=`lw      $s2, 0x20+var_s8($sp)`
- 8001D738 -> 8001D73C type=21 target=sub_8001D73C line=`lw      $s1, 0x20+var_s4($sp)`
- 8001D73C -> 8001D740 type=21 target=sub_8001D740 line=`lw      $s0, 0x20+var_s0($sp)`
- 8001D740 -> 8001D744 type=21 target=sub_8001D744 line=`addiu   $sp, 0x40`
- 8001D744 -> 8001D748 type=21 target=sub_8001D748 line=`jr      $ra`

### Full disasm
```asm
8001CE30: addiu   $sp, -0x40                                               ; C0 FF BD 27
8001CE34: sw      $s2, 0x20+var_s8($sp)                                    ; 28 00 B2 AF
8001CE38: move    $s2, $a0                                                 ; 21 90 80 00
8001CE3C: sw      $s4, 0x20+var_s10($sp)                                   ; 30 00 B4 AF
8001CE40: move    $s4, $zero                                               ; 21 A0 00 00
8001CE44: sw      $s3, 0x20+var_sC($sp)                                    ; 2C 00 B3 AF
8001CE48: li      $s3, unk_80087288                                        ; 08 80 13 3C 88 72 73 26
8001CE50: sw      $s6, 0x20+var_s18($sp)                                   ; 38 00 B6 AF
8001CE54: lui     $s6, 0x14                                                ; 14 00 16 3C
8001CE58: sw      $s5, 0x20+var_s14($sp)                                   ; 34 00 B5 AF
8001CE5C: lui     $s5, 0x28  # '('                                         ; 28 00 15 3C
8001CE60: sw      $ra, 0x20+var_s1C($sp)                                   ; 3C 00 BF AF
8001CE64: sw      $s1, 0x20+var_s4($sp)                                    ; 24 00 B1 AF
8001CE68: sw      $s0, 0x20+var_s0($sp)                                    ; 20 00 B0 AF
8001CE6C: sra     $s1, $s5, 16                                             ; 03 8C 15 00
8001CE70: move    $a0, $s1                                                 ; 21 20 20 02
8001CE74: move    $a1, $zero                                               ; 21 28 00 00
8001CE78: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001CE80: move    $a3, $zero                                               ; 21 38 00 00
8001CE84: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CE88: addiu   $s4, 1                                                   ; 01 00 94 26
8001CE8C: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CE90: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CE94: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CE98: addu    $v0, $v1                                                 ; 21 10 43 00
8001CE9C: sll     $v0, 2                                                   ; 80 10 02 00
8001CEA0: addu    $v0, $s3                                                 ; 21 10 53 00
8001CEA4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CEA8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CEAC: sra     $s0, $s6, 16                                             ; 03 84 16 00
8001CEB0: move    $a0, $s0                                                 ; 21 20 00 02
8001CEB4: move    $a1, $zero                                               ; 21 28 00 00
8001CEB8: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001CEC0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CEC4: move    $a3, $zero                                               ; 21 38 00 00
8001CEC8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CECC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CED0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CED4: addu    $v0, $v1                                                 ; 21 10 43 00
8001CED8: sll     $v0, 2                                                   ; 80 10 02 00
8001CEDC: addu    $v0, $s3                                                 ; 21 10 53 00
8001CEE0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CEE4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CEE8: move    $a0, $s1                                                 ; 21 20 20 02
8001CEEC: li      $a1, 0xDC                                                ; DC 00 05 34
8001CEF0: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001CEF8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CEFC: move    $a3, $zero                                               ; 21 38 00 00
8001CF00: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CF04: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CF08: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CF0C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CF10: sll     $v0, 2                                                   ; 80 10 02 00
8001CF14: addu    $v0, $s3                                                 ; 21 10 53 00
8001CF18: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CF1C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CF20: move    $a0, $s0                                                 ; 21 20 00 02
8001CF24: li      $a1, 0xDC                                                ; DC 00 05 34
8001CF28: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001CF30: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CF34: move    $a3, $zero                                               ; 21 38 00 00
8001CF38: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CF3C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CF40: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CF44: addu    $v0, $v1                                                 ; 21 10 43 00
8001CF48: sll     $v0, 2                                                   ; 80 10 02 00
8001CF4C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CF50: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CF54: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CF58: move    $a0, $s1                                                 ; 21 20 20 02
8001CF5C: li      $a1, 0x14                                                ; 14 00 05 34
8001CF60: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001CF68: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CF6C: move    $a3, $zero                                               ; 21 38 00 00
8001CF70: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CF74: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CF78: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CF7C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CF80: sll     $v0, 2                                                   ; 80 10 02 00
8001CF84: addu    $v0, $s3                                                 ; 21 10 53 00
8001CF88: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CF8C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CF90: move    $a0, $s0                                                 ; 21 20 00 02
8001CF94: li      $a1, 0x14                                                ; 14 00 05 34
8001CF98: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001CFA0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CFA4: move    $a3, $zero                                               ; 21 38 00 00
8001CFA8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CFAC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CFB0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CFB4: addu    $v0, $v1                                                 ; 21 10 43 00
8001CFB8: sll     $v0, 2                                                   ; 80 10 02 00
8001CFBC: addu    $v0, $s3                                                 ; 21 10 53 00
8001CFC0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CFC4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CFC8: move    $a0, $s1                                                 ; 21 20 20 02
8001CFCC: li      $a1, 0xC8                                                ; C8 00 05 34
8001CFD0: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001CFD8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CFDC: move    $a3, $zero                                               ; 21 38 00 00
8001CFE0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CFE4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CFE8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CFEC: addu    $v0, $v1                                                 ; 21 10 43 00
8001CFF0: sll     $v0, 2                                                   ; 80 10 02 00
8001CFF4: addu    $v0, $s3                                                 ; 21 10 53 00
8001CFF8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CFFC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D000: move    $a0, $s0                                                 ; 21 20 00 02
8001D004: li      $a1, 0xC8                                                ; C8 00 05 34
8001D008: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001D010: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D014: move    $a3, $zero                                               ; 21 38 00 00
8001D018: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D01C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D020: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D024: addu    $v0, $v1                                                 ; 21 10 43 00
8001D028: sll     $v0, 2                                                   ; 80 10 02 00
8001D02C: addu    $v0, $s3                                                 ; 21 10 53 00
8001D030: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D034: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D038: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001D03C: addu    $s6, $v0                                                 ; 21 B0 C2 02
8001D040: addu    $s5, $v0                                                 ; 21 A8 A2 02
8001D044: slti    $v0, $s4, 7                                              ; 07 00 82 2A
8001D048: bnez    $v0, loc_8001CE70                                        ; 89 FF 40 14
8001D04C: sra     $s1, $s5, 16                                             ; 03 8C 15 00
8001D050: move    $s4, $zero                                               ; 21 A0 00 00
8001D054: li      $s6, unk_80087288                                        ; 08 80 16 3C 88 72 D6 26
8001D05C: move    $s5, $zero                                               ; 21 A8 00 00
8001D060: lui     $s3, 0x14                                                ; 14 00 13 3C
8001D064: move    $a0, $zero                                               ; 21 20 00 00
8001D068: sra     $s1, $s3, 16                                             ; 03 8C 13 00
8001D06C: move    $a1, $s1                                                 ; 21 28 20 02
8001D070: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001D078: move    $a3, $zero                                               ; 21 38 00 00
8001D07C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D080: addiu   $s4, 1                                                   ; 01 00 94 26
8001D084: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D088: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D08C: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D090: addu    $v0, $v1                                                 ; 21 10 43 00
8001D094: sll     $v0, 2                                                   ; 80 10 02 00
8001D098: addu    $v0, $s6                                                 ; 21 10 56 00
8001D09C: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D0A0: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D0A4: move    $a0, $zero                                               ; 21 20 00 00
8001D0A8: sra     $s0, $s5, 16                                             ; 03 84 15 00
8001D0AC: move    $a1, $s0                                                 ; 21 28 00 02
8001D0B0: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001D0B8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D0BC: move    $a3, $zero                                               ; 21 38 00 00
8001D0C0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D0C4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D0C8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D0CC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D0D0: sll     $v0, 2                                                   ; 80 10 02 00
8001D0D4: addu    $v0, $s6                                                 ; 21 10 56 00
8001D0D8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D0DC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D0E0: li      $a0, 0x12C                                               ; 2C 01 04 34
8001D0E4: move    $a1, $s1                                                 ; 21 28 20 02
8001D0E8: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001D0F0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D0F4: move    $a3, $zero                                               ; 21 38 00 00
8001D0F8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D0FC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D100: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D104: addu    $v0, $v1                                                 ; 21 10 43 00
8001D108: sll     $v0, 2                                                   ; 80 10 02 00
8001D10C: addu    $v0, $s6                                                 ; 21 10 56 00
8001D110: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D114: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D118: li      $a0, 0x12C                                               ; 2C 01 04 34
8001D11C: move    $a1, $s0                                                 ; 21 28 00 02
8001D120: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001D128: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D12C: move    $a3, $zero                                               ; 21 38 00 00
8001D130: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D134: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D138: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D13C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D140: sll     $v0, 2                                                   ; 80 10 02 00
8001D144: addu    $v0, $s6                                                 ; 21 10 56 00
8001D148: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D14C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D150: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001D154: addu    $s5, $v0                                                 ; 21 A8 A2 02
8001D158: addu    $s3, $v0                                                 ; 21 98 62 02
8001D15C: slti    $v0, $s4, 6                                              ; 06 00 82 2A
8001D160: bnez    $v0, loc_8001D068                                        ; C1 FF 40 14
8001D164: move    $a0, $zero                                               ; 21 20 00 00
8001D168: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001D16C: li      $a1, 0xB4                                                ; B4 00 05 34
8001D170: li      $a2, dword_8004EA60                                      ; 05 80 06 3C 60 EA C6 24
8001D178: move    $a3, $zero                                               ; 21 38 00 00
8001D17C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D180: li      $s0, unk_80087288                                        ; 08 80 10 3C 88 72 10 26
8001D188: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D18C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D190: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D194: addu    $v0, $v1                                                 ; 21 10 43 00
8001D198: sll     $v0, 2                                                   ; 80 10 02 00
8001D19C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D1A0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D1A4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D1A8: li      $a0, 0x3C  # '<'                                         ; 3C 00 04 34
8001D1AC: li      $a1, 0xB4                                                ; B4 00 05 34
8001D1B0: li      $a2, dword_8004EA70                                      ; 05 80 06 3C 70 EA C6 24
8001D1B8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D1BC: move    $a3, $zero                                               ; 21 38 00 00
8001D1C0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D1C4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D1C8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D1CC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D1D0: sll     $v0, 2                                                   ; 80 10 02 00
8001D1D4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D1D8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D1DC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D1E0: li      $a0, 0x50  # 'P'                                         ; 50 00 04 34
8001D1E4: li      $a1, 0xB4                                                ; B4 00 05 34
8001D1E8: li      $a2, dword_8004EA80                                      ; 05 80 06 3C 80 EA C6 24
8001D1F0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D1F4: move    $a3, $zero                                               ; 21 38 00 00
8001D1F8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D1FC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D200: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D204: addu    $v0, $v1                                                 ; 21 10 43 00
8001D208: sll     $v0, 2                                                   ; 80 10 02 00
8001D20C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D210: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D214: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D218: li      $a0, 0x64  # 'd'                                         ; 64 00 04 34
8001D21C: li      $a1, 0xB4                                                ; B4 00 05 34
8001D220: li      $a2, dword_8004EA90                                      ; 05 80 06 3C 90 EA C6 24
8001D228: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D22C: move    $a3, $zero                                               ; 21 38 00 00
8001D230: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D234: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D238: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D23C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D240: sll     $v0, 2                                                   ; 80 10 02 00
8001D244: addu    $v0, $s0                                                 ; 21 10 50 00
8001D248: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D24C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D250: li      $a0, 0x78  # 'x'                                         ; 78 00 04 34
8001D254: li      $a1, 0xB4                                                ; B4 00 05 34
8001D258: li      $a2, dword_8004EAA0                                      ; 05 80 06 3C A0 EA C6 24
8001D260: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D264: move    $a3, $zero                                               ; 21 38 00 00
8001D268: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D26C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D270: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D274: addu    $v0, $v1                                                 ; 21 10 43 00
8001D278: sll     $v0, 2                                                   ; 80 10 02 00
8001D27C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D280: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D284: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D288: li      $a0, 0x8C                                                ; 8C 00 04 34
8001D28C: li      $a1, 0xB4                                                ; B4 00 05 34
8001D290: li      $a2, dword_8004EAB0                                      ; 05 80 06 3C B0 EA C6 24
8001D298: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D29C: move    $a3, $zero                                               ; 21 38 00 00
8001D2A0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D2A4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D2A8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D2AC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D2B0: sll     $v0, 2                                                   ; 80 10 02 00
8001D2B4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D2B8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D2BC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D2C0: li      $a0, 0xA0                                                ; A0 00 04 34
8001D2C4: li      $a1, 0xB4                                                ; B4 00 05 34
8001D2C8: li      $a2, dword_8004EAC0                                      ; 05 80 06 3C C0 EA C6 24
8001D2D0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D2D4: move    $a3, $zero                                               ; 21 38 00 00
8001D2D8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D2DC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D2E0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D2E4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D2E8: sll     $v0, 2                                                   ; 80 10 02 00
8001D2EC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D2F0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D2F4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D2F8: li      $a0, 0xB4                                                ; B4 00 04 34
8001D2FC: li      $a1, 0xB4                                                ; B4 00 05 34
8001D300: li      $a2, dword_8004EAD0                                      ; 05 80 06 3C D0 EA C6 24
8001D308: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D30C: move    $a3, $zero                                               ; 21 38 00 00
8001D310: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D314: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D318: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D31C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D320: sll     $v0, 2                                                   ; 80 10 02 00
8001D324: addu    $v0, $s0                                                 ; 21 10 50 00
8001D328: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D32C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D330: li      $a0, 0xC8                                                ; C8 00 04 34
8001D334: li      $a1, 0xB4                                                ; B4 00 05 34
8001D338: li      $a2, dword_8004EAE0                                      ; 05 80 06 3C E0 EA C6 24
8001D340: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D344: move    $a3, $zero                                               ; 21 38 00 00
8001D348: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D34C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D350: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D354: addu    $v0, $v1                                                 ; 21 10 43 00
8001D358: sll     $v0, 2                                                   ; 80 10 02 00
8001D35C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D360: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D364: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D368: li      $a0, 0xDC                                                ; DC 00 04 34
8001D36C: li      $a1, 0xB4                                                ; B4 00 05 34
8001D370: li      $a2, dword_8004EAF0                                      ; 05 80 06 3C F0 EA C6 24
8001D378: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D37C: move    $a3, $zero                                               ; 21 38 00 00
8001D380: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D384: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D388: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D38C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D390: sll     $v0, 2                                                   ; 80 10 02 00
8001D394: addu    $v0, $s0                                                 ; 21 10 50 00
8001D398: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D39C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D3A0: li      $a0, 0xF0                                                ; F0 00 04 34
8001D3A4: li      $a1, 0xB4                                                ; B4 00 05 34
8001D3A8: li      $a2, dword_8004EB00                                      ; 05 80 06 3C 00 EB C6 24
8001D3B0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D3B4: move    $a3, $zero                                               ; 21 38 00 00
8001D3B8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D3BC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D3C0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D3C4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D3C8: sll     $v0, 2                                                   ; 80 10 02 00
8001D3CC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D3D0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D3D4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D3D8: li      $a0, 0x104                                               ; 04 01 04 34
8001D3DC: li      $a1, 0xB4                                                ; B4 00 05 34
8001D3E0: li      $a2, dword_8004EB10                                      ; 05 80 06 3C 10 EB C6 24
8001D3E8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D3EC: move    $a3, $zero                                               ; 21 38 00 00
8001D3F0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D3F4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D3F8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D3FC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D400: sll     $v0, 2                                                   ; 80 10 02 00
8001D404: addu    $v0, $s0                                                 ; 21 10 50 00
8001D408: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D40C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D410: li      $a0, 0x118                                               ; 18 01 04 34
8001D414: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D418: li      $a2, dword_8004E980                                      ; 05 80 06 3C 80 E9 C6 24
8001D420: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D424: move    $a3, $zero                                               ; 21 38 00 00
8001D428: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D42C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D430: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D434: addu    $v0, $v1                                                 ; 21 10 43 00
8001D438: sll     $v0, 2                                                   ; 80 10 02 00
8001D43C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D440: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D444: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D448: li      $a0, 0x14                                                ; 14 00 04 34
8001D44C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D450: li      $a2, dword_8004EA50                                      ; 05 80 06 3C 50 EA C6 24
8001D458: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D45C: move    $a3, $zero                                               ; 21 38 00 00
8001D460: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D464: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D468: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D46C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D470: sll     $v0, 2                                                   ; 80 10 02 00
8001D474: addu    $v0, $s0                                                 ; 21 10 50 00
8001D478: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D47C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D480: li      $a0, 0xA0                                                ; A0 00 04 34
8001D484: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D488: li      $a2, dword_8004E9F0                                      ; 05 80 06 3C F0 E9 C6 24
8001D490: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D494: move    $a3, $zero                                               ; 21 38 00 00
8001D498: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D49C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D4A0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D4A4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D4A8: sll     $v0, 2                                                   ; 80 10 02 00
8001D4AC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D4B0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D4B4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D4B8: li      $a0, 0x8C                                                ; 8C 00 04 34
8001D4BC: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D4C0: li      $a2, dword_8004E9E0                                      ; 05 80 06 3C E0 E9 C6 24
8001D4C8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D4CC: move    $a3, $zero                                               ; 21 38 00 00
8001D4D0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D4D4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D4D8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D4DC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D4E0: sll     $v0, 2                                                   ; 80 10 02 00
8001D4E4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D4E8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D4EC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D4F0: li      $a0, 0xB4                                                ; B4 00 04 34
8001D4F4: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D4F8: li      $a2, dword_8004EA00                                      ; 05 80 06 3C 00 EA C6 24
8001D500: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D504: move    $a3, $zero                                               ; 21 38 00 00
8001D508: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D50C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D510: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D514: addu    $v0, $v1                                                 ; 21 10 43 00
8001D518: sll     $v0, 2                                                   ; 80 10 02 00
8001D51C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D520: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D524: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D528: li      $a0, 0x78  # 'x'                                         ; 78 00 04 34
8001D52C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D530: li      $a2, dword_8004E9D0                                      ; 05 80 06 3C D0 E9 C6 24
8001D538: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D53C: move    $a3, $zero                                               ; 21 38 00 00
8001D540: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D544: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D548: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D54C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D550: sll     $v0, 2                                                   ; 80 10 02 00
8001D554: addu    $v0, $s0                                                 ; 21 10 50 00
8001D558: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D55C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D560: li      $a0, 0xC8                                                ; C8 00 04 34
8001D564: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D568: li      $a2, dword_8004EA10                                      ; 05 80 06 3C 10 EA C6 24
8001D570: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D574: move    $a3, $zero                                               ; 21 38 00 00
8001D578: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D57C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D580: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D584: addu    $v0, $v1                                                 ; 21 10 43 00
8001D588: sll     $v0, 2                                                   ; 80 10 02 00
8001D58C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D590: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D594: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D598: li      $a0, 0x64  # 'd'                                         ; 64 00 04 34
8001D59C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D5A0: li      $a2, dword_8004E9C0                                      ; 05 80 06 3C C0 E9 C6 24
8001D5A8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D5AC: move    $a3, $zero                                               ; 21 38 00 00
8001D5B0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D5B4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D5B8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D5BC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D5C0: sll     $v0, 2                                                   ; 80 10 02 00
8001D5C4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D5C8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D5CC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D5D0: li      $a0, 0xF0                                                ; F0 00 04 34
8001D5D4: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D5D8: li      $a2, dword_8004EA30                                      ; 05 80 06 3C 30 EA C6 24
8001D5E0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D5E4: move    $a3, $zero                                               ; 21 38 00 00
8001D5E8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D5EC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D5F0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D5F4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D5F8: sll     $v0, 2                                                   ; 80 10 02 00
8001D5FC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D600: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D604: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D608: li      $a0, 0x50  # 'P'                                         ; 50 00 04 34
8001D60C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D610: li      $a2, dword_8004E9B0                                      ; 05 80 06 3C B0 E9 C6 24
8001D618: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D61C: move    $a3, $zero                                               ; 21 38 00 00
8001D620: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D624: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D628: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D62C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D630: sll     $v0, 2                                                   ; 80 10 02 00
8001D634: addu    $v0, $s0                                                 ; 21 10 50 00
8001D638: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D63C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D640: li      $a0, 0xDC                                                ; DC 00 04 34
8001D644: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D648: li      $a2, dword_8004EA20                                      ; 05 80 06 3C 20 EA C6 24
8001D650: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D654: move    $a3, $zero                                               ; 21 38 00 00
8001D658: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D65C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D660: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D664: addu    $v0, $v1                                                 ; 21 10 43 00
8001D668: sll     $v0, 2                                                   ; 80 10 02 00
8001D66C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D670: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D674: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D678: li      $a0, 0x3C  # '<'                                         ; 3C 00 04 34
8001D67C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D680: li      $a2, dword_8004E9A0                                      ; 05 80 06 3C A0 E9 C6 24
8001D688: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D68C: move    $a3, $zero                                               ; 21 38 00 00
8001D690: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D694: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D698: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D69C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D6A0: sll     $v0, 2                                                   ; 80 10 02 00
8001D6A4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D6A8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D6AC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D6B0: li      $a0, 0x104                                               ; 04 01 04 34
8001D6B4: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D6B8: li      $a2, dword_8004EA40                                      ; 05 80 06 3C 40 EA C6 24
8001D6C0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D6C4: move    $a3, $zero                                               ; 21 38 00 00
8001D6C8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D6CC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D6D0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D6D4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D6D8: sll     $v0, 2                                                   ; 80 10 02 00
8001D6DC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D6E0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D6E4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D6E8: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001D6EC: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D6F0: li      $a2, dword_8004E990                                      ; 05 80 06 3C 90 E9 C6 24
8001D6F8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D6FC: move    $a3, $zero                                               ; 21 38 00 00
8001D700: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D704: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D708: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D70C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D710: sll     $v0, 2                                                   ; 80 10 02 00
8001D714: addu    $v0, $s0                                                 ; 21 10 50 00
8001D718: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D71C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D720: lw      $ra, 0x20+var_s1C($sp)                                   ; 3C 00 BF 8F
8001D724: lw      $s6, 0x20+var_s18($sp)                                   ; 38 00 B6 8F
8001D728: lw      $s5, 0x20+var_s14($sp)                                   ; 34 00 B5 8F
8001D72C: lw      $s4, 0x20+var_s10($sp)                                   ; 30 00 B4 8F
8001D730: lw      $s3, 0x20+var_sC($sp)                                    ; 2C 00 B3 8F
8001D734: lw      $s2, 0x20+var_s8($sp)                                    ; 28 00 B2 8F
8001D738: lw      $s1, 0x20+var_s4($sp)                                    ; 24 00 B1 8F
8001D73C: lw      $s0, 0x20+var_s0($sp)                                    ; 20 00 B0 8F
8001D740: addiu   $sp, 0x40                                                ; 40 00 BD 27
8001D744: jr      $ra                                                      ; 08 00 E0 03
8001D748: nop                                                              ; 00 00 00 00
```

## `80035510` `sub_80035510`
Bounds: `80035510-80035540`

### Pseudocode
```c
int sub_80035510()
{
  PAD_dr();
  return ~dword_800882F0;
}

```

### Calls in function
- 80035518: `jal     PAD_dr` -> 80048A00 PAD_dr

### Xrefs to
- 800157DC -> 80035510 type=17 func=sub_80015788 line=`jal     sub_80035510`
- 80015B9C -> 80035510 type=17 func=sub_80015B00 line=`jal     sub_80035510`
- 80016AF0 -> 80035510 type=17 func=sub_80016AB4 line=`jal     sub_80035510`
- 8001900C -> 80035510 type=17 func=sub_80018FB0 line=`jal     sub_80035510`
- 8002674C -> 80035510 type=17 func=sub_80026744 line=`jal     sub_80035510`
- 80026CA8 -> 80035510 type=17 func=sub_80026B94 line=`jal     sub_80035510`
- 80027AB0 -> 80035510 type=17 func=sub_8002776C line=`jal     sub_80035510`
- 80027EAC -> 80035510 type=17 func=sub_8002776C line=`jal     sub_80035510`
- 801C467C -> 80035510 type=17 func=sub_801C455C line=`jal     sub_80035510`
- 801C4B50 -> 80035510 type=17 func=sub_801C4894 line=`jal     sub_80035510`
- 801C4BC0 -> 80035510 type=17 func=sub_801C4894 line=`jal     sub_80035510`
- 801C4C58 -> 80035510 type=17 func=sub_801C4894 line=`jal     sub_80035510`
- 801C4CD4 -> 80035510 type=17 func=sub_801C4894 line=`jal     sub_80035510`

### Xrefs from
- 80035510 -> 80035514 type=21 target=sub_80035514 line=`addiu   $sp, -0x18`
- 80035514 -> 80035518 type=21 target=sub_80035518 line=`sw      $ra, 0x10+var_s0($sp)`
- 80035518 -> 8003551C type=21 target=sub_8003551C line=`jal     PAD_dr`
- 80035518 -> 80048A00 type=17 target=PAD_dr line=`jal     PAD_dr`
- 8003551C -> 80035520 type=21 target=sub_80035520 line=`nop`
- 80035520 -> 80035528 type=21 target=sub_80035528 line=`lw      $v0, dword_800882F0`
- 80035528 -> 8003552C type=21 target=sub_8003552C line=`nop`
- 8003552C -> 80035530 type=21 target=sub_80035530 line=`nor     $v0, $zero, $v0`
- 80035530 -> 80035534 type=21 target=sub_80035534 line=`lw      $ra, 0x10+var_s0($sp)`
- 80035534 -> 80035538 type=21 target=sub_80035538 line=`addiu   $sp, 0x18`
- 80035538 -> 8003553C type=21 target=sub_8003553C line=`jr      $ra`

### Full disasm
```asm
80035510: addiu   $sp, -0x18                                               ; E8 FF BD 27
80035514: sw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF AF
80035518: jal     PAD_dr                                                   ; 80 22 01 0C
8003551C: nop                                                              ; 00 00 00 00
80035520: lw      $v0, dword_800882F0                                      ; 09 80 02 3C F0 82 42 8C
80035528: nop                                                              ; 00 00 00 00
8003552C: nor     $v0, $zero, $v0                                          ; 27 10 02 00
80035530: lw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF 8F
80035534: addiu   $sp, 0x18                                                ; 18 00 BD 27
80035538: jr      $ra                                                      ; 08 00 E0 03
8003553C: nop                                                              ; 00 00 00 00
```

## `80040CA4` `sub_80040CA4`
Bounds: `80040CA4-80040CC8`

### Pseudocode
```c
int __fastcall sub_80040CA4(int a1)
{
  return sub_800450A0(*(_DWORD *)(a1 + 16));
}

```

### Calls in function
- 80040CB0: `jal     sub_800450A0` -> 800450A0 sub_800450A0

### Xrefs to
- 8001E3CC -> 80040CA4 type=17 func=sub_8001E3B0 line=`jal     sub_80040CA4`
- 8001EA58 -> 80040CA4 type=17 func=sub_8001EA00 line=`jal     sub_80040CA4`
- 8001EC3C -> 80040CA4 type=17 func=sub_8001EBF4 line=`jal     sub_80040CA4`
- 8001ED5C -> 80040CA4 type=17 func=sub_8001ED3C line=`jal     sub_80040CA4`
- 8001EE94 -> 80040CA4 type=17 func=sub_8001EE74 line=`jal     sub_80040CA4`
- 801C68F4 -> 80040CA4 type=17 func=sub_801C689C line=`jal     sub_80040CA4`

### Xrefs from
- 80040CA4 -> 80040CA8 type=21 target=sub_80040CA8 line=`addiu   $sp, -0x18`
- 80040CA8 -> 80040CAC type=21 target=sub_80040CAC line=`sw      $ra, 0x10+var_s0($sp)`
- 80040CAC -> 80040CB0 type=21 target=sub_80040CB0 line=`lw      $a0, 0x10($a0)`
- 80040CB0 -> 80040CB4 type=21 target=sub_80040CB4 line=`jal     sub_800450A0`
- 80040CB0 -> 800450A0 type=17 target=sub_800450A0 line=`jal     sub_800450A0`
- 80040CB4 -> 80040CB8 type=21 target=sub_80040CB8 line=`nop`
- 80040CB8 -> 80040CBC type=21 target=sub_80040CBC line=`lw      $ra, 0x10+var_s0($sp)`
- 80040CBC -> 80040CC0 type=21 target=sub_80040CC0 line=`addiu   $sp, 0x18`
- 80040CC0 -> 80040CC4 type=21 target=sub_80040CC4 line=`jr      $ra`

### Full disasm
```asm
80040CA4: addiu   $sp, -0x18                                               ; E8 FF BD 27
80040CA8: sw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF AF
80040CAC: lw      $a0, 0x10($a0)                                           ; 10 00 84 8C
80040CB0: jal     sub_800450A0                                             ; 28 14 01 0C
80040CB4: nop                                                              ; 00 00 00 00
80040CB8: lw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF 8F
80040CBC: addiu   $sp, 0x18                                                ; 18 00 BD 27
80040CC0: jr      $ra                                                      ; 08 00 E0 03
80040CC4: nop                                                              ; 00 00 00 00
```

## `80040370` `sub_80040370`
Bounds: `80040370-80040420`

### Pseudocode
```c
int sub_80040370()
{
  int v0; // $v0
  int v1; // $v0
  int v2; // $v1
  int v3; // $a0

  v0 = word_80096590;
  word_80091790 = word_8008ECA8[v0];
  word_80091792 = word_8008ECAC[v0];
  sub_800452EC();
  sub_80044AA0(1);
  v1 = dword_8009658C;
  v2 = ++dword_8009658C;
  v3 = 1;
  if ( v1 )
    v3 = v2;
  dword_8009658C = v3;
  word_80096590 = word_80096590 == 0;
  sub_800402E0();
  return sub_800401AC();
}

```

### Calls in function
- 800403B0: `jal     sub_800452EC` -> 800452EC sub_800452EC
- 800403B8: `jal     sub_80044AA0` -> 80044AA0 sub_80044AA0
- 800403D8: `beqz    $v0, loc_800403E4` -> 800403E4 loc_800403E4
- 80040400: `jal     sub_800402E0` -> 800402E0 sub_800402E0
- 80040408: `jal     sub_800401AC` -> 800401AC sub_800401AC

### Xrefs to
- 8001E554 -> 80040370 type=17 func=sub_8001E54C line=`jal     sub_80040370`
- 8001EA0C -> 80040370 type=17 func=sub_8001EA00 line=`jal     sub_80040370`
- 8001EBFC -> 80040370 type=17 func=sub_8001EBF4 line=`jal     sub_80040370`
- 8001ED7C -> 80040370 type=17 func=sub_8001ED74 line=`jal     sub_80040370`
- 801C4B7C -> 80040370 type=17 func=sub_801C4894 line=`jal     sub_80040370`
- 801C4B8C -> 80040370 type=17 func=sub_801C4894 line=`jal     sub_80040370`
- 801C68A4 -> 80040370 type=17 func=sub_801C689C line=`jal     sub_80040370`
- 801C6920 -> 80040370 type=17 func=sub_801C689C line=`jal     sub_80040370`

### Xrefs from
- 80040370 -> 80040378 type=21 target=sub_80040378 line=`lh      $v0, word_80096590`
- 80040378 -> 8004037C type=21 target=sub_8004037C line=`addiu   $sp, -0x18`
- 8004037C -> 80040380 type=21 target=sub_80040380 line=`sw      $ra, 0x10+var_s0($sp)`
- 80040380 -> 80040384 type=21 target=sub_80040384 line=`sll     $v0, 1`
- 80040384 -> 80040390 type=21 target=sub_80040390 line=`lhu     $v1, word_8008ECA8($v0)`
- 80040390 -> 80040398 type=21 target=sub_80040398 line=`li      $a0, word_80091790`
- 80040398 -> 8004039C type=21 target=sub_8004039C line=`sh      $v1, (word_80091790 - 0x80091790)($a0)`
- 8004039C -> 800403A8 type=21 target=sub_800403A8 line=`lhu     $v0, word_8008ECAC($v0)`
- 800403A8 -> 800403B0 type=21 target=sub_800403B0 line=`sh      $v0, word_80091792`
- 800403B0 -> 800403B4 type=21 target=sub_800403B4 line=`jal     sub_800452EC`
- 800403B0 -> 800452EC type=17 target=sub_800452EC line=`jal     sub_800452EC`
- 800403B4 -> 800403B8 type=21 target=sub_800403B8 line=`nop`
- 800403B8 -> 800403BC type=21 target=sub_800403BC line=`jal     sub_80044AA0`
- 800403B8 -> 80044AA0 type=17 target=sub_80044AA0 line=`jal     sub_80044AA0`
- 800403BC -> 800403C0 type=21 target=sub_800403C0 line=`li      $a0, 1`
- 800403C0 -> 800403C8 type=21 target=sub_800403C8 line=`lw      $v0, dword_8009658C`
- 800403C8 -> 800403CC type=21 target=sub_800403CC line=`nop`
- 800403CC -> 800403D0 type=21 target=sub_800403D0 line=`addiu   $v1, $v0, 1`
- 800403D0 -> 800403D8 type=21 target=sub_800403D8 line=`sw      $v1, dword_8009658C`
- 800403D8 -> 800403DC type=21 target=sub_800403DC line=`beqz    $v0, loc_800403E4`
- 800403D8 -> 800403E4 type=19 target=loc_800403E4 line=`beqz    $v0, loc_800403E4`
- 800403DC -> 800403E0 type=21 target=sub_800403E0 line=`li      $a0, 1`
- 800403E0 -> 800403E4 type=21 target=loc_800403E4 line=`move    $a0, $v1`
- 800403E4 -> 800403EC type=21 target=sub_800403EC line=`lh      $v0, word_80096590`
- 800403EC -> 800403F4 type=21 target=sub_800403F4 line=`sw      $a0, dword_8009658C`
- 800403F4 -> 800403F8 type=21 target=sub_800403F8 line=`sltiu   $v0, 1`
- 800403F8 -> 80040400 type=21 target=sub_80040400 line=`sh      $v0, word_80096590`
- 80040400 -> 80040404 type=21 target=sub_80040404 line=`jal     sub_800402E0`
- 80040400 -> 800402E0 type=17 target=sub_800402E0 line=`jal     sub_800402E0`
- 80040404 -> 80040408 type=21 target=sub_80040408 line=`nop`
- 80040408 -> 8004040C type=21 target=sub_8004040C line=`jal     sub_800401AC`
- 80040408 -> 800401AC type=17 target=sub_800401AC line=`jal     sub_800401AC`
- 8004040C -> 80040410 type=21 target=sub_80040410 line=`nop`
- 80040410 -> 80040414 type=21 target=sub_80040414 line=`lw      $ra, 0x10+var_s0($sp)`
- 80040414 -> 80040418 type=21 target=sub_80040418 line=`addiu   $sp, 0x18`
- 80040418 -> 8004041C type=21 target=sub_8004041C line=`jr      $ra`

### Full disasm
```asm
80040370: lh      $v0, word_80096590                                       ; 09 80 02 3C 90 65 42 84
80040378: addiu   $sp, -0x18                                               ; E8 FF BD 27
8004037C: sw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF AF
80040380: sll     $v0, 1                                                   ; 40 10 02 00
80040384: lhu     $v1, word_8008ECA8($v0)                                  ; 09 80 01 3C 21 08 22 00 A8 EC 23 94
80040390: li      $a0, word_80091790                                       ; 09 80 04 3C 90 17 84 24
80040398: sh      $v1, (word_80091790 - 0x80091790)($a0)                   ; 00 00 83 A4
8004039C: lhu     $v0, word_8008ECAC($v0)                                  ; 09 80 01 3C 21 08 22 00 AC EC 22 94
800403A8: sh      $v0, word_80091792                                       ; 09 80 01 3C 92 17 22 A4
800403B0: jal     sub_800452EC                                             ; BB 14 01 0C
800403B4: nop                                                              ; 00 00 00 00
800403B8: jal     sub_80044AA0                                             ; A8 12 01 0C
800403BC: li      $a0, 1                                                   ; 01 00 04 34
800403C0: lw      $v0, dword_8009658C                                      ; 09 80 02 3C 8C 65 42 8C
800403C8: nop                                                              ; 00 00 00 00
800403CC: addiu   $v1, $v0, 1                                              ; 01 00 43 24
800403D0: sw      $v1, dword_8009658C                                      ; 09 80 01 3C 8C 65 23 AC
800403D8: beqz    $v0, loc_800403E4                                        ; 02 00 40 10
800403DC: li      $a0, 1                                                   ; 01 00 04 34
800403E0: move    $a0, $v1                                                 ; 21 20 60 00
800403E4: lh      $v0, word_80096590                                       ; 09 80 02 3C 90 65 42 84
800403EC: sw      $a0, dword_8009658C                                      ; 09 80 01 3C 8C 65 24 AC
800403F4: sltiu   $v0, 1                                                   ; 01 00 42 2C
800403F8: sh      $v0, word_80096590                                       ; 09 80 01 3C 90 65 22 A4
80040400: jal     sub_800402E0                                             ; B8 00 01 0C
80040404: nop                                                              ; 00 00 00 00
80040408: jal     sub_800401AC                                             ; 6B 00 01 0C
8004040C: nop                                                              ; 00 00 00 00
80040410: lw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF 8F
80040414: addiu   $sp, 0x18                                                ; 18 00 BD 27
80040418: jr      $ra                                                      ; 08 00 E0 03
8004041C: nop                                                              ; 00 00 00 00
```



[console]
# IDA export: Stage1 Movie1 entry / skip transition audit

Generated: 2026-05-26T16:54:06.732354
Input IDB target: SCUS_941.83 current IDA session
Scope: Pro-requested evidence for Initial Movie1 pre-transition visibility and story Movie1 skip freeze/black frame. No runtime patch in this file.

## `801C4DC4` `sub_801C4DC4`
Bounds: `801C4DC4-801C4F68`

### Pseudocode
```c
int __fastcall sub_801C4DC4(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int (*a9)(void))
{
  int v10; // $a0
  int v11; // $s0
  int result; // $v0

  if ( !word_800916D2 )
  {
    word_800916D2 = 1;
    sub_80020110(&unk_801C3640, 2, 1, 2);
  }
  sub_800201AC(&unk_801C3640, 6, 1, 2);
  sub_801C44E0(108, 0);
  sub_801C455C(108, &unk_801C3640, 0);
  sub_8001B120(1);
  word_800916D2 = 1;
  sub_800201AC(&unk_801C3640, 5, 1, 2);
  v11 = sub_801C4894(156, (int)&unk_801C3640);
  sub_80026FA4(v10);
  sub_80020110(&unk_801C3640, 2, 1, 2);
  sub_8001EF14();
  if ( v11 == 1 )
  {
    word_800916D0 = 0;
    return -1;
  }
  else if ( v11 == 3 )
  {
    word_800916D0 = 1;
    do
      result = rand() % 6 + 1;
    while ( result == word_800916EE );
  }
  else
  {
    sub_80026EF8(dword_80094410);
    sub_80026ECC();
    result = a1 + 1;
    word_800916D0 = 0;
  }
  return result;
}

```

### Calls in function
- 801C4DF0: `bnez    $v0, loc_801C4E14` -> 801C4E14 loc_801C4E14
- 801C4E0C: `jal     sub_80020110` -> 80020110 sub_80020110
- 801C4E20: `jal     sub_800201AC` -> 800201AC sub_800201AC
- 801C4E34: `jal     sub_801C44E0` -> 801C44E0 sub_801C44E0
- 801C4E4C: `jal     sub_801C455C` -> 801C455C sub_801C455C
- 801C4E54: `jal     sub_8001B120` -> 8001B120 sub_8001B120
- 801C4E70: `jal     sub_800201AC` -> 800201AC sub_800201AC
- 801C4E84: `jal     sub_801C4894` -> 801C4894 sub_801C4894
- 801C4E8C: `jal     sub_80026FA4` -> 80026FA4 sub_80026FA4
- 801C4EA0: `jal     sub_80020110` -> 80020110 sub_80020110
- 801C4EA8: `jal     sub_8001EF14` -> 8001EF14 sub_8001EF14
- 801C4EB4: `bne     $s0, $v0, loc_801C4ECC` -> 801C4ECC loc_801C4ECC
- 801C4EC4: `j       loc_801C4F48` -> 801C4F48 loc_801C4F48
- 801C4ECC: `bne     $s0, $v0, loc_801C4F24` -> 801C4F24 loc_801C4F24
- 801C4EE0: `jal     rand` -> 80047FDC rand
- 801C4F14: `beq     $v0, $v1, loc_801C4EE0` -> 801C4EE0 loc_801C4EE0
- 801C4F1C: `j       loc_801C4F48` -> 801C4F48 loc_801C4F48
- 801C4F2C: `jal     sub_80026EF8` -> 80026EF8 sub_80026EF8
- 801C4F34: `jal     sub_80026ECC` -> 80026ECC sub_80026ECC

### Xrefs to
- 80048D30 -> 801C4DC4 type=1 func=<no func> line=`.word sub_801C4DC4`

### Xrefs from
- 801C4DC4 -> 801C4DC8 type=21 target=sub_801C4DC8 line=`addiu   $sp, -0x28`
- 801C4DC8 -> 801C4DCC type=21 target=sub_801C4DCC line=`sw      $s3, 0x10+var_sC($sp)`
- 801C4DCC -> 801C4DD0 type=21 target=sub_801C4DD0 line=`sw      $s0, 0x10+var_s0($sp)`
- 801C4DD0 -> 801C4DD8 type=21 target=sub_801C4DD8 line=`li      $s0, word_800916D2`
- 801C4DD8 -> 801C4DDC type=21 target=sub_801C4DDC line=`sw      $ra, 0x10+var_s10($sp)`
- 801C4DDC -> 801C4DE0 type=21 target=sub_801C4DE0 line=`sw      $s2, 0x10+var_s8($sp)`
- 801C4DE0 -> 801C4DE4 type=21 target=sub_801C4DE4 line=`sw      $s1, 0x10+var_s4($sp)`
- 801C4DE4 -> 801C4DE8 type=21 target=sub_801C4DE8 line=`lh      $v0, (word_800916D2 - 0x800916D2)($s0)`
- 801C4DE8 -> 801C4DF0 type=21 target=sub_801C4DF0 line=`li      $s1, unk_801C3640`
- 801C4DF0 -> 801C4DF4 type=21 target=sub_801C4DF4 line=`bnez    $v0, loc_801C4E14`
- 801C4DF0 -> 801C4E14 type=19 target=loc_801C4E14 line=`bnez    $v0, loc_801C4E14`
- 801C4DF4 -> 801C4DF8 type=21 target=sub_801C4DF8 line=`move    $s3, $a0`
- 801C4DF8 -> 801C4DFC type=21 target=sub_801C4DFC line=`li      $v0, 1`
- 801C4DFC -> 801C4E00 type=21 target=sub_801C4E00 line=`sh      $v0, (word_800916D2 - 0x800916D2)($s0)`
- 801C4E00 -> 801C4E04 type=21 target=sub_801C4E04 line=`move    $a0, $s1`
- 801C4E04 -> 801C4E08 type=21 target=sub_801C4E08 line=`li      $a1, 2`
- 801C4E08 -> 801C4E0C type=21 target=sub_801C4E0C line=`li      $a2, 1`
- 801C4E0C -> 801C4E10 type=21 target=sub_801C4E10 line=`jal     sub_80020110`
- 801C4E0C -> 80020110 type=17 target=sub_80020110 line=`jal     sub_80020110`
- 801C4E10 -> 801C4E14 type=21 target=loc_801C4E14 line=`li      $a3, 2`
- 801C4E14 -> 801C4E18 type=21 target=sub_801C4E18 line=`move    $a0, $s1`
- 801C4E18 -> 801C4E1C type=21 target=sub_801C4E1C line=`li      $a1, 6`
- 801C4E1C -> 801C4E20 type=21 target=sub_801C4E20 line=`li      $a2, 1`
- 801C4E20 -> 801C4E24 type=21 target=sub_801C4E24 line=`jal     sub_800201AC`
- 801C4E20 -> 800201AC type=17 target=sub_800201AC line=`jal     sub_800201AC`
- 801C4E24 -> 801C4E28 type=21 target=sub_801C4E28 line=`li      $a3, 2`
- 801C4E28 -> 801C4E30 type=21 target=sub_801C4E30 line=`lw      $a0, dword_8006ED98+0x20`
- 801C4E30 -> 801C4E34 type=21 target=sub_801C4E34 line=`move    $a1, $zero`
- 801C4E34 -> 801C4E38 type=21 target=sub_801C4E38 line=`jal     sub_801C44E0`
- 801C4E34 -> 801C44E0 type=17 target=sub_801C44E0 line=`jal     sub_801C44E0`
- 801C4E38 -> 801C4E3C type=21 target=sub_801C4E3C line=`addiu   $a0, 0x6C  # 'l'`
- 801C4E3C -> 801C4E40 type=21 target=sub_801C4E40 line=`move    $a1, $s1`
- 801C4E40 -> 801C4E48 type=21 target=sub_801C4E48 line=`lw      $a0, dword_8006ED98+0x20`
- 801C4E48 -> 801C4E4C type=21 target=sub_801C4E4C line=`move    $a2, $zero`
- 801C4E4C -> 801C4E50 type=21 target=sub_801C4E50 line=`jal     sub_801C455C`
- 801C4E4C -> 801C455C type=17 target=sub_801C455C line=`jal     sub_801C455C`
- 801C4E50 -> 801C4E54 type=21 target=sub_801C4E54 line=`addiu   $a0, 0x6C  # 'l'`
- 801C4E54 -> 801C4E58 type=21 target=sub_801C4E58 line=`jal     sub_8001B120`
- 801C4E54 -> 8001B120 type=17 target=sub_8001B120 line=`jal     sub_8001B120`
- 801C4E58 -> 801C4E5C type=21 target=sub_801C4E5C line=`li      $a0, 1`
- 801C4E5C -> 801C4E60 type=21 target=sub_801C4E60 line=`move    $a0, $s1`
- 801C4E60 -> 801C4E64 type=21 target=sub_801C4E64 line=`li      $a1, 5`
- 801C4E64 -> 801C4E68 type=21 target=sub_801C4E68 line=`li      $a2, 1`
- 801C4E68 -> 801C4E6C type=21 target=sub_801C4E6C line=`li      $a3, 2`
- 801C4E6C -> 801C4E70 type=21 target=sub_801C4E70 line=`li      $s2, 1`
- 801C4E70 -> 801C4E74 type=21 target=sub_801C4E74 line=`jal     sub_800201AC`
- 801C4E70 -> 800201AC type=17 target=sub_800201AC line=`jal     sub_800201AC`
- 801C4E74 -> 801C4E78 type=21 target=sub_801C4E78 line=`sh      $s2, (word_800916D2 - 0x800916D2)($s0)`
- 801C4E78 -> 801C4E80 type=21 target=sub_801C4E80 line=`lw      $a0, dword_8006ED98+0x20`
- 801C4E80 -> 801C4E84 type=21 target=sub_801C4E84 line=`move    $a1, $s1`
- 801C4E84 -> 801C4E88 type=21 target=sub_801C4E88 line=`jal     sub_801C4894`
- 801C4E84 -> 801C4894 type=17 target=sub_801C4894 line=`jal     sub_801C4894`
- 801C4E88 -> 801C4E8C type=21 target=sub_801C4E8C line=`addiu   $a0, 0x9C`
- 801C4E8C -> 801C4E90 type=21 target=sub_801C4E90 line=`jal     sub_80026FA4`
- 801C4E8C -> 80026FA4 type=17 target=sub_80026FA4 line=`jal     sub_80026FA4`
- 801C4E90 -> 801C4E94 type=21 target=sub_801C4E94 line=`move    $s0, $v0`
- 801C4E94 -> 801C4E98 type=21 target=sub_801C4E98 line=`move    $a0, $s1`
- 801C4E98 -> 801C4E9C type=21 target=sub_801C4E9C line=`li      $a1, 2`
- 801C4E9C -> 801C4EA0 type=21 target=sub_801C4EA0 line=`li      $a2, 1`
- 801C4EA0 -> 801C4EA4 type=21 target=sub_801C4EA4 line=`jal     sub_80020110`
- 801C4EA0 -> 80020110 type=17 target=sub_80020110 line=`jal     sub_80020110`
- 801C4EA4 -> 801C4EA8 type=21 target=sub_801C4EA8 line=`li      $a3, 2`
- 801C4EA8 -> 801C4EAC type=21 target=sub_801C4EAC line=`jal     sub_8001EF14`
- 801C4EA8 -> 8001EF14 type=17 target=sub_8001EF14 line=`jal     sub_8001EF14`
- 801C4EAC -> 801C4EB0 type=21 target=sub_801C4EB0 line=`nop`
- 801C4EB0 -> 801C4EB4 type=21 target=sub_801C4EB4 line=`li      $v0, 1`
- 801C4EB4 -> 801C4EB8 type=21 target=sub_801C4EB8 line=`bne     $s0, $v0, loc_801C4ECC`
- 801C4EB4 -> 801C4ECC type=19 target=loc_801C4ECC line=`bne     $s0, $v0, loc_801C4ECC`
- 801C4EB8 -> 801C4EBC type=21 target=sub_801C4EBC line=`li      $v0, 3`
- 801C4EBC -> 801C4EC4 type=21 target=sub_801C4EC4 line=`sh      $zero, word_800916D0`
- 801C4EC4 -> 801C4EC8 type=21 target=sub_801C4EC8 line=`j       loc_801C4F48`
- 801C4EC4 -> 801C4F48 type=19 target=loc_801C4F48 line=`j       loc_801C4F48`
- 801C4ECC -> 801C4ED0 type=21 target=sub_801C4ED0 line=`bne     $s0, $v0, loc_801C4F24`
- 801C4ECC -> 801C4F24 type=19 target=loc_801C4F24 line=`bne     $s0, $v0, loc_801C4F24`
- 801C4ED0 -> 801C4ED4 type=21 target=sub_801C4ED4 line=`lui     $s0, 0x2AAA`
- 801C4ED4 -> 801C4EDC type=21 target=sub_801C4EDC line=`sh      $s2, word_800916D0`
- 801C4EDC -> 801C4EE0 type=21 target=loc_801C4EE0 line=`li      $s0, 0x2AAAAAAB`
- 801C4EE0 -> 801C4EE4 type=21 target=sub_801C4EE4 line=`jal     rand`
- 801C4EE0 -> 80047FDC type=17 target=rand line=`jal     rand`
- 801C4EE4 -> 801C4EE8 type=21 target=sub_801C4EE8 line=`nop`
- 801C4EE8 -> 801C4EEC type=21 target=sub_801C4EEC line=`mult    $v0, $s0`
- 801C4EEC -> 801C4EF0 type=21 target=sub_801C4EF0 line=`sra     $a0, $v0, 31`
- 801C4EF0 -> 801C4EF4 type=21 target=sub_801C4EF4 line=`mfhi    $t0`
- 801C4EF4 -> 801C4EF8 type=21 target=sub_801C4EF8 line=`subu    $a0, $t0, $a0`
- 801C4EF8 -> 801C4EFC type=21 target=sub_801C4EFC line=`sll     $v1, $a0, 1`
- 801C4EFC -> 801C4F00 type=21 target=sub_801C4F00 line=`addu    $v1, $a0`
- 801C4F00 -> 801C4F04 type=21 target=sub_801C4F04 line=`sll     $v1, 1`
- 801C4F04 -> 801C4F08 type=21 target=sub_801C4F08 line=`subu    $v0, $v1`
- 801C4F08 -> 801C4F10 type=21 target=sub_801C4F10 line=`lh      $v1, word_800916EE`
- 801C4F10 -> 801C4F14 type=21 target=sub_801C4F14 line=`addiu   $v0, 1`
- 801C4F14 -> 801C4F18 type=21 target=sub_801C4F18 line=`beq     $v0, $v1, loc_801C4EE0`
- 801C4F14 -> 801C4EE0 type=19 target=loc_801C4EE0 line=`beq     $v0, $v1, loc_801C4EE0`
- 801C4F18 -> 801C4F1C type=21 target=sub_801C4F1C line=`nop`
- 801C4F1C -> 801C4F20 type=21 target=sub_801C4F20 line=`j       loc_801C4F48`
- 801C4F1C -> 801C4F48 type=19 target=loc_801C4F48 line=`j       loc_801C4F48`
- 801C4F24 -> 801C4F2C type=21 target=sub_801C4F2C line=`lw      $a0, dword_80094410`
- 801C4F2C -> 801C4F30 type=21 target=sub_801C4F30 line=`jal     sub_80026EF8`
- 801C4F2C -> 80026EF8 type=17 target=sub_80026EF8 line=`jal     sub_80026EF8`
- 801C4F30 -> 801C4F34 type=21 target=sub_801C4F34 line=`nop`
- 801C4F34 -> 801C4F38 type=21 target=sub_801C4F38 line=`jal     sub_80026ECC`
- 801C4F34 -> 80026ECC type=17 target=sub_80026ECC line=`jal     sub_80026ECC`
- 801C4F38 -> 801C4F3C type=21 target=sub_801C4F3C line=`nop`
- 801C4F3C -> 801C4F40 type=21 target=sub_801C4F40 line=`addiu   $v0, $s3, 1`
- 801C4F40 -> 801C4F48 type=21 target=loc_801C4F48 line=`sh      $zero, word_800916D0`
- 801C4F48 -> 801C4F4C type=21 target=sub_801C4F4C line=`lw      $ra, 0x10+var_s10($sp)`
- 801C4F4C -> 801C4F50 type=21 target=sub_801C4F50 line=`lw      $s3, 0x10+var_sC($sp)`
- 801C4F50 -> 801C4F54 type=21 target=sub_801C4F54 line=`lw      $s2, 0x10+var_s8($sp)`
- 801C4F54 -> 801C4F58 type=21 target=sub_801C4F58 line=`lw      $s1, 0x10+var_s4($sp)`
- 801C4F58 -> 801C4F5C type=21 target=sub_801C4F5C line=`lw      $s0, 0x10+var_s0($sp)`
- 801C4F5C -> 801C4F60 type=21 target=sub_801C4F60 line=`addiu   $sp, 0x28`
- 801C4F60 -> 801C4F64 type=21 target=sub_801C4F64 line=`jr      $ra`

### Full disasm
```asm
801C4DC4: addiu   $sp, -0x28                                               ; D8 FF BD 27
801C4DC8: sw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 AF
801C4DCC: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
801C4DD0: li      $s0, word_800916D2                                       ; 09 80 10 3C D2 16 10 26
801C4DD8: sw      $ra, 0x10+var_s10($sp)                                   ; 20 00 BF AF
801C4DDC: sw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 AF
801C4DE0: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
801C4DE4: lh      $v0, (word_800916D2 - 0x800916D2)($s0)                   ; 00 00 02 86
801C4DE8: li      $s1, unk_801C3640                                        ; 1C 80 11 3C 40 36 31 26
801C4DF0: bnez    $v0, loc_801C4E14                                        ; 08 00 40 14
801C4DF4: move    $s3, $a0                                                 ; 21 98 80 00
801C4DF8: li      $v0, 1                                                   ; 01 00 02 34
801C4DFC: sh      $v0, (word_800916D2 - 0x800916D2)($s0)                   ; 00 00 02 A6
801C4E00: move    $a0, $s1                                                 ; 21 20 20 02
801C4E04: li      $a1, 2                                                   ; 02 00 05 34
801C4E08: li      $a2, 1                                                   ; 01 00 06 34
801C4E0C: jal     sub_80020110                                             ; 44 80 00 0C
801C4E10: li      $a3, 2                                                   ; 02 00 07 34
801C4E14: move    $a0, $s1                                                 ; 21 20 20 02
801C4E18: li      $a1, 6                                                   ; 06 00 05 34
801C4E1C: li      $a2, 1                                                   ; 01 00 06 34
801C4E20: jal     sub_800201AC                                             ; 6B 80 00 0C
801C4E24: li      $a3, 2                                                   ; 02 00 07 34
801C4E28: lw      $a0, dword_8006ED98+0x20                                 ; 07 80 04 3C B8 ED 84 8C
801C4E30: move    $a1, $zero                                               ; 21 28 00 00
801C4E34: jal     sub_801C44E0                                             ; 38 11 07 0C
801C4E38: addiu   $a0, 0x6C  # 'l'                                         ; 6C 00 84 24
801C4E3C: move    $a1, $s1                                                 ; 21 28 20 02
801C4E40: lw      $a0, dword_8006ED98+0x20                                 ; 07 80 04 3C B8 ED 84 8C
801C4E48: move    $a2, $zero                                               ; 21 30 00 00
801C4E4C: jal     sub_801C455C                                             ; 57 11 07 0C
801C4E50: addiu   $a0, 0x6C  # 'l'                                         ; 6C 00 84 24
801C4E54: jal     sub_8001B120                                             ; 48 6C 00 0C
801C4E58: li      $a0, 1                                                   ; 01 00 04 34
801C4E5C: move    $a0, $s1                                                 ; 21 20 20 02
801C4E60: li      $a1, 5                                                   ; 05 00 05 34
801C4E64: li      $a2, 1                                                   ; 01 00 06 34
801C4E68: li      $a3, 2                                                   ; 02 00 07 34
801C4E6C: li      $s2, 1                                                   ; 01 00 12 34
801C4E70: jal     sub_800201AC                                             ; 6B 80 00 0C
801C4E74: sh      $s2, (word_800916D2 - 0x800916D2)($s0)                   ; 00 00 12 A6
801C4E78: lw      $a0, dword_8006ED98+0x20                                 ; 07 80 04 3C B8 ED 84 8C
801C4E80: move    $a1, $s1                                                 ; 21 28 20 02
801C4E84: jal     sub_801C4894                                             ; 25 12 07 0C
801C4E88: addiu   $a0, 0x9C                                                ; 9C 00 84 24
801C4E8C: jal     sub_80026FA4                                             ; E9 9B 00 0C
801C4E90: move    $s0, $v0                                                 ; 21 80 40 00
801C4E94: move    $a0, $s1                                                 ; 21 20 20 02
801C4E98: li      $a1, 2                                                   ; 02 00 05 34
801C4E9C: li      $a2, 1                                                   ; 01 00 06 34
801C4EA0: jal     sub_80020110                                             ; 44 80 00 0C
801C4EA4: li      $a3, 2                                                   ; 02 00 07 34
801C4EA8: jal     sub_8001EF14                                             ; C5 7B 00 0C
801C4EAC: nop                                                              ; 00 00 00 00
801C4EB0: li      $v0, 1                                                   ; 01 00 02 34
801C4EB4: bne     $s0, $v0, loc_801C4ECC                                   ; 05 00 02 16
801C4EB8: li      $v0, 3                                                   ; 03 00 02 34
801C4EBC: sh      $zero, word_800916D0                                     ; 09 80 01 3C D0 16 20 A4
801C4EC4: j       loc_801C4F48                                             ; D2 13 07 08
801C4EC8: li      $v0, 0xFFFFFFFF                                          ; FF FF 02 24
801C4ECC: bne     $s0, $v0, loc_801C4F24                                   ; 15 00 02 16
801C4ED0: lui     $s0, 0x2AAA                                              ; AA 2A 10 3C
801C4ED4: sh      $s2, word_800916D0                                       ; 09 80 01 3C D0 16 32 A4
801C4EDC: li      $s0, 0x2AAAAAAB                                          ; AB AA 10 36
801C4EE0: jal     rand                                                     ; F7 1F 01 0C
801C4EE4: nop                                                              ; 00 00 00 00
801C4EE8: mult    $v0, $s0                                                 ; 18 00 50 00
801C4EEC: sra     $a0, $v0, 31                                             ; C3 27 02 00
801C4EF0: mfhi    $t0                                                      ; 10 40 00 00
801C4EF4: subu    $a0, $t0, $a0                                            ; 23 20 04 01
801C4EF8: sll     $v1, $a0, 1                                              ; 40 18 04 00
801C4EFC: addu    $v1, $a0                                                 ; 21 18 64 00
801C4F00: sll     $v1, 1                                                   ; 40 18 03 00
801C4F04: subu    $v0, $v1                                                 ; 23 10 43 00
801C4F08: lh      $v1, word_800916EE                                       ; 09 80 03 3C EE 16 63 84
801C4F10: addiu   $v0, 1                                                   ; 01 00 42 24
801C4F14: beq     $v0, $v1, loc_801C4EE0                                   ; F2 FF 43 10
801C4F18: nop                                                              ; 00 00 00 00
801C4F1C: j       loc_801C4F48                                             ; D2 13 07 08
801C4F20: nop                                                              ; 00 00 00 00
801C4F24: lw      $a0, dword_80094410                                      ; 09 80 04 3C 10 44 84 8C
801C4F2C: jal     sub_80026EF8                                             ; BE 9B 00 0C
801C4F30: nop                                                              ; 00 00 00 00
801C4F34: jal     sub_80026ECC                                             ; B3 9B 00 0C
801C4F38: nop                                                              ; 00 00 00 00
801C4F3C: addiu   $v0, $s3, 1                                              ; 01 00 62 26
801C4F40: sh      $zero, word_800916D0                                     ; 09 80 01 3C D0 16 20 A4
801C4F48: lw      $ra, 0x10+var_s10($sp)                                   ; 20 00 BF 8F
801C4F4C: lw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 8F
801C4F50: lw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 8F
801C4F54: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
801C4F58: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
801C4F5C: addiu   $sp, 0x28                                                ; 28 00 BD 27
801C4F60: jr      $ra                                                      ; 08 00 E0 03
801C4F64: nop                                                              ; 00 00 00 00
```

## `801C44E0` `sub_801C44E0`
Bounds: `801C44E0-801C455C`

### Pseudocode
```c
int __fastcall sub_801C44E0(int a1, int a2)
{
  int v4; // $a0

  sub_80026FA4(a1);
  nullsub_8();
  sub_80024E98();
  v4 = *(__int16 *)(a1 + 6);
  dword_801C9554 = a2 != 1;
  sub_8001A478(v4);
  sub_80027288(a2);
  sub_8001A4D0(a1, 1);
  return sub_800274D4();
}

```

### Calls in function
- 801C44F4: `jal     sub_80026FA4` -> 80026FA4 sub_80026FA4
- 801C44FC: `jal     nullsub_8` -> 801C6094 nullsub_8
- 801C4504: `jal     sub_80024E98` -> 80024E98 sub_80024E98
- 801C4520: `jal     sub_8001A478` -> 8001A478 sub_8001A478
- 801C4528: `jal     sub_80027288` -> 80027288 sub_80027288
- 801C4534: `jal     sub_8001A4D0` -> 8001A4D0 sub_8001A4D0
- 801C453C: `jal     sub_800274D4` -> 800274D4 sub_800274D4

### Xrefs to
- 801C48F4 -> 801C44E0 type=17 func=sub_801C4894 line=`jal     sub_801C44E0`
- 801C4E34 -> 801C44E0 type=17 func=sub_801C4DC4 line=`jal     sub_801C44E0`

### Xrefs from
- 801C44E0 -> 801C44E4 type=21 target=sub_801C44E4 line=`addiu   $sp, -0x20`
- 801C44E4 -> 801C44E8 type=21 target=sub_801C44E8 line=`sw      $s1, 0x10+var_s4($sp)`
- 801C44E8 -> 801C44EC type=21 target=sub_801C44EC line=`move    $s1, $a0`
- 801C44EC -> 801C44F0 type=21 target=sub_801C44F0 line=`sw      $s0, 0x10+var_s0($sp)`
- 801C44F0 -> 801C44F4 type=21 target=sub_801C44F4 line=`sw      $ra, 0x10+var_s8($sp)`
- 801C44F4 -> 801C44F8 type=21 target=sub_801C44F8 line=`jal     sub_80026FA4`
- 801C44F4 -> 80026FA4 type=17 target=sub_80026FA4 line=`jal     sub_80026FA4`
- 801C44F8 -> 801C44FC type=21 target=sub_801C44FC line=`move    $s0, $a1`
- 801C44FC -> 801C4500 type=21 target=sub_801C4500 line=`jal     nullsub_8`
- 801C44FC -> 801C6094 type=17 target=nullsub_8 line=`jal     nullsub_8`
- 801C4500 -> 801C4504 type=21 target=sub_801C4504 line=`nop`
- 801C4504 -> 801C4508 type=21 target=sub_801C4508 line=`jal     sub_80024E98`
- 801C4504 -> 80024E98 type=17 target=sub_80024E98 line=`jal     sub_80024E98`
- 801C4508 -> 801C450C type=21 target=sub_801C450C line=`nop`
- 801C450C -> 801C4510 type=21 target=sub_801C4510 line=`xori    $v0, $s0, 1`
- 801C4510 -> 801C4514 type=21 target=sub_801C4514 line=`lh      $a0, 6($s1)`
- 801C4514 -> 801C4518 type=21 target=sub_801C4518 line=`sltu    $v0, $zero, $v0`
- 801C4518 -> 801C4520 type=21 target=sub_801C4520 line=`sw      $v0, dword_801C9554`
- 801C4520 -> 801C4524 type=21 target=sub_801C4524 line=`jal     sub_8001A478`
- 801C4520 -> 8001A478 type=17 target=sub_8001A478 line=`jal     sub_8001A478`
- 801C4524 -> 801C4528 type=21 target=sub_801C4528 line=`nop`
- 801C4528 -> 801C452C type=21 target=sub_801C452C line=`jal     sub_80027288`
- 801C4528 -> 80027288 type=17 target=sub_80027288 line=`jal     sub_80027288`
- 801C452C -> 801C4530 type=21 target=sub_801C4530 line=`move    $a0, $s0`
- 801C4530 -> 801C4534 type=21 target=sub_801C4534 line=`move    $a0, $s1`
- 801C4534 -> 801C4538 type=21 target=sub_801C4538 line=`jal     sub_8001A4D0`
- 801C4534 -> 8001A4D0 type=17 target=sub_8001A4D0 line=`jal     sub_8001A4D0`
- 801C4538 -> 801C453C type=21 target=sub_801C453C line=`li      $a1, 1`
- 801C453C -> 801C4540 type=21 target=sub_801C4540 line=`jal     sub_800274D4`
- 801C453C -> 800274D4 type=17 target=sub_800274D4 line=`jal     sub_800274D4`
- 801C4540 -> 801C4544 type=21 target=sub_801C4544 line=`nop`
- 801C4544 -> 801C4548 type=21 target=sub_801C4548 line=`lw      $ra, 0x10+var_s8($sp)`
- 801C4548 -> 801C454C type=21 target=sub_801C454C line=`lw      $s1, 0x10+var_s4($sp)`
- 801C454C -> 801C4550 type=21 target=sub_801C4550 line=`lw      $s0, 0x10+var_s0($sp)`
- 801C4550 -> 801C4554 type=21 target=sub_801C4554 line=`addiu   $sp, 0x20`
- 801C4554 -> 801C4558 type=21 target=sub_801C4558 line=`jr      $ra`

### Full disasm
```asm
801C44E0: addiu   $sp, -0x20                                               ; E0 FF BD 27
801C44E4: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
801C44E8: move    $s1, $a0                                                 ; 21 88 80 00
801C44EC: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
801C44F0: sw      $ra, 0x10+var_s8($sp)                                    ; 18 00 BF AF
801C44F4: jal     sub_80026FA4                                             ; E9 9B 00 0C
801C44F8: move    $s0, $a1                                                 ; 21 80 A0 00
801C44FC: jal     nullsub_8                                                ; 25 18 07 0C
801C4500: nop                                                              ; 00 00 00 00
801C4504: jal     sub_80024E98                                             ; A6 93 00 0C
801C4508: nop                                                              ; 00 00 00 00
801C450C: xori    $v0, $s0, 1                                              ; 01 00 02 3A
801C4510: lh      $a0, 6($s1)                                              ; 06 00 24 86
801C4514: sltu    $v0, $zero, $v0                                          ; 2B 10 02 00
801C4518: sw      $v0, dword_801C9554                                      ; 1D 80 01 3C 54 95 22 AC
801C4520: jal     sub_8001A478                                             ; 1E 69 00 0C
801C4524: nop                                                              ; 00 00 00 00
801C4528: jal     sub_80027288                                             ; A2 9C 00 0C
801C452C: move    $a0, $s0                                                 ; 21 20 00 02
801C4530: move    $a0, $s1                                                 ; 21 20 20 02
801C4534: jal     sub_8001A4D0                                             ; 34 69 00 0C
801C4538: li      $a1, 1                                                   ; 01 00 05 34
801C453C: jal     sub_800274D4                                             ; 35 9D 00 0C
801C4540: nop                                                              ; 00 00 00 00
801C4544: lw      $ra, 0x10+var_s8($sp)                                    ; 18 00 BF 8F
801C4548: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
801C454C: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
801C4550: addiu   $sp, 0x20                                                ; 20 00 BD 27
801C4554: jr      $ra                                                      ; 08 00 E0 03
801C4558: nop                                                              ; 00 00 00 00
```

## `801C455C` `sub_801C455C`
Bounds: `801C455C-801C4780`

### Pseudocode
```c
int __fastcall sub_801C455C(int a1, PrGameContext_Psx *a2, unsigned int a3)
{
  int *v5; // $a0
  int v6; // $s3
  int v7; // $s1
  __int16 v8; // $v0
  int v9; // $v1
  BOOL v10; // $s1

  if ( a3 >= 4 )
    v5 = 0;
  else
    v5 = (int *)(dword_800943CC + 28 * a3);
  v7 = 1800;
  sub_80024C84(v5);
  dword_801C954C = 0;
  do
  {
    if ( sub_8001A750() == 1 )
      break;
    if ( sub_8001EC54 )
      sub_8001EC54(a2, 7);
    sub_80035560(2);
    if ( sub_8001ED3C )
      sub_8001ED3C();
    --v7;
  }
  while ( v7 > 0 );
  v6 = 0;
  sub_8001A280();
  *(_DWORD *)&a2->_pad0[12] = MEMORY[0x164];
  *(_WORD *)&a2->_pad0[102] = word_800916D8;
  v8 = word_800916DC;
  *(_WORD *)&a2->_pad0[104] = word_800916DC;
  *(_WORD *)&a2->_pad0[84] = v8;
  while ( 1 )
  {
    *(_DWORD *)a2->_pad0 = 0;
    v9 = sub_80035510();
    if ( v9 )
    {
      if ( v9 == 256 )
      {
        v6 = 1;
        goto LABEL_24;
      }
      if ( v9 == 2048 || (v9 & 0x840) != 0 )
        goto LABEL_24;
    }
    sub_80024CF8(a2);
    sub_80027528();
    if ( dword_801C9554 == 1 )
      sub_8001EC54(a2, 7);
    sub_80027528();
    v10 = sub_801C448C(a1, (int)a2);
    sub_8001ED74();
    sub_8002756C();
    if ( dword_801C9554 == 1 )
      sub_8001ED3C();
    if ( GsGetWorkBase() == 1 )
      break;
    if ( !v10 )
      goto LABEL_24;
  }
  sub_8001A694();
  StopCallback();
LABEL_24:
  sub_80027664();
  sub_8001A4A4(1);
  sub_8001A694();
  nullsub_4(a2);
  sub_8001B120(1);
  return v6;
}

```

### Calls in function
- 801C4584: `beqz    $v0, loc_801C45C8` -> 801C45C8 loc_801C45C8
- 801C45A0: `j       loc_801C45CC` -> 801C45CC loc_801C45CC
- 801C45A8: `j       loc_801C472C` -> 801C472C loc_801C472C
- 801C45B0: `jal     sub_8001A694` -> 8001A694 sub_8001A694
- 801C45B8: `jal     StopCallback` -> 80035838 StopCallback
- 801C45C0: `j       loc_801C472C` -> 801C472C loc_801C472C
- 801C45CC: `jal     sub_80024C84` -> 80024C84 sub_80024C84
- 801C45F0: `jal     sub_8001A750` -> 8001A750 sub_8001A750
- 801C45F8: `beq     $v0, $s2, loc_801C4634` -> 801C4634 loc_801C4634
- 801C4600: `beqz    $s4, loc_801C4610` -> 801C4610 loc_801C4610
- 801C4608: `jal     sub_8001EC54` -> 8001EC54 sub_8001EC54
- 801C4610: `jal     sub_80035560` -> 80035560 sub_80035560
- 801C4618: `beqz    $s3, loc_801C4628` -> 801C4628 loc_801C4628
- 801C4620: `jal     sub_8001ED3C` -> 8001ED3C sub_8001ED3C
- 801C462C: `bgtz    $s1, loc_801C45F0` -> 801C45F0 loc_801C45F0
- 801C4634: `jal     sub_8001A280` -> 8001A280 sub_8001A280
- 801C467C: `jal     sub_80035510` -> 80035510 sub_80035510
- 801C4688: `beqz    $v1, loc_801C46A8` -> 801C46A8 loc_801C46A8
- 801C4690: `beq     $v1, $v0, loc_801C45A8` -> 801C45A8 loc_801C45A8
- 801C4698: `beq     $v1, $v0, loc_801C472C` -> 801C472C loc_801C472C
- 801C46A0: `bnez    $v0, loc_801C472C` -> 801C472C loc_801C472C
- 801C46A8: `jal     sub_80024CF8` -> 80024CF8 sub_80024CF8
- 801C46B0: `jal     sub_80027528` -> 80027528 sub_80027528
- 801C46C4: `bne     $v0, $s2, loc_801C46D4` -> 801C46D4 loc_801C46D4
- 801C46CC: `jal     sub_8001EC54` -> 8001EC54 sub_8001EC54
- 801C46D4: `jal     sub_80027528` -> 80027528 sub_80027528
- 801C46E0: `jal     sub_801C448C` -> 801C448C sub_801C448C
- 801C46E8: `jal     sub_8001ED74` -> 8001ED74 sub_8001ED74
- 801C46F0: `jal     sub_8002756C` -> 8002756C sub_8002756C
- 801C4704: `bne     $v0, $s2, loc_801C4714` -> 801C4714 loc_801C4714
- 801C470C: `jal     sub_8001ED3C` -> 8001ED3C sub_8001ED3C
- 801C4714: `jal     GsGetWorkBase` -> 8001A3B8 GsGetWorkBase
- 801C471C: `beq     $v0, $s2, loc_801C45B0` -> 801C45B0 loc_801C45B0
- 801C4724: `beq     $s1, $s2, loc_801C4678` -> 801C4678 loc_801C4678
- 801C472C: `jal     sub_80027664` -> 80027664 sub_80027664
- 801C4734: `jal     sub_8001A4A4` -> 8001A4A4 sub_8001A4A4
- 801C473C: `jal     sub_8001A694` -> 8001A694 sub_8001A694
- 801C4744: `jal     nullsub_4` -> 80024CF0 nullsub_4
- 801C474C: `jal     sub_8001B120` -> 8001B120 sub_8001B120

### Xrefs to
- 801C4E4C -> 801C455C type=17 func=sub_801C4DC4 line=`jal     sub_801C455C`

### Xrefs from
- 801C455C -> 801C4560 type=21 target=sub_801C4560 line=`addiu   $sp, -0x30`
- 801C4560 -> 801C4564 type=21 target=sub_801C4564 line=`sw      $s5, 0x10+var_s14($sp)`
- 801C4564 -> 801C4568 type=21 target=sub_801C4568 line=`move    $s5, $a0`
- 801C4568 -> 801C456C type=21 target=sub_801C456C line=`sw      $s0, 0x10+var_s0($sp)`
- 801C456C -> 801C4570 type=21 target=sub_801C4570 line=`move    $s0, $a1`
- 801C4570 -> 801C4574 type=21 target=sub_801C4574 line=`sltiu   $v0, $a2, 4`
- 801C4574 -> 801C4578 type=21 target=sub_801C4578 line=`sw      $ra, 0x10+var_s18($sp)`
- 801C4578 -> 801C457C type=21 target=sub_801C457C line=`sw      $s4, 0x10+var_s10($sp)`
- 801C457C -> 801C4580 type=21 target=sub_801C4580 line=`sw      $s3, 0x10+var_sC($sp)`
- 801C4580 -> 801C4584 type=21 target=sub_801C4584 line=`sw      $s2, 0x10+var_s8($sp)`
- 801C4584 -> 801C4588 type=21 target=sub_801C4588 line=`beqz    $v0, loc_801C45C8`
- 801C4584 -> 801C45C8 type=19 target=loc_801C45C8 line=`beqz    $v0, loc_801C45C8`
- 801C4588 -> 801C458C type=21 target=sub_801C458C line=`sw      $s1, 0x10+var_s4($sp)`
- 801C458C -> 801C4590 type=21 target=sub_801C4590 line=`sll     $a0, $a2, 3`
- 801C4590 -> 801C4594 type=21 target=sub_801C4594 line=`subu    $a0, $a2`
- 801C4594 -> 801C459C type=21 target=sub_801C459C line=`lw      $v0, dword_800943CC`
- 801C459C -> 801C45A0 type=21 target=sub_801C45A0 line=`sll     $a0, 2`
- 801C45A0 -> 801C45A4 type=21 target=sub_801C45A4 line=`j       loc_801C45CC`
- 801C45A0 -> 801C45CC type=19 target=loc_801C45CC line=`j       loc_801C45CC`
- 801C45A8 -> 801C45AC type=21 target=sub_801C45AC line=`j       loc_801C472C`
- 801C45A8 -> 801C472C type=19 target=loc_801C472C line=`j       loc_801C472C`
- 801C45B0 -> 801C45B4 type=21 target=sub_801C45B4 line=`jal     sub_8001A694`
- 801C45B0 -> 8001A694 type=17 target=sub_8001A694 line=`jal     sub_8001A694`
- 801C45B4 -> 801C45B8 type=21 target=sub_801C45B8 line=`nop`
- 801C45B8 -> 801C45BC type=21 target=sub_801C45BC line=`jal     StopCallback`
- 801C45B8 -> 80035838 type=17 target=StopCallback line=`jal     StopCallback`
- 801C45BC -> 801C45C0 type=21 target=sub_801C45C0 line=`nop`
- 801C45C0 -> 801C45C4 type=21 target=sub_801C45C4 line=`j       loc_801C472C`
- 801C45C0 -> 801C472C type=19 target=loc_801C472C line=`j       loc_801C472C`
- 801C45C8 -> 801C45CC type=21 target=loc_801C45CC line=`move    $a0, $zero`
- 801C45CC -> 801C45D0 type=21 target=sub_801C45D0 line=`jal     sub_80024C84`
- 801C45CC -> 80024C84 type=17 target=sub_80024C84 line=`jal     sub_80024C84`
- 801C45D0 -> 801C45D4 type=21 target=sub_801C45D4 line=`li      $s1, 0x708`
- 801C45D4 -> 801C45DC type=21 target=sub_801C45DC line=`li      $s4, sub_8001EC54`
- 801C45D4 -> 8001EC54 type=1 target=sub_8001EC54 line=`li      $s4, sub_8001EC54`
- 801C45DC -> 801C45E4 type=21 target=sub_801C45E4 line=`li      $s3, sub_8001ED3C`
- 801C45DC -> 8001ED3C type=1 target=sub_8001ED3C line=`li      $s3, sub_8001ED3C`
- 801C45E4 -> 801C45EC type=21 target=sub_801C45EC line=`sw      $zero, dword_801C954C`
- 801C45EC -> 801C45F0 type=21 target=loc_801C45F0 line=`li      $s2, 1`
- 801C45F0 -> 801C45F4 type=21 target=sub_801C45F4 line=`jal     sub_8001A750`
- 801C45F0 -> 8001A750 type=17 target=sub_8001A750 line=`jal     sub_8001A750`
- 801C45F4 -> 801C45F8 type=21 target=sub_801C45F8 line=`nop`
- 801C45F8 -> 801C45FC type=21 target=sub_801C45FC line=`beq     $v0, $s2, loc_801C4634`
- 801C45F8 -> 801C4634 type=19 target=loc_801C4634 line=`beq     $v0, $s2, loc_801C4634`
- 801C45FC -> 801C4600 type=21 target=sub_801C4600 line=`nop`
- 801C4600 -> 801C4604 type=21 target=sub_801C4604 line=`beqz    $s4, loc_801C4610`
- 801C4600 -> 801C4610 type=19 target=loc_801C4610 line=`beqz    $s4, loc_801C4610`
- 801C4604 -> 801C4608 type=21 target=sub_801C4608 line=`move    $a0, $s0`
- 801C4608 -> 801C460C type=21 target=sub_801C460C line=`jal     sub_8001EC54`
- 801C4608 -> 8001EC54 type=17 target=sub_8001EC54 line=`jal     sub_8001EC54`
- 801C460C -> 801C4610 type=21 target=loc_801C4610 line=`li      $a1, 7`
- 801C4610 -> 801C4614 type=21 target=sub_801C4614 line=`jal     sub_80035560`
- 801C4610 -> 80035560 type=17 target=sub_80035560 line=`jal     sub_80035560`
- 801C4614 -> 801C4618 type=21 target=sub_801C4618 line=`li      $a0, 2`
- 801C4618 -> 801C461C type=21 target=sub_801C461C line=`beqz    $s3, loc_801C4628`
- 801C4618 -> 801C4628 type=19 target=loc_801C4628 line=`beqz    $s3, loc_801C4628`
- 801C461C -> 801C4620 type=21 target=sub_801C4620 line=`nop`
- 801C4620 -> 801C4624 type=21 target=sub_801C4624 line=`jal     sub_8001ED3C`
- 801C4620 -> 8001ED3C type=17 target=sub_8001ED3C line=`jal     sub_8001ED3C`
- 801C4624 -> 801C4628 type=21 target=loc_801C4628 line=`move    $a0, $s0`
- 801C4628 -> 801C462C type=21 target=sub_801C462C line=`addiu   $s1, -1`
- 801C462C -> 801C4630 type=21 target=sub_801C4630 line=`bgtz    $s1, loc_801C45F0`
- 801C462C -> 801C45F0 type=19 target=loc_801C45F0 line=`bgtz    $s1, loc_801C45F0`
- 801C4630 -> 801C4634 type=21 target=loc_801C4634 line=`nop`
- 801C4634 -> 801C4638 type=21 target=sub_801C4638 line=`jal     sub_8001A280`
- 801C4634 -> 8001A280 type=17 target=sub_8001A280 line=`jal     sub_8001A280`
- 801C4638 -> 801C463C type=21 target=sub_801C463C line=`move    $s3, $zero`
- 801C463C -> 801C4644 type=21 target=sub_801C4644 line=`lw      $v0, dword_8006ED98+0x20`
- 801C4644 -> 801C4648 type=21 target=sub_801C4648 line=`nop`
- 801C4648 -> 801C464C type=21 target=sub_801C464C line=`lw      $v0, 0x164($v0)`
- 801C464C -> 801C4650 type=21 target=sub_801C4650 line=`nop`
- 801C4650 -> 801C4654 type=21 target=sub_801C4654 line=`sw      $v0, 0xC($s0)`
- 801C4654 -> 801C465C type=21 target=sub_801C465C line=`lhu     $v0, word_800916D8`
- 801C465C -> 801C4660 type=21 target=sub_801C4660 line=`nop`
- 801C4660 -> 801C4664 type=21 target=sub_801C4664 line=`sh      $v0, 0x66($s0)`
- 801C4664 -> 801C466C type=21 target=sub_801C466C line=`lhu     $v0, word_800916DC`
- 801C466C -> 801C4670 type=21 target=sub_801C4670 line=`li      $s2, 1`
- 801C4670 -> 801C4674 type=21 target=sub_801C4674 line=`sh      $v0, 0x68($s0)`
- 801C4674 -> 801C4678 type=21 target=loc_801C4678 line=`sh      $v0, 0x54($s0)`
- 801C4678 -> 801C467C type=21 target=sub_801C467C line=`sw      $zero, 0($s0)`
- 801C467C -> 801C4680 type=21 target=sub_801C4680 line=`jal     sub_80035510`
- 801C467C -> 80035510 type=17 target=sub_80035510 line=`jal     sub_80035510`
- 801C4680 -> 801C4684 type=21 target=sub_801C4684 line=`li      $a0, 1`
- 801C4684 -> 801C4688 type=21 target=sub_801C4688 line=`move    $v1, $v0`
- 801C4688 -> 801C468C type=21 target=sub_801C468C line=`beqz    $v1, loc_801C46A8`
- 801C4688 -> 801C46A8 type=19 target=loc_801C46A8 line=`beqz    $v1, loc_801C46A8`
- 801C468C -> 801C4690 type=21 target=sub_801C4690 line=`li      $v0, 0x100`
- 801C4690 -> 801C4694 type=21 target=sub_801C4694 line=`beq     $v1, $v0, loc_801C45A8`
- 801C4690 -> 801C45A8 type=19 target=loc_801C45A8 line=`beq     $v1, $v0, loc_801C45A8`
- 801C4694 -> 801C4698 type=21 target=sub_801C4698 line=`li      $v0, 0x800`
- 801C4698 -> 801C469C type=21 target=sub_801C469C line=`beq     $v1, $v0, loc_801C472C`
- 801C4698 -> 801C472C type=19 target=loc_801C472C line=`beq     $v1, $v0, loc_801C472C`
- 801C469C -> 801C46A0 type=21 target=sub_801C46A0 line=`andi    $v0, $v1, 0x840`
- 801C46A0 -> 801C46A4 type=21 target=sub_801C46A4 line=`bnez    $v0, loc_801C472C`
- 801C46A0 -> 801C472C type=19 target=loc_801C472C line=`bnez    $v0, loc_801C472C`
- 801C46A4 -> 801C46A8 type=21 target=loc_801C46A8 line=`nop`
- 801C46A8 -> 801C46AC type=21 target=sub_801C46AC line=`jal     sub_80024CF8`
- 801C46A8 -> 80024CF8 type=17 target=sub_80024CF8 line=`jal     sub_80024CF8`
- 801C46AC -> 801C46B0 type=21 target=sub_801C46B0 line=`move    $a0, $s0`
- 801C46B0 -> 801C46B4 type=21 target=sub_801C46B4 line=`jal     sub_80027528`
- 801C46B0 -> 80027528 type=17 target=sub_80027528 line=`jal     sub_80027528`
- 801C46B4 -> 801C46B8 type=21 target=sub_801C46B8 line=`nop`
- 801C46B8 -> 801C46C0 type=21 target=sub_801C46C0 line=`lw      $v0, dword_801C9554`
- 801C46C0 -> 801C46C4 type=21 target=sub_801C46C4 line=`nop`
- 801C46C4 -> 801C46C8 type=21 target=sub_801C46C8 line=`bne     $v0, $s2, loc_801C46D4`
- 801C46C4 -> 801C46D4 type=19 target=loc_801C46D4 line=`bne     $v0, $s2, loc_801C46D4`
- 801C46C8 -> 801C46CC type=21 target=sub_801C46CC line=`move    $a0, $s0`
- 801C46CC -> 801C46D0 type=21 target=sub_801C46D0 line=`jal     sub_8001EC54`
- 801C46CC -> 8001EC54 type=17 target=sub_8001EC54 line=`jal     sub_8001EC54`
- 801C46D0 -> 801C46D4 type=21 target=loc_801C46D4 line=`li      $a1, 7`
- 801C46D4 -> 801C46D8 type=21 target=sub_801C46D8 line=`jal     sub_80027528`
- 801C46D4 -> 80027528 type=17 target=sub_80027528 line=`jal     sub_80027528`
- 801C46D8 -> 801C46DC type=21 target=sub_801C46DC line=`nop`
- 801C46DC -> 801C46E0 type=21 target=sub_801C46E0 line=`move    $a0, $s5`
- 801C46E0 -> 801C46E4 type=21 target=sub_801C46E4 line=`jal     sub_801C448C`
- 801C46E0 -> 801C448C type=17 target=sub_801C448C line=`jal     sub_801C448C`
- 801C46E4 -> 801C46E8 type=21 target=sub_801C46E8 line=`move    $a1, $s0`
- 801C46E8 -> 801C46EC type=21 target=sub_801C46EC line=`jal     sub_8001ED74`
- 801C46E8 -> 8001ED74 type=17 target=sub_8001ED74 line=`jal     sub_8001ED74`
- 801C46EC -> 801C46F0 type=21 target=sub_801C46F0 line=`move    $s1, $v0`
- 801C46F0 -> 801C46F4 type=21 target=sub_801C46F4 line=`jal     sub_8002756C`
- 801C46F0 -> 8002756C type=17 target=sub_8002756C line=`jal     sub_8002756C`
- 801C46F4 -> 801C46F8 type=21 target=sub_801C46F8 line=`nop`
- 801C46F8 -> 801C4700 type=21 target=sub_801C4700 line=`lw      $v0, dword_801C9554`
- 801C4700 -> 801C4704 type=21 target=sub_801C4704 line=`nop`
- 801C4704 -> 801C4708 type=21 target=sub_801C4708 line=`bne     $v0, $s2, loc_801C4714`
- 801C4704 -> 801C4714 type=19 target=loc_801C4714 line=`bne     $v0, $s2, loc_801C4714`
- 801C4708 -> 801C470C type=21 target=sub_801C470C line=`nop`
- 801C470C -> 801C4710 type=21 target=sub_801C4710 line=`jal     sub_8001ED3C`
- 801C470C -> 8001ED3C type=17 target=sub_8001ED3C line=`jal     sub_8001ED3C`
- 801C4710 -> 801C4714 type=21 target=loc_801C4714 line=`move    $a0, $s0`
- 801C4714 -> 801C4718 type=21 target=sub_801C4718 line=`jal     GsGetWorkBase`
- 801C4714 -> 8001A3B8 type=17 target=GsGetWorkBase line=`jal     GsGetWorkBase`
- 801C4718 -> 801C471C type=21 target=sub_801C471C line=`nop`
- 801C471C -> 801C4720 type=21 target=sub_801C4720 line=`beq     $v0, $s2, loc_801C45B0`
- 801C471C -> 801C45B0 type=19 target=loc_801C45B0 line=`beq     $v0, $s2, loc_801C45B0`
- 801C4720 -> 801C4724 type=21 target=sub_801C4724 line=`nop`
- 801C4724 -> 801C4728 type=21 target=sub_801C4728 line=`beq     $s1, $s2, loc_801C4678`
- 801C4724 -> 801C4678 type=19 target=loc_801C4678 line=`beq     $s1, $s2, loc_801C4678`
- 801C4728 -> 801C472C type=21 target=loc_801C472C line=`nop`
- 801C472C -> 801C4730 type=21 target=sub_801C4730 line=`jal     sub_80027664`
- 801C472C -> 80027664 type=17 target=sub_80027664 line=`jal     sub_80027664`
- 801C4730 -> 801C4734 type=21 target=sub_801C4734 line=`nop`
- 801C4734 -> 801C4738 type=21 target=sub_801C4738 line=`jal     sub_8001A4A4`
- 801C4734 -> 8001A4A4 type=17 target=sub_8001A4A4 line=`jal     sub_8001A4A4`
- 801C4738 -> 801C473C type=21 target=sub_801C473C line=`li      $a0, 1`
- 801C473C -> 801C4740 type=21 target=sub_801C4740 line=`jal     sub_8001A694`
- 801C473C -> 8001A694 type=17 target=sub_8001A694 line=`jal     sub_8001A694`
- 801C4740 -> 801C4744 type=21 target=sub_801C4744 line=`nop`
- 801C4744 -> 801C4748 type=21 target=sub_801C4748 line=`jal     nullsub_4`
- 801C4744 -> 80024CF0 type=17 target=nullsub_4 line=`jal     nullsub_4`
- 801C4748 -> 801C474C type=21 target=sub_801C474C line=`move    $a0, $s0`
- 801C474C -> 801C4750 type=21 target=sub_801C4750 line=`jal     sub_8001B120`
- 801C474C -> 8001B120 type=17 target=sub_8001B120 line=`jal     sub_8001B120`
- 801C4750 -> 801C4754 type=21 target=sub_801C4754 line=`li      $a0, 1`
- 801C4754 -> 801C4758 type=21 target=sub_801C4758 line=`move    $v0, $s3`
- 801C4758 -> 801C475C type=21 target=sub_801C475C line=`lw      $ra, 0x10+var_s18($sp)`
- 801C475C -> 801C4760 type=21 target=sub_801C4760 line=`lw      $s5, 0x10+var_s14($sp)`
- 801C4760 -> 801C4764 type=21 target=sub_801C4764 line=`lw      $s4, 0x10+var_s10($sp)`
- 801C4764 -> 801C4768 type=21 target=sub_801C4768 line=`lw      $s3, 0x10+var_sC($sp)`
- 801C4768 -> 801C476C type=21 target=sub_801C476C line=`lw      $s2, 0x10+var_s8($sp)`
- 801C476C -> 801C4770 type=21 target=sub_801C4770 line=`lw      $s1, 0x10+var_s4($sp)`
- 801C4770 -> 801C4774 type=21 target=sub_801C4774 line=`lw      $s0, 0x10+var_s0($sp)`
- 801C4774 -> 801C4778 type=21 target=sub_801C4778 line=`addiu   $sp, 0x30`
- 801C4778 -> 801C477C type=21 target=sub_801C477C line=`jr      $ra`

### Full disasm
```asm
801C455C: addiu   $sp, -0x30                                               ; D0 FF BD 27
801C4560: sw      $s5, 0x10+var_s14($sp)                                   ; 24 00 B5 AF
801C4564: move    $s5, $a0                                                 ; 21 A8 80 00
801C4568: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
801C456C: move    $s0, $a1                                                 ; 21 80 A0 00
801C4570: sltiu   $v0, $a2, 4                                              ; 04 00 C2 2C
801C4574: sw      $ra, 0x10+var_s18($sp)                                   ; 28 00 BF AF
801C4578: sw      $s4, 0x10+var_s10($sp)                                   ; 20 00 B4 AF
801C457C: sw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 AF
801C4580: sw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 AF
801C4584: beqz    $v0, loc_801C45C8                                        ; 10 00 40 10
801C4588: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
801C458C: sll     $a0, $a2, 3                                              ; C0 20 06 00
801C4590: subu    $a0, $a2                                                 ; 23 20 86 00
801C4594: lw      $v0, dword_800943CC                                      ; 09 80 02 3C CC 43 42 8C
801C459C: sll     $a0, 2                                                   ; 80 20 04 00
801C45A0: j       loc_801C45CC                                             ; 73 11 07 08
801C45A4: addu    $a0, $v0, $a0                                            ; 21 20 44 00
801C45A8: j       loc_801C472C                                             ; CB 11 07 08
801C45AC: li      $s3, 1                                                   ; 01 00 13 34
801C45B0: jal     sub_8001A694                                             ; A5 69 00 0C
801C45B4: nop                                                              ; 00 00 00 00
801C45B8: jal     StopCallback                                             ; 0E D6 00 0C
801C45BC: nop                                                              ; 00 00 00 00
801C45C0: j       loc_801C472C                                             ; CB 11 07 08
801C45C4: nop                                                              ; 00 00 00 00
801C45C8: move    $a0, $zero                                               ; 21 20 00 00
801C45CC: jal     sub_80024C84                                             ; 21 93 00 0C
801C45D0: li      $s1, 0x708                                               ; 08 07 11 34
801C45D4: li      $s4, sub_8001EC54                                        ; 02 80 14 3C 54 EC 94 26
801C45DC: li      $s3, sub_8001ED3C                                        ; 02 80 13 3C 3C ED 73 26
801C45E4: sw      $zero, dword_801C954C                                    ; 1D 80 01 3C 4C 95 20 AC
801C45EC: li      $s2, 1                                                   ; 01 00 12 34
801C45F0: jal     sub_8001A750                                             ; D4 69 00 0C
801C45F4: nop                                                              ; 00 00 00 00
801C45F8: beq     $v0, $s2, loc_801C4634                                   ; 0E 00 52 10
801C45FC: nop                                                              ; 00 00 00 00
801C4600: beqz    $s4, loc_801C4610                                        ; 03 00 80 12
801C4604: move    $a0, $s0                                                 ; 21 20 00 02
801C4608: jal     sub_8001EC54                                             ; 15 7B 00 0C
801C460C: li      $a1, 7                                                   ; 07 00 05 34
801C4610: jal     sub_80035560                                             ; 58 D5 00 0C
801C4614: li      $a0, 2                                                   ; 02 00 04 34
801C4618: beqz    $s3, loc_801C4628                                        ; 03 00 60 12
801C461C: nop                                                              ; 00 00 00 00
801C4620: jal     sub_8001ED3C                                             ; 4F 7B 00 0C
801C4624: move    $a0, $s0                                                 ; 21 20 00 02
801C4628: addiu   $s1, -1                                                  ; FF FF 31 26
801C462C: bgtz    $s1, loc_801C45F0                                        ; F0 FF 20 1E
801C4630: nop                                                              ; 00 00 00 00
801C4634: jal     sub_8001A280                                             ; A0 68 00 0C
801C4638: move    $s3, $zero                                               ; 21 98 00 00
801C463C: lw      $v0, dword_8006ED98+0x20                                 ; 07 80 02 3C B8 ED 42 8C
801C4644: nop                                                              ; 00 00 00 00
801C4648: lw      $v0, 0x164($v0)                                          ; 64 01 42 8C
801C464C: nop                                                              ; 00 00 00 00
801C4650: sw      $v0, 0xC($s0)                                            ; 0C 00 02 AE
801C4654: lhu     $v0, word_800916D8                                       ; 09 80 02 3C D8 16 42 94
801C465C: nop                                                              ; 00 00 00 00
801C4660: sh      $v0, 0x66($s0)                                           ; 66 00 02 A6
801C4664: lhu     $v0, word_800916DC                                       ; 09 80 02 3C DC 16 42 94
801C466C: li      $s2, 1                                                   ; 01 00 12 34
801C4670: sh      $v0, 0x68($s0)                                           ; 68 00 02 A6
801C4674: sh      $v0, 0x54($s0)                                           ; 54 00 02 A6
801C4678: sw      $zero, 0($s0)                                            ; 00 00 00 AE
801C467C: jal     sub_80035510                                             ; 44 D5 00 0C
801C4680: li      $a0, 1                                                   ; 01 00 04 34
801C4684: move    $v1, $v0                                                 ; 21 18 40 00
801C4688: beqz    $v1, loc_801C46A8                                        ; 07 00 60 10
801C468C: li      $v0, 0x100                                               ; 00 01 02 34
801C4690: beq     $v1, $v0, loc_801C45A8                                   ; C5 FF 62 10
801C4694: li      $v0, 0x800                                               ; 00 08 02 34
801C4698: beq     $v1, $v0, loc_801C472C                                   ; 24 00 62 10
801C469C: andi    $v0, $v1, 0x840                                          ; 40 08 62 30
801C46A0: bnez    $v0, loc_801C472C                                        ; 22 00 40 14
801C46A4: nop                                                              ; 00 00 00 00
801C46A8: jal     sub_80024CF8                                             ; 3E 93 00 0C
801C46AC: move    $a0, $s0                                                 ; 21 20 00 02
801C46B0: jal     sub_80027528                                             ; 4A 9D 00 0C
801C46B4: nop                                                              ; 00 00 00 00
801C46B8: lw      $v0, dword_801C9554                                      ; 1D 80 02 3C 54 95 42 8C
801C46C0: nop                                                              ; 00 00 00 00
801C46C4: bne     $v0, $s2, loc_801C46D4                                   ; 03 00 52 14
801C46C8: move    $a0, $s0                                                 ; 21 20 00 02
801C46CC: jal     sub_8001EC54                                             ; 15 7B 00 0C
801C46D0: li      $a1, 7                                                   ; 07 00 05 34
801C46D4: jal     sub_80027528                                             ; 4A 9D 00 0C
801C46D8: nop                                                              ; 00 00 00 00
801C46DC: move    $a0, $s5                                                 ; 21 20 A0 02
801C46E0: jal     sub_801C448C                                             ; 23 11 07 0C
801C46E4: move    $a1, $s0                                                 ; 21 28 00 02
801C46E8: jal     sub_8001ED74                                             ; 5D 7B 00 0C
801C46EC: move    $s1, $v0                                                 ; 21 88 40 00
801C46F0: jal     sub_8002756C                                             ; 5B 9D 00 0C
801C46F4: nop                                                              ; 00 00 00 00
801C46F8: lw      $v0, dword_801C9554                                      ; 1D 80 02 3C 54 95 42 8C
801C4700: nop                                                              ; 00 00 00 00
801C4704: bne     $v0, $s2, loc_801C4714                                   ; 03 00 52 14
801C4708: nop                                                              ; 00 00 00 00
801C470C: jal     sub_8001ED3C                                             ; 4F 7B 00 0C
801C4710: move    $a0, $s0                                                 ; 21 20 00 02
801C4714: jal     GsGetWorkBase                                            ; EE 68 00 0C
801C4718: nop                                                              ; 00 00 00 00
801C471C: beq     $v0, $s2, loc_801C45B0                                   ; A4 FF 52 10
801C4720: nop                                                              ; 00 00 00 00
801C4724: beq     $s1, $s2, loc_801C4678                                   ; D4 FF 32 12
801C4728: nop                                                              ; 00 00 00 00
801C472C: jal     sub_80027664                                             ; 99 9D 00 0C
801C4730: nop                                                              ; 00 00 00 00
801C4734: jal     sub_8001A4A4                                             ; 29 69 00 0C
801C4738: li      $a0, 1                                                   ; 01 00 04 34
801C473C: jal     sub_8001A694                                             ; A5 69 00 0C
801C4740: nop                                                              ; 00 00 00 00
801C4744: jal     nullsub_4                                                ; 3C 93 00 0C
801C4748: move    $a0, $s0                                                 ; 21 20 00 02
801C474C: jal     sub_8001B120                                             ; 48 6C 00 0C
801C4750: li      $a0, 1                                                   ; 01 00 04 34
801C4754: move    $v0, $s3                                                 ; 21 10 60 02
801C4758: lw      $ra, 0x10+var_s18($sp)                                   ; 28 00 BF 8F
801C475C: lw      $s5, 0x10+var_s14($sp)                                   ; 24 00 B5 8F
801C4760: lw      $s4, 0x10+var_s10($sp)                                   ; 20 00 B4 8F
801C4764: lw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 8F
801C4768: lw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 8F
801C476C: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
801C4770: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
801C4774: addiu   $sp, 0x30                                                ; 30 00 BD 27
801C4778: jr      $ra                                                      ; 08 00 E0 03
801C477C: nop                                                              ; 00 00 00 00
```

## `800201AC` `sub_800201AC`
Bounds: `800201AC-80020248`

### Pseudocode
```c
int __fastcall sub_800201AC(int a1, int a2, int a3, int a4)
{
  int v7; // $a0
  int v8; // $v0
  int v9; // $gp

  sub_8001FFD4(a3);
  v7 = a1;
  do
  {
    sub_8001EA74(v7, a2);
    sub_80035560(2);
    sub_8001EBF4();
    v8 = sub_8001F518();
    v7 = a1;
  }
  while ( v8 );
  sub_8001FFD4(a4);
  *(_DWORD *)(v9 + 196) = 190;
  return sub_80020090(a1, a2, sub_8001EA74, sub_8001EBF4);
}

```

### Calls in function
- 800201CC: `jal     sub_8001FFD4` -> 8001FFD4 sub_8001FFD4
- 800201D8: `jal     sub_8001EA74` -> 8001EA74 sub_8001EA74
- 800201E0: `jal     sub_80035560` -> 80035560 sub_80035560
- 800201E8: `jal     sub_8001EBF4` -> 8001EBF4 sub_8001EBF4
- 800201F0: `jal     sub_8001F518` -> 8001F518 sub_8001F518
- 800201F8: `bnez    $v0, loc_800201D8` -> 800201D8 loc_800201D8
- 80020200: `jal     sub_8001FFD4` -> 8001FFD4 sub_8001FFD4
- 80020224: `jal     sub_80020090` -> 80020090 sub_80020090

### Xrefs to
- 801C4E20 -> 800201AC type=17 func=sub_801C4DC4 line=`jal     sub_800201AC`
- 801C4E70 -> 800201AC type=17 func=sub_801C4DC4 line=`jal     sub_800201AC`

### Xrefs from
- 800201AC -> 800201B0 type=21 target=sub_800201B0 line=`addiu   $sp, -0x20`
- 800201B0 -> 800201B4 type=21 target=sub_800201B4 line=`sw      $s0, 0x10+var_s0($sp)`
- 800201B4 -> 800201B8 type=21 target=sub_800201B8 line=`move    $s0, $a0`
- 800201B8 -> 800201BC type=21 target=sub_800201BC line=`sw      $s1, 0x10+var_s4($sp)`
- 800201BC -> 800201C0 type=21 target=sub_800201C0 line=`move    $s1, $a1`
- 800201C0 -> 800201C4 type=21 target=sub_800201C4 line=`move    $a0, $a2`
- 800201C4 -> 800201C8 type=21 target=sub_800201C8 line=`sw      $s2, 0x10+var_s8($sp)`
- 800201C8 -> 800201CC type=21 target=sub_800201CC line=`sw      $ra, 0x10+var_sC($sp)`
- 800201CC -> 800201D0 type=21 target=sub_800201D0 line=`jal     sub_8001FFD4`
- 800201CC -> 8001FFD4 type=17 target=sub_8001FFD4 line=`jal     sub_8001FFD4`
- 800201D0 -> 800201D4 type=21 target=sub_800201D4 line=`move    $s2, $a3`
- 800201D4 -> 800201D8 type=21 target=loc_800201D8 line=`move    $a0, $s0`
- 800201D8 -> 800201DC type=21 target=sub_800201DC line=`jal     sub_8001EA74`
- 800201D8 -> 8001EA74 type=17 target=sub_8001EA74 line=`jal     sub_8001EA74`
- 800201DC -> 800201E0 type=21 target=sub_800201E0 line=`move    $a1, $s1`
- 800201E0 -> 800201E4 type=21 target=sub_800201E4 line=`jal     sub_80035560`
- 800201E0 -> 80035560 type=17 target=sub_80035560 line=`jal     sub_80035560`
- 800201E4 -> 800201E8 type=21 target=sub_800201E8 line=`li      $a0, 2`
- 800201E8 -> 800201EC type=21 target=sub_800201EC line=`jal     sub_8001EBF4`
- 800201E8 -> 8001EBF4 type=17 target=sub_8001EBF4 line=`jal     sub_8001EBF4`
- 800201EC -> 800201F0 type=21 target=sub_800201F0 line=`move    $a0, $s0`
- 800201F0 -> 800201F4 type=21 target=sub_800201F4 line=`jal     sub_8001F518`
- 800201F0 -> 8001F518 type=17 target=sub_8001F518 line=`jal     sub_8001F518`
- 800201F4 -> 800201F8 type=21 target=sub_800201F8 line=`nop`
- 800201F8 -> 800201FC type=21 target=sub_800201FC line=`bnez    $v0, loc_800201D8`
- 800201F8 -> 800201D8 type=19 target=loc_800201D8 line=`bnez    $v0, loc_800201D8`
- 800201FC -> 80020200 type=21 target=sub_80020200 line=`move    $a0, $s0`
- 80020200 -> 80020204 type=21 target=sub_80020204 line=`jal     sub_8001FFD4`
- 80020200 -> 8001FFD4 type=17 target=sub_8001FFD4 line=`jal     sub_8001FFD4`
- 80020204 -> 80020208 type=21 target=sub_80020208 line=`move    $a0, $s2`
- 80020208 -> 8002020C type=21 target=sub_8002020C line=`li      $v0, 0xBE`
- 8002020C -> 80020210 type=21 target=sub_80020210 line=`sw      $v0, 0xC4($gp)`
- 80020210 -> 80020214 type=21 target=sub_80020214 line=`move    $a0, $s0`
- 80020214 -> 8002021C type=21 target=sub_8002021C line=`li      $a2, sub_8001EA74`
- 80020214 -> 8001EA74 type=1 target=sub_8001EA74 line=`li      $a2, sub_8001EA74`
- 8002021C -> 80020224 type=21 target=sub_80020224 line=`li      $a3, sub_8001EBF4`
- 8002021C -> 8001EBF4 type=1 target=sub_8001EBF4 line=`li      $a3, sub_8001EBF4`
- 80020224 -> 80020228 type=21 target=sub_80020228 line=`jal     sub_80020090`
- 80020224 -> 80020090 type=17 target=sub_80020090 line=`jal     sub_80020090`
- 80020228 -> 8002022C type=21 target=sub_8002022C line=`move    $a1, $s1`
- 8002022C -> 80020230 type=21 target=sub_80020230 line=`lw      $ra, 0x10+var_sC($sp)`
- 80020230 -> 80020234 type=21 target=sub_80020234 line=`lw      $s2, 0x10+var_s8($sp)`
- 80020234 -> 80020238 type=21 target=sub_80020238 line=`lw      $s1, 0x10+var_s4($sp)`
- 80020238 -> 8002023C type=21 target=sub_8002023C line=`lw      $s0, 0x10+var_s0($sp)`
- 8002023C -> 80020240 type=21 target=sub_80020240 line=`addiu   $sp, 0x20`
- 80020240 -> 80020244 type=21 target=sub_80020244 line=`jr      $ra`

### Full disasm
```asm
800201AC: addiu   $sp, -0x20                                               ; E0 FF BD 27
800201B0: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
800201B4: move    $s0, $a0                                                 ; 21 80 80 00
800201B8: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
800201BC: move    $s1, $a1                                                 ; 21 88 A0 00
800201C0: move    $a0, $a2                                                 ; 21 20 C0 00
800201C4: sw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 AF
800201C8: sw      $ra, 0x10+var_sC($sp)                                    ; 1C 00 BF AF
800201CC: jal     sub_8001FFD4                                             ; F5 7F 00 0C
800201D0: move    $s2, $a3                                                 ; 21 90 E0 00
800201D4: move    $a0, $s0                                                 ; 21 20 00 02
800201D8: jal     sub_8001EA74                                             ; 9D 7A 00 0C
800201DC: move    $a1, $s1                                                 ; 21 28 20 02
800201E0: jal     sub_80035560                                             ; 58 D5 00 0C
800201E4: li      $a0, 2                                                   ; 02 00 04 34
800201E8: jal     sub_8001EBF4                                             ; FD 7A 00 0C
800201EC: move    $a0, $s0                                                 ; 21 20 00 02
800201F0: jal     sub_8001F518                                             ; 46 7D 00 0C
800201F4: nop                                                              ; 00 00 00 00
800201F8: bnez    $v0, loc_800201D8                                        ; F7 FF 40 14
800201FC: move    $a0, $s0                                                 ; 21 20 00 02
80020200: jal     sub_8001FFD4                                             ; F5 7F 00 0C
80020204: move    $a0, $s2                                                 ; 21 20 40 02
80020208: li      $v0, 0xBE                                                ; BE 00 02 34
8002020C: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
80020210: move    $a0, $s0                                                 ; 21 20 00 02
80020214: li      $a2, sub_8001EA74                                        ; 02 80 06 3C 74 EA C6 24
8002021C: li      $a3, sub_8001EBF4                                        ; 02 80 07 3C F4 EB E7 24
80020224: jal     sub_80020090                                             ; 24 80 00 0C
80020228: move    $a1, $s1                                                 ; 21 28 20 02
8002022C: lw      $ra, 0x10+var_sC($sp)                                    ; 1C 00 BF 8F
80020230: lw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 8F
80020234: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
80020238: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
8002023C: addiu   $sp, 0x20                                                ; 20 00 BD 27
80020240: jr      $ra                                                      ; 08 00 E0 03
80020244: nop                                                              ; 00 00 00 00
```

## `8001EA74` `sub_8001EA74`
Bounds: `8001EA74-8001EBF4`

### Pseudocode
```c
int __fastcall sub_8001EA74(int a1, int a2)
{
  int v3; // $v0
  int v5; // $a0
  int v6; // $gp
  int v7; // $gp
  int result; // $v0
  int v9; // $gp
  int v10; // $gp
  int v11; // $gp
  int v12; // $gp

  v3 = sub_8004019C();
  v5 = dword_8006ED50[v3];
  *(_DWORD *)(v6 + 872) = v3;
  *(_DWORD *)(v6 + 792) = 1;
  sub_80040F90(v5);
  result = sub_80040CC8(0, 0, (int *)&unk_80087288 + 5 * *(_DWORD *)(v7 + 872));
  switch ( a2 )
  {
    case 1:
      return sub_8001F524(0, 8);
    case 2:
      *(_DWORD *)(v9 + 792) = 0;
      sub_8001FCBC(8, 4);
      goto LABEL_6;
    case 3:
      sub_80022CBC(4, a1);
      sub_8001D74C(5, *(_DWORD *)(v10 + 872));
      sub_8001FC40(4, 8);
      goto LABEL_6;
    case 4:
      sub_80021E60(0);
      sub_8001D74C(5, *(_DWORD *)(v11 + 872));
      sub_8001FC40(4, 8);
LABEL_6:
      result = sub_8001FDC0(0);
      break;
    case 5:
      result = sub_80020308(word_800916DC);
      *(_DWORD *)(v12 + 792) = result;
      break;
    case 6:
      result = sub_80020248(word_800916DC);
      break;
    default:
      if ( a1 )
        result = sub_8001EF40(word_800916E0, 0);
      break;
  }
  return result;
}

```

### Calls in function
- 8001EA88: `jal     sub_8004019C` -> 8004019C sub_8004019C
- 8001EAB0: `jal     sub_80040F90` -> 80040F90 sub_80040F90
- 8001EAD8: `jal     sub_80040CC8` -> 80040CC8 sub_80040CC8
- 8001EAE4: `beqz    $v0, def_8001EB00 # jumptable 8001EB00 default case` -> 8001EBC4 def_8001EB00
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB08 loc_8001EB08
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB1C loc_8001EB1C
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB48 loc_8001EB48
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB70 loc_8001EB70
- 8001EB00: `jr      $v0 # switch jump` -> 8001EB90 loc_8001EB90
- 8001EB00: `jr      $v0 # switch jump` -> 8001EBA8 loc_8001EBA8
- 8001EB0C: `jal     sub_8001F524` -> 8001F524 sub_8001F524
- 8001EB14: `j       loc_8001EBDC` -> 8001EBDC loc_8001EBDC
- 8001EB20: `jal     sub_80022CBC` -> 80022CBC sub_80022CBC
- 8001EB2C: `jal     sub_8001D74C` -> 8001D74C sub_8001D74C
- 8001EB38: `jal     sub_8001FC40` -> 8001FC40 sub_8001FC40
- 8001EB40: `j       loc_8001EB80` -> 8001EB80 loc_8001EB80
- 8001EB48: `jal     sub_80021E60 # jumptable 8001EB00 case 4` -> 80021E60 sub_80021E60
- 8001EB54: `jal     sub_8001D74C` -> 8001D74C sub_8001D74C
- 8001EB60: `jal     sub_8001FC40` -> 8001FC40 sub_8001FC40
- 8001EB68: `j       loc_8001EB80` -> 8001EB80 loc_8001EB80
- 8001EB78: `jal     sub_8001FCBC` -> 8001FCBC sub_8001FCBC
- 8001EB80: `jal     sub_8001FDC0` -> 8001FDC0 sub_8001FDC0
- 8001EB88: `j       loc_8001EBDC` -> 8001EBDC loc_8001EBDC
- 8001EB98: `jal     sub_80020248` -> 80020248 sub_80020248
- 8001EBA0: `j       loc_8001EBDC` -> 8001EBDC loc_8001EBDC
- 8001EBB0: `jal     sub_80020308` -> 80020308 sub_80020308
- 8001EBBC: `j       loc_8001EBDC` -> 8001EBDC loc_8001EBDC
- 8001EBC4: `beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case` -> 8001EBDC loc_8001EBDC
- 8001EBD4: `jal     sub_8001EF40` -> 8001EF40 sub_8001EF40

### Xrefs to
- 800153A8 -> 8001EA74 type=17 func=sub_8001537C line=`jal     sub_8001EA74`
- 8002013C -> 8001EA74 type=17 func=sub_80020110 line=`jal     sub_8001EA74`
- 800201D8 -> 8001EA74 type=17 func=sub_800201AC line=`jal     sub_8001EA74`
- 80020178 -> 8001EA74 type=1 func=sub_80020110 line=`li      $a2, sub_8001EA74`
- 80020214 -> 8001EA74 type=1 func=sub_800201AC line=`li      $a2, sub_8001EA74`

### Xrefs from
- 8001EA74 -> 8001EA78 type=21 target=sub_8001EA78 line=`addiu   $sp, -0x20`
- 8001EA78 -> 8001EA7C type=21 target=sub_8001EA7C line=`sw      $s1, 0x10+var_s4($sp)`
- 8001EA7C -> 8001EA80 type=21 target=sub_8001EA80 line=`move    $s1, $a0`
- 8001EA80 -> 8001EA84 type=21 target=sub_8001EA84 line=`sw      $s0, 0x10+var_s0($sp)`
- 8001EA84 -> 8001EA88 type=21 target=sub_8001EA88 line=`sw      $ra, 0x10+var_s8($sp)`
- 8001EA88 -> 8001EA8C type=21 target=sub_8001EA8C line=`jal     sub_8004019C`
- 8001EA88 -> 8004019C type=17 target=sub_8004019C line=`jal     sub_8004019C`
- 8001EA8C -> 8001EA90 type=21 target=sub_8001EA90 line=`move    $s0, $a1`
- 8001EA90 -> 8001EA94 type=21 target=sub_8001EA94 line=`sll     $v1, $v0, 2`
- 8001EA94 -> 8001EA9C type=21 target=sub_8001EA9C line=`li      $at, dword_8006ED50`
- 8001EA9C -> 8001EAA0 type=21 target=sub_8001EAA0 line=`addu    $at, $v1`
- 8001EAA0 -> 8001EAA4 type=21 target=sub_8001EAA4 line=`lw      $a0, 0($at)`
- 8001EAA4 -> 8001EAA8 type=21 target=sub_8001EAA8 line=`sw      $v0, 0x368($gp)`
- 8001EAA8 -> 8001EAAC type=21 target=sub_8001EAAC line=`li      $v0, 1`
- 8001EAAC -> 8001EAB0 type=21 target=sub_8001EAB0 line=`sw      $v0, 0x318($gp)`
- 8001EAB0 -> 8001EAB4 type=21 target=sub_8001EAB4 line=`jal     sub_80040F90`
- 8001EAB0 -> 80040F90 type=17 target=sub_80040F90 line=`jal     sub_80040F90`
- 8001EAB4 -> 8001EAB8 type=21 target=sub_8001EAB8 line=`addiu   $s0, -1 # switch 6 cases`
- 8001EAB8 -> 8001EABC type=21 target=sub_8001EABC line=`move    $a0, $zero`
- 8001EABC -> 8001EAC0 type=21 target=sub_8001EAC0 line=`lw      $v0, 0x368($gp)`
- 8001EAC0 -> 8001EAC4 type=21 target=sub_8001EAC4 line=`move    $a1, $zero`
- 8001EAC4 -> 8001EAC8 type=21 target=sub_8001EAC8 line=`sll     $a2, $v0, 2`
- 8001EAC8 -> 8001EACC type=21 target=sub_8001EACC line=`addu    $a2, $v0`
- 8001EACC -> 8001EAD0 type=21 target=sub_8001EAD0 line=`sll     $a2, 2`
- 8001EAD0 -> 8001EAD8 type=21 target=sub_8001EAD8 line=`li      $v0, unk_80087288`
- 8001EAD8 -> 8001EADC type=21 target=sub_8001EADC line=`jal     sub_80040CC8`
- 8001EAD8 -> 80040CC8 type=17 target=sub_80040CC8 line=`jal     sub_80040CC8`
- 8001EADC -> 8001EAE0 type=21 target=sub_8001EAE0 line=`addu    $a2, $v0`
- 8001EAE0 -> 8001EAE4 type=21 target=sub_8001EAE4 line=`sltiu   $v0, $s0, 6`
- 8001EAE4 -> 8001EAE8 type=21 target=sub_8001EAE8 line=`beqz    $v0, def_8001EB00 # jumptable 8001EB00 default case`
- 8001EAE4 -> 8001EBC4 type=19 target=def_8001EB00 line=`beqz    $v0, def_8001EB00 # jumptable 8001EB00 default case`
- 8001EAE8 -> 8001EAEC type=21 target=sub_8001EAEC line=`sll     $v0, $s0, 2`
- 8001EAEC -> 8001EAF4 type=21 target=sub_8001EAF4 line=`li      $at, jpt_8001EB00`
- 8001EAF4 -> 8001EAF8 type=21 target=sub_8001EAF8 line=`addu    $at, $v0`
- 8001EAF8 -> 8001EAFC type=21 target=sub_8001EAFC line=`lw      $v0, 0($at)`
- 8001EAFC -> 8001EB00 type=21 target=sub_8001EB00 line=`nop`
- 8001EB00 -> 8001EB04 type=21 target=sub_8001EB04 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB08 type=19 target=loc_8001EB08 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB1C type=19 target=loc_8001EB1C line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB48 type=19 target=loc_8001EB48 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB70 type=19 target=loc_8001EB70 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EB90 type=19 target=loc_8001EB90 line=`jr      $v0 # switch jump`
- 8001EB00 -> 8001EBA8 type=19 target=loc_8001EBA8 line=`jr      $v0 # switch jump`
- 8001EB08 -> 8001EB0C type=21 target=sub_8001EB0C line=`move    $a0, $zero # jumptable 8001EB00 case 1`
- 8001EB0C -> 8001EB10 type=21 target=sub_8001EB10 line=`jal     sub_8001F524`
- 8001EB0C -> 8001F524 type=17 target=sub_8001F524 line=`jal     sub_8001F524`
- 8001EB10 -> 8001EB14 type=21 target=sub_8001EB14 line=`li      $a1, 8`
- 8001EB14 -> 8001EB18 type=21 target=sub_8001EB18 line=`j       loc_8001EBDC`
- 8001EB14 -> 8001EBDC type=19 target=loc_8001EBDC line=`j       loc_8001EBDC`
- 8001EB1C -> 8001EB20 type=21 target=sub_8001EB20 line=`li      $a0, 4 # jumptable 8001EB00 case 3`
- 8001EB20 -> 8001EB24 type=21 target=sub_8001EB24 line=`jal     sub_80022CBC`
- 8001EB20 -> 80022CBC type=17 target=sub_80022CBC line=`jal     sub_80022CBC`
- 8001EB24 -> 8001EB28 type=21 target=sub_8001EB28 line=`move    $a1, $s1`
- 8001EB28 -> 8001EB2C type=21 target=sub_8001EB2C line=`lw      $a1, 0x368($gp)`
- 8001EB2C -> 8001EB30 type=21 target=sub_8001EB30 line=`jal     sub_8001D74C`
- 8001EB2C -> 8001D74C type=17 target=sub_8001D74C line=`jal     sub_8001D74C`
- 8001EB30 -> 8001EB34 type=21 target=sub_8001EB34 line=`li      $a0, 5`
- 8001EB34 -> 8001EB38 type=21 target=sub_8001EB38 line=`li      $a0, 4`
- 8001EB38 -> 8001EB3C type=21 target=sub_8001EB3C line=`jal     sub_8001FC40`
- 8001EB38 -> 8001FC40 type=17 target=sub_8001FC40 line=`jal     sub_8001FC40`
- 8001EB3C -> 8001EB40 type=21 target=sub_8001EB40 line=`li      $a1, 8`
- 8001EB40 -> 8001EB44 type=21 target=sub_8001EB44 line=`j       loc_8001EB80`
- 8001EB40 -> 8001EB80 type=19 target=loc_8001EB80 line=`j       loc_8001EB80`
- 8001EB48 -> 8001EB4C type=21 target=sub_8001EB4C line=`jal     sub_80021E60 # jumptable 8001EB00 case 4`
- 8001EB48 -> 80021E60 type=17 target=sub_80021E60 line=`jal     sub_80021E60 # jumptable 8001EB00 case 4`
- 8001EB4C -> 8001EB50 type=21 target=sub_8001EB50 line=`move    $a0, $zero`
- 8001EB50 -> 8001EB54 type=21 target=sub_8001EB54 line=`lw      $a1, 0x368($gp)`
- 8001EB54 -> 8001EB58 type=21 target=sub_8001EB58 line=`jal     sub_8001D74C`
- 8001EB54 -> 8001D74C type=17 target=sub_8001D74C line=`jal     sub_8001D74C`
- 8001EB58 -> 8001EB5C type=21 target=sub_8001EB5C line=`li      $a0, 5`
- 8001EB5C -> 8001EB60 type=21 target=sub_8001EB60 line=`li      $a0, 4`
- 8001EB60 -> 8001EB64 type=21 target=sub_8001EB64 line=`jal     sub_8001FC40`
- 8001EB60 -> 8001FC40 type=17 target=sub_8001FC40 line=`jal     sub_8001FC40`
- 8001EB64 -> 8001EB68 type=21 target=sub_8001EB68 line=`li      $a1, 8`
- 8001EB68 -> 8001EB6C type=21 target=sub_8001EB6C line=`j       loc_8001EB80`
- 8001EB68 -> 8001EB80 type=19 target=loc_8001EB80 line=`j       loc_8001EB80`
- 8001EB70 -> 8001EB74 type=21 target=sub_8001EB74 line=`sw      $zero, 0x318($gp) # jumptable 8001EB00 case 2`
- 8001EB74 -> 8001EB78 type=21 target=sub_8001EB78 line=`li      $a0, 8`
- 8001EB78 -> 8001EB7C type=21 target=sub_8001EB7C line=`jal     sub_8001FCBC`
- 8001EB78 -> 8001FCBC type=17 target=sub_8001FCBC line=`jal     sub_8001FCBC`
- 8001EB7C -> 8001EB80 type=21 target=loc_8001EB80 line=`li      $a1, 4`
- 8001EB80 -> 8001EB84 type=21 target=sub_8001EB84 line=`jal     sub_8001FDC0`
- 8001EB80 -> 8001FDC0 type=17 target=sub_8001FDC0 line=`jal     sub_8001FDC0`
- 8001EB84 -> 8001EB88 type=21 target=sub_8001EB88 line=`move    $a0, $zero`
- 8001EB88 -> 8001EB8C type=21 target=sub_8001EB8C line=`j       loc_8001EBDC`
- 8001EB88 -> 8001EBDC type=19 target=loc_8001EBDC line=`j       loc_8001EBDC`
- 8001EB90 -> 8001EB98 type=21 target=sub_8001EB98 line=`lh      $a0, word_800916DC # jumptable 8001EB00 case 6`
- 8001EB98 -> 8001EB9C type=21 target=sub_8001EB9C line=`jal     sub_80020248`
- 8001EB98 -> 80020248 type=17 target=sub_80020248 line=`jal     sub_80020248`
- 8001EB9C -> 8001EBA0 type=21 target=sub_8001EBA0 line=`nop`
- 8001EBA0 -> 8001EBA4 type=21 target=sub_8001EBA4 line=`j       loc_8001EBDC`
- 8001EBA0 -> 8001EBDC type=19 target=loc_8001EBDC line=`j       loc_8001EBDC`
- 8001EBA8 -> 8001EBB0 type=21 target=sub_8001EBB0 line=`lh      $a0, word_800916DC # jumptable 8001EB00 case 5`
- 8001EBB0 -> 8001EBB4 type=21 target=sub_8001EBB4 line=`jal     sub_80020308`
- 8001EBB0 -> 80020308 type=17 target=sub_80020308 line=`jal     sub_80020308`
- 8001EBB4 -> 8001EBB8 type=21 target=sub_8001EBB8 line=`nop`
- 8001EBB8 -> 8001EBBC type=21 target=sub_8001EBBC line=`sw      $v0, 0x318($gp)`
- 8001EBBC -> 8001EBC0 type=21 target=sub_8001EBC0 line=`j       loc_8001EBDC`
- 8001EBBC -> 8001EBDC type=19 target=loc_8001EBDC line=`j       loc_8001EBDC`
- 8001EBC4 -> 8001EBC8 type=21 target=sub_8001EBC8 line=`beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case`
- 8001EBC4 -> 8001EBDC type=19 target=loc_8001EBDC line=`beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case`
- 8001EBC8 -> 8001EBCC type=21 target=sub_8001EBCC line=`nop`
- 8001EBCC -> 8001EBD4 type=21 target=sub_8001EBD4 line=`lh      $a0, word_800916E0`
- 8001EBD4 -> 8001EBD8 type=21 target=sub_8001EBD8 line=`jal     sub_8001EF40`
- 8001EBD4 -> 8001EF40 type=17 target=sub_8001EF40 line=`jal     sub_8001EF40`
- 8001EBD8 -> 8001EBDC type=21 target=loc_8001EBDC line=`move    $a1, $zero`
- 8001EBDC -> 8001EBE0 type=21 target=sub_8001EBE0 line=`lw      $ra, 0x10+var_s8($sp)`
- 8001EBE0 -> 8001EBE4 type=21 target=sub_8001EBE4 line=`lw      $s1, 0x10+var_s4($sp)`
- 8001EBE4 -> 8001EBE8 type=21 target=sub_8001EBE8 line=`lw      $s0, 0x10+var_s0($sp)`
- 8001EBE8 -> 8001EBEC type=21 target=sub_8001EBEC line=`addiu   $sp, 0x20`
- 8001EBEC -> 8001EBF0 type=21 target=sub_8001EBF0 line=`jr      $ra`

### Full disasm
```asm
8001EA74: addiu   $sp, -0x20                                               ; E0 FF BD 27
8001EA78: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
8001EA7C: move    $s1, $a0                                                 ; 21 88 80 00
8001EA80: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
8001EA84: sw      $ra, 0x10+var_s8($sp)                                    ; 18 00 BF AF
8001EA88: jal     sub_8004019C                                             ; 67 00 01 0C
8001EA8C: move    $s0, $a1                                                 ; 21 80 A0 00
8001EA90: sll     $v1, $v0, 2                                              ; 80 18 02 00
8001EA94: li      $at, dword_8006ED50                                      ; 07 80 01 3C 50 ED 21 24
8001EA9C: addu    $at, $v1                                                 ; 21 08 23 00
8001EAA0: lw      $a0, 0($at)                                              ; 00 00 24 8C
8001EAA4: sw      $v0, 0x368($gp)                                          ; 68 03 82 AF
8001EAA8: li      $v0, 1                                                   ; 01 00 02 34
8001EAAC: sw      $v0, 0x318($gp)                                          ; 18 03 82 AF
8001EAB0: jal     sub_80040F90                                             ; E4 03 01 0C
8001EAB4: addiu   $s0, -1 # switch 6 cases                                 ; FF FF 10 26
8001EAB8: move    $a0, $zero                                               ; 21 20 00 00
8001EABC: lw      $v0, 0x368($gp)                                          ; 68 03 82 8F
8001EAC0: move    $a1, $zero                                               ; 21 28 00 00
8001EAC4: sll     $a2, $v0, 2                                              ; 80 30 02 00
8001EAC8: addu    $a2, $v0                                                 ; 21 30 C2 00
8001EACC: sll     $a2, 2                                                   ; 80 30 06 00
8001EAD0: li      $v0, unk_80087288                                        ; 08 80 02 3C 88 72 42 24
8001EAD8: jal     sub_80040CC8                                             ; 32 03 01 0C
8001EADC: addu    $a2, $v0                                                 ; 21 30 C2 00
8001EAE0: sltiu   $v0, $s0, 6                                              ; 06 00 02 2E
8001EAE4: beqz    $v0, def_8001EB00 # jumptable 8001EB00 default case      ; 37 00 40 10
8001EAE8: sll     $v0, $s0, 2                                              ; 80 10 10 00
8001EAEC: li      $at, jpt_8001EB00                                        ; 01 80 01 3C 44 10 21 24
8001EAF4: addu    $at, $v0                                                 ; 21 08 22 00
8001EAF8: lw      $v0, 0($at)                                              ; 00 00 22 8C
8001EAFC: nop                                                              ; 00 00 00 00
8001EB00: jr      $v0 # switch jump                                        ; 08 00 40 00
8001EB04: nop                                                              ; 00 00 00 00
8001EB08: move    $a0, $zero # jumptable 8001EB00 case 1                   ; 21 20 00 00
8001EB0C: jal     sub_8001F524                                             ; 49 7D 00 0C
8001EB10: li      $a1, 8                                                   ; 08 00 05 34
8001EB14: j       loc_8001EBDC                                             ; F7 7A 00 08
8001EB18: nop                                                              ; 00 00 00 00
8001EB1C: li      $a0, 4 # jumptable 8001EB00 case 3                       ; 04 00 04 34
8001EB20: jal     sub_80022CBC                                             ; 2F 8B 00 0C
8001EB24: move    $a1, $s1                                                 ; 21 28 20 02
8001EB28: lw      $a1, 0x368($gp)                                          ; 68 03 85 8F
8001EB2C: jal     sub_8001D74C                                             ; D3 75 00 0C
8001EB30: li      $a0, 5                                                   ; 05 00 04 34
8001EB34: li      $a0, 4                                                   ; 04 00 04 34
8001EB38: jal     sub_8001FC40                                             ; 10 7F 00 0C
8001EB3C: li      $a1, 8                                                   ; 08 00 05 34
8001EB40: j       loc_8001EB80                                             ; E0 7A 00 08
8001EB44: nop                                                              ; 00 00 00 00
8001EB48: jal     sub_80021E60 # jumptable 8001EB00 case 4                 ; 98 87 00 0C
8001EB4C: move    $a0, $zero                                               ; 21 20 00 00
8001EB50: lw      $a1, 0x368($gp)                                          ; 68 03 85 8F
8001EB54: jal     sub_8001D74C                                             ; D3 75 00 0C
8001EB58: li      $a0, 5                                                   ; 05 00 04 34
8001EB5C: li      $a0, 4                                                   ; 04 00 04 34
8001EB60: jal     sub_8001FC40                                             ; 10 7F 00 0C
8001EB64: li      $a1, 8                                                   ; 08 00 05 34
8001EB68: j       loc_8001EB80                                             ; E0 7A 00 08
8001EB6C: nop                                                              ; 00 00 00 00
8001EB70: sw      $zero, 0x318($gp) # jumptable 8001EB00 case 2            ; 18 03 80 AF
8001EB74: li      $a0, 8                                                   ; 08 00 04 34
8001EB78: jal     sub_8001FCBC                                             ; 2F 7F 00 0C
8001EB7C: li      $a1, 4                                                   ; 04 00 05 34
8001EB80: jal     sub_8001FDC0                                             ; 70 7F 00 0C
8001EB84: move    $a0, $zero                                               ; 21 20 00 00
8001EB88: j       loc_8001EBDC                                             ; F7 7A 00 08
8001EB8C: nop                                                              ; 00 00 00 00
8001EB90: lh      $a0, word_800916DC # jumptable 8001EB00 case 6           ; 09 80 04 3C DC 16 84 84
8001EB98: jal     sub_80020248                                             ; 92 80 00 0C
8001EB9C: nop                                                              ; 00 00 00 00
8001EBA0: j       loc_8001EBDC                                             ; F7 7A 00 08
8001EBA4: nop                                                              ; 00 00 00 00
8001EBA8: lh      $a0, word_800916DC # jumptable 8001EB00 case 5           ; 09 80 04 3C DC 16 84 84
8001EBB0: jal     sub_80020308                                             ; C2 80 00 0C
8001EBB4: nop                                                              ; 00 00 00 00
8001EBB8: sw      $v0, 0x318($gp)                                          ; 18 03 82 AF
8001EBBC: j       loc_8001EBDC                                             ; F7 7A 00 08
8001EBC0: nop                                                              ; 00 00 00 00
8001EBC4: beqz    $s1, loc_8001EBDC # jumptable 8001EB00 default case      ; 05 00 20 12
8001EBC8: nop                                                              ; 00 00 00 00
8001EBCC: lh      $a0, word_800916E0                                       ; 09 80 04 3C E0 16 84 84
8001EBD4: jal     sub_8001EF40                                             ; D0 7B 00 0C
8001EBD8: move    $a1, $zero                                               ; 21 28 00 00
8001EBDC: lw      $ra, 0x10+var_s8($sp)                                    ; 18 00 BF 8F
8001EBE0: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
8001EBE4: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
8001EBE8: addiu   $sp, 0x20                                                ; 20 00 BD 27
8001EBEC: jr      $ra                                                      ; 08 00 E0 03
8001EBF0: nop                                                              ; 00 00 00 00
```

## `80020248` `sub_80020248`
Bounds: `80020248-80020308`

### Pseudocode
```c
int __fastcall sub_80020248(int a1)
{
  int v1; // $gp
  int v3; // $gp
  int v4; // $v0
  BOOL v5; // $v0
  int v6; // $v0
  bool v7; // dc
  int result; // $v0

  if ( !*(_DWORD *)(v1 + 196) )
    sub_800271E4(1);
  if ( a1 )
  {
    if ( *(_DWORD *)(v1 + 196) == 15 )
      sub_800271E4(0);
    if ( *(int *)(v1 + 196) >= 15 )
      sub_8001C864(5);
    else
      sub_8001F230(5);
    v6 = *(_DWORD *)(v3 + 196) + 1;
    *(_DWORD *)(v3 + 196) = v6;
    v5 = v6 < 31;
  }
  else
  {
    sub_8001CE30(5);
    v4 = *(_DWORD *)(v3 + 196) + 1;
    *(_DWORD *)(v3 + 196) = v4;
    v5 = v4 < 16;
  }
  v7 = v5;
  result = 192;
  if ( !v7 )
    *(_DWORD *)(v3 + 196) = 192;
  return result;
}

```

### Calls in function
- 80020258: `bnez    $v0, loc_80020268` -> 80020268 loc_80020268
- 80020260: `jal     sub_800271E4` -> 800271E4 sub_800271E4
- 80020268: `bnez    $s0, loc_80020290` -> 80020290 loc_80020290
- 80020270: `jal     sub_8001CE30` -> 8001CE30 sub_8001CE30
- 80020288: `j       loc_800202E8` -> 800202E8 loc_800202E8
- 80020298: `bne     $v1, $v0, loc_800202A8` -> 800202A8 loc_800202A8
- 800202A0: `jal     sub_800271E4` -> 800271E4 sub_800271E4
- 800202B4: `beqz    $v0, loc_800202CC` -> 800202CC loc_800202CC
- 800202BC: `jal     sub_8001F230` -> 8001F230 sub_8001F230
- 800202C4: `j       loc_800202D4` -> 800202D4 loc_800202D4
- 800202CC: `jal     sub_8001C864` -> 8001C864 sub_8001C864
- 800202E8: `bnez    $v0, loc_800202F4` -> 800202F4 loc_800202F4

### Xrefs to
- 8001EB98 -> 80020248 type=17 func=sub_8001EA74 line=`jal     sub_80020248`

### Xrefs from
- 80020248 -> 8002024C type=21 target=sub_8002024C line=`lw      $v0, 0xC4($gp)`
- 8002024C -> 80020250 type=21 target=sub_80020250 line=`addiu   $sp, -0x18`
- 80020250 -> 80020254 type=21 target=sub_80020254 line=`sw      $s0, 0x10+var_s0($sp)`
- 80020254 -> 80020258 type=21 target=sub_80020258 line=`move    $s0, $a0`
- 80020258 -> 8002025C type=21 target=sub_8002025C line=`bnez    $v0, loc_80020268`
- 80020258 -> 80020268 type=19 target=loc_80020268 line=`bnez    $v0, loc_80020268`
- 8002025C -> 80020260 type=21 target=sub_80020260 line=`sw      $ra, 0x10+var_s4($sp)`
- 80020260 -> 80020264 type=21 target=sub_80020264 line=`jal     sub_800271E4`
- 80020260 -> 800271E4 type=17 target=sub_800271E4 line=`jal     sub_800271E4`
- 80020264 -> 80020268 type=21 target=loc_80020268 line=`li      $a0, 1`
- 80020268 -> 8002026C type=21 target=sub_8002026C line=`bnez    $s0, loc_80020290`
- 80020268 -> 80020290 type=19 target=loc_80020290 line=`bnez    $s0, loc_80020290`
- 8002026C -> 80020270 type=21 target=sub_80020270 line=`nop`
- 80020270 -> 80020274 type=21 target=sub_80020274 line=`jal     sub_8001CE30`
- 80020270 -> 8001CE30 type=17 target=sub_8001CE30 line=`jal     sub_8001CE30`
- 80020274 -> 80020278 type=21 target=sub_80020278 line=`li      $a0, 5`
- 80020278 -> 8002027C type=21 target=sub_8002027C line=`lw      $v0, 0xC4($gp)`
- 8002027C -> 80020280 type=21 target=sub_80020280 line=`nop`
- 80020280 -> 80020284 type=21 target=sub_80020284 line=`addiu   $v0, 1`
- 80020284 -> 80020288 type=21 target=sub_80020288 line=`sw      $v0, 0xC4($gp)`
- 80020288 -> 8002028C type=21 target=sub_8002028C line=`j       loc_800202E8`
- 80020288 -> 800202E8 type=19 target=loc_800202E8 line=`j       loc_800202E8`
- 80020290 -> 80020294 type=21 target=sub_80020294 line=`lw      $v1, 0xC4($gp)`
- 80020294 -> 80020298 type=21 target=sub_80020298 line=`li      $v0, 0xF`
- 80020298 -> 8002029C type=21 target=sub_8002029C line=`bne     $v1, $v0, loc_800202A8`
- 80020298 -> 800202A8 type=19 target=loc_800202A8 line=`bne     $v1, $v0, loc_800202A8`
- 8002029C -> 800202A0 type=21 target=sub_800202A0 line=`nop`
- 800202A0 -> 800202A4 type=21 target=sub_800202A4 line=`jal     sub_800271E4`
- 800202A0 -> 800271E4 type=17 target=sub_800271E4 line=`jal     sub_800271E4`
- 800202A4 -> 800202A8 type=21 target=loc_800202A8 line=`move    $a0, $zero`
- 800202A8 -> 800202AC type=21 target=sub_800202AC line=`lw      $v0, 0xC4($gp)`
- 800202AC -> 800202B0 type=21 target=sub_800202B0 line=`nop`
- 800202B0 -> 800202B4 type=21 target=sub_800202B4 line=`slti    $v0, 0xF`
- 800202B4 -> 800202B8 type=21 target=sub_800202B8 line=`beqz    $v0, loc_800202CC`
- 800202B4 -> 800202CC type=19 target=loc_800202CC line=`beqz    $v0, loc_800202CC`
- 800202B8 -> 800202BC type=21 target=sub_800202BC line=`nop`
- 800202BC -> 800202C0 type=21 target=sub_800202C0 line=`jal     sub_8001F230`
- 800202BC -> 8001F230 type=17 target=sub_8001F230 line=`jal     sub_8001F230`
- 800202C0 -> 800202C4 type=21 target=sub_800202C4 line=`li      $a0, 5`
- 800202C4 -> 800202C8 type=21 target=sub_800202C8 line=`j       loc_800202D4`
- 800202C4 -> 800202D4 type=19 target=loc_800202D4 line=`j       loc_800202D4`
- 800202CC -> 800202D0 type=21 target=sub_800202D0 line=`jal     sub_8001C864`
- 800202CC -> 8001C864 type=17 target=sub_8001C864 line=`jal     sub_8001C864`
- 800202D0 -> 800202D4 type=21 target=loc_800202D4 line=`li      $a0, 5`
- 800202D4 -> 800202D8 type=21 target=sub_800202D8 line=`lw      $v0, 0xC4($gp)`
- 800202D8 -> 800202DC type=21 target=sub_800202DC line=`nop`
- 800202DC -> 800202E0 type=21 target=sub_800202E0 line=`addiu   $v0, 1`
- 800202E0 -> 800202E4 type=21 target=sub_800202E4 line=`sw      $v0, 0xC4($gp)`
- 800202E4 -> 800202E8 type=21 target=loc_800202E8 line=`slti    $v0, 0x1F`
- 800202E8 -> 800202EC type=21 target=sub_800202EC line=`bnez    $v0, loc_800202F4`
- 800202E8 -> 800202F4 type=19 target=loc_800202F4 line=`bnez    $v0, loc_800202F4`
- 800202EC -> 800202F0 type=21 target=sub_800202F0 line=`li      $v0, 0xC0`
- 800202F0 -> 800202F4 type=21 target=loc_800202F4 line=`sw      $v0, 0xC4($gp)`
- 800202F4 -> 800202F8 type=21 target=sub_800202F8 line=`lw      $ra, 0x10+var_s4($sp)`
- 800202F8 -> 800202FC type=21 target=sub_800202FC line=`lw      $s0, 0x10+var_s0($sp)`
- 800202FC -> 80020300 type=21 target=sub_80020300 line=`addiu   $sp, 0x18`
- 80020300 -> 80020304 type=21 target=sub_80020304 line=`jr      $ra`

### Full disasm
```asm
80020248: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002024C: addiu   $sp, -0x18                                               ; E8 FF BD 27
80020250: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
80020254: move    $s0, $a0                                                 ; 21 80 80 00
80020258: bnez    $v0, loc_80020268                                        ; 03 00 40 14
8002025C: sw      $ra, 0x10+var_s4($sp)                                    ; 14 00 BF AF
80020260: jal     sub_800271E4                                             ; 79 9C 00 0C
80020264: li      $a0, 1                                                   ; 01 00 04 34
80020268: bnez    $s0, loc_80020290                                        ; 09 00 00 16
8002026C: nop                                                              ; 00 00 00 00
80020270: jal     sub_8001CE30                                             ; 8C 73 00 0C
80020274: li      $a0, 5                                                   ; 05 00 04 34
80020278: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002027C: nop                                                              ; 00 00 00 00
80020280: addiu   $v0, 1                                                   ; 01 00 42 24
80020284: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
80020288: j       loc_800202E8                                             ; BA 80 00 08
8002028C: slti    $v0, 0x10                                                ; 10 00 42 28
80020290: lw      $v1, 0xC4($gp)                                           ; C4 00 83 8F
80020294: li      $v0, 0xF                                                 ; 0F 00 02 34
80020298: bne     $v1, $v0, loc_800202A8                                   ; 03 00 62 14
8002029C: nop                                                              ; 00 00 00 00
800202A0: jal     sub_800271E4                                             ; 79 9C 00 0C
800202A4: move    $a0, $zero                                               ; 21 20 00 00
800202A8: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
800202AC: nop                                                              ; 00 00 00 00
800202B0: slti    $v0, 0xF                                                 ; 0F 00 42 28
800202B4: beqz    $v0, loc_800202CC                                        ; 05 00 40 10
800202B8: nop                                                              ; 00 00 00 00
800202BC: jal     sub_8001F230                                             ; 8C 7C 00 0C
800202C0: li      $a0, 5                                                   ; 05 00 04 34
800202C4: j       loc_800202D4                                             ; B5 80 00 08
800202C8: nop                                                              ; 00 00 00 00
800202CC: jal     sub_8001C864                                             ; 19 72 00 0C
800202D0: li      $a0, 5                                                   ; 05 00 04 34
800202D4: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
800202D8: nop                                                              ; 00 00 00 00
800202DC: addiu   $v0, 1                                                   ; 01 00 42 24
800202E0: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
800202E4: slti    $v0, 0x1F                                                ; 1F 00 42 28
800202E8: bnez    $v0, loc_800202F4                                        ; 02 00 40 14
800202EC: li      $v0, 0xC0                                                ; C0 00 02 34
800202F0: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
800202F4: lw      $ra, 0x10+var_s4($sp)                                    ; 14 00 BF 8F
800202F8: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
800202FC: addiu   $sp, 0x18                                                ; 18 00 BD 27
80020300: jr      $ra                                                      ; 08 00 E0 03
80020304: nop                                                              ; 00 00 00 00
```

## `80020308` `sub_80020308`
Bounds: `80020308-800203D4`

### Pseudocode
```c
int __fastcall sub_80020308(int a1)
{
  int v1; // $gp
  int v3; // $gp
  int v4; // $s0
  int v5; // $v0
  BOOL v6; // $v0
  int v7; // $v0
  bool v8; // dc
  int result; // $v0

  if ( !*(_DWORD *)(v1 + 196) )
    sub_800271E4(0);
  if ( a1 )
  {
    if ( *(_DWORD *)(v1 + 196) == 15 )
      sub_800271E4(1);
    v4 = 0;
    if ( *(int *)(v1 + 196) >= 15 )
    {
      v4 = 1;
      sub_8001FEB4(0);
    }
    else
    {
      sub_8001F230(5);
    }
    v7 = *(_DWORD *)(v3 + 196) + 1;
    *(_DWORD *)(v3 + 196) = v7;
    v6 = v7 < 31;
  }
  else
  {
    sub_8001FEB4(0);
    v4 = 0;
    v5 = *(_DWORD *)(v3 + 196) + 1;
    *(_DWORD *)(v3 + 196) = v5;
    v6 = v5 < 16;
  }
  v8 = v6;
  result = v4;
  if ( !v8 )
  {
    *(_DWORD *)(v3 + 196) = 192;
    return v4;
  }
  return result;
}

```

### Calls in function
- 80020318: `bnez    $v0, loc_80020328` -> 80020328 loc_80020328
- 80020320: `jal     sub_800271E4` -> 800271E4 sub_800271E4
- 80020328: `bnez    $s0, loc_80020350` -> 80020350 loc_80020350
- 80020330: `jal     sub_8001FEB4` -> 8001FEB4 sub_8001FEB4
- 80020348: `j       loc_800203AC` -> 800203AC loc_800203AC
- 80020358: `bne     $v1, $v0, loc_80020368` -> 80020368 loc_80020368
- 80020360: `jal     sub_800271E4` -> 800271E4 sub_800271E4
- 80020374: `beqz    $v0, loc_8002038C` -> 8002038C loc_8002038C
- 8002037C: `jal     sub_8001F230` -> 8001F230 sub_8001F230
- 80020384: `j       loc_80020398` -> 80020398 loc_80020398
- 80020390: `jal     sub_8001FEB4` -> 8001FEB4 sub_8001FEB4
- 800203AC: `bnez    $v0, loc_800203C0` -> 800203C0 loc_800203C0

### Xrefs to
- 8001EBB0 -> 80020308 type=17 func=sub_8001EA74 line=`jal     sub_80020308`

### Xrefs from
- 80020308 -> 8002030C type=21 target=sub_8002030C line=`lw      $v0, 0xC4($gp)`
- 8002030C -> 80020310 type=21 target=sub_80020310 line=`addiu   $sp, -0x18`
- 80020310 -> 80020314 type=21 target=sub_80020314 line=`sw      $s0, 0x10+var_s0($sp)`
- 80020314 -> 80020318 type=21 target=sub_80020318 line=`move    $s0, $a0`
- 80020318 -> 8002031C type=21 target=sub_8002031C line=`bnez    $v0, loc_80020328`
- 80020318 -> 80020328 type=19 target=loc_80020328 line=`bnez    $v0, loc_80020328`
- 8002031C -> 80020320 type=21 target=sub_80020320 line=`sw      $ra, 0x10+var_s4($sp)`
- 80020320 -> 80020324 type=21 target=sub_80020324 line=`jal     sub_800271E4`
- 80020320 -> 800271E4 type=17 target=sub_800271E4 line=`jal     sub_800271E4`
- 80020324 -> 80020328 type=21 target=loc_80020328 line=`move    $a0, $zero`
- 80020328 -> 8002032C type=21 target=sub_8002032C line=`bnez    $s0, loc_80020350`
- 80020328 -> 80020350 type=19 target=loc_80020350 line=`bnez    $s0, loc_80020350`
- 8002032C -> 80020330 type=21 target=sub_80020330 line=`nop`
- 80020330 -> 80020334 type=21 target=sub_80020334 line=`jal     sub_8001FEB4`
- 80020330 -> 8001FEB4 type=17 target=sub_8001FEB4 line=`jal     sub_8001FEB4`
- 80020334 -> 80020338 type=21 target=sub_80020338 line=`move    $a0, $zero`
- 80020338 -> 8002033C type=21 target=sub_8002033C line=`lw      $v0, 0xC4($gp)`
- 8002033C -> 80020340 type=21 target=sub_80020340 line=`move    $s0, $zero`
- 80020340 -> 80020344 type=21 target=sub_80020344 line=`addiu   $v0, 1`
- 80020344 -> 80020348 type=21 target=sub_80020348 line=`sw      $v0, 0xC4($gp)`
- 80020348 -> 8002034C type=21 target=sub_8002034C line=`j       loc_800203AC`
- 80020348 -> 800203AC type=19 target=loc_800203AC line=`j       loc_800203AC`
- 80020350 -> 80020354 type=21 target=sub_80020354 line=`lw      $v1, 0xC4($gp)`
- 80020354 -> 80020358 type=21 target=sub_80020358 line=`li      $v0, 0xF`
- 80020358 -> 8002035C type=21 target=sub_8002035C line=`bne     $v1, $v0, loc_80020368`
- 80020358 -> 80020368 type=19 target=loc_80020368 line=`bne     $v1, $v0, loc_80020368`
- 8002035C -> 80020360 type=21 target=sub_80020360 line=`nop`
- 80020360 -> 80020364 type=21 target=sub_80020364 line=`jal     sub_800271E4`
- 80020360 -> 800271E4 type=17 target=sub_800271E4 line=`jal     sub_800271E4`
- 80020364 -> 80020368 type=21 target=loc_80020368 line=`li      $a0, 1`
- 80020368 -> 8002036C type=21 target=sub_8002036C line=`lw      $v0, 0xC4($gp)`
- 8002036C -> 80020370 type=21 target=sub_80020370 line=`nop`
- 80020370 -> 80020374 type=21 target=sub_80020374 line=`slti    $v0, 0xF`
- 80020374 -> 80020378 type=21 target=sub_80020378 line=`beqz    $v0, loc_8002038C`
- 80020374 -> 8002038C type=19 target=loc_8002038C line=`beqz    $v0, loc_8002038C`
- 80020378 -> 8002037C type=21 target=sub_8002037C line=`move    $s0, $zero`
- 8002037C -> 80020380 type=21 target=sub_80020380 line=`jal     sub_8001F230`
- 8002037C -> 8001F230 type=17 target=sub_8001F230 line=`jal     sub_8001F230`
- 80020380 -> 80020384 type=21 target=sub_80020384 line=`li      $a0, 5`
- 80020384 -> 80020388 type=21 target=sub_80020388 line=`j       loc_80020398`
- 80020384 -> 80020398 type=19 target=loc_80020398 line=`j       loc_80020398`
- 8002038C -> 80020390 type=21 target=sub_80020390 line=`li      $s0, 1`
- 80020390 -> 80020394 type=21 target=sub_80020394 line=`jal     sub_8001FEB4`
- 80020390 -> 8001FEB4 type=17 target=sub_8001FEB4 line=`jal     sub_8001FEB4`
- 80020394 -> 80020398 type=21 target=loc_80020398 line=`move    $a0, $zero`
- 80020398 -> 8002039C type=21 target=sub_8002039C line=`lw      $v0, 0xC4($gp)`
- 8002039C -> 800203A0 type=21 target=sub_800203A0 line=`nop`
- 800203A0 -> 800203A4 type=21 target=sub_800203A4 line=`addiu   $v0, 1`
- 800203A4 -> 800203A8 type=21 target=sub_800203A8 line=`sw      $v0, 0xC4($gp)`
- 800203A8 -> 800203AC type=21 target=loc_800203AC line=`slti    $v0, 0x1F`
- 800203AC -> 800203B0 type=21 target=sub_800203B0 line=`bnez    $v0, loc_800203C0`
- 800203AC -> 800203C0 type=19 target=loc_800203C0 line=`bnez    $v0, loc_800203C0`
- 800203B0 -> 800203B4 type=21 target=sub_800203B4 line=`move    $v0, $s0`
- 800203B4 -> 800203B8 type=21 target=sub_800203B8 line=`li      $v0, 0xC0`
- 800203B8 -> 800203BC type=21 target=sub_800203BC line=`sw      $v0, 0xC4($gp)`
- 800203BC -> 800203C0 type=21 target=loc_800203C0 line=`move    $v0, $s0`
- 800203C0 -> 800203C4 type=21 target=sub_800203C4 line=`lw      $ra, 0x10+var_s4($sp)`
- 800203C4 -> 800203C8 type=21 target=sub_800203C8 line=`lw      $s0, 0x10+var_s0($sp)`
- 800203C8 -> 800203CC type=21 target=sub_800203CC line=`addiu   $sp, 0x18`
- 800203CC -> 800203D0 type=21 target=sub_800203D0 line=`jr      $ra`

### Full disasm
```asm
80020308: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002030C: addiu   $sp, -0x18                                               ; E8 FF BD 27
80020310: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
80020314: move    $s0, $a0                                                 ; 21 80 80 00
80020318: bnez    $v0, loc_80020328                                        ; 03 00 40 14
8002031C: sw      $ra, 0x10+var_s4($sp)                                    ; 14 00 BF AF
80020320: jal     sub_800271E4                                             ; 79 9C 00 0C
80020324: move    $a0, $zero                                               ; 21 20 00 00
80020328: bnez    $s0, loc_80020350                                        ; 09 00 00 16
8002032C: nop                                                              ; 00 00 00 00
80020330: jal     sub_8001FEB4                                             ; AD 7F 00 0C
80020334: move    $a0, $zero                                               ; 21 20 00 00
80020338: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002033C: move    $s0, $zero                                               ; 21 80 00 00
80020340: addiu   $v0, 1                                                   ; 01 00 42 24
80020344: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
80020348: j       loc_800203AC                                             ; EB 80 00 08
8002034C: slti    $v0, 0x10                                                ; 10 00 42 28
80020350: lw      $v1, 0xC4($gp)                                           ; C4 00 83 8F
80020354: li      $v0, 0xF                                                 ; 0F 00 02 34
80020358: bne     $v1, $v0, loc_80020368                                   ; 03 00 62 14
8002035C: nop                                                              ; 00 00 00 00
80020360: jal     sub_800271E4                                             ; 79 9C 00 0C
80020364: li      $a0, 1                                                   ; 01 00 04 34
80020368: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002036C: nop                                                              ; 00 00 00 00
80020370: slti    $v0, 0xF                                                 ; 0F 00 42 28
80020374: beqz    $v0, loc_8002038C                                        ; 05 00 40 10
80020378: move    $s0, $zero                                               ; 21 80 00 00
8002037C: jal     sub_8001F230                                             ; 8C 7C 00 0C
80020380: li      $a0, 5                                                   ; 05 00 04 34
80020384: j       loc_80020398                                             ; E6 80 00 08
80020388: nop                                                              ; 00 00 00 00
8002038C: li      $s0, 1                                                   ; 01 00 10 34
80020390: jal     sub_8001FEB4                                             ; AD 7F 00 0C
80020394: move    $a0, $zero                                               ; 21 20 00 00
80020398: lw      $v0, 0xC4($gp)                                           ; C4 00 82 8F
8002039C: nop                                                              ; 00 00 00 00
800203A0: addiu   $v0, 1                                                   ; 01 00 42 24
800203A4: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
800203A8: slti    $v0, 0x1F                                                ; 1F 00 42 28
800203AC: bnez    $v0, loc_800203C0                                        ; 04 00 40 14
800203B0: move    $v0, $s0                                                 ; 21 10 00 02
800203B4: li      $v0, 0xC0                                                ; C0 00 02 34
800203B8: sw      $v0, 0xC4($gp)                                           ; C4 00 82 AF
800203BC: move    $v0, $s0                                                 ; 21 10 00 02
800203C0: lw      $ra, 0x10+var_s4($sp)                                    ; 14 00 BF 8F
800203C4: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
800203C8: addiu   $sp, 0x18                                                ; 18 00 BD 27
800203CC: jr      $ra                                                      ; 08 00 E0 03
800203D0: nop                                                              ; 00 00 00 00
```

## `8001F230` `sub_8001F230`
Bounds: `8001F230-8001F518`

### Pseudocode
```c
int __fastcall sub_8001F230(unsigned __int16 a1)
{
  int v2; // $s5
  int v3; // $s4
  int v4; // $s3
  int v5; // $s1
  int v6; // $s5
  int v7; // $s7
  int v8; // $s6
  int v9; // $s4
  int v10; // $s3

  sub_8001C550(20, 20, (int)dword_80050380, a1);
  sub_8001C550(280, 20, (int)dword_80050390, a1);
  sub_8001C550(40, 20, (int)dword_800503A0, a1);
  sub_8001C550(160, 20, (int)dword_800503B0, a1);
  sub_8001C550(40, 160, (int)dword_800503C0, a1);
  sub_8001C550(160, 160, (int)dword_800503D0, a1);
  v2 = 0;
  v3 = 1310720;
  v4 = 2621440;
  LOWORD(v5) = 40;
  do
  {
    sub_8001C550(v5, 0, (int)dword_800503E0, a1);
    sub_8001C550(SHIWORD(v3), 0, (int)dword_800503F0, a1);
    sub_8001C550(v5, 220, (int)dword_80050400, a1);
    sub_8001C550(SHIWORD(v3), 220, (int)dword_80050410, a1);
    v3 += 2621440;
    v4 += 2621440;
    ++v2;
    v5 = v4 >> 16;
  }
  while ( v2 < 7 );
  v6 = 0;
  v7 = 3932160;
  v8 = 2621440;
  v9 = 0;
  v10 = 1310720;
  do
  {
    sub_8001C550(0, SHIWORD(v10), (int)dword_80050400, a1);
    sub_8001C550(0, SHIWORD(v9), (int)dword_800503E0, a1);
    sub_8001C550(300, SHIWORD(v10), (int)dword_80050410, a1);
    sub_8001C550(300, SHIWORD(v9), (int)dword_800503F0, a1);
    sub_8001C550(SHIWORD(v8), 200, (int)dword_800503E0, a1);
    sub_8001C550(SHIWORD(v7), 200, (int)dword_800503F0, a1);
    sub_8001C550(SHIWORD(v8), 180, (int)dword_80050400, a1);
    sub_8001C550(SHIWORD(v7), 180, (int)dword_80050410, a1);
    v7 += 2621440;
    v8 += 2621440;
    v9 += 2621440;
    v10 += 2621440;
    ++v6;
  }
  while ( v6 < 6 );
  sub_8001C550(280, 200, (int)dword_800503E0, a1);
  sub_8001C550(20, 200, (int)dword_800503F0, a1);
  sub_8001C550(280, 180, (int)dword_80050400, a1);
  return sub_8001C550(20, 180, (int)dword_80050410, a1);
}

```

### Calls in function
- 8001F270: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F288: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F2A0: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F2B8: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F2D0: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F2E8: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F310: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F32C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F344: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F35C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F378: `bnez    $v0, loc_8001F300` -> 8001F300 loc_8001F300
- 8001F3B0: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F3C8: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F3E0: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F3F8: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F410: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F42C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F444: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F45C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F480: `bnez    $v0, loc_8001F3A0` -> 8001F3A0 loc_8001F3A0
- 8001F494: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F4AC: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F4C4: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001F4DC: `jal     sub_8001C550` -> 8001C550 sub_8001C550

### Xrefs to
- 800202BC -> 8001F230 type=17 func=sub_80020248 line=`jal     sub_8001F230`
- 8002037C -> 8001F230 type=17 func=sub_80020308 line=`jal     sub_8001F230`

### Xrefs from
- 8001F230 -> 8001F234 type=21 target=sub_8001F234 line=`addiu   $sp, -0x38`
- 8001F234 -> 8001F238 type=21 target=sub_8001F238 line=`sw      $s2, 0x10+var_s8($sp)`
- 8001F238 -> 8001F23C type=21 target=sub_8001F23C line=`move    $s2, $a0`
- 8001F23C -> 8001F240 type=21 target=sub_8001F240 line=`li      $a0, 0x14`
- 8001F240 -> 8001F244 type=21 target=sub_8001F244 line=`li      $a1, 0x14`
- 8001F244 -> 8001F24C type=21 target=sub_8001F24C line=`li      $a2, dword_80050380`
- 8001F24C -> 8001F250 type=21 target=sub_8001F250 line=`move    $a3, $s2`
- 8001F250 -> 8001F254 type=21 target=sub_8001F254 line=`sw      $ra, 0x10+var_s24($sp)`
- 8001F254 -> 8001F258 type=21 target=sub_8001F258 line=`sw      $fp, 0x10+var_s20($sp)`
- 8001F258 -> 8001F25C type=21 target=sub_8001F25C line=`sw      $s7, 0x10+var_s1C($sp)`
- 8001F25C -> 8001F260 type=21 target=sub_8001F260 line=`sw      $s6, 0x10+var_s18($sp)`
- 8001F260 -> 8001F264 type=21 target=sub_8001F264 line=`sw      $s5, 0x10+var_s14($sp)`
- 8001F264 -> 8001F268 type=21 target=sub_8001F268 line=`sw      $s4, 0x10+var_s10($sp)`
- 8001F268 -> 8001F26C type=21 target=sub_8001F26C line=`sw      $s3, 0x10+var_sC($sp)`
- 8001F26C -> 8001F270 type=21 target=sub_8001F270 line=`sw      $s1, 0x10+var_s4($sp)`
- 8001F270 -> 8001F274 type=21 target=sub_8001F274 line=`jal     sub_8001C550`
- 8001F270 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F274 -> 8001F278 type=21 target=sub_8001F278 line=`sw      $s0, 0x10+var_s0($sp)`
- 8001F278 -> 8001F27C type=21 target=sub_8001F27C line=`li      $a0, 0x118`
- 8001F27C -> 8001F280 type=21 target=sub_8001F280 line=`li      $a1, 0x14`
- 8001F280 -> 8001F288 type=21 target=sub_8001F288 line=`li      $a2, dword_80050390`
- 8001F288 -> 8001F28C type=21 target=sub_8001F28C line=`jal     sub_8001C550`
- 8001F288 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F28C -> 8001F290 type=21 target=sub_8001F290 line=`move    $a3, $s2`
- 8001F290 -> 8001F294 type=21 target=sub_8001F294 line=`li      $a0, 0x28  # '('`
- 8001F294 -> 8001F298 type=21 target=sub_8001F298 line=`li      $a1, 0x14`
- 8001F298 -> 8001F2A0 type=21 target=sub_8001F2A0 line=`li      $a2, dword_800503A0`
- 8001F2A0 -> 8001F2A4 type=21 target=sub_8001F2A4 line=`jal     sub_8001C550`
- 8001F2A0 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F2A4 -> 8001F2A8 type=21 target=sub_8001F2A8 line=`move    $a3, $s2`
- 8001F2A8 -> 8001F2AC type=21 target=sub_8001F2AC line=`li      $a0, 0xA0`
- 8001F2AC -> 8001F2B0 type=21 target=sub_8001F2B0 line=`li      $a1, 0x14`
- 8001F2B0 -> 8001F2B8 type=21 target=sub_8001F2B8 line=`li      $a2, dword_800503B0`
- 8001F2B8 -> 8001F2BC type=21 target=sub_8001F2BC line=`jal     sub_8001C550`
- 8001F2B8 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F2BC -> 8001F2C0 type=21 target=sub_8001F2C0 line=`move    $a3, $s2`
- 8001F2C0 -> 8001F2C4 type=21 target=sub_8001F2C4 line=`li      $a0, 0x28  # '('`
- 8001F2C4 -> 8001F2C8 type=21 target=sub_8001F2C8 line=`li      $a1, 0xA0`
- 8001F2C8 -> 8001F2D0 type=21 target=sub_8001F2D0 line=`li      $a2, dword_800503C0`
- 8001F2D0 -> 8001F2D4 type=21 target=sub_8001F2D4 line=`jal     sub_8001C550`
- 8001F2D0 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F2D4 -> 8001F2D8 type=21 target=sub_8001F2D8 line=`move    $a3, $s2`
- 8001F2D8 -> 8001F2DC type=21 target=sub_8001F2DC line=`li      $a0, 0xA0`
- 8001F2DC -> 8001F2E0 type=21 target=sub_8001F2E0 line=`li      $a1, 0xA0`
- 8001F2E0 -> 8001F2E8 type=21 target=sub_8001F2E8 line=`li      $a2, dword_800503D0`
- 8001F2E8 -> 8001F2EC type=21 target=sub_8001F2EC line=`jal     sub_8001C550`
- 8001F2E8 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F2EC -> 8001F2F0 type=21 target=sub_8001F2F0 line=`move    $a3, $s2`
- 8001F2F0 -> 8001F2F4 type=21 target=sub_8001F2F4 line=`move    $s5, $zero`
- 8001F2F4 -> 8001F2F8 type=21 target=sub_8001F2F8 line=`lui     $s4, 0x14`
- 8001F2F8 -> 8001F2FC type=21 target=sub_8001F2FC line=`lui     $s3, 0x28  # '('`
- 8001F2FC -> 8001F300 type=21 target=loc_8001F300 line=`sra     $s1, $s3, 16`
- 8001F300 -> 8001F304 type=21 target=sub_8001F304 line=`move    $a0, $s1`
- 8001F304 -> 8001F308 type=21 target=sub_8001F308 line=`move    $a1, $zero`
- 8001F308 -> 8001F310 type=21 target=sub_8001F310 line=`li      $a2, dword_800503E0`
- 8001F310 -> 8001F314 type=21 target=sub_8001F314 line=`jal     sub_8001C550`
- 8001F310 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F314 -> 8001F318 type=21 target=sub_8001F318 line=`move    $a3, $s2`
- 8001F318 -> 8001F31C type=21 target=sub_8001F31C line=`sra     $s0, $s4, 16`
- 8001F31C -> 8001F320 type=21 target=sub_8001F320 line=`move    $a0, $s0`
- 8001F320 -> 8001F324 type=21 target=sub_8001F324 line=`move    $a1, $zero`
- 8001F324 -> 8001F32C type=21 target=sub_8001F32C line=`li      $a2, dword_800503F0`
- 8001F32C -> 8001F330 type=21 target=sub_8001F330 line=`jal     sub_8001C550`
- 8001F32C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F330 -> 8001F334 type=21 target=sub_8001F334 line=`move    $a3, $s2`
- 8001F334 -> 8001F338 type=21 target=sub_8001F338 line=`move    $a0, $s1`
- 8001F338 -> 8001F33C type=21 target=sub_8001F33C line=`li      $a1, 0xDC`
- 8001F33C -> 8001F344 type=21 target=sub_8001F344 line=`li      $a2, dword_80050400`
- 8001F344 -> 8001F348 type=21 target=sub_8001F348 line=`jal     sub_8001C550`
- 8001F344 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F348 -> 8001F34C type=21 target=sub_8001F34C line=`move    $a3, $s2`
- 8001F34C -> 8001F350 type=21 target=sub_8001F350 line=`move    $a0, $s0`
- 8001F350 -> 8001F354 type=21 target=sub_8001F354 line=`li      $a1, 0xDC`
- 8001F354 -> 8001F35C type=21 target=sub_8001F35C line=`li      $a2, dword_80050410`
- 8001F35C -> 8001F360 type=21 target=sub_8001F360 line=`jal     sub_8001C550`
- 8001F35C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F360 -> 8001F364 type=21 target=sub_8001F364 line=`move    $a3, $s2`
- 8001F364 -> 8001F368 type=21 target=sub_8001F368 line=`lui     $v0, 0x28  # '('`
- 8001F368 -> 8001F36C type=21 target=sub_8001F36C line=`addu    $s4, $v0`
- 8001F36C -> 8001F370 type=21 target=sub_8001F370 line=`addu    $s3, $v0`
- 8001F370 -> 8001F374 type=21 target=sub_8001F374 line=`addiu   $s5, 1`
- 8001F374 -> 8001F378 type=21 target=sub_8001F378 line=`slti    $v0, $s5, 7`
- 8001F378 -> 8001F37C type=21 target=sub_8001F37C line=`bnez    $v0, loc_8001F300`
- 8001F378 -> 8001F300 type=19 target=loc_8001F300 line=`bnez    $v0, loc_8001F300`
- 8001F37C -> 8001F380 type=21 target=sub_8001F380 line=`sra     $s1, $s3, 16`
- 8001F380 -> 8001F384 type=21 target=sub_8001F384 line=`move    $s5, $zero`
- 8001F384 -> 8001F38C type=21 target=sub_8001F38C line=`li      $fp, dword_800503E0`
- 8001F38C -> 8001F390 type=21 target=sub_8001F390 line=`lui     $s7, 0x3C  # '<'`
- 8001F390 -> 8001F394 type=21 target=sub_8001F394 line=`lui     $s6, 0x28  # '('`
- 8001F394 -> 8001F398 type=21 target=sub_8001F398 line=`move    $s4, $zero`
- 8001F398 -> 8001F39C type=21 target=sub_8001F39C line=`lui     $s3, 0x14`
- 8001F39C -> 8001F3A0 type=21 target=loc_8001F3A0 line=`move    $a0, $zero`
- 8001F3A0 -> 8001F3A4 type=21 target=sub_8001F3A4 line=`sra     $s1, $s3, 16`
- 8001F3A4 -> 8001F3A8 type=21 target=sub_8001F3A8 line=`move    $a1, $s1`
- 8001F3A8 -> 8001F3B0 type=21 target=sub_8001F3B0 line=`li      $a2, dword_80050400`
- 8001F3B0 -> 8001F3B4 type=21 target=sub_8001F3B4 line=`jal     sub_8001C550`
- 8001F3B0 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F3B4 -> 8001F3B8 type=21 target=sub_8001F3B8 line=`move    $a3, $s2`
- 8001F3B8 -> 8001F3BC type=21 target=sub_8001F3BC line=`move    $a0, $zero`
- 8001F3BC -> 8001F3C0 type=21 target=sub_8001F3C0 line=`sra     $s0, $s4, 16`
- 8001F3C0 -> 8001F3C4 type=21 target=sub_8001F3C4 line=`move    $a1, $s0`
- 8001F3C4 -> 8001F3C8 type=21 target=sub_8001F3C8 line=`move    $a2, $fp`
- 8001F3C8 -> 8001F3CC type=21 target=sub_8001F3CC line=`jal     sub_8001C550`
- 8001F3C8 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F3CC -> 8001F3D0 type=21 target=sub_8001F3D0 line=`move    $a3, $s2`
- 8001F3D0 -> 8001F3D4 type=21 target=sub_8001F3D4 line=`li      $a0, 0x12C`
- 8001F3D4 -> 8001F3D8 type=21 target=sub_8001F3D8 line=`move    $a1, $s1`
- 8001F3D8 -> 8001F3E0 type=21 target=sub_8001F3E0 line=`li      $a2, dword_80050410`
- 8001F3E0 -> 8001F3E4 type=21 target=sub_8001F3E4 line=`jal     sub_8001C550`
- 8001F3E0 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F3E4 -> 8001F3E8 type=21 target=sub_8001F3E8 line=`move    $a3, $s2`
- 8001F3E8 -> 8001F3EC type=21 target=sub_8001F3EC line=`li      $a0, 0x12C`
- 8001F3EC -> 8001F3F0 type=21 target=sub_8001F3F0 line=`move    $a1, $s0`
- 8001F3F0 -> 8001F3F8 type=21 target=sub_8001F3F8 line=`li      $a2, dword_800503F0`
- 8001F3F8 -> 8001F3FC type=21 target=sub_8001F3FC line=`jal     sub_8001C550`
- 8001F3F8 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F3FC -> 8001F400 type=21 target=sub_8001F400 line=`move    $a3, $s2`
- 8001F400 -> 8001F404 type=21 target=sub_8001F404 line=`sra     $s1, $s6, 16`
- 8001F404 -> 8001F408 type=21 target=sub_8001F408 line=`move    $a0, $s1`
- 8001F408 -> 8001F40C type=21 target=sub_8001F40C line=`li      $a1, 0xC8`
- 8001F40C -> 8001F410 type=21 target=sub_8001F410 line=`move    $a2, $fp`
- 8001F410 -> 8001F414 type=21 target=sub_8001F414 line=`jal     sub_8001C550`
- 8001F410 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F414 -> 8001F418 type=21 target=sub_8001F418 line=`move    $a3, $s2`
- 8001F418 -> 8001F41C type=21 target=sub_8001F41C line=`sra     $s0, $s7, 16`
- 8001F41C -> 8001F420 type=21 target=sub_8001F420 line=`move    $a0, $s0`
- 8001F420 -> 8001F424 type=21 target=sub_8001F424 line=`li      $a1, 0xC8`
- 8001F424 -> 8001F42C type=21 target=sub_8001F42C line=`li      $a2, dword_800503F0`
- 8001F42C -> 8001F430 type=21 target=sub_8001F430 line=`jal     sub_8001C550`
- 8001F42C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F430 -> 8001F434 type=21 target=sub_8001F434 line=`move    $a3, $s2`
- 8001F434 -> 8001F438 type=21 target=sub_8001F438 line=`move    $a0, $s1`
- 8001F438 -> 8001F43C type=21 target=sub_8001F43C line=`li      $a1, 0xB4`
- 8001F43C -> 8001F444 type=21 target=sub_8001F444 line=`li      $a2, dword_80050400`
- 8001F444 -> 8001F448 type=21 target=sub_8001F448 line=`jal     sub_8001C550`
- 8001F444 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F448 -> 8001F44C type=21 target=sub_8001F44C line=`move    $a3, $s2`
- 8001F44C -> 8001F450 type=21 target=sub_8001F450 line=`move    $a0, $s0`
- 8001F450 -> 8001F454 type=21 target=sub_8001F454 line=`li      $a1, 0xB4`
- 8001F454 -> 8001F45C type=21 target=sub_8001F45C line=`li      $a2, dword_80050410`
- 8001F45C -> 8001F460 type=21 target=sub_8001F460 line=`jal     sub_8001C550`
- 8001F45C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F460 -> 8001F464 type=21 target=sub_8001F464 line=`move    $a3, $s2`
- 8001F464 -> 8001F468 type=21 target=sub_8001F468 line=`lui     $v0, 0x28  # '('`
- 8001F468 -> 8001F46C type=21 target=sub_8001F46C line=`addu    $s7, $v0`
- 8001F46C -> 8001F470 type=21 target=sub_8001F470 line=`addu    $s6, $v0`
- 8001F470 -> 8001F474 type=21 target=sub_8001F474 line=`addu    $s4, $v0`
- 8001F474 -> 8001F478 type=21 target=sub_8001F478 line=`addu    $s3, $v0`
- 8001F478 -> 8001F47C type=21 target=sub_8001F47C line=`addiu   $s5, 1`
- 8001F47C -> 8001F480 type=21 target=sub_8001F480 line=`slti    $v0, $s5, 6`
- 8001F480 -> 8001F484 type=21 target=sub_8001F484 line=`bnez    $v0, loc_8001F3A0`
- 8001F480 -> 8001F3A0 type=19 target=loc_8001F3A0 line=`bnez    $v0, loc_8001F3A0`
- 8001F484 -> 8001F488 type=21 target=sub_8001F488 line=`move    $a0, $zero`
- 8001F488 -> 8001F48C type=21 target=sub_8001F48C line=`li      $a0, 0x118`
- 8001F48C -> 8001F490 type=21 target=sub_8001F490 line=`li      $a1, 0xC8`
- 8001F490 -> 8001F494 type=21 target=sub_8001F494 line=`move    $a2, $fp`
- 8001F494 -> 8001F498 type=21 target=sub_8001F498 line=`jal     sub_8001C550`
- 8001F494 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F498 -> 8001F49C type=21 target=sub_8001F49C line=`move    $a3, $s2`
- 8001F49C -> 8001F4A0 type=21 target=sub_8001F4A0 line=`li      $a0, 0x14`
- 8001F4A0 -> 8001F4A4 type=21 target=sub_8001F4A4 line=`li      $a1, 0xC8`
- 8001F4A4 -> 8001F4AC type=21 target=sub_8001F4AC line=`li      $a2, dword_800503F0`
- 8001F4AC -> 8001F4B0 type=21 target=sub_8001F4B0 line=`jal     sub_8001C550`
- 8001F4AC -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F4B0 -> 8001F4B4 type=21 target=sub_8001F4B4 line=`move    $a3, $s2`
- 8001F4B4 -> 8001F4B8 type=21 target=sub_8001F4B8 line=`li      $a0, 0x118`
- 8001F4B8 -> 8001F4BC type=21 target=sub_8001F4BC line=`li      $a1, 0xB4`
- 8001F4BC -> 8001F4C4 type=21 target=sub_8001F4C4 line=`li      $a2, dword_80050400`
- 8001F4C4 -> 8001F4C8 type=21 target=sub_8001F4C8 line=`jal     sub_8001C550`
- 8001F4C4 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F4C8 -> 8001F4CC type=21 target=sub_8001F4CC line=`move    $a3, $s2`
- 8001F4CC -> 8001F4D0 type=21 target=sub_8001F4D0 line=`li      $a0, 0x14`
- 8001F4D0 -> 8001F4D4 type=21 target=sub_8001F4D4 line=`li      $a1, 0xB4`
- 8001F4D4 -> 8001F4DC type=21 target=sub_8001F4DC line=`li      $a2, dword_80050410`
- 8001F4DC -> 8001F4E0 type=21 target=sub_8001F4E0 line=`jal     sub_8001C550`
- 8001F4DC -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001F4E0 -> 8001F4E4 type=21 target=sub_8001F4E4 line=`move    $a3, $s2`
- 8001F4E4 -> 8001F4E8 type=21 target=sub_8001F4E8 line=`lw      $ra, 0x10+var_s24($sp)`
- 8001F4E8 -> 8001F4EC type=21 target=sub_8001F4EC line=`lw      $fp, 0x10+var_s20($sp)`
- 8001F4EC -> 8001F4F0 type=21 target=sub_8001F4F0 line=`lw      $s7, 0x10+var_s1C($sp)`
- 8001F4F0 -> 8001F4F4 type=21 target=sub_8001F4F4 line=`lw      $s6, 0x10+var_s18($sp)`
- 8001F4F4 -> 8001F4F8 type=21 target=sub_8001F4F8 line=`lw      $s5, 0x10+var_s14($sp)`
- 8001F4F8 -> 8001F4FC type=21 target=sub_8001F4FC line=`lw      $s4, 0x10+var_s10($sp)`
- 8001F4FC -> 8001F500 type=21 target=sub_8001F500 line=`lw      $s3, 0x10+var_sC($sp)`
- 8001F500 -> 8001F504 type=21 target=sub_8001F504 line=`lw      $s2, 0x10+var_s8($sp)`
- 8001F504 -> 8001F508 type=21 target=sub_8001F508 line=`lw      $s1, 0x10+var_s4($sp)`
- 8001F508 -> 8001F50C type=21 target=sub_8001F50C line=`lw      $s0, 0x10+var_s0($sp)`
- 8001F50C -> 8001F510 type=21 target=sub_8001F510 line=`addiu   $sp, 0x38`
- 8001F510 -> 8001F514 type=21 target=sub_8001F514 line=`jr      $ra`

### Full disasm
```asm
8001F230: addiu   $sp, -0x38                                               ; C8 FF BD 27
8001F234: sw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 AF
8001F238: move    $s2, $a0                                                 ; 21 90 80 00
8001F23C: li      $a0, 0x14                                                ; 14 00 04 34
8001F240: li      $a1, 0x14                                                ; 14 00 05 34
8001F244: li      $a2, dword_80050380                                      ; 05 80 06 3C 80 03 C6 24
8001F24C: move    $a3, $s2                                                 ; 21 38 40 02
8001F250: sw      $ra, 0x10+var_s24($sp)                                   ; 34 00 BF AF
8001F254: sw      $fp, 0x10+var_s20($sp)                                   ; 30 00 BE AF
8001F258: sw      $s7, 0x10+var_s1C($sp)                                   ; 2C 00 B7 AF
8001F25C: sw      $s6, 0x10+var_s18($sp)                                   ; 28 00 B6 AF
8001F260: sw      $s5, 0x10+var_s14($sp)                                   ; 24 00 B5 AF
8001F264: sw      $s4, 0x10+var_s10($sp)                                   ; 20 00 B4 AF
8001F268: sw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 AF
8001F26C: sw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 AF
8001F270: jal     sub_8001C550                                             ; 54 71 00 0C
8001F274: sw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 AF
8001F278: li      $a0, 0x118                                               ; 18 01 04 34
8001F27C: li      $a1, 0x14                                                ; 14 00 05 34
8001F280: li      $a2, dword_80050390                                      ; 05 80 06 3C 90 03 C6 24
8001F288: jal     sub_8001C550                                             ; 54 71 00 0C
8001F28C: move    $a3, $s2                                                 ; 21 38 40 02
8001F290: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001F294: li      $a1, 0x14                                                ; 14 00 05 34
8001F298: li      $a2, dword_800503A0                                      ; 05 80 06 3C A0 03 C6 24
8001F2A0: jal     sub_8001C550                                             ; 54 71 00 0C
8001F2A4: move    $a3, $s2                                                 ; 21 38 40 02
8001F2A8: li      $a0, 0xA0                                                ; A0 00 04 34
8001F2AC: li      $a1, 0x14                                                ; 14 00 05 34
8001F2B0: li      $a2, dword_800503B0                                      ; 05 80 06 3C B0 03 C6 24
8001F2B8: jal     sub_8001C550                                             ; 54 71 00 0C
8001F2BC: move    $a3, $s2                                                 ; 21 38 40 02
8001F2C0: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001F2C4: li      $a1, 0xA0                                                ; A0 00 05 34
8001F2C8: li      $a2, dword_800503C0                                      ; 05 80 06 3C C0 03 C6 24
8001F2D0: jal     sub_8001C550                                             ; 54 71 00 0C
8001F2D4: move    $a3, $s2                                                 ; 21 38 40 02
8001F2D8: li      $a0, 0xA0                                                ; A0 00 04 34
8001F2DC: li      $a1, 0xA0                                                ; A0 00 05 34
8001F2E0: li      $a2, dword_800503D0                                      ; 05 80 06 3C D0 03 C6 24
8001F2E8: jal     sub_8001C550                                             ; 54 71 00 0C
8001F2EC: move    $a3, $s2                                                 ; 21 38 40 02
8001F2F0: move    $s5, $zero                                               ; 21 A8 00 00
8001F2F4: lui     $s4, 0x14                                                ; 14 00 14 3C
8001F2F8: lui     $s3, 0x28  # '('                                         ; 28 00 13 3C
8001F2FC: sra     $s1, $s3, 16                                             ; 03 8C 13 00
8001F300: move    $a0, $s1                                                 ; 21 20 20 02
8001F304: move    $a1, $zero                                               ; 21 28 00 00
8001F308: li      $a2, dword_800503E0                                      ; 05 80 06 3C E0 03 C6 24
8001F310: jal     sub_8001C550                                             ; 54 71 00 0C
8001F314: move    $a3, $s2                                                 ; 21 38 40 02
8001F318: sra     $s0, $s4, 16                                             ; 03 84 14 00
8001F31C: move    $a0, $s0                                                 ; 21 20 00 02
8001F320: move    $a1, $zero                                               ; 21 28 00 00
8001F324: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001F32C: jal     sub_8001C550                                             ; 54 71 00 0C
8001F330: move    $a3, $s2                                                 ; 21 38 40 02
8001F334: move    $a0, $s1                                                 ; 21 20 20 02
8001F338: li      $a1, 0xDC                                                ; DC 00 05 34
8001F33C: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001F344: jal     sub_8001C550                                             ; 54 71 00 0C
8001F348: move    $a3, $s2                                                 ; 21 38 40 02
8001F34C: move    $a0, $s0                                                 ; 21 20 00 02
8001F350: li      $a1, 0xDC                                                ; DC 00 05 34
8001F354: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001F35C: jal     sub_8001C550                                             ; 54 71 00 0C
8001F360: move    $a3, $s2                                                 ; 21 38 40 02
8001F364: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001F368: addu    $s4, $v0                                                 ; 21 A0 82 02
8001F36C: addu    $s3, $v0                                                 ; 21 98 62 02
8001F370: addiu   $s5, 1                                                   ; 01 00 B5 26
8001F374: slti    $v0, $s5, 7                                              ; 07 00 A2 2A
8001F378: bnez    $v0, loc_8001F300                                        ; E1 FF 40 14
8001F37C: sra     $s1, $s3, 16                                             ; 03 8C 13 00
8001F380: move    $s5, $zero                                               ; 21 A8 00 00
8001F384: li      $fp, dword_800503E0                                      ; 05 80 1E 3C E0 03 DE 27
8001F38C: lui     $s7, 0x3C  # '<'                                         ; 3C 00 17 3C
8001F390: lui     $s6, 0x28  # '('                                         ; 28 00 16 3C
8001F394: move    $s4, $zero                                               ; 21 A0 00 00
8001F398: lui     $s3, 0x14                                                ; 14 00 13 3C
8001F39C: move    $a0, $zero                                               ; 21 20 00 00
8001F3A0: sra     $s1, $s3, 16                                             ; 03 8C 13 00
8001F3A4: move    $a1, $s1                                                 ; 21 28 20 02
8001F3A8: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001F3B0: jal     sub_8001C550                                             ; 54 71 00 0C
8001F3B4: move    $a3, $s2                                                 ; 21 38 40 02
8001F3B8: move    $a0, $zero                                               ; 21 20 00 00
8001F3BC: sra     $s0, $s4, 16                                             ; 03 84 14 00
8001F3C0: move    $a1, $s0                                                 ; 21 28 00 02
8001F3C4: move    $a2, $fp                                                 ; 21 30 C0 03
8001F3C8: jal     sub_8001C550                                             ; 54 71 00 0C
8001F3CC: move    $a3, $s2                                                 ; 21 38 40 02
8001F3D0: li      $a0, 0x12C                                               ; 2C 01 04 34
8001F3D4: move    $a1, $s1                                                 ; 21 28 20 02
8001F3D8: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001F3E0: jal     sub_8001C550                                             ; 54 71 00 0C
8001F3E4: move    $a3, $s2                                                 ; 21 38 40 02
8001F3E8: li      $a0, 0x12C                                               ; 2C 01 04 34
8001F3EC: move    $a1, $s0                                                 ; 21 28 00 02
8001F3F0: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001F3F8: jal     sub_8001C550                                             ; 54 71 00 0C
8001F3FC: move    $a3, $s2                                                 ; 21 38 40 02
8001F400: sra     $s1, $s6, 16                                             ; 03 8C 16 00
8001F404: move    $a0, $s1                                                 ; 21 20 20 02
8001F408: li      $a1, 0xC8                                                ; C8 00 05 34
8001F40C: move    $a2, $fp                                                 ; 21 30 C0 03
8001F410: jal     sub_8001C550                                             ; 54 71 00 0C
8001F414: move    $a3, $s2                                                 ; 21 38 40 02
8001F418: sra     $s0, $s7, 16                                             ; 03 84 17 00
8001F41C: move    $a0, $s0                                                 ; 21 20 00 02
8001F420: li      $a1, 0xC8                                                ; C8 00 05 34
8001F424: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001F42C: jal     sub_8001C550                                             ; 54 71 00 0C
8001F430: move    $a3, $s2                                                 ; 21 38 40 02
8001F434: move    $a0, $s1                                                 ; 21 20 20 02
8001F438: li      $a1, 0xB4                                                ; B4 00 05 34
8001F43C: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001F444: jal     sub_8001C550                                             ; 54 71 00 0C
8001F448: move    $a3, $s2                                                 ; 21 38 40 02
8001F44C: move    $a0, $s0                                                 ; 21 20 00 02
8001F450: li      $a1, 0xB4                                                ; B4 00 05 34
8001F454: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001F45C: jal     sub_8001C550                                             ; 54 71 00 0C
8001F460: move    $a3, $s2                                                 ; 21 38 40 02
8001F464: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001F468: addu    $s7, $v0                                                 ; 21 B8 E2 02
8001F46C: addu    $s6, $v0                                                 ; 21 B0 C2 02
8001F470: addu    $s4, $v0                                                 ; 21 A0 82 02
8001F474: addu    $s3, $v0                                                 ; 21 98 62 02
8001F478: addiu   $s5, 1                                                   ; 01 00 B5 26
8001F47C: slti    $v0, $s5, 6                                              ; 06 00 A2 2A
8001F480: bnez    $v0, loc_8001F3A0                                        ; C7 FF 40 14
8001F484: move    $a0, $zero                                               ; 21 20 00 00
8001F488: li      $a0, 0x118                                               ; 18 01 04 34
8001F48C: li      $a1, 0xC8                                                ; C8 00 05 34
8001F490: move    $a2, $fp                                                 ; 21 30 C0 03
8001F494: jal     sub_8001C550                                             ; 54 71 00 0C
8001F498: move    $a3, $s2                                                 ; 21 38 40 02
8001F49C: li      $a0, 0x14                                                ; 14 00 04 34
8001F4A0: li      $a1, 0xC8                                                ; C8 00 05 34
8001F4A4: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001F4AC: jal     sub_8001C550                                             ; 54 71 00 0C
8001F4B0: move    $a3, $s2                                                 ; 21 38 40 02
8001F4B4: li      $a0, 0x118                                               ; 18 01 04 34
8001F4B8: li      $a1, 0xB4                                                ; B4 00 05 34
8001F4BC: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001F4C4: jal     sub_8001C550                                             ; 54 71 00 0C
8001F4C8: move    $a3, $s2                                                 ; 21 38 40 02
8001F4CC: li      $a0, 0x14                                                ; 14 00 04 34
8001F4D0: li      $a1, 0xB4                                                ; B4 00 05 34
8001F4D4: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001F4DC: jal     sub_8001C550                                             ; 54 71 00 0C
8001F4E0: move    $a3, $s2                                                 ; 21 38 40 02
8001F4E4: lw      $ra, 0x10+var_s24($sp)                                   ; 34 00 BF 8F
8001F4E8: lw      $fp, 0x10+var_s20($sp)                                   ; 30 00 BE 8F
8001F4EC: lw      $s7, 0x10+var_s1C($sp)                                   ; 2C 00 B7 8F
8001F4F0: lw      $s6, 0x10+var_s18($sp)                                   ; 28 00 B6 8F
8001F4F4: lw      $s5, 0x10+var_s14($sp)                                   ; 24 00 B5 8F
8001F4F8: lw      $s4, 0x10+var_s10($sp)                                   ; 20 00 B4 8F
8001F4FC: lw      $s3, 0x10+var_sC($sp)                                    ; 1C 00 B3 8F
8001F500: lw      $s2, 0x10+var_s8($sp)                                    ; 18 00 B2 8F
8001F504: lw      $s1, 0x10+var_s4($sp)                                    ; 14 00 B1 8F
8001F508: lw      $s0, 0x10+var_s0($sp)                                    ; 10 00 B0 8F
8001F50C: addiu   $sp, 0x38                                                ; 38 00 BD 27
8001F510: jr      $ra                                                      ; 08 00 E0 03
8001F514: nop                                                              ; 00 00 00 00
```

## `8001FEB4` `sub_8001FEB4`
Bounds: `8001FEB4-8001FFD4`

### Pseudocode
```c
BOOL __fastcall sub_8001FEB4(unsigned __int16 a1)
{
  int v2; // $fp
  int v3; // $s7
  int v4; // $s3
  int v5; // $s2
  int v6; // $s1
  BOOL result; // $v0
  int i; // [sp+10h] [-8h]

  v2 = 1310720;
  v3 = 0;
  for ( i = 0; i < 8; ++i )
  {
    v4 = 0;
    v5 = 1310720;
    v6 = 0;
    do
    {
      sub_8001C550(SHIWORD(v3), SHIWORD(v6), (int)dword_800503E0, a1);
      sub_8001C550(SHIWORD(v2), SHIWORD(v6), (int)dword_800503F0, a1);
      sub_8001C550(SHIWORD(v3), SHIWORD(v5), (int)dword_80050400, a1);
      sub_8001C550(SHIWORD(v2), SHIWORD(v5), (int)dword_80050410, a1);
      v5 += 2621440;
      ++v4;
      v6 += 2621440;
    }
    while ( v4 < 6 );
    v2 += 2621440;
    v3 += 2621440;
    result = i + 1 < 8;
  }
  return result;
}

```

### Calls in function
- 8001FF18: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001FF30: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001FF4C: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001FF64: `jal     sub_8001C550` -> 8001C550 sub_8001C550
- 8001FF7C: `bnez    $v0, loc_8001FF04` -> 8001FF04 loc_8001FF04
- 8001FF98: `bnez    $v0, loc_8001FEF0` -> 8001FEF0 loc_8001FEF0

### Xrefs to
- 80020330 -> 8001FEB4 type=17 func=sub_80020308 line=`jal     sub_8001FEB4`
- 80020390 -> 8001FEB4 type=17 func=sub_80020308 line=`jal     sub_8001FEB4`

### Xrefs from
- 8001FEB4 -> 8001FEB8 type=21 target=sub_8001FEB8 line=`addiu   $sp, -0x40`
- 8001FEB8 -> 8001FEBC type=21 target=sub_8001FEBC line=`sw      $s4, 0x18+var_s10($sp)`
- 8001FEBC -> 8001FEC0 type=21 target=sub_8001FEC0 line=`move    $s4, $a0`
- 8001FEC0 -> 8001FEC4 type=21 target=sub_8001FEC4 line=`sw      $fp, 0x18+var_s20($sp)`
- 8001FEC4 -> 8001FEC8 type=21 target=sub_8001FEC8 line=`lui     $fp, 0x14`
- 8001FEC8 -> 8001FECC type=21 target=sub_8001FECC line=`sw      $s7, 0x18+var_s1C($sp)`
- 8001FECC -> 8001FED0 type=21 target=sub_8001FED0 line=`move    $s7, $zero`
- 8001FED0 -> 8001FED4 type=21 target=sub_8001FED4 line=`sw      $ra, 0x18+var_s24($sp)`
- 8001FED4 -> 8001FED8 type=21 target=sub_8001FED8 line=`sw      $s6, 0x18+var_s18($sp)`
- 8001FED8 -> 8001FEDC type=21 target=sub_8001FEDC line=`sw      $s5, 0x18+var_s14($sp)`
- 8001FEDC -> 8001FEE0 type=21 target=sub_8001FEE0 line=`sw      $s3, 0x18+var_sC($sp)`
- 8001FEE0 -> 8001FEE4 type=21 target=sub_8001FEE4 line=`sw      $s2, 0x18+var_s8($sp)`
- 8001FEE4 -> 8001FEE8 type=21 target=sub_8001FEE8 line=`sw      $s1, 0x18+var_s4($sp)`
- 8001FEE8 -> 8001FEEC type=21 target=sub_8001FEEC line=`sw      $s0, 0x18+var_s0($sp)`
- 8001FEEC -> 8001FEF0 type=21 target=loc_8001FEF0 line=`sw      $zero, 0x18+var_8($sp)`
- 8001FEF0 -> 8001FEF4 type=21 target=sub_8001FEF4 line=`move    $s3, $zero`
- 8001FEF4 -> 8001FEF8 type=21 target=sub_8001FEF8 line=`sra     $s6, $s7, 16`
- 8001FEF8 -> 8001FEFC type=21 target=sub_8001FEFC line=`sra     $s5, $fp, 16`
- 8001FEFC -> 8001FF00 type=21 target=sub_8001FF00 line=`lui     $s2, 0x14`
- 8001FF00 -> 8001FF04 type=21 target=loc_8001FF04 line=`move    $s1, $zero`
- 8001FF04 -> 8001FF08 type=21 target=sub_8001FF08 line=`move    $a0, $s6`
- 8001FF08 -> 8001FF0C type=21 target=sub_8001FF0C line=`sra     $s0, $s1, 16`
- 8001FF0C -> 8001FF10 type=21 target=sub_8001FF10 line=`move    $a1, $s0`
- 8001FF10 -> 8001FF18 type=21 target=sub_8001FF18 line=`li      $a2, dword_800503E0`
- 8001FF18 -> 8001FF1C type=21 target=sub_8001FF1C line=`jal     sub_8001C550`
- 8001FF18 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001FF1C -> 8001FF20 type=21 target=sub_8001FF20 line=`move    $a3, $s4`
- 8001FF20 -> 8001FF24 type=21 target=sub_8001FF24 line=`move    $a0, $s5`
- 8001FF24 -> 8001FF28 type=21 target=sub_8001FF28 line=`move    $a1, $s0`
- 8001FF28 -> 8001FF30 type=21 target=sub_8001FF30 line=`li      $a2, dword_800503F0`
- 8001FF30 -> 8001FF34 type=21 target=sub_8001FF34 line=`jal     sub_8001C550`
- 8001FF30 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001FF34 -> 8001FF38 type=21 target=sub_8001FF38 line=`move    $a3, $s4`
- 8001FF38 -> 8001FF3C type=21 target=sub_8001FF3C line=`move    $a0, $s6`
- 8001FF3C -> 8001FF40 type=21 target=sub_8001FF40 line=`sra     $s0, $s2, 16`
- 8001FF40 -> 8001FF44 type=21 target=sub_8001FF44 line=`move    $a1, $s0`
- 8001FF44 -> 8001FF4C type=21 target=sub_8001FF4C line=`li      $a2, dword_80050400`
- 8001FF4C -> 8001FF50 type=21 target=sub_8001FF50 line=`jal     sub_8001C550`
- 8001FF4C -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001FF50 -> 8001FF54 type=21 target=sub_8001FF54 line=`move    $a3, $s4`
- 8001FF54 -> 8001FF58 type=21 target=sub_8001FF58 line=`move    $a0, $s5`
- 8001FF58 -> 8001FF5C type=21 target=sub_8001FF5C line=`move    $a1, $s0`
- 8001FF5C -> 8001FF64 type=21 target=sub_8001FF64 line=`li      $a2, dword_80050410`
- 8001FF64 -> 8001FF68 type=21 target=sub_8001FF68 line=`jal     sub_8001C550`
- 8001FF64 -> 8001C550 type=17 target=sub_8001C550 line=`jal     sub_8001C550`
- 8001FF68 -> 8001FF6C type=21 target=sub_8001FF6C line=`move    $a3, $s4`
- 8001FF6C -> 8001FF70 type=21 target=sub_8001FF70 line=`lui     $v1, 0x28  # '('`
- 8001FF70 -> 8001FF74 type=21 target=sub_8001FF74 line=`addu    $s2, $v1`
- 8001FF74 -> 8001FF78 type=21 target=sub_8001FF78 line=`addiu   $s3, 1`
- 8001FF78 -> 8001FF7C type=21 target=sub_8001FF7C line=`slti    $v0, $s3, 6`
- 8001FF7C -> 8001FF80 type=21 target=sub_8001FF80 line=`bnez    $v0, loc_8001FF04`
- 8001FF7C -> 8001FF04 type=19 target=loc_8001FF04 line=`bnez    $v0, loc_8001FF04`
- 8001FF80 -> 8001FF84 type=21 target=sub_8001FF84 line=`addu    $s1, $v1`
- 8001FF84 -> 8001FF88 type=21 target=sub_8001FF88 line=`addu    $fp, $v1`
- 8001FF88 -> 8001FF8C type=21 target=sub_8001FF8C line=`lw      $t0, 0x18+var_8($sp)`
- 8001FF8C -> 8001FF90 type=21 target=sub_8001FF90 line=`addu    $s7, $v1`
- 8001FF90 -> 8001FF94 type=21 target=sub_8001FF94 line=`addiu   $t0, 1`
- 8001FF94 -> 8001FF98 type=21 target=sub_8001FF98 line=`slti    $v0, $t0, 8`
- 8001FF98 -> 8001FF9C type=21 target=sub_8001FF9C line=`bnez    $v0, loc_8001FEF0`
- 8001FF98 -> 8001FEF0 type=19 target=loc_8001FEF0 line=`bnez    $v0, loc_8001FEF0`
- 8001FF9C -> 8001FFA0 type=21 target=sub_8001FFA0 line=`sw      $t0, 0x18+var_8($sp)`
- 8001FFA0 -> 8001FFA4 type=21 target=sub_8001FFA4 line=`lw      $ra, 0x18+var_s24($sp)`
- 8001FFA4 -> 8001FFA8 type=21 target=sub_8001FFA8 line=`lw      $fp, 0x18+var_s20($sp)`
- 8001FFA8 -> 8001FFAC type=21 target=sub_8001FFAC line=`lw      $s7, 0x18+var_s1C($sp)`
- 8001FFAC -> 8001FFB0 type=21 target=sub_8001FFB0 line=`lw      $s6, 0x18+var_s18($sp)`
- 8001FFB0 -> 8001FFB4 type=21 target=sub_8001FFB4 line=`lw      $s5, 0x18+var_s14($sp)`
- 8001FFB4 -> 8001FFB8 type=21 target=sub_8001FFB8 line=`lw      $s4, 0x18+var_s10($sp)`
- 8001FFB8 -> 8001FFBC type=21 target=sub_8001FFBC line=`lw      $s3, 0x18+var_sC($sp)`
- 8001FFBC -> 8001FFC0 type=21 target=sub_8001FFC0 line=`lw      $s2, 0x18+var_s8($sp)`
- 8001FFC0 -> 8001FFC4 type=21 target=sub_8001FFC4 line=`lw      $s1, 0x18+var_s4($sp)`
- 8001FFC4 -> 8001FFC8 type=21 target=sub_8001FFC8 line=`lw      $s0, 0x18+var_s0($sp)`
- 8001FFC8 -> 8001FFCC type=21 target=sub_8001FFCC line=`addiu   $sp, 0x40`
- 8001FFCC -> 8001FFD0 type=21 target=sub_8001FFD0 line=`jr      $ra`

### Full disasm
```asm
8001FEB4: addiu   $sp, -0x40                                               ; C0 FF BD 27
8001FEB8: sw      $s4, 0x18+var_s10($sp)                                   ; 28 00 B4 AF
8001FEBC: move    $s4, $a0                                                 ; 21 A0 80 00
8001FEC0: sw      $fp, 0x18+var_s20($sp)                                   ; 38 00 BE AF
8001FEC4: lui     $fp, 0x14                                                ; 14 00 1E 3C
8001FEC8: sw      $s7, 0x18+var_s1C($sp)                                   ; 34 00 B7 AF
8001FECC: move    $s7, $zero                                               ; 21 B8 00 00
8001FED0: sw      $ra, 0x18+var_s24($sp)                                   ; 3C 00 BF AF
8001FED4: sw      $s6, 0x18+var_s18($sp)                                   ; 30 00 B6 AF
8001FED8: sw      $s5, 0x18+var_s14($sp)                                   ; 2C 00 B5 AF
8001FEDC: sw      $s3, 0x18+var_sC($sp)                                    ; 24 00 B3 AF
8001FEE0: sw      $s2, 0x18+var_s8($sp)                                    ; 20 00 B2 AF
8001FEE4: sw      $s1, 0x18+var_s4($sp)                                    ; 1C 00 B1 AF
8001FEE8: sw      $s0, 0x18+var_s0($sp)                                    ; 18 00 B0 AF
8001FEEC: sw      $zero, 0x18+var_8($sp)                                   ; 10 00 A0 AF
8001FEF0: move    $s3, $zero                                               ; 21 98 00 00
8001FEF4: sra     $s6, $s7, 16                                             ; 03 B4 17 00
8001FEF8: sra     $s5, $fp, 16                                             ; 03 AC 1E 00
8001FEFC: lui     $s2, 0x14                                                ; 14 00 12 3C
8001FF00: move    $s1, $zero                                               ; 21 88 00 00
8001FF04: move    $a0, $s6                                                 ; 21 20 C0 02
8001FF08: sra     $s0, $s1, 16                                             ; 03 84 11 00
8001FF0C: move    $a1, $s0                                                 ; 21 28 00 02
8001FF10: li      $a2, dword_800503E0                                      ; 05 80 06 3C E0 03 C6 24
8001FF18: jal     sub_8001C550                                             ; 54 71 00 0C
8001FF1C: move    $a3, $s4                                                 ; 21 38 80 02
8001FF20: move    $a0, $s5                                                 ; 21 20 A0 02
8001FF24: move    $a1, $s0                                                 ; 21 28 00 02
8001FF28: li      $a2, dword_800503F0                                      ; 05 80 06 3C F0 03 C6 24
8001FF30: jal     sub_8001C550                                             ; 54 71 00 0C
8001FF34: move    $a3, $s4                                                 ; 21 38 80 02
8001FF38: move    $a0, $s6                                                 ; 21 20 C0 02
8001FF3C: sra     $s0, $s2, 16                                             ; 03 84 12 00
8001FF40: move    $a1, $s0                                                 ; 21 28 00 02
8001FF44: li      $a2, dword_80050400                                      ; 05 80 06 3C 00 04 C6 24
8001FF4C: jal     sub_8001C550                                             ; 54 71 00 0C
8001FF50: move    $a3, $s4                                                 ; 21 38 80 02
8001FF54: move    $a0, $s5                                                 ; 21 20 A0 02
8001FF58: move    $a1, $s0                                                 ; 21 28 00 02
8001FF5C: li      $a2, dword_80050410                                      ; 05 80 06 3C 10 04 C6 24
8001FF64: jal     sub_8001C550                                             ; 54 71 00 0C
8001FF68: move    $a3, $s4                                                 ; 21 38 80 02
8001FF6C: lui     $v1, 0x28  # '('                                         ; 28 00 03 3C
8001FF70: addu    $s2, $v1                                                 ; 21 90 43 02
8001FF74: addiu   $s3, 1                                                   ; 01 00 73 26
8001FF78: slti    $v0, $s3, 6                                              ; 06 00 62 2A
8001FF7C: bnez    $v0, loc_8001FF04                                        ; E1 FF 40 14
8001FF80: addu    $s1, $v1                                                 ; 21 88 23 02
8001FF84: addu    $fp, $v1                                                 ; 21 F0 C3 03
8001FF88: lw      $t0, 0x18+var_8($sp)                                     ; 10 00 A8 8F
8001FF8C: addu    $s7, $v1                                                 ; 21 B8 E3 02
8001FF90: addiu   $t0, 1                                                   ; 01 00 08 25
8001FF94: slti    $v0, $t0, 8                                              ; 08 00 02 29
8001FF98: bnez    $v0, loc_8001FEF0                                        ; D5 FF 40 14
8001FF9C: sw      $t0, 0x18+var_8($sp)                                     ; 10 00 A8 AF
8001FFA0: lw      $ra, 0x18+var_s24($sp)                                   ; 3C 00 BF 8F
8001FFA4: lw      $fp, 0x18+var_s20($sp)                                   ; 38 00 BE 8F
8001FFA8: lw      $s7, 0x18+var_s1C($sp)                                   ; 34 00 B7 8F
8001FFAC: lw      $s6, 0x18+var_s18($sp)                                   ; 30 00 B6 8F
8001FFB0: lw      $s5, 0x18+var_s14($sp)                                   ; 2C 00 B5 8F
8001FFB4: lw      $s4, 0x18+var_s10($sp)                                   ; 28 00 B4 8F
8001FFB8: lw      $s3, 0x18+var_sC($sp)                                    ; 24 00 B3 8F
8001FFBC: lw      $s2, 0x18+var_s8($sp)                                    ; 20 00 B2 8F
8001FFC0: lw      $s1, 0x18+var_s4($sp)                                    ; 1C 00 B1 8F
8001FFC4: lw      $s0, 0x18+var_s0($sp)                                    ; 18 00 B0 8F
8001FFC8: addiu   $sp, 0x40                                                ; 40 00 BD 27
8001FFCC: jr      $ra                                                      ; 08 00 E0 03
8001FFD0: nop                                                              ; 00 00 00 00
```

## `8001C864` `sub_8001C864`
Bounds: `8001C864-8001CE30`

### Pseudocode
```c
int __fastcall sub_8001C864(unsigned __int16 a1)
{
  int v1; // $gp
  int v3; // $s4
  int v4; // $s6
  int v5; // $s5
  int v6; // $gp
  int v7; // $gp
  int v8; // $gp
  int v9; // $gp
  int v10; // $s1
  int v11; // $gp
  int v12; // $gp
  int v13; // $gp
  int v14; // $s4
  int v15; // $fp
  int v16; // $s7
  int v17; // $s6
  int v18; // $s5
  __int16 v19; // $s0
  __int16 v20; // $a1
  int v21; // $gp
  int v22; // $gp
  int v23; // $gp
  int v24; // $gp
  int v25; // $gp
  int v26; // $gp
  int v27; // $gp
  int v28; // $gp
  int v29; // $gp
  int v30; // $gp
  int v31; // $gp
  int v32; // $gp

  v3 = 0;
  v4 = 1310720;
  v5 = 2621440;
  sub_8001B590(280, 200, (int)dword_8004E940, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v1 + 872));
  sub_8001B590(20, 200, (int)dword_8004E950, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v6 + 872));
  sub_8001B590(280, 180, (int)dword_8004E960, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v7 + 872));
  sub_8001B590(20, 180, (int)dword_8004E970, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v8 + 872));
  LOWORD(v10) = 40;
  do
  {
    ++v3;
    sub_8001B590(v10, 0, (int)dword_8004E900, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
    sub_8001B590(SHIWORD(v4), 0, (int)dword_8004E910, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v11 + 872));
    sub_8001B590(v10, 220, (int)dword_8004E920, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v12 + 872));
    sub_8001B590(SHIWORD(v4), 220, (int)dword_8004E930, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v13 + 872));
    v4 += 2621440;
    v5 += 2621440;
    v10 = v5 >> 16;
  }
  while ( v3 < 7 );
  v14 = 0;
  v15 = 3932160;
  v16 = 2621440;
  v17 = 1310720;
  v18 = 0;
  do
  {
    ++v14;
    sub_8001B590(0, SHIWORD(v17), (int)dword_8004E920, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
    v19 = 8 * v18;
    v20 = 8 * v18;
    v18 += 5;
    sub_8001B590(0, v20, (int)dword_8004E900, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v21 + 872));
    sub_8001B590(300, SHIWORD(v17), (int)dword_8004E930, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v22 + 872));
    sub_8001B590(300, v19, (int)dword_8004E910, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v23 + 872));
    sub_8001B590(SHIWORD(v16), 200, (int)dword_8004E860, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v24 + 872));
    sub_8001B590(SHIWORD(v15), 200, (int)dword_8004E870, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v25 + 872));
    sub_8001B590(SHIWORD(v16), 180, (int)dword_8004E880, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v26 + 872));
    sub_8001B590(SHIWORD(v15), 180, (int)dword_8004E890, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v27 + 872));
    v15 += 2621440;
    v16 += 2621440;
    v17 += 2621440;
  }
  while ( v14 < 6 );
  sub_8001B590(40, 160, (int)dword_8004E8E0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
  sub_8001B590(160, 160, (int)dword_8004E8F0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v28 + 872));
  sub_8001B590(40, 20, (int)dword_8004E8C0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v29 + 872));
  sub_8001B590(160, 20, (int)dword_8004E8D0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v30 + 872));
  sub_8001B590(20, 20, (int)dword_8004E8A0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v31 + 872));
  return sub_8001B590(280, 20, (int)dword_8004E8B0, 0, 0, a1, (int)&unk_80087288 + 20 * *(_DWORD *)(v32 + 872));
}

```

### Calls in function
- 8001C8DC: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001C914: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001C94C: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001C984: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001C9C4: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CA00: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CA38: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CA70: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CA88: `bnez    $v0, loc_8001C990` -> 8001C990 loc_8001C990
- 8001CAE4: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CB28: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CB60: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CB98: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CBD4: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CC10: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CC48: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CC80: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CC9C: `bnez    $v0, loc_8001CAB0` -> 8001CAB0 loc_8001CAB0
- 8001CCDC: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CD14: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CD4C: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CD84: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CDBC: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CDF4: `jal     sub_8001B590` -> 8001B590 sub_8001B590

### Xrefs to
- 8001DB38 -> 8001C864 type=17 func=sub_8001DB00 line=`jal     sub_8001C864`
- 8001DB70 -> 8001C864 type=17 func=sub_8001DB58 line=`jal     sub_8001C864`
- 800202CC -> 8001C864 type=17 func=sub_80020248 line=`jal     sub_8001C864`

### Xrefs from
- 8001C864 -> 8001C868 type=21 target=sub_8001C868 line=`addiu   $sp, -0x48`
- 8001C868 -> 8001C86C type=21 target=sub_8001C86C line=`sw      $s2, 0x20+var_s8($sp)`
- 8001C86C -> 8001C870 type=21 target=sub_8001C870 line=`move    $s2, $a0`
- 8001C870 -> 8001C874 type=21 target=sub_8001C874 line=`li      $a0, 0x118`
- 8001C874 -> 8001C878 type=21 target=sub_8001C878 line=`li      $a1, 0xC8`
- 8001C878 -> 8001C880 type=21 target=sub_8001C880 line=`li      $a2, dword_8004E940`
- 8001C880 -> 8001C884 type=21 target=sub_8001C884 line=`move    $a3, $zero`
- 8001C884 -> 8001C888 type=21 target=sub_8001C888 line=`sw      $s4, 0x20+var_s10($sp)`
- 8001C888 -> 8001C88C type=21 target=sub_8001C88C line=`move    $s4, $zero`
- 8001C88C -> 8001C890 type=21 target=sub_8001C890 line=`lw      $v1, 0x368($gp)`
- 8001C890 -> 8001C898 type=21 target=sub_8001C898 line=`li      $v0, unk_80087288`
- 8001C898 -> 8001C89C type=21 target=sub_8001C89C line=`sw      $s3, 0x20+var_sC($sp)`
- 8001C89C -> 8001C8A0 type=21 target=sub_8001C8A0 line=`move    $s3, $v0`
- 8001C8A0 -> 8001C8A4 type=21 target=sub_8001C8A4 line=`sw      $s6, 0x20+var_s18($sp)`
- 8001C8A4 -> 8001C8A8 type=21 target=sub_8001C8A8 line=`lui     $s6, 0x14`
- 8001C8A8 -> 8001C8AC type=21 target=sub_8001C8AC line=`sw      $s5, 0x20+var_s14($sp)`
- 8001C8AC -> 8001C8B0 type=21 target=sub_8001C8B0 line=`lui     $s5, 0x28  # '('`
- 8001C8B0 -> 8001C8B4 type=21 target=sub_8001C8B4 line=`sw      $ra, 0x20+var_s24($sp)`
- 8001C8B4 -> 8001C8B8 type=21 target=sub_8001C8B8 line=`sw      $fp, 0x20+var_s20($sp)`
- 8001C8B8 -> 8001C8BC type=21 target=sub_8001C8BC line=`sw      $s7, 0x20+var_s1C($sp)`
- 8001C8BC -> 8001C8C0 type=21 target=sub_8001C8C0 line=`sw      $s1, 0x20+var_s4($sp)`
- 8001C8C0 -> 8001C8C4 type=21 target=sub_8001C8C4 line=`sw      $s0, 0x20+var_s0($sp)`
- 8001C8C4 -> 8001C8C8 type=21 target=sub_8001C8C8 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C8C8 -> 8001C8CC type=21 target=sub_8001C8CC line=`sw      $s2, 0x20+var_C($sp)`
- 8001C8CC -> 8001C8D0 type=21 target=sub_8001C8D0 line=`sll     $v0, $v1, 2`
- 8001C8D0 -> 8001C8D4 type=21 target=sub_8001C8D4 line=`addu    $v0, $v1`
- 8001C8D4 -> 8001C8D8 type=21 target=sub_8001C8D8 line=`sll     $v0, 2`
- 8001C8D8 -> 8001C8DC type=21 target=sub_8001C8DC line=`addu    $v0, $s3`
- 8001C8DC -> 8001C8E0 type=21 target=sub_8001C8E0 line=`jal     sub_8001B590`
- 8001C8DC -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C8E0 -> 8001C8E4 type=21 target=sub_8001C8E4 line=`sw      $v0, 0x20+var_8($sp)`
- 8001C8E4 -> 8001C8E8 type=21 target=sub_8001C8E8 line=`li      $a0, 0x14`
- 8001C8E8 -> 8001C8EC type=21 target=sub_8001C8EC line=`li      $a1, 0xC8`
- 8001C8EC -> 8001C8F4 type=21 target=sub_8001C8F4 line=`li      $a2, dword_8004E950`
- 8001C8F4 -> 8001C8F8 type=21 target=sub_8001C8F8 line=`lw      $v1, 0x368($gp)`
- 8001C8F8 -> 8001C8FC type=21 target=sub_8001C8FC line=`move    $a3, $zero`
- 8001C8FC -> 8001C900 type=21 target=sub_8001C900 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C900 -> 8001C904 type=21 target=sub_8001C904 line=`sw      $s2, 0x20+var_C($sp)`
- 8001C904 -> 8001C908 type=21 target=sub_8001C908 line=`sll     $v0, $v1, 2`
- 8001C908 -> 8001C90C type=21 target=sub_8001C90C line=`addu    $v0, $v1`
- 8001C90C -> 8001C910 type=21 target=sub_8001C910 line=`sll     $v0, 2`
- 8001C910 -> 8001C914 type=21 target=sub_8001C914 line=`addu    $v0, $s3`
- 8001C914 -> 8001C918 type=21 target=sub_8001C918 line=`jal     sub_8001B590`
- 8001C914 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C918 -> 8001C91C type=21 target=sub_8001C91C line=`sw      $v0, 0x20+var_8($sp)`
- 8001C91C -> 8001C920 type=21 target=sub_8001C920 line=`li      $a0, 0x118`
- 8001C920 -> 8001C924 type=21 target=sub_8001C924 line=`li      $a1, 0xB4`
- 8001C924 -> 8001C92C type=21 target=sub_8001C92C line=`li      $a2, dword_8004E960`
- 8001C92C -> 8001C930 type=21 target=sub_8001C930 line=`lw      $v1, 0x368($gp)`
- 8001C930 -> 8001C934 type=21 target=sub_8001C934 line=`move    $a3, $zero`
- 8001C934 -> 8001C938 type=21 target=sub_8001C938 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C938 -> 8001C93C type=21 target=sub_8001C93C line=`sw      $s2, 0x20+var_C($sp)`
- 8001C93C -> 8001C940 type=21 target=sub_8001C940 line=`sll     $v0, $v1, 2`
- 8001C940 -> 8001C944 type=21 target=sub_8001C944 line=`addu    $v0, $v1`
- 8001C944 -> 8001C948 type=21 target=sub_8001C948 line=`sll     $v0, 2`
- 8001C948 -> 8001C94C type=21 target=sub_8001C94C line=`addu    $v0, $s3`
- 8001C94C -> 8001C950 type=21 target=sub_8001C950 line=`jal     sub_8001B590`
- 8001C94C -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C950 -> 8001C954 type=21 target=sub_8001C954 line=`sw      $v0, 0x20+var_8($sp)`
- 8001C954 -> 8001C958 type=21 target=sub_8001C958 line=`li      $a0, 0x14`
- 8001C958 -> 8001C95C type=21 target=sub_8001C95C line=`li      $a1, 0xB4`
- 8001C95C -> 8001C964 type=21 target=sub_8001C964 line=`li      $a2, dword_8004E970`
- 8001C964 -> 8001C968 type=21 target=sub_8001C968 line=`lw      $v1, 0x368($gp)`
- 8001C968 -> 8001C96C type=21 target=sub_8001C96C line=`move    $a3, $zero`
- 8001C96C -> 8001C970 type=21 target=sub_8001C970 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C970 -> 8001C974 type=21 target=sub_8001C974 line=`sw      $s2, 0x20+var_C($sp)`
- 8001C974 -> 8001C978 type=21 target=sub_8001C978 line=`sll     $v0, $v1, 2`
- 8001C978 -> 8001C97C type=21 target=sub_8001C97C line=`addu    $v0, $v1`
- 8001C97C -> 8001C980 type=21 target=sub_8001C980 line=`sll     $v0, 2`
- 8001C980 -> 8001C984 type=21 target=sub_8001C984 line=`addu    $v0, $s3`
- 8001C984 -> 8001C988 type=21 target=sub_8001C988 line=`jal     sub_8001B590`
- 8001C984 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C988 -> 8001C98C type=21 target=sub_8001C98C line=`sw      $v0, 0x20+var_8($sp)`
- 8001C98C -> 8001C990 type=21 target=loc_8001C990 line=`sra     $s1, $s5, 16`
- 8001C990 -> 8001C994 type=21 target=sub_8001C994 line=`move    $a0, $s1`
- 8001C994 -> 8001C998 type=21 target=sub_8001C998 line=`move    $a1, $zero`
- 8001C998 -> 8001C9A0 type=21 target=sub_8001C9A0 line=`li      $a2, dword_8004E900`
- 8001C9A0 -> 8001C9A4 type=21 target=sub_8001C9A4 line=`move    $a3, $zero`
- 8001C9A4 -> 8001C9A8 type=21 target=sub_8001C9A8 line=`lw      $v1, 0x368($gp)`
- 8001C9A8 -> 8001C9AC type=21 target=sub_8001C9AC line=`addiu   $s4, 1`
- 8001C9AC -> 8001C9B0 type=21 target=sub_8001C9B0 line=`sw      $zero, 0x20+var_10($sp)`
- 8001C9B0 -> 8001C9B4 type=21 target=sub_8001C9B4 line=`sw      $s2, 0x20+var_C($sp)`
- 8001C9B4 -> 8001C9B8 type=21 target=sub_8001C9B8 line=`sll     $v0, $v1, 2`
- 8001C9B8 -> 8001C9BC type=21 target=sub_8001C9BC line=`addu    $v0, $v1`
- 8001C9BC -> 8001C9C0 type=21 target=sub_8001C9C0 line=`sll     $v0, 2`
- 8001C9C0 -> 8001C9C4 type=21 target=sub_8001C9C4 line=`addu    $v0, $s3`
- 8001C9C4 -> 8001C9C8 type=21 target=sub_8001C9C8 line=`jal     sub_8001B590`
- 8001C9C4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001C9C8 -> 8001C9CC type=21 target=sub_8001C9CC line=`sw      $v0, 0x20+var_8($sp)`
- 8001C9CC -> 8001C9D0 type=21 target=sub_8001C9D0 line=`sra     $s0, $s6, 16`
- 8001C9D0 -> 8001C9D4 type=21 target=sub_8001C9D4 line=`move    $a0, $s0`
- 8001C9D4 -> 8001C9D8 type=21 target=sub_8001C9D8 line=`move    $a1, $zero`
- 8001C9D8 -> 8001C9E0 type=21 target=sub_8001C9E0 line=`li      $a2, dword_8004E910`
- 8001C9E0 -> 8001C9E4 type=21 target=sub_8001C9E4 line=`lw      $v1, 0x368($gp)`
- 8001C9E4 -> 8001C9E8 type=21 target=sub_8001C9E8 line=`move    $a3, $zero`
- 8001C9E8 -> 8001C9EC type=21 target=sub_8001C9EC line=`sw      $zero, 0x20+var_10($sp)`
- 8001C9EC -> 8001C9F0 type=21 target=sub_8001C9F0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001C9F0 -> 8001C9F4 type=21 target=sub_8001C9F4 line=`sll     $v0, $v1, 2`
- 8001C9F4 -> 8001C9F8 type=21 target=sub_8001C9F8 line=`addu    $v0, $v1`
- 8001C9F8 -> 8001C9FC type=21 target=sub_8001C9FC line=`sll     $v0, 2`
- 8001C9FC -> 8001CA00 type=21 target=sub_8001CA00 line=`addu    $v0, $s3`
- 8001CA00 -> 8001CA04 type=21 target=sub_8001CA04 line=`jal     sub_8001B590`
- 8001CA00 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CA04 -> 8001CA08 type=21 target=sub_8001CA08 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CA08 -> 8001CA0C type=21 target=sub_8001CA0C line=`move    $a0, $s1`
- 8001CA0C -> 8001CA10 type=21 target=sub_8001CA10 line=`li      $a1, 0xDC`
- 8001CA10 -> 8001CA18 type=21 target=sub_8001CA18 line=`li      $a2, dword_8004E920`
- 8001CA18 -> 8001CA1C type=21 target=sub_8001CA1C line=`lw      $v1, 0x368($gp)`
- 8001CA1C -> 8001CA20 type=21 target=sub_8001CA20 line=`move    $a3, $zero`
- 8001CA20 -> 8001CA24 type=21 target=sub_8001CA24 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CA24 -> 8001CA28 type=21 target=sub_8001CA28 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CA28 -> 8001CA2C type=21 target=sub_8001CA2C line=`sll     $v0, $v1, 2`
- 8001CA2C -> 8001CA30 type=21 target=sub_8001CA30 line=`addu    $v0, $v1`
- 8001CA30 -> 8001CA34 type=21 target=sub_8001CA34 line=`sll     $v0, 2`
- 8001CA34 -> 8001CA38 type=21 target=sub_8001CA38 line=`addu    $v0, $s3`
- 8001CA38 -> 8001CA3C type=21 target=sub_8001CA3C line=`jal     sub_8001B590`
- 8001CA38 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CA3C -> 8001CA40 type=21 target=sub_8001CA40 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CA40 -> 8001CA44 type=21 target=sub_8001CA44 line=`move    $a0, $s0`
- 8001CA44 -> 8001CA48 type=21 target=sub_8001CA48 line=`li      $a1, 0xDC`
- 8001CA48 -> 8001CA50 type=21 target=sub_8001CA50 line=`li      $a2, dword_8004E930`
- 8001CA50 -> 8001CA54 type=21 target=sub_8001CA54 line=`lw      $v1, 0x368($gp)`
- 8001CA54 -> 8001CA58 type=21 target=sub_8001CA58 line=`move    $a3, $zero`
- 8001CA58 -> 8001CA5C type=21 target=sub_8001CA5C line=`sw      $zero, 0x20+var_10($sp)`
- 8001CA5C -> 8001CA60 type=21 target=sub_8001CA60 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CA60 -> 8001CA64 type=21 target=sub_8001CA64 line=`sll     $v0, $v1, 2`
- 8001CA64 -> 8001CA68 type=21 target=sub_8001CA68 line=`addu    $v0, $v1`
- 8001CA68 -> 8001CA6C type=21 target=sub_8001CA6C line=`sll     $v0, 2`
- 8001CA6C -> 8001CA70 type=21 target=sub_8001CA70 line=`addu    $v0, $s3`
- 8001CA70 -> 8001CA74 type=21 target=sub_8001CA74 line=`jal     sub_8001B590`
- 8001CA70 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CA74 -> 8001CA78 type=21 target=sub_8001CA78 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CA78 -> 8001CA7C type=21 target=sub_8001CA7C line=`lui     $v0, 0x28  # '('`
- 8001CA7C -> 8001CA80 type=21 target=sub_8001CA80 line=`addu    $s6, $v0`
- 8001CA80 -> 8001CA84 type=21 target=sub_8001CA84 line=`addu    $s5, $v0`
- 8001CA84 -> 8001CA88 type=21 target=sub_8001CA88 line=`slti    $v0, $s4, 7`
- 8001CA88 -> 8001CA8C type=21 target=sub_8001CA8C line=`bnez    $v0, loc_8001C990`
- 8001CA88 -> 8001C990 type=19 target=loc_8001C990 line=`bnez    $v0, loc_8001C990`
- 8001CA8C -> 8001CA90 type=21 target=sub_8001CA90 line=`sra     $s1, $s5, 16`
- 8001CA90 -> 8001CA94 type=21 target=sub_8001CA94 line=`move    $s4, $zero`
- 8001CA94 -> 8001CA9C type=21 target=sub_8001CA9C line=`li      $s3, unk_80087288`
- 8001CA9C -> 8001CAA0 type=21 target=sub_8001CAA0 line=`lui     $fp, 0x3C  # '<'`
- 8001CAA0 -> 8001CAA4 type=21 target=sub_8001CAA4 line=`lui     $s7, 0x28  # '('`
- 8001CAA4 -> 8001CAA8 type=21 target=sub_8001CAA8 line=`lui     $s6, 0x14`
- 8001CAA8 -> 8001CAAC type=21 target=sub_8001CAAC line=`move    $s5, $zero`
- 8001CAAC -> 8001CAB0 type=21 target=loc_8001CAB0 line=`move    $a0, $zero`
- 8001CAB0 -> 8001CAB4 type=21 target=sub_8001CAB4 line=`sra     $s1, $s6, 16`
- 8001CAB4 -> 8001CAB8 type=21 target=sub_8001CAB8 line=`move    $a1, $s1`
- 8001CAB8 -> 8001CAC0 type=21 target=sub_8001CAC0 line=`li      $a2, dword_8004E920`
- 8001CAC0 -> 8001CAC4 type=21 target=sub_8001CAC4 line=`move    $a3, $zero`
- 8001CAC4 -> 8001CAC8 type=21 target=sub_8001CAC8 line=`lw      $v1, 0x368($gp)`
- 8001CAC8 -> 8001CACC type=21 target=sub_8001CACC line=`addiu   $s4, 1`
- 8001CACC -> 8001CAD0 type=21 target=sub_8001CAD0 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CAD0 -> 8001CAD4 type=21 target=sub_8001CAD4 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CAD4 -> 8001CAD8 type=21 target=sub_8001CAD8 line=`sll     $v0, $v1, 2`
- 8001CAD8 -> 8001CADC type=21 target=sub_8001CADC line=`addu    $v0, $v1`
- 8001CADC -> 8001CAE0 type=21 target=sub_8001CAE0 line=`sll     $v0, 2`
- 8001CAE0 -> 8001CAE4 type=21 target=sub_8001CAE4 line=`addu    $v0, $s3`
- 8001CAE4 -> 8001CAE8 type=21 target=sub_8001CAE8 line=`jal     sub_8001B590`
- 8001CAE4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CAE8 -> 8001CAEC type=21 target=sub_8001CAEC line=`sw      $v0, 0x20+var_8($sp)`
- 8001CAEC -> 8001CAF0 type=21 target=sub_8001CAF0 line=`move    $a0, $zero`
- 8001CAF0 -> 8001CAF4 type=21 target=sub_8001CAF4 line=`sll     $s0, $s5, 19`
- 8001CAF4 -> 8001CAF8 type=21 target=sub_8001CAF8 line=`sra     $s0, 16`
- 8001CAF8 -> 8001CAFC type=21 target=sub_8001CAFC line=`move    $a1, $s0`
- 8001CAFC -> 8001CB04 type=21 target=sub_8001CB04 line=`li      $a2, dword_8004E900`
- 8001CB04 -> 8001CB08 type=21 target=sub_8001CB08 line=`move    $a3, $zero`
- 8001CB08 -> 8001CB0C type=21 target=sub_8001CB0C line=`lw      $v1, 0x368($gp)`
- 8001CB0C -> 8001CB10 type=21 target=sub_8001CB10 line=`addiu   $s5, 5`
- 8001CB10 -> 8001CB14 type=21 target=sub_8001CB14 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CB14 -> 8001CB18 type=21 target=sub_8001CB18 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CB18 -> 8001CB1C type=21 target=sub_8001CB1C line=`sll     $v0, $v1, 2`
- 8001CB1C -> 8001CB20 type=21 target=sub_8001CB20 line=`addu    $v0, $v1`
- 8001CB20 -> 8001CB24 type=21 target=sub_8001CB24 line=`sll     $v0, 2`
- 8001CB24 -> 8001CB28 type=21 target=sub_8001CB28 line=`addu    $v0, $s3`
- 8001CB28 -> 8001CB2C type=21 target=sub_8001CB2C line=`jal     sub_8001B590`
- 8001CB28 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CB2C -> 8001CB30 type=21 target=sub_8001CB30 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CB30 -> 8001CB34 type=21 target=sub_8001CB34 line=`li      $a0, 0x12C`
- 8001CB34 -> 8001CB38 type=21 target=sub_8001CB38 line=`move    $a1, $s1`
- 8001CB38 -> 8001CB40 type=21 target=sub_8001CB40 line=`li      $a2, dword_8004E930`
- 8001CB40 -> 8001CB44 type=21 target=sub_8001CB44 line=`lw      $v1, 0x368($gp)`
- 8001CB44 -> 8001CB48 type=21 target=sub_8001CB48 line=`move    $a3, $zero`
- 8001CB48 -> 8001CB4C type=21 target=sub_8001CB4C line=`sw      $zero, 0x20+var_10($sp)`
- 8001CB4C -> 8001CB50 type=21 target=sub_8001CB50 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CB50 -> 8001CB54 type=21 target=sub_8001CB54 line=`sll     $v0, $v1, 2`
- 8001CB54 -> 8001CB58 type=21 target=sub_8001CB58 line=`addu    $v0, $v1`
- 8001CB58 -> 8001CB5C type=21 target=sub_8001CB5C line=`sll     $v0, 2`
- 8001CB5C -> 8001CB60 type=21 target=sub_8001CB60 line=`addu    $v0, $s3`
- 8001CB60 -> 8001CB64 type=21 target=sub_8001CB64 line=`jal     sub_8001B590`
- 8001CB60 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CB64 -> 8001CB68 type=21 target=sub_8001CB68 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CB68 -> 8001CB6C type=21 target=sub_8001CB6C line=`li      $a0, 0x12C`
- 8001CB6C -> 8001CB70 type=21 target=sub_8001CB70 line=`move    $a1, $s0`
- 8001CB70 -> 8001CB78 type=21 target=sub_8001CB78 line=`li      $a2, dword_8004E910`
- 8001CB78 -> 8001CB7C type=21 target=sub_8001CB7C line=`lw      $v1, 0x368($gp)`
- 8001CB7C -> 8001CB80 type=21 target=sub_8001CB80 line=`move    $a3, $zero`
- 8001CB80 -> 8001CB84 type=21 target=sub_8001CB84 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CB84 -> 8001CB88 type=21 target=sub_8001CB88 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CB88 -> 8001CB8C type=21 target=sub_8001CB8C line=`sll     $v0, $v1, 2`
- 8001CB8C -> 8001CB90 type=21 target=sub_8001CB90 line=`addu    $v0, $v1`
- 8001CB90 -> 8001CB94 type=21 target=sub_8001CB94 line=`sll     $v0, 2`
- 8001CB94 -> 8001CB98 type=21 target=sub_8001CB98 line=`addu    $v0, $s3`
- 8001CB98 -> 8001CB9C type=21 target=sub_8001CB9C line=`jal     sub_8001B590`
- 8001CB98 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CB9C -> 8001CBA0 type=21 target=sub_8001CBA0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CBA0 -> 8001CBA4 type=21 target=sub_8001CBA4 line=`sra     $s1, $s7, 16`
- 8001CBA4 -> 8001CBA8 type=21 target=sub_8001CBA8 line=`move    $a0, $s1`
- 8001CBA8 -> 8001CBAC type=21 target=sub_8001CBAC line=`li      $a1, 0xC8`
- 8001CBAC -> 8001CBB4 type=21 target=sub_8001CBB4 line=`li      $a2, dword_8004E860`
- 8001CBB4 -> 8001CBB8 type=21 target=sub_8001CBB8 line=`lw      $v1, 0x368($gp)`
- 8001CBB8 -> 8001CBBC type=21 target=sub_8001CBBC line=`move    $a3, $zero`
- 8001CBBC -> 8001CBC0 type=21 target=sub_8001CBC0 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CBC0 -> 8001CBC4 type=21 target=sub_8001CBC4 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CBC4 -> 8001CBC8 type=21 target=sub_8001CBC8 line=`sll     $v0, $v1, 2`
- 8001CBC8 -> 8001CBCC type=21 target=sub_8001CBCC line=`addu    $v0, $v1`
- 8001CBCC -> 8001CBD0 type=21 target=sub_8001CBD0 line=`sll     $v0, 2`
- 8001CBD0 -> 8001CBD4 type=21 target=sub_8001CBD4 line=`addu    $v0, $s3`
- 8001CBD4 -> 8001CBD8 type=21 target=sub_8001CBD8 line=`jal     sub_8001B590`
- 8001CBD4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CBD8 -> 8001CBDC type=21 target=sub_8001CBDC line=`sw      $v0, 0x20+var_8($sp)`
- 8001CBDC -> 8001CBE0 type=21 target=sub_8001CBE0 line=`sra     $s0, $fp, 16`
- 8001CBE0 -> 8001CBE4 type=21 target=sub_8001CBE4 line=`move    $a0, $s0`
- 8001CBE4 -> 8001CBE8 type=21 target=sub_8001CBE8 line=`li      $a1, 0xC8`
- 8001CBE8 -> 8001CBF0 type=21 target=sub_8001CBF0 line=`li      $a2, dword_8004E870`
- 8001CBF0 -> 8001CBF4 type=21 target=sub_8001CBF4 line=`lw      $v1, 0x368($gp)`
- 8001CBF4 -> 8001CBF8 type=21 target=sub_8001CBF8 line=`move    $a3, $zero`
- 8001CBF8 -> 8001CBFC type=21 target=sub_8001CBFC line=`sw      $zero, 0x20+var_10($sp)`
- 8001CBFC -> 8001CC00 type=21 target=sub_8001CC00 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CC00 -> 8001CC04 type=21 target=sub_8001CC04 line=`sll     $v0, $v1, 2`
- 8001CC04 -> 8001CC08 type=21 target=sub_8001CC08 line=`addu    $v0, $v1`
- 8001CC08 -> 8001CC0C type=21 target=sub_8001CC0C line=`sll     $v0, 2`
- 8001CC0C -> 8001CC10 type=21 target=sub_8001CC10 line=`addu    $v0, $s3`
- 8001CC10 -> 8001CC14 type=21 target=sub_8001CC14 line=`jal     sub_8001B590`
- 8001CC10 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CC14 -> 8001CC18 type=21 target=sub_8001CC18 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CC18 -> 8001CC1C type=21 target=sub_8001CC1C line=`move    $a0, $s1`
- 8001CC1C -> 8001CC20 type=21 target=sub_8001CC20 line=`li      $a1, 0xB4`
- 8001CC20 -> 8001CC28 type=21 target=sub_8001CC28 line=`li      $a2, dword_8004E880`
- 8001CC28 -> 8001CC2C type=21 target=sub_8001CC2C line=`lw      $v1, 0x368($gp)`
- 8001CC2C -> 8001CC30 type=21 target=sub_8001CC30 line=`move    $a3, $zero`
- 8001CC30 -> 8001CC34 type=21 target=sub_8001CC34 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CC34 -> 8001CC38 type=21 target=sub_8001CC38 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CC38 -> 8001CC3C type=21 target=sub_8001CC3C line=`sll     $v0, $v1, 2`
- 8001CC3C -> 8001CC40 type=21 target=sub_8001CC40 line=`addu    $v0, $v1`
- 8001CC40 -> 8001CC44 type=21 target=sub_8001CC44 line=`sll     $v0, 2`
- 8001CC44 -> 8001CC48 type=21 target=sub_8001CC48 line=`addu    $v0, $s3`
- 8001CC48 -> 8001CC4C type=21 target=sub_8001CC4C line=`jal     sub_8001B590`
- 8001CC48 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CC4C -> 8001CC50 type=21 target=sub_8001CC50 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CC50 -> 8001CC54 type=21 target=sub_8001CC54 line=`move    $a0, $s0`
- 8001CC54 -> 8001CC58 type=21 target=sub_8001CC58 line=`li      $a1, 0xB4`
- 8001CC58 -> 8001CC60 type=21 target=sub_8001CC60 line=`li      $a2, dword_8004E890`
- 8001CC60 -> 8001CC64 type=21 target=sub_8001CC64 line=`lw      $v1, 0x368($gp)`
- 8001CC64 -> 8001CC68 type=21 target=sub_8001CC68 line=`move    $a3, $zero`
- 8001CC68 -> 8001CC6C type=21 target=sub_8001CC6C line=`sw      $zero, 0x20+var_10($sp)`
- 8001CC6C -> 8001CC70 type=21 target=sub_8001CC70 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CC70 -> 8001CC74 type=21 target=sub_8001CC74 line=`sll     $v0, $v1, 2`
- 8001CC74 -> 8001CC78 type=21 target=sub_8001CC78 line=`addu    $v0, $v1`
- 8001CC78 -> 8001CC7C type=21 target=sub_8001CC7C line=`sll     $v0, 2`
- 8001CC7C -> 8001CC80 type=21 target=sub_8001CC80 line=`addu    $v0, $s3`
- 8001CC80 -> 8001CC84 type=21 target=sub_8001CC84 line=`jal     sub_8001B590`
- 8001CC80 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CC84 -> 8001CC88 type=21 target=sub_8001CC88 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CC88 -> 8001CC8C type=21 target=sub_8001CC8C line=`lui     $v0, 0x28  # '('`
- 8001CC8C -> 8001CC90 type=21 target=sub_8001CC90 line=`addu    $fp, $v0`
- 8001CC90 -> 8001CC94 type=21 target=sub_8001CC94 line=`addu    $s7, $v0`
- 8001CC94 -> 8001CC98 type=21 target=sub_8001CC98 line=`addu    $s6, $v0`
- 8001CC98 -> 8001CC9C type=21 target=sub_8001CC9C line=`slti    $v0, $s4, 6`
- 8001CC9C -> 8001CCA0 type=21 target=sub_8001CCA0 line=`bnez    $v0, loc_8001CAB0`
- 8001CC9C -> 8001CAB0 type=19 target=loc_8001CAB0 line=`bnez    $v0, loc_8001CAB0`
- 8001CCA0 -> 8001CCA4 type=21 target=sub_8001CCA4 line=`move    $a0, $zero`
- 8001CCA4 -> 8001CCA8 type=21 target=sub_8001CCA8 line=`li      $a0, 0x28  # '('`
- 8001CCA8 -> 8001CCAC type=21 target=sub_8001CCAC line=`li      $a1, 0xA0`
- 8001CCAC -> 8001CCB4 type=21 target=sub_8001CCB4 line=`li      $a2, dword_8004E8E0`
- 8001CCB4 -> 8001CCB8 type=21 target=sub_8001CCB8 line=`move    $a3, $zero`
- 8001CCB8 -> 8001CCBC type=21 target=sub_8001CCBC line=`lw      $v1, 0x368($gp)`
- 8001CCBC -> 8001CCC4 type=21 target=sub_8001CCC4 line=`li      $s0, unk_80087288`
- 8001CCC4 -> 8001CCC8 type=21 target=sub_8001CCC8 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CCC8 -> 8001CCCC type=21 target=sub_8001CCCC line=`sw      $s2, 0x20+var_C($sp)`
- 8001CCCC -> 8001CCD0 type=21 target=sub_8001CCD0 line=`sll     $v0, $v1, 2`
- 8001CCD0 -> 8001CCD4 type=21 target=sub_8001CCD4 line=`addu    $v0, $v1`
- 8001CCD4 -> 8001CCD8 type=21 target=sub_8001CCD8 line=`sll     $v0, 2`
- 8001CCD8 -> 8001CCDC type=21 target=sub_8001CCDC line=`addu    $v0, $s0`
- 8001CCDC -> 8001CCE0 type=21 target=sub_8001CCE0 line=`jal     sub_8001B590`
- 8001CCDC -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CCE0 -> 8001CCE4 type=21 target=sub_8001CCE4 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CCE4 -> 8001CCE8 type=21 target=sub_8001CCE8 line=`li      $a0, 0xA0`
- 8001CCE8 -> 8001CCEC type=21 target=sub_8001CCEC line=`li      $a1, 0xA0`
- 8001CCEC -> 8001CCF4 type=21 target=sub_8001CCF4 line=`li      $a2, dword_8004E8F0`
- 8001CCF4 -> 8001CCF8 type=21 target=sub_8001CCF8 line=`lw      $v1, 0x368($gp)`
- 8001CCF8 -> 8001CCFC type=21 target=sub_8001CCFC line=`move    $a3, $zero`
- 8001CCFC -> 8001CD00 type=21 target=sub_8001CD00 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CD00 -> 8001CD04 type=21 target=sub_8001CD04 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CD04 -> 8001CD08 type=21 target=sub_8001CD08 line=`sll     $v0, $v1, 2`
- 8001CD08 -> 8001CD0C type=21 target=sub_8001CD0C line=`addu    $v0, $v1`
- 8001CD0C -> 8001CD10 type=21 target=sub_8001CD10 line=`sll     $v0, 2`
- 8001CD10 -> 8001CD14 type=21 target=sub_8001CD14 line=`addu    $v0, $s0`
- 8001CD14 -> 8001CD18 type=21 target=sub_8001CD18 line=`jal     sub_8001B590`
- 8001CD14 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CD18 -> 8001CD1C type=21 target=sub_8001CD1C line=`sw      $v0, 0x20+var_8($sp)`
- 8001CD1C -> 8001CD20 type=21 target=sub_8001CD20 line=`li      $a0, 0x28  # '('`
- 8001CD20 -> 8001CD24 type=21 target=sub_8001CD24 line=`li      $a1, 0x14`
- 8001CD24 -> 8001CD2C type=21 target=sub_8001CD2C line=`li      $a2, dword_8004E8C0`
- 8001CD2C -> 8001CD30 type=21 target=sub_8001CD30 line=`lw      $v1, 0x368($gp)`
- 8001CD30 -> 8001CD34 type=21 target=sub_8001CD34 line=`move    $a3, $zero`
- 8001CD34 -> 8001CD38 type=21 target=sub_8001CD38 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CD38 -> 8001CD3C type=21 target=sub_8001CD3C line=`sw      $s2, 0x20+var_C($sp)`
- 8001CD3C -> 8001CD40 type=21 target=sub_8001CD40 line=`sll     $v0, $v1, 2`
- 8001CD40 -> 8001CD44 type=21 target=sub_8001CD44 line=`addu    $v0, $v1`
- 8001CD44 -> 8001CD48 type=21 target=sub_8001CD48 line=`sll     $v0, 2`
- 8001CD48 -> 8001CD4C type=21 target=sub_8001CD4C line=`addu    $v0, $s0`
- 8001CD4C -> 8001CD50 type=21 target=sub_8001CD50 line=`jal     sub_8001B590`
- 8001CD4C -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CD50 -> 8001CD54 type=21 target=sub_8001CD54 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CD54 -> 8001CD58 type=21 target=sub_8001CD58 line=`li      $a0, 0xA0`
- 8001CD58 -> 8001CD5C type=21 target=sub_8001CD5C line=`li      $a1, 0x14`
- 8001CD5C -> 8001CD64 type=21 target=sub_8001CD64 line=`li      $a2, dword_8004E8D0`
- 8001CD64 -> 8001CD68 type=21 target=sub_8001CD68 line=`lw      $v1, 0x368($gp)`
- 8001CD68 -> 8001CD6C type=21 target=sub_8001CD6C line=`move    $a3, $zero`
- 8001CD6C -> 8001CD70 type=21 target=sub_8001CD70 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CD70 -> 8001CD74 type=21 target=sub_8001CD74 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CD74 -> 8001CD78 type=21 target=sub_8001CD78 line=`sll     $v0, $v1, 2`
- 8001CD78 -> 8001CD7C type=21 target=sub_8001CD7C line=`addu    $v0, $v1`
- 8001CD7C -> 8001CD80 type=21 target=sub_8001CD80 line=`sll     $v0, 2`
- 8001CD80 -> 8001CD84 type=21 target=sub_8001CD84 line=`addu    $v0, $s0`
- 8001CD84 -> 8001CD88 type=21 target=sub_8001CD88 line=`jal     sub_8001B590`
- 8001CD84 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CD88 -> 8001CD8C type=21 target=sub_8001CD8C line=`sw      $v0, 0x20+var_8($sp)`
- 8001CD8C -> 8001CD90 type=21 target=sub_8001CD90 line=`li      $a0, 0x14`
- 8001CD90 -> 8001CD94 type=21 target=sub_8001CD94 line=`li      $a1, 0x14`
- 8001CD94 -> 8001CD9C type=21 target=sub_8001CD9C line=`li      $a2, dword_8004E8A0`
- 8001CD9C -> 8001CDA0 type=21 target=sub_8001CDA0 line=`lw      $v1, 0x368($gp)`
- 8001CDA0 -> 8001CDA4 type=21 target=sub_8001CDA4 line=`move    $a3, $zero`
- 8001CDA4 -> 8001CDA8 type=21 target=sub_8001CDA8 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CDA8 -> 8001CDAC type=21 target=sub_8001CDAC line=`sw      $s2, 0x20+var_C($sp)`
- 8001CDAC -> 8001CDB0 type=21 target=sub_8001CDB0 line=`sll     $v0, $v1, 2`
- 8001CDB0 -> 8001CDB4 type=21 target=sub_8001CDB4 line=`addu    $v0, $v1`
- 8001CDB4 -> 8001CDB8 type=21 target=sub_8001CDB8 line=`sll     $v0, 2`
- 8001CDB8 -> 8001CDBC type=21 target=sub_8001CDBC line=`addu    $v0, $s0`
- 8001CDBC -> 8001CDC0 type=21 target=sub_8001CDC0 line=`jal     sub_8001B590`
- 8001CDBC -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CDC0 -> 8001CDC4 type=21 target=sub_8001CDC4 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CDC4 -> 8001CDC8 type=21 target=sub_8001CDC8 line=`li      $a0, 0x118`
- 8001CDC8 -> 8001CDCC type=21 target=sub_8001CDCC line=`li      $a1, 0x14`
- 8001CDCC -> 8001CDD4 type=21 target=sub_8001CDD4 line=`li      $a2, dword_8004E8B0`
- 8001CDD4 -> 8001CDD8 type=21 target=sub_8001CDD8 line=`lw      $v1, 0x368($gp)`
- 8001CDD8 -> 8001CDDC type=21 target=sub_8001CDDC line=`move    $a3, $zero`
- 8001CDDC -> 8001CDE0 type=21 target=sub_8001CDE0 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CDE0 -> 8001CDE4 type=21 target=sub_8001CDE4 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CDE4 -> 8001CDE8 type=21 target=sub_8001CDE8 line=`sll     $v0, $v1, 2`
- 8001CDE8 -> 8001CDEC type=21 target=sub_8001CDEC line=`addu    $v0, $v1`
- 8001CDEC -> 8001CDF0 type=21 target=sub_8001CDF0 line=`sll     $v0, 2`
- 8001CDF0 -> 8001CDF4 type=21 target=sub_8001CDF4 line=`addu    $v0, $s0`
- 8001CDF4 -> 8001CDF8 type=21 target=sub_8001CDF8 line=`jal     sub_8001B590`
- 8001CDF4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CDF8 -> 8001CDFC type=21 target=sub_8001CDFC line=`sw      $v0, 0x20+var_8($sp)`
- 8001CDFC -> 8001CE00 type=21 target=sub_8001CE00 line=`lw      $ra, 0x20+var_s24($sp)`
- 8001CE00 -> 8001CE04 type=21 target=sub_8001CE04 line=`lw      $fp, 0x20+var_s20($sp)`
- 8001CE04 -> 8001CE08 type=21 target=sub_8001CE08 line=`lw      $s7, 0x20+var_s1C($sp)`
- 8001CE08 -> 8001CE0C type=21 target=sub_8001CE0C line=`lw      $s6, 0x20+var_s18($sp)`
- 8001CE0C -> 8001CE10 type=21 target=sub_8001CE10 line=`lw      $s5, 0x20+var_s14($sp)`
- 8001CE10 -> 8001CE14 type=21 target=sub_8001CE14 line=`lw      $s4, 0x20+var_s10($sp)`
- 8001CE14 -> 8001CE18 type=21 target=sub_8001CE18 line=`lw      $s3, 0x20+var_sC($sp)`
- 8001CE18 -> 8001CE1C type=21 target=sub_8001CE1C line=`lw      $s2, 0x20+var_s8($sp)`
- 8001CE1C -> 8001CE20 type=21 target=sub_8001CE20 line=`lw      $s1, 0x20+var_s4($sp)`
- 8001CE20 -> 8001CE24 type=21 target=sub_8001CE24 line=`lw      $s0, 0x20+var_s0($sp)`
- 8001CE24 -> 8001CE28 type=21 target=sub_8001CE28 line=`addiu   $sp, 0x48`
- 8001CE28 -> 8001CE2C type=21 target=sub_8001CE2C line=`jr      $ra`

### Full disasm
```asm
8001C864: addiu   $sp, -0x48                                               ; B8 FF BD 27
8001C868: sw      $s2, 0x20+var_s8($sp)                                    ; 28 00 B2 AF
8001C86C: move    $s2, $a0                                                 ; 21 90 80 00
8001C870: li      $a0, 0x118                                               ; 18 01 04 34
8001C874: li      $a1, 0xC8                                                ; C8 00 05 34
8001C878: li      $a2, dword_8004E940                                      ; 05 80 06 3C 40 E9 C6 24
8001C880: move    $a3, $zero                                               ; 21 38 00 00
8001C884: sw      $s4, 0x20+var_s10($sp)                                   ; 30 00 B4 AF
8001C888: move    $s4, $zero                                               ; 21 A0 00 00
8001C88C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C890: li      $v0, unk_80087288                                        ; 08 80 02 3C 88 72 42 24
8001C898: sw      $s3, 0x20+var_sC($sp)                                    ; 2C 00 B3 AF
8001C89C: move    $s3, $v0                                                 ; 21 98 40 00
8001C8A0: sw      $s6, 0x20+var_s18($sp)                                   ; 38 00 B6 AF
8001C8A4: lui     $s6, 0x14                                                ; 14 00 16 3C
8001C8A8: sw      $s5, 0x20+var_s14($sp)                                   ; 34 00 B5 AF
8001C8AC: lui     $s5, 0x28  # '('                                         ; 28 00 15 3C
8001C8B0: sw      $ra, 0x20+var_s24($sp)                                   ; 44 00 BF AF
8001C8B4: sw      $fp, 0x20+var_s20($sp)                                   ; 40 00 BE AF
8001C8B8: sw      $s7, 0x20+var_s1C($sp)                                   ; 3C 00 B7 AF
8001C8BC: sw      $s1, 0x20+var_s4($sp)                                    ; 24 00 B1 AF
8001C8C0: sw      $s0, 0x20+var_s0($sp)                                    ; 20 00 B0 AF
8001C8C4: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C8C8: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C8CC: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C8D0: addu    $v0, $v1                                                 ; 21 10 43 00
8001C8D4: sll     $v0, 2                                                   ; 80 10 02 00
8001C8D8: addu    $v0, $s3                                                 ; 21 10 53 00
8001C8DC: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C8E0: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C8E4: li      $a0, 0x14                                                ; 14 00 04 34
8001C8E8: li      $a1, 0xC8                                                ; C8 00 05 34
8001C8EC: li      $a2, dword_8004E950                                      ; 05 80 06 3C 50 E9 C6 24
8001C8F4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C8F8: move    $a3, $zero                                               ; 21 38 00 00
8001C8FC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C900: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C904: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C908: addu    $v0, $v1                                                 ; 21 10 43 00
8001C90C: sll     $v0, 2                                                   ; 80 10 02 00
8001C910: addu    $v0, $s3                                                 ; 21 10 53 00
8001C914: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C918: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C91C: li      $a0, 0x118                                               ; 18 01 04 34
8001C920: li      $a1, 0xB4                                                ; B4 00 05 34
8001C924: li      $a2, dword_8004E960                                      ; 05 80 06 3C 60 E9 C6 24
8001C92C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C930: move    $a3, $zero                                               ; 21 38 00 00
8001C934: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C938: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C93C: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C940: addu    $v0, $v1                                                 ; 21 10 43 00
8001C944: sll     $v0, 2                                                   ; 80 10 02 00
8001C948: addu    $v0, $s3                                                 ; 21 10 53 00
8001C94C: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C950: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C954: li      $a0, 0x14                                                ; 14 00 04 34
8001C958: li      $a1, 0xB4                                                ; B4 00 05 34
8001C95C: li      $a2, dword_8004E970                                      ; 05 80 06 3C 70 E9 C6 24
8001C964: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C968: move    $a3, $zero                                               ; 21 38 00 00
8001C96C: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C970: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C974: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C978: addu    $v0, $v1                                                 ; 21 10 43 00
8001C97C: sll     $v0, 2                                                   ; 80 10 02 00
8001C980: addu    $v0, $s3                                                 ; 21 10 53 00
8001C984: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C988: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C98C: sra     $s1, $s5, 16                                             ; 03 8C 15 00
8001C990: move    $a0, $s1                                                 ; 21 20 20 02
8001C994: move    $a1, $zero                                               ; 21 28 00 00
8001C998: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001C9A0: move    $a3, $zero                                               ; 21 38 00 00
8001C9A4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C9A8: addiu   $s4, 1                                                   ; 01 00 94 26
8001C9AC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C9B0: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C9B4: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C9B8: addu    $v0, $v1                                                 ; 21 10 43 00
8001C9BC: sll     $v0, 2                                                   ; 80 10 02 00
8001C9C0: addu    $v0, $s3                                                 ; 21 10 53 00
8001C9C4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001C9C8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001C9CC: sra     $s0, $s6, 16                                             ; 03 84 16 00
8001C9D0: move    $a0, $s0                                                 ; 21 20 00 02
8001C9D4: move    $a1, $zero                                               ; 21 28 00 00
8001C9D8: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001C9E0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001C9E4: move    $a3, $zero                                               ; 21 38 00 00
8001C9E8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001C9EC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001C9F0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001C9F4: addu    $v0, $v1                                                 ; 21 10 43 00
8001C9F8: sll     $v0, 2                                                   ; 80 10 02 00
8001C9FC: addu    $v0, $s3                                                 ; 21 10 53 00
8001CA00: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CA04: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CA08: move    $a0, $s1                                                 ; 21 20 20 02
8001CA0C: li      $a1, 0xDC                                                ; DC 00 05 34
8001CA10: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001CA18: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CA1C: move    $a3, $zero                                               ; 21 38 00 00
8001CA20: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CA24: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CA28: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CA2C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CA30: sll     $v0, 2                                                   ; 80 10 02 00
8001CA34: addu    $v0, $s3                                                 ; 21 10 53 00
8001CA38: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CA3C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CA40: move    $a0, $s0                                                 ; 21 20 00 02
8001CA44: li      $a1, 0xDC                                                ; DC 00 05 34
8001CA48: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001CA50: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CA54: move    $a3, $zero                                               ; 21 38 00 00
8001CA58: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CA5C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CA60: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CA64: addu    $v0, $v1                                                 ; 21 10 43 00
8001CA68: sll     $v0, 2                                                   ; 80 10 02 00
8001CA6C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CA70: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CA74: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CA78: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001CA7C: addu    $s6, $v0                                                 ; 21 B0 C2 02
8001CA80: addu    $s5, $v0                                                 ; 21 A8 A2 02
8001CA84: slti    $v0, $s4, 7                                              ; 07 00 82 2A
8001CA88: bnez    $v0, loc_8001C990                                        ; C1 FF 40 14
8001CA8C: sra     $s1, $s5, 16                                             ; 03 8C 15 00
8001CA90: move    $s4, $zero                                               ; 21 A0 00 00
8001CA94: li      $s3, unk_80087288                                        ; 08 80 13 3C 88 72 73 26
8001CA9C: lui     $fp, 0x3C  # '<'                                         ; 3C 00 1E 3C
8001CAA0: lui     $s7, 0x28  # '('                                         ; 28 00 17 3C
8001CAA4: lui     $s6, 0x14                                                ; 14 00 16 3C
8001CAA8: move    $s5, $zero                                               ; 21 A8 00 00
8001CAAC: move    $a0, $zero                                               ; 21 20 00 00
8001CAB0: sra     $s1, $s6, 16                                             ; 03 8C 16 00
8001CAB4: move    $a1, $s1                                                 ; 21 28 20 02
8001CAB8: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001CAC0: move    $a3, $zero                                               ; 21 38 00 00
8001CAC4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CAC8: addiu   $s4, 1                                                   ; 01 00 94 26
8001CACC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CAD0: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CAD4: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CAD8: addu    $v0, $v1                                                 ; 21 10 43 00
8001CADC: sll     $v0, 2                                                   ; 80 10 02 00
8001CAE0: addu    $v0, $s3                                                 ; 21 10 53 00
8001CAE4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CAE8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CAEC: move    $a0, $zero                                               ; 21 20 00 00
8001CAF0: sll     $s0, $s5, 19                                             ; C0 84 15 00
8001CAF4: sra     $s0, 16                                                  ; 03 84 10 00
8001CAF8: move    $a1, $s0                                                 ; 21 28 00 02
8001CAFC: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001CB04: move    $a3, $zero                                               ; 21 38 00 00
8001CB08: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CB0C: addiu   $s5, 5                                                   ; 05 00 B5 26
8001CB10: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CB14: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CB18: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CB1C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CB20: sll     $v0, 2                                                   ; 80 10 02 00
8001CB24: addu    $v0, $s3                                                 ; 21 10 53 00
8001CB28: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CB2C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CB30: li      $a0, 0x12C                                               ; 2C 01 04 34
8001CB34: move    $a1, $s1                                                 ; 21 28 20 02
8001CB38: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001CB40: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CB44: move    $a3, $zero                                               ; 21 38 00 00
8001CB48: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CB4C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CB50: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CB54: addu    $v0, $v1                                                 ; 21 10 43 00
8001CB58: sll     $v0, 2                                                   ; 80 10 02 00
8001CB5C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CB60: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CB64: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CB68: li      $a0, 0x12C                                               ; 2C 01 04 34
8001CB6C: move    $a1, $s0                                                 ; 21 28 00 02
8001CB70: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001CB78: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CB7C: move    $a3, $zero                                               ; 21 38 00 00
8001CB80: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CB84: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CB88: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CB8C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CB90: sll     $v0, 2                                                   ; 80 10 02 00
8001CB94: addu    $v0, $s3                                                 ; 21 10 53 00
8001CB98: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CB9C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CBA0: sra     $s1, $s7, 16                                             ; 03 8C 17 00
8001CBA4: move    $a0, $s1                                                 ; 21 20 20 02
8001CBA8: li      $a1, 0xC8                                                ; C8 00 05 34
8001CBAC: li      $a2, dword_8004E860                                      ; 05 80 06 3C 60 E8 C6 24
8001CBB4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CBB8: move    $a3, $zero                                               ; 21 38 00 00
8001CBBC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CBC0: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CBC4: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CBC8: addu    $v0, $v1                                                 ; 21 10 43 00
8001CBCC: sll     $v0, 2                                                   ; 80 10 02 00
8001CBD0: addu    $v0, $s3                                                 ; 21 10 53 00
8001CBD4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CBD8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CBDC: sra     $s0, $fp, 16                                             ; 03 84 1E 00
8001CBE0: move    $a0, $s0                                                 ; 21 20 00 02
8001CBE4: li      $a1, 0xC8                                                ; C8 00 05 34
8001CBE8: li      $a2, dword_8004E870                                      ; 05 80 06 3C 70 E8 C6 24
8001CBF0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CBF4: move    $a3, $zero                                               ; 21 38 00 00
8001CBF8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CBFC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CC00: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CC04: addu    $v0, $v1                                                 ; 21 10 43 00
8001CC08: sll     $v0, 2                                                   ; 80 10 02 00
8001CC0C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CC10: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CC14: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CC18: move    $a0, $s1                                                 ; 21 20 20 02
8001CC1C: li      $a1, 0xB4                                                ; B4 00 05 34
8001CC20: li      $a2, dword_8004E880                                      ; 05 80 06 3C 80 E8 C6 24
8001CC28: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CC2C: move    $a3, $zero                                               ; 21 38 00 00
8001CC30: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CC34: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CC38: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CC3C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CC40: sll     $v0, 2                                                   ; 80 10 02 00
8001CC44: addu    $v0, $s3                                                 ; 21 10 53 00
8001CC48: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CC4C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CC50: move    $a0, $s0                                                 ; 21 20 00 02
8001CC54: li      $a1, 0xB4                                                ; B4 00 05 34
8001CC58: li      $a2, dword_8004E890                                      ; 05 80 06 3C 90 E8 C6 24
8001CC60: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CC64: move    $a3, $zero                                               ; 21 38 00 00
8001CC68: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CC6C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CC70: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CC74: addu    $v0, $v1                                                 ; 21 10 43 00
8001CC78: sll     $v0, 2                                                   ; 80 10 02 00
8001CC7C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CC80: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CC84: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CC88: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001CC8C: addu    $fp, $v0                                                 ; 21 F0 C2 03
8001CC90: addu    $s7, $v0                                                 ; 21 B8 E2 02
8001CC94: addu    $s6, $v0                                                 ; 21 B0 C2 02
8001CC98: slti    $v0, $s4, 6                                              ; 06 00 82 2A
8001CC9C: bnez    $v0, loc_8001CAB0                                        ; 84 FF 40 14
8001CCA0: move    $a0, $zero                                               ; 21 20 00 00
8001CCA4: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001CCA8: li      $a1, 0xA0                                                ; A0 00 05 34
8001CCAC: li      $a2, dword_8004E8E0                                      ; 05 80 06 3C E0 E8 C6 24
8001CCB4: move    $a3, $zero                                               ; 21 38 00 00
8001CCB8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CCBC: li      $s0, unk_80087288                                        ; 08 80 10 3C 88 72 10 26
8001CCC4: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CCC8: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CCCC: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CCD0: addu    $v0, $v1                                                 ; 21 10 43 00
8001CCD4: sll     $v0, 2                                                   ; 80 10 02 00
8001CCD8: addu    $v0, $s0                                                 ; 21 10 50 00
8001CCDC: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CCE0: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CCE4: li      $a0, 0xA0                                                ; A0 00 04 34
8001CCE8: li      $a1, 0xA0                                                ; A0 00 05 34
8001CCEC: li      $a2, dword_8004E8F0                                      ; 05 80 06 3C F0 E8 C6 24
8001CCF4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CCF8: move    $a3, $zero                                               ; 21 38 00 00
8001CCFC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CD00: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CD04: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CD08: addu    $v0, $v1                                                 ; 21 10 43 00
8001CD0C: sll     $v0, 2                                                   ; 80 10 02 00
8001CD10: addu    $v0, $s0                                                 ; 21 10 50 00
8001CD14: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CD18: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CD1C: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001CD20: li      $a1, 0x14                                                ; 14 00 05 34
8001CD24: li      $a2, dword_8004E8C0                                      ; 05 80 06 3C C0 E8 C6 24
8001CD2C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CD30: move    $a3, $zero                                               ; 21 38 00 00
8001CD34: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CD38: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CD3C: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CD40: addu    $v0, $v1                                                 ; 21 10 43 00
8001CD44: sll     $v0, 2                                                   ; 80 10 02 00
8001CD48: addu    $v0, $s0                                                 ; 21 10 50 00
8001CD4C: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CD50: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CD54: li      $a0, 0xA0                                                ; A0 00 04 34
8001CD58: li      $a1, 0x14                                                ; 14 00 05 34
8001CD5C: li      $a2, dword_8004E8D0                                      ; 05 80 06 3C D0 E8 C6 24
8001CD64: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CD68: move    $a3, $zero                                               ; 21 38 00 00
8001CD6C: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CD70: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CD74: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CD78: addu    $v0, $v1                                                 ; 21 10 43 00
8001CD7C: sll     $v0, 2                                                   ; 80 10 02 00
8001CD80: addu    $v0, $s0                                                 ; 21 10 50 00
8001CD84: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CD88: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CD8C: li      $a0, 0x14                                                ; 14 00 04 34
8001CD90: li      $a1, 0x14                                                ; 14 00 05 34
8001CD94: li      $a2, dword_8004E8A0                                      ; 05 80 06 3C A0 E8 C6 24
8001CD9C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CDA0: move    $a3, $zero                                               ; 21 38 00 00
8001CDA4: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CDA8: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CDAC: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CDB0: addu    $v0, $v1                                                 ; 21 10 43 00
8001CDB4: sll     $v0, 2                                                   ; 80 10 02 00
8001CDB8: addu    $v0, $s0                                                 ; 21 10 50 00
8001CDBC: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CDC0: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CDC4: li      $a0, 0x118                                               ; 18 01 04 34
8001CDC8: li      $a1, 0x14                                                ; 14 00 05 34
8001CDCC: li      $a2, dword_8004E8B0                                      ; 05 80 06 3C B0 E8 C6 24
8001CDD4: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CDD8: move    $a3, $zero                                               ; 21 38 00 00
8001CDDC: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CDE0: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CDE4: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CDE8: addu    $v0, $v1                                                 ; 21 10 43 00
8001CDEC: sll     $v0, 2                                                   ; 80 10 02 00
8001CDF0: addu    $v0, $s0                                                 ; 21 10 50 00
8001CDF4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CDF8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CDFC: lw      $ra, 0x20+var_s24($sp)                                   ; 44 00 BF 8F
8001CE00: lw      $fp, 0x20+var_s20($sp)                                   ; 40 00 BE 8F
8001CE04: lw      $s7, 0x20+var_s1C($sp)                                   ; 3C 00 B7 8F
8001CE08: lw      $s6, 0x20+var_s18($sp)                                   ; 38 00 B6 8F
8001CE0C: lw      $s5, 0x20+var_s14($sp)                                   ; 34 00 B5 8F
8001CE10: lw      $s4, 0x20+var_s10($sp)                                   ; 30 00 B4 8F
8001CE14: lw      $s3, 0x20+var_sC($sp)                                    ; 2C 00 B3 8F
8001CE18: lw      $s2, 0x20+var_s8($sp)                                    ; 28 00 B2 8F
8001CE1C: lw      $s1, 0x20+var_s4($sp)                                    ; 24 00 B1 8F
8001CE20: lw      $s0, 0x20+var_s0($sp)                                    ; 20 00 B0 8F
8001CE24: addiu   $sp, 0x48                                                ; 48 00 BD 27
8001CE28: jr      $ra                                                      ; 08 00 E0 03
8001CE2C: nop                                                              ; 00 00 00 00
```

## `8001CE30` `sub_8001CE30`
Bounds: `8001CE30-8001D74C`

### Pseudocode
```c
int __fastcall sub_8001CE30(int a1)
{
  int v1; // $gp
  int v3; // $s4
  int v4; // $s6
  int v5; // $s5
  int v6; // $s1
  int v7; // $gp
  int v8; // $gp
  int v9; // $gp
  int v10; // $gp
  int v11; // $gp
  int v12; // $gp
  int v13; // $gp
  int v14; // $s4
  int v15; // $s5
  int v16; // $s3
  int v17; // $gp
  int v18; // $gp
  int v19; // $gp
  int v20; // $gp
  int v21; // $gp
  int v22; // $gp
  int v23; // $gp
  int v24; // $gp
  int v25; // $gp
  int v26; // $gp
  int v27; // $gp
  int v28; // $gp
  int v29; // $gp
  int v30; // $gp
  int v31; // $gp
  int v32; // $gp
  int v33; // $gp
  int v34; // $gp
  int v35; // $gp
  int v36; // $gp
  int v37; // $gp
  int v38; // $gp
  int v39; // $gp
  int v40; // $gp
  int v41; // $gp
  int v42; // $gp
  int v43; // $gp
  int v44; // $gp

  v3 = 0;
  v4 = 1310720;
  v5 = 2621440;
  v6 = 40;
  do
  {
    ++v3;
    sub_8001B590(v6, 0, dword_8004E900, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v1 + 872));
    sub_8001B590(v4 >> 16, 0, dword_8004E910, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v7 + 872));
    sub_8001B590(v6, 220, dword_8004E920, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v8 + 872));
    sub_8001B590(v4 >> 16, 220, dword_8004E930, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v9 + 872));
    sub_8001B590(v6, 20, dword_8004E920, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v10 + 872));
    sub_8001B590(v4 >> 16, 20, dword_8004E930, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v11 + 872));
    sub_8001B590(v6, 200, dword_8004E900, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v12 + 872));
    sub_8001B590(v4 >> 16, 200, dword_8004E910, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v13 + 872));
    v4 += 2621440;
    v5 += 2621440;
    v6 = v5 >> 16;
  }
  while ( v3 < 7 );
  v14 = 0;
  v15 = 0;
  v16 = 1310720;
  do
  {
    ++v14;
    sub_8001B590(0, v16 >> 16, dword_8004E920, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v1 + 872));
    sub_8001B590(0, v15 >> 16, dword_8004E900, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v17 + 872));
    sub_8001B590(300, v16 >> 16, dword_8004E930, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v18 + 872));
    sub_8001B590(300, v15 >> 16, dword_8004E910, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v19 + 872));
    v15 += 2621440;
    v16 += 2621440;
  }
  while ( v14 < 6 );
  sub_8001B590(40, 180, dword_8004EA60, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v1 + 872));
  sub_8001B590(60, 180, dword_8004EA70, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v20 + 872));
  sub_8001B590(80, 180, dword_8004EA80, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v21 + 872));
  sub_8001B590(100, 180, dword_8004EA90, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v22 + 872));
  sub_8001B590(120, 180, dword_8004EAA0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v23 + 872));
  sub_8001B590(140, 180, dword_8004EAB0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v24 + 872));
  sub_8001B590(160, 180, dword_8004EAC0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v25 + 872));
  sub_8001B590(180, 180, dword_8004EAD0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v26 + 872));
  sub_8001B590(200, 180, dword_8004EAE0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v27 + 872));
  sub_8001B590(220, 180, dword_8004EAF0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v28 + 872));
  sub_8001B590(240, 180, dword_8004EB00, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v29 + 872));
  sub_8001B590(260, 180, dword_8004EB10, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v30 + 872));
  sub_8001B590(280, 40, dword_8004E980, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v31 + 872));
  sub_8001B590(20, 40, dword_8004EA50, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v32 + 872));
  sub_8001B590(160, 40, dword_8004E9F0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v33 + 872));
  sub_8001B590(140, 40, dword_8004E9E0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v34 + 872));
  sub_8001B590(180, 40, dword_8004EA00, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v35 + 872));
  sub_8001B590(120, 40, dword_8004E9D0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v36 + 872));
  sub_8001B590(200, 40, dword_8004EA10, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v37 + 872));
  sub_8001B590(100, 40, dword_8004E9C0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v38 + 872));
  sub_8001B590(240, 40, dword_8004EA30, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v39 + 872));
  sub_8001B590(80, 40, dword_8004E9B0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v40 + 872));
  sub_8001B590(220, 40, dword_8004EA20, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v41 + 872));
  sub_8001B590(60, 40, dword_8004E9A0, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v42 + 872));
  sub_8001B590(260, 40, dword_8004EA40, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v43 + 872));
  return sub_8001B590(40, 40, dword_8004E990, 0, 0, a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v44 + 872));
}

```

### Calls in function
- 8001CEA4: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CEE0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CF18: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CF50: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CF88: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CFC0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001CFF8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D030: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D048: `bnez    $v0, loc_8001CE70` -> 8001CE70 loc_8001CE70
- 8001D09C: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D0D8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D110: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D148: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D160: `bnez    $v0, loc_8001D068` -> 8001D068 loc_8001D068
- 8001D1A0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D1D8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D210: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D248: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D280: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D2B8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D2F0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D328: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D360: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D398: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D3D0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D408: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D440: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D478: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D4B0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D4E8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D520: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D558: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D590: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D5C8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D600: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D638: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D670: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D6A8: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D6E0: `jal     sub_8001B590` -> 8001B590 sub_8001B590
- 8001D718: `jal     sub_8001B590` -> 8001B590 sub_8001B590

### Xrefs to
- 8001ED18 -> 8001CE30 type=17 func=sub_8001EC54 line=`jal     sub_8001CE30`
- 80020270 -> 8001CE30 type=17 func=sub_80020248 line=`jal     sub_8001CE30`

### Xrefs from
- 8001CE30 -> 8001CE34 type=21 target=sub_8001CE34 line=`addiu   $sp, -0x40`
- 8001CE34 -> 8001CE38 type=21 target=sub_8001CE38 line=`sw      $s2, 0x20+var_s8($sp)`
- 8001CE38 -> 8001CE3C type=21 target=sub_8001CE3C line=`move    $s2, $a0`
- 8001CE3C -> 8001CE40 type=21 target=sub_8001CE40 line=`sw      $s4, 0x20+var_s10($sp)`
- 8001CE40 -> 8001CE44 type=21 target=sub_8001CE44 line=`move    $s4, $zero`
- 8001CE44 -> 8001CE48 type=21 target=sub_8001CE48 line=`sw      $s3, 0x20+var_sC($sp)`
- 8001CE48 -> 8001CE50 type=21 target=sub_8001CE50 line=`li      $s3, unk_80087288`
- 8001CE50 -> 8001CE54 type=21 target=sub_8001CE54 line=`sw      $s6, 0x20+var_s18($sp)`
- 8001CE54 -> 8001CE58 type=21 target=sub_8001CE58 line=`lui     $s6, 0x14`
- 8001CE58 -> 8001CE5C type=21 target=sub_8001CE5C line=`sw      $s5, 0x20+var_s14($sp)`
- 8001CE5C -> 8001CE60 type=21 target=sub_8001CE60 line=`lui     $s5, 0x28  # '('`
- 8001CE60 -> 8001CE64 type=21 target=sub_8001CE64 line=`sw      $ra, 0x20+var_s1C($sp)`
- 8001CE64 -> 8001CE68 type=21 target=sub_8001CE68 line=`sw      $s1, 0x20+var_s4($sp)`
- 8001CE68 -> 8001CE6C type=21 target=sub_8001CE6C line=`sw      $s0, 0x20+var_s0($sp)`
- 8001CE6C -> 8001CE70 type=21 target=loc_8001CE70 line=`sra     $s1, $s5, 16`
- 8001CE70 -> 8001CE74 type=21 target=sub_8001CE74 line=`move    $a0, $s1`
- 8001CE74 -> 8001CE78 type=21 target=sub_8001CE78 line=`move    $a1, $zero`
- 8001CE78 -> 8001CE80 type=21 target=sub_8001CE80 line=`li      $a2, dword_8004E900`
- 8001CE80 -> 8001CE84 type=21 target=sub_8001CE84 line=`move    $a3, $zero`
- 8001CE84 -> 8001CE88 type=21 target=sub_8001CE88 line=`lw      $v1, 0x368($gp)`
- 8001CE88 -> 8001CE8C type=21 target=sub_8001CE8C line=`addiu   $s4, 1`
- 8001CE8C -> 8001CE90 type=21 target=sub_8001CE90 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CE90 -> 8001CE94 type=21 target=sub_8001CE94 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CE94 -> 8001CE98 type=21 target=sub_8001CE98 line=`sll     $v0, $v1, 2`
- 8001CE98 -> 8001CE9C type=21 target=sub_8001CE9C line=`addu    $v0, $v1`
- 8001CE9C -> 8001CEA0 type=21 target=sub_8001CEA0 line=`sll     $v0, 2`
- 8001CEA0 -> 8001CEA4 type=21 target=sub_8001CEA4 line=`addu    $v0, $s3`
- 8001CEA4 -> 8001CEA8 type=21 target=sub_8001CEA8 line=`jal     sub_8001B590`
- 8001CEA4 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CEA8 -> 8001CEAC type=21 target=sub_8001CEAC line=`sw      $v0, 0x20+var_8($sp)`
- 8001CEAC -> 8001CEB0 type=21 target=sub_8001CEB0 line=`sra     $s0, $s6, 16`
- 8001CEB0 -> 8001CEB4 type=21 target=sub_8001CEB4 line=`move    $a0, $s0`
- 8001CEB4 -> 8001CEB8 type=21 target=sub_8001CEB8 line=`move    $a1, $zero`
- 8001CEB8 -> 8001CEC0 type=21 target=sub_8001CEC0 line=`li      $a2, dword_8004E910`
- 8001CEC0 -> 8001CEC4 type=21 target=sub_8001CEC4 line=`lw      $v1, 0x368($gp)`
- 8001CEC4 -> 8001CEC8 type=21 target=sub_8001CEC8 line=`move    $a3, $zero`
- 8001CEC8 -> 8001CECC type=21 target=sub_8001CECC line=`sw      $zero, 0x20+var_10($sp)`
- 8001CECC -> 8001CED0 type=21 target=sub_8001CED0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CED0 -> 8001CED4 type=21 target=sub_8001CED4 line=`sll     $v0, $v1, 2`
- 8001CED4 -> 8001CED8 type=21 target=sub_8001CED8 line=`addu    $v0, $v1`
- 8001CED8 -> 8001CEDC type=21 target=sub_8001CEDC line=`sll     $v0, 2`
- 8001CEDC -> 8001CEE0 type=21 target=sub_8001CEE0 line=`addu    $v0, $s3`
- 8001CEE0 -> 8001CEE4 type=21 target=sub_8001CEE4 line=`jal     sub_8001B590`
- 8001CEE0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CEE4 -> 8001CEE8 type=21 target=sub_8001CEE8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CEE8 -> 8001CEEC type=21 target=sub_8001CEEC line=`move    $a0, $s1`
- 8001CEEC -> 8001CEF0 type=21 target=sub_8001CEF0 line=`li      $a1, 0xDC`
- 8001CEF0 -> 8001CEF8 type=21 target=sub_8001CEF8 line=`li      $a2, dword_8004E920`
- 8001CEF8 -> 8001CEFC type=21 target=sub_8001CEFC line=`lw      $v1, 0x368($gp)`
- 8001CEFC -> 8001CF00 type=21 target=sub_8001CF00 line=`move    $a3, $zero`
- 8001CF00 -> 8001CF04 type=21 target=sub_8001CF04 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CF04 -> 8001CF08 type=21 target=sub_8001CF08 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CF08 -> 8001CF0C type=21 target=sub_8001CF0C line=`sll     $v0, $v1, 2`
- 8001CF0C -> 8001CF10 type=21 target=sub_8001CF10 line=`addu    $v0, $v1`
- 8001CF10 -> 8001CF14 type=21 target=sub_8001CF14 line=`sll     $v0, 2`
- 8001CF14 -> 8001CF18 type=21 target=sub_8001CF18 line=`addu    $v0, $s3`
- 8001CF18 -> 8001CF1C type=21 target=sub_8001CF1C line=`jal     sub_8001B590`
- 8001CF18 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CF1C -> 8001CF20 type=21 target=sub_8001CF20 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CF20 -> 8001CF24 type=21 target=sub_8001CF24 line=`move    $a0, $s0`
- 8001CF24 -> 8001CF28 type=21 target=sub_8001CF28 line=`li      $a1, 0xDC`
- 8001CF28 -> 8001CF30 type=21 target=sub_8001CF30 line=`li      $a2, dword_8004E930`
- 8001CF30 -> 8001CF34 type=21 target=sub_8001CF34 line=`lw      $v1, 0x368($gp)`
- 8001CF34 -> 8001CF38 type=21 target=sub_8001CF38 line=`move    $a3, $zero`
- 8001CF38 -> 8001CF3C type=21 target=sub_8001CF3C line=`sw      $zero, 0x20+var_10($sp)`
- 8001CF3C -> 8001CF40 type=21 target=sub_8001CF40 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CF40 -> 8001CF44 type=21 target=sub_8001CF44 line=`sll     $v0, $v1, 2`
- 8001CF44 -> 8001CF48 type=21 target=sub_8001CF48 line=`addu    $v0, $v1`
- 8001CF48 -> 8001CF4C type=21 target=sub_8001CF4C line=`sll     $v0, 2`
- 8001CF4C -> 8001CF50 type=21 target=sub_8001CF50 line=`addu    $v0, $s3`
- 8001CF50 -> 8001CF54 type=21 target=sub_8001CF54 line=`jal     sub_8001B590`
- 8001CF50 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CF54 -> 8001CF58 type=21 target=sub_8001CF58 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CF58 -> 8001CF5C type=21 target=sub_8001CF5C line=`move    $a0, $s1`
- 8001CF5C -> 8001CF60 type=21 target=sub_8001CF60 line=`li      $a1, 0x14`
- 8001CF60 -> 8001CF68 type=21 target=sub_8001CF68 line=`li      $a2, dword_8004E920`
- 8001CF68 -> 8001CF6C type=21 target=sub_8001CF6C line=`lw      $v1, 0x368($gp)`
- 8001CF6C -> 8001CF70 type=21 target=sub_8001CF70 line=`move    $a3, $zero`
- 8001CF70 -> 8001CF74 type=21 target=sub_8001CF74 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CF74 -> 8001CF78 type=21 target=sub_8001CF78 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CF78 -> 8001CF7C type=21 target=sub_8001CF7C line=`sll     $v0, $v1, 2`
- 8001CF7C -> 8001CF80 type=21 target=sub_8001CF80 line=`addu    $v0, $v1`
- 8001CF80 -> 8001CF84 type=21 target=sub_8001CF84 line=`sll     $v0, 2`
- 8001CF84 -> 8001CF88 type=21 target=sub_8001CF88 line=`addu    $v0, $s3`
- 8001CF88 -> 8001CF8C type=21 target=sub_8001CF8C line=`jal     sub_8001B590`
- 8001CF88 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CF8C -> 8001CF90 type=21 target=sub_8001CF90 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CF90 -> 8001CF94 type=21 target=sub_8001CF94 line=`move    $a0, $s0`
- 8001CF94 -> 8001CF98 type=21 target=sub_8001CF98 line=`li      $a1, 0x14`
- 8001CF98 -> 8001CFA0 type=21 target=sub_8001CFA0 line=`li      $a2, dword_8004E930`
- 8001CFA0 -> 8001CFA4 type=21 target=sub_8001CFA4 line=`lw      $v1, 0x368($gp)`
- 8001CFA4 -> 8001CFA8 type=21 target=sub_8001CFA8 line=`move    $a3, $zero`
- 8001CFA8 -> 8001CFAC type=21 target=sub_8001CFAC line=`sw      $zero, 0x20+var_10($sp)`
- 8001CFAC -> 8001CFB0 type=21 target=sub_8001CFB0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CFB0 -> 8001CFB4 type=21 target=sub_8001CFB4 line=`sll     $v0, $v1, 2`
- 8001CFB4 -> 8001CFB8 type=21 target=sub_8001CFB8 line=`addu    $v0, $v1`
- 8001CFB8 -> 8001CFBC type=21 target=sub_8001CFBC line=`sll     $v0, 2`
- 8001CFBC -> 8001CFC0 type=21 target=sub_8001CFC0 line=`addu    $v0, $s3`
- 8001CFC0 -> 8001CFC4 type=21 target=sub_8001CFC4 line=`jal     sub_8001B590`
- 8001CFC0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CFC4 -> 8001CFC8 type=21 target=sub_8001CFC8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001CFC8 -> 8001CFCC type=21 target=sub_8001CFCC line=`move    $a0, $s1`
- 8001CFCC -> 8001CFD0 type=21 target=sub_8001CFD0 line=`li      $a1, 0xC8`
- 8001CFD0 -> 8001CFD8 type=21 target=sub_8001CFD8 line=`li      $a2, dword_8004E900`
- 8001CFD8 -> 8001CFDC type=21 target=sub_8001CFDC line=`lw      $v1, 0x368($gp)`
- 8001CFDC -> 8001CFE0 type=21 target=sub_8001CFE0 line=`move    $a3, $zero`
- 8001CFE0 -> 8001CFE4 type=21 target=sub_8001CFE4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001CFE4 -> 8001CFE8 type=21 target=sub_8001CFE8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001CFE8 -> 8001CFEC type=21 target=sub_8001CFEC line=`sll     $v0, $v1, 2`
- 8001CFEC -> 8001CFF0 type=21 target=sub_8001CFF0 line=`addu    $v0, $v1`
- 8001CFF0 -> 8001CFF4 type=21 target=sub_8001CFF4 line=`sll     $v0, 2`
- 8001CFF4 -> 8001CFF8 type=21 target=sub_8001CFF8 line=`addu    $v0, $s3`
- 8001CFF8 -> 8001CFFC type=21 target=sub_8001CFFC line=`jal     sub_8001B590`
- 8001CFF8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001CFFC -> 8001D000 type=21 target=sub_8001D000 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D000 -> 8001D004 type=21 target=sub_8001D004 line=`move    $a0, $s0`
- 8001D004 -> 8001D008 type=21 target=sub_8001D008 line=`li      $a1, 0xC8`
- 8001D008 -> 8001D010 type=21 target=sub_8001D010 line=`li      $a2, dword_8004E910`
- 8001D010 -> 8001D014 type=21 target=sub_8001D014 line=`lw      $v1, 0x368($gp)`
- 8001D014 -> 8001D018 type=21 target=sub_8001D018 line=`move    $a3, $zero`
- 8001D018 -> 8001D01C type=21 target=sub_8001D01C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D01C -> 8001D020 type=21 target=sub_8001D020 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D020 -> 8001D024 type=21 target=sub_8001D024 line=`sll     $v0, $v1, 2`
- 8001D024 -> 8001D028 type=21 target=sub_8001D028 line=`addu    $v0, $v1`
- 8001D028 -> 8001D02C type=21 target=sub_8001D02C line=`sll     $v0, 2`
- 8001D02C -> 8001D030 type=21 target=sub_8001D030 line=`addu    $v0, $s3`
- 8001D030 -> 8001D034 type=21 target=sub_8001D034 line=`jal     sub_8001B590`
- 8001D030 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D034 -> 8001D038 type=21 target=sub_8001D038 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D038 -> 8001D03C type=21 target=sub_8001D03C line=`lui     $v0, 0x28  # '('`
- 8001D03C -> 8001D040 type=21 target=sub_8001D040 line=`addu    $s6, $v0`
- 8001D040 -> 8001D044 type=21 target=sub_8001D044 line=`addu    $s5, $v0`
- 8001D044 -> 8001D048 type=21 target=sub_8001D048 line=`slti    $v0, $s4, 7`
- 8001D048 -> 8001D04C type=21 target=sub_8001D04C line=`bnez    $v0, loc_8001CE70`
- 8001D048 -> 8001CE70 type=19 target=loc_8001CE70 line=`bnez    $v0, loc_8001CE70`
- 8001D04C -> 8001D050 type=21 target=sub_8001D050 line=`sra     $s1, $s5, 16`
- 8001D050 -> 8001D054 type=21 target=sub_8001D054 line=`move    $s4, $zero`
- 8001D054 -> 8001D05C type=21 target=sub_8001D05C line=`li      $s6, unk_80087288`
- 8001D05C -> 8001D060 type=21 target=sub_8001D060 line=`move    $s5, $zero`
- 8001D060 -> 8001D064 type=21 target=sub_8001D064 line=`lui     $s3, 0x14`
- 8001D064 -> 8001D068 type=21 target=loc_8001D068 line=`move    $a0, $zero`
- 8001D068 -> 8001D06C type=21 target=sub_8001D06C line=`sra     $s1, $s3, 16`
- 8001D06C -> 8001D070 type=21 target=sub_8001D070 line=`move    $a1, $s1`
- 8001D070 -> 8001D078 type=21 target=sub_8001D078 line=`li      $a2, dword_8004E920`
- 8001D078 -> 8001D07C type=21 target=sub_8001D07C line=`move    $a3, $zero`
- 8001D07C -> 8001D080 type=21 target=sub_8001D080 line=`lw      $v1, 0x368($gp)`
- 8001D080 -> 8001D084 type=21 target=sub_8001D084 line=`addiu   $s4, 1`
- 8001D084 -> 8001D088 type=21 target=sub_8001D088 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D088 -> 8001D08C type=21 target=sub_8001D08C line=`sw      $s2, 0x20+var_C($sp)`
- 8001D08C -> 8001D090 type=21 target=sub_8001D090 line=`sll     $v0, $v1, 2`
- 8001D090 -> 8001D094 type=21 target=sub_8001D094 line=`addu    $v0, $v1`
- 8001D094 -> 8001D098 type=21 target=sub_8001D098 line=`sll     $v0, 2`
- 8001D098 -> 8001D09C type=21 target=sub_8001D09C line=`addu    $v0, $s6`
- 8001D09C -> 8001D0A0 type=21 target=sub_8001D0A0 line=`jal     sub_8001B590`
- 8001D09C -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D0A0 -> 8001D0A4 type=21 target=sub_8001D0A4 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D0A4 -> 8001D0A8 type=21 target=sub_8001D0A8 line=`move    $a0, $zero`
- 8001D0A8 -> 8001D0AC type=21 target=sub_8001D0AC line=`sra     $s0, $s5, 16`
- 8001D0AC -> 8001D0B0 type=21 target=sub_8001D0B0 line=`move    $a1, $s0`
- 8001D0B0 -> 8001D0B8 type=21 target=sub_8001D0B8 line=`li      $a2, dword_8004E900`
- 8001D0B8 -> 8001D0BC type=21 target=sub_8001D0BC line=`lw      $v1, 0x368($gp)`
- 8001D0BC -> 8001D0C0 type=21 target=sub_8001D0C0 line=`move    $a3, $zero`
- 8001D0C0 -> 8001D0C4 type=21 target=sub_8001D0C4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D0C4 -> 8001D0C8 type=21 target=sub_8001D0C8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D0C8 -> 8001D0CC type=21 target=sub_8001D0CC line=`sll     $v0, $v1, 2`
- 8001D0CC -> 8001D0D0 type=21 target=sub_8001D0D0 line=`addu    $v0, $v1`
- 8001D0D0 -> 8001D0D4 type=21 target=sub_8001D0D4 line=`sll     $v0, 2`
- 8001D0D4 -> 8001D0D8 type=21 target=sub_8001D0D8 line=`addu    $v0, $s6`
- 8001D0D8 -> 8001D0DC type=21 target=sub_8001D0DC line=`jal     sub_8001B590`
- 8001D0D8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D0DC -> 8001D0E0 type=21 target=sub_8001D0E0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D0E0 -> 8001D0E4 type=21 target=sub_8001D0E4 line=`li      $a0, 0x12C`
- 8001D0E4 -> 8001D0E8 type=21 target=sub_8001D0E8 line=`move    $a1, $s1`
- 8001D0E8 -> 8001D0F0 type=21 target=sub_8001D0F0 line=`li      $a2, dword_8004E930`
- 8001D0F0 -> 8001D0F4 type=21 target=sub_8001D0F4 line=`lw      $v1, 0x368($gp)`
- 8001D0F4 -> 8001D0F8 type=21 target=sub_8001D0F8 line=`move    $a3, $zero`
- 8001D0F8 -> 8001D0FC type=21 target=sub_8001D0FC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D0FC -> 8001D100 type=21 target=sub_8001D100 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D100 -> 8001D104 type=21 target=sub_8001D104 line=`sll     $v0, $v1, 2`
- 8001D104 -> 8001D108 type=21 target=sub_8001D108 line=`addu    $v0, $v1`
- 8001D108 -> 8001D10C type=21 target=sub_8001D10C line=`sll     $v0, 2`
- 8001D10C -> 8001D110 type=21 target=sub_8001D110 line=`addu    $v0, $s6`
- 8001D110 -> 8001D114 type=21 target=sub_8001D114 line=`jal     sub_8001B590`
- 8001D110 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D114 -> 8001D118 type=21 target=sub_8001D118 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D118 -> 8001D11C type=21 target=sub_8001D11C line=`li      $a0, 0x12C`
- 8001D11C -> 8001D120 type=21 target=sub_8001D120 line=`move    $a1, $s0`
- 8001D120 -> 8001D128 type=21 target=sub_8001D128 line=`li      $a2, dword_8004E910`
- 8001D128 -> 8001D12C type=21 target=sub_8001D12C line=`lw      $v1, 0x368($gp)`
- 8001D12C -> 8001D130 type=21 target=sub_8001D130 line=`move    $a3, $zero`
- 8001D130 -> 8001D134 type=21 target=sub_8001D134 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D134 -> 8001D138 type=21 target=sub_8001D138 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D138 -> 8001D13C type=21 target=sub_8001D13C line=`sll     $v0, $v1, 2`
- 8001D13C -> 8001D140 type=21 target=sub_8001D140 line=`addu    $v0, $v1`
- 8001D140 -> 8001D144 type=21 target=sub_8001D144 line=`sll     $v0, 2`
- 8001D144 -> 8001D148 type=21 target=sub_8001D148 line=`addu    $v0, $s6`
- 8001D148 -> 8001D14C type=21 target=sub_8001D14C line=`jal     sub_8001B590`
- 8001D148 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D14C -> 8001D150 type=21 target=sub_8001D150 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D150 -> 8001D154 type=21 target=sub_8001D154 line=`lui     $v0, 0x28  # '('`
- 8001D154 -> 8001D158 type=21 target=sub_8001D158 line=`addu    $s5, $v0`
- 8001D158 -> 8001D15C type=21 target=sub_8001D15C line=`addu    $s3, $v0`
- 8001D15C -> 8001D160 type=21 target=sub_8001D160 line=`slti    $v0, $s4, 6`
- 8001D160 -> 8001D164 type=21 target=sub_8001D164 line=`bnez    $v0, loc_8001D068`
- 8001D160 -> 8001D068 type=19 target=loc_8001D068 line=`bnez    $v0, loc_8001D068`
- 8001D164 -> 8001D168 type=21 target=sub_8001D168 line=`move    $a0, $zero`
- 8001D168 -> 8001D16C type=21 target=sub_8001D16C line=`li      $a0, 0x28  # '('`
- 8001D16C -> 8001D170 type=21 target=sub_8001D170 line=`li      $a1, 0xB4`
- 8001D170 -> 8001D178 type=21 target=sub_8001D178 line=`li      $a2, dword_8004EA60`
- 8001D178 -> 8001D17C type=21 target=sub_8001D17C line=`move    $a3, $zero`
- 8001D17C -> 8001D180 type=21 target=sub_8001D180 line=`lw      $v1, 0x368($gp)`
- 8001D180 -> 8001D188 type=21 target=sub_8001D188 line=`li      $s0, unk_80087288`
- 8001D188 -> 8001D18C type=21 target=sub_8001D18C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D18C -> 8001D190 type=21 target=sub_8001D190 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D190 -> 8001D194 type=21 target=sub_8001D194 line=`sll     $v0, $v1, 2`
- 8001D194 -> 8001D198 type=21 target=sub_8001D198 line=`addu    $v0, $v1`
- 8001D198 -> 8001D19C type=21 target=sub_8001D19C line=`sll     $v0, 2`
- 8001D19C -> 8001D1A0 type=21 target=sub_8001D1A0 line=`addu    $v0, $s0`
- 8001D1A0 -> 8001D1A4 type=21 target=sub_8001D1A4 line=`jal     sub_8001B590`
- 8001D1A0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D1A4 -> 8001D1A8 type=21 target=sub_8001D1A8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D1A8 -> 8001D1AC type=21 target=sub_8001D1AC line=`li      $a0, 0x3C  # '<'`
- 8001D1AC -> 8001D1B0 type=21 target=sub_8001D1B0 line=`li      $a1, 0xB4`
- 8001D1B0 -> 8001D1B8 type=21 target=sub_8001D1B8 line=`li      $a2, dword_8004EA70`
- 8001D1B8 -> 8001D1BC type=21 target=sub_8001D1BC line=`lw      $v1, 0x368($gp)`
- 8001D1BC -> 8001D1C0 type=21 target=sub_8001D1C0 line=`move    $a3, $zero`
- 8001D1C0 -> 8001D1C4 type=21 target=sub_8001D1C4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D1C4 -> 8001D1C8 type=21 target=sub_8001D1C8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D1C8 -> 8001D1CC type=21 target=sub_8001D1CC line=`sll     $v0, $v1, 2`
- 8001D1CC -> 8001D1D0 type=21 target=sub_8001D1D0 line=`addu    $v0, $v1`
- 8001D1D0 -> 8001D1D4 type=21 target=sub_8001D1D4 line=`sll     $v0, 2`
- 8001D1D4 -> 8001D1D8 type=21 target=sub_8001D1D8 line=`addu    $v0, $s0`
- 8001D1D8 -> 8001D1DC type=21 target=sub_8001D1DC line=`jal     sub_8001B590`
- 8001D1D8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D1DC -> 8001D1E0 type=21 target=sub_8001D1E0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D1E0 -> 8001D1E4 type=21 target=sub_8001D1E4 line=`li      $a0, 0x50  # 'P'`
- 8001D1E4 -> 8001D1E8 type=21 target=sub_8001D1E8 line=`li      $a1, 0xB4`
- 8001D1E8 -> 8001D1F0 type=21 target=sub_8001D1F0 line=`li      $a2, dword_8004EA80`
- 8001D1F0 -> 8001D1F4 type=21 target=sub_8001D1F4 line=`lw      $v1, 0x368($gp)`
- 8001D1F4 -> 8001D1F8 type=21 target=sub_8001D1F8 line=`move    $a3, $zero`
- 8001D1F8 -> 8001D1FC type=21 target=sub_8001D1FC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D1FC -> 8001D200 type=21 target=sub_8001D200 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D200 -> 8001D204 type=21 target=sub_8001D204 line=`sll     $v0, $v1, 2`
- 8001D204 -> 8001D208 type=21 target=sub_8001D208 line=`addu    $v0, $v1`
- 8001D208 -> 8001D20C type=21 target=sub_8001D20C line=`sll     $v0, 2`
- 8001D20C -> 8001D210 type=21 target=sub_8001D210 line=`addu    $v0, $s0`
- 8001D210 -> 8001D214 type=21 target=sub_8001D214 line=`jal     sub_8001B590`
- 8001D210 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D214 -> 8001D218 type=21 target=sub_8001D218 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D218 -> 8001D21C type=21 target=sub_8001D21C line=`li      $a0, 0x64  # 'd'`
- 8001D21C -> 8001D220 type=21 target=sub_8001D220 line=`li      $a1, 0xB4`
- 8001D220 -> 8001D228 type=21 target=sub_8001D228 line=`li      $a2, dword_8004EA90`
- 8001D228 -> 8001D22C type=21 target=sub_8001D22C line=`lw      $v1, 0x368($gp)`
- 8001D22C -> 8001D230 type=21 target=sub_8001D230 line=`move    $a3, $zero`
- 8001D230 -> 8001D234 type=21 target=sub_8001D234 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D234 -> 8001D238 type=21 target=sub_8001D238 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D238 -> 8001D23C type=21 target=sub_8001D23C line=`sll     $v0, $v1, 2`
- 8001D23C -> 8001D240 type=21 target=sub_8001D240 line=`addu    $v0, $v1`
- 8001D240 -> 8001D244 type=21 target=sub_8001D244 line=`sll     $v0, 2`
- 8001D244 -> 8001D248 type=21 target=sub_8001D248 line=`addu    $v0, $s0`
- 8001D248 -> 8001D24C type=21 target=sub_8001D24C line=`jal     sub_8001B590`
- 8001D248 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D24C -> 8001D250 type=21 target=sub_8001D250 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D250 -> 8001D254 type=21 target=sub_8001D254 line=`li      $a0, 0x78  # 'x'`
- 8001D254 -> 8001D258 type=21 target=sub_8001D258 line=`li      $a1, 0xB4`
- 8001D258 -> 8001D260 type=21 target=sub_8001D260 line=`li      $a2, dword_8004EAA0`
- 8001D260 -> 8001D264 type=21 target=sub_8001D264 line=`lw      $v1, 0x368($gp)`
- 8001D264 -> 8001D268 type=21 target=sub_8001D268 line=`move    $a3, $zero`
- 8001D268 -> 8001D26C type=21 target=sub_8001D26C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D26C -> 8001D270 type=21 target=sub_8001D270 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D270 -> 8001D274 type=21 target=sub_8001D274 line=`sll     $v0, $v1, 2`
- 8001D274 -> 8001D278 type=21 target=sub_8001D278 line=`addu    $v0, $v1`
- 8001D278 -> 8001D27C type=21 target=sub_8001D27C line=`sll     $v0, 2`
- 8001D27C -> 8001D280 type=21 target=sub_8001D280 line=`addu    $v0, $s0`
- 8001D280 -> 8001D284 type=21 target=sub_8001D284 line=`jal     sub_8001B590`
- 8001D280 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D284 -> 8001D288 type=21 target=sub_8001D288 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D288 -> 8001D28C type=21 target=sub_8001D28C line=`li      $a0, 0x8C`
- 8001D28C -> 8001D290 type=21 target=sub_8001D290 line=`li      $a1, 0xB4`
- 8001D290 -> 8001D298 type=21 target=sub_8001D298 line=`li      $a2, dword_8004EAB0`
- 8001D298 -> 8001D29C type=21 target=sub_8001D29C line=`lw      $v1, 0x368($gp)`
- 8001D29C -> 8001D2A0 type=21 target=sub_8001D2A0 line=`move    $a3, $zero`
- 8001D2A0 -> 8001D2A4 type=21 target=sub_8001D2A4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D2A4 -> 8001D2A8 type=21 target=sub_8001D2A8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D2A8 -> 8001D2AC type=21 target=sub_8001D2AC line=`sll     $v0, $v1, 2`
- 8001D2AC -> 8001D2B0 type=21 target=sub_8001D2B0 line=`addu    $v0, $v1`
- 8001D2B0 -> 8001D2B4 type=21 target=sub_8001D2B4 line=`sll     $v0, 2`
- 8001D2B4 -> 8001D2B8 type=21 target=sub_8001D2B8 line=`addu    $v0, $s0`
- 8001D2B8 -> 8001D2BC type=21 target=sub_8001D2BC line=`jal     sub_8001B590`
- 8001D2B8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D2BC -> 8001D2C0 type=21 target=sub_8001D2C0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D2C0 -> 8001D2C4 type=21 target=sub_8001D2C4 line=`li      $a0, 0xA0`
- 8001D2C4 -> 8001D2C8 type=21 target=sub_8001D2C8 line=`li      $a1, 0xB4`
- 8001D2C8 -> 8001D2D0 type=21 target=sub_8001D2D0 line=`li      $a2, dword_8004EAC0`
- 8001D2D0 -> 8001D2D4 type=21 target=sub_8001D2D4 line=`lw      $v1, 0x368($gp)`
- 8001D2D4 -> 8001D2D8 type=21 target=sub_8001D2D8 line=`move    $a3, $zero`
- 8001D2D8 -> 8001D2DC type=21 target=sub_8001D2DC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D2DC -> 8001D2E0 type=21 target=sub_8001D2E0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D2E0 -> 8001D2E4 type=21 target=sub_8001D2E4 line=`sll     $v0, $v1, 2`
- 8001D2E4 -> 8001D2E8 type=21 target=sub_8001D2E8 line=`addu    $v0, $v1`
- 8001D2E8 -> 8001D2EC type=21 target=sub_8001D2EC line=`sll     $v0, 2`
- 8001D2EC -> 8001D2F0 type=21 target=sub_8001D2F0 line=`addu    $v0, $s0`
- 8001D2F0 -> 8001D2F4 type=21 target=sub_8001D2F4 line=`jal     sub_8001B590`
- 8001D2F0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D2F4 -> 8001D2F8 type=21 target=sub_8001D2F8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D2F8 -> 8001D2FC type=21 target=sub_8001D2FC line=`li      $a0, 0xB4`
- 8001D2FC -> 8001D300 type=21 target=sub_8001D300 line=`li      $a1, 0xB4`
- 8001D300 -> 8001D308 type=21 target=sub_8001D308 line=`li      $a2, dword_8004EAD0`
- 8001D308 -> 8001D30C type=21 target=sub_8001D30C line=`lw      $v1, 0x368($gp)`
- 8001D30C -> 8001D310 type=21 target=sub_8001D310 line=`move    $a3, $zero`
- 8001D310 -> 8001D314 type=21 target=sub_8001D314 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D314 -> 8001D318 type=21 target=sub_8001D318 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D318 -> 8001D31C type=21 target=sub_8001D31C line=`sll     $v0, $v1, 2`
- 8001D31C -> 8001D320 type=21 target=sub_8001D320 line=`addu    $v0, $v1`
- 8001D320 -> 8001D324 type=21 target=sub_8001D324 line=`sll     $v0, 2`
- 8001D324 -> 8001D328 type=21 target=sub_8001D328 line=`addu    $v0, $s0`
- 8001D328 -> 8001D32C type=21 target=sub_8001D32C line=`jal     sub_8001B590`
- 8001D328 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D32C -> 8001D330 type=21 target=sub_8001D330 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D330 -> 8001D334 type=21 target=sub_8001D334 line=`li      $a0, 0xC8`
- 8001D334 -> 8001D338 type=21 target=sub_8001D338 line=`li      $a1, 0xB4`
- 8001D338 -> 8001D340 type=21 target=sub_8001D340 line=`li      $a2, dword_8004EAE0`
- 8001D340 -> 8001D344 type=21 target=sub_8001D344 line=`lw      $v1, 0x368($gp)`
- 8001D344 -> 8001D348 type=21 target=sub_8001D348 line=`move    $a3, $zero`
- 8001D348 -> 8001D34C type=21 target=sub_8001D34C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D34C -> 8001D350 type=21 target=sub_8001D350 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D350 -> 8001D354 type=21 target=sub_8001D354 line=`sll     $v0, $v1, 2`
- 8001D354 -> 8001D358 type=21 target=sub_8001D358 line=`addu    $v0, $v1`
- 8001D358 -> 8001D35C type=21 target=sub_8001D35C line=`sll     $v0, 2`
- 8001D35C -> 8001D360 type=21 target=sub_8001D360 line=`addu    $v0, $s0`
- 8001D360 -> 8001D364 type=21 target=sub_8001D364 line=`jal     sub_8001B590`
- 8001D360 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D364 -> 8001D368 type=21 target=sub_8001D368 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D368 -> 8001D36C type=21 target=sub_8001D36C line=`li      $a0, 0xDC`
- 8001D36C -> 8001D370 type=21 target=sub_8001D370 line=`li      $a1, 0xB4`
- 8001D370 -> 8001D378 type=21 target=sub_8001D378 line=`li      $a2, dword_8004EAF0`
- 8001D378 -> 8001D37C type=21 target=sub_8001D37C line=`lw      $v1, 0x368($gp)`
- 8001D37C -> 8001D380 type=21 target=sub_8001D380 line=`move    $a3, $zero`
- 8001D380 -> 8001D384 type=21 target=sub_8001D384 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D384 -> 8001D388 type=21 target=sub_8001D388 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D388 -> 8001D38C type=21 target=sub_8001D38C line=`sll     $v0, $v1, 2`
- 8001D38C -> 8001D390 type=21 target=sub_8001D390 line=`addu    $v0, $v1`
- 8001D390 -> 8001D394 type=21 target=sub_8001D394 line=`sll     $v0, 2`
- 8001D394 -> 8001D398 type=21 target=sub_8001D398 line=`addu    $v0, $s0`
- 8001D398 -> 8001D39C type=21 target=sub_8001D39C line=`jal     sub_8001B590`
- 8001D398 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D39C -> 8001D3A0 type=21 target=sub_8001D3A0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D3A0 -> 8001D3A4 type=21 target=sub_8001D3A4 line=`li      $a0, 0xF0`
- 8001D3A4 -> 8001D3A8 type=21 target=sub_8001D3A8 line=`li      $a1, 0xB4`
- 8001D3A8 -> 8001D3B0 type=21 target=sub_8001D3B0 line=`li      $a2, dword_8004EB00`
- 8001D3B0 -> 8001D3B4 type=21 target=sub_8001D3B4 line=`lw      $v1, 0x368($gp)`
- 8001D3B4 -> 8001D3B8 type=21 target=sub_8001D3B8 line=`move    $a3, $zero`
- 8001D3B8 -> 8001D3BC type=21 target=sub_8001D3BC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D3BC -> 8001D3C0 type=21 target=sub_8001D3C0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D3C0 -> 8001D3C4 type=21 target=sub_8001D3C4 line=`sll     $v0, $v1, 2`
- 8001D3C4 -> 8001D3C8 type=21 target=sub_8001D3C8 line=`addu    $v0, $v1`
- 8001D3C8 -> 8001D3CC type=21 target=sub_8001D3CC line=`sll     $v0, 2`
- 8001D3CC -> 8001D3D0 type=21 target=sub_8001D3D0 line=`addu    $v0, $s0`
- 8001D3D0 -> 8001D3D4 type=21 target=sub_8001D3D4 line=`jal     sub_8001B590`
- 8001D3D0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D3D4 -> 8001D3D8 type=21 target=sub_8001D3D8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D3D8 -> 8001D3DC type=21 target=sub_8001D3DC line=`li      $a0, 0x104`
- 8001D3DC -> 8001D3E0 type=21 target=sub_8001D3E0 line=`li      $a1, 0xB4`
- 8001D3E0 -> 8001D3E8 type=21 target=sub_8001D3E8 line=`li      $a2, dword_8004EB10`
- 8001D3E8 -> 8001D3EC type=21 target=sub_8001D3EC line=`lw      $v1, 0x368($gp)`
- 8001D3EC -> 8001D3F0 type=21 target=sub_8001D3F0 line=`move    $a3, $zero`
- 8001D3F0 -> 8001D3F4 type=21 target=sub_8001D3F4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D3F4 -> 8001D3F8 type=21 target=sub_8001D3F8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D3F8 -> 8001D3FC type=21 target=sub_8001D3FC line=`sll     $v0, $v1, 2`
- 8001D3FC -> 8001D400 type=21 target=sub_8001D400 line=`addu    $v0, $v1`
- 8001D400 -> 8001D404 type=21 target=sub_8001D404 line=`sll     $v0, 2`
- 8001D404 -> 8001D408 type=21 target=sub_8001D408 line=`addu    $v0, $s0`
- 8001D408 -> 8001D40C type=21 target=sub_8001D40C line=`jal     sub_8001B590`
- 8001D408 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D40C -> 8001D410 type=21 target=sub_8001D410 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D410 -> 8001D414 type=21 target=sub_8001D414 line=`li      $a0, 0x118`
- 8001D414 -> 8001D418 type=21 target=sub_8001D418 line=`li      $a1, 0x28  # '('`
- 8001D418 -> 8001D420 type=21 target=sub_8001D420 line=`li      $a2, dword_8004E980`
- 8001D420 -> 8001D424 type=21 target=sub_8001D424 line=`lw      $v1, 0x368($gp)`
- 8001D424 -> 8001D428 type=21 target=sub_8001D428 line=`move    $a3, $zero`
- 8001D428 -> 8001D42C type=21 target=sub_8001D42C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D42C -> 8001D430 type=21 target=sub_8001D430 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D430 -> 8001D434 type=21 target=sub_8001D434 line=`sll     $v0, $v1, 2`
- 8001D434 -> 8001D438 type=21 target=sub_8001D438 line=`addu    $v0, $v1`
- 8001D438 -> 8001D43C type=21 target=sub_8001D43C line=`sll     $v0, 2`
- 8001D43C -> 8001D440 type=21 target=sub_8001D440 line=`addu    $v0, $s0`
- 8001D440 -> 8001D444 type=21 target=sub_8001D444 line=`jal     sub_8001B590`
- 8001D440 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D444 -> 8001D448 type=21 target=sub_8001D448 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D448 -> 8001D44C type=21 target=sub_8001D44C line=`li      $a0, 0x14`
- 8001D44C -> 8001D450 type=21 target=sub_8001D450 line=`li      $a1, 0x28  # '('`
- 8001D450 -> 8001D458 type=21 target=sub_8001D458 line=`li      $a2, dword_8004EA50`
- 8001D458 -> 8001D45C type=21 target=sub_8001D45C line=`lw      $v1, 0x368($gp)`
- 8001D45C -> 8001D460 type=21 target=sub_8001D460 line=`move    $a3, $zero`
- 8001D460 -> 8001D464 type=21 target=sub_8001D464 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D464 -> 8001D468 type=21 target=sub_8001D468 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D468 -> 8001D46C type=21 target=sub_8001D46C line=`sll     $v0, $v1, 2`
- 8001D46C -> 8001D470 type=21 target=sub_8001D470 line=`addu    $v0, $v1`
- 8001D470 -> 8001D474 type=21 target=sub_8001D474 line=`sll     $v0, 2`
- 8001D474 -> 8001D478 type=21 target=sub_8001D478 line=`addu    $v0, $s0`
- 8001D478 -> 8001D47C type=21 target=sub_8001D47C line=`jal     sub_8001B590`
- 8001D478 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D47C -> 8001D480 type=21 target=sub_8001D480 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D480 -> 8001D484 type=21 target=sub_8001D484 line=`li      $a0, 0xA0`
- 8001D484 -> 8001D488 type=21 target=sub_8001D488 line=`li      $a1, 0x28  # '('`
- 8001D488 -> 8001D490 type=21 target=sub_8001D490 line=`li      $a2, dword_8004E9F0`
- 8001D490 -> 8001D494 type=21 target=sub_8001D494 line=`lw      $v1, 0x368($gp)`
- 8001D494 -> 8001D498 type=21 target=sub_8001D498 line=`move    $a3, $zero`
- 8001D498 -> 8001D49C type=21 target=sub_8001D49C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D49C -> 8001D4A0 type=21 target=sub_8001D4A0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D4A0 -> 8001D4A4 type=21 target=sub_8001D4A4 line=`sll     $v0, $v1, 2`
- 8001D4A4 -> 8001D4A8 type=21 target=sub_8001D4A8 line=`addu    $v0, $v1`
- 8001D4A8 -> 8001D4AC type=21 target=sub_8001D4AC line=`sll     $v0, 2`
- 8001D4AC -> 8001D4B0 type=21 target=sub_8001D4B0 line=`addu    $v0, $s0`
- 8001D4B0 -> 8001D4B4 type=21 target=sub_8001D4B4 line=`jal     sub_8001B590`
- 8001D4B0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D4B4 -> 8001D4B8 type=21 target=sub_8001D4B8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D4B8 -> 8001D4BC type=21 target=sub_8001D4BC line=`li      $a0, 0x8C`
- 8001D4BC -> 8001D4C0 type=21 target=sub_8001D4C0 line=`li      $a1, 0x28  # '('`
- 8001D4C0 -> 8001D4C8 type=21 target=sub_8001D4C8 line=`li      $a2, dword_8004E9E0`
- 8001D4C8 -> 8001D4CC type=21 target=sub_8001D4CC line=`lw      $v1, 0x368($gp)`
- 8001D4CC -> 8001D4D0 type=21 target=sub_8001D4D0 line=`move    $a3, $zero`
- 8001D4D0 -> 8001D4D4 type=21 target=sub_8001D4D4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D4D4 -> 8001D4D8 type=21 target=sub_8001D4D8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D4D8 -> 8001D4DC type=21 target=sub_8001D4DC line=`sll     $v0, $v1, 2`
- 8001D4DC -> 8001D4E0 type=21 target=sub_8001D4E0 line=`addu    $v0, $v1`
- 8001D4E0 -> 8001D4E4 type=21 target=sub_8001D4E4 line=`sll     $v0, 2`
- 8001D4E4 -> 8001D4E8 type=21 target=sub_8001D4E8 line=`addu    $v0, $s0`
- 8001D4E8 -> 8001D4EC type=21 target=sub_8001D4EC line=`jal     sub_8001B590`
- 8001D4E8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D4EC -> 8001D4F0 type=21 target=sub_8001D4F0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D4F0 -> 8001D4F4 type=21 target=sub_8001D4F4 line=`li      $a0, 0xB4`
- 8001D4F4 -> 8001D4F8 type=21 target=sub_8001D4F8 line=`li      $a1, 0x28  # '('`
- 8001D4F8 -> 8001D500 type=21 target=sub_8001D500 line=`li      $a2, dword_8004EA00`
- 8001D500 -> 8001D504 type=21 target=sub_8001D504 line=`lw      $v1, 0x368($gp)`
- 8001D504 -> 8001D508 type=21 target=sub_8001D508 line=`move    $a3, $zero`
- 8001D508 -> 8001D50C type=21 target=sub_8001D50C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D50C -> 8001D510 type=21 target=sub_8001D510 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D510 -> 8001D514 type=21 target=sub_8001D514 line=`sll     $v0, $v1, 2`
- 8001D514 -> 8001D518 type=21 target=sub_8001D518 line=`addu    $v0, $v1`
- 8001D518 -> 8001D51C type=21 target=sub_8001D51C line=`sll     $v0, 2`
- 8001D51C -> 8001D520 type=21 target=sub_8001D520 line=`addu    $v0, $s0`
- 8001D520 -> 8001D524 type=21 target=sub_8001D524 line=`jal     sub_8001B590`
- 8001D520 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D524 -> 8001D528 type=21 target=sub_8001D528 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D528 -> 8001D52C type=21 target=sub_8001D52C line=`li      $a0, 0x78  # 'x'`
- 8001D52C -> 8001D530 type=21 target=sub_8001D530 line=`li      $a1, 0x28  # '('`
- 8001D530 -> 8001D538 type=21 target=sub_8001D538 line=`li      $a2, dword_8004E9D0`
- 8001D538 -> 8001D53C type=21 target=sub_8001D53C line=`lw      $v1, 0x368($gp)`
- 8001D53C -> 8001D540 type=21 target=sub_8001D540 line=`move    $a3, $zero`
- 8001D540 -> 8001D544 type=21 target=sub_8001D544 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D544 -> 8001D548 type=21 target=sub_8001D548 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D548 -> 8001D54C type=21 target=sub_8001D54C line=`sll     $v0, $v1, 2`
- 8001D54C -> 8001D550 type=21 target=sub_8001D550 line=`addu    $v0, $v1`
- 8001D550 -> 8001D554 type=21 target=sub_8001D554 line=`sll     $v0, 2`
- 8001D554 -> 8001D558 type=21 target=sub_8001D558 line=`addu    $v0, $s0`
- 8001D558 -> 8001D55C type=21 target=sub_8001D55C line=`jal     sub_8001B590`
- 8001D558 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D55C -> 8001D560 type=21 target=sub_8001D560 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D560 -> 8001D564 type=21 target=sub_8001D564 line=`li      $a0, 0xC8`
- 8001D564 -> 8001D568 type=21 target=sub_8001D568 line=`li      $a1, 0x28  # '('`
- 8001D568 -> 8001D570 type=21 target=sub_8001D570 line=`li      $a2, dword_8004EA10`
- 8001D570 -> 8001D574 type=21 target=sub_8001D574 line=`lw      $v1, 0x368($gp)`
- 8001D574 -> 8001D578 type=21 target=sub_8001D578 line=`move    $a3, $zero`
- 8001D578 -> 8001D57C type=21 target=sub_8001D57C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D57C -> 8001D580 type=21 target=sub_8001D580 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D580 -> 8001D584 type=21 target=sub_8001D584 line=`sll     $v0, $v1, 2`
- 8001D584 -> 8001D588 type=21 target=sub_8001D588 line=`addu    $v0, $v1`
- 8001D588 -> 8001D58C type=21 target=sub_8001D58C line=`sll     $v0, 2`
- 8001D58C -> 8001D590 type=21 target=sub_8001D590 line=`addu    $v0, $s0`
- 8001D590 -> 8001D594 type=21 target=sub_8001D594 line=`jal     sub_8001B590`
- 8001D590 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D594 -> 8001D598 type=21 target=sub_8001D598 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D598 -> 8001D59C type=21 target=sub_8001D59C line=`li      $a0, 0x64  # 'd'`
- 8001D59C -> 8001D5A0 type=21 target=sub_8001D5A0 line=`li      $a1, 0x28  # '('`
- 8001D5A0 -> 8001D5A8 type=21 target=sub_8001D5A8 line=`li      $a2, dword_8004E9C0`
- 8001D5A8 -> 8001D5AC type=21 target=sub_8001D5AC line=`lw      $v1, 0x368($gp)`
- 8001D5AC -> 8001D5B0 type=21 target=sub_8001D5B0 line=`move    $a3, $zero`
- 8001D5B0 -> 8001D5B4 type=21 target=sub_8001D5B4 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D5B4 -> 8001D5B8 type=21 target=sub_8001D5B8 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D5B8 -> 8001D5BC type=21 target=sub_8001D5BC line=`sll     $v0, $v1, 2`
- 8001D5BC -> 8001D5C0 type=21 target=sub_8001D5C0 line=`addu    $v0, $v1`
- 8001D5C0 -> 8001D5C4 type=21 target=sub_8001D5C4 line=`sll     $v0, 2`
- 8001D5C4 -> 8001D5C8 type=21 target=sub_8001D5C8 line=`addu    $v0, $s0`
- 8001D5C8 -> 8001D5CC type=21 target=sub_8001D5CC line=`jal     sub_8001B590`
- 8001D5C8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D5CC -> 8001D5D0 type=21 target=sub_8001D5D0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D5D0 -> 8001D5D4 type=21 target=sub_8001D5D4 line=`li      $a0, 0xF0`
- 8001D5D4 -> 8001D5D8 type=21 target=sub_8001D5D8 line=`li      $a1, 0x28  # '('`
- 8001D5D8 -> 8001D5E0 type=21 target=sub_8001D5E0 line=`li      $a2, dword_8004EA30`
- 8001D5E0 -> 8001D5E4 type=21 target=sub_8001D5E4 line=`lw      $v1, 0x368($gp)`
- 8001D5E4 -> 8001D5E8 type=21 target=sub_8001D5E8 line=`move    $a3, $zero`
- 8001D5E8 -> 8001D5EC type=21 target=sub_8001D5EC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D5EC -> 8001D5F0 type=21 target=sub_8001D5F0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D5F0 -> 8001D5F4 type=21 target=sub_8001D5F4 line=`sll     $v0, $v1, 2`
- 8001D5F4 -> 8001D5F8 type=21 target=sub_8001D5F8 line=`addu    $v0, $v1`
- 8001D5F8 -> 8001D5FC type=21 target=sub_8001D5FC line=`sll     $v0, 2`
- 8001D5FC -> 8001D600 type=21 target=sub_8001D600 line=`addu    $v0, $s0`
- 8001D600 -> 8001D604 type=21 target=sub_8001D604 line=`jal     sub_8001B590`
- 8001D600 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D604 -> 8001D608 type=21 target=sub_8001D608 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D608 -> 8001D60C type=21 target=sub_8001D60C line=`li      $a0, 0x50  # 'P'`
- 8001D60C -> 8001D610 type=21 target=sub_8001D610 line=`li      $a1, 0x28  # '('`
- 8001D610 -> 8001D618 type=21 target=sub_8001D618 line=`li      $a2, dword_8004E9B0`
- 8001D618 -> 8001D61C type=21 target=sub_8001D61C line=`lw      $v1, 0x368($gp)`
- 8001D61C -> 8001D620 type=21 target=sub_8001D620 line=`move    $a3, $zero`
- 8001D620 -> 8001D624 type=21 target=sub_8001D624 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D624 -> 8001D628 type=21 target=sub_8001D628 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D628 -> 8001D62C type=21 target=sub_8001D62C line=`sll     $v0, $v1, 2`
- 8001D62C -> 8001D630 type=21 target=sub_8001D630 line=`addu    $v0, $v1`
- 8001D630 -> 8001D634 type=21 target=sub_8001D634 line=`sll     $v0, 2`
- 8001D634 -> 8001D638 type=21 target=sub_8001D638 line=`addu    $v0, $s0`
- 8001D638 -> 8001D63C type=21 target=sub_8001D63C line=`jal     sub_8001B590`
- 8001D638 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D63C -> 8001D640 type=21 target=sub_8001D640 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D640 -> 8001D644 type=21 target=sub_8001D644 line=`li      $a0, 0xDC`
- 8001D644 -> 8001D648 type=21 target=sub_8001D648 line=`li      $a1, 0x28  # '('`
- 8001D648 -> 8001D650 type=21 target=sub_8001D650 line=`li      $a2, dword_8004EA20`
- 8001D650 -> 8001D654 type=21 target=sub_8001D654 line=`lw      $v1, 0x368($gp)`
- 8001D654 -> 8001D658 type=21 target=sub_8001D658 line=`move    $a3, $zero`
- 8001D658 -> 8001D65C type=21 target=sub_8001D65C line=`sw      $zero, 0x20+var_10($sp)`
- 8001D65C -> 8001D660 type=21 target=sub_8001D660 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D660 -> 8001D664 type=21 target=sub_8001D664 line=`sll     $v0, $v1, 2`
- 8001D664 -> 8001D668 type=21 target=sub_8001D668 line=`addu    $v0, $v1`
- 8001D668 -> 8001D66C type=21 target=sub_8001D66C line=`sll     $v0, 2`
- 8001D66C -> 8001D670 type=21 target=sub_8001D670 line=`addu    $v0, $s0`
- 8001D670 -> 8001D674 type=21 target=sub_8001D674 line=`jal     sub_8001B590`
- 8001D670 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D674 -> 8001D678 type=21 target=sub_8001D678 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D678 -> 8001D67C type=21 target=sub_8001D67C line=`li      $a0, 0x3C  # '<'`
- 8001D67C -> 8001D680 type=21 target=sub_8001D680 line=`li      $a1, 0x28  # '('`
- 8001D680 -> 8001D688 type=21 target=sub_8001D688 line=`li      $a2, dword_8004E9A0`
- 8001D688 -> 8001D68C type=21 target=sub_8001D68C line=`lw      $v1, 0x368($gp)`
- 8001D68C -> 8001D690 type=21 target=sub_8001D690 line=`move    $a3, $zero`
- 8001D690 -> 8001D694 type=21 target=sub_8001D694 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D694 -> 8001D698 type=21 target=sub_8001D698 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D698 -> 8001D69C type=21 target=sub_8001D69C line=`sll     $v0, $v1, 2`
- 8001D69C -> 8001D6A0 type=21 target=sub_8001D6A0 line=`addu    $v0, $v1`
- 8001D6A0 -> 8001D6A4 type=21 target=sub_8001D6A4 line=`sll     $v0, 2`
- 8001D6A4 -> 8001D6A8 type=21 target=sub_8001D6A8 line=`addu    $v0, $s0`
- 8001D6A8 -> 8001D6AC type=21 target=sub_8001D6AC line=`jal     sub_8001B590`
- 8001D6A8 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D6AC -> 8001D6B0 type=21 target=sub_8001D6B0 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D6B0 -> 8001D6B4 type=21 target=sub_8001D6B4 line=`li      $a0, 0x104`
- 8001D6B4 -> 8001D6B8 type=21 target=sub_8001D6B8 line=`li      $a1, 0x28  # '('`
- 8001D6B8 -> 8001D6C0 type=21 target=sub_8001D6C0 line=`li      $a2, dword_8004EA40`
- 8001D6C0 -> 8001D6C4 type=21 target=sub_8001D6C4 line=`lw      $v1, 0x368($gp)`
- 8001D6C4 -> 8001D6C8 type=21 target=sub_8001D6C8 line=`move    $a3, $zero`
- 8001D6C8 -> 8001D6CC type=21 target=sub_8001D6CC line=`sw      $zero, 0x20+var_10($sp)`
- 8001D6CC -> 8001D6D0 type=21 target=sub_8001D6D0 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D6D0 -> 8001D6D4 type=21 target=sub_8001D6D4 line=`sll     $v0, $v1, 2`
- 8001D6D4 -> 8001D6D8 type=21 target=sub_8001D6D8 line=`addu    $v0, $v1`
- 8001D6D8 -> 8001D6DC type=21 target=sub_8001D6DC line=`sll     $v0, 2`
- 8001D6DC -> 8001D6E0 type=21 target=sub_8001D6E0 line=`addu    $v0, $s0`
- 8001D6E0 -> 8001D6E4 type=21 target=sub_8001D6E4 line=`jal     sub_8001B590`
- 8001D6E0 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D6E4 -> 8001D6E8 type=21 target=sub_8001D6E8 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D6E8 -> 8001D6EC type=21 target=sub_8001D6EC line=`li      $a0, 0x28  # '('`
- 8001D6EC -> 8001D6F0 type=21 target=sub_8001D6F0 line=`li      $a1, 0x28  # '('`
- 8001D6F0 -> 8001D6F8 type=21 target=sub_8001D6F8 line=`li      $a2, dword_8004E990`
- 8001D6F8 -> 8001D6FC type=21 target=sub_8001D6FC line=`lw      $v1, 0x368($gp)`
- 8001D6FC -> 8001D700 type=21 target=sub_8001D700 line=`move    $a3, $zero`
- 8001D700 -> 8001D704 type=21 target=sub_8001D704 line=`sw      $zero, 0x20+var_10($sp)`
- 8001D704 -> 8001D708 type=21 target=sub_8001D708 line=`sw      $s2, 0x20+var_C($sp)`
- 8001D708 -> 8001D70C type=21 target=sub_8001D70C line=`sll     $v0, $v1, 2`
- 8001D70C -> 8001D710 type=21 target=sub_8001D710 line=`addu    $v0, $v1`
- 8001D710 -> 8001D714 type=21 target=sub_8001D714 line=`sll     $v0, 2`
- 8001D714 -> 8001D718 type=21 target=sub_8001D718 line=`addu    $v0, $s0`
- 8001D718 -> 8001D71C type=21 target=sub_8001D71C line=`jal     sub_8001B590`
- 8001D718 -> 8001B590 type=17 target=sub_8001B590 line=`jal     sub_8001B590`
- 8001D71C -> 8001D720 type=21 target=sub_8001D720 line=`sw      $v0, 0x20+var_8($sp)`
- 8001D720 -> 8001D724 type=21 target=sub_8001D724 line=`lw      $ra, 0x20+var_s1C($sp)`
- 8001D724 -> 8001D728 type=21 target=sub_8001D728 line=`lw      $s6, 0x20+var_s18($sp)`
- 8001D728 -> 8001D72C type=21 target=sub_8001D72C line=`lw      $s5, 0x20+var_s14($sp)`
- 8001D72C -> 8001D730 type=21 target=sub_8001D730 line=`lw      $s4, 0x20+var_s10($sp)`
- 8001D730 -> 8001D734 type=21 target=sub_8001D734 line=`lw      $s3, 0x20+var_sC($sp)`
- 8001D734 -> 8001D738 type=21 target=sub_8001D738 line=`lw      $s2, 0x20+var_s8($sp)`
- 8001D738 -> 8001D73C type=21 target=sub_8001D73C line=`lw      $s1, 0x20+var_s4($sp)`
- 8001D73C -> 8001D740 type=21 target=sub_8001D740 line=`lw      $s0, 0x20+var_s0($sp)`
- 8001D740 -> 8001D744 type=21 target=sub_8001D744 line=`addiu   $sp, 0x40`
- 8001D744 -> 8001D748 type=21 target=sub_8001D748 line=`jr      $ra`

### Full disasm
```asm
8001CE30: addiu   $sp, -0x40                                               ; C0 FF BD 27
8001CE34: sw      $s2, 0x20+var_s8($sp)                                    ; 28 00 B2 AF
8001CE38: move    $s2, $a0                                                 ; 21 90 80 00
8001CE3C: sw      $s4, 0x20+var_s10($sp)                                   ; 30 00 B4 AF
8001CE40: move    $s4, $zero                                               ; 21 A0 00 00
8001CE44: sw      $s3, 0x20+var_sC($sp)                                    ; 2C 00 B3 AF
8001CE48: li      $s3, unk_80087288                                        ; 08 80 13 3C 88 72 73 26
8001CE50: sw      $s6, 0x20+var_s18($sp)                                   ; 38 00 B6 AF
8001CE54: lui     $s6, 0x14                                                ; 14 00 16 3C
8001CE58: sw      $s5, 0x20+var_s14($sp)                                   ; 34 00 B5 AF
8001CE5C: lui     $s5, 0x28  # '('                                         ; 28 00 15 3C
8001CE60: sw      $ra, 0x20+var_s1C($sp)                                   ; 3C 00 BF AF
8001CE64: sw      $s1, 0x20+var_s4($sp)                                    ; 24 00 B1 AF
8001CE68: sw      $s0, 0x20+var_s0($sp)                                    ; 20 00 B0 AF
8001CE6C: sra     $s1, $s5, 16                                             ; 03 8C 15 00
8001CE70: move    $a0, $s1                                                 ; 21 20 20 02
8001CE74: move    $a1, $zero                                               ; 21 28 00 00
8001CE78: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001CE80: move    $a3, $zero                                               ; 21 38 00 00
8001CE84: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CE88: addiu   $s4, 1                                                   ; 01 00 94 26
8001CE8C: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CE90: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CE94: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CE98: addu    $v0, $v1                                                 ; 21 10 43 00
8001CE9C: sll     $v0, 2                                                   ; 80 10 02 00
8001CEA0: addu    $v0, $s3                                                 ; 21 10 53 00
8001CEA4: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CEA8: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CEAC: sra     $s0, $s6, 16                                             ; 03 84 16 00
8001CEB0: move    $a0, $s0                                                 ; 21 20 00 02
8001CEB4: move    $a1, $zero                                               ; 21 28 00 00
8001CEB8: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001CEC0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CEC4: move    $a3, $zero                                               ; 21 38 00 00
8001CEC8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CECC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CED0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CED4: addu    $v0, $v1                                                 ; 21 10 43 00
8001CED8: sll     $v0, 2                                                   ; 80 10 02 00
8001CEDC: addu    $v0, $s3                                                 ; 21 10 53 00
8001CEE0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CEE4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CEE8: move    $a0, $s1                                                 ; 21 20 20 02
8001CEEC: li      $a1, 0xDC                                                ; DC 00 05 34
8001CEF0: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001CEF8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CEFC: move    $a3, $zero                                               ; 21 38 00 00
8001CF00: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CF04: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CF08: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CF0C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CF10: sll     $v0, 2                                                   ; 80 10 02 00
8001CF14: addu    $v0, $s3                                                 ; 21 10 53 00
8001CF18: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CF1C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CF20: move    $a0, $s0                                                 ; 21 20 00 02
8001CF24: li      $a1, 0xDC                                                ; DC 00 05 34
8001CF28: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001CF30: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CF34: move    $a3, $zero                                               ; 21 38 00 00
8001CF38: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CF3C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CF40: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CF44: addu    $v0, $v1                                                 ; 21 10 43 00
8001CF48: sll     $v0, 2                                                   ; 80 10 02 00
8001CF4C: addu    $v0, $s3                                                 ; 21 10 53 00
8001CF50: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CF54: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CF58: move    $a0, $s1                                                 ; 21 20 20 02
8001CF5C: li      $a1, 0x14                                                ; 14 00 05 34
8001CF60: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001CF68: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CF6C: move    $a3, $zero                                               ; 21 38 00 00
8001CF70: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CF74: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CF78: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CF7C: addu    $v0, $v1                                                 ; 21 10 43 00
8001CF80: sll     $v0, 2                                                   ; 80 10 02 00
8001CF84: addu    $v0, $s3                                                 ; 21 10 53 00
8001CF88: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CF8C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CF90: move    $a0, $s0                                                 ; 21 20 00 02
8001CF94: li      $a1, 0x14                                                ; 14 00 05 34
8001CF98: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001CFA0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CFA4: move    $a3, $zero                                               ; 21 38 00 00
8001CFA8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CFAC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CFB0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CFB4: addu    $v0, $v1                                                 ; 21 10 43 00
8001CFB8: sll     $v0, 2                                                   ; 80 10 02 00
8001CFBC: addu    $v0, $s3                                                 ; 21 10 53 00
8001CFC0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CFC4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001CFC8: move    $a0, $s1                                                 ; 21 20 20 02
8001CFCC: li      $a1, 0xC8                                                ; C8 00 05 34
8001CFD0: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001CFD8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001CFDC: move    $a3, $zero                                               ; 21 38 00 00
8001CFE0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001CFE4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001CFE8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001CFEC: addu    $v0, $v1                                                 ; 21 10 43 00
8001CFF0: sll     $v0, 2                                                   ; 80 10 02 00
8001CFF4: addu    $v0, $s3                                                 ; 21 10 53 00
8001CFF8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001CFFC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D000: move    $a0, $s0                                                 ; 21 20 00 02
8001D004: li      $a1, 0xC8                                                ; C8 00 05 34
8001D008: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001D010: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D014: move    $a3, $zero                                               ; 21 38 00 00
8001D018: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D01C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D020: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D024: addu    $v0, $v1                                                 ; 21 10 43 00
8001D028: sll     $v0, 2                                                   ; 80 10 02 00
8001D02C: addu    $v0, $s3                                                 ; 21 10 53 00
8001D030: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D034: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D038: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001D03C: addu    $s6, $v0                                                 ; 21 B0 C2 02
8001D040: addu    $s5, $v0                                                 ; 21 A8 A2 02
8001D044: slti    $v0, $s4, 7                                              ; 07 00 82 2A
8001D048: bnez    $v0, loc_8001CE70                                        ; 89 FF 40 14
8001D04C: sra     $s1, $s5, 16                                             ; 03 8C 15 00
8001D050: move    $s4, $zero                                               ; 21 A0 00 00
8001D054: li      $s6, unk_80087288                                        ; 08 80 16 3C 88 72 D6 26
8001D05C: move    $s5, $zero                                               ; 21 A8 00 00
8001D060: lui     $s3, 0x14                                                ; 14 00 13 3C
8001D064: move    $a0, $zero                                               ; 21 20 00 00
8001D068: sra     $s1, $s3, 16                                             ; 03 8C 13 00
8001D06C: move    $a1, $s1                                                 ; 21 28 20 02
8001D070: li      $a2, dword_8004E920                                      ; 05 80 06 3C 20 E9 C6 24
8001D078: move    $a3, $zero                                               ; 21 38 00 00
8001D07C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D080: addiu   $s4, 1                                                   ; 01 00 94 26
8001D084: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D088: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D08C: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D090: addu    $v0, $v1                                                 ; 21 10 43 00
8001D094: sll     $v0, 2                                                   ; 80 10 02 00
8001D098: addu    $v0, $s6                                                 ; 21 10 56 00
8001D09C: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D0A0: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D0A4: move    $a0, $zero                                               ; 21 20 00 00
8001D0A8: sra     $s0, $s5, 16                                             ; 03 84 15 00
8001D0AC: move    $a1, $s0                                                 ; 21 28 00 02
8001D0B0: li      $a2, dword_8004E900                                      ; 05 80 06 3C 00 E9 C6 24
8001D0B8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D0BC: move    $a3, $zero                                               ; 21 38 00 00
8001D0C0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D0C4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D0C8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D0CC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D0D0: sll     $v0, 2                                                   ; 80 10 02 00
8001D0D4: addu    $v0, $s6                                                 ; 21 10 56 00
8001D0D8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D0DC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D0E0: li      $a0, 0x12C                                               ; 2C 01 04 34
8001D0E4: move    $a1, $s1                                                 ; 21 28 20 02
8001D0E8: li      $a2, dword_8004E930                                      ; 05 80 06 3C 30 E9 C6 24
8001D0F0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D0F4: move    $a3, $zero                                               ; 21 38 00 00
8001D0F8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D0FC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D100: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D104: addu    $v0, $v1                                                 ; 21 10 43 00
8001D108: sll     $v0, 2                                                   ; 80 10 02 00
8001D10C: addu    $v0, $s6                                                 ; 21 10 56 00
8001D110: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D114: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D118: li      $a0, 0x12C                                               ; 2C 01 04 34
8001D11C: move    $a1, $s0                                                 ; 21 28 00 02
8001D120: li      $a2, dword_8004E910                                      ; 05 80 06 3C 10 E9 C6 24
8001D128: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D12C: move    $a3, $zero                                               ; 21 38 00 00
8001D130: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D134: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D138: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D13C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D140: sll     $v0, 2                                                   ; 80 10 02 00
8001D144: addu    $v0, $s6                                                 ; 21 10 56 00
8001D148: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D14C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D150: lui     $v0, 0x28  # '('                                         ; 28 00 02 3C
8001D154: addu    $s5, $v0                                                 ; 21 A8 A2 02
8001D158: addu    $s3, $v0                                                 ; 21 98 62 02
8001D15C: slti    $v0, $s4, 6                                              ; 06 00 82 2A
8001D160: bnez    $v0, loc_8001D068                                        ; C1 FF 40 14
8001D164: move    $a0, $zero                                               ; 21 20 00 00
8001D168: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001D16C: li      $a1, 0xB4                                                ; B4 00 05 34
8001D170: li      $a2, dword_8004EA60                                      ; 05 80 06 3C 60 EA C6 24
8001D178: move    $a3, $zero                                               ; 21 38 00 00
8001D17C: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D180: li      $s0, unk_80087288                                        ; 08 80 10 3C 88 72 10 26
8001D188: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D18C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D190: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D194: addu    $v0, $v1                                                 ; 21 10 43 00
8001D198: sll     $v0, 2                                                   ; 80 10 02 00
8001D19C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D1A0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D1A4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D1A8: li      $a0, 0x3C  # '<'                                         ; 3C 00 04 34
8001D1AC: li      $a1, 0xB4                                                ; B4 00 05 34
8001D1B0: li      $a2, dword_8004EA70                                      ; 05 80 06 3C 70 EA C6 24
8001D1B8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D1BC: move    $a3, $zero                                               ; 21 38 00 00
8001D1C0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D1C4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D1C8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D1CC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D1D0: sll     $v0, 2                                                   ; 80 10 02 00
8001D1D4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D1D8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D1DC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D1E0: li      $a0, 0x50  # 'P'                                         ; 50 00 04 34
8001D1E4: li      $a1, 0xB4                                                ; B4 00 05 34
8001D1E8: li      $a2, dword_8004EA80                                      ; 05 80 06 3C 80 EA C6 24
8001D1F0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D1F4: move    $a3, $zero                                               ; 21 38 00 00
8001D1F8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D1FC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D200: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D204: addu    $v0, $v1                                                 ; 21 10 43 00
8001D208: sll     $v0, 2                                                   ; 80 10 02 00
8001D20C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D210: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D214: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D218: li      $a0, 0x64  # 'd'                                         ; 64 00 04 34
8001D21C: li      $a1, 0xB4                                                ; B4 00 05 34
8001D220: li      $a2, dword_8004EA90                                      ; 05 80 06 3C 90 EA C6 24
8001D228: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D22C: move    $a3, $zero                                               ; 21 38 00 00
8001D230: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D234: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D238: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D23C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D240: sll     $v0, 2                                                   ; 80 10 02 00
8001D244: addu    $v0, $s0                                                 ; 21 10 50 00
8001D248: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D24C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D250: li      $a0, 0x78  # 'x'                                         ; 78 00 04 34
8001D254: li      $a1, 0xB4                                                ; B4 00 05 34
8001D258: li      $a2, dword_8004EAA0                                      ; 05 80 06 3C A0 EA C6 24
8001D260: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D264: move    $a3, $zero                                               ; 21 38 00 00
8001D268: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D26C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D270: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D274: addu    $v0, $v1                                                 ; 21 10 43 00
8001D278: sll     $v0, 2                                                   ; 80 10 02 00
8001D27C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D280: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D284: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D288: li      $a0, 0x8C                                                ; 8C 00 04 34
8001D28C: li      $a1, 0xB4                                                ; B4 00 05 34
8001D290: li      $a2, dword_8004EAB0                                      ; 05 80 06 3C B0 EA C6 24
8001D298: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D29C: move    $a3, $zero                                               ; 21 38 00 00
8001D2A0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D2A4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D2A8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D2AC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D2B0: sll     $v0, 2                                                   ; 80 10 02 00
8001D2B4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D2B8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D2BC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D2C0: li      $a0, 0xA0                                                ; A0 00 04 34
8001D2C4: li      $a1, 0xB4                                                ; B4 00 05 34
8001D2C8: li      $a2, dword_8004EAC0                                      ; 05 80 06 3C C0 EA C6 24
8001D2D0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D2D4: move    $a3, $zero                                               ; 21 38 00 00
8001D2D8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D2DC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D2E0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D2E4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D2E8: sll     $v0, 2                                                   ; 80 10 02 00
8001D2EC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D2F0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D2F4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D2F8: li      $a0, 0xB4                                                ; B4 00 04 34
8001D2FC: li      $a1, 0xB4                                                ; B4 00 05 34
8001D300: li      $a2, dword_8004EAD0                                      ; 05 80 06 3C D0 EA C6 24
8001D308: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D30C: move    $a3, $zero                                               ; 21 38 00 00
8001D310: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D314: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D318: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D31C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D320: sll     $v0, 2                                                   ; 80 10 02 00
8001D324: addu    $v0, $s0                                                 ; 21 10 50 00
8001D328: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D32C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D330: li      $a0, 0xC8                                                ; C8 00 04 34
8001D334: li      $a1, 0xB4                                                ; B4 00 05 34
8001D338: li      $a2, dword_8004EAE0                                      ; 05 80 06 3C E0 EA C6 24
8001D340: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D344: move    $a3, $zero                                               ; 21 38 00 00
8001D348: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D34C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D350: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D354: addu    $v0, $v1                                                 ; 21 10 43 00
8001D358: sll     $v0, 2                                                   ; 80 10 02 00
8001D35C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D360: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D364: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D368: li      $a0, 0xDC                                                ; DC 00 04 34
8001D36C: li      $a1, 0xB4                                                ; B4 00 05 34
8001D370: li      $a2, dword_8004EAF0                                      ; 05 80 06 3C F0 EA C6 24
8001D378: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D37C: move    $a3, $zero                                               ; 21 38 00 00
8001D380: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D384: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D388: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D38C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D390: sll     $v0, 2                                                   ; 80 10 02 00
8001D394: addu    $v0, $s0                                                 ; 21 10 50 00
8001D398: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D39C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D3A0: li      $a0, 0xF0                                                ; F0 00 04 34
8001D3A4: li      $a1, 0xB4                                                ; B4 00 05 34
8001D3A8: li      $a2, dword_8004EB00                                      ; 05 80 06 3C 00 EB C6 24
8001D3B0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D3B4: move    $a3, $zero                                               ; 21 38 00 00
8001D3B8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D3BC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D3C0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D3C4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D3C8: sll     $v0, 2                                                   ; 80 10 02 00
8001D3CC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D3D0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D3D4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D3D8: li      $a0, 0x104                                               ; 04 01 04 34
8001D3DC: li      $a1, 0xB4                                                ; B4 00 05 34
8001D3E0: li      $a2, dword_8004EB10                                      ; 05 80 06 3C 10 EB C6 24
8001D3E8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D3EC: move    $a3, $zero                                               ; 21 38 00 00
8001D3F0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D3F4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D3F8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D3FC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D400: sll     $v0, 2                                                   ; 80 10 02 00
8001D404: addu    $v0, $s0                                                 ; 21 10 50 00
8001D408: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D40C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D410: li      $a0, 0x118                                               ; 18 01 04 34
8001D414: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D418: li      $a2, dword_8004E980                                      ; 05 80 06 3C 80 E9 C6 24
8001D420: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D424: move    $a3, $zero                                               ; 21 38 00 00
8001D428: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D42C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D430: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D434: addu    $v0, $v1                                                 ; 21 10 43 00
8001D438: sll     $v0, 2                                                   ; 80 10 02 00
8001D43C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D440: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D444: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D448: li      $a0, 0x14                                                ; 14 00 04 34
8001D44C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D450: li      $a2, dword_8004EA50                                      ; 05 80 06 3C 50 EA C6 24
8001D458: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D45C: move    $a3, $zero                                               ; 21 38 00 00
8001D460: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D464: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D468: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D46C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D470: sll     $v0, 2                                                   ; 80 10 02 00
8001D474: addu    $v0, $s0                                                 ; 21 10 50 00
8001D478: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D47C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D480: li      $a0, 0xA0                                                ; A0 00 04 34
8001D484: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D488: li      $a2, dword_8004E9F0                                      ; 05 80 06 3C F0 E9 C6 24
8001D490: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D494: move    $a3, $zero                                               ; 21 38 00 00
8001D498: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D49C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D4A0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D4A4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D4A8: sll     $v0, 2                                                   ; 80 10 02 00
8001D4AC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D4B0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D4B4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D4B8: li      $a0, 0x8C                                                ; 8C 00 04 34
8001D4BC: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D4C0: li      $a2, dword_8004E9E0                                      ; 05 80 06 3C E0 E9 C6 24
8001D4C8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D4CC: move    $a3, $zero                                               ; 21 38 00 00
8001D4D0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D4D4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D4D8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D4DC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D4E0: sll     $v0, 2                                                   ; 80 10 02 00
8001D4E4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D4E8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D4EC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D4F0: li      $a0, 0xB4                                                ; B4 00 04 34
8001D4F4: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D4F8: li      $a2, dword_8004EA00                                      ; 05 80 06 3C 00 EA C6 24
8001D500: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D504: move    $a3, $zero                                               ; 21 38 00 00
8001D508: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D50C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D510: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D514: addu    $v0, $v1                                                 ; 21 10 43 00
8001D518: sll     $v0, 2                                                   ; 80 10 02 00
8001D51C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D520: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D524: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D528: li      $a0, 0x78  # 'x'                                         ; 78 00 04 34
8001D52C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D530: li      $a2, dword_8004E9D0                                      ; 05 80 06 3C D0 E9 C6 24
8001D538: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D53C: move    $a3, $zero                                               ; 21 38 00 00
8001D540: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D544: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D548: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D54C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D550: sll     $v0, 2                                                   ; 80 10 02 00
8001D554: addu    $v0, $s0                                                 ; 21 10 50 00
8001D558: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D55C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D560: li      $a0, 0xC8                                                ; C8 00 04 34
8001D564: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D568: li      $a2, dword_8004EA10                                      ; 05 80 06 3C 10 EA C6 24
8001D570: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D574: move    $a3, $zero                                               ; 21 38 00 00
8001D578: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D57C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D580: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D584: addu    $v0, $v1                                                 ; 21 10 43 00
8001D588: sll     $v0, 2                                                   ; 80 10 02 00
8001D58C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D590: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D594: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D598: li      $a0, 0x64  # 'd'                                         ; 64 00 04 34
8001D59C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D5A0: li      $a2, dword_8004E9C0                                      ; 05 80 06 3C C0 E9 C6 24
8001D5A8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D5AC: move    $a3, $zero                                               ; 21 38 00 00
8001D5B0: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D5B4: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D5B8: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D5BC: addu    $v0, $v1                                                 ; 21 10 43 00
8001D5C0: sll     $v0, 2                                                   ; 80 10 02 00
8001D5C4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D5C8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D5CC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D5D0: li      $a0, 0xF0                                                ; F0 00 04 34
8001D5D4: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D5D8: li      $a2, dword_8004EA30                                      ; 05 80 06 3C 30 EA C6 24
8001D5E0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D5E4: move    $a3, $zero                                               ; 21 38 00 00
8001D5E8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D5EC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D5F0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D5F4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D5F8: sll     $v0, 2                                                   ; 80 10 02 00
8001D5FC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D600: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D604: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D608: li      $a0, 0x50  # 'P'                                         ; 50 00 04 34
8001D60C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D610: li      $a2, dword_8004E9B0                                      ; 05 80 06 3C B0 E9 C6 24
8001D618: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D61C: move    $a3, $zero                                               ; 21 38 00 00
8001D620: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D624: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D628: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D62C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D630: sll     $v0, 2                                                   ; 80 10 02 00
8001D634: addu    $v0, $s0                                                 ; 21 10 50 00
8001D638: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D63C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D640: li      $a0, 0xDC                                                ; DC 00 04 34
8001D644: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D648: li      $a2, dword_8004EA20                                      ; 05 80 06 3C 20 EA C6 24
8001D650: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D654: move    $a3, $zero                                               ; 21 38 00 00
8001D658: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D65C: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D660: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D664: addu    $v0, $v1                                                 ; 21 10 43 00
8001D668: sll     $v0, 2                                                   ; 80 10 02 00
8001D66C: addu    $v0, $s0                                                 ; 21 10 50 00
8001D670: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D674: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D678: li      $a0, 0x3C  # '<'                                         ; 3C 00 04 34
8001D67C: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D680: li      $a2, dword_8004E9A0                                      ; 05 80 06 3C A0 E9 C6 24
8001D688: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D68C: move    $a3, $zero                                               ; 21 38 00 00
8001D690: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D694: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D698: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D69C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D6A0: sll     $v0, 2                                                   ; 80 10 02 00
8001D6A4: addu    $v0, $s0                                                 ; 21 10 50 00
8001D6A8: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D6AC: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D6B0: li      $a0, 0x104                                               ; 04 01 04 34
8001D6B4: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D6B8: li      $a2, dword_8004EA40                                      ; 05 80 06 3C 40 EA C6 24
8001D6C0: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D6C4: move    $a3, $zero                                               ; 21 38 00 00
8001D6C8: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D6CC: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D6D0: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D6D4: addu    $v0, $v1                                                 ; 21 10 43 00
8001D6D8: sll     $v0, 2                                                   ; 80 10 02 00
8001D6DC: addu    $v0, $s0                                                 ; 21 10 50 00
8001D6E0: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D6E4: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D6E8: li      $a0, 0x28  # '('                                         ; 28 00 04 34
8001D6EC: li      $a1, 0x28  # '('                                         ; 28 00 05 34
8001D6F0: li      $a2, dword_8004E990                                      ; 05 80 06 3C 90 E9 C6 24
8001D6F8: lw      $v1, 0x368($gp)                                          ; 68 03 83 8F
8001D6FC: move    $a3, $zero                                               ; 21 38 00 00
8001D700: sw      $zero, 0x20+var_10($sp)                                  ; 10 00 A0 AF
8001D704: sw      $s2, 0x20+var_C($sp)                                     ; 14 00 B2 AF
8001D708: sll     $v0, $v1, 2                                              ; 80 10 03 00
8001D70C: addu    $v0, $v1                                                 ; 21 10 43 00
8001D710: sll     $v0, 2                                                   ; 80 10 02 00
8001D714: addu    $v0, $s0                                                 ; 21 10 50 00
8001D718: jal     sub_8001B590                                             ; 64 6D 00 0C
8001D71C: sw      $v0, 0x20+var_8($sp)                                     ; 18 00 A2 AF
8001D720: lw      $ra, 0x20+var_s1C($sp)                                   ; 3C 00 BF 8F
8001D724: lw      $s6, 0x20+var_s18($sp)                                   ; 38 00 B6 8F
8001D728: lw      $s5, 0x20+var_s14($sp)                                   ; 34 00 B5 8F
8001D72C: lw      $s4, 0x20+var_s10($sp)                                   ; 30 00 B4 8F
8001D730: lw      $s3, 0x20+var_sC($sp)                                    ; 2C 00 B3 8F
8001D734: lw      $s2, 0x20+var_s8($sp)                                    ; 28 00 B2 8F
8001D738: lw      $s1, 0x20+var_s4($sp)                                    ; 24 00 B1 8F
8001D73C: lw      $s0, 0x20+var_s0($sp)                                    ; 20 00 B0 8F
8001D740: addiu   $sp, 0x40                                                ; 40 00 BD 27
8001D744: jr      $ra                                                      ; 08 00 E0 03
8001D748: nop                                                              ; 00 00 00 00
```

## `80035510` `sub_80035510`
Bounds: `80035510-80035540`

### Pseudocode
```c
int sub_80035510()
{
  PAD_dr();
  return ~dword_800882F0;
}

```

### Calls in function
- 80035518: `jal     PAD_dr` -> 80048A00 PAD_dr

### Xrefs to
- 800157DC -> 80035510 type=17 func=sub_80015788 line=`jal     sub_80035510`
- 80015B9C -> 80035510 type=17 func=sub_80015B00 line=`jal     sub_80035510`
- 80016AF0 -> 80035510 type=17 func=sub_80016AB4 line=`jal     sub_80035510`
- 8001900C -> 80035510 type=17 func=sub_80018FB0 line=`jal     sub_80035510`
- 8002674C -> 80035510 type=17 func=sub_80026744 line=`jal     sub_80035510`
- 80026CA8 -> 80035510 type=17 func=sub_80026B94 line=`jal     sub_80035510`
- 80027AB0 -> 80035510 type=17 func=sub_8002776C line=`jal     sub_80035510`
- 80027EAC -> 80035510 type=17 func=sub_8002776C line=`jal     sub_80035510`
- 801C467C -> 80035510 type=17 func=sub_801C455C line=`jal     sub_80035510`
- 801C4B50 -> 80035510 type=17 func=sub_801C4894 line=`jal     sub_80035510`
- 801C4BC0 -> 80035510 type=17 func=sub_801C4894 line=`jal     sub_80035510`
- 801C4C58 -> 80035510 type=17 func=sub_801C4894 line=`jal     sub_80035510`
- 801C4CD4 -> 80035510 type=17 func=sub_801C4894 line=`jal     sub_80035510`

### Xrefs from
- 80035510 -> 80035514 type=21 target=sub_80035514 line=`addiu   $sp, -0x18`
- 80035514 -> 80035518 type=21 target=sub_80035518 line=`sw      $ra, 0x10+var_s0($sp)`
- 80035518 -> 8003551C type=21 target=sub_8003551C line=`jal     PAD_dr`
- 80035518 -> 80048A00 type=17 target=PAD_dr line=`jal     PAD_dr`
- 8003551C -> 80035520 type=21 target=sub_80035520 line=`nop`
- 80035520 -> 80035528 type=21 target=sub_80035528 line=`lw      $v0, dword_800882F0`
- 80035528 -> 8003552C type=21 target=sub_8003552C line=`nop`
- 8003552C -> 80035530 type=21 target=sub_80035530 line=`nor     $v0, $zero, $v0`
- 80035530 -> 80035534 type=21 target=sub_80035534 line=`lw      $ra, 0x10+var_s0($sp)`
- 80035534 -> 80035538 type=21 target=sub_80035538 line=`addiu   $sp, 0x18`
- 80035538 -> 8003553C type=21 target=sub_8003553C line=`jr      $ra`

### Full disasm
```asm
80035510: addiu   $sp, -0x18                                               ; E8 FF BD 27
80035514: sw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF AF
80035518: jal     PAD_dr                                                   ; 80 22 01 0C
8003551C: nop                                                              ; 00 00 00 00
80035520: lw      $v0, dword_800882F0                                      ; 09 80 02 3C F0 82 42 8C
80035528: nop                                                              ; 00 00 00 00
8003552C: nor     $v0, $zero, $v0                                          ; 27 10 02 00
80035530: lw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF 8F
80035534: addiu   $sp, 0x18                                                ; 18 00 BD 27
80035538: jr      $ra                                                      ; 08 00 E0 03
8003553C: nop                                                              ; 00 00 00 00
```

## `80040CA4` `sub_80040CA4`
Bounds: `80040CA4-80040CC8`

### Pseudocode
```c
int __fastcall sub_80040CA4(int a1)
{
  return sub_800450A0(*(_DWORD *)(a1 + 16));
}

```

### Calls in function
- 80040CB0: `jal     sub_800450A0` -> 800450A0 sub_800450A0

### Xrefs to
- 8001E3CC -> 80040CA4 type=17 func=sub_8001E3B0 line=`jal     sub_80040CA4`
- 8001EA58 -> 80040CA4 type=17 func=sub_8001EA00 line=`jal     sub_80040CA4`
- 8001EC3C -> 80040CA4 type=17 func=sub_8001EBF4 line=`jal     sub_80040CA4`
- 8001ED5C -> 80040CA4 type=17 func=sub_8001ED3C line=`jal     sub_80040CA4`
- 8001EE94 -> 80040CA4 type=17 func=sub_8001EE74 line=`jal     sub_80040CA4`
- 801C68F4 -> 80040CA4 type=17 func=sub_801C689C line=`jal     sub_80040CA4`

### Xrefs from
- 80040CA4 -> 80040CA8 type=21 target=sub_80040CA8 line=`addiu   $sp, -0x18`
- 80040CA8 -> 80040CAC type=21 target=sub_80040CAC line=`sw      $ra, 0x10+var_s0($sp)`
- 80040CAC -> 80040CB0 type=21 target=sub_80040CB0 line=`lw      $a0, 0x10($a0)`
- 80040CB0 -> 80040CB4 type=21 target=sub_80040CB4 line=`jal     sub_800450A0`
- 80040CB0 -> 800450A0 type=17 target=sub_800450A0 line=`jal     sub_800450A0`
- 80040CB4 -> 80040CB8 type=21 target=sub_80040CB8 line=`nop`
- 80040CB8 -> 80040CBC type=21 target=sub_80040CBC line=`lw      $ra, 0x10+var_s0($sp)`
- 80040CBC -> 80040CC0 type=21 target=sub_80040CC0 line=`addiu   $sp, 0x18`
- 80040CC0 -> 80040CC4 type=21 target=sub_80040CC4 line=`jr      $ra`

### Full disasm
```asm
80040CA4: addiu   $sp, -0x18                                               ; E8 FF BD 27
80040CA8: sw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF AF
80040CAC: lw      $a0, 0x10($a0)                                           ; 10 00 84 8C
80040CB0: jal     sub_800450A0                                             ; 28 14 01 0C
80040CB4: nop                                                              ; 00 00 00 00
80040CB8: lw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF 8F
80040CBC: addiu   $sp, 0x18                                                ; 18 00 BD 27
80040CC0: jr      $ra                                                      ; 08 00 E0 03
80040CC4: nop                                                              ; 00 00 00 00
```

## `80040370` `sub_80040370`
Bounds: `80040370-80040420`

### Pseudocode
```c
int sub_80040370()
{
  int v0; // $v0
  int v1; // $v0
  int v2; // $v1
  int v3; // $a0

  v0 = word_80096590;
  word_80091790 = word_8008ECA8[v0];
  word_80091792 = word_8008ECAC[v0];
  sub_800452EC();
  sub_80044AA0(1);
  v1 = dword_8009658C;
  v2 = ++dword_8009658C;
  v3 = 1;
  if ( v1 )
    v3 = v2;
  dword_8009658C = v3;
  word_80096590 = word_80096590 == 0;
  sub_800402E0();
  return sub_800401AC();
}

```

### Calls in function
- 800403B0: `jal     sub_800452EC` -> 800452EC sub_800452EC
- 800403B8: `jal     sub_80044AA0` -> 80044AA0 sub_80044AA0
- 800403D8: `beqz    $v0, loc_800403E4` -> 800403E4 loc_800403E4
- 80040400: `jal     sub_800402E0` -> 800402E0 sub_800402E0
- 80040408: `jal     sub_800401AC` -> 800401AC sub_800401AC

### Xrefs to
- 8001E554 -> 80040370 type=17 func=sub_8001E54C line=`jal     sub_80040370`
- 8001EA0C -> 80040370 type=17 func=sub_8001EA00 line=`jal     sub_80040370`
- 8001EBFC -> 80040370 type=17 func=sub_8001EBF4 line=`jal     sub_80040370`
- 8001ED7C -> 80040370 type=17 func=sub_8001ED74 line=`jal     sub_80040370`
- 801C4B7C -> 80040370 type=17 func=sub_801C4894 line=`jal     sub_80040370`
- 801C4B8C -> 80040370 type=17 func=sub_801C4894 line=`jal     sub_80040370`
- 801C68A4 -> 80040370 type=17 func=sub_801C689C line=`jal     sub_80040370`
- 801C6920 -> 80040370 type=17 func=sub_801C689C line=`jal     sub_80040370`

### Xrefs from
- 80040370 -> 80040378 type=21 target=sub_80040378 line=`lh      $v0, word_80096590`
- 80040378 -> 8004037C type=21 target=sub_8004037C line=`addiu   $sp, -0x18`
- 8004037C -> 80040380 type=21 target=sub_80040380 line=`sw      $ra, 0x10+var_s0($sp)`
- 80040380 -> 80040384 type=21 target=sub_80040384 line=`sll     $v0, 1`
- 80040384 -> 80040390 type=21 target=sub_80040390 line=`lhu     $v1, word_8008ECA8($v0)`
- 80040390 -> 80040398 type=21 target=sub_80040398 line=`li      $a0, word_80091790`
- 80040398 -> 8004039C type=21 target=sub_8004039C line=`sh      $v1, (word_80091790 - 0x80091790)($a0)`
- 8004039C -> 800403A8 type=21 target=sub_800403A8 line=`lhu     $v0, word_8008ECAC($v0)`
- 800403A8 -> 800403B0 type=21 target=sub_800403B0 line=`sh      $v0, word_80091792`
- 800403B0 -> 800403B4 type=21 target=sub_800403B4 line=`jal     sub_800452EC`
- 800403B0 -> 800452EC type=17 target=sub_800452EC line=`jal     sub_800452EC`
- 800403B4 -> 800403B8 type=21 target=sub_800403B8 line=`nop`
- 800403B8 -> 800403BC type=21 target=sub_800403BC line=`jal     sub_80044AA0`
- 800403B8 -> 80044AA0 type=17 target=sub_80044AA0 line=`jal     sub_80044AA0`
- 800403BC -> 800403C0 type=21 target=sub_800403C0 line=`li      $a0, 1`
- 800403C0 -> 800403C8 type=21 target=sub_800403C8 line=`lw      $v0, dword_8009658C`
- 800403C8 -> 800403CC type=21 target=sub_800403CC line=`nop`
- 800403CC -> 800403D0 type=21 target=sub_800403D0 line=`addiu   $v1, $v0, 1`
- 800403D0 -> 800403D8 type=21 target=sub_800403D8 line=`sw      $v1, dword_8009658C`
- 800403D8 -> 800403DC type=21 target=sub_800403DC line=`beqz    $v0, loc_800403E4`
- 800403D8 -> 800403E4 type=19 target=loc_800403E4 line=`beqz    $v0, loc_800403E4`
- 800403DC -> 800403E0 type=21 target=sub_800403E0 line=`li      $a0, 1`
- 800403E0 -> 800403E4 type=21 target=loc_800403E4 line=`move    $a0, $v1`
- 800403E4 -> 800403EC type=21 target=sub_800403EC line=`lh      $v0, word_80096590`
- 800403EC -> 800403F4 type=21 target=sub_800403F4 line=`sw      $a0, dword_8009658C`
- 800403F4 -> 800403F8 type=21 target=sub_800403F8 line=`sltiu   $v0, 1`
- 800403F8 -> 80040400 type=21 target=sub_80040400 line=`sh      $v0, word_80096590`
- 80040400 -> 80040404 type=21 target=sub_80040404 line=`jal     sub_800402E0`
- 80040400 -> 800402E0 type=17 target=sub_800402E0 line=`jal     sub_800402E0`
- 80040404 -> 80040408 type=21 target=sub_80040408 line=`nop`
- 80040408 -> 8004040C type=21 target=sub_8004040C line=`jal     sub_800401AC`
- 80040408 -> 800401AC type=17 target=sub_800401AC line=`jal     sub_800401AC`
- 8004040C -> 80040410 type=21 target=sub_80040410 line=`nop`
- 80040410 -> 80040414 type=21 target=sub_80040414 line=`lw      $ra, 0x10+var_s0($sp)`
- 80040414 -> 80040418 type=21 target=sub_80040418 line=`addiu   $sp, 0x18`
- 80040418 -> 8004041C type=21 target=sub_8004041C line=`jr      $ra`

### Full disasm
```asm
80040370: lh      $v0, word_80096590                                       ; 09 80 02 3C 90 65 42 84
80040378: addiu   $sp, -0x18                                               ; E8 FF BD 27
8004037C: sw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF AF
80040380: sll     $v0, 1                                                   ; 40 10 02 00
80040384: lhu     $v1, word_8008ECA8($v0)                                  ; 09 80 01 3C 21 08 22 00 A8 EC 23 94
80040390: li      $a0, word_80091790                                       ; 09 80 04 3C 90 17 84 24
80040398: sh      $v1, (word_80091790 - 0x80091790)($a0)                   ; 00 00 83 A4
8004039C: lhu     $v0, word_8008ECAC($v0)                                  ; 09 80 01 3C 21 08 22 00 AC EC 22 94
800403A8: sh      $v0, word_80091792                                       ; 09 80 01 3C 92 17 22 A4
800403B0: jal     sub_800452EC                                             ; BB 14 01 0C
800403B4: nop                                                              ; 00 00 00 00
800403B8: jal     sub_80044AA0                                             ; A8 12 01 0C
800403BC: li      $a0, 1                                                   ; 01 00 04 34
800403C0: lw      $v0, dword_8009658C                                      ; 09 80 02 3C 8C 65 42 8C
800403C8: nop                                                              ; 00 00 00 00
800403CC: addiu   $v1, $v0, 1                                              ; 01 00 43 24
800403D0: sw      $v1, dword_8009658C                                      ; 09 80 01 3C 8C 65 23 AC
800403D8: beqz    $v0, loc_800403E4                                        ; 02 00 40 10
800403DC: li      $a0, 1                                                   ; 01 00 04 34
800403E0: move    $a0, $v1                                                 ; 21 20 60 00
800403E4: lh      $v0, word_80096590                                       ; 09 80 02 3C 90 65 42 84
800403EC: sw      $a0, dword_8009658C                                      ; 09 80 01 3C 8C 65 24 AC
800403F4: sltiu   $v0, 1                                                   ; 01 00 42 2C
800403F8: sh      $v0, word_80096590                                       ; 09 80 01 3C 90 65 22 A4
80040400: jal     sub_800402E0                                             ; B8 00 01 0C
80040404: nop                                                              ; 00 00 00 00
80040408: jal     sub_800401AC                                             ; 6B 00 01 0C
8004040C: nop                                                              ; 00 00 00 00
80040410: lw      $ra, 0x10+var_s0($sp)                                    ; 10 00 BF 8F
80040414: addiu   $sp, 0x18                                                ; 18 00 BD 27
80040418: jr      $ra                                                      ; 08 00 E0 03
8004041C: nop                                                              ; 00 00 00 00
```



[result]
null
