#pragma once

#include "pr_movie_subtitles.h"
#include "pr_scn1.h"
#include "pr_psx_event_frame_direct.h"
#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_graph_owner_direct.h"
#include "pr_stage1_scene1_movie1_direct.h"
#include "pr_stage1_vtext_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace PrStage1MovieTextDirect {

inline constexpr size_t kMovie1CommonLyricsModeCount = 3u;
inline constexpr uint32_t kMovie1SubtitleDescAddr = 0x801CE080u;
inline constexpr uint32_t kMovie1SubtitleEntryCount = 41u;
inline constexpr std::array<uint32_t, kMovie1CommonLyricsModeCount> kStage1CommonLyricsDescAddrByMode = {
    kMovie1SubtitleDescAddr,
    0x801CE09Cu,
    0x801CE0B8u,
};
inline constexpr std::array<uint32_t, kMovie1CommonLyricsModeCount> kStage1CommonLyricsEntryCountByMode = {
    kMovie1SubtitleEntryCount,
    6u,
    6u,
};

struct PsxOverlayAddressResolver {
    uint32_t psxBase = 0;
    uint32_t fileOffsetBase = 0;
};

using ComodOverlayAddressResolver = PsxOverlayAddressResolver;

struct PsxVTextTrackLoadSpec {
    uint32_t descAddr = 0;
    uint32_t expectedEntryCount = 0;
};

using VTextTrackLoadSpec = PsxVTextTrackLoadSpec;

struct PsxVTextDescriptorState {
    bool valid = false;
    uint32_t descAddr = 0;
    uint32_t descOffset = 0;
    std::array<uint32_t, 5> textTableAddrs{};
    uint32_t selectedTextTableAddr = 0;
    uint32_t entriesAddr = 0;
    uint32_t entryCount = 0;
};

struct PsxMovieTextLoadSpec {
    const PsxOverlayAddressResolver* overlayResolvers = nullptr;
    size_t overlayResolverCount = 0;
    PsxVTextTrackLoadSpec movieSubtitle{};
    std::array<PsxVTextTrackLoadSpec, kMovie1CommonLyricsModeCount> commonLyrics{};
};

using Movie1TextLoadSpec = PsxMovieTextLoadSpec;

struct Movie1TextRuntime {
    PrMovieSubtitles::MovieSubtitleTrack subtitleTrack{};
    std::vector<PrStage1VTextDirectLineMeta> subtitleLineMeta{};
    PsxVTextDescriptorState subtitleDesc{};
    PrStage1Scene1Movie1Direct::SubtitleFrameWindow subtitleWindow{};
    std::array<PrMovieSubtitles::MovieSubtitleTrack, kMovie1CommonLyricsModeCount> commonLyricsTracks{};
    std::array<std::vector<PrStage1VTextDirectLineMeta>, kMovie1CommonLyricsModeCount>
        commonLyricsLineMetas{};
    std::array<PsxVTextDescriptorState, kMovie1CommonLyricsModeCount> commonLyricsDescs{};
    std::array<uint32_t, kMovie1CommonLyricsModeCount> commonLyricsDescAddrByMode{};
    PrStage1VTextDirectRuntime commonLyricsVText{};
    bool playAndWaitActiveSub801C77C0 = false;
    bool playAndWaitDescSelectedSub80024C84 = false;
    bool playAndWaitModeGapSub801C77C0 = false;
    uint8_t playAndWaitModeSub801C77C0 = 0;
    uint32_t playAndWaitDescAddrSub801C77C0 = 0;
    bool movie1DisplayFlushGate801D3044Known = false;
    bool movie1DisplayFlushGate801D3044 = false;
    uint32_t movie1DisplayFlushGateSourceFunction = 0;
    uint32_t movie1DisplayFlushGateSetupArgA2 = 0;
    PrStage1CommonLyricsSnapshot commonLyricsSnapshot{};
    bool commonLyricsSnapshotValid = false;
};

struct Movie1TextWindowInput {
    uint16_t word800916DC = 0;
    uint32_t movieFrame30 = 0;
    bool currentGp872SlotKnown = false;
    uint8_t currentGp872Slot = 0;
};

struct Movie1TextGlobalOptionsCarrier {
    bool subtitleEnabled = false;
    bool word800916DCKnown = false;
    uint16_t word800916DC = 0;
};

