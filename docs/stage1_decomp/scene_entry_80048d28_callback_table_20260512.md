# 80048D28 scene callback table export (2026-05-12)

Authority: IDA data table + function pseudo C/disassembly from SCUS_941.83.

## Table

| scene | fn0 | fn0 name | fn1 | fn1 name | fn2 | fn2 name |
|---:|---:|---|---:|---|---:|---|
| 0 | `801C5B14` | `sub_801C5B14` | `801C4260` | `sub_801C4260` | `801C4DC4` | `sub_801C4DC4` |
| 1 | `801CA3BC` | `loc_801CA3BC` | `801C7284` | `loc_801C7284` | `801C81EC` | `loc_801C81EC` |
| 2 | `801C97EC` | `loc_801C97EC` | `801C657C` | `loc_801C657C` | `801C74E4` | `sub_801C74E4` |
| 3 | `801C998C` | `loc_801C998C` | `801C6918` | `loc_801C6918` | `801C7880` | `unk_801C7880` |
| 4 | `801CB348` | `loc_801CB348` | `801C7D20` | `unk_801C7D20` | `801C8C88` | `unk_801C8C88` |
| 5 | `801C9310` | `loc_801C9310` | `801C60B0` | `loc_801C60B0` | `801C7030` | `unk_801C7030` |
| 6 | `801C9EA0` | `loc_801C9EA0` | `801C6AC4` | `unk_801C6AC4` | `801C7A2C` | `unk_801C7A2C` |
| 7 | `801C5780` | `loc_801C5780` | `801C3870` | `dword_801C3870` | `801C4870` | `loc_801C4870` |
| 8 | `801C5BDC` | `loc_801C5BDC` | `801C3870` | `dword_801C3870` | `801C4834` | `loc_801C4834` |

## Functions

### 801C5B14 sub_801C5B14

#### Pseudo C
```c
int sub_801C5B14()
{
  dword_800943C0 = (int)&unk_801C6F88;
  dword_800943C4 = (int)&unk_801C6F68;
  dword_800943C8 = 1;
  dword_800943CC = (int)&off_801C6BF8;
  dword_800943D0 = (int)&unk_801C6DA4;
  dword_800943D8 = (int)&unk_801C6EF0;
  dword_800943E0 = (int)&unk_801C6F18;
  dword_800943E4 = (int)&dword_801C6F84;
  dword_800943FC = (int)&unk_801C6E88;
  dword_80094400 = (int)&unk_801C6E7C;
  dword_80094414 = (int)&unk_801C6EB4;
  dword_80094418 = (int)&unk_801C6EBC;
  dword_8009441C = (int)&unk_801C6EC4;
  dword_80094420 = (int)&unk_801C6ED0;
  dword_80094424 = (int)&unk_801C6ED8;
  dword_80094428 = (int)&unk_801C6EE0;
  dword_8009442C = (int)&unk_801C6EE8;
  dword_80094404 = (int)&unk_801C6E94;
  dword_80094408 = (int)&unk_801C6E9C;
  dword_8009440C = (int)&unk_801C6EA4;
  dword_80094410 = (int)&unk_801C6EAC;
  dword_800943EC = (int)&unk_801C6E5C;
  dword_800943F0 = (int)&unk_801C6E64;
  dword_800943F4 = (int)&unk_801C6E6C;
  dword_800943F8 = (int)&unk_801C6E74;
  dword_80094430 = (int)&nullsub_9;
  dword_80094434 = (int)&nullsub_11;
  dword_800943D4 = dword_801C6DD0;
  dword_800943DC = dword_801C6F14;
  dword_800943E8 = dword_801C6F84;
  dword_80094438 = (int (__fastcall *)(_DWORD))&nullsub_10;
  dword_8009443C = (int)&nullsub_12;
  dword_80094440 = (int)&sub_801C4FC8;
  return 0;
}

```

#### Disassembly
```asm
801C5B14: lw      $v1, dword_801C6DD0
801C5B1C: lw      $a0, dword_801C6F14
801C5B24: lw      $a1, dword_801C6F84
801C5B2C: li      $v0, unk_801C6F88
801C5B34: sw      $v0, dword_800943C0
801C5B3C: li      $v0, unk_801C6F68
801C5B44: sw      $v0, dword_800943C4
801C5B4C: li      $v0, 1
801C5B50: sw      $v0, dword_800943C8
801C5B58: li      $v0, off_801C6BF8
801C5B60: sw      $v0, dword_800943CC
801C5B68: li      $v0, unk_801C6DA4
801C5B70: sw      $v0, dword_800943D0
801C5B78: li      $v0, unk_801C6EF0
801C5B80: sw      $v0, dword_800943D8
801C5B88: li      $v0, unk_801C6F18
801C5B90: sw      $v0, dword_800943E0
801C5B98: li      $v0, dword_801C6F84
801C5BA0: sw      $v0, dword_800943E4
801C5BA8: li      $v0, unk_801C6E88
801C5BB0: sw      $v0, dword_800943FC
801C5BB8: li      $v0, unk_801C6E7C
801C5BC0: sw      $v0, dword_80094400
801C5BC8: li      $v0, unk_801C6EB4
801C5BD0: sw      $v0, dword_80094414
801C5BD8: lui     $v0, 0x801C
801C5BDC: li      $v0, unk_801C6EBC
801C5BE0: sw      $v0, dword_80094418
801C5BE8: li      $v0, unk_801C6EC4
801C5BF0: sw      $v0, dword_8009441C
801C5BF8: li      $v0, unk_801C6ED0
801C5C00: sw      $v0, dword_80094420
801C5C08: li      $v0, unk_801C6ED8
801C5C10: sw      $v0, dword_80094424
801C5C18: li      $v0, unk_801C6EE0
801C5C20: sw      $v0, dword_80094428
801C5C28: li      $v0, unk_801C6EE8
801C5C30: sw      $v0, dword_8009442C
801C5C38: li      $v0, unk_801C6E94
801C5C40: sw      $v0, dword_80094404
801C5C48: li      $v0, unk_801C6E9C
801C5C50: sw      $v0, dword_80094408
801C5C58: li      $v0, unk_801C6EA4
801C5C60: sw      $v0, dword_8009440C
801C5C68: li      $v0, unk_801C6EAC
801C5C70: sw      $v0, dword_80094410
801C5C78: li      $v0, unk_801C6E5C
801C5C80: sw      $v0, dword_800943EC
801C5C88: li      $v0, unk_801C6E64
801C5C90: sw      $v0, dword_800943F0
801C5C98: li      $v0, unk_801C6E6C
801C5CA0: sw      $v0, dword_800943F4
801C5CA8: li      $v0, unk_801C6E74
801C5CB0: sw      $v0, dword_800943F8
801C5CB8: li      $v0, nullsub_9
801C5CC0: sw      $v0, dword_80094430
801C5CC8: li      $v0, nullsub_11
801C5CD0: sw      $v0, dword_80094434
801C5CD8: li      $v0, nullsub_10
801C5CE0: sw      $v1, dword_800943D4
801C5CE8: sw      $a0, dword_800943DC
801C5CF0: sw      $a1, dword_800943E8
801C5CF8: sw      $v0, dword_80094438
801C5D00: li      $v0, nullsub_12
801C5D08: sw      $v0, dword_8009443C
801C5D10: li      $v0, sub_801C4FC8
801C5D18: sw      $v0, dword_80094440
801C5D20: jr      $ra
801C5D24: move    $v0, $zero
```

### 801C4260 sub_801C4260

#### Pseudo C
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

#### Disassembly
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

### 801C4DC4 sub_801C4DC4

#### Pseudo C
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

#### Disassembly
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

### 801C657C sub_801C6410

