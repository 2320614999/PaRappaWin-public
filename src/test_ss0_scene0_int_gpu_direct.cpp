#include "pr/pr_ss0_scene0_int_gpu_direct.h"
#include "pr/pr_ss0_scene0_int_renderer_direct.h"

#include "pr/pr_stage1_loader_cd_hal.h"

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

void WriteU16Le(std::vector<uint8_t>& bytes,
                std::size_t offset,
                uint16_t value) {
    bytes[offset + 0u] = static_cast<uint8_t>(value & 0xFFu);
    bytes[offset + 1u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
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

void WriteMinimalClutTim(std::vector<uint8_t>& bytes,
                         std::size_t offset,
                         uint16_t x,
                         uint16_t pixelValue) {
    WriteU32Le(bytes, offset + 0u, 0x10u);
    WriteU32Le(bytes, offset + 4u, 0x08u);
    WriteU32Le(bytes, offset + 8u, 14u);
    WriteU16Le(bytes, offset + 12u, x);
    WriteU16Le(bytes, offset + 14u, 0u);
    WriteU16Le(bytes, offset + 16u, 1u);
    WriteU16Le(bytes, offset + 18u, 1u);
    WriteU16Le(bytes, offset + 20u, static_cast<uint16_t>(0x4000u + x));
    WriteU32Le(bytes, offset + 22u, 14u);
    WriteU16Le(bytes, offset + 26u, x);
    WriteU16Le(bytes, offset + 28u, 1u);
    WriteU16Le(bytes, offset + 30u, 1u);
    WriteU16Le(bytes, offset + 32u, 1u);
    WriteU16Le(bytes, offset + 34u, pixelValue);
}

std::vector<uint8_t> BuildSyntheticYCompoGpuArchive() {
    using namespace PrSS0Scene0IntLoadDirect;

    constexpr uint32_t kTimBytes = 36u;
    constexpr uint32_t kTimSectors = 2u;
    constexpr std::size_t kTimBlockBytes =
        kHeaderBytes8001A8F0 +
        static_cast<std::size_t>(kTimSectors) * kSectorBytes8001A8F0;
    constexpr std::size_t kVabOffset = kTimBlockBytes;
    constexpr std::size_t kVabBlockBytes =
        kHeaderBytes8001A8F0 + kSectorBytes8001A8F0;
    constexpr std::size_t kEofOffset = kVabOffset + kVabBlockBytes;
    std::vector<uint8_t> bytes(kEofOffset + kHeaderBytes8001A8F0, 0u);

    WriteU32Le(bytes, 0u, 1u);
    WriteU32Le(bytes, 4u, kExpectedYCompoTimEntries80015618);
    WriteU32Le(bytes, 8u, kTimSectors);
    std::size_t timDataOffset = kHeaderBytes8001A8F0;
    for (uint32_t index = 0u;
         index < kExpectedYCompoTimEntries80015618;
         ++index) {
        char name[16]{};
        std::snprintf(name, sizeof(name), "T%03u.TIM", index);
        WriteArchiveEntry(bytes, 0u, index, kTimBytes, name);
        WriteMinimalClutTim(
            bytes,
            timDataOffset,
            static_cast<uint16_t>(index),
            static_cast<uint16_t>(0x100u + index));
        timDataOffset += kTimBytes;
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

    WriteU32Le(bytes, kEofOffset, 0xFFFFFFFFu);
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
    constexpr char kName[] = "YCOMPO.INT;1";
    for (std::size_t index = 0u; index < sizeof(kName) - 1u; ++index) {
        source.yCompoRow.cdlFileNameA1Plus18[index] =
            static_cast<uint8_t>(kName[index]);
    }
    return source;
}

std::unique_ptr<PrStage1LoaderMemoryDirectState> MakeResetLoader() {
    auto reset = std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*reset);
    reset->gpPlus320LowWater =
        PrSS0Scene0IntSideEffectDirect::kScene0PacketArenaLowWater801C4260;
    return reset;
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

PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0 BuildLoaderCandidate(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& load) {
    auto reset = std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*reset);
    reset->gpPlus320LowWater =
        PrSS0Scene0IntSideEffectDirect::
            kScene0PacketArenaLowWater801C4260;
    return PrSS0Scene0IntSideEffectDirect::BuildTransaction8001A8F0(
        load, std::move(reset));
}

void CheckRejected(
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& load,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    using namespace PrSS0Scene0IntGpuDirect;
    CHECK(!transaction.accepted);
    CHECK(!transaction.directVramCommitted);
    CHECK(!transaction.partialVramWordAuthority);
    CHECK(!transaction.fullVramBackingAuthority);
    CHECK(!transaction.rendererAtlasCommitted);
    CHECK(!transaction.vabSpuCommitted);
    CHECK(!transaction.final8001AC18ResultKnown);
    CHECK(!IsExactPreparedTransaction8001AE7C(
        transaction, load, loaderCandidate));
    CHECK(!IsExactCommittedTransaction8001AE7C(
        transaction, load, loaderCandidate));
    CHECK(!IsExactPreparedYCompoTransaction8001AE7C(
        transaction, load, loaderCandidate));
}

void TestFailClosed() {
    using namespace PrSS0Scene0IntGpuDirect;
    PrSS0Scene0IntLoadDirect::Transaction8001AC18 load{};
    PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0 loader{};
    CheckRejected(BuildTransaction8001AE7C(load, loader), load, loader);
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C gpu{};
    const auto renderer =
        PrSS0Scene0IntRendererDirect::BuildTransaction8001AE7C(
            load, loader, gpu);
    CHECK(!renderer.accepted);
    CHECK(!renderer.cpuAtlasCommitted);
}

void TestSyntheticYCompoGpuCandidate() {
    using namespace PrSS0Scene0IntGpuDirect;
    using namespace PrSS0Scene0IntLoadDirect;

    const std::vector<uint8_t> archive = BuildSyntheticYCompoGpuArchive();
    const std::vector<uint8_t> raw = PackMode2RawSectors(archive);
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        ("test_ss0_scene0_ycompo_gpu_mode2_" +
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

    const auto load = BuildYCompoTransaction80015618(
        MakeYCompoSource(path, static_cast<uint32_t>(archive.size())));
    const auto loader =
        PrSS0Scene0IntSideEffectDirect::BuildYCompoTransaction8001A8F0(
            load, MakeResetLoader());
    CHECK(IsExactAcceptedYCompoTransaction80015618(load));
    CHECK(PrSS0Scene0IntSideEffectDirect::
              IsExactAcceptedYCompoTransaction8001A8F0(loader, load));

    Transaction8001AE7C transaction =
        BuildYCompoTransaction8001AE7C(load, loader);
    CHECK(IsExactPreparedYCompoTransaction8001AE7C(
        transaction, load, loader));
    CHECK(!IsExactPreparedTransaction8001AE7C(
        transaction, load, loader));
    CHECK(transaction.sourceArchiveKind ==
          ArchiveKind8001AC18::PracticeYCompo);
    CHECK(transaction.sourceTimRequestCount ==
          kExpectedYCompoTimEntries80015618);
    CHECK(transaction.timUploads.size() ==
          kExpectedYCompoTimEntries80015618);
    CHECK(transaction.totalGpuActionCount == 455u);
    CHECK(transaction.totalLoadImageCount == 182u);
    CHECK(transaction.totalDrawSyncCount == 182u);
    CHECK(transaction.finalKnownWordCount == 182u);
    CHECK(transaction.totalWriteWordCount == 182u);
    for (const auto& upload : transaction.timUploads) {
        CHECK(upload.hasClut);
        CHECK(upload.gpuActionCount == 5u);
        CHECK(upload.loadImageCount == 2u);
        CHECK(upload.drawSyncCount == 2u);
    }
    CHECK(transaction.partialVramCandidateKnown);
    CHECK(transaction.partialVramCandidate != nullptr);
    CHECK(transaction.partialVramCandidate->committedBatchCount == 0u);
    CHECK(!transaction.directVramCommitted);
    CHECK(!transaction.partialVramWordAuthority);
    CHECK(!transaction.fullVramBackingAuthority);
    CHECK(!transaction.rendererAtlasCommitted);
    CHECK(!transaction.vabSpuCommitted);
    CHECK(!transaction.final8001AC18ResultKnown);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.hostFilesystemAuthority);
    CHECK(!transaction.consumerReadAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.stage2PlusAuthority);
    CHECK(!CommitPreparedTransaction8001AE7C(
        transaction, load, loader));
    CHECK(!IsExactCommittedTransaction8001AE7C(
        transaction, load, loader));
    CHECK(IsExactPreparedYCompoTransaction8001AE7C(
        transaction, load, loader));
    CHECK(!transaction.directVramCommitted);
    CHECK(!transaction.partialVramWordAuthority);
    CHECK(transaction.partialVramCandidate->committedBatchCount == 0u);

    Transaction8001AE7C commitMutation =
        BuildYCompoTransaction8001AE7C(load, loader);
    ++commitMutation.sourceTimRequestCount;
    CHECK(!CommitPreparedYCompoTransaction8001AE7C(
        commitMutation, load, loader));
    CHECK(!commitMutation.directVramCommitted);
    CHECK(!commitMutation.partialVramWordAuthority);
    CHECK(commitMutation.partialVramCandidate->committedBatchCount == 0u);

    transaction.sourceArchiveKind = ArchiveKind8001AC18::Scene0Compo00;
    CHECK(!IsExactPreparedYCompoTransaction8001AE7C(
        transaction, load, loader));
    transaction.sourceArchiveKind = ArchiveKind8001AC18::PracticeYCompo;
    const auto firstKnown = std::find(
        transaction.partialVramCandidate->known.begin(),
        transaction.partialVramCandidate->known.end(),
        static_cast<uint8_t>(1u));
    CHECK(firstKnown != transaction.partialVramCandidate->known.end());
    if (firstKnown != transaction.partialVramCandidate->known.end()) {
        const std::size_t offset = static_cast<std::size_t>(
            firstKnown - transaction.partialVramCandidate->known.begin());
        transaction.partialVramCandidate->words[offset] ^= 1u;
        CHECK(!IsExactPreparedYCompoTransaction8001AE7C(
            transaction, load, loader));
        transaction.partialVramCandidate->words[offset] ^= 1u;
    }
    CHECK(IsExactPreparedYCompoTransaction8001AE7C(
        transaction, load, loader));
    CHECK(CommitPreparedYCompoTransaction8001AE7C(
        transaction, load, loader));
    CHECK(IsExactCommittedYCompoTransaction8001AE7C(
        transaction, load, loader));
    CHECK(!IsExactPreparedYCompoTransaction8001AE7C(
        transaction, load, loader));
    CHECK(!IsExactCommittedTransaction8001AE7C(
        transaction, load, loader));
    CHECK(transaction.directVramCommitted);
    CHECK(transaction.partialVramWordAuthority);
    CHECK(transaction.partialVramCandidate->committedBatchCount == 1u);
    CHECK(!transaction.fullVramBackingAuthority);
    CHECK(!transaction.rendererAtlasCommitted);
    CHECK(!CommitPreparedYCompoTransaction8001AE7C(
        transaction, load, loader));
    CHECK(IsExactCommittedYCompoTransaction8001AE7C(
        transaction, load, loader));

    auto renderer = PrSS0Scene0IntRendererDirect::
        BuildYCompoTransaction8001AE7C(
            load, loader, transaction);
    CHECK(PrSS0Scene0IntRendererDirect::
        IsExactPreparedYCompoTransaction8001AE7C(
            renderer, load, loader, transaction));
    CHECK(!PrSS0Scene0IntRendererDirect::
        IsExactPreparedTransaction8001AE7C(
            renderer, load, loader, transaction));
    CHECK(renderer.sourceArchiveKind ==
          ArchiveKind8001AC18::PracticeYCompo);
    CHECK(renderer.sourceTimRequestCount ==
          kExpectedYCompoTimEntries80015618);
    CHECK(renderer.timProjectionRequests.size() ==
          kExpectedYCompoTimEntries80015618);
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            renderer, load, loader, transaction));
    CHECK(!renderer.d3dUploadCommitted);
    auto legacyRendererCommit = renderer;
    legacyRendererCommit.cpuAtlasCandidatePrepared = true;
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitCpuAtlasProjection8001AE7C(legacyRendererCommit));
    CHECK(!legacyRendererCommit.cpuAtlasCommitted);
    renderer.cpuAtlasCandidatePrepared = true;
    CHECK(PrSS0Scene0IntRendererDirect::
        CommitYCompoCpuAtlasProjection8001AE7C(renderer));
    CHECK(PrSS0Scene0IntRendererDirect::
        IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
            renderer, load, loader, transaction));
    CHECK(!PrSS0Scene0IntRendererDirect::
        IsExactCommittedYCompoTransaction8001AE7C(
            renderer, load, loader, transaction));
    CHECK(!PrSS0Scene0IntRendererDirect::
        IsExactCommittedTransaction8001AE7C(
            renderer, load, loader, transaction));
    CHECK(renderer.cpuAtlasCommitted);
    CHECK(!renderer.d3dUploadCommitted);
    CHECK(!renderer.fullVramBackingAuthority);
    CHECK(!renderer.replayValueAuthority);
    CHECK(!renderer.hostFilesystemAuthority);
    CHECK(!renderer.consumerReadAuthority);
    CHECK(!renderer.oldWinS0Authority);
    CHECK(!renderer.stage2PlusAuthority);
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoCpuAtlasProjection8001AE7C(renderer));
    CHECK(PrSS0Scene0IntRendererDirect::
        IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
            renderer, load, loader, transaction));

    auto mutationRenderer = renderer;
    CHECK(!mutationRenderer.timProjectionRequests.empty());
    if (!mutationRenderer.timProjectionRequests.empty()) {
        ++mutationRenderer.timProjectionRequests[0].sourceBytes;
        CHECK(!PrSS0Scene0IntRendererDirect::
            IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
                mutationRenderer, load, loader, transaction));
        CHECK(!PrSS0Scene0IntRendererDirect::
            CommitYCompoD3DAtlasProjection8001AE7C(
                mutationRenderer, load, loader, transaction));
        CHECK(!mutationRenderer.d3dUploadCommitted);
    }

    auto oldCompoRenderer = renderer;
    oldCompoRenderer.sourceArchiveKind = ArchiveKind8001AC18::Scene0Compo00;
    CHECK(!PrSS0Scene0IntRendererDirect::
        IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
            oldCompoRenderer, load, loader, transaction));
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            oldCompoRenderer, load, loader, transaction));
    CHECK(!oldCompoRenderer.d3dUploadCommitted);

    auto fullVramRenderer = renderer;
    fullVramRenderer.fullVramBackingAuthority = true;
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            fullVramRenderer, load, loader, transaction));
    CHECK(!fullVramRenderer.d3dUploadCommitted);

    auto replayRenderer = renderer;
    replayRenderer.replayValueAuthority = true;
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            replayRenderer, load, loader, transaction));
    CHECK(!replayRenderer.d3dUploadCommitted);

    auto hostRenderer = renderer;
    hostRenderer.hostFilesystemAuthority = true;
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            hostRenderer, load, loader, transaction));
    CHECK(!hostRenderer.d3dUploadCommitted);

    auto consumerRenderer = renderer;
    consumerRenderer.consumerReadAuthority = true;
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            consumerRenderer, load, loader, transaction));
    CHECK(!consumerRenderer.d3dUploadCommitted);

    auto oldS0Renderer = renderer;
    oldS0Renderer.oldWinS0Authority = true;
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            oldS0Renderer, load, loader, transaction));
    CHECK(!oldS0Renderer.d3dUploadCommitted);

    auto stage2PlusRenderer = renderer;
    stage2PlusRenderer.stage2PlusAuthority = true;
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            stage2PlusRenderer, load, loader, transaction));
    CHECK(!stage2PlusRenderer.d3dUploadCommitted);

    CHECK(PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            renderer, load, loader, transaction));
    CHECK(renderer.d3dUploadCommitted);
    CHECK(!PrSS0Scene0IntRendererDirect::
        IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
            renderer, load, loader, transaction));
    CHECK(PrSS0Scene0IntRendererDirect::
        IsExactCommittedYCompoTransaction8001AE7C(
            renderer, load, loader, transaction));
    auto duplicateD3DRenderer = renderer;
    CHECK(!PrSS0Scene0IntRendererDirect::
        CommitYCompoD3DAtlasProjection8001AE7C(
            duplicateD3DRenderer, load, loader, transaction));
    CHECK(PrSS0Scene0IntRendererDirect::
        IsExactCommittedYCompoTransaction8001AE7C(
            duplicateD3DRenderer, load, loader, transaction));

    transaction.fullVramBackingAuthority = true;
    CHECK(!IsExactCommittedYCompoTransaction8001AE7C(
        transaction, load, loader));
    transaction.fullVramBackingAuthority = false;
    CHECK(IsExactCommittedYCompoTransaction8001AE7C(
        transaction, load, loader));
    CheckRejected(BuildTransaction8001AE7C(load, loader), load, loader);

    std::error_code removeError;
    std::filesystem::remove(path, removeError);
    CHECK(!removeError);
}

