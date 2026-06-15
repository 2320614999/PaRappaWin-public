#include "pr_psx_gs_sprite_submit_direct.h"

namespace PrPsxGsSpriteSubmitDirect {
namespace {

#include "pr_psx_rotmatrix_trig_table_800581c0.inc"

constexpr const char* kOtSlotExpression8003F1B4 =
    "4 * priority + ot.headAddr_04 - 4 * ot.length_08";

uint16_t WrapAdd16(int32_t lhs, int32_t rhs) {
    return static_cast<uint16_t>(
        static_cast<uint32_t>(lhs) + static_cast<uint32_t>(rhs));
}

uint32_t MakePacketLinkTag8003F1B4(uint32_t oldValue,
                                   uint32_t payloadWordCount) {
    return (oldValue & kGsSortSpritePacketAddrMask8003F1B4) |
           ((payloadWordCount & 0xFFu) << 24);
}

uint32_t MakeDrawModeWord8003F1B4(uint32_t attr, uint16_t tpage) {
    return (static_cast<uint32_t>(tpage) & 0x1Fu) |
           ((attr >> 17) & 0x180u) |
           0xE1000200u |
           ((attr >> 23) & 0x60u);
}

uint32_t MakeFastColorCodeWord8003F1B4(uint32_t attr,
                                       uint8_t r,
                                       uint8_t g,
                                       uint8_t b) {
    return ((attr >> 5) & 0x02000000u) |
           ((attr << 18) & 0x01000000u) |
           0x64000000u |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8) |
           static_cast<uint32_t>(r);
}

uint32_t MakeFastXyWord8003F1B4(const GsSpriteLocalFields8003F1B4& sprite,
                                const DrawOffsetGlobals8003F1B4& offsets) {
    const uint16_t x =
        WrapAdd16(WrapAdd16(sprite.x_04, offsets.word_800917AA),
                  -sprite.mx_18);
    const uint16_t y =
        WrapAdd16(WrapAdd16(sprite.y_06, offsets.word_800917AC),
                  -sprite.my_1A);
    return static_cast<uint32_t>(x) | (static_cast<uint32_t>(y) << 16);
}

uint32_t MakeUvClutWord8003F1B4(const GsSpriteLocalFields8003F1B4& sprite) {
    return static_cast<uint32_t>(sprite.u_0E) |
           (static_cast<uint32_t>(sprite.v_0F) << 8) |
           (static_cast<uint32_t>(static_cast<uint16_t>(sprite.clutY_12))
            << 22) |
           ((static_cast<uint32_t>(static_cast<uint16_t>(sprite.clutX_10))
             << 12) &
            0x003F0000u);
}

uint32_t MakeWhWord8003F1B4(const GsSpriteLocalFields8003F1B4& sprite) {
    return static_cast<uint32_t>(sprite.width_08) |
           (static_cast<uint32_t>(sprite.height_0A) << 16);
}

uint32_t MakeTransformColorCodeWord8003F1B4(uint32_t attr,
                                            uint8_t r,
                                            uint8_t g,
                                            uint8_t b) {
    return ((attr >> 5) & 0x02000000u) |
           ((attr << 18) & 0x01000000u) |
           0x2C000000u |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8) |
           static_cast<uint32_t>(r);
}

uint8_t TransformUvLeft8003F1B4(const GsSpriteLocalFields8003F1B4& sprite) {
    if ((sprite.attr_00 & 0x00800000u) != 0u) {
        return static_cast<uint8_t>(sprite.u_0E + sprite.width_08 - 1u);
    }
    return sprite.u_0E;
}

uint8_t TransformUvRight8003F1B4(const GsSpriteLocalFields8003F1B4& sprite) {
    if ((sprite.attr_00 & 0x00800000u) != 0u) {
        return sprite.u_0E;
    }
    return static_cast<uint8_t>(sprite.u_0E + sprite.width_08 - 1u);
}

uint8_t TransformUvTop8003F1B4(const GsSpriteLocalFields8003F1B4& sprite) {
    if ((sprite.attr_00 & 0x00400000u) != 0u) {
        return static_cast<uint8_t>(sprite.v_0F + sprite.height_0A - 1u);
    }
    return sprite.v_0F;
}

uint8_t TransformUvBottom8003F1B4(const GsSpriteLocalFields8003F1B4& sprite) {
    if ((sprite.attr_00 & 0x00400000u) != 0u) {
        return sprite.v_0F;
    }
    return static_cast<uint8_t>(sprite.v_0F + sprite.height_0A - 1u);
}

