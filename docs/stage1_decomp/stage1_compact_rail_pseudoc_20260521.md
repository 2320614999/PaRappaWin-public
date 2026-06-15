# Stage1 compact rail pseudo-C export 2026-05-21

Source IDB:

## 0x8001C550 sub_8001C550
- function range: 0x8001C550..0x8001C5A8
- xrefs_to: 0x8001F1D0, 0x8001F270, 0x8001F288, 0x8001F2A0, 0x8001F2B8, 0x8001F2D0, 0x8001F2E8, 0x8001F310, 0x8001F32C, 0x8001F344, 0x8001F35C, 0x8001F3B0, 0x8001F3C8, 0x8001F3E0, 0x8001F3F8, 0x8001F410, 0x8001F42C, 0x8001F444, 0x8001F45C, 0x8001F494, 0x8001F4AC, 0x8001F4C4, 0x8001F4DC, 0x8001F628, 0x8001FE44, 0x8001FF18, 0x8001FF30, 0x8001FF4C, 0x8001FF64, 0x800203F4, 0x80020414, 0x80020454
```c
int __fastcall sub_8001C550(__int16 a1, __int16 a2, int a3, unsigned __int16 a4)
{
  int v4; // $gp

  return sub_8001B590(a1, a2, a3, 0, 0, a4, (int)&unk_80087288 + 20 * *(_DWORD *)(v4 + 872));
}

```

## 0x8001C804 sub_8001C804
- function range: 0x8001C804..0x8001C864
- xrefs_to: 0x80024490
```c
int __fastcall sub_8001C804(int a1, _WORD *a2, __int16 a3, __int16 a4)
{
  int v5; // $gp

  sub_8001B338(a1, a2, a3, a4);
  return GsSortSprite(a1, (char *)&unk_80087288 + 20 * *(_DWORD *)(v5 + 872), 1);
}

```

## 0x80023F20 sub_80023F20
- function range: 0x80023F20..0x80024114
- xrefs_to: 0x80023B3C, 0x80023B78, 0x800247B4
```c
void __fastcall sub_80023F20(int a1)
{
  int *v2; // $s3
  unsigned int v3; // $s0
  __int16 *v4; // $s2
  __int16 *v5; // $s1
  int v6; // $v0

  if ( a1 > 0 )
  {
    v2 = dword_80087704;
    v3 = 0;
    v4 = word_800876B0;
    v5 = word_80087668;
    do
    {
      if ( *v2 < 24 )
      {
        if ( *v2 >= 6 )
        {
          if ( *v2 >= 22 )
          {
            *v5 = 4096;
            *v4 = 4096;
          }
          else
          {
            if ( *(int *)((char *)&unk_800876F8 + v3) >= 8193 )
              *(_DWORD *)((char *)&unk_800876F8 + v3) = 0;
            *v5 = rsin(*(_DWORD *)((char *)&unk_800876F8 + v3));
            *v4 = 4096;
            *(_DWORD *)((char *)&unk_800876F8 + v3) += 256;
          }
        }
        else
        {
          *v5 = *(_DWORD *)((char *)&unk_800876FC + v3) + 4096;
          *v4 = *(_DWORD *)((char *)&unk_800876FC + v3) + 4096;
          v6 = *(_DWORD *)((char *)&unk_800876FC + v3) + *(_DWORD *)((char *)&unk_80087700 + v3);
          *(_DWORD *)((char *)&unk_800876FC + v3) = v6;
          if ( v6 >= 4096 )
            *(_DWORD *)((char *)&unk_80087700 + v3) = -1024;
        }
        dword_80087704[v3 / 4] = *v2 + 1;
      }
      v2 += 4;
      v3 += 16;
      ++v4;
      ++v5;
    }
    while ( (int)v2 < (int)&dword_80087704[4 * a1] );
  }
}

```

## 0x80024114 sub_80024114
- function range: 0x80024114..0x80024308
- xrefs_to: 0x80024868
```c
void __fastcall sub_80024114(int a1)
{
  int *v2; // $s3
  unsigned int v3; // $s0
  __int16 *v4; // $s2
  __int16 *v5; // $s1
  int v6; // $v0

  if ( a1 > 0 )
  {
    v2 = dword_80087944;
    v3 = 0;
    v4 = word_800876B0;
    v5 = word_80087668;
    do
    {
      if ( *v2 < 24 )
      {
        if ( *v2 >= 5 )
        {
          if ( *v2 >= 22 )
          {
            *v5 = 4096;
            *v4 = 4096;
          }
          else
          {
            if ( *(int *)((char *)&unk_80087938 + v3) >= 8193 )
              *(_DWORD *)((char *)&unk_80087938 + v3) = 0;
            *v5 = rsin(*(_DWORD *)((char *)&unk_80087938 + v3));
            *v4 = 4096;
            *(_DWORD *)((char *)&unk_80087938 + v3) += 256;
          }
        }
        else
        {
          v6 = *(_DWORD *)((char *)&unk_8008793C + v3) + *(_DWORD *)((char *)&unk_80087940 + v3);
          *(_DWORD *)((char *)&unk_8008793C + v3) = v6;
          *v5 = v6 + 4096;
          *v4 = *(_DWORD *)((char *)&unk_8008793C + v3) + 4096;
          if ( *(int *)((char *)&unk_8008793C + v3) >= 4096 )
            *(_DWORD *)((char *)&unk_80087940 + v3) = -1024;
        }
        dword_80087944[v3 / 4] = *v2 + 1;
      }
      v2 += 4;
      v3 += 16;
      ++v4;
      ++v5;
    }
    while ( (int)v2 < (int)&dword_80087944[4 * a1] );
  }
}

```

## 0x80024308 sub_80024308
- function range: 0x80024308..0x80024390
- xrefs_to: 0x80027864, 0x8002796C, 0x800279C0, 0x80027CC4, 0x80027D18, 0x80027D80
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

## 0x80024390 sub_80024390
- function range: 0x80024390..0x80024418
- xrefs_to: <none>
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

## 0x80024418 sub_80024418
- function range: 0x80024418..0x800244A8
- xrefs_to: 0x80023BF8, 0x80024AA4
```c
int __fastcall sub_80024418(_WORD *a1)
{
  int v2; // [sp+10h] [-28h] BYREF
  __int16 v3; // [sp+14h] [-24h]
  __int16 v4; // [sp+16h] [-22h]
  int v5; // [sp+30h] [-8h]

  v5 = 0;
  v2 = 1342177344;
  v3 = *a1 - 160;
  v4 = a1[1] - 120;
  return ((int (__fastcall *)(int *, int, _DWORD, _DWORD))sub_8001C804)(
           &v2,
           dword_800540BC[(__int16)a1[3]],
           word_80087668[(__int16)a1[2]],
           word_800876B0[(__int16)a1[2]]);
}

```

## 0x80024744 sub_80024744
- function range: 0x80024744..0x80024B54
- xrefs_to: <none>
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
