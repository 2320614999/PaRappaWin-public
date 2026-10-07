#include "pr/pr_stage1_scorer_direct.h"

#include <cstdio>

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

PrStage1ScorerDirectDescriptorRow MakeRow3PhaseDescriptor(uint32_t requiredMask,
                                                          uint16_t flagWord) {
    PrStage1ScorerDirectDescriptorRow row{};
    row.valid = true;
    row.defaultBranch.byte01AnchorSlotIndex = 8u;
    row.defaultBranch.byte02RequiredClassToken =
        requiredMask == 0x10u ? 1u : 2u;
    row.defaultBranch.byte03PenaltyWeight = 1u;
    row.defaultBranch.dword08RequiredMask = requiredMask;
    row.defaultBranch.word0ELookbackPageCount = 1u;
    row.defaultBranch.word10FlagWord = flagWord;
    row.word10FinalResolutionThreshold = 0;
    return row;
}

PrStage1ScorerDirectBucketContext MakeRow3Ctx(uint32_t tick96) {
    PrStage1ScorerDirectBucketContext ctx{};
    ctx.dword0CTick96 = tick96;
    ctx.word4ERightRankActiveRow = 3u;
    ctx.word50DescriptorSubstate = 0u;
    ctx.word56WritePageOrdinal1Based = 12u;
    ctx.descriptorPointerAvailable = true;
    return ctx;
}

PrStage1ScorerDirectBucket30OwnerSliceInput MakeOwnerInput(uint32_t tick96) {
    PrStage1ScorerDirectBucket30OwnerSliceInput in{};
    in.preBucket30Ed00 = 0;
    in.followUpPhaseIsNone = true;
    in.goodToCoolGateEnabled = false;
    in.tick96 = static_cast<int32_t>(tick96);
    return in;
}

void SeedPositiveGrowthState(PrStage1ScorerDirectGlobals& globals,
                             int16_t preCommitAccumulator,
                             int16_t sharedBaseline) {
    globals.word91816Accumulator = preCommitAccumulator;
    globals.word91818SharedBaseline = sharedBaseline;
    globals.word9181ASnapshot = sharedBaseline;
    globals.word9181CCompareBaseline = sharedBaseline;
    globals.word91810AcceptedCount = 0u;
    globals.word91812RecordWriteShadowCount = 4u;
    globals.word91814PenaltySplitCount = 0u;
}

void TestPsxRow3FirstBeatArmsPhaseLatchWithoutV22Resolution() {
    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectBucketContext ctx = MakeRow3Ctx(4564u);
    const PrStage1ScorerDirectDescriptorRow row =
        MakeRow3PhaseDescriptor(0x10u, 0x000Au);
    const PrStage1ScorerDirectBucket30OwnerSliceInput in =
        MakeOwnerInput(4564u);

    globals.dword8ED08DescriptorFlags = 0x000A;
    SeedPositiveGrowthState(globals, 10, 0);

    const PrStage1ScorerDirectBucket30OwnerSliceResult result =
        PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
            globals,
            ctx,
            row,
            in);

    CHECK(result.ownerKernelOpen);
    CHECK(result.phase1.firstBeat);
    CHECK(result.phase1.sampledClassifier == 1u);
    CHECK(globals.word91816Accumulator == 9);
    CHECK(globals.word91818SharedBaseline == 9);
    CHECK(globals.word8ED36Phase1Cache == 1u);
    CHECK(globals.word8ED38PhaseCounter == 1u);
    CHECK(!result.resolution.resolutionCalled);
    CHECK(!result.rowWrite.rowWrite.resolutionKnown);
    CHECK(!result.rowWrite.rowWrite.rightRankWritebackCommitted);
    CHECK(ctx.word4ERightRankActiveRow == 3u);
}

