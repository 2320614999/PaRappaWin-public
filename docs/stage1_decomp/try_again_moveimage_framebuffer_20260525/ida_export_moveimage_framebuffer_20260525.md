# Stage1 Try Again MoveImage framebuffer evidence 2026-05-25

IDA input: E:\game\PS模拟器\parappa the rapper\SCUS_941.83

## 0x800468E0 sub_800468E0

Bounds: 0x800468E0..0x80046BC4

### Hex-Rays pseudo-C
```c
// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_800468E0(void (__fastcall *a1)(int, int), int a2, int a3, int a4)
{
  int v7; // $v0

  sub_80047144();
  v7 = SetIntrMask(0);
  dword_8005D73C = 1;
  dword_8005D840 = v7;
  while ( (MEMORY[0x1F801814] & 0x4000000) == 0 )
    ;
  a1(a2, a4);
  dword_8005D828 = (int)a1;
  dword_8005D82C = a2;
  dword_8005D830 = a4;
  SetIntrMask(0);
  return 0;
}

```

### Disassembly
```asm
0x800468E0: addiu   $sp, -0x28
0x800468E4: sw      $s3, 0x10+var_sC($sp)
0x800468E8: move    $s3, $a0
0x800468EC: sw      $s0, 0x10+var_s0($sp)
0x800468F0: move    $s0, $a1
0x800468F4: sw      $s1, 0x10+var_s4($sp)
0x800468F8: move    $s1, $a2
0x800468FC: sw      $s2, 0x10+var_s8($sp)
0x80046900: sw      $ra, 0x10+var_s10($sp)
0x80046904: jal     sub_80047144
0x80046908: move    $s2, $a3
0x8004690C: j       loc_8004692C
0x80046910: nop
0x80046914: jal     sub_80047178
0x80046918: nop
0x8004691C: bnez    $v0, loc_80046BA4
0x80046920: li      $v0, 0xFFFFFFFF
0x80046924: jal     sub_80046BC4
0x80046928: nop
0x8004692C: lw      $v0, dword_8005D838
0x80046934: lw      $v1, dword_8005D83C
0x8004693C: addiu   $v0, 1
0x80046940: andi    $v0, 0x3F
0x80046944: beq     $v0, $v1, loc_80046914
0x80046948: nop
0x8004694C: jal     SetIntrMask
0x80046950: move    $a0, $zero
0x80046954: li      $v1, dword_8005D73C
0x8004695C: li      $a0, 1
0x80046960: sw      $a0, (dword_8005D73C - 0x8005D73C)($v1)
0x80046964: lbu     $v1, byte_8005D735
0x8004696C: sw      $v0, dword_8005D840
0x80046974: beqz    $v1, loc_800469CC
0x80046978: lui     $a0, 0x400
0x8004697C: lw      $v1, dword_8005D838
0x80046984: lw      $v0, dword_8005D83C
0x8004698C: nop
0x80046990: bne     $v1, $v0, loc_80046A2C
0x80046994: nop
0x80046998: lw      $v0, dword_8005D814
0x800469A0: nop
0x800469A4: lw      $v0, 0($v0)
0x800469A8: lui     $v1, 0x100
0x800469AC: and     $v0, $v1
0x800469B0: bnez    $v0, loc_80046A2C
0x800469B4: nop
0x800469B8: lw      $v0, dword_8005D740
0x800469C0: nop
0x800469C4: bnez    $v0, loc_80046A2C
0x800469C8: nop
0x800469CC: lw      $v1, dword_8005D808
0x800469D4: nop
0x800469D8: lw      $v0, 0($v1)
0x800469DC: nop
0x800469E0: and     $v0, $a0
0x800469E4: beqz    $v0, loc_800469D4
0x800469E8: nop
0x800469EC: move    $a0, $s0
0x800469F0: jalr    $s3
0x800469F4: move    $a1, $s2
0x800469F8: lw      $a0, dword_8005D840
0x80046A00: li      $v0, dword_8005D828
0x80046A08: sw      $s3, (dword_8005D828 - 0x8005D828)($v0)
0x80046A0C: sw      $s0, dword_8005D82C
0x80046A14: sw      $s2, dword_8005D830
0x80046A1C: jal     SetIntrMask
0x80046A20: nop
0x80046A24: j       loc_80046BA4
0x80046A28: move    $v0, $zero
0x80046A2C: li      $a1, sub_80046BC4
0x80046A34: jal     DMACallback
0x80046A38: li      $a0, 2
0x80046A3C: beqz    $s1, loc_80046AEC
0x80046A40: move    $a2, $zero
0x80046A44: li      $t0, unk_80094454
0x80046A4C: move    $a3, $s0
0x80046A50: move    $v0, $s1
0x80046A54: bgez    $v0, loc_80046A60
0x80046A58: nop
0x80046A5C: addiu   $v0, 3
0x80046A60: sra     $v0, 2
0x80046A64: slt     $v0, $a2, $v0
0x80046A68: beqz    $v0, loc_80046AA4
0x80046A6C: sll     $a0, $a2, 2
0x80046A70: lw      $a1, 0($a3)
0x80046A74: addiu   $a3, 4
0x80046A78: lw      $v1, dword_8005D838
0x80046A80: addiu   $a2, 1
0x80046A84: sll     $v0, $v1, 1
0x80046A88: addu    $v0, $v1
0x80046A8C: sll     $v0, 5
0x80046A90: addu    $v0, $t0
0x80046A94: addu    $a0, $v0
0x80046A98: sw      $a1, 0($a0)
0x80046A9C: j       loc_80046A54
0x80046AA0: move    $v0, $s1
0x80046AA4: lw      $v0, dword_8005D838
0x80046AAC: lw      $v1, dword_8005D838
0x80046AB4: sll     $a0, $v0, 1
0x80046AB8: addu    $a0, $v0
0x80046ABC: sll     $a0, 5
0x80046AC0: sll     $v0, $v1, 1
0x80046AC4: addu    $v0, $v1
0x80046AC8: sll     $v0, 5
0x80046ACC: li      $v1, unk_80094454
0x80046AD4: addu    $v0, $v1
0x80046AD8: sw      $v0, dword_8009444C($a0)
0x80046AE4: j       loc_80046B10
0x80046AE8: nop
0x80046AEC: lw      $v1, dword_8005D838
0x80046AF4: nop
0x80046AF8: sll     $v0, $v1, 1
0x80046AFC: addu    $v0, $v1
0x80046B00: sll     $v0, 5
0x80046B04: sw      $s0, dword_8009444C($v0)
0x80046B10: lw      $v1, dword_8005D838
0x80046B18: nop
0x80046B1C: sll     $v0, $v1, 1
0x80046B20: addu    $v0, $v1
0x80046B24: sll     $v0, 5
0x80046B28: sw      $s2, dword_80094450($v0)
0x80046B34: lw      $v1, dword_8005D838
0x80046B3C: nop
0x80046B40: sll     $v0, $v1, 1
0x80046B44: addu    $v0, $v1
0x80046B48: sll     $v0, 5
0x80046B4C: sw      $s3, dword_80094448($v0)
0x80046B58: lw      $v0, dword_8005D838
0x80046B60: lw      $a0, dword_8005D840
0x80046B68: addiu   $v0, 1
0x80046B6C: andi    $v0, 0x3F
0x80046B70: sw      $v0, dword_8005D838
0x80046B78: jal     SetIntrMask
0x80046B7C: nop
0x80046B80: jal     sub_80046BC4
0x80046B84: nop
0x80046B88: lw      $v0, dword_8005D838
0x80046B90: lw      $v1, dword_8005D83C
0x80046B98: nop
0x80046B9C: subu    $v0, $v1
0x80046BA0: andi    $v0, 0x3F
0x80046BA4: lw      $ra, 0x10+var_s10($sp)
0x80046BA8: lw      $s3, 0x10+var_sC($sp)
0x80046BAC: lw      $s2, 0x10+var_s8($sp)
0x80046BB0: lw      $s1, 0x10+var_s4($sp)
0x80046BB4: lw      $s0, 0x10+var_s0($sp)
0x80046BB8: addiu   $sp, 0x28
0x80046BBC: jr      $ra
0x80046BC0: nop
```

