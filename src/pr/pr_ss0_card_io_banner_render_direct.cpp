#include "pr_ss0_card_io_banner_render_direct.h"

#include "pr_stage1_movie_text_direct.h"

#include <cstddef>
#include <cstdint>

namespace PrSS0CardIoBannerRenderDirect {
namespace {

constexpr const char* kRemoveCardText8005326C[5] = {
    "Don't remove memory card.",
    "Memory Card nicht entfernen.",
    "Ne pas enlever la carte m\xE9moire.",
    "Non rimuovere la scheda di memoria.",
    "No extraigas la tarjeta de memoria.",
};

constexpr const char* kPleaseWaitText80053280[5] = {
    "Please wait a minute.",
    "Bitte eine Minute warten.",
    "Attends un moment, s'il te pla\xEEt.",
    "Aspetta un attimo.",
    "Espera un momento",
};

constexpr uint16_t kRemoveCardWidth80020A3C[5] = {
    173u, 196u, 213u, 238u, 237u,
};

constexpr uint16_t kPleaseWaitWidth80020A3C[5] = {
    142u, 172u, 217u, 125u, 119u,
};

constexpr const char kNowSavingText800111CC[] = "Now saving.";
constexpr uint16_t kNowSavingWidth80020A3C = 78u;

struct CardIoBannerCornerTemplate8001B25C {
    uint32_t address = 0;
    uint32_t attr = 0;
    uint16_t texX = 0;
    uint16_t texY = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;
};

constexpr CardIoBannerCornerTemplate8001B25C kCardIoBannerCorners[] = {
    {0x80050900u, 0x40000040u, 0x03FDu, 0x0191u, 0x0008u, 0x0008u, 0x0120u, 0x01EEu},
    {0x800508F0u, 0x40000040u, 0x03FDu, 0x0189u, 0x0008u, 0x0008u, 0x0120u, 0x01EEu},
    {0x800508E0u, 0x40000040u, 0x03FDu, 0x01A1u, 0x0008u, 0x0008u, 0x0120u, 0x01EEu},
    {0x800508D0u, 0x40000040u, 0x03FDu, 0x0199u, 0x0008u, 0x0008u, 0x0120u, 0x01EEu},
};

uint16_t ResolveTemplateTpage8001B25C(uint16_t texX, uint16_t texY) {
    const uint16_t uvWord = static_cast<uint16_t>(4u * texX);
    const uint16_t helperA3 = static_cast<uint16_t>((uvWord & 0xFF00u) >> 2u);
    const uint16_t helperA4 = static_cast<uint16_t>(texY & 0xFF00u);
    return static_cast<uint16_t>(
        0x20u |
        ((helperA4 & 0x0100u) >> 4u) |
        ((helperA3 & 0x03FFu) >> 6u) |
        (4u * (helperA4 & 0x0200u)));
}

bool AppendSprite80020A3C(
    CardIoBannerDrawList80020A3C& out,
    const CardIoBannerSpriteCommand80020A3C& sprite) {
    if (!sprite.known || !sprite.rawTexture || sprite.width == 0u ||
        sprite.height == 0u ||
        out.count >= kCardIoBannerCommandCapacity80020A3C) {
        out.truncated = out.count >= kCardIoBannerCommandCapacity80020A3C;
        return false;
    }
    CardIoBannerCommand80020A3C& command = out.commands[out.count];
    command = {};
    command.known = true;
    command.kind = CardIoBannerCommandKind80020A3C::Sprite;
    command.callOrder = out.count;
    command.sprite = sprite;
    ++out.count;
    out.rawTextureSpritesOnly = out.rawTextureSpritesOnly && sprite.rawTexture;
    return true;
}

bool AppendSolidRect80020A3C(
    CardIoBannerDrawList80020A3C& out,
    int16_t x,
    int16_t y,
    uint16_t width,
    uint16_t height) {
    if (width == 0u || height == 0u ||
        out.count >= kCardIoBannerCommandCapacity80020A3C) {
        out.truncated = out.count >= kCardIoBannerCommandCapacity80020A3C;
        return false;
    }
    CardIoBannerCommand80020A3C& command = out.commands[out.count];
    command = {};
    command.known = true;
    command.kind = CardIoBannerCommandKind80020A3C::SolidRect;
    command.callOrder = out.count;
    command.solidRect.known = true;
    command.solidRect.x = x;
    command.solidRect.y = y;
    command.solidRect.width = width;
    command.solidRect.height = height;
    command.solidRect.priority = 0u;
    command.solidRect.attr = 0x400F0F0Fu;
    command.solidRect.r = 0x0Fu;
    command.solidRect.g = 0x0Fu;
    command.solidRect.b = 0x0Fu;
    ++out.count;
    ++out.solidRectCount;
    return true;
}

PrStage1MovieTextDirect::Movie1TextCurrentGp872WorkCarrier
BuildStaticTextWorkCarrier80020A3C() {
    PrStage1MovieTextDirect::Movie1TextCurrentGp872WorkCarrier out{};
    out.usesCurrentGp872DrawBuffer = true;
    out.workBasePsxAddr =
        PrStage1MovieTextDirect::kMovie1TextOtBufferBasePsxAddr;
    out.workStrideBytes =
        PrStage1MovieTextDirect::kMovie1TextOtBufferStrideBytes;
    out.gp872SlotKnown = true;
    out.gp872Slot = 0u;
    out.workAddrKnown = true;
    out.workAddr = out.workBasePsxAddr;
    out.workLastAddrOffset = 0x10u;
    out.drawOtagAddrKnown = true;
    out.drawOtagAddr = out.workAddr + out.workLastAddrOffset;
    return out;
}

PrStage1MovieTextDirect::Movie1TextDrawCommand
BuildTextCommand80020A3C(const char* text) {
    PrStage1MovieTextDirect::Movie1TextDrawCommand command{};
    command.kind =
        PrStage1MovieTextDirect::Movie1TextDrawCommandKind::
            SubmitTextFastSpriteSequenceSub8001B954;
    command.psxFunctionAddr =
        PrStage1MovieTextDirect::kMovie1TextGlyphLoopFunctionSub8001B954;
    command.fastSpriteSourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
            SS0Scene0CardIoBanner;
    command.usesCurrentGp872DrawBuffer = true;
    command.otBufferBasePsxAddr =
        PrStage1MovieTextDirect::kMovie1TextOtBufferBasePsxAddr;
    command.otBufferStrideBytes =
        PrStage1MovieTextDirect::kMovie1TextOtBufferStrideBytes;
    command.x = 30;
    command.y = 121;
    command.z = 0;
    command.scale = 480;
    command.textPtr = text;
    command.centeredLineWidth =
        PrStage1MovieTextDirect::kMovie1TextGlyphCenteredBodyWidthSub8001B954;
    return command;
}

CardIoBannerSpriteCommand80020A3C BuildGlyphSprite80020A3C(
    const PrStage1MovieTextDirect::Movie1TextGlyphSubmitResultSub8001B954&
        submit,
    uint8_t charCode) {
    CardIoBannerSpriteCommand80020A3C out{};
    const auto& local = submit.localFastSprite;
    out.known = submit.valid && submit.wouldCall8003FA20 &&
                submit.localFastSpriteStaticFieldsKnown;
    out.rawTexture = (local.attr_00 & 0x40u) != 0u;
    out.charCode = charCode;
    out.x = static_cast<int16_t>(static_cast<int32_t>(local.x_04) + 160);
    out.y = static_cast<int16_t>(
        static_cast<int32_t>(static_cast<int16_t>(local.y_06)) + 120);
    out.width = local.width_08;
    out.height = local.height_0A;
    out.priority = 0u;
    out.attr = local.attr_00;
    out.tpage = local.tpage_0C;
    out.u = local.u_0E;
    out.v = local.v_0F;
    out.clutX = static_cast<uint16_t>(local.clutX_10);
    out.clutY = static_cast<uint16_t>(local.clutY_12);
    return out;
}

CardIoBannerSpriteCommand80020A3C BuildCornerSprite80020A3C(
    const CardIoBannerCornerTemplate8001B25C& corner,
    int16_t x,
    int16_t y) {
    CardIoBannerSpriteCommand80020A3C out{};
    out.known = true;
    out.rawTexture = (corner.attr & 0x40u) != 0u;
    out.sourceAddress = corner.address;
    out.x = x;
    out.y = y;
    out.width = corner.width;
    out.height = corner.height;
    out.priority = 0u;
    out.attr = corner.attr;
    out.tpage = ResolveTemplateTpage8001B25C(corner.texX, corner.texY);
    out.u = static_cast<uint8_t>(4u * corner.texX);
    out.v = static_cast<uint8_t>(corner.texY);
    out.clutX = corner.clutX;
    out.clutY = corner.clutY;
    return out;
}

const char* ResolveCardIoBannerText80020A3C(
    int32_t messageType,
    int32_t language) {
    if (messageType == kMainDirectoryCardIoMessageType80020A3C) {
        return kPleaseWaitText80053280[language];
    }
    if (messageType == kSavingMessageType80020A3C && language == 0) {
        return kNowSavingText800111CC;
    }
    return kRemoveCardText8005326C[language];
}

uint16_t ResolveCardIoBannerWidth80020A3C(
    int32_t messageType,
    int32_t language) {
    if (messageType == kMainDirectoryCardIoMessageType80020A3C) {
        return kPleaseWaitWidth80020A3C[language];
    }
    if (messageType == kSavingMessageType80020A3C && language == 0) {
        return kNowSavingWidth80020A3C;
    }
    return kRemoveCardWidth80020A3C[language];
}

}  // namespace

CardIoBannerDrawList80020A3C
BuildCardIoBannerDrawList80020A3C(int32_t messageType, int32_t language) {
    CardIoBannerDrawList80020A3C out{};
    out.rawTextureSpritesOnly = true;
    out.messageType = messageType;
    out.language = language;
    // 80020A3C compares the full argument only with 2 and 1. Every other
    // value uses the original remove-card text branch.
    if (language < 0 || language >= 5) {
        return out;
    }

    out.sourceKnown = true;
    out.text = ResolveCardIoBannerText80020A3C(messageType, language);
    const uint16_t expectedTextWidth =
        ResolveCardIoBannerWidth80020A3C(messageType, language);
    const auto sequence =
        PrStage1MovieTextDirect::BuildTextFastSpriteSequenceSub8001B954(
            BuildTextCommand80020A3C(out.text),
            PrStage1MovieTextDirect::Movie1TextGlyphMetricTablesSub8001B954{},
            BuildStaticTextWorkCarrier80020A3C());
    if (!sequence.valid || !sequence.glyphMetricTablesKnown ||
        sequence.lineCapacityExceeded || sequence.glyphCommandCapacityExceeded ||
        sequence.lineCount != 1u || !sequence.lines[0].widthKnown ||
        sequence.lines[0].widthPx != expectedTextWidth ||
        sequence.glyphSubmitResultCount == 0u) {
        return out;
    }

    out.textWidth = sequence.lines[0].widthPx;
    out.barWidth = static_cast<uint16_t>(out.textWidth + 8u);
    out.left = static_cast<int16_t>(
        (320 - static_cast<int32_t>(out.barWidth)) / 2);
    out.right = static_cast<int16_t>(out.left + out.barWidth);

    bool ok = true;
    for (std::size_t i = 0; i < sequence.glyphSubmitResultCount; ++i) {
        const auto& submit = sequence.glyphSubmitResults[i];
        if (submit.glyphCommandIndex >= sequence.glyphCommandCount) {
            ok = false;
            break;
        }
        const uint8_t charCode =
            sequence.glyphCommands[submit.glyphCommandIndex].charCode;
        const bool appended = AppendSprite80020A3C(
            out, BuildGlyphSprite80020A3C(submit, charCode));
        ok = appended && ok;
        if (appended) {
            ++out.glyphSpriteCount;
        }
    }

    ok = AppendSolidRect80020A3C(out, out.left, 123, 8u, 9u) && ok;
    ok = AppendSolidRect80020A3C(
             out, static_cast<int16_t>(out.left + 8), 115,
             out.textWidth, 25u) && ok;
    ok = AppendSolidRect80020A3C(out, out.right, 123, 8u, 9u) && ok;

    constexpr int16_t kCornerYOffset[4] = {115, 132, 115, 132};
    for (std::size_t i = 0; i < 4u; ++i) {
        const int16_t x = i < 2u ? out.left : out.right;
        const bool appended = AppendSprite80020A3C(
            out,
            BuildCornerSprite80020A3C(
                kCardIoBannerCorners[i], x, kCornerYOffset[i]));
        ok = appended && ok;
        if (appended) {
            ++out.cornerSpriteCount;
        }
    }

    out.accepted = ok && !out.truncated && out.rawTextureSpritesOnly &&
                   out.glyphSpriteCount == sequence.glyphSubmitResultCount &&
                   out.solidRectCount == 3u && out.cornerSpriteCount == 4u &&
                   out.count == out.glyphSpriteCount + 7u;
    out.complete = out.sourceKnown && out.accepted;
    return out;
}

CardIoBannerDrawList80020A3C
BuildMainDirectoryCardIoBannerDrawList80020A3C(int32_t language) {
    return BuildCardIoBannerDrawList80020A3C(
        kMainDirectoryCardIoMessageType80020A3C, language);
}

}  // namespace PrSS0CardIoBannerRenderDirect
