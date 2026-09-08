#pragma once

#include "pr_ss0_scene0_int_gpu_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace PrSS0Scene0IntRendererDirect {

enum class Status8001AE7C : uint8_t {
    SourceUnknown = 0,
    IntLoadRejected,
    LoaderCandidateRejected,
    GpuTransactionRejected,
    TimRequestMalformed,
    CandidateShapeMismatch,
    Accepted,
};

struct TimProjectionRequest8001AE7C {
    uint32_t requestIndex = 0u;
    uint64_t archiveDataOffset = 0u;
    uint32_t sourcePsxAddress = 0u;
    uint32_t sourceBytes = 0u;
    std::array<uint8_t, 16> name{};
    bool hasClut = false;
};

struct Transaction8001AE7C {
    Status8001AE7C status = Status8001AE7C::SourceUnknown;
    bool accepted = false;
    bool complete = false;
    bool sourceIntLoadKnown = false;
    bool sourceLoaderCandidateKnown = false;
    bool sourceGpuTransactionKnown = false;
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 sourceArchiveKind =
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Unknown;
    bool sourcePartialVramAuthority = false;
    uint32_t sourceTimRequestCount = 0u;
    std::vector<TimProjectionRequest8001AE7C> timProjectionRequests{};

    bool requestBatchAuthority = false;
    bool cpuAtlasCandidatePrepared = false;
    bool cpuAtlasCommitted = false;
    bool d3dUploadCommitted = false;
    bool fullVramBackingAuthority = false;
    bool replayValueAuthority = false;
    bool hostFilesystemAuthority = false;
    bool consumerReadAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
};

Transaction8001AE7C BuildTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);

bool IsExactPreparedTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);

bool CommitCpuAtlasProjection8001AE7C(Transaction8001AE7C& transaction);

bool IsExactCommittedTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);

Transaction8001AE7C BuildYCompoTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);

bool IsExactPreparedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);

bool CommitYCompoCpuAtlasProjection8001AE7C(
    Transaction8001AE7C& transaction);

bool IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);

bool CommitYCompoD3DAtlasProjection8001AE7C(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);

bool IsExactCommittedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction);

}  // namespace PrSS0Scene0IntRendererDirect
