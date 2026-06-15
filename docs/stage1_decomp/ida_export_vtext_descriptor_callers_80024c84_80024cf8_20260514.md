[summary]
success: True
mode: script
filename: E:\game\PSgame\parappa the rapper\win\temp\ida_export_vtext_descriptor_callers_20260514.py
persist_globals: False
result_type: NoneType
result_repr: None

[stdout]
# IDA export: vtext descriptor callers 80024C84/80024CF8 (2026-05-14)

Scope: narrow current-IDB export for Stage1 vtext descriptor source closure.
Targets: `80024C84`, `80024CF8`; includes direct xrefs, caller pseudocode, and call-site disassembly windows.

## Target `80024C84` `sub_80024C84`

### Pseudocode

```c
int __fastcall sub_80024C84(int *a1)
{
  _DWORD *v1; // $gp
  int v3; // $v1
  int v4; // $a0
  int v5; // $a1
  int result; // $v0

  word_8008ECFA = 0;
  dword_8008ECE4 = 0;
  v1[223] = 0;
  v1[200] = 0;
  if ( a1 )
  {
    v3 = a1[6];
    v4 = a1[5];
    v5 = a1[word_800916D8];
    result = *a1;
    v1[219] = v3;
    v1[220] = v4;
    v1[217] = result;
    v1[221] = v5;
  }
  else
  {
    v1[219] = 0;
    v1[220] = 0;
  }
  return result;
}

```

### Direct xrefs

- `801C45CC -> 80024C84` type=17 caller=`801C455C sub_801C455C`
- `801C4944 -> 80024C84` type=17 caller=`801C4894 sub_801C4894`

### Caller `801C455C` `sub_801C455C` pseudocode

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

### Caller `801C455C` call-site windows

#### `801C45CC -> 80024C84`

```asm
   801C45AC: li      $s3, 1
   801C45B0: jal     sub_8001A694
   801C45B4: nop
   801C45B8: jal     StopCallback
   801C45BC: nop
   801C45C0: j       loc_801C472C
   801C45C4: nop
   801C45C8: move    $a0, $zero
=> 801C45CC: jal     sub_80024C84
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
```

### Caller `801C4894` `sub_801C4894` pseudocode

