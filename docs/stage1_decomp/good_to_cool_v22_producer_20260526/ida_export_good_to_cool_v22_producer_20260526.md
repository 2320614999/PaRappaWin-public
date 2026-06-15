# IDA export: Stage1 GOOD->COOL v22 producer

- Date: 2026-05-26
- Input: `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`
- Root: `SCUS_941.83`
- Purpose: close accepted producer / ED36-ED38 / resolutionV22 evidence after second-run GOOD->COOL setup gate proved insufficient.

## Function summary

| Address | Name | Bounds | Purpose |
| --- | --- | --- | --- |
| `0x80014614` | `sub_80014614` | `0x80014614-0x80014A80` | accepted producer / source cell materialize |
| `0x80014D58` | `sub_80014D58` | `0x80014D58-0x800152D0` | accumulator/page-history commit |
| `0x80014D28` | `sub_80014D28` | `0x80014D28-0x80014D58` | positive-growth classifier |
| `0x80014548` | `sub_80014548` | `0x80014548-0x80014614` | bucket30 tie-breaker |
| `0x80024FD0` | `sub_80024FD0` | `0x80024FD0-0x800259C0` | bucket dispatcher / bucket30 owner |
| `0x80024B54` | `sub_80024B54` | `0x80024B54-0x80024BC0` | input class/token dispatch |
| `0x80035510` | `sub_80035510` | `0x80035510-0x80035540` | input/read wrapper |
| `0x801C9094` | `sub_801C9094` | `<missing func>` | COMOD1 script/scorer bridge |
| `0x801C7A60` | `sub_801C7A60` | `<missing func>` | COMOD1 stage1 main loop |

## 0x80014614 `sub_80014614` - accepted producer / source cell materialize

- Bounds: `0x80014614` - `0x80014A80`
- Size: `1132` bytes

### Hex-Rays pseudo-C

```c
int __fastcall sub_80014614(int a1)
{
  bool v2; // dc
  int v3; // $gp
  int result; // $v0
  int v5; // $t0
  int v6; // $a1
  int v7; // $a3
  int v8; // $t1
  int v9; // $a1
  int v10; // $a0
  _BYTE *v11; // $v0
  int v12; // $s3
  BOOL v13; // $s4
  int v14; // $v1
  int v15; // $v1
  int v16; // $v0
  int v17; // $a0
  __int16 v18; // $v0
  int v19; // $a1
  int v20; // $s2
  __int16 v21; // $hi
  int v22; // $gp
  int v23; // $s1
  int v24; // $a0
  __int16 v25; // $v0
  int v26; // $gp
  int v27; // $s0
  __int16 v28; // $v1
  int v29; // $a0
  int v30; // [sp+10h] [-20h]

  v2 = sub_80024BF4(a1);
  result = -1;
  if ( !v2 )
  {
    result = -2;
    if ( g_PrStageEventStreamFlag )
    {
      result = -3;
      if ( *(_DWORD *)(a1 + 68) )
      {
        v5 = *(_DWORD *)(a1 + 24);
        if ( v5 == 1 )
        {
          LOWORD(v5) = 4;
        }
        else if ( v5 == 2 )
        {
          LOWORD(v5) = 8;
        }
        v6 = *(_DWORD *)(a1 + 68) + 6 * *(__int16 *)(a1 + 80);
        if ( *(_BYTE *)(v6 + 12) )
        {
          v7 = *(_DWORD *)(4 * *(_DWORD *)(a1 + 32) + 36 * *(unsigned __int8 *)(v6 + 12) + dword_800943D8);
          result = -6;
          if ( v7 )
          {
            v8 = *(unsigned __int8 *)(v6 + 13);
            if ( *(_BYTE *)(v6 + 13) )
            {
              v9 = *(_DWORD *)(a1 + 52);
              v10 = (*(_DWORD *)(a1 + 16) + v9) % 384;
              v30 = v10 / 24;
              v11 = (_BYTE *)(16 * v8 + dword_800943E0 + v10 / 24 / 2);
              v12 = (unsigned __int8)*v11;
              v13 = 2 * v9 >= v10 % 24;
              if ( *v11 )
              {
                v14 = *(_DWORD *)(a1 + 32);
                if ( dword_80091800 == v14 || !v14 )
                {
                  *(_DWORD *)(v3 + 32) = 0;
                }
                else
                {
                  *(_WORD *)(v7 + 6) = 0;
                  *(_DWORD *)(v3 + 32) = 1;
                }
                v15 = *(_DWORD *)(v3 + 32);
                dword_80091800 = *(_DWORD *)(a1 + 32);
                v16 = v5 & 0x8000;
                if ( !v15 )
                {
                  v17 = *(__int16 *)(v7 + 4);
                  v16 = v5 & 0x8000;
                  if ( v17 >= 2 )
                  {
                    v16 = v5 & 0x8000;
                    if ( (v5 & 0x2000) != 0 )
                    {
                      if ( *(__int16 *)(v7 + 6) <= 0 )
                        v18 = v17 - 1;
                      else
                        v18 = *(_WORD *)(v7 + 6) - 1;
                      *(_WORD *)(v7 + 6) = v18;
                      v16 = v5 & 0x8000;
                    }
                  }
                }
                if ( v16 )
                  *(_WORD *)(v7 + 6) = 0;
                v19 = *(__int16 *)(v7 + 6);
                v20 = *(_DWORD *)v7 + 12 * v19;
                if ( *(__int16 *)(v7 + 4) >= 2 )
                {
                  v21 = (__int16)(v19 + 1) % *(_WORD *)(v7 + 4);
                  if ( !*(_WORD *)(v7 + 4) )
                    _break(7u, 0);
                  *(_WORD *)(v7 + 6) = v19 + 1;
                  *(_WORD *)(v7 + 6) = v21;
                }
                sub_80026FC4(v20);
                if ( *(_DWORD *)(v20 + 8) )
                  dword_80094430(a1);
                result = -9;
                if ( v12 == 2 )
                {
                  v23 = *(_DWORD *)(a1 + 24);
                  if ( v23 == 1 )
                  {
                    v23 = 4;
                  }
                  else if ( v23 == 2 )
                  {
                    v23 = 8;
                  }
                  if ( !*(_WORD *)(a1 + 82) )
                  {
                    v24 = *(_DWORD *)(a1 + 16);
                    if ( dword_800901C0 < 600 )
                    {
                      dword_8008EEFC[2 * dword_800901C0] = v23;
                      *((_DWORD *)&byte_8008EEF8 + 2 * dword_800901C0++) = v24;
                      dword_800901BC = dword_800901C0;
                    }
                  }
                  if ( v13 )
                  {
                    *(_DWORD *)(12 * v30 + *(_DWORD *)(v22 + 28)) = v23;
                    v25 = sub_80024B54(v23);
                    v27 = 12 * v30 + *(_DWORD *)(v26 + 28);
                    v28 = *(_WORD *)(v27 + 6);
                    *(_WORD *)(v27 + 4) = v25;
                    *(_DWORD *)(v27 + 8) = v20;
                    *(_WORD *)(v27 + 6) = v28 + 1;
                    ++word_80091812;
                    v29 = 1;
                  }
                  else
                  {
                    v29 = 0;
                    ++word_80091814;
                  }
                  ++word_80091810;
                  dword_80091808 |= v23;
                  result = 0;
                  if ( v29 == 1 )
                  {
                    word_80091824 = *(_WORD *)(v20 + 6);
                    return 0;
                  }
                }
              }
              else
              {
                return -8;
              }
            }
            else
            {
              return -7;
            }
          }
        }
        else
        {
          return -5;
        }
      }
    }
  }
  return result;
}

```

### Disassembly

```asm
0x80014614: addiu   $sp, -0x48
0x80014618: sw      $s0, 0x30+var_s0($sp)
0x8001461C: move    $s0, $a0
0x80014620: sw      $ra, 0x30+var_s14($sp)
0x80014624: sw      $s4, 0x30+var_s10($sp)
0x80014628: sw      $s3, 0x30+var_sC($sp)
0x8001462C: sw      $s2, 0x30+var_s8($sp)
0x80014630: jal     sub_80024BF4
0x80014634: sw      $s1, 0x30+var_s4($sp)
0x80014638: li      $v1, 1
0x8001463C: beq     $v0, $v1, loc_80014A5C
0x80014640: li      $v0, 0xFFFFFFFF
0x80014644: lh      $v0, g_PrStageEventStreamFlag
0x8001464C: nop
0x80014650: beqz    $v0, loc_80014A5C
0x80014654: li      $v0, 0xFFFFFFFE
0x80014658: lw      $v0, 0x44($s0)
0x8001465C: nop
0x80014660: beqz    $v0, loc_80014A5C
0x80014664: li      $v0, 0xFFFFFFFD
0x80014668: lw      $v0, 0x18($s0)
0x8001466C: nop
0x80014670: bne     $v0, $v1, loc_80014680
0x80014674: move    $t0, $v0
0x80014678: j       loc_80014690
0x8001467C: li      $t0, 4
0x80014680: li      $v0, 2
0x80014684: bne     $t0, $v0, loc_80014690
0x80014688: nop
0x8001468C: li      $t0, 8
0x80014690: lh      $v0, 0x50($s0)
0x80014694: nop
0x80014698: sll     $v1, $v0, 1
0x8001469C: addu    $v1, $v0
0x800146A0: lw      $v0, 0x44($s0)
0x800146A4: sll     $v1, 1
0x800146A8: addu    $a1, $v0, $v1
0x800146AC: lbu     $v1, 0xC($a1)
0x800146B0: nop
0x800146B4: bnez    $v1, loc_800146C4
0x800146B8: sll     $v0, $v1, 3
0x800146BC: j       loc_80014A5C
0x800146C0: li      $v0, 0xFFFFFFFB
0x800146C4: addu    $v0, $v1
0x800146C8: sll     $v0, 2
0x800146CC: lw      $a0, dword_800943D8
0x800146D4: lw      $v1, 0x20($s0)
0x800146D8: addu    $v0, $a0
0x800146DC: sll     $v1, 2
0x800146E0: addu    $v1, $v0
0x800146E4: lw      $a3, 0($v1)
0x800146E8: nop
0x800146EC: beqz    $a3, loc_80014A5C
0x800146F0: li      $v0, 0xFFFFFFFA
0x800146F4: lbu     $t1, 0xD($a1)
0x800146F8: nop
0x800146FC: bnez    $t1, loc_8001470C
0x80014700: lui     $a2, 0x2AAA
0x80014704: j       loc_80014A5C
0x80014708: li      $v0, 0xFFFFFFF9
0x8001470C: lw      $v1, 0x10($s0)
0x80014710: lw      $a1, 0x34($s0)
0x80014714: li      $a2, 0x2AAAAAAB
0x80014718: addu    $v1, $a1
0x8001471C: mult    $v1, $a2
0x80014720: sra     $v0, $v1, 31
0x80014724: mfhi    $t2
0x80014728: sra     $a0, $t2, 6
0x8001472C: subu    $a0, $v0
0x80014730: sll     $v0, $a0, 1
0x80014734: addu    $v0, $a0
0x80014738: sll     $v0, 7
0x8001473C: subu    $a0, $v1, $v0
0x80014740: mult    $a0, $a2
0x80014744: sll     $a1, 1
0x80014748: sra     $v0, $a0, 31
0x8001474C: mfhi    $t2
0x80014750: sra     $v1, $t2, 2
0x80014754: subu    $v0, $v1, $v0
0x80014758: move    $v1, $v0
0x8001475C: sll     $v0, $v1, 1
0x80014760: addu    $v0, $v1
0x80014764: sll     $v0, 3
0x80014768: subu    $v0, $a0, $v0
0x8001476C: slt     $a1, $v0
0x80014770: lw      $a0, dword_800943E0
0x80014778: sll     $v0, $t1, 4
0x8001477C: sw      $v1, 0x30+var_20($sp)
0x80014780: addu    $v0, $a0
0x80014784: srl     $a0, $v1, 31
0x80014788: addu    $v1, $a0
0x8001478C: sra     $v1, 1
0x80014790: addu    $v0, $v1
0x80014794: lbu     $s3, 0($v0)
0x80014798: nop
0x8001479C: bnez    $s3, loc_800147AC
0x800147A0: xori    $s4, $a1, 1
0x800147A4: j       loc_80014A5C
0x800147A8: li      $v0, 0xFFFFFFF8
0x800147AC: lw      $v0, dword_80091800
0x800147B4: lw      $v1, 0x20($s0)
0x800147B8: nop
0x800147BC: beq     $v0, $v1, loc_800147DC
0x800147C0: nop
0x800147C4: beqz    $v1, loc_800147DC
0x800147C8: li      $v0, 1
0x800147CC: sh      $zero, 6($a3)
0x800147D0: sw      $v0, 0x20($gp)
0x800147D4: j       loc_800147E0
0x800147D8: nop
0x800147DC: sw      $zero, 0x20($gp)
0x800147E0: lw      $v0, 0x20($s0)
0x800147E4: lw      $v1, 0x20($gp)
0x800147E8: sw      $v0, dword_80091800
0x800147F0: bnez    $v1, loc_80014840
0x800147F4: andi    $v0, $t0, 0x8000
0x800147F8: lh      $v0, 4($a3)
0x800147FC: nop
0x80014800: move    $a0, $v0
0x80014804: slti    $v0, 2
0x80014808: bnez    $v0, loc_80014840
0x8001480C: andi    $v0, $t0, 0x8000
0x80014810: andi    $v0, $t0, 0x2000
0x80014814: beqz    $v0, loc_80014840
0x80014818: andi    $v0, $t0, 0x8000
0x8001481C: lh      $v0, 6($a3)
0x80014820: nop
0x80014824: blez    $v0, loc_80014834
0x80014828: move    $v1, $v0
0x8001482C: j       loc_80014838
0x80014830: addiu   $v0, $v1, -1
0x80014834: addiu   $v0, $a0, -1
0x80014838: sh      $v0, 6($a3)
0x8001483C: andi    $v0, $t0, 0x8000
0x80014840: beqz    $v0, loc_8001484C
0x80014844: nop
0x80014848: sh      $zero, 6($a3)
0x8001484C: lh      $v0, 6($a3)
0x80014850: lw      $a0, 0($a3)
0x80014854: move    $a1, $v0
0x80014858: sll     $v1, $v0, 1
0x8001485C: addu    $v1, $v0
0x80014860: sll     $v1, 2
0x80014864: lh      $v0, 4($a3)
0x80014868: nop
0x8001486C: slti    $v0, 2
0x80014870: bnez    $v0, loc_800148BC
0x80014874: addu    $s2, $a0, $v1
0x80014878: addiu   $a0, $a1, 1
0x8001487C: sll     $v0, $a0, 16
0x80014880: lh      $v1, 4($a3)
0x80014884: sra     $v0, 16
0x80014888: div     $v0, $v1
0x8001488C: bnez    $v1, loc_80014898
0x80014890: nop
0x80014894: break   7
0x80014898: li      $at, 0xFFFFFFFF
0x8001489C: bne     $v1, $at, loc_800148B0
0x800148A0: lui     $at, 0x8000
0x800148A4: bne     $v0, $at, loc_800148B0
0x800148A8: nop
0x800148AC: break   6
0x800148B0: mfhi    $v1
0x800148B4: sh      $a0, 6($a3)
0x800148B8: sh      $v1, 6($a3)
0x800148BC: jal     sub_80026FC4
0x800148C0: move    $a0, $s2
0x800148C4: lw      $a1, 8($s2)
0x800148C8: nop
0x800148CC: beqz    $a1, loc_800148EC
0x800148D0: li      $v0, 2
0x800148D4: lw      $v0, dword_80094430
0x800148DC: nop
0x800148E0: jalr    $v0
0x800148E4: move    $a0, $s0
0x800148E8: li      $v0, 2
0x800148EC: bne     $s3, $v0, loc_80014A5C
0x800148F0: li      $v0, 0xFFFFFFF7
0x800148F4: li      $v0, 1
0x800148F8: lw      $a0, 0x18($s0)
0x800148FC: lw      $a1, 0x30+var_20($sp)
0x80014900: bne     $a0, $v0, loc_80014910
0x80014904: move    $s1, $a0
0x80014908: j       loc_8001491C
0x8001490C: li      $s1, 4
0x80014910: bne     $s1, $s3, loc_8001491C
0x80014914: nop
0x80014918: li      $s1, 8
0x8001491C: lh      $v0, 0x52($s0)
0x80014920: nop
0x80014924: bnez    $v0, loc_80014998
0x80014928: li      $v0, 1
0x8001492C: lw      $v1, dword_800901C0
0x80014934: lw      $a0, 0x10($s0)
0x80014938: slti    $v0, $v1, 0x258
0x8001493C: beqz    $v0, loc_80014994
0x80014940: sll     $v0, $v1, 3
0x80014944: li      $at, dword_8008EEFC
0x8001494C: addu    $at, $v0
0x80014950: sw      $s1, 0($at)
0x80014954: lw      $v0, dword_800901C0
0x8001495C: nop
0x80014960: sll     $v0, 3
0x80014964: li      $at, byte_8008EEF8
0x8001496C: addu    $at, $v0
0x80014970: sw      $a0, 0($at)
0x80014974: lw      $v0, dword_800901C0
0x8001497C: nop
0x80014980: addiu   $v0, 1
0x80014984: sw      $v0, dword_800901C0
0x8001498C: sw      $v0, dword_800901BC
0x80014994: li      $v0, 1
0x80014998: bne     $s4, $v0, loc_800149FC
0x8001499C: move    $a0, $s1
0x800149A0: sll     $s0, $a1, 1
0x800149A4: addu    $s0, $a1
0x800149A8: lw      $v0, 0x1C($gp)
0x800149AC: sll     $s0, 2
0x800149B0: addu    $v0, $s0, $v0
0x800149B4: jal     sub_80024B54
0x800149B8: sw      $s1, 0($v0)
0x800149BC: lw      $v1, 0x1C($gp)
0x800149C0: nop
0x800149C4: addu    $s0, $v1
0x800149C8: lhu     $v1, 6($s0)
0x800149CC: sh      $v0, 4($s0)
0x800149D0: sw      $s2, 8($s0)
0x800149D4: addiu   $v1, 1
0x800149D8: sh      $v1, 6($s0)
0x800149DC: lhu     $v0, word_80091812
0x800149E4: nop
0x800149E8: addiu   $v0, 1
0x800149EC: sh      $v0, word_80091812
0x800149F4: j       loc_80014A14
0x800149F8: li      $a0, 1
0x800149FC: lhu     $v0, word_80091814
0x80014A04: move    $a0, $zero
0x80014A08: addiu   $v0, 1
0x80014A0C: sh      $v0, word_80091814
0x80014A14: lhu     $v0, word_80091810
0x80014A1C: lw      $v1, dword_80091808
0x80014A24: addiu   $v0, 1
0x80014A28: or      $v1, $s1
0x80014A2C: sh      $v0, word_80091810
0x80014A34: li      $v0, 1
0x80014A38: sw      $v1, dword_80091808
0x80014A40: bne     $a0, $v0, loc_80014A5C
0x80014A44: move    $v0, $zero
0x80014A48: lhu     $v0, 6($s2)
0x80014A4C: nop
0x80014A50: sh      $v0, word_80091824
0x80014A58: move    $v0, $zero
0x80014A5C: lw      $ra, 0x30+var_s14($sp)
0x80014A60: lw      $s4, 0x30+var_s10($sp)
0x80014A64: lw      $s3, 0x30+var_sC($sp)
0x80014A68: lw      $s2, 0x30+var_s8($sp)
0x80014A6C: lw      $s1, 0x30+var_s4($sp)
0x80014A70: lw      $s0, 0x30+var_s0($sp)
0x80014A74: addiu   $sp, 0x48
0x80014A78: jr      $ra
0x80014A7C: nop
```

### Xrefs to

- <none>

### Xrefs from