#### Pseudo C
```c
// write access to const memory has been detected, the output may be wrong!
int __fastcall sub_801C6410(_DWORD *a1)
{
  int v2; // $v0
  int v3; // $a0
  int v4; // $a1
  int v5; // $a3
  int v6; // $s2
  int v7; // $a1
  int v8; // $a1
  int result; // $v0
  int v10; // $a2
  int v11; // $s2
  _DWORD *v12; // $s1
  int v13; // $s0

  v2 = sub_8004019C();
  v3 = dword_801C956C[v2];
  dword_8006ED98[4] = v2;
  sub_80040F90(v3);
  sub_80040CC8(0, 0, &dword_801C9574);
  sub_8001E374(0);
  if ( (*a1 & 0x20) != 0 )
    dword_801C9548 = 1;
  v5 = 0;
  if ( (*a1 & 0x4000000) != 0 )
  {
    dword_801CFAEC = 1;
    dword_801C9548 = 0;
    sub_800140E0(dword_800918B0, dword_8009188C, 0, 0, 999);
    sub_80014050(2, dword_800918B0, dword_8009188C, &loc_801CF81C, &loc_801CF82C, &dword_801C9540);
    dword_801CB59C = 299;
    sub_80014050(1, dword_800918A8, dword_80091884, &loc_801CC66C, &unk_801CC67C, &dword_801C9538);
    sub_80014050(3, dword_800918A0, dword_8009187C, &unk_801CD71C, &unk_801CD72C, &dword_801C953C);
    v6 = 0;
    if ( word_801CC676 )
    {
      do
      {
        v7 = dword_801C9538++;
        sub_80013EA8(&loc_801CC66C, v7, 1);
        v8 = dword_801C953C++;
        sub_80013EA8(&unk_801CD71C, v8, 3);
        ++v6;
      }
      while ( v6 < (unsigned __int16)word_801CC676 );
    }
  }
  if ( (*a1 & 0x8000000) != 0 )
  {
    dword_801CFAEC = 0;
    dword_801CB600 = 1;
    dword_801C9548 = 1;
  }
  result = dword_801C9548;
  if ( dword_801C9548 )
  {
    if ( (*a1 & 0x400000) != 0 )
    {
      sub_80014050(1, a1[57], a1[60], &loc_801CC66C, &unk_801CC67C, &dword_801C9538);
      dword_801CB600 = 1;
    }
    if ( (*a1 & 0x1000000) != 0 )
      sub_80014050(3, a1[70], a1[69], &unk_801CD71C, &unk_801CD72C, &dword_801C953C);
    if ( (*a1 & 0x40000) != 0 )
    {
      sub_8001AFD8(a1[61], &loc_801C9534, &unk_801CB658, v5);
      dword_801CB604 = 0;
    }
    v10 = 0;
    if ( (*a1 & 0x10000) != 0 )
    {
      sub_800140E0(a1[55], a1[58], 0, 0, 999);
      sub_80014050(2, a1[55], a1[58], &loc_801CF81C, &loc_801CF82C, &dword_801C9540);
    }
    if ( (*a1 & 0x8000) != 0 )
    {
      v11 = 0;
      v12 = a1;
      do
      {
        if ( *(_WORD *)v12[43] )
        {
          v13 = 0;
          do
          {
            sub_8001ADEC(dword_80091858[*(__int16 *)(v13 + v12[43])], 1, v10, v5);
            v13 += 2;
          }
          while ( *(_WORD *)(v13 + v12[43]) );
        }
        ++v11;
        ++v12;
      }
      while ( v11 <= 0 );
    }
    if ( (*a1 & 0x400) != 0 )
      dword_801CB59C = 0;
    dword_801CB59C = 1;
    sub_80041D3C(&unk_801C6FB4, v4, v10, v5);
    return sub_801C5EF0();
  }
  return result;
}

```

