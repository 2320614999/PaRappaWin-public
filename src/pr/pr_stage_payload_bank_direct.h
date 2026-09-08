#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

struct PrStage1ScorerDirectReplayBufferState;

namespace PrStage1SaveCardHalDirect {
struct LoadSavePayloadAuthorityAccess800164B4;
}

namespace PrStagePayloadBankDirect {

constexpr uint32_t kFn80015CC4 = 0x80015CC4u;
constexpr uint32_t kFn80015744 = 0x80015744u;
constexpr uint32_t kFn8001615C = 0x8001615Cu;
constexpr uint32_t kFn800161A8 = 0x800161A8u;
constexpr uint32_t kFn800161F4 = 0x800161F4u;
constexpr uint32_t kFn8001628C = 0x8001628Cu;
constexpr uint32_t kFn8001635C = 0x8001635Cu;
constexpr uint32_t kFn800164B4 = 0x800164B4u;
constexpr uint32_t kFn800166AC = 0x800166ACu;
constexpr uint32_t kFn800167A8 = 0x800167A8u;
constexpr uint32_t kFn800169E0 = 0x800169E0u;
constexpr uint32_t kFn800185D0 = 0x800185D0u;
constexpr uint32_t kReplayMirrorProducerFn801C8660 = 0x801C8660u;
constexpr uint32_t kReplayMirrorProducerFn80014614 = 0x80014614u;
constexpr uint32_t kReplayMirrorProducerFn8001681C = 0x8001681Cu;

constexpr uint32_t kBaseAddress80092F10 = 0x80092F10u;
constexpr uint32_t kByteCount80092F10 = 4876u;
constexpr uint32_t kStatusBaseAddress80092F1D =
    kBaseAddress80092F10 + 0x0Du;
constexpr uint32_t kScoreBaseAddress80092F24 =
    kBaseAddress80092F10 + 0x14u;
constexpr uint32_t kCarrierIndexAddress80092F3C =
    kBaseAddress80092F10 + 0x2Cu;
constexpr uint32_t kCarrierA3Address80092F40 =
    kBaseAddress80092F10 + 0x30u;
constexpr uint32_t kCarrierCompleteAddress80092F44 =
    kBaseAddress80092F10 + 0x34u;
constexpr uint32_t kCarrierSourceAddress80092F48 =
    kBaseAddress80092F10 + 0x38u;
constexpr uint32_t kMirrorDstAddress80092F5C =
    kBaseAddress80092F10 + 0x4Cu;
constexpr uint32_t kMirrorSrcAddress8008EEF8 = 0x8008EEF8u;
constexpr uint32_t kTypedPayloadSourceAddress8007ADE8 = 0x8007ADE8u;
constexpr uint32_t kMapSaveStageTable80048DB8 = 0x80048DB8u;
constexpr uint32_t kMapSceneTable80048DD8 = 0x80048DD8u;
constexpr int32_t kMirrorBytes8001635C = 4800;
constexpr int32_t kStatusSlotCount800161F4 = 6;

// Authorizes the coupled replay-restore payload fields: published count at
// `80092F48` and the 4800-byte record backing at `80092F5C`.
enum class ReplayPayloadBackingProvenance80092F5C : uint8_t {
    Unknown = 0,
    TypedCardCopy800164B4 = 1,
    ReplayMirrorCopy8001635C = 2,
};

bool IsKnownReplayRestorePayloadProvenance80092F48_80092F5C(
    ReplayPayloadBackingProvenance80092F5C provenance);

struct ReplayPayloadBackingAuthorityAccess80092F5C;

class ReplayPayloadBackingAuthority80092F5C {
public:
    ReplayPayloadBackingAuthority80092F5C() = default;

    ReplayPayloadBackingProvenance80092F5C Kind() const {
        return kind_;
    }

    bool MatchesSeal(ReplayPayloadBackingProvenance80092F5C kind,
                     uint64_t seal) const {
        return kind_ == kind && seal_ == seal;
    }

private:
    void Mint(ReplayPayloadBackingProvenance80092F5C kind, uint64_t seal) {
        kind_ = kind;
        seal_ = seal;
    }

