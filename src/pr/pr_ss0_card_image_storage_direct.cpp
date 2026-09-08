#include "pr_ss0_card_image_storage_direct.h"

#include "logger.h"
#include "pr_stage1_save_ui_direct.h"

#include <windows.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace PrSS0CardImageStorageDirect {
namespace {

constexpr std::size_t kCardImageBytes8007A318 = 128u * 1024u;
constexpr int32_t kDefaultSlotPolicyBlock8007A318 = 0;

struct CardMediaSnapshot8007A318 {
    bool initialized = false;
    bool present = false;
    uintmax_t byteCount = 0u;
    int64_t writeTime = 0;
    std::filesystem::path path{};
};

CardMediaSnapshot8007A318 s_cardMediaSnapshot8007A318{};

std::filesystem::path ExecutableDirectory() {
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0u || length >= MAX_PATH) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(path).parent_path();
}

std::filesystem::path PrimaryCardPath() {
    return ExecutableDirectory() / L"save" / L"bu00.mcr";
}

std::filesystem::path ResolveReadableCardPath() {
    const std::filesystem::path save = ExecutableDirectory() / L"save";
    const std::array<std::filesystem::path, 3> candidates{{
        save / L"bu00.mcr",
        save / L"bu01.mcr",
        save / L"bu10.mcr",
    }};
    std::error_code error;
    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate, error)) {
            return candidate;
        }
        error.clear();
    }
    return {};
}

CardMediaSnapshot8007A318 CapturePrimaryCardMediaSnapshot8007A318() {
    CardMediaSnapshot8007A318 snapshot{};
    const std::filesystem::path path = ResolveReadableCardPath();
    if (path.empty()) {
        snapshot.present = false;
        return snapshot;
    }
    std::error_code error;
    const uintmax_t byteCount = std::filesystem::file_size(path, error);
    if (error) {
        return snapshot;
    }
    const auto writeTime = std::filesystem::last_write_time(path, error);
    if (error) {
        return snapshot;
    }
    snapshot.present = true;
    snapshot.byteCount = byteCount;
    snapshot.writeTime = static_cast<int64_t>(writeTime.time_since_epoch().count());
    snapshot.path = path;
    return snapshot;
}

bool SamePrimaryCardMediaSnapshot8007A318(
    const CardMediaSnapshot8007A318& lhs,
    const CardMediaSnapshot8007A318& rhs) {
    return lhs.present == rhs.present &&
           lhs.byteCount == rhs.byteCount &&
           lhs.writeTime == rhs.writeTime && lhs.path == rhs.path;
}

bool ReadExactCardImage(const std::filesystem::path& path,
                        std::array<uint8_t, kCardImageBytes8007A318>& image) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.read(reinterpret_cast<char*>(image.data()),
              static_cast<std::streamsize>(image.size()));
    return file.gcount() == static_cast<std::streamsize>(image.size()) &&
           file.peek() == std::ifstream::traits_type::eof();
}

bool IsCardImageHeaderKnown(const uint8_t* bytes, std::size_t byteCount) {
    return bytes != nullptr && byteCount == kCardImageBytes8007A318 &&
           bytes[0] == static_cast<uint8_t>('M') &&
           bytes[1] == static_cast<uint8_t>('C');
}

bool WritePrimaryCardImageAtomically(int32_t blockIndex,
                                     const uint8_t* bytes,
                                     std::size_t byteCount,
                                     void*) {
    if (blockIndex < 0 || blockIndex >= 15 ||
        !IsCardImageHeaderKnown(bytes, byteCount)) {
        return false;
    }

    const std::filesystem::path path = PrimaryCardPath();
    const std::filesystem::path temporary = path.wstring() + L".tmp";
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return false;
    }
    {
        std::ofstream file(temporary,
                           std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            return false;
        }
        file.write(reinterpret_cast<const char*>(bytes),
                   static_cast<std::streamsize>(byteCount));
        file.flush();
        if (!file) {
            return false;
        }
    }

    if (MoveFileExW(temporary.c_str(), path.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0) {
        error.clear();
        std::filesystem::remove(temporary, error);
        return false;
    }
    RefreshPrimaryCardMediaSnapshot8007A318();
    return true;
}

} // namespace

