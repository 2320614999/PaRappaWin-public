#include "pr_ss0_title_hud_events_direct.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace PrSS0TitleHudEventsDirect {
namespace {

static const TitleHudEventRecord kEventStreamRecords[] = {
    {192u, 0x02050020u, {0x1Bu, 0x03u, 0x05u, 0, 0, 0, 0, 0}},
    {384u, 0x00050000u, {0x00u, 0x02u, 0x03u, 0, 0, 0, 0, 0}},
    {672u, 0x00000000u, {0, 0, 0, 0x02u, 0, 0, 0, 0}},
    {1056u, 0x00000000u, {0, 0, 0, 0x02u, 0, 0, 0, 0}},
    {1272u, 0x00000000u, {0, 0, 0, 0x01u, 0, 0, 0, 0}},
    {1728u, 0x01400000u, {0, 0, 0, 0, 0, 0x02u, 0x01u, 0}},
    {1920u, 0x00000100u, {0, 0, 0, 0, 0, 0, 0, 0}},
};

static_assert(sizeof(TitleHudTimelineRecord801C5094) ==
                  kHudTimelineRecordBytes,
              "S0 HUD timeline records must retain their PSX layout");

static const TitleHudTimelineRecord801C5094 kTimelineSlot1Records[] = {
    {0u, {29, 0, 0, 0}},
    {24u, {30, 0, 0, 0}},
    {48u, {28, 0, 0, 0}},
    {72u, {29, 0, 0, 0}},
    {96u, {30, 0, 0, 0}},
    {120u, {37, 45, 0, 0}},
    {192u, {33, 41, 28, 0}},
};

static const TitleHudTimelineRecord801C5094 kTimelineSlot2Records[] = {
    {0u, {34, 42, 0, 0}},
    {24u, {33, 41, 0, 0}},
};

static const TitleHudTimelineRecord801C5094 kTimelineSlot3Records[] = {
    {0u, {60, 54, 0, 0}},
    {0u, {55, 49, 0, 0}},
    {0u, {34, 42, 0, 0}},
    {24u, {33, 41, 0, 0}},
};

static const TitleHudTimelineRecord801C5094 kTimelineSlot4Records[] = {
    {0u, {57, 51, 0, 0}},
    {0u, {58, 52, 0, 0}},
    {0u, {34, 42, 0, 0}},
    {24u, {33, 41, 0, 0}},
};

static const TitleHudTimelineRecord801C5094 kTimelineSlot5Records[] = {
    {0u, {57, 51, 0, 0}},
    {0u, {58, 52, 0, 0}},
    {0u, {33, 41, 0, 0}},
};

static const TitleHudTimelineRecord801C5094 kTimelineSlot6Records[] = {
    {0u, {60, 54, 0, 0}},
    {0u, {55, 49, 0, 0}},
    {0u, {33, 41, 0, 0}},
};

struct TitleHudTimelineSlotRecords {
    const TitleHudTimelineRecord801C5094* records = nullptr;
    uint32_t count = 0;
};

static const TitleHudTimelineSlotRecords kTimelineSlotRecords[] = {
    {nullptr, 0u},
    {kTimelineSlot1Records, 7u},
    {kTimelineSlot2Records, 2u},
    {kTimelineSlot3Records, 4u},
    {kTimelineSlot4Records, 4u},
    {kTimelineSlot5Records, 3u},
    {kTimelineSlot6Records, 3u},
};

static_assert(sizeof(kTimelineSlotRecords) /
                      sizeof(kTimelineSlotRecords[0]) ==
                  kHudTimelineDescriptorCount,
              "S0 HUD timeline records must match the descriptor table");

static const TitleHudTimelineDescriptor kTimelineDescriptors[] = {
    {0x00000000u, 0u, 0u},
    {0x801C6C38u, 7u, 0u},
    {0x801C6C8Cu, 2u, 0u},
    {0x801C6CA4u, 4u, 0u},
    {0x801C6CD4u, 4u, 0u},
    {0x801C6D04u, 3u, 0u},
    {0x801C6D28u, 3u, 0u},
};

static const TitleHudSelectorRecord kSelectorRecords[] = {
    {25, 16, 17, 6},
    {26, 17, 17, 5},
    {22, 13, 17, 3},
    {24, 15, 17, 4},
};

static const uint8_t kSelectorResourcePairIndices[] = {7u, 8u, 4u, 6u};

static_assert(sizeof(kSelectorResourcePairIndices) /
                      sizeof(kSelectorResourcePairIndices[0]) ==
                  sizeof(kSelectorRecords) / sizeof(kSelectorRecords[0]),
              "S0 selector records must retain exact resource-pair owners");

constexpr std::size_t kComod0ResourcePairCount801C6C14 = 78u;
constexpr std::size_t kComod0TimelineRecordCapacity801C6D4C = 7u;

std::array<TitleHudEventRecord, kEventStreamRecordCount>
    s_comod0EventStreamRecords{};
std::array<TitleHudTimelineDescriptor, kHudTimelineDescriptorCount>
    s_comod0TimelineDescriptors{};
std::array<std::array<TitleHudTimelineRecord801C5094,
                         kComod0TimelineRecordCapacity801C6D4C>,
           kHudTimelineDescriptorCount>
    s_comod0TimelineRecords{};
std::array<uint32_t, kHudTimelineDescriptorCount>
    s_comod0TimelineRecordCounts{};
std::array<TitleHudSelectorRecord, kSelectorRecordCount>
    s_comod0SelectorRecords{};
std::array<uint8_t, kSelectorRecordCount>
    s_comod0SelectorResourcePairIndices{};
std::array<std::array<int16_t, 2>, kComod0ResourcePairCount801C6C14>
    s_comod0ResourcePairs{};
TitleHudComodSourceState801C6Dxx s_comod0SourceState{};

bool ReadComod0Range(const uint8_t* data,
                     std::size_t size,
                     uint32_t mappedBase,
                     uint32_t address,
                     std::size_t byteCount,
                     const uint8_t*& out)
{
    if (data == nullptr || address < mappedBase) {
        return false;
    }
    const uint64_t offset = static_cast<uint64_t>(address - mappedBase);
    if (offset > size || byteCount > size - static_cast<std::size_t>(offset)) {
        return false;
    }
    out = data + static_cast<std::size_t>(offset);
    return true;
}

bool ReadComod0U16(const uint8_t* data,
                   std::size_t size,
                   uint32_t mappedBase,
                   uint32_t address,
                   uint16_t& out)
{
    const uint8_t* bytes = nullptr;
    if (!ReadComod0Range(data, size, mappedBase, address, 2u, bytes)) {
        return false;
    }
    out = static_cast<uint16_t>(bytes[0]) |
          static_cast<uint16_t>(bytes[1]) << 8u;
    return true;
}

bool ReadComod0U32(const uint8_t* data,
                   std::size_t size,
                   uint32_t mappedBase,
                   uint32_t address,
                   uint32_t& out)
{
    const uint8_t* bytes = nullptr;
    if (!ReadComod0Range(data, size, mappedBase, address, 4u, bytes)) {
        return false;
    }
    out = static_cast<uint32_t>(bytes[0]) |
          static_cast<uint32_t>(bytes[1]) << 8u |
          static_cast<uint32_t>(bytes[2]) << 16u |
          static_cast<uint32_t>(bytes[3]) << 24u;
    return true;
}

bool ReadComod0S16(const uint8_t* data,
                   std::size_t size,
                   uint32_t mappedBase,
                   uint32_t address,
                   int16_t& out)
{
    uint16_t value = 0u;
    if (!ReadComod0U16(data, size, mappedBase, address, value)) {
        return false;
    }
    out = static_cast<int16_t>(value);
    return true;
}

bool ReadComod0EventRecord(const uint8_t* data,
                           std::size_t size,
                           uint32_t mappedBase,
                           uint32_t address,
                           TitleHudEventRecord& out)
{
    if (!ReadComod0U32(data, size, mappedBase, address, out.frame) ||
        !ReadComod0U32(data, size, mappedBase, address + 4u, out.flags)) {
        return false;
    }
    const uint8_t* bytes = nullptr;
    if (!ReadComod0Range(data, size, mappedBase, address + 8u, 8u, bytes)) {
        return false;
    }
    std::memcpy(out.bytes, bytes, sizeof(out.bytes));
    return true;
}

bool ReadComod0TimelineRecord(const uint8_t* data,
                              std::size_t size,
                              uint32_t mappedBase,
                              uint32_t address,
                              TitleHudTimelineRecord801C5094& out)
{
    if (!ReadComod0U32(data, size, mappedBase, address, out.deltaFrame)) {
        return false;
    }
    for (std::size_t index = 0u; index < kHudTimelineMaxTimIds; ++index) {
        if (!ReadComod0S16(data,
                           size,
                           mappedBase,
                           address + 4u +
                               static_cast<uint32_t>(index * sizeof(int16_t)),
                           out.timIds[index])) {
            return false;
        }
    }
    return true;
}

bool ReadComod0SelectorRecord(const uint8_t* data,
                              std::size_t size,
                              uint32_t mappedBase,
                              uint32_t address,
                              TitleHudSelectorRecord& out)
{
    return ReadComod0S16(data, size, mappedBase, address, out.idA) &&
           ReadComod0S16(data, size, mappedBase, address + 2u, out.idB) &&
           ReadComod0U16(data, size, mappedBase, address + 4u, out.ticks) &&
           ReadComod0S16(data, size, mappedBase, address + 6u,
                         out.hudSlotId);
}

static constexpr uint32_t kKnownEventFlagMask801C5190 =
    0x00000080u | 0x00000020u | 0x00040000u | 0x00010000u |
    0x00400000u | 0x00800000u | 0x02000000u | 0x00000100u |
    0x01000000u;

static bool IsKnownCompleteTitleMimePairIndex801C6C14(uint8_t pairIndex)
{
    return pairIndex >= 1u && pairIndex <= 8u;
}

static bool IsTitlePairTableIndexInBounds801C6C14(uint8_t pairIndex)
{
    return pairIndex < kResourcePairCount801C6C14;
}

static bool Append(TitleHudPlan& plan, const TitleHudAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

static void AppendAction(TitleHudPlan& plan,
                         TitleHudActionKind kind,
                         uint32_t psxFunction,
                         TitleHudTableKind table =
                             TitleHudTableKind::Unknown,
                         uint32_t index = 0,
                         uint32_t arg0 = 0,
                         uint32_t arg1 = 0,
                         uint32_t arg2 = 0,
                         uint32_t arg3 = 0,
                         bool conditional = false)
{
    TitleHudAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.table = table;
    action.index = index;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.arg2 = arg2;
    action.arg3 = arg3;
    action.conditional = conditional;
    (void)Append(plan, action);
}

static void AppendPlan(TitleHudPlan& dst, const TitleHudPlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.hasOpenP0Gap = dst.hasOpenP0Gap || src.hasOpenP0Gap;
}

static TitleHudPlan MakePlan()
{
    TitleHudPlan plan{};
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    return plan;
}

static uint32_t SelectorIndex(uint32_t cursor, uint32_t bank)
{
    return (bank != 0 ? 2u : 0u) + (cursor & 1u);
}

static bool ActivateHudSlot801C57E0(
    TitleEventRuntimeState801C5190& state,
    uint8_t slotId,
    uint32_t currentTick96)
{
    if (slotId == 0u || slotId >= KnownHudTimelineDescriptorCount() ||
        KnownHudTimelineRecordCount(slotId) == 0u) {
        return false;
    }
    state.hudSlot0Known = true;
    state.hudSlot0 = slotId;
    state.hudSlot0BaseTick96Known = true;
    state.hudSlot0BaseTick96 = currentTick96;
    state.hudTimelineChannel0.descriptorAvailable = true;
    state.hudTimelineChannel0.cursorKnown = true;
    state.hudTimelineChannel0.cursor = 0u;
    return true;
}

static void AppendHudSlotStart(TitleHudPlan& plan,
                               uint32_t psxFunction,
                               uint32_t slotId,
                               uint32_t ctxFrame)
{
    AppendAction(plan, TitleHudActionKind::SetHudSlot, psxFunction,
                 TitleHudTableKind::HudTimelines, slotId, slotId);
    AppendAction(plan, TitleHudActionKind::SetHudSlotBaseFrame,
                 psxFunction, TitleHudTableKind::HudTimelines,
                 slotId, ctxFrame);
    AppendAction(plan, TitleHudActionKind::ResetHudTimelineCursor,
                 psxFunction, TitleHudTableKind::HudTimelines, slotId);
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

TitleNaturalLoopCadenceTick801C4894 TickTitleNaturalLoopCadence801C4894(
    TitleNaturalLoopCadenceState801C4894& state,
    bool holdOneHostTick)
{
    TitleNaturalLoopCadenceTick801C4894 tick{};
    // The original 801C4894 outer loop waits for two PSX VBlanks, which is
    // one 30 Hz host-logic tick in this runtime.  The optional hold remains
    // available for callers that explicitly model one VBlank at a time;
    // SS0's live title loop passes false and gates only the 15 fps MDEC
    // picture source separately.
    if (!holdOneHostTick) {
        state.holdTicksRemaining = 0u;
        tick.processIteration = true;
        return tick;
    }
    if (state.holdTicksRemaining != 0u) {
        --state.holdTicksRemaining;
        tick.holdTick = true;
        return tick;
    }

    state.holdTicksRemaining = 1u;
    tick.processIteration = true;
    return tick;
}

TitleEventClock801C4894 ComputeTitleEventClock801C4894(
    int32_t loopTickV10,
    bool initialTick96Known,
    uint32_t initialTick96)
{
    TitleEventClock801C4894 clock{};
    clock.loopTickV10 = loopTickV10;

    if (loopTickV10 <= 0) {
        clock.known = initialTick96Known;
        clock.tick96 = initialTick96Known ? initialTick96 : 0u;
        if (clock.known) {
            const uint32_t tickInBar = clock.tick96 % 384u;
            clock.beatKnown = true;
            clock.beat = static_cast<uint8_t>(tickInBar / 96u + 1u);
            clock.tickWithinBeat =
                static_cast<uint8_t>(tickInBar % 96u + 1u);
        }
        return clock;
    }

    constexpr int64_t kTitleClockNumerator801C4894 = 9792;
    constexpr int64_t kTitleClockDenominator801C4894 = 1800;
    const int64_t tick96 =
        (kTitleClockNumerator801C4894 * loopTickV10) /
        kTitleClockDenominator801C4894;
    if (tick96 < 0 ||
        tick96 > static_cast<int64_t>(
                     std::numeric_limits<uint32_t>::max())) {
        return clock;
    }

    clock.known = true;
    clock.tick96 = static_cast<uint32_t>(tick96);
    const uint32_t tickInBar = clock.tick96 % 384u;
    clock.beatKnown = true;
    clock.beat = static_cast<uint8_t>(tickInBar / 96u + 1u);
    clock.tickWithinBeat =
        static_cast<uint8_t>(tickInBar % 96u + 1u);
    return clock;
}

void ResetTitleSharedEventBucketState80024E98(
    TitleSharedEventBucketState80024FD0& state)
{
    state = TitleSharedEventBucketState80024FD0{};
    state.resetKnown = true;
}

void ResetTitleEventRuntimeState80024E98(
    TitleEventRuntimeState801C5190& state)
{
    state = TitleEventRuntimeState801C5190{};
}

void ResetTitleAbsoluteEventStreamState80024E98(
    TitleAbsoluteEventStreamState801C5538& state)
{
    // sub_80024E98 clears the shared event area, then publishes
    // g_PrStageEventStreamFlag=1 and g_PrStageEventStreamId=0.  These are
    // translated state owners; no PSX memory or host shell is implied.
    state = TitleAbsoluteEventStreamState801C5538{};
    state.resetKnown = true;
    state.streamFlagKnown = true;
    state.streamFlag = 1u;
    state.streamIdKnown = true;
    state.streamId = 0u;
}

TitleSharedEventBucketResult80024FD0 TickTitleSharedEventBucketPrefix80024FD0(
    const TitleSharedEventBucketState80024FD0& state,
    const TitleSharedEventBucketInput80024FD0& input)
{
    TitleSharedEventBucketResult80024FD0 result{};
    result.nextState = state;
    if (!state.resetKnown || !input.callsiteKnown ||
        !input.ctxTick96Known) {
        return result;
    }

    if (input.callsite != kCallsite801C4C9C &&
        input.callsite != kCallsite801C4D14) {
        result.status =
            TitleSharedEventBucketStatus80024FD0::UnsupportedCallsite;
        return result;
    }

    result.accepted = true;
    if (state.dword8008ED20 == 1u) {
        result.status = TitleSharedEventBucketStatus80024FD0::Disabled;
        result.earlyReturnEd20 = true;
        return result;
    }

    TitleSharedEventBucketState80024FD0 next = state;
    const uint32_t tickInBar = input.ctxTick96 % 384u;
    const uint32_t bucket = tickInBar / 12u;
    next.dword8008ECE8 = tickInBar;
    next.dword8008ECF4 = bucket;
    result.prefixApplied = true;
    if (state.dword8008ECF0 == bucket) {
        result.status =
            TitleSharedEventBucketStatus80024FD0::BucketUnchanged;
        result.nextState = next;
        return result;
    }

    next.dword8008ECF0 = bucket;
    result.status = TitleSharedEventBucketStatus80024FD0::BucketAdvanced;
    result.bucketChanged = true;
    result.sharedTailRequired = true;
    result.nextState = next;
    return result;
}

TitleAttractTimeoutPolicyResult801C48C0
ResolveTitleAttractTimeoutPolicy801C48C0(
    const TitleAttractTimeoutPolicyInput801C48C0& input)
{
    TitleAttractTimeoutPolicyResult801C48C0 result{};
    if (!input.word800916FCKnown) {
        return result;
    }

    result.accepted = true;
    result.timeoutKnown = true;
    if (input.word800916FC == 1u) {
        result.status = TitleAttractTimeoutPolicyStatus801C48C0::Short450;
        result.timeoutFrames = kTitleAttractTimeoutShort801C4894;
        return result;
    }

    result.status = TitleAttractTimeoutPolicyStatus801C48C0::Default1800;
    result.timeoutFrames = kTitleAttractTimeoutDefault801C4894;
    return result;
}

TitleEventEffect801C5190 DecodeTitleEventEffect801C5190(
    uint32_t eventIndex)
{
    TitleEventEffect801C5190 effect{};
    if (eventIndex >= KnownEventStreamRecordCount()) {
        return effect;
    }

    const TitleHudEventRecord record = KnownEventStreamRecordAt(eventIndex);
    if ((record.flags & ~kKnownEventFlagMask801C5190) != 0u) {
        return effect;
    }

    effect.eventIndex = eventIndex;
    effect.thresholdTick96 = record.frame;
    effect.eventStatus80SetOne = (record.flags & 0x00000080u) != 0u;

    if ((record.flags & 0x00000020u) != 0u) {
        effect.ctxFlagsSetMask |= 0x00000020u;
    }
    if ((record.flags & 0x00040000u) != 0u && record.bytes[1] != 0u) {
        effect.ctxFlagsSetMask |= 0x00040000u;
        effect.todResourceF4Write = true;
        effect.todResourceF4Index = record.bytes[1];
    }
    if ((record.flags & 0x00010000u) != 0u) {
        if (!IsKnownCompleteTitleMimePairIndex801C6C14(record.bytes[2])) {
            return TitleEventEffect801C5190{};
        }
        effect.ctxFlagsSetMask |= 0x00010000u;
        effect.channel2PairWrite = true;
        effect.channel2PairIndex = record.bytes[2];
    }
    if (record.bytes[0] != 0u) {
        effect.ctxFlagsSetMask |= 0x00000400u;
        effect.objectResource104Write = true;
        effect.objectResource104Index = record.bytes[0];
    }
    if ((record.flags & 0x00400000u) != 0u) {
        if (!IsTitlePairTableIndexInBounds801C6C14(record.bytes[5])) {
            return TitleEventEffect801C5190{};
        }
        effect.ctxFlagsSetMask |= 0x00400000u;
        effect.channel1PairWrite = true;
        effect.channel1PairIndex = record.bytes[5];
    }
    if ((record.flags & 0x00800000u) != 0u) {
        effect.ctxFlagsSetMask |= 0x00800000u;
        effect.extraResourceFCWrite = true;
        effect.extraResourceFCIndex = record.bytes[4];
    }
    if ((record.flags & 0x02000000u) != 0u) {
        effect.ctxFlagsSetMask |= 0x02000000u;
    }
    if ((record.flags & 0x00000100u) != 0u) {
        effect.ctxFlagsSetMask |= 0x00000100u;
    }
    if ((record.flags & 0x01000000u) != 0u) {
        if (!IsTitlePairTableIndexInBounds801C6C14(record.bytes[6])) {
            return TitleEventEffect801C5190{};
        }
        effect.ctxFlagsSetMask |= 0x01000000u;
        effect.channel3PairWrite = true;
        effect.channel3PairIndex = record.bytes[6];
    }
    if (record.bytes[3] != 0u) {
        effect.hudSlot0Write = true;
        effect.hudSlot0 = record.bytes[3];
    }

    effect.known = true;
    return effect;
}

TitleEventRuntimeCommitResult801C5190 BuildTitleEventRuntimeCommit801C5190(
    const TitleEventRuntimeState801C5190& state,
    const TitleEventEffect801C5190& effect)
{
    TitleEventRuntimeCommitResult801C5190 result{};
    result.nextState = state;
    if (!effect.known || !effect.appliedTick96Known ||
        effect.appliedTick96 < effect.thresholdTick96) {
        return result;
    }

    TitleEventRuntimeState801C5190 next = state;
    next.ctxFlags |= effect.ctxFlagsSetMask;
    if (effect.eventStatus80SetOne) {
        next.eventStatus80Known = true;
        next.eventStatus80 = true;
    }
    if (effect.objectResource104Write) {
        next.objectResource104Known = true;
        next.objectResource104Index = effect.objectResource104Index;
    }
    if (effect.todResourceF4Write) {
        next.todResourceF4Known = true;
        next.todResourceF4Index = effect.todResourceF4Index;
    }
    if (effect.channel2PairWrite) {
        next.channel2PairKnown = true;
        next.channel2PairIndex = effect.channel2PairIndex;
    }
    if (effect.extraResourceFCWrite) {
        next.extraResourceFCKnown = true;
        next.extraResourceFCIndex = effect.extraResourceFCIndex;
    }
    if (effect.channel1PairWrite) {
        next.channel1PairKnown = true;
        next.channel1PairIndex = effect.channel1PairIndex;
    }
    if (effect.channel3PairWrite) {
        next.channel3PairKnown = true;
        next.channel3PairIndex = effect.channel3PairIndex;
    }
    if (effect.hudSlot0Write) {
        next.hudSlot0Known = true;
        next.hudSlot0 = effect.hudSlot0;
        next.hudSlot0BaseTick96Known = true;
        next.hudSlot0BaseTick96 = effect.appliedTick96;
        next.hudTimelineChannel0.descriptorAvailable =
            effect.hudSlot0 != 0u &&
            effect.hudSlot0 < KnownHudTimelineDescriptorCount() &&
            KnownHudTimelineRecordCount(effect.hudSlot0) != 0u;
        next.hudTimelineChannel0.cursorKnown = true;
        next.hudTimelineChannel0.cursor = 0u;
    }

    result.accepted = true;
    result.nextState = next;
    return result;
}

TitleHudTimelineTickResult801C5094 TickTitleHudTimeline801C5094(
    const TitleEventRuntimeState801C5190& state,
    const TitleHudTimelineTickInput801C5094& input)
{
    TitleHudTimelineTickResult801C5094 result{};
    result.nextState = state;

    if (!input.channelKnown || !input.currentTick96Known ||
        !state.hudSlot0Known || !state.hudSlot0BaseTick96Known ||
        !state.hudTimelineChannel0.cursorKnown) {
        return result;
    }
    if (input.channel != kCtxRuntimeTimListObservedChannel) {
        result.status =
            TitleHudTimelineTickStatus801C5094::UnsupportedChannel;
        return result;
    }

    const uint32_t slotId = state.hudSlot0;
    if (slotId == 0u || slotId >= KnownHudTimelineDescriptorCount()) {
        result.status = TitleHudTimelineTickStatus801C5094::InvalidSlot;
        return result;
    }
    if (!state.hudTimelineChannel0.descriptorAvailable) {
        result.status =
            TitleHudTimelineTickStatus801C5094::DescriptorUnavailable;
        return result;
    }

    const TitleHudTimelineDescriptor descriptor =
        KnownHudTimelineDescriptorAt(slotId);
    const uint32_t recordCount = KnownHudTimelineRecordCount(slotId);
    if (descriptor.eventsPtr == 0u || descriptor.count == 0u ||
        descriptor.count != recordCount) {
        result.status =
            TitleHudTimelineTickStatus801C5094::DescriptorUnavailable;
        return result;
    }

    const uint32_t cursor = state.hudTimelineChannel0.cursor;
    if (cursor > recordCount) {
        result.status = TitleHudTimelineTickStatus801C5094::InvalidCursor;
        return result;
    }
    if (cursor == recordCount) {
        result.nextState.hudSlot0Known = false;
        result.nextState.hudSlot0 = 0u;
        result.nextState.hudTimelineChannel0.descriptorAvailable = false;
        result.nextState.hudTimelineChannel0.cursorKnown = true;
        result.nextState.hudTimelineChannel0.cursor = 0u;
        result.status = TitleHudTimelineTickStatus801C5094::Complete;
        return result;
    }

    const TitleHudTimelineRecord801C5094 record =
        KnownHudTimelineRecordAt(slotId, cursor);
    if (state.hudSlot0BaseTick96 >
        std::numeric_limits<uint32_t>::max() - record.deltaFrame) {
        result.status =
            TitleHudTimelineTickStatus801C5094::ThresholdOverflow;
        return result;
    }

    const uint32_t thresholdTick96 =
        state.hudSlot0BaseTick96 + record.deltaFrame;
    if (input.currentTick96 < thresholdTick96) {
        result.status = TitleHudTimelineTickStatus801C5094::NotDue;
        return result;
    }

    result.nextState.hudTimelineChannel0.cursor = cursor + 1u;
    result.nextState.ctxFlags |= kCtxFlagRuntimeTimList;
    result.nextState.hudTimelineChannel0.emittedTimIdCount = 0u;
    for (uint32_t i = 0; i < kHudTimelineMaxTimIds; ++i) {
        result.nextState.hudTimelineChannel0.emittedTimIds[i] = 0;
    }

    result.request.valid = true;
    result.request.slotId = static_cast<uint8_t>(slotId);
    result.request.recordIndex = cursor;
    result.request.baseTick96 = state.hudSlot0BaseTick96;
    result.request.appliedTick96 = input.currentTick96;
    for (uint32_t i = 0; i < kHudTimelineMaxTimIds; ++i) {
        const int16_t timId = record.timIds[i];
        if (timId == 0) {
            break;
        }
        result.request.timIds[result.request.timIdCount++] = timId;
        result.nextState.hudTimelineChannel0.emittedTimIds[
            result.nextState.hudTimelineChannel0.emittedTimIdCount++] = timId;
    }

    result.status = TitleHudTimelineTickStatus801C5094::Applied;
    return result;
}

TitleSelectorHudStartResult801C57E0 StartTitleSelectorHud801C57E0(
    const TitleEventRuntimeState801C5190& eventState,
    const TitleSelectorHudRuntimeState801C5854& selectorState,
    bool currentTick96Known,
    uint32_t currentTick96)
{
    TitleSelectorHudStartResult801C57E0 result{};
    result.nextEventState = eventState;
    result.nextSelectorState = selectorState;
    if (!currentTick96Known ||
        !ActivateHudSlot801C57E0(
            result.nextEventState,
            kTitleSelectorInitialHudSlot801C57E0,
            currentTick96)) {
        return result;
    }

    result.nextSelectorState.known = true;
    result.nextSelectorState.bank = kTitleSelectorInitialBank801C57E0;
    result.nextSelectorState.lastCursor =
        kTitleSelectorInitialLastCursor801C57E0;
    result.nextSelectorState.timelineEnabled = true;
    result.nextSelectorState.timelineCountdown =
        kTitleSelectorInitialCountdown801C57E0;
    result.accepted = true;
    return result;
}

TitleSelectorHudTickResult801C5854 TickTitleSelectorHud801C5854(
    const TitleEventRuntimeState801C5190& eventState,
    const TitleSelectorHudRuntimeState801C5854& selectorState,
    const TitleSelectorHudTickInput801C5854& input)
{
    TitleSelectorHudTickResult801C5854 result{};
    result.nextEventState = eventState;
    result.nextSelectorState = selectorState;
    if (!input.currentTick96Known || !input.beatKnown ||
        !input.cursorKnown || !selectorState.known ||
        !selectorState.resourceCooldownKnown) {
        return result;
    }
    if (input.beat > 4u) {
        result.status = TitleSelectorHudTickStatus801C5854::InvalidBeat;
        return result;
    }
    if (input.cursor > 1u) {
        result.status = TitleSelectorHudTickStatus801C5854::InvalidCursor;
        return result;
    }
    if (selectorState.bank > 1u ||
        selectorState.timelineCountdown < 0 ||
        selectorState.timelineCountdown >
            kTitleSelectorInitialCountdown801C57E0 ||
        selectorState.resourceCooldown <
            (std::numeric_limits<int16_t>::min)() ||
        selectorState.resourceCooldown >
            (std::numeric_limits<int16_t>::max)()) {
        result.status = TitleSelectorHudTickStatus801C5854::InvalidState;
        return result;
    }

    if (input.cursor != result.nextSelectorState.lastCursor) {
        result.nextSelectorState.bank = 0u;
    }
    result.selectorIndex =
        SelectorIndex(input.cursor, result.nextSelectorState.bank);
    const TitleHudSelectorRecord selector =
        KnownSelectorRecordAt(result.selectorIndex);
    const uint8_t resourcePairIndex =
        KnownSelectorResourcePairIndexAt(result.selectorIndex);
    if (selector.idA == 0 || selector.idB == 0 || selector.ticks == 0u ||
        selector.hudSlotId <= 0 || resourcePairIndex == 0u) {
        result.status =
            TitleSelectorHudTickStatus801C5854::InvalidSelectorRecord;
        return result;
    }

    if (result.nextSelectorState.bank == 0u) {
        if (!ActivateHudSlot801C57E0(
                result.nextEventState,
                static_cast<uint8_t>(selector.hudSlotId),
                input.currentTick96)) {
            result.status =
                TitleSelectorHudTickStatus801C5854::InvalidSelectorRecord;
            return result;
        }
        result.nextSelectorState.timelineEnabled = true;
        result.nextSelectorState.timelineCountdown =
            kTitleSelectorInitialCountdown801C57E0;
        ++result.nextSelectorState.bank;
    } else {
        if (input.beat == 3u) {
            result.nextSelectorState.timelineEnabled = false;
        }
        if (result.nextSelectorState.timelineCountdown <= 0 &&
            input.beat == 4u) {
            result.nextSelectorState.timelineEnabled = true;
            if (!ActivateHudSlot801C57E0(
                    result.nextEventState,
                    static_cast<uint8_t>(selector.hudSlotId),
                    input.currentTick96)) {
                result.status =
                    TitleSelectorHudTickStatus801C5854::
                        InvalidSelectorRecord;
                return result;
            }
        }
    }

    if (result.nextSelectorState.timelineEnabled &&
        result.nextEventState.hudSlot0Known) {
        TitleHudTimelineTickInput801C5094 timelineInput{};
        timelineInput.channelKnown = true;
        timelineInput.channel = kCtxRuntimeTimListObservedChannel;
        timelineInput.currentTick96Known = true;
        timelineInput.currentTick96 = input.currentTick96;
        const auto timeline = TickTitleHudTimeline801C5094(
            result.nextEventState, timelineInput);
        if (timeline.status == TitleHudTimelineTickStatus801C5094::Applied ||
            timeline.status == TitleHudTimelineTickStatus801C5094::NotDue ||
            timeline.status == TitleHudTimelineTickStatus801C5094::Complete) {
            result.nextEventState = timeline.nextState;
            result.request = timeline.request;
        } else {
            result.status =
                TitleSelectorHudTickStatus801C5854::TimelineRejected;
            return result;
        }
    }

    if (result.nextSelectorState.timelineCountdown > 0) {
        --result.nextSelectorState.timelineCountdown;
    }
    if (result.nextSelectorState.resourceCooldown > 0) {
        --result.nextSelectorState.resourceCooldown;
        if (result.nextSelectorState.resourceCooldown > 0) {
            result.status = TitleSelectorHudTickStatus801C5854::Applied;
            return result;
        }
    }

    result.selectorMimeEffect.known = true;
    result.selectorMimeEffect.eventIndex = result.selectorIndex;
    result.selectorMimeEffect.thresholdTick96 = input.currentTick96;
    result.selectorMimeEffect.appliedTick96Known = true;
    result.selectorMimeEffect.appliedTick96 = input.currentTick96;
    result.selectorMimeEffect.ctxFlagsSetMask = kCtxFlagSelectorMime;
    result.selectorMimeEffect.channel2PairWrite = true;
    result.selectorMimeEffect.channel2PairIndex = resourcePairIndex;
    const auto commit = BuildTitleEventRuntimeCommit801C5190(
        result.nextEventState, result.selectorMimeEffect);
    if (!commit.accepted) {
        result.status =
            TitleSelectorHudTickStatus801C5854::InvalidSelectorRecord;
        return result;
    }
    result.nextEventState = commit.nextState;
    result.selectorResourceUpdate.known = true;
    result.selectorResourceUpdate.selectorIndex = result.selectorIndex;
    result.selectorResourceUpdate.idA = selector.idA;
    result.selectorResourceUpdate.idB = selector.idB;
    result.selectorResourceUpdate.targetOffsetA =
        kCtxSelectorResourceOffsetDC;
    result.selectorResourceUpdate.targetOffsetB =
        kCtxSelectorResourceOffsetE8;
    result.selectorResourceUpdate.appliedTick96Known = true;
    result.selectorResourceUpdate.appliedTick96 = input.currentTick96;
    result.nextSelectorState.resourceCooldown = selector.ticks;
    result.nextSelectorState.lastCursor = input.cursor;
    result.status = TitleSelectorHudTickStatus801C5854::Applied;
    return result;
}

TitleSelectorInputResult801C47EC ResolveTitleSelectorInput801C47EC(
    bool inputMaskKnown,
    uint16_t inputMask,
    bool cursorKnown,
    uint8_t cursor)
{
    TitleSelectorInputResult801C47EC result{};
    if (!inputMaskKnown || !cursorKnown) {
        return result;
    }
    if (cursor > 1u) {
        result.status = TitleSelectorInputStatus801C47EC::InvalidCursor;
        return result;
    }

    result.nextCursor = cursor;
    if ((inputMask & kTitleSelectorInputConfirmMask801C47EC) != 0u) {
        result.playCue = true;
        result.cueMask = kTitleSelectorInputConfirmCue801C47EC;
    } else if ((inputMask & kTitleSelectorInputMoveMask801C47EC) != 0u) {
        result.playCue = true;
        result.cueMask = kTitleSelectorInputMoveCue801C47EC;
    }

    if (inputMask == kTitleSelectorInputConfirmMask801C47EC) {
        result.selectorResult = cursor == 0u
            ? kTitleSelectorResultStart801C47EC
            : kTitleSelectorResultMenu801C47EC;
    } else if (inputMask == kTitleSelectorInputLeftMask801C47EC ||
               inputMask == kTitleSelectorInputRightMask801C47EC) {
        result.cursorWrite = true;
        result.nextCursor = cursor == 0u ? 1u : 0u;
    }

    result.status = TitleSelectorInputStatus801C47EC::Applied;
    result.accepted = true;
    return result;
}

TitleSelectorInputEdgeResult801C4CE0 ResolveTitleSelectorInputEdge801C4CE0(
    bool previousInputKnown,
    uint16_t previousInput,
    bool currentInputKnown,
    uint16_t currentInput,
    bool cursorKnown,
    uint8_t cursor)
{
    TitleSelectorInputEdgeResult801C4CE0 result{};
    result.nextPreviousInput = previousInput;
    if (!previousInputKnown || !currentInputKnown || !cursorKnown) {
        return result;
    }
    if (previousInput == currentInput) {
        result.accepted = true;
        return result;
    }

    result.action = ResolveTitleSelectorInput801C47EC(
        true, currentInput, true, cursor);
    if (!result.action.accepted) {
        return result;
    }
    result.inputChanged = true;
    result.resetTimeout = currentInput != 0u;
    result.nextPreviousInput = currentInput;
    result.accepted = true;
    return result;
}

TitleShortcutHudResetTransaction801C5AB4
BuildShortcutTitleSelectorHudStartAndFirstTick801C5AB4()
{
    TitleShortcutHudResetTransaction801C5AB4 transaction{};
    transaction.reset80024E98Applied = true;

    TitleEventRuntimeState801C5190 shortcutEventState{};
    ResetTitleEventRuntimeState80024E98(shortcutEventState);
    shortcutEventState.ctxFlags = kCtxFlagShortcutStart;
    shortcutEventState.todResourceF4Known = true;
    shortcutEventState.todResourceF4Index =
        kTitleShortcutTodResourceF4Index801C5AB4;
    TitleSelectorHudRuntimeState801C5854 selectorState{};
    selectorState.resourceCooldownKnown = true;
    selectorState.resourceCooldown = 0;
    const auto start = StartTitleSelectorHud801C57E0(
        shortcutEventState,
        selectorState,
        true,
        transaction.firstTickCtxTick96);
    if (!start.accepted) {
        return transaction;
    }

    TitleSelectorHudTickInput801C5854 firstTickInput{};
    firstTickInput.currentTick96Known = true;
    firstTickInput.currentTick96 = transaction.firstTickCtxTick96;
    firstTickInput.beatKnown = true;
    firstTickInput.beat = transaction.firstTickBeatDerived;
    firstTickInput.cursorKnown = true;
    firstTickInput.cursor = transaction.firstTickCursor;
    transaction.firstTick = TickTitleSelectorHud801C5854(
        start.nextEventState, start.nextSelectorState, firstTickInput);
    if (transaction.firstTick.status !=
        TitleSelectorHudTickStatus801C5854::Applied) {
        return transaction;
    }

    transaction.firstTick.nextEventState.ctxFlags =
        kTitleShortcutFirstTickCtxFlags801C5AB4;
    transaction.callerReadyFlagApplied = true;
    return transaction;
}

TitleSelectorExitWaitTickResult801C4894
TickTitleSelectorExitWait801C4894(uint32_t waitCounter,
                                  uint32_t frameWaitRemaining)
{
    TitleSelectorExitWaitTickResult801C4894 result{};
    result.nextWaitCounter = waitCounter;
    result.nextFrameWaitRemaining = frameWaitRemaining;
    if (waitCounter > kTitleSelectorExitWaitInitialCounter801C4894 ||
        frameWaitRemaining >= kTitleSelectorExitWaitFrameWait801C4894 ||
        (waitCounter == 0u && frameWaitRemaining != 0u)) {
        return result;
    }
    if (waitCounter == 0u) {
        result.accepted = true;
        result.complete = true;
        result.status = TitleSelectorExitWaitTickStatus801C4894::Complete;
        return result;
    }
    if (frameWaitRemaining != 0u) {
        --result.nextFrameWaitRemaining;
        result.accepted = true;
        result.status = TitleSelectorExitWaitTickStatus801C4894::FrameWait;
        return result;
    }

    --result.nextWaitCounter;
    result.renderFrame = true;
    result.complete = result.nextWaitCounter == 0u;
    if (result.complete) {
        result.status =
            TitleSelectorExitWaitTickStatus801C4894::RenderAndComplete;
    } else {
        result.nextFrameWaitRemaining =
            kTitleSelectorExitWaitFrameWait801C4894 - 1u;
        result.status =
            TitleSelectorExitWaitTickStatus801C4894::RenderAndWait;
    }
    result.accepted = true;
    return result;
}

TitleSelectorExitWaitPresentGateResult801C4D58
TickTitleSelectorExitWaitPresentGate801C4D58(
    const TitleSelectorExitWaitPresentGateState801C4D58& state)
{
    TitleSelectorExitWaitPresentGateResult801C4D58 result{};
    result.nextState = state;
    if (state.presentPending) {
        if (state.pendingNextWaitCounter >
                kTitleSelectorExitWaitInitialCounter801C4894 ||
            state.pendingNextFrameWaitRemaining >=
                kTitleSelectorExitWaitFrameWait801C4894 ||
            (state.pendingComplete &&
             (state.pendingNextWaitCounter != 0u ||
              state.pendingNextFrameWaitRemaining != 0u))) {
            return result;
        }
        if (!state.presentAcknowledged) {
            result.status =
                TitleSelectorExitWaitPresentGateStatus801C4D58::AwaitPresent;
            result.accepted = true;
            return result;
        }

        result.nextState.waitCounter = state.pendingNextWaitCounter;
        result.nextState.frameWaitRemaining =
            state.pendingNextFrameWaitRemaining;
        result.nextState.presentPending = false;
        result.nextState.presentAcknowledged = false;
        result.nextState.pendingNextWaitCounter = 0u;
        result.nextState.pendingNextFrameWaitRemaining = 0u;
        result.nextState.pendingComplete = false;
        if (state.pendingComplete) {
            result.status =
                TitleSelectorExitWaitPresentGateStatus801C4D58::Complete;
            result.transitionReady = true;
            result.accepted = true;
            return result;
        }

        const auto frameWait = TickTitleSelectorExitWait801C4894(
            result.nextState.waitCounter,
            result.nextState.frameWaitRemaining);
        if (!frameWait.accepted || frameWait.renderFrame || frameWait.complete) {
            return TitleSelectorExitWaitPresentGateResult801C4D58{};
        }
        result.nextState.waitCounter = frameWait.nextWaitCounter;
        result.nextState.frameWaitRemaining =
            frameWait.nextFrameWaitRemaining;
        result.status = TitleSelectorExitWaitPresentGateStatus801C4D58::
            PresentCommitted;
        result.accepted = true;
        return result;
    }

    if (state.presentAcknowledged || state.pendingNextWaitCounter != 0u ||
        state.pendingNextFrameWaitRemaining != 0u || state.pendingComplete) {
        return result;
    }
    const auto cadence = TickTitleSelectorExitWait801C4894(
        state.waitCounter, state.frameWaitRemaining);
    if (!cadence.accepted) {
        return result;
    }
    if (cadence.renderFrame) {
        result.nextState.presentPending = true;
        result.nextState.pendingNextWaitCounter = cadence.nextWaitCounter;
        result.nextState.pendingNextFrameWaitRemaining =
            cadence.nextFrameWaitRemaining;
        result.nextState.pendingComplete = cadence.complete;
        result.status =
            TitleSelectorExitWaitPresentGateStatus801C4D58::PreparePresent;
        result.preparePresent = true;
        result.accepted = true;
        return result;
    }

    result.nextState.waitCounter = cadence.nextWaitCounter;
    result.nextState.frameWaitRemaining = cadence.nextFrameWaitRemaining;
    result.transitionReady = cadence.complete;
    result.status = cadence.complete
        ? TitleSelectorExitWaitPresentGateStatus801C4D58::Complete
        : TitleSelectorExitWaitPresentGateStatus801C4D58::FrameWait;
    result.accepted = true;
    return result;
}

TitleSelectorExitWaitPresentAckResult801C4D74
AcknowledgeTitleSelectorExitWaitPresent801C4D74(
    const TitleSelectorExitWaitPresentGateState801C4D58& state,
    bool selectorPromptDrawn,
    bool titlePacketFrameSubmitted,
    bool presentModelApplied)
{
    TitleSelectorExitWaitPresentAckResult801C4D74 result{};
    result.nextState = state;
    if (!state.presentPending || state.presentAcknowledged ||
        !selectorPromptDrawn || !titlePacketFrameSubmitted ||
        !presentModelApplied) {
        return result;
    }
    result.nextState.presentAcknowledged = true;
    result.accepted = true;
    return result;
}

TitleEarlyInputShortcutHoldTickResult801C4894
TickTitleEarlyInputShortcutHold801C4894(uint32_t completedWaitCalls,
                                        uint32_t frameWaitRemaining)
{
    TitleEarlyInputShortcutHoldTickResult801C4894 result{};
    result.nextCompletedWaitCalls = completedWaitCalls;
    result.nextFrameWaitRemaining = frameWaitRemaining;
    if (completedWaitCalls > kShortcutHoldWaitFrames ||
        frameWaitRemaining > kShortcutFrameWait ||
        (completedWaitCalls < kShortcutHoldWaitFrames &&
         frameWaitRemaining == 0u) ||
        (completedWaitCalls == kShortcutHoldWaitFrames &&
         frameWaitRemaining != 0u)) {
        return result;
    }
    if (completedWaitCalls == kShortcutHoldWaitFrames) {
        result.accepted = true;
        result.ready = true;
        result.status =
            TitleEarlyInputShortcutHoldTickStatus801C4894::Ready;
        return result;
    }

    --result.nextFrameWaitRemaining;
    if (result.nextFrameWaitRemaining == 0u) {
        ++result.nextCompletedWaitCalls;
        if (result.nextCompletedWaitCalls == kShortcutHoldWaitFrames) {
            result.ready = true;
            result.status =
                TitleEarlyInputShortcutHoldTickStatus801C4894::Ready;
        } else {
            result.nextFrameWaitRemaining = kShortcutFrameWait;
            result.status =
                TitleEarlyInputShortcutHoldTickStatus801C4894::Waiting;
        }
    } else {
        result.status =
            TitleEarlyInputShortcutHoldTickStatus801C4894::Waiting;
    }
    result.accepted = true;
    return result;
}

TitleSelectorPromptPlan80020488 BuildTitleSelectorPrompt8001E3E4(
    bool selectorValueKnown,
    uint32_t selectorValue)
{
    TitleSelectorPromptPlan80020488 result{};
    if (!selectorValueKnown) {
        return result;
    }
    if (selectorValue > 1u) {
        result.status =
            TitleSelectorPromptPlanStatus80020488::UnsupportedSelector;
        return result;
    }

    auto makeSprite = [](int16_t x,
                         int16_t y,
                         uint32_t sourceAddress,
                         uint32_t attr,
                         uint16_t texX,
                         uint16_t texY,
                         uint16_t width,
                         uint16_t height,
                         uint16_t clutX,
                         uint16_t clutY) {
        TitleSelectorPromptSprite80020488 sprite{};
        sprite.active = true;
        sprite.x = x;
        sprite.y = y;
        sprite.spriteTemplate.sourceAddress = sourceAddress;
        sprite.spriteTemplate.attr = attr;
        sprite.spriteTemplate.texX = texX;
        sprite.spriteTemplate.texY = texY;
        sprite.spriteTemplate.width = width;
        sprite.spriteTemplate.height = height;
        sprite.spriteTemplate.clutX = clutX;
        sprite.spriteTemplate.clutY = clutY;
        return sprite;
    };

    result.selectorValue = selectorValue;
    result.sprites[0] = makeSprite(
        278, 78, 0x80052FE0u, 0x01000040u, 640u, 20u, 16u, 10u, 320u, 508u);
    result.sprites[1] = makeSprite(
        10, 207, 0x80052FB0u, 0x51000040u, 640u, 40u, 168u, 10u, 320u, 508u);
    result.sprites[2] = makeSprite(
        178, 207, 0x80052FD0u, 0x01000040u, 640u, 30u, 132u, 10u, 320u, 508u);
    result.sprites[3] = makeSprite(
        46, 220, 0x80052FC0u, 0x51000040u, 640u, 50u, 228u, 10u, 320u, 508u);
    if (selectorValue == 1u) {
        result.sprites[4] = makeSprite(
            24, 139, 0x80052F90u, 0x50000040u, 656u, 91u, 68u, 24u, 864u, 282u);
        result.sprites[5] = makeSprite(
            231, 139, 0x80052F80u, 0x50000040u, 640u, 91u, 64u, 24u, 864u, 281u);
    } else {
        result.sprites[4] = makeSprite(
            24, 139, 0x80052FA0u, 0x50000040u, 656u, 91u, 68u, 24u, 864u, 283u);
        result.sprites[5] = makeSprite(
            231, 139, 0x80052F70u, 0x50000040u, 640u, 91u, 64u, 24u, 864u, 280u);
    }
    result.accepted = true;
    result.status = TitleSelectorPromptPlanStatus80020488::Ready;
    return result;
}

TitleAbsoluteEventTickResult801C5538 TickTitleAbsoluteEventStream801C5538(
    const TitleAbsoluteEventStreamState801C5538& state,
    const TitleAbsoluteEventTickInput801C5538& input)
{
    TitleAbsoluteEventTickResult801C5538 result{};
    result.nextState = state;

    if (!state.resetKnown || !state.streamFlagKnown ||
        !state.streamIdKnown || !input.ctxTick96Known ||
        !input.streamFlagKnown || !input.streamIdKnown) {
        return result;
    }
    if (input.streamFlag != state.streamFlag ||
        input.streamId != state.streamId || state.streamFlag != 1u ||
        state.streamId != 0u) {
        result.status =
            TitleAbsoluteEventTickStatus801C5538::UnsupportedMode;
        return result;
    }

    const uint32_t recordCount = KnownEventStreamRecordCount();
    if (state.cursor > recordCount) {
        result.status = TitleAbsoluteEventTickStatus801C5538::InvalidState;
        return result;
    }
    if (state.cursor == recordCount) {
        result.status = TitleAbsoluteEventTickStatus801C5538::Complete;
        return result;
    }

    const TitleHudEventRecord record =
        KnownEventStreamRecordAt(state.cursor);
    if (input.ctxTick96 < record.frame) {
        result.status = TitleAbsoluteEventTickStatus801C5538::NotDue;
        return result;
    }

    TitleEventEffect801C5190 effect =
        DecodeTitleEventEffect801C5190(state.cursor);
    if (!effect.known) {
        result.status = TitleAbsoluteEventTickStatus801C5538::InvalidState;
        return result;
    }

    effect.appliedTick96Known = true;
    effect.appliedTick96 = input.ctxTick96;
    result.nextState.cursor = state.cursor + 1u;
    result.effect = effect;
    result.status = TitleAbsoluteEventTickStatus801C5538::Applied;
    return result;
}

uint32_t KnownEventStreamRecordCount()
{
    return s_comod0SourceState.eventStreamKnown
               ? s_comod0SourceState.eventStreamCount
               : static_cast<uint32_t>(sizeof(kEventStreamRecords) /
                                       sizeof(kEventStreamRecords[0]));
}

TitleHudEventRecord KnownEventStreamRecordAt(uint32_t index)
{
    if (index >= KnownEventStreamRecordCount()) {
        return TitleHudEventRecord{};
    }
    return s_comod0SourceState.eventStreamKnown
               ? s_comod0EventStreamRecords[index]
               : kEventStreamRecords[index];
}

void ResetTitleHudComodSource801C6Dxx()
{
    s_comod0SourceState = TitleHudComodSourceState801C6Dxx{};
    s_comod0EventStreamRecords = {};
    s_comod0TimelineDescriptors = {};
    s_comod0TimelineRecords = {};
    s_comod0TimelineRecordCounts = {};
    s_comod0SelectorRecords = {};
    s_comod0SelectorResourcePairIndices = {};
    s_comod0ResourcePairs = {};
}

bool LoadTitleHudTablesFromComod0(const uint8_t* data,
                                  std::size_t size,
                                  uint32_t mappedBase)
{
    ResetTitleHudComodSource801C6Dxx();
    if (data == nullptr || mappedBase != kComod0MappedBase801C3870) {
        return false;
    }

    uint32_t eventStreamAddress = 0u;
    uint32_t eventStreamCount = 0u;
    uint32_t eventStreamCursor = 0u;
    if (!ReadComod0U32(data,
                       size,
                       mappedBase,
                       kEventStreamDescriptor801C6E50,
                       eventStreamAddress) ||
        !ReadComod0U32(data,
                       size,
                       mappedBase,
                       kEventStreamDescriptor801C6E50 + 4u,
                       eventStreamCount) ||
        !ReadComod0U32(data,
                       size,
                       mappedBase,
                       kEventStreamDescriptor801C6E50 + 8u,
                       eventStreamCursor) ||
        eventStreamAddress != kEventStreamRecords801C6DD4 ||
        eventStreamCount != kEventStreamRecordCount ||
        eventStreamCursor != 0u) {
        return false;
    }

    std::array<TitleHudEventRecord, kEventStreamRecordCount>
        eventRecords{};
    for (uint32_t index = 0u; index < eventStreamCount; ++index) {
        if (!ReadComod0EventRecord(
                data,
                size,
                mappedBase,
                eventStreamAddress + index * kEventStreamRecordBytes,
                eventRecords[index])) {
            return false;
        }
    }

    std::array<std::array<int16_t, 2>, kComod0ResourcePairCount801C6C14>
        resourcePairs{};
    for (std::size_t index = 0u; index < resourcePairs.size(); ++index) {
        const uint32_t address = kResourcePairTable801C6C14 +
                                 static_cast<uint32_t>(index * 4u);
        if (!ReadComod0S16(data, size, mappedBase, address,
                           resourcePairs[index][0]) ||
            !ReadComod0S16(data, size, mappedBase, address + 2u,
                           resourcePairs[index][1])) {
            return false;
        }
    }

    std::array<TitleHudTimelineDescriptor, kHudTimelineDescriptorCount>
        timelineDescriptors{};
    std::array<std::array<TitleHudTimelineRecord801C5094,
                           kComod0TimelineRecordCapacity801C6D4C>,
               kHudTimelineDescriptorCount>
        timelineRecords{};
    std::array<uint32_t, kHudTimelineDescriptorCount> timelineCounts{};
    for (std::size_t slot = 0u; slot < timelineDescriptors.size(); ++slot) {
        const uint32_t address = kHudTimelineDescriptor801C6D4C +
                                 static_cast<uint32_t>(slot *
                                                       kHudTimelineDescriptorBytes);
        auto& descriptor = timelineDescriptors[slot];
        if (!ReadComod0U32(data, size, mappedBase, address,
                           descriptor.eventsPtr) ||
            !ReadComod0U32(data, size, mappedBase, address + 4u,
                           descriptor.count) ||
            !ReadComod0U32(data, size, mappedBase, address + 8u,
                           descriptor.cursorInitial) ||
            descriptor.cursorInitial != 0u ||
            descriptor.count > kComod0TimelineRecordCapacity801C6D4C) {
            return false;
        }
        timelineCounts[slot] = descriptor.count;
        if (descriptor.count == 0u) {
            if (descriptor.eventsPtr != 0u) {
                return false;
            }
            continue;
        }
        if (descriptor.eventsPtr < mappedBase) {
            return false;
        }
        for (uint32_t index = 0u; index < descriptor.count; ++index) {
            if (!ReadComod0TimelineRecord(
                    data,
                    size,
                    mappedBase,
                    descriptor.eventsPtr +
                        index * kHudTimelineRecordBytes,
                    timelineRecords[slot][index])) {
                return false;
            }
        }
    }

    std::array<TitleHudSelectorRecord, kSelectorRecordCount> selectorRecords{};
    for (std::size_t index = 0u; index < selectorRecords.size(); ++index) {
        if (!ReadComod0SelectorRecord(
                data,
                size,
                mappedBase,
                kSelectorTable801C6F94 +
                    static_cast<uint32_t>(index * kSelectorRecordBytes),
                selectorRecords[index])) {
            return false;
        }
    }

    std::array<uint8_t, kSelectorRecordCount> selectorResourcePairIndices{};
    for (std::size_t selectorIndex = 0u;
         selectorIndex < selectorRecords.size(); ++selectorIndex) {
        uint8_t matchedPair = 0u;
        for (std::size_t pairIndex = 1u;
             pairIndex < resourcePairs.size(); ++pairIndex) {
            if (resourcePairs[pairIndex][0] != selectorRecords[selectorIndex].idA ||
                resourcePairs[pairIndex][1] != selectorRecords[selectorIndex].idB) {
                continue;
            }
            if (matchedPair != 0u) {
                return false;
            }
            matchedPair = static_cast<uint8_t>(pairIndex);
        }
        if (matchedPair == 0u) {
            return false;
        }
        selectorResourcePairIndices[selectorIndex] = matchedPair;
    }

    s_comod0EventStreamRecords = eventRecords;
    s_comod0TimelineDescriptors = timelineDescriptors;
    s_comod0TimelineRecords = timelineRecords;
    s_comod0TimelineRecordCounts = timelineCounts;
    s_comod0SelectorRecords = selectorRecords;
    s_comod0SelectorResourcePairIndices = selectorResourcePairIndices;
    s_comod0ResourcePairs = resourcePairs;
    s_comod0SourceState.loaded = true;
    s_comod0SourceState.descriptorKnown = true;
    s_comod0SourceState.eventStreamKnown = true;
    s_comod0SourceState.resourcePairsKnown = true;
    s_comod0SourceState.timelineKnown = true;
    s_comod0SourceState.selectorKnown = true;
    s_comod0SourceState.mappedBase = mappedBase;
    s_comod0SourceState.eventStreamAddress = eventStreamAddress;
    s_comod0SourceState.eventStreamCount = eventStreamCount;
    s_comod0SourceState.resourcePairCount =
        static_cast<uint32_t>(resourcePairs.size());
    s_comod0SourceState.timelineDescriptorCount =
        static_cast<uint32_t>(timelineDescriptors.size());
    s_comod0SourceState.selectorCount =
        static_cast<uint32_t>(selectorRecords.size());
    return true;
}

const TitleHudComodSourceState801C6Dxx&
GetTitleHudComodSourceState801C6Dxx()
{
    return s_comod0SourceState;
}

uint32_t KnownHudTimelineDescriptorCount()
{
    return s_comod0SourceState.timelineKnown
               ? s_comod0SourceState.timelineDescriptorCount
               : static_cast<uint32_t>(sizeof(kTimelineDescriptors) /
                                       sizeof(kTimelineDescriptors[0]));
}

TitleHudTimelineDescriptor KnownHudTimelineDescriptorAt(uint32_t index)
{
    if (index >= KnownHudTimelineDescriptorCount()) {
        return TitleHudTimelineDescriptor{};
    }
    return s_comod0SourceState.timelineKnown
               ? s_comod0TimelineDescriptors[index]
               : kTimelineDescriptors[index];
}

uint32_t KnownHudTimelineRecordCount(uint32_t slotId)
{
    if (slotId >= KnownHudTimelineDescriptorCount()) {
        return 0u;
    }
    return s_comod0SourceState.timelineKnown
               ? s_comod0TimelineRecordCounts[slotId]
               : kTimelineSlotRecords[slotId].count;
}

TitleHudTimelineRecord801C5094 KnownHudTimelineRecordAt(
    uint32_t slotId,
    uint32_t recordIndex)
{
    if (slotId >= KnownHudTimelineDescriptorCount() ||
        recordIndex >= KnownHudTimelineRecordCount(slotId)) {
        return TitleHudTimelineRecord801C5094{};
    }
    if (s_comod0SourceState.timelineKnown) {
        return s_comod0TimelineRecords[slotId][recordIndex];
    }
    if (kTimelineSlotRecords[slotId].records == nullptr) {
        return TitleHudTimelineRecord801C5094{};
    }
    return kTimelineSlotRecords[slotId].records[recordIndex];
}

uint32_t KnownSelectorRecordCount()
{
    return s_comod0SourceState.selectorKnown
               ? s_comod0SourceState.selectorCount
               : static_cast<uint32_t>(sizeof(kSelectorRecords) /
                                       sizeof(kSelectorRecords[0]));
}

TitleHudSelectorRecord KnownSelectorRecordAt(uint32_t index)
{
    if (index >= KnownSelectorRecordCount()) {
        return TitleHudSelectorRecord{};
    }
    return s_comod0SourceState.selectorKnown
               ? s_comod0SelectorRecords[index]
               : kSelectorRecords[index];
}

uint8_t KnownSelectorResourcePairIndexAt(uint32_t index)
{
    if (index >= KnownSelectorRecordCount()) {
        return 0u;
    }
    return s_comod0SourceState.selectorKnown
               ? s_comod0SelectorResourcePairIndices[index]
               : kSelectorResourcePairIndices[index];
}

TitleHudPlan BuildEventStreamClear801C4FA0Plan()
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;
    AppendAction(plan,
                 TitleHudActionKind::ClearEventStreamCursor801C4FA0,
                 kFn801C4FA0,
                 TitleHudTableKind::EventStream,
                 0,
                 kEventStreamDescriptor801C6E50);
    return plan;
}

TitleHudPlan BuildRelativeEventReset801C4F68Plan(uint32_t ctxFrame,
                                                 uint32_t id)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;
    if (id == 1u) {
        AppendAction(plan,
                     TitleHudActionKind::ResetRelativeEventCursor801C4F68,
                     kFn801C4F68,
                     TitleHudTableKind::EventStream,
                     0,
                     kEventStreamDescriptor801C6E50);
    }
    AppendAction(plan,
                 TitleHudActionKind::StoreEventBaseFrame801C9558,
                 kFn801C4F68,
                 TitleHudTableKind::EventStream,
                 0,
                 kEventStreamBaseFrame801C9558,
                 ctxFrame,
                 id);
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C4F68,
                 TitleHudTableKind::EventStream, 0, id);
    return plan;
}

