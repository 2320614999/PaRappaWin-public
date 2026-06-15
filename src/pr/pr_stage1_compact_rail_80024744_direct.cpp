#include "pr_stage1_compact_rail_80024744_direct.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace PrStage1CompactRail80024744Direct {
namespace {

using PrStageSceneSubmitDirect::PsxCompactRailRow80024744;
using PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744;
using PrStageSceneSubmitDirect::PsxSpriteTemplate80024744;

PsxCompactRailFrameDebug80024744 s_lastFrameDebug80024744{};
uint32_t s_frameDebugSerial80024744 = 0;

constexpr int32_t kCompactWobbleBankEntryCount80024744 = 36;
constexpr int32_t kTeacherLinearEnd80023F20 = 6;
constexpr int32_t kStudentLinearEnd80024114 = 5;
constexpr int32_t kCompactWobbleSinEnd80023F20_80024114 = 22;
constexpr int32_t kCompactWobbleStepCount80023F20_80024114 = 24;
constexpr int32_t kCompactWobblePhaseWrap80023F20_80024114 = 8193;
constexpr int32_t kCompactWobblePhaseStep80023F20_80024114 = 256;
constexpr int16_t kCompactSharedScaleIdentity80024418 = 4096;
constexpr int32_t kCompactWobbleLinearAccInit80024308_80024390 = 2048;
constexpr int32_t kCompactWobbleLinearVelInit80024308_80024390 = 2048;
constexpr int32_t kCompactWobbleLinearVelFlip80023F20_80024114 = -1024;
constexpr uint32_t kLocalSpriteAttr80024418 = 0x50000000u;
constexpr uint32_t kNoteTemplateTableBase800540BC = 0x800540BCu;
constexpr uint32_t kLargeThresholdTable800540E8 = 0x800540E8u;
constexpr uint32_t kSmallThresholdTable80054100 = 0x80054100u;
constexpr uint32_t kLargeLitTemplate800540AC = 0x800540ACu;
constexpr uint32_t kLargeUnlitTemplate8005409C = 0x8005409Cu;
constexpr uint32_t kSmallLitTemplate8005408C = 0x8005408Cu;
constexpr uint32_t kSmallUnlitTemplate8005407C = 0x8005407Cu;
constexpr uint32_t kTeacherOnSiTemplate80053FFC = 0x80053FFCu;
constexpr uint32_t kStudentPaSiTemplate8005400C = 0x8005400Cu;
constexpr uint8_t kCompactRailMaxRows80024744 =
    static_cast<uint8_t>(PrStageSceneSubmitDirect::kCompactRailMaxRows80024744);
constexpr uint8_t kCompactRailBodySlotCount80024744 = static_cast<uint8_t>(
    PrStageSceneSubmitDirect::kCompactRailBodySlotCount80024744);

PsxSpriteTemplate80024744 MakeSpriteTemplate80024744(
    uint16_t texX,
    uint16_t texY,
    uint16_t w,
    uint16_t h,
    uint16_t clutY) {
    PsxSpriteTemplate80024744 out{};
    out.attr = 0x50000040u;
    out.texX_hw = texX;
    out.texY_px = texY;
    out.w = w;
    out.h = h;
    out.clutX_px = 0x0120u;
    out.clutY_px = clutY;
    return out;
}

uint16_t PsxGetTPage80043DF4(uint16_t tp,
                             uint16_t abr,
                             uint16_t x,
                             uint16_t y) {
    return static_cast<uint16_t>(((tp & 0x3u) << 7) |
                                 ((abr & 0x3u) << 5) |
                                 ((x & 0x03FFu) >> 6) |
                                 ((y & 0x0100u) >> 4));
}

uint16_t PsxFastSpriteTPageFromTemplate8001B25C(
    const PsxSpriteTemplate80024744& tpl,
    uint16_t glyphIndex) {
    const uint16_t uFull = static_cast<uint16_t>(
        4u * tpl.texX_hw + glyphIndex * tpl.w);
    return PsxGetTPage80043DF4(
        0,
        1,
        static_cast<uint16_t>((uFull & 0xFF00u) >> 2),
        static_cast<uint16_t>(tpl.texY_px & 0xFF00u));
}

uint16_t PsxGsSpriteTPageFromTemplate8001B338(
    const PsxSpriteTemplate80024744& tpl) {
    return PsxGetTPage80043DF4(
        0,
        1,
        static_cast<uint16_t>(tpl.texX_hw & 0x3FC0u),
        static_cast<uint16_t>(tpl.texY_px & 0xFF00u));
}

PsxCompactRailWorkOtMetadata80024744 MakeWorkOtMetadata8001C550_8001C804() {
    PsxCompactRailWorkOtMetadata80024744 out{};
    out.valid = true;
    return out;
}

PsxCompactRailLocalSpriteRgbCarrier80024744 NormalizeRgbCarrier80024744(
    PsxCompactRailLocalSpriteRgbCarrier80024744 carrier) {
    if (carrier.known) {
        if (carrier.source ==
            PsxCompactRailLocalSpriteRgbCarrier80024744::Source::Unknown) {
            carrier.source = PsxCompactRailLocalSpriteRgbCarrier80024744::
                Source::AuthoritativeCallerLocal;
        }
        carrier.gap = PsxCompactRailLocalSpriteRgbCarrier80024744::Gap::None;
    } else {
        carrier.source =
            PsxCompactRailLocalSpriteRgbCarrier80024744::Source::Unknown;
        if (carrier.gap ==
            PsxCompactRailLocalSpriteRgbCarrier80024744::Gap::None) {
            carrier.gap = PsxCompactRailLocalSpriteRgbCarrier80024744::Gap::
                NoAuthoritativeCallerField;
        }
    }
    return carrier;
}

PsxCompactRailLocalSpriteRgbCarrier80024744
ApplyStaticNoRgbWriteEvidence80024744(
    PsxCompactRailLocalSpriteRgbCarrier80024744 carrier,
    uint32_t wrapperFunction,
    uint32_t localBuilderFunction,
    uint32_t submitFunction) {
    if (!carrier.known && carrier.evidence ==
                              PsxCompactRailLocalSpriteRgbCarrier80024744::
                                  Evidence::None) {
        carrier.gap = PsxCompactRailLocalSpriteRgbCarrier80024744::Gap::
            StaticChainHasNoRgbWrite;
        carrier.evidence =
            PsxCompactRailLocalSpriteRgbCarrier80024744::Evidence::
                CompactRail80024744StaticChainNoRgbWrite;
        carrier.callerFunction80024744 = 0x80024744u;
        carrier.wrapperFunction = wrapperFunction;
        carrier.localBuilderFunction = localBuilderFunction;
        carrier.submitFunction = submitFunction;
        carrier.fixedConstantWriteObserved = false;
        carrier.templateWriteObserved = false;
        carrier.callArgumentWriteObserved = false;
        carrier.globalTableWriteObserved = false;
        carrier.stackLocalWriteObserved = false;
    }
    return carrier;
}

void ApplyWorkOtMetadataFromRuntime8003FA20(
    PsxCompactRailWorkOtMetadata80024744& out,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20*
        runtime8003FA20,
    uint8_t workIndex8004019C,
    uint16_t priority) {
    if (runtime8003FA20 != nullptr) {
        out.workIndexKnown = true;
        out.workIndex = workIndex8004019C & 1u;
        out.workPtrKnown = true;
        out.workPtr =
            out.workBase80087288 +
            out.workStrideBytes * static_cast<uint32_t>(out.workIndex);
        out.packetAllocatorKnown = true;
        out.packetAllocatorAddr = runtime8003FA20->dword_800901C8;
        out.drawOffsetsKnown = true;
        out.drawOffsetX = runtime8003FA20->word_800917AA;
        out.drawOffsetY = runtime8003FA20->word_800917AC;
        out.otSlotKnown = true;
        out.otSlotAddr = PrPsxFastSpriteSubmitDirect::
            ResolveRuntimeOtSlotAddr8003FA20(runtime8003FA20->ot, priority);
    }
}

void ApplyWorkOtMetadataFromRuntime8003F1B4(
    PsxCompactRailWorkOtMetadata80024744& out,
    const PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4* runtime8003F1B4,
    uint8_t workIndex8004019C,
    uint16_t priority) {
    if (runtime8003F1B4 != nullptr) {
        out.workIndexKnown = true;
        out.workIndex = workIndex8004019C & 1u;
        out.workPtrKnown = true;
        out.workPtr =
            out.workBase80087288 +
            out.workStrideBytes * static_cast<uint32_t>(out.workIndex);
        out.packetAllocatorKnown = true;
        out.packetAllocatorAddr = runtime8003F1B4->dword_800901C8;
        out.drawOffsetsKnown = true;
        out.drawOffsetX = runtime8003F1B4->word_800917AA;
        out.drawOffsetY = runtime8003F1B4->word_800917AC;
        out.otSlotKnown = true;
        out.otSlotAddr = PrPsxGsSpriteSubmitDirect::
            ResolveRuntimeOtSlotAddr8003F1B4(runtime8003F1B4->ot, priority);
    }
}

PsxCompactRailSubmit8001C550Debug80024744 BuildSubmit8001C550Metadata(
    const PsxSpriteTemplate80024744& tpl,
    uint32_t templatePtr,
    int16_t screenX,
    int16_t screenY,
    uint16_t priority,
    uint32_t callsite,
    uint32_t psxCallOrder,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime8003FA20,
    uint8_t workIndex8004019C,
    PsxCompactRailLocalSpriteRgbCarrier80024744 rgbCarrier) {
    PsxCompactRailSubmit8001C550Debug80024744 out{};
    out.valid = true;
    out.priority = priority;
    out.templatePtr = templatePtr;
    out.argScreenX = screenX;
    out.argScreenY = screenY;
    out.localX = static_cast<int16_t>(screenX - 160);
    out.localY = static_cast<int16_t>(screenY - 120);
    out.localFastSpritePrefixKnown = true;
    out.localFastSpriteRgb = NormalizeRgbCarrier80024744(
        ApplyStaticNoRgbWriteEvidence80024744(
            rgbCarrier,
            0x8001C550u,
            0x8001B25Cu,
            0x8003FA20u));
    out.localFastSpriteRgbKnown = out.localFastSpriteRgb.known;
    out.localFastSpriteKnown = out.localFastSpritePrefixKnown &&
                               out.localFastSpriteRgbKnown;
    out.localFastSprite.attr_00 = tpl.attr;
    out.localFastSprite.x_04 = out.localX;
    out.localFastSprite.y_06 = static_cast<uint16_t>(out.localY);
    out.localFastSprite.width_08 = tpl.w;
    out.localFastSprite.height_0A = tpl.h;
    out.localFastSprite.tpage_0C =
        PsxFastSpriteTPageFromTemplate8001B25C(tpl, out.glyphIndex);
    out.localFastSprite.u_0E =
        static_cast<uint8_t>((4u * tpl.texX_hw) & 0xFFu);
    out.localFastSprite.v_0F = static_cast<uint8_t>(tpl.texY_px & 0xFFu);
    out.localFastSprite.clutX_10 = static_cast<int16_t>(tpl.clutX_px);
    out.localFastSprite.clutY_12 = static_cast<int16_t>(tpl.clutY_px);
    if (out.localFastSpriteRgbKnown) {
        out.localFastSprite.r_14 = out.localFastSpriteRgb.r;
        out.localFastSprite.g_15 = out.localFastSpriteRgb.g;
        out.localFastSprite.b_16 = out.localFastSpriteRgb.b;
    }
    out.workOt = MakeWorkOtMetadata8001C550_8001C804();
    ApplyWorkOtMetadataFromRuntime8003FA20(
        out.workOt,
        runtime8003FA20,
        workIndex8004019C,
        priority);
    if (out.localFastSpriteKnown && runtime8003FA20 != nullptr) {
        PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20
            submitInput =
                PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
                    *runtime8003FA20,
                    out.localFastSprite,
                    priority);
        submitInput.provenance.active = true;
        submitInput.provenance.sourceKind =
            PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
                Stage1CompactRail;
        submitInput.provenance.sourceFunction = 0x8001C550u;
        submitInput.provenance.callsite = callsite;
        submitInput.provenance.helper = 0x8001B590u;
        submitInput.provenance.rawLocalProvenance =
            PrPsxFastSpriteSubmitDirect::
                FastSpriteRawLocalProvenance8003FA20::StaticLocalFields;
        submitInput.provenance.psxCallOrder = psxCallOrder;
        submitInput.provenance.priority = priority;
        out.submitMetadata8003FA20 =
            PrPsxFastSpriteSubmitDirect::PsxCall8003FA20_GsSortFastSprite(
                submitInput);
        out.submitRuntimeUpdate8003FA20 =
            PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
                *runtime8003FA20,
                out.submitMetadata8003FA20);
        out.submitPacketGap =
            !out.submitRuntimeUpdate8003FA20.packetWriteMirrored;
    }
    return out;
}

