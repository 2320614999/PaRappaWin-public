#pragma once

#include "pr_movie_segment_direct.h"
#include "pr_stage1_loader_cd_hal.h"
#include "pr_stage1_loader_direct.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace PrStage1MovieSegmentDirect {

static constexpr uint32_t kMovieSegmentRecordSize8001A324 = 0x30u;
static constexpr uint32_t kSceneEntryMovieSegmentOffset801C4DC4 = 0x6Cu;
static constexpr uint32_t kSceneEntryMovieSegmentCount801C4780 = 7u;
static constexpr uint32_t kStage1SceneEntryMovieSegmentSceneIndex801C4780 = 1u;
static constexpr uint32_t kStage1SceneEntryBase801C4780 = 0x800548B8u;
static constexpr uint32_t kStage1MovieSegmentPathPtrComod1Bin801C4780 =
    0x80011784u;
static constexpr uint32_t kStage1MovieSegmentPathPtrCompo01Int801C4780 =
    0x80011770u;
static constexpr uint32_t kStage1MovieSegmentPathPtrMovie1Str801C4780 =
    0x8001175Cu;
static constexpr uint32_t kStage1MovieSegmentPathPtrStage1Xa1801C4780 =
    0x80011748u;
static constexpr uint32_t kStage1MovieSegmentPathPtrXmovie1Str801C4780 =
    0x80011734u;
static constexpr uint32_t kStage1MovieSegmentPathPtrZcompoInt801C4780 =
    0x80011720u;

enum class Stage1MovieSegmentPathIdentity801C4780 : uint8_t {
    Unknown = 0,
    Comod1Bin,
    Compo01Int,
    Movie1Str,
    Stage1Xa1,
    Xmovie1Str,
    ZcompoInt,
};

enum class Stage1MovieSegmentRowIdentity801C4780 : uint8_t {
    Unknown = 0,
    Row0Comod1Bin,
    Row1Compo01Int,
    Row2Movie1Str,
    Row3Stage1Xa1,
    Row4Xmovie1Str,
    Row5Xmovie1Str,
    Row6ZcompoInt,
};

struct Stage1MovieSegmentIdentity801C4780 {
    bool known = false;
    bool rowIndexKnown = false;
    uint32_t rowIndex = 0;
    bool pathPtrKnown = false;
    uint32_t pathPtr = 0;
    Stage1MovieSegmentRowIdentity801C4780 rowIdentity =
        Stage1MovieSegmentRowIdentity801C4780::Unknown;
    Stage1MovieSegmentPathIdentity801C4780 pathIdentity =
        Stage1MovieSegmentPathIdentity801C4780::Unknown;
    const char* psxPath = nullptr;
    const char* relativeWinPath = nullptr;
};

using MsfBcd80036A78 = PrMovieSegmentDirect::MsfBcd80036A78;
using MovieSegmentRecord48 = PrMovieSegmentDirect::MovieSegmentRecord48;
using MovieSegmentRowInitFeedback8001A324 =
    PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324;
using SceneEntryMovieSegmentRawRow801C4780 =
    PrMovieSegmentDirect::SceneEntryMovieSegmentRawRow801C4780;
using SceneEntryMovieSegmentTable801C4780 =
    PrMovieSegmentDirect::SceneEntryMovieSegmentTable801C4780;
using MsfToLbaResult80036A78 =
    PrMovieSegmentDirect::MsfToLbaResult80036A78;
using SegmentInitResult8001A324 =
    PrMovieSegmentDirect::SegmentInitResult8001A324;
using MovieSegmentScanResult801C4780 =
    PrMovieSegmentDirect::MovieSegmentScanResult801C4780;
using SceneEntrySegmentSelection801C4DC4 =
    PrMovieSegmentDirect::SceneEntrySegmentSelection801C4DC4;
using MovieStepSegmentFields801C4350 =
    PrMovieSegmentDirect::MovieStepSegmentFields801C4350;
using StreamClockProducerCarrier800493F4 =
    PrMovieSegmentDirect::StreamClockProducerCarrier800493F4;

struct MovieSegmentCdSeamProbeFeedback801C4780 {
    bool probeAccepted = false;
    bool matched = false;
    uint32_t matchedRowIndex = 0;
    uint32_t matchedRowAddr = 0;
    uint32_t resultPtr = 0;
    std::array<MovieSegmentRowInitFeedback8001A324,
               kSceneEntryMovieSegmentCount801C4780>
        rowFeedback{};
    bool streamClock800493F4Known = false;
    StreamClockProducerCarrier800493F4 streamClock800493F4{};
    bool gapMissingStreamClock800493F4Producer = false;
};

