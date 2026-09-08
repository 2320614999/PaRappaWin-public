#include "pr/pr_ss0_title_packet_work_direct.h"
#include "pr/pr_ss0_transition_direct.h"
#include "pr/pr_stage1_loader_memory_direct.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <memory>
#include <vector>

namespace {

struct PresentFixture {
    std::unique_ptr<PrStage1LoaderMemoryDirectState> loader;
    std::unique_ptr<PrSS0TitlePacketWorkDirect::RuntimeState801C609C>
        runtime;
    std::unique_ptr<PrPsxGraphOwnerDirect::PsxGraphState> graph;

    PresentFixture()
        : loader(std::make_unique<PrStage1LoaderMemoryDirectState>()),
          runtime(std::make_unique<
                  PrSS0TitlePacketWorkDirect::RuntimeState801C609C>()),
          graph(std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>())
    {
        PrStage1LoaderMemoryDirectReset(*loader);
        PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
            *graph, 320u, 240u);
        assert(PrSS0TitlePacketWorkDirect::Initialize801C609C(
                   *runtime, loader.get(), *graph)
                   .initialized);
        assert(PrSS0TitlePacketWorkDirect::BeginFrame801C6410(
                   *runtime, *graph)
                   .prepared);
        assert(PrSS0TitlePacketWorkDirect::ApplyTitleFrameFlags801C6410(
                   *runtime,
                   PrSS0TitlePacketWorkDirect::
                       kCtxFlagTitleRenderActive801C6410)
                   .applied);
    }
};

struct LoaderResetFixture {
    std::unique_ptr<PrStage1LoaderMemoryDirectState> loader;
    std::unique_ptr<PrSS0TitlePacketWorkDirect::RuntimeState801C609C>
        runtime;
    std::unique_ptr<PrPsxGraphOwnerDirect::PsxGraphState> graph;

    LoaderResetFixture()
        : loader(std::make_unique<PrStage1LoaderMemoryDirectState>()),
          runtime(std::make_unique<
                  PrSS0TitlePacketWorkDirect::RuntimeState801C609C>()),
          graph(std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>())
    {
        PrStage1LoaderMemoryDirectReset(*loader);
        PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
            *graph, 320u, 240u);
        assert(PrSS0TitlePacketWorkDirect::Initialize801C609C(
                   *runtime, loader.get(), *graph)
                   .initialized);
    }
};

template <typename T>
std::vector<uint8_t> SnapshotBytes(const T& value)
{
    const auto* bytes = reinterpret_cast<const uint8_t*>(&value);
    return std::vector<uint8_t>(bytes, bytes + sizeof(value));
}

template <typename T>
void AssertBytesUnchanged(const T& value,
                          const std::vector<uint8_t>& before)
{
    assert(before.size() == sizeof(value));
    assert(std::memcmp(&value, before.data(), sizeof(value)) == 0);
}

void AssertLoaderResetRejectedWithoutWrites(
    LoaderResetFixture& fixture,
    PrSS0TitlePacketWorkDirect::LoaderResetFailure801C4260 expectedFailure)
{
    const auto runtimeBefore = SnapshotBytes(*fixture.runtime);
    const auto graphBefore = SnapshotBytes(*fixture.graph);
    const auto loaderBefore = SnapshotBytes(*fixture.loader);
    const auto result = PrSS0TitlePacketWorkDirect::
        ApplyLoaderReset80025A34PreservingPacketArena801C4260(
            *fixture.runtime, *fixture.graph, *fixture.loader);
    assert(!result.applied);
    assert(result.failure == expectedFailure);
    AssertBytesUnchanged(*fixture.runtime, runtimeBefore);
    AssertBytesUnchanged(*fixture.graph, graphBefore);
    AssertBytesUnchanged(*fixture.loader, loaderBefore);
}