PsxCompactRailSubmit8001C804Debug80024744 BuildSubmit8001C804Metadata(
    const PsxSpriteTemplate80024744& tpl,
    uint32_t templatePtr,
    int16_t centerX,
    int16_t centerY,
    int16_t scaleX,
    int16_t scaleY,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4* runtime8003F1B4,
    uint8_t workIndex8004019C,
    PsxCompactRailLocalSpriteRgbCarrier80024744 rgbCarrier) {
    PsxCompactRailSubmit8001C804Debug80024744 out{};
    out.valid = true;
    out.templatePtr = templatePtr;
    out.argCenterX = centerX;
    out.argCenterY = centerY;
    out.localX = static_cast<int16_t>(centerX - 160);
    out.localY = static_cast<int16_t>(centerY - 120);
    out.localGsSpritePrefixKnown = true;
    out.localGsSpriteRgb = NormalizeRgbCarrier80024744(
        ApplyStaticNoRgbWriteEvidence80024744(
            rgbCarrier,
            0x8001C804u,
            0x8001B338u,
            0x8003F1B4u));
    out.localGsSpriteRgbKnown = out.localGsSpriteRgb.known;
    out.localGsSpriteKnown = out.localGsSpritePrefixKnown &&
                             out.localGsSpriteRgbKnown;
    out.localGsSprite.attr_00 = kLocalSpriteAttr80024418 | 0x40u;
    out.localGsSprite.x_04 = out.localX;
    out.localGsSprite.y_06 = out.localY;
    out.localGsSprite.width_08 = tpl.w;
    out.localGsSprite.height_0A = tpl.h;
    out.localGsSprite.tpage_0C = PsxGsSpriteTPageFromTemplate8001B338(tpl);
    out.localGsSprite.u_0E = static_cast<uint8_t>((4u * tpl.texX_hw) & 0xFFu);
    out.localGsSprite.v_0F = static_cast<uint8_t>(tpl.texY_px & 0xFFu);
    out.localGsSprite.clutX_10 = static_cast<int16_t>(tpl.clutX_px);
    out.localGsSprite.clutY_12 = static_cast<int16_t>(tpl.clutY_px);
    out.localGsSprite.mx_18 = static_cast<int16_t>(tpl.w / 2u);
    out.localGsSprite.my_1A = static_cast<int16_t>(tpl.h / 2u);
    out.localGsSprite.scaleX_1C = scaleX;
    out.localGsSprite.scaleY_1E = scaleY;
    if (out.localGsSpriteRgbKnown) {
        out.localGsSprite.r_14 = out.localGsSpriteRgb.r;
        out.localGsSprite.g_15 = out.localGsSpriteRgb.g;
        out.localGsSprite.b_16 = out.localGsSpriteRgb.b;
    }
    out.localBackendGsSprite.attr_00 = out.localGsSprite.attr_00;
    out.localBackendGsSprite.x_04 = out.localGsSprite.x_04;
    out.localBackendGsSprite.y_06 = out.localGsSprite.y_06;
    out.localBackendGsSprite.width_08 = out.localGsSprite.width_08;
    out.localBackendGsSprite.height_0A = out.localGsSprite.height_0A;
    out.localBackendGsSprite.tpage_0C = out.localGsSprite.tpage_0C;
    out.localBackendGsSprite.u_0E = out.localGsSprite.u_0E;
    out.localBackendGsSprite.v_0F = out.localGsSprite.v_0F;
    out.localBackendGsSprite.clutX_10 = out.localGsSprite.clutX_10;
    out.localBackendGsSprite.clutY_12 = out.localGsSprite.clutY_12;
    out.localBackendGsSprite.r_14 = out.localGsSprite.r_14;
    out.localBackendGsSprite.g_15 = out.localGsSprite.g_15;
    out.localBackendGsSprite.b_16 = out.localGsSprite.b_16;
    out.localBackendGsSprite.mx_18 = out.localGsSprite.mx_18;
    out.localBackendGsSprite.my_1A = out.localGsSprite.my_1A;
    out.localBackendGsSprite.scaleX_1C = out.localGsSprite.scaleX_1C;
    out.localBackendGsSprite.scaleY_1E = out.localGsSprite.scaleY_1E;
    out.workOt = MakeWorkOtMetadata8001C550_8001C804();
    ApplyWorkOtMetadataFromRuntime8003F1B4(
        out.workOt,
        runtime8003F1B4,
        workIndex8004019C,
        out.priority);
    if ((out.localGsSpriteKnown || out.localGsSpritePrefixKnown) &&
        runtime8003F1B4 != nullptr) {
        const PrPsxGsSpriteSubmitDirect::GsSortSpriteInput8003F1B4 submitInput =
            PrPsxGsSpriteSubmitDirect::BuildInputFromRuntime8003F1B4(
                *runtime8003F1B4,
                out.localBackendGsSprite,
                out.priority,
                0u);
        const PrPsxGsSpriteSubmitDirect::GsSortSpritePacketPath8003F1B4
            packetPath =
                PrPsxGsSpriteSubmitDirect::ResolvePacketPath8003F1B4(
                    submitInput.sprite);
        if (!out.localGsSpriteKnown &&
            packetPath ==
                PrPsxGsSpriteSubmitDirect::GsSortSpritePacketPath8003F1B4::
                    Transform) {
            out.submitMetadata8003F1B4.priorityKnown = true;
            out.submitMetadata8003F1B4.priority = out.priority;
            out.submitMetadata8003F1B4.packetPath = packetPath;
            out.submitMetadata8003F1B4.packet.transformPrelude =
                PrPsxGsSpriteSubmitDirect::BuildTransformPrelude8003F1B4(
                    submitInput.sprite,
                    submitInput.gte);
            out.submitMetadata8003F1B4.packet.transformFieldsGap =
                !out.submitMetadata8003F1B4.packet.transformPrelude
                     .rotTransPers4.sxyKnown;
            out.submitPacketGap = true;
            return out;
        }
    }
    if (out.localGsSpriteKnown && runtime8003F1B4 != nullptr) {
        const PrPsxGsSpriteSubmitDirect::GsSortSpriteInput8003F1B4 submitInput =
            PrPsxGsSpriteSubmitDirect::BuildInputFromRuntime8003F1B4(
                *runtime8003F1B4,
                out.localBackendGsSprite,
                out.priority,
                0u);
        out.submitMetadata8003F1B4 =
            PrPsxGsSpriteSubmitDirect::PsxCall8003F1B4_GsSortSprite(
                submitInput);
        out.submitRuntimeUpdate8003F1B4 =
            PrPsxGsSpriteSubmitDirect::ApplyRuntimeUpdate8003F1B4(
                *runtime8003F1B4,
                out.submitMetadata8003F1B4);
        out.submitPacketGap =
            !out.submitRuntimeUpdate8003F1B4.packetWriteMirrored;
    }
    return out;
}

