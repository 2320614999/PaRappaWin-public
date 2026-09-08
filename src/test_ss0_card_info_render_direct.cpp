#include "pr/pr_ss0_card_info_render_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>

namespace {

using namespace PrSS0CardInfoRenderDirect;

int g_failedChecks = 0;

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            std::printf("CHECK failed: %s:%d: %s\n", __FILE__, __LINE__,      \
                        #condition);                                            \
            ++g_failedChecks;                                                   \
        }                                                                       \
    } while (false)

constexpr uint32_t kCardInfoArgAddress80049244 = 0x80049244u;
constexpr std::size_t kTitleCommandIndex = 0u;
constexpr std::size_t kFixedTitleCommandIndex = 1u;
constexpr std::size_t kMarkerCommandStart = 2u;
constexpr std::size_t kLowerCommandStart = 58u;
constexpr std::size_t kBaseCommandCount = 63u;

struct ExpectedCommand {
    uint32_t sourceTemplateAddress;
    int16_t x;
    int16_t y;
};

constexpr std::array<ExpectedCommand, 56> kMarkerCommands = {{
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
}};

CardInfoSpriteInput80020BE4 MakeInput(int32_t language = 0,
                                      int32_t selectedMarker = 0,
                                      int32_t lowerMode = 0,
                                      uint32_t topFlag = 0u) {
    CardInfoSpriteInput80020BE4 input{};
    input.requestBound = true;
    input.argAddress = kCardInfoArgAddress80049244;
    input.languageKnown = true;
    input.languageIndex = language;
    input.topFlagKnown = true;
    input.topFlag = topFlag;
    input.selectedMarkerKnown = true;
    input.selectedMarker = selectedMarker;
    input.lowerModeKnown = true;
    input.lowerMode = lowerMode;
    input.topIconTemplateSlotsKnown = true;
    input.topIconOffTemplate = 0x80052100u;
    input.topIconOnTemplate = 0x80052110u;
    input.previewBytesKnown = true;
    input.previewBytes = nullptr;
    input.previewByteCount = 0u;
    input.lowRamDescriptorKnown = true;
    input.lowRamDescriptorAtPhysicalZero.known = true;
    return input;
}

void CheckFailClosed(const CardInfoSpriteInput80020BE4& input) {
    const auto list = BuildCardInfoSpriteDrawList80020BE4(input);
    CHECK(!list.accepted);
    CHECK(!list.complete);
    CHECK(!list.rawTextureOnly);
    CHECK(list.glyphSpriteCount == 0u);
    CHECK(list.count == 0u);
    CHECK(!list.truncated);
}

template <typename DrawList>
void CheckCommand(const DrawList& list,
                  std::size_t index,
                  uint32_t sourceTemplateAddress,
                  int16_t x,
                  int16_t y) {
    CHECK(index < list.count);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.known);
    CHECK(command.rawTexture);
    CHECK(command.sourceTemplateAddress == sourceTemplateAddress);
    CHECK(command.x == x);
    CHECK(command.y == y);
    CHECK(command.priority == 0u);
    CHECK(command.callOrder == index);
}

template <typename DrawList>
void CheckAccepted(const DrawList& list,
                   std::size_t expectedGlyphSpriteCount = 0u,
                   std::size_t expectedLowRamMarkerPacketCount = 0u) {
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(list.glyphSpriteCount == expectedGlyphSpriteCount);
    CHECK(list.count == kBaseCommandCount + expectedGlyphSpriteCount +
                            expectedLowRamMarkerPacketCount);
    CHECK(!list.truncated);
    for (std::size_t index = 0u; index < list.count; ++index) {
        const auto& command = list.commands[index];
        CHECK(command.known);
        CHECK(command.rawTexture);
        CHECK(command.priority == 0u);
        CHECK(command.callOrder == index);
        CHECK(command.descriptor.attr == 0x50000040u);
        CHECK((command.descriptor.attr & 0x40u) != 0u);
        CHECK(command.descriptor.width != 0u);
        CHECK(command.descriptor.height != 0u);
    }
}

