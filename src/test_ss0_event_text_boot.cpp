#include "pr/pr_ss0_event_text_direct.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>

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

EventTextFontBinding80043394 MakeBinding(uint8_t gpuType = 1u) {
    EventTextFontBinding80043394 binding{};
    binding.known = true;
    binding.tpage = 0x2345u;
    binding.clut = 0xA55Au;
    binding.gpuType8005D734 = gpuType;
    return binding;
}

bool SameState(const EventTextRuntimeState800436F0& lhs,
               const EventTextRuntimeState800436F0& rhs) {
    return std::memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}

EventTextRuntimeState800436F0 MakeLoadedState(uint8_t gpuType = 1u) {
    EventTextRuntimeState800436F0 state{};
    state.textBankKnown = true;
    state.glyphBankKnown = true;
    EventTextFntLoadResult80043394 result{};
    CHECK(ExecuteFntLoadSoftware80043394(
        state, MakeBinding(gpuType), result));
    CHECK(result.status == EventTextFntLoadStatus80043394::Executed);
    return state;
}

struct DisplayCapture {
    EventTextRuntimeState800436F0* state = nullptr;
    int calls = 0;
    int32_t enabled = 0;
    int32_t returnValue = 0;
    int32_t observedCount = -1;
    int32_t observedCurrent = -1;
    int32_t observedCursor = -1;
    uint32_t observedCallback = 0;
};

int32_t CaptureDisplay(int32_t enabled, void* userData) {
    DisplayCapture& capture = *static_cast<DisplayCapture*>(userData);
    ++capture.calls;
    capture.enabled = enabled;
    if (capture.state != nullptr) {
        capture.observedCount = capture.state->recordCount8005CCDC;
        capture.observedCurrent = capture.state->currentRecord8005CCE0;
        capture.observedCursor = capture.state->allocationCursor8005D6E4;
        capture.observedCallback = capture.state->appendCallback8005D730;
    }
    return capture.returnValue;
}

struct MenuHelpSubmitCapture80026314 {
    int calls = 0;
    uint32_t selectedSlot = 0;
    uint32_t glyphCount = 0;
    uint32_t headAddress = 0;
    int32_t returnValue = 0;
};

int32_t CaptureMenuHelpSubmit80026314(
    const EventTextFlushSubmission800436F0& submission,
    void* userData) {
    auto& capture =
        *static_cast<MenuHelpSubmitCapture80026314*>(userData);
    ++capture.calls;
    capture.selectedSlot = submission.selectedSlot;
    capture.glyphCount = submission.glyphCount;
    capture.headAddress = submission.headAddress;
    return capture.returnValue;
}

struct DispatchCapture80046BC4 {
    std::vector<uint32_t> callbacks;
    std::vector<uint32_t> heads;
    std::vector<uint32_t> arguments;
    std::vector<uint32_t> payloadSizes;
    std::vector<uint8_t> payload;
    bool returnValue = true;
};

bool CaptureDispatch80046BC4(
    const EventTextDispatchCallbackInvocation80046BC4& invocation,
    void* userData) {
    auto& capture = *static_cast<DispatchCapture80046BC4*>(userData);
    capture.callbacks.push_back(invocation.callbackFunction);
    capture.heads.push_back(invocation.headAddress);
    capture.arguments.push_back(invocation.callbackArgument);
    capture.payloadSizes.push_back(invocation.payloadByteCount);
    if (invocation.payloadBytes != nullptr && invocation.payloadByteCount != 0u) {
        capture.payload.assign(invocation.payloadBytes,
                               invocation.payloadBytes +
                                   invocation.payloadByteCount);
    }
    return capture.returnValue;
}

void TestFntLoadClearsOnlyFormalRecordState() {
    EventTextRuntimeState800436F0 state{};
    std::memset(state.records.data(), 0x7C,
                sizeof(EventTextRecord800436F0) * state.records.size());
    state.recordCount8005CCDC = 7;
    state.currentRecord8005CCE0 = 6;
    state.allocationCursor8005D6E4 = 777;
    state.appendCallback8005D730 = 0x12345678u;
    state.textBytes[9] = 0xA1u;
    state.glyphPackets[9].word0 = 0xB2B2B2B2u;

    EventTextRuntimeState800436F0 before = state;
    EventTextFntLoadResult80043394 result{};
    CHECK(!ExecuteFntLoadSoftware80043394(
        state, EventTextFontBinding80043394{}, result));
    CHECK(result.status ==
          EventTextFntLoadStatus80043394::FontBindingUnknown);
    CHECK(SameState(state, before));

    const EventTextFontBinding80043394 binding = MakeBinding();
    CHECK(ExecuteFntLoadSoftware80043394(state, binding, result));
    CHECK(result.accepted);
    CHECK(!result.fontUploadExecuted);
    CHECK(result.returnedRecordBankAddress == kTextRecordBase8005CB5C);
    CHECK(state.recordCount8005CCDC == 0);
    CHECK(state.recordBankKnown);
    CHECK(state.recordCountKnown);
    CHECK(state.fontBindingKnown);
    CHECK(!state.globalsKnown);
    CHECK(!state.currentRecordKnown);
    CHECK(!state.allocationCursorKnown);
    CHECK(!state.appendCallbackKnown);
    CHECK(!state.textBankKnown);
    CHECK(!state.glyphBankKnown);
    CHECK(state.allocationCursor8005D6E4 == 777);
    CHECK(state.currentRecord8005CCE0 == 6);
    CHECK(state.appendCallback8005D730 == 0x12345678u);
    CHECK(state.textBytes[9] == 0xA1u);
    CHECK(state.glyphPackets[9].word0 == 0xB2B2B2B2u);
    CHECK(state.fontTPage8008EB50 == binding.tpage);
    CHECK(state.fontClut8008EB54 == binding.clut);
    for (const EventTextRecord800436F0& record : state.records) {
        CHECK(record.word0 == 0u);
        CHECK(record.listHeadWord10 == 0u);
        CHECK(record.textPointer24 == 0u);
    }
}

void TestFntLoadBindingMatchesCurrentIdaGetTPageGetClut() {
    EventTextFontBinding80043394 binding{};
    CHECK(ComputeFntLoadFontBinding80043394(960, 256, 0, 0u, binding));
    CHECK(binding.known);
    CHECK(binding.tpage == 0x001Fu);
    CHECK(binding.clut == 0x603Cu);
    CHECK(binding.gpuType8005D734 == 0u);

    CHECK(ComputeFntLoadFontBinding80043394(960, 256, 0, 1u, binding));
    CHECK(binding.tpage == 0x002Fu);
    CHECK(binding.clut == 0x603Cu);
    CHECK(!ComputeFntLoadFontBinding80043394(960, 256, 3, 0u, binding));
    CHECK(!binding.known);
    CHECK(!ComputeFntLoadFontBinding80043394(960, 960, 0, 0u, binding));
    CHECK(!binding.known);
}

