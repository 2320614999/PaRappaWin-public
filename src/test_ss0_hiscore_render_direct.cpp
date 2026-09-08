#include "pr/pr_ss0_hiscore_render_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>

namespace {

using namespace PrSS0HiScoreRenderDirect;

int g_failedChecks = 0;

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            std::printf("CHECK failed: %s:%d: %s\n", __FILE__, __LINE__,      \
                        #condition);                                            \
            ++g_failedChecks;                                                   \
        }                                                                       \
    } while (false)

using Table = std::array<uint8_t, kHiScoreTableSize80019284>;

static_assert(kHiScoreCellTextMaxBytes80021594 == 15u,
              "80021594 cell text must fit before the stride terminator");
static_assert(kHiScoreTableStaticSpriteCommandCount80021594 == 14u,
              "80021594 table must retain fourteen static sprites");
static_assert(kHiScoreSpriteCommandCapacity80021594 == 302u,
              "80021594 table command capacity must cover the full grid");
static_assert(kHiScoreNonEmptyCellTemplate80052EE0 == 0x80052EE0u,
              "80021594 nonempty cell template address must stay exact");
static_assert(kHiScoreTableStaticSpriteCommandCount80021594 +
                      kHiScoreCellCount80021594 *
                          (1u + kHiScoreCellTextMaxBytes80021594) ==
                  kHiScoreSpriteCommandCapacity80021594,
              "80021594 command capacity must cover icons and max glyphs");

void WriteS16LE(Table& table, std::size_t offset, int16_t value) {
    const uint16_t bits = static_cast<uint16_t>(value);
    table[offset] = static_cast<uint8_t>(bits & 0xFFu);
    table[offset + 1u] = static_cast<uint8_t>(bits >> 8u);
}

void WriteU32LE(Table& table, std::size_t offset, uint32_t value) {
    table[offset] = static_cast<uint8_t>(value & 0xFFu);
    table[offset + 1u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
    table[offset + 2u] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
    table[offset + 3u] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
}

Table MakeEmptyTable() {
    Table table{};
    WriteS16LE(table, kHiScoreRowsOffset80021594, 6);
    WriteS16LE(table, kHiScoreColsOffset80021594, 3);
    return table;
}

HiScoreEmptyTableInput80021594 MakeInput(const Table& table,
                                         int32_t language = 0,
                                         int32_t iconState = 0,
                                         int32_t labelState = 0) {
    HiScoreEmptyTableInput80021594 input{};
    input.requestBound80019284 = true;
    input.tableBytes80019284 = table.data();
    input.tableByteCount80019284 = table.size();
    input.languageKnown = true;
    input.language = language;
    input.translatedLiveExitIconStateKnown = true;
    input.translatedLiveExitIconState = iconState;
    input.requestBoundExitLabelStateKnown = true;
    input.requestBoundExitLabelState = labelState;
    return input;
}

void CheckFailed(const HiScoreEmptyTableDrawList80021594& list,
                 bool blockedByNonEmptyCell = false) {
    CHECK(!list.accepted);
    CHECK(!list.complete);
    CHECK(!list.rawTextureOnly);
    CHECK(list.blockedByNonEmptyCell == blockedByNonEmptyCell);
    CHECK(list.count == 0u);
    CHECK(!list.sourceKnown);
    CHECK(!list.requestBound80019284);
    CHECK(!list.truncated);
    for (const auto& command : list.commands) {
        CHECK(!command.known);
        CHECK(!command.rawTexture);
        CHECK(command.templateAddress == 0u);
    }
}

void CheckAccepted(const HiScoreEmptyTableDrawList80021594& list) {
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(!list.blockedByNonEmptyCell);
    CHECK(list.sourceKnown);
    CHECK(list.requestBound80019284);
    CHECK(!list.truncated);
    CHECK(list.count == 32u);
    CHECK(list.count == kHiScoreSpriteCommandCount80021594);
    for (std::size_t index = 0u; index < list.count; ++index) {
        const auto& command = list.commands[index];
        CHECK(command.known);
        CHECK(command.rawTexture);
        CHECK(command.descriptor.attr == 0x50000040u);
        CHECK(command.descriptor.width != 0u);
        CHECK(command.descriptor.height != 0u);
        CHECK(command.priority == 0u);
        CHECK(command.callOrder == index);
    }
}

template <typename DrawList>
void CheckCommand(const DrawList& list,
                  std::size_t index,
                  uint32_t address,
                  int16_t x,
                  int16_t y) {
    CHECK(index < list.count);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.templateAddress == address);
    CHECK(command.x == x);
    CHECK(command.y == y);
    CHECK(command.priority == 0u);
    CHECK(command.callOrder == index);
}

