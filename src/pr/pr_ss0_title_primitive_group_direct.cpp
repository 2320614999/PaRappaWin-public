#include "pr_ss0_title_primitive_group_direct.h"

#include <limits>

namespace PrSS0TitlePrimitiveGroupDirect {
namespace {

ExecuteResult8004274C Fail(Failure failure,
                           std::size_t firstPrimitiveIndex,
                           std::size_t primitiveCount,
                           uint32_t allocator,
                           std::size_t failedPrimitiveIndex)
{
    ExecuteResult8004274C out{};
    out.failure = failure;
    out.firstPrimitiveIndex =
        static_cast<uint16_t>(firstPrimitiveIndex);
    out.requestedCount = static_cast<uint16_t>(primitiveCount);
    out.failedPrimitiveIndex =
        failedPrimitiveIndex <= std::numeric_limits<uint16_t>::max()
            ? static_cast<uint16_t>(failedPrimitiveIndex)
            : 0xFFFFu;
    out.allocatorBefore = allocator;
    out.allocatorAfter = allocator;
    return out;
}

bool TryExpectedPrimitiveCount(
    PrSS0TitleTransformDirect::ModelPath path,
    std::size_t& primitiveCount)
{
    switch (path) {
    case PrSS0TitleTransformDirect::ModelPath::Pa:
        primitiveCount = PrSS0TitleDrawDescDirect::kPaPrimitiveCount;
        return true;
    case PrSS0TitleTransformDirect::ModelPath::PaKage:
        primitiveCount = PrSS0TitleDrawDescDirect::kPaKagePrimitiveCount;
        return true;
    case PrSS0TitleTransformDirect::ModelPath::Lo:
        primitiveCount = PrSS0TitleDrawDescDirect::kLoPrimitiveCount;
        return true;
    case PrSS0TitleTransformDirect::ModelPath::Hp:
        primitiveCount = PrSS0TitleDrawDescDirect::kHpPrimitiveCount;
        return true;
    }
    primitiveCount = 0u;
    return false;
}

bool DrawDescriptorMatchesPath(
    const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C& drawDesc,
    PrSS0TitleTransformDirect::ModelPath path)
{
    if (!drawDesc.modelBindingKnown || drawDesc.model == nullptr) {
        return false;
    }
    switch (path) {
    case PrSS0TitleTransformDirect::ModelPath::Pa:
        return drawDesc.modelPath ==
               PrSS0TitleDrawDescDirect::ModelPath::Pa;
    case PrSS0TitleTransformDirect::ModelPath::PaKage:
        return drawDesc.modelPath ==
               PrSS0TitleDrawDescDirect::ModelPath::PaKage;
    case PrSS0TitleTransformDirect::ModelPath::Lo:
        return drawDesc.modelPath ==
               PrSS0TitleDrawDescDirect::ModelPath::Lo;
    case PrSS0TitleTransformDirect::ModelPath::Hp:
        return drawDesc.modelPath ==
               PrSS0TitleDrawDescDirect::ModelPath::Hp;
    }
    return false;
}

bool IsPacketRuntimeReady(
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    const PrPsxGraphOwnerDirect::PsxGraphState& graph)
{
    if (!packetWork.initialized || !packetWork.framePrepared801C6410 ||
        packetWork.currentDrawBuffer8004019C >=
            packetWork.workLists801C9574.size() ||
        packetWork.packetArenaBase80025B28 == 0u ||
        !graph.mainPageWorkLists80087288Initialized ||
        static_cast<uint8_t>(graph.word_80096590 & 1u) !=
            packetWork.currentDrawBuffer8004019C ||
        graph.dword_800901C8 !=
            packetWork.currentPacketAllocator800901C8) {
        return false;
    }

    const uint64_t arenaBase = packetWork.packetArenaBase80025B28;
    const uint64_t laneBytes =
        PrSS0TitlePacketWorkDirect::kPacketArenaLaneBytes801C5D28;
    const uint64_t arenaEnd = arenaBase + 2u * laneBytes;
    if (arenaEnd > std::numeric_limits<uint32_t>::max() ||
        packetWork.packetAllocatorBases801C956C[0] != arenaBase ||
        packetWork.packetAllocatorBases801C956C[1] != arenaBase + laneBytes) {
        return false;
    }

    const uint8_t lane = packetWork.currentDrawBuffer8004019C;
    const uint8_t titleWorkLane =
        PrSS0TitlePacketWorkDirect::kTitleWorkFixedLane801C609C;
    if (titleWorkLane >= packetWork.workLists801C9574.size()) {
        return false;
    }
    const auto& work = packetWork.workLists801C9574[titleWorkLane];
    const uint64_t expectedHead =
        static_cast<uint64_t>(
            PrSS0TitlePacketWorkDirect::kWork0OtHead801C959C);
    const uint64_t expectedTail =
        expectedHead +
        (1u << PrSS0TitlePacketWorkDirect::kWorkOrder801C609C) * 4u - 4u;
    return work.order_00 ==
               PrSS0TitlePacketWorkDirect::kWorkOrder801C609C &&
           work.headAddr_04 == expectedHead &&
           work.lastAddr_10 == expectedTail && work.x_08 == 0u &&
           work.y_0C == 0u && work.tmdOtSlotMirrorKnown &&
           work.tmdPacketWriteMirrorKnown;
}

Failure MapCommitFailure(PrSS0TitlePacketCommitDirect::Failure failure)
{
    using CommitFailure = PrSS0TitlePacketCommitDirect::Failure;
    switch (failure) {
    case CommitFailure::PacketArenaRangeMismatch:
        return Failure::PacketArenaCapacityExceeded;
    case CommitFailure::PacketCapacityExceeded:
        return Failure::PacketMirrorCapacityExceeded;
    case CommitFailure::PacketAddressAlreadyWritten:
        return Failure::PacketAddressConflict;
    case CommitFailure::OrderingTableMirrorUnknown:
    case CommitFailure::OrderingTableSlotNotFound:
    case CommitFailure::OrderingTableSlotMismatch:
        return Failure::OrderingTableSlotUnknown;
    default:
        return Failure::PacketCommitRejected;
    }
}

} // namespace

ExecuteResult8004274C ExecuteMode25PrimitiveRange8004274C(
    const PrSS0TitleDrawDescDirect::RuntimeState8001AF1C& drawDesc,
    PrSS0TitleTransformDirect::ModelPath path,
    const PrSS0TitleTransformDirect::MimeRuntimeInput80013EA8& mime,
    const PrPsxGteDirect::Matrix3x4* runtimeMatrix,
    bool runtimeMatrixKnown,
    PrSS0TitlePacketWorkDirect::RuntimeState801C609C& packetWork,
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    std::size_t firstPrimitiveIndex,
    std::size_t primitiveCount)
{
    const uint32_t allocator = packetWork.currentPacketAllocator800901C8;
    std::size_t expectedPrimitiveCount = 0u;
    if (!TryExpectedPrimitiveCount(path, expectedPrimitiveCount)) {
        return Fail(Failure::UnsupportedModelPath,
                    firstPrimitiveIndex,
                    primitiveCount,
                    allocator,
                    firstPrimitiveIndex);
    }
    const auto attr = PrSS0TitleDrawDescDirect::DecodeAttr800428B0(drawDesc);
    if (!drawDesc.initialized || !drawDesc.primitiveGroup.known ||
        !drawDesc.modelBindingKnown || drawDesc.model == nullptr ||
        !drawDesc.objectBindingKnown || drawDesc.object == nullptr ||
        !drawDesc.attrKnown || !attr.known || !attr.active ||
        drawDesc.objectIndex != 0u) {
        return Fail(Failure::DrawDescriptorNotReady,
                    firstPrimitiveIndex,
                    primitiveCount,
                    allocator,
                    firstPrimitiveIndex);
    }
    if (!DrawDescriptorMatchesPath(drawDesc, path)) {
        return Fail(Failure::PrimitiveProvenanceMismatch,
                    firstPrimitiveIndex,
                    primitiveCount,
                    allocator,
                    firstPrimitiveIndex);
    }
    if (!PrSS0TitleDrawDescDirect::IsMode25PrimitiveReady800428B0(
            drawDesc,
            drawDesc.object,
            drawDesc.primitiveGroup.firstPrimitiveIndex) ||
        drawDesc.primitiveGroup.firstPrimitiveIndex != 0u ||
        drawDesc.primitiveGroup.primitiveCount !=
            expectedPrimitiveCount ||
        drawDesc.primitiveGroup.mode !=
            PrSS0TitleDrawDescDirect::kMode25Mode ||
        drawDesc.primitiveGroup.flag !=
            PrSS0TitleDrawDescDirect::kMode25Flag ||
        drawDesc.primitiveGroup.wordStride !=
            PrSS0TitleDrawDescDirect::kMode25WordStride8004274C) {
        return Fail(Failure::DrawDescriptorNotReady,
                    firstPrimitiveIndex,
                    primitiveCount,
                    allocator,
                    firstPrimitiveIndex);
    }
    const uint64_t rangeEnd =
        static_cast<uint64_t>(firstPrimitiveIndex) + primitiveCount;
    const uint64_t groupEnd =
        static_cast<uint64_t>(drawDesc.primitiveGroup.firstPrimitiveIndex) +
        drawDesc.primitiveGroup.primitiveCount;
    if (primitiveCount == 0u ||
        firstPrimitiveIndex < drawDesc.primitiveGroup.firstPrimitiveIndex ||
        rangeEnd > groupEnd || rangeEnd > drawDesc.object->primitives.size() ||
        primitiveCount > expectedPrimitiveCount) {
        return Fail(Failure::PrimitiveRangeInvalid,
                    firstPrimitiveIndex,
                    primitiveCount,
                    allocator,
                    firstPrimitiveIndex);
    }
    if (!runtimeMatrixKnown || runtimeMatrix == nullptr) {
        return Fail(Failure::RuntimeMatrixUnknown,
                    firstPrimitiveIndex,
                    primitiveCount,
                    allocator,
                    firstPrimitiveIndex);
    }
    if (!IsPacketRuntimeReady(packetWork, graph)) {
        return Fail(Failure::PacketRuntimeNotReady,
                    firstPrimitiveIndex,
                    primitiveCount,
                    allocator,
                    firstPrimitiveIndex);
    }

    ExecuteResult8004274C out{};
    out.firstPrimitiveIndex = static_cast<uint16_t>(firstPrimitiveIndex);
    out.requestedCount = static_cast<uint16_t>(primitiveCount);
    out.allocatorBefore = allocator;
    out.allocatorAfter = allocator;

    for (std::size_t offset = 0u; offset < primitiveCount; ++offset) {
        const std::size_t primitiveIndex = firstPrimitiveIndex + offset;
        ++out.visitedCount;
        const auto base =
            PrSS0TitleTransformDirect::
                BuildMode25BaseTriangleCarrier801C5E60(
                    &graph.gte,
                    drawDesc.object,
                    drawDesc.objectIndex,
                    primitiveIndex,
                    true,
                    path);
        if (!base.baseCarrierReady || !base.carrier.known) {
            out.failure = Failure::BaseCarrierRejected;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            out.transformFailure = base.failure;
            break;
        }
        if (base.carrier.primitiveIndex != primitiveIndex ||
            base.carrier.objectIndex != drawDesc.objectIndex ||
            base.carrier.path != path) {
            out.failure = Failure::PrimitiveProvenanceMismatch;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            break;
        }

        const auto deformed =
            PrSS0TitleTransformDirect::ApplyMode25Mime80013EA8(
                base.carrier, mime);
        if (!deformed.deformedTriangleReady || !deformed.carrier.known) {
            out.failure = Failure::MimeDeformationRejected;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            out.transformFailure = deformed.failure;
            break;
        }
        if (deformed.carrier.primitiveIndex != primitiveIndex ||
            deformed.carrier.vertexIndices != base.carrier.vertexIndices ||
            deformed.carrier.path != path) {
            out.failure = Failure::PrimitiveProvenanceMismatch;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            break;
        }

        const auto joined =
            PrSS0TitleTransformDirect::BuildMode25Rtpt3Input801C5E60(
                deformed.carrier, runtimeMatrix, runtimeMatrixKnown);
        if (!joined.ready || !joined.input.known) {
            out.failure = Failure::Rtpt3Rejected;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            out.transformFailure = joined.failure;
            break;
        }
        if (joined.input.primitiveIndex != primitiveIndex ||
            joined.input.vertexIndices != deformed.carrier.vertexIndices ||
            joined.input.path != path) {
            out.failure = Failure::PrimitiveProvenanceMismatch;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            break;
        }

        auto executed =
            PrSS0TitleTransformDirect::ExecuteMode25Rtpt3At800428B0(
                joined.input);
        if (!executed.ready || !executed.output.flagAfterRtptKnown ||
            !executed.geometryReady ||
            !executed.geometry.projectedSxyKnown ||
            !executed.geometry.visibilityKnown ||
            !executed.geometry.otzKnown) {
            out.failure = Failure::Rtpt3Rejected;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            out.transformFailure = executed.failure;
            break;
        }
        if (executed.input.primitiveIndex != primitiveIndex ||
            executed.input.vertexIndices != joined.input.vertexIndices ||
            executed.input.path != path) {
            out.failure = Failure::PrimitiveProvenanceMismatch;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            break;
        }
        if (static_cast<int32_t>(executed.output.flagAfterRtpt) < 0) {
            executed.geometry.visible = false;
        }
        ++out.preparedCount;
        if (offset == 0u) {
            out.firstExecutionKnown = true;
            out.firstExecution = executed;
        }
        if (!executed.geometry.visible) {
            ++out.culledCount;
            continue;
        }

        const auto built =
            PrSS0TitlePacketPlanDirect::
                BuildMode25PacketPlan800428B0(
                    drawDesc,
                    packetWork,
                    graph,
                    executed.geometry,
                    primitiveIndex);
        if (!built.ready || !built.plan.readyToCommit) {
            out.failure = Failure::PacketPlanRejected;
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            out.packetPlanFailure = built.failure;
            break;
        }
        if (offset == 0u) {
            out.firstPlanKnown = true;
            out.firstPlan = built;
        }

        const auto committed =
            PrSS0TitlePacketCommitDirect::
                CommitMode25PacketPlan800428B0(
                    packetWork, graph, built);
        if (!committed.committed) {
            out.failure = MapCommitFailure(committed.failure);
            out.failedPrimitiveIndex =
                static_cast<uint16_t>(primitiveIndex);
            out.packetCommitFailure = committed.failure;
            break;
        }
        if (offset == 0u) {
            out.firstCommitKnown = true;
            out.firstCommit = committed;
        }
        ++out.committedCount;
    }

    out.allocatorAfter = packetWork.currentPacketAllocator800901C8;
    if (out.failure != Failure::None) {
        out.partialWrites = out.committedCount > 0u;
        return out;
    }

    const uint64_t expectedAllocatorAfter =
        static_cast<uint64_t>(out.allocatorBefore) +
        static_cast<uint64_t>(out.committedCount) *
            PrPsxTmdSubmitDirect::kPacketByteSize;
    if (expectedAllocatorAfter > std::numeric_limits<uint32_t>::max() ||
        out.allocatorAfter != expectedAllocatorAfter ||
        graph.dword_800901C8 != out.allocatorAfter) {
        out.failure = Failure::AllocatorAdvanceMismatch;
        out.partialWrites = out.committedCount > 0u;
        return out;
    }
    out.complete = out.visitedCount == out.requestedCount &&
                   out.preparedCount == out.requestedCount &&
                   out.committedCount + out.culledCount ==
                       out.requestedCount;
    out.partialWrites = !out.complete && out.committedCount > 0u;
    return out;
}

} // namespace PrSS0TitlePrimitiveGroupDirect
