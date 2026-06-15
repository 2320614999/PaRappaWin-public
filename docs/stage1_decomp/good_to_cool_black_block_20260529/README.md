# Stage1 GOOD->COOL Raised-Camera Black Block Handoff - 2026-05-29

Purpose: prepare a source-backed handoff for external Pro review without adding
diagnostic code to the game runtime, then record the follow-up direct-port fix
found from replay/probe evidence.

Follow-up:

- `pro_review_result_20260529.md` records the ChatGPT Web Pro response. It
  returned `no safe patch` and recommends live PSX/GDB capture as the next safe
  action.
- Subsequent local replay/probe comparison found one safe direct-port mismatch:
  Win registered the 5-way dynamic row cursor outside the PSX `ctx&0x2000`
  branch.

## Local Boundary

- Workspace: `E:\game\PSgame\parappa the rapper\win`
- Branch target: `codex/stage1-blackblock-pro-handoff-20260529`
- Runtime code diff after reverting the failed GTE attempt: one direct-port
  fix in `src/pr/pr_stage_scene_submit_direct.cpp`.
- External probe diff: `tools/stage1_mem_probe/stage1_scene_submit_probe.py`
  now samples `0x801D7F40..50`, `0x801D7F64`, A/B/C/D item counters, and
  related `0x801D3018..3028` fields. It remains read-only.
- Build rule used: full `.\build.ps1`, no quick build.
- Full build after the dynamic-row fix succeeded and wrote:
  `E:\game\PSgame\parappa the rapper\PaRappaWin.exe`

## User-Visible Symptom

Stage1 GOOD->COOL transition can expose a large black rectangular void when the
camera raises/rotates. User screenshots showed the emulator reference retaining
background/scene layering while the Win port exposes black blocks.

Local reproduced screenshots after reverting the failed patch:

- `E:\game\PSgame\parappa the rapper\screenshots\goodtocool_revert_stage_520_20260529__0001.png`
- `E:\game\PSgame\parappa the rapper\screenshots\goodtocool_revert_stage_650_20260529__0002.png`
- `E:\game\PSgame\parappa the rapper\screenshots\goodtocool_revert_stage_720_20260529__0003.png`

Earlier screenshots before the failed patch already showed the same class of
black block:

- `E:\game\PSgame\parappa the rapper\screenshots\goodtocool_stage_520_20260529__0001.png`
- `E:\game\PSgame\parappa the rapper\screenshots\goodtocool_stage_650_20260529__0002.png`
- `E:\game\PSgame\parappa the rapper\screenshots\goodtocool_stage_720_20260529__0003.png`

Conclusion: the black block is not caused by the reverted GTE/SXY saturation
attempt. It is present in the current baseline for this forced GOOD->COOL path.

## Failed Attempt Reverted

A tentative core change in `src/pr/pr_stage_scene_submit_direct.cpp` rejected
`PsxProjectVertexRtpt428B0` outputs when `cz > 0xFFFF` or projected SXY fell
outside `[-1024, 1023]`. It was based on a broad GTE FLAG hypothesis.

Result: full build succeeded, but screenshots still showed the black block.
Because the attempt did not hit the symptom and risked altering unrelated scene
priority, it was reverted. The source now has no runtime-code diff.

## External Tools Added

`tools/stage1_mem_probe/stage1_scene_submit_probe.py` is a read-only sidecar. It
uses `ReadProcessMemory` plus DebugServer and samples:

- local desc states
- `801CBFDC` camera/presentation state
- `801CB190` submit ranges
- `s_stage1SceneSubmit428B0Debug`
- DebugServer `status`, `json`, and `stage1ovl scene428b0`

It does not inject, patch, or write target memory.

`tools/stage1_mem_probe/stage1_force_good_to_cool.py` is reproduction-only. It
writes PDB-verified runtime facts to force a GOOD->COOL transition and should
not be treated as PSX authority.

## Reproduction Commands

```powershell
.\build.ps1
python test_dispatcher.py --stage-select-to-stage1
python tools\stage1_mem_probe\stage1_force_good_to_cool.py `
  --wait-active --min-query-frame 170 `
  --natural-bucket30-good-to-cool `
  --scene-camera-trace `
  --duration 0.8 --interval 0.05 `
  --post-samples 30 --debug-sample-every 0 `
  --out artifacts\stage1_mem_probe\natural_bucket30_after_revert_20260529.jsonl
python tools\stage1_mem_probe\stage1_scene_submit_probe.py `
  --count 24 --interval 0.05 `
  --out artifacts\stage1_mem_probe\20260529_scene_submit_after_revert_black.jsonl
```

## Win Runtime Evidence

Representative post-revert sample:

```text
artifact: artifacts/stage1_mem_probe/20260529_scene_submit_after_revert_black.jsonl
stageFrame=1462
lastAdvanceQueryFrame=1458
UIList_Block=1
UIRender_Block=1
UIRender_Block2=0
dword_801D302C=0x00000021
dword_801D3034=0x00000001
camera pos=[12,-499,-2231]
camera target=[-579,-562,-72]
scene428B0 commandCount=453
scene428B0 gapCount=0
801CB190 submit order:
  ON, PA, FLOOR, TAMA, TOUROU, AUX_7FF8, KAGE
```