template <typename DrawList>
void CheckLower(const DrawList& list,
                uint32_t topIcon,
                uint32_t topText,
                int16_t topTextX,
                int16_t topTextY,
                uint32_t bottomText,
                int16_t bottomTextX,
                int16_t bottomTextY,
                uint32_t middle,
                uint32_t final) {
    CheckCommand(list, kLowerCommandStart + 0u, topIcon, 223, 160);
    CheckCommand(list, kLowerCommandStart + 1u, topText,
                 topTextX, topTextY);
    CheckCommand(list, kLowerCommandStart + 2u, bottomText,
                 bottomTextX, bottomTextY);
    CheckCommand(list, kLowerCommandStart + 3u, middle, 232, 169);
    CheckCommand(list, kLowerCommandStart + 4u, final, 232, 188);
}

void TestFailClosedGates() {
    CardInfoSpriteInput80020BE4 input = MakeInput();
    input.requestBound = false;
    CheckFailClosed(input);

    input = MakeInput();
    input.argAddress = 0x80049240u;
    CheckFailClosed(input);

    input = MakeInput();
    input.languageKnown = false;
    CheckFailClosed(input);

    input = MakeInput(-1);
    CheckFailClosed(input);
    input = MakeInput(5);
    CheckFailClosed(input);

    input = MakeInput();
    input.topFlagKnown = false;
    CheckFailClosed(input);

    input = MakeInput();
    input.selectedMarkerKnown = false;
    CheckFailClosed(input);

    input = MakeInput(0, -1);
    CheckFailClosed(input);
    input = MakeInput(0, 57);
    CheckFailClosed(input);

    input = MakeInput();
    input.lowerModeKnown = false;
    CheckFailClosed(input);

    input = MakeInput(
        0, 0, std::numeric_limits<int32_t>::min());
    CheckFailClosed(input);
    input = MakeInput(
        0, 0, std::numeric_limits<int32_t>::max());
    CheckFailClosed(input);

    input = MakeInput();
    input.topIconTemplateSlotsKnown = false;
    CheckFailClosed(input);

    input = MakeInput();
    input.topIconOffTemplate = 0u;
    CheckFailClosed(input);

    input = MakeInput();
    input.topIconOnTemplate = 0u;
    CheckFailClosed(input);

    input = MakeInput();
    input.topIconOffTemplate = 0x80050000u;
    CheckFailClosed(input);

    input = MakeInput();
    input.previewBytesKnown = false;
    CheckFailClosed(input);

    input = MakeInput();
    input.lowRamDescriptorKnown = false;
    CheckFailClosed(input);

    input = MakeInput();
    input.lowRamDescriptorAtPhysicalZero.known = false;
    CheckFailClosed(input);
}

template <typename DrawList>
void CheckPreviewGlyph(const DrawList& list,
                       std::size_t index,
                       uint8_t glyphCode,
                       int16_t x,
                       uint16_t width,
                       uint8_t u) {
    CheckCommand(list, index, 0x8001B744u, x, 64);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.textureCoordinatesResolved);
    CHECK(command.glyphCode == glyphCode);
    CHECK(command.descriptor.width == width);
    CHECK(command.descriptor.height == 15u);
    CHECK(command.tpage == 0x003Eu);
    CHECK(command.u == u);
    CHECK(command.v == 0u);
    CHECK(command.descriptor.clutX == 256u);
    CHECK(command.descriptor.clutY == 482u);
}

void TestEncodedPreviewGlyphs() {
    const auto empty =
        BuildCardInfoSpriteDrawList80020BE4(MakeInput());
    CheckAccepted(empty, 0u);
    CheckCommand(empty, 0u, 0x80051C10u, 95, 45);

    constexpr std::array<uint8_t, 1> oneBytePreview = {{
        static_cast<uint8_t>('0'),
    }};
    CardInfoSpriteInput80020BE4 oneByte = MakeInput();
    oneByte.previewBytes = oneBytePreview.data();
    oneByte.previewByteCount = oneBytePreview.size();
    const auto one = BuildCardInfoSpriteDrawList80020BE4(oneByte);
    CheckAccepted(one, 1u);
    CheckPreviewGlyph(one, 0u, 0x30u, 117, 8u, 0x83u);
    CheckCommand(one, 1u, 0x80051C10u, 95, 45);

    constexpr std::array<uint8_t, 2> twoBytePreview = {{
        static_cast<uint8_t>('0'),
        static_cast<uint8_t>('1'),
    }};
    CardInfoSpriteInput80020BE4 twoBytes = MakeInput();
    twoBytes.previewBytes = twoBytePreview.data();
    twoBytes.previewByteCount = twoBytePreview.size();
    const auto two = BuildCardInfoSpriteDrawList80020BE4(twoBytes);
    CheckAccepted(two, 2u);
    CheckPreviewGlyph(two, 0u, 0x30u, 117, 8u, 0x83u);
    CheckPreviewGlyph(two, 1u, 0x31u, 125, 7u, 0x8Bu);
    CheckCommand(two, 2u, 0x80051C10u, 95, 45);
    CHECK(two.commands[0u].callOrder == 0u);
    CHECK(two.commands[1u].callOrder == 1u);
    CHECK(two.commands[2u].callOrder == 2u);
}

