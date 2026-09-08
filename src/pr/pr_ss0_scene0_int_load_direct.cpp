#include "pr_ss0_scene0_int_load_direct.h"

#include "pr_stage1_loader_cd_hal.h"

#include <limits>
#include <utility>

namespace PrSS0Scene0IntLoadDirect {
namespace {

constexpr uint32_t kBlockTypeTim8001A8F0 = 1u;
constexpr uint32_t kBlockTypeVab8001A8F0 = 2u;
constexpr uint32_t kBlockTypeMem8001A8F0 = 3u;
constexpr uint32_t kBlockTypeEof8001A8F0 = 0xFFFFFFFFu;
constexpr uint32_t kBlockFieldBytes8001A8F0 = 16u;
constexpr uint32_t kExpectedCompoBlockCount8001AC18 = 4u;
constexpr uint32_t kExpectedCommonBlockCount80016B84 = 3u;
constexpr uint32_t kExpectedZCompoBlockCount80015590 = 4u;
constexpr uint32_t kExpectedYCompoBlockCount80015618 = 3u;
constexpr uint32_t kMaxProfileBlockCount8001AC18 = 4u;

struct ArchiveProfile8001AC18 {
    ArchiveKind8001AC18 kind = ArchiveKind8001AC18::Unknown;
    uint32_t rowPsxAddress = 0u;
    uint32_t rowTableIndex = 0u;
    uint32_t pathPointer = 0u;
    const char* cdlFileName = nullptr;
    uint32_t timEntries = 0u;
    uint32_t vabEntries = 0u;
    uint32_t memEntries = 0u;
    uint32_t blockCount = 0u;
    uint32_t blockTypes[kMaxProfileBlockCount8001AC18]{};
    uint32_t blockFiles[kMaxProfileBlockCount8001AC18]{};
};

ArchiveProfile8001AC18 ResolveArchiveProfile8001AC18(
    ArchiveKind8001AC18 kind) {
    ArchiveProfile8001AC18 profile{};
    profile.kind = kind;
    switch (kind) {
        case ArchiveKind8001AC18::Scene0Common:
            profile.rowPsxAddress = kCommonRowPsxAddr80016B84;
            profile.rowTableIndex = kCommonRowTableIndex80016B84;
            profile.pathPointer = kCommonPathPtr80016B84;
            profile.cdlFileName = "COMMON.INT;1";
            profile.timEntries = kExpectedCommonTimEntries80016B84;
            profile.vabEntries = kExpectedCommonVabEntries80016B84;
            profile.memEntries = kExpectedCommonMemEntries80016B84;
            profile.blockCount = kExpectedCommonBlockCount80016B84;
            profile.blockTypes[0] = kBlockTypeTim8001A8F0;
            profile.blockFiles[0] = kExpectedCommonTimEntries80016B84;
            profile.blockTypes[1] = kBlockTypeVab8001A8F0;
            profile.blockFiles[1] = kExpectedCommonVabEntries80016B84;
            profile.blockTypes[2] = kBlockTypeEof8001A8F0;
            profile.blockFiles[2] = 0u;
            return profile;
        case ArchiveKind8001AC18::Scene0Compo00:
            profile.rowPsxAddress = kCompoRowPsxAddr8001AC18;
            profile.rowTableIndex = kCompoRowTableIndex8001AC18;
            profile.pathPointer = kCompoPathPtr8001AC18;
            profile.cdlFileName = "COMPO00.INT;1";
            profile.timEntries = kExpectedTimEntries8001AC18;
            profile.vabEntries = kExpectedVabEntries8001AC18;
            profile.memEntries = kExpectedMemEntries8001AC18;
            profile.blockCount = kExpectedCompoBlockCount8001AC18;
            profile.blockTypes[0] = kBlockTypeTim8001A8F0;
            profile.blockFiles[0] = kExpectedTimEntries8001AC18;
            profile.blockTypes[1] = kBlockTypeVab8001A8F0;
            profile.blockFiles[1] = kExpectedVabEntries8001AC18;
            profile.blockTypes[2] = kBlockTypeMem8001A8F0;
            profile.blockFiles[2] = kExpectedMemEntries8001AC18;
            profile.blockTypes[3] = kBlockTypeEof8001A8F0;
            profile.blockFiles[3] = 0u;
            return profile;
        case ArchiveKind8001AC18::Scene0ZCompo:
            profile.rowPsxAddress = kZCompoRowPsxAddr80015590;
            profile.rowTableIndex = kZCompoRowTableIndex80015590;
            profile.pathPointer = kZCompoPathPtr80015590;
            profile.cdlFileName = "ZCOMPO.INT;1";
            profile.timEntries = kExpectedZCompoTimEntries80015590;
            profile.vabEntries = kExpectedZCompoVabEntries80015590;
            profile.memEntries = kExpectedZCompoMemEntries80015590;
            profile.blockCount = kExpectedZCompoBlockCount80015590;
            profile.blockTypes[0] = kBlockTypeTim8001A8F0;
            profile.blockFiles[0] =
                kExpectedZCompoTimBlock0Files80015590;
            profile.blockTypes[1] = kBlockTypeTim8001A8F0;
            profile.blockFiles[1] =
                kExpectedZCompoTimBlock1Files80015590;
            profile.blockTypes[2] = kBlockTypeVab8001A8F0;
            profile.blockFiles[2] = kExpectedZCompoVabEntries80015590;
            profile.blockTypes[3] = kBlockTypeEof8001A8F0;
            profile.blockFiles[3] = 0u;
            return profile;
        case ArchiveKind8001AC18::PracticeYCompo:
            profile.rowPsxAddress = kYCompoRowPsxAddr80015618;
            profile.rowTableIndex = kYCompoRowTableIndex80015618;
            profile.pathPointer = kYCompoPathPtr80015618;
            profile.cdlFileName = "YCOMPO.INT;1";
            profile.timEntries = kExpectedYCompoTimEntries80015618;
            profile.vabEntries = kExpectedYCompoVabEntries80015618;
            profile.memEntries = kExpectedYCompoMemEntries80015618;
            profile.blockCount = kExpectedYCompoBlockCount80015618;
            profile.blockTypes[0] = kBlockTypeTim8001A8F0;
            profile.blockFiles[0] = kExpectedYCompoTimEntries80015618;
            profile.blockTypes[1] = kBlockTypeVab8001A8F0;
            profile.blockFiles[1] = kExpectedYCompoVabEntries80015618;
            profile.blockTypes[2] = kBlockTypeEof8001A8F0;
            profile.blockFiles[2] = 0u;
            return profile;
        case ArchiveKind8001AC18::Unknown:
        default:
            return profile;
    }
}

struct ParsedArchive8001A8F0 {
    std::vector<BlockMetadata8001A8F0> blocks{};
    std::vector<EntryMetadata8001A8F0> entries{};
    uint32_t timEntryCount = 0u;
    uint32_t vabEntryCount = 0u;
    uint32_t memEntryCount = 0u;
    bool eofPresent = false;
    uint64_t eofHeaderOffset = 0u;
    bool eofHeaderEndsArchive = false;
};

bool TryAddSize8001A8F0(
    std::size_t left,
    std::size_t right,
    std::size_t& out) {
    if (right > (std::numeric_limits<std::size_t>::max)() - left) {
        return false;
    }
    out = left + right;
    return true;
}

bool TryMultiplySize8001A8F0(
    std::size_t left,
    std::size_t right,
    std::size_t& out) {
    if (left != 0u &&
        right > (std::numeric_limits<std::size_t>::max)() / left) {
        return false;
    }
    out = left * right;
    return true;
}

uint32_t ReadU32Le8001A8F0(
    const std::vector<uint8_t>& bytes,
    std::size_t offset) {
    return static_cast<uint32_t>(bytes[offset + 0u]) |
           (static_cast<uint32_t>(bytes[offset + 1u]) << 8u) |
           (static_cast<uint32_t>(bytes[offset + 2u]) << 16u) |
           (static_cast<uint32_t>(bytes[offset + 3u]) << 24u);
}

bool IsBcdByte8001AC18(uint8_t value) {
    return (value >> 4u) <= 9u && (value & 0x0Fu) <= 9u;
}

bool IsValidStartMsf8001AC18(
    const PrMovieSegmentDirect::MsfBcd80036A78& msf) {
    if (!IsBcdByte8001AC18(msf.minute) ||
        !IsBcdByte8001AC18(msf.second) ||
        !IsBcdByte8001AC18(msf.frame)) {
        return false;
    }
    return PrMovieSegmentDirect::DecodeBcd80036A78(msf.second) < 60 &&
           PrMovieSegmentDirect::DecodeBcd80036A78(msf.frame) < 75;
}

bool IsExpectedArchiveName8001AC18(
    const std::array<uint8_t, 16>& name,
    const ArchiveProfile8001AC18& profile) {
    if (!profile.cdlFileName) {
        return false;
    }
    for (std::size_t index = 0u; index < name.size(); ++index) {
        const uint8_t expectedByte =
            profile.cdlFileName[index] != '\0'
                ? static_cast<uint8_t>(profile.cdlFileName[index])
                : 0u;
        if (name[index] != expectedByte) {
            return false;
        }
        if (profile.cdlFileName[index] == '\0') {
            for (std::size_t tail = index + 1u; tail < name.size(); ++tail) {
                if (name[tail] != 0u) {
                    return false;
                }
            }
            return true;
        }
    }
    return profile.cdlFileName[name.size()] == '\0';
}

bool IsWellFormedEntryName8001A8F0(
    const std::array<uint8_t, 16>& name) {
    bool contentSeen = false;
    bool terminated = false;
    for (uint8_t value : name) {
        if (terminated) {
            if (value != 0u && value != static_cast<uint8_t>(' ')) {
                return false;
            }
            continue;
        }
        if (value == 0u) {
            terminated = true;
            continue;
        }
        if (value < 0x20u || value > 0x7Eu) {
            return false;
        }
        if (value != static_cast<uint8_t>(' ')) {
            contentSeen = true;
        }
    }
    return contentSeen;
}

bool IsExactArchiveRow8001AC18(
    const PrMovieSegmentDirect::MovieSegmentRecord48& row,
    const ArchiveProfile8001AC18& profile) {
    if (profile.kind == ArchiveKind8001AC18::Unknown ||
        !row.known || row.psxAddr != profile.rowPsxAddress ||
        row.tableIndex != profile.rowTableIndex ||
        !row.pathPtrA1Plus00Known ||
        row.pathPtrA1Plus00 != profile.pathPointer ||
        !row.loadedStateA1Plus0CKnown ||
        row.loadedStateA1Plus0C != 1 ||
        !row.startMsfKnown || !IsValidStartMsf8001AC18(row.startMsf) ||
        !row.lengthSourceA1Plus20Known ||
        row.lengthSourceA1Plus20 == 0u ||
        !row.cdlFileNameA1Plus18Known ||
        !IsExpectedArchiveName8001AC18(
            row.cdlFileNameA1Plus18, profile)) {
        return false;
    }
    const auto lba =
        PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(row.startMsf);
    return lba.known && lba.lba >= 0;
}

BlockType8001A8F0 DecodeBlockType8001A8F0(uint32_t rawType) {
    switch (rawType) {
        case kBlockTypeTim8001A8F0:
            return BlockType8001A8F0::Tim;
        case kBlockTypeVab8001A8F0:
            return BlockType8001A8F0::Vab;
        case kBlockTypeMem8001A8F0:
            return BlockType8001A8F0::Mem;
        default:
            return BlockType8001A8F0::Eof;
    }
}

Status8001AC18 ParseArchive8001A8F0(
    const std::vector<uint8_t>& bytes,
    const ArchiveProfile8001AC18& profile,
    ParsedArchive8001A8F0& parsed) {
    parsed = ParsedArchive8001A8F0{};
    if (profile.kind == ArchiveKind8001AC18::Unknown ||
        profile.blockCount == 0u ||
        profile.blockCount > kMaxProfileBlockCount8001AC18) {
        return Status8001AC18::Scene0ShapeMismatch;
    }
    constexpr uint32_t maxEntries =
        (kHeaderBytes8001A8F0 - kBlockFieldBytes8001A8F0) /
        kEntryBytes8001A8F0;

    std::size_t offset = 0u;
    for (uint32_t blockIndex = 0u;
         blockIndex < profile.blockCount;
         ++blockIndex) {
        const uint32_t expectedType = profile.blockTypes[blockIndex];
        std::size_t headerEnd = 0u;
        if (!TryAddSize8001A8F0(
                offset, kHeaderBytes8001A8F0, headerEnd)) {
            return Status8001AC18::ArchiveOverflow;
        }
        if (headerEnd > bytes.size()) {
            return expectedType == kBlockTypeEof8001A8F0
                       ? Status8001AC18::MissingEof
                       : Status8001AC18::ArchiveTruncated;
        }

        const uint32_t rawType = ReadU32Le8001A8F0(bytes, offset + 0u);
        const uint32_t files = ReadU32Le8001A8F0(bytes, offset + 4u);
        const uint32_t sectors = ReadU32Le8001A8F0(bytes, offset + 8u);
        const uint32_t unk = ReadU32Le8001A8F0(bytes, offset + 12u);
        if (rawType != expectedType) {
            return Status8001AC18::UnexpectedBlockOrder;
        }
        if (files != profile.blockFiles[blockIndex]) {
            return Status8001AC18::Scene0ShapeMismatch;
        }

        BlockMetadata8001A8F0 block{};
        block.blockIndex = blockIndex;
        block.type = DecodeBlockType8001A8F0(rawType);
        block.files = files;
        block.sectors = sectors;
        block.unk = unk;
        block.headerOffset = static_cast<uint64_t>(offset);
        block.payloadOffset = static_cast<uint64_t>(headerEnd);
        block.firstEntryIndex =
            static_cast<uint32_t>(parsed.entries.size());

        if (rawType == kBlockTypeEof8001A8F0) {
            if (files != 0u || sectors != 0u) {
                return Status8001AC18::MalformedEntry;
            }
            parsed.blocks.push_back(block);
            parsed.eofPresent = true;
            parsed.eofHeaderOffset = static_cast<uint64_t>(offset);
            parsed.eofHeaderEndsArchive = headerEnd == bytes.size();
            if (!parsed.eofHeaderEndsArchive) {
                return Status8001AC18::TrailingBytes;
            }
            offset = headerEnd;
            break;
        }

        if (files == 0u || files > maxEntries || sectors == 0u) {
            return Status8001AC18::MalformedEntry;
        }
        if (rawType == kBlockTypeVab8001A8F0 &&
            files != profile.vabEntries) {
            return Status8001AC18::Scene0ShapeMismatch;
        }

        std::size_t payloadBytes = 0u;
        if (!TryMultiplySize8001A8F0(
                static_cast<std::size_t>(sectors),
                kSectorBytes8001A8F0,
                payloadBytes)) {
            return Status8001AC18::ArchiveOverflow;
        }
        std::size_t blockEnd = 0u;
        if (!TryAddSize8001A8F0(headerEnd, payloadBytes, blockEnd)) {
            return Status8001AC18::ArchiveOverflow;
        }
        if (blockEnd > bytes.size()) {
            return Status8001AC18::ArchiveTruncated;
        }

        std::size_t dataCursor = 0u;
        for (uint32_t entryIndex = 0u;
             entryIndex < files;
             ++entryIndex) {
            std::size_t entryDelta = 0u;
            if (!TryMultiplySize8001A8F0(
                    static_cast<std::size_t>(entryIndex),
                    kEntryBytes8001A8F0,
                    entryDelta)) {
                return Status8001AC18::ArchiveOverflow;
            }
            std::size_t entryHeader = 0u;
            if (!TryAddSize8001A8F0(
                    offset + kBlockFieldBytes8001A8F0,
                    entryDelta,
                    entryHeader) ||
                entryHeader + kEntryBytes8001A8F0 > headerEnd) {
                return Status8001AC18::MalformedEntry;
            }

            EntryMetadata8001A8F0 entry{};
            entry.blockIndex = blockIndex;
            entry.entryIndex = entryIndex;
            entry.type = block.type;
            entry.size = ReadU32Le8001A8F0(bytes, entryHeader);
            entry.headerOffset = static_cast<uint64_t>(entryHeader);
            for (std::size_t nameIndex = 0u;
                 nameIndex < entry.name.size();
                 ++nameIndex) {
                entry.name[nameIndex] = bytes[entryHeader + 4u + nameIndex];
            }
            if (entry.size == 0u ||
                !IsWellFormedEntryName8001A8F0(entry.name)) {
                return Status8001AC18::MalformedEntry;
            }

            std::size_t entryEnd = 0u;
            if (!TryAddSize8001A8F0(
                    dataCursor,
                    static_cast<std::size_t>(entry.size),
                    entryEnd)) {
                return Status8001AC18::ArchiveOverflow;
            }
            if (entryEnd > payloadBytes) {
                return Status8001AC18::MalformedEntry;
            }
            entry.dataOffset =
                static_cast<uint64_t>(headerEnd + dataCursor);
            parsed.entries.push_back(entry);
            dataCursor = entryEnd;
        }

        block.payloadBytes = static_cast<uint64_t>(payloadBytes);
        block.entryDataBytes = static_cast<uint64_t>(dataCursor);
        block.sectorPaddingBytes =
            static_cast<uint64_t>(payloadBytes - dataCursor);
        parsed.blocks.push_back(block);
        if (rawType == kBlockTypeTim8001A8F0) {
            parsed.timEntryCount += files;
        } else if (rawType == kBlockTypeVab8001A8F0) {
            parsed.vabEntryCount += files;
        } else {
            parsed.memEntryCount += files;
        }
        offset = blockEnd;
    }

    if (!parsed.eofPresent) {
        return Status8001AC18::MissingEof;
    }
    if (offset != bytes.size()) {
        return Status8001AC18::TrailingBytes;
    }
    if (parsed.timEntryCount != profile.timEntries ||
        parsed.vabEntryCount != profile.vabEntries ||
        parsed.memEntryCount != profile.memEntries) {
        return Status8001AC18::Scene0ShapeMismatch;
    }
    return Status8001AC18::Accepted;
}

void CopyParsedArchive8001A8F0(
    const ParsedArchive8001A8F0& parsed,
    Transaction8001AC18& transaction) {
    transaction.blocks = parsed.blocks;
    transaction.entries = parsed.entries;
    transaction.blockCount =
        static_cast<uint32_t>(transaction.blocks.size());
    transaction.entryCount =
        static_cast<uint32_t>(transaction.entries.size());
    transaction.timEntryCount = parsed.timEntryCount;
    transaction.vabEntryCount = parsed.vabEntryCount;
    transaction.memEntryCount = parsed.memEntryCount;
    transaction.eofPresent = parsed.eofPresent;
    transaction.eofHeaderOffset = parsed.eofHeaderOffset;
    transaction.eofHeaderEndsArchive = parsed.eofHeaderEndsArchive;
}

bool EqualBlockMetadata8001A8F0(
    const BlockMetadata8001A8F0& left,
    const BlockMetadata8001A8F0& right) {
    return left.blockIndex == right.blockIndex &&
           left.type == right.type && left.files == right.files &&
           left.sectors == right.sectors && left.unk == right.unk &&
           left.headerOffset == right.headerOffset &&
           left.payloadOffset == right.payloadOffset &&
           left.payloadBytes == right.payloadBytes &&
           left.entryDataBytes == right.entryDataBytes &&
           left.sectorPaddingBytes == right.sectorPaddingBytes &&
           left.firstEntryIndex == right.firstEntryIndex;
}

bool EqualEntryMetadata8001A8F0(
    const EntryMetadata8001A8F0& left,
    const EntryMetadata8001A8F0& right) {
    return left.blockIndex == right.blockIndex &&
           left.entryIndex == right.entryIndex &&
           left.type == right.type && left.size == right.size &&
           left.name == right.name &&
           left.headerOffset == right.headerOffset &&
           left.dataOffset == right.dataOffset;
}

bool EqualParsedArchive8001A8F0(
    const ParsedArchive8001A8F0& parsed,
    const Transaction8001AC18& transaction) {
    if (parsed.blocks.size() != transaction.blocks.size() ||
        parsed.entries.size() != transaction.entries.size() ||
        parsed.timEntryCount != transaction.timEntryCount ||
        parsed.vabEntryCount != transaction.vabEntryCount ||
        parsed.memEntryCount != transaction.memEntryCount ||
        parsed.eofPresent != transaction.eofPresent ||
        parsed.eofHeaderOffset != transaction.eofHeaderOffset ||
        parsed.eofHeaderEndsArchive != transaction.eofHeaderEndsArchive) {
        return false;
    }
    for (std::size_t index = 0u; index < parsed.blocks.size(); ++index) {
        if (!EqualBlockMetadata8001A8F0(
                parsed.blocks[index], transaction.blocks[index])) {
            return false;
        }
    }
    for (std::size_t index = 0u; index < parsed.entries.size(); ++index) {
        if (!EqualEntryMetadata8001A8F0(
                parsed.entries[index], transaction.entries[index])) {
            return false;
        }
    }
    return true;
}

}  // namespace

static Transaction8001AC18 BuildTransactionForArchive8001AC18(
    ArchiveKind8001AC18 archiveKind,
    bool ingressKnown,
    bool ingressAccepted,
    bool discBinPathKnown,
    const std::filesystem::path& discBinPath,
    bool requestRowKnown,
    const PrMovieSegmentDirect::MovieSegmentRecord48& requestRow) {
    Transaction8001AC18 transaction{};
    transaction.archiveKind = archiveKind;
    const ArchiveProfile8001AC18 profile =
        ResolveArchiveProfile8001AC18(archiveKind);
    if (profile.kind == ArchiveKind8001AC18::Unknown ||
        !ingressKnown || !discBinPathKnown || !requestRowKnown) {
        return transaction;
    }
    if (!ingressAccepted) {
        transaction.status = Status8001AC18::IngressRejected;
        return transaction;
    }

    transaction.requestPathKnown = true;
    transaction.requestPath = discBinPath;
    transaction.requestRowKnown = true;
    transaction.requestRow = requestRow;
    if (discBinPath.empty() ||
        !IsExactArchiveRow8001AC18(requestRow, profile)) {
        transaction.status = Status8001AC18::MalformedRequest;
        return transaction;
    }

    const auto lba = PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(
        requestRow.startMsf);
    if (!lba.known || lba.lba < 0) {
        transaction.status = Status8001AC18::MalformedRequest;
        return transaction;
    }
    transaction.requestLbaKnown = true;
    transaction.requestLba = lba.lba;
    transaction.requestByteCountKnown = true;
    transaction.requestByteCount = requestRow.lengthSourceA1Plus20;
    transaction.requestCdlFileNameKnown = true;
    transaction.requestCdlFileName = requestRow.cdlFileNameA1Plus18;
    transaction.requestBound = true;

    PrStage1LoaderCdHal::Iso9660UserDataReadInput8001A818 input{};
    input.valid = true;
    input.binPath = discBinPath;
    input.lba = transaction.requestLba;
    input.byteCount = transaction.requestByteCount;
    auto read =
        PrStage1LoaderCdHal::ReadIso9660UserDataBytes8001A818(input);
    transaction.discReadAttempted = read.attempted;
    if (!read.attempted || !read.success || !read.binPathKnown ||
        read.binPath != transaction.requestPath || !read.lbaKnown ||
        read.lba != transaction.requestLba || !read.byteCountKnown ||
        read.byteCount != transaction.requestByteCount ||
        !read.sectorLayoutKnown ||
        read.sectorBytes !=
            PrStage1LoaderCdHal::kIso9660LookupSectorBytes800381F8 ||
        read.userDataOffset !=
            PrStage1LoaderCdHal::kIso9660LookupUserDataOffset800381F8 ||
        read.bytes.size() != transaction.requestByteCount) {
        transaction.status = Status8001AC18::DiscPayloadUnavailable;
        return transaction;
    }

    transaction.discReadSucceeded = true;
    transaction.archiveBytesKnown = true;
    transaction.archiveBytes = std::move(read.bytes);
    transaction.discImagePayloadAuthority = true;
    transaction.discImagePathAuthority = false;
    transaction.hostFilesystemAuthority = false;
    transaction.consumerReadAuthority = false;
    transaction.psxMemoryBackingAuthority = false;

    ParsedArchive8001A8F0 parsed{};
    transaction.status =
        ParseArchive8001A8F0(transaction.archiveBytes, profile, parsed);
    CopyParsedArchive8001A8F0(parsed, transaction);
    if (transaction.status != Status8001AC18::Accepted) {
        return transaction;
    }

    transaction.accepted = true;
    transaction.complete = true;
    transaction.final8001AC18ResultKnown = false;
    transaction.final8001AC18Result = 0;
    transaction.final8001AC18ResultAuthority = false;
    transaction.sideEffectsCommitted = false;
    transaction.replayValueAuthority = false;
    transaction.oldWinS0Authority = false;
    transaction.hostExtractedFileAuthority = false;
    transaction.stage2PlusAuthority = false;
    return transaction;
}

Transaction8001AC18 BuildTransaction8001AC18(
    const Source8001AC18& source) {
    return BuildTransactionForArchive8001AC18(
        ArchiveKind8001AC18::Scene0Compo00,
        source.ingressKnown,
        source.ingressAccepted,
        source.discBinPathKnown,
        source.discBinPath,
        source.compoRowKnown,
        source.compoRow);
}

Transaction8001AC18 BuildStartupCommonTransaction80016B84(
    const StartupCommonSource80016B84& source) {
    return BuildTransactionForArchive8001AC18(
        ArchiveKind8001AC18::Scene0Common,
        source.ingressKnown,
        source.ingressAccepted,
        source.discBinPathKnown,
        source.discBinPath,
        source.commonRowKnown,
        source.commonRow);
}

Transaction8001AC18 BuildYCompoTransaction80015618(
    const YCompoSource80015618& source) {
    return BuildTransactionForArchive8001AC18(
        ArchiveKind8001AC18::PracticeYCompo,
        source.ingressKnown,
        source.ingressAccepted,
        source.discBinPathKnown,
        source.discBinPath,
        source.yCompoRowKnown,
        source.yCompoRow);
}

Transaction8001AC18 BuildZCompoTransaction80015590(
    const ZCompoSource80015590& source) {
    return BuildTransactionForArchive8001AC18(
        ArchiveKind8001AC18::Scene0ZCompo,
        source.ingressKnown,
        source.ingressAccepted,
        source.discBinPathKnown,
        source.discBinPath,
        source.zCompoRowKnown,
        source.zCompoRow);
}

static bool IsExactAcceptedTransactionForArchive8001AC18(
    const Transaction8001AC18& transaction,
    ArchiveKind8001AC18 archiveKind) {
    const ArchiveProfile8001AC18 profile =
        ResolveArchiveProfile8001AC18(archiveKind);
    if (transaction.status != Status8001AC18::Accepted ||
        !transaction.accepted || !transaction.complete ||
        transaction.archiveKind != archiveKind ||
        profile.kind == ArchiveKind8001AC18::Unknown ||
        !transaction.requestBound || !transaction.requestPathKnown ||
        transaction.requestPath.empty() || !transaction.requestRowKnown ||
        !IsExactArchiveRow8001AC18(transaction.requestRow, profile) ||
        !transaction.requestLbaKnown || transaction.requestLba < 0 ||
        !transaction.requestByteCountKnown ||
        transaction.requestByteCount == 0u ||
        !transaction.requestCdlFileNameKnown ||
        transaction.requestCdlFileName !=
            transaction.requestRow.cdlFileNameA1Plus18 ||
        transaction.requestByteCount !=
            transaction.requestRow.lengthSourceA1Plus20 ||
        !transaction.discReadAttempted ||
        !transaction.discReadSucceeded ||
        !transaction.archiveBytesKnown ||
        transaction.archiveBytes.size() != transaction.requestByteCount ||
        !transaction.discImagePayloadAuthority ||
        transaction.discImagePathAuthority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority ||
        transaction.psxMemoryBackingAuthority ||
        transaction.final8001AC18ResultKnown ||
        transaction.final8001AC18Result != 0 ||
        transaction.final8001AC18ResultAuthority ||
        transaction.sideEffectsCommitted ||
        transaction.replayValueAuthority ||
        transaction.oldWinS0Authority ||
        transaction.hostExtractedFileAuthority ||
        transaction.stage2PlusAuthority ||
        transaction.blockCount != profile.blockCount ||
        transaction.blockCount != transaction.blocks.size() ||
        transaction.entryCount != transaction.entries.size() ||
        transaction.timEntryCount != profile.timEntries ||
        transaction.vabEntryCount != profile.vabEntries ||
        transaction.memEntryCount != profile.memEntries ||
        !transaction.eofPresent ||
        !transaction.eofHeaderEndsArchive) {
        return false;
    }

    const auto lba = PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(
        transaction.requestRow.startMsf);
    if (!lba.known || lba.lba != transaction.requestLba ||
        !IsExpectedArchiveName8001AC18(
            transaction.requestCdlFileName, profile)) {
        return false;
    }

    ParsedArchive8001A8F0 parsed{};
    return ParseArchive8001A8F0(
               transaction.archiveBytes, profile, parsed) ==
               Status8001AC18::Accepted &&
           EqualParsedArchive8001A8F0(parsed, transaction);
}

bool IsExactAcceptedTransaction8001AC18(
    const Transaction8001AC18& transaction) {
    return IsExactAcceptedTransactionForArchive8001AC18(
        transaction, ArchiveKind8001AC18::Scene0Compo00);
}

bool IsExactAcceptedStartupCommonTransaction80016B84(
    const Transaction8001AC18& transaction) {
    return IsExactAcceptedTransactionForArchive8001AC18(
        transaction, ArchiveKind8001AC18::Scene0Common);
}

bool IsExactAcceptedYCompoTransaction80015618(
    const Transaction8001AC18& transaction) {
    return IsExactAcceptedTransactionForArchive8001AC18(
        transaction, ArchiveKind8001AC18::PracticeYCompo);
}

bool IsExactAcceptedZCompoTransaction80015590(
    const Transaction8001AC18& transaction) {
    return IsExactAcceptedTransactionForArchive8001AC18(
        transaction, ArchiveKind8001AC18::Scene0ZCompo);
}

}  // namespace PrSS0Scene0IntLoadDirect
