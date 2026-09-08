#pragma once

#include "pr_movie_segment_direct.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace PrSS0Scene0IntLoadDirect {

constexpr uint32_t kCompoRowPsxAddr8001AC18 = 0x80054788u;
constexpr uint32_t kCompoRowTableIndex8001AC18 = 1u;
constexpr uint32_t kCompoPathPtr8001AC18 = 0x800117C0u;
constexpr uint32_t kHeaderBytes8001A8F0 = 8192u;
constexpr uint32_t kSectorBytes8001A8F0 = 2048u;
constexpr uint32_t kEntryBytes8001A8F0 = 20u;
constexpr uint32_t kExpectedTimEntries8001AC18 = 87u;
constexpr uint32_t kExpectedVabEntries8001AC18 = 2u;
constexpr uint32_t kExpectedMemEntries8001AC18 = 59u;
constexpr uint32_t kCommonRowPsxAddr80016B84 = 0x8005468Cu;
constexpr uint32_t kCommonRowTableIndex80016B84 = 0u;
constexpr uint32_t kCommonPathPtr80016B84 = 0x800113A0u;
constexpr uint32_t kExpectedCommonTimEntries80016B84 = 53u;
constexpr uint32_t kExpectedCommonVabEntries80016B84 = 2u;
constexpr uint32_t kExpectedCommonMemEntries80016B84 = 0u;
constexpr uint32_t kZCompoRowPsxAddr80015590 = 0x80054878u;
constexpr uint32_t kZCompoRowTableIndex80015590 = 6u;
constexpr uint32_t kZCompoPathPtr80015590 = 0x80011798u;
constexpr uint32_t kExpectedZCompoTimEntries80015590 = 581u;
constexpr uint32_t kExpectedZCompoTimBlock0Files80015590 = 408u;
constexpr uint32_t kExpectedZCompoTimBlock1Files80015590 = 173u;
constexpr uint32_t kExpectedZCompoVabEntries80015590 = 2u;
constexpr uint32_t kExpectedZCompoMemEntries80015590 = 0u;
constexpr uint32_t kYCompoRowPsxAddr80015618 = 0x800546ECu;
constexpr uint32_t kYCompoRowTableIndex80015618 = 0u;
constexpr uint32_t kYCompoPathPtr80015618 = 0x800113C8u;
constexpr uint32_t kExpectedYCompoTimEntries80015618 = 91u;
constexpr uint32_t kExpectedYCompoVabEntries80015618 = 2u;
constexpr uint32_t kExpectedYCompoMemEntries80015618 = 0u;

enum class ArchiveKind8001AC18 : uint8_t {
    Unknown = 0,
    Scene0Compo00 = 1,
    PracticeYCompo = 2,
    Scene0ZCompo = 3,
    Scene0Common = 4,
};

static_assert(
    static_cast<uint8_t>(ArchiveKind8001AC18::PracticeYCompo) == 2u,
    "PracticeYCompo archive-kind ABI must remain stable");

enum class Status8001AC18 : uint8_t {
    SourceUnknown = 0,
    IngressRejected,
    MalformedRequest,
    DiscPayloadUnavailable,
    ArchiveOverflow,
    ArchiveTruncated,
    UnexpectedBlockOrder,
    MalformedEntry,
    MissingEof,
    TrailingBytes,
    Scene0ShapeMismatch,
    Accepted,
};

enum class BlockType8001A8F0 : int32_t {
    Tim = 1,
    Vab = 2,
    Mem = 3,
    Eof = -1,
};

struct Source8001AC18 {
    bool ingressKnown = false;
    bool ingressAccepted = false;
    bool discBinPathKnown = false;
    std::filesystem::path discBinPath{};
    bool compoRowKnown = false;
    PrMovieSegmentDirect::MovieSegmentRecord48 compoRow{};
};

struct StartupCommonSource80016B84 {
    bool ingressKnown = false;
    bool ingressAccepted = false;
    bool discBinPathKnown = false;
    std::filesystem::path discBinPath{};
    bool commonRowKnown = false;
    PrMovieSegmentDirect::MovieSegmentRecord48 commonRow{};
};