inline Movie1TextGlobalOptionsCarrier BuildGlobalOptionsCarrierFromWord800916DC(
    bool known,
    uint16_t word800916DC) {
    Movie1TextGlobalOptionsCarrier out{};
    out.subtitleEnabled = word800916DC != 0u;
    out.word800916DCKnown = known;
    out.word800916DC = word800916DC;
    return out;
}

struct Movie1TextScene1FrameInputSub801C77C0 {
    Movie1TextGlobalOptionsCarrier globalOptions{};
    int32_t movieFrame30 = 0;
    bool stageRunning = false;
    int32_t runnerFrame30 = 0;
    bool renderHalfStep = false;
    bool sceneEntryMemory164Known = false;
    uint32_t sceneEntryMemory164 = 0;
    bool inputMaskSub80035510Known = false;
    uint32_t inputMaskSub80035510 = 0;
    bool sub8001A750Known = false;
    uint32_t sub8001A750Result = 0;
    bool sub801C448CKnown = false;
    bool sub801C448CResult = false;
    bool currentGp872SlotKnown = false;
    uint8_t currentGp872Slot = 0;
};

enum class Movie1TextDisplayHelperSub8001EC54 : uint8_t {
    None = 0,
    Sub8001DB00_Text268,
    Sub8001CE30_NoSubtitleFrame,
};

inline constexpr size_t kMovie1TextDrawCommandCapacity = 8u;
inline constexpr uint32_t kMovie1TextOtBufferBasePsxAddr = 0x80087288u;
inline constexpr uint32_t kMovie1TextOtBufferStrideBytes = 20u;
inline constexpr uint32_t kMovie1TextGlyphLoopFunctionSub8001B954 = 0x8001B954u;
inline constexpr uint32_t kMovie1TextGlyphMetricTableBase8004945C = 0x8004945Cu;
inline constexpr uint32_t kMovie1TextGlyphMetricTableStride8004945C = 8u;
inline constexpr uint32_t kMovie1TextGlyphWidthTableByte80049460 = 0x80049460u;
inline constexpr uint16_t kMovie1TextGlyphCenteredBodyWidthSub8001B954 = 264u;
inline constexpr uint16_t kMovie1TextGlyphLineAdvanceYSub8001B954 = 15u;
inline constexpr uint16_t kMovie1TextGlyphPrioritySub8001B954 = 0u;
inline constexpr uint32_t kMovie1TextGlyphFastSpriteAttrSub8001B954 =
    0x50000040u;
inline constexpr uint16_t kMovie1TextGlyphFastSpriteClutXSub8001B954 = 256u;
inline constexpr size_t kMovie1TextGlyphLineCapacitySub8001B954 = 2u;
inline constexpr size_t kMovie1TextGlyphCommandCapacitySub8001B954 = 224u;
inline constexpr size_t kMovie1TextFinalizeSubmitCapacitySub8001C864 = 96u;

enum class Movie1TextDrawCommandKind : uint8_t {
    None = 0,
    AcquireDrawBufferSub8004019C,
    SelectDrawBufferSub80040F90,
    BindOtBufferSub80040CC8,
    SetTextOriginSub8001B730,
    SubmitTextFastSpriteSequenceSub8001B954,
    FinalizeTextSub8001C864,
    DrawNoSubtitleFrameSub8001CE30,
};

enum class Movie1TextGlyphLoopGapSub8001B954 : uint8_t {
    None = 0,
    MissingText,
    MissingGlyphMetricTables,
    LineCapacityExceeded,
    GlyphCommandCapacityExceeded,
    FastSpriteOwnerMissing,
    LocalFastSpriteRgbMissing,
};

enum class Movie1TextGlyphRgbEvidenceKindSub8001B954 : uint8_t {
    None = 0,
    StackLocalRgbUnresolved8001B954,
};

struct Movie1TextCurrentGp872WorkCarrier {
    bool usesCurrentGp872DrawBuffer = true;
    uint32_t workBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
    uint32_t workStrideBytes = kMovie1TextOtBufferStrideBytes;
    bool gp872SlotKnown = false;
    uint8_t gp872Slot = 0;
    bool workAddrKnown = false;
    uint32_t workAddr = 0;
    uint32_t workLastAddrOffset = 0x10u;
    bool drawOtagAddrKnown = false;
    uint32_t drawOtagAddr = 0;
};

