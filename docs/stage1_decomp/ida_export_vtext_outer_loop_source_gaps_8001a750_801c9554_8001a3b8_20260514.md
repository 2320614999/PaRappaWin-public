[summary]
success: True
mode: exec
filename: <mcp-idapython-exec-job>
persist_globals: True
result_type: NoneType
result_repr: None

[stdout]
# IDA export: vtext outer-loop source gaps 8001A750 / 801C9554 / 8001A3B8 (2026-05-14)

Authority: current active IDA database. Static evidence only; no Win fallback, ISO, payload, or render-derived success authority.
Input file: `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`

## Function `8001A750` `sub_8001A750`
Bounds: `8001A750-8001A7A4`

### Pseudocode
```c
int sub_8001A750()
{
  bool v0; // dc
  int result; // $v0
  _BYTE v2[16]; // [sp+10h] [-10h] BYREF

  v0 = sub_800364D0(1, v2) != 2;
  result = 0;
  if ( !v0 )
  {
    result = 1;
    if ( (v2[0] & 0x20) == 0 )
    {
      sub_80036678(1, 0);
      return 0;
    }
  }
  return result;
}
```

### Direct xrefs to function
- `801C45F0` -> `8001A750` type=17 caller=801C455C `sub_801C455C`: `jal     sub_8001A750`
- `801C4968` -> `8001A750` type=17 caller=801C4894 `sub_801C4894`: `jal     sub_8001A750`

### Disassembly
```asm
8001A750: addiu   $sp, -0x28
8001A754: sw      $ra, 0x20+var_s0($sp)
8001A758: li      $a0, 1
8001A75C: jal     sub_800364D0
8001A760: addiu   $a1, $sp, 0x20+var_10
8001A764: li      $v1, 2
8001A768: bne     $v0, $v1, loc_8001A794
8001A76C: move    $v0, $zero
8001A770: lbu     $v0, 0x20+var_10($sp)
8001A774: nop
8001A778: andi    $v0, 0x20
8001A77C: bnez    $v0, loc_8001A794
8001A780: li      $v0, 1
8001A784: li      $a0, 1
8001A788: jal     sub_80036678
8001A78C: move    $a1, $zero
8001A790: move    $v0, $zero
8001A794: lw      $ra, 0x20+var_s0($sp)
8001A798: addiu   $sp, 0x28
8001A79C: jr      $ra
8001A7A0: nop
```

## Function `8001A3B8` `GsGetWorkBase`
Bounds: `8001A3B8-8001A3C8`

### Pseudocode
```c
int GsGetWorkBase()
{
  return 0;
}
```

### Direct xrefs to function
- `801C4714` -> `8001A3B8` type=17 caller=801C455C `sub_801C455C`: `jal     GsGetWorkBase`

### Disassembly
```asm
8001A3B8: lw      $v0, dword_80049428
8001A3C0: jr      $ra
8001A3C4: nop
```

## Function `8001A3C8` `sub_8001A3C8`
Bounds: `8001A3C8-8001A478`

### Pseudocode
```c
// write access to const memory has been detected, the output may be wrong!
int sub_8001A3C8()
{
  int v0; // $v1
  int result; // $v0
  bool v2; // dc
  _BYTE v3[16]; // [sp+10h] [-10h] BYREF

  v0 = sub_800364D0(1, (int)v3);
  if ( v0 == 5 && (v3[0] & 0x10) != 0 )
  {
    dword_80049428 = 1;
    return 0;
  }
  else
  {
    result = 0;
    if ( v0 == 2 )
    {
      v2 = sub_800363A4() == 13;
      result = 1;
      if ( v2 )
      {
        return 0;
      }
      else
      {
        dword_80049428 = 0;
        byte_800493F4 = v3[0];
        byte_800493F5 = v3[1];
        byte_800493F6 = v3[2];
      }
    }
  }
  return result;
}
```