- `0x80014614` -> `0x80014618` type=21 to `sub_80014618`
- `0x80014618` -> `0x8001461C` type=21 to `sub_8001461C`
- `0x8001461C` -> `0x80014620` type=21 to `sub_80014620`
- `0x80014620` -> `0x80014624` type=21 to `sub_80014624`
- `0x80014624` -> `0x80014628` type=21 to `sub_80014628`
- `0x80014628` -> `0x8001462C` type=21 to `sub_8001462C`
- `0x8001462C` -> `0x80014630` type=21 to `sub_80014630`
- `0x80014630` -> `0x80014634` type=21 to `sub_80014634`
- `0x80014630` -> `0x80024BF4` type=17 to `sub_80024BF4`
- `0x80014634` -> `0x80014638` type=21 to `sub_80014638`
- `0x80014638` -> `0x8001463C` type=21 to `sub_8001463C`
- `0x8001463C` -> `0x80014640` type=21 to `sub_80014640`
- `0x8001463C` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x80014640` -> `0x80014644` type=21 to `sub_80014644`
- `0x80014644` -> `0x8001464C` type=21 to `sub_8001464C`
- `0x80014644` -> `0x8008ED2E` type=3 to `g_PrStageEventStreamFlag`
- `0x8001464C` -> `0x80014650` type=21 to `sub_80014650`
- `0x80014650` -> `0x80014654` type=21 to `sub_80014654`
- `0x80014650` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x80014654` -> `0x80014658` type=21 to `sub_80014658`
- `0x80014658` -> `0x8001465C` type=21 to `sub_8001465C`
- `0x8001465C` -> `0x80014660` type=21 to `sub_80014660`
- `0x80014660` -> `0x80014664` type=21 to `sub_80014664`
- `0x80014660` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x80014664` -> `0x80014668` type=21 to `sub_80014668`
- `0x80014668` -> `0x8001466C` type=21 to `sub_8001466C`
- `0x8001466C` -> `0x80014670` type=21 to `sub_80014670`
- `0x80014670` -> `0x80014674` type=21 to `sub_80014674`
- `0x80014670` -> `0x80014680` type=19 to `loc_80014680`
- `0x80014674` -> `0x80014678` type=21 to `sub_80014678`
- `0x80014678` -> `0x8001467C` type=21 to `sub_8001467C`
- `0x80014678` -> `0x80014690` type=19 to `loc_80014690`
- `0x80014680` -> `0x80014684` type=21 to `sub_80014684`
- `0x80014684` -> `0x80014688` type=21 to `sub_80014688`
- `0x80014684` -> `0x80014690` type=19 to `loc_80014690`
- `0x80014688` -> `0x8001468C` type=21 to `sub_8001468C`
- `0x8001468C` -> `0x80014690` type=21 to `loc_80014690`
- `0x80014690` -> `0x80014694` type=21 to `sub_80014694`
- `0x80014694` -> `0x80014698` type=21 to `sub_80014698`
- `0x80014698` -> `0x8001469C` type=21 to `sub_8001469C`
- `0x8001469C` -> `0x800146A0` type=21 to `sub_800146A0`
- `0x800146A0` -> `0x800146A4` type=21 to `sub_800146A4`
- `0x800146A4` -> `0x800146A8` type=21 to `sub_800146A8`
- `0x800146A8` -> `0x800146AC` type=21 to `sub_800146AC`
- `0x800146AC` -> `0x800146B0` type=21 to `sub_800146B0`
- `0x800146B0` -> `0x800146B4` type=21 to `sub_800146B4`
- `0x800146B4` -> `0x800146B8` type=21 to `sub_800146B8`
- `0x800146B4` -> `0x800146C4` type=19 to `loc_800146C4`
- `0x800146B8` -> `0x800146BC` type=21 to `sub_800146BC`
- `0x800146BC` -> `0x800146C0` type=21 to `sub_800146C0`
- `0x800146BC` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x800146C4` -> `0x800146C8` type=21 to `sub_800146C8`
- `0x800146C8` -> `0x800146CC` type=21 to `sub_800146CC`
- `0x800146CC` -> `0x800146D4` type=21 to `sub_800146D4`
- `0x800146CC` -> `0x800943D8` type=3 to `dword_800943D8`
- `0x800146D4` -> `0x800146D8` type=21 to `sub_800146D8`
- `0x800146D8` -> `0x800146DC` type=21 to `sub_800146DC`
- `0x800146DC` -> `0x800146E0` type=21 to `sub_800146E0`
- `0x800146E0` -> `0x800146E4` type=21 to `sub_800146E4`
- `0x800146E4` -> `0x800146E8` type=21 to `sub_800146E8`
- `0x800146E8` -> `0x800146EC` type=21 to `sub_800146EC`
- `0x800146EC` -> `0x800146F0` type=21 to `sub_800146F0`
- `0x800146EC` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x800146F0` -> `0x800146F4` type=21 to `sub_800146F4`
- `0x800146F4` -> `0x800146F8` type=21 to `sub_800146F8`
- `0x800146F8` -> `0x800146FC` type=21 to `sub_800146FC`
- `0x800146FC` -> `0x80014700` type=21 to `sub_80014700`
- `0x800146FC` -> `0x8001470C` type=19 to `loc_8001470C`
- `0x80014700` -> `0x80014704` type=21 to `sub_80014704`
- `0x80014704` -> `0x80014708` type=21 to `sub_80014708`
- `0x80014704` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x8001470C` -> `0x80014710` type=21 to `sub_80014710`
- `0x80014710` -> `0x80014714` type=21 to `sub_80014714`
- `0x80014714` -> `0x80014718` type=21 to `sub_80014718`
- `0x80014718` -> `0x8001471C` type=21 to `sub_8001471C`
- `0x8001471C` -> `0x80014720` type=21 to `sub_80014720`
- `0x80014720` -> `0x80014724` type=21 to `sub_80014724`
- `0x80014724` -> `0x80014728` type=21 to `sub_80014728`
- `0x80014728` -> `0x8001472C` type=21 to `sub_8001472C`
- `0x8001472C` -> `0x80014730` type=21 to `sub_80014730`
- `0x80014730` -> `0x80014734` type=21 to `sub_80014734`
- `0x80014734` -> `0x80014738` type=21 to `sub_80014738`
- `0x80014738` -> `0x8001473C` type=21 to `sub_8001473C`
- `0x8001473C` -> `0x80014740` type=21 to `sub_80014740`
- `0x80014740` -> `0x80014744` type=21 to `sub_80014744`
- `0x80014744` -> `0x80014748` type=21 to `sub_80014748`
- `0x80014748` -> `0x8001474C` type=21 to `sub_8001474C`
- `0x8001474C` -> `0x80014750` type=21 to `sub_80014750`
- `0x80014750` -> `0x80014754` type=21 to `sub_80014754`
- `0x80014754` -> `0x80014758` type=21 to `sub_80014758`
- `0x80014758` -> `0x8001475C` type=21 to `sub_8001475C`
- `0x8001475C` -> `0x80014760` type=21 to `sub_80014760`
- `0x80014760` -> `0x80014764` type=21 to `sub_80014764`
- `0x80014764` -> `0x80014768` type=21 to `sub_80014768`
- `0x80014768` -> `0x8001476C` type=21 to `sub_8001476C`
- `0x8001476C` -> `0x80014770` type=21 to `sub_80014770`
- `0x80014770` -> `0x80014778` type=21 to `sub_80014778`
- `0x80014770` -> `0x800943E0` type=3 to `dword_800943E0`
- `0x80014778` -> `0x8001477C` type=21 to `sub_8001477C`
- `0x8001477C` -> `0x80014780` type=21 to `sub_80014780`
- `0x80014780` -> `0x80014784` type=21 to `sub_80014784`
- `0x80014784` -> `0x80014788` type=21 to `sub_80014788`
- `0x80014788` -> `0x8001478C` type=21 to `sub_8001478C`
- `0x8001478C` -> `0x80014790` type=21 to `sub_80014790`
- `0x80014790` -> `0x80014794` type=21 to `sub_80014794`
- `0x80014794` -> `0x80014798` type=21 to `sub_80014798`
- `0x80014798` -> `0x8001479C` type=21 to `sub_8001479C`
- `0x8001479C` -> `0x800147A0` type=21 to `sub_800147A0`
- `0x8001479C` -> `0x800147AC` type=19 to `loc_800147AC`
- `0x800147A0` -> `0x800147A4` type=21 to `sub_800147A4`
- `0x800147A4` -> `0x800147A8` type=21 to `sub_800147A8`
- `0x800147A4` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x800147AC` -> `0x800147B4` type=21 to `sub_800147B4`
- `0x800147AC` -> `0x80091800` type=3 to `dword_80091800`
- `0x800147B4` -> `0x800147B8` type=21 to `sub_800147B8`
- `0x800147B8` -> `0x800147BC` type=21 to `sub_800147BC`
- `0x800147BC` -> `0x800147C0` type=21 to `sub_800147C0`
- `0x800147BC` -> `0x800147DC` type=19 to `loc_800147DC`
- `0x800147C0` -> `0x800147C4` type=21 to `sub_800147C4`
- `0x800147C4` -> `0x800147C8` type=21 to `sub_800147C8`
- `0x800147C4` -> `0x800147DC` type=19 to `loc_800147DC`
- `0x800147C8` -> `0x800147CC` type=21 to `sub_800147CC`
- `0x800147CC` -> `0x800147D0` type=21 to `sub_800147D0`
- `0x800147D0` -> `0x800147D4` type=21 to `sub_800147D4`
- `0x800147D4` -> `0x800147D8` type=21 to `sub_800147D8`
- `0x800147D4` -> `0x800147E0` type=19 to `loc_800147E0`
- `0x800147DC` -> `0x800147E0` type=21 to `loc_800147E0`
- `0x800147E0` -> `0x800147E4` type=21 to `sub_800147E4`
- `0x800147E4` -> `0x800147E8` type=21 to `sub_800147E8`
- `0x800147E8` -> `0x800147F0` type=21 to `sub_800147F0`
- `0x800147E8` -> `0x80091800` type=2 to `dword_80091800`
- `0x800147F0` -> `0x800147F4` type=21 to `sub_800147F4`
- `0x800147F0` -> `0x80014840` type=19 to `loc_80014840`
- `0x800147F4` -> `0x800147F8` type=21 to `sub_800147F8`
- `0x800147F8` -> `0x800147FC` type=21 to `sub_800147FC`
- `0x800147FC` -> `0x80014800` type=21 to `sub_80014800`
- `0x80014800` -> `0x80014804` type=21 to `sub_80014804`
- `0x80014804` -> `0x80014808` type=21 to `sub_80014808`
- `0x80014808` -> `0x8001480C` type=21 to `sub_8001480C`
- `0x80014808` -> `0x80014840` type=19 to `loc_80014840`
- `0x8001480C` -> `0x80014810` type=21 to `sub_80014810`
- `0x80014810` -> `0x80014814` type=21 to `sub_80014814`
- `0x80014814` -> `0x80014818` type=21 to `sub_80014818`
- `0x80014814` -> `0x80014840` type=19 to `loc_80014840`
- `0x80014818` -> `0x8001481C` type=21 to `sub_8001481C`
- `0x8001481C` -> `0x80014820` type=21 to `sub_80014820`
- `0x80014820` -> `0x80014824` type=21 to `sub_80014824`
- `0x80014824` -> `0x80014828` type=21 to `sub_80014828`
- `0x80014824` -> `0x80014834` type=19 to `loc_80014834`
- `0x80014828` -> `0x8001482C` type=21 to `sub_8001482C`
- `0x8001482C` -> `0x80014830` type=21 to `sub_80014830`
- `0x8001482C` -> `0x80014838` type=19 to `loc_80014838`
- `0x80014834` -> `0x80014838` type=21 to `loc_80014838`
- `0x80014838` -> `0x8001483C` type=21 to `sub_8001483C`
- `0x8001483C` -> `0x80014840` type=21 to `loc_80014840`
- `0x80014840` -> `0x80014844` type=21 to `sub_80014844`
- `0x80014840` -> `0x8001484C` type=19 to `loc_8001484C`
- `0x80014844` -> `0x80014848` type=21 to `sub_80014848`
- `0x80014848` -> `0x8001484C` type=21 to `loc_8001484C`
- `0x8001484C` -> `0x80014850` type=21 to `sub_80014850`
- `0x80014850` -> `0x80014854` type=21 to `sub_80014854`
- `0x80014854` -> `0x80014858` type=21 to `sub_80014858`
- `0x80014858` -> `0x8001485C` type=21 to `sub_8001485C`
- `0x8001485C` -> `0x80014860` type=21 to `sub_80014860`
- `0x80014860` -> `0x80014864` type=21 to `sub_80014864`
- `0x80014864` -> `0x80014868` type=21 to `sub_80014868`
- `0x80014868` -> `0x8001486C` type=21 to `sub_8001486C`
- `0x8001486C` -> `0x80014870` type=21 to `sub_80014870`
- `0x80014870` -> `0x80014874` type=21 to `sub_80014874`
- `0x80014870` -> `0x800148BC` type=19 to `loc_800148BC`
- `0x80014874` -> `0x80014878` type=21 to `sub_80014878`
- `0x80014878` -> `0x8001487C` type=21 to `sub_8001487C`
- `0x8001487C` -> `0x80014880` type=21 to `sub_80014880`
- `0x80014880` -> `0x80014884` type=21 to `sub_80014884`
- `0x80014884` -> `0x80014888` type=21 to `sub_80014888`
- `0x80014888` -> `0x8001488C` type=21 to `sub_8001488C`
- `0x8001488C` -> `0x80014890` type=21 to `sub_80014890`
- `0x8001488C` -> `0x80014898` type=19 to `loc_80014898`
- `0x80014890` -> `0x80014894` type=21 to `sub_80014894`
- `0x80014898` -> `0x8001489C` type=21 to `sub_8001489C`
- `0x8001489C` -> `0x800148A0` type=21 to `sub_800148A0`
- `0x8001489C` -> `0x800148B0` type=19 to `loc_800148B0`
- `0x800148A0` -> `0x800148A4` type=21 to `sub_800148A4`
- `0x800148A4` -> `0x800148A8` type=21 to `sub_800148A8`
- `0x800148A4` -> `0x800148B0` type=19 to `loc_800148B0`
- `0x800148A8` -> `0x800148AC` type=21 to `sub_800148AC`
- `0x800148B0` -> `0x800148B4` type=21 to `sub_800148B4`
- `0x800148B4` -> `0x800148B8` type=21 to `sub_800148B8`
- `0x800148B8` -> `0x800148BC` type=21 to `loc_800148BC`
- `0x800148BC` -> `0x800148C0` type=21 to `sub_800148C0`
- `0x800148BC` -> `0x80026FC4` type=17 to `sub_80026FC4`
- `0x800148C0` -> `0x800148C4` type=21 to `sub_800148C4`
- `0x800148C4` -> `0x800148C8` type=21 to `sub_800148C8`
- `0x800148C8` -> `0x800148CC` type=21 to `sub_800148CC`
- `0x800148CC` -> `0x800148D0` type=21 to `sub_800148D0`
- `0x800148CC` -> `0x800148EC` type=19 to `loc_800148EC`
- `0x800148D0` -> `0x800148D4` type=21 to `sub_800148D4`
- `0x800148D4` -> `0x800148DC` type=21 to `sub_800148DC`
- `0x800148D4` -> `0x80094430` type=3 to `dword_80094430`
- `0x800148DC` -> `0x800148E0` type=21 to `sub_800148E0`
- `0x800148E0` -> `0x800148E4` type=21 to `sub_800148E4`
- `0x800148E4` -> `0x800148E8` type=21 to `sub_800148E8`
- `0x800148E8` -> `0x800148EC` type=21 to `loc_800148EC`
- `0x800148EC` -> `0x800148F0` type=21 to `sub_800148F0`
- `0x800148EC` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x800148F0` -> `0x800148F4` type=21 to `sub_800148F4`
- `0x800148F4` -> `0x800148F8` type=21 to `sub_800148F8`
- `0x800148F8` -> `0x800148FC` type=21 to `sub_800148FC`
- `0x800148FC` -> `0x80014900` type=21 to `sub_80014900`
- `0x80014900` -> `0x80014904` type=21 to `sub_80014904`
- `0x80014900` -> `0x80014910` type=19 to `loc_80014910`
- `0x80014904` -> `0x80014908` type=21 to `sub_80014908`
- `0x80014908` -> `0x8001490C` type=21 to `sub_8001490C`
- `0x80014908` -> `0x8001491C` type=19 to `loc_8001491C`
- `0x80014910` -> `0x80014914` type=21 to `sub_80014914`
- `0x80014910` -> `0x8001491C` type=19 to `loc_8001491C`
- `0x80014914` -> `0x80014918` type=21 to `sub_80014918`
- `0x80014918` -> `0x8001491C` type=21 to `loc_8001491C`
- `0x8001491C` -> `0x80014920` type=21 to `sub_80014920`
- `0x80014920` -> `0x80014924` type=21 to `sub_80014924`
- `0x80014924` -> `0x80014928` type=21 to `sub_80014928`
- `0x80014924` -> `0x80014998` type=19 to `loc_80014998`
- `0x80014928` -> `0x8001492C` type=21 to `sub_8001492C`
- `0x8001492C` -> `0x80014934` type=21 to `sub_80014934`
- `0x8001492C` -> `0x800901C0` type=3 to `dword_800901C0`
- `0x80014934` -> `0x80014938` type=21 to `sub_80014938`
- `0x80014938` -> `0x8001493C` type=21 to `sub_8001493C`
- `0x8001493C` -> `0x80014940` type=21 to `sub_80014940`
- `0x8001493C` -> `0x80014994` type=19 to `loc_80014994`
- `0x80014940` -> `0x80014944` type=21 to `sub_80014944`
- `0x80014944` -> `0x8001494C` type=21 to `sub_8001494C`
- `0x80014944` -> `0x8008EEFC` type=1 to `dword_8008EEFC`
- `0x8001494C` -> `0x80014950` type=21 to `sub_80014950`
- `0x80014950` -> `0x80014954` type=21 to `sub_80014954`
- `0x80014954` -> `0x8001495C` type=21 to `sub_8001495C`
- `0x80014954` -> `0x800901C0` type=3 to `dword_800901C0`
- `0x8001495C` -> `0x80014960` type=21 to `sub_80014960`
- `0x80014960` -> `0x80014964` type=21 to `sub_80014964`
- `0x80014964` -> `0x8001496C` type=21 to `sub_8001496C`
- `0x80014964` -> `0x8008EEF8` type=1 to `byte_8008EEF8`
- `0x8001496C` -> `0x80014970` type=21 to `sub_80014970`
- `0x80014970` -> `0x80014974` type=21 to `sub_80014974`
- `0x80014974` -> `0x8001497C` type=21 to `sub_8001497C`
- `0x80014974` -> `0x800901C0` type=3 to `dword_800901C0`
- `0x8001497C` -> `0x80014980` type=21 to `sub_80014980`
- `0x80014980` -> `0x80014984` type=21 to `sub_80014984`
- `0x80014984` -> `0x8001498C` type=21 to `sub_8001498C`
- `0x80014984` -> `0x800901C0` type=2 to `dword_800901C0`
- `0x8001498C` -> `0x80014994` type=21 to `loc_80014994`
- `0x8001498C` -> `0x800901BC` type=2 to `dword_800901BC`
- `0x80014994` -> `0x80014998` type=21 to `loc_80014998`
- `0x80014998` -> `0x8001499C` type=21 to `sub_8001499C`
- `0x80014998` -> `0x800149FC` type=19 to `loc_800149FC`
- `0x8001499C` -> `0x800149A0` type=21 to `sub_800149A0`
- `0x800149A0` -> `0x800149A4` type=21 to `sub_800149A4`
- `0x800149A4` -> `0x800149A8` type=21 to `sub_800149A8`
- `0x800149A8` -> `0x800149AC` type=21 to `sub_800149AC`
- `0x800149AC` -> `0x800149B0` type=21 to `sub_800149B0`
- `0x800149B0` -> `0x800149B4` type=21 to `sub_800149B4`
- `0x800149B4` -> `0x800149B8` type=21 to `sub_800149B8`
- `0x800149B4` -> `0x80024B54` type=17 to `sub_80024B54`
- `0x800149B8` -> `0x800149BC` type=21 to `sub_800149BC`
- `0x800149BC` -> `0x800149C0` type=21 to `sub_800149C0`
- `0x800149C0` -> `0x800149C4` type=21 to `sub_800149C4`
- `0x800149C4` -> `0x800149C8` type=21 to `sub_800149C8`
- `0x800149C8` -> `0x800149CC` type=21 to `sub_800149CC`
- `0x800149CC` -> `0x800149D0` type=21 to `sub_800149D0`
- `0x800149D0` -> `0x800149D4` type=21 to `sub_800149D4`
- `0x800149D4` -> `0x800149D8` type=21 to `sub_800149D8`
- `0x800149D8` -> `0x800149DC` type=21 to `sub_800149DC`
- `0x800149DC` -> `0x800149E4` type=21 to `sub_800149E4`
- `0x800149DC` -> `0x80091812` type=3 to `word_80091812`
- `0x800149E4` -> `0x800149E8` type=21 to `sub_800149E8`
- `0x800149E8` -> `0x800149EC` type=21 to `sub_800149EC`
- `0x800149EC` -> `0x800149F4` type=21 to `sub_800149F4`
- `0x800149EC` -> `0x80091812` type=2 to `word_80091812`
- `0x800149F4` -> `0x800149F8` type=21 to `sub_800149F8`
- `0x800149F4` -> `0x80014A14` type=19 to `loc_80014A14`
- `0x800149FC` -> `0x80014A04` type=21 to `sub_80014A04`
- `0x800149FC` -> `0x80091814` type=3 to `word_80091814`
- `0x80014A04` -> `0x80014A08` type=21 to `sub_80014A08`
- `0x80014A08` -> `0x80014A0C` type=21 to `sub_80014A0C`
- `0x80014A0C` -> `0x80014A14` type=21 to `loc_80014A14`
- `0x80014A0C` -> `0x80091814` type=2 to `word_80091814`
- `0x80014A14` -> `0x80014A1C` type=21 to `sub_80014A1C`
- `0x80014A14` -> `0x80091810` type=3 to `word_80091810`
- `0x80014A1C` -> `0x80014A24` type=21 to `sub_80014A24`
- `0x80014A1C` -> `0x80091808` type=3 to `dword_80091808`
- `0x80014A24` -> `0x80014A28` type=21 to `sub_80014A28`
- `0x80014A28` -> `0x80014A2C` type=21 to `sub_80014A2C`
- `0x80014A2C` -> `0x80014A34` type=21 to `sub_80014A34`
- `0x80014A2C` -> `0x80091810` type=2 to `word_80091810`
- `0x80014A34` -> `0x80014A38` type=21 to `sub_80014A38`
- `0x80014A38` -> `0x80014A40` type=21 to `sub_80014A40`
- `0x80014A38` -> `0x80091808` type=2 to `dword_80091808`
- `0x80014A40` -> `0x80014A44` type=21 to `sub_80014A44`
- `0x80014A40` -> `0x80014A5C` type=19 to `loc_80014A5C`
- `0x80014A44` -> `0x80014A48` type=21 to `sub_80014A48`
- `0x80014A48` -> `0x80014A4C` type=21 to `sub_80014A4C`
- `0x80014A4C` -> `0x80014A50` type=21 to `sub_80014A50`
- `0x80014A50` -> `0x80014A58` type=21 to `sub_80014A58`
- `0x80014A50` -> `0x80091824` type=2 to `word_80091824`
- `0x80014A58` -> `0x80014A5C` type=21 to `loc_80014A5C`
- `0x80014A5C` -> `0x80014A60` type=21 to `sub_80014A60`
- `0x80014A60` -> `0x80014A64` type=21 to `sub_80014A64`
- `0x80014A64` -> `0x80014A68` type=21 to `sub_80014A68`
- `0x80014A68` -> `0x80014A6C` type=21 to `sub_80014A6C`
- `0x80014A6C` -> `0x80014A70` type=21 to `sub_80014A70`
- `0x80014A70` -> `0x80014A74` type=21 to `sub_80014A74`
- `0x80014A74` -> `0x80014A78` type=21 to `sub_80014A78`
- `0x80014A78` -> `0x80014A7C` type=21 to `sub_80014A7C`

### Referenced 0x80000000-0x801FFFFF operands

- `0x80014630` op0 type=7 -> `0x80024BF4` `sub_80024BF4`
- `0x8001463C` op2 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x80014644` op1 type=2 -> `0x8008ED2E` `g_PrStageEventStreamFlag`
- `0x80014650` op1 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x80014660` op1 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x80014670` op2 type=7 -> `0x80014680` `loc_80014680`
- `0x80014678` op0 type=7 -> `0x80014690` `loc_80014690`
- `0x80014684` op2 type=7 -> `0x80014690` `loc_80014690`
- `0x800146B4` op1 type=7 -> `0x800146C4` `loc_800146C4`
- `0x800146BC` op0 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x800146CC` op1 type=2 -> `0x800943D8` `dword_800943D8`
- `0x800146EC` op1 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x800146FC` op1 type=7 -> `0x8001470C` `loc_8001470C`
- `0x80014704` op0 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x80014770` op1 type=2 -> `0x800943E0` `dword_800943E0`
- `0x8001479C` op1 type=7 -> `0x800147AC` `loc_800147AC`
- `0x800147A4` op0 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x800147AC` op1 type=2 -> `0x80091800` `dword_80091800`
- `0x800147BC` op2 type=7 -> `0x800147DC` `loc_800147DC`
- `0x800147C4` op1 type=7 -> `0x800147DC` `loc_800147DC`
- `0x800147D4` op0 type=7 -> `0x800147E0` `loc_800147E0`
- `0x800147E8` op1 type=2 -> `0x80091800` `dword_80091800`
- `0x800147F0` op1 type=7 -> `0x80014840` `loc_80014840`
- `0x80014808` op1 type=7 -> `0x80014840` `loc_80014840`
- `0x80014814` op1 type=7 -> `0x80014840` `loc_80014840`
- `0x80014824` op1 type=7 -> `0x80014834` `loc_80014834`
- `0x8001482C` op0 type=7 -> `0x80014838` `loc_80014838`
- `0x80014840` op1 type=7 -> `0x8001484C` `loc_8001484C`
- `0x80014870` op1 type=7 -> `0x800148BC` `loc_800148BC`
- `0x8001488C` op1 type=7 -> `0x80014898` `loc_80014898`
- `0x8001489C` op2 type=7 -> `0x800148B0` `loc_800148B0`
- `0x800148A4` op2 type=7 -> `0x800148B0` `loc_800148B0`
- `0x800148BC` op0 type=7 -> `0x80026FC4` `sub_80026FC4`
- `0x800148CC` op1 type=7 -> `0x800148EC` `loc_800148EC`
- `0x800148D4` op1 type=2 -> `0x80094430` `dword_80094430`
- `0x800148EC` op2 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x80014900` op2 type=7 -> `0x80014910` `loc_80014910`
- `0x80014908` op0 type=7 -> `0x8001491C` `loc_8001491C`
- `0x80014910` op2 type=7 -> `0x8001491C` `loc_8001491C`
- `0x80014924` op1 type=7 -> `0x80014998` `loc_80014998`
- `0x8001492C` op1 type=2 -> `0x800901C0` `dword_800901C0`
- `0x8001493C` op1 type=7 -> `0x80014994` `loc_80014994`
- `0x80014944` op1 type=5 -> `0x8008EEFC` `dword_8008EEFC`
- `0x80014954` op1 type=2 -> `0x800901C0` `dword_800901C0`
- `0x80014964` op1 type=5 -> `0x8008EEF8` `byte_8008EEF8`
- `0x80014974` op1 type=2 -> `0x800901C0` `dword_800901C0`
- `0x80014984` op1 type=2 -> `0x800901C0` `dword_800901C0`
- `0x8001498C` op1 type=2 -> `0x800901BC` `dword_800901BC`
- `0x80014998` op2 type=7 -> `0x800149FC` `loc_800149FC`
- `0x800149B4` op0 type=7 -> `0x80024B54` `sub_80024B54`
- `0x800149DC` op1 type=2 -> `0x80091812` `word_80091812`
- `0x800149EC` op1 type=2 -> `0x80091812` `word_80091812`
- `0x800149F4` op0 type=7 -> `0x80014A14` `loc_80014A14`
- `0x800149FC` op1 type=2 -> `0x80091814` `word_80091814`
- `0x80014A0C` op1 type=2 -> `0x80091814` `word_80091814`
- `0x80014A14` op1 type=2 -> `0x80091810` `word_80091810`
- `0x80014A1C` op1 type=2 -> `0x80091808` `dword_80091808`
- `0x80014A2C` op1 type=2 -> `0x80091810` `word_80091810`
- `0x80014A38` op1 type=2 -> `0x80091808` `dword_80091808`
- `0x80014A40` op2 type=7 -> `0x80014A5C` `loc_80014A5C`
- `0x80014A50` op1 type=2 -> `0x80091824` `word_80091824`

## 0x80014D58 `sub_80014D58` - accumulator/page-history commit

- Bounds: `0x80014D58` - `0x800152D0`
- Size: `1400` bytes

### Hex-Rays pseudo-C

```c
int __fastcall sub_80014D58(int a1)
{
  int v2; // $a0
  int v3; // $v0
  int v4; // $t4
  int v5; // $a2
  int v6; // $s6
  int v7; // $s7
  int v8; // $a1
  int v9; // $t1
  int v10; // $t5
  int v11; // $v0
  int v12; // $a2
  int v13; // $a0
  int v14; // $t0
  BOOL v15; // $s0
  int *v16; // $t3
  char **v17; // $t2
  char *v18; // $a0
  int v19; // $a3
  int i; // $a1
  int v21; // $s1
  int v22; // $s3
  int v23; // $s2
  int *v24; // $s0
  int *v25; // $s5
  int v26; // $a2
  _BYTE *v27; // $v0
  int v28; // $a0
  int v29; // $v0
  int v30; // $t2
  int v31; // $a1
  int v32; // $s5
  int v33; // $a3
  int v34; // $t3
  int *v35; // $t1
  char **v36; // $t0
  int v37; // $a1
  int v38; // $a0
  char *v39; // $v1
  int v40; // $s4
  int v41; // $s0
  int *v42; // $s1
  int v43; // $s2
  int v44; // $a2
  _BYTE *v45; // $v0
  int v46; // $v1
  int v47; // $a0
  int v48; // $v0
  int result; // $v0
  _BYTE v50[16]; // [sp+0h] [-40h] BYREF
  char *v51; // [sp+10h] [-30h] BYREF
  char *v52; // [sp+14h] [-2Ch]

  if ( !word_80091810 )
  {
    v2 = -1;
    goto LABEL_71;
  }
  if ( *(_WORD *)(a1 + 80) )
  {
    v30 = *(__int16 *)(*(_DWORD *)(a1 + 64) + 6 * *(__int16 *)(a1 + 80) + 14);
    v31 = *(_DWORD *)(a1 + 56);
    v32 = 0;
    if ( v30 == 1 )
    {
      v51 = (char *)&unk_80092910 + 384 * (((_BYTE)v31 - 1) & 3);
    }
    else
    {
      v33 = 0;
      if ( v30 != 2 )
        goto LABEL_48;
      v51 = (char *)&unk_80092910 + 384 * (((_BYTE)v31 - 2) & 3);
      v52 = (char *)&unk_80092910 + 384 * (((_BYTE)v31 - 1) & 3);
    }
    v33 = 0;
LABEL_48:
    v34 = 0;
    if ( v30 > 0 )
    {
      v35 = &dword_80048CA8[4 * v30];
      v36 = &v51;
      do
      {
        v37 = *v35;
        if ( word_800916E2 == 1 && *v35 == 15 )
          v37 = 14;
        v38 = 0;
        if ( v37 > 0 )
        {
          v39 = *v36;
          do
          {
            if ( *((_WORD *)v39 + 3) )
              ++v33;
            ++v38;
            v39 += 12;
          }
          while ( v38 < v37 );
        }
        ++v35;
        ++v34;
        ++v36;
      }
      while ( v34 < v30 );
    }
    v40 = v33;
    if ( 12 * v30 >= word_80091810 )
    {
      v41 = 0;
      if ( v30 > 0 )
      {
        v42 = &dword_80048CA8[4 * v30];
        v43 = 4 * v30;
        do
        {
          v44 = *v42;
          v45 = &v50[v41];
          if ( word_800916E2 == 1 )
          {
            v45 = &v50[v41];
            if ( v44 == 15 )
              v44 = 14;
          }
          v41 += 4;
          ++v42;
          v32 += sub_80014A80(*((_DWORD *)v45 + 4), 0, v44, v33, v51, v52);
        }
        while ( v41 < v43 );
        v46 = 2 * v40;
        goto LABEL_70;
      }
    }
    else
    {
      v46 = 2 * v33;
      if ( 12 * v30 >= word_80091810 )
      {
LABEL_70:
        v2 = v46 + v40 - 2 * word_80091814 + v32;
        goto LABEL_71;
      }
      v32 = 0;
    }
    v46 = 2 * v33;
    goto LABEL_70;
  }
  v3 = *(_DWORD *)(a1 + 64);
  v4 = *(__int16 *)(v3 + 14);
  v5 = *(unsigned __int8 *)(v3 + 1);
  v6 = 0;
  if ( v4 <= 0 )
  {
    v2 = 0;
    goto LABEL_71;
  }
  v7 = 12 * v4;
  v8 = *(_DWORD *)(a1 + 56);
  if ( v4 == 1 )
  {
    v51 = (char *)&unk_80092910 + 384 * (((_BYTE)v8 - 1) & 3);
LABEL_10:
    v9 = 0;
    goto LABEL_11;
  }
  v9 = 0;
  if ( v4 == 2 )
  {
    v51 = (char *)&unk_80092910 + 384 * (((_BYTE)v8 - 2) & 3);
    v52 = (char *)&unk_80092910 + 384 * (((_BYTE)v8 - 1) & 3);
    goto LABEL_10;
  }
LABEL_11:
  v10 = 0;
  v11 = 12 * v5;
  v12 = 0;
  v13 = *(_DWORD *)(a1 + 64);
  v14 = *(_DWORD *)(v13 + 8);
  v15 = *(__int16 *)&v51[v11 + 4] == *(unsigned __int8 *)(v13 + 2);
  v16 = &dword_80048CA8[4 * v4];
  v17 = &v51;
  do
  {
    v18 = *v17;
    v19 = *v16;
    if ( word_800916E2 == 1 && *v16 == 15 )
      v19 = 14;
    for ( i = 0; i < v19; v18 += 12 )
    {
      v9 |= *(_DWORD *)v18;
      if ( *((_WORD *)v18 + 3) && (!v14 || (v14 & *(_DWORD *)v18) != 0) )
        ++v12;
      ++i;
    }
    ++v16;
    ++v10;
    ++v17;
  }
  while ( v10 < v4 );
  if ( v14 )
    v21 = (v9 & v14) == v14 ? v12 : 0;
  else
    v21 = v12;
  if ( v15 && 12 * v4 >= word_80091810 )
  {
    v22 = 3 * v21;
    v23 = 0;
    v24 = &dword_80048CA8[4 * v4];
    v25 = &v24[v4];
    do
    {
      if ( v21 > 0 )
      {
        v26 = *v24;
        v27 = &v50[v23];
        if ( word_800916E2 == 1 )
        {
          v27 = &v50[v23];
          if ( v26 == 15 )
            v26 = 14;
        }
        v6 += sub_80014A80(*((_DWORD *)v27 + 4), 0, v26, v19, v51, v52);
      }
      ++v24;
      v23 += 4;
    }
    while ( (int)v24 < (int)v25 );
  }
  else
  {
    v22 = 3 * v21 - *(unsigned __int8 *)(*(_DWORD *)(a1 + 64) + 3);
  }
  v28 = -(*(unsigned __int8 *)(*(_DWORD *)(a1 + 64) + 3) * word_80091814);
  if ( v28 < ~(3 * *(unsigned __int8 *)(*(_DWORD *)(a1 + 64) + 3)) )
    v28 = ~(3 * *(unsigned __int8 *)(*(_DWORD *)(a1 + 64) + 3));
  v29 = v22 + v6;
  if ( v7 < word_80091810 )
  {
    v28 = 0;
    v29 = 0;
  }
  v2 = v29 + v28;
LABEL_71:
  v47 = v2 + word_80091822;
  v48 = (unsigned __int16)word_80091816 + v47;
  word_80091816 += v47;
  result = v48 << 16;
  if ( result < 0 )
    word_80091816 = 0;
  return result;
}

```

### Disassembly