uint32_t MakeTransformUvClutWord8003F1B4(uint8_t u,
                                         uint8_t v,
                                         const GsSpriteLocalFields8003F1B4& sprite) {
    return static_cast<uint32_t>(u) |
           (static_cast<uint32_t>(v) << 8) |
           (static_cast<uint32_t>(static_cast<uint16_t>(sprite.clutY_12))
            << 22) |
           ((static_cast<uint32_t>(static_cast<uint16_t>(sprite.clutX_10))
             << 12) &
            0x003F0000u);
}

uint32_t MakeTransformUvTpageWord8003F1B4(uint8_t u,
                                          uint8_t v,
                                          const GsSpriteLocalFields8003F1B4& sprite) {
    return static_cast<uint32_t>(u) |
           (static_cast<uint32_t>(v) << 8) |
           ((static_cast<uint32_t>(sprite.tpage_0C) & 0x1Fu) << 16) |
           ((sprite.attr_00 >> 1) & 0x01800000u) |
           ((sprite.attr_00 >> 7) & 0x00600000u);
}

uint32_t ResolveOtSlotAddr8003F1B4(const GsSortSpriteInput8003F1B4& input) {
    return input.ot.headAddr_04 +
           static_cast<uint32_t>(input.priority) * 4u -
           input.ot.length_08 * 4u;
}

GsSortSpriteOtSlot8003F1B4 PsxCall8003EF5C_LinkPacket(
    uint32_t packetAddr,
    const OrderingTableState8003F1B4& ot,
    uint16_t priority,
    uint8_t payloadWordCount) {
    GsSortSpriteOtSlot8003F1B4 out{};
    out.expression = kOtSlotExpression8003F1B4;
    out.addr = ot.headAddr_04 + 4u * static_cast<uint32_t>(priority) -
               4u * ot.length_08;
    out.oldValue = ot.slotOldValue;
    out.newValue = packetAddr & kGsSortSpritePacketAddrMask8003F1B4;
    out.zResolutionOverflow8003EF5C =
        static_cast<int32_t>(priority) - static_cast<int32_t>(ot.length_08) < 0;
    (void)payloadWordCount;
    return out;
}

int32_t MulShift12Signed8003B0FC(int32_t lhs, int32_t rhs) {
    return static_cast<int32_t>(
        (static_cast<int64_t>(lhs) * static_cast<int64_t>(rhs)) >> 12);
}

int32_t SignExtendLow16(uint32_t value) {
    return static_cast<int16_t>(value & 0xFFFFu);
}

uint32_t PackSignedLowHigh16(int32_t low, int32_t high) {
    return static_cast<uint32_t>(static_cast<uint16_t>(low)) |
           (static_cast<uint32_t>(static_cast<uint16_t>(high)) << 16);
}

void SetMatrixRotationHalfword8003F790(Matrix8003F1B4& matrix,
                                       uint32_t halfwordIndex,
                                       int32_t value) {
    const uint32_t wordIndex = halfwordIndex / 2u;
    const uint32_t shift = (halfwordIndex & 1u) != 0u ? 16u : 0u;
    const uint32_t mask = 0xFFFFu << shift;
    matrix.words[wordIndex] =
        (matrix.words[wordIndex] & ~mask) |
        (static_cast<uint32_t>(static_cast<uint16_t>(value)) << shift);
}

int32_t TrigSinForSignedAngle8003F790(int32_t angle,
                                      uint32_t* packedOut) {
    const uint32_t index =
        static_cast<uint32_t>(angle < 0 ? -angle : angle) & 0xFFFu;
    const uint32_t packed = kRotMatrixTrigTable800581C0[index];
    if (packedOut != nullptr) {
        *packedOut = packed;
    }
    const int32_t sinValue = static_cast<int16_t>(packed & 0xFFFFu);
    return angle < 0 ? -sinValue : sinValue;
}

