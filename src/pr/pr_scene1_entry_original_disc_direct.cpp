#include "pr_scene1_entry_original_disc_direct.h"

#include "pr_scene_entry_direct.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <system_error>

namespace PrScene1EntryOriginalDiscDirect {
namespace {

constexpr char kOriginalDiscFileName[] = "PaRappa the Rapper.bin";
constexpr char kCdlFileName[] = "COMOD1.BIN;1";

bool SameBcd(const PrStage1LoaderCdHal::BcdMsf80036974& left,
             const PrStage1LoaderCdHal::BcdMsf80036974& right) {
    return left.minute == right.minute && left.second == right.second &&
           left.frame == right.frame;
}

bool IsExactCdlFileName(const std::array<uint8_t, 16>& name) {
    for (std::size_t index = 0u; index < name.size(); ++index) {
        const uint8_t expected =
            index < sizeof(kCdlFileName) - 1u
                ? static_cast<uint8_t>(kCdlFileName[index])
                : 0u;
        if (name[index] != expected) {
            return false;
        }
    }
    return true;
}

bool IsValidBcdByte(uint8_t value) {
    return (value & 0x0Fu) <= 9u && ((value >> 4u) & 0x0Fu) <= 9u;
}

bool IsValidBcdMsf(
    const PrMovieSegmentDirect::MsfBcd80036A78& position) {
    // SCUS 80036A78 decodes each CdlLOC byte from its high/low BCD nibbles.
    // For example, second 58 is 0x58, so a raw comparison with decimal 60
    // rejects a valid original-disc position.
    return IsValidBcdByte(position.minute) &&
           IsValidBcdByte(position.second) &&
           IsValidBcdByte(position.frame) &&
           PrMovieSegmentDirect::DecodeBcd80036A78(position.second) < 60 &&
           PrMovieSegmentDirect::DecodeBcd80036A78(position.frame) < 75;
}

bool IsExactStaticRowAndPath() {
    const auto key =
        PrSceneEntryDirect::BuildSceneEntryKeyFromSceneIndex(
            kSceneIndex80015D18);
    const auto row = PrSceneEntryDirect::IdentifySceneEntryRow(
        key, kRowIndex80015D18);
    return key.sceneIndexKnown && key.sceneIndex == kSceneIndex80015D18 &&
           key.baseKnown && row.known && row.rowIndexKnown &&
           row.rowIndex == kRowIndex80015D18 && row.rowAddrKnown &&
           row.rowAddr == kRowAddress80015D18 &&
           row.role == PrSceneEntryDirect::SceneEntryRowRole::Comod &&
           row.raw.known && row.raw.pathPtrKnown &&
           row.raw.pathPtr == kPathPtr80015D18 &&
           row.raw.loadedState0CKnown && row.raw.loadedState0C == 0 &&
           row.path.known && row.path.pathPtrKnown &&
           row.path.pathPtr == kPathPtr80015D18 &&
           row.path.role == PrSceneEntryDirect::SceneEntryPathRole::Comod &&
           row.path.psxPath != nullptr &&
           std::strcmp(row.path.psxPath, kPsxPath80015D18) == 0;
}

bool IsExactLookup(
    const PrStage1LoaderCdHal::Iso9660LookupResult800381F8& lookup,
    const std::filesystem::path& discPath) {
    const auto expectedMsf = PrStage1LoaderCdHal::LbaToBcdMsf80036974(
        static_cast<int32_t>(lookup.matchedExtentLba));
    const auto& feedback = lookup.feedback;
    return lookup.attempted && lookup.success && lookup.binPathKnown &&
           lookup.binPath == discPath && lookup.psxPathKnown &&
           lookup.psxPath == kPsxPath80015D18 &&
           lookup.cdlFilePtr == kRowAddress80015D18 + 0x10u &&
           lookup.pathPtr == kPathPtr80015D18 && lookup.retryIndex == 0u &&
           lookup.sectorLayoutKnown &&
           lookup.sectorBytes ==
               PrStage1LoaderCdHal::kIso9660LookupSectorBytes800381F8 &&
           lookup.userDataOffset ==
               PrStage1LoaderCdHal::kIso9660LookupUserDataOffset800381F8 &&
           lookup.pvdSector ==
               PrStage1LoaderCdHal::kIso9660LookupPvdSector800381F8 &&
           lookup.pvdRootRecordOffset ==
               PrStage1LoaderCdHal::
                   kIso9660LookupPvdRootRecordOffset800381F8 &&
           lookup.pvdKnown && lookup.matchedSize != 0u &&
           lookup.matchedExtentLba <=
               static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) &&
           IsExactCdlFileName(lookup.matchedCdlFileName) &&
           feedback.result.kind ==
               PrStage1LoaderCdHal::ActionKind::Lookup800381F8 &&
           feedback.result.handled && feedback.result.success &&
           feedback.result.psxReturn == 1 && feedback.requestKnown &&
           feedback.requestCdlFilePtr == kRowAddress80015D18 + 0x10u &&
           feedback.requestPathPtrKnown &&
           feedback.requestPathPtr == kPathPtr80015D18 &&
           feedback.cdlFilePosKnown &&
           SameBcd(feedback.cdlFilePos, expectedMsf) &&
           feedback.cdlFileSizeKnown &&
           feedback.cdlFileSize == lookup.matchedSize &&
           feedback.cdlFileNameKnown &&
           feedback.cdlFileName == lookup.matchedCdlFileName;
}

bool BuildExactRowFeedback(
    const PrStage1LoaderCdHal::Iso9660LookupResult800381F8& lookup,
    PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324& out) {
    out = {};
    auto probe = PrStage1LoaderCdHal::BeginProbe8001A2B0(
        kRowAddress80015D18 + 0x10u, kPathPtr80015D18);
    PrStage1LoaderCdHal::Action action{};
    if (!PrStage1LoaderCdHal::BuildNextLookupAction8001A2B0(
            probe, action) ||
        action.kind != PrStage1LoaderCdHal::ActionKind::Lookup800381F8 ||
        action.cdlFilePtr != kRowAddress80015D18 + 0x10u ||
        action.pathPtr != kPathPtr80015D18 || action.retryIndex != 0u) {
        return false;
    }
    PrStage1LoaderCdHal::ApplyLookupFeedback8001A2B0(
        probe, lookup.feedback);
    const auto completion =
        PrStage1LoaderCdHal::BuildProbeCompletionFeedback8001A2B0(
            probe, lookup.feedback);
    if (!completion.known || !completion.complete ||
        !completion.explicitCdLookupFeedback || !completion.requestKnown ||
        completion.requestCdlFilePtr != kRowAddress80015D18 + 0x10u ||
        !completion.requestPathPtrKnown ||
        completion.requestPathPtr != kPathPtr80015D18 ||
        !completion.lookupSucceeded || completion.lookupFailed ||
        completion.resultPtr != kRowAddress80015D18 + 0x10u) {
        return false;
    }

    PrMovieSegmentDirect::CdLookupProbeCompletion8001A2B0 generic{};
    generic.known = completion.known;
    generic.complete = completion.complete;
    generic.explicitCdLookupFeedback = completion.explicitCdLookupFeedback;
    generic.requestKnown = completion.requestKnown;
    generic.requestCdlFilePtr = completion.requestCdlFilePtr;
    generic.requestPathPtrKnown = completion.requestPathPtrKnown;
    generic.requestPathPtr = completion.requestPathPtr;
    generic.lookupSucceeded = completion.lookupSucceeded;
    generic.lookupFailed = completion.lookupFailed;
    generic.resultPtr = completion.resultPtr;
    generic.cdlFilePosKnown = completion.cdlFilePosKnown;
    generic.cdlFilePos.minute = completion.cdlFilePos.minute;
    generic.cdlFilePos.second = completion.cdlFilePos.second;
    generic.cdlFilePos.frame = completion.cdlFilePos.frame;
    generic.cdlFileSizeKnown = completion.cdlFileSizeKnown;
    generic.cdlFileSize = completion.cdlFileSize;
    generic.cdlFileNameKnown = completion.cdlFileNameKnown;
    generic.cdlFileName = completion.cdlFileName;
    out = PrMovieSegmentDirect::
        BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(
            generic, true, 0);
    return out.known && out.explicitCdLookupFeedback &&
           out.lookupRequestKnown &&
           out.lookupRequestCdlFilePtr == kRowAddress80015D18 + 0x10u &&
           out.lookupRequestPathPtrKnown &&
           out.lookupRequestPathPtr == kPathPtr80015D18 &&
           out.lookupResultPtrKnown &&
           out.lookupResultPtr == kRowAddress80015D18 + 0x10u &&
           out.loadedStateA1Plus0CKnown &&
           out.loadedStateA1Plus0C == 0 && out.cdlFilePosKnown &&
           out.cdlFileSizeKnown && out.cdlFileNameKnown &&
           IsExactCdlFileName(out.cdlFileName);
}

PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution
BuildAttemptAttribution(uint32_t sectorCount) {
    PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution out{};
    out.known = true;
    out.sourceFunction =
        PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper;
    out.transferFunction =
        PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer;
    out.attemptIndexKnown = true;
    out.attemptIndex = kAttemptIndex80015D18;
    out.rowAddrKnown = true;
    out.rowAddr = kRowAddress80015D18;
    out.dstKnown = true;
    out.dst = kDestination80015D18;
    out.sectorCountKnown = true;
    out.sectorCount = sectorCount;
    return out;
}

PrStage1LoaderDirect::CdSeamResult BuildCdSeam(
    const PrStage1LoaderCdHal::Action& action,
    const PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution&
        attribution,
    bool success,
    int32_t psxReturn,
    bool exposeReadPayloadShape) {
    PrStage1LoaderDirect::CdSeamResult out{};
    out.present = true;
    out.feedback.kind = action.kind;
    out.feedback.handled = true;
    out.feedback.success = success;
    out.feedback.psxReturn = psxReturn;
    out.lowerRequest =
        PrStage1LoaderCdHal::BuildLowerActionRequestMetadata(action);
    out.overlayTransferAttempt = attribution;
    if (exposeReadPayloadShape) {
        out.dstPtrKnown = true;
        out.dstPtr = kDestination80015D18;
        out.sectorCountKnown = true;
        out.sectorCount = static_cast<int32_t>(attribution.sectorCount);
    }
    return out;
}

bool SameLowerRequest(
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& left,
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& right) {
    return left.known == right.known &&
           left.actionKind == right.actionKind &&
           left.callerFunction == right.callerFunction &&
           left.directHelperFunction == right.directHelperFunction &&
           left.lowerFunction == right.lowerFunction &&
           left.finalFunction == right.finalFunction &&
           left.seekRequestKnown == right.seekRequestKnown &&
           left.seekMsfTargetPtrKnown == right.seekMsfTargetPtrKnown &&
           left.seekMsfTargetPtr == right.seekMsfTargetPtr &&
           left.seekLbaKnown == right.seekLbaKnown &&
           left.seekLba == right.seekLba &&
           left.seekMsfTargetKnown == right.seekMsfTargetKnown &&
           SameBcd(left.seekMsfTarget, right.seekMsfTarget) &&
           left.seekArg0 == right.seekArg0 &&
           left.seekArg1 == right.seekArg1 &&
           left.seekArg2 == right.seekArg2 &&
           left.readStartRequestKnown == right.readStartRequestKnown &&
           left.readStartDstPtrKnown == right.readStartDstPtrKnown &&
           left.readStartDstPtr == right.readStartDstPtr &&
           left.readStartSectorCountKnown ==
               right.readStartSectorCountKnown &&
           left.readStartSectorCount == right.readStartSectorCount &&
           left.readStartModeFlagKnown == right.readStartModeFlagKnown &&
           left.readStartModeFlag == right.readStartModeFlag &&
           left.readStartArg0 == right.readStartArg0 &&
           left.readStartArg1 == right.readStartArg1 &&
           left.readSyncRequestKnown == right.readSyncRequestKnown &&
           left.readSyncFunction == right.readSyncFunction &&
           left.readSyncArg0 == right.readSyncArg0 &&
           left.readSyncArg1 == right.readSyncArg1;
}

bool SameAttribution(
    const PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution& left,
    const PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution& right) {
    return left.known == right.known &&
           left.sourceFunction == right.sourceFunction &&
           left.transferFunction == right.transferFunction &&
           left.attemptIndexKnown == right.attemptIndexKnown &&
           left.attemptIndex == right.attemptIndex &&
           left.rowAddrKnown == right.rowAddrKnown &&
           left.rowAddr == right.rowAddr &&
           left.dstKnown == right.dstKnown && left.dst == right.dst &&
           left.sectorCountKnown == right.sectorCountKnown &&
           left.sectorCount == right.sectorCount;
}

bool IsExactCdSeam(
    const PrStage1LoaderDirect::CdSeamResult& seam,
    const PrStage1LoaderCdHal::Action& action,
    const PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution&
        attribution,
    bool success,
    int32_t psxReturn,
    bool exposeReadPayloadShape) {
    const auto expectedRequest =
        PrStage1LoaderCdHal::BuildLowerActionRequestMetadata(action);
    return seam.present && seam.feedback.kind == action.kind &&
           seam.feedback.handled && seam.feedback.success == success &&
           seam.feedback.psxReturn == psxReturn &&
           SameLowerRequest(seam.lowerRequest, expectedRequest) &&
           SameAttribution(seam.overlayTransferAttempt, attribution) &&
           seam.dstPtrKnown == exposeReadPayloadShape &&
           (!exposeReadPayloadShape ||
            seam.dstPtr == kDestination80015D18) &&
           seam.sectorCountKnown == exposeReadPayloadShape &&
           (!exposeReadPayloadShape ||
            seam.sectorCount == static_cast<int32_t>(attribution.sectorCount)) &&
           !seam.lookup800381F8Known && !seam.probe8001A2B0Known &&
           !seam.descriptorPtrKnown && !seam.livePayloadBytesKnown &&
           seam.livePayloadData == nullptr && seam.livePayloadSize == 0u &&
           !seam.streamClock800493F4Known &&
           !seam.gapMissingStreamClock800493F4Producer;
}

bool IsExactRowFeedback(
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324& row,
    const PrStage1LoaderCdHal::Iso9660LookupResult800381F8& lookup) {
    return IsExactInitRow0Feedback80015D18(row) &&
           SameBcd({row.cdlFilePos.minute,
                    row.cdlFilePos.second,
                    row.cdlFilePos.frame},
                   lookup.feedback.cdlFilePos) &&
           row.cdlFileSize == lookup.matchedSize && row.cdlFileNameKnown &&
           row.cdlFileName == lookup.matchedCdlFileName &&
           IsExactCdlFileName(row.cdlFileName);
}

}  // namespace

