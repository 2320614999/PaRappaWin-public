#include "pr/pr_stage_payload_bank_direct.h"
#include "pr/pr_stage1_scorer_direct.h"
#include "test_stage_payload_authority_fixture.h"

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

LoadSavePayloadAuthority800164B4 MakeRuntimeLowerCardPayloadAuthority(
    const uint8_t* payload,
    size_t payloadBytes) {
    LoadSavePayloadAuthority800164B4 authority{};
    CHECK(TestStagePayloadAuthorityFixture::
              MintRuntimeLowerCardPayloadAuthority800164B4(
                  payload, payloadBytes, &authority));
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
    const Payload payload = MakePayload(seed);
    CHECK(LoadSavePayload800164B4(
              state,
              kTypedPayloadSourceAddress8007ADE8,
              payload.data(),
              payload.size(),
              MakeRuntimeLowerCardPayloadAuthority(
                  payload.data(), payload.size()))
              .ok);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(state));

    PrStage1ScorerDirectReplayBufferState replay{};
    CHECK(PrStage1ScorerDirectInitializeReplayMirrorFromStartupZero80028590(
        replay));
    CHECK(PrStage1ScorerDirectBuildStage1EventTable801C8660(replay).applied);
    CHECK(ImportAuthoritativeReplayMirror8008EEF8(state, replay));
}

void CheckUpdateFailurePreservedPrefix(const MemoryState80092F10& state,
                                       const Payload& before,
                                       bool payloadKnown = true,
                                       bool statusKnown = true) {
    CHECK(state.savePayloadBankKnown == payloadKnown);
    CHECK(state.statusBankKnown80092F1D == statusKnown);
    CHECK(state.savePayloadBankLastWriterFunction == kFn800164B4);
    CHECK(state.wrote800164B4);
    CHECK(!state.wrote8001635C);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(state));
    CHECK(state.savePayloadBank == before);
}

void TestMarkPayloadWriterCannotMintReplayRestoreProvenance() {
    CHECK(!IsKnownReplayMirrorProducerFunction8001635C(0x801C4FC8u));
    MemoryState80092F10 state{};
    MarkPayloadWriter80092F10(state, kFn800164B4);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::Unknown);
    MarkPayloadWriter80092F10(state, kFn8001635C);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::Unknown);
}

void TestLoadSavePayload800164B4CopiesFullPrefix() {
    MemoryState80092F10 state{};
    const Payload payload = MakePayload(0x31u);

    const LoadSavePayloadResult800164B4 result =
        LoadSavePayload800164B4(state,
                                0x8007ADE8u,
                                 payload.data(),
                                 payload.size(),
                                 MakeRuntimeLowerCardPayloadAuthority(
                                     payload.data(), payload.size()));

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
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(state));

    const SaveStatusPrefixSnapshot80092F10 prefix =
        SnapshotSaveStatusPrefix80092F10(state);
    CHECK(prefix.known);
    CHECK(prefix.statusBankKnown80092F1D);
    CHECK(prefix.psxAddress == kBaseAddress80092F10);
    CHECK(prefix.byteCount == kByteCount80092F10);
    CHECK(prefix.lastWriterFunction == kFn800164B4);
    CHECK(prefix.wrote800164B4);
    CHECK(!prefix.wrote80015744);
    CHECK(prefix.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
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

void TestLoadSavePayload800164B4CapabilityCannotCrossPayloads() {
    MemoryState80092F10 state{};
    const Payload payloadA = MakePayload(0x35u);
    const Payload payloadB = MakePayload(0x36u);
    const LoadSavePayloadAuthority800164B4 authorityA =
        MakeRuntimeLowerCardPayloadAuthority(
            payloadA.data(), payloadA.size());

    CHECK(authorityA.KnownForAddress(kTypedPayloadSourceAddress8007ADE8));
    CHECK(authorityA.Matches(kTypedPayloadSourceAddress8007ADE8,
                             payloadA.data(),
                             payloadA.size()));
    CHECK(!authorityA.Matches(kTypedPayloadSourceAddress8007ADE8,
                              payloadB.data(),
                              payloadB.size()));

    const LoadSavePayloadResult800164B4 result =
        LoadSavePayload800164B4(state,
                                kTypedPayloadSourceAddress8007ADE8,
                                payloadB.data(),
                                payloadB.size(),
                                authorityA);

    CHECK(!result.ok);
    CHECK(result.sourceAuthorityKnown);
    CHECK(!state.savePayloadBankKnown);
    CHECK(!state.statusBankKnown80092F1D);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::Unknown);
    CHECK(!ReplayPayloadBackingAuthorityMatchesState80092F5C(state));
    CHECK(SnapshotSaveStatusPrefix80092F10(state)
              .replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::Unknown);
}

