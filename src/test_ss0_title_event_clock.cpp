#include "pr/pr_ss0_title_hud_events_direct.h"
#include "pr/pr_ss0_title_tmd_backend.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <limits>
#include <vector>

namespace {

int g_failedChecks = 0;

#define CHECK(expr)                                                       \
    do {                                                                  \
        if (!(expr)) {                                                    \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__, \
                        #expr);                                           \
            ++g_failedChecks;                                             \
        }                                                                 \
    } while (0)

void CheckKnown(int32_t loopTickV10, uint32_t expectedTick96)
{
    const auto clock =
        PrSS0TitleHudEventsDirect::ComputeTitleEventClock801C4894(
            loopTickV10, false, 0u);
    CHECK(clock.known);
    CHECK(clock.loopTickV10 == loopTickV10);
    CHECK(clock.tick96 == expectedTick96);
    CHECK(clock.beatKnown);
    const uint32_t tickInBar = expectedTick96 % 384u;
    CHECK(clock.beat == static_cast<uint8_t>(tickInBar / 96u + 1u));
    CHECK(clock.tickWithinBeat ==
          static_cast<uint8_t>(tickInBar % 96u + 1u));
}

PrSS0TitleHudEventsDirect::TitleAbsoluteEventTickInput801C5538
MakeAbsoluteInput(uint32_t tick96)
{
    PrSS0TitleHudEventsDirect::TitleAbsoluteEventTickInput801C5538 input{};
    input.ctxTick96Known = true;
    input.ctxTick96 = tick96;
    input.streamFlagKnown = true;
    input.streamFlag = 1u;
    input.streamIdKnown = true;
    input.streamId = 0u;
    return input;
}

PrSS0TitleHudEventsDirect::TitleHudTimelineTickInput801C5094
MakeHudTimelineInput(uint32_t tick96, uint32_t channel = 0u)
{
    PrSS0TitleHudEventsDirect::TitleHudTimelineTickInput801C5094 input{};
    input.channelKnown = true;
    input.channel = channel;
    input.currentTick96Known = true;
    input.currentTick96 = tick96;
    return input;
}

PrSS0TitleHudEventsDirect::TitleSelectorHudTickInput801C5854
MakeSelectorHudInput(uint32_t tick96,
                     uint8_t beat,
                     uint8_t cursor = 0u)
{
    PrSS0TitleHudEventsDirect::TitleSelectorHudTickInput801C5854 input{};
    input.currentTick96Known = true;
    input.currentTick96 = tick96;
    input.beatKnown = true;
    input.beat = beat;
    input.cursorKnown = true;
    input.cursor = cursor;
    return input;
}

PrSS0TitleHudEventsDirect::TitleEventRuntimeState801C5190
MakeHudTimelineState(uint8_t slotId,
                     uint32_t baseTick96,
                     uint32_t cursor)
{
    PrSS0TitleHudEventsDirect::TitleEventRuntimeState801C5190 state{};
    state.hudSlot0Known = true;
    state.hudSlot0 = slotId;
    state.hudSlot0BaseTick96Known = true;
    state.hudSlot0BaseTick96 = baseTick96;
    state.hudTimelineChannel0.descriptorAvailable = true;
    state.hudTimelineChannel0.cursorKnown = true;
    state.hudTimelineChannel0.cursor = cursor;
    return state;
}

void CheckAppliedHudTimRequest(
    const PrSS0TitleHudEventsDirect::TitleHudTimelineTickResult801C5094&
        result,
    uint8_t slotId,
    uint32_t recordIndex,
    uint32_t baseTick96,
    uint32_t appliedTick96,
    const int16_t* expectedTimIds,
    uint32_t expectedTimIdCount)
{
    using namespace PrSS0TitleHudEventsDirect;
    CHECK(result.status == TitleHudTimelineTickStatus801C5094::Applied);
    CHECK(result.request.valid);
    CHECK(result.request.slotId == slotId);
    CHECK(result.request.recordIndex == recordIndex);
    CHECK(result.request.baseTick96 == baseTick96);
    CHECK(result.request.appliedTick96 == appliedTick96);
    CHECK(result.request.timIdCount == expectedTimIdCount);
    CHECK(result.nextState.hudTimelineChannel0.cursor == recordIndex + 1u);
    CHECK((result.nextState.ctxFlags & kCtxFlagRuntimeTimList) != 0u);
    CHECK(result.nextState.hudTimelineChannel0.emittedTimIdCount ==
          expectedTimIdCount);
    for (uint32_t i = 0; i < kHudTimelineMaxTimIds; ++i) {
        const int16_t expected =
            i < expectedTimIdCount ? expectedTimIds[i] : 0;
        CHECK(result.request.timIds[i] == expected);
        CHECK(result.nextState.hudTimelineChannel0.emittedTimIds[i] ==
              expected);
    }
}

void CheckExactHudTimelineTable()
{
    using namespace PrSS0TitleHudEventsDirect;
    static const TitleHudTimelineRecord801C5094 slot1[] = {
        {0u, {29, 0, 0, 0}},
        {24u, {30, 0, 0, 0}},
        {48u, {28, 0, 0, 0}},
        {72u, {29, 0, 0, 0}},
        {96u, {30, 0, 0, 0}},
        {120u, {37, 45, 0, 0}},
        {192u, {33, 41, 28, 0}},
    };
    static const TitleHudTimelineRecord801C5094 slot2[] = {
        {0u, {34, 42, 0, 0}},
        {24u, {33, 41, 0, 0}},
    };
    static const TitleHudTimelineRecord801C5094 slot3[] = {
        {0u, {60, 54, 0, 0}},
        {0u, {55, 49, 0, 0}},
        {0u, {34, 42, 0, 0}},
        {24u, {33, 41, 0, 0}},
    };
    static const TitleHudTimelineRecord801C5094 slot4[] = {
        {0u, {57, 51, 0, 0}},
        {0u, {58, 52, 0, 0}},
        {0u, {34, 42, 0, 0}},
        {24u, {33, 41, 0, 0}},
    };
    static const TitleHudTimelineRecord801C5094 slot5[] = {
        {0u, {57, 51, 0, 0}},
        {0u, {58, 52, 0, 0}},
        {0u, {33, 41, 0, 0}},
    };
    static const TitleHudTimelineRecord801C5094 slot6[] = {
        {0u, {60, 54, 0, 0}},
        {0u, {55, 49, 0, 0}},
        {0u, {33, 41, 0, 0}},
    };
    struct ExpectedSlot {
        const TitleHudTimelineRecord801C5094* records;
        uint32_t count;
        uint32_t eventsPtr;
    };
    static const ExpectedSlot expected[] = {
        {nullptr, 0u, 0x00000000u},
        {slot1, 7u, 0x801C6C38u},
        {slot2, 2u, 0x801C6C8Cu},
        {slot3, 4u, 0x801C6CA4u},
        {slot4, 4u, 0x801C6CD4u},
        {slot5, 3u, 0x801C6D04u},
        {slot6, 3u, 0x801C6D28u},
    };

    CHECK(sizeof(TitleHudTimelineRecord801C5094) ==
          kHudTimelineRecordBytes);
    CHECK(KnownHudTimelineDescriptorCount() == 7u);
    for (uint32_t slotId = 0; slotId < 7u; ++slotId) {
        const auto descriptor = KnownHudTimelineDescriptorAt(slotId);
        CHECK(descriptor.eventsPtr == expected[slotId].eventsPtr);
        CHECK(descriptor.count == expected[slotId].count);
        CHECK(descriptor.cursorInitial == 0u);
        CHECK(KnownHudTimelineRecordCount(slotId) ==
              expected[slotId].count);
        for (uint32_t recordIndex = 0;
             recordIndex < expected[slotId].count;
             ++recordIndex) {
            const auto actual =
                KnownHudTimelineRecordAt(slotId, recordIndex);
            const auto& wanted = expected[slotId].records[recordIndex];
            CHECK(actual.deltaFrame == wanted.deltaFrame);
            for (uint32_t i = 0; i < kHudTimelineMaxTimIds; ++i) {
                CHECK(actual.timIds[i] == wanted.timIds[i]);
            }
        }
    }

    CHECK(KnownHudTimelineRecordCount(7u) == 0u);
    const auto nullRecord = KnownHudTimelineRecordAt(0u, 0u);
    CHECK(nullRecord.deltaFrame == 0u);
    for (uint32_t i = 0; i < kHudTimelineMaxTimIds; ++i) {
        CHECK(nullRecord.timIds[i] == 0);
    }
}

void CheckSlot1HudTimelineTiming()
{
    using namespace PrSS0TitleHudEventsDirect;
    constexpr uint32_t baseTick96 = 1000u;
    auto state = MakeHudTimelineState(1u, baseTick96, 0u);
    state.ctxFlags = 0x40u;

    for (uint32_t recordIndex = 0;
         recordIndex < KnownHudTimelineRecordCount(1u);
         ++recordIndex) {
        const auto record = KnownHudTimelineRecordAt(1u, recordIndex);
        const uint32_t threshold = baseTick96 + record.deltaFrame;
        if (record.deltaFrame != 0u) {
            const auto notDue = TickTitleHudTimeline801C5094(
                state, MakeHudTimelineInput(threshold - 1u));
            CHECK(notDue.status ==
                  TitleHudTimelineTickStatus801C5094::NotDue);
            CHECK(!notDue.request.valid);
            CHECK(notDue.nextState.hudTimelineChannel0.cursor ==
                  recordIndex);
            CHECK(notDue.nextState.ctxFlags == state.ctxFlags);
        }

        const auto applied = TickTitleHudTimeline801C5094(
            state, MakeHudTimelineInput(threshold));
        uint32_t expectedCount = 0u;
        while (expectedCount < kHudTimelineMaxTimIds &&
               record.timIds[expectedCount] != 0) {
            ++expectedCount;
        }
        CheckAppliedHudTimRequest(applied, 1u, recordIndex, baseTick96,
                                 threshold, record.timIds, expectedCount);
        CHECK(applied.nextState.ctxFlags ==
              (state.ctxFlags | kCtxFlagRuntimeTimList));
        state = applied.nextState;
    }
}

void CheckConsecutiveZeroDeltaHudRecords()
{
    using namespace PrSS0TitleHudEventsDirect;
    constexpr uint32_t baseTick96 = 500u;
    auto state = MakeHudTimelineState(3u, baseTick96, 0u);

    for (uint32_t recordIndex = 0; recordIndex < 3u; ++recordIndex) {
        const auto record = KnownHudTimelineRecordAt(3u, recordIndex);
        const auto applied = TickTitleHudTimeline801C5094(
            state, MakeHudTimelineInput(baseTick96));
        CheckAppliedHudTimRequest(applied, 3u, recordIndex, baseTick96,
                                 baseTick96, record.timIds, 2u);
        state = applied.nextState;
    }

    const auto notDue = TickTitleHudTimeline801C5094(
        state, MakeHudTimelineInput(baseTick96));
    CHECK(notDue.status == TitleHudTimelineTickStatus801C5094::NotDue);
    CHECK(notDue.nextState.hudTimelineChannel0.cursor == 3u);
    CHECK(!notDue.request.valid);

    const auto finalRecord = KnownHudTimelineRecordAt(3u, 3u);
    const auto finalApplied = TickTitleHudTimeline801C5094(
        state, MakeHudTimelineInput(baseTick96 + 24u));
    CheckAppliedHudTimRequest(finalApplied, 3u, 3u, baseTick96,
                             baseTick96 + 24u, finalRecord.timIds, 2u);
}

void CheckHudTimelineFailClosedAndCompletion()
{
    using namespace PrSS0TitleHudEventsDirect;
    auto state = MakeHudTimelineState(1u, 100u, 1u);
    state.ctxFlags = 0x1234u;
    state.hudTimelineChannel0.emittedTimIds[0] = 77;
    state.hudTimelineChannel0.emittedTimIdCount = 1u;
    auto input = MakeHudTimelineInput(123u);

    const auto notDue = TickTitleHudTimeline801C5094(state, input);
    CHECK(notDue.status == TitleHudTimelineTickStatus801C5094::NotDue);
    CHECK(notDue.nextState.ctxFlags == 0x1234u);
    CHECK(notDue.nextState.hudTimelineChannel0.cursor == 1u);
    CHECK(notDue.nextState.hudTimelineChannel0.emittedTimIds[0] == 77);
    CHECK(!notDue.request.valid);

    auto unknownInput = input;
    unknownInput.currentTick96Known = false;
    auto failed = TickTitleHudTimeline801C5094(state, unknownInput);
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::SourceUnknown);