bool IsExactInitRow0Feedback80015D18(
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324&
        feedback) {
    return feedback.known && feedback.explicitCdLookupFeedback &&
           feedback.lookupRequestKnown &&
           feedback.lookupRequestCdlFilePtr ==
               kRowAddress80015D18 + 0x10u &&
           feedback.lookupRequestPathPtrKnown &&
           feedback.lookupRequestPathPtr == kPathPtr80015D18 &&
           feedback.lookupResultPtrKnown &&
           feedback.lookupResultPtr == kRowAddress80015D18 + 0x10u &&
           feedback.loadedStateA1Plus0CKnown &&
           feedback.loadedStateA1Plus0C == 0 &&
           feedback.cdlFilePosKnown && IsValidBcdMsf(feedback.cdlFilePos) &&
           feedback.cdlFileSizeKnown && feedback.cdlFileSize != 0u &&
           feedback.cdlFileNameKnown &&
           IsExactCdlFileName(feedback.cdlFileName);
}

bool TryResolveOriginalDiscPath80015D18(
    const std::filesystem::path& dataRoot,
    std::filesystem::path& outPath) {
    outPath.clear();
    if (dataRoot.empty()) {
        return false;
    }

    std::error_code ec;
    std::filesystem::path current = dataRoot;
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / kOriginalDiscFileName;
        ec.clear();
        if (std::filesystem::is_regular_file(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const auto parent = current.parent_path();
        if (parent == current) {
            break;
        }
        current = parent;
    }
    return false;
}

Transaction80015D18 BuildTransaction80015D18(
    const Source80015D18& source) {
    Transaction80015D18 out{};
    if (!source.sceneIndexKnown || !source.rowIndexKnown ||
        !source.rowAddressKnown || !source.pathPtrKnown ||
        !source.psxPathKnown || source.dataRoot.empty() ||
        !source.projectedComodViewKnown ||
        source.projectedComodViewData == nullptr ||
        source.projectedComodViewSize == 0u ||
        !source.projectedComodComparisonOnly) {
        return out;
    }

    out.sceneIndexKnown = true;
    out.sceneIndex = source.sceneIndex;
    out.rowIndexKnown = true;
    out.rowIndex = source.rowIndex;
    out.rowAddressKnown = true;
    out.rowAddress = source.rowAddress;
    out.pathPtrKnown = true;
    out.pathPtr = source.pathPtr;
    out.psxPathKnown = true;
    out.psxPath = source.psxPath;
    out.projectedComodViewKnown = true;
    out.projectedComodComparisonOnly = true;

    if (source.projectedComodSourceAuthority ||
        source.hostFilesystemAuthority || source.replayAuthority ||
        source.oldS0Authority || source.stage2PlusAuthority) {
        out.status = Status80015D18::SourceAuthorityRejected;
        return out;
    }
    if (source.sceneIndex != kSceneIndex80015D18 ||
        source.rowIndex != kRowIndex80015D18 ||
        source.rowAddress != kRowAddress80015D18 ||
        !IsExactStaticRowAndPath()) {
        out.status = Status80015D18::RowIdentityMismatch;
        return out;
    }
    if (source.pathPtr != kPathPtr80015D18 ||
        source.psxPath != kPsxPath80015D18) {
        out.status = Status80015D18::PathIdentityMismatch;
        return out;
    }

    std::filesystem::path discPath;
    std::error_code ec;
    if (source.originalDiscPathKnown) {
        if (source.originalDiscPath.empty() ||
            source.originalDiscPath.filename() != kOriginalDiscFileName ||
            !std::filesystem::is_regular_file(source.originalDiscPath, ec)) {
            out.status = Status80015D18::OriginalDiscUnavailable;
            return out;
        }
        discPath = source.originalDiscPath;
    } else if (!TryResolveOriginalDiscPath80015D18(
                   source.dataRoot, discPath)) {
        out.status = Status80015D18::OriginalDiscUnavailable;
        return out;
    }
    out.originalDiscPathKnown = true;
    out.originalDiscPath = discPath;

    PrStage1LoaderCdHal::Iso9660LookupInput800381F8 lookupInput{};
    lookupInput.valid = true;
    lookupInput.binPath = discPath;
    lookupInput.psxPath = kPsxPath80015D18;
    lookupInput.cdlFilePtr = kRowAddress80015D18 + 0x10u;
    lookupInput.pathPtr = kPathPtr80015D18;
    lookupInput.retryIndex = 0u;
    out.lookup800381F8 =
        PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(
            lookupInput);
    if (!IsExactLookup(out.lookup800381F8, discPath) ||
        !BuildExactRowFeedback(out.lookup800381F8,
                               out.rowFeedback8001A324)) {
        out.status = Status80015D18::DiscLookupMismatch;
        return out;
    }

    PrStage1LoaderCdHal::Iso9660UserDataReadInput8001A818 readInput{};
    readInput.valid = true;
    readInput.binPath = discPath;
    readInput.lba =
        static_cast<int32_t>(out.lookup800381F8.matchedExtentLba);
    readInput.byteCount = out.lookup800381F8.matchedSize;
    out.read8001A818 =
        PrStage1LoaderCdHal::ReadIso9660UserDataBytes8001A818(readInput);
    if (!out.read8001A818.attempted || !out.read8001A818.success) {
        out.status = Status80015D18::DiscReadFailed;
        return out;
    }
    if (out.read8001A818.bytes.size() !=
            out.lookup800381F8.matchedSize ||
        source.projectedComodViewSize !=
            out.lookup800381F8.matchedSize) {
        out.status = Status80015D18::SizeMismatch;
        return out;
    }

    out.projectedComodViewBytes.assign(
        source.projectedComodViewData,
        source.projectedComodViewData + source.projectedComodViewSize);
    if (out.read8001A818.bytes != out.projectedComodViewBytes) {
        out.status = Status80015D18::ProjectionMismatch;
        return out;
    }
    out.byteForByteProjectionMatch = true;

    const uint64_t sectorCount64 =
        (static_cast<uint64_t>(out.lookup800381F8.matchedSize) + 2047u) >>
        11u;
    if (sectorCount64 == 0u ||
        sectorCount64 >
            static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
        out.status = Status80015D18::SizeMismatch;
        return out;
    }
    out.sectorCountKnown = true;
    out.sectorCount = static_cast<uint32_t>(sectorCount64);

    const auto attribution = BuildAttemptAttribution(out.sectorCount);
    const auto seekAction =
        PrStage1LoaderCdHal::MakeSeekSyncAction800367A4(
            kRowAddress80015D18 + 0x10u,
            static_cast<int32_t>(out.lookup800381F8.matchedExtentLba));
    const auto readStartAction =
        PrStage1LoaderCdHal::MakeReadStartAction80038FC0(
            static_cast<int32_t>(out.sectorCount),
            kDestination80015D18,
            PrStage1LoaderCdHal::BuildReadModeFlag8001A818(1));
    const auto readSyncAction =
        PrStage1LoaderCdHal::MakeReadSyncAction800390C8();
    out.cdSeams[0] =
        BuildCdSeam(seekAction, attribution, true, 1, false);
    out.cdSeams[1] =
        BuildCdSeam(readStartAction, attribution, true, 1, true);
    out.cdSeams[2] =
        BuildCdSeam(readSyncAction, attribution, true, 0, true);
    out.cdSeamCount = kCdSeamCount80015D18;

    out.status = Status80015D18::Accepted;
    out.accepted = true;
    out.complete = true;
    out.originalDiscSourceAuthority = true;
    out.projectedComodSourceAuthority = false;
    out.hostFilesystemAuthority = false;
    out.replayAuthority = false;
    out.oldS0Authority = false;
    out.stage2PlusAuthority = false;
    return out;
}

bool IsExactAcceptedTransaction80015D18(
    const Transaction80015D18& transaction) {
    if (transaction.status != Status80015D18::Accepted ||
        !transaction.accepted || !transaction.complete ||
        !transaction.sceneIndexKnown ||
        transaction.sceneIndex != kSceneIndex80015D18 ||
        !transaction.rowIndexKnown ||
        transaction.rowIndex != kRowIndex80015D18 ||
        !transaction.rowAddressKnown ||
        transaction.rowAddress != kRowAddress80015D18 ||
        !transaction.pathPtrKnown ||
        transaction.pathPtr != kPathPtr80015D18 ||
        !transaction.psxPathKnown ||
        transaction.psxPath != kPsxPath80015D18 ||
        !transaction.originalDiscPathKnown ||
        transaction.originalDiscPath.empty() ||
        transaction.originalDiscPath.filename() != kOriginalDiscFileName ||
        !transaction.originalDiscSourceAuthority ||
        !transaction.projectedComodViewKnown ||
        !transaction.projectedComodComparisonOnly ||
        transaction.projectedComodSourceAuthority ||
        !transaction.byteForByteProjectionMatch ||
        transaction.hostFilesystemAuthority || transaction.replayAuthority ||
        transaction.oldS0Authority || transaction.stage2PlusAuthority ||
        !IsExactStaticRowAndPath() ||
        !IsExactLookup(transaction.lookup800381F8,
                       transaction.originalDiscPath) ||
        !IsExactRowFeedback(transaction.rowFeedback8001A324,
                            transaction.lookup800381F8) ||
        !transaction.read8001A818.attempted ||
        !transaction.read8001A818.success ||
        !transaction.read8001A818.binPathKnown ||
        transaction.read8001A818.binPath != transaction.originalDiscPath ||
        !transaction.read8001A818.lbaKnown ||
        transaction.read8001A818.lba !=
            static_cast<int32_t>(
                transaction.lookup800381F8.matchedExtentLba) ||
        !transaction.read8001A818.byteCountKnown ||
        transaction.read8001A818.byteCount !=
            transaction.lookup800381F8.matchedSize ||
        !transaction.read8001A818.sectorLayoutKnown ||
        transaction.read8001A818.sectorBytes !=
            PrStage1LoaderCdHal::kIso9660LookupSectorBytes800381F8 ||
        transaction.read8001A818.userDataOffset !=
            PrStage1LoaderCdHal::kIso9660LookupUserDataOffset800381F8 ||
        transaction.read8001A818.bytes !=
            transaction.projectedComodViewBytes ||
        transaction.read8001A818.bytes.size() !=
            transaction.lookup800381F8.matchedSize ||
        !transaction.sectorCountKnown || transaction.sectorCount == 0u ||
        transaction.sectorCount !=
            (transaction.lookup800381F8.matchedSize + 2047u) >> 11u ||
        transaction.cdSeamCount != kCdSeamCount80015D18) {
        return false;
    }

    const auto attribution = BuildAttemptAttribution(transaction.sectorCount);
    const auto seekAction =
        PrStage1LoaderCdHal::MakeSeekSyncAction800367A4(
            kRowAddress80015D18 + 0x10u,
            static_cast<int32_t>(
                transaction.lookup800381F8.matchedExtentLba));
    const auto readStartAction =
        PrStage1LoaderCdHal::MakeReadStartAction80038FC0(
            static_cast<int32_t>(transaction.sectorCount),
            kDestination80015D18,
            PrStage1LoaderCdHal::BuildReadModeFlag8001A818(1));
    const auto readSyncAction =
        PrStage1LoaderCdHal::MakeReadSyncAction800390C8();
    return IsExactCdSeam(transaction.cdSeams[0],
                         seekAction,
                         attribution,
                         true,
                         1,
                         false) &&
           IsExactCdSeam(transaction.cdSeams[1],
                         readStartAction,
                         attribution,
                         true,
                         1,
                         true) &&
           IsExactCdSeam(transaction.cdSeams[2],
                         readSyncAction,
                         attribution,
                         true,
                         0,
                         true);
}

}  // namespace PrScene1EntryOriginalDiscDirect