#### Disassembly
```asm
801C6410: addiu   $sp, -0x38
801C6414: sw      $s3, 0x20+var_sC($sp)
801C6418: move    $s3, $a0
801C641C: sw      $ra, 0x20+var_s14($sp)
801C6420: sw      $s4, 0x20+var_s10($sp)
801C6424: sw      $s2, 0x20+var_s8($sp)
801C6428: sw      $s1, 0x20+var_s4($sp)
801C642C: jal     sub_8004019C
801C6430: sw      $s0, 0x20+var_s0($sp)
801C6434: sll     $v1, $v0, 2
801C6438: li      $at, dword_801C956C
801C6440: addu    $at, $v1
801C6444: lw      $a0, 0($at)
801C6448: sw      $v0, dword_8006ED98+0x10
801C6450: jal     sub_80040F90
801C6454: nop
801C6458: move    $a0, $zero
801C645C: lw      $v0, dword_8006ED98+0x10
801C6464: move    $a1, $zero
801C6468: sll     $a2, $v0, 2
801C646C: addu    $a2, $v0
801C6470: sll     $a2, 2
801C6474: li      $v0, dword_801C9574
801C647C: jal     sub_80040CC8
801C6480: addu    $a2, $v0
801C6484: lw      $a0, dword_8006ED98+0x10
801C648C: jal     sub_8001E374
801C6490: nop
801C6494: lw      $v0, 0($s3)
801C6498: nop
801C649C: andi    $v0, 0x20
801C64A0: beqz    $v0, loc_801C64B0
801C64A4: li      $v0, 1
801C64A8: sw      $v0, dword_801C9548
801C64B0: lw      $v0, 0($s3)
801C64B4: lui     $v1, 0x400
801C64B8: and     $v0, $v1
801C64BC: beqz    $v0, loc_801C6628
801C64C0: move    $a3, $zero
801C64C4: move    $a2, $zero
801C64C8: li      $s0, dword_800918B0
801C64D0: lw      $a0, (dword_800918B0 - 0x800918B0)($s0)
801C64D4: lw      $a1, dword_8009188C
801C64DC: li      $v0, 1
801C64E0: sw      $v0, dword_801CFAEC
801C64E8: li      $v0, 0x3E7
801C64EC: sw      $zero, dword_801C9548
801C64F4: jal     sub_800140E0
801C64F8: sw      $v0, 0x20+var_10($sp)
801C64FC: li      $a0, 2
801C6500: li      $a3, loc_801CF81C
801C6508: lw      $a1, (dword_800918B0 - 0x800918B0)($s0)
801C650C: lw      $a2, dword_8009188C
801C6514: li      $v0, loc_801CF82C
801C651C: sw      $v0, 0x20+var_10($sp)
801C6520: li      $v0, dword_801C9540
801C6528: jal     sub_80014050
801C652C: sw      $v0, 0x20+var_C($sp)
801C6530: li      $a0, 1
801C6534: li      $s0, loc_801CC66C
801C653C: move    $a3, $s0
801C6540: lw      $a1, dword_800918A8
801C6548: lw      $a2, dword_80091884
801C6550: li      $v0, 0x12B
801C6554: sw      $v0, dword_801CB59C
801C655C: li      $v0, unk_801CC67C
801C6564: sw      $v0, 0x20+var_10($sp)
801C6568: li      $v0, dword_801C9538
801C6570: jal     sub_80014050
801C6574: sw      $v0, 0x20+var_C($sp)
801C6578: li      $a0, 3
801C657C: li      $a3, unk_801CD71C
801C6584: lw      $a1, dword_800918A0
801C658C: lw      $a2, dword_8009187C
801C6594: li      $v0, unk_801CD72C
801C659C: sw      $v0, 0x20+var_10($sp)
801C65A0: li      $v0, dword_801C953C
801C65A8: jal     sub_80014050
801C65AC: sw      $v0, 0x20+var_C($sp)
801C65B0: lhu     $v0, word_801CC676
801C65B8: nop
801C65BC: blez    $v0, loc_801C6628
801C65C0: move    $s2, $zero
801C65C4: move    $a0, $s0
801C65C8: lw      $a1, dword_801C9538
801C65D0: nop
801C65D4: addiu   $v0, $a1, 1
801C65D8: sw      $v0, dword_801C9538
801C65E0: jal     sub_80013EA8
801C65E4: li      $a2, 1
801C65E8: li      $a0, unk_801CD71C
801C65F0: lw      $a1, dword_801C953C
801C65F8: nop
801C65FC: addiu   $v0, $a1, 1
801C6600: sw      $v0, dword_801C953C
801C6608: jal     sub_80013EA8
801C660C: li      $a2, 3
801C6610: lhu     $v0, word_801CC676
801C6618: addiu   $s2, 1
801C661C: slt     $v0, $s2, $v0
801C6620: bnez    $v0, loc_801C65C8
801C6624: move    $a0, $s0
801C6628: lw      $v0, 0($s3)
801C662C: lui     $v1, 0x800
801C6630: and     $v0, $v1
801C6634: beqz    $v0, loc_801C6654
801C6638: li      $v0, 1
801C663C: sw      $zero, dword_801CFAEC
801C6644: sw      $v0, dword_801CB600
801C664C: sw      $v0, dword_801C9548
801C6654: lw      $v0, dword_801C9548
801C665C: nop
801C6660: beqz    $v0, loc_801C6878
801C6664: lui     $v1, 0x40  # '@'
801C6668: lw      $v0, 0($s3)
801C666C: nop
801C6670: and     $v0, $v1
801C6674: beqz    $v0, loc_801C66B8
801C6678: nop
801C667C: li      $v0, unk_801CC67C
801C6684: sw      $v0, 0x20+var_10($sp)
801C6688: li      $v0, dword_801C9538
801C6690: sw      $v0, 0x20+var_C($sp)
801C6694: lw      $a1, 0xE4($s3)
801C6698: lw      $a2, 0xF0($s3)
801C669C: li      $a3, loc_801CC66C
801C66A4: jal     sub_80014050
801C66A8: li      $a0, 1
801C66AC: li      $v0, 1
801C66B0: sw      $v0, dword_801CB600
801C66B8: lw      $v0, 0($s3)
801C66BC: lui     $v1, 0x100
801C66C0: and     $v0, $v1
801C66C4: beqz    $v0, loc_801C66FC
801C66C8: nop
801C66CC: li      $v0, unk_801CD72C
801C66D4: sw      $v0, 0x20+var_10($sp)
801C66D8: li      $v0, dword_801C953C
801C66E0: sw      $v0, 0x20+var_C($sp)
801C66E4: lw      $a1, 0x118($s3)
801C66E8: lw      $a2, 0x114($s3)
801C66EC: li      $a3, unk_801CD71C
801C66F4: jal     sub_80014050
801C66F8: li      $a0, 3
801C66FC: lw      $v0, 0($s3)
801C6700: lui     $v1, 4
801C6704: and     $v0, $v1
801C6708: beqz    $v0, loc_801C6734
801C670C: nop
801C6710: li      $a1, loc_801C9534
801C6718: lw      $a0, 0xF4($s3)
801C671C: li      $a2, unk_801CB658
801C6724: jal     sub_8001AFD8
801C6728: nop
801C672C: sw      $zero, dword_801CB604
801C6734: lw      $v0, 0($s3)
801C6738: lui     $v1, 1
801C673C: and     $v0, $v1
801C6740: beqz    $v0, loc_801C6790
801C6744: move    $a2, $zero
801C6748: li      $v0, 0x3E7
801C674C: sw      $v0, 0x20+var_10($sp)
801C6750: lw      $a0, 0xDC($s3)
801C6754: lw      $a1, 0xE8($s3)
801C6758: jal     sub_800140E0
801C675C: move    $a3, $zero
801C6760: li      $v0, loc_801CF82C
801C6768: sw      $v0, 0x20+var_10($sp)
801C676C: li      $v0, dword_801C9540
801C6774: sw      $v0, 0x20+var_C($sp)
801C6778: lw      $a1, 0xDC($s3)
801C677C: lw      $a2, 0xE8($s3)
801C6780: li      $a3, loc_801CF81C
801C6788: jal     sub_80014050
801C678C: li      $a0, 2
801C6790: lw      $v0, 0($s3)
801C6794: nop
801C6798: andi    $v0, 0x8000
801C679C: beqz    $v0, loc_801C6820
801C67A0: nop
801C67A4: move    $s2, $zero
801C67A8: li      $s4, dword_80091858
801C67B0: move    $s1, $s3
801C67B4: lw      $v0, 0xAC($s1)
801C67B8: nop
801C67BC: lh      $v0, 0($v0)
801C67C0: nop
801C67C4: beqz    $v0, loc_801C6814
801C67C8: nop
801C67CC: move    $s0, $zero
801C67D0: lw      $v0, 0xAC($s1)
801C67D4: nop
801C67D8: addu    $v0, $s0, $v0
801C67DC: lh      $v0, 0($v0)
801C67E0: nop
801C67E4: sll     $v0, 2
801C67E8: addu    $v0, $s4
801C67EC: lw      $a0, 0($v0)
801C67F0: jal     sub_8001ADEC
801C67F4: li      $a1, 1
801C67F8: lw      $v0, 0xAC($s1)
801C67FC: addiu   $s0, 2
801C6800: addu    $v0, $s0, $v0
801C6804: lh      $v0, 0($v0)
801C6808: nop
801C680C: bnez    $v0, loc_801C67D0
801C6810: nop
801C6814: addiu   $s2, 1
801C6818: blez    $s2, loc_801C67B4
801C681C: addiu   $s1, 4
801C6820: lw      $v0, 0($s3)
801C6824: nop
801C6828: andi    $v0, 0x400
801C682C: beqz    $v0, loc_801C683C
801C6830: nop
801C6834: sw      $zero, dword_801CB59C
801C683C: lw      $v1, dword_801CB59C
801C6844: nop
801C6848: sltiu   $v0, $v1, 0x12C
801C684C: beqz    $v0, loc_801C6870
801C6850: addiu   $v0, $v1, 1
801C6854: sw      $v0, dword_801CB59C
801C685C: sll     $v0, $v1, 5
801C6860: li      $a0, unk_801C6FB4
801C6868: jal     sub_80041D3C
801C686C: addu    $a0, $v0, $a0
801C6870: jal     sub_801C5EF0
801C6874: nop
801C6878: lw      $ra, 0x20+var_s14($sp)
801C687C: lw      $s4, 0x20+var_s10($sp)
801C6880: lw      $s3, 0x20+var_sC($sp)
801C6884: lw      $s2, 0x20+var_s8($sp)
801C6888: lw      $s1, 0x20+var_s4($sp)
801C688C: lw      $s0, 0x20+var_s0($sp)
801C6890: addiu   $sp, 0x38
801C6894: jr      $ra
801C6898: nop
```

### 801C74E4 sub_801C74E4

#### Pseudo C
```c
void __noreturn sub_801C74E4()
{
  __asm
  {
    sd      $ra, -0x578($ra)
    sd      $ra, -0xBB2($ra)
    sd      $ra, -0x15DD($ra)
    sd      $ra, -0x578($ra)
    sd      $ra, -0xBD7($ra)
    sd      $ra, -0x156F($ra)
    sd      $ra, -0x578($ra)
    sd      $ra, -0xBFC($ra)
    sd      $ra, -0x1500($ra)
    sd      $ra, -0x578($ra)
    sd      $ra, -0xC21($ra)
    sd      $ra, -0x1490($ra)
    sd      $ra, -0x578($ra)
    sd      $ra, -0xC46($ra)
    sd      $ra, -0x141F($ra)
    sd      $ra, -0x578($ra)
    ddiv    $zero, $zero
    sd      $ra, -0xC6C($ra)
    sd      $ra, -0x13AD($ra)
    sd      $ra, -0x578($ra)
    sd      $ra, -0xC91($ra)
    sd      $ra, -0x1339($ra)
    sd      $ra, -0x578($ra)
    sd      $ra, -0xCB6($ra)
    sd      $ra, -0x12C5($ra)
    sd      $ra, -0x578($ra)
    sd      $ra, -0xCDB($ra)
    sd      $ra, -0x124F($ra)
    sd      $ra, -0x578($ra)
    sd      $ra, -0xD01($ra)
    sd      $ra, -0x11D8($ra)
    sd      $ra, -0x578($ra)
  }
  __trap(0x3Cu);
}

```