    auto unknownState = state;
    unknownState.hudSlot0Known = false;
    failed = TickTitleHudTimeline801C5094(unknownState, input);
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::SourceUnknown);
    unknownState = state;
    unknownState.hudSlot0BaseTick96Known = false;
    failed = TickTitleHudTimeline801C5094(unknownState, input);
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::SourceUnknown);
    unknownState = state;
    unknownState.hudTimelineChannel0.cursorKnown = false;
    failed = TickTitleHudTimeline801C5094(unknownState, input);
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::SourceUnknown);

    auto unknownChannelInput = input;
    unknownChannelInput.channelKnown = false;
    failed = TickTitleHudTimeline801C5094(state, unknownChannelInput);
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::SourceUnknown);
    failed = TickTitleHudTimeline801C5094(
        state, MakeHudTimelineInput(124u, 1u));
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::UnsupportedChannel);

    auto invalidState = MakeHudTimelineState(0u, 100u, 0u);
    failed = TickTitleHudTimeline801C5094(invalidState, input);
    CHECK(failed.status == TitleHudTimelineTickStatus801C5094::InvalidSlot);
    invalidState = MakeHudTimelineState(7u, 100u, 0u);
    failed = TickTitleHudTimeline801C5094(invalidState, input);
    CHECK(failed.status == TitleHudTimelineTickStatus801C5094::InvalidSlot);
    invalidState = MakeHudTimelineState(1u, 100u, 0u);
    invalidState.hudTimelineChannel0.descriptorAvailable = false;
    failed = TickTitleHudTimeline801C5094(invalidState, input);
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::DescriptorUnavailable);
    invalidState = MakeHudTimelineState(1u, 100u, 8u);
    failed = TickTitleHudTimeline801C5094(invalidState, input);
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::InvalidCursor);

    const uint32_t overflowBase =
        std::numeric_limits<uint32_t>::max() - 12u;
    invalidState = MakeHudTimelineState(1u, overflowBase, 1u);
    failed = TickTitleHudTimeline801C5094(
        invalidState,
        MakeHudTimelineInput(std::numeric_limits<uint32_t>::max()));
    CHECK(failed.status ==
          TitleHudTimelineTickStatus801C5094::ThresholdOverflow);
    CHECK(failed.nextState.hudSlot0BaseTick96 == overflowBase);
    CHECK(failed.nextState.hudTimelineChannel0.cursor == 1u);
    CHECK(!failed.request.valid);

    auto completeState = MakeHudTimelineState(2u, 444u, 2u);
    completeState.ctxFlags = 0xA58000u;
    completeState.hudTimelineChannel0.emittedTimIds[0] = 33;
    completeState.hudTimelineChannel0.emittedTimIds[1] = 41;
    completeState.hudTimelineChannel0.emittedTimIdCount = 2u;
    const auto complete = TickTitleHudTimeline801C5094(
        completeState, MakeHudTimelineInput(999u));
    CHECK(complete.status == TitleHudTimelineTickStatus801C5094::Complete);
    CHECK(!complete.request.valid);
    CHECK(!complete.nextState.hudSlot0Known);
    CHECK(complete.nextState.hudSlot0 == 0u);
    CHECK(complete.nextState.hudSlot0BaseTick96Known);
    CHECK(complete.nextState.hudSlot0BaseTick96 == 444u);
    CHECK(!complete.nextState.hudTimelineChannel0.descriptorAvailable);
    CHECK(complete.nextState.hudTimelineChannel0.cursorKnown);
    CHECK(complete.nextState.hudTimelineChannel0.cursor == 0u);
    CHECK(complete.nextState.hudTimelineChannel0.emittedTimIdCount == 2u);
    CHECK(complete.nextState.hudTimelineChannel0.emittedTimIds[0] == 33);
    CHECK(complete.nextState.hudTimelineChannel0.emittedTimIds[1] == 41);
    CHECK(complete.nextState.ctxFlags == 0xA58000u);
}

void CheckHudSlotCommitResetsTimelineCursor()
{
    using namespace PrSS0TitleHudEventsDirect;
    auto state = MakeHudTimelineState(6u, 400u, 3u);
    state.ctxFlags = 0x40u;
    state.hudTimelineChannel0.descriptorAvailable = false;
    state.hudTimelineChannel0.emittedTimIds[0] = 60;
    state.hudTimelineChannel0.emittedTimIds[1] = 54;
    state.hudTimelineChannel0.emittedTimIdCount = 2u;

    auto effect = DecodeTitleEventEffect801C5190(2u);
    CHECK(effect.known);
    CHECK(effect.hudSlot0Write);
    CHECK(effect.hudSlot0 == 2u);
    effect.appliedTick96Known = true;
    effect.appliedTick96 = 700u;
    const auto commit = BuildTitleEventRuntimeCommit801C5190(state, effect);
    CHECK(commit.accepted);
    CHECK(commit.nextState.hudSlot0Known);
    CHECK(commit.nextState.hudSlot0 == 2u);
    CHECK(commit.nextState.hudSlot0BaseTick96Known);
    CHECK(commit.nextState.hudSlot0BaseTick96 == 700u);
    CHECK(commit.nextState.hudTimelineChannel0.descriptorAvailable);
    CHECK(commit.nextState.hudTimelineChannel0.cursorKnown);
    CHECK(commit.nextState.hudTimelineChannel0.cursor == 0u);
    CHECK(commit.nextState.hudTimelineChannel0.emittedTimIdCount == 2u);
    CHECK(commit.nextState.hudTimelineChannel0.emittedTimIds[0] == 60);
    CHECK(commit.nextState.hudTimelineChannel0.emittedTimIds[1] == 54);
}

void CheckSelectorResourceUpdate801C5854(
    const PrSS0TitleHudEventsDirect::TitleSelectorHudTickResult801C5854&
        result,
    uint32_t selectorIndex,
    int16_t idA,
    int16_t idB,
    uint32_t tick96)
{
    CHECK(result.selectorResourceUpdate.known);
    CHECK(result.selectorResourceUpdate.selectorIndex == selectorIndex);
    CHECK(result.selectorResourceUpdate.idA == idA);
    CHECK(result.selectorResourceUpdate.idB == idB);
    CHECK(result.selectorResourceUpdate.targetOffsetA ==
          PrSS0TitleHudEventsDirect::kCtxSelectorResourceOffsetDC);
    CHECK(result.selectorResourceUpdate.targetOffsetB ==
          PrSS0TitleHudEventsDirect::kCtxSelectorResourceOffsetE8);
    CHECK(result.selectorResourceUpdate.appliedTick96Known);
    CHECK(result.selectorResourceUpdate.appliedTick96 == tick96);
}