```c
int __fastcall sub_801C4894(int a1, int a2)
{
  int v4; // $s7
  int v5; // $s5
  int v6; // $fp
  int v7; // $s2
  int v8; // $s2
  int v9; // $s4
  int v10; // $s3
  int v11; // $a0
  int v12; // $a0
  int v13; // kr04_4
  int v14; // $v0
  int v15; // $a0
  int v16; // $a0
  int v17; // $v0
  int i; // $s0
  int v19; // $v0
  int v20; // $s0
  int v22[2]; // [sp+10h] [-10h] BYREF
  int v23; // [sp+18h] [-8h]

  v4 = 0;
  v5 = 0;
  v6 = 1800;
  if ( word_800916FC == 1 )
    v6 = 450;
  v7 = 1800;
  sub_801C609C();
  sub_801C44E0(a1, 1);
  sub_8001A478(*(__int16 *)(a1 + 6));
  sub_8001A654(*(__int16 *)(a1 + 4));
  sub_80014344();
  sub_80024E98();
  sub_80024FC0(a2);
  sub_801C4FA0();
  sub_80024C84(0);
  sub_80020110(a2, 1, 2, 1);
  dword_801C954C = 0;
  do
  {
    if ( sub_8001A750() == 1 )
      break;
    if ( sub_801C6410 )
      sub_801C6410(a2, 0);
    sub_80035560(2);
    if ( sub_801C689C )
      sub_801C689C(a2);
    --v7;
  }
  while ( v7 > 0 );
  v8 = 0;
  sub_8001A280();
  v9 = 0;
  v10 = -17;
  v23 = 15;
  *(_DWORD *)(a2 + 12) = MEMORY[0x164];
  sub_80026FA4(v11);
  v22[0] = 0;
  *(_WORD *)(a2 + 96) = 1;
  *(_WORD *)(a2 + 84) = 0;
  do
  {
    v12 = -1851654144;
    if ( v10 > 0 )
    {
      v12 = -1851608123;
      *(_DWORD *)(a2 + 12) = 9792 * v10 / 1800;
    }
    if ( v8 || v10 < 50 )
    {
      if ( v8 == 1 && v10 >= 370 )
      {
        v8 = 2;
        sub_8001A694();
        v5 = v6;
        sub_80026EF8(dword_80094410);
        sub_80026ECC();
        sub_801C57E0(a2);
      }
    }
    else
    {
      v8 = 1;
      sub_80027664(v12);
    }
    v13 = *(_DWORD *)(a2 + 12) % 384;
    *(_WORD *)(a2 + 4) = (__int16)*(_DWORD *)(a2 + 12) / 384 + 1;
    *(_BYTE *)(a2 + 6) = (char)v13 / 96 + 1;
    *(_BYTE *)(a2 + 7) = v13 % 96 + 1;
    if ( !v8 )
    {
      v14 = sub_80035510();
      if ( !v14 || v9 == v14 )
      {
        v9 = v14;
        sub_80040370();
        sub_80027528(v16);
        *(_DWORD *)a2 = 0;
        sub_8002756C();
        sub_80035560(2);
        goto LABEL_45;
      }
      v10 = 370;
      sub_80027664(v15);
      sub_8001A694();
      sub_80040370();
      goto LABEL_28;
    }
    if ( v8 != 1 )
    {
      if ( v8 != 2 )
      {
        --v23;
        sub_801C6410(a2, 0);
        *(_DWORD *)a2 = 0;
        sub_8001E3E4(v22);
        goto LABEL_44;
      }
      sub_801C6410(a2, 0);
      *(_DWORD *)a2 = 0;
      sub_8001E3E4(v22);
      v19 = sub_80035510();
      v20 = v19;
      if ( v9 != v19 )
      {
        if ( v19 )
          v5 = v6;
        v4 = sub_801C47EC(v19, v22);
        v9 = v20;
        if ( !v4 )
          goto LABEL_41;
        v8 = 3;
      }
      v9 = v20;
LABEL_41:
      sub_80024FD0(a2);
      --v5;
      sub_801C5854(a2, v22[0]);
      if ( v5 <= 0 )
      {
        v8 = 3;
        v4 = 3;
      }
      goto LABEL_44;
    }
    v17 = sub_80035510();
    if ( !v17 || v9 == v17 )
    {
      v9 = v17;
      sub_801C6410(a2, 0);
      *(_DWORD *)a2 = 0;
      sub_80024FD0(a2);
      sub_801C5538((_DWORD *)a2);
LABEL_44:
      sub_80035560(2);
      sub_801C689C(a2);
      goto LABEL_45;
    }
    sub_8001A694();
LABEL_28:
    *(_DWORD *)a2 = 0x4000000;
    sub_801C6410(a2, 0);
    sub_80035560(2);
    sub_801C689C(a2);
    for ( i = 15; i > 0; --i )
      sub_80035560(2);
    sub_801C5AB4(a2);
    v8 = 2;
    *(_DWORD *)a2 |= 0x8000000u;
    sub_801C6410(a2, 0);
    sub_80035560(2);
    sub_801C689C(a2);
    while ( sub_80035510() )
      ;
    v5 = v6;
    sub_80026EF8(dword_80094410);
    sub_80026ECC();
LABEL_45:
    ++v10;
  }
  while ( v23 > 0 );
  return v4;
}

```

### Caller `801C4894` call-site windows

