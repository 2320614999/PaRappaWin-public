#pragma once

#include "pr_psx_gte_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrPsxGsSpriteSubmitDirect {

constexpr uint32_t kGsSortSpriteFunction8003F1B4 = 0x8003F1B4u;
constexpr uint32_t kGsSortSpritePacketAddrMask8003F1B4 = 0x00FFFFFFu;
constexpr uint32_t kGsSortSpritePacketFastPayloadWords8003F1B4 = 5u;
constexpr uint32_t kGsSortSpritePacketTransformPayloadWords8003F1B4 = 9u;
constexpr uint32_t kGsSortSpritePacketFastTotalWords8003F1B4 =
    kGsSortSpritePacketFastPayloadWords8003F1B4 + 1u;
constexpr uint32_t kGsSortSpritePacketTransformTotalWords8003F1B4 =
    kGsSortSpritePacketTransformPayloadWords8003F1B4 + 1u;
constexpr uint32_t kGsSortSpritePacketMaxTotalWords8003F1B4 =
    kGsSortSpritePacketTransformTotalWords8003F1B4;
constexpr uint32_t kGsSortSpriteFastIdentityScale8003F1B4 = 0x10001000u;
constexpr std::size_t kGsSortSpriteRuntimeOtSlotCapacity8003F1B4 = 256u;
constexpr std::size_t kGsSortSpriteRuntimePacketWriteCapacity8003F1B4 =
    512u;

struct GsSpriteLocalFields8003F1B4 {
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
    int32_t rot_20 = 0;
};

struct OrderingTableState8003F1B4 {
    uint32_t headAddr_04 = 0;
    uint32_t length_08 = 0;
    uint32_t slotOldValue = 0;
};

struct DrawOffsetGlobals8003F1B4 {
    int16_t word_800917AA = 0;
    int16_t word_800917AC = 0;
};

using GteControlState8003F1B4 = PrPsxGteDirect::GteControlState;

struct GsSortSpriteInput8003F1B4 {
    GsSpriteLocalFields8003F1B4 sprite{};
    OrderingTableState8003F1B4 ot{};
    DrawOffsetGlobals8003F1B4 drawOffsets{};
    GteControlState8003F1B4 gte{};
    uint32_t packetAllocatorAddr_dword_800901C8 = 0;
    uint16_t priority = 0;
    uint16_t rotMatrixArg = 0;
};

enum class GsSortSpriteSkipReason8003F1B4 : uint8_t {
    None,
    NegativeAttr,
    ZeroWidth,
    ZeroHeight,
};

enum class GsSortSpritePacketPath8003F1B4 : uint8_t {
    None,
    Fast,
    Transform,
};

enum class GsSortSpriteReturnValueSource8003F1B4 : uint8_t {
    VoidOrUnknown,
};

struct GsSortSpriteReturnValue8003F1B4 {
    bool known = false;
    uint32_t value = 0;
    GsSortSpriteReturnValueSource8003F1B4 source =
        GsSortSpriteReturnValueSource8003F1B4::VoidOrUnknown;
};

struct GsSortSpriteOtSlot8003F1B4 {
    const char* expression = nullptr;
    uint32_t addr = 0;
    uint32_t oldValue = 0;
    uint32_t newValue = 0;
    bool zResolutionOverflow8003EF5C = false;
};

using Matrix8003F1B4 = PrPsxGteDirect::Matrix3x4;

struct ScaleVector8003F1B4 {
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
};

using Vertex8003F710 = PrPsxGteDirect::VertexS16;
using GteSxy8003F710 = PrPsxGteDirect::GteSxy;
using GteSzFifo8003F710 = PrPsxGteDirect::GteSzFifo;
using GteDivisionTrace8003F710 = PrPsxGteDirect::GteDivisionTrace;
using PsxGteRtInput8003F710 = PrPsxGteDirect::RtptRtps4Input8003F710;
using PsxGteRtOutput8003F710 = PrPsxGteDirect::RtptRtps4Output8003F710;

struct RotTransPers4Call8003F710 {
    bool evaluated = false;
    std::array<Vertex8003F710, 4> vertices{};
    bool matrixKnown = false;
    Matrix8003F1B4 matrix{};
    GteControlState8003F1B4 gte{};
    PsxGteRtInput8003F710 rtInput{};
    PsxGteRtOutput8003F710 rtOutput{};
    bool loadedRtptDataRegs0005 = false;
    bool rtpt280030Called = false;
    bool storedRtptSxy012 = false;
    bool flagAfterRtptKnown = false;
    uint32_t flagAfterRtpt = 0;
    bool loadedRtpsDataRegs0001 = false;
    bool rtps180001Called = false;
    bool storedRtpsSxy2AndIr0 = false;
    bool flagAfterRtpsKnown = false;
    uint32_t flagAfterRtps = 0;
    std::array<GteSxy8003F710, 4> sxy{};
    GteSzFifo8003F710 szFifoAfterRtpt{};
    GteSzFifo8003F710 szFifoAfterRtps{};
    bool sz3AfterRtpsKnown = false;
    uint32_t sz3AfterRtps = 0;
    bool sxyKnown = false;
    bool flagKnown = false;
    uint32_t flag = 0;
    bool ir0DepthKnown = false;
    uint32_t ir0Depth = 0;
    bool returnValueKnown = false;
    uint32_t returnValue = 0;
};