void TestEmbeddedFontDecodeUsesFormal4bppLayout() {
    const size_t fileOffset = 0x40u;
    std::vector<uint8_t> raw(
        fileOffset + kEmbeddedFontClutByteCount + 0x200u +
            kEmbeddedFontImageByteCount,
        0u);
    raw[fileOffset + 0u] = 0x00u;
    raw[fileOffset + 1u] = 0x00u;
    raw[fileOffset + 2u] = 0xFFu;
    raw[fileOffset + 3u] = 0xFFu;
    raw[fileOffset + 0x200u] = 0x10u;
    raw[fileOffset + 0x201u] = 0x11u;

    EventTextEmbeddedFont8005CCE4 font{};
    CHECK(DecodeEmbeddedFont8005CCE4(
        raw.data(), raw.size(), fileOffset, font));
    CHECK(font.sourceKnown);
    CHECK(font.fileOffset == fileOffset);
    CHECK(font.clutBytes[0] == 0x00u);
    CHECK(font.clutBytes[1] == 0x00u);
    CHECK(font.imageBytes[0] == 0x10u);
    CHECK(font.imageBytes[1] == 0x11u);
    CHECK(font.clut[0] == 0x0000u);
    CHECK(font.clut[1] == 0xFFFFu);
    CHECK(font.rgba[0] == 0x00000000u);
    CHECK(font.rgba[1] == 0xFFFFFFFFu);
    CHECK(font.rgba[2] == 0xFFFFFFFFu);
    CHECK(font.rgba[3] == 0xFFFFFFFFu);
    CHECK(!DecodeEmbeddedFont8005CCE4(
        raw.data(), raw.size() - 1u, fileOffset, font));
    CHECK(!font.sourceKnown);
}

void TestEmbeddedFontDecodeReadsOriginalExecutableSource() {
    const std::filesystem::path cwd = std::filesystem::current_path();
    const std::array<std::filesystem::path, 4> candidates = {
        cwd / "SCUS_941.83",
        cwd.parent_path() / "SCUS_941.83",
        cwd.parent_path().parent_path() / "SCUS_941.83",
        cwd.parent_path().parent_path().parent_path() / "SCUS_941.83",
    };
    std::filesystem::path original;
    for (const auto& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate)) {
            original = candidate;
            break;
        }
    }
    std::ifstream input(original, std::ios::binary | std::ios::ate);
    CHECK(static_cast<bool>(input));
    if (!input) {
        return;
    }
    const std::streamoff end = input.tellg();
    CHECK(end > 0);
    if (end <= 0) {
        return;
    }
    std::vector<uint8_t> bytes(static_cast<size_t>(end));
    input.seekg(0, std::ios::beg);
    CHECK(input.read(reinterpret_cast<char*>(bytes.data()), end));

    constexpr size_t fileOffset =
        kEmbeddedFontFilePayloadOffset +
        (kEmbeddedFontSourceAddress8005CCE4 - kEmbeddedFontTextLoadAddress);
    EventTextEmbeddedFont8005CCE4 font{};
    CHECK(DecodeEmbeddedFont8005CCE4(
        bytes.data(), bytes.size(), fileOffset, font));
    CHECK(font.sourceKnown);
    CHECK(font.sourceAddress == kEmbeddedFontSourceAddress8005CCE4);
    CHECK(font.imageAddress == kEmbeddedFontImageAddress8005CEE4);
    CHECK(font.fileOffset == fileOffset);
    CHECK(font.clut[0] == 0x0000u);
    CHECK(font.clut[1] == 0xFFFFu);
}

EventTextRuntimeState800436F0 MakeAppendState(uint32_t capacity = 128u) {
    EventTextRuntimeState800436F0 state = MakeLoadedState();
    EventTextFntOpenRequest80043438 open{};
    open.x = -156;
    open.y = -120;
    open.width = 320;
    open.height = 200;
    open.requestedCapacity = static_cast<int32_t>(capacity);
    EventTextFntOpenResult80043438 openResult{};
    CHECK(ExecuteFntOpenSoftware80043438(state, open, openResult));
    CHECK(openResult.status == EventTextFntOpenStatus80043438::Executed);
    EventTextSetDumpFntResult80043354 dumpResult{};
    CHECK(ExecuteSetDumpFntSoftware80043354(
        state, openResult.returnedSlot, dumpResult));
    CHECK(dumpResult.updated);
    return state;
}

void TestTextAppendSoftwareCoreFormatsBoundHostArguments() {
    EventTextRuntimeState800436F0 state = MakeAppendState();
    EventTextAppendRequest80043A14 request{};
    request.slot = -1;
    request.format = "A%%:%d:%04x:%X:%c:%5s";
    request.argCount = 5;
    request.args[0].kind = EventTextAppendArgKind80043A14::Signed32;
    request.args[0].signedValue = -12;
    request.args[1].kind = EventTextAppendArgKind80043A14::Unsigned32;
    request.args[1].unsignedValue = 0x2Au;
    request.args[2].kind = EventTextAppendArgKind80043A14::Unsigned32;
    request.args[2].unsignedValue = 0xBEEFu;
    request.args[3].kind = EventTextAppendArgKind80043A14::Character;
    request.args[3].characterValue = 'Q';
    request.args[4].kind = EventTextAppendArgKind80043A14::String;
    request.args[4].stringValue = "OK";

    EventTextAppendResult80043A14 result{};
    CHECK(ExecuteTextAppendSoftware80043A14(state, request, result));
    CHECK(result.status == EventTextAppendStatus80043A14::Executed);
    CHECK(result.accepted);
    CHECK(result.usedFallbackSlot);
    CHECK(result.selectedSlot == 0u);
    CHECK(result.argumentsConsumed == 5u);
    CHECK(result.initialCursor == 0u);
    const char expected[] = "A%:-12:002a:BEEF:Q:   OK";
    CHECK(result.appendedBytes == sizeof(expected) - 1u);
    CHECK(result.finalCursor == sizeof(expected) - 1u);
    CHECK(result.returnedCursor == static_cast<int32_t>(sizeof(expected) - 1u));
    CHECK(std::strcmp(
        reinterpret_cast<const char*>(state.textBytes.data()), expected) == 0);
    CHECK(state.records[0].appendCursor28 == sizeof(expected) - 1u);

    request = {};
    request.format = "!";
    CHECK(ExecuteTextAppendSoftware80043A14(state, request, result));
    CHECK(result.initialCursor == sizeof(expected) - 1u);
    CHECK(result.finalCursor == sizeof(expected));
    CHECK(state.textBytes[sizeof(expected) - 1u] == '!');
    CHECK(state.textBytes[sizeof(expected)] == 0u);
}

