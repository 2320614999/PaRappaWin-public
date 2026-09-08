#include "pr/pr_ss0_scene0_int_load_direct.h"

#include "pr/pr_stage1_loader_cd_hal.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
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

void FillCompo00Name(std::array<uint8_t, 16>& out) {
    constexpr char kName[] = "COMPO00.INT;1";
    for (std::size_t index = 0u; index < sizeof(kName) - 1u; ++index) {
        out[index] = static_cast<uint8_t>(kName[index]);
    }
}

void FillYCompoName(std::array<uint8_t, 16>& out) {
    constexpr char kName[] = "YCOMPO.INT;1";
    for (std::size_t index = 0u; index < sizeof(kName) - 1u; ++index) {
        out[index] = static_cast<uint8_t>(kName[index]);
    }
}

void FillCommonName(std::array<uint8_t, 16>& out) {
    constexpr char kName[] = "COMMON.INT;1";
    for (std::size_t index = 0u; index < sizeof(kName) - 1u; ++index) {
        out[index] = static_cast<uint8_t>(kName[index]);
    }
}

void FillZCompoName(std::array<uint8_t, 16>& out) {
    constexpr char kName[] = "ZCOMPO.INT;1";
    for (std::size_t index = 0u; index < sizeof(kName) - 1u; ++index) {
        out[index] = static_cast<uint8_t>(kName[index]);
    }
}

PrSS0Scene0IntLoadDirect::Source8001AC18 MakePlausibleSource(
    const std::filesystem::path& discBinPath) {
    using namespace PrSS0Scene0IntLoadDirect;

    Source8001AC18 source{};
    source.ingressKnown = true;
    source.ingressAccepted = true;
    source.discBinPathKnown = true;
    source.discBinPath = discBinPath;
    source.compoRowKnown = true;
    source.compoRow.known = true;
    source.compoRow.psxAddr = kCompoRowPsxAddr8001AC18;
    source.compoRow.tableIndex = kCompoRowTableIndex8001AC18;
    source.compoRow.pathPtrA1Plus00Known = true;
    source.compoRow.pathPtrA1Plus00 = kCompoPathPtr8001AC18;
    source.compoRow.loadedStateA1Plus0CKnown = true;
    source.compoRow.loadedStateA1Plus0C = 1;
    source.compoRow.startMsfKnown = true;
    source.compoRow.startMsf.minute = 0x02u;
    source.compoRow.startMsf.second = 0x00u;
    source.compoRow.startMsf.frame = 0x00u;
    source.compoRow.lengthSourceA1Plus20Known = true;
    source.compoRow.lengthSourceA1Plus20 = kHeaderBytes8001A8F0;
    source.compoRow.cdlFileNameA1Plus18Known = true;
    FillCompo00Name(source.compoRow.cdlFileNameA1Plus18);
    return source;
}

PrSS0Scene0IntLoadDirect::YCompoSource80015618
MakePlausibleYCompoSource(const std::filesystem::path& discBinPath) {
    using namespace PrSS0Scene0IntLoadDirect;

    YCompoSource80015618 source{};
    source.ingressKnown = true;
    source.ingressAccepted = true;
    source.discBinPathKnown = true;
    source.discBinPath = discBinPath;
    source.yCompoRowKnown = true;
    source.yCompoRow.known = true;
    source.yCompoRow.psxAddr = kYCompoRowPsxAddr80015618;
    source.yCompoRow.tableIndex = kYCompoRowTableIndex80015618;
    source.yCompoRow.pathPtrA1Plus00Known = true;
    source.yCompoRow.pathPtrA1Plus00 = kYCompoPathPtr80015618;
    source.yCompoRow.loadedStateA1Plus0CKnown = true;
    source.yCompoRow.loadedStateA1Plus0C = 1;
    source.yCompoRow.startMsfKnown = true;
    source.yCompoRow.startMsf.minute = 0x00u;
    source.yCompoRow.startMsf.second = 0x02u;
    source.yCompoRow.startMsf.frame = 0x00u;
    source.yCompoRow.lengthSourceA1Plus20Known = true;
    source.yCompoRow.lengthSourceA1Plus20 = kHeaderBytes8001A8F0;
    source.yCompoRow.cdlFileNameA1Plus18Known = true;
    FillYCompoName(source.yCompoRow.cdlFileNameA1Plus18);
    return source;
}

