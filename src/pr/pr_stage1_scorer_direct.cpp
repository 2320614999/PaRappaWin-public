#include "pr_stage1_scorer_direct.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

static int32_t PrStage1ScorerDirectPairBonus14A80(
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectPage& page,
    size_t coverageCount);

static PrStage1ScorerDirectAcceptedReplaySetupResult
PrStage1ScorerDirectRunAcceptedReplaySetupCore24E54_7A60(
    PrStage1ScorerDirectReplayBufferState& replay,
    uint16_t transitionState);

namespace {

struct Stage1EventTableSourceRow801C8660 {
    int16_t timeIndex = 0;
    int16_t kind = 0;
};

constexpr std::array<std::array<uint32_t, 4>, 5> kPrStage1ScorerCoverageTable_80048CA8 = {{
    {{ 15u, 0u, 0u, 0u }},
    {{ 15u, 0u, 0u, 0u }},
    {{ 16u, 15u, 0u, 0u }},
    {{ 16u, 16u, 15u, 0u }},
    {{ 16u, 16u, 16u, 15u }},
}};

constexpr std::array<uint32_t, 9> kPrStage1ScorerMaskTable80054168 = {{
    0x0000u, 0x0010u, 0x0020u, 0x0040u, 0x0080u,
    0x0004u, 0x0004u, 0x0008u, 0x0008u,
}};

constexpr std::array<Stage1EventTableSourceRow801C8660, 53>
    kStage1EventTableSource801D2E2C = {{
        {120, 1}, {136, 2}, {152, 3}, {168, 4},
        {184, 1}, {200, 2}, {216, 3}, {232, 4},
        {280, 5}, {296, 5}, {312, 8}, {328, 8},
        {344, 5}, {360, 8}, {376, 5}, {392, 8},
        {448, 1}, {452, 2}, {480, 3}, {484, 4},
        {512, 3}, {516, 1}, {544, 2}, {548, 4},
        {576, 5}, {580, 5}, {608, 8}, {612, 8},
        {640, 5}, {644, 8}, {672, 5}, {676, 8},
        {784, 1}, {786, 2}, {788, 4}, {816, 3},
        {818, 1}, {820, 4}, {848, 4}, {850, 8},
        {852, 1}, {880, 4}, {882, 5}, {884, 2},
        {912, 5}, {914, 5}, {916, 8}, {944, 5},
        {946, 1}, {948, 3}, {976, 2}, {978, 2},
        {980, 2},
    }};

constexpr int32_t kPrStage1ScorerDirectMaxLookbackPages = 4;

uint32_t PrStage1ScorerDirectMapKind80024BC0(int32_t kind) {
    if (static_cast<uint32_t>(kind - 1) >= 8u) {
        return 0u;
    }
    return kPrStage1ScorerMaskTable80054168[static_cast<size_t>(kind)];
}

size_t ClampCoverageCount(
    const PrStage1ScorerDirectGlobals& globals,
    uint16_t lookbackPageCount,
    size_t pageOffset) {
    if (lookbackPageCount == 0u || pageOffset >= kPrStage1ScorerDirectMaxLookbackPages) {
        return 0u;
    }

    const size_t tableRow =
        static_cast<size_t>((std::min<int32_t>)(lookbackPageCount, kPrStage1ScorerDirectMaxLookbackPages));
    uint32_t coverageCount = kPrStage1ScorerCoverageTable_80048CA8[tableRow][pageOffset];
    if (coverageCount == 15u && globals.word916E2CurrentSceneIndex == 1u) {
        coverageCount = 14u;
    }
    return (std::min<size_t>)(coverageCount, kPrStage1ScorerDirectSlotsPerPage);
}

size_t ResolveRingIndex(int pageOrdinalZeroBased) {
    const int wrapped = ((pageOrdinalZeroBased % 4) + 4) % 4;
    return static_cast<size_t>(wrapped);
}

bool IsSlotOccupied(const PrStage1ScorerDirectPage& page, size_t slotIndex) {
    if (slotIndex >= kPrStage1ScorerDirectSlotsPerPage) {
        return false;
    }
    return page.records[slotIndex].word06Occupied != 0u;
}

uint32_t ResolveSlotMask(const PrStage1ScorerDirectPage& page, size_t slotIndex) {
    if (slotIndex >= kPrStage1ScorerDirectSlotsPerPage) {
        return 0u;
    }
    return page.records[slotIndex].dword00AcceptedMask;
}

uint16_t ResolveSlotClassToken(const PrStage1ScorerDirectPage& page, size_t slotIndex) {
    if (slotIndex >= kPrStage1ScorerDirectSlotsPerPage) {
        return 0u;
    }
    return page.records[slotIndex].word04Companion;
}

int ResolveReaderPageOrdinal(const PrStage1ScorerDirectGlobals& globals) {
    if (globals.currentPageOrdinal1Based == 0u) {
        return -1;
    }
    return static_cast<int>(globals.currentPageOrdinal1Based) - 1;
}

int ResolveAcceptedWriterPageOrdinal14614(
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectAcceptedProducerCoreInput& in) {
    // Literal 14BDC seeds the mutable writer-page helper with
    // `unk_80092910 + 384 * (a1 & 3)`, so raw page 0 is a valid known
    // writer page and must not fall back to tick-derived page ownership.
    if (globals.currentWritePageOrdinalKnown) {
        return static_cast<int>(globals.currentWritePageOrdinal1Based);
    }
    return in.writePageOrdinal38;
}

struct CommitWritebackState {
    bool known = false;
    int32_t value = 0;
    bool clampActive = false;
};

CommitWritebackState ResolveCommitWriteback(
    PrStage1ScorerDirectGlobals& globals,
    int32_t commitTermValue) {
    CommitWritebackState out{};
    out.known = true;
    const int16_t nextValue =
        static_cast<int16_t>(static_cast<int32_t>(globals.word91816Accumulator) + commitTermValue);
    globals.word91816Accumulator = nextValue;
    if (globals.word91816Accumulator < 0) {
        globals.word91816Accumulator = 0;
        out.clampActive = true;
    }
    out.value = globals.word91816Accumulator;
    return out;
}

PrStage1ScorerDirectBranchState BuildDefaultBranchState(
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectDescriptorRow& row,
    int32_t acceptedCount,
    int32_t penaltyCount) {
    PrStage1ScorerDirectBranchState out{};
    if (!row.valid) {
        return out;
    }

    const PrStage1ScorerDirectDescriptorBranch& branch = row.defaultBranch;
    out.countKnown = true;
    out.requiredMaskKnown = true;
    out.requiredMaskValue = static_cast<int32_t>(branch.dword08RequiredMask);
    out.anchorSlotIndexKnown = true;
    out.anchorSlotIndexValue = static_cast<int32_t>(branch.byte01AnchorSlotIndex);
    out.requiredClassTokenKnown = true;
    out.requiredClassTokenValue = static_cast<int32_t>(branch.byte02RequiredClassToken);

    if (acceptedCount == 0) {
        out.pairBonusKnown = true;
        out.spillPenaltyKnown = true;
        out.descriptorSubdeltaKnown = true;
        out.descriptorSubdeltaValue = -1;
        return out;
    }

    const int32_t lookbackPageCount = branch.word0ELookbackPageCount;
    if (lookbackPageCount <= 0) {
        out.pairBonusKnown = true;
        out.spillPenaltyKnown = true;
        out.descriptorSubdeltaKnown = true;
        out.descriptorSubdeltaValue = 0;
        return out;
    }

    const bool enoughGate = acceptedCount <= 12 * lookbackPageCount;
    const int readerPageOrdinal = ResolveReaderPageOrdinal(globals);
    out.readerPageOrdinalKnown = true;
    out.readerPageOrdinalValue = readerPageOrdinal;
    if (readerPageOrdinal < 0) {
        return out;
    }

    const size_t pageCount =
        static_cast<size_t>((std::min<int32_t>)(lookbackPageCount, kPrStage1ScorerDirectMaxLookbackPages));
    if (pageCount == 0u) {
        return out;
    }

    uint32_t unionMask = 0u;
    int32_t matchedCount = 0;
    int32_t pairBonusValue = 0;
    const PrStage1ScorerDirectPage* anchorPage = nullptr;
    for (size_t pageIndex = 0u; pageIndex < pageCount; ++pageIndex) {
        const int pageOrdinal =
            readerPageOrdinal - static_cast<int>(pageCount - 1u - pageIndex);
        const PrStage1ScorerDirectPage& page =
            globals.ringPages[ResolveRingIndex(pageOrdinal)];
        if (pageIndex == 0u) {
            anchorPage = &page;
        }

        const size_t coverageCount = ClampCoverageCount(
            globals,
            static_cast<uint16_t>(lookbackPageCount),
            pageIndex);
        if (coverageCount == 0u) {
            continue;
        }

        for (size_t slotIndex = 0u; slotIndex < coverageCount; ++slotIndex) {
            const uint32_t slotMask = ResolveSlotMask(page, slotIndex);
            unionMask |= slotMask;
            if (!IsSlotOccupied(page, slotIndex)) {
                continue;
            }

            if (branch.dword08RequiredMask == 0u ||
                (slotMask & branch.dword08RequiredMask) != 0u) {
                ++matchedCount;
            }
        }

        pairBonusValue += PrStage1ScorerDirectPairBonus14A80(
            globals,
            page,
            coverageCount);
    }

    if (branch.dword08RequiredMask != 0u &&
        (unionMask & branch.dword08RequiredMask) != branch.dword08RequiredMask) {
        matchedCount = 0;
    }
    out.countValue = matchedCount;
    out.unionMaskKnown = true;
    out.unionMaskValue = static_cast<int32_t>(unionMask);

    if (!anchorPage) {
        return out;
    }

    const size_t anchorSlotIndex = static_cast<size_t>(branch.byte01AnchorSlotIndex);
    if (anchorSlotIndex >= kPrStage1ScorerDirectSlotsPerPage) {
        return out;
    }

    out.anchorSlotOccupiedKnown = true;
    out.anchorSlotOccupiedValue = IsSlotOccupied(*anchorPage, anchorSlotIndex) ? 1 : 0;
    out.anchorSlotClassTokenKnown = true;
    out.anchorSlotClassTokenValue =
        static_cast<int32_t>(ResolveSlotClassToken(*anchorPage, anchorSlotIndex));
    const bool anchorClassMatch =
        out.anchorSlotClassTokenValue ==
        static_cast<int32_t>(branch.byte02RequiredClassToken);
    out.anchorClassMatchKnown = true;
    out.anchorClassMatchValue = anchorClassMatch ? 1 : 0;

    const int32_t descriptorWeight = static_cast<int32_t>(branch.byte03PenaltyWeight);
    int32_t branchBase = anchorClassMatch
        ? (3 * matchedCount)
        : (3 * matchedCount - descriptorWeight);
    int32_t pairBonusTerm =
        (anchorClassMatch && enoughGate && matchedCount > 0)
            ? pairBonusValue
            : 0;
    int32_t spillPenaltyTerm = -(descriptorWeight * penaltyCount);
    const int32_t spillFloor = ~(descriptorWeight * 3);
    if (spillPenaltyTerm < spillFloor) {
        spillPenaltyTerm = spillFloor;
    }
    if (!enoughGate) {
        branchBase = 0;
        pairBonusTerm = 0;
        spillPenaltyTerm = 0;
    }

    out.pairBonusKnown = true;
    out.pairBonusValue = pairBonusTerm;
    out.spillPenaltyKnown = true;
    out.spillPenaltyValue = spillPenaltyTerm;
    out.descriptorSubdeltaKnown = true;
    out.descriptorSubdeltaValue = branchBase + pairBonusTerm + spillPenaltyTerm;
    return out;
}

PrStage1ScorerDirectBranchState BuildSpecialBranchState(
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectDescriptorRow& row,
    int32_t acceptedCount,
    int32_t penaltyCount) {
    PrStage1ScorerDirectBranchState out{};
    if (!row.valid) {
        return out;
    }

    if (acceptedCount == 0) {
        out.pairBonusKnown = true;
        out.spillPenaltyKnown = true;
        out.descriptorSubdeltaKnown = true;
        out.descriptorSubdeltaValue = -1;
        return out;
    }

    const PrStage1ScorerDirectDescriptorBranch& branch = row.substate1Branch;
    const int32_t lookbackPageCount = branch.word0ELookbackPageCount;
    const int32_t spillPenalty = -2 * penaltyCount;
    out.spillPenaltyKnown = true;
    out.spillPenaltyValue = spillPenalty;
    out.pairBonusKnown = true;
    if (lookbackPageCount <= 0) {
        out.descriptorSubdeltaKnown = true;
        out.descriptorSubdeltaValue = spillPenalty;
        return out;
    }

    const bool enoughGate = acceptedCount <= 12 * lookbackPageCount;
    const int readerPageOrdinal = ResolveReaderPageOrdinal(globals);
    out.readerPageOrdinalKnown = true;
    out.readerPageOrdinalValue = readerPageOrdinal;
    if (readerPageOrdinal < 0) {
        return out;
    }

    const size_t pageCount =
        static_cast<size_t>((std::min<int32_t>)(lookbackPageCount, kPrStage1ScorerDirectMaxLookbackPages));
    if (pageCount == 0u) {
        return out;
    }

    int32_t filledCount = 0;
    int32_t pairBonusValue = 0;
    for (size_t pageIndex = 0u; pageIndex < pageCount; ++pageIndex) {
        const int pageOrdinal =
            readerPageOrdinal - static_cast<int>(pageCount - 1u - pageIndex);
        const PrStage1ScorerDirectPage& page =
            globals.ringPages[ResolveRingIndex(pageOrdinal)];
        const size_t coverageCount = ClampCoverageCount(
            globals,
            static_cast<uint16_t>(lookbackPageCount),
            pageIndex);
        if (coverageCount == 0u) {
            continue;
        }

        for (size_t slotIndex = 0u; slotIndex < coverageCount; ++slotIndex) {
            if (IsSlotOccupied(page, slotIndex)) {
                ++filledCount;
            }
        }

        pairBonusValue += PrStage1ScorerDirectPairBonus14A80(
            globals,
            page,
            coverageCount);
    }

    out.countKnown = true;
    out.countValue = filledCount;
    out.pairBonusValue = enoughGate ? pairBonusValue : 0;
    out.descriptorSubdeltaKnown = true;
    out.descriptorSubdeltaValue =
        (3 * filledCount) + spillPenalty + out.pairBonusValue;
    return out;
}

} // namespace

