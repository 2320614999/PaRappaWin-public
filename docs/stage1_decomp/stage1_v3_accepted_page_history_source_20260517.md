# Stage1 v3 accepted page-history source facts (2026-05-17)

Scope: latest user-provided recording only. This is recorder evidence for the
upstream accepted page-history source/timing boundary. No Win runtime code,
harness, observer, sentinel, parser, renderer, lifecycle, row/v22 table, or
scorer formula behavior is changed by this document.

Recording:

```text
E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec
```

## Commands

The old `3848..3863` frame window from the earlier recording does not line up
with this v3 recording. The matching window was found by scanning `ctx+0x18`
changes and then narrowing around the `0x10 -> 0x20` sequence:

```powershell
python .\tools\模拟器内存回放器.py "<v3.psxrec>" changes 0x801C3658 4 0 14385
python .\tools\模拟器内存回放器.py "<v3.psxrec>" writers 0x801C3658 4 5388 5490
python .\tools\模拟器内存回放器.py "<v3.psxrec>" writers 0x801C3660 4 5388 5490
python .\tools\模拟器内存回放器.py "<v3.psxrec>" writers 0x80091816 2 5388 5520
python .\tools\模拟器内存回放器.py "<v3.psxrec>" export_windows 5396 5516 artifacts\psx_row3_firstbeat_ring_pages_v3_5396_5516.csv --format csv --step 1 --window ctx@0x801C3640:112 --window counters@0x80091808:48 --window page0@0x80092910:384 --window page1@0x80092A90:384 --window page2@0x80092C10:384 --window page3@0x80092D90:384 --window pad@0x800882F0:16
```

Artifacts:

```text
artifacts\psx_row3_firstbeat_input_source_v3_3848_3863.csv
artifacts\psx_row3_firstbeat_input_source_v3_candidate_5388_5490.csv
artifacts\psx_row3_firstbeat_ring_pages_v3_5396_5516.csv
```

The first artifact is a negative alignment check: the earlier absolute frame
window had no `ctx+0x18/+0x20` writers in v3.

## Writer facts

`ctx+0x18` (`0x801C3658`) writers in the narrowed v3 window:

```text
Frame 5399  PC=0x801C7F24  0x00000000 -> 0x00000010
Frame 5405  PC=0x801C7F44  0x00000010 -> 0x00000000
Frame 5429  PC=0x801C7F24  0x00000000 -> 0x00000020
Frame 5435  PC=0x801C7F44  0x00000020 -> 0x00000000
Frame 5449  PC=0x801C7F24  0x00000000 -> 0x00000020
Frame 5455  PC=0x801C7F44  0x00000020 -> 0x00000000
Frame 5479  PC=0x801C7F24  0x00000000 -> 0x00000020
Frame 5485  PC=0x801C7F44  0x00000020 -> 0x00000000
```

`ctx+0x20` (`0x801C3660`) writers:

```text
Frame 5399  PC=0x801C7F38  0x00000000 -> 0x00000001
Frame 5405  PC=0x801C7F48  0x00000001 -> 0x00000000
Frame 5429  PC=0x801C7F38  0x00000000 -> 0x00000002
Frame 5435  PC=0x801C7F48  0x00000002 -> 0x00000000
Frame 5449  PC=0x801C7F38  0x00000000 -> 0x00000002
Frame 5455  PC=0x801C7F48  0x00000002 -> 0x00000000
Frame 5479  PC=0x801C7F38  0x00000000 -> 0x00000002
Frame 5485  PC=0x801C7F48  0x00000002 -> 0x00000000
```

Score accumulator writers:

```text
Frame 5513  PC=0x8001528C  word_80091816: 0x0028 -> 0x0034
Frame 5513  PC=0x80014D44  word_80091818: 0x0028 -> 0x0034
Frame 5513  PC=0x80025434  word_8008ED36: 0x0002 -> 0x0001
```

## Window facts

Key decoded frames from `artifacts\psx_row3_firstbeat_ring_pages_v3_5396_5516.csv`:

| frame | ctx+0x0C | ctx+0x10 | ctx+0x18 | ctx+0x20 | ctx+0x34 | ctx+0x38 | 91810/12/14 | 91816/18 | page0 records |
|---:|---:|---:|---:|---:|---:|---:|---|---|---|
| 5398 | 10752 | 9408 | 0 | 0 | 8 | 28 | 0/0/0 | 40/40 | empty |
| 5399 | 10755 | 10755 | 0x10 | 1 | 8 | 29 | 1/1/0 | 40/40 | slot0 `0x10/1/1` |
| 5429 | 10841 | 10841 | 0x20 | 2 | 8 | 29 | 2/2/0 | 40/40 | slot0 `0x10/1/1`, slot4 `0x20/2/1` |
| 5449 | 10900 | 10900 | 0x20 | 2 | 8 | 29 | 3/3/0 | 40/40 | plus slot6 `0x20/2/1` |
| 5479 | 10989 | 10989 | 0x20 | 2 | 8 | 29 | 4/4/0 | 40/40 | plus slot10 `0x20/2/1` |
| 5513 | 11090 | 10989 | 0 | 0 | 8 | 29 | 0/0/0 | 52/52 | same page0 records retained |

Here `page0` is the ring page at `0x80092910`; `ctx+0x38=29`, and `29 & 3`
maps to this ring page.

## Conclusion

The latest v3 recording independently confirms the upstream source pattern:
PSX writes a fresh non-replay `0x10/class1` accepted record first, then writes
`0x20/class2` records, and only after that does `80014D58` advance the score
accumulators.

This supports the current boundary decision:

- Do not continue editing `80014614`, `80014D58`, `80014D28`, `80024F8C`,
  coverage/slot14, row/v22 tables, renderer, lifecycle clear, or full-song
  marker logic.
- The remaining Win gap is the runtime source/timing that feeds accepted page
  history. A full-song runtime artifact that only produces `0x20/class2` is
  still not valid authority for the PSX first accepted beat.
