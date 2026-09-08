#include "pr_ss0_title_packet_work_direct.h"

#include "pr_psx_dma_submit_direct.h"
#include "pr_stage1_loader_memory_direct.h"

namespace PrSS0TitlePacketWorkDirect {
namespace {

constexpr uint32_t kPacketArenaBase801C4260 =
    kPrStage1LoaderMemoryDirectHeapEnd801C35B0 -
    static_cast<uint32_t>(kPacketArenaBytes801C5D28);
static_assert(kPacketArenaBase801C4260 == 0x801AE430u,
              "Scene0 packet arena base must remain exact");

bool IsExactPreparedWorkList(
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint32_t expectedOrder,
    uint32_t expectedHead)
{
    const uint32_t expectedLength = 1u << expectedOrder;
    const uint32_t expectedTail =
        expectedHead + expectedLength * 4u - 4u;
    return work.order_00 == expectedOrder &&
           work.headAddr_04 == expectedHead && work.x_08 == 0u &&
           work.y_0C == 0u && work.lastAddr_10 == expectedTail &&
           work.clearOtagRCalled &&
           work.clearOtagRHeadAddr == expectedHead &&
           work.clearOtagRLength == expectedLength;
}

bool IsExactPacketLaneBinding801C609C(
    const RuntimeState801C609C& state)
{
    const auto& binding = state.packetLaneBinding8001E33C;
    return binding.known && binding.sourceFunction == kFn801C609C &&
           binding.calleeFunction == 0x8001E33Cu && binding.arg0Known &&
           state.packetLaneBindingCallsite801C60C8 ==
               kCallsite801C60C8 &&
           binding.arg0 == state.packetAllocatorBases801C956C[0] &&
           binding.arg1Known &&
           binding.arg1 == state.packetAllocatorBases801C956C[1] &&
           binding.psxWouldWriteGpPlus310 &&
           binding.psxWouldWriteGpPlus314 && !binding.hostSideEffects &&
           binding.gpPlus310Offset ==
               PrSceneDrawBufferDirect::kGpOffsetDrawBufferA8001E33C &&
           binding.gpPlus314Offset ==
               PrSceneDrawBufferDirect::kGpOffsetDrawBufferB8001E33C &&
           binding.gpPlus310 == state.packetAllocatorBases801C956C[0] &&
           binding.gpPlus314 == state.packetAllocatorBases801C956C[1];
}

} // namespace

void Clear(RuntimeState801C609C& state)
{
    state = {};
}

void ResetTitleFrameGates801C609C(RuntimeState801C609C& state)
{
    state.frameFlagsApplied801C6410 = false;
    state.titleRenderActive801C9548 = 0u;
    state.loHpGate801CB600 = 0u;
    state.presentExtra801CFAEC = 0u;
    state.titleShortcutSetupApplied801C6410 = false;
    state.titleShortcutReadyApplied801C6410 = false;
    state.frameSetupSource801C6410 = TitleFrameSetupSource801C6410::None;
}

InitializeResult801C609C Initialize801C609C(
    RuntimeState801C609C& state,
    PrStage1LoaderMemoryDirectState* loaderMemory,
    PrPsxGraphOwnerDirect::PsxGraphState& graph)
{
    Clear(state);
    InitializeResult801C609C out{};
    if (loaderMemory == nullptr ||
        !graph.mainPageWorkLists80087288Initialized) {
        return out;
    }

    const PrStage1LoaderMemoryDirectAllocResult allocation =
        PrStage1LoaderMemoryDirectApply80025B28(
            *loaderMemory, kPacketArenaBytes801C5D28);
    if (!allocation.success ||
        allocation.alignedBytes !=
            static_cast<uint32_t>(kPacketArenaBytes801C5D28) ||
        allocation.psxAddress + 2u * kPacketArenaLaneBytes801C5D28 <
            allocation.psxAddress ||
        allocation.psxAddress + 2u * kPacketArenaLaneBytes801C5D28 >
            loaderMemory->gpPlus900HeapEnd) {
        return out;
    }

    for (uint32_t lane = 0; lane < 2u; ++lane) {
        state.packetAllocatorBases801C956C[lane] =
            allocation.psxAddress + lane * kPacketArenaLaneBytes801C5D28;
    }
    state.packetLaneBinding8001E33C =
        PrSceneDrawBufferDirect::PsxCall8001E33C_SetDrawBufferGlobals(
            state.packetAllocatorBases801C956C[0],
            state.packetAllocatorBases801C956C[1]);
    state.packetLaneBinding8001E33C.sourceFunction = kFn801C609C;
    state.packetLaneBinding8001E33C.calleeFunction = 0x8001E33Cu;
    state.packetLaneBindingCallsite801C60C8 = kCallsite801C60C8;
    if (!IsExactPacketLaneBinding801C609C(state)) {
        Clear(state);
        return out;
    }

    const std::array<uint32_t, 2> nextPacketLaneGlobals8006ED50 = {
        state.packetLaneBinding8001E33C.gpPlus310,
        state.packetLaneBinding8001E33C.gpPlus314,
    };
    if (nextPacketLaneGlobals8006ED50 !=
        state.packetAllocatorBases801C956C) {
        Clear(state);
        return out;
    }
    graph.dword_8006ED50 = nextPacketLaneGlobals8006ED50;

    state.packetArenaBase80025B28 = allocation.psxAddress;
    auto& work = state.workLists801C9574[kTitleWorkFixedLane801C609C];
    work.order_00 = kWorkOrder801C609C;
    work.headAddr_04 = kWork0OtHead801C959C;
    work.x_08 = 0;
    work.y_0C = 0;
    // Keep the second slot inert.  It is retained in the host-side state
    // shape for compatibility with existing probes, but it is not a title
    // work list in the original COMOD0 call chain.
    state.workLists801C9574[1] = {};
    state.initialized = true;

    out.initialized = true;
    out.packetArenaBase80025B28 = allocation.psxAddress;
    out.packetArenaBytes = allocation.alignedBytes;
    out.packetLaneBinding8001E33C = state.packetLaneBinding8001E33C;
    out.packetLaneBindingCallsite801C60C8 =
        state.packetLaneBindingCallsite801C60C8;
    out.packetLaneGraphBindingApplied8001E33C = true;
    return out;
}

LoaderResetResult801C4260
ApplyLoaderReset80025A34PreservingPacketArena801C4260(
    const RuntimeState801C609C& state,
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    PrStage1LoaderMemoryDirectState& loaderMemory)
{
    LoaderResetResult801C4260 out{};
    if (!state.initialized) {
        return out;
    }
    if (state.packetArenaBase80025B28 != kPacketArenaBase801C4260) {
        out.failure = LoaderResetFailure801C4260::PacketArenaMismatch;
        return out;
    }

    const std::array<uint32_t, 2> expectedPacketLanes = {
        kPacketArenaBase801C4260,
        kPacketArenaBase801C4260 + kPacketArenaLaneBytes801C5D28,
    };
    if (state.packetAllocatorBases801C956C != expectedPacketLanes) {
        out.failure = LoaderResetFailure801C4260::PacketLaneMismatch;
        return out;
    }
    if (!IsExactPacketLaneBinding801C609C(state)) {
        out.failure = LoaderResetFailure801C4260::PacketBindingMismatch;
        return out;
    }
    if (graph.dword_8006ED50 != expectedPacketLanes) {
        out.failure = LoaderResetFailure801C4260::GraphPacketLaneMismatch;
        return out;
    }
    if (loaderMemory.gpPlus896HeapBase !=
        kPrStage1LoaderMemoryDirectHeapBase800965B0) {
        out.failure = LoaderResetFailure801C4260::LoaderHeapBaseMismatch;
        return out;
    }
    if (loaderMemory.gpPlus900HeapEnd !=
        kPrStage1LoaderMemoryDirectHeapEnd801C35B0) {
        out.failure = LoaderResetFailure801C4260::LoaderHeapEndMismatch;
        return out;
    }
    if (loaderMemory.gpPlus320LowWater != kPacketArenaBase801C4260) {
        out.failure = LoaderResetFailure801C4260::LoaderLowWaterMismatch;
        return out;
    }
    if (loaderMemory.gpPlus904HeapCursor <
            kPrStage1LoaderMemoryDirectHeapBase800965B0 ||
        loaderMemory.gpPlus904HeapCursor >= kPacketArenaBase801C4260) {
        out.failure = LoaderResetFailure801C4260::LoaderHeapCursorMismatch;
        return out;
    }

    loaderMemory.gpPlus320LowWater = kPacketArenaBase801C4260;
    loaderMemory.gpPlus324StackDepth = 0u;
    loaderMemory.gpPlus896HeapBase =
        kPrStage1LoaderMemoryDirectHeapBase800965B0;
    loaderMemory.gpPlus900HeapEnd =
        kPrStage1LoaderMemoryDirectHeapEnd801C35B0;
    loaderMemory.gpPlus904HeapCursor =
        kPrStage1LoaderMemoryDirectHeapBase800965B0;
    loaderMemory.stackTable80091858.fill(0u);

    out.applied = true;
    out.failure = LoaderResetFailure801C4260::None;
    return out;
}

BeginFrameResult801C6410 BeginFrame801C6410(
    RuntimeState801C609C& state,
    PrPsxGraphOwnerDirect::PsxGraphState& graph)
{
    BeginFrameResult801C6410 out{};
    state.framePrepared801C6410 = false;
    state.frameFlagsApplied801C6410 = false;
    state.frameSetupSource801C6410 = TitleFrameSetupSource801C6410::None;
    state.currentDrawBuffer8004019C = 0;
    state.currentPacketAllocator800901C8 = 0;
    if (!state.initialized || !IsExactPacketLaneBinding801C609C(state) ||
        graph.dword_8006ED50 != state.packetAllocatorBases801C956C ||
        !graph.mainPageWorkLists80087288Initialized) {
        return out;
    }

    const uint8_t slot = static_cast<uint8_t>(
        PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(graph) & 1u);
    const uint32_t allocator = state.packetAllocatorBases801C956C[slot];
    auto& work = state.workLists801C9574[kTitleWorkFixedLane801C609C];
    if (allocator == 0u || work.order_00 != kWorkOrder801C609C ||
        work.headAddr_04 != kWork0OtHead801C959C ||
        work.x_08 != 0u || work.y_0C != 0u) {
        return out;
    }

    PrPsxGraphOwnerDirect::PsxCall80040F90_SetPacketAllocator(
        graph, allocator);
    out.titleClear80040CC8 =
        PrPsxGraphOwnerDirect::PsxCall80040CC8_ClearWorkList(
            work, 0u, 0u);
    out.mainPageClear8001E374 =
        PrPsxGraphOwnerDirect::PsxCall8001E374_ClearMainPageWork(
            graph, kMainPageFixedLane8001E3B0);

    state.framePrepared801C6410 = true;
    state.currentDrawBuffer8004019C = slot;
    state.currentPacketAllocator800901C8 = allocator;

    out.prepared = true;
    out.drawBuffer8004019C = slot;
    out.packetAllocator800901C8 = allocator;
    out.workAddress801C9574 = kWork0Address801C9574;
    return out;
}

namespace {

FrameFlagsResult801C6410 ApplyFrameFlagsCore801C6410(
    RuntimeState801C609C& state,
    uint32_t ctxFlags,
    TitleFrameSetupSource801C6410 source,
    bool allowShortcutFlags)
{
    FrameFlagsResult801C6410 out{};
    out.ctxFlags = ctxFlags;
    out.titleRenderActiveBefore801C9548 =
        state.titleRenderActive801C9548;
    out.titleRenderActiveAfter801C9548 =
        state.titleRenderActive801C9548;
    out.loHpGateBefore801CB600 = state.loHpGate801CB600;
    out.loHpGateAfter801CB600 = state.loHpGate801CB600;
    out.presentExtraBefore801CFAEC = state.presentExtra801CFAEC;
    out.presentExtraAfter801CFAEC = state.presentExtra801CFAEC;
    if (!state.initialized || !state.framePrepared801C6410) {
        return out;
    }
    if (state.frameFlagsApplied801C6410) {
        out.failure = FrameFlagsFailure801C6410::AlreadyApplied;
        return out;
    }
    if (!allowShortcutFlags &&
        (ctxFlags & kCtxFlagTitleShortcutSetup801C6410) != 0u) {
        out.failure = FrameFlagsFailure801C6410::ShortcutSetupUnsupported;
        return out;
    }
    if (!allowShortcutFlags &&
        (ctxFlags & kCtxFlagTitleImmediateSetup801C6410) != 0u) {
        out.failure = FrameFlagsFailure801C6410::ImmediateSetupUnsupported;
        return out;
    }
    uint32_t nextTitleRenderActive = state.titleRenderActive801C9548;
    uint32_t nextLoHpGate = state.loHpGate801CB600;
    uint32_t nextPresentExtra = state.presentExtra801CFAEC;
    bool nextShortcutSetup = state.titleShortcutSetupApplied801C6410;
    bool nextShortcutReady = state.titleShortcutReadyApplied801C6410;
    if ((ctxFlags & kCtxFlagTitleRenderActive801C6410) != 0u) {
        nextTitleRenderActive = 1u;
    }
    if ((ctxFlags & kCtxFlagTitleShortcutSetup801C6410) != 0u) {
        nextPresentExtra = 1u;
        nextTitleRenderActive = 0u;
        nextShortcutSetup = true;
        nextShortcutReady = false;
        out.shortcutSetupWriter801C64E0 = true;
    }
    if ((ctxFlags & kCtxFlagTitleImmediateSetup801C6410) != 0u) {
        if (!nextShortcutSetup) {
            out.failure =
                FrameFlagsFailure801C6410::ShortcutReadyBeforeSetup;
            return out;
        }
        nextPresentExtra = 0u;
        nextLoHpGate = 1u;
        nextTitleRenderActive = 1u;
        nextShortcutReady = true;
        out.shortcutReadyWriter801C663C = true;
    }
    if (nextTitleRenderActive != 0u &&
        (ctxFlags & kCtxFlagLoPairWrite801C6410) != 0u) {
        nextLoHpGate = 1u;
        out.loHpWriterFromPairSetup801C66B0 = true;
    }

    state.titleRenderActive801C9548 = nextTitleRenderActive;
    state.loHpGate801CB600 = nextLoHpGate;
    state.presentExtra801CFAEC = nextPresentExtra;
    state.titleShortcutSetupApplied801C6410 = nextShortcutSetup;
    state.titleShortcutReadyApplied801C6410 = nextShortcutReady;
    state.frameFlagsApplied801C6410 = true;
    state.frameSetupSource801C6410 = source;

    out.applied = true;
    out.failure = FrameFlagsFailure801C6410::None;
    out.titleRenderActiveAfter801C9548 = nextTitleRenderActive;
    out.loHpGateAfter801CB600 = nextLoHpGate;
    out.presentExtraAfter801CFAEC = nextPresentExtra;
    return out;
}

} // namespace

FrameFlagsResult801C6410 ApplyTitleFrameFlags801C6410(
    RuntimeState801C609C& state,
    uint32_t ctxFlags)
{
    return ApplyFrameFlagsCore801C6410(
        state,
        ctxFlags,
        TitleFrameSetupSource801C6410::EventCtxFlags,
        false);
}

EarlyInputShortcutResult801C6410
ApplyEarlyInputShortcutSetupFrame801C6410(
    RuntimeState801C609C& state,
    PrPsxGraphOwnerDirect::PsxGraphState& graph)
{
    EarlyInputShortcutResult801C6410 out{};
    if (!state.initialized ||
        !graph.mainPageWorkLists80087288Initialized) {
        return out;
    }
    if (state.titleShortcutSetupApplied801C6410) {
        out.failure = EarlyInputShortcutFailure801C6410::SetupAlreadyApplied;
        return out;
    }

    RuntimeState801C609C nextState = state;
    PrPsxGraphOwnerDirect::PsxGraphState nextGraph = graph;
    out.frame = BeginFrame801C6410(nextState, nextGraph);
    if (!out.frame.prepared) {
        out.failure = EarlyInputShortcutFailure801C6410::FramePrepareFailed;
        return out;
    }
    out.flags = ApplyFrameFlagsCore801C6410(
        nextState,
        kCtxFlagTitleShortcutSetup801C6410,
        TitleFrameSetupSource801C6410::EarlyInputShortcut,
        true);
    if (!out.flags.applied ||
        !nextState.titleShortcutSetupApplied801C6410 ||
        nextState.titleShortcutReadyApplied801C6410) {
        out.failure = EarlyInputShortcutFailure801C6410::FlagApplyFailed;
        return out;
    }

    state = nextState;
    graph = nextGraph;
    out.committed = true;
    out.failure = EarlyInputShortcutFailure801C6410::None;
    return out;
}

EarlyInputShortcutResult801C6410
ApplyEarlyInputShortcutReadyFrame801C6410(
    RuntimeState801C609C& state,
    PrPsxGraphOwnerDirect::PsxGraphState& graph)
{
    EarlyInputShortcutResult801C6410 out{};
    if (!state.initialized ||
        !graph.mainPageWorkLists80087288Initialized) {
        return out;
    }
    if (!state.titleShortcutSetupApplied801C6410) {
        out.failure = EarlyInputShortcutFailure801C6410::SetupMissing;
        return out;
    }
    if (state.titleShortcutReadyApplied801C6410) {
        out.failure = EarlyInputShortcutFailure801C6410::ReadyAlreadyApplied;
        return out;
    }

    RuntimeState801C609C nextState = state;
    PrPsxGraphOwnerDirect::PsxGraphState nextGraph = graph;
    out.frame = BeginFrame801C6410(nextState, nextGraph);
    if (!out.frame.prepared) {
        out.failure = EarlyInputShortcutFailure801C6410::FramePrepareFailed;
        return out;
    }
    out.flags = ApplyFrameFlagsCore801C6410(
        nextState,
        kCtxFlagTitleImmediateSetup801C6410,
        TitleFrameSetupSource801C6410::EarlyInputShortcut,
        true);
    if (!out.flags.applied ||
        !nextState.titleShortcutSetupApplied801C6410 ||
        !nextState.titleShortcutReadyApplied801C6410) {
        out.failure = EarlyInputShortcutFailure801C6410::FlagApplyFailed;
        return out;
    }

    state = nextState;
    graph = nextGraph;
    out.committed = true;
    out.failure = EarlyInputShortcutFailure801C6410::None;
    return out;
}

PresentFrameResult801C689C ApplyPresentFrameModel801C689C(
    RuntimeState801C609C& state,
    PrPsxGraphOwnerDirect::PsxGraphState& graph)
{
    PresentFrameResult801C689C out{};
    out.cachedDrawLane8006EDA8 = state.currentDrawBuffer8004019C;
    if (!state.initialized || !state.framePrepared801C6410 ||
        !graph.mainPageWorkLists80087288Initialized) {
        return out;
    }
    if (!state.frameFlagsApplied801C6410) {
        out.failure = PresentFrameFailure801C689C::FrameFlagsNotApplied;
        return out;
    }

    const uint8_t lane = state.currentDrawBuffer8004019C;
    if (lane >= graph.mainPageWorkLists80087288.size()) {
        out.failure = PresentFrameFailure801C689C::CachedLaneInvalid;
        return out;
    }
    if (graph.word_80096590 > 1u || graph.word_80096590 != lane) {
        out.failure = PresentFrameFailure801C689C::GraphSlotMismatch;
        return out;
    }

    const uint64_t arenaBase = state.packetArenaBase80025B28;
    const uint64_t laneBytes = kPacketArenaLaneBytes801C5D28;
    const uint64_t laneBase = arenaBase + lane * laneBytes;
    const uint64_t laneEnd = laneBase + laneBytes;
    if (arenaBase == 0u ||
        state.packetAllocatorBases801C956C[0] != arenaBase ||
        state.packetAllocatorBases801C956C[1] != arenaBase + laneBytes ||
        state.currentPacketAllocator800901C8 < laneBase ||
        state.currentPacketAllocator800901C8 > laneEnd ||
        graph.dword_800901C8 !=
            state.currentPacketAllocator800901C8) {
        out.failure = PresentFrameFailure801C689C::AllocatorMismatch;
        return out;
    }

    const uint8_t titleLane = kTitleWorkFixedLane801C609C;
    const uint32_t titleWorkAddress = kWork0Address801C9574;
    const uint32_t titleHeadAddress = kWork0OtHead801C959C;
    if (!IsExactPreparedWorkList(
            state.workLists801C9574[titleLane],
            kWorkOrder801C609C,
            titleHeadAddress)) {
        out.failure = PresentFrameFailure801C689C::TitleWorkMismatch;
        return out;
    }

    // 801C689C calls 8001E3B0(0), not 8001E3B0(current title lane).  Keep
    // this independent page fixed at lane zero even while 80040370 toggles
    // the title packet lane.
    const uint8_t mainLane = kMainPageFixedLane8001E3B0;
    const auto& mainPage = graph.mainPageWorkLists80087288[mainLane];
    const uint32_t mainWorkAddress =
        kMainPageWork0Address80087288 +
        mainLane * kMainPageWorkLaneStride8001E3B0;
    const uint32_t mainHeadAddress =
        kMainPageOtHead0Address80088288 +
        mainLane * kMainPageOtHeadLaneStride8001E3B0;
    if (mainPage.workAddr != mainWorkAddress ||
        mainPage.otHeadAddr != mainHeadAddress ||
        !IsExactPreparedWorkList(
            mainPage.work,
            kMainPageWorkOrder8001E3B0,
            mainHeadAddress)) {
        out.failure = PresentFrameFailure801C689C::MainPageWorkMismatch;
        return out;
    }

    // Original 801C689C flips the graph first, then calls 80040060 and
    // 80040CA4 on the title work list, followed by 8001E3B0(0), whose body
    // calls 80040CA4 on the selected main-page list.  Build the local graph
    // flip before these two translated software transactions so a rejected
    // submit cannot publish a half-advanced global state.
    auto nextGraph = graph;
    const auto flip =
        PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(nextGraph);
    const uint16_t expectedNextLane =
        static_cast<uint16_t>(lane ^ 1u);
    if (flip.previousSlot != lane || flip.nextSlot != expectedNextLane ||
        nextGraph.word_80096590 != expectedNextLane) {
        out.failure = PresentFrameFailure801C689C::FlipModelMismatch;
        return out;
    }

    // 80040060 runs against the post-flip 80096590 lane.  Its source pointer
    // is the fixed title work record at 801C9574, and the resulting 16-byte
    // draw-environment packet is linked through that record's OT head.  Keep
    // the exact PSX address/fields as software state for later HAL consumers.
    TitleDrawEnvState80040060 drawEnv{};
    drawEnv.known = true;
    drawEnv.graphSlot80096590 = nextGraph.word_80096590;
    drawEnv.sourceWorkListAddress801C9574 = kWork0Address801C9574;
    drawEnv.sourceOtagHeadAddress =
        state.workLists801C9574[titleLane].lastAddr_10;
    drawEnv.commandAddress8008A730 =
        kDrawEnvBase8008A730 +
        static_cast<uint32_t>(drawEnv.graphSlot80096590) *
            kDrawEnvStride80040060;
    drawEnv.r = 255u;
    drawEnv.g = 255u;
    drawEnv.b = 255u;
    drawEnv.clipX =
        nextGraph.word_8008ECA8[drawEnv.graphSlot80096590];
    drawEnv.clipY =
        nextGraph.word_8008ECAC[drawEnv.graphSlot80096590];
    drawEnv.clipWidth = static_cast<uint16_t>(nextGraph.word_800928D4);
    drawEnv.clipHeight = static_cast<uint16_t>(nextGraph.word_800928D6);

    // Execute both translated software transactions without writing host
    // MMIO or routing title presentation through the old S0 renderer.
    PrPsxDmaSubmitDirect::PsxWorkListSubmitInput80040CA4 titleSubmitInput{};
    titleSubmitInput.workListAddress = titleWorkAddress;
    titleSubmitInput.workListSlot = titleLane;
    titleSubmitInput.order = state.workLists801C9574[titleLane].order_00;
    titleSubmitInput.headAddress = state.workLists801C9574[titleLane].headAddr_04;
    titleSubmitInput.lastAddressOffset10 =
        state.workLists801C9574[titleLane].lastAddr_10;
    titleSubmitInput.clearOtagRCalled =
        state.workLists801C9574[titleLane].clearOtagRCalled;
    titleSubmitInput.priorCallCount = state.titleSubmitCallCount80040CA4;
    titleSubmitInput.expectedWorkListBaseAddress = kWork0Address801C9574;
    titleSubmitInput.expectedWorkListStride = kWorkLaneStride801C609C;
    titleSubmitInput.expectedWorkListCount =
        static_cast<uint32_t>(titleLane + 1u);
    const auto titleSubmit =
        PrPsxDmaSubmitDirect::ExecuteWorkListSubmitSoftware80040CA4(
            titleSubmitInput);
    if (titleSubmit.status !=
            PrPsxDmaSubmitDirect::PsxWorkListSubmitStatus80040CA4::Executed ||
        !titleSubmit.drawOtag800450A0Called ||
        !titleSubmit.softwareStateCommitted ||
        titleSubmit.hostMmioWritten || titleSubmit.hostGpuSubmitted ||
        titleSubmit.callCount != state.titleSubmitCallCount80040CA4 + 1u) {
        out.failure = PresentFrameFailure801C689C::TitleWorkSubmitFailed;
        return out;
    }

    PrPsxDmaSubmitDirect::PsxWorkListSubmitInput80040CA4 mainSubmitInput{};
    mainSubmitInput.workListAddress = mainWorkAddress;
    mainSubmitInput.workListSlot = mainLane;
    mainSubmitInput.order = mainPage.work.order_00;
    mainSubmitInput.headAddress = mainPage.work.headAddr_04;
    mainSubmitInput.lastAddressOffset10 = mainPage.work.lastAddr_10;
    mainSubmitInput.clearOtagRCalled = mainPage.work.clearOtagRCalled;
    mainSubmitInput.priorCallCount = state.mainSubmitCallCount8001E3B0;
    const auto mainSubmit =
        PrPsxDmaSubmitDirect::ExecuteWorkListSubmitSoftware80040CA4(
            mainSubmitInput);
    if (mainSubmit.status !=
            PrPsxDmaSubmitDirect::PsxWorkListSubmitStatus80040CA4::Executed ||
        !mainSubmit.drawOtag800450A0Called ||
        !mainSubmit.softwareStateCommitted || mainSubmit.hostMmioWritten ||
        mainSubmit.hostGpuSubmitted ||
        mainSubmit.callCount != state.mainSubmitCallCount8001E3B0 + 1u) {
        out.failure = PresentFrameFailure801C689C::MainPageWorkSubmitFailed;
        return out;
    }

    const bool presentExtra = state.presentExtra801CFAEC != 0u;
    PrPsxGraphOwnerDirect::PsxGraphFlipResult80040370 extraFlip{};
    if (presentExtra) {
        extraFlip =
            PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(nextGraph);
        if (extraFlip.previousSlot != expectedNextLane ||
            extraFlip.nextSlot != lane ||
            nextGraph.word_80096590 != lane) {
            out.failure = PresentFrameFailure801C689C::FlipModelMismatch;
            return out;
        }
    }

    graph = nextGraph;
    state.framePrepared801C6410 = false;
    state.titleSubmitCallCount80040CA4 = titleSubmit.callCount;
    state.mainSubmitCallCount8001E3B0 = mainSubmit.callCount;
    state.drawEnv80040060 = drawEnv;

    out.modelApplied = true;
    out.failure = PresentFrameFailure801C689C::None;
    out.titleWorkAddress801C9574 = titleWorkAddress;
    out.mainPageWorkAddress80087288 = mainWorkAddress;
    out.clearR80040060 = 255u;
    out.clearG80040060 = 255u;
    out.clearB80040060 = 255u;
    out.flipGraphStateAdvanced80040370 = true;
    out.whiteDrawEnv80040060Requested = true;
    out.titleWork80040CA4SubmitRequested = true;
    out.mainPageWork8001E3B0SubmitRequested = true;
    out.presentExtraSource801CFAEC = presentExtra;
    out.extraFlipGraphStateAdvanced80040370 = presentExtra;
    out.fullHeightWhiteFill8001B1B0Requested = presentExtra;
    out.titleWorkSubmit80040CA4Executed = true;
    out.mainPageWorkSubmit8001E3B0Executed = true;
    out.titleWorkSubmitCallCount80040CA4 = titleSubmit.callCount;
    out.mainPageWorkSubmitCallCount8001E3B0 = mainSubmit.callCount;
    out.drawEnv80040060 = drawEnv;
    if (presentExtra) {
        out.fullHeightWhiteFillX8001B1B0 = kPresentExtraFillX8001B1B0;
        out.fullHeightWhiteFillY8001B1B0 = kPresentExtraFillY8001B1B0;
        out.fullHeightWhiteFillWidth8001B1B0 =
            kPresentExtraFillWidth8001B1B0;
        out.fullHeightWhiteFillHeight8001B1B0 =
            kPresentExtraFillHeight8001B1B0;
    }
    out.flip = flip;
    out.extraFlip = extraFlip;
    return out;
}

const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8*
GetCurrentWorkList801C6410(const RuntimeState801C609C& state)
{
    if (!state.initialized || !state.framePrepared801C6410 ||
        state.currentDrawBuffer8004019C >= state.workLists801C9574.size()) {
        return nullptr;
    }
    return &state.workLists801C9574[kTitleWorkFixedLane801C609C];
}

} // namespace PrSS0TitlePacketWorkDirect
