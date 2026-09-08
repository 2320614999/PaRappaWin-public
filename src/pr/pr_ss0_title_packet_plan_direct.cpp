#include "pr_ss0_title_packet_plan_direct.h"

#include <limits>

namespace PrSS0TitlePacketPlanDirect {
namespace {

BuildResult800428B0 Fail(Failure failure)
{
    BuildResult800428B0 out{};
    out.failure = failure;
    return out;
}

} // namespace

BuildResult800428B0 BuildMode25PacketPlan800428B0(
    const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C& drawDesc,
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrPsxGteDirect::Mode25TriangleGeometryTrace& geometry,
    std::size_t primitiveIndex)
{
    if (!PrSS0TitleDrawDescDirect::IsMode25PrimitiveReady800428B0(
            drawDesc, drawDesc.object, primitiveIndex)) {
        return Fail(Failure::DrawDescriptorNotReady);
    }
    if (!geometry.projectedSxyKnown || !geometry.visibilityKnown ||
        !geometry.otzKnown) {
        return Fail(Failure::GeometryNotReady);
    }
    if (!packetWork.initialized || !packetWork.framePrepared801C6410 ||
        packetWork.currentDrawBuffer8004019C >=
            packetWork.workLists801C9574.size() ||
        packetWork.currentPacketAllocator800901C8 == 0u) {
        return Fail(Failure::PacketFrameNotReady);
    }
    if (graph.dword_800901C8 !=
        packetWork.currentPacketAllocator800901C8) {
        return Fail(Failure::GraphAllocatorMismatch);
    }

    const uint8_t titleWorkLane =
        PrSS0TitlePacketWorkDirect::kTitleWorkFixedLane801C609C;
    if (titleWorkLane >= packetWork.workLists801C9574.size()) {
        return Fail(Failure::PacketFrameNotReady);
    }
    const auto& work = packetWork.workLists801C9574[titleWorkLane];
    if (work.order_00 != PrSS0TitlePacketWorkDirect::kWorkOrder801C609C ||
        work.order_00 >= 31u || work.x_08 != 0u || work.y_0C != 0u ||
        !work.tmdOtSlotMirrorKnown) {
        return Fail(Failure::OrderingTableShapeMismatch);
    }
    const uint32_t length = 1u << work.order_00;
    const uint16_t priority = static_cast<uint16_t>(
        geometry.otz >> PrPsxTmdSubmitDirect::kRequiredOtShift);
    if (priority >= length ||
        priority >= work.tmdOtSlotMirror.size()) {
        return Fail(Failure::OrderingTableSlotUnknown);
    }
    const auto& slot = work.tmdOtSlotMirror[priority];
    const uint64_t tableBytes = static_cast<uint64_t>(length) * 4u;
    const uint64_t tableEnd64 =
        static_cast<uint64_t>(work.headAddr_04) + tableBytes;
    const uint64_t expectedSlot64 =
        static_cast<uint64_t>(work.headAddr_04) +
        static_cast<uint64_t>(priority) * 4u;
    if (tableEnd64 > std::numeric_limits<uint32_t>::max() ||
        expectedSlot64 > std::numeric_limits<uint32_t>::max() ||
        !slot.valid || slot.addr != static_cast<uint32_t>(expectedSlot64)) {
        return Fail(Failure::OrderingTableSlotUnknown);
    }

    const auto attr = PrSS0TitleDrawDescDirect::DecodeAttr800428B0(drawDesc);
    PrPsxTmdSubmitDirect::Input input{};
    input.object = drawDesc.object;
    input.primitiveIndex = primitiveIndex;
    input.sourceParseComplete = true;
    for (std::size_t index = 0; index < input.projectedSxy.size(); ++index) {
        input.projectedSxy[index].known = geometry.sxy[index].known;
        input.projectedSxy[index].value = geometry.sxy[index].word;
    }
    input.visibilityKnown = geometry.visibilityKnown;
    input.visible = geometry.visible;
    input.otzKnown = geometry.otzKnown;
    input.otz = geometry.otz;
    input.otShiftKnown = true;
    input.otShift = PrPsxTmdSubmitDirect::kRequiredOtShift;
    input.highPriorityKnown = attr.known;
    input.highPriority = attr.bit30;
    input.packetAllocatorKnown = true;
    input.packetAllocatorAddr = packetWork.currentPacketAllocator800901C8;
    input.orderingTable.headKnown = true;
    input.orderingTable.head = static_cast<uint32_t>(tableEnd64);
    input.orderingTable.lengthKnown = true;
    input.orderingTable.length = length;
    input.orderingTable.slotOldValueKnown = true;
    input.orderingTable.slotOldValue = slot.value;

    const auto plan = PrPsxTmdSubmitDirect::Build(input);
    if (!plan.readyToCommit ||
        plan.otDelta.slotAddress != slot.addr ||
        plan.otDelta.oldValue != slot.value ||
        plan.allocatorDelta.oldAddress !=
            packetWork.currentPacketAllocator800901C8) {
        BuildResult800428B0 out = Fail(Failure::SubmitRejected);
        out.submitFailure = plan.failure;
        return out;
    }

    BuildResult800428B0 out{};
    out.priority = priority;
    out.orderingTableEnd = static_cast<uint32_t>(tableEnd64);
    out.plan = plan;
    out.ready = true;
    return out;
}

BuildResult800428B0 BuildFirstPaKageMode25PacketPlan800428B0(
    const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C& drawDesc,
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrPsxGteDirect::Mode25TriangleGeometryTrace& geometry,
    std::size_t primitiveIndex)
{
    return BuildMode25PacketPlan800428B0(
        drawDesc, packetWork, graph, geometry, primitiveIndex);
}

} // namespace PrSS0TitlePacketPlanDirect