void CheckComod0HudTableImport()
{
    std::ifstream input("..\\S0\\COMOD0.BIN", std::ios::binary);
    CHECK(static_cast<bool>(input));
    if (!input) {
        return;
    }
    input.seekg(0, std::ios::end);
    const std::streamoff length = input.tellg();
    input.seekg(0, std::ios::beg);
    CHECK(length > 0);
    if (length <= 0) {
        return;
    }
    std::vector<uint8_t> bytes(static_cast<std::size_t>(length));
    input.read(reinterpret_cast<char*>(bytes.data()), length);
    CHECK(static_cast<bool>(input));
    if (!input) {
        return;
    }

    using namespace PrSS0TitleHudEventsDirect;
    CHECK(LoadTitleHudTablesFromComod0(
        bytes.data(), bytes.size(), kComod0MappedBase801C3870));
    const auto& source = GetTitleHudComodSourceState801C6Dxx();
    CHECK(source.loaded);
    CHECK(source.descriptorKnown);
    CHECK(source.eventStreamKnown && source.eventStreamCount == 7u);
    CHECK(source.resourcePairsKnown && source.resourcePairCount == 78u);
    CHECK(source.timelineKnown && source.timelineDescriptorCount == 7u);
    CHECK(source.selectorKnown && source.selectorCount == 4u);
    CHECK(KnownEventStreamRecordAt(0u).frame == 192u);
    CHECK(KnownEventStreamRecordAt(6u).frame == 1920u);
    CHECK(KnownHudTimelineRecordCount(1u) == 7u);
    CHECK(KnownHudTimelineRecordAt(1u, 5u).timIds[0] == 37);
    CHECK(KnownSelectorRecordAt(0u).idA == 25);
    CHECK(KnownSelectorResourcePairIndexAt(0u) == 7u);
    CHECK(KnownSelectorResourcePairIndexAt(1u) == 8u);
    CHECK(KnownSelectorResourcePairIndexAt(2u) == 4u);
    CHECK(KnownSelectorResourcePairIndexAt(3u) == 6u);
    ResetTitleHudComodSource801C6Dxx();
}

void CheckTitleSelectorHudStartAndFirstTick()
{
    using namespace PrSS0TitleHudEventsDirect;

    static const uint8_t expectedPairIndices[] = {7u, 8u, 4u, 6u};
    CHECK(KnownSelectorRecordCount() == 4u);
    for (uint32_t index = 0u; index < 4u; ++index) {
        CHECK(KnownSelectorResourcePairIndexAt(index) ==
              expectedPairIndices[index]);
    }
    CHECK(KnownSelectorResourcePairIndexAt(4u) == 0u);

    TitleEventRuntimeState801C5190 eventState{};
    eventState.ctxFlags = 0x20u;
    TitleSelectorHudRuntimeState801C5854 selectorState{};
    selectorState.resourceCooldownKnown = true;
    selectorState.resourceCooldown = 0;

    const auto start = StartTitleSelectorHud801C57E0(
        eventState, selectorState, true, 2012u);
    CHECK(start.accepted);
    CHECK(start.nextSelectorState.known);
    CHECK(start.nextSelectorState.bank ==
          kTitleSelectorInitialBank801C57E0);
    CHECK(start.nextSelectorState.lastCursor ==
          kTitleSelectorInitialLastCursor801C57E0);
    CHECK(start.nextSelectorState.timelineEnabled);
    CHECK(start.nextSelectorState.timelineCountdown ==
          kTitleSelectorInitialCountdown801C57E0);
    CHECK(start.nextSelectorState.resourceCooldownKnown);
    CHECK(start.nextSelectorState.resourceCooldown == 0);
    CHECK(start.nextEventState.hudSlot0Known);
    CHECK(start.nextEventState.hudSlot0 ==
          kTitleSelectorInitialHudSlot801C57E0);
    CHECK(start.nextEventState.hudSlot0BaseTick96Known);
    CHECK(start.nextEventState.hudSlot0BaseTick96 == 2012u);
    CHECK(start.nextEventState.hudTimelineChannel0.cursorKnown);
    CHECK(start.nextEventState.hudTimelineChannel0.cursor == 0u);

    const auto first = TickTitleSelectorHud801C5854(
        start.nextEventState,
        start.nextSelectorState,
        MakeSelectorHudInput(2012u, 1u));
    CHECK(first.status == TitleSelectorHudTickStatus801C5854::Applied);
    CHECK(first.selectorIndex == 2u);
    CHECK(first.request.valid);
    CHECK(first.request.slotId == 6u);
    CHECK(first.request.recordIndex == 0u);
    CHECK(first.request.timIdCount == 2u);
    CHECK(first.request.timIds[0] == 60);
    CHECK(first.request.timIds[1] == 54);
    CHECK(first.nextSelectorState.bank == 1u);
    CHECK(first.nextSelectorState.lastCursor == 0u);
    CHECK(first.nextSelectorState.timelineCountdown == 5);
    CHECK(first.nextSelectorState.resourceCooldown == 17);
    CHECK(first.selectorMimeEffect.known);
    CHECK(first.selectorMimeEffect.channel2PairWrite);
    CHECK(first.selectorMimeEffect.channel2PairIndex == 4u);
    CheckSelectorResourceUpdate801C5854(first, 2u, 22, 13, 2012u);
    CHECK(first.nextEventState.channel2PairKnown);
    CHECK(first.nextEventState.channel2PairIndex == 4u);
    CHECK((first.nextEventState.ctxFlags & kCtxFlagRuntimeTimList) != 0u);
    CHECK((first.nextEventState.ctxFlags & kCtxFlagSelectorMime) != 0u);

    const auto cursorChange = TickTitleSelectorHud801C5854(
        first.nextEventState,
        first.nextSelectorState,
        MakeSelectorHudInput(2012u, 1u, 1u));
    CHECK(cursorChange.status ==
          TitleSelectorHudTickStatus801C5854::Applied);
    CHECK(cursorChange.selectorIndex == 1u);
    CHECK(cursorChange.nextSelectorState.bank == 1u);
    CHECK(cursorChange.nextSelectorState.lastCursor == 0u);
    CHECK(cursorChange.nextSelectorState.resourceCooldown == 16);
    CHECK(!cursorChange.selectorMimeEffect.known);
    CHECK(!cursorChange.selectorResourceUpdate.known);
    CHECK(!cursorChange.selectorResourceUpdate.appliedTick96Known);
    CHECK(cursorChange.request.valid);
    CHECK(cursorChange.request.slotId == 5u);
    CHECK(cursorChange.request.timIds[0] == 57);
    CHECK(cursorChange.request.timIds[1] == 51);

    const auto beat3Start = StartTitleSelectorHud801C57E0(
        eventState, selectorState, true, 192u);
    CHECK(beat3Start.accepted);
    const auto beat3 = TickTitleSelectorHud801C5854(
        beat3Start.nextEventState,
        beat3Start.nextSelectorState,
        MakeSelectorHudInput(192u, 3u));
    CHECK(beat3.status == TitleSelectorHudTickStatus801C5854::Applied);
    CHECK(!beat3.nextSelectorState.timelineEnabled);
    CHECK(beat3.nextSelectorState.timelineCountdown == 5);
    CHECK(!beat3.request.valid);
    CHECK(beat3.selectorMimeEffect.known);
    CHECK(beat3.selectorMimeEffect.channel2PairIndex == 4u);
    CheckSelectorResourceUpdate801C5854(beat3, 2u, 22, 13, 192u);

    auto unknownCooldown = selectorState;
    unknownCooldown.resourceCooldownKnown = false;
    const auto unknownStart = StartTitleSelectorHud801C57E0(
        eventState, unknownCooldown, true, 2012u);
    CHECK(unknownStart.accepted);
    const auto unknownTick = TickTitleSelectorHud801C5854(
        unknownStart.nextEventState,
        unknownStart.nextSelectorState,
        MakeSelectorHudInput(2012u, 1u));
    CHECK(unknownTick.status ==
          TitleSelectorHudTickStatus801C5854::SourceUnknown);

    auto negativeCooldown = selectorState;
    negativeCooldown.resourceCooldown = -1;
    const auto negativeStart = StartTitleSelectorHud801C57E0(
        eventState, negativeCooldown, true, 2012u);
    CHECK(negativeStart.accepted);
    const auto negativeTick = TickTitleSelectorHud801C5854(
        negativeStart.nextEventState,
        negativeStart.nextSelectorState,
        MakeSelectorHudInput(2012u, 1u));
    CHECK(negativeTick.status ==
          TitleSelectorHudTickStatus801C5854::Applied);
    CHECK(negativeTick.selectorMimeEffect.known);
    CheckSelectorResourceUpdate801C5854(
        negativeTick, 2u, 22, 13, 2012u);
    CHECK(negativeTick.nextSelectorState.resourceCooldown == 17);

    const auto shortcutTransaction =
        BuildShortcutTitleSelectorHudStartAndFirstTick801C5AB4();
    CHECK(shortcutTransaction.reset80024E98Applied);
    CHECK(shortcutTransaction.firstTickCtxTick96 == 0u);
    CHECK(shortcutTransaction.firstTickBeatDerived == 0u);
    CHECK(shortcutTransaction.firstTickCursor == 0u);
    CHECK(shortcutTransaction.callerReadyFlagApplied);
    const auto& shortcut = shortcutTransaction.firstTick;
    CHECK(shortcut.status ==
          TitleSelectorHudTickStatus801C5854::Applied);
    CHECK(shortcut.nextEventState.ctxFlags ==
          kTitleShortcutFirstTickCtxFlags801C5AB4);
    CHECK((shortcut.nextEventState.ctxFlags & kCtxFlagHudStart) == 0u);
    CHECK(shortcut.nextEventState.todResourceF4Known);
    CHECK(shortcut.nextEventState.todResourceF4Index ==
          kTitleShortcutTodResourceF4Index801C5AB4);
    CHECK(shortcut.nextEventState.hudSlot0Known);
    CHECK(shortcut.nextEventState.hudSlot0 == 6u);
    CHECK(shortcut.nextEventState.hudSlot0BaseTick96Known);
    CHECK(shortcut.nextEventState.hudSlot0BaseTick96 == 0u);
    CHECK(shortcut.request.valid);
    CHECK(shortcut.request.slotId == 6u);
    CHECK(shortcut.request.recordIndex == 0u);
    CHECK(shortcut.request.baseTick96 == 0u);
    CHECK(shortcut.request.appliedTick96 == 0u);
    CHECK(shortcut.request.timIdCount == 2u);
    CHECK(shortcut.request.timIds[0] == 60);
    CHECK(shortcut.request.timIds[1] == 54);
    CHECK(shortcut.selectorIndex == 2u);
    CHECK(shortcut.selectorMimeEffect.appliedTick96Known);
    CHECK(shortcut.selectorMimeEffect.appliedTick96 == 0u);
    CHECK(shortcut.selectorMimeEffect.channel2PairWrite);
    CHECK(shortcut.selectorMimeEffect.channel2PairIndex == 4u);
    CHECK(shortcut.nextEventState.channel2PairKnown);
    CHECK(shortcut.nextEventState.channel2PairIndex == 4u);
    CheckSelectorResourceUpdate801C5854(shortcut, 2u, 22, 13, 0u);
    CHECK(shortcut.nextSelectorState.resourceCooldownKnown);
    CHECK(shortcut.nextSelectorState.resourceCooldown == 17);

    static const uint8_t expectedCursors[] = {0u, 1u, 0u, 1u};
    static const uint8_t expectedBanks[] = {0u, 0u, 1u, 1u};
    static const int16_t expectedIdAs[] = {25, 26, 22, 24};
    static const int16_t expectedIdBs[] = {16, 17, 13, 15};
    for (uint32_t index = 0u; index < 4u; ++index) {
        TitleSelectorHudRuntimeState801C5854 rowState{};
        rowState.known = true;
        rowState.bank = expectedBanks[index];
        rowState.lastCursor = expectedCursors[index];
        rowState.resourceCooldownKnown = true;
        rowState.resourceCooldown = 0;
        const uint32_t tick96 = 3000u + index;
        const auto rowTick = TickTitleSelectorHud801C5854(
            eventState,
            rowState,
            MakeSelectorHudInput(
                tick96, 1u, expectedCursors[index]));
        CHECK(rowTick.status ==
              TitleSelectorHudTickStatus801C5854::Applied);
        CheckSelectorResourceUpdate801C5854(
            rowTick,
            index,
            expectedIdAs[index],
            expectedIdBs[index],
            tick96);
    }

    TitleEventRuntimeState801C5190 dirtyShortcutState{};
    dirtyShortcutState.ctxFlags = 0xFFFFFFFFu;
    dirtyShortcutState.eventStatus80Known = true;
    dirtyShortcutState.eventStatus80 = true;
    dirtyShortcutState.objectResource104Known = true;
    dirtyShortcutState.objectResource104Index = 31u;
    dirtyShortcutState.todResourceF4Known = true;
    dirtyShortcutState.todResourceF4Index = 31u;
    dirtyShortcutState.extraResourceFCKnown = true;
    dirtyShortcutState.extraResourceFCIndex = 31u;
    dirtyShortcutState.channel1PairKnown = true;
    dirtyShortcutState.channel1PairIndex = 31u;
    dirtyShortcutState.channel3PairKnown = true;
    dirtyShortcutState.channel3PairIndex = 31u;
    dirtyShortcutState.hudSlot0Known = true;
    dirtyShortcutState.hudSlot0 = 3u;
    dirtyShortcutState.hudSlot0BaseTick96Known = true;
    dirtyShortcutState.hudSlot0BaseTick96 = 999u;
    dirtyShortcutState.hudTimelineChannel0.cursorKnown = true;
    dirtyShortcutState.hudTimelineChannel0.cursor = 99u;
    ResetTitleEventRuntimeState80024E98(dirtyShortcutState);
    CHECK(dirtyShortcutState.ctxFlags == 0u);
    CHECK(!dirtyShortcutState.eventStatus80Known);
    CHECK(!dirtyShortcutState.objectResource104Known);
    CHECK(!dirtyShortcutState.todResourceF4Known);
    CHECK(!dirtyShortcutState.extraResourceFCKnown);
    CHECK(!dirtyShortcutState.channel1PairKnown);
    CHECK(!dirtyShortcutState.channel3PairKnown);
    CHECK(!dirtyShortcutState.hudSlot0Known);
    CHECK(!dirtyShortcutState.hudSlot0BaseTick96Known);
    CHECK(!dirtyShortcutState.hudTimelineChannel0.cursorKnown);
}

