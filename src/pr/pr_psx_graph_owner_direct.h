#pragma once

#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_gs_sprite_submit_direct.h"

#include <array>
#include <cstdint>
#include <cstddef>

namespace PrPsxGraphOwnerDirect {

constexpr uint32_t kFn8003FB9C_InitGraph = 0x8003FB9Cu;
constexpr uint32_t kFn8004019C_GetDrawBuffer = 0x8004019Cu;
constexpr uint32_t kFn800401AC_ApplyDrawOffset = 0x800401ACu;
constexpr uint32_t kFn800402E0_ApplyViewport = 0x800402E0u;
constexpr uint32_t kFn800402C0_SetGeomOffset = 0x800402C0u;
constexpr uint32_t kFn80040370_FlipGraph = 0x80040370u;
constexpr uint32_t kFn80040C74_GsSetProjection = 0x80040C74u;
constexpr uint32_t kFn80040D6C_InitGteDefaults = 0x80040D6Cu;
constexpr uint32_t kFn80040F90_SetPacketAllocator = 0x80040F90u;
constexpr uint32_t kFn80040CC8_ClearWorkList = 0x80040CC8u;

struct PsxGraphWorkList80040CC8 {
    uint32_t order_00 = 0;
    uint32_t headAddr_04 = 0;
    uint32_t x_08 = 0;
    uint32_t y_0C = 0;
    uint32_t lastAddr_10 = 0;
    bool clearOtagRCalled = false;
    uint32_t clearOtagRHeadAddr = 0;
    uint32_t clearOtagRLength = 0;
    bool otSlotMirrorKnown = false;
    std::array<PrPsxFastSpriteSubmitDirect::RuntimeOtSlotValue8003FA20,
               PrPsxFastSpriteSubmitDirect::
                   kGsSortFastSpriteRuntimeOtSlotCapacity8003FA20>
        otSlotMirror{};
    bool packetWriteMirrorKnown = false;
    std::array<PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20,
               PrPsxFastSpriteSubmitDirect::
                   kGsSortFastSpriteRuntimePacketWriteCapacity8003FA20>
        packetWriteMirror{};
    bool gsSpriteOtSlotMirrorKnown = false;
    std::array<PrPsxGsSpriteSubmitDirect::RuntimeOtSlotValue8003F1B4,
               PrPsxGsSpriteSubmitDirect::
                   kGsSortSpriteRuntimeOtSlotCapacity8003F1B4>
        gsSpriteOtSlotMirror{};
    bool gsSpritePacketWriteMirrorKnown = false;
    std::array<PrPsxGsSpriteSubmitDirect::RuntimePacketWrite8003F1B4,
               PrPsxGsSpriteSubmitDirect::
                   kGsSortSpriteRuntimePacketWriteCapacity8003F1B4>
        gsSpritePacketWriteMirror{};
};

struct PsxGraphPageWorkList8001E374 {
    uint32_t workAddr = 0;
    uint32_t otHeadAddr = 0;
    PsxGraphWorkList80040CC8 work{};
};

struct PsxGraphDrawOffsetState800401AC {
    bool setGeomOffsetCalled = false;
    bool setDrawEnvCalled = false;
    int16_t word_800917AA = 0;
    int16_t word_800917AC = 0;
    int16_t word_80091730 = 0;
    int16_t word_80091732 = 0;
    int16_t word_80091734 = 0;
    int16_t word_80091736 = 0;
    int16_t word_80091738 = 0;
    int16_t word_8009173A = 0;
};

using PsxGraphDrawOffsetState = PsxGraphDrawOffsetState800401AC;

struct PsxGraphState {
    uint16_t word_80096590 = 0;
    uint32_t dword_8009658C = 1;
    uint16_t word_800965A0 = 0;
    std::array<int16_t, 2> word_8008ECA8{};
    std::array<int16_t, 2> word_8008ECAC{};
    int16_t word_800901C4 = 0;
    int16_t word_800901C6 = 0;
    int16_t word_800928D0 = 0;
    int16_t word_800928D2 = 0;
    int16_t word_800928D4 = 0;
    int16_t word_800928D6 = 0;
    int16_t word_80091790 = 0;
    int16_t word_80091792 = 0;
    uint32_t dword_800901C8 = 0;
    PrPsxGsSpriteSubmitDirect::GteControlState8003F1B4 gte{};
    PsxGraphDrawOffsetState drawOffset{};
    std::array<uint32_t, 2> dword_8006ED50{};
    std::array<PsxGraphPageWorkList8001E374, 2>
        mainPageWorkLists80087288{};
    bool mainPageWorkLists80087288Initialized = false;
};

struct PsxGraphFlipResult80040370 {
    uint16_t previousSlot = 0;
    uint16_t nextSlot = 0;
    uint32_t previousFrameCounter8009658C = 0;
    uint32_t nextFrameCounter8009658C = 0;
    bool sub800452ECCalled = false;
    bool sub80044AA0Called = false;
    bool sub800402E0Called = false;
    bool sub800401ACCalled = false;
    PsxGraphDrawOffsetState drawOffsetAfter800401AC{};
};

struct PsxGraphClearWorkListResult80040CC8 {
    uint32_t previousOrder = 0;
    uint32_t previousHeadAddr = 0;
    uint32_t previousLastAddr = 0;
    uint32_t nextLastAddr = 0;
    uint32_t clearOtagRLength = 0;
};

struct PsxFastSpriteRuntimeWorkSeed8003FA20 {
    uint8_t slot8004019C = 0;
    uint32_t packetAllocatorBase8006ED50 = 0;
    PsxGraphWorkList80040CC8 work{};
};

struct PsxFastSpriteRuntimeBuildResult8003FA20 {
    bool built = false;
    bool seedFound = false;
    uint8_t slot8004019C = 0;
    uint32_t packetAllocatorBase8006ED50 = 0;
    PsxGraphClearWorkListResult80040CC8 clearWorkResult{};
};

struct PsxFastSpriteRuntimeCommitResult8003FA20 {
    bool committed = false;
    bool runtimeValid = false;
    bool pageWorkFound = false;
    uint8_t slot8004019C = 0;
    uint32_t workListAddr = 0;
    uint32_t allocatorBefore = 0;
    uint32_t allocatorAfter = 0;
    bool allocatorUpdated = false;
    uint32_t otSlotCount = 0;
    bool otSlotMirrorUpdated = false;
    uint32_t packetWriteCount = 0;
    bool packetWriteMirrorUpdated = false;
    bool packetMemoryGap = true;
};

struct PsxGsSpriteRuntimeCommitResult8003F1B4 {
    bool committed = false;
    bool runtimeValid = false;
    bool pageWorkFound = false;
    uint8_t slot8004019C = 0;
    uint32_t workListAddr = 0;
    uint32_t allocatorBefore = 0;
    uint32_t allocatorAfter = 0;
    bool allocatorUpdated = false;
    uint32_t otSlotCount = 0;
    bool otSlotMirrorUpdated = false;
    uint32_t packetWriteCount = 0;
    bool packetWriteMirrorUpdated = false;
    bool packetMemoryGap = true;
};

struct PsxGsSpriteRuntimeBuildResult8003F1B4 {
    bool built = false;
    bool seedFound = false;
    uint8_t slot8004019C = 0;
    uint32_t packetAllocatorBase8006ED50 = 0;
    PsxGraphClearWorkListResult80040CC8 clearWorkResult{};
};

void PsxInitializeGraphState8003FB9C(PsxGraphState& state,
                                     uint16_t width,
                                     uint16_t height);

uint16_t PsxCall8004019C_GetDrawBuffer(const PsxGraphState& state);

PsxGraphDrawOffsetState PsxCall800402E0_ApplyViewport(PsxGraphState& state);

PsxGraphDrawOffsetState PsxCall800401AC_ApplyDrawOffset(
    PsxGraphState& state);

PsxGraphDrawOffsetState PsxCall80040B84_ApplyScreenCenterAndDrawOffset(
    PsxGraphState& state);

PsxGraphFlipResult80040370 PsxCall80040370_FlipGraph(PsxGraphState& state);

void PsxCall800402C0_SetGeomOffset(PsxGraphState& state,
                                   int32_t x,
                                   int32_t y);

void PsxCall80040D6C_InitGteDefaults(PsxGraphState& state);

void PsxCall80040C74_GsSetProjection(PsxGraphState& state,
                                     uint32_t geomScreen);

void PsxCall80040F90_SetPacketAllocator(PsxGraphState& state,
                                        uint32_t allocatorAddr);

void PsxSeedMainPageWorkLists80087288(PsxGraphState& state);

PsxGraphClearWorkListResult80040CC8
PsxCall8001E374_ClearMainPageWork(PsxGraphState& state,
                                  uint8_t pageIndex);

PsxGraphClearWorkListResult80040CC8 PsxCall80040CC8_ClearWorkList(
    PsxGraphWorkList80040CC8& work,
    uint16_t x,
    uint16_t y);

PsxFastSpriteRuntimeBuildResult8003FA20
BuildRuntimeState8003FA20FromGraphOwner(
    PsxGraphState& graph,
    const PsxFastSpriteRuntimeWorkSeed8003FA20* seeds,
    std::size_t seedCount,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime);

bool BuildRuntimeState8003FA20FromPageWork(
    const PsxGraphState& graph,
    const PsxGraphPageWorkList8001E374& pageWork,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime);

PsxFastSpriteRuntimeCommitResult8003FA20
CommitRuntimeState8003FA20ToMainPageWork(
    PsxGraphState& graph,
    uint8_t pageIndex,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime);

PsxGsSpriteRuntimeBuildResult8003F1B4
BuildRuntimeState8003F1B4FromGraphOwner(
    PsxGraphState& graph,
    const PsxFastSpriteRuntimeWorkSeed8003FA20* seeds,
    std::size_t seedCount,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& outRuntime);

bool BuildRuntimeState8003F1B4FromPageWork(
    const PsxGraphState& graph,
    const PsxGraphPageWorkList8001E374& pageWork,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& outRuntime);

PsxFastSpriteRuntimeBuildResult8003FA20
BuildStage1CompactRailRuntimeState8003FA20FromGraphOwner(
    PsxGraphState& graph,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime);

PsxFastSpriteRuntimeBuildResult8003FA20
BuildMovie1TextMainRuntimeState8003FA20FromGraphOwner(
    PsxGraphState& graph,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime);

PsxFastSpriteRuntimeBuildResult8003FA20
BuildMovie1SmallOtRuntimeState8003FA20FromGraphOwner(
    PsxGraphState& graph,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime);

PsxGsSpriteRuntimeBuildResult8003F1B4
BuildStage1CompactRailRuntimeState8003F1B4FromGraphOwner(
    PsxGraphState& graph,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& outRuntime);

PsxGsSpriteRuntimeCommitResult8003F1B4
CommitRuntimeState8003F1B4ToMainPageWork(
    PsxGraphState& graph,
    uint8_t pageIndex,
    const PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& runtime);

} // namespace PrPsxGraphOwnerDirect
