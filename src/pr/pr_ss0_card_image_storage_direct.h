#pragma once

#include <cstdint>

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
CardMediaChangePoll8007A318 PollPrimaryCardMediaChange8007A318();
BiosCardResetProbe80047EE4 ProbePrimaryCardBiosReset80047EE4();
void RefreshPrimaryCardMediaSnapshot8007A318();

} // namespace PrSS0CardImageStorageDirect