```asm
0x80014D58: lh      $v0, word_80091810
0x80014D60: addiu   $sp, -0x68
0x80014D64: sw      $s4, 0x40+var_s10($sp)
0x80014D68: move    $s4, $a0
0x80014D6C: sw      $ra, 0x40+var_s20($sp)
0x80014D70: sw      $s7, 0x40+var_s1C($sp)
0x80014D74: sw      $s6, 0x40+var_s18($sp)
0x80014D78: sw      $s5, 0x40+var_s14($sp)
0x80014D7C: sw      $s3, 0x40+var_sC($sp)
0x80014D80: sw      $s2, 0x40+var_s8($sp)
0x80014D84: sw      $s1, 0x40+var_s4($sp)
0x80014D88: bnez    $v0, loc_80014D98
0x80014D8C: sw      $s0, 0x40+var_s0($sp)
0x80014D90: j       loc_8001526C
0x80014D94: li      $a0, 0xFFFFFFFF
0x80014D98: lh      $v1, 0x50($s4)
0x80014D9C: nop
0x80014DA0: bnez    $v1, loc_80015068
0x80014DA4: sll     $v0, $v1, 1
0x80014DA8: lw      $v0, 0x40($s4)
0x80014DAC: nop
0x80014DB0: lh      $t4, 0xE($v0)
0x80014DB4: lbu     $a2, 1($v0)
0x80014DB8: bgtz    $t4, loc_80014DC8
0x80014DBC: move    $s6, $zero
0x80014DC0: j       loc_8001526C
0x80014DC4: move    $a0, $zero
0x80014DC8: sll     $v0, $t4, 1
0x80014DCC: addu    $v0, $t4
0x80014DD0: sll     $s7, $v0, 2
0x80014DD4: lh      $t7, word_80091810
0x80014DDC: lw      $a1, 0x38($s4)
0x80014DE0: slt     $v0, $s7, $t7
0x80014DE4: xori    $t7, $v0, 1
0x80014DE8: li      $v0, 1
0x80014DEC: bne     $t4, $v0, loc_80014E1C
0x80014DF0: li      $v0, 2
0x80014DF4: addiu   $v0, $a1, -1
0x80014DF8: andi    $v0, 3
0x80014DFC: sll     $v1, $v0, 1
0x80014E00: addu    $v1, $v0
0x80014E04: sll     $v1, 7
0x80014E08: li      $v0, unk_80092910
0x80014E10: addu    $v1, $v0
0x80014E14: j       loc_80014E64
0x80014E18: sw      $v1, 0x40+var_30($sp)
0x80014E1C: bne     $t4, $v0, loc_80014E68
0x80014E20: move    $t1, $zero
0x80014E24: addiu   $v1, $a1, -2
0x80014E28: andi    $v1, 3
0x80014E2C: sll     $v0, $v1, 1
0x80014E30: addu    $v0, $v1
0x80014E34: sll     $v0, 7
0x80014E38: li      $a0, unk_80092910
0x80014E40: addu    $v0, $a0
0x80014E44: addiu   $v1, $a1, -1
0x80014E48: andi    $v1, 3
0x80014E4C: sw      $v0, 0x40+var_30($sp)
0x80014E50: sll     $v0, $v1, 1
0x80014E54: addu    $v0, $v1
0x80014E58: sll     $v0, 7
0x80014E5C: addu    $v0, $a0
0x80014E60: sw      $v0, 0x40+var_2C($sp)
0x80014E64: move    $t1, $zero
0x80014E68: move    $t5, $zero
0x80014E6C: sll     $v0, $a2, 1
0x80014E70: addu    $v0, $a2
0x80014E74: sll     $v0, 2
0x80014E78: move    $a2, $zero
0x80014E7C: addiu   $a1, $sp, 0x40+var_30
0x80014E80: lw      $v1, 0x40+var_30($sp)
0x80014E84: lw      $a0, 0x40($s4)
0x80014E88: addu    $v0, $v1
0x80014E8C: lh      $v0, 4($v0)
0x80014E90: lbu     $v1, 2($a0)
0x80014E94: lw      $t0, 8($a0)
0x80014E98: xor     $v0, $v1
0x80014E9C: blez    $t4, loc_80014F38
0x80014EA0: sltiu   $s0, $v0, 1
0x80014EA4: lh      $t6, word_800916E2
0x80014EAC: li      $v1, dword_80048CA8
0x80014EB4: sll     $v0, $t4, 4
0x80014EB8: addu    $t3, $v0, $v1
0x80014EBC: move    $t2, $a1
0x80014EC0: li      $v0, 1
0x80014EC4: lw      $v1, 0($t3)
0x80014EC8: lw      $a0, 0($t2)
0x80014ECC: bne     $t6, $v0, loc_80014EE4
0x80014ED0: move    $a3, $v1
0x80014ED4: li      $v0, 0xF
0x80014ED8: bne     $v1, $v0, loc_80014EE4
0x80014EDC: li      $v1, 0xE
0x80014EE0: move    $a3, $v1
0x80014EE4: blez    $a3, loc_80014F24
0x80014EE8: move    $a1, $zero
0x80014EEC: lw      $v1, 0($a0)
0x80014EF0: lh      $v0, 6($a0)
0x80014EF4: nop
0x80014EF8: beqz    $v0, loc_80014F14
0x80014EFC: or      $t1, $v1
0x80014F00: beqz    $t0, loc_80014F10
0x80014F04: and     $v0, $t0, $v1
0x80014F08: beqz    $v0, loc_80014F14
0x80014F0C: nop
0x80014F10: addiu   $a2, 1
0x80014F14: addiu   $a1, 1
0x80014F18: slt     $v0, $a1, $a3
0x80014F1C: bnez    $v0, loc_80014EEC
0x80014F20: addiu   $a0, 0xC
0x80014F24: addiu   $t3, 4
0x80014F28: addiu   $t5, 1
0x80014F2C: slt     $v0, $t5, $t4
0x80014F30: bnez    $v0, loc_80014EC0
0x80014F34: addiu   $t2, 4
0x80014F38: bnez    $t0, loc_80014F48
0x80014F3C: and     $v0, $t1, $t0
0x80014F40: j       loc_80014F58
0x80014F44: move    $s1, $a2
0x80014F48: xor     $v0, $t0
0x80014F4C: sltiu   $v0, 1
0x80014F50: negu    $v0, $v0
0x80014F54: and     $s1, $a2, $v0
0x80014F58: li      $v0, 1
0x80014F5C: bne     $s0, $v0, loc_80014FE4
0x80014F60: nop
0x80014F64: bne     $t7, $s0, loc_80014FE4
0x80014F68: sll     $v0, $s1, 1
0x80014F6C: blez    $t4, loc_80014FF8
0x80014F70: addu    $s3, $v0, $s1
0x80014F74: move    $s2, $zero
0x80014F78: li      $v0, dword_80048CA8
0x80014F80: sll     $v1, $t4, 4
0x80014F84: addu    $s0, $v1, $v0
0x80014F88: sll     $v0, $t4, 2
0x80014F8C: addu    $s5, $v0, $s0
0x80014F90: blez    $s1, loc_80014FCC
0x80014F94: li      $v0, 1
0x80014F98: lh      $v1, word_800916E2
0x80014FA0: lw      $a2, 0($s0)
0x80014FA4: bne     $v1, $v0, loc_80014FBC
0x80014FA8: addu    $v0, $sp, $s2
0x80014FAC: li      $v0, 0xF
0x80014FB0: bne     $a2, $v0, loc_80014FBC
0x80014FB4: addu    $v0, $sp, $s2
0x80014FB8: li      $a2, 0xE
0x80014FBC: lw      $a0, 0x10($v0)
0x80014FC0: jal     sub_80014A80
0x80014FC4: move    $a1, $zero
0x80014FC8: addu    $s6, $v0
0x80014FCC: addiu   $s0, 4
0x80014FD0: slt     $v0, $s0, $s5
0x80014FD4: bnez    $v0, loc_80014F90
0x80014FD8: addiu   $s2, 4
0x80014FDC: j       loc_80014FF8
0x80014FE0: nop
0x80014FE4: lw      $v0, 0x40($s4)
0x80014FE8: sll     $v1, $s1, 1
0x80014FEC: lbu     $v0, 3($v0)
0x80014FF0: addu    $v1, $s1
0x80014FF4: subu    $s3, $v1, $v0
0x80014FF8: lw      $v0, 0x40($s4)
0x80014FFC: nop
0x80015000: lbu     $v1, 3($v0)
0x80015004: lh      $v0, word_80091814
0x8001500C: nop
0x80015010: mult    $v1, $v0
0x80015014: sll     $v0, $v1, 1
0x80015018: addu    $v0, $v1
0x8001501C: nor     $v1, $zero, $v0
0x80015020: mflo    $t8
0x80015024: negu    $a0, $t8
0x80015028: slt     $v0, $a0, $v1
0x8001502C: beqz    $v0, loc_80015038
0x80015030: nop
0x80015034: move    $a0, $v1
0x80015038: lh      $v0, word_80091810
0x80015040: nop
0x80015044: slt     $v0, $s7, $v0
0x80015048: beqz    $v0, loc_80015060
0x8001504C: addu    $v0, $s3, $s6
0x80015050: move    $a0, $zero
0x80015054: move    $s6, $zero
0x80015058: move    $s3, $zero
0x8001505C: addu    $v0, $s3, $s6
0x80015060: j       loc_8001526C
0x80015064: addu    $a0, $v0, $a0
0x80015068: addu    $v0, $v1
0x8001506C: sll     $v0, 1
0x80015070: lw      $v1, 0x40($s4)
0x80015074: lh      $t4, word_80091810
0x8001507C: addu    $v1, $v0
0x80015080: lh      $t2, 0xE($v1)
0x80015084: lw      $a1, 0x38($s4)
0x80015088: sll     $v0, $t2, 1
0x8001508C: addu    $v0, $t2
0x80015090: sll     $t6, $v0, 2
0x80015094: slt     $v0, $t6, $t4
0x80015098: xori    $t4, $v0, 1
0x8001509C: li      $v0, 1
0x800150A0: bne     $t2, $v0, loc_800150D0
0x800150A4: move    $s5, $zero
0x800150A8: addiu   $v0, $a1, -1
0x800150AC: andi    $v0, 3
0x800150B0: sll     $v1, $v0, 1
0x800150B4: addu    $v1, $v0
0x800150B8: sll     $v1, 7
0x800150BC: li      $v0, unk_80092910
0x800150C4: addu    $v1, $v0
0x800150C8: j       loc_8001511C
0x800150CC: sw      $v1, 0x40+var_30($sp)
0x800150D0: li      $v0, 2
0x800150D4: bne     $t2, $v0, loc_80015120
0x800150D8: move    $a3, $zero
0x800150DC: addiu   $v1, $a1, -2
0x800150E0: andi    $v1, 3
0x800150E4: sll     $v0, $v1, 1
0x800150E8: addu    $v0, $v1
0x800150EC: sll     $v0, 7
0x800150F0: li      $a0, unk_80092910
0x800150F8: addu    $v0, $a0
0x800150FC: addiu   $v1, $a1, -1
0x80015100: andi    $v1, 3
0x80015104: sw      $v0, 0x40+var_30($sp)
0x80015108: sll     $v0, $v1, 1
0x8001510C: addu    $v0, $v1
0x80015110: sll     $v0, 7
0x80015114: addu    $v0, $a0
0x80015118: sw      $v0, 0x40+var_2C($sp)
0x8001511C: move    $a3, $zero
0x80015120: move    $t3, $zero
0x80015124: addiu   $a0, $sp, 0x40+var_30
0x80015128: blez    $t2, loc_800151B8
0x8001512C: move    $s3, $zero
0x80015130: lh      $t5, word_800916E2
0x80015138: li      $v1, dword_80048CA8
0x80015140: sll     $v0, $t2, 4
0x80015144: addu    $t1, $v0, $v1
0x80015148: move    $t0, $a0
0x8001514C: li      $v0, 1
0x80015150: lw      $v1, 0($t1)
0x80015154: lw      $a2, 0($t0)
0x80015158: bne     $t5, $v0, loc_80015174
0x8001515C: move    $a1, $v1
0x80015160: li      $v0, 0xF
0x80015164: bne     $v1, $v0, loc_80015174
0x80015168: nop
0x8001516C: li      $v1, 0xE
0x80015170: move    $a1, $v1
0x80015174: blez    $a1, loc_800151A4
0x80015178: move    $a0, $zero
0x8001517C: move    $v1, $a2
0x80015180: lh      $v0, 6($v1)
0x80015184: nop
0x80015188: beqz    $v0, loc_80015194
0x8001518C: nop
0x80015190: addiu   $a3, 1
0x80015194: addiu   $a0, 1
0x80015198: slt     $v0, $a0, $a1
0x8001519C: bnez    $v0, loc_80015180
0x800151A0: addiu   $v1, 0xC
0x800151A4: addiu   $t1, 4
0x800151A8: addiu   $t3, 1
0x800151AC: slt     $v0, $t3, $t2
0x800151B0: bnez    $v0, loc_8001514C
0x800151B4: addiu   $t0, 4
0x800151B8: li      $v0, 1
0x800151BC: bne     $t4, $v0, loc_80015234
0x800151C0: move    $s4, $a3
0x800151C4: blez    $t2, loc_80015250
0x800151C8: move    $s0, $zero
0x800151CC: li      $v1, dword_80048CA8
0x800151D4: sll     $v0, $t2, 4
0x800151D8: addu    $s1, $v0, $v1
0x800151DC: sll     $s2, $t2, 2
0x800151E0: li      $v0, 1
0x800151E4: lh      $v1, word_800916E2
0x800151EC: lw      $a2, 0($s1)
0x800151F0: bne     $v1, $v0, loc_80015208
0x800151F4: addu    $v0, $sp, $s0
0x800151F8: li      $v0, 0xF
0x800151FC: bne     $a2, $v0, loc_80015208
0x80015200: addu    $v0, $sp, $s0
0x80015204: li      $a2, 0xE
0x80015208: lw      $a0, 0x10($v0)
0x8001520C: move    $a1, $s3
0x80015210: addiu   $s0, 4
0x80015214: jal     sub_80014A80
0x80015218: addiu   $s1, 4
0x8001521C: addu    $s5, $v0
0x80015220: slt     $v0, $s0, $s2
0x80015224: bnez    $v0, loc_800151E4
0x80015228: li      $v0, 1
0x8001522C: j       loc_80015254
0x80015230: sll     $v1, $s4, 1
0x80015234: lh      $v0, word_80091810
0x8001523C: nop
0x80015240: slt     $v0, $t6, $v0
0x80015244: beqz    $v0, loc_80015254
0x80015248: sll     $v1, $s4, 1
0x8001524C: move    $s5, $zero
0x80015250: sll     $v1, $s4, 1
0x80015254: lh      $v0, word_80091814
0x8001525C: addu    $v1, $s4
0x80015260: sll     $v0, 1
0x80015264: subu    $v1, $v0
0x80015268: addu    $a0, $v1, $s5
0x8001526C: lh      $v1, word_80091822
0x80015274: lhu     $v0, word_80091816
0x8001527C: addu    $a0, $v1
0x80015280: addu    $v0, $a0
0x80015284: sh      $v0, word_80091816
0x8001528C: sll     $v0, 16
0x80015290: bgez    $v0, loc_800152A0
0x80015294: nop
0x80015298: sh      $zero, word_80091816
0x800152A0: lw      $ra, 0x40+var_s20($sp)
0x800152A4: lw      $s7, 0x40+var_s1C($sp)
0x800152A8: lw      $s6, 0x40+var_s18($sp)
0x800152AC: lw      $s5, 0x40+var_s14($sp)
0x800152B0: lw      $s4, 0x40+var_s10($sp)
0x800152B4: lw      $s3, 0x40+var_sC($sp)
0x800152B8: lw      $s2, 0x40+var_s8($sp)
0x800152BC: lw      $s1, 0x40+var_s4($sp)
0x800152C0: lw      $s0, 0x40+var_s0($sp)
0x800152C4: addiu   $sp, 0x68
0x800152C8: jr      $ra
0x800152CC: nop
```

### Xrefs to

- `0x80025320` -> `0x80014D58` type=17 from `sub_80025320`

### Xrefs from

- `0x80014D58` -> `0x80014D60` type=21 to `sub_80014D60`
- `0x80014D58` -> `0x80091810` type=3 to `word_80091810`
- `0x80014D60` -> `0x80014D64` type=21 to `sub_80014D64`
- `0x80014D64` -> `0x80014D68` type=21 to `sub_80014D68`
- `0x80014D68` -> `0x80014D6C` type=21 to `sub_80014D6C`
- `0x80014D6C` -> `0x80014D70` type=21 to `sub_80014D70`
- `0x80014D70` -> `0x80014D74` type=21 to `sub_80014D74`
- `0x80014D74` -> `0x80014D78` type=21 to `sub_80014D78`
- `0x80014D78` -> `0x80014D7C` type=21 to `sub_80014D7C`
- `0x80014D7C` -> `0x80014D80` type=21 to `sub_80014D80`
- `0x80014D80` -> `0x80014D84` type=21 to `sub_80014D84`
- `0x80014D84` -> `0x80014D88` type=21 to `sub_80014D88`
- `0x80014D88` -> `0x80014D8C` type=21 to `sub_80014D8C`
- `0x80014D88` -> `0x80014D98` type=19 to `loc_80014D98`
- `0x80014D8C` -> `0x80014D90` type=21 to `sub_80014D90`
- `0x80014D90` -> `0x80014D94` type=21 to `sub_80014D94`
- `0x80014D90` -> `0x8001526C` type=19 to `loc_8001526C`
- `0x80014D98` -> `0x80014D9C` type=21 to `sub_80014D9C`
- `0x80014D9C` -> `0x80014DA0` type=21 to `sub_80014DA0`
- `0x80014DA0` -> `0x80014DA4` type=21 to `sub_80014DA4`
- `0x80014DA0` -> `0x80015068` type=19 to `loc_80015068`
- `0x80014DA4` -> `0x80014DA8` type=21 to `sub_80014DA8`
- `0x80014DA8` -> `0x80014DAC` type=21 to `sub_80014DAC`
- `0x80014DAC` -> `0x80014DB0` type=21 to `sub_80014DB0`
- `0x80014DB0` -> `0x80014DB4` type=21 to `sub_80014DB4`
- `0x80014DB4` -> `0x80014DB8` type=21 to `sub_80014DB8`
- `0x80014DB8` -> `0x80014DBC` type=21 to `sub_80014DBC`
- `0x80014DB8` -> `0x80014DC8` type=19 to `loc_80014DC8`
- `0x80014DBC` -> `0x80014DC0` type=21 to `sub_80014DC0`
- `0x80014DC0` -> `0x80014DC4` type=21 to `sub_80014DC4`
- `0x80014DC0` -> `0x8001526C` type=19 to `loc_8001526C`
- `0x80014DC8` -> `0x80014DCC` type=21 to `sub_80014DCC`
- `0x80014DCC` -> `0x80014DD0` type=21 to `sub_80014DD0`
- `0x80014DD0` -> `0x80014DD4` type=21 to `sub_80014DD4`
- `0x80014DD4` -> `0x80014DDC` type=21 to `sub_80014DDC`
- `0x80014DD4` -> `0x80091810` type=3 to `word_80091810`
- `0x80014DDC` -> `0x80014DE0` type=21 to `sub_80014DE0`
- `0x80014DE0` -> `0x80014DE4` type=21 to `sub_80014DE4`
- `0x80014DE4` -> `0x80014DE8` type=21 to `sub_80014DE8`
- `0x80014DE8` -> `0x80014DEC` type=21 to `sub_80014DEC`
- `0x80014DEC` -> `0x80014DF0` type=21 to `sub_80014DF0`
- `0x80014DEC` -> `0x80014E1C` type=19 to `loc_80014E1C`
- `0x80014DF0` -> `0x80014DF4` type=21 to `sub_80014DF4`
- `0x80014DF4` -> `0x80014DF8` type=21 to `sub_80014DF8`
- `0x80014DF8` -> `0x80014DFC` type=21 to `sub_80014DFC`
- `0x80014DFC` -> `0x80014E00` type=21 to `sub_80014E00`
- `0x80014E00` -> `0x80014E04` type=21 to `sub_80014E04`
- `0x80014E04` -> `0x80014E08` type=21 to `sub_80014E08`
- `0x80014E08` -> `0x80014E10` type=21 to `sub_80014E10`
- `0x80014E08` -> `0x80092910` type=1 to `unk_80092910`
- `0x80014E10` -> `0x80014E14` type=21 to `sub_80014E14`
- `0x80014E14` -> `0x80014E18` type=21 to `sub_80014E18`
- `0x80014E14` -> `0x80014E64` type=19 to `loc_80014E64`
- `0x80014E1C` -> `0x80014E20` type=21 to `sub_80014E20`
- `0x80014E1C` -> `0x80014E68` type=19 to `loc_80014E68`
- `0x80014E20` -> `0x80014E24` type=21 to `sub_80014E24`
- `0x80014E24` -> `0x80014E28` type=21 to `sub_80014E28`
- `0x80014E28` -> `0x80014E2C` type=21 to `sub_80014E2C`
- `0x80014E2C` -> `0x80014E30` type=21 to `sub_80014E30`
- `0x80014E30` -> `0x80014E34` type=21 to `sub_80014E34`
- `0x80014E34` -> `0x80014E38` type=21 to `sub_80014E38`
- `0x80014E38` -> `0x80014E40` type=21 to `sub_80014E40`
- `0x80014E38` -> `0x80092910` type=1 to `unk_80092910`
- `0x80014E40` -> `0x80014E44` type=21 to `sub_80014E44`
- `0x80014E44` -> `0x80014E48` type=21 to `sub_80014E48`
- `0x80014E48` -> `0x80014E4C` type=21 to `sub_80014E4C`
- `0x80014E4C` -> `0x80014E50` type=21 to `sub_80014E50`
- `0x80014E50` -> `0x80014E54` type=21 to `sub_80014E54`
- `0x80014E54` -> `0x80014E58` type=21 to `sub_80014E58`
- `0x80014E58` -> `0x80014E5C` type=21 to `sub_80014E5C`
- `0x80014E5C` -> `0x80014E60` type=21 to `sub_80014E60`
- `0x80014E60` -> `0x80014E64` type=21 to `loc_80014E64`
- `0x80014E64` -> `0x80014E68` type=21 to `loc_80014E68`
- `0x80014E68` -> `0x80014E6C` type=21 to `sub_80014E6C`
- `0x80014E6C` -> `0x80014E70` type=21 to `sub_80014E70`
- `0x80014E70` -> `0x80014E74` type=21 to `sub_80014E74`
- `0x80014E74` -> `0x80014E78` type=21 to `sub_80014E78`
- `0x80014E78` -> `0x80014E7C` type=21 to `sub_80014E7C`
- `0x80014E7C` -> `0x80014E80` type=21 to `sub_80014E80`
- `0x80014E80` -> `0x80014E84` type=21 to `sub_80014E84`
- `0x80014E84` -> `0x80014E88` type=21 to `sub_80014E88`
- `0x80014E88` -> `0x80014E8C` type=21 to `sub_80014E8C`
- `0x80014E8C` -> `0x80014E90` type=21 to `sub_80014E90`
- `0x80014E90` -> `0x80014E94` type=21 to `sub_80014E94`
- `0x80014E94` -> `0x80014E98` type=21 to `sub_80014E98`
- `0x80014E98` -> `0x80014E9C` type=21 to `sub_80014E9C`
- `0x80014E9C` -> `0x80014EA0` type=21 to `sub_80014EA0`
- `0x80014E9C` -> `0x80014F38` type=19 to `loc_80014F38`
- `0x80014EA0` -> `0x80014EA4` type=21 to `sub_80014EA4`
- `0x80014EA4` -> `0x80014EAC` type=21 to `sub_80014EAC`
- `0x80014EA4` -> `0x800916E2` type=3 to `word_800916E2`
- `0x80014EAC` -> `0x80014EB4` type=21 to `sub_80014EB4`
- `0x80014EAC` -> `0x80048CA8` type=1 to `dword_80048CA8`
- `0x80014EB4` -> `0x80014EB8` type=21 to `sub_80014EB8`
- `0x80014EB8` -> `0x80014EBC` type=21 to `sub_80014EBC`
- `0x80014EBC` -> `0x80014EC0` type=21 to `loc_80014EC0`
- `0x80014EC0` -> `0x80014EC4` type=21 to `sub_80014EC4`
- `0x80014EC4` -> `0x80014EC8` type=21 to `sub_80014EC8`
- `0x80014EC8` -> `0x80014ECC` type=21 to `sub_80014ECC`
- `0x80014ECC` -> `0x80014ED0` type=21 to `sub_80014ED0`
- `0x80014ECC` -> `0x80014EE4` type=19 to `loc_80014EE4`
- `0x80014ED0` -> `0x80014ED4` type=21 to `sub_80014ED4`
- `0x80014ED4` -> `0x80014ED8` type=21 to `sub_80014ED8`
- `0x80014ED8` -> `0x80014EDC` type=21 to `sub_80014EDC`
- `0x80014ED8` -> `0x80014EE4` type=19 to `loc_80014EE4`
- `0x80014EDC` -> `0x80014EE0` type=21 to `sub_80014EE0`
- `0x80014EE0` -> `0x80014EE4` type=21 to `loc_80014EE4`
- `0x80014EE4` -> `0x80014EE8` type=21 to `sub_80014EE8`
- `0x80014EE4` -> `0x80014F24` type=19 to `loc_80014F24`
- `0x80014EE8` -> `0x80014EEC` type=21 to `loc_80014EEC`
- `0x80014EEC` -> `0x80014EF0` type=21 to `sub_80014EF0`
- `0x80014EF0` -> `0x80014EF4` type=21 to `sub_80014EF4`
- `0x80014EF4` -> `0x80014EF8` type=21 to `sub_80014EF8`
- `0x80014EF8` -> `0x80014EFC` type=21 to `sub_80014EFC`
- `0x80014EF8` -> `0x80014F14` type=19 to `loc_80014F14`
- `0x80014EFC` -> `0x80014F00` type=21 to `sub_80014F00`
- `0x80014F00` -> `0x80014F04` type=21 to `sub_80014F04`
- `0x80014F00` -> `0x80014F10` type=19 to `loc_80014F10`
- `0x80014F04` -> `0x80014F08` type=21 to `sub_80014F08`
- `0x80014F08` -> `0x80014F0C` type=21 to `sub_80014F0C`
- `0x80014F08` -> `0x80014F14` type=19 to `loc_80014F14`
- `0x80014F0C` -> `0x80014F10` type=21 to `loc_80014F10`
- `0x80014F10` -> `0x80014F14` type=21 to `loc_80014F14`
- `0x80014F14` -> `0x80014F18` type=21 to `sub_80014F18`
- `0x80014F18` -> `0x80014F1C` type=21 to `sub_80014F1C`
- `0x80014F1C` -> `0x80014EEC` type=19 to `loc_80014EEC`
- `0x80014F1C` -> `0x80014F20` type=21 to `sub_80014F20`
- `0x80014F20` -> `0x80014F24` type=21 to `loc_80014F24`
- `0x80014F24` -> `0x80014F28` type=21 to `sub_80014F28`
- `0x80014F28` -> `0x80014F2C` type=21 to `sub_80014F2C`
- `0x80014F2C` -> `0x80014F30` type=21 to `sub_80014F30`
- `0x80014F30` -> `0x80014EC0` type=19 to `loc_80014EC0`
- `0x80014F30` -> `0x80014F34` type=21 to `sub_80014F34`
- `0x80014F34` -> `0x80014F38` type=21 to `loc_80014F38`
- `0x80014F38` -> `0x80014F3C` type=21 to `sub_80014F3C`
- `0x80014F38` -> `0x80014F48` type=19 to `loc_80014F48`
- `0x80014F3C` -> `0x80014F40` type=21 to `sub_80014F40`
- `0x80014F40` -> `0x80014F44` type=21 to `sub_80014F44`
- `0x80014F40` -> `0x80014F58` type=19 to `loc_80014F58`
- `0x80014F48` -> `0x80014F4C` type=21 to `sub_80014F4C`
- `0x80014F4C` -> `0x80014F50` type=21 to `sub_80014F50`
- `0x80014F50` -> `0x80014F54` type=21 to `sub_80014F54`
- `0x80014F54` -> `0x80014F58` type=21 to `loc_80014F58`
- `0x80014F58` -> `0x80014F5C` type=21 to `sub_80014F5C`
- `0x80014F5C` -> `0x80014F60` type=21 to `sub_80014F60`
- `0x80014F5C` -> `0x80014FE4` type=19 to `loc_80014FE4`
- `0x80014F60` -> `0x80014F64` type=21 to `sub_80014F64`
- `0x80014F64` -> `0x80014F68` type=21 to `sub_80014F68`
- `0x80014F64` -> `0x80014FE4` type=19 to `loc_80014FE4`
- `0x80014F68` -> `0x80014F6C` type=21 to `sub_80014F6C`
- `0x80014F6C` -> `0x80014F70` type=21 to `sub_80014F70`
- `0x80014F6C` -> `0x80014FF8` type=19 to `loc_80014FF8`
- `0x80014F70` -> `0x80014F74` type=21 to `sub_80014F74`
- `0x80014F74` -> `0x80014F78` type=21 to `sub_80014F78`
- `0x80014F78` -> `0x80014F80` type=21 to `sub_80014F80`
- `0x80014F78` -> `0x80048CA8` type=1 to `dword_80048CA8`
- `0x80014F80` -> `0x80014F84` type=21 to `sub_80014F84`
- `0x80014F84` -> `0x80014F88` type=21 to `sub_80014F88`
- `0x80014F88` -> `0x80014F8C` type=21 to `sub_80014F8C`
- `0x80014F8C` -> `0x80014F90` type=21 to `loc_80014F90`
- `0x80014F90` -> `0x80014F94` type=21 to `sub_80014F94`
- `0x80014F90` -> `0x80014FCC` type=19 to `loc_80014FCC`
- `0x80014F94` -> `0x80014F98` type=21 to `sub_80014F98`
- `0x80014F98` -> `0x80014FA0` type=21 to `sub_80014FA0`
- `0x80014F98` -> `0x800916E2` type=3 to `word_800916E2`
- `0x80014FA0` -> `0x80014FA4` type=21 to `sub_80014FA4`
- `0x80014FA4` -> `0x80014FA8` type=21 to `sub_80014FA8`
- `0x80014FA4` -> `0x80014FBC` type=19 to `loc_80014FBC`
- `0x80014FA8` -> `0x80014FAC` type=21 to `sub_80014FAC`
- `0x80014FAC` -> `0x80014FB0` type=21 to `sub_80014FB0`
- `0x80014FB0` -> `0x80014FB4` type=21 to `sub_80014FB4`
- `0x80014FB0` -> `0x80014FBC` type=19 to `loc_80014FBC`
- `0x80014FB4` -> `0x80014FB8` type=21 to `sub_80014FB8`
- `0x80014FB8` -> `0x80014FBC` type=21 to `loc_80014FBC`
- `0x80014FBC` -> `0x80014FC0` type=21 to `sub_80014FC0`
- `0x80014FC0` -> `0x80014A80` type=17 to `sub_80014A80`
- `0x80014FC0` -> `0x80014FC4` type=21 to `sub_80014FC4`
- `0x80014FC4` -> `0x80014FC8` type=21 to `sub_80014FC8`
- `0x80014FC8` -> `0x80014FCC` type=21 to `loc_80014FCC`
- `0x80014FCC` -> `0x80014FD0` type=21 to `sub_80014FD0`
- `0x80014FD0` -> `0x80014FD4` type=21 to `sub_80014FD4`
- `0x80014FD4` -> `0x80014F90` type=19 to `loc_80014F90`
- `0x80014FD4` -> `0x80014FD8` type=21 to `sub_80014FD8`
- `0x80014FD8` -> `0x80014FDC` type=21 to `sub_80014FDC`
- `0x80014FDC` -> `0x80014FE0` type=21 to `sub_80014FE0`
- `0x80014FDC` -> `0x80014FF8` type=19 to `loc_80014FF8`
- `0x80014FE4` -> `0x80014FE8` type=21 to `sub_80014FE8`
- `0x80014FE8` -> `0x80014FEC` type=21 to `sub_80014FEC`
- `0x80014FEC` -> `0x80014FF0` type=21 to `sub_80014FF0`
- `0x80014FF0` -> `0x80014FF4` type=21 to `sub_80014FF4`
- `0x80014FF4` -> `0x80014FF8` type=21 to `loc_80014FF8`
- `0x80014FF8` -> `0x80014FFC` type=21 to `sub_80014FFC`
- `0x80014FFC` -> `0x80015000` type=21 to `sub_80015000`
- `0x80015000` -> `0x80015004` type=21 to `sub_80015004`
- `0x80015004` -> `0x8001500C` type=21 to `sub_8001500C`
- `0x80015004` -> `0x80091814` type=3 to `word_80091814`
- `0x8001500C` -> `0x80015010` type=21 to `sub_80015010`
- `0x80015010` -> `0x80015014` type=21 to `sub_80015014`
- `0x80015014` -> `0x80015018` type=21 to `sub_80015018`
- `0x80015018` -> `0x8001501C` type=21 to `sub_8001501C`
- `0x8001501C` -> `0x80015020` type=21 to `sub_80015020`
- `0x80015020` -> `0x80015024` type=21 to `sub_80015024`
- `0x80015024` -> `0x80015028` type=21 to `sub_80015028`
- `0x80015028` -> `0x8001502C` type=21 to `sub_8001502C`
- `0x8001502C` -> `0x80015030` type=21 to `sub_80015030`
- `0x8001502C` -> `0x80015038` type=19 to `loc_80015038`
- `0x80015030` -> `0x80015034` type=21 to `sub_80015034`
- `0x80015034` -> `0x80015038` type=21 to `loc_80015038`
- `0x80015038` -> `0x80015040` type=21 to `sub_80015040`
- `0x80015038` -> `0x80091810` type=3 to `word_80091810`
- `0x80015040` -> `0x80015044` type=21 to `sub_80015044`
- `0x80015044` -> `0x80015048` type=21 to `sub_80015048`
- `0x80015048` -> `0x8001504C` type=21 to `sub_8001504C`
- `0x80015048` -> `0x80015060` type=19 to `loc_80015060`
- `0x8001504C` -> `0x80015050` type=21 to `sub_80015050`
- `0x80015050` -> `0x80015054` type=21 to `sub_80015054`
- `0x80015054` -> `0x80015058` type=21 to `sub_80015058`
- `0x80015058` -> `0x8001505C` type=21 to `sub_8001505C`
- `0x8001505C` -> `0x80015060` type=21 to `loc_80015060`
- `0x80015060` -> `0x80015064` type=21 to `sub_80015064`
- `0x80015060` -> `0x8001526C` type=19 to `loc_8001526C`
- `0x80015068` -> `0x8001506C` type=21 to `sub_8001506C`
- `0x8001506C` -> `0x80015070` type=21 to `sub_80015070`
- `0x80015070` -> `0x80015074` type=21 to `sub_80015074`
- `0x80015074` -> `0x8001507C` type=21 to `sub_8001507C`
- `0x80015074` -> `0x80091810` type=3 to `word_80091810`
- `0x8001507C` -> `0x80015080` type=21 to `sub_80015080`
- `0x80015080` -> `0x80015084` type=21 to `sub_80015084`
- `0x80015084` -> `0x80015088` type=21 to `sub_80015088`
- `0x80015088` -> `0x8001508C` type=21 to `sub_8001508C`
- `0x8001508C` -> `0x80015090` type=21 to `sub_80015090`
- `0x80015090` -> `0x80015094` type=21 to `sub_80015094`
- `0x80015094` -> `0x80015098` type=21 to `sub_80015098`
- `0x80015098` -> `0x8001509C` type=21 to `sub_8001509C`
- `0x8001509C` -> `0x800150A0` type=21 to `sub_800150A0`
- `0x800150A0` -> `0x800150A4` type=21 to `sub_800150A4`
- `0x800150A0` -> `0x800150D0` type=19 to `loc_800150D0`
- `0x800150A4` -> `0x800150A8` type=21 to `sub_800150A8`
- `0x800150A8` -> `0x800150AC` type=21 to `sub_800150AC`
- `0x800150AC` -> `0x800150B0` type=21 to `sub_800150B0`
- `0x800150B0` -> `0x800150B4` type=21 to `sub_800150B4`
- `0x800150B4` -> `0x800150B8` type=21 to `sub_800150B8`
- `0x800150B8` -> `0x800150BC` type=21 to `sub_800150BC`
- `0x800150BC` -> `0x800150C4` type=21 to `sub_800150C4`
- `0x800150BC` -> `0x80092910` type=1 to `unk_80092910`
- `0x800150C4` -> `0x800150C8` type=21 to `sub_800150C8`
- `0x800150C8` -> `0x800150CC` type=21 to `sub_800150CC`
- `0x800150C8` -> `0x8001511C` type=19 to `loc_8001511C`
- `0x800150D0` -> `0x800150D4` type=21 to `sub_800150D4`
- `0x800150D4` -> `0x800150D8` type=21 to `sub_800150D8`
- `0x800150D4` -> `0x80015120` type=19 to `loc_80015120`
- `0x800150D8` -> `0x800150DC` type=21 to `sub_800150DC`
- `0x800150DC` -> `0x800150E0` type=21 to `sub_800150E0`
- `0x800150E0` -> `0x800150E4` type=21 to `sub_800150E4`
- `0x800150E4` -> `0x800150E8` type=21 to `sub_800150E8`
- `0x800150E8` -> `0x800150EC` type=21 to `sub_800150EC`
- `0x800150EC` -> `0x800150F0` type=21 to `sub_800150F0`
- `0x800150F0` -> `0x800150F8` type=21 to `sub_800150F8`
- `0x800150F0` -> `0x80092910` type=1 to `unk_80092910`
- `0x800150F8` -> `0x800150FC` type=21 to `sub_800150FC`
- `0x800150FC` -> `0x80015100` type=21 to `sub_80015100`
- `0x80015100` -> `0x80015104` type=21 to `sub_80015104`
- `0x80015104` -> `0x80015108` type=21 to `sub_80015108`
- `0x80015108` -> `0x8001510C` type=21 to `sub_8001510C`
- `0x8001510C` -> `0x80015110` type=21 to `sub_80015110`
- `0x80015110` -> `0x80015114` type=21 to `sub_80015114`
- `0x80015114` -> `0x80015118` type=21 to `sub_80015118`
- `0x80015118` -> `0x8001511C` type=21 to `loc_8001511C`
- `0x8001511C` -> `0x80015120` type=21 to `loc_80015120`
- `0x80015120` -> `0x80015124` type=21 to `sub_80015124`
- `0x80015124` -> `0x80015128` type=21 to `sub_80015128`
- `0x80015128` -> `0x8001512C` type=21 to `sub_8001512C`
- `0x80015128` -> `0x800151B8` type=19 to `loc_800151B8`
- `0x8001512C` -> `0x80015130` type=21 to `sub_80015130`
- `0x80015130` -> `0x80015138` type=21 to `sub_80015138`
- `0x80015130` -> `0x800916E2` type=3 to `word_800916E2`
- `0x80015138` -> `0x80015140` type=21 to `sub_80015140`
- `0x80015138` -> `0x80048CA8` type=1 to `dword_80048CA8`
- `0x80015140` -> `0x80015144` type=21 to `sub_80015144`
- `0x80015144` -> `0x80015148` type=21 to `sub_80015148`
- `0x80015148` -> `0x8001514C` type=21 to `loc_8001514C`
- `0x8001514C` -> `0x80015150` type=21 to `sub_80015150`
- `0x80015150` -> `0x80015154` type=21 to `sub_80015154`
- `0x80015154` -> `0x80015158` type=21 to `sub_80015158`
- `0x80015158` -> `0x8001515C` type=21 to `sub_8001515C`
- `0x80015158` -> `0x80015174` type=19 to `loc_80015174`
- `0x8001515C` -> `0x80015160` type=21 to `sub_80015160`
- `0x80015160` -> `0x80015164` type=21 to `sub_80015164`
- `0x80015164` -> `0x80015168` type=21 to `sub_80015168`
- `0x80015164` -> `0x80015174` type=19 to `loc_80015174`
- `0x80015168` -> `0x8001516C` type=21 to `sub_8001516C`
- `0x8001516C` -> `0x80015170` type=21 to `sub_80015170`
- `0x80015170` -> `0x80015174` type=21 to `loc_80015174`
- `0x80015174` -> `0x80015178` type=21 to `sub_80015178`
- `0x80015174` -> `0x800151A4` type=19 to `loc_800151A4`
- `0x80015178` -> `0x8001517C` type=21 to `sub_8001517C`
- `0x8001517C` -> `0x80015180` type=21 to `loc_80015180`
- `0x80015180` -> `0x80015184` type=21 to `sub_80015184`
- `0x80015184` -> `0x80015188` type=21 to `sub_80015188`
- `0x80015188` -> `0x8001518C` type=21 to `sub_8001518C`
- `0x80015188` -> `0x80015194` type=19 to `loc_80015194`
- `0x8001518C` -> `0x80015190` type=21 to `sub_80015190`
- `0x80015190` -> `0x80015194` type=21 to `loc_80015194`
- `0x80015194` -> `0x80015198` type=21 to `sub_80015198`
- `0x80015198` -> `0x8001519C` type=21 to `sub_8001519C`
- `0x8001519C` -> `0x80015180` type=19 to `loc_80015180`
- `0x8001519C` -> `0x800151A0` type=21 to `sub_800151A0`
- `0x800151A0` -> `0x800151A4` type=21 to `loc_800151A4`
- `0x800151A4` -> `0x800151A8` type=21 to `sub_800151A8`
- `0x800151A8` -> `0x800151AC` type=21 to `sub_800151AC`
- `0x800151AC` -> `0x800151B0` type=21 to `sub_800151B0`
- `0x800151B0` -> `0x8001514C` type=19 to `loc_8001514C`
- `0x800151B0` -> `0x800151B4` type=21 to `sub_800151B4`
- `0x800151B4` -> `0x800151B8` type=21 to `loc_800151B8`
- `0x800151B8` -> `0x800151BC` type=21 to `sub_800151BC`
- `0x800151BC` -> `0x800151C0` type=21 to `sub_800151C0`
- `0x800151BC` -> `0x80015234` type=19 to `loc_80015234`
- `0x800151C0` -> `0x800151C4` type=21 to `sub_800151C4`
- `0x800151C4` -> `0x800151C8` type=21 to `sub_800151C8`
- `0x800151C4` -> `0x80015250` type=19 to `loc_80015250`
- `0x800151C8` -> `0x800151CC` type=21 to `sub_800151CC`
- `0x800151CC` -> `0x800151D4` type=21 to `sub_800151D4`
- `0x800151CC` -> `0x80048CA8` type=1 to `dword_80048CA8`
- `0x800151D4` -> `0x800151D8` type=21 to `sub_800151D8`
- `0x800151D8` -> `0x800151DC` type=21 to `sub_800151DC`
- `0x800151DC` -> `0x800151E0` type=21 to `sub_800151E0`
- `0x800151E0` -> `0x800151E4` type=21 to `loc_800151E4`
- `0x800151E4` -> `0x800151EC` type=21 to `sub_800151EC`
- `0x800151E4` -> `0x800916E2` type=3 to `word_800916E2`
- `0x800151EC` -> `0x800151F0` type=21 to `sub_800151F0`
- `0x800151F0` -> `0x800151F4` type=21 to `sub_800151F4`
- `0x800151F0` -> `0x80015208` type=19 to `loc_80015208`
- `0x800151F4` -> `0x800151F8` type=21 to `sub_800151F8`
- `0x800151F8` -> `0x800151FC` type=21 to `sub_800151FC`
- `0x800151FC` -> `0x80015200` type=21 to `sub_80015200`
- `0x800151FC` -> `0x80015208` type=19 to `loc_80015208`
- `0x80015200` -> `0x80015204` type=21 to `sub_80015204`
- `0x80015204` -> `0x80015208` type=21 to `loc_80015208`
- `0x80015208` -> `0x8001520C` type=21 to `sub_8001520C`
- `0x8001520C` -> `0x80015210` type=21 to `sub_80015210`
- `0x80015210` -> `0x80015214` type=21 to `sub_80015214`
- `0x80015214` -> `0x80014A80` type=17 to `sub_80014A80`
- `0x80015214` -> `0x80015218` type=21 to `sub_80015218`
- `0x80015218` -> `0x8001521C` type=21 to `sub_8001521C`
- `0x8001521C` -> `0x80015220` type=21 to `sub_80015220`
- `0x80015220` -> `0x80015224` type=21 to `sub_80015224`
- `0x80015224` -> `0x800151E4` type=19 to `loc_800151E4`
- `0x80015224` -> `0x80015228` type=21 to `sub_80015228`
- `0x80015228` -> `0x8001522C` type=21 to `sub_8001522C`
- `0x8001522C` -> `0x80015230` type=21 to `sub_80015230`
- `0x8001522C` -> `0x80015254` type=19 to `loc_80015254`
- `0x80015234` -> `0x8001523C` type=21 to `sub_8001523C`
- `0x80015234` -> `0x80091810` type=3 to `word_80091810`
- `0x8001523C` -> `0x80015240` type=21 to `sub_80015240`
- `0x80015240` -> `0x80015244` type=21 to `sub_80015244`
- `0x80015244` -> `0x80015248` type=21 to `sub_80015248`
- `0x80015244` -> `0x80015254` type=19 to `loc_80015254`
- `0x80015248` -> `0x8001524C` type=21 to `sub_8001524C`
- `0x8001524C` -> `0x80015250` type=21 to `loc_80015250`
- `0x80015250` -> `0x80015254` type=21 to `loc_80015254`
- `0x80015254` -> `0x8001525C` type=21 to `sub_8001525C`
- `0x80015254` -> `0x80091814` type=3 to `word_80091814`
- `0x8001525C` -> `0x80015260` type=21 to `sub_80015260`
- `0x80015260` -> `0x80015264` type=21 to `sub_80015264`
- `0x80015264` -> `0x80015268` type=21 to `sub_80015268`
- `0x80015268` -> `0x8001526C` type=21 to `loc_8001526C`
- `0x8001526C` -> `0x80015274` type=21 to `sub_80015274`
- `0x8001526C` -> `0x80091822` type=3 to `word_80091822`
- `0x80015274` -> `0x8001527C` type=21 to `sub_8001527C`
- `0x80015274` -> `0x80091816` type=3 to `word_80091816`
- `0x8001527C` -> `0x80015280` type=21 to `sub_80015280`
- `0x80015280` -> `0x80015284` type=21 to `sub_80015284`
- `0x80015284` -> `0x8001528C` type=21 to `sub_8001528C`
- `0x80015284` -> `0x80091816` type=2 to `word_80091816`
- `0x8001528C` -> `0x80015290` type=21 to `sub_80015290`
- `0x80015290` -> `0x80015294` type=21 to `sub_80015294`
- `0x80015290` -> `0x800152A0` type=19 to `loc_800152A0`
- `0x80015294` -> `0x80015298` type=21 to `sub_80015298`
- `0x80015298` -> `0x800152A0` type=21 to `loc_800152A0`
- `0x80015298` -> `0x80091816` type=2 to `word_80091816`
- `0x800152A0` -> `0x800152A4` type=21 to `sub_800152A4`
- `0x800152A4` -> `0x800152A8` type=21 to `sub_800152A8`
- `0x800152A8` -> `0x800152AC` type=21 to `sub_800152AC`
- `0x800152AC` -> `0x800152B0` type=21 to `sub_800152B0`
- `0x800152B0` -> `0x800152B4` type=21 to `sub_800152B4`
- `0x800152B4` -> `0x800152B8` type=21 to `sub_800152B8`
- `0x800152B8` -> `0x800152BC` type=21 to `sub_800152BC`
- `0x800152BC` -> `0x800152C0` type=21 to `sub_800152C0`
- `0x800152C0` -> `0x800152C4` type=21 to `sub_800152C4`
- `0x800152C4` -> `0x800152C8` type=21 to `sub_800152C8`
- `0x800152C8` -> `0x800152CC` type=21 to `sub_800152CC`

