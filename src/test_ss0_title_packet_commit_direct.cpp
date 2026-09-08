#include "pr/pr_ss0_title_packet_commit_direct.h"

#include <cassert>
#include <cstdio>
#include <memory>

namespace {

using namespace PrSS0TitlePacketCommitDirect;

struct Fixture {
    PrSS0TitlePacketWorkDirect::RuntimeState801C609C packetWork{};
    PrPsxGraphOwnerDirect::PsxGraphState graph{};
    PrSS0TitlePacketPlanDirect::BuildResult800428B0 built{};
};

std::unique_ptr<Fixture> MakeFixture()
{
    auto fixture = std::make_unique<Fixture>();
    auto& packetWork = fixture->packetWork;
    packetWork.initialized = true;
    packetWork.packetArenaBase80025B28 = 0x801AE430u;
    packetWork.packetAllocatorBases801C956C = {
        0x801AE430u, 0x801B8CF0u};
    packetWork.framePrepared801C6410 = true;
    packetWork.currentDrawBuffer8004019C = 0u;
    packetWork.currentPacketAllocator800901C8 = 0x801AE430u;
    auto& work = packetWork.workLists801C9574[0];
    work.order_00 = 10u;
    work.headAddr_04 = 0x801C959Cu;
    work.lastAddr_10 = 0x801CA598u;
    work.tmdOtSlotMirrorKnown = true;
    work.tmdOtSlotMirror[120].valid = true;
    work.tmdOtSlotMirror[120].addr = 0x801C977Cu;
    work.tmdOtSlotMirror[120].value = 0x001C9778u;
    work.tmdPacketWriteMirrorKnown = true;

    fixture->graph.mainPageWorkLists80087288Initialized = true;
    fixture->graph.dword_800901C8 = 0x801AE430u;

    auto& built = fixture->built;
    built.ready = true;
    built.priority = 120u;
    built.orderingTableEnd = 0x801CA59Cu;
    auto& plan = built.plan;
    plan.readyToCommit = true;
    plan.packetWrite.marked = true;
    plan.packetWrite.address = 0x801AE430u;
    plan.packetWrite.words = {{
        0x071C9778u,
        0x24808080u,
        0x00070034u,
        0x48B5008Cu,
        0x0018002Au,
        0x000936B7u,
        0x00130029u,
        0x0000368Cu,
    }};
    plan.otDelta.marked = true;
    plan.otDelta.slotAddress = 0x801C977Cu;
    plan.otDelta.oldValue = 0x001C9778u;
    plan.otDelta.newValue = 0x001AE430u;
    plan.allocatorDelta.marked = true;
    plan.allocatorDelta.oldAddress = 0x801AE430u;
    plan.allocatorDelta.newAddress = 0x801AE450u;
    plan.allocatorDelta.advanceBytes = 32u;
    return fixture;
}

struct Snapshot {
    uint32_t graphAllocator = 0;
    uint32_t titleAllocator = 0;
    uint32_t otValue = 0;
    uint32_t packetCount = 0;
};

Snapshot TakeSnapshot(const Fixture& fixture)
{
    Snapshot out{};
    out.graphAllocator = fixture.graph.dword_800901C8;
    out.titleAllocator =
        fixture.packetWork.currentPacketAllocator800901C8;
    const auto& work = fixture.packetWork.workLists801C9574[0];
    out.otValue = work.tmdOtSlotMirror[120].value;
    for (const auto& write : work.tmdPacketWriteMirror) {
        if (write.valid) {
            ++out.packetCount;
        }
    }
    return out;
}

void CheckFailureAtomic(Failure expected, void (*mutate)(Fixture&))
{
    auto fixture = MakeFixture();
    mutate(*fixture);
    const Snapshot before = TakeSnapshot(*fixture);
    const auto result = CommitMode25PacketPlan800428B0(
        fixture->packetWork, fixture->graph, fixture->built);
    assert(!result.committed);
    assert(result.failure == expected);
    const Snapshot after = TakeSnapshot(*fixture);
    assert(after.graphAllocator == before.graphAllocator);
    assert(after.titleAllocator == before.titleAllocator);
    assert(after.otValue == before.otValue);
    assert(after.packetCount == before.packetCount);
}

} // namespace

