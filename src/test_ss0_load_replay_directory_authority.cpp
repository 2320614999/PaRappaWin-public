#include "pr/pr_ss0_card_memcard_handoff_direct.h"

#include <cstdio>

using namespace PrSS0CardMemcardHandoffDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

void CheckSameCardDriverVisualRuntime80018FB0(
    const CardDriverVisualRuntime80018FB0& actual,
    const CardDriverVisualRuntime80018FB0& expected) {
    CHECK(actual.known == expected.known);
    CHECK(actual.state == expected.state);
    CHECK(actual.eventId == expected.eventId);
    CHECK(actual.phase == expected.phase);
    CHECK(actual.blinkCounter8006ED18 == expected.blinkCounter8006ED18);
    CHECK(actual.promptFlashFramesRemaining80017E6C ==
          expected.promptFlashFramesRemaining80017E6C);
    CHECK(actual.exitFrameStateArg0 == expected.exitFrameStateArg0);
    CHECK(actual.exitBlinkStateArg4 == expected.exitBlinkStateArg4);
    CHECK(actual.cardIoFlagArg8 == expected.cardIoFlagArg8);
    CHECK(actual.gp720Known == expected.gp720Known);
    CHECK(actual.gp720 == expected.gp720);
}

void SetTitle(char (&dst)[kCardDirectoryNameMax80017900 + 1u],
              const char* text) {
    uint32_t i = 0;
    for (; i < kCardDirectoryNameMax80017900 && text[i] != '\0'; ++i) {
        dst[i] = text[i];
    }
    for (; i <= kCardDirectoryNameMax80017900; ++i) {
        dst[i] = '\0';
    }
}

LoadReplayDirectoryTypedCarrier80019D7C MakeDirectory(
    CardMode800191E4 mode,
    int32_t entryCount) {
    LoadReplayDirectoryTypedCarrier80019D7C carrier{};
    carrier.known = true;
    carrier.mode = mode;
    carrier.entryCountKnown = true;
    carrier.entryCount = entryCount;
    for (int32_t row = 0; row < entryCount; ++row) {
        carrier.rows[row].blockIndexKnown = true;
        carrier.rows[row].blockIndex = row + 2;
        carrier.rows[row].rowNameKnown8007A590 = true;
        SetTitle(carrier.rows[row].rowName8007A590,
                 row == 0 ? "BASCUS-94183A" : "BASCUS-94183B");
        carrier.rows[row].titleKnown = true;
        SetTitle(carrier.rows[row].title, row == 0 ? "A" : "B");
    }
    return carrier;
}

LoadReplayDirectoryScanFacts80019D7C MakeScanFacts(
    CardMode800191E4 mode,
    int32_t entryCount) {
    LoadReplayDirectoryScanFacts80019D7C facts{};
    facts.known = true;
    facts.mode = mode;
    facts.directoryRowsKnown80017B08 = true;
    facts.snapshotKnown80017B18 = true;
    facts.listRowsBuilt80019D7C = true;
    facts.entryCountKnown = true;
    facts.entryCount = entryCount;
    for (int32_t row = 0; row < entryCount; ++row) {
        facts.rows[row].blockIndexKnown = true;
        facts.rows[row].blockIndex = row + 4;
        facts.rows[row].rowNameKnown8007A590 = true;
        SetTitle(facts.rows[row].rowName8007A590,
                 row == 0 ? "BASCUS-94183C" : "BASCUS-94183D");
        facts.rows[row].titleKnown = true;
        SetTitle(facts.rows[row].title, row == 0 ? "C" : "D");
    }
    return facts;
}

LoadReplayState16CardIoResultCarrier80017594
MakeState16CardIoResult80017594(CardMode800191E4 mode,
                                int32_t selectedBlock,
                                int32_t ioResult) {
    LoadReplayState16CardIoResultCarrier80017594 carrier{};
    carrier.known = true;
    carrier.source =
        LoadReplayState16CardIoResultSource80017594::
            RuntimeCardEventProducer;
    carrier.requestBound = true;
    carrier.mode = mode;
    carrier.stateKnown = true;
    carrier.state = 16;
    carrier.selectedBlockKnown = true;
    carrier.selectedBlock = selectedBlock;
    carrier.ioResultKnown80017594 = true;
    carrier.ioResult80017594 = ioResult;
    carrier.producerWired80017594 = true;
    return carrier;
}

void TestGenericPublishCannotPromoteRuntimeDirectoryProducer() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    LoadReplayDirectoryTypedCarrier80019D7C carrier =
        MakeDirectory(CardMode800191E4::Load, 1);

    CHECK(!PublishLoadReplayDirectoryTypedCarrier80019D7C(carrier));

    LoadReplayDirectoryTypedCarrier80019D7C out{};
    CHECK(!GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Load, &out));
    CHECK(!out.known);
    CHECK(out.source ==
          LoadReplayDirectoryTypedCarrierSource80019D7C::Unknown);
}

