#include "pr/pr_ss0_title_packet_render_direct.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>

namespace {

using namespace PrSS0TitlePacketRenderDirect;

constexpr std::size_t kOtSlotCount =
    std::size_t{1u} << PrSS0TitlePacketWorkDirect::kWorkOrder801C609C;
constexpr uint32_t kAddressMask =
    PrPsxTmdSubmitDirect::kPacketAddressMask;

using Runtime = PrSS0TitlePacketWorkDirect::RuntimeState801C609C;
using Work = PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8;

std::unique_ptr<Runtime> MakeRuntime(uint8_t drawBuffer = 0u)
{
    auto runtime = std::make_unique<Runtime>();
    runtime->initialized = true;
    runtime->framePrepared801C6410 = true;
    runtime->currentDrawBuffer8004019C = drawBuffer;

    Work& work = runtime->workLists801C9574[drawBuffer];
    work.order_00 = PrSS0TitlePacketWorkDirect::kWorkOrder801C609C;
    work.headAddr_04 =
        PrSS0TitlePacketWorkDirect::kWork0OtHead801C959C +
        static_cast<uint32_t>(drawBuffer) *
            PrSS0TitlePacketWorkDirect::kWorkOtHeadStride801C609C;
    work.x_08 = 0u;
    work.y_0C = 0u;
    work.lastAddr_10 =
        work.headAddr_04 + static_cast<uint32_t>(kOtSlotCount - 1u) * 4u;
    work.tmdOtSlotMirrorKnown = true;
    work.tmdPacketWriteMirrorKnown = true;
    for (std::size_t priority = 0; priority < kOtSlotCount; ++priority) {
        auto& slot = work.tmdOtSlotMirror[priority];
        slot.valid = true;
        slot.addr =
            work.headAddr_04 + static_cast<uint32_t>(priority) * 4u;
        slot.value =
            priority == 0u
                ? kAddressMask
                : (slot.addr - 4u) & kAddressMask;
    }
    return runtime;
}

std::array<uint32_t, PrPsxTmdSubmitDirect::kPacketWordCount>
MakePacketWords(uint32_t nextAddress,
                uint8_t length = 7u,
                uint8_t opcode = 0x24u)
{
    return {{
        (static_cast<uint32_t>(length) << 24u) |
            (nextAddress & kAddressMask),
        (static_cast<uint32_t>(opcode) << 24u) | 0x00CCBBAAu,
        0x0003FFFEu,
        0x48B52211u,
        0x80007FFFu,
        0x00094433u,
        0xFFFF8000u,
        0x12346655u,
    }};
}

void PushPacket(Runtime& runtime,
                std::size_t mirrorIndex,
                uint16_t priority,
                uint32_t packetAddress,
                uint8_t length = 7u,
                uint8_t opcode = 0x24u)
{
    Work& work =
        runtime.workLists801C9574[runtime.currentDrawBuffer8004019C];
    auto& slot = work.tmdOtSlotMirror[priority];
    auto& packet = work.tmdPacketWriteMirror[mirrorIndex];
    packet.valid = true;
    packet.addr = packetAddress;
    packet.words = MakePacketWords(slot.value, length, opcode);
    slot.value = packetAddress & kAddressMask;
}

void CheckFailClosed(const BuildResult801C689C& result, Failure failure)
{
    assert(!result.complete);
    assert(result.failure == failure);
    assert(result.visitedOtSlots == 0u);
    assert(result.visitedPackets == 0u);
    assert(result.commandCount == 0u);
    for (const auto& command : result.commands) {
        assert(!command.valid);
    }
}

bool SameCommand(const Mode25DrawCommand801C689C& lhs,
                 const Mode25DrawCommand801C689C& rhs)
{
    return lhs.valid == rhs.valid &&
           lhs.packetAddress == rhs.packetAddress &&
           lhs.traversalOrder == rhs.traversalOrder &&
           lhs.priority == rhs.priority && lhs.tpage == rhs.tpage &&
           lhs.clut == rhs.clut && lhs.x == rhs.x && lhs.y == rhs.y &&
           lhs.u == rhs.u && lhs.v == rhs.v && lhs.r == rhs.r &&
           lhs.g == rhs.g && lhs.b == rhs.b;
}

void CheckSameCache(const HostVisibleTitleFrameCache801C689C& lhs,
                    const HostVisibleTitleFrameCache801C689C& rhs)
{
    assert(lhs.valid == rhs.valid);
    assert(lhs.resourceGeneration == rhs.resourceGeneration);
    assert(lhs.sourceDrawBuffer == rhs.sourceDrawBuffer);
    assert(lhs.decoded.complete == rhs.decoded.complete);
    assert(lhs.decoded.failure == rhs.decoded.failure);
    assert(lhs.decoded.drawBuffer == rhs.decoded.drawBuffer);
    assert(lhs.decoded.visitedOtSlots == rhs.decoded.visitedOtSlots);
    assert(lhs.decoded.visitedPackets == rhs.decoded.visitedPackets);
    assert(lhs.decoded.commandCount == rhs.decoded.commandCount);
    for (std::size_t index = 0; index < lhs.decoded.commands.size();
         ++index) {
        assert(SameCommand(lhs.decoded.commands[index],
                           rhs.decoded.commands[index]));
    }
}

void TestCrossPriorityAndSamePriorityLifo()
{
    auto runtime = MakeRuntime();
    PushPacket(*runtime, 0u, 700u, 0x801AE430u);
    PushPacket(*runtime, 1u, 700u, 0x801AE450u);
    PushPacket(*runtime, 2u, 120u, 0x801AE470u);

    const auto result = BuildCurrentMode25DrawCommands801C689C(*runtime);
    assert(result.complete);
    assert(result.failure == Failure::None);
    assert(result.drawBuffer == 0u);
    assert(result.visitedOtSlots == kOtSlotCount);
    assert(result.visitedPackets == 3u);
    assert(result.commandCount == 3u);

    assert(result.commands[0].packetAddress == 0x801AE450u);
    assert(result.commands[0].priority == 700u);
    assert(result.commands[0].traversalOrder == 0u);
    assert(result.commands[1].packetAddress == 0x801AE430u);
    assert(result.commands[1].priority == 700u);
    assert(result.commands[1].traversalOrder == 1u);
    assert(result.commands[2].packetAddress == 0x801AE470u);
    assert(result.commands[2].priority == 120u);
    assert(result.commands[2].traversalOrder == 2u);

    const auto& decoded = result.commands[0];
    assert(decoded.valid);
    assert(decoded.r == 0xAAu);
    assert(decoded.g == 0xBBu);
    assert(decoded.b == 0xCCu);
    assert((decoded.x ==
            std::array<int16_t, 3>{{-2, 32767, -32768}}));
    assert((decoded.y ==
            std::array<int16_t, 3>{{3, -32768, -1}}));
    assert((decoded.u ==
            std::array<uint8_t, 3>{{0x11u, 0x33u, 0x55u}}));
    assert((decoded.v ==
            std::array<uint8_t, 3>{{0x22u, 0x44u, 0x66u}}));
    assert(decoded.clut == 0x48B5u);
    assert(decoded.tpage == 0x0009u);
    assert(!result.commands[3].valid);
}

void TestUnknownMirrorsAndShapes()
{
    {
        auto runtime = MakeRuntime();
        runtime->framePrepared801C6410 = false;
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::RuntimeNotReady);
    }
    {
        auto runtime = MakeRuntime();
        runtime->workLists801C9574[0].lastAddr_10 -= 4u;
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::WorkShapeMismatch);
    }
    {
        auto runtime = MakeRuntime();
        runtime->workLists801C9574[0].tmdOtSlotMirrorKnown = false;
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::OtMirrorUnknown);
    }
    {
        auto runtime = MakeRuntime();
        runtime->workLists801C9574[0].tmdPacketWriteMirrorKnown = false;
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::PacketMirrorUnknown);
    }
}

