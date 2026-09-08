#include "pr/pr_ss0_event_text_direct.h"

#include <cstdio>
#include <cstring>
#include <initializer_list>

using namespace PrSS0EventTextDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

struct SubmitCapture {
    int calls = 0;
    int32_t returnValue = 0;
    EventTextRuntimeState800436F0* stateToObserve = nullptr;
    uint32_t redirectTextPointer = 0;
    uint32_t observedHeadWord = 0;
    uint32_t observedAppendCursor = 0;
    uint8_t observedFirstTextByte = 0;
    uint32_t observedFirstPacketWord = 0;
    EventTextFlushSubmission800436F0 last{};
};

int32_t CaptureSubmit(const EventTextFlushSubmission800436F0& submission,
                      void* userData) {
    SubmitCapture& capture = *static_cast<SubmitCapture*>(userData);
    ++capture.calls;
    capture.last = submission;
    if (capture.stateToObserve != nullptr) {
        EventTextRecord800436F0& record =
            capture.stateToObserve->records[submission.selectedSlot];
        capture.observedHeadWord = record.listHeadWord10;
        capture.observedAppendCursor = record.appendCursor28;
        const uint32_t textOffset =
            record.textPointer24 - kTextRecordTextBufferBase8008A750;
        if (textOffset < capture.stateToObserve->textBytes.size()) {
            capture.observedFirstTextByte =
                capture.stateToObserve->textBytes[textOffset];
        }
        capture.observedFirstPacketWord =
            capture.stateToObserve->glyphPackets[0].word0;
        if (capture.redirectTextPointer != 0u) {
            record.textPointer24 = capture.redirectTextPointer;
        }
    }
    return capture.returnValue;
}

EventTextRuntimeState800436F0 MakeState() {
    EventTextRuntimeState800436F0 state{};
    state.recordBankKnown = true;
    state.globalsKnown = true;
    state.textBankKnown = true;
    state.glyphBankKnown = true;
    state.recordCount8005CCDC = 2;
    state.currentRecord8005CCE0 = 0;

    EventTextRecord800436F0& record = state.records[0];
    record.word0 = 0xBB001234u;
    record.byte04 = 0x41u;
    record.byte05 = 0x42u;
    record.byte06 = 0x43u;
    record.x = 10;
    record.y = 20;
    record.w = 300;
    record.h = 100;
    record.listHeadWord10 = 0xAA000321u;
    record.word14 = 0x14141414u;
    record.word18 = 0x18181818u;
    record.parseBudget1C = 16;
    record.glyphPacketCursor20 = kTextRecordGlyphBufferBase8008AB50;
    record.textPointer24 = kTextRecordTextBufferBase8008A750;
    record.appendCursor28 = 77u;

    for (uint32_t i = 0; i < 4u; ++i) {
        EventTextGlyphPacket800436F0& packet = state.glyphPackets[i];
        packet.word0 = (0xC0u + i) << 24;
        packet.code = static_cast<uint8_t>(0x70u + i);
        packet.clut = static_cast<uint16_t>(0x6000u + i);
    }
    return state;
}

void WriteText(EventTextRuntimeState800436F0& state,
               uint32_t offset,
               std::initializer_list<uint8_t> bytes) {
    uint32_t index = offset;
    for (uint8_t byte : bytes) {
        CHECK(index < state.textBytes.size());
        if (index < state.textBytes.size()) {
            state.textBytes[index++] = byte;
        }
    }
}

bool SameState(const EventTextRuntimeState800436F0& lhs,
               const EventTextRuntimeState800436F0& rhs) {
    return std::memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}

