#pragma once

#include "pr_scene_boot_worklist_direct.h"

#include <cstdint>

namespace PrSceneBootstrapDirect {

static constexpr uint32_t kFn8001C470_BootstrapGraphInit = 0x8001C470u;
static constexpr uint32_t kFn8001ED94_BootstrapGraphAndWorkLists = 0x8001ED94u;
static constexpr uint32_t kFn800154F4_BootstrapInit = 0x800154F4u;
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