static const PrStage1ScorerDirectDescriptorBranch&
PrStage1ScorerDirectSelectBranch(const PrStage1ScorerDirectDescriptorRow& row,
                                 uint16_t descriptorSubstate50) {
    return descriptorSubstate50 != 0u ? row.substate1Branch : row.defaultBranch;
}

size_t PrStage1ScorerDirectResolvePageOrdinal56_24FD0(
    int32_t tick96) {
    if (tick96 < 0) {
        return 0u;
    }
    return static_cast<size_t>((static_cast<uint32_t>(tick96) / 384u) + 1u);
}

size_t PrStage1ScorerDirectResolveLookaheadRowIndex44_24FD0(
    int32_t tick96,
    uint16_t word916D0) {
    if (tick96 < 0) {
        return 0u;
    }

    const uint32_t intra384 = static_cast<uint32_t>(tick96) % 384u;
    const uint32_t threshold =
        (word916D0 == 1u || word916D0 == 2u) ? 372u : 368u;
    return PrStage1ScorerDirectResolvePageOrdinal56_24FD0(tick96) +
           ((intra384 >= threshold) ? 1u : 0u);
}

uint16_t PrStage1ScorerDirectResolveDescriptorFlagWordWithLatch24FD0(
    bool latchedFlagWordKnown,
    uint16_t latchedFlagWord) {
    if (latchedFlagWordKnown) {
        return latchedFlagWord;
    }
    return 0u;
}

static bool PrStage1ScorerDirectBusyGate24BF4(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectBucketContext& ctx) {
    if (globals.dword8ED00FollowUpState == 1) {
        if (globals.dword8ED14FollowUpActive == 0) {
            return false;
        }
        if (globals.dword8ED0CFollowUpDeadlineTick <
            static_cast<int32_t>(ctx.dword0CTick96)) {
            globals.dword8ED14FollowUpActive = 0;
            return false;
        }
        return true;
    }

    if (globals.gPrStageEventStreamId == 0u) {
        return globals.dword8ED14FollowUpActive != 0 &&
               globals.dword8ED00FollowUpState == 5;
    }
    return true;
}

static bool PrStage1ScorerDirectConsumeBucket0ReturnGate144B8(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectBucketContext& ctx) {
    bool result = false;
    if (ctx.word4ERightRankActiveRow == 3u) {
        if (globals.word91820NoInputCounter > 0u &&
            ctx.word6ABucket0ConsumerGate != 0u) {
            result = true;
        } else if (globals.word9181ETwitterCarryLatch == 1u) {
            result = ctx.word6ABucket0ConsumerGate != 0u;
        }
    } else if (ctx.word4ERightRankActiveRow == 0u) {
        if (globals.word9181ETwitterCarryLatch == 1u) {
            result = ctx.word6ABucket0ConsumerGate != 0u;
        }
    }

    globals.word9181ETwitterCarryLatch = 0u;
    globals.word91820NoInputCounter = 0u;
    return result;
}

uint16_t PrStage1ScorerDirectInjectAdditive14C5C(
    PrStage1ScorerDirectGlobals& globals,
    int16_t additiveValue) {
    globals.word91822AdditiveTerm = additiveValue;
    return ++globals.word91810AcceptedCount;
}

static uint32_t PrStage1ScorerDirectAdvanceNoInputCounter14458(
    PrStage1ScorerDirectGlobals& globals) {
    if (globals.word91810AcceptedCount == 0u) {
        const uint16_t previous = globals.word91820NoInputCounter;
        globals.word91820NoInputCounter =
            static_cast<uint16_t>(previous + 1u);
        return static_cast<uint32_t>(previous) + 1u;
    }
    return globals.word91810AcceptedCount;
}

PrStage1ScorerDirectBaselineRefresh1448CResult
PrStage1ScorerDirectRunBaselineRefresh1448C(
    PrStage1ScorerDirectGlobals& globals) {
    globals.word91818SharedBaseline = globals.word91816Accumulator;
    globals.word9181ASnapshot = globals.word91816Accumulator;
    globals.word9181CCompareBaseline = globals.word91816Accumulator;
    PrStage1ScorerDirectBaselineRefresh1448CResult out{};
    out.applied = true;
    out.word80091816Accumulator = globals.word91816Accumulator;
    out.word80091818SharedBaseline = globals.word91818SharedBaseline;
    out.word8009181ASnapshot = globals.word9181ASnapshot;
    out.word8009181CCompareBaseline = globals.word9181CCompareBaseline;
    out.returnValue = static_cast<uint16_t>(globals.word91816Accumulator);
    return out;
}

PrStage1ScorerDirectBaselineRefresh1448CResult
PrStage1ScorerDirectApplyResetAction80024F8C(
    PrStage1ScorerDirectGlobals& globals,
    const PrStageEventDirectResetAction80024F8C& action) {
    if (!action.valid || !action.scorerBaselineRefresh1448CRequested) {
        return PrStage1ScorerDirectBaselineRefresh1448CResult{};
    }

    PrStage1ScorerDirectBaselineRefresh1448CResult out =
        PrStage1ScorerDirectRunBaselineRefresh1448C(globals);
    out.sourceReset80024F8C = true;
    out.sourceResetPsxAddr80024F8C = action.resetPsxAddr80024F8C;
    return out;
}

static uint16_t PrStage1ScorerDirectRefreshBaselines1448C(
    PrStage1ScorerDirectGlobals& globals) {
    return PrStage1ScorerDirectRunBaselineRefresh1448C(globals).returnValue;
}

static int32_t PrStage1ScorerDirectPositiveGrowth14D28(
    PrStage1ScorerDirectGlobals& globals) {
    const int16_t previousValue = globals.word91818SharedBaseline;
    globals.word91818SharedBaseline = globals.word91816Accumulator;
    return globals.word91816Accumulator - previousValue > 0 ? 1 : 0;
}

static int32_t PrStage1ScorerDirectCoolClassifier14C80(
    PrStage1ScorerDirectGlobals& globals) {
    const int16_t previousValue = globals.word91818SharedBaseline;
    globals.word91818SharedBaseline = globals.word91816Accumulator;
    if (globals.word91816Accumulator - previousValue >= 66) {
        if (globals.word91828CoolGainStreak < 2u) {
            globals.word91826CoolGainLatch = 0;
            globals.word91828CoolGainStreak = 0;
            return 0;
        }
    } else {
        ++globals.word91828CoolGainStreak;
        if (globals.word91828CoolGainStreak < 2u) {
            return 1;
        }
    }

    globals.word91826CoolGainLatch = 1;
    return 1;
}

static int32_t PrStage1ScorerDirectTieBreak14548(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectBucketContext& ctx,
    const PrStage1ScorerDirectDescriptorRow& row) {
    int32_t result = 2;
    if (!ctx.descriptorPointerAvailable) {
        return result;
    }

    const int16_t previousValue = globals.word9181CCompareBaseline;
    globals.word9181ASnapshot = globals.word91816Accumulator;
    globals.word9181CCompareBaseline = globals.word91816Accumulator;
    const int32_t delta = globals.word91816Accumulator - previousValue;
    const uint16_t activeRow = ctx.word4ERightRankActiveRow;
    if (activeRow == 1u) {
        if (delta > row.word10FinalResolutionThreshold) {
            return 1;
        }
        if (delta < 0) {
            return 0;
        }
        return globals.word9181ETwitterCarryLatch != 1u ? 2 : 0;
    }

    if (activeRow == 2u) {
        if (delta > 0) {
            return 1;
        }
        if (delta >= 0) {
            return globals.word9181ETwitterCarryLatch != 1u ? 2 : 0;
        }
        return 0;
    }

    result = 1;
    if (delta <= 0) {
        result = 2;
        if (delta < 0) {
            globals.word9181ETwitterCarryLatch = 1;
            return 0;
        }
    }
    return result;
}

static uint32_t PrStage1ScorerDirectClearCurrentBucket14C1C(
    PrStage1ScorerDirectGlobals& globals) {
    const uint32_t previousMask = globals.dword91808AcceptedMask;
    globals.word91810AcceptedCount = 0;
    globals.word91812RecordWriteShadowCount = 0;
    globals.word91814PenaltySplitCount = 0;
    globals.word91822AdditiveTerm = 0;
    globals.dword91808AcceptedMask = 0;
    globals.dword9180CLastClearedAcceptedMask = previousMask;
    return previousMask;
}

static uint16_t PrStage1ScorerDirectWiderHelperReset14400(
    PrStage1ScorerDirectGlobals& globals) {
    globals.word91820NoInputCounter = 0;
    globals.word9181ETwitterCarryLatch = 0;
    globals.word91814PenaltySplitCount = 0;
    globals.word91812RecordWriteShadowCount = 0;
    globals.word91810AcceptedCount = 0;
    globals.word91826CoolGainLatch = 0;
    globals.word91828CoolGainStreak = 0;
    return PrStage1ScorerDirectRefreshBaselines1448C(globals);
}

