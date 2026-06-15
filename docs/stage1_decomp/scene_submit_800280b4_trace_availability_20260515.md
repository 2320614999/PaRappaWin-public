# Scene Submit `800280B4` trace availability recheck

Date: 2026-05-15

Scope: Stage1 Scene Submit / TOD `80028054` `coordNode+0x44` authority only.

## Raw artifact

`artifacts/coord44_backing_recorder/coord44_frame2212_800280b4_trace_availability_20260515.json`

Recording:

`E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Frame: `2212`

Channels:

- `pc_trace=4096`
- `call_trace=4486`
- `full_branch=24563`
- `read_log=0`
- `write_log=13084`

## Target counts

| Target | pc_trace | call_trace pc/target | full_branch | read_log pc |
|---|---:|---:|---:|---:|
| `80028054` | 0 | 18 | 18 | 0 |
| `800280AC` | 0 | 0 | 18 | 0 |
| `800280B4` | 0 | 0 | 0 | 0 |
| `800280BC` | 0 | 0 | 0 | 0 |
| `80028550` | 0 | 18 | 18 | 0 |

Interpretation:

- `80028054` and `80028550` are visible as call/return or flow anchors.
- `800280AC` is visible in `full_branch` 18 times, confirming the observed
  calls enter the non-null desc/COORD node path.
- `800280B4` and `800280BC` are not visible in `pc_trace`, `call_trace`,
  `full_branch`, or `read_log`.
- `read_log=0` means no offline read entry exists for
  `lw $s0,0x44($v0)`.

## GDB availability

DuckStation GDB connection attempts to `127.0.0.1:2345` are still unavailable in
this session. The MCP connection returned target actively refused, and the local
socket probe did not find a usable listener.

## Authority boundary

This recheck provides a negative recorder fact for `800280B4`: current frame
`2212` traces cannot provide instruction-time `$v0` or `mem32[$v0+0x44]`.

Still valid next evidence:

- start DuckStation GDB server,
- gate on `801CBD60` or `801CBDA8`,
- break on `800280B4`,
- capture `$v0` and `mem32[$v0+0x44]`.

Not valid:

- candidate absolute slots,
- call/return trace alone,
- full-branch `800280AC` alone,
- `80028054` entry registers from frame-boundary state,
- stack saves,
- case0 desc/header stores,
- case4 matrix/translation stores,
- Win renderer/TMD/visual sampling.