#### `801C4944 -> 80024C84`

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
```

## Target `80024CF8` `sub_80024CF8`

### Pseudocode

```c
PrGameContext_Psx *__fastcall sub_80024CF8(PrGameContext_Psx *a1)
{
  _DWORD *v1; // $gp
  int v3; // $v1
  int v4; // $v1
  int v5; // $a3
  unsigned __int16 *v6; // $a2
  unsigned __int16 *v7; // $a0
  PrGameContext_Psx *result; // $v0
  int v9; // $v0
  bool v10; // dc
  int v11; // $v0

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
  v5 = v1[223];
  if ( v5 < v1[219]
    && (v6 = (unsigned __int16 *)(v1[220] + 16 * v5),
        (*v6 << 16)
      + (*((unsigned __int8 *)v6 + 2) << 8)
      + *((unsigned __int8 *)v6 + 3)
      - ((*(unsigned __int16 *)&a1->_pad0[4] << 16)
       + (a1->_pad0[6] << 8)
       + a1->_pad0[7]) <= 0) )
  {
    v1[223] = v5 + 1;
    v7 = v6;
  }
  else
  {
    v7 = 0;
  }
  result = 0;
  if ( v7 )
  {
    v9 = (__int16)v7[word_800916D8 + 3];
    v10 = v9 <= 0;
    v11 = 4 * v9;
    if ( !v10 )
    {
      dword_8008ECE4 = *(_DWORD *)(v11 + v1[221]);
      word_8008ECFA = v7[2];
    }
    result = a1;
    a1->text268 = dword_8008ECE4;
  }
  return result;
}

```

### Direct xrefs

- `801C46A8 -> 80024CF8` type=17 caller=`801C455C sub_801C455C`

### Caller `801C455C` `sub_801C455C` pseudocode

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

### Caller `801C455C` call-site windows

#### `801C46A8 -> 80024CF8`

```asm
   801C4688: beqz    $v1, loc_801C46A8
   801C468C: li      $v0, 0x100
   801C4690: beq     $v1, $v0, loc_801C45A8
   801C4694: li      $v0, 0x800
   801C4698: beq     $v1, $v0, loc_801C472C
   801C469C: andi    $v0, $v1, 0x840
   801C46A0: bnez    $v0, loc_801C472C
   801C46A4: nop
=> 801C46A8: jal     sub_80024CF8
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
```



[console]
# IDA export: vtext descriptor callers 80024C84/80024CF8 (2026-05-14)

Scope: narrow current-IDB export for Stage1 vtext descriptor source closure.
Targets: `80024C84`, `80024CF8`; includes direct xrefs, caller pseudocode, and call-site disassembly windows.

## Target `80024C84` `sub_80024C84`

### Pseudocode

```c
int __fastcall sub_80024C84(int *a1)
{
  _DWORD *v1; // $gp
  int v3; // $v1
  int v4; // $a0
  int v5; // $a1
  int result; // $v0

  word_8008ECFA = 0;
  dword_8008ECE4 = 0;
  v1[223] = 0;
  v1[200] = 0;
  if ( a1 )
  {
    v3 = a1[6];
    v4 = a1[5];
    v5 = a1[word_800916D8];
    result = *a1;
    v1[219] = v3;
    v1[220] = v4;
    v1[217] = result;
    v1[221] = v5;
  }
  else
  {
    v1[219] = 0;
    v1[220] = 0;
  }
  return result;
}

```

### Direct xrefs

- `801C45CC -> 80024C84` type=17 caller=`801C455C sub_801C455C`
- `801C4944 -> 80024C84` type=17 caller=`801C4894 sub_801C4894`

### Caller `801C455C` `sub_801C455C` pseudocode

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

### Caller `801C455C` call-site windows

#### `801C45CC -> 80024C84`

```asm
   801C45AC: li      $s3, 1
   801C45B0: jal     sub_8001A694
   801C45B4: nop
   801C45B8: jal     StopCallback
   801C45BC: nop
   801C45C0: j       loc_801C472C
   801C45C4: nop
   801C45C8: move    $a0, $zero
