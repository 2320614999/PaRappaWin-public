#include "pr/pr_ss0_directory_pages_render_direct.h"
#include "pr/pr_ss0_event_backdrop_render_direct.h"
#include "pr/pr_psx_graph_owner_direct.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>

namespace {

using namespace PrSS0DirectoryPagesRenderDirect;
namespace EventBackdrop = PrSS0EventBackdropRenderDirect;

constexpr uint16_t kEventBackdropTestPriority = 0xBEEFu;

static_assert(sizeof(OptionsSpriteCommand80021910) ==
              sizeof(MainDirectorySpriteCommand80021E60));
static_assert(alignof(OptionsSpriteCommand80021910) ==
              alignof(MainDirectorySpriteCommand80021E60));
static_assert(offsetof(OptionsSpriteCommand80021910, known) ==
              offsetof(MainDirectorySpriteCommand80021E60, known));
static_assert(offsetof(OptionsSpriteCommand80021910, x) ==
              offsetof(MainDirectorySpriteCommand80021E60, x));
static_assert(offsetof(OptionsSpriteCommand80021910, y) ==
              offsetof(MainDirectorySpriteCommand80021E60, y));
static_assert(offsetof(OptionsSpriteCommand80021910, priority) ==
              offsetof(MainDirectorySpriteCommand80021E60, priority));
static_assert(offsetof(OptionsSpriteCommand80021910, callOrder) ==
              offsetof(MainDirectorySpriteCommand80021E60, callOrder));
static_assert(offsetof(OptionsSpriteCommand80021910, sprite) ==
              offsetof(MainDirectorySpriteCommand80021E60, sprite));

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,  \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

void CheckAcceptedCommands(const MainDirectoryDrawList80021E60& list,
                           bool expectedComplete) {
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete == expectedComplete);
    CHECK(list.rawTextureOnly);
    CHECK(!list.truncated);
    const uint32_t expectedCount =
        kMainDirectoryBaseSpriteCount80021E60 +
        (list.choice2ReplayThenLoadFallthrough
             ? kMainDirectoryChoice2FallthroughSpriteCount80021E60
             : 0u);
    CHECK(list.count == expectedCount);
    for (uint32_t index = 0; index < list.count; ++index) {
        const auto& command = list.commands[index];
        CHECK(command.known);
        CHECK(command.sprite.known);
        CHECK(command.callOrder == index);
        CHECK(command.sprite.attr == 0x50000040u);
        CHECK((command.sprite.attr & 0x40u) != 0u);
    }
}

void CheckCommand(const MainDirectoryDrawList80021E60& list,
                  uint32_t index,
                  uint32_t address,
                  int16_t x,
                  int16_t y,
                  uint16_t priority) {
    CHECK(index < list.count);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.sprite.psxAddress == address);
    CHECK(command.x == x);
    CHECK(command.y == y);
    CHECK(command.priority == priority);
    CHECK(command.callOrder == index);
}

void CheckBlinkSwitch(MainDirectoryState80021E60 state,
                      const uint32_t* commandIndices,
                      std::size_t commandCount);

void CheckFailClosed(const MainDirectoryState80021E60& state) {
    const auto list = BuildMainDirectoryDrawList80021E60(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);
    CHECK(!list.complete);
    CHECK(!list.truncated);
    CHECK(list.count == 0u);
    CHECK(!list.cardIoOverlay80020A3CRequired);
    CHECK(!list.blockedByCardIoOverlay80020A3C);
    CHECK(!list.cardIoOverlayInsertIndexKnown);
}

struct ExpectedOptionsFamily {
    uint32_t address;
    int16_t x;
    int16_t y;
    uint16_t texX;
    uint16_t texY;
    uint16_t width;
    uint16_t height;
    uint16_t clutX;
    uint16_t firstClutY;
};

constexpr ExpectedOptionsFamily kOptionsTitleLanguageLabel = {
    0x80050F20u, 41, 64, 320u, 267u, 60u, 12u, 960u, 11u,
};
constexpr ExpectedOptionsFamily kOptionsTitlePanel = {
    0x80050AE0u, 36, 45, 482u, 0u, 72u, 49u, 976u, 0u,
};
constexpr ExpectedOptionsFamily kOptionsSubtitleOn[5] = {
    {0x80050AF0u, 220, 49, 384u, 356u, 48u, 19u, 960u, 23u},
    {0x80050B50u, 219, 50, 396u, 356u, 48u, 18u, 960u, 23u},
    {0x80050B20u, 218, 50, 408u, 356u, 52u, 18u, 960u, 23u},
    {0x80050B80u, 218, 50, 421u, 356u, 52u, 18u, 960u, 23u},
    {0x80050BB0u, 220, 48, 434u, 356u, 48u, 20u, 960u, 23u},
};
constexpr ExpectedOptionsFamily kOptionsSubtitleOff[5] = {
    {0x80050BE0u, 220, 72, 384u, 376u, 48u, 20u, 960u, 29u},
    {0x80050C40u, 219, 73, 396u, 376u, 48u, 18u, 960u, 29u},
    {0x80050C10u, 218, 73, 408u, 376u, 52u, 18u, 960u, 29u},
    {0x80050C70u, 218, 73, 421u, 376u, 52u, 18u, 960u, 29u},
    {0x80050CA0u, 220, 71, 434u, 376u, 48u, 21u, 960u, 29u},
};
constexpr ExpectedOptionsFamily kOptionsLanguageA[5] = {
    {0x80050D00u, 42, 127, 384u, 256u, 40u, 26u, 960u, 35u},
    {0x80050D80u, 89, 146, 384u, 282u, 44u, 18u, 960u, 23u},
    {0x80050DE0u, 138, 153, 384u, 300u, 44u, 16u, 960u, 41u},
    {0x80050E60u, 187, 150, 384u, 316u, 44u, 14u, 960u, 53u},
    {0x80050EC0u, 235, 127, 384u, 330u, 44u, 26u, 960u, 47u},
};
constexpr ExpectedOptionsFamily kOptionsLanguageB[5] = {
    {0x80050CD0u, 33, 115, 482u, 49u, 60u, 52u, 976u, 1u},
    {0x80050D50u, 81, 133, 482u, 101u, 56u, 44u, 976u, 4u},
    {0x80050DB0u, 134, 142, 482u, 145u, 52u, 37u, 976u, 7u},
    {0x80050E30u, 183, 133, 497u, 49u, 56u, 45u, 976u, 10u},
    {0x80050E90u, 228, 115, 496u, 101u, 60u, 52u, 976u, 13u},
};
constexpr ExpectedOptionsFamily kOptionsDecoA = {
    0x80051170u, 216, 47, 320u, 45u, 68u, 22u, 960u, 20u,
};
constexpr ExpectedOptionsFamily kOptionsDecoB = {
    0x800512B0u, 216, 71, 320u, 67u, 68u, 22u, 960u, 26u,
};
constexpr ExpectedOptionsFamily kOptionsTopRightPanel = {
    0x80051290u, 207, 34, 384u, 68u, 88u, 67u, 960u, 0u,
};
constexpr ExpectedOptionsFamily kOptionsLeftPlate = {
    0x80050D30u, 26, 106, 448u, 0u, 136u, 82u, 960u, 0u,
};
constexpr ExpectedOptionsFamily kOptionsRightPlate = {
    0x80050E10u, 162, 106, 448u, 82u, 136u, 82u, 960u, 0u,
};
constexpr ExpectedOptionsFamily kOptionsExitFrame = {
    0x80050AC0u, 231, 179, 384u, 0u, 64u, 37u, 960u, 0u,
};
constexpr ExpectedOptionsFamily kOptionsExitText[5] = {
    {0x800509D0u, 242, 191, 320u, 256u, 28u, 11u, 960u, 5u},
    {0x80050A30u, 238, 193, 335u, 256u, 32u, 8u, 960u, 5u},
    {0x80050A00u, 238, 193, 327u, 256u, 32u, 8u, 960u, 5u},
    {0x80050A60u, 241, 190, 343u, 256u, 28u, 12u, 960u, 5u},
    {0x80050A90u, 239, 191, 350u, 256u, 32u, 10u, 960u, 5u},
};
constexpr ExpectedOptionsFamily kOptionsExitBar = {
    0x800509A0u, 238, 188, 320u, 0u, 48u, 17u, 960u, 2u,
};

void CheckAcceptedOptionsCommands(const OptionsDrawList80021910& list) {
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(!list.truncated);
    CHECK(list.count == kOptionsSpriteCapacity80021910);
    for (uint32_t index = 0; index < list.count; ++index) {
        const auto& command = list.commands[index];
        CHECK(command.known);
        CHECK(command.sprite.known);
        CHECK(command.callOrder == index);
        CHECK(command.priority == (index < 2u ? 1u : 0u));
        CHECK(command.sprite.attr == 0x50000040u);
        CHECK((command.sprite.attr & 0x40u) != 0u);
    }
}

void CheckOptionsCommand(const OptionsDrawList80021910& list,
                         uint32_t index,
                         const ExpectedOptionsFamily& family,
                         uint32_t state) {
    CHECK(index < list.count);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.sprite.psxAddress == family.address + state * 0x10u);
    CHECK(command.x == family.x);
    CHECK(command.y == family.y);
    CHECK(command.priority == (index < 2u ? 1u : 0u));
    CHECK(command.callOrder == index);
    CHECK(command.sprite.attr == 0x50000040u);
    CHECK(command.sprite.texX == family.texX);
    CHECK(command.sprite.texY == family.texY);
    CHECK(command.sprite.width == family.width);
    CHECK(command.sprite.height == family.height);
    CHECK(command.sprite.clutX == family.clutX);
    CHECK(command.sprite.clutY == family.firstClutY + state);
}

void CheckOptionsFailClosed(const OptionsState80021910& state) {
    const auto list = BuildOptionsDrawList80021910(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);
    CHECK(!list.complete);
    CHECK(!list.truncated);
    CHECK(list.count == 0u);
}

void CheckAcceptedStageSelectCommands(
    const StageSelectDrawList80020568& list,
    uint32_t expectedCount,
    bool expectedBonusOverlay) {
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(!list.truncated);
    CHECK(list.bonusOverlayPresent == expectedBonusOverlay);
    CHECK(list.count == expectedCount);
    for (uint32_t index = 0; index < list.count; ++index) {
        const auto& command = list.commands[index];
        CHECK(command.known);
        CHECK(command.sprite.known);
        CHECK(command.callOrder == index);
        CHECK((command.sprite.attr & 0x40u) != 0u);
        CHECK(command.priority == (index + 1u == list.count ? 1u : 0u));
    }
}

void CheckStageSelectCommand(const StageSelectDrawList80020568& list,
                             uint32_t index,
                             uint32_t address,
                             int16_t x,
                             int16_t y,
                             uint16_t priority = 0u) {
    CHECK(index < list.count);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.sprite.psxAddress == address);
    CHECK(command.x == x);
    CHECK(command.y == y);
    CHECK(command.priority == priority);
    CHECK(command.callOrder == index);
}

void CheckStageSelectSlice(const StageSelectDrawList80020568& list,
                           uint32_t index,
                           int16_t x,
                           int16_t y,
                           uint16_t width,
                           uint16_t clutY,
                           uint8_t u) {
    CheckStageSelectCommand(list, index, 0x80051BF0u, x, y);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.textureCoordinatesResolved);
    CHECK(command.sprite.attr == 0x50000040u);
    CHECK(command.sprite.texX == 606u);
    CHECK(command.sprite.texY == 208u);
    CHECK(command.sprite.width == width);
    CHECK(command.sprite.height == 10u);
    CHECK(command.sprite.clutX == 1008u);
    CHECK(command.sprite.clutY == clutY);
    CHECK(command.tpage == 0x29u);
    CHECK(command.u == u);
    CHECK(command.v == 208u);
}

void CheckStageSelectFailClosed(const StageSelectState80020568& state) {
    const auto list = BuildStageSelectDrawList80020568(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);
    CHECK(!list.complete);
    CHECK(!list.truncated);
    CHECK(!list.bonusOverlayPresent);
    CHECK(list.count == 0u);
}

void TestContextNullProducesKnownCommands() {
    MainDirectoryState80021E60 state{};
    state.contextPresent = false;
    state.language = 4;

    const auto list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    for (uint32_t index = 0; index < list.count; ++index) {
        CHECK(list.commands[index].priority == 1u);
    }

    CheckCommand(list, 0u, 0x80051010u, 28, 36, 1u);
    CHECK(list.commands[0].sprite.texX == 400u);
    CHECK(list.commands[0].sprite.texY == 0u);
    CHECK(list.commands[0].sprite.width == 88u);
    CHECK(list.commands[0].sprite.height == 68u);
    CHECK(list.commands[0].sprite.clutX == 960u);
    CHECK(list.commands[0].sprite.clutY == 0u);
    CheckCommand(list, 8u, 0x80050F20u, 41, 64, 1u);
    CheckCommand(list, 9u, 0x80051120u, 128, 71, 1u);
    CheckCommand(list, 25u, 0x800509A0u, 238, 188, 1u);
}

void TestDefaultCursor3DrawList() {
    const MainDirectoryState80021E60 state{};
    const auto list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(!list.cardIoOverlay80020A3CRequired);
    CHECK(!list.blockedByCardIoOverlay80020A3C);

    CheckCommand(list, 0u, 0x80050F20u, 41, 64, 0u);
    CHECK(list.commands[0].sprite.texX == 320u);
    CHECK(list.commands[0].sprite.texY == 267u);
    CHECK(list.commands[0].sprite.width == 60u);
    CHECK(list.commands[0].sprite.height == 12u);
    CHECK(list.commands[0].sprite.clutX == 960u);
    CHECK(list.commands[0].sprite.clutY == 11u);
    CheckCommand(list, 2u, 0x80051010u, 28, 36, 1u);
    CheckCommand(list, 11u, 0x80051410u, 41, 127, 0u);
    CheckCommand(list, 19u, 0x800514F0u, 26, 106, 0u);
    CheckCommand(list, 25u, 0x800509A0u, 238, 188, 0u);
}

void TestCursorAndChoiceVariants() {
    MainDirectoryState80021E60 state{};

    state.cursor = 0;
    state.itemValue[0] = 0;
    auto list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[0].sprite.psxAddress == 0x80050F30u);
    CHECK(list.commands[1].sprite.psxAddress == 0x80050F00u);
    state.itemValue[0] = 1;
    list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[0].sprite.psxAddress == 0x80050F40u);
    CHECK(list.commands[1].sprite.psxAddress == 0x80050F10u);

    state = MainDirectoryState80021E60{};
    state.cursor = 1;
    state.itemValue[1] = -1;
    list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[3].sprite.psxAddress == 0x80051070u);
    CHECK(list.commands[4].sprite.psxAddress == 0x80051040u);

    state = MainDirectoryState80021E60{};
    state.cursor = 2;
    state.itemValue[2] = 0;
    list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[6].sprite.psxAddress == 0x800511C0u);
    CHECK(list.commands[7].sprite.psxAddress == 0x800512F0u);
    CHECK(list.commands[8].sprite.psxAddress == 0x80051190u);
    CHECK(list.commands[9].sprite.psxAddress == 0x800512C0u);
    state.itemValue[2] = 1;
    list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[6].sprite.psxAddress == 0x800511B0u);
    CHECK(list.commands[7].sprite.psxAddress == 0x80051300u);
    CHECK(list.commands[8].sprite.psxAddress == 0x80051180u);
    CHECK(list.commands[9].sprite.psxAddress == 0x800512D0u);

    constexpr uint32_t kChoiceTextBase[] = {
        0x80051400u,
        0x80051540u,
        0x80051680u,
        0x800517C0u,
    };
    constexpr uint32_t kChoicePlateBase[] = {
        0x800513D0u,
        0x80051510u,
        0x80051650u,
        0x80051790u,
    };
    for (int32_t choice = -1; choice <= 3; ++choice) {
        state = MainDirectoryState80021E60{};
        state.cursor = 3;
        state.itemValue[3] = choice;
        list = BuildMainDirectoryDrawList80021E60(state);
        CheckAcceptedCommands(list, true);
        const uint32_t firstSelectedOption =
            choice == 0 || choice == 1 || choice == 2
                ? static_cast<uint32_t>(choice)
                : (choice == 3 ? 3u : 4u);
        for (uint32_t option = 0; option < 4u; ++option) {
            const uint32_t templateState =
                firstSelectedOption == option ? 2u : 1u;
            CHECK(list.commands[11u + option].sprite.psxAddress ==
                  kChoiceTextBase[option] + templateState * 0x10u);
            CHECK(list.commands[15u + option].sprite.psxAddress ==
                  kChoicePlateBase[option] + templateState * 0x10u);
        }
        CHECK(list.choice2ReplayThenLoadFallthrough == (choice == 2));
        if (choice == 2) {
            CHECK(list.count == kMainDirectorySpriteCapacity80021E60);
            for (uint32_t option = 0; option < 4u; ++option) {
                const uint32_t templateState = option == 3u ? 2u : 1u;
                CHECK(list.commands[19u + option].sprite.psxAddress ==
                      kChoiceTextBase[option] + templateState * 0x10u);
                CHECK(list.commands[23u + option].sprite.psxAddress ==
                      kChoicePlateBase[option] + templateState * 0x10u);
            }
            CheckCommand(list, 27u, 0x800514F0u, 26, 106, 0u);
            CheckCommand(list, 33u, 0x800509A0u, 238, 188, 0u);
        }
    }

    state = MainDirectoryState80021E60{};
    state.cursor = 4;
    state.exitConfirmed = false;
    list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[24].sprite.psxAddress == 0x800509E0u);
    CHECK(list.commands[25].sprite.psxAddress == 0x800509B0u);
    state.exitConfirmed = true;
    list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[24].sprite.psxAddress == 0x800509F0u);
    CHECK(list.commands[25].sprite.psxAddress == 0x800509C0u);
}