### Xrefs to
```text
0x80044D3C -> 0x800468E0 type=17 from_func=sub_80044CD0
0x80044DA8 -> 0x800468E0 type=17 from_func=sub_80044D64
0x80044E0C -> 0x800468E0 type=17 from_func=sub_80044DC8
0x80044ECC -> 0x800468E0 type=17 from_func=sub_80044E2C
0x800450F8 -> 0x800468E0 type=17 from_func=sub_800450A0
0x800451A0 -> 0x800468E0 type=17 from_func=sub_80045114
0x8004527C -> 0x800468E0 type=17 from_func=sub_800451D8
0x800468C8 -> 0x800468E0 type=17 from_func=sub_800468BC
0x8005D6F4 -> 0x800468E0 type=1 from_func=<none>
```

### Xrefs from
```text
0x80046904 -> 0x80047144 type=17 target=sub_80047144
0x80046914 -> 0x80047178 type=17 target=sub_80047178
0x80046924 -> 0x80046BC4 type=17 target=sub_80046BC4
0x8004692C -> 0x8005D838 type=3 target=dword_8005D838
0x80046934 -> 0x8005D83C type=3 target=dword_8005D83C
0x8004694C -> 0x800358C0 type=17 target=SetIntrMask
0x80046954 -> 0x8005D73C type=1 target=dword_8005D73C
0x80046960 -> 0x8005D73C type=2 target=dword_8005D73C
0x80046964 -> 0x8005D735 type=3 target=byte_8005D735
0x8004696C -> 0x8005D840 type=2 target=dword_8005D840
0x8004697C -> 0x8005D838 type=3 target=dword_8005D838
0x80046984 -> 0x8005D83C type=3 target=dword_8005D83C
0x80046998 -> 0x8005D814 type=3 target=dword_8005D814
0x800469A4 -> 0x1F8010A8 type=3 target=sub_1F8010A8
0x800469B8 -> 0x8005D740 type=3 target=dword_8005D740
0x800469CC -> 0x8005D808 type=3 target=dword_8005D808
0x800469D8 -> 0x1F801814 type=3 target=sub_1F801814
0x800469F8 -> 0x8005D840 type=3 target=dword_8005D840
0x80046A00 -> 0x8005D828 type=1 target=dword_8005D828
0x80046A08 -> 0x8005D828 type=2 target=dword_8005D828
0x80046A0C -> 0x8005D82C type=2 target=dword_8005D82C
0x80046A14 -> 0x8005D830 type=2 target=dword_8005D830
0x80046A1C -> 0x800358C0 type=17 target=SetIntrMask
0x80046A2C -> 0x80046BC4 type=1 target=sub_80046BC4
0x80046A34 -> 0x800357A4 type=17 target=DMACallback
0x80046A44 -> 0x80094454 type=1 target=unk_80094454
0x80046A78 -> 0x8005D838 type=3 target=dword_8005D838
0x80046AA4 -> 0x8005D838 type=3 target=dword_8005D838
0x80046AAC -> 0x8005D838 type=3 target=dword_8005D838
0x80046ACC -> 0x80094454 type=1 target=unk_80094454
0x80046AD8 -> 0x8009444C type=2 target=dword_8009444C
0x80046AEC -> 0x8005D838 type=3 target=dword_8005D838
0x80046B04 -> 0x8009444C type=2 target=dword_8009444C
0x80046B10 -> 0x8005D838 type=3 target=dword_8005D838
0x80046B28 -> 0x80094450 type=2 target=dword_80094450
0x80046B34 -> 0x8005D838 type=3 target=dword_8005D838
0x80046B4C -> 0x80094448 type=2 target=dword_80094448
0x80046B58 -> 0x8005D838 type=3 target=dword_8005D838
0x80046B60 -> 0x8005D840 type=3 target=dword_8005D840
0x80046B70 -> 0x8005D838 type=2 target=dword_8005D838
0x80046B78 -> 0x800358C0 type=17 target=SetIntrMask
0x80046B80 -> 0x80046BC4 type=17 target=sub_80046BC4
0x80046B88 -> 0x8005D838 type=3 target=dword_8005D838
0x80046B90 -> 0x8005D83C type=3 target=dword_8005D83C
```