void TestSourceSlotAndFallbackNullGates() {
    EventTextRuntimeState800436F0 state = MakeState();
    WriteText(state, 0u, {'A', 0});
    SubmitCapture capture{};
    EventTextFlushExecutionResult800436F0 result{};

    state.recordBankKnown = false;
    const EventTextRuntimeState800436F0 unknownBefore = state;
    CHECK(!ExecuteTextFlush800436F0(
        state, -1, CaptureSubmit, &capture, result));
    CHECK(result.status == EventTextFlushStatus800436F0::SourceUnknown);
    CHECK(SameState(state, unknownBefore));
    CHECK(capture.calls == 0);

    state = MakeState();
    state.recordCount8005CCDC = 9;
    WriteText(state, 0u, {'A', 0});
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(!result.usedFallbackSlot);
    CHECK(result.selectedSlot == 0u);

    state = MakeState();
    state.recordCount8005CCDC = 9;
    CHECK(!ExecuteTextFlush800436F0(
        state, 8, CaptureSubmit, &capture, result));
    CHECK(result.status == EventTextFlushStatus800436F0::InvalidSelectedSlot);

    state = MakeState();
    state.currentRecord8005CCE0 = 8;
    CHECK(!ExecuteTextFlush800436F0(
        state, -1, CaptureSubmit, &capture, result));
    CHECK(result.status == EventTextFlushStatus800436F0::InvalidSelectedSlot);

    state = MakeState();
    state.recordCount8005CCDC = 1;
    state.records[0].textPointer24 = 0u;
    // The original dereferences this explicit-slot path; only the host binding fails.
    CHECK(!ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.status ==
          EventTextFlushStatus800436F0::HostTextPointerBindingLimit);

    state = MakeState();
    state.currentRecord8005CCE0 = 1;
    state.records[1].textPointer24 = 0u;
    state.records[1].appendCursor28 = 99u;
    const EventTextRuntimeState800436F0 nullBefore = state;
    CHECK(ExecuteTextFlush800436F0(
        state, -1, CaptureSubmit, &capture, result));
    CHECK(result.accepted);
    CHECK(result.usedFallbackSlot);
    CHECK(result.selectedSlot == 1u);
    CHECK(result.status ==
          EventTextFlushStatus800436F0::CurrentTextPointerNull);
    CHECK(result.returnedHeadAddress == 0u);
    CHECK(!result.submitInvoked);
    CHECK(SameState(state, nullBefore));

    state = MakeState();
    state.currentRecord8005CCE0 = 1;
    state.records[1] = state.records[0];
    WriteText(state, 0u, {'A', 0});
    CHECK(ExecuteTextFlush800436F0(
        state, 2, CaptureSubmit, &capture, result));
    CHECK(result.usedFallbackSlot);
    CHECK(result.selectedSlot == 1u);
}

void TestEmptyAndZeroBudgetStillSubmitAndClear() {
    EventTextRuntimeState800436F0 state = MakeState();
    state.glyphBankKnown = false;
    state.records[0].glyphPacketCursor20 = 1u;
    state.textBytes[0] = 0u;
    state.textBytes[1] = 0x5Au;
    SubmitCapture capture{};
    EventTextFlushExecutionResult800436F0 result{};
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.status == EventTextFlushStatus800436F0::Executed);
    CHECK(result.parserIterations == 0u);
    CHECK(result.submission.glyphCount == 0u);
    CHECK(result.submission.headWord == 0xAAFFFFFFu);
    CHECK(result.submission.recordBeforeCleanup.appendCursor28 == 77u);
    CHECK(result.returnedHeadAddress == kTextRecordBase8005CB5C + 0x10u);
    CHECK(capture.calls == 1);
    CHECK(state.records[0].appendCursor28 == 0u);
    CHECK(state.records[0].parseBudget1C == 16);
    CHECK(state.textBytes[0] == 0u);
    CHECK(state.textBytes[1] == 0x5Au);

    state = MakeState();
    WriteText(state, 0u, {'A', 'B', 0});
    state.records[0].parseBudget1C = 0;
    state.glyphBankKnown = false;
    state.records[0].glyphPacketCursor20 = 1u;
    capture = SubmitCapture{};
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.parserIterations == 0u);
    CHECK(result.localBudgetRemaining == 0);
    CHECK(result.submission.glyphCount == 0u);
    CHECK(capture.calls == 1);
    CHECK(state.textBytes[0] == 0u);
    CHECK(state.textBytes[1] == 'B');
}