void TestDefaultOrderAndMarkerTable() {
    const auto list = BuildCardInfoSpriteDrawList80020BE4(MakeInput());
    CheckAccepted(list);
    CHECK(list.lowRamMarkerPointerZeroKnown);
    CHECK(list.lowRamMarkerDescriptorKnown);
    CHECK(list.lowRamMarkerPacketOutcomeKnown);
    CHECK(!list.lowRamMarkerPacketEmitted);
    CHECK(list.lowRamMarkerSourceCallOrder == 58u);
    CheckCommand(list, kTitleCommandIndex, 0x80051C10u, 95, 45);
    CheckCommand(list, kFixedTitleCommandIndex, 0x80051C00u, 36, 28);

    for (std::size_t marker = 0u; marker < kMarkerCommands.size();
         ++marker) {
        const auto& expected = kMarkerCommands[marker];
        CheckCommand(list, kMarkerCommandStart + marker,
                     expected.sourceTemplateAddress, expected.x, expected.y);
    }

    const auto& selected = list.commands[kMarkerCommandStart];
    CHECK(selected.descriptor.texX == 0x0200u);
    CHECK(selected.descriptor.texY == 0x001Cu);
    CHECK(selected.descriptor.width == 0x0018u);
    CHECK(selected.descriptor.height == 0x001Cu);
    CHECK(selected.descriptor.clutX == 0x03E0u);
    CHECK(selected.descriptor.clutY == 0u);
    CHECK(list.commands[kMarkerCommandStart + 1u].descriptor.clutY == 1u);
    CHECK(list.commands[kMarkerCommandStart + 55u].sourceCallOrder == 57u);
    CHECK(list.commands[kLowerCommandStart].sourceCallOrder == 59u);
    CHECK(list.commands[kLowerCommandStart].callOrder == 58u);

    CheckLower(list, 0x80052100u,
               0x80052010u, 234, 171,
               0x80052150u, 232, 191,
               0x80051FE0u, 0x80052120u);
}

void TestSelectedMarkerStateChangesOnlyTwoMarkers() {
    const auto selected0 =
        BuildCardInfoSpriteDrawList80020BE4(MakeInput(0, 0));
    const auto selected1 =
        BuildCardInfoSpriteDrawList80020BE4(MakeInput(0, 1));
    CheckAccepted(selected0);
    CheckAccepted(selected1);

    for (std::size_t index = 0u; index < selected0.count; ++index) {
        const auto& before = selected0.commands[index];
        const auto& after = selected1.commands[index];
        CHECK(before.sourceTemplateAddress == after.sourceTemplateAddress);
        CHECK(before.x == after.x);
        CHECK(before.y == after.y);
        CHECK(before.priority == after.priority);
        CHECK(before.descriptor.attr == after.descriptor.attr);
        CHECK(before.descriptor.texX == after.descriptor.texX);
        CHECK(before.descriptor.texY == after.descriptor.texY);
        CHECK(before.descriptor.width == after.descriptor.width);
        CHECK(before.descriptor.height == after.descriptor.height);
        CHECK(before.descriptor.clutX == after.descriptor.clutX);
        if (index != kMarkerCommandStart &&
            index != kMarkerCommandStart + 1u) {
            CHECK(before.descriptor.clutY == after.descriptor.clutY);
        }
    }

    CHECK(selected0.commands[kMarkerCommandStart].descriptor.clutY == 0u);
    CHECK(selected1.commands[kMarkerCommandStart].descriptor.clutY == 1u);
    CHECK(selected0.commands[kMarkerCommandStart + 1u].descriptor.clutY == 1u);
    CHECK(selected1.commands[kMarkerCommandStart + 1u].descriptor.clutY == 0u);
    CHECK(selected1.commands[kMarkerCommandStart].sourceTemplateAddress ==
          0x80051D10u);
    CHECK(selected1.commands[kMarkerCommandStart + 1u].sourceTemplateAddress ==
          0x80051D50u);
}

