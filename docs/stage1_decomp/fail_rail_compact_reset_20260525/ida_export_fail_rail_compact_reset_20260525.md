# Stage1 fail rail compact reset evidence 2026-05-25

IDA input: E:\game\PS模拟器\parappa the rapper\SCUS_941.83

## 0x801C8D78 sub_801C8D78

<function not found>

## 0x80024308 sub_80024308

Bounds: 0x80024308..0x80024390

### Hex-Rays pseudo-C
```c
BOOL sub_80024308()
{
  int v0; // $a2
  __int16 *v1; // $a1
  __int16 *v2; // $a0
  unsigned int v3; // $v1
  BOOL result; // $v0

  v0 = 0;
  v1 = word_800876B0;
  v2 = word_80087668;
  v3 = 0;
  do
  {
    *(_DWORD *)((char *)&unk_800876F8 + v3) = 0;
    *(_DWORD *)((char *)&unk_800876FC + v3) = 2048;
    *(_DWORD *)((char *)&unk_80087700 + v3) = 2048;
    dword_80087704[v3 / 4] = 0;
    *v2 = 4096;
    *v1++ = 4096;
    ++v2;
    result = ++v0 < 36;
    v3 += 16;
  }
  while ( v0 < 36 );
  return result;
}

```

### Disassembly
```asm
0x80024308: move    $a2, $zero
0x8002430C: li      $t0, 0x800
0x80024310: li      $a3, 0x1000
0x80024314: li      $a1, word_800876B0
0x8002431C: li      $a0, word_80087668
0x80024324: move    $v1, $zero
0x80024328: li      $at, unk_800876F8
0x80024330: addu    $at, $v1
0x80024334: sw      $zero, 0($at)
0x80024338: li      $at, unk_800876FC
0x80024340: addu    $at, $v1
0x80024344: sw      $t0, 0($at)
0x80024348: li      $at, unk_80087700
0x80024350: addu    $at, $v1
0x80024354: sw      $t0, 0($at)
0x80024358: li      $at, dword_80087704
0x80024360: addu    $at, $v1
0x80024364: sw      $zero, 0($at)
0x80024368: sh      $a3, 0($a0)
0x8002436C: sh      $a3, 0($a1)
0x80024370: addiu   $a1, 2
0x80024374: addiu   $a0, 2
0x80024378: addiu   $a2, 1
0x8002437C: slti    $v0, $a2, 0x24  # '$'
0x80024380: bnez    $v0, loc_80024328
0x80024384: addiu   $v1, 0x10
0x80024388: jr      $ra
0x8002438C: nop
```

### Xrefs to
```text
0x80027864 -> 0x80024308 type=17 from_func=sub_8002776C
0x8002796C -> 0x80024308 type=17 from_func=sub_8002776C
0x800279C0 -> 0x80024308 type=17 from_func=sub_8002776C
0x80027CC4 -> 0x80024308 type=17 from_func=sub_8002776C
0x80027D18 -> 0x80024308 type=17 from_func=sub_8002776C
0x80027D80 -> 0x80024308 type=17 from_func=sub_8002776C
```

### Xrefs from
```text
0x80024314 -> 0x800876B0 type=1 target=word_800876B0
0x8002431C -> 0x80087668 type=1 target=word_80087668
0x80024328 -> 0x800876F8 type=1 target=unk_800876F8
0x80024338 -> 0x800876FC type=1 target=unk_800876FC
0x80024348 -> 0x80087700 type=1 target=unk_80087700
0x80024358 -> 0x80087704 type=1 target=dword_80087704
```

### Referenced globals / immediates
```text
0x80024380 op1 -> 0x80024328 name=loc_80024328 | bnez    $v0, loc_80024328
```

## 0x80024390 sub_80024390

Bounds: 0x80024390..0x80024418

### Hex-Rays pseudo-C
```c
BOOL sub_80024390()
{
  int v0; // $a2
  __int16 *v1; // $a1
  __int16 *v2; // $a0
  unsigned int v3; // $v1
  BOOL result; // $v0

  v0 = 0;
  v1 = word_800876B0;
  v2 = word_80087668;
  v3 = 0;
  do
  {
    *(_DWORD *)((char *)&unk_80087938 + v3) = 0;
    *(_DWORD *)((char *)&unk_8008793C + v3) = 2048;
    *(_DWORD *)((char *)&unk_80087940 + v3) = 2048;
    dword_80087944[v3 / 4] = 0;
    *v2 = 4096;
    *v1++ = 4096;
    ++v2;
    result = ++v0 < 36;
    v3 += 16;
  }
  while ( v0 < 36 );
  return result;
}

```