### Referenced globals / immediates
```text
0x80046904 op0 -> 0x80047144 name=sub_80047144 | jal     sub_80047144
0x8004690C op0 -> 0x8004692C name=loc_8004692C | j       loc_8004692C
0x80046914 op0 -> 0x80047178 name=sub_80047178 | jal     sub_80047178
0x8004691C op1 -> 0x80046BA4 name=loc_80046BA4 | bnez    $v0, loc_80046BA4
0x80046924 op0 -> 0x80046BC4 name=sub_80046BC4 | jal     sub_80046BC4
0x8004692C op1 -> 0x8005D838 name=dword_8005D838 | lw      $v0, dword_8005D838
0x80046934 op1 -> 0x8005D83C name=dword_8005D83C | lw      $v1, dword_8005D83C
0x80046944 op2 -> 0x80046914 name=loc_80046914 | beq     $v0, $v1, loc_80046914
0x8004694C op0 -> 0x800358C0 name=SetIntrMask | jal     SetIntrMask
0x80046964 op1 -> 0x8005D735 name=byte_8005D735 | lbu     $v1, byte_8005D735
0x8004696C op1 -> 0x8005D840 name=dword_8005D840 | sw      $v0, dword_8005D840
0x80046974 op1 -> 0x800469CC name=loc_800469CC | beqz    $v1, loc_800469CC
0x8004697C op1 -> 0x8005D838 name=dword_8005D838 | lw      $v1, dword_8005D838
0x80046984 op1 -> 0x8005D83C name=dword_8005D83C | lw      $v0, dword_8005D83C
0x80046990 op2 -> 0x80046A2C name=loc_80046A2C | bne     $v1, $v0, loc_80046A2C
0x80046998 op1 -> 0x8005D814 name=dword_8005D814 | lw      $v0, dword_8005D814
0x800469B0 op1 -> 0x80046A2C name=loc_80046A2C | bnez    $v0, loc_80046A2C
0x800469B8 op1 -> 0x8005D740 name=dword_8005D740 | lw      $v0, dword_8005D740
0x800469C4 op1 -> 0x80046A2C name=loc_80046A2C | bnez    $v0, loc_80046A2C
0x800469CC op1 -> 0x8005D808 name=dword_8005D808 | lw      $v1, dword_8005D808
0x800469E4 op1 -> 0x800469D4 name=loc_800469D4 | beqz    $v0, loc_800469D4
0x800469F8 op1 -> 0x8005D840 name=dword_8005D840 | lw      $a0, dword_8005D840
0x80046A0C op1 -> 0x8005D82C name=dword_8005D82C | sw      $s0, dword_8005D82C
0x80046A14 op1 -> 0x8005D830 name=dword_8005D830 | sw      $s2, dword_8005D830
0x80046A1C op0 -> 0x800358C0 name=SetIntrMask | jal     SetIntrMask
0x80046A24 op0 -> 0x80046BA4 name=loc_80046BA4 | j       loc_80046BA4
0x80046A34 op0 -> 0x800357A4 name=DMACallback | jal     DMACallback
0x80046A3C op1 -> 0x80046AEC name=loc_80046AEC | beqz    $s1, loc_80046AEC
0x80046A54 op1 -> 0x80046A60 name=loc_80046A60 | bgez    $v0, loc_80046A60
0x80046A68 op1 -> 0x80046AA4 name=loc_80046AA4 | beqz    $v0, loc_80046AA4
0x80046A78 op1 -> 0x8005D838 name=dword_8005D838 | lw      $v1, dword_8005D838
0x80046A9C op0 -> 0x80046A54 name=loc_80046A54 | j       loc_80046A54
0x80046AA4 op1 -> 0x8005D838 name=dword_8005D838 | lw      $v0, dword_8005D838
0x80046AAC op1 -> 0x8005D838 name=dword_8005D838 | lw      $v1, dword_8005D838
0x80046AD8 op1 -> 0x8009444C name=dword_8009444C | sw      $v0, dword_8009444C($a0)
0x80046AE4 op0 -> 0x80046B10 name=loc_80046B10 | j       loc_80046B10
0x80046AEC op1 -> 0x8005D838 name=dword_8005D838 | lw      $v1, dword_8005D838
0x80046B04 op1 -> 0x8009444C name=dword_8009444C | sw      $s0, dword_8009444C($v0)
0x80046B10 op1 -> 0x8005D838 name=dword_8005D838 | lw      $v1, dword_8005D838
0x80046B28 op1 -> 0x80094450 name=dword_80094450 | sw      $s2, dword_80094450($v0)
0x80046B34 op1 -> 0x8005D838 name=dword_8005D838 | lw      $v1, dword_8005D838
0x80046B4C op1 -> 0x80094448 name=dword_80094448 | sw      $s3, dword_80094448($v0)
0x80046B58 op1 -> 0x8005D838 name=dword_8005D838 | lw      $v0, dword_8005D838
0x80046B60 op1 -> 0x8005D840 name=dword_8005D840 | lw      $a0, dword_8005D840
0x80046B70 op1 -> 0x8005D838 name=dword_8005D838 | sw      $v0, dword_8005D838
0x80046B78 op0 -> 0x800358C0 name=SetIntrMask | jal     SetIntrMask
0x80046B80 op0 -> 0x80046BC4 name=sub_80046BC4 | jal     sub_80046BC4
0x80046B88 op1 -> 0x8005D838 name=dword_8005D838 | lw      $v0, dword_8005D838
0x80046B90 op1 -> 0x8005D83C name=dword_8005D83C | lw      $v1, dword_8005D83C
```

