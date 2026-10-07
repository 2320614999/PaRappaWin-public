#include "pr/pr_ss0_str_lifecycle_direct.h"
#include "pr/pr_ss0_mdec_output_direct.h"
#include "pr/pr_ss0_transition_direct.h"
#include "pr/pr_movie_segment_direct.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>

using namespace PrSS0StrLifecycleDirect;

namespace {

int g_failed = 0;

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);       \
            ++g_failed;                                                       \
        }                                                                     \
    } while (0)

const StrAction* FindAction(const StrPlan& plan,
                            StrActionKind kind,
                            uint32_t start = 0) {
    for (uint32_t i = start; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            return &plan.actions[i];
        }
    }
    return nullptr;
}

uint32_t CountActions(const StrPlan& plan, StrActionKind kind) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            ++count;
        }
    }
    return count;
}

uint64_t Fnv1a64(const uint8_t* bytes, size_t byteCount) {
    uint64_t hash = 1469598103934665603ull;
    for (size_t index = 0u; index < byteCount; ++index) {
        hash ^= bytes[index];
        hash *= 1099511628211ull;
    }
    return hash;
}

uint32_t IndexOfAction(const StrPlan& plan,
                       StrActionKind kind,
                       uint32_t start = 0) {
    for (uint32_t i = start; i < plan.count; ++i) {
        if (plan.actions[i].kind == kind) {
            return i;
        }
    }
    return plan.count;
}

void CheckOrder(const StrPlan& plan,
                const StrActionKind* kinds,
                uint32_t count) {
    uint32_t start = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t index = IndexOfAction(plan, kinds[i], start);
        CHECK(index < plan.count);
        start = index + 1;
    }
}

std::filesystem::path WriteSyntheticMode2StrDisc8001A4D0(
    uint8_t filterFile,
    uint8_t filterChannel) {
    constexpr int32_t kStartLba = 2;
    std::array<uint8_t, kRawCdSectorBytes8001A4D0 * 3u> disc{};
    uint8_t* raw =
        disc.data() + kStartLba * kRawCdSectorBytes8001A4D0;
    raw[0] = 0u;
    for (uint32_t index = 1u;
         index + 1u < kRawCdSyncBytes8001A4D0;
         ++index) {
        raw[index] = 0xFFu;
    }
    raw[kRawCdSyncBytes8001A4D0 - 1u] = 0u;
    raw[kRawCdHeaderOffset8001A4D0 + 0u] = 0x00u;
    raw[kRawCdHeaderOffset8001A4D0 + 1u] = 0x02u;
    raw[kRawCdHeaderOffset8001A4D0 + 2u] = 0x02u;
    raw[kRawCdHeaderOffset8001A4D0 + 3u] =
        kRawCdMode2Byte8001A4D0;
    raw[kRawCdSubheaderOffset8001A4D0 + 0u] = filterFile;
    raw[kRawCdSubheaderOffset8001A4D0 + 1u] = filterChannel;
    raw[kRawCdSubheaderOffset8001A4D0 + 2u] =
        kXaSubmodeRealtime8001A4D0 | kXaSubmodeVideo8001A4D0;
    raw[kRawCdSubheaderOffset8001A4D0 + 3u] = 0x80u;
    for (uint32_t index = 0u; index < 4u; ++index) {
        raw[kRawCdSubheaderOffset8001A4D0 + 4u + index] =
            raw[kRawCdSubheaderOffset8001A4D0 + index];
    }

    const auto path =
        std::filesystem::temp_directory_path() /
        "parappa_ss0_8001a4d0_mode2_test.bin";
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(disc.data()),
              static_cast<std::streamsize>(disc.size()));
    out.close();
    return path;
}

std::filesystem::path WriteSyntheticMode2StrFrameDisc80039670(
    uint8_t filterFile,
    uint8_t filterChannel,
    uint16_t sectorCount = 3u) {
    constexpr int32_t kStartLba = 2;
    const uint32_t rawSectorCount =
        static_cast<uint32_t>(kStartLba) + sectorCount;
    std::vector<uint8_t> disc(
        rawSectorCount * kRawCdSectorBytes8001A4D0,
        0u);
    for (uint16_t chunk = 0u; chunk < sectorCount; ++chunk) {
        uint8_t* raw = disc.data() +
            (static_cast<uint32_t>(kStartLba) + chunk) *
                kRawCdSectorBytes8001A4D0;
        raw[0] = 0u;
        for (uint32_t index = 1u;
             index + 1u < kRawCdSyncBytes8001A4D0;
             ++index) {
            raw[index] = 0xFFu;
        }
        raw[kRawCdSyncBytes8001A4D0 - 1u] = 0u;
        raw[kRawCdHeaderOffset8001A4D0 + 0u] = 0x00u;
        raw[kRawCdHeaderOffset8001A4D0 + 1u] = 0x02u;
        raw[kRawCdHeaderOffset8001A4D0 + 2u] =
            static_cast<uint8_t>(0x02u + chunk);
        raw[kRawCdHeaderOffset8001A4D0 + 3u] =
            kRawCdMode2Byte8001A4D0;
        raw[kRawCdSubheaderOffset8001A4D0 + 0u] = filterFile;
        raw[kRawCdSubheaderOffset8001A4D0 + 1u] = filterChannel;
        raw[kRawCdSubheaderOffset8001A4D0 + 2u] =
            kXaSubmodeRealtime8001A4D0 | kXaSubmodeVideo8001A4D0;
        raw[kRawCdSubheaderOffset8001A4D0 + 3u] = 0x80u;
        for (uint32_t index = 0u; index < 4u; ++index) {
            raw[kRawCdSubheaderOffset8001A4D0 + 4u + index] =
                raw[kRawCdSubheaderOffset8001A4D0 + index];
        }

        uint8_t* logical =
            raw + kRawCdSubheaderOffset8001A4D0 + 8u;
        const auto write16 = [](uint8_t* dst, uint16_t value) {
            dst[0] = static_cast<uint8_t>(value & 0xFFu);
            dst[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
        };
        const auto write32 = [](uint8_t* dst, uint32_t value) {
            dst[0] = static_cast<uint8_t>(value & 0xFFu);
            dst[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
            dst[2] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
            dst[3] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
        };
        write16(logical + 0u, kStrSectorHeaderId80039670);
        write16(logical + 2u, 0x8001u);
        write16(logical + 4u, chunk);
        write16(logical + 6u, sectorCount);
        write32(logical + 8u, 7u);
        write32(logical + 0x0Cu,
                sectorCount * kStrSectorPayloadBytes80039670);
        write16(logical + 0x10u, 320u);
        write16(logical + 0x12u, 240u);
        std::fill(logical + kStrSectorMetadataBytes80039670,
                  logical + kRawCdLogicalBytes8001A4D0,
                  static_cast<uint8_t>(0x30u + chunk));
    }

    const auto path =
        std::filesystem::temp_directory_path() /
        "parappa_ss0_80039670_sector_ring_test.bin";
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(disc.data()),
              static_cast<std::streamsize>(disc.size()));
    out.close();
    return path;
}

std::filesystem::path WriteSyntheticInterleavedXaDisc8001A4D0(
    uint8_t filterFile,
    uint8_t filterChannel) {
    constexpr int32_t kStartLba = 2;
    constexpr uint32_t kLogicalSectors = 4u;
    std::vector<uint8_t> disc(
        (static_cast<uint32_t>(kStartLba) + kLogicalSectors) *
            kRawCdSectorBytes8001A4D0,
        0u);
    for (uint32_t cursor = 0u; cursor < kLogicalSectors; ++cursor) {
        uint8_t* raw = disc.data() +
            (static_cast<uint32_t>(kStartLba) + cursor) *
                kRawCdSectorBytes8001A4D0;
        raw[0] = 0u;
        for (uint32_t index = 1u;
             index + 1u < kRawCdSyncBytes8001A4D0;
             ++index) {
            raw[index] = 0xFFu;
        }
        raw[kRawCdSyncBytes8001A4D0 - 1u] = 0u;
        raw[kRawCdHeaderOffset8001A4D0 + 0u] = 0x00u;
        raw[kRawCdHeaderOffset8001A4D0 + 1u] = 0x02u;
        raw[kRawCdHeaderOffset8001A4D0 + 2u] =
            static_cast<uint8_t>(0x02u + cursor);
        raw[kRawCdHeaderOffset8001A4D0 + 3u] =
            kRawCdMode2Byte8001A4D0;
        uint8_t* subheader =
            raw + kRawCdSubheaderOffset8001A4D0;
        subheader[0] = filterFile;
        subheader[1] = filterChannel;
        subheader[2] =
            kXaSubmodeRealtime8001A4D0 | kXaSubmodeVideo8001A4D0;
        subheader[3] = 0x80u;
        if (cursor == 1u) {
            subheader[1] = static_cast<uint8_t>(filterChannel + 1u);
            subheader[2] =
                kXaSubmodeRealtime8001A4D0 |
                kXaSubmodeAudio8001A4D0;
            subheader[3] = 0x01u;
        } else if (cursor == 2u) {
            subheader[2] = 0x08u;
        } else if (cursor == 3u) {
            subheader[2] =
                kXaSubmodeRealtime8001A4D0 |
                kXaSubmodeAudio8001A4D0;
            subheader[3] = 0x01u;
        }
        std::memcpy(subheader + 4u, subheader, 4u);
        if (cursor == 3u) {
            std::fill(subheader + 8u,
                      subheader + 8u + kXaAudioPayloadBytes8001A4D0,
                      0x5Au);
        }
    }

    const auto path =
        std::filesystem::temp_directory_path() /
        "parappa_ss0_8001a4d0_interleaved_xa_test.bin";
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(disc.data()),
              static_cast<std::streamsize>(disc.size()));
    out.close();
    return path;
}

void TestStrXaAudioDiscRuntime8001A4D0() {
    const auto discPath = WriteSyntheticInterleavedXaDisc8001A4D0(1u, 1u);
    StrLowerCdStartSource8001A4D0 source{};
    source.segmentKnown = true;
    source.startLbaKnown = true;
    source.startLba = 2;
    source.lengthBytesKnown = true;
    source.lengthBytes = 4u * kRawCdLogicalBytes8001A4D0;
    source.filterKnown = true;
    source.filterFile = 1u;
    source.filterChannel = 1u;
    source.discBinPath = discPath;
    source.discImageDirectoryAuthority = true;
    StrLowerCdStartRuntime8001A4D0 start{};
    CHECK(TryInitializeStrLowerCdStartRuntime8001A4D0(source, start));

    StrXaAudioDiscRuntime8001A4D0 runtime{};
    CHECK(TryInitializeStrXaAudioDiscRuntime8001A4D0(start, runtime));
    CHECK(runtime.initialized);
    CHECK(runtime.currentScusSemanticAuthority);
    CHECK(runtime.currentComod0CallerAuthority);
    CHECK(runtime.discImagePayloadAuthority);
    CHECK(runtime.directXaSectorSelectionAuthority);
    CHECK(!runtime.psxCdXaDecodeAuthority);
    CHECK(!runtime.psxSpuHardwareAuthority);
    CHECK(!runtime.hostExtractedFileAuthority);
    CHECK(!runtime.replayValueAuthority);
    CHECK(!runtime.oldWinS0Authority);
    CHECK(!runtime.stage2PlusAuthority);
    CHECK(!runtime.comod2Authority);

    const auto beforeAudio = ReadNextStrXaAudioSectorFromDisc8001A4D0(
        start, 3u, runtime);
    CHECK(beforeAudio.known);
    CHECK(!beforeAudio.available);
    CHECK(beforeAudio.rawSectorCursorBefore == 0u);
    CHECK(beforeAudio.rawSectorCursorAfter == 3u);
    CHECK(beforeAudio.rawSectorsScanned == 3u);
    CHECK(beforeAudio.targetCursorReached);
    CHECK(runtime.filteredAudioSectors == 0u);

    const auto audio = ReadNextStrXaAudioSectorFromDisc8001A4D0(
        start, 4u, runtime);
    CHECK(audio.known);
    CHECK(audio.available);
    CHECK(audio.rawSectorCursorBefore == 3u);
    CHECK(audio.rawSectorCursorAfter == 4u);
    CHECK(audio.matchedRawSectorCursor == 3u);
    CHECK(audio.rawSectorsScanned == 1u);
    CHECK(audio.targetCursorReached);
    CHECK(audio.file == 1u);
    CHECK(audio.channel == 1u);
    CHECK(audio.submode == 0x44u);
    CHECK(audio.coding == 0x01u);
    CHECK(audio.payload.front() == 0x5Au);
    CHECK(audio.payload.back() == 0x5Au);
    CHECK(audio.syncHeaderMatched);
    CHECK(audio.mode2Matched);
    CHECK(audio.xaSubheaderDuplicateMatched);
    CHECK(audio.xaFilterMatched);
    CHECK(audio.xaRealtimeAudioMatched);
    CHECK(audio.currentScusSemanticAuthority);
    CHECK(audio.currentComod0CallerAuthority);
    CHECK(audio.discImagePayloadAuthority);
    CHECK(audio.directXaSectorSelectionAuthority);
    CHECK(!audio.psxCdXaDecodeAuthority);
    CHECK(!audio.psxSpuHardwareAuthority);
    CHECK(!audio.hostExtractedFileAuthority);
    CHECK(!audio.replayValueAuthority);
    CHECK(!audio.oldWinS0Authority);
    CHECK(!audio.stage2PlusAuthority);
    CHECK(!audio.comod2Authority);
    CHECK(runtime.rawSectorsScanned == 4u);
    CHECK(runtime.filteredAudioSectors == 1u);

    runtime.hostExtractedFileAuthority = true;
    CHECK(!ReadNextStrXaAudioSectorFromDisc8001A4D0(
               start, 4u, runtime).known);
    std::error_code ec;
    std::filesystem::remove(discPath, ec);
}

void TestStrLowerCdStartRuntime8001A4D0() {
    const auto discPath = WriteSyntheticMode2StrDisc8001A4D0(1u, 1u);
    StrLowerCdStartSource8001A4D0 source{};
    source.segmentKnown = true;
    source.startLbaKnown = true;
    source.startLba = 2;
    source.lengthBytesKnown = true;
    source.lengthBytes = 2048u;
    source.filterKnown = true;
    source.filterFile = 1u;
    source.filterChannel = 1u;
    source.discBinPath = discPath;
    source.discImageDirectoryAuthority = true;

    StrLowerCdStartRuntime8001A4D0 state{};
    CHECK(TryInitializeStrLowerCdStartRuntime8001A4D0(source, state));
    CHECK(state.initialized);
    CHECK(state.sourceFunction == kFn8001A4D0);
    CHECK(state.startLba == 2);
    CHECK(state.lengthBytes == 2048u);
    CHECK(state.logicalSectorCount == 1u);
    CHECK(state.rawSectorReadable);
    CHECK(state.syncHeaderMatched);
    CHECK(state.headerLbaMatched);
    CHECK(state.mode2Matched);
    CHECK(state.xaSubheaderDuplicateMatched);
    CHECK(state.xaFilterMatched);
    CHECK(state.xaRealtimeVideoMatched);
    CHECK(state.filterFile == 1u);
    CHECK(state.filterChannel == 1u);
    CHECK(state.xaSubmode == 0x42u);
    CHECK(state.xaCoding == 0x80u);
    CHECK(state.setlocComplete800364D0);
    CHECK(state.setfilterComplete800364D0);
    CHECK(state.setmodeReadSComplete800391AC);
    CHECK(state.startWaitFrames80035560 == 3u);
    CHECK(state.streamStartComplete800364D0);
    CHECK(state.startFlagsWritten8001A4D0);
    CHECK(state.dword80049410 == 1);
    CHECK(state.dword80049420 == -16);
    CHECK(state.primeStatusCalled80036678);
    CHECK(state.commandWrapper80036678Succeeded);
    CHECK(state.statusPollInputKnown8001A750);
    CHECK(state.syncReturn800364D0 == 2);
    CHECK(state.status0 == 0x20u);
    CHECK(state.exactCallOrder);
    CHECK(state.currentScusSemanticAuthority);
    CHECK(state.currentComod0CallerAuthority);
    CHECK(state.discImageDirectoryAuthority);
    CHECK(state.discImagePayloadAuthority);
    CHECK(state.directPsxStatusAuthority);
    CHECK(!state.hardwareCallbackAuthority);
    CHECK(!state.hostProjection);
    CHECK(!state.hostExtractedFileAuthority);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);

    PrMovieSegmentDirect::StreamStatusPollInput8001A750 pollInput{};
    pollInput.sub800364D0Known = state.statusPollInputKnown8001A750;
    pollInput.sub800364D0Result = state.syncReturn800364D0;
    pollInput.statusBytesKnown = state.statusPollInputKnown8001A750;
    pollInput.statusBytes[0] = state.status0;
    const auto poll =
        PrMovieSegmentDirect::PsxCall8001A750_StreamStatusPoll(pollInput);
    CHECK(poll.resultKnown);
    CHECK(poll.psxReturn == 1);
    CHECK(!poll.requestedCommandWrapper80036678);

    source.discImageDirectoryAuthority = false;
    CHECK(!TryInitializeStrLowerCdStartRuntime8001A4D0(source, state));
    CHECK(!state.initialized);
    source.discImageDirectoryAuthority = true;
    source.filterChannel = 2u;
    CHECK(!TryInitializeStrLowerCdStartRuntime8001A4D0(source, state));
    CHECK(!state.initialized);
    std::error_code ec;
    std::filesystem::remove(discPath, ec);
}

void TestStrWorkBaseCleanupRuntime80049428() {
    StrWorkBaseRuntime80049428 workBase{};
    CHECK(TryInitializeStrWorkBaseRuntime80049428(workBase));
    CHECK(workBase.initialized);
    CHECK(workBase.sourceAddress == 0x80049428u);
    CHECK(workBase.dword80049428Known);
    CHECK(workBase.dword80049428 == 0);
    CHECK(!workBase.byte80057119Known);
    CHECK(workBase.scusLoadedInitialZeroAuthority);
    CHECK(workBase.currentScusSemanticAuthority);
    CHECK(workBase.currentComod0CallerAuthority);
    CHECK(workBase.directLowerCdStateAuthority);
    CHECK(!workBase.hostProjection);
    CHECK(!workBase.replayValueAuthority);
    CHECK(!workBase.oldWinS0Authority);
    CHECK(!workBase.stage2PlusAuthority);
    CHECK(!workBase.comod2Authority);

    const auto initialRead = ReadStrWorkBase8001A3B8(workBase);
    CHECK(initialRead.known);
    CHECK(initialRead.called);
    CHECK(initialRead.sourceFunction == 0x8001A3B8u);
    CHECK(initialRead.sourceAddress == 0x80049428u);
    CHECK(initialRead.valueKnown);
    CHECK(initialRead.value == 0);
    CHECK(!initialRead.returnsOne);
    CHECK(initialRead.readOnly);
    CHECK(initialRead.currentScusSemanticAuthority);
    CHECK(initialRead.currentComod0CallerAuthority);
    CHECK(initialRead.directLowerCdStateAuthority);
    CHECK(!initialRead.hardwareCallbackAuthority);
    CHECK(!initialRead.hostProjection);
    CHECK(!initialRead.replayValueAuthority);
    CHECK(!initialRead.oldWinS0Authority);
    CHECK(!initialRead.stage2PlusAuthority);
    CHECK(!initialRead.comod2Authority);

    auto syncCallback = InitializeStrCdSyncCallbackRuntime800570F8();
    CHECK(syncCallback.initialized);
    CHECK(syncCallback.callbackKnown);
    CHECK(syncCallback.callback == 0u);
    CHECK(syncCallback.scusLoadedInitialZeroAuthority);
    CHECK(syncCallback.currentScusSemanticAuthority);
    CHECK(syncCallback.directCallbackStateAuthority);
    const auto installCallback =
        SetStrCdSyncCallback80036510(syncCallback, 0x80039318u);
    CHECK(installCallback.known);
    CHECK(installCallback.called);
    CHECK(installCallback.sourceFunction == 0x80036510u);
    CHECK(installCallback.sourceAddress == 0x800570F8u);
    CHECK(installCallback.priorCallbackKnown);
    CHECK(installCallback.priorCallback == 0u);
    CHECK(installCallback.callbackWritten);
    CHECK(installCallback.readBeforeWrite);
    CHECK(syncCallback.callback == 0x80039318u);
    const auto clearCallback =
        SetStrCdSyncCallback80036510(syncCallback, 0u);
    CHECK(clearCallback.known);
    CHECK(clearCallback.priorCallback == 0x80039318u);
    CHECK(syncCallback.callback == 0u);
    syncCallback.hostProjection = true;
    const auto rejectedCallback =
        SetStrCdSyncCallback80036510(syncCallback, 0x80039240u);
    CHECK(!rejectedCallback.known);
    CHECK(syncCallback.callback == 0u);

    CHECK(!ResolveStrPostUploadDecision801C455C(
               workBase, false, true, true).known);

    const auto continueDecision = ResolveStrPostUploadDecision801C455C(
        workBase, true, true, true);
    CHECK(continueDecision.known);
    CHECK(continueDecision.workBaseRead.known);
    CHECK(!continueDecision.specialWorkBaseExit);
    CHECK(!continueDecision.callWaitCleanup8001A694);
    CHECK(!continueDecision.callStopCallback80035838);
    CHECK(!continueDecision.specialCleanupDirectExecutable);
    CHECK(!continueDecision.gapSpecialCleanupNotYetDirect);
    CHECK(continueDecision.continueLoop);
    CHECK(!continueDecision.terminateFromPlayerTick);
    CHECK(continueDecision.exactBranchOrder);
    CHECK(continueDecision.directPlayerTickAuthority);
    CHECK(continueDecision.hostProjectionResultIgnored);
    CHECK(!continueDecision.hostProjection);

    const auto terminalDecision = ResolveStrPostUploadDecision801C455C(
        workBase, true, true, false);
    CHECK(terminalDecision.known);
    CHECK(!terminalDecision.continueLoop);
    CHECK(terminalDecision.terminateFromPlayerTick);
    CHECK(terminalDecision.directPlayerTickAuthority);
    CHECK(terminalDecision.hostProjectionResultIgnored);
    CHECK(!terminalDecision.hostProjection);

    const auto coldCleanup = ApplyStrCleanup8001A280(workBase);
    CHECK(coldCleanup.known);
    CHECK(coldCleanup.called);
    CHECK(coldCleanup.dword80049428Known);
    CHECK(coldCleanup.dword80049428 == 0);
    CHECK(!coldCleanup.skippedNonZeroWorkBase);
    CHECK(coldCleanup.commandWrapperCalled80036678);
    CHECK(coldCleanup.command80036678 == 0x10u);
    CHECK(coldCleanup.commandWrapperSucceeded80036678);
    CHECK(coldCleanup.commandSinkCalled800375BC);
    CHECK(coldCleanup.commandSinkSkipWait800375BC);
    CHECK(coldCleanup.byte80057119Written);
    CHECK(coldCleanup.byte80057119 == 0x10u);
    CHECK(coldCleanup.directCommandSinkAuthority);
    CHECK(!coldCleanup.hardwareCallbackAuthority);
    CHECK(workBase.byte80057119Known);
    CHECK(workBase.byte80057119 == 0x10u);

    const auto discPath = WriteSyntheticMode2StrDisc8001A4D0(1u, 1u);
    StrLowerCdStartSource8001A4D0 startSource{};
    startSource.segmentKnown = true;
    startSource.startLbaKnown = true;
    startSource.startLba = 2;
    startSource.lengthBytesKnown = true;
    startSource.lengthBytes = 2048u;
    startSource.filterKnown = true;
    startSource.filterFile = 1u;
    startSource.filterChannel = 1u;
    startSource.discBinPath = discPath;
    startSource.discImageDirectoryAuthority = true;
    StrLowerCdStartRuntime8001A4D0 start{};
    CHECK(TryInitializeStrLowerCdStartRuntime8001A4D0(
        startSource, start));

    CHECK(TryInitializeStrWorkBaseRuntime80049428(workBase));
    CHECK(PublishStrLowerCdStartCommand8001A4D0(start, workBase));
    CHECK(workBase.byte80057119Known);
    CHECK(workBase.byte80057119 == 1u);

    const auto startMsf =
        PrMovieSegmentDirect::PsxCall80036974_LbaToMsf(2);
    CHECK(startMsf.known);
    CHECK(startMsf.msf.minute == 0x00u);
    CHECK(startMsf.msf.second == 0x02u);
    CHECK(startMsf.msf.frame == 0x02u);
    const auto startLba =
        PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(startMsf.msf);
    CHECK(startLba.known);
    CHECK(startLba.lba == 2);

    StrLowerCdClockSource801C4350 clockSource{};
    clockSource.segmentTimeBaseKnown = true;
    clockSource.segmentTimeBase = 2;
    clockSource.segmentEndKnown = true;
    clockSource.segmentEnd = 20;
    clockSource.segmentEndBiasKnown = true;
    clockSource.segmentEndBias = 0;
    clockSource.discImageDirectoryAuthority = true;
    StrLowerCdClockRuntime801C4350 clock{};
    CHECK(TryInitializeStrLowerCdClockRuntime801C4350(
        clockSource, clock));

    const auto firstTick = StepStrPlayerTick801C448C(workBase, clock);
    CHECK(firstTick.known);
    CHECK(firstTick.vblankWaitCalled80035560);
    CHECK(firstTick.vblankWaitMode80035560 == 2);
    CHECK(firstTick.clockPollCalled8001A3C8);
    CHECK(firstTick.currentLba == 7);
    CHECK(firstTick.lbaToMsfCalled80036974);
    CHECK(firstTick.syncFeedbackKnown800364D0);
    CHECK(firstTick.syncReturn800364D0 == 2);
    CHECK(firstTick.syncMinuteBcd == 0x00u);
    CHECK(firstTick.syncSecondBcd == 0x02u);
    CHECK(firstTick.syncFrameBcd == 0x07u);
    CHECK(firstTick.readyStatusKnown800363A4);
    CHECK(firstTick.readyStatus800363A4 == 1u);
    CHECK(firstTick.clockPollAccepted8001A3C8);
    CHECK(firstTick.dword80049428Known);
    CHECK(firstTick.dword80049428 == 0);
    CHECK(firstTick.cleanup.known);
    CHECK(firstTick.cleanup.command80036678 == 0x10u);
    CHECK(firstTick.clock.known);
    CHECK(firstTick.clock.currentLba == 7);
    CHECK(firstTick.exactCallOrder);
    CHECK(firstTick.currentScusSemanticAuthority);
    CHECK(firstTick.currentComod0CallerAuthority);
    CHECK(firstTick.directLowerCdStateAuthority);
    CHECK(!firstTick.hardwareCallbackAuthority);
    CHECK(!firstTick.hostProjection);
    CHECK(!firstTick.replayValueAuthority);
    CHECK(!firstTick.oldWinS0Authority);
    CHECK(!firstTick.stage2PlusAuthority);
    CHECK(!firstTick.comod2Authority);
    CHECK(workBase.clockPollCount8001A3C8 == 1u);
    CHECK(workBase.cleanupCallCount8001A280 == 1u);
    CHECK(workBase.byte80057119 == 0x10u);

    const auto secondTick = StepStrPlayerTick801C448C(workBase, clock);
    CHECK(secondTick.known);
    CHECK(secondTick.currentLba == 12);
    CHECK(secondTick.readyStatus800363A4 == 0x10u);
    CHECK(secondTick.clockPollAccepted8001A3C8);
    CHECK(secondTick.cleanup.command80036678 == 0x10u);

    workBase.dword80049428 = 1;
    const auto exitRead = ReadStrWorkBase8001A3B8(workBase);
    CHECK(exitRead.known);
    CHECK(exitRead.value == 1);
    CHECK(exitRead.returnsOne);
    const auto specialExitDecision =
        ResolveStrPostUploadDecision801C455C(workBase, true, true, true);
    CHECK(specialExitDecision.known);
    CHECK(specialExitDecision.specialWorkBaseExit);
    CHECK(specialExitDecision.callWaitCleanup8001A694);
    CHECK(specialExitDecision.callStopCallback80035838);
    CHECK(specialExitDecision.specialCleanupDirectExecutable);
    CHECK(!specialExitDecision.gapSpecialCleanupNotYetDirect);
    CHECK(!specialExitDecision.continueLoop);
    CHECK(!specialExitDecision.terminateFromPlayerTick);
    CHECK(specialExitDecision.exactBranchOrder);
    const auto skippedCleanup = ApplyStrCleanup8001A280(workBase);
    CHECK(skippedCleanup.known);
    CHECK(skippedCleanup.skippedNonZeroWorkBase);
    CHECK(!skippedCleanup.commandWrapperCalled80036678);
    CHECK(!skippedCleanup.commandSinkCalled800375BC);
    CHECK(skippedCleanup.directCommandSinkAuthority);

    workBase.hostProjection = true;
    CHECK(!ReadStrWorkBase8001A3B8(workBase).known);
    CHECK(!ResolveStrPostUploadDecision801C455C(
               workBase, true, true, true).known);
    CHECK(!ApplyStrCleanup8001A280(workBase).known);

    std::error_code ec;
    std::filesystem::remove(discPath, ec);
}