TitleHudPlan BuildApplyEventRecord801C5190Plan(uint32_t eventIndex)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    if (eventIndex >= KnownEventStreamRecordCount()) {
        AppendAction(plan, TitleHudActionKind::Gap, kFn801C5190,
                     TitleHudTableKind::EventStream, eventIndex);
        return plan;
    }

    const TitleHudEventRecord record =
        KnownEventStreamRecordAt(eventIndex);
    AppendAction(plan,
                 TitleHudActionKind::ApplyEventRecord801C5190,
                 kFn801C5190,
                 TitleHudTableKind::EventStream,
                 eventIndex,
                 record.frame,
                 record.flags);
    AppendAction(plan,
                 TitleHudActionKind::SetCtxFlagsFromEvent,
                 kFn801C5190,
                 TitleHudTableKind::EventStream,
                 eventIndex,
                 record.flags);

    const uint32_t primaryPairIndex = record.bytes[2];
    const uint32_t hudSlotId = record.bytes[3];
    const uint32_t secondaryPairIndex = record.bytes[5];
    const uint32_t tertiaryPairIndex = record.bytes[6];
    if ((record.flags & 0x00010000u) != 0u && primaryPairIndex != 0) {
        AppendAction(plan,
                     TitleHudActionKind::BindResourcePairIndex,
                     kFn801C5190,
                     TitleHudTableKind::ResourcePairs,
                     primaryPairIndex,
                     primaryPairIndex);
    }
    if ((record.flags & 0x00400000u) != 0u && secondaryPairIndex != 0) {
        AppendAction(plan,
                     TitleHudActionKind::BindResourcePairIndex,
                     kFn801C5190,
                     TitleHudTableKind::ResourcePairs,
                     secondaryPairIndex,
                     secondaryPairIndex);
    }
    if ((record.flags & 0x01000000u) != 0u && tertiaryPairIndex != 0) {
        AppendAction(plan,
                     TitleHudActionKind::BindResourcePairIndex,
                     kFn801C5190,
                     TitleHudTableKind::ResourcePairs,
                     tertiaryPairIndex,
                     tertiaryPairIndex);
    }
    if (hudSlotId != 0) {
        AppendHudSlotStart(plan, kFn801C5190, hudSlotId, record.frame);
    }
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C5190,
                 TitleHudTableKind::EventStream, eventIndex,
                 record.bytes[0],
                 record.bytes[1],
                 record.bytes[3]);
    return plan;
}