void TestExactDisc(const std::filesystem::path& discBin) {
    using namespace PrSS0Scene0IntGpuDirect;
    const auto load = BuildExactInt(discBin);
    const auto loader = BuildLoaderCandidate(load);
    CHECK(PrSS0Scene0IntLoadDirect::
              IsExactAcceptedTransaction8001AC18(load));
    CHECK(PrSS0Scene0IntSideEffectDirect::
              IsExactAcceptedTransaction8001A8F0(loader, load));

    Transaction8001AE7C transaction =
        BuildTransaction8001AE7C(load, loader);
    CHECK(IsExactPreparedTransaction8001AE7C(
        transaction, load, loader));
    CHECK(transaction.partialVramCandidateKnown);
    CHECK(transaction.partialVramCandidate != nullptr);
    CHECK(transaction.sourceTimRequestCount ==
          PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);
    CHECK(transaction.timUploads.size() ==
          PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);
    CHECK(transaction.totalGpuActionCount == 435u);
    CHECK(transaction.totalLoadImageCount == 174u);
    CHECK(transaction.totalDrawSyncCount == 174u);
    CHECK(transaction.finalKnownWordCount > 0u);
    CHECK(transaction.totalWriteWordCount >=
          transaction.finalKnownWordCount);

    for (std::size_t index = 0u;
         index < transaction.timUploads.size();
         ++index) {
        const auto& upload = transaction.timUploads[index];
        CHECK(upload.requestIndex == index);
        CHECK(upload.hasClut);
        CHECK(upload.gpuActionCount == 5u);
        CHECK(upload.loadImageCount == 2u);
        CHECK(upload.drawSyncCount == 2u);
        CHECK(upload.writes[0].section ==
              PrStage1LoaderGpuHal::GpuUploadSection::Pixel);
        CHECK(upload.writes[1].section ==
              PrStage1LoaderGpuHal::GpuUploadSection::Clut);
        CHECK(upload.writes[0].writtenWords > 0u);
        CHECK(upload.writes[1].writtenWords > 0u);
    }

    const auto& vram = *transaction.partialVramCandidate;
    const uint32_t countedKnown = static_cast<uint32_t>(std::count(
        vram.known.begin(), vram.known.end(), static_cast<uint8_t>(1u)));
    CHECK(countedKnown == transaction.finalKnownWordCount);
    CHECK(vram.totalWriteWordCount == transaction.totalWriteWordCount);
    CHECK(vram.committedBatchCount == 0u);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.hostFilesystemAuthority);
    CHECK(!transaction.consumerReadAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.stage2PlusAuthority);

    CHECK(CommitPreparedTransaction8001AE7C(
        transaction, load, loader));
    CHECK(IsExactCommittedTransaction8001AE7C(
        transaction, load, loader));
    CHECK(transaction.directVramCommitted);
    CHECK(transaction.partialVramWordAuthority);
    CHECK(!transaction.fullVramBackingAuthority);
    CHECK(!transaction.rendererAtlasCommitted);
    CHECK(transaction.partialVramCandidate->committedBatchCount == 1u);

    auto renderer =
        PrSS0Scene0IntRendererDirect::BuildTransaction8001AE7C(
            load, loader, transaction);
    CHECK(PrSS0Scene0IntRendererDirect::
              IsExactPreparedTransaction8001AE7C(
                  renderer, load, loader, transaction));
    CHECK(renderer.sourcePartialVramAuthority);
    CHECK(renderer.requestBatchAuthority);
    CHECK(renderer.timProjectionRequests.size() ==
          PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);
    CHECK(!renderer.cpuAtlasCandidatePrepared);
    CHECK(!renderer.cpuAtlasCommitted);
    CHECK(!renderer.d3dUploadCommitted);
    CHECK(!renderer.fullVramBackingAuthority);
    renderer.cpuAtlasCandidatePrepared = true;
    CHECK(PrSS0Scene0IntRendererDirect::
              CommitCpuAtlasProjection8001AE7C(renderer));
    CHECK(PrSS0Scene0IntRendererDirect::
              IsExactCommittedTransaction8001AE7C(
                  renderer, load, loader, transaction));
    CHECK(renderer.cpuAtlasCommitted);
    CHECK(!renderer.d3dUploadCommitted);

    transaction.fullVramBackingAuthority = true;
    CHECK(!IsExactCommittedTransaction8001AE7C(
        transaction, load, loader));
    transaction.fullVramBackingAuthority = false;

    const auto firstKnown = std::find(
        transaction.partialVramCandidate->known.begin(),
        transaction.partialVramCandidate->known.end(),
        static_cast<uint8_t>(1u));
    CHECK(firstKnown != transaction.partialVramCandidate->known.end());
    if (firstKnown != transaction.partialVramCandidate->known.end()) {
        const std::size_t offset = static_cast<std::size_t>(
            firstKnown - transaction.partialVramCandidate->known.begin());
        transaction.partialVramCandidate->words[offset] ^= 1u;
        CHECK(!IsExactCommittedTransaction8001AE7C(
            transaction, load, loader));
        transaction.partialVramCandidate->words[offset] ^= 1u;
    }
    CHECK(IsExactCommittedTransaction8001AE7C(
        transaction, load, loader));
}

}  // namespace

int main(int argc, char** argv) {
    TestFailClosed();
    TestSyntheticYCompoGpuCandidate();
    const bool exactDisc = argc >= 2;
    if (exactDisc) {
        TestExactDisc(std::filesystem::path(argv[1]));
    }
    if (g_failures != 0) {
        return 1;
    }
    std::printf(
        "test_ss0_scene0_int_gpu_direct: ok exactDisc=%d\n",
        exactDisc ? 1 : 0);
    return 0;
}