struct Movie1TextGlyphRgbGapEvidenceSub8001B954 {
    bool valid = false;
    Movie1TextGlyphRgbEvidenceKindSub8001B954 kind =
        Movie1TextGlyphRgbEvidenceKindSub8001B954::None;
    uint32_t directTextCallerFunction = 0x8001DB00u;
    uint32_t directTextCallsite = 0x8001DB30u;
    uint32_t glyphFunction = kMovie1TextGlyphLoopFunctionSub8001B954;
    uint32_t compareGlyphFunction = 0x8001B744u;
    uint32_t globalFastSpriteFunction = 0x8001BCA0u;
    uint32_t globalFastSpriteObject = 0x8007CED0u;
    uint32_t submitFunction = 0x8003FA20u;
    uint8_t localSpriteR_14 = 0x14u;
    uint8_t localSpriteG_15 = 0x15u;
    uint8_t localSpriteB_16 = 0x16u;
    bool workOtOwnerKnown = false;
    bool localStackRgbWriteProven = false;
    bool dword8007CED0AppliesToThisStackLocal = false;
};

struct Movie1TextGlyphMetricSub8001B954 {
    bool valid = false;
    std::array<uint8_t, kMovie1TextGlyphMetricTableStride8004945C> raw{};
    int16_t word0_8004945C = 0;
    int16_t word_8004945E = 0;
    uint8_t advanceWidth = 0;
    uint8_t height = 0;
    int16_t word_80049462 = 0;
    bool skipsGlyphSubmit = false;
    bool localFastSpriteStaticFieldsKnown = false;
    bool localFastSpriteRgbKnown = false;
    int16_t texCoordRoundedV22 = 0;
    uint16_t texHelperA3 = 0;
    uint16_t texHelperA4 = 0;
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 localFastSprite{};
};

struct Movie1TextGlyphMetricTablesSub8001B954 {
    bool valid = false;
    uint32_t base_8004945C = kMovie1TextGlyphMetricTableBase8004945C;
    uint32_t stride = kMovie1TextGlyphMetricTableStride8004945C;
    uint32_t byte_80049460 = kMovie1TextGlyphWidthTableByte80049460;
    const Movie1TextGlyphMetricSub8001B954* glyphs = nullptr;
    size_t glyphCount = 0;
};

struct Movie1TextGlyphLineSub8001B954 {
    bool valid = false;
    uint8_t lineIndex = 0;
    uint16_t textStartIndex = 0;
    uint16_t textByteCount = 0;
    uint16_t drawableGlyphCount = 0;
    bool widthKnown = false;
    uint16_t widthPx = 0;
    bool centeredXKnown = false;
    int16_t centeredX = 0;
    int16_t y = 0;
};

struct Movie1TextGlyphCommandSub8001B954 {
    bool valid = false;
    uint32_t sourceFunction = kMovie1TextGlyphLoopFunctionSub8001B954;
    uint16_t textByteIndex = 0;
    uint8_t lineIndex = 0;
    uint8_t charCode = 0;
    bool newline = false;
    bool drawable = false;
    bool glyphMetricKnown = false;
    bool skipsGlyphSubmit = false;
    int16_t word0_8004945C = 0;
    int16_t word_8004945E = 0;
    uint8_t advanceWidth = 0;
    uint8_t height = 0;
    int16_t word_80049462 = 0;
    bool positionKnown = false;
    int16_t x = 0;
    int16_t y = 0;
    bool localSpritePositionKnown = false;
    int16_t localSpriteX = 0;
    uint16_t localSpriteY = 0;
    uint16_t arg1_480 = 0;
    uint16_t priority = kMovie1TextGlyphPrioritySub8001B954;
    bool localFastSpriteStaticFieldsKnown = false;
    bool localFastSpriteRgbKnown = false;
    int16_t texCoordRoundedV22 = 0;
    uint16_t texHelperA3 = 0;
    uint16_t texHelperA4 = 0;
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20 localFastSprite{};
    PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
        rawLocalBytes8003FA20{};
    bool emitsGsSortFastSprite = false;
    bool fastSpriteOwnerKnown = false;
    Movie1TextGlyphRgbGapEvidenceSub8001B954 rgbGapEvidence{};
};