Later post-revert samples keep the same `801CB190` order:

```text
stageFrame=1472 query=1468 camera=[32,-510,-2318] target=[-443,-583,-62]
stageFrame=1482 query=1478 camera=[63,-522,-2438] target=[-268,-601,-52]
late query=1557 camera=[217,-573,-3052] target=[624,-693,-3]
submit order remains:
  ON, PA, FLOOR, TAMA, TOUROU, AUX_7FF8, KAGE
scene428B0 commandCount eventually reaches 509; gapCount remains 0.
```

Pre-revert and failed-patch samples showed the same UIList path:

```text
artifact: artifacts/stage1_mem_probe/20260529_scene_submit_probe_natural_goodtocool.jsonl
stageFrame=735 query=731
UIList_Block=1
UIRender_Block=0
submit order:
  ON, PA, FLOOR, TAMA, TOUROU, AUX_7FF8, KAGE

artifact: artifacts/stage1_mem_probe/20260529_scene_submit_after_gte_flag_gate.jsonl
stageFrame=757 query=753
UIList_Block=1
submit order:
  ON, PA, FLOOR, TAMA, TOUROU, AUX_7FF8, KAGE
```

## Pseudo-C Authority Already Checked

`docs/stage1_decomp/ida_fresh_801cb6bc_20260524/decompile_801cb190.txt`
matches the Win submit membership in the black-block frame.

Key branch shape:

```c
if ( g_PrStageRunner_UIList_Block )
{
  MEMORY[0x8001B084](&unk_801D7F68, 1,
                     (char *)&unk_801D3244 + 20 * MEMORY[0x8006EDA8], 10);
  MEMORY[0x8001B084](&unk_801D7FF8, 3,
                     (char *)&unk_801D3244 + 20 * MEMORY[0x8006EDA8], 10);
}
else
{
  MEMORY[0x8001B084](&unk_801D7F68, 1,
                     (char *)&unk_801D80A8 + 20 * MEMORY[0x8006EDA8], 8);
  MEMORY[0x8001B084](&unk_801D7F78, 1,
                     (char *)&unk_801D8058 + 20 * MEMORY[0x8006EDA8], 8);
  MEMORY[0x8001B084](&unk_801D7F88, 2,
                     (char *)&unk_801D8080 + 20 * MEMORY[0x8006EDA8], 8);
  MEMORY[0x8001B084](&unk_801D7FA8, 1,
                     (char *)&unk_801D80A8 + 20 * MEMORY[0x8006EDA8], 8);
  MEMORY[0x8001B084](&unk_801D7FB8, 7,
                     (char *)&unk_801D3244 + 20 * MEMORY[0x8006EDA8], 10);
  if ( g_PrStageRunner_UIRender_Block )
    return g_PrStageRunner_UIRender_Block;
}
return MEMORY[0x8001B084](&unk_801D8028, 1,
                          (char *)&unk_801D526C + 20 * MEMORY[0x8006EDA8], 8);
```

Therefore, do not "fix" this by simply adding `KABE_*` or `TENNJOU` to the
UIList branch. The visible symptom is not enough to override this pseudo-C.

## Existing PSX Recording Coverage Gap

All currently available `.psxrec` files under `..\duckstation` were scanned for:

- `g_PrStageRunner_UIList_Block` at `0x801DB340`
- `g_PrStageRunner_UIRender_Block` at `0x801DB33C`
- `g_PrStageRunner_UIRender_Block2` at `0x801DB344`
- `dword_801D3034` at `0x801D3034`
- `0x801D7F40..50` dynamic row cursors
- `0x801D7F64` shared init counter

Results:

```text
stage1二周目_无cool_good评级通关3.psxrec
  frames=9176
  UIList_Block nonzero=0
  UIRender_Block nonzero frames=164..9175

标题画面-menu-选关选stage1-good通关-保存记忆卡（存档重名保存失败重试成功）（过场动画均为跳过）v3.psxrec
  frames=14386
  UIList_Block nonzero=0
  UIRender_Block nonzero frames=1780..14385

标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）.psxrec
  frames=14318
  UIList_Block nonzero=0
  UIRender_Block nonzero frames=2350..14317

标题画面-menu-选关选stage1-good通关-保存记忆卡（过场动画均为跳过）v2.psxrec
  frames=13274
  UIList_Block nonzero=0
  UIRender_Block nonzero frames=1778..13273
```

Expanded replay conclusion: these recordings do contain visual Stage1 scene
frames, but the available PSX path keeps `UIList_Block=0`, `UIRender_Block2=0`,
`UIRender_RefreshRequest=0`, `0x801D7F40..50=0`, and `0x801D7F64=0` throughout.
They are useful authority for normal Stage1 render paths, not same-path
authority for a GOOD->COOL `ctx&0x2000` dynamic-row reload.

## Direct-Port Fix After Replay Comparison