void TestExactMovie0TDisc8001A4D0(
    const std::filesystem::path& discPath) {
    StrLowerCdStartSource8001A4D0 source{};
    source.segmentKnown = true;
    source.startLbaKnown = true;
    source.startLba = 280924;
    source.lengthBytesKnown = true;
    source.lengthBytes = 4061184u;
    source.filterKnown = true;
    source.filterFile = 1u;
    source.filterChannel = 1u;
    source.discBinPath = discPath;
    source.discImageDirectoryAuthority = true;

    StrLowerCdStartRuntime8001A4D0 state{};
    CHECK(TryInitializeStrLowerCdStartRuntime8001A4D0(source, state));
    CHECK(state.initialized);
    CHECK(state.startLba == 280924);
    CHECK(state.lengthBytes == 4061184u);
    CHECK(state.logicalSectorCount == 1983u);
    CHECK(state.filterFile == 1u);
    CHECK(state.filterChannel == 1u);
    CHECK(state.xaSubmode == 0x42u);
    CHECK(state.xaCoding == 0x80u);
    CHECK(state.syncReturn800364D0 == 2);
    CHECK(state.status0 == 0x20u);
    CHECK(state.directPsxStatusAuthority);
    CHECK(state.discImagePayloadAuthority);
    CHECK(!state.hostProjection);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);

    StrXaAudioDiscRuntime8001A4D0 xaRuntime{};
    CHECK(TryInitializeStrXaAudioDiscRuntime8001A4D0(
        state, xaRuntime));
    const auto firstXa = ReadNextStrXaAudioSectorFromDisc8001A4D0(
        state, 16u, xaRuntime);
    CHECK(firstXa.known);
    CHECK(firstXa.available);
    CHECK(firstXa.rawSectorCursorBefore == 0u);
    CHECK(firstXa.rawSectorCursorAfter == 16u);
    CHECK(firstXa.matchedRawSectorCursor == 15u);
    CHECK(firstXa.rawSectorsScanned == 16u);
    CHECK(firstXa.file == 1u);
    CHECK(firstXa.channel == 1u);
    CHECK(firstXa.submode == 0x64u);
    CHECK(firstXa.coding == 0x00u);
    CHECK(Fnv1a64(firstXa.payload.data(), firstXa.payload.size()) ==
          0x406A58A2F8B81C8Eull);
    CHECK(firstXa.directXaSectorSelectionAuthority);
    CHECK(!firstXa.psxCdXaDecodeAuthority);
    CHECK(!firstXa.psxSpuHardwareAuthority);
    CHECK(!firstXa.hostExtractedFileAuthority);
    CHECK(!firstXa.replayValueAuthority);
    CHECK(!firstXa.oldWinS0Authority);
    CHECK(!firstXa.stage2PlusAuthority);
    CHECK(!firstXa.comod2Authority);

    StrDecoderAllocationState80027288 decoder{};
    CHECK(TryInitializeStrDecoderState80027288(1, false, decoder));
    StrDecoderMemoryRuntime80027288 runtime{};
    CHECK(TryInitializeStrDecoderMemoryRuntime80027288(decoder, runtime));
    CHECK(PublishStrDecoderStreamingStart8001A4D0(state, runtime));
    const StrSectorRingPrimeResult80039670 prime =
        PrimeStrSectorRingFromDisc80039670(state, runtime);
    CHECK(prime.known);
    CHECK(prime.framePublished);
    CHECK(prime.rawSectorsScanned == 10u);
    CHECK(prime.filteredVideoSectors == 10u);
    CHECK(prime.rejectedSectors == 0u);
    CHECK(prime.firstSlot == 0u);
    CHECK(prime.sectorCount == 10u);
    CHECK(prime.frameNumber == 1u);
    CHECK(prime.width == 320u);
    CHECK(prime.height == 240u);
    CHECK(prime.payloadByteOffset == 1024u);
    CHECK(prime.directDiscSectorAuthority);
    CHECK(prime.synchronousCallbackOrdering);
    CHECK(!prime.hardwareCallbackTimingAuthority);
    CHECK(!prime.psxPointerAuthority);
    CHECK(!prime.hostProjection);
    CHECK(!prime.replayValueAuthority);
    CHECK(!prime.oldWinS0Authority);
    CHECK(!prime.stage2PlusAuthority);
    CHECK(!prime.comod2Authority);
    const StrSectorRingAcquireResult8003958C acquire =
        AcquireStrSectorRingFrame8003958C(runtime);
    CHECK(acquire.known);
    CHECK(acquire.available);
    CHECK(acquire.returnValue == 0);
    CHECK(acquire.payloadHandle == 1024u);
    const StrVlcDecodeReleaseResult800273A4 vlcRelease =
        ExecuteStrVlcDecodeAndRelease800273A4(
            acquire.payloadHandle,
            runtime);
    CHECK(vlcRelease.known);
    CHECK(vlcRelease.executed);
    CHECK(vlcRelease.inputBytes == 20160u);
    CHECK(vlcRelease.vlc.known);
    CHECK(vlcRelease.vlc.executed);
    CHECK(vlcRelease.vlc.returnValue == 0);
    CHECK(vlcRelease.vlc.frameCodeCount == 0x0DC0u);
    CHECK(vlcRelease.vlc.frameMagic == 0x3800u);
    CHECK(vlcRelease.vlc.quantScale == 1u);
    CHECK(vlcRelease.vlc.version == 2u);
    CHECK(vlcRelease.vlc.frameComplete);
    CHECK(vlcRelease.vlc.trailingFe00Halfwords == 65u);
    CHECK(vlcRelease.vlc.outputBytesWritten > 134u);
    CHECK(vlcRelease.vlc.outputBytesWritten <=
          runtime.allocations[2].size());
    CHECK(vlcRelease.releaseFrameCalled80039490);
    CHECK(vlcRelease.release.known);
    CHECK(vlcRelease.release.released);
    CHECK(vlcRelease.release.returnValue == 0);
    CHECK(vlcRelease.release.stateTransition4To0);
    CHECK(vlcRelease.exactCurrentIdaCallOrder);
    CHECK(vlcRelease.currentScusSemanticAuthority);
    CHECK(vlcRelease.directSectorRingAuthority);
    CHECK(!vlcRelease.mdecHardwareExecutionAuthority);
    uint64_t outputFnv1a = 1469598103934665603ull;
    for (uint32_t index = 0u;
         index < vlcRelease.vlc.outputBytesWritten;
         ++index) {
        outputFnv1a ^= runtime.allocations[2][index];
        outputFnv1a *= 1099511628211ull;
    }
    std::printf(
        "test_ss0_str_lifecycle: exact MOVIE0T VLC input=%u output=%u fnv1a=%016llX\n",
        vlcRelease.inputBytes,
        vlcRelease.vlc.outputBytesWritten,
        static_cast<unsigned long long>(outputFnv1a));
    CHECK(vlcRelease.vlc.outputBytesWritten == 14158u);
    CHECK(outputFnv1a == 0x4E8ADEB4531E27AFull);
    StrDecoderControlState80027288 mdecControl = decoder.control;
    mdecControl.decodedWidth = acquire.width;
    mdecControl.decodedHeight = acquire.height;
    mdecControl.vlcDecodeResult = 0u;
    mdecControl.decodeInFlight = 1u;
    const StrMdecCommandOutputSubmission800273A4 mdecSubmission =
        ExecuteStrMdecCommandOutputSubmission800273A4(
            mdecControl,
            vlcRelease,
            runtime);
    CHECK(mdecSubmission.known);
    CHECK(mdecSubmission.executed);
    CHECK(mdecSubmission.controlArgument80047558 == 2u);
    CHECK(mdecSubmission.vlcCommandWordBefore == 0x38000DC0u);
    CHECK(mdecSubmission.vlcCommandWordAfter == 0x3A000DC0u);
    CHECK(mdecSubmission.inputDmaWordCount80047778 == 0x0DC0u);
    CHECK(mdecSubmission.inputSyncCalled8004789C);
    CHECK(mdecSubmission.dmaPriorityOrMask == 0x88u);
    CHECK(mdecSubmission.inputMadrByteOffset == 4u);
    CHECK(mdecSubmission.inputBcr == 0x006E0020u);
    CHECK(mdecSubmission.inputChcr == 0x01000201u);
    CHECK(mdecSubmission.mdecCommandPortWritten);
    CHECK(mdecSubmission.outputSyncCalled80047934);
    CHECK(mdecSubmission.outputDmaWordCount8004780C == 38400u);
    CHECK(mdecSubmission.outputMadrByteOffset == 0u);
    CHECK(mdecSubmission.outputBcr == 0x04B00020u);
    CHECK(mdecSubmission.outputChcr == 0x01000200u);
    CHECK(mdecSubmission.outputReadyStored800273A4);
    CHECK(mdecSubmission.dmaCompletionCallbackPending80027220);
    CHECK(mdecSubmission.outputBufferUntouched);
    CHECK(mdecSubmission.exactCurrentIdaCallOrder);
    CHECK(mdecSubmission.currentScusSemanticAuthority);
    CHECK(mdecSubmission.directCommandStateAuthority);
    CHECK(!mdecSubmission.mdecHardwareExecutionAuthority);
    CHECK(!mdecSubmission.hardwareCallbackTimingAuthority);
    CHECK(!mdecSubmission.psxPointerAuthority);
    CHECK(!mdecSubmission.psxHardwareMmioAuthority);
    CHECK(!mdecSubmission.hostProjection);
    CHECK(!mdecSubmission.replayValueAuthority);
    CHECK(!mdecSubmission.oldWinS0Authority);
    CHECK(!mdecSubmission.stage2PlusAuthority);
    CHECK(!mdecSubmission.comod2Authority);
    CHECK(runtime.allocations[2][0] == 0xC0u);
    CHECK(runtime.allocations[2][1] == 0x0Du);
    CHECK(runtime.allocations[2][2] == 0x00u);
    CHECK(runtime.allocations[2][3] == 0x3Au);
    CHECK(std::all_of(
        runtime.allocations[3].begin(),
        runtime.allocations[3].end(),
        [](uint8_t value) { return value == 0u; }));
    const PrSS0MdecOutputDirect::Mdec15bppResult mdecOutput =
        PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
            runtime.allocations[2].data(),
            vlcRelease.vlc.outputBytesWritten,
            acquire.width,
            acquire.height,
            runtime.allocations[3].data(),
            runtime.allocations[3].size());
    CHECK(mdecOutput.known);
    CHECK(mdecOutput.executed);
    CHECK(mdecOutput.commandWord == 0x3A000DC0u);
    CHECK(mdecOutput.inputWordCount == 0x0DC0u);
    CHECK(mdecOutput.inputHalfwordsConsumed <= 0x1B80u);
    CHECK(mdecOutput.width == 320u);
    CHECK(mdecOutput.height == 240u);
    CHECK(mdecOutput.macroblockColumns == 20u);
    CHECK(mdecOutput.macroblockRows == 15u);
    CHECK(mdecOutput.macroblocksDecoded == 300u);
    CHECK(mdecOutput.outputBytesWritten == 153600u);
    CHECK(mdecOutput.inputHalfwordsConsumed == 7012u);
    CHECK(mdecOutput.outputFnv1a64 == 0x7E088BE6E7CFE1CFull);
    CHECK(mdecOutput.output15bpp);
    CHECK(mdecOutput.outputUnsigned);
    CHECK(mdecOutput.outputBit15Set);
    CHECK(mdecOutput.dma1VerticalMacroblockLayout);
    CHECK(mdecOutput.currentScusQuantTableAuthority);
    CHECK(mdecOutput.currentScusScaleTableAuthority);
    CHECK(mdecOutput.currentIdaCommandAuthority);
    CHECK(mdecOutput.documentedPsxMdecAlgorithmAuthority);
    CHECK(!mdecOutput.mdecHardwareBitExactAuthority);
    CHECK(!mdecOutput.dmaCompletionCallbackTimingAuthority);
    CHECK(!mdecOutput.psxHardwareMmioAuthority);
    CHECK(!mdecOutput.hostProjection);
    CHECK(!mdecOutput.replayValueAuthority);
    CHECK(!mdecOutput.oldWinS0Authority);
    CHECK(!mdecOutput.stage2PlusAuthority);
    CHECK(!mdecOutput.comod2Authority);
    CHECK(std::any_of(
        runtime.allocations[3].begin(),
        runtime.allocations[3].end(),
        [](uint8_t value) { return value != 0u; }));
    std::printf(
        "test_ss0_str_lifecycle: exact MOVIE0T MDEC halfwords=%u output=%u fnv1a=%016llX\n",
        mdecOutput.inputHalfwordsConsumed,
        mdecOutput.outputBytesWritten,
        static_cast<unsigned long long>(mdecOutput.outputFnv1a64));
    StrDecoderControlState80027288 completionControl = mdecControl;
    completionControl.outputReady = 1u;
    const StrMdecDmaCompletionState80027220 completion =
        ExecuteStrMdecDmaCompletion80027220(
            completionControl, mdecSubmission, mdecOutput, runtime);
    CHECK(completion.known);
    CHECK(completion.executed);
    CHECK(completion.controlBefore.outputReady == 1u);
    CHECK(completion.controlAfter.outputReady == 0u);
    CHECK(completion.controlAfter.decodeInFlight == 1u);
    CHECK(completion.dmaChannelOneCallbackInstalled);
    CHECK(completion.callbackFunction80027220 == kFn80027220);
    CHECK(completion.callbackDmaChannel == 1u);
    CHECK(completion.completedOutputValidated);
    CHECK(completion.completedOutputBytes == 153600u);
    CHECK(completion.outputReadyCleared);
    CHECK(completion.decodeInFlightPreserved);
    CHECK(completion.exactCurrentIdaStoreAuthority);
    CHECK(completion.directSynchronousOutputCompletionAuthority);
    CHECK(!completion.hardwareCallbackTimingAuthority);
    CHECK(!completion.psxPointerAuthority);
    CHECK(!completion.psxHardwareMmioAuthority);
    CHECK(!completion.hostProjection);
    CHECK(!completion.replayValueAuthority);
    CHECK(!completion.oldWinS0Authority);
    CHECK(!completion.stage2PlusAuthority);
    CHECK(!completion.comod2Authority);
    const StrDecodeGateState80027528 firstPollGate =
        BuildStrDecodeGateState80027528(completion.controlAfter);
    CHECK(firstPollGate.known);
    CHECK(firstPollGate.branch ==
          StrDecodeGateBranch80027528::OutputNotReady);
    CHECK(firstPollGate.callDecodeAdvance800273A4);

    const StrSectorRingPrimeResult80039670 secondPrime =
        PrimeStrSectorRingFromDisc80039670(state, runtime);
    CHECK(secondPrime.known);
    CHECK(secondPrime.framePublished);
    CHECK(secondPrime.frameNumber == 2u);
    CHECK(secondPrime.sectorCount == 9u);
    CHECK(secondPrime.width == 320u);
    CHECK(secondPrime.height == 240u);
    const StrSectorRingAcquireResult8003958C secondAcquire =
        AcquireStrSectorRingFrame8003958C(runtime);
    CHECK(secondAcquire.known);
    CHECK(secondAcquire.available);
    CHECK(secondAcquire.returnValue == 0);
    CHECK(secondAcquire.frameNumber == 2u);
    const StrVlcDecodeReleaseResult800273A4 secondVlcRelease =
        ExecuteStrVlcDecodeAndRelease800273A4(
            secondAcquire.payloadHandle, runtime);
    CHECK(secondVlcRelease.known);
    CHECK(secondVlcRelease.executed);
    CHECK(secondVlcRelease.vlc.known);
    CHECK(secondVlcRelease.vlc.executed);
    CHECK(secondVlcRelease.vlc.returnValue == 0);
    CHECK(secondVlcRelease.vlc.outputBytesWritten == 15738u);
    StrDecoderControlState80027288 secondMdecControl =
        completion.controlAfter;
    secondMdecControl.decodedWidth = secondAcquire.width;
    secondMdecControl.decodedHeight = secondAcquire.height;
    secondMdecControl.vlcDecodeResult = 0u;
    secondMdecControl.decodeInFlight = 1u;
    const StrMdecCommandOutputSubmission800273A4 secondSubmission =
        ExecuteStrMdecCommandOutputSubmission800273A4(
            secondMdecControl, secondVlcRelease, runtime);
    CHECK(secondSubmission.known);
    CHECK(secondSubmission.executed);
    const PrSS0MdecOutputDirect::Mdec15bppResult secondOutput =
        PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
            runtime.allocations[2].data(),
            secondVlcRelease.vlc.outputBytesWritten,
            secondAcquire.width,
            secondAcquire.height,
            runtime.allocations[3].data(),
            runtime.allocations[3].size());
    CHECK(secondOutput.known);
    CHECK(secondOutput.executed);
    CHECK(secondOutput.macroblocksDecoded == 300u);
    CHECK(secondOutput.inputHalfwordsConsumed == 7802u);
    CHECK(secondOutput.outputBytesWritten == 153600u);
    CHECK(secondOutput.outputFnv1a64 == 0xDC3FAA01654ED12Eull);
    StrDecoderControlState80027288 secondUploadControl =
        secondMdecControl;
    secondUploadControl.outputReady = 1u;
    const StrStripUploadState8002756C secondUploadPlan =
        BuildStrStripUploadState8002756C(
            secondUploadControl, 2u, true, 0);
    CHECK(secondUploadPlan.known);
    CHECK(!secondUploadPlan.skippedForCounterWarmup);
    CHECK(secondUploadPlan.destinationY == secondUploadControl.dstY);
    CHECK(secondUploadPlan.stripCount == 20u);
    CHECK(secondUploadPlan.uploadCallsPlanned80044D64);
    const StrStripUploadExecution8002756C secondUpload =
        ExecuteStrStripUpload8002756C(
            secondUploadPlan,
            runtime.allocations[3].data(),
            secondOutput.outputBytesWritten);
    CHECK(secondUpload.known);
    CHECK(secondUpload.executed);
    CHECK(secondUpload.width == 320u);
    CHECK(secondUpload.height == 240u);
    CHECK(secondUpload.stripCount == 20u);
    CHECK(secondUpload.commandsExecuted80044D64 == 20u);
    CHECK(secondUpload.sourceBytesConsumed == 153600u);
    CHECK(secondUpload.rgbaBytesWritten == 307200u);
    CHECK(secondUpload.rgbaPixels.size() == 76800u);
    CHECK(secondUpload.sourceFnv1a64 == secondOutput.outputFnv1a64);
    CHECK(secondUpload.rgbaFnv1a64 == 0xF7CE58BC2EEDA505ull);
    CHECK(secondUpload.exactCurrentIdaStripOrder);
    CHECK(secondUpload.directMdecVerticalLayoutAuthority);
    CHECK(secondUpload.directRgbaAssemblyAuthority);
    CHECK(secondUpload.translatedGpuDmaStateAuthority);
    CHECK(secondUpload.translatedGpuCompletionAuthority);
    CHECK(secondUpload.translatedGpuCommandCount == secondUpload.stripCount);
    CHECK(secondUpload.hostTextureUploadPending);
    CHECK(!secondUpload.hostTextureUploadExecuted);
    CHECK(!secondUpload.psxGpuDmaExecutionAuthority);
    CHECK(!secondUpload.psxPointerAuthority);
    CHECK(!secondUpload.replayValueAuthority);
    CHECK(!secondUpload.oldWinS0Authority);
    CHECK(!secondUpload.stage2PlusAuthority);
    CHECK(!secondUpload.comod2Authority);
    const StrStripUploadState8002756C secondUploadLane1Plan =
        BuildStrStripUploadState8002756C(
            secondUploadControl, 2u, true, 1);
    CHECK(secondUploadLane1Plan.known);
    CHECK(secondUploadLane1Plan.destinationY ==
          static_cast<int16_t>(secondUploadControl.dstY + 240));
    CHECK(secondUploadLane1Plan.stripCount == secondUploadPlan.stripCount);
    const StrStripUploadExecution8002756C secondUploadLane1 =
        ExecuteStrStripUpload8002756C(
            secondUploadLane1Plan,
            runtime.allocations[3].data(),
            secondOutput.outputBytesWritten);
    CHECK(secondUploadLane1.executed);
    CHECK(secondUploadLane1.sourceFnv1a64 == secondUpload.sourceFnv1a64);
    CHECK(secondUploadLane1.rgbaFnv1a64 == secondUpload.rgbaFnv1a64);

    StrVramPageRuntime8001B120 vramPages =
        InitializeStrVramPageRuntime8001B120();
    CHECK(vramPages.initialized);
    CHECK(vramPages.width == 320u);
    CHECK(vramPages.height == 240u);
    CHECK(vramPages.currentScusSemanticAuthority);
    CHECK(vramPages.directVramPixelStateAuthority);
    CHECK(!vramPages.psxGpuMmioAuthority);
    StrVramPageRuntime8001B120 clearedVramPages =
        InitializeStrVramPageRuntime8001B120();
    const auto clearResult = ClearStrVramPages8001B1B0(
        clearedVramPages, 0, 0, 0);
    CHECK(clearResult.known);
    CHECK(clearResult.executionAccepted);
    CHECK(clearResult.completeDoubleBuffer);
    CHECK(clearResult.width == 320u);
    CHECK(clearResult.height == 480u);
    CHECK(clearResult.directVramPixelStateAuthority);
    CHECK(!clearResult.psxGpuMmioAuthority);
    CHECK(clearedVramPages.pages[0].contentKnown);
    CHECK(clearedVramPages.pages[1].contentKnown);
    CHECK(clearedVramPages.pages[0].rgbaPixels.size() == 320u * 240u);
    CHECK(clearedVramPages.pages[1].rgbaPixels ==
          clearedVramPages.pages[0].rgbaPixels);
    const auto clearedPageMove = ExecuteStrDisplayMove8001B120(
        1, 0u, true, 0u, clearedVramPages);
    CHECK(clearedPageMove.known);
    CHECK(clearedPageMove.executionAccepted);
    CHECK(clearedPageMove.sourcePage == 1u);
    CHECK(clearedPageMove.destinationPage == 0u);
    const auto missingSourceMove = ExecuteStrDisplayMove8001B120(
        1, 0u, true, 0u, vramPages);
    CHECK(!missingSourceMove.known);
    const auto page0Publish = PublishStrVramPageUpload8002756C(
        vramPages, 0u, secondUpload);
    CHECK(page0Publish.known);
    CHECK(page0Publish.executionAccepted);
    CHECK(page0Publish.pageIndex == 0u);
    CHECK(page0Publish.pixelCount == 76800u);
    CHECK(page0Publish.rgbaFnv1a64 == secondUpload.rgbaFnv1a64);
    CHECK(page0Publish.sourcePixelCount == secondUpload.rgbaPixels.size());
    CHECK(page0Publish.sourceRgbaFnv1a64 == secondUpload.rgbaFnv1a64);
    CHECK(!page0Publish.partialPageUpload);
    CHECK(page0Publish.exactFullPageShape);
    CHECK(page0Publish.directVramPixelStateAuthority);
    CHECK(vramPages.pages[0].contentKnown);
    CHECK(vramPages.pages[0].uploadCount8002756C == 1u);
    CHECK(vramPages.pages[0].rgbaPixels == secondUpload.rgbaPixels);

    // MOVIE0 is the 256x144 row at (35,25), not a 320x240 source frame.
    // 8002756C writes that rectangle into the selected 320x240 VRAM page;
    // rejecting it as a full-page upload leaves the real title path stuck.
    StrDecoderControlState80027288 partialControl = secondUploadControl;
    partialControl.decodedWidth = 256u;
    partialControl.decodedHeight = 144u;
    partialControl.dstX = 35;
    partialControl.dstY = 25;
    const auto partialPlan = BuildStrStripUploadState8002756C(
        partialControl, 2u, true, 0);
    CHECK(partialPlan.known);
    CHECK(partialPlan.stripCount == 16u);
    const auto partialUpload = ExecuteStrStripUpload8002756C(
        partialPlan,
        runtime.allocations[3].data(),
        secondOutput.outputBytesWritten);
    CHECK(partialUpload.known);
    CHECK(partialUpload.executed);
    CHECK(partialUpload.width == 256u);
    CHECK(partialUpload.height == 144u);
    CHECK(partialUpload.destinationX == 35);
    CHECK(partialUpload.destinationY == 25);
    CHECK(partialUpload.rgbaPixels.size() == 256u * 144u);
    StrVramPageRuntime8001B120 partialVramPages =
        InitializeStrVramPageRuntime8001B120();
    const auto partialPublish = PublishStrVramPageUpload8002756C(
        partialVramPages, 0u, partialUpload);
    CHECK(partialPublish.known);
    CHECK(partialPublish.executionAccepted);
    CHECK(partialPublish.partialPageUpload);
    CHECK(partialPublish.destinationX == 35);
    CHECK(partialPublish.destinationY == 25);
    CHECK(partialPublish.sourcePixelCount == 256u * 144u);
    CHECK(partialPublish.sourceRgbaFnv1a64 == partialUpload.rgbaFnv1a64);
    CHECK(partialPublish.pixelCount == 320u * 240u);
    CHECK(partialVramPages.pages[0].rgbaPixels.size() == 320u * 240u);
    CHECK(partialVramPages.pages[0].rgbaPixels[
              25u * 320u + 35u] == partialUpload.rgbaPixels[0]);
    CHECK(partialVramPages.pages[0].rgbaPixels[0] == 0u);
    CHECK(!ExecuteStrDisplayMove8001B120(
               1, 1u, false, 0u, vramPages).known);
    const auto page0To1Move = ExecuteStrDisplayMove8001B120(
        1, 1u, true, 0u, vramPages);
    CHECK(page0To1Move.known);
    CHECK(page0To1Move.called);
    CHECK(page0To1Move.sourceFunction == 0x8001B120u);
    CHECK(page0To1Move.argument == 1);
    CHECK(page0To1Move.drawBufferRead8004019C);
    CHECK(page0To1Move.drawBufferSlot80096590 == 1u);
    CHECK(page0To1Move.sourcePage == 0u);
    CHECK(page0To1Move.destinationPage == 1u);
    CHECK(page0To1Move.sourceY == 0);
    CHECK(page0To1Move.destinationY == 240u);
    CHECK(page0To1Move.width == 320);
    CHECK(page0To1Move.height == 240);
    CHECK(page0To1Move.pixelCountCopied == 76800u);
    CHECK(page0To1Move.sourceRgbaFnv1a64 ==
          secondUpload.rgbaFnv1a64);
    CHECK(page0To1Move.destinationRgbaFnv1a64 ==
          secondUpload.rgbaFnv1a64);
    CHECK(page0To1Move.dma.status ==
          PrPsxDmaSubmitDirect::PsxMoveImageStatus80044E2C::
              SoftwareTransactionRecorded);
    CHECK(page0To1Move.dma.packetWords[0] == 0x04FFFFFFu);
    CHECK(page0To1Move.dma.packetWords[1] == 0x80000000u);
    CHECK(page0To1Move.dma.packetWords[2] == 0x00000000u);
    CHECK(page0To1Move.dma.packetWords[3] == 0x00F00000u);
    CHECK(page0To1Move.dma.packetWords[4] == 0x00F00140u);
    CHECK(page0To1Move.dma.mmioWrites[1].value == 0x8005D7DCu);
    CHECK(page0To1Move.exact8001B120Branch);
    CHECK(page0To1Move.softwareVramMoveExecuted);
    CHECK(page0To1Move.executionAccepted);
    CHECK(page0To1Move.currentScusSemanticAuthority);
    CHECK(page0To1Move.currentComod0CallerAuthority);
    CHECK(page0To1Move.directVramPixelStateAuthority);
    CHECK(page0To1Move.directGpuPacketStateAuthority);
    CHECK(page0To1Move.translatedGpuDmaStateAuthority);
    CHECK(page0To1Move.translatedGpuCompletionAuthority);
    CHECK(!page0To1Move.psxGpuMmioAuthority);
    CHECK(!page0To1Move.hardwareCompletionTimingAuthority);
    CHECK(!page0To1Move.hostProjection);
    CHECK(!page0To1Move.replayValueAuthority);
    CHECK(!page0To1Move.oldWinS0Authority);
    CHECK(!page0To1Move.stage2PlusAuthority);
    CHECK(!page0To1Move.comod2Authority);
    CHECK(vramPages.pages[1].contentKnown);
    CHECK(vramPages.pages[1].moveWriteCount8001B120 == 1u);
    CHECK(vramPages.pages[1].rgbaPixels == secondUpload.rgbaPixels);
    CHECK(vramPages.moveCount8001B120 == 1u);
    const auto page1To0Move = ExecuteStrDisplayMove8001B120(
        0, 1u, true, 0u, vramPages);
    CHECK(page1To0Move.known);
    CHECK(page1To0Move.sourcePage == 1u);
    CHECK(page1To0Move.destinationPage == 0u);
    CHECK(page1To0Move.sourceY == 240);
    CHECK(page1To0Move.destinationY == 0u);
    CHECK(page1To0Move.dma.packetWords[2] == 0x00F00000u);
    CHECK(page1To0Move.dma.packetWords[3] == 0x00000000u);
    CHECK(vramPages.moveCount8001B120 == 2u);
    const auto playAndWaitReturn =
        ResolveStrPlayAndWaitMode0Return801C455C();
    CHECK(playAndWaitReturn.known);
    CHECK(playAndWaitReturn.activeCallerModeZero);
    CHECK(playAndWaitReturn.callerMode == 0);
    CHECK(playAndWaitReturn.returnCarrierS3InitializedZero801C4638);
    CHECK(playAndWaitReturn.returnCarrierS3UnmodifiedAfterInitialization);
    CHECK(playAndWaitReturn.returnMoveV0FromS3Observed801C4754);
    CHECK(playAndWaitReturn.exactEpilogueObserved);
    CHECK(playAndWaitReturn.returnValue == 0);
    CHECK(playAndWaitReturn.currentComod0SemanticAuthority);
    CHECK(playAndWaitReturn.directReturnStateAuthority);
    CHECK(!playAndWaitReturn.hostProjection);
    CHECK(!playAndWaitReturn.replayValueAuthority);
    CHECK(!playAndWaitReturn.oldWinS0Authority);
    CHECK(!playAndWaitReturn.stage2PlusAuthority);
    CHECK(!playAndWaitReturn.comod2Authority);
    const auto outerPostWait = ExecuteStrOuterPostWait801C4DC4(
        1u, true, 0u, vramPages);
    CHECK(outerPostWait.known);
    CHECK(outerPostWait.called);
    CHECK(outerPostWait.sourceFunction == 0x801C4DC4u);
    CHECK(outerPostWait.playAndWaitCallsite == 0x801C4E4Cu);
    CHECK(outerPostWait.playAndWaitReturn.returnValue == 0);
    CHECK(outerPostWait.secondDisplayMoveCallsite == 0x801C4E54u);
    CHECK(outerPostWait.secondDisplayMove.known);
    CHECK(outerPostWait.secondDisplayMove.argument == 1);
    CHECK(outerPostWait.secondDisplayMove.sourcePage == 0u);
    CHECK(outerPostWait.secondDisplayMove.destinationPage == 1u);
    CHECK(outerPostWait.secondDisplayMove.pixelCountCopied == 76800u);
    CHECK(outerPostWait.secondDisplayMove.dma.packetWords[0] ==
          0x04FFFFFFu);
    CHECK(outerPostWait.fastTransitionCallsite == 0x801C4E70u);
    CHECK(outerPostWait.fastTransitionFunction == 0x800201ACu);
    CHECK(outerPostWait.fastTransitionContext == kScene0WorkAddress);
    CHECK(outerPostWait.fastTransitionMode == 5);
    CHECK(outerPostWait.fastTransitionPreArg == 1);
    CHECK(outerPostWait.fastTransitionPostArg == 2);
    CHECK(outerPostWait.fastTransitionPending);
    CHECK(outerPostWait.word800916D2WriteCallsite == 0x801C4E74u);
    CHECK(outerPostWait.word800916D2Value == 1u);
    CHECK(outerPostWait.word800916D2WritePending);
    CHECK(outerPostWait.exactCallerOrder);
    CHECK(outerPostWait.currentComod0SemanticAuthority);
    CHECK(outerPostWait.directSoftwareStateAuthority);
    CHECK(!outerPostWait.psxGpuMmioAuthority);
    CHECK(!outerPostWait.hardwareCompletionTimingAuthority);
    CHECK(!outerPostWait.hostProjection);
    CHECK(!outerPostWait.replayValueAuthority);
    CHECK(!outerPostWait.oldWinS0Authority);
    CHECK(!outerPostWait.stage2PlusAuthority);
    CHECK(!outerPostWait.comod2Authority);
    CHECK(vramPages.moveCount8001B120 == 3u);
    CHECK(!ExecuteStrOuterPostWait801C4DC4(
               1u, false, 0u, vramPages).known);
    std::printf(
        "test_ss0_str_lifecycle: exact MOVIE0T frame2 sectors=%u VLC=%u halfwords=%u output=%u fnv1a=%016llX rgba=%u rgbaFnv1a=%016llX\n",
        static_cast<unsigned>(secondPrime.sectorCount),
        secondVlcRelease.vlc.outputBytesWritten,
        secondOutput.inputHalfwordsConsumed,
        secondOutput.outputBytesWritten,
        static_cast<unsigned long long>(secondOutput.outputFnv1a64),
        secondUpload.rgbaBytesWritten,
        static_cast<unsigned long long>(secondUpload.rgbaFnv1a64));

    StrStreamStartState800274D4 secondStream{};
    CHECK(TryStartStrStreamState800274D4(decoder, secondStream));
    secondStream.control = completion.controlAfter;
    StrDecodeAdvanceInput800273A4 secondAdvanceInput{};
    secondAdvanceInput.acquireReturnKnown = true;
    secondAdvanceInput.acquireReturn = secondAcquire.returnValue;
    secondAdvanceInput.payloadHandleKnown = secondAcquire.payloadHandleKnown;
    secondAdvanceInput.payloadHandle = secondAcquire.payloadHandle;
    secondAdvanceInput.sectorHeaderKnown = secondAcquire.sectorHeaderKnown;
    secondAdvanceInput.sectorWidth = secondAcquire.width;
    secondAdvanceInput.sectorHeight = secondAcquire.height;
    secondAdvanceInput.vlcDecodeResultKnown = true;
    secondAdvanceInput.vlcDecodeResult = secondVlcRelease.vlc.returnValue;
    secondAdvanceInput.mdecCommandOutputSubmissionKnown = true;
    secondAdvanceInput.mdecCommandOutputSubmissionAuthority = true;
    secondAdvanceInput.sectorCounterBefore = 1u;
    secondAdvanceInput.sectorRingExecutionAuthority = true;
    const StrDecodeAdvanceState800273A4 secondAdvance =
        BuildStrDecodeAdvanceState800273A4(
            secondStream, secondAdvanceInput);
    CHECK(secondAdvance.planned);
    CHECK(secondAdvance.executed);
    CHECK(secondAdvance.sectorCounterAfter == 2u);
    CHECK(secondAdvance.control.outputReady == 1u);
    CHECK(secondAdvance.control.decodeInFlight == 1u);
    const StrDecodeGateState80027528 secondPollGate =
        BuildStrDecodeGateState80027528(secondAdvance.control);
    CHECK(secondPollGate.known);
    CHECK(secondPollGate.branch == StrDecodeGateBranch80027528::DecodeBusy);
    CHECK(!secondPollGate.callDecodeAdvance800273A4);
    CHECK(secondPollGate.returnValueKnown);
    CHECK(secondPollGate.returnValue == 1);
    CHECK(!runtime.sectorRingFrameAcquired);
    CHECK(ExecuteStrStopReset80027664(runtime).executionAccepted);
}