### Referenced 0x80000000-0x801FFFFF operands

- `0x80014D58` op1 type=2 -> `0x80091810` `word_80091810`
- `0x80014D88` op1 type=7 -> `0x80014D98` `loc_80014D98`
- `0x80014D90` op0 type=7 -> `0x8001526C` `loc_8001526C`
- `0x80014DA0` op1 type=7 -> `0x80015068` `loc_80015068`
- `0x80014DB8` op1 type=7 -> `0x80014DC8` `loc_80014DC8`
- `0x80014DC0` op0 type=7 -> `0x8001526C` `loc_8001526C`
- `0x80014DD4` op1 type=2 -> `0x80091810` `word_80091810`
- `0x80014DEC` op2 type=7 -> `0x80014E1C` `loc_80014E1C`
- `0x80014E08` op1 type=5 -> `0x80092910` `unk_80092910`
- `0x80014E14` op0 type=7 -> `0x80014E64` `loc_80014E64`
- `0x80014E1C` op2 type=7 -> `0x80014E68` `loc_80014E68`
- `0x80014E38` op1 type=5 -> `0x80092910` `unk_80092910`
- `0x80014E9C` op1 type=7 -> `0x80014F38` `loc_80014F38`
- `0x80014EA4` op1 type=2 -> `0x800916E2` `word_800916E2`
- `0x80014EAC` op1 type=5 -> `0x80048CA8` `dword_80048CA8`
- `0x80014ECC` op2 type=7 -> `0x80014EE4` `loc_80014EE4`
- `0x80014ED8` op2 type=7 -> `0x80014EE4` `loc_80014EE4`
- `0x80014EE4` op1 type=7 -> `0x80014F24` `loc_80014F24`
- `0x80014EF8` op1 type=7 -> `0x80014F14` `loc_80014F14`
- `0x80014F00` op1 type=7 -> `0x80014F10` `loc_80014F10`
- `0x80014F08` op1 type=7 -> `0x80014F14` `loc_80014F14`
- `0x80014F1C` op1 type=7 -> `0x80014EEC` `loc_80014EEC`
- `0x80014F30` op1 type=7 -> `0x80014EC0` `loc_80014EC0`
- `0x80014F38` op1 type=7 -> `0x80014F48` `loc_80014F48`
- `0x80014F40` op0 type=7 -> `0x80014F58` `loc_80014F58`
- `0x80014F5C` op2 type=7 -> `0x80014FE4` `loc_80014FE4`
- `0x80014F64` op2 type=7 -> `0x80014FE4` `loc_80014FE4`
- `0x80014F6C` op1 type=7 -> `0x80014FF8` `loc_80014FF8`
- `0x80014F78` op1 type=5 -> `0x80048CA8` `dword_80048CA8`
- `0x80014F90` op1 type=7 -> `0x80014FCC` `loc_80014FCC`
- `0x80014F98` op1 type=2 -> `0x800916E2` `word_800916E2`
- `0x80014FA4` op2 type=7 -> `0x80014FBC` `loc_80014FBC`
- `0x80014FB0` op2 type=7 -> `0x80014FBC` `loc_80014FBC`
- `0x80014FC0` op0 type=7 -> `0x80014A80` `sub_80014A80`
- `0x80014FD4` op1 type=7 -> `0x80014F90` `loc_80014F90`
- `0x80014FDC` op0 type=7 -> `0x80014FF8` `loc_80014FF8`
- `0x80015004` op1 type=2 -> `0x80091814` `word_80091814`
- `0x8001502C` op1 type=7 -> `0x80015038` `loc_80015038`
- `0x80015038` op1 type=2 -> `0x80091810` `word_80091810`
- `0x80015048` op1 type=7 -> `0x80015060` `loc_80015060`
- `0x80015060` op0 type=7 -> `0x8001526C` `loc_8001526C`
- `0x80015074` op1 type=2 -> `0x80091810` `word_80091810`
- `0x800150A0` op2 type=7 -> `0x800150D0` `loc_800150D0`
- `0x800150BC` op1 type=5 -> `0x80092910` `unk_80092910`
- `0x800150C8` op0 type=7 -> `0x8001511C` `loc_8001511C`
- `0x800150D4` op2 type=7 -> `0x80015120` `loc_80015120`
- `0x800150F0` op1 type=5 -> `0x80092910` `unk_80092910`
- `0x80015128` op1 type=7 -> `0x800151B8` `loc_800151B8`
- `0x80015130` op1 type=2 -> `0x800916E2` `word_800916E2`
- `0x80015138` op1 type=5 -> `0x80048CA8` `dword_80048CA8`
- `0x80015158` op2 type=7 -> `0x80015174` `loc_80015174`
- `0x80015164` op2 type=7 -> `0x80015174` `loc_80015174`
- `0x80015174` op1 type=7 -> `0x800151A4` `loc_800151A4`
- `0x80015188` op1 type=7 -> `0x80015194` `loc_80015194`
- `0x8001519C` op1 type=7 -> `0x80015180` `loc_80015180`
- `0x800151B0` op1 type=7 -> `0x8001514C` `loc_8001514C`
- `0x800151BC` op2 type=7 -> `0x80015234` `loc_80015234`
- `0x800151C4` op1 type=7 -> `0x80015250` `loc_80015250`
- `0x800151CC` op1 type=5 -> `0x80048CA8` `dword_80048CA8`
- `0x800151E4` op1 type=2 -> `0x800916E2` `word_800916E2`
- `0x800151F0` op2 type=7 -> `0x80015208` `loc_80015208`
- `0x800151FC` op2 type=7 -> `0x80015208` `loc_80015208`
- `0x80015214` op0 type=7 -> `0x80014A80` `sub_80014A80`
- `0x80015224` op1 type=7 -> `0x800151E4` `loc_800151E4`
- `0x8001522C` op0 type=7 -> `0x80015254` `loc_80015254`
- `0x80015234` op1 type=2 -> `0x80091810` `word_80091810`
- `0x80015244` op1 type=7 -> `0x80015254` `loc_80015254`
- `0x80015254` op1 type=2 -> `0x80091814` `word_80091814`
- `0x8001526C` op1 type=2 -> `0x80091822` `word_80091822`
- `0x80015274` op1 type=2 -> `0x80091816` `word_80091816`
- `0x80015284` op1 type=2 -> `0x80091816` `word_80091816`
- `0x80015290` op1 type=7 -> `0x800152A0` `loc_800152A0`
- `0x80015298` op1 type=2 -> `0x80091816` `word_80091816`

## 0x80014D28 `sub_80014D28` - positive-growth classifier

- Bounds: `0x80014D28` - `0x80014D58`
- Size: `48` bytes

### Hex-Rays pseudo-C

```c
BOOL sub_80014D28()
{
  int v0; // $v1

  v0 = word_80091818;
  word_80091818 = word_80091816;
  return word_80091816 - v0 > 0;
}

```

### Disassembly

```asm
0x80014D28: lh      $v0, word_80091816
0x80014D30: lh      $v1, word_80091818
0x80014D38: addiu   $sp, -8
0x80014D3C: sh      $v0, word_80091818
0x80014D44: subu    $v0, $v1
0x80014D48: slt     $v0, $zero, $v0
0x80014D4C: addiu   $sp, 8
0x80014D50: jr      $ra
0x80014D54: nop
```

### Xrefs to

- `0x800253A4` -> `0x80014D28` type=17 from `loc_800253A4`

### Xrefs from

- `0x80014D28` -> `0x80014D30` type=21 to `sub_80014D30`
- `0x80014D28` -> `0x80091816` type=3 to `word_80091816`
- `0x80014D30` -> `0x80014D38` type=21 to `sub_80014D38`
- `0x80014D30` -> `0x80091818` type=3 to `word_80091818`
- `0x80014D38` -> `0x80014D3C` type=21 to `sub_80014D3C`
- `0x80014D3C` -> `0x80014D44` type=21 to `sub_80014D44`
- `0x80014D3C` -> `0x80091818` type=2 to `word_80091818`
- `0x80014D44` -> `0x80014D48` type=21 to `sub_80014D48`
- `0x80014D48` -> `0x80014D4C` type=21 to `sub_80014D4C`
- `0x80014D4C` -> `0x80014D50` type=21 to `sub_80014D50`
- `0x80014D50` -> `0x80014D54` type=21 to `sub_80014D54`

### Referenced 0x80000000-0x801FFFFF operands

- `0x80014D28` op1 type=2 -> `0x80091816` `word_80091816`
- `0x80014D30` op1 type=2 -> `0x80091818` `word_80091818`
- `0x80014D3C` op1 type=2 -> `0x80091818` `word_80091818`

## 0x80014548 `sub_80014548` - bucket30 tie-breaker

- Bounds: `0x80014548` - `0x80014614`
- Size: `204` bytes

### Hex-Rays pseudo-C

```c
int __fastcall sub_80014548(int a1)
{
  int result; // $v0
  int v2; // $v1
  int v3; // $v1
  int v4; // $a1

  result = 2;
  if ( *(_DWORD *)(a1 + 64) )
  {
    v2 = word_8009181C;
    word_8009181A = word_80091816;
    word_8009181C = word_80091816;
    v3 = word_80091816 - v2;
    v4 = *(__int16 *)(a1 + 78);
    if ( v4 == 1 )
    {
      result = 1;
      if ( *(__int16 *)(*(_DWORD *)(a1 + 64) + 4) < v3 )
        return result;
      goto LABEL_7;
    }
    if ( v4 == 2 )
    {
      result = 1;
      if ( v3 > 0 )
        return result;
LABEL_7:
      if ( v3 >= 0 )
        return 2 * (word_8009181E != 1);
      return 0;
    }
    result = 1;
    if ( v3 <= 0 )
    {
      result = 2;
      if ( v3 < 0 )
      {
        word_8009181E = 1;
        return 0;
      }
    }
  }
  return result;
}

```

### Disassembly

```asm
0x80014548: lw      $v0, 0x40($a0)
0x8001454C: nop
0x80014550: beqz    $v0, locret_8001460C
0x80014554: li      $v0, 2
0x80014558: lhu     $v0, word_80091816
0x80014560: lh      $v1, word_8009181C
0x80014568: sh      $v0, word_8009181A
0x80014570: sh      $v0, word_8009181C
0x80014578: sll     $v0, 16
0x8001457C: sra     $v0, 16
0x80014580: subu    $v1, $v0, $v1
0x80014584: lh      $a1, 0x4E($a0)
0x80014588: li      $v0, 1
0x8001458C: bne     $a1, $v0, loc_800145B8
0x80014590: li      $v0, 2
0x80014594: lw      $v0, 0x40($a0)
0x80014598: nop
0x8001459C: lh      $v0, 4($v0)
0x800145A0: nop
0x800145A4: slt     $v0, $v1
0x800145A8: bnez    $v0, locret_8001460C
0x800145AC: li      $v0, 1
0x800145B0: j       loc_800145C8
0x800145B4: nop
0x800145B8: bne     $a1, $v0, loc_800145EC
0x800145BC: nop
0x800145C0: bgtz    $v1, locret_8001460C
0x800145C4: li      $v0, 1
0x800145C8: bltz    $v1, loc_80014608
0x800145CC: nop
0x800145D0: lh      $v0, word_8009181E
0x800145D8: nop
0x800145DC: xori    $v0, 1
0x800145E0: sltu    $v0, $zero, $v0
0x800145E4: j       locret_8001460C
0x800145E8: sll     $v0, 1
0x800145EC: bgtz    $v1, locret_8001460C
0x800145F0: li      $v0, 1
0x800145F4: bgez    $v1, locret_8001460C
0x800145F8: li      $v0, 2
0x800145FC: li      $v0, 1
0x80014600: sh      $v0, word_8009181E
0x80014608: move    $v0, $zero
0x8001460C: jr      $ra
0x80014610: nop
```

### Xrefs to

- `0x80025504` -> `0x80014548` type=17 from `sub_80025504`
- `0x80025534` -> `0x80014548` type=17 from `sub_80025534`

### Xrefs from

- `0x80014548` -> `0x8001454C` type=21 to `sub_8001454C`
- `0x8001454C` -> `0x80014550` type=21 to `sub_80014550`
- `0x80014550` -> `0x80014554` type=21 to `sub_80014554`
- `0x80014550` -> `0x8001460C` type=19 to `locret_8001460C`
- `0x80014554` -> `0x80014558` type=21 to `sub_80014558`
- `0x80014558` -> `0x80014560` type=21 to `sub_80014560`
- `0x80014558` -> `0x80091816` type=3 to `word_80091816`
- `0x80014560` -> `0x80014568` type=21 to `sub_80014568`
- `0x80014560` -> `0x8009181C` type=3 to `word_8009181C`
- `0x80014568` -> `0x80014570` type=21 to `sub_80014570`
- `0x80014568` -> `0x8009181A` type=2 to `word_8009181A`
- `0x80014570` -> `0x80014578` type=21 to `sub_80014578`
- `0x80014570` -> `0x8009181C` type=2 to `word_8009181C`
- `0x80014578` -> `0x8001457C` type=21 to `sub_8001457C`
- `0x8001457C` -> `0x80014580` type=21 to `sub_80014580`
- `0x80014580` -> `0x80014584` type=21 to `sub_80014584`
- `0x80014584` -> `0x80014588` type=21 to `sub_80014588`
- `0x80014588` -> `0x8001458C` type=21 to `sub_8001458C`
- `0x8001458C` -> `0x80014590` type=21 to `sub_80014590`
- `0x8001458C` -> `0x800145B8` type=19 to `loc_800145B8`
- `0x80014590` -> `0x80014594` type=21 to `sub_80014594`
- `0x80014594` -> `0x80014598` type=21 to `sub_80014598`
- `0x80014598` -> `0x8001459C` type=21 to `sub_8001459C`
- `0x8001459C` -> `0x800145A0` type=21 to `sub_800145A0`
- `0x800145A0` -> `0x800145A4` type=21 to `sub_800145A4`
- `0x800145A4` -> `0x800145A8` type=21 to `sub_800145A8`
- `0x800145A8` -> `0x800145AC` type=21 to `sub_800145AC`
- `0x800145A8` -> `0x8001460C` type=19 to `locret_8001460C`
- `0x800145AC` -> `0x800145B0` type=21 to `sub_800145B0`
- `0x800145B0` -> `0x800145B4` type=21 to `sub_800145B4`
- `0x800145B0` -> `0x800145C8` type=19 to `loc_800145C8`
- `0x800145B8` -> `0x800145BC` type=21 to `sub_800145BC`
- `0x800145B8` -> `0x800145EC` type=19 to `loc_800145EC`
- `0x800145BC` -> `0x800145C0` type=21 to `sub_800145C0`
- `0x800145C0` -> `0x800145C4` type=21 to `sub_800145C4`
- `0x800145C0` -> `0x8001460C` type=19 to `locret_8001460C`
- `0x800145C4` -> `0x800145C8` type=21 to `loc_800145C8`
- `0x800145C8` -> `0x800145CC` type=21 to `sub_800145CC`
- `0x800145C8` -> `0x80014608` type=19 to `loc_80014608`
- `0x800145CC` -> `0x800145D0` type=21 to `sub_800145D0`
- `0x800145D0` -> `0x800145D8` type=21 to `sub_800145D8`
- `0x800145D0` -> `0x8009181E` type=3 to `word_8009181E`
- `0x800145D8` -> `0x800145DC` type=21 to `sub_800145DC`
- `0x800145DC` -> `0x800145E0` type=21 to `sub_800145E0`
- `0x800145E0` -> `0x800145E4` type=21 to `sub_800145E4`
- `0x800145E4` -> `0x800145E8` type=21 to `sub_800145E8`
- `0x800145E4` -> `0x8001460C` type=19 to `locret_8001460C`
- `0x800145EC` -> `0x800145F0` type=21 to `sub_800145F0`
- `0x800145EC` -> `0x8001460C` type=19 to `locret_8001460C`
- `0x800145F0` -> `0x800145F4` type=21 to `sub_800145F4`
- `0x800145F4` -> `0x800145F8` type=21 to `sub_800145F8`
- `0x800145F4` -> `0x8001460C` type=19 to `locret_8001460C`
- `0x800145F8` -> `0x800145FC` type=21 to `sub_800145FC`
- `0x800145FC` -> `0x80014600` type=21 to `sub_80014600`
- `0x80014600` -> `0x80014608` type=21 to `loc_80014608`
- `0x80014600` -> `0x8009181E` type=2 to `word_8009181E`
- `0x80014608` -> `0x8001460C` type=21 to `locret_8001460C`
- `0x8001460C` -> `0x80014610` type=21 to `sub_80014610`

### Referenced 0x80000000-0x801FFFFF operands

- `0x80014550` op1 type=7 -> `0x8001460C` `locret_8001460C`
- `0x80014558` op1 type=2 -> `0x80091816` `word_80091816`
- `0x80014560` op1 type=2 -> `0x8009181C` `word_8009181C`
- `0x80014568` op1 type=2 -> `0x8009181A` `word_8009181A`
- `0x80014570` op1 type=2 -> `0x8009181C` `word_8009181C`
- `0x8001458C` op2 type=7 -> `0x800145B8` `loc_800145B8`
- `0x800145A8` op1 type=7 -> `0x8001460C` `locret_8001460C`
- `0x800145B0` op0 type=7 -> `0x800145C8` `loc_800145C8`
- `0x800145B8` op2 type=7 -> `0x800145EC` `loc_800145EC`
- `0x800145C0` op1 type=7 -> `0x8001460C` `locret_8001460C`
- `0x800145C8` op1 type=7 -> `0x80014608` `loc_80014608`
- `0x800145D0` op1 type=2 -> `0x8009181E` `word_8009181E`
- `0x800145E4` op0 type=7 -> `0x8001460C` `locret_8001460C`
- `0x800145EC` op1 type=7 -> `0x8001460C` `locret_8001460C`
- `0x800145F4` op1 type=7 -> `0x8001460C` `locret_8001460C`
- `0x80014600` op1 type=2 -> `0x8009181E` `word_8009181E`