### Disassembly
```asm
0x80024390: move    $a2, $zero
0x80024394: li      $t0, 0x800
0x80024398: li      $a3, 0x1000
0x8002439C: li      $a1, word_800876B0
0x800243A4: li      $a0, word_80087668
0x800243AC: move    $v1, $zero
0x800243B0: li      $at, unk_80087938
0x800243B8: addu    $at, $v1
0x800243BC: sw      $zero, 0($at)
0x800243C0: li      $at, unk_8008793C
0x800243C8: addu    $at, $v1
0x800243CC: sw      $t0, 0($at)
0x800243D0: li      $at, unk_80087940
0x800243D8: addu    $at, $v1
0x800243DC: sw      $t0, 0($at)
0x800243E0: li      $at, dword_80087944
0x800243E8: addu    $at, $v1
0x800243EC: sw      $zero, 0($at)
0x800243F0: sh      $a3, 0($a0)
0x800243F4: sh      $a3, 0($a1)
0x800243F8: addiu   $a1, 2
0x800243FC: addiu   $a0, 2
0x80024400: addiu   $a2, 1
0x80024404: slti    $v0, $a2, 0x24  # '$'
0x80024408: bnez    $v0, loc_800243B0
0x8002440C: addiu   $v1, 0x10
0x80024410: jr      $ra
0x80024414: nop
```

### Xrefs to
```text
<none>
```

### Xrefs from
```text
0x8002439C -> 0x800876B0 type=1 target=word_800876B0
0x800243A4 -> 0x80087668 type=1 target=word_80087668
0x800243B0 -> 0x80087938 type=1 target=unk_80087938
0x800243C0 -> 0x8008793C type=1 target=unk_8008793C
0x800243D0 -> 0x80087940 type=1 target=unk_80087940
0x800243E0 -> 0x80087944 type=1 target=dword_80087944
```

### Referenced globals / immediates
```text
0x80024408 op1 -> 0x800243B0 name=loc_800243B0 | bnez    $v0, loc_800243B0
```

## 0x801C9094 sub_801C9094

<function not found>

## 0x80024744 sub_80024744

Bounds: 0x80024744..0x80024B54