void TestTextAppendSoftwareCoreRejectsUnboundInputsAtomically() {
    EventTextRuntimeState800436F0 state = MakeAppendState(32u);
    EventTextAppendRequest80043A14 request{};
    request.format = "%s";
    request.argCount = 1;
    request.args[0].kind = EventTextAppendArgKind80043A14::String;
    EventTextAppendResult80043A14 result{};
    EventTextRuntimeState800436F0 before = state;
    CHECK(!ExecuteTextAppendSoftware80043A14(state, request, result));
    CHECK(result.status == EventTextAppendStatus80043A14::StringBindingLimit);
    CHECK(SameState(state, before));

    request = {};
    request.format = "%q";
    CHECK(!ExecuteTextAppendSoftware80043A14(state, request, result));
    CHECK(result.status == EventTextAppendStatus80043A14::UnsupportedSpecifier);
    CHECK(SameState(state, before));

    request = {};
    request.format = "%d";
    CHECK(!ExecuteTextAppendSoftware80043A14(state, request, result));
    CHECK(result.status == EventTextAppendStatus80043A14::MissingArgument);
    CHECK(SameState(state, before));

    state.records[0].appendCursor28 = 31u;
    state.textBytes[31] = 0u;
    before = state;
    request = {};
    request.format = "X";
    CHECK(!ExecuteTextAppendSoftware80043A14(state, request, result));
    CHECK(result.status == EventTextAppendStatus80043A14::CapacityBindingLimit);
    CHECK(SameState(state, before));
}

void TestMenuHelpSoftwareCoreExecutesFormalAppendAndFlushOrder() {
    EventTextRuntimeState800436F0 state = MakeAppendState(512u);
    std::array<EventTextMenuHelpGroup80026314, 2> groups{};
    groups[0].title = "GAME";
    groups[0].itemCount = 2u;
    groups[0].itemLabels[0] = "START";
    groups[0].itemLabels[1] = "LOAD";
    groups[0].selectedItem = 1;
    groups[1].title = "OPTION";
    groups[1].itemCount = 1u;
    groups[1].itemLabels[0] = "LANG";
    groups[1].selectedItem = 0;

    EventTextMenuHelpContext80026314 context{};
    context.sourceKnown = true;
    context.groups = groups.data();
    context.groupCount = static_cast<uint32_t>(groups.size());
    context.selectedGroup = 0;

    MenuHelpSubmitCapture80026314 capture{};
    EventTextMenuHelpResult80026314 result{};
    CHECK(ExecuteMenuHelpSoftware80026314(
        state, context, CaptureMenuHelpSubmit80026314, &capture, result));
    CHECK(result.status == EventTextMenuHelpStatus80026314::Executed);
    CHECK(result.accepted);
    CHECK(result.flushInvoked);
    CHECK(result.flushAccepted);
    CHECK(result.groupsProcessed == 2u);
    CHECK(result.itemsProcessed == 3u);
    const char expected[] =
        "\n\n\n\n"
        "~c888 >>>GAME:~c000 START ~c888*LOAD  \n"
        "~c000    OPTION:~c888*LANG  \n"
        "\n\n~c222      O: OK   X: CANCEL~c888\n";
    CHECK(result.appendedBytes == sizeof(expected) - 1u);
    CHECK(capture.calls == 1);
    CHECK(capture.selectedSlot == 0u);
    CHECK(capture.glyphCount != 0u);
    CHECK(capture.headAddress == kTextRecordBase8005CB5C + 0x10u);
    CHECK(state.records[0].appendCursor28 == 0u);
    CHECK(state.textBytes[0] == 0u);
}

void TestMenuHelpSoftwareCoreRejectsUnknownSourcesAtomically() {
    EventTextRuntimeState800436F0 state = MakeAppendState(512u);
    const EventTextRuntimeState800436F0 before = state;
    EventTextMenuHelpContext80026314 context{};
    EventTextMenuHelpResult80026314 result{};
    CHECK(!ExecuteMenuHelpSoftware80026314(
        state, context, CaptureMenuHelpSubmit80026314, nullptr, result));
    CHECK(result.status == EventTextMenuHelpStatus80026314::SourceUnknown);
    CHECK(SameState(state, before));

    EventTextMenuHelpGroup80026314 group{};
    group.title = "GAME";
    group.itemCount = 1u;
    context.sourceKnown = true;
    context.groups = &group;
    context.groupCount = 1u;
    context.selectedGroup = 0;
    CHECK(!ExecuteMenuHelpSoftware80026314(
        state, context, CaptureMenuHelpSubmit80026314, nullptr, result));
    CHECK(result.status == EventTextMenuHelpStatus80026314::InvalidItemLabel);
    CHECK(SameState(state, before));

    group.itemLabels[0] = "START";
    CHECK(!ExecuteMenuHelpSoftware80026314(
        state, context, nullptr, nullptr, result));
    CHECK(result.status == EventTextMenuHelpStatus80026314::FlushFailed);
    CHECK(result.flush.status ==
          EventTextFlushStatus800436F0::SubmitSinkMissing);
    CHECK(SameState(state, before));
}

void TestStageClearSoftwareCoreExecutesFormalAppendAndFlushOrder() {
    EventTextRuntimeState800436F0 state = MakeAppendState(512u);
    EventTextStageClearContext80026B94 context{};
    context.sourceKnown = true;
    context.gateKnown = true;
    context.stageClearEnabled = true;
    context.statusBytes = {1u, 0u, 2u, 7u, 8u, 9u};

    MenuHelpSubmitCapture80026314 capture{};
    EventTextStageClearResult80026B94 result{};
    CHECK(ExecuteStageClearSoftware80026B94(
        state, context, CaptureMenuHelpSubmit80026314, &capture, result));
    CHECK(result.status == EventTextStageClearStatus80026B94::Executed);
    CHECK(result.accepted);
    CHECK(result.flushInvoked);
    CHECK(result.flushAccepted);
    CHECK(result.statusesAppended == kStageClearStatusAppendCount80026DAC);
    const char expected[] = "\n\n\n~c000StageClear: 102789";
    CHECK(result.appendedBytes == sizeof(expected) - 1u);
    CHECK(capture.calls == 1);
    CHECK(capture.glyphCount != 0u);
    CHECK(state.records[0].appendCursor28 == 0u);
    CHECK(state.textBytes[0] == 0u);
}