void TestLowerModes() {
    const auto mode1 =
        BuildCardInfoSpriteDrawList80020BE4(MakeInput(0, 0, 1));
    CheckAccepted(mode1);
    CheckLower(mode1, 0x80052110u,
               0x80052030u, 234, 171,
               0x80052160u, 232, 191,
               0x80052000u, 0x80052130u);

    const auto mode2 =
        BuildCardInfoSpriteDrawList80020BE4(MakeInput(0, 0, 2));
    CheckAccepted(mode2);
    CheckLower(mode2, 0x80052110u,
               0x80052020u, 234, 171,
               0x80052170u, 232, 191,
               0x80051FF0u, 0x80052140u);
}

void TestNoncanonicalLowerModeUsesOriginalDefaultBranch() {
    constexpr std::array<int32_t, 4u> kDefaultModes = {
        static_cast<int32_t>(std::numeric_limits<int16_t>::min()),
        -1,
        3,
        static_cast<int32_t>(std::numeric_limits<int16_t>::max()),
    };
    for (const int32_t lowerMode : kDefaultModes) {
        const auto ordinary = BuildCardInfoSpriteDrawList80020BE4(
            MakeInput(0, 0, lowerMode, true));
        CheckAccepted(ordinary);
        CheckLower(ordinary, 0x80052100u,
                   0x80052010u, 234, 171,
                   0x80052150u, 232, 191,
                   0x80051FE0u, 0x80052120u);

        const auto lowRamOff = BuildCardInfoSpriteDrawList80020BE4(
            MakeInput(0, 56, lowerMode, false));
        CheckAccepted(lowRamOff);
        CheckLower(lowRamOff, 0x80052100u,
                   0x80052020u, 234, 171,
                   0x80052160u, 232, 191,
                   0x80051FF0u, 0x80052130u);

        const auto lowRamOn = BuildCardInfoSpriteDrawList80020BE4(
            MakeInput(0, 56, lowerMode, true));
        CheckAccepted(lowRamOn);
        CheckLower(lowRamOn, 0x80052110u,
                   0x80052020u, 234, 171,
                   0x80052160u, 232, 191,
                   0x80051FF0u, 0x80052130u);
    }
}

void TestLowRamMarker56AndTopFlag() {
    const auto topOff =
        BuildCardInfoSpriteDrawList80020BE4(MakeInput(0, 56, 0, false));
    CheckAccepted(topOff);
    CheckLower(topOff, 0x80052100u,
               0x80052020u, 234, 171,
               0x80052160u, 232, 191,
               0x80051FF0u, 0x80052130u);

    for (std::size_t marker = 0u; marker < kMarkerCommands.size();
         ++marker) {
        CHECK(topOff.commands[kMarkerCommandStart + marker]
                  .descriptor.clutY == 1u);
    }
    CHECK(topOff.commands[kMarkerCommandStart + 55u]
              .sourceTemplateAddress == 0x80051FC0u);

    const auto topOn =
        BuildCardInfoSpriteDrawList80020BE4(MakeInput(0, 56, 0, true));
    CheckAccepted(topOn);
    CheckLower(topOn, 0x80052110u,
               0x80052020u, 234, 171,
               0x80052160u, 232, 191,
               0x80051FF0u, 0x80052130u);

    const auto nonNullTopOn =
        BuildCardInfoSpriteDrawList80020BE4(MakeInput(0, 0, 0, true));
    CheckAccepted(nonNullTopOn);
    CHECK(nonNullTopOn.commands[kLowerCommandStart]
              .sourceTemplateAddress == 0x80052100u);
}

void TestNoncanonicalTopFlagUsesOriginalNonzeroPredicate() {
    constexpr std::array<uint32_t, 3u> kNonzeroTopFlags = {
        2u,
        0x80000000u,
        0xFFFFFFFFu,
    };
    for (const uint32_t topFlag : kNonzeroTopFlags) {
        const auto lowRamOn = BuildCardInfoSpriteDrawList80020BE4(
            MakeInput(0, 56, 0, topFlag));
        CheckAccepted(lowRamOn);
        CheckLower(lowRamOn, 0x80052110u,
                   0x80052020u, 234, 171,
                   0x80052160u, 232, 191,
                   0x80051FF0u, 0x80052130u);

        const auto ordinaryMarker = BuildCardInfoSpriteDrawList80020BE4(
            MakeInput(0, 0, 0, topFlag));
        CheckAccepted(ordinaryMarker);
        CHECK(ordinaryMarker.commands[kLowerCommandStart]
                  .sourceTemplateAddress == 0x80052100u);
    }
}

