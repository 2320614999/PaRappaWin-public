#include "pr_ss0_scene0_int_gpu_direct.h"

#include <cstring>
#include <limits>

namespace PrSS0Scene0IntGpuDirect {
namespace {

using PrStage1LoaderGpuHal::GpuAction8001AE7C;
using PrStage1LoaderGpuHal::GpuActionKind;
using PrStage1LoaderGpuHal::GpuUploadSection;

uint16_t ReadU16LE8001AE7C(const uint8_t* data) {
    uint16_t value = 0u;
    std::memcpy(&value, data, sizeof(value));
    return value;
}

bool IsSourceRange8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    uint64_t offset,
    uint64_t size) {
    return offset <= intLoad.archiveBytes.size() &&
           size <= intLoad.archiveBytes.size() - offset;
}

bool TryU32FromSize8001AE7C(std::size_t value, uint32_t& out) {
    if (value > (std::numeric_limits<uint32_t>::max)()) {
        return false;
    }
    out = static_cast<uint32_t>(value);
    return true;
}

bool ApplyLoadImage8001AE7C(
    PartialVramState8001AE7C& state,
    const uint8_t* timBytes,
    std::size_t timSize,
    uint32_t timPsxAddress,
    const GpuAction8001AE7C& action,
    VramWrite8001AE7C& out) {
    const auto& rect = action.rect;
    if (action.kind != GpuActionKind::LoadImage ||
        action.uploadSection == GpuUploadSection::None ||
        action.payload.data == nullptr || rect.x < 0 || rect.y < 0 ||
        rect.w <= 0 || rect.h <= 0) {
        return false;
    }

    const uint32_t x = static_cast<uint32_t>(rect.x);
    const uint32_t y = static_cast<uint32_t>(rect.y);
    const uint32_t width = static_cast<uint32_t>(rect.w);
    const uint32_t height = static_cast<uint32_t>(rect.h);
    if (x > kVramWidth8001AE7C || y > kVramHeight8001AE7C ||
        width > kVramWidth8001AE7C - x ||
        height > kVramHeight8001AE7C - y) {
        return false;
    }

    const uint64_t wordCount64 =
        static_cast<uint64_t>(width) * height;
    const uint64_t byteCount64 = wordCount64 * sizeof(uint16_t);
    if (wordCount64 > (std::numeric_limits<uint32_t>::max)() ||
        byteCount64 != action.payload.size ||
        action.payload.offset > timSize ||
        action.payload.size > timSize - action.payload.offset ||
        action.payload.data != timBytes + action.payload.offset ||
        !action.payload.psxAddressKnown ||
        action.payload.psxAddress !=
            timPsxAddress + static_cast<uint32_t>(action.payload.offset)) {
        return false;
    }

    uint32_t sourceOffset = 0u;
    uint32_t sourceBytes = 0u;
    if (!TryU32FromSize8001AE7C(action.payload.offset, sourceOffset) ||
        !TryU32FromSize8001AE7C(action.payload.size, sourceBytes)) {
        return false;
    }

    uint32_t sourceWord = 0u;
    for (uint32_t row = 0u; row < height; ++row) {
        const std::size_t destinationRow =
            static_cast<std::size_t>(y + row) * kVramWidth8001AE7C + x;
        for (uint32_t column = 0u; column < width; ++column) {
            const std::size_t destination = destinationRow + column;
            if (state.known[destination] == 0u) {
                state.known[destination] = 1u;
                ++state.knownWordCount;
            }
            state.words[destination] = ReadU16LE8001AE7C(
                action.payload.data +
                static_cast<std::size_t>(sourceWord) * sizeof(uint16_t));
            ++sourceWord;
        }
    }
    state.totalWriteWordCount += wordCount64;

    out.section = action.uploadSection;
    out.rect = rect;
    out.sourceOffset = sourceOffset;
    out.sourceBytes = sourceBytes;
    out.sourcePsxAddress = action.payload.psxAddress;
    out.sourcePsxAddressKnown = true;
    out.writtenWords = static_cast<uint32_t>(wordCount64);
    return sourceWord == wordCount64;
}

bool EqualRect8001AE7C(
    const PrStage1LoaderGpuHal::PsxRect& left,
    const PrStage1LoaderGpuHal::PsxRect& right) {
    return left.x == right.x && left.y == right.y && left.w == right.w &&
           left.h == right.h;
}

bool EqualWrite8001AE7C(
    const VramWrite8001AE7C& left,
    const VramWrite8001AE7C& right) {
    return left.section == right.section &&
           EqualRect8001AE7C(left.rect, right.rect) &&
           left.sourceOffset == right.sourceOffset &&
           left.sourceBytes == right.sourceBytes &&
           left.sourcePsxAddress == right.sourcePsxAddress &&
           left.sourcePsxAddressKnown == right.sourcePsxAddressKnown &&
           left.writtenWords == right.writtenWords;
}

bool EqualUpload8001AE7C(
    const TimUpload8001AE7C& left,
    const TimUpload8001AE7C& right) {
    if (left.requestIndex != right.requestIndex ||
        left.sourcePsxAddress != right.sourcePsxAddress ||
        left.sourceBytes != right.sourceBytes || left.name != right.name ||
        left.hasClut != right.hasClut ||
        left.gpuActionCount != right.gpuActionCount ||
        left.loadImageCount != right.loadImageCount ||
        left.drawSyncCount != right.drawSyncCount) {
        return false;
    }
    for (std::size_t index = 0u; index < left.writes.size(); ++index) {
        if (!EqualWrite8001AE7C(left.writes[index], right.writes[index])) {
            return false;
        }
    }
    return true;
}

bool EqualPartialVram8001AE7C(
    const PartialVramState8001AE7C& left,
    const PartialVramState8001AE7C& right) {
    return left.words == right.words && left.known == right.known &&
           left.knownWordCount == right.knownWordCount &&
           left.totalWriteWordCount == right.totalWriteWordCount &&
           left.committedBatchCount == right.committedBatchCount;
}

bool EqualTransaction8001AE7C(
    const Transaction8001AE7C& left,
    const Transaction8001AE7C& right) {
    if (left.status != right.status || left.accepted != right.accepted ||
        left.complete != right.complete ||
        left.sourceIntLoadKnown != right.sourceIntLoadKnown ||
        left.sourceLoaderCandidateKnown !=
            right.sourceLoaderCandidateKnown ||
        left.sourceArchiveKind != right.sourceArchiveKind ||
        left.sourceTimRequestCount != right.sourceTimRequestCount ||
        left.timUploads.size() != right.timUploads.size() ||
        left.totalGpuActionCount != right.totalGpuActionCount ||
        left.totalLoadImageCount != right.totalLoadImageCount ||
        left.totalDrawSyncCount != right.totalDrawSyncCount ||
        left.totalWriteWordCount != right.totalWriteWordCount ||
        left.finalKnownWordCount != right.finalKnownWordCount ||
        left.partialVramCandidateKnown != right.partialVramCandidateKnown ||
        left.directVramCommitted != right.directVramCommitted ||
        left.partialVramWordAuthority != right.partialVramWordAuthority ||
        left.fullVramBackingAuthority != right.fullVramBackingAuthority ||
        left.rendererAtlasCommitted != right.rendererAtlasCommitted ||
        left.vabSpuCommitted != right.vabSpuCommitted ||
        left.final8001AC18ResultKnown != right.final8001AC18ResultKnown ||
        left.replayValueAuthority != right.replayValueAuthority ||
        left.hostFilesystemAuthority != right.hostFilesystemAuthority ||
        left.consumerReadAuthority != right.consumerReadAuthority ||
        left.oldWinS0Authority != right.oldWinS0Authority ||
        left.stage2PlusAuthority != right.stage2PlusAuthority ||
        static_cast<bool>(left.partialVramCandidate) !=
            static_cast<bool>(right.partialVramCandidate)) {
        return false;
    }
    for (std::size_t index = 0u; index < left.timUploads.size(); ++index) {
        if (!EqualUpload8001AE7C(
                left.timUploads[index], right.timUploads[index])) {
            return false;
        }
    }
    return left.partialVramCandidate == nullptr ||
           EqualPartialVram8001AE7C(
               *left.partialVramCandidate, *right.partialVramCandidate);
}

void MarkCommitted8001AE7C(Transaction8001AE7C& transaction) {
    transaction.partialVramCandidate->committedBatchCount = 1u;
    transaction.directVramCommitted = true;
    transaction.partialVramWordAuthority = true;
}

}  // namespace

