#pragma once

#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <array>

namespace PrSS0CardImageStorageDirect {

struct StartupBinding8007A318 {
    bool backendBound = false;
    bool existingSinkPreserved = false;
    bool imageFileFound = false;
    bool imageRead = false;
    bool imageValidated = false;
    bool sinkImported = false;
    int32_t blockIndex = -1;
};

// card_info observes device readability; card_load additionally checks the
// directory format. An unformatted, readable card must reach 80017594 state3
// event4 (I/O result5), not be treated as an absent durable save image.
struct CardMediaReadProbe80017594 {
    bool sourceInstalled = false;
    bool observationKnown = false;
    bool mediaPresent = false;
    bool imageRead = false;
    bool headerValid = false;
};

CardMediaReadProbe80017594 ProbeCardMediaAtDirectory80017594(
    const std::filesystem::path& saveDirectory);
CardMediaReadProbe80017594 ProbePrimaryCardMedia80017594();
int32_t ResolveCardInfoEvent80017594(const CardMediaReadProbe80017594& probe);
int32_t ResolveCardLoadEvent80017594(const StartupBinding8007A318& reload,
                                    bool directoryLoaded);

enum class CardMediaChangeKind8007A318 : uint8_t {
    None = 0,
    Inserted,
    Removed,
    Replaced,
};

struct CardMediaChangePoll8007A318 {
    bool sourceInstalled = false;
    bool observationKnown = false;
    bool changed = false;
    CardMediaChangeKind8007A318 kind = CardMediaChangeKind8007A318::None;
};

struct BiosCardResetProbe80047EE4 {
    bool sourceInstalled = false;
    bool observationKnown = false;
    bool mediaPresent = false;
    bool imageReadable = false;
    bool writable = false;
    bool cardWriteResultKnown = false;
    int32_t cardWriteResult = -1;
};

StartupBinding8007A318 BindAndLoadPrimaryCardImage8007A318();
StartupBinding8007A318 ReloadCardImageAtDirectory8007A318(
    const std::filesystem::path& saveDirectory,
    int32_t blockIndex = 0);
StartupBinding8007A318 ReloadPrimaryCardImage8007A318();
// Named PSX one-block file backed by the current image, opened read-only for
// each request. These are actual host I/O receipts, not PSX event/FD facts.
struct NamedCardBlockRead800173A8 {
    bool requestValid = false;
    bool imageOpened = false;
    bool directoryValid = false;
    bool fileFound = false;
    bool readComplete = false;
    bool closeAttempted = false;
    bool closeSucceeded = false;
    int32_t blockIndex = -1;
    std::size_t bytesRead = 0;
};
using NamedCardBlockReader800173A8 = NamedCardBlockRead800173A8 (*)(
    const char*, uint8_t*, std::size_t, void*);
struct NamedCardReadHandle800173A8;
struct DeferredNamedCardRead800173A8 {
    NamedCardBlockRead800173A8 receipt{};
    std::shared_ptr<NamedCardReadHandle800173A8> handle;
    std::array<char, 32> name{};
    bool logPrimaryReceipt = false;
};
using DeferredNamedCardReader800173A8 = DeferredNamedCardRead800173A8 (*)(
    const char*, uint8_t*, std::size_t, void*);
// A successful named open retains its read-only host handle until the caller
// has completed 80016EB8. A failed lookup closes only the container used for
// lookup; it does not invent a successful PSX open or a completion event.
DeferredNamedCardRead800173A8 BeginCardFileBlockAtDirectory800173A8(
    const std::filesystem::path& saveDirectory, const char* name,
    uint8_t* output, std::size_t outputBytes);
DeferredNamedCardRead800173A8 BeginPrimaryCardFileBlock800173A8(
    const char* name, uint8_t* output, std::size_t outputBytes, void*);
void CloseNamedCardFileAfterPoll800179B4(DeferredNamedCardRead800173A8& read);
NamedCardBlockRead800173A8 ReadCardFileBlockAtDirectory800173A8(
    const std::filesystem::path& saveDirectory, const char* name,
    uint8_t* output, std::size_t outputBytes);
NamedCardBlockRead800173A8 ReadPrimaryCardFileBlock800173A8(
    const char* name, uint8_t* output, std::size_t outputBytes, void*);
CardMediaChangePoll8007A318 PollPrimaryCardMediaChange8007A318();
BiosCardResetProbe80047EE4 ProbePrimaryCardBiosReset80047EE4();
// Explicit storage dependency for isolated callers. Empty/relative directories
// are rejected; this never falls back to the executable's real save directory.
BiosCardResetProbe80047EE4 ProbeCardBiosResetAtDirectory80047EE4(
    const std::filesystem::path& saveDirectory);
void RefreshPrimaryCardMediaSnapshot8007A318();

} // namespace PrSS0CardImageStorageDirect