void TestSnapshotRejectsMutatedReplayPayloadBackingBytes() {
    const Payload payload = MakePayload(0x39u);
    const LoadSavePayloadAuthority800164B4 authority =
        MakeRuntimeLowerCardPayloadAuthority(payload.data(), payload.size());

    MemoryState80092F10 countMutation{};
    CHECK(LoadSavePayload800164B4(countMutation,
                                  kTypedPayloadSourceAddress8007ADE8,
                                  payload.data(),
                                  payload.size(),
                                  authority)
              .ok);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(countMutation));
    countMutation.savePayloadBank
        [kCarrierSourceAddress80092F48 - kBaseAddress80092F10] ^= 0x01u;
    CHECK(!ReplayPayloadBackingAuthorityMatchesState80092F5C(countMutation));
    CHECK(SnapshotSaveStatusPrefix80092F10(countMutation)
              .replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::Unknown);

    MemoryState80092F10 mirrorMutation{};
    CHECK(LoadSavePayload800164B4(mirrorMutation,
                                  kTypedPayloadSourceAddress8007ADE8,
                                  payload.data(),
                                  payload.size(),
                                  authority)
              .ok);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(mirrorMutation));
    mirrorMutation.savePayloadBank
        [(kMirrorDstAddress80092F5C - kBaseAddress80092F10) + 137u] ^= 0x80u;
    CHECK(!ReplayPayloadBackingAuthorityMatchesState80092F5C(mirrorMutation));
    CHECK(SnapshotSaveStatusPrefix80092F10(mirrorMutation)
              .replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::Unknown);
}