struct Movie1TextGlyphSubmitResultSub8001B954 {
    bool valid = false;
    uint32_t sourceFunction = kMovie1TextGlyphLoopFunctionSub8001B954;
    uint16_t glyphCommandIndex = 0;
    bool wouldCall8003FA20 = false;
    bool fastSpriteOwnerKnown = false;
    bool currentWorkKnown = false;
    Movie1TextCurrentGp872WorkCarrier currentWork{};
    bool packetAllocatorKnown = false;
    uint32_t packetAllocatorAddr = 0;
    bool runtimeOwnerKnown = false;
    PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeBuildResult8003FA20
        runtimeBuild8003FA20{};
    bool localFastSpriteStaticFieldsKnown = false;
    bool localFastSpriteRgbKnown = false;
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
        localFastSprite{};
    PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
        rawLocalBytes8003FA20{};
    bool packetKnown = false;
    PrPsxFastSpriteSubmitDirect::GsSortFastSpritePartialResult8003FA20
        partial8003FA20{};
    Movie1TextGlyphLoopGapSub8001B954 gap =
        Movie1TextGlyphLoopGapSub8001B954::None;
    Movie1TextGlyphRgbGapEvidenceSub8001B954 rgbGapEvidence{};
};

struct Movie1TextFastSpriteSequenceSub8001B954 {
    bool valid = false;
    uint32_t sourceFunction = kMovie1TextGlyphLoopFunctionSub8001B954;
    PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20
        fastSpriteSourceKind =
            PrPsxFastSpriteSubmitDirect::
                FastSpriteSubmitSourceKind8003FA20::Stage1MovieText;
    uint32_t textPsxAddr = 0;
    const char* textPtr = nullptr;
    uint16_t strlenBytes = 0;
    uint16_t arg1_480 = 0;
    int16_t gp_752_originX = 0;
    int16_t gp_756_originY = 0;
    int16_t gp_748_originZ = 0;
    uint16_t centeredBodyWidth = kMovie1TextGlyphCenteredBodyWidthSub8001B954;
    uint16_t lineAdvanceY = kMovie1TextGlyphLineAdvanceYSub8001B954;
    uint32_t byte_80049460 = kMovie1TextGlyphWidthTableByte80049460;
    uint32_t metricTableBase_8004945C = kMovie1TextGlyphMetricTableBase8004945C;
    uint32_t metricTableStride = kMovie1TextGlyphMetricTableStride8004945C;
    bool glyphMetricTablesKnown = false;
    bool lineCapacityExceeded = false;
    bool glyphCommandCapacityExceeded = false;
    bool fastSpriteOwnerKnown = false;
    Movie1TextCurrentGp872WorkCarrier currentWork{};
    Movie1TextGlyphRgbGapEvidenceSub8001B954 rgbGapEvidence{};
    Movie1TextGlyphLoopGapSub8001B954 firstGap =
        Movie1TextGlyphLoopGapSub8001B954::None;
    std::array<Movie1TextGlyphLineSub8001B954,
               kMovie1TextGlyphLineCapacitySub8001B954>
        lines{};
    size_t lineCount = 0;
    std::array<Movie1TextGlyphCommandSub8001B954,
               kMovie1TextGlyphCommandCapacitySub8001B954>
        glyphCommands{};
    size_t glyphCommandCount = 0;
    std::array<Movie1TextGlyphSubmitResultSub8001B954,
               kMovie1TextGlyphCommandCapacitySub8001B954>
        glyphSubmitResults{};
    size_t glyphSubmitResultCount = 0;
};

struct Movie1TextFastSpriteSequenceApplyResultSub8001B954 {
    bool valid = false;
    size_t appliedSubmitCount = 0;
};

struct Movie1TextOriginStateSub8001B730 {
    bool valid = false;
    uint32_t sourceFunction = 0x8001B730u;
    uint32_t gpOriginZOffset = 748u;
    uint32_t gpOriginXOffset = 752u;
    uint32_t gpOriginYOffset = 756u;
    int16_t gp_752_originX = 0;
    int16_t gp_756_originY = 0;
    int32_t gp_748_originZ = 0;
};

struct Movie1TextFinalizeSubmitSub8001C864 {
    bool valid = false;
    uint32_t sourceFunction = 0x8001C864u;
    uint32_t submittedHelper = 0x8001B590u;
    bool rawDrawSubmitCarrierKnown = false;
    uint16_t callOrder = 0;
    int16_t screenX = 0;
    int16_t screenY = 0;
    uint32_t templateAddr = 0;
    uint16_t glyphIndex = 0;
    bool addGlyphIndexToFinalWord = false;
    uint16_t priority = 0;
    bool usesCurrentGp872DrawBuffer = true;
    uint32_t workBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
    uint32_t workStrideBytes = kMovie1TextOtBufferStrideBytes;
    Movie1TextCurrentGp872WorkCarrier currentWork{};
};

