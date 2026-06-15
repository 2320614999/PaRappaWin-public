# Stage1 scene submit/draw export notes 2026-05-12

Authority rule: IDA pseudo C / disassembly is the source of truth. This file
only records export status and the next graph boundary; it is not a replacement
for full per-function dumps.

## Export status

- `8001B084`: full current-session pseudo C, disassembly, xref, and callee
  table now persisted in
  `../ida_export_scene_submit_static_graph_main_8001b084_800428b0_8001b000_80028504_80028054_20260512.txt`.
  The older shape below remains a summary.
  Shape: iterates `count`, calls `80041A68(desc+4)`, `80040544(stack)`, then
  `800428B0(desc, otag, 14-depth, 0x1F800000)`.
- `800428B0`: full current-session pseudo C, disassembly, xref, and callee
  table now persisted in the same main-SCUS export. The older shape below
  remains a summary.
  Shape: decodes desc header flags, selects primitive dispatch table by type and
  mode, walks TMD primitive stream until count is consumed.
- `8001B000`: full current-session pseudo C, disassembly, xref, and callee
  table now persisted in the same main-SCUS export. The older shape below
  remains a summary.
  Shape: validates cursor/count, calls `80028504(frame, cursor, coord, 0)`,
  stores the returned cursor, and decrements count only when cursor advances.
- `80028504`: full current-session pseudo C, disassembly, xref, and callee
  table now persisted in the same main-SCUS export. The older shape below
  remains a summary.
  Shape: when `frame >= cursor+4`, reads block count at `cursor+2`, then loops
  through `80028054`.
- `80028054`: full current-session pseudo C, disassembly, xref, and callee
  table now persisted in the same main-SCUS export. The older shape below
  remains a summary.
  Shape: decodes TOD command nibble/flags and mutates the target coord/matrix
  state, including `RotMatrix*`, `TransMatrix`, and command-mode skips.
- `801CAC34`: pseudo C dumped from `S1/COMOD1.BIN.i64`.
  See `decompile_801cac34.txt` and `comod1_manifest.json`. The synchronous
  tool call hit the 120 second client timeout, but the output was written after
  timeout.
  Shape: submits Stage1 backdrop scene ranges through `8001B084`, emits the
  gradient rectangle, then emits `8001B590` backdrop sprites for `YU_KUMO`,
  `YU_YAMA`, `YU_HI`, and the `UIRender_Block2` gated `ON_KAO` / `YU_JHAN`.

## Required next graph

- `801CAC34 -> 8001B084 -> 800428B0`: backdrop/scene submit to range flush to
  TMD primitive dispatch.
- `8001B000 -> 80028504 -> 80028054`: TOD cursor to block loop to coord/matrix
  side effect.
- Incomplete graph manifest:
  `../scene_submit_static_graph_801cac34_8001b084_800428b0__8001b000_80028504_80028054_20260512.md`.

## Do-not-delete boundary

- Do not delete renderer/backend helpers for this graph until the new main-SCUS
  export is translated into a written side-effect graph. `801CAC34` now has
  pseudo C, but still needs a clean standalone disassembly/xref file if the
  graph document requires single-file evidence.
- `pr_stage_scene_submit_backend.cpp` still owns Win VRAM/TIM/TMD/D3D HAL.
- `pr_tmd_renderer.cpp` is not a Stage1 scene-state owner; handle Scene0/title
  dependencies separately before deleting anything there.
- Do not use this export status note to expand `800428B0` primitive depth or to
  fill packet fields from Win defaults, renderer state, or sampled values.

## 801CAC34 implementation delta

- `PrStageRunner_EmitBackdropPrimitives801CAC34` was missing the final
  `g_PrStageRunner_UIRender_Block2` branch from the pseudo C:
  `8001B590(v6+40, v5-2, 801D2F70)` and
  `8001B590(v6+40, v5+93, 801D2F80)`.
- `COMPO01.INT` contains the backing TIMs `ON_KAO.TIM` and `YU_JHAN.TIM`, so the
  Win texture keys are `on_kao` and `yu_jhan`.