#### Disassembly
```asm
801C74E4: sd      $ra, -0x578($ra)
801C74E8: nop
801C74EC: nop
801C74F0: nop
801C74F4: move    $at, $zero
801C74F8: sd      $ra, -0xBB2($ra)
801C74FC: sd      $ra, -0x15DD($ra)
801C7500: nop
801C7504: sd      $ra, -0x578($ra)
801C7508: nop
801C750C: nop
801C7510: nop
801C7514: and     $at, $zero, $zero
801C7518: sd      $ra, -0xBD7($ra)
801C751C: sd      $ra, -0x156F($ra)
801C7520: nop
801C7524: sd      $ra, -0x578($ra)
801C7528: nop
801C752C: nop
801C7530: nop
801C7534: move    $at, $zero
801C7538: sd      $ra, -0xBFC($ra)
801C753C: sd      $ra, -0x1500($ra)
801C7540: nop
801C7544: sd      $ra, -0x578($ra)
801C7548: nop
801C754C: nop
801C7550: nop
801C7554: and     $at, $zero, $zero
801C7558: sd      $ra, -0xC21($ra)
801C755C: sd      $ra, -0x1490($ra)
801C7560: nop
801C7564: sd      $ra, -0x578($ra)
801C7568: nop
801C756C: nop
801C7570: nop
801C7574: move    $at, $zero
801C7578: sd      $ra, -0xC46($ra)
801C757C: sd      $ra, -0x141F($ra)
801C7580: nop
801C7584: sd      $ra, -0x578($ra)
801C7588: nop
801C758C: nop
801C7590: nop
801C7594: ddiv    $zero, $zero
801C7598: sd      $ra, -0xC6C($ra)
801C759C: sd      $ra, -0x13AD($ra)
801C75A0: nop
801C75A4: sd      $ra, -0x578($ra)
801C75A8: nop
801C75AC: nop
801C75B0: nop
801C75B4: multu   $ac1, $zero, $zero
801C75B8: sd      $ra, -0xC91($ra)
801C75BC: sd      $ra, -0x1339($ra)
801C75C0: nop
801C75C4: sd      $ra, -0x578($ra)
801C75C8: nop
801C75CC: nop
801C75D0: nop
801C75D4: mflo    $at
801C75D8: sd      $ra, -0xCB6($ra)
801C75DC: sd      $ra, -0x12C5($ra)
801C75E0: nop
801C75E4: sd      $ra, -0x578($ra)
801C75E8: nop
801C75EC: nop
801C75F0: nop
801C75F4: movz    $at, $zero, $zero
801C75F8: sd      $ra, -0xCDB($ra)
801C75FC: sd      $ra, -0x124F($ra)
801C7600: nop
801C7604: sd      $ra, -0x578($ra)
801C7608: nop
801C760C: nop
801C7610: nop
801C7614: sll     $at, $zero, 28
801C7618: sd      $ra, -0xD01($ra)
801C761C: sd      $ra, -0x11D8($ra)
801C7620: nop
801C7624: sd      $ra, -0x578($ra)
801C7628: nop
801C762C: nop
801C7630: nop
801C7634: teq     $zero, $zero  #0x3C
801C7638: sd      $ra, -0xD26($ra)
801C763C: sd      $ra, -0x115F($ra)
801C7640: nop
801C7644: sd      $ra, -0x578($ra)
801C7648: nop
801C764C: nop
801C7650: nop
801C7654: nor     $at, $zero, $zero
801C7658: sd      $ra, -0xD4B($ra)
801C765C: sd      $ra, -0x10E6($ra)
801C7660: nop
801C7664: sd      $ra, -0x578($ra)
801C7668: nop
801C766C: nop
801C7670: nop
801C7674: dsrav   $at, $zero, $zero
801C7678: sd      $ra, -0xD70($ra)
801C767C: sd      $ra, -0x106B($ra)
801C7680: nop
801C7684: sd      $ra, -0x578($ra)
801C7688: nop
801C768C: nop
801C7690: nop
801C7694: rotrv   $at, $zero, $zero
801C7698: sd      $ra, -0xD96($ra)
801C769C: sd      $ra, -0xFEE($ra)
801C76A0: nop
801C76A4: sd      $ra, -0x578($ra)
801C76A8: nop
801C76AC: nop
801C76B0: nop
801C76B4: tltu    $zero, $zero  #0x3F
801C76B8: sd      $ra, -0xDBB($ra)
801C76BC: sd      $ra, -0xF70($ra)
801C76C0: nop
801C76C4: sd      $ra, -0x578($ra)
801C76C8: nop
801C76CC: nop
801C76D0: nop
801C76D4: ddivu   $zero, $zero
801C76D8: sd      $ra, -0xDE0($ra)
801C76DC: sd      $ra, -0xEF1($ra)
801C76E0: nop
801C76E4: sd      $ra, -0x578($ra)
801C76E8: nop
801C76EC: nop
801C76F0: nop
801C76F4: jr      $zero  # unk_80000000
801C76F8: sd      $ra, -0xE06($ra)
```

### 801C6918 sub_801C689C

#### Pseudo C
```c
int sub_801C689C()
{
  int result; // $v0

  sub_80040370();
  sub_80040060(255, 255, 255, &dword_801C9574);
  sub_80040CA4((int)&dword_801C9574);
  sub_8001E3B0(0);
  result = dword_801CFAEC;
  if ( dword_801CFAEC )
  {
    sub_80040370();
    return sub_8001B1B0(255, 255, 255);
  }
  return result;
}

```

#### Disassembly
```asm
801C689C: addiu   $sp, -0x18
801C68A0: sw      $ra, 0x10+var_s4($sp)
801C68A4: jal     sub_80040370
801C68A8: sw      $s0, 0x10+var_s0($sp)
801C68AC: li      $a0, 0xFF
801C68B0: li      $a1, 0xFF
801C68B4: li      $a2, 0xFF
801C68B8: lw      $v0, dword_8006ED98+0x10
801C68C0: li      $s0, dword_801C9574
801C68C8: sll     $a3, $v0, 2
801C68CC: addu    $a3, $v0
801C68D0: sll     $a3, 2
801C68D4: jal     sub_80040060
801C68D8: addu    $a3, $s0
801C68DC: lw      $v0, dword_8006ED98+0x10
801C68E4: nop
801C68E8: sll     $a0, $v0, 2
801C68EC: addu    $a0, $v0
801C68F0: sll     $a0, 2
801C68F4: jal     sub_80040CA4
801C68F8: addu    $a0, $s0
801C68FC: lw      $a0, dword_8006ED98+0x10
801C6904: jal     sub_8001E3B0
801C6908: nop
801C690C: lw      $v0, dword_801CFAEC
801C6914: nop
801C6918: beqz    $v0, loc_801C6938
801C691C: nop
801C6920: jal     sub_80040370
801C6924: nop
801C6928: li      $a0, 0xFF
801C692C: li      $a1, 0xFF
801C6930: jal     sub_8001B1B0
801C6934: li      $a2, 0xFF
801C6938: lw      $ra, 0x10+var_s4($sp)
801C693C: lw      $s0, 0x10+var_s0($sp)
801C6940: addiu   $sp, 0x18
801C6944: jr      $ra
801C6948: nop
```

### 801C60B0 sub_801C609C

