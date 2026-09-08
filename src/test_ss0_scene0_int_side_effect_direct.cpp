#include "pr/pr_ss0_scene0_int_side_effect_direct.h"

#include "pr/pr_stage1_loader_cd_hal.h"
#include "pr/pr_stage1_loader_memory_direct.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
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

void FillCompoName(std::array<uint8_t, 16>& out) {
    constexpr char kName[] = "COMPO00.INT;1";
    for (std::size_t i = 0; i < sizeof(kName) - 1u; ++i) {
        out[i] = static_cast<uint8_t>(kName[i]);
    }
}

void FillYCompoName(std::array<uint8_t, 16>& out) {
    constexpr char kName[] = "YCOMPO.INT;1";
    for (std::size_t i = 0; i < sizeof(kName) - 1u; ++i) {
        out[i] = static_cast<uint8_t>(kName[i]);
    }
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
                       uint32_t size,
                       const std::string& name) {
    const std::size_t entryOffset =
        blockOffset + 16u + static_cast<std::size_t>(entryIndex) * 20u;
    WriteU32Le(bytes, entryOffset, size);
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
        WriteArchiveEntry(bytes, kTimOffset, index, 1u, name);
        bytes[kHeaderBytes8001A8F0 + index] =
            static_cast<uint8_t>(0x20u + index);
    }

    WriteU32Le(bytes, kVabOffset + 0u, 2u);
    WriteU32Le(bytes, kVabOffset + 4u,
               kExpectedYCompoVabEntries80015618);
    WriteU32Le(bytes, kVabOffset + 8u, 1u);
    WriteArchiveEntry(bytes, kVabOffset, 0u, 8u, "PRACTICE.VH");
    WriteArchiveEntry(bytes, kVabOffset, 1u, 8u, "PRACTICE.VB");
    const std::size_t vabPayload = kVabOffset + kHeaderBytes8001A8F0;
    for (std::size_t index = 0u; index < 16u; ++index) {
        bytes[vabPayload + index] = static_cast<uint8_t>(0xA0u + index);
    }

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
        const std::size_t count = (std::min)(
            std::size_t{2048u}, userData.size() - sourceOffset);
        const std::size_t destinationOffset =
            sector * kIso9660LookupSectorBytes800381F8 +
            kIso9660LookupUserDataOffset800381F8;
        std::copy_n(userData.data() + sourceOffset,
                    count,
                    raw.data() + destinationOffset);
    }
    return raw;
}

PrSS0Scene0IntLoadDirect::YCompoSource80015618 MakeYCompoSource(
    const std::filesystem::path& discBin,
    uint32_t requestByteCount) {
    using namespace PrSS0Scene0IntLoadDirect;

    YCompoSource80015618 source{};
    source.ingressKnown = true;
    source.ingressAccepted = true;
    source.discBinPathKnown = true;
    source.discBinPath = discBin;
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
    source.yCompoRow.lengthSourceA1Plus20 = requestByteCount;
    source.yCompoRow.cdlFileNameA1Plus18Known = true;
    FillYCompoName(source.yCompoRow.cdlFileNameA1Plus18);
    return source;
}

PrSS0Scene0IntLoadDirect::Transaction8001AC18 BuildExactInt(
    const std::filesystem::path& discBin) {
    using namespace PrSS0Scene0IntLoadDirect;
    PrStage1LoaderCdHal::Iso9660LookupInput800381F8 lookupInput{};
    lookupInput.valid = true;
    lookupInput.binPath = discBin;
    lookupInput.psxPath = "\\S0\\COMPO00.INT;1";
    lookupInput.cdlFilePtr = kCompoRowPsxAddr8001AC18 + 0x10u;
    lookupInput.pathPtr = kCompoPathPtr8001AC18;
    const auto lookup =
        PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(lookupInput);
    CHECK(lookup.success);

    Source8001AC18 source{};
    source.ingressKnown = true;
    source.ingressAccepted = true;
    source.discBinPathKnown = true;
    source.discBinPath = discBin;
    source.compoRowKnown = true;
    source.compoRow.known = true;
    source.compoRow.psxAddr = kCompoRowPsxAddr8001AC18;
    source.compoRow.tableIndex = kCompoRowTableIndex8001AC18;
    source.compoRow.pathPtrA1Plus00Known = true;
    source.compoRow.pathPtrA1Plus00 = kCompoPathPtr8001AC18;
    source.compoRow.loadedStateA1Plus0CKnown = true;
    source.compoRow.loadedStateA1Plus0C = 1;
    source.compoRow.startMsfKnown = true;
    source.compoRow.startMsf.minute = lookup.feedback.cdlFilePos.minute;
    source.compoRow.startMsf.second = lookup.feedback.cdlFilePos.second;
    source.compoRow.startMsf.frame = lookup.feedback.cdlFilePos.frame;
    source.compoRow.lengthSourceA1Plus20Known = true;
    source.compoRow.lengthSourceA1Plus20 = lookup.matchedSize;
    source.compoRow.cdlFileNameA1Plus18Known = true;
    source.compoRow.cdlFileNameA1Plus18 = lookup.feedback.cdlFileName;
    return BuildTransaction8001AC18(source);
}