struct YCompoSource80015618 {
    bool ingressKnown = false;
    bool ingressAccepted = false;
    bool discBinPathKnown = false;
    std::filesystem::path discBinPath{};
    bool yCompoRowKnown = false;
    PrMovieSegmentDirect::MovieSegmentRecord48 yCompoRow{};
};

struct ZCompoSource80015590 {
    bool ingressKnown = false;
    bool ingressAccepted = false;
    bool discBinPathKnown = false;
    std::filesystem::path discBinPath{};
    bool zCompoRowKnown = false;
    PrMovieSegmentDirect::MovieSegmentRecord48 zCompoRow{};
};

struct BlockMetadata8001A8F0 {
    uint32_t blockIndex = 0u;
    BlockType8001A8F0 type = BlockType8001A8F0::Eof;
    uint32_t files = 0u;
    uint32_t sectors = 0u;
    uint32_t unk = 0u;
    uint64_t headerOffset = 0u;
    uint64_t payloadOffset = 0u;
    uint64_t payloadBytes = 0u;
    uint64_t entryDataBytes = 0u;
    uint64_t sectorPaddingBytes = 0u;
    uint32_t firstEntryIndex = 0u;
};

struct EntryMetadata8001A8F0 {
    uint32_t blockIndex = 0u;
    uint32_t entryIndex = 0u;
    BlockType8001A8F0 type = BlockType8001A8F0::Eof;
    uint32_t size = 0u;
    std::array<uint8_t, 16> name{};
    uint64_t headerOffset = 0u;
    uint64_t dataOffset = 0u;
};

struct Transaction8001AC18 {
    Status8001AC18 status = Status8001AC18::SourceUnknown;
    bool accepted = false;
    bool complete = false;
    ArchiveKind8001AC18 archiveKind = ArchiveKind8001AC18::Unknown;

    bool requestBound = false;
    bool requestPathKnown = false;
    std::filesystem::path requestPath{};
    bool requestRowKnown = false;
    PrMovieSegmentDirect::MovieSegmentRecord48 requestRow{};
    bool requestLbaKnown = false;
    int32_t requestLba = 0;
    bool requestByteCountKnown = false;
    uint32_t requestByteCount = 0u;
    bool requestCdlFileNameKnown = false;
    std::array<uint8_t, 16> requestCdlFileName{};

    bool discReadAttempted = false;
    bool discReadSucceeded = false;
    bool archiveBytesKnown = false;
    std::vector<uint8_t> archiveBytes{};
    std::vector<BlockMetadata8001A8F0> blocks{};
    std::vector<EntryMetadata8001A8F0> entries{};
    uint32_t blockCount = 0u;
    uint32_t entryCount = 0u;
    uint32_t timEntryCount = 0u;
    uint32_t vabEntryCount = 0u;
    uint32_t memEntryCount = 0u;
    bool eofPresent = false;
    uint64_t eofHeaderOffset = 0u;
    bool eofHeaderEndsArchive = false;

    bool discImagePayloadAuthority = false;
    bool discImagePathAuthority = false;
    bool hostFilesystemAuthority = false;
    bool consumerReadAuthority = false;
    bool psxMemoryBackingAuthority = false;
    bool final8001AC18ResultKnown = false;
    int32_t final8001AC18Result = 0;
    bool final8001AC18ResultAuthority = false;
    bool sideEffectsCommitted = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool hostExtractedFileAuthority = false;
    bool stage2PlusAuthority = false;
};

Transaction8001AC18 BuildTransaction8001AC18(
    const Source8001AC18& source);
bool IsExactAcceptedTransaction8001AC18(
    const Transaction8001AC18& transaction);
Transaction8001AC18 BuildStartupCommonTransaction80016B84(
    const StartupCommonSource80016B84& source);
bool IsExactAcceptedStartupCommonTransaction80016B84(
    const Transaction8001AC18& transaction);
Transaction8001AC18 BuildZCompoTransaction80015590(
    const ZCompoSource80015590& source);
bool IsExactAcceptedZCompoTransaction80015590(
    const Transaction8001AC18& transaction);
Transaction8001AC18 BuildYCompoTransaction80015618(
    const YCompoSource80015618& source);
bool IsExactAcceptedYCompoTransaction80015618(
    const Transaction8001AC18& transaction);

}  // namespace PrSS0Scene0IntLoadDirect
