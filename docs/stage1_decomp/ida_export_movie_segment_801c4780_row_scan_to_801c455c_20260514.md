[summary]
success: True
mode: exec
filename: <mcp-idapython-exec-job>
persist_globals: True
result_type: NoneType
result_repr: None

[stdout]
# IDA export: movie segment 801C4780 row scan to 801C455C (2026-05-14)

Authority: current active IDA database. Static evidence only; no Win file/ISO/playback-derived success authority.
Input file: `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`

## Function `801C4780` `sub_801C4780`
Bounds: `801C4780-801C47EC`

### Pseudocode
```c
int sub_801C4780()
{
  int v0; // $s1
  int v1; // $s0

  v0 = 0;
  v1 = 12;
  do
  {
    ++v0;
    sub_8001A324((_DWORD *)v1);
    v1 += 48;
  }
  while ( v0 < 7 );
  sub_80025A00();
  return sub_8001AC18((_DWORD *)0x3C, 0);
}
```

### Direct xrefs
- `801C4330` -> `801C4780` type=17 caller=801C4260 `sub_801C4260`: `jal     sub_801C4780`

### Disassembly
```asm
801C4780: addiu   $sp, -0x20
801C4784: sw      $s1, 0x10+var_s4($sp)
801C4788: move    $s1, $zero
801C478C: sw      $s0, 0x10+var_s0($sp)
801C4790: li      $s0, 0xC
801C4794: sw      $ra, 0x10+var_s8($sp)
801C4798: lw      $a0, dword_8006ED98+0x20
801C47A0: addiu   $s1, 1
801C47A4: jal     sub_8001A324
801C47A8: addu    $a0, $s0
801C47AC: slti    $v0, $s1, 7
801C47B0: bnez    $v0, loc_801C4798
801C47B4: addiu   $s0, 0x30  # '0'
801C47B8: jal     sub_80025A00
801C47BC: nop
801C47C0: lw      $a0, dword_8006ED98+0x20
801C47C8: move    $a1, $zero
801C47CC: jal     sub_8001AC18
801C47D0: addiu   $a0, 0x3C  # '<'
801C47D4: lw      $ra, 0x10+var_s8($sp)
801C47D8: lw      $s1, 0x10+var_s4($sp)
801C47DC: lw      $s0, 0x10+var_s0($sp)
801C47E0: addiu   $sp, 0x20
801C47E4: jr      $ra
801C47E8: nop
```

## Function `8001A324` `sub_8001A324`
Bounds: `8001A324-8001A3B8`

### Pseudocode
```c
int __fastcall sub_8001A324(_DWORD *a1)
{
  int result; // $v0
  bool v3; // dc
  int v4; // $a0
  unsigned int v5; // $v1

  if ( a1[3] == 1 || !*a1 )
    return 0;
  v3 = sub_8001A2B0(a1 + 4) == 0;
  result = -1;
  if ( !v3 )
  {
    v4 = sub_80036A78(a1 + 4);
    v5 = a1[5];
    result = 0;
    a1[10] = v4;
    a1[3] = 1;
    a1[11] = v4 + (v5 >> 11);
  }
  return result;
}
```

### Direct xrefs
- `80015E10` -> `8001A324` type=17 caller=80015D18 `sub_80015D18`: `jal     sub_8001A324`
- `8001AC64` -> `8001A324` type=17 caller=8001AC18 `sub_8001AC18`: `jal     sub_8001A324`
- `8001AD3C` -> `8001A324` type=17 caller=8001ACF8 `sub_8001ACF8`: `jal     sub_8001A324`
- `801C47A4` -> `8001A324` type=17 caller=801C4780 `sub_801C4780`: `jal     sub_8001A324`

### Disassembly
```asm
8001A324: addiu   $sp, -0x20
8001A328: sw      $s0, 0x10+var_s0($sp)
8001A32C: move    $s0, $a0
8001A330: sw      $ra, 0x10+var_sC($sp)
8001A334: sw      $s2, 0x10+var_s8($sp)
8001A338: sw      $s1, 0x10+var_s4($sp)
8001A33C: lw      $v0, 0xC($s0)
8001A340: li      $s2, 1
8001A344: beq     $v0, $s2, loc_8001A35C
8001A348: nop
8001A34C: lw      $a1, 0($s0)
8001A350: nop
8001A354: bnez    $a1, loc_8001A364
8001A358: addiu   $s1, $s0, 0x10
8001A35C: j       loc_8001A39C
8001A360: move    $v0, $zero
8001A364: jal     sub_8001A2B0
8001A368: move    $a0, $s1
8001A36C: beqz    $v0, loc_8001A39C
8001A370: li      $v0, 0xFFFFFFFF
8001A374: jal     sub_80036A78
8001A378: move    $a0, $s1
8001A37C: move    $a0, $v0
8001A380: lw      $v1, 0x14($s0)
8001A384: move    $v0, $zero
8001A388: sw      $a0, 0x28($s0)
8001A38C: sw      $s2, 0xC($s0)
8001A390: srl     $v1, 11
8001A394: addu    $a0, $v1
8001A398: sw      $a0, 0x2C($s0)
8001A39C: lw      $ra, 0x10+var_sC($sp)
8001A3A0: lw      $s2, 0x10+var_s8($sp)
8001A3A4: lw      $s1, 0x10+var_s4($sp)
8001A3A8: lw      $s0, 0x10+var_s0($sp)
8001A3AC: addiu   $sp, 0x20
8001A3B0: jr      $ra
8001A3B4: nop
```

## Function `8001A2B0` `sub_8001A2B0`
Bounds: `8001A2B0-8001A324`

### Pseudocode
```c
int __fastcall sub_8001A2B0(int a1, int a2)
{
  int v2; // $s2
  int i; // $s0

  v2 = a1;
  if ( !a2 )
    return 0;
  for ( i = 0; i < 4; ++i )
  {
    if ( sub_800381F8(a1, a2) )
      return v2;
    a1 = v2;
  }
  return 0;
}
```

### Direct xrefs
- `8001A364` -> `8001A2B0` type=17 caller=8001A324 `sub_8001A324`: `jal     sub_8001A2B0`

### Disassembly
```asm
8001A2B0: addiu   $sp, -0x20
8001A2B4: sw      $s2, 0x10+var_s8($sp)
8001A2B8: move    $s2, $a0
8001A2BC: sw      $s1, 0x10+var_s4($sp)
8001A2C0: move    $s1, $a1
8001A2C4: sw      $ra, 0x10+var_sC($sp)
8001A2C8: bnez    $s1, loc_8001A2E0
8001A2CC: sw      $s0, 0x10+var_s0($sp)
8001A2D0: j       loc_8001A308
8001A2D4: move    $v0, $zero
8001A2D8: j       loc_8001A308
8001A2DC: move    $v0, $s2
8001A2E0: move    $s0, $zero
8001A2E4: move    $a0, $s2
8001A2E8: jal     sub_800381F8
8001A2EC: move    $a1, $s1
8001A2F0: bnez    $v0, loc_8001A2D8
8001A2F4: addiu   $s0, 1
8001A2F8: slti    $v0, $s0, 4
8001A2FC: bnez    $v0, loc_8001A2E8
8001A300: move    $a0, $s2
8001A304: move    $v0, $zero
8001A308: lw      $ra, 0x10+var_sC($sp)
8001A30C: lw      $s2, 0x10+var_s8($sp)
8001A310: lw      $s1, 0x10+var_s4($sp)
8001A314: lw      $s0, 0x10+var_s0($sp)
8001A318: addiu   $sp, 0x20
8001A31C: jr      $ra
8001A320: nop
```

## Function `80036A78` `sub_80036A78`
Bounds: `80036A78-80036AF8`

### Pseudocode
```c
int __fastcall sub_80036A78(unsigned __int8 *a1)
{
  return 75 * (60 * (10 * (*a1 >> 4) + (*a1 & 0xF)) + 10 * (a1[1] >> 4) + (a1[1] & 0xF))
       + 10 * (a1[2] >> 4)
       + (a1[2] & 0xF)
       - 150;
}
```

### Direct xrefs
- `8001A374` -> `80036A78` type=17 caller=8001A324 `sub_8001A324`: `jal     sub_80036A78`
- `8001A520` -> `80036A78` type=17 caller=8001A4D0 `sub_8001A4D0`: `jal     sub_80036A78`
- `8001A7C0` -> `80036A78` type=17 caller=8001A7A4 `sub_8001A7A4`: `jal     sub_80036A78`
- `8001A7D4` -> `80036A78` type=17 caller=8001A7A4 `sub_8001A7A4`: `jal     sub_80036A78`
- `8001A8B0` -> `80036A78` type=17 caller=8001A89C `sub_8001A89C`: `jal     sub_80036A78`
- `80038C18` -> `80036A78` type=17 caller=80038BC4 `sub_80038BC4`: `jal     sub_80036A78`
- `80038F38` -> `80036A78` type=17 caller=80038DE8 `sub_80038DE8`: `jal     sub_80036A78`
- `800393CC` -> `80036A78` type=17 caller=800393A8 `sub_800393A8`: `jal     sub_80036A78`