### Direct xrefs to function
- `801C44AC` -> `8001A3C8` type=17 caller=801C448C `sub_801C448C`: `jal     sub_8001A3C8`

### Disassembly
```asm
8001A3C8: addiu   $sp, -0x28
8001A3CC: sw      $ra, 0x20+var_s0($sp)
8001A3D0: li      $a0, 1
8001A3D4: jal     sub_800364D0
8001A3D8: addiu   $a1, $sp, 0x20+var_10
8001A3DC: move    $v1, $v0
8001A3E0: li      $v0, 5
8001A3E4: bne     $v1, $v0, loc_8001A414
8001A3E8: li      $v0, 2
8001A3EC: lbu     $v0, 0x20+var_10($sp)
8001A3F0: nop
8001A3F4: andi    $v0, 0x10
8001A3F8: beqz    $v0, loc_8001A410
8001A3FC: li      $v0, 1
8001A400: sw      $v0, dword_80049428
8001A408: j       loc_8001A468
8001A40C: move    $v0, $zero
8001A410: li      $v0, 2
8001A414: bne     $v1, $v0, loc_8001A468
8001A418: move    $v0, $zero
8001A41C: jal     sub_800363A4
8001A420: nop
8001A424: li      $v1, 0xD
8001A428: beq     $v0, $v1, loc_8001A464
8001A42C: li      $v0, 1
8001A430: lbu     $v1, 0x20+var_10($sp)
8001A434: lbu     $a0, 0x20+var_F($sp)
8001A438: lbu     $a1, 0x20+var_E($sp)
8001A43C: sw      $zero, dword_80049428
8001A444: sb      $v1, byte_800493F4
8001A44C: sb      $a0, byte_800493F5
8001A454: sb      $a1, byte_800493F6
8001A45C: j       loc_8001A468
8001A460: nop
8001A464: move    $v0, $zero
8001A468: lw      $ra, 0x20+var_s0($sp)
8001A46C: addiu   $sp, 0x28
8001A470: jr      $ra
8001A474: nop
```

## Data xrefs to `801C9554` `dword_801C9554`
- `801C4518` -> `801C9554` type=2 caller=801C44E0 `sub_801C44E0`: `sw      $v0, dword_801C9554`
- `801C46B8` -> `801C9554` type=3 caller=801C455C `sub_801C455C`: `lw      $v0, dword_801C9554`
- `801C46F8` -> `801C9554` type=3 caller=801C455C `sub_801C455C`: `lw      $v0, dword_801C9554`

## Data xrefs to `00000164` `sub_00000164`
- none

## Caller windows
### Around `801C45E8` in `sub_801C455C`
```asm
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
```

### Around `801C46A0` in `sub_801C455C`
```asm
   801C4680: li      $a0, 1
   801C4684: move    $v1, $v0
   801C4688: beqz    $v1, loc_801C46A8
   801C468C: li      $v0, 0x100
   801C4690: beq     $v1, $v0, loc_801C45A8
   801C4694: li      $v0, 0x800
   801C4698: beq     $v1, $v0, loc_801C472C
   801C469C: andi    $v0, $v1, 0x840
=> 801C46A0: bnez    $v0, loc_801C472C
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
```

### Around `801C46B8` in `sub_801C455C`
```asm
   801C4698: beq     $v1, $v0, loc_801C472C
   801C469C: andi    $v0, $v1, 0x840
   801C46A0: bnez    $v0, loc_801C472C
   801C46A4: nop
   801C46A8: jal     sub_80024CF8
   801C46AC: move    $a0, $s0
   801C46B0: jal     sub_80027528
   801C46B4: nop
=> 801C46B8: lw      $v0, dword_801C9554
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
```