void CheckTableAccepted(const HiScoreTableDrawList80021594& list,
                        bool allCellsEmpty,
                        std::size_t nonEmptyCellCount,
                        std::size_t glyphSpriteCount,
                        std::size_t count) {
    CHECK(list.accepted);
    CHECK(list.complete);
    CHECK(list.rawTextureOnly);
    CHECK(list.allCellsEmpty == allCellsEmpty);
    CHECK(!list.blockedByMalformedCell);
    CHECK(list.nonEmptyCellCount == nonEmptyCellCount);
    CHECK(list.glyphSpriteCount == glyphSpriteCount);
    CHECK(list.count == count);
    CHECK(list.count == kHiScoreTableStaticSpriteCommandCount80021594 +
                            kHiScoreCellCount80021594 + glyphSpriteCount);
    CHECK(list.sourceKnown);
    CHECK(list.requestBound80019284);
    CHECK(!list.truncated);
    CHECK(list.commands.size() == kHiScoreSpriteCommandCapacity80021594);
    for (std::size_t index = 0u; index < list.count; ++index) {
        const auto& command = list.commands[index];
        CHECK(command.known);
        CHECK(command.rawTexture);
        CHECK(command.descriptor.attr == kHiScoreRawTextureAttr80021594);
        CHECK(command.descriptor.width != 0u);
        CHECK(command.descriptor.height != 0u);
        CHECK(command.priority == kHiScoreSpritePriority80021594);
        CHECK(command.callOrder == index);
    }
}

void CheckFirstZeroGlyph(const HiScoreTableDrawList80021594& list) {
    constexpr std::size_t index = 0u;
    CHECK(index < list.count);
    if (index >= list.count) {
        return;
    }
    const auto& command = list.commands[index];
    CHECK(command.x == 57);
    CHECK(command.y == 72);
    CHECK(command.descriptor.attr == 0x50000040u);
    CHECK(command.descriptor.width == 8u);
    CHECK(command.descriptor.height == 15u);
    CHECK(command.textureCoordinatesResolved);
    CHECK(command.tpage == 0x003Eu);
    CHECK(command.u == 0x83u);
    CHECK(command.v == 0u);
    CHECK(command.descriptor.clutX == 256u);
    CHECK(command.descriptor.clutY == 483u);
    CHECK(command.glyphCode == 0x30u);
}

void TestGeneralEmptyTable() {
    const Table table = MakeEmptyTable();
    const auto list = BuildHiScoreTableDrawList80021594(MakeInput(table));
    CheckTableAccepted(list, true, 0u, 0u, 32u);
    CheckCommand(list, 0u, kHiScoreEmptyCellTemplate80052F00, 55, 68);
}

void TestGeneralSingleGlyphCell() {
    Table table = MakeEmptyTable();
    table[kHiScoreCellBaseOffset80021594] = static_cast<uint8_t>('0');
    table[kHiScoreCellBaseOffset80021594 + 1u] = 0u;

    const auto list = BuildHiScoreTableDrawList80021594(MakeInput(table));
    CheckTableAccepted(list, false, 1u, 1u, 33u);
    CheckFirstZeroGlyph(list);
    CheckCommand(list, 1u, kHiScoreNonEmptyCellTemplate80052EE0, 55, 68);
}

void TestGeneralTwoGlyphCellAdvancesX() {
    Table table = MakeEmptyTable();
    table[kHiScoreCellBaseOffset80021594] = static_cast<uint8_t>('0');
    table[kHiScoreCellBaseOffset80021594 + 1u] =
        static_cast<uint8_t>('1');
    table[kHiScoreCellBaseOffset80021594 + 2u] = 0u;

    const auto list = BuildHiScoreTableDrawList80021594(MakeInput(table));
    CheckTableAccepted(list, false, 1u, 2u, 34u);
    CheckFirstZeroGlyph(list);
    CHECK(list.commands[1u].x == 65);
    CHECK(list.commands[1u].glyphCode == 0x31u);
    CheckCommand(list, 2u, kHiScoreNonEmptyCellTemplate80052EE0, 55, 68);
}