## 0x80046840 sub_80046840

Bounds: 0x80046840..0x8004688C

### Hex-Rays pseudo-C
```c
int __fastcall sub_80046840(int a1)
{
  int result; // $v0

  MEMORY[0x1F801814] = 67108866;
  MEMORY[0x1F8010A0] = a1;
  MEMORY[0x1F8010A4] = 0;
  result = 528486568;
  MEMORY[0x1F8010A8] = 16778241;
  return result;
}

```

### Disassembly
```asm
0x80046840: lui     $v1, 0x400
0x80046844: lw      $v0, dword_8005D808
0x8004684C: li      $v1, 0x4000002
0x80046850: sw      $v1, 0($v0)
0x80046854: lw      $v0, dword_8005D80C
0x8004685C: nop
0x80046860: sw      $a0, 0($v0)
0x80046864: lw      $v0, dword_8005D810
0x8004686C: lui     $v1, 0x100
0x80046870: sw      $zero, 0($v0)
0x80046874: lw      $v0, dword_8005D814
0x8004687C: li      $v1, 0x1000401
0x80046880: sw      $v1, 0($v0)
0x80046884: jr      $ra
0x80046888: nop
```

### Xrefs to
```text
0x800462A0 -> 0x80046840 type=17 from_func=sub_800460AC
0x8005D704 -> 0x80046840 type=1 from_func=<none>
```

### Xrefs from
```text
0x80046844 -> 0x8005D808 type=3 target=dword_8005D808
0x80046850 -> 0x1F801814 type=2 target=sub_1F801814
0x80046854 -> 0x8005D80C type=3 target=dword_8005D80C
0x80046860 -> 0x1F8010A0 type=2 target=sub_1F8010A0
0x80046864 -> 0x8005D810 type=3 target=dword_8005D810
0x80046870 -> 0x1F8010A4 type=2 target=sub_1F8010A4
0x80046874 -> 0x8005D814 type=3 target=dword_8005D814
0x80046880 -> 0x1F8010A8 type=2 target=sub_1F8010A8
```

