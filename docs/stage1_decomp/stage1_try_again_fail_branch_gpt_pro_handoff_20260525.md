# Stage1 Try Again and fail-branch handoff for GPT-5.5 Pro

## Repository and branch

- Repository: `https://github.com/2320614999/PaRappaWin`
- Branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Use the latest pushed HEAD of this branch after the 2026-05-25 handoff commit.
- Latest narrow follow-up after commit `67f54a6a`:
  `docs/stage1_decomp/stage1_try_again_fail_branch_pro_followup_20260525.md`.
  Audit that file first before revisiting the broader handoff below.
- Latest fail-terminal visual result after commit `1da66d74`:
  `docs/stage1_decomp/stage1_fail_terminal_runner_stop_pro_followup_20260525.md`.
  Black flash is fixed, but fail animation/gameplay/compact rail still keep
  running; audit that runner-stop follow-up before proposing further runtime
  patches.
- Do not switch to `main`, `master`, or older Codex branches.
- Expected patch artifact: one `git apply --index` compatible unified diff.
- Build rule: full build only, `.\build.ps1`. Do not rely on quick builds.

## Current observed state

- Stage1 is on the direct-port route.
- Try Again can appear.
- The transparent gray-black Try Again backdrop now covers 1920x1080 widescreen correctly.
- The compact rail/HUD no longer leaks into the Try Again underlay.
- Remaining visible failures:
  1. A black flash happens just before Try Again appears.
  2. The failure branch still has wrong linkage between gameplay time, action triggers, animation, and music/SFX.

Do not spend the audit on retuning the widescreen rect unless pseudo-C proves it is still wrong. The current open problem is frame sequencing and fail-branch state flow.

## Rules

1. IDA Hex-Rays pseudo-C under `docs/stage1_decomp` is the runtime authority.
2. Do not guess from visual symptoms.
3. Do not add renderer/UI/shell symptom patches.
4. Do not add sentinel, matcher, observer, or harness changes.
5. Do not add a runtime dependency on `SCUS_941.83`.
6. Keep `pr_scenes.cpp` thin. Real behavior belongs in `*_direct.cpp/.h` direct-port modules.
7. If no concrete mismatch is proven, output no code diff and explain the missing evidence.

## Current Win implementation to audit

- Event dispatcher and event4 state:
  - `src/pr/pr_event.cpp`
  - `src/pr/pr_psx_event_frame_direct.cpp`
  - `src/pr/pr_psx_event_frame_direct.h`
  - `src/pr/pr_ui_overlay.cpp`
- Stage1 stopped underlay and base-only draw:
  - `src/pr/pr_scenes.cpp`
  - `src/pr/pr_scn1.h`
  - `src/pr/pr_stage1_scene1_draw_backend.cpp`
  - `src/pr/pr_stage1_scene1_draw_backend.h`
  - `src/pr/pr_stage_scene_submit_backend.cpp`
  - `src/pr/pr_stage_scene_submit_backend.h`
  - `src/pr/pr_stage_scene_submit_direct.cpp`
- Fail lifecycle and timing/action/music flow:
  - `src/pr/pr_stage1_lifecycle_direct.cpp`
  - `src/pr/pr_stage1_lifecycle_executor_direct.cpp`
  - `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp`
  - `src/pr/pr_stage1_runtime_slots_direct.cpp`
  - `src/pr/pr_stage1_scorer_direct.cpp`
  - `src/pr/pr_stage1_script_event_runtime_direct.cpp`
  - `src/pr/pr_stage_runner.cpp`
  - `src/pr/pr_stage_runner_direct.cpp`

## Current direct-port facts already implemented

- `8001E750(event=4)` `gp+0x38C == 1` uses box-fill color `0x400F0F0F`, not the old Win value `0x4000000F`.
- `801CB6BC` scene init writes `MEMORY[0x8006EDCC] = 1`. With `gp=0x8006EA40`, this is `gp+0x38C`.
- Win now carries that Stage1 seed through event4 init/reset once, so `8001E750(event=4)` can see `gp+0x38C == 1`.
- `8001B120(0)` is modeled as a MoveImage/backbuffer side-effect fact, not a generic event4-active renderer path.
- Event-frame render order is currently:
  1. stopped Stage1 direct runtime base-only underlay,
  2. flush,
  3. copied previous-frame event4 fullscreen box-fill,
  4. flush,
  5. current event-frame prompt packets.
