#include "pr_ss0_title_packet_commit_direct.h"

#include <limits>

namespace PrSS0TitlePacketCommitDirect {
namespace {

CommitResult800428B0 Fail(Failure failure,
                          uint8_t drawBuffer,
                          uint32_t allocator,
                          const PrPsxTmdSubmitDirect::Result& plan)
{
    CommitResult800428B0 out{};
    out.failure = failure;
    out.drawBuffer = drawBuffer;
    out.allocatorBefore = allocator;
    out.allocatorAfter = allocator;
    out.orderingTableSlotAddress = plan.otDelta.slotAddress;
    out.packetAddress = plan.packetWrite.address;
    return out;
}

bool PlanInternallyConsistent(const PrPsxTmdSubmitDirect::Result& plan)
{
    if (!plan.readyToCommit || !plan.packetWrite.marked ||
        !plan.otDelta.marked || !plan.allocatorDelta.marked) {
        return false;
    }
    if ((plan.packetWrite.address & 3u) != 0u ||
        (plan.otDelta.slotAddress & 3u) != 0u ||
        (plan.allocatorDelta.oldAddress & 3u) != 0u ||
        (plan.allocatorDelta.newAddress & 3u) != 0u) {
        return false;
    }
    if (plan.packetWrite.address != plan.allocatorDelta.oldAddress ||
        plan.allocatorDelta.advanceBytes !=
            PrPsxTmdSubmitDirect::kPacketByteSize ||
        plan.allocatorDelta.newAddress < plan.allocatorDelta.oldAddress ||
        plan.allocatorDelta.newAddress !=
            plan.allocatorDelta.oldAddress +
                PrPsxTmdSubmitDirect::kPacketByteSize ||
        plan.otDelta.newValue !=
            (plan.packetWrite.address &
             PrPsxTmdSubmitDirect::kPacketAddressMask)) {
        return false;
    }
    const uint32_t expectedLink =
        (7u << 24u) |
        (plan.otDelta.oldValue &
         PrPsxTmdSubmitDirect::kPacketAddressMask);
    return plan.packetWrite.words[0] == expectedLink;
}

} // namespace

CommitResult800428B0 CommitMode25PacketPlan800428B0(
    PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrSS0TitlePacketPlanDirect::BuildResult800428B0& built)
{
    const auto& plan = built.plan;
    const uint8_t drawBuffer = packetWork.currentDrawBuffer8004019C;
    const uint32_t allocator = packetWork.currentPacketAllocator800901C8;
    if (!built.ready || !plan.readyToCommit || !plan.packetWrite.marked ||
        !plan.otDelta.marked || !plan.allocatorDelta.marked) {
        return Fail(Failure::PlanNotReady, drawBuffer, allocator, plan);
    }
    if (!PlanInternallyConsistent(plan)) {
        return Fail(Failure::PlanInconsistent, drawBuffer, allocator, plan);
    }
    const uint8_t titleWorkLane =
        PrSS0TitlePacketWorkDirect::kTitleWorkFixedLane801C609C;
    if (!packetWork.initialized || !packetWork.framePrepared801C6410 ||
        drawBuffer >= packetWork.packetAllocatorBases801C956C.size() ||
        titleWorkLane >= packetWork.workLists801C9574.size() ||
        packetWork.packetArenaBase80025B28 == 0u) {
        return Fail(Failure::RuntimeNotReady, drawBuffer, allocator, plan);
    }
    if (!graph.mainPageWorkLists80087288Initialized) {
        return Fail(Failure::GraphNotReady, drawBuffer, allocator, plan);
    }
    if (static_cast<uint8_t>(graph.word_80096590 & 1u) != drawBuffer) {
        return Fail(Failure::DrawBufferMismatch, drawBuffer, allocator, plan);
    }

    const uint64_t arenaBase = packetWork.packetArenaBase80025B28;
    const uint64_t laneBytes =
        PrSS0TitlePacketWorkDirect::kPacketArenaLaneBytes801C5D28;
    const uint64_t arenaEnd = arenaBase + 2u * laneBytes;
    if (arenaEnd > std::numeric_limits<uint32_t>::max() ||
        packetWork.packetAllocatorBases801C956C[0] != arenaBase ||
        packetWork.packetAllocatorBases801C956C[1] != arenaBase + laneBytes) {
        return Fail(
            Failure::PacketArenaRangeMismatch, drawBuffer, allocator, plan);
    }

    auto& work = packetWork.workLists801C9574[titleWorkLane];
    const uint64_t expectedHead =
        static_cast<uint64_t>(
            PrSS0TitlePacketWorkDirect::kWork0OtHead801C959C);
    const uint32_t length =
        1u << PrSS0TitlePacketWorkDirect::kWorkOrder801C609C;
    const uint64_t expectedOtEnd = expectedHead + length * 4u;
    const uint64_t expectedOtTail = expectedOtEnd - 4u;
    const uint64_t expectedOtSlot =
        expectedHead + static_cast<uint64_t>(built.priority) * 4u;
    if (work.order_00 !=
            PrSS0TitlePacketWorkDirect::kWorkOrder801C609C ||
        work.headAddr_04 != expectedHead || work.x_08 != 0u ||
        work.y_0C != 0u || work.lastAddr_10 != expectedOtTail ||
        built.priority >= length ||
        built.priority >= work.tmdOtSlotMirror.size() ||
        built.orderingTableEnd != expectedOtEnd ||
        plan.otDelta.slotAddress != expectedOtSlot) {
        return Fail(Failure::WorkShapeMismatch, drawBuffer, allocator, plan);
    }

    const uint64_t laneBase =
        packetWork.packetAllocatorBases801C956C[drawBuffer];
    const uint64_t laneEnd = laneBase + laneBytes;
    if (allocator < laneBase || allocator > laneEnd ||
        plan.allocatorDelta.newAddress > laneEnd) {
        return Fail(
            Failure::PacketArenaRangeMismatch, drawBuffer, allocator, plan);
    }
    if (graph.dword_800901C8 != allocator ||
        plan.allocatorDelta.oldAddress != allocator) {
        return Fail(Failure::AllocatorMismatch, drawBuffer, allocator, plan);
    }
    if (!work.tmdOtSlotMirrorKnown) {
        return Fail(
            Failure::OrderingTableMirrorUnknown, drawBuffer, allocator, plan);
    }

    auto& targetOtSlot = work.tmdOtSlotMirror[built.priority];
    if (!targetOtSlot.valid ||
        targetOtSlot.addr != plan.otDelta.slotAddress) {
        return Fail(
            Failure::OrderingTableSlotNotFound, drawBuffer, allocator, plan);
    }
    if (targetOtSlot.value != plan.otDelta.oldValue) {
        return Fail(
            Failure::OrderingTableSlotMismatch, drawBuffer, allocator, plan);
    }
    if (!work.tmdPacketWriteMirrorKnown) {
        return Fail(Failure::PacketMirrorUnknown, drawBuffer, allocator, plan);
    }

    PrPsxGraphOwnerDirect::PsxTmdPacketWrite* freePacketSlot = nullptr;
    for (auto& write : work.tmdPacketWriteMirror) {
        if (write.valid && write.addr == plan.packetWrite.address) {
            return Fail(Failure::PacketAddressAlreadyWritten,
                        drawBuffer,
                        allocator,
                        plan);
        }
        if (!write.valid && freePacketSlot == nullptr) {
            freePacketSlot = &write;
        }
    }
    if (freePacketSlot == nullptr) {
        return Fail(
            Failure::PacketCapacityExceeded, drawBuffer, allocator, plan);
    }

    freePacketSlot->valid = true;
    freePacketSlot->addr = plan.packetWrite.address;
    freePacketSlot->words = plan.packetWrite.words;
    targetOtSlot.value = plan.otDelta.newValue;
    graph.dword_800901C8 = plan.allocatorDelta.newAddress;
    packetWork.currentPacketAllocator800901C8 =
        plan.allocatorDelta.newAddress;

    CommitResult800428B0 out{};
    out.committed = true;
    out.drawBuffer = drawBuffer;
    out.allocatorBefore = allocator;
    out.allocatorAfter = plan.allocatorDelta.newAddress;
    out.orderingTableSlotAddress = plan.otDelta.slotAddress;
    out.packetAddress = plan.packetWrite.address;
    return out;
}

CommitResult800428B0 CommitFirstPaKageMode25PacketPlan800428B0(
    PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrSS0TitlePacketPlanDirect::BuildResult800428B0& built)
{
    return CommitMode25PacketPlan800428B0(packetWork, graph, built);
}

} // namespace PrSS0TitlePacketCommitDirect