### Hex-Rays pseudo-C
```c
int __fastcall sub_80024744(int a1)
{
  int result; // $v0
  int v3; // $s7
  int v4; // $s5
  int v5; // $fp
  int v6; // $a0
  _DWORD *v7; // $gp
  int v8; // $v1
  int v9; // $a0
  int v10; // $gp
  int v11; // $s0
  int v12; // $v1
  int v13; // $v1
  int v14; // $s3
  int *v15; // $s1
  __int16 *v16; // $s2
  int *v17; // $a2
  int v18; // $s0
  __int16 *v19; // $s2
  int *v20; // $s1
  int *v21; // $a2
  __int16 v22; // $s1
  __int16 v23; // $s4
  __int16 v24; // $s3
  int v25; // $s2
  __int16 v26; // $v1
  _WORD v27[8]; // [sp+10h] [-38h] BYREF
  int v28; // [sp+20h] [-28h]
  int v29; // [sp+28h] [-20h]
  int v30; // [sp+30h] [-18h]
  int v31; // [sp+38h] [-10h]
  unsigned int v32; // [sp+40h] [-8h]

  result = *(__int16 *)(a1 + 138);
  v3 = 0;
  if ( result > 0 )
  {
    v4 = a1;
    v5 = 0;
    v28 = a1;
    v29 = 0;
    v30 = 24;
    v31 = 0;
    v32 = 0;
    while ( 1 )
    {
      v6 = *(__int16 *)(v4 + 140);
      if ( v6 >= 0 )
      {
        sub_80023F20(v6 + v5);
        if ( *(_WORD *)(a1 + 122) == 1 )
        {
          v8 = *(__int16 *)(v4 + 140);
          if ( v7[77] == v8 )
          {
            ++v7[76];
          }
          else
          {
            v7[77] = v8;
            v7[76] = 0;
          }
          if ( (int)v7[76] > 4 )
            v7[76] = 4;
          if ( v8 > 0 )
            sub_8001C550(15 * v8 + 26 + 4 * *((_WORD *)v7 + 152), v31 + 18, v7[199], 0);
        }
      }
      v9 = *(__int16 *)(v4 + 158);
      if ( v9 < 0 )
        goto LABEL_21;
      sub_80024114(v9 + v5);
      v11 = 0;
      if ( *(_WORD *)(a1 + 122) == 1 )
        break;
LABEL_37:
      v22 = 32;
      v23 = v30;
      v24 = v29;
      v25 = v28;
      do
      {
        v27[0] = v22;
        v27[1] = v23;
        v27[2] = v24 + v11;
        v26 = *(char *)(*(_DWORD *)(v25 + 148) + v11);
        v27[3] = v26;
        if ( *(_WORD *)(a1 + 122) == 1 && (unsigned __int16)(v26 - 1) < 8u )
          sub_80024418(v27);
        ++v11;
        v22 += 15;
      }
      while ( v11 < 18 );
      v28 += 4;
      v29 += 17;
      v30 += 20;
      v4 += 2;
      v31 += 20;
      v32 += 4;
      result = ++v3 < *(__int16 *)(a1 + 138);
      v5 += 18;
      if ( v3 >= *(__int16 *)(a1 + 138) )
        return result;
    }
    v12 = *(__int16 *)(v4 + 158);
    if ( *(_DWORD *)(v10 + 316) == v12 )
    {
      ++*(_DWORD *)(v10 + 312);
    }
    else
    {
      *(_DWORD *)(v10 + 316) = v12;
      *(_DWORD *)(v10 + 312) = 1;
    }
    if ( *(int *)(v10 + 312) >= 5 )
      *(_DWORD *)(v10 + 312) = 4;
    if ( v12 >= 0 )
      sub_8001C550(15 * v12 + 4 * *(_WORD *)(v10 + 312) + 26, v31 + 16, (int)dword_8005400C, 0);
LABEL_21:
    v11 = 0;
    if ( *(_WORD *)(a1 + 122) == 1 )
    {
      v13 = *(__int16 *)(v4 + 140);
      if ( v13 <= 0 )
      {
        v14 = 0;
        v11 = 0;
      }
      else
      {
        v14 = 15 * v13 + 31;
      }
      v15 = dword_800540E8;
      v16 = (__int16 *)&dword_800540E0[v32 / 4];
      do
      {
        if ( v14 >= *v15 )
          v17 = dword_800540AC;
        else
          v17 = dword_8005409C;
        sub_8001C550(*v15, *v16, (int)v17, 3u);
        ++v11;
        ++v15;
      }
      while ( v11 < 4 );
      v18 = 2 * (v3 > 0);
      if ( v18 < 14 )
      {
        v19 = (__int16 *)&dword_800540F8[v32 / 4];
        v20 = &dword_80054100[2 * (v3 > 0)];
        do
        {
          if ( v14 >= *v20 )
            v21 = dword_8005408C;
          else
            v21 = dword_8005407C;
          sub_8001C550(*v20, *v19, (int)v21, 3u);
          ++v18;
          ++v20;
        }
        while ( v18 < 14 );
      }
      v11 = 0;
    }
    goto LABEL_37;
  }
  return result;
}

```