void TestRuntimePublishAcceptsMatchingModeAndRows() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    LoadReplayDirectoryTypedCarrier80019D7C carrier =
        MakeDirectory(CardMode800191E4::Load, 2);

    CHECK(PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(carrier));

    LoadReplayDirectoryTypedCarrier80019D7C out{};
    CHECK(GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Load, &out));
    CHECK(out.known);
    CHECK(out.source ==
          LoadReplayDirectoryTypedCarrierSource80019D7C::
              RuntimeDirectoryProducer);
    CHECK(out.producerWired80017B08_80017B18_80019D7C);
    CHECK(out.entryCountKnown);
    CHECK(out.entryCount == 2);
    CHECK(out.rows[0].blockIndexKnown);
    CHECK(out.rows[0].blockIndex == 2);
    CHECK(out.rows[0].titleKnown);
    CHECK(out.rows[0].title[0] == 'A');
    CHECK(out.rows[0].rowName8007A590[0] == 'B');
    CHECK(!out.incomplete);

    LoadReplayDirectoryTypedCarrier80019D7C wrongMode{};
    CHECK(!GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Replay, &wrongMode));
    CHECK(!wrongMode.known);
}

void TestGetDoesNotConsumeAndClearIsExplicit() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    CHECK(PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(
        MakeDirectory(CardMode800191E4::Replay, 1)));

    LoadReplayDirectoryTypedCarrier80019D7C first{};
    LoadReplayDirectoryTypedCarrier80019D7C second{};
    CHECK(GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Replay, &first));
    CHECK(GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Replay, &second));
    CHECK(first.rows[0].blockIndex == second.rows[0].blockIndex);

    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    LoadReplayDirectoryTypedCarrier80019D7C afterClear{};
    CHECK(!GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Replay, &afterClear));
}

void TestInvalidRuntimePublishClearsPreviousCarrier() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    CHECK(PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(
        MakeDirectory(CardMode800191E4::Load, 1)));

    LoadReplayDirectoryTypedCarrier80019D7C invalid =
        MakeDirectory(CardMode800191E4::Load, 1);
    invalid.rows[0].titleKnown = false;
    CHECK(!PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(invalid));

    LoadReplayDirectoryTypedCarrier80019D7C out{};
    CHECK(!GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Load, &out));
}

void TestRejectsBadModeAndEntryCount() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    LoadReplayDirectoryTypedCarrier80019D7C badMode =
        MakeDirectory(CardMode800191E4::Save, 1);
    CHECK(!PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(badMode));

    LoadReplayDirectoryTypedCarrier80019D7C tooMany =
        MakeDirectory(CardMode800191E4::Load, 0);
    tooMany.entryCount = static_cast<int32_t>(kSaveListRowCount80019458) + 1;
    CHECK(!PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(tooMany));
}

void TestTitleIsNormalizedToDirectoryNameLimit() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    LoadReplayDirectoryTypedCarrier80019D7C carrier =
        MakeDirectory(CardMode800191E4::Load, 1);
    for (uint32_t i = 0; i <= kCardDirectoryNameMax80017900; ++i) {
        carrier.rows[0].title[i] = 'Z';
    }

    CHECK(PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(carrier));

    LoadReplayDirectoryTypedCarrier80019D7C out{};
    CHECK(GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Load, &out));
    CHECK(out.rows[0].title[kCardDirectoryNameMax80017900] == '\0');
}

void TestScanFactsBuildRuntimeDirectoryCarrier() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    LoadReplayDirectoryScanFacts80019D7C facts =
        MakeScanFacts(CardMode800191E4::Replay, 2);

    LoadReplayDirectoryTypedCarrier80019D7C carrier{};
    CHECK(BuildRuntimeLoadReplayDirectoryCarrierFromScanFacts80019D7C(
        facts, &carrier));
    CHECK(carrier.source ==
          LoadReplayDirectoryTypedCarrierSource80019D7C::
              RuntimeDirectoryProducer);
    CHECK(carrier.producerWired80017B08_80017B18_80019D7C);
    CHECK(!carrier.incomplete);
    CHECK(carrier.rows[0].blockIndex == 4);

    CHECK(PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(carrier));
    LoadReplayDirectoryTypedCarrier80019D7C out{};
    CHECK(GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Replay, &out));
    CHECK(out.entryCount == 2);
    CHECK(out.rows[1].blockIndex == 5);
}

void TestScanFactsRejectMissingDirectorySource() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    LoadReplayDirectoryScanFacts80019D7C facts =
        MakeScanFacts(CardMode800191E4::Load, 1);
    facts.snapshotKnown80017B18 = false;

    LoadReplayDirectoryTypedCarrier80019D7C carrier{};
    CHECK(!BuildRuntimeLoadReplayDirectoryCarrierFromScanFacts80019D7C(
        facts, &carrier));
    CHECK(carrier.incomplete);
    CHECK(carrier.source ==
          LoadReplayDirectoryTypedCarrierSource80019D7C::Unknown);
    CHECK(!PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(carrier));

    LoadReplayDirectoryTypedCarrier80019D7C out{};
    CHECK(!GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Load, &out));
}

