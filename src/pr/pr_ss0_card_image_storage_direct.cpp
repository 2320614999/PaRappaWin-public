#include "pr_ss0_card_image_storage_direct.h"

#include "logger.h"
#include "pr_stage1_save_ui_direct.h"

#include <windows.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
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

std::filesystem::path ResolveReadableCardPath(const std::filesystem::path& save) {
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

std::filesystem::path ResolveReadableCardPath() {
    return ResolveReadableCardPath(ExecutableDirectory() / L"save");
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

struct NamedCardReadHandle800173A8 {
    std::ifstream file;
};

void CloseNamedCardFileAfterPoll800179B4(DeferredNamedCardRead800173A8& read) {
    if (read.handle) {
        auto& file = read.handle->file;
        file.clear();
        read.receipt.closeAttempted = true;
        file.close();
        read.receipt.closeSucceeded = !file.fail();
        read.handle.reset();
    }
    if (read.logPrimaryReceipt) {
        const auto& r = read.receipt;
        Log::Printf("SS0 card named host read: name=%s request=%d open=%d directory=%d found=%d block=%d read=%d bytes=%zu close=%d/%d psxEventAuthority=0 psxFdAuthority=0",
            read.name.data(), r.requestValid, r.imageOpened, r.directoryValid,
            r.fileFound, r.blockIndex, r.readComplete, r.bytesRead,
            r.closeAttempted, r.closeSucceeded);
        read.logPrimaryReceipt = false;
    }
}

DeferredNamedCardRead800173A8 BeginCardFileBlockAtDirectory800173A8(
    const std::filesystem::path& saveDirectory, const char* name,
    uint8_t* output, std::size_t outputBytes) {
    DeferredNamedCardRead800173A8 pending{};
    auto& result = pending.receipt;
    if (output && outputBytes == 8192u) std::memset(output, 0, outputBytes);
    if (saveDirectory.empty() || !saveDirectory.is_absolute() || !name ||
        !output || outputBytes != 8192u) return pending;
    std::size_t length = 0;
    while (length < 21u && name[length]) {
        if (name[length] == ':' || name[length] == '/' || name[length] == '\\') return pending;
        ++length;
    }
    if (!length || length > 20u) return pending;
    std::memcpy(pending.name.data(), name, length);
    result.requestValid = true;
    // Resolve anew per open, including the existing primary-media fallback
    // policy. Neither directory rows nor payloads come from the live sink.
    const auto path = ResolveReadableCardPath(saveDirectory);
    if (path.empty()) return pending;
    pending.handle = std::make_shared<NamedCardReadHandle800173A8>();
    auto& file = pending.handle->file;
    file.open(path, std::ios::binary);
    if (!file.is_open()) { pending.handle.reset(); return pending; }
    result.imageOpened = true;
    auto finish = [&]() {
        const bool readFailed = !file;
        if (readFailed || !result.readComplete) {
            result.readComplete = false;
            std::memset(output, 0, outputBytes);
        }
        if (!result.fileFound) CloseNamedCardFileAfterPoll800179B4(pending);
        return pending;
    };
    file.seekg(0, std::ios::end);
    if (file.tellg() != static_cast<std::streamoff>(kCardImageBytes8007A318)) return finish();
    file.seekg(0, std::ios::beg);
    std::array<uint8_t, 128> frame{};
    file.read(reinterpret_cast<char*>(frame.data()), frame.size());
    if (!file || frame[0] != 'M' || frame[1] != 'C') return finish();
    result.directoryValid = true;
    for (int32_t physical = 0; physical < 15; ++physical) {
        file.seekg(static_cast<std::streamoff>((physical + 1) * 128), std::ios::beg);
        file.read(reinterpret_cast<char*>(frame.data()), frame.size());
        if (!file) return finish();
        const uint32_t bytes = static_cast<uint32_t>(frame[4]) |
            (static_cast<uint32_t>(frame[5]) << 8) |
            (static_cast<uint32_t>(frame[6]) << 16) |
            (static_cast<uint32_t>(frame[7]) << 24);
        if (frame[0] != 0x51 || bytes != 8192u) continue;
        char candidate[21]{};
        std::memcpy(candidate, frame.data() + 10, 20);
        if (std::strcmp(candidate, name) != 0) continue;
        result.fileFound = true;
        result.blockIndex = physical;
        file.seekg(static_cast<std::streamoff>((physical + 1) * 8192), std::ios::beg);
        file.read(reinterpret_cast<char*>(output), static_cast<std::streamsize>(outputBytes));
        result.bytesRead = static_cast<std::size_t>(file.gcount());
        result.readComplete = file.good() && result.bytesRead == outputBytes;
        return finish();
    }
    return finish();
}

NamedCardBlockRead800173A8 ReadCardFileBlockAtDirectory800173A8(
    const std::filesystem::path& saveDirectory, const char* name,
    uint8_t* output, std::size_t outputBytes) {
    auto pending = BeginCardFileBlockAtDirectory800173A8(saveDirectory, name, output, outputBytes);
    CloseNamedCardFileAfterPoll800179B4(pending);
    return pending.receipt;
}

DeferredNamedCardRead800173A8 BeginPrimaryCardFileBlock800173A8(
    const char* name, uint8_t* output, std::size_t outputBytes, void*) {
    auto pending = BeginCardFileBlockAtDirectory800173A8(
        ExecutableDirectory() / L"save", name, output, outputBytes);
    pending.logPrimaryReceipt = true;
    Log::Printf("SS0 card named host begin: name=%s found=%d retainedHandle=%d closeAttempted=%d",
        pending.name.data(), pending.receipt.fileFound, pending.handle ? 1 : 0,
        pending.receipt.closeAttempted);
    return pending;
}

NamedCardBlockRead800173A8 ReadPrimaryCardFileBlock800173A8(
    const char* name, uint8_t* output, std::size_t outputBytes, void*) {
    const auto result = ReadCardFileBlockAtDirectory800173A8(
        ExecutableDirectory() / L"save", name, output, outputBytes);
    Log::Printf("SS0 card named host read: name=%s request=%d open=%d directory=%d found=%d block=%d read=%d bytes=%zu close=%d/%d psxEventAuthority=0 psxFdAuthority=0",
        name ? name : "", result.requestValid, result.imageOpened, result.directoryValid,
        result.fileFound, result.blockIndex, result.readComplete, result.bytesRead,
        result.closeAttempted, result.closeSucceeded);
    return result;
}

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

StartupBinding8007A318 ReloadCardImageAtDirectory8007A318(
    const std::filesystem::path& saveDirectory,
    int32_t blockIndex) {
    StartupBinding8007A318 result{};
    if (saveDirectory.empty() || !saveDirectory.is_absolute() ||
        blockIndex < 0 || blockIndex >= 15) return result;
    PrStage1SaveUiDirect::InvalidateSaveUiCardImagePersistence8007A318();
    const std::filesystem::path path = ResolveReadableCardPath(saveDirectory);
    result.imageFileFound = !path.empty();
    if (path.empty()) return result;
    std::array<uint8_t, kCardImageBytes8007A318> image{};
    result.imageRead = ReadExactCardImage(path, image);
    result.imageValidated = result.imageRead &&
                            IsCardImageHeaderKnown(image.data(), image.size());
    if (!result.imageValidated) return result;
    const auto imported =
        PrStage1SaveUiDirect::ImportSaveUiCardImagePersistenceSinkFromDirectDurableRead8007A318(
            image.data(), image.size(), blockIndex);
    result.sinkImported = imported.sinkCommitted;
    result.blockIndex = imported.blockIndex;
    return result;
}

StartupBinding8007A318 ReloadPrimaryCardImage8007A318() {
    PrStage1SaveUiDirect::SetSaveUiCardImageDirectDurableCommitBackend8007A318(
        WritePrimaryCardImageAtomically, nullptr);
    const auto result = ReloadCardImageAtDirectory8007A318(
        ExecutableDirectory() / L"save", kDefaultSlotPolicyBlock8007A318);
    RefreshPrimaryCardMediaSnapshot8007A318();
    Log::Printf("SS0 direct card storage: card_load re-read primary found=%d read=%d validated=%d imported=%d block=%d",
        result.imageFileFound ? 1 : 0, result.imageRead ? 1 : 0,
        result.imageValidated ? 1 : 0, result.sinkImported ? 1 : 0,
        result.blockIndex);
    return result;
}

CardMediaReadProbe80017594 ProbeCardMediaAtDirectory80017594(
    const std::filesystem::path& saveDirectory) {
    CardMediaReadProbe80017594 result{};
    if (saveDirectory.empty() || !saveDirectory.is_absolute()) return result;
    result.sourceInstalled = true;
    result.observationKnown = true;
    const auto path = ResolveReadableCardPath(saveDirectory);
    result.mediaPresent = !path.empty();
    if (!result.mediaPresent) return result;
    std::array<uint8_t, kCardImageBytes8007A318> image{};
    result.imageRead = ReadExactCardImage(path, image);
    result.headerValid = result.imageRead &&
        IsCardImageHeaderKnown(image.data(), image.size());
    return result;
}

CardMediaReadProbe80017594 ProbePrimaryCardMedia80017594() {
    return ProbeCardMediaAtDirectory80017594(ExecutableDirectory() / L"save");
}

int32_t ResolveCardInfoEvent80017594(const CardMediaReadProbe80017594& probe) {
    if (!probe.sourceInstalled || !probe.observationKnown) return 0;
    // A completed host read with missing/unreadable media is the error
    // event (third SwCARD handle), not the 80016E18 elapsed-counter timeout.
    return probe.mediaPresent && probe.imageRead ? 1 : 3;
}

int32_t ResolveCardLoadEvent80017594(const StartupBinding8007A318& reload,
                                    bool directoryLoaded) {
    if (reload.imageFileFound && reload.imageRead &&
        reload.imageValidated && reload.sinkImported && directoryLoaded) return 1;
    if (reload.imageFileFound && reload.imageRead && !reload.imageValidated) return 4;
    return 3;
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
    return ProbeCardBiosResetAtDirectory80047EE4(ExecutableDirectory() / L"save");
}

BiosCardResetProbe80047EE4 ProbeCardBiosResetAtDirectory80047EE4(
    const std::filesystem::path& saveDirectory) {
    BiosCardResetProbe80047EE4 result{};
    if (saveDirectory.empty() || !saveDirectory.is_absolute()) return result;
    result.sourceInstalled = true;
    const std::filesystem::path path = ResolveReadableCardPath(saveDirectory);
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