struct MovieSegmentCdLookupBatchFeedback801C4780 {
    bool sceneEntryBaseKnown = false;
    uint32_t sourceSeamCount = 0;
    bool anyProbeAccepted = false;
    uint32_t sourceProbeCount = 0;
    uint32_t acceptedUnmatchedCount = 0;
    uint32_t matchedCount = 0;
    uint32_t duplicateMatchedCount = 0;
    bool streamClock800493F4Known = false;
    StreamClockProducerCarrier800493F4 streamClock800493F4{};
    bool gapMissingStreamClock800493F4Producer = false;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780> matchedMask{};
    std::array<uint32_t, kSceneEntryMovieSegmentCount801C4780>
        matchedRequestCdlFilePtr{};
    std::array<bool, kSceneEntryMovieSegmentCount801C4780>
        matchedRequestPathPtrKnown{};
    std::array<uint32_t, kSceneEntryMovieSegmentCount801C4780>
        matchedRequestPathPtr{};
    std::array<uint32_t, kSceneEntryMovieSegmentCount801C4780>
        matchedResultPtr{};
    std::array<MovieSegmentRowInitFeedback8001A324,
               kSceneEntryMovieSegmentCount801C4780>
        rowFeedback{};
};

struct Stage1MovieSegmentCdLookupRuntime801C4780 {
    std::vector<PrStage1LoaderDirect::CdSeamResult> seams;
    MovieSegmentCdLookupBatchFeedback801C4780 batch{};
    bool batchKnown = false;
    bool batchSceneIndexKnown = false;
    uint32_t batchSceneIndex = 0;
    bool isoLookupAttempted = false;
    uint32_t isoLookupSceneIndex = 0;
    bool isoLookupPathKnown = false;
    std::filesystem::path isoLookupPath;
    uint32_t isoLookupMatchedCount = 0;
};

Stage1MovieSegmentIdentity801C4780 IdentifyStage1MovieSegmentPathPtr801C4780(
    uint32_t pathPtr);
Stage1MovieSegmentIdentity801C4780 IdentifyStage1MovieSegmentRow801C4780(
    uint32_t rowIndex);
Stage1MovieSegmentIdentity801C4780 IdentifyStage1MovieSegmentRecord801C4780(
    const MovieSegmentRecord48& record);
int32_t DecodeBcd80036A78(uint8_t value);
MsfToLbaResult80036A78 PsxCall80036A78_MsfToLba(
    const MsfBcd80036A78& msf);
SegmentInitResult8001A324 PsxCall8001A324_InitSegmentRecord(
    const MovieSegmentRecord48& source);
MovieSegmentRecord48 BuildMovieSegmentRecordFromRawRow801C4780(
    const SceneEntryMovieSegmentRawRow801C4780& raw,
    uint32_t rowIndex);
SceneEntryMovieSegmentTable801C4780 MaterializeSceneEntryMovieSegments801C4780(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    const MovieSegmentRecord48* rows,
    uint32_t rowCount);
SceneEntryMovieSegmentTable801C4780
MaterializeSceneEntryMovieSegmentsFromRawRows801C4780(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    const SceneEntryMovieSegmentRawRow801C4780* rows,
    uint32_t rowCount);
SceneEntryMovieSegmentTable801C4780
MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(
    uint32_t sceneIndex);
MovieSegmentRecord48 ApplyMovieSegmentRowFeedback8001A324(
    const MovieSegmentRecord48& row,
    const MovieSegmentRowInitFeedback8001A324& feedback);
MovieSegmentRowInitFeedback8001A324
BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(
    const PrStage1LoaderCdHal::ProbeCompletionFeedback8001A2B0& feedback,
    bool loadedStateA1Plus0CKnown,
    int32_t loadedStateA1Plus0C);
MovieSegmentCdSeamProbeFeedback801C4780
BuildMovieSegmentScanFeedbackFromLoaderCdSeamProbe801C4780(
    const SceneEntryMovieSegmentTable801C4780& table,
    const PrStage1LoaderDirect::CdSeamResult& cd);
MovieSegmentCdLookupBatchFeedback801C4780
BuildMovieSegmentCdLookupBatchFeedback801C4780(
    const SceneEntryMovieSegmentTable801C4780& table,
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount);
bool IsMovieSegmentRowLookupReady801C4780(
    const MovieSegmentRowInitFeedback8001A324& feedback);