#### Pseudo C
```c
// write access to const memory has been detected, the output may be wrong!
int sub_801C609C()
{
  int v0; // $a1
  char *v1; // $a0
  int v2; // $v1
  int v3; // $a2
  int v4; // $a2
  int v5; // $a2
  int v6; // $a2
  int v7; // $a3

  sub_801C5D28();
  sub_8001E33C(dword_801C956C[0], dword_801C9570);
  v0 = 0;
  v1 = (char *)&unk_801C959C;
  v2 = 0;
  do
  {
    *(_DWORD *)((char *)&unk_801C9578 + v2 * 4) = v1;
    v1 += 4096;
    dword_801C9574[v2] = 10;
    *(_DWORD *)((char *)&unk_801C957C + v2 * 4) = 0;
    ++v0;
    v2 += 5;
  }
  while ( v0 < 2 );
  dword_801CB600 = 0;
  dword_801CFAEC = 0;
  dword_8006ED98[13] = 1;
  dword_801C9548 = 0;
  dword_801CB59C = 0;
  sub_8004049C(0, &unk_801CB608, 10);
  sub_8004049C(0, &unk_801CB5A0, v3);
  sub_8004049C(0, &unk_801CD76C, v4);
  sub_8004049C(0, &unk_801CC6BC, v5);
  sub_8004049C(0, &dword_801CD7BC, v6);
  sub_801C5D8C(-163, 204, 192);
  dword_801CC6D8 = -100;
  if ( dword_801C9544 )
  {
    sub_8001AF1C(dword_80091874, &unk_801CB65C, &unk_801CB608);
    sub_8001AF1C(dword_80091870, &unk_801CB5F0, &unk_801CB5A0);
    sub_8001AF1C(dword_8009186C, &unk_801CC70C, &unk_801CC6BC);
    sub_8001AF1C(dword_80091878, &loc_801CD80C, &unk_801CD76C);
    dword_801C9544 = 0;
  }
  else
  {
    sub_800139F8(0);
    sub_800139F8(1);
    sub_800139F8(2);
    sub_800139F8(3);
  }
  sub_8001EEE8();
  GsSetProjection(440);
  dword_801CB604 = 0;
  sub_8001AFD8(dword_80091868, &loc_801C9534, &unk_801CB658, v7);
  sub_8001B000(dword_801CB604, &unk_801CB658, &loc_801C9534, &unk_801CB65C);
  sub_80014164(dword_800918A4, dword_80091880, dword_80091874, 1);
  sub_8001385C(1, dword_80091870, dword_80091880, &unk_801CB66C);
  sub_80013D10(&loc_801CC66C, &unk_801CC67C, dword_800918A4, 0);
  sub_8001385C(2, dword_80091878, dword_80091890, &loc_801CD81C);
  sub_80013D10(&loc_801CF81C, &loc_801CF82C, dword_800918B4, 0);
  sub_8001385C(3, dword_8009186C, dword_8009187C, &unk_801CC71C);
  sub_80013D10(&unk_801CD71C, &unk_801CD72C, dword_800918A0, 0);
  dword_80095C60 = 78;
  dword_80095C64 = -1437;
  dword_80095C68 = -9869;
  dword_80095C6C = 0;
  dword_80095C70 = -1400;
  dword_80095C74 = 0;
  return sub_80013E40();
}

```

#### Disassembly
```asm
801C609C: addiu   $sp, -0x28
801C60A0: sw      $ra, 0x10+var_s10($sp)
801C60A4: sw      $s3, 0x10+var_sC($sp)
801C60A8: sw      $s2, 0x10+var_s8($sp)
801C60AC: sw      $s1, 0x10+var_s4($sp)
801C60B0: jal     sub_801C5D28
801C60B4: sw      $s0, 0x10+var_s0($sp)
801C60B8: lw      $a0, dword_801C956C
801C60C0: lw      $a1, dword_801C9570
801C60C8: jal     sub_8001E33C
801C60CC: nop
801C60D0: move    $a1, $zero
801C60D4: li      $a2, 0xA
801C60D8: li      $a0, unk_801C959C
801C60E0: move    $v1, $zero
801C60E4: li      $at, unk_801C9578
801C60EC: addu    $at, $v1
801C60F0: sw      $a0, 0($at)
801C60F4: addiu   $a0, 0x1000
801C60F8: li      $at, dword_801C9574
801C6100: addu    $at, $v1
801C6104: sw      $a2, 0($at)
801C6108: li      $at, unk_801C957C
801C6110: addu    $at, $v1
801C6114: sw      $zero, 0($at)
801C6118: addiu   $a1, 1
801C611C: slti    $v0, $a1, 2
801C6120: bnez    $v0, loc_801C60E4
801C6124: addiu   $v1, 0x14
801C6128: move    $a0, $zero
801C612C: li      $s3, unk_801CB608
801C6134: li      $v0, 1
801C6138: sw      $zero, dword_801CB600
801C6140: sw      $zero, dword_801CFAEC
801C6148: sw      $v0, dword_8006ED98+0x34
801C6150: sw      $zero, dword_801C9548
801C6158: sw      $zero, dword_801CB59C
801C6160: jal     sub_8004049C
801C6164: move    $a1, $s3
801C6168: move    $a0, $zero
801C616C: li      $s1, unk_801CB5A0
801C6174: jal     sub_8004049C
801C6178: move    $a1, $s1
801C617C: move    $a0, $zero
801C6180: li      $s2, unk_801CD76C
801C6188: jal     sub_8004049C
801C618C: move    $a1, $s2
801C6190: move    $a0, $zero
801C6194: li      $s0, unk_801CC6BC
801C619C: jal     sub_8004049C
801C61A0: move    $a1, $s0
801C61A4: li      $a1, dword_801CD7BC
801C61AC: jal     sub_8004049C
801C61B0: move    $a0, $zero
801C61B4: li      $a0, 0xFFFFFF5D
801C61B8: li      $a1, 0xCC
801C61BC: jal     sub_801C5D8C
801C61C0: li      $a2, 0xC0
801C61C4: lw      $v1, dword_801C9544
801C61CC: li      $v0, 0xFFFFFF9C
801C61D0: sw      $v0, dword_801CC6D8
801C61D8: beqz    $v1, loc_801C6250
801C61DC: nop
801C61E0: li      $a1, unk_801CB65C
801C61E8: lw      $a0, dword_80091874
801C61F0: jal     sub_8001AF1C
801C61F4: move    $a2, $s3
801C61F8: li      $a1, unk_801CB5F0
801C6200: lw      $a0, dword_80091870
801C6208: jal     sub_8001AF1C
801C620C: move    $a2, $s1
801C6210: li      $a1, unk_801CC70C
801C6218: lw      $a0, dword_8009186C
801C6220: jal     sub_8001AF1C
801C6224: move    $a2, $s0
801C6228: li      $a1, loc_801CD80C
801C6230: lw      $a0, dword_80091878
801C6238: jal     sub_8001AF1C
801C623C: move    $a2, $s2
801C6240: sw      $zero, dword_801C9544
801C6248: j       loc_801C6270
801C624C: nop
801C6250: jal     sub_800139F8
801C6254: move    $a0, $zero
801C6258: jal     sub_800139F8
801C625C: li      $a0, 1
801C6260: jal     sub_800139F8
801C6264: li      $a0, 2
801C6268: jal     sub_800139F8
801C626C: li      $a0, 3
801C6270: jal     sub_8001EEE8
801C6274: nop
801C6278: jal     GsSetProjection
801C627C: li      $a0, 0x1B8
801C6280: li      $s1, loc_801C9534
801C6288: move    $a1, $s1
801C628C: li      $s0, unk_801CB658
801C6294: lw      $a0, dword_80091868
801C629C: sw      $zero, dword_801CB604
801C62A4: jal     sub_8001AFD8
801C62A8: move    $a2, $s0
801C62AC: move    $a1, $s0
801C62B0: lw      $a0, dword_801CB604
801C62B8: li      $a3, unk_801CB65C
801C62C0: jal     sub_8001B000
801C62C4: move    $a2, $s1
801C62C8: lw      $a0, dword_800918A4
801C62D0: lw      $a1, dword_80091880
801C62D8: lw      $a2, dword_80091874
801C62E0: jal     sub_80014164
801C62E4: li      $a3, 1
801C62E8: lw      $a1, dword_80091870
801C62F0: lw      $a2, dword_80091880
801C62F8: li      $a3, unk_801CB66C
801C6300: jal     sub_8001385C
801C6304: li      $a0, 1
801C6308: li      $a0, loc_801CC66C
801C6310: li      $a1, unk_801CC67C
801C6318: lw      $a2, dword_800918A4
801C6320: jal     sub_80013D10
801C6324: move    $a3, $zero
801C6328: lw      $a1, dword_80091878
801C6330: lw      $a2, dword_80091890
801C6338: li      $a3, loc_801CD81C
801C6340: jal     sub_8001385C
801C6344: li      $a0, 2
801C6348: li      $a0, loc_801CF81C
801C6350: li      $a1, loc_801CF82C
801C6358: lw      $a2, dword_800918B4
801C6360: jal     sub_80013D10
801C6364: move    $a3, $zero
801C6368: lw      $a1, dword_8009186C
801C6370: lw      $a2, dword_8009187C
801C6378: li      $a3, unk_801CC71C
801C6380: jal     sub_8001385C
801C6384: li      $a0, 3
801C6388: li      $a0, unk_801CD71C
801C6390: li      $a1, unk_801CD72C
801C6398: lw      $a2, dword_800918A0
801C63A0: jal     sub_80013D10
801C63A4: move    $a3, $zero
801C63A8: li      $v0, 0x4E  # 'N'
801C63AC: sw      $v0, dword_80095C60
801C63B4: li      $v0, 0xFFFFFA63
801C63B8: sw      $v0, dword_80095C64
801C63C0: li      $v0, 0xFFFFD973
801C63C4: sw      $v0, dword_80095C68
801C63CC: li      $v0, 0xFFFFFA88
801C63D0: sw      $zero, dword_80095C6C
801C63D8: sw      $v0, dword_80095C70
801C63E0: sw      $zero, dword_80095C74
801C63E8: jal     sub_80013E40
801C63EC: nop
801C63F0: lw      $ra, 0x10+var_s10($sp)
801C63F4: lw      $s3, 0x10+var_sC($sp)
801C63F8: lw      $s2, 0x10+var_s8($sp)
801C63FC: lw      $s1, 0x10+var_s4($sp)
801C6400: lw      $s0, 0x10+var_s0($sp)
801C6404: addiu   $sp, 0x28
801C6408: jr      $ra
801C640C: nop
```