void TestScanFactsRejectUnknownRowsAndBadMode() {
    LoadReplayDirectoryScanFacts80019D7C missingRow =
        MakeScanFacts(CardMode800191E4::Load, 1);
    missingRow.rows[0].blockIndexKnown = false;
    LoadReplayDirectoryTypedCarrier80019D7C carrier{};
    CHECK(!BuildRuntimeLoadReplayDirectoryCarrierFromScanFacts80019D7C(
        missingRow, &carrier));
    CHECK(carrier.incomplete);
    CHECK(!carrier.producerWired80017B08_80017B18_80019D7C);

    LoadReplayDirectoryScanFacts80019D7C badMode =
        MakeScanFacts(CardMode800191E4::Save, 1);
    CHECK(!BuildRuntimeLoadReplayDirectoryCarrierFromScanFacts80019D7C(
        badMode, &carrier));
    CHECK(carrier.incomplete);
}

void TestLoadReplayCardDriverVisualRuntime80018FB0() {
    CardDriverVisualRuntime80018FB0 runtime{};
    CHECK(InitLoadReplayCardDriverVisualRuntime80018FB0(
        CardMode800191E4::Load, &runtime));
    CHECK(runtime.known);
    CHECK(runtime.state == 12);
    CHECK(runtime.eventId == CardEventFrameId::Load);
    CHECK(runtime.phase == CardDriverVisualPhase80018FB0::ListIdle);
    CHECK(runtime.blinkCounter8006ED18 == 0u);
    CHECK(runtime.promptFlashFramesRemaining80017E6C == 0u);
    CHECK(runtime.exitFrameStateArg0 == 1);
    CHECK(runtime.exitBlinkStateArg4 == 0);
    CHECK(runtime.cardIoFlagArg8 == 0);
    CHECK(!runtime.gp720Known);
    CHECK(runtime.gp720 == 0);

    CHECK(TickCardDriverVisualRuntime80018FB0(&runtime));
    CHECK(runtime.blinkCounter8006ED18 == 1u);
    CHECK(runtime.exitFrameStateArg0 == 0);
    for (uint32_t i = 0; i < 18u; ++i) {
        CHECK(TickCardDriverVisualRuntime80018FB0(&runtime));
    }
    CHECK(runtime.blinkCounter8006ED18 == 19u);
    CHECK(runtime.exitFrameStateArg0 == 0);
    CHECK(TickCardDriverVisualRuntime80018FB0(&runtime));
    CHECK(runtime.blinkCounter8006ED18 == 0u);
    CHECK(runtime.exitFrameStateArg0 == 0);
    CHECK(TickCardDriverVisualRuntime80018FB0(&runtime));
    CHECK(runtime.blinkCounter8006ED18 == 1u);
    CHECK(runtime.exitFrameStateArg0 == 1);

    CHECK(InitLoadReplayCardDriverVisualRuntime80018FB0(
        CardMode800191E4::Replay, &runtime));
    CHECK(runtime.state == 13);
    CHECK(runtime.eventId == CardEventFrameId::Replay);
    runtime.state = 12;
    CHECK(!TickCardDriverVisualRuntime80018FB0(&runtime));

    CHECK(!InitLoadReplayCardDriverVisualRuntime80018FB0(
        CardMode800191E4::Save, &runtime));
    CHECK(!runtime.known);
    CHECK(!TickCardDriverVisualRuntime80018FB0(&runtime));
    CHECK(!InitLoadReplayCardDriverVisualRuntime80018FB0(
        CardMode800191E4::Load, nullptr));
    CHECK(!TickCardDriverVisualRuntime80018FB0(nullptr));
}