void TestBadLinksAndTraversal()
{
    {
        auto runtime = MakeRuntime();
        runtime->workLists801C9574[0]
            .tmdOtSlotMirror[kOtSlotCount - 1u]
            .value = 0x00123456u;
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::LinkNotFound);
    }
    {
        auto runtime = MakeRuntime();
        Work& work = runtime->workLists801C9574[0];
        auto& packet = work.tmdPacketWriteMirror[0];
        packet.valid = true;
        packet.addr = work.lastAddr_10;
        packet.words = MakePacketWords(kAddressMask);
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::LinkAmbiguous);
    }
    {
        auto runtime = MakeRuntime();
        Work& work = runtime->workLists801C9574[0];
        work.tmdOtSlotMirror[kOtSlotCount - 1u].value =
            work.lastAddr_10 & kAddressMask;
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::TraversalLimitExceeded);
    }
    {
        auto runtime = MakeRuntime();
        Work& work = runtime->workLists801C9574[0];
        work.tmdOtSlotMirror[kOtSlotCount - 1u].value =
            work.tmdOtSlotMirror[kOtSlotCount - 3u].addr & kAddressMask;
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::OtOrderMismatch);
    }
    {
        auto runtime = MakeRuntime();
        Work& work = runtime->workLists801C9574[0];
        const uint32_t tailAddress = work.lastAddr_10;
        work.tmdOtSlotMirror[kOtSlotCount - 1u].valid = false;
        auto& packet = work.tmdPacketWriteMirror[0];
        packet.valid = true;
        packet.addr = tailAddress;
        packet.words = MakePacketWords(
            work.tmdOtSlotMirror[kOtSlotCount - 2u].addr);
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::PacketBeforeOt);
    }
}