### Around `801C4748` in `sub_801C455C`
```asm
   801C4728: nop
   801C472C: jal     sub_80027664
   801C4730: nop
   801C4734: jal     sub_8001A4A4
   801C4738: li      $a0, 1
   801C473C: jal     sub_8001A694
   801C4740: nop
   801C4744: jal     nullsub_4
=> 801C4748: move    $a0, $s0
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

### Around `801C4944` in `sub_801C4894`
```asm
   801C4924: jal     sub_80014344
   801C4928: nop
   801C492C: jal     sub_80024E98
   801C4930: nop
   801C4934: jal     sub_80024FC0
   801C4938: move    $a0, $s1
   801C493C: jal     sub_801C4FA0
   801C4940: nop
=> 801C4944: jal     sub_80024C84
   801C4948: move    $a0, $zero
   801C494C: move    $a0, $s1
   801C4950: li      $a1, 1
   801C4954: li      $a2, 2
   801C4958: jal     sub_80020110
   801C495C: li      $a3, 1
   801C4960: sw      $zero, dword_801C954C
   801C4968: jal     sub_8001A750
   801C496C: nop
   801C4970: beq     $v0, $s0, loc_801C49AC
   801C4974: nop
   801C4978: beqz    $s4, loc_801C4988
   801C497C: move    $a0, $s1
   801C4980: jal     sub_801C6410
```

### Around `801C4960` in `sub_801C4894`
```asm
   801C4940: nop
   801C4944: jal     sub_80024C84
   801C4948: move    $a0, $zero
   801C494C: move    $a0, $s1
   801C4950: li      $a1, 1
   801C4954: li      $a2, 2
   801C4958: jal     sub_80020110
   801C495C: li      $a3, 1
=> 801C4960: sw      $zero, dword_801C954C
   801C4968: jal     sub_8001A750
   801C496C: nop
   801C4970: beq     $v0, $s0, loc_801C49AC
   801C4974: nop
   801C4978: beqz    $s4, loc_801C4988
   801C497C: move    $a0, $s1
   801C4980: jal     sub_801C6410
   801C4984: move    $a1, $zero
   801C4988: jal     sub_80035560
   801C498C: li      $a0, 2
   801C4990: beqz    $s3, loc_801C49A0
   801C4994: nop
   801C4998: jal     sub_801C689C
   801C499C: move    $a0, $s1
