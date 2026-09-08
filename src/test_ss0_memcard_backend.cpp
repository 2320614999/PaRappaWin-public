#include "pr/pr_memcard_backend.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>

namespace {

constexpr std::size_t kCardBlockSize = 8192u;
constexpr std::size_t kCardPayloadOffset = 0x200u;
constexpr std::size_t kCardPayloadSize = 4876u;
constexpr std::size_t kStageLastSavedSlotPayloadOffset = 0x2Cu;

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

std::filesystem::path ExecutableDir() {
    wchar_t path[MAX_PATH];
    const DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(path).parent_path();
}

void WriteU32LE(uint8_t* p, uint32_t value) {
    p[0] = static_cast<uint8_t>(value & 0xFFu);
    p[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
    p[2] = static_cast<uint8_t>((value >> 16) & 0xFFu);
    p[3] = static_cast<uint8_t>((value >> 24) & 0xFFu);
}

void FillPayload(uint8_t value) {
    uint8_t* payload = PrMemCardBackend::MutablePayload();
    std::fill(payload, payload + PrMemCardBackend::PayloadSize(), value);
}

bool PayloadEquals(uint8_t value) {
    const uint8_t* payload = PrMemCardBackend::CurrentPayload();
    const std::size_t size = PrMemCardBackend::PayloadSize();
    return std::all_of(payload, payload + size, [value](uint8_t byte) {
        return byte == value;
    });
}

std::array<uint8_t, kCardBlockSize> MakeReplayBlock(uint8_t payloadValue,
                                                     uint32_t savedSlot) {
    std::array<uint8_t, kCardBlockSize> block{};
    block[0] = 'S';
    block[1] = 'C';
    std::fill(block.begin() + kCardPayloadOffset,
              block.begin() + kCardPayloadOffset + kCardPayloadSize,
              payloadValue);
    WriteU32LE(block.data() + kCardPayloadOffset +
                   kStageLastSavedSlotPayloadOffset,
               savedSlot);
    return block;
}

int SaveReplayBlock(const char* name,
                    const std::array<uint8_t, kCardBlockSize>& block) {
    const PrMemCardBackend::EntryWriteResult result =
        PrMemCardBackend::SaveEntryBlockByName(name, block.data(), block.size());
    CHECK(result.attempted);
    CHECK(result.saved);
    CHECK(result.blockIndex >= 0);
    return result.blockIndex;
}

void ResetIsolatedSaveDir() {
    const std::filesystem::path saveDir = ExecutableDir() / L"save";
    std::error_code ec;
    std::filesystem::remove_all(saveDir, ec);
    std::filesystem::create_directories(saveDir, ec);
}

void TestLoadReplayEntryRestoresOnMissingEntry() {
    FillPayload(0xA5u);
    const PrMemCardBackend::ReplayEntryLoadResult result =
        PrMemCardBackend::LoadReplayEntry(14);
    CHECK(!result.entryLoaded);
    CHECK(!result.payloadCommitted);
    CHECK(result.replayScene == -1);
    CHECK(!result.rawSavedSlotKnown);
    CHECK(PayloadEquals(0xA5u));
}

void CheckLoadReplayEntryRestoresInvalidScene(const char* name,
                                              uint32_t savedSlot) {
    const auto invalidBlock = MakeReplayBlock(0x44u, savedSlot);
    const int blockIndex = SaveReplayBlock(name, invalidBlock);

    FillPayload(0xA5u);
    const PrMemCardBackend::ReplayEntryLoadResult result =
        PrMemCardBackend::LoadReplayEntry(blockIndex);
    CHECK(result.entryLoaded);
    CHECK(!result.payloadCommitted);
    CHECK(result.replayScene == -1);
    CHECK(result.rawSavedSlotKnown);
    CHECK(result.rawSavedSlot == savedSlot);
    CHECK(PayloadEquals(0xA5u));
}

void TestLoadReplayEntryRestoresInvalidScene() {
    CheckLoadReplayEntryRestoresInvalidScene("SS0TESTINVALID6", 6u);
    CheckLoadReplayEntryRestoresInvalidScene("SS0TESTINVALID7", 7u);
    CheckLoadReplayEntryRestoresInvalidScene("SS0TESTINVALID8", 8u);
    CheckLoadReplayEntryRestoresInvalidScene("SS0TESTINVALIDMAX", 0xFFFFFFFFu);
}

void CheckLoadReplayEntryCommitsValidScene(const char* name,
                                           uint32_t savedSlot,
                                           int expectedReplayScene) {
    const auto validBlock = MakeReplayBlock(0x33u, savedSlot);
    const int blockIndex = SaveReplayBlock(name, validBlock);

    FillPayload(0xA5u);
    const PrMemCardBackend::ReplayEntryLoadResult result =
        PrMemCardBackend::LoadReplayEntry(blockIndex);
    CHECK(result.entryLoaded);
    CHECK(result.payloadCommitted);
    CHECK(result.replayScene == expectedReplayScene);
    CHECK(result.rawSavedSlotKnown);
    CHECK(result.rawSavedSlot == savedSlot);
    CHECK(!PayloadEquals(0xA5u));
    CHECK(PrMemCardBackend::ReadPayloadU32(kStageLastSavedSlotPayloadOffset) ==
          savedSlot);
}

void TestLoadReplayEntryCommitsValidScene() {
    CheckLoadReplayEntryCommitsValidScene("SS0TESTVALID0", 0u, 1);
    CheckLoadReplayEntryCommitsValidScene("SS0TESTVALID1", 1u, 2);
    CheckLoadReplayEntryCommitsValidScene("SS0TESTVALID2", 2u, 3);
    CheckLoadReplayEntryCommitsValidScene("SS0TESTVALID3", 3u, 4);
    CheckLoadReplayEntryCommitsValidScene("SS0TESTVALID4", 4u, 5);
    CheckLoadReplayEntryCommitsValidScene("SS0TESTVALID5", 5u, 6);
}

} // namespace

int main() {
    ResetIsolatedSaveDir();
    CHECK(PrMemCardBackend::FormatPrimaryCard());
    TestLoadReplayEntryRestoresOnMissingEntry();
    TestLoadReplayEntryRestoresInvalidScene();
    TestLoadReplayEntryCommitsValidScene();

    if (g_failed != 0) {
        std::printf("test_ss0_memcard_backend: failed checks=%d\n", g_failed);
        return 1;
    }

    std::printf("test_ss0_memcard_backend: ok\n");
    return 0;
}