void TestLoadReplayListInputRuntime800181D0() {
    LoadReplayListInputRuntime800181D0 runtime{};
    CHECK(InitLoadReplayListInputRuntime800181D0(
        CardMode800191E4::Load, 2, &runtime));
    CHECK(runtime.requestBound);
    CHECK(runtime.mode == CardMode800191E4::Load);
    CHECK(runtime.state == 12);
    CHECK(runtime.step == 3);
    CHECK(runtime.itemCount == 16);
    CHECK(runtime.entryCount == 2);
    CHECK(runtime.selected == 0);
    CHECK(runtime.enabled[0] == 1);
    CHECK(runtime.enabled[1] == 1);
    CHECK(runtime.enabled[2] == 0);
    CHECK(runtime.enabled[15] == 1);

    auto input = TickLoadReplayListInput800181D0(
        &runtime, kInputNameRight800185D0);
    CHECK(input.accepted);
    CHECK(input.action ==
          LoadReplayListInputAction800181D0::MoveSelection);
    CHECK(input.previousSelected == 0);
    CHECK(input.selected == 1);
    CHECK(input.playSfx);
    CHECK(input.sfx == 0x1000);

    input = TickLoadReplayListInput800181D0(
        &runtime, kInputNameDown800185D0);
    CHECK(input.selected == 15);
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputNameUp800185D0);
    CHECK(input.selected == 1);
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputNameLeft800185D0);
    CHECK(input.selected == 0);
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputNameLeft800185D0);
    CHECK(input.selected == 15);
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputNameRight800185D0);
    CHECK(input.selected == 0);

    input = TickLoadReplayListInput800181D0(
        &runtime, kInputCross800185D0);
    CHECK(input.action == LoadReplayListInputAction800181D0::SelectEntry);
    CHECK(input.listResult == 1);
    CHECK(input.eventId == CardEventFrameId::Load);
    CHECK(input.promptArg2 == 2);
    CHECK(input.promptArg3 == 1);
    CHECK(input.nextState == 16);
    CHECK(input.playSfx);
    CHECK(input.sfx == 0x20);

    const int32_t selectedBeforeIgnoredFaceInput = runtime.selected;
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputCircle800185D0);
    CHECK(input.accepted);
    CHECK(input.action == LoadReplayListInputAction800181D0::NoChange);
    CHECK(input.listResult == 0);
    CHECK(!input.playSfx);
    CHECK(input.nextState == 12);
    CHECK(runtime.selected == selectedBeforeIgnoredFaceInput);
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputTriangle800185D0);
    CHECK(input.accepted);
    CHECK(input.action == LoadReplayListInputAction800181D0::NoChange);
    CHECK(input.listResult == 0);
    CHECK(!input.playSfx);
    CHECK(input.nextState == 12);
    CHECK(runtime.selected == selectedBeforeIgnoredFaceInput);

    runtime.selected = 15;
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputCross800185D0);
    CHECK(input.action == LoadReplayListInputAction800181D0::SelectExit);
    CHECK(input.listResult == 2);
    CHECK(input.eventId == CardEventFrameId::Load);
    CHECK(input.promptArg2 == 2);
    CHECK(input.promptArg3 == 0);
    CHECK(input.nextState == 23);

    CHECK(InitLoadReplayListInputRuntime800181D0(
        CardMode800191E4::Replay, 1, &runtime));
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputCross800185D0);
    CHECK(input.action == LoadReplayListInputAction800181D0::SelectEntry);
    CHECK(input.eventId == CardEventFrameId::Replay);
    CHECK(input.promptArg2 == 1);
    CHECK(input.promptArg3 == 1);
    CHECK(input.nextState == 16);

    CHECK(InitLoadReplayListInputRuntime800181D0(
        CardMode800191E4::Load, 0, &runtime));
    CHECK(runtime.selected == 15);
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputCross800185D0);
    CHECK(input.action == LoadReplayListInputAction800181D0::SelectExit);
    CHECK(input.listResult == 2);

    runtime.enabled[15] = 0;
    input = TickLoadReplayListInput800181D0(
        &runtime, kInputCross800185D0);
    CHECK(!input.accepted);
    CHECK(input.action == LoadReplayListInputAction800181D0::Rejected);
    CHECK(!InitLoadReplayListInputRuntime800181D0(
        CardMode800191E4::Save, 1, &runtime));
    CHECK(!runtime.requestBound);
    CHECK(!InitLoadReplayListInputRuntime800181D0(
        CardMode800191E4::Load, 16, &runtime));
    CHECK(!InitLoadReplayListInputRuntime800181D0(
        CardMode800191E4::Load, 1, nullptr));
    input = TickLoadReplayListInput800181D0(nullptr, kInputCross800185D0);
    CHECK(!input.accepted);
}

void TestSelectedRowIdentity800181D0() {
    ClearLoadReplayDirectoryTypedCarrier80019D7C();
    CHECK(PublishRuntimeLoadReplayDirectoryTypedCarrier80019D7C(
        MakeDirectory(CardMode800191E4::Load, 2)));
    LoadReplayDirectoryTypedCarrier80019D7C directory{};
    CHECK(GetLoadReplayDirectoryTypedCarrier80019D7C(
        CardMode800191E4::Load, &directory));

    LoadReplayListInputRuntime800181D0 list{};
    CHECK(InitLoadReplayListInputRuntime800181D0(
        CardMode800191E4::Load, 2, &list));
    CHECK(TickLoadReplayListInput800181D0(
              &list, kInputNameRight800185D0)
              .selected == 1);
    const auto selection = TickLoadReplayListInput800181D0(
        &list, kInputCross800185D0);

    LoadReplaySelectedRowIdentity800181D0 identity{};
    CHECK(CommitLoadReplaySelectedRowIdentity800181D0(
        directory, selection, &identity));
    CHECK(identity.committed);
    CHECK(identity.mode == CardMode800191E4::Load);
    CHECK(identity.selectedRow == 1);
    CHECK(identity.blockIndexKnown);
    CHECK(identity.blockIndex == 3);
    CHECK(identity.gp716Known);
    CHECK(identity.gp716 == 1);
    CHECK(identity.nameBuffer8007CBE8Known);
    CHECK(identity.nameBuffer8007CBE8[12] == 'B');

    LoadReplayDirectoryTypedCarrier80019D7C missingName = directory;
    missingName.rows[1].rowNameKnown8007A590 = false;
    CHECK(!CommitLoadReplaySelectedRowIdentity800181D0(
        missingName, selection, &identity));
    CHECK(!identity.committed);
    CHECK(!identity.gp716Known);
    CHECK(!identity.nameBuffer8007CBE8Known);

    LoadReplayDirectoryTypedCarrier80019D7C untrusted = directory;
    untrusted.source = LoadReplayDirectoryTypedCarrierSource80019D7C::Unknown;
    untrusted.producerWired80017B08_80017B18_80019D7C = false;
    CHECK(!CommitLoadReplaySelectedRowIdentity800181D0(
        untrusted, selection, &identity));

    list.selected = kLoadReplayListTerminalRow800181D0;
    const auto exitSelection = TickLoadReplayListInput800181D0(
        &list, kInputCross800185D0);
    CHECK(!CommitLoadReplaySelectedRowIdentity800181D0(
        directory, exitSelection, &identity));

    LoadReplayListInputRuntime800181D0 replayList{};
    CHECK(InitLoadReplayListInputRuntime800181D0(
        CardMode800191E4::Replay, 1, &replayList));
    const auto replaySelection = TickLoadReplayListInput800181D0(
        &replayList, kInputCross800185D0);
    CHECK(!CommitLoadReplaySelectedRowIdentity800181D0(
        directory, replaySelection, &identity));
    CHECK(!CommitLoadReplaySelectedRowIdentity800181D0(
        directory, selection, nullptr));
}

