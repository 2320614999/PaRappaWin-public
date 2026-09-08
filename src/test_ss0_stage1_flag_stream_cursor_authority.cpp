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

PrStageEventDirectStage1Runtime MakeRuntime() {
    PrStageEventDirectStage1Runtime runtime{};
    PrStageEventDirectStage1Reset(runtime);
    runtime.gPrStageEventStreamFlag = 1u;
    runtime.dword801D3048 = 0u;
    runtime.unk801D2D64[1u].cursor = 0u;
    return runtime;
}

PrStageEventDirectStage1FrameInput MakeFrameInput(uint32_t queryFrame,
                                                  uint32_t scriptFrame,
                                                  bool allowSameQueryRefresh) {
    PrStageEventDirectStage1FrameInput input{};
    input.queryFrame = queryFrame;
    input.scriptFrame = scriptFrame;
    input.tick96 = static_cast<int32_t>(scriptFrame);
    input.allowSameQueryRefresh = allowSameQueryRefresh;
    input.rightRankActiveRow = 1u;
    input.currentMode = 1u;
    return input;
}

void TestClearTailTriggerDoesNotConsumeNoFlagCurrentEvent() {
    PrStageEventDirectStage1Runtime runtime = MakeRuntime();
    const PrStage1OverlayData data =
        MakeOverlay({Event(0u, 0u, 11u, 12u), Event(192u, kFlag80, 13u, 14u)});

    const bool accepted =
        PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
            runtime, data, 99u, 99u, 1u);

    CHECK(!accepted);
    CHECK(runtime.lastClearTerminalBranchTriggerBlockedMissingFlag80);
    CHECK(runtime.unk801D2D64[1u].cursor == 0u);

    PrStageRunnerDirectEventStreamCursor801C9094 cursor{};
    CHECK(PrStageEventDirectStage1BuildRunnerCursor801C9094(
        runtime, data, 1u, cursor));
    CHECK(cursor.valid);
    CHECK(cursor.index == 0u);
    CHECK(cursor.flags04 == 0u);
    CHECK(cursor.byte29 == 11u);
}

void TestProjectionRefreshDoesNotAdvanceFlagStreamCursor() {
    PrStageEventDirectStage1Runtime runtime = MakeRuntime();
    runtime.lastQueryFrame = 100;
    const PrStage1OverlayData data = MakeOverlay({Event(0u, kFlag80, 21u, 22u)});

    PrStageEventDirectStage1Advance(
        runtime,
        data,
        MakeFrameInput(100u, 100u, true));

    CHECK(runtime.unk801D2D64[1u].cursor == 0u);
    CHECK(!runtime.flagStreamEvent801C9094.valid);
}

void TestMutationFrameConsumesOnlyCurrentDueEvent() {
    PrStageEventDirectStage1Runtime runtime = MakeRuntime();
    const PrStage1OverlayData data =
        MakeOverlay({Event(0u, 0u, 31u, 32u), Event(192u, kFlag80, 33u, 34u)});

    PrStageEventDirectStage1Advance(
        runtime,
        data,
        MakeFrameInput(99u, 99u, false));

    CHECK(runtime.flagStreamEvent801C9094.valid);
    CHECK(runtime.flagStreamEvent801C9094.eventIndex == 0u);
    CHECK(runtime.flagStreamEvent801C9094.flags04 == 0u);
    CHECK(runtime.unk801D2D64[1u].cursor == 1u);

    const bool accepted =
        PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
            runtime, data, 99u, 99u, 1u);

    CHECK(!accepted);
    CHECK(runtime.lastClearTerminalBranchTriggerBlockedEventNotDue);
    CHECK(!runtime.lastClearTerminalBranchTriggerBlockedMissingFlag80);
    CHECK(runtime.lastClearTerminalBranchTriggerStream1Cursor == 1u);
    CHECK(runtime.lastClearTerminalBranchTriggerStream1DueFrame == 192u);
}

void TestFlag80CurrentEventAcceptedBeforeMutationConsumption() {
    PrStageEventDirectStage1Runtime runtime = MakeRuntime();
    runtime.unk801D2D64[1u].cursor = 1u;
    const PrStage1OverlayData data =
        MakeOverlay({Event(0u, 0u, 41u, 42u), Event(120u, kFlag80, 43u, 44u)});

    const bool accepted =
        PrStageEventDirectStage1ConsumeClearTerminalBranchTrigger801C9094(
            runtime, data, 120u, 120u, 1u);

    CHECK(accepted);
    CHECK(runtime.clearTerminalBranchTriggerConsumed801C9094);
    CHECK(runtime.lastClearTerminalBranchTriggerAccepted);
    CHECK(runtime.unk801D2D64[1u].cursor == 1u);

    PrStageEventDirectStage1Advance(
        runtime,
        data,
        MakeFrameInput(120u, 120u, false));

    CHECK(runtime.flagStreamEvent801C9094.valid);
    CHECK(runtime.flagStreamEvent801C9094.eventIndex == 1u);
    CHECK((runtime.flagStreamEvent801C9094.flags04 & kFlag80) != 0u);
    CHECK(runtime.unk801D2D64[1u].cursor == 2u);
}

void TestRouteTablesKeepStream4FailSideAndClearStreamsSeparate() {
    CHECK(PrStageEventDirectStage1TailStreamForMode(0u) == 3u);
    CHECK(PrStageEventDirectStage1TailStreamForMode(1u) == 2u);
    CHECK(PrStageEventDirectStage1TailStreamForMode(2u) == 4u);
    CHECK(PrStageEventDirectStage1TailStreamForMode(3u) == 4u);
    CHECK(PrStageEventDirectStage1TailStreamForMode(9u) == 4u);

    CHECK(PrStageEventDirectStage1Flag40StreamForMode(0u) == 5u);
    CHECK(PrStageEventDirectStage1Flag40StreamForMode(1u) == 4u);
    CHECK(PrStageEventDirectStage1Flag40StreamForMode(2u) == 4u);
    CHECK(PrStageEventDirectStage1Flag40StreamForMode(3u) == 4u);
    CHECK(PrStageEventDirectStage1Flag40StreamForMode(9u) == 4u);

    CHECK(PrStageEventDirectStage1IsFlag40Stream(4u));
    CHECK(PrStageEventDirectStage1IsFlag40Stream(5u));
    CHECK(!PrStageEventDirectStage1IsFlag40Stream(2u));
    CHECK(!PrStageEventDirectStage1IsFlag40Stream(3u));
    CHECK(PrStageEventDirectStage1IsTerminalStream(2u));
    CHECK(PrStageEventDirectStage1IsTerminalStream(3u));
    CHECK(PrStageEventDirectStage1IsTerminalStream(4u));
    CHECK(PrStageEventDirectStage1IsTerminalStream(5u));
}

} // namespace

int main() {
    TestClearTailTriggerDoesNotConsumeNoFlagCurrentEvent();
    TestProjectionRefreshDoesNotAdvanceFlagStreamCursor();
    TestMutationFrameConsumesOnlyCurrentDueEvent();
    TestFlag80CurrentEventAcceptedBeforeMutationConsumption();
    TestRouteTablesKeepStream4FailSideAndClearStreamsSeparate();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_stage1_flag_stream_cursor_authority: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_stage1_flag_stream_cursor_authority: ok\n");
    return 0;
}
