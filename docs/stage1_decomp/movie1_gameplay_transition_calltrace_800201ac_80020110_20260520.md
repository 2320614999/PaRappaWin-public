# Movie1 -> Gameplay transition call_trace evidence (2026-05-20)

## Scope

- User-triggered stop-loss: no symptom guessing before IDA export, replay frame
  lookup, PSX call_trace, and Win-vs-PSX call order comparison.
- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec`

## IDA MCP pseudocode

### 800201AC

```c
int __fastcall sub_800201AC(int a1, int a2, int a3, int a4)
{
  int v7; // $a0
  int v8; // $v0
  int v9; // $gp

  sub_8001FFD4(a3);
  v7 = a1;
  do
  {
    sub_8001EA74(v7, a2);
    sub_80035560(2);
    sub_8001EBF4();
    v8 = sub_8001F518();
    v7 = a1;
  }
  while ( v8 );
  sub_8001FFD4(a4);
  *(_DWORD *)(v9 + 196) = 190;
  return sub_80020090(a1, a2, sub_8001EA74, sub_8001EBF4);
}
```

### 80020110

```c
int __fastcall sub_80020110(int a1, int a2, int a3, int a4)
{
  int v7; // $a0
  int v8; // $v0

  sub_8001FFD4(a3);
  v7 = a1;
  do
  {
    sub_8001EA74(v7, a2);
    sub_80027194(30);
    sub_80035560(2);
    sub_8001EBF4(a1);
    v8 = sub_8001F518();
    v7 = a1;
  }
  while ( v8 );
  sub_8001FFD4(a4);
  return sub_80020008(a1, a2, sub_8001EA74, sub_8001EBF4);
}
```

## Replay frame facts

- Input edge: Movie1 skip press at frame 1526, release at frame 1534.
- Post-movie transition starts at frame 1570.
- Post `800201AC` tail enters `80020090` at frame 1631.
- `8001A4D0` is called at frame 1639.
- `801C7A60` is called at frame 1642.

## PSX call_trace sequence

```text
FRAME 1570
  seq=23425 pc=0x801C82B0 -> 80026FA4
  seq=23553 pc=0x801C82C4 -> 800201AC
  seq=23559 pc=0x800201CC -> 8001FFD4
  seq=23760 pc=0x800201D8 -> 8001EA74

FRAME 1571
  seq= 1800 pc=0x800201E8 -> 8001EBF4
  seq= 2346 pc=0x800201F0 -> 8001F518
  seq= 2351 pc=0x800201D8 -> 8001EA74

FRAME 1631
  seq= 1784 pc=0x800201E8 -> 8001EBF4
  seq= 3075 pc=0x800201F0 -> 8001F518
  seq= 3079 pc=0x80020200 -> 8001FFD4
  seq= 3478 pc=0x80020224 -> 80020090
  seq= 3484 pc=0x800200C4 -> 8001EA74

FRAME 1639
  seq= 1782 pc=0x800200D4 -> 8001EBF4
  seq= 3076 pc=0x801C82D8 -> 8001A4D0

FRAME 1642
  seq= 1895 pc=0x801C82F0 -> 801C7A60
  seq= 1905 pc=0x801C7A00 -> 80026FA4
```

Dedicated filtering of frames 1570, 1631, 1639, 1640, 1641, and 1642 found no
calls to `801C4894`, `801C4FA0`, `80024C84`, or `80020110` in the
Movie1-skip -> gameplay handoff window.

## Win comparison

PSX observed sequence:

```text
80026FA4
800201AC(&801C3640,5,1,2)
8001A4D0(sceneEntry+0x9C,0)
801C7A60(sceneEntry+0x9C,&801C3640,sceneId)
```

Win direct lifecycle before this fix:

```text
800201AC(&801C3640,5,1,2)
8001A4D0(sceneEntry+0x9C,0)
80020110(&801C3640,1,2,1)
801C7A60(sceneEntry+0x9C,&801C3640,sceneId)
```

Differences:

- Missing post-playwait `80026FA4` before `800201AC(5,1,2)`.
- Extra first StageLoop `80020110(1,2,1)` between `8001A4D0` and `801C7A60`.

## Runtime change

- `Step801C81EC` now emits `AudioReset26FA4` before post-Movie1
  `Transition201AC(5,1,2)`.
- `EmitStageLoopRequest` now emits only
  `StageRecordTick1A4D0 -> StageRunnerRun7A60`.
- No `pr_scenes.cpp` change; no observer/sentinel/matcher/harness/parser
  expansion.