Matrix8003F1B4 PsxCall8003F790_RotMatrix(int16_t x,
                                          int16_t y,
                                          int16_t z) {
    Matrix8003F1B4 matrix{};

    uint32_t packedX = 0u;
    const int32_t sinX = TrigSinForSignedAngle8003F790(x, &packedX);
    const int32_t cosX = static_cast<int32_t>(packedX) >> 16;

    uint32_t packedY = 0u;
    const int32_t rawSinY = TrigSinForSignedAngle8003F790(y, &packedY);
    const int32_t cosY = static_cast<int32_t>(packedY) >> 16;
    const int32_t rot02 = rawSinY;
    const int32_t negSinY = -rawSinY;

    SetMatrixRotationHalfword8003F790(matrix, 2u, rot02);
    SetMatrixRotationHalfword8003F790(
        matrix, 5u, -MulShift12Signed8003B0FC(cosY, sinX));
    SetMatrixRotationHalfword8003F790(
        matrix, 8u, MulShift12Signed8003B0FC(cosY, cosX));

    uint32_t packedZ = 0u;
    const int32_t sinZ = TrigSinForSignedAngle8003F790(z, &packedZ);
    const int32_t cosZ = static_cast<int32_t>(packedZ) >> 16;
    const int32_t zCosY = MulShift12Signed8003B0FC(cosZ, cosY);
    const int32_t zSinY = MulShift12Signed8003B0FC(cosZ, negSinY);
    const int32_t sinZSinY = MulShift12Signed8003B0FC(sinZ, negSinY);

    SetMatrixRotationHalfword8003F790(matrix, 0u, zCosY);
    SetMatrixRotationHalfword8003F790(
        matrix, 1u, -MulShift12Signed8003B0FC(sinZ, cosY));
    SetMatrixRotationHalfword8003F790(
        matrix,
        3u,
        MulShift12Signed8003B0FC(sinZ, cosX) -
            MulShift12Signed8003B0FC(zSinY, sinX));
    SetMatrixRotationHalfword8003F790(
        matrix,
        6u,
        MulShift12Signed8003B0FC(sinZ, sinX) +
            MulShift12Signed8003B0FC(zSinY, cosX));
    SetMatrixRotationHalfword8003F790(
        matrix,
        4u,
        MulShift12Signed8003B0FC(cosZ, cosX) +
            MulShift12Signed8003B0FC(sinZSinY, sinX));
    SetMatrixRotationHalfword8003F790(
        matrix,
        7u,
        MulShift12Signed8003B0FC(cosZ, sinX) -
            MulShift12Signed8003B0FC(sinZSinY, cosX));
    return matrix;
}

Matrix8003F1B4 PsxCall8003B0FC_ScaleMatrix(Matrix8003F1B4 matrix,
                                           const ScaleVector8003F1B4& scale) {
    const uint32_t word0 = matrix.words[0];
    const uint32_t word1 = matrix.words[1];
    const uint32_t word2 = matrix.words[2];
    const uint32_t word3 = matrix.words[3];
    const uint32_t word4 = matrix.words[4];

    matrix.words[0] =
        PackSignedLowHigh16(MulShift12Signed8003B0FC(SignExtendLow16(word0),
                                                    scale.x),
                            MulShift12Signed8003B0FC(
                                static_cast<int16_t>(word0 >> 16),
                                scale.y));
    matrix.words[1] =
        PackSignedLowHigh16(MulShift12Signed8003B0FC(SignExtendLow16(word1),
                                                    scale.z),
                            MulShift12Signed8003B0FC(
                                static_cast<int16_t>(word1 >> 16),
                                scale.x));
    matrix.words[2] =
        PackSignedLowHigh16(MulShift12Signed8003B0FC(SignExtendLow16(word2),
                                                    scale.y),
                            MulShift12Signed8003B0FC(
                                static_cast<int16_t>(word2 >> 16),
                                scale.z));
    matrix.words[3] =
        PackSignedLowHigh16(MulShift12Signed8003B0FC(SignExtendLow16(word3),
                                                    scale.x),
                            MulShift12Signed8003B0FC(
                                static_cast<int16_t>(word3 >> 16),
                                scale.y));
    matrix.words[4] =
        static_cast<uint32_t>(
            MulShift12Signed8003B0FC(SignExtendLow16(word4), scale.z));
    return matrix;
}

Matrix8003F1B4 PsxCall8003B0CC_TransMatrix(
    Matrix8003F1B4 matrix,
    const ScaleVector8003F1B4& translation) {
    matrix.words[5] = static_cast<uint32_t>(translation.x);
    matrix.words[6] = static_cast<uint32_t>(translation.y);
    matrix.words[7] = static_cast<uint32_t>(translation.z);
    return matrix;
}