TitleHudPlan BuildEventStreamTick801C5538Plan(uint32_t ctxFrame,
                                              uint32_t cursor,
                                              bool relativeMode,
                                              uint32_t baseFrame)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan,
                 TitleHudActionKind::GateEventStreamTimeSource801C5538,
                 kFn801C5538,
                 TitleHudTableKind::EventStream,
                 cursor,
                 kCtxFrameOffset0C,
                 relativeMode ? kEventStreamBaseFrame801C9558 : 0u,
                 ctxFrame,
                 baseFrame,
                 true);
    AppendAction(plan,
                 TitleHudActionKind::Call801C5538Tick,
                 kFn801C5538,
                 TitleHudTableKind::EventStream,
                 cursor,
                 ctxFrame,
                 relativeMode ? 1u : 0u,
                 baseFrame);

    if (cursor >= KnownEventStreamRecordCount()) {
        if (relativeMode) {
            AppendAction(plan, TitleHudActionKind::SetCtxFlagsFromEvent,
                         kFn801C5538, TitleHudTableKind::EventStream,
                         cursor, kCtxFlagEventStreamDone);
            AppendAction(plan,
                         TitleHudActionKind::ClearEventStreamCursor801C4FA0,
                         kFn801C5538,
                         TitleHudTableKind::EventStream,
                         cursor,
                         kEventStreamDescriptor801C6E50);
        }
        AppendAction(plan, TitleHudActionKind::Gap, kFn801C5538,
                     TitleHudTableKind::EventStream, cursor);
        return plan;
    }

    const TitleHudEventRecord record = KnownEventStreamRecordAt(cursor);
    const uint32_t threshold =
        relativeMode ? baseFrame + record.frame : record.frame;
    if (ctxFrame >= threshold) {
        AppendAction(plan,
                     TitleHudActionKind::MatchEventRecord,
                     kFn801C5538,
                     TitleHudTableKind::EventStream,
                     cursor,
                     ctxFrame,
                     threshold,
                     record.flags);
        AppendPlan(plan, BuildApplyEventRecord801C5190Plan(cursor));
    }
    AppendPlan(plan, BuildHudTimelinePoll801C5094Plan(
        0, 0, 0, ctxFrame, baseFrame));
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C5538,
                 TitleHudTableKind::EventStream, cursor,
                 ctxFrame, threshold);
    return plan;
}

