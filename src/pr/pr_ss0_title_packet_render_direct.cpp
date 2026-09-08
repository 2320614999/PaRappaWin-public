#include "pr_ss0_title_packet_render_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace PrSS0TitlePacketRenderDirect {
namespace {

constexpr uint32_t kAddressMask =
    PrPsxTmdSubmitDirect::kPacketAddressMask;
constexpr uint32_t kEndOfChain = kAddressMask;
constexpr uint8_t kMode25PacketLength = 7u;
constexpr uint8_t kMode25PacketOpcode = 0x24u;
constexpr std::size_t kOtSlotCount =
    std::size_t{1u} << PrSS0TitlePacketWorkDirect::kWorkOrder801C609C;
constexpr std::size_t kTraversalNodeLimit =
    kOtSlotCount + kMaxMode25DrawCommands801C689C;

static_assert(
    kOtSlotCount <= PrPsxGraphOwnerDirect::kTmdRuntimeOtSlotCapacity,
    "title OT mirror must hold every order-10 slot");

enum class LinkKind : uint8_t {
    None = 0,
    OtSlot,
    Packet,
};

struct LinkMatch {
    LinkKind kind = LinkKind::None;
    std::size_t index = 0;
    uint32_t count = 0;
};

uint32_t Low24(uint32_t value)
{
    return value & kAddressMask;
}

BuildResult801C689C Fail(Failure failure, uint8_t drawBuffer)
{
    BuildResult801C689C out{};
    out.failure = failure;
    out.drawBuffer = drawBuffer;
    return out;
}

bool HasExactWorkShape(
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint8_t drawBuffer)
{
    (void)drawBuffer;
    const uint64_t expectedHead =
        static_cast<uint64_t>(
            PrSS0TitlePacketWorkDirect::kWork0OtHead801C959C);
    const uint64_t expectedLast =
        expectedHead + static_cast<uint64_t>(kOtSlotCount - 1u) * 4u;
    if (expectedLast > std::numeric_limits<uint32_t>::max()) {
        return false;
    }

    return work.order_00 ==
               PrSS0TitlePacketWorkDirect::kWorkOrder801C609C &&
           work.x_08 == 0u && work.y_0C == 0u &&
           work.headAddr_04 == static_cast<uint32_t>(expectedHead) &&
           work.lastAddr_10 == static_cast<uint32_t>(expectedLast);
}

LinkMatch FindLink(
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint32_t address)
{
    LinkMatch out{};
    for (std::size_t index = 0; index < work.tmdOtSlotMirror.size();
         ++index) {
        const auto& slot = work.tmdOtSlotMirror[index];
        if (!slot.valid || Low24(slot.addr) != address) {
            continue;
        }
        if (out.count == 0u) {
            out.kind = LinkKind::OtSlot;
            out.index = index;
        }
        ++out.count;
    }
    for (std::size_t index = 0; index < work.tmdPacketWriteMirror.size();
         ++index) {
        const auto& packet = work.tmdPacketWriteMirror[index];
        if (!packet.valid || Low24(packet.addr) != address) {
            continue;
        }
        if (out.count == 0u) {
            out.kind = LinkKind::Packet;
            out.index = index;
        }
        ++out.count;
    }
    return out;
}

int16_t DecodeSignedHalf(uint32_t word, uint32_t shift)
{
    const uint32_t bits = (word >> shift) & 0xFFFFu;
    int32_t value = static_cast<int32_t>(bits);
    if (bits >= 0x8000u) {
        value -= 0x10000;
    }
    return static_cast<int16_t>(value);
}

Mode25DrawCommand801C689C DecodeCommand(
    const PrPsxGraphOwnerDirect::PsxTmdPacketWrite& packet,
    uint16_t priority,
    uint32_t traversalOrder)
{
    Mode25DrawCommand801C689C out{};
    out.valid = true;
    out.packetAddress = packet.addr;
    out.traversalOrder = traversalOrder;
    out.priority = priority;

    const uint32_t rgb = packet.words[1];
    out.r = static_cast<uint8_t>(rgb & 0xFFu);
    out.g = static_cast<uint8_t>((rgb >> 8u) & 0xFFu);
    out.b = static_cast<uint8_t>((rgb >> 16u) & 0xFFu);

    for (std::size_t vertex = 0; vertex < out.x.size(); ++vertex) {
        const uint32_t sxy = packet.words[2u + vertex * 2u];
        const uint32_t uv = packet.words[3u + vertex * 2u];
        out.x[vertex] = DecodeSignedHalf(sxy, 0u);
        out.y[vertex] = DecodeSignedHalf(sxy, 16u);
        out.u[vertex] = static_cast<uint8_t>(uv & 0xFFu);
        out.v[vertex] = static_cast<uint8_t>((uv >> 8u) & 0xFFu);
    }
    out.clut = static_cast<uint16_t>(packet.words[3] >> 16u);
    out.tpage = static_cast<uint16_t>(packet.words[5] >> 16u);
    return out;
}

bool HasConsistentDecodedShape(const BuildResult801C689C& decoded)
{
    if (!decoded.complete || decoded.failure != Failure::None ||
        decoded.drawBuffer >= 2u ||
        decoded.visitedOtSlots != kOtSlotCount ||
        decoded.visitedPackets != decoded.commandCount ||
        decoded.commandCount > decoded.commands.size()) {
        return false;
    }

    for (std::size_t index = 0; index < decoded.commands.size(); ++index) {
        const bool expectedValid = index < decoded.commandCount;
        const auto& command = decoded.commands[index];
        if (command.valid != expectedValid ||
            (expectedValid && command.traversalOrder != index)) {
            return false;
        }
    }
    return true;
}

bool HasConsistentCacheShape(
    const HostVisibleTitleFrameCache801C689C& cache)
{
    return cache.valid && cache.resourceGeneration != 0u &&
           cache.sourceDrawBuffer == cache.decoded.drawBuffer &&
           HasConsistentDecodedShape(cache.decoded);
}

} // namespace