void CheckTitleSelectorInput801C47EC()
{
    using namespace PrSS0TitleHudEventsDirect;

    auto result = ResolveTitleSelectorInput801C47EC(false, 0u, true, 0u);
    CHECK(!result.accepted);
    CHECK(result.status == TitleSelectorInputStatus801C47EC::SourceUnknown);

    result = ResolveTitleSelectorInput801C47EC(true, 0u, true, 2u);
    CHECK(!result.accepted);
    CHECK(result.status == TitleSelectorInputStatus801C47EC::InvalidCursor);

    result = ResolveTitleSelectorInput801C47EC(true, 0u, true, 1u);
    CHECK(result.accepted);
    CHECK(!result.playCue);
    CHECK(!result.cursorWrite);
    CHECK(result.nextCursor == 1u);
    CHECK(result.selectorResult == kTitleSelectorResultNone801C47EC);

    result = ResolveTitleSelectorInput801C47EC(
        true, kTitleSelectorInputConfirmMask801C47EC, true, 0u);
    CHECK(result.accepted);
    CHECK(result.playCue);
    CHECK(result.cueMask == kTitleSelectorInputConfirmCue801C47EC);
    CHECK(!result.cursorWrite);
    CHECK(result.selectorResult == kTitleSelectorResultStart801C47EC);

    result = ResolveTitleSelectorInput801C47EC(
        true, kTitleSelectorInputConfirmMask801C47EC, true, 1u);
    CHECK(result.selectorResult == kTitleSelectorResultMenu801C47EC);

    result = ResolveTitleSelectorInput801C47EC(
        true, kTitleSelectorInputLeftMask801C47EC, true, 0u);
    CHECK(result.playCue);
    CHECK(result.cueMask == kTitleSelectorInputMoveCue801C47EC);
    CHECK(result.cursorWrite);
    CHECK(result.nextCursor == 1u);
    CHECK(result.selectorResult == kTitleSelectorResultNone801C47EC);

    result = ResolveTitleSelectorInput801C47EC(
        true, kTitleSelectorInputRightMask801C47EC, true, 1u);
    CHECK(result.cursorWrite);
    CHECK(result.nextCursor == 0u);

    result = ResolveTitleSelectorInput801C47EC(
        true, kTitleSelectorInputMoveMask801C47EC, true, 0u);
    CHECK(result.playCue);
    CHECK(result.cueMask == kTitleSelectorInputMoveCue801C47EC);
    CHECK(!result.cursorWrite);
    CHECK(result.nextCursor == 0u);
    CHECK(result.selectorResult == kTitleSelectorResultNone801C47EC);

    result = ResolveTitleSelectorInput801C47EC(
        true,
        static_cast<uint16_t>(kTitleSelectorInputConfirmMask801C47EC |
                              kTitleSelectorInputLeftMask801C47EC),
        true,
        0u);
    CHECK(result.playCue);
    CHECK(result.cueMask == kTitleSelectorInputConfirmCue801C47EC);
    CHECK(!result.cursorWrite);
    CHECK(result.selectorResult == kTitleSelectorResultNone801C47EC);

    result = ResolveTitleSelectorInput801C47EC(true, 0x0800u, true, 1u);
    CHECK(result.accepted);
    CHECK(!result.playCue);
    CHECK(!result.cursorWrite);
    CHECK(result.nextCursor == 1u);
    CHECK(result.selectorResult == kTitleSelectorResultNone801C47EC);
}

