# VText descriptor reconcile: Scene0 vs Scene1 dword_800943CC (2026-05-14)

Scope: correct the `80024C84/80024CF8` descriptor-source evidence after the
current UI IDB produced `801C5B14 -> dword_800943CC -> 801C6BF8`.

## Correction

`801C6BF8` is not Stage1/Scene1 common-lyrics authority. It belongs to the
Scene0 / COMOD0 init path:

| Scene | fn0 | dword_800943CC source | Meaning |
|---:|---:|---:|---|
| `0` | `801C5B14` | `0x801C6BF8` | Scene0 / COMOD0 movie/menu vtext descriptor base. |
| `1` | `801CA3BC` | `0x801CE080` | Scene1 / COMOD1 Stage1 movie/common-lyrics descriptor base. |

Both functions write the same main-exe global slot `dword_800943CC`, but only
the active scene's fn0 is authoritative for that scene. The scene callback table
maps scene index `0` to `801C5B14` and scene index `1` to `801CA3BC`.

## Evidence

- `docs/stage1_decomp/ida_export_vtext_descriptor_data_sources_20260514.md`
  captures the current UI IDB xrefs:
  `801C5B14 @ 801C5B60` writes `dword_800943CC = &off_801C6BF8`.
- `docs/stage1_decomp/menuhelp_80026314_context_20260511/S1_COMOD1/decompile_801ca3bc.txt`
  captures the COMOD1-specific Scene1 init:
  `PrOverlay1_InitGlobals()` writes `MEMORY[0x800943CC] = &off_801CE080`.
- `src/pr/pr_scene_entry_direct.cpp` maps scene index `0` to
  `801C5B14/801C4260/801C4DC4` and scene index `1` to
  `801CA3BC/801C7284/801C81EC`.
- `src/pr/pr_stage1_lifecycle_direct.cpp` already models
  `InitGlobals801CA3BC()` with `ptr800943CC = 0x801CE080`.

## Descriptor Tables

Scene0 `801C6BF8` is descriptor-shaped and valid for its own context:

| desc | text tables | entries | count |
|---:|---|---:|---:|
| `0x801C6BF8` | `0x801C69C8, 0x801C6A08, 0x801C6A48, 0x801C6A88, 0x801C6AC8` | `0x801C6B08` | `0x0F` |

Scene1/Stage1 current Win authority remains:

| mode | desc | entries | count |
|---:|---:|---:|---:|
| `0` | `0x801CE080` | `0x801CDD90` | `41` |
| `1` | `0x801CE09C` | `0x801CE020` | `6` |
| `2` | `0x801CE0B8` | `0x801CE020` | `6` |

## Runtime Boundary

Do not replace Stage1 `0x801CE080/0x801CE09C/0x801CE0B8` with `0x801C6BF8`.
That would feed a Scene0 descriptor into the Scene1 movie/common-lyrics path.

If the vtext loader is generalized later, pass the active scene fn0 globals
explicitly:

- Scene0 path receives `801C5B14` facts and `ptr800943CC = 0x801C6BF8`.
- Scene1 path receives `801CA3BC` facts and `ptr800943CC = 0x801CE080`.

Until that direct-port widening is done, the current Stage1 loader constants
are the correct Scene1 authority.