void TestSnapshotRejectsForgedPublicReplayPayloadProvenance() {
    const ReplayPayloadBackingProvenance80092F5C forgedKinds[] = {
        ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4,
        ReplayPayloadBackingProvenance80092F5C::ReplayMirrorCopy8001635C,
    };
    for (const ReplayPayloadBackingProvenance80092F5C forgedKind :
         forgedKinds) {
        MemoryState80092F10 state{};
        state.savePayloadBank = MakePayload(0x3Du);
        state.savePayloadBankKnown = true;
        state.statusBankKnown80092F1D = true;
        state.replayPayloadBackingProvenance80092F5C = forgedKind;

        CHECK(!ReplayPayloadBackingAuthorityMatchesState80092F5C(state));
        const SaveStatusPrefixSnapshot80092F10 snapshot =
            SnapshotSaveStatusPrefix80092F10(state);
        CHECK(snapshot.known);
        CHECK(snapshot.replayPayloadBackingProvenance80092F5C ==
              ReplayPayloadBackingProvenance80092F5C::Unknown);
    }
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
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::Unknown);
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
                                 MakeRuntimeLowerCardPayloadAuthority(
                                     payload.data(), kByteCount80092F10));

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
                                 MakeRuntimeLowerCardPayloadAuthority(
                                     payload.data(), kByteCount80092F10));

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
    const Payload authorityPayload = MakePayload(0x91u);

    const LoadSavePayloadResult800164B4 result =
        LoadSavePayload800164B4(state,
                                0x8007ADE8u,
                                 nullptr,
                                 kByteCount80092F10,
                                 MakeRuntimeLowerCardPayloadAuthority(
                                     authorityPayload.data(),
                                     authorityPayload.size()));

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
                                  53u);

    CHECK(result.ok);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(result.mapped);
    CHECK(result.preflightPayloadKnown);
    CHECK(result.preflightStatusBankKnown);
    CHECK(result.preflightMapped);
    CHECK(result.slotIndex == 2);
    CHECK(result.carrierA3 == 1u);
    CHECK(result.preflightCarrierSourceKnown);
    CHECK(result.preflightCarrierSourceMatchesReplayAuthority);
    CHECK(result.preflightCarrierSource == 53u);
    CHECK(result.preflightMirrorSourceKnown);
    CHECK(result.scratchAuthorityKnown);
    CHECK(result.carrierSourceKnown);
    CHECK(result.carrierSource == 53u);
    CHECK(result.mirrorSourceKnown);
    CHECK(result.mirrorCopied);
    CHECK(result.allClearQueried);
    CHECK(result.allClearWritten);
    CHECK(result.result == 1u);
    CHECK(state.savePayloadBankKnown);
    CHECK(state.statusBankKnown80092F1D);
    CHECK(state.savePayloadBankLastWriterFunction == kFn8001635C);
    CHECK(state.wrote8001635C);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::ReplayMirrorCopy8001635C);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(state));

    CHECK(state.savePayloadBank[(kStatusBaseAddress80092F1D - kBaseAddress80092F10) +
                                2u] == 3u);
    CHECK(ReadLe32Payload(state, kScoreBaseAddress80092F24 + 2u * 4u) ==
          0x11223344u);
    CHECK(ReadLe32Payload(state, kCarrierIndexAddress80092F3C) == 2u);
    CHECK(ReadLe32Payload(state, kCarrierA3Address80092F40) == 1u);
    CHECK(ReadLe32Payload(state, kCarrierCompleteAddress80092F44) == 1u);
    CHECK(ReadLe32Payload(state, kCarrierSourceAddress80092F48) == 53u);
    CHECK(state.savePayloadBank[kMirrorDstAddress80092F5C - kBaseAddress80092F10] ==
          state.replayMirror[0]);
    CHECK(state.savePayloadBank[(kMirrorDstAddress80092F5C - kBaseAddress80092F10) +
                                kMirrorBytes8001635C - 1u] ==
          state.replayMirror[kMirrorBytes8001635C - 1u]);
}

void TestUpdateSavePayload8001635CRejectsForgedReplayMirrorMetadata() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x25u);

    state.replayMirrorKnown8008EEF8 = true;
    state.replayMirrorProducerKnown8008EEF8 = true;
    state.replayMirrorProducerFunction = kReplayMirrorProducerFn801C8660;
    state.replayMirrorByteCountKnown8008EEF8 = true;
    state.replayMirrorKnownByteCount8008EEF8 =
        static_cast<uint32_t>(kMirrorBytes8001635C);
    state.replayMirrorFullBackingKnown8008EEF8 = true;
    for (size_t i = 0; i < state.replayMirror.size(); ++i) {
        state.replayMirror[i] = static_cast<uint8_t>(0xA0u + ((i * 3u) & 0xffu));
    }
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(
            state, 3, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(result.preflightCarrierSourceKnown);
    CHECK(!result.preflightCarrierSourceMatchesReplayAuthority);
    CHECK(!result.preflightMirrorSourceKnown);
    CHECK(!result.scratchAuthorityKnown);
    CHECK(!result.mirrorCopied);
    CheckUpdateFailurePreservedPrefix(state, before);
}