## 0x80024FD0 `sub_80024FD0` - bucket dispatcher / bucket30 owner

- Bounds: `0x80024FD0` - `0x800259C0`
- Size: `2544` bytes

### Hex-Rays pseudo-C

```c
int __fastcall sub_80024FD0(int a1)
{
  int v2; // $a0
  int v3; // $a1
  int v4; // $a3
  int v5; // $v1
  int v6; // $t0
  int v7; // $a1
  int result; // $v0
  int v9; // $s0
  int v10; // $v1
  unsigned __int8 *v11; // $a0
  int v12; // $v1
  int v13; // $v0
  int v14; // $v0
  int v15; // $s0
  int v16; // $v1
  BOOL v17; // $a0
  int v18; // $v1
  _BOOL2 v19; // $a0
  int v20; // $v0
  int v21; // $s3
  int v22; // $s2
  int v23; // $v1
  int v24; // $a0
  int v25; // $v1
  int v26; // $v1
  __int16 v27; // $v0
  int v28; // $s0
  int v29; // $v1
  int v30; // $v0
  int v31; // $v1
  __int16 v32; // $v0
  __int16 v33; // $v0
  int v34; // $v1

  if ( dword_8008ED20 == 1 )
    return a1;
  v2 = *(_DWORD *)(a1 + 12);
  v3 = v2 / 384;
  v4 = v2 % 384;
  if ( word_800916D0 == 2 || (v5 = v2 + 16, word_800916D0 == 1) )
  {
    v5 = v2 + 12;
    v6 = v3 + 1;
    if ( v4 < 372 )
    {
      v7 = v3 + 1;
      goto LABEL_9;
    }
    goto LABEL_7;
  }
  v6 = v3 + 1;
  if ( v4 >= 368 )
  {
LABEL_7:
    v7 = v5 / 384 + 1;
    goto LABEL_9;
  }
  v7 = v3 + 1;
LABEL_9:
  dword_8008ECE8 = v2 % 384;
  dword_8008ECF4 = v4 / 12;
  result = a1;
  if ( dword_8008ECF0 == v4 / 12 )
    return result;
  dword_8008ECF0 = v4 / 12;
  if ( v7 < dword_800943C8 )
    *(_DWORD *)(a1 + 68) = dword_800943C4 + 24 * v7;
  *(_DWORD *)(a1 + 56) = v6;
  v9 = 0;
  if ( !dword_8008ECF4 )
  {
    v10 = *(_DWORD *)(a1 + 56);
    *(_DWORD *)a1 |= 2u;
    if ( v10 < dword_800943C8 )
    {
      v11 = (unsigned __int8 *)(dword_800943C4 + 24 * v10);
      *(_DWORD *)(a1 + 64) = v11;
      v9 = *v11;
      if ( v11 )
        dword_8008ED08 = *(unsigned __int16 *)&v11[6 * *(__int16 *)(a1 + 80) + 16];
      else
        dword_8008ED08 = 0;
    }
  }
  if ( (dword_8008ECF4 & 1) == 0 )
  {
    v12 = *(_DWORD *)a1;
    *(_DWORD *)a1 |= 8u;
    if ( (dword_8008ECF4 & 7) == 0 )
      *(_DWORD *)a1 = v12 | 0xC;
  }
  if ( dword_8008ECF4 == *(_DWORD *)dword_800943C0 )
  {
    if ( sub_80024BF4(a1) != 1
      && ((dword_8008ED08 & 0x10) != 0 && (unsigned int)*(unsigned __int16 *)(a1 + 78) - 2 < 2
       || (dword_8008ED08 & 8) != 0 && sub_800144B8(a1) == 1) )
    {
      v13 = *(_DWORD *)a1;
      *(_WORD *)(a1 + 118) = 1;
      *(_WORD *)(a1 + 84) = 1;
      *(_DWORD *)a1 = v13 | 0x40;
    }
    if ( (dword_8008ED08 & 0x80) != 0 )
      sub_800152D0(a1);
    if ( *(__int16 *)(a1 + 94) == v9 || !v9 )
    {
      *(_WORD *)(a1 + 92) = 0;
    }
    else
    {
      v14 = *(__int16 *)(a1 + 78);
      *(_WORD *)(a1 + 94) = v9;
      if ( v14 )
        *(_WORD *)(a1 + 92) = 1;
    }
  }
  if ( dword_8008ECF4 == *(_DWORD *)(dword_800943C0 + 4) )
  {
    if ( sub_80024BF4(a1) )
      goto LABEL_106;
    if ( (dword_8008ED08 & 3) != 0 )
    {
      sub_80014D58(a1);
      sub_80014458(a1);
    }
    if ( (dword_8008ED08 & 2) == 0 )
      goto LABEL_106;
    v15 = 0;
    if ( !*(_WORD *)(a1 + 78) )
    {
      v16 = sub_80014C80(a1);
      switch ( v16 )
      {
        case 0:
          v15 = 1;
LABEL_60:
          *(_WORD *)(a1 + 90) = 0;
          break;
        case 1:
          *(_WORD *)(a1 + 88) = 1;
          *(_WORD *)(a1 + 90) = 1;
          break;
        case 2:
          *(_WORD *)(a1 + 88) = 3;
          *(_WORD *)(a1 + 90) = 1;
          break;
      }
LABEL_61:
      v21 = v15;
      if ( dword_8008ED00 == 1 || dword_8008ED00 == 4 )
      {
LABEL_103:
        sub_80026EF8(dword_80094400 + 6 * v21);
        sub_80014C1C(a1);
LABEL_104:
        if ( v21 == 1 )
          dword_80094438(a1);
        goto LABEL_106;
      }
      if ( dword_8008ED00 )
        goto LABEL_104;
      if ( (dword_8008ED08 & 4) == 0 )
        goto LABEL_78;
      if ( word_8008ED36 == 1 )
      {
        if ( *(__int16 *)(a1 + 78) != word_8008ED36 || (v22 = 2, sub_80014548(a1) == 1) )
          v22 = 1;
      }
      else
      {
        v22 = 2;
        if ( !word_8008ED36 )
        {
          v22 = 0;
          if ( *(_WORD *)(a1 + 78) == 3 )
            v22 = 2 * (sub_80014548(a1) != 0);
        }
      }
      sub_80024F8C(a1);
      if ( v22 == 2 )
        goto LABEL_78;
      v23 = *(__int16 *)(a1 + 78);
      if ( v23 == 3 && !v22 )
        goto LABEL_78;
      if ( v23 == 1 && v22 == 1 )
      {
        v24 = 2;
        if ( !word_8009182A )
        {
LABEL_78:
          v25 = 0;
          goto LABEL_102;
        }
      }
      else
      {
        v24 = 2 * v22;
      }
      sub_80026EF8(dword_800943FC + 2 * (v24 + v22));
      if ( *(_WORD *)(a1 + 78) == 1 && v22 == 1 )
      {
        v26 = *(_DWORD *)a1 | 0x2000;
        v27 = *(_WORD *)(a1 + 114) + 1;
        *(_WORD *)(a1 + 114) = v27;
        *(_DWORD *)a1 = v26;
        if ( v27 >= 2 )
          v28 = dword_800943F8;
        else
          v28 = dword_800943F4;
        sub_80026EF8(v28);
        v29 = *(__int16 *)(v28 + 4);
        v30 = *(_DWORD *)(a1 + 12);
        dword_8008ED14 = 1;
        dword_8008ED0C = v30 + v29;
        *(_WORD *)(a1 + 80) = 1;
        *(_WORD *)(a1 + 138) = 0;
        dword_8008ED00 = 7;
        dword_8008ED24 = 1;
      }
      else
      {
        dword_8008ED00 = 6;
        *(_WORD *)(a1 + 80) = 0;
      }
      v31 = *(__int16 *)(a1 + 78);
      if ( v31 == 1 )
      {
        if ( v22 == 1 )
        {
          *(_WORD *)(a1 + 78) = 0;
          *(_WORD *)(a1 + 398) = 5;
          *(_WORD *)(a1 + 122) = 0;
        }
        else
        {
          *(_WORD *)(a1 + 78) = 2;
          v32 = *(_WORD *)(a1 + 116);
          *(_WORD *)(a1 + 398) = 1;
          *(_WORD *)(a1 + 116) = v32 + 1;
        }
        goto LABEL_99;
      }
      if ( v31 == 2 )
      {
        if ( v22 == 1 )
        {
          *(_WORD *)(a1 + 78) = 1;
          v33 = 4;
        }
        else
        {
          *(_WORD *)(a1 + 78) = 3;
          v33 = 2;
        }
      }
      else
      {
        if ( v31 != 3 || v22 != 1 )
        {
LABEL_99:
          v25 = 1;
          if ( !*(_WORD *)(a1 + 78) && !v22 )
          {
            *(_WORD *)(a1 + 78) = 1;
            *(_WORD *)(a1 + 398) = 6;
            v25 = 1;
          }
LABEL_102:
          if ( v25 == 1 )
          {
LABEL_106:
            if ( dword_8008ED00 )
            {
              if ( dword_8008ED00 == 1 && sub_80024BF4(a1) != 1 && (dword_8008ED08 & 0x20) != 0 && sub_80014538(a1) )
                dword_8008ED00 = 4;
              v34 = dword_8008ED00;
              if ( dword_8008ED00 == 4 && (dword_8008ED08 & 0x20) != 0 )
              {
                sub_80026EF8(dword_800943F0);
                *(_DWORD *)a1 |= 0x4000u;
                dword_8008ED14 = 1;
                dword_8008ED00 = 5;
                v34 = 5;
              }
              if ( v34 == 5 && (dword_8008ED08 & 0x40) != 0 )
              {
                dword_8008ED00 = 8;
                g_PrStageEventStreamDone = 1;
                dword_8008ED14 = 0;
                *(_WORD *)(a1 + 78) = 1;
                *(_WORD *)(a1 + 398) = 6;
                *(_WORD *)(a1 + 80) = 0;
                *(_WORD *)(a1 + 84) = 1;
                *(_WORD *)(a1 + 122) = 1;
                word_8008ED36 = 1;
              }
            }
            sub_80024FC0(a1);
            goto LABEL_119;
          }
          goto LABEL_103;
        }
        *(_WORD *)(a1 + 78) = 2;
        v33 = 3;
      }
      *(_WORD *)(a1 + 398) = v33;
      goto LABEL_99;
    }
    v15 = sub_80014D28(a1);
    if ( !word_8008ED38 )
    {
      v17 = 0;
      if ( *(_WORD *)(a1 + 78) == 1 && v15 == 1 )
        v17 = word_8008ED34 == 0;
      if ( !v17 )
      {
        v18 = *(__int16 *)(a1 + 78);
        *(_WORD *)(a1 + 90) = 1;
        *(_WORD *)(a1 + 88) = *((_WORD *)&dword_80055420[3 * v18] + v15);
      }
      word_8008ED36 = v15;
      ++word_8008ED38;
      goto LABEL_61;
    }
    v19 = 0;
    if ( *(_WORD *)(a1 + 78) == 3 )
    {
      v20 = word_8008ED36;
      if ( word_8008ED36 )
      {
LABEL_57:
        word_8009181E = v19;
        if ( v20 != v15 )
          word_8008ED36 = 2;
        word_8008ED38 = 0;
        goto LABEL_60;
      }
      v19 = v15 == 0;
    }
    v20 = word_8008ED36;
    goto LABEL_57;
  }
LABEL_119:
  if ( dword_8008ECF4 == 31 )
    sub_80014BDC(*(_DWORD *)(a1 + 56));
  result = a1;
  if ( dword_8008ECF4 == *(_DWORD *)(dword_800943C0 + 8) )
  {
    result = a1;
    if ( dword_8008ED00 >= 6 )
    {
      switch ( dword_8008ED00 )
      {
        case 6:
          dword_8008ED00 = 0;
          break;
        case 7:
          dword_8008ED00 = 1;
          break;
        case 8:
          dword_8008ED00 = 0;
          sub_80026EF8(dword_800943FC);
          break;
      }
      *(_DWORD *)a1 |= 0x200u;
      sub_80014400();
      sub_80014C1C(a1);
      sub_80024F8C(a1);
      return a1;
    }
  }
  return result;
}

```

### Disassembly

```asm
0x80024FD0: addiu   $sp, -0x28
0x80024FD4: sw      $s1, 0x10+var_s4($sp)
0x80024FD8: move    $s1, $a0
0x80024FDC: lw      $v0, dword_8008ED20
0x80024FE4: li      $t0, 1
0x80024FE8: sw      $ra, 0x10+var_s10($sp)
0x80024FEC: sw      $s3, 0x10+var_sC($sp)
0x80024FF0: sw      $s2, 0x10+var_s8($sp)
0x80024FF4: beq     $v0, $t0, loc_8002599C
0x80024FF8: sw      $s0, 0x10+var_s0($sp)
0x80024FFC: lui     $a2, 0x2AAA
0x80025000: lw      $a0, 0xC($s1)
0x80025004: li      $a2, 0x2AAAAAAB
0x80025008: mult    $a0, $a2
0x8002500C: sra     $v0, $a0, 31
0x80025010: mfhi    $t1
0x80025014: sra     $v1, $t1, 6
0x80025018: subu    $a1, $v1, $v0
0x8002501C: sll     $v0, $a1, 1
0x80025020: addu    $v0, $a1
0x80025024: sll     $v0, 7
0x80025028: subu    $a3, $a0, $v0
0x8002502C: lh      $v1, word_800916D0
0x80025034: li      $v0, 2
0x80025038: bne     $v1, $v0, loc_80025058
0x8002503C: nop
0x80025040: addiu   $v1, $a0, 0xC
0x80025044: slti    $v0, $a3, 0x174
0x80025048: beqz    $v0, loc_8002506C
0x8002504C: addiu   $t0, $a1, 1
0x80025050: j       loc_8002508C
0x80025054: move    $a1, $t0
0x80025058: beq     $v1, $t0, loc_80025040
0x8002505C: addiu   $v1, $a0, 0x10
0x80025060: slti    $v0, $a3, 0x170
0x80025064: bnez    $v0, loc_80025088
0x80025068: addiu   $t0, $a1, 1
0x8002506C: mult    $v1, $a2
0x80025070: sra     $v1, 31
0x80025074: mfhi    $t1
0x80025078: sra     $v0, $t1, 6
0x8002507C: subu    $v0, $v1
0x80025080: j       loc_8002508C
0x80025084: addiu   $a1, $v0, 1
0x80025088: move    $a1, $t0
0x8002508C: li      $v0, 0x2AAAAAAB
0x80025094: mult    $a3, $v0
0x80025098: li      $a2, dword_8008ECE8
0x800250A0: sra     $v0, $a3, 31
0x800250A4: sw      $a3, (dword_8008ECE8 - 0x8008ECE8)($a2)
0x800250A8: lw      $a0, dword_8008ECF0
0x800250B0: mfhi    $t1
0x800250B4: sra     $v1, $t1, 1
0x800250B8: subu    $v1, $v0
0x800250BC: sw      $v1, dword_8008ECF4
0x800250C4: beq     $a0, $v1, loc_800259A0
0x800250C8: move    $v0, $s1
0x800250CC: li      $a0, dword_800943C8
0x800250D4: lw      $v0, (dword_800943C8 - 0x800943C8)($a0)
0x800250D8: sw      $v1, dword_8008ECF0
0x800250E0: slt     $v0, $a1, $v0
0x800250E4: beqz    $v0, loc_80025104
0x800250E8: sll     $v0, $a1, 1
0x800250EC: addu    $v0, $a1
0x800250F0: lw      $v1, dword_800943C4
0x800250F8: sll     $v0, 3
0x800250FC: addu    $v1, $v0
0x80025100: sw      $v1, 0x44($s1)
0x80025104: sw      $t0, 0x38($s1)
0x80025108: lw      $v0, dword_8008ECF4
0x80025110: nop
0x80025114: bnez    $v0, loc_80025194
0x80025118: move    $s0, $zero
0x8002511C: lw      $v0, 0($s1)
0x80025120: lw      $v1, 0x38($s1)
0x80025124: ori     $v0, 2
0x80025128: sw      $v0, 0($s1)
0x8002512C: lw      $v0, (dword_800943C8 - 0x800943C8)($a0)
0x80025130: nop
0x80025134: slt     $v0, $v1, $v0
0x80025138: beqz    $v0, loc_80025194
0x8002513C: sll     $v0, $v1, 1
0x80025140: addu    $v0, $v1
0x80025144: sll     $v0, 3
0x80025148: lw      $v1, dword_800943C4
0x80025150: nop
0x80025154: addu    $a0, $v1, $v0
0x80025158: sw      $a0, 0x40($s1)
0x8002515C: lbu     $s0, 0($a0)
0x80025160: beqz    $a0, loc_8002518C
0x80025164: addiu   $a1, $a2, (dword_8008ED08 - 0x8008ECE8)
0x80025168: lh      $v1, 0x50($s1)
0x8002516C: nop
0x80025170: sll     $v0, $v1, 1
0x80025174: addu    $v0, $v1
0x80025178: sll     $v0, 1
0x8002517C: addu    $v0, $a0, $v0
0x80025180: lhu     $v0, 0x10($v0)
0x80025184: j       loc_80025194
0x80025188: sw      $v0, (dword_8008ED08 - 0x8008ED08)($a1)
0x8002518C: move    $v0, $zero
0x80025190: sw      $v0, (dword_8008ED08 - 0x8008ED08)($a1)
0x80025194: li      $a0, dword_8008ECF4
0x8002519C: lw      $v0, (dword_8008ECF4 - 0x8008ECF4)($a0)
0x800251A0: nop
0x800251A4: andi    $v0, 1
0x800251A8: bnez    $v0, loc_800251D8
0x800251AC: nop
0x800251B0: lw      $v1, 0($s1)
0x800251B4: nop
0x800251B8: ori     $v0, $v1, 8
0x800251BC: sw      $v0, 0($s1)
0x800251C0: lw      $v0, (dword_8008ECF4 - 0x8008ECF4)($a0)
0x800251C4: nop
0x800251C8: andi    $v0, 7
0x800251CC: bnez    $v0, loc_800251D8
0x800251D0: ori     $v0, $v1, 0xC
0x800251D4: sw      $v0, 0($s1)
0x800251D8: lw      $v0, dword_800943C0
0x800251E0: lw      $v1, dword_8008ECF4
0x800251E8: lw      $v0, 0($v0)
0x800251EC: nop
0x800251F0: bne     $v1, $v0, loc_800252D8
0x800251F4: nop
0x800251F8: jal     sub_80024BF4
0x800251FC: move    $a0, $s1
0x80025200: li      $v1, 1
0x80025204: beq     $v0, $v1, loc_80025280
0x80025208: nop
0x8002520C: lw      $v0, dword_8008ED08
0x80025214: nop
0x80025218: andi    $v0, 0x10
0x8002521C: beqz    $v0, loc_8002523C
0x80025220: nop
0x80025224: lhu     $v0, 0x4E($s1)
0x80025228: nop
0x8002522C: addiu   $v0, -2
0x80025230: sltiu   $v0, 2
0x80025234: bnez    $v0, loc_80025268
0x80025238: nop
0x8002523C: lw      $v0, dword_8008ED08
0x80025244: nop
0x80025248: andi    $v0, 8
0x8002524C: beqz    $v0, loc_80025280
0x80025250: nop
0x80025254: jal     sub_800144B8
0x80025258: move    $a0, $s1
0x8002525C: li      $v1, 1
0x80025260: bne     $v0, $v1, loc_80025280
0x80025264: nop
0x80025268: lw      $v0, 0($s1)
0x8002526C: li      $v1, 1
0x80025270: sh      $v1, 0x76($s1)
0x80025274: sh      $v1, 0x54($s1)
0x80025278: ori     $v0, 0x40  # '@'
0x8002527C: sw      $v0, 0($s1)
0x80025280: lw      $v0, dword_8008ED08
0x80025288: nop
0x8002528C: andi    $v0, 0x80
0x80025290: beqz    $v0, loc_800252A0
0x80025294: nop
0x80025298: jal     sub_800152D0
0x8002529C: move    $a0, $s1
0x800252A0: lh      $v0, 0x5E($s1)
0x800252A4: nop
0x800252A8: beq     $v0, $s0, loc_800252D4
0x800252AC: nop
0x800252B0: beqz    $s0, loc_800252D4
0x800252B4: nop
0x800252B8: lh      $v0, 0x4E($s1)
0x800252BC: nop
0x800252C0: beqz    $v0, loc_800252D8
0x800252C4: sh      $s0, 0x5E($s1)
0x800252C8: li      $v0, 1
0x800252CC: j       loc_800252D8
0x800252D0: sh      $v0, 0x5C($s1)
0x800252D4: sh      $zero, 0x5C($s1)
0x800252D8: lw      $v0, dword_800943C0
0x800252E0: lw      $v1, dword_8008ECF4
0x800252E8: lw      $v0, 4($v0)
0x800252EC: nop
0x800252F0: bne     $v1, $v0, loc_800258C8
0x800252F4: nop
0x800252F8: jal     sub_80024BF4
0x800252FC: move    $a0, $s1
0x80025300: bnez    $v0, loc_80025788
0x80025304: nop
0x80025308: lw      $v0, dword_8008ED08
0x80025310: nop
0x80025314: andi    $v0, 3
0x80025318: beqz    $v0, loc_80025330
0x8002531C: nop
0x80025320: jal     sub_80014D58
0x80025324: move    $a0, $s1
0x80025328: jal     sub_80014458
0x8002532C: move    $a0, $s1
0x80025330: lw      $v0, dword_8008ED08
0x80025338: nop
0x8002533C: andi    $v0, 2
0x80025340: beqz    $v0, loc_80025788
0x80025344: nop
0x80025348: lh      $v0, 0x4E($s1)
0x8002534C: nop
0x80025350: bnez    $v0, loc_800253A4
0x80025354: move    $s0, $zero
0x80025358: jal     sub_80014C80
0x8002535C: move    $a0, $s1
0x80025360: move    $v1, $v0
0x80025364: bnez    $v1, loc_80025374
0x80025368: li      $v0, 1
0x8002536C: j       loc_8002549C
0x80025370: li      $s0, 1
0x80025374: bne     $v1, $v0, loc_8002538C
0x80025378: li      $v0, 2
0x8002537C: li      $v0, 1
0x80025380: sh      $v0, 0x58($s1)
0x80025384: j       loc_800254A0
0x80025388: sh      $v0, 0x5A($s1)
0x8002538C: bne     $v1, $v0, loc_800254A0
0x80025390: li      $v0, 3
0x80025394: sh      $v0, 0x58($s1)
0x80025398: li      $v0, 1
0x8002539C: j       loc_800254A0
0x800253A0: sh      $v0, 0x5A($s1)
0x800253A4: jal     sub_80014D28
0x800253A8: move    $a0, $s1
0x800253AC: lh      $v1, word_8008ED38
0x800253B4: nop
0x800253B8: bnez    $v1, loc_80025448
0x800253BC: move    $s0, $v0
0x800253C0: lh      $v1, 0x4E($s1)
0x800253C4: li      $v0, 1
0x800253C8: bne     $v1, $v0, loc_800253E8
0x800253CC: move    $a0, $zero
0x800253D0: bne     $s0, $v1, loc_800253E8
0x800253D4: nop
0x800253D8: lh      $v0, word_8008ED34
0x800253E0: nop
0x800253E4: sltiu   $a0, $v0, 1
0x800253E8: bnez    $a0, loc_80025424
0x800253EC: li      $v0, 1
0x800253F0: lh      $v1, 0x4E($s1)
0x800253F4: li      $a0, dword_80055420
0x800253FC: sh      $v0, 0x5A($s1)
0x80025400: sll     $v0, $v1, 1
0x80025404: addu    $v0, $v1
0x80025408: sll     $v0, 2
0x8002540C: sll     $v1, $s0, 1
0x80025410: addu    $v0, $a0
0x80025414: addu    $v1, $v0
0x80025418: lhu     $v0, 0($v1)
0x8002541C: nop
0x80025420: sh      $v0, 0x58($s1)
0x80025424: lhu     $v0, word_8008ED38
0x8002542C: sh      $s0, word_8008ED36
0x80025434: addiu   $v0, 1
0x80025438: sh      $v0, word_8008ED38
0x80025440: j       loc_800254A0
0x80025444: nop
0x80025448: lh      $v1, 0x4E($s1)
0x8002544C: li      $v0, 3
0x80025450: bne     $v1, $v0, loc_80025474
0x80025454: move    $a0, $zero
0x80025458: lh      $v0, word_8008ED36
0x80025460: nop
0x80025464: bnez    $v0, loc_8002547C
0x80025468: nop
0x8002546C: sltiu   $v0, $s0, 1
0x80025470: move    $a0, $v0
0x80025474: lh      $v0, word_8008ED36
0x8002547C: sh      $a0, word_8009181E
0x80025484: beq     $v0, $s0, loc_80025494
0x80025488: li      $v0, 2
0x8002548C: sh      $v0, word_8008ED36
0x80025494: sh      $zero, word_8008ED38
0x8002549C: sh      $zero, 0x5A($s1)
0x800254A0: lw      $v1, dword_8008ED00
0x800254A8: li      $a0, 1
0x800254AC: beq     $v1, $a0, loc_80025744
0x800254B0: move    $s3, $s0
0x800254B4: li      $v0, 4
0x800254B8: beq     $v1, $v0, loc_80025744
0x800254BC: nop
0x800254C0: bnez    $v1, loc_8002576C
0x800254C4: li      $v0, 1
0x800254C8: lw      $v0, dword_8008ED08
0x800254D0: nop
0x800254D4: andi    $v0, 4
0x800254D8: beqz    $v0, loc_80025598
0x800254DC: nop
0x800254E0: lh      $s0, word_8008ED36
0x800254E8: nop
0x800254EC: bne     $s0, $a0, loc_8002551C
0x800254F0: nop
0x800254F4: lh      $v0, 0x4E($s1)
0x800254F8: nop
0x800254FC: bne     $v0, $s0, loc_80025514
0x80025500: nop
0x80025504: jal     sub_80014548
0x80025508: move    $a0, $s1
0x8002550C: bne     $v0, $s0, loc_80025544
0x80025510: li      $s2, 2
0x80025514: j       loc_80025544
0x80025518: li      $s2, 1
0x8002551C: bnez    $s0, loc_80025544
0x80025520: li      $s2, 2
0x80025524: lh      $v1, 0x4E($s1)
0x80025528: li      $v0, 3
0x8002552C: bne     $v1, $v0, loc_80025544
0x80025530: move    $s2, $zero
0x80025534: jal     sub_80014548
0x80025538: move    $a0, $s1
0x8002553C: sltu    $v0, $zero, $v0
0x80025540: sll     $s2, $v0, 1
0x80025544: jal     sub_80024F8C
0x80025548: move    $a0, $s1
0x8002554C: li      $v0, 2
0x80025550: beq     $s2, $v0, loc_80025598
0x80025554: li      $v0, 3
0x80025558: lh      $v1, 0x4E($s1)
0x8002555C: nop
0x80025560: bne     $v1, $v0, loc_80025570
0x80025564: nop
0x80025568: beqz    $s2, loc_80025598
0x8002556C: nop
0x80025570: li      $v0, 1
0x80025574: bne     $v1, $v0, loc_800255A0
0x80025578: nop
0x8002557C: bne     $s2, $v1, loc_800255A0
0x80025580: nop
0x80025584: lh      $v0, word_8009182A
0x8002558C: nop
0x80025590: bnez    $v0, loc_800255A4
0x80025594: sll     $a0, $s2, 1
0x80025598: j       loc_80025738
0x8002559C: move    $v1, $zero
0x800255A0: sll     $a0, $s2, 1
0x800255A4: addu    $a0, $s2
0x800255A8: lw      $v0, dword_800943FC
0x800255B0: sll     $a0, 1
0x800255B4: jal     sub_80026EF8
0x800255B8: addu    $a0, $v0, $a0
0x800255BC: lh      $v1, 0x4E($s1)
0x800255C0: li      $v0, 1
0x800255C4: bne     $v1, $v0, loc_80025664
0x800255C8: nop
0x800255CC: bne     $s2, $v1, loc_80025668
0x800255D0: li      $v0, 6
0x800255D4: lw      $v1, 0($s1)
0x800255D8: lhu     $v0, 0x72($s1)
0x800255DC: ori     $v1, 0x2000
0x800255E0: addiu   $v0, 1
0x800255E4: sh      $v0, 0x72($s1)
0x800255E8: sll     $v0, 16
0x800255EC: sra     $v0, 16
0x800255F0: slti    $v0, 2
0x800255F4: beqz    $v0, loc_8002560C
0x800255F8: sw      $v1, 0($s1)
0x800255FC: lw      $s0, dword_800943F4
0x80025604: j       loc_80025614
0x80025608: nop
0x8002560C: lw      $s0, dword_800943F8
0x80025614: jal     sub_80026EF8
0x80025618: move    $a0, $s0
0x8002561C: lh      $v1, 4($s0)
0x80025620: lw      $v0, 0xC($s1)
0x80025624: li      $a0, 1
0x80025628: sw      $a0, dword_8008ED14
0x80025630: addu    $v0, $v1
0x80025634: sw      $v0, dword_8008ED0C
0x8002563C: li      $v0, 1
0x80025640: sh      $v0, 0x50($s1)
0x80025644: li      $v0, 7
0x80025648: sh      $zero, 0x8A($s1)
0x8002564C: sw      $v0, dword_8008ED00
0x80025654: sw      $a0, dword_8008ED24
0x8002565C: j       loc_80025674
0x80025660: nop
0x80025664: li      $v0, 6
0x80025668: sw      $v0, dword_8008ED00
0x80025670: sh      $zero, 0x50($s1)
0x80025674: lh      $v1, 0x4E($s1)
0x80025678: li      $a0, 1
0x8002567C: bne     $v1, $a0, loc_800256BC
0x80025680: li      $v0, 2
0x80025684: bne     $s2, $a0, loc_800256A0
0x80025688: li      $v0, 2
0x8002568C: li      $v0, 5
0x80025690: sh      $zero, 0x4E($s1)
0x80025694: sh      $v0, 0x18E($s1)
0x80025698: j       loc_8002570C
0x8002569C: sh      $zero, 0x7A($s1)
0x800256A0: sh      $v0, 0x4E($s1)
0x800256A4: lhu     $v0, 0x74($s1)
0x800256A8: li      $v1, 1
0x800256AC: sh      $v1, 0x18E($s1)
0x800256B0: addiu   $v0, 1
0x800256B4: j       loc_8002570C
0x800256B8: sh      $v0, 0x74($s1)
0x800256BC: bne     $v1, $v0, loc_800256EC
0x800256C0: nop
0x800256C4: bne     $s2, $a0, loc_800256DC
0x800256C8: nop
0x800256CC: li      $v0, 1
0x800256D0: sh      $v0, 0x4E($s1)
0x800256D4: j       loc_80025708
0x800256D8: li      $v0, 4
0x800256DC: li      $v0, 3
0x800256E0: sh      $v0, 0x4E($s1)
0x800256E4: j       loc_80025708
0x800256E8: li      $v0, 2
0x800256EC: li      $v0, 3
0x800256F0: bne     $v1, $v0, loc_8002570C
0x800256F4: nop
0x800256F8: bne     $s2, $a0, loc_8002570C
0x800256FC: li      $v0, 2
0x80025700: sh      $v0, 0x4E($s1)
0x80025704: li      $v0, 3
0x80025708: sh      $v0, 0x18E($s1)
0x8002570C: lh      $v0, 0x4E($s1)
0x80025710: nop
0x80025714: bnez    $v0, loc_80025738
0x80025718: li      $v1, 1
0x8002571C: bnez    $s2, loc_8002573C
0x80025720: li      $v0, 1
0x80025724: li      $v0, 1
0x80025728: sh      $v0, 0x4E($s1)
0x8002572C: li      $v0, 6
0x80025730: sh      $v0, 0x18E($s1)
0x80025734: li      $v1, 1
0x80025738: li      $v0, 1
0x8002573C: beq     $v1, $v0, loc_80025788
0x80025740: nop
0x80025744: sll     $a0, $s3, 1
0x80025748: addu    $a0, $s3
0x8002574C: lw      $v0, dword_80094400
0x80025754: sll     $a0, 1
0x80025758: jal     sub_80026EF8
0x8002575C: addu    $a0, $v0, $a0
0x80025760: jal     sub_80014C1C
0x80025764: move    $a0, $s1
0x80025768: li      $v0, 1
0x8002576C: bne     $s3, $v0, loc_80025788
0x80025770: nop
0x80025774: lw      $v0, dword_80094438
0x8002577C: nop
0x80025780: jalr    $v0
0x80025784: move    $a0, $s1
0x80025788: li      $s2, dword_8008ED00
0x80025790: lw      $v0, (dword_8008ED00 - 0x8008ED00)($s2)
0x80025794: nop
0x80025798: beqz    $v0, loc_800258C0
0x8002579C: li      $v0, 1
0x800257A0: lw      $s0, dword_8008ED00
0x800257A8: nop
0x800257AC: bne     $s0, $v0, loc_800257F0
0x800257B0: nop
0x800257B4: jal     sub_80024BF4
0x800257B8: move    $a0, $s1
0x800257BC: beq     $v0, $s0, loc_800257F0
0x800257C0: nop
0x800257C4: lw      $v0, dword_8008ED08
0x800257CC: nop
0x800257D0: andi    $v0, 0x20
0x800257D4: beqz    $v0, loc_800257F0
0x800257D8: nop
0x800257DC: jal     sub_80014538
0x800257E0: move    $a0, $s1
0x800257E4: beqz    $v0, loc_800257F0
0x800257E8: li      $v0, 4
0x800257EC: sw      $v0, (dword_8008ED00 - 0x8008ED00)($s2)
0x800257F0: lw      $v1, dword_8008ED00
0x800257F8: li      $v0, 4
0x800257FC: bne     $v1, $v0, loc_80025860
0x80025800: li      $v0, 5
0x80025804: lw      $v0, dword_8008ED08
0x8002580C: nop
0x80025810: andi    $v0, 0x20
0x80025814: beqz    $v0, loc_80025860
0x80025818: li      $v0, 5
0x8002581C: lw      $a0, dword_800943F0
0x80025824: jal     sub_80026EF8
0x80025828: nop
0x8002582C: lw      $v0, 0($s1)
0x80025830: nop
0x80025834: ori     $v0, 0x4000
0x80025838: sw      $v0, 0($s1)
0x8002583C: li      $v0, 1
0x80025840: sw      $v0, dword_8008ED14
0x80025848: li      $v0, 5
0x8002584C: sw      $v0, dword_8008ED00
0x80025854: lw      $v1, dword_8008ED00
0x8002585C: li      $v0, 5
0x80025860: bne     $v1, $v0, loc_800258C0
0x80025864: nop
0x80025868: lw      $v0, dword_8008ED08
0x80025870: nop
0x80025874: andi    $v0, 0x40
0x80025878: beqz    $v0, loc_800258C0
0x8002587C: li      $v0, 8
0x80025880: sw      $v0, dword_8008ED00
0x80025888: li      $v0, 1
0x8002588C: li      $v1, 1
0x80025890: sw      $v0, g_PrStageEventStreamDone
0x80025898: li      $v0, 6
0x8002589C: sw      $zero, dword_8008ED14
0x800258A4: sh      $v1, 0x4E($s1)
0x800258A8: sh      $v0, 0x18E($s1)
0x800258AC: sh      $zero, 0x50($s1)
0x800258B0: sh      $v1, 0x54($s1)
0x800258B4: sh      $v1, 0x7A($s1)
0x800258B8: sh      $v1, word_8008ED36
0x800258C0: jal     sub_80024FC0
0x800258C4: move    $a0, $s1
0x800258C8: li      $s0, dword_8008ECF4
0x800258D0: lw      $v1, (dword_8008ECF4 - 0x8008ECF4)($s0)
0x800258D4: li      $v0, 0x1F
0x800258D8: bne     $v1, $v0, loc_800258EC
0x800258DC: nop
0x800258E0: lw      $a0, 0x38($s1)
0x800258E4: jal     sub_80014BDC
0x800258E8: nop
0x800258EC: lw      $v0, dword_800943C0
0x800258F4: lw      $v1, (dword_8008ECF4 - 0x8008ECF4)($s0)
0x800258F8: lw      $v0, 8($v0)
0x800258FC: nop
0x80025900: bne     $v1, $v0, loc_800259A0
0x80025904: move    $v0, $s1
0x80025908: lw      $v1, dword_8008ED00
0x80025910: nop
0x80025914: slti    $v0, $v1, 6
0x80025918: bnez    $v0, loc_800259A0
0x8002591C: move    $v0, $s1
0x80025920: li      $v0, 6
0x80025924: bne     $v1, $v0, loc_8002593C
0x80025928: li      $v0, 7
0x8002592C: sw      $zero, dword_8008ED00
0x80025934: j       loc_80025978
0x80025938: nop
0x8002593C: bne     $v1, $v0, loc_80025958
0x80025940: li      $v0, 8
0x80025944: li      $v0, 1
0x80025948: sw      $v0, dword_8008ED00
0x80025950: j       loc_80025978
0x80025954: nop
0x80025958: bne     $v1, $v0, loc_80025978
0x8002595C: nop
0x80025960: lw      $a0, dword_800943FC
0x80025968: sw      $zero, dword_8008ED00
0x80025970: jal     sub_80026EF8
0x80025974: nop
0x80025978: lw      $v0, 0($s1)
0x8002597C: nop
0x80025980: ori     $v0, 0x200
0x80025984: jal     sub_80014400
0x80025988: sw      $v0, 0($s1)
0x8002598C: jal     sub_80014C1C
0x80025990: move    $a0, $s1
0x80025994: jal     sub_80024F8C
0x80025998: move    $a0, $s1
0x8002599C: move    $v0, $s1
0x800259A0: lw      $ra, 0x10+var_s10($sp)
0x800259A4: lw      $s3, 0x10+var_sC($sp)
0x800259A8: lw      $s2, 0x10+var_s8($sp)
0x800259AC: lw      $s1, 0x10+var_s4($sp)
0x800259B0: lw      $s0, 0x10+var_s0($sp)
0x800259B4: addiu   $sp, 0x28
0x800259B8: jr      $ra
0x800259BC: nop
```

