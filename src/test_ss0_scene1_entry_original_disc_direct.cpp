#include "pr/pr_scene1_entry_original_disc_direct.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

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

std::vector<uint8_t> ReadAllBytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(input),
                                std::istreambuf_iterator<char>());
}

std::filesystem::path FindProjectedComod(
    const std::filesystem::path& originalDisc) {
    std::error_code ec;
    const auto cwd = std::filesystem::current_path(ec);
    if (!ec) {
        const auto candidate = cwd.parent_path() / "S1" / "COMOD1.BIN";
        if (std::filesystem::is_regular_file(candidate, ec)) {
            return candidate;
        }
    }
    ec.clear();
    const auto candidate = originalDisc.parent_path() /
                           "parappa the rapper" / "S1" / "COMOD1.BIN";
    if (std::filesystem::is_regular_file(candidate, ec)) {
        return candidate;
    }
    return {};
}

PrScene1EntryOriginalDiscDirect::Source80015D18 MakeSource(
    const std::filesystem::path& originalDisc,
    const std::filesystem::path& dataRoot,
    const std::vector<uint8_t>& projection) {
    using namespace PrScene1EntryOriginalDiscDirect;
    Source80015D18 source{};
    source.sceneIndexKnown = true;
    source.sceneIndex = kSceneIndex80015D18;
    source.rowIndexKnown = true;
    source.rowIndex = kRowIndex80015D18;
    source.rowAddressKnown = true;
    source.rowAddress = kRowAddress80015D18;
    source.pathPtrKnown = true;
    source.pathPtr = kPathPtr80015D18;
    source.psxPathKnown = true;
    source.psxPath = kPsxPath80015D18;
    source.dataRoot = dataRoot;
    source.originalDiscPathKnown = true;
    source.originalDiscPath = originalDisc;
    source.projectedComodViewKnown = true;
    source.projectedComodViewData = projection.data();
    source.projectedComodViewSize = projection.size();
    source.projectedComodComparisonOnly = true;
    return source;
}

void TestNoSourceAndMissingDisc() {
    using namespace PrScene1EntryOriginalDiscDirect;
    const Source80015D18 empty{};
    const auto noSource = BuildTransaction80015D18(empty);
    CHECK(noSource.status == Status80015D18::SourceUnknown);
    CHECK(!IsExactAcceptedTransaction80015D18(noSource));

    const std::vector<uint8_t> oneByte{0x5Au};
    auto missing = MakeSource(
        std::filesystem::current_path() / "missing-original-disc" /
            "PaRappa the Rapper.bin",
        std::filesystem::current_path(),
        oneByte);
    const auto missingDisc = BuildTransaction80015D18(missing);
    CHECK(missingDisc.status == Status80015D18::OriginalDiscUnavailable);
    CHECK(!IsExactAcceptedTransaction80015D18(missingDisc));
}

void TestSourceMutations(
    const std::filesystem::path& originalDisc,
    const std::filesystem::path& dataRoot,
    const std::vector<uint8_t>& projection) {
    using namespace PrScene1EntryOriginalDiscDirect;
    auto source = MakeSource(originalDisc, dataRoot, projection);

    source.sceneIndex = 0u;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::RowIdentityMismatch);
    source = MakeSource(originalDisc, dataRoot, projection);
    source.rowIndex = 1u;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::RowIdentityMismatch);
    source = MakeSource(originalDisc, dataRoot, projection);
    source.rowAddress ^= 4u;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::RowIdentityMismatch);

    source = MakeSource(originalDisc, dataRoot, projection);
    source.pathPtr ^= 4u;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::PathIdentityMismatch);
    source = MakeSource(originalDisc, dataRoot, projection);
    source.psxPath = "\\S1\\COMOD0.BIN;1";
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::PathIdentityMismatch);

    source = MakeSource(originalDisc, dataRoot, projection);
    source.projectedComodSourceAuthority = true;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::SourceAuthorityRejected);
    source = MakeSource(originalDisc, dataRoot, projection);
    source.hostFilesystemAuthority = true;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::SourceAuthorityRejected);
    source = MakeSource(originalDisc, dataRoot, projection);
    source.replayAuthority = true;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::SourceAuthorityRejected);
    source = MakeSource(originalDisc, dataRoot, projection);
    source.oldS0Authority = true;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::SourceAuthorityRejected);
    source = MakeSource(originalDisc, dataRoot, projection);
    source.stage2PlusAuthority = true;
    CHECK(BuildTransaction80015D18(source).status ==
          Status80015D18::SourceAuthorityRejected);
}