```



[console]
# IDA export: vtext outer-loop source gaps 8001A750 / 801C9554 / 8001A3B8 (2026-05-14)

Authority: current active IDA database. Static evidence only; no Win fallback, ISO, payload, or render-derived success authority.
Input file: `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`

## Function `8001A750` `sub_8001A750`
Bounds: `8001A750-8001A7A4`

### Pseudocode
```c
int sub_8001A750()
{
  bool v0; // dc
  int result; // $v0
  _BYTE v2[16]; // [sp+10h] [-10h] BYREF

  v0 = sub_800364D0(1, v2) != 2;
  result = 0;
  if ( !v0 )
  {
    result = 1;
    if ( (v2[0] & 0x20) == 0 )
    {
      sub_80036678(1, 0);
      return 0;
    }
  }
  return result;
}
```

### Direct xrefs to function
- `801C45F0` -> `8001A750` type=17 caller=801C455C `sub_801C455C`: `jal     sub_8001A750`
- `801C4968` -> `8001A750` type=17 caller=801C4894 `sub_801C4894`: `jal     sub_8001A750`

### Disassembly
```asm
8001A750: addiu   $sp, -0x28
8001A754: sw      $ra, 0x20+var_s0($sp)
8001A758: li      $a0, 1
8001A75C: jal     sub_800364D0
8001A760: addiu   $a1, $sp, 0x20+var_10
8001A764: li      $v1, 2
8001A768: bne     $v0, $v1, loc_8001A794
8001A76C: move    $v0, $zero
8001A770: lbu     $v0, 0x20+var_10($sp)
8001A774: nop
8001A778: andi    $v0, 0x20
8001A77C: bnez    $v0, loc_8001A794
8001A780: li      $v0, 1
8001A784: li      $a0, 1
8001A788: jal     sub_80036678
8001A78C: move    $a1, $zero
8001A790: move    $v0, $zero
8001A794: lw      $ra, 0x20+var_s0($sp)
8001A798: addiu   $sp, 0x28
8001A79C: jr      $ra
8001A7A0: nop
```

## Function `8001A3B8` `GsGetWorkBase`
Bounds: `8001A3B8-8001A3C8`

### Pseudocode
```c
int GsGetWorkBase()
{
  return 0;
}
```

### Direct xrefs to function
- `801C4714` -> `8001A3B8` type=17 caller=801C455C `sub_801C455C`: `jal     GsGetWorkBase`

### Disassembly
```asm
8001A3B8: lw      $v0, dword_80049428
8001A3C0: jr      $ra
8001A3C4: nop
```

## Function `8001A3C8` `sub_8001A3C8`
Bounds: `8001A3C8-8001A478`

### Pseudocode
```c
// write access to const memory has been detected, the output may be wrong!
int sub_8001A3C8()
{
  int v0; // $v1
  int result; // $v0
  bool v2; // dc
  _BYTE v3[16]; // [sp+10h] [-10h] BYREF

  v0 = sub_800364D0(1, (int)v3);
  if ( v0 == 5 && (v3[0] & 0x10) != 0 )
  {
    dword_80049428 = 1;
    return 0;
  }
  else
  {
    result = 0;
    if ( v0 == 2 )
    {
      v2 = sub_800363A4() == 13;
      result = 1;
      if ( v2 )
      {
        return 0;
      }
      else
      {
        dword_80049428 = 0;
        byte_800493F4 = v3[0];
        byte_800493F5 = v3[1];
        byte_800493F6 = v3[2];
      }
    }
  }
  return result;
}
```

### Direct xrefs to function
- `801C44AC` -> `8001A3C8` type=17 caller=801C448C `sub_801C448C`: `jal     sub_8001A3C8`

### Disassembly
```asm
8001A3C8: addiu   $sp, -0x28
8001A3CC: sw      $ra, 0x20+var_s0($sp)
8001A3D0: li      $a0, 1
8001A3D4: jal     sub_800364D0
8001A3D8: addiu   $a1, $sp, 0x20+var_10
8001A3DC: move    $v1, $v0
8001A3E0: li      $v0, 5
8001A3E4: bne     $v1, $v0, loc_8001A414
8001A3E8: li      $v0, 2
8001A3EC: lbu     $v0, 0x20+var_10($sp)
8001A3F0: nop
8001A3F4: andi    $v0, 0x10
8001A3F8: beqz    $v0, loc_8001A410
8001A3FC: li      $v0, 1
8001A400: sw      $v0, dword_80049428
8001A408: j       loc_8001A468
8001A40C: move    $v0, $zero
8001A410: li      $v0, 2
8001A414: bne     $v1, $v0, loc_8001A468
8001A418: move    $v0, $zero
8001A41C: jal     sub_800363A4
8001A420: nop
8001A424: li      $v1, 0xD
8001A428: beq     $v0, $v1, loc_8001A464
8001A42C: li      $v0, 1
8001A430: lbu     $v1, 0x20+var_10($sp)
8001A434: lbu     $a0, 0x20+var_F($sp)
8001A438: lbu     $a1, 0x20+var_E($sp)
8001A43C: sw      $zero, dword_80049428
8001A444: sb      $v1, byte_800493F4
8001A44C: sb      $a0, byte_800493F5
8001A454: sb      $a1, byte_800493F6
8001A45C: j       loc_8001A468
8001A460: nop
8001A464: move    $v0, $zero
8001A468: lw      $ra, 0x20+var_s0($sp)
8001A46C: addiu   $sp, 0x28
8001A470: jr      $ra
8001A474: nop
```

## Data xrefs to `801C9554` `dword_801C9554`
- `801C4518` -> `801C9554` type=2 caller=801C44E0 `sub_801C44E0`: `sw      $v0, dword_801C9554`
- `801C46B8` -> `801C9554` type=3 caller=801C455C `sub_801C455C`: `lw      $v0, dword_801C9554`
- `801C46F8` -> `801C9554` type=3 caller=801C455C `sub_801C455C`: `lw      $v0, dword_801C9554`

## Data xrefs to `00000164` `sub_00000164`
- none

## Caller windows
### Around `801C45E8` in `sub_801C455C`
```asm
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
```

### Around `801C46A0` in `sub_801C455C`
```asm
   801C4680: li      $a0, 1
   801C4684: move    $v1, $v0
   801C4688: beqz    $v1, loc_801C46A8
   801C468C: li      $v0, 0x100
   801C4690: beq     $v1, $v0, loc_801C45A8
   801C4694: li      $v0, 0x800
   801C4698: beq     $v1, $v0, loc_801C472C
   801C469C: andi    $v0, $v1, 0x840