### Xrefs to

- `0x801C4C9C` -> `0x80024FD0` type=17 from `sub_801C4C9C`
- `0x801C4D14` -> `0x80024FD0` type=17 from `loc_801C4D14`

### Xrefs from

- `0x80024FD0` -> `0x80024FD4` type=21 to `sub_80024FD4`
- `0x80024FD4` -> `0x80024FD8` type=21 to `sub_80024FD8`
- `0x80024FD8` -> `0x80024FDC` type=21 to `sub_80024FDC`
- `0x80024FDC` -> `0x80024FE4` type=21 to `sub_80024FE4`
- `0x80024FDC` -> `0x8008ED20` type=3 to `dword_8008ED20`
- `0x80024FE4` -> `0x80024FE8` type=21 to `sub_80024FE8`
- `0x80024FE8` -> `0x80024FEC` type=21 to `sub_80024FEC`
- `0x80024FEC` -> `0x80024FF0` type=21 to `sub_80024FF0`
- `0x80024FF0` -> `0x80024FF4` type=21 to `sub_80024FF4`
- `0x80024FF4` -> `0x80024FF8` type=21 to `sub_80024FF8`
- `0x80024FF4` -> `0x8002599C` type=19 to `loc_8002599C`
- `0x80024FF8` -> `0x80024FFC` type=21 to `sub_80024FFC`
- `0x80024FFC` -> `0x80025000` type=21 to `sub_80025000`
- `0x80025000` -> `0x80025004` type=21 to `sub_80025004`
- `0x80025004` -> `0x80025008` type=21 to `sub_80025008`
- `0x80025008` -> `0x8002500C` type=21 to `sub_8002500C`
- `0x8002500C` -> `0x80025010` type=21 to `sub_80025010`
- `0x80025010` -> `0x80025014` type=21 to `sub_80025014`
- `0x80025014` -> `0x80025018` type=21 to `sub_80025018`
- `0x80025018` -> `0x8002501C` type=21 to `sub_8002501C`
- `0x8002501C` -> `0x80025020` type=21 to `sub_80025020`
- `0x80025020` -> `0x80025024` type=21 to `sub_80025024`
- `0x80025024` -> `0x80025028` type=21 to `sub_80025028`
- `0x80025028` -> `0x8002502C` type=21 to `sub_8002502C`
- `0x8002502C` -> `0x80025034` type=21 to `sub_80025034`
- `0x8002502C` -> `0x800916D0` type=3 to `word_800916D0`
- `0x80025034` -> `0x80025038` type=21 to `sub_80025038`
- `0x80025038` -> `0x8002503C` type=21 to `sub_8002503C`
- `0x80025038` -> `0x80025058` type=19 to `loc_80025058`
- `0x8002503C` -> `0x80025040` type=21 to `loc_80025040`
- `0x80025040` -> `0x80025044` type=21 to `sub_80025044`
- `0x80025044` -> `0x80025048` type=21 to `sub_80025048`
- `0x80025048` -> `0x8002504C` type=21 to `sub_8002504C`
- `0x80025048` -> `0x8002506C` type=19 to `loc_8002506C`
- `0x8002504C` -> `0x80025050` type=21 to `sub_80025050`
- `0x80025050` -> `0x80025054` type=21 to `sub_80025054`
- `0x80025050` -> `0x8002508C` type=19 to `loc_8002508C`
- `0x80025058` -> `0x80025040` type=19 to `loc_80025040`
- `0x80025058` -> `0x8002505C` type=21 to `sub_8002505C`
- `0x8002505C` -> `0x80025060` type=21 to `sub_80025060`
- `0x80025060` -> `0x80025064` type=21 to `sub_80025064`
- `0x80025064` -> `0x80025068` type=21 to `sub_80025068`
- `0x80025064` -> `0x80025088` type=19 to `loc_80025088`
- `0x80025068` -> `0x8002506C` type=21 to `loc_8002506C`
- `0x8002506C` -> `0x80025070` type=21 to `sub_80025070`
- `0x80025070` -> `0x80025074` type=21 to `sub_80025074`
- `0x80025074` -> `0x80025078` type=21 to `sub_80025078`
- `0x80025078` -> `0x8002507C` type=21 to `sub_8002507C`
- `0x8002507C` -> `0x80025080` type=21 to `sub_80025080`
- `0x80025080` -> `0x80025084` type=21 to `sub_80025084`
- `0x80025080` -> `0x8002508C` type=19 to `loc_8002508C`
- `0x80025088` -> `0x8002508C` type=21 to `loc_8002508C`
- `0x8002508C` -> `0x80025094` type=21 to `sub_80025094`
- `0x80025094` -> `0x80025098` type=21 to `sub_80025098`
- `0x80025098` -> `0x800250A0` type=21 to `sub_800250A0`
- `0x80025098` -> `0x8008ECE8` type=1 to `dword_8008ECE8`
- `0x800250A0` -> `0x800250A4` type=21 to `sub_800250A4`
- `0x800250A4` -> `0x800250A8` type=21 to `sub_800250A8`
- `0x800250A4` -> `0x8008ECE8` type=2 to `dword_8008ECE8`
- `0x800250A8` -> `0x800250B0` type=21 to `sub_800250B0`
- `0x800250A8` -> `0x8008ECF0` type=3 to `dword_8008ECF0`
- `0x800250B0` -> `0x800250B4` type=21 to `sub_800250B4`
- `0x800250B4` -> `0x800250B8` type=21 to `sub_800250B8`
- `0x800250B8` -> `0x800250BC` type=21 to `sub_800250BC`
- `0x800250BC` -> `0x800250C4` type=21 to `sub_800250C4`
- `0x800250BC` -> `0x8008ECF4` type=2 to `dword_8008ECF4`
- `0x800250C4` -> `0x800250C8` type=21 to `sub_800250C8`
- `0x800250C4` -> `0x800259A0` type=19 to `loc_800259A0`
- `0x800250C8` -> `0x800250CC` type=21 to `sub_800250CC`
- `0x800250CC` -> `0x800250D4` type=21 to `sub_800250D4`
- `0x800250CC` -> `0x800943C8` type=1 to `dword_800943C8`
- `0x800250D4` -> `0x800250D8` type=21 to `sub_800250D8`
- `0x800250D4` -> `0x800943C8` type=3 to `dword_800943C8`
- `0x800250D8` -> `0x800250E0` type=21 to `sub_800250E0`
- `0x800250D8` -> `0x8008ECF0` type=2 to `dword_8008ECF0`
- `0x800250E0` -> `0x800250E4` type=21 to `sub_800250E4`
- `0x800250E4` -> `0x800250E8` type=21 to `sub_800250E8`
- `0x800250E4` -> `0x80025104` type=19 to `loc_80025104`
- `0x800250E8` -> `0x800250EC` type=21 to `sub_800250EC`
- `0x800250EC` -> `0x800250F0` type=21 to `sub_800250F0`
- `0x800250F0` -> `0x800250F8` type=21 to `sub_800250F8`
- `0x800250F0` -> `0x800943C4` type=3 to `dword_800943C4`
- `0x800250F8` -> `0x800250FC` type=21 to `sub_800250FC`
- `0x800250FC` -> `0x80025100` type=21 to `sub_80025100`
- `0x80025100` -> `0x80025104` type=21 to `loc_80025104`
- `0x80025104` -> `0x80025108` type=21 to `sub_80025108`
- `0x80025108` -> `0x80025110` type=21 to `sub_80025110`
- `0x80025108` -> `0x8008ECF4` type=3 to `dword_8008ECF4`
- `0x80025110` -> `0x80025114` type=21 to `sub_80025114`
- `0x80025114` -> `0x80025118` type=21 to `sub_80025118`
- `0x80025114` -> `0x80025194` type=19 to `loc_80025194`
- `0x80025118` -> `0x8002511C` type=21 to `sub_8002511C`
- `0x8002511C` -> `0x80025120` type=21 to `sub_80025120`
- `0x80025120` -> `0x80025124` type=21 to `sub_80025124`
- `0x80025124` -> `0x80025128` type=21 to `sub_80025128`
- `0x80025128` -> `0x8002512C` type=21 to `sub_8002512C`
- `0x8002512C` -> `0x80025130` type=21 to `sub_80025130`
- `0x8002512C` -> `0x800943C8` type=3 to `dword_800943C8`
- `0x80025130` -> `0x80025134` type=21 to `sub_80025134`
- `0x80025134` -> `0x80025138` type=21 to `sub_80025138`
- `0x80025138` -> `0x8002513C` type=21 to `sub_8002513C`
- `0x80025138` -> `0x80025194` type=19 to `loc_80025194`
- `0x8002513C` -> `0x80025140` type=21 to `sub_80025140`
- `0x80025140` -> `0x80025144` type=21 to `sub_80025144`
- `0x80025144` -> `0x80025148` type=21 to `sub_80025148`
- `0x80025148` -> `0x80025150` type=21 to `sub_80025150`
- `0x80025148` -> `0x800943C4` type=3 to `dword_800943C4`
- `0x80025150` -> `0x80025154` type=21 to `sub_80025154`
- `0x80025154` -> `0x80025158` type=21 to `sub_80025158`
- `0x80025158` -> `0x8002515C` type=21 to `sub_8002515C`
- `0x8002515C` -> `0x80025160` type=21 to `sub_80025160`
- `0x80025160` -> `0x80025164` type=21 to `sub_80025164`
- `0x80025160` -> `0x8002518C` type=19 to `loc_8002518C`
- `0x80025164` -> `0x80025168` type=21 to `sub_80025168`
- `0x80025164` -> `0x8008ED08` type=1 to `dword_8008ED08`
- `0x80025168` -> `0x8002516C` type=21 to `sub_8002516C`
- `0x8002516C` -> `0x80025170` type=21 to `sub_80025170`
- `0x80025170` -> `0x80025174` type=21 to `sub_80025174`
- `0x80025174` -> `0x80025178` type=21 to `sub_80025178`
- `0x80025178` -> `0x8002517C` type=21 to `sub_8002517C`
- `0x8002517C` -> `0x80025180` type=21 to `sub_80025180`
- `0x80025180` -> `0x80025184` type=21 to `sub_80025184`
- `0x80025184` -> `0x80025188` type=21 to `sub_80025188`
- `0x80025184` -> `0x80025194` type=19 to `loc_80025194`
- `0x80025188` -> `0x8008ED08` type=2 to `dword_8008ED08`
- `0x8002518C` -> `0x80025190` type=21 to `sub_80025190`
- `0x80025190` -> `0x80025194` type=21 to `loc_80025194`
- `0x80025190` -> `0x8008ED08` type=2 to `dword_8008ED08`
- `0x80025194` -> `0x8002519C` type=21 to `sub_8002519C`
- `0x80025194` -> `0x8008ECF4` type=1 to `dword_8008ECF4`
- `0x8002519C` -> `0x800251A0` type=21 to `sub_800251A0`
- `0x8002519C` -> `0x8008ECF4` type=3 to `dword_8008ECF4`
- `0x800251A0` -> `0x800251A4` type=21 to `sub_800251A4`
- `0x800251A4` -> `0x800251A8` type=21 to `sub_800251A8`
- `0x800251A8` -> `0x800251AC` type=21 to `sub_800251AC`
- `0x800251A8` -> `0x800251D8` type=19 to `loc_800251D8`
- `0x800251AC` -> `0x800251B0` type=21 to `sub_800251B0`
- `0x800251B0` -> `0x800251B4` type=21 to `sub_800251B4`
- `0x800251B4` -> `0x800251B8` type=21 to `sub_800251B8`
- `0x800251B8` -> `0x800251BC` type=21 to `sub_800251BC`
- `0x800251BC` -> `0x800251C0` type=21 to `sub_800251C0`
- `0x800251C0` -> `0x800251C4` type=21 to `sub_800251C4`
- `0x800251C0` -> `0x8008ECF4` type=3 to `dword_8008ECF4`
- `0x800251C4` -> `0x800251C8` type=21 to `sub_800251C8`
- `0x800251C8` -> `0x800251CC` type=21 to `sub_800251CC`
- `0x800251CC` -> `0x800251D0` type=21 to `sub_800251D0`
- `0x800251CC` -> `0x800251D8` type=19 to `loc_800251D8`
- `0x800251D0` -> `0x800251D4` type=21 to `sub_800251D4`
- `0x800251D4` -> `0x800251D8` type=21 to `loc_800251D8`
- `0x800251D8` -> `0x800251E0` type=21 to `sub_800251E0`
- `0x800251D8` -> `0x800943C0` type=3 to `dword_800943C0`
- `0x800251E0` -> `0x800251E8` type=21 to `sub_800251E8`
- `0x800251E0` -> `0x8008ECF4` type=3 to `dword_8008ECF4`
- `0x800251E8` -> `0x800251EC` type=21 to `sub_800251EC`
- `0x800251EC` -> `0x800251F0` type=21 to `sub_800251F0`
- `0x800251F0` -> `0x800251F4` type=21 to `sub_800251F4`
- `0x800251F0` -> `0x800252D8` type=19 to `loc_800252D8`
- `0x800251F4` -> `0x800251F8` type=21 to `sub_800251F8`
- `0x800251F8` -> `0x80024BF4` type=17 to `sub_80024BF4`
- `0x800251F8` -> `0x800251FC` type=21 to `sub_800251FC`
- `0x800251FC` -> `0x80025200` type=21 to `sub_80025200`
- `0x80025200` -> `0x80025204` type=21 to `sub_80025204`
- `0x80025204` -> `0x80025208` type=21 to `sub_80025208`
- `0x80025204` -> `0x80025280` type=19 to `loc_80025280`
- `0x80025208` -> `0x8002520C` type=21 to `sub_8002520C`
- `0x8002520C` -> `0x80025214` type=21 to `sub_80025214`
- `0x8002520C` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x80025214` -> `0x80025218` type=21 to `sub_80025218`
- `0x80025218` -> `0x8002521C` type=21 to `sub_8002521C`
- `0x8002521C` -> `0x80025220` type=21 to `sub_80025220`
- `0x8002521C` -> `0x8002523C` type=19 to `loc_8002523C`
- `0x80025220` -> `0x80025224` type=21 to `sub_80025224`
- `0x80025224` -> `0x80025228` type=21 to `sub_80025228`
- `0x80025228` -> `0x8002522C` type=21 to `sub_8002522C`
- `0x8002522C` -> `0x80025230` type=21 to `sub_80025230`
- `0x80025230` -> `0x80025234` type=21 to `sub_80025234`
- `0x80025234` -> `0x80025238` type=21 to `sub_80025238`
- `0x80025234` -> `0x80025268` type=19 to `loc_80025268`
- `0x80025238` -> `0x8002523C` type=21 to `loc_8002523C`
- `0x8002523C` -> `0x80025244` type=21 to `sub_80025244`
- `0x8002523C` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x80025244` -> `0x80025248` type=21 to `sub_80025248`
- `0x80025248` -> `0x8002524C` type=21 to `sub_8002524C`
- `0x8002524C` -> `0x80025250` type=21 to `sub_80025250`
- `0x8002524C` -> `0x80025280` type=19 to `loc_80025280`
- `0x80025250` -> `0x80025254` type=21 to `sub_80025254`
- `0x80025254` -> `0x800144B8` type=17 to `sub_800144B8`
- `0x80025254` -> `0x80025258` type=21 to `sub_80025258`
- `0x80025258` -> `0x8002525C` type=21 to `sub_8002525C`
- `0x8002525C` -> `0x80025260` type=21 to `sub_80025260`
- `0x80025260` -> `0x80025264` type=21 to `sub_80025264`
- `0x80025260` -> `0x80025280` type=19 to `loc_80025280`
- `0x80025264` -> `0x80025268` type=21 to `loc_80025268`
- `0x80025268` -> `0x8002526C` type=21 to `sub_8002526C`
- `0x8002526C` -> `0x80025270` type=21 to `sub_80025270`
- `0x80025270` -> `0x80025274` type=21 to `sub_80025274`
- `0x80025274` -> `0x80025278` type=21 to `sub_80025278`
- `0x80025278` -> `0x8002527C` type=21 to `sub_8002527C`
- `0x8002527C` -> `0x80025280` type=21 to `loc_80025280`
- `0x80025280` -> `0x80025288` type=21 to `sub_80025288`
- `0x80025280` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x80025288` -> `0x8002528C` type=21 to `sub_8002528C`
- `0x8002528C` -> `0x80025290` type=21 to `sub_80025290`
- `0x80025290` -> `0x80025294` type=21 to `sub_80025294`
- `0x80025290` -> `0x800252A0` type=19 to `loc_800252A0`
- `0x80025294` -> `0x80025298` type=21 to `sub_80025298`
- `0x80025298` -> `0x800152D0` type=17 to `sub_800152D0`
- `0x80025298` -> `0x8002529C` type=21 to `sub_8002529C`
- `0x8002529C` -> `0x800252A0` type=21 to `loc_800252A0`
- `0x800252A0` -> `0x800252A4` type=21 to `sub_800252A4`
- `0x800252A4` -> `0x800252A8` type=21 to `sub_800252A8`
- `0x800252A8` -> `0x800252AC` type=21 to `sub_800252AC`
- `0x800252A8` -> `0x800252D4` type=19 to `loc_800252D4`
- `0x800252AC` -> `0x800252B0` type=21 to `sub_800252B0`
- `0x800252B0` -> `0x800252B4` type=21 to `sub_800252B4`
- `0x800252B0` -> `0x800252D4` type=19 to `loc_800252D4`
- `0x800252B4` -> `0x800252B8` type=21 to `sub_800252B8`
- `0x800252B8` -> `0x800252BC` type=21 to `sub_800252BC`
- `0x800252BC` -> `0x800252C0` type=21 to `sub_800252C0`
- `0x800252C0` -> `0x800252C4` type=21 to `sub_800252C4`
- `0x800252C0` -> `0x800252D8` type=19 to `loc_800252D8`
- `0x800252C4` -> `0x800252C8` type=21 to `sub_800252C8`
- `0x800252C8` -> `0x800252CC` type=21 to `sub_800252CC`
- `0x800252CC` -> `0x800252D0` type=21 to `sub_800252D0`
- `0x800252CC` -> `0x800252D8` type=19 to `loc_800252D8`
- `0x800252D4` -> `0x800252D8` type=21 to `loc_800252D8`
- `0x800252D8` -> `0x800252E0` type=21 to `sub_800252E0`
- `0x800252D8` -> `0x800943C0` type=3 to `dword_800943C0`
- `0x800252E0` -> `0x800252E8` type=21 to `sub_800252E8`
- `0x800252E0` -> `0x8008ECF4` type=3 to `dword_8008ECF4`
- `0x800252E8` -> `0x800252EC` type=21 to `sub_800252EC`
- `0x800252EC` -> `0x800252F0` type=21 to `sub_800252F0`
- `0x800252F0` -> `0x800252F4` type=21 to `sub_800252F4`
- `0x800252F0` -> `0x800258C8` type=19 to `loc_800258C8`
- `0x800252F4` -> `0x800252F8` type=21 to `sub_800252F8`
- `0x800252F8` -> `0x80024BF4` type=17 to `sub_80024BF4`
- `0x800252F8` -> `0x800252FC` type=21 to `sub_800252FC`
- `0x800252FC` -> `0x80025300` type=21 to `sub_80025300`
- `0x80025300` -> `0x80025304` type=21 to `sub_80025304`
- `0x80025300` -> `0x80025788` type=19 to `loc_80025788`
- `0x80025304` -> `0x80025308` type=21 to `sub_80025308`
- `0x80025308` -> `0x80025310` type=21 to `sub_80025310`
- `0x80025308` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x80025310` -> `0x80025314` type=21 to `sub_80025314`
- `0x80025314` -> `0x80025318` type=21 to `sub_80025318`
- `0x80025318` -> `0x8002531C` type=21 to `sub_8002531C`
- `0x80025318` -> `0x80025330` type=19 to `loc_80025330`
- `0x8002531C` -> `0x80025320` type=21 to `sub_80025320`
- `0x80025320` -> `0x80014D58` type=17 to `sub_80014D58`
- `0x80025320` -> `0x80025324` type=21 to `sub_80025324`
- `0x80025324` -> `0x80025328` type=21 to `sub_80025328`
- `0x80025328` -> `0x80014458` type=17 to `sub_80014458`
- `0x80025328` -> `0x8002532C` type=21 to `sub_8002532C`
- `0x8002532C` -> `0x80025330` type=21 to `loc_80025330`
- `0x80025330` -> `0x80025338` type=21 to `sub_80025338`
- `0x80025330` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x80025338` -> `0x8002533C` type=21 to `sub_8002533C`
- `0x8002533C` -> `0x80025340` type=21 to `sub_80025340`
- `0x80025340` -> `0x80025344` type=21 to `sub_80025344`
- `0x80025340` -> `0x80025788` type=19 to `loc_80025788`
- `0x80025344` -> `0x80025348` type=21 to `sub_80025348`
- `0x80025348` -> `0x8002534C` type=21 to `sub_8002534C`
- `0x8002534C` -> `0x80025350` type=21 to `sub_80025350`
- `0x80025350` -> `0x80025354` type=21 to `sub_80025354`
- `0x80025350` -> `0x800253A4` type=19 to `loc_800253A4`
- `0x80025354` -> `0x80025358` type=21 to `sub_80025358`
- `0x80025358` -> `0x80014C80` type=17 to `sub_80014C80`
- `0x80025358` -> `0x8002535C` type=21 to `sub_8002535C`
- `0x8002535C` -> `0x80025360` type=21 to `sub_80025360`
- `0x80025360` -> `0x80025364` type=21 to `sub_80025364`
- `0x80025364` -> `0x80025368` type=21 to `sub_80025368`
- `0x80025364` -> `0x80025374` type=19 to `loc_80025374`
- `0x80025368` -> `0x8002536C` type=21 to `sub_8002536C`
- `0x8002536C` -> `0x80025370` type=21 to `sub_80025370`
- `0x8002536C` -> `0x8002549C` type=19 to `loc_8002549C`
- `0x80025374` -> `0x80025378` type=21 to `sub_80025378`
- `0x80025374` -> `0x8002538C` type=19 to `loc_8002538C`
- `0x80025378` -> `0x8002537C` type=21 to `sub_8002537C`
- `0x8002537C` -> `0x80025380` type=21 to `sub_80025380`
- `0x80025380` -> `0x80025384` type=21 to `sub_80025384`
- `0x80025384` -> `0x80025388` type=21 to `sub_80025388`
- `0x80025384` -> `0x800254A0` type=19 to `loc_800254A0`
- `0x8002538C` -> `0x80025390` type=21 to `sub_80025390`
- `0x8002538C` -> `0x800254A0` type=19 to `loc_800254A0`
- `0x80025390` -> `0x80025394` type=21 to `sub_80025394`
- `0x80025394` -> `0x80025398` type=21 to `sub_80025398`
- `0x80025398` -> `0x8002539C` type=21 to `sub_8002539C`
- `0x8002539C` -> `0x800253A0` type=21 to `sub_800253A0`
- `0x8002539C` -> `0x800254A0` type=19 to `loc_800254A0`
- `0x800253A4` -> `0x80014D28` type=17 to `sub_80014D28`
- `0x800253A4` -> `0x800253A8` type=21 to `sub_800253A8`
- `0x800253A8` -> `0x800253AC` type=21 to `sub_800253AC`
- `0x800253AC` -> `0x800253B4` type=21 to `sub_800253B4`
- `0x800253AC` -> `0x8008ED38` type=3 to `word_8008ED38`
- `0x800253B4` -> `0x800253B8` type=21 to `sub_800253B8`
- `0x800253B8` -> `0x800253BC` type=21 to `sub_800253BC`
- `0x800253B8` -> `0x80025448` type=19 to `loc_80025448`
- `0x800253BC` -> `0x800253C0` type=21 to `sub_800253C0`
- `0x800253C0` -> `0x800253C4` type=21 to `sub_800253C4`
- `0x800253C4` -> `0x800253C8` type=21 to `sub_800253C8`
- `0x800253C8` -> `0x800253CC` type=21 to `sub_800253CC`
- `0x800253C8` -> `0x800253E8` type=19 to `loc_800253E8`
- `0x800253CC` -> `0x800253D0` type=21 to `sub_800253D0`
- `0x800253D0` -> `0x800253D4` type=21 to `sub_800253D4`
- `0x800253D0` -> `0x800253E8` type=19 to `loc_800253E8`
- `0x800253D4` -> `0x800253D8` type=21 to `sub_800253D8`
- `0x800253D8` -> `0x800253E0` type=21 to `sub_800253E0`
- `0x800253D8` -> `0x8008ED34` type=3 to `word_8008ED34`
- `0x800253E0` -> `0x800253E4` type=21 to `sub_800253E4`
- `0x800253E4` -> `0x800253E8` type=21 to `loc_800253E8`
- `0x800253E8` -> `0x800253EC` type=21 to `sub_800253EC`
- `0x800253E8` -> `0x80025424` type=19 to `loc_80025424`
- `0x800253EC` -> `0x800253F0` type=21 to `sub_800253F0`
- `0x800253F0` -> `0x800253F4` type=21 to `sub_800253F4`
- `0x800253F4` -> `0x800253FC` type=21 to `sub_800253FC`
- `0x800253F4` -> `0x80055420` type=1 to `dword_80055420`
- `0x800253FC` -> `0x80025400` type=21 to `sub_80025400`
- `0x80025400` -> `0x80025404` type=21 to `sub_80025404`
- `0x80025404` -> `0x80025408` type=21 to `sub_80025408`
- `0x80025408` -> `0x8002540C` type=21 to `sub_8002540C`
- `0x8002540C` -> `0x80025410` type=21 to `sub_80025410`
- `0x80025410` -> `0x80025414` type=21 to `sub_80025414`
- `0x80025414` -> `0x80025418` type=21 to `sub_80025418`
- `0x80025418` -> `0x8002541C` type=21 to `sub_8002541C`
- `0x8002541C` -> `0x80025420` type=21 to `sub_80025420`
- `0x80025420` -> `0x80025424` type=21 to `loc_80025424`
- `0x80025424` -> `0x8002542C` type=21 to `sub_8002542C`
- `0x80025424` -> `0x8008ED38` type=3 to `word_8008ED38`
- `0x8002542C` -> `0x80025434` type=21 to `sub_80025434`
- `0x8002542C` -> `0x8008ED36` type=2 to `word_8008ED36`
- `0x80025434` -> `0x80025438` type=21 to `sub_80025438`
- `0x80025438` -> `0x80025440` type=21 to `sub_80025440`
- `0x80025438` -> `0x8008ED38` type=2 to `word_8008ED38`
- `0x80025440` -> `0x80025444` type=21 to `sub_80025444`
- `0x80025440` -> `0x800254A0` type=19 to `loc_800254A0`
- `0x80025448` -> `0x8002544C` type=21 to `sub_8002544C`
- `0x8002544C` -> `0x80025450` type=21 to `sub_80025450`
- `0x80025450` -> `0x80025454` type=21 to `sub_80025454`
- `0x80025450` -> `0x80025474` type=19 to `loc_80025474`
- `0x80025454` -> `0x80025458` type=21 to `sub_80025458`
- `0x80025458` -> `0x80025460` type=21 to `sub_80025460`
- `0x80025458` -> `0x8008ED36` type=3 to `word_8008ED36`
- `0x80025460` -> `0x80025464` type=21 to `sub_80025464`
- `0x80025464` -> `0x80025468` type=21 to `sub_80025468`
- `0x80025464` -> `0x8002547C` type=19 to `loc_8002547C`
- `0x80025468` -> `0x8002546C` type=21 to `sub_8002546C`
- `0x8002546C` -> `0x80025470` type=21 to `sub_80025470`
- `0x80025470` -> `0x80025474` type=21 to `loc_80025474`
- `0x80025474` -> `0x8002547C` type=21 to `loc_8002547C`
- `0x80025474` -> `0x8008ED36` type=3 to `word_8008ED36`
- `0x8002547C` -> `0x80025484` type=21 to `sub_80025484`
- `0x8002547C` -> `0x8009181E` type=2 to `word_8009181E`
- `0x80025484` -> `0x80025488` type=21 to `sub_80025488`
- `0x80025484` -> `0x80025494` type=19 to `loc_80025494`
- `0x80025488` -> `0x8002548C` type=21 to `sub_8002548C`
- `0x8002548C` -> `0x80025494` type=21 to `loc_80025494`
- `0x8002548C` -> `0x8008ED36` type=2 to `word_8008ED36`
- `0x80025494` -> `0x8002549C` type=21 to `loc_8002549C`
- `0x80025494` -> `0x8008ED38` type=2 to `word_8008ED38`
- `0x8002549C` -> `0x800254A0` type=21 to `loc_800254A0`
- `0x800254A0` -> `0x800254A8` type=21 to `sub_800254A8`
- `0x800254A0` -> `0x8008ED00` type=3 to `dword_8008ED00`
- `0x800254A8` -> `0x800254AC` type=21 to `sub_800254AC`
- `0x800254AC` -> `0x800254B0` type=21 to `sub_800254B0`
- `0x800254AC` -> `0x80025744` type=19 to `loc_80025744`
- `0x800254B0` -> `0x800254B4` type=21 to `sub_800254B4`
- `0x800254B4` -> `0x800254B8` type=21 to `sub_800254B8`
- `0x800254B8` -> `0x800254BC` type=21 to `sub_800254BC`
- `0x800254B8` -> `0x80025744` type=19 to `loc_80025744`
- `0x800254BC` -> `0x800254C0` type=21 to `sub_800254C0`
- `0x800254C0` -> `0x800254C4` type=21 to `sub_800254C4`
- `0x800254C0` -> `0x8002576C` type=19 to `loc_8002576C`
- `0x800254C4` -> `0x800254C8` type=21 to `sub_800254C8`
- `0x800254C8` -> `0x800254D0` type=21 to `sub_800254D0`
- `0x800254C8` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x800254D0` -> `0x800254D4` type=21 to `sub_800254D4`
- `0x800254D4` -> `0x800254D8` type=21 to `sub_800254D8`
- `0x800254D8` -> `0x800254DC` type=21 to `sub_800254DC`
- `0x800254D8` -> `0x80025598` type=19 to `loc_80025598`
- `0x800254DC` -> `0x800254E0` type=21 to `sub_800254E0`
- `0x800254E0` -> `0x800254E8` type=21 to `sub_800254E8`
- `0x800254E0` -> `0x8008ED36` type=3 to `word_8008ED36`
- `0x800254E8` -> `0x800254EC` type=21 to `sub_800254EC`
- `0x800254EC` -> `0x800254F0` type=21 to `sub_800254F0`
- `0x800254EC` -> `0x8002551C` type=19 to `loc_8002551C`
- `0x800254F0` -> `0x800254F4` type=21 to `sub_800254F4`
- `0x800254F4` -> `0x800254F8` type=21 to `sub_800254F8`
- `0x800254F8` -> `0x800254FC` type=21 to `sub_800254FC`
- `0x800254FC` -> `0x80025500` type=21 to `sub_80025500`
- `0x800254FC` -> `0x80025514` type=19 to `loc_80025514`
- `0x80025500` -> `0x80025504` type=21 to `sub_80025504`
- `0x80025504` -> `0x80014548` type=17 to `sub_80014548`
- `0x80025504` -> `0x80025508` type=21 to `sub_80025508`
- `0x80025508` -> `0x8002550C` type=21 to `sub_8002550C`
- `0x8002550C` -> `0x80025510` type=21 to `sub_80025510`
- `0x8002550C` -> `0x80025544` type=19 to `loc_80025544`
- `0x80025510` -> `0x80025514` type=21 to `loc_80025514`
- `0x80025514` -> `0x80025518` type=21 to `sub_80025518`
- `0x80025514` -> `0x80025544` type=19 to `loc_80025544`
- `0x8002551C` -> `0x80025520` type=21 to `sub_80025520`
- `0x8002551C` -> `0x80025544` type=19 to `loc_80025544`
- `0x80025520` -> `0x80025524` type=21 to `sub_80025524`
- `0x80025524` -> `0x80025528` type=21 to `sub_80025528`
- `0x80025528` -> `0x8002552C` type=21 to `sub_8002552C`
- `0x8002552C` -> `0x80025530` type=21 to `sub_80025530`
- `0x8002552C` -> `0x80025544` type=19 to `loc_80025544`
- `0x80025530` -> `0x80025534` type=21 to `sub_80025534`
- `0x80025534` -> `0x80014548` type=17 to `sub_80014548`
- `0x80025534` -> `0x80025538` type=21 to `sub_80025538`
- `0x80025538` -> `0x8002553C` type=21 to `sub_8002553C`
- `0x8002553C` -> `0x80025540` type=21 to `sub_80025540`
- `0x80025540` -> `0x80025544` type=21 to `loc_80025544`
- `0x80025544` -> `0x80024F8C` type=17 to `sub_80024F8C`
- `0x80025544` -> `0x80025548` type=21 to `sub_80025548`
- `0x80025548` -> `0x8002554C` type=21 to `sub_8002554C`
- `0x8002554C` -> `0x80025550` type=21 to `sub_80025550`
- `0x80025550` -> `0x80025554` type=21 to `sub_80025554`
- `0x80025550` -> `0x80025598` type=19 to `loc_80025598`
- `0x80025554` -> `0x80025558` type=21 to `sub_80025558`
- `0x80025558` -> `0x8002555C` type=21 to `sub_8002555C`
- `0x8002555C` -> `0x80025560` type=21 to `sub_80025560`
- `0x80025560` -> `0x80025564` type=21 to `sub_80025564`
- `0x80025560` -> `0x80025570` type=19 to `loc_80025570`
- `0x80025564` -> `0x80025568` type=21 to `sub_80025568`
- `0x80025568` -> `0x8002556C` type=21 to `sub_8002556C`
- `0x80025568` -> `0x80025598` type=19 to `loc_80025598`
- `0x8002556C` -> `0x80025570` type=21 to `loc_80025570`
- `0x80025570` -> `0x80025574` type=21 to `sub_80025574`
- `0x80025574` -> `0x80025578` type=21 to `sub_80025578`
- `0x80025574` -> `0x800255A0` type=19 to `loc_800255A0`
- `0x80025578` -> `0x8002557C` type=21 to `sub_8002557C`
- `0x8002557C` -> `0x80025580` type=21 to `sub_80025580`
- `0x8002557C` -> `0x800255A0` type=19 to `loc_800255A0`
- `0x80025580` -> `0x80025584` type=21 to `sub_80025584`
- `0x80025584` -> `0x8002558C` type=21 to `sub_8002558C`
- `0x80025584` -> `0x8009182A` type=3 to `word_8009182A`
- `0x8002558C` -> `0x80025590` type=21 to `sub_80025590`
- `0x80025590` -> `0x80025594` type=21 to `sub_80025594`
- `0x80025590` -> `0x800255A4` type=19 to `loc_800255A4`
- `0x80025594` -> `0x80025598` type=21 to `loc_80025598`
- `0x80025598` -> `0x8002559C` type=21 to `sub_8002559C`
- `0x80025598` -> `0x80025738` type=19 to `loc_80025738`
- `0x800255A0` -> `0x800255A4` type=21 to `loc_800255A4`
- `0x800255A4` -> `0x800255A8` type=21 to `sub_800255A8`
- `0x800255A8` -> `0x800255B0` type=21 to `sub_800255B0`
- `0x800255A8` -> `0x800943FC` type=3 to `dword_800943FC`
- `0x800255B0` -> `0x800255B4` type=21 to `sub_800255B4`
- `0x800255B4` -> `0x800255B8` type=21 to `sub_800255B8`
- `0x800255B4` -> `0x80026EF8` type=17 to `sub_80026EF8`
- `0x800255B8` -> `0x800255BC` type=21 to `sub_800255BC`
- `0x800255BC` -> `0x800255C0` type=21 to `sub_800255C0`
- `0x800255C0` -> `0x800255C4` type=21 to `sub_800255C4`
- `0x800255C4` -> `0x800255C8` type=21 to `sub_800255C8`
- `0x800255C4` -> `0x80025664` type=19 to `loc_80025664`
- `0x800255C8` -> `0x800255CC` type=21 to `sub_800255CC`
- `0x800255CC` -> `0x800255D0` type=21 to `sub_800255D0`
- `0x800255CC` -> `0x80025668` type=19 to `loc_80025668`
- `0x800255D0` -> `0x800255D4` type=21 to `sub_800255D4`
- `0x800255D4` -> `0x800255D8` type=21 to `sub_800255D8`
- `0x800255D8` -> `0x800255DC` type=21 to `sub_800255DC`
- `0x800255DC` -> `0x800255E0` type=21 to `sub_800255E0`
- `0x800255E0` -> `0x800255E4` type=21 to `sub_800255E4`
- `0x800255E4` -> `0x800255E8` type=21 to `sub_800255E8`
- `0x800255E8` -> `0x800255EC` type=21 to `sub_800255EC`
- `0x800255EC` -> `0x800255F0` type=21 to `sub_800255F0`
- `0x800255F0` -> `0x800255F4` type=21 to `sub_800255F4`
- `0x800255F4` -> `0x800255F8` type=21 to `sub_800255F8`
- `0x800255F4` -> `0x8002560C` type=19 to `loc_8002560C`
- `0x800255F8` -> `0x800255FC` type=21 to `sub_800255FC`
- `0x800255FC` -> `0x80025604` type=21 to `sub_80025604`
- `0x800255FC` -> `0x800943F4` type=3 to `dword_800943F4`
- `0x80025604` -> `0x80025608` type=21 to `sub_80025608`
- `0x80025604` -> `0x80025614` type=19 to `loc_80025614`
- `0x8002560C` -> `0x80025614` type=21 to `loc_80025614`
- `0x8002560C` -> `0x800943F8` type=3 to `dword_800943F8`
- `0x80025614` -> `0x80025618` type=21 to `sub_80025618`
- `0x80025614` -> `0x80026EF8` type=17 to `sub_80026EF8`
- `0x80025618` -> `0x8002561C` type=21 to `sub_8002561C`
- `0x8002561C` -> `0x80025620` type=21 to `sub_80025620`
- `0x80025620` -> `0x80025624` type=21 to `sub_80025624`
- `0x80025624` -> `0x80025628` type=21 to `sub_80025628`
- `0x80025628` -> `0x80025630` type=21 to `sub_80025630`
- `0x80025628` -> `0x8008ED14` type=2 to `dword_8008ED14`
- `0x80025630` -> `0x80025634` type=21 to `sub_80025634`
- `0x80025634` -> `0x8002563C` type=21 to `sub_8002563C`
- `0x80025634` -> `0x8008ED0C` type=2 to `dword_8008ED0C`
- `0x8002563C` -> `0x80025640` type=21 to `sub_80025640`
- `0x80025640` -> `0x80025644` type=21 to `sub_80025644`
- `0x80025644` -> `0x80025648` type=21 to `sub_80025648`
- `0x80025648` -> `0x8002564C` type=21 to `sub_8002564C`
- `0x8002564C` -> `0x80025654` type=21 to `sub_80025654`
- `0x8002564C` -> `0x8008ED00` type=2 to `dword_8008ED00`
- `0x80025654` -> `0x8002565C` type=21 to `sub_8002565C`
- `0x80025654` -> `0x8008ED24` type=2 to `dword_8008ED24`
- `0x8002565C` -> `0x80025660` type=21 to `sub_80025660`
- `0x8002565C` -> `0x80025674` type=19 to `loc_80025674`
- `0x80025664` -> `0x80025668` type=21 to `loc_80025668`
- `0x80025668` -> `0x80025670` type=21 to `sub_80025670`
- `0x80025668` -> `0x8008ED00` type=2 to `dword_8008ED00`
- `0x80025670` -> `0x80025674` type=21 to `loc_80025674`
- `0x80025674` -> `0x80025678` type=21 to `sub_80025678`
- `0x80025678` -> `0x8002567C` type=21 to `sub_8002567C`
- `0x8002567C` -> `0x80025680` type=21 to `sub_80025680`
- `0x8002567C` -> `0x800256BC` type=19 to `loc_800256BC`
- `0x80025680` -> `0x80025684` type=21 to `sub_80025684`
- `0x80025684` -> `0x80025688` type=21 to `sub_80025688`
- `0x80025684` -> `0x800256A0` type=19 to `loc_800256A0`
- `0x80025688` -> `0x8002568C` type=21 to `sub_8002568C`
- `0x8002568C` -> `0x80025690` type=21 to `sub_80025690`
- `0x80025690` -> `0x80025694` type=21 to `sub_80025694`
- `0x80025694` -> `0x80025698` type=21 to `sub_80025698`
- `0x80025698` -> `0x8002569C` type=21 to `sub_8002569C`
- `0x80025698` -> `0x8002570C` type=19 to `loc_8002570C`
- `0x800256A0` -> `0x800256A4` type=21 to `sub_800256A4`
- `0x800256A4` -> `0x800256A8` type=21 to `sub_800256A8`
- `0x800256A8` -> `0x800256AC` type=21 to `sub_800256AC`
- `0x800256AC` -> `0x800256B0` type=21 to `sub_800256B0`
- `0x800256B0` -> `0x800256B4` type=21 to `sub_800256B4`
- `0x800256B4` -> `0x800256B8` type=21 to `sub_800256B8`
- `0x800256B4` -> `0x8002570C` type=19 to `loc_8002570C`
- `0x800256BC` -> `0x800256C0` type=21 to `sub_800256C0`
- `0x800256BC` -> `0x800256EC` type=19 to `loc_800256EC`
- `0x800256C0` -> `0x800256C4` type=21 to `sub_800256C4`
- `0x800256C4` -> `0x800256C8` type=21 to `sub_800256C8`
- `0x800256C4` -> `0x800256DC` type=19 to `loc_800256DC`
- `0x800256C8` -> `0x800256CC` type=21 to `sub_800256CC`
- `0x800256CC` -> `0x800256D0` type=21 to `sub_800256D0`
- `0x800256D0` -> `0x800256D4` type=21 to `sub_800256D4`
- `0x800256D4` -> `0x800256D8` type=21 to `sub_800256D8`
- `0x800256D4` -> `0x80025708` type=19 to `loc_80025708`
- `0x800256DC` -> `0x800256E0` type=21 to `sub_800256E0`
- `0x800256E0` -> `0x800256E4` type=21 to `sub_800256E4`
- `0x800256E4` -> `0x800256E8` type=21 to `sub_800256E8`
- `0x800256E4` -> `0x80025708` type=19 to `loc_80025708`
- `0x800256EC` -> `0x800256F0` type=21 to `sub_800256F0`
- `0x800256F0` -> `0x800256F4` type=21 to `sub_800256F4`
- `0x800256F0` -> `0x8002570C` type=19 to `loc_8002570C`
- `0x800256F4` -> `0x800256F8` type=21 to `sub_800256F8`
- `0x800256F8` -> `0x800256FC` type=21 to `sub_800256FC`
- `0x800256F8` -> `0x8002570C` type=19 to `loc_8002570C`
- `0x800256FC` -> `0x80025700` type=21 to `sub_80025700`
- `0x80025700` -> `0x80025704` type=21 to `sub_80025704`
- `0x80025704` -> `0x80025708` type=21 to `loc_80025708`
- `0x80025708` -> `0x8002570C` type=21 to `loc_8002570C`
- `0x8002570C` -> `0x80025710` type=21 to `sub_80025710`
- `0x80025710` -> `0x80025714` type=21 to `sub_80025714`
- `0x80025714` -> `0x80025718` type=21 to `sub_80025718`
- `0x80025714` -> `0x80025738` type=19 to `loc_80025738`
- `0x80025718` -> `0x8002571C` type=21 to `sub_8002571C`
- `0x8002571C` -> `0x80025720` type=21 to `sub_80025720`
- `0x8002571C` -> `0x8002573C` type=19 to `loc_8002573C`
- `0x80025720` -> `0x80025724` type=21 to `sub_80025724`
- `0x80025724` -> `0x80025728` type=21 to `sub_80025728`
- `0x80025728` -> `0x8002572C` type=21 to `sub_8002572C`
- `0x8002572C` -> `0x80025730` type=21 to `sub_80025730`
- `0x80025730` -> `0x80025734` type=21 to `sub_80025734`
- `0x80025734` -> `0x80025738` type=21 to `loc_80025738`
- `0x80025738` -> `0x8002573C` type=21 to `loc_8002573C`
- `0x8002573C` -> `0x80025740` type=21 to `sub_80025740`
- `0x8002573C` -> `0x80025788` type=19 to `loc_80025788`
- `0x80025740` -> `0x80025744` type=21 to `loc_80025744`
- `0x80025744` -> `0x80025748` type=21 to `sub_80025748`
- `0x80025748` -> `0x8002574C` type=21 to `sub_8002574C`
- `0x8002574C` -> `0x80025754` type=21 to `sub_80025754`
- `0x8002574C` -> `0x80094400` type=3 to `dword_80094400`
- `0x80025754` -> `0x80025758` type=21 to `sub_80025758`
- `0x80025758` -> `0x8002575C` type=21 to `sub_8002575C`
- `0x80025758` -> `0x80026EF8` type=17 to `sub_80026EF8`
- `0x8002575C` -> `0x80025760` type=21 to `sub_80025760`
- `0x80025760` -> `0x80014C1C` type=17 to `sub_80014C1C`
- `0x80025760` -> `0x80025764` type=21 to `sub_80025764`
- `0x80025764` -> `0x80025768` type=21 to `sub_80025768`
- `0x80025768` -> `0x8002576C` type=21 to `loc_8002576C`
- `0x8002576C` -> `0x80025770` type=21 to `sub_80025770`
- `0x8002576C` -> `0x80025788` type=19 to `loc_80025788`
- `0x80025770` -> `0x80025774` type=21 to `sub_80025774`
- `0x80025774` -> `0x8002577C` type=21 to `sub_8002577C`
- `0x80025774` -> `0x80094438` type=3 to `dword_80094438`
- `0x8002577C` -> `0x80025780` type=21 to `sub_80025780`
- `0x80025780` -> `0x80025784` type=21 to `sub_80025784`
- `0x80025784` -> `0x80025788` type=21 to `loc_80025788`
- `0x80025788` -> `0x80025790` type=21 to `sub_80025790`
- `0x80025788` -> `0x8008ED00` type=1 to `dword_8008ED00`
- `0x80025790` -> `0x80025794` type=21 to `sub_80025794`
- `0x80025790` -> `0x8008ED00` type=3 to `dword_8008ED00`
- `0x80025794` -> `0x80025798` type=21 to `sub_80025798`
- `0x80025798` -> `0x8002579C` type=21 to `sub_8002579C`
- `0x80025798` -> `0x800258C0` type=19 to `loc_800258C0`
- `0x8002579C` -> `0x800257A0` type=21 to `sub_800257A0`
- `0x800257A0` -> `0x800257A8` type=21 to `sub_800257A8`
- `0x800257A0` -> `0x8008ED00` type=3 to `dword_8008ED00`
- `0x800257A8` -> `0x800257AC` type=21 to `sub_800257AC`
- `0x800257AC` -> `0x800257B0` type=21 to `sub_800257B0`
- `0x800257AC` -> `0x800257F0` type=19 to `loc_800257F0`
- `0x800257B0` -> `0x800257B4` type=21 to `sub_800257B4`
- `0x800257B4` -> `0x80024BF4` type=17 to `sub_80024BF4`
- `0x800257B4` -> `0x800257B8` type=21 to `sub_800257B8`
- `0x800257B8` -> `0x800257BC` type=21 to `sub_800257BC`
- `0x800257BC` -> `0x800257C0` type=21 to `sub_800257C0`
- `0x800257BC` -> `0x800257F0` type=19 to `loc_800257F0`
- `0x800257C0` -> `0x800257C4` type=21 to `sub_800257C4`
- `0x800257C4` -> `0x800257CC` type=21 to `sub_800257CC`
- `0x800257C4` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x800257CC` -> `0x800257D0` type=21 to `sub_800257D0`
- `0x800257D0` -> `0x800257D4` type=21 to `sub_800257D4`
- `0x800257D4` -> `0x800257D8` type=21 to `sub_800257D8`
- `0x800257D4` -> `0x800257F0` type=19 to `loc_800257F0`
- `0x800257D8` -> `0x800257DC` type=21 to `sub_800257DC`
- `0x800257DC` -> `0x80014538` type=17 to `sub_80014538`
- `0x800257DC` -> `0x800257E0` type=21 to `sub_800257E0`
- `0x800257E0` -> `0x800257E4` type=21 to `sub_800257E4`
- `0x800257E4` -> `0x800257E8` type=21 to `sub_800257E8`
- `0x800257E4` -> `0x800257F0` type=19 to `loc_800257F0`
- `0x800257E8` -> `0x800257EC` type=21 to `sub_800257EC`
- `0x800257EC` -> `0x800257F0` type=21 to `loc_800257F0`
- `0x800257EC` -> `0x8008ED00` type=2 to `dword_8008ED00`
- `0x800257F0` -> `0x800257F8` type=21 to `sub_800257F8`
- `0x800257F0` -> `0x8008ED00` type=3 to `dword_8008ED00`
- `0x800257F8` -> `0x800257FC` type=21 to `sub_800257FC`
- `0x800257FC` -> `0x80025800` type=21 to `sub_80025800`
- `0x800257FC` -> `0x80025860` type=19 to `loc_80025860`
- `0x80025800` -> `0x80025804` type=21 to `sub_80025804`
- `0x80025804` -> `0x8002580C` type=21 to `sub_8002580C`
- `0x80025804` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x8002580C` -> `0x80025810` type=21 to `sub_80025810`
- `0x80025810` -> `0x80025814` type=21 to `sub_80025814`
- `0x80025814` -> `0x80025818` type=21 to `sub_80025818`
- `0x80025814` -> `0x80025860` type=19 to `loc_80025860`
- `0x80025818` -> `0x8002581C` type=21 to `sub_8002581C`
- `0x8002581C` -> `0x80025824` type=21 to `sub_80025824`
- `0x8002581C` -> `0x800943F0` type=3 to `dword_800943F0`
- `0x80025824` -> `0x80025828` type=21 to `sub_80025828`
- `0x80025824` -> `0x80026EF8` type=17 to `sub_80026EF8`
- `0x80025828` -> `0x8002582C` type=21 to `sub_8002582C`
- `0x8002582C` -> `0x80025830` type=21 to `sub_80025830`
- `0x80025830` -> `0x80025834` type=21 to `sub_80025834`
- `0x80025834` -> `0x80025838` type=21 to `sub_80025838`
- `0x80025838` -> `0x8002583C` type=21 to `sub_8002583C`
- `0x8002583C` -> `0x80025840` type=21 to `sub_80025840`
- `0x80025840` -> `0x80025848` type=21 to `sub_80025848`
- `0x80025840` -> `0x8008ED14` type=2 to `dword_8008ED14`
- `0x80025848` -> `0x8002584C` type=21 to `sub_8002584C`
- `0x8002584C` -> `0x80025854` type=21 to `sub_80025854`
- `0x8002584C` -> `0x8008ED00` type=2 to `dword_8008ED00`
- `0x80025854` -> `0x8002585C` type=21 to `sub_8002585C`
- `0x80025854` -> `0x8008ED00` type=3 to `dword_8008ED00`
- `0x8002585C` -> `0x80025860` type=21 to `loc_80025860`
- `0x80025860` -> `0x80025864` type=21 to `sub_80025864`
- `0x80025860` -> `0x800258C0` type=19 to `loc_800258C0`
- `0x80025864` -> `0x80025868` type=21 to `sub_80025868`
- `0x80025868` -> `0x80025870` type=21 to `sub_80025870`
- `0x80025868` -> `0x8008ED08` type=3 to `dword_8008ED08`
- `0x80025870` -> `0x80025874` type=21 to `sub_80025874`
- `0x80025874` -> `0x80025878` type=21 to `sub_80025878`
- `0x80025878` -> `0x8002587C` type=21 to `sub_8002587C`
- `0x80025878` -> `0x800258C0` type=19 to `loc_800258C0`
- `0x8002587C` -> `0x80025880` type=21 to `sub_80025880`
- `0x80025880` -> `0x80025888` type=21 to `sub_80025888`
- `0x80025880` -> `0x8008ED00` type=2 to `dword_8008ED00`
- `0x80025888` -> `0x8002588C` type=21 to `sub_8002588C`
- `0x8002588C` -> `0x80025890` type=21 to `sub_80025890`
- `0x80025890` -> `0x80025898` type=21 to `sub_80025898`
- `0x80025890` -> `0x8008ED28` type=2 to `g_PrStageEventStreamDone`
- `0x80025898` -> `0x8002589C` type=21 to `sub_8002589C`
- `0x8002589C` -> `0x800258A4` type=21 to `sub_800258A4`
- `0x8002589C` -> `0x8008ED14` type=2 to `dword_8008ED14`
- `0x800258A4` -> `0x800258A8` type=21 to `sub_800258A8`
- `0x800258A8` -> `0x800258AC` type=21 to `sub_800258AC`
- `0x800258AC` -> `0x800258B0` type=21 to `sub_800258B0`
- `0x800258B0` -> `0x800258B4` type=21 to `sub_800258B4`
- `0x800258B4` -> `0x800258B8` type=21 to `sub_800258B8`
- `0x800258B8` -> `0x800258C0` type=21 to `loc_800258C0`
- `0x800258B8` -> `0x8008ED36` type=2 to `word_8008ED36`
- `0x800258C0` -> `0x80024FC0` type=17 to `sub_80024FC0`
- `0x800258C0` -> `0x800258C4` type=21 to `sub_800258C4`
- `0x800258C4` -> `0x800258C8` type=21 to `loc_800258C8`
- `0x800258C8` -> `0x800258D0` type=21 to `sub_800258D0`
- `0x800258C8` -> `0x8008ECF4` type=1 to `dword_8008ECF4`
- `0x800258D0` -> `0x800258D4` type=21 to `sub_800258D4`
- `0x800258D0` -> `0x8008ECF4` type=3 to `dword_8008ECF4`
- `0x800258D4` -> `0x800258D8` type=21 to `sub_800258D8`
- `0x800258D8` -> `0x800258DC` type=21 to `sub_800258DC`
- `0x800258D8` -> `0x800258EC` type=19 to `loc_800258EC`
- `0x800258DC` -> `0x800258E0` type=21 to `sub_800258E0`
- `0x800258E0` -> `0x800258E4` type=21 to `sub_800258E4`
- `0x800258E4` -> `0x80014BDC` type=17 to `sub_80014BDC`
- `0x800258E4` -> `0x800258E8` type=21 to `sub_800258E8`
- `0x800258E8` -> `0x800258EC` type=21 to `loc_800258EC`
- `0x800258EC` -> `0x800258F4` type=21 to `sub_800258F4`
- `0x800258EC` -> `0x800943C0` type=3 to `dword_800943C0`
- `0x800258F4` -> `0x800258F8` type=21 to `sub_800258F8`
- `0x800258F4` -> `0x8008ECF4` type=3 to `dword_8008ECF4`
- `0x800258F8` -> `0x800258FC` type=21 to `sub_800258FC`
- `0x800258FC` -> `0x80025900` type=21 to `sub_80025900`
- `0x80025900` -> `0x80025904` type=21 to `sub_80025904`
- `0x80025900` -> `0x800259A0` type=19 to `loc_800259A0`
- `0x80025904` -> `0x80025908` type=21 to `sub_80025908`
- `0x80025908` -> `0x80025910` type=21 to `sub_80025910`
- `0x80025908` -> `0x8008ED00` type=3 to `dword_8008ED00`
- `0x80025910` -> `0x80025914` type=21 to `sub_80025914`
- `0x80025914` -> `0x80025918` type=21 to `sub_80025918`
- `0x80025918` -> `0x8002591C` type=21 to `sub_8002591C`
- `0x80025918` -> `0x800259A0` type=19 to `loc_800259A0`
- `0x8002591C` -> `0x80025920` type=21 to `sub_80025920`
- `0x80025920` -> `0x80025924` type=21 to `sub_80025924`
- `0x80025924` -> `0x80025928` type=21 to `sub_80025928`
- `0x80025924` -> `0x8002593C` type=19 to `loc_8002593C`
- `0x80025928` -> `0x8002592C` type=21 to `sub_8002592C`
- `0x8002592C` -> `0x80025934` type=21 to `sub_80025934`
- `0x8002592C` -> `0x8008ED00` type=2 to `dword_8008ED00`
- `0x80025934` -> `0x80025938` type=21 to `sub_80025938`
- `0x80025934` -> `0x80025978` type=19 to `loc_80025978`
- `0x8002593C` -> `0x80025940` type=21 to `sub_80025940`
- `0x8002593C` -> `0x80025958` type=19 to `loc_80025958`
- `0x80025940` -> `0x80025944` type=21 to `sub_80025944`
- `0x80025944` -> `0x80025948` type=21 to `sub_80025948`
- `0x80025948` -> `0x80025950` type=21 to `sub_80025950`
- `0x80025948` -> `0x8008ED00` type=2 to `dword_8008ED00`
- `0x80025950` -> `0x80025954` type=21 to `sub_80025954`
- `0x80025950` -> `0x80025978` type=19 to `loc_80025978`
- `0x80025958` -> `0x8002595C` type=21 to `sub_8002595C`
- `0x80025958` -> `0x80025978` type=19 to `loc_80025978`
- `0x8002595C` -> `0x80025960` type=21 to `sub_80025960`
- `0x80025960` -> `0x80025968` type=21 to `sub_80025968`
- `0x80025960` -> `0x800943FC` type=3 to `dword_800943FC`
- `0x80025968` -> `0x80025970` type=21 to `sub_80025970`
- `0x80025968` -> `0x8008ED00` type=2 to `dword_8008ED00`
- `0x80025970` -> `0x80025974` type=21 to `sub_80025974`
- `0x80025970` -> `0x80026EF8` type=17 to `sub_80026EF8`
- `0x80025974` -> `0x80025978` type=21 to `loc_80025978`
- `0x80025978` -> `0x8002597C` type=21 to `sub_8002597C`
- `0x8002597C` -> `0x80025980` type=21 to `sub_80025980`
- `0x80025980` -> `0x80025984` type=21 to `sub_80025984`
- `0x80025984` -> `0x80014400` type=17 to `sub_80014400`
- `0x80025984` -> `0x80025988` type=21 to `sub_80025988`
- `0x80025988` -> `0x8002598C` type=21 to `sub_8002598C`
- `0x8002598C` -> `0x80014C1C` type=17 to `sub_80014C1C`
- `0x8002598C` -> `0x80025990` type=21 to `sub_80025990`
- `0x80025990` -> `0x80025994` type=21 to `sub_80025994`
- `0x80025994` -> `0x80024F8C` type=17 to `sub_80024F8C`
- `0x80025994` -> `0x80025998` type=21 to `sub_80025998`
- `0x80025998` -> `0x8002599C` type=21 to `loc_8002599C`
- `0x8002599C` -> `0x800259A0` type=21 to `loc_800259A0`
- `0x800259A0` -> `0x800259A4` type=21 to `sub_800259A4`
- `0x800259A4` -> `0x800259A8` type=21 to `sub_800259A8`
- `0x800259A8` -> `0x800259AC` type=21 to `sub_800259AC`
- `0x800259AC` -> `0x800259B0` type=21 to `sub_800259B0`
- `0x800259B0` -> `0x800259B4` type=21 to `sub_800259B4`
- `0x800259B4` -> `0x800259B8` type=21 to `sub_800259B8`
- `0x800259B8` -> `0x800259BC` type=21 to `sub_800259BC`

