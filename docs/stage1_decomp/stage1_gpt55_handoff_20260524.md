# Stage1 direct-port handoff for GPT-5.5 Pro

## Repository and branch

- Repo: `https://github.com/2320614999/PaRappaWin`
- Handoff branch: `codex/stage1-gpt55-pro-handoff-20260524`
- Handoff base commit: `85eebdf9`
- Important: inspect this handoff branch only. Do not base the audit on `main`,
  `master`, or the older `codex/stage1-shell-cleanup` branch.
- Build command: run the full build only: `.\build.ps1`
- Current output path after successful full build:
  `E:\game\PSgame\parappa the rapper\PaRappaWin.exe`

## Project rules that matter

- IDA Hex-Rays pseudo-C is the authority for runtime logic.
- Memory replay / PSX recordings are useful for locating entry frames and validating behavior, not for inventing Win-side behavior.
- Do not add renderer/UI/shell symptom patches for Stage1 behavior bugs.
- Do not add sentinel / matcher / observer / harness changes for this pass.
- Do not add runtime dependencies on `SCUS_941.83`; it is an offline reverse-engineering source only.
- Keep `pr_scenes.cpp` as thin as possible. Prefer moving logic into `*_direct.cpp/.h` direct-port modules.
- Full build only; do not rely on quick builds.

## Current high-level status

- Stage1 now mostly runs through the direct-port path.
- Recent visual fixes that are already on the branch:
  - Stage1 flat primitive color scale corrected for most wall / frame / background color differences.
  - Stage1 facial-expression TIM delay path restored.
  - Stage1 rail portrait movement mostly smoothed by fixing timecode authority.
  - Stage1 rap/action replay double-trigger fixed.
  - Stage1 ninja onion / mid-floor trigger animation fixed.
- Current blocking area:
  - There are still many Stage1 wiring gaps.
  - Failure / Try Again branch is incomplete. Try Again can appear, but gameplay pause timing is wrong, gameplay HUD may still leak, and the Try Again backdrop dim/half-black state does not match PSX.
  - Broader fail / pass / alternate branches still need a systematic pseudo-C-to-Win wiring audit.

## Most relevant current code

- Stage1 lifecycle / scene driver:
  - `src/pr/pr_scenes.cpp`
  - `src/pr/pr_stage1_lifecycle_direct.cpp`
  - `src/pr/pr_stage1_lifecycle_executor_direct.cpp`
  - `src/pr/pr_stage1_lifecycle_host_adapter_801c81ec.cpp`
  - `src/pr/pr_stage_runner.cpp`
  - `src/pr/pr_stage_runner_direct.cpp`
- Stage1 scorer / timing / terminal facts:
  - `src/pr/pr_stage1_scorer_direct.cpp`
  - `src/pr/pr_stage1_scorer_host.cpp`
  - `src/pr/pr_stage1_scorer_host_direct.cpp`
  - `src/pr/pr_stage1_script_event_runtime_direct.cpp`
  - `src/pr/pr_stage1_runtime_slots_direct.cpp`
- Stage1 scene submit / draw:
  - `src/pr/pr_stage_scene_submit_direct.cpp`
  - `src/pr/pr_stage_scene_submit_backend.cpp`
  - `src/pr/pr_stage1_scene1_draw_backend.cpp`
  - `src/pr/pr_psx_event_frame_direct.cpp`
  - `src/pr/pr_psx_fast_sprite_submit_direct.cpp`
  - `src/pr/pr_psx_gte_direct.cpp`
- Host UI / remaining shell risk:
  - `src/pr/pr_ui_overlay.cpp`
  - `src/pr/pr_stage1_fail_prompt_direct.cpp`
  - `src/d3d11_renderer.cpp`

## Pseudo-C and evidence entry points

The repo currently has more than 500 files under `docs/stage1_decomp/`; about
400 are pseudo-C / IDA export / analysis text files. Start with these instead
of scanning everything at once.

### Stage1 core lifecycle / failure branch

- `docs/stage1_decomp/801C7A60_sub_801C7A60.c`
- `docs/stage1_decomp/801C79E4_sub_801C79E4.c`
- `docs/stage1_decomp/movie_segment_20260510/ida_comod1_lifecycle_exports_20260510/decompile_801c81ec.txt`
- `docs/stage1_decomp/comod_status_coupled_pseudocode_20260511/S1_COMOD1/decompile_801c81ec.txt`
- `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_comod1_801C7A60_801C81EC_p0_20260510.txt`
- `docs/stage1_decomp/801c81ec_lifecycle_callsite_table_20260514.md`
- `docs/stage1_decomp/801c81ec_lifecycle_followup_boundaries_800916e0_80015590_80019148_20260514.md`

### Event dispatcher / Try Again / event4

- `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_fail_prompt_ev4_full.txt`
- `docs/stage1_decomp/fail_prompt_ev4_20260510/event4_prompt_sprite_packet_plan_20260510.md`
- `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_scus_80026B94_lower_helpers_current_p0_20260510.txt`
- `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_event4_helper_closure_20260510.txt`
- `docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_event4_helper_closure_20260510.md`
- `docs/stage1_decomp/ida_export_stageclear_menuhelp_80026b94_800916f6_20260513.txt`