uint32_t ResolveNoteTemplatePtr800540BC(int16_t type) {
    switch (type) {
    case 1:
        return 0x8005405Cu;
    case 2:
        return 0x8005403Cu;
    case 3:
        return 0x8005404Cu;
    case 4:
        return 0x8005406Cu;
    case 5:
    case 6:
        return 0x8005401Cu;
    case 7:
    case 8:
        return 0x8005402Cu;
    default:
        break;
    }
    return 0u;
}

uint32_t ResolveMarkerTemplatePtr8001C550(
    PsxCompactRailSpriteCommand80024744::Kind kind,
    bool lit) {
    switch (kind) {
    case PsxCompactRailSpriteCommand80024744::Kind::LargeMarker:
        return lit ? kLargeLitTemplate800540AC : kLargeUnlitTemplate8005409C;
    case PsxCompactRailSpriteCommand80024744::Kind::SmallMarker:
        return lit ? kSmallLitTemplate8005408C : kSmallUnlitTemplate8005407C;
    default:
        break;
    }
    return 0u;
}

bool ResolveNoteTemplate80024418(
    int16_t type,
    PsxSpriteTemplate80024744& out) {
    switch (type) {
    case 1:
        out = MakeSpriteTemplate80024744(0x03F8u, 0x01DDu, 16u, 16u, 0x01ECu);
        return true;
    case 2:
        out = MakeSpriteTemplate80024744(0x03F8u, 0x01CDu, 16u, 16u, 0x01E7u);
        return true;
    case 3:
        out = MakeSpriteTemplate80024744(0x03FCu, 0x01CDu, 16u, 16u, 0x01E8u);
        return true;
    case 4:
        out = MakeSpriteTemplate80024744(0x03FCu, 0x01DDu, 16u, 16u, 0x01EDu);
        return true;
    case 5:
    case 6:
        out = MakeSpriteTemplate80024744(0x03F4u, 0x01CDu, 16u, 16u, 0x01E6u);
        return true;
    case 7:
    case 8:
        out = MakeSpriteTemplate80024744(0x03F4u, 0x01DDu, 16u, 16u, 0x01E9u);
        return true;
    default:
        break;
    }
    return false;
}

