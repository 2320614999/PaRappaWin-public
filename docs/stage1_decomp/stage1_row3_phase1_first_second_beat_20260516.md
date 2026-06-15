# Stage1 row3 phase1 first/second beat facts - 2026-05-16

Scope: narrow PSX recorder facts for the Stage1 row3 -> row2 bucket28
classifier. This does not authorize renderer, marker, lifecycle clear,
F5/no-fail, stage status, synthetic `v22`, row-table forcing, or full-song
harness input patches.

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

## Exports

```text
python .\tools\模拟器内存回放器.py "<recording>" export_windows 3862 3867 artifacts\psx_row3_firstbeat_3862_3867.csv --format csv --step 1 --window scorer@0x8008ECE8:128 --window ctx@0x801C3640:192 --window counters@0x80091810:64 --window timingPtr@0x800943C0:16 --window timingTable@0x801D2F04:32

python .\tools\模拟器内存回放器.py "<recording>" export_windows 3993 3996 artifacts\psx_row3_bucket28_resolution_3993_3996.csv --format csv --step 1 --window scorer@0x8008ECE8:128 --window ctx@0x801C3640:192 --window counters@0x80091810:64 --window timingPtr@0x800943C0:16 --window timingTable@0x801D2F04:32
```

Artifacts:

- `artifacts/psx_row3_firstbeat_3862_3867.csv`
- `artifacts/psx_row3_bucket28_resolution_3993_3996.csv`

## First Beat

Frame `3865` is the first row3 phase1 beat for this descent pair:

| field | value |
|---|---:|
| `ctx+0x0C` | `4564` |
| `dword_8008ECE8` | `340` |
| `dword_8008ECF0/ECF4` | `28 / 28` |
| `ctx+0x38` page ordinal | `12` |
| `ctx+0x40/0x44` descriptor ptrs | `0x801CD4AC / 0x801CD4AC` |
| `ctx+0x4E` row | `3` |
| `ctx+0x58/0x5A` | `2 / 1` |
| `dword_8008ED00` | `0` |
| `dword_8008ED08` | `0x000A` |
| `word_80091810/12/14` before consumption | `4 / 4 / 0` |
| `word_80091816/18` after beat | `9 / 9` |
| `word_8008ED36/38` after beat | `1 / 1` |

`writers 0x8008ED36 2 3736 3995` confirms the first-beat cache write:

```text
Frame 3865 PC=0x80025434 0x0002 -> 0x0001
```

Because `ED08=0x0A` has bit `0x2` but not bit `0x4`, this beat runs
`14D58 -> 14D28` and arms `ED36/ED38`, but does not resolve `v22`.

## Second Beat / Resolution

Frame `3995` is the matching second beat and row-write frame:

| field | value |
|---|---:|
| `ctx+0x0C` | `4947` |
| `dword_8008ECE8` | `339` |
| `dword_8008ECF0/ECF4` | `28 / 28` |
| `ctx+0x38` page ordinal | `13` |
| `ctx+0x40/0x44` descriptor ptrs | `0x801CD4C4 / 0x801CD4C4` |
| `ctx+0x4E` row | `2` after row write |
| `ctx+0x58/0x5A` | `2 / 0` |
| `dword_8008ED00` | `6` after row write |
| `dword_8008ED08` | `0x000E` |
| `word_80091810/12/14` | `4 / 4 / 0` |
| `word_80091816/18` after beat | `18 / 18` |
| `word_8009181A/1C` after resolution | `18 / 18` |
| `word_8008ED36/38` after reset | `2 / 0` |

Focused writers in `3993..3996`:

```text
0x8008ECF4: frame 3995 PC=0x800250C4 27 -> 28
0x80091816: frame 3995 PC=0x8001528C 9 -> 18
0x80091818: frame 3995 PC=0x80014D44 9 -> 18
0x8008ED38: frame 3995 PC=0x8002549C 1 -> 0
0x8008ED36: frame 3995 PC=0x80024FA0 1 -> 2
0x801C368E: frame 3995 PC=0x80025704 3 -> 2
```

This proves the PSX order for the target descent:

```text
first beat: 14D58 positive growth -> 14D28 caches ED36=1 / ED38=1
second beat: 14D58 positive growth -> 14D28 keeps resolution input ED36=1
             -> 24F8C resets ED36=2 / ED38=0
             -> row 3 -> 2
```

## Descriptor Rows

The descriptor rows used by the two beats are:

| frame | ptr | first dword | required mask | flag word |
|---:|---|---:|---:|---:|
| `3865` | `0x801CD4AC` | `0x01010801` | `0x10` | `0x000A` |
| `3995` | `0x801CD4C4` | `0x01020801` | `0x20` | `0x000E` |

The second beat uses a different descriptor row and flag word from the first
beat. Therefore the Win gap should not be closed by forcing `v22` or changing
the row transition table. The next code target is the accepted page-record
history that must make `14D58` produce the PSX positive growth on the first
beat and again on the second beat.

## Current Win Contrast

Corrected-clock runtime artifact:

`artifacts/stage1_rail_probe_runtime/20260516_203744_stage1_rail_full_song_runtime/summary.txt`

It still reports:

- `rowWritebackV22OneSamples=0`
- last positive-delta sample has `resolutionV22=2`
- last positive-delta sample has `rankRow=3`, page `13`, `ED08=0x0E`,
  pre `ED36=0`, pre `ED38=1`, and `accumulator91816=4`

That shape is not the PSX first/second-beat pair above. It means Win reaches a
positive page13 sample after caching the wrong first-beat classifier, or with
insufficient accepted page-record history. Keep the investigation on
accepted record timing/history into `14D58`; do not patch renderer, marker,
lifecycle clear, `v22`, or row tables.