void TestMdec15bppCurrentScusSynthetic() {
    std::array<uint8_t, 4u + 0x20u * 4u> input{};
    input[0] = 0x20u;
    input[1] = 0x00u;
    input[2] = 0x00u;
    input[3] = 0x3Au;
    for (size_t offset = 4u; offset < input.size(); offset += 2u) {
        input[offset] = 0x00u;
        input[offset + 1u] = 0xFEu;
    }
    for (size_t block = 0u; block < 6u; ++block) {
        const size_t offset = 4u + block * 4u;
        input[offset] = 0x00u;
        input[offset + 1u] = 0x00u;
        input[offset + 2u] = 0x00u;
        input[offset + 3u] = 0xFEu;
    }
    std::array<uint8_t, 512> output{};
    const PrSS0MdecOutputDirect::Mdec15bppResult result =
        PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
            input.data(), input.size(), 16u, 16u,
            output.data(), output.size());
    CHECK(result.known);
    CHECK(result.executed);
    CHECK(result.inputHalfwordsConsumed == 12u);
    CHECK(result.macroblocksDecoded == 1u);
    CHECK(result.outputBytesWritten == output.size());
    for (size_t offset = 0u; offset < output.size(); offset += 2u) {
        CHECK(output[offset] == 0x10u);
        CHECK(output[offset + 1u] == 0xC2u);
    }
    input[3] = 0x3Eu;
    CHECK(!PrSS0MdecOutputDirect::ExecuteMdec15bppCurrentScus(
        input.data(), input.size(), 16u, 16u,
        output.data(), output.size()).known);
}

void TestStrSectorRingDirectInput80039670() {
    const auto discPath =
        WriteSyntheticMode2StrFrameDisc80039670(1u, 1u);
    StrLowerCdStartSource8001A4D0 lowerSource{};
    lowerSource.segmentKnown = true;
    lowerSource.startLbaKnown = true;
    lowerSource.startLba = 2;
    lowerSource.lengthBytesKnown = true;
    lowerSource.lengthBytes = 3u * kRawCdLogicalBytes8001A4D0;
    lowerSource.filterKnown = true;
    lowerSource.filterFile = 1u;
    lowerSource.filterChannel = 1u;
    lowerSource.discBinPath = discPath;
    lowerSource.discImageDirectoryAuthority = true;
    StrLowerCdStartRuntime8001A4D0 lowerStart{};
    CHECK(TryInitializeStrLowerCdStartRuntime8001A4D0(
        lowerSource,
        lowerStart));
    CHECK(lowerStart.discBinPath == discPath);

    StrDecoderAllocationState80027288 decoder{};
    CHECK(TryInitializeStrDecoderState80027288(0, false, decoder));
    StrDecoderMemoryRuntime80027288 runtime{};
    CHECK(TryInitializeStrDecoderMemoryRuntime80027288(decoder, runtime));
    CHECK(PublishStrDecoderStreamingStart8001A4D0(lowerStart, runtime));

    const StrSectorRingPrimeResult80039670 prime =
        PrimeStrSectorRingFromDisc80039670(lowerStart, runtime);
    CHECK(prime.known);
    CHECK(prime.framePublished);
    CHECK(prime.rawSectorsScanned == 3u);
    CHECK(prime.filteredVideoSectors == 3u);
    CHECK(prime.rejectedSectors == 0u);
    CHECK(prime.firstSlot == 0u);
    CHECK(prime.sectorCount == 3u);
    CHECK(prime.frameNumber == 7u);
    CHECK(prime.width == 320u);
    CHECK(prime.height == 240u);
    CHECK(prime.payloadByteOffset ==
          kStrSectorMetadataAreaBytes80039670);
    CHECK(prime.directDiscSectorAuthority);
    CHECK(prime.synchronousCallbackOrdering);
    CHECK(!prime.hardwareCallbackTimingAuthority);
    CHECK(!prime.psxPointerAuthority);
    CHECK(!prime.hostProjection);
    CHECK(!prime.replayValueAuthority);
    CHECK(!prime.oldWinS0Authority);
    CHECK(!prime.stage2PlusAuthority);
    CHECK(!prime.comod2Authority);
    CHECK(runtime.sectorRingProducerIndex80085C50 == 3u);
    CHECK(runtime.sectorRingPublishedIndex80085C54 == 3u);
    CHECK(runtime.sectorRingReadIndex80085C58 == 0u);
    CHECK(runtime.sectorRingQueuedSlots == 3u);
    CHECK(runtime.sectorRingPublishedFrames == 1u);
    CHECK(runtime.sectorRingFramePublished);
    CHECK(!runtime.sectorRingFrameAcquired);
    CHECK(runtime.directSectorRingInputAuthority);
    CHECK(runtime.allocations[1][0u] == 2u);
    CHECK(runtime.allocations[1][1u] == 0u);
    CHECK(runtime.allocations[1][32u] == 3u);
    CHECK(runtime.allocations[1][64u] == 3u);
    CHECK(runtime.allocations[1][16u] == 0x40u);
    CHECK(runtime.allocations[1][17u] == 0x01u);
    CHECK(runtime.allocations[1][18u] == 0xF0u);
    CHECK(runtime.allocations[1][19u] == 0x00u);
    CHECK(runtime.allocations[1][1024u] == 0x30u);
    CHECK(runtime.allocations[1][1024u + 2016u] == 0x31u);
    CHECK(runtime.allocations[1][1024u + 4032u] == 0x32u);
    CHECK(!PrimeStrSectorRingFromDisc80039670(
               lowerStart,
               runtime).known);

    const StrSectorRingAcquireResult8003958C acquire =
        AcquireStrSectorRingFrame8003958C(runtime);
    CHECK(acquire.known);
    CHECK(acquire.returnValue == 0);
    CHECK(acquire.available);
    CHECK(acquire.payloadHandleKnown);
    CHECK(acquire.payloadHandle == 1024u);
    CHECK(acquire.sectorHeaderKnown);
    CHECK(acquire.width == 320u);
    CHECK(acquire.height == 240u);
    CHECK(acquire.firstSlot == 0u);
    CHECK(acquire.sectorCount == 3u);
    CHECK(acquire.frameNumber == 7u);
    CHECK(acquire.stateTransition2To4);
    CHECK(acquire.directSectorRingAuthority);
    CHECK(!acquire.psxPointerAuthority);
    CHECK(!acquire.hostProjection);
    CHECK(!acquire.replayValueAuthority);
    CHECK(!acquire.oldWinS0Authority);
    CHECK(!acquire.stage2PlusAuthority);
    CHECK(!acquire.comod2Authority);
    CHECK(runtime.allocations[1][0u] == 4u);
    CHECK(runtime.sectorRingFrameAcquired);

    const StrSectorRingAcquireResult8003958C unavailable =
        AcquireStrSectorRingFrame8003958C(runtime);
    CHECK(unavailable.known);
    CHECK(unavailable.returnValue == 1);
    CHECK(!unavailable.available);

    StrStreamStartState800274D4 stream{};
    CHECK(TryStartStrStreamState800274D4(decoder, stream));
    StrDecodeAdvanceInput800273A4 input{};
    input.acquireReturnKnown = true;
    input.acquireReturn = acquire.returnValue;
    input.payloadHandleKnown = acquire.payloadHandleKnown;
    input.payloadHandle = acquire.payloadHandle;
    input.sectorHeaderKnown = acquire.sectorHeaderKnown;
    input.sectorWidth = acquire.width;
    input.sectorHeight = acquire.height;
    input.sectorCounterBefore = 0u;
    input.sectorRingExecutionAuthority = true;
    const StrDecodeAdvanceState800273A4 advance =
        BuildStrDecodeAdvanceState800273A4(stream, input);
    CHECK(advance.planned);
    CHECK(!advance.executed);
    CHECK(!advance.hasOpenInputGap);
    CHECK(advance.branch ==
          StrDecodeAdvanceBranch800273A4::MdecBoundary);
    CHECK(advance.control.decodeInFlight == 1u);
    CHECK(advance.sectorCounterIncremented);
    CHECK(advance.sectorCounterAfter == 1u);
    CHECK(advance.runtimeInputBound);
    CHECK(advance.sectorRingExecutionAuthority);
    CHECK(advance.executedThroughAcquire8003958C);
    CHECK(advance.reachedDecDctVlc2Boundary);
    CHECK(advance.blockedAtMdecDecDCTvlc2);
    CHECK(!advance.decDctVlc2Called);
    CHECK(!advance.releaseFrameCalled80039490);
    CHECK(!advance.mdecExecutionAuthority);
    CHECK(!advance.psxPointerAuthority);
    CHECK(!advance.hostProjection);
    CHECK(!advance.replayValueAuthority);
    CHECK(!advance.oldWinS0Authority);
    CHECK(!advance.stage2PlusAuthority);
    CHECK(!advance.comod2Authority);

    const StrSectorRingReleaseResult80039490 badRelease =
        ReleaseStrSectorRingFrame80039490(
            acquire.payloadHandle + 1u,
            runtime);
    CHECK(badRelease.known);
    CHECK(!badRelease.released);
    CHECK(badRelease.returnValue == 1);
    const StrSectorRingReleaseResult80039490 release =
        ReleaseStrSectorRingFrame80039490(
            acquire.payloadHandle,
            runtime);
    CHECK(release.known);
    CHECK(release.returnValue == 0);
    CHECK(release.released);
    CHECK(release.firstSlot == 0u);
    CHECK(release.sectorCount == 3u);
    CHECK(release.nextReadIndex == 3u);
    CHECK(release.stateTransition4To0);
    CHECK(release.directSectorRingAuthority);
    CHECK(!release.psxPointerAuthority);
    CHECK(!release.hostProjection);
    CHECK(!release.replayValueAuthority);
    CHECK(!release.oldWinS0Authority);
    CHECK(!release.stage2PlusAuthority);
    CHECK(!release.comod2Authority);
    CHECK(runtime.allocations[1][0u] == 0u);
    CHECK(runtime.allocations[1][32u] == 0u);
    CHECK(runtime.allocations[1][64u] == 0u);
    CHECK(runtime.sectorRingReadIndex80085C58 == 3u);
    CHECK(runtime.sectorRingQueuedSlots == 0u);
    CHECK(!runtime.sectorRingFramePublished);
    CHECK(!runtime.sectorRingFrameAcquired);
    CHECK(!ReleaseStrSectorRingFrame80039490(
               acquire.payloadHandle,
               runtime).released);

    CHECK(ExecuteStrStopReset80027664(runtime).executionAccepted);
    CHECK(!runtime.directSectorRingInputAuthority);
    std::error_code ec;
    std::filesystem::remove(discPath, ec);
}

void TestTitleEarlyInputWaitCleanupHostProjection8001A694() {
    static constexpr bool kBooleanStates[] = {false, true};
    for (bool playerPresent : kBooleanStates) {
        for (bool strStarted : kBooleanStates) {
            for (bool playerActive : kBooleanStates) {
                TitleEarlyInputWaitCleanupHostProjectionInput8001A694 input{};
                input.playerPresent = playerPresent;
                input.strStarted = strStarted;
                input.playerActive = playerActive;

                const auto result =
                    BuildTitleEarlyInputWaitCleanupHostProjection8001A694(
                        input);
                const bool available =
                    playerPresent && !strStarted && !playerActive;
                CHECK(result.available == available);
                CHECK(result.hostProjection == available);
                CHECK(result.hostStrStopped == available);
                CHECK(!result.psxCdAuthority);
                CHECK(!result.psxCommand8WaitKnown);
                CHECK(!result.psxCallbackClearKnown);
            }
        }
    }
}

void TestScene0StrAudioRoute801C44E0() {
    Scene0StrAudioRouteSource801C44E0 source{};
    auto route = BuildScene0StrAudioRoute801C44E0(source);
    CHECK(!route.known);
    CHECK(!route.hostProjection);
    CHECK(!route.psxCdAuthority);
    CHECK(!route.volumeKnown);
    CHECK(!route.filterKnown);
    CHECK(!route.filterCommandSerialKnown);

    source.rowKnown = true;
    route = BuildScene0StrAudioRoute801C44E0(source);
    CHECK(!route.known);
    CHECK(!route.hostProjection);

    source.opaque04Known = true;
    source.opaque04 = 0x005A0001u;
    route = BuildScene0StrAudioRoute801C44E0(source);
    CHECK(route.known);
    CHECK(!route.hostProjection);
    CHECK(!route.psxCdAuthority);
    CHECK(route.currentScusSemanticAuthority);
    CHECK(route.currentComod0CallerAuthority);
    CHECK(route.volumeKnown);
    CHECK(route.volume8001A478 == 90);
    CHECK(route.volumeCommand8002AB24 == kFn8002AB24);
    CHECK(route.volumeMask8002AB24 == 0xC0u);
    CHECK(route.scaledLeft8002AB24 == 90 * 258);
    CHECK(route.scaledRight8002AB24 == 90 * 258);
    CHECK(route.directVolumeCommandAuthority);
    CHECK(!route.psxSpuHardwareAuthority);
    CHECK(std::fabs(route.normalizedVolume - 90.0f / 127.0f) < 0.0001f);
    CHECK(route.filterKnown);
    CHECK(route.filterFile8004940C == 1u);
    CHECK(route.filterChannel8004940D == 1u);
    CHECK(route.filterCommand8001A654 == kCdCommandSetfilter);
    CHECK(route.filterCommandSerialKnown);
    CHECK(route.directFilterCommandAuthority);
    CHECK(!route.replayValueAuthority);
    CHECK(!route.oldWinS0Authority);
    CHECK(!route.stage2PlusAuthority);
    CHECK(!route.comod2Authority);

    source.opaque04 = 0x007F0001u;
    route = BuildScene0StrAudioRoute801C44E0(source);
    CHECK(route.known);
    CHECK(route.volume8001A478 == 127);
    CHECK(route.normalizedVolume == 1.0f);
    CHECK(route.filterChannel8004940D == 1u);

    source.opaque04 = 0x00800002u;
    route = BuildScene0StrAudioRoute801C44E0(source);
    CHECK(route.volume8001A478 == 128);
    CHECK(route.scaledLeft8002AB24 == 127 * 258);
    CHECK(route.scaledRight8002AB24 == 127 * 258);
    CHECK(route.normalizedVolume == 1.0f);
    CHECK(route.filterChannel8004940D == 2u);

    source.opaque04 = 0xFFFF0003u;
    route = BuildScene0StrAudioRoute801C44E0(source);
    CHECK(route.volume8001A478 == -1);
    CHECK(route.scaledLeft8002AB24 == -258);
    CHECK(route.scaledRight8002AB24 == -258);
    CHECK(route.normalizedVolume == 0.0f);
    CHECK(route.filterChannel8004940D == 3u);
}