void TestLoaderResetPreservingPacketArena801C4260()
{
    using namespace PrSS0TitlePacketWorkDirect;

    LoaderResetFixture success;
    success.loader->heap800965B0.front() = 0x5Au;
    success.loader->heap800965B0.back() = 0xA5u;
    assert(PrStage1LoaderMemoryDirectApply80025A70(
               *success.loader, 0x80)
               .success);
    success.loader->stackTable80091858[17] = 0xDEADBEEFu;
    const auto heapBefore = SnapshotBytes(success.loader->heap800965B0);
    const auto runtimeBefore = SnapshotBytes(*success.runtime);
    const auto graphBefore = SnapshotBytes(*success.graph);

    const auto applied =
        ApplyLoaderReset80025A34PreservingPacketArena801C4260(
            *success.runtime, *success.graph, *success.loader);
    assert(applied.applied);
    assert(applied.failure == LoaderResetFailure801C4260::None);
    assert(success.loader->gpPlus320LowWater == 0x801AE430u);
    assert(success.loader->gpPlus324StackDepth == 0u);
    assert(success.loader->gpPlus904HeapCursor == 0x800965B0u);
    for (uint32_t entry : success.loader->stackTable80091858) {
        assert(entry == 0u);
    }
    AssertBytesUnchanged(success.loader->heap800965B0, heapBefore);
    AssertBytesUnchanged(*success.runtime, runtimeBefore);
    AssertBytesUnchanged(*success.graph, graphBefore);

    const int32_t bytesToArena = static_cast<int32_t>(
        success.loader->gpPlus320LowWater -
        success.loader->gpPlus896HeapBase);
    assert(!PrStage1LoaderMemoryDirectApply80025A70(
                *success.loader, bytesToArena)
                .success);
    assert(!PrStage1LoaderMemoryDirectApply80025BBC(
                *success.loader, 0x801AE428u, 1, 8)
                .success);
    AssertBytesUnchanged(success.loader->heap800965B0, heapBefore);

    LoaderResetFixture badPacket;
    ++badPacket.runtime->packetAllocatorBases801C956C[0];
    AssertLoaderResetRejectedWithoutWrites(
        badPacket, LoaderResetFailure801C4260::PacketLaneMismatch);

    LoaderResetFixture badGraph;
    ++badGraph.graph->dword_8006ED50[1];
    AssertLoaderResetRejectedWithoutWrites(
        badGraph, LoaderResetFailure801C4260::GraphPacketLaneMismatch);

    LoaderResetFixture badLoader;
    badLoader.loader->gpPlus904HeapCursor = 0x801AE430u;
    AssertLoaderResetRejectedWithoutWrites(
        badLoader, LoaderResetFailure801C4260::LoaderHeapCursorMismatch);

    LoaderResetFixture bareReset;
    PrStage1LoaderMemoryDirectReset(*bareReset.loader);
    AssertLoaderResetRejectedWithoutWrites(
        bareReset, LoaderResetFailure801C4260::LoaderLowWaterMismatch);
}

void TestInactiveTitlePresentFrame801C4D58()
{
    using namespace PrSS0TitlePacketWorkDirect;

    LoaderResetFixture fixture;
    assert(fixture.runtime->titleRenderActive801C9548 == 0u);
    const auto frame = BeginFrame801C6410(*fixture.runtime, *fixture.graph);
    assert(frame.prepared);
    const auto inactiveFlags =
        ApplyTitleFrameFlags801C6410(*fixture.runtime, 0u);
    assert(inactiveFlags.applied);
    assert(inactiveFlags.titleRenderActiveBefore801C9548 == 0u);
    assert(inactiveFlags.titleRenderActiveAfter801C9548 == 0u);
    assert(fixture.runtime->framePrepared801C6410);
    assert(fixture.runtime->frameFlagsApplied801C6410);

    const auto present =
        ApplyPresentFrameModel801C689C(*fixture.runtime, *fixture.graph);
    assert(present.modelApplied);
    assert(present.failure == PresentFrameFailure801C689C::None);
    assert(!fixture.runtime->framePrepared801C6410);
}