static void PrStage1ScorerDirectSeedCurrentPageHelper14BDC(
    PrStage1ScorerDirectGlobals& globals,
    uint8_t pageOrdinal1Based) {
    globals.currentWritePageOrdinalKnown = true;
    globals.currentWritePageOrdinal1Based = pageOrdinal1Based;
    globals.pageClearPending14BDC = true;
    globals.pageClearOrdinal1Based14BDC = pageOrdinal1Based;
}

static void PrStage1ScorerDirectClearCurrentPageStorage25C44(
    PrStage1ScorerDirectGlobals& globals,
    uint8_t pageOrdinal1Based) {
    globals.ringPages[ResolveRingIndex(static_cast<int>(pageOrdinal1Based))]
        .records.fill({});
}

static void PrStage1ScorerDirectClearCurrentPage14BDC(
    PrStage1ScorerDirectGlobals& globals,
    uint8_t pageOrdinal1Based) {
    // Literal `sub_80014BDC(ctx+56)` order:
    // 1. seed the writer-only current-page helper (`gp+0x1C`)
    // 2. clear the selected 384-byte page via `sub_80025C44`
    PrStage1ScorerDirectSeedCurrentPageHelper14BDC(globals, pageOrdinal1Based);
    PrStage1ScorerDirectClearCurrentPageStorage25C44(globals, pageOrdinal1Based);
}

PrStage1ScorerDirectPageClear14BDCResult
PrStage1ScorerDirectRunPageClearCore14BDC(
    PrStage1ScorerDirectGlobals& globals,
    uint16_t pageOrdinal1Based,
    bool runPageClear14BDC) {
    PrStage1ScorerDirectPageClear14BDCResult out{};
    out.requested = runPageClear14BDC;
    out.targetKnown = runPageClear14BDC;
    out.targetOrdinal1Based = static_cast<uint8_t>(pageOrdinal1Based);
    if (!out.requested || !out.targetKnown) {
        return out;
    }

    PrStage1ScorerDirectClearCurrentPage14BDC(globals, out.targetOrdinal1Based);
    out.clearApplied = true;
    return out;
}

static void PrStage1ScorerDirectApplyLatentPageClearTarget14BDC(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectPageClear14BDCResult& pageClear) {
    if (!pageClear.targetKnown) {
        return;
    }

    // Helper-only seam: mirror the literal 14BDC local page maintenance into
    // direct globals without committing any host-side activation yet.
    PrStage1ScorerDirectClearCurrentPage14BDC(
        globals,
        pageClear.targetOrdinal1Based);
}

static void PrStage1ScorerDirectResetReplayBuffer24E54(
    PrStage1ScorerDirectReplayBufferState& replay,
    bool preservePublishedCountIfEmpty) {
    // `sub_80024E54` only resets the shared replay-buffer cursors/state.
    // The backing arrays stay intact until later setup chooses to restore or
    // overwrite them.
    replay.dword901C0WriteCount = 0u;
    if (preservePublishedCountIfEmpty) {
        if (replay.dword901BCPublishedCount == 0u) {
            replay.dword901BCPublishedCount =
                static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity);
        }
    } else {
        replay.dword901BCPublishedCount = 0u;
    }
}

static bool PrStage1ScorerDirectReplayMirrorShapeKnown(
    const PrStage1ScorerDirectReplayBufferState& replay) {
    const uint32_t capacity =
        static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity);
    return replay.replayMirrorKnown8008EEF8 &&
           replay.replayMirrorProducerKnown8008EEF8 &&
           PrStage1ScorerDirectIsKnownReplayMirrorProducerFunction(
               replay.replayMirrorProducerFunction) &&
           replay.replayMirrorByteCountKnown8008EEF8 &&
           replay.replayMirrorKnownByteCount8008EEF8 >=
               kPrStage1ScorerDirectReplayMirrorByteCount &&
           replay.dword901BCPublishedCount <= capacity &&
           replay.dword901C0WriteCount <= capacity;
}

bool PrStage1ScorerDirectIsKnownReplayMirrorProducerFunction(
    uint32_t psxFunction) {
    return psxFunction == kPrStage1ScorerDirectFn801C4FC8 ||
           psxFunction == kPrStage1ScorerDirectFn801C8660 ||
           psxFunction == kPrStage1ScorerDirectFn80014614 ||
           psxFunction == kPrStage1ScorerDirectFn8001681C;
}

PrStage1ScorerDirectReplayBackupCaptureResult
PrStage1ScorerDirectRunReplayBackupCaptureCore1635C(
    const PrStage1ScorerDirectReplayBufferState& replay) {
    PrStage1ScorerDirectReplayBackupCaptureResult out{};
    if (!PrStage1ScorerDirectReplayMirrorShapeKnown(replay)) {
        return out;
    }
    out.captureApplied = true;
    out.backup.valid = true;
    out.backup.dword92F48PublishedCount = replay.dword901BCPublishedCount;
    for (size_t i = 0; i < kPrStage1ScorerDirectReplayBufferCapacity; ++i) {
        out.backup.dwordEEF8Tick96[i] = replay.dwordEEF8Tick96[i];
        out.backup.dwordEEFCClassMask[i] = replay.dwordEEFCClassMask[i];
    }
    return out;
}

PrStage1ScorerDirectEventTableBuild801C8660Result
PrStage1ScorerDirectBuildStage1EventTable801C8660(
    PrStage1ScorerDirectReplayBufferState& replay) {
    PrStage1ScorerDirectEventTableBuild801C8660Result out{};
    out.applied = true;
    out.count800901BC =
        static_cast<uint32_t>(kStage1EventTableSource801D2E2C.size());

    const size_t count =
        (std::min)(kStage1EventTableSource801D2E2C.size(),
                   kPrStage1ScorerDirectReplayBufferCapacity);
    for (size_t i = 0; i < count; ++i) {
        const Stage1EventTableSourceRow801C8660& row =
            kStage1EventTableSource801D2E2C[i];
        replay.dwordEEF8Tick96[i] =
            static_cast<uint32_t>(24 * static_cast<int32_t>(row.timeIndex));
        replay.dwordEEFCClassMask[i] =
            PrStage1ScorerDirectMapKind80024BC0(row.kind);
    }
    replay.replayMirrorKnown8008EEF8 = true;
    replay.replayMirrorProducerKnown8008EEF8 = true;
    replay.replayMirrorProducerFunction = kPrStage1ScorerDirectFn801C8660;
    replay.replayMirrorByteCountKnown8008EEF8 = true;
    replay.replayMirrorKnownByteCount8008EEF8 =
        static_cast<uint32_t>(count * 2u * sizeof(uint32_t));
    replay.dword901BCPublishedCount = out.count800901BC;
    return out;
}

static bool PrStage1ScorerDirectRestoreReplayBuffer1681C(
    const PrStage1ScorerDirectReplayBackupState& backup,
    PrStage1ScorerDirectReplayBufferState& replay) {
    if (!backup.valid) {
        return false;
    }
    if (backup.dword92F48PublishedCount >
        static_cast<uint32_t>(kPrStage1ScorerDirectReplayBufferCapacity)) {
        replay.replayMirrorKnown8008EEF8 = false;
        replay.replayMirrorProducerKnown8008EEF8 = false;
        replay.replayMirrorProducerFunction = 0;
        replay.replayMirrorByteCountKnown8008EEF8 = false;
        replay.replayMirrorKnownByteCount8008EEF8 = 0;
        return false;
    }

    replay.dword901BCPublishedCount = backup.dword92F48PublishedCount;
    for (size_t i = 0; i < replay.dword901BCPublishedCount; ++i) {
        replay.dwordEEF8Tick96[i] = backup.dwordEEF8Tick96[i];
        replay.dwordEEFCClassMask[i] = backup.dwordEEFCClassMask[i];
    }
    replay.replayMirrorKnown8008EEF8 = true;
    replay.replayMirrorProducerKnown8008EEF8 = true;
    replay.replayMirrorProducerFunction = kPrStage1ScorerDirectFn8001681C;
    replay.replayMirrorByteCountKnown8008EEF8 = true;
    replay.replayMirrorKnownByteCount8008EEF8 =
        backup.dword92F48PublishedCount * 2u * sizeof(uint32_t);
    // `sub_8001681C` leaves `901C0` at 0 after `24E54(0)` so the replay
    // consumer restarts from the first restored slot.
    return replay.dword901BCPublishedCount != 0u;
}

static PrStage1ScorerDirectAcceptedReplayRestoreResult
PrStage1ScorerDirectRunAcceptedReplayRestoreCore1681C(
    const PrStage1ScorerDirectReplayBackupState& backup,
    PrStage1ScorerDirectReplayBufferState& replay) {
    PrStage1ScorerDirectAcceptedReplayRestoreResult out{};
    if (!backup.valid) {
        return out;
    }
    out.restoreApplied =
        PrStage1ScorerDirectRestoreReplayBuffer1681C(backup, replay);
    out.scriptedWriterResetRequired = true;
    return out;
}

PrStage1ScorerDirectAcceptedSpecialSetupResult
PrStage1ScorerDirectRunAcceptedSpecialSetupCore24E54_1681C(
    uint16_t transitionState,
    const PrStage1ScorerDirectResolvedReplayBackup1681C& resolvedBackup,
    PrStage1ScorerDirectReplayBufferState& replay) {
    PrStage1ScorerDirectAcceptedSpecialSetupResult out{};
    out.setup = PrStage1ScorerDirectRunAcceptedReplaySetupCore24E54_7A60(
        replay,
        transitionState);
    out.restoreSource = resolvedBackup.source;
    if (out.setup.restoreReplayBuffer1681CRequested &&
        resolvedBackup.source != PrStage1ScorerDirectReplayRestoreSource1681C::None) {
        out.restore = PrStage1ScorerDirectRunAcceptedReplayRestoreCore1681C(
            resolvedBackup.backup,
            replay);
    }
    return out;
}

PrStage1ScorerDirectResolvedReplayBackup1681C
PrStage1ScorerDirectResolveReplayRestoreSource1681C(
    bool payloadBackupValid,
    const PrStage1ScorerDirectReplayBackupState& payloadBackup,
    bool sidecarBackupValid,
    const PrStage1ScorerDirectReplayBackupState& sidecarBackup) {
    PrStage1ScorerDirectResolvedReplayBackup1681C out{};
    if (payloadBackupValid) {
        out.backup = payloadBackup;
        out.source =
            PrStage1ScorerDirectReplayRestoreSource1681C::Payload92F48_92F5C;
        return out;
    }
    if (sidecarBackupValid) {
        out.backup = sidecarBackup;
        out.source = PrStage1ScorerDirectReplayRestoreSource1681C::Sidecar;
    }
    return out;
}

static int32_t PrStage1ScorerDirectResolveStageSelectSaveSlot1615C(int32_t stage) {
    if (stage < 1 || stage > 6) {
        return -1;
    }
    return stage - 1;
}

static bool PrStage1ScorerDirectResolveAllClearLatch161F4(
    const int16_t* stageSelectStatus) {
    if (!stageSelectStatus) {
        return false;
    }
    for (int32_t i = 1; i <= 6; ++i) {
        if (stageSelectStatus[i] != 3) {
            return false;
        }
    }
    return true;
}