struct Movie1TextFinalizeStateSub8001C864 {
    bool valid = false;
    uint32_t sourceFunction = 0x8001C864u;
    uint16_t arg0_priority = 0;
    uint32_t submittedHelper = 0x8001B590u;
    bool usesCurrentGp872DrawBuffer = true;
    uint32_t workBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
    uint32_t workStrideBytes = kMovie1TextOtBufferStrideBytes;
    bool fastSpriteSubmitOwnerKnown = false;
    Movie1TextCurrentGp872WorkCarrier currentWork{};
    std::array<Movie1TextFinalizeSubmitSub8001C864,
               kMovie1TextFinalizeSubmitCapacitySub8001C864>
        submits{};
    size_t submitCount = 0;
};

struct Movie1TextDisplayChainSub8001DB00 {
    bool valid = false;
    uint32_t sourceFunction = 0x8001DB00u;
    uint32_t textPsxAddr = 0;
    const char* textPtr = nullptr;
    bool usesCurrentGp872DrawBuffer = true;
    uint32_t workBasePsxAddr = kMovie1TextOtBufferBasePsxAddr;
    uint32_t workStrideBytes = kMovie1TextOtBufferStrideBytes;
    uint32_t callOrderSub8001B730 = 0;
    uint32_t callOrderSub8001B954 = 1;
    uint32_t callOrderSub8001C864 = 2;
    Movie1TextOriginStateSub8001B730 origin{};
    uint16_t glyphScaleArg1 = 480;
    uint8_t finalizeArg0 = 5;
};

struct Movie1TextDrawCommand {
    Movie1TextDrawCommandKind kind = Movie1TextDrawCommandKind::None;
    uint32_t psxFunctionAddr = 0;
    PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20
        fastSpriteSourceKind =
            PrPsxFastSpriteSubmitDirect::
                FastSpriteSubmitSourceKind8003FA20::Stage1MovieText;
    uint8_t ctxMode = 0;
    bool usesCurrentGp872DrawBuffer = false;
    uint32_t otBufferBasePsxAddr = 0;
    uint32_t otBufferStrideBytes = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t z = 0;
    uint16_t scale = 0;
    uint8_t arg0 = 0;
    uint8_t arg1 = 0;
    const char* textPtr = nullptr;
    uint32_t textPsxAddr = 0;
    bool emitsGsSortFastSprite = false;
    uint16_t centeredLineWidth = 264;
    uint32_t glyphWidthTablePsxAddr = kMovie1TextGlyphMetricTableBase8004945C;
    bool glyphSequenceBuiltSub8001B954 = false;
    Movie1TextOriginStateSub8001B730 originStateSub8001B730{};
    Movie1TextFinalizeStateSub8001C864 finalizeStateSub8001C864{};
};

struct Movie1TextDisplayActionSub8001EC54 {
    bool requested = false;
    uint8_t ctxMode = 7;
    uint16_t word800916DC = 0;
    Movie1TextCurrentGp872WorkCarrier currentWork{};
    bool text268Known = false;
    uint32_t text268PsxAddr = 0;
    const char* text268Ptr = nullptr;
    uint16_t textDrawXSub8001DB00 = 24;
    uint16_t textDrawYSub8001DB00 = 184;
    uint16_t textDrawScaleSub8001DB00 = 480;
    uint8_t textDrawFinalizeArgSub8001DB00 = 5;
    uint8_t noSubtitleFrameArgSub8001CE30 = 5;
    Movie1TextDisplayHelperSub8001EC54 helper =
        Movie1TextDisplayHelperSub8001EC54::None;
    std::array<Movie1TextDrawCommand, kMovie1TextDrawCommandCapacity> drawCommands{};
    size_t drawCommandCount = 0;
    Movie1TextDisplayChainSub8001DB00 displayChainSub8001DB00{};
    Movie1TextFastSpriteSequenceSub8001B954 textFastSpriteSequenceSub8001B954{};
};

struct Movie1TextFlushActionSub8001ED3C {
    bool requested = false;
    Movie1TextCurrentGp872WorkCarrier currentWork{};
};

struct Movie1TextWindowTickResult {
    Movie1TextDisplayActionSub8001EC54 displayActionSub8001EC54Ctx7{};
    Movie1TextFlushActionSub8001ED3C flushActionSub8001ED3C{};
    bool commonLyricsSnapshotValid = false;
    PrStage1CommonLyricsSnapshot commonLyricsSnapshot{};
};