void AssertPresentRejectedWithoutFlip(
    PresentFixture& fixture,
    PrSS0TitlePacketWorkDirect::PresentFrameFailure801C689C expectedFailure)
{
    const bool framePrepared = fixture.runtime->framePrepared801C6410;
    const uint8_t cachedLane = fixture.runtime->currentDrawBuffer8004019C;
    const uint32_t allocator =
        fixture.runtime->currentPacketAllocator800901C8;
    const uint16_t graphSlot = fixture.graph->word_80096590;
    const uint32_t frameCounter = fixture.graph->dword_8009658C;
    const int16_t displayX = fixture.graph->word_80091790;
    const int16_t displayY = fixture.graph->word_80091792;

    const auto rejected =
        PrSS0TitlePacketWorkDirect::ApplyPresentFrameModel801C689C(
            *fixture.runtime, *fixture.graph);
    assert(!rejected.modelApplied);
    assert(rejected.failure == expectedFailure);
    assert(fixture.runtime->framePrepared801C6410 == framePrepared);
    assert(fixture.runtime->currentDrawBuffer8004019C == cachedLane);
    assert(fixture.runtime->currentPacketAllocator800901C8 == allocator);
    assert(fixture.graph->word_80096590 == graphSlot);
    assert(fixture.graph->dword_8009658C == frameCounter);
    assert(fixture.graph->word_80091790 == displayX);
    assert(fixture.graph->word_80091792 == displayY);
}

} // namespace