TitleHudPlan BuildHudTimelinePoll801C5094Plan(uint32_t channel,
                                              uint32_t slotId,
                                              uint32_t cursor,
                                              uint32_t ctxFrame,
                                              uint32_t baseFrame)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;
    const uint32_t runtimeTimListOffset =
        kCtxRuntimeTimListOffsetAC + channel * kCtxRuntimeTimListStride;

    AppendAction(plan,
                 TitleHudActionKind::GateHudTimelineChannelSource801C5094,
                 kFn801C5094,
                 TitleHudTableKind::HudTimelines,
                 slotId,
                 channel,
                 kCtxRuntimeTimListOffsetAC,
                 kCtxRuntimeTimListStride,
                 kCtxRuntimeTimListObservedChannel,
                 true);

    AppendAction(plan,
                 TitleHudActionKind::PollHudTimeline801C5094,
                 kFn801C5094,
                 TitleHudTableKind::HudTimelines,
                 slotId,
                 channel,
                 cursor,
                 ctxFrame,
                 baseFrame);

    const TitleHudTimelineDescriptor desc =
        KnownHudTimelineDescriptorAt(slotId);
    if (slotId >= KnownHudTimelineDescriptorCount() ||
        desc.count == 0 ||
        cursor >= desc.count) {
        AppendAction(plan,
                     TitleHudActionKind::ClearHudTimelineSlot,
                     kFn801C5094,
                     TitleHudTableKind::HudTimelines,
                     slotId,
                     channel);
        AppendAction(plan, TitleHudActionKind::Gap, kFn801C5094,
                     TitleHudTableKind::HudTimelines, slotId);
        return plan;
    }

    AppendAction(plan,
                 TitleHudActionKind::WriteRuntimeTimList,
                 kFn801C5094,
                 TitleHudTableKind::HudTimelines,
                 slotId,
                 runtimeTimListOffset,
                 channel,
                 desc.eventsPtr,
                 cursor);
    AppendAction(plan,
                 TitleHudActionKind::SetCtxFlagsFromEvent,
                 kFn801C5094,
                 TitleHudTableKind::HudTimelines,
                 slotId,
                 kCtxFlagRuntimeTimList);
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C5094,
                 TitleHudTableKind::HudTimelines, slotId,
                 desc.eventsPtr, desc.count, cursor);
    return plan;
}