std::unique_ptr<PrStage1LoaderMemoryDirectState> MakeResetLoader() {
    auto state = std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*state);
    state->gpPlus320LowWater =
        PrSS0Scene0IntSideEffectDirect::kScene0PacketArenaLowWater801C4260;
    return state;
}

void TestExactLoaderReset80025A34() {
    auto state = std::make_unique<PrStage1LoaderMemoryDirectState>();
    state->gpPlus320LowWater = 0xDEADBEEFu;
    state->gpPlus324StackDepth = 17u;
    state->gpPlus904HeapCursor =
        kPrStage1LoaderMemoryDirectHeapBase800965B0 + 0x40u;
    state->stackTable80091858.fill(0xA5A5A5A5u);

    const auto reset = PrStage1LoaderMemoryDirectReset80025A34(*state);
    CHECK(reset.known);
    CHECK(reset.committed);
    CHECK(reset.function == kPrStage1LoaderMemoryDirectFn80025A34);
    CHECK(reset.tailFunction == kPrStage1LoaderMemoryDirectFn80025A00);
    CHECK(reset.zeroStartAddress == 0x80092854u);
    CHECK(reset.zeroEndAddress ==
          kPrStage1LoaderMemoryDirectStackTable80091858);
    CHECK(reset.zeroDwordCount ==
          kPrStage1LoaderMemoryDirectStackTableEntryCount);
    CHECK(reset.stackTableCleared);
    CHECK(reset.softwareStateCommitted);
    CHECK(!reset.physicalMemoryAuthority);
    CHECK(state->gpPlus320LowWater == 0u);
    CHECK(state->gpPlus324StackDepth == 0u);
    CHECK(state->gpPlus896HeapBase ==
          kPrStage1LoaderMemoryDirectHeapBase800965B0);
    CHECK(state->gpPlus900HeapEnd ==
          kPrStage1LoaderMemoryDirectHeapEnd801C35B0);
    CHECK(state->gpPlus904HeapCursor ==
          kPrStage1LoaderMemoryDirectHeapBase800965B0);
    for (uint32_t value : state->stackTable80091858) {
        CHECK(value == 0u);
    }
}

const PrSS0Scene0IntLoadDirect::EntryMetadata8001A8F0* FindEntry(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& load,
    uint32_t blockIndex,
    uint32_t entryIndex) {
    for (const auto& entry : load.entries) {
        if (entry.blockIndex == blockIndex && entry.entryIndex == entryIndex) {
            return &entry;
        }
    }
    return nullptr;
}

bool CandidateBytesEqual(
    const PrStage1LoaderMemoryDirectState& candidate,
    uint32_t psxAddress,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& load,
    const PrSS0Scene0IntLoadDirect::EntryMetadata8001A8F0& entry) {
    if (entry.dataOffset + entry.size > load.archiveBytes.size()) {
        return false;
    }
    const uint8_t* bytes = PrStage1LoaderMemoryDirectConstPtr(
        candidate, psxAddress, entry.size);
    return bytes != nullptr &&
           std::equal(bytes, bytes + entry.size,
                      load.archiveBytes.begin() + entry.dataOffset);
}