int main()
{
    auto fixture = MakeFixture();
    const auto result = CommitMode25PacketPlan800428B0(
        fixture->packetWork, fixture->graph, fixture->built);
    assert(result.committed);
    assert(result.failure == Failure::None);
    assert(result.drawBuffer == 0u);
    assert(result.allocatorBefore == 0x801AE430u);
    assert(result.allocatorAfter == 0x801AE450u);
    assert(result.orderingTableSlotAddress == 0x801C977Cu);
    assert(result.packetAddress == 0x801AE430u);
    assert(fixture->graph.dword_800901C8 == 0x801AE450u);
    assert(fixture->packetWork.currentPacketAllocator800901C8 ==
           0x801AE450u);
    const auto& work = fixture->packetWork.workLists801C9574[0];
    assert(work.tmdOtSlotMirror[120].value == 0x001AE430u);
    assert(work.tmdPacketWriteMirror[0].valid);
    assert(work.tmdPacketWriteMirror[0].addr == 0x801AE430u);
    assert(work.tmdPacketWriteMirror[0].words ==
           fixture->built.plan.packetWrite.words);
    assert(!fixture->graph.mainPageWorkLists80087288[0]
                .work.tmdPacketWriteMirrorKnown);

    auto legacyPaKage = MakeFixture();
    const auto legacyResult =
        CommitFirstPaKageMode25PacketPlan800428B0(
            legacyPaKage->packetWork,
            legacyPaKage->graph,
            legacyPaKage->built);
    assert(legacyResult.committed);
    assert(legacyResult.failure == Failure::None);
    assert(legacyResult.allocatorAfter == result.allocatorAfter);

    CheckFailureAtomic(Failure::PlanNotReady, [](Fixture& f) {
        f.built.ready = false;
    });
    CheckFailureAtomic(Failure::PlanInconsistent, [](Fixture& f) {
        f.built.plan.packetWrite.words[0] ^= 1u;
    });
    CheckFailureAtomic(Failure::RuntimeNotReady, [](Fixture& f) {
        f.packetWork.framePrepared801C6410 = false;
    });
    CheckFailureAtomic(Failure::DrawBufferMismatch, [](Fixture& f) {
        f.graph.word_80096590 = 1u;
    });
    CheckFailureAtomic(Failure::WorkShapeMismatch, [](Fixture& f) {
        f.packetWork.workLists801C9574[0].lastAddr_10 -= 4u;
    });
    CheckFailureAtomic(Failure::AllocatorMismatch, [](Fixture& f) {
        f.graph.dword_800901C8 += 32u;
    });
    CheckFailureAtomic(Failure::PacketArenaRangeMismatch, [](Fixture& f) {
        constexpr uint32_t laneBytes =
            PrSS0TitlePacketWorkDirect::kPacketArenaLaneBytes801C5D28;
        const uint32_t base = 0x801AE430u - laneBytes + 16u;
        f.packetWork.packetArenaBase80025B28 = base;
        f.packetWork.packetAllocatorBases801C956C = {
            base, base + laneBytes};
    });
    CheckFailureAtomic(Failure::OrderingTableMirrorUnknown, [](Fixture& f) {
        f.packetWork.workLists801C9574[0].tmdOtSlotMirrorKnown = false;
    });
    CheckFailureAtomic(Failure::OrderingTableSlotNotFound, [](Fixture& f) {
        f.packetWork.workLists801C9574[0]
            .tmdOtSlotMirror[120].valid = false;
    });
    CheckFailureAtomic(Failure::OrderingTableSlotMismatch, [](Fixture& f) {
        ++f.packetWork.workLists801C9574[0]
              .tmdOtSlotMirror[120].value;
    });
    CheckFailureAtomic(Failure::PacketMirrorUnknown, [](Fixture& f) {
        f.packetWork.workLists801C9574[0]
            .tmdPacketWriteMirrorKnown = false;
    });
    CheckFailureAtomic(Failure::PacketAddressAlreadyWritten, [](Fixture& f) {
        auto& write = f.packetWork.workLists801C9574[0]
                          .tmdPacketWriteMirror[0];
        write.valid = true;
        write.addr = f.built.plan.packetWrite.address;
    });
    CheckFailureAtomic(Failure::PacketCapacityExceeded, [](Fixture& f) {
        auto& writes =
            f.packetWork.workLists801C9574[0].tmdPacketWriteMirror;
        for (std::size_t index = 0; index < writes.size(); ++index) {
            writes[index].valid = true;
            writes[index].addr = 0x80001000u +
                static_cast<uint32_t>(index) * 32u;
        }
    });

    std::printf("test_ss0_title_packet_commit_direct: ok\n");
    return 0;
}