static Transaction8001AE7C BuildArchiveTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 expectedArchiveKind,
    uint32_t expectedTimRequestCount) {
    Transaction8001AE7C transaction{};
    transaction.sourceIntLoadKnown = true;
    transaction.sourceArchiveKind = intLoad.archiveKind;
    bool exactIntLoad = false;
    switch (expectedArchiveKind) {
    case PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Compo00:
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedTransaction8001AC18(intLoad);
        break;
    case PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::PracticeYCompo:
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedYCompoTransaction80015618(intLoad);
        break;
    case PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Common:
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedStartupCommonTransaction80016B84(intLoad);
        break;
    case PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0ZCompo:
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedZCompoTransaction80015590(intLoad);
        break;
    case PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Unknown:
        break;
    }
    if (!exactIntLoad || intLoad.archiveKind != expectedArchiveKind) {
        transaction.status = Status8001AE7C::IntLoadRejected;
        return transaction;
    }
    transaction.sourceLoaderCandidateKnown = true;
    const bool exactLoaderCandidate =
        expectedArchiveKind ==
                PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Compo00
            ? PrSS0Scene0IntSideEffectDirect::
                  IsExactAcceptedTransaction8001A8F0(
                      loaderCandidate, intLoad)
            : PrSS0Scene0IntSideEffectDirect::
                  IsExactAcceptedYCompoTransaction8001A8F0(
                      loaderCandidate, intLoad);
    if (!exactLoaderCandidate) {
        transaction.status = Status8001AE7C::LoaderCandidateRejected;
        return transaction;
    }

    transaction.sourceTimRequestCount =
        static_cast<uint32_t>(loaderCandidate.timUploadRequests.size());
    if (transaction.sourceTimRequestCount != expectedTimRequestCount) {
        transaction.status = Status8001AE7C::TimRequestMalformed;
        return transaction;
    }
    transaction.partialVramCandidate =
        std::make_unique<PartialVramState8001AE7C>();
    transaction.timUploads.reserve(transaction.sourceTimRequestCount);

    for (uint32_t requestIndex = 0u;
         requestIndex < transaction.sourceTimRequestCount;
         ++requestIndex) {
        const auto& request = loaderCandidate.timUploadRequests[requestIndex];
        if (request.blockIndex != 0u || request.entryIndex != requestIndex ||
            request.type !=
                PrSS0Scene0IntLoadDirect::BlockType8001A8F0::Tim ||
            !IsSourceRange8001AE7C(
                intLoad, request.archiveDataOffset, request.size)) {
            transaction.status = Status8001AE7C::TimRequestMalformed;
            return transaction;
        }

        const uint8_t* timBytes =
            intLoad.archiveBytes.data() + request.archiveDataOffset;
        PrStage1LoaderGpuHal::TimPayloadView view{};
        view.data = timBytes;
        view.size = request.size;
        view.psxAddress = request.psxAddress;
        view.psxAddressKnown = true;
        const auto actions =
            PrStage1LoaderGpuHal::BuildGpuActions8001AE7C(view);
        if (actions.overflow ||
            actions.timParse.status !=
                PrStage1LoaderGpuHal::TimParseStatus::Ok ||
            !actions.timParse.info.valid || !actions.pixelUploadQueued ||
            !actions.firstDrawSyncQueued) {
            transaction.status = Status8001AE7C::TimParseRejected;
            return transaction;
        }

        TimUpload8001AE7C upload{};
        upload.requestIndex = requestIndex;
        upload.sourcePsxAddress = request.psxAddress;
        upload.sourceBytes = request.size;
        upload.name = request.name;
        upload.hasClut = actions.timParse.info.hasClut;
        if (!TryU32FromSize8001AE7C(actions.count, upload.gpuActionCount)) {
            transaction.status = Status8001AE7C::GpuActionMalformed;
            return transaction;
        }

        for (std::size_t actionIndex = 0u;
             actionIndex < actions.count;
             ++actionIndex) {
            const auto& action = actions.actions[actionIndex];
            if (action.sequenceIndex != actionIndex ||
                action.directFunction !=
                    PrStage1LoaderGpuHal::kFn8001AE7C) {
                transaction.status = Status8001AE7C::GpuActionMalformed;
                return transaction;
            }
            switch (action.kind) {
            case GpuActionKind::GsGetTimInfo:
                if (actionIndex != 0u ||
                    action.psxFunction !=
                        PrStage1LoaderGpuHal::kFnGsGetTimInfo80040EAC ||
                    action.timInfoInputOffset !=
                        PrStage1LoaderGpuHal::
                            kTimInfoPayloadOffset8001AE7C) {
                    transaction.status =
                        Status8001AE7C::GpuActionMalformed;
                    return transaction;
                }
                break;
            case GpuActionKind::LoadImage:
                if (upload.loadImageCount >= upload.writes.size() ||
                    action.psxFunction !=
                        PrStage1LoaderGpuHal::kFnLoadImage80044D64 ||
                    action.lowerPsxFunction !=
                        PrStage1LoaderGpuHal::kFnLowerLoadImage800468E0 ||
                    !ApplyLoadImage8001AE7C(
                        *transaction.partialVramCandidate,
                        timBytes,
                        request.size,
                        request.psxAddress,
                        action,
                        upload.writes[upload.loadImageCount])) {
                    transaction.status =
                        Status8001AE7C::VramWriteOutOfRange;
                    return transaction;
                }
                ++upload.loadImageCount;
                break;
            case GpuActionKind::DrawSync:
                if (action.psxFunction !=
                        PrStage1LoaderGpuHal::kFnDrawSync80044B3C ||
                    action.lowerPsxFunction !=
                        PrStage1LoaderGpuHal::kFnLowerDrawSync80046FFC ||
                    action.drawSyncMode !=
                        PrStage1LoaderGpuHal::kDrawSyncMode8001AE7C) {
                    transaction.status =
                        Status8001AE7C::GpuActionMalformed;
                    return transaction;
                }
                ++upload.drawSyncCount;
                break;
            default:
                transaction.status = Status8001AE7C::GpuActionMalformed;
                return transaction;
            }
        }

        const uint32_t expectedUploads = upload.hasClut ? 2u : 1u;
        const uint32_t expectedActions = 1u + expectedUploads * 2u;
        if (upload.loadImageCount != expectedUploads ||
            upload.drawSyncCount != expectedUploads ||
            upload.gpuActionCount != expectedActions ||
            actions.clutUploadQueued != upload.hasClut ||
            actions.secondDrawSyncQueued != upload.hasClut) {
            transaction.status = Status8001AE7C::GpuActionMalformed;
            return transaction;
        }
        transaction.totalGpuActionCount += upload.gpuActionCount;
        transaction.totalLoadImageCount += upload.loadImageCount;
        transaction.totalDrawSyncCount += upload.drawSyncCount;
        transaction.timUploads.push_back(upload);
    }

    transaction.totalWriteWordCount =
        transaction.partialVramCandidate->totalWriteWordCount;
    transaction.finalKnownWordCount =
        transaction.partialVramCandidate->knownWordCount;
    if (transaction.timUploads.size() != expectedTimRequestCount ||
        transaction.finalKnownWordCount == 0u ||
        transaction.totalWriteWordCount < transaction.finalKnownWordCount ||
        transaction.partialVramCandidate->committedBatchCount != 0u) {
        transaction.status = Status8001AE7C::CandidateShapeMismatch;
        return transaction;
    }

    transaction.partialVramCandidateKnown = true;
    transaction.directVramCommitted = false;
    transaction.partialVramWordAuthority = false;
    transaction.fullVramBackingAuthority = false;
    transaction.rendererAtlasCommitted = false;
    transaction.vabSpuCommitted = false;
    transaction.final8001AC18ResultKnown = false;
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

Transaction8001AE7C BuildTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    return BuildArchiveTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Compo00,
        PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18);
}