### Referenced globals / immediates
```text
0x80046844 op1 -> 0x8005D808 name=dword_8005D808 | lw      $v0, dword_8005D808
0x80046854 op1 -> 0x8005D80C name=dword_8005D80C | lw      $v0, dword_8005D80C
0x80046864 op1 -> 0x8005D810 name=dword_8005D810 | lw      $v0, dword_8005D810
0x80046874 op1 -> 0x8005D814 name=dword_8005D814 | lw      $v0, dword_8005D814
```

## 0x80044E2C sub_80044E2C

Bounds: 0x80044E2C..0x80044EF0

### Hex-Rays pseudo-C
```c
// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_80044E2C(int a1, unsigned __int16 a2, int a3)
{
  int result; // $v0
  int v7; // $a0

  sub_80044BA8("MoveImage", a1);
  result = -1;
  if ( *(_WORD *)(a1 + 4) )
  {
    if ( *(_WORD *)(a1 + 6) )
    {
      v7 = *(_DWORD *)a1;
      dword_8005D7E8 = (a3 << 16) | a2;
      dword_8005D7E4 = v7;
      dword_8005D7EC = *(_DWORD *)(a1 + 4);
      return sub_800468E0(sub_80046840, dword_8005D7DC, 20, 0);
    }
    else
    {
      return -1;
    }
  }
  return result;
}

```

### Disassembly
```asm
0x80044E2C: addiu   $sp, -0x20
0x80044E30: sw      $s0, 0x10+var_s0($sp)
0x80044E34: move    $s0, $a0
0x80044E38: sw      $s2, 0x10+var_s8($sp)
0x80044E3C: move    $s2, $a1
0x80044E40: sw      $s1, 0x10+var_s4($sp)
0x80044E44: move    $s1, $a2
0x80044E48: li      $a0, aMoveimage # "MoveImage"
0x80044E50: sw      $ra, 0x10+var_sC($sp)
0x80044E54: jal     sub_80044BA8
0x80044E58: move    $a1, $s0
0x80044E5C: lh      $v0, 4($s0)
0x80044E60: nop
0x80044E64: beqz    $v0, loc_80044ED4
0x80044E68: li      $v0, 0xFFFFFFFF
0x80044E6C: lh      $v0, 6($s0)
0x80044E70: nop
0x80044E74: bnez    $v0, loc_80044E84
0x80044E78: sll     $v0, $s1, 16
0x80044E7C: j       loc_80044ED4
0x80044E80: li      $v0, 0xFFFFFFFF
0x80044E84: andi    $v1, $s2, 0xFFFF
0x80044E88: or      $v0, $v1
0x80044E8C: li      $a1, dword_8005D7E4
0x80044E94: lw      $a0, 0($s0)
0x80044E98: lw      $v1, off_8005D72C
0x80044EA0: li      $a2, 0x14
0x80044EA4: sw      $v0, dword_8005D7E8
0x80044EAC: sw      $a0, (dword_8005D7E4 - 0x8005D7E4)($a1)
0x80044EB0: lw      $v0, 4($s0)
0x80044EB4: move    $a3, $zero
0x80044EB8: sw      $v0, dword_8005D7EC
0x80044EC0: lw      $a0, (off_8005D704 - 0x8005D6EC)($v1)
0x80044EC4: lw      $v0, (off_8005D6F4 - 0x8005D6EC)($v1)
0x80044EC8: nop
0x80044ECC: jalr    $v0  # sub_800468E0
0x80044ED0: addiu   $a1, (dword_8005D7DC - 0x8005D7E4)
0x80044ED4: lw      $ra, 0x10+var_sC($sp)
0x80044ED8: lw      $s2, 0x10+var_s8($sp)
0x80044EDC: lw      $s1, 0x10+var_s4($sp)
0x80044EE0: lw      $s0, 0x10+var_s0($sp)
0x80044EE4: addiu   $sp, 0x20
0x80044EE8: jr      $ra
0x80044EEC: nop
```

### Xrefs to
```text
0x8001B194 -> 0x80044E2C type=17 from_func=sub_8001B120
```

### Xrefs from
```text
0x80044E48 -> 0x80012554 type=1 target=aMoveimage
0x80044E54 -> 0x80044BA8 type=17 target=sub_80044BA8
0x80044E8C -> 0x8005D7E4 type=1 target=dword_8005D7E4
0x80044E98 -> 0x8005D72C type=3 target=off_8005D72C
0x80044EA4 -> 0x8005D7E8 type=2 target=dword_8005D7E8
0x80044EAC -> 0x8005D7E4 type=2 target=dword_8005D7E4
0x80044EB8 -> 0x8005D7EC type=2 target=dword_8005D7EC
0x80044EC0 -> 0x8005D704 type=3 target=off_8005D704
0x80044EC4 -> 0x8005D6F4 type=3 target=off_8005D6F4
0x80044ECC -> 0x800468E0 type=17 target=sub_800468E0
0x80044ED0 -> 0x8005D7DC type=1 target=dword_8005D7DC
```