static PrStage1ScorerDirectStageUnlockCore1628CResult
PrStage1ScorerDirectRunStageUnlockCore1628C(
    int32_t stage,
    int32_t currentStatus) {
    PrStage1ScorerDirectStageUnlockCore1628CResult out{};
    out.slotValid = PrStage1ScorerDirectResolveStageSelectSaveSlot1615C(stage) >= 0;
    if (!out.slotValid) {
        return out;
    }
    out.promoteRequired = currentStatus == 0;
    out.resultingStatus = out.promoteRequired ? 1 : currentStatus;
    return out;
}

static PrStage1ScorerDirectClearTerminalSaveInputs1635C
PrStage1ScorerDirectResolveClearTerminalSaveInputs1635C(
    uint16_t rightRankActiveRow,
    int32_t currentStageStatus166AC) {
    PrStage1ScorerDirectClearTerminalSaveInputs1635C out{};
    out.statusPromotion = rightRankActiveRow == 0u ? 3 : 2;
    out.prevGrade92F40 =
        static_cast<uint32_t>((std::max)(currentStageStatus166AC, 1));
    return out;
}

PrStage1ScorerDirectClearTerminalSaveTailCoreResult
PrStage1ScorerDirectRunClearTerminalSaveTailCore1635C_1628C(
    uint16_t rightRankActiveRow,
    int32_t currentStageStatus166AC,
    int32_t nextStage,
    int32_t nextStageCurrentStatus) {
    PrStage1ScorerDirectClearTerminalSaveTailCoreResult out{};
    out.clearInputs = PrStage1ScorerDirectResolveClearTerminalSaveInputs1635C(
        rightRankActiveRow,
        currentStageStatus166AC);
    out.unlockResult = PrStage1ScorerDirectRunStageUnlockCore1628C(
        nextStage,
        nextStageCurrentStatus);
    return out;
}

PrStage1ScorerDirectStageSelectSaveStatusCore1635CResult
PrStage1ScorerDirectRunStageSelectSaveStatusCore1635C(
    int32_t stage,
    int32_t score,
    const int16_t* stageSelectStatus) {
    PrStage1ScorerDirectStageSelectSaveStatusCore1635CResult out{};
    if (!stageSelectStatus) {
        return out;
    }

    const int32_t slotIndex =
        PrStage1ScorerDirectResolveStageSelectSaveSlot1615C(stage);
    if (slotIndex < 0) {
        return out;
    }
    out.slotValid = true;
    out.slotIndex = slotIndex;
    out.dword92F24ScoreValue = static_cast<uint32_t>(score);
    out.dword92F3CLastSavedSlot = static_cast<uint32_t>(slotIndex);
    out.dword92F44AllClearLatch =
        PrStage1ScorerDirectResolveAllClearLatch161F4(stageSelectStatus) ? 1u : 0u;
    return out;
}

static PrStage1ScorerDirectAcceptedReplaySetupResult
PrStage1ScorerDirectRunAcceptedReplaySetupCore24E54_7A60(
    PrStage1ScorerDirectReplayBufferState& replay,
    uint16_t transitionState) {
    // COMOD1 setup enters the accepted replay setup lane as:
    //   `24E54(0) -> ctx+0x52 write -> optional 1681C restore`
    // This helper only covers the direct-core portion and returns the
    // caller-side replay/suppress/restore intents.
    PrStage1ScorerDirectAcceptedReplaySetupResult out{};
    PrStage1ScorerDirectResetReplayBuffer24E54(replay, false);
    out.restoreReplayBuffer1681CRequested = transitionState == 2u;
    return out;
}

int32_t PrStage1ScorerDirectPublishScoreMirror80024FC0(
    const PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx) {
    ctx.dword30ScoreDisplayValue = globals.word91816Accumulator;
    return globals.word91816Accumulator;
}

static void PrStage1ScorerDirectResetRightRankCache24F8C(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx) {
    globals.word8ED36Phase1Cache = 2u;
    globals.word8ED38PhaseCounter = 0u;
    ctx.word5ABlinkEnabled = 0u;
    PrStage1ScorerDirectRefreshBaselines1448C(globals);
}

static uint16_t PrStage1ScorerDirectResolveNonCoolBlinkTarget55420(
    uint16_t activeRow,
    uint8_t classifier) {
    static constexpr uint16_t kBlinkTarget55420[4][2] = {
        {1u, 0u},
        {2u, 0u},
        {3u, 1u},
        {3u, 2u},
    };
    if (activeRow < 4u && classifier < 2u) {
        return kBlinkTarget55420[activeRow][classifier];
    }
    return activeRow;
}

static int32_t PrStage1ScorerDirectPairBonus14A80(
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectPage& page,
    size_t coverageCount) {
    if (globals.word9182APairBonusGate == 0u) {
        return 0;
    }

    size_t clampedCount = (std::min)(coverageCount, kPrStage1ScorerDirectSlotsPerPage);
    if (clampedCount < 2u) {
        return 0;
    }

    int32_t emptyPairCount = 0;
    int32_t secondOnlyCount = 0;
    int32_t firstOnlyCount = 0;
    int32_t bothFilledCount = 0;
    uint8_t occupancyMask = 0u;
    for (size_t slotIndex = 0u; slotIndex + 1u < clampedCount; slotIndex += 2u) {
        const bool firstFilled = page.records[slotIndex].word06Occupied != 0u;
        const bool secondFilled = page.records[slotIndex + 1u].word06Occupied != 0u;
        if (firstFilled) {
            if (secondFilled) {
                occupancyMask |= 8u;
                ++bothFilledCount;
            } else {
                occupancyMask |= 4u;
                ++firstOnlyCount;
            }
        } else if (secondFilled) {
            occupancyMask |= 2u;
            ++secondOnlyCount;
        } else {
            ++emptyPairCount;
        }
    }

    switch (occupancyMask) {
    case 14u: {
        int32_t result = 15 * secondOnlyCount + 6 * firstOnlyCount + 9 * bothFilledCount;
        if (emptyPairCount > 0) {
            result += 18;
        }
        return result;
    }
    case 12u:
        return 9 * bothFilledCount + 6 * firstOnlyCount;
    case 10u:
        return 9 * bothFilledCount + 15 * secondOnlyCount;
    case 6u:
        return 15 * secondOnlyCount + 6 * firstOnlyCount;
    default:
        return 0;
    }
}

static PrStage1ScorerDirectCommitState PrStage1ScorerDirectCommit14D58(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectDescriptorRow& row,
    uint16_t descriptorSubstate50) {
    PrStage1ScorerDirectCommitState out{};
    out.additiveTermKnown = true;
    out.additiveTermValue = globals.word91822AdditiveTerm;
    out.acceptedCountKnown = true;
    out.acceptedCountValue = globals.word91810AcceptedCount;
    if (globals.word91810AcceptedCount == 0u) {
        out.commitTermKnown = true;
        out.commitTermValue = -1 + globals.word91822AdditiveTerm;
        const CommitWritebackState writeback =
            ResolveCommitWriteback(globals, out.commitTermValue);
        out.scoreWritebackKnown = writeback.known;
        out.scoreWritebackValue = writeback.value;
        out.clampActiveKnown = true;
        out.clampActive = writeback.clampActive;
        return out;
    }
    if (!row.valid) {
        return out;
    }

    out.currentBranchKnown = true;
    out.currentBranchSpecial = descriptorSubstate50 != 0u;

    const PrStage1ScorerDirectDescriptorBranch& selectedBranch =
        PrStage1ScorerDirectSelectBranch(row, descriptorSubstate50);
    out.currentLookbackPageCountKnown = true;
    out.currentLookbackPageCountValue = selectedBranch.word0ELookbackPageCount;
    out.overflowActiveKnown = true;
    out.overflowActive =
        selectedBranch.word0ELookbackPageCount > 0u &&
        static_cast<int32_t>(globals.word91810AcceptedCount) >
            12 * static_cast<int32_t>(selectedBranch.word0ELookbackPageCount);

    out.defaultBranch = BuildDefaultBranchState(
        globals,
        row,
        globals.word91810AcceptedCount,
        globals.word91814PenaltySplitCount);
    out.specialBranch = BuildSpecialBranchState(
        globals,
        row,
        globals.word91810AcceptedCount,
        globals.word91814PenaltySplitCount);
    out.activeBranch = out.currentBranchSpecial ? out.specialBranch : out.defaultBranch;

    if (out.activeBranch.descriptorSubdeltaKnown) {
        out.commitTermKnown = true;
        out.commitTermValue =
            out.activeBranch.descriptorSubdeltaValue + globals.word91822AdditiveTerm;
        const CommitWritebackState writeback =
            ResolveCommitWriteback(globals, out.commitTermValue);
        out.scoreWritebackKnown = writeback.known;
        out.scoreWritebackValue = writeback.value;
        out.clampActiveKnown = true;
        out.clampActive = writeback.clampActive;
    }

    return out;
}

static PrStage1ScorerDirectCommitSliceResult PrStage1ScorerDirectRunBucket30CommitSlice(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectDescriptorRow& row,
    const PrStage1ScorerDirectBucketContext& ctx) {
    PrStage1ScorerDirectCommitSliceResult out{};
    out.busyGateActive = PrStage1ScorerDirectBusyGate24BF4(globals, ctx);
    out.scorerWindowOpen =
        !out.busyGateActive && (globals.dword8ED08DescriptorFlags & 0x0003) != 0;
    if (!out.scorerWindowOpen) {
        return out;
    }

    out.scorerRan = true;
    out.commit = PrStage1ScorerDirectCommit14D58(
        globals,
        row,
        ctx.word50DescriptorSubstate);
    PrStage1ScorerDirectAdvanceNoInputCounter14458(globals);
    out.noInputCounterRan = true;
    return out;
}

static PrStage1ScorerDirectPhase1StepResult PrStage1ScorerDirectRunBucket30Phase1Slice(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectDescriptorRow& row,
    PrStage1ScorerDirectBucketContext& ctx) {
    (void)row;
    PrStage1ScorerDirectPhase1StepResult out{};
    if (ctx.word4ERightRankActiveRow == 0u) {
        const uint8_t coolRet = static_cast<uint8_t>(
            PrStage1ScorerDirectCoolClassifier14C80(globals));
        out.sampledClassifier = coolRet;
        out.directConsumerSlotKnown = true;
        switch (coolRet) {
        case 0u:
            // PSX `ret==0` path: local `v15=1`, `v21=1`, `ctx+0x5A=0`.
            out.directConsumerSlot = 1u;
            ctx.word5ABlinkEnabled = 0u;
            break;
        case 1u:
            // PSX `ret==1` path: local `v15=0`, `v21=0`,
            // `ctx+0x58=1`, `ctx+0x5A=1`.
            out.directConsumerSlot = 0u;
            out.blinkEmit = true;
            out.blinkTargetRow = 1u;
            ctx.word58BlinkTargetRow = out.blinkTargetRow;
            ctx.word5ABlinkEnabled = 1u;
            break;
        default:
            // Keep the dead `ret==2` caller arm encoded so the direct-port
            // shape still mirrors `sub_80024FD0` even though the current
            // `sub_80014C80` binary only returns 0/1.
            out.directConsumerSlot = 0u;
            out.blinkEmit = true;
            out.blinkTargetRow = 3u;
            ctx.word58BlinkTargetRow = out.blinkTargetRow;
            ctx.word5ABlinkEnabled = 1u;
            break;
        }
        return out;
    }

    out.sampledClassifier =
        static_cast<uint8_t>(PrStage1ScorerDirectPositiveGrowth14D28(globals));
    if (globals.word8ED38PhaseCounter == 0u) {
        const bool suppressGoodPositiveEmit =
            ctx.word4ERightRankActiveRow == 1u &&
            out.sampledClassifier == 1u &&
            globals.word8ED34ShortWindowLatch == 0u;
        if (!suppressGoodPositiveEmit) {
            out.blinkEmit = true;
            out.blinkTargetRow =
                PrStage1ScorerDirectResolveNonCoolBlinkTarget55420(
                    ctx.word4ERightRankActiveRow,
                    out.sampledClassifier);
            ctx.word5ABlinkEnabled = 1u;
            ctx.word58BlinkTargetRow = out.blinkTargetRow;
        }
        globals.word8ED36Phase1Cache = out.sampledClassifier;
        globals.word8ED38PhaseCounter = static_cast<uint16_t>(globals.word8ED38PhaseCounter + 1u);
        out.firstBeat = true;
        return out;
    }

    const bool awfulZeroCarry =
        ctx.word4ERightRankActiveRow == 3u &&
        globals.word8ED36Phase1Cache == 0u &&
        out.sampledClassifier == 0u;
    globals.word9181ETwitterCarryLatch = awfulZeroCarry ? 1u : 0u;
    if (globals.word8ED36Phase1Cache != out.sampledClassifier) {
        globals.word8ED36Phase1Cache = 2u;
    }
    globals.word8ED38PhaseCounter = 0u;
    ctx.word5ABlinkEnabled = 0u;
    return out;
}

