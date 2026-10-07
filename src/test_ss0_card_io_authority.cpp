#include "pr/pr_stage1_save_card_hal_direct.h"
#include "pr/pr_stage1_save_ui_direct.h"
#include "pr/pr_game_context.h"
#include "pr/pr_ss0_card_image_storage_direct.h"
#include "pr/pr_pad.h"
#include "pr/pr_ss0_menu_context_direct.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace PrStage1SaveCardHalDirect;

PrPadState PrPad::GetState(int) {
    return PrPadState{};
}

namespace {

int g_failed = 0;
bool g_pauseAfterStorageFixture = false;

constexpr std::size_t kCardImageBytes8007A318 = 128u * 1024u;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

std::filesystem::path CreateFreshCardTestDirectory() {
    std::error_code error;
    const auto temporary = std::filesystem::temp_directory_path(error);
    if (error || !temporary.is_absolute()) return {};
    const auto prefix = L"parappawin-card-io-" + std::to_wstring(GetCurrentProcessId()) +
                        L"-" + std::to_wstring(GetTickCount64()) + L"-";
    for (unsigned attempt = 0; attempt < 128; ++attempt) {
        const auto candidate = temporary / (prefix + std::to_wstring(attempt));
        // Atomic create, never adopt a pre-existing directory or link.
        if (std::filesystem::create_directory(candidate, error)) {
            std::printf("card-I/O isolated directory (retained): %s\n", candidate.u8string().c_str());
            return candidate;
        }
        if (error) return {};
    }
    return {};
}

std::vector<uint8_t> ReadWholeFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>());
}

bool WriteWholeFile(const std::filesystem::path& path,
                    const std::vector<uint8_t>& bytes) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return false;
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file.write(reinterpret_cast<const char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
    file.flush();
    return static_cast<bool>(file);
}

std::vector<uint8_t> MakeCardImageWithOneEntry(const char* internalName) {
    std::vector<uint8_t> image(kCardImageBytes8007A318, 0u);
    image[0] = static_cast<uint8_t>('M');
    image[1] = static_cast<uint8_t>('C');
    auto checksum = [](uint8_t* frame) {
        uint8_t value = 0;
        for (std::size_t i = 0; i < 0x7Fu; ++i) value ^= frame[i];
        frame[0x7Fu] = value;
    };
    checksum(image.data());
    uint8_t* entry = image.data() + 128u;
    entry[0] = 0x51u;
    entry[4] = 0x00u;
    entry[5] = 0x20u;
    if (internalName != nullptr) {
        for (std::size_t i = 0; i + 1u < 20u && internalName[i]; ++i)
            entry[0x0Au + i] = static_cast<uint8_t>(internalName[i]);
    }
    checksum(entry);
    for (int frame = 2; frame <= 15; ++frame)
        checksum(image.data() + static_cast<std::size_t>(frame) * 128u);
    return image;
}

void TestHiScoreCase6SnapshotSurvivesDirectoryChange() {
    std::array<uint8_t, 600> rawBytes{};
    constexpr char keep[] = "BASCUS-94183KEEP";
    constexpr char foreign[] = "OTHER-GAME";
    std::memcpy(rawBytes.data(), foreign, sizeof(foreign));
    std::memcpy(rawBytes.data() + 14 * 40, keep, sizeof(keep));
    PrStage1SaveUiDirectoryRawBankView8007A318 raw{};
    raw.known = true;
    raw.bytes = rawBytes.data();
    raw.byteCount = rawBytes.size();
    HiScoreCase6Directory80019D7C directory{};
    CHECK(BuildHiScoreCase6Directory80019D7C(raw, &directory));
    CHECK(directory.known && directory.entryCount == 1);
    CHECK(std::strcmp(directory.names[0].data(), keep) == 0);
    rawBytes.fill(0); // Case17 must not borrow the raw directory owner.

    auto image = MakeCardImageWithOneEntry(keep);
    std::fill(image.begin() + 8192, image.begin() + 16384, 0x31u);
    PrStage1SaveUiCardImagePersistenceView8007A318 view{};
    view.known = view.slotPolicyKnown = true;
    view.bytes = image.data();
    view.byteCount = image.size();
    Case17CardReadTypedCarrier800179B4 carrier{};
    auto read = [&]() { return PublishRuntimeCase17FromCase6Directory80019D7C(view, directory); };
    CHECK(read());
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.feedback.word8007ABE4 == 1 && carrier.blockStorage[0][512] == 0x31u);
    // Original filename moves; a newly added matching save must not join Case17.
    std::copy_n(image.begin() + 128, 128, image.begin() + 15 * 128);
    std::fill(image.begin() + 15 * 8192, image.end(), 0x72u);
    auto added = MakeCardImageWithOneEntry("BASCUS-94183NEW");
    std::copy_n(added.begin() + 128, 128, image.begin() + 128);
    CHECK(read());
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.feedback.word8007ABE4 == 1 && carrier.blockStorage[0][512] == 0x72u);
    CHECK(!carrier.feedback.attempts[1].rowEnabled);
    // Disappearance is a produced platform failure, not a fabricated success
    // and not a fresh empty directory. No PSX polling event is invented.
    std::fill_n(image.begin() + 15 * 128, 128, 0u);
    CHECK(read());
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.case17LoopCompletionKnown80019D7C && !carrier.incomplete);
    CHECK(carrier.feedback.word8007ABE4 == 1 && carrier.feedback.attempts[0].rowEnabled);
    CHECK(carrier.hal.attempts[0].produced && carrier.hal.attempts[0].psxReturn800179B4 == -1);
    CHECK(!carrier.hal.attempts[0].readSucceeded && !carrier.payloadBytesKnown8007ADE8);
    CHECK(!carrier.feedback.attempts[0].poll.called && !carrier.feedback.attempts[0].closeKnown800179B4);
    CHECK(!carrier.feedback.attempts[0].blockBytesKnown);
    raw.byteCount = 599;
    CHECK(!BuildHiScoreCase6Directory80019D7C(raw, &directory));
    CHECK(!directory.known && directory.entryCount == 0);
    CHECK(!read());
    CHECK(!GetCase17CardReadTypedCarrier800179B4(&carrier));
}

void TestNamedCardFileReadOwnsReadOnlyHandle() {
    namespace Storage = PrSS0CardImageStorageDirect;
    const auto save = CreateFreshCardTestDirectory();
    CHECK(!save.empty());
    if (save.empty()) return;
    const auto path = save / L"bu00.mcr";
    auto image = MakeCardImageWithOneEntry("BASCUS-94183KEEP");
    std::fill(image.begin() + 8192, image.begin() + 16384, 0x31u);
    CHECK(WriteWholeFile(path, image));
    std::array<uint8_t, 8192> bytes{};
    auto read = [&](const char* name) {
        return Storage::ReadCardFileBlockAtDirectory800173A8(save, name, bytes.data(), bytes.size());
    };
    auto result = read("BASCUS-94183KEEP");
    CHECK(result.requestValid && result.imageOpened && result.directoryValid && result.fileFound);
    CHECK(result.blockIndex == 0 && result.readComplete && result.bytesRead == bytes.size());
    CHECK(result.closeAttempted && result.closeSucceeded && bytes[512] == 0x31u);
    CHECK(ReadWholeFile(path) == image);
    HANDLE exclusive = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    CHECK(exclusive != INVALID_HANDLE_VALUE); // The read must have released its actual host handle.
    if (exclusive != INVALID_HANDLE_VALUE) CloseHandle(exclusive);
    result = read("BASCUS-94183MISSING");
    CHECK(result.imageOpened && result.closeAttempted && result.closeSucceeded);
    CHECK(!result.fileFound && !result.readComplete && bytes[512] == 0);
    result = read("../bu00.mcr");
    CHECK(!result.requestValid && !result.imageOpened);
    CHECK(!Storage::ReadCardFileBlockAtDirectory800173A8({}, "KEEP", bytes.data(), bytes.size()).requestValid);
    CHECK(!Storage::ReadCardFileBlockAtDirectory800173A8(L"save", "KEEP", bytes.data(), bytes.size()).requestValid);
    std::copy_n(image.begin() + 128, 128, image.begin() + 15 * 128);
    std::fill_n(image.begin() + 128, 128, 0);
    std::fill(image.begin() + 15 * 8192, image.end(), 0x72u);
    CHECK(WriteWholeFile(path, image));
    result = read("BASCUS-94183KEEP");
    CHECK(result.fileFound && result.blockIndex == 14 && result.readComplete && bytes[512] == 0x72u);
    image.resize(1024); // Explicitly corrupt only the fresh disposable test image.
    CHECK(WriteWholeFile(path, image));
    result = read("BASCUS-94183KEEP");
    CHECK(result.imageOpened && !result.directoryValid && !result.readComplete);
    CHECK(result.closeAttempted && result.closeSucceeded && bytes[512] == 0);
    CHECK(ReadWholeFile(path) == image);
}

void TestCase17OpensEachCapturedNameAndOwnsAllReadBuffers() {
    namespace Storage = PrSS0CardImageStorageDirect;
    struct ReaderContext { std::filesystem::path save; std::vector<uint8_t> afterFirst; int calls = 0; };
    ReaderContext context{CreateFreshCardTestDirectory(), {}, 0};
    CHECK(!context.save.empty());
    if (context.save.empty()) return;
    auto image = MakeCardImageWithOneEntry("BASCUS-94183FIRST");
    auto second = MakeCardImageWithOneEntry("BASCUS-94183SECOND");
    std::copy_n(second.begin() + 128, 128, image.begin() + 2 * 128);
    std::fill(image.begin() + 8192, image.begin() + 16384, 0x11u);
    std::fill(image.begin() + 16384, image.begin() + 24576, 0x22u);
    context.afterFirst = image;
    std::copy_n(image.begin() + 2 * 128, 128, context.afterFirst.begin() + 15 * 128);
    std::fill_n(context.afterFirst.begin() + 2 * 128, 128, 0);
    std::fill(context.afterFirst.begin() + 8192, context.afterFirst.begin() + 16384, 0x90u);
    std::fill(context.afterFirst.begin() + 15 * 8192, context.afterFirst.end(), 0x72u);
    CHECK(WriteWholeFile(context.save / L"bu00.mcr", image));
    HiScoreCase6Directory80019D7C directory{};
    directory.known = true;
    directory.entryCount = 2;
    std::memcpy(directory.names[0].data(), "BASCUS-94183FIRST", sizeof("BASCUS-94183FIRST"));
    std::memcpy(directory.names[1].data(), "BASCUS-94183SECOND", sizeof("BASCUS-94183SECOND"));
    PrStage1SaveUiCardImagePersistenceView8007A318 view{};
    view.known = view.slotPolicyKnown = true;
    view.bytes = image.data();
    view.byteCount = image.size();
    auto reader = [](const char* name, uint8_t* output, std::size_t size, void* owner) {
        auto& ctx = *static_cast<ReaderContext*>(owner);
        const auto result = Storage::ReadCardFileBlockAtDirectory800173A8(ctx.save, name, output, size);
        if (++ctx.calls == 1) CHECK(WriteWholeFile(ctx.save / L"bu00.mcr", ctx.afterFirst));
        return result;
    };
    CHECK(PublishRuntimeCase17FromCase6Directory80019D7C(view, directory, reader, &context));
    CHECK(context.calls == 2);
    Case17CardReadTypedCarrier800179B4 carrier{};
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.blockStorage[0][512] == 0x11u && carrier.blockStorage[1][512] == 0x72u);
    // Neither overwritten source memory nor later physical/file content may
    // replace the first read's already-owned result.
    std::fill(image.begin(), image.end(), 0xE1u);
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.blockStorage[0][512] == 0x11u && carrier.blockStorage[1][512] == 0x72u);
    CHECK(!carrier.feedback.attempts[0].poll.called); // No fabricated PSX event receipts.
    auto closeFailedReader = [](const char*, uint8_t* output, std::size_t size, void*) {
        Storage::NamedCardBlockRead800173A8 result{};
        result.requestValid = result.imageOpened = result.directoryValid = true;
        result.fileFound = result.readComplete = result.closeAttempted = true;
        result.closeSucceeded = false;
        result.blockIndex = 0;
        result.bytesRead = size;
        std::memset(output, 0x33u, size);
        return result;
    };
    CHECK(PublishRuntimeCase17FromCase6Directory80019D7C(view, directory, closeFailedReader));
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.hal.attempts[0].readSucceeded && carrier.blockStorage[0][512] == 0x33u);
    ClearCase17CardReadTypedCarrier800179B4();
}