void TestStageClearSoftwareCoreRejectsUnknownGateAndSinkAtomically() {
    EventTextRuntimeState800436F0 state = MakeAppendState(512u);
    const EventTextRuntimeState800436F0 before = state;
    EventTextStageClearContext80026B94 context{};
    EventTextStageClearResult80026B94 result{};
    CHECK(!ExecuteStageClearSoftware80026B94(
        state, context, CaptureMenuHelpSubmit80026314, nullptr, result));
    CHECK(result.status == EventTextStageClearStatus80026B94::SourceUnknown);
    CHECK(SameState(state, before));

    context.sourceKnown = true;
    CHECK(!ExecuteStageClearSoftware80026B94(
        state, context, CaptureMenuHelpSubmit80026314, nullptr, result));
    CHECK(result.status == EventTextStageClearStatus80026B94::GateUnknown);
    CHECK(SameState(state, before));

    context.gateKnown = true;
    context.stageClearEnabled = false;
    CHECK(!ExecuteStageClearSoftware80026B94(
        state, context, CaptureMenuHelpSubmit80026314, nullptr, result));
    CHECK(result.status == EventTextStageClearStatus80026B94::GateDisabled);
    CHECK(SameState(state, before));

    context.stageClearEnabled = true;
    CHECK(!ExecuteStageClearSoftware80026B94(
        state, context, nullptr, nullptr, result));
    CHECK(result.status == EventTextStageClearStatus80026B94::FlushFailed);
    CHECK(result.flush.status ==
          EventTextFlushStatus800436F0::SubmitSinkMissing);
    CHECK(SameState(state, before));
}

void TestLoadImageSoftwareCoreCapturesFormalDispatch() {
    EventTextRuntimeState800436F0 state = MakeLoadedState();
    const std::array<uint8_t, 16> pixels{};
    EventTextLoadImageRequest80044D64 request{};
    request.rect = {960, 256, 32, 32};
    request.pixelBytes = pixels.data();
    request.pixelByteCount = pixels.size();
    request.pixelAddress = kEmbeddedFontImageAddress8005CEE4;
    request.pixelAddressKnown = true;

    EventTextLoadImageResult80044D64 result{};
    CHECK(ExecuteLoadImageSoftware80044D64(state, request, result));
    CHECK(result.status == EventTextLoadImageStatus80044D64::Executed);
    CHECK(result.accepted);
    CHECK(!result.exactPsxHalParity);
    CHECK(!result.rectValidationWarning);
    CHECK(result.wrapperFunction == kFn80044D64);
    CHECK(result.rectLoggerFunction == kFn80044BA8);
    CHECK(result.lowerDispatchFunction == kFn800468E0);
    CHECK(result.lowerUploadCallback == kFn800462C4);
    CHECK(result.dispatchWidth == 8u);
    CHECK(result.rect.x == 960);
    CHECK(result.rect.y == 256);
    CHECK(result.rect.w == 32);
    CHECK(result.rect.h == 32);
    CHECK(result.pixelAddress == kEmbeddedFontImageAddress8005CEE4);
    CHECK(result.pixelByteCount == pixels.size());
    CHECK(state.loadImageTransactionKnown);
}

void TestLoadImageSoftwareCoreRejectsUnboundInputsAtomically() {
    EventTextRuntimeState800436F0 state = MakeLoadedState();
    const std::array<uint8_t, 16> pixels{};
    EventTextLoadImageRequest80044D64 request{};
    request.rect = {960, 256, 32, 32};
    request.pixelBytes = pixels.data();
    request.pixelByteCount = pixels.size();
    EventTextLoadImageResult80044D64 result{};
    const EventTextRuntimeState800436F0 before = state;

    CHECK(!ExecuteLoadImageSoftware80044D64(state, request, result));
    CHECK(result.status == EventTextLoadImageStatus80044D64::PixelBindingLimit);
    CHECK(SameState(state, before));

    request.pixelAddressKnown = true;
    request.pixelBytes = nullptr;
    CHECK(!ExecuteLoadImageSoftware80044D64(state, request, result));
    CHECK(result.status == EventTextLoadImageStatus80044D64::PixelBindingLimit);
    CHECK(SameState(state, before));
}

void TestTextDispatchSoftwareCapturesFormalQueueShape() {
    const EventTextPsxRect80044D64 rect = {960, 256, 32, 32};
    std::array<uint8_t, sizeof(rect)> rectBytes{};
    std::memcpy(rectBytes.data(), &rect, rectBytes.size());
    EventTextDispatchRequest800468E0 request{};
    request.callbackFunction = kFn800462C4;
    request.headBytes = rectBytes.data();
    request.headByteCount = rectBytes.size();
    request.copyByteCount = static_cast<uint32_t>(rectBytes.size());
    request.callbackArgument = kEmbeddedFontImageAddress8005CEE4;
    request.callbackArgumentKnown = true;

    EventTextDispatchResult800468E0 result{};
    CHECK(ExecuteTextDispatchSoftware800468E0(request, result));
    CHECK(result.status == EventTextDispatchStatus800468E0::Executed);
    CHECK(result.accepted);
    CHECK(result.dispatcherFunction == kFn800468E0);
    CHECK(result.callbackFunction == kFn800462C4);
    CHECK(result.copyByteCount == 8u);
    CHECK(result.copyWordCount == 2u);
    CHECK(result.queueCapacity == 64u);
    CHECK(result.queueStride == 0x60u);
    CHECK(result.queueCallbackBase == 0x80094448u);
    CHECK(result.queuePayloadBase == 0x80094454u);
    CHECK(result.queueShapeKnown);
    CHECK(!result.immediateCallbackArgumentsKnown);
    CHECK(std::memcmp(result.copiedHeadBytes.data(), rectBytes.data(),
                      rectBytes.size()) == 0);
}

void TestTextDispatchSoftwareRejectsUnboundQueuePayloadAtomically() {
    EventTextDispatchRequest800468E0 request{};
    request.callbackFunction = kFn800462C4;
    request.copyByteCount = 8u;
    EventTextDispatchResult800468E0 result{};
    CHECK(!ExecuteTextDispatchSoftware800468E0(request, result));
    CHECK(result.status ==
          EventTextDispatchStatus800468E0::QueuePayloadBindingLimit);

    request = {};
    request.copyByteCount = 0x55u;
    request.callbackFunction = kFn800462C4;
    CHECK(!ExecuteTextDispatchSoftware800468E0(request, result));
    CHECK(result.status ==
          EventTextDispatchStatus800468E0::QueuePayloadSizeLimit);
}