static PrStage1ScorerDirectResolutionResult
PrStage1ScorerDirectResolveBucket30V22AndResetCache(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectDescriptorRow& row,
    PrStage1ScorerDirectBucketContext& ctx) {
    PrStage1ScorerDirectResolutionResult out{};
    if ((globals.dword8ED08DescriptorFlags & 0x0004) == 0) {
        return out;
    }
    if (globals.dword8ED00FollowUpState != 0) {
        return out;
    }

    out.resolutionCalled = true;
    out.resolutionInputPhase1Classifier36 = globals.word8ED36Phase1Cache;
    uint8_t v22 = 2u;
    if (globals.word8ED36Phase1Cache == 1u) {
        if (ctx.word4ERightRankActiveRow != globals.word8ED36Phase1Cache) {
            v22 = 1u;
        } else {
            out.tieBreakerCalled = true;
            out.tieBreakerResult14548 =
                PrStage1ScorerDirectTieBreak14548(globals, ctx, row);
            if (out.tieBreakerResult14548 == 1) {
                v22 = 1u;
            }
        }
    } else if (globals.word8ED36Phase1Cache == 0u) {
        v22 = 0u;
        if (ctx.word4ERightRankActiveRow == 3u) {
            out.tieBreakerCalled = true;
            out.tieBreakerResult14548 =
                PrStage1ScorerDirectTieBreak14548(globals, ctx, row);
            v22 =
                static_cast<uint8_t>(2 * (out.tieBreakerResult14548 != 0));
        }
    }

    out.resolutionV22 = v22;
    PrStage1ScorerDirectResetRightRankCache24F8C(globals, ctx);
    return out;
}

static PrStage1ScorerDirectBucket30RowWriteDecision
PrStage1ScorerDirectResolveBucket30RowWriteDecision(
    uint8_t activeRow,
    bool goodToCoolGateEnabled,
    const PrStage1ScorerDirectResolutionResult& resolution) {
    PrStage1ScorerDirectBucket30RowWriteDecision out{};
    // Keep `resolutionKnown` tied to the same-tick `24F8C/v22` result, not
    // to committed row-write. The Stage1 no-commit contract intentionally
    // observes `resolutionKnown=1, v22=2, writeback=0` on GOOD row samples.
    out.resolutionKnown = resolution.resolutionCalled;
    out.resolutionV22 = resolution.resolutionV22;
    out.resolvedRightRankRowKnown = true;
    out.resolvedRightRankRow = activeRow;
    if (resolution.resolutionCalled) {
        switch (activeRow) {
        case 1:
            if (resolution.resolutionV22 == 1u) {
                out.resolvedRightRankRow = goodToCoolGateEnabled ? 0u : 1u;
            } else if (resolution.resolutionV22 == 0u) {
                out.resolvedRightRankRow = 2u;
            }
            break;
        case 2:
            if (resolution.resolutionV22 == 1u) {
                out.resolvedRightRankRow = 1u;
            } else if (resolution.resolutionV22 == 0u) {
                out.resolvedRightRankRow = 3u;
            }
            break;
        case 3:
            out.resolvedRightRankRow =
                (resolution.resolutionV22 == 1u) ? 2u : 3u;
            break;
        case 0:
        default:
            out.resolvedRightRankRow =
                (resolution.resolutionV22 == 0u) ? 1u : 0u;
            break;
        }
    }

    out.goodToCoolCommitted =
        activeRow == 1u &&
        resolution.resolutionCalled &&
        resolution.resolutionV22 == 1u &&
        goodToCoolGateEnabled &&
        out.resolvedRightRankRow == 0u;
    out.rightRankWritebackCommitted =
        resolution.resolutionCalled &&
        resolution.resolutionV22 != 2u &&
        !(activeRow == 3u && resolution.resolutionV22 == 0u) &&
        !(activeRow == 1u && resolution.resolutionV22 == 1u &&
          !goodToCoolGateEnabled);
    out.directConsumerFallback94400 =
        resolution.resolutionCalled && !out.rightRankWritebackCommitted;
    if (!out.rightRankWritebackCommitted) {
        out.resolvedRightRankRow = activeRow;
    }
    return out;
}

static uint16_t PrStage1ScorerDirectResolveBucket30RowWriteTransitionAnim18E(
    uint8_t activeRow,
    uint8_t resolutionV22,
    bool goodToCoolGateEnabled) {
    if (activeRow == 1u && resolutionV22 == 1u && goodToCoolGateEnabled) {
        return 5u;
    }
    if (activeRow == 1u && resolutionV22 == 0u) {
        return 1u;
    }
    if (activeRow == 2u && resolutionV22 == 1u) {
        return 4u;
    }
    if (activeRow == 2u && resolutionV22 == 0u) {
        return 2u;
    }
    if (activeRow == 3u && resolutionV22 == 1u) {
        return 3u;
    }
    if (activeRow == 0u && resolutionV22 == 0u) {
        return 6u;
    }
    return 0u;
}

static PrStage1ScorerDirectBucket30RowWriteSliceResult
PrStage1ScorerDirectRunBucket30RowWriteSlice(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    uint8_t activeRow,
    bool goodToCoolGateEnabled,
    int32_t tick96,
    int32_t goodToCoolDelayTick96,
    const PrStage1ScorerDirectResolutionResult& resolution) {
    PrStage1ScorerDirectBucket30RowWriteSliceResult out{};
    out.rowWrite = PrStage1ScorerDirectResolveBucket30RowWriteDecision(
        activeRow,
        goodToCoolGateEnabled,
        resolution);
    if (!out.rowWrite.resolutionKnown) {
        return out;
    }

    if (out.rowWrite.goodToCoolCommitted) {
        const uint16_t nextCtx72 =
            static_cast<uint16_t>(ctx.word72FollowUpCounter + 1u);
        out.playPrepareCue = true;
        out.prepareCueUseRepeatVariant = nextCtx72 >= 2u;
        out.ctxFlag2000Pulse = true;
        out.followUpPhaseAction =
            PrStage1ScorerDirectFollowUpPhaseAction::SetAwaitBucket31AfterGoodToCool;
        ctx.word50DescriptorSubstate = 1u;
        globals.dword8ED24FollowUpWriteback = 1;
        globals.dword8ED00FollowUpState = 7;
        globals.dword8ED14FollowUpActive = 1;
        globals.dword8ED0CFollowUpDeadlineTick =
            tick96 + goodToCoolDelayTick96;
        ctx.word72FollowUpCounter = nextCtx72;
        ctx.word4ERightRankActiveRow = out.rowWrite.resolvedRightRankRow;
        ctx.word18ETransitionAnim = 5u;
        ctx.word7AFollowUpState = 0u;
        return out;
    }

    if (out.rowWrite.rightRankWritebackCommitted) {
        ctx.word4ERightRankActiveRow = out.rowWrite.resolvedRightRankRow;
        ctx.word18ETransitionAnim =
            PrStage1ScorerDirectResolveBucket30RowWriteTransitionAnim18E(
                activeRow,
                out.rowWrite.resolutionV22,
                goodToCoolGateEnabled);
        if (activeRow == 1u && out.rowWrite.resolutionV22 == 0u) {
            ctx.word74HelperCounter =
                static_cast<uint16_t>(ctx.word74HelperCounter + 1u);
        }
        out.followUpPhaseAction = PrStage1ScorerDirectFollowUpPhaseAction::ClearToNone;
        ctx.word50DescriptorSubstate = 0u;
        globals.dword8ED00FollowUpState = 6;
        globals.dword8ED14FollowUpActive = 0;
    }
    return out;
}

static PrStage1ScorerDirectBucket30DelayedTailSliceResult
PrStage1ScorerDirectRunBucket30DelayedTailSlice(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    uint16_t descriptorFlagWord) {
    PrStage1ScorerDirectBucket30DelayedTailSliceResult out{};
    int tailEd00 = globals.dword8ED00FollowUpState;
    if (tailEd00 == 1 &&
        !PrStage1ScorerDirectBusyGate24BF4(globals, ctx) &&
        (descriptorFlagWord & 0x0020u) != 0u &&
        globals.word91826CoolGainLatch != 0u) {
        tailEd00 = 4;
        globals.dword8ED00FollowUpState = 4;
    }

    if (tailEd00 == 4 && (descriptorFlagWord & 0x0020u) != 0u) {
        out.playArmCue = true;
        out.flag4000Set = true;
        globals.dword8ED14FollowUpActive = 1;
        globals.dword8ED00FollowUpState = 5;
        tailEd00 = 5;
    }

    if (tailEd00 != 5 || (descriptorFlagWord & 0x0040u) == 0u) {
        return out;
    }

    const uint8_t previousRow =
        static_cast<uint8_t>(ctx.word4ERightRankActiveRow);
    out.publishForcedGoodRowWrite = true;
    out.forcedGoodPublish.resolvedRightRankRowKnown = true;
    out.forcedGoodPublish.resolvedRightRankRow = 1u;
    out.forcedGoodPublish.rightRankWritebackCommitted = true;
    out.forcedGoodPublish.rowWriteEventKnown = previousRow != 1u;
    out.forcedGoodPublish.rowWritePreviousRow = previousRow;
    ctx.word50DescriptorSubstate = 0u;
    out.followUpPhaseAction = PrStage1ScorerDirectFollowUpPhaseAction::ClearToNone;
    globals.gPrStageEventStreamDone = true;
    ctx.word4ERightRankActiveRow = 1u;
    ctx.word54Permit = 1u;
    ctx.word7AFollowUpState = 1u;
    ctx.word18ETransitionAnim = 6u;
    globals.dword8ED00FollowUpState = 8;
    globals.dword8ED14FollowUpActive = 0;
    globals.word8ED36Phase1Cache = 1u;
    globals.word8ED38PhaseCounter = 0u;
    out.armBucket31DelayedSteadySfxCompletion = true;
    return out;
}

