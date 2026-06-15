# MOVSEG core pseudo C export - 2026-05-10

Source: IDA MCP `decompile_function` on the active database.

This note records the direct-port authority for the generic MOVSEG carrier split.
The current IDB decompiler loses the scene-entry base in `801C4780`; existing raw
exports and disassembly show the loop is `sceneEntry+0x0C+i*0x30`, not absolute
address `0x0C`.

## `80036A78`

```c
int __fastcall sub_80036A78(unsigned __int8 *a1)
{
  return 75 * (60 * (10 * (*a1 >> 4) + (*a1 & 0xF)) + 10 * (a1[1] >> 4) + (a1[1] & 0xF))
       + 10 * (a1[2] >> 4)
       + (a1[2] & 0xF)
       - 150;
}
```

## `8001A2B0`

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

## `800381F8`

IDA warning: Hex-Rays reports possible write access to const memory. The useful
authority here is the visible writeback: when a path match succeeds, six words
from the directory entry are copied into the caller-provided `CdlFILE`.

```c
char *__fastcall sub_800381F8(_DWORD *a1, unsigned __int8 *a2)
{
  bool v4; // dc
  char *result; // $v0
  int v6; // $a0
  char *v7; // $s0
  int v8; // $s2
  int v9; // $v0
  _BYTE *v10; // $s1
  int v11; // $v0
  BOOL v12; // $v0
  char v13; // $v0
  int v14; // $s2
  char *v15; // $s0
  __int16 *v16; // $s3
  int i; // $s1
  int v18; // $v1
  int v19; // $a0
  int v20; // $a1
  int v21; // $v1
  _BYTE v22[32]; // [sp+10h] [-20h] BYREF

  v4 = sub_800384FC() == 0;
  result = 0;
  if ( !v4 )
  {
    dword_80057408 = 0;
    v6 = 1;
    if ( *a2 != 92 )
      return 0;
    v22[0] = 0;
    v7 = (char *)a2;
    v8 = 0;
    while ( 1 )
    {
      v9 = (unsigned __int8)*v7;
      v10 = v22;
      if ( v9 != 92 )
        break;
LABEL_11:
      v4 = v9 == 0;
      v12 = v8 < 8;
      if ( v4 )
        goto LABEL_15;
      ++v7;
      *v10 = 0;
      v6 = sub_800387D8(v6, v22);
      if ( v6 == -1 )
      {
        v22[0] = 0;
LABEL_14:
        v12 = v8 < 8;
        goto LABEL_15;
      }
      if ( ++v8 >= 8 )
        goto LABEL_14;
    }
    v11 = (unsigned __int8)*v7;
    while ( 1 )
    {
      v4 = v11 == 0;
      v12 = v8 < 8;
      if ( v4 )
        break;
      v13 = *v7++;
      *v10 = v13;
      v11 = (unsigned __int8)*v7;
      ++v10;
      if ( v11 == 92 )
      {
        v9 = (unsigned __int8)*v7;
        goto LABEL_11;
      }
    }
LABEL_15:
    if ( v12 && v22[0] )
    {
      *v10 = 0;
      if ( !sub_80038880(v6) )
        return 0;
      v14 = 0;
      v15 = &byte_80088320;
      v16 = word_80088328;
      for ( i = 0; LOBYTE(word_80088328[i]); i += 12 )
      {
        if ( sub_800384D8(v16, v22) )
        {
          v18 = *((_DWORD *)v15 + 1);
          v19 = *((_DWORD *)v15 + 2);
          v20 = *((_DWORD *)v15 + 3);
          *a1 = *(_DWORD *)v15;
          a1[1] = v18;
          a1[2] = v19;
          a1[3] = v20;
          v21 = *((_DWORD *)v15 + 5);
          a1[4] = *((_DWORD *)v15 + 4);
          a1[5] = v21;
          return v15;
        }
        v15 += 24;
        v16 += 12;
        if ( ++v14 >= 64 )
          return 0;
      }
    }
    return 0;
  }
  return result;
}
```

Porting note: the generic MOVSEG layer should receive this as explicit
`CdlFILE` feedback. Directory traversal, path matching and host I/O remain lower
CD/HAL adapter concerns.

## `8001A324`

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

## `80025A00`

```c
int *sub_80025A00()
{
  _DWORD *v0; // $gp
  int *result; // $v0

  v0[224] = &unk_800965B0;
  v0[226] = &unk_800965B0;
  result = &dword_801C35B0;
  v0[225] = &dword_801C35B0;
  v0[81] = 0;
  dword_80091858 = 0;
  return result;
}
```

## `8001AC18`

```c
int __fastcall sub_8001AC18(_DWORD *a1, int a2)
{
  int v4; // $s3
  int v5; // $s1
  int i; // $s0
  int v7; // $v1
  int v8; // $a0
  int v9; // $a1
  int v10; // $v1
  int result; // $v0
  _DWORD v12[6]; // [sp+10h] [-18h] BYREF

  v4 = 1;
  v5 = 0;
  for ( i = 0; i < 4; ++i )
  {
    if ( i > 0 )
      v4 = 0;
    sub_80025A34();
    if ( sub_8001A324(a1) >= 0 )
    {
      v7 = a1[5];
      v8 = a1[6];
      v9 = a1[7];
      v12[0] = a1[4];
      v12[1] = v7;
      v12[2] = v8;
      v12[3] = v9;
      v10 = a1[9];
      v12[4] = a1[8];
      v12[5] = v10;
      result = ((int (__fastcall *)(_DWORD *, int, int))sub_8001A8F0)(v12, v4, a2);
      v5 = result;
      if ( result == 1 )
        break;
    }
    result = v5;
  }
  return result;
}
```

## `801C4780`

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

Porting note: use `sceneEntry+0x0C` for the loop base and `sceneEntry+0x3C`
for the final `8001AC18` action. Do not encode the decompiler's lost base as
absolute `0x0C/0x3C`.

## `801C4DC4`

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

Porting note: `108 == sceneEntry+0x6C` movie segment row 2 and
`156 == sceneEntry+0x9C` stage runtime row 3 in the surrounding lifecycle group.
