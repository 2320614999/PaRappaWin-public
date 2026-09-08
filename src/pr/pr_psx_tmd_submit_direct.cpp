#include "pr_psx_tmd_submit_direct.h"

#include <limits>

namespace PrPsxTmdSubmitDirect {
namespace {

Result Fail(Failure failure) {
    Result result{};
    result.failure = failure;
    return result;
}

bool HasSupportedShape(const TmdPrimitive& primitive) {
    return primitive.mode == kSupportedMode && primitive.ilen == 6u &&
           primitive.textured && !primitive.quad &&
           primitive.rawPacketByteSize == 28u;
}

uint32_t PackUv(uint8_t u, uint8_t v, uint16_t upper) {
    return static_cast<uint32_t>(u) |
           (static_cast<uint32_t>(v) << 8u) |
           (static_cast<uint32_t>(upper) << 16u);
}

} // namespace

Result Build(const Input& input) {
    if (input.object == nullptr) {
        return Fail(Failure::NullObject);
    }
    if (!input.sourceParseComplete) {
        return Fail(Failure::SourceParseIncomplete);
    }
    if (input.primitiveIndex >= input.object->primitives.size()) {
        return Fail(Failure::PrimitiveIndexOutOfRange);
    }

    const TmdPrimitive& primitive =
        input.object->primitives[input.primitiveIndex];
    if (!primitive.rawPacketKnown) {
        return Fail(Failure::RawPacketUnknown);
    }
    if (!HasSupportedShape(primitive)) {
        return Fail(Failure::UnsupportedPrimitiveShape);
    }
    const std::size_t vertexCount = input.object->vertices.size();
    if (primitive.v0_idx >= vertexCount || primitive.v1_idx >= vertexCount ||
        primitive.v2_idx >= vertexCount) {
        return Fail(Failure::VertexIndexOutOfRange);
    }
    for (const ProjectedSxyWord& sxy : input.projectedSxy) {
        if (!sxy.known) {
            return Fail(Failure::ProjectedSxyUnknown);
        }
    }
    if (!input.visibilityKnown) {
        return Fail(Failure::VisibilityUnknown);
    }
    if (!input.visible) {
        return Fail(Failure::Culled);
    }
    if (!input.otzKnown) {
        return Fail(Failure::OtzUnknown);
    }
    if (!input.otShiftKnown) {
        return Fail(Failure::OtShiftUnknown);
    }
    if (input.otShift != kRequiredOtShift) {
        return Fail(Failure::UnsupportedOtShift);
    }
    if (!input.highPriorityKnown) {
        return Fail(Failure::HighPriorityUnknown);
    }
    if (input.highPriority) {
        return Fail(Failure::UnsupportedHighPriority);
    }
    if (!input.packetAllocatorKnown) {
        return Fail(Failure::PacketAllocatorUnknown);
    }
    if ((input.packetAllocatorAddr & 3u) != 0u) {
        return Fail(Failure::PacketAllocatorMisaligned);
    }
    if (input.packetAllocatorAddr >
        std::numeric_limits<uint32_t>::max() - kPacketByteSize) {
        return Fail(Failure::PacketAllocatorOverflow);
    }

    const OrderingTableInput& ot = input.orderingTable;
    if (!ot.headKnown) {
        return Fail(Failure::OrderingTableHeadUnknown);
    }
    if (!ot.lengthKnown) {
        return Fail(Failure::OrderingTableLengthUnknown);
    }
    if (!ot.slotOldValueKnown) {
        return Fail(Failure::OrderingTableSlotOldValueUnknown);
    }
    if ((ot.head & 3u) != 0u) {
        return Fail(Failure::OrderingTableHeadMisaligned);
    }

    const uint16_t priority =
        static_cast<uint16_t>(input.otz >> kRequiredOtShift);
    if (static_cast<uint32_t>(priority) >= ot.length) {
        return Fail(Failure::PriorityOutOfRange);
    }

    const uint64_t priorityBytes = static_cast<uint64_t>(priority) * 4u;
    const uint64_t lengthBytes = static_cast<uint64_t>(ot.length) * 4u;
    const uint64_t headPlusPriority =
        static_cast<uint64_t>(ot.head) + priorityBytes;
    if (lengthBytes > std::numeric_limits<uint32_t>::max() ||
        headPlusPriority > std::numeric_limits<uint32_t>::max()) {
        return Fail(Failure::OrderingTableAddressOverflow);
    }
    if (headPlusPriority < lengthBytes) {
        return Fail(Failure::OrderingTableUnderflow);
    }
    const uint32_t slotAddress =
        static_cast<uint32_t>(headPlusPriority - lengthBytes);

    Result result{};
    result.packetWrite.marked = true;
    result.packetWrite.address = input.packetAllocatorAddr;
    result.packetWrite.words = {{
        (7u << 24u) | (ot.slotOldValue & kPacketAddressMask),
        0x24000000u | static_cast<uint32_t>(primitive.r) |
            (static_cast<uint32_t>(primitive.g) << 8u) |
            (static_cast<uint32_t>(primitive.b) << 16u),
        input.projectedSxy[0].value,
        PackUv(primitive.u0, primitive.v0, primitive.clut),
        input.projectedSxy[1].value,
        PackUv(primitive.u1, primitive.v1, primitive.tpage),
        input.projectedSxy[2].value,
        PackUv(primitive.u2, primitive.v2, 0u),
    }};

    result.otDelta.marked = true;
    result.otDelta.slotAddress = slotAddress;
    result.otDelta.oldValue = ot.slotOldValue;
    result.otDelta.newValue = input.packetAllocatorAddr & kPacketAddressMask;

    result.allocatorDelta.marked = true;
    result.allocatorDelta.oldAddress = input.packetAllocatorAddr;
    result.allocatorDelta.newAddress =
        input.packetAllocatorAddr + kPacketByteSize;
    result.allocatorDelta.advanceBytes = kPacketByteSize;
    result.priority = priority;
    result.readyToCommit = true;
    return result;
}

} // namespace PrPsxTmdSubmitDirect
