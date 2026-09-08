#include "pr_scene_bootstrap_direct.h"

namespace PrSceneBootstrapDirect {

ResetGraphState800446A0 PsxCall800446A0_ResetGraphRuntime(int32_t inputArg) {
    ResetGraphState800446A0 out{};
    out.sourceKnown = true;
    out.inputArg = inputArg;
    out.modeLow3 = static_cast<uint32_t>(inputArg) & 7u;
    // The 8001C470 startup seam calls ResetGraph(0).  The original function
    // only takes the reset path for modes 0 and 3; other modes tail-call its
    // display helper and must remain fail-closed here.
    if (out.modeLow3 != 0u && out.modeLow3 != 3u) {
        out.unsupportedModeRejected = true;
        return out;
    }

    out.accepted = true;
    out.environmentZeroed = true;
    out.environmentBytes = kResetGraphEnvironmentBytes800446A0;
    out.resetCallbackCalled = true;
    out.gpuControlWordSubmitted = true;
    // IDA shows GPU_cw((uint)&off_8005D6EC & 0xFFFFFF).  Keep the PSX
    // source address in the transaction and do not write host GPU registers.
    out.gpuCommandListAddress =
        kResetGraphGpuCommandListAddress8005D6EC & 0x00FFFFFFu;
    out.displayInitCalled = true;
    out.displayInitArg = out.modeLow3 != 0u ? 1 : 0;
    // sub_80046EC0 is a hardware display initializer; no current PSV-memory
    // replay or host HAL evidence proves its return byte.
    out.displayInitReturnKnown = false;
    out.mainEnvironmentCleared = true;
    out.mainEnvironmentBytes = kResetGraphMainEnvironmentBytes800446A0;
    out.tailEnvironmentCleared = true;
    out.tailEnvironmentBytes = kResetGraphTailEnvironmentBytes800446A0;
    out.softwareStateCommitted = true;
    return out;
}

static bool IsExactBootstrapGraphState8001C470(
    const PrPsxGraphOwnerDirect::PsxGraphState& graph) {
    using namespace PrPsxGraphOwnerDirect;
    return IsExactTmdFastHandlerTable8001C1E8(graph) &&
           graph.word_80096590 == 0u &&
           graph.word_800965A0 == 4u &&
           graph.word_800928D4 == 320 &&
           graph.word_800928D6 == 240 &&
           graph.word_8008ECA8 ==
               (std::array<int16_t, 2>{{0, 0}}) &&
           graph.word_8008ECAC ==
               (std::array<int16_t, 2>{{0, 240}}) &&
           graph.word_8008EEF0 ==
               (std::array<int16_t, 2>{{0, 0}}) &&
           graph.word_8008EEF4 ==
               (std::array<int16_t, 2>{{0, 0}}) &&
           graph.word_800901C4 == 160 &&
           graph.word_800901C6 == 120 &&
           graph.drawOffset.setDrawEnvCalled &&
           graph.drawOffset.word_800917AA == 0 &&
           graph.drawOffset.word_800917AC == 0 &&
           graph.drawOffset.word_80091730 == 0 &&
           graph.drawOffset.word_80091732 == 0 &&
           graph.drawOffset.word_80091734 == 320 &&
           graph.drawOffset.word_80091736 == 240 &&
           graph.drawOffset.word_80091738 == 160 &&
           graph.drawOffset.word_8009173A == 120 &&
           graph.gte.geomScreenKnown &&
           graph.gte.geomScreen == 440u &&
           graph.gte.geomOffsetKnown &&
           graph.gte.geomOffsetX == 0 &&
           graph.gte.geomOffsetY == 0 &&
           graph.gte.depthCueKnown &&
           graph.gte.depthCueA == -4194 &&
           graph.gte.depthCueB == 0x01400000 &&
           graph.gte.zScaleFactorKnown &&
           graph.gte.zScaleFactor3 == 341 &&
           graph.gte.zScaleFactor4 == 256 &&
           graph.mainPageWorkLists80087288Initialized &&
           graph.mainPageWorkLists80087288[0].work.order_00 == 14u &&
           graph.mainPageWorkLists80087288[0].work.headAddr_04 ==
               0x80088288u &&
           graph.mainPageWorkLists80087288[1].work.order_00 == 14u &&
           graph.mainPageWorkLists80087288[1].work.headAddr_04 ==
               0x80098288u;
}

static bool IsExactResetGraphState8001C470(
    const ResetGraphState800446A0& resetGraph) {
    return resetGraph.sourceKnown && resetGraph.accepted &&
           resetGraph.inputArg == 0 && resetGraph.modeLow3 == 0u &&
           !resetGraph.unsupportedModeRejected &&
           resetGraph.environmentZeroed &&
           resetGraph.environmentAddress ==
               kResetGraphEnvironmentAddress8005D734 &&
           resetGraph.environmentBytes ==
               kResetGraphEnvironmentBytes800446A0 &&
           resetGraph.resetCallbackCalled &&
           resetGraph.gpuControlWordSubmitted &&
           resetGraph.gpuCommandListAddress ==
               (kResetGraphGpuCommandListAddress8005D6EC & 0x00FFFFFFu) &&
           resetGraph.displayInitCalled &&
           resetGraph.displayInitArg == 0 &&
           resetGraph.mainEnvironmentCleared &&
           resetGraph.mainEnvironmentAddress ==
               kResetGraphMainEnvironmentAddress8005D744 &&
           resetGraph.mainEnvironmentBytes ==
               kResetGraphMainEnvironmentBytes800446A0 &&
           resetGraph.tailEnvironmentCleared &&
           resetGraph.tailEnvironmentAddress ==
               kResetGraphTailEnvironmentAddress8005D7A0 &&
           resetGraph.tailEnvironmentBytes ==
               kResetGraphTailEnvironmentBytes800446A0 &&
           resetGraph.softwareStateCommitted &&
           !resetGraph.hardwareResetHalAuthority &&
           !resetGraph.hostProjection &&
           !resetGraph.replayValueAuthority &&
           !resetGraph.oldWinS0Authority &&
           !resetGraph.stage2PlusAuthority &&
           !resetGraph.comod2Authority;
}

static bool IsExactPadInitState8001C470(
    const PrPsxPadDirect::PadInitState800354C0& padInit) {
    return padInit.sourceKnown && padInit.accepted &&
           padInit.inputArg == 0 &&
           padInit.modeGlobalAddress ==
               PrPsxPadDirect::kPadModeGlobal80091830 &&
           padInit.modeGlobalValue == 0 &&
           padInit.statusGlobalAddress ==
               PrPsxPadDirect::kPadStatusGlobal800882F0 &&
           padInit.statusGlobalValue == -1 &&
           padInit.resetCallbackCalled && padInit.padInit2Called &&
           padInit.padInit2Protocol ==
               PrPsxPadDirect::kPadInit2Protocol800354C0 &&
           padInit.padInit2StatusAddress ==
               PrPsxPadDirect::kPadStatusGlobal800882F0 &&
           padInit.changeClearPadCalled && padInit.changeClearPadArg == 0 &&
           padInit.softwareStateCommitted &&
           !padInit.hardwarePadHalAuthority && !padInit.hostProjection &&
           !padInit.replayValueAuthority && !padInit.oldWinS0Authority &&
           !padInit.stage2PlusAuthority && !padInit.comod2Authority;
}

static bool IsExactIndependentWorkListState8001ED94(
    const PrSceneBootWorkListDirect::WorkListDrawBufferInit8001E6D0&
        workList) {
    using namespace PrSceneBootWorkListDirect;
    return workList.known && workList.workListsKnown &&
           workList.workLists.size() == kWorkListCount8001E6D0 &&
           workList.workLists[0].known && workList.workLists[1].known &&
           workList.workLists[0].descAddr == kWorkListBase8001E6D0 &&
           workList.workLists[1].descAddr ==
               kWorkListBase8001E6D0 + kWorkListStride8001E6D0 &&
           workList.workLists[0].order_00 == 4u &&
           workList.workLists[1].order_00 == 4u &&
           workList.workLists[0].headAddr_04 ==
               kWorkListHeadBase8001E6D0 &&
           workList.workLists[1].headAddr_04 ==
               kWorkListHeadBase8001E6D0 + kWorkListHeadStride8001E6D0 &&
           workList.workLists[0].lastAddr_08 == 0u &&
           workList.workLists[1].lastAddr_08 == 0u &&
           workList.drawBuffers.known &&
           workList.drawBuffers.arg0Known &&
           workList.drawBuffers.arg1Known &&
           workList.drawBuffers.arg0 ==
               PrSceneDrawBufferDirect::kDrawBufferBase80080CF8 &&
           workList.drawBuffers.arg1 ==
               PrSceneDrawBufferDirect::kDrawBufferBase80083FC0 &&
           !workList.drawBuffers.hostSideEffects;
}

BootstrapGraphRuntime8001C470
CommitBootstrapGraphRuntime8001C470(
    const ResetGraphState800446A0& resetGraph,
    const PrPsxPadDirect::PadInitState800354C0& padInit,
    const PrSceneBootWorkListDirect::WorkListDrawBufferInit8001E6D0& workList,
    const PrPsxGraphOwnerDirect::PsxGraphState* graphState) {
    BootstrapGraphRuntime8001C470 out{};
    out.known = true;
    out.graphStateExact =
        graphState != nullptr &&
        IsExactBootstrapGraphState8001C470(*graphState);
    out.resetGraphExact = IsExactResetGraphState8001C470(resetGraph);
    out.padInitExact = IsExactPadInitState8001C470(padInit);
    out.workListExact = IsExactIndependentWorkListState8001ED94(workList);
    // The direct 8001B1B0 owner is executed when the first Scene0 STR page
    // carrier is materialized.  Binding the source call here records the
    // cold-boot call target without pretending that PSX GPU MMIO ran.
    out.initialClearOwnerBound = true;
    out.softwareStateCommitted =
        out.graphStateExact && out.resetGraphExact && out.padInitExact &&
        out.workListExact && out.initialClearOwnerBound;
    return out;
}

static BootstrapGraphInit8001C470 BuildBootstrapGraphInit8001C470() {
    BootstrapGraphInit8001C470 out{};
    out.known = true;
    out.sourceFunction = kFn8001C470_BootstrapGraphInit;

    out.called8001C1E8 = true;
    out.called800446A0 = true;
    out.arg800446A0 = 0;
    out.called800354C0 = true;
    out.arg800354C0 = 0;

    out.called8003FB9C = true;
    out.graphWidth8003FB9C = 320u;
    out.graphHeight8003FB9C = 240u;
    out.graphMode8003FB9C = 4u;
    out.graphArg3_8003FB9C = 0u;
    out.graphArg4_8003FB9C = 0u;

    out.called80040AE4 = true;
    out.clipX80040AE4 = 0;
    out.clipY80040AE4 = 0;
    out.clipW80040AE4 = 0;
    out.clipH80040AE4 = 240;

    out.called80040B84 = true;
    out.called80040C74 = true;
    out.projectionH80040C74 = 440u;

    out.called8001B1B0 = true;
    out.arg0_8001B1B0 = 0;
    out.arg1_8001B1B0 = 0;
    out.arg2_8001B1B0 = 0;
    out.returnValueKnown = false;
    out.untranslatedCalleeGap = true;
    return out;
}

static BootstrapGraphAndWorkLists8001ED94
BuildBootstrapGraphAndWorkLists8001ED94() {
    BootstrapGraphAndWorkLists8001ED94 out{};
    out.known = true;
    out.sourceFunction = kFn8001ED94_BootstrapGraphAndWorkLists;
    out.called8001C470 = true;
    out.graphInit = BuildBootstrapGraphInit8001C470();
    out.called8001E6D0 = true;
    out.workListInit =
        PrSceneBootWorkListDirect::PsxCall8001E6D0_InitWorkListsAndDrawBuffers();
    out.orderMatchesPsx = true;
    return out;
}

ColdBootPlan800154F4 PsxCall800154F4_ColdBootPlan() {
    ColdBootPlan800154F4 out{};
    out.known = true;
    out.sourceFunction = kFn800154F4_BootstrapInit;

    out.calledSetMem80048970 = true;
    out.setMemArg0 = 2;
    out.calledResetCallback80035744 = true;

    out.called8001ED94 = true;
    out.graphAndWorkLists8001ED94 = BuildBootstrapGraphAndWorkLists8001ED94();

    out.calledResetLoaderMemory80025A00 = true;
    out.called8001A1CC = true;
    out.calledPadStartCom80026E4C = true;
    out.calledTextSystemBoot80027FAC = true;

    out.calledSaveStatus8001635C = true;
    out.saveStatusArg0 = 1;
    out.saveStatusArg1 = 1;
    out.saveStatusArg2 = 1;
    out.saveStatusArg3 = 0;

    out.calledClearWord800916D0_80016A80 = true;
    out.calledWaitFrame80035560 = true;
    out.waitFrameArg0 = -1;

    out.calledSrand80047FEC = true;
    out.srandSeedFromWaitFrameReturn = true;
    out.waitFrameReturnKnown = false;
    out.returnValueKnown = false;
    out.untranslatedCalleeGap = true;
    out.orderMatchesPsx = true;
    return out;
}

} // namespace PrSceneBootstrapDirect