static PrStage1ScorerDirectBucket31ConsumerSliceResult
PrStage1ScorerDirectRunBucket31ConsumerSlice(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    int32_t preBucket31Ed00,
    bool awaitBucket31AfterGoodToCoolActive,
    bool bucket31NarrowClearPending,
    bool bucket31AdditiveClearPending) {
    PrStage1ScorerDirectBucket31ConsumerSliceResult out{};
    if (awaitBucket31AfterGoodToCoolActive) {
        out.followUpPhaseAction =
            PrStage1ScorerDirectFollowUpPhaseAction::SetAwaitForceGoodWriteback;
    }
    if (preBucket31Ed00 < 6) {
        (void)bucket31NarrowClearPending;
        (void)bucket31AdditiveClearPending;
        return out;
    }

    out.consumerPackageRan = true;
    out.ctxFlag0200Pulse = true;
    switch (preBucket31Ed00) {
    case 6:
        globals.dword8ED00FollowUpState = 0;
        break;
    case 7:
        globals.dword8ED00FollowUpState = 1;
        break;
    case 8:
        globals.dword8ED00FollowUpState = 0;
        out.playCompletionCue = true;
        out.clearDelayedCompletionPending = true;
        break;
    default:
        break;
    }

    PrStage1ScorerDirectWiderHelperReset14400(globals);
    (void)PrStage1ScorerDirectClearCurrentBucket14C1C(globals);
    out.bucketLocalClearRan = true;
    out.narrowClearFired = bucket31NarrowClearPending;
    out.clearDeferredAdditiveBookkeeping = bucket31AdditiveClearPending;
    PrStage1ScorerDirectResetRightRankCache24F8C(globals, ctx);
    return out;
}

static PrStage1ScorerDirectBucket30ClearSliceResult
PrStage1ScorerDirectRunBucket30ClearSlice(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectBucket30AcceptedClearDecision& decision) {
    PrStage1ScorerDirectBucket30ClearSliceResult out{};
    out.action = decision.action;
    out.markBucket31AdditiveClearPending =
        decision.markBucket31AdditiveClearPending;
    if (decision.action != PrStage1ScorerDirectAcceptedClearAction::ClearBucket30Now) {
        return out;
    }

    (void)PrStage1ScorerDirectClearCurrentBucket14C1C(globals);
    out.bucketLocalClearRan = true;
    out.clearDeferredAdditiveBookkeeping =
        decision.clearAdditiveBookkeepingNow;
    return out;
}

struct PrStage1ScorerDirectBucket30DirectConsumerSliceResult {
    PrStage1ScorerDirectSteadySfxAction steadySfx{};
    PrStage1ScorerDirectBucket30AcceptedClearDecision clearDecision{};
};

static void PrStage1ScorerDirectPublishBucket30PreRowWriteCue943FC(
    PrStage1ScorerDirectBucket30DirectConsumerSliceResult& out,
    const PrStage1ScorerDirectBucket30RowWriteDecision& rowWrite) {
    if (!rowWrite.rightRankWritebackCommitted ||
        !rowWrite.resolutionKnown ||
        rowWrite.resolutionV22 >= 2u) {
        return;
    }

    out.steadySfx.play = true;
    out.steadySfx.cueId = PrStage1ScorerDirectSteadyCueId::Bucket30Verdict;
    out.steadySfx.tableSlot = rowWrite.resolutionV22;
}

static PrStage1ScorerDirectBucket30DirectConsumerSliceResult
PrStage1ScorerDirectRunBucket30DirectConsumerSlice103104(
    int32_t preBucket30Ed00,
    uint8_t activeRow,
    bool ownerKernelOpen,
    bool resolverGateBit4,
    bool phase1LatchArmed38,
    bool followUpPhaseIsNone,
    const PrStage1ScorerDirectPhase1StepResult& phase1,
    const PrStage1ScorerDirectBucket30RowWriteSliceResult& rowWrite) {
    (void)activeRow;
    PrStage1ScorerDirectBucket30DirectConsumerSliceResult out{};
    const bool preBucket30ImmediateFollowUpClear =
        preBucket30Ed00 == 1 || preBucket30Ed00 == 4;
    // PSX `ED08&4 == 0` falls through `LABEL_78` into the same
    // `LABEL_103` direct-consumer clear as the v22==2 no-commit path.
    const bool directConsumer94400 =
        preBucket30ImmediateFollowUpClear ||
        (ownerKernelOpen && preBucket30Ed00 == 0 && !resolverGateBit4 &&
         !rowWrite.rowWrite.resolutionKnown) ||
        rowWrite.rowWrite.directConsumerFallback94400;
    const bool waitSecondBeatInsideBucket30 =
        preBucket30Ed00 == 0 &&
        resolverGateBit4 &&
        !rowWrite.rowWrite.resolutionKnown &&
        phase1LatchArmed38 &&
        followUpPhaseIsNone;
    const bool acceptedTailSurvived =
        preBucket30Ed00 == 0 &&
        resolverGateBit4 &&
        rowWrite.rowWrite.resolutionKnown &&
        !rowWrite.rowWrite.directConsumerFallback94400 &&
        rowWrite.rowWrite.rightRankWritebackCommitted;

    uint8_t directSlot = phase1.sampledClassifier;
    bool directSlotKnown = true;
    if (phase1.directConsumerSlotKnown) {
        directSlot = static_cast<uint8_t>(phase1.directConsumerSlot);
    }

    if (directConsumer94400) {
        out.steadySfx.callbackSiteReached =
            directSlotKnown && directSlot == 1u;
        if (directSlotKnown && directSlot <= 2u) {
            out.steadySfx.play = true;
            out.steadySfx.cueId =
                PrStage1ScorerDirectSteadyCueId::Bucket30DirectCue94400;
            out.steadySfx.tableSlot = directSlot;
        } else {
            out.steadySfx.clearMirror = true;
        }
        out.clearDecision.action =
            PrStage1ScorerDirectAcceptedClearAction::ClearBucket30Now;
        out.clearDecision.clearAdditiveBookkeepingNow = true;
        return out;
    }

    if (rowWrite.rowWrite.goodToCoolCommitted) {
        PrStage1ScorerDirectPublishBucket30PreRowWriteCue943FC(
            out,
            rowWrite.rowWrite);
        out.clearDecision.action =
            PrStage1ScorerDirectAcceptedClearAction::DeferBucket31;
        out.clearDecision.markBucket31AdditiveClearPending = true;
        return out;
    }

    if (preBucket30Ed00 != 0) {
        out.steadySfx.clearMirror = true;
        out.steadySfx.callbackSiteReached =
            directSlotKnown && directSlot == 1u;
        return out;
    }

    if (waitSecondBeatInsideBucket30) {
        out.clearDecision.action =
            PrStage1ScorerDirectAcceptedClearAction::DeferBucket30;
        return out;
    }

    if (acceptedTailSurvived) {
        PrStage1ScorerDirectPublishBucket30PreRowWriteCue943FC(
            out,
            rowWrite.rowWrite);
        out.clearDecision.action =
            PrStage1ScorerDirectAcceptedClearAction::DeferBucket31;
        return out;
    }

    if (rowWrite.rowWrite.resolutionKnown &&
        rowWrite.rowWrite.resolutionV22 < 2u &&
        !rowWrite.rowWrite.goodToCoolCommitted) {
        out.steadySfx.play = true;
        out.steadySfx.tableSlot = rowWrite.rowWrite.resolutionV22;
        out.steadySfx.cueId =
            rowWrite.rowWrite.rightRankWritebackCommitted
                ? PrStage1ScorerDirectSteadyCueId::Bucket30RowCommit
                : PrStage1ScorerDirectSteadyCueId::Bucket30Verdict;
        return out;
    }

    out.steadySfx.clearMirror = true;
    return out;
}

static PrStage1ScorerDirectBucket30ResolvedPublish
PrStage1ScorerDirectBuildBucket30ResolvedPublish(
    const PrStage1ScorerDirectBucket30RowWriteDecision& rowWrite,
    uint8_t previousRow) {
    PrStage1ScorerDirectBucket30ResolvedPublish out{};
    out.bucket30ResolutionKnown = rowWrite.resolutionKnown;
    out.bucket30ResolutionV22 = rowWrite.resolutionV22;
    out.resolvedRightRankRowKnown = rowWrite.resolvedRightRankRowKnown;
    out.resolvedRightRankRow = rowWrite.resolvedRightRankRow;
    out.rightRankWritebackCommitted = rowWrite.rightRankWritebackCommitted;
    out.rowWriteEventKnown =
        rowWrite.resolvedRightRankRowKnown &&
        rowWrite.resolvedRightRankRow != previousRow;
    out.rowWritePreviousRow = previousRow;
    return out;
}

PrStage1ScorerDirectBucket30OwnerSliceResult
PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    const PrStage1ScorerDirectDescriptorRow& row,
    const PrStage1ScorerDirectBucket30OwnerSliceInput& in) {
    PrStage1ScorerDirectBucket30OwnerSliceResult out{};
    out.descriptorFlagWord =
        static_cast<uint16_t>(globals.dword8ED08DescriptorFlags);
    out.commitSlice = PrStage1ScorerDirectRunBucket30CommitSlice(
        globals,
        row,
        ctx);
    out.busyGateActive = out.commitSlice.busyGateActive;
    out.scorerWindowOpen = out.commitSlice.scorerWindowOpen;
    out.ownerKernelOpen =
        !out.busyGateActive && (out.descriptorFlagWord & 0x0002u) != 0u;

    if (out.ownerKernelOpen) {
        out.phase1 = PrStage1ScorerDirectRunBucket30Phase1Slice(
            globals,
            row,
            ctx);
        if ((out.descriptorFlagWord & 0x0004u) != 0u &&
            globals.dword8ED00FollowUpState == 0) {
            out.resolution = PrStage1ScorerDirectResolveBucket30V22AndResetCache(
                globals,
                row,
                ctx);
        }
        const uint8_t activeRowBeforeRowWrite =
            static_cast<uint8_t>(ctx.word4ERightRankActiveRow);
        out.rowWrite = PrStage1ScorerDirectRunBucket30RowWriteSlice(
            globals,
            ctx,
            activeRowBeforeRowWrite,
            in.goodToCoolGateEnabled,
            in.tick96,
            in.goodToCoolDelayTick96,
            out.resolution);
        out.resolvedPublish =
            PrStage1ScorerDirectBuildBucket30ResolvedPublish(
                out.rowWrite.rowWrite,
                activeRowBeforeRowWrite);
    }

    PrStage1ScorerDirectBucket30DirectConsumerSliceResult directConsumer{};
    if (out.ownerKernelOpen) {
        directConsumer = PrStage1ScorerDirectRunBucket30DirectConsumerSlice103104(
            in.preBucket30Ed00,
            static_cast<uint8_t>(ctx.word4ERightRankActiveRow),
            out.ownerKernelOpen,
            (out.descriptorFlagWord & 0x0004u) != 0u,
            globals.word8ED38PhaseCounter != 0u,
            in.followUpPhaseIsNone,
            out.phase1,
            out.rowWrite);
    }
    out.steadySfx = directConsumer.steadySfx;
    out.clearDecision = directConsumer.clearDecision;
    if (out.clearDecision.action ==
        PrStage1ScorerDirectAcceptedClearAction::ClearBucket30Now) {
        out.clearSlice = PrStage1ScorerDirectRunBucket30ClearSlice(
            globals,
            out.clearDecision);
    } else {
        out.clearSlice.action = out.clearDecision.action;
        out.clearSlice.markBucket31AdditiveClearPending =
            out.clearDecision.markBucket31AdditiveClearPending;
    }

    out.delayedTail = PrStage1ScorerDirectRunBucket30DelayedTailSlice(
        globals,
        ctx,
        out.descriptorFlagWord);
    out.scoreDisplayValue =
        PrStage1ScorerDirectPublishScoreMirror80024FC0(globals, ctx);
    return out;
}