### Referenced 0x80000000-0x801FFFFF operands

- `0x80024FDC` op1 type=2 -> `0x8008ED20` `dword_8008ED20`
- `0x80024FF4` op2 type=7 -> `0x8002599C` `loc_8002599C`
- `0x8002502C` op1 type=2 -> `0x800916D0` `word_800916D0`
- `0x80025038` op2 type=7 -> `0x80025058` `loc_80025058`
- `0x80025048` op1 type=7 -> `0x8002506C` `loc_8002506C`
- `0x80025050` op0 type=7 -> `0x8002508C` `loc_8002508C`
- `0x80025058` op2 type=7 -> `0x80025040` `loc_80025040`
- `0x80025064` op1 type=7 -> `0x80025088` `loc_80025088`
- `0x80025080` op0 type=7 -> `0x8002508C` `loc_8002508C`
- `0x80025098` op1 type=5 -> `0x8008ECE8` `dword_8008ECE8`
- `0x800250A8` op1 type=2 -> `0x8008ECF0` `dword_8008ECF0`
- `0x800250BC` op1 type=2 -> `0x8008ECF4` `dword_8008ECF4`
- `0x800250C4` op2 type=7 -> `0x800259A0` `loc_800259A0`
- `0x800250CC` op1 type=5 -> `0x800943C8` `dword_800943C8`
- `0x800250D8` op1 type=2 -> `0x8008ECF0` `dword_8008ECF0`
- `0x800250E4` op1 type=7 -> `0x80025104` `loc_80025104`
- `0x800250F0` op1 type=2 -> `0x800943C4` `dword_800943C4`
- `0x80025108` op1 type=2 -> `0x8008ECF4` `dword_8008ECF4`
- `0x80025114` op1 type=7 -> `0x80025194` `loc_80025194`
- `0x80025138` op1 type=7 -> `0x80025194` `loc_80025194`
- `0x80025148` op1 type=2 -> `0x800943C4` `dword_800943C4`
- `0x80025160` op1 type=7 -> `0x8002518C` `loc_8002518C`
- `0x80025184` op0 type=7 -> `0x80025194` `loc_80025194`
- `0x80025194` op1 type=5 -> `0x8008ECF4` `dword_8008ECF4`
- `0x800251A8` op1 type=7 -> `0x800251D8` `loc_800251D8`
- `0x800251CC` op1 type=7 -> `0x800251D8` `loc_800251D8`
- `0x800251D8` op1 type=2 -> `0x800943C0` `dword_800943C0`
- `0x800251E0` op1 type=2 -> `0x8008ECF4` `dword_8008ECF4`
- `0x800251F0` op2 type=7 -> `0x800252D8` `loc_800252D8`
- `0x800251F8` op0 type=7 -> `0x80024BF4` `sub_80024BF4`
- `0x80025204` op2 type=7 -> `0x80025280` `loc_80025280`
- `0x8002520C` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x8002521C` op1 type=7 -> `0x8002523C` `loc_8002523C`
- `0x80025234` op1 type=7 -> `0x80025268` `loc_80025268`
- `0x8002523C` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x8002524C` op1 type=7 -> `0x80025280` `loc_80025280`
- `0x80025254` op0 type=7 -> `0x800144B8` `sub_800144B8`
- `0x80025260` op2 type=7 -> `0x80025280` `loc_80025280`
- `0x80025280` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x80025290` op1 type=7 -> `0x800252A0` `loc_800252A0`
- `0x80025298` op0 type=7 -> `0x800152D0` `sub_800152D0`
- `0x800252A8` op2 type=7 -> `0x800252D4` `loc_800252D4`
- `0x800252B0` op1 type=7 -> `0x800252D4` `loc_800252D4`
- `0x800252C0` op1 type=7 -> `0x800252D8` `loc_800252D8`
- `0x800252CC` op0 type=7 -> `0x800252D8` `loc_800252D8`
- `0x800252D8` op1 type=2 -> `0x800943C0` `dword_800943C0`
- `0x800252E0` op1 type=2 -> `0x8008ECF4` `dword_8008ECF4`
- `0x800252F0` op2 type=7 -> `0x800258C8` `loc_800258C8`
- `0x800252F8` op0 type=7 -> `0x80024BF4` `sub_80024BF4`
- `0x80025300` op1 type=7 -> `0x80025788` `loc_80025788`
- `0x80025308` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x80025318` op1 type=7 -> `0x80025330` `loc_80025330`
- `0x80025320` op0 type=7 -> `0x80014D58` `sub_80014D58`
- `0x80025328` op0 type=7 -> `0x80014458` `sub_80014458`
- `0x80025330` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x80025340` op1 type=7 -> `0x80025788` `loc_80025788`
- `0x80025350` op1 type=7 -> `0x800253A4` `loc_800253A4`
- `0x80025358` op0 type=7 -> `0x80014C80` `sub_80014C80`
- `0x80025364` op1 type=7 -> `0x80025374` `loc_80025374`
- `0x8002536C` op0 type=7 -> `0x8002549C` `loc_8002549C`
- `0x80025374` op2 type=7 -> `0x8002538C` `loc_8002538C`
- `0x80025384` op0 type=7 -> `0x800254A0` `loc_800254A0`
- `0x8002538C` op2 type=7 -> `0x800254A0` `loc_800254A0`
- `0x8002539C` op0 type=7 -> `0x800254A0` `loc_800254A0`
- `0x800253A4` op0 type=7 -> `0x80014D28` `sub_80014D28`
- `0x800253AC` op1 type=2 -> `0x8008ED38` `word_8008ED38`
- `0x800253B8` op1 type=7 -> `0x80025448` `loc_80025448`
- `0x800253C8` op2 type=7 -> `0x800253E8` `loc_800253E8`
- `0x800253D0` op2 type=7 -> `0x800253E8` `loc_800253E8`
- `0x800253D8` op1 type=2 -> `0x8008ED34` `word_8008ED34`
- `0x800253E8` op1 type=7 -> `0x80025424` `loc_80025424`
- `0x800253F4` op1 type=5 -> `0x80055420` `dword_80055420`
- `0x80025424` op1 type=2 -> `0x8008ED38` `word_8008ED38`
- `0x8002542C` op1 type=2 -> `0x8008ED36` `word_8008ED36`
- `0x80025438` op1 type=2 -> `0x8008ED38` `word_8008ED38`
- `0x80025440` op0 type=7 -> `0x800254A0` `loc_800254A0`
- `0x80025450` op2 type=7 -> `0x80025474` `loc_80025474`
- `0x80025458` op1 type=2 -> `0x8008ED36` `word_8008ED36`
- `0x80025464` op1 type=7 -> `0x8002547C` `loc_8002547C`
- `0x80025474` op1 type=2 -> `0x8008ED36` `word_8008ED36`
- `0x8002547C` op1 type=2 -> `0x8009181E` `word_8009181E`
- `0x80025484` op2 type=7 -> `0x80025494` `loc_80025494`
- `0x8002548C` op1 type=2 -> `0x8008ED36` `word_8008ED36`
- `0x80025494` op1 type=2 -> `0x8008ED38` `word_8008ED38`
- `0x800254A0` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x800254AC` op2 type=7 -> `0x80025744` `loc_80025744`
- `0x800254B8` op2 type=7 -> `0x80025744` `loc_80025744`
- `0x800254C0` op1 type=7 -> `0x8002576C` `loc_8002576C`
- `0x800254C8` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x800254D8` op1 type=7 -> `0x80025598` `loc_80025598`
- `0x800254E0` op1 type=2 -> `0x8008ED36` `word_8008ED36`
- `0x800254EC` op2 type=7 -> `0x8002551C` `loc_8002551C`
- `0x800254FC` op2 type=7 -> `0x80025514` `loc_80025514`
- `0x80025504` op0 type=7 -> `0x80014548` `sub_80014548`
- `0x8002550C` op2 type=7 -> `0x80025544` `loc_80025544`
- `0x80025514` op0 type=7 -> `0x80025544` `loc_80025544`
- `0x8002551C` op1 type=7 -> `0x80025544` `loc_80025544`
- `0x8002552C` op2 type=7 -> `0x80025544` `loc_80025544`
- `0x80025534` op0 type=7 -> `0x80014548` `sub_80014548`
- `0x80025544` op0 type=7 -> `0x80024F8C` `sub_80024F8C`
- `0x80025550` op2 type=7 -> `0x80025598` `loc_80025598`
- `0x80025560` op2 type=7 -> `0x80025570` `loc_80025570`
- `0x80025568` op1 type=7 -> `0x80025598` `loc_80025598`
- `0x80025574` op2 type=7 -> `0x800255A0` `loc_800255A0`
- `0x8002557C` op2 type=7 -> `0x800255A0` `loc_800255A0`
- `0x80025584` op1 type=2 -> `0x8009182A` `word_8009182A`
- `0x80025590` op1 type=7 -> `0x800255A4` `loc_800255A4`
- `0x80025598` op0 type=7 -> `0x80025738` `loc_80025738`
- `0x800255A8` op1 type=2 -> `0x800943FC` `dword_800943FC`
- `0x800255B4` op0 type=7 -> `0x80026EF8` `sub_80026EF8`
- `0x800255C4` op2 type=7 -> `0x80025664` `loc_80025664`
- `0x800255CC` op2 type=7 -> `0x80025668` `loc_80025668`
- `0x800255F4` op1 type=7 -> `0x8002560C` `loc_8002560C`
- `0x800255FC` op1 type=2 -> `0x800943F4` `dword_800943F4`
- `0x80025604` op0 type=7 -> `0x80025614` `loc_80025614`
- `0x8002560C` op1 type=2 -> `0x800943F8` `dword_800943F8`
- `0x80025614` op0 type=7 -> `0x80026EF8` `sub_80026EF8`
- `0x80025628` op1 type=2 -> `0x8008ED14` `dword_8008ED14`
- `0x80025634` op1 type=2 -> `0x8008ED0C` `dword_8008ED0C`
- `0x8002564C` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x80025654` op1 type=2 -> `0x8008ED24` `dword_8008ED24`
- `0x8002565C` op0 type=7 -> `0x80025674` `loc_80025674`
- `0x80025668` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x8002567C` op2 type=7 -> `0x800256BC` `loc_800256BC`
- `0x80025684` op2 type=7 -> `0x800256A0` `loc_800256A0`
- `0x80025698` op0 type=7 -> `0x8002570C` `loc_8002570C`
- `0x800256B4` op0 type=7 -> `0x8002570C` `loc_8002570C`
- `0x800256BC` op2 type=7 -> `0x800256EC` `loc_800256EC`
- `0x800256C4` op2 type=7 -> `0x800256DC` `loc_800256DC`
- `0x800256D4` op0 type=7 -> `0x80025708` `loc_80025708`
- `0x800256E4` op0 type=7 -> `0x80025708` `loc_80025708`
- `0x800256F0` op2 type=7 -> `0x8002570C` `loc_8002570C`
- `0x800256F8` op2 type=7 -> `0x8002570C` `loc_8002570C`
- `0x80025714` op1 type=7 -> `0x80025738` `loc_80025738`
- `0x8002571C` op1 type=7 -> `0x8002573C` `loc_8002573C`
- `0x8002573C` op2 type=7 -> `0x80025788` `loc_80025788`
- `0x8002574C` op1 type=2 -> `0x80094400` `dword_80094400`
- `0x80025758` op0 type=7 -> `0x80026EF8` `sub_80026EF8`
- `0x80025760` op0 type=7 -> `0x80014C1C` `sub_80014C1C`
- `0x8002576C` op2 type=7 -> `0x80025788` `loc_80025788`
- `0x80025774` op1 type=2 -> `0x80094438` `dword_80094438`
- `0x80025788` op1 type=5 -> `0x8008ED00` `dword_8008ED00`
- `0x80025798` op1 type=7 -> `0x800258C0` `loc_800258C0`
- `0x800257A0` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x800257AC` op2 type=7 -> `0x800257F0` `loc_800257F0`
- `0x800257B4` op0 type=7 -> `0x80024BF4` `sub_80024BF4`
- `0x800257BC` op2 type=7 -> `0x800257F0` `loc_800257F0`
- `0x800257C4` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x800257D4` op1 type=7 -> `0x800257F0` `loc_800257F0`
- `0x800257DC` op0 type=7 -> `0x80014538` `sub_80014538`
- `0x800257E4` op1 type=7 -> `0x800257F0` `loc_800257F0`
- `0x800257F0` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x800257FC` op2 type=7 -> `0x80025860` `loc_80025860`
- `0x80025804` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x80025814` op1 type=7 -> `0x80025860` `loc_80025860`
- `0x8002581C` op1 type=2 -> `0x800943F0` `dword_800943F0`
- `0x80025824` op0 type=7 -> `0x80026EF8` `sub_80026EF8`
- `0x80025840` op1 type=2 -> `0x8008ED14` `dword_8008ED14`
- `0x8002584C` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x80025854` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x80025860` op2 type=7 -> `0x800258C0` `loc_800258C0`
- `0x80025868` op1 type=2 -> `0x8008ED08` `dword_8008ED08`
- `0x80025878` op1 type=7 -> `0x800258C0` `loc_800258C0`
- `0x80025880` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x80025890` op1 type=2 -> `0x8008ED28` `g_PrStageEventStreamDone`
- `0x8002589C` op1 type=2 -> `0x8008ED14` `dword_8008ED14`
- `0x800258B8` op1 type=2 -> `0x8008ED36` `word_8008ED36`
- `0x800258C0` op0 type=7 -> `0x80024FC0` `sub_80024FC0`
- `0x800258C8` op1 type=5 -> `0x8008ECF4` `dword_8008ECF4`
- `0x800258D8` op2 type=7 -> `0x800258EC` `loc_800258EC`
- `0x800258E4` op0 type=7 -> `0x80014BDC` `sub_80014BDC`
- `0x800258EC` op1 type=2 -> `0x800943C0` `dword_800943C0`
- `0x80025900` op2 type=7 -> `0x800259A0` `loc_800259A0`
- `0x80025908` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x80025918` op1 type=7 -> `0x800259A0` `loc_800259A0`
- `0x80025924` op2 type=7 -> `0x8002593C` `loc_8002593C`
- `0x8002592C` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x80025934` op0 type=7 -> `0x80025978` `loc_80025978`
- `0x8002593C` op2 type=7 -> `0x80025958` `loc_80025958`
- `0x80025948` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x80025950` op0 type=7 -> `0x80025978` `loc_80025978`
- `0x80025958` op2 type=7 -> `0x80025978` `loc_80025978`
- `0x80025960` op1 type=2 -> `0x800943FC` `dword_800943FC`
- `0x80025968` op1 type=2 -> `0x8008ED00` `dword_8008ED00`
- `0x80025970` op0 type=7 -> `0x80026EF8` `sub_80026EF8`
- `0x80025984` op0 type=7 -> `0x80014400` `sub_80014400`
- `0x8002598C` op0 type=7 -> `0x80014C1C` `sub_80014C1C`
- `0x80025994` op0 type=7 -> `0x80024F8C` `sub_80024F8C`

## 0x80024B54 `sub_80024B54` - input class/token dispatch

- Bounds: `0x80024B54` - `0x80024BC0`
- Size: `108` bytes

### Hex-Rays pseudo-C

```c
int __fastcall sub_80024B54(char a1)
{
  int result; // $v0

  result = 1;
  if ( (a1 & 0x10) == 0 )
  {
    result = 2;
    if ( (a1 & 0x20) == 0 )
    {
      result = 3;
      if ( (a1 & 0x40) == 0 )
      {
        result = 4;
        if ( (a1 & 0x80) == 0 )
        {
          result = 5;
          if ( (a1 & 4) == 0 )
          {
            result = 5;
            if ( (a1 & 1) == 0 )
            {
              result = 7;
              if ( (a1 & 8) == 0 )
                return (a1 & 2) != 0 ? 7 : 0;
            }
          }
        }
      }
    }
  }
  return result;
}

