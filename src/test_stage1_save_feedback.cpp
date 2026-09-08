// White-box tests of the actual native blink/input producer. No game/card
// device is opened; the separate normal-clear probe covers the host consumer.
#define NOMINMAX
#include "pr/pr_stage1_save_ui_direct.cpp"
#include "pr/pr_stage1_save_presentation_direct.h"
#include <cassert>
#include <vector>
#include <utility>
#include <fstream>
#include <iterator>

PrPadState PrPad::GetState(int) { return {}; }

using Kind = PrStage1SaveUi19148ActionKind;
namespace Presentation = PrStage1SavePresentationDirect;

static PrStage1SaveUi19148Action Action(Kind kind, uint32_t fn = 0) {
    PrStage1SaveUi19148Action action{};
    action.kind = kind;
    action.psxFunction = fn;
    return action;
}

static void TestNativeBlink() {
    for (int state : {15, 16}) {
        for (int phase = 0; phase <= 19; ++phase) {
            s_saveUi19148 = {};
            s_saveUi19148.state = state;
            s_saveUi19148.eventId = 11;
            s_saveUi19148.gp728_blinkCounter = phase;
            PrStage1SaveUi19148TickResult tick{};
            StepBlinkAndDraw80018FB0(tick);
            assert(s_saveUi19148.gp728_blinkCounter == phase);
            assert(tick.actions.count == 3);
            assert(tick.actions.actions[0].arg1 == static_cast<int32_t>(
                PrStage1SaveUiEventArgUpdate80018FB0::ForceWord0AndWord2One));
            s_saveUi19148.state = 2;
            tick = {};
            StepBlinkAndDraw80018FB0(tick);
            assert(s_saveUi19148.gp728_blinkCounter == (phase == 19 ? 0 : phase + 1));
            assert(tick.actions.actions[0].arg1 == static_cast<int32_t>(phase == 0
                ? PrStage1SaveUiEventArgUpdate80018FB0::ToggleWord0
                : PrStage1SaveUiEventArgUpdate80018FB0::None));
        }
    }
}

static PrStage1SaveUi19148ActionList NativeCancel() {
    PrStage1SaveUiDirect::Reset19148();
    PrGameContext ctx{};
    assert(PrStage1SaveUiDirect::Start19148(ctx));
    (void)PrStage1SaveUiDirect::Tick19148(ctx);
    ctx.debugPadInput = kInputCircle800185D0; // this field carries the native, not Win-local mask
    // Explicit unit input for the card-info call; no fabricated runtime card.
    PrStage1SaveUi19148LowerFeedback lower{};
    lower.cardIoFeedbackKnown80017594 = true;
    lower.cardIoFeedback80017594.stateBeforeKnown = true;
    lower.cardIoFeedback80017594.stateBefore = s_saveUi19148.cardIoState80017594;
    lower.cardIoFeedback80017594.cardInfoKnown = true;
    lower.cardIoFeedback80017594.cardInfoArgKnown = true;
    lower.cardIoFeedback80017594.cardInfoArg = 0;
    auto tick = PrStage1SaveUiDirect::Tick19148(ctx, &lower);
    if (!tick.done) {
        std::fprintf(stderr, "cancel state=%d active=%d gap=%d input=%d actions=%u\n",
            tick.psxState, tick.active, tick.helperGap, tick.inputMaskAfterDedup80018FB0, tick.actions.count);
        for (uint32_t i = 0; i < tick.actions.count; ++i) {
            const auto& a = tick.actions.actions[i];
            std::fprintf(stderr, "%s %08X args %d %d %d %d\n",
                PrStage1SaveUiDirect::ActionKindName19148(a.kind), a.psxFunction, a.arg0, a.arg1, a.arg2, a.arg3);
        }
    }
    assert(tick.done && !tick.active && tick.saveResult == 0);
    uint32_t firstFlash = 96, lastFlash = 0, normalDraw = 96, normalEnd = 96, sound = 96;
    for (uint32_t i = 0; i < tick.actions.count; ++i) {
        const auto& action = tick.actions.actions[i];
        if (Presentation::IsFlash(action)) {
            if (firstFlash == 96) firstFlash = i;
            lastFlash = i;
        }
        if (action.kind == Kind::Call8001E750DrawEvent) normalDraw = i;
        if (action.kind == Kind::Call8001EA00EndFrame) normalEnd = i;
        if (action.kind == Kind::Call80025C8CPlayInputSfx) sound = i;
    }
    assert(sound < firstFlash && firstFlash < normalDraw && normalDraw < normalEnd && normalEnd < lastFlash);
    assert(tick.actions.actions[firstFlash].arg1 == 2);
    assert(tick.actions.actions[lastFlash].arg1 == -1);
    return tick.actions;
}