void RebuildMovieSegmentCdLookupBatchReadyMask801C4780(
    MovieSegmentCdLookupBatchFeedback801C4780& batch);
bool HasMovieSegmentReadyRows801C455C(
    const MovieSegmentCdLookupBatchFeedback801C4780& batch);
void MergeMovieSegmentCdLookupBatchFeedback801C4780(
    MovieSegmentCdLookupBatchFeedback801C4780& dst,
    const MovieSegmentCdLookupBatchFeedback801C4780& src);
void MergeMovieSegmentStreamClockCarrier801C4780(
    MovieSegmentCdLookupBatchFeedback801C4780& batch,
    const StreamClockProducerCarrier800493F4& carrier,
    bool gapMissingStreamClockProducer);
bool HasMovieSegmentCdLookupBatchFeedback801C4780(
    const MovieSegmentCdLookupBatchFeedback801C4780& batch);
void ResetStage1MovieSegmentCdLookupRuntime801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime);
bool RecordStage1MovieSegmentCdLookupSeam801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex,
    const PrStage1LoaderDirect::CdSeamResult& cd);
bool TryGetStage1MovieSegmentCdLookupBatch801C4780(
    const Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex,
    MovieSegmentCdLookupBatchFeedback801C4780& out);
bool RunStage1MovieSegmentCdLookupIsoProducer801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex,
    const std::filesystem::path& binPath,
    bool binPathKnown);
bool BuildStage1MovieSegmentCdLookupCarrier801C4780(
    Stage1MovieSegmentCdLookupRuntime801C4780& runtime,
    uint32_t sceneIndex,
    const std::filesystem::path& binPath,
    bool binPathKnown,
    const StreamClockProducerCarrier800493F4* streamClockCarrier,
    bool gapMissingStreamClockProducer,
    MovieSegmentCdLookupBatchFeedback801C4780& out);
MovieSegmentCdLookupBatchFeedback801C4780
RunMovieSegmentCdLookupProducer801C4780(
    const SceneEntryMovieSegmentTable801C4780& table,
    const std::filesystem::path& binPath,
    bool binPathKnown);
MovieSegmentCdLookupBatchFeedback801C4780
RunStage1StaticMovieSegmentCdLookupProducer801C4780(
    uint32_t sceneIndex,
    const std::filesystem::path& binPath,
    bool binPathKnown);
bool TryBuildStage1MovieSegmentRowFeedbackFromIso801C4780(
    uint32_t sceneIndex,
    uint32_t rowIndex,
    const std::filesystem::path& binPath,
    bool binPathKnown,
    MovieSegmentRowInitFeedback8001A324& out);
bool BuildOverlayTransferSeekFeedbackFromCdSeam8001A89C(
    const PrStage1LoaderDirect::CdSeamResult& cd,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown,
    PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C& out);
bool BuildOverlayTransferReadLowerFeedbackFromCdSeams8001A818(
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown,
    PrMovieSegmentDirect::OverlayTransferReadLowerFeedback8001A818& out);
bool BuildOverlayTransferAttemptFeedbackFromCdSeams8001ACF8(
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount,
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown,
    int32_t firstAttemptFlag,
    PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C& outSeek,
    PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818& outRead);
void TagOverlayTransferAttemptCdSeam8001ACF8(
    PrStage1LoaderDirect::CdSeamResult& cd,
    const PrMovieSegmentDirect::OverlayTransferAttemptProvenance8001ACF8&
        provenance);
MovieSegmentScanResult801C4780 PsxCall801C4780_ScanMovieSegments(
    const SceneEntryMovieSegmentTable801C4780& table);
MovieSegmentScanResult801C4780 PsxCall801C4780_ScanMovieSegmentsWithFeedback(
    const SceneEntryMovieSegmentTable801C4780& table,
    const MovieSegmentRowInitFeedback8001A324* feedback,
    uint32_t feedbackCount);
SceneEntrySegmentSelection801C4DC4 PsxSelectMovieSegment801C4DC4(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown);
SceneEntrySegmentSelection801C4DC4 PsxSelectMovieSegment801C4DC4FromScan(
    const MovieSegmentScanResult801C4780& scan);
MovieStepSegmentFields801C4350 BuildMovieStepSegmentFields801C4350(
    const MovieSegmentRecord48& record);

}  // namespace PrStage1MovieSegmentDirect
