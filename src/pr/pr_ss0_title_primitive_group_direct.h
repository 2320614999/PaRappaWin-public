#pragma once

#include "pr_ss0_title_packet_commit_direct.h"
#include "pr_ss0_title_transform_direct.h"

#include <cstddef>
#include <cstdint>

namespace PrSS0TitlePrimitiveGroupDirect {

enum class Failure : uint8_t {
    None = 0,
    UnsupportedModelPath,
    DrawDescriptorNotReady,
    PrimitiveRangeInvalid,
    RuntimeMatrixUnknown,
    PacketRuntimeNotReady,
    PrimitiveProvenanceMismatch,
    BaseCarrierRejected,
    MimeDeformationRejected,
    Rtpt3Rejected,
    OrderingTableSlotUnknown,
    PacketArenaCapacityExceeded,
    PacketMirrorCapacityExceeded,
    PacketAddressConflict,
    PacketPlanRejected,
    PacketCommitRejected,
    AllocatorAdvanceMismatch,
};

struct ExecuteResult8004274C {
    bool complete = false;
    Failure failure = Failure::None;
    uint16_t firstPrimitiveIndex = 0;
    uint16_t requestedCount = 0;
    uint16_t visitedCount = 0;
    uint16_t preparedCount = 0;
    uint16_t culledCount = 0;
    uint16_t committedCount = 0;
    uint16_t failedPrimitiveIndex = 0xFFFFu;
    uint32_t allocatorBefore = 0;
    uint32_t allocatorAfter = 0;
    bool partialWrites = false;
    PrSS0TitleTransformDirect::Failure transformFailure =
        PrSS0TitleTransformDirect::Failure::None;
    PrSS0TitlePacketPlanDirect::Failure packetPlanFailure =
        PrSS0TitlePacketPlanDirect::Failure::None;
    PrSS0TitlePacketCommitDirect::Failure packetCommitFailure =
        PrSS0TitlePacketCommitDirect::Failure::None;
    bool firstExecutionKnown = false;
    PrSS0TitleTransformDirect::Rtpt3ExecutionResult800428B0 firstExecution{};
    bool firstPlanKnown = false;
    PrSS0TitlePacketPlanDirect::BuildResult800428B0 firstPlan{};
    bool firstCommitKnown = false;
    PrSS0TitlePacketCommitDirect::CommitResult800428B0 firstCommit{};
};

ExecuteResult8004274C ExecuteMode25PrimitiveRange8004274C(
    const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C& drawDesc,
    PrSS0TitleTransformDirect::ModelPath path,
    const PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8& mime,
    const PrPsxGteDirect::Matrix3x4* runtimeMatrix,
    bool runtimeMatrixKnown,
    PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    std::size_t firstPrimitiveIndex,
    std::size_t primitiveCount);

} // namespace PrSS0TitlePrimitiveGroupDirect