void CheckTitleSelectorInputEdge801C4CE0()
{
    using namespace PrSS0TitleHudEventsDirect;

    auto edge = ResolveTitleSelectorInputEdge801C4CE0(
        false, 0u, true, 0u, true, 0u);
    CHECK(!edge.accepted);

    edge = ResolveTitleSelectorInputEdge801C4CE0(
        true, 0u, true, 0u, true, 0u);
    CHECK(edge.accepted);
    CHECK(!edge.inputChanged);
    CHECK(!edge.resetTimeout);
    CHECK(edge.nextPreviousInput == 0u);
    CHECK(!edge.action.accepted);

    edge = ResolveTitleSelectorInputEdge801C4CE0(
        true,
        0u,
        true,
        kTitleSelectorInputConfirmMask801C47EC,
        true,
        0u);
    CHECK(edge.accepted);
    CHECK(edge.inputChanged);
    CHECK(edge.resetTimeout);
    CHECK(edge.nextPreviousInput ==
          kTitleSelectorInputConfirmMask801C47EC);
    CHECK(edge.action.accepted);
    CHECK(edge.action.selectorResult == kTitleSelectorResultStart801C47EC);

    edge = ResolveTitleSelectorInputEdge801C4CE0(
        true,
        kTitleSelectorInputLeftMask801C47EC,
        true,
        static_cast<uint16_t>(kTitleSelectorInputLeftMask801C47EC |
                              kTitleSelectorInputConfirmMask801C47EC),
        true,
        0u);
    CHECK(edge.accepted);
    CHECK(edge.inputChanged);
    CHECK(edge.resetTimeout);
    CHECK(edge.action.playCue);
    CHECK(edge.action.cueMask == kTitleSelectorInputConfirmCue801C47EC);
    CHECK(!edge.action.cursorWrite);
    CHECK(edge.action.selectorResult == kTitleSelectorResultNone801C47EC);

    edge = ResolveTitleSelectorInputEdge801C4CE0(
        true,
        kTitleSelectorInputConfirmMask801C47EC,
        true,
        0u,
        true,
        0u);
    CHECK(edge.accepted);
    CHECK(edge.inputChanged);
    CHECK(!edge.resetTimeout);
    CHECK(edge.nextPreviousInput == 0u);
    CHECK(edge.action.accepted);
    CHECK(!edge.action.playCue);
    CHECK(edge.action.selectorResult == kTitleSelectorResultNone801C47EC);
}

void CheckTitleSelectorExitWaitCadence()
{
    using namespace PrSS0TitleHudEventsDirect;

    uint32_t waitCounter = kTitleSelectorExitWaitInitialCounter801C4894;
    uint32_t frameWaitRemaining = 0u;
    uint32_t renderFrames[15]{};
    uint32_t renderCount = 0u;
    for (uint32_t hostFrame = 0u; hostFrame <= 28u; ++hostFrame) {
        const auto tick = TickTitleSelectorExitWait801C4894(
            waitCounter, frameWaitRemaining);
        CHECK(tick.accepted);
        CHECK(tick.renderFrame == ((hostFrame & 1u) == 0u));
        if (tick.renderFrame) {
            CHECK(renderCount < 15u);
            renderFrames[renderCount++] = hostFrame;
        }
        CHECK(tick.complete == (hostFrame == 28u));
        waitCounter = tick.nextWaitCounter;
        frameWaitRemaining = tick.nextFrameWaitRemaining;
    }
    CHECK(renderCount == 15u);
    for (uint32_t index = 0u; index < renderCount; ++index) {
        CHECK(renderFrames[index] == index * 2u);
    }
    CHECK(waitCounter == 0u);
    CHECK(frameWaitRemaining == 0u);

    const auto complete =
        TickTitleSelectorExitWait801C4894(0u, 0u);
    CHECK(complete.accepted);
    CHECK(!complete.renderFrame);
    CHECK(complete.complete);
    CHECK(complete.status ==
          TitleSelectorExitWaitTickStatus801C4894::Complete);
    CHECK(!TickTitleSelectorExitWait801C4894(16u, 0u).accepted);
    CHECK(!TickTitleSelectorExitWait801C4894(15u, 2u).accepted);
    CHECK(!TickTitleSelectorExitWait801C4894(0u, 1u).accepted);
}

void CheckTitleSelectorExitWaitPresentGate801C4D58()
{
    using namespace PrSS0TitleHudEventsDirect;

    TitleSelectorExitWaitPresentGateState801C4D58 state{};
    state.waitCounter = kTitleSelectorExitWaitInitialCounter801C4894;
    uint32_t acknowledgedPresentCount = 0u;
    for (uint32_t index = 0u;
         index < kTitleSelectorExitWaitInitialCounter801C4894;
         ++index) {
        const auto prepare =
            TickTitleSelectorExitWaitPresentGate801C4D58(state);
        CHECK(prepare.accepted);
        CHECK(prepare.preparePresent);
        CHECK(!prepare.transitionReady);
        CHECK(prepare.status ==
              TitleSelectorExitWaitPresentGateStatus801C4D58::PreparePresent);
        CHECK(prepare.nextState.presentPending);
        CHECK(!prepare.nextState.presentAcknowledged);
        CHECK(prepare.nextState.waitCounter == state.waitCounter);

        const auto blocked =
            TickTitleSelectorExitWaitPresentGate801C4D58(prepare.nextState);
        CHECK(blocked.accepted);
        CHECK(!blocked.preparePresent);
        CHECK(!blocked.transitionReady);
        CHECK(blocked.status ==
              TitleSelectorExitWaitPresentGateStatus801C4D58::AwaitPresent);

        CHECK(!AcknowledgeTitleSelectorExitWaitPresent801C4D74(
                   prepare.nextState, false, true, true)
                   .accepted);
        CHECK(!AcknowledgeTitleSelectorExitWaitPresent801C4D74(
                   prepare.nextState, true, false, true)
                   .accepted);
        CHECK(!AcknowledgeTitleSelectorExitWaitPresent801C4D74(
                   prepare.nextState, true, true, false)
                   .accepted);
        const auto acknowledged =
            AcknowledgeTitleSelectorExitWaitPresent801C4D74(
                prepare.nextState, true, true, true);
        CHECK(acknowledged.accepted);
        CHECK(acknowledged.nextState.presentAcknowledged);
        ++acknowledgedPresentCount;

        const auto committed =
            TickTitleSelectorExitWaitPresentGate801C4D58(
                acknowledged.nextState);
        CHECK(committed.accepted);
        CHECK(!committed.preparePresent);
        CHECK(!committed.nextState.presentPending);
        CHECK(!committed.nextState.presentAcknowledged);
        if (index + 1u == kTitleSelectorExitWaitInitialCounter801C4894) {
            CHECK(committed.transitionReady);
            CHECK(committed.status ==
                  TitleSelectorExitWaitPresentGateStatus801C4D58::Complete);
            CHECK(committed.nextState.waitCounter == 0u);
        } else {
            CHECK(!committed.transitionReady);
            CHECK(committed.status ==
                  TitleSelectorExitWaitPresentGateStatus801C4D58::
                      PresentCommitted);
            CHECK(committed.nextState.waitCounter ==
                  kTitleSelectorExitWaitInitialCounter801C4894 - index - 1u);
            CHECK(committed.nextState.frameWaitRemaining == 0u);
        }
        state = committed.nextState;
    }
    CHECK(acknowledgedPresentCount ==
          kTitleSelectorExitWaitInitialCounter801C4894);

    state = {};
    state.waitCounter = 1u;
    state.presentAcknowledged = true;
    CHECK(!TickTitleSelectorExitWaitPresentGate801C4D58(state).accepted);
}

void CheckTitleNaturalLoopCadence()
{
    using namespace PrSS0TitleHudEventsDirect;

    TitleNaturalLoopCadenceState801C4894 state{};
    CHECK(state.holdTicksRemaining == 0u);

    const auto first = TickTitleNaturalLoopCadence801C4894(state);
    CHECK(first.processIteration);
    CHECK(!first.holdTick);
    CHECK(state.holdTicksRemaining == 1u);

    const auto second = TickTitleNaturalLoopCadence801C4894(state);
    CHECK(!second.processIteration);
    CHECK(second.holdTick);
    CHECK(state.holdTicksRemaining == 0u);

    const auto armedBeforeReset =
        TickTitleNaturalLoopCadence801C4894(state);
    CHECK(armedBeforeReset.processIteration);
    CHECK(state.holdTicksRemaining == 1u);
    state.holdTicksRemaining = 0u;
    const auto afterPendingReset =
        TickTitleNaturalLoopCadence801C4894(state);
    CHECK(afterPendingReset.processIteration);
    CHECK(!afterPendingReset.holdTick);
    CHECK(state.holdTicksRemaining == 1u);

    state = {};
    uint32_t processedCount = 0u;
    uint32_t holdCount = 0u;
    for (uint32_t hostTick = 0u; hostTick < 134u; ++hostTick) {
        const auto tick = TickTitleNaturalLoopCadence801C4894(state);
        const bool shouldProcess = (hostTick & 1u) == 0u;
        CHECK(tick.processIteration == shouldProcess);
        CHECK(tick.holdTick == !shouldProcess);
        if (shouldProcess) {
            ++processedCount;
        } else {
            ++holdCount;
        }
    }
    CHECK(processedCount == 67u);
    CHECK(holdCount == 67u);
    CHECK(state.holdTicksRemaining == 0u);

    state = {};
    processedCount = 0u;
    holdCount = 0u;
    for (uint32_t hostTick = 0u; hostTick < 642u; ++hostTick) {
        const auto tick = TickTitleNaturalLoopCadence801C4894(state);
        CHECK(tick.processIteration != tick.holdTick);
        if (tick.processIteration) {
            ++processedCount;
        } else {
            CHECK(tick.holdTick);
            ++holdCount;
        }
    }
    CHECK(processedCount == 321u);
    CHECK(holdCount == 321u);
    CHECK(state.holdTicksRemaining == 0u);

    // The live 801C4894 owner advances title logic at 30 Hz while its
    // separate MOVIE0T MDEC gate supplies 15 fps pictures.  Keep the
    // two-VBlank helper above as the explicit VBlank model and verify the
    // live-mode override does not leave a stale hold tick behind.
    state = {};
    for (uint32_t hostTick = 0u; hostTick < 8u; ++hostTick) {
        const auto tick =
            TickTitleNaturalLoopCadence801C4894(state, false);
        CHECK(tick.processIteration);
        CHECK(!tick.holdTick);
        CHECK(state.holdTicksRemaining == 0u);
    }
}

