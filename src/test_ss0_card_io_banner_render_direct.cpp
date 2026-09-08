#include "pr/pr_ss0_card_io_banner_render_direct.h"
#include "pr/pr_stage1_movie_text_direct.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

namespace PrStage1MovieTextDirect {
namespace {

struct StubTextWidth {
    const char* text;
    uint16_t width;
};

constexpr StubTextWidth kStubTextWidths[] = {
    {"Don't remove memory card.", 173u},
    {"Memory Card nicht entfernen.", 196u},
    {"Ne pas enlever la carte m\xE9moire.", 213u},
    {"Non rimuovere la scheda di memoria.", 238u},
    {"No extraigas la tarjeta de memoria.", 237u},
    {"Please wait a minute.", 142u},
    {"Bitte eine Minute warten.", 172u},
    {"Attends un moment, s'il te pla\xEEt.", 217u},
    {"Aspetta un attimo.", 125u},
    {"Espera un momento", 119u},
    {"Now saving.", 78u},
};

uint16_t StubTextWidthFor80020A3C(const char* text) {
    if (text == nullptr) {
        return 0u;
    }
    for (const StubTextWidth& entry : kStubTextWidths) {
        if (std::strcmp(text, entry.text) == 0) {
            return entry.width;
        }
    }
    return 0u;
}

}  // namespace

Movie1TextFastSpriteSequenceSub8001B954 BuildTextFastSpriteSequenceSub8001B954(
    const Movie1TextDrawCommand& command,
    const Movie1TextGlyphMetricTablesSub8001B954&,
    const Movie1TextCurrentGp872WorkCarrier&) {
    Movie1TextFastSpriteSequenceSub8001B954 out{};
    const uint16_t width = StubTextWidthFor80020A3C(command.textPtr);
    if (width == 0u) {
        return out;
    }

    out.valid = true;
    out.glyphMetricTablesKnown = true;
    out.lineCount = 1u;
    out.lines[0].valid = true;
    out.lines[0].widthKnown = true;
    out.lines[0].widthPx = width;
    out.glyphCommandCount = 1u;
    out.glyphCommands[0].valid = true;
    out.glyphCommands[0].charCode = static_cast<uint8_t>('A');
    out.glyphSubmitResultCount = 1u;
    auto& submit = out.glyphSubmitResults[0];
    submit.valid = true;
    submit.glyphCommandIndex = 0u;
    submit.wouldCall8003FA20 = true;
    submit.localFastSpriteStaticFieldsKnown = true;
    submit.localFastSprite.attr_00 = 0x50000040u;
    submit.localFastSprite.x_04 = -4;
    submit.localFastSprite.y_06 = 1u;
    submit.localFastSprite.width_08 = 8u;
    submit.localFastSprite.height_0A = 9u;
    submit.localFastSprite.tpage_0C = 0x20u;
    submit.localFastSprite.clutX_10 = 256;
    submit.localFastSprite.clutY_12 = 247;
    return out;
}

}  // namespace PrStage1MovieTextDirect

namespace {

using namespace PrSS0CardIoBannerRenderDirect;

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__, \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

void CheckAccepted(const CardIoBannerDrawList80020A3C& list) {
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureSpritesOnly);
    CHECK(!list.truncated);
    CHECK(list.glyphSpriteCount == 1u);
    CHECK(list.solidRectCount == 3u);
    CHECK(list.cornerSpriteCount == 4u);
    CHECK(list.count == 8u);
}

