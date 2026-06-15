# Stage1 row3 page12 slot10 accepted writer fact - 2026-05-16

Scope: narrow recorder fact for the Stage1 row3 -> row2 window. This does not
authorize lifecycle clear, renderer fallback, no-fail, marker, payload, Win
file, ISO/STR/XA, or default-value success.

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

## Recorder hit

Target window: PSX row write frame `3995`, where prior evidence records
`ctx+0x4E: 3 -> 2` through `80024FD0`, with `ED36=1`, `ED38=1`,
`ED00=0` immediately before the write.

New accepted writer hit feeding that row-write window:

| Frame | Address | PC | Before | After | Meaning |
|---:|---|---|---:|---:|---|
| `3961` | `0x80092988[4]` | `0x80024B54` | `0` | `0x20` | page-record accepted mask |
| `3961` | `0x8009298C[2]` | `0x800149D0` | `0` | `2` | page-record companion/class |
| `3961` | `0x8009298E[2]` | `0x800149DC` | `0` | `1` | page-record occupied/count |

Same frame state:

| Frame | Field | Value |
|---:|---|---:|
| `3961` | `ctx+0x0C` (`0x801C364C`) | `4846` |
| `3961` | `ctx+0x38` (`0x801C3678`) | `13` |
| `3961` | `ctx+0x18` (`0x801C3658`) | `0x20` |
| `3961` | `ctx+0x20` (`0x801C3660`) | `2` |

The page-record address is the page12 ring slot that `14D58` consumes at the
row3 -> row2 window. By frame `3995`, reads confirm:

| Address | Value |
|---|---:|
| `0x80092988[4]` | `0x20` |
| `0x8009298C[2]` | `2` |
| `0x8009298E[2]` | `1` |

## Win contrast

Current runtime artifact:

`artifacts/stage1_rail_probe_runtime/20260516_151246_stage1_rail_full_song_runtime/summary.txt`

Key contrast from existing handoff samples:

- Win q`1535`: page `12`, descriptor flag `0x0A`, row `3`, phase first beat
  samples `0`, accumulator remains `0`, `ED36=0`, `ED38=1`.
- Win q`1664`: page `13`, descriptor flag `0x0E`, row `3`, second beat sees
  page12 positive growth (`91816=4`), but cached first beat was `0`; the
  `0 -> 1` mismatch resets `ED36` to `2`, so `v22=2` and no row write.
- PSX target has page12 slot10 written before the row3 resolution window, so
  the first cached phase for the row3 -> row2 pair is already positive.

Conclusion:

The next direct-port cut should stay on the accepted writer timing/page-slot
path that feeds page12 before the row3 two-phase classifier. The current gap is
not the `v22` row table, `14548`, lifecycle clear, fail/marker, renderer, or
Stage status.
