#include "pr_ss0_scene0_int_renderer_direct.h"

namespace PrSS0Scene0IntRendererDirect {
namespace {

bool IsSourceRange8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    uint64_t offset,
    uint64_t size) {
    return offset <= intLoad.archiveBytes.size() &&
           size <= intLoad.archiveBytes.size() - offset;
}

bool EqualRequest8001AE7C(
    const TimProjectionRequest8001AE7C& left,
    const TimProjectionRequest8001AE7C& right) {
    return left.requestIndex == right.requestIndex &&
           left.archiveDataOffset == right.archiveDataOffset &&
           left.sourcePsxAddress == right.sourcePsxAddress &&
           left.sourceBytes == right.sourceBytes && left.name == right.name &&
           left.hasClut == right.hasClut;
}

bool EqualTransaction8001AE7C(
    const Transaction8001AE7C& left,
    const Transaction8001AE7C& right) {
    if (left.status != right.status || left.accepted != right.accepted ||
        left.complete != right.complete ||
        left.sourceIntLoadKnown != right.sourceIntLoadKnown ||
        left.sourceLoaderCandidateKnown != right.sourceLoaderCandidateKnown ||
        left.sourceGpuTransactionKnown != right.sourceGpuTransactionKnown ||
        left.sourceArchiveKind != right.sourceArchiveKind ||
        left.sourcePartialVramAuthority !=
            right.sourcePartialVramAuthority ||
        left.sourceTimRequestCount != right.sourceTimRequestCount ||
        left.timProjectionRequests.size() !=
            right.timProjectionRequests.size() ||
        left.requestBatchAuthority != right.requestBatchAuthority ||
        left.cpuAtlasCandidatePrepared !=
            right.cpuAtlasCandidatePrepared ||
        left.cpuAtlasCommitted != right.cpuAtlasCommitted ||
        left.d3dUploadCommitted != right.d3dUploadCommitted ||
        left.fullVramBackingAuthority != right.fullVramBackingAuthority ||
        left.replayValueAuthority != right.replayValueAuthority ||
        left.hostFilesystemAuthority != right.hostFilesystemAuthority ||
        left.consumerReadAuthority != right.consumerReadAuthority ||
        left.oldWinS0Authority != right.oldWinS0Authority ||
        left.stage2PlusAuthority != right.stage2PlusAuthority) {
        return false;
    }
    for (std::size_t index = 0u;
         index < left.timProjectionRequests.size(); ++index) {
        if (!EqualRequest8001AE7C(left.timProjectionRequests[index],
                                  right.timProjectionRequests[index])) {
            return false;
        }
    }
    return true;
}

using ArchiveKind8001AC18 =
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18;

bool IsExactIntLoadForProfile8001AE7C(
    ArchiveKind8001AC18 profile,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad) {
    if (intLoad.archiveKind != profile) {
        return false;
    }
    if (profile == ArchiveKind8001AC18::Scene0Compo00) {
        return PrSS0Scene0IntLoadDirect::
            IsExactAcceptedTransaction8001AC18(intLoad);
    }
    if (profile == ArchiveKind8001AC18::PracticeYCompo) {
        return PrSS0Scene0IntLoadDirect::
            IsExactAcceptedYCompoTransaction80015618(intLoad);
    }
    return false;
}

bool IsExactLoaderForProfile8001AE7C(
    ArchiveKind8001AC18 profile,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad) {
    if (profile == ArchiveKind8001AC18::Scene0Compo00) {
        return PrSS0Scene0IntSideEffectDirect::
            IsExactAcceptedTransaction8001A8F0(loaderCandidate, intLoad);
    }
    if (profile == ArchiveKind8001AC18::PracticeYCompo) {
        return PrSS0Scene0IntSideEffectDirect::
            IsExactAcceptedYCompoTransaction8001A8F0(
                loaderCandidate, intLoad);
    }
    return false;
}

bool IsExactCommittedGpuForProfile8001AE7C(
    ArchiveKind8001AC18 profile,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    if (gpuTransaction.sourceArchiveKind != profile) {
        return false;
    }
    if (profile == ArchiveKind8001AC18::Scene0Compo00) {
        return PrSS0Scene0IntGpuDirect::
            IsExactCommittedTransaction8001AE7C(
                gpuTransaction, intLoad, loaderCandidate);
    }
    if (profile == ArchiveKind8001AC18::PracticeYCompo) {
        return PrSS0Scene0IntGpuDirect::
            IsExactCommittedYCompoTransaction8001AE7C(
                gpuTransaction, intLoad, loaderCandidate);
    }
    return false;
}

Transaction8001AE7C BuildProfileTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction,
    ArchiveKind8001AC18 profile,
    uint32_t expectedTimRequestCount) {
    Transaction8001AE7C transaction{};
    transaction.sourceIntLoadKnown = true;
    transaction.sourceArchiveKind = intLoad.archiveKind;
    if (!IsExactIntLoadForProfile8001AE7C(profile, intLoad)) {
        transaction.status = Status8001AE7C::IntLoadRejected;
        return transaction;
    }
    transaction.sourceLoaderCandidateKnown = true;
    if (!IsExactLoaderForProfile8001AE7C(
            profile, loaderCandidate, intLoad)) {
        transaction.status = Status8001AE7C::LoaderCandidateRejected;
        return transaction;
    }
    transaction.sourceGpuTransactionKnown = true;
    if (!IsExactCommittedGpuForProfile8001AE7C(
            profile, gpuTransaction, intLoad, loaderCandidate)) {
        transaction.status = Status8001AE7C::GpuTransactionRejected;
        return transaction;
    }

    transaction.sourcePartialVramAuthority =
        gpuTransaction.directVramCommitted &&
        gpuTransaction.partialVramWordAuthority &&
        !gpuTransaction.fullVramBackingAuthority;
    transaction.sourceTimRequestCount =
        static_cast<uint32_t>(loaderCandidate.timUploadRequests.size());
    if (!transaction.sourcePartialVramAuthority ||
        transaction.sourceTimRequestCount != expectedTimRequestCount ||
        gpuTransaction.timUploads.size() !=
            loaderCandidate.timUploadRequests.size()) {
        transaction.status = Status8001AE7C::CandidateShapeMismatch;
        return transaction;
    }

    transaction.timProjectionRequests.reserve(
        transaction.sourceTimRequestCount);
    for (uint32_t index = 0u;
         index < transaction.sourceTimRequestCount; ++index) {
        const auto& request = loaderCandidate.timUploadRequests[index];
        const auto& upload = gpuTransaction.timUploads[index];
        if (request.blockIndex != 0u || request.entryIndex != index ||
            request.type !=
                PrSS0Scene0IntLoadDirect::BlockType8001A8F0::Tim ||
            upload.requestIndex != index ||
            upload.sourcePsxAddress != request.psxAddress ||
            upload.sourceBytes != request.size ||
            upload.name != request.name || !upload.hasClut ||
            !IsSourceRange8001AE7C(
                intLoad, request.archiveDataOffset, request.size)) {
            transaction.status = Status8001AE7C::TimRequestMalformed;
            return transaction;
        }

        TimProjectionRequest8001AE7C projection{};
        projection.requestIndex = index;
        projection.archiveDataOffset = request.archiveDataOffset;
        projection.sourcePsxAddress = request.psxAddress;
        projection.sourceBytes = request.size;
        projection.name = request.name;
        projection.hasClut = upload.hasClut;
        transaction.timProjectionRequests.push_back(projection);
    }

    transaction.requestBatchAuthority = true;
    transaction.cpuAtlasCandidatePrepared = false;
    transaction.cpuAtlasCommitted = false;
    transaction.d3dUploadCommitted = false;
    transaction.fullVramBackingAuthority = false;
    transaction.replayValueAuthority = false;
    transaction.hostFilesystemAuthority = false;
    transaction.consumerReadAuthority = false;
    transaction.oldWinS0Authority = false;
    transaction.stage2PlusAuthority = false;
    transaction.status = Status8001AE7C::Accepted;
    transaction.accepted = true;
    transaction.complete = true;
    return transaction;
}