PrSS0Scene0IntLoadDirect::StartupCommonSource80016B84
MakePlausibleCommonSource(const std::filesystem::path& discBinPath) {
    using namespace PrSS0Scene0IntLoadDirect;

    StartupCommonSource80016B84 source{};
    source.ingressKnown = true;
    source.ingressAccepted = true;
    source.discBinPathKnown = true;
    source.discBinPath = discBinPath;
    source.commonRowKnown = true;
    source.commonRow.known = true;
    source.commonRow.psxAddr = kCommonRowPsxAddr80016B84;
    source.commonRow.tableIndex = kCommonRowTableIndex80016B84;
    source.commonRow.pathPtrA1Plus00Known = true;
    source.commonRow.pathPtrA1Plus00 = kCommonPathPtr80016B84;
    source.commonRow.loadedStateA1Plus0CKnown = true;
    source.commonRow.loadedStateA1Plus0C = 1;
    source.commonRow.startMsfKnown = true;
    source.commonRow.startMsf.minute = 0x02u;
    source.commonRow.startMsf.second = 0x00u;
    source.commonRow.startMsf.frame = 0x00u;
    source.commonRow.lengthSourceA1Plus20Known = true;
    source.commonRow.lengthSourceA1Plus20 = kHeaderBytes8001A8F0;
    source.commonRow.cdlFileNameA1Plus18Known = true;
    FillCommonName(source.commonRow.cdlFileNameA1Plus18);
    return source;
}

PrSS0Scene0IntLoadDirect::ZCompoSource80015590
MakePlausibleZCompoSource(const std::filesystem::path& discBinPath) {
    using namespace PrSS0Scene0IntLoadDirect;

    ZCompoSource80015590 source{};
    source.ingressKnown = true;
    source.ingressAccepted = true;
    source.discBinPathKnown = true;
    source.discBinPath = discBinPath;
    source.zCompoRowKnown = true;
    source.zCompoRow.known = true;
    source.zCompoRow.psxAddr = kZCompoRowPsxAddr80015590;
    source.zCompoRow.tableIndex = kZCompoRowTableIndex80015590;
    source.zCompoRow.pathPtrA1Plus00Known = true;
    source.zCompoRow.pathPtrA1Plus00 = kZCompoPathPtr80015590;
    source.zCompoRow.loadedStateA1Plus0CKnown = true;
    source.zCompoRow.loadedStateA1Plus0C = 1;
    source.zCompoRow.startMsfKnown = true;
    source.zCompoRow.startMsf.minute = 0x02u;
    source.zCompoRow.startMsf.second = 0x00u;
    source.zCompoRow.startMsf.frame = 0x00u;
    source.zCompoRow.lengthSourceA1Plus20Known = true;
    source.zCompoRow.lengthSourceA1Plus20 = kHeaderBytes8001A8F0;
    source.zCompoRow.cdlFileNameA1Plus18Known = true;
    FillZCompoName(source.zCompoRow.cdlFileNameA1Plus18);
    return source;
}