void TestParserColorOtagCleanupAndFrozenReplay() {
    EventTextRuntimeState800436F0 state = MakeState();
    state.records[0].control07 = 1u;
    state.records[0].parseBudget1C = 10;
    WriteText(state,
              0u,
              {'~', 'c', '1', '2', '3', 'a', 'A', ' ', '~', 'x', 'B', 0});
    const EventTextRecord800436F0 recordBefore = state.records[0];
    const uint8_t byteAfterFirst = state.textBytes[1];
    SubmitCapture capture{};
    capture.returnValue = -7;
    capture.stateToObserve = &state;
    EventTextFlushExecutionResult800436F0 result{};

    CHECK(ExecuteTextFlush800436F0(
        state, -1, CaptureSubmit, &capture, result));
    CHECK(result.status == EventTextFlushStatus800436F0::Executed);
    CHECK(result.submitInvoked);
    CHECK(result.submitResultIgnored == -7);
    CHECK(result.returnedHeadAddress == kTextRecordBase8005CB5C + 0x10u);
    CHECK(result.parserIterations == 6u);
    CHECK(result.localBudgetRemaining == 4);
    CHECK(result.submission.glyphCount == 3u);
    CHECK(result.submission.recordLinked);
    CHECK(capture.calls == 1);

    const EventTextSubmittedGlyph800436F0& first = result.submission.glyphs[0];
    const EventTextSubmittedGlyph800436F0& second = result.submission.glyphs[1];
    const EventTextSubmittedGlyph800436F0& third = result.submission.glyphs[2];
    CHECK(first.psxAddress == kTextRecordGlyphBufferBase8008AB50);
    CHECK(first.packet.word0 == 0xC0FFFFFFu);
    CHECK(first.packet.r == 0x10u);
    CHECK(first.packet.g == 0x20u);
    CHECK(first.packet.b == 0x30u);
    CHECK(first.packet.x == 10);
    CHECK(first.packet.y == 20);
    CHECK(first.packet.u == 8u);
    CHECK(first.packet.v == 16u);
    CHECK(first.packet.code == 0x70u);
    CHECK(first.packet.clut == 0x6000u);
    CHECK(second.packet.word0 == 0xC108AB50u);
    CHECK(second.packet.x == 18);
    CHECK(second.packet.u == 8u);
    CHECK(second.packet.v == 16u);
    CHECK(third.packet.word0 == 0xC208AB60u);
    CHECK(third.packet.x == 34);
    CHECK(result.submission.recordBeforeCleanup.word0 == 0xBB08AB70u);
    CHECK(result.submission.headWord == 0xAA05CB5Cu);
    CHECK(result.submission.recordBeforeCleanup.appendCursor28 ==
          recordBefore.appendCursor28);
    CHECK(capture.observedHeadWord == result.submission.headWord);
    CHECK(capture.observedAppendCursor == 77u);
    CHECK(capture.observedFirstTextByte == '~');
    CHECK(capture.observedFirstPacketWord == first.packet.word0);

    CHECK(state.records[0].appendCursor28 == 0u);
    CHECK(state.records[0].parseBudget1C == recordBefore.parseBudget1C);
    CHECK(state.records[0].glyphPacketCursor20 ==
          recordBefore.glyphPacketCursor20);
    CHECK(state.textBytes[0] == 0u);
    CHECK(state.textBytes[1] == byteAfterFirst);
    CHECK(state.textBytes[10] == 'B');
    CHECK(state.glyphPackets[3].word0 == 0xC3000000u);

    const EventTextRuntimeState800436F0 afterExecute = state;
    int32_t replayResult = 0;
    capture.returnValue = 91;
    capture.stateToObserve = nullptr;
    CHECK(ReplayFrozenTextFlushSubmission800450A0(
        result.submission, CaptureSubmit, &capture, replayResult));
    CHECK(replayResult == 91);
    CHECK(capture.calls == 2);
    CHECK(capture.last.glyphCount == 3u);
    CHECK(SameState(state, afterExecute));
}