void TestReplayChoice2FallsThroughToLoad() {
    MainDirectoryState80021E60 state{};
    state.cursor = 3;
    state.itemValue[3] = 2;

    const auto list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.choice2ReplayThenLoadFallthrough);
    CHECK(list.count == 34u);

    constexpr uint32_t kReplaySelectedText[] = {
        0x80051410u, 0x80051550u, 0x800516A0u, 0x800517D0u,
    };
    constexpr uint32_t kReplaySelectedPlate[] = {
        0x800513E0u, 0x80051520u, 0x80051670u, 0x800517A0u,
    };
    constexpr uint32_t kLoadSelectedText[] = {
        0x80051410u, 0x80051550u, 0x80051690u, 0x800517E0u,
    };
    constexpr uint32_t kLoadSelectedPlate[] = {
        0x800513E0u, 0x80051520u, 0x80051660u, 0x800517B0u,
    };
    for (uint32_t option = 0; option < 4u; ++option) {
        CHECK(list.commands[11u + option].sprite.psxAddress ==
              kReplaySelectedText[option]);
        CHECK(list.commands[15u + option].sprite.psxAddress ==
              kReplaySelectedPlate[option]);
        CHECK(list.commands[19u + option].sprite.psxAddress ==
              kLoadSelectedText[option]);
        CHECK(list.commands[23u + option].sprite.psxAddress ==
              kLoadSelectedPlate[option]);
    }
    CheckCommand(list, 27u, 0x800514F0u, 26, 106, 0u);
    CheckCommand(list, 28u, 0x80051630u, 94, 122, 0u);
    CheckCommand(list, 29u, 0x80051770u, 162, 122, 0u);
    CheckCommand(list, 30u, 0x800518B0u, 226, 106, 0u);
    CheckCommand(list, 31u, 0x80050AC0u, 231, 179, 0u);
    CheckCommand(list, 32u, 0x800509D0u, 242, 191, 0u);
    CheckCommand(list, 33u, 0x800509A0u, 238, 188, 0u);

    constexpr uint32_t kChoice2Blink[] = {27u, 28u, 29u, 30u};
    CheckBlinkSwitch(state, kChoice2Blink, 4u);
}

void TestFiveLanguageBoundaryAndFixedLanguageLabel() {
    constexpr uint32_t kLanguageIndexedTextBase[5][8] = {
        {0x80051060u, 0x800511A0u, 0x800512E0u, 0x80051400u,
         0x80051540u, 0x80051680u, 0x800517C0u, 0x800509D0u},
        {0x800510C0u, 0x80051200u, 0x80051340u, 0x80051460u,
         0x800515A0u, 0x800516E0u, 0x80051820u, 0x80050A30u},
        {0x80051090u, 0x800511D0u, 0x80051310u, 0x80051430u,
         0x80051570u, 0x800516B0u, 0x800517F0u, 0x80050A00u},
        {0x800510F0u, 0x80051230u, 0x80051370u, 0x80051490u,
         0x800515D0u, 0x80051710u, 0x80051850u, 0x80050A60u},
        {0x80051120u, 0x80051260u, 0x800513A0u, 0x800514C0u,
         0x80051600u, 0x80051740u, 0x80051880u, 0x80050A90u},
    };
    constexpr uint32_t kCommandIndex[8] = {4u, 7u, 8u, 11u,
                                            12u, 13u, 14u, 24u};
    constexpr uint32_t kSelectedStateOffset[8] = {0u, 0u, 0u, 0x10u,
                                                   0x10u, 0x10u, 0x10u, 0u};

    for (int32_t language = 0; language < 5; ++language) {
        MainDirectoryState80021E60 state{};
        state.language = language;
        const auto list = BuildMainDirectoryDrawList80021E60(state);
        CheckAcceptedCommands(list, true);

        // 80021E60 always uses the first 80053554 language-label record.
        CheckCommand(list, 0u, 0x80050F20u, 41, 64, 0u);
        for (uint32_t text = 0; text < 8u; ++text) {
            CHECK(list.commands[kCommandIndex[text]].sprite.psxAddress ==
                  kLanguageIndexedTextBase[language][text] +
                      kSelectedStateOffset[text]);
        }
    }
}

void CheckBlinkSwitch(MainDirectoryState80021E60 state,
                      const uint32_t* commandIndices,
                      std::size_t commandCount) {
    state.blinkOnOff = 0;
    const auto off = BuildMainDirectoryDrawList80021E60(state);
    state.blinkOnOff = 1;
    const auto on = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(off, true);
    CheckAcceptedCommands(on, true);

    for (std::size_t index = 0; index < commandCount; ++index) {
        const uint32_t commandIndex = commandIndices[index];
        const auto& offCommand = off.commands[commandIndex];
        const auto& onCommand = on.commands[commandIndex];
        CHECK(onCommand.sprite.psxAddress ==
              offCommand.sprite.psxAddress + 0x10u);
        CHECK(onCommand.sprite.clutY == offCommand.sprite.clutY + 1u);
        CHECK(onCommand.x == offCommand.x);
        CHECK(onCommand.y == offCommand.y);
        CHECK(onCommand.priority == offCommand.priority);
        CHECK(onCommand.callOrder == offCommand.callOrder);
    }
}

void TestBlinkTemplateSwitches() {
    MainDirectoryState80021E60 state{};
    constexpr uint32_t kCursor0[] = {2u};
    constexpr uint32_t kCursor1[] = {5u};
    constexpr uint32_t kCursor2[] = {10u};
    constexpr uint32_t kCursor3[] = {19u, 20u, 21u, 22u};
    constexpr uint32_t kCursor4[] = {23u};

    state.cursor = 0;
    CheckBlinkSwitch(state, kCursor0, 1u);
    state = MainDirectoryState80021E60{};
    state.cursor = 1;
    CheckBlinkSwitch(state, kCursor1, 1u);
    state = MainDirectoryState80021E60{};
    state.cursor = 2;
    CheckBlinkSwitch(state, kCursor2, 1u);
    state = MainDirectoryState80021E60{};
    state.cursor = 3;
    CheckBlinkSwitch(state, kCursor3, 4u);
    state = MainDirectoryState80021E60{};
    state.cursor = 4;
    CheckBlinkSwitch(state, kCursor4, 1u);
}

void TestInvalidLanguageFailsClosed() {
    MainDirectoryState80021E60 state{};

    state.language = -1;
    CheckFailClosed(state);
    state = MainDirectoryState80021E60{};
    state.language = 5;
    CheckFailClosed(state);

    state = MainDirectoryState80021E60{};
    state.cursor = 4;
    state.itemValue[4] = -1;
    CheckAcceptedCommands(BuildMainDirectoryDrawList80021E60(state), true);
    state.itemValue[4] = 1;
    CheckAcceptedCommands(BuildMainDirectoryDrawList80021E60(state), true);

    state = MainDirectoryState80021E60{};
    state.contextPresent = false;
    state.blinkOnOff = 99;
    CheckAcceptedCommands(BuildMainDirectoryDrawList80021E60(state), true);
}

void TestNoncanonicalMainDirectoryStateUsesOriginalPredicates() {
    constexpr uint32_t kBaseBlinkCommands[] = {
        2u, 3u, 6u, 11u, 12u, 13u, 14u, 23u,
    };
    constexpr uint32_t kBaseBlinkTemplates[] = {
        0x80051010u, 0x80051150u, 0x80051290u, 0x800514F0u,
        0x80051630u, 0x80051770u, 0x800518B0u, 0x80050AC0u,
    };

    for (int32_t cursor : {
             static_cast<int32_t>(std::numeric_limits<int16_t>::min()), -1, 5,
             static_cast<int32_t>(std::numeric_limits<int16_t>::max()),
         }) {
        MainDirectoryState80021E60 state{};
        state.cursor = cursor;
        state.blinkOnOff = 1;
        const auto list = BuildMainDirectoryDrawList80021E60(state);
        CheckAcceptedCommands(list, true);
        CHECK(!list.cardIoOverlay80020A3CRequired);
        CHECK(!list.choice2ReplayThenLoadFallthrough);
        for (std::size_t index = 0; index < std::size(kBaseBlinkCommands);
             ++index) {
            CHECK(list.commands[kBaseBlinkCommands[index]].sprite.psxAddress ==
                  kBaseBlinkTemplates[index]);
        }
        CHECK(list.commands[24].sprite.psxAddress == 0x800509D0u);
        CHECK(list.commands[25].sprite.psxAddress == 0x800509A0u);
    }

    for (int32_t blink : {
             std::numeric_limits<int32_t>::min(), -1, 0, 2,
             std::numeric_limits<int32_t>::max(),
         }) {
        MainDirectoryState80021E60 state{};
        state.cursor = 0;
        state.blinkOnOff = blink;
        const auto list = BuildMainDirectoryDrawList80021E60(state);
        CheckAcceptedCommands(list, true);
        CHECK(list.commands[2].sprite.psxAddress == 0x80051010u);
    }

    MainDirectoryState80021E60 state{};
    state.cursor = 0;
    state.blinkOnOff = 1;
    const auto exactOne = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(exactOne, true);
    CHECK(exactOne.commands[2].sprite.psxAddress == 0x80051020u);
}

void TestItemValuesUsePsxPredicates() {
    MainDirectoryState80021E60 state{};
    state.cursor = 0;
    state.itemValue[0] = -7;
    auto list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[0].sprite.psxAddress == 0x80050F40u);

    state = MainDirectoryState80021E60{};
    state.cursor = 1;
    state.itemValue[1] = 7;
    list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, false);
    CHECK(list.cardIoOverlay80020A3CRequired);
    CHECK(list.cardIoOverlayInsertIndexKnown);
    CHECK(list.cardIoOverlayInsertIndex == 5u);

    state = MainDirectoryState80021E60{};
    state.cursor = 2;
    state.itemValue[2] = -7;
    list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, true);
    CHECK(list.commands[6].sprite.psxAddress == 0x800511B0u);
    CHECK(list.commands[7].sprite.psxAddress == 0x80051300u);

    for (int32_t choice : {-2, 4}) {
        state = MainDirectoryState80021E60{};
        state.cursor = 3;
        state.itemValue[3] = choice;
        list = BuildMainDirectoryDrawList80021E60(state);
        CheckAcceptedCommands(list, true);
        CHECK(!list.choice2ReplayThenLoadFallthrough);
        CHECK(list.commands[11].sprite.psxAddress == 0x80051410u);
        CHECK(list.commands[12].sprite.psxAddress == 0x80051550u);
        CHECK(list.commands[13].sprite.psxAddress == 0x80051690u);
        CHECK(list.commands[14].sprite.psxAddress == 0x800517D0u);
    }
}

void TestHiScoreCardIoOverlayGap() {
    MainDirectoryState80021E60 state{};
    state.cursor = 1;
    state.itemValue[1] = 0;

    const auto list = BuildMainDirectoryDrawList80021E60(state);
    CheckAcceptedCommands(list, false);
    CHECK(list.cardIoOverlay80020A3CRequired);
    CHECK(list.blockedByCardIoOverlay80020A3C);
    CHECK(list.cardIoOverlayInsertIndexKnown);
    CHECK(list.cardIoOverlayInsertIndex == 5u);
    CHECK(list.commands[3].sprite.psxAddress == 0x80051080u);
    CHECK(list.commands[4].sprite.psxAddress == 0x80051050u);
}

void TestOptionsExactOrderAndCount() {
    const OptionsState80021910 state{};
    const auto list = BuildOptionsDrawList80021910(state);
    CheckAcceptedOptionsCommands(list);

    CheckOptionsCommand(list, 0u, kOptionsTitleLanguageLabel, 2u);
    CheckOptionsCommand(list, 1u, kOptionsTitlePanel, 0u);
    CheckOptionsCommand(list, 2u, kOptionsSubtitleOn[0], 0u);
    CheckOptionsCommand(list, 3u, kOptionsSubtitleOff[0], 0u);
    CheckOptionsCommand(list, 4u, kOptionsDecoA, 0u);
    CheckOptionsCommand(list, 5u, kOptionsDecoB, 0u);
    CheckOptionsCommand(list, 6u, kOptionsTopRightPanel, 0u);
    for (uint32_t language = 0; language < 5u; ++language) {
        const uint32_t stateIndex = language == 0u ? 2u : 1u;
        CheckOptionsCommand(
            list, 7u + language * 2u, kOptionsLanguageA[language],
            stateIndex);
        CheckOptionsCommand(
            list, 8u + language * 2u, kOptionsLanguageB[language],
            stateIndex);
    }
    CheckOptionsCommand(list, 17u, kOptionsLeftPlate, 0u);
    CheckOptionsCommand(list, 18u, kOptionsRightPlate, 0u);
    CheckOptionsCommand(list, 19u, kOptionsExitFrame, 0u);
    CheckOptionsCommand(list, 20u, kOptionsExitText[0], 0u);
    CheckOptionsCommand(list, 21u, kOptionsExitBar, 0u);
}

void TestOptionsCursor0AllLanguagesAndSubtitleStates() {
    for (int32_t language = 0; language < 5; ++language) {
        for (int32_t subtitleValue = 0; subtitleValue < 2;
             ++subtitleValue) {
            for (int32_t blink = 0; blink < 2; ++blink) {
                OptionsState80021910 state{};
                state.language = language;
                state.blinkOnOff = blink;
                state.cursor = 0;
                state.opt0Value = subtitleValue;
                state.opt1Value = (language + 1) % 5;

                const auto list = BuildOptionsDrawList80021910(state);
                CheckAcceptedOptionsCommands(list);
                const bool subtitleBlinkActive = subtitleValue == 0;
                const uint32_t onState = subtitleBlinkActive
                    ? (blink != 0 ? 1u : 2u)
                    : 0u;
                const uint32_t offState = subtitleBlinkActive
                    ? (blink != 0 ? 2u : 1u)
                    : 0u;
                CheckOptionsCommand(
                    list, 2u, kOptionsSubtitleOn[language], onState);
                CheckOptionsCommand(
                    list, 3u, kOptionsSubtitleOff[language], offState);
                CheckOptionsCommand(list, 4u, kOptionsDecoA, onState);
                CheckOptionsCommand(list, 5u, kOptionsDecoB, offState);
                CheckOptionsCommand(
                    list, 6u, kOptionsTopRightPanel,
                    subtitleBlinkActive ? static_cast<uint32_t>(blink) : 0u);
                for (uint32_t index = 0; index < 5u; ++index) {
                    CheckOptionsCommand(
                        list, 7u + index * 2u, kOptionsLanguageA[index], 0u);
                    CheckOptionsCommand(
                        list, 8u + index * 2u, kOptionsLanguageB[index], 0u);
                }
                CheckOptionsCommand(list, 17u, kOptionsLeftPlate, 0u);
                CheckOptionsCommand(list, 18u, kOptionsRightPlate, 0u);
                CheckOptionsCommand(list, 19u, kOptionsExitFrame, 0u);
                CheckOptionsCommand(
                    list, 20u, kOptionsExitText[language], 0u);
                CheckOptionsCommand(list, 21u, kOptionsExitBar, 0u);
            }
        }
    }
}

void TestOptionsCursor1AllSelectedLanguages() {
    for (int32_t selectedLanguage = 0; selectedLanguage < 5;
         ++selectedLanguage) {
        for (int32_t blink = 0; blink < 2; ++blink) {
            OptionsState80021910 state{};
            state.language = (selectedLanguage + 2) % 5;
            state.blinkOnOff = blink;
            state.cursor = 1;
            state.opt1Value = selectedLanguage;

            const auto list = BuildOptionsDrawList80021910(state);
            CheckAcceptedOptionsCommands(list);
            CheckOptionsCommand(
                list, 2u, kOptionsSubtitleOn[state.language], 0u);
            CheckOptionsCommand(
                list, 3u, kOptionsSubtitleOff[state.language], 0u);
            CheckOptionsCommand(list, 4u, kOptionsDecoA, 0u);
            CheckOptionsCommand(list, 5u, kOptionsDecoB, 0u);
            CheckOptionsCommand(list, 6u, kOptionsTopRightPanel, 0u);
            for (uint32_t index = 0; index < 5u; ++index) {
                const uint32_t stateIndex =
                    index == static_cast<uint32_t>(selectedLanguage)
                        ? 2u
                        : 1u;
                CheckOptionsCommand(
                    list, 7u + index * 2u, kOptionsLanguageA[index],
                    stateIndex);
                CheckOptionsCommand(
                    list, 8u + index * 2u, kOptionsLanguageB[index],
                    stateIndex);
            }
            CheckOptionsCommand(
                list, 17u, kOptionsLeftPlate,
                static_cast<uint32_t>(blink));
            CheckOptionsCommand(
                list, 18u, kOptionsRightPlate,
                static_cast<uint32_t>(blink));
            CheckOptionsCommand(list, 19u, kOptionsExitFrame, 0u);
            CheckOptionsCommand(
                list, 20u, kOptionsExitText[state.language], 0u);
            CheckOptionsCommand(list, 21u, kOptionsExitBar, 0u);
        }
    }
}

void TestOptionsCursor2ExitStates() {
    for (int32_t language = 0; language < 5; ++language) {
        for (int32_t done = 0; done < 2; ++done) {
            for (int32_t blink = 0; blink < 2; ++blink) {
                OptionsState80021910 state{};
                state.language = language;
                state.blinkOnOff = blink;
                state.doneFlag = done;
                state.cursor = 2;
                state.opt1Value = (language + 1) % 5;

                const auto list = BuildOptionsDrawList80021910(state);
                CheckAcceptedOptionsCommands(list);
                CheckOptionsCommand(
                    list, 2u, kOptionsSubtitleOn[language], 0u);
                CheckOptionsCommand(
                    list, 3u, kOptionsSubtitleOff[language], 0u);
                CheckOptionsCommand(list, 4u, kOptionsDecoA, 0u);
                CheckOptionsCommand(list, 5u, kOptionsDecoB, 0u);
                CheckOptionsCommand(list, 6u, kOptionsTopRightPanel, 0u);
                for (uint32_t index = 0; index < 5u; ++index) {
                    CheckOptionsCommand(
                        list, 7u + index * 2u, kOptionsLanguageA[index], 0u);
                    CheckOptionsCommand(
                        list, 8u + index * 2u, kOptionsLanguageB[index], 0u);
                }
                CheckOptionsCommand(list, 17u, kOptionsLeftPlate, 0u);
                CheckOptionsCommand(list, 18u, kOptionsRightPlate, 0u);
                CheckOptionsCommand(
                    list, 19u, kOptionsExitFrame,
                    static_cast<uint32_t>(blink));
                const uint32_t exitState = done == 0 ? 1u : 2u;
                CheckOptionsCommand(
                    list, 20u, kOptionsExitText[language], exitState);
                CheckOptionsCommand(list, 21u, kOptionsExitBar, exitState);
            }
        }
    }
}

