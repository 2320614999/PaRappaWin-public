# 2026-05-10 current-IDB decompile: event4 tail follow-up

Source: current IDA database `SCUS_941.83`.

## `80040CA4`

```c
int __fastcall sub_80040CA4(int a1)
{
  return sub_800450A0(*(_DWORD *)(a1 + 16));
}
```

## `800356A8`

```c
int __fastcall sub_800356A8(int a1, int a2)
{
  int result;
  int v3;

  v3 = a2 << 15;
  result = a1 > 0;
  if ( a1 > 0 )
  {
    do
      --v3;
    while ( v3 != -1 );
    sub_80047F4C("VSync: timeout\n");
    ChangeClearPAD(0);
    return ChangeClearRCnt(3, 0);
  }
  return result;
}
```

## `800436F0`

```c
int sub_800436F0()
{
  return 0;
}
```

## `800450A0`

```c
int __fastcall sub_800450A0(int a1)
{
  return sub_800468E0(sub_80046840, a1, 0, 0);
}
```

## `8001B6C4`

```c
int __fastcall sub_8001B6C4(__int16 a1, __int16 a2, __int16 a3,
                            __int16 a4, int a5, unsigned __int16 a6,
                            int a7)
{
  unsigned int v8;
  __int16 v9;
  __int16 v10;
  __int16 v11;
  __int16 v12;
  char v13;
  char v14;
  char v15;

  v9 = a1 - 160;
  v10 = a2 - 120;
  v11 = a3;
  v12 = a4;
  v8 = a5 & 0xFF000000;
  v13 = BYTE2(a5);
  v14 = BYTE1(a5);
  v15 = a5;
  return GsSortBoxFill(&v8, a7, a6);
}
```

## `8001B120`

```c
int __fastcall sub_8001B120(int a1)
{
  int v1;
  int v2;
  int v4;
  int v6;
  int v7;

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

Notes:
- `80040CA4` still depends on `800450A0` and the work-list word at offset `+16`.
- `800450A0` continues into `800468E0(sub_80046840, head, 0, 0)`.
- `8001B6C4` and `8001B120` both submit GPU work; they are not safe no-ops.
- `800436F0` remains a decompiler-degraded stub here; do not treat it as proof
  that the full helper has no side effects without disassembly-level review.