### Disassembly
```asm
80036A78: lbu     $v1, 0($a0)
80036A7C: lbu     $a2, 1($a0)
80036A80: srl     $a1, $v1, 4
80036A84: sll     $v0, $a1, 2
80036A88: addu    $v0, $a1
80036A8C: sll     $v0, 1
80036A90: andi    $v1, 0xF
80036A94: addu    $v0, $v1
80036A98: sll     $a1, $v0, 4
80036A9C: subu    $a1, $v0
80036AA0: sll     $a1, 2
80036AA4: srl     $v1, $a2, 4
80036AA8: sll     $v0, $v1, 2
80036AAC: addu    $v0, $v1
80036AB0: sll     $v0, 1
80036AB4: andi    $a2, 0xF
80036AB8: addu    $v0, $a2
80036ABC: addu    $a1, $v0
80036AC0: sll     $v1, $a1, 2
80036AC4: addu    $v1, $a1
80036AC8: sll     $v0, $v1, 4
80036ACC: lbu     $a1, 2($a0)
80036AD0: subu    $v0, $v1
80036AD4: srl     $a0, $a1, 4
80036AD8: sll     $v1, $a0, 2
80036ADC: addu    $v1, $a0
80036AE0: sll     $v1, 1
80036AE4: andi    $a1, 0xF
80036AE8: addu    $v1, $a1
80036AEC: addu    $v0, $v1
80036AF0: jr      $ra
80036AF4: addiu   $v0, -0x96
```

## Function `801C4260` `sub_801C4260`
Bounds: `801C4260-801C4350`

### Pseudocode
```c
// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_801C4260(int a1, int a2, int a3, int a4, int a5, int a6, int (*a7)(void))
{
  int v8; // $v0
  int v9; // $a2
  int v10; // $a0

  sub_80025A34();
  sub_801C4FA0();
  sub_80024E98();
  sub_80014344();
  v8 = 96 * *(__int16 *)(a1 + 6);
  dword_8006ED98[8] = a1;
  v9 = *(__int16 *)(a1 + 10);
  *(_DWORD *)(a1 + 360) = 16;
  v10 = *(__int16 *)(a1 + 8);
  *(_DWORD *)(a1 + 348) = v8 / 100;
  *(_DWORD *)(a1 + 356) = v10 + v9;
  *(_DWORD *)(a1 + 352) = (v8 / 3600 + 50) / 100;
  return sub_801C4780();
}
```

### Direct xrefs
- `80048D2C` -> `801C4260` type=1 caller=<no function>: `.word sub_801C4260`

### Disassembly
```asm
801C4260: addiu   $sp, -0x20
801C4264: sw      $s0, 0x10+var_s0($sp)
801C4268: move    $s0, $a0
801C426C: sw      $s1, 0x10+var_s4($sp)
801C4270: sw      $ra, 0x10+var_s8($sp)
801C4274: jal     sub_80025A34
801C4278: move    $s1, $a1
801C427C: jal     sub_801C4FA0
801C4280: nop
801C4284: jal     sub_80024E98
801C4288: nop
801C428C: jal     sub_80014344
801C4290: nop
801C4294: lui     $a0, 0x91A2
801C4298: lh      $v1, 6($s0)
801C429C: li      $a0, 0x91A2B3C5
801C42A0: sll     $v0, $v1, 1
801C42A4: addu    $v0, $v1
801C42A8: sll     $v0, 5
801C42AC: mult    $v0, $a0
801C42B0: lui     $a0, 0x51EB
801C42B4: lh      $v1, 6($s0)
801C42B8: li      $a0, 0x51EB851F
801C42BC: sll     $a1, $v1, 1
801C42C0: mfhi    $a2
801C42C4: addu    $a1, $v1
801C42C8: sll     $a1, 5
801C42CC: mult    $a1, $a0
801C42D0: sw      $s0, dword_8006ED98+0x20
801C42D8: addu    $v1, $a2, $v0
801C42DC: sra     $v1, 11
801C42E0: sra     $v0, 31
801C42E4: subu    $v1, $v0
801C42E8: mfhi    $a3
801C42EC: addiu   $v1, 0x32  # '2'
801C42F0: li      $v0, 0x10
801C42F4: mult    $v1, $a0
801C42F8: lh      $a2, 0xA($s0)
801C42FC: sra     $a1, 31
801C4300: sw      $v0, 0x168($s0)
801C4304: sra     $v0, $a3, 5
801C4308: subu    $v0, $a1
801C430C: lh      $a0, 8($s0)
801C4310: sra     $v1, 31
801C4314: sw      $v0, 0x15C($s0)
801C4318: addu    $a0, $a2
801C431C: sw      $a0, 0x164($s0)
801C4320: move    $a0, $s1
801C4324: mfhi    $t1
801C4328: sra     $v0, $t1, 5
801C432C: subu    $v0, $v1
801C4330: jal     sub_801C4780
801C4334: sw      $v0, 0x160($s0)
801C4338: lw      $ra, 0x10+var_s8($sp)
801C433C: lw      $s1, 0x10+var_s4($sp)
801C4340: lw      $s0, 0x10+var_s0($sp)
801C4344: addiu   $sp, 0x20
801C4348: jr      $ra
801C434C: nop
```

## Function `801C4DC4` `sub_801C4DC4`
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

### Direct xrefs
- `80048D30` -> `801C4DC4` type=1 caller=<no function>: `.word sub_801C4DC4`

### Disassembly
```asm
801C4DC4: addiu   $sp, -0x28
801C4DC8: sw      $s3, 0x10+var_sC($sp)
801C4DCC: sw      $s0, 0x10+var_s0($sp)
801C4DD0: li      $s0, word_800916D2
801C4DD8: sw      $ra, 0x10+var_s10($sp)
801C4DDC: sw      $s2, 0x10+var_s8($sp)
801C4DE0: sw      $s1, 0x10+var_s4($sp)
801C4DE4: lh      $v0, (word_800916D2 - 0x800916D2)($s0)
801C4DE8: li      $s1, unk_801C3640
801C4DF0: bnez    $v0, loc_801C4E14
801C4DF4: move    $s3, $a0
801C4DF8: li      $v0, 1
801C4DFC: sh      $v0, (word_800916D2 - 0x800916D2)($s0)
801C4E00: move    $a0, $s1
801C4E04: li      $a1, 2
801C4E08: li      $a2, 1
801C4E0C: jal     sub_80020110
801C4E10: li      $a3, 2
801C4E14: move    $a0, $s1
801C4E18: li      $a1, 6
801C4E1C: li      $a2, 1
801C4E20: jal     sub_800201AC
801C4E24: li      $a3, 2
801C4E28: lw      $a0, dword_8006ED98+0x20
801C4E30: move    $a1, $zero
801C4E34: jal     sub_801C44E0
801C4E38: addiu   $a0, 0x6C  # 'l'
801C4E3C: move    $a1, $s1
801C4E40: lw      $a0, dword_8006ED98+0x20
801C4E48: move    $a2, $zero
801C4E4C: jal     sub_801C455C
801C4E50: addiu   $a0, 0x6C  # 'l'
801C4E54: jal     sub_8001B120
801C4E58: li      $a0, 1
801C4E5C: move    $a0, $s1
801C4E60: li      $a1, 5
801C4E64: li      $a2, 1
801C4E68: li      $a3, 2
801C4E6C: li      $s2, 1
801C4E70: jal     sub_800201AC
801C4E74: sh      $s2, (word_800916D2 - 0x800916D2)($s0)
801C4E78: lw      $a0, dword_8006ED98+0x20
801C4E80: move    $a1, $s1
801C4E84: jal     sub_801C4894
801C4E88: addiu   $a0, 0x9C
801C4E8C: jal     sub_80026FA4
801C4E90: move    $s0, $v0
801C4E94: move    $a0, $s1
801C4E98: li      $a1, 2
801C4E9C: li      $a2, 1
801C4EA0: jal     sub_80020110
801C4EA4: li      $a3, 2
801C4EA8: jal     sub_8001EF14
801C4EAC: nop
801C4EB0: li      $v0, 1
801C4EB4: bne     $s0, $v0, loc_801C4ECC
801C4EB8: li      $v0, 3
801C4EBC: sh      $zero, word_800916D0
801C4EC4: j       loc_801C4F48
801C4EC8: li      $v0, 0xFFFFFFFF
801C4ECC: bne     $s0, $v0, loc_801C4F24
801C4ED0: lui     $s0, 0x2AAA
801C4ED4: sh      $s2, word_800916D0
801C4EDC: li      $s0, 0x2AAAAAAB
801C4EE0: jal     rand
801C4EE4: nop
801C4EE8: mult    $v0, $s0
801C4EEC: sra     $a0, $v0, 31
801C4EF0: mfhi    $t0
801C4EF4: subu    $a0, $t0, $a0
801C4EF8: sll     $v1, $a0, 1
801C4EFC: addu    $v1, $a0
801C4F00: sll     $v1, 1
801C4F04: subu    $v0, $v1
801C4F08: lh      $v1, word_800916EE
801C4F10: addiu   $v0, 1
801C4F14: beq     $v0, $v1, loc_801C4EE0
801C4F18: nop
801C4F1C: j       loc_801C4F48
801C4F20: nop
801C4F24: lw      $a0, dword_80094410
801C4F2C: jal     sub_80026EF8
801C4F30: nop
801C4F34: jal     sub_80026ECC
801C4F38: nop
801C4F3C: addiu   $v0, $s3, 1
801C4F40: sh      $zero, word_800916D0
801C4F48: lw      $ra, 0x10+var_s10($sp)
801C4F4C: lw      $s3, 0x10+var_sC($sp)
801C4F50: lw      $s2, 0x10+var_s8($sp)
801C4F54: lw      $s1, 0x10+var_s4($sp)
801C4F58: lw      $s0, 0x10+var_s0($sp)
801C4F5C: addiu   $sp, 0x28
801C4F60: jr      $ra
801C4F64: nop
```

## Function `801C455C` `sub_801C455C`
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

### Direct xrefs
- `801C4E4C` -> `801C455C` type=17 caller=801C4DC4 `sub_801C4DC4`: `jal     sub_801C455C`

