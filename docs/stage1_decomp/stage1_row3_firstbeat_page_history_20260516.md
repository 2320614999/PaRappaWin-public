# Stage1 row3 first-beat page history facts - 2026-05-16

Scope: narrow PSX recorder facts for the row3 first-beat classifier. This
does not authorize renderer, marker, lifecycle clear, no-fail, row/v22 forcing,
or full-song harness input patches.

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Exporter:

```text
python .\tools\模拟器内存回放器.py "<recording>" export_windows 3858 3866 artifacts\psx_row3_firstbeat_ring_pages_3858_3866.csv --format csv --step 1 --window ctx0@0x801C3640:112 --window counters@0x80091808:48 --window page9@0x80092A90:384 --window page10@0x80092C10:384 --window page11@0x80092D90:384 --window page12@0x80092910:384
```

Artifact:

`artifacts/psx_row3_firstbeat_ring_pages_3858_3866.csv`

## Facts

Before the first row3 phase1 beat, PSX has three accepted `0x10` records on
page11:

| Frame | `ctx+0x0C` | `ctx+0x10` | `ctx+0x18` | `ctx+0x20` | `ctx+0x38` | `91810/12/14` | page11 occupied slots |
|---:|---:|---:|---:|---:|---:|---|---|
| `3858` | `4543` | `4505` | `0` | `0` | `12` | `3/3/0` | `8,10,12 = 0x10/1/1` |

The fresh first-beat accepted input is written before `14D58` resolves:

| Frame | `ctx+0x0C` | `ctx+0x10` | `ctx+0x18` | `ctx+0x20` | `ctx+0x38` | `91810/12/14` | page11 occupied slots |
|---:|---:|---:|---:|---:|---:|---|---|
| `3861` | `4552` | `4552` | `0x10` | `1` | `12` | `4/4/0` | `8,10,12,14 = 0x10/1/1` |

At the first classifier beat:

| Frame | `ctx+0x0C` | `ctx+0x10` | `ctx+0x18` | `ctx+0x20` | `ctx+0x38` | `ctx+0x58/5A` | `91808/0C` | `91810/12/14` | `91816/18` |
|---:|---:|---:|---:|---:|---:|---|---|---|---|
| `3865` | `4564` | `4552` | `0x10` | `1` | `12` | `2/1` | `0/0x10` | `0/0/0` | `9/9` |

Page12 is still empty across this first-beat window. The first positive
classifier therefore comes from page11 slot history, not from the later page12
slot10 write.

Important coverage note: `80014D58` converts a scene1 coverage table value
`15` to `14` and loops with `< 14`, so 0-based slot14 is not required for this
first `9/9` result. Slots `8,10,12` already provide three `0x10` matches; with
the anchor class match, the direct formula gives `3 * 3 = 9`.

## Win contrast

Latest runtime artifact:

`artifacts/stage1_rail_probe_runtime/20260516_203744_stage1_rail_full_song_runtime/summary.txt`

Its first row3 beat at query `768` has:

- `tick96=4596`
- page/descriptor `12`, `ED08=0x0A`
- `gAcc=3`
- `gReq=0x10`, `gUnion=0x20`
- `gCommit=-1`, `gWb=0`
- `phase1 sampledClassifier=0`, `ED36=0`, `ED38=1`

The same artifact shows the accepted input source immediately before it as:

- `acceptedTickSeedQuery=757`
- `acceptedTick96=4537`
- `writePage38=11`
- `recordSlot24=13`
- `recordRem24=9`

PSX still records the fresh first-beat input at `ctx+0x10=4552`, which maps to
page11 slot14:

```text
(4552 + 8) % 384 = 336
336 / 24 = slot 14
```

## Conclusion

The scorer/direct `14D58 -> 14D28 -> 24F8C` path should not be patched. The
current first-beat mismatch is upstream page history/mask source: Win feeds
`14D58` with `gReq=0x10` but a `0x20` page union in the target window, while
PSX has enough `0x10` accepted page history to produce `9/9`. Do not change
the `15 -> 14` coverage clamp or force slot14 into the first-beat scan.

Next code target: accepted input mask/source timing into `80014614` for the
first row3 `0x10` beat, then the following `0x20` beat. Do not force `ED36`,
`v22`, row tables, renderer, marker, or lifecycle clear.