### Stage1 script / scorer / timing / terminal state

- `docs/stage1_decomp/801C9094_sub_801C9094.c`
- `docs/stage1_decomp/801C85DC_sub_801C85DC.c`
- `docs/stage1_decomp/stage1_script_box_timing_801c9094_20260518.md`
- `docs/stage1_decomp/stage1_timecode_801c7560_source_20260515.md`
- `docs/stage1_decomp/ida_export_801c7560_timecode_source_20260515.txt`
- `docs/stage1_decomp/stage1_hud_timed_20260523/decompile_801c872c.txt`
- `docs/stage1_decomp/stage1_action_submit_20260524/decompile_801ca0e0.txt`
- `docs/stage1_decomp/stage1_action_submit_20260524/decompile_801c9094.txt`
- `docs/stage1_decomp/stage1_action_submit_20260524/decompile_801c895c.txt`
- `docs/stage1_decomp/stage1_action_submit_20260524/decompile_801c88c8.txt`
- `docs/stage1_decomp/stage1_action_submit_20260524/decompile_801c8888.txt`

### Stage1 scene submit / actor / floor / animation

- `docs/stage1_decomp/ida_fresh_801cb6bc_20260524/decompile_801cbfdc.txt`
- `docs/stage1_decomp/ida_fresh_801cb6bc_20260524/decompile_801cb6bc.txt`
- `docs/stage1_decomp/ida_fresh_801cb6bc_20260524/decompile_801cb190.txt`
- `docs/stage1_decomp/ida_fresh_801cb6bc_20260524/decompile_801cab34.txt`
- `docs/stage1_decomp/ida_export_stage1_runner_alignment_20260521/decompile_801cbfdc.txt`
- `docs/stage1_decomp/ida_export_stage1_runner_alignment_20260521/decompile_801cb190.txt`
- `docs/stage1_decomp/ida_export_stage1_runner_alignment_20260521/decompile_801c7a60.txt`
- `docs/stage1_decomp/scene_submit_draw_exports_20260512/decompile_801cac34.txt`
- `docs/stage1_decomp/tmp_20260521_camera_recheck/decompile_801cbfdc.txt`
- `docs/stage1_decomp/tmp_20260521_camera_recheck/decompile_801cac34.txt`
- `docs/stage1_decomp/tmp_20260521_camera_recheck/decompile_801c7a60.txt`

### Movie1 / gameplay transition / transition tiles

- `docs/stage1_decomp/ida_export_movie1_stage_loop_pseudocode_authority_20260520.txt`
- `docs/stage1_decomp/ida_export_movie1_transition_800201ac_8001a4d0_20260520.txt`
- `docs/stage1_decomp/movie1_gameplay_transition_calltrace_800201ac_80020110_20260520.md`
- `docs/stage1_decomp/ida_export_transition_core_80020110_8001ea74_8001ebf4_80020008_80020090_20260520.txt`
- `docs/stage1_decomp/ida_export_transition_tile_helpers_8001f524_8001fdc0_8001ef40_20260520.txt`

### Rail / GTE / portrait movement

- `docs/stage1_decomp/stage1_compact_rail_pseudoc_20260521.md`
- `docs/stage1_decomp/rail_tail_801c9094_writer_clock_gap_20260515.md`
- `docs/stage1_decomp/rail_gte_live_8003f710_positive_depth_cutover_20260513.md`
- `docs/stage1_decomp/ida_export_rail_tail_compact_writers_current_idb_20260515.txt`
- `docs/stage1_decomp/ida_export_rail_tail_compact_writers_801c9334_801c933c_801c947c_801c9cc4_801c9e70_20260515.txt`

## Suggested GPT-5.5 Pro patch-only prompt

Paste this as the task prompt after connecting the GitHub repository. This
version assumes the web model can inspect the repo but cannot push commits, so
the expected artifact is a `git apply`-compatible unified diff.