Transaction8001AE7C BuildYCompoTransaction8001AE7C(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    return BuildArchiveTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::PracticeYCompo,
        PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618);
}

Transaction8001AE7C BuildStartupCommonTransaction80016B84(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    return BuildArchiveTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Common,
        PrSS0Scene0IntLoadDirect::kExpectedCommonTimEntries80016B84);
}

Transaction8001AE7C BuildStartupZCompoTransaction80015590(
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    return BuildArchiveTransaction8001AE7C(
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0ZCompo,
        PrSS0Scene0IntLoadDirect::kExpectedZCompoTimEntries80015590);
}

bool IsExactPreparedTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    if (!transaction.accepted || !transaction.complete ||
        transaction.status != Status8001AE7C::Accepted ||
        transaction.partialVramCandidate == nullptr ||
        transaction.directVramCommitted ||
        transaction.partialVramWordAuthority) {
        return false;
    }
    const Transaction8001AE7C expected =
        BuildTransaction8001AE7C(intLoad, loaderCandidate);
    return expected.accepted &&
           EqualTransaction8001AE7C(transaction, expected);
}

bool IsExactPreparedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    if (!transaction.accepted || !transaction.complete ||
        transaction.status != Status8001AE7C::Accepted ||
        transaction.sourceArchiveKind !=
            PrSS0Scene0IntLoadDirect::
                ArchiveKind8001AC18::PracticeYCompo ||
        transaction.partialVramCandidate == nullptr ||
        transaction.directVramCommitted ||
        transaction.partialVramWordAuthority) {
        return false;
    }
    const Transaction8001AE7C expected =
        BuildYCompoTransaction8001AE7C(intLoad, loaderCandidate);
    return expected.accepted &&
           EqualTransaction8001AE7C(transaction, expected);
}