void TestRuntimeGateNamesGeometryAndInputs() {
    CHECK(!RuntimeCutoverAllowed());
    CHECK(StrPlanKindName(StrPlanKind::Scene0Movie0Playback)[0] == 'S');
    CHECK(StrMovieKindName(StrMovieKind::OpeningMovie0)[0] == 'O');
    CHECK(StrActionKindName(StrActionKind::Call8002756CUploadStrips)[0] == 'C');

    StrDecodeGeometry geometry = KnownDecodeGeometry80027288(0, false);
    CHECK(geometry.known);
    CHECK(geometry.movieKind == 0);
    CHECK(geometry.width == 256);
    CHECK(geometry.height == 144);
    CHECK(geometry.dstX == 35);
    CHECK(geometry.dstY == 48);
    CHECK(geometry.stripCount == 16);
    CHECK(geometry.word800916DCApplied);

    geometry = KnownDecodeGeometry80027288(0, true);
    CHECK(geometry.known);
    CHECK(geometry.dstY == 25);
    CHECK(!geometry.word800916DCApplied);

    geometry = KnownDecodeGeometry80027288(1, false);
    CHECK(geometry.known);
    CHECK(geometry.width == 320);
    CHECK(geometry.height == 240);
    CHECK(geometry.dstX == 0);
    CHECK(geometry.dstY == 23);
    CHECK(geometry.stripCount == 20);

    geometry = KnownDecodeGeometry80027288(99, false);
    CHECK(!geometry.known);
    CHECK(geometry.movieKind == 99);
    CHECK(geometry.stripCount == 0);

    CHECK(!IsKnownExitInput80035510(0));
    CHECK(IsKnownExitInput80035510(kInputForceReturnOne));
    CHECK(IsKnownExitInput80035510(kInputDirectExit));
    CHECK(IsKnownExitInput80035510(0x40));
    CHECK(KnownReturnValueForInput80035510(kInputForceReturnOne) == 1);
    CHECK(KnownReturnValueForInput80035510(kInputDirectExit) == 0);
    CHECK(KnownReturnValueForInput80035510(0x40) == 0);
}

void TestStrDecoderInitialState80027288() {
    StrDecoderAllocationState80027288 state{};
    CHECK(TryInitializeStrDecoderState80027288(0, true, state));
    CHECK(state.accepted);
    CHECK(state.movieKind == 0);
    CHECK(state.word800916DC);
    CHECK(state.tableRow.known);
    CHECK(state.tableRow.expectedWidth == 256u);
    CHECK(state.tableRow.expectedHeight == 144u);
    CHECK(state.tableRow.dstX == 35);
    CHECK(state.tableRow.dstY == 25);
    CHECK(state.tableRow.tableWord08 == 0x20u);
    CHECK(state.tableRow.vlcBufferBytes == 0x12000u);
    CHECK(state.tableRow.imageBufferBytes == 0x24000u);
    CHECK(state.allocationCount == 4u);
    CHECK(state.allocationSizes[0] == 0x24u);
    CHECK(state.allocationSizes[1] == 0x10000u);
    CHECK(state.allocationSizes[2] == 0x12000u);
    CHECK(state.allocationSizes[3] == 0x24000u);
    CHECK(state.allocationAlignment == 8u);
    CHECK(state.allocationsZeroFilled);
    CHECK(state.allocationFailureExitsProcess);
    CHECK(state.allocationPointerSlots[0] == 0x800965A8u);
    CHECK(state.allocationPointerSlots[1] == 0x800965ACu);
    CHECK(state.allocationPointerSlots[2] == 0x800965B0u);
    CHECK(state.allocationPointerSlots[3] == 0x800965B4u);
    CHECK(state.decDctResetCalled800473EC);
    CHECK(state.decDctResetArgument800473EC == 0u);
    CHECK(state.resetCallbackCalled800473EC);
    CHECK(state.sectorRingInitialized8003624C);
    CHECK(state.sectorRingSlots8003624C == 32u);
    CHECK(state.sectorRingCountAddress801C3868 == 0x801C3868u);
    CHECK(state.outputCallbackInstalled80047658);
    CHECK(state.outputCallbackFunction80027220 == kFn80027220);
    CHECK(state.outputCallbackDmaChannel80047658 == 1u);
    CHECK(state.control.known);
    CHECK(state.control.vlcBufferBound);
    CHECK(state.control.imageBufferBound);
    CHECK(state.control.decodedWidth == 0u);
    CHECK(state.control.decodedHeight == 0u);
    CHECK(state.control.vlcDecodeResult == 0u);
    CHECK(state.control.dstX == 35);
    CHECK(state.control.dstY == 25);
    CHECK(state.control.decodeInFlight == 0u);
    CHECK(state.control.outputReady == 0u);
    CHECK(state.currentScusSemanticAuthority);
    CHECK(state.currentComod0CallerAuthority);
    CHECK(state.comod1ComparisonOnly);
    CHECK(!state.hostProjection);
    CHECK(!state.psxPointerAuthority);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);

    CHECK(TryInitializeStrDecoderState80027288(0, false, state));
    CHECK(state.control.dstY == 48);
    CHECK(TryInitializeStrDecoderState80027288(1, false, state));
    CHECK(state.tableRow.expectedWidth == 320u);
    CHECK(state.tableRow.expectedHeight == 240u);
    CHECK(state.tableRow.vlcBufferBytes == 0x20000u);
    CHECK(state.tableRow.imageBufferBytes == 0x4B000u);
    CHECK(state.control.dstX == 0);
    CHECK(state.control.dstY == 23);
    CHECK(TryInitializeStrDecoderState80027288(2, false, state));
    CHECK(state.tableRow.expectedWidth == 256u);
    CHECK(state.tableRow.expectedHeight == 144u);
    CHECK(state.control.dstX == 32);
    CHECK(state.control.dstY == 48);

    CHECK(!TryInitializeStrDecoderState80027288(-1, false, state));
    CHECK(!state.accepted);
    CHECK(state.movieKind == -1);
    CHECK(!state.tableRow.known);
    CHECK(!TryInitializeStrDecoderState80027288(3, false, state));
    CHECK(!state.accepted);
    CHECK(state.movieKind == 3);
}

void TestStrStreamStartState800274D4() {
    StrDecoderAllocationState80027288 decoder{};
    CHECK(TryInitializeStrDecoderState80027288(0, false, decoder));
    decoder.control.decodedWidth = 256u;
    decoder.control.decodedHeight = 144u;
    decoder.control.vlcDecodeResult = 7u;
    decoder.control.decodeInFlight = 1u;
    decoder.control.outputReady = 1u;

    StrStreamStartState800274D4 state{};
    CHECK(TryStartStrStreamState800274D4(decoder, state));
    CHECK(state.accepted);
    CHECK(state.control.known);
    CHECK(state.control.vlcBufferBound);
    CHECK(state.control.imageBufferBound);
    CHECK(state.control.decodedWidth == 256u);
    CHECK(state.control.decodedHeight == 144u);
    CHECK(state.control.dstX == 35);
    CHECK(state.control.dstY == 48);
    CHECK(state.control.vlcDecodeResult == 0u);
    CHECK(state.control.decodeInFlight == 0u);
    CHECK(state.control.outputReady == 0u);
    CHECK(state.vlcDecodeResultCleared);
    CHECK(state.decodeInFlightCleared);
    CHECK(state.outputReadyCleared);
    CHECK(state.stSetStreamCalled);
    CHECK(state.stSetStreamArgs[0] == 0);
    CHECK(state.stSetStreamArgs[1] == 1);
    CHECK(state.stSetStreamArgs[2] == -1);
    CHECK(state.stSetStreamArgs[3] == 0);
    CHECK(state.stSetStreamArgs[4] == 0);
    CHECK(state.decDctVlcSize2Called);
    CHECK(state.decDctVlcSize2Argument == 0);
    CHECK(state.initialDecodeAdvanceCalled800273A4);
    CHECK(state.sectorCounterResetAfterAdvance);
    CHECK(state.sectorCounterAddress800965B8 == 0x800965B8u);
    CHECK(state.sectorCounterValue == 0u);
    CHECK(state.exactCallOrder);
    CHECK(state.currentScusSemanticAuthority);
    CHECK(state.currentComod0CallerAuthority);
    CHECK(state.comod1ComparisonOnly);
    CHECK(!state.lowerCdStartAuthority);
    CHECK(!state.initialDecodeAdvanceSemanticTranslated);
    CHECK(!state.streamLibraryAuthority);
    CHECK(!state.psxPointerAuthority);
    CHECK(!state.hostProjection);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);

    decoder.accepted = false;
    CHECK(!TryStartStrStreamState800274D4(decoder, state));
    CHECK(!state.accepted);
    CHECK(!state.stSetStreamCalled);
}

void TestStrDecodeAdvanceState800273A4() {
    StrDecoderAllocationState80027288 decoder{};
    CHECK(TryInitializeStrDecoderState80027288(0, false, decoder));
    StrStreamStartState800274D4 stream{};
    CHECK(TryStartStrStreamState800274D4(decoder, stream));

    StrDecodeAdvanceInput800273A4 input{};
    StrDecodeAdvanceState800273A4 state =
        BuildStrDecodeAdvanceState800273A4(stream, input);
    CHECK(state.planned);
    CHECK(!state.executed);
    CHECK(state.hasOpenInputGap);
    CHECK(state.branch ==
          StrDecodeAdvanceBranch800273A4::UnknownInput);
    CHECK(state.acquireFrameCalled8003958C);
    CHECK(!state.acquireReturnKnown);
    CHECK(state.branchSemanticsTranslated);
    CHECK(!state.runtimeInputBound);
    CHECK(!state.sectorRingExecutionAuthority);
    CHECK(!state.mdecExecutionAuthority);
    CHECK(!state.psxPointerAuthority);
    CHECK(!state.hostProjection);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);

    input.acquireReturnKnown = true;
    input.acquireReturn = 1;
    state = BuildStrDecodeAdvanceState800273A4(stream, input);
    CHECK(state.executed);
    CHECK(!state.hasOpenInputGap);
    CHECK(state.branch ==
          StrDecodeAdvanceBranch800273A4::AcquireUnavailable);
    CHECK(state.control.decodeInFlight == 0u);
    CHECK(!state.sectorCounterIncremented);
    CHECK(!state.decDctVlc2Called);

    input.acquireReturn = 0;
    input.payloadHandleKnown = true;
    input.payloadHandle = 0u;
    input.sectorHeaderKnown = true;
    input.sectorWidth = 320u;
    input.sectorHeight = 240u;
    input.sectorCounterBefore = 9u;
    state = BuildStrDecodeAdvanceState800273A4(stream, input);
    CHECK(state.executed);
    CHECK(state.branch == StrDecodeAdvanceBranch800273A4::EmptyPayload);
    CHECK(state.sectorHeaderApplied);
    CHECK(state.control.decodedWidth == 320u);
    CHECK(state.control.decodedHeight == 240u);
    CHECK(state.sectorCounterIncremented);
    CHECK(state.sectorCounterAddress800965B8 == 0x800965B8u);
    CHECK(state.sectorCounterBefore == 9u);
    CHECK(state.sectorCounterAfter == 10u);
    CHECK(state.control.decodeInFlight == 0u);

    input.payloadHandle = 0x1234u;
    input.vlcDecodeResultKnown = true;
    input.vlcDecodeResult = 7;
    state = BuildStrDecodeAdvanceState800273A4(stream, input);
    CHECK(state.executed);
    CHECK(state.branch ==
          StrDecodeAdvanceBranch800273A4::VlcDecodeFailed);
    CHECK(state.control.decodeInFlight == 1u);
    CHECK(state.decDctVlc2Called);
    CHECK(state.control.vlcDecodeResult == 7u);
    CHECK(state.releaseFrameCalled80039490);
    CHECK(!state.mdecControlCalled80047558);
    CHECK(!state.outputTransferCalled);
    CHECK(state.control.outputReady == 0u);

    input.vlcDecodeResult = 0;
    state = BuildStrDecodeAdvanceState800273A4(stream, input);
    CHECK(!state.executed);
    CHECK(state.branch ==
          StrDecodeAdvanceBranch800273A4::MdecHardwareBoundary);
    CHECK(state.reachedMdecControlBoundary80047558);
    CHECK(state.blockedAtMdecHardwareExecution);
    CHECK(state.hasOpenMdecHardwareGap);
    CHECK(!state.mdecControlCalled80047558);
    CHECK(!state.outputTransferCalled);
    CHECK(state.control.outputReady == 0u);

    input.mdecCommandOutputSubmissionKnown = true;
    input.mdecCommandOutputSubmissionAuthority = true;
    state = BuildStrDecodeAdvanceState800273A4(stream, input);
    CHECK(state.executed);
    CHECK(state.branch == StrDecodeAdvanceBranch800273A4::
                              MdecOutputSubmittedHardwareBoundary);
    CHECK(state.mdecControlCalled80047558);
    CHECK(state.mdecControlArgument == 2);
    CHECK(state.outputTransferCalled);
    CHECK(state.outputTransferWordCount == 38400u);
    CHECK(state.control.outputReady == 1u);
    CHECK(state.mdecCommandOutputSubmissionAuthority);
    CHECK(state.blockedAtMdecHardwareExecution);
    CHECK(state.blockedAtMdecHardwareOutput);
    CHECK(state.hasOpenMdecHardwareGap);
    CHECK(!state.mdecExecutionAuthority);

    stream.accepted = false;
    state = BuildStrDecodeAdvanceState800273A4(stream, input);
    CHECK(!state.planned);
    CHECK(!state.executed);
}

void TestStrDecodeGateState80027528() {
    StrDecoderControlState80027288 control{};
    StrDecodeGateState80027528 state =
        BuildStrDecodeGateState80027528(control);
    CHECK(!state.known);

    control.known = true;
    control.outputReady = 0u;
    control.decodeInFlight = 1u;
    state = BuildStrDecodeGateState80027528(control);
    CHECK(state.known);
    CHECK(state.branch ==
          StrDecodeGateBranch80027528::OutputNotReady);
    CHECK(state.callDecodeAdvance800273A4);
    CHECK(!state.returnValueKnown);

    control.outputReady = 1u;
    control.decodeInFlight = 0u;
    state = BuildStrDecodeGateState80027528(control);
    CHECK(state.branch == StrDecodeGateBranch80027528::DecodeIdle);
    CHECK(state.callDecodeAdvance800273A4);
    CHECK(!state.returnValueKnown);

    control.outputReady = 1u;
    control.decodeInFlight = 3u;
    state = BuildStrDecodeGateState80027528(control);
    CHECK(state.branch == StrDecodeGateBranch80027528::DecodeBusy);
    CHECK(!state.callDecodeAdvance800273A4);
    CHECK(state.returnValueKnown);
    CHECK(state.returnValue == 3);
    CHECK(state.branchSemanticsTranslated);
    CHECK(!state.decodeAdvanceExecuted);
    CHECK(!state.hostProjection);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);
}

void TestStrStripUploadState8002756C() {
    StrDecoderControlState80027288 control{};
    StrStripUploadState8002756C state =
        BuildStrStripUploadState8002756C(control, 0u, false, 0);
    CHECK(!state.known);

    control.known = true;
    control.imageBufferBound = true;
    control.decodedWidth = 320u;
    control.decodedHeight = 240u;
    control.dstX = 35;
    control.dstY = 48;
    state = BuildStrStripUploadState8002756C(control, 1u, false, 0);
    CHECK(state.known);
    CHECK(state.skippedForCounterWarmup);
    CHECK(state.stripCount == 0u);
    CHECK(!state.uploadCallsPlanned80044D64);

    state = BuildStrStripUploadState8002756C(control, 2u, false, 0);
    CHECK(!state.known);
    CHECK(!state.displayBufferResultKnown);

    state = BuildStrStripUploadState8002756C(control, 2u, true, 0);
    CHECK(state.known);
    CHECK(!state.skippedForCounterWarmup);
    CHECK(state.displayBufferResultKnown);
    CHECK(state.displayBufferResult8004019C == 0);
    CHECK(state.destinationY == 48);
    CHECK(state.macroblockRows == 15u);
    CHECK(state.stripCount == 20u);
    CHECK(!state.truncated);
    CHECK(state.uploadCallsPlanned80044D64);
    CHECK(state.commands[0].x == 35);
    CHECK(state.commands[0].y == 48);
    CHECK(state.commands[0].width == 16);
    CHECK(state.commands[0].height == 240);
    CHECK(state.commands[0].sourcePitchTerm == 0u);
    CHECK(state.commands[0].sourceByteOffset == 0u);
    CHECK(state.commands[1].x == 51);
    CHECK(state.commands[1].sourcePitchTerm == 3840u);
    CHECK(state.commands[1].sourceByteOffset == 7680u);
    CHECK(state.commands[19].x == 339);
    CHECK(state.commands[19].sourceByteOffset == 145920u);
    CHECK(!state.gpuUploadExecuted);
    CHECK(!state.psxImagePointerAuthority);
    CHECK(!state.hostProjection);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);

    state = BuildStrStripUploadState8002756C(control, 2u, true, 1);
    CHECK(state.destinationY == 288);
    CHECK(state.commands[0].y == 288);

    control.decodedWidth = 8u;
    state = BuildStrStripUploadState8002756C(control, 2u, true, 0);
    CHECK(state.known);
    CHECK(state.skippedForZeroStrips);
    CHECK(state.stripCount == 0u);
    CHECK(!state.uploadCallsPlanned80044D64);
}

void TestStrStopResetState80027664() {
    const StrStopResetState80027664 state =
        BuildStrStopResetState80027664();
    CHECK(state.known);
    CHECK(state.clearDmaCallbackCalled80047658);
    CHECK(state.clearDmaCallbackArgument == 0);
    CHECK(state.clearDmaCallbackChannel == 1u);
    CHECK(state.stopCdCalled800392C0);
    CHECK(state.memoryResetCallCount80025AF8 == 4u);
    CHECK(state.exactCallOrder);
    CHECK(!state.runtimeWasActive);
    CHECK(!state.executionSkippedInactive);
    CHECK(!state.executionAccepted);
    CHECK(!state.dmaCallbackClearExecuted);
    CHECK(!state.cdStopExecuted);
    CHECK(!state.memoryResetExecuted);
    CHECK(!state.hostProjection);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);
}