RotTransPers4Call8003F710 PsxCall8003F710_RotTransPers4Gap(
    const GsSpriteLocalFields8003F1B4& sprite,
    const Matrix8003F1B4& matrix,
    bool matrixKnown,
    const GteControlState8003F1B4& gte) {
    RotTransPers4Call8003F710 out{};
    out.evaluated = true;
    out.matrixKnown = matrixKnown;
    out.matrix = matrix;
    out.gte = gte;
    out.vertices[0] = Vertex8003F710{
        static_cast<int16_t>(-sprite.mx_18),
        static_cast<int16_t>(-sprite.my_1A),
        0,
        0};
    out.vertices[1] = Vertex8003F710{
        static_cast<int16_t>(static_cast<int32_t>(sprite.width_08) -
                             static_cast<int32_t>(sprite.mx_18)),
        static_cast<int16_t>(-sprite.my_1A),
        0,
        0};
    out.vertices[2] = Vertex8003F710{
        static_cast<int16_t>(-sprite.mx_18),
        static_cast<int16_t>(static_cast<int32_t>(sprite.height_0A) -
                             static_cast<int32_t>(sprite.my_1A)),
        0,
        0};
    out.vertices[3] = Vertex8003F710{
        static_cast<int16_t>(static_cast<int32_t>(sprite.width_08) -
                             static_cast<int32_t>(sprite.mx_18)),
        static_cast<int16_t>(static_cast<int32_t>(sprite.height_0A) -
                             static_cast<int32_t>(sprite.my_1A)),
        0,
        0};
    out.rtInput.matrixKnown = matrixKnown;
    out.rtInput.matrix = matrix;
    out.rtInput.controlKnown = gte.geomScreenKnown && gte.geomOffsetKnown;
    out.rtInput.control = gte;
    out.rtInput.verticesKnown = true;
    out.rtInput.vertices = out.vertices;
    out.rtOutput =
        PrPsxGteDirect::PsxCall8003F710_RotTransPers4RtptRtpsGap(
            out.rtInput);
    out.loadedRtptDataRegs0005 =
        out.rtOutput.schedule.loadedRtptDataRegs0005;
    out.rtpt280030Called = out.rtOutput.schedule.rtpt280030Called;
    out.storedRtptSxy012 = out.rtOutput.schedule.storedRtptSxy012;
    out.loadedRtpsDataRegs0001 =
        out.rtOutput.schedule.loadedRtpsDataRegs0001;
    out.rtps180001Called = out.rtOutput.schedule.rtps180001Called;
    out.storedRtpsSxy2AndIr0 =
        out.rtOutput.schedule.storedRtpsSxy2AndIr0;
    out.flagAfterRtptKnown = out.rtOutput.flagAfterRtptKnown;
    out.flagAfterRtpt = out.rtOutput.flagAfterRtpt;
    out.flagAfterRtpsKnown = out.rtOutput.flagAfterRtpsKnown;
    out.flagAfterRtps = out.rtOutput.flagAfterRtps;
    out.sxy = out.rtOutput.sxy;
    out.szFifoAfterRtpt = out.rtOutput.szAfterRtpt;
    out.szFifoAfterRtps = out.rtOutput.szAfterRtps;
    out.sz3AfterRtpsKnown = out.rtOutput.sz3AfterRtpsKnown;
    out.sz3AfterRtps = out.rtOutput.sz3AfterRtps;
    out.sxyKnown = out.rtOutput.sxyWordsKnown;
    out.flagKnown = out.rtOutput.flagOrKnown;
    out.flag = out.rtOutput.flagOr;
    out.ir0DepthKnown = out.rtOutput.ir0Known;
    out.ir0Depth = out.rtOutput.ir0;
    out.returnValueKnown = out.rtOutput.returnValueKnown;
    out.returnValue = out.rtOutput.returnValue;
    return out;
}

Matrix8003F1B4 IdentityMatrixGlobals80091838() {
    Matrix8003F1B4 matrix{};
    matrix.words[0] = 0x00001000u;
    matrix.words[1] = 0x00000000u;
    matrix.words[2] = 0x00001000u;
    matrix.words[3] = 0x00000000u;
    matrix.words[4] = 0x00001000u;
    return matrix;
}