- The copied fullscreen box-fill is drawn over the current renderer-visible framebuffer size only for the `8001B120(0)` copied framebuffer case. Prompt packets remain PSX-coordinate event-frame packets.

These facts may be audited, but do not delete them just because they look unusual. They are there to model PSX event-frame and MoveImage ordering.

## Primary pseudo-C evidence

### `80026B94` event frame loop

Source:
`docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_scus_80026B94_lower_helpers_current_p0_20260510.txt`

```c
int __fastcall sub_80026B94(int a1, int a2)
{
  switch (a1) {
    case 4:
      v4 = off_80054564;
      break;
    ...
  }
  if (!v4)
    return 0;
  ((void (__fastcall *)(int (**)(), int, int))*v4)(v4, a1, a2);
  if (a1 == 3)
    sub_80020110(0, 4, 2, 1);
  VSyncCallback(PadChkVsync);
  do
    v7 = 1;
  while (sub_80035510());
  v8 = 0;
  v9 = 1;
  v10 = v4[3];
  v11 = 60;
  *(gp + 816) = 0;
  do {
    if (v7) {
      v12 = sub_80026744();
      if (v12) {
        v9 = 0;
        v8 = v4[1](v12, v4[4], a2);
        if (v8)
          v7 = 0;
      }
    } else {
      --v11;
    }
    if ((int)v4[3] > 0 && v9) {
      v10 = (int (*)())((char *)v10 - 1);
      if ((int)v10 <= 0) {
        v8 = 3;
        v7 = 0;
      }
    }
    if (v4[2])
      v4[2](v4);
    sub_8001E750(a1, v4[4]);
    sub_80035560(0);
    sub_8001EA00(a1);
    sub_800436F0(-1);
  } while (v11 > 0);
  VSyncCallback(0);
  VSyncCallback(0);
  return v8;
}
```

Audit questions:

- Does Win call the event4 tick, draw, `80035560(0)`, `8001EA00(4)`, and `800436F0(-1)` in this order for the Try Again phase?
- Does Win introduce a clear/black frame between gameplay stop and event4 first visible frame?
- Does Win keep or drop the previous gameplay framebuffer at the same point PSX would keep it?

### `800267C8` event4 init

Source:
`docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_fail_prompt_ev4_full.txt`

```c
int __fastcall sub_800267C8(int a1)
{
  v2 = *(_DWORD **)(a1 + 16);
  *(gp + 812) = 2;
  *(gp + 808) = 0;
  *v2 = -1;
  return -1;
}
```

Important: this init does not write `gp+0x38C`.

### `80025E6C` event4 cue/SFX tick

Source:
`docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_fail_prompt_ev4_full.txt`

```c
int sub_80025E6C()
{
  result = *(gp + 812);
  if (result > 0) {
    v2 = *(gp + 808);
    if (v2) {
      if (v2 == 36) {
        sub_80026EF8(dword_8009441C + 6);
        sub_80026ECC();
        --*(gp + 812);
      }
    } else {
      sub_80026EF8(dword_8009441C);
      sub_80026ECC();
    }
    v3 = *(gp + 808);
    result = v3 + 1;
    if (v3 == 72)
      *(gp + 808) = 0;
    else
      *(gp + 808) = result;
  }
  return result;
}
```

Audit questions:

- Does Win preserve the 0/36/72 cue cadence?
- Does fail terminal gameplay stop or music transition happen before or after this cue path in PSX order?
- Is the current Win action/animation trigger still running during frames where PSX has already entered the blocking `80026B94(4,0)` event loop?

### `8001E750(event=4)` draw-state branch

Source:
`docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_fail_prompt_ev4_full.txt`

```c
int __fastcall sub_8001E750(int a1, _DWORD *a2)
{
  v3 = sub_8004019C();
  *(gp + 872) = v3;
  sub_80040F90(dword_8006ED50[v3]);
  sub_80040CC8(0, 0, &unk_80087288 + 5 * *(gp + 872));
  switch (a1) {
    case 4:
      v10 = *(gp + 908);
      if (!v10)
        return sub_800203D4(*a2);
      if (v10 == 1) {
        sub_8001B6C4(0, 0, 320, 240, 0x400F0F0F, 0,
                     &unk_80087288 + 20 * *(gp + 872));
        result = *(gp + 908) + 1;
        *(gp + 908) = result;
      } else {
        result = sub_8001B120(0);
        *(gp + 908) = 0;
      }
      return result;
  }
}
```