void TestDeferredNamedReadHandleAndNativePollOrder() {
    namespace Storage = PrSS0CardImageStorageDirect;
    const auto save = CreateFreshCardTestDirectory();
    CHECK(!save.empty());
    if (save.empty()) return;
    const auto path = save / L"bu00.mcr";
    auto image = MakeCardImageWithOneEntry("BASCUS-94183KEEP");
    std::fill(image.begin() + 8192, image.begin() + 16384, 0x31u);
    CHECK(WriteWholeFile(path, image));
    std::array<uint8_t, 8192> bytes{};
    auto pending = Storage::BeginCardFileBlockAtDirectory800173A8(
        save, "BASCUS-94183KEEP", bytes.data(), bytes.size());
    CHECK(pending.handle && pending.receipt.readComplete && !pending.receipt.closeAttempted);
    HANDLE exclusive = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    CHECK(exclusive == INVALID_HANDLE_VALUE); // An actual open handle, not a receipt bit.
    if (exclusive != INVALID_HANDLE_VALUE) CloseHandle(exclusive);
    Storage::CloseNamedCardFileAfterPoll800179B4(pending);
    CHECK(!pending.handle && pending.receipt.closeAttempted && pending.receipt.closeSucceeded);
    CHECK(pending.receipt.readComplete && bytes[512] == 0x31u);
    exclusive = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    CHECK(exclusive != INVALID_HANDLE_VALUE);
    if (exclusive != INVALID_HANDLE_VALUE) CloseHandle(exclusive);

    ResetTranslatedCardEventBroker800170C4();
    CHECK(SignalTranslatedSwCardEvent80016E18(CardTranslatedEventSignalSource::FileRead800173A8, 1));
    CHECK(SignalTranslatedSwCardEvent80016E18(CardTranslatedEventSignalSource::CardInfo80017594, 4));
    int32_t poll = -1;
    CHECK(PollTranslatedReadEvents80016EB8(&poll) && poll == 1);
    CHECK(GetTranslatedCardEventBrokerState800170C4().swPending[3]);
    CHECK(PollTranslatedReadEvents80016EB8(&poll) && poll == 4);
    CHECK(PollTranslatedReadEvents80016EB8(&poll) && poll == 0);

    auto reader = [](const char* name, uint8_t* output, std::size_t size, void* owner) {
        return Storage::BeginCardFileBlockAtDirectory800173A8(
            *static_cast<const std::filesystem::path*>(owner), name, output, size);
    };
    HiScoreCase6Directory80019D7C directory{};
    directory.known = true;
    directory.entryCount = 2;
    std::strcpy(directory.names[0].data(), "BASCUS-94183MISSING");
    std::strcpy(directory.names[1].data(), "BASCUS-94183KEEP");
    auto e = std::make_unique<HiScoreNamedReadExecution800179B4>();
    auto mutableSave = save;
    CHECK(BeginHiScoreNamedReads800179B4(*e, directory, reader, &mutableSave));
    CHECK(!e->complete && e->waitPending && e->row == 0 && e->rowWaits == 0);
    CHECK(!e->pendingRead.handle && e->pendingRead.receipt.closeAttempted);
    Case17CardReadTypedCarrier800179B4 carrier{};
    CHECK(!GetCase17CardReadTypedCarrier800179B4(&carrier));
    for (int i = 1; i < 300; ++i) {
        CHECK(ResumeHiScoreNamedReadsAfterVSync80016EB8(*e));
        CHECK(!e->complete && e->waitPending && e->rowWaits == i && e->row == 0);
    }
    // Event arrives during the final VSync: no301st poll. The next successful
    // open will drain it, so the missing file MUST still be classified failed.
    CHECK(SignalTranslatedSwCardEvent80016E18(CardTranslatedEventSignalSource::PhysicalHotplug80017594, 1));
    CHECK(ResumeHiScoreNamedReadsAfterVSync80016EB8(*e));
    CHECK(e->complete && !e->failed && !e->waitPending && e->totalWaits == 300 && e->row == 2);
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.hal.attempts[0].psxReturn800179B4 == -1 && !carrier.hal.attempts[0].readSucceeded);
    CHECK(carrier.hal.attempts[1].readSucceeded && carrier.blockStorage[1][512] == 0x31u);
    CHECK(!e->pendingRead.handle && e->pendingRead.receipt.closeAttempted);
    e.reset(); // published carrier must own its buffers independently
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier) && carrier.blockStorage[1][512] == 0x31u);

    directory.entryCount = 1;
    ResetTranslatedCardEventBroker800170C4();
    CHECK(SignalTranslatedSwCardEvent80016E18(CardTranslatedEventSignalSource::PhysicalHotplug80017594, 1));
    e = std::make_unique<HiScoreNamedReadExecution800179B4>();
    CHECK(BeginHiScoreNamedReads800179B4(*e, directory, reader, &mutableSave));
    // Original179B4 ignores173A8's return; a preexisting event1 still returns0
    // and hands the cleared block to the bank. Do not invent an open-result gate.
    CHECK(e->complete && e->totalWaits == 0 && e->lastPollResult == 1);
    CHECK(GetCase17CardReadTypedCarrier800179B4(&carrier) && carrier.hal.attempts[0].readSucceeded);
    CHECK(carrier.blockStorage[0][512] == 0);

    struct RowWitness {
        std::filesystem::path path;
        int opens = 0;
        std::vector<int32_t> rows;
    } witness{save};
    auto orderedReader = [](const char* name, uint8_t* output, size_t size, void* owner) {
        auto& w = *static_cast<RowWitness*>(owner);
        CHECK(static_cast<int>(w.rows.size()) == w.opens);
        ++w.opens;
        return Storage::BeginCardFileBlockAtDirectory800173A8(w.path, name, output, size);
    };
    auto consume = [](int32_t row, bool enabled, int32_t poll,
                      const uint8_t* block, size_t size, void* owner) {
        auto& w = *static_cast<RowWitness*>(owner);
        CHECK(row == static_cast<int32_t>(w.rows.size()));
        CHECK(size == 8192u && block != nullptr);
        CHECK(enabled == (row < 2));
        CHECK(poll == (row == 0 ? 1 : row == 1 ? 2 : 0));
        // Closing occurs before consumption, even when the next open waits.
        HANDLE handle = CreateFileW((w.path / L"bu00.mcr").c_str(), GENERIC_READ,
            0, nullptr, OPEN_EXISTING, 0, nullptr);
        CHECK(handle != INVALID_HANDLE_VALUE);
        if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
        w.rows.push_back(row);
        return true;
    };
    directory.entryCount = 2;
    std::strcpy(directory.names[0].data(), "BASCUS-94183KEEP");
    std::strcpy(directory.names[1].data(), "BASCUS-94183MISSING");
    ResetTranslatedCardEventBroker800170C4();
    e = std::make_unique<HiScoreNamedReadExecution800179B4>();
    CHECK(BeginHiScoreNamedReads800179B4(*e, directory, orderedReader, &witness, consume, &witness));
    CHECK(e->waitPending && !e->complete && witness.opens == 2);
    CHECK(witness.rows.size() == 1u && witness.rows[0] == 0); // Already consumed during wait.
    for (int i = 0; i < 300; ++i) CHECK(ResumeHiScoreNamedReadsAfterVSync80016EB8(*e));
    CHECK(e->complete && witness.rows.size() == 15u);
    CHECK(ReadWholeFile(path) == image);
    ClearCase17CardReadTypedCarrier800179B4();
}

void TestState16ReadsCurrentFileByNameNotCachedPhysicalBlock() {
    auto image = MakeCardImageWithOneEntry("BASCUS-94183KEEP");
    std::fill(image.begin() + 8192u, image.begin() + 16384u, 0x31u);
    PrStage1SaveUiCardImagePersistenceView8007A318 view{};
    view.known = view.slotPolicyKnown = true;
    view.blockIndex = 0;
    view.bytes = image.data();
    view.byteCount = image.size();
    NamedCardReadLocation800173A8 location{};
    State16CardReadTypedCarrier800179B4 carrier{};
    constexpr char name[] = "BASCUS-94183KEEP";
    auto publish = [&]() {
        return PublishRuntimeState16CardReadByName800173A8(
            view, name, sizeof(name), 0, &location);
    };
    CHECK(publish());
    CHECK(location.known && location.requestSlot == 0 && location.sourcePhysicalBlock == 0);
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.blockStorage[0][512] == 0x31u);

    // Same filename moves to the last block; the old location now contains
    // another valid file. Keep request slot 0 but read the current named file.
    std::copy_n(image.begin() + 128u, 128u, image.begin() + 15u * 128u);
    std::fill(image.begin() + 15u * 8192u, image.end(), 0x72u);
    const auto other = MakeCardImageWithOneEntry("BASCUS-94183OTHER");
    std::copy_n(other.begin() + 128u, 128u, image.begin() + 128u);
    CHECK(publish());
    CHECK(location.known && location.requestSlot == 0 && location.sourcePhysicalBlock == 14);
    CHECK(GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(carrier.selectedBlockKnown && carrier.selectedBlockIndex == 0);
    CHECK(carrier.source == CardReadTypedCarrierSource800179B4::RuntimeLowerCardProducer);
    CHECK(carrier.typedReadSuccessKnown800179B4 && carrier.payloadBytesKnown8007ADE8);
    CHECK(carrier.blockStorage[0][512] == 0x72u);
    CHECK(std::equal(carrier.blockStorage[0].begin(), carrier.blockStorage[0].end(),
                     image.begin() + 15u * 8192u));

    // The filename disappears, though its old physical frame still exists.
    // Failure must revoke the previous typed payload rather than reuse it.
    std::fill_n(image.begin() + 15u * 128u, 128u, 0u);
    CHECK(!publish());
    CHECK(!location.known && location.sourcePhysicalBlock == -1);
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));

    std::copy_n(other.begin() + 128u, 128u, image.begin() + 15u * 128u);
    const char fullName[] = "12345678901234567890";
    std::copy_n(fullName, 20u, image.begin() + 15u * 128u + 0x0Au);
    CHECK(PublishRuntimeState16CardReadByName800173A8(view, fullName, sizeof(fullName), 0, &location));
    CHECK(location.sourcePhysicalBlock == 14);
    CHECK(!PublishRuntimeState16CardReadByName800173A8(view, fullName, 20u, 0, &location));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
    CHECK(!PublishRuntimeState16CardReadByName800173A8(view, nullptr, 0, 0, &location));
    CHECK(!PublishRuntimeState16CardReadByName800173A8(view, "", 1, 0, &location));
    CHECK(!PublishRuntimeState16CardReadByName800173A8(view, fullName, sizeof(fullName), -1, &location));
    CHECK(!PublishRuntimeState16CardReadByName800173A8(view, fullName, sizeof(fullName), 15, &location));
    CHECK(!PublishRuntimeState16CardReadByName800173A8(view, fullName, sizeof(fullName), 0, nullptr));
    view.known = false;
    CHECK(!PublishRuntimeState16CardReadByName800173A8(view, fullName, sizeof(fullName), 0, &location));
    view.known = true;
    CHECK(!PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(view, 14));
    CHECK(!GetState16CardReadTypedCarrier800179B4(&carrier));
}

