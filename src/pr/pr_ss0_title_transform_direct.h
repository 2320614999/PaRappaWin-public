#pragma once

#include "pr_mime.h"
#include "pr_psx_gte_direct.h"
#include "pr_tmd.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0TitleTransformDirect {

enum class ModelPath : uint8_t {
    Pa = 0,
    PaKage = 1,
    Lo = 2,
    Hp = 3,
};

enum class Failure : uint8_t {
    None = 0,
    GteControlUnknown,
    GteControlMismatch,
    NullObject,
    SourceParseIncomplete,
    PrimitiveIndexOutOfRange,
    RawPacketUnknown,
    UnsupportedPrimitiveShape,
    VertexIndexOutOfRange,
    UnsupportedModelPath,
    BaseTriangleUnknown,
    RuntimeMimeBindingUnknown,
    RuntimeMimeCursorUnknown,
    RuntimeMimeLoopFlagUnknown,
    UnsupportedMimeLoopFlag,
    VdfSourceUnknown,
    DatSourceUnknown,
    MimeKeyCountMismatch,
    VdfShapeMismatch,
    DatShapeMismatch,
    MimeObjectSourceMissing,
    RuntimeMatrixUnknown,
    DeformedTriangleUnknown,
    DrawCoordWorldUnknown,
    PanelWithCameraUnknown,
    RuntimeMatrixOverflow,
    TitleTodBindingUnknown,
    TitleTodCursorUnknown,
    TitleTodSourceUnknown,
    TitleTodSourceIncomplete,
    TitleTodCursorOutOfRange,
    TitleTodCommandUnsupported,
    TitleTodCommandMalformed,
    TitleTodCursorOverflow,
    TitlePanelCoordInputUnknown,
    TitlePanelCoordFrameCountInvalid,
    TitlePanelCoordOverflow,
    TitlePanelCoordUnknown,
    TitleCameraMatrixUnknown,
    TitleCameraSourceUnknown,
    TitleCameraSourceInvalid,
    TitleCameraCursorUnknown,
    TitleCameraCursorOutOfRange,
    TitleCameraVectorOverflow,
    TitleCameraDegenerateView,
    TitleCameraTwistUnsupported,
    TitleCameraSuperUnsupported,
};

inline constexpr std::size_t kScene0Comod0ExpectedBytes = 49792u;
inline constexpr std::size_t kTitleCameraTableOffset801C6FB4 = 0x3744u;
inline constexpr std::size_t kTitleCameraRecordCount801C6FB4 = 300u;
inline constexpr std::size_t kTitleCameraRecordStride801C6FB4 = 0x20u;
inline constexpr std::size_t kTitleCameraTableEnd801C6FB4 =
    kTitleCameraTableOffset801C6FB4 +
    kTitleCameraRecordCount801C6FB4 *
        kTitleCameraRecordStride801C6FB4;

struct TitleCameraView801C6FB4 {
    bool known = false;
    std::array<int32_t, 3> position{};
    std::array<int32_t, 3> target{};
    int32_t twist = 0;
    uint32_t superAddress = 0;
};

struct TitleCameraTable801C6FB4 {
    bool known = false;
    std::array<TitleCameraView801C6FB4,
               kTitleCameraRecordCount801C6FB4>
        views{};
};

struct TitleCameraCursorState801CB59C {
    bool cursorKnown = false;
    uint32_t cursor = 0;
};

struct TitleCameraMatrixResult80041D3C {
    Failure failure = Failure::None;
    bool cameraMatrixKnown80092880 = false;
    PrPsxGteDirect::Matrix3x4 cameraMatrix80092880{};
};

struct TitleCameraAdvanceResult801C6410 {
    Failure failure = Failure::None;
    bool accepted = false;
    bool sourceExhausted = false;
    uint32_t sampledIndex = 0;
    TitleCameraCursorState801CB59C nextState{};
    bool cameraMatrixKnown80092880 = false;
    PrPsxGteDirect::Matrix3x4 cameraMatrix80092880{};
};

struct Mode25BaseTriangleCarrier {
    bool known = false;
    PrPsxGteDirect::GteControlState control{};
    std::array<PrPsxGteDirect::VertexS16, 3> baseVertices{};
    std::array<uint16_t, 3> vertexIndices{};
    uint32_t objectIndex = 0;
    uint32_t objectVertexCount = 0;
    std::size_t primitiveIndex = 0;
    ModelPath path = ModelPath::Pa;
    bool runtimeMimeBindingKnown = false;
    bool runtimeMimeCursorKnown = false;
    bool runtimeVertexDeformationKnown = false;
    bool runtimeMatrixKnown = false;
    bool rtpt3InputKnown = false;
};

struct BaseCarrierResult {
    Failure failure = Failure::None;
    Mode25BaseTriangleCarrier carrier{};
    bool baseCarrierReady = false;
};

