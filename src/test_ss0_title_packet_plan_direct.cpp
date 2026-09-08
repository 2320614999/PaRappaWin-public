#include "pr/pr_ss0_title_packet_plan_direct.h"

#include <cassert>
#include <cstdio>

namespace {

TmdModel MakeExactMode25Model()
{
    using namespace PrSS0TitleDrawDescDirect;
    TmdModel model{};
    model.id = 0x41u;
    model.objects.resize(1u);
    TmdObject& object = model.objects[0];
    object.vertices.resize(kMode25VertexCount);
    object.rawPrimitivePackets.resize(kMode25PrimitiveCount);
    object.primitives.resize(kMode25PrimitiveCount);
    for (uint32_t index = 0; index < kMode25PrimitiveCount; ++index) {
        const uint32_t offset =
            kMode25FirstPrimitiveOffset + index * kMode25PacketBytes;
        auto& raw = object.rawPrimitivePackets[index];
        raw.rawPrimitiveIndex = index;
        raw.rawPacketOffset = offset;
        raw.rawPacketByteSize = kMode25PacketBytes;
        raw.olen = kMode25Olen;
        raw.ilen = kMode25Ilen;
        raw.flag = kMode25Flag;
        raw.mode = kMode25Mode;
        raw.parsed = true;
        raw.parsedPrimitiveIndex = index;
        auto& primitive = object.primitives[index];
        primitive.rawPacketKnown = true;
        primitive.rawPrimitiveIndex = index;
        primitive.rawPacketOffset = offset;
        primitive.rawPacketByteSize = kMode25PacketBytes;
        primitive.olen = kMode25Olen;
        primitive.ilen = kMode25Ilen;
        primitive.flag = kMode25Flag;
        primitive.mode = kMode25Mode;
        primitive.textured = true;
    }
    TmdPrimitive& first = object.primitives[0];
    first.r = 0x80u;
    first.g = 0x80u;
    first.b = 0x80u;
    first.u0 = 0x8Cu;
    first.v0 = 0x00u;
    first.clut = 0x48B5u;
    first.u1 = 0xB7u;
    first.v1 = 0x36u;
    first.tpage = 0x0009u;
    first.u2 = 0x8Cu;
    first.v2 = 0x36u;
    return model;
}

PrPsxGteDirect::Mode25TriangleGeometryTrace MakeGeometry()
{
    PrPsxGteDirect::Mode25TriangleGeometryTrace geometry{};
    geometry.sxy[0].known = true;
    geometry.sxy[0].word = 0x00070034u;
    geometry.sxy[1].known = true;
    geometry.sxy[1].word = 0x0018002Au;
    geometry.sxy[2].known = true;
    geometry.sxy[2].word = 0x00130029u;
    geometry.projectedSxyKnown = true;
    geometry.visibilityKnown = true;
    geometry.visible = true;
    geometry.otzKnown = true;
    geometry.otz = 1921u;
    return geometry;
}

} // namespace