### Disassembly
```asm
801C455C: addiu   $sp, -0x30
801C4560: sw      $s5, 0x10+var_s14($sp)
801C4564: move    $s5, $a0
801C4568: sw      $s0, 0x10+var_s0($sp)
801C456C: move    $s0, $a1
801C4570: sltiu   $v0, $a2, 4
801C4574: sw      $ra, 0x10+var_s18($sp)
801C4578: sw      $s4, 0x10+var_s10($sp)
801C457C: sw      $s3, 0x10+var_sC($sp)
801C4580: sw      $s2, 0x10+var_s8($sp)
801C4584: beqz    $v0, loc_801C45C8
801C4588: sw      $s1, 0x10+var_s4($sp)
801C458C: sll     $a0, $a2, 3
801C4590: subu    $a0, $a2
801C4594: lw      $v0, dword_800943CC
801C459C: sll     $a0, 2
801C45A0: j       loc_801C45CC
801C45A4: addu    $a0, $v0, $a0
801C45A8: j       loc_801C472C
801C45AC: li      $s3, 1
801C45B0: jal     sub_8001A694
801C45B4: nop
801C45B8: jal     StopCallback
801C45BC: nop
801C45C0: j       loc_801C472C
801C45C4: nop
801C45C8: move    $a0, $zero
801C45CC: jal     sub_80024C84
801C45D0: li      $s1, 0x708
801C45D4: li      $s4, sub_8001EC54
801C45DC: li      $s3, sub_8001ED3C
801C45E4: sw      $zero, dword_801C954C
801C45EC: li      $s2, 1
801C45F0: jal     sub_8001A750
801C45F4: nop
801C45F8: beq     $v0, $s2, loc_801C4634
801C45FC: nop
801C4600: beqz    $s4, loc_801C4610
801C4604: move    $a0, $s0
801C4608: jal     sub_8001EC54
801C460C: li      $a1, 7
801C4610: jal     sub_80035560
801C4614: li      $a0, 2
801C4618: beqz    $s3, loc_801C4628
801C461C: nop
801C4620: jal     sub_8001ED3C
801C4624: move    $a0, $s0
801C4628: addiu   $s1, -1
801C462C: bgtz    $s1, loc_801C45F0
801C4630: nop
801C4634: jal     sub_8001A280
801C4638: move    $s3, $zero
801C463C: lw      $v0, dword_8006ED98+0x20
801C4644: nop
801C4648: lw      $v0, 0x164($v0)
801C464C: nop
801C4650: sw      $v0, 0xC($s0)
801C4654: lhu     $v0, word_800916D8
801C465C: nop
801C4660: sh      $v0, 0x66($s0)
801C4664: lhu     $v0, word_800916DC
801C466C: li      $s2, 1
801C4670: sh      $v0, 0x68($s0)
801C4674: sh      $v0, 0x54($s0)
801C4678: sw      $zero, 0($s0)
801C467C: jal     sub_80035510
801C4680: li      $a0, 1
801C4684: move    $v1, $v0
801C4688: beqz    $v1, loc_801C46A8
801C468C: li      $v0, 0x100
801C4690: beq     $v1, $v0, loc_801C45A8
801C4694: li      $v0, 0x800
801C4698: beq     $v1, $v0, loc_801C472C
801C469C: andi    $v0, $v1, 0x840
801C46A0: bnez    $v0, loc_801C472C
801C46A4: nop
801C46A8: jal     sub_80024CF8
801C46AC: move    $a0, $s0
801C46B0: jal     sub_80027528
801C46B4: nop
801C46B8: lw      $v0, dword_801C9554
801C46C0: nop
801C46C4: bne     $v0, $s2, loc_801C46D4
801C46C8: move    $a0, $s0
801C46CC: jal     sub_8001EC54
801C46D0: li      $a1, 7
801C46D4: jal     sub_80027528
801C46D8: nop
801C46DC: move    $a0, $s5
801C46E0: jal     sub_801C448C
801C46E4: move    $a1, $s0
801C46E8: jal     sub_8001ED74
801C46EC: move    $s1, $v0
801C46F0: jal     sub_8002756C
801C46F4: nop
801C46F8: lw      $v0, dword_801C9554
801C4700: nop
801C4704: bne     $v0, $s2, loc_801C4714
801C4708: nop
801C470C: jal     sub_8001ED3C
801C4710: move    $a0, $s0
801C4714: jal     GsGetWorkBase
801C4718: nop
801C471C: beq     $v0, $s2, loc_801C45B0
801C4720: nop
801C4724: beq     $s1, $s2, loc_801C4678
801C4728: nop
801C472C: jal     sub_80027664
801C4730: nop
801C4734: jal     sub_8001A4A4
801C4738: li      $a0, 1
801C473C: jal     sub_8001A694
801C4740: nop
801C4744: jal     nullsub_4
801C4748: move    $a0, $s0
801C474C: jal     sub_8001B120
801C4750: li      $a0, 1
801C4754: move    $v0, $s3
801C4758: lw      $ra, 0x10+var_s18($sp)
801C475C: lw      $s5, 0x10+var_s14($sp)
801C4760: lw      $s4, 0x10+var_s10($sp)
801C4764: lw      $s3, 0x10+var_sC($sp)
801C4768: lw      $s2, 0x10+var_s8($sp)
801C476C: lw      $s1, 0x10+var_s4($sp)
801C4770: lw      $s0, 0x10+var_s0($sp)
801C4774: addiu   $sp, 0x30
801C4778: jr      $ra
801C477C: nop
```

## Data xrefs to `8006EDB8` `sub_8006EDB8`
- `801C42D0` -> `8006EDB8` type=2 caller=801C4260 `sub_801C4260`: `sw      $s0, dword_8006ED98+0x20`
- `801C463C` -> `8006EDB8` type=3 caller=801C455C `sub_801C455C`: `lw      $v0, dword_8006ED98+0x20`
- `801C4798` -> `8006EDB8` type=3 caller=801C4780 `sub_801C4780`: `lw      $a0, dword_8006ED98+0x20`
- `801C47C0` -> `8006EDB8` type=3 caller=801C4780 `sub_801C4780`: `lw      $a0, dword_8006ED98+0x20`
- `801C49C0` -> `8006EDB8` type=3 caller=801C4894 `sub_801C4894`: `lw      $v0, dword_8006ED98+0x20`
- `801C4E28` -> `8006EDB8` type=3 caller=801C4DC4 `sub_801C4DC4`: `lw      $a0, dword_8006ED98+0x20`
- `801C4E40` -> `8006EDB8` type=3 caller=801C4DC4 `sub_801C4DC4`: `lw      $a0, dword_8006ED98+0x20`
- `801C4E78` -> `8006EDB8` type=3 caller=801C4DC4 `sub_801C4DC4`: `lw      $a0, dword_8006ED98+0x20`

## Data xrefs to `8006ED98` `dword_8006ED98`
- none

## Focus windows
### Around `801C42D0` in `sub_801C4260`
```asm
   801C42A0: sll     $v0, $v1, 1
   801C42A4: addu    $v0, $v1
   801C42A8: sll     $v0, 5
   801C42AC: mult    $v0, $a0
   801C42B0: lui     $a0, 0x51EB
   801C42B4: lh      $v1, 6($s0)
   801C42B8: li      $a0, 0x51EB851F
   801C42BC: sll     $a1, $v1, 1
   801C42C0: mfhi    $a2
   801C42C4: addu    $a1, $v1
   801C42C8: sll     $a1, 5
   801C42CC: mult    $a1, $a0
=> 801C42D0: sw      $s0, dword_8006ED98+0x20
   801C42D8: addu    $v1, $a2, $v0
   801C42DC: sra     $v1, 11
   801C42E0: sra     $v0, 31
   801C42E4: subu    $v1, $v0
   801C42E8: mfhi    $a3
   801C42EC: addiu   $v1, 0x32  # '2'
   801C42F0: li      $v0, 0x10
   801C42F4: mult    $v1, $a0
   801C42F8: lh      $a2, 0xA($s0)
   801C42FC: sra     $a1, 31
   801C4300: sw      $v0, 0x168($s0)
   801C4304: sra     $v0, $a3, 5
   801C4308: subu    $v0, $a1
   801C430C: lh      $a0, 8($s0)
   801C4310: sra     $v1, 31
   801C4314: sw      $v0, 0x15C($s0)
   801C4318: addu    $a0, $a2
   801C431C: sw      $a0, 0x164($s0)
   801C4320: move    $a0, $s1
   801C4324: mfhi    $t1
   801C4328: sra     $v0, $t1, 5
   801C432C: subu    $v0, $v1
   801C4330: jal     sub_801C4780
   801C4334: sw      $v0, 0x160($s0)
   801C4338: lw      $ra, 0x10+var_s8($sp)
   801C433C: lw      $s1, 0x10+var_s4($sp)
```

### Around `801C4780` in `sub_801C4780`
```asm
=> 801C4780: addiu   $sp, -0x20
   801C4784: sw      $s1, 0x10+var_s4($sp)
   801C4788: move    $s1, $zero
   801C478C: sw      $s0, 0x10+var_s0($sp)
   801C4790: li      $s0, 0xC
   801C4794: sw      $ra, 0x10+var_s8($sp)
   801C4798: lw      $a0, dword_8006ED98+0x20
   801C47A0: addiu   $s1, 1
   801C47A4: jal     sub_8001A324
   801C47A8: addu    $a0, $s0
   801C47AC: slti    $v0, $s1, 7
   801C47B0: bnez    $v0, loc_801C4798
   801C47B4: addiu   $s0, 0x30  # '0'
   801C47B8: jal     sub_80025A00
   801C47BC: nop
   801C47C0: lw      $a0, dword_8006ED98+0x20
   801C47C8: move    $a1, $zero
   801C47CC: jal     sub_8001AC18
   801C47D0: addiu   $a0, 0x3C  # '<'
   801C47D4: lw      $ra, 0x10+var_s8($sp)
   801C47D8: lw      $s1, 0x10+var_s4($sp)
   801C47DC: lw      $s0, 0x10+var_s0($sp)
   801C47E0: addiu   $sp, 0x20
   801C47E4: jr      $ra
   801C47E8: nop
```