### `8001B120(0)` and `80044E2C` MoveImage

Sources:

- `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_fail_prompt_ev4_full.txt`
- `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_event4_helper_closure_20260510.md`

```c
int __fastcall sub_8001B120(int a1)
{
  v1 = sub_8004019C();
  v7 = 15728960;
  LOWORD(v6) = 0;
  if (a1) {
    HIWORD(v6) = v1 ? 0 : 240;
    v4 = v1 != 0;
  } else {
    HIWORD(v6) = v1 ? 240 : 0;
    v4 = v1 == 0;
  }
  return sub_80044E2C(&v6, 0, -v4 & 0xF0, v2, v6, v7);
}
```

```c
int __fastcall sub_80044E2C(int a1, unsigned __int16 a2, int a3)
{
  sub_80044BA8("MoveImage", a1);
  if (*(_WORD *)(a1 + 4) && *(_WORD *)(a1 + 6)) {
    dword_8005D7E8 = (a3 << 16) | a2;
    dword_8005D7E4 = *(_DWORD *)a1;
    dword_8005D7EC = *(_DWORD *)(a1 + 4);
    return sub_800468E0(sub_80046840, dword_8005D7DC, 20, 0);
  }
  return -1;
}
```

Audit questions:

- Is Win clearing the destination framebuffer before the modeled MoveImage copy should appear?
- Should the copied dimmed frame persist for a frame that Win currently replaces with black?
- Does `8001EA00(4)` or `800436F0(-1)` have a side effect missing from Win that would avoid the black flash?

### `801CB6BC` Stage1 scene-init seed

Source:
`docs/stage1_decomp/801CB6BC_sub_801CB6BC.c`

```c
MEMORY[0x800246A8](1, v2, v1, 8);
MEMORY[0x8001DDFC]();
dword_801D3034 = 0;
g_PrStageRunner_UIRender_RefreshRequest = 0;
g_PrStageRunner_UIRender_Block2 = 0;
g_PrStageRunner_UIList_Block = 0;
MEMORY[0x8006EDCC] = 1;
g_PrStageRunner_UIListB_Active = 0;
g_PrStageRunner_UIListC_Active = 0;
g_PrStageRunner_UIListD_Active = 0;
g_PrStageRunner_UIListA_Active = 0;
```

Important: `0x8006EDCC == gp+0x38C`. This is the source of the event4 draw-state seed.

## Specific audit targets

1. Black flash before Try Again:
   - Compare PSX `80026B94(4,0)` frame loop to Win event dispatcher and render loop.
   - Confirm whether the first black frame is caused by route change, missing previous framebuffer, `8001EA00`, `80035560`, `800436F0`, or Stage1 runtime stop order.
   - Patch only a proven PSX ordering/side-effect mismatch.

2. Failure branch timing/action/music linkage:
   - Compare Stage1 fail terminal path from `801C7A60`/`801C81EC` to Win lifecycle.
   - Check whether gameplay continues for frames where PSX should already be inside blocking `80026B94(4,0)`.
   - Check whether teacher/student action triggers continue after fail terminal.
   - Check whether `80025E6C -> 80026EF8/80026ECC` cue cadence and music stop/fade side effects are wired in the same order as PSX.

3. HUD and underlay:
   - Current user feedback says the rail HUD no longer leaks into Try Again after the base-only underlay route. Do not reintroduce HUD.
   - If a patch changes underlay, explain why pseudo-C requires it.

## Output format for web GPT Pro

Return:

1. A concise coverage matrix for the checked fail/Try Again seams.
2. A list of each proven mismatch, with PSX pseudo-C file/function and Win file/function.
3. Exactly one fenced `diff` block containing a complete unified diff applyable from repo root.
4. Include `docs/memory.md` and `docs/待办事项.md` updates if runtime behavior changes.
5. Build status: run or not run.
6. Expected manual checks:
   - no black flash before Try Again;
   - transparent gray-black backdrop still covers 1920x1080;
   - no rail HUD in Try Again underlay;
   - fail terminal stops gameplay/action/music in PSX order;
   - Cross/Yes retry and Circle/No abort still work.

If no safe patch is proven, output no diff and state the exact pseudo-C evidence still missing.

