#include "pr_stage1_movie_segment_direct.h"

#include "pr_movie_segment_direct.h"
#include "pr_scene_entry_direct.h"
#include "pr_stage1_loader_direct.h"

#include <algorithm>

namespace PrStage1MovieSegmentDirect {

bool IsOverlayTransferAttemptSeam8001ACF8(
    const PrStage1LoaderDirect::CdSeamResult& cd,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown);

namespace {

Stage1MovieSegmentPathIdentity801C4780 ConvertPathIdentity801C4780(
    PrSceneEntryDirect::SceneEntryPathRole role) {
    using GenericRole = PrSceneEntryDirect::SceneEntryPathRole;
    switch (role) {
    case GenericRole::Comod:
        return Stage1MovieSegmentPathIdentity801C4780::Comod1Bin;
    case GenericRole::Compo:
        return Stage1MovieSegmentPathIdentity801C4780::Compo01Int;
    case GenericRole::OpeningMovie:
        return Stage1MovieSegmentPathIdentity801C4780::Movie1Str;
    case GenericRole::StageRuntime:
        return Stage1MovieSegmentPathIdentity801C4780::Stage1Xa1;
    case GenericRole::ClearMovie:
        return Stage1MovieSegmentPathIdentity801C4780::Xmovie1Str;
    case GenericRole::ZCompo:
        return Stage1MovieSegmentPathIdentity801C4780::ZcompoInt;
    default:
        return Stage1MovieSegmentPathIdentity801C4780::Unknown;
    }
}

Stage1MovieSegmentRowIdentity801C4780 ConvertRowIdentity801C4780(
    PrSceneEntryDirect::SceneEntryRowRole role) {
    using GenericRole = PrSceneEntryDirect::SceneEntryRowRole;
    switch (role) {
    case GenericRole::Comod:
        return Stage1MovieSegmentRowIdentity801C4780::Row0Comod1Bin;
    case GenericRole::Compo:
        return Stage1MovieSegmentRowIdentity801C4780::Row1Compo01Int;
    case GenericRole::OpeningMovie:
        return Stage1MovieSegmentRowIdentity801C4780::Row2Movie1Str;
    case GenericRole::StageRuntime:
        return Stage1MovieSegmentRowIdentity801C4780::Row3Stage1Xa1;
    case GenericRole::ClearMovieA:
        return Stage1MovieSegmentRowIdentity801C4780::Row4Xmovie1Str;
    case GenericRole::ClearMovieB:
        return Stage1MovieSegmentRowIdentity801C4780::Row5Xmovie1Str;
    case GenericRole::ZCompo:
        return Stage1MovieSegmentRowIdentity801C4780::Row6ZcompoInt;
    default:
        return Stage1MovieSegmentRowIdentity801C4780::Unknown;
    }
}

Stage1MovieSegmentIdentity801C4780 MakeIdentity801C4780(
    const PrSceneEntryDirect::SceneEntryRowIdentity& row) {
    Stage1MovieSegmentIdentity801C4780 out{};
    out.known = row.known;
    out.rowIndexKnown = row.rowIndexKnown;
    if (out.rowIndexKnown) {
        out.rowIndex = row.rowIndex;
        out.rowIdentity = ConvertRowIdentity801C4780(row.role);
    }
    out.pathPtrKnown = row.path.pathPtrKnown;
    out.pathPtr = row.path.pathPtr;
    out.pathIdentity = ConvertPathIdentity801C4780(row.path.role);
    out.psxPath = row.path.psxPath;
    out.relativeWinPath = row.path.relativeWinPath;
    return out;
}

Stage1MovieSegmentIdentity801C4780 MakeIdentity801C4780(
    const PrSceneEntryDirect::SceneEntryPathIdentity& path) {
    Stage1MovieSegmentIdentity801C4780 out{};
    out.known = path.known;
    out.pathPtrKnown = path.pathPtrKnown;
    out.pathPtr = path.pathPtr;
    out.pathIdentity = ConvertPathIdentity801C4780(path.role);
    out.psxPath = path.psxPath;
    out.relativeWinPath = path.relativeWinPath;
    return out;
}

SceneEntryMovieSegmentRawRow801C4780 ConvertSceneEntryRawRow801C4780(
    const PrSceneEntryDirect::SceneEntryRawRow& raw) {
    SceneEntryMovieSegmentRawRow801C4780 row{};
    if (!raw.known) {
        return row;
    }

    row.known = true;
    row.pathPtrA1Plus00Known = raw.pathPtrKnown;
    row.pathPtrA1Plus00 = raw.pathPtr;
    row.opaqueA1Plus04Known = raw.opaque04Known;
    row.opaqueA1Plus04 = raw.opaque04;
    row.endBiasA1Plus08Known = raw.endBias08Known;
    row.endBiasA1Plus08 = raw.endBias08;
    row.loadedStateA1Plus0CKnown = raw.loadedState0CKnown;
    row.loadedStateA1Plus0C = raw.loadedState0C;

    // SCUS static rows reserve +0x10..+0x27 for CdlFILE, but Stage1 exports
    // show this area is zero until 8001A2B0/800381F8 supplies live feedback.
    return row;
}

}  // namespace

Stage1MovieSegmentIdentity801C4780 IdentifyStage1MovieSegmentPathPtr801C4780(
    uint32_t pathPtr) {
    return MakeIdentity801C4780(
        PrSceneEntryDirect::IdentifySceneEntryPathPtr(pathPtr));
}

Stage1MovieSegmentIdentity801C4780 IdentifyStage1MovieSegmentRow801C4780(
    uint32_t rowIndex) {
    const PrSceneEntryDirect::SceneEntryKey key =
        PrSceneEntryDirect::BuildSceneEntryKeyFromSceneIndex(
            kStage1SceneEntryMovieSegmentSceneIndex801C4780);
    return MakeIdentity801C4780(
        PrSceneEntryDirect::IdentifySceneEntryRow(key, rowIndex));
}

Stage1MovieSegmentIdentity801C4780 IdentifyStage1MovieSegmentRecord801C4780(
    const MovieSegmentRecord48& record) {
    if (record.known &&
        record.tableIndex < kSceneEntryMovieSegmentCount801C4780) {
        Stage1MovieSegmentIdentity801C4780 identity =
            IdentifyStage1MovieSegmentRow801C4780(record.tableIndex);
        if (!record.pathPtrA1Plus00Known) {
            return identity;
        }
        if (identity.known && identity.pathPtr == record.pathPtrA1Plus00) {
            return identity;
        }
    }

    if (record.pathPtrA1Plus00Known) {
        return IdentifyStage1MovieSegmentPathPtr801C4780(
            record.pathPtrA1Plus00);
    }
    return {};
}

int32_t DecodeBcd80036A78(uint8_t value) {
    return PrMovieSegmentDirect::DecodeBcd80036A78(value);
}

MsfToLbaResult80036A78 PsxCall80036A78_MsfToLba(
    const MsfBcd80036A78& msf) {
    return PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(msf);
}

SegmentInitResult8001A324 PsxCall8001A324_InitSegmentRecord(
    const MovieSegmentRecord48& source) {
    SegmentInitResult8001A324 result{};
    result.record = source;

    if (source.psxAddr != 0u) {
        result.lookupRequestKnown = true;
        result.lookupRequestCdlFilePtr = source.psxAddr + 0x10u;
    }
    result.lookupRequestPathPtrKnown = source.pathPtrA1Plus00Known;
    result.lookupRequestPathPtr = source.pathPtrA1Plus00;

    if (source.pathPtrA1Plus00Known && source.pathPtrA1Plus00 == 0u) {
        result.skippedPathPtrZero = true;
        return result;
    }

    if (source.loadedStateA1Plus0CKnown &&
        source.loadedStateA1Plus0C == 1) {
        result.skippedAlreadyLoaded = true;
        return result;
    }

    if (source.startMsfKnown) {
        const MsfToLbaResult80036A78 lba =
            ::PrStage1MovieSegmentDirect::PsxCall80036A78_MsfToLba(
                source.startMsf);
        result.record.timeBaseA1Plus40Known = lba.known;
        result.record.timeBaseA1Plus40 = lba.lba;
        result.timeBaseWrittenA1Plus40 = lba.known;
    }

    if (result.record.timeBaseA1Plus40Known &&
        source.lengthSourceA1Plus20Known) {
        result.record.endA1Plus44Known = true;
        result.record.endA1Plus44 =
            result.record.timeBaseA1Plus40 +
            static_cast<int32_t>(source.lengthSourceA1Plus20 >> 11);
        result.endWrittenA1Plus44 = true;
    }

    return result;
}

MovieSegmentRecord48 BuildMovieSegmentRecordFromRawRow801C4780(
    const SceneEntryMovieSegmentRawRow801C4780& raw,
    uint32_t rowIndex) {
    return PrMovieSegmentDirect::BuildMovieSegmentRecordFromRawRow801C4780(
        raw,
        rowIndex);
}

SceneEntryMovieSegmentTable801C4780 MaterializeSceneEntryMovieSegments801C4780(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    const MovieSegmentRecord48* rows,
    uint32_t rowCount) {
    return PrMovieSegmentDirect::MaterializeSceneEntryMovieSegments801C4780(
        sceneEntryBase,
        sceneEntryBaseKnown,
        rows,
        rowCount);
}

SceneEntryMovieSegmentTable801C4780
MaterializeSceneEntryMovieSegmentsFromRawRows801C4780(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    const SceneEntryMovieSegmentRawRow801C4780* rows,
    uint32_t rowCount) {
    return PrMovieSegmentDirect::
        MaterializeSceneEntryMovieSegmentsFromRawRows801C4780(
        sceneEntryBase,
        sceneEntryBaseKnown,
        rows,
        rowCount);
}

SceneEntryMovieSegmentTable801C4780
MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(
    uint32_t sceneIndex) {
    if (sceneIndex != kStage1SceneEntryMovieSegmentSceneIndex801C4780) {
        return {};
    }

    SceneEntryMovieSegmentRawRow801C4780
        rows[kSceneEntryMovieSegmentCount801C4780]{};
    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        rows[index] = ConvertSceneEntryRawRow801C4780(
            PrSceneEntryDirect::GetSceneEntryStaticRawRow(sceneIndex, index));
    }