void TestStrDecoderMemoryRuntimeAndStopResetExecution80027664() {
    StrDecoderAllocationState80027288 source{};
    CHECK(TryInitializeStrDecoderState80027288(
        0,
        false,
        source));
    StrDecoderMemoryRuntime80027288 runtime{};
    CHECK(TryInitializeStrDecoderMemoryRuntime80027288(
        source,
        runtime));
    CHECK(runtime.initialized);
    CHECK(runtime.movieKind == 0);
    CHECK(runtime.allocationStackDepth == 4u);
    CHECK(runtime.allocationBytes == 286756u);
    CHECK(runtime.allocationsZeroFilled);
    CHECK(runtime.allocationAlignmentVerified);
    CHECK(runtime.sectorRingInitialized8003624C);
    CHECK(runtime.sectorRingSlots8003624C == 32u);
    CHECK(runtime.sectorRingSlotBytes == 2048u);
    CHECK(runtime.sectorRingQueuedSlots == 0u);
    CHECK(runtime.outputCallbackInstalled80047658);
    CHECK(runtime.outputCallbackFunction80027220 == 0x80027220u);
    CHECK(runtime.outputCallbackDmaChannel80047658 == 1u);
    CHECK(runtime.directMemoryAuthority);
    CHECK(runtime.directSectorRingLifecycleAuthority);
    CHECK(!runtime.streamingCdActive8001A4D0);
    CHECK(!runtime.psxPointerAuthority);
    CHECK(!runtime.psxHardwareMmioAuthority);
    CHECK(!runtime.hardwareCallbackTimingAuthority);
    CHECK(!runtime.hostProjection);
    CHECK(!runtime.replayValueAuthority);
    CHECK(!runtime.oldWinS0Authority);
    CHECK(!runtime.stage2PlusAuthority);
    CHECK(!runtime.comod2Authority);
    for (uint32_t index = 0u; index < 4u; ++index) {
        CHECK(runtime.allocationPresent[index]);
        CHECK(runtime.allocations[index].size() ==
              source.allocationSizes[index]);
        CHECK(std::all_of(
            runtime.allocations[index].begin(),
            runtime.allocations[index].end(),
            [](uint8_t value) { return value == 0u; }));
    }
    CHECK(!TryInitializeStrDecoderMemoryRuntime80027288(
        source,
        runtime));

    const auto discPath = WriteSyntheticMode2StrDisc8001A4D0(1u, 1u);
    StrLowerCdStartSource8001A4D0 lowerSource{};
    lowerSource.segmentKnown = true;
    lowerSource.startLbaKnown = true;
    lowerSource.startLba = 2;
    lowerSource.lengthBytesKnown = true;
    lowerSource.lengthBytes = 2048u;
    lowerSource.filterKnown = true;
    lowerSource.filterFile = 1u;
    lowerSource.filterChannel = 1u;
    lowerSource.discBinPath = discPath;
    lowerSource.discImageDirectoryAuthority = true;
    StrLowerCdStartRuntime8001A4D0 lowerStart{};
    CHECK(TryInitializeStrLowerCdStartRuntime8001A4D0(
        lowerSource,
        lowerStart));
    CHECK(PublishStrDecoderStreamingStart8001A4D0(
        lowerStart,
        runtime));
    CHECK(runtime.streamingCdActive8001A4D0);
    CHECK(runtime.cdDataCallbackInstalled80036930);
    CHECK(runtime.cdDataCallbackFunction80039318 == 0x80039318u);
    CHECK(runtime.cdReadyCallbackInstalled80036528);
    CHECK(runtime.cdReadyCallbackFunction80039240 == 0x80039240u);
    CHECK(runtime.cdStatusByteKnown);
    CHECK(runtime.cdStatusByte == lowerStart.status0);
    CHECK(runtime.directStreamingCdStateAuthority);
    CHECK(runtime.directCdCommandStateAuthority);
    CHECK(!PublishStrDecoderStreamingStart8001A4D0(
        lowerStart,
        runtime));

    StrWorkBaseRuntime80049428 workBase{};
    CHECK(TryInitializeStrWorkBaseRuntime80049428(workBase));
    CHECK(PublishStrLowerCdStartCommand8001A4D0(lowerStart, workBase));
    auto syncCallback = InitializeStrCdSyncCallbackRuntime800570F8();
    const auto pause = ExecuteStrCommand8Pause800367A4(
        runtime, syncCallback, workBase);
    CHECK(pause.known);
    CHECK(pause.called);
    CHECK(pause.command == 8u);
    CHECK(pause.commandParameterTableSlotZero);
    CHECK(pause.commandResponseTableSlotZero);
    CHECK(pause.commandCallbackTableSlotZero);
    CHECK(pause.clearCallbackBeforeCommand.known);
    CHECK(pause.restoreCallbackBeforeSend.known);
    CHECK(pause.commandSinkCalled800375BC);
    CHECK(pause.commandLatchWritten80057119);
    CHECK(workBase.byte80057119 == 8u);
    CHECK(pause.syncWaitCalled80037070);
    CHECK(pause.syncState800573D4 == 2u);
    CHECK(pause.responseWritten80049414);
    CHECK(pause.response80049414[0] == lowerStart.status0);
    CHECK(runtime.cdSyncResponseKnown80049414);
    CHECK(runtime.cdSyncResponse80049414 == pause.response80049414);
    CHECK(runtime.cdSyncState800573D4 == 2u);
    CHECK(pause.pauseStreamingStateCommitted);
    CHECK(!runtime.streamingCdActive8001A4D0);
    CHECK(pause.attemptCount == 1u);
    CHECK(pause.returnValue == 1);
    CHECK(pause.exactSoftwareCallOrder);
    CHECK(!pause.psxHardwareMmioAuthority);
    CHECK(!pause.hardwareCallbackTimingAuthority);
    CHECK(!pause.hostProjection);
    CHECK(!pause.replayValueAuthority);
    CHECK(!pause.oldWinS0Authority);
    CHECK(!pause.stage2PlusAuthority);
    CHECK(!pause.comod2Authority);
    CHECK(syncCallback.callback == 0u);
    const auto repeatedPause = ExecuteStrCommand8Pause800367A4(
        runtime, syncCallback, workBase);
    CHECK(repeatedPause.known);
    CHECK(repeatedPause.returnValue == 1);
    CHECK(repeatedPause.directCdCommandStateAuthority);
    CHECK(!runtime.streamingCdActive8001A4D0);

    StrDecoderMemoryRuntime80027288 cleanupRuntime{};
    CHECK(TryInitializeStrDecoderMemoryRuntime80027288(
        source, cleanupRuntime));
    CHECK(PublishStrDecoderStreamingStart8001A4D0(
        lowerStart, cleanupRuntime));
    StrWorkBaseRuntime80049428 cleanupWorkBase{};
    CHECK(TryInitializeStrWorkBaseRuntime80049428(cleanupWorkBase));
    CHECK(PublishStrLowerCdStartCommand8001A4D0(
        lowerStart, cleanupWorkBase));
    auto cleanupCallback = InitializeStrCdSyncCallbackRuntime800570F8();
    CHECK(SetStrCdSyncCallback80036510(
              cleanupCallback, 0x80039240u).known);
    const auto waitCleanup = ExecuteStrWaitCleanup8001A694(
        cleanupRuntime, cleanupCallback, cleanupWorkBase);
    CHECK(waitCleanup.known);
    CHECK(waitCleanup.called);
    CHECK(waitCleanup.command8Pause.known);
    CHECK(waitCleanup.command8Pause.returnValue == 1);
    CHECK(waitCleanup.clearSyncCallback.known);
    CHECK(waitCleanup.clearSyncCallback.priorCallback == 0x80039240u);
    CHECK(cleanupCallback.callback == 0u);
    CHECK(waitCleanup.exactCallOrder);
    CHECK(waitCleanup.executionAccepted);
    CHECK(waitCleanup.currentScusSemanticAuthority);
    CHECK(waitCleanup.directLowerCdStateAuthority);
    CHECK(waitCleanup.directCdCommandStateAuthority);
    CHECK(!waitCleanup.hardwareCallbackTimingAuthority);
    CHECK(!waitCleanup.hostProjection);
    CHECK(!waitCleanup.replayValueAuthority);
    CHECK(!waitCleanup.oldWinS0Authority);
    CHECK(!waitCleanup.stage2PlusAuthority);
    CHECK(!waitCleanup.comod2Authority);

    auto rootCallback = InitializeStrRootCallbackRuntime80055F78();
    CHECK(rootCallback.initialized);
    CHECK(rootCallback.enabledKnown);
    CHECK(!rootCallback.enabled);
    CHECK(!rootCallback.interruptStatusKnown);
    CHECK(!rootCallback.interruptMaskKnown);
    CHECK(!rootCallback.dmaControlKnown);
    CHECK(rootCallback.savedInterruptMaskKnown);
    CHECK(rootCallback.savedInterruptMask80055FAA == 0u);
    CHECK(rootCallback.savedDmaControlKnown);
    CHECK(rootCallback.savedDmaControl80055FAC == 0u);
    CHECK(rootCallback.entryIntHookedKnown);
    CHECK(!rootCallback.entryIntHooked);
    CHECK(rootCallback.criticalSectionHeldKnown);
    CHECK(!rootCallback.criticalSectionHeld);
    CHECK(rootCallback.scusLoadedInitialZeroAuthority);
    CHECK(rootCallback.currentScusSemanticAuthority);
    CHECK(rootCallback.directSoftwareStateAuthority);

    const auto rootReset =
        ExecuteStrRootCallbackReset800358DC(rootCallback);
    CHECK(rootReset.known);
    CHECK(rootReset.called);
    CHECK(rootReset.sourceFunction == 0x800358DCu);
    CHECK(rootReset.callerFunction800473EC == 0x800473ECu);
    CHECK(rootReset.wrapperFunction80035744 == 0x80035744u);
    CHECK(rootReset.functionTableSlot80056FEC == 0x80056FECu);
    CHECK(rootReset.functionTableTarget800358DC == 0x800358DCu);
    CHECK(rootReset.enabledBeforeKnown);
    CHECK(!rootReset.enabledBefore);
    CHECK(!rootReset.noOpAlreadyEnabled);
    CHECK(rootReset.interruptMaskCleared);
    CHECK(rootReset.interruptStatusCleared);
    CHECK(rootReset.dmaControlInitialized);
    CHECK(rootReset.callbackStateInitialized80035E28);
    CHECK(rootReset.setjmpInitialReturnZero80047F5C);
    CHECK(rootReset.unexpectedInterruptDispatchSkipped800359B8);
    CHECK(rootReset.entryIntHooked);
    CHECK(rootReset.enabledWritten);
    CHECK(rootReset.defaultVSyncCallbackQueried80035E54);
    CHECK(rootReset.defaultDmaCallbackQueried80035F7C);
    CHECK(rootReset.memoryConfigurationCalled80048960);
    CHECK(rootReset.exitCriticalSectionExecuted);
    CHECK(rootReset.returnValue == 0x80055F78u);
    CHECK(rootReset.exactSoftwareCallOrder);
    CHECK(rootReset.executionAccepted);
    CHECK(rootReset.currentScusSemanticAuthority);
    CHECK(rootReset.directSoftwareStateAuthority);
    CHECK(!rootReset.psxHardwareMmioAuthority);
    CHECK(!rootReset.hardwareCallbackTimingAuthority);
    CHECK(!rootReset.hostProjection);
    CHECK(!rootReset.replayValueAuthority);
    CHECK(!rootReset.oldWinS0Authority);
    CHECK(!rootReset.stage2PlusAuthority);
    CHECK(!rootReset.comod2Authority);
    CHECK(rootCallback.enabled);
    CHECK(rootCallback.interruptStatusKnown);
    CHECK(rootCallback.interruptStatus == 0u);
    CHECK(rootCallback.interruptMaskKnown);
    CHECK(rootCallback.interruptMask == 0u);
    CHECK(rootCallback.dmaControlKnown);
    CHECK(rootCallback.dmaControl == 0x33333333u);
    CHECK(rootCallback.entryIntHooked);
    CHECK(!rootCallback.criticalSectionHeld);

    const auto repeatedRootReset =
        ExecuteStrRootCallbackReset800358DC(rootCallback);
    CHECK(repeatedRootReset.known);
    CHECK(repeatedRootReset.noOpAlreadyEnabled);
    CHECK(repeatedRootReset.returnValue == 0u);

    const auto rootStop = ExecuteStrStopCallback80035838(rootCallback);
    CHECK(rootStop.known);
    CHECK(rootStop.called);
    CHECK(rootStop.sourceFunction == 0x80035838u);
    CHECK(rootStop.functionTableSlot80056FF0 == 0x80056FF0u);
    CHECK(rootStop.functionTableTarget80035CF4 == 0x80035CF4u);
    CHECK(rootStop.enabledBeforeKnown);
    CHECK(rootStop.enabledBefore);
    CHECK(!rootStop.noOpAlreadyDisabled);
    CHECK(rootStop.enterCriticalSectionExecuted);
    CHECK(rootStop.interruptMaskSaved80055FAA);
    CHECK(rootStop.dmaControlSaved80055FAC);
    CHECK(rootStop.interruptMaskCleared);
    CHECK(rootStop.interruptStatusClearedFromMask);
    CHECK(rootStop.dmaControlMasked77777777);
    CHECK(rootStop.resetEntryIntExecuted);
    CHECK(rootStop.enabledCleared);
    CHECK(rootStop.returnValue == 0x80055F78u);
    CHECK(rootStop.exactSoftwareCallOrder);
    CHECK(rootStop.executionAccepted);
    CHECK(rootStop.currentScusSemanticAuthority);
    CHECK(rootStop.directSoftwareStateAuthority);
    CHECK(!rootStop.psxHardwareMmioAuthority);
    CHECK(!rootStop.hardwareCallbackTimingAuthority);
    CHECK(!rootStop.hostProjection);
    CHECK(!rootStop.replayValueAuthority);
    CHECK(!rootStop.oldWinS0Authority);
    CHECK(!rootStop.stage2PlusAuthority);
    CHECK(!rootStop.comod2Authority);
    CHECK(!rootCallback.enabled);
    CHECK(rootCallback.savedInterruptMask80055FAA == 0u);
    CHECK(rootCallback.savedDmaControl80055FAC == 0x33333333u);
    CHECK(rootCallback.interruptMask == 0u);
    CHECK(rootCallback.interruptStatus == 0u);
    CHECK(rootCallback.dmaControl == 0x33333333u);
    CHECK(!rootCallback.entryIntHooked);
    CHECK(rootCallback.criticalSectionHeld);

    const auto repeatedRootStop =
        ExecuteStrStopCallback80035838(rootCallback);
    CHECK(repeatedRootStop.known);
    CHECK(repeatedRootStop.noOpAlreadyDisabled);
    CHECK(repeatedRootStop.returnValue == 0u);

    const auto restartedRoot =
        ExecuteStrRootCallbackReset800358DC(rootCallback);
    CHECK(restartedRoot.known);
    CHECK(restartedRoot.executionAccepted);
    CHECK(rootCallback.enabled);
    CHECK(rootCallback.entryIntHooked);
    CHECK(!rootCallback.criticalSectionHeld);
    rootCallback.hostProjection = true;
    const auto rejectedRootStop =
        ExecuteStrStopCallback80035838(rootCallback);
    CHECK(!rejectedRootStop.known);
    CHECK(rootCallback.enabled);
    rootCallback.hostProjection = false;

    const auto prematureCleanupTail = ExecuteStrCleanupTailPrefix801C455C(
        cleanupRuntime,
        cleanupCallback,
        cleanupWorkBase,
        kScene0WorkAddress);
    CHECK(!prematureCleanupTail.known);
    CHECK(!prematureCleanupTail.stopReset80027664Observed);

    const auto cleanupStopReset =
        ExecuteStrStopReset80027664(cleanupRuntime);
    CHECK(cleanupStopReset.executionAccepted);
    CHECK(!cleanupRuntime.initialized);
    CHECK(!cleanupRuntime.directStreamingCdStateAuthority);
    CHECK(cleanupRuntime.directCdCommandStateAuthority);

    const auto mutedVolume = ExecuteStrSerialVolume8001A4A4(1);
    CHECK(mutedVolume.known);
    CHECK(mutedVolume.called);
    CHECK(mutedVolume.sourceFunction == 0x8001A4A4u);
    CHECK(mutedVolume.argumentComparedWithOne);
    CHECK(mutedVolume.mappedVolume == 0u);
    CHECK(mutedVolume.wrapperCalled8001A478);
    CHECK(mutedVolume.wrapperFunction8001A478 == 0x8001A478u);
    CHECK(mutedVolume.serialChannel8002AB24 == 0u);
    CHECK(mutedVolume.leftVolume8002AB24 == 0);
    CHECK(mutedVolume.rightVolume8002AB24 == 0);
    CHECK(mutedVolume.attributeMask8002AB24 == 0x00C0u);
    CHECK(mutedVolume.scaledLeftVolume8002AB24 == 0u);
    CHECK(mutedVolume.scaledRightVolume8002AB24 == 0u);
    CHECK(mutedVolume.serialAttributeCommandBuilt8002AB24);
    CHECK(mutedVolume.lowerCommonAttributeCalled8002A6FC);
    CHECK(mutedVolume.exactSoftwareCallOrder);
    CHECK(mutedVolume.executionAccepted);
    CHECK(mutedVolume.currentScusSemanticAuthority);
    CHECK(mutedVolume.currentComod0CallerAuthority);
    CHECK(mutedVolume.directAudioCommandStateAuthority);
    CHECK(!mutedVolume.psxSpuHardwareAuthority);
    CHECK(!mutedVolume.hostProjection);
    CHECK(!mutedVolume.replayValueAuthority);
    CHECK(!mutedVolume.oldWinS0Authority);
    CHECK(!mutedVolume.stage2PlusAuthority);
    CHECK(!mutedVolume.comod2Authority);

    const auto fullVolume = ExecuteStrSerialVolume8001A4A4(0);
    CHECK(fullVolume.known);
    CHECK(fullVolume.mappedVolume == 0x7Fu);
    CHECK(fullVolume.leftVolume8002AB24 == 0x7F);
    CHECK(fullVolume.rightVolume8002AB24 == 0x7F);
    CHECK(fullVolume.scaledLeftVolume8002AB24 == 0x7FFEu);
    CHECK(fullVolume.scaledRightVolume8002AB24 == 0x7FFEu);

    const auto nullSub = ExecuteStrNullSub80024CF0(kScene0WorkAddress);
    CHECK(nullSub.known);
    CHECK(nullSub.called);
    CHECK(nullSub.sourceFunction == 0x80024CF0u);
    CHECK(nullSub.contextAddress == kScene0WorkAddress);
    CHECK(nullSub.returnInstructionObserved);
    CHECK(nullSub.delaySlotNopObserved);
    CHECK(nullSub.contextUnmodified);
    CHECK(nullSub.exactNoOp);
    CHECK(nullSub.executionAccepted);
    CHECK(nullSub.currentScusSemanticAuthority);
    CHECK(nullSub.currentComod0CallerAuthority);
    CHECK(nullSub.directNoOpAuthority);

    const auto cleanupTailPrefix = ExecuteStrCleanupTailPrefix801C455C(
        cleanupRuntime,
        cleanupCallback,
        cleanupWorkBase,
        kScene0WorkAddress);
    CHECK(cleanupTailPrefix.known);
    CHECK(cleanupTailPrefix.called);
    CHECK(cleanupTailPrefix.stopReset80027664RequiredBeforeCall);
    CHECK(cleanupTailPrefix.stopReset80027664Observed);
    CHECK(cleanupTailPrefix.muteSerialVolume.known);
    CHECK(cleanupTailPrefix.muteSerialVolume.mappedVolume == 0u);
    CHECK(cleanupTailPrefix.secondWaitCleanup.known);
    CHECK(cleanupTailPrefix.secondWaitCleanup.executionAccepted);
    CHECK(cleanupTailPrefix.secondWaitCleanup.directCdCommandStateAuthority);
    CHECK(cleanupTailPrefix.contextNoOp.known);
    CHECK(cleanupTailPrefix.contextNoOp.exactNoOp);
    CHECK(cleanupTailPrefix.nextDisplayMovePending8001B120);
    CHECK(cleanupTailPrefix.exactCallOrder);
    CHECK(cleanupTailPrefix.executionAccepted);
    CHECK(cleanupTailPrefix.currentScusSemanticAuthority);
    CHECK(cleanupTailPrefix.currentComod0CallerAuthority);
    CHECK(cleanupTailPrefix.directSoftwareStateAuthority);
    CHECK(!cleanupTailPrefix.psxSpuHardwareAuthority);
    CHECK(!cleanupTailPrefix.psxGpuDmaAuthority);
    CHECK(!cleanupTailPrefix.hostProjection);
    CHECK(!cleanupTailPrefix.replayValueAuthority);
    CHECK(!cleanupTailPrefix.oldWinS0Authority);
    CHECK(!cleanupTailPrefix.stage2PlusAuthority);
    CHECK(!cleanupTailPrefix.comod2Authority);

    const StrStopResetState80027664 stop =
        ExecuteStrStopReset80027664(runtime);
    CHECK(stop.known);
    CHECK(stop.runtimeWasActive);
    CHECK(!stop.executionSkippedInactive);
    CHECK(stop.executionAccepted);
    CHECK(stop.dmaCallbackClearExecuted);
    CHECK(stop.enterCriticalSectionExecuted);
    CHECK(stop.cdDataCallbackClearExecuted80036930);
    CHECK(stop.cdReadyCallbackClearExecuted80036528);
    CHECK(stop.cdromReg0ClearExecuted);
    CHECK(stop.cdromReg3ClearExecuted);
    CHECK(stop.exitCriticalSectionExecuted);
    CHECK(stop.cdStopExecuted);
    CHECK(stop.memoryResetCountExecuted == 4u);
    CHECK(stop.memoryResetPointerSlotOrder[0] ==
          kDecoderImagePointerSlot800965B4);
    CHECK(stop.memoryResetPointerSlotOrder[1] ==
          kDecoderVlcPointerSlot800965B0);
    CHECK(stop.memoryResetPointerSlotOrder[2] ==
          kDecoderSectorPointerSlot800965AC);
    CHECK(stop.memoryResetPointerSlotOrder[3] ==
          kDecoderControlPointerSlot800965A8);
    CHECK(stop.memoryBytesReleased == 286756u);
    CHECK(stop.memoryResetExecuted);
    CHECK(stop.sectorRingReset);
    CHECK(stop.directDmaCallbackStateAuthority);
    CHECK(stop.directCdStateAuthority);
    CHECK(stop.directMemoryStackAuthority);
    CHECK(!stop.psxPointerAuthority);
    CHECK(!stop.psxHardwareMmioAuthority);
    CHECK(!stop.hardwareCallbackTimingAuthority);
    CHECK(!stop.hostProjection);
    CHECK(!stop.replayValueAuthority);
    CHECK(!stop.oldWinS0Authority);
    CHECK(!stop.stage2PlusAuthority);
    CHECK(!stop.comod2Authority);
    CHECK(!runtime.initialized);
    CHECK(runtime.allocationStackDepth == 0u);
    CHECK(runtime.allocationBytes == 0u);
    CHECK(!runtime.sectorRingInitialized8003624C);
    CHECK(!runtime.outputCallbackInstalled80047658);
    CHECK(!runtime.streamingCdActive8001A4D0);
    CHECK(runtime.directCdCommandStateAuthority);
    CHECK(runtime.cdromReg0Known);
    CHECK(runtime.cdromReg0 == 0u);
    CHECK(runtime.cdromReg3Known);
    CHECK(runtime.cdromReg3 == 0u);
    for (uint32_t index = 0u; index < 4u; ++index) {
        CHECK(!runtime.allocationPresent[index]);
        CHECK(runtime.allocations[index].empty());
        CHECK(runtime.allocations[index].capacity() == 0u);
    }

    const StrStopResetState80027664 inactive =
        ExecuteStrStopReset80027664(runtime);
    CHECK(inactive.known);
    CHECK(!inactive.runtimeWasActive);
    CHECK(inactive.executionSkippedInactive);
    CHECK(!inactive.executionAccepted);
    std::error_code ec;
    std::filesystem::remove(discPath, ec);
}

void TestStrClockPollState801C4350() {
    StrClockPollInput801C4350 input{};
    StrClockPollState801C4350 state =
        BuildStrClockPollState801C4350(input);
    CHECK(state.planned);
    CHECK(!state.executed);
    CHECK(state.hasOpenInputGap);
    CHECK(state.branch == StrClockPollBranch801C4350::UnknownInput);
    CHECK(state.clockQueryCalled8001A7A4);
    CHECK(state.branchSemanticsTranslated);
    CHECK(state.currentScusSemanticAuthority);
    CHECK(state.currentComod0CallerAuthority);
    CHECK(state.comod1ComparisonOnly);
    CHECK(!state.runtimeInputBound);
    CHECK(!state.psxCdClockAuthority);
    CHECK(!state.hostProjection);
    CHECK(!state.replayValueAuthority);
    CHECK(!state.oldWinS0Authority);
    CHECK(!state.stage2PlusAuthority);
    CHECK(!state.comod2Authority);

    input.clockDeltaKnown8001A7A4 = true;
    input.clockDelta8001A7A4 = -1;
    input.previousClockKnown801C954C = true;
    input.previousClock801C954C = 0;
    state = BuildStrClockPollState801C4350(input);
    CHECK(state.executed);
    CHECK(!state.hasOpenInputGap);
    CHECK(state.branch ==
          StrClockPollBranch801C4350::NegativeClockDelta);
    CHECK(state.returnValueKnown);
    CHECK(state.returnValue);
    CHECK(!state.clockFieldsUpdated);
    CHECK(!state.endCheckCalled8001A7F8);

    input.clockDelta8001A7A4 = 301;
    state = BuildStrClockPollState801C4350(input);
    CHECK(state.branch == StrClockPollBranch801C4350::Discontinuity);
    CHECK(state.absoluteClockDelta == 301u);
    CHECK(state.returnValueKnown);
    CHECK(state.returnValue);
    CHECK(!state.clockFieldsUpdated);

    input.clockDelta8001A7A4 = 9305;
    input.previousClock801C954C = 9005;
    state = BuildStrClockPollState801C4350(input);
    CHECK(state.branch == StrClockPollBranch801C4350::EndCheckUnknown);
    CHECK(state.absoluteClockDelta == 300u);
    CHECK(state.clockFieldsUpdated);
    CHECK(state.updatedPreviousClock == 9305);
    CHECK(state.minuteFieldOffset4 == 1);
    CHECK(state.secondFieldOffset6 == 2u);
    CHECK(state.frameFieldOffset7 == 1u);
    CHECK(state.endCheckCalled8001A7F8);
    CHECK(!state.returnValueKnown);
    CHECK(state.hasOpenInputGap);

    input.endCheckKnown8001A7F8 = true;
    input.endCheckReturn8001A7F8 = 0;
    state = BuildStrClockPollState801C4350(input);
    CHECK(state.branch == StrClockPollBranch801C4350::ContinuePlayback);
    CHECK(!state.hasOpenInputGap);
    CHECK(state.returnValueKnown);
    CHECK(state.returnValue);

    input.endCheckReturn8001A7F8 = 1;
    state = BuildStrClockPollState801C4350(input);
    CHECK(state.branch == StrClockPollBranch801C4350::EndPlayback);
    CHECK(state.returnValueKnown);
    CHECK(!state.returnValue);
}

void TestStrLowerCdClockRuntime801C4350() {
    StrLowerCdClockSource801C4350 source{};
    StrLowerCdClockRuntime801C4350 runtime{};
    CHECK(!TryInitializeStrLowerCdClockRuntime801C4350(source, runtime));
    CHECK(!runtime.initialized);

    source.segmentTimeBaseKnown = true;
    source.segmentTimeBase = 1000;
    source.segmentEndKnown = true;
    source.segmentEnd = 2000;
    source.segmentEndBiasKnown = true;
    source.segmentEndBias = 0;
    CHECK(!TryInitializeStrLowerCdClockRuntime801C4350(source, runtime));

    source.discImageDirectoryAuthority = true;
    CHECK(TryInitializeStrLowerCdClockRuntime801C4350(source, runtime));
    CHECK(runtime.initialized);
    CHECK(runtime.timeBaseLba == 1000);
    CHECK(runtime.endLba == 2000);
    CHECK(runtime.effectiveEndLba == 2000);
    CHECK(runtime.currentLba == 1000);
    CHECK(runtime.discImageDirectoryAuthority);
    CHECK(runtime.runtimeInputBound);
    CHECK(runtime.psxCdClockAuthority);
    CHECK(!runtime.hostClockAuthority);
    CHECK(!runtime.replayValueAuthority);
    CHECK(!runtime.oldWinS0Authority);
    CHECK(!runtime.stage2PlusAuthority);
    CHECK(!runtime.comod2Authority);

    StrLowerCdClockTick801C4350 tick =
        StepStrLowerCdClockRuntime801C4350(runtime);
    CHECK(tick.known);
    CHECK(tick.polled);
    CHECK(tick.currentLba == 1005);
    CHECK(tick.relativeClock == 5);
    CHECK(tick.deadlineLba == 1155);
    CHECK(!tick.endCheckReturn);
    CHECK(!tick.terminal);
    CHECK(tick.poll.branch ==
          StrClockPollBranch801C4350::ContinuePlayback);
    CHECK(tick.poll.runtimeInputBound);
    CHECK(tick.poll.psxCdClockAuthority);
    CHECK(tick.poll.minuteFieldOffset4 == 0);
    CHECK(tick.poll.secondFieldOffset6 == 0u);
    CHECK(tick.poll.frameFieldOffset7 == 1u);
    CHECK(runtime.previousClock == 5);
    CHECK(runtime.pollCount == 1u);

    for (uint32_t index = 0u; index < 168u; ++index) {
        tick = StepStrLowerCdClockRuntime801C4350(runtime);
        CHECK(tick.known);
    }
    CHECK(tick.currentLba == 1845);
    CHECK(tick.deadlineLba == 1995);
    CHECK(!tick.endCheckReturn);
    CHECK(!tick.terminal);
    tick = StepStrLowerCdClockRuntime801C4350(runtime);
    CHECK(tick.currentLba == 1850);
    CHECK(tick.deadlineLba == 2000);
    CHECK(tick.endCheckReturn);
    CHECK(tick.terminal);
    CHECK(tick.poll.branch == StrClockPollBranch801C4350::EndPlayback);

    source.segmentTimeBase = 1000;
    source.segmentEnd = 1200;
    source.segmentEndBias = -50;
    CHECK(TryInitializeStrLowerCdClockRuntime801C4350(source, runtime));
    CHECK(runtime.effectiveEndLba == 1150);

    source.segmentEndBias = -201;
    CHECK(!TryInitializeStrLowerCdClockRuntime801C4350(source, runtime));
    source.segmentTimeBase = INT32_MIN;
    source.segmentEnd = INT32_MAX;
    source.segmentEndBias = 0;
    CHECK(!TryInitializeStrLowerCdClockRuntime801C4350(source, runtime));
}

void TestStrLowerCdClockOwnerScope801C4350() {
    CHECK(UsesStrLowerCdClockRuntime801C4350(
        StrMovieKind::OpeningMovie0));
    CHECK(!UsesStrLowerCdClockRuntime801C4350(
        StrMovieKind::TitleMovie0T));
    CHECK(!UsesStrLowerCdClockRuntime801C4350(
        StrMovieKind::Unknown));
}