void CheckExactSeamShapes(
    const PrScene1EntryOriginalDiscDirect::Transaction80015D18& tx) {
    using namespace PrScene1EntryOriginalDiscDirect;
    using Kind = PrStage1LoaderCdHal::ActionKind;
    CHECK(tx.cdSeamCount == kCdSeamCount80015D18);

    const auto& seek = tx.cdSeams[0];
    CHECK(seek.present && seek.feedback.handled && seek.feedback.success);
    CHECK(seek.feedback.kind == Kind::SeekSync800367A4);
    CHECK(seek.feedback.psxReturn == 1);
    CHECK(seek.lowerRequest.known);
    CHECK(seek.lowerRequest.callerFunction ==
          PrStage1LoaderCdHal::kFn8001A89C);
    CHECK(seek.lowerRequest.directHelperFunction ==
          PrStage1LoaderCdHal::kFn80036974);
    CHECK(seek.lowerRequest.lowerFunction ==
          PrStage1LoaderCdHal::kFn800367A4);
    CHECK(seek.lowerRequest.seekRequestKnown);
    CHECK(seek.lowerRequest.seekMsfTargetPtrKnown);
    CHECK(seek.lowerRequest.seekMsfTargetPtr == kRowAddress80015D18 + 0x10u);
    CHECK(seek.lowerRequest.seekLbaKnown);
    CHECK(seek.lowerRequest.seekLba ==
          static_cast<int32_t>(tx.lookup800381F8.matchedExtentLba));
    CHECK(seek.lowerRequest.seekMsfTargetKnown);
    CHECK(seek.lowerRequest.seekArg0 ==
          PrStage1LoaderCdHal::kSeek8001A89CSyncMode);
    CHECK(seek.lowerRequest.seekArg1 ==
          static_cast<int32_t>(kRowAddress80015D18 + 0x10u));
    CHECK(seek.lowerRequest.seekArg2 ==
          PrStage1LoaderCdHal::kSeek8001A89CSyncArg2);
    CHECK(!seek.dstPtrKnown && !seek.sectorCountKnown);

    const auto& readStart = tx.cdSeams[1];
    CHECK(readStart.present && readStart.feedback.handled &&
          readStart.feedback.success);
    CHECK(readStart.feedback.kind == Kind::ReadStart80038FC0);
    CHECK(readStart.feedback.psxReturn == 1);
    CHECK(readStart.lowerRequest.callerFunction ==
          PrStage1LoaderCdHal::kFn8001A818);
    CHECK(readStart.lowerRequest.lowerFunction ==
          PrStage1LoaderCdHal::kFn80038FC0);
    CHECK(readStart.lowerRequest.finalFunction ==
          PrStage1LoaderCdHal::kFn800390C8);
    CHECK(readStart.lowerRequest.readStartRequestKnown);
    CHECK(readStart.lowerRequest.readStartDstPtrKnown);
    CHECK(readStart.lowerRequest.readStartDstPtr == kDestination80015D18);
    CHECK(readStart.lowerRequest.readStartSectorCountKnown);
    CHECK(readStart.lowerRequest.readStartSectorCount ==
          static_cast<int32_t>(tx.sectorCount));
    CHECK(readStart.lowerRequest.readStartModeFlagKnown);
    CHECK(readStart.lowerRequest.readStartModeFlag == 0x80);
    CHECK(readStart.lowerRequest.readStartArg0 ==
          static_cast<int32_t>(tx.sectorCount));
    CHECK(readStart.lowerRequest.readStartArg1 ==
          static_cast<int32_t>(kDestination80015D18));
    CHECK(readStart.lowerRequest.readSyncRequestKnown);
    CHECK(readStart.lowerRequest.readSyncFunction ==
          PrStage1LoaderCdHal::kFn800390C8);
    CHECK(readStart.dstPtrKnown &&
          readStart.dstPtr == kDestination80015D18);
    CHECK(readStart.sectorCountKnown &&
          readStart.sectorCount == static_cast<int32_t>(tx.sectorCount));

    const auto& readSync = tx.cdSeams[2];
    CHECK(readSync.present && readSync.feedback.handled &&
          readSync.feedback.success);
    CHECK(readSync.feedback.kind == Kind::ReadSync800390C8);
    CHECK(readSync.feedback.psxReturn == 0);
    CHECK(readSync.lowerRequest.callerFunction ==
          PrStage1LoaderCdHal::kFn8001A818);
    CHECK(readSync.lowerRequest.lowerFunction ==
          PrStage1LoaderCdHal::kFn800390C8);
    CHECK(readSync.lowerRequest.finalFunction ==
          PrStage1LoaderCdHal::kFn800364F0);
    CHECK(readSync.lowerRequest.readSyncRequestKnown);
    CHECK(readSync.lowerRequest.readSyncFunction ==
          PrStage1LoaderCdHal::kFn800390C8);
    CHECK(readSync.lowerRequest.readSyncArg0 ==
          PrStage1LoaderCdHal::kRead8001A818SyncArg0);
    CHECK(readSync.lowerRequest.readSyncArg1 ==
          PrStage1LoaderCdHal::kRead8001A818SyncArg1);

    for (const auto& seam : tx.cdSeams) {
        CHECK(seam.overlayTransferAttempt.known);
        CHECK(seam.overlayTransferAttempt.sourceFunction ==
              PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper);
        CHECK(seam.overlayTransferAttempt.transferFunction ==
              PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer);
        CHECK(seam.overlayTransferAttempt.attemptIndexKnown);
        CHECK(seam.overlayTransferAttempt.attemptIndex ==
              kAttemptIndex80015D18);
        CHECK(seam.overlayTransferAttempt.rowAddrKnown);
        CHECK(seam.overlayTransferAttempt.rowAddr == kRowAddress80015D18);
        CHECK(seam.overlayTransferAttempt.dstKnown);
        CHECK(seam.overlayTransferAttempt.dst == kDestination80015D18);
        CHECK(seam.overlayTransferAttempt.sectorCountKnown);
        CHECK(seam.overlayTransferAttempt.sectorCount == tx.sectorCount);
    }
}