    return ::PrStage1MovieSegmentDirect::
        MaterializeSceneEntryMovieSegmentsFromRawRows801C4780(
        kStage1SceneEntryBase801C4780,
        true,
        rows,
        kSceneEntryMovieSegmentCount801C4780);
}

MovieSegmentRecord48 ApplyMovieSegmentRowFeedback8001A324(
    const MovieSegmentRecord48& row,
    const MovieSegmentRowInitFeedback8001A324& feedback) {
    return PrMovieSegmentDirect::ApplyMovieSegmentRowFeedback8001A324(
        row,
        feedback);
}

MovieSegmentRowInitFeedback8001A324
BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(
    const PrStage1LoaderCdHal::ProbeCompletionFeedback8001A2B0& feedback,
    bool loadedStateA1Plus0CKnown,
    int32_t loadedStateA1Plus0C) {
    PrMovieSegmentDirect::CdLookupProbeCompletion8001A2B0 generic{};
    generic.known = feedback.known;
    generic.complete = feedback.complete;
    generic.explicitCdLookupFeedback = true;
    generic.requestKnown = feedback.requestKnown;
    generic.requestCdlFilePtr = feedback.requestCdlFilePtr;
    generic.requestPathPtrKnown = feedback.requestPathPtrKnown;
    generic.requestPathPtr = feedback.requestPathPtr;
    generic.lookupSucceeded = feedback.lookupSucceeded;
    generic.lookupFailed = feedback.lookupFailed;
    generic.resultPtr = feedback.resultPtr;
    generic.cdlFilePosKnown = feedback.cdlFilePosKnown;
    generic.cdlFilePos.minute = feedback.cdlFilePos.minute;
    generic.cdlFilePos.second = feedback.cdlFilePos.second;
    generic.cdlFilePos.frame = feedback.cdlFilePos.frame;
    generic.cdlFileSizeKnown = feedback.cdlFileSizeKnown;
    generic.cdlFileSize = feedback.cdlFileSize;
    generic.cdlFileNameKnown = feedback.cdlFileNameKnown;
    generic.cdlFileName = feedback.cdlFileName;

    return PrMovieSegmentDirect::
        BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(
            generic,
            loadedStateA1Plus0CKnown,
            loadedStateA1Plus0C);
}