=> 801C46A0: bnez    $v0, loc_801C472C
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
```

### Around `801C46B8` in `sub_801C455C`
```asm
   801C4698: beq     $v1, $v0, loc_801C472C
   801C469C: andi    $v0, $v1, 0x840
   801C46A0: bnez    $v0, loc_801C472C
   801C46A4: nop
   801C46A8: jal     sub_80024CF8
   801C46AC: move    $a0, $s0
   801C46B0: jal     sub_80027528
   801C46B4: nop
=> 801C46B8: lw      $v0, dword_801C9554
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
```

### Around `801C4748` in `sub_801C455C`
```asm
   801C4728: nop
   801C472C: jal     sub_80027664
   801C4730: nop
   801C4734: jal     sub_8001A4A4
   801C4738: li      $a0, 1
   801C473C: jal     sub_8001A694
   801C4740: nop
   801C4744: jal     nullsub_4
=> 801C4748: move    $a0, $s0
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

### Around `801C4944` in `sub_801C4894`
```asm
   801C4924: jal     sub_80014344
   801C4928: nop
   801C492C: jal     sub_80024E98
   801C4930: nop
   801C4934: jal     sub_80024FC0
   801C4938: move    $a0, $s1
   801C493C: jal     sub_801C4FA0
   801C4940: nop
=> 801C4944: jal     sub_80024C84
   801C4948: move    $a0, $zero
   801C494C: move    $a0, $s1
   801C4950: li      $a1, 1
   801C4954: li      $a2, 2
   801C4958: jal     sub_80020110
   801C495C: li      $a3, 1
   801C4960: sw      $zero, dword_801C954C
   801C4968: jal     sub_8001A750
   801C496C: nop
   801C4970: beq     $v0, $s0, loc_801C49AC
   801C4974: nop
   801C4978: beqz    $s4, loc_801C4988
   801C497C: move    $a0, $s1
   801C4980: jal     sub_801C6410
```

### Around `801C4960` in `sub_801C4894`
```asm
   801C4940: nop
   801C4944: jal     sub_80024C84
   801C4948: move    $a0, $zero
   801C494C: move    $a0, $s1
   801C4950: li      $a1, 1
   801C4954: li      $a2, 2
   801C4958: jal     sub_80020110
   801C495C: li      $a3, 1
=> 801C4960: sw      $zero, dword_801C954C
   801C4968: jal     sub_8001A750
   801C496C: nop
   801C4970: beq     $v0, $s0, loc_801C49AC
   801C4974: nop
   801C4978: beqz    $s4, loc_801C4988
   801C497C: move    $a0, $s1
   801C4980: jal     sub_801C6410
   801C4984: move    $a1, $zero
   801C4988: jal     sub_80035560
   801C498C: li      $a0, 2
   801C4990: beqz    $s3, loc_801C49A0
   801C4994: nop
   801C4998: jal     sub_801C689C
   801C499C: move    $a0, $s1
```



[result]
null