=> 801C45CC: jal     sub_80024C84
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
```

### Caller `801C4894` `sub_801C4894` pseudocode

```c
int __fastcall sub_801C4894(int a1, int a2)
{
  int v4; // $s7
  int v5; // $s5
  int v6; // $fp
  int v7; // $s2
  int v8; // $s2
  int v9; // $s4
  int v10; // $s3
  int v11; // $a0
  int v12; // $a0
  int v13; // kr04_4
  int v14; // $v0
  int v15; // $a0
  int v16; // $a0
  int v17; // $v0
  int i; // $s0
  int v19; // $v0
  int v20; // $s0
  int v22[2]; // [sp+10h] [-10h] BYREF
  int v23; // [sp+18h] [-8h]

  v4 = 0;
  v5 = 0;
  v6 = 1800;
  if ( word_800916FC == 1 )
    v6 = 450;
  v7 = 1800;
  sub_801C609C();
  sub_801C44E0(a1, 1);
  sub_8001A478(*(__int16 *)(a1 + 6));
  sub_8001A654(*(__int16 *)(a1 + 4));
  sub_80014344();
  sub_80024E98();
  sub_80024FC0(a2);
  sub_801C4FA0();
  sub_80024C84(0);
  sub_80020110(a2, 1, 2, 1);
  dword_801C954C = 0;
  do
  {
    if ( sub_8001A750() == 1 )
      break;
    if ( sub_801C6410 )
      sub_801C6410(a2, 0);
    sub_80035560(2);
    if ( sub_801C689C )
      sub_801C689C(a2);
    --v7;
  }
  while ( v7 > 0 );
  v8 = 0;
  sub_8001A280();
  v9 = 0;
  v10 = -17;
  v23 = 15;
  *(_DWORD *)(a2 + 12) = MEMORY[0x164];
  sub_80026FA4(v11);
  v22[0] = 0;
  *(_WORD *)(a2 + 96) = 1;
  *(_WORD *)(a2 + 84) = 0;
  do
  {
    v12 = -1851654144;
    if ( v10 > 0 )
    {
      v12 = -1851608123;
      *(_DWORD *)(a2 + 12) = 9792 * v10 / 1800;
    }
    if ( v8 || v10 < 50 )
    {
      if ( v8 == 1 && v10 >= 370 )
      {
        v8 = 2;
        sub_8001A694();
        v5 = v6;
        sub_80026EF8(dword_80094410);
        sub_80026ECC();
        sub_801C57E0(a2);
      }
    }
    else
    {
      v8 = 1;
      sub_80027664(v12);
    }
    v13 = *(_DWORD *)(a2 + 12) % 384;
    *(_WORD *)(a2 + 4) = (__int16)*(_DWORD *)(a2 + 12) / 384 + 1;
    *(_BYTE *)(a2 + 6) = (char)v13 / 96 + 1;
    *(_BYTE *)(a2 + 7) = v13 % 96 + 1;
    if ( !v8 )
    {
      v14 = sub_80035510();
      if ( !v14 || v9 == v14 )
      {
        v9 = v14;
        sub_80040370();
        sub_80027528(v16);
        *(_DWORD *)a2 = 0;
        sub_8002756C();
        sub_80035560(2);
        goto LABEL_45;
      }
      v10 = 370;
      sub_80027664(v15);
      sub_8001A694();
      sub_80040370();
      goto LABEL_28;
    }
    if ( v8 != 1 )
    {
      if ( v8 != 2 )
      {
        --v23;
        sub_801C6410(a2, 0);
        *(_DWORD *)a2 = 0;
        sub_8001E3E4(v22);
        goto LABEL_44;
      }
      sub_801C6410(a2, 0);
      *(_DWORD *)a2 = 0;
      sub_8001E3E4(v22);
      v19 = sub_80035510();
      v20 = v19;
      if ( v9 != v19 )
      {
        if ( v19 )
          v5 = v6;
        v4 = sub_801C47EC(v19, v22);
        v9 = v20;
        if ( !v4 )
          goto LABEL_41;
        v8 = 3;
      }
      v9 = v20;
LABEL_41:
      sub_80024FD0(a2);
      --v5;
      sub_801C5854(a2, v22[0]);
      if ( v5 <= 0 )
      {
        v8 = 3;
        v4 = 3;
      }
      goto LABEL_44;
    }
    v17 = sub_80035510();
    if ( !v17 || v9 == v17 )
    {
      v9 = v17;
      sub_801C6410(a2, 0);
      *(_DWORD *)a2 = 0;
      sub_80024FD0(a2);
      sub_801C5538((_DWORD *)a2);
LABEL_44:
      sub_80035560(2);
      sub_801C689C(a2);
      goto LABEL_45;
    }
    sub_8001A694();
LABEL_28:
    *(_DWORD *)a2 = 0x4000000;
    sub_801C6410(a2, 0);
    sub_80035560(2);
    sub_801C689C(a2);
    for ( i = 15; i > 0; --i )
      sub_80035560(2);
    sub_801C5AB4(a2);
    v8 = 2;
    *(_DWORD *)a2 |= 0x8000000u;
    sub_801C6410(a2, 0);
    sub_80035560(2);
    sub_801C689C(a2);
    while ( sub_80035510() )
      ;
    v5 = v6;
    sub_80026EF8(dword_80094410);
    sub_80026ECC();
LABEL_45:
    ++v10;
  }
  while ( v23 > 0 );
  return v4;
}