TransformPrelude8003F1B4 BuildTransformPreludeImpl8003F1B4(
    const GsSpriteLocalFields8003F1B4& sprite,
    const GteControlState8003F1B4& gte) {
    TransformPrelude8003F1B4 out{};
    out.evaluated = true;
    out.scale = {sprite.scaleX_1C, sprite.scaleY_1E, 0};
    if (sprite.rot_20 != 0) {
        out.usedRotMatrix8003F790 = true;
        out.matrix = PsxCall8003F790_RotMatrix(
            0,
            0,
            static_cast<int16_t>(sprite.rot_20 / 360));
    } else {
        out.copiedIdentityMatrixGlobals80091838 = true;
        out.matrix = IdentityMatrixGlobals80091838();
    }
    out.matrixValuesKnown = true;
    const bool hasNonIdentityScale =
        (static_cast<uint32_t>(static_cast<uint16_t>(sprite.scaleX_1C)) |
         (static_cast<uint32_t>(static_cast<uint16_t>(sprite.scaleY_1E))
          << 16)) != kGsSortSpriteFastIdentityScale8003F1B4;
    if (hasNonIdentityScale) {
        out.matrix = PsxCall8003B0FC_ScaleMatrix(out.matrix, out.scale);
        out.scaleMatrixApplied8003B0FC = true;
    }

    out.readGeomScreen8003F700 = true;
    out.geomScreenKnown = gte.geomScreenKnown;
    out.geomScreen = gte.geomScreen;
    out.geomOffsetKnown = gte.geomOffsetKnown;
    out.geomOffsetX = gte.geomOffsetX;
    out.geomOffsetY = gte.geomOffsetY;
    out.depthCueKnown = gte.depthCueKnown;
    out.zScaleFactorKnown = gte.zScaleFactorKnown;
    if (!out.geomScreenKnown) {
        out.geomScreenGap8003F700 = true;
        out.matrixValuesKnown = false;
    }
    if (!out.geomOffsetKnown) {
        out.geomOffsetGap800402C8 = true;
    }
    out.transMatrixApplied8003B0CC = true;
    if (out.geomScreenKnown) {
        out.matrix = PsxCall8003B0CC_TransMatrix(
            out.matrix,
            ScaleVector8003F1B4{sprite.x_04, sprite.y_06,
                                static_cast<int32_t>(out.geomScreen)});
    }
    out.setRotMatrix8003F6B0 = true;
    out.setTransMatrix8003F6E0 = true;
    out.rotTransPers4 =
        PsxCall8003F710_RotTransPers4Gap(sprite,
                                          out.matrix,
                                          out.matrixValuesKnown,
                                          gte);
    out.rotTransPers4Gap8003F710 = !out.rotTransPers4.sxyKnown;
    return out;
}

GsSortSpriteSkipReason8003F1B4 ResolveSkipReason8003F1B4(
    const GsSpriteLocalFields8003F1B4& sprite) {
    if ((sprite.attr_00 & 0x80000000u) != 0u) {
        return GsSortSpriteSkipReason8003F1B4::NegativeAttr;
    }
    if (sprite.width_08 == 0u) {
        return GsSortSpriteSkipReason8003F1B4::ZeroWidth;
    }
    if (sprite.height_0A == 0u) {
        return GsSortSpriteSkipReason8003F1B4::ZeroHeight;
    }
    return GsSortSpriteSkipReason8003F1B4::None;
}

GsSortSpritePacketPath8003F1B4 ResolvePacketPathImpl8003F1B4(
    const GsSpriteLocalFields8003F1B4& sprite) {
    const uint32_t scalePair =
        static_cast<uint32_t>(static_cast<uint16_t>(sprite.scaleX_1C)) |
        (static_cast<uint32_t>(static_cast<uint16_t>(sprite.scaleY_1E))
         << 16);
    const bool attrBit27 = (sprite.attr_00 & 0x08000000u) != 0u;
    if (attrBit27) {
        return GsSortSpritePacketPath8003F1B4::Fast;
    }
    if (scalePair == kGsSortSpriteFastIdentityScale8003F1B4 &&
        sprite.rot_20 == 0 &&
        (sprite.attr_00 & 0x00C00000u) == 0u) {
        return GsSortSpritePacketPath8003F1B4::Fast;
    }
    return GsSortSpritePacketPath8003F1B4::Transform;
}

uint32_t PayloadWordCountForPath8003F1B4(
    GsSortSpritePacketPath8003F1B4 path) {
    switch (path) {
    case GsSortSpritePacketPath8003F1B4::Fast:
        return kGsSortSpritePacketFastPayloadWords8003F1B4;
    case GsSortSpritePacketPath8003F1B4::Transform:
        return kGsSortSpritePacketTransformPayloadWords8003F1B4;
    case GsSortSpritePacketPath8003F1B4::None:
        return 0;
    }
    return 0;
}

bool AppendRuntimePacketWrite8003F1B4(
    RuntimeState8003F1B4& runtime,
    const RuntimePacketWrite8003F1B4& write) {
    uint32_t submitOrder = 1u;
    for (RuntimePacketWrite8003F1B4& slot : runtime.packetWrites) {
        if (slot.valid) {
            ++submitOrder;
            continue;
        }
        RuntimePacketWrite8003F1B4 orderedWrite = write;
        orderedWrite.submitOrderKnown = true;
        orderedWrite.submitOrder = submitOrder;
        slot = orderedWrite;
        slot.valid = true;
        return true;
    }
    return false;
}