## Paste-ready prompt

```text
You are auditing the PaRappaWin Stage1 direct-port branch.

Repository: https://github.com/2320614999/PaRappaWin
Branch: codex/stage1-gpt55-pro-handoff-20260524
Use the latest pushed HEAD of this branch after the 2026-05-25 handoff commit. Do not switch to main, master, or older Codex branches.

Current user-verified state:
- Try Again appears.
- The transparent gray-black Try Again backdrop now covers 1920x1080 widescreen correctly.
- The compact rail/HUD no longer leaks into the Try Again underlay.
- Remaining bugs:
  1. A black flash happens just before Try Again appears.
  2. The failure branch still has wrong linkage between gameplay time, action triggers, animation, and music/SFX.

Hard rules:
1. IDA Hex-Rays pseudo-C under docs/stage1_decomp is the runtime authority.
2. Do not guess from visual symptoms.
3. Do not add renderer/UI/shell symptom patches.
4. Do not add sentinel/matcher/observer/harness changes.
5. Do not add runtime dependency on SCUS_941.83.
6. Keep pr_scenes.cpp thin. Real behavior belongs in *_direct.cpp/.h direct-port modules.
7. If no concrete mismatch is proven, output no code diff and explain the missing evidence.

Start from:
- docs/项目规则.md
- docs/待办事项.md
- docs/memory.md
- docs/stage1_decomp/stage1_gpt55_handoff_20260524.md
- docs/stage1_decomp/stage1_try_again_fail_branch_gpt_pro_handoff_20260525.md

Primary pseudo-C evidence:
- docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_scus_80026B94_lower_helpers_current_p0_20260510.txt
- docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_fail_prompt_ev4_full.txt
- docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_event4_helper_closure_20260510.md
- docs/stage1_decomp/801CB6BC_sub_801CB6BC.c
- docs/stage1_decomp/801C7A60_sub_801C7A60.c
- docs/stage1_decomp/movie_segment_20260510/ida_comod1_lifecycle_exports_20260510/decompile_801c81ec.txt

Audit target:
Compare the PSX fail/Try Again call order and side effects against the Win direct implementation. Focus on:
- 80026B94(4,0) blocking frame loop;
- event4 table init/input/tick/draw tail;
- 80025E6C cue/SFX cadence;
- 8001E750(event4) gp+0x38C state branch;
- 8001B120(0) -> 80044E2C MoveImage/backbuffer side effect;
- 80035560(0), 8001EA00(4), 800436F0(-1) frame tail;
- Stage1 fail terminal from 801C7A60/801C81EC into event4;
- Win lifecycle points that stop gameplay, hide HUD, stop/change music, and suppress further action triggers.

Current Win files likely involved:
- src/pr/pr_event.cpp
- src/pr/pr_psx_event_frame_direct.cpp
- src/pr/pr_psx_event_frame_direct.h
- src/pr/pr_ui_overlay.cpp
- src/pr/pr_scenes.cpp
- src/pr/pr_scn1.h
- src/pr/pr_stage1_scene1_draw_backend.cpp
- src/pr/pr_stage_scene_submit_backend.cpp
- src/pr/pr_stage_scene_submit_direct.cpp
- src/pr/pr_stage1_lifecycle_direct.cpp
- src/pr/pr_stage1_lifecycle_executor_direct.cpp
- src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp
- src/pr/pr_stage1_runtime_slots_direct.cpp
- src/pr/pr_stage1_scorer_direct.cpp
- src/pr/pr_stage1_script_event_runtime_direct.cpp
- src/pr/pr_stage_runner.cpp
- src/pr/pr_stage_runner_direct.cpp

Return:
1. A concise coverage matrix for the checked fail/Try Again seams.
2. A list of every proven mismatch, with PSX pseudo-C file/function and Win file/function.
3. Exactly one fenced diff block containing a complete unified diff applyable from repo root with git apply --index.
4. Include docs/memory.md and docs/待办事项.md updates if runtime behavior changes.
5. Build status: run or not run.
6. Manual checks expected after applying the patch:
   - no black flash before Try Again;
   - transparent gray-black backdrop still covers 1920x1080;
   - no rail HUD in Try Again underlay;
   - fail terminal stops gameplay/action/music in PSX order;
   - Cross/Yes retry and Circle/No abort still work.
```
