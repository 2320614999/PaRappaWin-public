#include "pr_ss0_scene0_resource_ingress_direct.h"

#include "pr_scene_entry_direct.h"
#include "pr_stage1_loader_cd_hal.h"

#include <array>
#include <cstddef>
#include <cstring>
#include <limits>
#include <system_error>

namespace PrSS0Scene0ResourceIngressDirect {
namespace {

using PrMovieSegmentDirect::CdLookupProbeCompletion8001A2B0;
using PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324;
using PrMovieSegmentDirect::SceneEntryMovieSegmentRawRow801C4780;
using PrMovieSegmentDirect::SceneEntryMovieSegmentTable801C4780;

SceneEntryMovieSegmentRawRow801C4780 ConvertStaticRow801C4780(
    const PrSceneEntryDirect::SceneEntryRawRow& source) {
    SceneEntryMovieSegmentRawRow801C4780 out{};
    out.known = source.known;
    out.pathPtrA1Plus00Known = source.pathPtrKnown;
    out.pathPtrA1Plus00 = source.pathPtr;
    out.opaqueA1Plus04Known = source.opaque04Known;
    out.opaqueA1Plus04 = source.opaque04;
    out.endBiasA1Plus08Known = source.endBias08Known;
    out.endBiasA1Plus08 = source.endBias08;
    out.loadedStateA1Plus0CKnown = source.loadedState0CKnown;
    out.loadedStateA1Plus0C = source.loadedState0C;
    return out;
}

bool BuildStaticTable801C4780(
    uint32_t sceneIndex,
    uint32_t sceneEntryBase,
    SceneEntryMovieSegmentTable801C4780& out) {
    std::array<SceneEntryMovieSegmentRawRow801C4780,
               PrMovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780>
        rows{};
    for (uint32_t index = 0u;
         index < PrMovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        const auto staticRow =
            PrSceneEntryDirect::GetSceneEntryStaticRawRow(sceneIndex, index);
        if (!staticRow.known || !staticRow.pathPtrKnown) {
            return false;
        }
        rows[index] = ConvertStaticRow801C4780(staticRow);
    }

    out = PrMovieSegmentDirect::
        MaterializeSceneEntryMovieSegmentsFromRawRows801C4780(
            sceneEntryBase,
            true,
            rows.data(),
            static_cast<uint32_t>(rows.size()));
    return out.sceneEntryBaseKnown &&
           out.sceneEntryBase == kSceneEntryBase801C4780;
}

bool BuildRowLookupFeedback801C4780(
    const std::filesystem::path& discBinPath,
    const PrMovieSegmentDirect::MovieSegmentRecord48& row,
    MovieSegmentRowInitFeedback8001A324& out) {
    out = MovieSegmentRowInitFeedback8001A324{};
    if (!row.known || !row.pathPtrA1Plus00Known ||
        row.pathPtrA1Plus00 == 0u || row.psxAddr == 0u) {
        return false;
    }

    const auto identity =
        PrSceneEntryDirect::IdentifySceneEntryPathPtr(row.pathPtrA1Plus00);
    if (!identity.known || identity.psxPath == nullptr) {
        return false;
    }

    PrStage1LoaderCdHal::Probe8001A2B0State probe =
        PrStage1LoaderCdHal::BeginProbe8001A2B0(
            row.psxAddr + 0x10u,
            row.pathPtrA1Plus00);
    PrStage1LoaderCdHal::LookupFeedback800381F8 lastLookup{};
    for (;;) {
        PrStage1LoaderCdHal::Action action{};
        if (!PrStage1LoaderCdHal::BuildNextLookupAction8001A2B0(
                probe, action)) {
            break;
        }

        PrStage1LoaderCdHal::Iso9660LookupInput800381F8 input{};
        input.valid = true;
        input.binPath = discBinPath;
        input.psxPath = identity.psxPath;
        input.cdlFilePtr = action.cdlFilePtr;
        input.pathPtr = action.pathPtr;
        input.retryIndex = action.retryIndex;
        const auto lookup =
            PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(input);
        lastLookup = lookup.feedback;
        PrStage1LoaderCdHal::ApplyLookupFeedback8001A2B0(
            probe, lastLookup);
    }

    const auto completion =
        PrStage1LoaderCdHal::BuildProbeCompletionFeedback8001A2B0(
            probe, lastLookup);
    if (!completion.known || !completion.complete ||
        !completion.explicitCdLookupFeedback ||
        !completion.lookupSucceeded || completion.resultPtr == 0u) {
        return false;
    }

    CdLookupProbeCompletion8001A2B0 generic{};
    generic.known = completion.known;
    generic.complete = completion.complete;
    generic.explicitCdLookupFeedback =
        completion.explicitCdLookupFeedback;
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
            generic,
            row.loadedStateA1Plus0CKnown,
            row.loadedStateA1Plus0C);
    return out.known && out.explicitCdLookupFeedback &&
           out.lookupRequestKnown && out.lookupRequestPathPtrKnown &&
           out.lookupResultPtrKnown && out.cdlFilePosKnown &&
           out.cdlFileSizeKnown && out.cdlFileNameKnown;
}

bool IsExactPracticeYCompoName80015618(
    const std::array<uint8_t, 16>& name) {
    constexpr char kExpectedName[] = "YCOMPO.INT;1";
    for (std::size_t index = 0u; index < name.size(); ++index) {
        const uint8_t expected =
            index < sizeof(kExpectedName) - 1u
                ? static_cast<uint8_t>(kExpectedName[index])
                : 0u;
        if (name[index] != expected) {
            return false;
        }
    }
    return true;
}

bool IsExactStartupCommonName80016B84(
    const std::array<uint8_t, 16>& name) {
    constexpr char kExpectedName[] = "COMMON.INT;1";
    for (std::size_t index = 0u; index < name.size(); ++index) {
        const uint8_t expected =
            index < sizeof(kExpectedName) - 1u
                ? static_cast<uint8_t>(kExpectedName[index])
                : 0u;
        if (name[index] != expected) {
            return false;
        }
    }
    return true;
}

bool IsBcdByte80015618(uint8_t value) {
    return (value >> 4u) <= 9u && (value & 0x0Fu) <= 9u;
}

bool TryGetStartupCommonLba80016B84(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row,
    int32_t& outLba) {
    outLba = 0;
    if (!row.known || row.psxAddr != kStartupCommonRowAddress80016B84 ||
        row.tableIndex != kStartupCommonRowTableIndex80016B84 ||
        !row.pathPtrA1Plus00Known ||
        row.pathPtrA1Plus00 != kStartupCommonPathPtr80016B84 ||
        !row.startMsfKnown || !IsBcdByte80015618(row.startMsf.minute) ||
        !IsBcdByte80015618(row.startMsf.second) ||
        !IsBcdByte80015618(row.startMsf.frame) ||
        PrMovieSegmentDirect::DecodeBcd80036A78(row.startMsf.second) >= 60 ||
        PrMovieSegmentDirect::DecodeBcd80036A78(row.startMsf.frame) >= 75 ||
        !row.lengthSourceA1Plus20Known ||
        row.lengthSourceA1Plus20 == 0u ||
        !row.cdlFileNameA1Plus18Known ||
        !IsExactStartupCommonName80016B84(row.cdlFileNameA1Plus18)) {
        return false;
    }
    const auto lba =
        PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(row.startMsf);
    if (!lba.known || lba.lba < 0) {
        return false;
    }
    outLba = lba.lba;
    return true;
}

bool IsExactStartupCommonLookupRow80016B84(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row) {
    int32_t lba = 0;
    return TryGetStartupCommonLba80016B84(row, lba) &&
           row.loadedStateA1Plus0CKnown &&
           row.loadedStateA1Plus0C == 0 &&
           !row.timeBaseA1Plus40Known && !row.endA1Plus44Known;
}

bool IsExactStartupCommonRow80016B84(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row) {
    int32_t lba = 0;
    if (!TryGetStartupCommonLba80016B84(row, lba) ||
        !row.loadedStateA1Plus0CKnown || row.loadedStateA1Plus0C != 1 ||
        !row.timeBaseA1Plus40Known || row.timeBaseA1Plus40 != lba ||
        !row.endA1Plus44Known) {
        return false;
    }
    const int64_t expectedEnd =
        static_cast<int64_t>(lba) +
        static_cast<int64_t>(row.lengthSourceA1Plus20 >> 11u);
    return expectedEnd <= std::numeric_limits<int32_t>::max() &&
           row.endA1Plus44 == static_cast<int32_t>(expectedEnd);
}

bool IsExactStartupZCompoName80016B84(
    const std::array<uint8_t, 16>& name) {
    constexpr char kExpectedName[] = "ZCOMPO.INT;1";
    for (std::size_t index = 0u; index < name.size(); ++index) {
        const uint8_t expected =
            index < sizeof(kExpectedName) - 1u
                ? static_cast<uint8_t>(kExpectedName[index])
                : 0u;
        if (name[index] != expected) {
            return false;
        }
    }
    return true;
}

bool TryGetStartupZCompoLba80016B84(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row,
    int32_t& outLba) {
    outLba = 0;
    if (!row.known ||
        row.psxAddr != kStartupZCompoRowAddress80016B84 ||
        row.tableIndex != kStartupZCompoRowTableIndex80016B84 ||
        !row.pathPtrA1Plus00Known ||
        row.pathPtrA1Plus00 != kStartupZCompoPathPtr80016B84 ||
        !row.startMsfKnown || !IsBcdByte80015618(row.startMsf.minute) ||
        !IsBcdByte80015618(row.startMsf.second) ||
        !IsBcdByte80015618(row.startMsf.frame) ||
        PrMovieSegmentDirect::DecodeBcd80036A78(row.startMsf.second) >= 60 ||
        PrMovieSegmentDirect::DecodeBcd80036A78(row.startMsf.frame) >= 75 ||
        !row.lengthSourceA1Plus20Known ||
        row.lengthSourceA1Plus20 == 0u ||
        !row.cdlFileNameA1Plus18Known ||
        !IsExactStartupZCompoName80016B84(row.cdlFileNameA1Plus18)) {
        return false;
    }
    const auto lba =
        PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(row.startMsf);
    if (!lba.known || lba.lba < 0) {
        return false;
    }
    outLba = lba.lba;
    return true;
}

bool IsExactStartupZCompoLookupRow80016B84(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row) {
    int32_t lba = 0;
    return TryGetStartupZCompoLba80016B84(row, lba) &&
           row.loadedStateA1Plus0CKnown && row.loadedStateA1Plus0C == 0 &&
           !row.timeBaseA1Plus40Known && !row.endA1Plus44Known;
}

bool IsExactStartupZCompoRow80016B84(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row) {
    int32_t lba = 0;
    if (!TryGetStartupZCompoLba80016B84(row, lba) ||
        !row.loadedStateA1Plus0CKnown || row.loadedStateA1Plus0C != 1 ||
        !row.timeBaseA1Plus40Known || row.timeBaseA1Plus40 != lba ||
        !row.endA1Plus44Known) {
        return false;
    }
    const int64_t expectedEnd =
        static_cast<int64_t>(lba) +
        static_cast<int64_t>(row.lengthSourceA1Plus20 >> 11u);
    return expectedEnd <= std::numeric_limits<int32_t>::max() &&
           row.endA1Plus44 == static_cast<int32_t>(expectedEnd);
}

bool BuildStartupCommonRowFromDisc80016B84(
    const std::filesystem::path& discBinPath,
    PrMovieSegmentDirect::MovieSegmentRecord48& out) {
    out = PrMovieSegmentDirect::MovieSegmentRecord48{};
    const auto identity = PrSceneEntryDirect::IdentifySceneEntryPathPtr(
        kStartupCommonPathPtr80016B84);
    constexpr char kExpectedPsxPath[] = "\\S0\\COMMON.INT;1";
    if (!identity.known || !identity.pathPtrKnown ||
        identity.pathPtr != kStartupCommonPathPtr80016B84 ||
        identity.role != PrSceneEntryDirect::SceneEntryPathRole::Common ||
        identity.psxPath == nullptr ||
        std::strcmp(identity.psxPath, kExpectedPsxPath) != 0) {
        return false;
    }

    PrMovieSegmentDirect::MovieSegmentRecord48 lookupRow{};
    lookupRow.known = true;
    lookupRow.psxAddr = kStartupCommonRowAddress80016B84;
    lookupRow.tableIndex = kStartupCommonRowTableIndex80016B84;
    lookupRow.pathPtrA1Plus00Known = true;
    lookupRow.pathPtrA1Plus00 = kStartupCommonPathPtr80016B84;
    lookupRow.loadedStateA1Plus0CKnown = true;
    lookupRow.loadedStateA1Plus0C = 0;

    MovieSegmentRowInitFeedback8001A324 feedback{};
    if (!BuildRowLookupFeedback801C4780(
            discBinPath, lookupRow, feedback) ||
        !feedback.loadedStateA1Plus0CKnown ||
        feedback.loadedStateA1Plus0C != 0 ||
        feedback.lookupRequestCdlFilePtr !=
            kStartupCommonRowAddress80016B84 + 0x10u ||
        feedback.lookupRequestPathPtr != kStartupCommonPathPtr80016B84 ||
        feedback.lookupResultPtr !=
            kStartupCommonRowAddress80016B84 + 0x10u) {
        return false;
    }

    const auto candidate =
        PrMovieSegmentDirect::ApplyMovieSegmentRowFeedback8001A324(
            lookupRow, feedback);
    if (!IsExactStartupCommonLookupRow80016B84(candidate)) {
        return false;
    }
    const auto init =
        PrMovieSegmentDirect::PsxCall8001A324_InitSegmentRecord(candidate);
    if (init.result != 0 || init.skippedPathPtrZero ||
        init.skippedAlreadyLoaded || !init.cdLookupSucceeded ||
        !init.lookupRequestKnown ||
        init.lookupRequestCdlFilePtr !=
            kStartupCommonRowAddress80016B84 + 0x10u ||
        !init.lookupRequestPathPtrKnown ||
        init.lookupRequestPathPtr != kStartupCommonPathPtr80016B84 ||
        !init.loadedStateWrittenA1Plus0C ||
        !init.timeBaseWrittenA1Plus40 || !init.endWrittenA1Plus44 ||
        !IsExactStartupCommonRow80016B84(init.record)) {
        return false;
    }
    out = init.record;
    return true;
}

bool BuildStartupZCompoRowFromDisc80016B84(
    const std::filesystem::path& discBinPath,
    PrMovieSegmentDirect::MovieSegmentRecord48& out) {
    out = PrMovieSegmentDirect::MovieSegmentRecord48{};
    const auto identity = PrSceneEntryDirect::IdentifySceneEntryPathPtr(
        kStartupZCompoPathPtr80016B84);
    constexpr char kExpectedPsxPath[] = "\\S0\\ZCOMPO.INT;1";
    if (!identity.known || !identity.pathPtrKnown ||
        identity.pathPtr != kStartupZCompoPathPtr80016B84 ||
        identity.role != PrSceneEntryDirect::SceneEntryPathRole::ZCompo ||
        identity.psxPath == nullptr ||
        std::strcmp(identity.psxPath, kExpectedPsxPath) != 0) {
        return false;
    }

    SceneEntryMovieSegmentTable801C4780 table{};
    if (!BuildStaticTable801C4780(
            kSceneIndex801C4780, kSceneEntryBase801C4780, table)) {
        return false;
    }
    const auto& lookupRow =
        table.rows[kStartupZCompoRowTableIndex80016B84];
    if (!lookupRow.known ||
        lookupRow.psxAddr != kStartupZCompoRowAddress80016B84 ||
        !lookupRow.pathPtrA1Plus00Known ||
        lookupRow.pathPtrA1Plus00 != kStartupZCompoPathPtr80016B84 ||
        !lookupRow.loadedStateA1Plus0CKnown ||
        lookupRow.loadedStateA1Plus0C != 0) {
        return false;
    }

    MovieSegmentRowInitFeedback8001A324 feedback{};
    if (!BuildRowLookupFeedback801C4780(
            discBinPath, lookupRow, feedback) ||
        !feedback.loadedStateA1Plus0CKnown ||
        feedback.loadedStateA1Plus0C != 0 ||
        feedback.lookupRequestCdlFilePtr !=
            kStartupZCompoRowAddress80016B84 + 0x10u ||
        feedback.lookupRequestPathPtr != kStartupZCompoPathPtr80016B84 ||
        feedback.lookupResultPtr !=
            kStartupZCompoRowAddress80016B84 + 0x10u) {
        return false;
    }
    const auto candidate =
        PrMovieSegmentDirect::ApplyMovieSegmentRowFeedback8001A324(
            lookupRow, feedback);
    if (!IsExactStartupZCompoLookupRow80016B84(candidate)) {
        return false;
    }
    const auto init =
        PrMovieSegmentDirect::PsxCall8001A324_InitSegmentRecord(candidate);
    if (init.result != 0 || init.skippedPathPtrZero ||
        init.skippedAlreadyLoaded || !init.cdLookupSucceeded ||
        !init.lookupRequestKnown ||
        init.lookupRequestCdlFilePtr !=
            kStartupZCompoRowAddress80016B84 + 0x10u ||
        !init.lookupRequestPathPtrKnown ||
        init.lookupRequestPathPtr != kStartupZCompoPathPtr80016B84 ||
        !init.loadedStateWrittenA1Plus0C ||
        !init.timeBaseWrittenA1Plus40 || !init.endWrittenA1Plus44 ||
        !IsExactStartupZCompoRow80016B84(init.record)) {
        return false;
    }
    out = init.record;
    return true;
}

bool TryGetPracticeYCompoLba80015618(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row,
    int32_t& outLba) {
    outLba = 0;
    if (!row.known || row.psxAddr != kPracticeYCompoRowAddress80015618 ||
        row.tableIndex != kPracticeYCompoRowTableIndex80015618 ||
        !row.pathPtrA1Plus00Known ||
        row.pathPtrA1Plus00 != kPracticeYCompoPathPtr80015618 ||
        !row.startMsfKnown || !IsBcdByte80015618(row.startMsf.minute) ||
        !IsBcdByte80015618(row.startMsf.second) ||
        !IsBcdByte80015618(row.startMsf.frame) ||
        PrMovieSegmentDirect::DecodeBcd80036A78(row.startMsf.second) >= 60 ||
        PrMovieSegmentDirect::DecodeBcd80036A78(row.startMsf.frame) >= 75 ||
        !row.lengthSourceA1Plus20Known ||
        row.lengthSourceA1Plus20 == 0u ||
        !row.cdlFileNameA1Plus18Known ||
        !IsExactPracticeYCompoName80015618(row.cdlFileNameA1Plus18)) {
        return false;
    }
    const auto lba =
        PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(row.startMsf);
    if (!lba.known || lba.lba < 0) {
        return false;
    }
    outLba = lba.lba;
    return true;
}

bool IsExactPracticeYCompoLookupRow80015618(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row) {
    int32_t lba = 0;
    return TryGetPracticeYCompoLba80015618(row, lba) &&
           row.loadedStateA1Plus0CKnown &&
           row.loadedStateA1Plus0C == 0 &&
           !row.timeBaseA1Plus40Known && !row.endA1Plus44Known;
}

bool IsExactPracticeYCompoRow80015618(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row) {
    int32_t lba = 0;
    if (!TryGetPracticeYCompoLba80015618(row, lba) ||
        !row.loadedStateA1Plus0CKnown || row.loadedStateA1Plus0C != 1 ||
        !row.timeBaseA1Plus40Known || row.timeBaseA1Plus40 != lba ||
        !row.endA1Plus44Known) {
        return false;
    }
    const int64_t expectedEnd =
        static_cast<int64_t>(lba) +
        static_cast<int64_t>(row.lengthSourceA1Plus20 >> 11u);
    return expectedEnd <= std::numeric_limits<int32_t>::max() &&
           row.endA1Plus44 == static_cast<int32_t>(expectedEnd);
}

bool IsExactScan801C4780(
    const PrMovieSegmentDirect::MovieSegmentScanResult801C4780& scan) {
    if (!scan.called || !scan.table.sceneEntryBaseKnown ||
        scan.table.sceneEntryBase != kSceneEntryBase801C4780 ||
        scan.feedbackCount !=
            PrMovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780 ||
        scan.feedbackAppliedCount != kExpectedLookupRowCount801C4780 ||
        scan.rowNeedsCdLookupCount != kExpectedLookupRowCount801C4780 ||
        scan.rowCdLookupRequestKnownCount != kExpectedLookupRowCount801C4780 ||
        scan.rowCdLookupRequestPathPtrKnownCount !=
            kExpectedLookupRowCount801C4780 ||
        scan.rowCdLookupResultPtrKnownCount !=
            kExpectedLookupRowCount801C4780 ||
        scan.rowCdLookupReadyCount != kExpectedLookupRowCount801C4780 ||
        scan.rowCdlFileNameReadyCount != kExpectedLookupRowCount801C4780 ||
        scan.rowMissingCdLookupFeedbackCount != 0u ||
        scan.rowMissingCdlFileNameFeedbackCount != 0u ||
        scan.rowMissingCdLookupRequestCount != 0u ||
        scan.gapMissingCdLookupFeedback ||
        scan.gapMissingCdlFileNameFeedback ||
        scan.gapMissingCdLookupRequest || !scan.reset80025A00Action ||
        !scan.loadCompo8001AC18Action ||
        scan.loadCompoArg0 != kCompoRowAddress801C4780 ||
        scan.loadCompoArg1 != 0) {
        return false;
    }

    uint32_t disabledCount = 0u;
    for (uint32_t index = 0u;
         index < PrMovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        const auto& row = scan.table.rows[index];
        const auto& init = scan.rowInit[index];
        if (!row.pathPtrA1Plus00Known) {
            return false;
        }
        if (row.pathPtrA1Plus00 == 0u) {
            ++disabledCount;
            if (!init.skippedPathPtrZero || init.result != 0 ||
                scan.rowNeedsCdLookupMask[index] ||
                scan.rowCdLookupReadyMask[index]) {
                return false;
            }
            continue;
        }
        if (!scan.rowNeedsCdLookupMask[index] ||
            !scan.rowCdLookupReadyMask[index] ||
            !scan.rowCdlFileNameReadyMask[index] ||
            !init.cdLookupSucceeded || init.result != 0 ||
            !init.loadedStateWrittenA1Plus0C ||
            !init.timeBaseWrittenA1Plus40 || !init.endWrittenA1Plus44) {
            return false;
        }
    }
    return disabledCount == kExpectedDisabledRowCount801C4780;
}

}  // namespace