### Disassembly
```asm
0x80024744: addiu   $sp, -0x70
0x80024748: sw      $s6, 0x48+var_s18($sp)
0x8002474C: move    $s6, $a0
0x80024750: sw      $ra, 0x48+var_s24($sp)
0x80024754: sw      $fp, 0x48+var_s20($sp)
0x80024758: sw      $s7, 0x48+var_s1C($sp)
0x8002475C: sw      $s5, 0x48+var_s14($sp)
0x80024760: sw      $s4, 0x48+var_s10($sp)
0x80024764: sw      $s3, 0x48+var_sC($sp)
0x80024768: sw      $s2, 0x48+var_s8($sp)
0x8002476C: sw      $s1, 0x48+var_s4($sp)
0x80024770: sw      $s0, 0x48+var_s0($sp)
0x80024774: lh      $v0, 0x8A($s6)
0x80024778: nop
0x8002477C: blez    $v0, loc_80024B20
0x80024780: move    $s7, $zero
0x80024784: li      $t0, 0x18
0x80024788: move    $s5, $s6
0x8002478C: move    $fp, $zero
0x80024790: sw      $s6, 0x48+var_28($sp)
0x80024794: sw      $zero, 0x48+var_20($sp)
0x80024798: sw      $t0, 0x48+var_18($sp)
0x8002479C: sw      $zero, 0x48+var_10($sp)
0x800247A0: sw      $zero, 0x48+var_8($sp)
0x800247A4: lh      $a0, 0x8C($s5)
0x800247A8: nop
0x800247AC: bltz    $a0, loc_80024858
0x800247B0: nop
0x800247B4: jal     sub_80023F20
0x800247B8: addu    $a0, $fp
0x800247BC: lh      $v0, 0x7A($s6)
0x800247C0: li      $t0, 1
0x800247C4: bne     $v0, $t0, loc_80024858
0x800247C8: nop
0x800247CC: lh      $v1, 0x8C($s5)
0x800247D0: lw      $v0, 0x134($gp)
0x800247D4: nop
0x800247D8: beq     $v0, $v1, loc_800247F0
0x800247DC: li      $a0, 4
0x800247E0: sw      $v1, 0x134($gp)
0x800247E4: sw      $zero, 0x130($gp)
0x800247E8: j       loc_80024800
0x800247EC: nop
0x800247F0: lw      $v0, 0x130($gp)
0x800247F4: nop
0x800247F8: addiu   $v0, 1
0x800247FC: sw      $v0, 0x130($gp)
0x80024800: lw      $v0, 0x130($gp)
0x80024804: nop
0x80024808: slt     $v0, $a0, $v0
0x8002480C: beqz    $v0, loc_80024818
0x80024810: nop
0x80024814: sw      $a0, 0x130($gp)
0x80024818: blez    $v1, loc_80024858
0x8002481C: move    $a3, $zero
0x80024820: sll     $a0, $v1, 4
0x80024824: subu    $a0, $v1
0x80024828: addiu   $a0, 0x1A
0x8002482C: lw      $t0, 0x48+var_10($sp)
0x80024830: lw      $a2, 0x31C($gp)
0x80024834: lw      $v0, 0x130($gp)
0x80024838: addiu   $a1, $t0, 0x12
0x8002483C: sll     $a1, 16
0x80024840: sra     $a1, 16
0x80024844: sll     $v0, 2
0x80024848: addu    $a0, $v0
0x8002484C: sll     $a0, 16
0x80024850: jal     sub_8001C550
0x80024854: sra     $a0, 16
0x80024858: lh      $a0, 0x9E($s5)
0x8002485C: nop
0x80024860: bltz    $a0, loc_80024910
0x80024864: nop
0x80024868: jal     sub_80024114
0x8002486C: addu    $a0, $fp
0x80024870: lh      $v0, 0x7A($s6)
0x80024874: li      $t0, 1
0x80024878: bne     $v0, $t0, loc_80024A44
0x8002487C: move    $s0, $zero
0x80024880: lh      $v1, 0x9E($s5)
0x80024884: lw      $v0, 0x13C($gp)
0x80024888: nop
0x8002488C: beq     $v0, $v1, loc_800248A4
0x80024890: nop
0x80024894: sw      $v1, 0x13C($gp)
0x80024898: sw      $t0, 0x138($gp)
0x8002489C: j       loc_800248B4
0x800248A0: nop
0x800248A4: lw      $v0, 0x138($gp)
0x800248A8: nop
0x800248AC: addiu   $v0, 1
0x800248B0: sw      $v0, 0x138($gp)
0x800248B4: lw      $v0, 0x138($gp)
0x800248B8: nop
0x800248BC: slti    $v0, 5
0x800248C0: bnez    $v0, loc_800248CC
0x800248C4: li      $v0, 4
0x800248C8: sw      $v0, 0x138($gp)
0x800248CC: bltz    $v1, loc_80024910
0x800248D0: move    $a3, $zero
0x800248D4: li      $a2, dword_8005400C
0x800248DC: sll     $a0, $v1, 4
0x800248E0: subu    $a0, $v1
0x800248E4: lw      $t0, 0x48+var_10($sp)
0x800248E8: lw      $v0, 0x138($gp)
0x800248EC: addiu   $a1, $t0, 0x10
0x800248F0: sll     $a1, 16
0x800248F4: sra     $a1, 16
0x800248F8: sll     $v0, 2
0x800248FC: addiu   $v0, 0x1A
0x80024900: addu    $a0, $v0
0x80024904: sll     $a0, 16
0x80024908: jal     sub_8001C550
0x8002490C: sra     $a0, 16
0x80024910: lh      $v0, 0x7A($s6)
0x80024914: li      $t0, 1
0x80024918: bne     $v0, $t0, loc_80024A44
0x8002491C: move    $s0, $zero
0x80024920: lh      $v1, 0x8C($s5)
0x80024924: nop
0x80024928: blez    $v1, loc_8002493C
0x8002492C: sll     $v0, $v1, 4
0x80024930: subu    $v0, $v1
0x80024934: j       loc_80024944
0x80024938: addiu   $s3, $v0, 0x1F
0x8002493C: move    $s3, $zero
0x80024940: move    $s0, $zero
0x80024944: li      $v0, dword_800540E0
0x8002494C: lw      $t0, 0x48+var_8($sp)
0x80024950: li      $s1, dword_800540E8
0x80024958: addu    $s2, $t0, $v0
0x8002495C: lw      $a0, 0($s1)
0x80024960: nop
0x80024964: slt     $v0, $s3, $a0
0x80024968: beqz    $v0, loc_80024988
0x8002496C: nop
0x80024970: sll     $a0, 16
0x80024974: lh      $a1, 0($s2)
0x80024978: li      $a2, dword_8005409C
0x80024980: j       loc_8002499C
0x80024984: sra     $a0, 16
0x80024988: sll     $a0, 16
0x8002498C: sra     $a0, 16
0x80024990: lh      $a1, 0($s2)
0x80024994: li      $a2, dword_800540AC
0x8002499C: jal     sub_8001C550
0x800249A0: li      $a3, 3
0x800249A4: addiu   $s0, 1
0x800249A8: slti    $v0, $s0, 4
0x800249AC: bnez    $v0, loc_8002495C
0x800249B0: addiu   $s1, 4
0x800249B4: slt     $v0, $zero, $s7
0x800249B8: sll     $s0, $v0, 1
0x800249BC: slti    $v0, $s0, 0xE
0x800249C0: beqz    $v0, loc_80024A40
0x800249C4: nop
0x800249C8: li      $v0, dword_800540F8
0x800249D0: lw      $t0, 0x48+var_8($sp)
0x800249D4: li      $v1, dword_80054100
0x800249DC: addu    $s2, $t0, $v0
0x800249E0: sll     $v0, $s0, 2
0x800249E4: addu    $s1, $v0, $v1
0x800249E8: lw      $a0, 0($s1)
0x800249EC: nop
0x800249F0: slt     $v0, $s3, $a0
0x800249F4: beqz    $v0, loc_80024A14
0x800249F8: nop
0x800249FC: sll     $a0, 16
0x80024A00: lh      $a1, 0($s2)
0x80024A04: li      $a2, dword_8005407C
0x80024A0C: j       loc_80024A28
0x80024A10: sra     $a0, 16
0x80024A14: sll     $a0, 16
0x80024A18: sra     $a0, 16
0x80024A1C: lh      $a1, 0($s2)
0x80024A20: li      $a2, dword_8005408C
0x80024A28: jal     sub_8001C550
0x80024A2C: li      $a3, 3
0x80024A30: addiu   $s0, 1
0x80024A34: slti    $v0, $s0, 0xE
0x80024A38: bnez    $v0, loc_800249E8
0x80024A3C: addiu   $s1, 4
0x80024A40: move    $s0, $zero
0x80024A44: li      $s1, 0x20  # ' '
0x80024A48: lw      $s4, 0x48+var_18($sp)
0x80024A4C: lw      $s3, 0x48+var_20($sp)
0x80024A50: lw      $s2, 0x48+var_28($sp)
0x80024A54: addu    $v0, $s3, $s0
0x80024A58: sh      $s1, 0x48+var_38($sp)
0x80024A5C: sh      $s4, 0x48+var_36($sp)
0x80024A60: sh      $v0, 0x48+var_34($sp)
0x80024A64: lw      $v0, 0x94($s2)
0x80024A68: nop
0x80024A6C: addu    $v0, $s0
0x80024A70: lbu     $v0, 0($v0)
0x80024A74: nop
0x80024A78: sll     $v0, 24
0x80024A7C: sra     $v1, $v0, 24
0x80024A80: sh      $v1, 0x48+var_32($sp)
0x80024A84: lh      $v0, 0x7A($s6)
0x80024A88: li      $t0, 1
0x80024A8C: bne     $v0, $t0, loc_80024AAC
0x80024A90: addiu   $v0, $v1, -1
0x80024A94: andi    $v0, 0xFFFF
0x80024A98: sltiu   $v0, 8
0x80024A9C: beqz    $v0, loc_80024AAC
0x80024AA0: nop
0x80024AA4: jal     sub_80024418
0x80024AA8: addiu   $a0, $sp, 0x48+var_38
0x80024AAC: addiu   $s0, 1
0x80024AB0: slti    $v0, $s0, 0x12
0x80024AB4: bnez    $v0, loc_80024A54
0x80024AB8: addiu   $s1, 0xF
0x80024ABC: lw      $t0, 0x48+var_28($sp)
0x80024AC0: nop
0x80024AC4: addiu   $t0, 4
0x80024AC8: sw      $t0, 0x48+var_28($sp)
0x80024ACC: lw      $t0, 0x48+var_20($sp)
0x80024AD0: nop
0x80024AD4: addiu   $t0, 0x11
0x80024AD8: sw      $t0, 0x48+var_20($sp)
0x80024ADC: lw      $t0, 0x48+var_18($sp)
0x80024AE0: nop
0x80024AE4: addiu   $t0, 0x14
0x80024AE8: sw      $t0, 0x48+var_18($sp)
0x80024AEC: lw      $t0, 0x48+var_10($sp)
0x80024AF0: addiu   $s5, 2
0x80024AF4: addiu   $t0, 0x14
0x80024AF8: sw      $t0, 0x48+var_10($sp)
0x80024AFC: lw      $t0, 0x48+var_8($sp)
0x80024B00: nop
0x80024B04: addiu   $t0, 4
0x80024B08: sw      $t0, 0x48+var_8($sp)
0x80024B0C: lh      $v0, 0x8A($s6)
0x80024B10: addiu   $s7, 1
0x80024B14: slt     $v0, $s7, $v0
0x80024B18: bnez    $v0, loc_800247A4
0x80024B1C: addiu   $fp, 0x12
0x80024B20: lw      $ra, 0x48+var_s24($sp)
0x80024B24: lw      $fp, 0x48+var_s20($sp)
0x80024B28: lw      $s7, 0x48+var_s1C($sp)
0x80024B2C: lw      $s6, 0x48+var_s18($sp)
0x80024B30: lw      $s5, 0x48+var_s14($sp)
0x80024B34: lw      $s4, 0x48+var_s10($sp)
0x80024B38: lw      $s3, 0x48+var_sC($sp)
0x80024B3C: lw      $s2, 0x48+var_s8($sp)
0x80024B40: lw      $s1, 0x48+var_s4($sp)
0x80024B44: lw      $s0, 0x48+var_s0($sp)
0x80024B48: addiu   $sp, 0x70
0x80024B4C: jr      $ra
0x80024B50: nop
```