void TestTextDispatchQueueSoftwareCapturesProducerConsumerOrder() {
    const std::array<uint8_t, 8> source =
        {0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66u, 0x77u, 0x88u};
    DispatchCapture80046BC4 capture{};
    EventTextDispatchQueueState800468E0 state{};
    EventTextDispatchQueueRequest800468E0 request{};
    request.dispatch.callbackFunction = kFn800462C4;
    request.dispatch.headAddress = 0x80012340u;
    request.dispatch.headAddressKnown = true;
    request.dispatch.headBytes = source.data();
    request.dispatch.headByteCount = source.size();
    request.dispatch.copyByteCount = 3u;
    request.dispatch.callbackArgument = 0x8005CEE4u;
    request.dispatch.callbackArgumentKnown = true;
    request.forceQueue = true;
    request.callbackSink = CaptureDispatch80046BC4;
    request.callbackSinkUserData = &capture;

    EventTextDispatchQueueResult80046BC4 result{};
    CHECK(ExecuteTextDispatchQueueSoftware800468E0(request, state, result));
    CHECK(result.status == EventTextDispatchQueueStatus80046BC4::Executed);
    CHECK(result.accepted);
    CHECK(!result.immediatePath);
    CHECK(result.processedCount == 1u);
    CHECK(result.producerBefore == 0u);
    CHECK(result.producerAfter == 1u);
    CHECK(result.consumerAfter == 1u);
    CHECK(result.remainingCount == 0u);
    CHECK(capture.callbacks.size() == 1u);
    CHECK(capture.callbacks[0] == kFn800462C4);
    CHECK(capture.heads[0] == kEventTextDispatchQueuePayloadBase800468E0);
    CHECK(capture.arguments[0] == 0x8005CEE4u);
    CHECK(capture.payloadSizes[0] == 4u);
    CHECK(capture.payload.size() == 4u);
    CHECK(capture.payload[0] == 0x11u && capture.payload[3] == 0x44u);

    request.forceQueue = false;
    request.dispatch.headAddress = 0x80023450u;
    request.dispatch.copyByteCount = 8u;
    CHECK(ExecuteTextDispatchQueueSoftware800468E0(request, state, result));
    CHECK(result.immediatePath);
    CHECK(result.producerAfter == result.producerBefore);
    CHECK(capture.callbacks.size() == 2u);
    CHECK(capture.heads[1] == 0x80023450u);

    state = {};
    state.dmaActive = true;
    request.forceQueue = true;
    CHECK(ExecuteTextDispatchQueueSoftware800468E0(request, state, result));
    CHECK(result.accepted);
    CHECK(result.status == EventTextDispatchQueueStatus80046BC4::Executed);
    CHECK(result.remainingCount == 1u);
    state.dmaActive = false;
    CHECK(ExecuteTextDispatchConsumerSoftware80046BC4(
        state, CaptureDispatch80046BC4, &capture, result));
    CHECK(result.processedCount == 1u);
    CHECK(result.remainingCount == 0u);

    state = {};
    state.producerIndex8005D838 =
        kEventTextDispatchQueueCapacity800468E0 - 1u;
    state.consumerIndex8005D83C = 0u;
    const EventTextDispatchQueueState800468E0 before = state;
    request.forceQueue = true;
    CHECK(!ExecuteTextDispatchQueueSoftware800468E0(request, state, result));
    CHECK(result.status == EventTextDispatchQueueStatus80046BC4::QueueFull);
    CHECK(state.producerIndex8005D838 == before.producerIndex8005D838);
    CHECK(state.consumerIndex8005D83C == before.consumerIndex8005D83C);

    state = {};
    state.completionPending = true;
    state.completionCallbackFunction = 0x8004ABCDu;
    state.completionCallbackArgument = 0x1234u;
    CHECK(ExecuteTextDispatchConsumerSoftware80046BC4(
        state, CaptureDispatch80046BC4, &capture, result));
    CHECK(result.completionCallbackInvoked);
    CHECK(!state.completionPending);
    CHECK(capture.callbacks.back() == 0x8004ABCDu);

    state = {};
    request.dispatch.copyByteCount = 3u;
    request.dispatch.headByteCount = 3u;
    const EventTextDispatchQueueState800468E0 shortBefore = state;
    CHECK(!ExecuteTextDispatchQueueSoftware800468E0(request, state, result));
    CHECK(result.status ==
          EventTextDispatchQueueStatus80046BC4::PayloadBindingLimit);
    CHECK(state.producerIndex8005D838 == shortBefore.producerIndex8005D838);
    CHECK(state.consumerIndex8005D83C == shortBefore.consumerIndex8005D83C);
}

void TestGpuUploadWaitSoftwareCapturesTimeoutGuard() {
    EventTextGpuWaitState80047144 state{};
    EventTextGpuWaitResult80047178 result{};
    CHECK(ExecuteGpuUploadPrepareSoftware80047144(100u, state, result));
    CHECK(result.status == EventTextGpuWaitStatus80047178::PollCompleted);
    CHECK(result.accepted);
    CHECK(result.deadlineTick == 100u + 0xF0u);
    CHECK(state.prepared);
    CHECK(state.pollCount == 0u);

    CHECK(ExecuteGpuUploadWaitSoftware80047178(200u, state, result));
    CHECK(result.status == EventTextGpuWaitStatus80047178::PollCompleted);
    CHECK(!result.deadlineOverdue);
    CHECK(result.pollCount == 0u);

    CHECK(ExecuteGpuUploadWaitSoftware80047178(400u, state, result));
    CHECK(result.deadlineOverdue);
    CHECK(result.pollCount == 1u);

    state.pollCount = kEventTextGpuWaitPollLimit80047178 + 1u;
    state.prepared = true;
    CHECK(!ExecuteGpuUploadWaitSoftware80047178(500u, state, result));
    CHECK(result.status == EventTextGpuWaitStatus80047178::TimedOut);
    CHECK(result.resetIssued);
    CHECK(result.resetDma2Chcr == 0x401u);
    CHECK(result.resetDmaControlMask == 0x800u);
    CHECK(result.resetGpuStatusFirst == 0x20000000u);
    CHECK(result.resetGpuStatusSecond == 0x10000000u);
    CHECK(!state.prepared);

    state = {};
    CHECK(!ExecuteGpuUploadWaitSoftware80047178(1u, state, result));
    CHECK(result.status ==
          EventTextGpuWaitStatus80047178::PreparationBindingLimit);
}

void TestLoadImageUploadSoftwareCapturesFormalCpuAndDmaSplit() {
    const std::array<uint8_t, kEmbeddedFontClutByteCount> clutBytes{};
    EventTextLoadImageRequest80044D64 clutRequest{};
    clutRequest.rect = {960, 384, 16, 1};
    clutRequest.pixelBytes = clutBytes.data();
    clutRequest.pixelByteCount = clutBytes.size();
    clutRequest.pixelAddress = kEmbeddedFontSourceAddress8005CCE4;
    clutRequest.pixelAddressKnown = true;

    EventTextUploadResult800462C4 clutResult{};
    CHECK(ExecuteLoadImageUploadSoftware800462C4(clutRequest, clutResult));
    CHECK(clutResult.status == EventTextUploadStatus800462C4::Executed);
    CHECK(clutResult.callbackFunction == kFn800462C4);
    CHECK(clutResult.prepareFunction == kFn80047144);
    CHECK(clutResult.waitFunction == kFn80047178);
    CHECK(clutResult.transferWords == 8u);
    CHECK(clutResult.cpuTailWords == 8u);
    CHECK(clutResult.cpuTailWritten);
    CHECK(!clutResult.dmaSubmitted);
    CHECK(clutResult.dma2Madr ==
          kEmbeddedFontSourceAddress8005CCE4 + 8u * sizeof(uint32_t));

    const std::array<uint8_t, kEmbeddedFontImageByteCount> imageBytes{};
    EventTextLoadImageRequest80044D64 imageRequest{};
    imageRequest.rect = {960, 256, 32, 32};
    imageRequest.pixelBytes = imageBytes.data();
    imageRequest.pixelByteCount = imageBytes.size();
    imageRequest.pixelAddress = kEmbeddedFontImageAddress8005CEE4;
    imageRequest.pixelAddressKnown = true;

    EventTextUploadResult800462C4 imageResult{};
    CHECK(ExecuteLoadImageUploadSoftware800462C4(imageRequest, imageResult));
    CHECK(imageResult.transferWords == 512u);
    CHECK(imageResult.cpuTailWords == 0u);
    CHECK(imageResult.dmaBlockCount == 32u);
    CHECK(imageResult.dmaSubmitted);
    CHECK(imageResult.gp0Command == 0xA0000000u);
    CHECK(imageResult.gp1DmaDirection == 0x04000002u);
    CHECK(imageResult.dma2Madr == kEmbeddedFontImageAddress8005CEE4);
    CHECK(imageResult.dma2Bcr == 0x00200010u);
    CHECK(imageResult.dma2Chcr == 0x01000201u);
}

