#include "pr/pr_stage_payload_bank_direct.h"

#include <array>
#include <cstdio>

using namespace PrStagePayloadBankDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

using Payload = std::array<uint8_t, kByteCount80092F10>;

Payload MakePayload(uint8_t seed) {
    Payload payload{};
    for (size_t i = 0; i < payload.size(); ++i) {
        payload[i] = static_cast<uint8_t>(seed + ((i * 17u) & 0xffu));
    }
    return payload;
}

LoadSavePayloadAuthority800164B4 MakeRuntimeLowerCardPayloadAuthority() {
    LoadSavePayloadAuthority800164B4 authority{};
    authority.runtimeLowerCardProducerKnown = true;
    authority.typedReadSuccessKnown800179B4 = true;
    authority.payloadBytesKnown8007ADE8 = true;
    authority.payloadAddress8007ADE8 = kTypedPayloadSourceAddress8007ADE8;
    return authority;
}

uint32_t ReadLe32Payload(const MemoryState80092F10& state, uint32_t address) {
    const size_t offset = static_cast<size_t>(address - kBaseAddress80092F10);
    return static_cast<uint32_t>(state.savePayloadBank[offset]) |
           (static_cast<uint32_t>(state.savePayloadBank[offset + 1u]) << 8u) |
           (static_cast<uint32_t>(state.savePayloadBank[offset + 2u]) << 16u) |
           (static_cast<uint32_t>(state.savePayloadBank[offset + 3u]) << 24u);
}

void WriteLe32Payload(MemoryState80092F10& state,
                      uint32_t address,
                      uint32_t value) {
    const size_t offset = static_cast<size_t>(address - kBaseAddress80092F10);
    state.savePayloadBank[offset] = static_cast<uint8_t>(value & 0xffu);
    state.savePayloadBank[offset + 1u] =
        static_cast<uint8_t>((value >> 8u) & 0xffu);
    state.savePayloadBank[offset + 2u] =
        static_cast<uint8_t>((value >> 16u) & 0xffu);
    state.savePayloadBank[offset + 3u] =
        static_cast<uint8_t>((value >> 24u) & 0xffu);
}

void SeedKnownPayloadBank(MemoryState80092F10& state, uint8_t seed) {
    state.savePayloadBank = MakePayload(seed);
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;
    state.replayMirrorKnown8008EEF8 = true;
    state.replayMirrorProducerKnown8008EEF8 = true;
    state.replayMirrorProducerFunction = kReplayMirrorProducerFn801C8660;
    state.replayMirrorByteCountKnown8008EEF8 = true;
    state.replayMirrorKnownByteCount8008EEF8 =
        static_cast<uint32_t>(kMirrorBytes8001635C);
    state.savePayloadBankLastWriterFunction = kFn800164B4;
    state.wrote800164B4 = true;
    for (size_t i = 0; i < state.replayMirror.size(); ++i) {
        state.replayMirror[i] = static_cast<uint8_t>(0xA0u + ((i * 3u) & 0xffu));
    }
}

void TestLoadSavePayload800164B4CopiesFullPrefix() {
    MemoryState80092F10 state{};
    const Payload payload = MakePayload(0x31u);

    const LoadSavePayloadResult800164B4 result =
        LoadSavePayload800164B4(state,
                                0x8007ADE8u,
                                payload.data(),
                                payload.size(),
                                MakeRuntimeLowerCardPayloadAuthority());

    CHECK(result.ok);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(result.sourceAuthorityKnown);
    CHECK(result.srcAddress == 0x8007ADE8u);
    CHECK(result.sourceBytes == kByteCount80092F10);
    CHECK(!state.boundsFault);
    CHECK(state.savePayloadBankKnown);
    CHECK(state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == kFn800164B4);
    CHECK(state.wrote800164B4);
    CHECK(!state.wrote80015744);

    const SaveStatusPrefixSnapshot80092F10 prefix =
        SnapshotSaveStatusPrefix80092F10(state);
    CHECK(prefix.known);
    CHECK(prefix.statusBankKnown80092F1D);
    CHECK(prefix.psxAddress == kBaseAddress80092F10);
    CHECK(prefix.byteCount == kByteCount80092F10);
    CHECK(prefix.lastWriterFunction == kFn800164B4);
    CHECK(prefix.wrote800164B4);
    CHECK(!prefix.wrote80015744);
    CHECK(prefix.bytes[0] == payload[0]);
    CHECK(prefix.bytes[0x0Du] == payload[0x0Du]);
    CHECK(prefix.bytes[0x14u] == payload[0x14u]);
    CHECK(prefix.bytes[kByteCount80092F10 - 1u] ==
          payload[kByteCount80092F10 - 1u]);

    const StatusBankSnapshot80092F10 status =
        SnapshotStatusBank80092F10(state);
    CHECK(status.statusBytesKnown80092F1D);
    CHECK(status.scoreDwordsKnown80092F24);
    CHECK(status.byte80092F1D[0] == payload[0x0Du]);
    CHECK(status.byte80092F1D[5] == payload[0x12u]);
}