MovieSegmentCdSeamProbeFeedback801C4780
BuildMovieSegmentScanFeedbackFromLoaderCdSeamProbe801C4780(
    const SceneEntryMovieSegmentTable801C4780& table,
    const PrStage1LoaderDirect::CdSeamResult& cd) {
    MovieSegmentCdSeamProbeFeedback801C4780 out{};
    if (!cd.probe8001A2B0Known || !cd.probe8001A2B0.known ||
        !cd.probe8001A2B0.complete) {
        return out;
    }

    const PrStage1LoaderCdHal::ProbeCompletionFeedback8001A2B0& probe =
        cd.probe8001A2B0;
    out.probeAccepted = true;
    out.resultPtr = probe.resultPtr;

    if (!table.sceneEntryBaseKnown) {
        return out;
    }

    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        const uint32_t rowAddr =
            PrMovieSegmentDirect::ComputeMovieSegmentRowAddr801C4780(
                table.sceneEntryBase,
                index);
        if (probe.resultPtr != rowAddr + 0x10u) {
            continue;
        }

        out.matched = true;
        out.matchedRowIndex = index;
        out.matchedRowAddr = rowAddr;
        out.rowFeedback[index] =
            ::PrStage1MovieSegmentDirect::
                BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(
                    probe,
                    false,
                    0);
        out.rowFeedback[index].explicitCdLookupFeedback = true;
        if (!out.rowFeedback[index].lookupRequestKnown) {
            out.rowFeedback[index].lookupRequestKnown = true;
            out.rowFeedback[index].lookupRequestCdlFilePtr = rowAddr + 0x10u;
        }
        if (!out.rowFeedback[index].lookupRequestPathPtrKnown) {
            out.rowFeedback[index].lookupRequestPathPtrKnown =
                table.rows[index].pathPtrA1Plus00Known;
            out.rowFeedback[index].lookupRequestPathPtr =
                table.rows[index].pathPtrA1Plus00;
        }
        out.rowFeedback[index].lookupResultPtrKnown = true;
        out.rowFeedback[index].lookupResultPtr = probe.resultPtr;
        break;
    }

    return out;
}

MovieSegmentCdLookupBatchFeedback801C4780
BuildMovieSegmentCdLookupBatchFeedback801C4780(
    const SceneEntryMovieSegmentTable801C4780& table,
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount) {
    MovieSegmentCdLookupBatchFeedback801C4780 out{};
    out.sceneEntryBaseKnown = table.sceneEntryBaseKnown;
    out.sourceSeamCount = seamCount;
    if (seams == nullptr) {
        return out;
    }

    for (uint32_t i = 0; i < seamCount; ++i) {
        if (seams[i].streamClock800493F4Known) {
            out.streamClock800493F4Known = true;
            out.streamClock800493F4 = seams[i].streamClock800493F4;
        } else if (seams[i].gapMissingStreamClock800493F4Producer) {
            out.gapMissingStreamClock800493F4Producer = true;
        }

        const MovieSegmentCdSeamProbeFeedback801C4780 one =
            BuildMovieSegmentScanFeedbackFromLoaderCdSeamProbe801C4780(
                table,
                seams[i]);
        if (one.probeAccepted) {
            out.anyProbeAccepted = true;
            ++out.sourceProbeCount;
        }
        if (!one.matched ||
            one.matchedRowIndex >= kSceneEntryMovieSegmentCount801C4780) {
            if (one.probeAccepted) {
                ++out.acceptedUnmatchedCount;
            }
            continue;
        }

        const uint32_t rowIndex = one.matchedRowIndex;
        if (!out.matchedMask[rowIndex]) {
            ++out.matchedCount;
        } else {
            ++out.duplicateMatchedCount;
        }
        out.matchedMask[rowIndex] = true;
        out.matchedRequestCdlFilePtr[rowIndex] =
            one.rowFeedback[rowIndex].lookupRequestCdlFilePtr;
        out.matchedRequestPathPtrKnown[rowIndex] =
            one.rowFeedback[rowIndex].lookupRequestPathPtrKnown;
        out.matchedRequestPathPtr[rowIndex] =
            one.rowFeedback[rowIndex].lookupRequestPathPtr;
        out.matchedResultPtr[rowIndex] = one.resultPtr;
        out.rowFeedback[rowIndex] = one.rowFeedback[rowIndex];
    }

    return out;
}

bool IsMovieSegmentRowLookupReady801C4780(
    const MovieSegmentRowInitFeedback8001A324& feedback) {
    return feedback.known && feedback.explicitCdLookupFeedback &&
           feedback.cdlFilePosKnown && feedback.cdlFileSizeKnown;
}