static void TestNativeCancelPresentation(int transportStep) {
    const auto actions = NativeCancel();
    Presentation::State state{};
    Presentation::Begin(state, 100);
    std::vector<std::pair<Kind, uint64_t>> trace;
    int word0 = 0, word1 = 0, word2 = 0, flashCount = 0, soundCount = 0;
    auto consume = [&](const PrStage1SaveUi19148Action& action, uint64_t time) {
        trace.emplace_back(action.kind, time);
        if (Presentation::IsFlash(action)) {
            word0 = 1;
            if (action.arg1 >= 0) word1 = action.arg1;
            word2 = action.arg2;
            assert(word0 == 1 && word1 == 2 && word2 == 0);
            assert(time == static_cast<uint64_t>(100 + flashCount + (flashCount >= 20 ? 1 : 0)));
            ++flashCount;
        }
        if (action.kind == Kind::Call80025C8CPlayInputSfx) {
            assert(time == 100 && flashCount == 0);
            ++soundCount;
        }
        if (action.kind == Kind::Call8001E750DrawEvent) assert(time == 120 && flashCount == 20);
        return true;
    };
    for (uint64_t time = 100; time < 141; time += transportStep) {
        assert(!Presentation::Advance(state, actions, time, consume));
        assert(state.pending);
        const auto count = trace.size();
        assert(!Presentation::Advance(state, actions, time, consume));
        assert(trace.size() == count); // duplicate redraw cannot advance native time
    }
    assert(flashCount == 40 && soundCount == 1 && state.presentations == 41);
    assert(Presentation::Advance(state, actions, transportStep == 1 ? 141 : 142, consume));
    assert(state.nextVblank == 141 && !state.pending);
    const auto count = trace.size();
    assert(Presentation::Advance(state, actions, 200, consume) && trace.size() == count);
}

static void TestActionBarrierAndRejection() {
    PrStage1SaveUi19148ActionList actions{};
    actions.count = 3;
    actions.actions[0] = Action(Kind::Call80025C8CPlayInputSfx);
    actions.actions[1] = Action(Kind::Call80017E6CSetEventResult, 0x80017E6Cu);
    actions.actions[1].arg3 = 20;
    actions.actions[2] = Action(Kind::Call80017A10WriteSaveBlock);
    Presentation::State state{};
    Presentation::Begin(state, 0);
    int writes = 0, consumed = 0;
    auto accept = [&](const auto& action, uint64_t time) {
        ++consumed;
        if (action.kind == Kind::Call80017A10WriteSaveBlock) { assert(time == 20); ++writes; }
        return true;
    };
    for (uint64_t time = 0; time < 20; ++time) {
        assert(!Presentation::Advance(state, actions, time, accept));
        assert(writes == 0);
    }
    assert(Presentation::Advance(state, actions, 20, accept));
    assert(writes == 1 && consumed == 22);
    Presentation::Begin(state, 0);
    consumed = 0;
    assert(!Presentation::Advance(state, actions, 100, [&](const auto& action, uint64_t) {
        ++consumed;
        return action.kind != Kind::Call80017E6CSetEventResult;
    }));
    assert(state.blocked && state.pending && consumed == 2);
    assert(!Presentation::Advance(state, actions, 200, accept) && consumed == 2);
    Presentation::Begin(state, 0);
    actions.actions[1].arg3 = 19;
    assert(!Presentation::Advance(state, actions, 100, accept) && state.blocked);
    Presentation::Begin(state, 0);
    actions.truncated = true;
    consumed = 0;
    assert(!Presentation::Advance(state, actions, 100, accept) && state.blocked && consumed == 0);
}