### 801C5780 sub_801C5538

#### Pseudo C
```c
PrGameContext_Psx *__fastcall sub_801C5538(PrGameContext_Psx *a1)
{
  _DWORD *v2; // $s0
  int v3; // $v1
  int v4; // $v1
  PrGameContext_Psx *result; // $v0
  _UNKNOWN **v6; // $a0
  int v7; // $a2
  _DWORD *v8; // $a1
  _UNKNOWN **v9; // $a0
  int v10; // $a1
  int v11; // $a1
  _DWORD *v12; // $a2
  _UNKNOWN **v13; // $v1
  BOOL v14; // $v0
  _UNKNOWN **v15; // $v0
  PrGameContext_Psx *v16; // $a0

  v2 = 0;
  v3 = word_8008ECF8;
  if ( word_8008ECF8 > 0 )
  {
    --word_8008ECF8;
    if ( !((v3 - 1) << 16) )
    {
      dword_8008ECE0 = 0;
      a1->text264 = 0;
    }
  }
  v4 = word_8008ECFA;
  if ( word_8008ECFA > 0 )
  {
    --word_8008ECFA;
    if ( !((v4 - 1) << 16) )
    {
      dword_8008ECE4 = 0;
      a1->text268 = 0;
    }
  }
  sub_801C5094(a1, 0);
  result = 0;
  if ( a1 )
  {
    if ( g_PrStageEventStreamFlag )
    {
      v6 = 0;
      if ( g_PrStageEventStreamFlag == 1 )
        v6 = &off_801C6E50;
      if ( v6
        && (v7 = (int)v6[2], v7 < (int)v6[1])
        && (v8 = (_DWORD *)((char *)*v6 + 16 * v7), *(_DWORD *)&a1->_pad0[12] >= *v8) )
      {
        v6[2] = (_UNKNOWN *)(v7 + 1);
        v2 = v8;
      }
      else
      {
        v2 = 0;
      }
    }
    if ( g_PrStageEventStreamId )
    {
      v9 = 0;
      if ( g_PrStageEventStreamId == 1 )
        v9 = &off_801C6E50;
      v10 = 0;
      if ( v9 )
      {
        v11 = (int)v9[2];
        if ( v11 < (int)v9[1]
          && (v12 = (_DWORD *)((char *)*v9 + 16 * v11), *(_DWORD *)&a1->_pad0[12] >= dword_801C9558 + *v12) )
        {
          v9[2] = (_UNKNOWN *)(v11 + 1);
          v10 = (int)v12;
        }
        else
        {
          v10 = 0;
        }
      }
      if ( !v10 )
      {
        v13 = 0;
        if ( g_PrStageEventStreamId == 1 )
          v13 = &off_801C6E50;
        v14 = 0;
        if ( v13 )
          v14 = (int)v13[2] >= (int)v13[1];
        if ( v14 )
        {
          v15 = 0;
          if ( g_PrStageEventStreamId == 1 )
          {
            *(_DWORD *)a1->_pad0 |= 0x100u;
            v15 = 0;
            if ( g_PrStageEventStreamId == 1 )
              v15 = &off_801C6E50;
          }
          if ( v15 )
            v15[2] = 0;
          g_PrStageEventStreamId = 0;
        }
        v10 = 0;
      }
      v16 = a1;
      if ( !v10 )
        return a1;
    }
    else
    {
      if ( !g_PrStageEventStreamFlag )
        return a1;
      v16 = a1;
      if ( !v2 )
        return a1;
      v10 = (int)v2;
    }
    sub_801C5190((int)v16, v10);
    return a1;
  }
  return result;
}

```