BuildResult801C689C BuildCurrentMode25DrawCommands801C689C(
    const PrSS0TitlePacketWorkDirect::RuntimeState801C609C& runtime)
{
    const uint8_t drawBuffer = runtime.currentDrawBuffer8004019C;
    const uint8_t titleWorkLane =
        PrSS0TitlePacketWorkDirect::kTitleWorkFixedLane801C609C;
    if (!runtime.initialized || !runtime.framePrepared801C6410 ||
        drawBuffer >= runtime.packetAllocatorBases801C956C.size() ||
        titleWorkLane >= runtime.workLists801C9574.size()) {
        return Fail(Failure::RuntimeNotReady, drawBuffer);
    }

    const auto& work = runtime.workLists801C9574[titleWorkLane];
    if (!HasExactWorkShape(work, drawBuffer)) {
        return Fail(Failure::WorkShapeMismatch, drawBuffer);
    }
    if (!work.tmdOtSlotMirrorKnown) {
        return Fail(Failure::OtMirrorUnknown, drawBuffer);
    }
    if (!work.tmdPacketWriteMirrorKnown) {
        return Fail(Failure::PacketMirrorUnknown, drawBuffer);
    }

    BuildResult801C689C candidate{};
    candidate.drawBuffer = drawBuffer;
    std::array<bool, PrPsxGraphOwnerDirect::kTmdRuntimeOtSlotCapacity>
        visitedOt{};
    std::array<bool, kMaxMode25DrawCommands801C689C> visitedPacket{};

    uint32_t link = Low24(work.lastAddr_10);
    uint16_t currentPriority = 0;
    std::size_t traversedNodes = 0;
    while (link != kEndOfChain) {
        if (traversedNodes >= kTraversalNodeLimit) {
            return Fail(Failure::TraversalLimitExceeded, drawBuffer);
        }

        const LinkMatch match = FindLink(work, link);
        if (match.count == 0u) {
            return Fail(Failure::LinkNotFound, drawBuffer);
        }
        if (match.count != 1u) {
            return Fail(Failure::LinkAmbiguous, drawBuffer);
        }
        ++traversedNodes;

        if (match.kind == LinkKind::OtSlot) {
            if (visitedOt[match.index]) {
                return Fail(Failure::TraversalLimitExceeded, drawBuffer);
            }
            if (candidate.visitedOtSlots >= kOtSlotCount) {
                return Fail(Failure::OtOrderMismatch, drawBuffer);
            }

            const std::size_t expectedPriority =
                kOtSlotCount - 1u - candidate.visitedOtSlots;
            const uint64_t expectedAddress =
                static_cast<uint64_t>(work.headAddr_04) +
                static_cast<uint64_t>(expectedPriority) * 4u;
            const auto& slot = work.tmdOtSlotMirror[match.index];
            if (match.index != expectedPriority ||
                expectedAddress > std::numeric_limits<uint32_t>::max() ||
                slot.addr != static_cast<uint32_t>(expectedAddress)) {
                return Fail(Failure::OtOrderMismatch, drawBuffer);
            }

            visitedOt[match.index] = true;
            ++candidate.visitedOtSlots;
            currentPriority = static_cast<uint16_t>(expectedPriority);
            link = Low24(slot.value);
            continue;
        }

        if (candidate.visitedOtSlots == 0u) {
            return Fail(Failure::PacketBeforeOt, drawBuffer);
        }
        if (visitedPacket[match.index]) {
            return Fail(Failure::TraversalLimitExceeded, drawBuffer);
        }
        if (candidate.commandCount >= candidate.commands.size()) {
            return Fail(Failure::TraversalLimitExceeded, drawBuffer);
        }

        const auto& packet = work.tmdPacketWriteMirror[match.index];
        visitedPacket[match.index] = true;
        ++candidate.visitedPackets;
        if (static_cast<uint8_t>(packet.words[0] >> 24u) !=
            kMode25PacketLength) {
            return Fail(Failure::PacketLengthMismatch, drawBuffer);
        }
        if (static_cast<uint8_t>(packet.words[1] >> 24u) !=
            kMode25PacketOpcode) {
            return Fail(Failure::PacketOpcodeMismatch, drawBuffer);
        }

        candidate.commands[candidate.commandCount] = DecodeCommand(
            packet, currentPriority, candidate.commandCount);
        ++candidate.commandCount;
        link = Low24(packet.words[0]);
    }

    if (candidate.visitedOtSlots != kOtSlotCount) {
        return Fail(Failure::OtOrderMismatch, drawBuffer);
    }
    for (std::size_t index = 0; index < work.tmdPacketWriteMirror.size();
         ++index) {
        if (work.tmdPacketWriteMirror[index].valid &&
            !visitedPacket[index]) {
            return Fail(Failure::UnreachablePacketWrite, drawBuffer);
        }
    }

    candidate.complete = true;
    return candidate;
}