```

### Disassembly

```asm
0x80024B54: andi    $v0, $a0, 0x10
0x80024B58: bnez    $v0, locret_80024BB8
0x80024B5C: li      $v0, 1
0x80024B60: andi    $v0, $a0, 0x20
0x80024B64: bnez    $v0, locret_80024BB8
0x80024B68: li      $v0, 2
0x80024B6C: andi    $v0, $a0, 0x40
0x80024B70: bnez    $v0, locret_80024BB8
0x80024B74: li      $v0, 3
0x80024B78: andi    $v0, $a0, 0x80
0x80024B7C: bnez    $v0, locret_80024BB8
0x80024B80: li      $v0, 4
0x80024B84: andi    $v0, $a0, 4
0x80024B88: bnez    $v0, locret_80024BB8
0x80024B8C: li      $v0, 5
0x80024B90: andi    $v0, $a0, 1
0x80024B94: bnez    $v0, locret_80024BB8
0x80024B98: li      $v0, 5
0x80024B9C: andi    $v0, $a0, 8
0x80024BA0: bnez    $v0, locret_80024BB8
0x80024BA4: li      $v0, 7
0x80024BA8: andi    $v0, $a0, 2
0x80024BAC: sltu    $v0, $zero, $v0
0x80024BB0: negu    $v0, $v0
0x80024BB4: andi    $v0, 7
0x80024BB8: jr      $ra
0x80024BBC: nop
```

### Xrefs to

- `0x800149B4` -> `0x80024B54` type=17 from `sub_800149B4`

### Xrefs from

- `0x80024B54` -> `0x80024B58` type=21 to `sub_80024B58`
- `0x80024B58` -> `0x80024B5C` type=21 to `sub_80024B5C`
- `0x80024B58` -> `0x80024BB8` type=19 to `locret_80024BB8`
- `0x80024B5C` -> `0x80024B60` type=21 to `sub_80024B60`
- `0x80024B60` -> `0x80024B64` type=21 to `sub_80024B64`
- `0x80024B64` -> `0x80024B68` type=21 to `sub_80024B68`
- `0x80024B64` -> `0x80024BB8` type=19 to `locret_80024BB8`
- `0x80024B68` -> `0x80024B6C` type=21 to `sub_80024B6C`
- `0x80024B6C` -> `0x80024B70` type=21 to `sub_80024B70`
- `0x80024B70` -> `0x80024B74` type=21 to `sub_80024B74`
- `0x80024B70` -> `0x80024BB8` type=19 to `locret_80024BB8`
- `0x80024B74` -> `0x80024B78` type=21 to `sub_80024B78`
- `0x80024B78` -> `0x80024B7C` type=21 to `sub_80024B7C`
- `0x80024B7C` -> `0x80024B80` type=21 to `sub_80024B80`
- `0x80024B7C` -> `0x80024BB8` type=19 to `locret_80024BB8`
- `0x80024B80` -> `0x80024B84` type=21 to `sub_80024B84`
- `0x80024B84` -> `0x80024B88` type=21 to `sub_80024B88`
- `0x80024B88` -> `0x80024B8C` type=21 to `sub_80024B8C`
- `0x80024B88` -> `0x80024BB8` type=19 to `locret_80024BB8`
- `0x80024B8C` -> `0x80024B90` type=21 to `sub_80024B90`
- `0x80024B90` -> `0x80024B94` type=21 to `sub_80024B94`
- `0x80024B94` -> `0x80024B98` type=21 to `sub_80024B98`
- `0x80024B94` -> `0x80024BB8` type=19 to `locret_80024BB8`
- `0x80024B98` -> `0x80024B9C` type=21 to `sub_80024B9C`
- `0x80024B9C` -> `0x80024BA0` type=21 to `sub_80024BA0`
- `0x80024BA0` -> `0x80024BA4` type=21 to `sub_80024BA4`
- `0x80024BA0` -> `0x80024BB8` type=19 to `locret_80024BB8`
- `0x80024BA4` -> `0x80024BA8` type=21 to `sub_80024BA8`
- `0x80024BA8` -> `0x80024BAC` type=21 to `sub_80024BAC`
- `0x80024BAC` -> `0x80024BB0` type=21 to `sub_80024BB0`
- `0x80024BB0` -> `0x80024BB4` type=21 to `sub_80024BB4`
- `0x80024BB4` -> `0x80024BB8` type=21 to `locret_80024BB8`
- `0x80024BB8` -> `0x80024BBC` type=21 to `sub_80024BBC`

### Referenced 0x80000000-0x801FFFFF operands

- `0x80024B58` op1 type=7 -> `0x80024BB8` `locret_80024BB8`
- `0x80024B64` op1 type=7 -> `0x80024BB8` `locret_80024BB8`
- `0x80024B70` op1 type=7 -> `0x80024BB8` `locret_80024BB8`
- `0x80024B7C` op1 type=7 -> `0x80024BB8` `locret_80024BB8`
- `0x80024B88` op1 type=7 -> `0x80024BB8` `locret_80024BB8`
- `0x80024B94` op1 type=7 -> `0x80024BB8` `locret_80024BB8`
- `0x80024BA0` op1 type=7 -> `0x80024BB8` `locret_80024BB8`

## 0x80035510 `sub_80035510` - input/read wrapper

- Bounds: `0x80035510` - `0x80035540`
- Size: `48` bytes

### Hex-Rays pseudo-C

```c
int sub_80035510()
{
  PAD_dr();
  return ~dword_800882F0;
}

```

### Disassembly

```asm
0x80035510: addiu   $sp, -0x18
0x80035514: sw      $ra, 0x10+var_s0($sp)
0x80035518: jal     PAD_dr
0x8003551C: nop
0x80035520: lw      $v0, dword_800882F0
0x80035528: nop
0x8003552C: nor     $v0, $zero, $v0
0x80035530: lw      $ra, 0x10+var_s0($sp)
0x80035534: addiu   $sp, 0x18
0x80035538: jr      $ra
0x8003553C: nop
```

### Xrefs to

- `0x800157DC` -> `0x80035510` type=17 from `sub_800157DC`
- `0x80015B9C` -> `0x80035510` type=17 from `sub_80015B9C`
- `0x80016AF0` -> `0x80035510` type=17 from `sub_80016AF0`
- `0x8001900C` -> `0x80035510` type=17 from `loc_8001900C`
- `0x8002674C` -> `0x80035510` type=17 from `sub_8002674C`
- `0x80026CA8` -> `0x80035510` type=17 from `loc_80026CA8`
- `0x80027AB0` -> `0x80035510` type=17 from `loc_80027AB0`
- `0x80027EAC` -> `0x80035510` type=17 from `sub_80027EAC`
- `0x801C467C` -> `0x80035510` type=17 from `sub_801C467C`
- `0x801C4B50` -> `0x80035510` type=17 from `sub_801C4B50`
- `0x801C4BC0` -> `0x80035510` type=17 from `sub_801C4BC0`
- `0x801C4C58` -> `0x80035510` type=17 from `loc_801C4C58`
- `0x801C4CD4` -> `0x80035510` type=17 from `sub_801C4CD4`

### Xrefs from

- `0x80035510` -> `0x80035514` type=21 to `sub_80035514`
- `0x80035514` -> `0x80035518` type=21 to `sub_80035518`
- `0x80035518` -> `0x8003551C` type=21 to `sub_8003551C`
- `0x80035518` -> `0x80048A00` type=17 to `PAD_dr`
- `0x8003551C` -> `0x80035520` type=21 to `sub_80035520`
- `0x80035520` -> `0x80035528` type=21 to `sub_80035528`
- `0x80035520` -> `0x800882F0` type=3 to `dword_800882F0`
- `0x80035528` -> `0x8003552C` type=21 to `sub_8003552C`
- `0x8003552C` -> `0x80035530` type=21 to `sub_80035530`
- `0x80035530` -> `0x80035534` type=21 to `sub_80035534`
- `0x80035534` -> `0x80035538` type=21 to `sub_80035538`
- `0x80035538` -> `0x8003553C` type=21 to `sub_8003553C`

### Referenced 0x80000000-0x801FFFFF operands

- `0x80035518` op0 type=7 -> `0x80048A00` `PAD_dr`
- `0x80035520` op1 type=2 -> `0x800882F0` `dword_800882F0`

## 0x801C9094 `sub_801C9094` - COMOD1 script/scorer bridge

Function not found at this address.

## 0x801C7A60 `sub_801C7A60` - COMOD1 stage1 main loop

Function not found at this address.