void TestPacketValidationAndReachability()
{
    {
        auto runtime = MakeRuntime();
        PushPacket(*runtime, 0u, 1023u, 0x801AE430u, 6u, 0x24u);
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::PacketLengthMismatch);
    }
    {
        auto runtime = MakeRuntime();
        PushPacket(*runtime, 0u, 1023u, 0x801AE430u, 7u, 0x25u);
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::PacketOpcodeMismatch);
    }
    {
        auto runtime = MakeRuntime();
        auto& packet = runtime->workLists801C9574[0]
                           .tmdPacketWriteMirror[0];
        packet.valid = true;
        packet.addr = 0x801AE430u;
        packet.words = MakePacketWords(kAddressMask);
        CheckFailClosed(BuildCurrentMode25DrawCommands801C689C(*runtime),
                        Failure::UnreachablePacketWrite);
    }
}

void TestHostPresentProjection()
{
    const auto normal = BuildHostPresentProjection801C689C(
        true, false, false);
    assert(normal.accepted);
    assert(normal.titleCommandsVisibleOnCurrentHostFrame);
    assert(!normal.offscreenWhiteFillRequested);
    assert(!normal.currentHostFrameWhiteFillRequested);

    const auto extra = BuildHostPresentProjection801C689C(
        true, true, true);
    assert(extra.accepted);
    assert(extra.titleCommandsVisibleOnCurrentHostFrame);
    assert(extra.offscreenWhiteFillRequested);
    assert(!extra.currentHostFrameWhiteFillRequested);

    assert(!BuildHostPresentProjection801C689C(false, false, false).accepted);
    assert(!BuildHostPresentProjection801C689C(true, true, false).accepted);
    assert(!BuildHostPresentProjection801C689C(true, false, true).accepted);
}

void TestHostVisibleTitleFrameCacheSuccess()
{
    auto runtime = MakeRuntime(1u);
    PushPacket(*runtime, 0u, 700u, 0x801AE430u);
    const auto decoded = BuildCurrentMode25DrawCommands801C689C(*runtime);
    const auto projection =
        BuildHostPresentProjection801C689C(true, false, false);

    HostVisibleTitleFrameCache801C689C cache{};
    assert(CommitHostVisibleTitleFrameCache801C689C(
        cache, decoded, projection, 17u));
    assert(cache.valid);
    assert(cache.resourceGeneration == 17u);
    assert(cache.sourceDrawBuffer == 1u);

    const auto* resolved =
        ResolveHostVisibleTitleFrameCache801C689C(cache, 17u);
    assert(resolved == &cache.decoded);
    assert(resolved->commandCount == 1u);
    assert(resolved->commands[0].packetAddress == 0x801AE430u);

    const auto snapshot = cache;
    assert(ResolveHostVisibleTitleFrameCache801C689C(cache, 18u) ==
           nullptr);
    CheckSameCache(cache, snapshot);
}