void TestStartPlanOpeningMovieOrderAndArgs() {
    const uint32_t segmentAddress = 0x12340000u;
    const StrPlan plan = BuildStrStart801C44E0Plan(segmentAddress, 0, false);
    CHECK(plan.kind == StrPlanKind::Start801C44E0);
    CHECK(!plan.runtimeCutoverAllowed);
    CHECK(plan.hasOpenP0Gap);
    CHECK(!plan.truncated);
    const StrAction* volume8001A478 =
        FindAction(plan, StrActionKind::Call8001A478);
    CHECK(volume8001A478 != nullptr);
    CHECK(volume8001A478->arg0 == segmentAddress + 6u);

    const StrActionKind expected[] = {
        StrActionKind::GateStrStartSource801C44E0,
        StrActionKind::Call801C44E0,
        StrActionKind::Call80026FA4,
        StrActionKind::Call80024E98,
        StrActionKind::SetMovieKindNotOneFlag,
        StrActionKind::Call8001A478,
        StrActionKind::Call80027288AllocDecoder,
        StrActionKind::Call8001A4D0StartStream,
        StrActionKind::Call80036A78SetlocLba,
        StrActionKind::Call800367A4SetlocCommand,
        StrActionKind::Poll800364D0SetlocComplete,
        StrActionKind::Call800367A4SetfilterCommand,
        StrActionKind::Poll800364D0SetfilterComplete,
        StrActionKind::Set8001A4D0StreamModeWord,
        StrActionKind::Set8001A4D0BufferCadence,
        StrActionKind::Call800391ACSetCdMode,
        StrActionKind::Call80036540SetModeCommand,
        StrActionKind::Call800391ACInstallDmaCallback,
        StrActionKind::Call800391ACInstallDataCallback,
        StrActionKind::Call80036540CommitModeCommand,
        StrActionKind::Call80035560StartWait,
        StrActionKind::Poll800364D0StreamStartComplete,
        StrActionKind::Set8001A4D0StartFlags,
        StrActionKind::Call80036678PrimeStatus,
        StrActionKind::Call800274D4StartStream,
        StrActionKind::CallStSetStreamStart,
        StrActionKind::CallDecDCTvlcSize2Reset,
        StrActionKind::Call800273A4DecodeFallback,
        StrActionKind::Gap,
    };
    CheckOrder(plan, expected, sizeof(expected) / sizeof(expected[0]));

    const StrAction* gate =
        FindAction(plan, StrActionKind::GateStrStartSource801C44E0);
    CHECK(gate != nullptr);
    CHECK(gate->psxFunction == kFn801C44E0);
    CHECK(gate->arg0 == segmentAddress);
    CHECK(gate->arg1 == 0);
    CHECK(gate->arg2 == 0);
    CHECK(gate->arg3 == 1);
    CHECK(gate->conditional);

    const StrAction* alloc =
        FindAction(plan, StrActionKind::Call80027288AllocDecoder);
    CHECK(alloc != nullptr);
    CHECK(alloc->arg0 == 0);
    CHECK(alloc->arg1 == 256);
    CHECK(alloc->arg2 == 144);
    CHECK(alloc->arg3 == ((35u << 16) | 48u));

    const StrAction* setloc =
        FindAction(plan, StrActionKind::Call800367A4SetlocCommand);
    CHECK(setloc != nullptr);
    CHECK(setloc->arg0 == kCdCommandSetloc);
    CHECK(setloc->arg1 == kDword800493EC);
    CHECK(setloc->arg2 == kDword80049414);

    const StrAction* setMode =
        FindAction(plan, StrActionKind::Call80036540SetModeCommand);
    CHECK(setMode != nullptr);
    CHECK(setMode->arg0 == kCdCommandSetmode);
    CHECK(setMode->arg1 == kStreamingCdModeByte);
    CHECK(setMode->arg2 == kStreamingCdModeWord);

    const StrAction* gap = FindAction(plan, StrActionKind::Gap);
    CHECK(gap != nullptr);
    CHECK(gap->psxFunction == kFn8001A4D0);
    CHECK(gap->arg0 == kStreamingCdModeWord);
}

void TestLoopPlanUnknownAndExitBranches() {
    const uint32_t segmentAddress = 0x20000000u;
    const uint32_t ctxAddress = 0x801C3640u;
    StrPlan plan = BuildStrLoop801C455CPlan(
        segmentAddress, ctxAddress, 0, 0, false, false, 0);
    CHECK(plan.kind == StrPlanKind::Loop801C455C);
    CHECK(!plan.runtimeCutoverAllowed);
    CHECK(plan.hasOpenP0Gap);

    const StrActionKind expected[] = {
        StrActionKind::GateStrLoopSource801C455C,
        StrActionKind::Call801C455C,
        StrActionKind::WarmupPoll8001A750,
        StrActionKind::Call80036678StatusRetry,
        StrActionKind::GateStrInputSource80035510,
        StrActionKind::Call80035510ReadInput,
        StrActionKind::Call80024CF8Events,
        StrActionKind::Call8001EC54TextEvent,
        StrActionKind::Call8001ED3CTextFlush,
        StrActionKind::Call80027528DecodeFrame,
        StrActionKind::Call800273A4DecodeFallback,
        StrActionKind::Call801C448CStepMovie,
        StrActionKind::Call801C4350UpdateClock,
        StrActionKind::Call8001A7A4QueryClock,
        StrActionKind::Call8001A7F8CheckEnd,
        StrActionKind::Call8001ED74PrepareFlip,
        StrActionKind::Call80040370Flip,
        StrActionKind::Call8002756CUploadStrips,
        StrActionKind::Call80044D64LoadImage,
        StrActionKind::Gap,
    };
    CheckOrder(plan, expected, sizeof(expected) / sizeof(expected[0]));

    const StrAction* inputGate =
        FindAction(plan, StrActionKind::GateStrInputSource80035510);
    CHECK(inputGate != nullptr);
    CHECK(inputGate->arg0 == 0);
    CHECK(inputGate->arg1 == 0);
    CHECK(inputGate->arg2 == kInputForceReturnOne);
    CHECK(inputGate->arg3 == kInputExitMask);
    CHECK(inputGate->conditional);

    const StrAction* upload =
        FindAction(plan, StrActionKind::Call8002756CUploadStrips);
    CHECK(upload != nullptr);
    CHECK(upload->arg0 == 256);
    CHECK(upload->arg1 == 144);
    CHECK(upload->arg2 == 35);
    CHECK(upload->arg3 == 48);
    CHECK(upload->repeatCount == 16);
    CHECK(upload->repeatKnown);

    const StrAction* load =
        FindAction(plan, StrActionKind::Call80044D64LoadImage);
    CHECK(load != nullptr);
    CHECK(load->arg0 == kUploadStripWidth);
    CHECK(load->arg1 == 144);
    CHECK(load->arg2 == kMacroblockBytes);
    CHECK(load->arg3 == 48);
    CHECK(load->repeatCount == 16);

    plan = BuildStrLoop801C455CPlan(
        segmentAddress, ctxAddress, 0, 0, true, true, kInputForceReturnOne);
    CHECK(FindAction(plan, StrActionKind::Call80024CF8Events) == nullptr);
    CHECK(FindAction(plan, StrActionKind::Call8001B120DisplayUpload) != nullptr);
    const StrAction* ret = FindAction(plan, StrActionKind::ReturnValue);
    CHECK(ret != nullptr);
    CHECK(ret->arg0 == 1);
    CHECK(CountActions(plan, StrActionKind::Gap) >= 1);

    plan = BuildStrLoop801C455CPlan(
        segmentAddress, ctxAddress, 0, 0, true, true, kInputDirectExit);
    ret = FindAction(plan, StrActionKind::ReturnValue);
    CHECK(ret != nullptr);
    CHECK(ret->arg0 == 0);
}

void TestCleanupAndCompositePlans() {
    StrPlan plan = BuildStrCleanup801C455CPlan(true);
    CHECK(plan.kind == StrPlanKind::Cleanup801C455C);
    CHECK(!plan.runtimeCutoverAllowed);
    CHECK(plan.hasOpenP0Gap);
    CHECK(FindAction(plan, StrActionKind::Call8001B120DisplayUpload) != nullptr);

    const StrActionKind cleanupExpected[] = {
        StrActionKind::GateStrCleanupSource80027664,
        StrActionKind::Call80027664StopReset,
        StrActionKind::Call80047658ClearCallback,
        StrActionKind::CallDMACallbackClear,
        StrActionKind::Call800392C0StopCd,
        StrActionKind::Call800392C0EnterCritical,
        StrActionKind::Call80036930CdReset,
        StrActionKind::Call80036528CdReset,
        StrActionKind::Write800392C0CdromMmioClear,
        StrActionKind::Call800392C0ExitCritical,
        StrActionKind::Call80025AF8ResetRetry,
        StrActionKind::Call8001A4A4StreamCommand,
        StrActionKind::Call8001A694WaitCleanup,
        StrActionKind::Call80024CF0NullSub,
        StrActionKind::Call8001B120DisplayUpload,
        StrActionKind::Gap,
    };
    CheckOrder(plan, cleanupExpected,
               sizeof(cleanupExpected) / sizeof(cleanupExpected[0]));

    const StrAction* gate =
        FindAction(plan, StrActionKind::GateStrCleanupSource80027664);
    CHECK(gate != nullptr);
    CHECK(gate->arg0 == 1);
    CHECK(gate->arg1 == kFn8001A4A4);
    CHECK(gate->arg2 == kFn8001A694);
    CHECK(gate->arg3 == kFn8001B120);
    CHECK(gate->conditional);

    const StrAction* nullSubAction =
        FindAction(plan, StrActionKind::Call80024CF0NullSub);
    CHECK(nullSubAction != nullptr);
    CHECK(nullSubAction->psxFunction == kFn80024CF0);
    CHECK(nullSubAction->arg0 == kScene0WorkAddress);

    const StrAction* mmio =
        FindAction(plan, StrActionKind::Write800392C0CdromMmioClear);
    CHECK(mmio != nullptr);
    CHECK(mmio->arg0 == kCdromIoReg0);
    CHECK(mmio->arg1 == 0);
    CHECK(mmio->arg2 == kCdromIoReg3);
    CHECK(mmio->arg3 == 0);

    plan = BuildStrCleanup801C455CPlan(false);
    CHECK(FindAction(plan, StrActionKind::Call8001B120DisplayUpload) == nullptr);

    const uint32_t sceneEntry = 0x801C1000u;
    plan = BuildScene0Movie0PlaybackPlan(sceneEntry, false, false);
    CHECK(plan.kind == StrPlanKind::Scene0Movie0Playback);
    CHECK(!plan.truncated);
    CHECK(FindAction(plan, StrActionKind::Call801C44E0)->arg0 ==
          sceneEntry + kSceneEntryMovie0SegmentOffset);
    CHECK(FindAction(plan, StrActionKind::Call801C455C)->arg1 ==
          kScene0WorkAddress);
    CHECK(FindAction(plan, StrActionKind::Call8001B120DisplayUpload) != nullptr);

    plan = BuildTitleMovie0TStartPlan(sceneEntry, true);
    CHECK(plan.kind == StrPlanKind::TitleMovie0TStart);
    CHECK(!plan.truncated);
    const StrAction* start = FindAction(plan, StrActionKind::Call801C44E0);
    CHECK(start != nullptr);
    CHECK(start->arg0 == sceneEntry + kSceneEntryMovie0TSegmentOffset);
    CHECK(start->arg1 == static_cast<uint32_t>(StrMovieKind::TitleMovie0T));
    const StrAction* alloc =
        FindAction(plan, StrActionKind::Call80027288AllocDecoder);
    CHECK(alloc != nullptr);
    CHECK(alloc->arg1 == 320);
    CHECK(alloc->arg2 == 240);
    CHECK(alloc->arg3 == 0);
}

void TestUnknownMovieGeometryFailsClosed() {
    const StrPlan plan =
        BuildStrLoop801C455CPlan(0x20000000u, kScene0WorkAddress, 0,
                                 99, false, false, 0);
    const StrAction* upload =
        FindAction(plan, StrActionKind::Call8002756CUploadStrips);
    CHECK(upload != nullptr);
    CHECK(upload->arg0 == 0);
    CHECK(upload->arg1 == 0);
    CHECK(upload->arg2 == 0);
    CHECK(upload->arg3 == 0);
    CHECK(upload->repeatCount == 0);
    CHECK(!upload->repeatKnown);

    const StrAction* load =
        FindAction(plan, StrActionKind::Call80044D64LoadImage);
    CHECK(load != nullptr);
    CHECK(load->arg0 == kUploadStripWidth);
    CHECK(load->arg1 == 0);
    CHECK(load->arg2 == kMacroblockBytes);
    CHECK(load->arg3 == 0);
    CHECK(load->repeatCount == 0);
    CHECK(!load->repeatKnown);
    CHECK(FindAction(plan, StrActionKind::Gap) != nullptr);
}

void TestScene0FastTransitionCadence800201AC() {
    using PrSS0TransitionDirect::FastTransitionFrameKind800201AC;
    using PrSS0TransitionDirect::FastTransitionRuntime800201AC;
    using PrSS0TransitionDirect::FastTransitionRuntimePhase800201AC;

    FastTransitionRuntime800201AC runtime{};
    CHECK(!PrSS0TransitionDirect::BeginFastTransitionRuntime800201AC(
        runtime, 0x801C0000u, 6, 1, 2, true));
    CHECK(!runtime.active);
    CHECK(!PrSS0TransitionDirect::BeginFastTransitionRuntime800201AC(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        2,
        1,
        2,
        true));
    CHECK(!PrSS0TransitionDirect::BeginFastTransitionRuntime800201AC(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        6,
        2,
        1,
        true));

    CHECK(PrSS0TransitionDirect::BeginFastTransitionRuntime800201AC(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        6,
        1,
        2,
        true));
    CHECK(runtime.active);
    CHECK(runtime.phase ==
          FastTransitionRuntimePhase800201AC::Loop8001EA74);
    CHECK(runtime.loopIterationsRequired == 31u);
    CHECK(runtime.tileMaskKnown8001EEAC);
    CHECK(!runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 0u);
    CHECK(runtime.tileMaskMutationSerial8001EEAC == 1u);
    for (uint32_t cell : runtime.tileMask8001EEAC) {
        CHECK(cell == 0u);
    }

    PrSS0TransitionDirect::FastTransitionTickResult800201AC tick{};
    for (uint32_t hostTick = 1; hostTick <= 70u; ++hostTick) {
        tick = PrSS0TransitionDirect::TickFastTransitionRuntime800201AC(
            runtime);
        CHECK(tick.accepted);
        const bool expectedCue = hostTick == 1u || hostTick == 31u;
        CHECK(tick.sub800271E4CallRequired == expectedCue);
        if (expectedCue) {
            const uint8_t expectedIndex = hostTick == 1u ? 1u : 0u;
            CHECK(tick.sub800271E4CueIndex == expectedIndex);
            CHECK(tick.sub800271E4CueAddress ==
                  PrSS0TransitionDirect::kCueGlobal9441C +
                      6u * static_cast<uint32_t>(expectedIndex));
            CHECK(tick.sub800271E4SourceFunction ==
                  PrSS0TransitionDirect::kFn800271E4);
        }
        if (hostTick == 1u) {
            CHECK(runtime.initialCue800271E4Dispatched);
        }
        const uint32_t bodyIteration = (hostTick - 1u) / 2u;
        const uint32_t expectedGp196 = hostTick <= 62u
            ? (bodyIteration + 1u >= 31u ? 192u : bodyIteration + 1u)
            : 192u;
        CHECK(tick.gp196 == expectedGp196);
        CHECK(tick.tileMaskMutationApplied8001EEAC == (hostTick == 63u));
        CHECK(tick.tileMaskMutationSerial8001EEAC ==
              (hostTick >= 63u ? 2u : 1u));
        CHECK(tick.visualFrame.known);
        CHECK(tick.visualFrame.mode == 6);
        CHECK(tick.visualFrame.clearColorKnown);
        CHECK(tick.visualFrame.clearR == 0xFFu);
        CHECK(tick.visualFrame.clearG == 0xFFu);
        CHECK(tick.visualFrame.clearB == 0xFFu);
        CHECK(tick.visualFrame.iteration ==
              (hostTick <= 62u ? (hostTick - 1u) / 2u : 190u));
        if (hostTick <= 30u) {
            CHECK(tick.visualFrame.kind ==
                  FastTransitionFrameKind800201AC::
                      OutroNoSubboxFrame8001F230);
        } else {
            CHECK(tick.visualFrame.kind ==
                  FastTransitionFrameKind800201AC::
                      SubtitleFrame8001C864);
        }
        CHECK(tick.complete == (hostTick == 70u));
    }
    CHECK(!runtime.active);
    CHECK(runtime.phase == FastTransitionRuntimePhase800201AC::Complete);
    CHECK(tick.loopIterationsCompleted == 31u);
    CHECK(tick.tailIterationsCompleted == 4u);
    CHECK(runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 192u);
    CHECK(runtime.tileMaskMutationSerial8001EEAC == 2u);
    for (uint32_t cell : runtime.tileMask8001EEAC) {
        CHECK(cell == 1u);
    }

    CHECK(PrSS0TransitionDirect::BeginFastTransitionRuntime800201AC(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        5,
        1,
        2,
        false));
    CHECK(runtime.loopIterationsRequired == 16u);
    CHECK(runtime.tileMaskKnown8001EEAC);
    CHECK(runtime.tileMaskMutationSerial8001EEAC == 1u);
    CHECK(runtime.gp196 == 0u);
    for (uint32_t hostTick = 1; hostTick <= 40u; ++hostTick) {
        tick = PrSS0TransitionDirect::TickFastTransitionRuntime800201AC(
            runtime);
        CHECK(tick.accepted);
        CHECK(tick.sub800271E4CallRequired == (hostTick == 1u));
        if (hostTick == 1u) {
            CHECK(tick.sub800271E4CueIndex == 0u);
            CHECK(tick.sub800271E4CueAddress ==
                  PrSS0TransitionDirect::kCueGlobal9441C);
            CHECK(tick.sub800271E4SourceFunction ==
                  PrSS0TransitionDirect::kFn800271E4);
            CHECK(runtime.initialCue800271E4Dispatched);
        }
        CHECK(tick.visualFrame.known);
        CHECK(!tick.visualFrame.clearColorKnown);
        CHECK(tick.visualFrame.kind ==
              FastTransitionFrameKind800201AC::
                      FinalNoVideoFrame8001FEB4);
        CHECK(tick.tileMaskMutationApplied8001EEAC == (hostTick == 33u));
        CHECK(tick.tileMaskMutationSerial8001EEAC ==
              (hostTick >= 33u ? 2u : 1u));
        CHECK(tick.gp196 ==
              (hostTick <= 30u ? (hostTick + 1u) / 2u : 192u));
        CHECK(tick.complete == (hostTick == 40u));
    }
    CHECK(tick.loopIterationsCompleted == 16u);
    CHECK(tick.tailIterationsCompleted == 4u);
    CHECK(runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 192u);
    CHECK(runtime.tileMaskMutationSerial8001EEAC == 2u);
    for (uint32_t cell : runtime.tileMask8001EEAC) {
        CHECK(cell == 1u);
    }

    CHECK(PrSS0TransitionDirect::BeginFastTransitionRuntime800201AC(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        5,
        1,
        2,
        true));
    CHECK(runtime.tileMaskKnown8001EEAC);
    CHECK(runtime.tileMaskMutationSerial8001EEAC == 1u);
    CHECK(runtime.gp196 == 0u);
    for (uint32_t hostTick = 1; hostTick <= 70u; ++hostTick) {
        tick = PrSS0TransitionDirect::TickFastTransitionRuntime800201AC(
            runtime);
        CHECK(tick.accepted);
        const bool expectedCue = hostTick == 1u || hostTick == 31u;
        CHECK(tick.sub800271E4CallRequired == expectedCue);
        if (expectedCue) {
            const uint8_t expectedIndex = hostTick == 1u ? 0u : 1u;
            CHECK(tick.sub800271E4CueIndex == expectedIndex);
            CHECK(tick.sub800271E4CueAddress ==
                  PrSS0TransitionDirect::kCueGlobal9441C +
                      6u * static_cast<uint32_t>(expectedIndex));
            CHECK(tick.sub800271E4SourceFunction ==
                  PrSS0TransitionDirect::kFn800271E4);
        }
        CHECK(tick.tileMaskMutationApplied8001EEAC == (hostTick == 63u));
        CHECK(tick.tileMaskMutationSerial8001EEAC ==
              (hostTick >= 63u ? 2u : 1u));
    }
    CHECK(!runtime.active);
    CHECK(runtime.phase == FastTransitionRuntimePhase800201AC::Complete);
    CHECK(tick.loopIterationsCompleted == 31u);
    CHECK(tick.tailIterationsCompleted == 4u);
    CHECK(runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 192u);
    CHECK(runtime.tileMaskMutationSerial8001EEAC == 2u);
    for (uint32_t cell : runtime.tileMask8001EEAC) {
        CHECK(cell == 1u);
    }

    PrSS0TransitionDirect::ResetFastTransitionRuntime800201AC(runtime);
    tick = PrSS0TransitionDirect::TickFastTransitionRuntime800201AC(runtime);
    CHECK(!tick.accepted);
    CHECK(!tick.complete);

    const auto finalVisual =
        PrSS0TransitionDirect::ResolveFastTransitionVisualFrame800201AC(
            5, true, false, 15u);
    CHECK(finalVisual.known);
    CHECK(finalVisual.clearColorKnown);
    CHECK(finalVisual.sourceFunction == PrSS0TransitionDirect::kFn80020308);
    CHECK(finalVisual.kind ==
          FastTransitionFrameKind800201AC::FinalNoVideoFrame8001FEB4);
    const auto finalPlan =
        PrSS0TransitionDirect::BuildFastTransitionFramePlan800201AC(
            finalVisual);
    CHECK(finalPlan.known);
    CHECK(!finalPlan.truncated);
    CHECK(finalPlan.commandCount == 192u);
    CHECK(finalPlan.commands[0].x == 0);
    CHECK(finalPlan.commands[0].y == 0);
    CHECK(finalPlan.commands[0].spriteTemplate.sourceAddress ==
          0x800503E0u);
    CHECK(finalPlan.commands[0].priority == 0u);
    CHECK(finalPlan.commands[0].rawTextureKnown);
    CHECK(finalPlan.commands[0].rawTexture);
    CHECK(!finalPlan.commands[0].rgbKnown);
    CHECK(finalPlan.commands[191].x == 300);
    CHECK(finalPlan.commands[191].y == 220);
    CHECK(finalPlan.commands[191].spriteTemplate.sourceAddress ==
          0x80050410u);
    for (std::size_t i = 0; i < finalPlan.commandCount; ++i) {
        CHECK(finalPlan.commands[i].rawTextureKnown);
        CHECK(finalPlan.commands[i].rawTexture);
        CHECK(finalPlan.commands[i].semiTransparentKnown);
        CHECK(!finalPlan.commands[i].semiTransparent);
        CHECK(finalPlan.commands[i].abrKnown);
        CHECK(finalPlan.commands[i].abr == 1u);
        CHECK(!finalPlan.commands[i].rgbKnown);
    }

    const auto outroVisual =
        PrSS0TransitionDirect::ResolveFastTransitionVisualFrame800201AC(
            5, true, false, 0u);
    CHECK(outroVisual.known);
    CHECK(outroVisual.kind ==
          FastTransitionFrameKind800201AC::OutroNoSubboxFrame8001F230);
    const auto outroPlan =
        PrSS0TransitionDirect::BuildFastTransitionFramePlan800201AC(
            outroVisual);
    CHECK(outroPlan.known);
    CHECK(!outroPlan.truncated);
    CHECK(outroPlan.commandCount == 86u);
    CHECK(outroPlan.commands[0].x == 20);
    CHECK(outroPlan.commands[0].y == 20);
    CHECK(outroPlan.commands[0].spriteTemplate.sourceAddress ==
          0x80050380u);
    CHECK(outroPlan.commands[0].priority == 5u);
    CHECK(outroPlan.commands[0].rawTextureKnown);
    CHECK(outroPlan.commands[0].rawTexture);
    CHECK(!outroPlan.commands[0].rgbKnown);
    CHECK(outroPlan.commands[5].x == 160);
    CHECK(outroPlan.commands[5].y == 160);
    CHECK(outroPlan.commands[5].spriteTemplate.sourceAddress ==
          0x800503D0u);
    CHECK(outroPlan.commands[33].x == 260);
    CHECK(outroPlan.commands[33].y == 220);
    CHECK(outroPlan.commands[33].spriteTemplate.sourceAddress ==
          0x80050410u);
    CHECK(outroPlan.commands[34].x == 0);
    CHECK(outroPlan.commands[34].y == 20);
    CHECK(outroPlan.commands[34].spriteTemplate.sourceAddress ==
          0x80050400u);
    CHECK(outroPlan.commands[85].x == 20);
    CHECK(outroPlan.commands[85].y == 180);
    CHECK(outroPlan.commands[85].spriteTemplate.sourceAddress ==
          0x80050410u);
    for (std::size_t i = 0; i < outroPlan.commandCount; ++i) {
        CHECK(outroPlan.commands[i].rawTextureKnown);
        CHECK(outroPlan.commands[i].rawTexture);
        CHECK(outroPlan.commands[i].semiTransparentKnown);
        CHECK(!outroPlan.commands[i].semiTransparent);
        CHECK(outroPlan.commands[i].abrKnown);
        CHECK(outroPlan.commands[i].abr == 1u);
        CHECK(!outroPlan.commands[i].rgbKnown);
    }

    const auto subtitleVisual =
        PrSS0TransitionDirect::ResolveFastTransitionVisualFrame800201AC(
            6, true, false, 15u);
    CHECK(subtitleVisual.known);
    CHECK(subtitleVisual.kind ==
          FastTransitionFrameKind800201AC::SubtitleFrame8001C864);
    const auto subtitlePlan =
        PrSS0TransitionDirect::BuildFastTransitionFramePlan800201AC(
            subtitleVisual);
    CHECK(subtitlePlan.known);
    CHECK(!subtitlePlan.truncated);
    CHECK(subtitlePlan.commandCount == 86u);
    CHECK(subtitlePlan.commands[0].x == 280);
    CHECK(subtitlePlan.commands[0].y == 200);
    CHECK(subtitlePlan.commands[0].spriteTemplate.sourceAddress ==
          0x8004E940u);
    CHECK(subtitlePlan.commands[43].x == 300);
    CHECK(subtitlePlan.commands[43].y == 40);
    CHECK(subtitlePlan.commands[43].spriteTemplate.sourceAddress ==
          0x8004E910u);
    CHECK(subtitlePlan.commands[85].x == 280);
    CHECK(subtitlePlan.commands[85].y == 20);
    CHECK(subtitlePlan.commands[85].spriteTemplate.sourceAddress ==
          0x8004E8B0u);
    for (std::size_t i = 0; i < subtitlePlan.commandCount; ++i) {
        CHECK(subtitlePlan.commands[i].priority == 5u);
        CHECK(subtitlePlan.commands[i].rawTextureKnown);
        CHECK(subtitlePlan.commands[i].rawTexture);
        CHECK(subtitlePlan.commands[i].semiTransparentKnown);
        CHECK(!subtitlePlan.commands[i].semiTransparent);
        CHECK(subtitlePlan.commands[i].abrKnown);
        CHECK(subtitlePlan.commands[i].abr == 1u);
        CHECK(!subtitlePlan.commands[i].rgbKnown);
    }

    const auto noSubtitleVisual =
        PrSS0TransitionDirect::ResolveFastTransitionVisualFrame800201AC(
            6, false, false, 0u);
    CHECK(noSubtitleVisual.known);
    CHECK(noSubtitleVisual.kind ==
          FastTransitionFrameKind800201AC::NoSubtitleFrame8001CE30);
    CHECK(noSubtitleVisual.clearColorKnown);
    const auto noSubtitlePlan =
        PrSS0TransitionDirect::BuildFastTransitionFramePlan800201AC(
            noSubtitleVisual);
    CHECK(noSubtitlePlan.known);
    CHECK(!noSubtitlePlan.truncated);
    CHECK(noSubtitlePlan.commandCount == 106u);
    CHECK(noSubtitlePlan.commands[0].x == 40);
    CHECK(noSubtitlePlan.commands[0].y == 0);
    CHECK(noSubtitlePlan.commands[0].spriteTemplate.sourceAddress ==
          0x8004E900u);
    CHECK(noSubtitlePlan.commands[53].x == 260);
    CHECK(noSubtitlePlan.commands[53].y == 20);
    CHECK(noSubtitlePlan.commands[53].spriteTemplate.sourceAddress ==
          0x8004E930u);
    CHECK(noSubtitlePlan.commands[105].x == 40);
    CHECK(noSubtitlePlan.commands[105].y == 40);
    CHECK(noSubtitlePlan.commands[105].spriteTemplate.sourceAddress ==
          0x8004E990u);
    std::size_t semiTransparentCount = 0u;
    for (std::size_t i = 0; i < noSubtitlePlan.commandCount; ++i) {
        const bool expectedSemiTransparent = i >= 84u && i <= 91u;
        CHECK(noSubtitlePlan.commands[i].priority == 5u);
        CHECK(noSubtitlePlan.commands[i].rawTextureKnown);
        CHECK(noSubtitlePlan.commands[i].rawTexture);
        CHECK(noSubtitlePlan.commands[i].semiTransparentKnown);
        CHECK(noSubtitlePlan.commands[i].semiTransparent ==
              expectedSemiTransparent);
        CHECK(noSubtitlePlan.commands[i].abrKnown);
        CHECK(noSubtitlePlan.commands[i].abr == 1u);
        CHECK(!noSubtitlePlan.commands[i].rgbKnown);
        if (expectedSemiTransparent) {
            CHECK(noSubtitlePlan.commands[i].spriteTemplate.sourceAddress ==
                  0x8004EAA0u +
                      static_cast<uint32_t>(i - 84u) * 0x10u);
            ++semiTransparentCount;
        }
    }
    CHECK(semiTransparentCount == 8u);

    PrSS0TransitionDirect::FastTransitionVisualFrame800201AC
        unresolvedVisual{};
    unresolvedVisual.kind =
        FastTransitionFrameKind800201AC::NoSubtitleFrame8001CE30;
    const auto unresolvedPlan =
        PrSS0TransitionDirect::BuildFastTransitionFramePlan800201AC(
            unresolvedVisual);
    CHECK(!unresolvedPlan.known);
    CHECK(unresolvedPlan.commandCount == 0u);

    auto invalidVisual = noSubtitleVisual;
    invalidVisual.kind = FastTransitionFrameKind800201AC::Unknown;
    const auto invalidPlan =
        PrSS0TransitionDirect::BuildFastTransitionFramePlan800201AC(
            invalidVisual);
    CHECK(!invalidPlan.known);
    CHECK(invalidPlan.commandCount == 0u);
    CHECK(!PrSS0TransitionDirect::
               ResolveFastTransitionSpriteTemplate800201AC(0x8004EB20u)
               .known);

    PrSS0TransitionDirect::FastTransitionGraphInput8001EA74 graphInput{};
    graphInput.known = true;
    graphInput.drawSlot8004019C = 0u;
    graphInput.packetAllocator8006ED50 = 0x801AE430u;
    graphInput.mainPageWorkAddress80087288 = 0x80087288u;
    graphInput.mainPageOtHeadAddress80088288 = 0x80088288u;
    graphInput.mainPageWorkHeadAddress80040CC8 = 0x80088288u;
    graphInput.mainPageWorkOrder = 14u;
    const auto presentPlan =
        PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
            finalVisual, graphInput);
    CHECK(presentPlan.known);
    CHECK(!presentPlan.truncated);
    CHECK(presentPlan.drawSlotBefore == 0u);
    CHECK(presentPlan.drawSlotAfter == 1u);
    CHECK(presentPlan.actionCount == 7u);
    CHECK(presentPlan.actions[0].kind ==
          PrSS0TransitionDirect::FastTransitionPresentActionKind8001EBF4::
              GetDrawBuffer8004019C);
    CHECK(presentPlan.actions[4].kind ==
          PrSS0TransitionDirect::FastTransitionPresentActionKind8001EBF4::
              FlipGraph80040370);
    CHECK(presentPlan.actions[5].kind ==
          PrSS0TransitionDirect::FastTransitionPresentActionKind8001EBF4::
              ClearColor80040420);
    CHECK(presentPlan.actions[6].kind ==
          PrSS0TransitionDirect::FastTransitionPresentActionKind8001EBF4::
              SubmitMainPageWork80040CA4);

    const auto noClearPresentPlan =
        PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
            PrSS0TransitionDirect::ResolveFastTransitionVisualFrame800201AC(
                5, false, false, 0u),
            graphInput);
    CHECK(noClearPresentPlan.known);
    CHECK(noClearPresentPlan.actionCount == 6u);
    CHECK(!noClearPresentPlan.clearColorRequested80040420);
    CHECK(noClearPresentPlan.actions[5].kind ==
          PrSS0TransitionDirect::FastTransitionPresentActionKind8001EBF4::
              SubmitMainPageWork80040CA4);

    graphInput.drawSlot8004019C = 1u;
    graphInput.packetAllocator8006ED50 = 0x801B8CF0u;
    graphInput.mainPageWorkAddress80087288 = 0x8008729Cu;
    graphInput.mainPageOtHeadAddress80088288 = 0x80098288u;
    graphInput.mainPageWorkHeadAddress80040CC8 = 0x80098288u;
    const auto slotOnePresentPlan =
        PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
            finalVisual, graphInput);
    CHECK(slotOnePresentPlan.known);
    CHECK(slotOnePresentPlan.drawSlotBefore == 1u);
    CHECK(slotOnePresentPlan.drawSlotAfter == 0u);

    graphInput.packetAllocator8006ED50 = 0x801B54B0u;
    CHECK(!PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
               finalVisual, graphInput)
               .known);
    graphInput.packetAllocator8006ED50 = 0x801B8CF0u;
    graphInput.mainPageWorkHeadAddress80040CC8 = 0x80088288u;
    CHECK(!PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
               finalVisual, graphInput)
               .known);
    graphInput.mainPageWorkHeadAddress80040CC8 = 0x80098288u;
    graphInput.packetAllocator8006ED50 = 0u;
    CHECK(!PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
               finalVisual, graphInput)
               .known);

    // The resident directory uses 8001E34C's lanes, never the title arena.
    for (uint16_t slot = 0; slot < 2; ++slot) {
        auto resident = graphInput;
        resident.residentMainPacketLanes8001E34C = true;
        resident.drawSlot8004019C = slot;
        resident.packetAllocator8006ED50 = slot ? 0x80083FC0u : 0x80080CF8u;
        resident.mainPageWorkAddress80087288 = slot ? 0x8008729Cu : 0x80087288u;
        resident.mainPageOtHeadAddress80088288 = slot ? 0x80098288u : 0x80088288u;
        resident.mainPageWorkHeadAddress80040CC8 = resident.mainPageOtHeadAddress80088288;
        const auto plan = PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
            finalVisual, resident);
        CHECK(plan.known && plan.drawSlotBefore == slot && plan.drawSlotAfter == (slot ^ 1u));
        CHECK(plan.packetAllocator8006ED50 == resident.packetAllocator8006ED50);
        CHECK(plan.actions[1].arg0 == resident.packetAllocator8006ED50);
        resident.residentMainPacketLanes8001E34C = false;
        CHECK(!PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
            finalVisual, resident).known);
        resident.residentMainPacketLanes8001E34C = true;
        resident.packetAllocator8006ED50 = slot ? 0x801B8CF0u : 0x801AE430u;
        CHECK(!PrSS0TransitionDirect::BuildFastTransitionPresentPlan8001EBF4(
            finalVisual, resident).known);
    }
}