void CheckExactInitRow0Feedback(
    const PrScene1EntryOriginalDiscDirect::Transaction80015D18& tx) {
    using namespace PrScene1EntryOriginalDiscDirect;
    const auto& feedback = tx.rowFeedback8001A324;
    CHECK(IsExactInitRow0Feedback80015D18(feedback));
    CHECK(feedback.cdlFilePos.minute == 0x30u);
    CHECK(feedback.cdlFilePos.second == 0x58u);
    CHECK(feedback.cdlFilePos.frame == 0x29u);

    auto mutation = feedback;
    mutation.cdlFilePos.second = 0x59u;
    CHECK(IsExactInitRow0Feedback80015D18(mutation));
    mutation = feedback;
    mutation.cdlFilePos.frame = 0x74u;
    CHECK(IsExactInitRow0Feedback80015D18(mutation));
    mutation = feedback;
    mutation.cdlFilePos.second = 0x5Au;
    CHECK(!IsExactInitRow0Feedback80015D18(mutation));
    mutation = feedback;
    mutation.cdlFilePos.second = 0x60u;
    CHECK(!IsExactInitRow0Feedback80015D18(mutation));
    mutation = feedback;
    mutation.cdlFilePos.frame = 0x7Au;
    CHECK(!IsExactInitRow0Feedback80015D18(mutation));
    mutation = feedback;
    mutation.cdlFilePos.frame = 0x75u;
    CHECK(!IsExactInitRow0Feedback80015D18(mutation));
    mutation = feedback;
    mutation.cdlFilePos.minute = 0xA0u;
    CHECK(!IsExactInitRow0Feedback80015D18(mutation));
    mutation = feedback;
    mutation.cdlFilePosKnown = false;
    CHECK(!IsExactInitRow0Feedback80015D18(mutation));
}

