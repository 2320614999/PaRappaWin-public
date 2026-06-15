# Stage1 COOL re-entry dojo restore Pro result 2026-05-31

ChatGPT Pro conversation:
https://chatgpt.com/c/6a1a6d24-c370-83a3-b2e1-628daf748dd3

Input handoff:
`docs/stage1_decomp/cool_reentry_dojo_restore_20260531/pro_handoff_cool_reentry_dojo_restore_20260531.md`

## Verdict

Pro judged the evidence sufficient for a minimal direct-port diagnostic patch.

The likely bug is not renderer/audio/HD cleanup. The high-confidence carrier mismatch is:

```text
restartFlag2000SceneFamily
=> sceneInitPulse801CB6BC
=> coord bank/init gate reset
=> ApplyStage1SceneInitGateReset801CB6BC clears UIList_Block
=> 801CB190 temporarily falls through to normal/full-building submit membership
=> already-broken dojo appears restored, then gets broken/hidden again
```

PSX evidence supports a narrower operation:

```text
second COOL entry
=> 801C9094 chooses stream8 when ctx+0x72 >= 2
=> 801C895C chooses row8
=> 801CBFDC handles ctx.flags & 0x2000
=> reload ctx+0x134..0x144 TOD/UI-list sources
=> if ctx+0x72 >= 2, set UIRender_RefreshRequest=1 and UIRender_Block2=0
=> preserve existing UIList/full-building submit membership unless another PSX block changes it
```

## Patch Shape

Split the current broad pulse into two explicit concepts:

```cpp
// PSX 801CBFDC dynamic row/TOD reload pulse.
runtime.sceneSubmit.flag2000DynamicRowReload801CBFDC = true;

// Full 801CB6BC coord/list init reset.
runtime.sceneSubmit.sceneInitPulse801CB6BC = true;
```

For `flag2000` stream6/stream8 re-entry where the scene-map submit family is unchanged, issue only the `801CBFDC` dynamic row/TOD reload pulse. Do not issue `sceneInitPulse801CB6BC`.

The guard must compare the stable scene-map/submit-membership tail only, not the actor TOD heads:

```text
tail handles: 18 17 04 03 02
resources: TORO / TENNJOU / KABE_R / KABE_L / KABE_B
```

Do not use a generic row equality helper if it compares all row handles, because row6 and row8 are expected to differ in `ON_TO/PA_TO` versus `ON_TOA/PA_TOA`.

## Expected Trace After Patch

```text
second COOL re-entry:
  ctx72 >= 2
  selectedStream = 8
  one-shot 0x2000 reaches 801CBFDC
  UIRender_RefreshRequest = 1
  UIRender_Block2 = 0
  sceneInitPulse801CB6BC = false for row6/row8 same scene-map tail
  coordBankInitialized801CB6BC is not forcibly reset
  UIList_Block is preserved
  801CB190 does not emit TENNJOU/KABE_R/KABE_B/KAMON_* for the transient frame
```

## Validation Data To Log

Minimum useful local proof for one COOL drop/re-entry sequence:

```text
scriptFrame
ctx72
flag2000Pulse
selectedStream
row id
row handles
scene-map tail handles
flag2000DynamicRowReload801CBFDC
sceneInitPulse801CB6BC
coordBankInitialized801CB6BC
UIList_Block before/after ApplyStage1SceneInitGateReset801CB6BC
801CB190 submitted object list
```

Pro's proving signature:

```text
before patch:
  second COOL re-entry
  sceneInitPulse801CB6BC = true
  UIList_Block temporarily false
  801CB190 emits TENNJOU/KABE_*/KAMON_*

after patch:
  second COOL re-entry
  selectedStream = 8
  one-shot 0x2000 reaches 801CBFDC
  sceneInitPulse801CB6BC = false for same scene-map tail
  UIList_Block does not get cleared
  801CB190 stays on COOL/UIList membership
```

## Local Code Landmarks

Current suspected broad pulse:

- `src/pr/pr_stage1_runtime_slots_direct.cpp`: `ApplyStage1RuntimeSceneRowFromScriptEvent`, around the `restartFlag2000SceneFamily` branch.
- `src/pr/pr_stage_scene_submit_direct.cpp`: `ApplyStage1SceneInitGateReset801CB6BC`, which currently clears `g_PrStageRunner_UIList_Block` and active UIList state.
- `src/pr/pr_stage_scene_submit_direct.cpp`: subframe advance path where `sceneInitPulse801CB6BC` clears `coordBankInitialized801CB6BC`.

Likely structural additions:

- Add `flag2000DynamicRowReload801CBFDC` beside `sceneInitPulse801CB6BC` in scene-submit runtime state/snapshot structs.
- Carry it into scene submit context construction as the one-shot `0x2000` source.
- Leave `sceneInitPulse801CB6BC` for true full scene/list init only.

## Risks

1. PSX may clear `UIList_Block` outside the pasted `801CBFDC` snippet.
2. `ON_TOA/PA_TOA` actor TOD heads may require an `801CB6BC` coord-bank reset not shown in the current evidence.
3. The visible restore flash may be a TOD replay, not only `UIList_Block` clearing.
4. A row equality helper that compares actor TOD heads will fail the row6 -> row8 case and accidentally keep the broad reset.