Win before this fix diverged even on normal Stage1 samples:

```text
artifact: artifacts/stage1_mem_probe/20260529_scene_submit_expanded_stage1_ready.jsonl
dword_801D7F40=0x60003708
dword_801D7F44=0x60004708
dword_801D7F48=0x60006208
dword_801D7F4C=0x60008008
dword_801D7F50=0x60009908
UIList_Block=0
UIRender_Block=1
submit order:
  ON, PA, FLOOR, TAMA/TAMA_1, TOUROU, TENNJOU, KABE_R, KABE_B, KAMON_L
```

Pseudo-C for `801CBFDC` Block E only calls `8001AFD8(...)` for the five dynamic
row slots when `(*a1 & 0x2000) != 0`. There is no non-`0x2000` else branch that
registers `0x801D7F40..50` cursor sources.

Fix:

- remove the non-`0x2000` `RegisterPsxTodCursorSource8001B000(...)` branch from
  `ApplyPrStageRunnerDynamicRowBlockE_801CBFDC`;
- keep the five `PsxCall8001AFD8_801CBFDC(...)` calls only under
  `ctx.flags_00 & 0x2000`;
- preserve the independent `ctx&0x4040 -> UIRender_Block2=false` side effect.

Post-fix normal Stage1 sample:

```text
artifact: artifacts/stage1_mem_probe/20260529_scene_submit_after_dynamic_row_fix_stage1_ready.jsonl
query range=566..665
0x801D7F40..50 all zero
UIList_Block true count=0
UIRender_Block true count=10/10
submit order:
  ON, PA, FLOOR, TAMA/TAMA_1, TOUROU, TENNJOU, KABE_R, KABE_B, KAMON_L
```

This answers the replay/shell question: the replay has the scene, but it does
not hit Win's stale `7F40/UIList_Block` path. The normal-path black-block
setup was a Win direct adapter mismatch, not PSX replay coverage.

## Remaining GOOD->COOL Boundary

The forced/natural GOOD->COOL reproduction still reaches a real `0x2000` path
after the fix:

```text
artifact: artifacts/stage1_mem_probe/natural_bucket30_after_dynamic_row_fix_q400_20260529.jsonl
first GOOD->COOL commit markers: query=435
bucket30OwnerGoodToCoolCommitted=1
bucket30WritebackCommitted=1
lastRowWritePrevRow=1
lastRowWriteResolvedRow=0
ctx72=1
selectedStream=6
eventRuntime.flags40_2000_4000=0x2000 for 2 samples
```

Later scene-submit samples from the same run show the expected consequence of
that `0x2000` path:

```text
artifact: artifacts/stage1_mem_probe/20260529_scene_submit_after_dynamic_row_fix_natural_goodtocool.jsonl
query range=724..1153
0x801D7F40=0x60009418 for 40/40 samples
UIList_Block=true for 40/40 samples
submit order sample:
  PA, FLOOR, TAMA/TAMA_1, TOUROU, AUX_7FF8, KAGE
```

This does not prove the black block is fully fixed. It proves the old stale
normal-stage cursor registration is fixed, and that the remaining reproduction
now depends on the actual GOOD->COOL `flag2000Pulse -> 801C9094 -> 801CBFDC`
chain. A real PSX GOOD->COOL replay or live PSX/GDB capture is still needed to
decide whether PSX also sets this `0x2000` dynamic-row reload and what the
correct later `801CB190/800428B0` packet behavior should be.

## Current Working Hypothesis

After the non-`0x2000` dynamic-row fix, normal Stage1 `7F40/UIList_Block`
behavior matches the available PSX replays. The remaining GOOD->COOL sample is
now a same-chain question, not a stale normal-path registration bug.

The current `801CB190` range membership appears pseudo-C aligned for the
remaining GOOD->COOL frame. If a real PSX same-path capture confirms
`ctx&0x2000 -> UIList_Block=1`, the likely unresolved area is lower than range
membership:

- `8001B084 -> 800428B0` packet/OT behavior
- `GsTMDfast*` GTE FLAG / NCLIP / AVSZ / OT bucket translation
- the renderer adapter's handling of PSX OT/head insertion/depth ordering
- or the forced GOOD->COOL route not matching a real PSX path, because no PSX
  same-path recording has yet been captured

Known Stage1 model fact relevant to the visual:

```text
kage.tmd: objs=1 v=4 p=2 bbox=[-3200..3200, 500..500, -3300..2800]
```

The huge flat KAGE plane is suspicious when the camera is raised, but changing
KAGE order/layer without PSX evidence previously risks breaking stable scene
priority.

## External Review Question

Please review only the evidence in this branch/document and answer:

1. Is there a safe direct-port patch justified now?
2. If yes, identify the exact pseudo-C / PSX behavior it implements and provide
   a narrow unified diff.
3. If no, say "no safe patch" and list the next missing PSX facts to capture.

Do not recommend symptom patches like hiding KAGE, adding walls to the UIList
branch, or changing renderer priority unless the recommendation is directly
backed by PSX pseudo-C/live facts.