PrSS0TitleHudEventsDirect::TitleSharedEventBucketInput80024FD0
MakeBucketPrefixInput(uint32_t callsite, uint32_t tick96)
{
    PrSS0TitleHudEventsDirect::TitleSharedEventBucketInput80024FD0 input{};
    input.callsiteKnown = true;
    input.callsite = callsite;
    input.ctxTick96Known = true;
    input.ctxTick96 = tick96;
    return input;
}

void CheckTitleEventBucketPrefix80024FD0()
{
    using namespace PrSS0TitleHudEventsDirect;

    CHECK(kFn801C4894 == 0x801C4894u);
    CHECK(kCallsite801C4C9C == 0x801C4C9Cu);
    CHECK(kCallsite801C4D14 == 0x801C4D14u);
    CHECK(kFn80024FD0 == 0x80024FD0u);

    TitleSharedEventBucketState80024FD0 state{};
    state.dword8008ED20 = 9u;
    state.dword8008ECE8 = 9u;
    state.dword8008ECF0 = 9u;
    state.dword8008ECF4 = 9u;
    ResetTitleSharedEventBucketState80024E98(state);
    CHECK(state.resetKnown);
    CHECK(state.dword8008ED20 == 0u);
    CHECK(state.dword8008ECE8 == 0u);
    CHECK(state.dword8008ECF0 == 0u);
    CHECK(state.dword8008ECF4 == 0u);

    TitleSharedEventBucketState80024FD0 unknownState{};
    auto input = MakeBucketPrefixInput(kCallsite801C4C9C, 12u);
    auto result = TickTitleSharedEventBucketPrefix80024FD0(unknownState, input);
    CHECK(result.status ==
          TitleSharedEventBucketStatus80024FD0::SourceUnknown);
    CHECK(!result.accepted);
    CHECK(!result.prefixApplied);
    CHECK(!result.earlyReturnEd20);
    CHECK(!result.bucketChanged);
    CHECK(!result.sharedTailRequired);
    CHECK(!result.nextState.resetKnown);

    input.callsiteKnown = false;
    result = TickTitleSharedEventBucketPrefix80024FD0(state, input);
    CHECK(result.status ==
          TitleSharedEventBucketStatus80024FD0::SourceUnknown);
    CHECK(!result.accepted);
    CHECK(result.nextState.dword8008ECF0 == 0u);

    input = MakeBucketPrefixInput(kCallsite801C4C9C, 12u);
    input.ctxTick96Known = false;
    result = TickTitleSharedEventBucketPrefix80024FD0(state, input);
    CHECK(result.status ==
          TitleSharedEventBucketStatus80024FD0::SourceUnknown);
    CHECK(!result.accepted);
    CHECK(result.nextState.dword8008ECF0 == 0u);

    input = MakeBucketPrefixInput(0x801C4D18u, 12u);
    result = TickTitleSharedEventBucketPrefix80024FD0(state, input);
    CHECK(result.status ==
          TitleSharedEventBucketStatus80024FD0::UnsupportedCallsite);
    CHECK(!result.accepted);
    CHECK(!result.prefixApplied);
    CHECK(result.nextState.dword8008ECE8 == 0u);

    auto disabledState = state;
    disabledState.dword8008ED20 = 1u;
    disabledState.dword8008ECE8 = 77u;
    disabledState.dword8008ECF0 = 6u;
    disabledState.dword8008ECF4 = 7u;
    input = MakeBucketPrefixInput(kCallsite801C4C9C, 96u);
    result = TickTitleSharedEventBucketPrefix80024FD0(disabledState, input);
    CHECK(result.status == TitleSharedEventBucketStatus80024FD0::Disabled);
    CHECK(result.accepted);
    CHECK(!result.prefixApplied);
    CHECK(result.earlyReturnEd20);
    CHECK(!result.bucketChanged);
    CHECK(!result.sharedTailRequired);
    CHECK(result.nextState.dword8008ECE8 == 77u);
    CHECK(result.nextState.dword8008ECF0 == 6u);
    CHECK(result.nextState.dword8008ECF4 == 7u);

    input = MakeBucketPrefixInput(kCallsite801C4C9C, 11u);
    result = TickTitleSharedEventBucketPrefix80024FD0(state, input);
    CHECK(result.status ==
          TitleSharedEventBucketStatus80024FD0::BucketUnchanged);
    CHECK(result.accepted);
    CHECK(result.prefixApplied);
    CHECK(!result.earlyReturnEd20);
    CHECK(!result.bucketChanged);
    CHECK(!result.sharedTailRequired);
    CHECK(result.nextState.dword8008ECE8 == 11u);
    CHECK(result.nextState.dword8008ECF0 == 0u);
    CHECK(result.nextState.dword8008ECF4 == 0u);

    input = MakeBucketPrefixInput(kCallsite801C4C9C, 12u);
    result = TickTitleSharedEventBucketPrefix80024FD0(state, input);
    CHECK(result.status ==
          TitleSharedEventBucketStatus80024FD0::BucketAdvanced);
    CHECK(result.accepted);
    CHECK(result.prefixApplied);
    CHECK(!result.earlyReturnEd20);
    CHECK(result.bucketChanged);
    CHECK(result.sharedTailRequired);
    CHECK(result.nextState.dword8008ECE8 == 12u);
    CHECK(result.nextState.dword8008ECF0 == 1u);
    CHECK(result.nextState.dword8008ECF4 == 1u);

    auto secondCallsiteState = result.nextState;
    input = MakeBucketPrefixInput(kCallsite801C4D14, 23u);
    result = TickTitleSharedEventBucketPrefix80024FD0(secondCallsiteState, input);
    CHECK(result.status ==
          TitleSharedEventBucketStatus80024FD0::BucketUnchanged);
    CHECK(result.accepted);
    CHECK(result.prefixApplied);
    CHECK(!result.bucketChanged);
    CHECK(!result.sharedTailRequired);
    CHECK(result.nextState.dword8008ECE8 == 23u);
    CHECK(result.nextState.dword8008ECF0 == 1u);
    CHECK(result.nextState.dword8008ECF4 == 1u);

    auto wrapState = state;
    wrapState.dword8008ECF0 = 31u;
    wrapState.dword8008ECF4 = 31u;
    input = MakeBucketPrefixInput(kCallsite801C4D14, 384u);
    result = TickTitleSharedEventBucketPrefix80024FD0(wrapState, input);
    CHECK(result.status ==
          TitleSharedEventBucketStatus80024FD0::BucketAdvanced);
    CHECK(result.accepted);
    CHECK(result.prefixApplied);
    CHECK(result.bucketChanged);
    CHECK(result.sharedTailRequired);
    CHECK(result.nextState.dword8008ECE8 == 0u);
    CHECK(result.nextState.dword8008ECF0 == 0u);
    CHECK(result.nextState.dword8008ECF4 == 0u);
}

void CheckTitleAttractTimeoutPolicy801C48C0()
{
    using namespace PrSS0TitleHudEventsDirect;

    TitleAttractTimeoutPolicyInput801C48C0 input{};
    auto result = ResolveTitleAttractTimeoutPolicy801C48C0(input);
    CHECK(result.status ==
          TitleAttractTimeoutPolicyStatus801C48C0::SourceUnknown);
    CHECK(!result.accepted);
    CHECK(!result.timeoutKnown);
    CHECK(result.timeoutFrames == 0u);

    input.word800916FCKnown = true;
    input.word800916FC = 0u;
    result = ResolveTitleAttractTimeoutPolicy801C48C0(input);
    CHECK(result.status ==
          TitleAttractTimeoutPolicyStatus801C48C0::Default1800);
    CHECK(result.accepted);
    CHECK(result.timeoutKnown);
    CHECK(result.timeoutFrames == kTitleAttractTimeoutDefault801C4894);

    input.word800916FC = 1u;
    result = ResolveTitleAttractTimeoutPolicy801C48C0(input);
    CHECK(result.status ==
          TitleAttractTimeoutPolicyStatus801C48C0::Short450);
    CHECK(result.accepted);
    CHECK(result.timeoutKnown);
    CHECK(result.timeoutFrames == kTitleAttractTimeoutShort801C4894);

    input.word800916FC = 7u;
    result = ResolveTitleAttractTimeoutPolicy801C48C0(input);
    CHECK(result.status ==
          TitleAttractTimeoutPolicyStatus801C48C0::Default1800);
    CHECK(result.accepted);
    CHECK(result.timeoutKnown);
    CHECK(result.timeoutFrames == kTitleAttractTimeoutDefault801C4894);
}