### Around `801C47A4` in `sub_801C4780`
```asm
   801C4780: addiu   $sp, -0x20
   801C4784: sw      $s1, 0x10+var_s4($sp)
   801C4788: move    $s1, $zero
   801C478C: sw      $s0, 0x10+var_s0($sp)
   801C4790: li      $s0, 0xC
   801C4794: sw      $ra, 0x10+var_s8($sp)
   801C4798: lw      $a0, dword_8006ED98+0x20
   801C47A0: addiu   $s1, 1
=> 801C47A4: jal     sub_8001A324
   801C47A8: addu    $a0, $s0
   801C47AC: slti    $v0, $s1, 7
   801C47B0: bnez    $v0, loc_801C4798
   801C47B4: addiu   $s0, 0x30  # '0'
   801C47B8: jal     sub_80025A00
   801C47BC: nop
   801C47C0: lw      $a0, dword_8006ED98+0x20
   801C47C8: move    $a1, $zero
   801C47CC: jal     sub_8001AC18
   801C47D0: addiu   $a0, 0x3C  # '<'
   801C47D4: lw      $ra, 0x10+var_s8($sp)
   801C47D8: lw      $s1, 0x10+var_s4($sp)
   801C47DC: lw      $s0, 0x10+var_s0($sp)
   801C47E0: addiu   $sp, 0x20
   801C47E4: jr      $ra
   801C47E8: nop
```

### Around `801C47B8` in `sub_801C4780`
```asm
   801C4788: move    $s1, $zero
   801C478C: sw      $s0, 0x10+var_s0($sp)
   801C4790: li      $s0, 0xC
   801C4794: sw      $ra, 0x10+var_s8($sp)
   801C4798: lw      $a0, dword_8006ED98+0x20
   801C47A0: addiu   $s1, 1
   801C47A4: jal     sub_8001A324
   801C47A8: addu    $a0, $s0
   801C47AC: slti    $v0, $s1, 7
   801C47B0: bnez    $v0, loc_801C4798
   801C47B4: addiu   $s0, 0x30  # '0'
=> 801C47B8: jal     sub_80025A00
   801C47BC: nop
   801C47C0: lw      $a0, dword_8006ED98+0x20
   801C47C8: move    $a1, $zero
   801C47CC: jal     sub_8001AC18
   801C47D0: addiu   $a0, 0x3C  # '<'
   801C47D4: lw      $ra, 0x10+var_s8($sp)
   801C47D8: lw      $s1, 0x10+var_s4($sp)
   801C47DC: lw      $s0, 0x10+var_s0($sp)
   801C47E0: addiu   $sp, 0x20
   801C47E4: jr      $ra
   801C47E8: nop
```

### Around `801C47D0` in `sub_801C4780`
```asm
   801C47A0: addiu   $s1, 1
   801C47A4: jal     sub_8001A324
   801C47A8: addu    $a0, $s0
   801C47AC: slti    $v0, $s1, 7
   801C47B0: bnez    $v0, loc_801C4798
   801C47B4: addiu   $s0, 0x30  # '0'
   801C47B8: jal     sub_80025A00
   801C47BC: nop
   801C47C0: lw      $a0, dword_8006ED98+0x20
   801C47C8: move    $a1, $zero
   801C47CC: jal     sub_8001AC18
=> 801C47D0: addiu   $a0, 0x3C  # '<'
   801C47D4: lw      $ra, 0x10+var_s8($sp)
   801C47D8: lw      $s1, 0x10+var_s4($sp)
   801C47DC: lw      $s0, 0x10+var_s0($sp)
   801C47E0: addiu   $sp, 0x20
   801C47E4: jr      $ra
   801C47E8: nop
```

### Around `801C4E14` in `sub_801C4DC4`
```asm
   801C4DE4: lh      $v0, (word_800916D2 - 0x800916D2)($s0)
   801C4DE8: li      $s1, unk_801C3640
   801C4DF0: bnez    $v0, loc_801C4E14
   801C4DF4: move    $s3, $a0
   801C4DF8: li      $v0, 1
   801C4DFC: sh      $v0, (word_800916D2 - 0x800916D2)($s0)
   801C4E00: move    $a0, $s1
   801C4E04: li      $a1, 2
   801C4E08: li      $a2, 1
   801C4E0C: jal     sub_80020110
   801C4E10: li      $a3, 2
=> 801C4E14: move    $a0, $s1
   801C4E18: li      $a1, 6
   801C4E1C: li      $a2, 1
   801C4E20: jal     sub_800201AC
   801C4E24: li      $a3, 2
   801C4E28: lw      $a0, dword_8006ED98+0x20
   801C4E30: move    $a1, $zero
   801C4E34: jal     sub_801C44E0
   801C4E38: addiu   $a0, 0x6C  # 'l'
   801C4E3C: move    $a1, $s1
   801C4E40: lw      $a0, dword_8006ED98+0x20
   801C4E48: move    $a2, $zero
   801C4E4C: jal     sub_801C455C
   801C4E50: addiu   $a0, 0x6C  # 'l'
   801C4E54: jal     sub_8001B120
   801C4E58: li      $a0, 1
   801C4E5C: move    $a0, $s1
   801C4E60: li      $a1, 5
   801C4E64: li      $a2, 1
   801C4E68: li      $a3, 2
   801C4E6C: li      $s2, 1
   801C4E70: jal     sub_800201AC
   801C4E74: sh      $s2, (word_800916D2 - 0x800916D2)($s0)
   801C4E78: lw      $a0, dword_8006ED98+0x20
   801C4E80: move    $a1, $s1
```

### Around `801C4E2C` in `sub_801C4DC4`
```asm
   801C4DFC: sh      $v0, (word_800916D2 - 0x800916D2)($s0)
   801C4E00: move    $a0, $s1
   801C4E04: li      $a1, 2
   801C4E08: li      $a2, 1
   801C4E0C: jal     sub_80020110
   801C4E10: li      $a3, 2
   801C4E14: move    $a0, $s1
   801C4E18: li      $a1, 6
   801C4E1C: li      $a2, 1
   801C4E20: jal     sub_800201AC
   801C4E24: li      $a3, 2
   801C4E28: lw      $a0, dword_8006ED98+0x20
   801C4E30: move    $a1, $zero
   801C4E34: jal     sub_801C44E0
   801C4E38: addiu   $a0, 0x6C  # 'l'
   801C4E3C: move    $a1, $s1
   801C4E40: lw      $a0, dword_8006ED98+0x20
   801C4E48: move    $a2, $zero
   801C4E4C: jal     sub_801C455C
   801C4E50: addiu   $a0, 0x6C  # 'l'
   801C4E54: jal     sub_8001B120
   801C4E58: li      $a0, 1
   801C4E5C: move    $a0, $s1
   801C4E60: li      $a1, 5
   801C4E64: li      $a2, 1
   801C4E68: li      $a3, 2
   801C4E6C: li      $s2, 1
   801C4E70: jal     sub_800201AC
   801C4E74: sh      $s2, (word_800916D2 - 0x800916D2)($s0)
   801C4E78: lw      $a0, dword_8006ED98+0x20
   801C4E80: move    $a1, $s1
   801C4E84: jal     sub_801C4894
   801C4E88: addiu   $a0, 0x9C
   801C4E8C: jal     sub_80026FA4
   801C4E90: move    $s0, $v0
   801C4E94: move    $a0, $s1
   801C4E98: li      $a1, 2
```

### Around `801C4648` in `sub_801C455C`
```asm
   801C4618: beqz    $s3, loc_801C4628
   801C461C: nop
   801C4620: jal     sub_8001ED3C
   801C4624: move    $a0, $s0
   801C4628: addiu   $s1, -1
   801C462C: bgtz    $s1, loc_801C45F0
   801C4630: nop
   801C4634: jal     sub_8001A280
   801C4638: move    $s3, $zero
   801C463C: lw      $v0, dword_8006ED98+0x20
   801C4644: nop
=> 801C4648: lw      $v0, 0x164($v0)
   801C464C: nop
   801C4650: sw      $v0, 0xC($s0)
   801C4654: lhu     $v0, word_800916D8
   801C465C: nop
   801C4660: sh      $v0, 0x66($s0)
   801C4664: lhu     $v0, word_800916DC
   801C466C: li      $s2, 1
   801C4670: sh      $v0, 0x68($s0)
   801C4674: sh      $v0, 0x54($s0)
   801C4678: sw      $zero, 0($s0)
   801C467C: jal     sub_80035510
   801C4680: li      $a0, 1
   801C4684: move    $v1, $v0
   801C4688: beqz    $v1, loc_801C46A8
   801C468C: li      $v0, 0x100
   801C4690: beq     $v1, $v0, loc_801C45A8
   801C4694: li      $v0, 0x800
   801C4698: beq     $v1, $v0, loc_801C472C
   801C469C: andi    $v0, $v1, 0x840
   801C46A0: bnez    $v0, loc_801C472C
   801C46A4: nop
   801C46A8: jal     sub_80024CF8
   801C46AC: move    $a0, $s0
   801C46B0: jal     sub_80027528
   801C46B4: nop
```