void TestUpdateSavePayload8001635CRejectsCarrierCountMismatch() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x26u);
    CHECK(ReplayMirrorSourceAuthorityMatchesState8008EEF8(state));
    CHECK(state.replayMirrorPublishedCountKnown800901BC);
    CHECK(state.replayMirrorPublishedCount800901BC == 53u);
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(
            state, 3, 3, 9, 0x11223344, true, 52u);

    CHECK(!result.ok);
    CHECK(result.preflightCarrierSourceKnown);
    CHECK(!result.preflightCarrierSourceMatchesReplayAuthority);
    CHECK(result.preflightMirrorSourceKnown);
    CHECK(result.preflightReplayMirrorProducerSourceKnown);
    CHECK(!result.scratchAuthorityKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(ReplayMirrorSourceAuthorityMatchesState8008EEF8(state));
    CheckUpdateFailurePreservedPrefix(state, before);
}

void TestLocalPayloadWritersPreserveAndInitClearsReplayRestoreProvenance() {
    MemoryState80092F10 state{};
    const Payload payload = MakePayload(0x27u);
    CHECK(LoadSavePayload800164B4(state,
                                  kTypedPayloadSourceAddress8007ADE8,
                                  payload.data(),
                                  payload.size(),
                                  MakeRuntimeLowerCardPayloadAuthority(
                                      payload.data(), payload.size()))
              .ok);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(state));

    CHECK(EnsureProgress8001628C(state, 1).ok);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(state));
    CHECK(WriteStatus800167A8(state, 1, 1).ok);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(state));
    MarkPayloadWriter80092F10(state, kFn800185D0);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::TypedCardCopy800164B4);
    CHECK(ReplayPayloadBackingAuthorityMatchesState80092F5C(state));

    CHECK(InitSavePayload80015CC4(state).ok);
    CHECK(state.replayPayloadBackingProvenance80092F5C ==
          ReplayPayloadBackingProvenance80092F5C::Unknown);
    CHECK(!ReplayPayloadBackingAuthorityMatchesState80092F5C(state));
}

void TestUpdateSavePayload8001635CCarrierGapDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x41u);
    WriteLe32Payload(state, kCarrierA3Address80092F40, 0xCAFEBABEu);
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, false, 0);

    CHECK(!result.ok);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(!result.preflightCarrierSourceKnown);
    CHECK(!result.preflightCarrierSourceMatchesReplayAuthority);
    CHECK(result.preflightMirrorSourceKnown);
    CHECK(!result.scratchAuthorityKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CheckUpdateFailurePreservedPrefix(state, before);
    CHECK(ReadLe32Payload(state, kCarrierA3Address80092F40) == 0xCAFEBABEu);
}

void TestUpdateSavePayload8001635CUnknownStatusDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x51u);
    state.statusBankKnown80092F1D = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(result.payloadKnown);
    CHECK(!result.statusBankKnown);
    CHECK(result.preflightPayloadKnown);
    CHECK(!result.preflightStatusBankKnown);
    CHECK(result.preflightCarrierSourceKnown);
    CHECK(result.preflightCarrierSourceMatchesReplayAuthority);
    CHECK(result.preflightCarrierSource == 53u);
    CHECK(result.preflightMirrorSourceKnown);
    CHECK(!result.scratchAuthorityKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CheckUpdateFailurePreservedPrefix(state, before, true, false);
}

void TestUpdateSavePayload8001635CUnmappedDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x61u);
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 99, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(!result.mapped);
    CHECK(!result.preflightMapped);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CheckUpdateFailurePreservedPrefix(state, before);
}

void TestUpdateSavePayload8001635CUnknownPayloadDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x71u);
    state.savePayloadBankKnown = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(!result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CheckUpdateFailurePreservedPrefix(state, before, false, true);
}

void TestUpdateSavePayload8001635CUnknownReplayMirrorDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x81u);
    state.replayMirrorKnown8008EEF8 = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(result.preflightCarrierSourceKnown);
    CHECK(!result.preflightCarrierSourceMatchesReplayAuthority);
    CHECK(!result.preflightMirrorSourceKnown);
    CHECK(!result.scratchAuthorityKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CheckUpdateFailurePreservedPrefix(state, before);
}