struct MimeRuntimeInput80013EA8 {
    bool bindingKnown = false;
    const VdfData* vdf = nullptr;
    const DatData* dat = nullptr;
    uint32_t objectIndex = 0;
    bool sampleFrameKnown = false;
    uint16_t sampleFrame = 0;
    bool loopFlagKnown = false;
    uint16_t loopFlag = 0;
};

struct Mode25DeformedTriangleCarrier {
    bool known = false;
    PrPsxGteDirect::GteControlState control{};
    std::array<PrPsxGteDirect::VertexS16, 3> vertices{};
    std::array<uint16_t, 3> vertexIndices{};
    std::size_t primitiveIndex = 0;
    ModelPath path = ModelPath::Pa;
    uint16_t sampleFrame = 0;
};

struct DeformationResult {
    Failure failure = Failure::None;
    Mode25DeformedTriangleCarrier carrier{};
    bool deformedTriangleReady = false;
};

struct TitleRuntimeMatrixInput801C5E60 {
    bool drawCoordWorldKnown800417A4 = false;
    PrPsxGteDirect::Matrix3x4 drawCoordWorld800417A4{};
    bool panelWithCameraKnown80041A68 = false;
    PrPsxGteDirect::Matrix3x4 panelWithCamera80041A68{};
};

struct TitleRuntimeMatrixResult801C5E60 {
    Failure failure = Failure::None;
    bool runtimeMatrixKnown = false;
    PrPsxGteDirect::Matrix3x4 runtimeMatrix800406D8{};
};

struct TitleTodCursorState8001B000 {
    bool bindingKnown = false;
    uint8_t resourceHandle = 0;
    bool sampleSeqKnown = false;
    uint32_t sampleSeq = 0;
    uint32_t blockIndex = 0;
    bool drawCoordKnown = false;
    TodCoordMatrix drawCoord{};
};

struct TitleTodAdvanceResult8001B000 {
    Failure failure = Failure::None;
    bool accepted = false;
    bool blockAdvanced = false;
    bool sourceExhausted = false;
    TitleTodCursorState8001B000 nextState{};
    bool drawCoordWorldKnown800417A4 = false;
    PrPsxGteDirect::Matrix3x4 drawCoordWorld800417A4{};
};

struct TitleCoordWithCameraInput80041A68 {
    bool drawCoordWorldKnown800417A4 = false;
    PrPsxGteDirect::Matrix3x4 drawCoordWorld800417A4{};
    bool cameraMatrixKnown80092880 = false;
    PrPsxGteDirect::Matrix3x4 cameraMatrix80092880{};
};

struct TitleCoordWithCameraResult80041A68 {
    Failure failure = Failure::None;
    bool coordWithCameraKnown80041A68 = false;
    PrPsxGteDirect::Matrix3x4 coordWithCamera80041A68{};
};

struct TitlePanelCoordInput801C5D8C {
    bool known = false;
    int32_t deltaX = 0;
    int32_t totalFrames = 0;
    int32_t deltaY = 0;
};

struct TitlePanelCoordResult801C5D8C {
    Failure failure = Failure::None;
    bool panelCoordKnown801CD7BC = false;
    PrPsxGteDirect::Matrix3x4 panelCoord801CD7BC{};
};

struct TitlePanelWithCameraInput80041A68 {
    bool panelCoordKnown801CD7BC = false;
    PrPsxGteDirect::Matrix3x4 panelCoord801CD7BC{};
    bool cameraMatrixKnown80092880 = false;
    PrPsxGteDirect::Matrix3x4 cameraMatrix80092880{};
};

struct TitlePanelWithCameraResult80041A68 {
    Failure failure = Failure::None;
    bool panelWithCameraKnown80041A68 = false;
    PrPsxGteDirect::Matrix3x4 panelWithCamera80041A68{};
};

struct Mode25Rtpt3Input {
    bool known = false;
    PrPsxGteDirect::Matrix3x4 matrix{};
    PrPsxGteDirect::GteControlState control{};
    std::array<PrPsxGteDirect::VertexS16, 3> vertices{};
    std::array<uint16_t, 3> vertexIndices{};
    std::size_t primitiveIndex = 0;
    ModelPath path = ModelPath::Pa;
    uint16_t sampleFrame = 0;
};

struct Rtpt3InputResult {
    Failure failure = Failure::None;
    Mode25Rtpt3Input input{};
    bool ready = false;
};

struct Rtpt3ExecutionResult800428B0 {
    Failure failure = Failure::None;
    Mode25Rtpt3Input input{};
    PrPsxGteDirect::Rtpt3ExactOutput280030 output{};
    PrPsxGteDirect::Mode25TriangleGeometryTrace geometry{};
    bool geometryReady = false;
    bool ready = false;
};

bool IsExactTitleGteControl801C609C(
    const PrPsxGteDirect::GteControlState& control);