void TestExactAndProjectionMutations(
    const std::filesystem::path& originalDisc,
    const std::filesystem::path& dataRoot,
    const std::vector<uint8_t>& projection) {
    using namespace PrScene1EntryOriginalDiscDirect;
    const auto source = MakeSource(originalDisc, dataRoot, projection);
    const auto tx = BuildTransaction80015D18(source);
    CHECK(IsExactAcceptedTransaction80015D18(tx));
    CHECK(tx.status == Status80015D18::Accepted);
    CHECK(tx.originalDiscSourceAuthority);
    CHECK(tx.projectedComodComparisonOnly);
    CHECK(!tx.projectedComodSourceAuthority);
    CHECK(!tx.hostFilesystemAuthority);
    CHECK(!tx.replayAuthority);
    CHECK(!tx.oldS0Authority);
    CHECK(!tx.stage2PlusAuthority);
    CHECK(tx.byteForByteProjectionMatch);
    CHECK(tx.read8001A818.bytes == projection);
    CHECK(tx.rowFeedback8001A324.known);
    CHECK(tx.rowFeedback8001A324.lookupRequestCdlFilePtr ==
          kRowAddress80015D18 + 0x10u);
    CHECK(tx.rowFeedback8001A324.lookupRequestPathPtr ==
          kPathPtr80015D18);
    CHECK(tx.rowFeedback8001A324.cdlFileSize == projection.size());
    CheckExactInitRow0Feedback(tx);
    CheckExactSeamShapes(tx);

    std::vector<uint8_t> changed = projection;
    changed[changed.size() / 2u] ^= 0x01u;
    auto changedSource = MakeSource(originalDisc, dataRoot, changed);
    CHECK(BuildTransaction80015D18(changedSource).status ==
          Status80015D18::ProjectionMismatch);

    std::vector<uint8_t> shortProjection = projection;
    shortProjection.pop_back();
    auto shortSource = MakeSource(originalDisc, dataRoot, shortProjection);
    CHECK(BuildTransaction80015D18(shortSource).status ==
          Status80015D18::SizeMismatch);

    auto mutation = tx;
    mutation.hostFilesystemAuthority = true;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
    mutation = tx;
    mutation.replayAuthority = true;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
    mutation = tx;
    mutation.oldS0Authority = true;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
    mutation = tx;
    mutation.stage2PlusAuthority = true;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
    mutation = tx;
    mutation.rowAddress ^= 4u;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
    mutation = tx;
    mutation.pathPtr ^= 4u;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
    mutation = tx;
    mutation.cdSeams[1].lowerRequest.readStartModeFlag ^= 1;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
    mutation = tx;
    mutation.cdSeams[2].overlayTransferAttempt.attemptIndex = 1u;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
    mutation = tx;
    mutation.projectedComodViewBytes[0] ^= 1u;
    CHECK(!IsExactAcceptedTransaction80015D18(mutation));
}

}  // namespace

int main(int argc, char** argv) {
    TestNoSourceAndMissingDisc();
    if (argc < 2) {
        std::fprintf(stderr, "original-disc argument is required\n");
        return 2;
    }

    const std::filesystem::path originalDisc(argv[1]);
    const auto projectionPath = FindProjectedComod(originalDisc);
    const auto projection = ReadAllBytes(projectionPath);
    if (projectionPath.empty() || projection.empty()) {
        std::fprintf(stderr, "projected COMOD1.BIN view was not found\n");
        return 2;
    }
    const auto dataRoot = projectionPath.parent_path().parent_path();

    TestSourceMutations(originalDisc, dataRoot, projection);
    TestExactAndProjectionMutations(originalDisc, dataRoot, projection);
    if (g_failures != 0) {
        return 1;
    }
    std::printf(
        "test_ss0_scene1_entry_original_disc_direct: ok originalDisc=1 "
        "projection=1 seams=3 rowBcd=30:58:29\n");
    return 0;
}