void TestCardLoadRereadsReplacementMediumAndDropsStaleDirectory() {
    const auto oldDir = CreateFreshCardTestDirectory();
    const auto newDir = CreateFreshCardTestDirectory();
    CHECK(!oldDir.empty() && !newDir.empty());
    if (oldDir.empty() || newDir.empty()) return;
    const auto oldPath = oldDir / L"bu00.mcr";
    const auto newPath = newDir / L"bu00.mcr";
    const auto oldImage = MakeCardImageWithOneEntry("BASCUS-94183OLD");
    const auto newImage = MakeCardImageWithOneEntry("BASCUS-94183NEW");
    CHECK(WriteWholeFile(oldPath, oldImage));
    CHECK(WriteWholeFile(newPath, newImage));

    PrStage1SaveUiDirect::Reset19148();
    auto oldReload =
        PrSS0CardImageStorageDirect::ReloadCardImageAtDirectory8007A318(oldDir);
    CHECK(oldReload.imageFileFound && oldReload.imageValidated && oldReload.sinkImported);
    const auto oldLoad = PrStage1SaveUiDirect::LoadSaveUiDirectCardImageDirectory80017594();
    CHECK(oldLoad.directoryLoaded && oldLoad.activeRows == 1);
    CHECK(PrStage1SaveUiDirect::ScanSaveUiDirectoryRawBankName80017900(
              "BASCUS-94183OLD").found);
    CHECK(!PrStage1SaveUiDirect::ScanSaveUiDirectoryRawBankName80017900(
              "BASCUS-94183NEW").found);

    const auto payloadBefore = PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    auto newReload =
        PrSS0CardImageStorageDirect::ReloadCardImageAtDirectory8007A318(newDir);
    CHECK(newReload.imageFileFound && newReload.imageValidated && newReload.sinkImported);
    const auto newLoad = PrStage1SaveUiDirect::LoadSaveUiDirectCardImageDirectory80017594();
    CHECK(newLoad.directoryLoaded && newLoad.activeRows == 1);
    CHECK(!PrStage1SaveUiDirect::ScanSaveUiDirectoryRawBankName80017900(
              "BASCUS-94183OLD").found);
    CHECK(PrStage1SaveUiDirect::ScanSaveUiDirectoryRawBankName80017900(
              "BASCUS-94183NEW").found);
    const auto payloadAfter = PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
    CHECK(payloadBefore.payloadKnown == payloadAfter.payloadKnown);
    CHECK(payloadBefore.statusBankKnown80092F1D == payloadAfter.statusBankKnown80092F1D);

    const auto missingDir = CreateFreshCardTestDirectory();
    CHECK(!missingDir.empty());
    auto missingReload =
        PrSS0CardImageStorageDirect::ReloadCardImageAtDirectory8007A318(missingDir);
    CHECK(!missingReload.imageFileFound && !missingReload.sinkImported);
    const auto missingLoad = PrStage1SaveUiDirect::LoadSaveUiDirectCardImageDirectory80017594();
    CHECK(!missingLoad.directoryLoaded);
    CHECK(!PrStage1SaveUiDirect::ScanSaveUiDirectoryRawBankName80017900(
              "BASCUS-94183NEW").found);
    CHECK(ReadWholeFile(oldPath) == oldImage);
    CHECK(ReadWholeFile(newPath) == newImage);
}

PrStage1SaveUi19148LowerFeedbackRequest MakeCardIoRequest(int32_t state) {
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594;
    request.psxFunction = 0x80017594u;
    request.cardIoState.dword800917E8 = state;
    request.cardIoState.gp700 = 300;
    return request;
}

void CheckNoTypedSwPollAuthority(const CardIoHostFacts80017594& facts) {
    CHECK(!facts.pollSwKnown80016E18);
    CHECK(facts.pollSwResult80016E18 == 0);
    CHECK(!facts.pollSwGp700BeforeKnown80016E18);
    CHECK(!facts.pollSwGp700AfterKnown80016E18);
    CHECK(!facts.pollSwTimedOutKnown80016E18);
    CHECK(!facts.pollSwTimedOut80016E18);
}

void TestRejectsNonCardIoRequest() {
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10;
    request.psxFunction = 0x80017594u;
    CardIoHostFacts80017594 facts{};

    CHECK(!BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(!facts.factsKnown);
    CHECK(!facts.stateBeforeKnown);
    CHECK(!facts.stateAfterKnown);
    CheckNoTypedSwPollAuthority(facts);
}

void TestState0CarriesInfoSubmitShapeOnly() {
    CardIoHostFacts80017594 facts{};
    CHECK(BuildSaveUiCardIoObservedNormalPathFacts80017594(
        MakeCardIoRequest(0), &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(facts.stateAfterKnown);
    CHECK(facts.cardInfoKnown);
    CHECK(facts.cardInfoArgKnown);
    CHECK(facts.cardInfoArg == 0);
    CHECK(facts.stateAfter.dword800917E8 == 1);
    CHECK(facts.stateAfter.dword800917EC == 0);
    CHECK(facts.stateAfter.gp700 == 300);
    CheckNoTypedSwPollAuthority(facts);
}

void TestState1RequiresExplicitTypedPollFeedback() {
    CardIoHostFacts80017594 facts{};
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(1);
    request.cardIoState.dword800917F4 = 0;
    request.cardIoState.gp700 = 300;

    CHECK(!BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(!facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(!facts.stateAfterKnown);
    CHECK(!facts.cardInfoKnown);
    CHECK(!facts.cardLoadKnown);
    CheckNoTypedSwPollAuthority(facts);
}

void TestExplicitState1TypedPollSuccessFeedback() {
    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 1;
    facts.stateBefore.gp700 = 300;
    facts.stateAfterKnown = true;
    facts.stateAfter = facts.stateBefore;
    facts.stateAfter.dword800917E8 = 2;
    facts.stateAfter.dword800917F0 = 1;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 1;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    CardIoLowerFeedbackBuildResult80017594 build{};
    BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &build);

    CHECK(build.lowerFeedbackKnown);
    CHECK(build.lowerFeedback.cardIoFeedbackKnown80017594);
    const PrStage1SaveUiCardIoFeedback80017594& feedback =
        build.lowerFeedback.cardIoFeedback80017594;
    CHECK(feedback.stateBeforeKnown);
    CHECK(feedback.stateAfterKnown);
    CHECK(feedback.stateBefore.dword800917E8 == 1);
    CHECK(feedback.stateAfter.dword800917E8 == 2);
    CHECK(feedback.stateAfter.dword800917F0 == 1);
    CHECK(feedback.stateAfter.gp700 == 299);
    CHECK(feedback.pollSwKnown80016E18);
    CHECK(feedback.pollSwResult80016E18 == 1);
    CHECK(feedback.pollSwGp700BeforeKnown80016E18);
    CHECK(feedback.pollSwGp700Before80016E18 == 300);
    CHECK(feedback.pollSwGp700AfterKnown80016E18);
    CHECK(feedback.pollSwGp700After80016E18 == 299);
    CHECK(feedback.pollSwTimedOutKnown80016E18);
    CHECK(!feedback.pollSwTimedOut80016E18);
}

void TestCurrentIdaPollResultFacts80016E18() {
    CardIoHostFacts80017594 facts{};

    PrStage1SaveUi19148LowerFeedbackRequest state1 = MakeCardIoRequest(1);
    state1.cardIoState.gp700 = 300;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 1, &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.pollSwKnown80016E18);
    CHECK(facts.pollSwResult80016E18 == 1);
    CHECK(facts.pollSwGp700Before80016E18 == 300);
    CHECK(facts.pollSwGp700After80016E18 == 299);
    CHECK(!facts.pollSwTimedOut80016E18);
    CHECK(facts.stateAfter.dword800917E8 == 2);
    CHECK(facts.stateAfter.dword800917F0 == 1);
    CHECK(facts.stateAfter.gp700 == 299);

    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 2, &facts));
    CHECK(facts.pollSwResult80016E18 == 2);
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == -3);
    CHECK(facts.stateAfter.dword800917F4 == 0);

    state1.cardIoState.dword800917F4 = 1;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 1, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 1);
    CHECK(facts.stateAfter.dword800917F4 == 1);
    state1.cardIoState.dword800917F4 = 0;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 3, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 3);
    CHECK(facts.stateAfter.dword800917F4 == 0);
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 4, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 2);
    CHECK(facts.stateAfter.dword800917F0 == 4);
    CHECK(facts.stateAfter.dword800917F4 == 0);

    PrStage1SaveUi19148LowerFeedbackRequest state3 = MakeCardIoRequest(3);
    state3.cardIoState.dword800917F0 = 1;
    state3.cardIoState.gp700 = 300;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state3, 1, &facts));
    CHECK(facts.pollSwResult80016E18 == 1);
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 1);
    CHECK(facts.stateAfter.dword800917F4 == 1);
    CHECK(facts.stateAfter.gp700 == 299);
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state3, 3, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 3);
    CHECK(facts.stateAfter.dword800917F4 == 0);
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state3, 4, &facts));
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 5);
    CHECK(facts.stateAfter.dword800917F4 == 0);

    state3.cardIoState.gp700 = 0;
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(state3, 1, &facts));
    CHECK(facts.pollSwResult80016E18 == 2);
    CHECK(facts.pollSwTimedOut80016E18);
    CHECK(facts.pollSwGp700After80016E18 == -1);
    CHECK(facts.stateAfter.dword800917E8 == 4);
    CHECK(facts.stateAfter.dword800917F0 == 2);
    CHECK(facts.stateAfter.dword800917F4 == 0);

    PrStage1SaveUi19148LowerFeedbackRequest invalid = state1;
    invalid.psxFunction = 0x80017598u;
    CHECK(!BuildSaveUiCardIoPollFactsFromResult80016E18(invalid, 1, &facts));
    CHECK(!facts.factsKnown);
    CHECK(!BuildSaveUiCardIoPollFactsFromResult80016E18(state1, 5, &facts));
    CHECK(!facts.factsKnown);
}

