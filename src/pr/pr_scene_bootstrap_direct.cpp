#include "pr_scene_bootstrap_direct.h"

namespace PrSceneBootstrapDirect {

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