RuntimePacketWrite8003F1B4 BuildRuntimePacketWrite8003F1B4(
    const GsSortSpriteResult8003F1B4& result) {
    const GsSortSpritePacket8003F1B4& packet = result.packet;
    RuntimePacketWrite8003F1B4 out{};
    out.valid = packet.written;
    out.addr = packet.addr;
    out.wordCount = packet.totalWordCount;
    out.packetPath = result.packetPath;
    out.priorityKnown = result.priorityKnown;
    out.priority = result.priority;
    out.otSlotKnown = true;
    out.otSlot = result.otSlot;
    out.allocatorKnown = true;
    out.allocator = result.allocator;
    switch (packet.totalWordCount) {
    case kGsSortSpritePacketFastTotalWords8003F1B4: {
        const auto words = packet.FastWords();
        for (std::size_t i = 0; i < words.size(); ++i) {
            out.words[i] = words[i];
            out.wordKnown[i] = packet.fieldsKnown;
        }
        break;
    }
    case kGsSortSpritePacketTransformTotalWords8003F1B4: {
        const auto words = packet.TransformWords();
        for (std::size_t i = 0; i < words.size(); ++i) {
            out.words[i] = words[i];
        }
        out.wordKnown[0] = true;
        out.wordKnown[1] = packet.transformNonGeometryFieldsKnown;
        out.wordKnown[2] = !packet.transformFieldsGap;
        out.wordKnown[3] = packet.transformNonGeometryFieldsKnown;
        out.wordKnown[4] = !packet.transformFieldsGap;
        out.wordKnown[5] = packet.transformNonGeometryFieldsKnown;
        out.wordKnown[6] = !packet.transformFieldsGap;
        out.wordKnown[7] = packet.transformNonGeometryFieldsKnown;
        out.wordKnown[8] = !packet.transformFieldsGap;
        out.wordKnown[9] = packet.transformNonGeometryFieldsKnown;
        break;
    }
    default:
        out.valid = false;
        break;
    }
    return out;
}

RuntimeUpdate8003F1B4 ResolveRuntimeUpdate8003F1B4(
    RuntimeState8003F1B4& runtime,
    const GsSortSpriteResult8003F1B4& result,
    bool apply) {
    RuntimeUpdate8003F1B4 out{};
    out.dryRun = !apply;
    out.skipped = result.skipped;
    out.skipReason = result.skipReason;
    out.packetPath = result.packetPath;
    out.oldAllocatorAddr = runtime.dword_800901C8;
    out.newAllocatorAddr = runtime.dword_800901C8;
    out.allocatorOldMatchesRuntime =
        runtime.dword_800901C8 == result.allocator.oldAddr;
    out.packetWouldWrite = result.packet.written;
    out.packetAddr = result.packet.addr;
    out.packetPayloadWordCount = result.packet.payloadWordCount;
    out.packetTotalWordCount = result.packet.totalWordCount;
    out.otSlot = result.otSlot;
    out.returnValue = result.returnValue;

    const uint32_t runtimeSlotOldValue =
        GetRuntimeOtSlotOldValue8003F1B4(runtime.ot, result.otSlot.addr);
    out.otSlotOldMatchesRuntime =
        runtimeSlotOldValue == result.otSlot.oldValue;

    if (result.skipped) {
        return out;
    }

    out.allocatorWouldUpdate = true;
    out.newAllocatorAddr = result.allocator.newAddr;
    out.otSlotWouldUpdate = true;

    if (!apply) {
        return out;
    }

    if (result.packet.written) {
        out.packetWriteMirrored = AppendRuntimePacketWrite8003F1B4(
            runtime,
            BuildRuntimePacketWrite8003F1B4(result));
        out.packetWriteCapacityExceeded = !out.packetWriteMirrored;
    }
    out.otSlotUpdated =
        SetRuntimeOtSlotValue8003F1B4(runtime.ot,
                                      result.otSlot.addr,
                                      result.otSlot.newValue);
    out.otSlotCapacityExceeded = !out.otSlotUpdated;
    if (!out.otSlotUpdated) {
        return out;
    }

    runtime.dword_800901C8 = result.allocator.newAddr;
    out.allocatorUpdated = true;
    out.newAllocatorAddr = result.allocator.newAddr;
    return out;
}

} // namespace