bool IsExactPreparedProfileTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction,
    ArchiveKind8001AC18 profile,
    uint32_t expectedTimRequestCount) {
    if (!transaction.accepted || !transaction.complete ||
        transaction.status != Status8001AE7C::Accepted ||
        transaction.sourceArchiveKind != profile ||
        transaction.sourceTimRequestCount != expectedTimRequestCount ||
        transaction.cpuAtlasCandidatePrepared ||
        transaction.cpuAtlasCommitted) {
        return false;
    }
    const Transaction8001AE7C expected = BuildProfileTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        gpuTransaction,
        profile,
        expectedTimRequestCount);
    return expected.accepted &&
           EqualTransaction8001AE7C(transaction, expected);
}

bool CommitCpuAtlasProjectionForProfile8001AE7C(
    Transaction8001AE7C& transaction,
    ArchiveKind8001AC18 profile,
    uint32_t expectedTimRequestCount) {
    if (!transaction.accepted || !transaction.complete ||
        transaction.status != Status8001AE7C::Accepted ||
        transaction.sourceArchiveKind != profile ||
        !transaction.sourcePartialVramAuthority ||
        transaction.sourceTimRequestCount != expectedTimRequestCount ||
        transaction.timProjectionRequests.size() !=
            transaction.sourceTimRequestCount ||
        !transaction.requestBatchAuthority ||
        !transaction.cpuAtlasCandidatePrepared ||
        transaction.cpuAtlasCommitted || transaction.d3dUploadCommitted ||
        transaction.fullVramBackingAuthority ||
        transaction.replayValueAuthority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority || transaction.oldWinS0Authority ||
        transaction.stage2PlusAuthority) {
        return false;
    }
    transaction.cpuAtlasCommitted = true;
    return true;
}