void TestOptionsInvalidLanguageFailsClosed() {
    OptionsState80021910 state{};

    state.language = -1;
    CheckOptionsFailClosed(state);
    state = OptionsState80021910{};
    state.language = 5;
    CheckOptionsFailClosed(state);
}

void TestOptionsNoncanonicalStateUsesOriginalPredicates() {
    OptionsState80021910 state{};
    state.language = 2;
    state.blinkOnOff = -1;
    state.doneFlag = -1;
    state.cursor = -1;
    state.opt0Value = -1;
    state.opt1Value = -1;
    state.opt2Value = std::numeric_limits<int32_t>::min();
    auto list = BuildOptionsDrawList80021910(state);
    CheckAcceptedOptionsCommands(list);
    CheckOptionsCommand(list, 2u, kOptionsSubtitleOn[2], 0u);
    CheckOptionsCommand(list, 3u, kOptionsSubtitleOff[2], 0u);
    CheckOptionsCommand(list, 17u, kOptionsLeftPlate, 0u);
    CheckOptionsCommand(list, 18u, kOptionsRightPlate, 0u);
    CheckOptionsCommand(list, 19u, kOptionsExitFrame, 0u);
    CheckOptionsCommand(list, 20u, kOptionsExitText[2], 0u);
    CheckOptionsCommand(list, 21u, kOptionsExitBar, 0u);

    state.cursor = 0;
    state.blinkOnOff = std::numeric_limits<int32_t>::max();
    state.opt0Value = std::numeric_limits<int32_t>::max();
    state.opt2Value = std::numeric_limits<int32_t>::max();
    list = BuildOptionsDrawList80021910(state);
    CheckAcceptedOptionsCommands(list);
    CheckOptionsCommand(list, 2u, kOptionsSubtitleOn[2], 0u);
    CheckOptionsCommand(list, 3u, kOptionsSubtitleOff[2], 0u);
    CheckOptionsCommand(list, 6u, kOptionsTopRightPanel, 0u);

    state.cursor = 1;
    state.blinkOnOff = 1;
    state.opt1Value = 5;
    list = BuildOptionsDrawList80021910(state);
    CheckAcceptedOptionsCommands(list);
    for (uint32_t index = 0u; index < 5u; ++index) {
        CheckOptionsCommand(
            list, 7u + index * 2u, kOptionsLanguageA[index], 1u);
        CheckOptionsCommand(
            list, 8u + index * 2u, kOptionsLanguageB[index], 1u);
    }
    CheckOptionsCommand(list, 17u, kOptionsLeftPlate, 1u);
    CheckOptionsCommand(list, 18u, kOptionsRightPlate, 1u);

    state.cursor = 2;
    state.doneFlag = -1;
    list = BuildOptionsDrawList80021910(state);
    CheckAcceptedOptionsCommands(list);
    CheckOptionsCommand(list, 19u, kOptionsExitFrame, 1u);
    CheckOptionsCommand(list, 20u, kOptionsExitText[2], 2u);
    CheckOptionsCommand(list, 21u, kOptionsExitBar, 2u);
}

void TestStageSelectExactOrderAndSlices() {
    StageSelectState80020568 state{};
    state.cursor = 1;
    const uint8_t rawStatus[7] = {1u, 2u, 3u, 1u, 2u, 3u, 0u};
    for (uint32_t index = 0; index < 7u; ++index) {
        state.rawStatus80092F1DTo23[index] = rawStatus[index];
    }

    const auto list = BuildStageSelectDrawList80020568(state);
    CheckAcceptedStageSelectCommands(
        list, kStageSelectBaseSpriteCount80020568, false);
    CheckStageSelectCommand(list, 0u, 0x80051AB0u, 44, 36);
    CheckStageSelectCommand(list, 1u, 0x80051AA0u, 32, 33);
    CHECK(list.commands[0].sprite.texX == 448u);
    CHECK(list.commands[0].sprite.texY == 256u);
    CHECK(list.commands[0].sprite.width == 96u);
    CHECK(list.commands[0].sprite.height == 13u);
    CHECK(list.commands[1].sprite.texX == 576u);
    CHECK(list.commands[1].sprite.texY == 208u);
    CHECK(list.commands[1].sprite.width == 120u);
    CHECK(list.commands[1].sprite.height == 20u);

    constexpr int16_t kTopX[6] = {48, 141, 229, 48, 141, 229};
    constexpr int16_t kTopY[6] = {122, 106, 90, 199, 182, 166};
    constexpr int16_t kDotX[6] = {92, 183, 271, 90, 184, 271};
    constexpr int16_t kDotY[6] = {122, 106, 90, 199, 182, 166};
    constexpr uint16_t kSliceWidth[6] = {4u, 8u, 8u, 8u, 8u, 8u};
    constexpr uint8_t kSliceU[6] = {120u, 124u, 132u, 140u, 148u, 156u};
    constexpr int16_t kPanelX[6] = {37, 130, 218, 37, 130, 218};
    constexpr int16_t kPanelY[6] = {66, 50, 34, 143, 126, 110};
    constexpr int16_t kNameX[6] = {45, 138, 226, 45, 138, 226};
    constexpr int16_t kNameY[6] = {75, 59, 43, 152, 135, 119};
    constexpr int16_t kBadgeX[6] = {78, 171, 259, 78, 171, 259};
    constexpr int16_t kBadgeY[6] = {64, 48, 32, 141, 124, 108};
    constexpr uint32_t kBadgeA[6] = {
        0x800519B0u, 0x80051980u, 0x80051990u,
        0x80051970u, 0x80051980u, 0x80051990u,
    };
    for (uint32_t stage = 0; stage < 6u; ++stage) {
        const uint32_t base = 2u + stage * 6u;
        CheckStageSelectCommand(
            list, base, stage == 0u ? 0x80051B10u : 0x80051B00u,
            kTopX[stage], kTopY[stage]);
        CheckStageSelectSlice(
            list, base + 1u, kDotX[stage], kDotY[stage],
            kSliceWidth[stage], stage == 0u ? 32u : 31u,
            kSliceU[stage]);
        CheckStageSelectCommand(
            list, base + 2u,
            stage == 0u ? 0x800518E0u : 0x800518D0u,
            kPanelX[stage], kPanelY[stage]);
        CheckStageSelectCommand(
            list, base + 3u, 0x80051900u + stage * 0x10u,
            kNameX[stage], kNameY[stage]);
        CheckStageSelectCommand(
            list, base + 4u, kBadgeA[stage],
            kBadgeX[stage], kBadgeY[stage]);
        CheckStageSelectCommand(
            list, base + 5u,
            stage == 0u ? 0x800519A0u : 0x80051960u,
            kBadgeX[stage], kBadgeY[stage]);
    }
    CheckStageSelectCommand(list, 38u, 0x800509D0u, 242, 191);
    CheckStageSelectCommand(list, 39u, 0x800509A0u, 238, 188);
    CheckStageSelectCommand(list, 40u, 0x80050AC0u, 231, 179, 1u);
}

void TestStageSelectDisabledSelectedLanguage1() {
    StageSelectState80020568 state{};
    state.language = 1;
    state.cursor = 2;
    const auto list = BuildStageSelectDrawList80020568(state);
    CheckAcceptedStageSelectCommands(
        list, kStageSelectBaseSpriteCount80020568, false);
    CheckStageSelectCommand(list, 0u, 0x80051AD0u, 43, 35);

    constexpr int16_t kTopX[6] = {50, 143, 231, 50, 143, 231};
    constexpr int16_t kTopY[6] = {122, 106, 90, 199, 182, 166};
    constexpr int16_t kDotX[6] = {45, 135, 223, 42, 135, 223};
    constexpr int16_t kDotY[6] = {122, 106, 90, 199, 182, 166};
    constexpr uint16_t kSliceWidth[6] = {4u, 8u, 8u, 8u, 8u, 8u};
    constexpr uint8_t kSliceU[6] = {120u, 124u, 132u, 140u, 148u, 156u};
    constexpr int16_t kPanelX[6] = {37, 130, 218, 37, 130, 218};
    constexpr int16_t kPanelY[6] = {66, 50, 34, 143, 126, 110};
    constexpr int16_t kNameX[6] = {45, 138, 226, 45, 138, 226};
    constexpr int16_t kNameY[6] = {75, 59, 43, 152, 135, 119};
    constexpr int16_t kBadgeX[6] = {78, 171, 259, 78, 171, 259};
    constexpr int16_t kBadgeY[6] = {64, 48, 32, 141, 124, 108};
    for (uint32_t stage = 0; stage < 6u; ++stage) {
        const bool selected = stage == 1u;
        const uint32_t base = 2u + stage * 6u;
        CheckStageSelectCommand(
            list, base, selected ? 0x80051B70u : 0x80051B80u,
            kTopX[stage], kTopY[stage]);
        CheckStageSelectSlice(
            list, base + 1u, kDotX[stage], kDotY[stage],
            kSliceWidth[stage], selected ? 32u : 33u, kSliceU[stage]);
        CheckStageSelectCommand(
            list, base + 2u,
            selected ? 0x800518E0u : 0x800518F0u,
            kPanelX[stage], kPanelY[stage]);
        CheckStageSelectCommand(
            list, base + 3u, 0x80051A20u + stage * 0x10u,
            kNameX[stage], kNameY[stage]);
        CheckStageSelectCommand(
            list, base + 4u, selected ? 0x800519B0u : 0x80051A90u,
            kBadgeX[stage], kBadgeY[stage]);
        CheckStageSelectCommand(
            list, base + 5u, selected ? 0x800519A0u : 0x80051A80u,
            kBadgeX[stage], kBadgeY[stage]);
    }
    CheckStageSelectCommand(list, 38u, 0x80050A30u, 238, 193);
}

void TestStageSelectBonusAndExitStates() {
    StageSelectState80020568 state{};
    state.cursor = 7;
    for (uint32_t index = 0; index < 6u; ++index) {
        state.rawStatus80092F1DTo23[index] = 1u;
    }
    auto list = BuildStageSelectDrawList80020568(state);
    CheckAcceptedStageSelectCommands(
        list, kStageSelectSpriteCapacity80020568, true);
    CheckStageSelectCommand(list, 2u, 0x800519E0u, 63, 98);
    CheckStageSelectCommand(list, 3u, 0x80051A00u, 66, 103);
    CheckStageSelectCommand(list, 40u, 0x800509D0u, 242, 191);
    CheckStageSelectCommand(list, 42u, 0x80050AC0u, 231, 179, 1u);

    state.doneFlag = 1;
    list = BuildStageSelectDrawList80020568(state);
    CheckAcceptedStageSelectCommands(
        list, kStageSelectSpriteCapacity80020568, true);
    CheckStageSelectCommand(list, 2u, 0x800519F0u, 64, 100);

    state = StageSelectState80020568{};
    state.language = 4;
    state.cursor = 8;
    state.blinkOnOff = 1;
    state.rawStatus80092F1DTo23[6] = 1u;
    list = BuildStageSelectDrawList80020568(state);
    CheckAcceptedStageSelectCommands(
        list, kStageSelectSpriteCapacity80020568, true);
    CheckStageSelectCommand(list, 2u, 0x80051A10u, 63, 98);
    CheckStageSelectCommand(list, 3u, 0x80051A00u, 66, 103);
    CheckStageSelectCommand(list, 40u, 0x80050AA0u, 239, 191);
    CheckStageSelectCommand(list, 41u, 0x800509B0u, 238, 188);
    CheckStageSelectCommand(list, 42u, 0x80050AD0u, 231, 179, 1u);

    state.doneFlag = 1;
    list = BuildStageSelectDrawList80020568(state);
    CheckAcceptedStageSelectCommands(
        list, kStageSelectSpriteCapacity80020568, true);
    CheckStageSelectCommand(list, 40u, 0x80050AB0u, 239, 191);
    CheckStageSelectCommand(list, 41u, 0x800509C0u, 238, 188);

    state.rawStatus80092F1DTo23[6] = 0u;
    list = BuildStageSelectDrawList80020568(state);
    CheckAcceptedStageSelectCommands(
        list, kStageSelectBaseSpriteCount80020568, false);
    CheckStageSelectCommand(list, 38u, 0x80050AB0u, 239, 191);
    CheckStageSelectCommand(list, 39u, 0x800509C0u, 238, 188);
    CheckStageSelectCommand(list, 40u, 0x80050AD0u, 231, 179, 1u);
}

void TestStageSelectFiveLanguageTitleBoundary() {
    constexpr uint32_t kTitleTemplate[5] = {
        0x80051AB0u, 0x80051AD0u, 0x80051AC0u,
        0x80051AE0u, 0x80051AF0u,
    };
    constexpr int16_t kTitleX[5] = {44, 43, 35, 35, 35};
    constexpr int16_t kTitleY[5] = {36, 35, 37, 37, 35};
    for (int32_t language = 0; language < 5; ++language) {
        StageSelectState80020568 state{};
        state.language = language;
        state.rawStatus80092F1DTo23[0] = 1u;
        const auto list = BuildStageSelectDrawList80020568(state);
        CheckAcceptedStageSelectCommands(
            list, kStageSelectBaseSpriteCount80020568, false);
        CheckStageSelectCommand(
            list, 0u, kTitleTemplate[language],
            kTitleX[language], kTitleY[language]);
    }
}

void TestStageSelectNoncanonicalPredicateStates() {
    constexpr int32_t kNonzeroValues[] = {
        std::numeric_limits<int32_t>::min(), -1, 2,
        std::numeric_limits<int32_t>::max(),
    };

    for (const int32_t done : kNonzeroValues) {
        StageSelectState80020568 state{};
        state.cursor = 7;
        state.doneFlag = done;
        const auto list = BuildStageSelectDrawList80020568(state);
        CheckAcceptedStageSelectCommands(
            list, kStageSelectSpriteCapacity80020568, true);
        CheckStageSelectCommand(list, 2u, 0x800519F0u, 64, 100);
    }

    for (const int32_t blink : kNonzeroValues) {
        StageSelectState80020568 state{};
        state.cursor = 8;
        state.blinkOnOff = blink;
        const auto list = BuildStageSelectDrawList80020568(state);
        CheckAcceptedStageSelectCommands(
            list, kStageSelectBaseSpriteCount80020568, false);
        CheckStageSelectCommand(list, 40u, 0x80050AD0u, 231, 179, 1u);
    }
}

void TestStageSelectInvalidInputFailsClosed() {
    StageSelectState80020568 state{};
    state.language = -1;
    CheckStageSelectFailClosed(state);
    state = StageSelectState80020568{};
    state.language = 5;
    CheckStageSelectFailClosed(state);
    state.cursor = 0;
    CheckStageSelectFailClosed(state);
    state = StageSelectState80020568{};
    state.cursor = 9;
    CheckStageSelectFailClosed(state);
    state = StageSelectState80020568{};
    state.rawStatus80092F1DTo23[5] = 4u;
    CheckStageSelectFailClosed(state);
}

void CheckPracticeGuideList(const PracticeGuideDrawList80023518& list) {
    CHECK(list.sourceKnown);
    CHECK(list.arithmeticSupported);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.helperLevelOnly);
    CHECK(list.runtimeSubmitAllowed);
    CHECK(!list.rendererAuthorityPublished);
    CHECK(!list.runtimeDrawPublished);
    CHECK(!list.hostRendererUsed);
    CHECK(!list.replaySourceUsed);
    CHECK(list.staticTemplateSourcesResolved);
    CHECK(!list.truncated);
    CHECK(list.count == kPracticeGuideSpriteCount80023518);
    for (uint32_t index = 0; index < list.count; ++index) {
        CHECK(list.sprites[index].psxFunction == kFn8001C550);
        CHECK(list.sprites[index].x == list.sprites[index].threshold);
        CHECK(list.sprites[index].staticDescriptorKnown);
        CHECK(list.sprites[index].attr == 0x50000040u);
        CHECK(list.sprites[index].clutX == 0x0120u);
        CHECK(list.sprites[index].priority == 2u);
        CHECK(list.sprites[index].callOrder == index);
    }
}

void TestPracticeGuideMinusOneExactCalls() {
    PracticeGuideState80023518 state{};
    state.progressKnown = true;
    state.progress = -1;
    const auto list = BuildPracticeGuideDrawList80023518(state);
    CheckPracticeGuideList(list);
    CHECK(list.scaledProgress == 0);

    constexpr int32_t kLarge[4] = {63, 119, 175, 231};
    for (uint32_t index = 0; index < 4u; ++index) {
        CHECK(list.sprites[index].threshold == kLarge[index]);
        CHECK(list.sprites[index].y == 93);
        CHECK(!list.sprites[index].thresholdReached);
        CHECK(list.sprites[index].templateAddress == 0x80050930u);
        CHECK(list.sprites[index].texX == 0x03FAu);
        CHECK(list.sprites[index].texY == 0x0181u);
        CHECK(list.sprites[index].width == 12u);
        CHECK(list.sprites[index].height == 12u);
        CHECK(list.sprites[index].clutY == 0x01E2u);
    }
    constexpr int32_t kSmall[14] = {
        39, 53, 81, 95, 109, 137, 151,
        165, 193, 207, 221, 249, 263, 277,
    };
    for (uint32_t index = 0; index < 14u; ++index) {
        const auto& sprite = list.sprites[4u + index];
        CHECK(sprite.threshold == kSmall[index]);
        CHECK(sprite.y == 97);
        CHECK(!sprite.thresholdReached);
        CHECK(sprite.templateAddress == 0x80050910u);
        CHECK(sprite.texX == 0x03FDu);
        CHECK(sprite.texY == 0x0181u);
        CHECK(sprite.width == 8u);
        CHECK(sprite.height == 8u);
        CHECK(sprite.clutY == 0x01E0u);
    }
}