void TestPsxRow3SecondBeatResolvesV22OneAndCommitsRow3ToRow2() {
    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectBucketContext ctx = MakeRow3Ctx(4947u);
    const PrStage1ScorerDirectDescriptorRow row =
        MakeRow3PhaseDescriptor(0x20u, 0x000Eu);
    const PrStage1ScorerDirectBucket30OwnerSliceInput in =
        MakeOwnerInput(4947u);

    globals.dword8ED08DescriptorFlags = 0x000E;
    globals.word8ED36Phase1Cache = 1u;
    globals.word8ED38PhaseCounter = 1u;
    SeedPositiveGrowthState(globals, 19, 9);

    const PrStage1ScorerDirectBucket30OwnerSliceResult result =
        PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
            globals,
            ctx,
            row,
            in);

    CHECK(result.ownerKernelOpen);
    CHECK(!result.phase1.firstBeat);
    CHECK(result.phase1.sampledClassifier == 1u);
    CHECK(result.resolution.resolutionCalled);
    CHECK(result.resolution.resolutionInputPhase1Classifier36 == 1u);
    CHECK(result.resolution.resolutionV22 == 1u);
    CHECK(result.rowWrite.rowWrite.resolutionKnown);
    CHECK(result.rowWrite.rowWrite.resolutionV22 == 1u);
    CHECK(result.rowWrite.rowWrite.rightRankWritebackCommitted);
    CHECK(result.resolvedPublish.rowWriteEventKnown);
    CHECK(result.resolvedPublish.rowWritePreviousRow == 3u);
    CHECK(result.resolvedPublish.resolvedRightRankRow == 2u);
    CHECK(ctx.word4ERightRankActiveRow == 2u);
    CHECK(ctx.word18ETransitionAnim == 3u);
    CHECK(globals.dword8ED00FollowUpState == 6);
    CHECK(globals.word91816Accumulator == 18);
    CHECK(globals.word91818SharedBaseline == 18);
    CHECK(globals.word8ED36Phase1Cache == 2u);
    CHECK(globals.word8ED38PhaseCounter == 0u);
}

void TestPsxGoodToBadKeepsRowWriteAndDefersClear() {
    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectBucketContext ctx = MakeRow3Ctx(4947u);
    const PrStage1ScorerDirectDescriptorRow row =
        MakeRow3PhaseDescriptor(0x20u, 0x000Eu);
    const PrStage1ScorerDirectBucket30OwnerSliceInput in =
        MakeOwnerInput(4947u);

    ctx.word4ERightRankActiveRow = 1u;
    globals.dword8ED08DescriptorFlags = 0x000E;
    globals.word8ED36Phase1Cache = 0u;
    globals.word8ED38PhaseCounter = 1u;
    SeedPositiveGrowthState(globals, 18, 18);
    globals.dword91808AcceptedMask = 0x20u;
    globals.word91810AcceptedCount = 1u;
    globals.word91812RecordWriteShadowCount = 1u;
    globals.word91814PenaltySplitCount = 1u;

    const PrStage1ScorerDirectBucket30OwnerSliceResult result =
        PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
            globals,
            ctx,
            row,
            in);

    CHECK(result.resolution.resolutionCalled);
    CHECK(result.resolution.resolutionInputPhase1Classifier36 == 0u);
    CHECK(result.resolution.resolutionV22 == 0u);
    CHECK(result.clearDecision.acceptedTailSurvived);
    CHECK(result.clearDecision.rowWriteCommitted);
    CHECK(result.clearDecision.action ==
          PrStage1ScorerDirectAcceptedClearAction::DeferBucket31);
    CHECK(!result.clearSlice.bucketLocalClearRan);
    CHECK(result.rowWrite.rowWrite.rightRankWritebackCommitted);
    CHECK(result.resolvedPublish.rowWriteEventKnown);
    CHECK(ctx.word4ERightRankActiveRow == 2u);
    CHECK(ctx.word18ETransitionAnim == 1u);
    CHECK(ctx.word74HelperCounter == 1u);
    CHECK(globals.dword8ED00FollowUpState == 6);
    CHECK(globals.word91810AcceptedCount == 1u);
    CHECK(globals.dword91808AcceptedMask == 0x20u);
    CHECK(result.steadySfx.play);
    CHECK(result.steadySfx.cueId == PrStage1ScorerDirectSteadyCueId::Bucket30Verdict);
    CHECK(result.steadySfx.tableSlot == 0u);

    // 80024FD0's third configured cadence bucket owns the cleanup and 0x200
    // animation pulse. A sampled q573 host clear is not the original rule.
    PrStage1ScorerDirectBucket31DispatcherInput completion{};
    completion.preBucket31Ed00 = globals.dword8ED00FollowUpState;
    completion.bucket31NarrowClearPending = true;
    completion.runPageClear14BDC = false;
    const auto completed = PrStage1ScorerDirectRunBucket31Dispatcher24FD0(
        globals, ctx, completion);
    CHECK(completed.consumer.consumerPackageRan);
    CHECK(completed.consumer.ctxFlag0200Pulse);
    CHECK(globals.dword8ED00FollowUpState == 0);
    CHECK(globals.word91810AcceptedCount == 0u);
    CHECK(globals.dword91808AcceptedMask == 0u);
    CHECK(globals.dword9180CLastClearedAcceptedMask == 0x20u);
    CHECK(ctx.word4ERightRankActiveRow == 2u);
}