void TestGeneralMissingCellTerminatorFailsClosed() {
    Table table = MakeEmptyTable();
    for (std::size_t byte = 0u; byte < kHiScoreCellStride80021594; ++byte) {
        table[kHiScoreCellBaseOffset80021594 + byte] =
            static_cast<uint8_t>('0');
    }

    const auto list = BuildHiScoreTableDrawList80021594(MakeInput(table));
    CHECK(!list.accepted);
    CHECK(!list.complete);
    CHECK(!list.rawTextureOnly);
    CHECK(!list.allCellsEmpty);
    CHECK(list.blockedByMalformedCell);
    CHECK(list.nonEmptyCellCount == 0u);
    CHECK(list.glyphSpriteCount == 0u);
    CHECK(list.count == 0u);
    CHECK(!list.sourceKnown);
    CHECK(!list.requestBound80019284);
    CHECK(!list.truncated);
}

void TestGeneralFullTableReachesExactCapacity() {
    Table table = MakeEmptyTable();
    for (std::size_t cell = 0u; cell < kHiScoreCellCount80021594; ++cell) {
        const std::size_t offset = kHiScoreCellBaseOffset80021594 +
                                   cell * kHiScoreCellStride80021594;
        for (std::size_t byte = 0u;
             byte < kHiScoreCellTextMaxBytes80021594;
             ++byte) {
            table[offset + byte] = static_cast<uint8_t>('0');
        }
        table[offset + kHiScoreCellTextMaxBytes80021594] = 0u;
    }

    const auto list = BuildHiScoreTableDrawList80021594(MakeInput(table));
    CheckTableAccepted(list,
                       false,
                       kHiScoreCellCount80021594,
                       kHiScoreCellCount80021594 *
                           kHiScoreCellTextMaxBytes80021594,
                       kHiScoreSpriteCommandCapacity80021594);
    CHECK(list.count == list.commands.size());
    CHECK(!list.truncated);
}

void TestGeneralNonRenderableByteStillUsesNonEmptyIcon() {
    Table table = MakeEmptyTable();
    table[kHiScoreCellBaseOffset80021594] = 0x01u;
    table[kHiScoreCellBaseOffset80021594 + 1u] = 0u;

    const auto list = BuildHiScoreTableDrawList80021594(MakeInput(table));
    CheckTableAccepted(list, false, 1u, 0u, 32u);
    CheckCommand(list, 0u, kHiScoreNonEmptyCellTemplate80052EE0, 55, 68);
}

void TestSuccessOffAndExactCallOrder() {
    const Table table = MakeEmptyTable();
    const auto list =
        BuildHiScoreEmptyTableDrawList80021594(MakeInput(table, 0, 0, 0));
    CheckAccepted(list);

    std::size_t index = 0u;
    for (int16_t row = 0; row < 6; ++row) {
        for (int16_t col = 0; col < 3; ++col) {
            CheckCommand(list,
                         index++,
                         0x80052F00u,
                         static_cast<int16_t>(55 + 79 * col),
                         static_cast<int16_t>(68 + 18 * row));
        }
    }
    CHECK(index == 18u);
    CheckCommand(list, 18u, 0x80052E60u, 52, 34);

    constexpr uint32_t kFixedAddress[] = {
        0x80052E50u, 0x80052EB0u, 0x80052EC0u, 0x80052ED0u,
        0x80052F10u, 0x80052F20u, 0x80052F30u, 0x80052F40u,
        0x80052F50u, 0x80052F60u,
    };
    constexpr int16_t kFixedX[] = {37, 84, 163, 242, 37, 37, 37, 37, 37, 37};
    constexpr int16_t kFixedY[] = {31, 56, 56, 56, 69, 87, 105, 123, 141, 159};
    for (std::size_t fixed = 0u; fixed < 10u; ++fixed) {
        CheckCommand(list,
                     19u + fixed,
                     kFixedAddress[fixed],
                     kFixedX[fixed],
                     kFixedY[fixed]);
    }

    CheckCommand(list, 29u, 0x80050AC0u, 231, 179);
    CheckCommand(list, 30u, 0x800509E0u, 242, 191);
    CheckCommand(list, 31u, 0x800509B0u, 238, 188);

    const auto& first = list.commands[0];
    CHECK(first.descriptor.texX == 0x0280u);
    CHECK(first.descriptor.texY == 0x003Cu);
    CHECK(first.descriptor.width == 0x0050u);
    CHECK(first.descriptor.height == 0x0014u);
    CHECK(first.descriptor.clutX == 0x03F0u);
    CHECK(first.descriptor.clutY == 0x0041u);

    const auto& last = list.commands[31];
    CHECK(last.descriptor.texX == 0x0140u);
    CHECK(last.descriptor.texY == 0x0000u);
    CHECK(last.descriptor.width == 0x0030u);
    CHECK(last.descriptor.height == 0x0011u);
    CHECK(last.descriptor.clutX == 0x03C0u);
    CHECK(last.descriptor.clutY == 0x0003u);
}

