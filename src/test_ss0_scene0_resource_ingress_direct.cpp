#include "pr/pr_ss0_scene0_resource_ingress_direct.h"

#include <array>
#include <cstddef>
#include <cstdio>

namespace {

int g_failures = 0;

#define CHECK(expr)                                                         \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::fprintf(stderr, "check failed: %s:%d: %s\n",             \
                         __FILE__, __LINE__, #expr);                        \
            ++g_failures;                                                   \
        }                                                                   \
    } while (false)

PrSS0Scene0ResourceIngressDirect::Source801C4780 MakeSource() {
    PrSS0Scene0ResourceIngressDirect::Source801C4780 source{};
    source.sceneIndexKnown = true;
    source.sceneIndex =
        PrSS0Scene0ResourceIngressDirect::kSceneIndex801C4780;
    source.sceneEntryBaseKnown = true;
    source.sceneEntryBase =
        PrSS0Scene0ResourceIngressDirect::kSceneEntryBase801C4780;
    source.directCompoProjectionKnown = true;
    source.directCompoProjectionReady = true;
    return source;
}

void CheckPracticeYCompoRowRejected(
    const PrSS0Scene0ResourceIngressDirect::Transaction801C4780& transaction) {
    PrMovieSegmentDirect::MovieSegmentRecord48 row{};
    row.known = true;
    row.psxAddr = 0xFFFFFFFFu;
    CHECK(!PrSS0Scene0ResourceIngressDirect::
              BuildPracticeYCompoRow80015618(transaction, row));
    CHECK(!row.known);
    CHECK(row.psxAddr == 0u);
    CHECK(!row.startMsfKnown);
    CHECK(!row.lengthSourceA1Plus20Known);
    CHECK(!row.cdlFileNameA1Plus18Known);
}

void CheckStartupCommonRowRejected(
    const PrSS0Scene0ResourceIngressDirect::Transaction801C4780& transaction) {
    PrMovieSegmentDirect::MovieSegmentRecord48 row{};
    row.known = true;
    row.psxAddr = 0xFFFFFFFFu;
    CHECK(!PrSS0Scene0ResourceIngressDirect::
              BuildStartupCommonRow80016B84(transaction, row));
    CHECK(!row.known);
    CHECK(row.psxAddr == 0u);
    CHECK(!row.startMsfKnown);
    CHECK(!row.lengthSourceA1Plus20Known);
    CHECK(!row.cdlFileNameA1Plus18Known);
}

bool IsExactCommonName(const std::array<uint8_t, 16>& name) {
    constexpr char kExpected[] = "COMMON.INT;1";
    for (std::size_t index = 0u; index < name.size(); ++index) {
        const uint8_t expected =
            index < sizeof(kExpected) - 1u
                ? static_cast<uint8_t>(kExpected[index])
                : 0u;
        if (name[index] != expected) {
            return false;
        }
    }
    return true;
}

bool IsExactYCompoName(const std::array<uint8_t, 16>& name) {
    constexpr char kExpected[] = "YCOMPO.INT;1";
    for (std::size_t index = 0u; index < name.size(); ++index) {
        const uint8_t expected =
            index < sizeof(kExpected) - 1u
                ? static_cast<uint8_t>(kExpected[index])
                : 0u;
        if (name[index] != expected) {
            return false;
        }
    }
    return true;
}

bool IsExactZCompoName(const std::array<uint8_t, 16>& name) {
    constexpr char kExpected[] = "ZCOMPO.INT;1";
    for (std::size_t index = 0u; index < name.size(); ++index) {
        const uint8_t expected =
            index < sizeof(kExpected) - 1u
                ? static_cast<uint8_t>(kExpected[index])
                : 0u;
        if (name[index] != expected) {
            return false;
        }
    }
    return true;
}

void TestFailClosedSources() {
    using namespace PrSS0Scene0ResourceIngressDirect;

    Source801C4780 source{};
    auto transaction = BuildTransaction801C4780(source);
    CHECK(transaction.status == Status801C4780::SourceUnknown);
    CHECK(!transaction.accepted);
    CheckStartupCommonRowRejected(transaction);
    CheckPracticeYCompoRowRejected(transaction);

    source = MakeSource();
    source.sceneIndex = 1u;
    transaction = BuildTransaction801C4780(source);
    CHECK(transaction.status == Status801C4780::MalformedSource);

    source = MakeSource();
    source.sceneEntryBase ^= 4u;
    transaction = BuildTransaction801C4780(source);
    CHECK(transaction.status == Status801C4780::MalformedSource);

    source = MakeSource();
    source.directCompoProjectionReady = false;
    transaction = BuildTransaction801C4780(source);
    CHECK(transaction.status ==
          Status801C4780::DirectCompoProjectionUnavailable);

    source = MakeSource();
    source.discBinPathKnown = true;
    source.discBinPath = "missing-ss0-disc-image.bin";
    transaction = BuildTransaction801C4780(source);
    CHECK(transaction.status == Status801C4780::DiscImageUnavailable);

    StartupPreloadSource80016B84 startupSource{};
    auto startup = BuildStartupPreloadTransaction80016B84(startupSource);
    CHECK(startup.status == StartupPreloadStatus80016B84::SourceUnknown);
    CHECK(!startup.accepted);
    startupSource.discBinPathKnown = true;
    startupSource.discBinPath = "missing-ss0-disc-image.bin";
    startup = BuildStartupPreloadTransaction80016B84(startupSource);
    CHECK(startup.status ==
          StartupPreloadStatus80016B84::DiscImageUnavailable);
    CHECK(!startup.accepted);
}

