# Scene Submit frame 2212 `80028054` per-call path facts

Date: 2026-05-15

Scope: Stage1 Scene Submit / TOD `80028054` `coordNode+0x44` authority only.
This note does not touch RGB, Rail/GTE, lower-CD, case17, MenuHelp/StageClear,
boot seam, renderer, or runtime code.

## Inputs

- Recording:
  `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`
- Raw artifact:
  `artifacts/coord44_backing_recorder/coord44_frame2212_80028054_per_call_paths_20260515.json`
- Prior full frame context:
  `artifacts/coord44_backing_recorder/coord44_frame2212_full_context_20260515.json`
- Static recheck:
  `docs/stage1_decomp/ida_export_scene_submit_coord44_owner_recheck_20260515.txt`

Frame `2212` channels:

- `call_trace=4486`
- `full_branch_trace=24563`
- `write_log=13084`
- `read_log=0`

## Static branch map

- `800280AC`: non-null desc path, `lw $v0, 4($a1)` loads the COORD node
  pointer from `desc+4`.
- `800280B4`: sequential `lw $s0, 0x44($v0)`, the narrow live authority for
  `coordNode+0x44`.
- `800280BC`: sequential `sw $zero, 0($v0)`, clears `coordNode+0` header only,
  not `+0x44`.
- `800280E8`: switch case 0, desc/header mask-or update.
- `80028104`: switch case 1, type1 COORD/TRS path.
- `80028440`: switch case 4, raw matrix/translation copy path.
- `800284D0`: common/default tail that computes the next TOD command pointer.
- `800284FC`: epilogue `jr $ra`; no semantic side effect.

Because `800280B4` and `800280BC` are sequential instructions, they are not
expected to appear as branch targets in `full_branch`. With `read_log=0`, this
recording still cannot provide instruction-time `$v0` or `mem32[$v0+0x44]`.

## Per-call grouping

The 18 `80028054` calls are two sibling submit groups, not one continuous TOD
stream:

- `first_801CBD60`: `801CBD60 -> 8001B000 -> 80028504 -> 80028054`, 9 calls.
- `second_801CBDA8`: `801CBDA8 -> 8001B000 -> 80028504 -> 80028054`, 9 calls.

All 18 calls include branch target `800280AC`. That proves every observed call
in this window takes the non-null descriptor/COORD-node path before the switch,
so a live `800280B4` breakpoint is immediately relevant for every row below.

| # | Submit | Seq range | Path class | Semantic writes |
|---:|---|---:|---|---|
| 1 | `first_801CBD60` | `18347..18359` | default/no semantic store | none |
| 2 | `first_801CBD60` | `18362..18367` | default/no semantic store | none |
| 3 | `first_801CBD60` | `18370..18375` | default/no semantic store | none |
| 4 | `first_801CBD60` | `18378..18383` | default/no semantic store | none |
| 5 | `first_801CBD60` | `18386..18391` | default/no semantic store | none |
| 6 | `first_801CBD60` | `18394..18401` | case0/header path | `800284D0 -> 801D98F0: 00000000 -> 00000080` |
| 7 | `first_801CBD60` | `18404..18409` | default/no semantic store | none |
| 8 | `first_801CBD60` | `18412..18417` | default/no semantic store | none |
| 9 | `first_801CBD60` | `18420..18427` | case4 matrix copy | `800284B8 -> 801D9918: 00000000 -> FFFFFDA8`; `800284C4 -> 801D991C: 00000000 -> FFFFFF9C` |
| 10 | `second_801CBDA8` | `18453..18459` | default/no semantic store | none |
| 11 | `second_801CBDA8` | `18462..18467` | default/no semantic store | none |
| 12 | `second_801CBDA8` | `18470..18475` | default/no semantic store | none |
| 13 | `second_801CBDA8` | `18478..18483` | default/no semantic store | none |
| 14 | `second_801CBDA8` | `18486..18491` | default/no semantic store | none |
| 15 | `second_801CBDA8` | `18494..18501` | case0/header path | `800284D0 -> 801DB2D8: 00000000 -> 00000080` |
| 16 | `second_801CBDA8` | `18504..18509` | default/no semantic store | none |
| 17 | `second_801CBDA8` | `18512..18517` | default/no semantic store | none |
| 18 | `second_801CBDA8` | `18520..18530` | case4 matrix copy | `8002844C -> 801DB2EC: 00001000 -> 00000C40`; `8002847C -> 801DB2F4: 00001000 -> 00000C40`; `800284AC -> 801DB2FC: 00001000 -> 00000C40`; `800284B8 -> 801DB300: 00000000 -> 00000258`; `800284C4 -> 801DB304: 00000000 -> FFFFFF9C` |

Stack-save writes from the `80028054` prologue are separated in the raw artifact
and must not be treated as TOD/COORD semantic state updates.

## Authority boundary

Positive recorder facts:

- frame `2212` contains two sibling `8001B000` submit groups from
  `801CBD60` and `801CBDA8`.
- each group dispatches one `80028504` call and nine downstream `80028054`
  calls.
- all 18 downstream calls take the `800280AC` desc/COORD-node path.
- write-log confirms cursor/count writebacks plus case0 desc/header stores and
  case4 matrix/translation stores.

Still not authorized:

- `coord44BackingSlotKnown80028054`
- `PsxCoordNode::type1Trs80028054` as a runtime value source for this window
- any inference from candidate absolute slots, frame-boundary registers,
  stack-save writes, desc/header stores, matrix stores, Win renderer, TMD data,
  or visual sampling

Next valid evidence remains live GDB at `800280B4` after gating on
`801CBD60/801CBDA8`, capturing instruction-time `$v0` and
`mem32[$v0+0x44]`.