[console]
# IDA export: movie segment 801C4780 row scan to 801C455C (2026-05-14)

Authority: current active IDA database. Static evidence only; no Win file/ISO/playback-derived success authority.
Input file: `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`

## Function `801C4780` `sub_801C4780`
Bounds: `801C4780-801C47EC`

### Pseudocode
```c
int sub_801C4780()
{
  int v0; // $s1
  int v1; // $s0

  v0 = 0;
  v1 = 12;
  do
  {
    ++v0;
    sub_8001A324((_DWORD *)v1);
    v1 += 48;
  }
  while ( v0 < 7 );
  sub_80025A00();
  return sub_8001AC18((_DWORD *)0x3C, 0);
}
```

### Direct xrefs
- `801C4330` -> `801C4780` type=17 caller=801C4260 `sub_801C4260`: `jal     sub_801C4780`

### Disassembly
```asm
801C4780: addiu   $sp, -0x20
801C4784: sw      $s1, 0x10+var_s4($sp)
801C4788: move    $s1, $zero
801C478C: sw      $s0, 0x10+var_s0($sp)
801C4790: li      $s0, 0xC
801C4794: sw      $ra, 0x10+var_s8($sp)
801C4798: lw      $a0, dword_8006ED98+0x20
801C47A0: addiu   $s1, 1
801C47A4: jal     sub_8001A324
801C47A8: addu    $a0, $s0
801C47AC: slti    $v0, $s1, 7
801C47B0: bnez    $v0, loc_801C4798
801C47B4: addiu   $s0, 0x30  # '0'
801C47B8: jal     sub_80025A00
801C47BC: nop
801C47C0: lw      $a0, dword_8006ED98+0x20
801C47C8: move    $a1, $zero
801C47CC: jal     sub_8001AC18
801C47D0: addiu   $a0, 0x3C  # '<'
801C47D4: lw      $ra, 0x10+var_s8($sp)
801C47D8: lw      $s1, 0x10+var_s4($sp)
801C47DC: lw      $s0, 0x10+var_s0($sp)
801C47E0: addiu   $sp, 0x20
801C47E4: jr      $ra
801C47E8: nop
```

## Function `8001A324` `sub_8001A324`
Bounds: `8001A324-8001A3B8`

### Pseudocode
```c
int __fastcall sub_8001A324(_DWORD *a1)
{
  int result; // $v0
  bool v3; // dc
  int v4; // $a0
  unsigned int v5; // $v1

  if ( a1[3] == 1 || !*a1 )
    return 0;
  v3 = sub_8001A2B0(a1 + 4) == 0;
  result = -1;
  if ( !v3 )
  {
    v4 = sub_80036A78(a1 + 4);
    v5 = a1[5];
    result = 0;
    a1[10] = v4;
    a1[3] = 1;
    a1[11] = v4 + (v5 >> 11);
  }
  return result;
}
```

### Direct xrefs
- `80015E10` -> `8001A324` type=17 caller=80015D18 `sub_80015D18`: `jal     sub_8001A324`
- `8001AC64` -> `8001A324` type=17 caller=8001AC18 `sub_8001AC18`: `jal     sub_8001A324`
- `8001AD3C` -> `8001A324` type=17 caller=8001ACF8 `sub_8001ACF8`: `jal     sub_8001A324`
- `801C47A4` -> `8001A324` type=17 caller=801C4780 `sub_801C4780`: `jal     sub_8001A324`

### Disassembly
```asm
8001A324: addiu   $sp, -0x20
8001A328: sw      $s0, 0x10+var_s0($sp)
8001A32C: move    $s0, $a0
8001A330: sw      $ra, 0x10+var_sC($sp)
8001A334: sw      $s2, 0x10+var_s8($sp)
8001A338: sw      $s1, 0x10+var_s4($sp)
8001A33C: lw      $v0, 0xC($s0)
8001A340: li      $s2, 1
8001A344: beq     $v0, $s2, loc_8001A35C
8001A348: nop
8001A34C: lw      $a1, 0($s0)
8001A350: nop
8001A354: bnez    $a1, loc_8001A364
8001A358: addiu   $s1, $s0, 0x10
8001A35C: j       loc_8001A39C
8001A360: move    $v0, $zero
8001A364: jal     sub_8001A2B0
8001A368: move    $a0, $s1
8001A36C: beqz    $v0, loc_8001A39C
8001A370: li      $v0, 0xFFFFFFFF
8001A374: jal     sub_80036A78
8001A378: move    $a0, $s1
8001A37C: move    $a0, $v0
8001A380: lw      $v1, 0x14($s0)
8001A384: move    $v0, $zero
8001A388: sw      $a0, 0x28($s0)
8001A38C: sw      $s2, 0xC($s0)
8001A390: srl     $v1, 11
8001A394: addu    $a0, $v1
8001A398: sw      $a0, 0x2C($s0)
8001A39C: lw      $ra, 0x10+var_sC($sp)
8001A3A0: lw      $s2, 0x10+var_s8($sp)
8001A3A4: lw      $s1, 0x10+var_s4($sp)
8001A3A8: lw      $s0, 0x10+var_s0($sp)
8001A3AC: addiu   $sp, 0x20
8001A3B0: jr      $ra
8001A3B4: nop
```

## Function `8001A2B0` `sub_8001A2B0`
Bounds: `8001A2B0-8001A324`

### Pseudocode
```c
int __fastcall sub_8001A2B0(int a1, int a2)
{
  int v2; // $s2
  int i; // $s0

  v2 = a1;
  if ( !a2 )
    return 0;
  for ( i = 0; i < 4; ++i )
  {
    if ( sub_800381F8(a1, a2) )
      return v2;
    a1 = v2;
  }
  return 0;
}
```

### Direct xrefs
- `8001A364` -> `8001A2B0` type=17 caller=8001A324 `sub_8001A324`: `jal     sub_8001A2B0`

### Disassembly
```asm
8001A2B0: addiu   $sp, -0x20
8001A2B4: sw      $s2, 0x10+var_s8($sp)
8001A2B8: move    $s2, $a0
8001A2BC: sw      $s1, 0x10+var_s4($sp)
8001A2C0: move    $s1, $a1
8001A2C4: sw      $ra, 0x10+var_sC($sp)
8001A2C8: bnez    $s1, loc_8001A2E0
8001A2CC: sw      $s0, 0x10+var_s0($sp)
8001A2D0: j       loc_8001A308
8001A2D4: move    $v0, $zero
8001A2D8: j       loc_8001A308
8001A2DC: move    $v0, $s2
8001A2E0: move    $s0, $zero
8001A2E4: move    $a0, $s2
8001A2E8: jal     sub_800381F8
8001A2EC: move    $a1, $s1
8001A2F0: bnez    $v0, loc_8001A2D8
8001A2F4: addiu   $s0, 1
8001A2F8: slti    $v0, $s0, 4
8001A2FC: bnez    $v0, loc_8001A2E8
8001A300: move    $a0, $s2
8001A304: move    $v0, $zero
8001A308: lw      $ra, 0x10+var_sC($sp)
8001A30C: lw      $s2, 0x10+var_s8($sp)
8001A310: lw      $s1, 0x10+var_s4($sp)
8001A314: lw      $s0, 0x10+var_s0($sp)
8001A318: addiu   $sp, 0x20
8001A31C: jr      $ra
8001A320: nop
```

## Function `80036A78` `sub_80036A78`
Bounds: `80036A78-80036AF8`

### Pseudocode
```c
int __fastcall sub_80036A78(unsigned __int8 *a1)
{
  return 75 * (60 * (10 * (*a1 >> 4) + (*a1 & 0xF)) + 10 * (a1[1] >> 4) + (a1[1] & 0xF))
       + 10 * (a1[2] >> 4)
       + (a1[2] & 0xF)
       - 150;
}
```

### Direct xrefs
- `8001A374` -> `80036A78` type=17 caller=8001A324 `sub_8001A324`: `jal     sub_80036A78`
- `8001A520` -> `80036A78` type=17 caller=8001A4D0 `sub_8001A4D0`: `jal     sub_80036A78`
- `8001A7C0` -> `80036A78` type=17 caller=8001A7A4 `sub_8001A7A4`: `jal     sub_80036A78`
- `8001A7D4` -> `80036A78` type=17 caller=8001A7A4 `sub_8001A7A4`: `jal     sub_80036A78`
- `8001A8B0` -> `80036A78` type=17 caller=8001A89C `sub_8001A89C`: `jal     sub_80036A78`
- `80038C18` -> `80036A78` type=17 caller=80038BC4 `sub_80038BC4`: `jal     sub_80036A78`
- `80038F38` -> `80036A78` type=17 caller=80038DE8 `sub_80038DE8`: `jal     sub_80036A78`
- `800393CC` -> `80036A78` type=17 caller=800393A8 `sub_800393A8`: `jal     sub_80036A78`