void TestLowRamMarkerDescriptorCanEmitPacket() {
    CardInfoSpriteInput80020BE4 selectedInput = MakeInput(0, 56);
    selectedInput.lowRamDescriptorAtPhysicalZero = {
        true,
        0x50000040u,
        0x0200u,
        0x001Cu,
        0x0018u,
        0x001Cu,
        0x03E0u,
        0x0007u,
    };
    const auto selected = BuildCardInfoSpriteDrawList80020BE4(selectedInput);
    CheckAccepted(selected, 0u, 1u);
    CHECK(selected.lowRamMarkerPacketEmitted);
    CHECK(selected.markerCommandCount == 57u);
    CHECK(selected.skippedSourceCallCount == 0u);
    CheckCommand(selected, 58u, 0u, 0, 0);
    CHECK(selected.commands[58u].markerIndex == 56);
    CHECK(selected.commands[58u].markerSelected);
    CHECK(!selected.commands[58u].markerPaletteAdjusted);
    CHECK(selected.commands[58u].descriptor.clutY == 7u);
    CHECK(selected.commands[58u].sourceCallOrder == 58u);
    CHECK(selected.commands[59u].sourceCallOrder == 59u);

    CardInfoSpriteInput80020BE4 unselectedInput = selectedInput;
    unselectedInput.selectedMarker = 0;
    const auto unselected =
        BuildCardInfoSpriteDrawList80020BE4(unselectedInput);
    CheckAccepted(unselected, 0u, 1u);
    CHECK(unselected.commands[58u].markerPaletteAdjusted);
    CHECK(unselected.commands[58u].descriptor.clutY == 8u);
}

void TestLanguageSpecificTitleAndLowerText() {
    constexpr ExpectedCommand kTitles[5] = {
        {0x80051C10u, 95, 45}, {0x80051C30u, 95, 45},
        {0x80051C20u, 98, 48}, {0x80051C40u, 97, 47},
        {0x80051C50u, 95, 47},
    };
    constexpr ExpectedCommand kTopText[5] = {
        {0x80052010u, 234, 171}, {0x80052070u, 240, 172},
        {0x80052040u, 234, 172}, {0x800520A0u, 234, 173},
        {0x800520D0u, 238, 172},
    };
    constexpr ExpectedCommand kBottomText[5] = {
        {0x80052150u, 232, 191}, {0x800521B0u, 232, 193},
        {0x80052180u, 232, 192}, {0x800521E0u, 232, 191},
        {0x80052210u, 233, 192},
    };

    for (int32_t language = 0; language < 5; ++language) {
        const auto list =
            BuildCardInfoSpriteDrawList80020BE4(MakeInput(language));
        CheckAccepted(list);
        const auto& title = kTitles[language];
        const auto& top = kTopText[language];
        const auto& bottom = kBottomText[language];
        CheckCommand(list, kTitleCommandIndex,
                     title.sourceTemplateAddress, title.x, title.y);
        CheckCommand(list, kLowerCommandStart + 1u,
                     top.sourceTemplateAddress, top.x, top.y);
        CheckCommand(list, kLowerCommandStart + 2u,
                     bottom.sourceTemplateAddress, bottom.x, bottom.y);
    }
}

}  // namespace

int main() {
    TestFailClosedGates();
    TestEncodedPreviewGlyphs();
    TestDefaultOrderAndMarkerTable();
    TestSelectedMarkerStateChangesOnlyTwoMarkers();
    TestLowerModes();
    TestNoncanonicalLowerModeUsesOriginalDefaultBranch();
    TestLowRamMarker56AndTopFlag();
    TestNoncanonicalTopFlagUsesOriginalNonzeroPredicate();
    TestLowRamMarkerDescriptorCanEmitPacket();
    TestLanguageSpecificTitleAndLowerText();

    if (g_failedChecks != 0) {
        std::printf("test_ss0_card_info_render_direct: failed checks=%d\n",
                    g_failedChecks);
        return 1;
    }

    std::printf("test_ss0_card_info_render_direct: ok\n");
    return 0;
}
