#include "pr_stage_payload_bank_direct.h"

#include <cstring>

namespace PrStagePayloadBankDirect {
namespace {

constexpr int32_t kSavePayloadMap80048DB8[] = {
    0, 0, 1, 2, 3, 4, 5, 6,
};

constexpr int32_t kSaveSceneMap80048DD8[] = {
    1, 2, 3, 4, 5, 6,
};

bool SpanContains(uint32_t start, size_t size, uint32_t address, size_t count) {
    const uint64_t begin = start;
    const uint64_t end = begin + size;
    const uint64_t accessBegin = address;
    const uint64_t accessEnd = accessBegin + count;
    return accessBegin >= begin && accessEnd <= end && accessEnd >= accessBegin;
}

bool DirectReadByte(MemoryState80092F10& state,
                    uint32_t address,
                    uint8_t& value) {
    const uint8_t* ptr = DirectMemoryReadPtr80092F10(state, address, 1);
    if (!ptr) {
        return false;
    }
    value = *ptr;
    return true;
}

bool DirectWriteByte(MemoryState80092F10& state,
                     uint32_t address,
                     uint8_t value) {
    uint8_t* ptr = DirectMemoryPtr80092F10(state, address, 1);
    if (!ptr) {
        return false;
    }
    *ptr = value;
    return true;
}

bool DirectReadLe32(MemoryState80092F10& state,
                    uint32_t address,
                    uint32_t& value) {
    uint8_t b0 = 0;
    uint8_t b1 = 0;
    uint8_t b2 = 0;
    uint8_t b3 = 0;
    if (!DirectReadByte(state, address, b0) ||
        !DirectReadByte(state, address + 1u, b1) ||
        !DirectReadByte(state, address + 2u, b2) ||
        !DirectReadByte(state, address + 3u, b3)) {
        return false;
    }
    value = static_cast<uint32_t>(b0) |
            (static_cast<uint32_t>(b1) << 8u) |
            (static_cast<uint32_t>(b2) << 16u) |
            (static_cast<uint32_t>(b3) << 24u);
    return true;
}

bool DirectWriteLe32(MemoryState80092F10& state,
                     uint32_t address,
                     uint32_t value) {
    return DirectWriteByte(state, address, static_cast<uint8_t>(value & 0xFFu)) &&
           DirectWriteByte(
               state, address + 1u, static_cast<uint8_t>((value >> 8u) & 0xFFu)) &&
           DirectWriteByte(
               state, address + 2u, static_cast<uint8_t>((value >> 16u) & 0xFFu)) &&
           DirectWriteByte(
               state, address + 3u, static_cast<uint8_t>((value >> 24u) & 0xFFu));
}

bool ReplayMirrorAuthorityKnown8001635C(const MemoryState80092F10& state) {
    return state.replayMirrorKnown8008EEF8 &&
           state.replayMirrorProducerKnown8008EEF8 &&
           IsKnownReplayMirrorProducerFunction8001635C(
               state.replayMirrorProducerFunction) &&
           state.replayMirrorByteCountKnown8008EEF8 &&
           state.replayMirrorKnownByteCount8008EEF8 >=
               static_cast<uint32_t>(kMirrorBytes8001635C);
}

bool LoadPayloadAuthorityKnown800164B4(
    const LoadSavePayloadAuthority800164B4& authority,
    uint32_t srcAddress) {
    return authority.runtimeLowerCardProducerKnown &&
           authority.typedReadSuccessKnown800179B4 &&
           authority.payloadBytesKnown8007ADE8 &&
           authority.payloadAddress8007ADE8 == kTypedPayloadSourceAddress8007ADE8 &&
           srcAddress == kTypedPayloadSourceAddress8007ADE8;
}

uint8_t* ScratchPayloadPtr80092F10(
    std::array<uint8_t, kByteCount80092F10>& scratch,
    uint32_t address,
    size_t count) {
    if (!SpanContains(kBaseAddress80092F10, scratch.size(), address, count)) {
        return nullptr;
    }
    return scratch.data() + (address - kBaseAddress80092F10);
}

bool ScratchReadByte80092F10(
    const std::array<uint8_t, kByteCount80092F10>& scratch,
    uint32_t address,
    uint8_t& value) {
    if (!SpanContains(kBaseAddress80092F10, scratch.size(), address, 1)) {
        return false;
    }
    value = scratch[address - kBaseAddress80092F10];
    return true;
}

bool ScratchWriteByte80092F10(
    std::array<uint8_t, kByteCount80092F10>& scratch,
    uint32_t address,
    uint8_t value) {
    uint8_t* ptr = ScratchPayloadPtr80092F10(scratch, address, 1);
    if (!ptr) {
        return false;
    }
    *ptr = value;
    return true;
}

bool ScratchWriteLe32_80092F10(
    std::array<uint8_t, kByteCount80092F10>& scratch,
    uint32_t address,
    uint32_t value) {
    return ScratchWriteByte80092F10(
               scratch, address, static_cast<uint8_t>(value & 0xFFu)) &&
           ScratchWriteByte80092F10(
               scratch, address + 1u, static_cast<uint8_t>((value >> 8u) & 0xFFu)) &&
           ScratchWriteByte80092F10(
               scratch, address + 2u, static_cast<uint8_t>((value >> 16u) & 0xFFu)) &&
           ScratchWriteByte80092F10(
               scratch, address + 3u, static_cast<uint8_t>((value >> 24u) & 0xFFu));
}

void ClearPayloadAuthority80092F10(MemoryState80092F10& state) {
    state.savePayloadBankKnown = false;
    state.statusBankKnown80092F1D = false;
    state.savePayloadBankLastWriterFunction = 0;
    state.wrote80015CC4 = false;
    state.wrote800164B4 = false;
    state.wrote8001635C = false;
    state.wrote8001628C = false;
    state.wrote800167A8 = false;
    state.wrote80015744 = false;
}

} // namespace

bool IsKnownReplayMirrorProducerFunction8001635C(uint32_t psxFunction) {
    return psxFunction == kReplayMirrorProducerFn801C4FC8 ||
           psxFunction == kReplayMirrorProducerFn801C8660 ||
           psxFunction == kReplayMirrorProducerFn80014614 ||
           psxFunction == kReplayMirrorProducerFn8001681C;
}

MapResult8001615C MapSaveStage8001615C(int32_t a1) {
    MapResult8001615C out{};
    out.input = a1;
    if (a1 < 0 ||
        static_cast<size_t>(a1) >=
            (sizeof(kSavePayloadMap80048DB8) /
             sizeof(kSavePayloadMap80048DB8[0]))) {
        return out;
    }

    out.mapped = true;
    out.mappedIndex = kSavePayloadMap80048DB8[a1];
    return out;
}

MapSceneResult800161A8 MapScene800161A8(int32_t a1) {
    MapSceneResult800161A8 out{};
    out.input = a1;
    if (a1 < 0 ||
        static_cast<size_t>(a1) >=
            (sizeof(kSaveSceneMap80048DD8) /
             sizeof(kSaveSceneMap80048DD8[0]))) {
        return out;
    }

    out.mapped = true;
    out.sceneId = kSaveSceneMap80048DD8[a1];
    return out;
}

int32_t AllStatusesClear800161F4(const uint8_t* bytes, size_t count) {
    if (!bytes || count < static_cast<size_t>(kStatusSlotCount800161F4)) {
        return 0;
    }

    for (int32_t i = 0; i < kStatusSlotCount800161F4; ++i) {
        if (bytes[i] != 3u) {
            return 0;
        }
    }
    return 1;
}

void MarkPayloadWriter80092F10(MemoryState80092F10& state,
                               uint32_t function) {
    state.savePayloadBankLastWriterFunction = function;
    switch (function) {
    case kFn80015CC4:
        state.wrote80015CC4 = true;
        break;
    case kFn800164B4:
        state.wrote800164B4 = true;
        break;
    case kFn8001635C:
        state.wrote8001635C = true;
        break;
    case kFn8001628C:
        state.wrote8001628C = true;
        break;
    case kFn800167A8:
        state.wrote800167A8 = true;
        break;
    case kFn80015744:
        state.wrote80015744 = true;
        break;
    default:
        break;
    }
}

uint8_t* DirectMemoryPtr80092F10(MemoryState80092F10& state,
                                 uint32_t address,
                                 size_t count) {
    if (SpanContains(kBaseAddress80092F10,
                     state.savePayloadBank.size(),
                     address,
                     count)) {
        return state.savePayloadBank.data() +
               (address - kBaseAddress80092F10);
    }
    if (SpanContains(0x80079008u,
                     state.saveStatusBackup.size(),
                     address,
                     count)) {
        return state.saveStatusBackup.data() + (address - 0x80079008u);
    }
    if (SpanContains(kMirrorSrcAddress8008EEF8,
                     state.replayMirror.size(),
                     address,
                     count)) {
        return state.replayMirror.data() +
               (address - kMirrorSrcAddress8008EEF8);
    }

    state.boundsFault = true;
    state.lastFaultAddress = address;
    return nullptr;
}

const uint8_t* DirectMemoryReadPtr80092F10(MemoryState80092F10& state,
                                           uint32_t address,
                                           size_t count) {
    return DirectMemoryPtr80092F10(state, address, count);
}

AllStatusQueryResult800161F4 QueryAllStatuses800161F4(
    MemoryState80092F10& state) {
    AllStatusQueryResult800161F4 out{};
    out.statusBankKnown = state.statusBankKnown80092F1D;
    if (!state.statusBankKnown80092F1D) {
        return out;
    }

    bool bytesKnown = true;
    for (uint32_t i = 0;
         i < static_cast<uint32_t>(kStatusSlotCount800161F4);
         ++i) {
        bytesKnown = DirectReadByte(state,
                                    kStatusBaseAddress80092F1D + i,
                                    out.byte80092F1D[i]) &&
                     bytesKnown;
    }
    out.statusBytesKnown80092F1D = bytesKnown;
    if (!bytesKnown) {
        state.statusBankKnown80092F1D = false;
        out.statusBankKnown = false;
        return out;
    }

    out.result =
        static_cast<uint32_t>(
            AllStatusesClear800161F4(out.byte80092F1D,
                                     sizeof(out.byte80092F1D)));
    out.ok = true;
    return out;
}

StatusQueryResult800166AC QueryStatus800166AC(MemoryState80092F10& state,
                                               int32_t a1) {
    StatusQueryResult800166AC out{};
    out.sceneId = a1;
    out.statusBankKnown = state.statusBankKnown80092F1D;

    const MapResult8001615C map = MapSaveStage8001615C(a1);
    out.mapped = map.mapped;
    out.slotIndex = map.mappedIndex;
    if (!map.mapped) {
        return out;
    }
    if (!state.statusBankKnown80092F1D) {
        return out;
    }

    if (!DirectReadByte(state,
                        kStatusBaseAddress80092F1D +
                            static_cast<uint32_t>(map.mappedIndex),
                        out.status)) {
        state.statusBankKnown80092F1D = false;
        out.statusBankKnown = false;
        return out;
    }

    out.ok = true;
    return out;
}

EnsureProgressResult8001628C EnsureProgress8001628C(
    MemoryState80092F10& state,
    int32_t a1) {
    EnsureProgressResult8001628C out{};
    out.sceneId = a1;
    out.payloadKnown = state.savePayloadBankKnown;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.lastFaultAddress = state.lastFaultAddress;

    const MapResult8001615C map = MapSaveStage8001615C(a1);
    out.mapped = map.mapped;
    out.slotIndex = map.mappedIndex;
    if (!map.mapped) {
        state.savePayloadBankKnown = false;
        out.payloadKnown = false;
        return out;
    }
    if (!state.statusBankKnown80092F1D) {
        state.savePayloadBankKnown = false;
        out.payloadKnown = false;
        return out;
    }

    const uint32_t progressAddress =
        kStatusBaseAddress80092F1D + static_cast<uint32_t>(map.mappedIndex);
    uint8_t progress = 0;
    if (!DirectReadByte(state, progressAddress, progress)) {
        state.savePayloadBankKnown = false;
        state.statusBankKnown80092F1D = false;
        out.payloadKnown = false;
        out.statusBankKnown = false;
        out.lastFaultAddress = state.lastFaultAddress;
        return out;
    }
    if (progress == 0) {
        progress = 1;
        if (!DirectWriteByte(state, progressAddress, progress)) {
            state.savePayloadBankKnown = false;
            state.statusBankKnown80092F1D = false;
            out.payloadKnown = false;
            out.statusBankKnown = false;
            out.lastFaultAddress = state.lastFaultAddress;
            return out;
        }
    }

    MarkPayloadWriter80092F10(state, kFn8001628C);
    out.ok = true;
    out.payloadKnown = state.savePayloadBankKnown;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.result = progress;
    out.lastFaultAddress = state.lastFaultAddress;
    return out;
}

InitSavePayloadResult80015CC4 InitSavePayload80015CC4(
    MemoryState80092F10& state) {
    InitSavePayloadResult80015CC4 out{};
    out.lastFaultAddress = state.lastFaultAddress;
    for (size_t i = 0; i < state.savePayloadBank.size(); ++i) {
        state.savePayloadBank[i] = 0;
    }
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;
    MarkPayloadWriter80092F10(state, kFn80015CC4);
    out.ok = true;
    out.payloadKnown = state.savePayloadBankKnown;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.lastFaultAddress = state.lastFaultAddress;
    return out;
}

LoadSavePayloadResult800164B4 LoadSavePayload800164B4(
    MemoryState80092F10& state,
    uint32_t srcAddress,
    const uint8_t* source,
    size_t sourceBytes,
    const LoadSavePayloadAuthority800164B4& authority) {
    LoadSavePayloadResult800164B4 out{};
    out.srcAddress = srcAddress;
    out.sourceBytes = sourceBytes;
    out.sourceAuthorityKnown =
        LoadPayloadAuthorityKnown800164B4(authority, srcAddress);
    out.lastFaultAddress = state.lastFaultAddress;
    if (!source || sourceBytes != kByteCount80092F10 ||
        !out.sourceAuthorityKnown) {
        ClearPayloadAuthority80092F10(state);
        state.boundsFault = true;
        state.lastFaultAddress = srcAddress;
        out.lastFaultAddress = state.lastFaultAddress;
        return out;
    }

    for (size_t i = 0; i < kByteCount80092F10; ++i) {
        state.savePayloadBank[i] = source[i];
    }
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;
    MarkPayloadWriter80092F10(state, kFn800164B4);
    out.ok = true;
    out.payloadKnown = state.savePayloadBankKnown;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.lastFaultAddress = state.lastFaultAddress;
    return out;
}

UpdateSavePayloadResult8001635C UpdateSavePayload8001635C(
    MemoryState80092F10& state,
    int32_t a1,
    int32_t a2,
    int32_t a3,
    int32_t a4,
    bool carrierSourceKnown,
    uint32_t carrierSource) {
    UpdateSavePayloadResult8001635C out{};
    out.sceneId = a1;
    out.targetStatus = a2;
    out.carrierA3Input = a3;
    out.scoreValue = a4;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.payloadKnown = state.savePayloadBankKnown;
    out.carrierSourceKnown = carrierSourceKnown;
    out.carrierSource = carrierSource;
    out.mirrorSourceKnown = ReplayMirrorAuthorityKnown8001635C(state);
    out.lastFaultAddress = state.lastFaultAddress;

    const MapResult8001615C map = MapSaveStage8001615C(a1);
    out.mapped = map.mapped;
    out.slotIndex = map.mappedIndex;
    out.carrierA3 = static_cast<uint32_t>(a3 != 0 ? a3 : 1);

    auto failClosed = [&]() {
        ClearPayloadAuthority80092F10(state);
        out.payloadKnown = state.savePayloadBankKnown;
        out.statusBankKnown = state.statusBankKnown80092F1D;
        out.carrierSourceKnown = false;
        out.carrierSource = 0;
        out.mirrorSourceKnown = false;
        out.mirrorCopied = false;
        out.allClearQueried = false;
        out.allClearWritten = false;
        out.lastFaultAddress = state.lastFaultAddress;
        return out;
    };

    if (!state.savePayloadBankKnown || !map.mapped ||
        !state.statusBankKnown80092F1D || !carrierSourceKnown ||
        !ReplayMirrorAuthorityKnown8001635C(state)) {
        return failClosed();
    }

    std::array<uint8_t, kByteCount80092F10> scratch = state.savePayloadBank;
    bool scratchAuthorityKnown =
        ScratchPayloadPtr80092F10(scratch, kCarrierA3Address80092F40, 4) &&
        ScratchPayloadPtr80092F10(scratch, kScoreBaseAddress80092F24 +
                                               static_cast<uint32_t>(map.mappedIndex) *
                                                   sizeof(uint32_t),
                                  4) &&
        ScratchPayloadPtr80092F10(scratch, kCarrierIndexAddress80092F3C, 4) &&
        ScratchPayloadPtr80092F10(scratch, kCarrierSourceAddress80092F48, 4) &&
        ScratchPayloadPtr80092F10(scratch,
                                  kMirrorDstAddress80092F5C,
                                  static_cast<size_t>(kMirrorBytes8001635C)) &&
        ScratchPayloadPtr80092F10(scratch, kCarrierCompleteAddress80092F44, 4);

    const uint32_t progressAddress =
        kStatusBaseAddress80092F1D + static_cast<uint32_t>(map.mappedIndex);
    uint8_t progress = 0;
    scratchAuthorityKnown =
        ScratchReadByte80092F10(scratch, progressAddress, progress) &&
        scratchAuthorityKnown;

    uint8_t statusBytes[kStatusSlotCount800161F4]{};
    bool statusBytesKnown = true;
    for (uint32_t i = 0;
         i < static_cast<uint32_t>(kStatusSlotCount800161F4);
         ++i) {
        statusBytesKnown =
            ScratchReadByte80092F10(
                scratch,
                kStatusBaseAddress80092F1D + i,
                statusBytes[i]) &&
            statusBytesKnown;
    }
    scratchAuthorityKnown = statusBytesKnown && scratchAuthorityKnown;

    if (!scratchAuthorityKnown) {
        return failClosed();
    }

    if (progress < static_cast<uint8_t>(a2)) {
        statusBytes[static_cast<size_t>(map.mappedIndex)] =
            static_cast<uint8_t>(a2);
    }
    out.allClearQueried = true;
    out.result =
        static_cast<uint32_t>(
            AllStatusesClear800161F4(statusBytes, sizeof(statusBytes)));

    bool complete =
        ScratchWriteLe32_80092F10(scratch, kCarrierA3Address80092F40, out.carrierA3);
    if (progress < static_cast<uint8_t>(a2)) {
        complete =
            ScratchWriteByte80092F10(
                scratch,
                progressAddress,
                static_cast<uint8_t>(a2)) &&
            complete;
    }

    complete =
        ScratchWriteLe32_80092F10(
            scratch,
            kScoreBaseAddress80092F24 +
                static_cast<uint32_t>(map.mappedIndex) * sizeof(uint32_t),
            static_cast<uint32_t>(a4)) &&
        complete;
    complete =
        ScratchWriteLe32_80092F10(
            scratch,
            kCarrierIndexAddress80092F3C,
            static_cast<uint32_t>(map.mappedIndex)) &&
        complete;
    complete =
        ScratchWriteLe32_80092F10(scratch, kCarrierSourceAddress80092F48, carrierSource) &&
        complete;

    bool mirrorCopied = true;
    for (int32_t i = 0; i < kMirrorBytes8001635C; ++i) {
        const uint32_t dstAddress =
            kMirrorDstAddress80092F5C + static_cast<uint32_t>(i);
        const uint8_t value = state.replayMirror[static_cast<size_t>(i)];
        if (!ScratchWriteByte80092F10(scratch, dstAddress, value)) {
            mirrorCopied = false;
            break;
        }
    }
    out.mirrorCopied = mirrorCopied;
    complete = mirrorCopied && complete;

    out.allClearWritten =
        ScratchWriteLe32_80092F10(
            scratch,
            kCarrierCompleteAddress80092F44,
            out.result);
    complete = out.allClearWritten && complete;

    if (!complete) {
        return failClosed();
    }

    state.savePayloadBank = scratch;
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;
    MarkPayloadWriter80092F10(state, kFn8001635C);

    out.ok = true;
    out.payloadKnown = true;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.lastFaultAddress = state.lastFaultAddress;
    return out;
}

StatusWriteResult800167A8 WriteStatus800167A8(MemoryState80092F10& state,
                                              int32_t a1,
                                              int32_t a2) {
    StatusWriteResult800167A8 out{};
    out.sceneId = a1;
    out.a2 = a2;
    out.status = static_cast<uint8_t>(a2 == 1 ? 2 : 1);
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.lastFaultAddress = state.lastFaultAddress;

    const MapResult8001615C map = MapSaveStage8001615C(a1);
    out.mapped = map.mapped;
    out.slotIndex = map.mappedIndex;
    if (!map.mapped) {
        return out;
    }
    if (!state.statusBankKnown80092F1D) {
        return out;
    }

    if (!DirectWriteByte(state,
                         kStatusBaseAddress80092F1D +
                             static_cast<uint32_t>(map.mappedIndex),
                         out.status)) {
        state.statusBankKnown80092F1D = false;
        out.statusBankKnown = false;
        out.lastFaultAddress = state.lastFaultAddress;
        return out;
    }

    MarkPayloadWriter80092F10(state, kFn800167A8);
    out.ok = true;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.lastFaultAddress = state.lastFaultAddress;
    return out;
}

SavedScoreSyncResult800169E0 SyncSavedScore800169E0(
    MemoryState80092F10& state,
    int32_t word800916D0,
    int32_t word800916E2) {
    SavedScoreSyncResult800169E0 out{};
    out.word800916D0 = word800916D0;
    out.word800916E2 = word800916E2;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.lastFaultAddress = state.lastFaultAddress;

    if (word800916D0 != 2) {
        out.ok = true;
        return out;
    }

    out.applied = true;
    const MapResult8001615C map = MapSaveStage8001615C(word800916E2);
    out.mapped = map.mapped;
    out.slotIndex = map.mappedIndex;
    if (!map.mapped) {
        return out;
    }
    if (!state.statusBankKnown80092F1D) {
        return out;
    }

    out.scoreDwordReadAttempted = true;
    if (!DirectReadLe32(state,
                        kScoreBaseAddress80092F24 +
                            static_cast<uint32_t>(map.mappedIndex) *
                                sizeof(uint32_t),
                        out.ctxScoreDword)) {
        state.statusBankKnown80092F1D = false;
        out.statusBankKnown = false;
        out.lastFaultAddress = state.lastFaultAddress;
        return out;
    }

    out.scoreDwordKnown = true;
    out.word80091816 = static_cast<uint16_t>(out.ctxScoreDword);
    out.ok = true;
    out.statusBankKnown = state.statusBankKnown80092F1D;
    out.lastFaultAddress = state.lastFaultAddress;
    return out;
}

StatusBankSnapshot80092F10 SnapshotStatusBank80092F10(
    MemoryState80092F10& state) {
    StatusBankSnapshot80092F10 out{};
    if (!state.statusBankKnown80092F1D) {
        return out;
    }

    bool bytesKnown = true;
    for (uint32_t i = 0; i < 6u; ++i) {
        bytesKnown = DirectReadByte(state,
                                    kStatusBaseAddress80092F1D + i,
                                    out.byte80092F1D[i]) &&
                     bytesKnown;
    }
    out.statusBytesKnown80092F1D = bytesKnown;

    bool scoresKnown = true;
    for (uint32_t i = 0; i < 6u; ++i) {
        scoresKnown = DirectReadLe32(state,
                                     kScoreBaseAddress80092F24 +
                                         i * sizeof(uint32_t),
                                     out.dword80092F24[i]) &&
                      scoresKnown;
    }
    out.scoreDwordsKnown80092F24 = scoresKnown;
    out.lastSavedSlotKnown80092F3C =
        DirectReadLe32(state,
                       kCarrierIndexAddress80092F3C,
                       out.dword80092F3C);
    out.allClearKnown80092F44 =
        DirectReadLe32(state,
                       kCarrierCompleteAddress80092F44,
                       out.dword80092F44);
    return out;
}

SaveStatusPrefixSnapshot80092F10 SnapshotSaveStatusPrefix80092F10(
    MemoryState80092F10& state) {
    SaveStatusPrefixSnapshot80092F10 out{};
    out.known = state.savePayloadBankKnown;
    out.statusBankKnown80092F1D = state.statusBankKnown80092F1D;
    out.lastWriterFunction = state.savePayloadBankLastWriterFunction;
    out.lastFaultAddress = state.lastFaultAddress;
    out.wrote80015CC4 = state.wrote80015CC4;
    out.wrote800164B4 = state.wrote800164B4;
    out.wrote8001635C = state.wrote8001635C;
    out.wrote8001628C = state.wrote8001628C;
    out.wrote800167A8 = state.wrote800167A8;
    out.wrote80015744 = state.wrote80015744;
    if (out.known) {
        std::memcpy(out.bytes,
                    state.savePayloadBank.data(),
                    sizeof(out.bytes));
    }
    return out;
}

} // namespace PrStagePayloadBankDirect
