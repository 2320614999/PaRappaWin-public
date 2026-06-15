# Stage1 COOL re-entry dojo restore Pro handoff 2026-05-31

## User-visible symptom

After the player falls out of COOL, the next promotion back into COOL starts a
transition where the dojo/building that was already broken apart briefly appears
restored, then gets broken apart again. The user suspects this is caused by an
incomplete pseudo-C alignment, not a renderer issue.

Scope for this handoff:

- Stay on the direct-port `801C9094 -> 801C895C -> 801CBFDC/801CB190` path.
- Do not propose renderer, D3D, HD geometry cleanup, texture cache, audio, or
  input patches.
- Treat ChatGPT Web Pro as an external reviewer only. It cannot inspect local
  files or run tests; all evidence is pasted below.

## Existing PSX facts

`801C9094` chooses the GOOD->COOL event stream from `ctx+0x72`:

```c
if ((ctx_flags & 0x2000) != 0) {
    stream = 8;
    if (*(s16 *)(ctx + 0x72) < 2)
        stream = 6;
    g_PrStageEventStreamId = stream;
}
PrStageRunner_EventStream_Reset(ctx, g_PrStageEventStreamId);
// then resets compact teacher/student cursors and calls 80024390/80024308
```

`801C895C` row selection already established in project notes:

```text
ctx->flags & 0x20                         -> row1
ctx->flags & 0x40 && dword_8008ED24 != 1  -> row4
ctx->flags & 0x40 && dword_8008ED24 == 1  -> row5
ctx->flags & 0x2000 && ctx+0x72 < 2       -> row6
ctx->flags & 0x2000 && ctx+0x72 >= 2      -> row8
ctx->flags & 0x4000                       -> row7
```

The relevant stream descriptor rows:

```text
stream6/row6: 0E 14 00 18 17 04 03 02
row8:        0F 15 00 18 17 04 03 02

row6 resources: ON_TO.TOD / PA_TO.TOD / 0 / TORO / TENNJOU / KABE_R/L/B
row8 resources: ON_TOA.TOD / PA_TOA.TOD / 0 / TORO / TENNJOU / KABE_R/L/B
```

`801CBFDC` consumes the selected scene row through `ctx+0x134..0x144`:

```c
if ((*ctx & 0x2000) != 0) {
    for (i = 0; i < 5; i++) {
        sub_8001AFD8(ctx[0x4D + i], &ui_list_enable[i],
                     &g_PrStageRunner_UIList_InitCounter);
    }
    if (*(s16 *)(ctx + 0x72) >= 2) {
        g_PrStageRunner_UIRender_RefreshRequest = 1;
        g_PrStageRunner_UIRender_Block2 = 0;
    }
}
```

`801CB190` then draws different submit lists depending on
`g_PrStageRunner_UIList_Block`:

```c
if (g_PrStageRunner_UIList_Block) {
    sub_8001B084(TOUROU, 1, W3244, 10);
    sub_8001B084(AUX_7FF8, 3, W3244, 10);
} else {
    sub_8001B084(TOUROU, 1, W80A8, 8);
    sub_8001B084(TENNJOU, 1, W8058, 8);
    sub_8001B084(KABE_R, 2, W8080, 8);
    sub_8001B084(KABE_B, 1, W80A8, 8);
    sub_8001B084(KAMON_L, 7, W3244, 10);
}
```

Earlier live Win scene-submit samples showed the COOL raised-camera / UIList
path submitting only:

```text
PA / FLOOR / TAMA / TOUROU / AUX_7FF8 / KAGE
```

and normal/non-UIList path submitting dojo pieces including:

```text
TENNJOU / KABE_R / KABE_L / KABE_B / KAMON_*
```

So the visible "dojo restored" symptom likely means Win temporarily re-enters
the non-UIList/full-building membership or replays an initial TOD/row phase that
PSX would not replay on a second COOL entry.

## Current Win carrier summary

Event stream routing:

```cpp
uint8_t PrStageEventDirectStage1Flag2000StreamForCtx72(uint16_t ctx72) {
    return ctx72 < 2u ? 6u : 8u;
}

if (input.flag2000Pulse) {
    SetStage1EventStreamFlag(runtime, 0x2000u, true);
    runtime.ctx72 = input.ctx72;
}
...
const bool flag2000Keep = input.flag2000Pulse;
SetStage1EventStreamFlag(runtime, 0x2000u, flag2000Keep);
...
if (HasFlag(runtime, 0x2000u)) {
    streamId = ResolveStage1EventStreamFollowUpStreamForFlag2000(runtime.ctx72);
    StartStage1EventStreamDispatch(..., streamId, scriptFrame, scriptFrame);
    ConsumeStartedStage1EventStreamDispatch801C9094(...);
    runtime.flag2000SceneFamilyActive = true;
    runtime.flag2000SceneFamilyStream = streamId;
    runtime.flag2000SceneFamilyStartFrame = scriptFrame;
    runtime.flag2000SceneFamilyStartScriptFrame = scriptFrame;
}
```

Runtime slot scene row application:

```cpp
scene.sharedTodListEnabled =
    (lateBranch.ctxFlags40_2000_4000 & 0x2000u) != 0u ||
    lateBranch.flag2000SceneFamilyActive;

const bool restartFlag2000SceneFamily =
    scene.sharedTodListEnabled &&
    flag2000SceneFamilyStartFrame != 0u &&
    (flag2000SceneFamilyStartFrame > runtime.scene.streamStartFrame ||
     lateBranch.flag2000SceneFamilyStream != runtime.scene.selectedStream);

if (restartFlag2000SceneFamily) {
    runtime.sceneSubmit.sceneInitPulse801CB6BC = true;
    scene.streamStartFrame = flag2000SceneFamilyStartFrame;
    scene.selectedStream = lateBranch.flag2000SceneFamilyStream != 0u
                               ? lateBranch.flag2000SceneFamilyStream
                               : streamId;
} else if (scene.sharedTodListEnabled &&
           runtime.scene.sharedTodListEnabled &&
           Stage1SceneRowHandlesEqual(scene.rowHandles, runtime.scene.rowHandles)) {
    scene.streamStartFrame = runtime.scene.streamStartFrame;
    scene.streamWindowFrames = runtime.scene.streamWindowFrames;
    scene.selectedStream = runtime.scene.selectedStream;
}
runtime.scene = scene;
```

Scene submit context construction:

```cpp
constexpr uint32_t kOneShotCtxFlags801CBFDC =
    0x00800000u | 0x00002000u | 0x00000400u;

ctx.flags_00 =
    (runtimeSlots.sceneSubmit.ctxFlagsFull & ~kOneShotCtxFlags801CBFDC) |
    (oneShotPulseFlags & kOneShotCtxFlags801CBFDC) |
    runtimeSlots.sceneSubmit.ctxFlags40_2000_4000;
ctx.ctx72_72 = runtimeSlots.sceneSubmit.ctx72;
```

Scene submit advance:

```cpp
if (renderSubFrame8 == 0u && runtimeSlots.sceneSubmit.sceneInitPulse801CB6BC) {
    runtimeState.coordBankInitialized801CB6BC = false;
}
```

`801CB6BC` init gate reset in Win clears render/list globals:

```cpp
g.dword_801D3034 = 0;
g.g_PrStageRunner_UIRender_RefreshRequest = false;
g.g_PrStageRunner_UIRender_Block2 = false;
g.g_PrStageRunner_UIList_Block = false;
g.g_PrStageRunner_UIListA/B/C/D_Active = false;
```

Follow-up facts from scorer:

```cpp
facts.compactRailRowCount8AClear80024FD0 = followUp.flag2000Pulse;
facts.compactPainterGate7A = state.rightRankBucketContext.ctx7A;
```

## Suspect mismatch to review

The first GOOD->COOL entry was recently fixed by making `0x2000` a one-shot
`801CBFDC` dynamic-row reload and by keeping the flag2000 scene family start on
`scriptFrame`. That prevented repeated row reload while stream6/8 stayed active.

The new symptom only appears after leaving COOL and later entering COOL again.
That points at a re-entry boundary rather than the first-entry camera path:

1. On the second promotion, `ctx+0x72` may now be `>= 2`, so PSX should choose
   stream/row8 (`ON_TOA/PA_TOA`) instead of replaying row6 (`ON_TO/PA_TO`).
2. Win's `restartFlag2000SceneFamily` deliberately sets
   `sceneInitPulse801CB6BC` whenever the flag2000 stream changes or the family
   start frame increases.
3. That pulse forces the scene submit coord bank to reinitialize, and Win's
   `ApplyStage1SceneInitGateReset801CB6BC` also clears `UIList_Block` and
   related state.
4. A brief cleared `UIList_Block` would route `801CB190` through the normal
   full-building submit list, making the dojo appear restored before the
   COOL/UIList path hides/breaks it again.

This is only a hypothesis; please test it against the pseudo-C order.

## Questions for Pro

1. Does the PSX pseudo-C justify resetting the `801CB6BC` coord bank or clearing
   `UIList_Block` when a second flag2000 scene-family stream starts, or should
   only `ctx+0x134..144` TOD sources be reloaded by the `801CBFDC & 0x2000`
   block?
2. Is Win's `restartFlag2000SceneFamily -> sceneInitPulse801CB6BC` too broad for
   the second GOOD->COOL path, especially when row6 and row8 share the same
   scene-map handles `18 17 04 03 02` and differ only in actor TOD
   `ON_TO/PA_TO` vs `ON_TOA/PA_TOA`?
3. Should `flag2000SceneFamilyActive` preserve the previous UIList/submit
   membership across stream6->stream8 or COOL drop/re-entry, while only changing
   actor TOD start frame/stream?
4. If a safe patch is justified from this evidence, output the smallest direct
   port patch shape. If not justified, output `no safe patch` and list the exact
   missing PSX/live evidence.

Please avoid renderer fixes. The desired answer is an audit of the pseudo-C
alignment and the narrow carrier that can explain "already broken dojo briefly
restores, then breaks again" on second COOL entry.