void RebuildMovieSegmentCdLookupBatchReadyMask801C4780(
    MovieSegmentCdLookupBatchFeedback801C4780& batch) {
    batch.matchedCount = 0u;
    batch.matchedMask.fill(false);
    batch.matchedRequestCdlFilePtr.fill(0u);
    batch.matchedRequestPathPtrKnown.fill(false);
    batch.matchedRequestPathPtr.fill(0u);
    batch.matchedResultPtr.fill(0u);

    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        const MovieSegmentRowInitFeedback8001A324& row =
            batch.rowFeedback[index];
        if (!IsMovieSegmentRowLookupReady801C4780(row)) {
            continue;
        }

        batch.matchedMask[index] = true;
        ++batch.matchedCount;
        if (row.lookupRequestKnown) {
            batch.matchedRequestCdlFilePtr[index] =
                row.lookupRequestCdlFilePtr;
        }
        batch.matchedRequestPathPtrKnown[index] =
            row.lookupRequestPathPtrKnown;
        batch.matchedRequestPathPtr[index] = row.lookupRequestPathPtr;
        if (row.lookupResultPtrKnown) {
            batch.matchedResultPtr[index] = row.lookupResultPtr;
        }
    }
}

bool HasMovieSegmentReadyRows801C455C(
    const MovieSegmentCdLookupBatchFeedback801C4780& batch) {
    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        if (IsMovieSegmentRowLookupReady801C4780(batch.rowFeedback[index])) {
            return true;
        }
    }
    return false;
}

void MergeMovieSegmentCdLookupBatchFeedback801C4780(
    MovieSegmentCdLookupBatchFeedback801C4780& dst,
    const MovieSegmentCdLookupBatchFeedback801C4780& src) {
    dst.sceneEntryBaseKnown =
        dst.sceneEntryBaseKnown || src.sceneEntryBaseKnown;
    dst.sourceSeamCount += src.sourceSeamCount;
    dst.anyProbeAccepted = dst.anyProbeAccepted || src.anyProbeAccepted;
    dst.sourceProbeCount += src.sourceProbeCount;
    dst.acceptedUnmatchedCount += src.acceptedUnmatchedCount;

    if (src.streamClock800493F4Known) {
        dst.streamClock800493F4Known = true;
        dst.streamClock800493F4 = src.streamClock800493F4;
    }
    if (src.gapMissingStreamClock800493F4Producer) {
        dst.gapMissingStreamClock800493F4Producer = true;
    }

    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        if (!IsMovieSegmentRowLookupReady801C4780(src.rowFeedback[index])) {
            continue;
        }
        if (IsMovieSegmentRowLookupReady801C4780(dst.rowFeedback[index])) {
            ++dst.duplicateMatchedCount;
        }
        dst.rowFeedback[index] = src.rowFeedback[index];
    }
    RebuildMovieSegmentCdLookupBatchReadyMask801C4780(dst);
}

void MergeMovieSegmentStreamClockCarrier801C4780(
    MovieSegmentCdLookupBatchFeedback801C4780& batch,
    const StreamClockProducerCarrier800493F4& carrier,
    bool gapMissingStreamClockProducer) {
    if (carrier.clockKnown) {
        batch.streamClock800493F4Known = true;
        batch.streamClock800493F4 = carrier;
    }
    if (gapMissingStreamClockProducer) {
        batch.gapMissingStreamClock800493F4Producer = true;
    }
}

bool HasMovieSegmentCdLookupBatchFeedback801C4780(
    const MovieSegmentCdLookupBatchFeedback801C4780& batch) {
    return HasMovieSegmentReadyRows801C455C(batch) ||
           batch.streamClock800493F4Known ||
           batch.gapMissingStreamClock800493F4Producer;
}

void EnsureStage1MovieSegmentCdLookupRuntimeScene801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex) {
    if (runtime.batchSceneIndexKnown &&
        runtime.batchSceneIndex == sceneIndex) {
        return;
    }
    runtime.seams.clear();
    runtime.batch = MovieSegmentCdLookupBatchFeedback801C4780{};
    runtime.batchKnown = false;
    runtime.batchSceneIndexKnown = true;
    runtime.batchSceneIndex = sceneIndex;
    runtime.isoLookupAttempted = false;
    runtime.isoLookupSceneIndex = 0;
    runtime.isoLookupPathKnown = false;
    runtime.isoLookupPath.clear();
    runtime.isoLookupMatchedCount = 0;
}

void ResetStage1MovieSegmentCdLookupRuntime801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime) {
    runtime.seams.clear();
    runtime.batch = MovieSegmentCdLookupBatchFeedback801C4780{};
    runtime.batchKnown = false;
    runtime.batchSceneIndexKnown = false;
    runtime.batchSceneIndex = 0;
    runtime.isoLookupAttempted = false;
    runtime.isoLookupSceneIndex = 0;
    runtime.isoLookupPathKnown = false;
    runtime.isoLookupPath.clear();
    runtime.isoLookupMatchedCount = 0;
}

bool RecordStage1MovieSegmentCdLookupSeam801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex,
    const PrStage1LoaderDirect::CdSeamResult& cd) {
    EnsureStage1MovieSegmentCdLookupRuntimeScene801C4780(
        runtime,
        sceneIndex);
    if (!cd.present) {
        return runtime.batchKnown;
    }
    runtime.seams.push_back(cd);
    const SceneEntryMovieSegmentTable801C4780 table =
        MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(
            sceneIndex);
    runtime.batch =
        BuildMovieSegmentCdLookupBatchFeedback801C4780(
            table,
            runtime.seams.data(),
            static_cast<uint32_t>(runtime.seams.size()));
    runtime.batchKnown =
        HasMovieSegmentCdLookupBatchFeedback801C4780(runtime.batch);
    return runtime.batchKnown;
}

bool TryGetStage1MovieSegmentCdLookupBatch801C4780(
    const Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex,
    MovieSegmentCdLookupBatchFeedback801C4780& out) {
    if (!runtime.batchKnown || !runtime.batchSceneIndexKnown ||
        runtime.batchSceneIndex != sceneIndex) {
        out = MovieSegmentCdLookupBatchFeedback801C4780{};
        return false;
    }
    out = runtime.batch;
    return true;
}