#### Disassembly
```asm
801C5538: lh      $v0, word_8008ECF8
801C5540: addiu   $sp, -0x30
801C5544: sw      $s1, 0x20+var_s4($sp)
801C5548: move    $s1, $a0
801C554C: sw      $s0, 0x20+var_s0($sp)
801C5550: move    $s0, $zero
801C5554: sw      $ra, 0x20+var_s8($sp)
801C5558: blez    $v0, loc_801C5584
801C555C: move    $v1, $v0
801C5560: addiu   $v0, $v1, -1
801C5564: sh      $v0, word_8008ECF8
801C556C: sll     $v0, 16
801C5570: bnez    $v0, loc_801C5584
801C5574: nop
801C5578: sw      $zero, dword_8008ECE0
801C5580: sw      $zero, 0x108($s1)
801C5584: lh      $v0, word_8008ECFA
801C558C: nop
801C5590: blez    $v0, loc_801C55BC
801C5594: move    $v1, $v0
801C5598: addiu   $v0, $v1, -1
801C559C: sh      $v0, word_8008ECFA
801C55A4: sll     $v0, 16
801C55A8: bnez    $v0, loc_801C55C0
801C55AC: move    $a0, $s1
801C55B0: sw      $zero, dword_8008ECE4
801C55B8: sw      $zero, 0x10C($s1)
801C55BC: move    $a0, $s1
801C55C0: jal     sub_801C5094
801C55C4: move    $a1, $zero
801C55C8: beqz    $s1, loc_801C57B0
801C55CC: move    $v0, $zero
801C55D0: lh      $v1, g_PrStageEventStreamFlag
801C55D8: nop
801C55DC: beqz    $v1, loc_801C5648
801C55E0: li      $v0, 1
801C55E4: lw      $a3, 0xC($s1)
801C55E8: bne     $v1, $v0, loc_801C55F8
801C55EC: move    $a0, $zero
801C55F0: li      $a0, off_801C6E50
801C55F8: beqz    $a0, loc_801C5638
801C55FC: nop
801C5600: lw      $a2, 8($a0)
801C5604: lw      $v0, 4($a0)
801C5608: nop
801C560C: slt     $v0, $a2, $v0
801C5610: beqz    $v0, loc_801C5638
801C5614: sll     $v0, $a2, 4
801C5618: lw      $v1, 0($a0)
801C561C: nop
801C5620: addu    $a1, $v1, $v0
801C5624: lw      $v0, 0($a1)
801C5628: nop
801C562C: slt     $v0, $a3, $v0
801C5630: beqz    $v0, loc_801C5640
801C5634: addiu   $v0, $a2, 1
801C5638: j       loc_801C5648
801C563C: move    $s0, $zero
801C5640: sw      $v0, 8($a0)
801C5644: move    $s0, $a1
801C5648: lh      $v0, g_PrStageEventStreamId
801C5650: nop
801C5654: beqz    $v0, loc_801C5784
801C5658: li      $v1, 1
801C565C: lh      $v0, g_PrStageEventStreamId
801C5664: lw      $a3, 0xC($s1)
801C5668: bne     $v0, $v1, loc_801C5678
801C566C: move    $a0, $zero
801C5670: li      $a0, off_801C6E50
801C5678: beqz    $a0, loc_801C56D4
801C567C: move    $a1, $zero
801C5680: lw      $a1, 8($a0)
801C5684: lw      $v0, 4($a0)
801C5688: nop
801C568C: slt     $v0, $a1, $v0
801C5690: beqz    $v0, loc_801C56C4
801C5694: sll     $v0, $a1, 4
801C5698: lw      $v1, 0($a0)
801C569C: nop
801C56A0: addu    $a2, $v1, $v0
801C56A4: lw      $v0, dword_801C9558
801C56AC: lw      $v1, 0($a2)
801C56B0: nop
801C56B4: addu    $v0, $v1
801C56B8: slt     $v0, $a3, $v0
801C56BC: beqz    $v0, loc_801C56CC
801C56C0: addiu   $v0, $a1, 1
801C56C4: j       loc_801C56D4
801C56C8: move    $a1, $zero
801C56CC: sw      $v0, 8($a0)
801C56D0: move    $a1, $a2
801C56D4: bnez    $a1, loc_801C5774
801C56D8: li      $v0, 1
801C56DC: lh      $v1, g_PrStageEventStreamId
801C56E4: nop
801C56E8: bne     $v1, $v0, loc_801C56F8
801C56EC: move    $v1, $zero
801C56F0: li      $v1, off_801C6E50
801C56F8: beqz    $v1, loc_801C5714
801C56FC: move    $v0, $zero
801C5700: lw      $v0, 8($v1)
801C5704: lw      $v1, 4($v1)
801C5708: nop
801C570C: slt     $v0, $v1
801C5710: xori    $v0, 1
801C5714: beqz    $v0, loc_801C5770
801C5718: li      $v1, 1
801C571C: lh      $v0, g_PrStageEventStreamId
801C5724: nop
801C5728: bne     $v0, $v1, loc_801C575C
801C572C: move    $v0, $zero
801C5730: lw      $v0, 0($s1)
801C5734: nop
801C5738: ori     $v0, 0x100
801C573C: sw      $v0, 0($s1)
801C5740: lh      $v0, g_PrStageEventStreamId
801C5748: nop
801C574C: bne     $v0, $v1, loc_801C575C
801C5750: move    $v0, $zero
801C5754: li      $v0, off_801C6E50
801C575C: beqz    $v0, loc_801C5768
801C5760: nop
801C5764: sw      $zero, 8($v0)
801C5768: sh      $zero, g_PrStageEventStreamId
801C5770: move    $a1, $zero
801C5774: beqz    $a1, loc_801C57AC
801C5778: move    $a0, $s1
801C577C: j       loc_801C57A4
801C5780: nop
801C5784: lh      $v0, g_PrStageEventStreamFlag
801C578C: nop
801C5790: beqz    $v0, loc_801C57AC
801C5794: nop
801C5798: beqz    $s0, loc_801C57AC
801C579C: move    $a0, $s1
801C57A0: move    $a1, $s0
801C57A4: jal     sub_801C5190
801C57A8: nop
801C57AC: move    $v0, $s1
801C57B0: lw      $ra, 0x20+var_s8($sp)
801C57B4: lw      $s1, 0x20+var_s4($sp)
801C57B8: lw      $s0, 0x20+var_s0($sp)
801C57BC: addiu   $sp, 0x30
801C57C0: jr      $ra
801C57C4: nop
```

### 801C4870 sub_801C47EC

#### Pseudo C
```c
int __fastcall sub_801C47EC(int a1, int *a2)
{
  int v4; // $s2
  bool v5; // dc
  int v6; // $a0
  int result; // $v0

  v4 = *a2;
  v5 = (a1 & 0x40) != 0;
  v6 = 32;
  if ( v5 || (v6 = 4096, (a1 & 0xA000) != 0) )
    sub_80025C8C(v6);
  if ( a1 != 64 )
    goto LABEL_13;
  result = 1;
  if ( !*a2 )
    return 2;
  if ( *a2 != 1 )
  {
LABEL_13:
    if ( a1 == 0x8000 || (result = 0, a1 == 0x2000) )
    {
      *a2 = v4 == 0;
      return 0;
    }
  }
  return result;
}

```

#### Disassembly
```asm
801C47EC: addiu   $sp, -0x20
801C47F0: sw      $s0, 0x10+var_s0($sp)
801C47F4: move    $s0, $a0
801C47F8: sw      $s1, 0x10+var_s4($sp)
801C47FC: move    $s1, $a1
801C4800: andi    $v0, $s0, 0x40
801C4804: sw      $ra, 0x10+var_sC($sp)
801C4808: sw      $s2, 0x10+var_s8($sp)
801C480C: lw      $s2, 0($s1)
801C4810: bnez    $v0, loc_801C4824
801C4814: li      $a0, 0x20  # ' '
801C4818: andi    $v0, $s0, 0xA000
801C481C: beqz    $v0, loc_801C482C
801C4820: li      $a0, 0x1000
801C4824: jal     sub_80025C8C
801C4828: nop
801C482C: li      $v0, 0x40  # '@'
801C4830: bne     $s0, $v0, loc_801C4858
801C4834: nop
801C4838: lw      $v1, 0($s1)
801C483C: nop
801C4840: bnez    $v1, loc_801C4850
801C4844: li      $v0, 1
801C4848: j       loc_801C4878
801C484C: li      $v0, 2
801C4850: beq     $v1, $v0, loc_801C4878
801C4854: nop
801C4858: li      $v0, 0x8000
801C485C: beq     $s0, $v0, loc_801C486C
801C4860: li      $v0, 0x2000
801C4864: bne     $s0, $v0, loc_801C4878
801C4868: move    $v0, $zero
801C486C: sltiu   $v0, $s2, 1
801C4870: sw      $v0, 0($s1)
801C4874: move    $v0, $zero
801C4878: lw      $ra, 0x10+var_sC($sp)
801C487C: lw      $s2, 0x10+var_s8($sp)
801C4880: lw      $s1, 0x10+var_s4($sp)
801C4884: lw      $s0, 0x10+var_s0($sp)
801C4888: addiu   $sp, 0x20
801C488C: jr      $ra
801C4890: nop
```

### 801C5BDC sub_801C5B14