void TestPsxRatingDirectionsAndGoodToCoolGate() {
    struct Case {
        uint8_t before, classifier, after;
        uint16_t animation;
        bool coolEnabled, committed;
    };
    const Case cases[] = {
        {1u, 0u, 2u, 1u, false, true},  // GOOD -> BAD
        {2u, 0u, 3u, 2u, false, true},  // BAD -> AWFUL
        {3u, 1u, 2u, 3u, false, true},  // AWFUL -> BAD
        {2u, 1u, 1u, 4u, false, true},  // BAD -> GOOD
        {1u, 1u, 0u, 5u, true,  true},  // GOOD -> COOL, source gate on
        {0u, 0u, 1u, 6u, true,  true},  // COOL -> GOOD
        {1u, 1u, 1u, 0u, false, false}, // source gate off: remain GOOD
        {3u, 0u, 3u, 0u, false, false}, // AWFUL cannot demote further
    };
    for (const auto& test : cases) {
        PrStage1ScorerDirectGlobals globals{};
        auto ctx = MakeRow3Ctx(4947u);
        ctx.word4ERightRankActiveRow = test.before;
        const auto row = MakeRow3PhaseDescriptor(0x20u, 0x000Eu);
        auto input = MakeOwnerInput(4947u);
        input.goodToCoolGateEnabled = test.coolEnabled;
        input.goodToCoolDelayTick96 = 96;
        globals.dword8ED08DescriptorFlags = 0x000Eu;
        globals.word8ED36Phase1Cache = test.classifier;
        globals.word8ED38PhaseCounter = 1u;
        SeedPositiveGrowthState(globals, test.classifier ? 19 : 0,
                                test.classifier ? 9 : 0);
        const auto result = PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
            globals, ctx, row, input);
        CHECK(result.rowWrite.rowWrite.rightRankWritebackCommitted == test.committed);
        CHECK(ctx.word4ERightRankActiveRow == test.after);
        CHECK(ctx.word18ETransitionAnim == test.animation);
        if (test.committed) {
            CHECK(!result.clearSlice.bucketLocalClearRan);
            CHECK(result.clearDecision.action ==
                  PrStage1ScorerDirectAcceptedClearAction::DeferBucket31);
            PrStage1ScorerDirectBucket31DispatcherInput completion{};
            completion.preBucket31Ed00 = globals.dword8ED00FollowUpState;
            completion.runPageClear14BDC = false;
            const auto completed = PrStage1ScorerDirectRunBucket31Dispatcher24FD0(
                globals, ctx, completion);
            CHECK(completed.consumer.ctxFlag0200Pulse);
            CHECK(ctx.word4ERightRankActiveRow == test.after);
            CHECK(globals.dword8ED00FollowUpState == (test.animation == 5u ? 1 : 0));
        }
    }
}

void TestPsx80024Fd0Ed20EarlyReturnGate() {
    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectBucketContext ctx = MakeRow3Ctx(4947u);
    const auto row = MakeRow3PhaseDescriptor(0x20u, 0x000Eu);
    const auto input = MakeOwnerInput(4947u);
    globals.word8008ED20 = 1u;
    globals.dword8ED08DescriptorFlags = 0x000Eu;
    globals.word8ED36Phase1Cache = 1u;
    globals.word8ED38PhaseCounter = 1u;
    SeedPositiveGrowthState(globals, 19, 9);
    const auto beforeGlobals = globals;
    const auto beforeCtx = ctx;

    const auto result = PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
        globals, ctx, row, input);

    CHECK(!result.ownerKernelOpen);
    CHECK(!result.resolution.resolutionCalled);
    CHECK(!result.clearSlice.bucketLocalClearRan);
    CHECK(globals.word8008ED20 == 1u);
    CHECK(globals.word91816Accumulator == beforeGlobals.word91816Accumulator);
    CHECK(globals.dword8ED00FollowUpState == beforeGlobals.dword8ED00FollowUpState);
    CHECK(ctx.word4ERightRankActiveRow == beforeCtx.word4ERightRankActiveRow);
    CHECK(ctx.word18ETransitionAnim == beforeCtx.word18ETransitionAnim);
}

} // namespace

int main() {
    TestPsxRow3FirstBeatArmsPhaseLatchWithoutV22Resolution();
    TestPsxRow3SecondBeatResolvesV22OneAndCommitsRow3ToRow2();
    TestPsxGoodToBadKeepsRowWriteAndDefersClear();
    TestPsxRatingDirectionsAndGoodToCoolGate();
    TestPsx80024Fd0Ed20EarlyReturnGate();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_stage1_bucket30_row3_phase_contract: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_stage1_bucket30_row3_phase_contract: ok\n");
    return 0;
}