void TestSuccessOn() {
    Table table = MakeEmptyTable();
    WriteU32LE(table, kHiScoreExitLabelStateOffset80021594, 1u);
    const auto list =
        BuildHiScoreEmptyTableDrawList80021594(MakeInput(table, 4, 1, 1));
    CheckAccepted(list);
    CheckCommand(list, 18u, 0x80052EA0u, 40, 34);
    CheckCommand(list, 29u, 0x80050AD0u, 231, 179);
    CheckCommand(list, 30u, 0x80050AB0u, 239, 191);
    CheckCommand(list, 31u, 0x800509C0u, 238, 188);
    CHECK(list.commands[29].descriptor.clutY == 0x0001u);
    CHECK(list.commands[30].descriptor.clutY == 0x0007u);
    CHECK(list.commands[31].descriptor.clutY == 0x0004u);
}

void TestExitIconUsesExactEqualsOnePredicate() {
    const Table table = MakeEmptyTable();
    for (const int32_t iconState : {-7, 0, 2, 7}) {
        const auto list = BuildHiScoreEmptyTableDrawList80021594(
            MakeInput(table, 0, iconState, 0));
        CheckAccepted(list);
        CheckCommand(list, 29u, 0x80050AC0u, 231, 179);
    }

    const auto on = BuildHiScoreEmptyTableDrawList80021594(
        MakeInput(table, 0, 1, 0));
    CheckAccepted(on);
    CheckCommand(on, 29u, 0x80050AD0u, 231, 179);
}

void TestExitLabelUsesNonzeroPredicate() {
    constexpr std::array<int32_t, 6u> kNonzeroStates = {
        std::numeric_limits<int32_t>::min(),
        -2,
        -1,
        1,
        2,
        std::numeric_limits<int32_t>::max(),
    };
    for (const int32_t labelState : kNonzeroStates) {
        Table table = MakeEmptyTable();
        WriteU32LE(table,
                   kHiScoreExitLabelStateOffset80021594,
                   static_cast<uint32_t>(labelState));
        const auto list = BuildHiScoreEmptyTableDrawList80021594(
            MakeInput(table, 0, 0, labelState));
        CheckAccepted(list);
        CheckCommand(list, 30u, 0x800509F0u, 242, 191);
        CheckCommand(list, 31u, 0x800509C0u, 238, 188);
    }

    const Table table = MakeEmptyTable();
    const auto off = BuildHiScoreEmptyTableDrawList80021594(
        MakeInput(table, 0, 0, 0));
    CheckAccepted(off);
    CheckCommand(off, 30u, 0x800509E0u, 242, 191);
    CheckCommand(off, 31u, 0x800509B0u, 238, 188);
}

