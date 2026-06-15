#pragma once

#include "pr_psx_fast_sprite_submit_direct.h"
#include "pr_psx_gs_sprite_submit_direct.h"
#include "pr_stage_scene_submit_draw_private.h"

#include <array>
#include <cstdint>
#include <vector>

namespace PrStage1CompactRail80024744Direct {

struct PsxCompactWobbleSlot80024744 {
    int32_t counter = 0;
    int32_t phase = 0;
    int32_t linearAcc = 2048;
    int32_t linearVel = 2048;
};

struct PsxCompactRailRuntime80024744 {
    std::array<int16_t, 36> sharedScaleX{};
    std::array<int16_t, 36> sharedScaleY{};
    std::array<PsxCompactWobbleSlot80024744, 36> bankA{};
    std::array<PsxCompactWobbleSlot80024744, 36> bankB{};
    int32_t teacherHoldCount304 = 0;
    int32_t teacherHoldRaw308 = -1;
    int32_t studentHoldCount312 = 0;
    int32_t studentHoldRaw316 = -1;
    bool initialized = false;
};

struct PsxCompactRailInput80024744 {
    bool inputCaptured = false;
    PrStageSceneSubmitDirect::PsxCompactRailCommand80024744 command{};
    bool submitRuntime8003FA20Valid = false;
    bool submitRuntime8003F1B4Valid = false;
    uint8_t submitRuntimeWorkIndex8004019C = 0;
    struct PsxCompactRailLocalSpriteRgbCarrier80024744 {
        enum class Source : uint8_t {
            Unknown = 0,
            AuthoritativeCallerLocal = 1,
            FixedConstant = 2,
            Template = 3,
            CallArgument = 4,
            GlobalTable = 5,
        };
        enum class Gap : uint8_t {
            None = 0,
            UnresolvedStackLocalTail = 1,
            NoAuthoritativeCallerField = 2,
            StaticChainHasNoRgbWrite = 3,
        };
        enum class Evidence : uint8_t {
            None = 0,
            CompactRail80024744StaticChainNoRgbWrite = 1,
        };