```

### Caller `801C4894` call-site windows

#### `801C4944 -> 80024C84`

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
```

## Target `80024CF8` `sub_80024CF8`

### Pseudocode

```c
PrGameContext_Psx *__fastcall sub_80024CF8(PrGameContext_Psx *a1)
{
  _DWORD *v1; // $gp
  int v3; // $v1
  int v4; // $v1
  int v5; // $a3
  unsigned __int16 *v6; // $a2
  unsigned __int16 *v7; // $a0
  PrGameContext_Psx *result; // $v0
  int v9; // $v0
  bool v10; // dc
  int v11; // $v0

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
  v5 = v1[223];
  if ( v5 < v1[219]
    && (v6 = (unsigned __int16 *)(v1[220] + 16 * v5),
        (*v6 << 16)
      + (*((unsigned __int8 *)v6 + 2) << 8)
      + *((unsigned __int8 *)v6 + 3)
      - ((*(unsigned __int16 *)&a1->_pad0[4] << 16)
       + (a1->_pad0[6] << 8)
       + a1->_pad0[7]) <= 0) )
  {
    v1[223] = v5 + 1;
    v7 = v6;
  }
  else
  {
    v7 = 0;
  }
  result = 0;
  if ( v7 )
  {
    v9 = (__int16)v7[word_800916D8 + 3];
    v10 = v9 <= 0;
    v11 = 4 * v9;
    if ( !v10 )
    {
      dword_8008ECE4 = *(_DWORD *)(v11 + v1[221]);
      word_8008ECFA = v7[2];
    }
    result = a1;
    a1->text268 = dword_8008ECE4;
  }
  return result;
}

```

### Direct xrefs

- `801C46A8 -> 80024CF8` type=17 caller=`801C455C sub_801C455C`

### Caller `801C455C` `sub_801C455C` pseudocode

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

### Caller `801C455C` call-site windows

#### `801C46A8 -> 80024CF8`

```asm
   801C4688: beqz    $v1, loc_801C46A8
   801C468C: li      $v0, 0x100
   801C4690: beq     $v1, $v0, loc_801C45A8
   801C4694: li      $v0, 0x800
   801C4698: beq     $v1, $v0, loc_801C472C
   801C469C: andi    $v0, $v1, 0x840
   801C46A0: bnez    $v0, loc_801C472C
   801C46A4: nop
=> 801C46A8: jal     sub_80024CF8
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
```



[result]
null