void TestNaturalSwCardEventIngressUsesIdaPriorityAndTimeout() {
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(1);
    request.cardIoState.gp700 = 300;
    CardNaturalSwCardEventInput80016E18 input{};
    input.sourceKnown = true;
    input.source =
        CardNaturalEventIngressSource::DeviceTestEventProvider;
    for (bool& known : input.testEventResultKnown) {
        known = true;
    }
    input.testEventResults[0] = 1;
    input.testEventResults[1] = 1;
    input.testEventResults[2] = 0;
    input.testEventResults[3] = 1;
    input.gp700BeforeKnown = true;
    input.gp700Before = 300;

    CardIoHostFacts80017594 facts{};
    CHECK(BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
        request, input, &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.pollSwResult80016E18 == 4);
    CHECK(facts.pollSwGp700Before80016E18 == 300);
    CHECK(facts.pollSwGp700After80016E18 == 299);
    CHECK(facts.naturalSwCardEventSourceKnown80016E18);
    CHECK(facts.stateAfter.dword800917E8 == 2);

    input.gp700Before = 0;
    request.cardIoState.gp700 = 0;
    CHECK(BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
        request, input, &facts));
    CHECK(facts.pollSwResult80016E18 == 2);
    CHECK(facts.pollSwTimedOut80016E18);
    CHECK(facts.naturalSwCardEventSourceKnown80016E18);

    input.gp700Before = 300;
    CHECK(!BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
        request, input, &facts));
    CHECK(!facts.factsKnown);
}

void TestNaturalHwCardEventIngressRequiresFirstHit() {
    CardNaturalHwCardEventInput80017008 input{};
    int32_t pollResult = 99;
    CHECK(!ComputeNaturalHwCardPollResult80017008(input, &pollResult));
    CHECK(pollResult == 0);

    input.sourceKnown = true;
    input.source =
        CardNaturalEventIngressSource::DeviceTestEventProvider;
    for (bool& known : input.testEventResultKnown) {
        known = true;
    }
    input.testEventResults[0] = 0;
    input.testEventResults[1] = 1;
    input.testEventResults[2] = 1;
    input.testEventResults[3] = 0;
    CHECK(ComputeNaturalHwCardPollResult80017008(input, &pollResult));
    CHECK(pollResult == 2);

    input.testEventResults[1] = 0;
    input.testEventResults[2] = 0;
    input.testEventResults[3] = 1;
    CHECK(ComputeNaturalHwCardPollResult80017008(input, &pollResult));
    CHECK(pollResult == 4);
}

void TestState1Event4RequiresResetAndPostResetHwFacts() {
    CardIoHostFacts80017594 facts{};
    CHECK(!AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(facts));

    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 1;
    facts.stateAfterKnown = true;
    facts.stateAfter.dword800917E8 = 2;
    facts.stateAfter.dword800917F0 = 4;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;
    facts.drainHwEventsKnown8001707C = true;
    facts.resetHwEventsKnown80047EE4 = true;
    facts.resetHwNewCardKnown80047EE4 = true;
    facts.resetHwCardWriteArgsKnown80047EE4 = true;
    facts.resetHwCardWriteArg1_80047EE4 = 63;
    facts.resetHwCardWriteResultKnown80047EE4 = true;
    facts.pollHwKnown80017008 = true;
    CHECK(AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(facts));

    facts.resetHwCardWriteArg1_80047EE4 = 62;
    CHECK(!AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(facts));
}

void TestState1Event4ResetProviderCopiesExactBiosShape() {
    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 1;
    facts.stateAfterKnown = true;
    facts.stateAfter.dword800917E8 = 2;
    facts.stateAfter.dword800917F0 = 4;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;

    CardBiosResetProviderFacts80047EE4 provider{};
    provider.sourceKnown = true;
    provider.drainHwEventsKnown8001707C = true;
    provider.newCardKnown80047EE4 = true;
    provider.cardWriteArgsKnown80047EE4 = true;
    provider.cardWriteArg0_80047EE4 = 0;
    provider.cardWriteArg1_80047EE4 = 63;
    provider.cardWriteArg2_80047EE4 = 0;
    provider.cardWriteResultKnown80047EE4 = true;
    provider.cardWriteResult80047EE4 = -1;
    provider.pollHwKnown80017008 = true;
    provider.pollHwResult80017008 = 2;
    CHECK(ApplySaveUiCardIoEvent4ResetProviderFacts80047EE4(
        provider, &facts));
    CHECK(facts.resetHwCardWriteArg0_80047EE4 == 0);
    CHECK(facts.resetHwCardWriteArg1_80047EE4 == 63);
    CHECK(facts.resetHwCardWriteArg2_80047EE4 == 0);
    CHECK(facts.pollHwResult80017008 == 2);

    provider.cardWriteArg1_80047EE4 = 62;
    CHECK(!ApplySaveUiCardIoEvent4ResetProviderFacts80047EE4(
        provider, &facts));
}

void TestBiosResetProductionStorageProbeIsNonMutating() {
    const auto save = CreateFreshCardTestDirectory();
    CHECK(!save.empty());
    if (save.empty()) return; // No fallback to cwd or executable-adjacent saves.
    const auto primary = save / L"bu00.mcr";

    CHECK(!PrSS0CardImageStorageDirect::ProbeCardBiosResetAtDirectory80047EE4({}).sourceInstalled);
    CHECK(!PrSS0CardImageStorageDirect::ProbeCardBiosResetAtDirectory80047EE4(L"save").sourceInstalled);

    PrSS0CardImageStorageDirect::BiosCardResetProbe80047EE4 probe =
        PrSS0CardImageStorageDirect::ProbeCardBiosResetAtDirectory80047EE4(save);
    CHECK(probe.sourceInstalled);
    CHECK(probe.observationKnown);
    CHECK(!probe.mediaPresent);
    CHECK(!probe.imageReadable);
    CHECK(!probe.writable);
    CHECK(probe.cardWriteResultKnown);
    CHECK(probe.cardWriteResult == -1);

    std::vector<uint8_t> valid(kCardImageBytes8007A318, 0u);
    valid[0] = static_cast<uint8_t>('M');
    valid[1] = static_cast<uint8_t>('C');
    CHECK(WriteWholeFile(primary, valid));
    const std::vector<uint8_t> validBefore = ReadWholeFile(primary);
    CHECK(validBefore == valid);
    if (g_pauseAfterStorageFixture) {
        std::puts("CARD_IO_FIXTURE_READY");
        std::fflush(stdout);
        Sleep(INFINITE); // Parent safety test hard-kills only this child process.
    }
    probe = PrSS0CardImageStorageDirect::ProbeCardBiosResetAtDirectory80047EE4(save);
    CHECK(probe.sourceInstalled);
    CHECK(probe.observationKnown);
    CHECK(probe.mediaPresent);
    CHECK(probe.imageReadable);
    CHECK(probe.writable);
    CHECK(probe.cardWriteResultKnown);
    CHECK(probe.cardWriteResult == 0);
    CHECK(ReadWholeFile(primary) == validBefore);

    std::vector<uint8_t> invalid(kCardImageBytes8007A318, 0xA5u);
    CHECK(WriteWholeFile(primary, invalid));
    const std::vector<uint8_t> invalidBefore = ReadWholeFile(primary);
    CHECK(invalidBefore == invalid);
    probe = PrSS0CardImageStorageDirect::ProbeCardBiosResetAtDirectory80047EE4(save);
    CHECK(probe.sourceInstalled);
    CHECK(probe.observationKnown);
    CHECK(probe.mediaPresent);
    CHECK(!probe.imageReadable);
    CHECK(probe.writable);
    CHECK(probe.cardWriteResultKnown);
    CHECK(probe.cardWriteResult == -1);
    CHECK(ReadWholeFile(primary) == invalidBefore);
    // Retain this small disposable fixture. Crash/forced termination is safe
    // without running any destructor or restoring executable-adjacent cards.
}

void TestTranslatedCardEventBrokerIsOneShotAndOrdered() {
    ResetTranslatedCardEventBroker800170C4();
    CardTranslatedEventBrokerState800170C4 broker =
        GetTranslatedCardEventBrokerState800170C4();
    CHECK(broker.initialized);
    for (int32_t i = 0; i < 4; ++i) {
        CHECK(!broker.swPending[i]);
        CHECK(!broker.hwPending[i]);
    }

    CHECK(SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::CardInfo80017594, 1));
    CHECK(SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::CardLoad80017594, 4));
    CHECK(!SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::CardLoad80017594, 4));

    CardNaturalSwCardEventInput80016E18 sw{};
    CHECK(PollTranslatedSwCardEvents80016E18(300, &sw));
    CHECK(sw.sourceKnown);
    CHECK(sw.source ==
          CardNaturalEventIngressSource::TranslatedDirectCardEventBroker);
    CHECK(sw.testEventResults[0] == 1);
    CHECK(sw.testEventResults[3] == 1);
    broker = GetTranslatedCardEventBrokerState800170C4();
    for (int32_t i = 0; i < 4; ++i) {
        CHECK(!broker.swPending[i]);
    }
    CHECK(SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::CardLoad80017594, 3));
    DrainTranslatedSwCardEvents80016FC0();
    broker = GetTranslatedCardEventBrokerState800170C4();
    CHECK(!broker.swPending[2]);

    CHECK(SignalTranslatedHwCardEvent80017008(
        CardTranslatedEventSignalSource::Format80017B60, 3));
    CHECK(SignalTranslatedHwCardEvent80017008(
        CardTranslatedEventSignalSource::Format80017B60, 2));
    CardNaturalHwCardEventInput80017008 hw{};
    int32_t pollResult = 0;
    CHECK(PollTranslatedHwCardEvents80017008(&hw));
    CHECK(ComputeNaturalHwCardPollResult80017008(hw, &pollResult));
    CHECK(pollResult == 2);
    broker = GetTranslatedCardEventBrokerState800170C4();
    CHECK(!broker.hwPending[1]);
    CHECK(broker.hwPending[2]);

    CHECK(PollTranslatedHwCardEvents80017008(&hw));
    CHECK(ComputeNaturalHwCardPollResult80017008(hw, &pollResult));
    CHECK(pollResult == 3);
    broker = GetTranslatedCardEventBrokerState800170C4();
    CHECK(!broker.hwPending[2]);
    CHECK(SignalTranslatedHwCardEvent80017008(
        CardTranslatedEventSignalSource::Format80017B60, 4));
    DrainTranslatedHwCardEvents8001707C();
    broker = GetTranslatedCardEventBrokerState800170C4();
    CHECK(!broker.hwPending[3]);

    CHECK(!SignalTranslatedHwCardEvent80017008(
        CardTranslatedEventSignalSource::CardInfo80017594, 1));
    CHECK(!SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::Format80017B60, 1));
    CHECK(SignalTranslatedSwCardEvent80016E18(
        CardTranslatedEventSignalSource::PhysicalHotplug80017594, 4));
    CardNaturalSwCardEventInput80016E18 hotplug{};
    CHECK(PollTranslatedSwCardEvents80016E18(300, &hotplug));
    CHECK(hotplug.testEventResults[3] == 1);
}

