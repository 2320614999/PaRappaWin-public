#include "pr/pr_stage_event_direct.h"

#include <cstdio>
#include <initializer_list>

namespace {

constexpr uint32_t kFlag80 = 0x00000080u;

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

PrStage1ScriptEvent Event(uint32_t frame,
                          uint32_t flags04,
                          uint8_t byte29,
                          uint8_t byte30) {
    PrStage1ScriptEvent event{};
    event.frame = frame;
    event.flags04 = flags04;
    event.byte1D = byte29;
    event.byte1E = byte30;
    return event;
}

PrStage1OverlayData MakeOverlay(
    std::initializer_list<PrStage1ScriptEvent> events) {
    PrStage1OverlayData data{};
    PrStage1EventStream stream{};
    stream.streamId = 1u;
    stream.count = static_cast<uint32_t>(events.size());
    stream.events.assign(events.begin(), events.end());
    data.streams.push_back(stream);
    data.valid = true;
    return data;
}

PrStageEventDirectStage1Runtime MakeRuntime(uint32_t baseFrame = 200u) {
    PrStageEventDirectStage1Runtime runtime{};
    PrStageEventDirectStage1Reset(runtime);
    runtime.dword801D3048 = baseFrame;
    runtime.unk801D2D64[1u].cursor = 0u;
    return runtime;
}

void TestRowAndStreamFlagGateReturnBeforeStreamRead() {
    PrStageEventDirectStage1Runtime runtime = MakeRuntime();
    runtime.gPrStageEventStreamFlag = 2u;
    const PrStage1OverlayData data = MakeOverlay({Event(100u, kFlag80, 1u, 2u)});

    const bool accepted =
        PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
            runtime, data, 100u, 100u, 3u);

    CHECK(!accepted);
    CHECK(runtime.lastClearTerminalBranchTriggerAttempted);
    CHECK(!runtime.lastClearTerminalBranchTriggerAccepted);
    CHECK(runtime.lastClearTerminalBranchTriggerBlockedFlagNotOne);
    CHECK(runtime.lastClearTerminalBranchTriggerBlockedRow);
    CHECK(runtime.clearTerminalBranchTriggerBlockedFlagAndRowCount == 1u);
    CHECK(runtime.clearTerminalBranchTriggerEligibleCount == 0u);
    CHECK(!runtime.lastClearTerminalBranchTriggerStream1DueKnown);
    CHECK(!runtime.clearTerminalBranchTriggerConsumed801C9094);
}

void TestDuePredicateBlocksBeforeFlag80Acceptance() {
    PrStageEventDirectStage1Runtime runtime = MakeRuntime(200u);
    const PrStage1OverlayData data = MakeOverlay({Event(140u, kFlag80, 7u, 8u)});

    const bool accepted =
        PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
            runtime, data, 120u, 120u, 1u);

    CHECK(!accepted);
    CHECK(runtime.clearTerminalBranchTriggerEligibleCount == 1u);
    CHECK(runtime.lastClearTerminalBranchTriggerBlockedEventNotDue);
    CHECK(!runtime.lastClearTerminalBranchTriggerBlockedMissingFlag80);
    CHECK(runtime.clearTerminalBranchTriggerBlockedEventNotDueCount == 1u);
    CHECK(runtime.clearTerminalBranchTriggerAcceptedCount == 0u);
    CHECK(!runtime.clearTerminalBranchTriggerConsumed801C9094);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1Known);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1DueFrame == 140u);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1DueDelta == 20);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1AbsDueFrame == 340u);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1AbsDueDelta == 220);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1Flags04 == kFlag80);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Known);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor == 0u);
    CHECK(runtime.firstClearTerminalBranchTriggerEventNotDueKnown);
    CHECK(runtime.firstClearTerminalBranchTriggerEventNotDueFrame == 140u);
}

void TestMissingFlag80BlocksCurrentEventEvenWhenNextFlag80Exists() {
    PrStageEventDirectStage1Runtime runtime = MakeRuntime(200u);
    const PrStage1OverlayData data =
        MakeOverlay({Event(100u, 0u, 11u, 12u), Event(135u, kFlag80, 13u, 14u)});

    const bool accepted =
        PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
            runtime, data, 120u, 120u, 1u);

    CHECK(!accepted);
    CHECK(runtime.clearTerminalBranchTriggerEligibleCount == 1u);
    CHECK(!runtime.lastClearTerminalBranchTriggerBlockedEventNotDue);
    CHECK(runtime.lastClearTerminalBranchTriggerBlockedMissingFlag80);
    CHECK(runtime.clearTerminalBranchTriggerBlockedMissingFlag80Count == 1u);
    CHECK(runtime.clearTerminalBranchTriggerAcceptedCount == 0u);
    CHECK(!runtime.clearTerminalBranchTriggerConsumed801C9094);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1Known);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1DueFrame == 100u);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1DueDelta == -20);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1Flags04 == 0u);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1Byte29 == 11u);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1Byte30 == 12u);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80SearchKnown);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Known);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Cursor == 1u);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Frame == 135u);
    CHECK(runtime.firstClearTerminalBranchTriggerEligibleStream1NextFlag80Delta == 15);
    CHECK(runtime.firstClearTerminalBranchTriggerMissingFlag80Known);
    CHECK(runtime.firstClearTerminalBranchTriggerMissingFlag80Frame == 100u);
}

void TestDueFlag80CurrentEventPublishesOneShotAcceptance() {
    PrStageEventDirectStage1Runtime runtime = MakeRuntime(200u);
    const PrStage1OverlayData data = MakeOverlay({Event(120u, kFlag80, 21u, 22u)});

    const bool firstAccepted =
        PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
            runtime, data, 120u, 120u, 0u);

    CHECK(firstAccepted);
    CHECK(runtime.lastClearTerminalBranchTriggerAccepted);
    CHECK(runtime.clearTerminalBranchTriggerAcceptedCount == 1u);
    CHECK(runtime.clearTerminalBranchTriggerConsumed801C9094);
    CHECK(!runtime.lastClearTerminalBranchTriggerBlockedEventNotDue);
    CHECK(!runtime.lastClearTerminalBranchTriggerBlockedMissingFlag80);

    const bool secondAccepted =
        PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
            runtime, data, 121u, 121u, 0u);

    CHECK(!secondAccepted);
    CHECK(!runtime.lastClearTerminalBranchTriggerAccepted);
    CHECK(runtime.lastClearTerminalBranchTriggerBlockedConsumed);
    CHECK(runtime.clearTerminalBranchTriggerBlockedConsumedCount == 1u);
    CHECK(runtime.clearTerminalBranchTriggerAcceptedCount == 1u);
}

} // namespace

int main() {
    TestRowAndStreamFlagGateReturnBeforeStreamRead();
    TestDuePredicateBlocksBeforeFlag80Acceptance();
    TestMissingFlag80BlocksCurrentEventEvenWhenNextFlag80Exists();
    TestDueFlag80CurrentEventPublishesOneShotAcceptance();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_stage1_clear_tail_trigger_gate: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_stage1_clear_tail_trigger_gate: ok\n");
    return 0;
}