bool TryResolveDiscBinPath801C4780(
    const std::filesystem::path& dataRoot,
    std::filesystem::path& outPath) {
    outPath.clear();
    std::error_code ec;
    std::filesystem::path current = dataRoot;
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / "PaRappa the Rapper.bin";
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

    ec.clear();
    current = std::filesystem::current_path(ec);
    if (ec) {
        return false;
    }
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / "PaRappa the Rapper.bin";
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

StartupPreloadTransaction80016B84
BuildStartupPreloadTransaction80016B84(
    const StartupPreloadSource80016B84& source) {
    StartupPreloadTransaction80016B84 transaction{};
    if (!source.dataRootKnown && !source.discBinPathKnown) {
        return transaction;
    }

    std::filesystem::path discBinPath;
    std::error_code ec;
    if (source.discBinPathKnown) {
        if (source.discBinPath.empty() ||
            !std::filesystem::is_regular_file(source.discBinPath, ec)) {
            transaction.status =
                StartupPreloadStatus80016B84::DiscImageUnavailable;
            return transaction;
        }
        discBinPath = source.discBinPath;
    } else if (!TryResolveDiscBinPath801C4780(
                   source.dataRoot, discBinPath)) {
        transaction.status =
            StartupPreloadStatus80016B84::DiscImageUnavailable;
        return transaction;
    }
    transaction.discBinPathKnown = true;
    transaction.discBinPath = discBinPath;

    SceneEntryMovieSegmentTable801C4780 table{};
    if (!BuildStaticTable801C4780(
            kSceneIndex801C4780, kSceneEntryBase801C4780, table)) {
        transaction.status =
            StartupPreloadStatus80016B84::StaticRowsUnavailable;
        return transaction;
    }
    if (!BuildStartupCommonRowFromDisc80016B84(
            discBinPath, transaction.commonRow)) {
        transaction.status =
            StartupPreloadStatus80016B84::CommonLookupUnavailable;
        return transaction;
    }
    if (!BuildStartupZCompoRowFromDisc80016B84(
            discBinPath, transaction.zCompoRow)) {
        transaction.status =
            StartupPreloadStatus80016B84::ZCompoLookupUnavailable;
        return transaction;
    }

    transaction.callOrder = {
        0x80025A34u,
        0x8001AC18u,
        0x80027120u,
        0x80025A34u,
        0x8001AC18u,
        0x80026FA4u,
        0x80015A4Cu,
        0x80015B00u,
        0x80015C20u,
        0x80015A4Cu,
        0x80015B00u,
        0x80015C20u,
        0x80025A34u,
    };
    transaction.firstLogoInitCallback = 0x8001C0A0u;
    transaction.secondLogoInitCallback = 0x8001C1B8u;
    transaction.finalResetFunction = 0x80025A34u;
    transaction.status = StartupPreloadStatus80016B84::Accepted;
    transaction.accepted = true;
    transaction.complete = true;
    transaction.exactTopLevelCallOrder = true;
    transaction.discImageDirectoryAuthority = true;
    transaction.intPayloadAuthority = false;
    transaction.timVramSideEffectsCommitted = false;
    transaction.vabSpuSideEffectsCommitted = false;
    transaction.inputAudioTail80026FA4Committed = false;
    transaction.hostProjection = false;
    transaction.psxMemoryBackingAuthority = false;
    transaction.replayValueAuthority = false;
    transaction.oldWinS0Authority = false;
    transaction.stage2PlusAuthority = false;
    transaction.hostExtractedFileAuthority = false;
    transaction.hostFilesystemAuthority = false;
    return transaction;
}

bool IsExactAcceptedStartupPreloadTransaction80016B84(
    const StartupPreloadTransaction80016B84& transaction) {
    static constexpr std::array<
        uint32_t, kStartupTopLevelCallCount80016B84> kExpectedOrder = {
        0x80025A34u,
        0x8001AC18u,
        0x80027120u,
        0x80025A34u,
        0x8001AC18u,
        0x80026FA4u,
        0x80015A4Cu,
        0x80015B00u,
        0x80015C20u,
        0x80015A4Cu,
        0x80015B00u,
        0x80015C20u,
        0x80025A34u,
    };
    return transaction.status == StartupPreloadStatus80016B84::Accepted &&
           transaction.accepted && transaction.complete &&
           transaction.discBinPathKnown &&
           !transaction.discBinPath.empty() &&
           IsExactStartupCommonRow80016B84(transaction.commonRow) &&
           IsExactStartupZCompoRow80016B84(transaction.zCompoRow) &&
           transaction.exactTopLevelCallOrder &&
           transaction.callOrder == kExpectedOrder &&
           transaction.firstLogoInitCallback == 0x8001C0A0u &&
           transaction.secondLogoInitCallback == 0x8001C1B8u &&
           transaction.finalResetFunction == 0x80025A34u &&
           transaction.discImageDirectoryAuthority &&
           !transaction.intPayloadAuthority &&
           !transaction.timVramSideEffectsCommitted &&
           !transaction.vabSpuSideEffectsCommitted &&
           !transaction.inputAudioTail80026FA4Committed &&
           !transaction.hostProjection &&
           !transaction.psxMemoryBackingAuthority &&
           !transaction.replayValueAuthority &&
           !transaction.oldWinS0Authority &&
           !transaction.stage2PlusAuthority &&
           !transaction.hostExtractedFileAuthority &&
           !transaction.hostFilesystemAuthority;
}

bool BuildStartupCommonRow80016B84(
    const Transaction801C4780& transaction,
    PrMovieSegmentDirect::MovieSegmentRecord48& out) {
    out = PrMovieSegmentDirect::MovieSegmentRecord48{};
    if (!IsExactAcceptedTransaction801C4780(transaction)) {
        return false;
    }
    return BuildStartupCommonRowFromDisc80016B84(
        transaction.discBinPath, out);
}

bool BuildPracticeYCompoRow80015618(
    const Transaction801C4780& transaction,
    PrMovieSegmentDirect::MovieSegmentRecord48& out) {
    out = PrMovieSegmentDirect::MovieSegmentRecord48{};
    if (!IsExactAcceptedTransaction801C4780(transaction)) {
        return false;
    }

    const auto identity = PrSceneEntryDirect::IdentifySceneEntryPathPtr(
        kPracticeYCompoPathPtr80015618);
    constexpr char kExpectedPsxPath[] = "\\S0\\YCOMPO.INT;1";
    if (!identity.known || !identity.pathPtrKnown ||
        identity.pathPtr != kPracticeYCompoPathPtr80015618 ||
        identity.role != PrSceneEntryDirect::SceneEntryPathRole::PracticeYCompo ||
        identity.psxPath == nullptr ||
        std::strcmp(identity.psxPath, kExpectedPsxPath) != 0) {
        return false;
    }

    PrMovieSegmentDirect::MovieSegmentRecord48 lookupRow{};
    lookupRow.known = true;
    lookupRow.psxAddr = kPracticeYCompoRowAddress80015618;
    lookupRow.tableIndex = kPracticeYCompoRowTableIndex80015618;
    lookupRow.pathPtrA1Plus00Known = true;
    lookupRow.pathPtrA1Plus00 = kPracticeYCompoPathPtr80015618;
    lookupRow.loadedStateA1Plus0CKnown = true;
    lookupRow.loadedStateA1Plus0C = 0;

    MovieSegmentRowInitFeedback8001A324 feedback{};
    if (!BuildRowLookupFeedback801C4780(
            transaction.discBinPath, lookupRow, feedback) ||
        !feedback.loadedStateA1Plus0CKnown ||
        feedback.loadedStateA1Plus0C != 0 ||
        feedback.lookupRequestCdlFilePtr !=
            kPracticeYCompoRowAddress80015618 + 0x10u ||
        feedback.lookupRequestPathPtr != kPracticeYCompoPathPtr80015618 ||
        feedback.lookupResultPtr !=
            kPracticeYCompoRowAddress80015618 + 0x10u) {
        return false;
    }

    const PrMovieSegmentDirect::MovieSegmentRecord48 candidate =
        PrMovieSegmentDirect::ApplyMovieSegmentRowFeedback8001A324(
            lookupRow, feedback);
    if (!IsExactPracticeYCompoLookupRow80015618(candidate)) {
        return false;
    }

    const auto init =
        PrMovieSegmentDirect::PsxCall8001A324_InitSegmentRecord(candidate);
    if (init.result != 0 || init.skippedPathPtrZero ||
        init.skippedAlreadyLoaded || !init.cdLookupSucceeded ||
        !init.lookupRequestKnown ||
        init.lookupRequestCdlFilePtr !=
            kPracticeYCompoRowAddress80015618 + 0x10u ||
        !init.lookupRequestPathPtrKnown ||
        init.lookupRequestPathPtr != kPracticeYCompoPathPtr80015618 ||
        !init.loadedStateWrittenA1Plus0C ||
        !init.timeBaseWrittenA1Plus40 || !init.endWrittenA1Plus44 ||
        !IsExactPracticeYCompoRow80015618(init.record)) {
        return false;
    }
    out = init.record;
    return true;
}

Transaction801C4780 BuildTransaction801C4780(
    const Source801C4780& source) {
    Transaction801C4780 transaction{};
    if (!source.sceneIndexKnown || !source.sceneEntryBaseKnown ||
        !source.directCompoProjectionKnown) {
        return transaction;
    }
    if (source.sceneIndex != kSceneIndex801C4780 ||
        source.sceneEntryBase != kSceneEntryBase801C4780) {
        transaction.status = Status801C4780::MalformedSource;
        return transaction;
    }
    transaction.directCompoProjectionKnown = true;
    transaction.directCompoProjectionReady =
        source.directCompoProjectionReady;
    if (!source.directCompoProjectionReady) {
        transaction.status =
            Status801C4780::DirectCompoProjectionUnavailable;
        return transaction;
    }

    std::filesystem::path discBinPath;
    std::error_code ec;
    if (source.discBinPathKnown) {
        if (source.discBinPath.empty() ||
            !std::filesystem::is_regular_file(source.discBinPath, ec)) {
            transaction.status = Status801C4780::DiscImageUnavailable;
            return transaction;
        }
        discBinPath = source.discBinPath;
    } else {
        if (!TryResolveDiscBinPath801C4780(source.dataRoot, discBinPath)) {
            transaction.status = Status801C4780::DiscImageUnavailable;
            return transaction;
        }
    }
    transaction.discBinPathKnown = true;
    transaction.discBinPath = discBinPath;

    SceneEntryMovieSegmentTable801C4780 table{};
    if (!BuildStaticTable801C4780(source.sceneIndex,
                                  source.sceneEntryBase,
                                  table)) {
        transaction.status = Status801C4780::StaticRowsUnavailable;
        return transaction;
    }

    std::array<MovieSegmentRowInitFeedback8001A324,
               PrMovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780>
        feedback{};
    for (uint32_t index = 0u;
         index < PrMovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        const auto& row = table.rows[index];
        if (row.pathPtrA1Plus00Known && row.pathPtrA1Plus00 == 0u) {
            continue;
        }
        if (!BuildRowLookupFeedback801C4780(
                discBinPath, row, feedback[index])) {
            transaction.status = Status801C4780::DiscLookupUnavailable;
            return transaction;
        }
    }

    transaction.scan801C4780 =
        PrMovieSegmentDirect::PsxCall801C4780_ScanMovieSegmentsWithFeedback(
            table,
            feedback.data(),
            static_cast<uint32_t>(feedback.size()));
    if (!IsExactScan801C4780(transaction.scan801C4780)) {
        transaction.status = Status801C4780::DiscLookupUnavailable;
        return transaction;
    }

    transaction.status = Status801C4780::Accepted;
    transaction.accepted = true;
    transaction.complete = true;
    transaction.discImageDirectoryAuthority = true;
    transaction.hostProjection = true;
    transaction.psxMemoryBackingAuthority = false;
    transaction.final8001AC18ResultKnown = false;
    transaction.final8001AC18Result = 0;
    transaction.final8001AC18ResultAuthority = false;
    transaction.replayValueAuthority = false;
    transaction.oldWinS0Authority = false;
    transaction.stage1StateAuthority = false;
    transaction.stage2PlusAuthority = false;
    transaction.hostExtractedFileAuthority = false;
    transaction.hostFilesystemAuthority = false;
    transaction.discImagePathAuthority = false;
    transaction.preopenRowResultsRequiredByPsx = false;
    transaction.directFailClosedAllNonNullRows = true;
    return transaction;
}

bool IsExactAcceptedTransaction801C4780(
    const Transaction801C4780& transaction) {
    return transaction.status == Status801C4780::Accepted &&
           transaction.accepted && transaction.complete &&
           transaction.discImageDirectoryAuthority &&
           transaction.hostProjection &&
           !transaction.psxMemoryBackingAuthority &&
           !transaction.final8001AC18ResultKnown &&
           transaction.final8001AC18Result == 0 &&
           !transaction.final8001AC18ResultAuthority &&
           !transaction.replayValueAuthority &&
           !transaction.oldWinS0Authority &&
           !transaction.stage1StateAuthority &&
           !transaction.stage2PlusAuthority &&
           !transaction.hostExtractedFileAuthority &&
           !transaction.hostFilesystemAuthority &&
           !transaction.discImagePathAuthority &&
           !transaction.preopenRowResultsRequiredByPsx &&
           transaction.directFailClosedAllNonNullRows &&
           transaction.directCompoProjectionKnown &&
           transaction.directCompoProjectionReady &&
           transaction.discBinPathKnown &&
           !transaction.discBinPath.empty() &&
           IsExactScan801C4780(transaction.scan801C4780);
}

}  // namespace PrSS0Scene0ResourceIngressDirect