// Typed device responses are unit-test inputs only. Exercise the actual
// 80019458 producer and 80017A10 retry reducer, never a substitute state machine.
static PrStage1SaveUi19148LowerFeedback WriteFeedback(int successAttempt = -1) {
    PrStage1SaveUi19148LowerFeedback lower{};
    lower.writeFeedbackKnown80017A10 = true;
    for (int i = 0; i < 4; ++i) {
        auto& a = lower.writeFeedback80017A10.attempts[i];
        a.scanResultKnown80017900 = true;
        a.scanResult80017900 = 1;
        a.openWriteKnown80017454 = true;
        a.openWriteFdKnown80017454 = true;
        a.openWriteReturnKnown80017454 = true;
        a.openWriteFd80017454 = a.openWriteReturn80017454 = 7;
        a.gp696FdWriteKnown80017454 = true;
        a.gp696Fd80017454 = 7;
        a.clearSwEventsKnown80016FC0 = true;
        a.writeKnown80017454 = a.writeByteCountKnown80017454 = true;
        a.writeByteCount80017454 = 8192;
        a.writeReturnKnown80017454 = true;
        a.writeReturn80017454 = 8192;
        a.submitReturnKnown80017454 = true;
        a.submitReturn80017454 = 0;
        a.waitCallKnown80035560 = true;
        a.waitArg80035560 = 4;
        a.pollResultKnown80016EB8 = true;
        a.pollResult80016EB8 = i == successAttempt ? 1 : 2;
        a.closeResultKnown = a.closeFdKnown = true;
        a.gp696FdCloseKnown80017A10 = true;
        a.closeFd = a.gp696FdClose80017A10 = 7;
    }
    return lower;
}

static void PrepareWriteState(bool payloadKnown = true) {
    PrStage1SaveUiDirect::Reset19148();
    PrGameContext ctx{};
    assert(PrStage1SaveUiDirect::Start19148(ctx));
    s_saveUi19148.state = 15;
    // No card/device or game is opened by these branch tests.
    s_saveUiMemory.payloadBank.savePayloadBankKnown = payloadKnown;
    s_saveUiMemory.payloadBank.savePayloadBank.fill(0x5A);
}

static void TestNativeWriteResultBranch() {
    auto lower = WriteFeedback();
    PrepareWriteState();
    PrStage1SaveUi19148TickResult failed{};
    const auto failedState = TickState80019458(15, 0, failed, &lower);
    assert(failed.saveWriteResultKnown80017A10 && failed.saveWriteResult80017A10 == -1);
    // Current SCUS 80019D2C BLTZ -> 800194C0: return 2, no result stores.
    assert(failedState == 2);
    assert(!failed.helperGap && !failed.actions.truncated);
    assert(failed.state15CopyTo8007ADE8 && !failed.saveWriteSucceeded80019458);
    assert(!failed.gp716After80019458Known && !failed.gp720After80019458Known);
    assert(s_saveUi19148.gp716_saveOk == 0 && s_saveUi19148.gp720_result == 0);
    int attempts = 0;
    for (uint32_t i = 0; i < failed.actions.count; ++i) {
        attempts += failed.actions.actions[i].kind == Kind::Call80017454SubmitWrite;
        assert(failed.actions.actions[i].kind != Kind::Call80019458CommitResultState15);
    }
    assert(attempts == 4);
    // The ordinary caller must be able to publish the initial question again.
    s_saveUi19148.state = failedState;
    Sub800180D8(failedState, failed);
    assert(s_saveUi19148.eventId == 11);

    for (const int successAttempt : {0, 3}) {
        lower = WriteFeedback(successAttempt);
        PrepareWriteState();
        PrStage1SaveUi19148TickResult success{};
        assert(TickState80019458(15, 0, success, &lower) == 23);
        assert(!success.helperGap && success.saveWriteSucceeded80019458);
        assert(success.saveWriteResultKnown80017A10 && success.saveWriteResult80017A10 == 0);
        assert(s_saveUi19148.gp716_saveOk == 1 && s_saveUi19148.gp720_result == 1);
    }
    // An absent/incomplete completion is NOT a native write failure.
    lower = WriteFeedback();
    lower.writeFeedback80017A10.attempts[3].closeFdKnown = false;
    PrepareWriteState();
    PrStage1SaveUi19148TickResult unknown{};
    assert(TickState80019458(15, 0, unknown, &lower) == 15);
    assert(unknown.helperGap && !unknown.saveWriteResultKnown80017A10);
    lower = WriteFeedback();
    PrepareWriteState(false);
    PrStage1SaveUi19148TickResult noPayload{};
    assert(TickState80019458(15, 0, noPayload, &lower) == 15);
    assert(noPayload.helperGap && !noPayload.consumedBy80019458State15);
    PrepareWriteState();
    PrStage1SaveUi19148TickResult removed{};
    assert(TickState80019458(15, 3, removed, nullptr) == 2);
    assert(removed.actions.count == 0); // native a2 == 3 returns before writing
}