void TestUnformattedMediaReachesNativeFormatEvent() {
    using namespace PrSS0CardImageStorageDirect;
    const auto save = CreateFreshCardTestDirectory();
    CHECK(!save.empty());
    if (save.empty()) return;
    CHECK(!ProbeCardMediaAtDirectory80017594({}).sourceInstalled);
    CHECK(!ProbeCardMediaAtDirectory80017594(L"save").sourceInstalled);
    CHECK(ResolveCardInfoEvent80017594({}) == 0);
    auto media = ProbeCardMediaAtDirectory80017594(save);
    CHECK(media.observationKnown && !media.mediaPresent);
    CHECK(ResolveCardInfoEvent80017594(media) == 3);
    const auto path = save / L"bu00.mcr";
    std::vector<uint8_t> raw(kCardImageBytes8007A318, 0u);
    CHECK(WriteWholeFile(path, raw));
    media = ProbeCardMediaAtDirectory80017594(save);
    CHECK(media.mediaPresent && media.imageRead && !media.headerValid);
    CHECK(ResolveCardInfoEvent80017594(media) == 1);
    const auto rawLoad = ReloadCardImageAtDirectory8007A318(save);
    CHECK(rawLoad.imageFileFound && rawLoad.imageRead && !rawLoad.imageValidated);
    CHECK(!rawLoad.sinkImported);
    CHECK(ResolveCardLoadEvent80017594(rawLoad, false) == 4);
    CHECK(ReadWholeFile(path) == raw);

    // Feed the observed event through the original state3 decoder. This
    // distinguishes successful device I/O from successful directory loading.
    PrStage1SaveUiCardIoState80017594 io{};
    io.dword800917E8 = 3;
    io.dword800917F0 = 1;
    io.gp700 = 300;
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594;
    request.psxFunction = 0x80017594u;
    request.cardIoState = io;
    CardIoHostFacts80017594 facts{};
    CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(
        request, ResolveCardLoadEvent80017594(rawLoad, false), &facts));
    CardIoLowerFeedbackBuildResult80017594 build{};
    BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &build);
    CHECK(build.lowerFeedbackKnown);
    const auto decoded = PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
        io, &build.lowerFeedback.cardIoFeedback80017594);
    CHECK(decoded.stateAfterKnown && !decoded.helperGap);
    CHECK(decoded.stateAfter.dword800917E8 == 4);
    CHECK(decoded.stateAfter.dword800917F0 == 5);
    const auto published = PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
        decoded.stateAfter, nullptr);
    CHECK(published.resultKnown && !published.helperGap);
    CHECK(published.result == 5);

    raw[0] = 'M'; raw[1] = 'C';
    CHECK(WriteWholeFile(path, raw));
    media = ProbeCardMediaAtDirectory80017594(save);
    CHECK(media.imageRead && media.headerValid);
    const auto validLoad = ReloadCardImageAtDirectory8007A318(save);
    CHECK(validLoad.sinkImported);
    CHECK(ResolveCardLoadEvent80017594(validLoad, true) == 1);
    CHECK(ResolveCardLoadEvent80017594(validLoad, false) == 3);
    CHECK(ReadWholeFile(path) == raw);
    raw.resize(7);
    CHECK(WriteWholeFile(path, raw));
    media = ProbeCardMediaAtDirectory80017594(save);
    CHECK(media.mediaPresent && !media.imageRead && !media.headerValid);
    CHECK(ResolveCardInfoEvent80017594(media) == 3);
    const auto shortLoad = ReloadCardImageAtDirectory8007A318(save);
    CHECK(ResolveCardLoadEvent80017594(shortLoad, false) == 3);
    CHECK(ReadWholeFile(path) == raw);
}

void TestKnownMediaErrorIsNotPollTimeout80017594() {
    // 80016E18 handle2/error -> event3; only the elapsed-counter timeout is2.
    for (int phase : {1, 3}) {
        PrStage1SaveUiCardIoState80017594 io{};
        io.dword800917E8 = phase;
        io.dword800917F0 = 1;
        io.gp700 = 300;
        PrStage1SaveUi19148LowerFeedbackRequest request{};
        request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594;
        request.psxFunction = 0x80017594u;
        request.cardIoState = io;
        CardIoHostFacts80017594 facts{};
        CHECK(BuildSaveUiCardIoPollFactsFromResult80016E18(request, 3, &facts));
        CardIoLowerFeedbackBuildResult80017594 feedback{};
        BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &feedback);
        CHECK(feedback.lowerFeedbackKnown);
        const auto decoded = PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
            io, &feedback.lowerFeedback.cardIoFeedback80017594);
        CHECK(decoded.stateAfterKnown && !decoded.helperGap);
        const auto published = PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
            decoded.stateAfter, nullptr);
        CHECK(published.resultKnown && !published.helperGap && published.result == 3);
    }
}

void TestCardCommunicationSetupAndTeardownOrder80017524() {
    const auto padInit = PrPsxPadDirect::PsxCall800354C0_InitPadRuntime(0);
    const auto setup = ExecuteCardCommunicationSetup80017524(padInit);
    CHECK(setup.sourceKnown);
    CHECK(setup.resetCallbackCalled);
    CHECK(setup.padInitCalled800354C0);
    CHECK(setup.padInit800354C0.accepted);
    CHECK(setup.initCard2Called);
    CHECK(setup.startCard2Called);
    CHECK(setup.buInitCalled);
    CHECK(setup.changeClearPadCalled);
    CHECK(setup.changeClearPadArg == 0);
    CHECK(setup.softwareEventHandlesOpened == 4u);
    CHECK(setup.hardwareEventHandlesOpened == 4u);
    CHECK(setup.eventsEnabled == 8u);
    CHECK(setup.cardGlobalsZeroed);
    CHECK(setup.dword800917E8 == 0);
    CHECK(setup.dword800917EC == 0);
    CHECK(setup.dword800917F0 == 0);
    CHECK(setup.dword800917F4 == 0);
    CHECK(setup.softwareStateCommitted);
    CHECK(!setup.physicalCardHalAuthority);
    CHECK(!setup.physicalPadHalAuthority);
    CHECK(GetTranslatedCardEventBrokerState800170C4().initialized);

    const auto teardown = ExecuteCardCommunicationTeardown80017574();
    CHECK(teardown.sourceKnown);
    CHECK(teardown.setupWasActive);
    CHECK(teardown.enterCriticalSectionCalled);
    CHECK(teardown.softwareEventHandlesClosed == 4u);
    CHECK(teardown.hardwareEventHandlesClosed == 4u);
    CHECK(teardown.exitCriticalSectionCalled);
    CHECK(teardown.softwareStateCommitted);
    CHECK(!GetTranslatedCardEventBrokerState800170C4().initialized);

    auto wrongPadInit = padInit;
    wrongPadInit.padInit2Protocol = 0;
    const auto rejected = ExecuteCardCommunicationSetup80017524(wrongPadInit);
    CHECK(!rejected.softwareStateCommitted);
    CHECK(!GetTranslatedCardEventBrokerState800170C4().initialized);

    // A second SaveUi entry must recreate the broker after native 17574.
    const auto setupAgain = ExecuteCardCommunicationSetup80017524(padInit);
    CHECK(setupAgain.softwareStateCommitted);
    CHECK(GetTranslatedCardEventBrokerState800170C4().initialized);
    ExecuteCardCommunicationTeardown80017574();
}

void TestSaveUiCardModeContextCopiesExact36Bytes80019148() {
    PrGameContext ctx{};
    PrStage1SaveUiDirect::Reset19148();
    const auto before = PrStage1SaveUiDirect::GetSaveUiCardModeContext8007CC50();
    CHECK(!before.known);
    CHECK(before.sourceAddress800544F8 == 0x800544F8u);
    CHECK(before.destinationAddress8007CC50 == 0x8007CC50u);
    CHECK(before.byteCount == 36u);
    CHECK(before.bytes != nullptr);
    CHECK(PrStage1SaveUiDirect::Start19148(ctx));
    const auto after = PrStage1SaveUiDirect::GetSaveUiCardModeContext8007CC50();
    CHECK(after.known);
    CHECK(after.byteCount == 36u);
    const uint8_t expected[36] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x01, 0x00};
    CHECK(std::equal(after.bytes, after.bytes + after.byteCount, expected));
    PrStage1SaveUiDirect::Reset19148();
}

void TestCardModeContextCopiesLiveMenuAndOwnsItsSnapshot() {
    namespace Menu = PrSS0MenuContextDirect;
    Menu::ResetColdBoot800544F8();
    PrSS0DirectoryDispatcherDirect::MainMenuState800264AC menu{};
    menu.cursor = 3;
    menu.itemValue[1] = -1;
    menu.itemValue[2] = menu.word800916DA = 1;
    menu.itemValue[3] = 3; // LOAD selection committed before 800191E4.
    CHECK(Menu::Publish800264AC(menu, 1));
    const auto source = Menu::Get80026784();
    const std::array<uint8_t, 36> expected = {
        1,0,0,0, 0,0,0,0, 0,0,0,0, 3,0,5,0,
        0,0,2,0, 255,255,1,0, 1,0,2,0, 3,0,4,0, 0,0,1,0};
    CHECK(source.known && source.bytes == expected);
    CHECK(PrStage1SaveUiDirect::CopyCurrentMainMenuContext80026784());
    auto copy = PrStage1SaveUiDirect::GetSaveUiCardModeContext8007CC50();
    CHECK(copy.known && copy.sourceGeneration800544F8 == source.generation);
    CHECK(std::equal(copy.bytes, copy.bytes + 36, expected.begin()));

    menu.itemValue[2] = menu.word800916DA = 0;
    menu.itemValue[3] = 2; // REPLAY selection on a later entry.
    CHECK(Menu::Publish800264AC(menu, 0));
    copy = PrStage1SaveUiDirect::GetSaveUiCardModeContext8007CC50();
    CHECK(std::equal(copy.bytes, copy.bytes + 36, expected.begin()));
    CHECK(copy.sourceGeneration800544F8 == source.generation);

    const auto liveMenuBeforeCallback = Menu::Get80026784();
    CHECK(PrStage1SaveUiDirect::SetCardModeContextControl8007CC50(0, -1, 1));
    copy = PrStage1SaveUiDirect::GetSaveUiCardModeContext8007CC50();
    CHECK(copy.bytes[0] == 0 && copy.bytes[4] == 255 && copy.bytes[7] == 255 && copy.bytes[8] == 1);
    CHECK(std::equal(copy.bytes + 12, copy.bytes + 36, expected.begin() + 12));
    CHECK(copy.sourceGeneration800544F8 == source.generation);
    CHECK(Menu::Get80026784().bytes == liveMenuBeforeCallback.bytes);

    PrGameContext ctx{};
    PrStage1SaveUiDirect::Reset19148();
    CHECK(PrStage1SaveUiDirect::Start19148(ctx));
    copy = PrStage1SaveUiDirect::GetSaveUiCardModeContext8007CC50();
    const auto later = Menu::Get80026784();
    CHECK(copy.known && copy.sourceGeneration800544F8 == later.generation);
    CHECK(std::equal(copy.bytes, copy.bytes + 36, later.bytes.begin()));
    menu.cursor = 4;
    menu.doneFlag = true;
    CHECK(Menu::Publish800264AC(menu, 1));
    CHECK(PrStage1SaveUiDirect::Start19148(ctx)); // active invocation: no recopy
    CHECK(std::equal(copy.bytes, copy.bytes + 36, later.bytes.begin()));

    menu.cursor = 32768;
    CHECK(!Menu::Publish800264AC(menu, 1));
    CHECK(!PrStage1SaveUiDirect::CopyCurrentMainMenuContext80026784());
    CHECK(!PrStage1SaveUiDirect::GetSaveUiCardModeContext8007CC50().known);
    CHECK(!PrStage1SaveUiDirect::SetCardModeContextControl8007CC50(1, 0, 0));
    PrStage1SaveUiDirect::Reset19148();
    CHECK(!PrStage1SaveUiDirect::Start19148(ctx));
    Menu::ResetColdBoot800544F8();
    PrStage1SaveUiDirect::Reset19148();
}