int16_t PsxRsinStep25680024744(int32_t phase) {
    static constexpr std::array<int16_t, 32> kRsin = {{
        0, 1567, 2896, 3784, 4096, 3784, 2896, 1567,
        0, -1567, -2896, -3784, -4096, -3784, -2896, -1567,
        0, 1567, 2896, 3784, 4096, 3784, 2896, 1567,
        0, -1567, -2896, -3784, -4096, -3784, -2896, -1567,
    }};
    int32_t wrapped = phase % 8192;
    if (wrapped < 0) {
        wrapped += 8192;
    }
    return kRsin[static_cast<std::size_t>(wrapped / 256) & 31u];
}

void PsxResetCompactSharedWordBanks80024308_80024390(
    PsxCompactRailRuntime80024744& runtime) {
    for (std::size_t i = 0; i < runtime.sharedScaleX.size(); ++i) {
        runtime.sharedScaleX[i] = kCompactSharedScaleIdentity80024418;
        runtime.sharedScaleY[i] = kCompactSharedScaleIdentity80024418;
    }
}

void PsxCall80024308_ResetTeacherWobbleBank(
    PsxCompactRailRuntime80024744& runtime) {
    PsxResetCompactSharedWordBanks80024308_80024390(runtime);
    for (std::size_t i = 0; i < runtime.bankA.size(); ++i) {
        PsxCompactWobbleSlot80024744& slot = runtime.bankA[i];
        slot.counter = 0;
        slot.phase = 0;
        slot.linearAcc = kCompactWobbleLinearAccInit80024308_80024390;
        slot.linearVel = kCompactWobbleLinearVelInit80024308_80024390;
    }
    runtime.initialized = true;
}

void PsxCall80024390_ResetStudentWobbleBank(
    PsxCompactRailRuntime80024744& runtime) {
    PsxResetCompactSharedWordBanks80024308_80024390(runtime);
    for (std::size_t i = 0; i < runtime.bankB.size(); ++i) {
        PsxCompactWobbleSlot80024744& slot = runtime.bankB[i];
        slot.counter = 0;
        slot.phase = 0;
        slot.linearAcc = kCompactWobbleLinearAccInit80024308_80024390;
        slot.linearVel = kCompactWobbleLinearVelInit80024308_80024390;
    }
    runtime.initialized = true;
}

void EnsureCompactWobbleBank80024744(
    PsxCompactRailRuntime80024744& runtime) {
    if (!runtime.initialized) {
        PsxCall80024308_ResetTeacherWobbleBank(runtime);
        PsxCall80024390_ResetStudentWobbleBank(runtime);
    }
}

void PsxCall80023F20_80024744(
    PsxCompactRailRuntime80024744& runtime,
    int32_t count) {
    EnsureCompactWobbleBank80024744(runtime);
    if (count <= 0) {
        return;
    }
    const std::size_t limit =
        (std::min)(runtime.bankA.size(), static_cast<std::size_t>(count));
    for (std::size_t i = 0; i < limit; ++i) {
        PsxCompactWobbleSlot80024744& slot = runtime.bankA[i];
        if (slot.counter < kCompactWobbleStepCount80023F20_80024114) {
            if (slot.counter >= kTeacherLinearEnd80023F20) {
                if (slot.counter >= kCompactWobbleSinEnd80023F20_80024114) {
                    runtime.sharedScaleX[i] =
                        kCompactSharedScaleIdentity80024418;
                    runtime.sharedScaleY[i] =
                        kCompactSharedScaleIdentity80024418;
                } else {
                    if (slot.phase >=
                        kCompactWobblePhaseWrap80023F20_80024114) {
                        slot.phase = 0;
                    }
                    runtime.sharedScaleX[i] = PsxRsinStep25680024744(slot.phase);
                    runtime.sharedScaleY[i] =
                        kCompactSharedScaleIdentity80024418;
                    slot.phase += kCompactWobblePhaseStep80023F20_80024114;
                }
            } else {
                runtime.sharedScaleX[i] =
                    static_cast<int16_t>(slot.linearAcc +
                                         kCompactSharedScaleIdentity80024418);
                runtime.sharedScaleY[i] =
                    static_cast<int16_t>(slot.linearAcc +
                                         kCompactSharedScaleIdentity80024418);
                slot.linearAcc += slot.linearVel;
                if (slot.linearAcc >= kCompactSharedScaleIdentity80024418) {
                    slot.linearVel =
                        kCompactWobbleLinearVelFlip80023F20_80024114;
                }
            }
            ++slot.counter;
        }
    }
}

void PsxCall80024114_80024744(
    PsxCompactRailRuntime80024744& runtime,
    int32_t count) {
    EnsureCompactWobbleBank80024744(runtime);
    if (count <= 0) {
        return;
    }
    const std::size_t limit =
        (std::min)(runtime.bankB.size(), static_cast<std::size_t>(count));
    for (std::size_t i = 0; i < limit; ++i) {
        PsxCompactWobbleSlot80024744& slot = runtime.bankB[i];
        if (slot.counter < kCompactWobbleStepCount80023F20_80024114) {
            if (slot.counter >= kStudentLinearEnd80024114) {
                if (slot.counter >= kCompactWobbleSinEnd80023F20_80024114) {
                    runtime.sharedScaleX[i] =
                        kCompactSharedScaleIdentity80024418;
                    runtime.sharedScaleY[i] =
                        kCompactSharedScaleIdentity80024418;
                } else {
                    if (slot.phase >=
                        kCompactWobblePhaseWrap80023F20_80024114) {
                        slot.phase = 0;
                    }
                    runtime.sharedScaleX[i] = PsxRsinStep25680024744(slot.phase);
                    runtime.sharedScaleY[i] =
                        kCompactSharedScaleIdentity80024418;
                    slot.phase += kCompactWobblePhaseStep80023F20_80024114;
                }
            } else {
                slot.linearAcc += slot.linearVel;
                runtime.sharedScaleX[i] =
                    static_cast<int16_t>(slot.linearAcc +
                                         kCompactSharedScaleIdentity80024418);
                runtime.sharedScaleY[i] =
                    static_cast<int16_t>(slot.linearAcc +
                                         kCompactSharedScaleIdentity80024418);
                if (slot.linearAcc >= kCompactSharedScaleIdentity80024418) {
                    slot.linearVel =
                        kCompactWobbleLinearVelFlip80023F20_80024114;
                }
            }
            ++slot.counter;
        }
    }
}