    ReplayPayloadBackingProvenance80092F5C kind_ =
        ReplayPayloadBackingProvenance80092F5C::Unknown;
    uint64_t seal_ = 0u;

    friend struct ReplayPayloadBackingAuthorityAccess80092F5C;
};

enum class ReplayMirrorSourceAuthorityKind8008EEF8 : uint8_t {
    Unknown = 0,
    StartupZero80028590,
    TranslatedProducer,
};

struct ReplayMirrorSourceAuthorityAccess8008EEF8;

class ReplayMirrorSourceAuthority8008EEF8 {
public:
    ReplayMirrorSourceAuthority8008EEF8() = default;

    ReplayMirrorSourceAuthorityKind8008EEF8 Kind() const {
        return kind_;
    }

    bool MatchesSeal(ReplayMirrorSourceAuthorityKind8008EEF8 kind,
                     uint64_t seal) const {
        return kind_ == kind && seal_ == seal;
    }

private:
    void Mint(ReplayMirrorSourceAuthorityKind8008EEF8 kind, uint64_t seal) {
        kind_ = kind;
        seal_ = seal;
    }

    ReplayMirrorSourceAuthorityKind8008EEF8 kind_ =
        ReplayMirrorSourceAuthorityKind8008EEF8::Unknown;
    uint64_t seal_ = 0u;

    friend struct ReplayMirrorSourceAuthorityAccess8008EEF8;
};

struct MapResult8001615C {
    bool mapped = false;
    int32_t input = 0;
    int32_t mappedIndex = -1;
    uint32_t tableAddress = kMapSaveStageTable80048DB8;
};

struct MapSceneResult800161A8 {
    bool mapped = false;
    int32_t input = 0;
    int32_t sceneId = -1;
    uint32_t tableAddress = kMapSceneTable80048DD8;
};

struct MemoryState80092F10 {
    ReplayMirrorSourceAuthority8008EEF8 replayMirrorSourceAuthority{};
    std::array<uint8_t, kMirrorBytes8001635C> replayMirror{};
    std::array<uint8_t, kByteCount80092F10> savePayloadBank{};
    std::array<uint8_t, kByteCount80092F10> saveStatusBackup{};
    bool replayMirrorKnown8008EEF8 = false;
    bool replayMirrorStartupZeroAuthorityKnown80028590 = false;
    bool replayMirrorProducerKnown8008EEF8 = false;
    uint32_t replayMirrorProducerFunction = 0;
    bool replayMirrorByteCountKnown8008EEF8 = false;
    uint32_t replayMirrorKnownByteCount8008EEF8 = 0;
    bool replayMirrorFullBackingKnown8008EEF8 = false;
    bool replayMirrorPublishedCountKnown800901BC = false;
    uint32_t replayMirrorPublishedCount800901BC = 0;
    bool savePayloadBankKnown = false;
    bool statusBankKnown80092F1D = false;
    ReplayPayloadBackingAuthority80092F5C
        replayPayloadBackingAuthority80092F5C{};
    ReplayPayloadBackingProvenance80092F5C
        replayPayloadBackingProvenance80092F5C =
            ReplayPayloadBackingProvenance80092F5C::Unknown;
    bool saveStatusBackupKnown80079008 = false;
    bool saveStatusBackupStatusBankKnown80092F1D = false;
    ReplayPayloadBackingProvenance80092F5C
        saveStatusBackupReplayPayloadBackingProvenance80092F5C =
            ReplayPayloadBackingProvenance80092F5C::Unknown;
    ReplayPayloadBackingAuthority80092F5C
        saveStatusBackupReplayPayloadBackingAuthority80092F5C{};
    uint32_t savePayloadBankLastWriterFunction = 0;
    uint32_t saveStatusSeedAuthorityFunction = 0;
    bool wrote80015CC4 = false;
    bool wrote800164B4 = false;
    bool wrote8001635C = false;
    bool wrote8001628C = false;
    bool wrote800167A8 = false;
    bool wrote80015744 = false;
    bool boundsFault = false;
    uint32_t lastFaultAddress = 0;
};

inline uint64_t ReplayMirrorSourceAuthorityMix8008EEF8(uint64_t hash,
                                                       uint64_t value) {
    constexpr uint64_t kPrime = 1099511628211ull;
    for (unsigned shift = 0; shift < 64u; shift += 8u) {
        hash ^= (value >> shift) & 0xFFu;
        hash *= kPrime;
    }
    return hash;
}

inline uint64_t ReplayMirrorSourceAuthoritySeal8008EEF8(
    const MemoryState80092F10& state,
    ReplayMirrorSourceAuthorityKind8008EEF8 kind) {
    uint64_t hash = 1469598103934665603ull;
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, static_cast<uint8_t>(kind));
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorKnown8008EEF8 ? 1u : 0u);
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorStartupZeroAuthorityKnown80028590 ? 1u : 0u);
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorProducerKnown8008EEF8 ? 1u : 0u);
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorProducerFunction);
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorByteCountKnown8008EEF8 ? 1u : 0u);
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorKnownByteCount8008EEF8);
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorFullBackingKnown8008EEF8 ? 1u : 0u);
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorPublishedCountKnown800901BC ? 1u : 0u);
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, state.replayMirrorPublishedCount800901BC);
    for (uint8_t value : state.replayMirror) {
        hash ^= value;
        hash *= 1099511628211ull;
    }
    return hash;
}

