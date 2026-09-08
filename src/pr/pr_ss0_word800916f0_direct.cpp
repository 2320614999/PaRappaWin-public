#include "pr_ss0_word800916f0_direct.h"

#include "logger.h"

namespace PrSS0Scene0RuntimeDirect {
namespace {

struct Word800916F0RuntimeState {
    bool known = false;
    uint16_t value = 0u;
    int acceptedCount = 0;
    int runtimeObservationAcceptedCount = 0;
    int startupObservationAcceptedCount = 0;
    int rejectedCount = 0;
    Word800916F0ObservationRejectReason lastRejectReason =
        Word800916F0ObservationRejectReason::None;
    Word800916F0InitialObservationSource lastStartupSource =
        Word800916F0InitialObservationSource::Unknown;
};

Word800916F0RuntimeState s_word800916F0;
Word800916F0SoftwareState s_word800916F0Software;
constexpr uint32_t kWord800916F0PsxAddress = 0x800916F0u;

const char* Word800916F0ObservationSourceName(
    Word800916F0ObservationSource source) {
    switch (source) {
    case Word800916F0ObservationSource::RuntimePsxMemoryObservation:
        return "runtime-psx-memory-observation";
    case Word800916F0ObservationSource::RuntimePsxConsumerReadObservation:
        return "runtime-psx-consumer-read-observation";
    case Word800916F0ObservationSource::Unknown:
    default:
        return "unknown";
    }
}

const char* Word800916F0InitialObservationSourceName(
    Word800916F0InitialObservationSource source) {
    switch (source) {
    case Word800916F0InitialObservationSource::DuckStationResetInjectBootExe:
        return "duckstation-reset-inject-boot-exe";
    case Word800916F0InitialObservationSource::DuckStationDiscBoot:
        return "duckstation-disc-boot";
    case Word800916F0InitialObservationSource::PsxrecFrameSnapshot:
        return "psxrec-frame-snapshot";
    case Word800916F0InitialObservationSource::RuntimePsxConsumerRead:
        return "runtime-psx-consumer-read";
    case Word800916F0InitialObservationSource::Unknown:
    default:
        return "unknown";
    }
}

const char* Word800916F0ObservationProvenanceName(
    Word800916F0InitialObservationSource startupSource) {
    return startupSource == Word800916F0InitialObservationSource::Unknown
               ? "runtime"
               : "startup";
}

bool RangeCoversAddress(uint32_t start, uint32_t end, uint32_t address) {
    return start <= address && address < end;
}

bool AcceptWord800916F0Observation(
    const Word800916F0Observation& observation,
    Word800916F0InitialObservationSource startupSource) {
    if (observation.source !=
        Word800916F0ObservationSource::RuntimePsxMemoryObservation) {
        ++s_word800916F0.rejectedCount;
        s_word800916F0.lastRejectReason =
            Word800916F0ObservationRejectReason::
                NonRuntimePsxMemoryObservation;
        return false;
    }
    if (observation.psxAddress != 0x800916F0u) {
        ++s_word800916F0.rejectedCount;
        s_word800916F0.lastRejectReason =
            Word800916F0ObservationRejectReason::WrongPsxAddress;
        return false;
    }
    if (observation.byteSize != 2u) {
        ++s_word800916F0.rejectedCount;
        s_word800916F0.lastRejectReason =
            Word800916F0ObservationRejectReason::WrongByteSize;
        return false;
    }
    if (!observation.valueKnown) {
        ++s_word800916F0.rejectedCount;
        s_word800916F0.lastRejectReason =
            Word800916F0ObservationRejectReason::UnknownValue;
        return false;
    }

    s_word800916F0.value = observation.value;
    s_word800916F0.known = true;
    ++s_word800916F0.acceptedCount;
    if (startupSource == Word800916F0InitialObservationSource::Unknown) {
        ++s_word800916F0.runtimeObservationAcceptedCount;
    } else {
        ++s_word800916F0.startupObservationAcceptedCount;
        s_word800916F0.lastStartupSource = startupSource;
    }
    s_word800916F0.lastRejectReason =
        Word800916F0ObservationRejectReason::None;
    Log::Printf(
        "SS0 direct runtime: word_800916F0 observation accepted source=%s provenance=%s value=%u frameKnown=%d frame=%u pcKnown=%d pc=0x%08X startupSource=%s",
        Word800916F0ObservationSourceName(observation.source),
        Word800916F0ObservationProvenanceName(startupSource),
        static_cast<unsigned>(observation.value),
        observation.frameKnown ? 1 : 0,
        observation.frame,
        observation.pcKnown ? 1 : 0,
        observation.pc,
        Word800916F0InitialObservationSourceName(startupSource));
    return true;
}

} // namespace

void InitializeWord800916F0FromProcessStartupZero80028590() {
    // 800285A0: sw zero,0(v0), advancing by 4 in [8006ECB8,801C3870).
    // Project this word of the native BSS into typed software state; this is
    // not a claim about physical RAM, aliases or an external observation.
    static_assert(kWord800916F0PsxAddress >= 0x8006ECB8u);
    static_assert(kWord800916F0PsxAddress + sizeof(uint32_t) <= 0x801C3870u);
    static_assert((kWord800916F0PsxAddress - 0x8006ECB8u) % sizeof(uint32_t) == 0u);
    if (s_word800916F0Software.initialized) {
        return;
    }
    s_word800916F0Software.value = 0u;
    s_word800916F0Software.initialized = true;
    ++s_word800916F0Software.startupCommitCount;
    Log::Printf(
        "SS0 native software: word_800916F0 initialized value=0 source=80028590 store=800285A0 range=[8006ECB8,801C3870) externalObservationPublished=0");
}

Word800916F0SoftwareState GetWord800916F0SoftwareState() {
    return s_word800916F0Software;
}

bool TryReadWord800916F0Software(uint16_t& value) {
    if (!s_word800916F0Software.initialized) {
        return false;
    }
    value = s_word800916F0Software.value;
    return true;
}

void ClearWord800916F0RuntimeObservation() {
    s_word800916F0.known = false;
    s_word800916F0.value = 0u;
    s_word800916F0.acceptedCount = 0;
    s_word800916F0.runtimeObservationAcceptedCount = 0;
    s_word800916F0.startupObservationAcceptedCount = 0;
    s_word800916F0.rejectedCount = 0;
    s_word800916F0.lastRejectReason =
        Word800916F0ObservationRejectReason::None;
    s_word800916F0.lastStartupSource =
        Word800916F0InitialObservationSource::Unknown;
}

uint16_t GetWord800916F0() {
    return s_word800916F0.value;
}

bool IsWord800916F0Known() {
    return s_word800916F0.known;
}

bool IsWord800916F0RuntimeObservationKnown() {
    return s_word800916F0.known &&
           s_word800916F0.runtimeObservationAcceptedCount > 0;
}

Word800916F0ObservationDebugSnapshot
GetWord800916F0ObservationDebugSnapshot() {
    Word800916F0ObservationDebugSnapshot snapshot{};
    snapshot.known = s_word800916F0.known;
    snapshot.acceptedCount = s_word800916F0.acceptedCount;
    snapshot.runtimeObservationAcceptedCount =
        s_word800916F0.runtimeObservationAcceptedCount;
    snapshot.startupObservationAcceptedCount =
        s_word800916F0.startupObservationAcceptedCount;
    snapshot.rejectedCount = s_word800916F0.rejectedCount;
    snapshot.lastRejectReason = s_word800916F0.lastRejectReason;
    snapshot.lastStartupSource = s_word800916F0.lastStartupSource;
    return snapshot;
}

bool ApplyWord800916F0RuntimeObservation(
    const Word800916F0Observation& observation) {
    return AcceptWord800916F0Observation(
        observation,
        Word800916F0InitialObservationSource::Unknown);
}

bool PublishWord800916F0FromRuntimePsxMemoryProviderRead(
    const Word800916F0RuntimePsxMemoryProviderRead& read) {
    if (!read.attempted || !read.readable ||
        read.psxAddress != kWord800916F0PsxAddress || read.byteSize != 2u) {
        return false;
    }

    Word800916F0Observation observation{};
    observation.source =
        Word800916F0ObservationSource::RuntimePsxMemoryObservation;
    observation.psxAddress = kWord800916F0PsxAddress;
    observation.byteSize = 2u;
    observation.valueKnown = true;
    observation.value =
        static_cast<uint16_t>(read.bytes[0]) |
        static_cast<uint16_t>(static_cast<uint16_t>(read.bytes[1]) << 8);
    observation.frameKnown = read.frameKnown;
    observation.frame = read.frame;
    observation.pcKnown = read.pcKnown;
    observation.pc = read.pc;
    return ApplyWord800916F0RuntimeObservation(observation);
}

bool PublishInitialWord800916F0FromStartupSource(
    const Word800916F0InitialObservationInput& input) {
    if (s_word800916F0.known) {
        return false;
    }
    if (input.source ==
            Word800916F0InitialObservationSource::PsxrecFrameSnapshot ||
        input.source ==
            Word800916F0InitialObservationSource::RuntimePsxConsumerRead ||
        input.source == Word800916F0InitialObservationSource::Unknown) {
        return false;
    }
    if (!input.ramResetZeroKnown || !input.psExeLoadRangeKnown ||
        !input.psExeMemfillRangeKnown || !input.targetAddressKnown ||
        input.targetAddress != kWord800916F0PsxAddress) {
        return false;
    }
    if (RangeCoversAddress(input.psExeLoadStart,
                           input.psExeLoadEnd,
                           kWord800916F0PsxAddress) ||
        RangeCoversAddress(input.psExeMemfillStart,
                           input.psExeMemfillEnd,
                           kWord800916F0PsxAddress)) {
        return false;
    }
    if (input.source == Word800916F0InitialObservationSource::DuckStationDiscBoot &&
        !input.discBootBiosNonWriterKnown) {
        return false;
    }
    if (input.source !=
            Word800916F0InitialObservationSource::DuckStationResetInjectBootExe &&
        input.source != Word800916F0InitialObservationSource::DuckStationDiscBoot) {
        return false;
    }

    Word800916F0Observation observation{};
    observation.source =
        Word800916F0ObservationSource::RuntimePsxMemoryObservation;
    observation.psxAddress = kWord800916F0PsxAddress;
    observation.byteSize = 2u;
    observation.valueKnown = true;
    observation.value = 0u;
    return AcceptWord800916F0Observation(observation, input.source);
}

bool PublishInitialWord800916F0FromDiscFullbootStartupSource() {
    Word800916F0InitialObservationInput input{};
    input.source = Word800916F0InitialObservationSource::DuckStationDiscBoot;
    input.ramResetZeroKnown = true;
    input.psExeLoadRangeKnown = true;
    input.psExeLoadStart = 0x80010000u;
    input.psExeLoadEnd = 0x8006F000u;
    input.psExeMemfillRangeKnown = true;
    input.psExeMemfillStart = 0u;
    input.psExeMemfillEnd = 0u;
    input.targetAddressKnown = true;
    input.targetAddress = kWord800916F0PsxAddress;
    input.discBootBiosNonWriterKnown = true;
    return PublishInitialWord800916F0FromStartupSource(input);
}

} // namespace PrSS0Scene0RuntimeDirect