void TestPracticeGuideThresholdsAndFailClosed() {
    PracticeGuideState80023518 state{};
    const auto unknown = BuildPracticeGuideDrawList80023518(state);
    CHECK(!unknown.sourceKnown);
    CHECK(!unknown.arithmeticSupported);
    CHECK(!unknown.accepted);
    CHECK(!unknown.complete);
    CHECK(unknown.count == 0u);

    state.progressKnown = true;
    state.progress = 1;
    const auto first = BuildPracticeGuideDrawList80023518(state);
    CheckPracticeGuideList(first);
    CHECK(first.scaledProgress == 46);
    CHECK(!first.sprites[0].thresholdReached);
    CHECK(first.sprites[0].templateAddress == 0x80050930u);
    CHECK(first.sprites[4].thresholdReached);
    CHECK(first.sprites[4].templateAddress == 0x80050920u);
    CHECK(first.sprites[4].clutY == 0x01E1u);
    CHECK(!first.sprites[5].thresholdReached);

    state.progress = 143165575;
    const auto overflow = BuildPracticeGuideDrawList80023518(state);
    CHECK(overflow.sourceKnown);
    CHECK(!overflow.arithmeticSupported);
    CHECK(!overflow.accepted);
    CHECK(!overflow.complete);
    CHECK(overflow.count == 0u);
}

void CheckPracticeIconPrecommit(const PracticeIconSubmit80024418& submit) {
    CHECK(submit.argumentsKnown);
    CHECK(submit.argumentsAccepted);
    CHECK(submit.scaleWordsKnown);
    CHECK(submit.sourceKnown);
    CHECK(submit.staticPrefixResolved);
    CHECK(submit.templateResolved);
    CHECK(!submit.localRgbKnown);
    CHECK(submit.rgbSourceRequired);
    CHECK(submit.accepted);
    CHECK(submit.complete);
    CHECK(submit.helperLevelOnly);
    CHECK(!submit.runtimeSubmitAllowed);
    CHECK(!submit.packetSubmitAllowed);
    CHECK(!submit.rendererAuthorityPublished);
    CHECK(!submit.runtimeDrawPublished);
    CHECK(!submit.hostRendererUsed);
    CHECK(!submit.replaySourceUsed);
    CHECK(submit.templateTableBase == 0x800540BCu);
    CHECK(submit.localAttr == 0x50000040u);
    CHECK(submit.width == 16u);
    CHECK(submit.height == 16u);
    CHECK(submit.clutX == 0x0120u);
    CHECK(submit.pivotX == 8);
    CHECK(submit.pivotY == 8);
    CHECK(submit.priority == 1u);
}

void TestPracticeIconExactStaticPrefixAndScaleSources() {
    constexpr uint32_t kTemplates[8] = {
        0x8005405Cu, 0x8005403Cu, 0x8005404Cu, 0x8005406Cu,
        0x8005401Cu, 0x8005401Cu, 0x8005402Cu, 0x8005402Cu,
    };
    constexpr uint16_t kTexX[8] = {
        0x03F8u, 0x03F8u, 0x03FCu, 0x03FCu,
        0x03F4u, 0x03F4u, 0x03F4u, 0x03F4u,
    };
    constexpr uint16_t kTexY[8] = {
        0x01DDu, 0x01CDu, 0x01CDu, 0x01DDu,
        0x01CDu, 0x01CDu, 0x01DDu, 0x01DDu,
    };
    constexpr uint16_t kClutY[8] = {
        0x01ECu, 0x01E7u, 0x01E8u, 0x01EDu,
        0x01E6u, 0x01E6u, 0x01E9u, 0x01E9u,
    };
    for (int16_t type = 1; type <= 8; ++type) {
        PracticeIconState80024418 state{};
        state.argumentsKnown = true;
        state.scaleWordsKnown = true;
        state.centerX = 41;
        state.centerY = 99;
        state.slotOrdinal = static_cast<int16_t>(type - 1);
        state.type = type;
        state.scaleX = static_cast<int16_t>(4096 - type);
        state.scaleY = static_cast<int16_t>(4096 + type);
        const auto submit = BuildPracticeIconSubmit80024418(state);
        CheckPracticeIconPrecommit(submit);
        const uint32_t index = static_cast<uint32_t>(type - 1);
        CHECK(submit.templateSourceAddress ==
              0x800540BCu + static_cast<uint32_t>(type) * 4u);
        CHECK(submit.templateAddress == kTemplates[index]);
        CHECK(submit.scaleXSourceAddress == 0x80087668u + index * 2u);
        CHECK(submit.scaleYSourceAddress == 0x800876B0u + index * 2u);
        CHECK(submit.centerX == 41);
        CHECK(submit.centerY == 99);
        CHECK(submit.localX == -119);
        CHECK(submit.localY == -21);
        CHECK(submit.texX == kTexX[index]);
        CHECK(submit.texY == kTexY[index]);
        CHECK(submit.clutY == kClutY[index]);
        CHECK(submit.scaleX == state.scaleX);
        CHECK(submit.scaleY == state.scaleY);
    }
}

void TestPracticeIconFailsClosedWithoutScaleOrValidArgs() {
    PracticeIconState80024418 state{};
    state.argumentsKnown = true;
    state.centerX = 41;
    state.centerY = 99;
    state.slotOrdinal = 17;
    state.type = 1;
    const auto noScale = BuildPracticeIconSubmit80024418(state);
    CHECK(noScale.argumentsKnown);
    CHECK(noScale.argumentsAccepted);
    CHECK(!noScale.scaleWordsKnown);
    CHECK(!noScale.sourceKnown);
    CHECK(noScale.staticPrefixResolved);
    CHECK(noScale.templateResolved);
    CHECK(noScale.scaleXSourceAddress == 0x8008768Au);
    CHECK(noScale.scaleYSourceAddress == 0x800876D2u);
    CHECK(!noScale.accepted);
    CHECK(!noScale.complete);
    CHECK(!noScale.packetSubmitAllowed);

    state.scaleWordsKnown = true;
    state.slotOrdinal = -1;
    const auto badSlot = BuildPracticeIconSubmit80024418(state);
    CHECK(!badSlot.argumentsAccepted);
    CHECK(!badSlot.accepted);
    state.slotOrdinal = 0;
    state.type = 9;
    const auto badType = BuildPracticeIconSubmit80024418(state);
    CHECK(!badType.argumentsAccepted);
    CHECK(!badType.accepted);
}

void TestPracticeWobbleBankProducerAndIconBridge() {
    PracticeWobbleBank80023F20 bank{};
    int16_t scaleX = 0;
    int16_t scaleY = 0;
    CHECK(!ReadPracticeWobbleScale80024418(bank, 0, scaleX, scaleY));
    CHECK(!UpdatePracticeWobbleBank80023F20(bank, 1));
    const auto beforeReset = BuildPracticeIconSubmitFromWobbleBank80024418(
        bank, 41, 99, 0, 1);
    CHECK(beforeReset.argumentsAccepted);
    CHECK(beforeReset.staticPrefixResolved);
    CHECK(!beforeReset.sourceKnown);
    CHECK(!beforeReset.accepted);

    ResetPracticeWobbleBank80024308(bank);
    CHECK(bank.initialized);
    for (uint32_t index = 0;
         index < kPracticeWobbleSlotCount80024308;
         ++index) {
        CHECK(bank.scaleX[index] == 4096);
        CHECK(bank.scaleY[index] == 4096);
        CHECK(bank.slots[index].counter == 0);
        CHECK(bank.slots[index].phase == 0);
        CHECK(bank.slots[index].linearAcc == 2048);
        CHECK(bank.slots[index].linearVel == 2048);
    }
    const auto resetIcon = BuildPracticeIconSubmitFromWobbleBank80024418(
        bank, 41, 99, 0, 1);
    CheckPracticeIconPrecommit(resetIcon);
    CHECK(resetIcon.scaleX == 4096);
    CHECK(resetIcon.scaleY == 4096);

    CHECK(UpdatePracticeWobbleBank80023F20(bank, 2));
    CHECK(bank.scaleX[0] == 6144);
    CHECK(bank.scaleY[0] == 6144);
    CHECK(bank.scaleX[1] == 6144);
    CHECK(bank.scaleY[1] == 6144);
    CHECK(bank.scaleX[2] == 4096);
    CHECK(bank.slots[0].counter == 1);
    CHECK(bank.slots[1].counter == 1);
    const auto updatedIcon = BuildPracticeIconSubmitFromWobbleBank80024418(
        bank, 41, 99, 0, 1);
    CheckPracticeIconPrecommit(updatedIcon);
    CHECK(updatedIcon.scaleX == 6144);
    CHECK(updatedIcon.scaleY == 6144);

    for (int32_t step = 0; step < 5; ++step) {
        CHECK(UpdatePracticeWobbleBank80023F20(bank, 1));
    }
    CHECK(bank.slots[0].counter == 6);
    CHECK(bank.scaleX[0] == 4096);
    CHECK(bank.scaleY[0] == 4096);
    CHECK(UpdatePracticeWobbleBank80023F20(bank, 1));
    CHECK(bank.slots[0].counter == 7);
    CHECK(bank.slots[0].phase == 256);
    CHECK(bank.scaleX[0] == 0);
    CHECK(bank.scaleY[0] == 4096);

    const int32_t counterBeforeReject = bank.slots[0].counter;
    CHECK(!UpdatePracticeWobbleBank80023F20(bank, 37));
    CHECK(bank.slots[0].counter == counterBeforeReject);
}

void TestPracticeIconRgbCarrierBridge() {
    PracticeWobbleBank80023F20 bank{};
    ResetPracticeWobbleBank80024308(bank);
    const auto submit =
        BuildPracticeIconSubmitFromWobbleBankAndRgb80024418(
            bank, 69, 99, 2, 1, 16, 0, 0);
    CHECK(submit.argumentsKnown);
    CHECK(submit.argumentsAccepted);
    CHECK(submit.scaleWordsKnown);
    CHECK(submit.sourceKnown);
    CHECK(submit.localRgbKnown);
    CHECK(submit.localR == 16);
    CHECK(submit.localG == 0);
    CHECK(submit.localB == 0);
    CHECK(submit.accepted);
    CHECK(submit.complete);
    CHECK(submit.runtimeSubmitAllowed);
    CHECK(!submit.packetSubmitAllowed);
    CHECK(!submit.rendererAuthorityPublished);
    CHECK(!submit.runtimeDrawPublished);
    CHECK(submit.centerX == 69);
    CHECK(submit.centerY == 99);
    CHECK(submit.scaleXSourceAddress == 0x8008766Cu);
    CHECK(submit.scaleYSourceAddress == 0x800876B4u);
    CHECK(submit.scaleX == 4096);
    CHECK(submit.scaleY == 4096);
}

void SetExactPracticeIconGraph80024418(
    PrPsxGraphOwnerDirect::PsxGraphState& graph) {
    graph = {};
    graph.word_800965A0 = 4u;
    graph.word_800928D4 = 320;
    graph.word_800928D6 = 240;
    graph.drawOffset.setDrawEnvCalled = true;
    graph.drawOffset.word_800917AA = 0;
    graph.drawOffset.word_800917AC = 0;
    graph.drawOffset.word_80091738 = 160;
    graph.drawOffset.word_8009173A = 120;
    graph.gte.geomScreenKnown = true;
    graph.gte.geomScreen = 440u;
    graph.gte.geomOffsetKnown = true;
    graph.gte.geomOffsetX = 0;
    graph.gte.geomOffsetY = 0;
    graph.gte.depthCueKnown = true;
    graph.gte.depthCueA = -4194;
    graph.gte.depthCueB = 0x01400000;
    graph.gte.zScaleFactorKnown = true;
    graph.gte.zScaleFactor3 = 341;
    graph.gte.zScaleFactor4 = 256;
}

void TestPracticeIconExactGsSpriteDirectPayload() {
    PracticeWobbleBank80023F20 bank{};
    ResetPracticeWobbleBank80024308(bank);
    auto submit = BuildPracticeIconSubmitFromWobbleBankAndRgb80024418(
        bank, 69, 99, 2, 1, 16, 0, 0);
    auto graph = std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>();
    SetExactPracticeIconGraph80024418(*graph);
    const auto fast = BuildPracticeIconDirectRenderPayload80024418(
        submit, graph.get());
    CHECK(fast.sourceKnown);
    CHECK(fast.graphControlKnown);
    CHECK(fast.gsSortSpriteEvaluated);
    CHECK(fast.fastPacketPath);
    CHECK(!fast.transformPacketPath);
    CHECK(!fast.transformGeometryExactRtpt);
    CHECK(fast.drawEnvOffsetApplied);
    CHECK(fast.renderPayloadKnown);
    CHECK(!fast.packetOtMutationPublished);
    CHECK(!fast.replaySourceUsed);
    CHECK(fast.packetWordCount == 6u);
    CHECK(fast.priority == 1u);
    CHECK(fast.attr == 0x50000040u);
    CHECK(fast.tpage == 0x003Fu);
    CHECK(fast.clut == 0x7B12u);
    CHECK(fast.drawOffsetX == 160);
    CHECK(fast.drawOffsetY == 120);
    CHECK(fast.u[0] == 0xE0u);
    CHECK(fast.v[0] == 0xDDu);
    CHECK(fast.x[0] == 61);
    CHECK(fast.y[0] == 91);
    CHECK(fast.width == 16u);
    CHECK(fast.height == 16u);
    CHECK(fast.r == 16u);
    CHECK(fast.g == 0u);
    CHECK(fast.b == 0u);

    bank.scaleX[2] = 6144;
    bank.scaleY[2] = 6144;
    submit = BuildPracticeIconSubmitFromWobbleBankAndRgb80024418(
        bank, 69, 99, 2, 1, 16, 0, 0);
    const auto transformed = BuildPracticeIconDirectRenderPayload80024418(
        submit, graph.get());
    CHECK(transformed.sourceKnown);
    CHECK(transformed.graphControlKnown);
    CHECK(transformed.gsSortSpriteEvaluated);
    CHECK(!transformed.fastPacketPath);
    CHECK(transformed.transformPacketPath);
    CHECK(transformed.transformGeometryExactRtpt);
    CHECK(transformed.drawEnvOffsetApplied);
    CHECK(transformed.renderPayloadKnown);
    CHECK(!transformed.packetOtMutationPublished);
    CHECK(transformed.packetWordCount == 10u);
    CHECK(transformed.x[0] == 57);
    CHECK(transformed.y[0] == 87);
    CHECK(transformed.x[1] == 81);
    CHECK(transformed.y[1] == 87);
    CHECK(transformed.x[2] == 57);
    CHECK(transformed.y[2] == 111);
    CHECK(transformed.x[3] == 81);
    CHECK(transformed.y[3] == 111);
    CHECK(transformed.u[0] == 0xE0u);
    CHECK(transformed.v[0] == 0xDDu);
    CHECK(transformed.u[1] == 0xEFu);
    CHECK(transformed.v[1] == 0xDDu);
    CHECK(transformed.u[2] == 0xE0u);
    CHECK(transformed.v[2] == 0xECu);
    CHECK(transformed.u[3] == 0xEFu);
    CHECK(transformed.v[3] == 0xECu);
    CHECK(transformed.tpage == 0x003Fu);
    CHECK(transformed.clut == 0x7B12u);

    // The title/menu graph can be on the second PSX draw page when Practice
    // begins.  80040AE4/800401AC then expose the same exact sprite packet
    // with the page-relative Y centre at 360; the runtime removes that page
    // offset when mapping back to the visible 320x240 host viewport.
    graph->word_80096590 = 1u;
    graph->drawOffset.word_8009173A = 360;
    const auto secondPage = BuildPracticeIconDirectRenderPayload80024418(
        transformed, graph.get());
    CHECK(secondPage.sourceKnown);
    CHECK(secondPage.graphControlKnown);
    CHECK(secondPage.renderPayloadKnown);
    CHECK(secondPage.drawOffsetX == 160);
    CHECK(secondPage.drawOffsetY == 360);

    // The second half of the PSX rsin flip uses a signed X scale.  The
    // translated packet must keep the same center while swapping left/right
    // geometry; the runtime renderer then reverses host triangle winding so
    // this mirrored primitive is not back-face culled.
    bank.scaleX[2] = -6144;
    bank.scaleY[2] = 6144;
    submit = BuildPracticeIconSubmitFromWobbleBankAndRgb80024418(
        bank, 69, 99, 2, 1, 16, 0, 0);
    const auto mirrored = BuildPracticeIconDirectRenderPayload80024418(
        submit, graph.get());
    CHECK(mirrored.sourceKnown);
    CHECK(mirrored.transformPacketPath);
    CHECK(mirrored.transformGeometryExactRtpt);
    CHECK(mirrored.x[0] == 81);
    CHECK(mirrored.x[1] == 57);
    CHECK(mirrored.x[2] == 81);
    CHECK(mirrored.x[3] == 57);
    CHECK(mirrored.u[0] == 0xE0u);
    CHECK(mirrored.u[1] == 0xEFu);
    CHECK(mirrored.u[2] == 0xE0u);
    CHECK(mirrored.u[3] == 0xEFu);
    CHECK(mirrored.v[3] == 0xECu);
}

void TestPracticeIconDirectPayloadFailsClosed() {
    PracticeWobbleBank80023F20 bank{};
    ResetPracticeWobbleBank80024308(bank);
    const auto noRgb = BuildPracticeIconSubmitFromWobbleBank80024418(
        bank, 69, 99, 2, 1);
    auto graph = std::make_unique<PrPsxGraphOwnerDirect::PsxGraphState>();
    SetExactPracticeIconGraph80024418(*graph);
    const auto missingRgb = BuildPracticeIconDirectRenderPayload80024418(
        noRgb, graph.get());
    CHECK(!missingRgb.sourceKnown);
    CHECK(!missingRgb.gsSortSpriteEvaluated);
    CHECK(!missingRgb.renderPayloadKnown);

    const auto submit = BuildPracticeIconSubmitFromWobbleBankAndRgb80024418(
        bank, 69, 99, 2, 1, 16, 0, 0);
    const auto noGraph = BuildPracticeIconDirectRenderPayload80024418(
        submit, nullptr);
    CHECK(noGraph.sourceKnown);
    CHECK(!noGraph.graphControlKnown);
    CHECK(!noGraph.gsSortSpriteEvaluated);
    CHECK(!noGraph.renderPayloadKnown);

    graph->gte.geomScreenKnown = false;
    const auto missingGte = BuildPracticeIconDirectRenderPayload80024418(
        submit, graph.get());
    CHECK(missingGte.sourceKnown);
    CHECK(!missingGte.graphControlKnown);
    CHECK(!missingGte.gsSortSpriteEvaluated);
    CHECK(!missingGte.renderPayloadKnown);

    SetExactPracticeIconGraph80024418(*graph);
    graph->drawOffset.setDrawEnvCalled = false;
    const auto missingDrawEnv =
        BuildPracticeIconDirectRenderPayload80024418(submit, graph.get());
    CHECK(missingDrawEnv.sourceKnown);
    CHECK(!missingDrawEnv.graphControlKnown);
    CHECK(!missingDrawEnv.gsSortSpriteEvaluated);
    CHECK(!missingDrawEnv.renderPayloadKnown);
}