TitleHudPlan BuildSelectorHud801C5854Plan(uint32_t cursor,
                                          uint32_t bank,
                                          uint32_t cooldown)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;
    const uint32_t selectorIndex = SelectorIndex(cursor, bank);

    AppendAction(plan,
                 TitleHudActionKind::GateSelectorStateSource801C5854,
                 kFn801C5854,
                 TitleHudTableKind::Selector,
                 selectorIndex,
                 cursor,
                 bank,
                 cooldown,
                 kHudCooldownWord8008ECFC,
                 true);

    AppendAction(plan,
                 TitleHudActionKind::SetSelectorBank,
                 kFn801C5854,
                 TitleHudTableKind::Selector,
                 0,
                 kSelectorBank801C955C,
                 bank);

    if (cooldown != 0) {
        AppendAction(plan,
                     TitleHudActionKind::SetSelectorCooldown,
                     kFn801C5854,
                     TitleHudTableKind::Selector,
                     0,
                     kHudCooldownWord8008ECFC,
                     cooldown - 1u,
                     cooldown);
        AppendAction(plan, TitleHudActionKind::Gap, kFn801C5854,
                     TitleHudTableKind::Selector, cursor, bank,
                     cooldown);
        return plan;
    }

    const TitleHudSelectorRecord record =
        KnownSelectorRecordAt(selectorIndex);
    AppendAction(plan,
                 TitleHudActionKind::UpdateSelectorResources,
                 kFn801C5854,
                 TitleHudTableKind::Selector,
                 selectorIndex,
                 static_cast<uint32_t>(record.idA),
                 static_cast<uint32_t>(record.idB),
                 kCtxSelectorResourceOffsetDC,
                 kCtxSelectorResourceOffsetE8);
    AppendAction(plan,
                 TitleHudActionKind::SetCtxFlagsFromEvent,
                 kFn801C5854,
                 TitleHudTableKind::Selector,
                 selectorIndex,
                 kCtxFlagSelectorMime);
    AppendAction(plan,
                 TitleHudActionKind::SetSelectorCooldown,
                 kFn801C5854,
                 TitleHudTableKind::Selector,
                 selectorIndex,
                 kHudCooldownWord8008ECFC,
                 record.ticks);
    AppendAction(plan,
                 TitleHudActionKind::SetSelectorLastCursor,
                 kFn801C5854,
                 TitleHudTableKind::Selector,
                 selectorIndex,
                 kSelectorLastCursor801C9560,
                 cursor);
    if (record.hudSlotId != 0) {
        AppendHudSlotStart(plan, kFn801C5854,
                           static_cast<uint32_t>(record.hudSlotId), 0);
        AppendPlan(plan, BuildHudTimelinePoll801C5094Plan(
            0, static_cast<uint32_t>(record.hudSlotId), 0, 0, 0));
    }
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C5854,
                 TitleHudTableKind::Selector, selectorIndex,
                 bank, cursor);
    return plan;
}

