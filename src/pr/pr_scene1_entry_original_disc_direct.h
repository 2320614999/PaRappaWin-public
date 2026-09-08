#pragma once

#include "pr_movie_segment_direct.h"
#include "pr_stage1_loader_cd_hal.h"
#include "pr_stage1_loader_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace PrScene1EntryOriginalDiscDirect {

constexpr uint32_t kSceneIndex80015D18 = 1u;
constexpr uint32_t kRowIndex80015D18 = 0u;
constexpr uint32_t kRowAddress80015D18 = 0x800548C4u;
constexpr uint32_t kPathPtr80015D18 = 0x80011784u;
constexpr char kPsxPath80015D18[] = "\\S1\\COMOD1.BIN;1";
constexpr uint32_t kDestination80015D18 = 0x801C3870u;
constexpr uint32_t kAttemptIndex80015D18 = 0u;
constexpr uint32_t kCdSeamCount80015D18 = 3u;

enum class Status80015D18 : uint8_t {
    SourceUnknown = 0,
    SourceAuthorityRejected,
    RowIdentityMismatch,
    PathIdentityMismatch,
    OriginalDiscUnavailable,
    DiscLookupMismatch,
    DiscReadFailed,
    SizeMismatch,
    ProjectionMismatch,
    Accepted,
};

struct Source80015D18 {
    bool sceneIndexKnown = false;
    uint32_t sceneIndex = 0u;
    bool rowIndexKnown = false;
    uint32_t rowIndex = 0u;
    bool rowAddressKnown = false;
    uint32_t rowAddress = 0u;
    bool pathPtrKnown = false;
    uint32_t pathPtr = 0u;
    bool psxPathKnown = false;
    std::string psxPath{};

    std::filesystem::path dataRoot{};
    bool originalDiscPathKnown = false;
    std::filesystem::path originalDiscPath{};

    bool projectedComodViewKnown = false;
    const uint8_t* projectedComodViewData = nullptr;
    std::size_t projectedComodViewSize = 0u;
    bool projectedComodComparisonOnly = false;
    bool projectedComodSourceAuthority = false;

    bool hostFilesystemAuthority = false;
    bool replayAuthority = false;
    bool oldS0Authority = false;
    bool stage2PlusAuthority = false;
};

struct Transaction80015D18 {
    Status80015D18 status = Status80015D18::SourceUnknown;
    bool accepted = false;
    bool complete = false;

    bool sceneIndexKnown = false;
    uint32_t sceneIndex = 0u;
    bool rowIndexKnown = false;
    uint32_t rowIndex = 0u;
    bool rowAddressKnown = false;
    uint32_t rowAddress = 0u;
    bool pathPtrKnown = false;
    uint32_t pathPtr = 0u;
    bool psxPathKnown = false;
    std::string psxPath{};

    bool originalDiscPathKnown = false;
    std::filesystem::path originalDiscPath{};
    bool originalDiscSourceAuthority = false;
    bool projectedComodViewKnown = false;
    bool projectedComodComparisonOnly = false;
    bool projectedComodSourceAuthority = false;
    bool byteForByteProjectionMatch = false;
    std::vector<uint8_t> projectedComodViewBytes{};

    bool hostFilesystemAuthority = false;
    bool replayAuthority = false;
    bool oldS0Authority = false;
    bool stage2PlusAuthority = false;

    PrStage1LoaderCdHal::Iso9660LookupResult800381F8 lookup800381F8{};
    PrStage1LoaderCdHal::Iso9660UserDataReadResult8001A818
        read8001A818{};
    PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324
        rowFeedback8001A324{};
    bool sectorCountKnown = false;
    uint32_t sectorCount = 0u;
    uint32_t cdSeamCount = 0u;
    std::array<PrStage1LoaderDirect::CdSeamResult,
               kCdSeamCount80015D18>
        cdSeams{};
};

bool TryResolveOriginalDiscPath80015D18(
    const std::filesystem::path& dataRoot,
    std::filesystem::path& outPath);
bool IsExactInitRow0Feedback80015D18(
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324&
        feedback);
Transaction80015D18 BuildTransaction80015D18(
    const Source80015D18& source);
bool IsExactAcceptedTransaction80015D18(
    const Transaction80015D18& transaction);

}  // namespace PrScene1EntryOriginalDiscDirect
