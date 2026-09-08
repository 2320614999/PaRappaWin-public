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

constexpr int32_t kQ573OwnerSetTick96 = 3449;
constexpr uint16_t kQ573OwnerSetPhase384 = 377u;
constexpr uint8_t kQ573OwnerSetBucket31 = 31u;
constexpr uint16_t kQ573OwnerSetPageOrdinal1Based = 9u;
constexpr size_t kQ573OwnerSetRingIndex = 1u;

PrStage1ScorerDirectRawRecord MakeSentinelRecord(uint32_t mask,
                                                 uint32_t payload) {
    PrStage1ScorerDirectRawRecord record{};
    record.dword00AcceptedMask = mask;
    record.word04Companion = 3u;
    record.word06Occupied = 1u;
    record.dword08Payload = payload;
    return record;
}

void SeedSentinelRecords(PrStage1ScorerDirectGlobals& globals) {
    globals.ringPages[kQ573OwnerSetRingIndex].records[7] =
        MakeSentinelRecord(0x40u, 0x801CCCF4u);
    globals.ringPages[0].records[7] =
        MakeSentinelRecord(0x10u, 0x801CA000u);
}

void TestQ573Tick96ResolvesBucket31Page9() {
    CHECK(PrStage1ScorerDirectResolvePageOrdinal56_24FD0(
              kQ573OwnerSetTick96) == kQ573OwnerSetPageOrdinal1Based);
    CHECK(static_cast<uint16_t>(
              static_cast<uint32_t>(kQ573OwnerSetTick96) % 384u) ==
          kQ573OwnerSetPhase384);
    CHECK(static_cast<uint8_t>(
              (static_cast<uint32_t>(kQ573OwnerSetTick96) % 384u) / 12u) ==
          kQ573OwnerSetBucket31);
}

void TestLiteral14BDCOwnerSetSeedsAndClearsPage9() {
    PrStage1ScorerDirectGlobals globals{};
    SeedSentinelRecords(globals);

    const PrStage1ScorerDirectPageClear14BDCResult pageClear =
        PrStage1ScorerDirectRunPageClearCore14BDC(
            globals,
            kQ573OwnerSetPageOrdinal1Based,
            true);

    CHECK(pageClear.requested);
    CHECK(pageClear.targetKnown);
    CHECK(pageClear.targetOrdinal1Based == kQ573OwnerSetPageOrdinal1Based);
    CHECK(pageClear.clearApplied);
    CHECK(globals.currentWritePageOrdinalKnown);
    CHECK(globals.currentWritePageOrdinal1Based ==
          kQ573OwnerSetPageOrdinal1Based);
    CHECK(globals.pageClearPending14BDC);
    CHECK(globals.pageClearOrdinal1Based14BDC ==
          kQ573OwnerSetPageOrdinal1Based);
    CHECK(globals.ringPages[kQ573OwnerSetRingIndex]
              .records[7]
              .word06Occupied == 0u);
    CHECK(globals.ringPages[0].records[7].word06Occupied == 1u);
    CHECK(globals.ringPages[0].records[7].dword08Payload == 0x801CA000u);
}

void TestBucket31DispatcherUsesWord56ForLiteral14BDCPage9() {
    PrStage1ScorerDirectGlobals globals{};
    SeedSentinelRecords(globals);
    PrStage1ScorerDirectBucketContext ctx{};
    ctx.word56WritePageOrdinal1Based = kQ573OwnerSetPageOrdinal1Based;
    PrStage1ScorerDirectBucket31DispatcherInput in{};
    in.runPageClear14BDC = true;

    const PrStage1ScorerDirectBucket31DispatcherResult result =
        PrStage1ScorerDirectRunBucket31Dispatcher24FD0(globals, ctx, in);

    CHECK(result.pageClear.requested);
    CHECK(result.pageClear.targetKnown);
    CHECK(result.pageClear.targetOrdinal1Based ==
          kQ573OwnerSetPageOrdinal1Based);
    CHECK(result.pageClear.clearApplied);
    CHECK(globals.currentWritePageOrdinalKnown);
    CHECK(globals.currentWritePageOrdinal1Based ==
          kQ573OwnerSetPageOrdinal1Based);
    CHECK(globals.ringPages[kQ573OwnerSetRingIndex]
              .records[7]
              .word06Occupied == 0u);
    CHECK(globals.ringPages[0].records[7].word06Occupied == 1u);
}

void TestDisabled14BDCClearDoesNotPublishOwner() {
    PrStage1ScorerDirectGlobals globals{};
    SeedSentinelRecords(globals);

    const PrStage1ScorerDirectPageClear14BDCResult pageClear =
        PrStage1ScorerDirectRunPageClearCore14BDC(
            globals,
            kQ573OwnerSetPageOrdinal1Based,
            false);

    CHECK(!pageClear.requested);
    CHECK(!pageClear.targetKnown);
    CHECK(pageClear.targetOrdinal1Based == kQ573OwnerSetPageOrdinal1Based);
    CHECK(!pageClear.clearApplied);
    CHECK(!globals.currentWritePageOrdinalKnown);
    CHECK(globals.currentWritePageOrdinal1Based == 0u);
    CHECK(!globals.pageClearPending14BDC);
    CHECK(globals.pageClearOrdinal1Based14BDC == 0u);
    CHECK(globals.ringPages[kQ573OwnerSetRingIndex]
              .records[7]
              .word06Occupied == 1u);
}

} // namespace

int main() {
    TestQ573Tick96ResolvesBucket31Page9();
    TestLiteral14BDCOwnerSetSeedsAndClearsPage9();
    TestBucket31DispatcherUsesWord56ForLiteral14BDCPage9();
    TestDisabled14BDCClearDoesNotPublishOwner();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_stage1_14bdc_owner_set_cadence_contract: failed checks=%d\n",
            g_failed);
        return 1;
    }
    std::printf("test_ss0_stage1_14bdc_owner_set_cadence_contract: ok\n");
    return 0;
}