PrStage1ScorerDirectBucket31DispatcherResult
PrStage1ScorerDirectRunBucket31Dispatcher24FD0(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    const PrStage1ScorerDirectBucket31DispatcherInput& in) {
    PrStage1ScorerDirectBucket31DispatcherResult out{};
    out.pageClear = PrStage1ScorerDirectRunPageClearCore14BDC(
        globals,
        ctx.word56WritePageOrdinal1Based,
        in.runPageClear14BDC);
    out.consumer = PrStage1ScorerDirectRunBucket31ConsumerSlice(
        globals,
        ctx,
        in.preBucket31Ed00,
        in.awaitBucket31AfterGoodToCoolActive,
        in.bucket31NarrowClearPending,
        in.bucket31AdditiveClearPending);
    return out;
}

PrStage1ScorerDirectBucket0WindowSliceResult
PrStage1ScorerDirectRunBucket0WindowSlice24FD0(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectBucketContext& ctx,
    uint16_t descriptorFlagWord) {
    PrStage1ScorerDirectBucket0WindowSliceResult out{};

    const bool busyGateActive = PrStage1ScorerDirectBusyGate24BF4(globals, ctx);
    if (busyGateActive) {
        return out;
    }

    const bool shortCircuitedByBit10 =
        (descriptorFlagWord & 0x0010u) != 0u &&
        ctx.word4ERightRankActiveRow >= 2u &&
        ctx.word4ERightRankActiveRow < 4u;
    const bool consumeGateBit8 = (descriptorFlagWord & 0x0008u) != 0u;
    if (!shortCircuitedByBit10 && !consumeGateBit8) {
        return out;
    }

    if (shortCircuitedByBit10) {
        out.ctx118WritePulse = true;
        ctx.word54Permit = 1u;
        return out;
    }

    out.callWindowOpen = true;
    out.ctx118WritePulse =
        PrStage1ScorerDirectConsumeBucket0ReturnGate144B8(globals, ctx);
    if (out.ctx118WritePulse) {
        ctx.word54Permit = 1u;
    }
    return out;
}

static uint16_t PrStage1ScorerDirectMapAcceptedMaskToCompanion24B54(
    uint32_t acceptedClassMask) {
    const uint8_t a1 = static_cast<uint8_t>(acceptedClassMask & 0xFFu);
    int result = 1;
    if ((a1 & 0x10u) == 0u) {
        result = 2;
        if ((a1 & 0x20u) == 0u) {
            result = 3;
            if ((a1 & 0x40u) == 0u) {
                result = 4;
                if ((a1 & 0x80u) == 0u) {
                    result = 5;
                    if ((a1 & 0x04u) == 0u) {
                        result = 5;
                        if ((a1 & 0x01u) == 0u) {
                            result = 7;
                            if ((a1 & 0x08u) == 0u) {
                                return static_cast<uint16_t>((a1 & 0x02u) != 0u ? 7 : 0);
                            }
                        }
                    }
                }
            }
        }
    }
    return static_cast<uint16_t>(result);
}

static uint16_t PrStage1ScorerDirectRemapWriterControl18_14614(
    uint16_t writerControlSample18) {
    if (writerControlSample18 == 1u) {
        return 4u;
    }
    if (writerControlSample18 == 2u) {
        return 8u;
    }
    return writerControlSample18;
}

static uint8_t PrStage1ScorerDirectResolveClassToken20_24B54(
    uint16_t writerControlSample18) {
    return static_cast<uint8_t>(
        PrStage1ScorerDirectMapAcceptedMaskToCompanion24B54(writerControlSample18));
}

static bool PrStage1ScorerDirectResolveSelectors14614(
    const PrStage1ScorerDirectDescriptorRow& row,
    uint16_t descriptorSubstate50,
    uint8_t& outSelectorByte0,
    uint8_t& outSelectorByte1) {
    outSelectorByte0 = 0u;
    outSelectorByte1 = 0u;
    if (!row.valid) {
        return false;
    }

    if (descriptorSubstate50 != 0u) {
        outSelectorByte0 = row.byte18Substate1Selector0;
        outSelectorByte1 = row.byte19Substate1Selector1;
    } else {
        outSelectorByte0 = row.byte12DefaultSelector0;
        outSelectorByte1 = row.byte13DefaultSelector1;
    }
    return true;
}

static uint8_t PrStage1ScorerDirectResolveTimingTemplateSlot48(
    int32_t acceptedTick96,
    uint8_t halfWindow34) {
    if (acceptedTick96 < 0) {
        return 0u;
    }

    const uint32_t phase384 =
        (static_cast<uint32_t>(acceptedTick96) + static_cast<uint32_t>(halfWindow34)) % 384u;
    return static_cast<uint8_t>(phase384 / 48u);
}

static PrStage1ScorerDirectSourceGroupCursorEntry* FindOrCreateCursorEntry14614(
    PrStage1ScorerDirectAcceptedProducerOwnerState& ownerState,
    const PrStage1ScorerDirectSourceCellHeader& header) {
    if (!header.valid || header.dword00HeaderAddr == 0u || header.word08Count == 0u) {
        return nullptr;
    }

    PrStage1ScorerDirectSourceGroupCursorEntry* firstFree = nullptr;
    for (PrStage1ScorerDirectSourceGroupCursorEntry& entry : ownerState.cursorEntries) {
        if (entry.occupied) {
            if (entry.dword00HeaderAddr == header.dword00HeaderAddr) {
                entry.dword04BasePtr = header.dword04BasePtr;
                entry.word08Count = header.word08Count;
                if (entry.word08Count != 0u) {
                    entry.word0ACursor =
                        static_cast<uint16_t>(entry.word0ACursor % entry.word08Count);
                } else {
                    entry.word0ACursor = 0u;
                }
                return &entry;
            }
            continue;
        }
        if (!firstFree) {
            firstFree = &entry;
        }
    }

    if (!firstFree) {
        return nullptr;
    }

    firstFree->occupied = true;
    firstFree->dword00HeaderAddr = header.dword00HeaderAddr;
    firstFree->dword04BasePtr = header.dword04BasePtr;
    firstFree->word08Count = header.word08Count;
    firstFree->word0ACursor =
        header.word08Count != 0u
            ? static_cast<uint16_t>(header.word0ACursor % header.word08Count)
            : 0u;
    return firstFree;
}

static PrStage1ScorerDirectAcceptedProducerResolvedInput
PrStage1ScorerDirectResolveAcceptedProducerInput14614(
    const PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectAcceptedProducerOwnerState& ownerState,
    const PrStage1ScorerDirectAcceptedProducerCoreInput& in,
    const PrStage1ScorerDirectAcceptedProducerAccessors& accessors) {
    PrStage1ScorerDirectAcceptedProducerResolvedInput out{};
    if (in.busyGate24BF4Active) {
        out.resultCode = -1;
        return out;
    }
    if (!in.eventStreamFlagActive) {
        out.resultCode = -2;
        return out;
    }
    if (!in.lookaheadDescriptorRow || !in.lookaheadDescriptorRow->valid) {
        out.resultCode = -3;
        return out;
    }

    out.remappedWriterControl18 =
        PrStage1ScorerDirectRemapWriterControl18_14614(in.writerControlSample18);
    out.classToken20 =
        in.classToken20Known
            ? in.classToken20
            : PrStage1ScorerDirectResolveClassToken20_24B54(out.remappedWriterControl18);

    if (!PrStage1ScorerDirectResolveSelectors14614(
            *in.lookaheadDescriptorRow,
            in.descriptorSubstate50,
            out.selectorByte0,
            out.selectorByte1)) {
        out.resultCode = -3;
        return out;
    }
    out.selectorResolved = true;

    if (out.selectorByte0 == 0u) {
        out.resultCode = -5;
        return out;
    }

    if (!accessors.readSourceCellHeader) {
        out.resultCode = -6;
        return out;
    }

    PrStage1ScorerDirectSourceCellHeader header{};
    if (!accessors.readSourceCellHeader(
            accessors.userData,
            out.selectorByte0,
            out.classToken20,
            header) ||
        !header.valid) {
        out.resultCode = -6;
        return out;
    }

    if (out.selectorByte1 == 0u) {
        out.resultCode = -7;
        return out;
    }

    out.timingTemplateSlot48 = PrStage1ScorerDirectResolveTimingTemplateSlot48(
        in.acceptedTick96Known ? in.acceptedTick96 : 0,
        in.halfWindow34);
    if (!accessors.readTimingTemplateState ||
        !accessors.readTimingTemplateState(
            accessors.userData,
            out.selectorByte1,
            out.timingTemplateSlot48,
            out.timingTemplateState) ||
        out.timingTemplateState == 0u) {
        out.resultCode = -8;
        return out;
    }

    PrStage1ScorerDirectSourceGroupCursorEntry* entry =
        FindOrCreateCursorEntry14614(ownerState, header);
    if (!entry || entry->word08Count == 0u) {
        out.resultCode = -8;
        return out;
    }

    const bool classChanged =
        out.classToken20 != 0u &&
        ownerState.dword91800LastClassToken != out.classToken20;
    if (classChanged) {
        entry->word0ACursor = 0u;
    }

    if (!classChanged &&
        entry->word08Count >= 2u &&
        (out.remappedWriterControl18 & 0x2000u) != 0u) {
        entry->word0ACursor =
            entry->word0ACursor == 0u
                ? static_cast<uint16_t>(entry->word08Count - 1u)
                : static_cast<uint16_t>(entry->word0ACursor - 1u);
    }
    if ((out.remappedWriterControl18 & 0x8000u) != 0u) {
        entry->word0ACursor = 0u;
    }

    const uint16_t selectedCursor =
        static_cast<uint16_t>(entry->word0ACursor % entry->word08Count);
    PrStage1ScorerDirectSourceCell cell{};
    if (!accessors.readSourceCell ||
        !accessors.readSourceCell(
            accessors.userData,
            header.dword04BasePtr +
                static_cast<uint32_t>(selectedCursor) * 12u,
            cell) ||
        !cell.valid) {
        out.resultCode = -8;
        return out;
    }
    out.sourceCellCursor = selectedCursor;
    out.sourceCell = cell;

    out.phase384 = static_cast<uint16_t>(
        (static_cast<uint32_t>(in.acceptedTick96Known ? in.acceptedTick96 : 0) +
         static_cast<uint32_t>(in.halfWindow34)) %
        384u);
    out.recordSlot24 = static_cast<uint8_t>(out.phase384 / 24u);
    out.recordRemainder24 = static_cast<uint8_t>(out.phase384 % 24u);
    out.recordedSplit =
        out.recordRemainder24 <=
        static_cast<uint8_t>(2u * static_cast<uint32_t>(in.halfWindow34));

    out.packet.valid = true;
    out.packet.materializeClass2 = out.timingTemplateState == 2u;
    out.packet.dword08RouteMask = out.remappedWriterControl18;
    out.packet.dword08SourceCellPtr = cell.dword00SourceCellPtr;
    out.packet.dword08CallbackArgOpaque = cell.dword08PayloadOpaque;
    out.packet.word06RecordCompanion = cell.word06RecordCompanion;
    out.packet.dword08PayloadNonZero = cell.dword08CallbackArgPresent;

    if (entry->word08Count >= 2u) {
        entry->word0ACursor =
            static_cast<uint16_t>((selectedCursor + 1u) % entry->word08Count);
    }
    ownerState.dword91800LastClassToken = out.classToken20;

    out.writeback.busyGate24BF4Active = false;
    out.writeback.eventStreamFlagActive = true;
    out.writeback.descriptorPointerAvailable68 = true;
    out.writeback.byte12StreamIdValid = out.selectorByte0 != 0u;
    out.writeback.byte12StreamId = out.selectorByte0;
    out.writeback.byte13SourceGroupValid = out.selectorByte1 != 0u;
    out.writeback.byte13SourceGroup = out.selectorByte1;
    out.writeback.dword18AcceptedClassMask = out.remappedWriterControl18;
    out.writeback.dword10Tick96 = in.acceptedTick96Known ? in.acceptedTick96 : 0;
    out.writeback.byte34HalfWindow = in.halfWindow34;
    out.writeback.word48Phase384 = out.phase384;
    out.writeback.dword38WritePageOrdinal =
        ResolveAcceptedWriterPageOrdinal14614(globals, in);
    out.writeback.byte24RecordSlot = out.recordSlot24;
    out.writeback.byte24RecordRemainder = out.recordRemainder24;
    out.writeback.recordedSplit = out.recordedSplit;
    out.writeback.word82ReplayLogBlocked = in.replayLogBlocked82;
    out.writeback.packet = out.packet;
    out.replayAppend.blockedByWord82 = in.replayLogBlocked82;
    out.replayAppend.append =
        out.packet.materializeClass2 && !in.replayLogBlocked82;
    out.replayAppend.dwordEEF8Tick96 =
        static_cast<uint32_t>(in.acceptedTick96Known ? in.acceptedTick96 : 0);
    out.replayAppend.dwordEEFCClassMask = out.remappedWriterControl18;

    out.resultCode = out.packet.materializeClass2 ? 0 : -9;
    return out;
}