void TestLoadSavePayload800164B4ExactSourceWithoutAuthorityFailsClosed() {
    MemoryState80092F10 state{};
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;
    state.savePayloadBankLastWriterFunction = kFn800164B4;
    state.wrote800164B4 = true;
    const Payload before = state.savePayloadBank;
    const Payload payload = MakePayload(0x51u);

    const LoadSavePayloadResult800164B4 result =
        LoadSavePayload800164B4(state,
                                kTypedPayloadSourceAddress8007ADE8,
                                payload.data(),
                                payload.size(),
                                LoadSavePayloadAuthority800164B4{});

    CHECK(!result.ok);
    CHECK(!result.payloadKnown);
    CHECK(!result.statusBankKnown);
    CHECK(!result.sourceAuthorityKnown);
    CHECK(result.sourceBytes == kByteCount80092F10);
    CHECK(result.lastFaultAddress == kTypedPayloadSourceAddress8007ADE8);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.boundsFault);
    CHECK(state.lastFaultAddress == kTypedPayloadSourceAddress8007ADE8);
    CHECK(!state.wrote800164B4);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(state.savePayloadBank == before);
}

void TestLoadSavePayload800164B4ShortSourceFailsClosed() {
    MemoryState80092F10 state{};
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;
    const Payload payload = MakePayload(0x71u);

    const LoadSavePayloadResult800164B4 result =
        LoadSavePayload800164B4(state,
                                0x8007ADE8u,
                                payload.data(),
                                payload.size() - 1u,
                                MakeRuntimeLowerCardPayloadAuthority());

    CHECK(!result.ok);
    CHECK(!result.payloadKnown);
    CHECK(!result.statusBankKnown);
    CHECK(result.sourceAuthorityKnown);
    CHECK(result.sourceBytes == kByteCount80092F10 - 1u);
    CHECK(result.lastFaultAddress == 0x8007ADE8u);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.boundsFault);
    CHECK(state.lastFaultAddress == 0x8007ADE8u);
    CHECK(!state.wrote800164B4);
    CHECK(state.savePayloadBankLastWriterFunction == 0);

    const SaveStatusPrefixSnapshot80092F10 prefix =
        SnapshotSaveStatusPrefix80092F10(state);
    CHECK(!prefix.known);
    CHECK(!prefix.statusBankKnown80092F1D);
    CHECK(prefix.lastWriterFunction == 0);
    CHECK(!prefix.wrote800164B4);
}

void TestLoadSavePayload800164B4OversizeSourceFailsClosed() {
    MemoryState80092F10 state{};
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;
    std::array<uint8_t, kByteCount80092F10 + 1u> payload{};
    for (size_t i = 0; i < payload.size(); ++i) {
        payload[i] = static_cast<uint8_t>(0x41u + ((i * 11u) & 0xffu));
    }

    const LoadSavePayloadResult800164B4 result =
        LoadSavePayload800164B4(state,
                                0x8007ADE8u,
                                payload.data(),
                                payload.size(),
                                MakeRuntimeLowerCardPayloadAuthority());

    CHECK(!result.ok);
    CHECK(!result.payloadKnown);
    CHECK(!result.statusBankKnown);
    CHECK(result.sourceAuthorityKnown);
    CHECK(result.sourceBytes == kByteCount80092F10 + 1u);
    CHECK(result.lastFaultAddress == 0x8007ADE8u);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.boundsFault);
    CHECK(state.lastFaultAddress == 0x8007ADE8u);
    CHECK(!state.wrote800164B4);
    CHECK(state.savePayloadBankLastWriterFunction == 0);

    const SaveStatusPrefixSnapshot80092F10 prefix =
        SnapshotSaveStatusPrefix80092F10(state);
    CHECK(!prefix.known);
    CHECK(!prefix.statusBankKnown80092F1D);
    CHECK(prefix.lastWriterFunction == 0);
    CHECK(!prefix.wrote800164B4);
}

