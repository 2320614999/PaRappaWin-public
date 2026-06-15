# Scene Submit `80028054` recorder call-trace hits

Scope: Stage1 Scene Submit / TOD `8001B000 -> 80028504 -> 80028054`; this is call-trace evidence only, not `800280B4` register authority.

Recording:

- `E:\game\PSgame\parappa the rapper\duckstation\bin\x64\标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec`

Raw artifact:

- `artifacts\coord44_backing_recorder\coord44_calltrace_frame2212_chain_20260515.json`
- Full target scan artifact: `artifacts/coord44_backing_recorder/coord44_calltrace_target_scan_0_14318_20260515.json`

Full target scan summary:

| target | hits | first observed frame |
| --- | ---: | ---: |
| `801C609C` | 0 | none |
| `801C5EF0` | 90 | 2 |
| `801C5E60` | 90 | 2 |
| `8001B000` | 9078 | 2 |
| `80028504` | 138 | 2212 |
| `80028054` | 202 | 2212 |

Representative command-interpreter frames:

| frame | 8001B000 | 80028504 | 80028054 | first seq | last seq |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 2212 | 2 | 2 | 18 | 18333 | 18520 |
| 2350 | 2 | 2 | 18 | 9588 | 12325 |
| 3336 | 4 | 2 | 18 | 11151 | 17039 |
| 3338 | 4 | 2 | 2 | 10836 | 17077 |
| 3340 | 4 | 2 | 2 | 11397 | 17580 |
| 3342 | 4 | 2 | 2 | 10904 | 16339 |
| 3344 | 4 | 2 | 2 | 11187 | 17032 |
| 3346 | 4 | 2 | 2 | 11410 | 17289 |
| 3348 | 4 | 2 | 2 | 11153 | 16689 |
| 3350 | 4 | 2 | 2 | 11353 | 16609 |

Frame `2212` call-chain sample:

| seq | type | pc | target | sp | ra |
| ---: | --- | --- | --- | --- | --- |
| 18333 | call | `0x801CBD60` | `0x8001B000` | `0x801FFEF8` | `0x801CBD68` |
| 18340 | call | `0x8001B044` | `0x80028504` | `0x801FFED8` | `0x8001B04C` |
| 18347 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18362 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18370 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18378 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18386 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18394 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18404 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18412 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18420 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18442 | call | `0x801CBDA8` | `0x8001B000` | `0x801FFEF8` | `0x801CBDB0` |
| 18448 | call | `0x8001B044` | `0x80028504` | `0x801FFED8` | `0x8001B04C` |
| 18453 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18462 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18470 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18478 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18486 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18494 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18504 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18512 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |
| 18520 | call | `0x80028548` | `0x80028054` | `0x801FFEA8` | `0x80028550` |

Boundary conclusion:

- This corrects the earlier narrow `0..1599` statement: the same PSXREC03 does cover `80028504 -> 80028054` later, first at frame `2212`.
- The trace still cannot authorize `coord44BackingSlotKnown80028054`, because it does not contain instruction-time `$v0` at `800280B4` or `mem32[$v0+0x44]`.
- Use frame `2212` as the first live-GDB reproduction window, then break on `0x800280B4` and capture `$v0` plus `mem32[$v0+0x44]`.
