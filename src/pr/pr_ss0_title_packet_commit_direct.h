#pragma once

#include "pr_ss0_title_packet_plan_direct.h"
#include "pr_ss0_title_packet_work_direct.h"

#include <cstdint>

namespace PrSS0TitlePacketCommitDirect {

enum class Failure : uint8_t {
    None = 0,
    PlanNotReady,
    PlanInconsistent,
    RuntimeNotReady,
    GraphNotReady,
    DrawBufferMismatch,
    WorkShapeMismatch,
    AllocatorMismatch,
    PacketArenaRangeMismatch,
    OrderingTableMirrorUnknown,
    OrderingTableSlotNotFound,
    OrderingTableSlotMismatch,
    PacketMirrorUnknown,
    PacketAddressAlreadyWritten,
    PacketCapacityExceeded,
};

struct CommitResult800428B0 {
    bool committed = false;
    Failure failure = Failure::None;
    uint8_t drawBuffer = 0;
    uint32_t allocatorBefore = 0;
    uint32_t allocatorAfter = 0;
    uint32_t orderingTableSlotAddress = 0;
    uint32_t packetAddress = 0;
};

CommitResult800428B0 CommitMode25PacketPlan800428B0(
    PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrSS0TitlePacketPlanDirect::BuildResult800428B0& built);

CommitResult800428B0 CommitFirstPaKageMode25PacketPlan800428B0(
    PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrSS0TitlePacketPlanDirect::BuildResult800428B0& built);

} // namespace PrSS0TitlePacketCommitDirect