inline bool ReplayMirrorSourceAuthorityMatchesState8008EEF8(
    const MemoryState80092F10& state) {
    const auto kind = state.replayMirrorSourceAuthority.Kind();
    if (kind == ReplayMirrorSourceAuthorityKind8008EEF8::Unknown) {
        return false;
    }
    return state.replayMirrorSourceAuthority.MatchesSeal(
        kind, ReplayMirrorSourceAuthoritySeal8008EEF8(state, kind));
}

inline uint64_t ReplayPayloadBackingAuthoritySeal80092F5C(
    const MemoryState80092F10& state,
    ReplayPayloadBackingProvenance80092F5C kind) {
    uint64_t hash = 1469598103934665603ull;
    hash = ReplayMirrorSourceAuthorityMix8008EEF8(
        hash, static_cast<uint8_t>(kind));
    constexpr size_t kCountOffset =
        kCarrierSourceAddress80092F48 - kBaseAddress80092F10;
    constexpr size_t kMirrorOffset =
        kMirrorDstAddress80092F5C - kBaseAddress80092F10;
    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
        hash ^= state.savePayloadBank[kCountOffset + i];
        hash *= 1099511628211ull;
    }
    for (size_t i = 0; i < static_cast<size_t>(kMirrorBytes8001635C); ++i) {
        hash ^= state.savePayloadBank[kMirrorOffset + i];
        hash *= 1099511628211ull;
    }
    return hash;
}

inline bool ReplayPayloadBackingAuthorityMatchesState80092F5C(
    const MemoryState80092F10& state) {
    const auto kind = state.replayPayloadBackingAuthority80092F5C.Kind();
    if (!IsKnownReplayRestorePayloadProvenance80092F48_80092F5C(kind) ||
        state.replayPayloadBackingProvenance80092F5C != kind) {
        return false;
    }
    return state.replayPayloadBackingAuthority80092F5C.MatchesSeal(
        kind, ReplayPayloadBackingAuthoritySeal80092F5C(state, kind));
}

bool IsKnownReplayMirrorProducerFunction8001635C(uint32_t psxFunction);
bool ImportAuthoritativeReplayMirror8008EEF8(
    MemoryState80092F10& state,
    const PrStage1ScorerDirectReplayBufferState& replay);
void ClearReplayMirrorSourceAuthority8008EEF8(MemoryState80092F10& state);

struct AllStatusQueryResult800161F4 {
    bool ok = false;
    bool statusBankKnown = false;
    bool statusBytesKnown80092F1D = false;
    uint8_t byte80092F1D[6]{};
    uint32_t result = 0;
};

struct StatusQueryResult800166AC {
    bool ok = false;
    bool statusBankKnown = false;
    bool mapped = false;
    int32_t sceneId = 0;
    int32_t slotIndex = -1;
    uint8_t status = 0;
};