```text
You are auditing the PaRappaWin Stage1 direct-port branch.

Repository: https://github.com/2320614999/PaRappaWin
Branch to inspect: codex/stage1-gpt55-pro-handoff-20260524
Base commit: 85eebdf9

Use only this branch and this base commit for the patch. Do not switch to main,
master, or codex/stage1-shell-cleanup. If the connected GitHub UI opens another
branch by default, switch to codex/stage1-gpt55-pro-handoff-20260524 before
auditing or generating diff.patch.

Goal:
Audit all Stage1 wiring/direct-port seams, not only the current Try Again symptom. Compare the Stage1 pseudo-C/control-flow evidence against the Win implementation and wiring. For every concrete mismatch you can prove, include the smallest direct-port fix in one complete patch file named diff.patch. The patch must be a unified diff that can be applied locally with git apply.

Do not stop after finding one local bug. Build a Stage1 wiring coverage matrix first, then patch all proven mismatches in the same diff.patch.

Coverage matrix scope:
- lifecycle / scene loop: 801C81EC, 801C7A60, 801C79E4, 8001A4D0 and related host-block transitions
- fail / Try Again / event4: 80026B94(4,0), event frame tail, prompt selection, backdrop box fill, input/result handling
- clear / pass / abort / retry branches: stage result propagation, 800916E0, transition calls, reset/restore side effects
- script/scorer/timecode: 801C9094, 801C872C, 801C7560, action submit, terminal stream pulses, accepted/page/bucket state
- scene submit / actors / floor / animation: 801CBFDC, 801CB190, 801CB6BC, 801CAB34, 801CAC34 and already-exported helpers
- movie1 -> gameplay transition: 800201AC, 80020110, transition tile helpers, stage-loop pre/post seams
- rail / GTE / portrait / HUD / face-expression wiring where pseudo-C evidence already exists
- resource/lifecycle side effects that are already translated but not connected

Hard rules:
1. IDA Hex-Rays pseudo-C under docs/stage1_decomp is the authority.
2. Do not guess from visual symptoms. Use pseudo-C/control-flow first.
3. Do not add renderer/UI/shell symptom patches.
4. Do not add sentinel/matcher/observer/harness changes.
5. Do not add runtime dependency on SCUS_941.83.
6. Keep pr_scenes.cpp thin; move real behavior into *_direct.cpp/.h direct-port modules.
7. If a module is repeatedly failing, expand the direct-port boundary instead of patching the shell.
8. If you cannot run the full build, say "build not run" explicitly. Do not claim the patch is verified.

Start here:
- docs/项目规则.md
- docs/待办事项.md
- docs/memory.md
- docs/stage1_decomp/stage1_gpt55_handoff_20260524.md

Primary pseudo-C evidence:
- docs/stage1_decomp/801C7A60_sub_801C7A60.c
- docs/stage1_decomp/movie_segment_20260510/ida_comod1_lifecycle_exports_20260510/decompile_801c81ec.txt
- docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_comod1_801C7A60_801C81EC_p0_20260510.txt
- docs/stage1_decomp/fail_prompt_ev4_20260510/ida_export_fail_prompt_ev4_full.txt
- docs/stage1_decomp/801C9094_sub_801C9094.c
- docs/stage1_decomp/ida_fresh_801cb6bc_20260524/decompile_801cbfdc.txt
- docs/stage1_decomp/ida_fresh_801cb6bc_20260524/decompile_801cb190.txt
- docs/stage1_decomp/ida_fresh_801cb6bc_20260524/decompile_801cab34.txt
- docs/stage1_decomp/stage1_action_submit_20260524/decompile_801ca0e0.txt
- docs/stage1_decomp/stage1_hud_timed_20260523/decompile_801c872c.txt

Current observed problem:
Try Again can appear, but gameplay pause timing is wrong, gameplay HUD may still leak, and Try Again backdrop dim/half-black is missing. There are likely broader Stage1 branch wiring gaps.

Output requirements:
1. Start with a concise Stage1 wiring coverage matrix. For each checked seam, mark one of:
   - match
   - mismatch patched
   - mismatch not patched, with reason
   - evidence missing
2. After the matrix, list each concrete pseudo-C mismatch that is patched, with PSX function/file references and Win file/function references.
3. Then output exactly one complete unified diff in a fenced `diff` block. Treat this as the contents of diff.patch. It must be applyable from repository root with:
   git apply --index diff.patch
4. The diff.patch should include all proven Stage1 wiring fixes found in this audit, not just the current Try Again bug.
5. Include docs/memory.md and docs/待办事项.md updates inside diff.patch when the code changes runtime behavior.
6. Do not put explanations inside the diff. Put all rationale before or after the diff.
7. End with:
   - Build status: run or not run.
   - Expected manual visual checks.
   - Known remaining gaps, if any.
8. If the safest conclusion is "no code change", output no diff and explain the exact pseudo-C evidence still missing.
```

## Local application workflow for Codex after receiving the patch

1. Save the web model's fenced diff block as `diff.patch` outside the repo or
   in a scratch path that will not be committed.
2. Run `git apply --check diff.patch`.
3. Apply with `git apply --index diff.patch` only after the check passes.
4. Review the staged diff against the pseudo-C references named in the audit.
5. Run the full build with `.\build.ps1`.
6. Let the user visually verify Stage1 before merging the patch back into the
   working branch.

## What not to do

- Do not "fix" Try Again dim by changing RGB constants in the renderer.
- Do not keep adding fallback rendering to `pr_ui_overlay.cpp`.
- Do not use Win-side duration, current HUD state, or visual disappearance as authority for fail/clear branch result.
- Do not treat replay-only frame samples as proof of logic; use them only to locate the pseudo-C entry path.
- Do not stage scratch directories such as raw screenshot frame dumps or `tools/stage1_mem_probe` unless explicitly needed by the branch.