void TestLoadReplayState16Completion80019D7C() {
    const CardMode800191E4 modes[] = {
        CardMode800191E4::Load,
        CardMode800191E4::Replay,
    };

    for (const CardMode800191E4 mode : modes) {
        const int32_t expectedArg4 =
            mode == CardMode800191E4::Load ? 2 : 1;
        LoadReplayListInputRuntime800181D0 list{};
        CardDriverVisualRuntime80018FB0 visual{};
        CHECK(InitLoadReplayListInputRuntime800181D0(mode, 1, &list));
        const auto selection = TickLoadReplayListInput800181D0(
            &list, kInputCross800185D0);
        CHECK(InitLoadReplayCardDriverVisualRuntime80018FB0(mode, &visual));
        CHECK(!IsLoadReplayState16AwaitingPayload80019D7C(visual));
        CHECK(BeginListEntrySelectionFlash80017E6C(&visual, selection));
        CHECK(visual.phase ==
              CardDriverVisualPhase80018FB0::EntrySelectionFlash80017E6C);
        CHECK(visual.promptFlashFramesRemaining80017E6C == 20u);
        CHECK(visual.exitFrameStateArg0 == 1);
        CHECK(visual.exitBlinkStateArg4 == expectedArg4);
        CHECK(visual.cardIoFlagArg8 == 1);
        CHECK(!visual.gp720Known);
        CHECK(!TickCardDriverVisualRuntime80018FB0(&visual));

        for (uint32_t frame = 0; frame < 19u; ++frame) {
            CHECK(TickListEntrySelectionFlash80017E6C(&visual) ==
                  CardDriverEntryTickResult80017E6C::RenderSelectionFlash);
        }
        CHECK(visual.promptFlashFramesRemaining80017E6C == 1u);
        CHECK(TickListEntrySelectionFlash80017E6C(&visual) ==
              CardDriverEntryTickResult80017E6C::EnterState16);
        CHECK(visual.state == 16);
        CHECK(visual.phase ==
              CardDriverVisualPhase80018FB0::
                  State16AwaitingPayload80019D7C);
        CHECK(visual.promptFlashFramesRemaining80017E6C == 0u);
        CHECK(visual.exitBlinkStateArg4 == expectedArg4);
        CHECK(visual.cardIoFlagArg8 == 1);
        CHECK(!visual.gp720Known);
        CHECK(IsLoadReplayState16AwaitingPayload80019D7C(visual));
        CHECK(TickListEntrySelectionFlash80017E6C(&visual) ==
              CardDriverEntryTickResult80017E6C::Rejected);
        CHECK(!BeginListEntrySelectionFlash80017E6C(&visual, selection));

        CardDriverVisualRuntime80018FB0 badPhase = visual;
        badPhase.phase = CardDriverVisualPhase80018FB0::Complete;
        const CardDriverVisualRuntime80018FB0 badPhaseBefore = badPhase;
        CHECK(!IsLoadReplayState16AwaitingPayload80019D7C(badPhase));
        CHECK(!BeginLoadReplayState16Completion80019D7C(&badPhase));
        CheckSameCardDriverVisualRuntime80018FB0(
            badPhase, badPhaseBefore);

        CHECK(BeginLoadReplayState16Completion80019D7C(&visual));
        CHECK(visual.state == 23);
        CHECK(visual.phase ==
              CardDriverVisualPhase80018FB0::TerminalFrame80018FB0);
        CHECK(visual.promptFlashFramesRemaining80017E6C == 0u);
        CHECK(visual.exitBlinkStateArg4 == expectedArg4);
        CHECK(visual.cardIoFlagArg8 == 1);
        CHECK(visual.gp720Known);
        CHECK(visual.gp720 == 1);
        CHECK(!IsLoadReplayState16AwaitingPayload80019D7C(visual));

        const CardDriverVisualRuntime80018FB0 terminalBeforeDoubleBegin =
            visual;
        CHECK(!BeginLoadReplayState16Completion80019D7C(&visual));
        CheckSameCardDriverVisualRuntime80018FB0(
            visual, terminalBeforeDoubleBegin);

        CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
              CardDriverExitTickResult80017E6C::RenderFinalFlash);
        CHECK(visual.phase ==
              CardDriverVisualPhase80018FB0::FinalFlash80017E6C);
        CHECK(visual.promptFlashFramesRemaining80017E6C == 20u);
        CHECK(visual.exitBlinkStateArg4 == expectedArg4);
        CHECK(visual.cardIoFlagArg8 == 0);
        CHECK(visual.gp720Known);
        CHECK(visual.gp720 == 1);

        CardDriverVisualRuntime80018FB0 replacedByNegativeSentinel = visual;
        replacedByNegativeSentinel.exitBlinkStateArg4 = -1;
        const auto replacedByNegativeSentinelBefore =
            replacedByNegativeSentinel;
        CHECK(TickLoadReplayExitPromptFlash80017E6C(
                  &replacedByNegativeSentinel) ==
              CardDriverExitTickResult80017E6C::Rejected);
        CheckSameCardDriverVisualRuntime80018FB0(
            replacedByNegativeSentinel,
            replacedByNegativeSentinelBefore);
        for (uint32_t frame = 0; frame < 19u; ++frame) {
            CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
                  CardDriverExitTickResult80017E6C::RenderFinalFlash);
        }
        CHECK(visual.promptFlashFramesRemaining80017E6C == 1u);
        CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
              CardDriverExitTickResult80017E6C::ReturnScene0);
        CHECK(visual.phase == CardDriverVisualPhase80018FB0::Complete);
        CHECK(visual.promptFlashFramesRemaining80017E6C == 0u);
        CHECK(visual.gp720Known);
        CHECK(visual.gp720 == 1);
        CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
              CardDriverExitTickResult80017E6C::Rejected);
    }

    CHECK(!BeginLoadReplayState16Completion80019D7C(nullptr));
}