bool RunStage1MovieSegmentCdLookupIsoProducer801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex,
    const std::filesystem::path& binPath,
    bool binPathKnown) {
    EnsureStage1MovieSegmentCdLookupRuntimeScene801C4780(
        runtime,
        sceneIndex);
    if (!binPathKnown) {
        return runtime.batchKnown;
    }
    if (runtime.isoLookupAttempted &&
        runtime.isoLookupSceneIndex == sceneIndex &&
        runtime.isoLookupPathKnown &&
        runtime.isoLookupPath == binPath) {
        return runtime.batchKnown;
    }

    const MovieSegmentCdLookupBatchFeedback801C4780 isoBatch =
        RunStage1StaticMovieSegmentCdLookupProducer801C4780(
            sceneIndex,
            binPath,
            true);
    MergeMovieSegmentCdLookupBatchFeedback801C4780(runtime.batch, isoBatch);
    runtime.batchKnown =
        HasMovieSegmentCdLookupBatchFeedback801C4780(runtime.batch);
    runtime.isoLookupAttempted = true;
    runtime.isoLookupSceneIndex = sceneIndex;
    runtime.isoLookupPathKnown = true;
    runtime.isoLookupPath = binPath;
    runtime.isoLookupMatchedCount = isoBatch.matchedCount;
    return runtime.batchKnown;
}

bool BuildStage1MovieSegmentCdLookupCarrier801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex,
    const std::filesystem::path& binPath,
    bool binPathKnown,
    const StreamClockProducerCarrier800493F4* streamClockCarrier,
    bool gapMissingStreamClockProducer,
    MovieSegmentCdLookupBatchFeedback801C4780& out) {
    out = MovieSegmentCdLookupBatchFeedback801C4780{};
    MovieSegmentCdLookupBatchFeedback801C4780 stored{};
    if (TryGetStage1MovieSegmentCdLookupBatch801C4780(
            runtime,
            sceneIndex,
            stored)) {
        MergeMovieSegmentCdLookupBatchFeedback801C4780(out, stored);
    }

    if (out.matchedCount < kSceneEntryMovieSegmentCount801C4780) {
        (void)RunStage1MovieSegmentCdLookupIsoProducer801C4780(
            runtime,
            sceneIndex,
            binPath,
            binPathKnown);
        out = MovieSegmentCdLookupBatchFeedback801C4780{};
        if (TryGetStage1MovieSegmentCdLookupBatch801C4780(
                runtime,
                sceneIndex,
                stored)) {
            MergeMovieSegmentCdLookupBatchFeedback801C4780(out, stored);
        }
    }

    if (streamClockCarrier != nullptr) {
        MergeMovieSegmentStreamClockCarrier801C4780(
            out,
            *streamClockCarrier,
            gapMissingStreamClockProducer);
    }
    return HasMovieSegmentCdLookupBatchFeedback801C4780(out);
}

MovieSegmentCdLookupBatchFeedback801C4780
RunMovieSegmentCdLookupProducer801C4780(
    const SceneEntryMovieSegmentTable801C4780& table,
    const std::filesystem::path& binPath,
    bool binPathKnown) {
    MovieSegmentCdLookupBatchFeedback801C4780 out{};
    out.sceneEntryBaseKnown = table.sceneEntryBaseKnown;
    if (!binPathKnown) {
        return out;
    }

    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        const MovieSegmentRecord48& row = table.rows[index];
        if (!row.known) {
            continue;
        }
        const bool disabledPath =
            row.pathPtrA1Plus00Known && row.pathPtrA1Plus00 == 0u;
        const bool alreadyLoaded =
            row.loadedStateA1Plus0CKnown && row.loadedStateA1Plus0C == 1;
        if (disabledPath || alreadyLoaded || !row.pathPtrA1Plus00Known) {
            continue;
        }

        uint32_t rowAddr = row.psxAddr;
        if (rowAddr == 0u && table.sceneEntryBaseKnown) {
            rowAddr = PrMovieSegmentDirect::ComputeMovieSegmentRowAddr801C4780(
                table.sceneEntryBase,
                index);
        }
        if (rowAddr == 0u) {
            continue;
        }

        const Stage1MovieSegmentIdentity801C4780 identity =
            IdentifyStage1MovieSegmentPathPtr801C4780(row.pathPtrA1Plus00);
        if (!identity.known || identity.psxPath == nullptr) {
            continue;
        }

        const uint32_t requestCdlFilePtr = rowAddr + 0x10u;
        PrStage1LoaderCdHal::Probe8001A2B0State probe =
            PrStage1LoaderCdHal::BeginProbe8001A2B0(
                requestCdlFilePtr,
                row.pathPtrA1Plus00);
        PrStage1LoaderCdHal::LookupFeedback800381F8 lastLookup{};
        for (;;) {
            PrStage1LoaderCdHal::Action action{};
            if (!PrStage1LoaderCdHal::BuildNextLookupAction8001A2B0(
                    probe,
                    action)) {
                break;
            }

            PrStage1LoaderCdHal::Iso9660LookupInput800381F8 isoInput{};
            isoInput.valid = true;
            isoInput.binPath = binPath;
            isoInput.psxPath = identity.psxPath;
            isoInput.cdlFilePtr = action.cdlFilePtr;
            isoInput.pathPtr = action.pathPtr;
            isoInput.retryIndex = action.retryIndex;
            const PrStage1LoaderCdHal::Iso9660LookupResult800381F8
                isoResult =
                    PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(
                        isoInput);
            lastLookup = isoResult.feedback;
            PrStage1LoaderCdHal::ApplyLookupFeedback8001A2B0(probe,
                                                             lastLookup);
        }

        const PrStage1LoaderCdHal::ProbeCompletionFeedback8001A2B0
            completion =
                PrStage1LoaderCdHal::BuildProbeCompletionFeedback8001A2B0(
                    probe,
                    lastLookup);
        if (!completion.known || !completion.complete) {
            continue;
        }

        out.anyProbeAccepted = true;
        ++out.sourceProbeCount;
        if (!completion.lookupSucceeded || completion.resultPtr == 0u) {
            ++out.acceptedUnmatchedCount;
            continue;
        }

        out.matchedMask[index] = true;
        ++out.matchedCount;
        out.matchedRequestCdlFilePtr[index] =
            completion.requestCdlFilePtr;
        out.matchedRequestPathPtrKnown[index] =
            completion.requestPathPtrKnown;
        out.matchedRequestPathPtr[index] = completion.requestPathPtr;
        out.matchedResultPtr[index] = completion.resultPtr;
        out.rowFeedback[index] =
            BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(
                completion,
                row.loadedStateA1Plus0CKnown,
                row.loadedStateA1Plus0C);
        out.rowFeedback[index].lookupResultPtrKnown = true;
        out.rowFeedback[index].lookupResultPtr = completion.resultPtr;
    }

    return out;
}