void WriteU32Le(std::vector<uint8_t>& bytes,
                std::size_t offset,
                uint32_t value) {
    bytes[offset + 0u] = static_cast<uint8_t>(value & 0xFFu);
    bytes[offset + 1u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
    bytes[offset + 2u] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
    bytes[offset + 3u] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
}

void WriteArchiveEntry(std::vector<uint8_t>& bytes,
                       std::size_t blockOffset,
                       uint32_t entryIndex,
                       const std::string& name) {
    const std::size_t entryOffset =
        blockOffset + 16u + static_cast<std::size_t>(entryIndex) * 20u;
    WriteU32Le(bytes, entryOffset, 1u);
    for (std::size_t index = 0u;
         index < name.size() && index < 16u;
         ++index) {
        bytes[entryOffset + 4u + index] =
            static_cast<uint8_t>(name[index]);
    }
}

std::vector<uint8_t> BuildSyntheticYCompoArchive() {
    using namespace PrSS0Scene0IntLoadDirect;

    constexpr std::size_t kBlockBytes =
        kHeaderBytes8001A8F0 + kSectorBytes8001A8F0;
    constexpr std::size_t kTimOffset = 0u;
    constexpr std::size_t kVabOffset = kBlockBytes;
    constexpr std::size_t kEofOffset = kBlockBytes * 2u;
    std::vector<uint8_t> bytes(
        kBlockBytes * 2u + kHeaderBytes8001A8F0, 0u);

    WriteU32Le(bytes, kTimOffset + 0u, 1u);
    WriteU32Le(bytes, kTimOffset + 4u,
               kExpectedYCompoTimEntries80015618);
    WriteU32Le(bytes, kTimOffset + 8u, 1u);
    for (uint32_t index = 0u;
         index < kExpectedYCompoTimEntries80015618;
         ++index) {
        char name[16]{};
        std::snprintf(name, sizeof(name), "T%03u.TIM", index);
        WriteArchiveEntry(bytes, kTimOffset, index, name);
    }

    WriteU32Le(bytes, kVabOffset + 0u, 2u);
    WriteU32Le(bytes, kVabOffset + 4u,
               kExpectedYCompoVabEntries80015618);
    WriteU32Le(bytes, kVabOffset + 8u, 1u);
    WriteArchiveEntry(bytes, kVabOffset, 0u, "PRACTICE.VH");
    WriteArchiveEntry(bytes, kVabOffset, 1u, "PRACTICE.VB");

    WriteU32Le(bytes, kEofOffset + 0u, 0xFFFFFFFFu);
    return bytes;
}

std::vector<uint8_t> PackMode2RawSectors(
    const std::vector<uint8_t>& userData) {
    using namespace PrStage1LoaderCdHal;

    const std::size_t sectorCount =
        (userData.size() + 2047u) / 2048u;
    std::vector<uint8_t> raw(
        sectorCount * kIso9660LookupSectorBytes800381F8, 0u);
    for (std::size_t sector = 0u; sector < sectorCount; ++sector) {
        const std::size_t sourceOffset = sector * 2048u;
        const std::size_t count =
            (std::min)(std::size_t{2048u}, userData.size() - sourceOffset);
        const std::size_t destinationOffset =
            sector * kIso9660LookupSectorBytes800381F8 +
            kIso9660LookupUserDataOffset800381F8;
        for (std::size_t index = 0u; index < count; ++index) {
            raw[destinationOffset + index] = userData[sourceOffset + index];
        }
    }
    return raw;
}

bool LookupCompo00Request(const std::filesystem::path& discBinPath,
                          PrStage1LoaderCdHal::Iso9660LookupResult800381F8&
                              outLookup) {
    PrStage1LoaderCdHal::Iso9660LookupInput800381F8 input{};
    input.valid = true;
    input.binPath = discBinPath;
    input.psxPath = "\\S0\\COMPO00.INT;1";
    input.cdlFilePtr =
        PrSS0Scene0IntLoadDirect::kCompoRowPsxAddr8001AC18 + 0x10u;
    input.pathPtr = 0x800117C0u;
    input.retryIndex = 0u;
    outLookup =
        PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(input);
    CHECK(outLookup.attempted);
    CHECK(outLookup.success);
    CHECK(outLookup.feedback.cdlFilePosKnown);
    CHECK(outLookup.feedback.cdlFileSizeKnown);
    CHECK(outLookup.feedback.cdlFileNameKnown);
    CHECK(outLookup.matchedSize == outLookup.feedback.cdlFileSize);
    if (!outLookup.success ||
        !outLookup.feedback.cdlFilePosKnown ||
        !outLookup.feedback.cdlFileSizeKnown ||
        !outLookup.feedback.cdlFileNameKnown) {
        return false;
    }
    return outLookup.matchedExtentLba <= 0x7FFFFFFFu &&
           outLookup.matchedSize >=
               PrSS0Scene0IntLoadDirect::kHeaderBytes8001A8F0;
}

bool LookupArchiveRequest(
    const std::filesystem::path& discBinPath,
    const char* psxPath,
    uint32_t rowAddress,
    uint32_t pathPointer,
    PrStage1LoaderCdHal::Iso9660LookupResult800381F8& outLookup) {
    PrStage1LoaderCdHal::Iso9660LookupInput800381F8 input{};
    input.valid = true;
    input.binPath = discBinPath;
    input.psxPath = psxPath;
    input.cdlFilePtr = rowAddress + 0x10u;
    input.pathPtr = pathPointer;
    input.retryIndex = 0u;
    outLookup =
        PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(input);
    CHECK(outLookup.attempted);
    CHECK(outLookup.success);
    CHECK(outLookup.feedback.cdlFilePosKnown);
    CHECK(outLookup.feedback.cdlFileSizeKnown);
    CHECK(outLookup.feedback.cdlFileNameKnown);
    return outLookup.success && outLookup.feedback.cdlFilePosKnown &&
           outLookup.feedback.cdlFileSizeKnown &&
           outLookup.feedback.cdlFileNameKnown &&
           outLookup.matchedExtentLba <= 0x7FFFFFFFu &&
           outLookup.matchedSize >=
               PrSS0Scene0IntLoadDirect::kHeaderBytes8001A8F0;
}

PrSS0Scene0IntLoadDirect::Source8001AC18 MakeExactSource(
    const std::filesystem::path& discBinPath,
    const PrStage1LoaderCdHal::Iso9660LookupResult800381F8& lookup) {
    using namespace PrSS0Scene0IntLoadDirect;

    Source8001AC18 source = MakePlausibleSource(discBinPath);
    source.compoRow.startMsf.minute = lookup.feedback.cdlFilePos.minute;
    source.compoRow.startMsf.second = lookup.feedback.cdlFilePos.second;
    source.compoRow.startMsf.frame = lookup.feedback.cdlFilePos.frame;
    source.compoRow.lengthSourceA1Plus20 = lookup.matchedSize;
    source.compoRow.cdlFileNameA1Plus18 =
        lookup.feedback.cdlFileName;
    return source;
}

PrSS0Scene0IntLoadDirect::StartupCommonSource80016B84
MakeExactCommonSource(
    const std::filesystem::path& discBinPath,
    const PrStage1LoaderCdHal::Iso9660LookupResult800381F8& lookup) {
    auto source = MakePlausibleCommonSource(discBinPath);
    source.commonRow.startMsf.minute = lookup.feedback.cdlFilePos.minute;
    source.commonRow.startMsf.second = lookup.feedback.cdlFilePos.second;
    source.commonRow.startMsf.frame = lookup.feedback.cdlFilePos.frame;
    source.commonRow.lengthSourceA1Plus20 = lookup.matchedSize;
    source.commonRow.cdlFileNameA1Plus18 = lookup.feedback.cdlFileName;
    return source;
}

PrSS0Scene0IntLoadDirect::ZCompoSource80015590 MakeExactZCompoSource(
    const std::filesystem::path& discBinPath,
    const PrStage1LoaderCdHal::Iso9660LookupResult800381F8& lookup) {
    auto source = MakePlausibleZCompoSource(discBinPath);
    source.zCompoRow.startMsf.minute = lookup.feedback.cdlFilePos.minute;
    source.zCompoRow.startMsf.second = lookup.feedback.cdlFilePos.second;
    source.zCompoRow.startMsf.frame = lookup.feedback.cdlFilePos.frame;
    source.zCompoRow.lengthSourceA1Plus20 = lookup.matchedSize;
    source.zCompoRow.cdlFileNameA1Plus18 = lookup.feedback.cdlFileName;
    return source;
}

void CheckRejectedTransaction(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& transaction) {
    using namespace PrSS0Scene0IntLoadDirect;
    CHECK(!transaction.accepted);
    CHECK(!transaction.discImagePayloadAuthority);
    CHECK(!transaction.final8001AC18ResultKnown);
    CHECK(!transaction.sideEffectsCommitted);
    CHECK(!IsExactAcceptedTransaction8001AC18(transaction));
    CHECK(!IsExactAcceptedStartupCommonTransaction80016B84(transaction));
    CHECK(!IsExactAcceptedZCompoTransaction80015590(transaction));
    CHECK(!IsExactAcceptedYCompoTransaction80015618(transaction));
}

void TestFailClosedSources() {
    using namespace PrSS0Scene0IntLoadDirect;

    Source8001AC18 source{};
    CheckRejectedTransaction(BuildTransaction8001AC18(source));

    source = MakePlausibleSource("missing-ss0-disc-image.bin");
    source.compoRow.psxAddr = kCompoRowPsxAddr8001AC18 ^ 0x30u;
    CheckRejectedTransaction(BuildTransaction8001AC18(source));

    source = MakePlausibleSource("missing-ss0-disc-image.bin");
    CheckRejectedTransaction(BuildTransaction8001AC18(source));

    source = MakePlausibleSource("missing-ss0-disc-image.bin");
    source.discBinPathKnown = false;
    source.discBinPath.clear();
    CheckRejectedTransaction(BuildTransaction8001AC18(source));

    source = MakePlausibleSource("missing-ss0-disc-image.bin");
    source.ingressAccepted = false;
    CheckRejectedTransaction(BuildTransaction8001AC18(source));

    YCompoSource80015618 yCompoSource{};
    CheckRejectedTransaction(BuildYCompoTransaction80015618(yCompoSource));

    StartupCommonSource80016B84 commonSource{};
    CheckRejectedTransaction(
        BuildStartupCommonTransaction80016B84(commonSource));
    commonSource = MakePlausibleCommonSource(
        "missing-ss0-common-disc-image.bin");
    commonSource.commonRow.pathPtrA1Plus00 = kCompoPathPtr8001AC18;
    CheckRejectedTransaction(
        BuildStartupCommonTransaction80016B84(commonSource));

    ZCompoSource80015590 zCompoSource{};
    CheckRejectedTransaction(BuildZCompoTransaction80015590(zCompoSource));
    zCompoSource = MakePlausibleZCompoSource(
        "missing-ss0-zcompo-disc-image.bin");
    zCompoSource.zCompoRow.pathPtrA1Plus00 = kCompoPathPtr8001AC18;
    CheckRejectedTransaction(BuildZCompoTransaction80015590(zCompoSource));

    yCompoSource = MakePlausibleYCompoSource(
        "missing-ss0-ycompo-disc-image.bin");
    yCompoSource.yCompoRow.pathPtrA1Plus00 = kCompoPathPtr8001AC18;
    CheckRejectedTransaction(BuildYCompoTransaction80015618(yCompoSource));

    yCompoSource = MakePlausibleYCompoSource(
        "missing-ss0-ycompo-disc-image.bin");
    CheckRejectedTransaction(BuildYCompoTransaction80015618(yCompoSource));
}

void TestSyntheticYCompoTransaction() {
    using namespace PrSS0Scene0IntLoadDirect;

    const std::vector<uint8_t> archive = BuildSyntheticYCompoArchive();
    const std::vector<uint8_t> raw = PackMode2RawSectors(archive);
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        ("test_ss0_scene0_ycompo_profile_mode2_" +
         std::to_string(reinterpret_cast<std::uintptr_t>(&g_failures)) +
         ".bin");
    {
        std::ofstream stream(path, std::ios::binary | std::ios::trunc);
        CHECK(static_cast<bool>(stream));
        if (!stream) {
            return;
        }
        stream.write(reinterpret_cast<const char*>(raw.data()),
                     static_cast<std::streamsize>(raw.size()));
        CHECK(static_cast<bool>(stream));
    }

    YCompoSource80015618 source = MakePlausibleYCompoSource(path);
    source.yCompoRow.lengthSourceA1Plus20 =
        static_cast<uint32_t>(archive.size());
    const Transaction8001AC18 transaction =
        BuildYCompoTransaction80015618(source);
    CHECK(IsExactAcceptedYCompoTransaction80015618(transaction));
    CHECK(!IsExactAcceptedTransaction8001AC18(transaction));
    CHECK(transaction.archiveKind == ArchiveKind8001AC18::PracticeYCompo);
    CHECK(transaction.requestRow.psxAddr == kYCompoRowPsxAddr80015618);
    CHECK(transaction.requestRow.pathPtrA1Plus00 == kYCompoPathPtr80015618);
    CHECK(transaction.requestLbaKnown);
    CHECK(transaction.requestLba == 0);
    CHECK(transaction.blockCount == 3u);
    CHECK(transaction.blocks.size() == 3u);
    CHECK(transaction.blocks[0].type == BlockType8001A8F0::Tim);
    CHECK(transaction.blocks[1].type == BlockType8001A8F0::Vab);
    CHECK(transaction.blocks[2].type == BlockType8001A8F0::Eof);
    CHECK(transaction.timEntryCount == kExpectedYCompoTimEntries80015618);
    CHECK(transaction.vabEntryCount == kExpectedYCompoVabEntries80015618);
    CHECK(transaction.memEntryCount == kExpectedYCompoMemEntries80015618);
    CHECK(transaction.entryCount == 93u);
    CHECK(transaction.discImagePayloadAuthority);
    CHECK(!transaction.sideEffectsCommitted);
    CHECK(!transaction.hostFilesystemAuthority);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.stage2PlusAuthority);

    Transaction8001AC18 mutation = transaction;
    mutation.archiveKind = ArchiveKind8001AC18::Scene0Compo00;
    CHECK(!IsExactAcceptedYCompoTransaction80015618(mutation));
    mutation = transaction;
    mutation.memEntryCount = 1u;
    CHECK(!IsExactAcceptedYCompoTransaction80015618(mutation));
    mutation = transaction;
    mutation.blocks[1].type = BlockType8001A8F0::Mem;
    CHECK(!IsExactAcceptedYCompoTransaction80015618(mutation));

    std::error_code removeError;
    std::filesystem::remove(path, removeError);
    CHECK(!removeError);
}