void TestUpdateSavePayload8001635CUnknownReplayProducerDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0x91u);
    state.replayMirrorProducerKnown8008EEF8 = false;
    state.replayMirrorProducerFunction = 0;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CheckUpdateFailurePreservedPrefix(state, before);
}

void TestUpdateSavePayload8001635CPartialReplayMirrorBytesDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0xA1u);
    state.replayMirrorByteCountKnown8008EEF8 = true;
    state.replayMirrorKnownByteCount8008EEF8 =
        static_cast<uint32_t>(kMirrorBytes8001635C - 8);
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CheckUpdateFailurePreservedPrefix(state, before);
}

void TestUpdateSavePayload8001635CFullBytesWithoutBackingDoesNotMutatePrefix() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0xA5u);
    state.replayMirrorFullBackingKnown8008EEF8 = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(result.payloadKnown);
    CHECK(result.statusBankKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CheckUpdateFailurePreservedPrefix(state, before);
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
                state, 3, 3, 9, 0x11223344, true, 53u);

        CHECK(!result.ok);
        CHECK(result.payloadKnown);
        CHECK(result.statusBankKnown);
        CHECK(!result.carrierSourceKnown);
        CHECK(result.carrierSource == 0u);
        CHECK(!result.mirrorSourceKnown);
        CHECK(!result.mirrorCopied);
        CHECK(!result.allClearQueried);
        CheckUpdateFailurePreservedPrefix(state, before);
    }
}

void TestUpdateSavePayload8001635CFailureClearsSourcePublishing() {
    MemoryState80092F10 state{};
    SeedKnownPayloadBank(state, 0xB1u);
    state.statusBankKnown80092F1D = false;
    const Payload before = state.savePayloadBank;

    const UpdateSavePayloadResult8001635C result =
        UpdateSavePayload8001635C(state, 3, 3, 9, 0x11223344, true, 53u);

    CHECK(!result.ok);
    CHECK(result.payloadKnown);
    CHECK(!result.statusBankKnown);
    CHECK(!result.carrierSourceKnown);
    CHECK(result.carrierSource == 0u);
    CHECK(!result.mirrorSourceKnown);
    CHECK(!result.mirrorCopied);
    CHECK(!result.allClearQueried);
    CHECK(!result.allClearWritten);
    CheckUpdateFailurePreservedPrefix(state, before, true, false);
}

} // namespace

int main() {
    TestMarkPayloadWriterCannotMintReplayRestoreProvenance();
    TestLoadSavePayload800164B4CopiesFullPrefix();
    TestLoadSavePayload800164B4CapabilityCannotCrossPayloads();
    TestSnapshotRejectsMutatedReplayPayloadBackingBytes();
    TestSnapshotRejectsForgedPublicReplayPayloadProvenance();
    TestLoadSavePayload800164B4ExactSourceWithoutAuthorityFailsClosed();
    TestLoadSavePayload800164B4ShortSourceFailsClosed();
    TestLoadSavePayload800164B4OversizeSourceFailsClosed();
    TestLoadSavePayload800164B4NullSourceFailsClosed();
    TestUpdateSavePayload8001635CSuccessCommitsAtomically();
    TestUpdateSavePayload8001635CRejectsForgedReplayMirrorMetadata();
    TestUpdateSavePayload8001635CRejectsCarrierCountMismatch();
    TestLocalPayloadWritersPreserveAndInitClearsReplayRestoreProvenance();
    TestUpdateSavePayload8001635CCarrierGapDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnknownStatusDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnmappedDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnknownPayloadDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnknownReplayMirrorDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CUnknownReplayProducerDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CPartialReplayMirrorBytesDoesNotMutatePrefix();
    TestUpdateSavePayload8001635CFullBytesWithoutBackingDoesNotMutatePrefix();
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