struct TransformPrelude8003F1B4 {
    bool evaluated = false;
    bool usedRotMatrix8003F790 = false;
    bool copiedIdentityMatrixGlobals80091838 = false;
    bool scaleMatrixApplied8003B0FC = false;
    bool readGeomScreen8003F700 = false;
    bool geomScreenKnown = false;
    bool geomScreenGap8003F700 = false;
    uint32_t geomScreen = 0;
    bool geomOffsetKnown = false;
    bool geomOffsetGap800402C8 = false;
    int32_t geomOffsetX = 0;
    int32_t geomOffsetY = 0;
    bool depthCueKnown = false;
    bool zScaleFactorKnown = false;
    bool transMatrixApplied8003B0CC = false;
    bool setRotMatrix8003F6B0 = false;
    bool setTransMatrix8003F6E0 = false;
    bool rotTransPers4Gap8003F710 = false;
    RotTransPers4Call8003F710 rotTransPers4{};
    bool matrixValuesKnown = false;
    bool defaultMatrixGlobalsGap80091838 = false;
    Matrix8003F1B4 matrix{};
    ScaleVector8003F1B4 scale{};
};

struct GsSortSpritePacket8003F1B4 {
    bool written = false;
    bool fieldsKnown = false;
    bool transformFieldsGap = false;
    bool transformNonGeometryFieldsKnown = false;
    TransformPrelude8003F1B4 transformPrelude{};
    uint32_t addr = 0;
    uint32_t payloadWordCount = 0;
    uint32_t totalWordCount = 0;
    uint32_t word0_linkTag = 0;
    uint32_t word1_drawModeOrColor = 0;
    uint32_t word2_colorOrXy0 = 0;
    uint32_t word3_xyOrUv0 = 0;
    uint32_t word4_uvOrXy1 = 0;
    uint32_t word5_whOrUv1 = 0;
    uint32_t word6_xy2 = 0;
    uint32_t word7_uv2 = 0;
    uint32_t word8_xy3 = 0;
    uint32_t word9_uv3 = 0;

    std::array<uint32_t, kGsSortSpritePacketFastTotalWords8003F1B4>
    FastWords() const {
        return {{
            word0_linkTag,
            word1_drawModeOrColor,
            word2_colorOrXy0,
            word3_xyOrUv0,
            word4_uvOrXy1,
            word5_whOrUv1,
        }};
    }

    std::array<uint32_t, kGsSortSpritePacketTransformTotalWords8003F1B4>
    TransformWords() const {
        return {{
            word0_linkTag,
            word1_drawModeOrColor,
            word2_colorOrXy0,
            word3_xyOrUv0,
            word4_uvOrXy1,
            word5_whOrUv1,
            word6_xy2,
            word7_uv2,
            word8_xy3,
            word9_uv3,
        }};
    }
};

struct GsSortSpriteAllocator8003F1B4 {
    uint32_t oldAddr = 0;
    uint32_t advanceWords = 0;
    uint32_t advanceBytes = 0;
    uint32_t newAddr = 0;
};

struct GsSortSpriteResult8003F1B4 {
    bool skipped = false;
    GsSortSpriteSkipReason8003F1B4 skipReason =
        GsSortSpriteSkipReason8003F1B4::None;
    GsSortSpritePacketPath8003F1B4 packetPath =
        GsSortSpritePacketPath8003F1B4::None;
    bool priorityKnown = false;
    uint16_t priority = 0;
    bool branchDependsOnScaleRotAttr = true;
    GsSortSpritePacket8003F1B4 packet{};
    GsSortSpriteOtSlot8003F1B4 otSlot{};
    GsSortSpriteAllocator8003F1B4 allocator{};
    GsSortSpriteReturnValue8003F1B4 returnValue{};
};

struct RuntimeOtSlotValue8003F1B4 {
    bool valid = false;
    uint32_t addr = 0;
    uint32_t value = 0;
};