void CheckRejected(
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& tx,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& load) {
    using namespace PrSS0Scene0IntSideEffectDirect;
    CHECK(!tx.accepted);
    CHECK(!tx.loaderMemoryCommitted);
    CHECK(!tx.timVramCommitted);
    CHECK(!tx.vabSpuCommitted);
    CHECK(!tx.memHandleTableCommitted);
    CHECK(!tx.sideEffectsCommitted);
    CHECK(!tx.final8001AC18ResultKnown);
    CHECK(!tx.final8001AC18ResultAuthority);
    CHECK(!IsExactAcceptedTransaction8001A8F0(tx, load));
    CHECK(!IsExactAcceptedYCompoTransaction8001A8F0(tx, load));
}

void TestFailClosed() {
    using namespace PrSS0Scene0IntSideEffectDirect;
    PrSS0Scene0IntLoadDirect::Transaction8001AC18 badLoad{};
    CheckRejected(
        BuildTransaction8001A8F0(badLoad, MakeResetLoader()), badLoad);
}

void TestSyntheticYCompoSideEffect() {
    using namespace PrSS0Scene0IntLoadDirect;
    using namespace PrSS0Scene0IntSideEffectDirect;

    const std::vector<uint8_t> archive = BuildSyntheticYCompoArchive();
    const std::vector<uint8_t> raw = PackMode2RawSectors(archive);
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        ("test_ss0_scene0_ycompo_side_effect_mode2_" +
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

    const auto load = PrSS0Scene0IntLoadDirect::
        BuildYCompoTransaction80015618(
            MakeYCompoSource(path, static_cast<uint32_t>(archive.size())));
    CHECK(PrSS0Scene0IntLoadDirect::
              IsExactAcceptedYCompoTransaction80015618(load));
    Transaction8001A8F0 tx = BuildYCompoTransaction8001A8F0(
        load, MakeResetLoader());
    CHECK(IsExactAcceptedYCompoTransaction8001A8F0(tx, load));
    CHECK(!IsExactAcceptedTransaction8001A8F0(tx, load));
    CHECK(tx.sourceArchiveKind == ArchiveKind8001AC18::PracticeYCompo);
    CHECK(tx.timUploadRequests.size() ==
          kExpectedYCompoTimEntries80015618);
    CHECK(tx.timTemporaryHandle == 1u);
    CHECK(tx.timTemporaryPsxAddress ==
          kPrStage1LoaderMemoryDirectHeapBase800965B0);
    CHECK(tx.timTemporaryAllocationPopped);
    CHECK(tx.vabRequest.known);
    CHECK(tx.vabRequest.vhPsxAddress ==
          kPrStage1LoaderMemoryDirectHeapBase800965B0);
    CHECK(tx.vabRequest.vbPsxAddress ==
          kPrStage1LoaderMemoryDirectHeapBase800965B0 + 8u);
    CHECK(tx.vabVhHandleRetained && tx.vabVbHandlePopped);
    CHECK(tx.vabRequest.closeRequested80027120);
    CHECK(tx.vabRequest.openRequested80027078);
    CHECK(tx.vabRequest.transferRequested800270D4);
    CHECK(tx.vabRequest.enableRequested800270FC);
    CHECK(!tx.memBlockAllocationKnown);
    CHECK(tx.memBlockPsxAddress == 0u);
    CHECK(tx.memBlockBaseHandle == 0u);
    CHECK(tx.memSectorPayloadBytes == 0u);
    CHECK(tx.memMapRequests.empty());
    CHECK(tx.finalStackDepth == 1u);
    CHECK(tx.finalHeapCursor == tx.vabRequest.vbPsxAddress);

    const auto& candidate = *tx.loaderMemoryCandidate;
    CHECK(candidate.gpPlus324StackDepth == 1u);
    CHECK(candidate.gpPlus904HeapCursor == tx.finalHeapCursor);
    CHECK(candidate.stackTable80091858[1u] ==
          tx.vabRequest.vhPsxAddress);
    for (std::size_t index = 2u;
         index < candidate.stackTable80091858.size();
         ++index) {
        CHECK(candidate.stackTable80091858[index] == 0u);
    }
    const auto* vhEntry = FindEntry(load, 1u, 0u);
    const auto* vbEntry = FindEntry(load, 1u, 1u);
    CHECK(vhEntry != nullptr && vbEntry != nullptr);
    if (vhEntry != nullptr && vbEntry != nullptr) {
        CHECK(CandidateBytesEqual(
            candidate, tx.vabRequest.vhPsxAddress, load, *vhEntry));
        CHECK(CandidateBytesEqual(
            candidate, tx.vabRequest.vbPsxAddress, load, *vbEntry));
    }
    CHECK(!tx.loaderMemoryCommitted && !tx.memHandleTableCommitted);
    CHECK(!tx.timVramCommitted && !tx.vabSpuCommitted);
    CHECK(!tx.sideEffectsCommitted && !tx.final8001AC18ResultKnown);
    CHECK(!tx.replayValueAuthority && !tx.oldWinS0Authority);
    CHECK(!tx.hostFilesystemAuthority && !tx.consumerReadAuthority);
    CHECK(!tx.psxMemoryBackingAuthority && !tx.stage2PlusAuthority);

    CHECK(!CommitLoaderMemoryAndHandleTable8001A8F0(tx, load));
    CHECK(IsExactAcceptedYCompoTransaction8001A8F0(tx, load));
    CHECK(!tx.loaderMemoryCommitted && !tx.memHandleTableCommitted);
    CHECK(CommitYCompoLoaderMemory8001A8F0(tx, load));
    CHECK(IsExactYCompoLoaderMemoryCommittedTransaction8001A8F0(
        tx, load));
    CHECK(!IsExactAcceptedYCompoTransaction8001A8F0(tx, load));
    CHECK(tx.loaderMemoryCommitted);
    CHECK(!tx.memHandleTableCommitted);
    CHECK(!tx.timVramCommitted && !tx.vabSpuCommitted);
    CHECK(!tx.sideEffectsCommitted);
    CHECK(!tx.final8001AC18ResultKnown);
    CHECK(!tx.final8001AC18ResultAuthority);
    CHECK(!CommitYCompoLoaderMemory8001A8F0(tx, load));
    CHECK(IsExactYCompoLoaderMemoryCommittedTransaction8001A8F0(
        tx, load));

    FinalResultSource8001AC18 finalSource{};
    finalSource.staticEofReturnChainKnown = true;
    finalSource.eofSuccessKnown = true;
    finalSource.eofSuccess = true;
    finalSource.loaderMemoryCommitted = true;
    finalSource.memHandleTableCommitted = true;
    finalSource.timPartialVramCommitted = true;
    finalSource.hostVabBankCommitted = true;
    CHECK(!CommitFinalResult8001AC18(tx, load, finalSource));
    CHECK(!tx.final8001AC18ResultKnown);
    CHECK(!tx.final8001AC18ResultAuthority);

    auto mutation = Transaction8001A8F0{};
    mutation = BuildYCompoTransaction8001A8F0(load, MakeResetLoader());
    mutation.memBlockAllocationKnown = true;
    CHECK(!IsExactAcceptedYCompoTransaction8001A8F0(mutation, load));
    CHECK(!CommitYCompoLoaderMemory8001A8F0(mutation, load));
    CHECK(!mutation.loaderMemoryCommitted);
    mutation = BuildYCompoTransaction8001A8F0(load, MakeResetLoader());
    mutation.loaderMemoryCandidate->stackTable80091858[2u] = 8u;
    CHECK(!IsExactAcceptedYCompoTransaction8001A8F0(mutation, load));
    CHECK(!CommitYCompoLoaderMemory8001A8F0(mutation, load));
    CHECK(!mutation.loaderMemoryCommitted);
    mutation = BuildYCompoTransaction8001A8F0(load, MakeResetLoader());
    mutation.finalHeapCursor += 8u;
    CHECK(!IsExactAcceptedYCompoTransaction8001A8F0(mutation, load));
    CHECK(!CommitYCompoLoaderMemory8001A8F0(mutation, load));
    CHECK(!mutation.loaderMemoryCommitted);
    CheckRejected(
        BuildTransaction8001A8F0(load, MakeResetLoader()), load);

    std::error_code removeError;
    std::filesystem::remove(path, removeError);
    CHECK(!removeError);
}

void TestExactDisc(const std::filesystem::path& discBin) {
    using namespace PrSS0Scene0IntLoadDirect;
    using namespace PrSS0Scene0IntSideEffectDirect;
    const Transaction8001AC18 load = BuildExactInt(discBin);
    CHECK(IsExactAcceptedTransaction8001AC18(load));
    Transaction8001A8F0 tx =
        BuildTransaction8001A8F0(load, MakeResetLoader());

    CHECK(IsExactAcceptedTransaction8001A8F0(tx, load));
    CHECK(tx.accepted && tx.loaderMemoryCandidateKnown &&
          tx.loaderMemoryCandidate);
    CHECK(tx.timTemporaryHandle == 1u);
    CHECK(tx.timTemporaryPsxAddress ==
          kPrStage1LoaderMemoryDirectHeapBase800965B0);
    CHECK(tx.timTemporaryAllocationPopped);
    CHECK(tx.timUploadRequests.size() == kExpectedTimEntries8001AC18);
    for (std::size_t index = 0u; index < tx.timUploadRequests.size(); ++index) {
        const auto& request = tx.timUploadRequests[index];
        const auto* entry = FindEntry(load, 0u, static_cast<uint32_t>(index));
        CHECK(request.blockIndex == 0u);
        CHECK(request.entryIndex == index);
        CHECK(request.type == BlockType8001A8F0::Tim);
        CHECK(entry != nullptr);
        if (entry != nullptr) {
            CHECK(request.size == entry->size);
            CHECK(request.name == entry->name);
            CHECK(request.archiveDataOffset == entry->dataOffset);
        }
    }

    CHECK(tx.vabRequest.known);
    CHECK(tx.vabRequest.vhPsxAddress ==
          kPrStage1LoaderMemoryDirectHeapBase800965B0);
    CHECK(tx.vabRequest.vbPsxAddress == 0x800971D0u);
    CHECK(tx.vabVhHandleRetained && tx.vabVbHandlePopped);
    CHECK(tx.vabRequest.closeRequested80027120);
    CHECK(tx.vabRequest.openRequested80027078);
    CHECK(tx.vabRequest.transferRequested800270D4);
    CHECK(tx.vabRequest.enableRequested800270FC);
    CHECK(tx.memMapRequests.size() == kExpectedMemEntries8001AC18);
    CHECK(tx.memBlockAllocationKnown);
    CHECK(tx.memBlockBaseHandle == 2u);
    CHECK(tx.memBlockPsxAddress == 0x800971D0u);
    CHECK(tx.finalStackDepth == 60u);
    CHECK(tx.finalHeapCursor == 0x800CE3F8u);

    const auto& candidate = *tx.loaderMemoryCandidate;
    CHECK(candidate.gpPlus324StackDepth == 60u);
    CHECK(candidate.gpPlus904HeapCursor == tx.finalHeapCursor);
    CHECK(candidate.stackTable80091858[1] ==
          tx.vabRequest.vhPsxAddress);
    for (std::size_t i = 0; i < tx.memMapRequests.size(); ++i) {
        const auto& map = tx.memMapRequests[i];
        const auto* entry = FindEntry(load, map.blockIndex, map.entryIndex);
        const uint32_t handle = 2u + static_cast<uint32_t>(i);
        CHECK(map.blockIndex == 2u);
        CHECK(map.type == BlockType8001A8F0::Mem);
        CHECK(candidate.stackTable80091858[handle] == map.psxAddress);
        CHECK(entry != nullptr);
        if (entry != nullptr) {
            CHECK(map.size == entry->size);
            CHECK(map.name == entry->name);
            CHECK(CandidateBytesEqual(candidate, map.psxAddress, load, *entry));
        }
    }

    CHECK(!tx.loaderMemoryCommitted && !tx.timVramCommitted);
    CHECK(!tx.vabSpuCommitted && !tx.memHandleTableCommitted);
    CHECK(!tx.sideEffectsCommitted && !tx.final8001AC18ResultKnown);
    CHECK(!tx.replayValueAuthority && !tx.oldWinS0Authority);
    CHECK(!tx.hostFilesystemAuthority && !tx.consumerReadAuthority);
    CHECK(!tx.psxMemoryBackingAuthority);
    CHECK(!tx.stage2PlusAuthority);

    auto liveState = std::make_unique<PrStage1LoaderMemoryDirectState>(
        *tx.loaderMemoryCandidate);
    CHECK(CommitLoaderMemoryAndHandleTable8001A8F0(tx, load));
    CHECK(tx.loaderMemoryCommitted);
    CHECK(tx.memHandleTableCommitted);
    CHECK(!tx.timVramCommitted && !tx.vabSpuCommitted);
    CHECK(!tx.sideEffectsCommitted && !tx.final8001AC18ResultKnown);
    CHECK(IsExactLoaderMemoryCommittedTransaction8001A8F0(tx, load));
    CHECK(liveState->gpPlus320LowWater ==
          tx.loaderMemoryCandidate->gpPlus320LowWater);
    CHECK(liveState->gpPlus324StackDepth ==
          tx.loaderMemoryCandidate->gpPlus324StackDepth);
    CHECK(liveState->gpPlus904HeapCursor ==
          tx.loaderMemoryCandidate->gpPlus904HeapCursor);
    CHECK(liveState->stackTable80091858 ==
          tx.loaderMemoryCandidate->stackTable80091858);
    CHECK(liveState->heap800965B0 ==
          tx.loaderMemoryCandidate->heap800965B0);
    CHECK(!IsExactAcceptedTransaction8001A8F0(tx, load));

    FinalResultSource8001AC18 finalSource{};
    CHECK(!CommitFinalResult8001AC18(tx, load, finalSource));
    finalSource.staticEofReturnChainKnown = true;
    finalSource.eofSuccessKnown = true;
    finalSource.eofSuccess = true;
    finalSource.loaderMemoryCommitted = true;
    finalSource.memHandleTableCommitted = true;
    finalSource.timPartialVramCommitted = true;
    finalSource.hostVabBankCommitted = true;
    CHECK(CommitFinalResult8001AC18(tx, load, finalSource));
    CHECK(tx.final8001AC18ResultKnown);
    CHECK(tx.final8001AC18Result == 1);
    CHECK(tx.final8001AC18ResultAuthority);
    CHECK(!tx.sideEffectsCommitted);
    CHECK(!tx.timVramCommitted && !tx.vabSpuCommitted);
    CHECK(IsExactFinalResultTransaction8001AC18(tx, load));
    tx.final8001AC18ResultAuthority = false;
    CHECK(!IsExactFinalResultTransaction8001AC18(tx, load));
    tx.final8001AC18ResultAuthority = true;
    CHECK(IsExactFinalResultTransaction8001AC18(tx, load));

    tx.memHandleTableCommitted = false;
    CHECK(!IsExactLoaderMemoryCommittedTransaction8001A8F0(tx, load));

    tx = BuildTransaction8001A8F0(load, MakeResetLoader());
    CHECK(IsExactAcceptedTransaction8001A8F0(tx, load));
    tx.memMapRequests[0].name[0] ^= 1u;
    CHECK(!IsExactAcceptedTransaction8001A8F0(tx, load));
    tx.memMapRequests[0].name[0] ^= 1u;
    tx.loaderMemoryCandidate->stackTable80091858[2u] ^= 8u;
    CHECK(!IsExactAcceptedTransaction8001A8F0(tx, load));
    tx.loaderMemoryCandidate->stackTable80091858[2u] ^= 8u;
    CHECK(IsExactAcceptedTransaction8001A8F0(tx, load));

    auto badReset = MakeResetLoader();
    badReset->gpPlus904HeapCursor += 8u;
    CheckRejected(
        BuildTransaction8001A8F0(load, std::move(badReset)), load);
    CheckRejected(BuildTransaction8001A8F0(load, nullptr), load);
    auto badLoad = load;
    badLoad.accepted = false;
    CheckRejected(
        BuildTransaction8001A8F0(badLoad, MakeResetLoader()), badLoad);
}

}  // namespace

int main(int argc, char** argv) {
    TestExactLoaderReset80025A34();
    TestFailClosed();
    TestSyntheticYCompoSideEffect();
    const bool exactDisc = argc >= 2;
    if (exactDisc) {
        TestExactDisc(std::filesystem::path(argv[1]));
    }
    if (g_failures != 0) {
        return 1;
    }
    std::printf("test_ss0_scene0_int_side_effect_direct: ok exactDisc=%d\n",
                exactDisc ? 1 : 0);
    return 0;
}
