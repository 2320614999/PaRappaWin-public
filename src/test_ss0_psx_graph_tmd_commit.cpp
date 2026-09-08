#include "pr/pr_psx_graph_owner_direct.h"

#include <cstdio>

namespace {

using namespace PrPsxGraphOwnerDirect;

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,  \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

struct Fixture {
    TmdObject object{};
    PsxGraphState graph{};
    PrPsxTmdSubmitDirect::Result plan{};
    uint8_t pageIndex = 0;
    std::size_t otSlotIndex = 5u;

    Fixture() {
        object.vertices.resize(3u);
        object.primitives.resize(1u);
        TmdPrimitive& primitive = object.primitives[0];
        primitive.rawPacketKnown = true;
        primitive.rawPacketByteSize = 28u;
        primitive.mode = 0x25u;
        primitive.ilen = 6u;
        primitive.textured = true;
        primitive.r = 0x11u;
        primitive.g = 0x22u;
        primitive.b = 0x33u;
        primitive.v0_idx = 0u;
        primitive.v1_idx = 1u;
        primitive.v2_idx = 2u;

        PsxInitializeGraphState8003FB9C(graph, 320u, 240u);
        PsxCall8001E374_ClearMainPageWork(graph, pageIndex);
        graph.dword_800901C8 = 0x80001000u;
        const PsxGraphWorkList80040CC8& work =
            graph.mainPageWorkLists80087288[pageIndex].work;
        CHECK(work.tmdOtSlotMirrorKnown);
        CHECK(work.tmdPacketWriteMirrorKnown);
        CHECK(work.tmdOtSlotMirror[otSlotIndex].valid);

        PrPsxTmdSubmitDirect::Input input{};
        input.object = &object;
        input.sourceParseComplete = true;
        input.projectedSxy = {{{true, 0x00100010u},
                               {true, 0x00200020u},
                               {true, 0x00300030u}}};
        input.visibilityKnown = true;
        input.visible = true;
        input.otzKnown = true;
        input.otz = static_cast<uint16_t>(otSlotIndex << 4u);
        input.otShiftKnown = true;
        input.otShift = PrPsxTmdSubmitDirect::kRequiredOtShift;
        input.highPriorityKnown = true;
        input.packetAllocatorKnown = true;
        input.packetAllocatorAddr = graph.dword_800901C8;
        input.orderingTable.headKnown = true;
        input.orderingTable.head = work.headAddr_04 + 64u * 4u;
        input.orderingTable.lengthKnown = true;
        input.orderingTable.length = 64u;
        input.orderingTable.slotOldValueKnown = true;
        input.orderingTable.slotOldValue =
            work.tmdOtSlotMirror[otSlotIndex].value;
        plan = PrPsxTmdSubmitDirect::Build(input);
        CHECK(plan.readyToCommit);
        CHECK(plan.otDelta.slotAddress ==
              work.tmdOtSlotMirror[otSlotIndex].addr);
    }
};

struct Snapshot {
    uint32_t allocator = 0;
    uint32_t otValue = 0;
    uint32_t packetCount = 0;
};

Snapshot TakeSnapshot(const Fixture& fixture) {
    Snapshot out{};
    out.allocator = fixture.graph.dword_800901C8;
    const PsxGraphWorkList80040CC8& work =
        fixture.graph.mainPageWorkLists80087288[fixture.pageIndex].work;
    out.otValue = work.tmdOtSlotMirror[fixture.otSlotIndex].value;
    for (const PsxTmdPacketWrite& write : work.tmdPacketWriteMirror) {
        if (write.valid) {
            ++out.packetCount;
        }
    }
    return out;
}

void CheckSnapshot(const Fixture& fixture, const Snapshot& expected) {
    const Snapshot actual = TakeSnapshot(fixture);
    CHECK(actual.allocator == expected.allocator);
    CHECK(actual.otValue == expected.otValue);
    CHECK(actual.packetCount == expected.packetCount);
}

void TestExactCommit() {
    Fixture fixture;
    const PsxTmdCommitResult result = CommitTmdResultToMainPageWork(
        fixture.graph, fixture.pageIndex, fixture.plan);
    CHECK(result.committed);
    CHECK(result.failure == PsxTmdCommitFailure::None);
    CHECK(result.allocatorBefore == 0x80001000u);
    CHECK(result.allocatorAfter == 0x80001020u);
    const PsxGraphWorkList80040CC8& work =
        fixture.graph.mainPageWorkLists80087288[fixture.pageIndex].work;
    CHECK(work.tmdOtSlotMirror[fixture.otSlotIndex].value ==
          fixture.plan.otDelta.newValue);
    CHECK(work.tmdPacketWriteMirror[0].valid);
    CHECK(work.tmdPacketWriteMirror[0].addr ==
          fixture.plan.packetWrite.address);
    CHECK(work.tmdPacketWriteMirror[0].words ==
          fixture.plan.packetWrite.words);
}

template <typename Mutate>
void CheckFailureAtomic(PsxTmdCommitFailure expected, Mutate mutate) {
    Fixture fixture;
    mutate(fixture);
    const Snapshot before = TakeSnapshot(fixture);
    const PsxTmdCommitResult result = CommitTmdResultToMainPageWork(
        fixture.graph, fixture.pageIndex, fixture.plan);
    CHECK(!result.committed);
    CHECK(result.failure == expected);
    CheckSnapshot(fixture, before);
}

void TestFailClosedAtomicity() {
    CheckFailureAtomic(PsxTmdCommitFailure::PlanNotReady,
        [](Fixture& f) { f.plan.readyToCommit = false; });
    CheckFailureAtomic(PsxTmdCommitFailure::GraphUninitialized,
        [](Fixture& f) { f.graph.mainPageWorkLists80087288Initialized = false; });
    CheckFailureAtomic(PsxTmdCommitFailure::PlanInconsistent,
        [](Fixture& f) { f.plan.packetWrite.words[0] ^= 1u; });
    CheckFailureAtomic(PsxTmdCommitFailure::AllocatorMismatch,
        [](Fixture& f) { f.graph.dword_800901C8 += 4u; });
    CheckFailureAtomic(PsxTmdCommitFailure::OtMirrorUnknown,
        [](Fixture& f) {
            f.graph.mainPageWorkLists80087288[0].work.tmdOtSlotMirrorKnown =
                false;
        });
    CheckFailureAtomic(PsxTmdCommitFailure::OtSlotNotFound,
        [](Fixture& f) { f.plan.otDelta.slotAddress = 0x1234u; });
    CheckFailureAtomic(PsxTmdCommitFailure::OtSlotMismatch,
        [](Fixture& f) {
            ++f.graph.mainPageWorkLists80087288[0]
                  .work.tmdOtSlotMirror[f.otSlotIndex].value;
        });
    CheckFailureAtomic(PsxTmdCommitFailure::PacketMirrorUnknown,
        [](Fixture& f) {
            f.graph.mainPageWorkLists80087288[0]
                .work.tmdPacketWriteMirrorKnown = false;
        });
    CheckFailureAtomic(PsxTmdCommitFailure::PacketAddressAlreadyWritten,
        [](Fixture& f) {
            PsxTmdPacketWrite& write = f.graph.mainPageWorkLists80087288[0]
                                           .work.tmdPacketWriteMirror[0];
            write.valid = true;
            write.addr = f.plan.packetWrite.address;
        });
    CheckFailureAtomic(PsxTmdCommitFailure::PacketCapacityExceeded,
        [](Fixture& f) {
            auto& writes = f.graph.mainPageWorkLists80087288[0]
                               .work.tmdPacketWriteMirror;
            for (std::size_t i = 0; i < writes.size(); ++i) {
                writes[i].valid = true;
                writes[i].addr = 0x81000000u + static_cast<uint32_t>(i * 4u);
            }
        });
}

} // namespace

int main() {
    TestExactCommit();
    TestFailClosedAtomicity();
    if (g_failedChecks != 0) {
        return 1;
    }
    std::printf("test_ss0_psx_graph_tmd_commit: ok\n");
    return 0;
}
