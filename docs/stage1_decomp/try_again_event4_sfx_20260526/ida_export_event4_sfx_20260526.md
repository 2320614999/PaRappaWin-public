# Stage1 Try Again event4 SFX exports - 2026-05-26

## Scope

- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Symptom: Try Again screen is visually accepted, but multiple sound effects are
  missing.
- Authority: local IDA Hex-Rays pseudo-C exported on 2026-05-26.

## `800267C8` event4 init

```c
int __fastcall sub_800267C8(int a1)
{
  int v1; // $gp
  _DWORD *v2; // $v1
  int result; // $v0

  v2 = *(_DWORD **)(a1 + 16);
  *(_DWORD *)(v1 + 812) = 2;
  result = -1;
  *(_DWORD *)(v1 + 808) = 0;
  *v2 = -1;
  return result;
}
```

Mapping:

- `gp+812` = `gp+0x32C`: event4 cue repeat count, initialized to `2`.
- `gp+808` = `gp+0x328`: event4 cue frame counter, initialized to `0`.
- `*ctx0 = -1`: no selected prompt choice yet.

## `80025E6C` event4 cue tick

```c
int sub_80025E6C()
{
  int v0; // $gp
  int result; // $v0
  int v2; // $v1
  int v3; // $v1

  result = *(_DWORD *)(v0 + 812);
  if ( result > 0 )
  {
    v2 = *(_DWORD *)(v0 + 808);
    if ( v2 )
    {
      if ( v2 == 36 )
      {
        sub_80026EF8(dword_8009441C + 6);
        sub_80026ECC();
        --*(_DWORD *)(v0 + 812);
      }
    }
    else
    {
      sub_80026EF8(dword_8009441C);
      sub_80026ECC();
    }
    v3 = *(_DWORD *)(v0 + 808);
    result = v3 + 1;
    if ( v3 == 72 )
      *(_DWORD *)(v0 + 808) = 0;
    else
      *(_DWORD *)(v0 + 808) = result;
  }
  return result;
}
```

Mapping:

- Frame `0`: play `dword_8009441C`, then `80026ECC`.
- Frame `36`: play `dword_8009441C + 6`, then `80026ECC`, then decrement
  `gp+0x32C`.
- Frame `72`: reset `gp+0x328` to `0`.
- The two-cue cycle repeats until `gp+0x32C <= 0`.

Stage1 cue table evidence from earlier exports:

- `dword_8009441C = &unk_801C6EC4`.
- `unk_801C6EC4`: `{00,0A,22,5A,00,00}`.
- `unk_801C6ECA`: `{00,0B,23,5A,00,00}`.

## `80025F0C` event4 input handler

```c
int __fastcall sub_80025F0C(int a1, _DWORD *a2)
{
  int result; // $v0

  if ( a1 == 64 )
  {
    sub_80025C8C(32);
    *a2 = 0;
    return 1;
  }
  else
  {
    result = 0;
    if ( a1 == 32 )
    {
      sub_80025C8C(64);
      *a2 = 1;
      return 2;
    }
  }
  return result;
}
```

Mapping:

- Cross (`0x40`) calls `80025C8C(0x20)`, writes prompt ctx `0`, returns `1`.
- Circle (`0x20`) calls `80025C8C(0x40)`, writes prompt ctx `1`, returns `2`.

## `80025C8C` input cue dispatcher

```c
int __fastcall sub_80025C8C(unsigned int a1)
{
  int result; // $v0
  unsigned __int8 *v2; // $a0

  if ( !a1 )
    return result;
  if ( a1 == 4096 )
    goto LABEL_19;
  if ( a1 >= 0x1001 )
  {
    if ( a1 != 0x4000 )
    {
      result = 0x2000;
      if ( a1 >= 0x4001 )
      {
        result = 0x8000;
        if ( a1 != 0x8000 )
          return result;
      }
      else if ( a1 != 0x2000 )
      {
        return result;
      }
    }
LABEL_19:
    v2 = (unsigned __int8 *)dword_80094420;
    goto LABEL_20;
  }
  if ( a1 == 64 )
  {
    v2 = (unsigned __int8 *)dword_80094424;
    goto LABEL_20;
  }
  result = 32;
  if ( a1 < 0x41 )
  {
    if ( a1 != 32 )
      return result;
    v2 = (unsigned __int8 *)dword_80094428;
LABEL_20:
    sub_80026EF8(v2);
    return sub_80026ECC();
  }
  result = 256;
  if ( a1 == 256 )
  {
    v2 = (unsigned __int8 *)dword_8009442C;
    goto LABEL_20;
  }
  return result;
}
```

Mapping for event4:

- `0x20 -> dword_80094428`, then `80026EF8` and `80026ECC`.
- `0x40 -> dword_80094424`, then `80026EF8` and `80026ECC`.

## Win mismatch closed

Before this pass, `DispatcherInit4Ex` reset only prompt/frame state,
`DispatcherHandle4Ex` wrote the choice without `80025C8C`, and
`DispatcherTick4Ex` drew the event frame without `80025E6C`.

The 2026-05-26 patch adds the missing direct side effects inside event4:

- `ResetStage1Event4CueTick800267C8()`
- `TickStage1Event4Cue80025E6C()`
- exact Cross/Circle `80025C8C(0x20/0x40)` cue dispatch

No renderer/UI drawing or Stage1 retry/gameplay logic changed.