#### Pseudo C
```c
int sub_801C5B14()
{
  dword_800943C0 = (int)&unk_801C6F88;
  dword_800943C4 = (int)&unk_801C6F68;
  dword_800943C8 = 1;
  dword_800943CC = (int)&off_801C6BF8;
  dword_800943D0 = (int)&unk_801C6DA4;
  dword_800943D8 = (int)&unk_801C6EF0;
  dword_800943E0 = (int)&unk_801C6F18;
  dword_800943E4 = (int)&dword_801C6F84;
  dword_800943FC = (int)&unk_801C6E88;
  dword_80094400 = (int)&unk_801C6E7C;
  dword_80094414 = (int)&unk_801C6EB4;
  dword_80094418 = (int)&unk_801C6EBC;
  dword_8009441C = (int)&unk_801C6EC4;
  dword_80094420 = (int)&unk_801C6ED0;
  dword_80094424 = (int)&unk_801C6ED8;
  dword_80094428 = (int)&unk_801C6EE0;
  dword_8009442C = (int)&unk_801C6EE8;
  dword_80094404 = (int)&unk_801C6E94;
  dword_80094408 = (int)&unk_801C6E9C;
  dword_8009440C = (int)&unk_801C6EA4;
  dword_80094410 = (int)&unk_801C6EAC;
  dword_800943EC = (int)&unk_801C6E5C;
  dword_800943F0 = (int)&unk_801C6E64;
  dword_800943F4 = (int)&unk_801C6E6C;
  dword_800943F8 = (int)&unk_801C6E74;
  dword_80094430 = (int)&nullsub_9;
  dword_80094434 = (int)&nullsub_11;
  dword_800943D4 = dword_801C6DD0;
  dword_800943DC = dword_801C6F14;
  dword_800943E8 = dword_801C6F84;
  dword_80094438 = (int (__fastcall *)(_DWORD))&nullsub_10;
  dword_8009443C = (int)&nullsub_12;
  dword_80094440 = (int)&sub_801C4FC8;
  return 0;
}

```

#### Disassembly
```asm
801C5B14: lw      $v1, dword_801C6DD0
801C5B1C: lw      $a0, dword_801C6F14
801C5B24: lw      $a1, dword_801C6F84
801C5B2C: li      $v0, unk_801C6F88
801C5B34: sw      $v0, dword_800943C0
801C5B3C: li      $v0, unk_801C6F68
801C5B44: sw      $v0, dword_800943C4
801C5B4C: li      $v0, 1
801C5B50: sw      $v0, dword_800943C8
801C5B58: li      $v0, off_801C6BF8
801C5B60: sw      $v0, dword_800943CC
801C5B68: li      $v0, unk_801C6DA4
801C5B70: sw      $v0, dword_800943D0
801C5B78: li      $v0, unk_801C6EF0
801C5B80: sw      $v0, dword_800943D8
801C5B88: li      $v0, unk_801C6F18
801C5B90: sw      $v0, dword_800943E0
801C5B98: li      $v0, dword_801C6F84
801C5BA0: sw      $v0, dword_800943E4
801C5BA8: li      $v0, unk_801C6E88
801C5BB0: sw      $v0, dword_800943FC
801C5BB8: li      $v0, unk_801C6E7C
801C5BC0: sw      $v0, dword_80094400
801C5BC8: li      $v0, unk_801C6EB4
801C5BD0: sw      $v0, dword_80094414
801C5BD8: lui     $v0, 0x801C
801C5BDC: li      $v0, unk_801C6EBC
801C5BE0: sw      $v0, dword_80094418
801C5BE8: li      $v0, unk_801C6EC4
801C5BF0: sw      $v0, dword_8009441C
801C5BF8: li      $v0, unk_801C6ED0
801C5C00: sw      $v0, dword_80094420
801C5C08: li      $v0, unk_801C6ED8
801C5C10: sw      $v0, dword_80094424
801C5C18: li      $v0, unk_801C6EE0
801C5C20: sw      $v0, dword_80094428
801C5C28: li      $v0, unk_801C6EE8
801C5C30: sw      $v0, dword_8009442C
801C5C38: li      $v0, unk_801C6E94
801C5C40: sw      $v0, dword_80094404
801C5C48: li      $v0, unk_801C6E9C
801C5C50: sw      $v0, dword_80094408
801C5C58: li      $v0, unk_801C6EA4
801C5C60: sw      $v0, dword_8009440C
801C5C68: li      $v0, unk_801C6EAC
801C5C70: sw      $v0, dword_80094410
801C5C78: li      $v0, unk_801C6E5C
801C5C80: sw      $v0, dword_800943EC
801C5C88: li      $v0, unk_801C6E64
801C5C90: sw      $v0, dword_800943F0
801C5C98: li      $v0, unk_801C6E6C
801C5CA0: sw      $v0, dword_800943F4
801C5CA8: li      $v0, unk_801C6E74
801C5CB0: sw      $v0, dword_800943F8
801C5CB8: li      $v0, nullsub_9
801C5CC0: sw      $v0, dword_80094430
801C5CC8: li      $v0, nullsub_11
801C5CD0: sw      $v0, dword_80094434
801C5CD8: li      $v0, nullsub_10
801C5CE0: sw      $v1, dword_800943D4
801C5CE8: sw      $a0, dword_800943DC
801C5CF0: sw      $a1, dword_800943E8
801C5CF8: sw      $v0, dword_80094438
801C5D00: li      $v0, nullsub_12
801C5D08: sw      $v0, dword_8009443C
801C5D10: li      $v0, sub_801C4FC8
801C5D18: sw      $v0, dword_80094440
801C5D20: jr      $ra
801C5D24: move    $v0, $zero
```

### 801C4834 sub_801C47EC

#### Pseudo C
```c
int __fastcall sub_801C47EC(int a1, int *a2)
{
  int v4; // $s2
  bool v5; // dc
  int v6; // $a0
  int result; // $v0

  v4 = *a2;
  v5 = (a1 & 0x40) != 0;
  v6 = 32;
  if ( v5 || (v6 = 4096, (a1 & 0xA000) != 0) )
    sub_80025C8C(v6);
  if ( a1 != 64 )
    goto LABEL_13;
  result = 1;
  if ( !*a2 )
    return 2;
  if ( *a2 != 1 )
  {
LABEL_13:
    if ( a1 == 0x8000 || (result = 0, a1 == 0x2000) )
    {
      *a2 = v4 == 0;
      return 0;
    }
  }
  return result;
}

```

#### Disassembly
```asm
801C47EC: addiu   $sp, -0x20
801C47F0: sw      $s0, 0x10+var_s0($sp)
801C47F4: move    $s0, $a0
801C47F8: sw      $s1, 0x10+var_s4($sp)
801C47FC: move    $s1, $a1
801C4800: andi    $v0, $s0, 0x40
801C4804: sw      $ra, 0x10+var_sC($sp)
801C4808: sw      $s2, 0x10+var_s8($sp)
801C480C: lw      $s2, 0($s1)
801C4810: bnez    $v0, loc_801C4824
801C4814: li      $a0, 0x20  # ' '
801C4818: andi    $v0, $s0, 0xA000
801C481C: beqz    $v0, loc_801C482C
801C4820: li      $a0, 0x1000
801C4824: jal     sub_80025C8C
801C4828: nop
801C482C: li      $v0, 0x40  # '@'
801C4830: bne     $s0, $v0, loc_801C4858
801C4834: nop
801C4838: lw      $v1, 0($s1)
801C483C: nop
801C4840: bnez    $v1, loc_801C4850
801C4844: li      $v0, 1
801C4848: j       loc_801C4878
801C484C: li      $v0, 2
801C4850: beq     $v1, $v0, loc_801C4878
801C4854: nop
801C4858: li      $v0, 0x8000
801C485C: beq     $s0, $v0, loc_801C486C
801C4860: li      $v0, 0x2000
801C4864: bne     $s0, $v0, loc_801C4878
801C4868: move    $v0, $zero
801C486C: sltiu   $v0, $s2, 1
801C4870: sw      $v0, 0($s1)
801C4874: move    $v0, $zero
801C4878: lw      $ra, 0x10+var_sC($sp)
801C487C: lw      $s2, 0x10+var_s8($sp)
801C4880: lw      $s1, 0x10+var_s4($sp)
801C4884: lw      $s0, 0x10+var_s0($sp)
801C4888: addiu   $sp, 0x20
801C488C: jr      $ra
801C4890: nop
```

## Notes

- `80015D18` indexes this table as `off_80048D28 + 3 * sceneIndex` and calls fn0, fn1, fn2 in order.
- This export is table evidence only; code changes must still preserve each function boundary.