void CheckTitleEarlyInputShortcutHoldCadence()
{
    using namespace PrSS0TitleHudEventsDirect;

    uint32_t completedWaitCalls = 0u;
    uint32_t frameWaitRemaining = kShortcutFrameWait;
    for (uint32_t hostTick = 1u; hostTick <= 30u; ++hostTick) {
        const auto tick = TickTitleEarlyInputShortcutHold801C4894(
            completedWaitCalls, frameWaitRemaining);
        CHECK(tick.accepted);
        CHECK(tick.ready == (hostTick == 30u));
        completedWaitCalls = tick.nextCompletedWaitCalls;
        frameWaitRemaining = tick.nextFrameWaitRemaining;
        CHECK(completedWaitCalls == hostTick / 2u);
        CHECK(frameWaitRemaining ==
              (hostTick == 30u ? 0u
                               : (hostTick & 1u) != 0u ? 1u : 2u));
    }
    CHECK(completedWaitCalls == kShortcutHoldWaitFrames);
    CHECK(frameWaitRemaining == 0u);

    const auto ready = TickTitleEarlyInputShortcutHold801C4894(
        completedWaitCalls, frameWaitRemaining);
    CHECK(ready.accepted);
    CHECK(ready.ready);
    CHECK(ready.status ==
          TitleEarlyInputShortcutHoldTickStatus801C4894::Ready);
    CHECK(!TickTitleEarlyInputShortcutHold801C4894(0u, 0u).accepted);
    CHECK(!TickTitleEarlyInputShortcutHold801C4894(16u, 0u).accepted);
    CHECK(!TickTitleEarlyInputShortcutHold801C4894(15u, 1u).accepted);
    CHECK(!TickTitleEarlyInputShortcutHold801C4894(0u, 3u).accepted);
}

void CheckTitleSelectorPromptPlan8001E3E4()
{
    using namespace PrSS0TitleHudEventsDirect;

    const auto unknown = BuildTitleSelectorPrompt8001E3E4(false, 0u);
    CHECK(!unknown.accepted);
    CHECK(unknown.status ==
          TitleSelectorPromptPlanStatus80020488::SourceUnknown);
    const auto unsupported = BuildTitleSelectorPrompt8001E3E4(true, 2u);
    CHECK(!unsupported.accepted);
    CHECK(unsupported.status ==
          TitleSelectorPromptPlanStatus80020488::UnsupportedSelector);

    auto checkSprite = [](const TitleSelectorPromptSprite80020488& sprite,
                          int16_t x,
                          int16_t y,
                          uint32_t sourceAddress,
                          uint32_t attr,
                          uint16_t texX,
                          uint16_t texY,
                          uint16_t width,
                          uint16_t height,
                          uint16_t clutX,
                          uint16_t clutY) {
        CHECK(sprite.active);
        CHECK(sprite.x == x);
        CHECK(sprite.y == y);
        CHECK(sprite.spriteTemplate.sourceAddress == sourceAddress);
        CHECK(sprite.spriteTemplate.attr == attr);
        CHECK(sprite.spriteTemplate.texX == texX);
        CHECK(sprite.spriteTemplate.texY == texY);
        CHECK(sprite.spriteTemplate.width == width);
        CHECK(sprite.spriteTemplate.height == height);
        CHECK(sprite.spriteTemplate.clutX == clutX);
        CHECK(sprite.spriteTemplate.clutY == clutY);
    };

    const auto start = BuildTitleSelectorPrompt8001E3E4(true, 0u);
    CHECK(start.accepted);
    CHECK(start.status == TitleSelectorPromptPlanStatus80020488::Ready);
    CHECK(start.wrapperFunction == kFn8001E3E4);
    CHECK(start.drawFunction == kFn80020488);
    checkSprite(start.sprites[0],
                278, 78, 0x80052FE0u, 0x01000040u,
                640u, 20u, 16u, 10u, 320u, 508u);
    checkSprite(start.sprites[1],
                10, 207, 0x80052FB0u, 0x51000040u,
                640u, 40u, 168u, 10u, 320u, 508u);
    checkSprite(start.sprites[2],
                178, 207, 0x80052FD0u, 0x01000040u,
                640u, 30u, 132u, 10u, 320u, 508u);
    checkSprite(start.sprites[3],
                46, 220, 0x80052FC0u, 0x51000040u,
                640u, 50u, 228u, 10u, 320u, 508u);
    checkSprite(start.sprites[4],
                24, 139, 0x80052FA0u, 0x50000040u,
                656u, 91u, 68u, 24u, 864u, 283u);
    checkSprite(start.sprites[5],
                231, 139, 0x80052F70u, 0x50000040u,
                640u, 91u, 64u, 24u, 864u, 280u);

    const auto menu = BuildTitleSelectorPrompt8001E3E4(true, 1u);
    CHECK(menu.accepted);
    CHECK(menu.status == TitleSelectorPromptPlanStatus80020488::Ready);
    checkSprite(menu.sprites[4],
                24, 139, 0x80052F90u, 0x50000040u,
                656u, 91u, 68u, 24u, 864u, 282u);
    checkSprite(menu.sprites[5],
                231, 139, 0x80052F80u, 0x50000040u,
                640u, 91u, 64u, 24u, 864u, 281u);
}

} // namespace