static void TestNativeNameDirections() {
    struct Move { PrPadButton local; int mask; int from; int to; };
    for (const auto& move : {Move{PrPadButton::Up, 0x1000, 0, 56},
                            Move{PrPadButton::Right, 0x2000, 56, 0},
                            Move{PrPadButton::Down, 0x4000, 0, 14},
                            Move{PrPadButton::Left, 0x8000, 0, 56}}) {
        assert(BuildLocalSaveUiInputMask80035510(static_cast<uint16_t>(move.local)) == move.mask);
        PrepareWriteState();
        PrGameContext ctx{};
        PrStage1SaveUi19148TickResult tick{};
        Sub80018060(tick, 11, 10);
        s_saveUi19148.word8004925A = move.from;
        assert(HandleInput800185D0(move.mask, 10, ctx, tick, nullptr) == 10);
        assert(s_saveUi19148.word8004925A == move.to);
    }
}

static void TestNativeDirectoryComparison() {
    // 80019458 case9 compares the current 600-byte directory returned by
    // 80017B08 with 8007CC74, not the filename/title work buffer at 8007CBE8.
    PrStage1SaveUiDirect::Reset19148();
    PrGameContext ctx{};
    assert(PrStage1SaveUiDirect::Start19148(ctx));
    auto* previous = DirectMemoryPtr(kAddrPreviousSnapshot8007CC74, 600);
    assert(previous);
    for (std::size_t i = 0; i < 600; ++i)
        previous[i] = s_saveUiMemory.dirBank[i] = static_cast<uint8_t>(i * 13 + 7);
    s_saveUiMemory.gp712_overwriteScanFlag = 1;
    auto prefersFree = [&]() {
        PrStage1SaveUi19148TickResult tick{};
        bool result = true;
        // Deliberately absent row feedback stops the later enumeration. The
        // native comparison precedes it; no filesystem/device input is used.
        assert(!EnumerateDirectoryListGap80019458(tick, 9, 11, true, nullptr, &result));
        return result;
    };
    assert(!prefersFree());
    for (std::size_t changed : {std::size_t(0), std::size_t(299), std::size_t(599)}) {
        s_saveUiMemory.dirBank[changed] ^= 0x40;
        assert(prefersFree());
        s_saveUiMemory.dirBank[changed] ^= 0x40;
        assert(!prefersFree());
    }
    s_saveUiMemory.gp712_overwriteScanFlag = 0;
    assert(prefersFree());
}

