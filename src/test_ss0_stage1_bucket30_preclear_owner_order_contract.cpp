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

PrStage1ScorerDirectDescriptorRow MakeQ501Row8Descriptor() {
    PrStage1ScorerDirectDescriptorRow row{};
    row.valid = true;
    row.defaultBranch.byte01AnchorSlotIndex = 8u;
    row.defaultBranch.byte02RequiredClassToken = 1u;
    row.defaultBranch.byte03PenaltyWeight = 1u;
    row.defaultBranch.dword08RequiredMask = 0x10u;
    row.defaultBranch.word0ELookbackPageCount = 1u;
    row.defaultBranch.word10FlagWord = 0x0002u;
    row.word10FinalResolutionThreshold = 0;
    return row;
}

PrStage1ScorerDirectBucketContext MakeQ501BucketContext() {
    PrStage1ScorerDirectBucketContext ctx{};
    ctx.dword0CTick96 = 3027u;
    ctx.word4ERightRankActiveRow = 8u;
    ctx.word50DescriptorSubstate = 0u;
    ctx.word56WritePageOrdinal1Based = 9u;
    ctx.descriptorPointerAvailable = true;
    return ctx;
}

PrStage1ScorerDirectBucket30OwnerSliceInput MakeQ501OwnerInput() {
    PrStage1ScorerDirectBucket30OwnerSliceInput in{};
    in.preBucket30Ed00 = 0;
    in.followUpPhaseIsNone = true;
    in.goodToCoolGateEnabled = false;
    in.tick96 = 3027;
    return in;
}

void SeedQ501Bucket28OwnerGlobals(PrStage1ScorerDirectGlobals& globals,
                                  uint16_t acceptedCount,
                                  uint32_t acceptedMask) {
    globals.dword8ED08DescriptorFlags = 0x0002u;
    globals.dword91808AcceptedMask = acceptedMask;
    globals.word91810AcceptedCount = acceptedCount;
    globals.word91812RecordWriteShadowCount = acceptedCount;
    globals.word91814PenaltySplitCount = acceptedCount;
    globals.word91820NoInputCounter = 0u;
    globals.word91822AdditiveTerm = 0;
}

void TestAcceptedCountIsSampledBeforeSameSliceClear() {
    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectBucketContext ctx = MakeQ501BucketContext();
    const PrStage1ScorerDirectDescriptorRow row = MakeQ501Row8Descriptor();
    const PrStage1ScorerDirectBucket30OwnerSliceInput in =
        MakeQ501OwnerInput();
    SeedQ501Bucket28OwnerGlobals(globals, 1u, 0x10u);

    const PrStage1ScorerDirectBucket30OwnerSliceResult result =
        PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
            globals,
            ctx,
            row,
            in);

    CHECK(result.commitSlice.scorerRan);
    CHECK(result.commitSlice.noInputCounterRan);
    CHECK(result.commitSlice.noInputCounterAcceptedCountInput == 1u);
    CHECK(result.commitSlice.noInputCounterInput == 0u);
    CHECK(result.commitSlice.noInputCounterOutput == 0u);
    CHECK(!result.commitSlice.noInputCounterIncremented);
    CHECK(result.clearDecision.action ==
          PrStage1ScorerDirectAcceptedClearAction::ClearBucket30Now);
    CHECK(result.clearDecision.directConsumerOwnerNoResolution94400);
    CHECK(result.clearSlice.bucketLocalClearRan);
    CHECK(globals.word91810AcceptedCount == 0u);
    CHECK(globals.word91812RecordWriteShadowCount == 0u);
    CHECK(globals.word91814PenaltySplitCount == 0u);
    CHECK(globals.dword91808AcceptedMask == 0u);
    CHECK(globals.dword9180CLastClearedAcceptedMask == 0x10u);
    CHECK(globals.word91820NoInputCounter == 0u);
}

void TestZeroCountEntryIsTheNoInputProducerShape() {
    PrStage1ScorerDirectGlobals globals{};
    PrStage1ScorerDirectBucketContext ctx = MakeQ501BucketContext();
    const PrStage1ScorerDirectDescriptorRow row = MakeQ501Row8Descriptor();
    const PrStage1ScorerDirectBucket30OwnerSliceInput in =
        MakeQ501OwnerInput();
    SeedQ501Bucket28OwnerGlobals(globals, 0u, 0u);

    const PrStage1ScorerDirectBucket30OwnerSliceResult result =
        PrStage1ScorerDirectRunBucket30OwnerSlice24FD0(
            globals,
            ctx,
            row,
            in);

    CHECK(result.commitSlice.scorerRan);
    CHECK(result.commitSlice.noInputCounterRan);
    CHECK(result.commitSlice.noInputCounterAcceptedCountInput == 0u);
    CHECK(result.commitSlice.noInputCounterInput == 0u);
    CHECK(result.commitSlice.noInputCounterOutput == 1u);
    CHECK(result.commitSlice.noInputCounterIncremented);
    CHECK(result.commitSlice.commit.commitTermKnown);
    CHECK(result.commitSlice.commit.commitTermValue == -1);
    CHECK(result.clearDecision.action ==
          PrStage1ScorerDirectAcceptedClearAction::ClearBucket30Now);
    CHECK(result.clearSlice.bucketLocalClearRan);
    CHECK(globals.word91810AcceptedCount == 0u);
    CHECK(globals.word91820NoInputCounter == 1u);
}

} // namespace

int main() {
    TestAcceptedCountIsSampledBeforeSameSliceClear();
    TestZeroCountEntryIsTheNoInputProducerShape();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_stage1_bucket30_preclear_owner_order_contract: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_stage1_bucket30_preclear_owner_order_contract: ok\n");
    return 0;
}