void TestPracticePortraitSharedCounterAndTemplateProducer() {
    PracticePortraitBank80024600 bank{};
    PracticePortraitState80024600 state{};
    state.argumentsKnown = true;
    state.row = 0;
    state.baseX = 16;
    state.baseY = 93;
    state.maxRepeatFrame = 3;
    state.value = 0;

    CHECK(!SelectPracticePortraitTemplate800246A8(bank, 4));
    const auto beforeReset = UpdatePracticePortraitBank80024600(bank, state);
    CHECK(beforeReset.argumentsKnown);
    CHECK(beforeReset.argumentsAccepted);
    CHECK(!beforeReset.templateKnown);
    CHECK(!beforeReset.staticDescriptorKnown);
    CHECK(!beforeReset.sourceKnown);
    CHECK(!beforeReset.accepted);

    ResetPracticePortraitBank80024600(bank);
    CHECK(bank.initialized);
    CHECK(bank.repeatFrame == 0);
    CHECK(bank.lastValue == 0);
    CHECK(!bank.templateKnown);
    CHECK(bank.templateAddress == 0u);

    constexpr uint32_t kExpectedModeTemplates[9] = {
        0x8005400Cu, 0x80053FFCu, 0x80053FECu,
        0x80053FBCu, 0x80053FCCu, 0x80053FACu,
        0x8005400Cu, 0x80053FDCu, 0x8005400Cu,
    };
    for (int32_t mode = 0; mode < 9; ++mode) {
        CHECK(SelectPracticePortraitTemplate800246A8(bank, mode));
        CHECK(bank.templateKnown);
        CHECK(bank.templateAddress == kExpectedModeTemplates[mode]);
    }
    CHECK(SelectPracticePortraitTemplate800246A8(bank, -1));
    CHECK(bank.templateAddress == 0x8005400Cu);
    CHECK(SelectPracticePortraitTemplate800246A8(bank, 9));
    CHECK(bank.templateAddress == 0x8005400Cu);

    CHECK(SelectPracticePortraitTemplate800246A8(bank, 4));
    const auto zero = UpdatePracticePortraitBank80024600(bank, state);
    CHECK(zero.argumentsAccepted);
    CHECK(zero.templateKnown);
    CHECK(zero.sourceKnown);
    CHECK(zero.accepted);
    CHECK(zero.complete);
    CHECK(!zero.drawRequested);
    CHECK(zero.repeatFrame == 1);
    CHECK(zero.templateAddress == 0x80053FCCu);
    CHECK(zero.repeatFrameSourceAddress ==
          kGpPracticePortraitRepeatFrame8006EB70);
    CHECK(zero.lastValueSourceAddress ==
          kGpPracticePortraitLastValue8006EB74);
    CHECK(zero.templateSourceAddress ==
          kGpPracticePortraitTemplate8006ED5C);
    CHECK(zero.helperLevelOnly);
    CHECK(zero.runtimeSubmitAllowed);
    CHECK(!zero.rendererAuthorityPublished);
    CHECK(!zero.runtimeDrawPublished);
    CHECK(!zero.hostRendererUsed);
    CHECK(!zero.replaySourceUsed);

    CHECK(SelectPracticePortraitTemplate800246A8(bank, 0));
    state.value = 1;
    auto portrait = UpdatePracticePortraitBank80024600(bank, state);
    CHECK(portrait.drawRequested);
    CHECK(portrait.repeatFrame == 0);
    CHECK(portrait.x == 31);
    CHECK(portrait.y == 93);
    CHECK(portrait.priority == 0u);
    CHECK(portrait.psxFunction == kFn8001C550);
    CHECK(portrait.templateAddress == 0x8005400Cu);
    for (int32_t frame = 1; frame <= 4; ++frame) {
        portrait = UpdatePracticePortraitBank80024600(bank, state);
        const int32_t clamped = frame > 3 ? 3 : frame;
        CHECK(portrait.repeatFrame == clamped);
        CHECK(portrait.x == 31 + 4 * clamped);
        CHECK(portrait.y == 93);
    }

    CHECK(SelectPracticePortraitTemplate800246A8(bank, 4));
    state.value = 2;
    const auto teacher = UpdatePracticePortraitBank80024600(bank, state);
    CHECK(teacher.repeatFrame == 0);
    CHECK(teacher.x == 46);
    CHECK(teacher.templateAddress == 0x80053FCCu);
    CHECK(teacher.runtimeSubmitAllowed);
    CHECK(teacher.staticDescriptorKnown);
    CHECK(teacher.attr == 0x50000040u);
    CHECK(teacher.texX == 0x03F5u);
    CHECK(teacher.texY == 0x019Du);
    CHECK(teacher.width == 16u);
    CHECK(teacher.height == 16u);
    CHECK(teacher.clutX == 0x0120u);
    CHECK(teacher.clutY == 0x01EFu);
    CHECK(SelectPracticePortraitTemplate800246A8(bank, 0));
    const auto student = UpdatePracticePortraitBank80024600(bank, state);
    CHECK(student.repeatFrame == 1);
    CHECK(student.x == 50);
    CHECK(student.templateAddress == 0x8005400Cu);
    CHECK(student.runtimeSubmitAllowed);
    CHECK(student.staticDescriptorKnown);
    CHECK(student.attr == 0x50000040u);
    CHECK(student.texX == 0x03F5u);
    CHECK(student.texY == 0x01BDu);
    CHECK(student.width == 16u);
    CHECK(student.height == 16u);
    CHECK(student.clutX == 0x0120u);
    CHECK(student.clutY == 0x01F3u);

    const int32_t repeatBeforeReject = bank.repeatFrame;
    state.value = 19;
    const auto badValue = UpdatePracticePortraitBank80024600(bank, state);
    CHECK(!badValue.argumentsAccepted);
    CHECK(!badValue.sourceKnown);
    CHECK(!badValue.accepted);
    CHECK(bank.repeatFrame == repeatBeforeReject);
}

void TestPracticeSliceRequiresOrderedPortraitRgbWriter() {
    PracticeSliceProducerState8001C604 state{};
    const auto unknown = BuildPracticeSliceDrawList8001C604(state);
    CHECK(!unknown.eventDispatchKnown);
    CHECK(!unknown.eventIdAccepted);
    CHECK(!unknown.precedingPortraitDrawProducerKnown);
    CHECK(!unknown.staticWriterSemanticsKnown);
    CHECK(!unknown.localRgbKnown);
    CHECK(!unknown.sourceKnown);
    CHECK(!unknown.accepted);
    CHECK(!unknown.complete);
    CHECK(!unknown.runtimeSubmitAllowed);
    CHECK(unknown.count == 0u);

    state.eventDispatchKnown = true;
    state.eventId = 16u;
    const auto noPortrait = BuildPracticeSliceDrawList8001C604(state);
    CHECK(noPortrait.eventDispatchKnown);
    CHECK(noPortrait.eventIdAccepted);
    CHECK(!noPortrait.precedingPortraitDrawProducerKnown);
    CHECK(!noPortrait.staticWriterSemanticsKnown);
    CHECK(!noPortrait.sourceKnown);
    CHECK(noPortrait.count == 0u);

    state.eventId = 15u;
    state.precedingPortraitDrawProducerKnown = true;
    const auto wrongEvent = BuildPracticeSliceDrawList8001C604(state);
    CHECK(!wrongEvent.eventIdAccepted);
    CHECK(wrongEvent.precedingPortraitDrawProducerKnown);
    CHECK(!wrongEvent.sourceKnown);
    CHECK(wrongEvent.count == 0u);
}

void TestPracticeSliceExactFourCallTransaction() {
    PracticeSliceProducerState8001C604 state{};
    state.eventDispatchKnown = true;
    state.eventId = 16u;
    state.precedingPortraitDrawProducerKnown = true;
    const auto list = BuildPracticeSliceDrawList8001C604(state);
    CHECK(list.eventDispatchKnown);
    CHECK(list.eventIdAccepted);
    CHECK(list.precedingPortraitDrawProducerKnown);
    CHECK(list.staticWriterSemanticsKnown);
    CHECK(list.localRgbKnown);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(list.runtimeSubmitAllowed);
    CHECK(!list.rendererAuthorityPublished);
    CHECK(!list.runtimeDrawPublished);
    CHECK(!list.packetOtMutationPublished);
    CHECK(!list.hostRendererUsed);
    CHECK(!list.replaySourceUsed);
    CHECK(!list.truncated);
    CHECK(list.count == kPracticeSliceSpriteCount8001C604);

    constexpr int16_t kX[kPracticeSliceSpriteCount8001C604] = {
        65, 122, 178, 234,
    };
    constexpr uint8_t kU[kPracticeSliceSpriteCount8001C604] = {
        0x84u, 0x8Bu, 0x92u, 0x99u,
    };
    for (uint32_t index = 0u; index < list.count; ++index) {
        const auto& sprite = list.sprites[index];
        CHECK(sprite.known);
        CHECK(sprite.staticDescriptorKnown);
        CHECK(sprite.textureCoordinatesResolved);
        CHECK(sprite.localRgbKnown);
        CHECK(sprite.psxFunction == kFn8001C604);
        CHECK(sprite.templateAddress == 0x80052DA0u);
        CHECK(sprite.attr == 0x50000040u);
        CHECK(sprite.texX == 0x01E1u);
        CHECK(sprite.texY == 0x0020u);
        CHECK(sprite.width == 7u);
        CHECK(sprite.height == 8u);
        CHECK(sprite.clutX == 0x01C0u);
        CHECK(sprite.clutY == 0x0101u);
        CHECK(sprite.tpage == 0x0027u);
        CHECK(sprite.u == kU[index]);
        CHECK(sprite.v == 0x20u);
        CHECK(sprite.localR == 16u);
        CHECK(sprite.localG == 0u);
        CHECK(sprite.localB == 0u);
        CHECK(sprite.x == kX[index]);
        CHECK(sprite.y == 81);
        CHECK(sprite.uOffset == static_cast<int16_t>(index * 7u));
        CHECK(sprite.priority == 0u);
        CHECK(sprite.callOrder == index);
    }
}

void TestPracticePostSliceFixedRequiresOrderedSlicePublish() {
    PracticePostSliceFixedProducerState80023618 state{};
    const auto unknown = BuildPracticePostSliceFixedDrawList80023618(state);
    CHECK(!unknown.eventDispatchKnown);
    CHECK(!unknown.eventIdAccepted);
    CHECK(!unknown.precedingSliceDrawPublished);
    CHECK(unknown.staticDescriptorSourcesKnown);
    CHECK(!unknown.sourceKnown);
    CHECK(!unknown.accepted);
    CHECK(!unknown.complete);
    CHECK(!unknown.runtimeSubmitAllowed);
    CHECK(unknown.count == 0u);

    state.eventDispatchKnown = true;
    state.eventId = 16u;
    const auto noSlices = BuildPracticePostSliceFixedDrawList80023618(state);
    CHECK(noSlices.eventIdAccepted);
    CHECK(!noSlices.precedingSliceDrawPublished);
    CHECK(!noSlices.sourceKnown);
    CHECK(noSlices.count == 0u);

    state.eventId = 15u;
    state.precedingSliceDrawPublished = true;
    const auto wrongEvent =
        BuildPracticePostSliceFixedDrawList80023618(state);
    CHECK(!wrongEvent.eventIdAccepted);
    CHECK(wrongEvent.precedingSliceDrawPublished);
    CHECK(!wrongEvent.sourceKnown);
    CHECK(wrongEvent.count == 0u);
}

void TestPracticePostSliceFixedExactFourCallTransaction() {
    PracticePostSliceFixedProducerState80023618 state{};
    state.eventDispatchKnown = true;
    state.eventId = 16u;
    state.precedingSliceDrawPublished = true;
    const auto list = BuildPracticePostSliceFixedDrawList80023618(state);
    CHECK(list.eventDispatchKnown);
    CHECK(list.eventIdAccepted);
    CHECK(list.precedingSliceDrawPublished);
    CHECK(list.staticDescriptorSourcesKnown);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(list.runtimeSubmitAllowed);
    CHECK(!list.rendererAuthorityPublished);
    CHECK(!list.runtimeDrawPublished);
    CHECK(!list.packetOtMutationPublished);
    CHECK(!list.hostRendererUsed);
    CHECK(!list.replaySourceUsed);
    CHECK(!list.truncated);
    CHECK(list.count == kPracticePostSliceFixedSpriteCount80023618);

    constexpr int16_t kX[kPracticePostSliceFixedSpriteCount80023618] = {
        32, 112, 211, 26,
    };
    constexpr int16_t kY[kPracticePostSliceFixedSpriteCount80023618] = {
        34, 31, 113, 119,
    };
    constexpr uint32_t
        kAddress[kPracticePostSliceFixedSpriteCount80023618] = {
            0x800529E0u, 0x800529F0u, 0x80052E30u, 0x80052E40u,
        };
    constexpr uint32_t kAttr[kPracticePostSliceFixedSpriteCount80023618] = {
        0x50000040u, 0x50000040u, 0x51000040u, 0x51000040u,
    };
    constexpr uint16_t kTexX[kPracticePostSliceFixedSpriteCount80023618] = {
        0x0200u, 0x0200u, 0x01C0u, 0x01C0u,
    };
    constexpr uint16_t kTexY[kPracticePostSliceFixedSpriteCount80023618] = {
        0x0034u, 0x0000u, 0x003Cu, 0x007Cu,
    };
    constexpr uint16_t kWidth[kPracticePostSliceFixedSpriteCount80023618] = {
        80u, 180u, 84u, 84u,
    };
    constexpr uint16_t kHeight[kPracticePostSliceFixedSpriteCount80023618] = {
        44u, 52u, 64u, 86u,
    };
    constexpr uint16_t kClutY[kPracticePostSliceFixedSpriteCount80023618] = {
        0x010Du, 0x010Eu, 0x01FEu, 0x01FFu,
    };
    for (uint32_t index = 0u; index < list.count; ++index) {
        const auto& command = list.sprites[index];
        CHECK(command.known);
        CHECK(command.psxFunction == kFn8001C550);
        CHECK(command.x == kX[index]);
        CHECK(command.y == kY[index]);
        CHECK(command.priority == 0u);
        CHECK(command.callOrder == index);
        CHECK(command.sprite.known);
        CHECK(command.sprite.psxAddress == kAddress[index]);
        CHECK(command.sprite.attr == kAttr[index]);
        CHECK(command.sprite.texX == kTexX[index]);
        CHECK(command.sprite.texY == kTexY[index]);
        CHECK(command.sprite.width == kWidth[index]);
        CHECK(command.sprite.height == kHeight[index]);
        CHECK(command.sprite.clutX == 0x01C0u);
        CHECK(command.sprite.clutY == kClutY[index]);
    }
}

void TestPracticePostSliceTailFailsClosedWithoutKnownState() {
    PracticePostSliceTailProducerState80023618 state{};
    const auto unknown = BuildPracticePostSliceTailDrawList80023618(state);
    CHECK(!unknown.eventDispatchKnown);
    CHECK(!unknown.eventIdAccepted);
    CHECK(!unknown.precedingSliceDrawPublished);
    CHECK(!unknown.languageAccepted);
    CHECK(!unknown.exitSelectionAccepted);
    CHECK(!unknown.exitBlinkAccepted);
    CHECK(unknown.staticWriterSemanticsKnown);
    CHECK(unknown.staticDescriptorSourcesKnown);
    CHECK(!unknown.sourceKnown);
    CHECK(!unknown.accepted);
    CHECK(!unknown.complete);
    CHECK(!unknown.runtimeSubmitAllowed);
    CHECK(unknown.count == 0u);

    state.eventDispatchKnown = true;
    state.eventId = 16u;
    state.precedingSliceDrawPublished = true;
    state.languageKnown = true;
    state.language = 5;
    state.exitSelectionKnown = true;
    state.exitSelection48 = 0;
    state.exitBlinkKnown = true;
    state.exitBlink4C = 0;
    const auto badLanguage =
        BuildPracticePostSliceTailDrawList80023618(state);
    CHECK(badLanguage.eventIdAccepted);
    CHECK(badLanguage.precedingSliceDrawPublished);
    CHECK(!badLanguage.languageAccepted);
    CHECK(badLanguage.exitSelectionAccepted);
    CHECK(badLanguage.exitBlinkAccepted);
    CHECK(!badLanguage.sourceKnown);
    CHECK(badLanguage.count == 0u);

    state.language = 0;
    state.exitSelectionKnown = false;
    const auto unknownSelection =
        BuildPracticePostSliceTailDrawList80023618(state);
    CHECK(unknownSelection.languageAccepted);
    CHECK(!unknownSelection.exitSelectionAccepted);
    CHECK(!unknownSelection.sourceKnown);
    CHECK(unknownSelection.count == 0u);

    state.exitSelectionKnown = true;
    state.exitBlinkKnown = false;
    const auto unknownBlink = BuildPracticePostSliceTailDrawList80023618(state);
    CHECK(!unknownBlink.exitBlinkAccepted);
    CHECK(!unknownBlink.sourceKnown);
    CHECK(unknownBlink.count == 0u);

    state.exitBlinkKnown = true;
    state.exitBlink4C = 0;
    state.precedingSliceDrawPublished = false;
    const auto noSlices = BuildPracticePostSliceTailDrawList80023618(state);
    CHECK(!noSlices.precedingSliceDrawPublished);
    CHECK(!noSlices.sourceKnown);
    CHECK(noSlices.count == 0u);
}