TitleHudPlan BuildTitleSelectorStart801C57E0Plan(uint32_t ctxFrame,
                                                 uint32_t initialHudSlot)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan,
                 TitleHudActionKind::StartHudOverlay801C57E0,
                 kFn801C57E0,
                 TitleHudTableKind::HudTimelines,
                 initialHudSlot,
                 kHudTimelineEnable801C9564,
                 1);
    if (initialHudSlot != 0) {
        AppendHudSlotStart(plan, kFn801C57E0, initialHudSlot, ctxFrame);
    }
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C57E0,
                 TitleHudTableKind::HudTimelines, initialHudSlot,
                 ctxFrame);
    return plan;
}

TitleHudPlan BuildTitleNormalSelectorEntry801C4894Plan(
    uint32_t ctxFrame,
    uint32_t selectorTimeoutFrames,
    uint32_t initialHudSlot)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan,
                 TitleHudActionKind::GateTitleNormalSelectorEntrySource801C4894,
                 kFn801C4894,
                 TitleHudTableKind::Selector,
                 0,
                 ctxFrame,
                 kTitleNormalSelectorEntryFrame801C4894,
                 selectorTimeoutFrames,
                 kCueDword80094410,
                 true);
    AppendAction(plan,
                 TitleHudActionKind::Call8001A694SelectorEntryWaitCleanup,
                 kFn8001A694,
                 TitleHudTableKind::Unknown,
                 0,
                 0x801C4A60u);
    AppendAction(plan,
                 TitleHudActionKind::LatchTitleSelectorStateV8,
                 kFn801C4894,
                 TitleHudTableKind::Selector,
                 0,
                 2,
                 0x801C4A64u);
    AppendAction(plan,
                 TitleHudActionKind::Call80026EF8SelectorEntryCue,
                 kFn80026EF8,
                 TitleHudTableKind::Unknown,
                 0,
                 kCueDword80094410,
                 0x801C4A70u);
    AppendAction(plan,
                 TitleHudActionKind::Nested80034240SelectorEntryVoice,
                 kFn80034240,
                 TitleHudTableKind::Unknown,
                 0,
                 0x80026F34u);
    AppendAction(plan,
                 TitleHudActionKind::Call80026ECCSelectorEntryFlush,
                 kFn80026ECC,
                 TitleHudTableKind::Unknown,
                 0,
                 0x801C4A78u);
    AppendPlan(plan, BuildTitleSelectorStart801C57E0Plan(ctxFrame,
                                                        initialHudSlot));
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C4894,
                 TitleHudTableKind::Selector, 0,
                 ctxFrame, selectorTimeoutFrames);
    return plan;
}

