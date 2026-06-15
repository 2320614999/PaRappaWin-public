#include "pr_psx_graph_owner_direct.h"

namespace PrPsxGraphOwnerDirect {
namespace {

int16_t AddI16(int16_t lhs, int16_t rhs) {
    return static_cast<int16_t>(static_cast<int32_t>(lhs) +
                                static_cast<int32_t>(rhs));
}

uint32_t ResolveWorkListLastAddr80040CC8(uint32_t order,
                                         uint32_t headAddr) {
    return (4u << order) + headAddr - 4u;
}

bool FillClearedOtSlots8003FA20(
    PrPsxFastSpriteSubmitDirect::RuntimeOrderingTableState8003FA20& ot,
    const PsxGraphWorkList80040CC8& work) {
    if (work.order_00 >= 31u || work.y_0C != 0u) {
        return false;
    }
    const uint32_t slotCount = 1u << work.order_00;
    const uint32_t firstSlotAddr = work.headAddr_04 - 4u * work.x_08;
    const uint32_t lastSlotAddr = firstSlotAddr + (slotCount - 1u) * 4u;
    if (lastSlotAddr != work.lastAddr_10) {
        return false;
    }

    ot.headAddr_04 = work.headAddr_04;
    ot.length_08 = work.x_08;
    constexpr uint32_t kPacketAddrMask =
        PrPsxFastSpriteSubmitDirect::
            kGsSortFastSpritePacketAddrMask8003FA20;
    const uint32_t slotLimit =
        slotCount <
                PrPsxFastSpriteSubmitDirect::
                    kGsSortFastSpriteRuntimeOtSlotCapacity8003FA20
            ? slotCount
            : static_cast<uint32_t>(
                  PrPsxFastSpriteSubmitDirect::
                      kGsSortFastSpriteRuntimeOtSlotCapacity8003FA20);
    for (uint32_t priority = 0; priority < slotLimit; ++priority) {
        const uint32_t slotAddr = firstSlotAddr + priority * 4u;
        const uint32_t oldValue =
            priority == 0u ? kPacketAddrMask
                           : ((slotAddr - 4u) & kPacketAddrMask);
        if (!PrPsxFastSpriteSubmitDirect::SetRuntimeOtSlotValue8003FA20(
                ot,
                slotAddr,
                oldValue)) {
            return false;
        }
    }
    return true;
}

bool FillClearedOtSlotMirror8003FA20(PsxGraphWorkList80040CC8& work) {
    work.otSlotMirror = {};
    work.otSlotMirrorKnown = false;
    PrPsxFastSpriteSubmitDirect::RuntimeOrderingTableState8003FA20 ot{};
    if (!FillClearedOtSlots8003FA20(ot, work)) {
        return false;
    }
    work.otSlotMirror = ot.slotValues;
    work.otSlotMirrorKnown = true;
    return true;
}

void ClearPacketWriteMirror8003FA20(PsxGraphWorkList80040CC8& work) {
    work.packetWriteMirror = {};
    work.packetWriteMirrorKnown = true;
}

bool FillClearedOtSlots8003F1B4(
    PrPsxGsSpriteSubmitDirect::RuntimeOrderingTableState8003F1B4& ot,
    const PsxGraphWorkList80040CC8& work) {
    if (work.order_00 >= 31u || work.y_0C != 0u) {
        return false;
    }
    const uint32_t slotCount = 1u << work.order_00;
    const uint32_t firstSlotAddr = work.headAddr_04 - 4u * work.x_08;
    const uint32_t lastSlotAddr = firstSlotAddr + (slotCount - 1u) * 4u;
    if (lastSlotAddr != work.lastAddr_10) {
        return false;
    }

    ot.headAddr_04 = work.headAddr_04;
    ot.length_08 = work.x_08;
    constexpr uint32_t kPacketAddrMask =
        PrPsxGsSpriteSubmitDirect::kGsSortSpritePacketAddrMask8003F1B4;
    const uint32_t slotLimit = slotCount <
                                       PrPsxGsSpriteSubmitDirect::
                                           kGsSortSpriteRuntimeOtSlotCapacity8003F1B4
                                   ? slotCount
                                   : static_cast<uint32_t>(
                                         PrPsxGsSpriteSubmitDirect::
                                             kGsSortSpriteRuntimeOtSlotCapacity8003F1B4);
    for (uint32_t priority = 0; priority < slotLimit; ++priority) {
        const uint32_t slotAddr = firstSlotAddr + priority * 4u;
        const uint32_t oldValue =
            priority == 0u ? kPacketAddrMask
                           : ((slotAddr - 4u) & kPacketAddrMask);
        if (!PrPsxGsSpriteSubmitDirect::SetRuntimeOtSlotValue8003F1B4(
                ot,
                slotAddr,
                oldValue)) {
            return false;
        }
    }
    return true;
}

bool FillClearedOtSlotMirror8003F1B4(PsxGraphWorkList80040CC8& work) {
    work.gsSpriteOtSlotMirror = {};
    work.gsSpriteOtSlotMirrorKnown = false;
    PrPsxGsSpriteSubmitDirect::RuntimeOrderingTableState8003F1B4 ot{};
    if (!FillClearedOtSlots8003F1B4(ot, work)) {
        return false;
    }
    work.gsSpriteOtSlotMirror = ot.slotValues;
    work.gsSpriteOtSlotMirrorKnown = true;
    return true;
}

void ClearGsSpritePacketWriteMirror8003F1B4(
    PsxGraphWorkList80040CC8& work) {
    work.gsSpritePacketWriteMirror = {};
    work.gsSpritePacketWriteMirrorKnown = true;
}

} // namespace

void PsxInitializeGraphState8003FB9C(PsxGraphState& state,
                                     uint16_t width,
                                     uint16_t height) {
    state = PsxGraphState{};
    state.word_80096590 = 0;
    state.dword_8009658C = 1;
    state.word_800928D4 = static_cast<int16_t>(width);
    state.word_800928D6 = static_cast<int16_t>(height);
    PsxCall80040D6C_InitGteDefaults(state);
    PsxSeedMainPageWorkLists80087288(state);
    PsxCall800402E0_ApplyViewport(state);
    PsxCall800401AC_ApplyDrawOffset(state);
}

uint16_t PsxCall8004019C_GetDrawBuffer(const PsxGraphState& state) {
    return state.word_80096590;
}

PsxGraphDrawOffsetState PsxCall800402E0_ApplyViewport(PsxGraphState& state) {
    const uint16_t slot = state.word_80096590 & 1u;
    PsxGraphDrawOffsetState out = state.drawOffset;
    out.word_80091734 = state.word_800928D4;
    out.word_80091736 = state.word_800928D6;
    out.word_80091730 =
        AddI16(state.word_800928D0, state.word_8008ECA8[slot]);
    out.word_80091732 =
        AddI16(state.word_800928D2, state.word_8008ECAC[slot]);
    out.setDrawEnvCalled = true;
    state.drawOffset = out;
    return out;
}

PsxGraphDrawOffsetState PsxCall800401AC_ApplyDrawOffset(
    PsxGraphState& state) {
    const uint16_t slot = state.word_80096590 & 1u;
    PsxGraphDrawOffsetState out = state.drawOffset;
    if (state.word_800965A0 != 0u) {
        out.word_800917AC = 0;
        out.word_800917AA = 0;
        out.word_80091738 =
            AddI16(state.word_800901C4, state.word_8008ECA8[slot]);
        out.word_8009173A =
            AddI16(state.word_800901C6, state.word_8008ECAC[slot]);
        out.setDrawEnvCalled = true;
    } else {
        const uint16_t oppositeSlot = (state.word_80096590 == 0u) ? 1u : 0u;
        out.word_800917AA =
            AddI16(state.word_800901C4, state.word_8008ECA8[oppositeSlot]);
        out.word_800917AC =
            AddI16(state.word_800901C6, state.word_8008ECAC[oppositeSlot]);
        out.setGeomOffsetCalled = true;
        PsxCall800402C0_SetGeomOffset(state,
                                       out.word_800917AA,
                                       out.word_800917AC);
    }
    state.drawOffset = out;
    return out;
}

PsxGraphDrawOffsetState PsxCall80040B84_ApplyScreenCenterAndDrawOffset(
    PsxGraphState& state) {
    state.word_800901C4 = static_cast<int16_t>(state.word_800928D4 / 2);
    state.word_800901C6 = static_cast<int16_t>(state.word_800928D6 / 2);
    return PsxCall800401AC_ApplyDrawOffset(state);
}

PsxGraphFlipResult80040370 PsxCall80040370_FlipGraph(PsxGraphState& state) {
    PsxGraphFlipResult80040370 out{};
    out.previousSlot = state.word_80096590;
    out.previousFrameCounter8009658C = state.dword_8009658C;

    const uint16_t slot = state.word_80096590 & 1u;
    state.word_80091790 = state.word_8008ECA8[slot];
    state.word_80091792 = state.word_8008ECAC[slot];
    out.sub800452ECCalled = true;
    out.sub80044AA0Called = true;

    state.dword_8009658C =
        state.dword_8009658C != 0u ? state.dword_8009658C + 1u : 1u;
    state.word_80096590 = (state.word_80096590 == 0u) ? 1u : 0u;
    out.nextSlot = state.word_80096590;
    out.nextFrameCounter8009658C = state.dword_8009658C;

    PsxCall800402E0_ApplyViewport(state);
    out.sub800402E0Called = true;
    out.drawOffsetAfter800401AC = PsxCall800401AC_ApplyDrawOffset(state);
    out.sub800401ACCalled = true;
    return out;
}

void PsxCall800402C0_SetGeomOffset(PsxGraphState& state,
                                   int32_t x,
                                   int32_t y) {
    state.gte.geomOffsetKnown = true;
    state.gte.geomOffsetX =
        static_cast<int32_t>(static_cast<uint32_t>(x) << 16);
    state.gte.geomOffsetY =
        static_cast<int32_t>(static_cast<uint32_t>(y) << 16);
}

void PsxCall80040D6C_InitGteDefaults(PsxGraphState& state) {
    state.gte.geomScreenKnown = true;
    state.gte.geomScreen = 1000u;
    state.gte.depthCueKnown = true;
    state.gte.depthCueA = -4194;
    state.gte.depthCueB = 0x01400000;
    state.gte.zScaleFactorKnown = true;
    state.gte.zScaleFactor3 = 341;
    state.gte.zScaleFactor4 = 256;
    PsxCall800402C0_SetGeomOffset(state, 0, 0);
}

void PsxCall80040C74_GsSetProjection(PsxGraphState& state,
                                     uint32_t geomScreen) {
    state.gte.geomScreenKnown = true;
    state.gte.geomScreen = geomScreen;
}

void PsxCall80040F90_SetPacketAllocator(PsxGraphState& state,
                                        uint32_t allocatorAddr) {
    state.dword_800901C8 = allocatorAddr;
}

void PsxSeedMainPageWorkLists80087288(PsxGraphState& state) {
    state.dword_8006ED50[0] = 0x801A73B0u;
    state.dword_8006ED50[1] = 0x801B54B0u;
    state.mainPageWorkLists80087288[0] =
        PsxGraphPageWorkList8001E374{
            0x80087288u,
            0x80088288u,
            {14u, 0x80088288u, 0u, 0u, 0u, false, 0u, 0u}};
    state.mainPageWorkLists80087288[1] =
        PsxGraphPageWorkList8001E374{
            0x8008729Cu,
            0x80098288u,
            {14u, 0x80098288u, 0u, 0u, 0u, false, 0u, 0u}};
    state.mainPageWorkLists80087288Initialized = true;
}

PsxGraphClearWorkListResult80040CC8
PsxCall8001E374_ClearMainPageWork(PsxGraphState& state,
                                  uint8_t pageIndex) {
    if (!state.mainPageWorkLists80087288Initialized) {
        PsxSeedMainPageWorkLists80087288(state);
    }
    PsxGraphPageWorkList8001E374& pageWork =
        state.mainPageWorkLists80087288[pageIndex & 1u];
    return PsxCall80040CC8_ClearWorkList(pageWork.work, 0u, 0u);
}

PsxGraphClearWorkListResult80040CC8 PsxCall80040CC8_ClearWorkList(
    PsxGraphWorkList80040CC8& work,
    uint16_t x,
    uint16_t y) {
    PsxGraphClearWorkListResult80040CC8 out{};
    out.previousOrder = work.order_00;
    out.previousHeadAddr = work.headAddr_04;
    out.previousLastAddr = work.lastAddr_10;
    work.x_08 = x;
    work.y_0C = y;
    work.lastAddr_10 =
        ResolveWorkListLastAddr80040CC8(work.order_00, work.headAddr_04);
    work.clearOtagRCalled = true;
    work.clearOtagRHeadAddr = work.headAddr_04;
    work.clearOtagRLength = 1u << work.order_00;
    FillClearedOtSlotMirror8003FA20(work);
    ClearPacketWriteMirror8003FA20(work);
    FillClearedOtSlotMirror8003F1B4(work);
    ClearGsSpritePacketWriteMirror8003F1B4(work);
    out.nextLastAddr = work.lastAddr_10;
    out.clearOtagRLength = work.clearOtagRLength;
    return out;
}

bool BuildRuntimeState8003FA20FromPageWork(
    const PsxGraphState& graph,
    const PsxGraphPageWorkList8001E374& pageWork,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime) {
    outRuntime = PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20{};
    outRuntime.dword_800901C8 = graph.dword_800901C8;
    outRuntime.word_800917AA = graph.drawOffset.word_800917AA;
    outRuntime.word_800917AC = graph.drawOffset.word_800917AC;
    outRuntime.drawEnvOffsetKnown800401AC = graph.drawOffset.setDrawEnvCalled;
    outRuntime.drawEnvOffsetX80091738 = graph.drawOffset.word_80091738;
    outRuntime.drawEnvOffsetY8009173A = graph.drawOffset.word_8009173A;
    outRuntime.ot.headAddr_04 = pageWork.work.headAddr_04;
    outRuntime.ot.length_08 = pageWork.work.x_08;
    if (pageWork.work.otSlotMirrorKnown) {
        outRuntime.ot.slotValues = pageWork.work.otSlotMirror;
    } else if (!FillClearedOtSlots8003FA20(outRuntime.ot, pageWork.work)) {
        return false;
    }
    if (pageWork.work.packetWriteMirrorKnown) {
        outRuntime.packetWrites = pageWork.work.packetWriteMirror;
    }
    return true;
}

bool BuildRuntimeState8003F1B4FromPageWork(
    const PsxGraphState& graph,
    const PsxGraphPageWorkList8001E374& pageWork,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& outRuntime) {
    outRuntime = PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4{};
    outRuntime.dword_800901C8 = graph.dword_800901C8;
    outRuntime.word_800917AA = graph.drawOffset.word_800917AA;
    outRuntime.word_800917AC = graph.drawOffset.word_800917AC;
    outRuntime.gte = graph.gte;
    outRuntime.ot.headAddr_04 = pageWork.work.headAddr_04;
    outRuntime.ot.length_08 = pageWork.work.x_08;
    if (pageWork.work.gsSpriteOtSlotMirrorKnown) {
        outRuntime.ot.slotValues = pageWork.work.gsSpriteOtSlotMirror;
    } else if (!FillClearedOtSlots8003F1B4(outRuntime.ot, pageWork.work)) {
        return false;
    }
    if (pageWork.work.gsSpritePacketWriteMirrorKnown) {
        outRuntime.packetWrites = pageWork.work.gsSpritePacketWriteMirror;
    }
    return true;
}

PsxFastSpriteRuntimeCommitResult8003FA20
CommitRuntimeState8003FA20ToMainPageWork(
    PsxGraphState& graph,
    uint8_t pageIndex,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime) {
    PsxFastSpriteRuntimeCommitResult8003FA20 out{};
    out.runtimeValid = runtime.ot.headAddr_04 != 0u;
    if (!graph.mainPageWorkLists80087288Initialized) {
        return out;
    }

    const uint8_t slot = pageIndex & 1u;
    out.pageWorkFound = true;
    out.slot8004019C = slot;
    PsxGraphPageWorkList8001E374& pageWork =
        graph.mainPageWorkLists80087288[slot];
    out.workListAddr = pageWork.workAddr;
    if (!out.runtimeValid ||
        runtime.ot.headAddr_04 != pageWork.work.headAddr_04 ||
        runtime.ot.length_08 != pageWork.work.x_08) {
        return out;
    }

    out.allocatorBefore = graph.dword_800901C8;
    out.allocatorAfter = runtime.dword_800901C8;
    graph.dword_800901C8 = runtime.dword_800901C8;
    out.allocatorUpdated = true;

    pageWork.work.otSlotMirror = runtime.ot.slotValues;
    pageWork.work.otSlotMirrorKnown = true;
    out.otSlotMirrorUpdated = true;
    for (const PrPsxFastSpriteSubmitDirect::RuntimeOtSlotValue8003FA20& slotValue :
         runtime.ot.slotValues) {
        if (slotValue.valid) {
            ++out.otSlotCount;
        }
    }

    pageWork.work.packetWriteMirror = runtime.packetWrites;
    pageWork.work.packetWriteMirrorKnown = true;
    out.packetWriteMirrorUpdated = true;
    out.packetMemoryGap = false;
    for (const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write :
         runtime.packetWrites) {
        if (!write.valid) {
            continue;
        }
        ++out.packetWriteCount;
        const std::size_t knownCount =
            write.wordCount < write.wordKnown.size()
                ? static_cast<std::size_t>(write.wordCount)
                : write.wordKnown.size();
        for (std::size_t i = 0; i < knownCount; ++i) {
            if (!write.wordKnown[i]) {
                out.packetMemoryGap = true;
                break;
            }
        }
    }

    out.committed = out.runtimeValid && out.pageWorkFound;
    return out;
}

PsxFastSpriteRuntimeBuildResult8003FA20
BuildRuntimeState8003FA20FromGraphOwner(
    PsxGraphState& graph,
    const PsxFastSpriteRuntimeWorkSeed8003FA20* seeds,
    std::size_t seedCount,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime) {
    outRuntime = PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20{};
    PsxFastSpriteRuntimeBuildResult8003FA20 result{};
    result.slot8004019C = static_cast<uint8_t>(
        PsxCall8004019C_GetDrawBuffer(graph) & 1u);

    const PsxFastSpriteRuntimeWorkSeed8003FA20* seed = nullptr;
    for (std::size_t i = 0; i < seedCount; ++i) {
        if (seeds[i].slot8004019C == result.slot8004019C) {
            seed = &seeds[i];
            break;
        }
    }
    if (seed == nullptr) {
        return result;
    }
    result.seedFound = true;
    result.packetAllocatorBase8006ED50 = seed->packetAllocatorBase8006ED50;

    PsxCall80040F90_SetPacketAllocator(graph,
                                       seed->packetAllocatorBase8006ED50);
    PsxGraphWorkList80040CC8 work = seed->work;
    result.clearWorkResult = PsxCall80040CC8_ClearWorkList(work, 0u, 0u);

    outRuntime.dword_800901C8 = graph.dword_800901C8;
    outRuntime.word_800917AA = graph.drawOffset.word_800917AA;
    outRuntime.word_800917AC = graph.drawOffset.word_800917AC;
    outRuntime.drawEnvOffsetKnown800401AC = graph.drawOffset.setDrawEnvCalled;
    outRuntime.drawEnvOffsetX80091738 = graph.drawOffset.word_80091738;
    outRuntime.drawEnvOffsetY8009173A = graph.drawOffset.word_8009173A;
    result.built = FillClearedOtSlots8003FA20(outRuntime.ot, work);
    return result;
}

PsxGsSpriteRuntimeBuildResult8003F1B4
BuildRuntimeState8003F1B4FromGraphOwner(
    PsxGraphState& graph,
    const PsxFastSpriteRuntimeWorkSeed8003FA20* seeds,
    std::size_t seedCount,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& outRuntime) {
    outRuntime = PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4{};
    PsxGsSpriteRuntimeBuildResult8003F1B4 result{};
    result.slot8004019C = static_cast<uint8_t>(
        PsxCall8004019C_GetDrawBuffer(graph) & 1u);

    const PsxFastSpriteRuntimeWorkSeed8003FA20* seed = nullptr;
    for (std::size_t i = 0; i < seedCount; ++i) {
        if (seeds[i].slot8004019C == result.slot8004019C) {
            seed = &seeds[i];
            break;
        }
    }
    if (seed == nullptr) {
        return result;
    }
    result.seedFound = true;
    result.packetAllocatorBase8006ED50 = seed->packetAllocatorBase8006ED50;

    PsxCall80040F90_SetPacketAllocator(graph,
                                       seed->packetAllocatorBase8006ED50);
    PsxGraphWorkList80040CC8 work = seed->work;
    result.clearWorkResult = PsxCall80040CC8_ClearWorkList(work, 0u, 0u);

    outRuntime.dword_800901C8 = graph.dword_800901C8;
    outRuntime.word_800917AA = graph.drawOffset.word_800917AA;
    outRuntime.word_800917AC = graph.drawOffset.word_800917AC;
    outRuntime.gte = graph.gte;
    outRuntime.ot.headAddr_04 = work.headAddr_04;
    outRuntime.ot.length_08 = work.x_08;
    if (work.gsSpriteOtSlotMirrorKnown) {
        outRuntime.ot.slotValues = work.gsSpriteOtSlotMirror;
        result.built = true;
    } else {
        result.built = FillClearedOtSlots8003F1B4(outRuntime.ot, work);
    }
    if (work.gsSpritePacketWriteMirrorKnown) {
        outRuntime.packetWrites = work.gsSpritePacketWriteMirror;
    }
    return result;
}

PsxFastSpriteRuntimeBuildResult8003FA20
BuildStage1CompactRailRuntimeState8003FA20FromGraphOwner(
    PsxGraphState& graph,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime) {
    outRuntime = PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20{};
    if (!graph.mainPageWorkLists80087288Initialized) {
        PsxSeedMainPageWorkLists80087288(graph);
    }

    PsxFastSpriteRuntimeBuildResult8003FA20 result{};
    result.slot8004019C = static_cast<uint8_t>(
        PsxCall8004019C_GetDrawBuffer(graph) & 1u);
    result.seedFound = true;
    result.packetAllocatorBase8006ED50 =
        graph.dword_8006ED50[result.slot8004019C];

    PsxCall80040F90_SetPacketAllocator(
        graph,
        result.packetAllocatorBase8006ED50);
    PsxGraphPageWorkList8001E374& pageWork =
        graph.mainPageWorkLists80087288[result.slot8004019C];
    result.clearWorkResult =
        PsxCall80040CC8_ClearWorkList(pageWork.work, 0u, 0u);

    outRuntime.dword_800901C8 = graph.dword_800901C8;
    outRuntime.word_800917AA = graph.drawOffset.word_800917AA;
    outRuntime.word_800917AC = graph.drawOffset.word_800917AC;
    outRuntime.drawEnvOffsetKnown800401AC = graph.drawOffset.setDrawEnvCalled;
    outRuntime.drawEnvOffsetX80091738 = graph.drawOffset.word_80091738;
    outRuntime.drawEnvOffsetY8009173A = graph.drawOffset.word_8009173A;
    result.built = FillClearedOtSlots8003FA20(outRuntime.ot, pageWork.work);
    return result;
}

PsxFastSpriteRuntimeBuildResult8003FA20
BuildMovie1TextMainRuntimeState8003FA20FromGraphOwner(
    PsxGraphState& graph,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime) {
    outRuntime = PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20{};
    if (!graph.mainPageWorkLists80087288Initialized) {
        PsxSeedMainPageWorkLists80087288(graph);
    }

    PsxFastSpriteRuntimeBuildResult8003FA20 result{};
    result.slot8004019C = static_cast<uint8_t>(
        PsxCall8004019C_GetDrawBuffer(graph) & 1u);
    result.seedFound = true;
    result.packetAllocatorBase8006ED50 =
        graph.dword_8006ED50[result.slot8004019C];

    PsxCall80040F90_SetPacketAllocator(
        graph,
        result.packetAllocatorBase8006ED50);
    PsxGraphPageWorkList8001E374& pageWork =
        graph.mainPageWorkLists80087288[result.slot8004019C];
    result.clearWorkResult =
        PsxCall80040CC8_ClearWorkList(pageWork.work, 0u, 0u);
    result.built =
        BuildRuntimeState8003FA20FromPageWork(graph, pageWork, outRuntime);
    return result;
}

PsxFastSpriteRuntimeBuildResult8003FA20
BuildMovie1SmallOtRuntimeState8003FA20FromGraphOwner(
    PsxGraphState& graph,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& outRuntime) {
    constexpr PsxFastSpriteRuntimeWorkSeed8003FA20 kMovie1SmallOtSeeds[] = {
        {0u,
         0x801A73B0u,
         {4u, 0x800872B0u, 0u, 0u, 0u, false, 0u, 0u}},
        {1u,
         0x801B54B0u,
         {4u, 0x800872F0u, 0u, 0u, 0u, false, 0u, 0u}},
    };
    return BuildRuntimeState8003FA20FromGraphOwner(
        graph,
        kMovie1SmallOtSeeds,
        sizeof(kMovie1SmallOtSeeds) / sizeof(kMovie1SmallOtSeeds[0]),
        outRuntime);
}

PsxGsSpriteRuntimeBuildResult8003F1B4
BuildStage1CompactRailRuntimeState8003F1B4FromGraphOwner(
    PsxGraphState& graph,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& outRuntime) {
    outRuntime = PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4{};
    if (!graph.mainPageWorkLists80087288Initialized) {
        PsxSeedMainPageWorkLists80087288(graph);
    }

    PsxGsSpriteRuntimeBuildResult8003F1B4 result{};
    result.slot8004019C = static_cast<uint8_t>(
        PsxCall8004019C_GetDrawBuffer(graph) & 1u);
    result.seedFound = true;
    result.packetAllocatorBase8006ED50 =
        graph.dword_8006ED50[result.slot8004019C];

    PsxCall80040F90_SetPacketAllocator(
        graph,
        result.packetAllocatorBase8006ED50);
    PsxGraphPageWorkList8001E374& pageWork =
        graph.mainPageWorkLists80087288[result.slot8004019C];
    result.clearWorkResult =
        PsxCall80040CC8_ClearWorkList(pageWork.work, 0u, 0u);

    outRuntime.dword_800901C8 = graph.dword_800901C8;
    outRuntime.word_800917AA = graph.drawOffset.word_800917AA;
    outRuntime.word_800917AC = graph.drawOffset.word_800917AC;
    outRuntime.gte = graph.gte;
    outRuntime.ot.headAddr_04 = pageWork.work.headAddr_04;
    outRuntime.ot.length_08 = pageWork.work.x_08;
    if (pageWork.work.gsSpriteOtSlotMirrorKnown) {
        outRuntime.ot.slotValues = pageWork.work.gsSpriteOtSlotMirror;
        result.built = true;
    } else {
        result.built =
            FillClearedOtSlots8003F1B4(outRuntime.ot, pageWork.work);
    }
    if (pageWork.work.gsSpritePacketWriteMirrorKnown) {
        outRuntime.packetWrites = pageWork.work.gsSpritePacketWriteMirror;
    }
    return result;
}

PsxGsSpriteRuntimeCommitResult8003F1B4
CommitRuntimeState8003F1B4ToMainPageWork(
    PsxGraphState& graph,
    uint8_t pageIndex,
    const PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& runtime) {
    PsxGsSpriteRuntimeCommitResult8003F1B4 out{};
    out.runtimeValid = runtime.ot.headAddr_04 != 0u;
    if (!graph.mainPageWorkLists80087288Initialized) {
        return out;
    }

    const uint8_t slot = pageIndex & 1u;
    out.pageWorkFound = true;
    out.slot8004019C = slot;
    PsxGraphPageWorkList8001E374& pageWork =
        graph.mainPageWorkLists80087288[slot];
    out.workListAddr = pageWork.workAddr;
    if (!out.runtimeValid ||
        runtime.ot.headAddr_04 != pageWork.work.headAddr_04 ||
        runtime.ot.length_08 != pageWork.work.x_08) {
        return out;
    }

    out.allocatorBefore = graph.dword_800901C8;
    out.allocatorAfter = runtime.dword_800901C8;
    graph.dword_800901C8 = runtime.dword_800901C8;
    out.allocatorUpdated = true;

    pageWork.work.gsSpriteOtSlotMirror = runtime.ot.slotValues;
    pageWork.work.gsSpriteOtSlotMirrorKnown = true;
    out.otSlotMirrorUpdated = true;
    for (const PrPsxGsSpriteSubmitDirect::RuntimeOtSlotValue8003F1B4&
             slotValue : runtime.ot.slotValues) {
        if (slotValue.valid) {
            ++out.otSlotCount;
        }
    }

    pageWork.work.gsSpritePacketWriteMirror = runtime.packetWrites;
    pageWork.work.gsSpritePacketWriteMirrorKnown = true;
    out.packetWriteMirrorUpdated = true;
    out.packetMemoryGap = false;
    for (const PrPsxGsSpriteSubmitDirect::RuntimePacketWrite8003F1B4& write :
         runtime.packetWrites) {
        if (!write.valid) {
            continue;
        }
        ++out.packetWriteCount;
        for (uint32_t i = 0; i < write.wordCount && i < write.wordKnown.size();
             ++i) {
            if (!write.wordKnown[i]) {
                out.packetMemoryGap = true;
                break;
            }
        }
    }

    out.committed = out.runtimeValid && out.pageWorkFound;
    return out;
}

} // namespace PrPsxGraphOwnerDirect