void TestPracticeAlwaysVisibleTailDoesNotRequireSlicePublication() {
    PracticePostSliceTailProducerState80023618 state{};
    state.eventDispatchKnown = true;
    state.eventId = 16u;
    state.precedingSliceDrawPublished = false;
    state.languageKnown = true;
    state.language = 0;
    state.exitSelectionKnown = true;
    state.exitSelection48 = 0;
    state.exitBlinkKnown = true;
    state.exitBlink4C = 0;

    const auto list = BuildPracticeAlwaysVisibleTailDrawList80023618(state);
    CHECK(list.eventDispatchKnown);
    CHECK(list.eventIdAccepted);
    CHECK(!list.precedingSliceDrawPublished);
    CHECK(list.alwaysVisibleFixedTail);
    CHECK(list.languageAccepted);
    CHECK(list.exitSelectionAccepted);
    CHECK(list.exitBlinkAccepted);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.runtimeSubmitAllowed);
    CHECK(list.count == kPracticePostSliceTailSpriteCount80023618);
}

void TestPracticePostSliceTailNoncanonicalExitStatesUseOriginalPredicates() {
    struct Case {
        int32_t selection;
        int16_t blink;
    };
    constexpr Case kCases[] = {
        {2, 2},
        {-1, -1},
        {(std::numeric_limits<int32_t>::min)(),
         (std::numeric_limits<int16_t>::min)()},
        {(std::numeric_limits<int32_t>::max)(),
         (std::numeric_limits<int16_t>::max)()},
        {0, 2},
        {2, 0},
    };

    for (const auto& value : kCases) {
        PracticePostSliceTailProducerState80023618 state{};
        state.eventDispatchKnown = true;
        state.eventId = 16u;
        state.precedingSliceDrawPublished = true;
        state.languageKnown = true;
        state.language = 0;
        state.exitSelectionKnown = true;
        state.exitSelection48 = value.selection;
        state.exitBlinkKnown = true;
        state.exitBlink4C = value.blink;

        const auto list = BuildPracticePostSliceTailDrawList80023618(state);
        CHECK(list.exitSelectionAccepted);
        CHECK(list.exitBlinkAccepted);
        CHECK(list.sourceKnown);
        CHECK(list.complete);
        CHECK(list.runtimeSubmitAllowed);
        CHECK(list.count == kPracticePostSliceTailSpriteCount80023618);
        CHECK(list.sprites[5].sprite.psxAddress ==
              (value.blink != 0 ? 0x80050AD0u : 0x80050AC0u));
        CHECK(list.sprites[6].sprite.psxAddress ==
              (value.selection != 0 ? 0x800509F0u : 0x800509E0u));
        CHECK(list.sprites[7].sprite.psxAddress ==
              (value.selection != 0 ? 0x800509C0u : 0x800509B0u));
    }
}

void TestPracticePostSliceTailExactEightCallTransaction() {
    constexpr uint32_t kCaptionAddress[5] = {
        0x80052A20u, 0x80052B80u, 0x80052AD0u,
        0x80052C30u, 0x80052CE0u,
    };
    constexpr int16_t kCaptionX[5] = {39, 40, 33, 37, 36};
    constexpr int16_t kCaptionY[5] = {49, 46, 50, 49, 46};
    constexpr uint16_t kCaptionTexY[5] = {
        0x0000u, 0x000Cu, 0x001Bu, 0x0026u, 0x0032u,
    };
    constexpr uint16_t kCaptionWidth[5] = {64u, 60u, 76u, 68u, 72u};
    constexpr uint16_t kCaptionHeight[5] = {12u, 15u, 11u, 12u, 15u};
    constexpr uint32_t kExitTextOff[5] = {
        0x800509E0u, 0x80050A40u, 0x80050A10u,
        0x80050A70u, 0x80050AA0u,
    };
    constexpr uint32_t kExitTextOn[5] = {
        0x800509F0u, 0x80050A50u, 0x80050A20u,
        0x80050A80u, 0x80050AB0u,
    };
    constexpr int16_t kExitX[5] = {242, 238, 238, 241, 239};
    constexpr int16_t kExitY[5] = {191, 193, 193, 190, 191};
    constexpr uint32_t kFixedAddress[4] = {
        0x800529E0u, 0x800529F0u, 0x80052E30u, 0x80052E40u,
    };
    constexpr int16_t kFixedX[4] = {32, 112, 211, 26};
    constexpr int16_t kFixedY[4] = {34, 31, 113, 119};

    for (int32_t language = 0; language < 5; ++language) {
        for (int32_t selected = 0; selected <= 1; ++selected) {
            for (int16_t blink = 0; blink <= 1; ++blink) {
                PracticePostSliceTailProducerState80023618 state{};
                state.eventDispatchKnown = true;
                state.eventId = 16u;
                state.precedingSliceDrawPublished = true;
                state.languageKnown = true;
                state.language = language;
                state.exitSelectionKnown = true;
                state.exitSelection48 = selected;
                state.exitBlinkKnown = true;
                state.exitBlink4C = blink;
                const auto list =
                    BuildPracticePostSliceTailDrawList80023618(state);

                CHECK(list.eventDispatchKnown);
                CHECK(list.eventIdAccepted);
                CHECK(list.precedingSliceDrawPublished);
                CHECK(!list.alwaysVisibleFixedTail);
                CHECK(list.languageAccepted);
                CHECK(list.exitSelectionAccepted);
                CHECK(list.exitBlinkAccepted);
                CHECK(list.staticWriterSemanticsKnown);
                CHECK(list.staticDescriptorSourcesKnown);
                CHECK(list.sourceKnown);
                CHECK(list.accepted);
                CHECK(list.complete);
                CHECK(list.rawTextureOnly);
                CHECK(list.runtimeSubmitAllowed);
                CHECK(!list.rendererAuthorityPublished);
                CHECK(!list.runtimeDrawPublished);
                CHECK(!list.packetOtMutationPublished);
                CHECK(!list.hostRendererUsed);
                CHECK(!list.replaySourceUsed);
                CHECK(!list.truncated);
                CHECK(list.count == kPracticePostSliceTailSpriteCount80023618);

                const auto& caption = list.sprites[0];
                CHECK(caption.psxFunction == kFn8001C550);
                CHECK(caption.x == kCaptionX[language]);
                CHECK(caption.y == kCaptionY[language]);
                CHECK(caption.sprite.psxAddress == kCaptionAddress[language]);
                CHECK(caption.sprite.attr == 0x50000040u);
                CHECK(caption.sprite.texX == 0x022Du);
                CHECK(caption.sprite.texY == kCaptionTexY[language]);
                CHECK(caption.sprite.width == kCaptionWidth[language]);
                CHECK(caption.sprite.height == kCaptionHeight[language]);
                CHECK(caption.sprite.clutX == 0x0200u);
                CHECK(caption.sprite.clutY == 0x0100u);

                for (uint32_t index = 0u; index < 4u; ++index) {
                    const auto& fixed = list.sprites[index + 1u];
                    CHECK(fixed.psxFunction == kFn8001C550);
                    CHECK(fixed.x == kFixedX[index]);
                    CHECK(fixed.y == kFixedY[index]);
                    CHECK(fixed.sprite.psxAddress == kFixedAddress[index]);
                }

                const auto& icon = list.sprites[5];
                CHECK(icon.psxFunction == kFn8001C550);
                CHECK(icon.x == 231);
                CHECK(icon.y == 179);
                CHECK(icon.sprite.psxAddress ==
                      (blink != 0 ? 0x80050AD0u : 0x80050AC0u));

                const auto& text = list.sprites[6];
                CHECK(text.psxFunction == kFn8001C5A8);
                CHECK(text.x == kExitX[language]);
                CHECK(text.y == kExitY[language]);
                CHECK(text.sprite.psxAddress ==
                      (selected != 0 ? kExitTextOn[language]
                                     : kExitTextOff[language]));

                const auto& bar = list.sprites[7];
                CHECK(bar.psxFunction == kFn8001C5A8);
                CHECK(bar.x == 238);
                CHECK(bar.y == 188);
                CHECK(bar.sprite.psxAddress ==
                      (selected != 0 ? 0x800509C0u : 0x800509B0u));

                for (uint32_t index = 0u; index < list.count; ++index) {
                    CHECK(list.sprites[index].known);
                    CHECK(list.sprites[index].priority == 0u);
                    CHECK(list.sprites[index].callOrder == index);
                    CHECK(list.sprites[index].sprite.known);
                }
            }
        }
    }
}

void CheckPracticeLeadingOverlayList(
    const PracticeLeadingOverlayDrawList80023618& list,
    uint32_t expectedCount) {
    CHECK(list.eventDispatchKnown);
    CHECK(list.eventIdAccepted);
    CHECK(list.languageAccepted);
    CHECK(list.flagsAccepted);
    CHECK(list.overlayStateAccepted);
    CHECK(list.staticWriterSemanticsKnown);
    CHECK(list.staticDescriptorSourcesKnown);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(list.runtimeSubmitAllowed);
    CHECK(!list.rendererAuthorityPublished);
    CHECK(!list.runtimeDrawPublished);
    CHECK(!list.packetOtMutationPublished);
    CHECK(!list.hostRendererUsed);
    CHECK(!list.replaySourceUsed);
    CHECK(!list.truncated);
    CHECK(list.count == expectedCount);
    for (uint32_t index = 0u; index < list.count; ++index) {
        const auto& command = list.sprites[index];
        CHECK(command.known);
        CHECK(command.psxFunction == kFn8001C550 ||
              command.psxFunction == kFn8001C5A8);
        CHECK(command.priority == 0u);
        CHECK(command.callOrder == index);
        CHECK(command.sprite.known);
        CHECK((command.sprite.attr & 0x40u) != 0u);
    }
}

void CheckPracticeLeadingOverlayFailClosed(
    const PracticeLeadingOverlayProducerState80023618& state) {
    const auto list = BuildPracticeLeadingOverlayDrawList80023618(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);
    CHECK(!list.complete);
    CHECK(!list.runtimeSubmitAllowed);
    CHECK(!list.rendererAuthorityPublished);
    CHECK(!list.runtimeDrawPublished);
    CHECK(!list.packetOtMutationPublished);
    CHECK(!list.hostRendererUsed);
    CHECK(!list.replaySourceUsed);
    CHECK(!list.truncated);
    CHECK(list.count == 0u);
}

void TestPracticeLeadingOverlayFailsClosedWithoutKnownState() {
    PracticeLeadingOverlayProducerState80023618 state{};
    CheckPracticeLeadingOverlayFailClosed(state);

    state.eventDispatchKnown = true;
    state.eventId = 15u;
    state.languageKnown = true;
    state.flagsKnown = true;
    CheckPracticeLeadingOverlayFailClosed(state);

    state.eventId = 16u;
    state.language = 5;
    CheckPracticeLeadingOverlayFailClosed(state);

    state.language = 0;
    state.flagsKnown = false;
    CheckPracticeLeadingOverlayFailClosed(state);

    state.flagsKnown = true;
    state.flags00 = 0x00400000u;
    state.overlayStateKnown = false;
    CheckPracticeLeadingOverlayFailClosed(state);
}

void TestPracticeLeadingOverlayDefaultSwitchNoOp() {
    constexpr int32_t kDefaultSwitchStates[] = {
        -1,
        9,
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max(),
    };

    PracticeLeadingOverlayProducerState80023618 state{};
    state.eventDispatchKnown = true;
    state.eventId = 16u;
    state.languageKnown = true;
    state.language = 0;
    state.flagsKnown = true;
    state.flags00 = 0x00400000u;
    state.overlayStateKnown = true;

    for (const int32_t overlayState : kDefaultSwitchStates) {
        state.overlayState1C = overlayState;
        const auto list = BuildPracticeLeadingOverlayDrawList80023618(state);
        CheckPracticeLeadingOverlayList(list, 0u);
        CHECK(list.overlayGateActive);

        PracticeState80023618 traceState{};
        traceState.language = state.language;
        traceState.flags00 = state.flags00;
        traceState.overlayState1C = overlayState;
        const auto trace = BuildPracticeDrawCallTrace80023618(traceState);
        CHECK(trace.sourceKnown);
        CHECK(trace.accepted);
        CHECK(trace.complete);
        CHECK(trace.staticOperandSourcesResolved);
        CHECK(!trace.truncated);
        CHECK(trace.count == 13u);
        CHECK(trace.calls[0].kind ==
              PracticeDrawCallKind80023618::Guide80023518);
    }
}

void TestPracticeLeadingOverlayExactTransactions() {
    constexpr int16_t kTitleX[5] = {204, 206, 257, 249, 244};
    constexpr int16_t kSubtitleX[5] = {136, 139, 128, 137, 142};
    constexpr int16_t kSubtitleY[5] = {40, 40, 40, 40, 40};
    constexpr uint32_t kSubtitleTemplate[5] = {
        0x80052A30u, 0x80052B90u, 0x80052AE0u,
        0x80052C40u, 0x80052CF0u,
    };
    constexpr uint32_t kOverlayCount[9] = {
        5u, 10u, 10u, 2u, 6u, 6u, 6u, 4u, 4u,
    };

    for (int32_t language = 0; language < 5; ++language) {
        PracticeLeadingOverlayProducerState80023618 state{};
        state.eventDispatchKnown = true;
        state.eventId = 16u;
        state.languageKnown = true;
        state.language = language;
        state.flagsKnown = true;
        const auto defaultList =
            BuildPracticeLeadingOverlayDrawList80023618(state);
        CheckPracticeLeadingOverlayList(defaultList, 2u);
        CHECK(!defaultList.overlayGateActive);
        CHECK(defaultList.sprites[0].psxFunction == kFn8001C5A8);
        CHECK(defaultList.sprites[0].x == kTitleX[language]);
        CHECK(defaultList.sprites[0].y == 55);
        CHECK(defaultList.sprites[0].sprite.psxAddress == 0x80052A00u);
        CHECK(defaultList.sprites[1].psxFunction == kFn8001C5A8);
        CHECK(defaultList.sprites[1].x == kSubtitleX[language]);
        CHECK(defaultList.sprites[1].y == kSubtitleY[language]);
        CHECK(defaultList.sprites[1].sprite.psxAddress ==
              kSubtitleTemplate[language]);

        state.flags00 = 0x00000800u;
        CheckPracticeLeadingOverlayList(
            BuildPracticeLeadingOverlayDrawList80023618(state), 2u);
        state.flags00 = 0x00100000u;
        CheckPracticeLeadingOverlayList(
            BuildPracticeLeadingOverlayDrawList80023618(state), 2u);

        state.flags00 = 0x00400000u;
        state.overlayStateKnown = true;
        for (int32_t overlay = 0; overlay <= 8; ++overlay) {
            state.overlayState1C = overlay;
            const auto list =
                BuildPracticeLeadingOverlayDrawList80023618(state);
            CheckPracticeLeadingOverlayList(list, kOverlayCount[overlay]);
            CHECK(list.overlayGateActive);

            PracticeState80023618 traceState{};
            traceState.language = language;
            traceState.flags00 = state.flags00;
            traceState.overlayState1C = overlay;
            const auto trace = BuildPracticeDrawCallTrace80023618(traceState);
            CHECK(trace.sourceKnown);
            CHECK(trace.accepted);
            CHECK(trace.complete);
            CHECK(trace.staticOperandSourcesResolved);
            CHECK(!trace.truncated);
            CHECK(trace.count == kOverlayCount[overlay] + 13u);
            for (uint32_t index = 0u; index < list.count; ++index) {
                CHECK(list.sprites[index].psxFunction ==
                      trace.calls[index].psxFunction);
                CHECK(list.sprites[index].x == trace.calls[index].resolvedX);
                CHECK(list.sprites[index].y == trace.calls[index].resolvedY);
                CHECK(list.sprites[index].sprite.psxAddress ==
                      trace.calls[index].resolvedTemplateAddress);
            }
        }
    }
}

void CheckPracticeTrace(const PracticeDrawCallTrace80023618& trace,
                        uint32_t expectedCount) {
    CHECK(trace.sourceKnown);
    CHECK(trace.accepted);
    CHECK(trace.complete);
    CHECK(trace.helperLevelOnly);
    CHECK(!trace.runtimeSubmitAllowed);
    CHECK(!trace.rendererAuthorityPublished);
    CHECK(!trace.runtimeDrawPublished);
    CHECK(!trace.hostRendererUsed);
    CHECK(!trace.replaySourceUsed);
    CHECK(trace.staticOperandSourcesResolved);
    CHECK(!trace.truncated);
    CHECK(trace.count == expectedCount);
    for (uint32_t index = 0; index < trace.count; ++index) {
        CHECK(trace.calls[index].callOrder == index);
        CHECK(trace.calls[index].kind !=
              PracticeDrawCallKind80023618::None);
        CHECK(trace.calls[index].psxFunction != 0u);
    }
}

void CheckPracticeCall(const PracticeDrawCallTrace80023618& trace,
                       uint32_t index,
                       PracticeDrawCallKind80023618 kind,
                       uint32_t function) {
    CHECK(index < trace.count);
    if (index >= trace.count) {
        return;
    }
    CHECK(trace.calls[index].kind == kind);
    CHECK(trace.calls[index].psxFunction == function);
}

void CheckPracticeFailClosed(const PracticeState80023618& state) {
    const auto trace = BuildPracticeDrawCallTrace80023618(state);
    CHECK(!trace.sourceKnown);
    CHECK(!trace.accepted);
    CHECK(!trace.complete);
    CHECK(trace.helperLevelOnly);
    CHECK(!trace.runtimeSubmitAllowed);
    CHECK(!trace.rendererAuthorityPublished);
    CHECK(!trace.runtimeDrawPublished);
    CHECK(!trace.hostRendererUsed);
    CHECK(!trace.replaySourceUsed);
    CHECK(!trace.staticOperandSourcesResolved);
    CHECK(!trace.truncated);
    CHECK(trace.count == 0u);
}

