#pragma once

#include "pr_psx_graph_owner_direct.h"
#include "pr_psx_gte_direct.h"
#include "pr_psx_tmd_submit_direct.h"
#include "pr_ss0_title_draw_desc_direct.h"
#include "pr_ss0_title_packet_work_direct.h"

#include <cstddef>
#include <cstdint>

namespace PrSS0TitlePacketPlanDirect {

enum class Failure : uint8_t {
    None = 0,
    DrawDescriptorNotReady,
    GeometryNotReady,
    PacketFrameNotReady,
    GraphAllocatorMismatch,
    OrderingTableShapeMismatch,
    OrderingTableSlotUnknown,
    SubmitRejected,
};

struct BuildResult800428B0 {
    Failure failure = Failure::None;
    PrPsxTmdSubmitDirect::Failure submitFailure =
        PrPsxTmdSubmitDirect::Failure::None;
    uint16_t priority = 0;
    uint32_t orderingTableEnd = 0;
    PrPsxTmdSubmitDirect::Result plan{};
    bool ready = false;
};

BuildResult800428B0 BuildMode25PacketPlan800428B0(
    const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C& drawDesc,
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrPsxGteDirect::Mode25TriangleGeometryTrace& geometry,
    std::size_t primitiveIndex);

BuildResult800428B0 BuildFirstPaKageMode25PacketPlan800428B0(
    const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C& drawDesc,
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    const PrPsxGraphOwnerDirect::PsxGraphState& graph,
    const PrPsxGteDirect::Mode25TriangleGeometryTrace& geometry,
    std::size_t primitiveIndex);

} // namespace PrSS0TitlePacketPlanDirect