// Extracts the only COORD command used by COMPO00 PA_LOC2.TOD. The parser is
// deliberately strict so another TOD cannot silently become title authority.
bool ExtractPaLoc2Coord80028054(const TodData& tod, TodCoordMatrix& outCoord);

// Parses the embedded Scene0 GsRVIEW2 table at 801C6FB4 from the exact
// original COMOD0 overlay image. The overlay is immutable PSX data only; no
// old Scene0 callbacks, types, or runtime state are retained.
bool ParseExactTitleCameraTable801C6FB4(
    const uint8_t* comod0,
    std::size_t byteCount,
    TitleCameraTable801C6FB4& outTable);

// Reproduces the natural Scene0 subset of 80041D3C used by all 300 embedded
// camera rows. The exact table has twist=0 and super=null; unsupported branches
// remain fail-closed instead of borrowing the Stage1 camera path.
TitleCameraMatrixResult80041D3C BuildTitleCameraMatrix80041D3C(
    const TitleCameraView801C6FB4& view);

// Consumes one 801C6FB4 row and increments 801CB59C only after the view matrix
// is completely known. Event-driven reset is owned by the backend commit.
TitleCameraAdvanceResult801C6410 AdvanceTitleCamera801C6410(
    const TitleCameraCursorState801CB59C& state,
    const TitleCameraTable801C6FB4* table);

// Reproduces the title draw descriptor's 8001B000 -> 80028504 -> 80028054
// type-4 COORD path. The caller owns the event-selected TOD handle and the
// explicit draw sequence. Camera/viewport commands are intentionally not
// promoted into matrix authority here.
TitleTodAdvanceResult8001B000 AdvanceTitleTodDrawCoord8001B000(
    const TitleTodCursorState8001B000& state,
    const TodData* tod);

// Reproduces the standard 8001B084 PA matrix path as
// cameraMatrix80092880 * drawCoordWorld800417A4. It does not consume the
// PA_KAGE panel or its 801C5E60 step.
TitleCoordWithCameraResult80041A68 BuildTitleCoordWithCamera80041A68(
    const TitleCoordWithCameraInput80041A68& input);

// Reproduces the root panel COORD writes made by
// 801C5D8C(-163, 204, 192). Inputs remain explicit so the function cannot
// silently become a generic or replay-derived matrix source.
TitlePanelCoordResult801C5D8C BuildTitlePanelCoord801C5D8C(
    const TitlePanelCoordInput801C5D8C& input);

// Reproduces the root-only Scene0 use of 80041A68: the explicit GsWSMATRIX is
// composed with the explicit panel COORD. It does not manufacture camera
// state, traverse an implicit host hierarchy, or consume replay samples.
TitlePanelWithCameraResult80041A68 BuildTitlePanelWithCamera80041A68(
    const TitlePanelWithCameraInput80041A68& input);

// Retains only resource-time facts. Runtime MIMe binding/cursor and the matrix
// produced by 800417A4/80041A68/800406D8 remain explicitly unknown.
BaseCarrierResult BuildMode25BaseTriangleCarrier801C5E60(
    const PrPsxGteDirect::GteControlState* control,
    const TmdObject* object,
    uint32_t objectIndex,
    std::size_t primitiveIndex,
    bool sourceParseComplete,
    ModelPath path);

// Reproduces the non-interpolating 80013D10/80013DB8/80013AA8 title path for
// an explicitly known active VDF/DAT binding and cursor. It starts from the
// immutable TMD base triangle on every call, matching the PSX channel reset.
DeformationResult ApplyMode25Mime80013EA8(
    const Mode25BaseTriangleCarrier& base,
    const MimeRuntimeInput80013EA8& runtime);

// Reproduces the final 801C5E60 matrix step only after the two preceding
// coordinate producers are explicit. It does not manufacture the draw COORD,
// panel COORD, or camera/view state.
TitleRuntimeMatrixResult801C5E60 BuildTitleRuntimeMatrix801C5E60(
    const TitleRuntimeMatrixInput801C5E60& input);

// Joins deformed vertices with the per-frame matrix only after both producers
// are authoritative. This is the sole API that can publish RTPT3 input known.
Rtpt3InputResult BuildMode25Rtpt3Input801C5E60(
    const Mode25DeformedTriangleCarrier& deformed,
    const PrPsxGteDirect::Matrix3x4* runtimeMatrix,
    bool runtimeMatrixKnown);

// Executes the exact three-vertex GTE operation only after the runtime MIMe
// and matrix producers have been joined. It preserves the joined provenance
// and immediately derives authoritative NCLIP/AVSZ3 geometry. It does not
// build packets, mutate graph state, or consume replay bytes.
Rtpt3ExecutionResult800428B0 ExecuteMode25Rtpt3At800428B0(
    const Mode25Rtpt3Input& input);

} // namespace PrSS0TitleTransformDirect