int main()
{
    CheckComod0HudTableImport();
    using PrSS0TitleHudEventsDirect::ComputeTitleEventClock801C4894;

    const auto initialUnknown =
        ComputeTitleEventClock801C4894(-17, false, 0u);
    CHECK(!initialUnknown.known);
    CHECK(initialUnknown.loopTickV10 == -17);
    CHECK(initialUnknown.tick96 == 0u);
    CHECK(!initialUnknown.beatKnown);

    const auto initialKnown =
        ComputeTitleEventClock801C4894(0, true, 123u);
    CHECK(initialKnown.known);
    CHECK(initialKnown.loopTickV10 == 0);
    CHECK(initialKnown.tick96 == 123u);
    CHECK(initialKnown.beatKnown);
    CHECK(initialKnown.beat == 2u);
    CHECK(initialKnown.tickWithinBeat == 28u);

    CheckKnown(1, 5u);
    CheckKnown(35, 190u);
    CheckKnown(36, 195u);
    CheckKnown(50, 272u);
    CheckKnown(70, 380u);
    CheckKnown(71, 386u);
    CheckKnown(317, 1724u);
    CheckKnown(318, 1729u);
    CheckKnown(370, 2012u);

    const auto overflow = ComputeTitleEventClock801C4894(
        std::numeric_limits<int32_t>::max(), false, 0u);
    CHECK(!overflow.known);
    CHECK(overflow.tick96 == 0u);

    using namespace PrSS0TitleHudEventsDirect;
    TitleAbsoluteEventStreamState801C5538 crossingStream{};
    ResetTitleAbsoluteEventStreamState80024E98(crossingStream);
    crossingStream.cursor = 5u;
    auto crossingTick = TickTitleAbsoluteEventStream801C5538(
        crossingStream, MakeAbsoluteInput(1724u));
    CHECK(crossingTick.status ==
          TitleAbsoluteEventTickStatus801C5538::NotDue);
    crossingTick = TickTitleAbsoluteEventStream801C5538(
        crossingStream, MakeAbsoluteInput(1729u));
    CHECK(crossingTick.status ==
          TitleAbsoluteEventTickStatus801C5538::Applied);
    CHECK(crossingTick.effect.eventIndex == 5u);
    CHECK(crossingTick.effect.thresholdTick96 == 1728u);
    CHECK(crossingTick.effect.appliedTick96Known);
    CHECK(crossingTick.effect.appliedTick96 == 1729u);

    TitleAbsoluteEventStreamState801C5538 stream{};
    ResetTitleAbsoluteEventStreamState80024E98(stream);
    auto eventTick = TickTitleAbsoluteEventStream801C5538(
        stream, MakeAbsoluteInput(191u));
    CHECK(eventTick.status ==
          TitleAbsoluteEventTickStatus801C5538::NotDue);
    CHECK(eventTick.nextState.cursor == 0u);
    CHECK(!eventTick.effect.known);

    eventTick = TickTitleAbsoluteEventStream801C5538(
        stream, MakeAbsoluteInput(192u));
    CHECK(eventTick.status ==
          TitleAbsoluteEventTickStatus801C5538::Applied);
    CHECK(eventTick.nextState.cursor == 1u);
    CHECK(eventTick.effect.known);
    CHECK(eventTick.effect.eventIndex == 0u);
    CHECK(eventTick.effect.ctxFlagsSetMask == 0x02050420u);
    CHECK(eventTick.effect.objectResource104Write);
    CHECK(eventTick.effect.objectResource104Index == 0x1Bu);
    CHECK(eventTick.effect.todResourceF4Write);
    CHECK(eventTick.effect.todResourceF4Index == 3u);
    CHECK(eventTick.effect.channel2PairWrite);
    CHECK(eventTick.effect.channel2PairIndex == 5u);
    CHECK(eventTick.effect.appliedTick96Known);
    CHECK(eventTick.effect.appliedTick96 == 192u);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(
              eventTick.effect.channel2PairIndex) ==
          PrSS0TitleTmdBackend::MimeKind::PaOki);

    TitleEventRuntimeState801C5190 runtimeState{};
    auto runtimeCommit = BuildTitleEventRuntimeCommit801C5190(
        runtimeState, eventTick.effect);
    CHECK(runtimeCommit.accepted);
    CHECK(runtimeCommit.nextState.ctxFlags == 0x02050420u);
    CHECK(runtimeCommit.nextState.objectResource104Known);
    CHECK(runtimeCommit.nextState.objectResource104Index == 0x1Bu);
    CHECK(runtimeCommit.nextState.todResourceF4Known);
    CHECK(runtimeCommit.nextState.todResourceF4Index == 3u);
    CHECK(runtimeCommit.nextState.channel2PairKnown);
    CHECK(runtimeCommit.nextState.channel2PairIndex == 5u);
    runtimeState = runtimeCommit.nextState;

    stream = eventTick.nextState;
    eventTick = TickTitleAbsoluteEventStream801C5538(
        stream, MakeAbsoluteInput(192u));
    CHECK(eventTick.status ==
          TitleAbsoluteEventTickStatus801C5538::NotDue);
    CHECK(eventTick.nextState.cursor == 1u);

    eventTick = TickTitleAbsoluteEventStream801C5538(
        stream, MakeAbsoluteInput(384u));
    CHECK(eventTick.status ==
          TitleAbsoluteEventTickStatus801C5538::Applied);
    CHECK(eventTick.nextState.cursor == 2u);
    CHECK(eventTick.effect.ctxFlagsSetMask == 0x00050000u);
    CHECK(eventTick.effect.channel2PairWrite);
    CHECK(eventTick.effect.channel2PairIndex == 3u);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(
              eventTick.effect.channel2PairIndex) ==
          PrSS0TitleTmdBackend::MimeKind::PaDance);
    runtimeCommit = BuildTitleEventRuntimeCommit801C5190(
        runtimeState, eventTick.effect);
    CHECK(runtimeCommit.accepted);
    CHECK(runtimeCommit.nextState.ctxFlags == 0x02050420u);
    CHECK(runtimeCommit.nextState.channel2PairIndex == 3u);
    runtimeState = runtimeCommit.nextState;

    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(0u) ==
          PrSS0TitleTmdBackend::MimeKind::Count);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(9u) ==
          PrSS0TitleTmdBackend::MimeKind::Count);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(1u) ==
          PrSS0TitleTmdBackend::MimeKind::Hiphop);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(2u) ==
          PrSS0TitleTmdBackend::MimeKind::LogoNew);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(4u) ==
          PrSS0TitleTmdBackend::MimeKind::PaLeft);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(6u) ==
          PrSS0TitleTmdBackend::MimeKind::PaRight);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(7u) ==
          PrSS0TitleTmdBackend::MimeKind::PaTurnLeft);
    CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(8u) ==
          PrSS0TitleTmdBackend::MimeKind::PaTurnRight);

    stream = eventTick.nextState;
    eventTick = TickTitleAbsoluteEventStream801C5538(
        stream, MakeAbsoluteInput(672u));
    CHECK(eventTick.status ==
          TitleAbsoluteEventTickStatus801C5538::Applied);
    CHECK(eventTick.effect.hudSlot0Write);
    CHECK(eventTick.effect.hudSlot0 == 2u);
    runtimeCommit = BuildTitleEventRuntimeCommit801C5190(
        runtimeState, eventTick.effect);
    CHECK(runtimeCommit.accepted);
    CHECK(runtimeCommit.nextState.hudSlot0Known);
    CHECK(runtimeCommit.nextState.hudSlot0 == 2u);
    CHECK(runtimeCommit.nextState.hudSlot0BaseTick96Known);
    CHECK(runtimeCommit.nextState.hudSlot0BaseTick96 == 672u);
    runtimeState = runtimeCommit.nextState;

    const auto decodedOnly = DecodeTitleEventEffect801C5190(0u);
    CHECK(decodedOnly.known);
    CHECK(!decodedOnly.appliedTick96Known);
    const auto rejectedCommit = BuildTitleEventRuntimeCommit801C5190(
        TitleEventRuntimeState801C5190{}, decodedOnly);
    CHECK(!rejectedCommit.accepted);
    CHECK(rejectedCommit.nextState.ctxFlags == 0u);
    CHECK(!rejectedCommit.nextState.channel2PairKnown);

    stream = eventTick.nextState;
    for (uint32_t i = stream.cursor;
         i < KnownEventStreamRecordCount(); ++i) {
        const auto record = KnownEventStreamRecordAt(i);
        eventTick = TickTitleAbsoluteEventStream801C5538(
            stream, MakeAbsoluteInput(record.frame));
        CHECK(eventTick.status ==
              TitleAbsoluteEventTickStatus801C5538::Applied);
        CHECK(eventTick.nextState.cursor == stream.cursor + 1u);
        CHECK(eventTick.effect.eventIndex == i);
        CHECK(eventTick.effect.appliedTick96Known);
        CHECK(eventTick.effect.appliedTick96 == record.frame);

        runtimeCommit = BuildTitleEventRuntimeCommit801C5190(
            runtimeState, eventTick.effect);
        CHECK(runtimeCommit.accepted);
        if (eventTick.effect.eventIndex == 5u) {
            CHECK(record.frame == 1728u);
            CHECK(eventTick.effect.ctxFlagsSetMask == 0x01400000u);
            CHECK(eventTick.effect.channel1PairWrite);
            CHECK(eventTick.effect.channel1PairIndex == 2u);
            CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(
                      eventTick.effect.channel1PairIndex) ==
                  PrSS0TitleTmdBackend::MimeKind::LogoNew);
            CHECK(eventTick.effect.channel3PairWrite);
            CHECK(eventTick.effect.channel3PairIndex == 1u);
            CHECK(PrSS0TitleTmdBackend::MimeKindFromResourcePairIndex801C6C14(
                      eventTick.effect.channel3PairIndex) ==
                  PrSS0TitleTmdBackend::MimeKind::Hiphop);
            CHECK(runtimeCommit.nextState.ctxFlags == 0x03450420u);
            CHECK(runtimeCommit.nextState.channel1PairKnown);
            CHECK(runtimeCommit.nextState.channel1PairIndex == 2u);
            CHECK(runtimeCommit.nextState.channel3PairKnown);
            CHECK(runtimeCommit.nextState.channel3PairIndex == 1u);
            CHECK(runtimeCommit.nextState.objectResource104Known);
            CHECK(runtimeCommit.nextState.objectResource104Index == 0x1Bu);
            CHECK(runtimeCommit.nextState.todResourceF4Known);
            CHECK(runtimeCommit.nextState.todResourceF4Index == 2u);
            CHECK(runtimeCommit.nextState.channel2PairKnown);
            CHECK(runtimeCommit.nextState.channel2PairIndex == 3u);
            CHECK(runtimeCommit.nextState.hudSlot0Known);
            CHECK(runtimeCommit.nextState.hudSlot0 == 1u);
            CHECK(runtimeCommit.nextState.hudSlot0BaseTick96Known);
            CHECK(runtimeCommit.nextState.hudSlot0BaseTick96 == 1272u);

            const auto decodedEvent5 =
                DecodeTitleEventEffect801C5190(5u);
            CHECK(decodedEvent5.known);
            CHECK(!decodedEvent5.appliedTick96Known);
            const auto rejectedEvent5Commit =
                BuildTitleEventRuntimeCommit801C5190(
                    runtimeState, decodedEvent5);
            CHECK(!rejectedEvent5Commit.accepted);
            CHECK(rejectedEvent5Commit.nextState.ctxFlags ==
                  runtimeState.ctxFlags);
            CHECK(!rejectedEvent5Commit.nextState.channel1PairKnown);
            CHECK(!rejectedEvent5Commit.nextState.channel3PairKnown);
        }
        runtimeState = runtimeCommit.nextState;
        stream = eventTick.nextState;
    }
    eventTick = TickTitleAbsoluteEventStream801C5538(
        stream, MakeAbsoluteInput(2000u));
    CHECK(eventTick.status ==
          TitleAbsoluteEventTickStatus801C5538::Complete);
    CHECK(eventTick.nextState.cursor == KnownEventStreamRecordCount());

    auto unknownInput = MakeAbsoluteInput(192u);
    unknownInput.ctxTick96Known = false;
    TitleAbsoluteEventStreamState801C5538 unknownState{};
    ResetTitleAbsoluteEventStreamState80024E98(unknownState);
    eventTick = TickTitleAbsoluteEventStream801C5538(
        unknownState, unknownInput);
    CHECK(eventTick.status ==
          TitleAbsoluteEventTickStatus801C5538::SourceUnknown);
    CHECK(eventTick.nextState.cursor == 0u);

    auto relativeInput = MakeAbsoluteInput(192u);
    relativeInput.streamId = 1u;
    TitleAbsoluteEventStreamState801C5538 relativeState{};
    ResetTitleAbsoluteEventStreamState80024E98(relativeState);
    eventTick = TickTitleAbsoluteEventStream801C5538(
        relativeState, relativeInput);
    CHECK(eventTick.status ==
          TitleAbsoluteEventTickStatus801C5538::UnsupportedMode);
    CHECK(eventTick.nextState.cursor == 0u);

    CheckExactHudTimelineTable();
    CheckSlot1HudTimelineTiming();
    CheckConsecutiveZeroDeltaHudRecords();
    CheckHudTimelineFailClosedAndCompletion();
    CheckHudSlotCommitResetsTimelineCursor();
    CheckTitleSelectorHudStartAndFirstTick();
    CheckTitleSelectorInput801C47EC();
    CheckTitleSelectorInputEdge801C4CE0();
    CheckTitleSelectorExitWaitCadence();
    CheckTitleSelectorExitWaitPresentGate801C4D58();
    CheckTitleNaturalLoopCadence();
    CheckTitleAttractTimeoutPolicy801C48C0();
    CheckTitleEarlyInputShortcutHoldCadence();
    CheckTitleSelectorPromptPlan8001E3E4();
    CheckTitleEventBucketPrefix80024FD0();

    if (g_failedChecks != 0) {
        std::printf("test_ss0_title_event_clock: FAIL (%d checks)\n",
                    g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_title_event_clock: PASS\n");
    return 0;
}