GsSortSpritePacketPath8003F1B4 ResolvePacketPath8003F1B4(
    const GsSpriteLocalFields8003F1B4& sprite) {
    return ResolvePacketPathImpl8003F1B4(sprite);
}

TransformPrelude8003F1B4 BuildTransformPrelude8003F1B4(
    const GsSpriteLocalFields8003F1B4& sprite,
    const GteControlState8003F1B4& gte) {
    return BuildTransformPreludeImpl8003F1B4(sprite, gte);
}

GsSortSpriteResult8003F1B4 PsxCall8003F1B4_GsSortSprite(
    const GsSortSpriteInput8003F1B4& input) {
    GsSortSpriteResult8003F1B4 out{};
    out.otSlot =
        PsxCall8003EF5C_LinkPacket(input.packetAllocatorAddr_dword_800901C8,
                                   input.ot,
                                   input.priority,
                                   0u);
    out.otSlot.newValue = input.ot.slotOldValue;
    out.allocator.oldAddr = input.packetAllocatorAddr_dword_800901C8;
    out.allocator.newAddr = input.packetAllocatorAddr_dword_800901C8;

    out.skipReason = ResolveSkipReason8003F1B4(input.sprite);
    out.skipped = out.skipReason != GsSortSpriteSkipReason8003F1B4::None;
    if (out.skipped) {
        return out;
    }

    out.packetPath = ResolvePacketPathImpl8003F1B4(input.sprite);
    out.priorityKnown = true;
    out.priority = input.priority;
    const uint32_t payloadWordCount =
        PayloadWordCountForPath8003F1B4(out.packetPath);
    const uint32_t totalWordCount = payloadWordCount + 1u;
    const uint32_t packetAddr = input.packetAllocatorAddr_dword_800901C8;

    out.packet.written = true;
    out.packet.addr = packetAddr;
    out.packet.payloadWordCount = payloadWordCount;
    out.packet.totalWordCount = totalWordCount;
    out.packet.word0_linkTag =
        MakePacketLinkTag8003F1B4(input.ot.slotOldValue, payloadWordCount);

    if (out.packetPath == GsSortSpritePacketPath8003F1B4::Fast) {
        out.packet.fieldsKnown = true;
        out.packet.word1_drawModeOrColor =
            MakeDrawModeWord8003F1B4(input.sprite.attr_00,
                                     input.sprite.tpage_0C);
        out.packet.word2_colorOrXy0 =
            MakeFastColorCodeWord8003F1B4(input.sprite.attr_00,
                                          input.sprite.r_14,
                                          input.sprite.g_15,
                                          input.sprite.b_16);
        out.packet.word3_xyOrUv0 =
            MakeFastXyWord8003F1B4(input.sprite, input.drawOffsets);
        out.packet.word4_uvOrXy1 = MakeUvClutWord8003F1B4(input.sprite);
        out.packet.word5_whOrUv1 = MakeWhWord8003F1B4(input.sprite);
    } else {
        out.packet.transformPrelude =
            BuildTransformPreludeImpl8003F1B4(input.sprite, input.gte);
        out.packet.transformFieldsGap =
            !out.packet.transformPrelude.rotTransPers4.sxyKnown;
        out.packet.transformNonGeometryFieldsKnown = true;

        const uint8_t u0 = TransformUvLeft8003F1B4(input.sprite);
        const uint8_t u1 = TransformUvRight8003F1B4(input.sprite);
        const uint8_t v0 = TransformUvTop8003F1B4(input.sprite);
        const uint8_t v1 = TransformUvBottom8003F1B4(input.sprite);
        out.packet.word1_drawModeOrColor =
            MakeTransformColorCodeWord8003F1B4(input.sprite.attr_00,
                                               input.sprite.r_14,
                                               input.sprite.g_15,
                                               input.sprite.b_16);
        out.packet.word3_xyOrUv0 =
            MakeTransformUvClutWord8003F1B4(u0, v0, input.sprite);
        out.packet.word5_whOrUv1 =
            MakeTransformUvTpageWord8003F1B4(u1, v0, input.sprite);
        out.packet.word7_uv2 =
            static_cast<uint32_t>(u0) | (static_cast<uint32_t>(v1) << 8);
        out.packet.word9_uv3 =
            static_cast<uint32_t>(u1) | (static_cast<uint32_t>(v1) << 8);
        if (!out.packet.transformFieldsGap) {
            const auto& sxy = out.packet.transformPrelude.rotTransPers4.sxy;
            out.packet.word2_colorOrXy0 = sxy[0].word;
            out.packet.word4_uvOrXy1 = sxy[1].word;
            out.packet.word6_xy2 = sxy[2].word;
            out.packet.word8_xy3 = sxy[3].word;
        }
    }

    out.otSlot = PsxCall8003EF5C_LinkPacket(
        packetAddr,
        input.ot,
        input.priority,
        static_cast<uint8_t>(payloadWordCount));
    out.allocator.advanceWords = totalWordCount;
    out.allocator.advanceBytes = totalWordCount * 4u;
    out.allocator.newAddr = packetAddr + out.allocator.advanceBytes;
    return out;
}