void TestScene0SlowTransitionMode1Cadence80020110() {
    using PrSS0TransitionDirect::FastTransitionPresentActionKind8001EBF4;
    using PrSS0TransitionDirect::SlowTransitionGridCoordinate8004EB80;
    using PrSS0TransitionDirect::SlowTransitionRuntime80020110;
    using PrSS0TransitionDirect::SlowTransitionRuntimePhase80020110;

    CHECK(PrSS0TransitionDirect::HasKnownModeBodyTicks8001EA74(1, false));
    CHECK(PrSS0TransitionDirect::KnownModeBodyTicks8001EA74(1, false) ==
          24);

    struct SpiralCheckpoint {
        std::size_t index;
        uint8_t row;
        uint8_t column;
    };
    static constexpr SpiralCheckpoint kSpiralCheckpoints[] = {
        {0u, 0u, 0u},
        {15u, 0u, 15u},
        {26u, 11u, 15u},
        {41u, 11u, 0u},
        {51u, 1u, 0u},
        {52u, 1u, 1u},
        {65u, 1u, 14u},
        {74u, 10u, 14u},
        {87u, 10u, 1u},
        {95u, 2u, 1u},
        {96u, 2u, 2u},
        {107u, 2u, 13u},
        {114u, 9u, 13u},
        {125u, 9u, 2u},
        {131u, 3u, 2u},
        {132u, 3u, 3u},
        {141u, 3u, 12u},
        {146u, 8u, 12u},
        {155u, 8u, 3u},
        {159u, 4u, 3u},
        {160u, 4u, 4u},
        {167u, 4u, 11u},
        {170u, 7u, 11u},
        {177u, 7u, 4u},
        {179u, 5u, 4u},
        {180u, 5u, 5u},
        {181u, 5u, 6u},
        {182u, 5u, 7u},
        {183u, 5u, 8u},
        {184u, 5u, 9u},
        {185u, 5u, 10u},
        {186u, 6u, 10u},
        {187u, 6u, 9u},
        {188u, 6u, 8u},
        {189u, 6u, 7u},
        {190u, 6u, 6u},
        {191u, 6u, 5u},
    };

    bool seen[PrSS0TransitionDirect::kSlowTransitionGridCells8001FDC0]{};
    SlowTransitionGridCoordinate8004EB80 previousCoordinate{};
    for (std::size_t orderIndex = 0u;
         orderIndex <
             PrSS0TransitionDirect::kSlowTransitionGridCells8001FDC0;
         ++orderIndex) {
        const SlowTransitionGridCoordinate8004EB80 coordinate =
            PrSS0TransitionDirect::
                ResolveSlowTransitionMode1SpiralCoordinate8004EB80(
                    orderIndex);
        CHECK(coordinate.known);
        CHECK(coordinate.row <
              PrSS0TransitionDirect::kSlowTransitionGridRows8001FDC0);
        CHECK(coordinate.column <
              PrSS0TransitionDirect::kSlowTransitionGridColumns8001FDC0);
        const std::size_t gridIndex =
            static_cast<std::size_t>(coordinate.row) *
                PrSS0TransitionDirect::kSlowTransitionGridColumns8001FDC0 +
            coordinate.column;
        CHECK(gridIndex <
              PrSS0TransitionDirect::kSlowTransitionGridCells8001FDC0);
        if (gridIndex <
            PrSS0TransitionDirect::kSlowTransitionGridCells8001FDC0) {
            CHECK(!seen[gridIndex]);
            seen[gridIndex] = true;
        }
        if (orderIndex != 0u) {
            const uint32_t rowDelta =
                coordinate.row > previousCoordinate.row
                    ? coordinate.row - previousCoordinate.row
                    : previousCoordinate.row - coordinate.row;
            const uint32_t columnDelta =
                coordinate.column > previousCoordinate.column
                    ? coordinate.column - previousCoordinate.column
                    : previousCoordinate.column - coordinate.column;
            CHECK(rowDelta + columnDelta == 1u);
        }
        previousCoordinate = coordinate;
    }
    for (bool cellSeen : seen) {
        CHECK(cellSeen);
    }
    for (const SpiralCheckpoint& checkpoint : kSpiralCheckpoints) {
        const SlowTransitionGridCoordinate8004EB80 coordinate =
            PrSS0TransitionDirect::
                ResolveSlowTransitionMode1SpiralCoordinate8004EB80(
                    checkpoint.index);
        CHECK(coordinate.known);
        CHECK(coordinate.row == checkpoint.row);
        CHECK(coordinate.column == checkpoint.column);
    }
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionMode1SpiralCoordinate8004EB80(192u)
               .known);

    SlowTransitionRuntime80020110 runtime{};
    CHECK(!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime, 0x801C0000u, 1, 2, 1));
    CHECK(!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        1,
        1,
        2));
    CHECK(!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        1,
        2,
        2));
    CHECK(!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        2,
        2,
        1));

    CHECK(PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        1,
        2,
        1));
    CHECK(runtime.active);
    CHECK(runtime.phase ==
          SlowTransitionRuntimePhase80020110::Loop8001EA74);
    CHECK(runtime.loopIterationsRequired == 24u);
    CHECK(runtime.sceneFrame ==
          PrSS0TransitionDirect::
              kScene0TitleIntroTransitionStartFrame6815);
    CHECK(runtime.tileMaskKnown);
    CHECK(!runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 0u);
    CHECK(runtime.tileMaskMutationSerial == 1u);
    for (uint8_t cell : runtime.tileMask) {
        CHECK(cell == 1u);
    }

    PrSS0TransitionDirect::FastTransitionGraphInput8001EA74 graphInput{};
    graphInput.known = true;
    graphInput.drawSlot8004019C = 0u;
    graphInput.packetAllocator8006ED50 = 0x801AE430u;
    graphInput.mainPageWorkAddress80087288 = 0x80087288u;
    graphInput.mainPageOtHeadAddress80088288 = 0x80088288u;
    graphInput.mainPageWorkHeadAddress80040CC8 = 0x80088288u;
    graphInput.mainPageWorkOrder = 14u;

    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110 firstVisual{};
    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110 previousVisual{};
    uint32_t acceptedTicks = 0u;
    uint32_t presentRequiredTicks = 0u;
    uint32_t builtPresentPlans = 0u;
    uint32_t completedMainIterations = 0u;
    uint32_t completedTailIterations = 0u;
    PrSS0TransitionDirect::SlowTransitionTickResult80020110 tick{};
    for (uint32_t hostTick = 1u; hostTick <= 56u; ++hostTick) {
        tick = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(
            runtime);
        CHECK(tick.accepted);
        if (tick.accepted) {
            ++acceptedTicks;
        }

        const bool expectedTail = hostTick > 48u;
        const uint32_t expectedIteration = expectedTail
            ? (hostTick - 49u) / 2u
            : (hostTick - 1u) / 2u;
        const uint32_t expectedActiveCount = expectedTail
            ? 0u
            : 192u - (expectedIteration + 1u) * 8u;
        const std::size_t expectedClearCount =
            expectedTail ? 192u : (expectedIteration + 1u) * 8u;
        CHECK(tick.visualFrame.known);
        CHECK(tick.visualFrame.tail == expectedTail);
        CHECK(tick.visualFrame.ctxAddress ==
              PrSS0TransitionDirect::kScene0WorkAddress);
        CHECK(tick.visualFrame.mode == 1);
        CHECK(tick.visualFrame.preFfd4Arg == 2);
        CHECK(tick.visualFrame.postFfd4Arg == 1);
        CHECK(tick.visualFrame.iteration == expectedIteration);
        CHECK(tick.visualFrame.activeCount == expectedActiveCount);
        CHECK(tick.visualFrame.sourceFunction ==
              PrSS0TransitionDirect::kFn8001F524);
        CHECK(tick.hostTicksCompleted == hostTick);
        CHECK(tick.sceneFrame ==
              PrSS0TransitionDirect::
                      kScene0TitleIntroTransitionStartFrame6815 +
                  hostTick);
        CHECK(tick.presentRequired == ((hostTick & 1u) == 0u));
        CHECK(tick.loopIterationCompleted ==
              (!expectedTail && (hostTick & 1u) == 0u));
        CHECK(tick.tailIterationCompleted ==
              (expectedTail && (hostTick & 1u) == 0u));
        CHECK(tick.complete == (hostTick == 56u));
        const bool expectedMaskMutation =
            !expectedTail ? ((hostTick & 1u) != 0u) : hostTick == 49u;
        CHECK(tick.tileMaskMutationApplied == expectedMaskMutation);
        CHECK(tick.tileMaskMutationSerial ==
              (!expectedTail
                   ? 1u + (hostTick + 1u) / 2u
                   : 26u));

        if (tick.loopIterationCompleted) {
            ++completedMainIterations;
        }
        if (tick.tailIterationCompleted) {
            ++completedTailIterations;
        }

        uint32_t gridActiveCount = 0u;
        for (std::size_t orderIndex = 0u;
             orderIndex <
                 PrSS0TransitionDirect::kSlowTransitionGridCells8001FDC0;
             ++orderIndex) {
            const SlowTransitionGridCoordinate8004EB80 coordinate =
                PrSS0TransitionDirect::
                    ResolveSlowTransitionMode1SpiralCoordinate8004EB80(
                        orderIndex);
            CHECK(coordinate.known);
            const std::size_t gridIndex =
                static_cast<std::size_t>(coordinate.row) *
                    PrSS0TransitionDirect::
                        kSlowTransitionGridColumns8001FDC0 +
                coordinate.column;
            const uint8_t expectedCell =
                orderIndex < expectedClearCount ? 0u : 1u;
            CHECK(tick.visualFrame.grid[gridIndex] == expectedCell);
            gridActiveCount += tick.visualFrame.grid[gridIndex];
        }
        CHECK(gridActiveCount == expectedActiveCount);

        const auto framePlan =
            PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(
                tick.visualFrame);
        CHECK(framePlan.known);
        CHECK(!framePlan.truncated);
        CHECK(framePlan.tail == expectedTail);
        CHECK(framePlan.iteration == expectedIteration);
        CHECK(framePlan.activeCount == expectedActiveCount);
        CHECK(framePlan.commandCount == expectedActiveCount);

        if (tick.presentRequired) {
            ++presentRequiredTicks;
            const auto presentPlan =
                PrSS0TransitionDirect::
                    BuildSlowTransitionPresentPlan8001EBF4(
                        tick.visualFrame, graphInput);
            CHECK(presentPlan.known);
            CHECK(!presentPlan.truncated);
            CHECK(presentPlan.actionCount == 6u);
            CHECK(presentPlan.actions[3].kind ==
                  FastTransitionPresentActionKind8001EBF4::
                      BuildVisualFrame8001EA74);
            CHECK(presentPlan.actions[3].psxFunction ==
                  PrSS0TransitionDirect::kFn8001F524);
            ++builtPresentPlans;
        }

        if ((hostTick & 1u) != 0u) {
            previousVisual = tick.visualFrame;
        } else {
            CHECK(previousVisual.tail == tick.visualFrame.tail);
            CHECK(previousVisual.iteration == tick.visualFrame.iteration);
            CHECK(previousVisual.activeCount ==
                  tick.visualFrame.activeCount);
            for (std::size_t i = 0u;
                 i < PrSS0TransitionDirect::
                         kSlowTransitionGridCells8001FDC0;
                 ++i) {
                CHECK(previousVisual.grid[i] == tick.visualFrame.grid[i]);
            }
        }

        if (hostTick == 1u) {
            firstVisual = tick.visualFrame;
            CHECK(framePlan.commandCount == 184u);
            CHECK(framePlan.commands[0].x == 160);
            CHECK(framePlan.commands[0].y == 0);
        } else if (hostTick == 48u) {
            CHECK(tick.sceneFrame ==
                  PrSS0TransitionDirect::
                      kScene0TitleIntroTransitionTailFrame6863);
            CHECK(tick.phaseAfter ==
                  SlowTransitionRuntimePhase80020110::Tail80020008);
            CHECK(framePlan.commandCount == 0u);
        } else if (hostTick == 49u) {
            CHECK(tick.visualFrame.tail);
            CHECK(framePlan.commandCount == 0u);
        }
    }

    CHECK(acceptedTicks == 56u);
    CHECK(presentRequiredTicks == 28u);
    CHECK(builtPresentPlans == 28u);
    CHECK(completedMainIterations == 24u);
    CHECK(completedTailIterations == 4u);
    CHECK(!runtime.active);
    CHECK(runtime.phase == SlowTransitionRuntimePhase80020110::Complete);
    CHECK(runtime.loopIterationsCompleted == 24u);
    CHECK(runtime.tailIterationsCompleted == 4u);
    CHECK(runtime.hostTicksCompleted == 56u);
    CHECK(runtime.tileMaskKnown);
    CHECK(runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 0u);
    CHECK(runtime.tileMaskMutationSerial == 26u);
    for (uint8_t cell : runtime.tileMask) {
        CHECK(cell == 0u);
    }
    CHECK(runtime.sceneFrame ==
          PrSS0TransitionDirect::kScene0TitleIntroTransitionEndFrame6871);
    CHECK(tick.sceneFrame ==
          PrSS0TransitionDirect::kScene0TitleIntroTransitionEndFrame6871);

    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   0x801C0000u, 1, 2, 1, false, 0u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   1,
                   1,
                   2,
                   false,
                   0u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   2,
                   2,
                   1,
                   false,
                   0u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   1,
                   2,
                   1,
                   false,
                   24u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   1,
                   2,
                   1,
                   true,
                   4u)
               .known);

    auto badVisual = firstVisual;
    badVisual.sourceFunction = PrSS0TransitionDirect::kFn8001FDC0;
    CHECK(!PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(
               badVisual)
               .known);
    CHECK(!PrSS0TransitionDirect::
               BuildSlowTransitionPresentPlan8001EBF4(
                   badVisual, graphInput)
               .known);
    badVisual = firstVisual;
    badVisual.grid[8] = 0u;
    CHECK(!PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(
               badVisual)
               .known);

    CHECK(PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        1,
        2,
        1));
    runtime.postFfd4Arg = 2;
    tick = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(runtime);
    CHECK(!tick.accepted);
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(runtime);
}