HostPresentProjection801C689C BuildHostPresentProjection801C689C(
    bool presentModelApplied,
    bool extraFlipGraphStateAdvanced,
    bool fullHeightWhiteFillRequested)
{
    HostPresentProjection801C689C out{};
    if (!presentModelApplied ||
        extraFlipGraphStateAdvanced != fullHeightWhiteFillRequested) {
        return out;
    }

    out.accepted = true;
    out.titleCommandsVisibleOnCurrentHostFrame = true;
    out.offscreenWhiteFillRequested = fullHeightWhiteFillRequested;
    out.currentHostFrameWhiteFillRequested = false;
    return out;
}

void ResetHostVisibleTitleFrameCache801C689C(
    HostVisibleTitleFrameCache801C689C& cache)
{
    cache = HostVisibleTitleFrameCache801C689C{};
}

bool CommitHostVisibleTitleFrameCache801C689C(
    HostVisibleTitleFrameCache801C689C& cache,
    const BuildResult801C689C& decoded,
    const HostPresentProjection801C689C& projection,
    uint32_t resourceGeneration)
{
    if (!HasConsistentDecodedShape(decoded) || !projection.accepted ||
        !projection.titleCommandsVisibleOnCurrentHostFrame ||
        resourceGeneration == 0u) {
        return false;
    }

    HostVisibleTitleFrameCache801C689C candidate{};
    candidate.valid = true;
    candidate.resourceGeneration = resourceGeneration;
    candidate.sourceDrawBuffer = decoded.drawBuffer;
    candidate.decoded = decoded;
    if (!HasConsistentCacheShape(candidate)) {
        return false;
    }

    cache = candidate;
    return true;
}

const BuildResult801C689C* ResolveHostVisibleTitleFrameCache801C689C(
    const HostVisibleTitleFrameCache801C689C& cache,
    uint32_t resourceGeneration)
{
    if (resourceGeneration == 0u ||
        resourceGeneration != cache.resourceGeneration ||
        !HasConsistentCacheShape(cache)) {
        return nullptr;
    }
    return &cache.decoded;
}

} // namespace PrSS0TitlePacketRenderDirect