void TestExactDiscTransaction(const std::filesystem::path& discBinPath) {
    using namespace PrSS0Scene0IntLoadDirect;

    PrStage1LoaderCdHal::Iso9660LookupResult800381F8 lookup{};
    if (!LookupCompo00Request(discBinPath, lookup)) {
        return;
    }
    PrStage1LoaderCdHal::Iso9660LookupResult800381F8 commonLookup{};
    PrStage1LoaderCdHal::Iso9660LookupResult800381F8 zCompoLookup{};
    if (!LookupArchiveRequest(
            discBinPath,
            "\\S0\\COMMON.INT;1",
            kCommonRowPsxAddr80016B84,
            kCommonPathPtr80016B84,
            commonLookup) ||
        !LookupArchiveRequest(
            discBinPath,
            "\\S0\\ZCOMPO.INT;1",
            kZCompoRowPsxAddr80015590,
            kZCompoPathPtr80015590,
            zCompoLookup)) {
        return;
    }

    const Transaction8001AC18 commonTransaction =
        BuildStartupCommonTransaction80016B84(
            MakeExactCommonSource(discBinPath, commonLookup));
    CHECK(IsExactAcceptedStartupCommonTransaction80016B84(
        commonTransaction));
    CHECK(commonTransaction.archiveKind ==
          ArchiveKind8001AC18::Scene0Common);
    CHECK(commonTransaction.blockCount == 3u);
    CHECK(commonTransaction.blocks[0].type == BlockType8001A8F0::Tim);
    CHECK(commonTransaction.blocks[1].type == BlockType8001A8F0::Vab);
    CHECK(commonTransaction.blocks[2].type == BlockType8001A8F0::Eof);
    CHECK(commonTransaction.timEntryCount ==
          kExpectedCommonTimEntries80016B84);
    CHECK(commonTransaction.vabEntryCount ==
          kExpectedCommonVabEntries80016B84);
    CHECK(commonTransaction.memEntryCount ==
          kExpectedCommonMemEntries80016B84);
    CHECK(commonTransaction.entryCount == 55u);

    const Transaction8001AC18 zCompoTransaction =
        BuildZCompoTransaction80015590(
            MakeExactZCompoSource(discBinPath, zCompoLookup));
    CHECK(IsExactAcceptedZCompoTransaction80015590(zCompoTransaction));
    CHECK(zCompoTransaction.archiveKind ==
          ArchiveKind8001AC18::Scene0ZCompo);
    CHECK(zCompoTransaction.blockCount == 4u);
    CHECK(zCompoTransaction.blocks[0].type == BlockType8001A8F0::Tim);
    CHECK(zCompoTransaction.blocks[1].type == BlockType8001A8F0::Tim);
    CHECK(zCompoTransaction.blocks[2].type == BlockType8001A8F0::Vab);
    CHECK(zCompoTransaction.blocks[3].type == BlockType8001A8F0::Eof);
    CHECK(zCompoTransaction.blocks[0].files ==
          kExpectedZCompoTimBlock0Files80015590);
    CHECK(zCompoTransaction.blocks[1].files ==
          kExpectedZCompoTimBlock1Files80015590);
    CHECK(zCompoTransaction.timEntryCount ==
          kExpectedZCompoTimEntries80015590);
    CHECK(zCompoTransaction.vabEntryCount ==
          kExpectedZCompoVabEntries80015590);
    CHECK(zCompoTransaction.memEntryCount ==
          kExpectedZCompoMemEntries80015590);
    CHECK(zCompoTransaction.entryCount == 583u);

    Transaction8001AC18 zMutation = zCompoTransaction;
    zMutation.timEntryCount = 173u;
    CHECK(!IsExactAcceptedZCompoTransaction80015590(zMutation));
    zMutation = zCompoTransaction;
    zMutation.blocks[1].type = BlockType8001A8F0::Vab;
    CHECK(!IsExactAcceptedZCompoTransaction80015590(zMutation));
    zMutation = zCompoTransaction;
    zMutation.blocks[0].files =
        kExpectedZCompoTimBlock0Files80015590 - 1u;
    zMutation.blocks[1].files =
        kExpectedZCompoTimBlock1Files80015590 + 1u;
    CHECK(!IsExactAcceptedZCompoTransaction80015590(zMutation));
    zMutation = zCompoTransaction;
    WriteU32Le(
        zMutation.archiveBytes,
        static_cast<std::size_t>(zMutation.blocks[0].headerOffset) + 4u,
        kExpectedZCompoTimBlock0Files80015590 - 1u);
    WriteU32Le(
        zMutation.archiveBytes,
        static_cast<std::size_t>(zMutation.blocks[1].headerOffset) + 4u,
        kExpectedZCompoTimBlock1Files80015590 + 1u);
    CHECK(!IsExactAcceptedZCompoTransaction80015590(zMutation));
    CHECK(static_cast<uint8_t>(ArchiveKind8001AC18::PracticeYCompo) == 2u);

    const Source8001AC18 source =
        MakeExactSource(discBinPath, lookup);
    const Transaction8001AC18 transaction =
        BuildTransaction8001AC18(source);
    CHECK(IsExactAcceptedTransaction8001AC18(transaction));
    CHECK(transaction.accepted);
    CHECK(transaction.complete);
    CHECK(transaction.archiveKind == ArchiveKind8001AC18::Scene0Compo00);
    CHECK(transaction.requestBound);
    CHECK(transaction.requestRowKnown);
    CHECK(transaction.requestRow.psxAddr == kCompoRowPsxAddr8001AC18);
    CHECK(transaction.requestLbaKnown);
    CHECK(transaction.requestLba ==
          static_cast<int32_t>(lookup.matchedExtentLba));
    CHECK(transaction.requestByteCountKnown);
    CHECK(transaction.requestByteCount == lookup.matchedSize);
    CHECK(transaction.discReadAttempted);
    CHECK(transaction.discReadSucceeded);
    CHECK(transaction.archiveBytesKnown);
    CHECK(transaction.archiveBytes.size() == lookup.matchedSize);

    CHECK(transaction.blockCount == 4u);
    CHECK(transaction.blocks.size() == 4u);
    CHECK(transaction.blocks[0].type == BlockType8001A8F0::Tim);
    CHECK(transaction.blocks[1].type == BlockType8001A8F0::Vab);
    CHECK(transaction.blocks[2].type == BlockType8001A8F0::Mem);
    CHECK(transaction.blocks[3].type == BlockType8001A8F0::Eof);
    CHECK(transaction.timEntryCount == kExpectedTimEntries8001AC18);
    CHECK(transaction.vabEntryCount == kExpectedVabEntries8001AC18);
    CHECK(transaction.memEntryCount == kExpectedMemEntries8001AC18);
    CHECK(transaction.eofPresent);
    CHECK(transaction.eofHeaderEndsArchive);

    CHECK(transaction.discImagePayloadAuthority);
    CHECK(!transaction.discImagePathAuthority);
    CHECK(!transaction.hostFilesystemAuthority);
    CHECK(!transaction.consumerReadAuthority);
    CHECK(!transaction.psxMemoryBackingAuthority);
    CHECK(!transaction.final8001AC18ResultKnown);
    CHECK(!transaction.sideEffectsCommitted);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.hostExtractedFileAuthority);
    CHECK(!transaction.stage2PlusAuthority);

    Transaction8001AC18 mutation = transaction;
    mutation.final8001AC18ResultKnown = true;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
    mutation = transaction;
    mutation.replayValueAuthority = true;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
    mutation = transaction;
    mutation.hostFilesystemAuthority = true;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
    mutation = transaction;
    mutation.consumerReadAuthority = true;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
    mutation = transaction;
    mutation.eofPresent = false;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
    mutation = transaction;
    mutation.timEntryCount = kExpectedTimEntries8001AC18 - 1u;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
    mutation = transaction;
    mutation.vabEntryCount = kExpectedVabEntries8001AC18 + 1u;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
    mutation = transaction;
    mutation.memEntryCount = kExpectedMemEntries8001AC18 - 1u;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
    mutation = transaction;
    mutation.blocks[1].type = BlockType8001A8F0::Mem;
    CHECK(!IsExactAcceptedTransaction8001AC18(mutation));
}

}  // namespace

int main(int argc, char** argv) {
    TestFailClosedSources();
    TestSyntheticYCompoTransaction();
    bool exactDiscTested = false;
    if (argc >= 2) {
        TestExactDiscTransaction(std::filesystem::path(argv[1]));
        exactDiscTested = true;
    }
    if (g_failures != 0) {
        return 1;
    }
    std::printf("test_ss0_scene0_int_load_direct: ok exactDisc=%d\n",
                exactDiscTested ? 1 : 0);
    return 0;
}