void TestLoadReplayState16IoErrorPrompt80017594() {
    const CardMode800191E4 modes[] = {
        CardMode800191E4::Load,
        CardMode800191E4::Replay,
    };

    for (const CardMode800191E4 mode : modes) {
        const int32_t selectedBlock =
            mode == CardMode800191E4::Load ? 4 : 7;
        LoadReplayListInputRuntime800181D0 list{};
        CardDriverVisualRuntime80018FB0 visual{};
        CHECK(InitLoadReplayListInputRuntime800181D0(mode, 1, &list));
        const auto selection = TickLoadReplayListInput800181D0(
            &list, kInputCross800185D0);
        CHECK(InitLoadReplayCardDriverVisualRuntime80018FB0(mode, &visual));
        CHECK(BeginListEntrySelectionFlash80017E6C(&visual, selection));
        for (uint32_t frame = 0; frame < 19u; ++frame) {
            CHECK(TickListEntrySelectionFlash80017E6C(&visual) ==
                  CardDriverEntryTickResult80017E6C::RenderSelectionFlash);
        }
        CHECK(TickListEntrySelectionFlash80017E6C(&visual) ==
              CardDriverEntryTickResult80017E6C::EnterState16);
        CHECK(IsLoadReplayState16AwaitingPayload80019D7C(visual));

        auto carrier = MakeState16CardIoResult80017594(
            mode,
            selectedBlock,
            static_cast<int32_t>(CardIoCode80017594::Timeout));
        CHECK(IsRequestBoundLoadReplayState16CardIoResult80017594(
            carrier, mode, selectedBlock));

        auto invalidSource = carrier;
        invalidSource.source =
            LoadReplayState16CardIoResultSource80017594::Unknown;
        const auto beforeInvalidSource = visual;
        CHECK(!ApplyLoadReplayState16CardIoResult80019D7C(
            &visual, invalidSource, mode, selectedBlock));
        CheckSameCardDriverVisualRuntime80018FB0(
            visual, beforeInvalidSource);

        auto nonTimeout = carrier;
        nonTimeout.ioResult80017594 =
            static_cast<int32_t>(CardIoCode80017594::IoSuccess);
        CHECK(!IsRequestBoundLoadReplayState16CardIoResult80017594(
            nonTimeout, mode, selectedBlock));
        const auto beforeNonTimeout = visual;
        CHECK(!ApplyLoadReplayState16CardIoResult80019D7C(
            &visual, nonTimeout, mode, selectedBlock));
        CheckSameCardDriverVisualRuntime80018FB0(
            visual, beforeNonTimeout);

        CHECK(!ApplyLoadReplayState16CardIoResult80019D7C(
            &visual, carrier, mode, selectedBlock + 1));
        CHECK(IsLoadReplayState16AwaitingPayload80019D7C(visual));
        CHECK(ApplyLoadReplayState16CardIoResult80019D7C(
            &visual, carrier, mode, selectedBlock));
        CHECK(!IsLoadReplayState16AwaitingPayload80019D7C(visual));
        CHECK(IsLoadReplayState5PromptIdle800180D8(visual));
        CHECK(visual.state == 5);
        CHECK(visual.eventId == CardEventFrameId::InsertCardPrompt);
        CHECK(visual.phase ==
              CardDriverVisualPhase80018FB0::State5PromptIdle800180D8);
        CHECK(visual.exitFrameStateArg0 == 1);
        CHECK(visual.exitBlinkStateArg4 == 0);
        CHECK(visual.cardIoFlagArg8 == 0);
        CHECK(!visual.gp720Known);
        CHECK(visual.gp720 == 0);
        CHECK(TickCardDriverVisualRuntime80018FB0(&visual));

        const auto beforeIgnoredInput = visual;
        CHECK(!BeginLoadReplayState5PromptFlash80017E6C(
            &visual, kInputCircle800185D0));
        CheckSameCardDriverVisualRuntime80018FB0(
            visual, beforeIgnoredInput);
        CHECK(BeginLoadReplayState5PromptFlash80017E6C(
            &visual, kInputCross800185D0));
        CHECK(visual.phase ==
              CardDriverVisualPhase80018FB0::SelectionFlash80017E6C);
        CHECK(visual.promptFlashFramesRemaining80017E6C == 20u);
        CHECK(visual.exitBlinkStateArg4 == 1);
        CHECK(visual.cardIoFlagArg8 == 0);
        CHECK(!visual.gp720Known);

        for (uint32_t frame = 0; frame < 19u; ++frame) {
            CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
                  CardDriverExitTickResult80017E6C::RenderSelectionFlash);
        }
        CHECK(visual.promptFlashFramesRemaining80017E6C == 1u);
        CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
              CardDriverExitTickResult80017E6C::RenderTerminalFrame);
        CHECK(visual.state == 23);
        CHECK(visual.eventId == CardEventFrameId::InsertCardPrompt);
        CHECK(!visual.gp720Known);
        CHECK(visual.gp720 == 0);
        CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
              CardDriverExitTickResult80017E6C::RenderFinalFlash);
        CHECK(visual.promptFlashFramesRemaining80017E6C == 20u);
        CHECK(visual.exitBlinkStateArg4 == 1);
        for (uint32_t frame = 0; frame < 19u; ++frame) {
            CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
                  CardDriverExitTickResult80017E6C::RenderFinalFlash);
        }
        CHECK(TickLoadReplayExitPromptFlash80017E6C(&visual) ==
              CardDriverExitTickResult80017E6C::ReturnScene0);
        CHECK(visual.phase == CardDriverVisualPhase80018FB0::Complete);
        CHECK(!visual.gp720Known);
        CHECK(visual.gp720 == 0);
    }

    const auto carrier = MakeState16CardIoResult80017594(
        CardMode800191E4::Load,
        0,
        static_cast<int32_t>(CardIoCode80017594::Timeout));
    CHECK(!ApplyLoadReplayState16CardIoResult80019D7C(
        nullptr, carrier, CardMode800191E4::Load, 0));
    CHECK(!BeginLoadReplayState5PromptFlash80017E6C(
        nullptr, kInputCross800185D0));
}