void TestPracticeDefaultHelperCallOrder() {
    PracticeState80023618 state{};
    const auto trace = BuildPracticeDrawCallTrace80023618(state);
    CheckPracticeTrace(trace, 15u);

    CheckPracticeCall(trace, 0u,
                      PracticeDrawCallKind80023618::FastSprite8001C5A8,
                      kFn8001C5A8);
    CHECK(trace.calls[0].positionSourceAddress == 0x80053D4Cu);
    CHECK(trace.calls[0].templateSourceAddress == 0x80053D48u);
    CHECK(trace.calls[0].templatePointerIndirect);
    CHECK(trace.calls[0].positionResolved);
    CHECK(trace.calls[0].resolvedX == 204);
    CHECK(trace.calls[0].resolvedY == 55);
    CHECK(trace.calls[0].templateResolved);
    CHECK(trace.calls[0].resolvedTemplateAddress == 0x80052A00u);
    CheckPracticeCall(trace, 1u,
                      PracticeDrawCallKind80023618::FastSprite8001C5A8,
                      kFn8001C5A8);
    CHECK(trace.calls[1].positionSourceAddress == 0x80053BBCu);
    CHECK(trace.calls[1].templateSourceAddress == 0x80053BB8u);
    CHECK(trace.calls[1].resolvedX == 136);
    CHECK(trace.calls[1].resolvedY == 40);
    CHECK(trace.calls[1].resolvedTemplateAddress == 0x80052A30u);
    CheckPracticeCall(trace, 2u,
                      PracticeDrawCallKind80023618::Guide80023518,
                      kFn80023518);
    CHECK(trace.calls[2].args[0] == -1);
    CHECK(trace.calls[2].helperExpansionResolved);
    CHECK(trace.calls[2].helperExpandedCallCount ==
          kPracticeGuideSpriteCount80023518);

    constexpr int32_t kSliceX[4] = {65, 122, 178, 234};
    for (uint32_t index = 0; index < 4u; ++index) {
        const auto& call = trace.calls[3u + index];
        CHECK(call.kind == PracticeDrawCallKind80023618::Slice8001C604);
        CHECK(call.psxFunction == kFn8001C604);
        CHECK(call.templateSourceAddress == 0x80052DA0u);
        CHECK(!call.templatePointerIndirect);
        CHECK(call.args[0] == kSliceX[index]);
        CHECK(call.args[1] == 81);
        CHECK(call.args[3] == static_cast<int32_t>(index * 7u));
        CHECK(call.args[4] == 7);
        CHECK(call.args[5] == 0);
        CHECK(call.args[6] == 0);
        CHECK(call.positionResolved);
        CHECK(call.resolvedX == kSliceX[index]);
        CHECK(call.resolvedY == 81);
        CHECK(call.templateResolved);
        CHECK(call.resolvedTemplateAddress == 0x80052DA0u);
    }

    CheckPracticeCall(trace, 7u,
                      PracticeDrawCallKind80023618::Sprite8001C550,
                      kFn8001C550);
    CHECK(trace.calls[7].positionSourceAddress == 0x80053B94u);
    CHECK(trace.calls[7].templateSourceAddress == 0x80053B90u);
    CHECK(trace.calls[7].templatePointerIndirect);
    CHECK(trace.calls[7].resolvedX == 39);
    CHECK(trace.calls[7].resolvedY == 49);
    CHECK(trace.calls[7].resolvedTemplateAddress == 0x80052A20u);
    CheckPracticeCall(trace, 12u,
                      PracticeDrawCallKind80023618::Sprite8001C550,
                      kFn8001C550);
    CHECK(trace.calls[12].templateSourceAddress ==
          kGpSharedExitIconSlotOff8006EB14);
    CHECK(trace.calls[12].templatePointerIndirect);
    CHECK(trace.calls[12].resolvedX == 231);
    CHECK(trace.calls[12].resolvedY == 179);
    CHECK(trace.calls[12].resolvedTemplateAddress == 0x80050AC0u);
    CheckPracticeCall(trace, 13u,
                      PracticeDrawCallKind80023618::FastSprite8001C5A8,
                      kFn8001C5A8);
    CHECK(trace.calls[13].positionSourceAddress == 0x8005300Cu);
    CHECK(trace.calls[13].templateSourceAddress == 0x80053004u);
    CHECK(trace.calls[13].templatePointerIndirect);
    CHECK(trace.calls[13].resolvedX == 242);
    CHECK(trace.calls[13].resolvedY == 191);
    CHECK(trace.calls[13].resolvedTemplateAddress == 0x800509E0u);
    CheckPracticeCall(trace, 14u,
                      PracticeDrawCallKind80023618::FastSprite8001C5A8,
                      kFn8001C5A8);
    CHECK(trace.calls[14].positionSourceAddress == 0x80052FFCu);
    CHECK(trace.calls[14].templateSourceAddress == 0x800509B0u);
    CHECK(!trace.calls[14].templatePointerIndirect);
    CHECK(trace.calls[14].resolvedX == 238);
    CHECK(trace.calls[14].resolvedY == 188);
    CHECK(trace.calls[14].resolvedTemplateAddress == 0x800509B0u);
}

void TestPracticeMaximumCase1HelperCallOrder() {
    PracticeState80023618 state{};
    state.language = 4;
    state.flags00 = 0x00400000u;
    state.overlayState1C = 1;
    state.laneA8C = 0;
    state.laneB9E = 18;
    state.exitSelection48 = 1;
    state.exitBlink4C = 1;
    for (uint32_t index = 0; index < 18u; ++index) {
        state.iconCodes[index] = static_cast<uint8_t>(index % 8u + 1u);
    }

    const auto trace = BuildPracticeDrawCallTrace80023618(state);
    CheckPracticeTrace(trace, 47u);
    CHECK(trace.calls[0].templateSourceAddress == 0x80053CC8u);
    CHECK(trace.calls[0].resolvedX == 132);
    CHECK(trace.calls[0].resolvedY == 144);
    CHECK(trace.calls[0].resolvedTemplateAddress == 0x80052D50u);
    CHECK(trace.calls[1].templateSourceAddress == 0x80053D40u);
    CHECK(trace.calls[1].resolvedX == 121);
    CHECK(trace.calls[1].resolvedY == 126);
    CHECK(trace.calls[1].resolvedTemplateAddress == 0x80052D80u);
    CHECK(trace.calls[8].templateSourceAddress == 0x80053D68u);
    CHECK(trace.calls[8].resolvedX == 244);
    CHECK(trace.calls[8].resolvedY == 55);
    CHECK(trace.calls[8].resolvedTemplateAddress == 0x80052A00u);
    CHECK(trace.calls[9].templateSourceAddress == 0x80053BD8u);
    CHECK(trace.calls[9].resolvedX == 142);
    CHECK(trace.calls[9].resolvedY == 40);
    CHECK(trace.calls[9].resolvedTemplateAddress == 0x80052CF0u);
    CheckPracticeCall(trace, 10u,
                      PracticeDrawCallKind80023618::WobbleUpdate80023F20,
                      kFn80023F20);
    CHECK(trace.calls[10].args[0] == 0);
    CheckPracticeCall(trace, 11u,
                      PracticeDrawCallKind80023618::Rec44Mode800246A8,
                      kFn800246A8);
    CHECK(trace.calls[11].args[0] == 4);
    CheckPracticeCall(trace, 12u,
                      PracticeDrawCallKind80023618::Rec44Draw80024600,
                      kFn80024600);
    CHECK(trace.calls[12].args[4] == 0);
    CHECK(trace.calls[12].templateResolved);
    CHECK(trace.calls[12].resolvedTemplateAddress == 0x80053FCCu);
    CHECK(!trace.calls[12].dynamicPositionRequired);
    CheckPracticeCall(trace, 14u,
                      PracticeDrawCallKind80023618::Rec44Mode800246A8,
                      kFn800246A8);
    CHECK(trace.calls[14].args[0] == 0);
    CHECK(trace.calls[13].kind ==
          PracticeDrawCallKind80023618::WobbleUpdate80023F20);
    CHECK(trace.calls[13].args[0] == 18);
    CHECK(trace.calls[15].args[4] == 18);
    CHECK(trace.calls[15].templateResolved);
    CHECK(trace.calls[15].resolvedTemplateAddress == 0x8005400Cu);
    CHECK(trace.calls[15].dynamicPositionRequired);

    constexpr uint32_t kExpectedIconTemplates[8] = {
        0x8005405Cu, 0x8005403Cu, 0x8005404Cu, 0x8005406Cu,
        0x8005401Cu, 0x8005401Cu, 0x8005402Cu, 0x8005402Cu,
    };
    for (uint32_t index = 0; index < 18u; ++index) {
        const auto& call = trace.calls[17u + index];
        CHECK(call.kind == PracticeDrawCallKind80023618::Icon80024418);
        CHECK(call.psxFunction == kFn80024418);
        CHECK(call.args[0] == 41 + static_cast<int32_t>(14u * index));
        CHECK(call.args[1] == 99);
        CHECK(call.args[2] == static_cast<int32_t>(index));
        CHECK(call.args[3] == static_cast<int32_t>(index % 8u + 1u));
        CHECK(call.positionResolved);
        CHECK(call.resolvedX ==
              41 + static_cast<int32_t>(14u * index));
        CHECK(call.resolvedY == 99);
        CHECK(call.templateResolved);
        CHECK(call.resolvedTemplateAddress ==
              kExpectedIconTemplates[index % 8u]);
        CHECK(call.dynamicScaleRequired);
        CHECK(call.scaleXSourceAddress == 0x80087668u + index * 2u);
        CHECK(call.scaleYSourceAddress == 0x800876B0u + index * 2u);
        CHECK(call.helperStaticPrefixResolved);
        CHECK(!call.helperExpansionResolved);
    }
    CHECK(trace.calls[44].templateSourceAddress ==
          kGpSharedExitIconSlotOn8006EB10);
    CHECK(trace.calls[44].resolvedTemplateAddress == 0x80050AD0u);
    CHECK(trace.calls[45].positionSourceAddress == 0x8005304Cu);
    CHECK(trace.calls[45].templateSourceAddress == 0x80053048u);
    CHECK(trace.calls[45].resolvedX == 239);
    CHECK(trace.calls[45].resolvedY == 191);
    CHECK(trace.calls[45].resolvedTemplateAddress == 0x80050AB0u);
    CHECK(trace.calls[46].templateSourceAddress == 0x800509C0u);
    CHECK(trace.calls[46].resolvedX == 238);
    CHECK(trace.calls[46].resolvedY == 188);
    CHECK(trace.calls[46].resolvedTemplateAddress == 0x800509C0u);
}

void TestPracticeOverlayCaseCountsAndCase8NoHeader() {
    constexpr uint32_t kExpectedCount[9] = {
        18u, 23u, 23u, 15u, 19u, 19u, 19u, 17u, 17u,
    };
    for (int32_t overlay = 0; overlay <= 8; ++overlay) {
        PracticeState80023618 state{};
        state.flags00 = 0x00400000u;
        state.overlayState1C = overlay;
        const auto trace = BuildPracticeDrawCallTrace80023618(state);
        CheckPracticeTrace(trace, kExpectedCount[overlay]);
    }

    PracticeState80023618 state{};
    state.flags00 = 0x00400000u;
    state.overlayState1C = 8;
    const auto trace = BuildPracticeDrawCallTrace80023618(state);
    CHECK(trace.calls[0].templateSourceAddress == 0x80053D70u);
    CHECK(trace.calls[0].resolvedTemplateAddress == 0x80052A10u);
    CHECK(trace.calls[1].templateSourceAddress == 0x80053BE0u);
    CHECK(trace.calls[1].resolvedTemplateAddress == 0x80052A40u);
    CHECK(trace.calls[2].templateSourceAddress == 0x80052DC0u);
    CHECK(trace.calls[3].templateSourceAddress == 0x80052DF0u);
    CheckPracticeCall(trace, 4u,
                      PracticeDrawCallKind80023618::Guide80023518,
                      kFn80023518);

    state.overlayState1C = 9;
    CheckPracticeTrace(BuildPracticeDrawCallTrace80023618(state), 13u);
}

void TestPracticeInvalidSourcesFailClosed() {
    PracticeState80023618 state{};
    state.contextPresent = false;
    CheckPracticeFailClosed(state);
    state = PracticeState80023618{};
    state.language = -1;
    CheckPracticeFailClosed(state);
    state = PracticeState80023618{};
    state.language = 5;
    CheckPracticeFailClosed(state);
    state = PracticeState80023618{};
    state.iconBytesKnown = false;
    CheckPracticeFailClosed(state);
    state = PracticeState80023618{};
    state.laneA8C = 19;
    CheckPracticeFailClosed(state);

    state = PracticeState80023618{};
    state.iconCodes[0] = 0xFFu;
    state.iconCodes[1] = 9u;
    CheckPracticeTrace(BuildPracticeDrawCallTrace80023618(state), 15u);
}

void CheckEventBackdropCommand(
    const EventBackdrop::EventBackdropDrawList8001D74C& list,
    uint32_t index,
    uint32_t address,
    int16_t x,
    int16_t y) {
    CHECK(index < list.count);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.known);
    CHECK(command.sprite.known);
    CHECK(command.sprite.psxAddress == address);
    CHECK(command.x == x);
    CHECK(command.y == y);
    CHECK(command.priority == kEventBackdropTestPriority);
    CHECK(command.callOrder == index);
    CHECK((command.sprite.attr & 0x40u) != 0u);
}

CardGridState80020F94 MakeCardGridState80020F94(int32_t eventId,
                                                int32_t language) {
    CardGridState80020F94 state{};
    state.requestBound = true;
    state.argAddress = kCardGridArgAddress80048E50;
    state.eventId = eventId;
    state.language = language;
    state.rows = 5;
    state.columns = 3;
    state.itemCount = 3;
    state.selected = 1;
    state.enabled[0] = 1;
    state.enabled[1] = 1;
    state.enabled[2] = 0;
    std::snprintf(state.slotText[0].data(), state.slotText[0].size(), "AAA");
    std::snprintf(state.slotText[1].data(), state.slotText[1].size(), "B");
    return state;
}

void CheckCardGridCommand80020F94(
    const CardGridDrawList80020F94& list,
    uint32_t index,
    CardGridSpriteRole80020F94 role,
    uint32_t address,
    int16_t x,
    int16_t y) {
    CHECK(index < list.count);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.known);
    CHECK(command.role == role);
    CHECK(command.sprite.known);
    CHECK(command.sprite.psxAddress == address);
    CHECK(command.x == x);
    CHECK(command.y == y);
    CHECK(command.priority == 0u);
    CHECK(command.callOrder == index);
    CHECK(command.sprite.attr == 0x50000040u);
}

void TestCardGridExactLoadTransaction80020F94() {
    const auto state = MakeCardGridState80020F94(8, 0);
    const auto list = BuildCardGridDrawList80020F94(state);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.runtimeSubmitAllowed);
    CHECK(list.rawTextureOnly);
    CHECK(!list.cardIoOverlay80020A3CRequired);
    CHECK(!list.truncated);
    CHECK(list.glyphSpriteCount == 4u);
    CHECK(list.glyphNoPacketCount == 0u);
    CHECK(list.markerCount == 3u);
    CHECK(list.count == 17u);
    CHECK(list.commands.size() == kCardGridSpriteCapacity80020F94);

    CheckCardGridCommand80020F94(
        list, 0u, CardGridSpriteRole80020F94::LanguageText,
        0x80052990u, 154, 44);
    CheckCardGridCommand80020F94(
        list, 1u, CardGridSpriteRole80020F94::LanguageText,
        0x80052980u, 189, 44);
    CheckCardGridCommand80020F94(
        list, 2u, CardGridSpriteRole80020F94::LanguageText,
        0x800527D0u, 53, 42);
    CheckCardGridCommand80020F94(
        list, 3u, CardGridSpriteRole80020F94::LanguageText,
        0x80052820u, 230, 44);
    CheckCardGridCommand80020F94(
        list, 4u, CardGridSpriteRole80020F94::LanguageText,
        0x80052920u, 208, 45);
    CheckCardGridCommand80020F94(
        list, 5u, CardGridSpriteRole80020F94::FixedPanel,
        0x80052870u, 37, 36);
    CheckCardGridCommand80020F94(
        list, 6u, CardGridSpriteRole80020F94::FixedPanel,
        0x800522C0u, 123, 30);

    CHECK(list.commands[7].role == CardGridSpriteRole80020F94::SlotGlyph);
    CHECK(list.commands[7].glyphCode == static_cast<uint8_t>('A'));
    CHECK(list.commands[7].textureCoordinatesResolved);
    CHECK(list.commands[7].sprite.clutX == 256u);
    CHECK(list.commands[7].sprite.clutY == 483u);
    CheckCardGridCommand80020F94(
        list, 10u, CardGridSpriteRole80020F94::SlotMarker,
        0x80052240u, 51, 73);
    CHECK(list.commands[11].role == CardGridSpriteRole80020F94::SlotGlyph);
    CHECK(list.commands[11].glyphCode == static_cast<uint8_t>('B'));
    CHECK(list.commands[11].sprite.clutY == 482u);
    CheckCardGridCommand80020F94(
        list, 12u, CardGridSpriteRole80020F94::SlotMarker,
        0x80052250u, 125, 73);
    CheckCardGridCommand80020F94(
        list, 13u, CardGridSpriteRole80020F94::SlotMarker,
        0x80052260u, 199, 73);
    CheckCardGridCommand80020F94(
        list, 14u, CardGridSpriteRole80020F94::ExitFrame,
        0x80050AC0u, 231, 179);
    CheckCardGridCommand80020F94(
        list, 15u, CardGridSpriteRole80020F94::ExitLabel,
        0x800509D0u, 242, 191);
    CheckCardGridCommand80020F94(
        list, 16u, CardGridSpriteRole80020F94::ExitBar,
        0x800509A0u, 238, 188);
}