void TestLoadImageUploadSoftwareRejectsShortSourceAtomically() {
    const std::array<uint8_t, 31> shortBytes{};
    EventTextLoadImageRequest80044D64 request{};
    request.rect = {960, 384, 16, 1};
    request.pixelBytes = shortBytes.data();
    request.pixelByteCount = shortBytes.size();
    request.pixelAddress = kEmbeddedFontSourceAddress8005CCE4;
    request.pixelAddressKnown = true;

    EventTextUploadResult800462C4 result{};
    CHECK(!ExecuteLoadImageUploadSoftware800462C4(request, result));
    CHECK(result.status == EventTextUploadStatus800462C4::TransferSizeLimit);
    CHECK(!result.accepted);
}

void TestFntOpenFirstAllocationAndPacketWrites() {
    EventTextRuntimeState800436F0 state = MakeLoadedState();
    state.allocationCursor8005D6E4 = 999;
    std::memset(&state.glyphPackets[0], 0x5A,
                sizeof(state.glyphPackets[0]));
    std::memset(&state.glyphPackets[1], 0x6B,
                sizeof(state.glyphPackets[1]));
    state.textBytes[0] = 'X';
    state.textBytes[1] = 'Y';

    EventTextFntOpenRequest80043438 request{};
    request.x = -156;
    request.y = -120;
    request.width = 320;
    request.height = 200;
    request.requestedCapacity = 2;
    EventTextFntOpenResult80043438 result{};
    CHECK(ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status == EventTextFntOpenStatus80043438::Executed);
    CHECK(result.returnedSlot == 0);
    CHECK(result.oldCursor == 0);
    CHECK(result.effectiveCapacity == 2);
    CHECK(result.newCursor == 2);
    CHECK(result.initializedPacketCount == 2u);
    CHECK(state.recordCount8005CCDC == 1);
    CHECK(state.allocationCursor8005D6E4 == 2);

    const EventTextRecord800436F0& record = state.records[0];
    CHECK(record.x == -156);
    CHECK(record.y == -120);
    CHECK(record.w == 320);
    CHECK(record.h == 200);
    CHECK(record.measureMode2C == 0);
    CHECK(record.parseBudget1C == 2);
    CHECK(record.listHeadWord10 == 0x02000000u);
    CHECK(record.word14 == 0xE1002345u);
    CHECK(record.word18 == 0xE2000000u);
    CHECK(record.textPointer24 == kTextRecordTextBufferBase8008A750);
    CHECK(record.glyphPacketCursor20 == kTextRecordGlyphBufferBase8008AB50);
    CHECK(state.textBytes[0] == 0u);
    CHECK(state.textBytes[1] == 'Y');

    CHECK(state.glyphPackets[0].word0 == 0x035A5A5Au);
    CHECK(state.glyphPackets[0].r == 0x5Au);
    CHECK(state.glyphPackets[0].code == 0x74u);
    CHECK(state.glyphPackets[0].x == static_cast<int16_t>(0x5A5A));
    CHECK(state.glyphPackets[0].clut == 0xA55Au);
    CHECK(state.glyphPackets[1].word0 == 0x036B6B6Bu);
    CHECK(state.glyphPackets[1].code == 0x74u);
    CHECK(state.glyphPackets[1].clut == 0xA55Au);

    EventTextRuntimeState800436F0 oldGpuState = MakeLoadedState(0u);
    request.requestedCapacity = 0;
    CHECK(ExecuteFntOpenSoftware80043438(oldGpuState, request, result));
    CHECK(oldGpuState.records[0].word14 == 0xE1000145u);

    EventTextRuntimeState800436F0 preserved = MakeLoadedState();
    preserved.records[0].byte04 = 0x11u;
    preserved.records[0].byte05 = 0x22u;
    preserved.records[0].byte06 = 0x33u;
    preserved.records[0].control07 = 0x44u;
    preserved.records[0].listHeadWord10 = 0x77ABCDEFu;
    request.width = 0x10000;
    request.requestedCapacity = 0;
    CHECK(ExecuteFntOpenSoftware80043438(preserved, request, result));
    CHECK(preserved.records[0].w == 0);
    CHECK(preserved.records[0].measureMode2C == 0);
    CHECK(preserved.records[0].byte04 == 0x11u);
    CHECK(preserved.records[0].byte05 == 0x22u);
    CHECK(preserved.records[0].byte06 == 0x33u);
    CHECK(preserved.records[0].control07 == 0x44u);
    CHECK(preserved.records[0].listHeadWord10 == 0x02ABCDEFu);

    EventTextRuntimeState800436F0 exactZero = MakeLoadedState();
    request.width = 0;
    CHECK(ExecuteFntOpenSoftware80043438(exactZero, request, result));
    CHECK(exactZero.records[0].w == 0);
    CHECK(exactZero.records[0].measureMode2C == 1);
}