void CheckSameRender(const CardIoBannerDrawList80020A3C& expected,
                     const CardIoBannerDrawList80020A3C& actual) {
    CheckAccepted(expected);
    CheckAccepted(actual);
    CHECK(actual.text == expected.text);
    CHECK(actual.textWidth == expected.textWidth);
    CHECK(actual.barWidth == expected.barWidth);
    CHECK(actual.left == expected.left);
    CHECK(actual.right == expected.right);
    CHECK(actual.count == expected.count);
    for (uint32_t index = 0u; index < expected.count; ++index) {
        const auto& lhs = expected.commands[index];
        const auto& rhs = actual.commands[index];
        CHECK(rhs.known == lhs.known);
        CHECK(rhs.kind == lhs.kind);
        CHECK(rhs.callOrder == lhs.callOrder);
        if (lhs.kind == CardIoBannerCommandKind80020A3C::Sprite) {
            CHECK(rhs.sprite.sourceAddress == lhs.sprite.sourceAddress);
            CHECK(rhs.sprite.charCode == lhs.sprite.charCode);
            CHECK(rhs.sprite.x == lhs.sprite.x);
            CHECK(rhs.sprite.y == lhs.sprite.y);
            CHECK(rhs.sprite.width == lhs.sprite.width);
            CHECK(rhs.sprite.height == lhs.sprite.height);
        } else {
            CHECK(rhs.solidRect.x == lhs.solidRect.x);
            CHECK(rhs.solidRect.y == lhs.solidRect.y);
            CHECK(rhs.solidRect.width == lhs.solidRect.width);
            CHECK(rhs.solidRect.height == lhs.solidRect.height);
        }
    }
}

void TestCanonicalMessageTypes() {
    constexpr const char* kRemoveText[] = {
        "Don't remove memory card.",
        "Memory Card nicht entfernen.",
        "Ne pas enlever la carte m\xE9moire.",
        "Non rimuovere la scheda di memoria.",
        "No extraigas la tarjeta de memoria.",
    };
    constexpr const char* kWaitText[] = {
        "Please wait a minute.",
        "Bitte eine Minute warten.",
        "Attends un moment, s'il te pla\xEEt.",
        "Aspetta un attimo.",
        "Espera un momento",
    };
    for (int32_t language = 0; language < 5; ++language) {
        const auto removeCard = BuildCardIoBannerDrawList80020A3C(
            kRemoveCardMessageType80020A3C, language);
        const auto wait = BuildCardIoBannerDrawList80020A3C(
            kMainDirectoryCardIoMessageType80020A3C, language);
        const auto saving = BuildCardIoBannerDrawList80020A3C(
            kSavingMessageType80020A3C, language);
        CheckAccepted(removeCard);
        CheckAccepted(wait);
        CheckAccepted(saving);
        CHECK(std::strcmp(removeCard.text, kRemoveText[language]) == 0);
        CHECK(std::strcmp(wait.text, kWaitText[language]) == 0);
        if (language == 0) {
            CHECK(std::strcmp(saving.text, "Now saving.") == 0);
        } else {
            CheckSameRender(removeCard, saving);
        }
    }
}

void TestNoncanonicalMessageTypesUseRemoveCardDefault() {
    constexpr std::array<int32_t, 4u> kDefaultMessageTypes = {
        std::numeric_limits<int32_t>::min(),
        -1,
        3,
        std::numeric_limits<int32_t>::max(),
    };
    for (int32_t language = 0; language < 5; ++language) {
        const auto removeCard = BuildCardIoBannerDrawList80020A3C(
            kRemoveCardMessageType80020A3C, language);
        for (const int32_t messageType : kDefaultMessageTypes) {
            const auto candidate =
                BuildCardIoBannerDrawList80020A3C(messageType, language);
            CheckSameRender(removeCard, candidate);
        }
    }
}

void TestInvalidLanguageFailsClosed() {
    constexpr std::array<int32_t, 4u> kInvalidLanguages = {
        std::numeric_limits<int32_t>::min(),
        -1,
        5,
        std::numeric_limits<int32_t>::max(),
    };
    for (const int32_t language : kInvalidLanguages) {
        const auto list = BuildCardIoBannerDrawList80020A3C(3, language);
        CHECK(!list.sourceKnown);
        CHECK(!list.accepted);
        CHECK(!list.complete);
        CHECK(list.count == 0u);
    }
}

}  // namespace

int main() {
    TestCanonicalMessageTypes();
    TestNoncanonicalMessageTypesUseRemoveCardDefault();
    TestInvalidLanguageFailsClosed();
    if (g_failedChecks != 0) {
        std::printf(
            "test_ss0_card_io_banner_render_direct: failed checks=%d\n",
            g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_card_io_banner_render_direct: ok\n");
    return 0;
}