void TestCardGridExitAndIoBoundary80020F94() {
    auto state = MakeCardGridState80020F94(7, 1);
    state.itemCount = 0;
    state.selected = 15;
    state.exitFrameState = 1;
    state.exitBlinkState = 1;
    state.cardIoFlag = 1;
    const auto list = BuildCardGridDrawList80020F94(state);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(!list.complete);
    CHECK(!list.runtimeSubmitAllowed);
    CHECK(list.rawTextureOnly);
    CHECK(list.cardIoOverlay80020A3CRequired);
    CHECK(list.blockedByCardIoOverlay80020A3C);
    CHECK(list.cardIoOverlayInsertIndexKnown);
    CHECK(list.cardIoOverlayInsertIndex == 5u);
    CHECK(list.cardIoMessageType == 2);
    CHECK(list.count == 10u);
    CheckCardGridCommand80020F94(
        list, 7u, CardGridSpriteRole80020F94::ExitFrame,
        0x80050AD0u, 231, 179);
    CheckCardGridCommand80020F94(
        list, 8u, CardGridSpriteRole80020F94::ExitLabel,
        0x80050A50u, 238, 193);
    CheckCardGridCommand80020F94(
        list, 9u, CardGridSpriteRole80020F94::ExitBar,
        0x800509C0u, 238, 188);

    for (int32_t eventId : {8, 9}) {
        state = MakeCardGridState80020F94(eventId, 0);
        state.itemCount = 0;
        state.selected = 15;
        state.cardIoFlag = 1;
        const auto loadReplayList = BuildCardGridDrawList80020F94(state);
        CHECK(loadReplayList.sourceKnown);
        CHECK(loadReplayList.accepted);
        CHECK(!loadReplayList.complete);
        CHECK(!loadReplayList.runtimeSubmitAllowed);
        CHECK(loadReplayList.cardIoOverlay80020A3CRequired);
        CHECK(loadReplayList.blockedByCardIoOverlay80020A3C);
        CHECK(loadReplayList.cardIoOverlayInsertIndexKnown);
        CHECK(loadReplayList.cardIoOverlayInsertIndex == 5u);
        CHECK(loadReplayList.cardIoMessageType == 0);
        CHECK(loadReplayList.count == 10u);
    }
}

void TestCardGridEmptyDirectoryIdleExit80020F94() {
    auto state = MakeCardGridState80020F94(8, 0);
    state.itemCount = 0;
    state.selected = 15;
    state.exitFrameState = 1;
    state.exitBlinkState = 0;
    state.cardIoFlag = 0;
    const auto list = BuildCardGridDrawList80020F94(state);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.runtimeSubmitAllowed);
    CHECK(list.rawTextureOnly);
    CHECK(!list.cardIoOverlay80020A3CRequired);
    CHECK(!list.truncated);
    CHECK(list.markerCount == 0u);
    CHECK(list.glyphSpriteCount == 0u);
    CHECK(list.count == 10u);
    CheckCardGridCommand80020F94(
        list, 7u, CardGridSpriteRole80020F94::ExitFrame,
        0x80050AD0u, 231, 179);
    CheckCardGridCommand80020F94(
        list, 8u, CardGridSpriteRole80020F94::ExitLabel,
        0x800509E0u, 242, 191);
    CheckCardGridCommand80020F94(
        list, 9u, CardGridSpriteRole80020F94::ExitBar,
        0x800509B0u, 238, 188);
}

void TestCardGridEmptyDirectoryExitFlash80020F94() {
    auto state = MakeCardGridState80020F94(8, 0);
    state.itemCount = 0;
    state.selected = 15;
    state.exitFrameState = 1;
    state.exitBlinkState = 2;
    state.cardIoFlag = 0;
    const auto list = BuildCardGridDrawList80020F94(state);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.runtimeSubmitAllowed);
    CHECK(list.rawTextureOnly);
    CHECK(!list.cardIoOverlay80020A3CRequired);
    CHECK(!list.truncated);
    CHECK(list.count == 10u);
    CheckCardGridCommand80020F94(
        list, 7u, CardGridSpriteRole80020F94::ExitFrame,
        0x80050AD0u, 231, 179);
    CheckCardGridCommand80020F94(
        list, 8u, CardGridSpriteRole80020F94::ExitLabel,
        0x800509F0u, 242, 191);
    CheckCardGridCommand80020F94(
        list, 9u, CardGridSpriteRole80020F94::ExitBar,
        0x800509C0u, 238, 188);

    state.exitBlinkState = -1;
    const auto finalFlash = BuildCardGridDrawList80020F94(state);
    CHECK(finalFlash.sourceKnown);
    CHECK(finalFlash.accepted);
    CHECK(finalFlash.complete);
    CHECK(finalFlash.runtimeSubmitAllowed);
    CHECK(finalFlash.count == 10u);
    CheckCardGridCommand80020F94(
        finalFlash, 7u, CardGridSpriteRole80020F94::ExitFrame,
        0x80050AD0u, 231, 179);
    CheckCardGridCommand80020F94(
        finalFlash, 8u, CardGridSpriteRole80020F94::ExitLabel,
        0x800509F0u, 242, 191);
    CheckCardGridCommand80020F94(
        finalFlash, 9u, CardGridSpriteRole80020F94::ExitBar,
        0x800509C0u, 238, 188);
}

void TestCardGridReplayGermanSpecialTemplate80020F94() {
    const auto state = MakeCardGridState80020F94(9, 1);
    const auto list = BuildCardGridDrawList80020F94(state);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CheckCardGridCommand80020F94(
        list, 4u, CardGridSpriteRole80020F94::LanguageText,
        0x80052950u, 213, 38);
}

void TestCardGridNoncanonicalPredicateStates80020F94() {
    constexpr int32_t kNonzeroDwords[] = {
        std::numeric_limits<int32_t>::min(), -2, 2,
        std::numeric_limits<int32_t>::max(),
    };
    for (const int32_t value : kNonzeroDwords) {
        auto state = MakeCardGridState80020F94(8, 0);
        state.itemCount = 0;
        state.selected = 15;
        state.exitFrameState = value;
        state.exitBlinkState = value;
        state.cardIoFlag = value;
        const auto list = BuildCardGridDrawList80020F94(state);
        CHECK(list.sourceKnown);
        CHECK(list.accepted);
        CHECK(!list.complete);
        CHECK(!list.runtimeSubmitAllowed);
        CHECK(list.cardIoOverlay80020A3CRequired);
        CHECK(list.cardIoMessageType == 0);
        CHECK(list.count == 10u);
        CheckCardGridCommand80020F94(
            list, 7u, CardGridSpriteRole80020F94::ExitFrame,
            0x80050AD0u, 231, 179);
        CheckCardGridCommand80020F94(
            list, 8u, CardGridSpriteRole80020F94::ExitLabel,
            0x800509F0u, 242, 191);
        CheckCardGridCommand80020F94(
            list, 9u, CardGridSpriteRole80020F94::ExitBar,
            0x800509C0u, 238, 188);
    }

    constexpr int16_t kNonzeroHalfwords[] = {
        std::numeric_limits<int16_t>::min(), -1, 2,
        std::numeric_limits<int16_t>::max(),
    };
    for (const int16_t enabled : kNonzeroHalfwords) {
        auto state = MakeCardGridState80020F94(8, 0);
        state.enabled[0] = enabled;
        const auto list = BuildCardGridDrawList80020F94(state);
        CHECK(list.sourceKnown);
        CHECK(list.accepted);
        CHECK(list.complete);
        CHECK(list.runtimeSubmitAllowed);
        CheckCardGridCommand80020F94(
            list, 10u, CardGridSpriteRole80020F94::SlotMarker,
            0x80052240u, 51, 73);
    }
}

void TestCardGridFailsClosedWithoutExactArg80020F94() {
    auto state = MakeCardGridState80020F94(8, 0);
    state.requestBound = false;
    auto list = BuildCardGridDrawList80020F94(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);
    CHECK(list.count == 0u);

    state = MakeCardGridState80020F94(8, 0);
    state.argAddress += 4u;
    list = BuildCardGridDrawList80020F94(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);

    state = MakeCardGridState80020F94(8, 0);
    state.itemCount = 16;
    list = BuildCardGridDrawList80020F94(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);

    state = MakeCardGridState80020F94(8, 0);
    state.selected = 14;
    list = BuildCardGridDrawList80020F94(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);

    state = MakeCardGridState80020F94(8, 0);
    state.slotText[0].fill('A');
    list = BuildCardGridDrawList80020F94(state);
    CHECK(!list.sourceKnown);
    CHECK(!list.accepted);
}

void TestEventBackdropStaticTemplates() {
    struct ExpectedTemplate {
        uint32_t address;
        uint32_t attr;
        uint16_t texX;
        uint16_t texY;
        uint16_t width;
        uint16_t height;
        uint16_t clutX;
        uint16_t clutY;
    };
    constexpr ExpectedTemplate kExpected[] = {
        {0x8004E7D0u, 0x50000040u, 0x0332u, 0x0100u, 0x0028u,
         0x0028u, 0x0100u, 0x01EAu},
        {0x8004E7E0u, 0x50000040u, 0x0300u, 0x0100u, 0x0014u,
         0x0064u, 0x0100u, 0x01EBu},
        {0x8004E7F0u, 0x50000040u, 0x0305u, 0x0100u, 0x0014u,
         0x0064u, 0x0100u, 0x01ECu},
        {0x8004E800u, 0x50000040u, 0x030Au, 0x0100u, 0x0014u,
         0x0064u, 0x0100u, 0x01EDu},
        {0x8004E810u, 0x50000040u, 0x030Fu, 0x0100u, 0x0014u,
         0x0064u, 0x0100u, 0x01EEu},
        {0x8004E820u, 0x50000040u, 0x0314u, 0x0100u, 0x0078u,
         0x0014u, 0x0100u, 0x01EFu},
        {0x8004E830u, 0x50000040u, 0x0314u, 0x0114u, 0x0078u,
         0x0014u, 0x0100u, 0x01F0u},
        {0x8004E840u, 0x50000040u, 0x0314u, 0x0128u, 0x0078u,
         0x0014u, 0x0100u, 0x01F1u},
        {0x8004E850u, 0x50000040u, 0x0314u, 0x013Cu, 0x0078u,
         0x0014u, 0x0100u, 0x01F2u},
        {0x8004E900u, 0x10000040u, 0x0380u, 0x0163u, 0x0014u,
         0x0014u, 0x0110u, 0x01EEu},
        {0x8004E910u, 0x10000040u, 0x0385u, 0x0163u, 0x0014u,
         0x0014u, 0x0110u, 0x01EFu},
        {0x8004E920u, 0x10000040u, 0x038Au, 0x0163u, 0x0014u,
         0x0014u, 0x0110u, 0x01F0u},
        {0x8004E930u, 0x10000040u, 0x038Fu, 0x0163u, 0x0014u,
         0x0014u, 0x0110u, 0x01F1u},
    };

    CHECK(std::size(kExpected) == 13u);
    for (const auto& expected : kExpected) {
        const auto sprite = EventBackdrop::
            ResolveEventBackdropSpriteTemplate8001B25C(expected.address);
        CHECK(sprite.known);
        CHECK(sprite.psxAddress == expected.address);
        CHECK(sprite.attr == expected.attr);
        CHECK(sprite.texX == expected.texX);
        CHECK(sprite.texY == expected.texY);
        CHECK(sprite.width == expected.width);
        CHECK(sprite.height == expected.height);
        CHECK(sprite.clutX == expected.clutX);
        CHECK(sprite.clutY == expected.clutY);
        CHECK((sprite.attr & 0x40u) != 0u);
    }
}

EventBackdrop::EventBackdropDrawList8001D74C BuildCheckedEventBackdrop() {
    const auto list = EventBackdrop::BuildEventBackdropDrawList8001D74C(
        kEventBackdropTestPriority);
    CHECK(list.sourceKnown);
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(!list.truncated);
    CHECK(list.count == 84u);
    CHECK(list.count == EventBackdrop::kEventBackdropSpriteCapacity8001D74C);
    for (uint32_t index = 0; index < list.count; ++index) {
        CHECK(list.commands[index].priority == kEventBackdropTestPriority);
        CHECK(list.commands[index].callOrder == index);
    }
    return list;
}

void TestEventBackdropFirstEightAnchors() {
    const auto list = BuildCheckedEventBackdrop();
    struct Anchor {
        uint32_t address;
        int16_t x;
        int16_t y;
    };
    constexpr Anchor kAnchors[] = {
        {0x8004E7E0u, 20, 20},   {0x8004E7F0u, 20, 120},
        {0x8004E800u, 280, 20},  {0x8004E810u, 280, 120},
        {0x8004E820u, 40, 20},   {0x8004E830u, 160, 20},
        {0x8004E840u, 40, 200},  {0x8004E850u, 160, 200},
    };
    CHECK(std::size(kAnchors) == 8u);
    for (uint32_t index = 0; index < std::size(kAnchors); ++index) {
        CheckEventBackdropCommand(list, index, kAnchors[index].address,
                                  kAnchors[index].x, kAnchors[index].y);
    }
}

void TestEventBackdropCenterEntries() {
    const auto list = BuildCheckedEventBackdrop();
    uint32_t index = 8u;
    for (int32_t x = 40; x < 280; x += 40) {
        for (int32_t y = 40; y < 200; y += 40) {
            CheckEventBackdropCommand(list, index++, 0x8004E7D0u,
                                      static_cast<int16_t>(x),
                                      static_cast<int16_t>(y));
        }
    }
    CHECK(index - 8u == 24u);
    CHECK(index == 32u);
}

void TestEventBackdropHorizontalBorderEntries() {
    const auto list = BuildCheckedEventBackdrop();
    uint32_t index = 32u;
    for (int32_t x = 40; x < 320; x += 40) {
        CheckEventBackdropCommand(list, index++, 0x8004E900u,
                                  static_cast<int16_t>(x), 0);
        CheckEventBackdropCommand(list, index++, 0x8004E910u,
                                  static_cast<int16_t>(x + 20), 0);
        CheckEventBackdropCommand(list, index++, 0x8004E920u,
                                  static_cast<int16_t>(x), 220);
        CheckEventBackdropCommand(list, index++, 0x8004E930u,
                                  static_cast<int16_t>(x + 20), 220);
    }
    CHECK(index - 32u == 28u);
    CHECK(index == 60u);
}

void TestEventBackdropVerticalBorderEntries() {
    const auto list = BuildCheckedEventBackdrop();
    uint32_t index = 60u;
    for (int32_t y = 0; y < 240; y += 40) {
        CheckEventBackdropCommand(list, index++, 0x8004E900u, 0,
                                  static_cast<int16_t>(y));
        CheckEventBackdropCommand(list, index++, 0x8004E910u, 300,
                                  static_cast<int16_t>(y));
        CheckEventBackdropCommand(list, index++, 0x8004E920u, 0,
                                  static_cast<int16_t>(y + 20));
        CheckEventBackdropCommand(list, index++, 0x8004E930u, 300,
                                  static_cast<int16_t>(y + 20));
    }
    CHECK(index - 60u == 24u);
    CHECK(index == 84u);
}

} // namespace

int main() {
    TestContextNullProducesKnownCommands();
    TestDefaultCursor3DrawList();
    TestCursorAndChoiceVariants();
    TestReplayChoice2FallsThroughToLoad();
    TestFiveLanguageBoundaryAndFixedLanguageLabel();
    TestBlinkTemplateSwitches();
    TestInvalidLanguageFailsClosed();
    TestNoncanonicalMainDirectoryStateUsesOriginalPredicates();
    TestItemValuesUsePsxPredicates();
    TestHiScoreCardIoOverlayGap();
    TestOptionsExactOrderAndCount();
    TestOptionsCursor0AllLanguagesAndSubtitleStates();
    TestOptionsCursor1AllSelectedLanguages();
    TestOptionsCursor2ExitStates();
    TestOptionsInvalidLanguageFailsClosed();
    TestOptionsNoncanonicalStateUsesOriginalPredicates();
    TestStageSelectExactOrderAndSlices();
    TestStageSelectDisabledSelectedLanguage1();
    TestStageSelectBonusAndExitStates();
    TestStageSelectFiveLanguageTitleBoundary();
    TestStageSelectNoncanonicalPredicateStates();
    TestStageSelectInvalidInputFailsClosed();
    TestPracticeGuideMinusOneExactCalls();
    TestPracticeGuideThresholdsAndFailClosed();
    TestPracticeIconExactStaticPrefixAndScaleSources();
    TestPracticeIconFailsClosedWithoutScaleOrValidArgs();
    TestPracticeWobbleBankProducerAndIconBridge();
    TestPracticeIconRgbCarrierBridge();
    TestPracticeIconExactGsSpriteDirectPayload();
    TestPracticeIconDirectPayloadFailsClosed();
    TestPracticePortraitSharedCounterAndTemplateProducer();
    TestPracticeSliceRequiresOrderedPortraitRgbWriter();
    TestPracticeSliceExactFourCallTransaction();
    TestPracticePostSliceFixedRequiresOrderedSlicePublish();
    TestPracticePostSliceFixedExactFourCallTransaction();
    TestPracticePostSliceTailFailsClosedWithoutKnownState();
    TestPracticeAlwaysVisibleTailDoesNotRequireSlicePublication();
    TestPracticePostSliceTailNoncanonicalExitStatesUseOriginalPredicates();
    TestPracticePostSliceTailExactEightCallTransaction();
    TestPracticeLeadingOverlayFailsClosedWithoutKnownState();
    TestPracticeLeadingOverlayDefaultSwitchNoOp();
    TestPracticeLeadingOverlayExactTransactions();
    TestPracticeDefaultHelperCallOrder();
    TestPracticeMaximumCase1HelperCallOrder();
    TestPracticeOverlayCaseCountsAndCase8NoHeader();
    TestPracticeInvalidSourcesFailClosed();
    TestCardGridExactLoadTransaction80020F94();
    TestCardGridExitAndIoBoundary80020F94();
    TestCardGridEmptyDirectoryIdleExit80020F94();
    TestCardGridEmptyDirectoryExitFlash80020F94();
    TestCardGridReplayGermanSpecialTemplate80020F94();
    TestCardGridNoncanonicalPredicateStates80020F94();
    TestCardGridFailsClosedWithoutExactArg80020F94();
    TestEventBackdropStaticTemplates();
    TestEventBackdropFirstEightAnchors();
    TestEventBackdropCenterEntries();
    TestEventBackdropHorizontalBorderEntries();
    TestEventBackdropVerticalBorderEntries();

    if (g_failedChecks != 0) {
        std::printf("test_ss0_directory_pages_render_direct: failed checks=%d\n",
                    g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_directory_pages_render_direct: ok\n");
    return 0;
}