### Disassembly
```asm
80036A78: lbu     $v1, 0($a0)
80036A7C: lbu     $a2, 1($a0)
80036A80: srl     $a1, $v1, 4
80036A84: sll     $v0, $a1, 2
80036A88: addu    $v0, $a1
80036A8C: sll     $v0, 1
80036A90: andi    $v1, 0xF
80036A94: addu    $v0, $v1
80036A98: sll     $a1, $v0, 4
80036A9C: subu    $a1, $v0
80036AA0: sll     $a1, 2
80036AA4: srl     $v1, $a2, 4
80036AA8: sll     $v0, $v1, 2
80036AAC: addu    $v0, $v1
80036AB0: sll     $v0, 1
80036AB4: andi    $a2, 0xF
80036AB8: addu    $v0, $a2
80036ABC: addu    $a1, $v0
80036AC0: sll     $v1, $a1, 2
80036AC4: addu    $v1, $a1
80036AC8: sll     $v0, $v1, 4
80036ACC: lbu     $a1, 2($a0)
80036AD0: subu    $v0, $v1
80036AD4: srl     $a0, $a1, 4
80036AD8: sll     $v1, $a0, 2
80036ADC: addu    $v1, $a0
80036AE0: sll     $v1, 1
80036AE4: andi    $a1, 0xF
80036AE8: addu    $v1, $a1
80036AEC: addu    $v0, $v1
80036AF0: jr      $ra
80036AF4: addiu   $v0, -0x96
```

## Function `801C4260` `sub_801C4260`
Bounds: `801C4260-801C4350`

### Pseudocode
```c
// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_801C4260(int a1, int a2, int a3, int a4, int a5, int a6, int (*a7)(void))
{
  int v8; // $v0
  int v9; // $a2
  int v10; // $a0

  sub_80025A34();
  sub_801C4FA0();
  sub_80024E98();
  sub_80014344();
  v8 = 96 * *(__int16 *)(a1 + 6);
  dword_8006ED98[8] = a1;
  v9 = *(__int16 *)(a1 + 10);
  *(_DWORD *)(a1 + 360) = 16;
  v10 = *(__int16 *)(a1 + 8);
  *(_DWORD *)(a1 + 348) = v8 / 100;
  *(_DWORD *)(a1 + 356) = v10 + v9;
  *(_DWORD *)(a1 + 352) = (v8 / 3600 + 50) / 100;
  return sub_801C4780();
}
```

### Direct xrefs
- `80048D2C` -> `801C4260` type=1 caller=<no function>: `.word sub_801C4260`

### Disassembly
```asm
801C4260: addiu   $sp, -0x20
801C4264: sw      $s0, 0x10+var_s0($sp)
801C4268: move    $s0, $a0
801C426C: sw      $s1, 0x10+var_s4($sp)
801C4270: sw      $ra, 0x10+var_s8($sp)
801C4274: jal     sub_80025A34
801C4278: move    $s1, $a1
801C427C: jal     sub_801C4FA0
801C4280: nop
801C4284: jal     sub_80024E98
801C4288: nop
801C428C: jal     sub_80014344
801C4290: nop
801C4294: lui     $a0, 0x91A2
801C4298: lh      $v1, 6($s0)
801C429C: li      $a0, 0x91A2B3C5
801C42A0: sll     $v0, $v1, 1
801C42A4: addu    $v0, $v1
801C42A8: sll     $v0, 5
801C42AC: mult    $v0, $a0
801C42B0: lui     $a0, 0x51EB
801C42B4: lh      $v1, 6($s0)
801C42B8: li      $a0, 0x51EB851F
801C42BC: sll     $a1, $v1, 1
801C42C0: mfhi    $a2
801C42C4: addu    $a1, $v1
801C42C8: sll     $a1, 5
801C42CC: mult    $a1, $a0
801C42D0: sw      $s0, dword_8006ED98+0x20
801C42D8: addu    $v1, $a2, $v0
801C42DC: sra     $v1, 11
801C42E0: sra     $v0, 31
801C42E4: subu    $v1, $v0
801C42E8: mfhi    $a3
801C42EC: addiu   $v1, 0x32  # '2'
801C42F0: li      $v0, 0x10
801C42F4: mult    $v1, $a0
801C42F8: lh      $a2, 0xA($s0)
801C42FC: sra     $a1, 31
801C4300: sw      $v0, 0x168($s0)
801C4304: sra     $v0, $a3, 5
801C4308: subu    $v0, $a1
801C430C: lh      $a0, 8($s0)
801C4310: sra     $v1, 31
801C4314: sw      $v0, 0x15C($s0)
801C4318: addu    $a0, $a2
801C431C: sw      $a0, 0x164($s0)
801C4320: move    $a0, $s1
801C4324: mfhi    $t1
801C4328: sra     $v0, $t1, 5
801C432C: subu    $v0, $v1
801C4330: jal     sub_801C4780
801C4334: sw      $v0, 0x160($s0)
801C4338: lw      $ra, 0x10+var_s8($sp)
801C433C: lw      $s1, 0x10+var_s4($sp)
801C4340: lw      $s0, 0x10+var_s0($sp)
801C4344: addiu   $sp, 0x20
801C4348: jr      $ra
801C434C: nop
```

## Function `801C4DC4` `sub_801C4DC4`
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

### Direct xrefs
- `80048D30` -> `801C4DC4` type=1 caller=<no function>: `.word sub_801C4DC4`

### Disassembly
```asm
801C4DC4: addiu   $sp, -0x28
801C4DC8: sw      $s3, 0x10+var_sC($sp)
801C4DCC: sw      $s0, 0x10+var_s0($sp)
801C4DD0: li      $s0, word_800916D2
801C4DD8: sw      $ra, 0x10+var_s10($sp)
801C4DDC: sw      $s2, 0x10+var_s8($sp)
801C4DE0: sw      $s1, 0x10+var_s4($sp)
801C4DE4: lh      $v0, (word_800916D2 - 0x800916D2)($s0)
801C4DE8: li      $s1, unk_801C3640
801C4DF0: bnez    $v0, loc_801C4E14
801C4DF4: move    $s3, $a0
801C4DF8: li      $v0, 1
801C4DFC: sh      $v0, (word_800916D2 - 0x800916D2)($s0)
801C4E00: move    $a0, $s1
801C4E04: li      $a1, 2
801C4E08: li      $a2, 1
801C4E0C: jal     sub_80020110
801C4E10: li      $a3, 2
801C4E14: move    $a0, $s1
801C4E18: li      $a1, 6
801C4E1C: li      $a2, 1
801C4E20: jal     sub_800201AC
801C4E24: li      $a3, 2
801C4E28: lw      $a0, dword_8006ED98+0x20
801C4E30: move    $a1, $zero
801C4E34: jal     sub_801C44E0
801C4E38: addiu   $a0, 0x6C  # 'l'
801C4E3C: move    $a1, $s1
801C4E40: lw      $a0, dword_8006ED98+0x20
801C4E48: move    $a2, $zero
801C4E4C: jal     sub_801C455C
801C4E50: addiu   $a0, 0x6C  # 'l'
801C4E54: jal     sub_8001B120
801C4E58: li      $a0, 1
801C4E5C: move    $a0, $s1
801C4E60: li      $a1, 5
801C4E64: li      $a2, 1
801C4E68: li      $a3, 2
801C4E6C: li      $s2, 1
801C4E70: jal     sub_800201AC
801C4E74: sh      $s2, (word_800916D2 - 0x800916D2)($s0)
801C4E78: lw      $a0, dword_8006ED98+0x20
801C4E80: move    $a1, $s1
801C4E84: jal     sub_801C4894
801C4E88: addiu   $a0, 0x9C
801C4E8C: jal     sub_80026FA4
801C4E90: move    $s0, $v0
801C4E94: move    $a0, $s1
801C4E98: li      $a1, 2
801C4E9C: li      $a2, 1
801C4EA0: jal     sub_80020110
801C4EA4: li      $a3, 2
801C4EA8: jal     sub_8001EF14
801C4EAC: nop
801C4EB0: li      $v0, 1
801C4EB4: bne     $s0, $v0, loc_801C4ECC
801C4EB8: li      $v0, 3
801C4EBC: sh      $zero, word_800916D0
801C4EC4: j       loc_801C4F48
801C4EC8: li      $v0, 0xFFFFFFFF
801C4ECC: bne     $s0, $v0, loc_801C4F24
801C4ED0: lui     $s0, 0x2AAA
801C4ED4: sh      $s2, word_800916D0
801C4EDC: li      $s0, 0x2AAAAAAB
801C4EE0: jal     rand
801C4EE4: nop
801C4EE8: mult    $v0, $s0
801C4EEC: sra     $a0, $v0, 31
801C4EF0: mfhi    $t0
801C4EF4: subu    $a0, $t0, $a0
801C4EF8: sll     $v1, $a0, 1
801C4EFC: addu    $v1, $a0
801C4F00: sll     $v1, 1
801C4F04: subu    $v0, $v1
801C4F08: lh      $v1, word_800916EE
801C4F10: addiu   $v0, 1
801C4F14: beq     $v0, $v1, loc_801C4EE0
801C4F18: nop
801C4F1C: j       loc_801C4F48
801C4F20: nop
801C4F24: lw      $a0, dword_80094410
801C4F2C: jal     sub_80026EF8
801C4F30: nop
801C4F34: jal     sub_80026ECC
801C4F38: nop
801C4F3C: addiu   $v0, $s3, 1
801C4F40: sh      $zero, word_800916D0
801C4F48: lw      $ra, 0x10+var_s10($sp)
801C4F4C: lw      $s3, 0x10+var_sC($sp)
801C4F50: lw      $s2, 0x10+var_s8($sp)
801C4F54: lw      $s1, 0x10+var_s4($sp)
801C4F58: lw      $s0, 0x10+var_s0($sp)
801C4F5C: addiu   $sp, 0x28
801C4F60: jr      $ra
801C4F64: nop
```

## Function `801C455C` `sub_801C455C`
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

### Direct xrefs
- `801C4E4C` -> `801C455C` type=17 caller=801C4DC4 `sub_801C4DC4`: `jal     sub_801C455C`