void TestScene0SlowTransitionCadence80020110() {
    using PrSS0TransitionDirect::FastTransitionPresentActionKind8001EBF4;
    using PrSS0TransitionDirect::SlowTransitionRuntime80020110;
    using PrSS0TransitionDirect::SlowTransitionRuntimePhase80020110;

    CHECK(PrSS0TransitionDirect::HasKnownModeBodyTicks8001EA74(2, false));
    CHECK(PrSS0TransitionDirect::KnownModeBodyTicks8001EA74(2, false) ==
          24);

    SlowTransitionRuntime80020110 runtime{};
    CHECK(!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime, 0x801C0000u, 2, 1, 2));
    CHECK(!runtime.active);
    CHECK(!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        5,
        1,
        2));
    CHECK(!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        2,
        2,
        2));
    CHECK(!PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        2,
        1,
        1));

    CHECK(PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        2,
        1,
        2));
    CHECK(runtime.active);
    CHECK(runtime.phase ==
          SlowTransitionRuntimePhase80020110::Loop8001EA74);
    CHECK(runtime.loopIterationsRequired == 24u);
    CHECK(runtime.sceneFrame ==
          PrSS0TransitionDirect::kScene0TitleExitStartFrame7854);
    CHECK(runtime.tileMaskKnown);
    CHECK(!runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 0u);
    CHECK(runtime.tileMaskMutationSerial == 1u);
    for (uint8_t cell : runtime.tileMask) {
        CHECK(cell == 0u);
    }

    PrSS0TransitionDirect::SlowTransitionFramePlan8001FDC0 firstPlan{};
    PrSS0TransitionDirect::SlowTransitionFramePlan8001FDC0 lastMainPlan{};
    PrSS0TransitionDirect::SlowTransitionFramePlan8001FDC0 tailPlan{};
    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110 tailVisual{};
    PrSS0TransitionDirect::SlowTransitionTickResult80020110 tick{};
    uint32_t acceptedTicks = 0u;
    uint32_t presentRequiredTicks = 0u;
    uint32_t completedMainIterations = 0u;
    uint32_t completedTailIterations = 0u;
    for (uint32_t hostTick = 1u; hostTick <= 56u; ++hostTick) {
        tick = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(
            runtime);
        CHECK(tick.accepted);
        if (tick.accepted) {
            ++acceptedTicks;
        }
        const bool expectedTail = hostTick > 48u;
        const uint32_t expectedIteration = expectedTail
            ? (hostTick - 49u) / 2u
            : (hostTick - 1u) / 2u;
        const uint32_t expectedActiveCount = expectedTail
            ? 192u
            : (expectedIteration + 1u) * 8u;
        CHECK(tick.visualFrame.known);
        CHECK(tick.visualFrame.tail == expectedTail);
        CHECK(tick.visualFrame.ctxAddress ==
              PrSS0TransitionDirect::kScene0WorkAddress);
        CHECK(tick.visualFrame.mode == 2);
        CHECK(tick.visualFrame.preFfd4Arg == 1);
        CHECK(tick.visualFrame.postFfd4Arg == 2);
        CHECK(tick.visualFrame.iteration == expectedIteration);
        CHECK(tick.visualFrame.activeCount == expectedActiveCount);
        CHECK(tick.visualFrame.sourceFunction ==
              PrSS0TransitionDirect::kFn8001FDC0);
        CHECK(tick.hostTicksCompleted == hostTick);
        CHECK(tick.presentRequired == ((hostTick & 1u) == 0u));
        if (tick.presentRequired) {
            ++presentRequiredTicks;
        }
        CHECK(tick.sceneFrame ==
              PrSS0TransitionDirect::kScene0TitleExitStartFrame7854 +
                  hostTick);
        CHECK(tick.loopIterationCompleted ==
              (!expectedTail && (hostTick & 1u) == 0u));
        CHECK(tick.tailIterationCompleted ==
              (expectedTail && (hostTick & 1u) == 0u));
        if (tick.loopIterationCompleted) {
            ++completedMainIterations;
        }
        if (tick.tailIterationCompleted) {
            ++completedTailIterations;
        }

        uint32_t gridActiveCount = 0u;
        for (std::size_t i = 0;
             i < PrSS0TransitionDirect::kSlowTransitionGridCells8001FDC0;
             ++i) {
            CHECK(tick.visualFrame.grid[i] <= 1u);
            gridActiveCount += tick.visualFrame.grid[i];
        }
        CHECK(gridActiveCount == expectedActiveCount);

        const auto plan =
            PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(
                tick.visualFrame);
        CHECK(plan.known);
        CHECK(!plan.truncated);
        CHECK(plan.tail == expectedTail);
        CHECK(plan.iteration == expectedIteration);
        CHECK(plan.activeCount == expectedActiveCount);
        CHECK(plan.commandCount == expectedActiveCount);
        std::size_t previousGridIndex = 0u;
        for (std::size_t i = 0; i < plan.commandCount; ++i) {
            const auto& command = plan.commands[i];
            CHECK(command.active);
            CHECK(command.rawTextureKnown);
            CHECK(command.rawTexture);
            CHECK(command.semiTransparentKnown);
            CHECK(!command.semiTransparent);
            CHECK(command.abrKnown);
            CHECK(command.abr == 1u);
            CHECK(!command.rgbKnown);
            CHECK(command.priority == 0u);
            CHECK(command.order == i + 1u);
            CHECK(command.x >= 0 && command.x <= 300);
            CHECK(command.y >= 0 && command.y <= 220);
            CHECK((command.x % 20) == 0);
            CHECK((command.y % 20) == 0);
            const std::size_t gridIndex =
                static_cast<std::size_t>(command.y / 20) * 16u +
                static_cast<std::size_t>(command.x / 20);
            CHECK(tick.visualFrame.grid[gridIndex] == 1u);
            CHECK(i == 0u || gridIndex > previousGridIndex);
            previousGridIndex = gridIndex;
        }

        if (hostTick == 1u) {
            firstPlan = plan;
        } else if (hostTick == 48u) {
            lastMainPlan = plan;
            CHECK(tick.sceneFrame ==
                  PrSS0TransitionDirect::kScene0TitleExitFadeTailFrame7902);
            CHECK(tick.phaseAfter ==
                  SlowTransitionRuntimePhase80020110::Tail80020008);
        } else if (hostTick == 49u) {
            tailPlan = plan;
            tailVisual = tick.visualFrame;
        }
        CHECK(tick.complete == (hostTick == 56u));
        const bool expectedMaskMutation =
            !expectedTail ? ((hostTick & 1u) != 0u) : hostTick == 49u;
        CHECK(tick.tileMaskMutationApplied == expectedMaskMutation);
        CHECK(tick.tileMaskMutationSerial ==
              (!expectedTail
                   ? 1u + (hostTick + 1u) / 2u
                   : 26u));
    }

    CHECK(acceptedTicks == 56u);
    CHECK(presentRequiredTicks == 28u);
    CHECK(completedMainIterations == 24u);
    CHECK(completedTailIterations == 4u);
    CHECK(!runtime.active);
    CHECK(runtime.phase ==
          SlowTransitionRuntimePhase80020110::Complete);
    CHECK(runtime.loopIterationsCompleted == 24u);
    CHECK(runtime.tailIterationsCompleted == 4u);
    CHECK(runtime.hostTicksCompleted == 56u);
    CHECK(runtime.sceneFrame ==
          PrSS0TransitionDirect::kScene0TitleExitSceneTailFrame7910);
    CHECK(runtime.tileMaskKnown);
    CHECK(runtime.postFfd4Applied);
    CHECK(runtime.gp196 == 0u);
    CHECK(runtime.tileMaskMutationSerial == 26u);
    for (uint8_t cell : runtime.tileMask) {
        CHECK(cell == 1u);
    }
    CHECK(tick.sceneFrame ==
          PrSS0TransitionDirect::kScene0TitleExitSceneTailFrame7910);

    CHECK(firstPlan.known);
    CHECK(firstPlan.commandCount == 8u);
    CHECK(firstPlan.commands[0].x == 120);
    CHECK(firstPlan.commands[0].y == 100);
    CHECK(firstPlan.commands[7].x == 200);
    CHECK(firstPlan.commands[7].y == 120);
    CHECK(lastMainPlan.known);
    CHECK(lastMainPlan.commandCount == 192u);
    CHECK(!lastMainPlan.tail);
    CHECK(tailPlan.known);
    CHECK(tailPlan.commandCount == 192u);
    CHECK(tailPlan.tail);

    static constexpr uint32_t kExpectedGridTemplates[4] = {
        0x800503E0u, 0x800503F0u, 0x80050400u, 0x80050410u};
    for (std::size_t row = 0; row < 12u; ++row) {
        for (std::size_t col = 0; col < 16u; ++col) {
            const std::size_t index = row * 16u + col;
            const auto& command = tailPlan.commands[index];
            CHECK(tailVisual.grid[index] == 1u);
            CHECK(command.x == static_cast<int16_t>(col * 20u));
            CHECK(command.y == static_cast<int16_t>(row * 20u));
            CHECK(command.spriteTemplate.sourceAddress ==
                  kExpectedGridTemplates[(row & 1u) * 2u + (col & 1u)]);
        }
    }

    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   0x801C0000u, 2, 1, 2, false, 0u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   5,
                   1,
                   2,
                   false,
                   0u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   2,
                   2,
                   2,
                   false,
                   0u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   2,
                   1,
                   1,
                   false,
                   0u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   2,
                   1,
                   2,
                   false,
                   24u)
               .known);
    CHECK(!PrSS0TransitionDirect::
               ResolveSlowTransitionVisualFrame80020110(
                   PrSS0TransitionDirect::kScene0WorkAddress,
                   2,
                   1,
                   2,
                   true,
                   4u)
               .known);

    PrSS0TransitionDirect::SlowTransitionVisualFrame80020110 unknownVisual{};
    CHECK(!PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(
               unknownVisual)
               .known);
    auto badVisual = tailVisual;
    badVisual.activeCount = 191u;
    CHECK(!PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(
               badVisual)
               .known);
    badVisual = tailVisual;
    badVisual.sourceFunction = PrSS0TransitionDirect::kFn8001EA74;
    CHECK(!PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(
               badVisual)
               .known);
    badVisual = tailVisual;
    badVisual.grid[0] = 0u;
    CHECK(!PrSS0TransitionDirect::BuildSlowTransitionFramePlan8001FDC0(
               badVisual)
               .known);

    PrSS0TransitionDirect::FastTransitionGraphInput8001EA74 graphInput{};
    graphInput.known = true;
    graphInput.drawSlot8004019C = 0u;
    graphInput.packetAllocator8006ED50 = 0x801AE430u;
    graphInput.mainPageWorkAddress80087288 = 0x80087288u;
    graphInput.mainPageOtHeadAddress80088288 = 0x80088288u;
    graphInput.mainPageWorkHeadAddress80040CC8 = 0x80088288u;
    graphInput.mainPageWorkOrder = 14u;
    const auto presentPlan =
        PrSS0TransitionDirect::BuildSlowTransitionPresentPlan8001EBF4(
            tailVisual, graphInput);
    CHECK(presentPlan.known);
    CHECK(!presentPlan.truncated);
    CHECK(presentPlan.actionCount == 6u);
    CHECK(!presentPlan.clearColorRequested80040420);
    CHECK(presentPlan.mainPageWorkSubmitRequested80040CA4);
    static constexpr FastTransitionPresentActionKind8001EBF4
        kExpectedPresentActions[6] = {
            FastTransitionPresentActionKind8001EBF4::
                GetDrawBuffer8004019C,
            FastTransitionPresentActionKind8001EBF4::
                SetPacketAllocator80040F90,
            FastTransitionPresentActionKind8001EBF4::
                ClearMainPageWork80040CC8,
            FastTransitionPresentActionKind8001EBF4::
                BuildVisualFrame8001EA74,
            FastTransitionPresentActionKind8001EBF4::FlipGraph80040370,
            FastTransitionPresentActionKind8001EBF4::
                SubmitMainPageWork80040CA4,
        };
    for (std::size_t i = 0; i < 6u; ++i) {
        CHECK(presentPlan.actions[i].kind == kExpectedPresentActions[i]);
        CHECK(presentPlan.actions[i].kind !=
              FastTransitionPresentActionKind8001EBF4::
                  ClearColor80040420);
    }
    CHECK(presentPlan.actions[3].psxFunction ==
          PrSS0TransitionDirect::kFn8001FDC0);

    graphInput.drawSlot8004019C = 1u;
    graphInput.packetAllocator8006ED50 = 0x801B8CF0u;
    graphInput.mainPageWorkAddress80087288 = 0x8008729Cu;
    graphInput.mainPageOtHeadAddress80088288 = 0x80098288u;
    graphInput.mainPageWorkHeadAddress80040CC8 = 0x80098288u;
    const auto reboundSlotOnePresentPlan =
        PrSS0TransitionDirect::BuildSlowTransitionPresentPlan8001EBF4(
            tailVisual, graphInput);
    CHECK(reboundSlotOnePresentPlan.known);
    CHECK(reboundSlotOnePresentPlan.drawSlotBefore == 1u);
    CHECK(reboundSlotOnePresentPlan.drawSlotAfter == 0u);
    CHECK(reboundSlotOnePresentPlan.packetAllocator8006ED50 ==
          0x801B8CF0u);

    const auto residentVisual = PrSS0TransitionDirect::ResolveSlowTransitionVisualFrame80020110(
        0u, 4, 2, 1, false, 0u);
    CHECK(residentVisual.known);
    for (uint16_t slot = 0; slot < 2; ++slot) {
        auto resident = graphInput;
        resident.residentMainPacketLanes8001E34C = true;
        resident.drawSlot8004019C = slot;
        resident.packetAllocator8006ED50 = slot ? 0x80083FC0u : 0x80080CF8u;
        resident.mainPageWorkAddress80087288 = slot ? 0x8008729Cu : 0x80087288u;
        resident.mainPageOtHeadAddress80088288 = slot ? 0x80098288u : 0x80088288u;
        resident.mainPageWorkHeadAddress80040CC8 = resident.mainPageOtHeadAddress80088288;
        const auto plan = PrSS0TransitionDirect::BuildSlowTransitionPresentPlan8001EBF4(
            residentVisual, resident);
        CHECK(plan.known && plan.drawSlotBefore == slot && plan.drawSlotAfter == (slot ^ 1u));
        CHECK(plan.packetAllocator8006ED50 == resident.packetAllocator8006ED50);
        CHECK(plan.actions[1].arg0 == resident.packetAllocator8006ED50);
        resident.residentMainPacketLanes8001E34C = false;
        CHECK(!PrSS0TransitionDirect::BuildSlowTransitionPresentPlan8001EBF4(
            residentVisual, resident).known);
        resident.residentMainPacketLanes8001E34C = true;
        resident.packetAllocator8006ED50 = slot ? 0x801B8CF0u : 0x801AE430u;
        CHECK(!PrSS0TransitionDirect::BuildSlowTransitionPresentPlan8001EBF4(
            residentVisual, resident).known);
    }

    graphInput.drawSlot8004019C = 0u;
    graphInput.packetAllocator8006ED50 = 0x801A73B0u;
    graphInput.mainPageWorkAddress80087288 = 0x80087288u;
    graphInput.mainPageOtHeadAddress80088288 = 0x80088288u;
    graphInput.mainPageWorkHeadAddress80040CC8 = 0x80088288u;
    CHECK(!PrSS0TransitionDirect::
               BuildSlowTransitionPresentPlan8001EBF4(
                   tailVisual, graphInput)
               .known);
    graphInput.packetAllocator8006ED50 = 0x801AE430u;

    graphInput.known = false;
    CHECK(!PrSS0TransitionDirect::
               BuildSlowTransitionPresentPlan8001EBF4(
                   tailVisual, graphInput)
               .known);
    graphInput.known = true;
    graphInput.mainPageWorkOrder = 13u;
    CHECK(!PrSS0TransitionDirect::
               BuildSlowTransitionPresentPlan8001EBF4(
                   tailVisual, graphInput)
               .known);
    graphInput.mainPageWorkOrder = 14u;
    CHECK(!PrSS0TransitionDirect::
               BuildSlowTransitionPresentPlan8001EBF4(
                   badVisual, graphInput)
               .known);

    CHECK(PrSS0TransitionDirect::BeginSlowTransitionRuntime80020110(
        runtime,
        PrSS0TransitionDirect::kScene0WorkAddress,
        2,
        1,
        2));
    runtime.mode = 5;
    tick = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(
        runtime);
    CHECK(!tick.accepted);
    PrSS0TransitionDirect::ResetSlowTransitionRuntime80020110(runtime);
    tick = PrSS0TransitionDirect::TickSlowTransitionRuntime80020110(
        runtime);
    CHECK(!tick.accepted);
    CHECK(!tick.complete);
}

void TestScene0LoadingPatternRuntime8001EF40() {
    using namespace PrSS0TransitionDirect;

    CHECK(ResolveLoadingPatternStyle8001EF40(0) == 0u);
    CHECK(ResolveLoadingPatternStyle8001EF40(2) == 1u);
    CHECK(ResolveLoadingPatternStyle8001EF40(3) == 2u);
    CHECK(ResolveLoadingPatternStyle8001EF40(4) == 3u);
    CHECK(ResolveLoadingPatternStyle8001EF40(5) == 0u);

    std::array<uint8_t, kLoadingPatternGridCells8001EF40> completed{};
    completed.fill(1u);
    LoadingPatternRuntime8001EF40 runtime{};
    CHECK(!BeginLoadingPatternRuntimeAfter8001FFD4(
        runtime, 0, nullptr, completed.size()));
    CHECK(!BeginLoadingPatternRuntimeAfter8001FFD4(
        runtime, 0, completed.data(), completed.size() - 1u));
    CHECK(BeginLoadingPatternRuntimeAfter8001FFD4(
        runtime, 0, completed.data(), completed.size()));
    CHECK(runtime.countersKnown);
    CHECK(runtime.active);
    CHECK(runtime.style == 0u);
    CHECK(runtime.bitCursorGp49 == 0u);
    CHECK(runtime.wordCursorGp50 == 0u);
    CHECK(runtime.frameCounterGp51 == 0u);

    const auto first = TickLoadingPatternRuntime8001EF40(runtime);
    CHECK(first.known);
    CHECK(first.mutationApplied);
    CHECK(first.sourceFunction == kFn8001EF40);
    CHECK(first.drawHighlightFunction == kFn8001C4EC);
    CHECK(first.drawTileFunction == kFn8001C550);
    CHECK(first.boxFillAttr8001B6C4 == 0x401C0B5Au);
    CHECK(first.boxFillGpuColorCode8003EE84 == 0x625A0B1Cu);
    CHECK(first.bitCursorGp49 == 1u);
    CHECK(first.wordCursorGp50 == 0u);
    CHECK(first.frameCounterGp51 == 1u);
    CHECK(first.mutationSerial == 1u);
    CHECK(first.highlightCount == 187u);
    CHECK(first.liveGrid[0u * 16u + 15u] == 0u);
    CHECK(first.liveGrid[2u * 16u + 15u] == 1u);
    CHECK(first.liveGrid[8u * 16u + 15u] == 1u);
    CHECK(first.liveGrid[9u * 16u + 15u] == 0u);
    CHECK(first.liveGridFnv1a ==
          Fnv1a64(first.liveGrid.data(), first.liveGrid.size()));

    const auto second = TickLoadingPatternRuntime8001EF40(runtime);
    const auto third = TickLoadingPatternRuntime8001EF40(runtime);
    CHECK(second.known && third.known);
    CHECK(!second.mutationApplied && !third.mutationApplied);
    CHECK(second.liveGridFnv1a == first.liveGridFnv1a);
    CHECK(third.liveGridFnv1a == first.liveGridFnv1a);
    CHECK(third.frameCounterGp51 == 3u);

    const auto fourth = TickLoadingPatternRuntime8001EF40(runtime);
    CHECK(fourth.known);
    CHECK(fourth.mutationApplied);
    CHECK(fourth.mutationSerial == 2u);
    CHECK(fourth.bitCursorGp49 == 2u);
    CHECK(fourth.frameCounterGp51 == 4u);
    CHECK(fourth.liveGridFnv1a != first.liveGridFnv1a);

    StopLoadingPatternRuntime8001EF40(runtime);
    CHECK(!runtime.active);
    CHECK(runtime.countersKnown);
    const uint32_t preservedFrameCounter = runtime.frameCounterGp51;
    CHECK(BeginLoadingPatternRuntimeAfter8001FFD4(
        runtime, 3, completed.data(), completed.size()));
    CHECK(runtime.style == 2u);
    CHECK(runtime.bitCursorGp49 == 0u);
    CHECK(runtime.frameCounterGp51 == preservedFrameCounter);
    const auto resumed = TickLoadingPatternRuntime8001EF40(runtime);
    CHECK(resumed.known);
    CHECK(resumed.style == 2u);
    CHECK(resumed.boxFillAttr8001B6C4 == 0x40AE2A02u);
    CHECK(resumed.boxFillGpuColorCode8003EE84 == 0x62022AAEu);
    CHECK(!resumed.mutationApplied);

    auto invalidGrid = completed;
    invalidGrid[17] = 2u;
    CHECK(!BeginLoadingPatternRuntimeAfter8001FFD4(
        runtime, 0, invalidGrid.data(), invalidGrid.size()));
}

void CheckScene0OuterEntryExactTuples801C4DC4(
    const PrSS0TransitionDirect::
        Scene0OuterEntryTransaction801C4DC4& transaction) {
    CHECK(transaction.slowCtxAddress80020110 ==
          PrSS0TransitionDirect::kScene0WorkAddress);
    CHECK(transaction.slowMode80020110 == 2);
    CHECK(transaction.slowPreFfd4Arg80020110 == 1);
    CHECK(transaction.slowPostFfd4Arg80020110 == 2);
    CHECK(transaction.fastCtxAddress800201AC ==
          PrSS0TransitionDirect::kScene0WorkAddress);
    CHECK(transaction.fastMode800201AC == 6);
    CHECK(transaction.fastPreFfd4Arg800201AC == 1);
    CHECK(transaction.fastPostFfd4Arg800201AC == 2);
}

void TestScene0OuterEntryTransaction801C4DC4() {
    using namespace PrSS0TransitionDirect;

    Scene0OuterEntryInput801C4DC4 input{};
    input.contextKnown = true;
    input.contextAddress = PrSS0TransitionDirect::kScene0WorkAddress;

    auto transaction =
        BuildScene0OuterEntryTransaction801C4DC4(input);
    CHECK(transaction.status ==
          Scene0OuterEntryStatus801C4DC4::SourceUnknown);
    CHECK(!transaction.accepted);
    CHECK(!transaction.publishWord800916D2);
    CHECK(!transaction.initialSlowTransitionRequired80020110);

    input.word800916D2Known = true;
    input.contextKnown = false;
    transaction = BuildScene0OuterEntryTransaction801C4DC4(input);
    CHECK(transaction.status ==
          Scene0OuterEntryStatus801C4DC4::InvalidContext);
    CHECK(!transaction.accepted);
    CHECK(!transaction.publishWord800916D2);
    CHECK(!transaction.initialSlowTransitionRequired80020110);

    input.contextKnown = true;
    input.contextAddress =
        PrSS0TransitionDirect::kScene0WorkAddress + 2u;
    transaction = BuildScene0OuterEntryTransaction801C4DC4(input);
    CHECK(transaction.status ==
          Scene0OuterEntryStatus801C4DC4::InvalidContext);
    CHECK(!transaction.accepted);
    CHECK(!transaction.publishWord800916D2);
    CHECK(!transaction.initialSlowTransitionRequired80020110);

    input.contextAddress = PrSS0TransitionDirect::kScene0WorkAddress;
    transaction = BuildScene0OuterEntryTransaction801C4DC4(input);
    CHECK(transaction.status ==
          Scene0OuterEntryStatus801C4DC4::AcceptedInitialSlow);
    CHECK(transaction.accepted);
    CHECK(transaction.publishWord800916D2);
    CHECK(transaction.nextWord800916D2 == 1u);
    CHECK(transaction.initialSlowTransitionRequired80020110);
    CheckScene0OuterEntryExactTuples801C4DC4(transaction);
    const auto firstEntryTransaction = transaction;

    input.word800916D2 = 7u;
    transaction = BuildScene0OuterEntryTransaction801C4DC4(input);
    CHECK(transaction.status ==
          Scene0OuterEntryStatus801C4DC4::AcceptedSkipInitialSlow);
    CHECK(transaction.accepted);
    CHECK(!transaction.publishWord800916D2);
    CHECK(transaction.nextWord800916D2 == 7u);
    CHECK(!transaction.initialSlowTransitionRequired80020110);
    CheckScene0OuterEntryExactTuples801C4DC4(transaction);

    input.word800916D2 = firstEntryTransaction.nextWord800916D2;
    transaction = BuildScene0OuterEntryTransaction801C4DC4(input);
    CHECK(transaction.status ==
          Scene0OuterEntryStatus801C4DC4::AcceptedSkipInitialSlow);
    CHECK(transaction.accepted);
    CHECK(!transaction.publishWord800916D2);
    CHECK(transaction.nextWord800916D2 == 1u);
    CHECK(!transaction.initialSlowTransitionRequired80020110);
    CheckScene0OuterEntryExactTuples801C4DC4(transaction);
}

void TestDecDCTvlc2CurrentIdaImmediateV3End80047B30() {
    std::array<uint8_t, 12u> input{};
    const auto write16 = [](uint8_t* dst, uint16_t value) {
        dst[0] = static_cast<uint8_t>(value & 0xFFu);
        dst[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
    };
    write16(input.data() + 0u, 0x1234u);
    write16(input.data() + 2u, 0x3800u);
    write16(input.data() + 4u, 8u);
    write16(input.data() + 6u, 3u);
    write16(input.data() + 8u, 0xFFC0u);
    write16(input.data() + 10u, 0u);

    std::array<uint8_t, 512u> output{};
    const auto result =
        PrSS0MdecVlcDirect::ExecuteDecDCTvlc2ResetSize80047B30(
            input.data(),
            input.size(),
            output.data(),
            output.size());
    CHECK(result.known);
    CHECK(result.executed);
    CHECK(result.returnValue == 0);
    CHECK(result.inputHalfwordsConsumed == 6u);
    CHECK(result.outputHalfwordsWritten == 67u);
    CHECK(result.outputBytesWritten == 134u);
    CHECK(result.frameCodeCount == 0x1234u);
    CHECK(result.frameMagic == 0x3800u);
    CHECK(result.quantScale == 8u);
    CHECK(result.version == 3u);
    CHECK(result.frameComplete);
    CHECK(result.trailingFe00PaddingWritten);
    CHECK(result.trailingFe00Halfwords == 65u);
    CHECK(result.decDctVlcSize2ResetAuthority);
    CHECK(result.exactCurrentIdaTableAuthority);
    CHECK(result.currentScusSemanticAuthority);
    CHECK(!result.psxPointerAuthority);
    CHECK(!result.psxHardwareMmioAuthority);
    CHECK(!result.hostProjection);
    CHECK(!result.replayValueAuthority);
    CHECK(!result.oldWinS0Authority);
    CHECK(!result.stage2PlusAuthority);
    CHECK(!result.comod2Authority);
    CHECK(output[0] == 0x34u && output[1] == 0x12u);
    CHECK(output[2] == 0x00u && output[3] == 0x38u);
    for (size_t offset = 4u; offset < 134u; offset += 2u) {
        CHECK(output[offset] == 0x00u);
        CHECK(output[offset + 1u] == 0xFEu);
    }
}

} // namespace

int main(int argc, char** argv) {
    TestStrLowerCdStartRuntime8001A4D0();
    TestStrXaAudioDiscRuntime8001A4D0();
    TestStrWorkBaseCleanupRuntime80049428();
    bool exactDiscTested = false;
    if (argc >= 2) {
        TestExactMovie0TDisc8001A4D0(
            std::filesystem::path(argv[1]));
        exactDiscTested = true;
    }
    TestTitleEarlyInputWaitCleanupHostProjection8001A694();
    TestScene0StrAudioRoute801C44E0();
    TestRuntimeGateNamesGeometryAndInputs();
    TestStrDecoderInitialState80027288();
    TestStrStreamStartState800274D4();
    TestStrDecodeAdvanceState800273A4();
    TestDecDCTvlc2CurrentIdaImmediateV3End80047B30();
    TestStrDecodeGateState80027528();
    TestStrStripUploadState8002756C();
    TestStrStopResetState80027664();
    TestMdec15bppCurrentScusSynthetic();
    TestStrSectorRingDirectInput80039670();
    TestStrDecoderMemoryRuntimeAndStopResetExecution80027664();
    TestStrClockPollState801C4350();
    TestStrLowerCdClockRuntime801C4350();
    TestStrLowerCdClockOwnerScope801C4350();
    TestStartPlanOpeningMovieOrderAndArgs();
    TestLoopPlanUnknownAndExitBranches();
    TestCleanupAndCompositePlans();
    TestUnknownMovieGeometryFailsClosed();
    TestScene0FastTransitionCadence800201AC();
    TestScene0SlowTransitionMode1Cadence80020110();
    TestScene0SlowTransitionCadence80020110();
    TestScene0LoadingPatternRuntime8001EF40();
    TestScene0OuterEntryTransaction801C4DC4();

    if (g_failed != 0) {
        std::printf("test_ss0_str_lifecycle: failed checks=%d\n", g_failed);
        return 1;
    }
    std::printf("test_ss0_str_lifecycle: ok exactDisc=%d\n",
                exactDiscTested ? 1 : 0);
    return 0;
}