void TestWhitespaceWrapBottomAndMeasureMode() {
    EventTextRuntimeState800436F0 state = MakeState();
    state.glyphBankKnown = false;
    state.records[0].glyphPacketCursor20 = 1u;
    state.records[0].x = 5;
    state.records[0].y = 7;
    state.records[0].w = 200;
    state.records[0].h = 100;
    state.records[0].parseBudget1C = 3;
    WriteText(state, 0u, {' ', '\t', '\n', 0});
    SubmitCapture capture{};
    EventTextFlushExecutionResult800436F0 result{};
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.parserIterations == 3u);
    CHECK(result.localBudgetRemaining == 0);
    CHECK(result.submission.glyphCount == 0u);
    CHECK(result.maxX == 45);
    CHECK(result.finalX == 5);
    CHECK(result.finalY == 15);

    state = MakeState();
    state.records[0].x = 0;
    state.records[0].y = 0;
    state.records[0].w = 8;
    state.records[0].h = 8;
    state.records[0].parseBudget1C = 4;
    WriteText(state, 0u, {'A', 'B', 0});
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.stoppedAtBottom);
    CHECK(result.parserIterations == 1u);
    CHECK(result.localBudgetRemaining == 4);
    CHECK(result.finalTextAddress == kTextRecordTextBufferBase8008A750);
    CHECK(result.submission.glyphCount == 1u);
    CHECK(result.maxX == 8);
    CHECK(result.finalY == 8);

    state = MakeState();
    state.records[0].x = 0;
    state.records[0].y = 0;
    state.records[0].w = 8;
    state.records[0].h = 100;
    state.records[0].measureMode2C = 1;
    state.records[0].parseBudget1C = 4;
    WriteText(state, 0u, {'A', 'B', '\n', 'C', 0});
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(!result.stoppedAtBottom);
    CHECK(result.parserIterations == 4u);
    CHECK(result.submission.glyphCount == 3u);
    CHECK(result.submission.glyphs[1].packet.x == 8);
    CHECK(result.submission.glyphs[2].packet.x == 0);
    CHECK(result.submission.glyphs[2].packet.y == 8);
    CHECK(result.submission.recordBeforeCleanup.w == 16);
    CHECK(result.submission.recordBeforeCleanup.h == 16);
    CHECK(state.records[0].w == 16);
    CHECK(state.records[0].h == 16);
}

void TestPreSubmitCommitAndPostSubmitTextPointerReload() {
    EventTextRuntimeState800436F0 state = MakeState();
    WriteText(state, 0u, {'A', 0});
    state.textBytes[100] = 'Z';
    SubmitCapture capture{};
    capture.stateToObserve = &state;
    capture.redirectTextPointer = kTextRecordTextBufferBase8008A750 + 100u;
    EventTextFlushExecutionResult800436F0 result{};
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(capture.calls == 1);
    CHECK(capture.observedHeadWord == 0xAA08AB50u);
    CHECK(capture.observedAppendCursor == 77u);
    CHECK(capture.observedFirstTextByte == 'A');
    CHECK(capture.observedFirstPacketWord == 0xC0FFFFFFu);
    CHECK(state.records[0].textPointer24 ==
          kTextRecordTextBufferBase8008A750 + 100u);
    CHECK(state.records[0].appendCursor28 == 0u);
    CHECK(state.textBytes[0] == 'A');
    CHECK(state.textBytes[100] == 0u);
}

void TestPostSubmitHostTextPointerBindingLimitKeepsFormalCleanupOrder() {
    EventTextRuntimeState800436F0 state = MakeState();
    WriteText(state, 0u, {'A', 0});
    SubmitCapture capture{};
    capture.stateToObserve = &state;
    capture.redirectTextPointer = 0xDEADBEEFu;
    EventTextFlushExecutionResult800436F0 result{};
    CHECK(!ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(capture.calls == 1);
    CHECK(result.submitInvoked);
    CHECK(result.status ==
          EventTextFlushStatus800436F0::HostTextPointerBindingLimit);
    CHECK(state.records[0].textPointer24 == 0xDEADBEEFu);
    CHECK(state.records[0].appendCursor28 == 0u);
    CHECK(state.records[0].listHeadWord10 == 0xAA08AB50u);
    CHECK(state.glyphPackets[0].word0 == 0xC0FFFFFFu);
    CHECK(state.textBytes[0] == 'A');
}

void TestSignedNonzeroBudgetUsesOriginalLoopRule() {
    EventTextRuntimeState800436F0 state = MakeState();
    WriteText(state, 0u, {'A', 0});
    state.records[0].parseBudget1C = -1;
    SubmitCapture capture{};
    EventTextFlushExecutionResult800436F0 result{};
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.parserIterations == 1u);
    CHECK(result.localBudgetRemaining == -2);
    CHECK(result.submission.glyphCount == 1u);
    CHECK(state.records[0].parseBudget1C == -1);

    state = MakeState();
    WriteText(state, 0u, {'A', 0});
    state.records[0].parseBudget1C = 1025;
    capture = SubmitCapture{};
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.parserIterations == 1u);
    CHECK(result.localBudgetRemaining == 1024);
    CHECK(result.submission.glyphCount == 1u);
    CHECK(state.records[0].parseBudget1C == 1025);
}