void TestAllLanguages() {
    constexpr uint32_t kTitleAddress[5] = {
        0x80052E60u, 0x80052E80u, 0x80052E70u, 0x80052E90u, 0x80052EA0u,
    };
    constexpr int16_t kTitleX[5] = {52, 49, 42, 61, 40};
    constexpr uint32_t kExitOffAddress[5] = {
        0x800509E0u, 0x80050A40u, 0x80050A10u, 0x80050A70u, 0x80050AA0u,
    };
    constexpr uint32_t kExitOnAddress[5] = {
        0x800509F0u, 0x80050A50u, 0x80050A20u, 0x80050A80u, 0x80050AB0u,
    };
    constexpr int16_t kExitX[5] = {242, 238, 238, 241, 239};
    constexpr int16_t kExitY[5] = {191, 193, 193, 190, 191};

    const Table offTable = MakeEmptyTable();
    Table onTable = MakeEmptyTable();
    WriteU32LE(onTable, kHiScoreExitLabelStateOffset80021594, 1u);
    for (int32_t language = 0; language < 5; ++language) {
        const auto off = BuildHiScoreEmptyTableDrawList80021594(
            MakeInput(offTable, language, 0, 0));
        const auto on = BuildHiScoreEmptyTableDrawList80021594(
            MakeInput(onTable, language, 1, 1));
        CheckAccepted(off);
        CheckAccepted(on);
        CheckCommand(off, 18u, kTitleAddress[language], kTitleX[language], 34);
        CheckCommand(on, 18u, kTitleAddress[language], kTitleX[language], 34);
        CheckCommand(off,
                     30u,
                     kExitOffAddress[language],
                     kExitX[language],
                     kExitY[language]);
        CheckCommand(on,
                     30u,
                     kExitOnAddress[language],
                     kExitX[language],
                     kExitY[language]);
    }
}

void TestInvalidSizeAndBindingFailClosed() {
    const Table table = MakeEmptyTable();
    auto input = MakeInput(table);
    input.requestBound80019284 = false;
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(input));

    input = MakeInput(table);
    input.tableBytes80019284 = nullptr;
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(input));
    input = MakeInput(table);
    input.tableByteCount80019284 = table.size() - 1u;
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(input));
    input = MakeInput(table);
    input.tableByteCount80019284 = table.size() + 1u;
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(input));
}

void TestInvalidShapeFailsClosed() {
    Table table = MakeEmptyTable();
    WriteS16LE(table, kHiScoreRowsOffset80021594, 5);
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(MakeInput(table)));
    table = MakeEmptyTable();
    WriteS16LE(table, kHiScoreColsOffset80021594, 4);
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(MakeInput(table)));
}

void TestInvalidLanguageFailsClosed() {
    const Table table = MakeEmptyTable();
    auto input = MakeInput(table);
    input.languageKnown = false;
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(input));
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(
        MakeInput(table, -1, 0, 0)));
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(
        MakeInput(table, 5, 0, 0)));
}

void TestUnknownAndMismatchedStatesFailClosed() {
    const Table table = MakeEmptyTable();
    auto input = MakeInput(table);
    input.translatedLiveExitIconStateKnown = false;
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(input));

    input = MakeInput(table);
    input.requestBoundExitLabelStateKnown = false;
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(input));

    // The request value must still match the raw table dword exactly.
    input = MakeInput(table, 0, 0, 1);
    CheckFailed(BuildHiScoreEmptyTableDrawList80021594(input));
}

void TestEveryNonEmptyCellFailsClosed() {
    for (std::size_t cell = 0u; cell < kHiScoreCellCount80021594; ++cell) {
        Table table = MakeEmptyTable();
        table[kHiScoreCellBaseOffset80021594 +
              cell * kHiScoreCellStride80021594] = 1u;
        CheckFailed(BuildHiScoreEmptyTableDrawList80021594(MakeInput(table)),
                    true);
    }
}

}  // namespace

int main() {
    TestGeneralEmptyTable();
    TestGeneralSingleGlyphCell();
    TestGeneralTwoGlyphCellAdvancesX();
    TestGeneralMissingCellTerminatorFailsClosed();
    TestGeneralFullTableReachesExactCapacity();
    TestGeneralNonRenderableByteStillUsesNonEmptyIcon();
    TestSuccessOffAndExactCallOrder();
    TestSuccessOn();
    TestExitIconUsesExactEqualsOnePredicate();
    TestExitLabelUsesNonzeroPredicate();
    TestAllLanguages();
    TestInvalidSizeAndBindingFailClosed();
    TestInvalidShapeFailsClosed();
    TestInvalidLanguageFailsClosed();
    TestUnknownAndMismatchedStatesFailClosed();
    TestEveryNonEmptyCellFailsClosed();

    if (g_failedChecks != 0) {
        std::printf("test_ss0_hiscore_render_direct: failed checks=%d\n",
                    g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_hiscore_render_direct: ok\n");
    return 0;
}
