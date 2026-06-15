# Stage1 Try Again retry evidence - 2026-05-25

## Scope

- Target branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Runtime target at audit start: `f5ccd110`
- Symptom: after Try Again YES, Stage1 restart lacks the character-grid/frozen-frame transition, and Try Again can keep interrupting gameplay.
- Authority: local IDA MCP Hex-Rays pseudo-C plus existing `801C81EC` pseudo-C.

## `801C81EC` Try Again branch

Source: `docs/stage1_decomp/comod_status_coupled_pseudocode_20260511/S1_COMOD1/decompile_801c81ec.txt`.

Relevant branch:

```c
LABEL_5:
  MEMORY[0x8001A4D0](MEMORY[0x8006EDB8] + 156, 0);
  v3 = PrStageRunner_Run(MEMORY[0x8006EDB8] + 156, (int *)0x801C3640, a1);
  ...
  if (v3 == 1)
    break;
  if (MEMORY[0x80026B94](4, 0) == 2)
  {
    MEMORY[0x80020110](-2145634752, 2, 1, 2);
    MEMORY[0x8001EF14]();
    v2 = 2;
    goto LABEL_26;
  }
  MEMORY[0x8001EF14]();
```

Interpretation:

- Event4 result `2` is the abort/exit path.
- Any non-`2` result executes `8001EF14()` and loops back to `LABEL_5`.
- The next retry starts through the normal `8001A4D0(sceneEntry+0x9C,0)` + `PrStageRunner_Run(...)` entry.

## `8001EF14`

IDA MCP `decompile_function(0x8001EF14)`:

```c
int sub_8001EF14()
{
  _DWORD *v0; // $gp

  v0[51] = 0;
  v0[49] = 0;
  v0[50] = 0;
  return sub_8001EEAC(0);
}
```

## `8001EEAC`

IDA MCP `decompile_function(0x8001EEAC)`:

```c
BOOL __fastcall sub_8001EEAC(int a1)
{
  int v1; // $a2
  char *v2; // $a1
  int v3; // $v1
  _DWORD *v4; // $v0
  BOOL result; // $v0

  v1 = 0;
  v2 = (char *)&unk_80087330;
  do
  {
    v3 = 15;
    v4 = v2 + 60;
    do
    {
      *v4 = a1;
      --v3;
      --v4;
    }
    while ( v3 >= 0 );
    result = ++v1 < 12;
    v2 += 64;
  }
  while ( v1 < 12 );
  return result;
}
```

## `8001A4D0`

IDA MCP `decompile_function(0x8001A4D0)` confirms the stage record tick path used at retry entry:

```c
int __fastcall sub_8001A4D0(int a1, int a2)
{
  int result; // $v0
  __int16 v5; // $v0
  int v6; // $s0
  int v7; // $v0

  result = *(_DWORD *)a1;
  if ( *(_DWORD *)a1 )
  {
    dword_800493EC = *(_DWORD *)(a1 + 16);
    dword_800493FC = sub_80036A78((unsigned __int8 *)&dword_800493EC);
    sub_8001A478(*(__int16 *)(a1 + 6));
    do
    {
      do
      {
        while ( !sub_800367A4(2, &dword_800493EC, dword_80049414) )
          ;
      }
      while ( sub_800364D0(0, 0) != 2 );
      byte_8004940C = 1;
      v5 = *(_WORD *)(a1 + 4);
      word_8004940E = 0;
      byte_8004940D = v5;
      do
      {
        while ( !sub_800367A4(13, &byte_8004940C, dword_80049414) )
          ;
      }
      while ( sub_800364D0(0, 0) != 2 );
      v6 = 72;
      if ( a2 == 1 )
      {
        v6 = 456;
        v7 = 16;
      }
      else
      {
        v7 = 4;
      }
      dword_80049424 = v7;
      while ( !sub_800391AC(v6) )
        ;
      sub_80035560(3);
    }
    while ( sub_800364D0(0, 0) != 2 );
    dword_80049410 = 1;
    dword_80049420 = 0;
    return sub_80036678(1, 0);
  }
  return result;
}
```