void TestState2CarriesLoadSubmitShapeOnly() {
    CardIoHostFacts80017594 facts{};
    CHECK(BuildSaveUiCardIoObservedNormalPathFacts80017594(
        MakeCardIoRequest(2), &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(facts.stateAfterKnown);
    CHECK(facts.clearSwEventsKnown80016FC0);
    CHECK(facts.cardLoadKnown);
    CHECK(facts.cardLoadArgKnown);
    CHECK(facts.cardLoadArg == 0);
    CHECK(facts.stateAfter.dword800917E8 == 3);
    CHECK(facts.stateAfter.gp700 == 300);
    CheckNoTypedSwPollAuthority(facts);
}

void TestState3RequiresExplicitTypedPollFeedback() {
    CardIoHostFacts80017594 facts{};
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(3);
    request.cardIoState.gp700 = 229;

    CHECK(!BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(!facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(!facts.stateAfterKnown);
    CHECK(!facts.cardInfoKnown);
    CHECK(!facts.cardLoadKnown);
    CheckNoTypedSwPollAuthority(facts);
}

void TestState3FormatCandidateShapeStillRequiresTypedPollFeedback() {
    CardIoHostFacts80017594 facts{};
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(3);
    request.cardIoState.dword800917EC = 0;
    request.cardIoState.dword800917F0 = 1;
    request.cardIoState.dword800917F4 = 0;
    request.cardIoState.gp700 = 229;

    CHECK(!BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(!facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(!facts.stateAfterKnown);
    CHECK(!facts.cardInfoKnown);
    CHECK(!facts.cardLoadKnown);
    CheckNoTypedSwPollAuthority(facts);
}

void TestExplicitState3TypedPollSuccessFeedback() {
    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 3;
    facts.stateBefore.dword800917F0 = 1;
    facts.stateBefore.gp700 = 300;
    facts.stateAfterKnown = true;
    facts.stateAfter = facts.stateBefore;
    facts.stateAfter.dword800917E8 = 4;
    facts.stateAfter.dword800917F4 = 1;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 1;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    CardIoLowerFeedbackBuildResult80017594 build{};
    BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &build);

    CHECK(build.lowerFeedbackKnown);
    CHECK(build.lowerFeedback.cardIoFeedbackKnown80017594);
    const PrStage1SaveUiCardIoFeedback80017594& feedback =
        build.lowerFeedback.cardIoFeedback80017594;
    CHECK(feedback.stateBeforeKnown);
    CHECK(feedback.stateAfterKnown);
    CHECK(feedback.stateBefore.dword800917E8 == 3);
    CHECK(feedback.stateAfter.dword800917E8 == 4);
    CHECK(feedback.stateAfter.dword800917F0 == 1);
    CHECK(feedback.stateAfter.dword800917F4 == 1);
    CHECK(feedback.stateAfter.gp700 == 299);
    CHECK(feedback.pollSwKnown80016E18);
    CHECK(feedback.pollSwResult80016E18 == 1);
    CHECK(feedback.pollSwGp700BeforeKnown80016E18);
    CHECK(feedback.pollSwGp700Before80016E18 == 300);
    CHECK(feedback.pollSwGp700AfterKnown80016E18);
    CHECK(feedback.pollSwGp700After80016E18 == 299);
    CHECK(feedback.pollSwTimedOutKnown80016E18);
    CHECK(!feedback.pollSwTimedOut80016E18);
}

void TestExplicitState3TypedPollNewCardPublishesFormatIoResult() {
    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore.dword800917E8 = 3;
    facts.stateBefore.dword800917F0 = 1;
    facts.stateBefore.gp700 = 300;
    facts.stateAfterKnown = true;
    facts.stateAfter = facts.stateBefore;
    facts.stateAfter.dword800917E8 = 4;
    facts.stateAfter.dword800917F0 = 5;
    facts.stateAfter.dword800917F4 = 0;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    CardIoLowerFeedbackBuildResult80017594 build{};
    BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &build);

    CHECK(build.lowerFeedbackKnown);
    CHECK(build.lowerFeedback.cardIoFeedbackKnown80017594);
    const PrStage1SaveUiCardIoFeedback80017594& feedback =
        build.lowerFeedback.cardIoFeedback80017594;
    CHECK(feedback.pollSwKnown80016E18);
    CHECK(feedback.pollSwResult80016E18 == 4);

    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiCardIoCarrier80017594 state3Carrier =
        PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
            facts.stateBefore,
            &feedback);
    CHECK(state3Carrier.resultKnown);
    CHECK(!state3Carrier.helperGap);
    CHECK(state3Carrier.stateAfterKnown);
    CHECK(state3Carrier.stateAfter.dword800917E8 == 4);
    CHECK(state3Carrier.stateAfter.dword800917F0 == 5);
    CHECK(state3Carrier.stateAfter.dword800917F4 == 0);

    const PrStage1SaveUiCardIoCarrier80017594 state4Carrier =
        PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
            state3Carrier.stateAfter,
            nullptr);
    CHECK(state4Carrier.resultKnown);
    CHECK(!state4Carrier.helperGap);
    CHECK(state4Carrier.result == 5);
    CHECK(state4Carrier.stateAfterKnown);
    CHECK(state4Carrier.stateAfter.dword800917E8 == 0);
    CHECK(state4Carrier.stateAfter.dword800917EC == 5);
}

void TestRuntimeState3TypedPollCarrierPublishesFormatIoResult() {
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(3);
    request.cardIoState.dword800917F0 = 1;
    request.cardIoState.gp700 = 300;

    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore = request.cardIoState;
    facts.stateAfterKnown = true;
    facts.stateAfter = request.cardIoState;
    facts.stateAfter.dword800917E8 = 4;
    facts.stateAfter.dword800917F0 = 5;
    facts.stateAfter.dword800917F4 = 0;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    ClearSaveUiCardIoState3TypedPollCarrier80017594();
    CHECK(PublishRuntimeSaveUiCardIoState3TypedPollCarrier80017594(
        request, facts));

    SaveUiCardIoState3TypedPollCarrier80017594 carrier{};
    CHECK(GetSaveUiCardIoState3TypedPollCarrier80017594(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.producerWired80016E18_80017594);
    CHECK(carrier.typedPollResultKnown80016E18);
    CHECK(carrier.pollResult80016E18 == 4);
    CHECK(carrier.lower.lowerFeedbackKnown);
    CHECK(carrier.lower.lowerFeedback.cardIoFeedbackKnown80017594);

    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiCardIoCarrier80017594 state3Carrier =
        PrStage1SaveUiDirect::BuildCardIoFeedback80017594(
            request.cardIoState,
            &carrier.lower.lowerFeedback.cardIoFeedback80017594);
    CHECK(state3Carrier.resultKnown);
    CHECK(!state3Carrier.helperGap);
    CHECK(state3Carrier.stateAfterKnown);
    CHECK(state3Carrier.stateAfter.dword800917E8 == 4);
    CHECK(state3Carrier.stateAfter.dword800917F0 == 5);
    CHECK(state3Carrier.stateAfter.dword800917F4 == 0);
    ClearSaveUiCardIoState3TypedPollCarrier80017594();
}

void TestRuntimeState3TypedPollCarrierClearsOnWrongRequest() {
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(3);
    request.cardIoState.dword800917F0 = 1;
    request.cardIoState.gp700 = 300;

    CardIoHostFacts80017594 facts{};
    facts.factsKnown = true;
    facts.stateBeforeKnown = true;
    facts.stateBefore = request.cardIoState;
    facts.stateAfterKnown = true;
    facts.stateAfter = request.cardIoState;
    facts.stateAfter.dword800917E8 = 4;
    facts.stateAfter.dword800917F0 = 5;
    facts.stateAfter.dword800917F4 = 0;
    facts.stateAfter.gp700 = 299;
    facts.pollSwKnown80016E18 = true;
    facts.pollSwResult80016E18 = 4;
    facts.pollSwGp700BeforeKnown80016E18 = true;
    facts.pollSwGp700Before80016E18 = 300;
    facts.pollSwGp700AfterKnown80016E18 = true;
    facts.pollSwGp700After80016E18 = 299;
    facts.pollSwTimedOutKnown80016E18 = true;
    facts.pollSwTimedOut80016E18 = false;

    ClearSaveUiCardIoState3TypedPollCarrier80017594();
    CHECK(PublishRuntimeSaveUiCardIoState3TypedPollCarrier80017594(
        request, facts));

    PrStage1SaveUi19148LowerFeedbackRequest wrongRequest = request;
    wrongRequest.psxFunction = 0x80017598u;
    CHECK(!PublishRuntimeSaveUiCardIoState3TypedPollCarrier80017594(
        wrongRequest, facts));

    SaveUiCardIoState3TypedPollCarrier80017594 carrier{};
    CHECK(!GetSaveUiCardIoState3TypedPollCarrier80017594(&carrier));
}

void TestState4CarriesPublishShapeOnly() {
    CardIoHostFacts80017594 facts{};
    PrStage1SaveUi19148LowerFeedbackRequest request = MakeCardIoRequest(4);
    request.cardIoState.dword800917F0 = 3;

    CHECK(BuildSaveUiCardIoObservedNormalPathFacts80017594(request, &facts));
    CHECK(facts.factsKnown);
    CHECK(facts.stateBeforeKnown);
    CHECK(facts.stateAfterKnown);
    CHECK(facts.stateAfter.dword800917E8 == 0);
    CHECK(facts.stateAfter.dword800917EC == 3);
    CheckNoTypedSwPollAuthority(facts);
}

PrStage1SaveUi19148LowerFeedbackRequest MakeWriteRequest() {
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10;
    request.psxFunction = kFn80017A10;
    request.retryCount = kWriteAttemptCount80017A10;
    request.nameAddress = kCardReadNameBufferAddr8007CBE8;
    request.dataAddress = kCardReadBlockBufferAddr800179B4;
    request.blockCount = kCardReadBlockCount800179B4;
    request.writeCloseGp696FactRequired80017A10 = true;
    request.writeCloseGp696Address80017A10 =
        kWriteCloseGp696Address80017A10;
    request.writeFdMustMatchCloseGp69680017A10 = true;
    request.action.kind =
        PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock;
    return request;
}

SaveUiWriteRuntimeFacts80017A10 MakeWriteRuntimeFacts() {
    SaveUiWriteRuntimeFacts80017A10 facts{};
    facts.factsKnown = true;
    facts.scanResultKnown80017900 = true;
    facts.scanResult80017900 = 1;
    facts.openWriteKnown80017454 = true;
    facts.openWriteFdKnown80017454 = true;
    facts.openWriteFd80017454 = 2;
    facts.openWriteReturnKnown80017454 = true;
    facts.openWriteReturn80017454 = 2;
    facts.gp696FdWriteKnown80017454 = true;
    facts.gp696Fd80017454 = 2;
    facts.clearSwEventsKnown80016FC0 = true;
    facts.writeKnown80017454 = true;
    facts.writeByteCountKnown80017454 = true;
    facts.writeByteCount80017454 =
        static_cast<int32_t>(kCardReadBlockBytes800179B4);
    facts.writeReturnKnown80017454 = true;
    facts.writeReturn80017454 = 0;
    facts.submitReturnKnown80017454 = true;
    facts.submitReturn80017454 = 0;
    facts.waitCallKnown80035560 = true;
    facts.waitArg80035560 = 4;
    facts.pollResultKnown80016EB8 = true;
    facts.pollResult80016EB8 = 1;
    facts.closeResultKnown = true;
    facts.closeResult = 0;
    facts.closeFdKnown = true;
    facts.closeFd = 2;
    facts.gp696FdCloseKnown80017A10 = true;
    facts.gp696FdClose80017A10 = 2;
    return facts;
}

void TestRuntimeWritePublishesTypedCarrier() {
    ClearSaveUiWriteTypedCarrier80017A10();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeWriteRequest();
    const SaveUiWriteRuntimeFacts80017A10 facts = MakeWriteRuntimeFacts();

    SaveUiWriteRuntimeProducerResult80017A10 producer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        request, facts, &producer);
    CHECK(producer.produced);
    CHECK(!producer.incomplete);
    CHECK(producer.requestMatched);
    CHECK(producer.hostFacts.factsKnown);

    CardWriteFeedbackProducerInput80017A10 input{};
    input.requestKnown = true;
    input.request = request;
    input.hostFactsKnown = true;
    input.hostFacts = producer.hostFacts;
    CardWriteLowerFeedbackBuildResult80017A10 lower{};
    BuildSaveUiWriteLowerFeedbackFromProducerInput80017A10(input, &lower);
    CHECK(lower.lowerFeedbackKnown);
    CHECK(lower.lowerFeedback.writeFeedbackKnown80017A10);
    CHECK(!lower.anyMissingRequiredFact);

    CHECK(PublishRuntimeSaveUiWriteTypedCarrier80017A10(request, facts));
    SaveUiWriteTypedCarrier80017A10 carrier{};
    CHECK(GetSaveUiWriteTypedCarrier80017A10(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.producerWired80017900_80017454_80016EB8_80017A10);
    CHECK(carrier.typedWriteSuccessKnown80017A10);
    CHECK(carrier.lower.lowerFeedback.writeFeedbackKnown80017A10);
    CHECK(!carrier.lower.anyMissingRequiredFact);

    ClearSaveUiWriteTypedCarrier80017A10();
}

void TestRuntimeWriteFailureClearsTypedCarrier() {
    ClearSaveUiWriteTypedCarrier80017A10();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeWriteRequest();
    const SaveUiWriteRuntimeFacts80017A10 successFacts =
        MakeWriteRuntimeFacts();
    CHECK(PublishRuntimeSaveUiWriteTypedCarrier80017A10(request,
                                                       successFacts));

    SaveUiWriteRuntimeFacts80017A10 failedPollFacts = successFacts;
    failedPollFacts.pollResult80016EB8 = 2;
    SaveUiWriteRuntimeProducerResult80017A10 failedPollProducer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        request,
        failedPollFacts,
        &failedPollProducer);
    CHECK(!failedPollProducer.produced);
    CHECK(failedPollProducer.incomplete);
    CHECK(failedPollProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiWriteTypedCarrier80017A10(request,
                                                        failedPollFacts));
    SaveUiWriteTypedCarrier80017A10 carrier{};
    CHECK(!GetSaveUiWriteTypedCarrier80017A10(&carrier));

    CHECK(PublishRuntimeSaveUiWriteTypedCarrier80017A10(request,
                                                       successFacts));
    SaveUiWriteRuntimeFacts80017A10 closeMismatchFacts = successFacts;
    closeMismatchFacts.closeFd = successFacts.closeFd + 1;
    closeMismatchFacts.gp696FdClose80017A10 = successFacts.gp696Fd80017454;
    SaveUiWriteRuntimeProducerResult80017A10 closeMismatchProducer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        request,
        closeMismatchFacts,
        &closeMismatchProducer);
    CHECK(!closeMismatchProducer.produced);
    CHECK(closeMismatchProducer.incomplete);
    CHECK(closeMismatchProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiWriteTypedCarrier80017A10(
        request,
        closeMismatchFacts));
    CHECK(!GetSaveUiWriteTypedCarrier80017A10(&carrier));

    CHECK(PublishRuntimeSaveUiWriteTypedCarrier80017A10(request,
                                                       successFacts));
    PrStage1SaveUi19148LowerFeedbackRequest wrongRequest = request;
    wrongRequest.dataAddress = kCardReadPayloadAddr8007ADE8;
    SaveUiWriteRuntimeProducerResult80017A10 wrongRequestProducer{};
    BuildSaveUiWriteFactsFromRuntimeProducerInput80017A10(
        wrongRequest,
        successFacts,
        &wrongRequestProducer);
    CHECK(!wrongRequestProducer.produced);
    CHECK(wrongRequestProducer.incomplete);
    CHECK(!wrongRequestProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiWriteTypedCarrier80017A10(wrongRequest,
                                                        successFacts));
    CHECK(!GetSaveUiWriteTypedCarrier80017A10(&carrier));

    ClearSaveUiWriteTypedCarrier80017A10();
}

CardFormatHostFacts80017B60 MakeFormatHostFacts(
    int32_t firstPoll,
    int32_t secondPoll,
    int32_t thirdPoll) {
    CardFormatHostFacts80017B60 facts{};
    facts.factsKnown = true;
    const int32_t polls[3] = {firstPoll, secondPoll, thirdPoll};
    for (int32_t i = 0; i < 3; ++i) {
        CardFormatHostAttemptFacts80017B60& attempt = facts.attempts[i];
        attempt.drainHwEventsKnown8001707C = true;
        attempt.formatKnown = true;
        attempt.formatArgsKnown = true;
        attempt.formatArg0 = 0x8006EABCu;
        attempt.formatArg1 = 0x8006EAC0u;
        attempt.pollResultKnown80017008 = true;
        attempt.pollResult80017008 = polls[i];
    }
    return facts;
}

PrStage1SaveUi19148LowerFeedbackRequest MakeFormatRequest() {
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    request.kind = PrStage1SaveUi19148LowerFeedbackRequestKind::Format80017B60;
    request.psxFunction = kFn80017B60;
    request.retryCount = kFormatAttemptCount80017B60;
    request.formatArg0 = kFormatArg0_80017B60;
    request.formatArg1 = kFormatArg1_80017B60;
    request.action.kind =
        PrStage1SaveUi19148ActionKind::Call80017B60FormatCard;
    return request;
}

SaveUiFormatRuntimeFacts80017B60 MakeFormatRuntimeFacts(
    int32_t firstPoll,
    int32_t secondPoll,
    int32_t thirdPoll) {
    SaveUiFormatRuntimeFacts80017B60 facts{};
    facts.factsKnown = true;
    const int32_t polls[3] = {firstPoll, secondPoll, thirdPoll};
    for (int32_t i = 0; i < kFormatAttemptCount80017B60; ++i) {
        SaveUiFormatRuntimeAttemptFacts80017B60& attempt =
            facts.attempts[i];
        attempt.drainHwEventsKnown8001707C = true;
        attempt.formatKnown = true;
        attempt.formatArgsKnown = true;
        attempt.formatArg0 = kFormatArg0_80017B60;
        attempt.formatArg1 = kFormatArg1_80017B60;
        attempt.pollResultKnown80017008 = true;
        attempt.pollResult80017008 = polls[i];
    }
    return facts;
}

void TestFormatHalCarriesPollOneSuccessFacts() {
    const CardFormatHostFacts80017B60 facts =
        MakeFormatHostFacts(1, 2, 2);
    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(facts, &lower);

    CHECK(lower.lowerFeedbackKnown);
    CHECK(!lower.incomplete);
    CHECK(lower.lowerFeedback.formatFeedbackKnown80017B60);
    const PrStage1SaveUiFormatAttemptFeedback80017B60& attempt =
        lower.lowerFeedback.formatFeedback80017B60.attempts[0];
    CHECK(attempt.drainHwEventsKnown8001707C);
    CHECK(attempt.formatKnown);
    CHECK(attempt.formatArgsKnown);
    CHECK(attempt.formatArg0 == 0x8006EABCu);
    CHECK(attempt.formatArg1 == 0x8006EAC0u);
    CHECK(attempt.pollResultKnown80017008);
    CHECK(attempt.pollResult80017008 == 1);
}

void TestFormatHalCarriesPollThreeFailureFacts() {
    const CardFormatHostFacts80017B60 facts =
        MakeFormatHostFacts(3, 1, 1);
    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(facts, &lower);

    CHECK(lower.lowerFeedbackKnown);
    CHECK(lower.lowerFeedback.formatFeedbackKnown80017B60);
    CHECK(lower.lowerFeedback.formatFeedback80017B60.attempts[0]
              .pollResultKnown80017008);
    CHECK(lower.lowerFeedback.formatFeedback80017B60.attempts[0]
              .pollResult80017008 == 3);
}

void TestFormatHalCarriesRetryExhaustionFacts() {
    const CardFormatHostFacts80017B60 facts =
        MakeFormatHostFacts(2, 2, 2);
    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(facts, &lower);

    CHECK(lower.lowerFeedbackKnown);
    CHECK(lower.lowerFeedback.formatFeedbackKnown80017B60);
    for (int32_t i = 0; i < 3; ++i) {
        const PrStage1SaveUiFormatAttemptFeedback80017B60& attempt =
            lower.lowerFeedback.formatFeedback80017B60.attempts[i];
        CHECK(attempt.drainHwEventsKnown8001707C);
        CHECK(attempt.formatKnown);
        CHECK(attempt.pollResultKnown80017008);
        CHECK(attempt.pollResult80017008 == 2);
    }
}

PrStage1SaveUiFormatFeedbackInput80017B60 MakeFormatFeedbackInput(
    int32_t firstPoll,
    int32_t secondPoll,
    int32_t thirdPoll) {
    const CardFormatHostFacts80017B60 facts =
        MakeFormatHostFacts(firstPoll, secondPoll, thirdPoll);
    CardFormatLowerFeedbackBuildResult80017B60 lower{};
    BuildSaveUiFormatLowerFeedbackFromHostFacts80017B60(facts, &lower);
    CHECK(lower.lowerFeedbackKnown);
    CHECK(!lower.incomplete);
    CHECK(lower.lowerFeedback.formatFeedbackKnown80017B60);
    return lower.lowerFeedback.formatFeedback80017B60;
}

void TestFormatTypedFeedbackDrivesSaveUiSuccessCarrier() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiFormatFeedbackInput80017B60 feedback =
        MakeFormatFeedbackInput(1, 2, 2);

    const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(&feedback);

    CHECK(carrier.translated);
    CHECK(carrier.callCompleted);
    CHECK(carrier.resultKnown);
    CHECK(!carrier.retryExhaustedReturnUnknown);
    CHECK(!carrier.helperGap);
    CHECK(carrier.result == 1);
    CHECK(carrier.attemptsUsed == 1);
    CHECK(carrier.stoppedOnSuccess);
    CHECK(!carrier.stoppedOnTimeout);
}