MovieSegmentCdLookupBatchFeedback801C4780
RunStage1StaticMovieSegmentCdLookupProducer801C4780(
    uint32_t sceneIndex,
    const std::filesystem::path& binPath,
    bool binPathKnown) {
    const SceneEntryMovieSegmentTable801C4780 table =
        MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(
            sceneIndex);
    return RunMovieSegmentCdLookupProducer801C4780(table, binPath, binPathKnown);
}

bool TryBuildStage1MovieSegmentRowFeedbackFromIso801C4780(
    uint32_t sceneIndex,
    uint32_t rowIndex,
    const std::filesystem::path& binPath,
    bool binPathKnown,
    MovieSegmentRowInitFeedback8001A324& out) {
    out = MovieSegmentRowInitFeedback8001A324{};
    if (rowIndex >= kSceneEntryMovieSegmentCount801C4780) {
        return false;
    }
    const MovieSegmentCdLookupBatchFeedback801C4780 batch =
        RunStage1StaticMovieSegmentCdLookupProducer801C4780(
            sceneIndex,
            binPath,
            binPathKnown);
    if (!IsMovieSegmentRowLookupReady801C4780(batch.rowFeedback[rowIndex])) {
        return false;
    }
    out = batch.rowFeedback[rowIndex];
    return true;
}

bool BuildOverlayTransferSeekFeedbackFromCdSeam8001A89C(
    const PrStage1LoaderDirect::CdSeamResult& cd,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown,
    PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C& out) {
    out = PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C{};
    if (!cd.present || !cd.feedback.handled ||
        cd.feedback.kind !=
            PrStage1LoaderCdHal::ActionKind::SeekSync800367A4 ||
        !IsOverlayTransferAttemptSeam8001ACF8(
            cd,
            dst,
            dstKnown,
            sectorCount,
            sectorCountKnown)) {
        return false;
    }

    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request =
        cd.lowerRequest;
    if (!request.known ||
        request.actionKind !=
            PrStage1LoaderCdHal::ActionKind::SeekSync800367A4 ||
        request.callerFunction != PrStage1LoaderCdHal::kFn8001A89C ||
        request.directHelperFunction != PrStage1LoaderCdHal::kFn80036974 ||
        request.lowerFunction != PrStage1LoaderCdHal::kFn800367A4 ||
        !request.seekRequestKnown ||
        !request.seekMsfTargetPtrKnown ||
        !request.seekLbaKnown ||
        !request.seekMsfTargetKnown ||
        request.seekArg0 != PrStage1LoaderCdHal::kSeek8001A89CSyncMode ||
        request.seekArg1 !=
            static_cast<int32_t>(request.seekMsfTargetPtr) ||
        request.seekArg2 != PrStage1LoaderCdHal::kSeek8001A89CSyncArg2) {
        return false;
    }

    PrMovieSegmentDirect::OverlayTransferSeekLowerFeedback8001A89C lower{};
    lower.known = true;
    lower.syncResultKnown = true;
    lower.syncResult = cd.feedback.psxReturn;
    lower.psxReturnKnown = true;
    lower.psxReturn = cd.feedback.success || cd.feedback.psxReturn != 0;
    out = PrMovieSegmentDirect::BuildOverlayTransferSeekFeedback8001A89C(
        lower);
    return true;
}

bool IsOverlayTransferReadStartSeam8001A818(
    const PrStage1LoaderDirect::CdSeamResult& cd,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown) {
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request =
        cd.lowerRequest;
    if (!request.known ||
        request.actionKind !=
            PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0 ||
        request.callerFunction != PrStage1LoaderCdHal::kFn8001A818 ||
        request.lowerFunction != PrStage1LoaderCdHal::kFn80038FC0 ||
        request.finalFunction != PrStage1LoaderCdHal::kFn800390C8 ||
        !request.readStartRequestKnown ||
        !request.readStartDstPtrKnown ||
        !request.readStartSectorCountKnown ||
        !request.readStartModeFlagKnown ||
        !request.readSyncRequestKnown ||
        request.readSyncFunction != PrStage1LoaderCdHal::kFn800390C8 ||
        request.readSyncArg0 != PrStage1LoaderCdHal::kRead8001A818SyncArg0 ||
        request.readSyncArg1 != PrStage1LoaderCdHal::kRead8001A818SyncArg1) {
        return false;
    }
    if (dstKnown && request.readStartDstPtr != dst) {
        return false;
    }
    if (sectorCountKnown &&
        request.readStartSectorCount != static_cast<int32_t>(sectorCount)) {
        return false;
    }
    return true;
}

bool IsOverlayTransferReadSyncSeam8001A818(
    const PrStage1LoaderDirect::CdSeamResult& cd) {
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& request =
        cd.lowerRequest;
    return request.known &&
           request.actionKind ==
               PrStage1LoaderCdHal::ActionKind::ReadSync800390C8 &&
           request.callerFunction == PrStage1LoaderCdHal::kFn8001A818 &&
           request.lowerFunction == PrStage1LoaderCdHal::kFn800390C8 &&
           request.finalFunction == PrStage1LoaderCdHal::kFn800364F0 &&
           request.readSyncRequestKnown &&
           request.readSyncFunction == PrStage1LoaderCdHal::kFn800390C8 &&
           request.readSyncArg0 ==
               PrStage1LoaderCdHal::kRead8001A818SyncArg0 &&
           request.readSyncArg1 ==
               PrStage1LoaderCdHal::kRead8001A818SyncArg1;
}