### Referenced globals / immediates
```text
0x80044E54 op0 -> 0x80044BA8 name=sub_80044BA8 | jal     sub_80044BA8
0x80044E64 op1 -> 0x80044ED4 name=loc_80044ED4 | beqz    $v0, loc_80044ED4
0x80044E74 op1 -> 0x80044E84 name=loc_80044E84 | bnez    $v0, loc_80044E84
0x80044E7C op0 -> 0x80044ED4 name=loc_80044ED4 | j       loc_80044ED4
0x80044E98 op1 -> 0x8005D72C name=off_8005D72C | lw      $v1, off_8005D72C
0x80044EA4 op1 -> 0x8005D7E8 name=dword_8005D7E8 | sw      $v0, dword_8005D7E8
0x80044EB8 op1 -> 0x8005D7EC name=dword_8005D7EC | sw      $v0, dword_8005D7EC
```

## 0x8001B120 sub_8001B120

Bounds: 0x8001B120..0x8001B1B0

### Hex-Rays pseudo-C
```c
int __fastcall sub_8001B120(int a1)
{
  int v1; // $v0
  int v2; // $a3
  int v4; // $a2
  int v6; // [sp+10h] [-8h] BYREF
  int v7; // [sp+14h] [-4h]

  v1 = sub_8004019C();
  v7 = 15728960;
  LOWORD(v6) = 0;
  if ( a1 )
  {
    if ( v1 )
      HIWORD(v6) = 0;
    else
      HIWORD(v6) = 240;
    v4 = v1 != 0;
  }
  else
  {
    if ( v1 )
      HIWORD(v6) = 240;
    else
      HIWORD(v6) = 0;
    v4 = v1 == 0;
  }
  return sub_80044E2C(&v6, 0, -v4 & 0xF0, v2, v6, v7);
}

```

### Disassembly
```asm
0x8001B120: addiu   $sp, -0x20
0x8001B124: sw      $s0, 0x18+var_s0($sp)
0x8001B128: sw      $ra, 0x18+var_s4($sp)
0x8001B12C: jal     sub_8004019C
0x8001B130: move    $s0, $a0
0x8001B134: li      $v1, 0x140
0x8001B138: sh      $v1, 0x18+var_4($sp)
0x8001B13C: li      $v1, 0xF0
0x8001B140: sh      $zero, 0x18+var_8($sp)
0x8001B144: beqz    $s0, loc_8001B170
0x8001B148: sh      $v1, 0x18+var_2($sp)
0x8001B14C: bnez    $v0, loc_8001B15C
0x8001B150: nop
0x8001B154: j       loc_8001B160
0x8001B158: sh      $v1, 0x18+var_6($sp)
0x8001B15C: sh      $zero, 0x18+var_6($sp)
0x8001B160: addiu   $a0, $sp, 0x18+var_8
0x8001B164: move    $a1, $zero
0x8001B168: j       loc_8001B190
0x8001B16C: sltu    $a2, $zero, $v0
0x8001B170: beqz    $v0, loc_8001B180
0x8001B174: nop
0x8001B178: j       loc_8001B184
0x8001B17C: sh      $v1, 0x18+var_6($sp)
0x8001B180: sh      $zero, 0x18+var_6($sp)
0x8001B184: addiu   $a0, $sp, 0x18+var_8
0x8001B188: move    $a1, $zero
0x8001B18C: sltiu   $a2, $v0, 1
0x8001B190: negu    $a2, $a2
0x8001B194: jal     sub_80044E2C
0x8001B198: andi    $a2, 0xF0
0x8001B19C: lw      $ra, 0x18+var_s4($sp)
0x8001B1A0: lw      $s0, 0x18+var_s0($sp)
0x8001B1A4: addiu   $sp, 0x20
0x8001B1A8: jr      $ra
0x8001B1AC: nop
```

### Xrefs to
```text
0x8001E850 -> 0x8001B120 type=17 from_func=sub_8001E750
0x801C474C -> 0x8001B120 type=17 from_func=sub_801C455C
0x801C4E54 -> 0x8001B120 type=17 from_func=sub_801C4DC4
```

### Xrefs from
```text
0x8001B12C -> 0x8004019C type=17 target=sub_8004019C
0x8001B194 -> 0x80044E2C type=17 target=sub_80044E2C
```

### Referenced globals / immediates
```text
0x8001B12C op0 -> 0x8004019C name=sub_8004019C | jal     sub_8004019C
0x8001B144 op1 -> 0x8001B170 name=loc_8001B170 | beqz    $s0, loc_8001B170
0x8001B14C op1 -> 0x8001B15C name=loc_8001B15C | bnez    $v0, loc_8001B15C
0x8001B154 op0 -> 0x8001B160 name=loc_8001B160 | j       loc_8001B160
0x8001B168 op0 -> 0x8001B190 name=loc_8001B190 | j       loc_8001B190
0x8001B170 op1 -> 0x8001B180 name=loc_8001B180 | beqz    $v0, loc_8001B180
0x8001B178 op0 -> 0x8001B184 name=loc_8001B184 | j       loc_8001B184
0x8001B194 op0 -> 0x80044E2C name=sub_80044E2C | jal     sub_80044E2C
```

## 0x8004019C sub_8004019C

Bounds: 0x8004019C..0x800401AC

### Hex-Rays pseudo-C
```c
int sub_8004019C()
{
  return word_80096590;
}

```

