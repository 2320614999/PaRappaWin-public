# Stage1 row3 first-beat input source - 2026-05-16

Scope: narrow PSX recorder facts for the accepted page-history blocker. This
does not authorize renderer, marker, lifecycle clear, full-song harness input
semantic changes, synthetic row writeback, `v22` forcing, row table edits,
coverage changes, or `24F8C` patches.

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Commands:

```text
python .\tools\模拟器内存回放器.py "<recording>" writers 0x801C3658 4 3848 3863
python .\tools\模拟器内存回放器.py "<recording>" writers 0x801C3660 4 3848 3863
python .\tools\模拟器内存回放器.py "<recording>" export_windows 3848 3863 artifacts\psx_row3_firstbeat_input_source_3848_3863.csv --format csv --step 1 --window ctx@0x801C3640:112 --window pad@0x800882F0:16 --window replayCounts@0x800901BC:16 --window replayTicks@0x8008EEF8:96 --window replayMasks@0x8008EEFC:96
```

Artifact:

`artifacts/psx_row3_firstbeat_input_source_3848_3863.csv`

## Writer facts

`ctx+0x18` (`0x801C3658`) in the first-beat window:

```text
Frame 3851 PC=0x801C7F44 0x10 -> 0x0
Frame 3861 PC=0x801C7F24 0x0  -> 0x10
```

`ctx+0x20` (`0x801C3660`) in the same window:

```text
Frame 3851 PC=0x801C7F48 0x1 -> 0x0
Frame 3861 PC=0x801C7F38 0x0 -> 0x1
```

Decoded frame facts:

| frame | `ctx+0x0C` | `ctx+0x10` | `ctx+0x18` | `ctx+0x20` | `ctx+0x34` | `ctx+0x38` | `ctx+0x52` | `dword_800882F0` | `80035510` return | `901BC/901C0` |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `3848` | `4512` | `4505` | `0x10` | `1` | `8` | `12` | `0` | `0xFFFFFFEF` | `0x10` | `9/9` |
| `3851` | `4522` | `4505` | `0x00` | `0` | `8` | `12` | `0` | `0xFFFFFFFF` | `0x00` | `9/9` |
| `3861` | `4552` | `4552` | `0x10` | `1` | `8` | `12` | `0` | `0xFFFFFFEF` | `0x10` | `10/10` |
| `3863` | `4557` | `4552` | `0x10` | `1` | `8` | `12` | `0` | `0xFFFFFFEF` | `0x10` | `10/10` |

## Static clear-path check

Static review of current direct code confirms the literal `14BDC` page-clear
projection is not conditional on accepted input:

- `RunStage1ScorerDispatcher24FD0(...)` returns a `sameTickPageClear14BDC`
  result for literal bucket `31`.
- `RunStage1AcceptedProducer14614(...)` calls
  `ApplyStage1PageMaintenanceProjectionFromLiteralPageClear14BDC_14614(...)`
  even when `callAcceptedProducer14614` is false and no accepted run was
  captured.
- The final projection resolver accepts `sameTickPageClear` alone and clears
  `state.pageRecordMirror.pages[targetOrdinal & 3]`.

So the current row3 mismatch should not be closed by adding another page-clear
carrier or by changing bucket31/literal bucket routing.

## Conclusion

PSX first-beat page history is fed by a fresh non-replay `80035510` return
`0x10`, class token `1`, and `ctx+0x10 = ctx+0x0C = 4552` on frame `3861`.

Latest runtime contrast remains input history, not scorer math: the target
runtime windows keep carrying `0x20/2` history or empty anchor slots where PSX
needs `0x10/1`. Do not patch `14D58`, `14D28`, `24F8C`, coverage, slot14,
`v22`, renderer, marker, lifecycle, or page-clear routing from this fact.