struct Movie1TextCommonLyricsHudCarrierSub8001EC54 {
    bool valid = false;
    bool displayActionRequested = false;
    bool displayActionUsesText268 = false;
    bool hasTextGeometrySub8001B954 = false;
    PrStage1CommonLyricsTextGeometrySub8001B954 textGeometrySub8001B954{};
    bool hasFrameSub8001C864 = false;
    PrStage1CommonLyricsFrameSub8001C864 frameSub8001C864{};
    PrStage1CommonLyricsSnapshot commonLyrics{};
};

uint8_t ResolveLanguageIndex(int languageIndex);

void ResetSub80024C84(Movie1TextRuntime& runtime);
void ResetPlaybackStateSub801C77C0(Movie1TextRuntime& runtime);
void ApplyPrStrPlayerInitSub801C7744(Movie1TextRuntime& runtime,
                                     uint32_t initArgA2);
void BeginPlayAndWaitSub801C77C0(Movie1TextRuntime& runtime,
                                 uint8_t mode,
                                 uint8_t languageIndex);
void EndPlayAndWaitSub801C77C0(Movie1TextRuntime& runtime);
void LoadFromComodSub80024C84(Movie1TextRuntime& runtime,
                              const std::vector<uint8_t>& comodBytes,
                              uint32_t ptr800943CC);
void LoadFromComodSub80024C84(Movie1TextRuntime& runtime,
                              const std::vector<uint8_t>& comodBytes,
                              const PsxMovieTextLoadSpec& loadSpec);
uint32_t GetCommonLyricsDescAddrSub80024C84(const Movie1TextRuntime& runtime,
                                            uint8_t mode);

const PrMovieSubtitles::MovieSubtitleTrack& GetMovieSubtitleTrack(const Movie1TextRuntime& runtime);
const PrMovieSubtitles::MovieSubtitleTrack& GetActiveMovieSubtitleTrack(
    const Movie1TextRuntime& runtime);
PrStage1Scene1Movie1Direct::SubtitleFrameWindow
GetActiveMovieSubtitleFrameWindow(const Movie1TextRuntime& runtime);
void ApplyMovieSubtitleWindow(Movie1TextRuntime& textRuntime,
                              PrStage1Scene1Movie1Direct::Movie1RuntimeState& movieRuntime);

void AdvanceCommonLyricsSub80024CF8(Movie1TextRuntime& runtime,
                                    uint8_t mode,
                                    uint8_t languageIndex,
                                    uint32_t queryFrame30);
Movie1TextFastSpriteSequenceSub8001B954 BuildTextFastSpriteSequenceSub8001B954(
    const Movie1TextDrawCommand& command,
    const Movie1TextGlyphMetricTablesSub8001B954& metrics,
    const Movie1TextCurrentGp872WorkCarrier& currentWork);
Movie1TextFastSpriteSequenceApplyResultSub8001B954
ApplyTextFastSpriteSequenceSub8001B954(
    const Movie1TextFastSpriteSequenceSub8001B954& sequence,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime);
Movie1TextFastSpriteSequenceApplyResultSub8001B954
ApplyMovieTextWindowSubmitSub801C77C0(
    Movie1TextWindowTickResult& tickResult,
    PrPsxGraphOwnerDirect::PsxGraphState& graph,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* fastSpriteRuntime);
Movie1TextWindowTickResult TickMovie1TextWindowSub801C77C0(
    Movie1TextRuntime& runtime,
    const Movie1TextWindowInput& input,
    uint8_t languageIndex,
    uint32_t queryFrame60);
Movie1TextWindowTickResult
BuildMovie1TextWindowAfterOuterAdvanceSub801C455C(
    Movie1TextRuntime& runtime,
    const Movie1TextScene1FrameInputSub801C77C0& frameInput,
    uint8_t languageIndex);
Movie1TextCommonLyricsHudCarrierSub8001EC54
BuildCommonLyricsHudCarrierSub8001EC54(
    const Movie1TextWindowTickResult& tickResult,
    const PrStage1CommonLyricsSnapshot& commonLyrics);
bool GetCommonLyricsSnapshot(const Movie1TextRuntime& runtime,
                             PrStage1CommonLyricsSnapshot& out);

}  // namespace PrStage1MovieTextDirect

#include "pr_stage1_movie_text_outer_loop_direct.h"
