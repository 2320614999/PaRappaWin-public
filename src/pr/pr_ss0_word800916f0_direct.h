#pragma once

#include <cstdint>

namespace PrSS0Scene0RuntimeDirect {

// Native software word owned by the translated process startup, independent
// of the external-memory observation/debug channel below.
struct Word800916F0SoftwareState {
    bool initialized = false;
    uint16_t value = 0u;
    uint32_t startupCommitCount = 0u;
};

void InitializeWord800916F0FromProcessStartupZero80028590();
Word800916F0SoftwareState GetWord800916F0SoftwareState();
bool TryReadWord800916F0Software(uint16_t& value);

enum class Word800916F0ObservationSource : uint8_t {
    Unknown = 0,
    RuntimePsxMemoryObservation = 1,
    RuntimePsxConsumerReadObservation = 2,
};

enum class Word800916F0InitialObservationSource : uint8_t {
    Unknown = 0,
    DuckStationResetInjectBootExe = 1,
    DuckStationDiscBoot = 2,
    PsxrecFrameSnapshot = 3,
    RuntimePsxConsumerRead = 4,
};

struct Word800916F0Observation {
    Word800916F0ObservationSource source =
        Word800916F0ObservationSource::Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint16_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class Word800916F0ObservationRejectReason : uint8_t {
    None = 0,
    NonRuntimePsxMemoryObservation = 1,
    WrongPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
};

struct Word800916F0ObservationDebugSnapshot {
    bool known = false;
    int acceptedCount = 0;
    int runtimeObservationAcceptedCount = 0;
    int startupObservationAcceptedCount = 0;
    int rejectedCount = 0;
    Word800916F0ObservationRejectReason lastRejectReason =
        Word800916F0ObservationRejectReason::None;
    Word800916F0InitialObservationSource lastStartupSource =
        Word800916F0InitialObservationSource::Unknown;
};

struct Word800916F0RuntimePsxMemoryProviderRead {
    bool attempted = false;
    bool readable = false;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    uint8_t bytes[2] = {};
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

struct Word800916F0InitialObservationInput {
    Word800916F0InitialObservationSource source =
        Word800916F0InitialObservationSource::Unknown;
    bool ramResetZeroKnown = false;
    bool psExeLoadRangeKnown = false;
    uint32_t psExeLoadStart = 0u;
    uint32_t psExeLoadEnd = 0u;
    bool psExeMemfillRangeKnown = false;
    uint32_t psExeMemfillStart = 0u;
    uint32_t psExeMemfillEnd = 0u;
    bool targetAddressKnown = false;
    uint32_t targetAddress = 0u;
    bool discBootBiosNonWriterKnown = false;
};

void ClearWord800916F0RuntimeObservation();
uint16_t GetWord800916F0();
bool IsWord800916F0Known();
bool IsWord800916F0RuntimeObservationKnown();
Word800916F0ObservationDebugSnapshot
GetWord800916F0ObservationDebugSnapshot();
bool ApplyWord800916F0RuntimeObservation(
    const Word800916F0Observation& observation);
bool PublishWord800916F0FromRuntimePsxMemoryProviderRead(
    const Word800916F0RuntimePsxMemoryProviderRead& read);
bool PublishInitialWord800916F0FromStartupSource(
    const Word800916F0InitialObservationInput& input);
bool PublishInitialWord800916F0FromDiscFullbootStartupSource();

} // namespace PrSS0Scene0RuntimeDirect