void TestLoadSavePayload800164B4NullSourceFailsClosed() {
    MemoryState80092F10 state{};
    state.savePayloadBankKnown = true;
    state.statusBankKnown80092F1D = true;

    const LoadSavePayloadResult800164B4 result =
        LoadSavePayload800164B4(state,
                                0x8007ADE8u,
                                nullptr,
                                kByteCount80092F10,
                                MakeRuntimeLowerCardPayloadAuthority());

    CHECK(!result.ok);
    CHECK(!result.payloadKnown);
    CHECK(!result.statusBankKnown);
    CHECK(result.sourceAuthorityKnown);
    CHECK(result.sourceBytes == kByteCount80092F10);
    CHECK(result.lastFaultAddress == 0x8007ADE8u);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.boundsFault);
    CHECK(state.lastFaultAddress == 0x8007ADE8u);
    CHECK(!state.wrote800164B4);
}

void TestUpdateSavePayload8001635CSuccessCommitsAtomically() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x21u);
    for (uint32_t i = 0; i < 6u; ++i) {
        state.savePayloadBank[(kStatusBaseAddress80092F1D - kBaseAddress80092F10) +
                              i] = 3u;
    }
    state.savePayloadBank[(kStatusBaseAddress80092F1D - kBaseAddress80092F10) +
                          2u] = 1u;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state,
                                  3,
                                  3,
                                  0,
                                  0x11223344,
                                  true,
                                  0x55667788u);

    CHECK(result.ok);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(result.mapped);
    CHECK(result.slotIndex == 2);
    CHECK(result.carrierA3 == 1u);
    CHECK(result.carrierSourceKnown);
    CHECK(result.carrierSource == 0x55667788u);
    CHECK(result.mirrorSourceKnown);
    CHECK(result.mirrorCopied);
    CHECK(result.allClearQueried);
    CHECK(result.allClearWritten);
    CHECK(result.result == 1u);
    CHECK(state.savePayloadBankKnown);
    CHECK(state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == kFn8001635C);
    CHECK(state.wrote8001635C);

    CHECK(state.savePayloadBank[(kStatusBaseAddress80092F1D - kBaseAddress80092F10) +
                                2u] == 3u);
    CHECK(ReadLe32Payload(state, kScoreBaseAddress80092F24 + 2u * 4u) ==
          0x11223344u);
    CHECK(ReadLe32Payload(state, kCarrierIndexAddress80092F3C) == 2u);
    CHECK(ReadLe32Payload(state, kCarrierA3Address80092F40) == 1u);
    CHECK(ReadLe32Payload(state, kCarrierCompleteAddress80092F44) == 1u);
    CHECK(ReadLe32Payload(state, kCarrierSourceAddress80092F48) == 0x55667788u);
    CHECK(state.savePayloadBank[kMirrorDstAddress80092F5C - kBaseAddress80092F10] ==
          state.replayMirror[0]);
    CHECK(state.savePayloadBank[(kMirrorDstAddress80092F5C - kBaseAddress80092F10) +
                                kMirrorBytes8001635C - 1u] ==
          state.replayMirror[kMirrorBytes8001635C - 1u]);
}

void TestUpdateSavePayload8001635CCarrierGapDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x41u);
    WriteLe32Payload(state, kCarrierA3Address80092F40, 0xCAFEBABEu);
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, false, 0);

    CHECK(!result.ok);
    CHECK(!result.payloadKnown);
    CHECK(!result.statusBankKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(!state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.savePayloadBank == before);
    CHECK(ReadLe32Payload(state, kCarrierA3Address80092F40) == 0xCAFEBABEu);
}

void TestUpdateSavePayload8001635CUnknownStatusDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x51u);
    state.statusBankKnown80092F1D = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 0x55667788u);

    CHECK(!result.ok);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(!state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.savePayloadBank == before);
}