static bool PrStage1ScorerDirectApplyReplayAppend14614(
    PrStage1ScorerDirectReplayBufferState& replay,
    const PrStage1ScorerDirectReplayAppendIntent& intent) {
    if (!intent.append) {
        return false;
    }
    if (replay.dword901C0WriteCount >= kPrStage1ScorerDirectReplayBufferCapacity) {
        return false;
    }

    const size_t index = static_cast<size_t>(replay.dword901C0WriteCount);
    replay.dwordEEFCClassMask[index] = intent.dwordEEFCClassMask;
    replay.dwordEEF8Tick96[index] = intent.dwordEEF8Tick96;
    ++replay.dword901C0WriteCount;
    replay.dword901BCPublishedCount = replay.dword901C0WriteCount;
    replay.replayMirrorKnown8008EEF8 = true;
    replay.replayMirrorProducerKnown8008EEF8 = true;
    replay.replayMirrorProducerFunction = kPrStage1ScorerDirectFn80014614;
    replay.replayMirrorByteCountKnown8008EEF8 = true;
    replay.replayMirrorKnownByteCount8008EEF8 =
        replay.dword901BCPublishedCount * 2u * sizeof(uint32_t);
    return true;
}

static PrStage1ScorerDirectAcceptedProducerResult
PrStage1ScorerDirectAcceptedProducer14614(
    PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectAcceptedProducerInput& in) {
    PrStage1ScorerDirectAcceptedProducerResult out{};
    if (in.busyGate24BF4Active) {
        out.resultCode = -1;
        return out;
    }
    if (!in.eventStreamFlagActive) {
        out.resultCode = -2;
        return out;
    }
    if (!in.descriptorPointerAvailable68) {
        out.resultCode = -3;
        return out;
    }
    if (!in.byte12StreamIdValid) {
        out.resultCode = -5;
        return out;
    }
    if (!in.byte13SourceGroupValid) {
        out.resultCode = -7;
        return out;
    }
    if (!in.packet.valid) {
        out.resultCode = -8;
        return out;
    }

    out.packetConsumed = true;
    if (!in.packet.materializeClass2) {
        out.resultCode = -9;
        return out;
    }

    if (in.dword38WritePageOrdinal < 0) {
        out.resultCode = -9;
        return out;
    }
    const size_t pageIndex = ResolveRingIndex(in.dword38WritePageOrdinal);
    const size_t slotIndex =
        (std::min<size_t>)(static_cast<size_t>(in.byte24RecordSlot), kPrStage1ScorerDirectSlotsPerPage - 1u);

    if (in.recordedSplit) {
        PrStage1ScorerDirectRawRecord& record =
            globals.ringPages[pageIndex].records[slotIndex];
        record.dword00AcceptedMask = in.dword18AcceptedClassMask;
        record.word04Companion =
            PrStage1ScorerDirectMapAcceptedMaskToCompanion24B54(in.dword18AcceptedClassMask);
        record.dword08Payload = in.packet.dword08SourceCellPtr;
        record.word06Occupied = static_cast<uint16_t>(record.word06Occupied + 1u);
        ++globals.word91812RecordWriteShadowCount;
        globals.word91824RecordCompanion = in.packet.word06RecordCompanion;
        out.recordWritten = true;
        out.recordCompanionWritten = true;
    } else {
        ++globals.word91814PenaltySplitCount;
        out.penaltySideIncremented = true;
    }

    ++globals.word91810AcceptedCount;
    globals.dword91808AcceptedMask |= in.dword18AcceptedClassMask;
    out.aggregateMaskUpdated = true;
    out.resultCode = 0;
    return out;
}

static PrStage1ScorerDirectPageStorageApply14614Result
PrStage1ScorerDirectResolvePageStorageApply14614(
    const PrStage1ScorerDirectGlobals& globals,
    const PrStage1ScorerDirectAcceptedProducerResolvedInput& resolved,
    const PrStage1ScorerDirectAcceptedProducerResult& writeResult) {
    PrStage1ScorerDirectPageStorageApply14614Result out{};
    if (!resolved.recordedSplit || !writeResult.recordWritten ||
        writeResult.resultCode != 0 ||
        resolved.writeback.dword38WritePageOrdinal < 0) {
        return out;
    }

    out.recordedWriteApplied = true;
    out.writePageOrdinal = resolved.writeback.dword38WritePageOrdinal;
    out.recordSlot = resolved.writeback.byte24RecordSlot;
    out.acceptedMask = resolved.writeback.dword18AcceptedClassMask;
    const size_t pageIndex = ResolveRingIndex(out.writePageOrdinal);
    const size_t slotIndex =
        (std::min<size_t>)(static_cast<size_t>(out.recordSlot), kPrStage1ScorerDirectSlotsPerPage - 1u);
    out.pageCompanion = PrStage1ScorerDirectMapAcceptedMaskToCompanion24B54(
        resolved.writeback.dword18AcceptedClassMask);
    out.occupiedCount = globals.ringPages[pageIndex].records[slotIndex].word06Occupied;
    out.sourceCellPtr = resolved.packet.dword08SourceCellPtr;
    return out;
}

static PrStage1ScorerDirectAcceptedProducerSideEffectResult
PrStage1ScorerDirectResolveAcceptedProducerSideEffect14614(
    const PrStage1ScorerDirectAcceptedProducerResolvedInput& in) {
    PrStage1ScorerDirectAcceptedProducerSideEffectResult out{};
    if (!in.sourceCell.valid) {
        return out;
    }

    out.playSourceCellCue = true;
    out.cueProgram = in.sourceCell.byte00Program;
    out.cueNote = in.sourceCell.byte01Note;
    out.cueKey = in.sourceCell.byte02Key;
    out.cueVolume = in.sourceCell.byte03Volume;
    out.callbackArgPresent = in.packet.dword08PayloadNonZero;
    out.callbackArgOpaque = in.packet.dword08CallbackArgOpaque;
    return out;
}

PrStage1ScorerDirectAcceptedProducerRunResult
PrStage1ScorerDirectRunAcceptedProducer14614(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectAcceptedProducerOwnerState& ownerState,
    PrStage1ScorerDirectReplayBufferState& replay,
    const PrStage1ScorerDirectAcceptedProducerCoreInput& in,
    const PrStage1ScorerDirectAcceptedProducerAccessors& accessors) {
    PrStage1ScorerDirectAcceptedProducerRunResult out{};
    out.resolved = PrStage1ScorerDirectResolveAcceptedProducerInput14614(
        globals,
        ownerState,
        in,
        accessors);
    out.sideEffect =
        PrStage1ScorerDirectResolveAcceptedProducerSideEffect14614(out.resolved);
    out.resultCode = out.resolved.resultCode;
    if (out.resolved.resultCode != 0) {
        return out;
    }

    out.replayAppendRan =
        PrStage1ScorerDirectApplyReplayAppend14614(
            replay,
            out.resolved.replayAppend);
    out.writeRan = true;
    out.writeResult = PrStage1ScorerDirectAcceptedProducer14614(
        globals,
        out.resolved.writeback);
    out.resultCode = out.writeResult.resultCode;
    if (out.writeResult.resultCode != 0) {
        return out;
    }
    out.pageStorageApply = PrStage1ScorerDirectResolvePageStorageApply14614(
        globals,
        out.resolved,
        out.writeResult);
    return out;
}

PrStage1ScorerDirectAcceptedProducerRunResult
PrStage1ScorerDirectRunAcceptedProducerWithLatentPageClear14614(
    PrStage1ScorerDirectGlobals& globals,
    PrStage1ScorerDirectAcceptedProducerOwnerState& ownerState,
    PrStage1ScorerDirectReplayBufferState& replay,
    const PrStage1ScorerDirectAcceptedProducerCoreInput& in,
    const PrStage1ScorerDirectAcceptedProducerAccessors& accessors,
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear) {
    if (sameTickPageClear) {
        PrStage1ScorerDirectApplyLatentPageClearTarget14BDC(
            globals,
            *sameTickPageClear);
    }
    return PrStage1ScorerDirectRunAcceptedProducer14614(
        globals,
        ownerState,
        replay,
        in,
        accessors);
}

PrStage1ScorerDirectFinalPageMaintenance14BDC_14614Result
PrStage1ScorerDirectResolveFinalPageMaintenance14BDC_14614(
    const PrStage1ScorerDirectPageClear14BDCResult* sameTickPageClear,
    const PrStage1ScorerDirectAcceptedProducerRunResult& acceptedRun) {
    PrStage1ScorerDirectFinalPageMaintenance14BDC_14614Result out{};
    uint8_t clearTargetOrdinal1Based = 0u;
    bool clearTargetKnown = false;

    if (sameTickPageClear &&
        sameTickPageClear->clearApplied &&
        sameTickPageClear->targetKnown) {
        clearTargetKnown = true;
        clearTargetOrdinal1Based = sameTickPageClear->targetOrdinal1Based;
        out.valid = true;
        out.pageKnown = true;
        out.pageOrdinal1Based = clearTargetOrdinal1Based;
        out.clearApplied = true;
        out.clearPageKnown = true;
        out.clearPageOrdinal1Based = clearTargetOrdinal1Based;
    }

    if (acceptedRun.pageStorageApply.recordedWriteApplied &&
        acceptedRun.pageStorageApply.writePageOrdinal >= 0) {
        out.valid = true;
        out.pageKnown = true;
        out.pageOrdinal1Based =
            static_cast<uint8_t>(acceptedRun.pageStorageApply.writePageOrdinal);
        out.clearBeforeWrite =
            clearTargetKnown &&
            clearTargetOrdinal1Based == out.pageOrdinal1Based;
        out.slotWriteApplied = true;
        out.recordSlot = acceptedRun.pageStorageApply.recordSlot;
        out.rawRecord.dword00AcceptedMask = acceptedRun.pageStorageApply.acceptedMask;
        out.rawRecord.word04Companion = acceptedRun.pageStorageApply.pageCompanion;
        out.rawRecord.word06Occupied = acceptedRun.pageStorageApply.occupiedCount;
        out.rawRecord.dword08Payload = acceptedRun.pageStorageApply.sourceCellPtr;
    }

    return out;
}