### Disassembly
```asm
0x8004019C: lh      $v0, word_80096590
0x800401A4: jr      $ra
0x800401A8: nop
```

### Xrefs to
```text
0x8001B12C -> 0x8004019C type=17 from_func=sub_8001B120
0x8001E414 -> 0x8004019C type=17 from_func=sub_8001E408
0x8001E5B0 -> 0x8004019C type=17 from_func=sub_8001E5A4
0x8001E764 -> 0x8004019C type=17 from_func=sub_8001E750
0x8001EA88 -> 0x8004019C type=17 from_func=sub_8001EA74
0x8001EC6C -> 0x8004019C type=17 from_func=sub_8001EC54
0x8001EDCC -> 0x8004019C type=17 from_func=sub_8001EDBC
0x8002758C -> 0x8004019C type=17 from_func=sub_8002756C
0x801C642C -> 0x8004019C type=17 from_func=sub_801C6410
```

### Xrefs from
```text
0x8004019C -> 0x80096590 type=3 target=word_80096590
```

### Referenced globals / immediates
```text
0x8004019C op1 -> 0x80096590 name=word_80096590 | lh      $v0, word_80096590
```

## 0x80040F90 sub_80040F90

Bounds: 0x80040F90..0x80040FA0

### Hex-Rays pseudo-C
```c
void __fastcall sub_80040F90(int a1)
{
  dword_800901C8 = a1;
}

```

### Disassembly
```asm
0x80040F90: sw      $a0, dword_800901C8
0x80040F98: jr      $ra
0x80040F9C: nop
```

### Xrefs to
```text
0x8001E434 -> 0x80040F90 type=17 from_func=sub_8001E408
0x8001E5D0 -> 0x80040F90 type=17 from_func=sub_8001E5A4
0x8001E784 -> 0x80040F90 type=17 from_func=sub_8001E750
0x8001EAB0 -> 0x80040F90 type=17 from_func=sub_8001EA74
0x8001EC8C -> 0x80040F90 type=17 from_func=sub_8001EC54
0x8001EDEC -> 0x80040F90 type=17 from_func=sub_8001EDBC
0x801C6450 -> 0x80040F90 type=17 from_func=sub_801C6410
```

### Xrefs from
```text
0x80040F90 -> 0x800901C8 type=2 target=dword_800901C8
```

### Referenced globals / immediates
```text
0x80040F90 op1 -> 0x800901C8 name=dword_800901C8 | sw      $a0, dword_800901C8
```

## 0x80040CC8 sub_80040CC8

Bounds: 0x80040CC8..0x80040D20

### Hex-Rays pseudo-C
```c
int __fastcall sub_80040CC8(unsigned __int16 a1, unsigned __int16 a2, int *a3)
{
  int v3; // $v1
  int v4; // $a0
  int v5; // $a1

  v3 = *a3;
  a3[2] = a1;
  v4 = a3[1];
  a3[3] = a2;
  v5 = *a3;
  a3[4] = (4 << v3) + v4 - 4;
  return sub_80044FA8(a3[1], 1 << v5);
}

```

### Disassembly
```asm
0x80040CC8: addiu   $sp, -0x18
0x80040CCC: andi    $a0, 0xFFFF
0x80040CD0: sw      $ra, 0x10+var_s0($sp)
0x80040CD4: lw      $v1, 0($a2)
0x80040CD8: andi    $a1, 0xFFFF
0x80040CDC: sw      $a0, 8($a2)
0x80040CE0: lw      $a0, 4($a2)
0x80040CE4: li      $v0, 4
0x80040CE8: sw      $a1, 0xC($a2)
0x80040CEC: lw      $a1, 0($a2)
0x80040CF0: sllv    $v0, $v1
0x80040CF4: addu    $v0, $a0
0x80040CF8: addiu   $v0, -4
0x80040CFC: sw      $v0, 0x10($a2)
0x80040D00: li      $v0, 1
0x80040D04: lw      $a0, 4($a2)
0x80040D08: jal     sub_80044FA8
0x80040D0C: sllv    $a1, $v0, $a1
0x80040D10: lw      $ra, 0x10+var_s0($sp)
0x80040D14: addiu   $sp, 0x18
0x80040D18: jr      $ra
0x80040D1C: nop
```

### Xrefs to
```text
0x8001E398 -> 0x80040CC8 type=17 from_func=sub_8001E374
0x8001E7AC -> 0x80040CC8 type=17 from_func=sub_8001E750
0x8001EAD8 -> 0x80040CC8 type=17 from_func=sub_8001EA74
0x8001ECB4 -> 0x80040CC8 type=17 from_func=sub_8001EC54
0x8001EE14 -> 0x80040CC8 type=17 from_func=sub_8001EDBC
0x801C647C -> 0x80040CC8 type=17 from_func=sub_801C6410
```

### Xrefs from
```text
0x80040D08 -> 0x80044FA8 type=17 target=sub_80044FA8
```

### Referenced globals / immediates
```text
0x80040D08 op0 -> 0x80044FA8 name=sub_80044FA8 | jal     sub_80044FA8
```