struct EnsureProgressResult8001628C {
    bool ok = false;
    bool payloadKnown = false;
    bool statusBankKnown = false;
    bool mapped = false;
    int32_t sceneId = 0;
    int32_t slotIndex = -1;
    int32_t result = 0;
    uint32_t lastFaultAddress = 0;
};

struct InitSavePayloadResult80015CC4 {
    bool ok = false;
    bool payloadKnown = false;
    bool statusBankKnown = false;
    uint32_t lastFaultAddress = 0;
};

struct LoadSavePayloadResult800164B4 {
    bool ok = false;
    bool payloadKnown = false;
    bool statusBankKnown = false;
    bool sourceAuthorityKnown = false;
    uint32_t srcAddress = 0;
    size_t sourceBytes = 0;
    uint32_t lastFaultAddress = 0;
};

class LoadSavePayloadAuthority800164B4 {
public:
    LoadSavePayloadAuthority800164B4() = default;

    bool KnownForAddress(uint32_t payloadAddress) const {
        return known_ &&
               payloadAddress == kTypedPayloadSourceAddress8007ADE8 &&
               payloadAddress_ == payloadAddress;
    }

    bool Matches(uint32_t payloadAddress,
                 const uint8_t* payload,
                 size_t payloadBytes) const {
        return known_ && payload &&
               payloadAddress == kTypedPayloadSourceAddress8007ADE8 &&
               payloadAddress_ == payloadAddress &&
               payloadBytes == kByteCount80092F10 &&
               payloadBytes_ == payloadBytes &&
               payloadHash_ == Hash(payload, payloadBytes);
    }

private:
    static uint64_t Hash(const uint8_t* payload, size_t payloadBytes) {
        uint64_t hash = 1469598103934665603ull;
        if (!payload) {
            return 0u;
        }
        for (size_t i = 0; i < payloadBytes; ++i) {
            hash ^= payload[i];
            hash *= 1099511628211ull;
        }
        return hash;
    }

    void Mint(uint32_t payloadAddress,
              const uint8_t* payload,
              size_t payloadBytes) {
        known_ = payload &&
                 payloadAddress == kTypedPayloadSourceAddress8007ADE8 &&
                 payloadBytes == kByteCount80092F10;
        payloadAddress_ = known_ ? payloadAddress : 0u;
        payloadBytes_ = known_ ? payloadBytes : 0u;
        payloadHash_ = known_ ? Hash(payload, payloadBytes) : 0u;
    }

    bool known_ = false;
    uint32_t payloadAddress_ = 0u;
    size_t payloadBytes_ = 0u;
    uint64_t payloadHash_ = 0u;

    friend struct PrStage1SaveCardHalDirect::
        LoadSavePayloadAuthorityAccess800164B4;
};

struct UpdateSavePayloadResult8001635C {
    bool ok = false;
    bool payloadKnown = false;
    bool statusBankKnown = false;
    bool mapped = false;
    bool preflightPayloadKnown = false;
    bool preflightStatusBankKnown = false;
    bool preflightMapped = false;
    bool preflightCarrierSourceKnown = false;
    bool preflightCarrierSourceMatchesReplayAuthority = false;
    bool preflightMirrorSourceKnown = false;
    bool preflightReplayMirrorProducerSourceKnown = false;
    bool preflightStartupZeroSourceKnown = false;
    bool scratchAuthorityKnown = false;
    bool carrierSourceKnown = false;
    bool mirrorSourceKnown = false;
    bool mirrorCopied = false;
    bool allClearQueried = false;
    bool allClearWritten = false;
    int32_t sceneId = 0;
    int32_t targetStatus = 0;
    int32_t carrierA3Input = 0;
    int32_t scoreValue = 0;
    int32_t slotIndex = -1;
    uint32_t result = 0;
    uint32_t carrierA3 = 0;
    uint32_t preflightCarrierSource = 0;
    uint32_t carrierSource = 0;
    uint32_t lastFaultAddress = 0;
};

struct StatusWriteResult800167A8 {
    bool ok = false;
    bool statusBankKnown = false;
    bool mapped = false;
    int32_t sceneId = 0;
    int32_t a2 = 0;
    int32_t slotIndex = -1;
    uint8_t status = 0;
    uint32_t lastFaultAddress = 0;
};

