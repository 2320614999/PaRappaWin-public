#include "pr/pr_psx_tmd_submit_direct.h"

#include <array>
#include <cstdio>

namespace {

using namespace PrPsxTmdSubmitDirect;

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,    \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

struct Fixture {
    TmdObject object{};
    Input input{};

    Fixture() {
        object.vertices.resize(3u);
        object.primitives.resize(1u);
        TmdPrimitive& primitive = object.primitives[0];
        primitive.rawPacketKnown = true;
        primitive.rawPacketByteSize = 28u;
        primitive.mode = 0x25u;
        primitive.ilen = 6u;
        primitive.textured = true;
        primitive.quad = false;
        primitive.r = 0x11u;
        primitive.g = 0x22u;
        primitive.b = 0x33u;
        primitive.u0 = 0x10u;
        primitive.v0 = 0x20u;
        primitive.u1 = 0x30u;
        primitive.v1 = 0x40u;
        primitive.u2 = 0x50u;
        primitive.v2 = 0x60u;
        primitive.clut = 0x4567u;
        primitive.tpage = 0x89ABu;
        primitive.v0_idx = 0u;
        primitive.v1_idx = 1u;
        primitive.v2_idx = 2u;

        input.object = &object;
        input.primitiveIndex = 0u;
        input.sourceParseComplete = true;
        input.projectedSxy = {{
            {true, 0x00200010u},
            {true, 0x00400030u},
            {true, 0x00600050u},
        }};
        input.visibilityKnown = true;
        input.visible = true;
        input.otzKnown = true;
        input.otz = 0x0050u;
        input.otShiftKnown = true;
        input.otShift = 4u;
        input.highPriorityKnown = true;
        input.highPriority = false;
        input.packetAllocatorKnown = true;
        input.packetAllocatorAddr = 0x80001000u;
        input.orderingTable.headKnown = true;
        input.orderingTable.head = 0x00002000u;
        input.orderingTable.lengthKnown = true;
        input.orderingTable.length = 64u;
        input.orderingTable.slotOldValueKnown = true;
        input.orderingTable.slotOldValue = 0xAB123456u;
    }
};

void CheckFailure(const Input& input, Failure expected) {
    const Result result = Build(input);
    CHECK(result.failure == expected);
    CHECK(!result.readyToCommit);
    CHECK(!result.packetWrite.marked);
    CHECK(!result.otDelta.marked);
    CHECK(!result.allocatorDelta.marked);
}

void TestExactSuccess() {
    Fixture fixture;
    const Result result = Build(fixture.input);
    const std::array<uint32_t, kPacketWordCount> expectedWords = {{
        0x07123456u,
        0x24332211u,
        0x00200010u,
        0x45672010u,
        0x00400030u,
        0x89AB4030u,
        0x00600050u,
        0x00006050u,
    }};

    CHECK(result.failure == Failure::None);
    CHECK(result.readyToCommit);
    CHECK(result.priority == 5u);
    CHECK(result.packetWrite.marked);
    CHECK(result.packetWrite.address == 0x80001000u);
    CHECK(result.packetWrite.words == expectedWords);
    CHECK(result.otDelta.marked);
    CHECK(result.otDelta.slotAddress == 0x00001F14u);
    CHECK(result.otDelta.oldValue == 0xAB123456u);
    CHECK(result.otDelta.newValue == 0x00001000u);
    CHECK(result.allocatorDelta.marked);
    CHECK(result.allocatorDelta.oldAddress == 0x80001000u);
    CHECK(result.allocatorDelta.newAddress == 0x80001020u);
    CHECK(result.allocatorDelta.advanceBytes == 32u);
}

void TestRequiredFailures() {
    {
        Fixture fixture;
        fixture.input.object = nullptr;
        CheckFailure(fixture.input, Failure::NullObject);
    }
    {
        Fixture fixture;
        fixture.input.sourceParseComplete = false;
        CheckFailure(fixture.input, Failure::SourceParseIncomplete);
    }
    {
        Fixture fixture;
        fixture.input.primitiveIndex = 1u;
        CheckFailure(fixture.input, Failure::PrimitiveIndexOutOfRange);
    }
    {
        Fixture fixture;
        fixture.object.primitives[0].mode = 0x27u;
        CheckFailure(fixture.input, Failure::UnsupportedPrimitiveShape);
    }
    {
        Fixture fixture;
        fixture.object.primitives[0].ilen = 5u;
        CheckFailure(fixture.input, Failure::UnsupportedPrimitiveShape);
    }
    {
        Fixture fixture;
        fixture.object.primitives[0].rawPacketByteSize = 24u;
        CheckFailure(fixture.input, Failure::UnsupportedPrimitiveShape);
    }
    {
        Fixture fixture;
        fixture.object.primitives[0].textured = false;
        CheckFailure(fixture.input, Failure::UnsupportedPrimitiveShape);
    }
    {
        Fixture fixture;
        fixture.object.primitives[0].quad = true;
        CheckFailure(fixture.input, Failure::UnsupportedPrimitiveShape);
    }
    {
        Fixture fixture;
        fixture.object.primitives[0].v2_idx = 3u;
        CheckFailure(fixture.input, Failure::VertexIndexOutOfRange);
    }
    for (std::size_t index = 0; index < 3u; ++index) {
        Fixture fixture;
        fixture.input.projectedSxy[index].known = false;
        CheckFailure(fixture.input, Failure::ProjectedSxyUnknown);
    }
    {
        Fixture fixture;
        fixture.input.visibilityKnown = false;
        CheckFailure(fixture.input, Failure::VisibilityUnknown);
    }
    {
        Fixture fixture;
        fixture.input.visible = false;
        CheckFailure(fixture.input, Failure::Culled);
    }
    {
        Fixture fixture;
        fixture.input.orderingTable.slotOldValueKnown = false;
        CheckFailure(fixture.input,
                     Failure::OrderingTableSlotOldValueUnknown);
    }
    {
        Fixture fixture;
        fixture.input.orderingTable.head = 0x00002002u;
        CheckFailure(fixture.input, Failure::OrderingTableHeadMisaligned);
    }
    {
        Fixture fixture;
        fixture.input.orderingTable.head = 0xFFFFFFFCu;
        CheckFailure(fixture.input, Failure::OrderingTableAddressOverflow);
    }
    {
        Fixture fixture;
        fixture.input.orderingTable.head = 0x00000040u;
        fixture.input.orderingTable.length = 32u;
        CheckFailure(fixture.input, Failure::OrderingTableUnderflow);
    }
    {
        Fixture fixture;
        fixture.input.orderingTable.length = 5u;
        CheckFailure(fixture.input, Failure::PriorityOutOfRange);
    }
    {
        Fixture fixture;
        fixture.input.packetAllocatorAddr = 0x80001002u;
        CheckFailure(fixture.input, Failure::PacketAllocatorMisaligned);
    }
    {
        Fixture fixture;
        fixture.input.packetAllocatorAddr = 0xFFFFFFF0u;
        CheckFailure(fixture.input, Failure::PacketAllocatorOverflow);
    }
}

void TestRemainingKnownnessAndSliceGuards() {
    {
        Fixture fixture;
        fixture.input.otzKnown = false;
        CheckFailure(fixture.input, Failure::OtzUnknown);
    }
    {
        Fixture fixture;
        fixture.input.otShiftKnown = false;
        CheckFailure(fixture.input, Failure::OtShiftUnknown);
    }
    {
        Fixture fixture;
        fixture.input.otShift = 3u;
        CheckFailure(fixture.input, Failure::UnsupportedOtShift);
    }
    {
        Fixture fixture;
        fixture.input.highPriorityKnown = false;
        CheckFailure(fixture.input, Failure::HighPriorityUnknown);
    }
    {
        Fixture fixture;
        fixture.input.highPriority = true;
        CheckFailure(fixture.input, Failure::UnsupportedHighPriority);
    }
    {
        Fixture fixture;
        fixture.input.packetAllocatorKnown = false;
        CheckFailure(fixture.input, Failure::PacketAllocatorUnknown);
    }
    {
        Fixture fixture;
        fixture.input.orderingTable.headKnown = false;
        CheckFailure(fixture.input, Failure::OrderingTableHeadUnknown);
    }
    {
        Fixture fixture;
        fixture.input.orderingTable.lengthKnown = false;
        CheckFailure(fixture.input, Failure::OrderingTableLengthUnknown);
    }
    {
        Fixture fixture;
        fixture.object.primitives[0].rawPacketKnown = false;
        CheckFailure(fixture.input, Failure::RawPacketUnknown);
    }
}

} // namespace

int main() {
    TestExactSuccess();
    TestRequiredFailures();
    TestRemainingKnownnessAndSliceGuards();

    if (g_failedChecks != 0) {
        return 1;
    }
    std::printf("test_ss0_psx_tmd_submit: ok\n");
    return 0;
}