## 0x80040370 sub_80040370

Bounds: 0x80040370..0x80040420

### Hex-Rays pseudo-C
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

### Disassembly
```asm
0x80040370: lh      $v0, word_80096590
0x80040378: addiu   $sp, -0x18
0x8004037C: sw      $ra, 0x10+var_s0($sp)
0x80040380: sll     $v0, 1
0x80040384: lhu     $v1, word_8008ECA8($v0)
0x80040390: li      $a0, word_80091790
0x80040398: sh      $v1, (word_80091790 - 0x80091790)($a0)
0x8004039C: lhu     $v0, word_8008ECAC($v0)
0x800403A8: sh      $v0, word_80091792
0x800403B0: jal     sub_800452EC
0x800403B4: nop
0x800403B8: jal     sub_80044AA0
0x800403BC: li      $a0, 1
0x800403C0: lw      $v0, dword_8009658C
0x800403C8: nop
0x800403CC: addiu   $v1, $v0, 1
0x800403D0: sw      $v1, dword_8009658C
0x800403D8: beqz    $v0, loc_800403E4
0x800403DC: li      $a0, 1
0x800403E0: move    $a0, $v1
0x800403E4: lh      $v0, word_80096590
0x800403EC: sw      $a0, dword_8009658C
0x800403F4: sltiu   $v0, 1
0x800403F8: sh      $v0, word_80096590
0x80040400: jal     sub_800402E0
0x80040404: nop
0x80040408: jal     sub_800401AC
0x8004040C: nop
0x80040410: lw      $ra, 0x10+var_s0($sp)
0x80040414: addiu   $sp, 0x18
0x80040418: jr      $ra
0x8004041C: nop
```

### Xrefs to
```text
0x8001E554 -> 0x80040370 type=17 from_func=sub_8001E54C
0x8001EA0C -> 0x80040370 type=17 from_func=sub_8001EA00
0x8001EBFC -> 0x80040370 type=17 from_func=sub_8001EBF4
0x8001ED7C -> 0x80040370 type=17 from_func=sub_8001ED74
0x801C4B7C -> 0x80040370 type=17 from_func=sub_801C4894
0x801C4B8C -> 0x80040370 type=17 from_func=sub_801C4894
0x801C68A4 -> 0x80040370 type=17 from_func=sub_801C689C
0x801C6920 -> 0x80040370 type=17 from_func=sub_801C689C
```

### Xrefs from
```text
0x80040370 -> 0x80096590 type=3 target=word_80096590
0x80040384 -> 0x8008ECA8 type=3 target=word_8008ECA8
0x80040390 -> 0x80091790 type=1 target=word_80091790
0x80040398 -> 0x80091790 type=2 target=word_80091790
0x8004039C -> 0x8008ECAC type=3 target=word_8008ECAC
0x800403A8 -> 0x80091792 type=2 target=word_80091792
0x800403B0 -> 0x800452EC type=17 target=sub_800452EC
0x800403B8 -> 0x80044AA0 type=17 target=sub_80044AA0
0x800403C0 -> 0x8009658C type=3 target=dword_8009658C
0x800403D0 -> 0x8009658C type=2 target=dword_8009658C
0x800403E4 -> 0x80096590 type=3 target=word_80096590
0x800403EC -> 0x8009658C type=2 target=dword_8009658C
0x800403F8 -> 0x80096590 type=2 target=word_80096590
0x80040400 -> 0x800402E0 type=17 target=sub_800402E0
0x80040408 -> 0x800401AC type=17 target=sub_800401AC
```

### Referenced globals / immediates
```text
0x80040370 op1 -> 0x80096590 name=word_80096590 | lh      $v0, word_80096590
0x80040384 op1 -> 0x8008ECA8 name=word_8008ECA8 | lhu     $v1, word_8008ECA8($v0)
0x8004039C op1 -> 0x8008ECAC name=word_8008ECAC | lhu     $v0, word_8008ECAC($v0)
0x800403A8 op1 -> 0x80091792 name=word_80091792 | sh      $v0, word_80091792
0x800403B0 op0 -> 0x800452EC name=sub_800452EC | jal     sub_800452EC
0x800403B8 op0 -> 0x80044AA0 name=sub_80044AA0 | jal     sub_80044AA0
0x800403C0 op1 -> 0x8009658C name=dword_8009658C | lw      $v0, dword_8009658C
0x800403D0 op1 -> 0x8009658C name=dword_8009658C | sw      $v1, dword_8009658C
0x800403D8 op1 -> 0x800403E4 name=loc_800403E4 | beqz    $v0, loc_800403E4
0x800403E4 op1 -> 0x80096590 name=word_80096590 | lh      $v0, word_80096590
0x800403EC op1 -> 0x8009658C name=dword_8009658C | sw      $a0, dword_8009658C
0x800403F8 op1 -> 0x80096590 name=word_80096590 | sh      $v0, word_80096590
0x80040400 op0 -> 0x800402E0 name=sub_800402E0 | jal     sub_800402E0
0x80040408 op0 -> 0x800401AC name=sub_800401AC | jal     sub_800401AC
```