struct SavedScoreSyncResult800169E0 {
    bool ok = false;
    bool applied = false;
    bool statusBankKnown = false;
    bool mapped = false;
    int32_t word800916D0 = 0;
    int32_t word800916E2 = 0;
    int32_t slotIndex = -1;
    bool scoreDwordReadAttempted = false;
    bool scoreDwordKnown = false;
    uint32_t ctxScoreDword = 0;
    uint16_t word80091816 = 0;
    uint32_t lastFaultAddress = 0;
};

struct StatusBankSnapshot80092F10 {
    bool statusBytesKnown80092F1D = false;
    uint8_t byte80092F1D[6]{};
    bool scoreDwordsKnown80092F24 = false;
    uint32_t dword80092F24[6]{};
    bool lastSavedSlotKnown80092F3C = false;
    uint32_t dword80092F3C = 0;
    bool allClearKnown80092F44 = false;
    uint32_t dword80092F44 = 0;
};

struct SaveStatusPrefixSnapshot80092F10 {
    static constexpr uint32_t kPsxAddress = kBaseAddress80092F10;
    static constexpr uint32_t kByteCount = kByteCount80092F10;

    bool known = false;
    bool statusBankKnown80092F1D = false;
    ReplayPayloadBackingProvenance80092F5C
        replayPayloadBackingProvenance80092F5C =
            ReplayPayloadBackingProvenance80092F5C::Unknown;
    uint32_t psxAddress = kPsxAddress;
    uint32_t byteCount = kByteCount;
    uint32_t lastWriterFunction = 0;
    uint32_t seedAuthorityFunction = 0;
    uint32_t lastFaultAddress = 0;
    bool wrote80015CC4 = false;
    bool wrote800164B4 = false;
    bool wrote8001635C = false;
    bool wrote8001628C = false;
    bool wrote800167A8 = false;
    bool wrote80015744 = false;
    uint8_t bytes[kByteCount]{};
};

MapResult8001615C MapSaveStage8001615C(int32_t a1);
MapSceneResult800161A8 MapScene800161A8(int32_t a1);
int32_t AllStatusesClear800161F4(const uint8_t* bytes, size_t count);
void MarkPayloadWriter80092F10(MemoryState80092F10& state,
                               uint32_t function);
uint8_t* DirectMemoryPtr80092F10(MemoryState80092F10& state,
                                 uint32_t address,
                                 size_t count);
const uint8_t* DirectMemoryReadPtr80092F10(
    MemoryState80092F10& state,
    uint32_t address,
    size_t count);
AllStatusQueryResult800161F4 QueryAllStatuses800161F4(
    MemoryState80092F10& state);
StatusQueryResult800166AC QueryStatus800166AC(MemoryState80092F10& state,
                                               int32_t a1);
EnsureProgressResult8001628C EnsureProgress8001628C(
    MemoryState80092F10& state,
    int32_t a1);
InitSavePayloadResult80015CC4 InitSavePayload80015CC4(
    MemoryState80092F10& state);
LoadSavePayloadResult800164B4 LoadSavePayload800164B4(
    MemoryState80092F10& state,
    uint32_t srcAddress,
    const uint8_t* source,
    size_t sourceBytes,
    const LoadSavePayloadAuthority800164B4& authority);
UpdateSavePayloadResult8001635C UpdateSavePayload8001635C(
    MemoryState80092F10& state,
    int32_t a1,
    int32_t a2,
    int32_t a3,
    int32_t a4,
    bool carrierSourceKnown,
    uint32_t carrierSource);
StatusWriteResult800167A8 WriteStatus800167A8(MemoryState80092F10& state,
                                              int32_t a1,
                                              int32_t a2);
SavedScoreSyncResult800169E0 SyncSavedScore800169E0(
    MemoryState80092F10& state,
    int32_t word800916D0,
    int32_t word800916E2);
StatusBankSnapshot80092F10 SnapshotStatusBank80092F10(
    MemoryState80092F10& state);
SaveStatusPrefixSnapshot80092F10 SnapshotSaveStatusPrefix80092F10(
    MemoryState80092F10& state);

} // namespace PrStagePayloadBankDirect