int main()
{
    using namespace PrSS0TitlePacketPlanDirect;

    TmdModel paModel = MakeExactMode25Model();
    PrSS0TitleDrawDescDirect::RuntimeState8001AF1C paDrawDesc{};
    assert(PrSS0TitleDrawDescDirect::InitializePa8001AF1C(
               paDrawDesc, &paModel, true)
               .initialized);

    PrSS0TitlePacketWorkDirect::RuntimeState801C609C packetWork{};
    packetWork.initialized = true;
    packetWork.framePrepared801C6410 = true;
    packetWork.currentDrawBuffer8004019C = 0u;
    packetWork.currentPacketAllocator800901C8 = 0x801AE430u;
    auto& work = packetWork.workLists801C9574[0];
    work.order_00 = 10u;
    work.headAddr_04 = 0x801C959Cu;
    work.tmdOtSlotMirrorKnown = true;
    work.tmdOtSlotMirror[120].valid = true;
    work.tmdOtSlotMirror[120].addr = 0x801C977Cu;
    work.tmdOtSlotMirror[120].value = 0x001C9778u;

    PrPsxGraphOwnerDirect::PsxGraphState graph{};
    graph.dword_800901C8 = packetWork.currentPacketAllocator800901C8;
    const auto geometry = MakeGeometry();
    const auto result = BuildMode25PacketPlan800428B0(
        paDrawDesc, packetWork, graph, geometry, 0u);
    assert(result.ready && result.failure == Failure::None);
    assert(result.priority == 120u);
    assert(result.orderingTableEnd == 0x801CA59Cu);
    assert(result.plan.readyToCommit);
    assert(result.plan.packetWrite.address == 0x801AE430u);
    assert(result.plan.packetWrite.words[0] == 0x071C9778u);
    assert(result.plan.packetWrite.words[1] == 0x24808080u);
    assert(result.plan.packetWrite.words[2] == 0x00070034u);
    assert(result.plan.packetWrite.words[3] == 0x48B5008Cu);
    assert(result.plan.packetWrite.words[4] == 0x0018002Au);
    assert(result.plan.packetWrite.words[5] == 0x000936B7u);
    assert(result.plan.packetWrite.words[6] == 0x00130029u);
    assert(result.plan.packetWrite.words[7] == 0x0000368Cu);
    assert(result.plan.otDelta.slotAddress == 0x801C977Cu);
    assert(result.plan.allocatorDelta.newAddress == 0x801AE450u);

    TmdModel paKageModel = MakeExactMode25Model();
    PrSS0TitleDrawDescDirect::RuntimeState8001AF1C paKageDrawDesc{};
    assert(PrSS0TitleDrawDescDirect::InitializePaKage8001AF1C(
               paKageDrawDesc, &paKageModel, true)
               .initialized);
    const auto legacyPaKage =
        BuildFirstPaKageMode25PacketPlan800428B0(
            paKageDrawDesc, packetWork, graph, geometry, 0u);
    assert(legacyPaKage.ready);
    assert(legacyPaKage.failure == Failure::None);
    assert(legacyPaKage.plan.packetWrite.words ==
           result.plan.packetWrite.words);

    auto wrongProvenance = paDrawDesc;
    wrongProvenance.modelPath =
        PrSS0TitleDrawDescDirect::ModelPath::PaKage;
    const auto provenanceRejected = BuildMode25PacketPlan800428B0(
        wrongProvenance, packetWork, graph, geometry, 0u);
    assert(!provenanceRejected.ready);
    assert(provenanceRejected.failure == Failure::DrawDescriptorNotReady);

    graph.dword_800901C8 = 0x801AE450u;
    const auto staleAllocator = BuildMode25PacketPlan800428B0(
        paDrawDesc, packetWork, graph, geometry, 0u);
    assert(!staleAllocator.ready);
    assert(staleAllocator.failure == Failure::GraphAllocatorMismatch);
    assert(!staleAllocator.plan.readyToCommit);

    graph.dword_800901C8 = packetWork.currentPacketAllocator800901C8;
    work.tmdOtSlotMirror[120].valid = false;
    const auto missingSlot = BuildMode25PacketPlan800428B0(
        paDrawDesc, packetWork, graph, geometry, 0u);
    assert(!missingSlot.ready);
    assert(missingSlot.failure == Failure::OrderingTableSlotUnknown);
    assert(!missingSlot.plan.readyToCommit);

    work.tmdOtSlotMirror[120].valid = true;
    paDrawDesc.attr = 1u << 30u;
    const auto unsupportedBit30 = BuildMode25PacketPlan800428B0(
        paDrawDesc, packetWork, graph, geometry, 0u);
    assert(!unsupportedBit30.ready);
    assert(unsupportedBit30.failure == Failure::SubmitRejected);
    assert(unsupportedBit30.submitFailure ==
           PrPsxTmdSubmitDirect::Failure::UnsupportedHighPriority);
    assert(!unsupportedBit30.plan.readyToCommit);

    std::printf("test_ss0_title_packet_plan_direct: ok\n");
    return 0;
}
