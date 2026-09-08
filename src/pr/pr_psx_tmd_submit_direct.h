#pragma once

#include "pr_tmd.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrPsxTmdSubmitDirect {

constexpr uint8_t kSupportedMode = 0x25u;
constexpr uint8_t kRequiredOtShift = 4u;
constexpr uint32_t kPacketAddressMask = 0x00FFFFFFu;
constexpr std::size_t kPacketWordCount = 8u;
constexpr uint32_t kPacketByteSize = 32u;

struct ProjectedSxyWord {
    bool known = false;
    uint32_t value = 0;
};

struct OrderingTableInput {
    bool headKnown = false;
    uint32_t head = 0;
    bool lengthKnown = false;
    uint32_t length = 0;
    bool slotOldValueKnown = false;
    uint32_t slotOldValue = 0;
};

struct Input {
    const TmdObject* object = nullptr;
    std::size_t primitiveIndex = 0;
    bool sourceParseComplete = false;
    std::array<ProjectedSxyWord, 3> projectedSxy{};
    bool visibilityKnown = false;
    bool visible = false;
    bool otzKnown = false;
    uint16_t otz = 0;
    bool otShiftKnown = false;
    uint8_t otShift = 0;
    bool highPriorityKnown = false;
    bool highPriority = false;
    bool packetAllocatorKnown = false;
    uint32_t packetAllocatorAddr = 0;
    OrderingTableInput orderingTable{};
};

enum class Failure : uint8_t {
    None = 0,
    NullObject,
    SourceParseIncomplete,
    PrimitiveIndexOutOfRange,
    RawPacketUnknown,
    UnsupportedPrimitiveShape,
    VertexIndexOutOfRange,
    ProjectedSxyUnknown,
    VisibilityUnknown,
    Culled,
    OtzUnknown,
    OtShiftUnknown,
    UnsupportedOtShift,
    HighPriorityUnknown,
    UnsupportedHighPriority,
    PacketAllocatorUnknown,
    PacketAllocatorMisaligned,
    PacketAllocatorOverflow,
    OrderingTableHeadUnknown,
    OrderingTableLengthUnknown,
    OrderingTableSlotOldValueUnknown,
    OrderingTableHeadMisaligned,
    OrderingTableAddressOverflow,
    OrderingTableUnderflow,
    PriorityOutOfRange,
};

struct PacketWrite {
    bool marked = false;
    uint32_t address = 0;
    std::array<uint32_t, kPacketWordCount> words{};
};

struct OrderingTableDelta {
    bool marked = false;
    uint32_t slotAddress = 0;
    uint32_t oldValue = 0;
    uint32_t newValue = 0;
};

struct AllocatorDelta {
    bool marked = false;
    uint32_t oldAddress = 0;
    uint32_t newAddress = 0;
    uint32_t advanceBytes = 0;
};

struct Result {
    Failure failure = Failure::None;
    PacketWrite packetWrite{};
    OrderingTableDelta otDelta{};
    AllocatorDelta allocatorDelta{};
    uint16_t priority = 0;
    bool readyToCommit = false;
};

Result Build(const Input& input);

} // namespace PrPsxTmdSubmitDirect