void TestLoadReplayExitPromptFlash80017E6C() {
    CardDriverVisualRuntime80018FB0 runtime{};
    CHECK(InitLoadReplayCardDriverVisualRuntime80018FB0(
        CardMode800191E4::Load, &runtime));
    CHECK(BeginLoadReplayExitPromptFlash80017E6C(&runtime));
    CHECK(runtime.phase ==
          CardDriverVisualPhase80018FB0::SelectionFlash80017E6C);
    CHECK(runtime.promptFlashFramesRemaining80017E6C == 20u);
    CHECK(runtime.exitFrameStateArg0 == 1);
    CHECK(runtime.exitBlinkStateArg4 == 2);
    CHECK(runtime.cardIoFlagArg8 == 0);
    CHECK(!runtime.gp720Known);
    CHECK(!TickCardDriverVisualRuntime80018FB0(&runtime));

    uint32_t renderedFrames = 1u;
    for (uint32_t i = 0; i < 19u; ++i) {
        CHECK(TickLoadReplayExitPromptFlash80017E6C(&runtime) ==
              CardDriverExitTickResult80017E6C::RenderSelectionFlash);
        ++renderedFrames;
    }
    CHECK(runtime.promptFlashFramesRemaining80017E6C == 1u);

    CHECK(TickLoadReplayExitPromptFlash80017E6C(&runtime) ==
          CardDriverExitTickResult80017E6C::RenderTerminalFrame);
    ++renderedFrames;
    CHECK(runtime.state == 23);
    CHECK(runtime.phase ==
          CardDriverVisualPhase80018FB0::TerminalFrame80018FB0);
    CHECK(runtime.promptFlashFramesRemaining80017E6C == 0u);
    CHECK(runtime.exitBlinkStateArg4 == 2);
    CHECK(runtime.cardIoFlagArg8 == 0);
    CHECK(!runtime.gp720Known);

    CHECK(TickLoadReplayExitPromptFlash80017E6C(&runtime) ==
          CardDriverExitTickResult80017E6C::RenderFinalFlash);
    ++renderedFrames;
    CHECK(runtime.phase ==
          CardDriverVisualPhase80018FB0::FinalFlash80017E6C);
    CHECK(runtime.promptFlashFramesRemaining80017E6C == 20u);
    CHECK(runtime.exitFrameStateArg0 == 1);
    CHECK(runtime.exitBlinkStateArg4 == 2);
    CHECK(runtime.cardIoFlagArg8 == 0);
    CHECK(!runtime.gp720Known);
    for (uint32_t i = 0; i < 19u; ++i) {
        CHECK(TickLoadReplayExitPromptFlash80017E6C(&runtime) ==
              CardDriverExitTickResult80017E6C::RenderFinalFlash);
        ++renderedFrames;
    }
    CHECK(renderedFrames == 41u);
    CHECK(runtime.promptFlashFramesRemaining80017E6C == 1u);
    CHECK(TickLoadReplayExitPromptFlash80017E6C(&runtime) ==
          CardDriverExitTickResult80017E6C::ReturnScene0);
    CHECK(runtime.phase == CardDriverVisualPhase80018FB0::Complete);
    CHECK(runtime.promptFlashFramesRemaining80017E6C == 0u);
    CHECK(!runtime.gp720Known);
    CHECK(TickLoadReplayExitPromptFlash80017E6C(&runtime) ==
          CardDriverExitTickResult80017E6C::Rejected);
    CHECK(!BeginLoadReplayExitPromptFlash80017E6C(&runtime));

    runtime = {};
    runtime.known = true;
    runtime.state = 23;
    runtime.eventId = CardEventFrameId::Load;
    runtime.phase = CardDriverVisualPhase80018FB0::ListIdle;
    CHECK(!TickCardDriverVisualRuntime80018FB0(&runtime));
    CHECK(!BeginLoadReplayExitPromptFlash80017E6C(&runtime));
}