        bool known = false;
        Source source = Source::Unknown;
        Gap gap = Gap::UnresolvedStackLocalTail;
        Evidence evidence = Evidence::None;
        uint32_t callerFunction80024744 = 0;
        uint32_t wrapperFunction = 0;
        uint32_t localBuilderFunction = 0;
        uint32_t submitFunction = 0;
        bool fixedConstantWriteObserved = false;
        bool templateWriteObserved = false;
        bool callArgumentWriteObserved = false;
        bool globalTableWriteObserved = false;
        bool stackLocalWriteObserved = false;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
    };
    PsxCompactRailLocalSpriteRgbCarrier80024744 localFastSpriteRgb8001C550{};
    PsxCompactRailLocalSpriteRgbCarrier80024744 localGsSpriteRgb8001C804{};
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20
        submitRuntime8003FA20{};
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4
        submitRuntime8003F1B4{};
};

struct PsxCompactRailWorkOtMetadata80024744 {
    bool valid = false;
    uint32_t workBase80087288 = 0x80087288u;
    uint32_t workStrideBytes = 20u;
    uint32_t workIndexGpOffset = 872u;
    bool workIndexKnown = false;
    uint32_t workIndex = 0;
    bool workPtrKnown = false;
    uint32_t workPtr = 0;
    uint32_t otHeadOffset = 4u;
    uint32_t otLengthOffset = 8u;
    uint32_t packetAllocatorGlobal800901C8 = 0x800901C8u;
    bool packetAllocatorKnown = false;
    uint32_t packetAllocatorAddr = 0;
    uint32_t drawOffsetXGlobal800917AA = 0x800917AAu;
    uint32_t drawOffsetYGlobal800917AC = 0x800917ACu;
    bool drawOffsetsKnown = false;
    int16_t drawOffsetX = 0;
    int16_t drawOffsetY = 0;
    bool otSlotKnown = false;
    uint32_t otSlotAddr = 0;
};

using PsxCompactRailLocalSpriteRgbCarrier80024744 =
    PsxCompactRailInput80024744::PsxCompactRailLocalSpriteRgbCarrier80024744;

struct PsxCompactRailSubmit8001C550Debug80024744 {
    bool valid = false;
    uint32_t sourceFunction8001C550 = 0x8001C550u;
    uint32_t callerFunction8001B590 = 0x8001B590u;
    uint32_t templateExpandFunction8001B25C = 0x8001B25Cu;
    uint32_t submitFunction8003FA20 = 0x8003FA20u;
    uint16_t priority = 0;
    uint32_t templatePtr = 0;
    int16_t argScreenX = 0;
    int16_t argScreenY = 0;
    int16_t localX = 0;
    int16_t localY = 0;
    uint16_t glyphIndex = 0;
    bool addGlyphIndexToClutY = false;
    bool localFastSpritePrefixKnown = false;
    bool localFastSpriteKnown = false;
    bool localFastSpriteRgbKnown = false;
    PsxCompactRailLocalSpriteRgbCarrier80024744 localFastSpriteRgb{};
    PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
        localFastSprite{};
    PsxCompactRailWorkOtMetadata80024744 workOt{};
    PrPsxFastSpriteSubmitDirect::GsSortFastSpriteResult8003FA20
        submitMetadata8003FA20{};
    PrPsxFastSpriteSubmitDirect::RuntimeUpdate8003FA20
        submitRuntimeUpdate8003FA20{};
    uint32_t emittedPacketWords8003FA20 = 6u;
    uint32_t packetLinkTagAddend8003FA20 = 0x05000000u;
    uint32_t packetAddrMask8003FA20 = 0x00FFFFFFu;
    bool rejectsNegativeAttr = true;
    bool rejectsZeroWidth = true;
    bool rejectsZeroHeight = true;
    bool writesPacketToOt = true;
    bool advancesPacketAllocator = true;
    bool submitPacketGap = true;
};

struct PsxCompactRailPortraitDebug80024744 {
    bool visited = false;
    bool drawSubmitted = false;
    int16_t raw = -1;
    int32_t holdRaw = -1;
    int32_t holdCount = 0;
    float x = 0.0f;
    float y = 0.0f;
    PsxCompactRailSubmit8001C550Debug80024744 submit8001C550{};
};

struct PsxCompactRailGsSpriteLocalFields80024744 {
    uint32_t attr_00 = 0;
    int16_t x_04 = 0;
    int16_t y_06 = 0;
    uint16_t width_08 = 0;
    uint16_t height_0A = 0;
    uint16_t tpage_0C = 0;
    uint8_t u_0E = 0;
    uint8_t v_0F = 0;
    int16_t clutX_10 = 0;
    int16_t clutY_12 = 0;
    uint8_t r_14 = 0;
    uint8_t g_15 = 0;
    uint8_t b_16 = 0;
    int16_t mx_18 = 0;
    int16_t my_1A = 0;
    int16_t scaleX_1C = 4096;
    int16_t scaleY_1E = 4096;
};

struct PsxCompactRailSubmit8001C804Debug80024744 {
    bool valid = false;
    uint32_t sourceFunction8001C804 = 0x8001C804u;
    uint32_t templateExpandFunction8001B338 = 0x8001B338u;
    uint32_t submitFunction8003F1B4 = 0x8003F1B4u;
    uint16_t priority = 1;
    uint32_t templatePtr = 0;
    int16_t argCenterX = 0;
    int16_t argCenterY = 0;
    int16_t localX = 0;
    int16_t localY = 0;
    bool localGsSpritePrefixKnown = false;
    bool localGsSpriteKnown = false;
    bool localGsSpriteRgbKnown = false;
    PsxCompactRailLocalSpriteRgbCarrier80024744 localGsSpriteRgb{};
    PsxCompactRailGsSpriteLocalFields80024744 localGsSprite{};
    PrPsxGsSpriteSubmitDirect::GsSpriteLocalFields8003F1B4
        localBackendGsSprite{};
    PsxCompactRailWorkOtMetadata80024744 workOt{};
    PrPsxGsSpriteSubmitDirect::GsSortSpriteResult8003F1B4
        submitMetadata8003F1B4{};
    PrPsxGsSpriteSubmitDirect::RuntimeUpdate8003F1B4
        submitRuntimeUpdate8003F1B4{};
    bool submitPacketGap = true;
};

struct PsxCompactRailBodyNoteDebug80024744 {
    bool visited = false;
    bool drawSubmitted = false;
    int8_t type = 0;
    uint8_t typeIndex80024418 = 0;
    uint8_t localSlot = 0;
    uint8_t slotOrdinal = 0;
    uint8_t scaleIndex = 0;
    uint32_t templateTableBase800540BC = 0;
    uint32_t localSpriteAttr80024418 = 0;
    uint32_t templatePtr800540BC = 0;
    int16_t scaleXWord80087668 = 4096;
    int16_t scaleYWord800876B0 = 4096;
    int16_t argX80024418 = 0;
    int16_t argY80024418 = 0;
    int16_t argSlot80024418 = 0;
    int16_t argType80024418 = 0;
    float centerX = 0.0f;
    float centerY = 0.0f;
    int16_t localSpriteX80024418 = 0;
    int16_t localSpriteY80024418 = 0;
    PsxCompactRailSubmit8001C804Debug80024744 submit8001C804{};
};

struct PsxCompactRailMarkerDebug80024744 {
    bool visited = false;
    bool drawSubmitted = false;
    bool lit = false;
    bool skippedByRowGate = false;
    uint8_t ordinal = 0;
    uint8_t thresholdOrdinal = 0;
    uint8_t otBucket8001C550 = 3;
    int16_t x = 0;
    int16_t y = 0;
    int16_t threshold = 0;
    int16_t raw = -1;
    int32_t teacherProgressX = 0;
    uint32_t thresholdTablePtr = 0;
    uint32_t litTemplatePtr = 0;
    uint32_t unlitTemplatePtr = 0;
    uint32_t selectedTemplatePtr = 0;
    PsxCompactRailSubmit8001C550Debug80024744 submit8001C550{};
};

struct PsxCompactRailRowDebug80024744 {
    bool visited = false;
    bool drawEnabled = false;
    bool painterGateMatched7A = false;
    uint8_t row = 0;
    uint8_t bodyStreamSlotCount = 18;
    uint8_t smallMarkerStartIndex = 0;
    int32_t rowOffset18 = 0;
    int32_t rowSlotBase17 = 0;
    int32_t rowYBase20 = 0;
    int32_t teacherRawSourceOffset = 0x8C;
    int32_t studentRawSourceOffset = 0x9E;
    int32_t bodyStreamPtrSourceOffset = 0x94;
    int16_t teacherRaw8C = -1;
    int16_t studentRaw9E = -1;
    uint32_t bodyStreamPtr94 = 0;
    int32_t teacherProgressX80024744 = 0;
    uint32_t largeThresholdTablePtr800540E8 = 0;
    uint32_t smallThresholdTablePtr80054100 = 0;
    uint8_t bodyNoteCount = 0;
    uint8_t largeMarkerLitCount = 0;
    uint8_t smallMarkerLitCount = 0;
    int32_t teacherWobbleCount80023F20 = 0;
    int32_t studentWobbleCount80024114 = 0;
    PsxCompactRailPortraitDebug80024744 teacherPortrait{};
    PsxCompactRailPortraitDebug80024744 studentPortrait{};
    std::array<PsxCompactRailMarkerDebug80024744, 4> largeMarkers{};
    std::array<PsxCompactRailMarkerDebug80024744, 14> smallMarkers{};
    std::array<PsxCompactRailBodyNoteDebug80024744,
               PrStageSceneSubmitDirect::kCompactRailBodySlotCount80024744>
        bodyNotes{};
};

struct PsxCompactRailFrameDebug80024744 {
    bool valid = false;
    uint32_t callSerial = 0;
    bool inputCaptured = false;
    bool resetWobbleBanks = false;
    bool resetTeacherWobbleBank80024308 = false;
    bool resetStudentWobbleBank80024390 = false;
    int32_t painterGateSourceOffset = 0x7A;
    int32_t rowCountSourceOffset = 0x8A;
    int16_t painterGate7A = 0;
    int16_t rowCount8A = 0;
    int16_t clampedRowCount = 0;
    uint8_t maxRows80024744 = 2;
    uint8_t bodyStreamSlotCount80024744 = 18;
    uint16_t spriteCommandCount = 0;
    uint16_t noteCommandCount = 0;
    uint16_t markerCommandCount = 0;
    uint16_t portraitCommandCount = 0;
    uint8_t wobbleBankEntryCount80024308_80024390 = 36;
    uint8_t teacherLinearEnd80023F20 = 6;
    uint8_t studentLinearEnd80024114 = 5;
    uint8_t wobbleSinEnd80023F20_80024114 = 22;
    uint8_t wobbleStepCount80023F20_80024114 = 24;
    int32_t teacherHoldRaw308 = -1;
    int32_t teacherHoldCount304 = 0;
    int32_t studentHoldRaw316 = -1;
    int32_t studentHoldCount312 = 0;
    std::array<PsxCompactRailRowDebug80024744,
               PrStageSceneSubmitDirect::kCompactRailMaxRows80024744>
        rows{};
};

void PsxCall80024744(
    const PsxCompactRailInput80024744& input,
    PsxCompactRailRuntime80024744& runtime,
    std::vector<PrStageSceneSubmitDirect::PsxCompactRailCommand80024744>&
        commandLog,
    std::vector<PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>&
        spriteCommands,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20*
        outSubmitRuntime8003FA20 = nullptr,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4*
        outSubmitRuntime8003F1B4 = nullptr);

const PsxCompactRailFrameDebug80024744& GetLastFrameDebug80024744();

} // namespace PrStage1CompactRail80024744Direct