struct RuntimePacketWrite8003F1B4 {
    bool valid = false;
    uint32_t addr = 0;
    uint32_t wordCount = 0;
    GsSortSpritePacketPath8003F1B4 packetPath =
        GsSortSpritePacketPath8003F1B4::None;
    bool priorityKnown = false;
    uint16_t priority = 0;
    bool otSlotKnown = false;
    GsSortSpriteOtSlot8003F1B4 otSlot{};
    bool allocatorKnown = false;
    GsSortSpriteAllocator8003F1B4 allocator{};
    bool submitOrderKnown = false;
    uint32_t submitOrder = 0;
    std::array<uint32_t, kGsSortSpritePacketMaxTotalWords8003F1B4> words{};
    std::array<bool, kGsSortSpritePacketMaxTotalWords8003F1B4> wordKnown{};
};

struct RuntimeOrderingTableState8003F1B4 {
    uint32_t headAddr_04 = 0;
    uint32_t length_08 = 0;
    std::array<RuntimeOtSlotValue8003F1B4,
               kGsSortSpriteRuntimeOtSlotCapacity8003F1B4>
        slotValues{};
};

struct RuntimeState8003F1B4 {
    uint32_t dword_800901C8 = 0;
    int16_t word_800917AA = 0;
    int16_t word_800917AC = 0;
    GteControlState8003F1B4 gte{};
    RuntimeOrderingTableState8003F1B4 ot{};
    std::array<RuntimePacketWrite8003F1B4,
               kGsSortSpriteRuntimePacketWriteCapacity8003F1B4>
        packetWrites{};
};

struct RuntimeUpdate8003F1B4 {
    bool dryRun = false;
    bool skipped = false;
    GsSortSpriteSkipReason8003F1B4 skipReason =
        GsSortSpriteSkipReason8003F1B4::None;
    GsSortSpritePacketPath8003F1B4 packetPath =
        GsSortSpritePacketPath8003F1B4::None;
    bool allocatorWouldUpdate = false;
    bool allocatorUpdated = false;
    bool allocatorOldMatchesRuntime = false;
    uint32_t oldAllocatorAddr = 0;
    uint32_t newAllocatorAddr = 0;
    bool packetWouldWrite = false;
    uint32_t packetAddr = 0;
    uint32_t packetPayloadWordCount = 0;
    uint32_t packetTotalWordCount = 0;
    bool packetWriteMirrored = false;
    bool packetWriteCapacityExceeded = false;
    bool otSlotWouldUpdate = false;
    bool otSlotUpdated = false;
    bool otSlotOldMatchesRuntime = false;
    bool otSlotCapacityExceeded = false;
    GsSortSpriteOtSlot8003F1B4 otSlot{};
    GsSortSpriteReturnValue8003F1B4 returnValue{};
};

GsSortSpriteResult8003F1B4 PsxCall8003F1B4_GsSortSprite(
    const GsSortSpriteInput8003F1B4& input);

GsSortSpritePacketPath8003F1B4 ResolvePacketPath8003F1B4(
    const GsSpriteLocalFields8003F1B4& sprite);

TransformPrelude8003F1B4 BuildTransformPrelude8003F1B4(
    const GsSpriteLocalFields8003F1B4& sprite,
    const GteControlState8003F1B4& gte);

uint32_t ResolveRuntimeOtSlotAddr8003F1B4(
    const RuntimeOrderingTableState8003F1B4& ot,
    uint16_t priority);

bool TryGetRuntimeOtSlotValue8003F1B4(
    const RuntimeOrderingTableState8003F1B4& ot,
    uint32_t addr,
    uint32_t* outValue);

uint32_t GetRuntimeOtSlotOldValue8003F1B4(
    const RuntimeOrderingTableState8003F1B4& ot,
    uint32_t addr);

bool SetRuntimeOtSlotValue8003F1B4(
    RuntimeOrderingTableState8003F1B4& ot,
    uint32_t addr,
    uint32_t value);

bool SetRuntimeOtSlotValueForPriority8003F1B4(
    RuntimeOrderingTableState8003F1B4& ot,
    uint16_t priority,
    uint32_t value);

OrderingTableState8003F1B4 BuildOrderingTableStateFromRuntime8003F1B4(
    const RuntimeOrderingTableState8003F1B4& ot,
    uint16_t priority);

GsSortSpriteInput8003F1B4 BuildInputFromRuntime8003F1B4(
    const RuntimeState8003F1B4& runtime,
    const GsSpriteLocalFields8003F1B4& sprite,
    uint16_t priority,
    uint16_t rotMatrixArg);

RuntimeUpdate8003F1B4 PredictRuntimeUpdate8003F1B4(
    const RuntimeState8003F1B4& runtime,
    const GsSortSpriteResult8003F1B4& result);

RuntimeUpdate8003F1B4 ApplyRuntimeUpdate8003F1B4(
    RuntimeState8003F1B4& runtime,
    const GsSortSpriteResult8003F1B4& result);

} // namespace PrPsxGsSpriteSubmitDirect