void TestRecordBudgetBeyondOldPlannerCap() {
    EventTextRuntimeState800436F0 state = MakeState();
    state.records[0].x = 0;
    state.records[0].y = 0;
    state.records[0].w = 32000;
    state.records[0].h = 100;
    state.records[0].parseBudget1C = 129;
    for (uint32_t i = 0; i < 129u; ++i) {
        state.textBytes[i] = 'A';
    }
    state.textBytes[129] = 0u;
    SubmitCapture capture{};
    EventTextFlushExecutionResult800436F0 result{};
    CHECK(ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.parserIterations == 129u);
    CHECK(result.submission.glyphCount == 129u);
    CHECK(result.localBudgetRemaining == 0);
    CHECK(result.finalPacketAddress ==
          kTextRecordGlyphBufferBase8008AB50 +
              129u * kTextGlyphPacketStride800436F0);
}

void TestMalformedInputsFailAtomically() {
    SubmitCapture capture{};
    EventTextFlushExecutionResult800436F0 result{};

    EventTextRuntimeState800436F0 state = MakeState();
    state.records[0].textPointer24 =
        kTextRecordTextBufferBase8008A750 +
        kTextBufferBankByteCount80043394;
    EventTextRuntimeState800436F0 before = state;
    CHECK(!ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.status ==
          EventTextFlushStatus800436F0::HostTextPointerBindingLimit);
    CHECK(SameState(state, before));

    state = MakeState();
    WriteText(state, 0u, {'A', 0});
    state.records[0].glyphPacketCursor20 += 1u;
    before = state;
    CHECK(!ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.status ==
          EventTextFlushStatus800436F0::InvalidGlyphPacketCursor);
    CHECK(SameState(state, before));

    state = MakeState();
    WriteText(state, 0u, {'A', 0});
    state.records[0].glyphPacketCursor20 =
        kTextRecordGlyphBufferBase8008AB50 +
        kTextGlyphPacketBankCount80043394 * kTextGlyphPacketStride800436F0;
    before = state;
    CHECK(!ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.status ==
          EventTextFlushStatus800436F0::GlyphPacketRangeExhausted);
    CHECK(SameState(state, before));

    state = MakeState();
    state.records[0].textPointer24 =
        kTextRecordTextBufferBase8008A750 +
        kTextBufferBankByteCount80043394 - 2u;
    state.textBytes[kTextBufferBankByteCount80043394 - 2u] = '~';
    state.textBytes[kTextBufferBankByteCount80043394 - 1u] = 'c';
    before = state;
    CHECK(!ExecuteTextFlush800436F0(
        state, 0, CaptureSubmit, &capture, result));
    CHECK(result.status == EventTextFlushStatus800436F0::TextRangeExhausted);
    CHECK(SameState(state, before));

    state = MakeState();
    WriteText(state, 0u, {'A', 0});
    before = state;
    CHECK(!ExecuteTextFlush800436F0(state, 0, nullptr, nullptr, result));
    CHECK(result.status == EventTextFlushStatus800436F0::SubmitSinkMissing);
    CHECK(SameState(state, before));

    int32_t ignored = 123;
    CHECK(!ReplayFrozenTextFlushSubmission800450A0(
        EventTextFlushSubmission800436F0{},
        CaptureSubmit,
        &capture,
        ignored));
    CHECK(ignored == 0);
}