### Disassembly
```asm
801C455C: addiu   $sp, -0x30
801C4560: sw      $s5, 0x10+var_s14($sp)
801C4564: move    $s5, $a0
801C4568: sw      $s0, 0x10+var_s0($sp)
801C456C: move    $s0, $a1
801C4570: sltiu   $v0, $a2, 4
801C4574: sw      $ra, 0x10+var_s18($sp)
801C4578: sw      $s4, 0x10+var_s10($sp)
801C457C: sw      $s3, 0x10+var_sC($sp)
801C4580: sw      $s2, 0x10+var_s8($sp)
801C4584: beqz    $v0, loc_801C45C8
801C4588: sw      $s1, 0x10+var_s4($sp)
801C458C: sll     $a0, $a2, 3
801C4590: subu    $a0, $a2
801C4594: lw      $v0, dword_800943CC
801C459C: sll     $a0, 2
801C45A0: j       loc_801C45CC
801C45A4: addu    $a0, $v0, $a0
801C45A8: j       loc_801C472C
801C45AC: li      $s3, 1
801C45B0: jal     sub_8001A694
801C45B4: nop
801C45B8: jal     StopCallback
801C45BC: nop
801C45C0: j       loc_801C472C
801C45C4: nop
801C45C8: move    $a0, $zero
801C45CC: jal     sub_80024C84
801C45D0: li      $s1, 0x708
801C45D4: li      $s4, sub_8001EC54
801C45DC: li      $s3, sub_8001ED3C
801C45E4: sw      $zero, dword_801C954C
801C45EC: li      $s2, 1
801C45F0: jal     sub_8001A750
801C45F4: nop
801C45F8: beq     $v0, $s2, loc_801C4634
801C45FC: nop
801C4600: beqz    $s4, loc_801C4610
801C4604: move    $a0, $s0
801C4608: jal     sub_8001EC54
801C460C: li      $a1, 7
801C4610: jal     sub_80035560
801C4614: li      $a0, 2
801C4618: beqz    $s3, loc_801C4628
801C461C: nop
801C4620: jal     sub_8001ED3C
801C4624: move    $a0, $s0
801C4628: addiu   $s1, -1
801C462C: bgtz    $s1, loc_801C45F0
801C4630: nop
801C4634: jal     sub_8001A280
801C4638: move    $s3, $zero
801C463C: lw      $v0, dword_8006ED98+0x20
801C4644: nop
801C4648: lw      $v0, 0x164($v0)
801C464C: nop
801C4650: sw      $v0, 0xC($s0)
801C4654: lhu     $v0, word_800916D8
801C465C: nop
801C4660: sh      $v0, 0x66($s0)
801C4664: lhu     $v0, word_800916DC
801C466C: li      $s2, 1
801C4670: sh      $v0, 0x68($s0)
801C4674: sh      $v0, 0x54($s0)
801C4678: sw      $zero, 0($s0)
801C467C: jal     sub_80035510
801C4680: li      $a0, 1
801C4684: move    $v1, $v0
801C4688: beqz    $v1, loc_801C46A8
801C468C: li      $v0, 0x100
801C4690: beq     $v1, $v0, loc_801C45A8
801C4694: li      $v0, 0x800
801C4698: beq     $v1, $v0, loc_801C472C
801C469C: andi    $v0, $v1, 0x840
801C46A0: bnez    $v0, loc_801C472C
801C46A4: nop
801C46A8: jal     sub_80024CF8
801C46AC: move    $a0, $s0
801C46B0: jal     sub_80027528
801C46B4: nop
801C46B8: lw      $v0, dword_801C9554
801C46C0: nop
801C46C4: bne     $v0, $s2, loc_801C46D4
801C46C8: move    $a0, $s0
801C46CC: jal     sub_8001EC54
801C46D0: li      $a1, 7
801C46D4: jal     sub_80027528
801C46D8: nop
801C46DC: move    $a0, $s5
801C46E0: jal     sub_801C448C
801C46E4: move    $a1, $s0
801C46E8: jal     sub_8001ED74
801C46EC: move    $s1, $v0
801C46F0: jal     sub_8002756C
801C46F4: nop
801C46F8: lw      $v0, dword_801C9554
801C4700: nop
801C4704: bne     $v0, $s2, loc_801C4714
801C4708: nop
801C470C: jal     sub_8001ED3C
801C4710: move    $a0, $s0
801C4714: jal     GsGetWorkBase
801C4718: nop
801C471C: beq     $v0, $s2, loc_801C45B0
801C4720: nop
801C4724: beq     $s1, $s2, loc_801C4678
801C4728: nop
801C472C: jal     sub_80027664
801C4730: nop
801C4734: jal     sub_8001A4A4
801C4738: li      $a0, 1
801C473C: jal     sub_8001A694
801C4740: nop
801C4744: jal     nullsub_4
801C4748: move    $a0, $s0
801C474C: jal     sub_8001B120
801C4750: li      $a0, 1
801C4754: move    $v0, $s3
801C4758: lw      $ra, 0x10+var_s18($sp)
801C475C: lw      $s5, 0x10+var_s14($sp)
801C4760: lw      $s4, 0x10+var_s10($sp)
801C4764: lw      $s3, 0x10+var_sC($sp)
801C4768: lw      $s2, 0x10+var_s8($sp)
801C476C: lw      $s1, 0x10+var_s4($sp)
801C4770: lw      $s0, 0x10+var_s0($sp)
801C4774: addiu   $sp, 0x30
801C4778: jr      $ra
801C477C: nop
```

## Data xrefs to `8006EDB8` `sub_8006EDB8`
- `801C42D0` -> `8006EDB8` type=2 caller=801C4260 `sub_801C4260`: `sw      $s0, dword_8006ED98+0x20`
- `801C463C` -> `8006EDB8` type=3 caller=801C455C `sub_801C455C`: `lw      $v0, dword_8006ED98+0x20`
- `801C4798` -> `8006EDB8` type=3 caller=801C4780 `sub_801C4780`: `lw      $a0, dword_8006ED98+0x20`
- `801C47C0` -> `8006EDB8` type=3 caller=801C4780 `sub_801C4780`: `lw      $a0, dword_8006ED98+0x20`
- `801C49C0` -> `8006EDB8` type=3 caller=801C4894 `sub_801C4894`: `lw      $v0, dword_8006ED98+0x20`
- `801C4E28` -> `8006EDB8` type=3 caller=801C4DC4 `sub_801C4DC4`: `lw      $a0, dword_8006ED98+0x20`
- `801C4E40` -> `8006EDB8` type=3 caller=801C4DC4 `sub_801C4DC4`: `lw      $a0, dword_8006ED98+0x20`
- `801C4E78` -> `8006EDB8` type=3 caller=801C4DC4 `sub_801C4DC4`: `lw      $a0, dword_8006ED98+0x20`

## Data xrefs to `8006ED98` `dword_8006ED98`
- none

## Focus windows
### Around `801C42D0` in `sub_801C4260`
```asm
   801C42A0: sll     $v0, $v1, 1
   801C42A4: addu    $v0, $v1
   801C42A8: sll     $v0, 5
   801C42AC: mult    $v0, $a0
   801C42B0: lui     $a0, 0x51EB
   801C42B4: lh      $v1, 6($s0)
   801C42B8: li      $a0, 0x51EB851F
   801C42BC: sll     $a1, $v1, 1
   801C42C0: mfhi    $a2
   801C42C4: addu    $a1, $v1
   801C42C8: sll     $a1, 5
   801C42CC: mult    $a1, $a0
=> 801C42D0: sw      $s0, dword_8006ED98+0x20
   801C42D8: addu    $v1, $a2, $v0
   801C42DC: sra     $v1, 11
   801C42E0: sra     $v0, 31
   801C42E4: subu    $v1, $v0
   801C42E8: mfhi    $a3
   801C42EC: addiu   $v1, 0x32  # '2'
   801C42F0: li      $v0, 0x10
   801C42F4: mult    $v1, $a0
   801C42F8: lh      $a2, 0xA($s0)
   801C42FC: sra     $a1, 31
   801C4300: sw      $v0, 0x168($s0)
   801C4304: sra     $v0, $a3, 5
   801C4308: subu    $v0, $a1
   801C430C: lh      $a0, 8($s0)
   801C4310: sra     $v1, 31
   801C4314: sw      $v0, 0x15C($s0)
   801C4318: addu    $a0, $a2
   801C431C: sw      $a0, 0x164($s0)
   801C4320: move    $a0, $s1
   801C4324: mfhi    $t1
   801C4328: sra     $v0, $t1, 5
   801C432C: subu    $v0, $v1
   801C4330: jal     sub_801C4780
   801C4334: sw      $v0, 0x160($s0)
   801C4338: lw      $ra, 0x10+var_s8($sp)
   801C433C: lw      $s1, 0x10+var_s4($sp)
```

### Around `801C4780` in `sub_801C4780`
```asm
=> 801C4780: addiu   $sp, -0x20
   801C4784: sw      $s1, 0x10+var_s4($sp)
   801C4788: move    $s1, $zero
   801C478C: sw      $s0, 0x10+var_s0($sp)
   801C4790: li      $s0, 0xC
   801C4794: sw      $ra, 0x10+var_s8($sp)
   801C4798: lw      $a0, dword_8006ED98+0x20
   801C47A0: addiu   $s1, 1
   801C47A4: jal     sub_8001A324
   801C47A8: addu    $a0, $s0
   801C47AC: slti    $v0, $s1, 7
   801C47B0: bnez    $v0, loc_801C4798
   801C47B4: addiu   $s0, 0x30  # '0'
   801C47B8: jal     sub_80025A00
   801C47BC: nop
   801C47C0: lw      $a0, dword_8006ED98+0x20
   801C47C8: move    $a1, $zero
   801C47CC: jal     sub_8001AC18
   801C47D0: addiu   $a0, 0x3C  # '<'
   801C47D4: lw      $ra, 0x10+var_s8($sp)
   801C47D8: lw      $s1, 0x10+var_s4($sp)
   801C47DC: lw      $s0, 0x10+var_s0($sp)
   801C47E0: addiu   $sp, 0x20
   801C47E4: jr      $ra
   801C47E8: nop
```

