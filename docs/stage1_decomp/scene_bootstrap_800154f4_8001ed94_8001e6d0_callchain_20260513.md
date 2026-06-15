# 800154F4 -> 8001ED94 -> 8001E6D0 bootstrap call-chain (2026-05-13)

## Scope

This note records the current evidence window for `8001E6D0`. It is a
bootstrap work-list/draw-buffer init seam, but the caller evidence points to
the cold boot function `800154F4`, not to the `80015D18` scene callback loop.

## PS-X EXE direct-call scan

The raw file is a PS-X EXE:

| Field | Value |
| - | - |
| text load | `0x80010000` |
| text file offset | `0x800` |
| text size | `0x5F000` |

The first naive scan used `va = 0x80010000 + fileOffset` and produced a false
`0x8001F5A4` hit. Correct mapping is
`va = 0x80010000 + (fileOffset - 0x800)`.

Correct direct `jal` hits:

| Target | `jal` word | Hits |
| - | - | - |
| `8001E6D0` | `0C0079B4` | `file+0x00F5A4 -> 8001EDA4` |
| `8001C470` | `0C00711C` | `file+0x00F59C -> 8001ED9C` |
| `8001ED94` | `0C007B65` | `file+0x005D18 -> 80015518` |
| `8001E34C` | `0C0078D3` | `file+0x0067D4 -> 80015FD4` |
| `80015D18` | `0C005746` | `file+0x0161E8 -> 800259E8` |

## Active IDB Hex-Rays

### `8001ED94`

```c
void sub_8001ED94()
{
  sub_8001C470();
  sub_8001E6D0();
}
```

### `8001C470`

```c
int sub_8001C470()
{
  sub_8001C1E8();
  sub_800446A0(0);
  sub_800354C0(0);
  sub_8003FB9C(320, 240, 4, 0, 0);
  sub_80040AE4(0, 0, 0, 240);
  sub_80040B84();
  GsSetProjection(440);
  return sub_8001B1B0(0, 0, 0);
}
```

### `800154F4`

```c
int sub_800154F4()
{
  int v0;

  SetMem(2);
  ResetCallback();
  sub_8001ED94();
  sub_80025A00();
  sub_8001A1CC();
  PadStartCom();
  sub_80027FAC();
  sub_8001635C(1, 1, 1, 0);
  sub_80016A80();
  v0 = sub_80035560(-1);
  return srand(v0);
}
```

## Narrow call-chain table

| Edge | Evidence | Meaning |
| - | - | - |
| `800154F4:80015518 -> 8001ED94` | corrected `jal` scan + active IDB decompile | cold boot calls graphics/work-list bootstrap wrapper |
| `8001ED94:8001ED9C -> 8001C470` | corrected `jal` scan + active IDB decompile | wrapper first runs graph/projection init |
| `8001ED94:8001EDA4 -> 8001E6D0` | corrected `jal` scan + active IDB decompile | wrapper then runs the work-list/draw-buffer init seam |
| `8001E6D0 -> 8001E33C(80080CF8,80083FC0)` | `scene_entry_8001e6d0_worklist_draw_buffers_20260512.md` | sets draw-buffer globals after writing two `order=4` work-list descriptors |
| `80015D18:80015FD4 -> 8001E34C` | corrected `jal` scan + existing request trace | loop-tail draw-buffer global assignment remains a separate `8001E34C` request |

## Boundary conclusion

- `8001E6D0` must remain an independent carrier:
  `order=4`, heads `800872B0/800872F0`, then
  `8001E33C(80080CF8,80083FC0)`.
- It must not be merged into or overwrite
  `PsxSeedMainPageWorkLists80087288`, whose seed is
  `order=14`, heads `80088288/80098288`.
- The caller evidence does not authorize wiring `8001E6D0` through
  `80015D18` scene callback init, lifecycle, or `8001E34C`.
- A future code change should first translate the `800154F4 -> 8001ED94`
  bootstrap boundary, or a narrower `8001ED94` carrier, and only then apply
  `PsxCall8001E6D0_InitWorkListsAndDrawBuffers()` from that boot path.

## 2026-05-13 carrier update

`src/pr/pr_scene_bootstrap_direct.*` now includes two narrow carriers:

| Carrier | Meaning | Runtime wiring |
| - | - | - |
| `PsxCall8001C470_BootstrapGraphInit()` | Records the `8001C470` call order: `8001C1E8`, `800446A0(0)`, `800354C0(0)`, `8003FB9C(320,240,4,0,0)`, `80040AE4(0,0,0,240)`, `80040B84`, `GsSetProjection(440)`, `8001B1B0(0,0,0)` | none |
| `PsxCall8001ED94_BootstrapGraphAndWorkLists()` | Records `8001ED94 -> 8001C470 -> 8001E6D0` and preserves PSX order | none |

`8001C470` still carries `untranslatedCalleeGap=true` because several callees
are only recorded by call/argument facts here, not fully translated side
effects. This does not authorize boot cutover by itself. The next safe step is
either translating those callees or introducing a fuller `800154F4` bootstrap
direct unit around the already-exported call sequence.

The carrier deliberately reuses the narrow
`PrSceneBootWorkListDirect::PsxCall8001E6D0_InitWorkListsAndDrawBuffers()`
leaf for the already-translated `8001E6D0` work-list seed and draw-buffer
global setup. The shared `8001E33C/8001E34C` draw-buffer-global leaf lives in
`PrSceneDrawBufferDirect`, because `8001E34C` is also consumed by the
`80015D18` loop-tail request and is not boot ownership. `pr_scene_bootstrap_direct.h`
no longer includes `pr_scene_entry_direct.h`, so the cold-boot carrier does
not pull in the scene-entry callback interface or imply `80015D18` ownership.

## 2026-05-13 cold boot plan carrier

`src/pr/pr_scene_bootstrap_direct.*` now also includes
`PsxCall800154F4_ColdBootPlan()`. It records the `800154F4` call order and
arguments from the active IDB export:

| Order | Call | Carrier status |
| - | - | - |
| 1 | `SetMem(2)` / `80048970(2)` | call/arg fact only |
| 2 | `ResetCallback()` / `80035744()` | call fact only |
| 3 | `8001ED94()` | reuses `PsxCall8001ED94_BootstrapGraphAndWorkLists()` |
| 4 | `80025A00()` | call fact only |
| 5 | `8001A1CC()` | call fact only |
| 6 | `PadStartCom()` / `80026E4C()` | call fact only |
| 7 | `80027FAC()` | call fact only |
| 8 | `8001635C(1,1,1,0)` | call/arg fact only |
| 9 | `80016A80()` | call fact only |
| 10 | `80035560(-1)` | call/arg fact only; return unknown |
| 11 | `srand(v0)` / `80047FEC(v0)` | records seed comes from `80035560` return |

This is not a runtime cutover. The plan keeps `untranslatedCalleeGap=true`
because `8001C470` still has untranslated callees and most `800154F4` callees
are only recorded as call/argument facts here. The carrier does not execute
`SetMem`, `ResetCallback`, `PadStartCom`, `80027FAC`, `8001635C`,
`80035560`, or `srand` host side effects. A future runtime path must first
close the relevant boot subgraph with real side-effect facts.
