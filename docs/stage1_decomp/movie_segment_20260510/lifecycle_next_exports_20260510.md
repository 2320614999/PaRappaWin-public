# MOVSEG lifecycle next exports

Scope: static export only. No `src` edits, no build, no test.

## Inputs checked

- Current IDA MCP IDB: `E:\game\PS模拟器\parappa the rapper\SCUS_941.83`
- Overlay IDB used for Stage1 lifecycle bodies:
  `E:\game\PSgame\parappa the rapper\S1\COMOD1.BIN.i64`
- New IDA MCP output directory:
  `docs/stage1_decomp/movie_segment_20260510/ida_comod1_lifecycle_exports_20260510/`

## Export results

| Address | Current SCUS IDB state | Overlay IDB state | Output |
|---|---|---|---|
| `800154B0` | function `sub_800154B0`, decompiles cleanly | N/A | summarized below |
| `801C7284` | `loc_801C7284`, current SCUS IDB decompile fails | function `PrScene1_Init`, dumped | `ida_comod1_lifecycle_exports_20260510/decompile_801c7284.txt` |
| `801C81EC` | `loc_801C81EC`, current SCUS IDB decompile fails | function `PrScene1_RunMovie1_AndStage1`, dumped | `ida_comod1_lifecycle_exports_20260510/decompile_801c81ec.txt` |

IDA MCP async job status polling returned `Unknown IDAPython job:
for-file-job-1`, but the job still wrote the expected products. The manifest
reports both requested addresses as `dumped`.

## `800154B0` main-program wrapper

Current IDA pseudocode:

```c
int __fastcall sub_800154B0(int a1)
{
  return sub_8001ACF8(a1, dword_801C3870);
}
```

Relevant disassembly:

```text
800154C8  lw      $a0, 0x10+arg_0($fp)
800154CC  lw      $a1, off_80010000
800154D4  jal     sub_8001ACF8
800154D8  nop
```

The caller-side implication for `80015D18` is narrow: after
`8001A324(sceneEntry+0x0C)`, `800154B0(sceneEntry+0x0C, 0)` reloads/copies the
row-0 overlay segment into the overlay destination from `off_80010000`
(rendered by Hex-Rays as `dword_801C3870`). It does not construct the seven
MOVSEG rows and does not change their static source boundary.

## `801C7284` / `PrScene1_Init`

Overlay IDB pseudocode confirms the Stage1 entry-init role:

```text
MEMORY[0x80025A34](a1, a2, a3, a4)
PrStageRunner_ResetState()
MEMORY[0x80024E98]()
MEMORY[0x80014344]()
MEMORY[0x8006EDB8] = a1
sceneEntry+0x168 = 16
sceneEntry+0x15C = 96 * *(s16 *)(sceneEntry+6) / 100
sceneEntry+0x164 = *(s16 *)(sceneEntry+8) + *(s16 *)(sceneEntry+10)
sceneEntry+0x160 = (96 * *(s16 *)(sceneEntry+6) / 3600 + 50) / 100
for rows 0..6: 8001A324(MEMORY[0x8006EDB8] + 0x0C + row*0x30)
80025A00()
8001EF14()
return 80015660(sceneIndex, word_801CCBBC[sceneIndex], 1)
```

This means Stage1 repeats the same seven-row loader pass seen in `801C4780`,
but inside the scene-1 init function. The row bytes still come from
`sceneEntry = 0x8005474C + sceneIndex * 0x16C`; `801C7284` caches the selected
entry in `0x8006EDB8`, derives scene timing fields, calls `8001A324` on each
row, and then starts the next lifecycle resource/transition helper.

## `801C81EC` / `PrScene1_RunMovie1_AndStage1`

Overlay IDB pseudocode confirms the Stage1 main lifecycle role:

```text
PrStrPlayer_ConfigMovieViewport()
80026FA4()
if 800916D0 is not 1 or 2:
  800201AC(0x801C3640, 6, 2, 1)
  PrStrPlayer_Init(MEMORY[0x8006EDB8] + 0x6C, 0)
  PrStrPlayer_PlayAndWait(MEMORY[0x8006EDB8] + 0x6C, 0x801C3640, 0)
  80026FA4()
  800201AC(0x801C3640, 5, 1, 2)
  if movie result is 1: set 800916E0 = 3; return -1

loop:
  8001A4D0(MEMORY[0x8006EDB8] + 0x9C, 0)
  result = PrStageRunner_Run(MEMORY[0x8006EDB8] + 0x9C, 0x801C3640, sceneIndex)
  handle 800916D0 == 1 / 2 exits
  if result != 1 and 80026B94(4,0) == 2: set 800916E0 = 2; return -1
  if result != 1: 8001EF14(); continue

after stage runner returns 1:
  choose clear-tail row at +0xFC or +0xCC based on 0x801C368E
  PrStrPlayer_Init(chosenRow, 0)
  800201AC(0x801C3640, 6, 2, 1)
  PrStrPlayer_PlayAndWait(chosenRow, 0x801C3640, mode 1 or 2)
  800201AC(0x801C3640, 5, 1, 2)
  audio flush / SFX / result update / optional 80015590 + 80019148
```

MOVSEG row consumption in this function is therefore explicit:

- `sceneEntry+0x6C` is row 2, initial `\SS\MOVIE1.STR;1`.
- `sceneEntry+0x9C` is row 3, `\S1\STAGE1.XA1;1`, passed to the stage runner setup path.
- `sceneEntry+0xCC` is row 4, `\S1\XMOVIE1.STR;1`, one clear-tail branch.
- `sceneEntry+0xFC` is row 5, `\S1\XMOVIE1.STR;1`, alternate clear-tail branch.

`801C81EC` consumes the already materialized rows and controls blocking movie /
stage lifecycle sequencing. It does not own the row table source.

## Boundary correction note

Do not use the current SCUS main IDB bytes at `801C7284` or `801C81EC` as
function bodies. In that IDB:

- `801C7284` is named `loc_801C7284`; `decompile_function` fails with
  `No function found at address 0x801c7284 and failed to create one automatically`.
- `801C81EC` is named `loc_801C81EC`; `decompile_function` fails with
  `No function found at address 0x801c81ec and failed to create one automatically`.
- The only SCUS xrefs are the lifecycle table entries:
  `80048D38 -> 801C7284` and `80048D3C -> 801C81EC`.

For direct-port work, use the overlay IDB/function boundaries:

- `801C7284` should be treated as `PrScene1_Init`.
- `801C81EC-801C858C` should be treated as
  `PrScene1_RunMovie1_AndStage1`.

If editing the current SCUS IDB later, the safe fix is not to infer bodies from
the main-program image; it is to load/apply the COMOD1 overlay at base
`0x801C3870` and then define/import these overlay function boundaries.

## Source-boundary conclusion

The next exports reinforce the previous row-source conclusion:

```text
SCUS scene table owns row bytes:
  0x8005474C + sceneIndex * 0x16C + 0x0C + row*0x30

800154B0:
  copies/loads row 0 overlay payload through 8001ACF8

801C7284:
  scene-1 init; caches sceneEntry, derives timing fields, calls 8001A324 on rows 0..6

801C81EC:
  scene-1 main lifecycle; consumes rows 2/3/4/5 for MOVIE1, stage runner, and clear-tail STR blocks
```

So the MOVSEG/lifecycle boundary should stay table-driven: static row bytes
come from SCUS scene entries, loader functions materialize CD/LBA fields, and
the Stage1 lifecycle functions only initialize and consume those rows.