bool IsOverlayTransferAttemptSeam8001ACF8(
    const PrStage1LoaderDirect::CdSeamResult& cd,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown) {
    const PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution&
        attempt = cd.overlayTransferAttempt;
    if (!attempt.known ||
        attempt.sourceFunction !=
            PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper ||
        attempt.transferFunction !=
            PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer) {
        return false;
    }
    if (dstKnown && (!attempt.dstKnown || attempt.dst != dst)) {
        return false;
    }
    if (sectorCountKnown &&
        (!attempt.sectorCountKnown || attempt.sectorCount != sectorCount)) {
        return false;
    }
    return true;
}

bool BuildOverlayTransferReadLowerFeedbackFromCdSeams8001A818(
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown,
    PrMovieSegmentDirect::OverlayTransferReadLowerFeedback8001A818& out) {
    out = PrMovieSegmentDirect::OverlayTransferReadLowerFeedback8001A818{};
    if (seams == nullptr) {
        return false;
    }

    for (uint32_t index = 0; index < seamCount; ++index) {
        const PrStage1LoaderDirect::CdSeamResult& cd = seams[index];
        if (!cd.present ||
            !cd.feedback.handled ||
            !IsOverlayTransferAttemptSeam8001ACF8(
                cd,
                dst,
                dstKnown,
                sectorCount,
                sectorCountKnown)) {
            continue;
        }
        if (cd.feedback.kind ==
            PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0) {
            if (!IsOverlayTransferReadStartSeam8001A818(
                    cd,
                    dst,
                    dstKnown,
                    sectorCount,
                    sectorCountKnown)) {
                continue;
            }
            out.known = true;
            out.readStartSucceededKnown = true;
            out.readStartSucceeded =
                cd.feedback.success || cd.feedback.psxReturn != 0;
            if (!out.readStartSucceeded) {
                out.readStartRetryCount += 1u;
            }
            out.readStartRetryCountKnown = true;
            continue;
        }
        if (cd.feedback.kind ==
            PrStage1LoaderCdHal::ActionKind::ReadSync800390C8) {
            if (!IsOverlayTransferReadSyncSeam8001A818(cd)) {
                continue;
            }
            out.known = true;
            out.syncResultKnown = true;
            out.syncResult = cd.feedback.psxReturn;
            out.syncPollCountKnown = true;
            out.syncPollCount += 1u;
        }
    }
    return out.known;
}

bool BuildOverlayTransferAttemptFeedbackFromCdSeams8001ACF8(
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown,
    int32_t firstAttemptFlag,
    PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C& outSeek,
    PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818& outRead) {
    outSeek = PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C{};
    outRead = PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818{};
    if (seams == nullptr || seamCount == 0u) {
        return false;
    }

    bool seekKnown = false;
    for (uint32_t index = 0; index < seamCount; ++index) {
        if (BuildOverlayTransferSeekFeedbackFromCdSeam8001A89C(
                seams[index],
                dst,
                dstKnown,
                sectorCount,
                sectorCountKnown,
                outSeek)) {
            seekKnown = true;
            break;
        }
    }

    PrMovieSegmentDirect::OverlayTransferReadLowerFeedback8001A818
        readLower{};
    const bool readLowerKnown =
        BuildOverlayTransferReadLowerFeedbackFromCdSeams8001A818(
            seams,
            seamCount,
            dst,
            dstKnown,
            sectorCount,
            sectorCountKnown,
            readLower);
    if (readLowerKnown) {
        outRead = PrMovieSegmentDirect::
            BuildOverlayTransferReadFeedback8001A818(
                dst,
                dstKnown,
                sectorCount,
                sectorCountKnown,
                firstAttemptFlag,
                true,
                readLower);
    }
    return seekKnown || readLowerKnown;
}

void TagOverlayTransferAttemptCdSeam8001ACF8(
    PrStage1LoaderDirect::CdSeamResult& cd,
    const PrMovieSegmentDirect::OverlayTransferAttemptProvenance8001ACF8&
        provenance) {
    cd.overlayTransferAttempt.known = provenance.known;
    cd.overlayTransferAttempt.sourceFunction = provenance.wrapperFunction;
    cd.overlayTransferAttempt.transferFunction = provenance.transferFunction;
    cd.overlayTransferAttempt.attemptIndexKnown =
        provenance.attemptIndexKnown;
    cd.overlayTransferAttempt.attemptIndex = provenance.attemptIndex;
    cd.overlayTransferAttempt.rowAddrKnown = provenance.rowAddrKnown;
    cd.overlayTransferAttempt.rowAddr = provenance.rowAddr;
    cd.overlayTransferAttempt.dstKnown = provenance.destKnown;
    cd.overlayTransferAttempt.dst = provenance.dest;
    cd.overlayTransferAttempt.sectorCountKnown =
        provenance.sectorCountKnown;
    cd.overlayTransferAttempt.sectorCount = provenance.sectorCount;
}

MovieSegmentScanResult801C4780 PsxCall801C4780_ScanMovieSegments(
    const SceneEntryMovieSegmentTable801C4780& table) {
    return ::PrStage1MovieSegmentDirect::
        PsxCall801C4780_ScanMovieSegmentsWithFeedback(table, nullptr, 0);
}