static bool IsExactPreparedStartupTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate,
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 expectedKind) {
    if (!transaction.accepted || !transaction.complete ||
        transaction.status != Status8001AE7C::Accepted ||
        transaction.sourceArchiveKind != expectedKind ||
        transaction.partialVramCandidate == nullptr ||
        transaction.directVramCommitted || transaction.partialVramWordAuthority) {
        return false;
    }
    const Transaction8001AE7C expected =
        expectedKind == PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Common
            ? BuildStartupCommonTransaction80016B84(intLoad, loaderCandidate)
            : BuildStartupZCompoTransaction80015590(intLoad, loaderCandidate);
    return expected.accepted && EqualTransaction8001AE7C(transaction, expected);
}

bool IsExactPreparedStartupCommonTransaction80016B84(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate) {
    return IsExactPreparedStartupTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Common);
}

bool IsExactPreparedStartupZCompoTransaction80015590(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate) {
    return IsExactPreparedStartupTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0ZCompo);
}

bool CommitPreparedYCompoTransaction8001AE7C(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    if (!IsExactPreparedYCompoTransaction8001AE7C(
            transaction, intLoad, loaderCandidate)) {
        return false;
    }
    MarkCommitted8001AE7C(transaction);
    return true;
}

bool IsExactCommittedYCompoTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    Transaction8001AE7C expected =
        BuildYCompoTransaction8001AE7C(intLoad, loaderCandidate);
    if (!expected.accepted || expected.partialVramCandidate == nullptr) {
        return false;
    }
    MarkCommitted8001AE7C(expected);
    return EqualTransaction8001AE7C(transaction, expected);
}