bool IsExactCommittedProfileTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction,
    ArchiveKind8001AC18 profile,
    uint32_t expectedTimRequestCount) {
    Transaction8001AE7C expected = BuildProfileTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        gpuTransaction,
        profile,
        expectedTimRequestCount);
    if (!expected.accepted) {
        return false;
    }
    expected.cpuAtlasCandidatePrepared = true;
    if (!CommitCpuAtlasProjectionForProfile8001AE7C(
            expected, profile, expectedTimRequestCount)) {
        return false;
    }
    return EqualTransaction8001AE7C(transaction, expected);
}

}  // namespace

Transaction8001AE7C BuildTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    return BuildProfileTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        gpuTransaction,
        ArchiveKind8001AC18::Scene0Compo00,
        PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);
}

bool IsExactPreparedTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    return IsExactPreparedProfileTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        gpuTransaction,
        ArchiveKind8001AC18::Scene0Compo00,
        PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);
}

bool CommitCpuAtlasProjection8001AE7C(Transaction8001AE7C& transaction) {
    return CommitCpuAtlasProjectionForProfile8001AE7C(
        transaction,
        ArchiveKind8001AC18::Scene0Compo00,
        PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);
}

bool IsExactCommittedTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    return IsExactCommittedProfileTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        gpuTransaction,
        ArchiveKind8001AC18::Scene0Compo00,
        PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);
}

Transaction8001AE7C BuildYCompoTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    return BuildProfileTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        gpuTransaction,
        ArchiveKind8001AC18::PracticeYCompo,
        PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618);
}

bool IsExactPreparedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    return IsExactPreparedProfileTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        gpuTransaction,
        ArchiveKind8001AC18::PracticeYCompo,
        PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618);
}

bool CommitYCompoCpuAtlasProjection8001AE7C(
    Transaction8001AE7C& transaction) {
    return CommitCpuAtlasProjectionForProfile8001AE7C(
        transaction,
        ArchiveKind8001AC18::PracticeYCompo,
        PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618);
}

bool IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    if (transaction.d3dUploadCommitted ||
        transaction.fullVramBackingAuthority ||
        transaction.replayValueAuthority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority || transaction.oldWinS0Authority ||
        transaction.stage2PlusAuthority) {
        return false;
    }
    return IsExactCommittedProfileTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        gpuTransaction,
        ArchiveKind8001AC18::PracticeYCompo,
        PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618);
}

bool CommitYCompoD3DAtlasProjection8001AE7C(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    if (!IsExactCpuAtlasCommittedYCompoTransaction8001AE7C(
            transaction, intLoad, loaderCandidate, gpuTransaction) ||
        transaction.d3dUploadCommitted ||
        transaction.fullVramBackingAuthority ||
        transaction.replayValueAuthority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority || transaction.oldWinS0Authority ||
        transaction.stage2PlusAuthority) {
        return false;
    }
    transaction.d3dUploadCommitted = true;
    return true;
}

bool IsExactCommittedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntGpuDirect::Transaction8001AE7C& gpuTransaction) {
    Transaction8001AE7C expected = BuildYCompoTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        gpuTransaction);
    if (!expected.accepted) {
        return false;
    }
    expected.cpuAtlasCandidatePrepared = true;
    if (!CommitYCompoCpuAtlasProjection8001AE7C(expected) ||
        !CommitYCompoD3DAtlasProjection8001AE7C(
            expected, intLoad, loaderCandidate, gpuTransaction)) {
        return false;
    }
    return EqualTransaction8001AE7C(transaction, expected);
}

}  // namespace PrSS0Scene0IntRendererDirect