void TestSoftwareSubmitBridge800450A0() {
    EventTextFlushSubmission800436F0 submission{};
    submission.valid = true;
    submission.recordAddress = kTextRecordBase8005CB5C;
    submission.headAddress = submission.recordAddress + 0x10u;

    EventTextSubmitContext800450A0 context{};
    EventTextSubmitExecutionResult800450A0 result{};
    CHECK(ExecuteTextSubmitSoftware800450A0(submission, context, result));
    CHECK(result.accepted);
    CHECK(result.callbackDispatched);
    CHECK(result.status == EventTextSubmitStatus800450A0::HostGpuBoundary);
    CHECK(!result.exactPsxHalParity);
    CHECK(!result.hostRuntimeConsumerWired);
    CHECK(result.dispatch.status == EventTextDispatchStatus800468E0::Executed);
    CHECK(result.dispatch.callbackFunction == kFn80046840);
    CHECK(result.dispatch.copyWordCount == 0u);
    CHECK(result.dispatch.immediateCallbackArgumentsKnown);
    CHECK(result.dispatch.queueCapacity == 64u);
    CHECK(result.transaction.valid);
    CHECK(result.transaction.callbackFunction == kFn80046840);
    CHECK(result.transaction.otagHeadAddress == submission.headAddress);
    CHECK(result.transaction.gp1DmaDirection == 0x04000002u);
    CHECK(result.transaction.dma2Madr == submission.headAddress);
    CHECK(result.transaction.dma2Bcr == 0u);
    CHECK(result.transaction.dma2Chcr == 0x01000401u);
    CHECK(SubmitTextOtagSoftware800450A0(submission, &context) == 0);
    CHECK(context.last.transaction.dma2Madr == submission.headAddress);

    context.hostGpuBoundaryKnown = true;
    context.hostRuntimeConsumerWired = true;
    CHECK(ExecuteTextSubmitSoftware800450A0(submission, context, result));
    CHECK(result.status == EventTextSubmitStatus800450A0::Executed);
    CHECK(result.hostRuntimeConsumerWired);

    struct GlyphCapture {
        int calls = 0;
        EventTextHostGlyphDraw800450A0 last{};
    } glyphCapture{};
    auto captureGlyph = [](const EventTextHostGlyphDraw800450A0& draw,
                           void* userData) -> bool {
        auto& capture = *static_cast<GlyphCapture*>(userData);
        ++capture.calls;
        capture.last = draw;
        return true;
    };
    submission.glyphCount = 1u;
    submission.glyphs[0].psxAddress = kTextRecordGlyphBufferBase8008AB50;
    submission.glyphs[0].packet.x = 12;
    submission.glyphs[0].packet.y = 34;
    submission.glyphs[0].packet.u = 40;
    submission.glyphs[0].packet.v = 8;
    submission.glyphs[0].packet.r = 0xA0u;
    submission.glyphs[0].packet.g = 0xB0u;
    submission.glyphs[0].packet.b = 0xC0u;
    submission.glyphs[0].packet.clut = 0x603Cu;
    glyphCapture = {};
    context.hostGlyphSink = captureGlyph;
    context.hostGlyphSinkUserData = &glyphCapture;
    CHECK(ExecuteTextSubmitSoftware800450A0(submission, context, result));
    CHECK(result.hostGlyphCount == 1u);
    CHECK(result.hostGlyphsSubmitted);
    CHECK(!result.hostGlyphSubmitFailed);
    CHECK(glyphCapture.calls == 1);
    CHECK(glyphCapture.last.x == 12);
    CHECK(glyphCapture.last.y == 34);
    CHECK(glyphCapture.last.u == 40);
    CHECK(glyphCapture.last.v == 8);
    CHECK(glyphCapture.last.clut == 0x603Cu);

    submission.headAddress += 4u;
    CHECK(!ExecuteTextSubmitSoftware800450A0(submission, context, result));
    CHECK(result.status == EventTextSubmitStatus800450A0::InvalidHeadAddress);
}

} // namespace

int main() {
    CHECK(!RuntimeCutoverAllowed());
    CHECK(kTextBufferBankByteCount80043394 == 1024u);
    CHECK(kTextGlyphPacketBankCount80043394 == 1024u);
    TestSourceSlotAndFallbackNullGates();
    TestEmptyAndZeroBudgetStillSubmitAndClear();
    TestParserColorOtagCleanupAndFrozenReplay();
    TestWhitespaceWrapBottomAndMeasureMode();
    TestPreSubmitCommitAndPostSubmitTextPointerReload();
    TestPostSubmitHostTextPointerBindingLimitKeepsFormalCleanupOrder();
    TestSignedNonzeroBudgetUsesOriginalLoopRule();
    TestRecordBudgetBeyondOldPlannerCap();
    TestMalformedInputsFailAtomically();
    TestSoftwareSubmitBridge800450A0();

    if (g_failed != 0) {
        std::printf("test_ss0_event_text_flush: %d failed checks\n", g_failed);
        return 1;
    }
    std::printf("test_ss0_event_text_flush: ok\n");
    return 0;
}