### Xrefs to
```text
<none>
```

### Xrefs from
```text
0x800247B4 -> 0x80023F20 type=17 target=sub_80023F20
0x80024850 -> 0x8001C550 type=17 target=sub_8001C550
0x80024868 -> 0x80024114 type=17 target=sub_80024114
0x800248D4 -> 0x8005400C type=1 target=dword_8005400C
0x80024908 -> 0x8001C550 type=17 target=sub_8001C550
0x80024944 -> 0x800540E0 type=1 target=dword_800540E0
0x80024950 -> 0x800540E8 type=1 target=dword_800540E8
0x80024978 -> 0x8005409C type=1 target=dword_8005409C
0x80024994 -> 0x800540AC type=1 target=dword_800540AC
0x8002499C -> 0x8001C550 type=17 target=sub_8001C550
0x800249C8 -> 0x800540F8 type=1 target=dword_800540F8
0x800249D4 -> 0x80054100 type=1 target=dword_80054100
0x80024A04 -> 0x8005407C type=1 target=dword_8005407C
0x80024A20 -> 0x8005408C type=1 target=dword_8005408C
0x80024A28 -> 0x8001C550 type=17 target=sub_8001C550
0x80024AA4 -> 0x80024418 type=17 target=sub_80024418
```

### Referenced globals / immediates
```text
0x8002477C op1 -> 0x80024B20 name=loc_80024B20 | blez    $v0, loc_80024B20
0x800247AC op1 -> 0x80024858 name=loc_80024858 | bltz    $a0, loc_80024858
0x800247B4 op0 -> 0x80023F20 name=sub_80023F20 | jal     sub_80023F20
0x800247C4 op2 -> 0x80024858 name=loc_80024858 | bne     $v0, $t0, loc_80024858
0x800247D8 op2 -> 0x800247F0 name=loc_800247F0 | beq     $v0, $v1, loc_800247F0
0x800247E8 op0 -> 0x80024800 name=loc_80024800 | j       loc_80024800
0x8002480C op1 -> 0x80024818 name=loc_80024818 | beqz    $v0, loc_80024818
0x80024818 op1 -> 0x80024858 name=loc_80024858 | blez    $v1, loc_80024858
0x80024850 op0 -> 0x8001C550 name=sub_8001C550 | jal     sub_8001C550
0x80024860 op1 -> 0x80024910 name=loc_80024910 | bltz    $a0, loc_80024910
0x80024868 op0 -> 0x80024114 name=sub_80024114 | jal     sub_80024114
0x80024878 op2 -> 0x80024A44 name=loc_80024A44 | bne     $v0, $t0, loc_80024A44
0x8002488C op2 -> 0x800248A4 name=loc_800248A4 | beq     $v0, $v1, loc_800248A4
0x8002489C op0 -> 0x800248B4 name=loc_800248B4 | j       loc_800248B4
0x800248C0 op1 -> 0x800248CC name=loc_800248CC | bnez    $v0, loc_800248CC
0x800248CC op1 -> 0x80024910 name=loc_80024910 | bltz    $v1, loc_80024910
0x80024908 op0 -> 0x8001C550 name=sub_8001C550 | jal     sub_8001C550
0x80024918 op2 -> 0x80024A44 name=loc_80024A44 | bne     $v0, $t0, loc_80024A44
0x80024928 op1 -> 0x8002493C name=loc_8002493C | blez    $v1, loc_8002493C
0x80024934 op0 -> 0x80024944 name=loc_80024944 | j       loc_80024944
0x80024968 op1 -> 0x80024988 name=loc_80024988 | beqz    $v0, loc_80024988
0x80024980 op0 -> 0x8002499C name=loc_8002499C | j       loc_8002499C
0x8002499C op0 -> 0x8001C550 name=sub_8001C550 | jal     sub_8001C550
0x800249AC op1 -> 0x8002495C name=loc_8002495C | bnez    $v0, loc_8002495C
0x800249C0 op1 -> 0x80024A40 name=loc_80024A40 | beqz    $v0, loc_80024A40
0x800249F4 op1 -> 0x80024A14 name=loc_80024A14 | beqz    $v0, loc_80024A14
0x80024A0C op0 -> 0x80024A28 name=loc_80024A28 | j       loc_80024A28
0x80024A28 op0 -> 0x8001C550 name=sub_8001C550 | jal     sub_8001C550
0x80024A38 op1 -> 0x800249E8 name=loc_800249E8 | bnez    $v0, loc_800249E8
0x80024A8C op2 -> 0x80024AAC name=loc_80024AAC | bne     $v0, $t0, loc_80024AAC
0x80024A9C op1 -> 0x80024AAC name=loc_80024AAC | beqz    $v0, loc_80024AAC
0x80024AA4 op0 -> 0x80024418 name=sub_80024418 | jal     sub_80024418
0x80024AB4 op1 -> 0x80024A54 name=loc_80024A54 | bnez    $v0, loc_80024A54
0x80024B18 op1 -> 0x800247A4 name=loc_800247A4 | bnez    $v0, loc_800247A4
```
