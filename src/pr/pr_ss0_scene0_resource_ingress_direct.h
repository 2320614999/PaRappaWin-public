#pragma once

#include "pr_movie_segment_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace PrSS0Scene0ResourceIngressDirect {

constexpr uint32_t kSceneIndex801C4780 = 0u;
constexpr uint32_t kSceneEntryBase801C4780 = 0x8005474Cu;
constexpr uint32_t kCompoRowAddress801C4780 = 0x80054788u;
constexpr uint32_t kExpectedLookupRowCount801C4780 = 5u;
constexpr uint32_t kExpectedDisabledRowCount801C4780 = 2u;
constexpr uint32_t kStartupCommonRowAddress80016B84 = 0x8005468Cu;
constexpr uint32_t kStartupCommonRowTableIndex80016B84 = 0u;
constexpr uint32_t kStartupCommonPathPtr80016B84 = 0x800113A0u;
constexpr uint32_t kPracticeYCompoRowAddress80015618 = 0x800546ECu;
constexpr uint32_t kPracticeYCompoRowTableIndex80015618 = 0u;
constexpr uint32_t kPracticeYCompoPathPtr80015618 = 0x800113C8u;
constexpr uint32_t kStartupZCompoRowAddress80016B84 = 0x80054878u;
constexpr uint32_t kStartupZCompoRowTableIndex80016B84 = 6u;
constexpr uint32_t kStartupZCompoPathPtr80016B84 = 0x80011798u;
constexpr std::size_t kStartupTopLevelCallCount80016B84 = 13u;

enum class Status801C4780 : uint8_t {
    SourceUnknown = 0,
    MalformedSource,
    DirectCompoProjectionUnavailable,
    DiscImageUnavailable,
    StaticRowsUnavailable,
    DiscLookupUnavailable,
    Accepted,
};

struct Source801C4780 {
    bool sceneIndexKnown = false;
    uint32_t sceneIndex = 0u;
    bool sceneEntryBaseKnown = false;
    uint32_t sceneEntryBase = 0u;

    std::filesystem::path dataRoot{};
    bool discBinPathKnown = false;
    std::filesystem::path discBinPath{};

    bool directCompoProjectionKnown = false;
    bool directCompoProjectionReady = false;
};

struct Transaction801C4780 {
    Status801C4780 status = Status801C4780::SourceUnknown;
    bool accepted = false;
    bool complete = false;

    bool discImageDirectoryAuthority = false;
    bool hostProjection = false;
    bool psxMemoryBackingAuthority = false;
    bool final8001AC18ResultKnown = false;
    int32_t final8001AC18Result = 0;
    bool final8001AC18ResultAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage1StateAuthority = false;
    bool stage2PlusAuthority = false;
    bool hostExtractedFileAuthority = false;
    bool hostFilesystemAuthority = false;
    bool discImagePathAuthority = false;

    bool preopenRowResultsRequiredByPsx = false;
    bool directFailClosedAllNonNullRows = false;
    bool directCompoProjectionKnown = false;
    bool directCompoProjectionReady = false;
    bool discBinPathKnown = false;
    std::filesystem::path discBinPath{};

    PrMovieSegmentDirect::MovieSegmentScanResult801C4780 scan801C4780{};
};

enum class StartupPreloadStatus80016B84 : uint8_t {
    SourceUnknown = 0,
    DiscImageUnavailable,
    StaticRowsUnavailable,
    CommonLookupUnavailable,
    ZCompoLookupUnavailable,
    Accepted,
};

struct StartupPreloadSource80016B84 {
    bool dataRootKnown = false;
    std::filesystem::path dataRoot{};
    bool discBinPathKnown = false;
    std::filesystem::path discBinPath{};
};

// Direct source transaction for the 80015D18 -> 80016B84 prefix.  It owns
// the original-disc lookup rows and exact top-level call order before the
// first logo callback.  INT payload parsing and runtime side effects are
// committed by the caller in their own typed transactions; this carrier does
// not claim TIM/VRAM, VAB/SPU, or 80026FA4 execution authority by itself.
struct StartupPreloadTransaction80016B84 {
    StartupPreloadStatus80016B84 status =
        StartupPreloadStatus80016B84::SourceUnknown;
    bool accepted = false;
    bool complete = false;
    bool discBinPathKnown = false;
    std::filesystem::path discBinPath{};
    PrMovieSegmentDirect::MovieSegmentRecord48 commonRow{};
    PrMovieSegmentDirect::MovieSegmentRecord48 zCompoRow{};
    bool exactTopLevelCallOrder = false;
    std::array<uint32_t, kStartupTopLevelCallCount80016B84> callOrder{};
    uint32_t firstLogoInitCallback = 0u;
    uint32_t secondLogoInitCallback = 0u;
    uint32_t finalResetFunction = 0u;
    bool discImageDirectoryAuthority = false;
    bool intPayloadAuthority = false;
    bool timVramSideEffectsCommitted = false;
    bool vabSpuSideEffectsCommitted = false;
    bool inputAudioTail80026FA4Committed = false;
    bool hostProjection = false;
    bool psxMemoryBackingAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool hostExtractedFileAuthority = false;
    bool hostFilesystemAuthority = false;
};

bool TryResolveDiscBinPath801C4780(
    const std::filesystem::path& dataRoot,
    std::filesystem::path& outPath);
StartupPreloadTransaction80016B84
BuildStartupPreloadTransaction80016B84(
    const StartupPreloadSource80016B84& source);
bool IsExactAcceptedStartupPreloadTransaction80016B84(
    const StartupPreloadTransaction80016B84& transaction);
Transaction801C4780 BuildTransaction801C4780(
    const Source801C4780& source);
bool IsExactAcceptedTransaction801C4780(
    const Transaction801C4780& transaction);
bool BuildStartupCommonRow80016B84(
    const Transaction801C4780& transaction,
    PrMovieSegmentDirect::MovieSegmentRecord48& out);
bool BuildPracticeYCompoRow80015618(
    const Transaction801C4780& transaction,
    PrMovieSegmentDirect::MovieSegmentRecord48& out);

}  // namespace PrSS0Scene0ResourceIngressDirect