void TestHostVisibleTitleFrameCacheZeroCommands()
{
    auto runtime = MakeRuntime();
    const auto decoded = BuildCurrentMode25DrawCommands801C689C(*runtime);
    assert(decoded.complete);
    assert(decoded.commandCount == 0u);

    HostVisibleTitleFrameCache801C689C cache{};
    const auto projection =
        BuildHostPresentProjection801C689C(true, false, false);
    assert(CommitHostVisibleTitleFrameCache801C689C(
        cache, decoded, projection, 23u));

    const auto* resolved =
        ResolveHostVisibleTitleFrameCache801C689C(cache, 23u);
    assert(resolved != nullptr);
    assert(resolved->commandCount == 0u);
    assert(resolved->visitedPackets == 0u);
}

void TestHostVisibleTitleFrameCacheFailedCommitPreservesState()
{
    auto runtime = MakeRuntime();
    PushPacket(*runtime, 0u, 700u, 0x801AE430u);
    const auto decoded = BuildCurrentMode25DrawCommands801C689C(*runtime);
    const auto projection =
        BuildHostPresentProjection801C689C(true, false, false);
    HostVisibleTitleFrameCache801C689C cache{};
    assert(CommitHostVisibleTitleFrameCache801C689C(
        cache, decoded, projection, 29u));
    const auto snapshot = cache;

    auto badDecoded = decoded;
    badDecoded.complete = false;
    assert(!CommitHostVisibleTitleFrameCache801C689C(
        cache, badDecoded, projection, 30u));
    CheckSameCache(cache, snapshot);

    badDecoded = decoded;
    badDecoded.failure = Failure::PacketOpcodeMismatch;
    assert(!CommitHostVisibleTitleFrameCache801C689C(
        cache, badDecoded, projection, 30u));
    CheckSameCache(cache, snapshot);

    auto badProjection = projection;
    badProjection.accepted = false;
    assert(!CommitHostVisibleTitleFrameCache801C689C(
        cache, decoded, badProjection, 30u));
    CheckSameCache(cache, snapshot);

    badProjection = projection;
    badProjection.titleCommandsVisibleOnCurrentHostFrame = false;
    assert(!CommitHostVisibleTitleFrameCache801C689C(
        cache, decoded, badProjection, 30u));
    CheckSameCache(cache, snapshot);

    assert(!CommitHostVisibleTitleFrameCache801C689C(
        cache, decoded, projection, 0u));
    CheckSameCache(cache, snapshot);
}

void TestHostVisibleTitleFrameCacheShapeAndReset()
{
    auto runtime = MakeRuntime();
    PushPacket(*runtime, 0u, 700u, 0x801AE430u);
    const auto decoded = BuildCurrentMode25DrawCommands801C689C(*runtime);
    const auto projection =
        BuildHostPresentProjection801C689C(true, false, false);
    HostVisibleTitleFrameCache801C689C cache{};
    assert(CommitHostVisibleTitleFrameCache801C689C(
        cache, decoded, projection, 31u));

    auto malformed = cache;
    malformed.sourceDrawBuffer = 1u;
    assert(ResolveHostVisibleTitleFrameCache801C689C(malformed, 31u) ==
           nullptr);
    malformed = cache;
    malformed.decoded.commands[0].valid = false;
    assert(ResolveHostVisibleTitleFrameCache801C689C(malformed, 31u) ==
           nullptr);
    malformed = cache;
    malformed.decoded.commandCount =
        static_cast<uint32_t>(malformed.decoded.commands.size() + 1u);
    assert(ResolveHostVisibleTitleFrameCache801C689C(malformed, 31u) ==
           nullptr);

    ResetHostVisibleTitleFrameCache801C689C(cache);
    assert(!cache.valid);
    assert(cache.resourceGeneration == 0u);
    assert(cache.sourceDrawBuffer == 0u);
    assert(!cache.decoded.complete);
    assert(cache.decoded.commandCount == 0u);
    assert(ResolveHostVisibleTitleFrameCache801C689C(cache, 31u) ==
           nullptr);
}

} // namespace

int main()
{
    TestCrossPriorityAndSamePriorityLifo();
    TestUnknownMirrorsAndShapes();
    TestBadLinksAndTraversal();
    TestPacketValidationAndReachability();
    TestHostPresentProjection();
    TestHostVisibleTitleFrameCacheSuccess();
    TestHostVisibleTitleFrameCacheZeroCommands();
    TestHostVisibleTitleFrameCacheFailedCommitPreservesState();
    TestHostVisibleTitleFrameCacheShapeAndReset();
    std::printf("test_ss0_title_packet_render_direct: ok\n");
    return 0;
}