bool TestExactDiscTransaction(const std::filesystem::path& discBinPath) {
    using namespace PrSS0Scene0ResourceIngressDirect;

    Source801C4780 source = MakeSource();
    source.discBinPathKnown = true;
    source.discBinPath = discBinPath;
    const Transaction801C4780 transaction =
        BuildTransaction801C4780(source);
    CHECK(IsExactAcceptedTransaction801C4780(transaction));
    CHECK(transaction.status == Status801C4780::Accepted);
    CHECK(transaction.discImageDirectoryAuthority);
    CHECK(transaction.hostProjection);
    CHECK(!transaction.final8001AC18ResultKnown);
    CHECK(!transaction.final8001AC18ResultAuthority);
    CHECK(!transaction.preopenRowResultsRequiredByPsx);
    CHECK(transaction.directFailClosedAllNonNullRows);
    CHECK(transaction.scan801C4780.feedbackAppliedCount == 5u);
    CHECK(transaction.scan801C4780.rowNeedsCdLookupCount == 5u);
    CHECK(transaction.scan801C4780.rowCdLookupReadyCount == 5u);
    CHECK(transaction.scan801C4780.rowMissingCdLookupFeedbackCount == 0u);
    CHECK(transaction.scan801C4780.loadCompoArg0 ==
          kCompoRowAddress801C4780);
    CHECK(transaction.scan801C4780.loadCompoArg1 == 0);

    PrMovieSegmentDirect::MovieSegmentRecord48 commonRow{};
    const bool commonRowBuilt =
        BuildStartupCommonRow80016B84(transaction, commonRow);
    CHECK(commonRowBuilt);
    CHECK(commonRow.known);
    CHECK(commonRow.psxAddr == kStartupCommonRowAddress80016B84);
    CHECK(commonRow.tableIndex == kStartupCommonRowTableIndex80016B84);
    CHECK(commonRow.pathPtrA1Plus00Known);
    CHECK(commonRow.pathPtrA1Plus00 == kStartupCommonPathPtr80016B84);
    CHECK(commonRow.loadedStateA1Plus0CKnown);
    CHECK(commonRow.loadedStateA1Plus0C == 1);
    CHECK(commonRow.startMsfKnown);
    CHECK(commonRow.lengthSourceA1Plus20Known);
    CHECK(commonRow.lengthSourceA1Plus20 > 0u);
    CHECK(commonRow.cdlFileNameA1Plus18Known);
    CHECK(IsExactCommonName(commonRow.cdlFileNameA1Plus18));
    CHECK(commonRow.timeBaseA1Plus40Known);
    CHECK(commonRow.endA1Plus44Known);
    const auto commonLba = PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(
        commonRow.startMsf);
    CHECK(commonLba.known);
    CHECK(commonLba.lba >= 0);
    CHECK(commonRow.timeBaseA1Plus40 == commonLba.lba);

    StartupPreloadSource80016B84 startupSource{};
    startupSource.discBinPathKnown = true;
    startupSource.discBinPath = discBinPath;
    const auto startup =
        BuildStartupPreloadTransaction80016B84(startupSource);
    CHECK(IsExactAcceptedStartupPreloadTransaction80016B84(startup));
    CHECK(startup.status == StartupPreloadStatus80016B84::Accepted);
    CHECK(startup.discImageDirectoryAuthority);
    CHECK(!startup.intPayloadAuthority);
    CHECK(!startup.timVramSideEffectsCommitted);
    CHECK(!startup.vabSpuSideEffectsCommitted);
    CHECK(!startup.inputAudioTail80026FA4Committed);
    CHECK(!startup.hostProjection);
    CHECK(startup.commonRow.psxAddr ==
          kStartupCommonRowAddress80016B84);
    CHECK(startup.zCompoRow.psxAddr ==
          kStartupZCompoRowAddress80016B84);
    CHECK(startup.zCompoRow.tableIndex ==
          kStartupZCompoRowTableIndex80016B84);
    CHECK(startup.zCompoRow.pathPtrA1Plus00Known);
    CHECK(startup.zCompoRow.pathPtrA1Plus00 ==
          kStartupZCompoPathPtr80016B84);
    CHECK(startup.zCompoRow.cdlFileNameA1Plus18Known);
    CHECK(IsExactZCompoName(startup.zCompoRow.cdlFileNameA1Plus18));
    CHECK(startup.callOrder.front() == 0x80025A34u);
    CHECK(startup.callOrder[1] == 0x8001AC18u);
    CHECK(startup.callOrder[2] == 0x80027120u);
    CHECK(startup.callOrder[5] == 0x80026FA4u);
    CHECK(startup.callOrder.back() == 0x80025A34u);
    CHECK(startup.firstLogoInitCallback == 0x8001C0A0u);
    CHECK(startup.secondLogoInitCallback == 0x8001C1B8u);

    auto startupMutation = startup;
    startupMutation.callOrder[2] = 0x80026FA4u;
    CHECK(!IsExactAcceptedStartupPreloadTransaction80016B84(
        startupMutation));
    startupMutation = startup;
    startupMutation.intPayloadAuthority = true;
    CHECK(!IsExactAcceptedStartupPreloadTransaction80016B84(
        startupMutation));
    startupMutation = startup;
    startupMutation.timVramSideEffectsCommitted = true;
    CHECK(!IsExactAcceptedStartupPreloadTransaction80016B84(
        startupMutation));

    PrMovieSegmentDirect::MovieSegmentRecord48 practiceRow{};
    const bool practiceRowBuilt =
        BuildPracticeYCompoRow80015618(transaction, practiceRow);
    CHECK(practiceRowBuilt);
    CHECK(practiceRow.known);
    CHECK(practiceRow.psxAddr == kPracticeYCompoRowAddress80015618);
    CHECK(practiceRow.tableIndex ==
          kPracticeYCompoRowTableIndex80015618);
    CHECK(practiceRow.pathPtrA1Plus00Known);
    CHECK(practiceRow.pathPtrA1Plus00 == kPracticeYCompoPathPtr80015618);
    CHECK(practiceRow.loadedStateA1Plus0CKnown);
    CHECK(practiceRow.loadedStateA1Plus0C == 1);
    CHECK(practiceRow.startMsfKnown);
    CHECK(practiceRow.lengthSourceA1Plus20Known);
    CHECK(practiceRow.lengthSourceA1Plus20 > 0u);
    CHECK(practiceRow.cdlFileNameA1Plus18Known);
    CHECK(IsExactYCompoName(practiceRow.cdlFileNameA1Plus18));
    CHECK(practiceRow.timeBaseA1Plus40Known);
    CHECK(practiceRow.endA1Plus44Known);
    const auto lba = PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(
        practiceRow.startMsf);
    CHECK(lba.known);
    CHECK(lba.lba >= 0);
    CHECK(practiceRow.timeBaseA1Plus40 == lba.lba);
    const int64_t expectedEnd =
        static_cast<int64_t>(practiceRow.timeBaseA1Plus40) +
        static_cast<int64_t>(practiceRow.lengthSourceA1Plus20 >> 11u);
    CHECK(practiceRow.endA1Plus44 == expectedEnd);

    Transaction801C4780 mutation = transaction;
    mutation.final8001AC18ResultKnown = true;
    CHECK(!IsExactAcceptedTransaction801C4780(mutation));
    CheckStartupCommonRowRejected(mutation);
    CheckPracticeYCompoRowRejected(mutation);
    mutation = transaction;
    mutation.replayValueAuthority = true;
    CHECK(!IsExactAcceptedTransaction801C4780(mutation));
    CheckPracticeYCompoRowRejected(mutation);
    mutation = transaction;
    mutation.hostExtractedFileAuthority = true;
    CHECK(!IsExactAcceptedTransaction801C4780(mutation));
    CheckPracticeYCompoRowRejected(mutation);
    mutation = transaction;
    mutation.oldWinS0Authority = true;
    CHECK(!IsExactAcceptedTransaction801C4780(mutation));
    CheckPracticeYCompoRowRejected(mutation);
    mutation = transaction;
    mutation.stage2PlusAuthority = true;
    CHECK(!IsExactAcceptedTransaction801C4780(mutation));
    CheckPracticeYCompoRowRejected(mutation);
    mutation = transaction;
    mutation.scan801C4780.rowCdLookupReadyCount = 4u;
    CHECK(!IsExactAcceptedTransaction801C4780(mutation));
    CheckPracticeYCompoRowRejected(mutation);
    return commonRowBuilt && practiceRowBuilt;
}

}  // namespace

int main(int argc, char** argv) {
    TestFailClosedSources();
    bool exactDiscTested = false;
    bool standaloneRowsTested = false;
    if (argc >= 2) {
        standaloneRowsTested =
            TestExactDiscTransaction(std::filesystem::path(argv[1]));
        exactDiscTested = true;
    }
    if (g_failures != 0) {
        return 1;
    }
    std::printf(
        "test_ss0_scene0_resource_ingress_direct: ok exactDisc=%d standaloneRows=%d\n",
        exactDiscTested ? 1 : 0,
        standaloneRowsTested ? 1 : 0);
    return 0;
}