void PushCompactRailSprite80024744(
    std::vector<PsxCompactRailSpriteCommand80024744>& spriteCommands,
    PsxCompactRailSpriteCommand80024744::Kind kind,
    PsxCompactRailSpriteCommand80024744::Anchor anchor,
    const PsxSpriteTemplate80024744& tpl,
    float x,
    float y,
    float scaleX,
    float scaleY,
    uint8_t otBucket,
    uint8_t row,
    uint8_t slot,
    int16_t raw,
    bool lit) {
    PsxCompactRailSpriteCommand80024744 command{};
    command.kind = kind;
    command.anchor = anchor;
    command.tpl = tpl;
    command.x = x;
    command.y = y;
    command.scaleX = scaleX;
    command.scaleY = scaleY;
    command.otBucket = otBucket;
    command.row = row;
    command.slot = slot;
    command.raw = raw;
    command.lit = lit;
    spriteCommands.push_back(command);
}

void CountDebugSprite80024744(
    PsxCompactRailFrameDebug80024744& debug,
    PsxCompactRailSpriteCommand80024744::Kind kind) {
    ++debug.spriteCommandCount;
    switch (kind) {
    case PsxCompactRailSpriteCommand80024744::Kind::TeacherPortrait:
    case PsxCompactRailSpriteCommand80024744::Kind::StudentPortrait:
        ++debug.portraitCommandCount;
        break;
    case PsxCompactRailSpriteCommand80024744::Kind::LargeMarker:
    case PsxCompactRailSpriteCommand80024744::Kind::SmallMarker:
        ++debug.markerCommandCount;
        break;
    case PsxCompactRailSpriteCommand80024744::Kind::Note:
    default:
        ++debug.noteCommandCount;
        break;
    }
}

uint32_t ResolveCallsite8001C550_80024744(
    PsxCompactRailSpriteCommand80024744::Kind kind) {
    switch (kind) {
    case PsxCompactRailSpriteCommand80024744::Kind::TeacherPortrait:
        return 0x80024850u;
    case PsxCompactRailSpriteCommand80024744::Kind::StudentPortrait:
        return 0x80024908u;
    case PsxCompactRailSpriteCommand80024744::Kind::LargeMarker:
        return 0x8002499Cu;
    case PsxCompactRailSpriteCommand80024744::Kind::SmallMarker:
        return 0x80024A28u;
    case PsxCompactRailSpriteCommand80024744::Kind::Note:
    default:
        return 0x80024744u;
    }
}

void PsxCall80024418_80024744(
    PsxCompactRailRuntime80024744& runtime,
    std::vector<PsxCompactRailSpriteCommand80024744>& spriteCommands,
    PsxCompactRailFrameDebug80024744& debug,
    PsxCompactRailRowDebug80024744& rowDebug,
    uint8_t row,
    uint8_t localSlot,
    uint8_t slotOrdinal,
    float centerX,
    float centerY,
    int16_t type,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4* runtime8003F1B4,
    uint8_t workIndex8004019C,
    PsxCompactRailLocalSpriteRgbCarrier80024744 rgbCarrier) {
    PsxSpriteTemplate80024744 tpl{};
    if (!ResolveNoteTemplate80024418(type, tpl)) {
        return;
    }
    const std::size_t scaleIndex = (std::min<std::size_t>)(
        slotOrdinal,
        runtime.sharedScaleX.size() - 1u);
    if (localSlot < rowDebug.bodyNotes.size()) {
        PsxCompactRailBodyNoteDebug80024744& noteDebug =
            rowDebug.bodyNotes[localSlot];
        noteDebug.visited = true;
        noteDebug.drawSubmitted = true;
        noteDebug.type = static_cast<int8_t>(type);
        noteDebug.typeIndex80024418 = static_cast<uint8_t>(type);
        noteDebug.localSlot = localSlot;
        noteDebug.slotOrdinal = slotOrdinal;
        noteDebug.scaleIndex = static_cast<uint8_t>(scaleIndex);
        noteDebug.templateTableBase800540BC = kNoteTemplateTableBase800540BC;
        noteDebug.localSpriteAttr80024418 = kLocalSpriteAttr80024418;
        noteDebug.templatePtr800540BC = ResolveNoteTemplatePtr800540BC(type);
        noteDebug.scaleXWord80087668 = runtime.sharedScaleX[scaleIndex];
        noteDebug.scaleYWord800876B0 = runtime.sharedScaleY[scaleIndex];
        noteDebug.argX80024418 =
            static_cast<int16_t>(static_cast<int32_t>(centerX));
        noteDebug.argY80024418 =
            static_cast<int16_t>(static_cast<int32_t>(centerY));
        noteDebug.argSlot80024418 = slotOrdinal;
        noteDebug.argType80024418 = type;
        noteDebug.centerX = centerX;
        noteDebug.centerY = centerY;
        noteDebug.localSpriteX80024418 =
            static_cast<int16_t>(static_cast<int32_t>(centerX) - 160);
        noteDebug.localSpriteY80024418 =
            static_cast<int16_t>(static_cast<int32_t>(centerY) - 120);
        noteDebug.submit8001C804 = BuildSubmit8001C804Metadata(
            tpl,
            noteDebug.templatePtr800540BC,
            noteDebug.argX80024418,
            noteDebug.argY80024418,
            noteDebug.scaleXWord80087668,
            noteDebug.scaleYWord800876B0,
            runtime8003F1B4,
            workIndex8004019C,
            rgbCarrier);
    }
    PushCompactRailSprite80024744(
        spriteCommands,
        PsxCompactRailSpriteCommand80024744::Kind::Note,
        PsxCompactRailSpriteCommand80024744::Anchor::Center,
        tpl,
        centerX,
        centerY,
        runtime.sharedScaleX[scaleIndex] /
            static_cast<float>(kCompactSharedScaleIdentity80024418),
        runtime.sharedScaleY[scaleIndex] /
            static_cast<float>(kCompactSharedScaleIdentity80024418),
        1u,
        row,
        slotOrdinal,
        type,
        false);
    CountDebugSprite80024744(debug,
                             PsxCompactRailSpriteCommand80024744::Kind::Note);
}