void TestFntOpenCapacityAndRecordLimits() {
    EventTextRuntimeState800436F0 state = MakeLoadedState();
    EventTextFntOpenRequest80043438 request{};
    request.width = 100;
    request.height = 20;
    request.requestedCapacity = 100;
    EventTextFntOpenResult80043438 result{};
    CHECK(ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.returnedSlot == 0);
    CHECK(state.allocationCursor8005D6E4 == 100);

    request.requestedCapacity = 1000;
    CHECK(ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.returnedSlot == 1);
    CHECK(result.oldCursor == 100);
    CHECK(result.effectiveCapacity == 924);
    CHECK(result.newCursor == 1024);
    CHECK(state.records[1].textPointer24 ==
          kTextRecordTextBufferBase8008A750 + 100u);
    CHECK(state.records[1].glyphPacketCursor20 ==
          kTextRecordGlyphBufferBase8008AB50 +
              100u * kTextGlyphPacketStride800436F0);

    state = MakeLoadedState();
    state.recordCount8005CCDC = 1;
    state.allocationCursor8005D6E4 = 500;
    state.allocationCursorKnown = true;
    state.textBytes[500] = 'Q';
    state.glyphPackets[500].word0 = 0xCAFEBABEu;
    request.requestedCapacity = 0;
    CHECK(ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.initializedPacketCount == 0u);
    CHECK(result.newCursor == 500);
    CHECK(state.recordCount8005CCDC == 2);
    CHECK(state.textBytes[500] == 0u);
    CHECK(state.glyphPackets[500].word0 == 0xCAFEBABEu);

    state.recordCount8005CCDC = 8;
    const EventTextRuntimeState800436F0 fullBefore = state;
    CHECK(ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status == EventTextFntOpenStatus80043438::RecordLimit);
    CHECK(result.returnedSlot == -1);
    CHECK(SameState(state, fullBefore));

    EventTextRuntimeState800436F0 limitOnly{};
    limitOnly.recordCountKnown = true;
    limitOnly.recordCount8005CCDC = 8;
    const EventTextRuntimeState800436F0 limitOnlyBefore = limitOnly;
    CHECK(ExecuteFntOpenSoftware80043438(limitOnly, request, result));
    CHECK(result.status == EventTextFntOpenStatus80043438::RecordLimit);
    CHECK(SameState(limitOnly, limitOnlyBefore));

    state = MakeLoadedState();
    state.recordCount8005CCDC = 1;
    state.allocationCursor8005D6E4 = 500;
    state.allocationCursorKnown = true;
    state.glyphBankKnown = false;
    state.textBytes[500] = 'Z';
    state.glyphPackets[500].word0 = 0x7A7A7A7Au;
    request.requestedCapacity = 0;
    CHECK(ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status == EventTextFntOpenStatus80043438::Executed);
    CHECK(result.initializedPacketCount == 0u);
    CHECK(state.textBytes[500] == 0u);
    CHECK(state.glyphPackets[500].word0 == 0x7A7A7A7Au);
    CHECK(!state.glyphBankKnown);

    state = MakeLoadedState();
    state.recordCount8005CCDC = 1;
    state.allocationCursor8005D6E4 = 500;
    state.allocationCursorKnown = true;
    state.glyphBankKnown = false;
    request.requestedCapacity = 1;
    const EventTextRuntimeState800436F0 unknownGlyphBefore = state;
    CHECK(!ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status == EventTextFntOpenStatus80043438::SourceUnknown);
    CHECK(SameState(state, unknownGlyphBefore));
}

void TestFntOpenHostLimitsAreAtomic() {
    EventTextRuntimeState800436F0 state = MakeLoadedState();
    state.recordCount8005CCDC = 1;
    state.allocationCursor8005D6E4 = 10;
    state.allocationCursorKnown = true;
    EventTextFntOpenRequest80043438 request{};
    request.width = 100;
    request.height = 20;
    request.requestedCapacity = -1;
    EventTextFntOpenResult80043438 result{};
    EventTextRuntimeState800436F0 before = state;
    CHECK(!ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status ==
          EventTextFntOpenStatus80043438::HostPoolBindingLimit);
    CHECK(SameState(state, before));

    request.requestedCapacity = 0;
    request.mode = 1;
    CHECK(!ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status ==
          EventTextFntOpenStatus80043438::HostUnsupportedNonzeroMode);
    CHECK(SameState(state, before));

    request.mode = 0;
    state.allocationCursor8005D6E4 = 1024;
    before = state;
    CHECK(!ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status ==
          EventTextFntOpenStatus80043438::HostPoolBindingLimit);
    CHECK(SameState(state, before));

    state = EventTextRuntimeState800436F0{};
    CHECK(!ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status == EventTextFntOpenStatus80043438::SourceUnknown);

    state = MakeLoadedState();
    state.recordCount8005CCDC = 1;
    state.allocationCursor8005D6E4 =
        std::numeric_limits<int32_t>::min();
    state.allocationCursorKnown = true;
    request.requestedCapacity =
        std::numeric_limits<int32_t>::min() + 1025;
    before = state;
    CHECK(!ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status ==
          EventTextFntOpenStatus80043438::HostPoolBindingLimit);
    CHECK(SameState(state, before));

    state = MakeLoadedState();
    state.recordCount8005CCDC = -1;
    before = state;
    request.requestedCapacity = 0;
    CHECK(!ExecuteFntOpenSoftware80043438(state, request, result));
    CHECK(result.status ==
          EventTextFntOpenStatus80043438::HostPoolBindingLimit);
    CHECK(SameState(state, before));
}

void TestSetDumpUsesAllocatorReturnDomain() {
    EventTextRuntimeState800436F0 state = MakeLoadedState();
    state.recordCount8005CCDC = 1;
    state.currentRecord8005CCE0 = 7;
    state.appendCallback8005D730 = 0x11111111u;
    EventTextSetDumpFntResult80043354 result{};

    EventTextRuntimeState800436F0 unknown{};
    const EventTextRuntimeState800436F0 unknownBefore = unknown;
    CHECK(ExecuteSetDumpFntSoftware80043354(unknown, -1, result));
    CHECK(result.accepted);
    CHECK(!result.sourceKnown);
    CHECK(!result.updated);
    CHECK(SameState(unknown, unknownBefore));
    CHECK(!ExecuteSetDumpFntSoftware80043354(unknown, 0, result));
    CHECK(!result.accepted);
    CHECK(!result.sourceKnown);
    CHECK(!result.updated);
    CHECK(SameState(unknown, unknownBefore));

    CHECK(ExecuteSetDumpFntSoftware80043354(state, 2, result));
    CHECK(result.accepted);
    CHECK(!result.updated);
    CHECK(state.currentRecord8005CCE0 == 7);

    CHECK(ExecuteSetDumpFntSoftware80043354(state, 1, result));
    CHECK(result.updated);
    CHECK(state.currentRecord8005CCE0 == 1);
    CHECK(state.appendCallback8005D730 == kFn80043A14);

    state.currentRecord8005CCE0 = 5;
    CHECK(ExecuteSetDumpFntSoftware80043354(state, -1, result));
    CHECK(!result.updated);
    CHECK(state.currentRecord8005CCE0 == 5);

    CHECK(ExecuteSetDumpFntSoftware80043354(state, 0, result));
    CHECK(result.updated);
    CHECK(state.currentRecord8005CCE0 == 0);

    state = MakeLoadedState();
    state.recordCount8005CCDC = -1;
    CHECK(ExecuteSetDumpFntSoftware80043354(state, 0, result));
    CHECK(result.sourceKnown);
    CHECK(!result.updated);
}