bool HasCardDriverAction(const CardHandoffPlan& plan,
                         CardHandoffActionKind kind,
                         uint32_t psxFunction) {
    for (uint32_t i = 0; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind &&
            plan.actions[i].psxFunction == psxFunction) {
            return true;
        }
    }
    return false;
}

void TestCardDriverCallbackBindings80018FB0() {
    const CardHandoffPlan save = BuildCardDriverLoop80018FB0Plan(
        21, CardEventFrameId::SaveUi, CardMode800191E4::Save);
    CHECK(HasCardDriverAction(
        save, CardHandoffActionKind::Call800185D0, kFn800185D0));
    CHECK(HasCardDriverAction(
        save, CardHandoffActionKind::Call80019458, kFn80019458));

    constexpr CardMode800191E4 kLoadCallbacks[] = {
        CardMode800191E4::Load,
        CardMode800191E4::Replay,
        CardMode800191E4::HiScore,
    };
    for (CardMode800191E4 mode : kLoadCallbacks) {
        const CardHandoffPlan plan = BuildCardDriverLoop80018FB0Plan(
            20, CardEventFrameId::Load, mode);
        CHECK(HasCardDriverAction(
            plan, CardHandoffActionKind::Call80018E10, kFn80018E10));
        CHECK(HasCardDriverAction(
            plan, CardHandoffActionKind::Call80019D7C, kFn80019D7C));
    }

    const CardHandoffPlan unknown = BuildCardDriverLoop80018FB0Plan(
        20, CardEventFrameId::Unknown, CardMode800191E4::Unknown);
    CHECK(unknown.count != 0u);
    if (unknown.count != 0u) {
        CHECK(unknown.actions[unknown.count - 1u].kind ==
              CardHandoffActionKind::Gap);
    }
    CHECK(!HasCardDriverAction(
        unknown, CardHandoffActionKind::Call80018E10, kFn80018E10));
    CHECK(!HasCardDriverAction(
        unknown, CardHandoffActionKind::Call80019D7C, kFn80019D7C));
}

} // namespace

int main() {
    TestGenericPublishCannotPromoteRuntimeDirectoryProducer();
    TestRuntimePublishAcceptsMatchingModeAndRows();
    TestGetDoesNotConsumeAndClearIsExplicit();
    TestInvalidRuntimePublishClearsPreviousCarrier();
    TestRejectsBadModeAndEntryCount();
    TestTitleIsNormalizedToDirectoryNameLimit();
    TestScanFactsBuildRuntimeDirectoryCarrier();
    TestScanFactsRejectMissingDirectorySource();
    TestScanFactsRejectUnknownRowsAndBadMode();
    TestLoadReplayCardDriverVisualRuntime80018FB0();
    TestLoadReplayListInputRuntime800181D0();
    TestSelectedRowIdentity800181D0();
    TestLoadReplayState16Completion80019D7C();
    TestLoadReplayState16IoErrorPrompt80017594();
    TestLoadReplayExitPromptFlash80017E6C();
    TestCardDriverCallbackBindings80018FB0();

    if (g_failed != 0) {
        std::printf(
            "test_ss0_load_replay_directory_authority: failed checks=%d\n",
            g_failed);
        return 1;
    }

    std::printf("test_ss0_load_replay_directory_authority: ok\n");
    return 0;
}