static void TestNativeDirectorySnapshotLifetime() {
    namespace Save = PrStage1SaveUiDirect;
    ResetDirectMemory();
    s_saveUiMemory.dirBank.fill(0xAA);
    Save::SnapshotDirectory80018F70(); // no medium: cleared directory, not stale rows
    auto* snapshot = DirectMemoryPtr(kAddrPreviousSnapshot8007CC74, 600);
    assert(snapshot);
    for (std::size_t i = 0; i < 600; ++i) assert(snapshot[i] == 0);
    // Explicit test medium only, never a user's file or runtime override.
    std::array<uint8_t, 128 * 1024> card{};
    Save::InitializeFormattedCardImage8007A318(card);
    auto* row = card.data() + 128;
    row[0] = 0x51;
    Save::WriteU32LE8007A318(row + 4, 8192);
    std::memcpy(row + 10, "BASCUS-94183PARAPP", 17);
    row[127] = Save::ComputeFrameChecksum8007A318(row);
    assert(Save::ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
        card.data(), card.size(), 0).sinkCommitted);
    Save::SnapshotDirectory80018F70();
    assert(std::memcmp(snapshot, "BASCUS-94183PARAPP", 17) == 0);
    assert(std::memcmp(snapshot, s_saveUiMemory.dirBank.data(), 600) == 0);
    assert(std::memcmp(card.data(), s_saveUiMemory.pendingCardImagePersistence.data(), card.size()) == 0);
    std::array<uint8_t, 600> saved{};
    std::memcpy(saved.data(), snapshot, saved.size());
    s_saveUiMemory.gp712_overwriteScanFlag = 1;
    Save::Reset19148();
    PrGameContext ctx{};
    assert(Save::Start19148(ctx));
    assert(s_saveUiMemory.gp712_overwriteScanFlag == 1);
    assert(std::memcmp(snapshot, saved.data(), saved.size()) == 0);
    ResetDirectMemory(); // startup BSS reset, unlike re-entry, clears both
    assert(s_saveUiMemory.gp712_overwriteScanFlag == 0);
    for (std::size_t i = 0; i < 600; ++i) assert(snapshot[i] == 0);
}

static PrStage1SaveUi19148LowerFeedback NameDirectoryFeedback(
    const char* existingSuffix, int freeSlots) {
    // Typed unit-test device input only; never injected into a running game.
    PrStage1SaveUi19148LowerFeedback lower{};
    lower.directoryRowsFeedbackKnown80019458 = true;
    auto& rows = lower.directoryRowsFeedback80019458;
    rows.translated = rows.sourceKnown = rows.entryCountKnown = true;
    rows.freeSlotsKnown = rows.rowsKnown = true;
    rows.source = PrStage1SaveUiDirectoryRowsSource80019458::RuntimeCardDirectoryProducer;
    rows.entryCount = 1 + freeSlots;
    rows.freeSlots = freeSlots;
    for (int i = 0; i < rows.entryCount; ++i) {
        auto& row = rows.rows[i];
        row.active = row.blockIndexKnown = row.suffixKnown = true;
        row.blockIndex = i;
        row.freeSlot = i != 0;
        if (i == 0) std::strcpy(row.suffix, existingSuffix);
    }
    return lower;
}

static void TestNativeNewNameCollision() {
    PrGameContext ctx{};
    for (const char* existing : {"PARAPP", "PAR", "PARAPPX", "OTHER"}) {
        for (int freeSlots : {0, 1, 14}) {
            for (bool delayedDirectory : {false, true}) {
                PrepareWriteState();
                s_saveUiMemory.gp712_overwriteScanFlag = 0;
                PrStage1SaveUi19148TickResult init{};
                Sub80018060(init, 11, 10);
                s_saveUi19148.state = 10;
                s_saveUi19148.word8004925A = 56; // SCUS END, original default PARAPP
                const auto lower = NameDirectoryFeedback(existing, freeSlots);
                if (delayedDirectory) {
                    PrStage1SaveUi19148TickResult waiting{};
                    assert(HandleInput800185D0(0x40, 10, ctx, waiting, nullptr) == 10);
                    assert(s_saveUi19148.state10ConfirmDirectoryPending);
                    assert(s_saveUiMemory.gp712_overwriteScanFlag == 0);
                }
                PrStage1SaveUi19148TickResult tick{};
                const int next = delayedDirectory
                    ? (tick = PrStage1SaveUiDirect::Tick19148(ctx, &lower), tick.psxState)
                    : HandleInput800185D0(0x40, 10, ctx, tick, &lower);
                const bool duplicate = std::strcmp(existing, "PARAPP") == 0;
                const int expected = duplicate ? 18 : (freeSlots == 0 ? 7 : 15);
                if (next != expected) std::fprintf(stderr,
                    "name collision: existing=%s free=%d delayed=%d expected=%d actual=%d\n",
                    existing, freeSlots, delayedDirectory, expected, next);
                assert(next == expected);
                assert(!s_saveUi19148.state10ConfirmDirectoryPending);
                assert(s_saveUiMemory.gp712_overwriteScanFlag == (expected == 15 ? 1 : 0));
                int flashes = 0;
                for (uint32_t i = 0; i < tick.actions.count; ++i) {
                    const auto& a = tick.actions.actions[i];
                    if (Presentation::IsFlash(a) && a.arg0 == 5) ++flashes;
                    if (expected != 15) assert(a.kind != Kind::Call80017A10WriteSaveBlock);
                }
                assert(flashes == (expected == 15 ? 2 : 1));
                if (duplicate) {
                    PrStage1SaveUi19148TickResult retry{};
                    assert(HandleInput800185D0(0x20, 18, ctx, retry, nullptr) == 18);
                    assert(HandleInput800185D0(0x40, 18, ctx, retry, nullptr) == 10);
                    assert(s_saveUiMemory.gp712_overwriteScanFlag == 0);
                }
            }
        }
    }
}