uint32_t ResolveRuntimeOtSlotAddr8003F1B4(
    const RuntimeOrderingTableState8003F1B4& ot,
    uint16_t priority) {
    return ot.headAddr_04 + static_cast<uint32_t>(priority) * 4u -
           ot.length_08 * 4u;
}

bool TryGetRuntimeOtSlotValue8003F1B4(
    const RuntimeOrderingTableState8003F1B4& ot,
    uint32_t addr,
    uint32_t* outValue) {
    for (const RuntimeOtSlotValue8003F1B4& slot : ot.slotValues) {
        if (slot.valid && slot.addr == addr) {
            if (outValue != nullptr) {
                *outValue = slot.value;
            }
            return true;
        }
    }
    return false;
}

uint32_t GetRuntimeOtSlotOldValue8003F1B4(
    const RuntimeOrderingTableState8003F1B4& ot,
    uint32_t addr) {
    uint32_t value = 0;
    if (TryGetRuntimeOtSlotValue8003F1B4(ot, addr, &value)) {
        return value;
    }
    return 0;
}

bool SetRuntimeOtSlotValue8003F1B4(
    RuntimeOrderingTableState8003F1B4& ot,
    uint32_t addr,
    uint32_t value) {
    for (RuntimeOtSlotValue8003F1B4& slot : ot.slotValues) {
        if (slot.valid && slot.addr == addr) {
            slot.value = value;
            return true;
        }
    }

    for (RuntimeOtSlotValue8003F1B4& slot : ot.slotValues) {
        if (!slot.valid) {
            slot.valid = true;
            slot.addr = addr;
            slot.value = value;
            return true;
        }
    }
    return false;
}

bool SetRuntimeOtSlotValueForPriority8003F1B4(
    RuntimeOrderingTableState8003F1B4& ot,
    uint16_t priority,
    uint32_t value) {
    return SetRuntimeOtSlotValue8003F1B4(
        ot,
        ResolveRuntimeOtSlotAddr8003F1B4(ot, priority),
        value);
}

OrderingTableState8003F1B4 BuildOrderingTableStateFromRuntime8003F1B4(
    const RuntimeOrderingTableState8003F1B4& ot,
    uint16_t priority) {
    OrderingTableState8003F1B4 out{};
    out.headAddr_04 = ot.headAddr_04;
    out.length_08 = ot.length_08;
    out.slotOldValue = GetRuntimeOtSlotOldValue8003F1B4(
        ot,
        ResolveRuntimeOtSlotAddr8003F1B4(ot, priority));
    return out;
}

GsSortSpriteInput8003F1B4 BuildInputFromRuntime8003F1B4(
    const RuntimeState8003F1B4& runtime,
    const GsSpriteLocalFields8003F1B4& sprite,
    uint16_t priority,
    uint16_t rotMatrixArg) {
    GsSortSpriteInput8003F1B4 out{};
    out.sprite = sprite;
    out.ot = BuildOrderingTableStateFromRuntime8003F1B4(runtime.ot, priority);
    out.drawOffsets.word_800917AA = runtime.word_800917AA;
    out.drawOffsets.word_800917AC = runtime.word_800917AC;
    out.gte = runtime.gte;
    out.packetAllocatorAddr_dword_800901C8 = runtime.dword_800901C8;
    out.priority = priority;
    out.rotMatrixArg = rotMatrixArg;
    return out;
}

RuntimeUpdate8003F1B4 PredictRuntimeUpdate8003F1B4(
    const RuntimeState8003F1B4& runtime,
    const GsSortSpriteResult8003F1B4& result) {
    RuntimeState8003F1B4 copy = runtime;
    return ResolveRuntimeUpdate8003F1B4(copy, result, false);
}

RuntimeUpdate8003F1B4 ApplyRuntimeUpdate8003F1B4(
    RuntimeState8003F1B4& runtime,
    const GsSortSpriteResult8003F1B4& result) {
    return ResolveRuntimeUpdate8003F1B4(runtime, result, true);
}

} // namespace PrPsxGsSpriteSubmitDirect