void TestUpdateSavePayload8001635CUnmappedDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x61u);
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 99, 3, 9, 0x11223344, true, 0x55667788u);

    CHECK(!result.ok);
    CHECK(!result.mapped);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(!state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.savePayloadBank == before);
}

void TestUpdateSavePayload8001635CUnknownPayloadDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x71u);
    state.savePayloadBankKnown = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 0x55667788u);

    CHECK(!result.ok);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(!state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.savePayloadBank == before);
}

void TestUpdateSavePayload8001635CUnknownReplayMirrorDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x81u);
    state.replayMirrorKnown8008EEF8 = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 0x55667788u);

    CHECK(!result.ok);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(!state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.savePayloadBank == before);
}

void TestUpdateSavePayload8001635CUnknownReplayProducerDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x91u);
    state.replayMirrorProducerKnown8008EEF8 = false;
    state.replayMirrorProducerFunction = 0;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 0x55667788u);

    CHECK(!result.ok);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(!state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.savePayloadBank == before);
}

void TestUpdateSavePayload8001635CPartialReplayMirrorBytesDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0xA1u);
    state.replayMirrorByteCountKnown8008EEF8 = true;
    state.replayMirrorKnownByteCount8008EEF8 =
        static_cast<uint32_t>(kMirrorBytes8001635C - 8);
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 0x55667788u);

    CHECK(!result.ok);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(!state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.savePayloadBank == before);
}

void TestUpdateSavePayload8001635CPartialAcceptedAppend80014614DoesNotMutatePrefix() {
    const uint32_t appendCounts[] = {1u, 2u};
    for (uint32_t appendCount : appendCounts) {
        MemoryState80092F10 state{};
        SeedKnownPayloadBank(state, 0xA1u);
        state.replayMirrorProducerFunction = kReplayMirrorProducerFn80014614;
        state.replayMirrorByteCountKnown8008EEF8 = true;
        state.replayMirrorKnownByteCount8008EEF8 =
            appendCount * 2u * sizeof(uint32_t);
        const Payload before = state.savePayloadBank;

        const UpdateSavePayloadResult8001635C result =
            UpdateSavePayload8001635C(
                state, 3, 3, 9, 0x11223344, true, 0x55667788u);

        CHECK(!result.ok);
        CHECK(!result.carrierSourceKnown);
        CHECK(result.carrierSource == 0u);
        CHECK(!result.mirrorSourceKnown);
        CHECK(!result.mirrorCopied);
        CHECK(!result.allClearQueried);
        CHECK(!state.savePayloadBankKnown);
        CHECK(!state.statusBankKnown80092F1D);
        CHECK(state.savePayloadBankLastWriterFunction == 0);
        CHECK(!state.wrote800164B4);
        CHECK(!state.wrote8001635C);
        CHECK(state.savePayloadBank == before);
    }
}

void TestUpdateSavePayload8001635CFailureClearsSourcePublishing() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0xB1u);
    state.statusBankKnown80092F1D = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 0x55667788u);

    CHECK(!result.ok);
    CHECK(!result.payloadKnown);
    CHECK(!result.statusBankKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!result.allClearWritten);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == 0);
    CHECK(!state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.savePayloadBank == before);
}

} // namespace

int main() {
    TestLoadSavePayload800164B4CopiesFullPrefix();
    TestLoadSavePayload800164B4ExactSourceWithoutAuthorityFailsClosed();
    TestLoadSavePayload800164B4ShortSourceFailsClosed();
    TestLoadSavePayload800164B4OversizeSourceFailsClosed();
    TestLoadSavePayload800164B4NullSourceFailsClosed();
    TestUpdateSavePayload8001635CSuccessCommitsAtomically();
    TestUpdateSavePayload8001635CCarrierGapDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnknownStatusDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnmappedDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnknownPayloadDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnknownReplayMirrorDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnknownReplayProducerDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CPartialReplayMirrorBytesDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CPartialAcceptedAppend80014614DoesNotMutatePrefix();
    TestUpdateSavePayload8001635CFailureClearsSourcePublishing();

    if (g_failed != 0) {
        std::printf("test_stage_payload_bank_800164b4: failed checks=%d\n",
                    g_failed);
        return 1;
    }
    std::printf("test_stage_payload_bank_800164b4: ok\n");
    return 0;
}