PsxCompactRailSubmit8001C550Debug80024744 PsxCall8001C550_80024744(
    std::vector<PsxCompactRailSpriteCommand80024744>& spriteCommands,
    PsxCompactRailFrameDebug80024744& debug,
    PsxCompactRailSpriteCommand80024744::Kind kind,
    const PsxSpriteTemplate80024744& tpl,
    uint32_t templatePtr,
    float x,
    float y,
    uint8_t otBucket,
    uint8_t row,
    uint8_t slot,
    int16_t raw,
    bool lit,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20*
        runtime8003FA20,
    uint8_t workIndex8004019C,
    uint32_t& fastSpriteCallOrder8003FA20,
    PsxCompactRailLocalSpriteRgbCarrier80024744 rgbCarrier) {
    const uint32_t psxCallOrder = fastSpriteCallOrder8003FA20++;
    const auto submitDebug = BuildSubmit8001C550Metadata(
        tpl,
        templatePtr,
        static_cast<int16_t>(static_cast<int32_t>(x)),
        static_cast<int16_t>(static_cast<int32_t>(y)),
        otBucket,
        ResolveCallsite8001C550_80024744(kind),
        psxCallOrder,
        runtime8003FA20,
        workIndex8004019C,
        rgbCarrier);
    PushCompactRailSprite80024744(
        spriteCommands,
        kind,
        PsxCompactRailSpriteCommand80024744::Anchor::TopLeft,
        tpl,
        x,
        y,
        1.0f,
        1.0f,
        otBucket,
        row,
        slot,
        raw,
        lit);
    CountDebugSprite80024744(debug, kind);
    return submitDebug;
}

void FillMarkerDebug80024744(
    PsxCompactRailMarkerDebug80024744& markerDebug,
    PsxCompactRailSpriteCommand80024744::Kind kind,
    uint8_t ordinal,
    uint8_t thresholdOrdinal,
    int16_t x,
    int16_t y,
    int16_t threshold,
    int16_t raw,
    int32_t teacherProgressX,
    bool lit) {
    markerDebug.visited = true;
    markerDebug.drawSubmitted = true;
    markerDebug.lit = lit;
    markerDebug.ordinal = ordinal;
    markerDebug.thresholdOrdinal = thresholdOrdinal;
    markerDebug.otBucket8001C550 = 3u;
    markerDebug.x = x;
    markerDebug.y = y;
    markerDebug.threshold = threshold;
    markerDebug.raw = raw;
    markerDebug.teacherProgressX = teacherProgressX;
    markerDebug.thresholdTablePtr =
        kind == PsxCompactRailSpriteCommand80024744::Kind::LargeMarker
            ? kLargeThresholdTable800540E8
            : kSmallThresholdTable80054100;
    markerDebug.litTemplatePtr =
        kind == PsxCompactRailSpriteCommand80024744::Kind::LargeMarker
            ? kLargeLitTemplate800540AC
            : kSmallLitTemplate8005408C;
    markerDebug.unlitTemplatePtr =
        kind == PsxCompactRailSpriteCommand80024744::Kind::LargeMarker
            ? kLargeUnlitTemplate8005409C
            : kSmallUnlitTemplate8005407C;
    markerDebug.selectedTemplatePtr =
        ResolveMarkerTemplatePtr8001C550(kind, lit);
}

} // namespace