void TestTextSystemBootSoftwareCore() {
    EventTextRuntimeState800436F0 state{};
    std::memset(state.records.data(), 0x7D,
                sizeof(EventTextRecord800436F0) * state.records.size());
    std::memset(&state.glyphPackets[0], 0x5A,
                sizeof(state.glyphPackets[0]));
    std::memset(&state.glyphPackets[511], 0x6B,
                sizeof(state.glyphPackets[511]));
    std::memset(&state.glyphPackets[512], 0x7C,
                sizeof(state.glyphPackets[512]));
    state.textBytes[0] = 'A';
    state.textBytes[1] = 'B';
    state.recordCount8005CCDC = 7;
    state.currentRecord8005CCE0 = 7;
    state.allocationCursor8005D6E4 = 777;
    state.appendCallback8005D730 = 0x22222222u;
    state.textBankKnown = true;
    state.glyphBankKnown = true;

    DisplayCapture capture{};
    capture.state = &state;
    capture.returnValue = -19;
    EventTextBootResult80027FAC result{};
    CHECK(ExecuteTextSystemBoot80027FACSoftwareCore(
        state, MakeBinding(), CaptureDisplay, &capture, result));
    CHECK(result.status == EventTextBootStatus80027FAC::Executed);
    CHECK(result.accepted);
    CHECK(result.fontBindingInjected);
    CHECK(!result.fontUploadExecuted);
    CHECK(result.displaySinkInvoked);
    CHECK(!result.exactPsxHalParity);
    CHECK(!result.runtimeConsumerWired);
    CHECK(result.displaySinkReturn == -19);
    CHECK(result.returnedSlot == 0);
    CHECK(result.stepCount == 4u);
    CHECK(result.steps[0] == EventTextBootStep80027FAC::FntLoad80043394);
    CHECK(result.steps[1] == EventTextBootStep80027FAC::FntOpen80043438);
    CHECK(result.steps[2] == EventTextBootStep80027FAC::SetDumpFnt80043354);
    CHECK(result.steps[3] ==
          EventTextBootStep80027FAC::DisplayEnable80044AA0);
    CHECK(capture.calls == 1);
    CHECK(capture.enabled == 1);
    CHECK(capture.observedCount == 1);
    CHECK(capture.observedCurrent == 0);
    CHECK(capture.observedCursor == 512);
    CHECK(capture.observedCallback == kFn80043A14);

    CHECK(state.recordCount8005CCDC == 1);
    CHECK(state.globalsKnown);
    CHECK(state.recordCountKnown);
    CHECK(state.currentRecordKnown);
    CHECK(state.allocationCursorKnown);
    CHECK(state.appendCallbackKnown);
    CHECK(state.currentRecord8005CCE0 == 0);
    CHECK(state.allocationCursor8005D6E4 == 512);
    CHECK(state.records[0].x == -156);
    CHECK(state.records[0].y == -120);
    CHECK(state.records[0].w == 320);
    CHECK(state.records[0].h == 200);
    CHECK(state.records[0].parseBudget1C == 512);
    CHECK(state.records[0].textPointer24 ==
          kTextRecordTextBufferBase8008A750);
    CHECK(state.records[0].glyphPacketCursor20 ==
          kTextRecordGlyphBufferBase8008AB50);
    CHECK(state.textBytes[0] == 0u);
    CHECK(state.textBytes[1] == 'B');
    CHECK(state.glyphPackets[0].word0 == 0x035A5A5Au);
    CHECK(state.glyphPackets[0].code == 0x74u);
    CHECK(state.glyphPackets[0].clut == 0xA55Au);
    CHECK(state.glyphPackets[511].word0 == 0x036B6B6Bu);
    CHECK(state.glyphPackets[511].code == 0x74u);
    CHECK(state.glyphPackets[511].clut == 0xA55Au);
    CHECK(state.glyphPackets[512].word0 == 0x7C7C7C7Cu);

    EventTextRuntimeState800436F0 before = state;
    CHECK(!ExecuteTextSystemBoot80027FACSoftwareCore(
        state,
        EventTextFontBinding80043394{},
        CaptureDisplay,
        &capture,
        result));
    CHECK(result.status == EventTextBootStatus80027FAC::FontBindingUnknown);
    CHECK(SameState(state, before));
    CHECK(!ExecuteTextSystemBoot80027FACSoftwareCore(
        state, MakeBinding(), nullptr, nullptr, result));
    CHECK(result.status == EventTextBootStatus80027FAC::DisplaySinkMissing);
    CHECK(SameState(state, before));

    EventTextRuntimeState800436F0 unknownPools{};
    const EventTextRuntimeState800436F0 unknownPoolsBefore = unknownPools;
    CHECK(!ExecuteTextSystemBoot80027FACSoftwareCore(
        unknownPools, MakeBinding(), CaptureDisplay, &capture, result));
    CHECK(result.status ==
          EventTextBootStatus80027FAC::HostPoolBackingUnknown);
    CHECK(SameState(unknownPools, unknownPoolsBefore));
}

} // namespace

int main() {
    CHECK(!RuntimeCutoverAllowed());
    CHECK(sizeof(EventTextRecord800436F0) == 0x30u);
    CHECK(sizeof(EventTextGlyphPacket800436F0) == 0x10u);
    TestFntLoadClearsOnlyFormalRecordState();
    TestFntLoadBindingMatchesCurrentIdaGetTPageGetClut();
    TestEmbeddedFontDecodeUsesFormal4bppLayout();
    TestEmbeddedFontDecodeReadsOriginalExecutableSource();
    TestTextAppendSoftwareCoreFormatsBoundHostArguments();
    TestTextAppendSoftwareCoreRejectsUnboundInputsAtomically();
    TestMenuHelpSoftwareCoreExecutesFormalAppendAndFlushOrder();
    TestMenuHelpSoftwareCoreRejectsUnknownSourcesAtomically();
    TestStageClearSoftwareCoreExecutesFormalAppendAndFlushOrder();
    TestStageClearSoftwareCoreRejectsUnknownGateAndSinkAtomically();
    TestLoadImageSoftwareCoreCapturesFormalDispatch();
    TestLoadImageSoftwareCoreRejectsUnboundInputsAtomically();
    TestTextDispatchSoftwareCapturesFormalQueueShape();
    TestTextDispatchSoftwareRejectsUnboundQueuePayloadAtomically();
    TestTextDispatchQueueSoftwareCapturesProducerConsumerOrder();
    TestGpuUploadWaitSoftwareCapturesTimeoutGuard();
    TestLoadImageUploadSoftwareCapturesFormalCpuAndDmaSplit();
    TestLoadImageUploadSoftwareRejectsShortSourceAtomically();
    TestFntOpenFirstAllocationAndPacketWrites();
    TestFntOpenCapacityAndRecordLimits();
    TestFntOpenHostLimitsAreAtomic();
    TestSetDumpUsesAllocatorReturnDomain();
    TestTextSystemBootSoftwareCore();

    if (g_failed != 0) {
        std::printf("test_ss0_event_text_boot: %d failed checks\n", g_failed);
        return 1;
    }
    std::printf("test_ss0_event_text_boot: ok\n");
    return 0;
}
