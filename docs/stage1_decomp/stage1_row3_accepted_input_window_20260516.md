# Stage1 row3 accepted input window fact - 2026-05-16

Scope: narrow PSX recorder fact for the row3 first-beat classifier blocker.
This file does not authorize renderer, marker, lifecycle clear, no-fail,
stage status, Win duration, or synthetic success.

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Exporter:

`tools/模拟器内存回放器.py`

## Window export

Command:

```text
python .\tools\模拟器内存回放器.py "<recording>" export_windows 3948 3965 artifacts\psx_row3_accepted_window_3948_3965.csv --format csv --step 1 --window ctx0@0x801C3640:96 --window padbuf@0x80090000:512 --window page12@0x80092910:384
```

Output:

`artifacts\psx_row3_accepted_window_3948_3965.csv`

Important decoded fields:

| Frame | ctx+0x0C | ctx+0x10 | ctx+0x18 | ctx+0x20 | ctx+0x34 | ctx+0x38 | page12 slot8 | page12 slot9 | page12 slot10 |
|---:|---:|---:|---:|---:|---:|---:|---|---|---|
| `3948` | `4808` | `4806` | `0x20` | `2` | `8` | `13` | `0x20/2/1` | `0/0/0` | `0/0/0` |
| `3953` | `4822` | `4806` | `0` | `0` | `8` | `13` | `0x20/2/1` | `0/0/0` | `0/0/0` |
| `3960` | `4843` | `4806` | `0` | `0` | `8` | `13` | `0x20/2/1` | `0/0/0` | `0/0/0` |
| `3961` | `4846` | `4846` | `0x20` | `2` | `8` | `13` | `0x20/2/1` | `0/0/0` | `0x20/2/1` |
| `3965` | `4857` | `4846` | `0x20` | `2` | `8` | `13` | `0x20/2/1` | `0/0/0` | `0x20/2/1` |

## Writer facts

`ctx+0x18` (`0x801C3658`) writers in the target window:

| Frame | PC | Before | After | Meaning |
|---:|---|---:|---:|---|
| `3953` | `0x801C7F44` | `0x20` | `0` | non-replay no-input clear |
| `3961` | `0x801C7F24` | `0` | `0x20` | non-replay accepted input write |

`ctx+0x20` (`0x801C3660`) writers:

| Frame | PC | Before | After | Meaning |
|---:|---|---:|---:|---|
| `3953` | `0x801C7F48` | `2` | `0` | class token clear |
| `3961` | `0x801C7F38` | `0` | `2` | `80024B54(0x20)` class token |

`ctx+0x10` (`0x801C3650`) writer:

| Frame | PC | Before | After | Meaning |
|---:|---|---:|---:|---|
| `3961` | `0x801C7FB8` | `4806` | `4846` | non-replay post-frame backup from `ctx+0x0C` before `80014614` |

Page12 slot10 writer:

| Frame | Address | PC | Before | After | Meaning |
|---:|---|---|---:|---:|---|
| `3961` | `0x80092988[4]` | `0x80024B54` | `0` | `0x20` | accepted mask |

Companion/count are the same slot from the prior writer fact:
`0x8009298C[2] = 2`, `0x8009298E[2] = 1`.

## Conclusion

The PSX target does not rely on a stale earlier accepted tick for this row3
window. It clears `ctx+0x18` at frame `3953`, then a fresh non-replay input
write occurs at frame `3961`; the same frame backs up `ctx+0x10 = ctx+0x0C =
4846` and writes page12 slot10.

The current Win artifact after the `acceptedGateOpen` fix still shows the
default full-song input path producing page12 slot9 (`acceptedTick96=4821`) and
then missing the fresh slot10 input. The remaining blocker is therefore the
runtime accepted input source/timing window feeding `80014614`, not the `v22`
table, row transition mapping, renderer, marker, lifecycle clear, or Stage
status.