void PsxCall80024744(
    const PsxCompactRailInput80024744& input,
    PsxCompactRailRuntime80024744& runtime,
    std::vector<PrStageSceneSubmitDirect::PsxCompactRailCommand80024744>&
        commandLog,
    std::vector<PrStageSceneSubmitDirect::PsxCompactRailSpriteCommand80024744>&
        spriteCommands,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20*
        outSubmitRuntime8003FA20,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4*
        outSubmitRuntime8003F1B4) {
    const auto& command = input.command;
    commandLog.push_back(command);

    PsxCompactRailFrameDebug80024744 debug{};
    debug.valid = true;
    debug.callSerial = ++s_frameDebugSerial80024744;
    debug.inputCaptured = input.inputCaptured;
    debug.resetWobbleBanks = command.resetWobbleBanks;
    debug.resetTeacherWobbleBank80024308 =
        command.resetTeacherWobbleBank80024308;
    debug.resetStudentWobbleBank80024390 =
        command.resetStudentWobbleBank80024390;
    debug.painterGate7A = command.painterGate7A;
    debug.rowCount8A = command.rowCount8A;
    debug.maxRows80024744 = kCompactRailMaxRows80024744;
    debug.bodyStreamSlotCount80024744 = kCompactRailBodySlotCount80024744;
    debug.wobbleBankEntryCount80024308_80024390 =
        static_cast<uint8_t>(kCompactWobbleBankEntryCount80024744);
    debug.teacherLinearEnd80023F20 =
        static_cast<uint8_t>(kTeacherLinearEnd80023F20);
    debug.studentLinearEnd80024114 =
        static_cast<uint8_t>(kStudentLinearEnd80024114);
    debug.wobbleSinEnd80023F20_80024114 =
        static_cast<uint8_t>(kCompactWobbleSinEnd80023F20_80024114);
    debug.wobbleStepCount80023F20_80024114 =
        static_cast<uint8_t>(kCompactWobbleStepCount80023F20_80024114);

    if (command.resetWobbleBanks) {
        PsxCall80024308_ResetTeacherWobbleBank(runtime);
        PsxCall80024390_ResetStudentWobbleBank(runtime);
    }
    if (command.resetTeacherWobbleBank80024308) {
        PsxCall80024308_ResetTeacherWobbleBank(runtime);
    }
    if (command.resetStudentWobbleBank80024390) {
        PsxCall80024390_ResetStudentWobbleBank(runtime);
    }
    EnsureCompactWobbleBank80024744(runtime);
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20
        submitRuntime8003FA20Storage{};
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* submitRuntime8003FA20 =
        nullptr;
    if (input.submitRuntime8003FA20Valid) {
        submitRuntime8003FA20Storage = input.submitRuntime8003FA20;
        submitRuntime8003FA20 = &submitRuntime8003FA20Storage;
    }
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4
        submitRuntime8003F1B4Storage{};
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4* submitRuntime8003F1B4 =
        nullptr;
    if (input.submitRuntime8003F1B4Valid) {
        submitRuntime8003F1B4Storage = input.submitRuntime8003F1B4;
        submitRuntime8003F1B4 = &submitRuntime8003F1B4Storage;
    }
    if (command.rowCount8A <= 0) {
        debug.teacherHoldRaw308 = runtime.teacherHoldRaw308;
        debug.teacherHoldCount304 = runtime.teacherHoldCount304;
        debug.studentHoldRaw316 = runtime.studentHoldRaw316;
        debug.studentHoldCount312 = runtime.studentHoldCount312;
        s_lastFrameDebug80024744 = debug;
        if (outSubmitRuntime8003FA20 != nullptr &&
            submitRuntime8003FA20 != nullptr) {
            *outSubmitRuntime8003FA20 = *submitRuntime8003FA20;
        }
        if (outSubmitRuntime8003F1B4 != nullptr &&
            submitRuntime8003F1B4 != nullptr) {
            *outSubmitRuntime8003F1B4 = *submitRuntime8003F1B4;
        }
        return;
    }

    static constexpr std::array<int16_t, 4> kLargeThresholds{{
        56, 116, 176, 236,
    }};
    static constexpr std::array<int16_t, 14> kSmallThresholds{{
        30, 45, 75, 90, 105, 135, 150,
        165, 195, 210, 225, 255, 270, 285,
    }};
    static constexpr std::array<int16_t, 2> kLargeMarkerY{{18, 38}};
    static constexpr std::array<int16_t, 2> kSmallMarkerY{{22, 41}};

    uint32_t fastSpriteCallOrder8003FA20 = 0;
    const int rowCount = (std::min<int>)(
        command.rowCount8A,
        static_cast<int>(command.rows.size()));
    debug.clampedRowCount = static_cast<int16_t>(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        const PsxCompactRailRow80024744& railRow =
            command.rows[static_cast<std::size_t>(row)];
        const int rowOffset18 = row * 18;
        const int rowSlotBase17 = row * 17;
        const int rowYBase20 = row * 20;
        const bool drawEnabled = command.painterGate7A == 1;
        const uint8_t smallMarkerStartIndex =
            static_cast<uint8_t>(row > 0 ? 2u : 0u);
        PsxCompactRailRowDebug80024744& rowDebug =
            debug.rows[static_cast<std::size_t>(row)];
        rowDebug.visited = true;
        rowDebug.drawEnabled = drawEnabled;
        rowDebug.painterGateMatched7A = drawEnabled;
        rowDebug.row = static_cast<uint8_t>(row);
        rowDebug.bodyStreamSlotCount = kCompactRailBodySlotCount80024744;
        rowDebug.smallMarkerStartIndex = smallMarkerStartIndex;
        rowDebug.rowOffset18 = rowOffset18;
        rowDebug.rowSlotBase17 = rowSlotBase17;
        rowDebug.rowYBase20 = rowYBase20;
        rowDebug.teacherRawSourceOffset = 0x8C + 2 * row;
        rowDebug.studentRawSourceOffset = 0x9E + 2 * row;
        rowDebug.bodyStreamPtrSourceOffset = 0x94 + 4 * row;
        rowDebug.teacherRaw8C = railRow.teacherRaw8C;
        rowDebug.studentRaw9E = railRow.studentRaw9E;
        rowDebug.bodyStreamPtr94 = railRow.bodyStreamPtr94;
        rowDebug.largeThresholdTablePtr800540E8 =
            kLargeThresholdTable800540E8;
        rowDebug.smallThresholdTablePtr80054100 =
            kSmallThresholdTable80054100;

        if (railRow.teacherRaw8C >= 0) {
            rowDebug.teacherWobbleCount80023F20 =
                railRow.teacherRaw8C + rowOffset18;
            PsxCall80023F20_80024744(runtime,
                                     railRow.teacherRaw8C + rowOffset18);
            if (drawEnabled) {
                if (runtime.teacherHoldRaw308 == railRow.teacherRaw8C) {
                    ++runtime.teacherHoldCount304;
                } else {
                    runtime.teacherHoldRaw308 = railRow.teacherRaw8C;
                    runtime.teacherHoldCount304 = 0;
                }
                if (runtime.teacherHoldCount304 > 4) {
                    runtime.teacherHoldCount304 = 4;
                }
                rowDebug.teacherPortrait.visited = true;
                rowDebug.teacherPortrait.raw = railRow.teacherRaw8C;
                rowDebug.teacherPortrait.holdRaw = runtime.teacherHoldRaw308;
                rowDebug.teacherPortrait.holdCount =
                    runtime.teacherHoldCount304;
                if (railRow.teacherRaw8C > 0) {
                    const int32_t hold = runtime.teacherHoldCount304;
                    const float x = static_cast<float>(
                        15 * railRow.teacherRaw8C + 26 + 4 * hold);
                    const float y = static_cast<float>(rowYBase20 + 18);
                    rowDebug.teacherPortrait.drawSubmitted = true;
                    rowDebug.teacherPortrait.x = x;
                    rowDebug.teacherPortrait.y = y;
                    rowDebug.teacherPortrait.submit8001C550 =
                        PsxCall8001C550_80024744(
                        spriteCommands,
                        debug,
                        PsxCompactRailSpriteCommand80024744::Kind::
                            TeacherPortrait,
                        MakeSpriteTemplate80024744(
                            0x03F9u, 0x01ADu, 16u, 16u, 0x01F2u),
                        kTeacherOnSiTemplate80053FFC,
                        x,
                        y,
                        0u,
                        static_cast<uint8_t>(row),
                        0u,
                        railRow.teacherRaw8C,
                        false,
                        submitRuntime8003FA20,
                        input.submitRuntimeWorkIndex8004019C,
                        fastSpriteCallOrder8003FA20,
                        input.localFastSpriteRgb8001C550);
                }
            }
        }

        if (railRow.studentRaw9E >= 0) {
            rowDebug.studentWobbleCount80024114 =
                railRow.studentRaw9E + rowOffset18;
            PsxCall80024114_80024744(runtime,
                                     railRow.studentRaw9E + rowOffset18);
            if (drawEnabled) {
                if (runtime.studentHoldRaw316 == railRow.studentRaw9E) {
                    ++runtime.studentHoldCount312;
                } else {
                    runtime.studentHoldRaw316 = railRow.studentRaw9E;
                    runtime.studentHoldCount312 = 1;
                }
                if (runtime.studentHoldCount312 >= 5) {
                    runtime.studentHoldCount312 = 4;
                }
                const int32_t hold = runtime.studentHoldCount312;
                const float x = static_cast<float>(
                    15 * railRow.studentRaw9E + 26 + 4 * hold);
                const float y = static_cast<float>(rowYBase20 + 16);
                rowDebug.studentPortrait.visited = true;
                rowDebug.studentPortrait.drawSubmitted = true;
                rowDebug.studentPortrait.raw = railRow.studentRaw9E;
                rowDebug.studentPortrait.holdRaw = runtime.studentHoldRaw316;
                rowDebug.studentPortrait.holdCount =
                    runtime.studentHoldCount312;
                rowDebug.studentPortrait.x = x;
                rowDebug.studentPortrait.y = y;
                rowDebug.studentPortrait.submit8001C550 =
                    PsxCall8001C550_80024744(
                    spriteCommands,
                    debug,
                    PsxCompactRailSpriteCommand80024744::Kind::StudentPortrait,
                    MakeSpriteTemplate80024744(
                        0x03F5u, 0x01BDu, 16u, 16u, 0x01F3u),
                    kStudentPaSiTemplate8005400C,
                    x,
                    y,
                    0u,
                    static_cast<uint8_t>(row),
                    0u,
                    railRow.studentRaw9E,
                    false,
                    submitRuntime8003FA20,
                    input.submitRuntimeWorkIndex8004019C,
                    fastSpriteCallOrder8003FA20,
                    input.localFastSpriteRgb8001C550);
            }
        }

        if (drawEnabled) {
            const int16_t raw = railRow.teacherRaw8C;
            const int32_t teacherProgressX =
                raw > 0 ? static_cast<int32_t>(15 * raw + 31) : 0;
            rowDebug.teacherProgressX80024744 = teacherProgressX;
            for (std::size_t i = 0; i < kLargeThresholds.size(); ++i) {
                const bool lit = teacherProgressX >= kLargeThresholds[i];
                if (lit) {
                    ++rowDebug.largeMarkerLitCount;
                }
                FillMarkerDebug80024744(
                    rowDebug.largeMarkers[i],
                    PsxCompactRailSpriteCommand80024744::Kind::LargeMarker,
                    static_cast<uint8_t>(i),
                    static_cast<uint8_t>(i),
                    kLargeThresholds[i],
                    kLargeMarkerY[static_cast<std::size_t>(row)],
                    kLargeThresholds[i],
                    raw,
                    teacherProgressX,
                    lit);
                rowDebug.largeMarkers[i].submit8001C550 =
                    PsxCall8001C550_80024744(
                    spriteCommands,
                    debug,
                    PsxCompactRailSpriteCommand80024744::Kind::LargeMarker,
                    lit ? MakeSpriteTemplate80024744(
                              0x03FAu, 0x0181u, 12u, 12u, 0x01E3u)
                        : MakeSpriteTemplate80024744(
                              0x03FAu, 0x0181u, 12u, 12u, 0x01E2u),
                    rowDebug.largeMarkers[i].selectedTemplatePtr,
                    static_cast<float>(kLargeThresholds[i]),
                    static_cast<float>(
                        kLargeMarkerY[static_cast<std::size_t>(row)]),
                    3u,
                    static_cast<uint8_t>(row),
                    static_cast<uint8_t>(i),
                    raw,
                    lit,
                    submitRuntime8003FA20,
                    input.submitRuntimeWorkIndex8004019C,
                    fastSpriteCallOrder8003FA20,
                    input.localFastSpriteRgb8001C550);
            }

            for (std::size_t i = 0; i < kSmallThresholds.size(); ++i) {
                if (i < smallMarkerStartIndex) {
                    rowDebug.smallMarkers[i].visited = true;
                    rowDebug.smallMarkers[i].skippedByRowGate = true;
                    rowDebug.smallMarkers[i].ordinal = static_cast<uint8_t>(i);
                    rowDebug.smallMarkers[i].thresholdOrdinal =
                        static_cast<uint8_t>(i);
                    rowDebug.smallMarkers[i].x = kSmallThresholds[i];
                    rowDebug.smallMarkers[i].y =
                        kSmallMarkerY[static_cast<std::size_t>(row)];
                    rowDebug.smallMarkers[i].threshold = kSmallThresholds[i];
                    rowDebug.smallMarkers[i].raw = raw;
                    rowDebug.smallMarkers[i].teacherProgressX =
                        teacherProgressX;
                    rowDebug.smallMarkers[i].thresholdTablePtr =
                        kSmallThresholdTable80054100;
                    rowDebug.smallMarkers[i].litTemplatePtr =
                        kSmallLitTemplate8005408C;
                    rowDebug.smallMarkers[i].unlitTemplatePtr =
                        kSmallUnlitTemplate8005407C;
                    continue;
                }
                const bool lit = teacherProgressX >= kSmallThresholds[i];
                if (lit) {
                    ++rowDebug.smallMarkerLitCount;
                }
                FillMarkerDebug80024744(
                    rowDebug.smallMarkers[i],
                    PsxCompactRailSpriteCommand80024744::Kind::SmallMarker,
                    static_cast<uint8_t>(i),
                    static_cast<uint8_t>(i),
                    kSmallThresholds[i],
                    kSmallMarkerY[static_cast<std::size_t>(row)],
                    kSmallThresholds[i],
                    raw,
                    teacherProgressX,
                    lit);
                rowDebug.smallMarkers[i].submit8001C550 =
                    PsxCall8001C550_80024744(
                    spriteCommands,
                    debug,
                    PsxCompactRailSpriteCommand80024744::Kind::SmallMarker,
                    lit ? MakeSpriteTemplate80024744(
                              0x03FDu, 0x0181u, 8u, 8u, 0x01E1u)
                        : MakeSpriteTemplate80024744(
                              0x03FDu, 0x0181u, 8u, 8u, 0x01E0u),
                    rowDebug.smallMarkers[i].selectedTemplatePtr,
                    static_cast<float>(kSmallThresholds[i]),
                    static_cast<float>(
                        kSmallMarkerY[static_cast<std::size_t>(row)]),
                    3u,
                    static_cast<uint8_t>(row),
                    static_cast<uint8_t>(i),
                    raw,
                    lit,
                    submitRuntime8003FA20,
                    input.submitRuntimeWorkIndex8004019C,
                    fastSpriteCallOrder8003FA20,
                    input.localFastSpriteRgb8001C550);
            }

            if (submitRuntime8003FA20 != nullptr &&
                submitRuntime8003F1B4 != nullptr) {
                submitRuntime8003F1B4->dword_800901C8 =
                    submitRuntime8003FA20->dword_800901C8;
            }
            for (std::size_t slot = 0; slot < railRow.bodyStreamBytes94.size();
                 ++slot) {
                const int16_t type = railRow.bodyStreamBytes94[slot];
                if (type >= 1 && type <= 8) {
                    ++rowDebug.bodyNoteCount;
                    PsxCall80024418_80024744(
                        runtime,
                        spriteCommands,
                        debug,
                        rowDebug,
                        static_cast<uint8_t>(row),
                        static_cast<uint8_t>(slot),
                        static_cast<uint8_t>(rowSlotBase17 +
                                             static_cast<int>(slot)),
                        static_cast<float>(32 + 15 * static_cast<int>(slot)),
                        static_cast<float>(rowYBase20 + 24),
                        type,
                        submitRuntime8003F1B4,
                        input.submitRuntimeWorkIndex8004019C,
                        input.localGsSpriteRgb8001C804);
                }
            }
            if (submitRuntime8003FA20 != nullptr &&
                submitRuntime8003F1B4 != nullptr) {
                submitRuntime8003FA20->dword_800901C8 =
                    submitRuntime8003F1B4->dword_800901C8;
            }
        }
    }
    debug.teacherHoldRaw308 = runtime.teacherHoldRaw308;
    debug.teacherHoldCount304 = runtime.teacherHoldCount304;
    debug.studentHoldRaw316 = runtime.studentHoldRaw316;
    debug.studentHoldCount312 = runtime.studentHoldCount312;
    s_lastFrameDebug80024744 = debug;
    if (outSubmitRuntime8003FA20 != nullptr &&
        submitRuntime8003FA20 != nullptr) {
        *outSubmitRuntime8003FA20 = *submitRuntime8003FA20;
    }
    if (outSubmitRuntime8003F1B4 != nullptr &&
        submitRuntime8003F1B4 != nullptr) {
        *outSubmitRuntime8003F1B4 = *submitRuntime8003F1B4;
    }
}

const PsxCompactRailFrameDebug80024744& GetLastFrameDebug80024744() {
    return s_lastFrameDebug80024744;
}

} // namespace PrStage1CompactRail80024744Direct