TitleHudPlan BuildTitleSelectorInputHelper801C47ECPlan(uint32_t inputMask,
                                                       uint32_t cursorBefore)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    const uint32_t cursor = cursorBefore;
    const bool playsConfirmCue =
        (inputMask & kTitleSelectorInputConfirmMask801C47EC) != 0u;
    const bool playsMoveCue =
        !playsConfirmCue &&
        ((inputMask & kTitleSelectorInputMoveMask801C47EC) != 0u);
    const bool playsInputSfx = playsConfirmCue || playsMoveCue;
    const uint32_t cueMask = playsConfirmCue
        ? kTitleSelectorInputConfirmCue801C47EC
        : kTitleSelectorInputMoveCue801C47EC;

    AppendAction(plan,
                 TitleHudActionKind::GateTitleSelectorInputHelperSource801C47EC,
                 kFn801C47EC,
                 TitleHudTableKind::Selector,
                 cursor,
                 inputMask,
                 0x801C4CF8u,
                 0x801C4824u,
                 kTitleSelectorResultNone801C47EC,
                 true);

    if (playsInputSfx) {
        AppendAction(plan,
                     TitleHudActionKind::Call80025C8CSelectorInputCue,
                     kFn80025C8C,
                     TitleHudTableKind::Unknown,
                     0,
                     cueMask,
                     0x801C4824u);
        AppendAction(plan,
                     TitleHudActionKind::Call80026EF8SelectorInputCue,
                     kFn80026EF8,
                     TitleHudTableKind::Unknown,
                     0,
                     cueMask,
                     0x80025D50u);
        AppendAction(plan,
                     TitleHudActionKind::Nested80034240SelectorInputVoice,
                     kFn80034240,
                     TitleHudTableKind::Unknown,
                     0,
                     0x80026F34u);
        AppendAction(plan,
                     TitleHudActionKind::Call80026ECCSelectorInputFlush,
                     kFn80026ECC,
                     TitleHudTableKind::Unknown,
                     0,
                     0x80025D58u);
    }

    if (inputMask == kTitleSelectorInputConfirmMask801C47EC) {
        const uint32_t selectorResult = cursor == 0u
            ? kTitleSelectorResultStart801C47EC
            : (cursor == 1u ? kTitleSelectorResultMenu801C47EC
                             : kTitleSelectorResultNone801C47EC);
        AppendAction(plan,
                     TitleHudActionKind::ReturnTitleSelectorConfirm801C47EC,
                     kFn801C47EC,
                     TitleHudTableKind::Selector,
                     cursor,
                     selectorResult,
                     inputMask,
                     0x801C4830u,
                     0x801C4850u);
    } else if (inputMask == kTitleSelectorInputLeftMask801C47EC ||
               inputMask == kTitleSelectorInputRightMask801C47EC) {
        AppendAction(plan,
                     TitleHudActionKind::ToggleTitleSelectorCursor801C47EC,
                     kFn801C47EC,
                     TitleHudTableKind::Selector,
                     cursor,
                     cursor == 0u ? 1u : 0u,
                     inputMask,
                     0x801C486Cu,
                     0x801C4870u);
    }

    AppendAction(plan, TitleHudActionKind::Gap, kFn801C47EC,
                 TitleHudTableKind::Selector, cursor,
                 inputMask, 0x801C4CF8u);
    return plan;
}

TitleHudPlan BuildTitleSelectorExitWait801C4894Plan(uint32_t selectorResult,
                                                    uint32_t waitCounter,
                                                    uint32_t ctxFrame)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    const uint32_t nextWaitCounter =
        waitCounter > 0u ? waitCounter - 1u : 0u;

    AppendAction(plan,
                 TitleHudActionKind::GateTitleSelectorExitWaitSource801C4894,
                 kFn801C4894,
                 TitleHudTableKind::Selector,
                 selectorResult,
                 waitCounter,
                 ctxFrame,
                 0x801C4D58u,
                 kTitleSelectorExitWaitInitialCounter801C4894,
                 true);
    AppendAction(plan,
                 TitleHudActionKind::DecrementTitleSelectorExitWaitCounter,
                 kFn801C4894,
                 TitleHudTableKind::Selector,
                 selectorResult,
                 waitCounter,
                 nextWaitCounter,
                 0x801C4D54u);
    AppendAction(plan,
                 TitleHudActionKind::Call801C6410SelectorExitWaitRender,
                 kFn801C6410,
                 TitleHudTableKind::Selector,
                 selectorResult,
                 0,
                 0x801C4D58u);
    AppendAction(plan,
                 TitleHudActionKind::ClearTitleCtxFlagsForExitWait,
                 kFn801C4894,
                 TitleHudTableKind::Selector,
                 selectorResult,
                 0,
                 0x801C4D60u);
    AppendAction(plan,
                 TitleHudActionKind::Call8001E3E4SelectorExitWaitTail,
                 kFn8001E3E4,
                 TitleHudTableKind::Selector,
                 selectorResult,
                 0x801C4D64u);
    AppendAction(plan,
                 TitleHudActionKind::Call80035560SelectorExitWaitWait,
                 kFn80035560,
                 TitleHudTableKind::Unknown,
                 0,
                 kTitleSelectorExitWaitFrameWait801C4894,
                 0x801C4D6Cu);
    AppendAction(plan,
                 TitleHudActionKind::Call801C689CSelectorExitWaitPresent,
                 kFn801C689C,
                 TitleHudTableKind::Selector,
                 selectorResult,
                 0x801C4D74u);
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C4894,
                 TitleHudTableKind::Selector, selectorResult,
                 waitCounter, ctxFrame);
    return plan;
}

TitleHudPlan BuildShortcutStartAndApply801C5AB4Plan(uint32_t ctxFrame,
                                                    uint32_t cursor)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan,
                 TitleHudActionKind::StartAndApply801C5AB4,
                 kFn801C5AB4,
                 TitleHudTableKind::HudTimelines,
                 cursor,
                 kCtxFlagHudStart);
    AppendPlan(plan, BuildTitleSelectorStart801C57E0Plan(
                         ctxFrame,
                         kTitleSelectorInitialHudSlot801C57E0));
    AppendPlan(plan, BuildSelectorHud801C5854Plan(
                         cursor,
                         kTitleSelectorInitialBank801C57E0,
                         0));
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C5AB4,
                 TitleHudTableKind::Selector, cursor, ctxFrame);
    return plan;
}