static bool CommitPreparedStartupTransaction8001AE7C(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate,
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 expectedKind) {
    const bool prepared =
        expectedKind == PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Common
            ? IsExactPreparedStartupCommonTransaction80016B84(
                  transaction, intLoad, loaderCandidate)
            : IsExactPreparedStartupZCompoTransaction80015590(
                  transaction, intLoad, loaderCandidate);
    if (!prepared) {
        return false;
    }
    MarkCommitted8001AE7C(transaction);
    return true;
}

bool CommitPreparedStartupCommonTransaction80016B84(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate) {
    return CommitPreparedStartupTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Common);
}

bool CommitPreparedStartupZCompoTransaction80015590(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate) {
    return CommitPreparedStartupTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0ZCompo);
}

static bool IsExactCommittedStartupTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate,
    PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18 expectedKind) {
    Transaction8001AE7C expected =
        expectedKind == PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Common
            ? BuildStartupCommonTransaction80016B84(intLoad, loaderCandidate)
            : BuildStartupZCompoTransaction80015590(intLoad, loaderCandidate);
    if (!expected.accepted || expected.partialVramCandidate == nullptr ||
        !CommitPreparedStartupTransaction8001AE7C(
            expected, intLoad, loaderCandidate, expectedKind)) {
        return false;
    }
    return EqualTransaction8001AE7C(transaction, expected);
}

bool IsExactCommittedStartupCommonTransaction80016B84(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate) {
    return IsExactCommittedStartupTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0Common);
}

bool IsExactCommittedStartupZCompoTransaction80015590(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0& loaderCandidate) {
    return IsExactCommittedStartupTransaction8001AE7C(
        transaction,
        intLoad,
        loaderCandidate,
        PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18::Scene0ZCompo);
}

bool CommitPreparedTransaction8001AE7C(
    Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    if (!IsExactPreparedTransaction8001AE7C(
            transaction, intLoad, loaderCandidate)) {
        return false;
    }
    MarkCommitted8001AE7C(transaction);
    return true;
}

bool IsExactCommittedTransaction8001AE7C(
    const Transaction8001AE7C& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate) {
    Transaction8001AE7C expected =
        BuildTransaction8001AE7C(intLoad, loaderCandidate);
    if (!expected.accepted || expected.partialVramCandidate == nullptr) {
        return false;
    }
    MarkCommitted8001AE7C(expected);
    return EqualTransaction8001AE7C(transaction, expected);
}

}  // namespace PrSS0Scene0IntGpuDirect