void TestFormatTypedFeedbackDrivesSaveUiPollThreeFailureCarrier() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiFormatFeedbackInput80017B60 feedback =
        MakeFormatFeedbackInput(3, 1, 1);

    const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(&feedback);

    CHECK(carrier.callCompleted);
    CHECK(carrier.resultKnown);
    CHECK(!carrier.retryExhaustedReturnUnknown);
    CHECK(!carrier.helperGap);
    CHECK(carrier.result == 3);
    CHECK(carrier.attemptsUsed == 1);
    CHECK(!carrier.stoppedOnSuccess);
    CHECK(carrier.stoppedOnTimeout);
}

void TestFormatTypedFeedbackDrivesSaveUiRetryExhaustionCarrier() {
    PrStage1SaveUiDirect::Reset19148();
    const PrStage1SaveUiFormatFeedbackInput80017B60 feedback =
        MakeFormatFeedbackInput(2, 2, 2);

    const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(&feedback);

    CHECK(carrier.callCompleted);
    CHECK(!carrier.resultKnown);
    CHECK(carrier.retryExhaustedReturnUnknown);
    CHECK(!carrier.helperGap);
    CHECK(carrier.attemptsUsed == 3);
    CHECK(!carrier.stoppedOnSuccess);
    CHECK(!carrier.stoppedOnTimeout);
}