TitleHudPlan BuildTitleEarlyInputShortcut801C4894Plan(uint32_t ctxFrame,
                                                      uint32_t cursor,
                                                      uint32_t stateV8,
                                                      uint32_t inputMask,
                                                      bool inputChanged)
{
    TitleHudPlan plan = MakePlan();
    plan.hasOpenP0Gap = true;

    AppendAction(plan,
                 TitleHudActionKind::GateTitleEarlyInputSource801C4894,
                 kFn801C4894,
                 TitleHudTableKind::Selector,
                 cursor,
                 stateV8,
                 inputMask,
                 inputChanged ? 1u : 0u,
                 ctxFrame,
                 true);

    if (inputMask == 0 || !inputChanged || stateV8 > 1u) {
        AppendAction(plan, TitleHudActionKind::Gap, kFn801C4894,
                     TitleHudTableKind::Selector, cursor,
                     stateV8, inputMask);
        return plan;
    }

    if (stateV8 == 0u) {
        AppendAction(plan,
                     TitleHudActionKind::Call80027664EarlyInputStopReset,
                     kFn80027664,
                     TitleHudTableKind::Unknown,
                     0,
                     0x801C4B6Cu);
        AppendAction(plan,
                     TitleHudActionKind::Call8001A694EarlyInputWaitCleanup,
                     kFn8001A694,
                     TitleHudTableKind::Unknown,
                     0,
                     0x801C4B74u);
        AppendAction(plan,
                     TitleHudActionKind::Call80040370EarlyInputFlip,
                     kFn80040370,
                     TitleHudTableKind::Unknown,
                     0,
                     0x801C4B7Cu);
    } else {
        AppendAction(plan,
                     TitleHudActionKind::Call8001A694EarlyInputWaitCleanup,
                     kFn8001A694,
                     TitleHudTableKind::Unknown,
                     0,
                     0x801C4BDCu);
    }

    AppendAction(plan,
                 TitleHudActionKind::SetTitleShortcutStartFlag,
                 kFn801C4894,
                 TitleHudTableKind::Selector,
                 cursor,
                 kCtxFlagShortcutStart);
    AppendAction(plan,
                 TitleHudActionKind::Call801C6410ShortcutRender,
                 kFn801C6410,
                 TitleHudTableKind::Selector,
                 cursor,
                 0);
    AppendAction(plan,
                 TitleHudActionKind::Call80035560ShortcutWait,
                 kFn80035560,
                 TitleHudTableKind::Unknown,
                 0,
                 kShortcutFrameWait);
    AppendAction(plan,
                 TitleHudActionKind::Call801C689CShortcutPresent,
                 kFn801C689C,
                 TitleHudTableKind::Selector,
                 cursor);
    AppendAction(plan,
                 TitleHudActionKind::Call80035560ShortcutHoldLoop,
                 kFn80035560,
                 TitleHudTableKind::Unknown,
                 0,
                 kShortcutFrameWait,
                 kShortcutHoldWaitFrames);
    AppendPlan(plan, BuildShortcutStartAndApply801C5AB4Plan(ctxFrame,
                                                            cursor));
    AppendAction(plan,
                 TitleHudActionKind::SetTitleShortcutReadyFlag,
                 kFn801C4894,
                 TitleHudTableKind::Selector,
                 cursor,
                 kCtxFlagShortcutReady);
    AppendAction(plan,
                 TitleHudActionKind::Call801C6410ShortcutRender,
                 kFn801C6410,
                 TitleHudTableKind::Selector,
                 cursor,
                 0);
    AppendAction(plan,
                 TitleHudActionKind::Call80035560ShortcutWait,
                 kFn80035560,
                 TitleHudTableKind::Unknown,
                 0,
                 kShortcutFrameWait);
    AppendAction(plan,
                 TitleHudActionKind::Call801C689CShortcutPresent,
                 kFn801C689C,
                 TitleHudTableKind::Selector,
                 cursor);
    AppendAction(plan,
                 TitleHudActionKind::WaitTitleShortcutInputRelease,
                 kFn80035510,
                 TitleHudTableKind::Selector,
                 cursor,
                 inputMask);
    AppendAction(plan,
                 TitleHudActionKind::Call80026EF8ShortcutCue,
                 kFn80026EF8,
                 TitleHudTableKind::Unknown,
                 0,
                 kCueDword80094410);
    AppendAction(plan,
                 TitleHudActionKind::Call80026ECCShortcutFlush,
                 kFn80026ECC);
    AppendAction(plan, TitleHudActionKind::Gap, kFn801C4894,
                 TitleHudTableKind::Selector, cursor,
                 stateV8, inputMask, ctxFrame);
    return plan;
}

const char* TitleHudTableKindName(TitleHudTableKind kind)
{
    switch (kind) {
    case TitleHudTableKind::Unknown:
        return "Unknown";
    case TitleHudTableKind::EventStream:
        return "EventStream";
    case TitleHudTableKind::ResourcePairs:
        return "ResourcePairs";
    case TitleHudTableKind::HudTimelines:
        return "HudTimelines";
    case TitleHudTableKind::Selector:
        return "Selector";
    case TitleHudTableKind::EventBuilderCount:
        return "EventBuilderCount";
    }
    return "Unknown";
}

const char* TitleHudActionKindName(TitleHudActionKind kind)
{
    switch (kind) {
    case TitleHudActionKind::None:
        return "None";
    case TitleHudActionKind::ClearEventStreamCursor801C4FA0:
        return "ClearEventStreamCursor801C4FA0";
    case TitleHudActionKind::ResetRelativeEventCursor801C4F68:
        return "ResetRelativeEventCursor801C4F68";
    case TitleHudActionKind::StoreEventBaseFrame801C9558:
        return "StoreEventBaseFrame801C9558";
    case TitleHudActionKind::GateEventStreamTimeSource801C5538:
        return "GateEventStreamTimeSource801C5538";
    case TitleHudActionKind::Call801C5538Tick:
        return "Call801C5538Tick";
    case TitleHudActionKind::MatchEventRecord:
        return "MatchEventRecord";
    case TitleHudActionKind::ApplyEventRecord801C5190:
        return "ApplyEventRecord801C5190";
    case TitleHudActionKind::SetCtxFlagsFromEvent:
        return "SetCtxFlagsFromEvent";
    case TitleHudActionKind::BindResourcePairIndex:
        return "BindResourcePairIndex";
    case TitleHudActionKind::SetHudSlot:
        return "SetHudSlot";
    case TitleHudActionKind::SetHudSlotBaseFrame:
        return "SetHudSlotBaseFrame";
    case TitleHudActionKind::ResetHudTimelineCursor:
        return "ResetHudTimelineCursor";
    case TitleHudActionKind::GateHudTimelineChannelSource801C5094:
        return "GateHudTimelineChannelSource801C5094";
    case TitleHudActionKind::PollHudTimeline801C5094:
        return "PollHudTimeline801C5094";
    case TitleHudActionKind::ClearHudTimelineSlot:
        return "ClearHudTimelineSlot";
    case TitleHudActionKind::WriteRuntimeTimList:
        return "WriteRuntimeTimList";
    case TitleHudActionKind::StartHudOverlay801C57E0:
        return "StartHudOverlay801C57E0";
    case TitleHudActionKind::GateSelectorStateSource801C5854:
        return "GateSelectorStateSource801C5854";
    case TitleHudActionKind::SetSelectorBank:
        return "SetSelectorBank";
    case TitleHudActionKind::SetSelectorLastCursor:
        return "SetSelectorLastCursor";
    case TitleHudActionKind::SetSelectorCooldown:
        return "SetSelectorCooldown";
    case TitleHudActionKind::UpdateSelectorResources:
        return "UpdateSelectorResources";
    case TitleHudActionKind::StartAndApply801C5AB4:
        return "StartAndApply801C5AB4";
    case TitleHudActionKind::GateTitleNormalSelectorEntrySource801C4894:
        return "GateTitleNormalSelectorEntrySource801C4894";
    case TitleHudActionKind::Call8001A694SelectorEntryWaitCleanup:
        return "Call8001A694SelectorEntryWaitCleanup";
    case TitleHudActionKind::LatchTitleSelectorStateV8:
        return "LatchTitleSelectorStateV8";
    case TitleHudActionKind::Call80026EF8SelectorEntryCue:
        return "Call80026EF8SelectorEntryCue";
    case TitleHudActionKind::Nested80034240SelectorEntryVoice:
        return "Nested80034240SelectorEntryVoice";
    case TitleHudActionKind::Call80026ECCSelectorEntryFlush:
        return "Call80026ECCSelectorEntryFlush";
    case TitleHudActionKind::GateTitleSelectorInputHelperSource801C47EC:
        return "GateTitleSelectorInputHelperSource801C47EC";
    case TitleHudActionKind::Call80025C8CSelectorInputCue:
        return "Call80025C8CSelectorInputCue";
    case TitleHudActionKind::Call80026EF8SelectorInputCue:
        return "Call80026EF8SelectorInputCue";
    case TitleHudActionKind::Nested80034240SelectorInputVoice:
        return "Nested80034240SelectorInputVoice";
    case TitleHudActionKind::Call80026ECCSelectorInputFlush:
        return "Call80026ECCSelectorInputFlush";
    case TitleHudActionKind::ToggleTitleSelectorCursor801C47EC:
        return "ToggleTitleSelectorCursor801C47EC";
    case TitleHudActionKind::ReturnTitleSelectorConfirm801C47EC:
        return "ReturnTitleSelectorConfirm801C47EC";
    case TitleHudActionKind::GateTitleSelectorExitWaitSource801C4894:
        return "GateTitleSelectorExitWaitSource801C4894";
    case TitleHudActionKind::DecrementTitleSelectorExitWaitCounter:
        return "DecrementTitleSelectorExitWaitCounter";
    case TitleHudActionKind::Call801C6410SelectorExitWaitRender:
        return "Call801C6410SelectorExitWaitRender";
    case TitleHudActionKind::ClearTitleCtxFlagsForExitWait:
        return "ClearTitleCtxFlagsForExitWait";
    case TitleHudActionKind::Call8001E3E4SelectorExitWaitTail:
        return "Call8001E3E4SelectorExitWaitTail";
    case TitleHudActionKind::Call80035560SelectorExitWaitWait:
        return "Call80035560SelectorExitWaitWait";
    case TitleHudActionKind::Call801C689CSelectorExitWaitPresent:
        return "Call801C689CSelectorExitWaitPresent";
    case TitleHudActionKind::GateTitleEarlyInputSource801C4894:
        return "GateTitleEarlyInputSource801C4894";
    case TitleHudActionKind::Call80027664EarlyInputStopReset:
        return "Call80027664EarlyInputStopReset";
    case TitleHudActionKind::Call8001A694EarlyInputWaitCleanup:
        return "Call8001A694EarlyInputWaitCleanup";
    case TitleHudActionKind::Call80040370EarlyInputFlip:
        return "Call80040370EarlyInputFlip";
    case TitleHudActionKind::SetTitleShortcutStartFlag:
        return "SetTitleShortcutStartFlag";
    case TitleHudActionKind::Call801C6410ShortcutRender:
        return "Call801C6410ShortcutRender";
    case TitleHudActionKind::Call80035560ShortcutWait:
        return "Call80035560ShortcutWait";
    case TitleHudActionKind::Call801C689CShortcutPresent:
        return "Call801C689CShortcutPresent";
    case TitleHudActionKind::Call80035560ShortcutHoldLoop:
        return "Call80035560ShortcutHoldLoop";
    case TitleHudActionKind::SetTitleShortcutReadyFlag:
        return "SetTitleShortcutReadyFlag";
    case TitleHudActionKind::WaitTitleShortcutInputRelease:
        return "WaitTitleShortcutInputRelease";
    case TitleHudActionKind::Call80026EF8ShortcutCue:
        return "Call80026EF8ShortcutCue";
    case TitleHudActionKind::Call80026ECCShortcutFlush:
        return "Call80026ECCShortcutFlush";
    case TitleHudActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0TitleHudEventsDirect