MovieSegmentScanResult801C4780 PsxCall801C4780_ScanMovieSegmentsWithFeedback(
    const SceneEntryMovieSegmentTable801C4780& table,
    const MovieSegmentRowInitFeedback8001A324* feedback,
    uint32_t feedbackCount) {
    MovieSegmentScanResult801C4780 result{};
    result.called = true;
    result.table = table;
    result.feedbackCount = feedbackCount;

    for (uint32_t index = 0; index < kSceneEntryMovieSegmentCount801C4780;
         ++index) {
        if (feedback != nullptr && index < feedbackCount &&
            feedback[index].known) {
            result.feedbackAppliedMask[index] = true;
            ++result.feedbackAppliedCount;
            result.table.rows[index] =
                ::PrStage1MovieSegmentDirect::
                    ApplyMovieSegmentRowFeedback8001A324(
                        result.table.rows[index],
                        feedback[index]);
        }
        const MovieSegmentRecord48& rowBeforeInit = result.table.rows[index];
        const bool explicitFeedback =
            feedback != nullptr && index < feedbackCount &&
            feedback[index].known &&
            feedback[index].explicitCdLookupFeedback;
        const bool disabledPath =
            rowBeforeInit.pathPtrA1Plus00Known &&
            rowBeforeInit.pathPtrA1Plus00 == 0u;
        const bool alreadyLoaded =
            rowBeforeInit.loadedStateA1Plus0CKnown &&
            rowBeforeInit.loadedStateA1Plus0C == 1;
        const bool lookupReady =
            explicitFeedback && feedback[index].cdlFilePosKnown &&
            feedback[index].cdlFileSizeKnown;
        const bool cdlFileNameReady =
            explicitFeedback && feedback[index].cdlFileNameKnown;
        const bool needsLookup =
            rowBeforeInit.known && !disabledPath && !alreadyLoaded;
        const bool requestKnown =
            needsLookup && rowBeforeInit.psxAddr != 0u;
        const bool requestPathPtrKnown =
            requestKnown && rowBeforeInit.pathPtrA1Plus00Known;
        result.rowNeedsCdLookupMask[index] = needsLookup;
        if (needsLookup) {
            ++result.rowNeedsCdLookupCount;
        }
        result.rowCdLookupRequestKnownMask[index] = requestKnown;
        if (requestKnown) {
            result.rowCdLookupRequestCdlFilePtr[index] =
                rowBeforeInit.psxAddr + 0x10u;
            ++result.rowCdLookupRequestKnownCount;
        }
        result.rowCdLookupRequestPathPtrKnownMask[index] =
            requestPathPtrKnown;
        if (requestPathPtrKnown) {
            result.rowCdLookupRequestPathPtr[index] =
                rowBeforeInit.pathPtrA1Plus00;
            ++result.rowCdLookupRequestPathPtrKnownCount;
        }
        result.rowMissingCdLookupRequestMask[index] =
            needsLookup && !requestPathPtrKnown;
        if (result.rowMissingCdLookupRequestMask[index]) {
            ++result.rowMissingCdLookupRequestCount;
        }
        if (feedback != nullptr && index < feedbackCount &&
            feedback[index].known && feedback[index].lookupResultPtrKnown) {
            result.rowCdLookupResultPtrKnownMask[index] = true;
            result.rowCdLookupResultPtr[index] = feedback[index].lookupResultPtr;
            ++result.rowCdLookupResultPtrKnownCount;
        }
        result.rowCdLookupReadyMask[index] = lookupReady;
        if (lookupReady) {
            ++result.rowCdLookupReadyCount;
        }
        result.rowCdlFileNameReadyMask[index] = cdlFileNameReady;
        if (cdlFileNameReady) {
            ++result.rowCdlFileNameReadyCount;
        }
        result.rowMissingCdLookupFeedbackMask[index] =
            needsLookup && !lookupReady;
        if (result.rowMissingCdLookupFeedbackMask[index]) {
            ++result.rowMissingCdLookupFeedbackCount;
        }
        result.rowMissingCdlFileNameFeedbackMask[index] =
            needsLookup && !cdlFileNameReady;
        if (result.rowMissingCdlFileNameFeedbackMask[index]) {
            ++result.rowMissingCdlFileNameFeedbackCount;
        }
        MovieSegmentRecord48 rowForInit = result.table.rows[index];
        if (needsLookup && !lookupReady) {
            rowForInit.startMsfKnown = false;
            rowForInit.lengthSourceA1Plus20Known = false;
            rowForInit.cdlFileNameA1Plus18Known = false;
        }
        result.rowInit[index] =
            ::PrStage1MovieSegmentDirect::PsxCall8001A324_InitSegmentRecord(
                rowForInit);
        result.table.rows[index] = result.rowInit[index].record;
    }
    result.gapMissingCdLookupFeedback =
        result.rowMissingCdLookupFeedbackCount != 0u;
    result.gapMissingCdlFileNameFeedback =
        result.rowMissingCdlFileNameFeedbackCount != 0u;
    result.gapMissingCdLookupRequest =
        result.rowMissingCdLookupRequestCount != 0u;

    result.reset80025A00Action = true;
    result.loadCompo8001AC18Action = true;
    if (table.sceneEntryBaseKnown) {
        result.loadCompoArg0 = table.sceneEntryBase + 0x3Cu;
    }
    result.loadCompoArg1 = 0;
    return result;
}

SceneEntrySegmentSelection801C4DC4 PsxSelectMovieSegment801C4DC4(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown) {
    return PrMovieSegmentDirect::PsxSelectMovieSegmentByOffset801C4DC4(
        sceneEntryBase,
        sceneEntryBaseKnown,
        kSceneEntryMovieSegmentOffset801C4DC4);
}

SceneEntrySegmentSelection801C4DC4 PsxSelectMovieSegment801C4DC4FromScan(
    const MovieSegmentScanResult801C4780& scan) {
    return PrMovieSegmentDirect::PsxSelectMovieSegmentByOffsetFromScan801C4DC4(
        scan,
        kSceneEntryMovieSegmentOffset801C4DC4);
}

MovieStepSegmentFields801C4350 BuildMovieStepSegmentFields801C4350(
    const MovieSegmentRecord48& record) {
    return PrMovieSegmentDirect::BuildMovieStepSegmentFields801C4350(record);
}

}  // namespace PrStage1MovieSegmentDirect
