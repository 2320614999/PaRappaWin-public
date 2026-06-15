#pragma once

#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_gs_sprite_submit_direct.h"
#include "pr_stage_scene_submit_runtime_private.h"

#include <array>
#include <cstddef>
#include <cstdint>

struct TmdModel;
struct DatData;
struct VdfData;

namespace PrStage1Scene1Movie1Direct {
struct Movie1DrawPlan;
}

namespace PrStageSceneSubmitDirect {

struct PsxDrawModelRef {
    enum class Set : uint8_t {
        Stage1Main,
        Stage1SceneMap,
    };
    Set set = Set::Stage1Main;
    uint16_t index = 0;
};

struct PsxDrawTriangleCommand428B0 {
    std::array<float, 3> x{};
    std::array<float, 3> y{};
    std::array<uint8_t, 3> u{};
    std::array<uint8_t, 3> v{};
    bool textured = false;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    std::array<float, 3> r{{1.0f, 1.0f, 1.0f}};
    std::array<float, 3> g{{1.0f, 1.0f, 1.0f}};
    std::array<float, 3> b{{1.0f, 1.0f, 1.0f}};
    std::array<float, 3> a{{1.0f, 1.0f, 1.0f}};
    std::array<float, 3> perspectiveW{{1.0f, 1.0f, 1.0f}};
    uint8_t blend = 0;
};

enum class PsxGpuPacketHandler428B0 : uint8_t {
    Unsupported,
    F3NL,
    NF3,
    TF3NL,
    TNF3,
    F4NL,
    NF4,
    TF4NL,
    TNF4,
    G3NL,
    NG3,
    TG3NL,
    TNG3,
    G4NL,
    NG4,
    TG4NL,
    TNG4,
};

enum class PsxGpuPacketDispatchGap428B0 : uint8_t {
    None,
    Flag4SpecialTableUnsupported,
};

struct PsxGpuPacketCommand428B0 {
    bool valid = false;
    PsxGpuPacketHandler428B0 handler =
        PsxGpuPacketHandler428B0::Unsupported;
    bool dispatchKnown800428B0 = false;
    PsxGpuPacketDispatchGap428B0 dispatchGap800428B0 =
        PsxGpuPacketDispatchGap428B0::None;
    uint8_t primitiveCode = 0;
    uint8_t primitiveFlag = 0;
    bool primitiveFlag4Path = false;
    bool rawPacketKnown800428B0 = false;
    uint32_t rawPrimitiveIndex800428B0 = 0;
    uint32_t rawPacketOffset800428B0 = 0;
    uint32_t rawPacketByteSize800428B0 = 0;
    uint8_t handlerTableSlot8001C1E8 = 0;
    uint32_t handlerTableEntryAddress8001C1E8 = 0;
    bool specialTablePath800428B0 = false;
    uint32_t specialTableBase800428B0 = 0;
    bool specialTableWriterKnown800428B0 = false;
    bool handlerTargetKnown800428B0 = false;
    uint32_t handlerTarget800428B0 = 0;
    uint16_t primitiveCursorAdvanceHalfwords800428B0 = 0;
    uint8_t wordCount = 0;
    std::array<uint32_t, 13> words{};
    uint16_t otz = 0;
    bool projectedDepthKnown800428B0 = false;
    uint8_t projectedVertexCount800428B0 = 0;
    std::array<int32_t, 4> projectedDepth800428B0{};
    bool projectedScreenKnown800428B0 = false;
    std::array<float, 4> projectedScreenX800428B0{};
    std::array<float, 4> projectedScreenY800428B0{};
    std::array<float, 4> projectedScreenZ800428B0{};
    int otBucket = 0;
    uint8_t workListFlushOrder = 0;
    uint32_t psxCallOrder = 0;
    bool semiTransparent = false;
    uint8_t abr = 0;
    bool sourceModelRefKnown800428B0 = false;
    uint8_t sourceModelSet800428B0 = 0;
    uint16_t sourceModelIndex800428B0 = 0;
};

struct PsxBackdropGradientCommand801CAC34 {
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    uint32_t topRgb = 0;
    uint32_t bottomRgb = 0;
};

struct PsxBackdropSpriteCommand801CAC34 {
    enum class Texture : uint8_t {
        YuKumo,
        YuYama,
        YuHi,
        OnKao,
        YuJhan,
    };
    Texture texture = Texture::YuKumo;
    float x = 0.0f;
    float y = 0.0f;
    float alpha = 1.0f;
};

struct PsxDrawEnvClearColorCommand80040060 {
    bool valid = false;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct PsxTimUploadCommand801CBFDC {
    uint8_t channel = 0;
    uint16_t timId = 0;
    bool uploadClut = false;
};

enum class PsxPreventTransitionSetup801CBFDC : uint8_t {
    None,
    FillActiveTiles8001FCBC,
    ClearActiveTiles8001FC40,
};

struct PsxPreventTransitionBoundary801CBFDC {
    PsxPreventTransitionSetup801CBFDC setup =
        PsxPreventTransitionSetup801CBFDC::None;
    int setupArg0 = 0;
    int setupArg1 = 0;
    bool drawCall8001FDC0 = false;
    int drawArg0 = 0;
};

struct PsxScriptBoxCommand80023E10 {
    const char* textPtr10C = nullptr;
    bool textPtr10CKnown = false;
    uint16_t maxGlyphs = 480;
    bool workListSlotKnown = false;
    uint8_t workListSlot = 0;
    uint32_t workListBaseAddr = 0x80087288u;
    uint32_t workListStrideBytes = 20u;
    bool workListAddrKnown = false;
    uint32_t workListAddr = 0;
    bool fastSpriteRuntimeOwnerKnown8003FA20 = false;
    bool glyphPacketRuntimeCommitted8003FA20 = false;
};

struct PsxNumericStatusCommand8001E2E4 {
    int16_t layoutArg68 = 0;
    int16_t uiRenderBlock4E = 0;
    uint16_t callGate64 = 0;
};

struct PsxFastSpritePacketCommand8003FA20 {
    bool valid = false;
    uint32_t packetAddr = 0;
    uint32_t wordCount = 0;
    std::array<uint32_t, 6> words{};
    std::array<bool, 6> wordKnown{};
    bool provenanceKnown = false;
    uint8_t sourceKind = 0;
    uint8_t rawLocalProvenance = 0;
    uint32_t sourceFunction = 0;
    uint32_t callsite = 0;
    uint32_t helper = 0;
    uint32_t psxCallOrder = 0;
    uint32_t localObjectAddr = 0;
    uint32_t lastWriterPc = 0;
    uint16_t priority = 0;
    bool allWordsKnown = false;
    bool renderPayloadKnown = false;
    bool auditOnlyPartial = false;
    bool geometryKnown = false;
    bool drawEnvOffsetKnown800401AC = false;
    int16_t drawEnvOffsetX80091738 = 0;
    int16_t drawEnvOffsetY8009173A = 0;
    bool colorKnown = false;
    bool colorAuthoritative = false;
    bool word2CommandKnown = false;
    uint8_t word2CommandCode = 0;
    bool rawTextureKnown = false;
    bool rawTexture = false;
    bool textureKnown = false;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct PsxGsSpritePacketCommand8003F1B4 {
    bool valid = false;
    uint32_t packetAddr = 0;
    uint32_t wordCount = 0;
    std::array<uint32_t,
               PrPsxGsSpriteSubmitDirect::
                   kGsSortSpritePacketMaxTotalWords8003F1B4>
        words{};
    std::array<bool,
               PrPsxGsSpriteSubmitDirect::
                   kGsSortSpritePacketMaxTotalWords8003F1B4>
        wordKnown{};
    uint32_t psxCallOrder = 0;
    bool priorityKnown = false;
    uint16_t priority = 0;
    bool allWordsKnown = false;
    bool renderPayloadKnown = false;
    bool geometryKnown = false;
    bool colorKnown = false;
    bool textureKnown = false;
    bool transformPath = false;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    std::array<uint8_t, 4> u{};
    std::array<uint8_t, 4> v{};
    std::array<int16_t, 4> x{};
    std::array<int16_t, 4> y{};
    uint16_t w = 0;
    uint16_t h = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct PsxBoxFillPacketCommand8003EE84 {
    bool valid = false;
    uint32_t packetAddr = 0;
    uint32_t psxCallOrder = 0;
    bool provenanceKnown = false;
    uint8_t sourceKind = 0;
    uint32_t sourceFunction = 0;
    uint16_t priority = 0;
    bool wordsKnown = false;
    uint32_t word0LinkTag = 0;
    uint32_t word1DrawMode = 0;
    uint32_t word2ColorCode = 0;
    uint32_t word3Xy = 0;
    uint32_t word4Wh = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t commandCode = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t w = 0;
    uint16_t h = 0;
};

constexpr std::size_t kCompactRailMaxRows80024744 = 2;
constexpr std::size_t kCompactRailBodySlotCount80024744 = 18;

struct PsxCompactRailRow80024744 {
    int16_t teacherRaw8C = -1;
    int16_t studentRaw9E = -1;
    uint32_t bodyStreamPtr94 = 0;
    std::array<int8_t, kCompactRailBodySlotCount80024744> bodyStreamBytes94{};
};

struct PsxCompactRailCommand80024744 {
    int16_t painterGate7A = 0;
    int16_t rowCount8A = 0;
    std::array<PsxCompactRailRow80024744, kCompactRailMaxRows80024744> rows{};
    bool resetWobbleBanks = false;
    bool resetTeacherWobbleBank80024308 = false;
    bool resetStudentWobbleBank80024390 = false;
};

struct PsxSpriteTemplate80024744 {
    uint32_t attr = 0;
    uint16_t texX_hw = 0;
    uint16_t texY_px = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    uint16_t clutX_px = 0;
    uint16_t clutY_px = 0;
};

struct PsxCompactRailSpriteCommand80024744 {
    enum class Anchor : uint8_t {
        TopLeft,
        Center,
    };
    enum class Kind : uint8_t {
        TeacherPortrait,
        StudentPortrait,
        LargeMarker,
        SmallMarker,
        Note,
    };

    Kind kind = Kind::Note;
    Anchor anchor = Anchor::TopLeft;
    PsxSpriteTemplate80024744 tpl{};
    float x = 0.0f;
    float y = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    uint8_t otBucket = 0;
    uint8_t row = 0;
    uint8_t slot = 0;
    int16_t raw = 0;
    bool lit = false;
};

struct PsxTodSideEffectStats80028054 {
    uint32_t observedCallCount8001B000 = 0;
    uint32_t lastSeq8001B000 = 0;
    uint32_t lastCountAddr8001B000 = 0;
    uint32_t lastCursorAddr8001B000 = 0;
    uint32_t lastDescAddr8001B000 = 0;
    uint16_t lastCommandCount80028504 = 0;
    uint16_t lastCommandObjectId80028054 = 0;
    uint16_t lastCommandType80028054 = 0;
    uint16_t lastCommandFlags80028054 = 0;
    bool lastBlockTriggered80028504 = false;
    bool lastDescCommandExecuted80028054 = false;
    bool lastDescHeaderCleared80028054 = false;
    bool lastDescAttrMerged80028054 = false;
    bool lastCoordWritten80028054 = false;
    bool lastType1CoordBackingMissing80028054 = false;
    bool lastType1CoordWritten80028054 = false;
    bool lastType4CoordWritten80028054 = false;
};

using PsxTmdModelProvider428B0 = const TmdModel* (*)(
    uint32_t descAddr,
    const PsxDrawModelRef& modelRef,
    void* userData);

using PsxMimeResourceProvider801CB190 = bool (*)(
    uint8_t datHandle,
    uint8_t vdfHandle,
    const DatData*& outDat,
    const VdfData*& outVdf,
    void* userData);

using PsxDrawTriangleCommandConsumer428B0 = void (*)(
    const PsxDrawTriangleCommand428B0& command,
    void* userData);

using PsxGpuPacketCommandConsumer428B0 = void (*)(
    const PsxGpuPacketCommand428B0& command,
    void* userData);

using PsxBackdropCommandConsumer801CAC34 = void (*)(
    const PsxBackdropGradientCommand801CAC34& gradient,
    const PsxBackdropSpriteCommand801CAC34* sprites,
    std::size_t spriteCount,
    void* userData);

using PsxTimUploadCommandConsumer801CBFDC = void (*)(
    const PsxTimUploadCommand801CBFDC& command,
    void* userData);

using PsxScriptBoxCommandConsumer80023E10 = void (*)(
    const PsxScriptBoxCommand80023E10& command,
    void* userData);

using PsxFastSpritePacketCommandConsumer8003FA20 = void (*)(
    const PsxFastSpritePacketCommand8003FA20& command,
    void* userData);

using PsxGsSpritePacketCommandConsumer8003F1B4 = void (*)(
    const PsxGsSpritePacketCommand8003F1B4& command,
    void* userData);

using PsxBoxFillPacketCommandConsumer8003EE84 = void (*)(
    const PsxBoxFillPacketCommand8003EE84& command,
    void* userData);

using PsxCompactRailSpriteCommandConsumer80024744 = void (*)(
    const PsxCompactRailSpriteCommand80024744& command,
    void* userData);

void BuildStage1DrawTriangleCommands801CBFDC(
    const Stage1SceneSubmitRuntime& runtime,
    PsxTmdModelProvider428B0 modelProvider,
    void* modelProviderUserData,
    PsxMimeResourceProvider801CB190 mimeProvider,
    void* mimeProviderUserData,
    PsxDrawTriangleCommandConsumer428B0 consumeCommand,
    void* commandUserData);

void BuildStage1GpuPacketCommands428B0(
    const Stage1SceneSubmitRuntime& runtime,
    PsxTmdModelProvider428B0 modelProvider,
    void* modelProviderUserData,
    PsxMimeResourceProvider801CB190 mimeProvider,
    void* mimeProviderUserData,
    PsxGpuPacketCommandConsumer428B0 consumeCommand,
    void* commandUserData);

void BuildStage1BackdropCommands801CAC34(
    const Stage1SceneSubmitRuntime& runtime,
    PsxBackdropCommandConsumer801CAC34 consumeCommand,
    void* commandUserData);

bool BuildStage1DrawEnvClearColorCommand80040060(
    const Stage1SceneSubmitRuntime& runtime,
    PsxDrawEnvClearColorCommand80040060& outCommand);

bool BuildStage1PreventTransitionDrawPlan801CBFDC(
    const Stage1SceneSubmitRuntime& runtime,
    float vx,
    float vy,
    float vs,
    PrStage1Scene1Movie1Direct::Movie1DrawPlan& outPlan);

void BuildStage1TimUploadCommands801CBFDC(
    const Stage1SceneSubmitRuntime& runtime,
    PsxTimUploadCommandConsumer801CBFDC consumeCommand,
    void* commandUserData);

void BuildStage1ScriptBoxCommands80023E10(
    const Stage1SceneSubmitRuntime& runtime,
    PsxScriptBoxCommandConsumer80023E10 consumeCommand,
    void* commandUserData);

void BuildStage1FastSpritePacketCommands8003FA20(
    const Stage1SceneSubmitRuntime& runtime,
    PsxFastSpritePacketCommandConsumer8003FA20 consumeCommand,
    void* commandUserData);

void BuildStage1GsSpritePacketCommands8003F1B4(
    const Stage1SceneSubmitRuntime& runtime,
    PsxGsSpritePacketCommandConsumer8003F1B4 consumeCommand,
    void* commandUserData);

void BuildFastSpritePacketCommands8003FA20(
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    PsxFastSpritePacketCommandConsumer8003FA20 consumeCommand,
    void* commandUserData);

void BuildStage1BoxFillPacketCommands8003EE84(
    const Stage1SceneSubmitRuntime& runtime,
    PsxBoxFillPacketCommandConsumer8003EE84 consumeCommand,
    void* commandUserData);

void BuildStage1CompactRailSpriteCommands80024744(
    const Stage1SceneSubmitRuntime& runtime,
    PsxCompactRailSpriteCommandConsumer80024744 consumeCommand,
    void* commandUserData);

} // namespace PrStageSceneSubmitDirect
