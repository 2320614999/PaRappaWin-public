# Stage1 v3 ctx52/replay append boundary (2026-05-17)

Scope: Stage1 accepted page-history source/timing only. This note closes the
`801C8660` replay-table suspicion for the latest v3 recording. It does not
change runtime/source code, harness input semantics, renderer, lifecycle,
lower-CD, Rail/GTE, case17, row/v22 tables, or scorer math.

Recording:

```text
E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec
```

## Commands

```powershell
python .\tools\模拟器内存回放器.py "<v3.psxrec>" export_windows 5396 5486 artifacts\psx_row3_v3_ctx52_replay_counts_5396_5486.csv --format csv --step 1 --window ctx@0x801C3640:112 --window replayCounts@0x800901BC:16 --window replayTicks@0x8008EEF8:128 --window replayMasks@0x8008EEFC:128
python .\tools\模拟器内存回放器.py "<v3.psxrec>" writers 0x800901BC 4 5396 5486
python .\tools\模拟器内存回放器.py "<v3.psxrec>" writers 0x800901C0 4 5396 5486
python .\tools\模拟器内存回放器.py "<v3.psxrec>" export_windows 5398 5480 artifacts\psx_row3_v3_ctx52_replay_append_slots_5398_5480.csv --format csv --step 1 --window ctx@0x801C3640:112 --window replayCounts@0x800901BC:16 --window replayLane@0x8008EEF8:224
```

Artifacts:

```text
artifacts\psx_row3_v3_ctx52_replay_counts_5396_5486.csv
artifacts\psx_row3_v3_ctx52_replay_append_slots_5398_5480.csv
```

## Writer facts

`dword_800901BC` writers:

```text
Frame 5399  PC=0x80014994  0x00000014 -> 0x00000015
Frame 5429  PC=0x80014994  0x00000015 -> 0x00000016
Frame 5449  PC=0x80014994  0x00000016 -> 0x00000017
Frame 5479  PC=0x80014994  0x00000017 -> 0x00000018
```

`dword_800901C0` writers:

```text
Frame 5399  PC=0x8001498C  0x00000014 -> 0x00000015
Frame 5429  PC=0x8001498C  0x00000015 -> 0x00000016
Frame 5449  PC=0x8001498C  0x00000016 -> 0x00000017
Frame 5479  PC=0x8001498C  0x00000017 -> 0x00000018
```

Both writer PCs are inside `80014614` replay-log append. No `801C4FC8` /
`801C8660` table rebuild writer appears in this window.

## Frame facts

| frame | ctx+0x52 | ctx+0x0C | ctx+0x10 | ctx+0x18 | ctx+0x20 | 901BC | 901C0 | appended slot |
|---:|---:|---:|---:|---:|---:|---:|---:|---|
| 5398 | 0 | 10752 | 9408 | 0 | 0 | 20 | 20 | none |
| 5399 | 0 | 10755 | 10755 | 0x10 | 1 | 21 | 21 | slot20 = 10755 / 0x10 |
| 5429 | 0 | 10841 | 10841 | 0x20 | 2 | 22 | 22 | slot21 = 10841 / 0x20 |
| 5449 | 0 | 10900 | 10900 | 0x20 | 2 | 23 | 23 | slot22 = 10900 / 0x20 |
| 5479 | 0 | 10989 | 10989 | 0x20 | 2 | 24 | 24 | slot23 = 10989 / 0x20 |
| 5480 | 0 | 10992 | 10989 | 0x20 | 2 | 24 | 24 | no new append |

The replay lane after frame `5479` contains:

```text
slot20 tick=10755 mask=0x10
slot21 tick=10841 mask=0x20
slot22 tick=10900 mask=0x20
slot23 tick=10989 mask=0x20
```

## Conclusion

The v3 target window is definitively non-replay: `ctx+0x52 == 0` throughout.
The `0x10/class1` first beat and following `0x20/class2` beats are fresh
`80035510` live-input writes at `801C7F24/801C7F38`, then `80014614` appends
them to the shared replay log.

`801C8660` / `801C4FC8` is a real setup path for `transitionState == 1`
replay-table seeding, but it is not the authority for this row3 first-beat
window. The current Win full-song artifact's `0x20/class2` bias is therefore a
host input schedule/runtime integration issue, not a missing event-table
consumer and not a direct-core replay-mode bug.
