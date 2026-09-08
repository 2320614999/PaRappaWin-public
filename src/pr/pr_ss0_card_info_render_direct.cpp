#include "pr_ss0_card_info_render_direct.h"

#include "pr_psx_text_glyph_metrics_direct.h"

#include <limits>

namespace PrSS0CardInfoRenderDirect {
namespace {

struct PositionedTemplate80020BE4 {
    uint32_t address;
    int16_t x;
    int16_t y;
};

struct LanguageTemplateSet80020BE4 {
    std::array<uint32_t, 3u> variants;
    int16_t x;
    int16_t y;
};

struct BuildFailure80020BE4 {
    bool preview = false;
    bool descriptor = false;
    bool rawTexture = false;
    bool callOrder = false;
    bool capacity = false;
    bool truncated = false;
};

constexpr uint32_t kTemplateBankBase80051C00 = 0x80051C00u;
constexpr std::size_t kTemplateBankDescriptorCount80051C00 = 100u;

// SCUS_941.83 80051C00..80052230, one 8001B25C descriptor per 0x10 bytes.
constexpr std::array<CardInfoSpriteDescriptor8001B25C,
                     kTemplateBankDescriptorCount80051C00>
    kTemplateDescriptors80051C00 = {{
        {true, 0x50000040u, 0x0200u, 0x00A8u, 0x00C8u, 0x0044u, 0x03E0u, 0x0003u},
        {true, 0x50000040u, 0x0180u, 0x01A8u, 0x0088u, 0x000Du, 0x03E0u, 0x0002u},
        {true, 0x50000040u, 0x0180u, 0x01E4u, 0x0080u, 0x000Bu, 0x03E0u, 0x0002u},
        {true, 0x50000040u, 0x0180u, 0x01B5u, 0x0088u, 0x000Du, 0x03E0u, 0x0002u},
        {true, 0x50000040u, 0x0180u, 0x01CCu, 0x0080u, 0x000Cu, 0x03E0u, 0x0002u},
        {true, 0x50000040u, 0x0180u, 0x01D8u, 0x0088u, 0x000Cu, 0x03E0u, 0x0002u},
        {true, 0x50000040u, 0x0200u, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0206u, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x020Cu, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0212u, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0218u, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x021Eu, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0224u, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x022Au, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0230u, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0236u, 0x0000u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03C0u, 0x0096u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0200u, 0x001Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0218u, 0x001Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03C6u, 0x0096u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x021Eu, 0x001Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0224u, 0x001Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03CCu, 0x0096u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0230u, 0x001Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03D2u, 0x0096u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03D8u, 0x0096u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0200u, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0206u, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x020Cu, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03DEu, 0x0096u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0212u, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0218u, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03E4u, 0x0096u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x021Eu, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0224u, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x022Au, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0230u, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0236u, 0x0038u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0200u, 0x0054u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0206u, 0x0054u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03C0u, 0x00B2u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x020Cu, 0x0054u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03C6u, 0x00B2u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0212u, 0x0054u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x021Eu, 0x0054u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x022Au, 0x0054u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03CCu, 0x00B2u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03D2u, 0x00B2u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03D8u, 0x00B2u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0230u, 0x0054u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x03DEu, 0x00B2u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0236u, 0x0054u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0200u, 0x0070u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0206u, 0x0070u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x020Cu, 0x0070u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0212u, 0x0070u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x021Eu, 0x0070u, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0200u, 0x008Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0206u, 0x008Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x020Cu, 0x008Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0218u, 0x008Cu, 0x001Cu, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0212u, 0x008Cu, 0x0018u, 0x001Cu, 0x03E0u, 0x0000u},
        {true, 0x50000040u, 0x0232u, 0x00A8u, 0x0034u, 0x0011u, 0x03E0u, 0x0004u},
        {true, 0x50000040u, 0x0232u, 0x00A8u, 0x0034u, 0x0011u, 0x03E0u, 0x0005u},
        {true, 0x50000040u, 0x0232u, 0x00A8u, 0x0034u, 0x0011u, 0x03E0u, 0x0006u},
        {true, 0x50000040u, 0x0180u, 0x0190u, 0x0020u, 0x000Du, 0x03E0u, 0x0007u},
        {true, 0x50000040u, 0x0180u, 0x0190u, 0x0020u, 0x000Du, 0x03E0u, 0x0008u},
        {true, 0x50000040u, 0x0180u, 0x0190u, 0x0020u, 0x000Du, 0x03E0u, 0x0009u},
        {true, 0x50000040u, 0x018Eu, 0x0190u, 0x0020u, 0x000Bu, 0x03E0u, 0x0007u},
        {true, 0x50000040u, 0x018Eu, 0x0190u, 0x0020u, 0x000Bu, 0x03E0u, 0x0008u},
        {true, 0x50000040u, 0x018Eu, 0x0190u, 0x0020u, 0x000Bu, 0x03E0u, 0x0009u},
        {true, 0x50000040u, 0x0188u, 0x0190u, 0x0018u, 0x000Bu, 0x03E0u, 0x0007u},
        {true, 0x50000040u, 0x0188u, 0x0190u, 0x0018u, 0x000Bu, 0x03E0u, 0x0008u},
        {true, 0x50000040u, 0x0188u, 0x0190u, 0x0018u, 0x000Bu, 0x03E0u, 0x0009u},
        {true, 0x50000040u, 0x0196u, 0x0190u, 0x0020u, 0x000Au, 0x03E0u, 0x0007u},
        {true, 0x50000040u, 0x0196u, 0x0190u, 0x0020u, 0x000Au, 0x03E0u, 0x0008u},
        {true, 0x50000040u, 0x0196u, 0x0190u, 0x0020u, 0x000Au, 0x03E0u, 0x0009u},
        {true, 0x50000040u, 0x019Eu, 0x0190u, 0x0018u, 0x000Cu, 0x03E0u, 0x0007u},
        {true, 0x50000040u, 0x019Eu, 0x0190u, 0x0018u, 0x000Cu, 0x03E0u, 0x0008u},
        {true, 0x50000040u, 0x019Eu, 0x0190u, 0x0018u, 0x000Cu, 0x03E0u, 0x0009u},
        {true, 0x50000040u, 0x022Au, 0x0070u, 0x0048u, 0x0036u, 0x03C0u, 0x0000u},
        {true, 0x50000040u, 0x022Au, 0x0070u, 0x0048u, 0x0036u, 0x03C0u, 0x0001u},
        {true, 0x50000040u, 0x0232u, 0x00B9u, 0x0034u, 0x0011u, 0x03E0u, 0x000Au},
        {true, 0x50000040u, 0x0232u, 0x00B9u, 0x0034u, 0x0011u, 0x03E0u, 0x000Bu},
        {true, 0x50000040u, 0x0232u, 0x00B9u, 0x0034u, 0x0011u, 0x03E0u, 0x000Cu},
        {true, 0x50000040u, 0x0180u, 0x019Du, 0x0024u, 0x000Bu, 0x03E0u, 0x000Du},
        {true, 0x50000040u, 0x0180u, 0x019Du, 0x0024u, 0x000Bu, 0x03E0u, 0x000Eu},
        {true, 0x50000040u, 0x0180u, 0x019Du, 0x0024u, 0x000Bu, 0x03E0u, 0x000Fu},
        {true, 0x50000040u, 0x0192u, 0x019Du, 0x0024u, 0x000Au, 0x03E0u, 0x000Du},
        {true, 0x50000040u, 0x0192u, 0x019Du, 0x0024u, 0x000Au, 0x03E0u, 0x000Eu},
        {true, 0x50000040u, 0x0192u, 0x019Du, 0x0024u, 0x000Au, 0x03E0u, 0x000Fu},
        {true, 0x50000040u, 0x0189u, 0x019Du, 0x0024u, 0x0007u, 0x03E0u, 0x000Du},
        {true, 0x50000040u, 0x0189u, 0x019Du, 0x0024u, 0x0007u, 0x03E0u, 0x000Eu},
        {true, 0x50000040u, 0x0189u, 0x019Du, 0x0024u, 0x0007u, 0x03E0u, 0x000Fu},
        {true, 0x50000040u, 0x019Bu, 0x019Du, 0x0024u, 0x000Bu, 0x03E0u, 0x000Du},
        {true, 0x50000040u, 0x019Bu, 0x019Du, 0x0024u, 0x000Bu, 0x03E0u, 0x000Eu},
        {true, 0x50000040u, 0x019Bu, 0x019Du, 0x0024u, 0x000Bu, 0x03E0u, 0x000Fu},
        {true, 0x50000040u, 0x01A4u, 0x019Du, 0x0024u, 0x0009u, 0x03E0u, 0x000Du},
        {true, 0x50000040u, 0x01A4u, 0x019Du, 0x0024u, 0x0009u, 0x03E0u, 0x000Eu},
        {true, 0x50000040u, 0x01A4u, 0x019Du, 0x0024u, 0x0009u, 0x03E0u, 0x000Fu},
    }};

constexpr std::array<PositionedTemplate80020BE4,
                     kCardInfoLanguageCount80020BE4>
    kLanguageTitles80053474 = {{
        {0x80051C10u, 95, 45},
        {0x80051C30u, 95, 45},
        {0x80051C20u, 98, 48},
        {0x80051C40u, 97, 47},
        {0x80051C50u, 95, 47},
    }};

constexpr PositionedTemplate80020BE4 kFixedTitle80051C00 = {
    0x80051C00u, 36, 28,
};

constexpr std::array<PositionedTemplate80020BE4,
                     kCardInfoMarkerCount80020BE4>
    kMarkers8005349C = {{
        {0x80051D10u, 31, 96},  {0x80051D50u, 49, 96},
        {0x80051D70u, 68, 96},  {0x80051DA0u, 85, 96},
        {0x80051DC0u, 103, 96}, {0x80051DF0u, 121, 96},
        {0x80051E10u, 140, 96}, {0x80051E20u, 158, 96},
        {0x80051E30u, 175, 96}, {0x80051E40u, 193, 96},
        {0x80051E50u, 210, 96}, {0x80051E60u, 229, 96},
        {0x80051E70u, 248, 96}, {0x80051EB0u, 265, 96},
        {0x80051EC0u, 32, 116}, {0x80051ED0u, 49, 116},
        {0x80051F10u, 68, 116}, {0x80051F30u, 86, 116},
        {0x80051F40u, 104, 116}, {0x80051F60u, 122, 116},
        {0x80051F70u, 140, 116}, {0x80051F80u, 158, 116},
        {0x80051F90u, 175, 116}, {0x80051FA0u, 194, 116},
        {0x80051FB0u, 212, 116}, {0x80051FD0u, 230, 116},
        {0x80051C70u, 248, 116}, {0x80051C80u, 266, 116},
        {0x80051C90u, 32, 137}, {0x80051CA0u, 50, 137},
        {0x80051CB0u, 68, 137}, {0x80051CC0u, 86, 137},
        {0x80051CD0u, 103, 137}, {0x80051CE0u, 122, 137},
        {0x80051CF0u, 140, 137}, {0x80051C60u, 158, 137},
        {0x80051E90u, 175, 137}, {0x80051DE0u, 194, 137},
        {0x80051D40u, 210, 137}, {0x80051F50u, 230, 137},
        {0x80051DB0u, 248, 136}, {0x80051D20u, 265, 135},
        {0x80051F00u, 31, 161}, {0x80051D80u, 51, 161},
        {0x80051DD0u, 71, 161}, {0x80051D30u, 91, 161},
        {0x80051E00u, 110, 161}, {0x80051D60u, 130, 161},
        {0x80051D00u, 151, 161}, {0x80051EF0u, 31, 185},
        {0x80051D90u, 51, 185}, {0x80051EE0u, 72, 185},
        {0x80051F20u, 91, 185}, {0x80051EA0u, 111, 185},
        {0x80051E80u, 130, 185}, {0x80051FC0u, 193, 185},
        {0x00000000u, 0, 0},
    }};

constexpr std::array<LanguageTemplateSet80020BE4,
                     kCardInfoLanguageCount80020BE4>
    kLowerTopLanguage80053244 = {{
        {{{0x80052010u, 0x80052020u, 0x80052030u}}, 234, 171},
        {{{0x80052070u, 0x80052080u, 0x80052090u}}, 240, 172},
        {{{0x80052040u, 0x80052050u, 0x80052060u}}, 234, 172},
        {{{0x800520A0u, 0x800520B0u, 0x800520C0u}}, 234, 173},
        {{{0x800520D0u, 0x800520E0u, 0x800520F0u}}, 238, 172},
    }};

constexpr std::array<LanguageTemplateSet80020BE4,
                     kCardInfoLanguageCount80020BE4>
    kLowerBottomLanguage80053364 = {{
        {{{0x80052150u, 0x80052160u, 0x80052170u}}, 232, 191},
        {{{0x800521B0u, 0x800521C0u, 0x800521D0u}}, 232, 193},
        {{{0x80052180u, 0x80052190u, 0x800521A0u}}, 232, 192},
        {{{0x800521E0u, 0x800521F0u, 0x80052200u}}, 232, 191},
        {{{0x80052210u, 0x80052220u, 0x80052230u}}, 233, 192},
    }};

constexpr std::array<uint32_t, 3u> kLowerMiddleFixed80053294 = {{
    0x80051FE0u,
    0x80051FF0u,
    0x80052000u,
}};

constexpr std::array<uint32_t, 3u> kLowerFinalFixed800532A0 = {{
    0x80052120u,
    0x80052130u,
    0x80052140u,
}};

constexpr bool IsRawTextureDescriptor80020BE4(
    const CardInfoSpriteDescriptor8001B25C& descriptor) {
    return descriptor.known &&
           descriptor.attr == kCardInfoRawTextureAttr80020BE4 &&
           (descriptor.attr & 0x40u) != 0u && descriptor.width != 0u &&
           descriptor.height != 0u;
}

constexpr bool AllTemplateDescriptorsRaw80020BE4() {
    for (const auto& descriptor : kTemplateDescriptors80051C00) {
        if (!IsRawTextureDescriptor80020BE4(descriptor)) {
            return false;
        }
    }
    return true;
}

static_assert(AllTemplateDescriptorsRaw80020BE4(),
              "all 80020BE4 static templates must be raw texture");
static_assert(kMarkers8005349C.size() == kCardInfoMarkerCount80020BE4,
              "80020BE4 marker table must contain 57 source calls");
static_assert(kMarkers8005349C[kCardInfoLowRamMarkerIndex80020BE4].address ==
                      0u &&
                  kMarkers8005349C[kCardInfoLowRamMarkerIndex80020BE4].x ==
                      0 &&
                  kMarkers8005349C[kCardInfoLowRamMarkerIndex80020BE4].y ==
                      0,
              "marker 56 must retain its explicit PSX low-RAM entry");

int16_t ReadS16LE80020BE4(const uint8_t* bytes) {
    const uint16_t value = static_cast<uint16_t>(bytes[0]) |
                           static_cast<uint16_t>(bytes[1]) << 8u;
    return static_cast<int16_t>(value);
}

bool TryResolveTemplateDescriptor80020BE4(
    uint32_t address,
    CardInfoSpriteDescriptor8001B25C& descriptor) {
    descriptor = {};
    if (address < kTemplateBankBase80051C00) {
        return false;
    }
    const uint32_t delta = address - kTemplateBankBase80051C00;
    if ((delta & 0x0Fu) != 0u) {
        return false;
    }
    const std::size_t index = static_cast<std::size_t>(delta >> 4u);
    if (index >= kTemplateDescriptors80051C00.size()) {
        return false;
    }
    descriptor = kTemplateDescriptors80051C00[index];
    return descriptor.known;
}

CardInfoSpriteDrawList80020BE4 RejectBuild80020BE4(
    const BuildFailure80020BE4& failure,
    bool inputFailure = false) {
    CardInfoSpriteDrawList80020BE4 rejected{};
    rejected.failure = true;
    rejected.inputFailure = inputFailure;
    rejected.previewFailure = failure.preview;
    rejected.descriptorFailure = failure.descriptor;
    rejected.rawTextureFailure = failure.rawTexture;
    rejected.callOrderFailure = failure.callOrder;
    rejected.capacityFailure = failure.capacity;
    rejected.truncated = failure.truncated;
    return rejected;
}

bool AppendStaticSprite80020BE4(
    CardInfoSpriteDrawList80020BE4& out,
    BuildFailure80020BE4& failure,
    const PositionedTemplate80020BE4& sprite,
    uint32_t sourceCallOrder,
    CardInfoSpriteRole80020BE4 role,
    int16_t markerIndex = -1,
    bool markerSelected = false) {
    CardInfoSpriteDescriptor8001B25C descriptor{};
    if (sprite.address == 0u ||
        !TryResolveTemplateDescriptor80020BE4(sprite.address, descriptor)) {
        failure.descriptor = true;
        return false;
    }

    bool markerPaletteAdjusted = false;
    if (markerIndex >= 0 && !markerSelected) {
        ++descriptor.clutY;
        markerPaletteAdjusted = true;
    }

    if (!IsRawTextureDescriptor80020BE4(descriptor)) {
        failure.rawTexture = true;
        out.rawTextureOnly = false;
        return false;
    }
    if (out.count >= out.commands.size()) {
        failure.capacity = true;
        failure.truncated = true;
        out.truncated = true;
        return false;
    }

    CardInfoSpriteCommand80020BE4 command{};
    command.known = true;
    command.sourceTemplateAddress = sprite.address;
    command.x = sprite.x;
    command.y = sprite.y;
    command.priority = kCardInfoSpritePriority80020BE4;
    command.descriptor = descriptor;
    command.rawTexture = true;
    command.callOrder = static_cast<uint32_t>(out.count);
    command.sourceCallOrder = sourceCallOrder;
    command.role = role;
    command.markerIndex = markerIndex;
    command.markerSelected = markerSelected;
    command.markerPaletteAdjusted = markerPaletteAdjusted;
    out.commands[out.count++] = command;
    if (role == CardInfoSpriteRole80020BE4::Marker) {
        ++out.markerCommandCount;
    }
    return true;
}

bool AppendLowRamMarkerSprite80020BE4(
    CardInfoSpriteDrawList80020BE4& out,
    BuildFailure80020BE4& failure,
    const CardInfoSpriteDescriptor8001B25C& sourceDescriptor,
    uint32_t sourceCallOrder,
    bool markerSelected) {
    if (!sourceDescriptor.known) {
        failure.descriptor = true;
        return false;
    }

    // 8001B428 copies all descriptor fields from PSX address 0 before
    // 8003FA20 applies its attr/width/height packet gates.
    CardInfoSpriteDescriptor8001B25C descriptor = sourceDescriptor;
    if ((descriptor.attr & 0x80000000u) != 0u || descriptor.width == 0u ||
        descriptor.height == 0u) {
        ++out.skippedSourceCallCount;
        return true;
    }

    if (!markerSelected) {
        ++descriptor.clutY;
    }
    if (!IsRawTextureDescriptor80020BE4(descriptor)) {
        failure.rawTexture = true;
        out.rawTextureOnly = false;
        return false;
    }
    if (out.count >= out.commands.size()) {
        failure.capacity = true;
        failure.truncated = true;
        out.truncated = true;
        return false;
    }

    CardInfoSpriteCommand80020BE4 command{};
    command.known = true;
    command.sourceTemplateAddress = 0u;
    command.x = 0;
    command.y = 0;
    command.priority = kCardInfoSpritePriority80020BE4;
    command.descriptor = descriptor;
    command.rawTexture = true;
    command.callOrder = static_cast<uint32_t>(out.count);
    command.sourceCallOrder = sourceCallOrder;
    command.role = CardInfoSpriteRole80020BE4::Marker;
    command.markerIndex =
        static_cast<int16_t>(kCardInfoLowRamMarkerIndex80020BE4);
    command.markerSelected = markerSelected;
    command.markerPaletteAdjusted = !markerSelected;
    out.commands[out.count++] = command;
    ++out.markerCommandCount;
    return true;
}

bool AppendPreviewGlyph8001B744(
    CardInfoSpriteDrawList80020BE4& out,
    BuildFailure80020BE4& failure,
    uint8_t glyphCode,
    uint32_t sourceCallOrder,
    int32_t& penX) {
    PrPsxTextGlyphMetricsDirect::GlyphMetricRaw8004945C raw{};
    if (!PrPsxTextGlyphMetricsDirect::TryCopyGlyphMetricRaw8004945C(
            glyphCode, raw)) {
        failure.preview = true;
        return false;
    }

    const int16_t textureX = ReadS16LE80020BE4(raw.data());
    const int16_t textureY = ReadS16LE80020BE4(raw.data() + 2u);
    const uint16_t width = raw[4u];
    const uint16_t height = raw[5u];
    const int16_t xOffset = ReadS16LE80020BE4(raw.data() + 6u);
    const int32_t penBefore = penX;
    penX += static_cast<int32_t>(width);

    // 8001B744 still advances the pen for a metric without a texture packet.
    if (textureY < 0) {
        ++out.previewNoPacketGlyphCount;
        ++out.skippedSourceCallCount;
        return true;
    }
    if (width == 0u || height == 0u) {
        failure.preview = true;
        return false;
    }
    if (out.count >= out.commands.size()) {
        failure.capacity = true;
        failure.truncated = true;
        out.truncated = true;
        return false;
    }

    const int32_t roundedUBase =
        textureX <= 0 ? static_cast<int32_t>(textureX) + 3584
                      : static_cast<int32_t>(textureX) + 3583;
    const uint16_t uvWord = static_cast<uint16_t>(roundedUBase);
    const uint16_t helperA3 =
        static_cast<uint16_t>((uvWord & 0xFF00u) >> 2u);
    const uint16_t helperA4 = static_cast<uint16_t>(
        static_cast<uint16_t>(static_cast<int32_t>(textureY) + 256) &
        0xFF00u);

    CardInfoSpriteCommand80020BE4 command{};
    command.known = true;
    command.sourceTemplateAddress = kFn8001B744_CardInfoTextGlyph;
    command.x = static_cast<int16_t>(
        static_cast<int32_t>(kCardInfoPreviewX80020BE4) + penBefore +
        xOffset);
    command.y = static_cast<int16_t>(
        static_cast<int32_t>(kCardInfoPreviewY80020BE4) +
        (glyphCode >= 192u && glyphCode <= 222u ? -3 : 0));
    command.priority = kCardInfoSpritePriority80020BE4;
    command.descriptor.known = true;
    command.descriptor.attr = kCardInfoRawTextureAttr80020BE4;
    command.descriptor.width = width;
    command.descriptor.height = height;
    command.descriptor.clutX = kCardInfoPreviewClutX80020BE4;
    command.descriptor.clutY = kCardInfoPreviewClutY80020BE4;
    command.rawTexture = true;
    command.textureCoordinatesResolved = true;
    command.callOrder = static_cast<uint32_t>(out.count);
    command.sourceCallOrder = sourceCallOrder;
    command.tpage = static_cast<uint16_t>(
        0x20u | ((helperA4 & 0x0100u) >> 4u) |
        ((helperA3 & 0x03FFu) >> 6u) | (4u * (helperA4 & 0x0200u)));
    command.u = static_cast<uint8_t>(uvWord);
    command.v = static_cast<uint8_t>(textureY);
    command.glyphCode = glyphCode;
    command.role = CardInfoSpriteRole80020BE4::PreviewGlyph;
    if (!IsRawTextureDescriptor80020BE4(command.descriptor)) {
        failure.rawTexture = true;
        out.rawTextureOnly = false;
        return false;
    }

    out.commands[out.count++] = command;
    ++out.previewGlyphSpriteCount;
    ++out.glyphSpriteCount;
    return true;
}

bool ValidatePublishedOrder80020BE4(
    const CardInfoSpriteDrawList80020BE4& out) {
    uint32_t priorSourceCallOrder = 0u;
    for (std::size_t i = 0u; i < out.count; ++i) {
        const auto& command = out.commands[i];
        if (!command.known || !command.rawTexture ||
            command.callOrder != static_cast<uint32_t>(i) ||
            command.sourceCallOrder >= out.sourceCallCount ||
            (i != 0u && command.sourceCallOrder <= priorSourceCallOrder)) {
            return false;
        }
        priorSourceCallOrder = command.sourceCallOrder;
    }
    return true;
}

}  // namespace

CardInfoSpriteDrawList80020BE4 BuildCardInfoSpriteDrawList80020BE4(
    const CardInfoSpriteInput80020BE4& input) {
    const bool previewInputFailure =
        !input.previewBytesKnown ||
        input.previewByteCount > kCardInfoPreviewMaxByteCount80020BE4 ||
        (input.previewByteCount != 0u && input.previewBytes == nullptr) ||
        !input.lowRamDescriptorKnown ||
        !input.lowRamDescriptorAtPhysicalZero.known;
    const bool baseInputFailure =
        !input.requestBound || input.argAddress != kCardInfoArgAddress80049244 ||
        !input.languageKnown || input.languageIndex < 0 ||
        input.languageIndex >=
            static_cast<int32_t>(kCardInfoLanguageCount80020BE4) ||
        !input.topFlagKnown || !input.selectedMarkerKnown ||
        input.selectedMarker < 0 ||
        input.selectedMarker >=
            static_cast<int32_t>(kCardInfoMarkerCount80020BE4) ||
        !input.lowerModeKnown ||
        input.lowerMode < std::numeric_limits<int16_t>::min() ||
        input.lowerMode > std::numeric_limits<int16_t>::max() ||
        !input.topIconTemplateSlotsKnown ||
        input.topIconOffTemplate == 0u || input.topIconOnTemplate == 0u;
    if (previewInputFailure || baseInputFailure) {
        BuildFailure80020BE4 failure{};
        failure.preview = previewInputFailure;
        return RejectBuild80020BE4(failure, true);
    }

    for (std::size_t i = 0u; i < input.previewByteCount; ++i) {
        if (input.previewBytes[i] == 0u) {
            BuildFailure80020BE4 failure{};
            failure.preview = true;
            return RejectBuild80020BE4(failure, true);
        }
    }

    CardInfoSpriteDrawList80020BE4 candidate{};
    candidate.rawTextureOnly = true;
    candidate.sourceKnown = true;
    candidate.previewByteCount = input.previewByteCount;
    BuildFailure80020BE4 failure{};
    uint32_t sourceCallOrder = 0u;
    int32_t previewPenX = 0;

    for (std::size_t i = 0u; i < input.previewByteCount; ++i) {
        if (!AppendPreviewGlyph8001B744(candidate,
                                        failure,
                                        input.previewBytes[i],
                                        sourceCallOrder,
                                        previewPenX)) {
            return RejectBuild80020BE4(failure);
        }
        ++sourceCallOrder;
    }

    const std::size_t language =
        static_cast<std::size_t>(input.languageIndex);
    if (!AppendStaticSprite80020BE4(candidate,
                                    failure,
                                    kLanguageTitles80053474[language],
                                    sourceCallOrder++,
                                    CardInfoSpriteRole80020BE4::LanguageTitle) ||
        !AppendStaticSprite80020BE4(candidate,
                                    failure,
                                    kFixedTitle80051C00,
                                    sourceCallOrder++,
                                    CardInfoSpriteRole80020BE4::FixedTitle)) {
        return RejectBuild80020BE4(failure);
    }

    for (std::size_t marker = 0u; marker < kMarkers8005349C.size();
         ++marker) {
        const PositionedTemplate80020BE4& sprite = kMarkers8005349C[marker];
        ++candidate.markerSourceCallCount;
        if (marker == kCardInfoLowRamMarkerIndex80020BE4) {
            if (sprite.address != 0u || sprite.x != 0 || sprite.y != 0) {
                failure.descriptor = true;
                return RejectBuild80020BE4(failure);
            }
            candidate.lowRamMarkerPointerZeroKnown = true;
            candidate.lowRamMarkerDescriptorKnown = true;
            candidate.lowRamMarkerPacketOutcomeKnown = true;
            candidate.lowRamMarkerSelected =
                input.selectedMarker ==
                static_cast<int32_t>(kCardInfoLowRamMarkerIndex80020BE4);
            candidate.lowRamMarkerSourceCallOrder = sourceCallOrder;
            const std::size_t commandCountBefore = candidate.count;
            if (!AppendLowRamMarkerSprite80020BE4(
                    candidate,
                    failure,
                    input.lowRamDescriptorAtPhysicalZero,
                    sourceCallOrder,
                    candidate.lowRamMarkerSelected)) {
                return RejectBuild80020BE4(failure);
            }
            candidate.lowRamMarkerPacketEmitted =
                candidate.count != commandCountBefore;
            ++sourceCallOrder;
            continue;
        }

        const bool selected =
            input.selectedMarker == static_cast<int32_t>(marker);
        if (!AppendStaticSprite80020BE4(
                candidate,
                failure,
                sprite,
                sourceCallOrder++,
                CardInfoSpriteRole80020BE4::Marker,
                static_cast<int16_t>(marker),
                selected)) {
            return RejectBuild80020BE4(failure);
        }
    }

    PositionedTemplate80020BE4 lowerTopIcon{};
    std::size_t topLanguageVariant = 0u;
    std::size_t bottomLanguageVariant = 0u;
    std::size_t middleFixedVariant = 0u;
    std::size_t finalFixedVariant = 0u;
    // 80020BE4 reads ctx+0x1A as a signed halfword and compares only with
    // 1 and 2. Every other representable value uses the mode-0/default body.
    if (input.lowerMode == 1) {
        lowerTopIcon = {input.topIconOnTemplate, 223, 160};
        topLanguageVariant = 2u;
        bottomLanguageVariant = 1u;
        middleFixedVariant = 2u;
        finalFixedVariant = 1u;
    } else if (input.lowerMode == 2) {
        lowerTopIcon = {input.topIconOnTemplate, 223, 160};
        topLanguageVariant = 1u;
        bottomLanguageVariant = 2u;
        middleFixedVariant = 1u;
        finalFixedVariant = 2u;
    } else if (input.selectedMarker ==
               static_cast<int32_t>(kCardInfoLowRamMarkerIndex80020BE4)) {
        // 80020E40 loads the full ctx+0 dword and 80020E48 tests only zero
        // versus nonzero. Preserve that predicate at the consumption point.
        lowerTopIcon = {
            input.topFlag != 0u ? input.topIconOnTemplate
                                : input.topIconOffTemplate,
            223,
            160,
        };
        topLanguageVariant = 1u;
        bottomLanguageVariant = 1u;
        middleFixedVariant = 1u;
        finalFixedVariant = 1u;
    } else {
        lowerTopIcon = {input.topIconOffTemplate, 223, 160};
    }

    const LanguageTemplateSet80020BE4& topLanguage =
        kLowerTopLanguage80053244[language];
    const LanguageTemplateSet80020BE4& bottomLanguage =
        kLowerBottomLanguage80053364[language];
    const PositionedTemplate80020BE4 lowerTopLanguage = {
        topLanguage.variants[topLanguageVariant],
        topLanguage.x,
        topLanguage.y,
    };
    const PositionedTemplate80020BE4 lowerBottomLanguage = {
        bottomLanguage.variants[bottomLanguageVariant],
        bottomLanguage.x,
        bottomLanguage.y,
    };
    const PositionedTemplate80020BE4 lowerMiddleFixed = {
        kLowerMiddleFixed80053294[middleFixedVariant], 232, 169,
    };
    const PositionedTemplate80020BE4 lowerFinalFixed = {
        kLowerFinalFixed800532A0[finalFixedVariant], 232, 188,
    };

    if (!AppendStaticSprite80020BE4(
            candidate,
            failure,
            lowerTopIcon,
            sourceCallOrder++,
            CardInfoSpriteRole80020BE4::LowerTopIcon) ||
        !AppendStaticSprite80020BE4(
            candidate,
            failure,
            lowerTopLanguage,
            sourceCallOrder++,
            CardInfoSpriteRole80020BE4::LowerTopLanguageText) ||
        !AppendStaticSprite80020BE4(
            candidate,
            failure,
            lowerBottomLanguage,
            sourceCallOrder++,
            CardInfoSpriteRole80020BE4::LowerBottomLanguageText) ||
        !AppendStaticSprite80020BE4(
            candidate,
            failure,
            lowerMiddleFixed,
            sourceCallOrder++,
            CardInfoSpriteRole80020BE4::LowerMiddleFixed) ||
        !AppendStaticSprite80020BE4(
            candidate,
            failure,
            lowerFinalFixed,
            sourceCallOrder++,
            CardInfoSpriteRole80020BE4::LowerFinalFixed)) {
        return RejectBuild80020BE4(failure);
    }

    candidate.sourceCallCount = sourceCallOrder;
    candidate.sourceSpriteCallCount = sourceCallOrder;
    const std::size_t expectedCommandCount =
        candidate.previewGlyphSpriteCount +
        kCardInfoStaticSpriteCommandCountWithoutLowRam80020BE4 +
        (candidate.lowRamMarkerPacketEmitted ? 1u : 0u);
    const std::size_t expectedSourceCallCount =
        input.previewByteCount + kCardInfoStaticSourceCallCount80020BE4;
    const std::size_t expectedSkippedSourceCallCount =
        candidate.previewNoPacketGlyphCount +
        (candidate.lowRamMarkerPacketEmitted ? 0u : 1u);
    if (candidate.count != expectedCommandCount ||
        candidate.sourceCallCount != expectedSourceCallCount ||
        candidate.sourceSpriteCallCount != expectedSourceCallCount ||
        candidate.skippedSourceCallCount != expectedSkippedSourceCallCount ||
        candidate.markerSourceCallCount != kCardInfoMarkerCount80020BE4 ||
        candidate.markerCommandCount !=
            kCardInfoMarkerCount80020BE4 -
                (candidate.lowRamMarkerPacketEmitted ? 0u : 1u) ||
        !candidate.lowRamMarkerPointerZeroKnown ||
        !candidate.lowRamMarkerDescriptorKnown ||
        !candidate.lowRamMarkerPacketOutcomeKnown ||
        !candidate.rawTextureOnly ||
        !ValidatePublishedOrder80020BE4(candidate)) {
        failure.callOrder = true;
        if (!candidate.rawTextureOnly) {
            failure.rawTexture = true;
        }
        return RejectBuild80020BE4(failure);
    }

    candidate.accepted = true;
    candidate.complete = true;
    return candidate;
}

}  // namespace PrSS0CardInfoRenderDirect
