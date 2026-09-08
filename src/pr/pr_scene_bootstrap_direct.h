#pragma once

#include "pr_psx_graph_owner_direct.h"
#include "pr_psx_pad_direct.h"
#include "pr_scene_boot_worklist_direct.h"

#include <cstdint>

namespace PrSceneBootstrapDirect {

static constexpr uint32_t kFn8001C470_BootstrapGraphInit = 0x8001C470u;
static constexpr uint32_t kFn8001ED94_BootstrapGraphAndWorkLists = 0x8001ED94u;
static constexpr uint32_t kFn800154F4_BootstrapInit = 0x800154F4u;
static constexpr uint32_t kFn800446A0_ResetGraph = 0x800446A0u;
static constexpr uint32_t kFn80046EC0_ResetGraphDisplayInit = 0x80046EC0u;
static constexpr uint32_t kFn800473C0_ResetGraphMemset = 0x800473C0u;
static constexpr uint32_t kFn80048950_ResetGraphGpuCommand = 0x80048950u;
static constexpr uint32_t kFn80048970_SetMem = 0x80048970u;
static constexpr uint32_t kFn80035744_ResetCallback = 0x80035744u;
static constexpr uint32_t kFn80025A00_ResetLoaderMemory = 0x80025A00u;
static constexpr uint32_t kFn8001A1CC_BootstrapCdReset = 0x8001A1CCu;
static constexpr uint32_t kFn80026E4C_PadStartCom = 0x80026E4Cu;
static constexpr uint32_t kFn80027FAC_TextSystemBoot = 0x80027FACu;
static constexpr uint32_t kFn8001635C_SaveStatus = 0x8001635Cu;
static constexpr uint32_t kFn80016A80_ClearWord800916D0 = 0x80016A80u;
static constexpr uint32_t kFn80035560_WaitFrame = 0x80035560u;
static constexpr uint32_t kFn80047FEC_Srand = 0x80047FECu;
static constexpr uint32_t kResetGraphEnvironmentAddress8005D734 = 0x8005D734u;
static constexpr uint32_t kResetGraphEnvironmentBytes800446A0 = 0x80u;
static constexpr uint32_t kResetGraphGpuCommandListAddress8005D6EC = 0x8005D6ECu;
static constexpr uint32_t kResetGraphMainEnvironmentAddress8005D744 = 0x8005D744u;
static constexpr uint32_t kResetGraphMainEnvironmentBytes800446A0 = 0x5Cu;
static constexpr uint32_t kResetGraphTailEnvironmentAddress8005D7A0 = 0x8005D7A0u;
static constexpr uint32_t kResetGraphTailEnvironmentBytes800446A0 = 0x14u;

struct ResetGraphState800446A0 {
    bool sourceKnown = false;
    bool accepted = false;
    int32_t inputArg = 0;
    uint32_t modeLow3 = 0;
    bool unsupportedModeRejected = false;
    bool environmentZeroed = false;
    uint32_t environmentAddress = kResetGraphEnvironmentAddress8005D734;
    uint32_t environmentBytes = 0;
    bool resetCallbackCalled = false;
    bool gpuControlWordSubmitted = false;
    uint32_t gpuCommandListAddress = 0;
    bool displayInitCalled = false;
    int32_t displayInitArg = 0;
    bool displayInitReturnKnown = false;
    int32_t displayInitReturnValue = 0;
    bool mainEnvironmentCleared = false;
    uint32_t mainEnvironmentAddress = kResetGraphMainEnvironmentAddress8005D744;
    uint32_t mainEnvironmentBytes = 0;
    bool tailEnvironmentCleared = false;
    uint32_t tailEnvironmentAddress = kResetGraphTailEnvironmentAddress8005D7A0;
    uint32_t tailEnvironmentBytes = 0;
    bool softwareStateCommitted = false;
    bool hardwareResetHalAuthority = false;
    bool exactPsxHalParity = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

ResetGraphState800446A0 PsxCall800446A0_ResetGraphRuntime(int32_t inputArg);

struct BootstrapGraphInit8001C470 {
    bool known = false;
    uint32_t sourceFunction = kFn8001C470_BootstrapGraphInit;
    bool called8001C1E8 = false;
    bool called800446A0 = false;
    int32_t arg800446A0 = 0;
    bool called800354C0 = false;
    int32_t arg800354C0 = 0;
    bool called8003FB9C = false;
    uint16_t graphWidth8003FB9C = 0;
    uint16_t graphHeight8003FB9C = 0;
    uint32_t graphMode8003FB9C = 0;
    uint32_t graphArg3_8003FB9C = 0;
    uint32_t graphArg4_8003FB9C = 0;
    bool called80040AE4 = false;
    int32_t clipX80040AE4 = 0;
    int32_t clipY80040AE4 = 0;
    int32_t clipW80040AE4 = 0;
    int32_t clipH80040AE4 = 0;
    bool called80040B84 = false;
    bool called80040C74 = false;
    uint32_t projectionH80040C74 = 0;
    bool called8001B1B0 = false;
    int32_t arg0_8001B1B0 = 0;
    int32_t arg1_8001B1B0 = 0;
    int32_t arg2_8001B1B0 = 0;
    bool returnValueKnown = false;
    int32_t returnValue = 0;
    bool untranslatedCalleeGap = false;
};

// Runtime binding for the coupled 8001ED94 -> 8001C470 -> 8001E6D0
// startup subgraph.  The individual carriers above are useful for static
// planning, but they are not runtime evidence by themselves.  This result is
// committed only after SS0 has observed the actual graph, ResetGraph, PAD,
// and independent work-list states produced by the cold Scene0 path.
struct BootstrapGraphRuntime8001C470 {
    bool known = false;
    uint32_t sourceFunction = kFn8001C470_BootstrapGraphInit;
    bool graphStateExact = false;
    bool resetGraphExact = false;
    bool padInitExact = false;
    bool workListExact = false;
    bool initialClearOwnerBound = false;
    bool softwareStateCommitted = false;
    // These remain explicit because no PSX MMIO/interrupt replay is present.
    bool physicalHalGap = true;
    bool exactPsxHalParity = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

BootstrapGraphRuntime8001C470
CommitBootstrapGraphRuntime8001C470(
    const ResetGraphState800446A0& resetGraph,
    const PrPsxPadDirect::PadInitState800354C0& padInit,
    const PrSceneBootWorkListDirect::WorkListDrawBufferInit8001E6D0& workList,
    const PrPsxGraphOwnerDirect::PsxGraphState* graphState);

struct BootstrapGraphAndWorkLists8001ED94 {
    bool known = false;
    uint32_t sourceFunction = kFn8001ED94_BootstrapGraphAndWorkLists;
    BootstrapGraphInit8001C470 graphInit{};
    PrSceneBootWorkListDirect::WorkListDrawBufferInit8001E6D0 workListInit{};
    bool called8001C470 = false;
    bool called8001E6D0 = false;
    bool orderMatchesPsx = false;
};

struct ColdBootPlan800154F4 {
    bool known = false;
    uint32_t sourceFunction = kFn800154F4_BootstrapInit;
    bool calledSetMem80048970 = false;
    int32_t setMemArg0 = 0;
    bool calledResetCallback80035744 = false;
    bool called8001ED94 = false;
    BootstrapGraphAndWorkLists8001ED94 graphAndWorkLists8001ED94{};
    bool calledResetLoaderMemory80025A00 = false;
    bool called8001A1CC = false;
    bool calledPadStartCom80026E4C = false;
    bool calledTextSystemBoot80027FAC = false;
    bool calledSaveStatus8001635C = false;
    int32_t saveStatusArg0 = 0;
    int32_t saveStatusArg1 = 0;
    int32_t saveStatusArg2 = 0;
    int32_t saveStatusArg3 = 0;
    bool calledClearWord800916D0_80016A80 = false;
    bool calledWaitFrame80035560 = false;
    int32_t waitFrameArg0 = 0;
    bool calledSrand80047FEC = false;
    bool srandSeedFromWaitFrameReturn = false;
    bool waitFrameReturnKnown = false;
    int32_t waitFrameReturn = 0;
    bool returnValueKnown = false;
    int32_t returnValue = 0;
    bool untranslatedCalleeGap = false;
    bool orderMatchesPsx = false;
};

ColdBootPlan800154F4 PsxCall800154F4_ColdBootPlan();

} // namespace PrSceneBootstrapDirect