static void TestNativeSaveTables(const char* filename) {
    std::ifstream input(filename, std::ios::binary);
    assert(input);
    const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
    assert(bytes.size() > 0x800 && std::memcmp(bytes.data(), "PS-X EXE", 8) == 0);
    uint32_t loadAddress = 0;
    std::memcpy(&loadAddress, bytes.data() + 0x18, sizeof(loadAddress));
    auto check = [&](uint32_t address, const void* translated, std::size_t count) {
        const std::size_t offset = 0x800u + address - loadAddress;
        assert(offset + count <= bytes.size());
        assert(std::memcmp(translated, bytes.data() + offset, count) == 0);
    };
    check(0x80010004u, kHeaderIconSource80010004, sizeof(kHeaderIconSource80010004));
    check(0x8006E999u, kTitleCharClass8006E999, sizeof(kTitleCharClass8006E999));
    check(0x8006EAC8u, kSaveUiGpSource8006EAC8, sizeof(kSaveUiGpSource8006EAC8));
    check(0x800101E0u, kSaveFilenamePrefix800101E0, sizeof(kSaveFilenamePrefix800101E0));
    check(0x800490E8u, kNameGlyphState800490E8, sizeof(kNameGlyphState800490E8));
    check(0x8006EAF0u, kDefaultNamePreview8006EAF0, sizeof(kDefaultNamePreview8006EAF0));
    check(0x800490E8u, kNameInputCharTable800490E8, sizeof(kNameInputCharTable800490E8));
    const uint16_t count = kNameInputCharCount80049258;
    check(0x80049258u, &count, sizeof(count));
    assert(kNameInputCharTable800490E8[55] == '\b');
    assert(kNameInputCharTable800490E8[56] == '\n');
}

int main(int argc, char** argv) {
    assert(argc == 2);
    TestNativeSaveTables(argv[1]);
    TestNativeBlink();
    TestNativeCancelPresentation(1);
    TestNativeCancelPresentation(2);
    TestActionBarrierAndRejection();
    TestNativeWriteResultBranch();
    TestNativeNameDirections();
    TestNativeDirectoryComparison();
    TestNativeDirectorySnapshotLifetime();
    TestNativeNewNameCollision();
    std::puts("stage1 save feedback: native blink phases, cancel 20+1+20 intervals, 30/60Hz, order and rejection passed");
    std::puts("stage1 save write: four failures -> question, success -> exit, unknown -> wait, removed -> question passed");
    std::puts("stage1 save tables: six original data ranges, all name characters and native count match SCUS");
    std::puts("stage1 save directory: exact 600-byte comparison, post-loop snapshot and re-entry lifetime passed");
    std::puts("stage1 save name: exact duplicate -> state18 before full-card -> state7, unique -> state15, delayed scan and retry passed");
}
