#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

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
constexpr uint32_t kReplayMirrorProducerFn801C4FC8 = 0x801C4FC8u;
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
    std::array<uint8_t, kMirrorBytes8001635C> replayMirror{};
    std::array<uint8_t, kByteCount80092F10> savePayloadBank{};
    std::array<uint8_t, kByteCount80092F10> saveStatusBackup{};
    bool replayMirrorKnown8008EEF8 = false;
    bool replayMirrorProducerKnown8008EEF8 = false;
    uint32_t replayMirrorProducerFunction = 0;
    bool replayMirrorByteCountKnown8008EEF8 = false;
    uint32_t replayMirrorKnownByteCount8008EEF8 = 0;
    bool savePayloadBankKnown = false;
    bool statusBankKnown80092F1D = false;
    bool saveStatusBackupKnown80079008 = false;
    bool saveStatusBackupStatusBankKnown80092F1D = false;
    uint32_t savePayloadBankLastWriterFunction = 0;
    bool wrote80015CC4 = false;
    bool wrote800164B4 = false;
    bool wrote8001635C = false;
    bool wrote8001628C = false;
    bool wrote800167A8 = false;
    bool wrote80015744 = false;
    bool boundsFault = false;
    uint32_t lastFaultAddress = 0;
};

bool IsKnownReplayMirrorProducerFunction8001635C(uint32_t psxFunction);

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

struct LoadSavePayloadAuthority800164B4 {
    bool runtimeLowerCardProducerKnown = false;
    bool typedReadSuccessKnown800179B4 = false;
    bool payloadBytesKnown8007ADE8 = false;
    uint32_t payloadAddress8007ADE8 = kTypedPayloadSourceAddress8007ADE8;
};

struct UpdateSavePayloadResult8001635C {
    bool ok = false;
    bool payloadKnown = false;
    bool statusBankKnown = false;
    bool mapped = false;
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
    uint32_t psxAddress = kPsxAddress;
    uint32_t byteCount = kByteCount;
    uint32_t lastWriterFunction = 0;
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