StartupBinding8007A318 BindAndLoadPrimaryCardImage8007A318() {
    StartupBinding8007A318 result{};
    RefreshPrimaryCardMediaSnapshot8007A318();
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        WritePrimaryCardImageAtomically, nullptr);
    result.backendBound = true;

    const PrStage1SaveUiCardImagePersistenceView8007A318 existing =
        PrStage1SaveUiDirect::GetSaveUiCardImagePersistenceSinkView8007A318();
    if (existing.known && existing.slotPolicyKnown &&
        existing.blockIndex >= 0 && existing.blockIndex < 15 &&
        existing.bytes != nullptr &&
        existing.byteCount == kCardImageBytes8007A318 &&
        existing.byteSize == kCardImageBytes8007A318) {
        result.existingSinkPreserved = true;
        result.sinkImported = true;
        result.blockIndex = existing.blockIndex;
        RefreshPrimaryCardMediaSnapshot8007A318();
        return result;
    }

    const std::filesystem::path path = ResolveReadableCardPath();
    result.imageFileFound = !path.empty();
    if (!result.imageFileFound) {
        return result;
    }

    std::array<uint8_t, kCardImageBytes8007A318> image{};
    result.imageRead = ReadExactCardImage(path, image);
    result.imageValidated = result.imageRead &&
                            IsCardImageHeaderKnown(image.data(), image.size());
    if (!result.imageValidated) {
        Log::Printf(
            "SS0 direct card storage: startup image rejected path=%s read=%d",
            path.string().c_str(),
            result.imageRead ? 1 : 0);
        return result;
    }

    const PrStage1SaveUiCardImageDurableReadIngress8007A318 imported =
        PrStage1SaveUiDirect::
            ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
                image.data(), image.size(),
                kDefaultSlotPolicyBlock8007A318);
    result.sinkImported = imported.sinkCommitted;
    result.blockIndex = imported.blockIndex;
    RefreshPrimaryCardMediaSnapshot8007A318();
    Log::Printf(
        "SS0 direct card storage: startup image path=%s bytes=%zu imported=%d block=%d",
        path.string().c_str(), image.size(),
        result.sinkImported ? 1 : 0, result.blockIndex);
    return result;
}

CardMediaChangePoll8007A318 PollPrimaryCardMediaChange8007A318() {
    CardMediaChangePoll8007A318 result{};
    result.sourceInstalled = true;
    const CardMediaSnapshot8007A318 current =
        CapturePrimaryCardMediaSnapshot8007A318();
    result.observationKnown =
        current.present || s_cardMediaSnapshot8007A318.initialized;
    if (!s_cardMediaSnapshot8007A318.initialized) {
        s_cardMediaSnapshot8007A318 = current;
        s_cardMediaSnapshot8007A318.initialized = true;
        return result;
    }
    if (SamePrimaryCardMediaSnapshot8007A318(
            s_cardMediaSnapshot8007A318, current)) {
        return result;
    }
    result.changed = true;
    if (!s_cardMediaSnapshot8007A318.present && current.present) {
        result.kind = CardMediaChangeKind8007A318::Inserted;
    } else if (s_cardMediaSnapshot8007A318.present && !current.present) {
        result.kind = CardMediaChangeKind8007A318::Removed;
    } else {
        result.kind = CardMediaChangeKind8007A318::Replaced;
    }
    s_cardMediaSnapshot8007A318 = current;
    s_cardMediaSnapshot8007A318.initialized = true;
    return result;
}

BiosCardResetProbe80047EE4 ProbePrimaryCardBiosReset80047EE4() {
    BiosCardResetProbe80047EE4 result{};
    result.sourceInstalled = true;
    const std::filesystem::path path = ResolveReadableCardPath();
    result.observationKnown = true;
    if (path.empty()) {
        result.cardWriteResultKnown = true;
        result.cardWriteResult = -1;
        return result;
    }

    result.mediaPresent = true;
    std::array<uint8_t, kCardImageBytes8007A318> image{};
    result.imageReadable = ReadExactCardImage(path, image) &&
                            IsCardImageHeaderKnown(image.data(), image.size());

    // B0:0x4E is a card-device handshake in this caller, not a SaveUi image
    // commit. Opening the existing media read/write verifies the host-side
    // device capability without mutating sector 63 or the durable image.
    std::fstream device(path, std::ios::in | std::ios::out | std::ios::binary);
    result.writable = device.is_open();
    result.cardWriteResultKnown = true;
    result.cardWriteResult = result.imageReadable && result.writable ? 0 : -1;
    return result;
}

void RefreshPrimaryCardMediaSnapshot8007A318() {
    s_cardMediaSnapshot8007A318 =
        CapturePrimaryCardMediaSnapshot8007A318();
    s_cardMediaSnapshot8007A318.initialized = true;
}

} // namespace PrSS0CardImageStorageDirect