void TestFormatTypedFeedbackMissingPollFailsClosedInSaveUiCarrier() {
    PrStage1SaveUiDirect::Reset19148();

    const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
        PrStage1SaveUiDirect::BuildFormatFeedback80017B60(nullptr);

    CHECK(!carrier.resultKnown);
    CHECK(carrier.helperGap);
    CHECK(carrier.attemptsUsed == 1);
    CHECK(!carrier.stoppedOnSuccess);
    CHECK(!carrier.stoppedOnTimeout);
}

void TestRuntimeFormatPublishesTypedCarrier() {
    ClearSaveUiFormatTypedCarrier80017B60();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeFormatRequest();
    const SaveUiFormatRuntimeFacts80017B60 facts =
        MakeFormatRuntimeFacts(1, 2, 2);

    SaveUiFormatRuntimeProducerResult80017B60 producer{};
    BuildSaveUiFormatFactsFromRuntimeProducerInput80017B60(
        request,
        facts,
        &producer);
    CHECK(producer.produced);
    CHECK(!producer.incomplete);
    CHECK(producer.requestMatched);
    CHECK(producer.runtimeFactsKnown);
    CHECK(producer.callCompleted);
    CHECK(producer.resultKnown);
    CHECK(!producer.retryExhaustedReturnUnknown);
    CHECK(producer.result80017B60 == 1);
    CHECK(producer.lower.lowerFeedback.formatFeedbackKnown80017B60);

    CHECK(PublishRuntimeSaveUiFormatTypedCarrier80017B60(request, facts));
    SaveUiFormatTypedCarrier80017B60 carrier{};
    CHECK(GetSaveUiFormatTypedCarrier80017B60(&carrier));
    CHECK(carrier.known);
    CHECK(carrier.producerWired8001707C_80017008_80017B60);
    CHECK(carrier.formatCallCompleted80017B60);
    CHECK(carrier.typedFormatResultKnown80017B60);
    CHECK(!carrier.retryExhaustedReturnUnknown80017B60);
    CHECK(carrier.result80017B60 == 1);
    CHECK(carrier.lower.lowerFeedback.formatFeedbackKnown80017B60);

    ClearSaveUiFormatTypedCarrier80017B60();
}

void TestRuntimeFormatPollThreePublishesTerminalCarrier() {
    ClearSaveUiFormatTypedCarrier80017B60();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeFormatRequest();
    const SaveUiFormatRuntimeFacts80017B60 facts =
        MakeFormatRuntimeFacts(3, 1, 1);

    CHECK(PublishRuntimeSaveUiFormatTypedCarrier80017B60(request, facts));
    SaveUiFormatTypedCarrier80017B60 carrier{};
    CHECK(GetSaveUiFormatTypedCarrier80017B60(&carrier));
    CHECK(carrier.formatCallCompleted80017B60);
    CHECK(carrier.typedFormatResultKnown80017B60);
    CHECK(!carrier.retryExhaustedReturnUnknown80017B60);
    CHECK(carrier.result80017B60 == 3);
    CHECK(carrier.lower.lowerFeedback.formatFeedback80017B60.attempts[0]
              .pollResultKnown80017008);
    CHECK(carrier.lower.lowerFeedback.formatFeedback80017B60.attempts[0]
              .pollResult80017008 == 3);

    ClearSaveUiFormatTypedCarrier80017B60();
}

void TestRuntimeFormatFailureClearsTypedCarrier() {
    ClearSaveUiFormatTypedCarrier80017B60();

    const PrStage1SaveUi19148LowerFeedbackRequest request =
        MakeFormatRequest();
    const SaveUiFormatRuntimeFacts80017B60 successFacts =
        MakeFormatRuntimeFacts(1, 2, 2);
    CHECK(PublishRuntimeSaveUiFormatTypedCarrier80017B60(request,
                                                        successFacts));

    SaveUiFormatRuntimeFacts80017B60 missingPollFacts = successFacts;
    missingPollFacts.attempts[0].pollResultKnown80017008 = false;
    SaveUiFormatRuntimeProducerResult80017B60 missingPollProducer{};
    BuildSaveUiFormatFactsFromRuntimeProducerInput80017B60(
        request,
        missingPollFacts,
        &missingPollProducer);
    CHECK(!missingPollProducer.produced);
    CHECK(missingPollProducer.incomplete);
    CHECK(missingPollProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiFormatTypedCarrier80017B60(
        request,
        missingPollFacts));
    SaveUiFormatTypedCarrier80017B60 carrier{};
    CHECK(!GetSaveUiFormatTypedCarrier80017B60(&carrier));

    CHECK(PublishRuntimeSaveUiFormatTypedCarrier80017B60(request,
                                                        successFacts));
    PrStage1SaveUi19148LowerFeedbackRequest wrongRequest = request;
    wrongRequest.formatArg1 = 0x8006EAC4u;
    SaveUiFormatRuntimeProducerResult80017B60 wrongRequestProducer{};
    BuildSaveUiFormatFactsFromRuntimeProducerInput80017B60(
        wrongRequest,
        successFacts,
        &wrongRequestProducer);
    CHECK(!wrongRequestProducer.produced);
    CHECK(wrongRequestProducer.incomplete);
    CHECK(!wrongRequestProducer.requestMatched);
    CHECK(!PublishRuntimeSaveUiFormatTypedCarrier80017B60(wrongRequest,
                                                         successFacts));
    CHECK(!GetSaveUiFormatTypedCarrier80017B60(&carrier));

    ClearSaveUiFormatTypedCarrier80017B60();
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--pause-after-storage-fixture") {
        g_pauseAfterStorageFixture = true;
    } else if (argc != 1) {
        return 2;
    }
    TestRejectsNonCardIoRequest();
    TestState0CarriesInfoSubmitShapeOnly();
    TestState1RequiresExplicitTypedPollFeedback();
    TestExplicitState1TypedPollSuccessFeedback();
    TestCurrentIdaPollResultFacts80016E18();
    TestNaturalSwCardEventIngressUsesIdaPriorityAndTimeout();
    TestNaturalHwCardEventIngressRequiresFirstHit();
    TestState1Event4RequiresResetAndPostResetHwFacts();
    TestState1Event4ResetProviderCopiesExactBiosShape();
    TestBiosResetProductionStorageProbeIsNonMutating();
    TestUnformattedMediaReachesNativeFormatEvent();
    TestKnownMediaErrorIsNotPollTimeout80017594();
    TestCardLoadRereadsReplacementMediumAndDropsStaleDirectory();
    TestState16ReadsCurrentFileByNameNotCachedPhysicalBlock();
    TestHiScoreCase6SnapshotSurvivesDirectoryChange();
    TestNamedCardFileReadOwnsReadOnlyHandle();
    TestCase17OpensEachCapturedNameAndOwnsAllReadBuffers();
    TestDeferredNamedReadHandleAndNativePollOrder();
    TestTranslatedCardEventBrokerIsOneShotAndOrdered();
    TestCardCommunicationSetupAndTeardownOrder80017524();
    TestSaveUiCardModeContextCopiesExact36Bytes80019148();
    TestCardModeContextCopiesLiveMenuAndOwnsItsSnapshot();
    TestState2CarriesLoadSubmitShapeOnly();
    TestState3RequiresExplicitTypedPollFeedback();
    TestState3FormatCandidateShapeStillRequiresTypedPollFeedback();
    TestExplicitState3TypedPollSuccessFeedback();
    TestExplicitState3TypedPollNewCardPublishesFormatIoResult();
    TestRuntimeState3TypedPollCarrierPublishesFormatIoResult();
    TestRuntimeState3TypedPollCarrierClearsOnWrongRequest();
    TestState4CarriesPublishShapeOnly();
    TestRuntimeWritePublishesTypedCarrier();
    TestRuntimeWriteFailureClearsTypedCarrier();
    TestFormatHalCarriesPollOneSuccessFacts();
    TestFormatHalCarriesPollThreeFailureFacts();
    TestFormatHalCarriesRetryExhaustionFacts();
    TestFormatTypedFeedbackDrivesSaveUiSuccessCarrier();
    TestFormatTypedFeedbackDrivesSaveUiPollThreeFailureCarrier();
    TestFormatTypedFeedbackDrivesSaveUiRetryExhaustionCarrier();
    TestFormatTypedFeedbackMissingPollFailsClosedInSaveUiCarrier();
    TestRuntimeFormatPublishesTypedCarrier();
    TestRuntimeFormatPollThreePublishesTerminalCarrier();
    TestRuntimeFormatFailureClearsTypedCarrier();

    if (g_failed != 0) {
        std::printf("test_ss0_card_io_authority: failed checks=%d\n",
                    g_failed);
        return 1;
    }

    std::printf("test_ss0_card_io_authority: ok\n");
    return 0;
}