int main()
{
    using namespace PrSS0TitlePacketWorkDirect;

    TestLoaderResetPreservingPacketArena801C4260();
    TestInactiveTitlePresentFrame801C4D58();

    auto loader = std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*loader);
    auto runtime = std::make_unique<RuntimeState801C609C>();
    auto graph = std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>();
    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
        *graph, 320u, 240u);
    const auto initialized =
        Initialize801C609C(*runtime, loader.get(), *graph);
    assert(initialized.initialized);
    assert(initialized.packetArenaBase80025B28 == 0x801AE430u);
    assert(initialized.packetArenaBytes == 0x15180u);
    assert(runtime->packetAllocatorBases801C956C[0] == 0x801AE430u);
    assert(runtime->packetAllocatorBases801C956C[1] == 0x801B8CF0u);
    assert(initialized.packetLaneBinding8001E33C.known);
    assert(initialized.packetLaneBinding8001E33C.sourceFunction ==
           0x801C609Cu);
    assert(initialized.packetLaneBinding8001E33C.calleeFunction ==
           0x8001E33Cu);
    assert(initialized.packetLaneBinding8001E33C.arg0Known);
    assert(initialized.packetLaneBinding8001E33C.arg0 == 0x801AE430u);
    assert(initialized.packetLaneBinding8001E33C.arg1Known);
    assert(initialized.packetLaneBinding8001E33C.arg1 == 0x801B8CF0u);
    assert(initialized.packetLaneBinding8001E33C.psxWouldWriteGpPlus310);
    assert(initialized.packetLaneBinding8001E33C.psxWouldWriteGpPlus314);
    assert(!initialized.packetLaneBinding8001E33C.hostSideEffects);
    assert(initialized.packetLaneBinding8001E33C.gpPlus310Offset == 0x310u);
    assert(initialized.packetLaneBinding8001E33C.gpPlus314Offset == 0x314u);
    assert(initialized.packetLaneBinding8001E33C.gpPlus310 == 0x801AE430u);
    assert(initialized.packetLaneBinding8001E33C.gpPlus314 == 0x801B8CF0u);
    assert(initialized.packetLaneBindingCallsite801C60C8 == 0x801C60C8u);
    assert(initialized.packetLaneGraphBindingApplied8001E33C);
    assert(runtime->packetLaneBinding8001E33C.gpPlus310 == 0x801AE430u);
    assert(runtime->packetLaneBinding8001E33C.gpPlus314 == 0x801B8CF0u);
    assert(runtime->packetLaneBindingCallsite801C60C8 == 0x801C60C8u);
    assert(graph->dword_8006ED50[0] == 0x801AE430u);
    assert(graph->dword_8006ED50[1] == 0x801B8CF0u);

    const auto transitionVisual = PrSS0TransitionDirect::
        ResolveSlowTransitionVisualFrame80020110(
            PrSS0TransitionDirect::kScene0WorkAddress,
            2,
            1,
            2,
            false,
            0u);
    assert(transitionVisual.known);
    for (uint16_t slot = 0u; slot < 2u; ++slot) {
        const auto& page = graph->mainPageWorkLists80087288[slot];
        PrSS0TransitionDirect::FastTransitionGraphInput8001EA74
            transitionGraph{};
        transitionGraph.known = true;
        transitionGraph.drawSlot8004019C = slot;
        transitionGraph.packetAllocator8006ED50 =
            graph->dword_8006ED50[slot];
        transitionGraph.mainPageWorkAddress80087288 = page.workAddr;
        transitionGraph.mainPageOtHeadAddress80088288 = page.otHeadAddr;
        transitionGraph.mainPageWorkHeadAddress80040CC8 =
            page.work.headAddr_04;
        transitionGraph.mainPageWorkOrder = page.work.order_00;
        const auto transitionPresent = PrSS0TransitionDirect::
            BuildSlowTransitionPresentPlan8001EBF4(
                transitionVisual, transitionGraph);
        assert(transitionPresent.known);
        assert(transitionPresent.drawSlotBefore == slot);
        assert(transitionPresent.drawSlotAfter == (slot ^ 1u));
        assert(transitionPresent.packetAllocator8006ED50 ==
               graph->dword_8006ED50[slot]);
        assert(transitionPresent.actions[1].kind ==
               PrSS0TransitionDirect::
                   FastTransitionPresentActionKind8001EBF4::
                       SetPacketAllocator80040F90);
        assert(transitionPresent.actions[1].arg0 ==
               graph->dword_8006ED50[slot]);
    }
    assert(runtime->workLists801C9574[0].order_00 == 10u);
    assert(runtime->workLists801C9574[0].headAddr_04 == 0x801C959Cu);
    assert(runtime->workLists801C9574[1].order_00 == 0u);
    assert(runtime->workLists801C9574[1].headAddr_04 == 0u);
    assert(runtime->titleRenderActive801C9548 == 0u);
    assert(runtime->loHpGate801CB600 == 0u);

    const auto frame0 = BeginFrame801C6410(*runtime, *graph);
    assert(frame0.prepared);
    assert(frame0.drawBuffer8004019C == 0u);
    assert(frame0.packetAllocator800901C8 == 0x801AE430u);
    assert(frame0.workAddress801C9574 == 0x801C9574u);
    assert(graph->dword_800901C8 == 0x801AE430u);
    assert(frame0.titleClear80040CC8.clearOtagRLength == 1024u);
    assert(frame0.mainPageClear8001E374.clearOtagRLength == 16384u);
    const auto* work0 = GetCurrentWorkList801C6410(*runtime);
    assert(work0 != nullptr);
    assert(work0->lastAddr_10 == 0x801CA598u);
    assert(work0->tmdOtSlotMirrorKnown);
    assert(work0->tmdOtSlotMirror[120].valid);
    assert(work0->tmdOtSlotMirror[120].addr == 0x801C977Cu);
    assert(work0->tmdOtSlotMirror[120].value == 0x001C9778u);

    const auto ungatedPresent =
        ApplyPresentFrameModel801C689C(*runtime, *graph);
    assert(!ungatedPresent.modelApplied);
    assert(ungatedPresent.failure ==
           PresentFrameFailure801C689C::FrameFlagsNotApplied);
    assert(runtime->framePrepared801C6410);
    assert(graph->word_80096590 == 0u);

    const auto activeFlags = ApplyTitleFrameFlags801C6410(
        *runtime, kCtxFlagTitleRenderActive801C6410);
    assert(activeFlags.applied);
    assert(activeFlags.failure == FrameFlagsFailure801C6410::None);
    assert(activeFlags.titleRenderActiveBefore801C9548 == 0u);
    assert(activeFlags.titleRenderActiveAfter801C9548 == 1u);
    assert(activeFlags.loHpGateBefore801CB600 == 0u);
    assert(activeFlags.loHpGateAfter801CB600 == 0u);
    assert(!activeFlags.loHpWriterFromPairSetup801C66B0);
    assert(runtime->frameFlagsApplied801C6410);
    assert(runtime->titleRenderActive801C9548 == 1u);

    const auto duplicateFlags = ApplyTitleFrameFlags801C6410(
        *runtime, kCtxFlagTitleRenderActive801C6410);
    assert(!duplicateFlags.applied);
    assert(duplicateFlags.failure ==
           FrameFlagsFailure801C6410::AlreadyApplied);

    const auto present0 = ApplyPresentFrameModel801C689C(*runtime, *graph);
    assert(present0.modelApplied);
    assert(present0.failure == PresentFrameFailure801C689C::None);
    assert(present0.cachedDrawLane8006EDA8 == 0u);
    assert(present0.titleWorkAddress801C9574 == 0x801C9574u);
    assert(present0.mainPageWorkAddress80087288 == 0x80087288u);
    assert(present0.clearR80040060 == 255u);
    assert(present0.clearG80040060 == 255u);
    assert(present0.clearB80040060 == 255u);
    assert(present0.flipGraphStateAdvanced80040370);
    assert(present0.whiteDrawEnv80040060Requested);
    assert(present0.titleWork80040CA4SubmitRequested);
    assert(present0.mainPageWork8001E3B0SubmitRequested);
    assert(present0.titleWorkSubmit80040CA4Executed);
    assert(present0.mainPageWorkSubmit8001E3B0Executed);
    assert(present0.titleWorkSubmitCallCount80040CA4 == 1u);
    assert(present0.mainPageWorkSubmitCallCount8001E3B0 == 1u);
    assert(runtime->titleSubmitCallCount80040CA4 == 1u);
    assert(runtime->mainSubmitCallCount8001E3B0 == 1u);
    assert(present0.drawEnv80040060.known);
    assert(present0.drawEnv80040060.graphSlot80096590 == 1u);
    assert(present0.drawEnv80040060.sourceWorkListAddress801C9574 ==
           0x801C9574u);
    assert(present0.drawEnv80040060.sourceOtagHeadAddress ==
           0x801CA598u);
    assert(present0.drawEnv80040060.commandAddress8008A730 ==
           0x8008A740u);
    assert(present0.drawEnv80040060.r == 255u);
    assert(present0.drawEnv80040060.g == 255u);
    assert(present0.drawEnv80040060.b == 255u);
    assert(present0.drawEnv80040060.clipWidth == 320u);
    assert(present0.drawEnv80040060.clipHeight == 240u);
    assert(present0.flip.previousSlot == 0u);
    assert(present0.flip.nextSlot == 1u);
    assert(!runtime->framePrepared801C6410);
    assert(runtime->currentDrawBuffer8004019C == 0u);

    const uint32_t counterAfterPresent0 = graph->dword_8009658C;
    const auto duplicatePresent =
        ApplyPresentFrameModel801C689C(*runtime, *graph);
    assert(!duplicatePresent.modelApplied);
    assert(duplicatePresent.failure ==
           PresentFrameFailure801C689C::RuntimeNotReady);
    assert(graph->word_80096590 == 1u);
    assert(graph->dword_8009658C == counterAfterPresent0);

    const auto frame1 = BeginFrame801C6410(*runtime, *graph);
    assert(frame1.prepared);
    assert(frame1.drawBuffer8004019C == 1u);
    assert(frame1.packetAllocator800901C8 == 0x801B8CF0u);
    assert(frame1.workAddress801C9574 == 0x801C9574u);
    assert(graph->dword_800901C8 == 0x801B8CF0u);
    const auto* work1 = GetCurrentWorkList801C6410(*runtime);
    assert(work1 != nullptr);
    assert(work1->lastAddr_10 == 0x801CA598u);

    const auto pairFlags = ApplyTitleFrameFlags801C6410(
        *runtime, 0x01400000u);
    assert(pairFlags.applied);
    assert(pairFlags.titleRenderActiveBefore801C9548 == 1u);
    assert(pairFlags.titleRenderActiveAfter801C9548 == 1u);
    assert(pairFlags.loHpGateBefore801CB600 == 0u);
    assert(pairFlags.loHpGateAfter801CB600 == 1u);
    assert(pairFlags.loHpWriterFromPairSetup801C66B0);
    assert(runtime->loHpGate801CB600 == 1u);

    const auto present1 = ApplyPresentFrameModel801C689C(*runtime, *graph);
    assert(present1.modelApplied);
    assert(present1.cachedDrawLane8006EDA8 == 1u);
    assert(present1.titleWorkAddress801C9574 == 0x801C9574u);
    assert(present1.mainPageWorkAddress80087288 == 0x80087288u);
    assert(present1.titleWorkSubmit80040CA4Executed);
    assert(present1.mainPageWorkSubmit8001E3B0Executed);
    assert(present1.titleWorkSubmitCallCount80040CA4 == 2u);
    assert(present1.mainPageWorkSubmitCallCount8001E3B0 == 2u);
    assert(present1.drawEnv80040060.known);
    assert(present1.drawEnv80040060.graphSlot80096590 == 0u);
    assert(present1.drawEnv80040060.commandAddress8008A730 ==
           0x8008A730u);
    assert(present1.flip.previousSlot == 1u);
    assert(present1.flip.nextSlot == 0u);
    assert(graph->word_80096590 == 0u);

    const auto frame2 = BeginFrame801C6410(*runtime, *graph);
    assert(frame2.prepared);
    const auto persistentActiveFlags =
        ApplyTitleFrameFlags801C6410(*runtime, 0u);
    assert(persistentActiveFlags.applied);
    assert(persistentActiveFlags.titleRenderActiveBefore801C9548 == 1u);
    assert(persistentActiveFlags.titleRenderActiveAfter801C9548 == 1u);
    assert(persistentActiveFlags.loHpGateBefore801CB600 == 1u);
    assert(persistentActiveFlags.loHpGateAfter801CB600 == 1u);
    assert(!persistentActiveFlags.loHpWriterFromPairSetup801C66B0);
    assert(ApplyPresentFrameModel801C689C(*runtime, *graph).modelApplied);

    const auto frame3 = BeginFrame801C6410(*runtime, *graph);
    assert(frame3.prepared);
    ResetTitleFrameGates801C609C(*runtime);
    assert(runtime->titleRenderActive801C9548 == 0u);
    assert(runtime->loHpGate801CB600 == 0u);
    assert(runtime->presentExtra801CFAEC == 0u);

    const auto shortcutFlags = ApplyTitleFrameFlags801C6410(
        *runtime, kCtxFlagTitleShortcutSetup801C6410);
    assert(!shortcutFlags.applied);
    assert(shortcutFlags.failure ==
           FrameFlagsFailure801C6410::ShortcutSetupUnsupported);
    assert(!runtime->frameFlagsApplied801C6410);
    assert(runtime->titleRenderActive801C9548 == 0u);
    assert(runtime->loHpGate801CB600 == 0u);
    assert(runtime->presentExtra801CFAEC == 0u);
    assert(!runtime->titleShortcutSetupApplied801C6410);
    assert(!runtime->titleShortcutReadyApplied801C6410);

    const auto immediateFlags = ApplyTitleFrameFlags801C6410(
        *runtime, kCtxFlagTitleImmediateSetup801C6410);
    assert(!immediateFlags.applied);
    assert(immediateFlags.failure ==
           FrameFlagsFailure801C6410::ImmediateSetupUnsupported);
    assert(runtime->titleRenderActive801C9548 == 0u);
    assert(runtime->loHpGate801CB600 == 0u);
    assert(runtime->presentExtra801CFAEC == 0u);

    assert(BeginFrame801C6410(*runtime, *graph).prepared);
    const auto naturalPairFlags = ApplyTitleFrameFlags801C6410(
        *runtime,
        kCtxFlagTitleRenderActive801C6410 |
            kCtxFlagLoPairWrite801C6410);
    assert(naturalPairFlags.applied);
    assert(naturalPairFlags.loHpWriterFromPairSetup801C66B0);
    assert(runtime->titleRenderActive801C9548 == 1u);
    assert(runtime->loHpGate801CB600 == 1u);
    assert(ApplyPresentFrameModel801C689C(*runtime, *graph).modelApplied);

    auto shortcutLoader = std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*shortcutLoader);
    auto shortcutRuntime = std::make_unique<RuntimeState801C609C>();
    auto shortcutGraph =
        std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>();
    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
        *shortcutGraph, 320u, 240u);
    assert(Initialize801C609C(
               *shortcutRuntime, shortcutLoader.get(), *shortcutGraph)
               .initialized);

    const auto readyBeforeSetup =
        ApplyEarlyInputShortcutReadyFrame801C6410(
            *shortcutRuntime, *shortcutGraph);
    assert(!readyBeforeSetup.committed);
    assert(readyBeforeSetup.failure ==
           EarlyInputShortcutFailure801C6410::SetupMissing);
    assert(!shortcutRuntime->framePrepared801C6410);
    assert(shortcutGraph->word_80096590 == 0u);

    const auto shortcutSetup =
        ApplyEarlyInputShortcutSetupFrame801C6410(
            *shortcutRuntime, *shortcutGraph);
    assert(shortcutSetup.committed);
    assert(shortcutSetup.flags.shortcutSetupWriter801C64E0);
    assert(!shortcutSetup.flags.shortcutReadyWriter801C663C);
    assert(shortcutRuntime->frameSetupSource801C6410 ==
           TitleFrameSetupSource801C6410::EarlyInputShortcut);
    assert(shortcutRuntime->presentExtra801CFAEC == 1u);
    assert(shortcutRuntime->titleRenderActive801C9548 == 0u);
    assert(shortcutRuntime->loHpGate801CB600 == 0u);
    assert(shortcutRuntime->titleShortcutSetupApplied801C6410);
    assert(!shortcutRuntime->titleShortcutReadyApplied801C6410);
    const uint32_t shortcutFrameCounterBeforePresent =
        shortcutGraph->dword_8009658C;
    const auto shortcutSetupPresent = ApplyPresentFrameModel801C689C(
        *shortcutRuntime, *shortcutGraph);
    assert(shortcutSetupPresent.modelApplied);
    assert(shortcutSetupPresent.presentExtraSource801CFAEC);
    assert(shortcutSetupPresent.extraFlipGraphStateAdvanced80040370);
    assert(shortcutSetupPresent.fullHeightWhiteFill8001B1B0Requested);
    assert(shortcutSetupPresent.fullHeightWhiteFillX8001B1B0 == 0);
    assert(shortcutSetupPresent.fullHeightWhiteFillY8001B1B0 == 0);
    assert(shortcutSetupPresent.fullHeightWhiteFillWidth8001B1B0 == 320);
    assert(shortcutSetupPresent.fullHeightWhiteFillHeight8001B1B0 == 480);
    assert(shortcutSetupPresent.flip.previousSlot == 0u);
    assert(shortcutSetupPresent.flip.nextSlot == 1u);
    assert(shortcutSetupPresent.extraFlip.previousSlot == 1u);
    assert(shortcutSetupPresent.extraFlip.nextSlot == 0u);
    assert(shortcutGraph->word_80096590 == 0u);
    assert(shortcutGraph->dword_8009658C ==
           shortcutFrameCounterBeforePresent + 2u);

    const auto duplicateSetup =
        ApplyEarlyInputShortcutSetupFrame801C6410(
            *shortcutRuntime, *shortcutGraph);
    assert(!duplicateSetup.committed);
    assert(duplicateSetup.failure ==
           EarlyInputShortcutFailure801C6410::SetupAlreadyApplied);

    const auto shortcutReady =
        ApplyEarlyInputShortcutReadyFrame801C6410(
            *shortcutRuntime, *shortcutGraph);
    assert(shortcutReady.committed);
    assert(!shortcutReady.flags.shortcutSetupWriter801C64E0);
    assert(shortcutReady.flags.shortcutReadyWriter801C663C);
    assert(shortcutRuntime->frameSetupSource801C6410 ==
           TitleFrameSetupSource801C6410::EarlyInputShortcut);
    assert(shortcutRuntime->presentExtra801CFAEC == 0u);
    assert(shortcutRuntime->titleRenderActive801C9548 == 1u);
    assert(shortcutRuntime->loHpGate801CB600 == 1u);
    assert(shortcutRuntime->titleShortcutReadyApplied801C6410);

    const auto duplicateReady =
        ApplyEarlyInputShortcutReadyFrame801C6410(
            *shortcutRuntime, *shortcutGraph);
    assert(!duplicateReady.committed);
    assert(duplicateReady.failure ==
           EarlyInputShortcutFailure801C6410::ReadyAlreadyApplied);

    PresentFixture staleSlot;
    staleSlot.graph->word_80096590 = 1u;
    AssertPresentRejectedWithoutFlip(
        staleSlot, PresentFrameFailure801C689C::GraphSlotMismatch);

    PresentFixture allocatorMismatch;
    ++allocatorMismatch.graph->dword_800901C8;
    AssertPresentRejectedWithoutFlip(
        allocatorMismatch,
        PresentFrameFailure801C689C::AllocatorMismatch);

    PresentFixture badTitleWork;
    badTitleWork.runtime->workLists801C9574[0].order_00 = 9u;
    AssertPresentRejectedWithoutFlip(
        badTitleWork,
        PresentFrameFailure801C689C::TitleWorkMismatch);
    assert(badTitleWork.runtime->workLists801C9574[0].order_00 == 9u);

    PresentFixture badMainWork;
    ++badMainWork.graph->mainPageWorkLists80087288[0].work.headAddr_04;
    AssertPresentRejectedWithoutFlip(
        badMainWork,
        PresentFrameFailure801C689C::MainPageWorkMismatch);
    assert(badMainWork.graph->mainPageWorkLists80087288[0]
               .work.headAddr_04 == 0x80088289u);

    auto badBindingLoader =
        std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*badBindingLoader);
    auto badBindingRuntime = std::make_unique<RuntimeState801C609C>();
    auto badBindingGraph =
        std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>();
    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
        *badBindingGraph, 320u, 240u);
    assert(Initialize801C609C(
               *badBindingRuntime, badBindingLoader.get(), *badBindingGraph)
               .initialized);
    const uint16_t badBindingSlotBefore = badBindingGraph->word_80096590;
    const uint32_t badBindingAllocatorBefore =
        badBindingGraph->dword_800901C8;
    const uint32_t badBindingWorkTailBefore =
        badBindingRuntime->workLists801C9574[0].lastAddr_10;
    ++badBindingRuntime->packetLaneBinding8001E33C.gpPlus310;
    const auto badBindingFrame =
        BeginFrame801C6410(*badBindingRuntime, *badBindingGraph);
    assert(!badBindingFrame.prepared);
    assert(!badBindingRuntime->framePrepared801C6410);
    assert(badBindingGraph->word_80096590 == badBindingSlotBefore);
    assert(badBindingGraph->dword_800901C8 == badBindingAllocatorBefore);
    assert(badBindingRuntime->workLists801C9574[0].lastAddr_10 ==
           badBindingWorkTailBefore);
    assert(!badBindingRuntime->workLists801C9574[0].clearOtagRCalled);
    --badBindingRuntime->packetLaneBinding8001E33C.gpPlus310;
    ++badBindingGraph->dword_8006ED50[1];
    const auto badGraphBindingFrame =
        BeginFrame801C6410(*badBindingRuntime, *badBindingGraph);
    assert(!badGraphBindingFrame.prepared);
    assert(badBindingGraph->dword_800901C8 == badBindingAllocatorBefore);
    assert(!badBindingRuntime->workLists801C9574[0].clearOtagRCalled);

    auto blockedLoader =
        std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*blockedLoader);
    blockedLoader->gpPlus904HeapCursor =
        blockedLoader->gpPlus900HeapEnd - 1u;
    auto blockedRuntime = std::make_unique<RuntimeState801C609C>();
    auto blockedGraph =
        std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>();
    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
        *blockedGraph, 320u, 240u);
    const auto blockedInitialize = Initialize801C609C(
        *blockedRuntime, blockedLoader.get(), *blockedGraph);
    assert(!blockedInitialize.initialized);
    assert(!blockedInitialize.packetLaneGraphBindingApplied8001E33C);
    assert(!blockedRuntime->initialized);
    assert(blockedGraph->dword_8006ED50[0] == 0x801A73B0u);
    assert(blockedGraph->dword_8006ED50[1] == 0x801B54B0u);

    auto graphMissingLoader =
        std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*graphMissingLoader);
    const uint32_t graphMissingCursorBefore =
        graphMissingLoader->gpPlus904HeapCursor;
    auto graphMissingRuntime = std::make_unique<RuntimeState801C609C>();
    auto graphMissing =
        std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>();
    assert(!Initialize801C609C(
                *graphMissingRuntime,
                graphMissingLoader.get(),
                *graphMissing)
                .initialized);
    assert(graphMissingLoader->gpPlus904HeapCursor ==
           graphMissingCursorBefore);
    assert(!graphMissingRuntime->initialized);

    Clear(*runtime);
    assert(!runtime->initialized);
    assert(!runtime->packetLaneBinding8001E33C.known);
    assert(runtime->packetLaneBindingCallsite801C60C8 == 0u);
    assert(runtime->packetLaneBinding8001E33C.gpPlus310 == 0u);
    assert(runtime->packetLaneBinding8001E33C.gpPlus314 == 0u);
    assert(GetCurrentWorkList801C6410(*runtime) == nullptr);
    std::printf("test_ss0_title_packet_work_direct: ok\n");
    return 0;
}
