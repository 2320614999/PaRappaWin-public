#pragma once

#include "pr_ss0_scene0_int_side_effect_direct.h"
#include "pr_stage1_loader_gpu_hal.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace PrSS0Scene0IntGpuDirect {

constexpr uint32_t kVramWidth8001AE7C = 1024u;
constexpr uint32_t kVramHeight8001AE7C = 512u;
constexpr std::size_t kVramWordCount8001AE7C =
    static_cast<std::size_t>(kVramWidth8001AE7C) *
    kVramHeight8001AE7C;

enum class Status8001AE7C : uint8_t {
    SourceUnknown = 0,
    IntLoadRejected,
    LoaderCandidateRejected,
    TimRequestMalformed,
    TimParseRejected,
    GpuActionMalformed,
    VramWriteOutOfRange,
    CandidateShapeMismatch,
    Accepted,
};

struct VramWrite8001AE7C {
    PrStage1LoaderGpuHal::GpuUploadSection section =
        PrStage1LoaderGpuHal::GpuUploadSection::None;
    PrStage1LoaderGpuHal::PsxRect rect{};
    uint32_t sourceOffset = 0u;
    uint32_t sourceBytes = 0u;
    uint32_t sourcePsxAddress = 0u;
    bool sourcePsxAddressKnown = false;
    uint32_t writtenWords = 0u;
};

struct TimUpload8001AE7C {
    uint32_t requestIndex = 0u;
    uint32_t sourcePsxAddress = 0u;
    uint32_t sourceBytes = 0u;
    std::array<uint8_t, 16> name{};
    bool hasClut = false;
    uint32_t gpuActionCount = 0u;
    uint32_t loadImageCount = 0u;
    uint32_t drawSyncCount = 0u;
    std::array<VramWrite8001AE7C, 2> writes{};
};

struct PartialVramState8001AE7C {
    std::array<uint16_t, kVramWordCount8001AE7C> words{};
    std::array<uint8_t, kVramWordCount8001AE7C> known{};
    uint32_t knownWordCount = 0u;
    uint64_t totalWriteWordCount = 0u;
    uint32_t committedBatchCount = 0u;
};

struct Transaction8001AE7C {
    Status8001AE7C status = Status8001AE7C::SourceUnknown;
    bool accepted = false;
    bool complete = false;
    bool sourceIntLoadKnown = false;
    bool sourceLoaderCandidateKnown = false;
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 sourceArchiveKind =
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Unknown;
    uint32_t sourceTimRequestCount = 0u;

    std::vector<TimUpload8001AE7C> timUploads{};
    uint32_t totalGpuActionCount = 0u;
    uint32_t totalLoadImageCount = 0u;
    uint32_t totalDrawSyncCount = 0u;
    uint64_t totalWriteWordCount = 0u;
    uint32_t finalKnownWordCount = 0u;

    bool partialVramCandidateKnown = false;
    std::unique_ptr<PartialVramState8001AE7C> partialVramCandidate{};
    bool directVramCommitted = false;
    bool partialVramWordAuthority = false;
    bool fullVramBackingAuthority = false;
    bool rendererAtlasCommitted = false;
    bool vabSpuCommitted = false;
    bool final8001AC18ResultKnown = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;
    bool consumerReadAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

Transaction8001AE7C BuildTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool IsExactPreparedTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

Transaction8001AE7C BuildYCompoTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

Transaction8001AE7C BuildStartupCommonTransaction80016B84(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

Transaction8001AE7C BuildStartupZCompoTransaction80015590(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool IsExactPreparedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool IsExactPreparedStartupCommonTransaction80016B84(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool IsExactPreparedStartupZCompoTransaction80015590(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool CommitPreparedYCompoTransaction8001AE7C(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool IsExactCommittedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool CommitPreparedStartupCommonTransaction80016B84(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool CommitPreparedStartupZCompoTransaction80015590(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool IsExactCommittedStartupCommonTransaction80016B84(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool IsExactCommittedStartupZCompoTransaction80015590(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool CommitPreparedTransaction8001AE7C(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

bool IsExactCommittedTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate);

}  // namespace PrSS0Scene0IntGpuDirect