### Around `801C47A4` in `sub_801C4780`
```asm
   801C4780: addiu   $sp, -0x20
   801C4784: sw      $s1, 0x10+var_s4($sp)
   801C4788: move    $s1, $zero
   801C478C: sw      $s0, 0x10+var_s0($sp)
   801C4790: li      $s0, 0xC
   801C4794: sw      $ra, 0x10+var_s8($sp)
   801C4798: lw      $a0, dword_8006ED98+0x20
   801C47A0: addiu   $s1, 1
=> 801C47A4: jal     sub_8001A324
   801C47A8: addu    $a0, $s0
   801C47AC: slti    $v0, $s1, 7
   801C47B0: bnez    $v0, loc_801C4798
   801C47B4: addiu   $s0, 0x30  # '0'
   801C47B8: jal     sub_80025A00
   801C47BC: nop
   801C47C0: lw      $a0, dword_8006ED98+0x20
   801C47C8: move    $a1, $zero
   801C47CC: jal     sub_8001AC18
   801C47D0: addiu   $a0, 0x3C  # '<'
   801C47D4: lw      $ra, 0x10+var_s8($sp)
   801C47D8: lw      $s1, 0x10+var_s4($sp)
   801C47DC: lw      $s0, 0x10+var_s0($sp)
   801C47E0: addiu   $sp, 0x20
   801C47E4: jr      $ra
   801C47E8: nop
```

### Around `801C47B8` in `sub_801C4780`
```asm
   801C4788: move    $s1, $zero
   801C478C: sw      $s0, 0x10+var_s0($sp)
   801C4790: li      $s0, 0xC
   801C4794: sw      $ra, 0x10+var_s8($sp)
   801C4798: lw      $a0, dword_8006ED98+0x20
   801C47A0: addiu   $s1, 1
   801C47A4: jal     sub_8001A324
   801C47A8: addu    $a0, $s0
   801C47AC: slti    $v0, $s1, 7
   801C47B0: bnez    $v0, loc_801C4798
   801C47B4: addiu   $s0, 0x30  # '0'
=> 801C47B8: jal     sub_80025A00
   801C47BC: nop
   801C47C0: lw      $a0, dword_8006ED98+0x20
   801C47C8: move    $a1, $zero
   801C47CC: jal     sub_8001AC18
   801C47D0: addiu   $a0, 0x3C  # '<'
   801C47D4: lw      $ra, 0x10+var_s8($sp)
   801C47D8: lw      $s1, 0x10+var_s4($sp)
   801C47DC: lw      $s0, 0x10+var_s0($sp)
   801C47E0: addiu   $sp, 0x20
   801C47E4: jr      $ra
   801C47E8: nop
```

### Around `801C47D0` in `sub_801C4780`
```asm
   801C47A0: addiu   $s1, 1
   801C47A4: jal     sub_8001A324
   801C47A8: addu    $a0, $s0
   801C47AC: slti    $v0, $s1, 7
   801C47B0: bnez    $v0, loc_801C4798
   801C47B4: addiu   $s0, 0x30  # '0'
   801C47B8: jal     sub_80025A00
   801C47BC: nop
   801C47C0: lw      $a0, dword_8006ED98+0x20
   801C47C8: move    $a1, $zero
   801C47CC: jal     sub_8001AC18
=> 801C47D0: addiu   $a0, 0x3C  # '<'
   801C47D4: lw      $ra, 0x10+var_s8($sp)
   801C47D8: lw      $s1, 0x10+var_s4($sp)
   801C47DC: lw      $s0, 0x10+var_s0($sp)
   801C47E0: addiu   $sp, 0x20
   801C47E4: jr      $ra
   801C47E8: nop
```

### Around `801C4E14` in `sub_801C4DC4`
```asm
   801C4DE4: lh      $v0, (word_800916D2 - 0x800916D2)($s0)
   801C4DE8: li      $s1, unk_801C3640
   801C4DF0: bnez    $v0, loc_801C4E14
   801C4DF4: move    $s3, $a0
   801C4DF8: li      $v0, 1
   801C4DFC: sh      $v0, (word_800916D2 - 0x800916D2)($s0)
   801C4E00: move    $a0, $s1
   801C4E04: li      $a1, 2
   801C4E08: li      $a2, 1
   801C4E0C: jal     sub_80020110
   801C4E10: li      $a3, 2
=> 801C4E14: move    $a0, $s1
   801C4E18: li      $a1, 6
   801C4E1C: li      $a2, 1
   801C4E20: jal     sub_800201AC
   801C4E24: li      $a3, 2
   801C4E28: lw      $a0, dword_8006ED98+0x20
   801C4E30: move    $a1, $zero
   801C4E34: jal     sub_801C44E0
   801C4E38: addiu   $a0, 0x6C  # 'l'
   801C4E3C: move    $a1, $s1
   801C4E40: lw      $a0, dword_8006ED98+0x20
   801C4E48: move    $a2, $zero
   801C4E4C: jal     sub_801C455C
   801C4E50: addiu   $a0, 0x6C  # 'l'
   801C4E54: jal     sub_8001B120
   801C4E58: li      $a0, 1
   801C4E5C: move    $a0, $s1
   801C4E60: li      $a1, 5
   801C4E64: li      $a2, 1
   801C4E68: li      $a3, 2
   801C4E6C: li      $s2, 1
   801C4E70: jal     sub_800201AC
   801C4E74: sh      $s2, (word_800916D2 - 0x800916D2)($s0)
   801C4E78: lw      $a0, dword_8006ED98+0x20
   801C4E80: move    $a1, $s1
```

### Around `801C4E2C` in `sub_801C4DC4`
```asm
   801C4DFC: sh      $v0, (word_800916D2 - 0x800916D2)($s0)
   801C4E00: move    $a0, $s1
   801C4E04: li      $a1, 2
   801C4E08: li      $a2, 1
   801C4E0C: jal     sub_80020110
   801C4E10: li      $a3, 2
   801C4E14: move    $a0, $s1
   801C4E18: li      $a1, 6
   801C4E1C: li      $a2, 1
   801C4E20: jal     sub_800201AC
   801C4E24: li      $a3, 2
   801C4E28: lw      $a0, dword_8006ED98+0x20
   801C4E30: move    $a1, $zero
   801C4E34: jal     sub_801C44E0
   801C4E38: addiu   $a0, 0x6C  # 'l'
   801C4E3C: move    $a1, $s1
   801C4E40: lw      $a0, dword_8006ED98+0x20
   801C4E48: move    $a2, $zero
   801C4E4C: jal     sub_801C455C
   801C4E50: addiu   $a0, 0x6C  # 'l'
   801C4E54: jal     sub_8001B120
   801C4E58: li      $a0, 1
   801C4E5C: move    $a0, $s1
   801C4E60: li      $a1, 5
   801C4E64: li      $a2, 1
   801C4E68: li      $a3, 2
   801C4E6C: li      $s2, 1
   801C4E70: jal     sub_800201AC
   801C4E74: sh      $s2, (word_800916D2 - 0x800916D2)($s0)
   801C4E78: lw      $a0, dword_8006ED98+0x20
   801C4E80: move    $a1, $s1
   801C4E84: jal     sub_801C4894
   801C4E88: addiu   $a0, 0x9C
   801C4E8C: jal     sub_80026FA4
   801C4E90: move    $s0, $v0
   801C4E94: move    $a0, $s1
   801C4E98: li      $a1, 2
```

### Around `801C4648` in `sub_801C455C`
```asm
   801C4618: beqz    $s3, loc_801C4628
   801C461C: nop
   801C4620: jal     sub_8001ED3C
   801C4624: move    $a0, $s0
   801C4628: addiu   $s1, -1
   801C462C: bgtz    $s1, loc_801C45F0
   801C4630: nop
   801C4634: jal     sub_8001A280
   801C4638: move    $s3, $zero
   801C463C: lw      $v0, dword_8006ED98+0x20
   801C4644: nop
=> 801C4648: lw      $v0, 0x164($v0)
   801C464C: nop
   801C4650: sw      $v0, 0xC($s0)
   801C4654: lhu     $v0, word_800916D8
   801C465C: nop
   801C4660: sh      $v0, 0x66($s0)
   801C4664: lhu     $v0, word_800916DC
   801C466C: li      $s2, 1
   801C4670: sh      $v0, 0x68($s0)
   801C4674: sh      $v0, 0x54($s0)
   801C4678: sw      $zero, 0($s0)
   801C467C: jal     sub_80035510
   801C4680: li      $a0, 1
   801C4684: move    $v1, $v0
   801C4688: beqz    $v1, loc_801C46A8
   801C468C: li      $v0, 0x100
   801C4690: beq     $v1, $v0, loc_801C45A8
   801C4694: li      $v0, 0x800
   801C4698: beq     $v1, $v0, loc_801C472C
   801C469C: andi    $v0, $v1, 0x840
   801C46A0: bnez    $v0, loc_801C472C
   801C46A4: nop
   801C46A8: jal     sub_80024CF8
   801C46AC: move    $a0, $s0
   801C46B0: jal     sub_80027528
   801C46B4: nop
```



[result]
null
