# IDA export: Stage1 GOOD->COOL pair-bonus helper 80014A80

- Date: 2026-05-27
- Source: local IDA MCP Hex-Rays decompile
- Scope: supplemental evidence for `80014D58 -> 80014A80` scorer audit

## 0x80014A80 `sub_80014A80`

```c
int __fastcall sub_80014A80(int a1, int a2, int a3)
{
  int v3; // $t4
  int result; // $v0
  int v5; // $t0
  int v6; // $t1
  int v7; // $a3
  int v8; // $v1
  int v9; // $t2
  int v10; // $a0
  int v11; // $t3
  int v12; // $v1
  int v13; // $v0

  v3 = 0;
  if ( !word_8009182A )
    return 0;
  v5 = 0;
  v6 = 0;
  v7 = 0;
  v8 = 0;
  v9 = a2;
  if ( a2 < a3 )
  {
    v10 = 12 * a2 + a1;
    do
    {
      if ( *(_WORD *)(v10 + 6) )
      {
        if ( *(_WORD *)(v10 + 18) )
        {
          v8 |= 8u;
          ++v7;
        }
        else
        {
          v8 |= 4u;
          ++v6;
        }
      }
      else if ( *(_WORD *)(v10 + 18) )
      {
        v8 |= 2u;
        ++v5;
      }
      else
      {
        ++v3;
      }
      v9 += 2;
      v10 += 24;
    }
    while ( v9 < a3 );
  }
  if ( v8 == 14 )
  {
    v11 = 15 * v5 + 6 * v6 + 9 * v7;
    if ( !a2 )
    {
      result = 15 * v5 + 6 * v6 + 9 * v7;
      if ( v3 <= 0 )
        return result;
      v11 += 18;
    }
    return v11;
  }
  if ( v8 == 12 )
  {
    v12 = 9 * v7;
LABEL_23:
    v13 = 6 * v6;
    return v12 + v13;
  }
  if ( v8 != 10 )
  {
    result = 0;
    if ( v8 != 6 )
      return result;
    v12 = 15 * v5;
    goto LABEL_23;
  }
  v12 = 9 * v7;
  v13 = 15 * v5;
  return v12 + v13;
}
```
